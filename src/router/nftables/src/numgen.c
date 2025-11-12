/*
 * Number generator expression definitions.
 *
 * Copyright (c) 2016 Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 (or any
 * later) as published by the Free Software Foundation.
 */

#include <nft.h>

#include <nftables.h>
#include <expression.h>
#include <datatype.h>
#include <gmputil.h>
#include <numgen.h>
#include <utils.h>

static const char *numgen_type[NFT_NG_RANDOM + 1] = {
	[NFT_NG_INCREMENTAL]	= "inc",
	[NFT_NG_RANDOM]		= "random",
};

static const char *numgen_type_str(enum nft_ng_types type)
{
	if (type > NFT_NG_RANDOM)
		return "[unknown numgen]";

	return numgen_type[type];
}

static void numgen_expr_print(const struct expr *expr, struct output_ctx *octx)
{
	nft_print(octx, "numgen %s mod %u",
		  numgen_type_str(expr->numgen.type),
		  expr->numgen.mod);
	if (expr->numgen.offset)
		nft_print(octx, " offset %u", expr->numgen.offset);
}

static bool numgen_expr_cmp(const struct expr *e1, const struct expr *e2)
{
	return e1->numgen.type == e2->numgen.type &&
	       e1->numgen.mod == e2->numgen.mod &&
	       e1->numgen.offset == e2->numgen.offset;
}

static void numgen_expr_clone(struct expr *new, const struct expr *expr)
{
	new->numgen.type = expr->numgen.type;
	new->numgen.mod = expr->numgen.mod;
	new->numgen.offset = expr->numgen.offset;
}

#define NFTNL_UDATA_NUMGEN_TYPE 0
#define NFTNL_UDATA_NUMGEN_MOD 1
#define NFTNL_UDATA_NUMGEN_OFFSET 2
#define NFTNL_UDATA_NUMGEN_MAX 3

static int numgen_expr_build_udata(struct nftnl_udata_buf *udbuf,
				   const struct expr *expr)
{
        nftnl_udata_put_u32(udbuf, NFTNL_UDATA_NUMGEN_TYPE, expr->numgen.type);
        nftnl_udata_put_u32(udbuf, NFTNL_UDATA_NUMGEN_MOD, expr->numgen.mod);
        nftnl_udata_put_u32(udbuf, NFTNL_UDATA_NUMGEN_OFFSET, expr->numgen.offset);

        return 0;
}

static int numgen_parse_udata(const struct nftnl_udata *attr, void *data)
{
        const struct nftnl_udata **ud = data;
        uint8_t type = nftnl_udata_type(attr);
        uint8_t len = nftnl_udata_len(attr);

        switch (type) {
        case NFTNL_UDATA_NUMGEN_TYPE:
        case NFTNL_UDATA_NUMGEN_MOD:
        case NFTNL_UDATA_NUMGEN_OFFSET:
                if (len != sizeof(uint32_t))
                        return -1;
                break;
        default:
                return 0;
        }

        ud[type] = attr;
        return 0;
}

static struct expr *numgen_expr_parse_udata(const struct nftnl_udata *attr)
{
        const struct nftnl_udata *ud[NFTNL_UDATA_NUMGEN_MAX + 1] = {};
	enum nft_ng_types type;
	uint32_t mod, offset;
        int err;

        err = nftnl_udata_parse(nftnl_udata_get(attr), nftnl_udata_len(attr),
                                numgen_parse_udata, ud);
        if (err < 0)
                return NULL;

        if (!ud[NFTNL_UDATA_NUMGEN_TYPE] ||
	    !ud[NFTNL_UDATA_NUMGEN_MOD] ||
	    !ud[NFTNL_UDATA_NUMGEN_OFFSET])
                return NULL;

	type = nftnl_udata_get_u32(ud[NFTNL_UDATA_NUMGEN_TYPE]);
	mod = nftnl_udata_get_u32(ud[NFTNL_UDATA_NUMGEN_MOD]);
	offset = nftnl_udata_get_u32(ud[NFTNL_UDATA_NUMGEN_OFFSET]);

	return numgen_expr_alloc(&internal_location, type, mod, offset);
}

const struct expr_ops numgen_expr_ops = {
	.type		= EXPR_NUMGEN,
	.name		= "numgen",
	.print		= numgen_expr_print,
	.json		= numgen_expr_json,
	.cmp		= numgen_expr_cmp,
	.clone		= numgen_expr_clone,
	.parse_udata	= numgen_expr_parse_udata,
	.build_udata	= numgen_expr_build_udata,
};

struct expr *numgen_expr_alloc(const struct location *loc,
			       enum nft_ng_types type, uint32_t mod,
			       uint32_t offset)
{
	struct expr *expr;

	expr = expr_alloc(loc, EXPR_NUMGEN, &integer_type,
			  BYTEORDER_HOST_ENDIAN, 4 * BITS_PER_BYTE);
	expr->numgen.type  = type;
	expr->numgen.mod   = mod;
	expr->numgen.offset = offset;

	return expr;
}
