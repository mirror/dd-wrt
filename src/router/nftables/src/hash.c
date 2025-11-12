/*
 * Hash expression definitions.
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
#include <hash.h>
#include <utils.h>

static void hash_expr_print(const struct expr *expr, struct output_ctx *octx)
{
	switch (expr->hash.type) {
	case NFT_HASH_SYM:
		nft_print(octx, "symhash");
		break;
	case NFT_HASH_JENKINS:
	default:
		nft_print(octx, "jhash ");
		expr_print(expr->hash.expr, octx);
	}

	nft_print(octx, " mod %u", expr->hash.mod);
	if (expr->hash.seed_set)
		nft_print(octx, " seed 0x%x", expr->hash.seed);
	if (expr->hash.offset)
		nft_print(octx, " offset %u", expr->hash.offset);
}

static bool hash_expr_cmp(const struct expr *e1, const struct expr *e2)
{
	return (!e1->hash.expr ||
		expr_cmp(e1->hash.expr, e2->hash.expr)) &&
	       e1->hash.mod == e2->hash.mod &&
	       e1->hash.seed_set == e2->hash.seed_set &&
	       e1->hash.seed == e2->hash.seed &&
	       e1->hash.offset == e2->hash.offset &&
	       e1->hash.type == e2->hash.type;
}

static void hash_expr_clone(struct expr *new, const struct expr *expr)
{
	if (expr->hash.expr)
		new->hash.expr = expr_clone(expr->hash.expr);
	new->hash.mod = expr->hash.mod;
	new->hash.seed_set = expr->hash.seed_set;
	new->hash.seed = expr->hash.seed;
	new->hash.offset = expr->hash.offset;
	new->hash.type = expr->hash.type;
}

static void hash_expr_destroy(struct expr *expr)
{
	expr_free(expr->hash.expr);
}

#define NFTNL_UDATA_HASH_TYPE 0
#define NFTNL_UDATA_HASH_OFFSET 1
#define NFTNL_UDATA_HASH_MOD 2
#define NFTNL_UDATA_HASH_SEED 3
#define NFTNL_UDATA_HASH_SEED_SET 4
#define NFTNL_UDATA_HASH_MAX 5

static int hash_expr_build_udata(struct nftnl_udata_buf *udbuf,
				 const struct expr *expr)
{
	nftnl_udata_put_u32(udbuf, NFTNL_UDATA_HASH_TYPE, expr->hash.type);
	nftnl_udata_put_u32(udbuf, NFTNL_UDATA_HASH_OFFSET, expr->hash.offset);
	nftnl_udata_put_u32(udbuf, NFTNL_UDATA_HASH_MOD, expr->hash.mod);
	nftnl_udata_put_u32(udbuf, NFTNL_UDATA_HASH_SEED, expr->hash.seed);
	nftnl_udata_put_u32(udbuf, NFTNL_UDATA_HASH_SEED_SET, expr->hash.seed_set);

	return 0;
}

static int hash_parse_udata(const struct nftnl_udata *attr, void *data)
{
	const struct nftnl_udata **ud = data;
	uint8_t type = nftnl_udata_type(attr);
	uint8_t len = nftnl_udata_len(attr);

	switch (type) {
	case NFTNL_UDATA_HASH_TYPE:
	case NFTNL_UDATA_HASH_OFFSET:
	case NFTNL_UDATA_HASH_SEED:
	case NFTNL_UDATA_HASH_SEED_SET:
	case NFTNL_UDATA_HASH_MOD:
		if (len != sizeof(uint32_t))
			return -1;
		break;
	default:
		return 0;
	}

	ud[type] = attr;
	return 0;
}

static struct expr *hash_expr_parse_udata(const struct nftnl_udata *attr)
{
	const struct nftnl_udata *ud[NFTNL_UDATA_HASH_MAX + 1] = {};
	uint32_t type, seed, seed_set, mod, offset;
	int err;

	err = nftnl_udata_parse(nftnl_udata_get(attr), nftnl_udata_len(attr),
				hash_parse_udata, ud);
	if (err < 0)
		return NULL;

	if (!ud[NFTNL_UDATA_HASH_TYPE] ||
	    !ud[NFTNL_UDATA_HASH_OFFSET] ||
	    !ud[NFTNL_UDATA_HASH_SEED] ||
	    !ud[NFTNL_UDATA_HASH_MOD] ||
	    !ud[NFTNL_UDATA_HASH_SEED_SET])
		return NULL;

	type = nftnl_udata_get_u32(ud[NFTNL_UDATA_HASH_TYPE]);
	offset = nftnl_udata_get_u32(ud[NFTNL_UDATA_HASH_OFFSET]);
	seed = nftnl_udata_get_u32(ud[NFTNL_UDATA_HASH_SEED]);
	seed_set = nftnl_udata_get_u32(ud[NFTNL_UDATA_HASH_SEED_SET]);
	mod = nftnl_udata_get_u32(ud[NFTNL_UDATA_HASH_MOD]);

	return hash_expr_alloc(&internal_location, mod, seed_set, seed,
			       offset, type);
}

const struct expr_ops hash_expr_ops = {
	.type		= EXPR_HASH,
	.name		= "hash",
	.print		= hash_expr_print,
	.json		= hash_expr_json,
	.cmp		= hash_expr_cmp,
	.clone		= hash_expr_clone,
	.destroy	= hash_expr_destroy,
	.parse_udata	= hash_expr_parse_udata,
	.build_udata	= hash_expr_build_udata,
};

struct expr *hash_expr_alloc(const struct location *loc,
			     uint32_t mod,
			     bool seed_set, uint32_t seed,
			     uint32_t offset,
			     enum nft_hash_types type)
{
	struct expr *expr;

	expr = expr_alloc(loc, EXPR_HASH, &integer_type,
			  BYTEORDER_HOST_ENDIAN, 4 * BITS_PER_BYTE);
	expr->hash.mod  = mod;
	expr->hash.seed_set = seed_set;
	expr->hash.seed = seed;
	expr->hash.offset = offset;
	expr->hash.type = type;

	return expr;
}
