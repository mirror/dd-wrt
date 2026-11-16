/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2016 Anders K. Pedersen <akp@cohaesio.com>
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <linux/netfilter/nf_tables.h>

#include "internal.h"
#include <libmnl/libmnl.h>
#include <libnftnl/expr.h>
#include <libnftnl/rule.h>

struct nftnl_expr_rt {
	enum nft_rt_keys	key;
	enum nft_registers	dreg;
};

static int
nftnl_expr_rt_set(struct nftnl_expr *e, uint16_t type,
		  const void *data, uint32_t data_len, uint32_t byteorder)
{
	struct nftnl_expr_rt *rt = nftnl_expr_data(e);

	switch (type) {
	case NFTNL_EXPR_RT_KEY:
		memcpy(&rt->key, data, data_len);
		break;
	case NFTNL_EXPR_RT_DREG:
		memcpy(&rt->dreg, data, data_len);
		break;
	}
	return 0;
}

static const void *
nftnl_expr_rt_get(const struct nftnl_expr *e, uint16_t type,
		       uint32_t *data_len)
{
	struct nftnl_expr_rt *rt = nftnl_expr_data(e);

	switch (type) {
	case NFTNL_EXPR_RT_KEY:
		*data_len = sizeof(rt->key);
		return &rt->key;
	case NFTNL_EXPR_RT_DREG:
		*data_len = sizeof(rt->dreg);
		return &rt->dreg;
	}
	return NULL;
}

static int nftnl_expr_rt_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NFTA_RT_MAX) < 0)
		return MNL_CB_OK;

	switch (type) {
	case NFTA_RT_KEY:
	case NFTA_RT_DREG:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			abi_breakage();
		break;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static void
nftnl_expr_rt_build(struct nlmsghdr *nlh, const struct nftnl_expr *e)
{
	struct nftnl_expr_rt *rt = nftnl_expr_data(e);

	if (e->flags & (1 << NFTNL_EXPR_RT_KEY))
		mnl_attr_put_u32(nlh, NFTA_RT_KEY, htonl(rt->key));
	if (e->flags & (1 << NFTNL_EXPR_RT_DREG))
		mnl_attr_put_u32(nlh, NFTA_RT_DREG, htonl(rt->dreg));
}

static int
nftnl_expr_rt_parse(struct nftnl_expr *e, struct nlattr *attr)
{
	struct nftnl_expr_rt *rt = nftnl_expr_data(e);
	struct nlattr *tb[NFTA_RT_MAX+1] = {};

	if (mnl_attr_parse_nested(attr, nftnl_expr_rt_cb, tb) < 0)
		return -1;

	if (tb[NFTA_RT_KEY]) {
		rt->key = ntohl(mnl_attr_get_u32(tb[NFTA_RT_KEY]));
		e->flags |= (1 << NFTNL_EXPR_RT_KEY);
	}
	if (tb[NFTA_RT_DREG]) {
		rt->dreg = ntohl(mnl_attr_get_u32(tb[NFTA_RT_DREG]));
		e->flags |= (1 << NFTNL_EXPR_RT_DREG);
	}

	return 0;
}

static const char *rt_key2str_array[NFT_RT_MAX + 1] = {
	[NFT_RT_CLASSID]	= "classid",
	[NFT_RT_NEXTHOP4]	= "nexthop4",
	[NFT_RT_NEXTHOP6]	= "nexthop6",
	[NFT_RT_TCPMSS]		= "tcpmss",
	[NFT_RT_XFRM]		= "ipsec",
};

static const char *rt_key2str(uint8_t key)
{
	if (key <= NFT_RT_MAX)
		return rt_key2str_array[key];

	return "unknown";
}

static int
nftnl_expr_rt_snprintf(char *buf, size_t len,
		       uint32_t flags, const struct nftnl_expr *e)
{
	struct nftnl_expr_rt *rt = nftnl_expr_data(e);

	if (e->flags & (1 << NFTNL_EXPR_RT_DREG)) {
		return snprintf(buf, len, "load %s => reg %u ",
				rt_key2str(rt->key), rt->dreg);
	}
	return 0;
}

static struct attr_policy rt_attr_policy[__NFTNL_EXPR_RT_MAX] = {
	[NFTNL_EXPR_RT_KEY]  = { .maxlen = sizeof(uint32_t) },
	[NFTNL_EXPR_RT_DREG] = { .maxlen = sizeof(uint32_t) },
};

struct expr_ops expr_ops_rt = {
	.name		= "rt",
	.alloc_len	= sizeof(struct nftnl_expr_rt),
	.nftnl_max_attr	= __NFTNL_EXPR_RT_MAX - 1,
	.attr_policy	= rt_attr_policy,
	.set		= nftnl_expr_rt_set,
	.get		= nftnl_expr_rt_get,
	.parse		= nftnl_expr_rt_parse,
	.build		= nftnl_expr_rt_build,
	.output		= nftnl_expr_rt_snprintf,
};
