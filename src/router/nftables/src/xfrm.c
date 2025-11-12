/*
 * XFRM (ipsec) expression
 *
 * Copyright (c) Red Hat GmbH.  Author: Florian Westphal <fw@strlen.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 (or any
 * later) as published by the Free Software Foundation.
 */

#include <nft.h>

#include <nftables.h>
#include <erec.h>
#include <expression.h>
#include <xfrm.h>
#include <datatype.h>
#include <gmputil.h>
#include <utils.h>

#include <netinet/ip.h>
#include <linux/netfilter.h>
#include <linux/xfrm.h>

#define XFRM_TEMPLATE_BE(__token, __dtype, __len) {	\
	.token		= (__token),			\
	.dtype		= (__dtype),			\
	.len		= (__len),			\
	.byteorder	= BYTEORDER_BIG_ENDIAN,		\
}

#define XFRM_TEMPLATE_HE(__token, __dtype, __len) {	\
	.token		= (__token),			\
	.dtype		= (__dtype),			\
	.len		= (__len),			\
	.byteorder	= BYTEORDER_HOST_ENDIAN,	\
}

const struct xfrm_template xfrm_templates[] = {
	[NFT_XFRM_KEY_DADDR_IP4]	= XFRM_TEMPLATE_BE("daddr", &ipaddr_type, 4 * BITS_PER_BYTE),
	[NFT_XFRM_KEY_SADDR_IP4]	= XFRM_TEMPLATE_BE("saddr", &ipaddr_type, 4 * BITS_PER_BYTE),
	[NFT_XFRM_KEY_DADDR_IP6]	= XFRM_TEMPLATE_BE("daddr", &ip6addr_type, 16 * BITS_PER_BYTE),
	[NFT_XFRM_KEY_SADDR_IP6]	= XFRM_TEMPLATE_BE("saddr", &ip6addr_type, 16 * BITS_PER_BYTE),
	[NFT_XFRM_KEY_REQID]		= XFRM_TEMPLATE_HE("reqid", &integer_type, 4 * BITS_PER_BYTE),
	[NFT_XFRM_KEY_SPI]		= XFRM_TEMPLATE_BE("spi", &integer_type, 4 * BITS_PER_BYTE),
};

static void xfrm_expr_print(const struct expr *expr, struct output_ctx *octx)
{
	switch (expr->xfrm.direction) {
	case XFRM_POLICY_IN:
		nft_print(octx, "ipsec in");
		break;
	case XFRM_POLICY_OUT:
		nft_print(octx, "ipsec out");
		break;
	default:
		nft_print(octx, "ipsec (unknown dir %d)", expr->xfrm.direction);
		break;
	}

	if (expr->xfrm.spnum)
		nft_print(octx, " spnum %u", expr->xfrm.spnum);

	switch (expr->xfrm.key) {
	case NFT_XFRM_KEY_DADDR_IP4:
	case NFT_XFRM_KEY_SADDR_IP4:
		 nft_print(octx, " ip");
		 break;
	case NFT_XFRM_KEY_DADDR_IP6:
	case NFT_XFRM_KEY_SADDR_IP6:
		 nft_print(octx, " ip6");
		 break;
	case NFT_XFRM_KEY_REQID:
	case NFT_XFRM_KEY_SPI:
		 break;
	default:
		 nft_print(octx, " (unknown key 0x%x)", expr->xfrm.key);
		 return;
	}

	nft_print(octx, " %s", xfrm_templates[expr->xfrm.key].token);
}

static bool xfrm_expr_cmp(const struct expr *e1, const struct expr *e2)
{
	return e1->xfrm.key == e2->xfrm.key &&
	       e1->xfrm.direction == e2->xfrm.direction &&
	       e1->xfrm.spnum == e2->xfrm.spnum;
}

static void xfrm_expr_clone(struct expr *new, const struct expr *expr)
{
	memcpy(&new->xfrm, &expr->xfrm, sizeof(new->xfrm));
}

#define NFTNL_UDATA_XFRM_KEY 0
#define NFTNL_UDATA_XFRM_SPNUM 1
#define NFTNL_UDATA_XFRM_DIR 2
#define NFTNL_UDATA_XFRM_MAX 3

static int xfrm_expr_build_udata(struct nftnl_udata_buf *udbuf,
				 const struct expr *expr)
{
	nftnl_udata_put_u32(udbuf, NFTNL_UDATA_XFRM_KEY, expr->xfrm.key);
	nftnl_udata_put_u32(udbuf, NFTNL_UDATA_XFRM_SPNUM, expr->xfrm.spnum);
	nftnl_udata_put_u32(udbuf, NFTNL_UDATA_XFRM_DIR, expr->xfrm.direction);

	return 0;
}

static int xfrm_parse_udata(const struct nftnl_udata *attr, void *data)
{
	const struct nftnl_udata **ud = data;
	uint8_t type = nftnl_udata_type(attr);
	uint8_t len = nftnl_udata_len(attr);

	switch (type) {
	case NFTNL_UDATA_XFRM_KEY:
	case NFTNL_UDATA_XFRM_SPNUM:
	case NFTNL_UDATA_XFRM_DIR:
		if (len != sizeof(uint32_t))
			return -1;
		break;
	default:
		return 0;
	}

	ud[type] = attr;
	return 0;
}

static struct expr *xfrm_expr_parse_udata(const struct nftnl_udata *attr)
{
	const struct nftnl_udata *ud[NFTNL_UDATA_XFRM_MAX + 1] = {};
	uint32_t key, dir, spnum;
	int err;

	err = nftnl_udata_parse(nftnl_udata_get(attr), nftnl_udata_len(attr),
				xfrm_parse_udata, ud);
	if (err < 0)
		return NULL;

	if (!ud[NFTNL_UDATA_XFRM_KEY] ||
	    !ud[NFTNL_UDATA_XFRM_DIR] ||
	    !ud[NFTNL_UDATA_XFRM_SPNUM])
		return NULL;

	key = nftnl_udata_get_u32(ud[NFTNL_UDATA_XFRM_KEY]);
	dir = nftnl_udata_get_u32(ud[NFTNL_UDATA_XFRM_DIR]);
	spnum = nftnl_udata_get_u32(ud[NFTNL_UDATA_XFRM_SPNUM]);

	return xfrm_expr_alloc(&internal_location, dir, spnum, key);
}

const struct expr_ops xfrm_expr_ops = {
	.type		= EXPR_XFRM,
	.name		= "xfrm",
	.print		= xfrm_expr_print,
	.json		= xfrm_expr_json,
	.cmp		= xfrm_expr_cmp,
	.clone		= xfrm_expr_clone,
	.parse_udata	= xfrm_expr_parse_udata,
	.build_udata	= xfrm_expr_build_udata,
};

struct expr *xfrm_expr_alloc(const struct location *loc,
			     uint8_t direction,
			     uint8_t spnum,
			     enum nft_xfrm_keys key)
{
	struct expr *expr;

	expr = expr_alloc(loc, EXPR_XFRM,
			  xfrm_templates[key].dtype,
			  xfrm_templates[key].byteorder,
			  xfrm_templates[key].len);

	expr->xfrm.direction = direction;
	expr->xfrm.spnum = spnum;
	expr->xfrm.key = key;

	return expr;
}
