/*
 * Copyright (c) 2008 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Development of this code funded by Astaro AG (http://www.astaro.com/)
 */

#include <nft.h>

#include <stddef.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <linux/netfilter.h>
#include <linux/netfilter_arp.h>
#include <linux/netfilter/nf_tables.h>
#include <linux/netfilter/nf_synproxy.h>
#include <linux/netfilter/nf_nat.h>
#include <linux/netfilter/nf_log.h>
#include <linux/netfilter_ipv4.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp6.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <errno.h>

#include <expression.h>
#include <statement.h>
#include <intervals.h>
#include <netlink.h>
#include <time.h>
#include <rule.h>
#include <cache.h>
#include <erec.h>
#include <gmputil.h>
#include <utils.h>
#include <xt.h>

struct proto_ctx *eval_proto_ctx(struct eval_ctx *ctx)
{
	uint8_t idx = ctx->inner_desc ? 1 : 0;

	return &ctx->_pctx[idx];
}

static int expr_evaluate(struct eval_ctx *ctx, struct expr **expr);

static const char * const byteorder_names[] = {
	[BYTEORDER_INVALID]		= "invalid",
	[BYTEORDER_HOST_ENDIAN]		= "host endian",
	[BYTEORDER_BIG_ENDIAN]		= "big endian",
};

#define chain_error(ctx, s1, fmt, args...) \
	__stmt_binary_error(ctx, &(s1)->location, NULL, fmt, ## args)
#define monitor_error(ctx, s1, fmt, args...) \
	__stmt_binary_error(ctx, &(s1)->location, NULL, fmt, ## args)
#define cmd_error(ctx, loc, fmt, args...) \
	__stmt_binary_error(ctx, loc, NULL, fmt, ## args)

static int __fmtstring(3, 4) set_error(struct eval_ctx *ctx,
				       const struct set *set,
				       const char *fmt, ...)
{
	struct error_record *erec;
	va_list ap;

	va_start(ap, fmt);
	erec = erec_vcreate(EREC_ERROR, &set->location, fmt, ap);
	va_end(ap);
	erec_queue(erec, ctx->msgs);
	return -1;
}

const char *stmt_name(const struct stmt *stmt)
{
	switch (stmt->type) {
	case STMT_NAT:
		switch (stmt->nat.type) {
		case NFT_NAT_SNAT:
			return "snat";
		case NFT_NAT_DNAT:
			return "dnat";
		case NFT_NAT_REDIR:
			return "redirect";
		case NFT_NAT_MASQ:
			return "masquerade";
		}
		break;
	default:
		break;
	}

	return stmt_ops(stmt)->name;
}

static int stmt_error_range(struct eval_ctx *ctx, const struct stmt *stmt, const struct expr *e)
{
	return expr_error(ctx->msgs, e, "%s: range argument not supported", stmt_name(stmt));
}

static void key_fix_dtype_byteorder(struct expr *key)
{
	const struct datatype *dtype = key->dtype;

	if (dtype->byteorder == key->byteorder)
		return;

	__datatype_set(key, set_datatype_alloc(dtype, key->byteorder));
}

static int set_evaluate(struct eval_ctx *ctx, struct set *set);
static struct expr *implicit_set_declaration(struct eval_ctx *ctx,
					     const char *name,
					     struct expr *key,
					     struct expr *data,
					     struct expr *expr,
					     uint32_t flags)
{
	struct cmd *cmd;
	struct set *set;
	struct handle h;

	if (set_is_datamap(flags))
		key_fix_dtype_byteorder(key);

	set = set_alloc(&expr->location);
	set->flags	= flags;
	set->handle.set.name = xstrdup(name);
	set->key	= key;
	set->data	= data;
	set->init	= expr;
	set->automerge	= flags & NFT_SET_INTERVAL;

	handle_merge(&set->handle, &ctx->cmd->handle);

	if (set_evaluate(ctx, set) < 0) {
		if (set->flags & (NFT_SET_MAP|NFT_SET_OBJECT))
			set->init = NULL;
		set_free(set);
		return NULL;
	}

	if (ctx->table != NULL)
		list_add_tail(&set->list, &ctx->table->sets);
	else {
		memset(&h, 0, sizeof(h));
		handle_merge(&h, &set->handle);
		h.set.location = expr->location;
		cmd = cmd_alloc(CMD_ADD, CMD_OBJ_SET, &h, &expr->location, set);
		cmd->location = set->location;
		list_add_tail(&cmd->list, &ctx->cmd->list);
	}

	return set_ref_expr_alloc(&expr->location, set);
}

static enum ops byteorder_conversion_op(struct expr *expr,
					enum byteorder byteorder)
{
	switch (expr->byteorder) {
	case BYTEORDER_HOST_ENDIAN:
		if (byteorder == BYTEORDER_BIG_ENDIAN)
			return OP_HTON;
		break;
	case BYTEORDER_BIG_ENDIAN:
		if (byteorder == BYTEORDER_HOST_ENDIAN)
			return OP_NTOH;
		break;
	default:
		break;
	}
	BUG("invalid byte order conversion %u => %u",
	    expr->byteorder, byteorder);
}

static int byteorder_conversion(struct eval_ctx *ctx, struct expr **expr,
				enum byteorder byteorder)
{
	enum datatypes basetype;
	enum ops op;

	assert(!expr_is_constant(*expr) || expr_is_singleton(*expr));

	if ((*expr)->byteorder == byteorder)
		return 0;

	/* Conversion for EXPR_CONCAT is handled for single composing ranges */
	if ((*expr)->etype == EXPR_CONCAT) {
		struct expr *i, *next, *unary;

		list_for_each_entry_safe(i, next, &expr_concat(*expr)->expressions, list) {
			if (i->byteorder == BYTEORDER_BIG_ENDIAN)
				continue;

			basetype = expr_basetype(i)->type;
			if (basetype == TYPE_STRING)
				continue;

			assert(basetype == TYPE_INTEGER);

			switch (i->etype) {
			case EXPR_VALUE:
				if (i->byteorder == BYTEORDER_HOST_ENDIAN)
					mpz_switch_byteorder(i->value, div_round_up(i->len, BITS_PER_BYTE));
				break;
			default:
				if (div_round_up(i->len, BITS_PER_BYTE) >= 2) {
					op = byteorder_conversion_op(i, byteorder);
					unary = unary_expr_alloc(&i->location, op, i);
					if (expr_evaluate(ctx, &unary) < 0)
						return -1;

					list_replace(&i->list, &unary->list);
				}
			}
		}

		return 0;
	}

	basetype = expr_basetype(*expr)->type;
	switch (basetype) {
	case TYPE_INTEGER:
		break;
	case TYPE_STRING:
		return 0;
	default:
		return expr_error(ctx->msgs, *expr,
				  "Byteorder mismatch: expected %s, %s got %s",
				  byteorder_names[byteorder],
				  expr_name(*expr),
				  byteorder_names[(*expr)->byteorder]);
	}

	if (expr_is_constant(*expr) || div_round_up((*expr)->len, BITS_PER_BYTE) < 2)
		(*expr)->byteorder = byteorder;
	else {
		op = byteorder_conversion_op(*expr, byteorder);
		*expr = unary_expr_alloc(&(*expr)->location, op, *expr);
		if (expr_evaluate(ctx, expr) < 0)
			return -1;
	}
	return 0;
}

static int table_not_found(struct eval_ctx *ctx)
{
	struct table *table;

	table = table_lookup_fuzzy(&ctx->cmd->handle, &ctx->nft->cache);
	if (table == NULL)
		return cmd_error(ctx, &ctx->cmd->handle.table.location,
				 "%s", strerror(ENOENT));

	return cmd_error(ctx, &ctx->cmd->handle.table.location,
			 "%s; did you mean table '%s' in family %s?",
			 strerror(ENOENT), table->handle.table.name,
			 family2str(table->handle.family));
}

static int chain_not_found(struct eval_ctx *ctx)
{
	const struct table *table;
	struct chain *chain;

	chain = chain_lookup_fuzzy(&ctx->cmd->handle, &ctx->nft->cache, &table);
	if (chain == NULL)
		return cmd_error(ctx, &ctx->cmd->handle.chain.location,
				 "%s", strerror(ENOENT));

	return cmd_error(ctx, &ctx->cmd->handle.chain.location,
			 "%s; did you mean chain '%s' in table %s '%s'?",
			 strerror(ENOENT), chain->handle.chain.name,
			 family2str(chain->handle.family),
			 table->handle.table.name);
}

static int set_not_found(struct eval_ctx *ctx, const struct location *loc,
			 const char *set_name)
{
	const struct table *table;
	struct set *set;

	set = set_lookup_fuzzy(set_name, &ctx->nft->cache, &table);
	if (set == NULL)
		return cmd_error(ctx, loc, "%s", strerror(ENOENT));

	return cmd_error(ctx, loc,
			 "%s; did you mean %s '%s' in table %s '%s'?",
			 strerror(ENOENT),
			 set_is_map(set->flags) ? "map" : "set",
			 set->handle.set.name,
			 family2str(set->handle.family),
			 table->handle.table.name);
}

static int flowtable_not_found(struct eval_ctx *ctx, const struct location *loc,
			       const char *ft_name)
{
	const struct table *table;
	struct flowtable *ft;

	ft = flowtable_lookup_fuzzy(ft_name, &ctx->nft->cache, &table);
	if (!ft)
		return cmd_error(ctx, loc, "%s", strerror(ENOENT));

	return cmd_error(ctx, loc,
			"%s; did you mean flowtable '%s' in table %s '%s'?",
			strerror(ENOENT), ft->handle.flowtable.name,
			family2str(ft->handle.family),
			table->handle.table.name);
}

/*
 * Symbol expression: parse symbol and evaluate resulting expression.
 */
static int expr_evaluate_symbol(struct eval_ctx *ctx, struct expr **expr)
{
	struct parse_ctx parse_ctx = {
		.tbl	= &ctx->nft->output.tbl,
		.input	= &ctx->nft->input,
	};
	struct error_record *erec;
	struct table *table;
	struct set *set;
	struct expr *new;

	switch ((*expr)->symtype) {
	case SYMBOL_VALUE:
		datatype_set(*expr, ctx->ectx.dtype);
		erec = symbol_parse(&parse_ctx, *expr, &new);
		if (erec != NULL) {
			erec_queue(erec, ctx->msgs);
			return -1;
		}
		break;
	case SYMBOL_SET:
		table = table_cache_find(&ctx->nft->cache.table_cache,
					 ctx->cmd->handle.table.name,
					 ctx->cmd->handle.family);
		if (table == NULL)
			return table_not_found(ctx);

		set = set_cache_find(table, (*expr)->identifier);
		if (set == NULL || !set->key)
			return set_not_found(ctx, &(*expr)->location,
					     (*expr)->identifier);

		new = set_ref_expr_alloc(&(*expr)->location, set);
		break;
	}

	expr_free(*expr);
	*expr = new;

	return expr_evaluate(ctx, expr);
}

static int expr_evaluate_string(struct eval_ctx *ctx, struct expr **exprp)
{
	struct expr *expr = *exprp;
	unsigned int len = div_round_up(expr->len, BITS_PER_BYTE), datalen;
	struct expr *value, *prefix;
	int data_len = ctx->ectx.len > 0 ? ctx->ectx.len : len + 1;
	char data[data_len];

	if (ctx->ectx.len > 0) {
		if (expr->len > ctx->ectx.len)
			return expr_error(ctx->msgs, expr,
					  "String exceeds maximum length of %u",
					  ctx->ectx.len / BITS_PER_BYTE);
		expr->len = ctx->ectx.len;
	}

	memset(data + len, 0, data_len - len);
	mpz_export_data(data, expr->value, BYTEORDER_HOST_ENDIAN, len);

	if (strlen(data) == 0)
		return expr_error(ctx->msgs, expr,
				  "Empty string is not allowed");

	datalen = strlen(data) - 1;
	if (data[datalen] != '*') {
		/* We need to reallocate the constant expression with the right
		 * expression length to avoid problems on big endian.
		 */
		value = constant_expr_alloc(&expr->location, ctx->ectx.dtype,
					    BYTEORDER_HOST_ENDIAN,
					    expr->len, data);
		expr_free(expr);
		*exprp = value;
		return 0;
	}

	if (datalen == 0)
		return expr_error(ctx->msgs, expr,
				  "All-wildcard strings are not supported");

	if (data[datalen - 1] == '\\') {
		char unescaped_str[data_len];

		memset(unescaped_str, 0, sizeof(unescaped_str));
		xstrunescape(data, unescaped_str);

		value = constant_expr_alloc(&expr->location, ctx->ectx.dtype,
					    BYTEORDER_HOST_ENDIAN,
					    expr->len, unescaped_str);
		expr_free(expr);
		*exprp = value;
		return 0;
	}

	data[datalen] = 0;
	value = constant_expr_alloc(&expr->location, ctx->ectx.dtype,
				    BYTEORDER_HOST_ENDIAN,
				    expr->len, data);

	prefix = prefix_expr_alloc(&expr->location, value,
				   datalen * BITS_PER_BYTE);
	datatype_set(prefix, ctx->ectx.dtype);
	prefix->flags |= EXPR_F_CONSTANT;
	prefix->byteorder = BYTEORDER_HOST_ENDIAN;
	prefix->len = expr->len;

	expr_free(expr);
	*exprp = prefix;
	return 0;
}

static int expr_evaluate_integer(struct eval_ctx *ctx, struct expr **exprp)
{
	struct expr *expr = *exprp;
	char *valstr, *rangestr;
	uint32_t masklen;
	mpz_t mask;

	if (ctx->ectx.maxval > 0 &&
	    mpz_cmp_ui(expr->value, ctx->ectx.maxval) > 0) {
		valstr = mpz_get_str(NULL, 10, expr->value);
		expr_error(ctx->msgs, expr,
			   "Value %s exceeds valid range 0-%u",
			   valstr, ctx->ectx.maxval);
		nft_gmp_free(valstr);
		return -1;
	}

	if (ctx->stmt_len > ctx->ectx.len)
		masklen = ctx->stmt_len;
	else
		masklen = ctx->ectx.len;

	mpz_init_bitmask(mask, masklen);
	if (mpz_cmp(expr->value, mask) > 0) {
		valstr = mpz_get_str(NULL, 10, expr->value);
		rangestr = mpz_get_str(NULL, 10, mask);
		expr_error(ctx->msgs, expr,
			   "Value %s exceeds valid range 0-%s",
			   valstr, rangestr);
		nft_gmp_free(valstr);
		nft_gmp_free(rangestr);
		mpz_clear(mask);
		return -1;
	}
	expr->byteorder = ctx->ectx.byteorder;
	expr->len = masklen;
	mpz_clear(mask);
	return 0;
}

static int expr_evaluate_value(struct eval_ctx *ctx, struct expr **expr)
{
	switch (expr_basetype(*expr)->type) {
	case TYPE_INTEGER:
		if (expr_evaluate_integer(ctx, expr) < 0)
			return -1;
		break;
	case TYPE_STRING:
		if (expr_evaluate_string(ctx, expr) < 0)
			return -1;
		break;
	default:
		return expr_error(ctx->msgs, *expr, "Unexpected datatype %s",
				  (*expr)->dtype->name);
	}
	return 0;
}

/*
 * Primary expressions determine the datatype context.
 */
static int expr_evaluate_primary(struct eval_ctx *ctx, struct expr **expr)
{
	__expr_set_context(&ctx->ectx, (*expr)->dtype, (*expr)->byteorder,
			   (*expr)->len, 0);
	return 0;
}

int stmt_dependency_evaluate(struct eval_ctx *ctx, struct stmt *stmt)
{
	uint32_t stmt_len = ctx->stmt_len;

	if (stmt_evaluate(ctx, stmt) < 0)
		return stmt_error(ctx, stmt, "dependency statement is invalid");

	ctx->stmt_len = stmt_len;

	return 0;
}

static int
ll_conflict_resolution_gen_dependency(struct eval_ctx *ctx, int protocol,
				      const struct expr *expr,
				      struct stmt **res)
{
	enum proto_bases base = expr->payload.base;
	const struct proto_hdr_template *tmpl;
	const struct proto_desc *desc = NULL;
	struct expr *dep, *left, *right;
	struct proto_ctx *pctx;
	struct stmt *stmt;

	assert(expr->payload.base == PROTO_BASE_LL_HDR);

	pctx = eval_proto_ctx(ctx);
	desc = pctx->protocol[base].desc;
	tmpl = &desc->templates[desc->protocol_key];
	left = payload_expr_alloc(&expr->location, desc, desc->protocol_key);

	right = constant_expr_alloc(&expr->location, tmpl->dtype,
				    tmpl->dtype->byteorder, tmpl->len,
				    constant_data_ptr(protocol, tmpl->len));

	dep = relational_expr_alloc(&expr->location, OP_EQ, left, right);
	stmt = expr_stmt_alloc(&dep->location, dep);
	if (stmt_dependency_evaluate(ctx, stmt) < 0)
		return expr_error(ctx->msgs, expr,
					  "dependency statement is invalid");

	if (ctx->inner_desc)
		left->payload.inner_desc = ctx->inner_desc;

	*res = stmt;
	return 0;
}

static uint8_t expr_offset_shift(const struct expr *expr, unsigned int offset,
				 unsigned int *extra_len)
{
	unsigned int new_offset, len;
	int shift;

	new_offset = offset % BITS_PER_BYTE;
	len = round_up(expr->len, BITS_PER_BYTE);
	shift = len - (new_offset + expr->len);
	while (shift < 0) {
		shift += BITS_PER_BYTE;
		*extra_len += BITS_PER_BYTE;
	}
	return shift;
}

static int expr_evaluate_bits(struct eval_ctx *ctx, struct expr **exprp)
{
	struct expr *expr = *exprp, *and, *mask, *rshift, *off;
	unsigned masklen, len = expr->len, extra_len = 0;
	enum byteorder byteorder;
	uint8_t shift;
	mpz_t bitmask;

	/* payload statement with relational expression as a value does not
	 * require the transformations that are needed for payload matching,
	 * skip this.
	 */
	if (ctx->stmt && ctx->stmt->type == STMT_PAYLOAD)
		return 0;

	switch (expr->etype) {
	case EXPR_PAYLOAD:
		shift = expr_offset_shift(expr, expr->payload.offset,
					  &extra_len);
		break;
	case EXPR_EXTHDR:
		shift = expr_offset_shift(expr, expr->exthdr.offset,
					  &extra_len);
		break;
	default:
		BUG("Unknown expression %s", expr_name(expr));
	}

	masklen = len + shift;

	if (masklen > NFT_REG_SIZE * BITS_PER_BYTE)
		return expr_error(ctx->msgs, expr, "mask length %u exceeds allowed maximum of %u\n",
				  masklen, NFT_REG_SIZE * BITS_PER_BYTE);

	mpz_init2(bitmask, masklen);
	mpz_bitmask(bitmask, len);
	mpz_lshift_ui(bitmask, shift);

	mask = constant_expr_alloc(&expr->location, expr_basetype(expr),
				   BYTEORDER_HOST_ENDIAN, masklen, NULL);
	mpz_set(mask->value, bitmask);
	mpz_clear(bitmask);

	and = binop_expr_alloc(&expr->location, OP_AND, expr, mask);
	and->dtype	= expr->dtype;
	and->byteorder	= expr->byteorder;
	and->len	= masklen;

	if (shift) {
		if ((ctx->ectx.key || ctx->stmt_len > 0) &&
		    div_round_up(masklen, BITS_PER_BYTE) > 1) {
			int op = byteorder_conversion_op(expr, BYTEORDER_HOST_ENDIAN);
			and = unary_expr_alloc(&expr->location, op, and);
			and->len = masklen;
			byteorder = BYTEORDER_HOST_ENDIAN;
		} else {
			byteorder = expr->byteorder;
		}

		off = constant_expr_alloc(&expr->location,
					  expr_basetype(expr),
					  BYTEORDER_HOST_ENDIAN,
					  sizeof(shift), &shift);

		rshift = binop_expr_alloc(&expr->location, OP_RSHIFT, and, off);
		rshift->dtype		= expr->dtype;
		rshift->byteorder	= byteorder;
		rshift->len		= masklen;

		*exprp = rshift;
	} else
		*exprp = and;

	if (extra_len)
		expr->len += extra_len;

	return 0;
}

static int __expr_evaluate_exthdr(struct eval_ctx *ctx, struct expr **exprp)
{
	const struct expr *key = ctx->ectx.key;
	struct expr *expr = *exprp;

	if (expr->exthdr.flags & NFT_EXTHDR_F_PRESENT)
		datatype_set(expr, &boolean_type);

	if (expr_evaluate_primary(ctx, exprp) < 0)
		return -1;

	ctx->ectx.key = key;

	if (expr->exthdr.offset % BITS_PER_BYTE != 0 ||
	    expr->len % BITS_PER_BYTE != 0) {
		int err = expr_evaluate_bits(ctx, exprp);

		if (err)
			return err;
	}

	switch (expr->exthdr.op) {
	case NFT_EXTHDR_OP_TCPOPT: {
		static const unsigned int max_tcpoptlen = (15 * 4 - 20) * BITS_PER_BYTE;
		unsigned int totlen;

		totlen = expr->exthdr.tmpl->len + expr->exthdr.offset;

		if (totlen > max_tcpoptlen)
			return expr_error(ctx->msgs, expr,
					  "offset and size %u exceeds max tcp headerlen (%u)",
					  totlen, max_tcpoptlen);
		break;
	}
	case NFT_EXTHDR_OP_IPV4: {
		static const unsigned int max_ipoptlen = 40 * BITS_PER_BYTE;
		unsigned int totlen;

		totlen = expr->exthdr.offset + expr->exthdr.tmpl->len;

		if (totlen > max_ipoptlen)
			return expr_error(ctx->msgs, expr,
					  "offset and size %u exceeds max ip option len (%u)",
					  totlen, max_ipoptlen);
		break;
	}
	default:
		break;
	}

	return 0;
}

/*
 * Exthdr expression: check whether dependencies are fulfilled, otherwise
 * generate the necessary relational expression and prepend it to the current
 * statement.
 */
static int expr_evaluate_exthdr(struct eval_ctx *ctx, struct expr **exprp)
{
	const struct proto_desc *base, *dependency = NULL;
	enum proto_bases pb = PROTO_BASE_NETWORK_HDR;
	struct expr *expr = *exprp;
	struct proto_ctx *pctx;
	struct stmt *nstmt;

	switch (expr->exthdr.op) {
	case NFT_EXTHDR_OP_TCPOPT:
	case NFT_EXTHDR_OP_SCTP:
	case NFT_EXTHDR_OP_DCCP:
		return __expr_evaluate_exthdr(ctx, exprp);
	case NFT_EXTHDR_OP_IPV4:
		dependency = &proto_ip;
		break;
	case NFT_EXTHDR_OP_IPV6:
	default:
		dependency = &proto_ip6;
		break;
	}

	assert(dependency);

	pctx = eval_proto_ctx(ctx);
	base = pctx->protocol[pb].desc;
	if (base == dependency)
		return __expr_evaluate_exthdr(ctx, exprp);

	if (base)
		return expr_error(ctx->msgs, expr,
				  "cannot use exthdr with %s", base->name);

	if (exthdr_gen_dependency(ctx, expr, dependency, pb - 1, &nstmt) < 0)
		return -1;

	list_add(&nstmt->list, &ctx->rule->stmts);

	return __expr_evaluate_exthdr(ctx, exprp);
}

/* dependency supersede.
 *
 * 'inet' is a 'phony' l2 dependency used by NFPROTO_INET to fulfil network
 * header dependency, i.e. ensure that 'ip saddr 1.2.3.4' only sees ip headers.
 *
 * If a match expression that depends on a particular L2 header, e.g. ethernet,
 * is used, we thus get a conflict since we already have a l2 header dependency.
 *
 * But in the inet case we can just ignore the conflict since only another
 * restriction is added, and these are not mutually exclusive.
 *
 * Example: inet filter in ip saddr 1.2.3.4 ether saddr a:b:c:d:e:f
 *
 * ip saddr adds meta dependency on ipv4 packets
 * ether saddr adds another dependency on ethernet frames.
 */
static int meta_iiftype_gen_dependency(struct eval_ctx *ctx,
				       struct expr *payload, struct stmt **res)
{
	struct stmt *nstmt;
	uint16_t type;

	if (proto_dev_type(payload->payload.desc, &type) < 0)
		return expr_error(ctx->msgs, payload,
				  "protocol specification is invalid "
				  "for this family");

	nstmt = meta_stmt_meta_iiftype(&payload->location, type);
	if (stmt_dependency_evaluate(ctx, nstmt) < 0)
		return -1;

	if (ctx->inner_desc)
		nstmt->expr->left->meta.inner_desc = ctx->inner_desc;

	*res = nstmt;
	return 0;
}

static bool proto_is_dummy(const struct proto_desc *desc)
{
	return desc == &proto_inet || desc == &proto_netdev;
}

static int stmt_dep_conflict(struct eval_ctx *ctx, const struct stmt *nstmt)
{
	struct stmt *stmt;

	list_for_each_entry(stmt, &ctx->rule->stmts, list) {
		if (stmt == nstmt)
			break;

		if (stmt->type != STMT_EXPRESSION ||
		    stmt->expr->etype != EXPR_RELATIONAL ||
		    stmt->expr->right->etype != EXPR_VALUE ||
		    stmt->expr->left->etype != EXPR_PAYLOAD ||
		    stmt->expr->left->etype != nstmt->expr->left->etype ||
		    stmt->expr->left->len != nstmt->expr->left->len)
			continue;

		if (stmt->expr->left->payload.desc != nstmt->expr->left->payload.desc ||
		    stmt->expr->left->payload.inner_desc != nstmt->expr->left->payload.inner_desc ||
		    stmt->expr->left->payload.base != nstmt->expr->left->payload.base ||
		    stmt->expr->left->payload.offset != nstmt->expr->left->payload.offset)
			continue;

		return stmt_binary_error(ctx, stmt, nstmt,
					 "conflicting statements");
	}

	return 0;
}

static int rule_stmt_dep_add(struct eval_ctx *ctx,
			     struct stmt *nstmt, struct stmt *stmt)
{
	rule_stmt_insert_at(ctx->rule, nstmt, ctx->stmt);

	if (stmt_dep_conflict(ctx, nstmt) < 0)
		return -1;

	return 0;
}

static int resolve_ll_protocol_conflict(struct eval_ctx *ctx,
				        const struct proto_desc *desc,
					struct expr *payload)
{
	enum proto_bases base = payload->payload.base;
	struct stmt *nstmt = NULL;
	struct proto_ctx *pctx;
	unsigned int i;
	int link, err;

	assert(base == PROTO_BASE_LL_HDR);

	pctx = eval_proto_ctx(ctx);

	if (proto_is_dummy(desc)) {
		if (ctx->inner_desc) {
	                proto_ctx_update(pctx, PROTO_BASE_LL_HDR, &payload->location, &proto_eth);
		} else {
			err = meta_iiftype_gen_dependency(ctx, payload, &nstmt);
			if (err < 0)
				return err;

			desc = payload->payload.desc;
			if (rule_stmt_dep_add(ctx, nstmt, ctx->stmt) < 0)
				return -1;
		}
	} else {
		/* payload desc stored in the L2 header stack? No conflict. */
		for (i = 0; i < pctx->stacked_ll_count; i++) {
			if (pctx->stacked_ll[i] == payload->payload.desc)
				return 0;
		}
	}

	link = proto_find_num(desc, payload->payload.desc);
	if (link < 0 ||
	    ll_conflict_resolution_gen_dependency(ctx, link, payload, &nstmt) < 0)
		return 1;

	for (i = 0; i < pctx->stacked_ll_count; i++)
		payload->payload.offset += pctx->stacked_ll[i]->length;

	if (rule_stmt_dep_add(ctx, nstmt, ctx->stmt) < 0)
		return -1;

	return 0;
}

/*
 * Payload expression: check whether dependencies are fulfilled, otherwise
 * generate the necessary relational expression and prepend it to the current
 * statement.
 */
static int __expr_evaluate_payload(struct eval_ctx *ctx, struct expr *expr)
{
	struct expr *payload = expr;
	enum proto_bases base = payload->payload.base;
	const struct proto_desc *desc;
	struct proto_ctx *pctx;
	struct stmt *nstmt;
	int err;

	if (expr->etype == EXPR_PAYLOAD && expr->payload.is_raw)
		return 0;

	pctx = eval_proto_ctx(ctx);
	desc = pctx->protocol[base].desc;
	if (desc == NULL) {
		if (payload_gen_dependency(ctx, payload, &nstmt) < 0)
			return -1;

		if (rule_stmt_dep_add(ctx, nstmt, ctx->stmt) < 0)
			return -1;

		desc = pctx->protocol[base].desc;

		if (desc == expr->payload.desc)
			goto check_icmp;

		if (base == PROTO_BASE_LL_HDR) {
			int link;

			link = proto_find_num(desc, payload->payload.desc);
			if (link < 0 ||
			    ll_conflict_resolution_gen_dependency(ctx, link, payload, &nstmt) < 0)
				return expr_error(ctx->msgs, payload,
						  "conflicting protocols specified: %s vs. %s",
						  desc->name,
						  payload->payload.desc->name);

			assert(pctx->stacked_ll_count);
			payload->payload.offset += pctx->stacked_ll[0]->length;

			if (rule_stmt_dep_add(ctx, nstmt, ctx->stmt) < 0)
				return -1;

			return 1;
		}
		goto check_icmp;
	}

	if (payload->payload.base == desc->base &&
	    proto_ctx_is_ambiguous(pctx, base)) {
		desc = proto_ctx_find_conflict(pctx, base, payload->payload.desc);
		assert(desc);

		return expr_error(ctx->msgs, payload,
				  "conflicting protocols specified: %s vs. %s",
				  desc->name,
				  payload->payload.desc->name);
	}

	/* No conflict: Same payload protocol as context, adjust offset
	 * if needed.
	 */
	if (desc == payload->payload.desc) {
		const struct proto_hdr_template *tmpl;

		if (desc->base == PROTO_BASE_LL_HDR) {
			unsigned int i;

			for (i = 0; i < pctx->stacked_ll_count; i++)
				payload->payload.offset += pctx->stacked_ll[i]->length;
		}
check_icmp:
		if (desc != &proto_icmp && desc != &proto_icmp6)
			return 0;

		tmpl = expr->payload.tmpl;

		if (!tmpl || !tmpl->icmp_dep)
			return 0;

		if (payload_gen_icmp_dependency(ctx, expr, &nstmt) < 0)
			return -1;

		if (nstmt && rule_stmt_dep_add(ctx, nstmt, ctx->stmt) < 0)
			return -1;

		return 0;
	}
	/* If we already have context and this payload is on the same
	 * base, try to resolve the protocol conflict.
	 */
	if (base == PROTO_BASE_LL_HDR) {
		err = resolve_ll_protocol_conflict(ctx, desc, payload);
		if (err <= 0)
			return err;

		desc = pctx->protocol[base].desc;
		if (desc == payload->payload.desc)
			return 0;
	}
	return expr_error(ctx->msgs, payload,
			  "conflicting %s protocols specified: %s vs. %s",
			  proto_base_names[base],
			  pctx->protocol[base].desc->name,
			  payload->payload.desc->name);
}

static bool payload_needs_adjustment(const struct expr *expr)
{
	return expr->payload.offset % BITS_PER_BYTE != 0 ||
	       expr->len % BITS_PER_BYTE != 0;
}

static int expr_evaluate_payload(struct eval_ctx *ctx, struct expr **exprp)
{
	const struct expr *key = ctx->ectx.key;
	struct expr *expr = *exprp;

	if (expr->payload.evaluated)
		return 0;

	if (__expr_evaluate_payload(ctx, expr) < 0)
		return -1;

	if (expr_evaluate_primary(ctx, exprp) < 0)
		return -1;

	ctx->ectx.key = key;

	if (payload_needs_adjustment(expr)) {
		int err = expr_evaluate_bits(ctx, exprp);

		if (err)
			return err;
	}

	expr->payload.evaluated = true;

	return 0;
}

static int expr_evaluate_inner(struct eval_ctx *ctx, struct expr **exprp)
{
	struct proto_ctx *pctx = eval_proto_ctx(ctx);
	const struct proto_desc *desc = NULL;
	struct expr *expr = *exprp;
	int ret;

	assert(expr->etype == EXPR_PAYLOAD);

	pctx = eval_proto_ctx(ctx);
	desc = pctx->protocol[PROTO_BASE_TRANSPORT_HDR].desc;

	if (desc == NULL &&
	    expr->payload.inner_desc->base < PROTO_BASE_INNER_HDR) {
		struct stmt *nstmt;

		if (payload_gen_inner_dependency(ctx, expr, &nstmt) < 0)
			return -1;

		if (rule_stmt_dep_add(ctx, nstmt, ctx->stmt) < 0)
			return -1;

		proto_ctx_update(pctx, PROTO_BASE_TRANSPORT_HDR, &expr->location, expr->payload.inner_desc);
	}

	if (expr->payload.inner_desc->base == PROTO_BASE_INNER_HDR) {
		desc = pctx->protocol[expr->payload.inner_desc->base - 1].desc;
		if (!desc) {
			return expr_error(ctx->msgs, expr,
					  "no transport protocol specified");
		}

		if (proto_find_num(desc, expr->payload.inner_desc) < 0) {
			return expr_error(ctx->msgs, expr,
					  "unexpected transport protocol %s",
					  desc->name);
		}

		proto_ctx_update(pctx, expr->payload.inner_desc->base, &expr->location,
				 expr->payload.inner_desc);
	}

	if (expr->payload.base != PROTO_BASE_INNER_HDR)
		ctx->inner_desc = expr->payload.inner_desc;

	ret = expr_evaluate_payload(ctx, exprp);

	return ret;
}

static int expr_evaluate_payload_inner(struct eval_ctx *ctx, struct expr **exprp)
{
	int ret;

	if ((*exprp)->payload.inner_desc)
		ret = expr_evaluate_inner(ctx, exprp);
	else
		ret = expr_evaluate_payload(ctx, exprp);

	return ret;
}

/*
 * RT expression: validate protocol dependencies.
 */
static int expr_evaluate_rt(struct eval_ctx *ctx, struct expr **expr)
{
	static const char emsg[] = "cannot determine ip protocol version, use \"ip nexthop\" or \"ip6 nexthop\" instead";
	struct expr *rt = *expr;
	struct proto_ctx *pctx;

	pctx = eval_proto_ctx(ctx);
	rt_expr_update_type(pctx, rt);

	switch (rt->rt.key) {
	case NFT_RT_NEXTHOP4:
		if (rt->dtype != &ipaddr_type)
			return expr_error(ctx->msgs, rt, "%s", emsg);
		if (pctx->family == NFPROTO_IPV6)
			return expr_error(ctx->msgs, rt, "%s nexthop will not match", "ip");
		break;
	case NFT_RT_NEXTHOP6:
		if (rt->dtype != &ip6addr_type)
			return expr_error(ctx->msgs, rt, "%s", emsg);
		if (pctx->family == NFPROTO_IPV4)
			return expr_error(ctx->msgs, rt, "%s nexthop will not match", "ip6");
		break;
	default:
		break;
	}

	return expr_evaluate_primary(ctx, expr);
}

static int ct_gen_nh_dependency(struct eval_ctx *ctx, struct expr *ct)
{
	const struct proto_desc *base, *base_now;
	struct expr *left, *right, *dep;
	struct stmt *nstmt = NULL;
	struct proto_ctx *pctx;

	pctx = eval_proto_ctx(ctx);
	base_now = pctx->protocol[PROTO_BASE_NETWORK_HDR].desc;

	switch (ct->ct.nfproto) {
	case NFPROTO_IPV4:
		base = &proto_ip;
		break;
	case NFPROTO_IPV6:
		base = &proto_ip6;
		break;
	default:
		base = pctx->protocol[PROTO_BASE_NETWORK_HDR].desc;
		if (base == &proto_ip)
			ct->ct.nfproto = NFPROTO_IPV4;
		else if (base == &proto_ip6)
			ct->ct.nfproto = NFPROTO_IPV6;

		if (base)
			break;

		return expr_error(ctx->msgs, ct,
				  "cannot determine ip protocol version, use \"ip %1$caddr\" or \"ip6 %1$caddr\" instead",
				  ct->ct.key == NFT_CT_SRC ? 's' : 'd');
	}

	/* no additional dependency needed? */
	if (base == base_now)
		return 0;

	if (base_now && base_now != base)
		return expr_error(ctx->msgs, ct,
				  "conflicting dependencies: %s vs. %s\n",
				  base->name,
				  pctx->protocol[PROTO_BASE_NETWORK_HDR].desc->name);
	switch (pctx->family) {
	case NFPROTO_IPV4:
	case NFPROTO_IPV6:
		return 0;
	}

	left = ct_expr_alloc(&ct->location, NFT_CT_L3PROTOCOL, ct->ct.direction);

	right = constant_expr_alloc(&ct->location, left->dtype,
				    left->dtype->byteorder, left->len,
				    constant_data_ptr(ct->ct.nfproto, left->len));
	dep = relational_expr_alloc(&ct->location, OP_EQ, left, right);

	relational_expr_pctx_update(pctx, dep);

	nstmt = expr_stmt_alloc(&dep->location, dep);

	if (rule_stmt_dep_add(ctx, nstmt, ctx->stmt) < 0)
		return -1;

	return 0;
}

/*
 * CT expression: update the protocol dependant types bases on the protocol
 * context.
 */
static int expr_evaluate_ct(struct eval_ctx *ctx, struct expr **expr)
{
	const struct proto_desc *base, *error;
	struct expr *ct = *expr;
	struct proto_ctx *pctx;

	pctx = eval_proto_ctx(ctx);
	base = pctx->protocol[PROTO_BASE_NETWORK_HDR].desc;

	switch (ct->ct.key) {
	case NFT_CT_SRC:
	case NFT_CT_DST:
		if (ct_gen_nh_dependency(ctx, ct) < 0)
			return -1;
		break;
	case NFT_CT_SRC_IP:
	case NFT_CT_DST_IP:
		if (base == &proto_ip6) {
			error = &proto_ip;
			goto err_conflict;
		}
		break;
	case NFT_CT_SRC_IP6:
	case NFT_CT_DST_IP6:
		if (base == &proto_ip) {
			error = &proto_ip6;
			goto err_conflict;
		}
		break;
	default:
		break;
	}

	ct_expr_update_type(pctx, ct);

	return expr_evaluate_primary(ctx, expr);

err_conflict:
	return stmt_binary_error(ctx, ct,
				 &pctx->protocol[PROTO_BASE_NETWORK_HDR],
				 "conflicting protocols specified: %s vs. %s",
				 base->name, error->name);
}

/*
 * Prefix expression: the argument must be a constant value of integer or
 * string base type; the prefix length must be less than or equal to the type
 * width.
 */
static int expr_evaluate_prefix(struct eval_ctx *ctx, struct expr **expr)
{
	struct expr *prefix = *expr, *base, *and, *mask;

	if (expr_evaluate(ctx, &prefix->prefix) < 0)
		return -1;
	base = prefix->prefix;

	if (!expr_is_constant(base))
		return expr_error(ctx->msgs, prefix,
				  "Prefix expression is undefined for "
				  "non-constant expressions");

	switch (expr_basetype(base)->type) {
	case TYPE_INTEGER:
	case TYPE_STRING:
		break;
	default:
		return expr_error(ctx->msgs, prefix,
				  "Prefix expression is undefined for "
				  "%s types", base->dtype->desc);
	}

	if (prefix->prefix_len > base->len)
		return expr_error(ctx->msgs, prefix,
				  "Prefix length %u is invalid for type "
				  "of %u bits width",
				  prefix->prefix_len, base->len);

	/* Clear the uncovered bits of the base value */
	mask = constant_expr_alloc(&prefix->location, expr_basetype(base),
				   BYTEORDER_HOST_ENDIAN, base->len, NULL);
	switch (expr_basetype(base)->type) {
	case TYPE_INTEGER:
		mpz_prefixmask(mask->value, base->len, prefix->prefix_len);
		break;
	case TYPE_STRING:
		mpz_bitmask(mask->value, prefix->prefix_len);
		break;
	}
	and  = binop_expr_alloc(&prefix->location, OP_AND, base, mask);
	prefix->prefix = and;
	if (expr_evaluate(ctx, &prefix->prefix) < 0)
		return -1;
	base = prefix->prefix;

	/* expr_evaluate may simplify EXPR_AND to another
	 * prefix expression for inputs like "2.2.2.2.3*1"/80.
	 *
	 * Recurse until all the expressions have been simplified.
	 * This also gets us the error checks for the expression
	 * chain.
	 */
	if (base->etype == EXPR_PREFIX)
		return expr_evaluate_prefix(ctx, &prefix->prefix);
	assert(expr_is_constant(base));

	prefix->dtype	  = datatype_get(base->dtype);
	prefix->byteorder = base->byteorder;
	prefix->len	  = base->len;
	prefix->flags	 |= EXPR_F_CONSTANT;
	return 0;
}

/*
 * Range expression: both sides must be constants of integer base type.
 */
static int expr_evaluate_range_expr(struct eval_ctx *ctx,
				    const struct expr *range,
				    struct expr **expr)
{
	if (expr_evaluate(ctx, expr) < 0)
		return -1;

	if (expr_basetype(*expr)->type != TYPE_INTEGER)
		return expr_binary_error(ctx->msgs, *expr, range,
					 "Range expression is undefined for "
					 "%s types", (*expr)->dtype->desc);
	if (!expr_is_constant(*expr))
		return expr_binary_error(ctx->msgs, *expr, range,
					 "Range is not constant");
	return 0;
}

static int __expr_evaluate_range(struct eval_ctx *ctx, struct expr **expr)
{
	struct expr *range = *expr;

	if (expr_evaluate_range_expr(ctx, range, &range->left) < 0)
		return -1;
	if (expr_evaluate_range_expr(ctx, range, &range->right) < 0)
		return -1;

	return 0;
}

static int expr_evaluate_range(struct eval_ctx *ctx, struct expr **exprp)
{
	struct expr *range = *exprp, *left, *right;
	int rc;

	rc = __expr_evaluate_range(ctx, exprp);
	if (rc)
		return rc;

	left = range->left;
	right = range->right;

	if (mpz_cmp(left->value, right->value) > 0)
		return expr_error(ctx->msgs, range, "Range negative size");

	if (mpz_cmp(left->value, right->value) == 0) {
		*exprp = expr_get(left);
		expr_free(range);
		return 0;
	}

	datatype_set(range, left->dtype);
	range->flags |= EXPR_F_CONSTANT;
	return 0;
}

/*
 * Unary expressions: unary expressions are only generated internally for
 * byteorder conversion of non-constant numerical expressions.
 */
static int expr_evaluate_unary(struct eval_ctx *ctx, struct expr **expr)
{
	struct expr *unary = *expr, *arg = unary->arg;
	enum byteorder byteorder;

	/* unary expression arguments has already been evaluated. */

	assert(!expr_is_constant(arg));
	assert(expr_basetype(arg)->type == TYPE_INTEGER);
	assert(arg->etype != EXPR_UNARY);

	switch (unary->op) {
	case OP_HTON:
		assert(arg->byteorder == BYTEORDER_HOST_ENDIAN);
		byteorder = BYTEORDER_BIG_ENDIAN;
		break;
	case OP_NTOH:
		assert(arg->byteorder == BYTEORDER_BIG_ENDIAN);
		byteorder = BYTEORDER_HOST_ENDIAN;
		break;
	default:
		BUG("invalid unary operation %u", unary->op);
	}

	__datatype_set(unary, datatype_clone(arg->dtype));
	unary->byteorder = byteorder;
	unary->len	 = arg->len;
	return 0;
}

/*
 * Binops
 */
static int constant_binop_simplify(struct eval_ctx *ctx, struct expr **expr)
{
	struct expr *op = *expr, *left = (*expr)->left, *right = (*expr)->right;
	struct expr *new;
	mpz_t val, mask;

	assert(left->etype == EXPR_VALUE);
	assert(right->etype == EXPR_VALUE);
	assert(left->byteorder == right->byteorder);

	mpz_init2(val, op->len);
	mpz_init_bitmask(mask, op->len);

	switch (op->op) {
	case OP_AND:
		mpz_and(val, left->value, right->value);
		mpz_and(val, val, mask);
		break;
	case OP_XOR:
		mpz_xor(val, left->value, right->value);
		mpz_and(val, val, mask);
		break;
	case OP_OR:
		mpz_ior(val, left->value, right->value);
		mpz_and(val, val, mask);
		break;
	case OP_LSHIFT:
		assert(left->byteorder == BYTEORDER_HOST_ENDIAN);
		mpz_set(val, left->value);
		mpz_lshift_ui(val, mpz_get_uint32(right->value));
		mpz_and(val, val, mask);
		break;
	case OP_RSHIFT:
		assert(left->byteorder == BYTEORDER_HOST_ENDIAN);
		mpz_set(val, left->value);
		mpz_and(val, val, mask);
		mpz_rshift_ui(val, mpz_get_uint32(right->value));
		break;
	default:
		BUG("invalid binary operation %u", op->op);
	}

	new = constant_expr_alloc(&op->location, op->dtype, op->byteorder,
				  op->len, NULL);
	mpz_set(new->value, val);

	expr_free(*expr);
	*expr = new;

	mpz_clear(mask);
	mpz_clear(val);

	return expr_evaluate(ctx, expr);
}

static int expr_evaluate_shift(struct eval_ctx *ctx, struct expr **expr)
{
	struct expr *op = *expr, *left = op->left, *right = op->right;
	unsigned int shift, max_shift_len;

	/* mpz_get_uint32 has assert() for huge values */
	if (mpz_cmp_ui(right->value, UINT_MAX) > 0)
		return expr_binary_error(ctx->msgs, right, left,
					 "shifts exceeding %u bits are not supported", UINT_MAX);

	shift = mpz_get_uint32(right->value);
	if (ctx->stmt_len > left->len)
		max_shift_len = ctx->stmt_len;
	else
		max_shift_len = left->len;

	if (shift >= max_shift_len)
		return expr_binary_error(ctx->msgs, right, left,
					 "%s shift of %u bits is undefined for type of %u bits width",
					 op->op == OP_LSHIFT ? "Left" : "Right",
					 shift, max_shift_len);

	/* Both sides need to be in host byte order */
	if (byteorder_conversion(ctx, &op->left, BYTEORDER_HOST_ENDIAN) < 0)
		return -1;
	left = op->left;
	if (byteorder_conversion(ctx, &op->right, BYTEORDER_HOST_ENDIAN) < 0)
		return -1;

	datatype_set(op, &integer_type);
	op->byteorder = BYTEORDER_HOST_ENDIAN;
	op->len	      = max_shift_len;

	if (expr_is_constant(left))
		return constant_binop_simplify(ctx, expr);
	return 0;
}

static int expr_evaluate_bitwise(struct eval_ctx *ctx, struct expr **expr)
{
	struct expr *op = *expr, *left = op->left;
	const struct datatype *dtype;
	enum byteorder byteorder;
	unsigned int max_len;

	if (ctx->stmt_len > left->len) {
		max_len = ctx->stmt_len;
		byteorder = BYTEORDER_HOST_ENDIAN;
		dtype = &integer_type;

		/* Both sides need to be in host byte order */
		if (byteorder_conversion(ctx, &op->left, BYTEORDER_HOST_ENDIAN) < 0)
			return -1;

		left = op->left;
	} else {
		max_len = left->len;
		byteorder = left->byteorder;
		dtype = left->dtype;
	}

	if (byteorder_conversion(ctx, &op->right, byteorder) < 0)
		return -1;

	datatype_set(op, dtype);
	op->byteorder = byteorder;
	op->len	      = max_len;

	if (expr_is_constant(left) && expr_is_constant(op->right))
		return constant_binop_simplify(ctx, expr);
	return 0;
}

/*
 * Binop expression: both sides must be of integer base type. The left-hand side
 * may be either constant or non-constant; if it is constant, it must be a
 * singleton.  For bitwise operations, the right-hand side must be constant if
 * the left-hand side is constant; the right-hand side may be constant or
 * non-constant, if the left-hand side is non-constant; for shifts, the
 * right-hand side must be constant; if it is constant, it must be a singleton.
 */
static int expr_evaluate_binop(struct eval_ctx *ctx, struct expr **expr)
{
	struct expr *op = *expr, *left, *right;
	const char *sym = expr_op_symbols[op->op];
	unsigned int max_shift_len = ctx->ectx.len;
	int ret = -1;

	if (ctx->recursion.binop >= USHRT_MAX)
		return expr_binary_error(ctx->msgs, op, NULL,
					 "Binary operation limit %u reached ",
					 ctx->recursion.binop);
	ctx->recursion.binop++;

	if (expr_evaluate(ctx, &op->left) < 0)
		return -1;
	left = op->left;

	if (op->op == OP_LSHIFT || op->op == OP_RSHIFT) {
		if (left->len > max_shift_len)
			max_shift_len = left->len;

		__expr_set_context(&ctx->ectx, &integer_type,
				   left->byteorder, max_shift_len, 0);
	}

	if (expr_evaluate(ctx, &op->right) < 0)
		return -1;
	right = op->right;

	/* evaluation expects constant to the right hand side. */
	if (expr_is_constant(left) && !expr_is_constant(right)) {
		range_expr_swap_values(op);
		left = op->left;
		right = op->right;
	}

	switch (expr_basetype(left)->type) {
	case TYPE_INTEGER:
	case TYPE_STRING:
		break;
	default:
		return expr_binary_error(ctx->msgs, left, op,
					 "Binary operation (%s) is undefined "
					 "for %s types",
					 sym, left->dtype->desc);
	}

	if (expr_is_constant(left) && !expr_is_singleton(left))
		return expr_binary_error(ctx->msgs, left, op,
					 "Binary operation (%s) is undefined "
					 "for %s expressions",
					 sym, expr_name(left));

	if (!datatype_equal(expr_basetype(left), expr_basetype(right)))
		return expr_binary_error(ctx->msgs, left, op,
					 "Binary operation (%s) with different base types "
					 "(%s vs %s) is not supported",
					 sym, expr_basetype(left)->name, expr_basetype(right)->name);

	switch (op->op) {
	case OP_LSHIFT:
	case OP_RSHIFT:
		if (!expr_is_constant(right))
			return expr_binary_error(ctx->msgs, right, op,
						 "Right hand side of binary operation "
						 "(%s) must be constant", sym);

		if (!expr_is_singleton(right))
			return expr_binary_error(ctx->msgs, left, op,
						 "Binary operation (%s) is undefined "
						 "for %s expressions",
						 sym, expr_name(right));

		ret = expr_evaluate_shift(ctx, expr);
		break;
	case OP_AND:
	case OP_XOR:
	case OP_OR:
		if (expr_is_constant(left) && !expr_is_constant(right))
			return expr_binary_error(ctx->msgs, right, op,
						 "Right hand side of binary operation "
						 "(%s) must be constant", sym);

		if (expr_is_constant(right) && !expr_is_singleton(right))
			return expr_binary_error(ctx->msgs, left, op,
						 "Binary operation (%s) is undefined "
						 "for %s expressions",
						 sym, expr_name(right));

		ret = expr_evaluate_bitwise(ctx, expr);
		break;
	default:
		BUG("invalid binary operation %u", op->op);
	}


	if (ctx->recursion.binop == 0)
		BUG("recursion counter underflow");

	/* can't check earlier: evaluate functions might do constant-merging + expr_free.
	 *
	 * So once we've evaluate everything check for remaining length of the
	 * binop chain.
	 */
	if (--ctx->recursion.binop == 0) {
		unsigned int to_linearize = 0;

		op = *expr;
	        while (op && op->etype == EXPR_BINOP && op->left != NULL) {
			to_linearize++;
			op = op->left;

			if (to_linearize >= NFT_MAX_EXPR_RECURSION)
				return expr_binary_error(ctx->msgs, op, NULL,
							 "Binary operation limit %u reached ",
							 NFT_MAX_EXPR_RECURSION);
		}
	}

	return ret;
}

static int list_member_evaluate(struct eval_ctx *ctx, struct expr **expr)
{
	struct expr *next = list_entry((*expr)->list.next, struct expr, list);
	int err;

	/* should never be hit in practice */
	if (ctx->recursion.list >= USHRT_MAX)
		return expr_binary_error(ctx->msgs, next, NULL,
					 "List limit %u reached ",
					 ctx->recursion.list);

	ctx->recursion.list++;
	assert(*expr != next);
	list_del(&(*expr)->list);
	err = expr_evaluate(ctx, expr);
	list_add_tail(&(*expr)->list, &next->list);
	ctx->recursion.list--;
	return err;
}

static int expr_evaluate_concat(struct eval_ctx *ctx, struct expr **expr)
{
	const struct datatype *dtype = ctx->ectx.dtype, *tmp;
	uint32_t type = dtype ? dtype->type : 0, ntype = 0;
	int off = dtype ? dtype->subtypes : 0;
	unsigned int flags = EXPR_F_CONSTANT | EXPR_F_SINGLETON;
	const struct list_head *expressions = NULL;
	struct expr *i, *next, *key = NULL;
	const struct expr *key_ctx = NULL;
	bool runaway = false;
	uint32_t size = 0;

	if (ctx->ectx.key && ctx->ectx.key->etype == EXPR_CONCAT) {
		key_ctx = ctx->ectx.key;
		assert(!list_empty(&expr_concat(ctx->ectx.key)->expressions));
		key = list_first_entry(&expr_concat(ctx->ectx.key)->expressions, struct expr, list);
		expressions = &expr_concat(ctx->ectx.key)->expressions;
	}

	list_for_each_entry_safe(i, next, &expr_concat(*expr)->expressions, list) {
		enum byteorder bo = BYTEORDER_INVALID;
		unsigned dsize_bytes, dsize = 0;

		if (runaway) {
			return expr_binary_error(ctx->msgs, *expr, key_ctx,
						 "too many concatenation components");
		}

		if (i->etype == EXPR_CT &&
		    (i->ct.key == NFT_CT_SRC ||
		     i->ct.key == NFT_CT_DST))
			return expr_error(ctx->msgs, i,
					  "specify either ip or ip6 for address matching");

		if (expr_is_constant(*expr) && dtype && off == 0)
			return expr_binary_error(ctx->msgs, i, *expr,
						 "unexpected concat component, "
						 "expecting %s",
						 dtype->desc);

		if (key) {
			tmp = key->dtype;
			dsize = key->len;
			bo = key->byteorder;
			off--;
		} else if (dtype == NULL || off == 0) {
			tmp = datatype_lookup(TYPE_INVALID);
		} else {
			tmp = concat_subtype_lookup(type, --off);
			dsize = tmp->size;
			bo = tmp->byteorder;
		}

		__expr_set_context(&ctx->ectx, tmp, bo, dsize, 0);
		ctx->ectx.key = i;

		if (list_member_evaluate(ctx, &i) < 0)
			return -1;

		switch (i->etype) {
		case EXPR_INVALID:
		case __EXPR_MAX:
			BUG("Unexpected etype %d", i->etype);
			break;
		case EXPR_VALUE:
		case EXPR_UNARY:
		case EXPR_BINOP:
		case EXPR_RELATIONAL:
		case EXPR_CONCAT:
		case EXPR_MAP:
		case EXPR_PAYLOAD:
		case EXPR_EXTHDR:
		case EXPR_META:
		case EXPR_RT:
		case EXPR_CT:
		case EXPR_SET_ELEM:
		case EXPR_NUMGEN:
		case EXPR_HASH:
		case EXPR_FIB:
		case EXPR_SOCKET:
		case EXPR_OSF:
		case EXPR_XFRM:
		case EXPR_TUNNEL:
			break;
		case EXPR_RANGE:
		case EXPR_PREFIX:
		case EXPR_RANGE_VALUE:
			/* allowed on RHS (e.g. th dport . mark { 1-65535 . 42 }
			 *                                       ~~~~~~~~ allowed
			 * but not on LHS (e.g  1-4 . mark { ...}
			 *                      ~~~ illegal
			 *
			 * recursion.list > 0 means that the concatenation is
			 * part of another expression, such as EXPR_MAPPING or
			 * EXPR_SET_ELEM (is used as RHS).
			 */
			if (ctx->recursion.list > 0)
				break;

			return expr_error(ctx->msgs, i,
					  "cannot use %s in concatenation",
					  expr_name(i));
		case EXPR_VERDICT:
		case EXPR_SYMBOL:
		case EXPR_VARIABLE:
		case EXPR_LIST:
		case EXPR_SET:
		case EXPR_SET_REF:
		case EXPR_MAPPING:
		case EXPR_SET_ELEM_CATCHALL:
		case EXPR_RANGE_SYMBOL:
			return expr_error(ctx->msgs, i,
					  "cannot use %s in concatenation",
					  expr_name(i));
		}

		if (!i->dtype)
			return expr_error(ctx->msgs, i,
					  "cannot use %s in concatenation, lacks datatype",
					  expr_name(i));

		flags &= i->flags;

		if (!key && i->dtype->type == TYPE_INTEGER) {
			struct datatype *clone;

			clone = datatype_clone(i->dtype);
			clone->size = i->len;
			clone->byteorder = i->byteorder;
			__datatype_set(i, clone);
		}

		if (dtype == NULL && i->dtype->size == 0)
			return expr_binary_error(ctx->msgs, i, *expr,
						 "can not use variable sized "
						 "data types (%s) in concat "
						 "expressions",
						 i->dtype->name);
		if (dsize == 0) /* reload after evaluation or clone above */
			dsize = i->dtype->size;

		ntype = concat_subtype_add(ntype, i->dtype->type);

		dsize_bytes = div_round_up(dsize, BITS_PER_BYTE);
		expr_concat(*expr)->field_len[expr_concat(*expr)->field_count++] = dsize_bytes;
		size += netlink_padded_len(dsize);
		if (key && expressions) {
			if (list_is_last(&key->list, expressions))
				runaway = true;
			else
				key = list_next_entry(key, list);
		}

		ctx->inner_desc = NULL;

		if (size > NFT_MAX_EXPR_LEN_BITS)
			return expr_error(ctx->msgs, i, "Concatenation of size %u exceeds maximum size of %lu",
					  size, NFT_MAX_EXPR_LEN_BITS);
	}

	(*expr)->flags |= flags;
	__datatype_set(*expr, concat_type_alloc(ntype));
	(*expr)->len   = size;

	if (off > 0)
		return expr_error(ctx->msgs, *expr,
				  "datatype mismatch, expected %s, "
				  "expression has type %s",
				  dtype->desc, (*expr)->dtype->desc);

	expr_set_context(&ctx->ectx, (*expr)->dtype, (*expr)->len);
	if (!key_ctx)
		ctx->ectx.key = *expr;
	else
		ctx->ectx.key = key_ctx;

	return 0;
}

static int expr_evaluate_list(struct eval_ctx *ctx, struct expr **expr)
{
	struct expr *list = *expr, *new, *i, *next;
	mpz_t val;

	mpz_init_set_ui(val, 0);
	list_for_each_entry_safe(i, next, &expr_list(list)->expressions, list) {
		if (list_member_evaluate(ctx, &i) < 0) {
			mpz_clear(val);
			return -1;
		}
		if (i->etype != EXPR_VALUE) {
			mpz_clear(val);
			return expr_error(ctx->msgs, i,
					  "List member must be a constant "
					  "value");
		}
		if (datatype_basetype(i->dtype)->type != TYPE_BITMASK) {
			mpz_clear(val);
			return expr_error(ctx->msgs, i,
					  "Basetype of type %s is not bitmask",
					  i->dtype->desc);
		}
		mpz_ior(val, val, i->value);
	}

	new = constant_expr_alloc(&list->location, ctx->ectx.dtype,
				  BYTEORDER_HOST_ENDIAN, ctx->ectx.len, NULL);
	mpz_set(new->value, val);
	mpz_clear(val);

	expr_free(*expr);
	*expr = new;
	return 0;
}

static int __expr_evaluate_set_elem(struct eval_ctx *ctx, struct expr *elem)
{
	int num_elem_exprs = 0, num_set_exprs = 0;
	struct set *set = ctx->set;
	struct stmt *stmt;

	list_for_each_entry(stmt, &elem->stmt_list, list)
		num_elem_exprs++;
	list_for_each_entry(stmt, &set->stmt_list, list)
		num_set_exprs++;

	if (num_elem_exprs > 0) {
		struct stmt *set_stmt, *elem_stmt;

		if (num_set_exprs > 0 && num_elem_exprs != num_set_exprs) {
			return expr_error(ctx->msgs, elem,
					  "number of statements mismatch, set expects %d "
					  "but element has %d", num_set_exprs,
					  num_elem_exprs);
		} else if (num_set_exprs == 0) {
			if (!(set->flags & NFT_SET_ANONYMOUS) &&
			    !(set->flags & NFT_SET_EVAL)) {
				elem_stmt = list_first_entry(&elem->stmt_list, struct stmt, list);
				return stmt_error(ctx, elem_stmt,
						  "missing statement in %s declaration",
						  set_is_map(set->flags) ? "map" : "set");
			}
			return 0;
		}

		set_stmt = list_first_entry(&set->stmt_list, struct stmt, list);

		list_for_each_entry(elem_stmt, &elem->stmt_list, list) {
			if (set_stmt->type != elem_stmt->type) {
				return stmt_error(ctx, elem_stmt,
						  "statement mismatch, element expects %s, "
						  "but %s has type %s",
						  stmt_name(elem_stmt),
						  set_is_map(set->flags) ? "map" : "set",
						  stmt_name(set_stmt));
			}
			set_stmt = list_next_entry(set_stmt, list);
		}
	}

	return 0;
}

static bool datatype_compatible(const struct datatype *a, const struct datatype *b)
{
	return (a->type == TYPE_MARK &&
		datatype_equal(datatype_basetype(a), datatype_basetype(b))) ||
		datatype_equal(a, b);
}

static bool elem_key_compatible(const struct expr *set_key,
				const struct expr *elem_key)
{
	/* Catchall element is always compatible with the set key declaration */
	if (expr_type_catchall(elem_key))
		return true;

	return datatype_compatible(set_key->dtype, elem_key->dtype);
}

static int expr_evaluate_set_elem(struct eval_ctx *ctx, struct expr **expr)
{
	struct expr *elem = *expr;
	const struct expr *key;

	if (ctx->set) {
		if (__expr_evaluate_set_elem(ctx, elem) < 0)
			return -1;

		key = ctx->set->key;
		__expr_set_context(&ctx->ectx, key->dtype, key->byteorder, key->len, 0);
		ctx->ectx.key = key;
	}

	if (expr_evaluate(ctx, &elem->key) < 0)
		return -1;

	if (ctx->set &&
	    !(ctx->set->flags & (NFT_SET_ANONYMOUS | NFT_SET_INTERVAL))) {
		switch (elem->key->etype) {
		case EXPR_PREFIX:
		case EXPR_RANGE:
		case EXPR_RANGE_VALUE:
			key = elem->key;
			goto err_missing_flag;
		case EXPR_CONCAT:
			list_for_each_entry(key, &expr_concat(elem->key)->expressions, list) {
				switch (key->etype) {
				case EXPR_PREFIX:
				case EXPR_RANGE:
				case EXPR_RANGE_VALUE:
					goto err_missing_flag;
				default:
					break;
				}
			}
			break;
		default:
			break;
		}
	}

	if (ctx->set && !elem_key_compatible(ctx->set->key, elem->key))
		return expr_error(ctx->msgs, elem,
				  "Element mismatches %s definition, expected %s, not '%s'",
				  set_is_map(ctx->set->flags) ? "map" : "set",
				  ctx->set->key->dtype->desc, elem->key->dtype->desc);

	datatype_set(elem, elem->key->dtype);
	elem->len   = elem->key->len;
	elem->flags = elem->key->flags;

	return 0;

err_missing_flag:
	return expr_error(ctx->msgs, key,
			  "You must add 'flags interval' to your %s declaration if you want to add %s elements",
			  set_is_map(ctx->set->flags) ? "map" : "set", expr_name(key));
}

static int expr_evaluate_set_elem_catchall(struct eval_ctx *ctx, struct expr **expr)
{
	struct expr *elem = *expr;

	if (ctx->set)
		elem->len = ctx->set->key->len;

	return 0;
}

static const struct expr *expr_set_elem(const struct expr *expr)
{
	if (expr->etype == EXPR_MAPPING)
		return expr->left;

	return expr;
}

static int interval_set_eval(struct eval_ctx *ctx, struct set *set,
			     struct expr *init)
{
	int ret;

	if (!init)
		return 0;

	ret = 0;
	switch (ctx->cmd->op) {
	case CMD_CREATE:
	case CMD_ADD:
	case CMD_REPLACE:
	case CMD_INSERT:
		if (set->automerge) {
			ret = set_automerge(ctx->msgs, ctx->cmd, set, init,
					    ctx->nft->debug_mask);
		} else {
			ret = set_overlap(ctx->msgs, set, init);
		}
		break;
	case CMD_DELETE:
	case CMD_DESTROY:
		ret = set_delete(ctx->msgs, ctx->cmd, set, init,
				 ctx->nft->debug_mask);
		break;
	case CMD_GET:
	case CMD_RESET:
		break;
	default:
		BUG("unhandled op %d", ctx->cmd->op);
		break;
	}

	return ret;
}

static void expr_evaluate_set_ref(struct eval_ctx *ctx, struct expr *expr)
{
	struct set *set = expr->set;

	expr_set_context(&ctx->ectx, set->key->dtype, set->key->len);
	ctx->ectx.key = set->key;
}

static int expr_evaluate_set(struct eval_ctx *ctx, struct expr **expr)
{
	struct expr *set = *expr, *i, *next;
	const struct expr *elem;

	list_for_each_entry_safe(i, next, &expr_set(set)->expressions, list) {
		if (list_member_evaluate(ctx, &i) < 0)
			return -1;

		if (i->etype == EXPR_MAPPING &&
		    i->left->etype == EXPR_SET_ELEM &&
		    i->left->key->etype == EXPR_SET) {
			struct expr *new, *j;

			list_for_each_entry(j, &expr_set(i->left->key)->expressions, list) {
				new = mapping_expr_alloc(&i->location,
							 expr_get(j),
							 expr_get(i->right));
				list_add_tail(&new->list, &expr_set(set)->expressions);
				expr_set(set)->size++;
			}
			list_del(&i->list);
			expr_free(i);
			continue;
		}

		elem = expr_set_elem(i);

		if (elem->etype == EXPR_SET_ELEM &&
		    elem->key->etype == EXPR_SET_REF)
			return expr_error(ctx->msgs, i,
					  "Set reference cannot be part of another set");

		if (elem->etype == EXPR_SET_ELEM &&
		    elem->key->etype == EXPR_SET) {
			struct expr *new = expr_get(elem->key);

			expr_set(set)->set_flags |= expr_set(elem->key)->set_flags;
			list_replace(&i->list, &new->list);
			expr_free(i);
			i = new;
			elem = expr_set_elem(i);
		}

		if (!expr_is_constant(i))
			return expr_error(ctx->msgs, i,
					  "Set member is not constant");

		if (i->etype == EXPR_SET) {
			/* Merge recursive set definitions */
			list_splice_tail_init(&expr_set(i)->expressions, &i->list);
			list_del(&i->list);
			expr_set(set)->size      += expr_set(i)->size - 1;
			expr_set(set)->set_flags |= expr_set(i)->set_flags;
			expr_free(i);
		} else if (!expr_is_singleton(i)) {
			expr_set(set)->set_flags |= NFT_SET_INTERVAL;
			if (elem->key->etype == EXPR_CONCAT)
				expr_set(set)->set_flags |= NFT_SET_CONCAT;
		}
	}

	if (ctx->set) {
		if (ctx->set->flags & NFT_SET_CONCAT)
			expr_set(set)->set_flags |= NFT_SET_CONCAT;
	}

	expr_set(set)->set_flags |= NFT_SET_CONSTANT;

	datatype_set(set, ctx->ectx.dtype);
	set->len   = ctx->ectx.len;
	set->flags |= EXPR_F_CONSTANT;

	return 0;
}

static int binop_transfer(struct eval_ctx *ctx, struct expr **expr);

static void map_set_concat_info(struct expr *map)
{
	map->mappings->set->flags |= expr_set(map->mappings->set->init)->set_flags;

	if (map->mappings->set->flags & NFT_SET_INTERVAL &&
	    map->map->etype == EXPR_CONCAT) {
		memcpy(&map->mappings->set->desc.field_len,
		       &expr_concat(map->map)->field_len,
		       sizeof(map->mappings->set->desc.field_len));
		map->mappings->set->desc.field_count =
			expr_concat(map->map)->field_count;
		map->mappings->flags |= NFT_SET_CONCAT;
	}
}

static void __mapping_expr_expand(struct expr *i)
{
	struct expr *j, *range, *next;

	assert(i->etype == EXPR_MAPPING);
	switch (i->right->etype) {
	case EXPR_VALUE:
		range = range_expr_alloc(&i->location, expr_get(i->right), expr_get(i->right));
		expr_free(i->right);
		i->right = range;
		break;
	case EXPR_CONCAT:
		list_for_each_entry_safe(j, next, &expr_concat(i->right)->expressions, list) {
			if (j->etype != EXPR_VALUE)
				continue;

			range = range_expr_alloc(&j->location, expr_get(j), expr_get(j));
			list_replace(&j->list, &range->list);
			expr_free(j);
		}
		i->right->flags &= ~EXPR_F_SINGLETON;
		break;
	default:
		break;
	}
}

static int mapping_expr_expand(struct eval_ctx *ctx)
{
	struct expr *i;

	if (!set_is_anonymous(ctx->set->flags))
		return 0;

	list_for_each_entry(i, &expr_set(ctx->set->init)->expressions, list) {
		if (i->etype != EXPR_MAPPING)
			return expr_error(ctx->msgs, i,
					  "expected mapping, not %s", expr_name(i));
		__mapping_expr_expand(i);
	}

	return 0;
}

static int expr_evaluate_map(struct eval_ctx *ctx, struct expr **expr)
{
	struct expr *map = *expr, *mappings;
	struct expr_ctx ectx = ctx->ectx;
	uint32_t set_flags = NFT_SET_MAP;
	struct expr *key, *data;

	if (map->map->etype == EXPR_CT &&
	    (map->map->ct.key == NFT_CT_SRC ||
	     map->map->ct.key == NFT_CT_DST))
		return expr_error(ctx->msgs, map->map,
				  "specify either ip or ip6 for address matching");
	else if (map->map->etype == EXPR_CONCAT) {
		struct expr *i;

		list_for_each_entry(i, &expr_concat(map->map)->expressions, list) {
			if (i->etype == EXPR_CT &&
			    (i->ct.key == NFT_CT_SRC ||
			     i->ct.key == NFT_CT_DST))
				return expr_error(ctx->msgs, i,
					  "specify either ip or ip6 for address matching");
		}
	}

	expr_set_context(&ctx->ectx, NULL, 0);
	if (expr_evaluate(ctx, &map->map) < 0)
		return -1;
	if (expr_is_constant(map->map))
		return expr_error(ctx->msgs, map->map,
				  "Map expression can not be constant");

	ctx->stmt_len = 0;
	mappings = map->mappings;

	switch (map->mappings->etype) {
	case EXPR_SET:
		set_flags |= expr_set(mappings)->set_flags;
		/* fallthrough */
	case EXPR_VARIABLE:
		if (ctx->ectx.key && ctx->ectx.key->etype == EXPR_CONCAT) {
			key = expr_clone(ctx->ectx.key);
		} else {
			key = constant_expr_alloc(&map->location,
						  ctx->ectx.dtype,
						  ctx->ectx.byteorder,
						  ctx->ectx.len, NULL);
		}

		if (!ectx.dtype) {
			expr_free(key);
			return expr_error(ctx->msgs, map,
					  "Implicit map expression without known datatype");
		}

		if (ectx.dtype->type == TYPE_VERDICT) {
			data = verdict_expr_alloc(&netlink_location, 0, NULL);
		} else {
			const struct datatype *dtype;

			dtype = set_datatype_alloc(ectx.dtype, ectx.byteorder);
			data = constant_expr_alloc(&netlink_location, dtype,
						   dtype->byteorder, ectx.len, NULL);
			datatype_free(dtype);
		}

		mappings = implicit_set_declaration(ctx, "__map%d",
						    key, data,
						    mappings,
						    NFT_SET_ANONYMOUS | set_flags);
		if (!mappings)
			return -1;

		if (ectx.len && mappings->set->data->len != ectx.len)
			BUG("%d vs %d", mappings->set->data->len, ectx.len);

		map->mappings = mappings;

		ctx->set = mappings->set;
		if (expr_evaluate(ctx, &map->mappings->set->init) < 0)
			return -1;

		if (map->mappings->set->init->etype != EXPR_SET) {
			return expr_error(ctx->msgs, map->mappings->set->init,
					  "Expression is not a map");
		}

		if (set_is_interval(expr_set(map->mappings->set->init)->set_flags) &&
		    !(expr_set(map->mappings->set->init)->set_flags & NFT_SET_CONCAT) &&
		    interval_set_eval(ctx, ctx->set, map->mappings->set->init) < 0)
			return -1;

		expr_set_context(&ctx->ectx, ctx->set->key->dtype, ctx->set->key->len);
		if (binop_transfer(ctx, expr) < 0)
			return -1;

		if (ctx->set->data->flags & EXPR_F_INTERVAL) {
			ctx->set->data->len *= 2;

			if (mapping_expr_expand(ctx))
				return -1;
		}

		ctx->set->key->len = ctx->ectx.len;
		ctx->set = NULL;
		map = *expr;

		map_set_concat_info(map);
		break;
	case EXPR_SYMBOL:
		if (expr_evaluate(ctx, &map->mappings) < 0)
			return -1;
		if (map->mappings->etype != EXPR_SET_REF ||
		    !set_is_datamap(map->mappings->set->flags))
			return expr_error(ctx->msgs, map->mappings,
					  "Expression is not a map");
		break;
	case EXPR_SET_REF:
		/* symbol has been already evaluated to set reference */
		if (!set_is_map(mappings->set->flags))
			return expr_error(ctx->msgs, map->mappings,
					  "Expression is not a map");
		break;
	default:
		return expr_binary_error(ctx->msgs, map->mappings, map->map,
					 "invalid mapping expression %s", expr_name(map->mappings));
	}

	if (!datatype_compatible(map->mappings->set->key->dtype, map->map->dtype))
		return expr_binary_error(ctx->msgs, map->mappings, map->map,
					 "datatype mismatch, map expects %s, "
					 "mapping expression has type %s",
					 map->mappings->set->key->dtype->desc,
					 map->map->dtype->desc);

	datatype_set(map, map->mappings->set->data->dtype);
	map->flags |= EXPR_F_CONSTANT;

	/* Data for range lookups needs to be in big endian order */
	if (map->mappings->set->flags & NFT_SET_INTERVAL &&
	    byteorder_conversion(ctx, &map->map, BYTEORDER_BIG_ENDIAN) < 0)
		return -1;

	return 0;
}

static bool data_mapping_has_interval(struct expr *data)
{
	struct expr *i;

	if (data->etype == EXPR_RANGE ||
	    data->etype == EXPR_RANGE_VALUE ||
	    data->etype == EXPR_PREFIX)
		return true;

	if (data->etype != EXPR_CONCAT)
		return false;

	list_for_each_entry(i, &expr_concat(data)->expressions, list) {
		if (i->etype == EXPR_RANGE ||
		    i->etype == EXPR_RANGE_VALUE ||
		    i->etype == EXPR_PREFIX)
			return true;
	}

	return false;
}

static bool elem_data_compatible(const struct expr *set_data,
				const struct expr *elem_data)
{
	if (elem_data->etype == EXPR_RANGE) {
		/* EXPR_RANGE has invalid_type, use the lhs type.
		 * It should be impossible to have a EXPR_RANGE where
		 * lhs and rhs don't have the same dtype.
		 */
		return elem_data_compatible(set_data, elem_data->left);
	}

	return datatype_compatible(set_data->dtype, elem_data->dtype);
}

static int expr_evaluate_mapping(struct eval_ctx *ctx, struct expr **expr)
{
	struct expr *mapping = *expr;
	struct set *set = ctx->set;
	uint32_t datalen;

	if (set == NULL)
		return expr_error(ctx->msgs, mapping,
				  "mapping outside of map context");
	if (!set_is_map(set->flags))
		return set_error(ctx, set, "set is not a map");

	expr_set_context(&ctx->ectx, set->key->dtype, set->key->len);
	if (expr_evaluate(ctx, &mapping->left) < 0)
		return -1;
	if (!expr_is_constant(mapping->left))
		return expr_error(ctx->msgs, mapping->left,
				  "Key must be a constant");
	mapping->flags |= mapping->left->flags & EXPR_F_SINGLETON;

	/* This can happen for malformed map definitions */
	if (!set->data)
		return set_error(ctx, set, "map has no mapping data");

	if (!set_is_anonymous(set->flags) &&
	    set->data->flags & EXPR_F_INTERVAL)
		datalen = set->data->len / 2;
	else
		datalen = set->data->len;
	__expr_set_context(&ctx->ectx, set->data->dtype,
			   set->data->byteorder, datalen, 0);

	if (expr_evaluate(ctx, &mapping->right) < 0)
		return -1;
	if (!expr_is_constant(mapping->right))
		return expr_error(ctx->msgs, mapping->right,
				  "Value must be a constant");

	if (set_is_anonymous(set->flags) &&
	    data_mapping_has_interval(mapping->right))
		set->data->flags |= EXPR_F_INTERVAL;

	if (!set_is_anonymous(set->flags) &&
	    set->data->flags & EXPR_F_INTERVAL)
		__mapping_expr_expand(mapping);

	if (!(set->data->flags & EXPR_F_INTERVAL) &&
	    !expr_is_singleton(mapping->right))
		return expr_error(ctx->msgs, mapping->right,
				  "Value must be a singleton");

	if (set_is_objmap(set->flags) && mapping->right->etype != EXPR_VALUE)
		return expr_error(ctx->msgs, mapping->right,
				  "Object mapping data should be a value, not %s",
				  expr_name(mapping->right));

	if (set_is_datamap(set->flags) &&
	    !elem_data_compatible(set->data, mapping->right))
		return expr_error(ctx->msgs, mapping->right,
				  "Element mapping mismatches map definition, expected %s, not '%s'",
				  set->data->dtype->desc, mapping->right->dtype->desc);

	mapping->flags |= EXPR_F_CONSTANT;
	return 0;
}

static struct expr *symbol_range_expand(struct expr *expr)
{
	struct expr *left, *right;

	/* expand to symbol and range expressions to consolidate evaluation. */
	left = symbol_expr_alloc(&expr->location, expr->symtype,
				 (struct scope *)expr->scope,
				 expr->identifier_range[0]);
	right = symbol_expr_alloc(&expr->location, expr->symtype,
				  (struct scope *)expr->scope,
				  expr->identifier_range[1]);
	return range_expr_alloc(&expr->location, left, right);
}

static int expr_evaluate_symbol_range(struct eval_ctx *ctx, struct expr **exprp)
{
	struct expr *left, *right, *range, *constant_range;
	struct expr *expr = *exprp;

	/* expand to symbol and range expressions to consolidate evaluation. */
	range = symbol_range_expand(expr);

	if (expr_evaluate(ctx, &range) < 0) {
		expr_free(range);
		return -1;
	}

	if (range->etype != EXPR_RANGE)
		goto out_done;

	left = range->left;
	right = range->right;

	if (ctx->set &&
	    left->etype == EXPR_VALUE &&
	    right->etype == EXPR_VALUE) {
		constant_range = constant_range_expr_alloc(&expr->location,
							   left->dtype,
							   left->byteorder,
							   left->len,
							   left->value,
							   right->value);
		expr_free(range);
		expr_free(expr);
		*exprp = constant_range;
		return 0;
	}

out_done:
	expr_free(expr);
	*exprp = range;

	return 0;
}

/* We got datatype context via statement. If the basetype is compatible, set
 * this expression datatype to the one of the statement to make it datatype
 * compatible. This is a more conservative approach than enabling datatype
 * compatibility between two different datatypes whose basetype is the same,
 * let's revisit this later once users come with valid usecases to generalize
 * this.
 */
static void expr_dtype_integer_compatible(struct eval_ctx *ctx,
					  struct expr *expr)
{
	if (ctx->ectx.dtype &&
	    ctx->ectx.dtype->basetype == &integer_type &&
	    ctx->ectx.len == 4 * BITS_PER_BYTE) {
		datatype_set(expr, ctx->ectx.dtype);
		expr->len   = ctx->ectx.len;
	}
}

static int expr_evaluate_numgen(struct eval_ctx *ctx, struct expr **exprp)
{
	struct expr *expr = *exprp;
	unsigned int maxval;

	expr_dtype_integer_compatible(ctx, expr);

	maxval = expr->numgen.mod + expr->numgen.offset - 1;
	__expr_set_context(&ctx->ectx, expr->dtype, expr->byteorder, expr->len,
			   maxval);
	return 0;
}

static int expr_evaluate_hash(struct eval_ctx *ctx, struct expr **exprp)
{
	struct expr *expr = *exprp;
	unsigned int maxval;

	expr_dtype_integer_compatible(ctx, expr);

	expr_set_context(&ctx->ectx, NULL, 0);
	if (expr->hash.expr &&
	    expr_evaluate(ctx, &expr->hash.expr) < 0)
		return -1;

	/* expr_evaluate_primary() sets the context to what to the input
         * expression to be hashed. Since this input is transformed to a 4 bytes
	 * integer, restore context to the datatype that results from hashing.
	 */
	maxval = expr->hash.mod + expr->hash.offset - 1;
	__expr_set_context(&ctx->ectx, expr->dtype, expr->byteorder, expr->len,
			   maxval);

	return 0;
}

/*
 * Transfer the invertible binops to the constant side of an equality
 * expression. A left shift is only invertible if the low n bits are
 * zero.
 */
static int binop_can_transfer(struct eval_ctx *ctx,
			      struct expr *left, struct expr *right)
{
	int err;

	switch (right->etype) {
	case EXPR_VALUE:
		break;
	case EXPR_SET_ELEM:
		return binop_can_transfer(ctx, left, right->key);
	case EXPR_RANGE:
		err = binop_can_transfer(ctx, left, right->left);
		if (err <= 0)
			return err;
		return binop_can_transfer(ctx, left, right->right);
	case EXPR_MAPPING:
		return binop_can_transfer(ctx, left, right->left);
	default:
		return 0;
	}

	switch (left->op) {
	case OP_LSHIFT:
		assert(left->right->etype == EXPR_VALUE);
		assert(right->etype == EXPR_VALUE);

		if (mpz_scan1(right->value, 0) < mpz_get_uint32(left->right->value))
			return expr_binary_error(ctx->msgs, right, left,
						 "Comparison is always false");
		return 1;
	case OP_RSHIFT:
		assert(left->right->etype == EXPR_VALUE);
		if (ctx->ectx.len < right->len + mpz_get_uint32(left->right->value))
			ctx->ectx.len += mpz_get_uint32(left->right->value);
		return 1;
	case OP_XOR:
		return expr_is_constant(left->right);
	default:
		return 0;
	}
}

static int binop_transfer_one(struct eval_ctx *ctx,
			      const struct expr *left, struct expr **right)
{
	int err;

	switch ((*right)->etype) {
	case EXPR_MAPPING:
		return binop_transfer_one(ctx, left, &(*right)->left);
	case EXPR_VALUE:
		break;
	case EXPR_SET_ELEM:
		return binop_transfer_one(ctx, left, &(*right)->key);
	case EXPR_RANGE:
		err = binop_transfer_one(ctx, left, &(*right)->left);
		if (err < 0)
			return err;
		return binop_transfer_one(ctx, left, &(*right)->right);
	default:
		return 0;
	}

	switch (left->op) {
	case OP_LSHIFT:
		(*right) = binop_expr_alloc(&(*right)->location, OP_RSHIFT,
					    *right, expr_get(left->right));
		break;
	case OP_RSHIFT:
		(*right) = binop_expr_alloc(&(*right)->location, OP_LSHIFT,
					    *right, expr_get(left->right));
		break;
	case OP_XOR:
		(*right) = binop_expr_alloc(&(*right)->location, OP_XOR,
					    *right, expr_get(left->right));
		break;
	default:
		BUG("invalid binary operation %u", left->op);
	}

	return expr_evaluate(ctx, right);
}

static void binop_transfer_handle_lhs(struct expr **expr)
{
	struct expr *tmp, *left = *expr;
	unsigned int shift;

	assert(left->etype == EXPR_BINOP);

	switch (left->op) {
	case OP_RSHIFT:
		/* Mask out the bits the shift would have masked out */
		shift = mpz_get_uint8(left->right->value);
		mpz_bitmask(left->right->value, left->left->len);
		mpz_lshift_ui(left->right->value, shift);
		left->op = OP_AND;
		break;
	case OP_LSHIFT:
	case OP_XOR:
		tmp = expr_get(left->left);
		datatype_set(tmp, left->dtype);
		expr_free(left);
		*expr = tmp;
		break;
	default:
		BUG("invalid binop operation %u", left->op);
	}
}

static int __binop_transfer(struct eval_ctx *ctx,
			    struct expr *left, struct expr **right)
{
	struct expr *i, *next;
	int err;

	assert(left->etype == EXPR_BINOP);

	switch ((*right)->etype) {
	case EXPR_VALUE:
		err = binop_can_transfer(ctx, left, *right);
		if (err <= 0)
			return err;
		if (binop_transfer_one(ctx, left, right) < 0)
			return -1;
		break;
	case EXPR_RANGE:
		err = binop_can_transfer(ctx, left, *right);
		if (err <= 0)
			return err;
		if (binop_transfer_one(ctx, left, right) < 0)
			return -1;
		break;
	case EXPR_SET:
		list_for_each_entry(i, &expr_set(*right)->expressions, list) {
			err = binop_can_transfer(ctx, left, i);
			if (err <= 0)
				return err;
		}
		list_for_each_entry_safe(i, next, &expr_set(*right)->expressions, list) {
			list_del(&i->list);
			err = binop_transfer_one(ctx, left, &i);
			list_add_tail(&i->list, &next->list);
			if (err < 0)
				return err;
		}
		break;
	case EXPR_SET_REF:
		if (!set_is_anonymous((*right)->set->flags))
			return 0;

		return __binop_transfer(ctx, left, &(*right)->set->init);
	default:
		return 0;
	}

	return 1;
}

static int binop_transfer(struct eval_ctx *ctx, struct expr **expr)
{
	struct expr *left = (*expr)->left;
	int ret;

	if (left->etype != EXPR_BINOP)
		return 0;

	ret = __binop_transfer(ctx, left, &(*expr)->right);
	if (ret <= 0)
		return ret;

	binop_transfer_handle_lhs(&(*expr)->left);
	return 0;
}

bool lhs_is_meta_hour(const struct expr *meta)
{
	if (meta->etype != EXPR_META)
		return false;

	return meta->meta.key == NFT_META_TIME_HOUR ||
	       meta->meta.key == NFT_META_TIME_DAY;
}

void range_expr_swap_values(struct expr *range)
{
	struct expr *left_tmp;

	left_tmp = range->left;
	range->left = range->right;
	range->right = left_tmp;
}

static bool range_needs_swap(const struct expr *range)
{
	const struct expr *right = range->right;
	const struct expr *left = range->left;

	return mpz_cmp(left->value, right->value) > 0;
}

static void optimize_singleton_set(struct expr *rel, struct expr **expr)
{
	struct expr *set = rel->right, *i;

	i = list_first_entry(&expr_set(set)->expressions, struct expr, list);
	if (i->etype == EXPR_SET_ELEM &&
	    list_empty(&i->stmt_list)) {

		switch (i->key->etype) {
		case EXPR_PREFIX:
		case EXPR_RANGE:
		case EXPR_VALUE:
			rel->right = *expr = i->key;
			i->key = NULL;
			expr_free(set);
			break;
		default:
			break;
		}
	}

	if (rel->op == OP_IMPLICIT &&
	    rel->right->dtype->basetype &&
	    rel->right->dtype->basetype->type == TYPE_BITMASK &&
	    rel->right->dtype->type != TYPE_CT_STATE) {
		rel->op = OP_EQ;
	}
}

static int expr_evaluate_relational(struct eval_ctx *ctx, struct expr **expr)
{
	struct expr *rel = *expr, *left, *right;
	struct proto_ctx *pctx;
	struct expr *range;
	int ret;

	right = rel->right;
	if (right->etype == EXPR_SYMBOL &&
	    right->symtype == SYMBOL_SET &&
	    expr_evaluate(ctx, &rel->right) < 0)
		return -1;

	if (expr_evaluate(ctx, &rel->left) < 0)
		return -1;
	left = rel->left;

	pctx = eval_proto_ctx(ctx);

	if (lhs_is_meta_hour(rel->left) &&
	    (rel->right->etype == EXPR_RANGE ||
	     rel->right->etype == EXPR_RANGE_SYMBOL)) {
		if (rel->right->etype == EXPR_RANGE_SYMBOL) {
			range = symbol_range_expand(rel->right);
			expr_free(rel->right);
			rel->right = range;
		}
		ret = __expr_evaluate_range(ctx, &rel->right);
		if (ret)
			return ret;

		range = rel->right;

		/*
		 * We may need to do this for proper cross-day ranges,
		 * e.g. meta hour 23:15-03:22
		 */
		if (range_needs_swap(range)) {
			if (ctx->nft->debug_mask & NFT_DEBUG_EVALUATION)
				nft_print(&ctx->nft->output,
					  "Inverting range values for cross-day hour matching\n\n");

			if (rel->op == OP_EQ || rel->op == OP_IMPLICIT) {
				range_expr_swap_values(range);
				rel->op = OP_NEQ;
			} else if (rel->op == OP_NEQ) {
				range_expr_swap_values(range);
				rel->op = OP_EQ;
			}
		}
	}

	if (expr_evaluate(ctx, &rel->right) < 0)
		return -1;
	right = rel->right;

	if (!expr_is_constant(right))
		return expr_binary_error(ctx->msgs, right, rel,
					 "Right hand side of relational "
					 "expression (%s) must be constant",
					 expr_op_symbols[rel->op]);
	if (expr_is_constant(left))
		return expr_binary_error(ctx->msgs, left, right,
					 "Relational expression (%s) has "
					 "constant value",
					 expr_op_symbols[rel->op]);

	if (!datatype_equal(left->dtype, right->dtype))
		return expr_binary_error(ctx->msgs, right, left,
					 "datatype mismatch, expected %s, "
					 "expression has type %s",
					 left->dtype->desc,
					 right->dtype->desc);

	/*
	 * Statements like 'ct secmark 12' are parsed as relational,
	 * disallow constant value on the right hand side.
	 */
	if (((left->etype == EXPR_META &&
	      left->meta.key == NFT_META_SECMARK) ||
	     (left->etype == EXPR_CT &&
	      left->ct.key == NFT_CT_SECMARK)) &&
	    right->flags & EXPR_F_CONSTANT)
		return expr_binary_error(ctx->msgs, right, left,
					 "Cannot be used with right hand side constant value");

	if (left->etype != EXPR_CONCAT) {
		switch (rel->op) {
		case OP_EQ:
		case OP_IMPLICIT:
		case OP_NEQ:
			if (right->etype == EXPR_SET && expr_set(right)->size == 1)
				optimize_singleton_set(rel, &right);
			break;
		default:
			break;
		}
	}

	switch (rel->op) {
	case OP_EQ:
	case OP_IMPLICIT:
		/*
		 * Update protocol context for payload and meta iiftype
		 * equality expressions.
		 */
		relational_expr_pctx_update(pctx, rel);

		/* fall through */
	case OP_NEQ:
	case OP_NEG:
		if (rel->op == OP_NEG) {
			if (left->etype == EXPR_BINOP)
				return expr_binary_error(ctx->msgs, left, right,
							 "cannot combine negation with binary expression");
			if (right->etype != EXPR_VALUE ||
			    right->dtype->basetype == NULL ||
			    right->dtype->basetype->type != TYPE_BITMASK)
				return expr_binary_error(ctx->msgs, left, right,
							 "negation can only be used with singleton bitmask values.  Did you mean \"!=\"?");
		}

		switch (right->etype) {
		case EXPR_RANGE:
			if (byteorder_conversion(ctx, &rel->left, BYTEORDER_BIG_ENDIAN) < 0)
				return -1;
			if (byteorder_conversion(ctx, &right->left, BYTEORDER_BIG_ENDIAN) < 0)
				return -1;
			if (byteorder_conversion(ctx, &right->right, BYTEORDER_BIG_ENDIAN) < 0)
				return -1;
			break;
		case EXPR_PREFIX:
			if (byteorder_conversion(ctx, &right->prefix, left->byteorder) < 0)
				return -1;
			break;
		case EXPR_VALUE:
			if (byteorder_conversion(ctx, &rel->right, left->byteorder) < 0)
				return -1;
			break;
		case EXPR_SET:
			if (expr_set(right)->size == 0)
				return expr_error(ctx->msgs, right, "Set is empty");

			right = rel->right =
				implicit_set_declaration(ctx, "__set%d",
							 expr_get(left), NULL,
							 right,
							 expr_set(right)->set_flags | NFT_SET_ANONYMOUS);
			if (!right)
				return -1;

			/* fall through */
		case EXPR_SET_REF:
			if (rel->left->etype == EXPR_CT &&
			    (rel->left->ct.key == NFT_CT_SRC ||
			     rel->left->ct.key == NFT_CT_DST))
				return expr_error(ctx->msgs, left,
						  "specify either ip or ip6 for address matching");

			/* Data for range lookups needs to be in big endian order */
			if (right->set->flags & NFT_SET_INTERVAL &&
			    byteorder_conversion(ctx, &rel->left, BYTEORDER_BIG_ENDIAN) < 0)
				return -1;
			break;
		case EXPR_CONCAT:
			return expr_binary_error(ctx->msgs, left, right,
						 "Use concatenations with sets and maps, not singleton values");
			break;
		default:
			BUG("invalid expression type %s", expr_name(right));
		}
		break;
	case OP_LT:
	case OP_GT:
	case OP_LTE:
	case OP_GTE:
		switch (left->etype) {
		case EXPR_CONCAT:
			return expr_binary_error(ctx->msgs, left, rel,
					"Relational expression (%s) is undefined "
				        "for %s expressions",
					expr_op_symbols[rel->op],
					expr_name(left));
		default:
			break;
		}

		if (!expr_is_singleton(right))
			return expr_binary_error(ctx->msgs, right, rel,
					"Relational expression (%s) is undefined "
				        "for %s expressions",
					expr_op_symbols[rel->op],
					expr_name(right));

		if (byteorder_conversion(ctx, &rel->left, BYTEORDER_BIG_ENDIAN) < 0)
			return -1;
		if (byteorder_conversion(ctx, &rel->right, BYTEORDER_BIG_ENDIAN) < 0)
			return -1;
		break;
	default:
		BUG("invalid relational operation %u", rel->op);
	}

	if (binop_transfer(ctx, expr) < 0)
		return -1;

	return 0;
}

static int expr_evaluate_fib(struct eval_ctx *ctx, struct expr **exprp)
{
	struct expr *expr = *exprp;

	if (expr->flags & EXPR_F_BOOLEAN) {
		expr->fib.flags |= NFTA_FIB_F_PRESENT;
		datatype_set(expr, &boolean_type);
		expr->len = BITS_PER_BYTE;
	}
	return expr_evaluate_primary(ctx, exprp);
}

static int expr_evaluate_meta(struct eval_ctx *ctx, struct expr **exprp)
{
	struct proto_ctx *pctx = eval_proto_ctx(ctx);
	struct expr *meta = *exprp;

	switch (meta->meta.key) {
	case NFT_META_NFPROTO:
		if (pctx->family != NFPROTO_INET &&
		    meta->flags & EXPR_F_PROTOCOL)
			return expr_error(ctx->msgs, meta,
					  "meta nfproto is only useful in the inet family");
		break;
	case NFT_META_TIME_DAY:
		__expr_set_context(&ctx->ectx, meta->dtype, meta->byteorder,
				   meta->len, 6);
		return 0;
	default:
		break;
	}

	return expr_evaluate_primary(ctx, exprp);
}

static int expr_evaluate_socket(struct eval_ctx *ctx, struct expr **expr)
{
	enum nft_socket_keys key = (*expr)->socket.key;
	int maxval = 0;

	if (key == NFT_SOCKET_TRANSPARENT ||
	    key == NFT_SOCKET_WILDCARD)
		maxval = 1;
	__expr_set_context(&ctx->ectx, (*expr)->dtype, (*expr)->byteorder,
			   (*expr)->len, maxval);
	return 0;
}

static int expr_evaluate_osf(struct eval_ctx *ctx, struct expr **expr)
{
	struct netlink_ctx nl_ctx = {
		.nft		= ctx->nft,
		.seqnum		= time(NULL),
	};

	nfnl_osf_load_fingerprints(&nl_ctx, 0);

	return expr_evaluate_primary(ctx, expr);
}

static int expr_evaluate_tunnel(struct eval_ctx *ctx, struct expr **exprp)
{
	return expr_evaluate_primary(ctx, exprp);
}

static int expr_evaluate_variable(struct eval_ctx *ctx, struct expr **exprp)
{
	struct symbol *sym = (*exprp)->sym;
	struct expr *new;

	/* If variable is reused from different locations in the ruleset, then
	 * clone expression.
	 */
	if (sym->refcnt > 2)
		new = expr_clone(sym->expr);
	else
		new = expr_get(sym->expr);

	if (expr_evaluate(ctx, &new) < 0) {
		expr_free(new);
		return -1;
	}

	expr_free(*exprp);
	*exprp = new;

	return 0;
}

static int expr_evaluate_xfrm(struct eval_ctx *ctx, struct expr **exprp)
{
	struct proto_ctx *pctx = eval_proto_ctx(ctx);
	struct expr *expr = *exprp;

	switch (pctx->family) {
	case NFPROTO_IPV4:
	case NFPROTO_IPV6:
	case NFPROTO_INET:
		break;
	default:
		return expr_error(ctx->msgs, expr, "ipsec expression is only"
				  " valid in ip/ip6/inet tables");
	}

	return expr_evaluate_primary(ctx, exprp);
}

static int verdict_validate_chain(struct eval_ctx *ctx,
				     struct expr *chain)
{
	char buf[NFT_CHAIN_MAXNAMELEN];
	unsigned int len;

	len = chain->len / BITS_PER_BYTE;
	if (len > NFT_CHAIN_MAXNAMELEN)
		return expr_error(ctx->msgs, chain,
				  "chain name too long (%u, max %u)",
				  chain->len / BITS_PER_BYTE,
				  NFT_CHAIN_MAXNAMELEN);

	if (!len)
		return expr_error(ctx->msgs, chain,
				  "chain name length 0 not allowed");

	memset(buf, 0, sizeof(buf));
	mpz_export_data(buf, chain->value, BYTEORDER_HOST_ENDIAN, len);

	if (strnlen(buf, sizeof(buf)) < sizeof(buf))
		return 0;

	return expr_error(ctx->msgs, chain,
			  "chain name must be smaller than %u",
			  NFT_CHAIN_MAXNAMELEN);
}

static int expr_evaluate_verdict(struct eval_ctx *ctx, struct expr **exprp)
{
	struct expr *expr = *exprp;

	switch (expr->verdict) {
	case NFT_GOTO:
	case NFT_JUMP:
		if (expr->chain->etype == EXPR_VALUE &&
		    verdict_validate_chain(ctx, expr->chain))
			return -1;

		break;
	}

	return expr_evaluate_primary(ctx, exprp);
}

static int expr_evaluate(struct eval_ctx *ctx, struct expr **expr)
{
	if (ctx->nft->debug_mask & NFT_DEBUG_EVALUATION) {
		struct error_record *erec;
		erec = erec_create(EREC_INFORMATIONAL, &(*expr)->location,
				   "Evaluate %s", expr_name(*expr));
		erec_print(&ctx->nft->output, erec, ctx->nft->debug_mask);
		expr_print(*expr, &ctx->nft->output);
		nft_print(&ctx->nft->output, "\n\n");
		erec_destroy(erec);
	}

	switch ((*expr)->etype) {
	case EXPR_SYMBOL:
		return expr_evaluate_symbol(ctx, expr);
	case EXPR_VARIABLE:
		return expr_evaluate_variable(ctx, expr);
	case EXPR_SET_REF:
		expr_evaluate_set_ref(ctx, *expr);
		return 0;
	case EXPR_VALUE:
		return expr_evaluate_value(ctx, expr);
	case EXPR_EXTHDR:
		return expr_evaluate_exthdr(ctx, expr);
	case EXPR_VERDICT:
		return expr_evaluate_verdict(ctx, expr);
	case EXPR_META:
		return expr_evaluate_meta(ctx, expr);
	case EXPR_SOCKET:
		return expr_evaluate_socket(ctx, expr);
	case EXPR_TUNNEL:
		return expr_evaluate_tunnel(ctx, expr);
	case EXPR_OSF:
		return expr_evaluate_osf(ctx, expr);
	case EXPR_FIB:
		return expr_evaluate_fib(ctx, expr);
	case EXPR_PAYLOAD:
		return expr_evaluate_payload_inner(ctx, expr);
	case EXPR_RT:
		return expr_evaluate_rt(ctx, expr);
	case EXPR_CT:
		return expr_evaluate_ct(ctx, expr);
	case EXPR_PREFIX:
		return expr_evaluate_prefix(ctx, expr);
	case EXPR_RANGE:
		return expr_evaluate_range(ctx, expr);
	case EXPR_UNARY:
		return expr_evaluate_unary(ctx, expr);
	case EXPR_BINOP:
		return expr_evaluate_binop(ctx, expr);
	case EXPR_CONCAT:
		return expr_evaluate_concat(ctx, expr);
	case EXPR_LIST:
		return expr_evaluate_list(ctx, expr);
	case EXPR_SET:
		return expr_evaluate_set(ctx, expr);
	case EXPR_SET_ELEM:
		return expr_evaluate_set_elem(ctx, expr);
	case EXPR_MAP:
		return expr_evaluate_map(ctx, expr);
	case EXPR_MAPPING:
		return expr_evaluate_mapping(ctx, expr);
	case EXPR_RELATIONAL:
		return expr_evaluate_relational(ctx, expr);
	case EXPR_NUMGEN:
		return expr_evaluate_numgen(ctx, expr);
	case EXPR_HASH:
		return expr_evaluate_hash(ctx, expr);
	case EXPR_XFRM:
		return expr_evaluate_xfrm(ctx, expr);
	case EXPR_SET_ELEM_CATCHALL:
		return expr_evaluate_set_elem_catchall(ctx, expr);
	case EXPR_RANGE_SYMBOL:
		return expr_evaluate_symbol_range(ctx, expr);
	default:
		BUG("unknown expression type %s", expr_name(*expr));
	}
}

static int stmt_evaluate_expr(struct eval_ctx *ctx, struct stmt *stmt)
{
	memset(&ctx->ectx, 0, sizeof(ctx->ectx));
	return expr_evaluate(ctx, &stmt->expr);
}

static int stmt_prefix_conversion(struct eval_ctx *ctx, struct expr **expr,
				  enum byteorder byteorder)
{
	struct expr *mask, *and, *or, *prefix, *base, *range;
	int ret;

	prefix = *expr;
	base = prefix->prefix;

	if (base->etype != EXPR_VALUE)
		return expr_error(ctx->msgs, prefix,
				  "you cannot specify a prefix here, "
				  "unknown type %s", base->dtype->name);

	if (!expr_is_constant(base))
		return expr_error(ctx->msgs, prefix,
				  "Prefix expression is undefined for "
				  "non-constant expressions");

	if (expr_basetype(base)->type != TYPE_INTEGER)
		return expr_error(ctx->msgs, prefix,
				  "Prefix expression expected integer value");

	mask = constant_expr_alloc(&prefix->location, expr_basetype(base),
				   BYTEORDER_HOST_ENDIAN, base->len, NULL);

	mpz_prefixmask(mask->value, base->len, prefix->prefix_len);
	and = binop_expr_alloc(&prefix->location, OP_AND, expr_get(base), mask);

	mask = constant_expr_alloc(&prefix->location, expr_basetype(base),
				   BYTEORDER_HOST_ENDIAN, base->len, NULL);
	mpz_bitmask(mask->value, prefix->len - prefix->prefix_len);
	or = binop_expr_alloc(&prefix->location, OP_OR, expr_get(base), mask);

	range = range_expr_alloc(&prefix->location, and, or);
	ret = expr_evaluate(ctx, &range);
	if (ret < 0) {
		expr_free(range);
		return ret;
	}

	expr_free(*expr);
	*expr = range;
	return 0;
}

static int __stmt_evaluate_arg(struct eval_ctx *ctx, struct stmt *stmt,
			       const struct datatype *dtype, unsigned int len,
			       enum byteorder byteorder, struct expr **expr)
{
	if ((*expr)->etype == EXPR_PAYLOAD &&
	    (*expr)->dtype->type == TYPE_INTEGER &&
	    ((*expr)->dtype->type != datatype_basetype(dtype)->type ||
	     (*expr)->len != len))
		return stmt_binary_error(ctx, *expr, stmt,
					 "datatype mismatch: expected %s, "
					 "expression has type %s with length %d",
					 dtype->desc, (*expr)->dtype->desc,
					 (*expr)->len);

	if (!datatype_compatible(dtype, (*expr)->dtype))
		return stmt_binary_error(ctx, *expr, stmt,		/* verdict vs invalid? */
					 "datatype mismatch: expected %s, "
					 "expression has type %s",
					 dtype->desc, (*expr)->dtype->desc);

	if (dtype->type == TYPE_MARK &&
	    datatype_equal(datatype_basetype(dtype), datatype_basetype((*expr)->dtype)) &&
	    !expr_is_constant(*expr))
		return byteorder_conversion(ctx, expr, byteorder);

	/* we are setting a value, we can't use a set */
	switch ((*expr)->etype) {
	case EXPR_SET:
		return stmt_binary_error(ctx, *expr, stmt,
					 "you cannot use a set here, unknown "
					 "value to use");
	case EXPR_SET_REF:
		return stmt_binary_error(ctx, *expr, stmt,
					 "you cannot reference a set here, "
					 "unknown value to use");
	case EXPR_RT:
		return byteorder_conversion(ctx, expr, byteorder);
	case EXPR_PREFIX:
		return stmt_prefix_conversion(ctx, expr, byteorder);
	case EXPR_NUMGEN:
		if (dtype->type == TYPE_IPADDR)
			return byteorder_conversion(ctx, expr, byteorder);
		break;
	default:
		break;
	}

	return 0;
}

static int stmt_evaluate_arg(struct eval_ctx *ctx, struct stmt *stmt,
			     const struct datatype *dtype, unsigned int len,
			     enum byteorder byteorder, struct expr **expr)
{
	__expr_set_context(&ctx->ectx, dtype, byteorder, len, 0);
	if (expr_evaluate(ctx, expr) < 0)
		return -1;

	return __stmt_evaluate_arg(ctx, stmt, dtype, len, byteorder, expr);
}

/* like stmt_evaluate_arg, but keep existing context created
 * by previous expr_evaluate().
 *
 * This is needed for add/update statements:
 * ctx->ectx.key has the set key, which may be needed for 'typeof'
 * sets: the 'add/update' expression might contain integer data types.
 *
 * Without the key we cannot derive the element size.
 */
static int stmt_evaluate_key(struct eval_ctx *ctx, struct stmt *stmt,
			     const struct datatype *dtype, unsigned int len,
			     enum byteorder byteorder, struct expr **expr)
{
	if (expr_evaluate(ctx, expr) < 0)
		return -1;

	return __stmt_evaluate_arg(ctx, stmt, dtype, len, byteorder, expr);
}

static int stmt_evaluate_verdict(struct eval_ctx *ctx, struct stmt *stmt)
{
	if (stmt_evaluate_arg(ctx, stmt, &verdict_type, 0, 0, &stmt->expr) < 0)
		return -1;

	switch (stmt->expr->etype) {
	case EXPR_VERDICT:
		if (stmt->expr->verdict != NFT_CONTINUE)
			stmt->flags |= STMT_F_TERMINAL;
		if (stmt->expr->chain != NULL) {
			if (stmt_evaluate_arg(ctx, stmt, &string_type, 0, 0,
					      &stmt->expr->chain) < 0)
				return -1;
			if (stmt->expr->chain->etype != EXPR_VALUE) {
				return expr_error(ctx->msgs, stmt->expr->chain,
						  "not a value expression");
			}

			if (verdict_validate_chain(ctx, stmt->expr->chain))
				return -1;
		}
		break;
	case EXPR_MAP:
		break;
	default:
		BUG("invalid verdict expression %s", expr_name(stmt->expr));
	}
	return 0;
}

static bool stmt_evaluate_payload_need_csum(const struct expr *payload)
{
	const struct proto_desc *desc;

	if (payload->payload.base == PROTO_BASE_INNER_HDR)
		return true;

	desc = payload->payload.desc;

	return desc && desc->checksum_key;
}

static bool stmt_evaluate_is_vlan(const struct expr *payload)
{
	return payload->payload.base == PROTO_BASE_LL_HDR &&
	       payload->payload.desc == &proto_vlan;
}

/** stmt_evaluate_payload_need_aligned_fetch
 *
 * @payload:	payload expression to check
 *
 * Some types of stores need to round up to an even sized byte length,
 * typically 1 -> 2 or 3 -> 4 bytes.
 *
 * This includes anything that needs inet checksum fixups and also writes
 * to the vlan header.  This is because of VLAN header removal in the
 * kernel: nftables kernel side provides illusion of a linear packet, i.e.
 * ethernet_header|vlan_header|network_header.
 *
 * When a write to the vlan header is performed, kernel side updates the
 * pseudoheader, but only accepts 2 or 4 byte writes to vlan proto/TCI.
 *
 * Return true if load needs to be expanded to cover even amount of bytes
 */
static bool stmt_evaluate_payload_need_aligned_fetch(const struct expr *payload)
{
	if (stmt_evaluate_payload_need_csum(payload))
		return true;

	if (stmt_evaluate_is_vlan(payload))
		return true;

	return false;
}

static int stmt_evaluate_exthdr(struct eval_ctx *ctx, struct stmt *stmt)
{
	struct expr *exthdr;
	int ret;

	if (__expr_evaluate_exthdr(ctx, &stmt->exthdr.expr) < 0)
		return -1;

	exthdr = stmt->exthdr.expr;
	ret = stmt_evaluate_arg(ctx, stmt, exthdr->dtype, exthdr->len,
				BYTEORDER_BIG_ENDIAN,
				&stmt->exthdr.val);
	if (ret < 0)
		return ret;

	if (stmt->exthdr.val->etype == EXPR_RANGE)
		return stmt_error_range(ctx, stmt, stmt->exthdr.val);

	return 0;
}

static int stmt_evaluate_payload(struct eval_ctx *ctx, struct stmt *stmt)
{
	struct expr *mask, *and, *xor, *expr, *payload_bytes;
	unsigned int payload_byte_size, payload_byte_offset;
	uint8_t shift_imm, data[NFT_REG_SIZE];
	unsigned int masklen, extra_len = 0;
	struct expr *payload;
	mpz_t bitmask, ff;
	bool aligned_fetch;

	if (stmt->payload.expr->payload.inner_desc) {
		return expr_error(ctx->msgs, stmt->payload.expr,
				  "payload statement for this expression is not supported");
	}

	if (__expr_evaluate_payload(ctx, stmt->payload.expr) < 0)
		return -1;

	payload = stmt->payload.expr;
	if (stmt_evaluate_arg(ctx, stmt, payload->dtype, payload->len,
			      payload->byteorder, &stmt->payload.val) < 0)
		return -1;

	if (!expr_is_constant(stmt->payload.val) &&
	    byteorder_conversion(ctx, &stmt->payload.val,
				 payload->byteorder) < 0)
		return -1;

	if (stmt->payload.val->etype == EXPR_RANGE)
		return stmt_error_range(ctx, stmt, stmt->payload.val);

	aligned_fetch = stmt_evaluate_payload_need_aligned_fetch(payload);

	if (!payload_needs_adjustment(payload)) {

		/* We still need to munge the payload in case we have to
		 * update checksum and the length is not even because
		 * kernel checksum functions cannot deal with odd lengths.
		 */
		if (!aligned_fetch || ((payload->len / BITS_PER_BYTE) & 1) == 0)
			return 0;
	}

	payload_byte_offset = payload->payload.offset / BITS_PER_BYTE;

	shift_imm = expr_offset_shift(payload, payload->payload.offset,
				      &extra_len);
	payload_byte_size = div_round_up(payload->len + extra_len,
					 BITS_PER_BYTE);

	if (payload_byte_size > sizeof(data))
		return expr_error(ctx->msgs, stmt->payload.expr,
				  "uneven load cannot span more than %zu bytes, got %u",
				  sizeof(data), payload_byte_size);

	if (aligned_fetch && payload_byte_size & 1) {
		payload_byte_size++;

		if (payload_byte_offset & 1) { /* prefer 16bit aligned fetch */
			payload_byte_offset--;
			assert(payload->payload.offset >= BITS_PER_BYTE);
		} else {
			shift_imm += BITS_PER_BYTE;
		}
	}

	switch (stmt->payload.val->etype) {
	case EXPR_VALUE:
		if (shift_imm)
			mpz_lshift_ui(stmt->payload.val->value, shift_imm);
		break;
	case EXPR_BINOP:
		expr = stmt->payload.val;
		while (expr->left->etype == EXPR_BINOP)
			expr = expr->left;

		if (expr->left->etype != EXPR_PAYLOAD)
			break;

		if (!payload_expr_cmp(payload, expr->left))
			break;

		/* Adjust payload to fetch 16-bits. */
		expr->left->payload.offset = payload_byte_offset * BITS_PER_BYTE;
		expr->left->len = payload_byte_size * BITS_PER_BYTE;
		expr->left->payload.is_raw = 1;

		switch (expr->right->etype) {
		case EXPR_VALUE:
			if (shift_imm)
				mpz_lshift_ui(expr->right->value, shift_imm);

			/* build bitmask to keep unmodified bits intact */
			if (expr->op == OP_AND) {
				masklen = payload_byte_size * BITS_PER_BYTE;
				mpz_init_bitmask(ff, masklen);

				mpz_init2(bitmask, masklen);
				mpz_bitmask(bitmask, payload->len);
				mpz_lshift_ui(bitmask, shift_imm);

				mpz_xor(bitmask, ff, bitmask);
				mpz_clear(ff);

				mpz_ior(bitmask, expr->right->value, bitmask);
				mpz_set(expr->right->value, bitmask);

				mpz_clear(bitmask);
			}
			break;
		default:
			return expr_error(ctx->msgs, expr->right,
					  "payload statement for this expression is not supported");
		}

		expr_free(stmt->payload.expr);
		/* statement payload is the same in expr and value, update it. */
		stmt->payload.expr = expr_clone(expr->left);
		payload = stmt->payload.expr;
		ctx->stmt_len = stmt->payload.expr->len;

		if (expr_evaluate(ctx, &stmt->payload.val) < 0)
			return -1;

		return 0;
	default:
		return expr_error(ctx->msgs, stmt->payload.val,
				  "payload statement for this expression is not supported");
	}

	masklen = payload_byte_size * BITS_PER_BYTE;
	mpz_init_bitmask(ff, masklen);

	mpz_init2(bitmask, masklen);
	mpz_bitmask(bitmask, payload->len);
	mpz_lshift_ui(bitmask, shift_imm);

	mpz_xor(bitmask, ff, bitmask);
	mpz_clear(ff);

	assert(sizeof(data) * BITS_PER_BYTE >= masklen);
	mpz_export_data(data, bitmask, payload->byteorder, payload_byte_size);
	mask = constant_expr_alloc(&payload->location, expr_basetype(payload),
				   payload->byteorder, masklen, data);
	mpz_clear(bitmask);

	payload_bytes = payload_expr_alloc(&payload->location, NULL, 0);
	payload_init_raw(payload_bytes, payload->payload.base,
			 payload_byte_offset * BITS_PER_BYTE,
			 payload_byte_size * BITS_PER_BYTE);

	payload_bytes->payload.is_raw = 1;
	payload_bytes->payload.desc	 = payload->payload.desc;
	payload_bytes->byteorder	 = payload->byteorder;

	payload->len = payload_bytes->len;
	payload->payload.offset = payload_bytes->payload.offset;

	and = binop_expr_alloc(&payload->location, OP_AND, payload_bytes, mask);

	and->dtype	= payload_bytes->dtype;
	and->byteorder	= payload_bytes->byteorder;
	and->len	= payload_bytes->len;

	xor = binop_expr_alloc(&payload->location, OP_XOR, and,
			       stmt->payload.val);
	xor->dtype	= payload->dtype;
	xor->byteorder	= payload->byteorder;
	xor->len	= mask->len;

	stmt->payload.val = xor;

	return expr_evaluate(ctx, &stmt->payload.val);
}

static int stmt_evaluate_stateful(struct eval_ctx *ctx, struct stmt *stmt, const char *name)
{
	if (stmt_evaluate(ctx, stmt) < 0)
		return -1;

	if (!(stmt->flags & STMT_F_STATEFUL))
		return stmt_error(ctx, stmt, "%s statement must be stateful", name);

	return 0;
}

static int stmt_evaluate_meter(struct eval_ctx *ctx, struct stmt *stmt)
{
	struct expr *key, *setref;
	struct set *existing_set;
	struct table *table;

	table = table_cache_find(&ctx->nft->cache.table_cache,
				 ctx->cmd->handle.table.name,
				 ctx->cmd->handle.family);
	if (table == NULL)
		return table_not_found(ctx);

	existing_set = set_cache_find(table, stmt->meter.name);
	if (existing_set &&
	    (!set_is_meter_compat(existing_set->flags) ||
	     set_is_map(existing_set->flags)))
		return cmd_error(ctx, &stmt->location,
				 "%s; meter '%s' overlaps an existing %s '%s' in family %s",
				 strerror(EEXIST),
				 stmt->meter.name,
				 set_is_map(existing_set->flags) ? "map" : "set",
				 existing_set->handle.set.name,
				 family2str(existing_set->handle.family));

	expr_set_context(&ctx->ectx, NULL, 0);
	if (expr_evaluate(ctx, &stmt->meter.key) < 0)
		return -1;
	if (expr_is_constant(stmt->meter.key))
		return expr_error(ctx->msgs, stmt->meter.key,
				  "Meter key expression can not be constant");
	if (stmt->meter.key->comment)
		return expr_error(ctx->msgs, stmt->meter.key,
				  "Meter key expression can not contain comments");

	/* Declare an empty set */
	key = stmt->meter.key;
	if (existing_set) {
		if ((existing_set->flags & NFT_SET_TIMEOUT) && !key->timeout)
			return expr_error(ctx->msgs, stmt->meter.key,
					  "existing set '%s' has timeout flag",
					  stmt->meter.name);

		if ((existing_set->flags & NFT_SET_TIMEOUT) == 0 && key->timeout)
			return expr_error(ctx->msgs, stmt->meter.key,
					  "existing set '%s' lacks timeout flag",
					  stmt->meter.name);

		if (stmt->meter.size > 0 && existing_set->desc.size != stmt->meter.size)
			return expr_error(ctx->msgs, stmt->meter.key,
					  "existing set '%s' has size %u, meter has %u",
					  stmt->meter.name, existing_set->desc.size,
					  stmt->meter.size);
		setref = set_ref_expr_alloc(&key->location, existing_set);
	} else {
		struct expr *set;

		set = set_expr_alloc(&key->location, existing_set);
		if (key->timeout)
			expr_set(set)->set_flags |= NFT_SET_TIMEOUT;

		expr_set(set)->set_flags |= NFT_SET_EVAL;
		setref = implicit_set_declaration(ctx, stmt->meter.name,
						  expr_get(key), NULL, set,
						  NFT_SET_EVAL | expr_set(set)->set_flags);
		if (setref)
			setref->set->desc.size = stmt->meter.size;
	}

	if (!setref)
		return -1;

	stmt->meter.set = setref;

	if (stmt_evaluate_stateful(ctx, stmt->meter.stmt, "meter") < 0)
		return -1;

	return 0;
}

static int stmt_evaluate_meta(struct eval_ctx *ctx, struct stmt *stmt)
{
	int ret;

	ctx->stmt_len = stmt->meta.tmpl->len;

	ret = stmt_evaluate_arg(ctx, stmt,
				stmt->meta.tmpl->dtype,
				stmt->meta.tmpl->len,
				stmt->meta.tmpl->byteorder,
				&stmt->meta.expr);
	if (ret < 0)
		return ret;

	if (stmt->meta.expr->etype == EXPR_RANGE)
		return stmt_error_range(ctx, stmt, stmt->meta.expr);

	return ret;
}

static int stmt_evaluate_ct(struct eval_ctx *ctx, struct stmt *stmt)
{
	int ret;

	ctx->stmt_len = stmt->ct.tmpl->len;

	ret = stmt_evaluate_arg(ctx, stmt,
				stmt->ct.tmpl->dtype,
				stmt->ct.tmpl->len,
				stmt->ct.tmpl->byteorder,
				&stmt->ct.expr);
	if (ret < 0)
		return -1;

	if (stmt->ct.key == NFT_CT_SECMARK && expr_is_constant(stmt->ct.expr))
		return stmt_error(ctx, stmt,
				  "ct secmark must not be set to constant value");

	if (stmt->ct.expr->etype == EXPR_RANGE)
		return stmt_error_range(ctx, stmt, stmt->ct.expr);

	return 0;
}

static int reject_payload_gen_dependency_tcp(struct eval_ctx *ctx,
					     struct stmt *stmt,
					     struct expr **payload)
{
	struct proto_ctx *pctx = eval_proto_ctx(ctx);
	const struct proto_desc *desc;

	desc = pctx->protocol[PROTO_BASE_TRANSPORT_HDR].desc;
	if (desc != NULL)
		return 0;
	*payload = payload_expr_alloc(&stmt->location, &proto_tcp,
				      TCPHDR_UNSPEC);
	return 1;
}

static int reject_payload_gen_dependency_family(struct eval_ctx *ctx,
						struct stmt *stmt,
						struct expr **payload)
{
	struct proto_ctx *pctx = eval_proto_ctx(ctx);
	const struct proto_desc *base;

	base = pctx->protocol[PROTO_BASE_NETWORK_HDR].desc;
	if (base != NULL)
		return 0;

	if (stmt->reject.icmp_code < 0)
		return stmt_error(ctx, stmt, "missing icmp error type");

	/* Generate a network dependency */
	switch (stmt->reject.family) {
	case NFPROTO_IPV4:
		*payload = payload_expr_alloc(&stmt->location, &proto_ip,
					     IPHDR_PROTOCOL);
		break;
	case NFPROTO_IPV6:
		*payload = payload_expr_alloc(&stmt->location, &proto_ip6,
					     IP6HDR_NEXTHDR);
		break;
	default:
		BUG("unknown reject family");
	}
	return 1;
}

static int stmt_reject_gen_dependency(struct eval_ctx *ctx, struct stmt *stmt)
{
	struct expr *payload = NULL;
	struct stmt *nstmt;
	int ret;

	switch (stmt->reject.type) {
	case NFT_REJECT_TCP_RST:
		ret = reject_payload_gen_dependency_tcp(ctx, stmt, &payload);
		break;
	case NFT_REJECT_ICMP_UNREACH:
		ret = reject_payload_gen_dependency_family(ctx, stmt, &payload);
		break;
	default:
		BUG("cannot generate reject dependency for type %d",
		    stmt->reject.type);
	}
	if (ret <= 0)
		return ret;

	if (payload_gen_dependency(ctx, payload, &nstmt) < 0) {
		ret = -1;
		goto out;
	}

	/*
	 * Unlike payload deps this adds the dependency at the beginning, i.e.
	 * log ... reject with tcp-reset
	 * turns into
	 * meta l4proto tcp log ... reject with tcp-reset
	 *
	 * Otherwise we'd log things that won't be rejected.
	 */
	list_add(&nstmt->list, &ctx->rule->stmts);
out:
	free(payload);
	return ret;
}

static int stmt_evaluate_reject_inet_family(struct eval_ctx *ctx,
					    struct stmt *stmt,
					    const struct proto_desc *desc)
{
	struct proto_ctx *pctx = eval_proto_ctx(ctx);
	const struct proto_desc *base;
	int protocol;

	switch (stmt->reject.type) {
	case NFT_REJECT_TCP_RST:
		break;
	case NFT_REJECT_ICMPX_UNREACH:
		break;
	case NFT_REJECT_ICMP_UNREACH:
		base = pctx->protocol[PROTO_BASE_LL_HDR].desc;
		protocol = proto_find_num(base, desc);
		switch (protocol) {
		case NFPROTO_IPV4:
		case __constant_htons(ETH_P_IP):
			if (stmt->reject.family == NFPROTO_IPV4)
				break;
			return stmt_binary_error(ctx, stmt->reject.expr,
				  &pctx->protocol[PROTO_BASE_NETWORK_HDR],
				  "conflicting protocols specified: ip vs ip6");
		case NFPROTO_IPV6:
		case __constant_htons(ETH_P_IPV6):
			if (stmt->reject.family == NFPROTO_IPV6)
				break;
			return stmt_binary_error(ctx, stmt->reject.expr,
				  &pctx->protocol[PROTO_BASE_NETWORK_HDR],
				  "conflicting protocols specified: ip vs ip6");
		default:
			return stmt_error(ctx, stmt,
				  "cannot infer ICMP reject variant to use: explicit value required.\n");
		}
		break;
	}

	return 0;
}

static int stmt_evaluate_reject_inet(struct eval_ctx *ctx, struct stmt *stmt)
{
	struct proto_ctx *pctx = eval_proto_ctx(ctx);
	const struct proto_desc *desc;

	desc = pctx->protocol[PROTO_BASE_NETWORK_HDR].desc;
	if (desc != NULL &&
	    stmt_evaluate_reject_inet_family(ctx, stmt, desc) < 0)
		return -1;
	if (stmt->reject.type == NFT_REJECT_ICMPX_UNREACH)
		return 0;
	if (stmt_reject_gen_dependency(ctx, stmt) < 0)
		return -1;
	return 0;
}

static int stmt_evaluate_reject_bridge_family(struct eval_ctx *ctx,
					      struct stmt *stmt,
					      const struct proto_desc *desc)
{
	struct proto_ctx *pctx = eval_proto_ctx(ctx);
	const struct proto_desc *base;
	int protocol;

	switch (stmt->reject.type) {
	case NFT_REJECT_ICMPX_UNREACH:
	case NFT_REJECT_TCP_RST:
		base = pctx->protocol[PROTO_BASE_LL_HDR].desc;
		protocol = proto_find_num(base, desc);
		switch (protocol) {
		case __constant_htons(ETH_P_IP):
		case __constant_htons(ETH_P_IPV6):
			break;
		default:
			return stmt_binary_error(ctx, stmt,
				    &pctx->protocol[PROTO_BASE_NETWORK_HDR],
				    "cannot reject this network family");
		}
		break;
	case NFT_REJECT_ICMP_UNREACH:
		base = pctx->protocol[PROTO_BASE_LL_HDR].desc;
		protocol = proto_find_num(base, desc);
		switch (protocol) {
		case __constant_htons(ETH_P_IP):
			if (NFPROTO_IPV4 == stmt->reject.family)
				break;
			return stmt_binary_error(ctx, stmt->reject.expr,
				  &pctx->protocol[PROTO_BASE_NETWORK_HDR],
				  "conflicting protocols specified: ip vs ip6");
		case __constant_htons(ETH_P_IPV6):
			if (NFPROTO_IPV6 == stmt->reject.family)
				break;
			return stmt_binary_error(ctx, stmt->reject.expr,
				  &pctx->protocol[PROTO_BASE_NETWORK_HDR],
				  "conflicting protocols specified: ip vs ip6");
		default:
			return stmt_binary_error(ctx, stmt,
				    &pctx->protocol[PROTO_BASE_NETWORK_HDR],
				    "cannot reject this network family");
		}
		break;
	}

	return 0;
}

static int stmt_evaluate_reject_bridge(struct eval_ctx *ctx, struct stmt *stmt)
{
	struct proto_ctx *pctx = eval_proto_ctx(ctx);
	const struct proto_desc *desc;

	desc = pctx->protocol[PROTO_BASE_LL_HDR].desc;
	if (desc != &proto_eth && desc != &proto_vlan && desc != &proto_netdev)
		return __stmt_binary_error(ctx, &stmt->location, NULL,
					   "cannot reject from this link layer protocol");

	desc = pctx->protocol[PROTO_BASE_NETWORK_HDR].desc;
	if (desc != NULL &&
	    stmt_evaluate_reject_bridge_family(ctx, stmt, desc) < 0)
		return -1;
	if (stmt->reject.type == NFT_REJECT_ICMPX_UNREACH)
		return 0;
	if (stmt_reject_gen_dependency(ctx, stmt) < 0)
		return -1;
	return 0;
}

static int stmt_reject_error(struct eval_ctx *ctx,
			     const struct stmt *stmt,
			     const char *msg)
{
	struct expr *e = stmt->reject.expr;

	if (e)
		return stmt_binary_error(ctx, e, stmt, "%s", msg);

	return stmt_error(ctx, stmt, "%s", msg);
}

static int stmt_evaluate_reject_family(struct eval_ctx *ctx, struct stmt *stmt)
{
	struct proto_ctx *pctx = eval_proto_ctx(ctx);

	switch (pctx->family) {
	case NFPROTO_ARP:
		return stmt_error(ctx, stmt, "cannot use reject with arp");
	case NFPROTO_IPV4:
	case NFPROTO_IPV6:
		switch (stmt->reject.type) {
		case NFT_REJECT_TCP_RST:
			if (stmt_reject_gen_dependency(ctx, stmt) < 0)
				return -1;
			break;
		case NFT_REJECT_ICMPX_UNREACH:
			return stmt_reject_error(ctx, stmt,
				   "abstracted ICMP unreachable not supported");
		case NFT_REJECT_ICMP_UNREACH:
			if (stmt->reject.family == pctx->family)
				break;
			return stmt_reject_error(ctx, stmt,
				  "conflicting protocols specified: ip vs ip6");
		}
		break;
	case NFPROTO_BRIDGE:
	case NFPROTO_NETDEV:
		if (stmt_evaluate_reject_bridge(ctx, stmt) < 0)
			return -1;
		break;
	case NFPROTO_INET:
		if (stmt_evaluate_reject_inet(ctx, stmt) < 0)
			return -1;
		break;
	}

	stmt->flags |= STMT_F_TERMINAL;
	return 0;
}

static int stmt_evaluate_reject_default(struct eval_ctx *ctx,
					  struct stmt *stmt)
{
	struct proto_ctx *pctx = eval_proto_ctx(ctx);
	const struct proto_desc *desc, *base;
	int protocol;

	switch (pctx->family) {
	case NFPROTO_IPV4:
	case NFPROTO_IPV6:
		stmt->reject.type = NFT_REJECT_ICMP_UNREACH;
		stmt->reject.family = pctx->family;
		if (pctx->family == NFPROTO_IPV4)
			stmt->reject.icmp_code = ICMP_PORT_UNREACH;
		else
			stmt->reject.icmp_code = ICMP6_DST_UNREACH_NOPORT;
		break;
	case NFPROTO_INET:
		desc = pctx->protocol[PROTO_BASE_NETWORK_HDR].desc;
		if (desc == NULL) {
			stmt->reject.type = NFT_REJECT_ICMPX_UNREACH;
			stmt->reject.icmp_code = NFT_REJECT_ICMPX_PORT_UNREACH;
			break;
		}
		stmt->reject.type = NFT_REJECT_ICMP_UNREACH;
		base = pctx->protocol[PROTO_BASE_LL_HDR].desc;
		protocol = proto_find_num(base, desc);
		switch (protocol) {
		case NFPROTO_IPV4:
		case __constant_htons(ETH_P_IP):
			stmt->reject.family = NFPROTO_IPV4;
			stmt->reject.icmp_code = ICMP_PORT_UNREACH;
			break;
		case NFPROTO_IPV6:
		case __constant_htons(ETH_P_IPV6):
			stmt->reject.family = NFPROTO_IPV6;
			stmt->reject.icmp_code = ICMP6_DST_UNREACH_NOPORT;
			break;
		}
		break;
	case NFPROTO_BRIDGE:
	case NFPROTO_NETDEV:
		desc = pctx->protocol[PROTO_BASE_NETWORK_HDR].desc;
		if (desc == NULL) {
			stmt->reject.type = NFT_REJECT_ICMPX_UNREACH;
			stmt->reject.icmp_code = NFT_REJECT_ICMPX_PORT_UNREACH;
			break;
		}
		stmt->reject.type = NFT_REJECT_ICMP_UNREACH;
		base = pctx->protocol[PROTO_BASE_LL_HDR].desc;
		protocol = proto_find_num(base, desc);
		switch (protocol) {
		case __constant_htons(ETH_P_IP):
			stmt->reject.family = NFPROTO_IPV4;
			stmt->reject.icmp_code = ICMP_PORT_UNREACH;
			break;
		case __constant_htons(ETH_P_IPV6):
			stmt->reject.family = NFPROTO_IPV6;
			stmt->reject.icmp_code = ICMP6_DST_UNREACH_NOPORT;
			break;
		}
		break;
	}
	return 0;
}

static int stmt_evaluate_reject_icmp(struct eval_ctx *ctx, struct stmt *stmt)
{
	struct parse_ctx parse_ctx = {
		.tbl	= &ctx->nft->output.tbl,
		.input	= &ctx->nft->input,
	};
	struct error_record *erec;
	struct expr *code;

	erec = symbol_parse(&parse_ctx, stmt->reject.expr, &code);
	if (erec != NULL) {
		erec_queue(erec, ctx->msgs);
		return -1;
	}

	if (mpz_cmp_ui(code->value, UINT8_MAX) > 0) {
		expr_free(code);
		return expr_error(ctx->msgs, stmt->reject.expr,
				  "reject code must be integer in range 0-255");
	}

	stmt->reject.icmp_code = mpz_get_uint8(code->value);
	expr_free(code);

	return 0;
}

static int stmt_evaluate_reset(struct eval_ctx *ctx, struct stmt *stmt)
{
	struct proto_ctx *pctx = eval_proto_ctx(ctx);
	const struct proto_desc *desc, *base;
	int protonum;

	desc = pctx->protocol[PROTO_BASE_TRANSPORT_HDR].desc;
	if (desc == NULL)
		return 0;

	base = pctx->protocol[PROTO_BASE_NETWORK_HDR].desc;
	if (base == NULL)
		base = &proto_inet_service;

	protonum = proto_find_num(base, desc);
	switch (protonum) {
	case IPPROTO_TCP:
		break;
	default:
		if (stmt->reject.type == NFT_REJECT_TCP_RST) {
			return stmt_binary_error(ctx, stmt,
				 &pctx->protocol[PROTO_BASE_TRANSPORT_HDR],
				 "you cannot use tcp reset with this protocol");
		}
		break;
	}
	return 0;
}

static int stmt_evaluate_reject(struct eval_ctx *ctx, struct stmt *stmt)
{
	if (stmt->reject.icmp_code < 0) {
		if (stmt_evaluate_reject_default(ctx, stmt) < 0)
			return -1;
	} else if (stmt->reject.expr != NULL) {
		if (stmt_evaluate_reject_icmp(ctx, stmt) < 0)
			return -1;
	} else {
		if (stmt_evaluate_reset(ctx, stmt) < 0)
			return -1;
	}

	return stmt_evaluate_reject_family(ctx, stmt);
}

static int nat_evaluate_family(struct eval_ctx *ctx, struct stmt *stmt)
{
	struct proto_ctx *pctx = eval_proto_ctx(ctx);
	const struct proto_desc *nproto;

	switch (pctx->family) {
	case NFPROTO_IPV4:
	case NFPROTO_IPV6:
		if (stmt->nat.family == NFPROTO_UNSPEC)
			stmt->nat.family = pctx->family;
		return 0;
	case NFPROTO_INET:
		if (!stmt->nat.addr) {
			stmt->nat.family = NFPROTO_INET;
			return 0;
		}
		if (stmt->nat.family != NFPROTO_UNSPEC)
			return 0;

		nproto = pctx->protocol[PROTO_BASE_NETWORK_HDR].desc;

		if (nproto == &proto_ip)
			stmt->nat.family = NFPROTO_IPV4;
		else if (nproto == &proto_ip6)
			stmt->nat.family = NFPROTO_IPV6;

		return 0;
	default:
		return stmt_error(ctx, stmt,
				  "NAT is only supported for IPv4/IPv6");
	}
}

static const struct datatype *get_addr_dtype(uint8_t family)
{
	switch (family) {
	case NFPROTO_IPV4:
		return &ipaddr_type;
	case NFPROTO_IPV6:
		return &ip6addr_type;
	}

	return &invalid_type;
}

static int evaluate_addr(struct eval_ctx *ctx, struct stmt *stmt,
			     struct expr **expr)
{
	struct proto_ctx *pctx = eval_proto_ctx(ctx);
	const struct datatype *dtype;

	dtype = get_addr_dtype(pctx->family);

	return stmt_evaluate_arg(ctx, stmt, dtype, dtype->size,
				 BYTEORDER_BIG_ENDIAN,
				 expr);
}

static bool nat_evaluate_addr_has_th_expr(const struct expr *map)
{
	const struct expr *i, *concat;

	if (!map || map->etype != EXPR_MAP)
		return false;

	concat = map->map;
	if (concat ->etype != EXPR_CONCAT)
		return false;

	list_for_each_entry(i, &expr_concat(concat)->expressions, list) {
		enum proto_bases base;

		if (i->etype == EXPR_PAYLOAD &&
		    i->payload.base == PROTO_BASE_TRANSPORT_HDR &&
		    i->payload.desc != &proto_th)
			return true;

		if ((i->flags & EXPR_F_PROTOCOL) == 0)
			continue;

		switch (i->etype) {
		case EXPR_META:
			base = i->meta.base;
			break;
		case EXPR_PAYLOAD:
			base = i->payload.base;
			break;
		default:
			return false;
		}

		if (base == PROTO_BASE_NETWORK_HDR)
			return true;
	}

	return false;
}

static int nat_evaluate_transport(struct eval_ctx *ctx, struct stmt *stmt,
				  struct expr **expr)
{
	struct proto_ctx *pctx = eval_proto_ctx(ctx);
	int err;

	err = stmt_evaluate_arg(ctx, stmt,
				&inet_service_type, 2 * BITS_PER_BYTE,
				BYTEORDER_BIG_ENDIAN, expr);
	if (err < 0)
		return err;

	if (pctx->protocol[PROTO_BASE_TRANSPORT_HDR].desc == NULL &&
	    !nat_evaluate_addr_has_th_expr(stmt->nat.addr))
		return stmt_binary_error(ctx, *expr, stmt,
					 "transport protocol mapping is only "
					 "valid after transport protocol match");

	return 0;
}

static int stmt_evaluate_l3proto(struct eval_ctx *ctx,
				 struct stmt *stmt, uint8_t family)
{
	struct proto_ctx *pctx = eval_proto_ctx(ctx);
	const struct proto_desc *nproto;

	nproto = pctx->protocol[PROTO_BASE_NETWORK_HDR].desc;

	if ((nproto == &proto_ip && family != NFPROTO_IPV4) ||
	    (nproto == &proto_ip6 && family != NFPROTO_IPV6))
		return stmt_binary_error(ctx, stmt,
					 &pctx->protocol[PROTO_BASE_NETWORK_HDR],
					 "conflicting protocols specified: %s vs. %s. You must specify ip or ip6 family in %s statement",
					 pctx->protocol[PROTO_BASE_NETWORK_HDR].desc->name,
					 family2str(family),
					 stmt_name(stmt));
	return 0;
}

static void expr_family_infer(struct proto_ctx *pctx, const struct expr *expr,
			      uint8_t *family)
{
	struct expr *i;

	if (expr->etype == EXPR_MAP) {
		switch (expr->map->etype) {
		case EXPR_CONCAT:
			list_for_each_entry(i, &expr_concat(expr->map)->expressions, list) {
				if (i->etype == EXPR_PAYLOAD) {
					if (i->payload.desc == &proto_ip)
						*family = NFPROTO_IPV4;
					else if (i->payload.desc == &proto_ip6)
						*family = NFPROTO_IPV6;
				}
			}
			break;
		case EXPR_PAYLOAD:
			if (expr->map->payload.desc == &proto_ip)
				*family = NFPROTO_IPV4;
			else if (expr->map->payload.desc == &proto_ip6)
				*family = NFPROTO_IPV6;
			break;
		default:
			break;
		}
	}
}

static int stmt_evaluate_addr(struct eval_ctx *ctx, struct stmt *stmt,
			      uint8_t *family, struct expr **addr)
{
	struct proto_ctx *pctx = eval_proto_ctx(ctx);
	const struct datatype *dtype;
	int err;

	if (pctx->family == NFPROTO_INET) {
		if (*family == NFPROTO_INET ||
		    *family == NFPROTO_UNSPEC)
			expr_family_infer(pctx, *addr, family);

		dtype = get_addr_dtype(*family);
		if (dtype->size == 0) {
			return stmt_error(ctx, stmt,
					  "specify `%s ip' or '%s ip6' in %s table to disambiguate",
					  stmt_name(stmt), stmt_name(stmt), family2str(pctx->family));
		}

		err = stmt_evaluate_arg(ctx, stmt, dtype, dtype->size,
					BYTEORDER_BIG_ENDIAN, addr);
	} else {
		err = evaluate_addr(ctx, stmt, addr);
	}

	return err;
}

static int stmt_evaluate_nat_map(struct eval_ctx *ctx, struct stmt *stmt)
{
	struct proto_ctx *pctx = eval_proto_ctx(ctx);
	struct expr *one, *two, *data, *tmp;
	const struct datatype *dtype = NULL;
	const struct datatype *dtype2;
	int addr_type;
	int err;

	if (stmt->nat.proto)
		return stmt_binary_error(ctx, stmt, stmt->nat.proto,
				  "nat map and protocol are mutually exclusive");

	if (stmt->nat.family == NFPROTO_INET)
		expr_family_infer(pctx, stmt->nat.addr, &stmt->nat.family);

	switch (stmt->nat.family) {
	case NFPROTO_IPV4:
		addr_type = TYPE_IPADDR;
		break;
	case NFPROTO_IPV6:
		addr_type = TYPE_IP6ADDR;
		break;
	default:
		return stmt_error(ctx, stmt,
				  "specify `%s ip' or '%s ip6' in %s table to disambiguate",
				  stmt_name(stmt), stmt_name(stmt), family2str(pctx->family));
	}
	dtype = concat_type_alloc((addr_type << TYPE_BITS) | TYPE_INET_SERVICE);

	expr_set_context(&ctx->ectx, dtype, dtype->size);
	if (expr_evaluate(ctx, &stmt->nat.addr)) {
		err = -1;
		goto out;
	}

	if (pctx->protocol[PROTO_BASE_TRANSPORT_HDR].desc == NULL &&
	    !nat_evaluate_addr_has_th_expr(stmt->nat.addr)) {
		err = stmt_binary_error(ctx, stmt->nat.addr, stmt,
					 "transport protocol mapping is only "
					 "valid after transport protocol match");
		goto out;
	}

	if (stmt->nat.addr->etype != EXPR_MAP) {
		err = 0;
		goto out;
	}

	data = stmt->nat.addr->mappings->set->data;
	if (data->flags & EXPR_F_INTERVAL)
		stmt->nat.type_flags |= STMT_NAT_F_INTERVAL;

	datatype_set(data, dtype);

	if (expr_ops(data)->type != EXPR_CONCAT) {
		err = __stmt_evaluate_arg(ctx, stmt, dtype, dtype->size,
					   BYTEORDER_BIG_ENDIAN,
					   &stmt->nat.addr);
		goto out;
	}

	one = list_first_entry(&expr_concat(data)->expressions, struct expr, list);
	two = list_entry(one->list.next, struct expr, list);

	if (one == two || !list_is_last(&two->list, &expr_concat(data)->expressions)) {
		err = __stmt_evaluate_arg(ctx, stmt, dtype, dtype->size,
					   BYTEORDER_BIG_ENDIAN,
					   &stmt->nat.addr);
		goto out;
	}

	dtype2 = get_addr_dtype(stmt->nat.family);
	tmp = one;
	err = __stmt_evaluate_arg(ctx, stmt, dtype2, dtype2->size,
				  BYTEORDER_BIG_ENDIAN,
				  &tmp);
	if (err < 0)
		goto out;
	if (tmp != one)
		BUG("Internal error: Unexpected alteration of l3 expression");

	tmp = two;
	err = nat_evaluate_transport(ctx, stmt, &tmp);
	if (err < 0)
		goto out;
	if (tmp != two)
		BUG("Internal error: Unexpected alteration of l4 expression");

out:
	datatype_free(dtype);
	return err;
}

static bool nat_concat_map(struct eval_ctx *ctx, struct stmt *stmt)
{
	struct expr *i;

	if (stmt->nat.addr->etype != EXPR_MAP)
		return false;

	switch (stmt->nat.addr->mappings->etype) {
	case EXPR_SET:
		list_for_each_entry(i, &expr_set(stmt->nat.addr->mappings)->expressions, list) {
			if (i->etype == EXPR_MAPPING &&
			    i->right->etype == EXPR_CONCAT) {
				stmt->nat.type_flags |= STMT_NAT_F_CONCAT;
				return true;
			}
		}
		break;
	case EXPR_SYMBOL:
		/* expr_evaluate_map() see EXPR_SET_REF after this evaluation. */
		if (expr_evaluate(ctx, &stmt->nat.addr->mappings))
			return false;

		if (!set_is_datamap(stmt->nat.addr->mappings->set->flags)) {
			expr_error(ctx->msgs, stmt->nat.addr->mappings,
					  "Expression is not a map");
			return false;
		}

		if (stmt->nat.addr->mappings->set->data->etype == EXPR_CONCAT ||
		    stmt->nat.addr->mappings->set->data->dtype->subtypes) {
			stmt->nat.type_flags |= STMT_NAT_F_CONCAT;
			return true;
		}
		break;
	default:
		break;
	}

	return false;
}

static int stmt_evaluate_nat(struct eval_ctx *ctx, struct stmt *stmt)
{
	int err;

	err = nat_evaluate_family(ctx, stmt);
	if (err < 0)
		return err;

	if (stmt->nat.addr != NULL) {
		err = stmt_evaluate_l3proto(ctx, stmt, stmt->nat.family);
		if (err < 0)
			return err;

		if (nat_concat_map(ctx, stmt) ||
		    stmt->nat.type_flags & STMT_NAT_F_CONCAT) {

			err = stmt_evaluate_nat_map(ctx, stmt);
			if (err < 0)
				return err;

			stmt->flags |= STMT_F_TERMINAL;
			return 0;
		}

		err = stmt_evaluate_addr(ctx, stmt, &stmt->nat.family,
					 &stmt->nat.addr);
		if (err < 0)
			return err;
	}

	if (stmt->nat.proto != NULL) {
		err = nat_evaluate_transport(ctx, stmt, &stmt->nat.proto);
		if (err < 0)
			return err;

		stmt->nat.flags |= NF_NAT_RANGE_PROTO_SPECIFIED;
	}

	stmt->flags |= STMT_F_TERMINAL;
	return 0;
}

static int stmt_evaluate_tproxy(struct eval_ctx *ctx, struct stmt *stmt)
{
	struct proto_ctx *pctx = eval_proto_ctx(ctx);
	int err;

	switch (pctx->family) {
	case NFPROTO_IPV4:
	case NFPROTO_IPV6: /* fallthrough */
		if (stmt->tproxy.family == NFPROTO_UNSPEC)
			stmt->tproxy.family = pctx->family;
		break;
	case NFPROTO_INET:
		break;
	default:
		return stmt_error(ctx, stmt,
				  "tproxy is only supported for IPv4/IPv6/INET");
	}

	if (pctx->protocol[PROTO_BASE_TRANSPORT_HDR].desc == NULL)
		return stmt_error(ctx, stmt, "Transparent proxy support requires"
					     " transport protocol match");

	if (!stmt->tproxy.addr && !stmt->tproxy.port)
		return stmt_error(ctx, stmt, "Either address or port must be specified!");

	err = stmt_evaluate_l3proto(ctx, stmt, stmt->tproxy.family);
	if (err < 0)
		return err;

	if (stmt->tproxy.addr != NULL) {
		err = stmt_evaluate_addr(ctx, stmt, &stmt->tproxy.family,
					 &stmt->tproxy.addr);
		if (err < 0)
			return err;

		if (stmt->tproxy.addr->etype == EXPR_RANGE)
			return stmt_error(ctx, stmt, "Address ranges are not supported for tproxy.");
	}

	if (stmt->tproxy.port != NULL) {
		err = nat_evaluate_transport(ctx, stmt, &stmt->tproxy.port);
		if (err < 0)
			return err;

		if (stmt->tproxy.port->etype == EXPR_RANGE)
			return stmt_error(ctx, stmt, "Port ranges are not supported for tproxy.");
	}

	return 0;
}

static int stmt_evaluate_synproxy(struct eval_ctx *ctx, struct stmt *stmt)
{
	if (stmt->synproxy.flags != 0 &&
	    !(stmt->synproxy.flags & (NF_SYNPROXY_OPT_MSS |
				      NF_SYNPROXY_OPT_WSCALE |
				      NF_SYNPROXY_OPT_TIMESTAMP |
				      NF_SYNPROXY_OPT_SACK_PERM)))
		return stmt_error(ctx, stmt, "This flags are not supported for SYNPROXY");

	return 0;
}

static int rule_evaluate(struct eval_ctx *ctx, struct rule *rule,
			 enum cmd_ops op);

static int stmt_evaluate_chain(struct eval_ctx *ctx, struct stmt *stmt)
{
	struct chain *chain = chain_get(stmt->chain.chain);
	struct cmd *cmd;

	chain->flags |= CHAIN_F_BINDING;

	if (ctx->table != NULL) {
		list_add_tail(&chain->list, &ctx->table->chains);
	} else {
		struct rule *rule, *next;
		struct handle h;

		memset(&h, 0, sizeof(h));
		handle_merge(&h, &chain->handle);
		h.family = ctx->rule->handle.family;
		free_const(h.table.name);
		h.table.name = xstrdup(ctx->rule->handle.table.name);
		h.chain.location = stmt->location;
		h.chain_id = chain->handle.chain_id;

		cmd = cmd_alloc(CMD_ADD, CMD_OBJ_CHAIN, &h, &stmt->location,
				chain);
		cmd->location = stmt->location;
		list_add_tail(&cmd->list, &ctx->cmd->list);
		h.chain_id = chain->handle.chain_id;

		list_for_each_entry_safe(rule, next, &chain->rules, list) {
			struct eval_ctx rule_ctx = {
				.nft	= ctx->nft,
				.msgs	= ctx->msgs,
				.cmd	= ctx->cmd,
			};
			struct handle h2 = {};

			handle_merge(&rule->handle, &ctx->rule->handle);
			free_const(rule->handle.table.name);
			rule->handle.table.name = xstrdup(ctx->rule->handle.table.name);
			free_const(rule->handle.chain.name);
			rule->handle.chain.name = NULL;
			rule->handle.chain_id = chain->handle.chain_id;
			if (rule_evaluate(&rule_ctx, rule, CMD_INVALID) < 0)
				return -1;

			handle_merge(&h2, &rule->handle);
			cmd = cmd_alloc(CMD_ADD, CMD_OBJ_RULE, &h2,
					&rule->location, rule);
			list_add_tail(&cmd->list, &ctx->cmd->list);
			list_del(&rule->list);
		}
	}

	return 0;
}

static int stmt_evaluate_optstrip(struct eval_ctx *ctx, struct stmt *stmt)
{
	return expr_evaluate(ctx, &stmt->optstrip.expr);
}

static int stmt_evaluate_dup(struct eval_ctx *ctx, struct stmt *stmt)
{
	struct proto_ctx *pctx = eval_proto_ctx(ctx);
	int err;

	switch (pctx->family) {
	case NFPROTO_IPV4:
	case NFPROTO_IPV6:
		if (stmt->dup.to == NULL)
			return stmt_error(ctx, stmt,
					  "missing destination address");
		err = evaluate_addr(ctx, stmt, &stmt->dup.to);
		if (err < 0)
			return err;

		if (stmt->dup.dev != NULL) {
			err = stmt_evaluate_arg(ctx, stmt, &ifindex_type,
						sizeof(uint32_t) * BITS_PER_BYTE,
						BYTEORDER_HOST_ENDIAN,
						&stmt->dup.dev);
			if (err < 0)
				return err;

			if (stmt->dup.dev->etype == EXPR_RANGE)
				return stmt_error_range(ctx, stmt, stmt->dup.dev);
		}
		break;
	case NFPROTO_NETDEV:
		if (stmt->dup.to == NULL)
			return stmt_error(ctx, stmt,
					  "missing destination interface");
		if (stmt->dup.dev != NULL)
			return stmt_error(ctx, stmt, "cannot specify device");

		err = stmt_evaluate_arg(ctx, stmt, &ifindex_type,
					sizeof(uint32_t) * BITS_PER_BYTE,
					BYTEORDER_HOST_ENDIAN, &stmt->dup.to);
		if (err < 0)
			return err;
		break;
	default:
		return stmt_error(ctx, stmt, "unsupported family");
	}

	if (stmt->dup.to->etype == EXPR_RANGE)
		return stmt_error_range(ctx, stmt, stmt->dup.to);

	return 0;
}

static int stmt_evaluate_fwd(struct eval_ctx *ctx, struct stmt *stmt)
{
	struct proto_ctx *pctx = eval_proto_ctx(ctx);
	const struct datatype *dtype;
	int err, len;

	switch (pctx->family) {
	case NFPROTO_NETDEV:
		if (stmt->fwd.dev == NULL)
			return stmt_error(ctx, stmt,
					  "missing destination interface");

		err = stmt_evaluate_arg(ctx, stmt, &ifindex_type,
					sizeof(uint32_t) * BITS_PER_BYTE,
					BYTEORDER_HOST_ENDIAN, &stmt->fwd.dev);
		if (err < 0)
			return err;

		if (stmt->fwd.dev->etype == EXPR_RANGE)
			return stmt_error_range(ctx, stmt, stmt->fwd.dev);

		if (stmt->fwd.addr != NULL) {
			switch (stmt->fwd.family) {
			case NFPROTO_IPV4:
				dtype = &ipaddr_type;
				len   = 4 * BITS_PER_BYTE;
				break;
			case NFPROTO_IPV6:
				dtype = &ip6addr_type;
				len   = 16 * BITS_PER_BYTE;
				break;
			default:
				return stmt_error(ctx, stmt, "missing family");
			}
			err = stmt_evaluate_arg(ctx, stmt, dtype, len,
						BYTEORDER_BIG_ENDIAN,
						&stmt->fwd.addr);
			if (err < 0)
				return err;

			if (stmt->fwd.addr->etype == EXPR_RANGE)
				return stmt_error_range(ctx, stmt, stmt->fwd.addr);
		}
		break;
	default:
		return stmt_error(ctx, stmt, "unsupported family");
	}
	stmt->flags |= STMT_F_TERMINAL;
	return 0;
}

static int stmt_evaluate_queue(struct eval_ctx *ctx, struct stmt *stmt)
{
	if (stmt->queue.queue != NULL) {
		if (stmt_evaluate_arg(ctx, stmt, &integer_type, 16,
				      BYTEORDER_HOST_ENDIAN,
				      &stmt->queue.queue) < 0)
			return -1;

		if ((stmt->queue.flags & NFT_QUEUE_FLAG_CPU_FANOUT) &&
		    stmt->queue.queue->etype != EXPR_RANGE)
			return expr_error(ctx->msgs, stmt->queue.queue,
					  "fanout requires a range to be "
					  "specified");

		if (ctx->ectx.maxval > USHRT_MAX)
			return expr_error(ctx->msgs, stmt->queue.queue,
					  "queue expression max value exceeds %u", USHRT_MAX);
	}
	stmt->flags |= STMT_F_TERMINAL;
	return 0;
}

static int stmt_evaluate_log_prefix(struct eval_ctx *ctx, struct stmt *stmt)
{
	unsigned int len = strlen(stmt->log.prefix);

	if (len >= NF_LOG_PREFIXLEN)
		return stmt_error(ctx, stmt, "log prefix is too long");
	else if (len == 0)
		return stmt_error(ctx, stmt, "log prefix must have a minimum length of 1 character");

	return 0;
}

static int stmt_evaluate_log(struct eval_ctx *ctx, struct stmt *stmt)
{
	int ret = 0;

	if (stmt->log.flags & (STMT_LOG_GROUP | STMT_LOG_SNAPLEN |
			       STMT_LOG_QTHRESHOLD)) {
		if (stmt->log.flags & STMT_LOG_LEVEL)
			return stmt_error(ctx, stmt,
				  "level and group are mutually exclusive");
		if (stmt->log.logflags)
			return stmt_error(ctx, stmt,
				  "flags and group are mutually exclusive");
	}
	if (stmt->log.level == NFT_LOGLEVEL_AUDIT &&
	    (stmt->log.flags & ~STMT_LOG_LEVEL || stmt->log.logflags))
		return stmt_error(ctx, stmt,
				  "log level audit doesn't support any further options");

	if (stmt->log.prefix)
		ret = stmt_evaluate_log_prefix(ctx, stmt);

	return ret;
}

static int stmt_evaluate_set(struct eval_ctx *ctx, struct stmt *stmt)
{
	struct set *this_set;
	struct stmt *this;

	expr_set_context(&ctx->ectx, NULL, 0);
	if (expr_evaluate(ctx, &stmt->set.set) < 0)
		return -1;
	if (stmt->set.set->etype != EXPR_SET_REF)
		return expr_error(ctx->msgs, stmt->set.set,
				  "Expression does not refer to a set");

	if (stmt_evaluate_key(ctx, stmt,
			      stmt->set.set->set->key->dtype,
			      stmt->set.set->set->key->len,
			      stmt->set.set->set->key->byteorder,
			      &stmt->set.key->key) < 0)
		return -1;
	if (expr_is_constant(stmt->set.key))
		return expr_error(ctx->msgs, stmt->set.key,
				  "Key expression can not be constant");
	if (stmt->set.key->comment != NULL)
		return expr_error(ctx->msgs, stmt->set.key,
				  "Key expression comments are not supported");
	list_for_each_entry(this, &stmt->set.stmt_list, list) {
		if (stmt_evaluate_stateful(ctx, this, "set") < 0)
			return -1;
	}

	this_set = stmt->set.set->set;

	/* Make sure EVAL flag is set on set definition so that kernel
	 * picks a set that allows updates from the packet path.
	 *
	 * Alternatively we could error out in case 'flags dynamic' was
	 * not given, but we can repair this here.
	 */
	this_set->flags |= NFT_SET_EVAL;
	return 0;
}

static int stmt_evaluate_map(struct eval_ctx *ctx, struct stmt *stmt)
{
	struct stmt *this;

	expr_set_context(&ctx->ectx, NULL, 0);
	if (expr_evaluate(ctx, &stmt->map.set) < 0)
		return -1;
	if (stmt->map.set->etype != EXPR_SET_REF)
		return expr_error(ctx->msgs, stmt->map.set,
				  "Expression does not refer to a set");

	if (!set_is_map(stmt->map.set->set->flags))
		return expr_error(ctx->msgs, stmt->map.set,
				  "%s is not a map", stmt->map.set->set->handle.set.name);

	if (stmt_evaluate_key(ctx, stmt,
			      stmt->map.set->set->key->dtype,
			      stmt->map.set->set->key->len,
			      stmt->map.set->set->key->byteorder,
			      &stmt->map.key->key) < 0)
		return -1;
	if (expr_is_constant(stmt->map.key))
		return expr_error(ctx->msgs, stmt->map.key,
				  "Key expression can not be constant");
	if (stmt->map.key->comment != NULL)
		return expr_error(ctx->msgs, stmt->map.key,
				  "Key expression comments are not supported");

	if (stmt_evaluate_arg(ctx, stmt,
			      stmt->map.set->set->data->dtype,
			      stmt->map.set->set->data->len,
			      stmt->map.set->set->data->byteorder,
			      &stmt->map.data->key) < 0)
		return -1;
	if (expr_is_constant(stmt->map.data))
		return expr_error(ctx->msgs, stmt->map.data,
				  "Data expression can not be constant");
	if (stmt->map.data->comment != NULL)
		return expr_error(ctx->msgs, stmt->map.data,
				  "Data expression comments are not supported");
	if (stmt->map.data->timeout > 0)
		return expr_error(ctx->msgs, stmt->map.data,
				  "Data expression timeouts are not supported");

	list_for_each_entry(this, &stmt->map.stmt_list, list) {
		if (stmt_evaluate_stateful(ctx, this, "map") < 0)
			return -1;
	}

	return 0;
}

static int stmt_evaluate_objref_map(struct eval_ctx *ctx, struct stmt *stmt)
{
	struct expr *map = stmt->objref.expr;
	uint32_t set_flags = NFT_SET_OBJECT;
	struct expr *mappings;
	struct expr *key;

	expr_set_context(&ctx->ectx, NULL, 0);
	if (expr_evaluate(ctx, &map->map) < 0)
		return -1;
	if (expr_is_constant(map->map))
		return expr_error(ctx->msgs, map->map,
				  "Map expression can not be constant");

	mappings = map->mappings;

	switch (map->mappings->etype) {
	case EXPR_SET:
		set_flags |= expr_set(mappings)->set_flags;
		/* fallthrough */
	case EXPR_VARIABLE:
		key = constant_expr_alloc(&stmt->location,
					  ctx->ectx.dtype,
					  ctx->ectx.byteorder,
					  ctx->ectx.len, NULL);

		mappings = implicit_set_declaration(ctx, "__objmap%d",
						    key, NULL, mappings,
						    set_flags | NFT_SET_ANONYMOUS);
		if (!mappings)
			return -1;
		mappings->set->objtype  = stmt->objref.type;

		map->mappings = mappings;

		ctx->set = mappings->set;
		if (expr_evaluate(ctx, &map->mappings->set->init) < 0)
			return -1;

		if (map->mappings->set->init->etype != EXPR_SET) {
			return expr_error(ctx->msgs, map->mappings->set->init,
					  "Expression is not a map");
		}

		if (set_is_interval(expr_set(map->mappings->set->init)->set_flags) &&
		    !(expr_set(map->mappings->set->init)->set_flags & NFT_SET_CONCAT) &&
		    interval_set_eval(ctx, ctx->set, map->mappings->set->init) < 0)
			return -1;

		ctx->set = NULL;

		map_set_concat_info(map);
		/* fall through */
	case EXPR_SYMBOL:
		if (expr_evaluate(ctx, &map->mappings) < 0)
			return -1;
		if (map->mappings->etype != EXPR_SET_REF)
			return expr_error(ctx->msgs, map->mappings,
					  "Expression is not a map");
		if (!set_is_objmap(map->mappings->set->flags))
			return expr_error(ctx->msgs, map->mappings,
					  "Expression is not a map with objects");
		break;
	default:
		return expr_binary_error(ctx->msgs, map->mappings, map->map,
					 "invalid mapping expression %s",
					 expr_name(map->mappings));
	}

	if (!datatype_compatible(map->mappings->set->key->dtype, map->map->dtype))
		return expr_binary_error(ctx->msgs, map->mappings, map->map,
					 "datatype mismatch, map expects %s, "
					 "mapping expression has type %s",
					 map->mappings->set->key->dtype->desc,
					 map->map->dtype->desc);

	datatype_set(map, map->mappings->set->data->dtype);
	map->flags |= EXPR_F_CONSTANT;

	/* Data for range lookups needs to be in big endian order */
	if (map->mappings->set->flags & NFT_SET_INTERVAL &&
	    byteorder_conversion(ctx, &map->map, BYTEORDER_BIG_ENDIAN) < 0)
		return -1;

	return 0;
}

static int stmt_evaluate_objref(struct eval_ctx *ctx, struct stmt *stmt)
{
	/* We need specific map evaluation for stateful objects. */
	if (stmt->objref.expr->etype == EXPR_MAP)
		return stmt_evaluate_objref_map(ctx, stmt);

	if (stmt_evaluate_arg(ctx, stmt,
			      &string_type, NFT_OBJ_MAXNAMELEN * BITS_PER_BYTE,
			      BYTEORDER_HOST_ENDIAN, &stmt->objref.expr) < 0)
		return -1;

	if (!expr_is_constant(stmt->objref.expr))
		return expr_error(ctx->msgs, stmt->objref.expr,
				  "Counter expression must be constant");

	return 0;
}

int stmt_evaluate(struct eval_ctx *ctx, struct stmt *stmt)
{
	if (ctx->nft->debug_mask & NFT_DEBUG_EVALUATION) {
		struct error_record *erec;
		erec = erec_create(EREC_INFORMATIONAL, &stmt->location,
				   "Evaluate %s", stmt_name(stmt));
		erec_print(&ctx->nft->output, erec, ctx->nft->debug_mask);
		stmt_print(stmt, &ctx->nft->output);
		nft_print(&ctx->nft->output, "\n\n");
		erec_destroy(erec);
	}

	ctx->stmt_len = 0;

	switch (stmt->type) {
	case STMT_CONNLIMIT:
	case STMT_COUNTER:
	case STMT_LAST:
	case STMT_LIMIT:
	case STMT_QUOTA:
	case STMT_NOTRACK:
	case STMT_FLOW_OFFLOAD:
		return 0;
	case STMT_EXPRESSION:
		return stmt_evaluate_expr(ctx, stmt);
	case STMT_VERDICT:
		return stmt_evaluate_verdict(ctx, stmt);
	case STMT_PAYLOAD:
		return stmt_evaluate_payload(ctx, stmt);
	case STMT_EXTHDR:
		return stmt_evaluate_exthdr(ctx, stmt);
	case STMT_METER:
		return stmt_evaluate_meter(ctx, stmt);
	case STMT_META:
		return stmt_evaluate_meta(ctx, stmt);
	case STMT_CT:
		return stmt_evaluate_ct(ctx, stmt);
	case STMT_LOG:
		return stmt_evaluate_log(ctx, stmt);
	case STMT_REJECT:
		return stmt_evaluate_reject(ctx, stmt);
	case STMT_NAT:
		return stmt_evaluate_nat(ctx, stmt);
	case STMT_TPROXY:
		return stmt_evaluate_tproxy(ctx, stmt);
	case STMT_QUEUE:
		return stmt_evaluate_queue(ctx, stmt);
	case STMT_DUP:
		return stmt_evaluate_dup(ctx, stmt);
	case STMT_FWD:
		return stmt_evaluate_fwd(ctx, stmt);
	case STMT_SET:
		return stmt_evaluate_set(ctx, stmt);
	case STMT_OBJREF:
		return stmt_evaluate_objref(ctx, stmt);
	case STMT_MAP:
		return stmt_evaluate_map(ctx, stmt);
	case STMT_SYNPROXY:
		return stmt_evaluate_synproxy(ctx, stmt);
	case STMT_CHAIN:
		return stmt_evaluate_chain(ctx, stmt);
	case STMT_OPTSTRIP:
		return stmt_evaluate_optstrip(ctx, stmt);
	default:
		BUG("unknown statement type %d", stmt->type);
	}
}

static int setelem_evaluate(struct eval_ctx *ctx, struct cmd *cmd)
{
	struct table *table;
	struct set *set;

	table = table_cache_find(&ctx->nft->cache.table_cache,
				 ctx->cmd->handle.table.name,
				 ctx->cmd->handle.family);
	if (table == NULL)
		return table_not_found(ctx);

	set = set_cache_find(table, ctx->cmd->handle.set.name);
	if (set == NULL)
		return set_not_found(ctx, &ctx->cmd->handle.set.location,
				     ctx->cmd->handle.set.name);

	if (set->key == NULL)
		return -1;

	set->existing_set = set;
	ctx->set = set;
	expr_set_context(&ctx->ectx, set->key->dtype, set->key->len);
	if (expr_evaluate(ctx, &cmd->expr) < 0)
		return -1;

	cmd->elem.set = set_get(set);
	if (set_is_interval(ctx->set->flags)) {
		if (!(set->flags & NFT_SET_CONCAT) &&
		    interval_set_eval(ctx, ctx->set, cmd->expr) < 0)
			return -1;

		assert(cmd->expr->etype == EXPR_SET);
		expr_set(cmd->expr)->set_flags |= NFT_SET_INTERVAL;
	}

	ctx->set = NULL;

	return 0;
}

static int set_key_data_error(struct eval_ctx *ctx, const struct set *set,
			      const struct datatype *dtype,
			      const char *name)
{
	const char *hint = "";

	if (dtype->size == 0)
		hint = ". Try \"typeof expression\" instead of \"type datatype\".";

	return set_error(ctx, set, "unqualified type %s "
			 "specified in %s definition%s",
			 dtype->name, name, hint);
}

static int set_expr_evaluate_concat(struct eval_ctx *ctx, struct expr **expr)
{
	unsigned int flags = EXPR_F_CONSTANT | EXPR_F_SINGLETON;
	uint32_t ntype = 0, size = 0;
	struct expr *i, *next;

	list_for_each_entry_safe(i, next, &expr_concat(*expr)->expressions, list) {
		unsigned dsize_bytes;

		if (i->etype == EXPR_CT &&
		    (i->ct.key == NFT_CT_SRC ||
		     i->ct.key == NFT_CT_DST))
			return expr_error(ctx->msgs, i,
					  "specify either ip or ip6 for address matching");

		if (i->etype == EXPR_PAYLOAD &&
		    i->dtype->type == TYPE_INTEGER) {
			struct datatype *dtype;

			dtype = datatype_clone(i->dtype);
			dtype->size = i->len;
			dtype->byteorder = i->byteorder;
			__datatype_set(i, dtype);
		}

		if (i->dtype->size == 0 && i->len == 0)
			return expr_binary_error(ctx->msgs, i, *expr,
						 "can not use variable sized "
						 "data types (%s) in concat "
						 "expressions",
						 i->dtype->name);

		flags &= i->flags;

		ntype = concat_subtype_add(ntype, i->dtype->type);

		dsize_bytes = div_round_up(i->len, BITS_PER_BYTE);

		if (i->dtype->size)
			assert(dsize_bytes == div_round_up(i->dtype->size, BITS_PER_BYTE));

		expr_concat(*expr)->field_len[expr_concat(*expr)->field_count++] = dsize_bytes;
		size += netlink_padded_len(i->len);

		if (size > NFT_MAX_EXPR_LEN_BITS)
			return expr_error(ctx->msgs, i, "Concatenation of size %u exceeds maximum size of %lu",
					  size, NFT_MAX_EXPR_LEN_BITS);
	}

	(*expr)->flags |= flags;
	__datatype_set(*expr, concat_type_alloc(ntype));
	(*expr)->len   = size;

	expr_set_context(&ctx->ectx, (*expr)->dtype, (*expr)->len);
	ctx->ectx.key = *expr;

	return 0;
}

static int elems_evaluate(struct eval_ctx *ctx, struct set *set)
{
	ctx->set = set;
	if (set->init != NULL) {
		if (set->key == NULL)
			return set_error(ctx, set, "set definition does not specify key");

		__expr_set_context(&ctx->ectx, set->key->dtype,
				   set->key->byteorder, set->key->len, 0);
		if (expr_evaluate(ctx, &set->init) < 0) {
			set->errors = true;
			return -1;
		}
		if (set->init->etype != EXPR_SET)
			return expr_error(ctx->msgs, set->init, "Set %s: Unexpected initial type %s, missing { }?",
					  set->handle.set.name, expr_name(set->init));
	}

	if (set_is_interval(ctx->set->flags) &&
	    !(ctx->set->flags & NFT_SET_CONCAT) &&
	    interval_set_eval(ctx, ctx->set, set->init) < 0)
		return -1;

	ctx->set = NULL;

	return 0;
}

static bool set_type_compatible(const struct set *set, const struct set *existing_set)
{
	if (set_is_datamap(set->flags))
		return set_is_datamap(existing_set->flags);

	if (set_is_objmap(set->flags))
		return set_is_objmap(existing_set->flags);

	assert(!set_is_map(set->flags));
	return !set_is_map(existing_set->flags);
}

static int set_evaluate(struct eval_ctx *ctx, struct set *set)
{
	struct set *existing_set = NULL;
	unsigned int num_stmts = 0;
	struct table *table;
	struct stmt *stmt;
	const char *type;

	type = set_is_map(set->flags) ? "map" : "set";

	if (set->key == NULL)
		return set_error(ctx, set, "%s definition does not specify key",
				 type);

	if (!set_is_anonymous(set->flags)) {
		table = table_cache_find(&ctx->nft->cache.table_cache,
					 set->handle.table.name,
					 set->handle.family);
		if (table == NULL)
			return table_not_found(ctx);

		existing_set = set_cache_find(table, set->handle.set.name);
		if (existing_set) {
			if (existing_set->flags & NFT_SET_EVAL) {
				uint32_t existing_flags = existing_set->flags & ~NFT_SET_EVAL;
				uint32_t new_flags = set->flags & ~NFT_SET_EVAL;

				if (existing_flags == new_flags)
					set->flags |= NFT_SET_EVAL;
			}
		} else {
			set_cache_add(set_get(set), table);
		}
	}

	if (!(set->flags & NFT_SET_INTERVAL) && set->automerge)
		return set_error(ctx, set, "auto-merge only works with interval sets");

	if (set->key == NULL)
		return set_error(ctx, set, "%s definition does not specify key",
				 type);

	if (set->key->len == 0) {
		if (set->key->etype == EXPR_CONCAT &&
		    set_expr_evaluate_concat(ctx, &set->key) < 0)
			return -1;

		if (set->key->len == 0)
			return set_key_data_error(ctx, set,
						  set->key->dtype, type);
	}

	if (set->flags & NFT_SET_INTERVAL && set->key->etype == EXPR_CONCAT) {
		memcpy(&set->desc.field_len, &expr_concat(set->key)->field_len,
		       sizeof(set->desc.field_len));
		set->desc.field_count = expr_concat(set->key)->field_count;
		set->flags |= NFT_SET_CONCAT;

		if (set->automerge)
			set->automerge = false;
	}

	if (set_is_anonymous(set->flags) && set->key->etype == EXPR_CONCAT) {
		struct expr *i;

		list_for_each_entry(i, &expr_set(set->init)->expressions, list) {
			if ((i->etype == EXPR_SET_ELEM &&
			     i->key->etype != EXPR_CONCAT &&
			     i->key->etype != EXPR_SET_ELEM_CATCHALL) ||
			    (i->etype == EXPR_MAPPING &&
			     i->left->etype == EXPR_SET_ELEM &&
			     i->left->key->etype != EXPR_CONCAT &&
			     i->left->key->etype != EXPR_SET_ELEM_CATCHALL))
				return expr_error(ctx->msgs, i, "expression is not a concatenation");
		}
	}

	if (set_is_datamap(set->flags)) {
		if (set->data == NULL)
			return set_error(ctx, set, "map definition does not "
					 "specify mapping data type");

		if (set->data->etype == EXPR_CONCAT &&
		    set_expr_evaluate_concat(ctx, &set->data) < 0)
			return -1;

		if (set->data->flags & EXPR_F_INTERVAL)
			set->data->len *= 2;

		if (set->data->len == 0 && set->data->dtype->type != TYPE_VERDICT)
			return set_key_data_error(ctx, set,
						  set->data->dtype, type);
	} else if (set_is_objmap(set->flags)) {
		assert(set->data == NULL);
		set->data = constant_expr_alloc(&netlink_location, &string_type,
						BYTEORDER_HOST_ENDIAN,
						NFT_OBJ_MAXNAMELEN * BITS_PER_BYTE,
						NULL);

	}

	/* Default timeout value implies timeout support */
	if (set->timeout)
		set->flags |= NFT_SET_TIMEOUT;

	list_for_each_entry(stmt, &set->stmt_list, list) {
		if (stmt_evaluate_stateful(ctx, stmt,type) < 0)
			return -1;
		num_stmts++;
	}

	if (num_stmts > 1)
		set->flags |= NFT_SET_EXPR;

	if (set_is_anonymous(set->flags)) {
		if (set->init->etype == EXPR_SET &&
		    set_is_interval(expr_set(set->init)->set_flags) &&
		    !(expr_set(set->init)->set_flags & NFT_SET_CONCAT) &&
		    interval_set_eval(ctx, set, set->init) < 0)
			return -1;

		return 0;
	}

	if (existing_set) {
		if (set_is_interval(set->flags) && !set_is_interval(existing_set->flags))
			return set_error(ctx, set,
					 "existing %s lacks interval flag", type);
		if (set->data && existing_set->data &&
		    !datatype_equal(existing_set->data->dtype, set->data->dtype))
			return set_error(ctx, set,
					 "%s already exists with different datatype (%s vs %s)",
					 type, existing_set->data->dtype->desc,
					 set->data->dtype->desc);
		if (!datatype_equal(existing_set->key->dtype, set->key->dtype))
			return set_error(ctx, set,
					 "%s already exists with different datatype (%s vs %s)",
					 type, existing_set->key->dtype->desc,
					 set->key->dtype->desc);
		/* Catch attempt to merge set and map */
		if (!set_type_compatible(set, existing_set))
			return set_error(ctx, set, "Cannot merge %s with incompatible existing %s of same name",
					type,
					set_is_map(existing_set->flags) ? "map" : "set");
	}

	set->existing_set = existing_set;

	return 0;
}

static bool evaluate_priority(struct eval_ctx *ctx, struct prio_spec *prio,
			      int family, int hook)
{
	char prio_str[NFT_NAME_MAXLEN];
	char prio_fst[NFT_NAME_MAXLEN];
	struct location loc;
	int priority;
	int prio_snd;
	char op;

	expr_set_context(&ctx->ectx, &priority_type, NFT_NAME_MAXLEN * BITS_PER_BYTE);

	if (expr_evaluate(ctx, &prio->expr) < 0)
		return false;
	if (prio->expr->etype != EXPR_VALUE) {
		expr_error(ctx->msgs, prio->expr, "%s is not a valid "
			   "priority expression", expr_name(prio->expr));
		return false;
	}
	if (prio->expr->dtype->type == TYPE_INTEGER)
		return true;

	mpz_export_data(prio_str, prio->expr->value, BYTEORDER_HOST_ENDIAN,
			NFT_NAME_MAXLEN);
	loc = prio->expr->location;

	if (sscanf(prio_str, "%255s %c %d", prio_fst, &op, &prio_snd) < 3) {
		priority = std_prio_lookup(prio_str, family, hook);
		if (priority == NF_IP_PRI_LAST)
			return false;
	} else {
		priority = std_prio_lookup(prio_fst, family, hook);
		if (priority == NF_IP_PRI_LAST)
			return false;
		if (op == '+')
			priority += prio_snd;
		else if (op == '-')
			priority -= prio_snd;
		else
			return false;
	}
	expr_free(prio->expr);
	prio->expr = constant_expr_alloc(&loc, &integer_type,
					 BYTEORDER_HOST_ENDIAN,
					 sizeof(int) * BITS_PER_BYTE,
					 &priority);
	return true;
}

static bool evaluate_expr_variable(struct eval_ctx *ctx, struct expr **exprp)
{
	struct expr *expr;

	if (expr_evaluate(ctx, exprp) < 0)
		return false;

	expr = *exprp;
	if (expr->etype != EXPR_VALUE &&
	    expr->etype != EXPR_SET) {
		expr_error(ctx->msgs, expr, "%s is not a valid "
			   "variable expression", expr_name(expr));
		return false;
	}

	return true;
}

static struct expr *expr_set_to_list(struct eval_ctx *ctx, struct expr *dev_expr)
{
	struct expr *expr, *next, *key;
	struct location loc;
	LIST_HEAD(tmp);

	list_for_each_entry_safe(expr, next, &expr_set(dev_expr)->expressions, list) {
		list_del(&expr->list);

		switch (expr->etype) {
		case EXPR_SET_ELEM:
			key = expr_clone(expr->key);
			expr_free(expr);
			expr = key;
			break;
		default:
			BUG("invalid expression type %s", expr_name(expr));
			break;
		}

		list_add(&expr->list, &tmp);
	}

	loc = dev_expr->location;
	expr_free(dev_expr);
	dev_expr = list_expr_alloc(&loc);
	list_splice_init(&tmp, &expr_list(dev_expr)->expressions);

	return dev_expr;
}

static bool evaluate_device_expr(struct eval_ctx *ctx, struct expr **dev_expr)
{
	struct expr *expr, *next;
	LIST_HEAD(tmp);

	if ((*dev_expr)->etype == EXPR_VARIABLE) {
		expr_set_context(&ctx->ectx, &ifname_type,
				 IFNAMSIZ * BITS_PER_BYTE);
		if (!evaluate_expr_variable(ctx, dev_expr))
			return false;
	}

	if ((*dev_expr)->etype == EXPR_SET)
		*dev_expr = expr_set_to_list(ctx, *dev_expr);

	assert((*dev_expr)->etype == EXPR_LIST);

	list_for_each_entry_safe(expr, next, &expr_list(*dev_expr)->expressions, list) {
		list_del(&expr->list);

		switch (expr->etype) {
		case EXPR_VARIABLE:
			expr_set_context(&ctx->ectx, &ifname_type,
					 IFNAMSIZ * BITS_PER_BYTE);
			if (!evaluate_expr_variable(ctx, &expr))
				return false;

			if (expr->etype == EXPR_SET) {
				expr = expr_set_to_list(ctx, expr);
				list_splice_init(&expr_list(expr)->expressions, &tmp);
				expr_free(expr);
				continue;
			}
			break;
		case EXPR_VALUE:
			break;
		default:
			BUG("invalid expression type %s", expr_name(expr));
			break;
		}

		list_add(&expr->list, &tmp);
	}
	list_splice_init(&tmp, &expr_list(*dev_expr)->expressions);

	return true;
}

static uint32_t str2hooknum(uint32_t family, const char *hook);

static int flowtable_evaluate(struct eval_ctx *ctx, struct flowtable *ft)
{
	struct table *table;

	table = table_cache_find(&ctx->nft->cache.table_cache,
				 ctx->cmd->handle.table.name,
				 ctx->cmd->handle.family);
	if (table == NULL)
		return table_not_found(ctx);

	if (!ft_cache_find(table, ft->handle.flowtable.name)) {
		if (!ft->hook.name && !ft->dev_expr)
			return chain_error(ctx, ft, "missing hook and priority in flowtable declaration");

		ft_cache_add(flowtable_get(ft), table);
	}

	if (ft->hook.name) {
		ft->hook.num = str2hooknum(NFPROTO_NETDEV, ft->hook.name);
		if (ft->hook.num == NF_INET_NUMHOOKS)
			return chain_error(ctx, ft, "invalid hook %s",
					   ft->hook.name);
		if (!evaluate_priority(ctx, &ft->priority, NFPROTO_NETDEV, ft->hook.num))
			return __stmt_binary_error(ctx, &ft->priority.loc, NULL,
						   "invalid priority expression %s.",
						   expr_name(ft->priority.expr));
	}

	if (ft->dev_expr && !evaluate_device_expr(ctx, &ft->dev_expr))
		return -1;

	return 0;
}

/* make src point at dst, either via handle.position or handle.position_id */
static void link_rules(struct rule *src, struct rule *dst)
{
	static uint32_t ref_id = 0;

	if (dst->handle.handle.id) {
		/* dst is in kernel, make src reference it by handle */
		src->handle.position.id = dst->handle.handle.id;
		src->handle.position.location = src->handle.index.location;
		return;
	}

	/* dst is not in kernel, make src reference it by per-transaction ID */
	if (!dst->handle.rule_id)
		dst->handle.rule_id = ++ref_id;
	src->handle.position_id = dst->handle.rule_id;
}

static int rule_cache_update(struct eval_ctx *ctx, enum cmd_ops op)
{
	struct rule *rule = ctx->rule, *ref = NULL;
	struct table *table;
	struct chain *chain;

	table = table_cache_find(&ctx->nft->cache.table_cache,
				 rule->handle.table.name,
				 rule->handle.family);
	if (!table)
		return table_not_found(ctx);

	/* chain is anonymous, adding new rules via index is not supported. */
	if (!rule->handle.chain.name)
		return 0;

	chain = chain_cache_find(table, rule->handle.chain.name);
	if (!chain)
		return chain_not_found(ctx);

	if (rule->handle.index.id) {
		ref = rule_lookup_by_index(chain, rule->handle.index.id);
		if (!ref)
			return cmd_error(ctx, &rule->handle.index.location,
					 "Could not process rule: %s",
					 strerror(ENOENT));

		link_rules(rule, ref);
	} else if (rule->handle.handle.id) {
		ref = rule_lookup(chain, rule->handle.handle.id);
		if (!ref)
			return cmd_error(ctx, &rule->handle.handle.location,
					 "Could not process rule: %s",
					 strerror(ENOENT));
	} else if (rule->handle.position.id) {
		ref = rule_lookup(chain, rule->handle.position.id);
		if (!ref)
			return cmd_error(ctx, &rule->handle.position.location,
					 "Could not process rule: %s",
					 strerror(ENOENT));
	}

	switch (op) {
	case CMD_INSERT:
		rule_get(rule);
		if (ref)
			list_add_tail(&rule->list, &ref->list);
		else
			list_add(&rule->list, &chain->rules);
		break;
	case CMD_ADD:
		rule_get(rule);
		if (ref)
			list_add(&rule->list, &ref->list);
		else
			list_add_tail(&rule->list, &chain->rules);
		break;
	case CMD_REPLACE:
		rule_get(rule);
		list_add(&rule->list, &ref->list);
		/* fall through */
	case CMD_DELETE:
		list_del(&ref->list);
		rule_free(ref);
		break;
	default:
		break;
	}
	return 0;
}

static int rule_evaluate(struct eval_ctx *ctx, struct rule *rule,
			 enum cmd_ops op)
{
	struct stmt *stmt, *tstmt = NULL;
	struct error_record *erec;

	proto_ctx_init(&ctx->_pctx[0], rule->handle.family, ctx->nft->debug_mask, false);
	/* use NFPROTO_BRIDGE to set up proto_eth as base protocol. */
	proto_ctx_init(&ctx->_pctx[1], NFPROTO_BRIDGE, ctx->nft->debug_mask, true);
	memset(&ctx->ectx, 0, sizeof(ctx->ectx));

	ctx->rule = rule;
	list_for_each_entry(stmt, &rule->stmts, list) {
		if (tstmt != NULL)
			return stmt_binary_error(ctx, stmt, tstmt,
						 "Statement after terminal "
						 "statement has no effect");

		ctx->stmt = stmt;
		if (stmt_evaluate(ctx, stmt) < 0)
			return -1;
		if (stmt->flags & STMT_F_TERMINAL)
			tstmt = stmt;

		ctx->inner_desc = NULL;
	}

	erec = rule_postprocess(rule);
	if (erec != NULL) {
		erec_queue(erec, ctx->msgs);
		return -1;
	}

	if (nft_cache_needs_update(&ctx->nft->cache))
		return rule_cache_update(ctx, op);

	return 0;
}

static uint32_t str2hooknum(uint32_t family, const char *hook)
{
	if (!hook)
		return NF_INET_NUMHOOKS;

	switch (family) {
	case NFPROTO_INET:
		if (!strcmp(hook, "ingress"))
			return NF_INET_INGRESS;
		/* fall through */
	case NFPROTO_IPV4:
	case NFPROTO_BRIDGE:
	case NFPROTO_IPV6:
		/* These families have overlapping values for each hook */
		if (!strcmp(hook, "prerouting"))
			return NF_INET_PRE_ROUTING;
		else if (!strcmp(hook, "input"))
			return NF_INET_LOCAL_IN;
		else if (!strcmp(hook, "forward"))
			return NF_INET_FORWARD;
		else if (!strcmp(hook, "postrouting"))
			return NF_INET_POST_ROUTING;
		else if (!strcmp(hook, "output"))
			return NF_INET_LOCAL_OUT;
		break;
	case NFPROTO_ARP:
		if (!strcmp(hook, "input"))
			return NF_ARP_IN;
		else if (!strcmp(hook, "forward"))
			return NF_ARP_FORWARD;
		else if (!strcmp(hook, "output"))
			return NF_ARP_OUT;
		break;
	case NFPROTO_NETDEV:
		if (!strcmp(hook, "ingress"))
			return NF_NETDEV_INGRESS;
		else if (!strcmp(hook, "egress"))
			return NF_NETDEV_EGRESS;
		break;
	default:
		break;
	}

	return NF_INET_NUMHOOKS;
}

static int chain_evaluate(struct eval_ctx *ctx, struct chain *chain)
{
	struct table *table;

	table = table_cache_find(&ctx->nft->cache.table_cache,
				 ctx->cmd->handle.table.name,
				 ctx->cmd->handle.family);
	if (table == NULL)
		return table_not_found(ctx);

	if (chain == NULL) {
		if (!chain_cache_find(table, ctx->cmd->handle.chain.name)) {
			chain = chain_alloc();
			handle_merge(&chain->handle, &ctx->cmd->handle);
			chain_cache_add(chain, table);
		}
		return 0;
	} else if (!(chain->flags & CHAIN_F_BINDING)) {
		if (!chain_cache_find(table, chain->handle.chain.name))
			chain_cache_add(chain_get(chain), table);
	}

	if (chain->flags & CHAIN_F_BASECHAIN) {
		if (chain->hook.name) {
			chain->hook.num = str2hooknum(chain->handle.family,
						      chain->hook.name);
			if (chain->hook.num == NF_INET_NUMHOOKS)
				return __stmt_binary_error(ctx, &chain->hook.loc, NULL,
							   "The %s family does not support this hook",
							   family2str(chain->handle.family));
		}
		if (chain->priority.expr) {
			if (!evaluate_priority(ctx, &chain->priority,
					       chain->handle.family, chain->hook.num))
				return __stmt_binary_error(ctx, &chain->priority.loc, NULL,
							   "invalid priority expression %s in this context.",
							   expr_name(chain->priority.expr));
		}
		if (chain->policy) {
			expr_set_context(&ctx->ectx, &policy_type,
					 NFT_NAME_MAXLEN * BITS_PER_BYTE);
			if (!evaluate_expr_variable(ctx, &chain->policy))
				return chain_error(ctx, chain, "invalid policy expression %s",
						   expr_name(chain->policy));
		}
	}

	if (chain->dev_expr) {
		if (!(chain->flags & CHAIN_F_BASECHAIN))
			chain->flags |= CHAIN_F_BASECHAIN;

		if (chain->handle.family == NFPROTO_NETDEV ||
		    (chain->handle.family == NFPROTO_INET &&
		     chain->hook.num == NF_INET_INGRESS)) {
			if (chain->dev_expr &&
			    !evaluate_device_expr(ctx, &chain->dev_expr))
				return -1;
		} else if (chain->dev_expr) {
			return __stmt_binary_error(ctx, &chain->dev_expr->location, NULL,
						   "This chain type cannot be bound to device");
		}
	}

	return 0;
}

static int ct_expect_evaluate(struct eval_ctx *ctx, struct obj *obj)
{
	struct ct_expect *ct = &obj->ct_expect;

	if (!ct->l4proto ||
	    !ct->dport ||
	    !ct->timeout ||
	    !ct->size)
		return __stmt_binary_error(ctx, &obj->location, NULL,
					   "missing options");

	return 0;
}

static int ct_timeout_evaluate(struct eval_ctx *ctx, struct obj *obj)
{
	struct ct_timeout *ct = &obj->ct_timeout;
	struct timeout_state *ts, *next;
	unsigned int i;

	for (i = 0; i < timeout_protocol[ct->l4proto].array_size; i++)
		ct->timeout[i] = timeout_protocol[ct->l4proto].dflt_timeout[i];

	list_for_each_entry_safe(ts, next, &ct->timeout_list, head) {
		if (timeout_str2num(ct->l4proto, ts) < 0)
			return __stmt_binary_error(ctx, &ts->location, NULL,
						   "invalid state for this protocol");

		ct->timeout[ts->timeout_index] = ts->timeout_value;
		list_del(&ts->head);
		free_const(ts->timeout_str);
		free(ts);
	}

	return 0;
}

static int tunnel_evaluate_addr(struct eval_ctx *ctx, struct expr **exprp)
{
	struct expr *e;
	int ret;

	ret = expr_evaluate(ctx, exprp);
	if (ret < 0)
		return ret;

	e = *exprp;
	if (e->etype != EXPR_VALUE)
		return expr_error(ctx->msgs, e, "must be a value, not %s", expr_name(e));

	return 0;
}

static int tunnel_evaluate(struct eval_ctx *ctx, struct obj *obj)
{
	if (obj->tunnel.src) {
		expr_set_context(&ctx->ectx, obj->tunnel.src->dtype,
				 obj->tunnel.src->dtype->size);
		if (tunnel_evaluate_addr(ctx, &obj->tunnel.src) < 0)
			return -1;
	}

	if (obj->tunnel.dst) {
		expr_set_context(&ctx->ectx, obj->tunnel.dst->dtype,
				 obj->tunnel.dst->dtype->size);
		if (tunnel_evaluate_addr(ctx, &obj->tunnel.dst) < 0)
			return -1;

		if (obj->tunnel.src &&
		    obj->tunnel.src->dtype != obj->tunnel.dst->dtype)
			return __stmt_binary_error(ctx, &obj->location, NULL,
						  "specify either ip or ip6 for address");
	}

	return 0;
}

static int obj_evaluate(struct eval_ctx *ctx, struct obj *obj)
{
	struct table *table;

	table = table_cache_find(&ctx->nft->cache.table_cache,
				 ctx->cmd->handle.table.name,
				 ctx->cmd->handle.family);
	if (!table)
		return table_not_found(ctx);

	if (!obj_cache_find(table, obj->handle.obj.name, obj->type))
		obj_cache_add(obj_get(obj), table);

	switch (obj->type) {
	case NFT_OBJECT_CT_TIMEOUT:
		return ct_timeout_evaluate(ctx, obj);
	case NFT_OBJECT_CT_EXPECT:
		return ct_expect_evaluate(ctx, obj);
	case NFT_OBJECT_TUNNEL:
		return tunnel_evaluate(ctx, obj);
	default:
		break;
	}

	return 0;
}

static int table_evaluate(struct eval_ctx *ctx, struct table *table)
{
	if (!table_cache_find(&ctx->nft->cache.table_cache,
			      ctx->cmd->handle.table.name,
			      ctx->cmd->handle.family)) {
		if (!table) {
			table = table_alloc();
			handle_merge(&table->handle, &ctx->cmd->handle);
			table_cache_add(table, &ctx->nft->cache);
		} else {
			table_cache_add(table_get(table), &ctx->nft->cache);
		}
	}

	return 0;
}

static int cmd_evaluate_add(struct eval_ctx *ctx, struct cmd *cmd)
{
	switch (cmd->obj) {
	case CMD_OBJ_ELEMENTS:
		return setelem_evaluate(ctx, cmd);
	case CMD_OBJ_SET:
		handle_merge(&cmd->set->handle, &cmd->handle);
		return set_evaluate(ctx, cmd->set);
	case CMD_OBJ_SETELEMS:
		return elems_evaluate(ctx, cmd->set);
	case CMD_OBJ_RULE:
		handle_merge(&cmd->rule->handle, &cmd->handle);
		return rule_evaluate(ctx, cmd->rule, cmd->op);
	case CMD_OBJ_CHAIN:
		return chain_evaluate(ctx, cmd->chain);
	case CMD_OBJ_TABLE:
		return table_evaluate(ctx, cmd->table);
	case CMD_OBJ_FLOWTABLE:
		handle_merge(&cmd->flowtable->handle, &cmd->handle);
		return flowtable_evaluate(ctx, cmd->flowtable);
	case CMD_OBJ_COUNTER:
	case CMD_OBJ_QUOTA:
	case CMD_OBJ_CT_HELPER:
	case CMD_OBJ_LIMIT:
	case CMD_OBJ_CT_TIMEOUT:
	case CMD_OBJ_SECMARK:
	case CMD_OBJ_CT_EXPECT:
	case CMD_OBJ_SYNPROXY:
	case CMD_OBJ_TUNNEL:
		handle_merge(&cmd->object->handle, &cmd->handle);
		return obj_evaluate(ctx, cmd->object);
	default:
		BUG("invalid command object type %u", cmd->obj);
	}
}

static void table_del_cache(struct eval_ctx *ctx, struct cmd *cmd)
{
	struct table *table;

	if (!cmd->handle.table.name)
		return;

	table = table_cache_find(&ctx->nft->cache.table_cache,
				 cmd->handle.table.name,
				 cmd->handle.family);
	if (!table)
		return;

	table_cache_del(table);
	table_free(table);
}

static void chain_del_cache(struct eval_ctx *ctx, struct cmd *cmd)
{
	struct table *table;
	struct chain *chain;

	if (!cmd->handle.chain.name)
		return;

	table = table_cache_find(&ctx->nft->cache.table_cache,
				 cmd->handle.table.name,
				 cmd->handle.family);
	if (!table)
		return;

	chain = chain_cache_find(table, cmd->handle.chain.name);
	if (!chain)
		return;

	chain_cache_del(chain);
	chain_free(chain);
}

static int chain_del_evaluate(struct eval_ctx *ctx, struct cmd *cmd)
{
	struct chain *chain = cmd->chain;

	if (chain && chain->flags & CHAIN_F_BASECHAIN && chain->hook.name) {
		chain->hook.num = str2hooknum(chain->handle.family,
					      chain->hook.name);
		if (chain->hook.num == NF_INET_NUMHOOKS)
			return __stmt_binary_error(ctx, &chain->hook.loc, NULL,
						   "The %s family does not support this hook",
						   family2str(chain->handle.family));
	}
	chain_del_cache(ctx, cmd);
	return 0;
}

static void set_del_cache(struct eval_ctx *ctx, struct cmd *cmd)
{
	struct table *table;
	struct set *set;

	if (!cmd->handle.set.name)
		return;

	table = table_cache_find(&ctx->nft->cache.table_cache,
				 cmd->handle.table.name,
				 cmd->handle.family);
	if (!table)
		return;

	set = set_cache_find(table, cmd->handle.set.name);
	if (!set)
		return;

	set_cache_del(set);
	set_free(set);
}

static void ft_del_cache(struct eval_ctx *ctx, struct cmd *cmd)
{
	struct flowtable *ft;
	struct table *table;

	if (!cmd->handle.flowtable.name)
		return;

	table = table_cache_find(&ctx->nft->cache.table_cache,
				 cmd->handle.table.name,
				 cmd->handle.family);
	if (!table)
		return;

	ft = ft_cache_find(table, cmd->handle.flowtable.name);
	if (!ft)
		return;

	ft_cache_del(ft);
	flowtable_free(ft);
}

static void obj_del_cache(struct eval_ctx *ctx, struct cmd *cmd, int type)
{
	struct table *table;
	struct obj *obj;

	if (!cmd->handle.obj.name)
		return;

	table = table_cache_find(&ctx->nft->cache.table_cache,
				 cmd->handle.table.name,
				 cmd->handle.family);
	if (!table)
		return;

	obj = obj_cache_find(table, cmd->handle.obj.name, type);
	if (!obj)
		return;

	obj_cache_del(obj);
	obj_free(obj);
}

static int cmd_evaluate_delete(struct eval_ctx *ctx, struct cmd *cmd)
{
	switch (cmd->obj) {
	case CMD_OBJ_ELEMENTS:
		return setelem_evaluate(ctx, cmd);
	case CMD_OBJ_SET:
		set_del_cache(ctx, cmd);
		return 0;
	case CMD_OBJ_RULE:
		return 0;
	case CMD_OBJ_CHAIN:
		return chain_del_evaluate(ctx, cmd);
	case CMD_OBJ_TABLE:
		table_del_cache(ctx, cmd);
		return 0;
	case CMD_OBJ_FLOWTABLE:
		ft_del_cache(ctx, cmd);
		return 0;
	case CMD_OBJ_COUNTER:
		obj_del_cache(ctx, cmd, NFT_OBJECT_COUNTER);
		return 0;
	case CMD_OBJ_QUOTA:
		obj_del_cache(ctx, cmd, NFT_OBJECT_QUOTA);
		return 0;
	case CMD_OBJ_CT_HELPER:
		obj_del_cache(ctx, cmd, NFT_OBJECT_CT_HELPER);
		return 0;
	case CMD_OBJ_CT_TIMEOUT:
		obj_del_cache(ctx, cmd, NFT_OBJECT_CT_TIMEOUT);
		return 0;
	case CMD_OBJ_LIMIT:
		obj_del_cache(ctx, cmd, NFT_OBJECT_LIMIT);
		return 0;
	case CMD_OBJ_SECMARK:
		obj_del_cache(ctx, cmd, NFT_OBJECT_SECMARK);
		return 0;
	case CMD_OBJ_CT_EXPECT:
		obj_del_cache(ctx, cmd, NFT_OBJECT_CT_EXPECT);
		return 0;
	case CMD_OBJ_SYNPROXY:
		obj_del_cache(ctx, cmd, NFT_OBJECT_SYNPROXY);
		return 0;
	case CMD_OBJ_TUNNEL:
		obj_del_cache(ctx, cmd, NFT_OBJECT_TUNNEL);
		return 0;
	default:
		BUG("invalid command object type %u", cmd->obj);
	}
}

static int cmd_evaluate_get(struct eval_ctx *ctx, struct cmd *cmd)
{
	switch (cmd->obj) {
	case CMD_OBJ_ELEMENTS:
		return setelem_evaluate(ctx, cmd);
	default:
		BUG("invalid command object type %u", cmd->obj);
	}
}

static int obj_not_found(struct eval_ctx *ctx, const struct location *loc,
			 const char *obj_name)
{
	const struct table *table;
	struct obj *obj;

	obj = obj_lookup_fuzzy(obj_name, &ctx->nft->cache, &table);
	if (obj == NULL)
		return cmd_error(ctx, loc, "%s", strerror(ENOENT));

	return cmd_error(ctx, loc,
			 "%s; did you mean obj '%s' in table %s '%s'?",
			 strerror(ENOENT), obj->handle.obj.name,
				 family2str(obj->handle.family),
				 table->handle.table.name);
}

static int cmd_evaluate_list_obj(struct eval_ctx *ctx, const struct cmd *cmd,
				 uint32_t obj_type)
{
	const struct table *table;

	if (obj_type == NFT_OBJECT_UNSPEC)
		obj_type = NFT_OBJECT_COUNTER;

	table = table_cache_find(&ctx->nft->cache.table_cache,
				 cmd->handle.table.name,
				 cmd->handle.family);
	if (table == NULL)
		return table_not_found(ctx);

	if (!obj_cache_find(table, cmd->handle.obj.name, obj_type))
		return obj_not_found(ctx, &cmd->handle.obj.location,
				     cmd->handle.obj.name);

	return 0;
}

static int cmd_evaluate_list(struct eval_ctx *ctx, struct cmd *cmd)
{
	struct flowtable *ft;
	struct table *table;
	struct set *set;

	switch (cmd->obj) {
	case CMD_OBJ_TABLE:
		if (cmd->handle.table.name == NULL)
			return 0;

		table = table_cache_find(&ctx->nft->cache.table_cache,
					 cmd->handle.table.name,
					 cmd->handle.family);
		if (!table)
			return table_not_found(ctx);

		return 0;
	case CMD_OBJ_SET:
	case CMD_OBJ_MAP:
	case CMD_OBJ_METER:
		table = table_cache_find(&ctx->nft->cache.table_cache,
					 cmd->handle.table.name,
					 cmd->handle.family);
		if (!table)
			return table_not_found(ctx);

		set = set_cache_find(table, cmd->handle.set.name);
		if (set == NULL)
			return set_not_found(ctx, &ctx->cmd->handle.set.location,
					     ctx->cmd->handle.set.name);
		if ((cmd->obj == CMD_OBJ_SET && !set_is_literal(set->flags)) ||
		    (cmd->obj == CMD_OBJ_MAP && !map_is_literal(set->flags)) ||
		    (cmd->obj == CMD_OBJ_METER && !set_is_meter_compat(set->flags)))
			return cmd_error(ctx, &ctx->cmd->handle.set.location,
					 "%s", strerror(ENOENT));

		cmd->set = set_get(set);
		return 0;
	case CMD_OBJ_CHAIN:
		table = table_cache_find(&ctx->nft->cache.table_cache,
					 cmd->handle.table.name,
					 cmd->handle.family);
		if (!table)
			return table_not_found(ctx);

		if (!chain_cache_find(table, cmd->handle.chain.name))
			return chain_not_found(ctx);

		return 0;
	case CMD_OBJ_FLOWTABLE:
		table = table_cache_find(&ctx->nft->cache.table_cache,
					 cmd->handle.table.name,
					 cmd->handle.family);
		if (!table)
			return table_not_found(ctx);

		ft = ft_cache_find(table, cmd->handle.flowtable.name);
		if (!ft)
			return flowtable_not_found(ctx, &ctx->cmd->handle.flowtable.location,
						   ctx->cmd->handle.flowtable.name);

		return 0;
	case CMD_OBJ_QUOTA:
		return cmd_evaluate_list_obj(ctx, cmd, NFT_OBJECT_QUOTA);
	case CMD_OBJ_COUNTER:
		return cmd_evaluate_list_obj(ctx, cmd, NFT_OBJECT_COUNTER);
	case CMD_OBJ_CT_HELPER:
		return cmd_evaluate_list_obj(ctx, cmd, NFT_OBJECT_CT_HELPER);
	case CMD_OBJ_CT_TIMEOUT:
		return cmd_evaluate_list_obj(ctx, cmd, NFT_OBJECT_CT_TIMEOUT);
	case CMD_OBJ_LIMIT:
		return cmd_evaluate_list_obj(ctx, cmd, NFT_OBJECT_LIMIT);
	case CMD_OBJ_SECMARK:
		return cmd_evaluate_list_obj(ctx, cmd, NFT_OBJECT_SECMARK);
	case CMD_OBJ_CT_EXPECT:
		return cmd_evaluate_list_obj(ctx, cmd, NFT_OBJECT_CT_EXPECT);
	case CMD_OBJ_SYNPROXY:
		return cmd_evaluate_list_obj(ctx, cmd, NFT_OBJECT_SYNPROXY);
	case CMD_OBJ_TUNNEL:
		return cmd_evaluate_list_obj(ctx, cmd, NFT_OBJECT_TUNNEL);
	case CMD_OBJ_COUNTERS:
	case CMD_OBJ_QUOTAS:
	case CMD_OBJ_CT_HELPERS:
	case CMD_OBJ_LIMITS:
	case CMD_OBJ_SETS:
	case CMD_OBJ_FLOWTABLES:
	case CMD_OBJ_SECMARKS:
	case CMD_OBJ_SYNPROXYS:
	case CMD_OBJ_CT_TIMEOUTS:
	case CMD_OBJ_CT_EXPECTATIONS:
	case CMD_OBJ_TUNNELS:
		if (cmd->handle.table.name == NULL)
			return 0;
		if (!table_cache_find(&ctx->nft->cache.table_cache,
				      cmd->handle.table.name,
				      cmd->handle.family))
			return table_not_found(ctx);

		return 0;
	case CMD_OBJ_CHAINS:
	case CMD_OBJ_RULESET:
	case CMD_OBJ_METERS:
	case CMD_OBJ_MAPS:
		return 0;
	case CMD_OBJ_HOOKS:
		if (cmd->handle.chain.name) {
			int hooknum = str2hooknum(cmd->handle.family, cmd->handle.chain.name);

			if (hooknum == NF_INET_NUMHOOKS)
				return chain_not_found(ctx);

			cmd->handle.chain_id = hooknum;
		}
		return 0;
	default:
		BUG("invalid command object type %u", cmd->obj);
	}
}

static int cmd_evaluate_reset(struct eval_ctx *ctx, struct cmd *cmd)
{
	switch (cmd->obj) {
	case CMD_OBJ_COUNTER:
	case CMD_OBJ_QUOTA:
	case CMD_OBJ_COUNTERS:
	case CMD_OBJ_QUOTAS:
	case CMD_OBJ_RULES:
	case CMD_OBJ_RULE:
		if (cmd->handle.table.name == NULL)
			return 0;
		if (!table_cache_find(&ctx->nft->cache.table_cache,
				      cmd->handle.table.name,
				      cmd->handle.family))
			return table_not_found(ctx);

		return 0;
	case CMD_OBJ_ELEMENTS:
		return setelem_evaluate(ctx, cmd);
	case CMD_OBJ_TABLE:
	case CMD_OBJ_CHAIN:
	case CMD_OBJ_SET:
	case CMD_OBJ_MAP:
		return cmd_evaluate_list(ctx, cmd);
	default:
		BUG("invalid command object type %u", cmd->obj);
	}
}

static void __flush_set_cache(struct set *set)
{
	if (set->init != NULL) {
		expr_free(set->init);
		set->init = NULL;
	}
}

static int cmd_evaluate_flush(struct eval_ctx *ctx, struct cmd *cmd)
{
	struct cache *table_cache = &ctx->nft->cache.table_cache;
	struct table *table;
	struct set *set;

	switch (cmd->obj) {
	case CMD_OBJ_RULESET:
		break;
	case CMD_OBJ_TABLE:
		/* Flushing a table does not empty the sets in the table nor remove
		 * any chains.
		 */
	case CMD_OBJ_CHAIN:
		/* Chains don't hold sets */
		break;
	case CMD_OBJ_SET:
		table = table_cache_find(table_cache, cmd->handle.table.name,
					 cmd->handle.family);
		if (!table)
			return table_not_found(ctx);

		set = set_cache_find(table, cmd->handle.set.name);
		if (set == NULL)
			return set_not_found(ctx, &ctx->cmd->handle.set.location,
					     ctx->cmd->handle.set.name);
		else if (!set_is_literal(set->flags))
			return cmd_error(ctx, &ctx->cmd->handle.set.location,
					 "%s", strerror(ENOENT));

		__flush_set_cache(set);

		return 0;
	case CMD_OBJ_MAP:
		table = table_cache_find(table_cache, cmd->handle.table.name,
					 cmd->handle.family);
		if (!table)
			return table_not_found(ctx);

		set = set_cache_find(table, cmd->handle.set.name);
		if (set == NULL)
			return set_not_found(ctx, &ctx->cmd->handle.set.location,
					     ctx->cmd->handle.set.name);
		else if (!map_is_literal(set->flags))
			return cmd_error(ctx, &ctx->cmd->handle.set.location,
					 "%s", strerror(ENOENT));

		__flush_set_cache(set);

		return 0;
	case CMD_OBJ_METER:
		table = table_cache_find(table_cache, cmd->handle.table.name,
					 cmd->handle.family);
		if (!table)
			return table_not_found(ctx);

		set = set_cache_find(table, cmd->handle.set.name);
		if (set == NULL)
			return set_not_found(ctx, &ctx->cmd->handle.set.location,
					     ctx->cmd->handle.set.name);
		else if (!set_is_meter_compat(set->flags))
			return cmd_error(ctx, &ctx->cmd->handle.set.location,
					 "%s", strerror(ENOENT));

		__flush_set_cache(set);

		return 0;
	default:
		BUG("invalid command object type %u", cmd->obj);
	}
	return 0;
}

static int cmd_evaluate_rename(struct eval_ctx *ctx, struct cmd *cmd)
{
	struct table *table;

	switch (cmd->obj) {
	case CMD_OBJ_CHAIN:
		table = table_cache_find(&ctx->nft->cache.table_cache,
					 cmd->handle.table.name,
					 cmd->handle.family);
		if (!table)
			return table_not_found(ctx);

		if (!chain_cache_find(table, ctx->cmd->handle.chain.name))
			return chain_not_found(ctx);

		break;
	default:
		BUG("invalid command object type %u", cmd->obj);
	}
	return 0;
}

enum {
	CMD_MONITOR_EVENT_ANY,
	CMD_MONITOR_EVENT_NEW,
	CMD_MONITOR_EVENT_DEL,
	CMD_MONITOR_EVENT_MAX
};

static uint32_t monitor_flags[CMD_MONITOR_EVENT_MAX][CMD_MONITOR_OBJ_MAX] = {
	[CMD_MONITOR_EVENT_ANY] = {
		[CMD_MONITOR_OBJ_ANY]		= 0xffffffff,
		[CMD_MONITOR_OBJ_TABLES]	= (1 << NFT_MSG_NEWTABLE) |
						  (1 << NFT_MSG_DELTABLE),
		[CMD_MONITOR_OBJ_CHAINS]	= (1 << NFT_MSG_NEWCHAIN) |
						  (1 << NFT_MSG_DELCHAIN),
		[CMD_MONITOR_OBJ_RULES]		= (1 << NFT_MSG_NEWRULE) |
						  (1 << NFT_MSG_DELRULE),
		[CMD_MONITOR_OBJ_SETS]		= (1 << NFT_MSG_NEWSET) |
						  (1 << NFT_MSG_DELSET),
		[CMD_MONITOR_OBJ_ELEMS]		= (1 << NFT_MSG_NEWSETELEM) |
						  (1 << NFT_MSG_DELSETELEM),
		[CMD_MONITOR_OBJ_RULESET]	= (1 << NFT_MSG_NEWTABLE) |
						  (1 << NFT_MSG_DELTABLE) |
						  (1 << NFT_MSG_NEWCHAIN) |
						  (1 << NFT_MSG_DELCHAIN) |
						  (1 << NFT_MSG_NEWRULE)  |
						  (1 << NFT_MSG_DELRULE)  |
						  (1 << NFT_MSG_NEWSET)   |
						  (1 << NFT_MSG_DELSET)	  |
						  (1 << NFT_MSG_NEWSETELEM) |
						  (1 << NFT_MSG_DELSETELEM) |
						  (1 << NFT_MSG_NEWOBJ)	  |
						  (1 << NFT_MSG_DELOBJ),
		[CMD_MONITOR_OBJ_TRACE]		= (1 << NFT_MSG_TRACE),
	},
	[CMD_MONITOR_EVENT_NEW] = {
		[CMD_MONITOR_OBJ_ANY]		= (1 << NFT_MSG_NEWTABLE) |
						  (1 << NFT_MSG_NEWCHAIN) |
						  (1 << NFT_MSG_NEWRULE)  |
						  (1 << NFT_MSG_NEWSET)   |
						  (1 << NFT_MSG_NEWSETELEM),
		[CMD_MONITOR_OBJ_TABLES]	= (1 << NFT_MSG_NEWTABLE),
		[CMD_MONITOR_OBJ_CHAINS]	= (1 << NFT_MSG_NEWCHAIN),
		[CMD_MONITOR_OBJ_RULES]		= (1 << NFT_MSG_NEWRULE),
		[CMD_MONITOR_OBJ_SETS]		= (1 << NFT_MSG_NEWSET),
		[CMD_MONITOR_OBJ_ELEMS]		= (1 << NFT_MSG_NEWSETELEM),
		[CMD_MONITOR_OBJ_RULESET]	= (1 << NFT_MSG_NEWTABLE) |
						  (1 << NFT_MSG_NEWCHAIN) |
						  (1 << NFT_MSG_NEWRULE)  |
						  (1 << NFT_MSG_NEWSET)   |
						  (1 << NFT_MSG_NEWSETELEM) |
						  (1 << NFT_MSG_NEWOBJ),
		[CMD_MONITOR_OBJ_TRACE]		= 0,
	},
	[CMD_MONITOR_EVENT_DEL] = {
		[CMD_MONITOR_OBJ_ANY]		= (1 << NFT_MSG_DELTABLE) |
						  (1 << NFT_MSG_DELCHAIN) |
						  (1 << NFT_MSG_DELRULE)  |
						  (1 << NFT_MSG_DELSET)   |
						  (1 << NFT_MSG_DELSETELEM),
		[CMD_MONITOR_OBJ_TABLES]	= (1 << NFT_MSG_DELTABLE),
		[CMD_MONITOR_OBJ_CHAINS]	= (1 << NFT_MSG_DELCHAIN),
		[CMD_MONITOR_OBJ_RULES]		= (1 << NFT_MSG_DELRULE),
		[CMD_MONITOR_OBJ_SETS]		= (1 << NFT_MSG_DELSET),
		[CMD_MONITOR_OBJ_ELEMS]		= (1 << NFT_MSG_DELSETELEM),
		[CMD_MONITOR_OBJ_RULESET]	= (1 << NFT_MSG_DELTABLE) |
						  (1 << NFT_MSG_DELCHAIN) |
						  (1 << NFT_MSG_DELRULE)  |
						  (1 << NFT_MSG_DELSET)   |
						  (1 << NFT_MSG_DELSETELEM) |
						  (1 << NFT_MSG_DELOBJ),
		[CMD_MONITOR_OBJ_TRACE]		= 0,
	},
};

static int cmd_evaluate_monitor(struct eval_ctx *ctx, struct cmd *cmd)
{
	uint32_t event;

	if (cmd->monitor->event == NULL)
		event = CMD_MONITOR_EVENT_ANY;
	else if (strcmp(cmd->monitor->event, "new") == 0)
		event = CMD_MONITOR_EVENT_NEW;
	else if (strcmp(cmd->monitor->event, "destroy") == 0)
		event = CMD_MONITOR_EVENT_DEL;
	else {
		return monitor_error(ctx, cmd->monitor, "invalid event %s",
				     cmd->monitor->event);
	}

	cmd->monitor->flags = monitor_flags[event][cmd->monitor->type];
	return 0;
}

static int cmd_evaluate_export(struct eval_ctx *ctx, struct cmd *cmd)
{
	if (cmd->markup->format == __NFT_OUTPUT_NOTSUPP)
		return cmd_error(ctx, &cmd->location,
				 "this output type is not supported, use nft -j list ruleset for JSON support instead");
	else if (cmd->markup->format == NFTNL_OUTPUT_JSON)
		return cmd_error(ctx, &cmd->location,
				 "JSON export is no longer supported, use 'nft -j list ruleset' instead");

	return 0;
}

static int cmd_evaluate_import(struct eval_ctx *ctx, struct cmd *cmd)
{
	if (cmd->markup->format == __NFT_OUTPUT_NOTSUPP)
		return cmd_error(ctx, &cmd->location,
				 "this output type not supported");

	return 0;
}

static const char * const cmd_op_name[] = {
	[CMD_INVALID]	= "invalid",
	[CMD_ADD]	= "add",
	[CMD_REPLACE]	= "replace",
	[CMD_CREATE]	= "create",
	[CMD_INSERT]	= "insert",
	[CMD_DELETE]	= "delete",
	[CMD_GET]	= "get",
	[CMD_LIST]	= "list",
	[CMD_FLUSH]	= "flush",
	[CMD_RENAME]	= "rename",
	[CMD_EXPORT]	= "export",
	[CMD_MONITOR]	= "monitor",
	[CMD_DESCRIBE]	= "describe",
	[CMD_DESTROY]   = "destroy",
};

static const char *cmd_op_to_name(enum cmd_ops op)
{
	if (op >= array_size(cmd_op_name))
		return "unknown";

	return cmd_op_name[op];
}

int cmd_evaluate(struct eval_ctx *ctx, struct cmd *cmd)
{
	if (ctx->nft->debug_mask & NFT_DEBUG_EVALUATION) {
		struct error_record *erec;

		erec = erec_create(EREC_INFORMATIONAL, &cmd->location,
				   "Evaluate %s", cmd_op_to_name(cmd->op));
		erec_print(&ctx->nft->output, erec, ctx->nft->debug_mask);
		nft_print(&ctx->nft->output, "\n\n");
		erec_destroy(erec);
	}

	memset(&ctx->ectx, 0, sizeof(ctx->ectx));

	ctx->cmd = cmd;
	switch (cmd->op) {
	case CMD_ADD:
	case CMD_REPLACE:
	case CMD_CREATE:
	case CMD_INSERT:
		return cmd_evaluate_add(ctx, cmd);
	case CMD_DELETE:
	case CMD_DESTROY:
		return cmd_evaluate_delete(ctx, cmd);
	case CMD_GET:
		return cmd_evaluate_get(ctx, cmd);
	case CMD_LIST:
		return cmd_evaluate_list(ctx, cmd);
	case CMD_RESET:
		return cmd_evaluate_reset(ctx, cmd);
	case CMD_FLUSH:
		return cmd_evaluate_flush(ctx, cmd);
	case CMD_RENAME:
		return cmd_evaluate_rename(ctx, cmd);
	case CMD_EXPORT:
		return cmd_evaluate_export(ctx, cmd);
	case CMD_DESCRIBE:
		return 0;
	case CMD_MONITOR:
		return cmd_evaluate_monitor(ctx, cmd);
	case CMD_IMPORT:
		return cmd_evaluate_import(ctx, cmd);
	case CMD_INVALID:
		break;
	};

	BUG("invalid command operation %u", cmd->op);
}
