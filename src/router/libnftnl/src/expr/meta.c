/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) 2012 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This code has been sponsored by Sophos Astaro <http://www.sophos.com>
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

#ifndef NFT_META_MAX
#define NFT_META_MAX (NFT_META_BRI_IIFHWADDR + 1)
#endif

struct nftnl_expr_meta {
	enum nft_meta_keys	key;
	enum nft_registers	dreg;
	enum nft_registers	sreg;
};

static int
nftnl_expr_meta_set(struct nftnl_expr *e, uint16_t type,
		       const void *data, uint32_t data_len)
{
	struct nftnl_expr_meta *meta = nftnl_expr_data(e);

	switch(type) {
	case NFTNL_EXPR_META_KEY:
		memcpy(&meta->key, data, data_len);
		break;
	case NFTNL_EXPR_META_DREG:
		memcpy(&meta->dreg, data, data_len);
		break;
	case NFTNL_EXPR_META_SREG:
		memcpy(&meta->sreg, data, data_len);
		break;
	}
	return 0;
}

static const void *
nftnl_expr_meta_get(const struct nftnl_expr *e, uint16_t type,
		       uint32_t *data_len)
{
	struct nftnl_expr_meta *meta = nftnl_expr_data(e);

	switch(type) {
	case NFTNL_EXPR_META_KEY:
		*data_len = sizeof(meta->key);
		return &meta->key;
	case NFTNL_EXPR_META_DREG:
		*data_len = sizeof(meta->dreg);
		return &meta->dreg;
	case NFTNL_EXPR_META_SREG:
		*data_len = sizeof(meta->sreg);
		return &meta->sreg;
	}
	return NULL;
}

static int nftnl_expr_meta_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NFTA_META_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case NFTA_META_KEY:
	case NFTA_META_DREG:
	case NFTA_META_SREG:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			abi_breakage();
		break;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static void
nftnl_expr_meta_build(struct nlmsghdr *nlh, const struct nftnl_expr *e)
{
	struct nftnl_expr_meta *meta = nftnl_expr_data(e);

	if (e->flags & (1 << NFTNL_EXPR_META_KEY))
		mnl_attr_put_u32(nlh, NFTA_META_KEY, htonl(meta->key));
	if (e->flags & (1 << NFTNL_EXPR_META_DREG))
		mnl_attr_put_u32(nlh, NFTA_META_DREG, htonl(meta->dreg));
	if (e->flags & (1 << NFTNL_EXPR_META_SREG))
		mnl_attr_put_u32(nlh, NFTA_META_SREG, htonl(meta->sreg));
}

static int
nftnl_expr_meta_parse(struct nftnl_expr *e, struct nlattr *attr)
{
	struct nftnl_expr_meta *meta = nftnl_expr_data(e);
	struct nlattr *tb[NFTA_META_MAX+1] = {};

	if (mnl_attr_parse_nested(attr, nftnl_expr_meta_cb, tb) < 0)
		return -1;

	if (tb[NFTA_META_KEY]) {
		meta->key = ntohl(mnl_attr_get_u32(tb[NFTA_META_KEY]));
		e->flags |= (1 << NFTNL_EXPR_META_KEY);
	}
	if (tb[NFTA_META_DREG]) {
		meta->dreg = ntohl(mnl_attr_get_u32(tb[NFTA_META_DREG]));
		e->flags |= (1 << NFTNL_EXPR_META_DREG);
	}
	if (tb[NFTA_META_SREG]) {
		meta->sreg = ntohl(mnl_attr_get_u32(tb[NFTA_META_SREG]));
		e->flags |= (1 << NFTNL_EXPR_META_SREG);
	}

	return 0;
}

static const char *meta_key2str_array[NFT_META_MAX] = {
	[NFT_META_LEN]		= "len",
	[NFT_META_PROTOCOL]	= "protocol",
	[NFT_META_NFPROTO]	= "nfproto",
	[NFT_META_L4PROTO]	= "l4proto",
	[NFT_META_PRIORITY]	= "priority",
	[NFT_META_MARK]		= "mark",
	[NFT_META_IIF]		= "iif",
	[NFT_META_OIF]		= "oif",
	[NFT_META_IIFNAME]	= "iifname",
	[NFT_META_OIFNAME]	= "oifname",
	[NFT_META_IIFTYPE]	= "iiftype",
	[NFT_META_OIFTYPE]	= "oiftype",
	[NFT_META_SKUID]	= "skuid",
	[NFT_META_SKGID]	= "skgid",
	[NFT_META_NFTRACE]	= "nftrace",
	[NFT_META_RTCLASSID]	= "rtclassid",
	[NFT_META_SECMARK]	= "secmark",
	[NFT_META_BRI_IIFNAME]	= "bri_iifname",
	[NFT_META_BRI_OIFNAME]	= "bri_oifname",
	[NFT_META_PKTTYPE]	= "pkttype",
	[NFT_META_CPU]		= "cpu",
	[NFT_META_IIFGROUP]	= "iifgroup",
	[NFT_META_OIFGROUP]	= "oifgroup",
	[NFT_META_CGROUP]	= "cgroup",
	[NFT_META_PRANDOM]	= "prandom",
	[NFT_META_SECPATH]	= "secpath",
	[NFT_META_IIFKIND]	= "iifkind",
	[NFT_META_OIFKIND]	= "oifkind",
	[NFT_META_BRI_IIFPVID]	 = "bri_iifpvid",
	[NFT_META_BRI_IIFVPROTO] = "bri_iifvproto",
	[NFT_META_TIME_NS]	= "time",
	[NFT_META_TIME_DAY]	= "day",
	[NFT_META_TIME_HOUR]	= "hour",
	[NFT_META_SDIF]		= "sdif",
	[NFT_META_SDIFNAME]	= "sdifname",
	[NFT_META_BRI_BROUTE]	= "broute",
	[NFT_META_BRI_IIFHWADDR] = "ibrhwaddr",
};

static const char *meta_key2str(uint8_t key)
{
	if (key < NFT_META_MAX)
		return meta_key2str_array[key];

	return "unknown";
}

static int
nftnl_expr_meta_snprintf(char *buf, size_t len,
			 uint32_t flags, const struct nftnl_expr *e)
{
	struct nftnl_expr_meta *meta = nftnl_expr_data(e);

	if (e->flags & (1 << NFTNL_EXPR_META_SREG)) {
		return snprintf(buf, len, "set %s with reg %u ",
				meta_key2str(meta->key), meta->sreg);
	}
	if (e->flags & (1 << NFTNL_EXPR_META_DREG)) {
		return snprintf(buf, len, "load %s => reg %u ",
				meta_key2str(meta->key), meta->dreg);
	}
	return 0;
}

static struct attr_policy meta_attr_policy[__NFTNL_EXPR_META_MAX] = {
	[NFTNL_EXPR_META_KEY]  = { .maxlen = sizeof(uint32_t) },
	[NFTNL_EXPR_META_DREG] = { .maxlen = sizeof(uint32_t) },
	[NFTNL_EXPR_META_SREG] = { .maxlen = sizeof(uint32_t) },
};

struct expr_ops expr_ops_meta = {
	.name		= "meta",
	.alloc_len	= sizeof(struct nftnl_expr_meta),
	.nftnl_max_attr	= __NFTNL_EXPR_META_MAX - 1,
	.attr_policy	= meta_attr_policy,
	.set		= nftnl_expr_meta_set,
	.get		= nftnl_expr_meta_get,
	.parse		= nftnl_expr_meta_parse,
	.build		= nftnl_expr_meta_build,
	.output		= nftnl_expr_meta_snprintf,
};
