/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) 2012-2013 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This code has been sponsored by Sophos Astaro <http://www.sophos.com>
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <errno.h>
#include <linux/netfilter/nf_tables.h>

#include "internal.h"
#include <libmnl/libmnl.h>
#include <libnftnl/expr.h>
#include <libnftnl/rule.h>

struct nftnl_expr_ct {
	enum nft_ct_keys        key;
	enum nft_registers	dreg;
	enum nft_registers	sreg;
	uint8_t			dir;
};

#define IP_CT_DIR_ORIGINAL	0
#define IP_CT_DIR_REPLY		1

static int
nftnl_expr_ct_set(struct nftnl_expr *e, uint16_t type,
		  const void *data, uint32_t data_len, uint32_t byteorder)
{
	struct nftnl_expr_ct *ct = nftnl_expr_data(e);

	switch(type) {
	case NFTNL_EXPR_CT_KEY:
		memcpy(&ct->key, data, data_len);
		break;
	case NFTNL_EXPR_CT_DIR:
		memcpy(&ct->dir, data, data_len);
		break;
	case NFTNL_EXPR_CT_DREG:
		memcpy(&ct->dreg, data, data_len);
		break;
	case NFTNL_EXPR_CT_SREG:
		memcpy(&ct->sreg, data, data_len);
		break;
	}
	return 0;
}

static const void *
nftnl_expr_ct_get(const struct nftnl_expr *e, uint16_t type,
		     uint32_t *data_len)
{
	struct nftnl_expr_ct *ct = nftnl_expr_data(e);

	switch(type) {
	case NFTNL_EXPR_CT_KEY:
		*data_len = sizeof(ct->key);
		return &ct->key;
	case NFTNL_EXPR_CT_DIR:
		*data_len = sizeof(ct->dir);
		return &ct->dir;
	case NFTNL_EXPR_CT_DREG:
		*data_len = sizeof(ct->dreg);
		return &ct->dreg;
	case NFTNL_EXPR_CT_SREG:
		*data_len = sizeof(ct->sreg);
		return &ct->sreg;
	}
	return NULL;
}

static int nftnl_expr_ct_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NFTA_CT_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case NFTA_CT_KEY:
	case NFTA_CT_DREG:
	case NFTA_CT_SREG:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			abi_breakage();
		break;
	case NFTA_CT_DIRECTION:
		if (mnl_attr_validate(attr, MNL_TYPE_U8) < 0)
			abi_breakage();
		break;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static void
nftnl_expr_ct_build(struct nlmsghdr *nlh, const struct nftnl_expr *e)
{
	struct nftnl_expr_ct *ct = nftnl_expr_data(e);

	if (e->flags & (1 << NFTNL_EXPR_CT_KEY))
		mnl_attr_put_u32(nlh, NFTA_CT_KEY, htonl(ct->key));
	if (e->flags & (1 << NFTNL_EXPR_CT_DREG))
		mnl_attr_put_u32(nlh, NFTA_CT_DREG, htonl(ct->dreg));
	if (e->flags & (1 << NFTNL_EXPR_CT_DIR))
		mnl_attr_put_u8(nlh, NFTA_CT_DIRECTION, ct->dir);
	if (e->flags & (1 << NFTNL_EXPR_CT_SREG))
		mnl_attr_put_u32(nlh, NFTA_CT_SREG, htonl(ct->sreg));
}

static int
nftnl_expr_ct_parse(struct nftnl_expr *e, struct nlattr *attr)
{
	struct nftnl_expr_ct *ct = nftnl_expr_data(e);
	struct nlattr *tb[NFTA_CT_MAX+1] = {};

	if (mnl_attr_parse_nested(attr, nftnl_expr_ct_cb, tb) < 0)
		return -1;

	if (tb[NFTA_CT_KEY]) {
		ct->key = ntohl(mnl_attr_get_u32(tb[NFTA_CT_KEY]));
		e->flags |= (1 << NFTNL_EXPR_CT_KEY);
	}
	if (tb[NFTA_CT_DREG]) {
		ct->dreg = ntohl(mnl_attr_get_u32(tb[NFTA_CT_DREG]));
		e->flags |= (1 << NFTNL_EXPR_CT_DREG);
	}
	if (tb[NFTA_CT_SREG]) {
		ct->sreg = ntohl(mnl_attr_get_u32(tb[NFTA_CT_SREG]));
		e->flags |= (1 << NFTNL_EXPR_CT_SREG);
	}
	if (tb[NFTA_CT_DIRECTION]) {
		ct->dir = mnl_attr_get_u8(tb[NFTA_CT_DIRECTION]);
		e->flags |= (1 << NFTNL_EXPR_CT_DIR);
	}

	return 0;
}

static const char *ctkey2str_array[NFT_CT_MAX + 1] = {
	[NFT_CT_STATE]		= "state",
	[NFT_CT_DIRECTION]	= "direction",
	[NFT_CT_STATUS]		= "status",
	[NFT_CT_MARK]		= "mark",
	[NFT_CT_SECMARK]	= "secmark",
	[NFT_CT_EXPIRATION]	= "expiration",
	[NFT_CT_HELPER]		= "helper",
	[NFT_CT_L3PROTOCOL]	= "l3protocol",
	[NFT_CT_PROTOCOL]	= "protocol",
	[NFT_CT_SRC]		= "src",
	[NFT_CT_DST]		= "dst",
	[NFT_CT_PROTO_SRC]	= "proto_src",
	[NFT_CT_PROTO_DST]	= "proto_dst",
	[NFT_CT_LABELS]		= "label",
	[NFT_CT_PKTS]		= "packets",
	[NFT_CT_BYTES]		= "bytes",
	[NFT_CT_AVGPKT]		= "avgpkt",
	[NFT_CT_ZONE]		= "zone",
	[NFT_CT_EVENTMASK]	= "event",
	[NFT_CT_SRC_IP]		= "src_ip",
	[NFT_CT_DST_IP]		= "dst_ip",
	[NFT_CT_SRC_IP6]	= "src_ip6",
	[NFT_CT_DST_IP6]	= "dst_ip6",
	[NFT_CT_ID]		= "id",
};

static const char *ctkey2str(uint32_t ctkey)
{
	if (ctkey > NFT_CT_MAX)
		return "unknown";

	return ctkey2str_array[ctkey];
}

static const char *ctdir2str(uint8_t ctdir)
{
	switch (ctdir) {
	case IP_CT_DIR_ORIGINAL:
		return "original";
	case IP_CT_DIR_REPLY:
		return "reply";
	default:
		return "unknown";
	}
}

static int
nftnl_expr_ct_snprintf(char *buf, size_t remain,
		       uint32_t flags, const struct nftnl_expr *e)
{
	struct nftnl_expr_ct *ct = nftnl_expr_data(e);
	int ret, offset = 0;

	if (e->flags & (1 << NFTNL_EXPR_CT_SREG)) {
		ret = snprintf(buf, remain, "set %s with reg %u ",
				ctkey2str(ct->key), ct->sreg);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);
	}

	if (e->flags & (1 << NFTNL_EXPR_CT_DREG)) {
		ret = snprintf(buf, remain, "load %s => reg %u ",
			       ctkey2str(ct->key), ct->dreg);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);
	}

	if (nftnl_expr_is_set(e, NFTNL_EXPR_CT_DIR)) {
		ret = snprintf(buf + offset, remain, ", dir %s ",
			       ctdir2str(ct->dir));
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);
	}

	return offset;
}

static struct attr_policy ct_attr_policy[__NFTNL_EXPR_CT_MAX] = {
	[NFTNL_EXPR_CT_DREG] = { .maxlen = sizeof(uint32_t) },
	[NFTNL_EXPR_CT_KEY]  = { .maxlen = sizeof(uint32_t) },
	[NFTNL_EXPR_CT_DIR]  = { .maxlen = sizeof(uint8_t) },
	[NFTNL_EXPR_CT_SREG] = { .maxlen = sizeof(uint32_t) },
};

struct expr_ops expr_ops_ct = {
	.name		= "ct",
	.alloc_len	= sizeof(struct nftnl_expr_ct),
	.nftnl_max_attr	= __NFTNL_EXPR_CT_MAX - 1,
	.attr_policy	= ct_attr_policy,
	.set		= nftnl_expr_ct_set,
	.get		= nftnl_expr_ct_get,
	.parse		= nftnl_expr_ct_parse,
	.build		= nftnl_expr_ct_build,
	.output		= nftnl_expr_ct_snprintf,
};
