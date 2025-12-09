/*
 * Copyright (c) 2008-2012 Patrick McHardy <kaber@trash.net>
 * Copyright (c) 2013 Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Development of this code funded by Astaro AG (http://www.astaro.com/)
 */

#include <nft.h>
#include <nftversion.h>

#include <errno.h>
#include <libmnl/libmnl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <inttypes.h>

#include <libnftnl/table.h>
#include <libnftnl/chain.h>
#include <libnftnl/expr.h>
#include <libnftnl/object.h>
#include <libnftnl/set.h>
#include <libnftnl/flowtable.h>
#include <libnftnl/udata.h>
#include <libnftnl/ruleset.h>
#include <libnftnl/common.h>
#include <libnftnl/udata.h>
#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nf_tables.h>
#include <linux/netfilter.h>

#include <nftables.h>
#include <parser.h>
#include <netlink.h>
#include <mnl.h>
#include <expression.h>
#include <statement.h>
#include <gmputil.h>
#include <utils.h>
#include <erec.h>

#define nft_mon_print(monh, ...) nft_print(&monh->ctx->nft->output, __VA_ARGS__)

const struct input_descriptor indesc_netlink = {
	.name	= "netlink",
	.type	= INDESC_NETLINK,
};

const struct location netlink_location = {
	.indesc	= &indesc_netlink,
};

void __noreturn __netlink_abi_error(const char *file, int line,
				    const char *reason)
{
	fprintf(stderr, "E: Contact urgently your Linux kernel vendor. "
		"Netlink ABI is broken: %s:%d %s\n", file, line, reason);
	abort();
}

int netlink_io_error(struct netlink_ctx *ctx, const struct location *loc,
		     const char *fmt, ...)
{
	struct error_record *erec;
	va_list ap;

	if (loc == NULL)
		loc = &netlink_location;

	va_start(ap, fmt);
	erec = erec_vcreate(EREC_ERROR, loc, fmt, ap);
	va_end(ap);
	erec_queue(erec, ctx->msgs);
	return -1;
}

void __noreturn __netlink_init_error(const char *filename, int line,
				     const char *reason)
{
	fprintf(stderr, "%s:%d: Unable to initialize Netlink socket: %s\n",
		filename, line, reason);
	exit(NFT_EXIT_NONL);
}

struct nftnl_expr *alloc_nft_expr(const char *name)
{
	struct nftnl_expr *nle;

	nle = nftnl_expr_alloc(name);
	if (nle == NULL)
		memory_allocation_error();

	return nle;
}
static void netlink_gen_key(const struct expr *expr,
			    struct nft_data_linearize *data);
static void __netlink_gen_data(const struct expr *expr,
			       struct nft_data_linearize *data, bool expand);

struct nftnl_set_elem *alloc_nftnl_setelem(const struct expr *set,
					   const struct expr *expr)
{
	const struct expr *elem, *data;
	struct nftnl_set_elem *nlse;
	struct nft_data_linearize nld;
	struct nftnl_udata_buf *udbuf = NULL;
	uint32_t flags = 0;
	int num_exprs = 0;
	struct stmt *stmt;
	struct expr *key;

	nlse = nftnl_set_elem_alloc();
	if (nlse == NULL)
		memory_allocation_error();

	data = NULL;
	if (expr->etype == EXPR_MAPPING) {
		elem = expr->left;
		if (!(expr->flags & EXPR_F_INTERVAL_END))
			data = expr->right;
	} else {
		elem = expr;
	}
	if (elem->etype != EXPR_SET_ELEM)
		BUG("Unexpected expression type: got %d", elem->etype);

	key = elem->key;

	switch (key->etype) {
	case EXPR_SET_ELEM_CATCHALL:
		break;
	default:
		if (expr_set(set)->set_flags & NFT_SET_INTERVAL &&
		    key->etype == EXPR_CONCAT && expr_concat(key)->field_count > 1) {
			key->flags |= EXPR_F_INTERVAL;
			netlink_gen_key(key, &nld);
			key->flags &= ~EXPR_F_INTERVAL;

			nftnl_set_elem_set(nlse, NFTNL_SET_ELEM_KEY, nld.value, nld.len);

			key->flags |= EXPR_F_INTERVAL_END;
			netlink_gen_key(key, &nld);
			key->flags &= ~EXPR_F_INTERVAL_END;

			nftnl_set_elem_set(nlse, NFTNL_SET_ELEM_KEY_END,
					   nld.value, nld.len);
		} else {
			netlink_gen_key(key, &nld);
			nftnl_set_elem_set(nlse, NFTNL_SET_ELEM_KEY, nld.value, nld.len);
		}
		break;
	}

	if (elem->timeout) {
		uint64_t timeout = elem->timeout;

		if (elem->timeout == NFT_NEVER_TIMEOUT)
			timeout = 0;

		nftnl_set_elem_set_u64(nlse, NFTNL_SET_ELEM_TIMEOUT, timeout);
	}
	if (elem->expiration)
		nftnl_set_elem_set_u64(nlse, NFTNL_SET_ELEM_EXPIRATION,
				       elem->expiration);
	list_for_each_entry(stmt, &elem->stmt_list, list)
		num_exprs++;

	if (num_exprs == 1) {
		list_for_each_entry(stmt, &elem->stmt_list, list) {
			nftnl_set_elem_set(nlse, NFTNL_SET_ELEM_EXPR,
					   netlink_gen_stmt_stateful(stmt), 0);
		}
	} else if (num_exprs > 1) {
		list_for_each_entry(stmt, &elem->stmt_list, list) {
			nftnl_set_elem_add_expr(nlse,
						netlink_gen_stmt_stateful(stmt));
		}
	}
	if (elem->comment || expr->flags & EXPR_F_INTERVAL_OPEN) {
		udbuf = nftnl_udata_buf_alloc(NFT_USERDATA_MAXLEN);
		if (!udbuf)
			memory_allocation_error();
	}
	if (elem->comment) {
		if (!nftnl_udata_put_strz(udbuf, NFTNL_UDATA_SET_ELEM_COMMENT,
					  elem->comment))
			memory_allocation_error();
	}
	if (expr->flags & EXPR_F_INTERVAL_OPEN) {
		if (!nftnl_udata_put_u32(udbuf, NFTNL_UDATA_SET_ELEM_FLAGS,
					 NFTNL_SET_ELEM_F_INTERVAL_OPEN))
			memory_allocation_error();
	}
	if (udbuf) {
		nftnl_set_elem_set(nlse, NFTNL_SET_ELEM_USERDATA,
				   nftnl_udata_buf_data(udbuf),
				   nftnl_udata_buf_len(udbuf));
		nftnl_udata_buf_free(udbuf);
	}
	if (set_is_datamap(expr_set(set)->set_flags) && data != NULL) {
		__netlink_gen_data(data, &nld, !(data->flags & EXPR_F_SINGLETON));
		switch (data->etype) {
		case EXPR_VERDICT:
			nftnl_set_elem_set_u32(nlse, NFTNL_SET_ELEM_VERDICT,
					       data->verdict);
			if (data->chain != NULL)
				nftnl_set_elem_set(nlse, NFTNL_SET_ELEM_CHAIN,
						   nld.chain, strlen(nld.chain));
			break;
		case EXPR_CONCAT:
			assert(nld.len > 0);
			/* fallthrough */
		case EXPR_VALUE:
		case EXPR_RANGE:
		case EXPR_RANGE_VALUE:
		case EXPR_PREFIX:
			nftnl_set_elem_set(nlse, NFTNL_SET_ELEM_DATA,
					   nld.value, nld.len);
			break;
		default:
			BUG("unexpected set element expression");
			break;
		}
	}
	if (set_is_objmap(expr_set(set)->set_flags) && data != NULL) {
		netlink_gen_data(data, &nld);
		nftnl_set_elem_set(nlse, NFTNL_SET_ELEM_OBJREF,
				   nld.value, nld.len);
	}

	if (expr->flags & EXPR_F_INTERVAL_END)
		flags |= NFT_SET_ELEM_INTERVAL_END;
	if (key->etype == EXPR_SET_ELEM_CATCHALL)
		flags |= NFT_SET_ELEM_CATCHALL;

	if (flags)
		nftnl_set_elem_set_u32(nlse, NFTNL_SET_ELEM_FLAGS, flags);

	return nlse;
}

void netlink_gen_raw_data(const mpz_t value, enum byteorder byteorder,
			  unsigned int len, struct nft_data_linearize *data)
{
	assert(len > 0);
	mpz_export_data(data->value, value, byteorder, len);
	data->len = len;
}

static int netlink_export_pad(unsigned char *data, const mpz_t v,
			      const struct expr *i)
{
	mpz_export_data(data, v, i->byteorder,
			div_round_up(i->len, BITS_PER_BYTE));

	return netlink_padded_len(i->len) / BITS_PER_BYTE;
}

static void byteorder_switch_expr_value(mpz_t v, const struct expr *e)
{
	mpz_switch_byteorder(v, div_round_up(e->len, BITS_PER_BYTE));
}

static int __netlink_gen_concat_key(uint32_t flags, const struct expr *i,
				    unsigned char *data)
{
	struct expr *expr;
	mpz_t value;
	int ret;

	switch (i->etype) {
	case EXPR_RANGE:
		if (flags & EXPR_F_INTERVAL_END)
			expr = i->right;
		else
			expr = i->left;

		mpz_init_set(value, expr->value);

		if (expr_basetype(expr)->type == TYPE_INTEGER &&
		    expr->byteorder == BYTEORDER_HOST_ENDIAN)
			byteorder_switch_expr_value(value, expr);

		i = expr;
		break;
	case EXPR_RANGE_VALUE:
		if (flags & EXPR_F_INTERVAL_END)
			mpz_init_set(value, i->range.high);
		else
			mpz_init_set(value, i->range.low);

		if (expr_basetype(i)->type == TYPE_INTEGER &&
		    i->byteorder == BYTEORDER_HOST_ENDIAN)
			byteorder_switch_expr_value(value, i);

		break;
	case EXPR_PREFIX:
		if (flags & EXPR_F_INTERVAL_END) {
			int count;
			mpz_t v;

			mpz_init_bitmask(v, i->len - i->prefix_len);

			if (i->byteorder == BYTEORDER_HOST_ENDIAN)
				byteorder_switch_expr_value(v, i);

			mpz_add(v, i->prefix->value, v);
			count = netlink_export_pad(data, v, i);
			mpz_clear(v);
			return count;
		}
		return netlink_export_pad(data, i->prefix->value, i);
	case EXPR_VALUE:
		mpz_init_set(value, i->value);

		/* Switch byteorder to big endian representation when the set
		 * contains concatenation of intervals.
		 */
		if (!(flags & (EXPR_F_INTERVAL| EXPR_F_INTERVAL_END)))
			break;

		expr = (struct expr *)i;
		if (expr_basetype(expr)->type == TYPE_INTEGER &&
		    expr->byteorder == BYTEORDER_HOST_ENDIAN)
			byteorder_switch_expr_value(value, expr);
		break;
	default:
		BUG("invalid expression type '%s' in set", expr_ops(i)->name);
	}

	ret = netlink_export_pad(data, value, i);
	mpz_clear(value);

	return ret;
}

static void nft_data_memcpy(struct nft_data_linearize *nld,
			    const void *src, unsigned int len)
{
	if (len > sizeof(nld->value))
		BUG("nld buffer overflow: want to copy %u, max %u",
		    len, (unsigned int)sizeof(nld->value));

	memcpy(nld->value, src, len);
	nld->len = len;
}

static void netlink_gen_concat_key(const struct expr *expr,
				    struct nft_data_linearize *nld)
{
	unsigned int len = netlink_padded_len(expr->len) / BITS_PER_BYTE;
	unsigned char data[NFT_MAX_EXPR_LEN_BYTES];
	unsigned int offset = 0;
	const struct expr *i;

	if (len > sizeof(data))
		BUG("Value export of %u bytes would overflow", len);

	memset(data, 0, sizeof(data));

	list_for_each_entry(i, &expr_concat(expr)->expressions, list)
		offset += __netlink_gen_concat_key(expr->flags, i, data + offset);

	nft_data_memcpy(nld, data, len);
}

static int __netlink_gen_concat_data(int end, const struct expr *i,
				     unsigned char *data)
{
	mpz_t value;
	int ret;

	switch (i->etype) {
	case EXPR_RANGE:
		if (end)
			i = i->right;
		else
			i = i->left;

		mpz_init_set(value, i->value);
		break;
	case EXPR_RANGE_VALUE:
		if (end)
			mpz_init_set(value, i->range.high);
		else
			mpz_init_set(value, i->range.low);
		break;
	case EXPR_PREFIX:
		if (end) {
			int count;

			mpz_init_bitmask(value, i->len - i->prefix_len);
			mpz_add(value, i->prefix->value, value);
			count = netlink_export_pad(data, value, i);
			mpz_clear(value);
			return count;
		}
		return netlink_export_pad(data, i->prefix->value, i);
	case EXPR_VALUE:
		mpz_init_set(value, i->value);
		break;
	default:
		BUG("invalid expression type '%s' in set", expr_ops(i)->name);
	}

	ret = netlink_export_pad(data, value, i);
	mpz_clear(value);

	return ret;
}

static void __netlink_gen_concat_expand(const struct expr *expr,
				        struct nft_data_linearize *nld)
{
	unsigned int len = (netlink_padded_len(expr->len) / BITS_PER_BYTE) * 2;
	unsigned char data[NFT_MAX_EXPR_LEN_BYTES];
	unsigned int offset = 0;
	const struct expr *i;

	if (len > sizeof(data))
		BUG("Value export of %u bytes would overflow", len);

	memset(data, 0, sizeof(data));

	list_for_each_entry(i, &expr_concat(expr)->expressions, list)
		offset += __netlink_gen_concat_data(false, i, data + offset);

	list_for_each_entry(i, &expr_concat(expr)->expressions, list)
		offset += __netlink_gen_concat_data(true, i, data + offset);

	nft_data_memcpy(nld, data, len);
}

static void __netlink_gen_concat(const struct expr *expr,
				 struct nft_data_linearize *nld)
{
	unsigned int len = netlink_padded_len(expr->len) / BITS_PER_BYTE;
	unsigned char data[NFT_MAX_EXPR_LEN_BYTES];
	unsigned int offset = 0;
	const struct expr *i;

	if (len > sizeof(data))
		BUG("Value export of %u bytes would overflow", len);

	memset(data, 0, sizeof(data));

	list_for_each_entry(i, &expr_concat(expr)->expressions, list)
		offset += __netlink_gen_concat_data(expr->flags, i, data + offset);

	nft_data_memcpy(nld, data, len);
}

static void netlink_gen_concat_data(const struct expr *expr,
				    struct nft_data_linearize *nld, bool expand)
{
	if (expand)
		__netlink_gen_concat_expand(expr, nld);
	else
		__netlink_gen_concat(expr, nld);
}

static void netlink_gen_constant_data(const struct expr *expr,
				      struct nft_data_linearize *data)
{
	assert(expr->etype == EXPR_VALUE);
	netlink_gen_raw_data(expr->value, expr->byteorder,
			     div_round_up(expr->len, BITS_PER_BYTE), data);
}

static void netlink_gen_chain(const struct expr *expr,
			      struct nft_data_linearize *data)
{
	char chain[NFT_CHAIN_MAXNAMELEN];
	unsigned int len;

	len = expr->chain->len / BITS_PER_BYTE;

	if (!len)
		BUG("chain length is 0");

	if (len > sizeof(chain))
		BUG("chain is too large (%u, %u max)",
		    len, (unsigned int)sizeof(chain));

	memset(chain, 0, sizeof(chain));

	mpz_export_data(chain, expr->chain->value,
			BYTEORDER_HOST_ENDIAN, len);
	snprintf(data->chain, NFT_CHAIN_MAXNAMELEN, "%s", chain);
}

static void netlink_gen_verdict(const struct expr *expr,
				struct nft_data_linearize *data)
{

	data->verdict = expr->verdict;

	switch (expr->verdict) {
	case NFT_JUMP:
	case NFT_GOTO:
		if (expr->chain)
			netlink_gen_chain(expr, data);
		else
			data->chain_id = expr->chain_id;
		break;
	}
}

static void netlink_gen_range(const struct expr *expr,
			      struct nft_data_linearize *nld)
{
	unsigned int len = (netlink_padded_len(expr->left->len) / BITS_PER_BYTE) * 2;
	unsigned char data[NFT_MAX_EXPR_LEN_BYTES];
	unsigned int offset;

	if (len > sizeof(data))
		BUG("Value export of %u bytes would overflow", len);

	memset(data, 0, sizeof(data));
	offset = netlink_export_pad(data, expr->left->value, expr->left);
	netlink_export_pad(data + offset, expr->right->value, expr->right);
	nft_data_memcpy(nld, data, len);
}

static void netlink_gen_range_value(const struct expr *expr,
				    struct nft_data_linearize *nld)
{
	unsigned int len = (netlink_padded_len(expr->len) / BITS_PER_BYTE) * 2;
	unsigned char data[NFT_MAX_EXPR_LEN_BYTES];
	unsigned int offset;

	if (len > sizeof(data))
		BUG("Value export of %u bytes would overflow", len);

	memset(data, 0, sizeof(data));
	offset = netlink_export_pad(data, expr->range.low, expr);
	netlink_export_pad(data + offset, expr->range.high, expr);
	nft_data_memcpy(nld, data, len);
}

static void netlink_gen_prefix(const struct expr *expr,
			       struct nft_data_linearize *nld)
{
	unsigned int len = (netlink_padded_len(expr->len) / BITS_PER_BYTE) * 2;
	unsigned char data[NFT_MAX_EXPR_LEN_BYTES];
	int offset;
	mpz_t v;

	if (len > sizeof(data))
		BUG("Value export of %u bytes would overflow", len);

	offset = netlink_export_pad(data, expr->prefix->value, expr);
	mpz_init_bitmask(v, expr->len - expr->prefix_len);
	mpz_add(v, expr->prefix->value, v);
	netlink_export_pad(data + offset, v, expr->prefix);
	mpz_clear(v);

	nft_data_memcpy(nld, data, len);
}

static void netlink_gen_key(const struct expr *expr,
			    struct nft_data_linearize *data)
{
	memset(data, 0, sizeof(*data));

	switch (expr->etype) {
	case EXPR_VALUE:
		return netlink_gen_constant_data(expr, data);
	case EXPR_CONCAT:
		return netlink_gen_concat_key(expr, data);
	case EXPR_RANGE:
		return netlink_gen_range(expr, data);
	case EXPR_PREFIX:
		return netlink_gen_prefix(expr, data);
	default:
		BUG("invalid data expression type %s", expr_name(expr));
	}
}

static void __netlink_gen_data(const struct expr *expr,
			       struct nft_data_linearize *data, bool expand)
{
	memset(data, 0, sizeof(*data));

	switch (expr->etype) {
	case EXPR_VALUE:
		return netlink_gen_constant_data(expr, data);
	case EXPR_CONCAT:
		return netlink_gen_concat_data(expr, data, expand);
	case EXPR_VERDICT:
		return netlink_gen_verdict(expr, data);
	case EXPR_RANGE:
		return netlink_gen_range(expr, data);
	case EXPR_RANGE_VALUE:
		return netlink_gen_range_value(expr, data);
	case EXPR_PREFIX:
		return netlink_gen_prefix(expr, data);
	default:
		BUG("invalid data expression type %s", expr_name(expr));
	}
}

void netlink_gen_data(const struct expr *expr, struct nft_data_linearize *data)
{
	__netlink_gen_data(expr, data, false);
}

struct expr *netlink_alloc_value(const struct location *loc,
				 const struct nft_data_delinearize *nld)
{
	return constant_expr_alloc(loc, &invalid_type, BYTEORDER_INVALID,
				   nld->len * BITS_PER_BYTE, nld->value);
}

static struct expr *netlink_alloc_verdict(const struct location *loc,
					  const struct nft_data_delinearize *nld)
{
	struct expr *chain;

	switch (nld->verdict) {
	case NFT_JUMP:
	case NFT_GOTO:
		chain = constant_expr_alloc(loc, &string_type,
					    BYTEORDER_HOST_ENDIAN,
					    strlen(nld->chain) * BITS_PER_BYTE,
					    nld->chain);
		break;
	default:
		chain = NULL;
		break;
	}

	return verdict_expr_alloc(loc, nld->verdict, chain);
}

struct expr *netlink_alloc_data(const struct location *loc,
				const struct nft_data_delinearize *nld,
				enum nft_registers dreg)
{
	switch (dreg) {
	case NFT_REG_VERDICT:
		return netlink_alloc_verdict(loc, nld);
	default:
		return netlink_alloc_value(loc, nld);
	}
}

void netlink_dump_rule(const struct nftnl_rule *nlr, struct netlink_ctx *ctx)
{
	FILE *fp = ctx->nft->output.output_fp;

	if (!(ctx->nft->debug_mask & NFT_DEBUG_NETLINK) || !fp)
		return;

	nftnl_rule_fprintf(fp, nlr, 0, 0);
	fprintf(fp, "\n");
}

void netlink_dump_expr(const struct nftnl_expr *nle,
		       FILE *fp, unsigned int debug_mask)
{
	if (!(debug_mask & NFT_DEBUG_NETLINK))
		return;

	nftnl_expr_fprintf(fp, nle, 0, 0);
	fprintf(fp, "\n");
}

void netlink_dump_chain(const struct nftnl_chain *nlc, struct netlink_ctx *ctx)
{
	FILE *fp = ctx->nft->output.output_fp;

	if (!(ctx->nft->debug_mask & NFT_DEBUG_NETLINK) || !fp)
		return;

	nftnl_chain_fprintf(fp, nlc, 0, 0);
	fprintf(fp, "\n");
}

static int chain_parse_udata_cb(const struct nftnl_udata *attr, void *data)
{
	unsigned char *value = nftnl_udata_get(attr);
	uint8_t type = nftnl_udata_type(attr);
	const struct nftnl_udata **tb = data;
	uint8_t len = nftnl_udata_len(attr);

	switch (type) {
		case NFTNL_UDATA_CHAIN_COMMENT:
			if (value[len - 1] != '\0')
				return -1;
			break;
		default:
			return 0;
	}
	tb[type] = attr;
	return 0;
}

static int qsort_device_cmp(const void *a, const void *b)
{
	const char **x = (const char **)a;
	const char **y = (const char **)b;

	return strcmp(*x, *y);
}

struct chain *netlink_delinearize_chain(struct netlink_ctx *ctx,
					const struct nftnl_chain *nlc)
{
	const struct nftnl_udata *ud[NFTNL_UDATA_CHAIN_MAX + 1] = {};
	int priority, policy, len = 0, i;
	const char * const *dev_array;
	struct chain *chain;
	const char *udata;
	uint32_t ulen;

	chain = chain_alloc();
	chain->handle.family =
		nftnl_chain_get_u32(nlc, NFTNL_CHAIN_FAMILY);
	chain->handle.table.name  =
		xstrdup(nftnl_chain_get_str(nlc, NFTNL_CHAIN_TABLE));
	chain->handle.chain.name =
		xstrdup(nftnl_chain_get_str(nlc, NFTNL_CHAIN_NAME));
	chain->handle.handle.id =
		nftnl_chain_get_u64(nlc, NFTNL_CHAIN_HANDLE);
	if (nftnl_chain_is_set(nlc, NFTNL_CHAIN_FLAGS))
		chain->flags = nftnl_chain_get_u32(nlc, NFTNL_CHAIN_FLAGS);

	if (nftnl_chain_is_set(nlc, NFTNL_CHAIN_HOOKNUM) &&
	    nftnl_chain_is_set(nlc, NFTNL_CHAIN_PRIO) &&
	    nftnl_chain_is_set(nlc, NFTNL_CHAIN_TYPE) &&
	    nftnl_chain_is_set(nlc, NFTNL_CHAIN_POLICY)) {
		chain->hook.num =
			nftnl_chain_get_u32(nlc, NFTNL_CHAIN_HOOKNUM);
		chain->hook.name =
			hooknum2str(chain->handle.family, chain->hook.num);
		priority = nftnl_chain_get_s32(nlc, NFTNL_CHAIN_PRIO);
		chain->priority.expr =
				constant_expr_alloc(&netlink_location,
						    &integer_type,
						    BYTEORDER_HOST_ENDIAN,
						    sizeof(int) * BITS_PER_BYTE,
						    &priority);
		chain->type.str =
			xstrdup(nftnl_chain_get_str(nlc, NFTNL_CHAIN_TYPE));
		policy = nftnl_chain_get_u32(nlc, NFTNL_CHAIN_POLICY);
		chain->policy = constant_expr_alloc(&netlink_location,
						    &integer_type,
						    BYTEORDER_HOST_ENDIAN,
						    sizeof(int) * BITS_PER_BYTE,
						    &policy);
			nftnl_chain_get_u32(nlc, NFTNL_CHAIN_POLICY);
		if (nftnl_chain_is_set(nlc, NFTNL_CHAIN_DEV)) {
			chain->dev_array = xmalloc(sizeof(char *) * 2);
			chain->dev_array_len = 1;
			chain->dev_array[0] =
				xstrdup(nftnl_chain_get_str(nlc, NFTNL_CHAIN_DEV));
			chain->dev_array[1] = NULL;
		} else if (nftnl_chain_is_set(nlc, NFTNL_CHAIN_DEVICES)) {
			dev_array = nftnl_chain_get(nlc, NFTNL_CHAIN_DEVICES);
			while (dev_array[len])
				len++;

			chain->dev_array = xmalloc((len + 1)* sizeof(char *));
			for (i = 0; i < len; i++)
				chain->dev_array[i] = xstrdup(dev_array[i]);

			chain->dev_array[i] = NULL;
			chain->dev_array_len = len;
		}
		chain->flags        |= CHAIN_F_BASECHAIN;

		if (chain->dev_array_len) {
			qsort(chain->dev_array, chain->dev_array_len,
			      sizeof(char *), qsort_device_cmp);
		}
	}

	if (nftnl_chain_is_set(nlc, NFTNL_CHAIN_USERDATA)) {
		udata = nftnl_chain_get_data(nlc, NFTNL_CHAIN_USERDATA, &ulen);
		if (nftnl_udata_parse(udata, ulen, chain_parse_udata_cb, ud) < 0) {
			netlink_io_error(ctx, NULL, "Cannot parse userdata");
			chain_free(chain);
			return NULL;
		}
		if (ud[NFTNL_UDATA_CHAIN_COMMENT])
			chain->comment = xstrdup(nftnl_udata_get(ud[NFTNL_UDATA_CHAIN_COMMENT]));
	}

	return chain;
}

static int table_parse_udata_cb(const struct nftnl_udata *attr, void *data)
{
	unsigned char *value = nftnl_udata_get(attr);
	const struct nftnl_udata **tb = data;
	uint8_t type = nftnl_udata_type(attr);
	uint8_t len = nftnl_udata_len(attr);

	switch (type) {
		case NFTNL_UDATA_TABLE_COMMENT:
			if (value[len - 1] != '\0')
				return -1;
			break;
		case NFTNL_UDATA_TABLE_NFTVER:
			if (len != sizeof(nftversion))
				return -1;
			break;
		case NFTNL_UDATA_TABLE_NFTBLD:
			if (len != sizeof(nftbuildstamp))
				return -1;
			break;
		default:
			return 0;
	}
	tb[type] = attr;
	return 0;
}

static int version_cmp(const struct nftnl_udata **ud)
{
	const char *udbuf;
	size_t i;

	/* netlink attribute lengths checked by table_parse_udata_cb() */
	if (ud[NFTNL_UDATA_TABLE_NFTVER]) {
		udbuf = nftnl_udata_get(ud[NFTNL_UDATA_TABLE_NFTVER]);
		for (i = 0; i < sizeof(nftversion); i++) {
			if (nftversion[i] != udbuf[i])
				return nftversion[i] - udbuf[i];
		}
	}
	if (ud[NFTNL_UDATA_TABLE_NFTBLD]) {
		udbuf = nftnl_udata_get(ud[NFTNL_UDATA_TABLE_NFTBLD]);
		for (i = 0; i < sizeof(nftbuildstamp); i++) {
			if (nftbuildstamp[i] != udbuf[i])
				return nftbuildstamp[i] - udbuf[i];
		}
	}
	return 0;
}

struct table *netlink_delinearize_table(struct netlink_ctx *ctx,
					const struct nftnl_table *nlt)
{
	const struct nftnl_udata *ud[NFTNL_UDATA_TABLE_MAX + 1] = {};
	struct table *table;
	const char *udata;
	uint32_t ulen;

	table = table_alloc();
	table->handle.family = nftnl_table_get_u32(nlt, NFTNL_TABLE_FAMILY);
	table->handle.table.name = xstrdup(nftnl_table_get_str(nlt, NFTNL_TABLE_NAME));
	table->flags	     = nftnl_table_get_u32(nlt, NFTNL_TABLE_FLAGS);
	table->handle.handle.id = nftnl_table_get_u64(nlt, NFTNL_TABLE_HANDLE);
	table->owner	     = nftnl_table_get_u32(nlt, NFTNL_TABLE_OWNER);

	if (nftnl_table_is_set(nlt, NFTNL_TABLE_USERDATA)) {
		udata = nftnl_table_get_data(nlt, NFTNL_TABLE_USERDATA, &ulen);
		if (nftnl_udata_parse(udata, ulen, table_parse_udata_cb, ud) < 0) {
			netlink_io_error(ctx, NULL, "Cannot parse userdata");
			table_free(table);
			return NULL;
		}
		if (ud[NFTNL_UDATA_TABLE_COMMENT])
			table->comment = xstrdup(nftnl_udata_get(ud[NFTNL_UDATA_TABLE_COMMENT]));
		table->is_from_future = version_cmp(ud) < 0;
	}

	return table;
}

static int list_table_cb(struct nftnl_table *nlt, void *arg)
{
	struct netlink_ctx *ctx = arg;
	struct table *table;

	table = netlink_delinearize_table(ctx, nlt);
	if (table)
		list_add_tail(&table->list, &ctx->list);

	return 0;
}

int netlink_list_tables(struct netlink_ctx *ctx, const struct handle *h,
			const struct nft_cache_filter *filter)
{
	struct nftnl_table_list *table_cache;
	uint32_t family = h->family;
	const char *table = NULL;

	if (filter) {
		family = filter->list.family;
		table = filter->list.table;
	}

	table_cache = mnl_nft_table_dump(ctx, family, table);
	if (table_cache == NULL) {
		if (errno == EINTR)
			return -1;

		return -1;
	}

	ctx->data = h;
	nftnl_table_list_foreach(table_cache, list_table_cb, ctx);
	nftnl_table_list_free(table_cache);
	return 0;
}

enum nft_data_types dtype_map_to_kernel(const struct datatype *dtype)
{
	switch (dtype->type) {
	case TYPE_VERDICT:
		return NFT_DATA_VERDICT;
	default:
		return dtype->type;
	}
}

static const struct datatype *dtype_map_from_kernel(enum nft_data_types type)
{
	/* The function always returns ownership of a reference. But for
	 * &verdict_Type and datatype_lookup(), those are static instances,
	 * we can omit the datatype_get() call.
	 */
	switch (type) {
	case NFT_DATA_VERDICT:
		return &verdict_type;
	default:
		if (type & ~TYPE_MASK)
			return concat_type_alloc(type);
		return datatype_lookup((enum datatypes) type);
	}
}

void netlink_dump_set(const struct nftnl_set *nls, struct netlink_ctx *ctx)
{
	FILE *fp = ctx->nft->output.output_fp;
	uint32_t family;

	if (!(ctx->nft->debug_mask & NFT_DEBUG_NETLINK) || !fp)
		return;

	family = nftnl_set_get_u32(nls, NFTNL_SET_FAMILY);
	fprintf(fp, "family %d ", family);
	nftnl_set_fprintf(fp, nls, 0, 0);
	fprintf(fp, "\n");
}

static int set_parse_udata_cb(const struct nftnl_udata *attr, void *data)
{
	unsigned char *value = nftnl_udata_get(attr);
	const struct nftnl_udata **tb = data;
	uint8_t type = nftnl_udata_type(attr);
	uint8_t len = nftnl_udata_len(attr);

	switch (type) {
	case NFTNL_UDATA_SET_KEYBYTEORDER:
	case NFTNL_UDATA_SET_DATABYTEORDER:
	case NFTNL_UDATA_SET_MERGE_ELEMENTS:
	case NFTNL_UDATA_SET_DATA_INTERVAL:
		if (len != sizeof(uint32_t))
			return -1;
		break;
	case NFTNL_UDATA_SET_KEY_TYPEOF:
	case NFTNL_UDATA_SET_DATA_TYPEOF:
		if (len < 3)
			return -1;
		break;
	case NFTNL_UDATA_SET_COMMENT:
		if (value[len - 1] != '\0')
			return -1;
		break;
	default:
		return 0;
	}
	tb[type] = attr;
	return 0;
}

static int set_key_parse_udata(const struct nftnl_udata *attr, void *data)
{
	const struct nftnl_udata **tb = data;
	uint8_t type = nftnl_udata_type(attr);
	uint8_t len = nftnl_udata_len(attr);

	switch (type) {
	case NFTNL_UDATA_SET_TYPEOF_EXPR:
		if (len != sizeof(uint32_t))
			return -1;
		break;
	case NFTNL_UDATA_SET_TYPEOF_DATA:
		break;
	default:
		return 0;
	}
	tb[type] = attr;
	return 0;
}

static struct expr *set_make_key(const struct nftnl_udata *attr)
{
	const struct nftnl_udata *ud[NFTNL_UDATA_SET_TYPEOF_MAX + 1] = {};
	const struct expr_ops *ops;
	struct expr *expr;
	uint32_t etype;
	int err;

	if (!attr)
		return NULL;

	err = nftnl_udata_parse(nftnl_udata_get(attr), nftnl_udata_len(attr),
				set_key_parse_udata, ud);
	if (err < 0)
		return NULL;

	if (!ud[NFTNL_UDATA_SET_TYPEOF_EXPR] ||
	    !ud[NFTNL_UDATA_SET_TYPEOF_DATA])
		return NULL;

	etype = nftnl_udata_get_u32(ud[NFTNL_UDATA_SET_TYPEOF_EXPR]);
	ops = expr_ops_by_type_u32(etype);
	if (!ops || !ops->parse_udata)
		return NULL;

	expr = ops->parse_udata(ud[NFTNL_UDATA_SET_TYPEOF_DATA]);
	if (!expr)
		return NULL;

	return expr;
}

static bool set_udata_key_valid(const struct expr *e, uint32_t len)
{
	if (!e)
		return false;

	return div_round_up(e->len, BITS_PER_BYTE) == len / BITS_PER_BYTE;
}

struct setelem_parse_ctx {
	struct set			*set;
	struct nft_cache		*cache;
	struct list_head		stmt_list;
};

static int set_elem_parse_expressions(struct nftnl_expr *e, void *data)
{
	struct setelem_parse_ctx *setelem_parse_ctx = data;
	struct nft_cache *cache = setelem_parse_ctx->cache;
	struct set *set = setelem_parse_ctx->set;
	struct stmt *stmt;

	stmt = netlink_parse_set_expr(set, cache, e);
	if (stmt)
		list_add_tail(&stmt->list, &setelem_parse_ctx->stmt_list);

	return 0;
}

struct set *netlink_delinearize_set(struct netlink_ctx *ctx,
				    const struct nftnl_set *nls)
{
	const struct nftnl_udata *ud[NFTNL_UDATA_SET_MAX + 1] = {};
	enum byteorder keybyteorder = BYTEORDER_INVALID;
	enum byteorder databyteorder = BYTEORDER_INVALID;
	struct setelem_parse_ctx set_parse_ctx;
	const struct datatype *datatype = NULL;
	const struct datatype *keytype = NULL;
	const struct datatype *dtype2 = NULL;
	const struct datatype *dtype = NULL;
	struct expr *typeof_expr_data = NULL;
	struct expr *typeof_expr_key = NULL;
	const char *udata, *comment = NULL;
	uint32_t flags, key, objtype = 0;
	uint32_t data_interval = 0;
	bool automerge = false;
	struct set *set;
	uint32_t ulen;
	uint32_t klen;

	if (nftnl_set_is_set(nls, NFTNL_SET_USERDATA)) {
		udata = nftnl_set_get_data(nls, NFTNL_SET_USERDATA, &ulen);
		if (nftnl_udata_parse(udata, ulen, set_parse_udata_cb, ud) < 0) {
			netlink_io_error(ctx, NULL, "Cannot parse userdata");
			return NULL;
		}

#define GET_U32_UDATA(var, attr)				\
		if (ud[attr])					\
			var = nftnl_udata_get_u32(ud[attr])

		GET_U32_UDATA(keybyteorder, NFTNL_UDATA_SET_KEYBYTEORDER);
		GET_U32_UDATA(databyteorder, NFTNL_UDATA_SET_DATABYTEORDER);
		GET_U32_UDATA(automerge, NFTNL_UDATA_SET_MERGE_ELEMENTS);
		GET_U32_UDATA(data_interval, NFTNL_UDATA_SET_DATA_INTERVAL);

#undef GET_U32_UDATA
		typeof_expr_key = set_make_key(ud[NFTNL_UDATA_SET_KEY_TYPEOF]);
		if (ud[NFTNL_UDATA_SET_DATA_TYPEOF])
			typeof_expr_data = set_make_key(ud[NFTNL_UDATA_SET_DATA_TYPEOF]);
		if (ud[NFTNL_UDATA_SET_COMMENT])
			comment = nftnl_udata_get(ud[NFTNL_UDATA_SET_COMMENT]);
	}

	key = nftnl_set_get_u32(nls, NFTNL_SET_KEY_TYPE);
	keytype = dtype_map_from_kernel(key);
	if (keytype == NULL) {
		netlink_io_error(ctx, NULL, "Unknown data type in set key %u",
				 key);
		return NULL;
	}

	flags = nftnl_set_get_u32(nls, NFTNL_SET_FLAGS);
	if (set_is_datamap(flags)) {
		uint32_t data;

		data = nftnl_set_get_u32(nls, NFTNL_SET_DATA_TYPE);
		datatype = dtype_map_from_kernel(data);
		if (datatype == NULL) {
			netlink_io_error(ctx, NULL,
					 "Unknown data type in set key %u",
					 data);
			set = NULL;
			goto out;
		}
	}

	if (set_is_objmap(flags)) {
		objtype = nftnl_set_get_u32(nls, NFTNL_SET_OBJ_TYPE);
		assert(!datatype);
		datatype = &string_type;
	}

	set = set_alloc(&netlink_location);
	set->handle.family = nftnl_set_get_u32(nls, NFTNL_SET_FAMILY);
	set->handle.table.name = xstrdup(nftnl_set_get_str(nls, NFTNL_SET_TABLE));
	set->handle.set.name = xstrdup(nftnl_set_get_str(nls, NFTNL_SET_NAME));
	set->automerge	   = automerge;
	if (comment)
		set->comment = xstrdup(comment);

	init_list_head(&set_parse_ctx.stmt_list);

	if (nftnl_set_is_set(nls, NFTNL_SET_EXPR)) {
		const struct nftnl_expr *nle;
		struct stmt *stmt;

		nle = nftnl_set_get(nls, NFTNL_SET_EXPR);
		stmt = netlink_parse_set_expr(set, &ctx->nft->cache, nle);
		list_add_tail(&stmt->list, &set_parse_ctx.stmt_list);
	} else if (nftnl_set_is_set(nls, NFTNL_SET_EXPRESSIONS)) {
		set_parse_ctx.cache = &ctx->nft->cache;
		set_parse_ctx.set = set;
		nftnl_set_expr_foreach(nls, set_elem_parse_expressions,
				       &set_parse_ctx);
	}
	list_splice_tail(&set_parse_ctx.stmt_list, &set->stmt_list);

	set->flags = nftnl_set_get_u32(nls, NFTNL_SET_FLAGS);

	if (datatype) {
		uint32_t dlen;

		dtype2 = set_datatype_alloc(datatype, databyteorder);
		klen = nftnl_set_get_u32(nls, NFTNL_SET_DATA_LEN) * BITS_PER_BYTE;

		dlen = data_interval ?  klen / 2 : klen;

		if (set_udata_key_valid(typeof_expr_data, dlen)) {
			typeof_expr_data->len = klen;
			set->data = typeof_expr_data;
			typeof_expr_data = NULL;
		} else if (set->flags & NFT_SET_OBJECT) {
			set->data = constant_expr_alloc(&netlink_location,
							dtype2,
							databyteorder, klen,
							NULL);
		} else {
			set->data = constant_expr_alloc(&netlink_location,
							dtype2,
							databyteorder, klen,
							NULL);

			/* Can't use 'typeof' keyword, so discard key too */
			expr_free(typeof_expr_key);
			typeof_expr_key = NULL;
		}

		if (data_interval)
			set->data->flags |= EXPR_F_INTERVAL;
	}

	dtype = set_datatype_alloc(keytype, keybyteorder);
	klen = nftnl_set_get_u32(nls, NFTNL_SET_KEY_LEN) * BITS_PER_BYTE;

	if (set_udata_key_valid(typeof_expr_key, klen)) {
		set->key = typeof_expr_key;
		typeof_expr_key = NULL;
		set->key_typeof_valid = true;
	} else {
		set->key = constant_expr_alloc(&netlink_location, dtype,
					       keybyteorder, klen,
					       NULL);
	}

	set->handle.handle.id = nftnl_set_get_u64(nls, NFTNL_SET_HANDLE);

	set->objtype = objtype;

	if (nftnl_set_is_set(nls, NFTNL_SET_TIMEOUT))
		set->timeout = nftnl_set_get_u64(nls, NFTNL_SET_TIMEOUT);
	if (nftnl_set_is_set(nls, NFTNL_SET_GC_INTERVAL))
		set->gc_int  = nftnl_set_get_u32(nls, NFTNL_SET_GC_INTERVAL);

	if (nftnl_set_is_set(nls, NFTNL_SET_POLICY))
		set->policy = nftnl_set_get_u32(nls, NFTNL_SET_POLICY);

	if (nftnl_set_is_set(nls, NFTNL_SET_DESC_SIZE))
		set->desc.size = nftnl_set_get_u32(nls, NFTNL_SET_DESC_SIZE);

	if (nftnl_set_is_set(nls, NFTNL_SET_COUNT))
		set->count = nftnl_set_get_u32(nls, NFTNL_SET_COUNT);

	if (nftnl_set_is_set(nls, NFTNL_SET_DESC_CONCAT)) {
		uint32_t len = NFT_REG32_COUNT;
		const uint8_t *data;

		data = nftnl_set_get_data(nls, NFTNL_SET_DESC_CONCAT, &len);
		if (data) {
			memcpy(set->desc.field_len, data, len);
			set->desc.field_count = len;
		}
	}

out:
	expr_free(typeof_expr_data);
	expr_free(typeof_expr_key);
	datatype_free(datatype);
	datatype_free(keytype);
	datatype_free(dtype2);
	datatype_free(dtype);
	return set;
}

void alloc_setelem_cache(const struct expr *set, struct nftnl_set *nls)
{
	struct nftnl_set_elem *nlse;
	const struct expr *expr;

	list_for_each_entry(expr, &expr_set(set)->expressions, list) {
		nlse = alloc_nftnl_setelem(set, expr);
		nftnl_set_elem_add(nls, nlse);
	}
}

static bool range_expr_is_prefix(const struct expr *range, uint32_t *prefix_len)
{
	const struct expr *right = range->right;
	const struct expr *left = range->left;
	uint32_t len = left->len;
	unsigned long n1, n2;
	uint32_t plen;
	mpz_t bitmask;

	mpz_init2(bitmask, left->len);
	mpz_xor(bitmask, left->value, right->value);

	n1 = mpz_scan0(bitmask, 0);
	if (n1 == ULONG_MAX)
		goto not_a_prefix;

	n2 = mpz_scan1(bitmask, n1 + 1);
	if (n2 < len)
		goto not_a_prefix;

	plen = len - n1;

	if (mpz_scan1(left->value, 0) < len - plen)
		goto not_a_prefix;

	mpz_clear(bitmask);
	*prefix_len = plen;

	return true;

not_a_prefix:
	mpz_clear(bitmask);

	return false;
}

struct expr *range_expr_to_prefix(struct expr *range)
{
	struct expr *prefix;
	uint32_t prefix_len;

	if (range_expr_is_prefix(range, &prefix_len)) {
		prefix = prefix_expr_alloc(&range->location,
					   expr_get(range->left),
					   prefix_len);
		expr_free(range);
		return prefix;
	}

	return range;
}

static struct expr *range_expr_reduce(struct expr *range)
{
	struct expr *expr;

	if (!mpz_cmp(range->left->value, range->right->value)) {
		expr = expr_get(range->left);
		expr_free(range);
		return expr;
	}

	if (range->left->dtype->type != TYPE_IPADDR &&
	    range->left->dtype->type != TYPE_IP6ADDR)
		return range;

	return range_expr_to_prefix(range);
}

static struct expr *netlink_parse_interval_elem(const struct set *set,
						struct expr *expr)
{
	unsigned int len = netlink_padded_len(expr->len) / BITS_PER_BYTE;
	const struct datatype *dtype = set->data->dtype;
	struct expr *range, *left, *right;
	char data[NFT_MAX_EXPR_LEN_BYTES];

	if (len > sizeof(data))
		BUG("Value export of %u bytes would overflow", len);

	memset(data, 0, sizeof(data));

	mpz_export_data(data, expr->value, dtype->byteorder, len);
	left = constant_expr_alloc(&internal_location, dtype,
				   dtype->byteorder,
				   (len / 2) * BITS_PER_BYTE, &data[0]);
	right = constant_expr_alloc(&internal_location, dtype,
				    dtype->byteorder,
				    (len / 2) * BITS_PER_BYTE, &data[len / 2]);
	range = range_expr_alloc(&expr->location, left, right);
	expr_free(expr);

	return range_expr_to_prefix(range);
}

static struct expr *concat_elem_expr(const struct set *set, struct expr *key,
				     const struct datatype *dtype,
				     struct expr *data, int *off)
{
	const struct datatype *subtype;
	unsigned int sub_length;
	struct expr *expr;

	if (key) {
		(*off)--;
		sub_length = round_up(key->len, BITS_PER_BYTE);

		expr = constant_expr_splice(data, sub_length);
		expr->dtype = datatype_get(key->dtype);
		expr->byteorder = key->byteorder;
		expr->len = key->len;
	} else {
		subtype = concat_subtype_lookup(dtype->type, --(*off));
		sub_length = round_up(subtype->size, BITS_PER_BYTE);
		expr = constant_expr_splice(data, sub_length);
		expr->dtype = subtype;
		expr->byteorder = subtype->byteorder;
	}

	if (expr_basetype(expr)->type == TYPE_STRING ||
	    (!(set->flags & NFT_SET_INTERVAL) &&
	     expr->byteorder == BYTEORDER_HOST_ENDIAN))
		mpz_switch_byteorder(expr->value, expr->len / BITS_PER_BYTE);

	if (expr->dtype->basetype != NULL &&
	    expr->dtype->basetype->type == TYPE_BITMASK)
		expr = bitmask_expr_to_binops(expr);

	data->len -= netlink_padding_len(sub_length);

	return expr;
}

static struct expr *netlink_parse_concat_elem_key(const struct set *set,
						  struct expr *data)
{
	const struct datatype *dtype = set->key->dtype;
	struct expr *concat, *expr, *n = NULL;
	int off = dtype->subtypes;

	if (set->key->etype == EXPR_CONCAT)
		n = list_first_entry(&expr_concat(set->key)->expressions, struct expr, list);

	concat = concat_expr_alloc(&data->location);
	while (off > 0) {
		expr = concat_elem_expr(set, n, dtype, data, &off);
		concat_expr_add(concat, expr);
		if (set->key->etype == EXPR_CONCAT)
			n = list_next_entry(n, list);
	}

	expr_free(data);

	return concat;
}

static struct expr *netlink_parse_concat_elem(const struct set *set,
					      struct expr *data)
{
	const struct datatype *dtype = set->data->dtype;
	struct expr *concat, *expr, *left, *range;
	struct list_head expressions;
	int off = dtype->subtypes;

	init_list_head(&expressions);

	concat = concat_expr_alloc(&data->location);
	while (off > 0) {
		expr = concat_elem_expr(set, NULL, dtype, data, &off);
		list_add_tail(&expr->list, &expressions);
	}

	if (set->data->flags & EXPR_F_INTERVAL) {
		assert(!list_empty(&expressions));

		off = dtype->subtypes;

		while (off > 0) {
			left = list_first_entry(&expressions, struct expr, list);

			expr = concat_elem_expr(set, NULL, dtype, data, &off);
			list_del(&left->list);

			range = range_expr_alloc(&data->location, left, expr);
			range = range_expr_reduce(range);
			concat_expr_add(concat, range);
		}
		assert(list_empty(&expressions));
	} else {
		list_splice_tail(&expressions, &expr_concat(concat)->expressions);
	}

	expr_free(data);

	return concat;
}

static int set_elem_parse_udata_cb(const struct nftnl_udata *attr, void *data)
{
	const struct nftnl_udata **tb = data;
	unsigned char *value = nftnl_udata_get(attr);
	uint8_t type = nftnl_udata_type(attr);
	uint8_t len = nftnl_udata_len(attr);

	switch (type) {
	case NFTNL_UDATA_SET_ELEM_COMMENT:
		if (value[len - 1] != '\0')
			return -1;
		break;
	case NFTNL_UDATA_SET_ELEM_FLAGS:
		if (len != sizeof(uint32_t))
			return -1;
		break;
	default:
		return 0;
	}
	tb[type] = attr;
	return 0;
}

static void set_elem_parse_udata(struct nftnl_set_elem *nlse,
				 struct expr *expr)
{
	const struct nftnl_udata *ud[NFTNL_UDATA_SET_ELEM_MAX + 1] = {};
	const void *data;
	uint32_t len;

	data = nftnl_set_elem_get(nlse, NFTNL_SET_ELEM_USERDATA, &len);
	if (nftnl_udata_parse(data, len, set_elem_parse_udata_cb, ud))
		return;

	if (ud[NFTNL_UDATA_SET_ELEM_COMMENT])
		expr->comment =
			xstrdup(nftnl_udata_get(ud[NFTNL_UDATA_SET_ELEM_COMMENT]));
	if (ud[NFTNL_UDATA_SET_ELEM_FLAGS]) {
		uint32_t elem_flags;

		elem_flags =
			nftnl_udata_get_u32(ud[NFTNL_UDATA_SET_ELEM_FLAGS]);
		if (elem_flags & NFTNL_SET_ELEM_F_INTERVAL_OPEN)
			expr->flags |= EXPR_F_INTERVAL_OPEN;
	}
}

int netlink_delinearize_setelem(struct netlink_ctx *ctx,
				struct nftnl_set_elem *nlse,
				struct set *set)
{
	struct setelem_parse_ctx setelem_parse_ctx = {
		.set	= set,
		.cache	= &ctx->nft->cache,
	};
	struct nft_data_delinearize nld;
	struct expr *expr, *key, *data;
	uint32_t flags = 0;

	init_list_head(&setelem_parse_ctx.stmt_list);

	if (nftnl_set_elem_is_set(nlse, NFTNL_SET_ELEM_KEY))
		nld.value = nftnl_set_elem_get(nlse, NFTNL_SET_ELEM_KEY, &nld.len);
	if (nftnl_set_elem_is_set(nlse, NFTNL_SET_ELEM_FLAGS))
		flags = nftnl_set_elem_get_u32(nlse, NFTNL_SET_ELEM_FLAGS);

key_end:
	if (nftnl_set_elem_is_set(nlse, NFTNL_SET_ELEM_KEY)) {
		key = netlink_alloc_value(&netlink_location, &nld);
		datatype_set(key, set->key->dtype);
		key->byteorder	= set->key->byteorder;
		if (set->key->dtype->subtypes)
			key = netlink_parse_concat_elem_key(set, key);

		if (!(set->flags & NFT_SET_INTERVAL) &&
		    key->byteorder == BYTEORDER_HOST_ENDIAN)
			mpz_switch_byteorder(key->value, key->len / BITS_PER_BYTE);

		if (key->dtype->basetype != NULL &&
		    key->dtype->basetype->type == TYPE_BITMASK)
			key = bitmask_expr_to_binops(key);
	} else if (flags & NFT_SET_ELEM_CATCHALL) {
		key = set_elem_catchall_expr_alloc(&netlink_location);
		datatype_set(key, set->key->dtype);
		key->byteorder = set->key->byteorder;
		key->len = set->key->len;
	} else {
		netlink_io_error(ctx, NULL,
			         "Unexpected set element with no key");
		return 0;
	}

	expr = set_elem_expr_alloc(&netlink_location, key);
	expr->flags |= EXPR_F_KERNEL;

	if (nftnl_set_elem_is_set(nlse, NFTNL_SET_ELEM_TIMEOUT)) {
		expr->timeout	 = nftnl_set_elem_get_u64(nlse, NFTNL_SET_ELEM_TIMEOUT);
		if (expr->timeout == 0)
			expr->timeout	 = NFT_NEVER_TIMEOUT;
	}

	if (nftnl_set_elem_is_set(nlse, NFTNL_SET_ELEM_EXPIRATION))
		expr->expiration = nftnl_set_elem_get_u64(nlse, NFTNL_SET_ELEM_EXPIRATION);
	if (nftnl_set_elem_is_set(nlse, NFTNL_SET_ELEM_USERDATA)) {
		set_elem_parse_udata(nlse, expr);
		if (expr->comment)
			set->elem_has_comment = true;
	}
	if (nftnl_set_elem_is_set(nlse, NFTNL_SET_ELEM_EXPR)) {
		const struct nftnl_expr *nle;
		struct stmt *stmt;

		nle = nftnl_set_elem_get(nlse, NFTNL_SET_ELEM_EXPR, NULL);
		stmt = netlink_parse_set_expr(set, &ctx->nft->cache, nle);
		list_add_tail(&stmt->list, &setelem_parse_ctx.stmt_list);
	} else if (nftnl_set_elem_is_set(nlse, NFTNL_SET_ELEM_EXPRESSIONS)) {
		nftnl_set_elem_expr_foreach(nlse, set_elem_parse_expressions,
					    &setelem_parse_ctx);
	}
	list_splice_tail_init(&setelem_parse_ctx.stmt_list, &expr->stmt_list);

	if (flags & NFT_SET_ELEM_INTERVAL_END) {
		expr->flags |= EXPR_F_INTERVAL_END;
		if (mpz_cmp_ui(set->key->value, 0) == 0)
			set->root = true;
	}

	if (set_is_datamap(set->flags)) {
		if (nftnl_set_elem_is_set(nlse, NFTNL_SET_ELEM_DATA)) {
			nld.value = nftnl_set_elem_get(nlse, NFTNL_SET_ELEM_DATA,
						       &nld.len);
		} else if (nftnl_set_elem_is_set(nlse, NFTNL_SET_ELEM_CHAIN)) {
			nld.chain = nftnl_set_elem_get_str(nlse, NFTNL_SET_ELEM_CHAIN);
			nld.verdict = nftnl_set_elem_get_u32(nlse, NFTNL_SET_ELEM_VERDICT);
		} else if (nftnl_set_elem_is_set(nlse, NFTNL_SET_ELEM_VERDICT)) {
			nld.verdict = nftnl_set_elem_get_u32(nlse, NFTNL_SET_ELEM_VERDICT);
		} else
			goto out;

		data = netlink_alloc_data(&netlink_location, &nld,
					  set->data->dtype->type == TYPE_VERDICT ?
					  NFT_REG_VERDICT : NFT_REG_1);

		if (set->data->dtype->is_typeof)
			datatype_set(data, set->data->dtype->basetype);
		else
			datatype_set(data, set->data->dtype);
		data->byteorder = set->data->byteorder;

		if (set->data->dtype->subtypes) {
			data = netlink_parse_concat_elem(set, data);
		} else if (set->data->flags & EXPR_F_INTERVAL)
			data = netlink_parse_interval_elem(set, data);

		if (data->byteorder == BYTEORDER_HOST_ENDIAN)
			mpz_switch_byteorder(data->value, data->len / BITS_PER_BYTE);

		expr = mapping_expr_alloc(&netlink_location, expr, data);
	}
	if (set_is_objmap(set->flags)) {
		if (nftnl_set_elem_is_set(nlse, NFTNL_SET_ELEM_OBJREF)) {
			nld.value = nftnl_set_elem_get(nlse,
						       NFTNL_SET_ELEM_OBJREF,
						       &nld.len);
		} else
			goto out;

		data = netlink_alloc_value(&netlink_location, &nld);
		data->dtype = &string_type;
		data->byteorder = BYTEORDER_HOST_ENDIAN;
		mpz_switch_byteorder(data->value, data->len / BITS_PER_BYTE);
		expr = mapping_expr_alloc(&netlink_location, expr, data);
	}
out:
	set_expr_add(set->init, expr);

	if (!(flags & NFT_SET_ELEM_INTERVAL_END) &&
	    nftnl_set_elem_is_set(nlse, NFTNL_SET_ELEM_KEY_END)) {
		flags |= NFT_SET_ELEM_INTERVAL_END;
		nld.value = nftnl_set_elem_get(nlse, NFTNL_SET_ELEM_KEY_END,
					       &nld.len);
		goto key_end;
	}

	return 0;
}

static int list_setelem_cb(struct nftnl_set_elem *nlse, void *arg)
{
	struct netlink_ctx *ctx = arg;
	return netlink_delinearize_setelem(ctx, nlse, ctx->set);
}

static int list_setelem_debug_cb(struct nftnl_set_elem *nlse, void *arg)
{
	int r;

	r = list_setelem_cb(nlse, arg);
	if (r == 0) {
		struct netlink_ctx *ctx = arg;
		FILE *fp = ctx->nft->output.output_fp;

		fprintf(fp, "\t");
		nftnl_set_elem_fprintf(fp, nlse, 0, 0);
		fprintf(fp, "\n");
	}

	return r;
}

static int list_setelements(struct nftnl_set *s, struct netlink_ctx *ctx)
{
	FILE *fp = ctx->nft->output.output_fp;

	if (fp && (ctx->nft->debug_mask & NFT_DEBUG_NETLINK)) {
		const char *table, *name;
		uint32_t family = nftnl_set_get_u32(s, NFTNL_SET_FAMILY);

		table = nftnl_set_get_str(s, NFTNL_SET_TABLE);
		name = nftnl_set_get_str(s, NFTNL_SET_NAME);

		fprintf(fp, "%s %s @%s\n", family2str(family), table, name);

		return nftnl_set_elem_foreach(s, list_setelem_debug_cb, ctx);
	}

	return nftnl_set_elem_foreach(s, list_setelem_cb, ctx);
}

int netlink_list_setelems(struct netlink_ctx *ctx, const struct handle *h,
			  struct set *set, bool reset)
{
	struct nftnl_set *nls;
	int err;

	nls = nftnl_set_alloc();
	if (nls == NULL)
		memory_allocation_error();

	nftnl_set_set_u32(nls, NFTNL_SET_FAMILY, h->family);
	nftnl_set_set_str(nls, NFTNL_SET_TABLE, h->table.name);
	nftnl_set_set_str(nls, NFTNL_SET_NAME, h->set.name);
	if (h->handle.id)
		nftnl_set_set_u64(nls, NFTNL_SET_HANDLE, h->handle.id);

	err = mnl_nft_setelem_get(ctx, nls, reset);
	if (err < 0) {
		nftnl_set_free(nls);
		if (errno == EINTR)
			return -1;

		return 0;
	}

	ctx->set = set;
	set->init = set_expr_alloc(&internal_location, set);
	list_setelements(nls, ctx);

	if (set->flags & NFT_SET_INTERVAL && set->desc.field_count > 1)
		concat_range_aggregate(set->init);
	else if (set->flags & NFT_SET_INTERVAL)
		interval_map_decompose(set->init);
	else
		list_expr_sort(&expr_set(ctx->set->init)->expressions);

	nftnl_set_free(nls);
	ctx->set = NULL;

	return 0;
}

int netlink_get_setelem(struct netlink_ctx *ctx, const struct handle *h,
			const struct location *loc, struct set *cache_set,
			struct set *set, struct expr *init, bool reset)
{
	struct nftnl_set *nls, *nls_out = NULL;
	int err = 0;

	nls = nftnl_set_alloc();
	if (nls == NULL)
		memory_allocation_error();

	nftnl_set_set_u32(nls, NFTNL_SET_FAMILY, h->family);
	nftnl_set_set_str(nls, NFTNL_SET_TABLE, h->table.name);
	nftnl_set_set_str(nls, NFTNL_SET_NAME, h->set.name);
	if (h->handle.id)
		nftnl_set_set_u64(nls, NFTNL_SET_HANDLE, h->handle.id);

	alloc_setelem_cache(init, nls);

	netlink_dump_set(nls, ctx);

	nls_out = mnl_nft_setelem_get_one(ctx, nls, reset);
	if (!nls_out) {
		nftnl_set_free(nls);
		return -1;
	}

	ctx->set = set;
	set->init = set_expr_alloc(loc, set);
	list_setelements(nls_out, ctx);

	if (set->flags & NFT_SET_INTERVAL && set->desc.field_count > 1)
		concat_range_aggregate(set->init);
	else if (set->flags & NFT_SET_INTERVAL)
		err = get_set_decompose(cache_set, set);
	else
		list_expr_sort(&expr_set(ctx->set->init)->expressions);

	nftnl_set_free(nls);
	nftnl_set_free(nls_out);
	ctx->set = NULL;

	return err;
}

void netlink_dump_obj(struct nftnl_obj *nln, struct netlink_ctx *ctx)
{
	FILE *fp = ctx->nft->output.output_fp;

	if (!(ctx->nft->debug_mask & NFT_DEBUG_NETLINK) || !fp)
		return;

	nftnl_obj_fprintf(fp, nln, 0, 0);
	fprintf(fp, "\n");
}

static struct in6_addr all_zeroes;

static struct expr *
netlink_obj_tunnel_parse_addr(struct nftnl_obj *nlo, int attr)
{
	struct nft_data_delinearize nld;
	const struct datatype *dtype;
	const uint32_t *addr6;
	struct expr *expr;
	uint32_t addr;

	memset(&nld, 0, sizeof(nld));

	switch (attr) {
	case NFTNL_OBJ_TUNNEL_IPV4_SRC:
	case NFTNL_OBJ_TUNNEL_IPV4_DST:
		addr = nftnl_obj_get_u32(nlo, attr);
		if (!addr)
			return NULL;

		dtype = &ipaddr_type;
		nld.value = &addr;
		nld.len = sizeof(struct in_addr);
		break;
	case NFTNL_OBJ_TUNNEL_IPV6_SRC:
	case NFTNL_OBJ_TUNNEL_IPV6_DST:
		addr6 = nftnl_obj_get(nlo, attr);
		if (!memcmp(addr6, &all_zeroes, sizeof(all_zeroes)))
			return NULL;

		dtype = &ip6addr_type;
		nld.value = addr6;
		nld.len = sizeof(struct in6_addr);
		break;
	default:
		return NULL;
	}

	expr = netlink_alloc_value(&netlink_location, &nld);
	expr->dtype     = dtype;
	expr->byteorder = BYTEORDER_BIG_ENDIAN;

	return expr;
}

static int obj_parse_udata_cb(const struct nftnl_udata *attr, void *data)
{
	unsigned char *value = nftnl_udata_get(attr);
	uint8_t type = nftnl_udata_type(attr);
	const struct nftnl_udata **tb = data;
	uint8_t len = nftnl_udata_len(attr);

	switch (type) {
		case NFTNL_UDATA_OBJ_COMMENT:
			if (value[len - 1] != '\0')
				return -1;
			break;
		default:
			return 0;
	}
	tb[type] = attr;
	return 0;
}

static int tunnel_parse_opt_cb(struct nftnl_tunnel_opt *opt, void *data) {

	struct obj *obj = data;

	switch (nftnl_tunnel_opt_get_type(opt)) {
	case NFTNL_TUNNEL_TYPE_ERSPAN:
		obj->tunnel.type = TUNNEL_ERSPAN;
		if (nftnl_tunnel_opt_get_flags(opt) & (1 << NFTNL_TUNNEL_ERSPAN_VERSION)) {
			obj->tunnel.erspan.version =
				nftnl_tunnel_opt_get_u32(opt,
							 NFTNL_TUNNEL_ERSPAN_VERSION);
		}
		if (nftnl_tunnel_opt_get_flags(opt) & (1 << NFTNL_TUNNEL_ERSPAN_V1_INDEX)) {
			obj->tunnel.erspan.v1.index =
				nftnl_tunnel_opt_get_u32(opt,
							 NFTNL_TUNNEL_ERSPAN_V1_INDEX);
		}
		if (nftnl_tunnel_opt_get_flags(opt) & (1 << NFTNL_TUNNEL_ERSPAN_V2_HWID)) {
			obj->tunnel.erspan.v2.hwid =
				nftnl_tunnel_opt_get_u8(opt,
							NFTNL_TUNNEL_ERSPAN_V2_HWID);
		}
		if (nftnl_tunnel_opt_get_flags(opt) & (1 << NFTNL_TUNNEL_ERSPAN_V2_DIR)) {
			obj->tunnel.erspan.v2.direction =
				nftnl_tunnel_opt_get_u8(opt,
							NFTNL_TUNNEL_ERSPAN_V2_DIR);
		}
		break;
	case NFTNL_TUNNEL_TYPE_VXLAN:
		obj->tunnel.type = TUNNEL_VXLAN;
		if (nftnl_tunnel_opt_get_flags(opt) & (1 << NFTNL_TUNNEL_VXLAN_GBP)) {
			obj->tunnel.type = TUNNEL_VXLAN;
			obj->tunnel.vxlan.gbp = nftnl_tunnel_opt_get_u32(opt, NFTNL_TUNNEL_VXLAN_GBP);
		}
		break;
	case NFTNL_TUNNEL_TYPE_GENEVE:
		struct tunnel_geneve *geneve;
		const void *data;

		if (!obj->tunnel.type) {
			init_list_head(&obj->tunnel.geneve_opts);
			obj->tunnel.type = TUNNEL_GENEVE;
		}

		geneve = xmalloc(sizeof(struct tunnel_geneve));
		if (!geneve)
			memory_allocation_error();

		if (nftnl_tunnel_opt_get_flags(opt) & (1 << NFTNL_TUNNEL_GENEVE_TYPE))
			geneve->type = nftnl_tunnel_opt_get_u8(opt, NFTNL_TUNNEL_GENEVE_TYPE);

		if (nftnl_tunnel_opt_get_flags(opt) & (1 << NFTNL_TUNNEL_GENEVE_CLASS))
			geneve->geneve_class = nftnl_tunnel_opt_get_u16(opt, NFTNL_TUNNEL_GENEVE_CLASS);

		if (nftnl_tunnel_opt_get_flags(opt) & (1 << NFTNL_TUNNEL_GENEVE_DATA)) {
			data = nftnl_tunnel_opt_get_data(opt, NFTNL_TUNNEL_GENEVE_DATA,
							 &geneve->data_len);
			if (!data)
				return -1;
			memcpy(&geneve->data, data, geneve->data_len);
		}

		list_add_tail(&geneve->list, &obj->tunnel.geneve_opts);
		break;
	default:
		break;
	}

	return 0;
}

struct obj *netlink_delinearize_obj(struct netlink_ctx *ctx,
				    struct nftnl_obj *nlo)
{
	const struct nftnl_udata *ud[NFTNL_UDATA_OBJ_MAX + 1] = {};
	const char *udata;
	struct obj *obj;
	uint32_t type;
	uint32_t ulen;

	obj = obj_alloc(&netlink_location);
	obj->handle.family = nftnl_obj_get_u32(nlo, NFTNL_OBJ_FAMILY);
	obj->handle.table.name =
		xstrdup(nftnl_obj_get_str(nlo, NFTNL_OBJ_TABLE));
	obj->handle.obj.name =
		xstrdup(nftnl_obj_get_str(nlo, NFTNL_OBJ_NAME));
	obj->handle.handle.id =
		nftnl_obj_get_u64(nlo, NFTNL_OBJ_HANDLE);
	if (nftnl_obj_is_set(nlo, NFTNL_OBJ_USERDATA)) {
		udata = nftnl_obj_get_data(nlo, NFTNL_OBJ_USERDATA, &ulen);
		if (nftnl_udata_parse(udata, ulen, obj_parse_udata_cb, ud) < 0) {
			netlink_io_error(ctx, NULL, "Cannot parse userdata");
			obj_free(obj);
			return NULL;
		}
		if (ud[NFTNL_UDATA_OBJ_COMMENT])
			obj->comment = xstrdup(nftnl_udata_get(ud[NFTNL_UDATA_OBJ_COMMENT]));
	}

	type = nftnl_obj_get_u32(nlo, NFTNL_OBJ_TYPE);
	switch (type) {
	case NFT_OBJECT_COUNTER:
		obj->counter.packets =
			nftnl_obj_get_u64(nlo, NFTNL_OBJ_CTR_PKTS);
		obj->counter.bytes =
			nftnl_obj_get_u64(nlo, NFTNL_OBJ_CTR_BYTES);
		break;
	case NFT_OBJECT_QUOTA:
		obj->quota.bytes =
			nftnl_obj_get_u64(nlo, NFTNL_OBJ_QUOTA_BYTES);
		obj->quota.used =
			nftnl_obj_get_u64(nlo, NFTNL_OBJ_QUOTA_CONSUMED);
		obj->quota.flags =
			nftnl_obj_get_u32(nlo, NFTNL_OBJ_QUOTA_FLAGS);
		break;
	case NFT_OBJECT_SECMARK:
		snprintf(obj->secmark.ctx, sizeof(obj->secmark.ctx), "%s",
			 nftnl_obj_get_str(nlo, NFTNL_OBJ_SECMARK_CTX));
		break;
	case NFT_OBJECT_CT_HELPER:
		snprintf(obj->ct_helper.name, sizeof(obj->ct_helper.name), "%s",
			 nftnl_obj_get_str(nlo, NFTNL_OBJ_CT_HELPER_NAME));
		obj->ct_helper.l3proto = nftnl_obj_get_u16(nlo, NFTNL_OBJ_CT_HELPER_L3PROTO);
		obj->ct_helper.l4proto = nftnl_obj_get_u8(nlo, NFTNL_OBJ_CT_HELPER_L4PROTO);
		break;
	case NFT_OBJECT_CT_TIMEOUT:
		init_list_head(&obj->ct_timeout.timeout_list);
		obj->ct_timeout.l3proto = nftnl_obj_get_u16(nlo, NFTNL_OBJ_CT_TIMEOUT_L3PROTO);
		obj->ct_timeout.l4proto = nftnl_obj_get_u8(nlo, NFTNL_OBJ_CT_TIMEOUT_L4PROTO);
		if (nftnl_obj_is_set(nlo, NFTNL_OBJ_CT_TIMEOUT_ARRAY))
			memcpy(obj->ct_timeout.timeout,
			       nftnl_obj_get(nlo, NFTNL_OBJ_CT_TIMEOUT_ARRAY),
			       NFTNL_CTTIMEOUT_ARRAY_MAX * sizeof(uint32_t));
		break;
	case NFT_OBJECT_LIMIT:
		obj->limit.rate =
			nftnl_obj_get_u64(nlo, NFTNL_OBJ_LIMIT_RATE);
		obj->limit.unit =
			nftnl_obj_get_u64(nlo, NFTNL_OBJ_LIMIT_UNIT);
		obj->limit.burst =
			nftnl_obj_get_u32(nlo, NFTNL_OBJ_LIMIT_BURST);
		obj->limit.type =
			nftnl_obj_get_u32(nlo, NFTNL_OBJ_LIMIT_TYPE);
		obj->limit.flags =
			nftnl_obj_get_u32(nlo, NFTNL_OBJ_LIMIT_FLAGS);
		break;
	case NFT_OBJECT_CT_EXPECT:
		obj->ct_expect.l3proto =
			nftnl_obj_get_u16(nlo, NFTNL_OBJ_CT_EXPECT_L3PROTO);
		obj->ct_expect.l4proto =
			nftnl_obj_get_u8(nlo, NFTNL_OBJ_CT_EXPECT_L4PROTO);
		obj->ct_expect.dport =
			nftnl_obj_get_u16(nlo, NFTNL_OBJ_CT_EXPECT_DPORT);
		obj->ct_expect.timeout =
			nftnl_obj_get_u32(nlo, NFTNL_OBJ_CT_EXPECT_TIMEOUT);
		obj->ct_expect.size =
			nftnl_obj_get_u8(nlo, NFTNL_OBJ_CT_EXPECT_SIZE);
		break;
	case NFT_OBJECT_SYNPROXY:
		obj->synproxy.mss =
			nftnl_obj_get_u16(nlo, NFTNL_OBJ_SYNPROXY_MSS);
		obj->synproxy.wscale =
			nftnl_obj_get_u8(nlo, NFTNL_OBJ_SYNPROXY_WSCALE);
		obj->synproxy.flags =
			nftnl_obj_get_u32(nlo, NFTNL_OBJ_SYNPROXY_FLAGS);
		break;
	case NFT_OBJECT_TUNNEL:
		if (nftnl_obj_is_set(nlo, NFTNL_OBJ_TUNNEL_ID))
			obj->tunnel.id = nftnl_obj_get_u32(nlo, NFTNL_OBJ_TUNNEL_ID);
		if (nftnl_obj_is_set(nlo, NFTNL_OBJ_TUNNEL_SPORT)) {
			obj->tunnel.sport =
				nftnl_obj_get_u16(nlo, NFTNL_OBJ_TUNNEL_SPORT);
		}
		if (nftnl_obj_is_set(nlo, NFTNL_OBJ_TUNNEL_DPORT)) {
			obj->tunnel.dport =
				nftnl_obj_get_u16(nlo, NFTNL_OBJ_TUNNEL_DPORT);
		}
		if (nftnl_obj_is_set(nlo, NFTNL_OBJ_TUNNEL_TOS)) {
			obj->tunnel.tos =
				nftnl_obj_get_u8(nlo, NFTNL_OBJ_TUNNEL_TOS);
		}
		if (nftnl_obj_is_set(nlo, NFTNL_OBJ_TUNNEL_TTL)) {
			obj->tunnel.ttl =
				nftnl_obj_get_u8(nlo, NFTNL_OBJ_TUNNEL_TTL);
		}
		if (nftnl_obj_is_set(nlo, NFTNL_OBJ_TUNNEL_IPV4_SRC)) {
			obj->tunnel.src =
				netlink_obj_tunnel_parse_addr(nlo, NFTNL_OBJ_TUNNEL_IPV4_SRC);
		}
		if (nftnl_obj_is_set(nlo, NFTNL_OBJ_TUNNEL_IPV4_DST)) {
			obj->tunnel.dst =
				netlink_obj_tunnel_parse_addr(nlo, NFTNL_OBJ_TUNNEL_IPV4_DST);
		}
		if (nftnl_obj_is_set(nlo, NFTNL_OBJ_TUNNEL_IPV6_SRC)) {
			obj->tunnel.src =
				netlink_obj_tunnel_parse_addr(nlo, NFTNL_OBJ_TUNNEL_IPV6_SRC);
		}
		if (nftnl_obj_is_set(nlo, NFTNL_OBJ_TUNNEL_IPV6_DST)) {
			obj->tunnel.dst =
				netlink_obj_tunnel_parse_addr(nlo, NFTNL_OBJ_TUNNEL_IPV6_DST);
		}
		if (nftnl_obj_is_set(nlo, NFTNL_OBJ_TUNNEL_OPTS)) {
			nftnl_obj_tunnel_opts_foreach(nlo, tunnel_parse_opt_cb, obj);
		}
		break;
	default:
		netlink_io_error(ctx, NULL, "Unknown object type %u", type);
		obj_free(obj);
		return NULL;
	}
	obj->type = type;

	return obj;
}

void netlink_dump_flowtable(struct nftnl_flowtable *flo,
			    struct netlink_ctx *ctx)
{
	FILE *fp = ctx->nft->output.output_fp;

	if (!(ctx->nft->debug_mask & NFT_DEBUG_NETLINK) || !fp)
		return;

	nftnl_flowtable_fprintf(fp, flo, 0, 0);
	fprintf(fp, "\n");
}

struct flowtable *
netlink_delinearize_flowtable(struct netlink_ctx *ctx,
			      struct nftnl_flowtable *nlo)
{
	struct flowtable *flowtable;
	const char * const *dev_array;
	int len = 0, i, priority;

	flowtable = flowtable_alloc(&netlink_location);
	flowtable->handle.family =
		nftnl_flowtable_get_u32(nlo, NFTNL_FLOWTABLE_FAMILY);
	flowtable->handle.table.name =
		xstrdup(nftnl_flowtable_get_str(nlo, NFTNL_FLOWTABLE_TABLE));
	flowtable->handle.flowtable.name =
		xstrdup(nftnl_flowtable_get_str(nlo, NFTNL_FLOWTABLE_NAME));
	flowtable->handle.handle.id =
		nftnl_flowtable_get_u64(nlo, NFTNL_FLOWTABLE_HANDLE);
	if (nftnl_flowtable_is_set(nlo, NFTNL_FLOWTABLE_FLAGS))
		flowtable->flags = nftnl_flowtable_get_u32(nlo, NFTNL_FLOWTABLE_FLAGS);
	dev_array = nftnl_flowtable_get(nlo, NFTNL_FLOWTABLE_DEVICES);
	while (dev_array && dev_array[len])
		len++;

	if (len)
		flowtable->dev_array = xmalloc(len * sizeof(char *));
	for (i = 0; i < len; i++)
		flowtable->dev_array[i] = xstrdup(dev_array[i]);

	flowtable->dev_array_len = len;

	if (flowtable->dev_array_len) {
		qsort(flowtable->dev_array, flowtable->dev_array_len,
		      sizeof(char *), qsort_device_cmp);
	}

	if (nftnl_flowtable_is_set(nlo, NFTNL_FLOWTABLE_PRIO)) {
		priority = nftnl_flowtable_get_u32(nlo, NFTNL_FLOWTABLE_PRIO);
		flowtable->priority.expr =
				constant_expr_alloc(&netlink_location,
						    &integer_type,
						    BYTEORDER_HOST_ENDIAN,
						    sizeof(int) *
						    BITS_PER_BYTE,
						    &priority);
	}
	flowtable->hook.num =
		nftnl_flowtable_get_u32(nlo, NFTNL_FLOWTABLE_HOOKNUM);
	flowtable->flags =
		nftnl_flowtable_get_u32(nlo, NFTNL_FLOWTABLE_FLAGS);

	return flowtable;
}

static int list_flowtable_cb(struct nftnl_flowtable *nls, void *arg)
{
	struct netlink_ctx *ctx = arg;
	struct flowtable *flowtable;

	flowtable = netlink_delinearize_flowtable(ctx, nls);
	if (flowtable == NULL)
		return -1;
	list_add_tail(&flowtable->list, &ctx->list);
	return 0;
}

int netlink_list_flowtables(struct netlink_ctx *ctx, const struct handle *h)
{
	struct nftnl_flowtable_list *flowtable_cache;
	int err;

	flowtable_cache = mnl_nft_flowtable_dump(ctx, h->family,
						 h->table.name, NULL);
	if (flowtable_cache == NULL) {
		if (errno == EINTR)
			return -1;

		return 0;
	}

	err = nftnl_flowtable_list_foreach(flowtable_cache, list_flowtable_cb, ctx);
	nftnl_flowtable_list_free(flowtable_cache);
	return err;
}
