/*
 * Copyright (c) 2008 Patrick McHardy <kaber@trash.net>
 * Copyright (c) 2013 Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Development of this code funded by Astaro AG (http://www.astaro.com/)
 */

#include <nft.h>

#include <limits.h>
#include <linux/netfilter/nf_tables.h>
#include <arpa/inet.h>
#include <linux/netfilter/nf_nat.h>
#include <linux/netfilter.h>
#include <net/ethernet.h>
#include <netlink.h>
#include <rule.h>
#include <statement.h>
#include <expression.h>
#include <gmputil.h>
#include <utils.h>
#include <erec.h>
#include <sys/socket.h>
#include <libnftnl/udata.h>
#include <cache.h>
#include <xt.h>

struct dl_proto_ctx *dl_proto_ctx(struct rule_pp_ctx *ctx)
{
	return ctx->dl;
}

static struct dl_proto_ctx *dl_proto_ctx_outer(struct rule_pp_ctx *ctx)
{
	return &ctx->_dl[0];
}

static int netlink_parse_expr(const struct nftnl_expr *nle,
			      struct netlink_parse_ctx *ctx);

static void __fmtstring(3, 4) netlink_error(struct netlink_parse_ctx *ctx,
					    const struct location *loc,
					    const char *fmt, ...)
{
	struct error_record *erec;
	va_list ap;

	va_start(ap, fmt);
	erec = erec_vcreate(EREC_ERROR, loc, fmt, ap);
	va_end(ap);
	erec_queue(erec, ctx->msgs);
}

static unsigned int netlink_parse_register(const struct nftnl_expr *nle,
					   unsigned int attr)
{
	unsigned int reg;

	reg = nftnl_expr_get_u32(nle, attr);
	/* Translate 128bit registers to corresponding 32bit registers */
	if (reg >= NFT_REG_1 && reg <= NFT_REG_4)
		reg = 1 + (reg - NFT_REG_1) * (NFT_REG_SIZE / NFT_REG32_SIZE);
	else if (reg >= NFT_REG32_00)
		reg = 1 + reg - NFT_REG32_00;

	return reg;
}

static void netlink_set_register(struct netlink_parse_ctx *ctx,
				 enum nft_registers reg,
				 struct expr *expr)
{
	if (reg == NFT_REG_VERDICT || reg > MAX_REGS) {
		netlink_error(ctx, &expr->location,
			      "Invalid destination register %u", reg);
		expr_free(expr);
		return;
	}

	expr_free(ctx->registers[reg]);

	ctx->registers[reg] = expr;
}

static struct expr *netlink_get_register(struct netlink_parse_ctx *ctx,
					 const struct location *loc,
					 enum nft_registers reg)
{
	struct expr *expr;

	if (reg == NFT_REG_VERDICT || reg > MAX_REGS) {
		netlink_error(ctx, loc, "Invalid source register %u", reg);
		return NULL;
	}

	expr = ctx->registers[reg];
	if (expr != NULL)
		expr = expr_clone(expr);

	return expr;
}

static void netlink_release_registers(struct netlink_parse_ctx *ctx)
{
	int i;

	for (i = 0; i <= MAX_REGS; i++)
		expr_free(ctx->registers[i]);
}

static struct expr *netlink_parse_concat_expr(struct netlink_parse_ctx *ctx,
					      const struct location *loc,
					      unsigned int reg,
					      unsigned int len)
{
	struct expr *concat, *expr;
	unsigned int consumed;

	concat = concat_expr_alloc(loc);
	while (len > 0) {
		expr = netlink_get_register(ctx, loc, reg);
		if (expr == NULL) {
			netlink_error(ctx, loc,
				      "Relational expression size mismatch");
			goto err;
		}
		concat_expr_add(concat, expr);

		consumed = netlink_padded_len(expr->len);
		assert(consumed > 0);
		len -= consumed;
		reg += netlink_register_space(expr->len);
	}
	return concat;

err:
	expr_free(concat);
	return NULL;
}

static struct expr *netlink_parse_concat_key(struct netlink_parse_ctx *ctx,
					       const struct location *loc,
					       unsigned int reg,
					       const struct expr *key)
{
	uint32_t type = key->dtype->type;
	unsigned int n, len = key->len;
	struct expr *concat, *expr;
	unsigned int consumed;

	concat = concat_expr_alloc(loc);
	n = div_round_up(fls(type), TYPE_BITS);

	while (len > 0) {
		const struct datatype *i;

		expr = netlink_get_register(ctx, loc, reg);
		if (expr == NULL) {
			netlink_error(ctx, loc,
				      "Concat expression size mismatch");
			goto err;
		}

		if (n > 0 && concat_subtype_id(type, --n)) {
			i = concat_subtype_lookup(type, n);

			expr_set_type(expr, i, i->byteorder);
		}

		concat_expr_add(concat, expr);

		consumed = netlink_padded_len(expr->len);
		assert(consumed > 0);
		len -= consumed;
		reg += netlink_register_space(expr->len);
	}

	return concat;

err:
	expr_free(concat);
	return NULL;
}

static struct expr *netlink_parse_concat_data(struct netlink_parse_ctx *ctx,
					      const struct location *loc,
					      unsigned int reg,
					      unsigned int len,
					      struct expr *data)
{
	struct expr *concat, *expr, *i;

	concat = concat_expr_alloc(loc);
	while (len > 0) {
		expr = netlink_get_register(ctx, loc, reg);
		if (expr == NULL) {
			netlink_error(ctx, loc,
				      "Relational expression size mismatch");
			goto err;
		}
		i = constant_expr_splice(data, expr->len);
		data->len -= netlink_padding_len(expr->len);
		concat_expr_add(concat, i);

		len -= netlink_padded_len(expr->len);
		reg += netlink_register_space(expr->len);
		expr_free(expr);
	}
	return concat;

err:
	expr_free(concat);
	return NULL;
}

static void netlink_parse_chain_verdict(struct netlink_parse_ctx *ctx,
					const struct location *loc,
					struct expr *expr,
					enum nft_verdicts verdict)
{
	char chain_name[NFT_CHAIN_MAXNAMELEN] = {};
	struct chain *chain;

	expr_chain_export(expr->chain, chain_name);
	chain = chain_binding_lookup(ctx->table, chain_name);

	/* Special case: 'nft list chain x y' needs to pull in implicit chains */
	if (!chain && !strncmp(chain_name, "__chain", strlen("__chain"))) {
		nft_chain_cache_update(ctx->nlctx, ctx->table, chain_name);
		chain = chain_binding_lookup(ctx->table, chain_name);
	}

	if (chain) {
		ctx->stmt = chain_stmt_alloc(loc, chain_get(chain), verdict);
		expr_free(expr);
	} else {
		ctx->stmt = verdict_stmt_alloc(loc, expr);
	}
}

static void netlink_parse_immediate(struct netlink_parse_ctx *ctx,
				    const struct location *loc,
				    const struct nftnl_expr *nle)
{
	struct nft_data_delinearize nld;
	enum nft_registers dreg;
	struct expr *expr;

	if (nftnl_expr_is_set(nle, NFTNL_EXPR_IMM_VERDICT)) {
		nld.verdict = nftnl_expr_get_u32(nle, NFTNL_EXPR_IMM_VERDICT);
		if  (nftnl_expr_is_set(nle, NFTNL_EXPR_IMM_CHAIN)) {
			nld.chain = nftnl_expr_get(nle, NFTNL_EXPR_IMM_CHAIN,
						   &nld.len);
		}
	} else if (nftnl_expr_is_set(nle, NFTNL_EXPR_IMM_DATA)) {
		nld.value = nftnl_expr_get(nle, NFTNL_EXPR_IMM_DATA, &nld.len);
	}

	dreg = netlink_parse_register(nle, NFTNL_EXPR_IMM_DREG);
	expr = netlink_alloc_data(loc, &nld, dreg);

	if (dreg == NFT_REG_VERDICT) {
		switch (expr->verdict) {
		case NFT_JUMP:
			netlink_parse_chain_verdict(ctx, loc, expr, NFT_JUMP);
			break;
		case NFT_GOTO:
			netlink_parse_chain_verdict(ctx, loc, expr, NFT_GOTO);
			break;
		default:
			ctx->stmt = verdict_stmt_alloc(loc, expr);
			break;
		}
	} else {
		netlink_set_register(ctx, dreg, expr);
	}
}

static void netlink_parse_xfrm(struct netlink_parse_ctx *ctx,
			       const struct location *loc,
			       const struct nftnl_expr *nle)
{
	enum nft_registers dreg;
	enum nft_xfrm_keys key;
	struct expr *expr;
	uint32_t spnum;
	uint8_t dir;

	key = nftnl_expr_get_u32(nle, NFTNL_EXPR_XFRM_KEY);
	dir = nftnl_expr_get_u8(nle, NFTNL_EXPR_XFRM_DIR);
	spnum = nftnl_expr_get_u32(nle, NFTNL_EXPR_XFRM_SPNUM);
	expr = xfrm_expr_alloc(loc, dir, spnum, key);

	dreg = netlink_parse_register(nle, NFTNL_EXPR_XFRM_DREG);
	netlink_set_register(ctx, dreg, expr);
}

static enum ops netlink_parse_range_op(const struct nftnl_expr *nle)
{
	switch (nftnl_expr_get_u32(nle, NFTNL_EXPR_RANGE_OP)) {
	case NFT_RANGE_EQ:
		return OP_EQ;
	case NFT_RANGE_NEQ:
		return OP_NEQ;
	default:
		return OP_INVALID;
	}
}

static void netlink_parse_range(struct netlink_parse_ctx *ctx,
				const struct location *loc,
				const struct nftnl_expr *nle)
{
	struct expr *expr, *left, *right, *from, *to;
	struct nft_data_delinearize nld;
	enum nft_registers sreg;
	enum ops op;

	sreg = netlink_parse_register(nle, NFTNL_EXPR_RANGE_SREG);
	left = netlink_get_register(ctx, loc, sreg);
	if (left == NULL)
		return netlink_error(ctx, loc,
				     "Relational expression has no left hand side");

	op = netlink_parse_range_op(nle);

	nld.value = nftnl_expr_get(nle, NFTNL_EXPR_RANGE_FROM_DATA, &nld.len);
	from = netlink_alloc_value(loc, &nld);

	nld.value = nftnl_expr_get(nle, NFTNL_EXPR_RANGE_TO_DATA, &nld.len);
	to = netlink_alloc_value(loc, &nld);

	right = range_expr_alloc(loc, from, to);
	expr = relational_expr_alloc(loc, op, left, right);
	ctx->stmt = expr_stmt_alloc(loc, expr);
}

static enum ops netlink_parse_cmp_op(const struct nftnl_expr *nle)
{
	switch (nftnl_expr_get_u32(nle, NFTNL_EXPR_CMP_OP)) {
	case NFT_CMP_EQ:
		return OP_EQ;
	case NFT_CMP_NEQ:
		return OP_NEQ;
	case NFT_CMP_LT:
		return OP_LT;
	case NFT_CMP_LTE:
		return OP_LTE;
	case NFT_CMP_GT:
		return OP_GT;
	case NFT_CMP_GTE:
		return OP_GTE;
	default:
		return OP_INVALID;
	}
}

static void netlink_parse_cmp(struct netlink_parse_ctx *ctx,
			      const struct location *loc,
			      const struct nftnl_expr *nle)
{
	struct nft_data_delinearize nld;
	enum nft_registers sreg;
	struct expr *expr, *left, *right, *tmp;
	enum ops op;

	sreg = netlink_parse_register(nle, NFTNL_EXPR_CMP_SREG);
	left = netlink_get_register(ctx, loc, sreg);
	if (left == NULL)
		return netlink_error(ctx, loc,
				     "Relational expression has no left "
				     "hand side");

	op = netlink_parse_cmp_op(nle);

	nld.value = nftnl_expr_get(nle, NFTNL_EXPR_CMP_DATA, &nld.len);
	right = netlink_alloc_value(loc, &nld);

	if (left->len > right->len &&
	    expr_basetype(left) != &string_type) {
		mpz_lshift_ui(right->value, left->len - right->len);
		right = prefix_expr_alloc(loc, right, right->len);
		right->prefix->len = left->len;
	} else if (left->len > 0 && left->len < right->len) {
		expr_free(left);
		left = netlink_parse_concat_expr(ctx, loc, sreg, right->len);
		if (left == NULL)
			goto err_free;
		tmp = netlink_parse_concat_data(ctx, loc, sreg, right->len, right);
		if (tmp == NULL)
			goto err_free;
		expr_free(right);
		right = tmp;
	}

	expr = relational_expr_alloc(loc, op, left, right);
	ctx->stmt = expr_stmt_alloc(loc, expr);
	return;
err_free:
	expr_free(left);
	expr_free(right);
}

static void netlink_parse_lookup(struct netlink_parse_ctx *ctx,
				 const struct location *loc,
				 const struct nftnl_expr *nle)
{
	enum nft_registers sreg, dreg;
	const char *name;
	struct expr *expr, *left, *right;
	struct set *set;
	uint32_t flag;

	name = nftnl_expr_get_str(nle, NFTNL_EXPR_LOOKUP_SET);
	set  = set_cache_find(ctx->table, name);
	if (set == NULL)
		return netlink_error(ctx, loc,
				     "Unknown set '%s' in lookup expression",
				     name);

	sreg = netlink_parse_register(nle, NFTNL_EXPR_LOOKUP_SREG);
	left = netlink_get_register(ctx, loc, sreg);
	if (left == NULL)
		return netlink_error(ctx, loc,
				     "Lookup expression has no left hand side");

	if (left->len < set->key->len) {
		expr_free(left);
		left = netlink_parse_concat_expr(ctx, loc, sreg, set->key->len);
		if (left == NULL)
			return;
	}

	right = set_ref_expr_alloc(loc, set);

	if (nftnl_expr_is_set(nle, NFTNL_EXPR_LOOKUP_DREG)) {
		dreg = netlink_parse_register(nle, NFTNL_EXPR_LOOKUP_DREG);
		expr = map_expr_alloc(loc, left, right);
		if (dreg != NFT_REG_VERDICT)
			return netlink_set_register(ctx, dreg, expr);
	} else {
		expr = relational_expr_alloc(loc, OP_EQ, left, right);
	}

	if (nftnl_expr_is_set(nle, NFTNL_EXPR_LOOKUP_FLAGS)) {
		flag = nftnl_expr_get_u32(nle, NFTNL_EXPR_LOOKUP_FLAGS);
		if (flag & NFT_LOOKUP_F_INV)
			expr->op = OP_NEQ;
	}

	ctx->stmt = expr_stmt_alloc(loc, expr);
}

static struct expr *
netlink_parse_bitwise_mask_xor(struct netlink_parse_ctx *ctx,
			       const struct location *loc,
			       const struct nftnl_expr *nle,
			       enum nft_registers sreg,
			       struct expr *left)
{
	struct nft_data_delinearize nld;
	struct expr *expr, *mask, *xor, *or;
	mpz_t m, x, o;

	expr = left;

	nld.value = nftnl_expr_get(nle, NFTNL_EXPR_BITWISE_MASK, &nld.len);
	mask = netlink_alloc_value(loc, &nld);
	mpz_init_set(m, mask->value);

	nld.value = nftnl_expr_get(nle, NFTNL_EXPR_BITWISE_XOR, &nld.len);
	xor  = netlink_alloc_value(loc, &nld);
	mpz_init_set(x, xor->value);

	mpz_init_set_ui(o, 0);
	if (mpz_scan0(m, 0) != mask->len || mpz_cmp_ui(x, 0)) {
		/* o = (m & x) ^ x */
		mpz_and(o, m, x);
		mpz_xor(o, o, x);
		/* x &= m */
		mpz_and(x, x, m);
		/* m |= o */
		mpz_ior(m, m, o);
	}

	if (left->len > 0 && mpz_scan0(m, 0) >= left->len) {
		/* mask encompasses the entire value */
		expr_free(mask);
	} else {
		mpz_set(mask->value, m);
		expr = binop_expr_alloc(loc, OP_AND, expr, mask);
		expr->len = left->len;
	}

	if (mpz_cmp_ui(x, 0)) {
		mpz_set(xor->value, x);
		expr = binop_expr_alloc(loc, OP_XOR, expr, xor);
		expr->len = left->len;
	} else
		expr_free(xor);

	if (mpz_cmp_ui(o, 0)) {
		nld.value = nftnl_expr_get(nle, NFTNL_EXPR_BITWISE_XOR,
					   &nld.len);

		or = netlink_alloc_value(loc, &nld);
		mpz_set(or->value, o);
		expr = binop_expr_alloc(loc, OP_OR, expr, or);
		expr->len = left->len;
	}

	mpz_clear(m);
	mpz_clear(x);
	mpz_clear(o);

	return expr;
}

static struct expr *netlink_parse_bitwise_bool(struct netlink_parse_ctx *ctx,
					       const struct location *loc,
					       const struct nftnl_expr *nle,
					       enum nft_bitwise_ops op,
					       enum nft_registers sreg,
					       struct expr *left)
{
	enum nft_registers sreg2;
	struct expr *right, *expr;

	sreg2 = netlink_parse_register(nle, NFTNL_EXPR_BITWISE_SREG2);
	right = netlink_get_register(ctx, loc, sreg2);
	if (right == NULL) {
		netlink_error(ctx, loc,
			      "Bitwise expression has no right-hand expression");
		return NULL;
	}

	expr = binop_expr_alloc(loc,
				op == NFT_BITWISE_XOR ? OP_XOR :
				op == NFT_BITWISE_AND ? OP_AND : OP_OR,
				left, right);

	if (left->len > 0)
		expr->len = left->len;

	return expr;
}

static struct expr *netlink_parse_bitwise_shift(struct netlink_parse_ctx *ctx,
						const struct location *loc,
						const struct nftnl_expr *nle,
						enum nft_bitwise_ops op,
						enum nft_registers sreg,
						struct expr *left)
{
	struct nft_data_delinearize nld;
	struct expr *expr, *right;

	nld.value = nftnl_expr_get(nle, NFTNL_EXPR_BITWISE_DATA, &nld.len);
	right = netlink_alloc_value(loc, &nld);
	right->byteorder = BYTEORDER_HOST_ENDIAN;

	expr = binop_expr_alloc(loc,
				op == NFT_BITWISE_LSHIFT ? OP_LSHIFT : OP_RSHIFT,
				left, right);
	expr->len = nftnl_expr_get_u32(nle, NFTNL_EXPR_BITWISE_LEN) * BITS_PER_BYTE;

	return expr;
}

static void netlink_parse_bitwise(struct netlink_parse_ctx *ctx,
				  const struct location *loc,
				  const struct nftnl_expr *nle)
{
	enum nft_registers sreg, dreg;
	struct expr *expr, *left;
	enum nft_bitwise_ops op;

	sreg = netlink_parse_register(nle, NFTNL_EXPR_BITWISE_SREG);
	left = netlink_get_register(ctx, loc, sreg);
	if (left == NULL)
		return netlink_error(ctx, loc,
				     "Bitwise expression has no left "
				     "hand side");

	op = nftnl_expr_get_u32(nle, NFTNL_EXPR_BITWISE_OP);

	switch (op) {
	case NFT_BITWISE_MASK_XOR:
		expr = netlink_parse_bitwise_mask_xor(ctx, loc, nle, sreg,
						      left);
		break;
	case NFT_BITWISE_XOR:
	case NFT_BITWISE_AND:
	case NFT_BITWISE_OR:
		expr = netlink_parse_bitwise_bool(ctx, loc, nle, op,
						  sreg, left);
		break;
	case NFT_BITWISE_LSHIFT:
	case NFT_BITWISE_RSHIFT:
		expr = netlink_parse_bitwise_shift(ctx, loc, nle, op,
						   sreg, left);
		break;
	default:
		return netlink_error(ctx, loc,
				     "Invalid bitwise operation %u", op);
	}

	dreg = netlink_parse_register(nle, NFTNL_EXPR_BITWISE_DREG);
	netlink_set_register(ctx, dreg, expr);
}

static void netlink_parse_byteorder(struct netlink_parse_ctx *ctx,
				    const struct location *loc,
				    const struct nftnl_expr *nle)
{
	uint32_t opval = nftnl_expr_get_u32(nle, NFTNL_EXPR_BYTEORDER_OP);
	enum nft_registers sreg, dreg;
	struct expr *expr, *arg;
	enum ops op;

	sreg = netlink_parse_register(nle, NFTNL_EXPR_BYTEORDER_SREG);
	arg = netlink_get_register(ctx, loc, sreg);
	if (arg == NULL)
		return netlink_error(ctx, loc,
				     "Byteorder expression has no left "
				     "hand side");

	switch (opval) {
	case NFT_BYTEORDER_NTOH:
		op = OP_NTOH;
		break;
	case NFT_BYTEORDER_HTON:
		op = OP_HTON;
		break;
	default:
		expr_free(arg);
		return netlink_error(ctx, loc,
				     "Invalid byteorder operation %u", opval);
	}

	expr = unary_expr_alloc(loc, op, arg);
	expr->len = arg->len;

	dreg = netlink_parse_register(nle, NFTNL_EXPR_BYTEORDER_DREG);
	netlink_set_register(ctx, dreg, expr);
}

static void netlink_parse_payload_expr(struct netlink_parse_ctx *ctx,
				       const struct location *loc,
				       const struct nftnl_expr *nle)
{
	enum nft_registers dreg;
	uint32_t base, offset, len;
	struct expr *expr;

	base   = nftnl_expr_get_u32(nle, NFTNL_EXPR_PAYLOAD_BASE) + 1;

	if (base == NFT_PAYLOAD_TUN_HEADER + 1)
		base = NFT_PAYLOAD_INNER_HEADER + 1;

	offset = nftnl_expr_get_u32(nle, NFTNL_EXPR_PAYLOAD_OFFSET) * BITS_PER_BYTE;
	len    = nftnl_expr_get_u32(nle, NFTNL_EXPR_PAYLOAD_LEN) * BITS_PER_BYTE;

	expr = payload_expr_alloc(loc, NULL, 0);
	payload_init_raw(expr, base, offset, len);

	dreg = netlink_parse_register(nle, NFTNL_EXPR_PAYLOAD_DREG);

	if (ctx->inner)
		ctx->inner_reg = dreg;

	netlink_set_register(ctx, dreg, expr);
}

static void netlink_parse_inner(struct netlink_parse_ctx *ctx,
				const struct location *loc,
				const struct nftnl_expr *nle)
{
	const struct proto_desc *inner_desc;
	const struct nftnl_expr *inner_nle;
	uint32_t hdrsize, flags, type;
	struct expr *expr;

	hdrsize = nftnl_expr_get_u32(nle, NFTNL_EXPR_INNER_HDRSIZE);
	type    = nftnl_expr_get_u32(nle, NFTNL_EXPR_INNER_TYPE);
	flags   = nftnl_expr_get_u32(nle, NFTNL_EXPR_INNER_FLAGS);

	inner_nle = nftnl_expr_get(nle, NFTNL_EXPR_INNER_EXPR, NULL);
	if (!inner_nle) {
		netlink_error(ctx, loc, "Could not parse inner expression");
		return;
	}

	inner_desc = proto_find_inner(type, hdrsize, flags);

	ctx->inner = true;
	if (netlink_parse_expr(inner_nle, ctx) < 0) {
		ctx->inner = false;
		return;
	}
	ctx->inner = false;

	expr = netlink_get_register(ctx, loc, ctx->inner_reg);
	assert(expr);

	if (expr->etype == EXPR_PAYLOAD &&
	    expr->payload.base == PROTO_BASE_INNER_HDR) {
		const struct proto_hdr_template *tmpl;
		unsigned int i;

		for (i = 1; i < array_size(inner_desc->templates); i++) {
			tmpl = &inner_desc->templates[i];

			if (tmpl->len == 0)
				return;

			if (tmpl->offset != expr->payload.offset ||
			    tmpl->len != expr->len)
				continue;

			expr->payload.desc = inner_desc;
			expr->payload.tmpl = tmpl;
			break;
		}
	}

	switch (expr->etype) {
	case EXPR_PAYLOAD:
		expr->payload.inner_desc = inner_desc;
		break;
	case EXPR_META:
		expr->meta.inner_desc = inner_desc;
		break;
	default:
		netlink_error(ctx, loc, "Unsupported inner expression type %s",
			      expr_ops(expr)->name);
		expr_free(expr);
		return;
	}

	netlink_set_register(ctx, ctx->inner_reg, expr);
}

static void netlink_parse_payload_stmt(struct netlink_parse_ctx *ctx,
				       const struct location *loc,
				       const struct nftnl_expr *nle)
{
	enum nft_registers sreg;
	uint32_t base, offset, len;
	struct expr *expr, *val;
	struct stmt *stmt;

	base   = nftnl_expr_get_u32(nle, NFTNL_EXPR_PAYLOAD_BASE) + 1;
	offset = nftnl_expr_get_u32(nle, NFTNL_EXPR_PAYLOAD_OFFSET) * BITS_PER_BYTE;
	len    = nftnl_expr_get_u32(nle, NFTNL_EXPR_PAYLOAD_LEN) * BITS_PER_BYTE;

	sreg = netlink_parse_register(nle, NFTNL_EXPR_PAYLOAD_SREG);
	val  = netlink_get_register(ctx, loc, sreg);
	if (val == NULL)
		return netlink_error(ctx, loc,
				     "payload statement has no expression");

	expr = payload_expr_alloc(loc, NULL, 0);
	payload_init_raw(expr, base, offset, len);

	stmt = payload_stmt_alloc(loc, expr, val);
	rule_stmt_append(ctx->rule, stmt);
}

static void netlink_parse_payload(struct netlink_parse_ctx *ctx,
				  const struct location *loc,
				  const struct nftnl_expr *nle)
{
	if (nftnl_expr_is_set(nle, NFTNL_EXPR_PAYLOAD_DREG))
		netlink_parse_payload_expr(ctx, loc, nle);
	else
		netlink_parse_payload_stmt(ctx, loc, nle);
}

static void netlink_parse_exthdr(struct netlink_parse_ctx *ctx,
				 const struct location *loc,
				 const struct nftnl_expr *nle)
{
	uint32_t offset, len, flags;
	enum nft_registers dreg;
	enum nft_exthdr_op op;
	uint8_t type;
	struct expr *expr;

	type   = nftnl_expr_get_u8(nle, NFTNL_EXPR_EXTHDR_TYPE);
	offset = nftnl_expr_get_u32(nle, NFTNL_EXPR_EXTHDR_OFFSET) * BITS_PER_BYTE;
	len    = nftnl_expr_get_u32(nle, NFTNL_EXPR_EXTHDR_LEN) * BITS_PER_BYTE;
	op     = nftnl_expr_get_u32(nle, NFTNL_EXPR_EXTHDR_OP);
	flags  = nftnl_expr_get_u32(nle, NFTNL_EXPR_EXTHDR_FLAGS);

	expr = exthdr_expr_alloc(loc, NULL, 0);
	exthdr_init_raw(expr, type, offset, len, op, flags);

	if (nftnl_expr_is_set(nle, NFTNL_EXPR_EXTHDR_DREG)) {
		dreg = netlink_parse_register(nle, NFTNL_EXPR_EXTHDR_DREG);
		netlink_set_register(ctx, dreg, expr);
	} else if (nftnl_expr_is_set(nle, NFTNL_EXPR_EXTHDR_SREG)) {
		enum nft_registers sreg;
		struct stmt *stmt;
		struct expr *val;

		sreg = netlink_parse_register(nle, NFTNL_EXPR_EXTHDR_SREG);
		val = netlink_get_register(ctx, loc, sreg);
		if (val == NULL) {
			expr_free(expr);
			return netlink_error(ctx, loc,
					     "exthdr statement has no expression");
		}

		expr_set_type(val, expr->dtype, expr->byteorder);

		stmt = exthdr_stmt_alloc(loc, expr, val);
		rule_stmt_append(ctx->rule, stmt);
	} else {
		struct stmt *stmt = optstrip_stmt_alloc(loc, expr);

		rule_stmt_append(ctx->rule, stmt);
	}
}

static void netlink_parse_hash(struct netlink_parse_ctx *ctx,
			       const struct location *loc,
			       const struct nftnl_expr *nle)
{
	enum nft_registers sreg, dreg;
	struct expr *expr, *hexpr;
	uint32_t mod, seed, len, offset;
	enum nft_hash_types type;
	bool seed_set;

	type = nftnl_expr_get_u32(nle, NFTNL_EXPR_HASH_TYPE);
	offset = nftnl_expr_get_u32(nle, NFTNL_EXPR_HASH_OFFSET);
	seed_set = nftnl_expr_is_set(nle, NFTNL_EXPR_HASH_SEED);
	seed = nftnl_expr_get_u32(nle, NFTNL_EXPR_HASH_SEED);
	mod  = nftnl_expr_get_u32(nle, NFTNL_EXPR_HASH_MODULUS);

	expr = hash_expr_alloc(loc, mod, seed_set, seed, offset, type);

	if (type != NFT_HASH_SYM) {
		sreg = netlink_parse_register(nle, NFTNL_EXPR_HASH_SREG);
		hexpr = netlink_get_register(ctx, loc, sreg);

		if (hexpr == NULL) {
			netlink_error(ctx, loc,
				      "hash statement has no expression");
			goto out_err;
		}
		len = nftnl_expr_get_u32(nle,
					 NFTNL_EXPR_HASH_LEN) * BITS_PER_BYTE;
		if (hexpr->len < len) {
			expr_free(hexpr);
			hexpr = netlink_parse_concat_expr(ctx, loc, sreg, len);
			if (hexpr == NULL)
				goto out_err;
		}
		expr->hash.expr = hexpr;
	}

	dreg = netlink_parse_register(nle, NFTNL_EXPR_HASH_DREG);
	netlink_set_register(ctx, dreg, expr);
	return;
out_err:
	expr_free(expr);
}

static void netlink_parse_fib(struct netlink_parse_ctx *ctx,
			      const struct location *loc,
			      const struct nftnl_expr *nle)
{
	enum nft_registers dreg;
	struct expr *expr;
	uint32_t flags, result;

	flags = nftnl_expr_get_u32(nle, NFTNL_EXPR_FIB_FLAGS);
	result = nftnl_expr_get_u32(nle, NFTNL_EXPR_FIB_RESULT);

	expr = fib_expr_alloc(loc, flags, result);

	dreg = netlink_parse_register(nle, NFTNL_EXPR_FIB_DREG);
	netlink_set_register(ctx, dreg, expr);
}

static void netlink_parse_meta_expr(struct netlink_parse_ctx *ctx,
				    const struct location *loc,
				    const struct nftnl_expr *nle)
{
	enum nft_registers dreg;
	uint32_t key;
	struct expr *expr;

	key  = nftnl_expr_get_u32(nle, NFTNL_EXPR_META_KEY);
	expr = meta_expr_alloc(loc, key);

	dreg = netlink_parse_register(nle, NFTNL_EXPR_META_DREG);
	if (ctx->inner)
		ctx->inner_reg = dreg;

	netlink_set_register(ctx, dreg, expr);
}

static void netlink_parse_socket(struct netlink_parse_ctx *ctx,
				      const struct location *loc,
				      const struct nftnl_expr *nle)
{
	enum nft_registers dreg;
	uint32_t key, level;
	struct expr * expr;

	key = nftnl_expr_get_u32(nle, NFTNL_EXPR_SOCKET_KEY);
	level = nftnl_expr_get_u32(nle, NFTNL_EXPR_SOCKET_LEVEL);
	expr = socket_expr_alloc(loc, key, level);

	dreg = netlink_parse_register(nle, NFTNL_EXPR_SOCKET_DREG);
	netlink_set_register(ctx, dreg, expr);
}

static void netlink_parse_osf(struct netlink_parse_ctx *ctx,
			      const struct location *loc,
			      const struct nftnl_expr *nle)
{
	enum nft_registers dreg;
	struct expr *expr;
	uint32_t flags;
	uint8_t ttl;

	ttl = nftnl_expr_get_u8(nle, NFTNL_EXPR_OSF_TTL);
	flags = nftnl_expr_get_u32(nle, NFTNL_EXPR_OSF_FLAGS);
	expr = osf_expr_alloc(loc, ttl, flags);

	dreg = netlink_parse_register(nle, NFTNL_EXPR_OSF_DREG);
	netlink_set_register(ctx, dreg, expr);
}

static void netlink_parse_tunnel(struct netlink_parse_ctx *ctx,
				 const struct location *loc,
				 const struct nftnl_expr *nle)
{
	enum nft_registers dreg;
	struct expr * expr;
	uint32_t key;

	key = nftnl_expr_get_u32(nle, NFTNL_EXPR_TUNNEL_KEY);
	expr = tunnel_expr_alloc(loc, key);

	dreg = netlink_parse_register(nle, NFTNL_EXPR_TUNNEL_DREG);
	netlink_set_register(ctx, dreg, expr);
}

static void netlink_parse_meta_stmt(struct netlink_parse_ctx *ctx,
				    const struct location *loc,
				    const struct nftnl_expr *nle)
{
	enum nft_registers sreg;
	uint32_t key;
	struct stmt *stmt;
	struct expr *expr;

	sreg = netlink_parse_register(nle, NFTNL_EXPR_META_SREG);
	expr = netlink_get_register(ctx, loc, sreg);
	if (expr == NULL)
		return netlink_error(ctx, loc,
				     "meta statement has no expression");

	key  = nftnl_expr_get_u32(nle, NFTNL_EXPR_META_KEY);
	stmt = meta_stmt_alloc(loc, key, expr);

	if (stmt->meta.tmpl)
		expr_set_type(expr, stmt->meta.tmpl->dtype, stmt->meta.tmpl->byteorder);

	ctx->stmt = stmt;
}

static void netlink_parse_meta(struct netlink_parse_ctx *ctx,
			       const struct location *loc,
			       const struct nftnl_expr *nle)
{
	if (nftnl_expr_is_set(nle, NFTNL_EXPR_META_DREG))
		netlink_parse_meta_expr(ctx, loc, nle);
	else
		netlink_parse_meta_stmt(ctx, loc, nle);
}

static void netlink_parse_rt(struct netlink_parse_ctx *ctx,
			     const struct location *loc,
			     const struct nftnl_expr *nle)
{
	enum nft_registers dreg;
	uint32_t key;
	struct expr *expr;

	key  = nftnl_expr_get_u32(nle, NFTNL_EXPR_RT_KEY);
	expr = rt_expr_alloc(loc, key, false);

	dreg = netlink_parse_register(nle, NFTNL_EXPR_RT_DREG);
	netlink_set_register(ctx, dreg, expr);
}

static void netlink_parse_numgen(struct netlink_parse_ctx *ctx,
				 const struct location *loc,
				 const struct nftnl_expr *nle)
{
	uint32_t type, until, offset;
	enum nft_registers dreg;
	struct expr *expr;

	type  = nftnl_expr_get_u32(nle, NFTNL_EXPR_NG_TYPE);
	until = nftnl_expr_get_u32(nle, NFTNL_EXPR_NG_MODULUS);
	offset = nftnl_expr_get_u32(nle, NFTNL_EXPR_NG_OFFSET);

	expr = numgen_expr_alloc(loc, type, until, offset);
	dreg = netlink_parse_register(nle, NFTNL_EXPR_NG_DREG);
	netlink_set_register(ctx, dreg, expr);
}

static void netlink_parse_notrack(struct netlink_parse_ctx *ctx,
				  const struct location *loc,
				  const struct nftnl_expr *nle)
{
	ctx->stmt = notrack_stmt_alloc(loc);
}

static void netlink_parse_flow_offload(struct netlink_parse_ctx *ctx,
				       const struct location *loc,
				       const struct nftnl_expr *nle)
{
	const char *table_name;

	table_name = xstrdup(nftnl_expr_get_str(nle, NFTNL_EXPR_FLOW_TABLE_NAME));
	ctx->stmt = flow_offload_stmt_alloc(loc, table_name);
}

static void netlink_parse_ct_stmt(struct netlink_parse_ctx *ctx,
				  const struct location *loc,
				  const struct nftnl_expr *nle)
{
	enum nft_registers sreg;
	uint32_t key;
	struct stmt *stmt;
	struct expr *expr;
	int8_t dir = -1;

	sreg = netlink_parse_register(nle, NFTNL_EXPR_CT_SREG);
	expr = netlink_get_register(ctx, loc, sreg);
	if (expr == NULL)
		return netlink_error(ctx, loc,
				     "ct statement has no expression");

	if (nftnl_expr_is_set(nle, NFTNL_EXPR_CT_DIR))
		dir = nftnl_expr_get_u8(nle, NFTNL_EXPR_CT_DIR);

	key  = nftnl_expr_get_u32(nle, NFTNL_EXPR_CT_KEY);
	stmt = ct_stmt_alloc(loc, key, dir, expr);
	expr_set_type(expr, stmt->ct.tmpl->dtype, stmt->ct.tmpl->byteorder);

	ctx->stmt = stmt;
}

static void netlink_parse_ct_expr(struct netlink_parse_ctx *ctx,
				  const struct location *loc,
				  const struct nftnl_expr *nle)
{
	struct expr *expr = NULL;
	enum nft_registers dreg;
	int8_t dir = -1;
	uint32_t key;

	if (nftnl_expr_is_set(nle, NFTNL_EXPR_CT_DIR))
		dir = nftnl_expr_get_u8(nle, NFTNL_EXPR_CT_DIR);

	key  = nftnl_expr_get_u32(nle, NFTNL_EXPR_CT_KEY);
	expr = ct_expr_alloc(loc, key, dir);

	dreg = netlink_parse_register(nle, NFTNL_EXPR_CT_DREG);
	netlink_set_register(ctx, dreg, expr);
}

static void netlink_parse_ct(struct netlink_parse_ctx *ctx,
			     const struct location *loc,
			     const struct nftnl_expr *nle)
{
	if (nftnl_expr_is_set(nle, NFTNL_EXPR_CT_DREG))
		netlink_parse_ct_expr(ctx, loc, nle);
	else
		netlink_parse_ct_stmt(ctx, loc, nle);
}

static void netlink_parse_connlimit(struct netlink_parse_ctx *ctx,
				    const struct location *loc,
				    const struct nftnl_expr *nle)
{
	struct stmt *stmt;

	stmt = connlimit_stmt_alloc(loc);
	stmt->connlimit.count =
		nftnl_expr_get_u32(nle, NFTNL_EXPR_CONNLIMIT_COUNT);
	stmt->connlimit.flags =
		nftnl_expr_get_u32(nle, NFTNL_EXPR_CONNLIMIT_FLAGS);

	ctx->stmt = stmt;
}

static void netlink_parse_counter(struct netlink_parse_ctx *ctx,
				  const struct location *loc,
				  const struct nftnl_expr *nle)
{
	struct stmt *stmt;

	stmt = counter_stmt_alloc(loc);
	stmt->counter.packets = nftnl_expr_get_u64(nle, NFTNL_EXPR_CTR_PACKETS);
	stmt->counter.bytes   = nftnl_expr_get_u64(nle, NFTNL_EXPR_CTR_BYTES);

	ctx->stmt = stmt;
}

static void netlink_parse_last(struct netlink_parse_ctx *ctx,
			       const struct location *loc,
			       const struct nftnl_expr *nle)
{
	struct stmt *stmt;

	stmt = last_stmt_alloc(loc);
	stmt->last.used = nftnl_expr_get_u64(nle, NFTNL_EXPR_LAST_MSECS);
	stmt->last.set = nftnl_expr_get_u32(nle, NFTNL_EXPR_LAST_SET);

	ctx->stmt = stmt;
}

static void netlink_parse_log(struct netlink_parse_ctx *ctx,
			      const struct location *loc,
			      const struct nftnl_expr *nle)
{
	struct stmt *stmt;
	const char *prefix;

	stmt = log_stmt_alloc(loc);
	prefix = nftnl_expr_get_str(nle, NFTNL_EXPR_LOG_PREFIX);
	if (nftnl_expr_is_set(nle, NFTNL_EXPR_LOG_PREFIX)) {
		stmt->log.prefix = xstrdup(prefix);
		stmt->log.flags |= STMT_LOG_PREFIX;
	}
	if (nftnl_expr_is_set(nle, NFTNL_EXPR_LOG_GROUP)) {
		stmt->log.group = nftnl_expr_get_u16(nle, NFTNL_EXPR_LOG_GROUP);
		stmt->log.flags |= STMT_LOG_GROUP;
	}
	if (nftnl_expr_is_set(nle, NFTNL_EXPR_LOG_SNAPLEN)) {
		stmt->log.snaplen =
			nftnl_expr_get_u32(nle, NFTNL_EXPR_LOG_SNAPLEN);
		stmt->log.flags |= STMT_LOG_SNAPLEN;
	}
	if (nftnl_expr_is_set(nle, NFTNL_EXPR_LOG_QTHRESHOLD)) {
		stmt->log.qthreshold =
			nftnl_expr_get_u16(nle, NFTNL_EXPR_LOG_QTHRESHOLD);
		stmt->log.flags |= STMT_LOG_QTHRESHOLD;
	}
	if (nftnl_expr_is_set(nle, NFTNL_EXPR_LOG_LEVEL)) {
		stmt->log.level =
			nftnl_expr_get_u32(nle, NFTNL_EXPR_LOG_LEVEL);
		stmt->log.flags |= STMT_LOG_LEVEL;
	}
	if (nftnl_expr_is_set(nle, NFTNL_EXPR_LOG_FLAGS)) {
		stmt->log.logflags =
			nftnl_expr_get_u32(nle, NFTNL_EXPR_LOG_FLAGS);
	}

	ctx->stmt = stmt;
}

static void netlink_parse_limit(struct netlink_parse_ctx *ctx,
				const struct location *loc,
				const struct nftnl_expr *nle)
{
	struct stmt *stmt;

	stmt = limit_stmt_alloc(loc);
	stmt->limit.rate = nftnl_expr_get_u64(nle, NFTNL_EXPR_LIMIT_RATE);
	stmt->limit.unit = nftnl_expr_get_u64(nle, NFTNL_EXPR_LIMIT_UNIT);
	stmt->limit.type = nftnl_expr_get_u32(nle, NFTNL_EXPR_LIMIT_TYPE);
	stmt->limit.burst = nftnl_expr_get_u32(nle, NFTNL_EXPR_LIMIT_BURST);
	stmt->limit.flags = nftnl_expr_get_u32(nle, NFTNL_EXPR_LIMIT_FLAGS);

	ctx->stmt = stmt;
}

static void netlink_parse_quota(struct netlink_parse_ctx *ctx,
				const struct location *loc,
				const struct nftnl_expr *nle)
{
	struct stmt *stmt;

	stmt = quota_stmt_alloc(loc);
	stmt->quota.bytes = nftnl_expr_get_u64(nle, NFTNL_EXPR_QUOTA_BYTES);
	stmt->quota.used =
		nftnl_expr_get_u64(nle, NFTNL_EXPR_QUOTA_CONSUMED);
	stmt->quota.flags = nftnl_expr_get_u32(nle, NFTNL_EXPR_QUOTA_FLAGS);

	ctx->stmt = stmt;
}

static void netlink_parse_reject(struct netlink_parse_ctx *ctx,
				 const struct location *loc,
				 const struct nftnl_expr *expr)
{
	struct stmt *stmt;
	uint8_t icmp_code;

	stmt = reject_stmt_alloc(loc);
	stmt->reject.type = nftnl_expr_get_u32(expr, NFTNL_EXPR_REJECT_TYPE);
	icmp_code = nftnl_expr_get_u8(expr, NFTNL_EXPR_REJECT_CODE);
	stmt->reject.icmp_code = icmp_code;
	stmt->reject.expr = constant_expr_alloc(loc, &integer_type,
						BYTEORDER_HOST_ENDIAN, 8,
						&icmp_code);
	ctx->stmt = stmt;
}

static bool is_nat_addr_map(const struct expr *addr, uint8_t family,
			    struct stmt *stmt)
{
	const struct expr *mappings, *data;
	const struct set *set;

	if (!addr ||
	    expr_ops(addr)->type != EXPR_MAP)
		return false;

	mappings = addr->right;
	if (expr_ops(mappings)->type != EXPR_SET_REF)
		return false;

	set = mappings->set;
	data = set->data;

	if (!(data->flags & EXPR_F_INTERVAL))
		return false;

	stmt->nat.family = family;

	/* if we're dealing with an address:address map,
	 * the length will be bit_sizeof(addr) + 32 (one register).
	 */
	switch (family) {
	case NFPROTO_IPV4:
		if (data->len == 32 + 32) {
			stmt->nat.type_flags |= STMT_NAT_F_INTERVAL;
			return true;
		} else if (data->len == 32 + 32 + 32 + 32) {
			stmt->nat.type_flags |= STMT_NAT_F_INTERVAL |
						STMT_NAT_F_CONCAT;
			return true;
		}
		break;
	case NFPROTO_IPV6:
		if (data->len == 128 + 128) {
			stmt->nat.type_flags |= STMT_NAT_F_INTERVAL;
			return true;
		} else if (data->len == 128 + 32 + 128 + 32) {
			stmt->nat.type_flags |= STMT_NAT_F_INTERVAL |
						STMT_NAT_F_CONCAT;
			return true;
		}
	}

	return false;
}

static bool is_nat_proto_map(const struct expr *addr, uint8_t family)
{
	const struct expr *mappings, *data;
	const struct set *set;

	if (!addr ||
	    expr_ops(addr)->type != EXPR_MAP)
		return false;

	mappings = addr->right;
	if (expr_ops(mappings)->type != EXPR_SET_REF)
		return false;

	set = mappings->set;
	data = set->data;

	/* if we're dealing with an address:inet_service map,
	 * the length will be bit_sizeof(addr) + 32 (one register).
	 */
	switch (family) {
	case NFPROTO_IPV4:
		return data->len == 32 + 32;
	case NFPROTO_IPV6:
		return data->len == 128 + 32;
	}

	return false;
}

static void netlink_parse_nat(struct netlink_parse_ctx *ctx,
			      const struct location *loc,
			      const struct nftnl_expr *nle)
{
	struct stmt *stmt;
	struct expr *addr, *proto;
	enum nft_registers reg1, reg2;
	int family;

	stmt = nat_stmt_alloc(loc,
			      nftnl_expr_get_u32(nle, NFTNL_EXPR_NAT_TYPE));

	family = nftnl_expr_get_u32(nle, NFTNL_EXPR_NAT_FAMILY);

	if (ctx->table->handle.family == NFPROTO_INET)
		stmt->nat.family = family;

	if (nftnl_expr_is_set(nle, NFTNL_EXPR_NAT_FLAGS))
		stmt->nat.flags = nftnl_expr_get_u32(nle, NFTNL_EXPR_NAT_FLAGS);

	if (stmt->nat.flags & NF_NAT_RANGE_NETMAP)
		stmt->nat.type_flags |= STMT_NAT_F_PREFIX;

	addr = NULL;
	reg1 = netlink_parse_register(nle, NFTNL_EXPR_NAT_REG_ADDR_MIN);
	if (reg1) {
		addr = netlink_get_register(ctx, loc, reg1);
		if (addr == NULL) {
			netlink_error(ctx, loc,
				      "NAT statement has no address expression");
			goto out_err;
		}

		if (family == NFPROTO_IPV4)
			expr_set_type(addr, &ipaddr_type, BYTEORDER_BIG_ENDIAN);
		else
			expr_set_type(addr, &ip6addr_type,
				      BYTEORDER_BIG_ENDIAN);
		stmt->nat.addr = addr;
	}

	if (is_nat_addr_map(addr, family, stmt)) {
		stmt->nat.family = family;
		ctx->stmt = stmt;
		return;
	}

	reg2 = netlink_parse_register(nle, NFTNL_EXPR_NAT_REG_ADDR_MAX);
	if (reg2 && reg2 != reg1) {
		addr = netlink_get_register(ctx, loc, reg2);
		if (addr == NULL) {
			netlink_error(ctx, loc,
				      "NAT statement has no address expression");
			goto out_err;
		}

		if (family == NFPROTO_IPV4)
			expr_set_type(addr, &ipaddr_type, BYTEORDER_BIG_ENDIAN);
		else
			expr_set_type(addr, &ip6addr_type,
				      BYTEORDER_BIG_ENDIAN);
		if (stmt->nat.addr != NULL) {
			addr = range_expr_alloc(loc, stmt->nat.addr, addr);
			addr = range_expr_to_prefix(addr);
		}
		stmt->nat.addr = addr;
	}

	if (is_nat_proto_map(addr, family)) {
		stmt->nat.family = family;
		stmt->nat.type_flags |= STMT_NAT_F_CONCAT;
		ctx->stmt = stmt;
		return;
	}

	reg1 = netlink_parse_register(nle, NFTNL_EXPR_NAT_REG_PROTO_MIN);
	if (reg1) {
		proto = netlink_get_register(ctx, loc, reg1);
		if (proto == NULL) {
			netlink_error(ctx, loc,
				      "NAT statement has no proto expression");
			goto out_err;
		}

		expr_set_type(proto, &inet_service_type, BYTEORDER_BIG_ENDIAN);
		stmt->nat.proto = proto;
	}

	reg2 = netlink_parse_register(nle, NFTNL_EXPR_NAT_REG_PROTO_MAX);
	if (reg2 && reg2 != reg1) {
		proto = netlink_get_register(ctx, loc, reg2);
		if (proto == NULL) {
			netlink_error(ctx, loc,
				      "NAT statement has no proto expression");
			goto out_err;
		}

		expr_set_type(proto, &inet_service_type, BYTEORDER_BIG_ENDIAN);
		if (stmt->nat.proto != NULL)
			proto = range_expr_alloc(loc, stmt->nat.proto, proto);
		stmt->nat.proto = proto;
	}

	ctx->stmt = stmt;
	return;
out_err:
	stmt_free(stmt);
}

static void netlink_parse_synproxy(struct netlink_parse_ctx *ctx,
				   const struct location *loc,
				   const struct nftnl_expr *nle)
{
	struct stmt *stmt;

	stmt = synproxy_stmt_alloc(loc);
	stmt->synproxy.mss = nftnl_expr_get_u16(nle, NFTNL_EXPR_SYNPROXY_MSS);
	stmt->synproxy.wscale = nftnl_expr_get_u8(nle,
						  NFTNL_EXPR_SYNPROXY_WSCALE);
	stmt->synproxy.flags = nftnl_expr_get_u32(nle,
						  NFTNL_EXPR_SYNPROXY_FLAGS);

	ctx->stmt = stmt;
}

static void netlink_parse_tproxy(struct netlink_parse_ctx *ctx,
			      const struct location *loc,
			      const struct nftnl_expr *nle)
{
	struct stmt *stmt;
	struct expr *addr, *port;
	enum nft_registers reg;

	stmt = tproxy_stmt_alloc(loc);
	stmt->tproxy.family = nftnl_expr_get_u32(nle, NFTNL_EXPR_TPROXY_FAMILY);
	stmt->tproxy.table_family = ctx->table->handle.family;

	reg = netlink_parse_register(nle, NFTNL_EXPR_TPROXY_REG_ADDR);
	if (reg) {
		addr = netlink_get_register(ctx, loc, reg);
		if (addr == NULL)
			goto err;

		switch (stmt->tproxy.family) {
		case NFPROTO_IPV4:
			expr_set_type(addr, &ipaddr_type, BYTEORDER_BIG_ENDIAN);
			break;
		case NFPROTO_IPV6:
			expr_set_type(addr, &ip6addr_type, BYTEORDER_BIG_ENDIAN);
			break;
		default:
			netlink_error(ctx, loc,
				      "tproxy address must be IPv4 or IPv6");
			goto err;
		}
		stmt->tproxy.addr = addr;
	}

	reg = netlink_parse_register(nle, NFTNL_EXPR_TPROXY_REG_PORT);
	if (reg) {
		port = netlink_get_register(ctx, loc, reg);
		if (port == NULL)
			goto err;
		expr_set_type(port, &inet_service_type, BYTEORDER_BIG_ENDIAN);
		stmt->tproxy.port = port;
	}

	ctx->stmt = stmt;
	return;
err:
	stmt_free(stmt);
}

static void netlink_parse_masq(struct netlink_parse_ctx *ctx,
			       const struct location *loc,
			       const struct nftnl_expr *nle)
{
	enum nft_registers reg1, reg2;
	struct expr *proto;
	struct stmt *stmt;
	uint32_t flags = 0;

	if (nftnl_expr_is_set(nle, NFTNL_EXPR_MASQ_FLAGS))
		flags = nftnl_expr_get_u32(nle, NFTNL_EXPR_MASQ_FLAGS);

	stmt = nat_stmt_alloc(loc, NFT_NAT_MASQ);
	stmt->nat.flags = flags;

	reg1 = netlink_parse_register(nle, NFTNL_EXPR_MASQ_REG_PROTO_MIN);
	if (reg1) {
		proto = netlink_get_register(ctx, loc, reg1);
		if (proto == NULL) {
			netlink_error(ctx, loc,
				      "MASQUERADE statement has no proto expression");
			goto out_err;
		}
		expr_set_type(proto, &inet_service_type, BYTEORDER_BIG_ENDIAN);
		stmt->nat.proto = proto;
	}

	reg2 = netlink_parse_register(nle, NFTNL_EXPR_MASQ_REG_PROTO_MAX);
	if (reg2 && reg2 != reg1) {
		proto = netlink_get_register(ctx, loc, reg2);
		if (proto == NULL) {
			netlink_error(ctx, loc,
				      "MASQUERADE statement has no proto expression");
			goto out_err;
		}
		expr_set_type(proto, &inet_service_type, BYTEORDER_BIG_ENDIAN);
		if (stmt->nat.proto != NULL)
			proto = range_expr_alloc(loc, stmt->nat.proto, proto);
		stmt->nat.proto = proto;
	}

	ctx->stmt = stmt;
	return;
out_err:
	stmt_free(stmt);
}

static void netlink_parse_redir(struct netlink_parse_ctx *ctx,
				const struct location *loc,
				const struct nftnl_expr *nle)
{
	struct stmt *stmt;
	struct expr *proto;
	enum nft_registers reg1, reg2;
	uint32_t flags;

	stmt = nat_stmt_alloc(loc, NFT_NAT_REDIR);

	if (nftnl_expr_is_set(nle, NFTNL_EXPR_REDIR_FLAGS)) {
		flags = nftnl_expr_get_u32(nle, NFTNL_EXPR_REDIR_FLAGS);
		stmt->nat.flags = flags;
	}

	reg1 = netlink_parse_register(nle, NFTNL_EXPR_REDIR_REG_PROTO_MIN);
	if (reg1) {
		proto = netlink_get_register(ctx, loc, reg1);
		if (proto == NULL) {
			netlink_error(ctx, loc,
				      "redirect statement has no proto expression");
			goto out_err;
		}

		expr_set_type(proto, &inet_service_type, BYTEORDER_BIG_ENDIAN);
		stmt->nat.proto = proto;
	}

	reg2 = netlink_parse_register(nle, NFTNL_EXPR_REDIR_REG_PROTO_MAX);
	if (reg2 && reg2 != reg1) {
		proto = netlink_get_register(ctx, loc, reg2);
		if (proto == NULL) {
			netlink_error(ctx, loc,
				      "redirect statement has no proto expression");
			goto out_err;
		}

		expr_set_type(proto, &inet_service_type, BYTEORDER_BIG_ENDIAN);
		if (stmt->nat.proto != NULL)
			proto = range_expr_alloc(loc, stmt->nat.proto,
						 proto);
		stmt->nat.proto = proto;
	}

	ctx->stmt = stmt;
	return;
out_err:
	stmt_free(stmt);
}

static void netlink_parse_dup(struct netlink_parse_ctx *ctx,
			      const struct location *loc,
			      const struct nftnl_expr *nle)
{
	enum nft_registers reg1, reg2;
	struct expr *addr, *dev;
	struct stmt *stmt;

	stmt = dup_stmt_alloc(loc);

	reg1 = netlink_parse_register(nle, NFTNL_EXPR_DUP_SREG_ADDR);
	if (reg1) {
		addr = netlink_get_register(ctx, loc, reg1);
		if (addr == NULL) {
			netlink_error(ctx, loc,
				      "DUP statement has no destination expression");
			goto out_err;
		}

		switch (ctx->table->handle.family) {
		case NFPROTO_IPV4:
			expr_set_type(addr, &ipaddr_type, BYTEORDER_BIG_ENDIAN);
			break;
		case NFPROTO_IPV6:
			expr_set_type(addr, &ip6addr_type,
				      BYTEORDER_BIG_ENDIAN);
			break;
		}
		stmt->dup.to = addr;
	}

	reg2 = netlink_parse_register(nle, NFTNL_EXPR_DUP_SREG_DEV);
	if (reg2) {
		dev = netlink_get_register(ctx, loc, reg2);
		if (dev == NULL) {
			netlink_error(ctx, loc,
				      "DUP statement has no output expression");
			goto out_err;
		}

		expr_set_type(dev, &ifindex_type, BYTEORDER_HOST_ENDIAN);
		if (stmt->dup.to == NULL)
			stmt->dup.to = dev;
		else
			stmt->dup.dev = dev;
	}

	ctx->stmt = stmt;
	return;
out_err:
	stmt_free(stmt);
}

static void netlink_parse_fwd(struct netlink_parse_ctx *ctx,
			      const struct location *loc,
			      const struct nftnl_expr *nle)
{
	enum nft_registers reg1, reg2;
	struct expr *dev, *addr;
	struct stmt *stmt;

	stmt = fwd_stmt_alloc(loc);

	reg1 = netlink_parse_register(nle, NFTNL_EXPR_FWD_SREG_DEV);
	if (reg1) {
		dev = netlink_get_register(ctx, loc, reg1);
		if (dev == NULL) {
			netlink_error(ctx, loc,
				      "fwd statement has no output expression");
			goto out_err;
		}

		expr_set_type(dev, &ifindex_type, BYTEORDER_HOST_ENDIAN);
		stmt->fwd.dev = dev;
	}

	if (nftnl_expr_is_set(nle, NFTNL_EXPR_FWD_NFPROTO)) {
		stmt->fwd.family =
			nftnl_expr_get_u32(nle, NFTNL_EXPR_FWD_NFPROTO);
	}

	if (nftnl_expr_is_set(nle, NFTNL_EXPR_FWD_SREG_ADDR)) {
		reg2 = netlink_parse_register(nle, NFTNL_EXPR_FWD_SREG_ADDR);
		if (reg2) {
			addr = netlink_get_register(ctx, loc, reg2);
			if (addr == NULL) {
				netlink_error(ctx, loc,
					      "fwd statement has no output expression");
				goto out_err;
			}

			switch (stmt->fwd.family) {
			case AF_INET:
				expr_set_type(addr, &ipaddr_type,
					      BYTEORDER_BIG_ENDIAN);
				break;
			case AF_INET6:
				expr_set_type(addr, &ip6addr_type,
					      BYTEORDER_BIG_ENDIAN);
				break;
			default:
				netlink_error(ctx, loc,
					      "fwd statement has no family");
				goto out_err;
			}
			stmt->fwd.addr = addr;
		}
	}

	ctx->stmt = stmt;
	return;
out_err:
	stmt_free(stmt);
}

static void netlink_parse_queue(struct netlink_parse_ctx *ctx,
			      const struct location *loc,
			      const struct nftnl_expr *nle)
{
	struct expr *expr;
	uint16_t flags;

	if (nftnl_expr_is_set(nle, NFTNL_EXPR_QUEUE_SREG_QNUM)) {
		enum nft_registers reg = netlink_parse_register(nle, NFTNL_EXPR_QUEUE_SREG_QNUM);

		expr = netlink_get_register(ctx, loc, reg);
		if (!expr) {
			netlink_error(ctx, loc, "queue statement has no sreg expression");
			return;
		}
	} else {
		uint16_t total = nftnl_expr_get_u16(nle, NFTNL_EXPR_QUEUE_TOTAL);
		uint16_t num = nftnl_expr_get_u16(nle, NFTNL_EXPR_QUEUE_NUM);

		expr = constant_expr_alloc(loc, &integer_type,
					   BYTEORDER_HOST_ENDIAN, 16, &num);

		if (total > 1) {
			struct expr *high;

			total += num - 1;
			high = constant_expr_alloc(loc, &integer_type,
					   BYTEORDER_HOST_ENDIAN, 16, &total);
			expr = range_expr_alloc(loc, expr, high);
		}
	}

	flags = nftnl_expr_get_u16(nle, NFTNL_EXPR_QUEUE_FLAGS);
	ctx->stmt = queue_stmt_alloc(loc, expr, flags);
}

struct dynset_parse_ctx {
	struct netlink_parse_ctx	*nlctx;
	const struct location		*loc;
	struct list_head		stmt_list;
};

static int dynset_parse_expressions(struct nftnl_expr *e, void *data)
{
	struct dynset_parse_ctx *dynset_parse_ctx = data;
	struct netlink_parse_ctx *ctx = dynset_parse_ctx->nlctx;
	const struct location *loc = dynset_parse_ctx->loc;
	struct stmt *stmt;

	if (netlink_parse_expr(e, ctx) < 0 || !ctx->stmt) {
		netlink_error(ctx, loc, "Could not parse dynset stmt");
		return -1;
	}
	stmt = ctx->stmt;

	list_add_tail(&stmt->list, &dynset_parse_ctx->stmt_list);

	return 0;
}

static void netlink_parse_dynset(struct netlink_parse_ctx *ctx,
				 const struct location *loc,
				 const struct nftnl_expr *nle)
{
	struct dynset_parse_ctx dynset_parse_ctx = {
		.nlctx	= ctx,
		.loc	= loc,
	};
	struct expr *expr, *expr_data = NULL;
	enum nft_registers sreg, sreg_data;
	struct stmt *stmt, *dstmt, *next;
	const struct nftnl_expr *dnle;
	struct set *set;
	const char *name;

	init_list_head(&dynset_parse_ctx.stmt_list);

	name = nftnl_expr_get_str(nle, NFTNL_EXPR_DYNSET_SET_NAME);
	set  = set_cache_find(ctx->table, name);
	if (set == NULL)
		return netlink_error(ctx, loc,
				     "Unknown set '%s' in dynset statement",
				     name);

	sreg = netlink_parse_register(nle, NFTNL_EXPR_DYNSET_SREG_KEY);
	expr = netlink_get_register(ctx, loc, sreg);
	if (expr == NULL)
		return netlink_error(ctx, loc,
				     "Dynset statement has no key expression");

	if (expr->len < set->key->len) {
		expr_free(expr);
		expr = netlink_parse_concat_key(ctx, loc, sreg, set->key);
		if (expr == NULL)
			return;
	} else if (expr->dtype == &invalid_type) {
		expr_set_type(expr, datatype_get(set->key->dtype), set->key->byteorder);
	}

	expr = set_elem_expr_alloc(&expr->location, expr);
	expr->timeout = nftnl_expr_get_u64(nle, NFTNL_EXPR_DYNSET_TIMEOUT);

	if (nftnl_expr_is_set(nle, NFTNL_EXPR_DYNSET_EXPR)) {
		dstmt = NULL;
		dnle = nftnl_expr_get(nle, NFTNL_EXPR_DYNSET_EXPR, NULL);
		if (dnle != NULL) {
			if (netlink_parse_expr(dnle, ctx) < 0)
				goto out_err;
			if (ctx->stmt == NULL) {
				netlink_error(ctx, loc,
					      "Could not parse dynset stmt");
				goto out_err;
			}
			dstmt = ctx->stmt;
			list_add_tail(&dstmt->list,
				      &dynset_parse_ctx.stmt_list);
		}
	} else if (nftnl_expr_is_set(nle, NFTNL_EXPR_DYNSET_EXPRESSIONS)) {
		if (nftnl_expr_expr_foreach(nle, dynset_parse_expressions,
					    &dynset_parse_ctx) < 0)
			goto out_err;
	}

	if (nftnl_expr_is_set(nle, NFTNL_EXPR_DYNSET_SREG_DATA)) {
		sreg_data = netlink_parse_register(nle, NFTNL_EXPR_DYNSET_SREG_DATA);
		expr_data = netlink_get_register(ctx, loc, sreg_data);

		if (expr_data && expr_data->len < set->data->len) {
			expr_free(expr_data);
			expr_data = netlink_parse_concat_expr(ctx, loc, sreg_data, set->data->len);
			if (expr_data == NULL)
				netlink_error(ctx, loc,
					      "Could not parse dynset map data expressions");
		}
	}

	if (expr_data != NULL) {
		expr_set_type(expr_data, set->data->dtype, set->data->byteorder);
		stmt = map_stmt_alloc(loc);
		stmt->map.set	= set_ref_expr_alloc(loc, set);
		stmt->map.key	= expr;
		stmt->map.data	= expr_data;
		stmt->map.op	= nftnl_expr_get_u32(nle, NFTNL_EXPR_DYNSET_OP);
		list_splice_tail(&dynset_parse_ctx.stmt_list,
				 &stmt->map.stmt_list);
	} else {
		if (!list_empty(&dynset_parse_ctx.stmt_list) &&
		    set_is_anonymous(set->flags)) {
			stmt = meter_stmt_alloc(loc);
			stmt->meter.set  = set_ref_expr_alloc(loc, set);
			stmt->meter.key  = expr;
			stmt->meter.stmt = list_first_entry(&dynset_parse_ctx.stmt_list,
							    struct stmt, list);
			stmt->meter.size = set->desc.size;
		} else {
			stmt = set_stmt_alloc(loc);
			stmt->set.set   = set_ref_expr_alloc(loc, set);
			stmt->set.op    = nftnl_expr_get_u32(nle, NFTNL_EXPR_DYNSET_OP);
			stmt->set.key   = expr;
			list_splice_tail(&dynset_parse_ctx.stmt_list,
					 &stmt->set.stmt_list);
		}
	}

	ctx->stmt = stmt;
	return;
out_err:
	list_for_each_entry_safe(dstmt, next, &dynset_parse_ctx.stmt_list, list)
		stmt_free(dstmt);

	expr_free(expr);
}

static void netlink_parse_objref(struct netlink_parse_ctx *ctx,
				 const struct location *loc,
				 const struct nftnl_expr *nle)
{
	uint32_t type = nftnl_expr_get_u32(nle, NFTNL_EXPR_OBJREF_IMM_TYPE);
	struct expr *expr;
	struct stmt *stmt;

	if (nftnl_expr_is_set(nle, NFTNL_EXPR_OBJREF_IMM_NAME)) {
		struct nft_data_delinearize nld;

		type = nftnl_expr_get_u32(nle, NFTNL_EXPR_OBJREF_IMM_TYPE);
		nld.value = nftnl_expr_get(nle, NFTNL_EXPR_OBJREF_IMM_NAME,
					   &nld.len);
		expr = netlink_alloc_value(&netlink_location, &nld);
		datatype_set(expr, &string_type);
		expr->byteorder = BYTEORDER_HOST_ENDIAN;
	} else if (nftnl_expr_is_set(nle, NFTNL_EXPR_OBJREF_SET_SREG)) {
		struct expr *left, *right;
		enum nft_registers sreg;
		const char *name;
		struct set *set;

		name = nftnl_expr_get_str(nle, NFTNL_EXPR_OBJREF_SET_NAME);
		set  = set_cache_find(ctx->table, name);
		if (set == NULL)
			return netlink_error(ctx, loc,
					     "Unknown set '%s' in objref expression",
					     name);

		sreg = netlink_parse_register(nle, NFTNL_EXPR_OBJREF_SET_SREG);
		left = netlink_get_register(ctx, loc, sreg);
		if (left == NULL)
			return netlink_error(ctx, loc,
					     "objref expression has no left hand side");

		if (left->len < set->key->len) {
			expr_free(left);
			left = netlink_parse_concat_expr(ctx, loc, sreg, set->key->len);
			if (left == NULL)
				return;
		}

		right = set_ref_expr_alloc(loc, set);
		expr = map_expr_alloc(loc, left, right);
		expr_set_type(expr, &string_type, BYTEORDER_HOST_ENDIAN);
		type = set->objtype;
	} else {
		netlink_error(ctx, loc, "unknown objref expression type %u",
			      type);
		return;
	}

	stmt = objref_stmt_alloc(loc);
	stmt->objref.type = type;
	stmt->objref.expr = expr;
	ctx->stmt = stmt;
}

struct expr_handler {
	const char	*name;
	void		(*parse)(struct netlink_parse_ctx *ctx,
				 const struct location *loc,
				 const struct nftnl_expr *nle);
};

static const struct expr_handler netlink_parsers[] = {
	{ .name = "immediate",	.parse = netlink_parse_immediate },
	{ .name = "cmp",	.parse = netlink_parse_cmp },
	{ .name = "lookup",	.parse = netlink_parse_lookup },
	{ .name = "bitwise",	.parse = netlink_parse_bitwise },
	{ .name = "byteorder",	.parse = netlink_parse_byteorder },
	{ .name = "payload",	.parse = netlink_parse_payload },
	{ .name = "inner",	.parse = netlink_parse_inner },
	{ .name = "exthdr",	.parse = netlink_parse_exthdr },
	{ .name = "meta",	.parse = netlink_parse_meta },
	{ .name = "socket",	.parse = netlink_parse_socket },
	{ .name = "tunnel",	.parse = netlink_parse_tunnel },
	{ .name = "osf",	.parse = netlink_parse_osf },
	{ .name = "rt",		.parse = netlink_parse_rt },
	{ .name = "ct",		.parse = netlink_parse_ct },
	{ .name = "connlimit",	.parse = netlink_parse_connlimit },
	{ .name = "counter",	.parse = netlink_parse_counter },
	{ .name = "last",	.parse = netlink_parse_last },
	{ .name = "log",	.parse = netlink_parse_log },
	{ .name = "limit",	.parse = netlink_parse_limit },
	{ .name = "range",	.parse = netlink_parse_range },
	{ .name = "reject",	.parse = netlink_parse_reject },
	{ .name = "nat",	.parse = netlink_parse_nat },
	{ .name = "tproxy",	.parse = netlink_parse_tproxy },
	{ .name = "notrack",	.parse = netlink_parse_notrack },
	{ .name = "masq",	.parse = netlink_parse_masq },
	{ .name = "redir",	.parse = netlink_parse_redir },
	{ .name = "dup",	.parse = netlink_parse_dup },
	{ .name = "queue",	.parse = netlink_parse_queue },
	{ .name = "dynset",	.parse = netlink_parse_dynset },
	{ .name = "fwd",	.parse = netlink_parse_fwd },
	{ .name = "target",	.parse = netlink_parse_target },
	{ .name = "match",	.parse = netlink_parse_match },
	{ .name = "objref",	.parse = netlink_parse_objref },
	{ .name = "quota",	.parse = netlink_parse_quota },
	{ .name = "numgen",	.parse = netlink_parse_numgen },
	{ .name = "hash",	.parse = netlink_parse_hash },
	{ .name = "fib",	.parse = netlink_parse_fib },
	{ .name = "tcpopt",	.parse = netlink_parse_exthdr },
	{ .name = "flow_offload", .parse = netlink_parse_flow_offload },
	{ .name = "xfrm",	.parse = netlink_parse_xfrm },
	{ .name = "synproxy",	.parse = netlink_parse_synproxy },
};

static int netlink_parse_expr(const struct nftnl_expr *nle,
			      struct netlink_parse_ctx *ctx)
{
	const char *type = nftnl_expr_get_str(nle, NFTNL_EXPR_NAME);
	struct location loc;
	unsigned int i;

	memset(&loc, 0, sizeof(loc));
	loc.indesc = &indesc_netlink;
	loc.nle = nle;

	for (i = 0; i < array_size(netlink_parsers); i++) {
		if (strcmp(type, netlink_parsers[i].name))
			continue;

		netlink_parsers[i].parse(ctx, &loc, nle);

		return 0;
	}
	netlink_error(ctx, &loc, "unknown expression type '%s'", type);

	return 0;
}

static int netlink_parse_rule_expr(struct nftnl_expr *nle, void *arg)
{
	struct netlink_parse_ctx *ctx = arg;
	int err;

	err = netlink_parse_expr(nle, ctx);
	if (err < 0)
		return err;
	if (ctx->stmt != NULL) {
		rule_stmt_append(ctx->rule, ctx->stmt);
		ctx->stmt = NULL;
	}
	return 0;
}

struct stmt *netlink_parse_set_expr(const struct set *set,
				    const struct nft_cache *cache,
				    const struct nftnl_expr *nle)
{
	struct netlink_parse_ctx ctx, *pctx = &ctx;
	struct handle h = {};

	handle_merge(&h, &set->handle);
	pctx->rule = rule_alloc(&netlink_location, &h);
	pctx->table = table_cache_find(&cache->table_cache,
				       set->handle.table.name,
				       set->handle.family);
	assert(pctx->table != NULL);

	if (netlink_parse_expr(nle, pctx) < 0)
		return NULL;

	init_list_head(&pctx->rule->stmts);
	rule_free(pctx->rule);

	return pctx->stmt;
}

static bool meta_outer_may_dependency_kill(struct rule_pp_ctx *ctx,
					   const struct expr *expr)
{
	struct dl_proto_ctx *dl_outer = dl_proto_ctx_outer(ctx);
	struct stmt *stmt = dl_outer->pdctx.pdeps[expr->payload.inner_desc->base];
	struct expr *dep;
	uint8_t l4proto;

	if (!stmt)
		return false;

	dep = stmt->expr;

	if (dep->left->meta.key != NFT_META_L4PROTO)
		return false;

	l4proto = mpz_get_uint8(dep->right->value);

	switch (l4proto) {
	case IPPROTO_GRE:
		if (expr->payload.inner_desc == &proto_gre ||
		    expr->payload.inner_desc == &proto_gretap)
			return true;
		break;
	default:
		break;
	}

	return false;
}

static void expr_postprocess(struct rule_pp_ctx *ctx, struct expr **exprp);

static void payload_match_expand(struct rule_pp_ctx *ctx,
				 struct expr *expr,
				 struct expr *payload)
{
	struct expr *left = payload, *right = expr->right, *tmp;
	struct list_head list = LIST_HEAD_INIT(list);
	struct dl_proto_ctx *dl = dl_proto_ctx(ctx);
	enum proto_bases base = left->payload.base;
	struct expr *nexpr = NULL;
	struct stmt *nstmt;

	payload_expr_expand(&list, left, &dl->pctx);

	list_for_each_entry(left, &list, list) {
		tmp = constant_expr_splice(right, left->len);
		expr_set_type(tmp, left->dtype, left->byteorder);

		if (left->payload.tmpl && (left->len < left->payload.tmpl->len)) {
			mpz_lshift_ui(tmp->value, left->payload.tmpl->len - left->len);
			tmp->len = left->payload.tmpl->len;
			tmp = prefix_expr_alloc(&tmp->location, tmp, left->len);
		}

		nexpr = relational_expr_alloc(&expr->location, expr->op,
					      left, tmp);
		if (expr->op == OP_EQ)
			relational_expr_pctx_update(&dl->pctx, nexpr);

		nstmt = expr_stmt_alloc(&ctx->stmt->location, nexpr);
		list_add_tail(&nstmt->list, &ctx->stmt->list);

		assert(left->etype == EXPR_PAYLOAD);
		assert(left->payload.base);
		assert(base == left->payload.base);

		if (expr->left->payload.inner_desc) {
			if (expr->left->payload.inner_desc == expr->left->payload.desc) {
				nexpr->left->payload.desc = expr->left->payload.desc;
				nexpr->left->payload.tmpl = expr->left->payload.tmpl;
			}
			nexpr->left->payload.inner_desc = expr->left->payload.inner_desc;

			if (meta_outer_may_dependency_kill(ctx, expr->left)) {
				struct dl_proto_ctx *dl_outer = dl_proto_ctx_outer(ctx);

				payload_dependency_release(&dl_outer->pdctx, expr->left->payload.inner_desc->base);
			}
		}

		if (payload_is_stacked(dl->pctx.protocol[base].desc, nexpr))
			base--;

		/* Remember the first payload protocol expression to
		 * kill it later on if made redundant by a higher layer
		 * payload expression.
		 */
		payload_dependency_kill(&dl->pdctx, nexpr->left,
					dl->pctx.family);
		expr_set_type(tmp, nexpr->left->dtype, nexpr->byteorder);
		if (expr->op == OP_EQ && left->flags & EXPR_F_PROTOCOL)
			payload_dependency_store(&dl->pdctx, nstmt, base);
	}
	list_del(&ctx->stmt->list);
	stmt_free(ctx->stmt);
	ctx->stmt = NULL;
}

static void payload_icmp_check(struct rule_pp_ctx *rctx, struct expr *expr, const struct expr *value)
{
	struct dl_proto_ctx *dl = dl_proto_ctx(rctx);
	const struct proto_hdr_template *tmpl;
	const struct proto_desc *desc;
	uint8_t icmp_type;
	unsigned int i;

	assert(expr->etype == EXPR_PAYLOAD);
	assert(value->etype == EXPR_VALUE);

	if (expr->payload.base != PROTO_BASE_TRANSPORT_HDR)
		return;

	/* icmp(v6) type is 8 bit, if value is smaller or larger, this is not
	 * a protocol dependency.
	 */
	if (expr->len != 8 || value->len != 8 || dl->pctx.th_dep.icmp.type)
		return;

	desc = dl->pctx.protocol[expr->payload.base].desc;
	if (desc == NULL)
		return;

	/* not icmp? ignore. */
	if (desc != &proto_icmp && desc != &proto_icmp6)
		return;

	assert(desc->base == expr->payload.base);

	icmp_type = mpz_get_uint8(value->value);

	for (i = 1; i < array_size(desc->templates); i++) {
		tmpl = &desc->templates[i];

		if (tmpl->len == 0)
			return;

		if (tmpl->offset != expr->payload.offset ||
		    tmpl->len != expr->len)
			continue;

		/* Matches but doesn't load a protocol key -> ignore. */
		if (desc->protocol_key != i)
			return;

		expr->payload.desc = desc;
		expr->payload.tmpl = tmpl;
		dl->pctx.th_dep.icmp.type = icmp_type;
		return;
	}
}

static void payload_match_postprocess(struct rule_pp_ctx *ctx,
				      struct expr *expr,
				      struct expr *payload)
{
	struct dl_proto_ctx *dl = dl_proto_ctx(ctx);

	switch (expr->op) {
	case OP_EQ:
	case OP_NEQ:
		if (expr->right->etype == EXPR_VALUE) {
			payload_match_expand(ctx, expr, payload);
			break;
		} else if (expr->right->etype == EXPR_SET_REF) {
			struct set *set = expr->right->set;

			if (set_is_anonymous(set->flags) &&
			    set->init &&
			    !list_empty(&expr_set(set->init)->expressions)) {
				struct expr *elem;

				elem = list_first_entry(&expr_set(set->init)->expressions, struct expr, list);
				assert(elem->etype == EXPR_SET_ELEM);

				if (elem->key->etype == EXPR_VALUE)
					payload_icmp_check(ctx, payload, elem->key);
			}
		}
		/* Fall through */
	default:
		payload_expr_complete(payload, &dl->pctx);
		expr_set_type(expr->right, payload->dtype,
			      payload->byteorder);
		payload_dependency_kill(&dl->pdctx, payload, dl->pctx.family);
		break;
	}
}

static uint8_t ether_type_to_nfproto(uint16_t l3proto)
{
	switch(l3proto) {
	case ETH_P_IP:
		return NFPROTO_IPV4;
	case ETH_P_IPV6:
		return NFPROTO_IPV6;
	default:
		break;
	}

	return NFPROTO_UNSPEC;
}

static bool __meta_dependency_may_kill(const struct expr *dep, uint8_t *nfproto)
{
	uint16_t l3proto;

	switch (dep->left->etype) {
	case EXPR_META:
		switch (dep->left->meta.key) {
		case NFT_META_NFPROTO:
			*nfproto = mpz_get_uint8(dep->right->value);
			break;
		case NFT_META_PROTOCOL:
			l3proto = mpz_get_uint16(dep->right->value);
			*nfproto = ether_type_to_nfproto(l3proto);
			break;
		default:
			return true;
		}
		break;
	case EXPR_PAYLOAD:
		if (dep->left->payload.base != PROTO_BASE_LL_HDR)
			return true;

		if (dep->left->dtype != &ethertype_type)
			return true;

		l3proto = mpz_get_uint16(dep->right->value);
		*nfproto = ether_type_to_nfproto(l3proto);
		break;
	default:
		return true;
	}

	return false;
}

static bool ct_may_dependency_kill(unsigned int meta_nfproto,
				   const struct expr *ct)
{
	assert(ct->etype == EXPR_CT);

	switch (ct->ct.key) {
	case NFT_CT_DST:
	case NFT_CT_SRC:
		switch (ct->len) {
		case 32:
			return meta_nfproto == NFPROTO_IPV4;
		case 128:
			return meta_nfproto == NFPROTO_IPV6;
		default:
			break;
		}
		return false;
	case NFT_CT_DST_IP:
	case NFT_CT_SRC_IP:
		return meta_nfproto == NFPROTO_IPV4;
	case NFT_CT_DST_IP6:
	case NFT_CT_SRC_IP6:
		return meta_nfproto == NFPROTO_IPV6;
	default:
		break;
	}

	return false;
}

static bool meta_may_dependency_kill(uint8_t nfproto, const struct expr *meta, const struct expr *v)
{
	uint8_t l4proto;

	if (meta->meta.key != NFT_META_L4PROTO)
		return true;

	if (v->etype != EXPR_VALUE || v->len != 8)
		return false;

	l4proto = mpz_get_uint8(v->value);

	switch (l4proto) {
	case IPPROTO_ICMP:
		return nfproto == NFPROTO_IPV4;
	case IPPROTO_ICMPV6:
		return nfproto == NFPROTO_IPV6;
	default:
		break;
	}

	return false;
}

/* We have seen a protocol key expression that restricts matching at the network
 * base, leave it in place since this is meaningful in bridge, inet and netdev
 * families. Exceptions are ICMP and ICMPv6 where this code assumes that can
 * only happen with IPv4 and IPv6.
 */
static bool ct_meta_may_dependency_kill(struct payload_dep_ctx *ctx,
				     unsigned int family,
				     const struct expr *expr)
{
	struct expr *dep = payload_dependency_get(ctx, PROTO_BASE_NETWORK_HDR);
	uint8_t nfproto = NFPROTO_UNSPEC;

	if (!dep)
		return true;

	if (__meta_dependency_may_kill(dep, &nfproto))
		return true;

	switch (family) {
	case NFPROTO_INET:
	case NFPROTO_NETDEV:
	case NFPROTO_BRIDGE:
		break;
	case NFPROTO_IPV4:
	case NFPROTO_IPV6:
		return family == nfproto;
	default:
		return true;
	}

	switch (expr->left->etype) {
	case EXPR_META:
		return meta_may_dependency_kill(nfproto, expr->left, expr->right);
	case EXPR_CT:
		return ct_may_dependency_kill(nfproto, expr->left);
	default:
		break;
	}

	return true;
}

static void ct_meta_common_postprocess(struct rule_pp_ctx *ctx,
				       const struct expr *expr,
				       enum proto_bases base)
{
	struct dl_proto_ctx *dl = dl_proto_ctx(ctx);
	const struct expr *left = expr->left;
	struct expr *right = expr->right;

	if (right->etype == EXPR_SET || right->etype == EXPR_SET_REF)
		expr_set_type(right, left->dtype, left->byteorder);

	switch (expr->op) {
	case OP_EQ:
		if (expr->right->etype == EXPR_RANGE ||
		    expr->right->etype == EXPR_SET ||
		    expr->right->etype == EXPR_SET_REF)
			break;

		relational_expr_pctx_update(&dl->pctx, expr);

		if (base < PROTO_BASE_TRANSPORT_HDR) {
			if (payload_dependency_exists(&dl->pdctx, base) &&
			    ct_meta_may_dependency_kill(&dl->pdctx,
							dl->pctx.family, expr))
				payload_dependency_release(&dl->pdctx, base);

			if (left->flags & EXPR_F_PROTOCOL)
				payload_dependency_store(&dl->pdctx, ctx->stmt, base);
		}
		break;
	default:
		break;
	}
}

static void meta_match_postprocess(struct rule_pp_ctx *ctx,
				   const struct expr *expr)
{
	const struct expr *left = expr->left;

	ct_meta_common_postprocess(ctx, expr, left->meta.base);
}

static void ct_match_postprocess(struct rule_pp_ctx *ctx,
				 const struct expr *expr)
{
	const struct expr *left = expr->left;

	ct_meta_common_postprocess(ctx, expr, left->ct.base);
}

/* Convert a bitmask to a prefix length */
static unsigned int expr_mask_to_prefix(const struct expr *expr)
{
	unsigned long n;

	n = mpz_scan1(expr->value, 0);
	if (n == ULONG_MAX)
		return 0;
	return mpz_scan0(expr->value, n + 1) - n;
}

/* Return true if a bitmask can be expressed as a prefix length */
static bool expr_mask_is_prefix(const struct expr *expr)
{
	unsigned long n1, n2;

	n1 = mpz_scan1(expr->value, 0);
	if (n1 == ULONG_MAX)
		return true;
	n2 = mpz_scan0(expr->value, n1 + 1);
	if (n2 < expr->len || n2 == ULONG_MAX)
		return false;
	return true;
}

/* Convert a series of inclusive OR expressions into a list */
static struct expr *binop_tree_to_list(struct expr *list, struct expr *expr)
{
	if (expr->etype == EXPR_BINOP && expr->op == OP_OR) {
		if (list == NULL)
			list = list_expr_alloc(&expr->location);
		list = binop_tree_to_list(list, expr->left);
		list = binop_tree_to_list(list, expr->right);
	} else {
		if (list == NULL)
			return expr_get(expr);
		list_expr_add(list, expr_get(expr));
	}

	return list;
}

static void binop_adjust_one(const struct expr *binop, struct expr *value,
			     unsigned int shift)
{
	struct expr *left = binop->left;

	assert(value->len >= binop->right->len);

	mpz_rshift_ui(value->value, shift);
	switch (left->etype) {
	case EXPR_PAYLOAD:
	case EXPR_EXTHDR:
		value->len = left->len;
		break;
	default:
		BUG("unknown expression type %s", expr_name(left));
		break;
	}
}

static void binop_adjust(const struct expr *binop, struct expr *right,
			 unsigned int shift)
{
	struct expr *i;

	switch (right->etype) {
	case EXPR_VALUE:
		binop_adjust_one(binop, right, shift);
		break;
	case EXPR_SET_REF:
		if (!set_is_anonymous(right->set->flags))
			break;

		list_for_each_entry(i, &expr_set(right->set->init)->expressions, list) {
			assert(i->etype == EXPR_SET_ELEM);

			switch (i->key->etype) {
			case EXPR_VALUE:
				binop_adjust_one(binop, i->key, shift);
				break;
			case EXPR_RANGE:
				binop_adjust_one(binop, i->key->left, shift);
				binop_adjust_one(binop, i->key->right, shift);
				break;
			case EXPR_MAPPING:
				if (i->key->left->etype == EXPR_RANGE)
					binop_adjust(binop, i->key->left, shift);
				else
					binop_adjust_one(binop, i->key->left, shift);
				break;
			default:
				BUG("unknown expression type %s",
				    expr_name(i->key));
			}
		}
		break;
	case EXPR_RANGE:
		binop_adjust_one(binop, right->left, shift);
		binop_adjust_one(binop, right->right, shift);
		break;
	default:
		BUG("unknown expression type %s", expr_name(right));
		break;
	}
}

static bool __binop_postprocess(struct rule_pp_ctx *ctx,
				struct expr *expr,
				struct expr *left,
				struct expr *mask,
				struct expr **expr_binop)
{
	struct dl_proto_ctx *dl = dl_proto_ctx(ctx);
	struct expr *binop = *expr_binop;
	unsigned int shift;

	assert(binop->etype == EXPR_BINOP);

	if ((left->etype == EXPR_PAYLOAD &&
	    payload_expr_trim(left, mask, &dl->pctx, &shift)) ||
	    (left->etype == EXPR_EXTHDR &&
	     exthdr_find_template(left, mask, &shift))) {
		struct expr *right = NULL;

		/* mask is implicit, binop needs to be removed.
		 *
		 * Fix all values of the expression according to the mask
		 * and then process the payload instruction using the real
		 * sizes and offsets we're interested in.
		 *
		 * Finally, convert the expression to 1) by replacing
		 * the binop with the binop payload/exthdr expression.
		 */
		switch (expr->etype) {
		case EXPR_BINOP:
		case EXPR_RELATIONAL:
			right = expr->right;
			binop_adjust(binop, right, shift);
			break;
		case EXPR_MAP:
			right = expr->mappings;
			binop_adjust(binop, right, shift);
			break;
		default:
			break;
		}

		assert(binop->left == left);
		*expr_binop = expr_get(left);

		if (left->etype == EXPR_PAYLOAD)
			payload_match_postprocess(ctx, expr, left);
		else if (left->etype == EXPR_EXTHDR && right)
			expr_set_type(right, left->dtype, left->byteorder);

		expr_free(binop);
		return true;
	} else if (left->etype == EXPR_PAYLOAD &&
		   expr->right->etype == EXPR_VALUE &&
		   payload_expr_trim_force(left, mask, &shift)) {
			mpz_rshift_ui(expr->right->value, shift);
			*expr_binop = expr_get(left);
			expr_free(binop);
			return true;
	}

	return false;
}

static bool binop_postprocess(struct rule_pp_ctx *ctx, struct expr *expr,
			      struct expr **expr_binop)
{
	struct expr *binop = *expr_binop;
	struct expr *left = binop->left;
	struct expr *mask = binop->right;

	return __binop_postprocess(ctx, expr, left, mask, expr_binop);
}

static void map_binop_postprocess(struct rule_pp_ctx *ctx, struct expr *expr)
{
	struct expr *binop = expr->map;

	if (binop->op != OP_AND)
		return;

	if (binop->left->etype == EXPR_PAYLOAD &&
	    binop->right->etype == EXPR_VALUE)
		binop_postprocess(ctx, expr, &expr->map);
}

static bool is_shift_by_zero(const struct expr *binop)
{
	struct expr *rhs;

	if (binop->op != OP_RSHIFT && binop->op != OP_LSHIFT)
		return false;

	rhs = binop->right;
	if (rhs->etype != EXPR_VALUE || rhs->len > 64)
		return false;

	return mpz_get_uint64(rhs->value) == 0;
}

static void relational_binop_postprocess(struct rule_pp_ctx *ctx,
					 struct expr **exprp)
{
	struct expr *expr = *exprp, *binop = expr->left, *right = expr->right;

	if (binop->op == OP_AND && (expr->op == OP_NEQ || expr->op == OP_EQ) &&
	    right->dtype->basetype &&
	    right->dtype->basetype->type == TYPE_BITMASK &&
	    right->etype == EXPR_VALUE &&
	    !mpz_cmp_ui(right->value, 0)) {
		/* Flag comparison: data & flags != 0
		 *
		 * Split the flags into a list of flag values and convert the
		 * op to OP_EQ.
		 */
		expr_free(right);

		expr->left  = expr_get(binop->left);
		expr->right = binop_tree_to_list(NULL, binop->right);
		switch (expr->op) {
		case OP_NEQ:
			expr->op = OP_IMPLICIT;
			break;
		case OP_EQ:
			expr->op = OP_NEG;
			break;
		default:
			BUG("unknown operation type %d", expr->op);
		}
		expr_free(binop);
	} else if (datatype_prefix_notation(binop->left->dtype) &&
		   binop->op == OP_AND && expr->right->etype == EXPR_VALUE &&
		   expr_mask_is_prefix(binop->right)) {
		expr->left = expr_get(binop->left);
		expr->right = prefix_expr_alloc(&expr->location,
						expr_get(right),
						expr_mask_to_prefix(binop->right));
		expr_free(right);
		expr_free(binop);
	} else if (binop->op == OP_AND &&
		   binop->right->etype == EXPR_VALUE) {
		/*
		 * This *might* be a payload match testing header fields that
		 * have non byte divisible offsets and/or bit lengths.
		 *
		 * Thus we need to deal with two different cases.
		 *
		 * 1 the simple version:
		 *        relation
		 * payload        value|setlookup
		 *
		 * expr: relation, left: payload, right: value, e.g.  tcp dport == 22.
		 *
		 * 2. The '&' version (this is what we're looking at now).
		 *            relation
		 *     binop          value1|setlookup
		 * payload  value2
		 *
		 * expr: relation, left: binop, right: value, e.g.
		 * ip saddr 10.0.0.0/8
		 *
		 * payload_expr_trim will figure out if the mask is needed to match
		 * templates.
		 */
		binop_postprocess(ctx, expr, &expr->left);
	} else if (binop->op == OP_RSHIFT && binop->left->op == OP_AND &&
		   binop->right->etype == EXPR_VALUE && binop->left->right->etype == EXPR_VALUE) {
		/* Handle 'ip version @s4' and similar, i.e. set lookups where the lhs needs
		 * fixups to mask out unwanted bits AND a shift.
		 */

		binop_postprocess(ctx, binop, &binop->left);
		if (is_shift_by_zero(binop)) {
			struct expr *lhs = binop->left;

			expr_get(lhs);
			expr_free(binop);
			expr->left = lhs;
		}
	}
}

static bool payload_binop_postprocess(struct rule_pp_ctx *ctx,
				      struct expr **exprp)
{
	struct expr *expr = *exprp;

	if (expr->op != OP_RSHIFT)
		return false;

	if (expr->left->etype == EXPR_UNARY) {
		/*
		 * If the payload value was originally in a different byte-order
		 * from the payload expression, there will be a byte-order
		 * conversion to remove.
		 */
		struct expr *left = expr_get(expr->left->arg);
		expr_free(expr->left);
		expr->left = left;
	}

	if (expr->left->etype != EXPR_BINOP || expr->left->op != OP_AND)
		return false;

	switch (expr->left->left->etype) {
	case EXPR_EXTHDR:
		break;
	case EXPR_PAYLOAD:
		break;
	default:
		return false;
	}

	expr_postprocess(ctx, &expr->left->left);

	expr_set_type(expr->right, &integer_type,
		      BYTEORDER_HOST_ENDIAN);
	expr_postprocess(ctx, &expr->right);

	binop_postprocess(ctx, expr, &expr->left);
	*exprp = expr_get(expr->left);
	expr_free(expr);

	return true;
}

static struct expr *string_wildcard_expr_alloc(struct location *loc,
					       const struct expr *mask,
					       const struct expr *expr)
{
	unsigned int len = div_round_up(expr->len, BITS_PER_BYTE);
	char data[len + 2];
	int pos;

	mpz_export_data(data, expr->value, BYTEORDER_HOST_ENDIAN, len);
	pos = div_round_up(expr_mask_to_prefix(mask), BITS_PER_BYTE);
	data[pos] = '*';
	data[pos + 1] = '\0';

	return constant_expr_alloc(loc, expr->dtype, BYTEORDER_HOST_ENDIAN,
				   expr->len + BITS_PER_BYTE, data);
}

/* This calculates the string length and checks if it is nul-terminated, this
 * function is quite a hack :)
 */
static bool __expr_postprocess_string(struct expr **exprp)
{
	struct expr *expr = *exprp;
	unsigned int len = div_round_up(expr->len, BITS_PER_BYTE);
	char data[len + 1];

	mpz_export_data(data, expr->value, BYTEORDER_HOST_ENDIAN, len);

	if (data[len - 1] != '\0')
		return false;

	len = strlen(data);
	if (len && data[len - 1] == '*') {
		data[len - 1]	= '\\';
		data[len]	= '*';
		data[len + 1]	= '\0';
		expr = constant_expr_alloc(&expr->location, expr->dtype,
					   BYTEORDER_HOST_ENDIAN,
					   (len + 2) * BITS_PER_BYTE, data);
		expr_free(*exprp);
		*exprp = expr;
	}

	return true;
}

static struct expr *expr_postprocess_string(struct expr *expr)
{
	struct expr *mask, *out;

	assert(expr_basetype(expr)->type == TYPE_STRING);
	if (__expr_postprocess_string(&expr))
		return expr;

	mask = constant_expr_alloc(&expr->location, &integer_type,
				   BYTEORDER_HOST_ENDIAN,
				   expr->len + BITS_PER_BYTE, NULL);
	mpz_clear(mask->value);
	mpz_init_bitmask(mask->value, expr->len);
	out = string_wildcard_expr_alloc(&expr->location, mask, expr);
	expr_free(expr);
	expr_free(mask);
	return out;
}

static void expr_postprocess_value(struct rule_pp_ctx *ctx, struct expr **exprp)
{
	bool interval = (ctx->set && ctx->set->flags & NFT_SET_INTERVAL);
	struct expr *expr = *exprp;

	// FIXME
	if (expr->byteorder == BYTEORDER_HOST_ENDIAN && !interval)
		mpz_switch_byteorder(expr->value, expr->len / BITS_PER_BYTE);

	if (expr_basetype(expr)->type == TYPE_STRING)
		*exprp = expr_postprocess_string(expr);

	expr = *exprp;
	if (expr->dtype->basetype != NULL &&
	    expr->dtype->basetype->type == TYPE_BITMASK)
		*exprp = bitmask_expr_to_binops(expr);
}

static void expr_postprocess_concat(struct rule_pp_ctx *ctx, struct expr **exprp)
{
	struct expr *i, *n, *expr = *exprp;
	unsigned int type = expr->dtype->type, ntype = 0;
	int off = expr->dtype->subtypes;
	const struct datatype *dtype;
	LIST_HEAD(tmp);

	assert(expr->etype == EXPR_CONCAT);

	ctx->flags |= RULE_PP_IN_CONCATENATION;
	list_for_each_entry_safe(i, n, &expr_concat(expr)->expressions, list) {
		if (type) {
			dtype = concat_subtype_lookup(type, --off);
			expr_set_type(i, dtype, dtype->byteorder);
		}
		list_del(&i->list);
		expr_postprocess(ctx, &i);
		list_add_tail(&i->list, &tmp);

		ntype = concat_subtype_add(ntype, i->dtype->type);
	}
	ctx->flags &= ~RULE_PP_IN_CONCATENATION;
	list_splice(&tmp, &expr_concat(expr)->expressions);
	__datatype_set(expr, concat_type_alloc(ntype));
}

static void expr_postprocess(struct rule_pp_ctx *ctx, struct expr **exprp)
{
	struct dl_proto_ctx *dl = dl_proto_ctx(ctx);
	struct expr *expr = *exprp, *i;

	switch (expr->etype) {
	case EXPR_MAP:
		switch (expr->map->etype) {
		case EXPR_BINOP:
			map_binop_postprocess(ctx, expr);
			break;
		default:
			break;
		}

		expr_postprocess(ctx, &expr->map);
		expr_postprocess(ctx, &expr->mappings);
		break;
	case EXPR_MAPPING:
		expr_postprocess(ctx, &expr->left);
		expr_postprocess(ctx, &expr->right);
		break;
	case EXPR_SET:
		list_for_each_entry(i, &expr_set(expr)->expressions, list) {
			assert(i->etype == EXPR_SET_ELEM);

			expr_postprocess(ctx, &i);
		}
		break;
	case EXPR_CONCAT:
		expr_postprocess_concat(ctx, exprp);
		break;
	case EXPR_UNARY:
		expr_postprocess(ctx, &expr->arg);
		expr_set_type(expr, expr->arg->dtype, !expr->arg->byteorder);
		break;
	case EXPR_BINOP:
		if (payload_binop_postprocess(ctx, exprp))
			break;

		expr_postprocess(ctx, &expr->left);
		switch (expr->op) {
		case OP_LSHIFT:
		case OP_RSHIFT:
			expr_set_type(expr->right, &integer_type,
				      BYTEORDER_HOST_ENDIAN);
			break;
		case OP_AND:
			if (expr->right->len > expr->left->len) {
				expr_set_type(expr->right, expr->left->dtype,
					      BYTEORDER_HOST_ENDIAN);
			} else {
				expr_set_type(expr->right, expr->left->dtype,
					      expr->left->byteorder);
			}

			/* Do not process OP_AND in ordinary rule context.
			 *
			 * Removal needs to be performed as part of the relational
			 * operation because the RHS constant might need to be adjusted
			 * (shifted).
			 *
			 * This is different in set element context or concatenations:
			 * There is no relational operation (eq, neq and so on), thus
			 * it needs to be processed right away.
			 */
			if ((ctx->flags & RULE_PP_REMOVE_OP_AND) &&
			    expr->left->etype == EXPR_PAYLOAD &&
			    expr->right->etype == EXPR_VALUE) {
				__binop_postprocess(ctx, expr, expr->left, expr->right, exprp);
				return;
			}
			break;
		default:
			if (expr->right->len > expr->left->len) {
				expr_set_type(expr->right, expr->left->dtype,
					      BYTEORDER_HOST_ENDIAN);
			} else {
				expr_set_type(expr->right, expr->left->dtype,
					      expr->left->byteorder);
			}
		}
		expr_postprocess(ctx, &expr->right);

		switch (expr->op) {
		case OP_LSHIFT:
		case OP_RSHIFT:
			expr_set_type(expr, &xinteger_type,
				      BYTEORDER_HOST_ENDIAN);
			break;
		default:
			expr_set_type(expr, expr->left->dtype,
				      expr->left->byteorder);
		}

		break;
	case EXPR_RELATIONAL:
		switch (expr->left->etype) {
		case EXPR_PAYLOAD:
			payload_match_postprocess(ctx, expr, expr->left);
			return;
		case EXPR_CONCAT:
			if (expr->right->etype == EXPR_SET_REF) {
				assert(expr->left->dtype == &invalid_type);
				assert(expr->right->dtype != &invalid_type);

				datatype_set(expr->left, expr->right->dtype);
			}
			ctx->set = expr->right->set;
			expr_postprocess(ctx, &expr->left);
			ctx->set = NULL;
			break;
		case EXPR_UNARY:
			if (lhs_is_meta_hour(expr->left->arg) &&
			    expr->right->etype == EXPR_RANGE) {
				struct expr *range = expr->right;

				/* Cross-day range needs to be reversed.
				 * Kernel handles time in UTC. Therefore,
				 * 03:00-14:00 AEDT (Sidney, Australia) time
				 * is a cross-day range.
				 */
				if (mpz_cmp(range->left->value,
					    range->right->value) <= 0 &&
				    expr->op == OP_NEQ) {
					range_expr_swap_values(range);
					expr->op = OP_IMPLICIT;
				}
			}
			/* fallthrough */
		default:
			expr_postprocess(ctx, &expr->left);
			break;
		}

		expr_set_type(expr->right, expr->left->dtype, expr->left->byteorder);
		expr_postprocess(ctx, &expr->right);

		switch (expr->left->etype) {
		case EXPR_CT:
			ct_match_postprocess(ctx, expr);
			break;
		case EXPR_META:
			meta_match_postprocess(ctx, expr);
			break;
		case EXPR_BINOP:
			relational_binop_postprocess(ctx, exprp);
			break;
		default:
			break;
		}
		break;
	case EXPR_PAYLOAD:
		payload_expr_complete(expr, &dl->pctx);
		if (expr->payload.inner_desc) {
			if (meta_outer_may_dependency_kill(ctx, expr)) {
				struct dl_proto_ctx *dl_outer = dl_proto_ctx_outer(ctx);

				payload_dependency_release(&dl_outer->pdctx, expr->payload.inner_desc->base);
			}
		}
		payload_dependency_kill(&dl->pdctx, expr, dl->pctx.family);
		break;
	case EXPR_VALUE:
		expr_postprocess_value(ctx, exprp);
		break;
	case EXPR_RANGE:
		expr_postprocess(ctx, &expr->left);
		expr_postprocess(ctx, &expr->right);
		break;
	case EXPR_PREFIX:
		expr_postprocess(ctx, &expr->prefix);
		break;
	case EXPR_SET_ELEM:
		ctx->flags |= RULE_PP_IN_SET_ELEM;
		expr_postprocess(ctx, &expr->key);
		ctx->flags &= ~RULE_PP_IN_SET_ELEM;
		break;
	case EXPR_EXTHDR:
		exthdr_dependency_kill(&dl->pdctx, expr, dl->pctx.family);
		break;
	case EXPR_SET_REF:
	case EXPR_META:
	case EXPR_RT:
	case EXPR_VERDICT:
	case EXPR_NUMGEN:
	case EXPR_FIB:
	case EXPR_SOCKET:
	case EXPR_TUNNEL:
	case EXPR_OSF:
	case EXPR_XFRM:
		break;
	case EXPR_HASH:
		if (expr->hash.expr)
			expr_postprocess(ctx, &expr->hash.expr);
		break;
	case EXPR_CT:
		ct_expr_update_type(&dl->pctx, expr);
		break;
	default:
		BUG("unknown expression type %s", expr_name(expr));
	}
}

static void stmt_reject_postprocess(struct rule_pp_ctx *rctx)
{
	struct dl_proto_ctx *dl = dl_proto_ctx(rctx);
	const struct proto_desc *desc, *base;
	struct stmt *stmt = rctx->stmt;
	int protocol;

	switch (dl->pctx.family) {
	case NFPROTO_IPV4:
		stmt->reject.family = dl->pctx.family;
		datatype_set(stmt->reject.expr, &reject_icmp_code_type);
		if (stmt->reject.type == NFT_REJECT_TCP_RST &&
		    payload_dependency_exists(&dl->pdctx,
					      PROTO_BASE_TRANSPORT_HDR))
			payload_dependency_release(&dl->pdctx,
						   PROTO_BASE_TRANSPORT_HDR);
		break;
	case NFPROTO_IPV6:
		stmt->reject.family = dl->pctx.family;
		datatype_set(stmt->reject.expr, &reject_icmpv6_code_type);
		if (stmt->reject.type == NFT_REJECT_TCP_RST &&
		    payload_dependency_exists(&dl->pdctx,
					      PROTO_BASE_TRANSPORT_HDR))
			payload_dependency_release(&dl->pdctx,
						   PROTO_BASE_TRANSPORT_HDR);
		break;
	case NFPROTO_INET:
	case NFPROTO_BRIDGE:
	case NFPROTO_NETDEV:
		if (stmt->reject.type == NFT_REJECT_ICMPX_UNREACH) {
			datatype_set(stmt->reject.expr, &reject_icmpx_code_type);
			break;
		}

		/* always print full icmp(6) name, simple 'reject' might be ambiguious
		 * because ipv4 vs. ipv6 info might be lost
		 */
		stmt->reject.verbose_print = 1;

		base = dl->pctx.protocol[PROTO_BASE_LL_HDR].desc;
		desc = dl->pctx.protocol[PROTO_BASE_NETWORK_HDR].desc;
		protocol = proto_find_num(base, desc);
		switch (protocol) {
		case NFPROTO_IPV4:			/* INET */
		case __constant_htons(ETH_P_IP):	/* BRIDGE, NETDEV */
			stmt->reject.family = NFPROTO_IPV4;
			datatype_set(stmt->reject.expr, &reject_icmp_code_type);
			break;
		case NFPROTO_IPV6:			/* INET */
		case __constant_htons(ETH_P_IPV6):	/* BRIDGE, NETDEV */
			stmt->reject.family = NFPROTO_IPV6;
			datatype_set(stmt->reject.expr, &reject_icmpv6_code_type);
			break;
		default:
			break;
		}

		if (payload_dependency_exists(&dl->pdctx, PROTO_BASE_NETWORK_HDR))
			payload_dependency_release(&dl->pdctx,
						   PROTO_BASE_NETWORK_HDR);
		break;
	default:
		break;
	}
}

static bool expr_may_merge_range(struct expr *expr, struct expr *prev,
				 enum ops *op)
{
	struct expr *left, *prev_left;

	if (prev->etype == EXPR_RELATIONAL &&
	    expr->etype == EXPR_RELATIONAL) {
		/* ct and meta needs an unary to swap byteorder, in this case
		 * we have to explore the inner branch in this tree.
		 */
		if (expr->left->etype == EXPR_UNARY)
			left = expr->left->arg;
		else
			left = expr->left;

		if (prev->left->etype == EXPR_UNARY)
			prev_left = prev->left->arg;
		else
			prev_left = prev->left;

		if (left->etype == prev_left->etype) {
			if (expr->op == OP_LTE && prev->op == OP_GTE) {
				*op = OP_EQ;
				return true;
			} else if (expr->op == OP_GT && prev->op == OP_LT) {
				*op = OP_NEQ;
				return true;
			}
		}
	}

	return false;
}

static void expr_postprocess_range(struct rule_pp_ctx *ctx, enum ops op)
{
	struct dl_proto_ctx *dl = dl_proto_ctx(ctx);
	struct stmt *nstmt, *stmt = ctx->stmt;
	struct expr *nexpr, *rel;

	nexpr = range_expr_alloc(&dl->pdctx.prev->location,
				 expr_clone(dl->pdctx.prev->expr->right),
				 expr_clone(stmt->expr->right));
	expr_set_type(nexpr, stmt->expr->right->dtype,
		      stmt->expr->right->byteorder);

	rel = relational_expr_alloc(&dl->pdctx.prev->location, op,
				    expr_clone(stmt->expr->left), nexpr);

	nstmt = expr_stmt_alloc(&stmt->location, rel);
	list_add_tail(&nstmt->list, &stmt->list);

	list_del(&dl->pdctx.prev->list);
	stmt_free(dl->pdctx.prev);

	list_del(&stmt->list);
	stmt_free(stmt);
	ctx->stmt = nstmt;
}

static void stmt_expr_postprocess(struct rule_pp_ctx *ctx)
{
	struct dl_proto_ctx *dl = dl_proto_ctx(ctx);
	enum ops op;

	expr_postprocess(ctx, &ctx->stmt->expr);

	if (dl->pdctx.prev && ctx->stmt &&
	    ctx->stmt->type == dl->pdctx.prev->type &&
	    expr_may_merge_range(ctx->stmt->expr, dl->pdctx.prev->expr, &op))
		expr_postprocess_range(ctx, op);
}

static void stmt_payload_binop_pp(struct rule_pp_ctx *ctx, struct expr *binop)
{
	struct dl_proto_ctx *dl = dl_proto_ctx(ctx);
	struct expr *payload = binop->left;
	struct expr *mask = binop->right;
	unsigned int shift;

	assert(payload->etype == EXPR_PAYLOAD);
	if (payload_expr_trim(payload, mask, &dl->pctx, &shift)) {
		binop_adjust(binop, mask, shift);
		payload_expr_complete(payload, &dl->pctx);
		expr_set_type(mask, payload->dtype,
			      payload->byteorder);
	}
}

/**
 * stmt_payload_binop_postprocess - decode payload set binop
 *
 * @ctx:	rule postprocessing context
 *
 * This helper has to be called if expr_postprocess() failed to
 * decode the payload operation.
 *
 * Usually a failure to decode means that userspace had to munge
 * the original payload expression because it has an odd size or
 * a non-byte divisible offset/length.
 *
 * If that was the case, the 'value' expression is not a value but
 * a binop expression with a munged payload expression on the left
 * and a mask to clear the real payload offset/length.
 *
 * So chech if we have one of the following binops:
 * I)
 *           binop (|)
 *       binop(&)   value/set
 * payload   value(mask)
 *
 * This is the normal case, the | RHS is the value the user wants
 * to set, the & RHS is the mask value that discards bits we need
 * to clear but retains everything unrelated to the set operation.
 *
 * IIa)
 *     binop (&)
 * payload   mask
 *
 * User specified a zero set value -- netlink bitwise decoding
 * discarded the redundant "| 0" part.  This is identical to I),
 * we can just set value to 0 after we inferred the real payload size.
 *
 * IIb)
 *     binop (|)
 * payload     value/set
 *
 * This happens when user wants to set all bits, netlink bitwise
 * decoding changed '(payload & mask) ^ bits_to_set' into
 * 'payload | bits_to_set', discarding the redundant "& 0xfff...".
 */
static void stmt_payload_binop_postprocess(struct rule_pp_ctx *ctx,
					   const struct proto_ctx *pctx)
{
	struct expr *expr, *binop, *payload, *value, *mask;
	struct stmt *stmt = ctx->stmt;
	mpz_t bitmask;

	expr = stmt->payload.val;

	if (expr->etype != EXPR_BINOP)
		return;

	switch (expr->left->etype) {
	case EXPR_BINOP: {/* I? */
		unsigned int shift = 0;
		mpz_t tmp;

		if (expr->op != OP_OR)
			return;

		value = expr->right;
		if (value->etype != EXPR_VALUE)
			return;

		binop = expr->left;
		if (binop->op != OP_AND)
			return;

		payload = binop->left;
		if (payload->etype != EXPR_PAYLOAD)
			return;

		if (!payload_expr_cmp(stmt->payload.expr, payload))
			return;

		mask = binop->right;
		if (mask->etype != EXPR_VALUE)
			return;

		mpz_init(tmp);
		mpz_set(tmp, mask->value);

		mpz_init_bitmask(bitmask, payload->len);
		mpz_xor(bitmask, bitmask, mask->value);
		mpz_xor(bitmask, bitmask, value->value);
		mpz_set(mask->value, bitmask);
		mpz_clear(bitmask);

		if (!binop_postprocess(ctx, expr, &expr->left) &&
		    !payload_is_known(payload) &&
		    !payload_expr_trim_force(payload,
					     mask, &shift)) {
			mpz_set(mask->value, tmp);
			mpz_clear(tmp);
			return;
		}

		if (shift)
			mpz_rshift_ui(value->value, shift);

		mpz_clear(tmp);
		expr_free(stmt->payload.expr);
		stmt->payload.expr = expr_get(payload);
		stmt->payload.val = expr_get(expr->right);
		expr_free(expr);
		break;
	}
	case EXPR_PAYLOAD: /* II? */
		payload = expr->left;
		mask = expr->right;

		if (mask->etype != EXPR_VALUE)
			return;

		if (!payload_expr_cmp(stmt->payload.expr, payload))
			return;

		switch (expr->op) {
		case OP_AND: { /* IIa */
			unsigned int shift_unused;
			mpz_t tmp;

			if (stmt_payload_expr_trim(stmt, pctx))
				return;

			mpz_init(tmp);
			mpz_set(tmp, mask->value);

			mpz_init_bitmask(bitmask, payload->len);
			mpz_xor(bitmask, bitmask, mask->value);
			mpz_set(mask->value, bitmask);
			mpz_clear(bitmask);

			stmt_payload_binop_pp(ctx, expr);
			if (!payload_is_known(expr->left) &&
			    !payload_expr_trim_force(expr->left, mask, &shift_unused)) {
				mpz_set(mask->value, tmp);
				mpz_clear(tmp);
				return;
			}

			mpz_clear(tmp);

			/* Mask was used to match payload, i.e. user asked to
			 * clear the payload expression.
			 * The "mask" value becomes new stmt->payload.value
			 * so set this to 0.
			 * Also the reason why &shift_unused is ignored.
			 */
			mpz_set_ui(mask->value, 0);
			break;
		}
		case OP_OR:  /* IIb */
			stmt_payload_binop_pp(ctx, expr);
			if (stmt_payload_expr_trim(stmt, pctx))
				return;
			if (!payload_is_known(expr->left))
				return;
			break;
		case OP_XOR:
			if (stmt_payload_expr_trim(stmt, pctx))
				return;

			return;
		default: /* No idea what to do */
			return;
		}

		expr_free(stmt->payload.expr);
		stmt->payload.expr = expr_get(expr->left);
		stmt->payload.val = expr_get(expr->right);
		expr_free(expr);
		break;
	default: /* No idea */
		break;
	}
}

static void stmt_payload_postprocess(struct rule_pp_ctx *ctx)
{
	struct dl_proto_ctx *dl = dl_proto_ctx(ctx);
	struct stmt *stmt = ctx->stmt;

	payload_expr_complete(stmt->payload.expr, &dl->pctx);
	if (!payload_is_known(stmt->payload.expr))
		stmt_payload_binop_postprocess(ctx, &dl->pctx);

	expr_postprocess(ctx, &stmt->payload.expr);

	expr_set_type(stmt->payload.val,
		      stmt->payload.expr->dtype,
		      stmt->payload.expr->byteorder);

	expr_postprocess(ctx, &stmt->payload.val);
}

static void stmt_queue_postprocess(struct rule_pp_ctx *ctx)
{
	struct stmt *stmt = ctx->stmt;
	struct expr *e = stmt->queue.queue;

	if (e == NULL || e->etype == EXPR_VALUE ||
	    e->etype == EXPR_RANGE)
		return;

	expr_postprocess(ctx, &stmt->queue.queue);
}

/*
 * We can only remove payload dependencies if they occur without
 * a statement with side effects in between.
 *
 * For instance:
 * 'ip protocol tcp tcp dport 22 counter' is same as
 * 'tcp dport 22 counter'.
 *
 * 'ip protocol tcp counter tcp dport 22' cannot be written as
 * 'counter tcp dport 22' (that would be counter ip protocol tcp, but
 * that counts every packet, not just ip/tcp).
 */
static void
rule_maybe_reset_payload_deps(struct payload_dep_ctx *pdctx, enum stmt_types t)
{
	if (t == STMT_EXPRESSION)
		return;

	payload_dependency_reset(pdctx);
}

static bool has_inner_desc(const struct expr *expr)
{
	struct expr *i;

	switch (expr->etype) {
	case EXPR_BINOP:
		return has_inner_desc(expr->left);
	case EXPR_CONCAT:
		list_for_each_entry(i, &expr_concat(expr)->expressions, list) {
			if (has_inner_desc(i))
				return true;
		}
		break;
	case EXPR_META:
		return expr->meta.inner_desc;
	case EXPR_PAYLOAD:
		return expr->payload.inner_desc;
	case EXPR_SET_ELEM:
		return has_inner_desc(expr->key);
	default:
		break;
	}

	return false;
}

static struct dl_proto_ctx *rule_update_dl_proto_ctx(struct rule_pp_ctx *rctx)
{
	const struct stmt *stmt = rctx->stmt;
	bool inner = false;

	switch (stmt->type) {
	case STMT_EXPRESSION:
		if (has_inner_desc(stmt->expr->left))
			inner = true;
		break;
	case STMT_SET:
		if (has_inner_desc(stmt->set.key))
			inner = true;
		break;
	default:
		break;
	}

	if (inner)
		rctx->dl = &rctx->_dl[1];
	else
		rctx->dl = &rctx->_dl[0];

	return rctx->dl;
}

static void rule_parse_postprocess(struct netlink_parse_ctx *ctx, struct rule *rule)
{
	struct stmt *stmt, *next;
	struct dl_proto_ctx *dl;
	struct rule_pp_ctx rctx;
	struct expr *expr;

	memset(&rctx, 0, sizeof(rctx));
	proto_ctx_init(&rctx._dl[0].pctx, rule->handle.family, ctx->debug_mask, false);
	/* use NFPROTO_BRIDGE to set up proto_eth as base protocol. */
	proto_ctx_init(&rctx._dl[1].pctx, NFPROTO_BRIDGE, ctx->debug_mask, true);

	list_for_each_entry_safe(stmt, next, &rule->stmts, list) {
		enum stmt_types type = stmt->type;

		rctx.stmt = stmt;
		dl = rule_update_dl_proto_ctx(&rctx);

		switch (type) {
		case STMT_EXPRESSION:
			stmt_expr_postprocess(&rctx);
			break;
		case STMT_PAYLOAD:
			stmt_payload_postprocess(&rctx);
			break;
		case STMT_METER:
			expr_postprocess(&rctx, &stmt->meter.key);
			break;
		case STMT_META:
			if (stmt->meta.expr != NULL)
				expr_postprocess(&rctx, &stmt->meta.expr);
			break;
		case STMT_CT:
			if (stmt->ct.expr != NULL) {
				expr_postprocess(&rctx, &stmt->ct.expr);

				if (stmt->ct.expr->etype == EXPR_BINOP &&
				    stmt->ct.key == NFT_CT_EVENTMASK) {
					expr = binop_tree_to_list(NULL, stmt->ct.expr);
					expr_free(stmt->ct.expr);
					stmt->ct.expr = expr;
				}
			}
			break;
		case STMT_NAT:
			if (stmt->nat.addr != NULL)
				expr_postprocess(&rctx, &stmt->nat.addr);
			if (stmt->nat.proto != NULL)
				expr_postprocess(&rctx, &stmt->nat.proto);
			break;
		case STMT_TPROXY:
			if (stmt->tproxy.addr)
				expr_postprocess(&rctx, &stmt->tproxy.addr);
			if (stmt->tproxy.port) {
				payload_dependency_reset(&dl->pdctx);
				expr_postprocess(&rctx, &stmt->tproxy.port);
			}
			break;
		case STMT_REJECT:
			stmt_reject_postprocess(&rctx);
			break;
		case STMT_SET:
			expr_postprocess(&rctx, &stmt->set.key);
			break;
		case STMT_MAP:
			expr_postprocess(&rctx, &stmt->map.key);
			expr_postprocess(&rctx, &stmt->map.data);
			break;
		case STMT_DUP:
			if (stmt->dup.to != NULL)
				expr_postprocess(&rctx, &stmt->dup.to);
			if (stmt->dup.dev != NULL)
				expr_postprocess(&rctx, &stmt->dup.dev);
			break;
		case STMT_FWD:
			expr_postprocess(&rctx, &stmt->fwd.dev);
			if (stmt->fwd.addr != NULL)
				expr_postprocess(&rctx, &stmt->fwd.addr);
			break;
		case STMT_XT:
			stmt_xt_postprocess(&rctx, stmt, rule);
			break;
		case STMT_OBJREF:
			expr_postprocess(&rctx, &stmt->objref.expr);
			break;
		case STMT_QUEUE:
			stmt_queue_postprocess(&rctx);
			break;
		default:
			break;
		}

		dl->pdctx.prev = rctx.stmt;

		rule_maybe_reset_payload_deps(&dl->pdctx, type);
	}
}

static int parse_rule_udata_cb(const struct nftnl_udata *attr, void *data)
{
	unsigned char *value = nftnl_udata_get(attr);
	uint8_t type = nftnl_udata_type(attr);
	uint8_t len = nftnl_udata_len(attr);
	const struct nftnl_udata **tb = data;

	switch (type) {
	case NFTNL_UDATA_RULE_COMMENT:
		if (value[len - 1] != '\0')
			return -1;
		break;
	default:
		return 0;
	}
	tb[type] = attr;
	return 0;
}

static char *nftnl_rule_get_comment(const struct nftnl_rule *nlr)
{
	const struct nftnl_udata *tb[NFTNL_UDATA_RULE_MAX + 1] = {};
	const void *data;
	uint32_t len;

	if (!nftnl_rule_is_set(nlr, NFTNL_RULE_USERDATA))
		return NULL;

	data = nftnl_rule_get_data(nlr, NFTNL_RULE_USERDATA, &len);

	if (nftnl_udata_parse(data, len, parse_rule_udata_cb, tb) < 0)
		return NULL;

	if (!tb[NFTNL_UDATA_RULE_COMMENT])
		return NULL;

	return xstrdup(nftnl_udata_get(tb[NFTNL_UDATA_RULE_COMMENT]));
}

struct rule *netlink_delinearize_rule(struct netlink_ctx *ctx,
				      struct nftnl_rule *nlr)
{
	struct netlink_parse_ctx _ctx, *pctx = &_ctx;
	struct handle h;

	memset(&_ctx, 0, sizeof(_ctx));
	_ctx.msgs = ctx->msgs;
	_ctx.debug_mask = ctx->nft->debug_mask;
	_ctx.nlctx = ctx;

	memset(&h, 0, sizeof(h));
	h.family = nftnl_rule_get_u32(nlr, NFTNL_RULE_FAMILY);
	h.table.name = xstrdup(nftnl_rule_get_str(nlr, NFTNL_RULE_TABLE));
	h.chain.name = xstrdup(nftnl_rule_get_str(nlr, NFTNL_RULE_CHAIN));
	h.handle.id = nftnl_rule_get_u64(nlr, NFTNL_RULE_HANDLE);

	if (nftnl_rule_is_set(nlr, NFTNL_RULE_POSITION))
		h.position.id = nftnl_rule_get_u64(nlr, NFTNL_RULE_POSITION);

	pctx->rule = rule_alloc(&netlink_location, &h);
	pctx->table = table_cache_find(&ctx->nft->cache.table_cache,
				       h.table.name, h.family);
	if (!pctx->table) {
		errno = ENOENT;
		return NULL;
	}

	pctx->rule->comment = nftnl_rule_get_comment(nlr);

	nftnl_expr_foreach(nlr, netlink_parse_rule_expr, pctx);

	rule_parse_postprocess(pctx, pctx->rule);
	netlink_release_registers(pctx);
	return pctx->rule;
}
