/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) 2016 Red Hat GmbH
 * Author: Florian Westphal <fw@strlen.de>
 */

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <linux/netfilter/nf_tables.h>

#include "internal.h"
#include <libmnl/libmnl.h>
#include <libnftnl/expr.h>
#include <libnftnl/rule.h>

struct nftnl_expr_fib {
	uint32_t		flags;
	uint32_t		result;
	enum nft_registers	dreg;
};

static int
nftnl_expr_fib_set(struct nftnl_expr *e, uint16_t result,
		    const void *data, uint32_t data_len)
{
	struct nftnl_expr_fib *fib = nftnl_expr_data(e);

	switch (result) {
	case NFTNL_EXPR_FIB_RESULT:
		memcpy(&fib->result, data, data_len);
		break;
	case NFTNL_EXPR_FIB_DREG:
		memcpy(&fib->dreg, data, data_len);
		break;
	case NFTNL_EXPR_FIB_FLAGS:
		memcpy(&fib->flags, data, data_len);
		break;
	}
	return 0;
}

static const void *
nftnl_expr_fib_get(const struct nftnl_expr *e, uint16_t result,
		    uint32_t *data_len)
{
	struct nftnl_expr_fib *fib = nftnl_expr_data(e);

	switch (result) {
	case NFTNL_EXPR_FIB_RESULT:
		*data_len = sizeof(fib->result);
		return &fib->result;
	case NFTNL_EXPR_FIB_DREG:
		*data_len = sizeof(fib->dreg);
		return &fib->dreg;
	case NFTNL_EXPR_FIB_FLAGS:
		*data_len = sizeof(fib->flags);
		return &fib->flags;
	}
	return NULL;
}

static int nftnl_expr_fib_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NFTA_FIB_MAX) < 0)
		return MNL_CB_OK;

	switch (type) {
	case NFTA_FIB_RESULT:
	case NFTA_FIB_DREG:
	case NFTA_FIB_FLAGS:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			abi_breakage();
		break;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static void
nftnl_expr_fib_build(struct nlmsghdr *nlh, const struct nftnl_expr *e)
{
	struct nftnl_expr_fib *fib = nftnl_expr_data(e);

	if (e->flags & (1 << NFTNL_EXPR_FIB_FLAGS))
		mnl_attr_put_u32(nlh, NFTA_FIB_FLAGS, htonl(fib->flags));
	if (e->flags & (1 << NFTNL_EXPR_FIB_RESULT))
		mnl_attr_put_u32(nlh, NFTA_FIB_RESULT, htonl(fib->result));
	if (e->flags & (1 << NFTNL_EXPR_FIB_DREG))
		mnl_attr_put_u32(nlh, NFTA_FIB_DREG, htonl(fib->dreg));
}

static int
nftnl_expr_fib_parse(struct nftnl_expr *e, struct nlattr *attr)
{
	struct nftnl_expr_fib *fib = nftnl_expr_data(e);
	struct nlattr *tb[NFTA_FIB_MAX+1] = {};
	int ret = 0;

	if (mnl_attr_parse_nested(attr, nftnl_expr_fib_cb, tb) < 0)
		return -1;

	if (tb[NFTA_FIB_RESULT]) {
		fib->result = ntohl(mnl_attr_get_u32(tb[NFTA_FIB_RESULT]));
		e->flags |= (1 << NFTNL_EXPR_FIB_RESULT);
	}
	if (tb[NFTA_FIB_DREG]) {
		fib->dreg = ntohl(mnl_attr_get_u32(tb[NFTA_FIB_DREG]));
		e->flags |= (1 << NFTNL_EXPR_FIB_DREG);
	}
	if (tb[NFTA_FIB_FLAGS]) {
		fib->flags = ntohl(mnl_attr_get_u32(tb[NFTA_FIB_FLAGS]));
		e->flags |= (1 << NFTNL_EXPR_FIB_FLAGS);
	}
	return ret;
}

static const char *fib_type[NFT_FIB_RESULT_MAX + 1] = {
	[NFT_FIB_RESULT_OIF] = "oif",
	[NFT_FIB_RESULT_OIFNAME] = "oifname",
	[NFT_FIB_RESULT_ADDRTYPE] = "type",
};

static const char *fib_type_str(enum nft_fib_result r)
{
	if (r <= NFT_FIB_RESULT_MAX)
		return fib_type[r];

	return "unknown";
}

static int
nftnl_expr_fib_snprintf(char *buf, size_t remain,
			 uint32_t printflags, const struct nftnl_expr *e)
{
	struct nftnl_expr_fib *fib = nftnl_expr_data(e);
	uint32_t flags = fib->flags & ~NFTA_FIB_F_PRESENT;
	uint32_t present_flag = fib->flags & NFTA_FIB_F_PRESENT;
	int offset = 0, ret, i;
	static const struct {
		int bit;
		const char *name;
	} tab[] = {
		{ NFTA_FIB_F_SADDR, "saddr" },
		{ NFTA_FIB_F_DADDR, "daddr" },
		{ NFTA_FIB_F_MARK, "mark" },
		{ NFTA_FIB_F_IIF, "iif" },
		{ NFTA_FIB_F_OIF, "oif" },
	};

	for (i = 0; i < (sizeof(tab) / sizeof(tab[0])); i++) {
		if (flags & tab[i].bit) {
			ret = snprintf(buf + offset, remain, "%s ",
				       tab[i].name);
			SNPRINTF_BUFFER_SIZE(ret, remain, offset);

			flags &= ~tab[i].bit;
			if (flags) {
				ret = snprintf(buf + offset, remain, ". ");
				SNPRINTF_BUFFER_SIZE(ret, remain, offset);
			}
		}
	}

	if (flags) {
		ret = snprintf(buf + offset, remain, "unknown 0x%" PRIx32,
			       flags);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);
	}

	ret = snprintf(buf + offset, remain, "%s%s => reg %d ",
		       fib_type_str(fib->result),
		       present_flag ? " present" : "",
		       fib->dreg);
	SNPRINTF_BUFFER_SIZE(ret, remain, offset);

	return offset;
}

static struct attr_policy fib_attr_policy[__NFTNL_EXPR_FIB_MAX] = {
	[NFTNL_EXPR_FIB_DREG]   = { .maxlen = sizeof(uint32_t) },
	[NFTNL_EXPR_FIB_RESULT] = { .maxlen = sizeof(uint32_t) },
	[NFTNL_EXPR_FIB_FLAGS]  = { .maxlen = sizeof(uint32_t) },
};

struct expr_ops expr_ops_fib = {
	.name		= "fib",
	.alloc_len	= sizeof(struct nftnl_expr_fib),
	.nftnl_max_attr	= __NFTNL_EXPR_FIB_MAX - 1,
	.attr_policy	= fib_attr_policy,
	.set		= nftnl_expr_fib_set,
	.get		= nftnl_expr_fib_get,
	.parse		= nftnl_expr_fib_parse,
	.build		= nftnl_expr_fib_build,
	.output		= nftnl_expr_fib_snprintf,
};
