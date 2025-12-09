/*
 * FIB expression.
 *
 * Copyright (c) Red Hat GmbH.	Author: Florian Westphal <fw@strlen.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 (or any
 * later) as published by the Free Software Foundation.
 */

#include <nft.h>

#include <nftables.h>
#include <erec.h>
#include <expression.h>
#include <datatype.h>
#include <gmputil.h>
#include <utils.h>
#include <fib.h>

#include <linux/rtnetlink.h>
#include <net/if.h>

static const char *fib_result[NFT_FIB_RESULT_MAX + 1] = {
	[NFT_FIB_RESULT_OIF] = "oif",
	[NFT_FIB_RESULT_OIFNAME] = "oifname",
	[NFT_FIB_RESULT_ADDRTYPE] = "type",
};

static const struct symbol_table addrtype_tbl = {
	.base		= BASE_DECIMAL,
	.symbols	= {
		SYMBOL("unspec",	RTN_UNSPEC),
		SYMBOL("unicast",	RTN_UNICAST),
		SYMBOL("local",		RTN_LOCAL),
		SYMBOL("broadcast",	RTN_BROADCAST),
		SYMBOL("anycast",	RTN_ANYCAST),
		SYMBOL("multicast",	RTN_MULTICAST),
		SYMBOL("blackhole",	RTN_BLACKHOLE),
		SYMBOL("unreachable",	RTN_UNREACHABLE),
		SYMBOL("prohibit",	RTN_PROHIBIT),
		SYMBOL_LIST_END
	}
};

const struct datatype fib_addr_type = {
	.type		= TYPE_FIB_ADDR,
	.name		= "fib_addrtype",
	.desc		= "fib address type",
	.byteorder	= BYTEORDER_HOST_ENDIAN,
	.size		= 4 * BITS_PER_BYTE,
	.basetype	= &integer_type,
	.sym_tbl	= &addrtype_tbl,
};

const char *fib_result_str(const struct expr *expr, bool check)
{
	enum nft_fib_result result = expr->fib.result;
	uint32_t flags = expr->fib.flags;

	/* Exception: check if route exists. */
	if (check &&
	    result == NFT_FIB_RESULT_OIF &&
	    flags & NFTA_FIB_F_PRESENT)
		return "check";

	if (result <= NFT_FIB_RESULT_MAX)
		return fib_result[result];

	return "unknown";
}

static void __fib_expr_print_f(unsigned int *flags, unsigned int f,
			       const char *s, struct output_ctx *octx)
{
	if ((*flags & f) == 0)
		return;

	nft_print(octx, "%s", s);
	*flags &= ~f;
	if (*flags)
		nft_print(octx, " . ");
}

static void fib_expr_print(const struct expr *expr, struct output_ctx *octx)
{
	unsigned int flags = expr->fib.flags & ~NFTA_FIB_F_PRESENT;

	nft_print(octx, "fib ");
	__fib_expr_print_f(&flags, NFTA_FIB_F_SADDR, "saddr", octx);
	__fib_expr_print_f(&flags, NFTA_FIB_F_DADDR, "daddr", octx);
	__fib_expr_print_f(&flags, NFTA_FIB_F_MARK, "mark", octx);
	__fib_expr_print_f(&flags, NFTA_FIB_F_IIF, "iif", octx);
	__fib_expr_print_f(&flags, NFTA_FIB_F_OIF, "oif", octx);

	if (flags)
		nft_print(octx, "0x%x", flags);

	nft_print(octx, " %s", fib_result_str(expr, true));
}

static bool fib_expr_cmp(const struct expr *e1, const struct expr *e2)
{
	return	e1->fib.result == e2->fib.result &&
		e1->fib.flags == e2->fib.flags;
}

static void fib_expr_clone(struct expr *new, const struct expr *expr)
{
	new->fib.result = expr->fib.result;
	new->fib.flags= expr->fib.flags;
}

#define NFTNL_UDATA_FIB_RESULT 0
#define NFTNL_UDATA_FIB_FLAGS 1
#define NFTNL_UDATA_FIB_MAX 2

static int fib_expr_build_udata(struct nftnl_udata_buf *udbuf,
				 const struct expr *expr)
{
	nftnl_udata_put_u32(udbuf, NFTNL_UDATA_FIB_RESULT, expr->fib.result);
	nftnl_udata_put_u32(udbuf, NFTNL_UDATA_FIB_FLAGS, expr->fib.flags);

	return 0;
}

static int fib_parse_udata(const struct nftnl_udata *attr, void *data)
{
	const struct nftnl_udata **ud = data;
	uint8_t type = nftnl_udata_type(attr);
	uint8_t len = nftnl_udata_len(attr);

	switch (type) {
	case NFTNL_UDATA_FIB_RESULT:
	case NFTNL_UDATA_FIB_FLAGS:
		if (len != sizeof(uint32_t))
			return -1;
		break;
	default:
		return 0;
	}

	ud[type] = attr;
	return 0;
}

static struct expr *fib_expr_parse_udata(const struct nftnl_udata *attr)
{
	const struct nftnl_udata *ud[NFTNL_UDATA_FIB_MAX + 1] = {};
	uint32_t flags, result;
	int err;

	err = nftnl_udata_parse(nftnl_udata_get(attr), nftnl_udata_len(attr),
				fib_parse_udata, ud);
	if (err < 0)
		return NULL;

	if (!ud[NFTNL_UDATA_FIB_RESULT] ||
	    !ud[NFTNL_UDATA_FIB_FLAGS])
		return NULL;

	result = nftnl_udata_get_u32(ud[NFTNL_UDATA_FIB_RESULT]);
	flags = nftnl_udata_get_u32(ud[NFTNL_UDATA_FIB_FLAGS]);

	return fib_expr_alloc(&internal_location, flags, result);
}

const struct expr_ops fib_expr_ops = {
	.type		= EXPR_FIB,
	.name		= "fib",
	.print		= fib_expr_print,
	.json		= fib_expr_json,
	.cmp		= fib_expr_cmp,
	.clone		= fib_expr_clone,
	.parse_udata	= fib_expr_parse_udata,
	.build_udata	= fib_expr_build_udata,
};

struct expr *fib_expr_alloc(const struct location *loc,
			    unsigned int flags, unsigned int result)
{
	const struct datatype *type;
	unsigned int len = 4 * BITS_PER_BYTE;
	struct expr *expr;

	switch (result) {
	case NFT_FIB_RESULT_OIF:
		type = &ifindex_type;
		break;
	case NFT_FIB_RESULT_OIFNAME:
		type = &ifname_type;
		len = IFNAMSIZ * BITS_PER_BYTE;
		break;
	case NFT_FIB_RESULT_ADDRTYPE:
		type = &fib_addr_type;
		break;
	default:
		BUG("Unknown result %d", result);
	}

	if (flags & NFTA_FIB_F_PRESENT) {
		type = &boolean_type;
		len = BITS_PER_BYTE;
	}

	expr = expr_alloc(loc, EXPR_FIB, type,
			  BYTEORDER_HOST_ENDIAN, len);

	expr->fib.result = result;
	expr->fib.flags	= flags;

	return expr;
}
