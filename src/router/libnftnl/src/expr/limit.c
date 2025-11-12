/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) 2012-2013 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This code has been sponsored by Sophos Astaro <http://www.sophos.com>
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

struct nftnl_expr_limit {
	uint64_t		rate;
	uint64_t		unit;
	uint32_t		burst;
	enum nft_limit_type	type;
	uint32_t		flags;
};

static int
nftnl_expr_limit_set(struct nftnl_expr *e, uint16_t type,
		       const void *data, uint32_t data_len)
{
	struct nftnl_expr_limit *limit = nftnl_expr_data(e);

	switch(type) {
	case NFTNL_EXPR_LIMIT_RATE:
		memcpy(&limit->rate, data, data_len);
		break;
	case NFTNL_EXPR_LIMIT_UNIT:
		memcpy(&limit->unit, data, data_len);
		break;
	case NFTNL_EXPR_LIMIT_BURST:
		memcpy(&limit->burst, data, data_len);
		break;
	case NFTNL_EXPR_LIMIT_TYPE:
		memcpy(&limit->type, data, data_len);
		break;
	case NFTNL_EXPR_LIMIT_FLAGS:
		memcpy(&limit->flags, data, data_len);
		break;
	}
	return 0;
}

static const void *
nftnl_expr_limit_get(const struct nftnl_expr *e, uint16_t type,
			uint32_t *data_len)
{
	struct nftnl_expr_limit *limit = nftnl_expr_data(e);

	switch(type) {
	case NFTNL_EXPR_LIMIT_RATE:
		*data_len = sizeof(uint64_t);
		return &limit->rate;
	case NFTNL_EXPR_LIMIT_UNIT:
		*data_len = sizeof(uint64_t);
		return &limit->unit;
	case NFTNL_EXPR_LIMIT_BURST:
		*data_len = sizeof(uint32_t);
		return &limit->burst;
	case NFTNL_EXPR_LIMIT_TYPE:
		*data_len = sizeof(uint32_t);
		return &limit->type;
	case NFTNL_EXPR_LIMIT_FLAGS:
		*data_len = sizeof(uint32_t);
		return &limit->flags;
	}
	return NULL;
}

static int nftnl_expr_limit_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NFTA_LIMIT_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case NFTA_LIMIT_RATE:
	case NFTA_LIMIT_UNIT:
		if (mnl_attr_validate(attr, MNL_TYPE_U64) < 0)
			abi_breakage();
		break;
	case NFTA_LIMIT_BURST:
	case NFTA_LIMIT_TYPE:
	case NFTA_LIMIT_FLAGS:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			abi_breakage();
		break;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static void
nftnl_expr_limit_build(struct nlmsghdr *nlh, const struct nftnl_expr *e)
{
	struct nftnl_expr_limit *limit = nftnl_expr_data(e);

	if (e->flags & (1 << NFTNL_EXPR_LIMIT_RATE))
		mnl_attr_put_u64(nlh, NFTA_LIMIT_RATE, htobe64(limit->rate));
	if (e->flags & (1 << NFTNL_EXPR_LIMIT_UNIT))
		mnl_attr_put_u64(nlh, NFTA_LIMIT_UNIT, htobe64(limit->unit));
	if (e->flags & (1 << NFTNL_EXPR_LIMIT_BURST))
		mnl_attr_put_u32(nlh, NFTA_LIMIT_BURST, htonl(limit->burst));
	if (e->flags & (1 << NFTNL_EXPR_LIMIT_TYPE))
		mnl_attr_put_u32(nlh, NFTA_LIMIT_TYPE, htonl(limit->type));
	if (e->flags & (1 << NFTNL_EXPR_LIMIT_FLAGS))
		mnl_attr_put_u32(nlh, NFTA_LIMIT_FLAGS, htonl(limit->flags));
}

static int
nftnl_expr_limit_parse(struct nftnl_expr *e, struct nlattr *attr)
{
	struct nftnl_expr_limit *limit = nftnl_expr_data(e);
	struct nlattr *tb[NFTA_LIMIT_MAX+1] = {};

	if (mnl_attr_parse_nested(attr, nftnl_expr_limit_cb, tb) < 0)
		return -1;

	if (tb[NFTA_LIMIT_RATE]) {
		limit->rate = be64toh(mnl_attr_get_u64(tb[NFTA_LIMIT_RATE]));
		e->flags |= (1 << NFTNL_EXPR_LIMIT_RATE);
	}
	if (tb[NFTA_LIMIT_UNIT]) {
		limit->unit = be64toh(mnl_attr_get_u64(tb[NFTA_LIMIT_UNIT]));
		e->flags |= (1 << NFTNL_EXPR_LIMIT_UNIT);
	}
	if (tb[NFTA_LIMIT_BURST]) {
		limit->burst = ntohl(mnl_attr_get_u32(tb[NFTA_LIMIT_BURST]));
		e->flags |= (1 << NFTNL_EXPR_LIMIT_BURST);
	}
	if (tb[NFTA_LIMIT_TYPE]) {
		limit->type = ntohl(mnl_attr_get_u32(tb[NFTA_LIMIT_TYPE]));
		e->flags |= (1 << NFTNL_EXPR_LIMIT_TYPE);
	}
	if (tb[NFTA_LIMIT_FLAGS]) {
		limit->flags = ntohl(mnl_attr_get_u32(tb[NFTA_LIMIT_FLAGS]));
		e->flags |= (1 << NFTNL_EXPR_LIMIT_FLAGS);
	}

	return 0;
}

static const char *get_time(uint64_t seconds, uint64_t *val)
{
	static const struct {
		unsigned int size;
		const char *name;
	} units[] = {
		{ 0,  "second" },
		{ 60, "minute" },
		{ 60, "hour" },
		{ 24, "day" },
		{ 7,  "week" }
	};
	int i;

	for (i = 1; i < array_size(units); i++) {
		if (seconds % units[i].size)
			break;
		seconds /= units[i].size;
	}
	if (val)
		*val = seconds;
	return units[i - 1].name;
}

static const char *limit_to_type(enum nft_limit_type type)
{
	switch (type) {
	default:
	case NFT_LIMIT_PKTS:
		return "packets";
	case NFT_LIMIT_PKT_BYTES:
		return "bytes";
	}
}

static int
nftnl_expr_limit_snprintf(char *buf, size_t len,
			  uint32_t flags, const struct nftnl_expr *e)
{
	struct nftnl_expr_limit *limit = nftnl_expr_data(e);
	unsigned int offset = 0;
	const char *time_unit;
	uint64_t time_val;
	int ret;

	ret = snprintf(buf, len, "rate %"PRIu64"/", limit->rate);
	SNPRINTF_BUFFER_SIZE(ret, len, offset);

	time_unit = get_time(limit->unit, &time_val);
	if (time_val > 1) {
		ret = snprintf(buf + offset, len, "%"PRIu64" ", time_val);
		SNPRINTF_BUFFER_SIZE(ret, len, offset);
	}

	ret = snprintf(buf + offset, len, "%s burst %u type %s flags 0x%x ",
		       time_unit, limit->burst, limit_to_type(limit->type),
		       limit->flags);
	SNPRINTF_BUFFER_SIZE(ret, len, offset);

	return offset;
}

static struct attr_policy limit_attr_policy[__NFTNL_EXPR_LIMIT_MAX] = {
	[NFTNL_EXPR_LIMIT_RATE]  = { .maxlen = sizeof(uint64_t) },
	[NFTNL_EXPR_LIMIT_UNIT]  = { .maxlen = sizeof(uint64_t) },
	[NFTNL_EXPR_LIMIT_BURST] = { .maxlen = sizeof(uint32_t) },
	[NFTNL_EXPR_LIMIT_TYPE]  = { .maxlen = sizeof(uint32_t) },
	[NFTNL_EXPR_LIMIT_FLAGS] = { .maxlen = sizeof(uint32_t) },
};

struct expr_ops expr_ops_limit = {
	.name		= "limit",
	.alloc_len	= sizeof(struct nftnl_expr_limit),
	.nftnl_max_attr	= __NFTNL_EXPR_LIMIT_MAX - 1,
	.attr_policy	= limit_attr_policy,
	.set		= nftnl_expr_limit_set,
	.get		= nftnl_expr_limit_get,
	.parse		= nftnl_expr_limit_parse,
	.build		= nftnl_expr_limit_build,
	.output		= nftnl_expr_limit_snprintf,
};
