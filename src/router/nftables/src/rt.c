/*
 * Routing expression related definition and types.
 *
 * Copyright (c) 2016 Anders K. Pedersen <akp@cohaesio.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <nft.h>

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <linux/netfilter.h>

#include <nftables.h>
#include <expression.h>
#include <datatype.h>
#include <rt.h>
#include <rule.h>
#include <json.h>

void realm_table_rt_init(struct nft_ctx *ctx)
{
	ctx->output.tbl.realm = rt_symbol_table_init("rt_realms");
}

void realm_table_rt_exit(struct nft_ctx *ctx)
{
	rt_symbol_table_free(ctx->output.tbl.realm);
}

static void realm_type_print(const struct expr *expr, struct output_ctx *octx)
{
	return symbolic_constant_print(octx->tbl.realm, expr, true, octx);
}

static struct error_record *realm_type_parse(struct parse_ctx *ctx,
					     const struct expr *sym,
					     struct expr **res)
{
	return symbolic_constant_parse(ctx, sym, ctx->tbl->realm, res);
}

static void realm_type_describe(struct output_ctx *octx)
{
	rt_symbol_table_describe(octx, "rt_realms",
				 octx->tbl.realm, &realm_type);
}

const struct datatype realm_type = {
	.type		= TYPE_REALM,
	.name		= "realm",
	.desc		= "routing realm",
	.describe	= realm_type_describe,
	.byteorder	= BYTEORDER_HOST_ENDIAN,
	.size		= 4 * BITS_PER_BYTE,
	.basetype	= &integer_type,
	.print		= realm_type_print,
	.parse		= realm_type_parse,
};

const struct rt_template rt_templates[] = {
	[NFT_RT_CLASSID]	= RT_TEMPLATE("classid",
					      &realm_type,
					      4 * BITS_PER_BYTE,
					      BYTEORDER_HOST_ENDIAN,
					      false),
	[NFT_RT_NEXTHOP4]	= RT_TEMPLATE("nexthop",
					      &ipaddr_type,
					      4 * BITS_PER_BYTE,
					      BYTEORDER_BIG_ENDIAN,
					      true),
	[NFT_RT_NEXTHOP6]	= RT_TEMPLATE("nexthop",
					      &ip6addr_type,
					      16 * BITS_PER_BYTE,
					      BYTEORDER_BIG_ENDIAN,
					      true),
	[NFT_RT_TCPMSS]		= RT_TEMPLATE("mtu",
					      &integer_type,
					      2 * BITS_PER_BYTE,
					      BYTEORDER_HOST_ENDIAN,
					      false),
	[NFT_RT_XFRM]		= RT_TEMPLATE("ipsec",
					      &boolean_type,
					      BITS_PER_BYTE,
					      BYTEORDER_HOST_ENDIAN,
					      false),
};

static void rt_expr_print(const struct expr *expr, struct output_ctx *octx)
{
	const char *ip = "";

	switch (expr->rt.key) {
	case NFT_RT_NEXTHOP4:
		ip = "ip ";
		break;
	case NFT_RT_NEXTHOP6:
		ip = "ip6 ";
		break;
	default:
		break;
	}

	nft_print(octx, "rt %s%s", ip, rt_templates[expr->rt.key].token);
}

static bool rt_expr_cmp(const struct expr *e1, const struct expr *e2)
{
	return e1->rt.key == e2->rt.key;
}

static void rt_expr_clone(struct expr *new, const struct expr *expr)
{
	new->rt.key = expr->rt.key;
}

#define NFTNL_UDATA_RT_KEY 0
#define NFTNL_UDATA_RT_MAX 1

static int rt_expr_build_udata(struct nftnl_udata_buf *udbuf,
				 const struct expr *expr)
{
	nftnl_udata_put_u32(udbuf, NFTNL_UDATA_RT_KEY, expr->rt.key);

	return 0;
}

static int rt_parse_udata(const struct nftnl_udata *attr, void *data)
{
	const struct nftnl_udata **ud = data;
	uint8_t type = nftnl_udata_type(attr);
	uint8_t len = nftnl_udata_len(attr);

	switch (type) {
	case NFTNL_UDATA_RT_KEY:
		if (len != sizeof(uint32_t))
			return -1;
		break;
	default:
		return 0;
	}

	ud[type] = attr;
	return 0;
}

static struct expr *rt_expr_parse_udata(const struct nftnl_udata *attr)
{
	const struct nftnl_udata *ud[NFTNL_UDATA_RT_MAX + 1] = {};
	uint32_t key;
	int err;

	err = nftnl_udata_parse(nftnl_udata_get(attr), nftnl_udata_len(attr),
				rt_parse_udata, ud);
	if (err < 0)
		return NULL;

	if (!ud[NFTNL_UDATA_RT_KEY])
		return NULL;

	key = nftnl_udata_get_u32(ud[NFTNL_UDATA_RT_KEY]);

	return rt_expr_alloc(&internal_location, key, false);
}

const struct expr_ops rt_expr_ops = {
	.type		= EXPR_RT,
	.name		= "rt",
	.print		= rt_expr_print,
	.json		= rt_expr_json,
	.cmp		= rt_expr_cmp,
	.clone		= rt_expr_clone,
	.parse_udata	= rt_expr_parse_udata,
	.build_udata	= rt_expr_build_udata,
};

struct expr *rt_expr_alloc(const struct location *loc, enum nft_rt_keys key,
			   bool invalid)
{
	const struct rt_template *tmpl = &rt_templates[key];
	struct expr *expr;

	if (invalid && tmpl->invalid)
		expr = expr_alloc(loc, EXPR_RT, &invalid_type,
				  tmpl->byteorder, 0);
	else
		expr = expr_alloc(loc, EXPR_RT, tmpl->dtype,
				  tmpl->byteorder, tmpl->len);
	expr->rt.key = key;

	return expr;
}

void rt_expr_update_type(struct proto_ctx *ctx, struct expr *expr)
{
	const struct proto_desc *desc;

	switch (expr->rt.key) {
	case NFT_RT_NEXTHOP4:
		desc = ctx->protocol[PROTO_BASE_NETWORK_HDR].desc;
		if (desc == &proto_ip)
			datatype_set(expr, &ipaddr_type);
		else if (desc == &proto_ip6) {
			expr->rt.key++;
			datatype_set(expr, &ip6addr_type);
		}
		expr->len = expr->dtype->size;
		break;
	default:
		break;
	}
}
