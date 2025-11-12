/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) 2012 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This code has been sponsored by Sophos Astaro <http://www.sophos.com>
 */

#include "internal.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <arpa/inet.h>
#include <errno.h>
#include <libmnl/libmnl.h>

#include <linux/netfilter/nf_tables.h>

#include <libnftnl/expr.h>
#include <libnftnl/rule.h>

struct nftnl_expr_payload {
	enum nft_registers	sreg;
	enum nft_registers	dreg;
	enum nft_payload_bases	base;
	uint32_t		offset;
	uint32_t		len;
	uint32_t		csum_type;
	uint32_t		csum_offset;
	uint32_t		csum_flags;
};

static int
nftnl_expr_payload_set(struct nftnl_expr *e, uint16_t type,
			  const void *data, uint32_t data_len)
{
	struct nftnl_expr_payload *payload = nftnl_expr_data(e);

	switch(type) {
	case NFTNL_EXPR_PAYLOAD_SREG:
		memcpy(&payload->sreg, data, data_len);
		break;
	case NFTNL_EXPR_PAYLOAD_DREG:
		memcpy(&payload->dreg, data, data_len);
		break;
	case NFTNL_EXPR_PAYLOAD_BASE:
		memcpy(&payload->base, data, data_len);
		break;
	case NFTNL_EXPR_PAYLOAD_OFFSET:
		memcpy(&payload->offset, data, data_len);
		break;
	case NFTNL_EXPR_PAYLOAD_LEN:
		memcpy(&payload->len, data, data_len);
		break;
	case NFTNL_EXPR_PAYLOAD_CSUM_TYPE:
		memcpy(&payload->csum_type, data, data_len);
		break;
	case NFTNL_EXPR_PAYLOAD_CSUM_OFFSET:
		memcpy(&payload->csum_offset, data, data_len);
		break;
	case NFTNL_EXPR_PAYLOAD_FLAGS:
		memcpy(&payload->csum_flags, data, data_len);
		break;
	}
	return 0;
}

static const void *
nftnl_expr_payload_get(const struct nftnl_expr *e, uint16_t type,
			  uint32_t *data_len)
{
	struct nftnl_expr_payload *payload = nftnl_expr_data(e);

	switch(type) {
	case NFTNL_EXPR_PAYLOAD_SREG:
		*data_len = sizeof(payload->sreg);
		return &payload->sreg;
	case NFTNL_EXPR_PAYLOAD_DREG:
		*data_len = sizeof(payload->dreg);
		return &payload->dreg;
	case NFTNL_EXPR_PAYLOAD_BASE:
		*data_len = sizeof(payload->base);
		return &payload->base;
	case NFTNL_EXPR_PAYLOAD_OFFSET:
		*data_len = sizeof(payload->offset);
		return &payload->offset;
	case NFTNL_EXPR_PAYLOAD_LEN:
		*data_len = sizeof(payload->len);
		return &payload->len;
	case NFTNL_EXPR_PAYLOAD_CSUM_TYPE:
		*data_len = sizeof(payload->csum_type);
		return &payload->csum_type;
	case NFTNL_EXPR_PAYLOAD_CSUM_OFFSET:
		*data_len = sizeof(payload->csum_offset);
		return &payload->csum_offset;
	case NFTNL_EXPR_PAYLOAD_FLAGS:
		*data_len = sizeof(payload->csum_flags);
		return &payload->csum_flags;
	}
	return NULL;
}

static int nftnl_expr_payload_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NFTA_PAYLOAD_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case NFTA_PAYLOAD_SREG:
	case NFTA_PAYLOAD_DREG:
	case NFTA_PAYLOAD_BASE:
	case NFTA_PAYLOAD_OFFSET:
	case NFTA_PAYLOAD_LEN:
	case NFTA_PAYLOAD_CSUM_TYPE:
	case NFTA_PAYLOAD_CSUM_OFFSET:
	case NFTA_PAYLOAD_CSUM_FLAGS:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			abi_breakage();
		break;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static void
nftnl_expr_payload_build(struct nlmsghdr *nlh, const struct nftnl_expr *e)
{
	struct nftnl_expr_payload *payload = nftnl_expr_data(e);

	if (e->flags & (1 << NFTNL_EXPR_PAYLOAD_SREG))
		mnl_attr_put_u32(nlh, NFTA_PAYLOAD_SREG, htonl(payload->sreg));
	if (e->flags & (1 << NFTNL_EXPR_PAYLOAD_DREG))
		mnl_attr_put_u32(nlh, NFTA_PAYLOAD_DREG, htonl(payload->dreg));
	if (e->flags & (1 << NFTNL_EXPR_PAYLOAD_BASE))
		mnl_attr_put_u32(nlh, NFTA_PAYLOAD_BASE, htonl(payload->base));
	if (e->flags & (1 << NFTNL_EXPR_PAYLOAD_OFFSET))
		mnl_attr_put_u32(nlh, NFTA_PAYLOAD_OFFSET, htonl(payload->offset));
	if (e->flags & (1 << NFTNL_EXPR_PAYLOAD_LEN))
		mnl_attr_put_u32(nlh, NFTA_PAYLOAD_LEN, htonl(payload->len));
	if (e->flags & (1 << NFTNL_EXPR_PAYLOAD_CSUM_TYPE))
		mnl_attr_put_u32(nlh, NFTA_PAYLOAD_CSUM_TYPE,
				 htonl(payload->csum_type));
	if (e->flags & (1 << NFTNL_EXPR_PAYLOAD_CSUM_OFFSET))
		mnl_attr_put_u32(nlh, NFTA_PAYLOAD_CSUM_OFFSET,
				 htonl(payload->csum_offset));
	if (e->flags & (1 << NFTNL_EXPR_PAYLOAD_FLAGS))
		mnl_attr_put_u32(nlh, NFTA_PAYLOAD_CSUM_FLAGS,
				 htonl(payload->csum_flags));
}

static int
nftnl_expr_payload_parse(struct nftnl_expr *e, struct nlattr *attr)
{
	struct nftnl_expr_payload *payload = nftnl_expr_data(e);
	struct nlattr *tb[NFTA_PAYLOAD_MAX+1] = {};

	if (mnl_attr_parse_nested(attr, nftnl_expr_payload_cb, tb) < 0)
		return -1;

	if (tb[NFTA_PAYLOAD_SREG]) {
		payload->sreg = ntohl(mnl_attr_get_u32(tb[NFTA_PAYLOAD_SREG]));
		e->flags |= (1 << NFTNL_EXPR_PAYLOAD_SREG);
	}
	if (tb[NFTA_PAYLOAD_DREG]) {
		payload->dreg = ntohl(mnl_attr_get_u32(tb[NFTA_PAYLOAD_DREG]));
		e->flags |= (1 << NFTNL_EXPR_PAYLOAD_DREG);
	}
	if (tb[NFTA_PAYLOAD_BASE]) {
		payload->base = ntohl(mnl_attr_get_u32(tb[NFTA_PAYLOAD_BASE]));
		e->flags |= (1 << NFTNL_EXPR_PAYLOAD_BASE);
	}
	if (tb[NFTA_PAYLOAD_OFFSET]) {
		payload->offset = ntohl(mnl_attr_get_u32(tb[NFTA_PAYLOAD_OFFSET]));
		e->flags |= (1 << NFTNL_EXPR_PAYLOAD_OFFSET);
	}
	if (tb[NFTA_PAYLOAD_LEN]) {
		payload->len = ntohl(mnl_attr_get_u32(tb[NFTA_PAYLOAD_LEN]));
		e->flags |= (1 << NFTNL_EXPR_PAYLOAD_LEN);
	}
	if (tb[NFTA_PAYLOAD_CSUM_TYPE]) {
		payload->csum_type = ntohl(mnl_attr_get_u32(tb[NFTA_PAYLOAD_CSUM_TYPE]));
		e->flags |= (1 << NFTNL_EXPR_PAYLOAD_CSUM_TYPE);
	}
	if (tb[NFTA_PAYLOAD_CSUM_OFFSET]) {
		payload->csum_offset = ntohl(mnl_attr_get_u32(tb[NFTA_PAYLOAD_CSUM_OFFSET]));
		e->flags |= (1 << NFTNL_EXPR_PAYLOAD_CSUM_OFFSET);
	}
	if (tb[NFTA_PAYLOAD_CSUM_FLAGS]) {
		payload->csum_flags = ntohl(mnl_attr_get_u32(tb[NFTA_PAYLOAD_CSUM_FLAGS]));
		e->flags |= (1 << NFTNL_EXPR_PAYLOAD_FLAGS);
	}
	return 0;
}

static const char *base2str_array[NFT_PAYLOAD_TUN_HEADER + 1] = {
	[NFT_PAYLOAD_LL_HEADER]		= "link",
	[NFT_PAYLOAD_NETWORK_HEADER] 	= "network",
	[NFT_PAYLOAD_TRANSPORT_HEADER]	= "transport",
	[NFT_PAYLOAD_INNER_HEADER]	= "inner",
	[NFT_PAYLOAD_TUN_HEADER]	= "tunnel",
};

static const char *base2str(enum nft_payload_bases base)
{
	if (base >= array_size(base2str_array))
		return "unknown";

	return base2str_array[base];
}

static int
nftnl_expr_payload_snprintf(char *buf, size_t len,
			    uint32_t flags, const struct nftnl_expr *e)
{
	struct nftnl_expr_payload *payload = nftnl_expr_data(e);

	if (payload->sreg)
		return snprintf(buf, len, "write reg %u => %ub @ %s header + %u csum_type %u csum_off %u csum_flags 0x%x ",
				payload->sreg,
				payload->len, base2str(payload->base),
				payload->offset, payload->csum_type,
				payload->csum_offset,
				payload->csum_flags);
	else
		return snprintf(buf, len, "load %ub @ %s header + %u => reg %u ",
				payload->len, base2str(payload->base),
				payload->offset, payload->dreg);
}

static struct attr_policy payload_attr_policy[__NFTNL_EXPR_PAYLOAD_MAX] = {
	[NFTNL_EXPR_PAYLOAD_DREG]        = { .maxlen = sizeof(uint32_t) },
	[NFTNL_EXPR_PAYLOAD_BASE]        = { .maxlen = sizeof(uint32_t) },
	[NFTNL_EXPR_PAYLOAD_OFFSET]      = { .maxlen = sizeof(uint32_t) },
	[NFTNL_EXPR_PAYLOAD_LEN]         = { .maxlen = sizeof(uint32_t) },
	[NFTNL_EXPR_PAYLOAD_SREG]        = { .maxlen = sizeof(uint32_t) },
	[NFTNL_EXPR_PAYLOAD_CSUM_TYPE]   = { .maxlen = sizeof(uint32_t) },
	[NFTNL_EXPR_PAYLOAD_CSUM_OFFSET] = { .maxlen = sizeof(uint32_t) },
	[NFTNL_EXPR_PAYLOAD_FLAGS]       = { .maxlen = sizeof(uint32_t) },
};

struct expr_ops expr_ops_payload = {
	.name		= "payload",
	.alloc_len	= sizeof(struct nftnl_expr_payload),
	.nftnl_max_attr	= __NFTNL_EXPR_PAYLOAD_MAX - 1,
	.attr_policy	= payload_attr_policy,
	.set		= nftnl_expr_payload_set,
	.get		= nftnl_expr_payload_get,
	.parse		= nftnl_expr_payload_parse,
	.build		= nftnl_expr_payload_build,
	.output		= nftnl_expr_payload_snprintf,
};
