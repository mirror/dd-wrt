/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) 2016 by Pablo Neira Ayuso <pablo@netfilter.org>
 */

#include "internal.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>

#include <libmnl/libmnl.h>
#include <linux/netfilter/nf_tables.h>
#include <libnftnl/expr.h>
#include <libnftnl/rule.h>

struct nftnl_expr_range {
	union nftnl_data_reg	data_from;
	union nftnl_data_reg	data_to;
	enum nft_registers	sreg;
	enum nft_range_ops	op;
};

static int
nftnl_expr_range_set(struct nftnl_expr *e, uint16_t type,
		     const void *data, uint32_t data_len, uint32_t byteorder)
{
	struct nftnl_expr_range *range = nftnl_expr_data(e);

	switch(type) {
	case NFTNL_EXPR_RANGE_SREG:
		memcpy(&range->sreg, data, data_len);
		break;
	case NFTNL_EXPR_RANGE_OP:
		memcpy(&range->op, data, data_len);
		break;
	case NFTNL_EXPR_RANGE_FROM_DATA:
		return nftnl_data_cpy(&range->data_from, data,
				      data_len, byteorder, NULL);
	case NFTNL_EXPR_RANGE_TO_DATA:
		return nftnl_data_cpy(&range->data_to, data,
				      data_len, byteorder, NULL);
	}
	return 0;
}

static const void *nftnl_expr_range_get(const struct nftnl_expr *e,
					uint16_t type, uint32_t *data_len)
{
	struct nftnl_expr_range *range = nftnl_expr_data(e);

	switch(type) {
	case NFTNL_EXPR_RANGE_SREG:
		*data_len = sizeof(range->sreg);
		return &range->sreg;
	case NFTNL_EXPR_RANGE_OP:
		*data_len = sizeof(range->op);
		return &range->op;
	case NFTNL_EXPR_RANGE_FROM_DATA:
		*data_len = range->data_from.len;
		return &range->data_from.val;
	case NFTNL_EXPR_RANGE_TO_DATA:
		*data_len = range->data_to.len;
		return &range->data_to.val;
	}
	return NULL;
}

static int nftnl_expr_range_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NFTA_RANGE_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case NFTA_RANGE_SREG:
	case NFTA_RANGE_OP:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			abi_breakage();
		break;
	case NFTA_RANGE_FROM_DATA:
	case NFTA_RANGE_TO_DATA:
		if (mnl_attr_validate(attr, MNL_TYPE_BINARY) < 0)
			abi_breakage();
		break;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static void
nftnl_expr_range_build(struct nlmsghdr *nlh, const struct nftnl_expr *e)
{
	struct nftnl_expr_range *range = nftnl_expr_data(e);

	if (e->flags & (1 << NFTNL_EXPR_RANGE_SREG))
		mnl_attr_put_u32(nlh, NFTA_RANGE_SREG, htonl(range->sreg));
	if (e->flags & (1 << NFTNL_EXPR_RANGE_OP))
		mnl_attr_put_u32(nlh, NFTA_RANGE_OP, htonl(range->op));
	if (e->flags & (1 << NFTNL_EXPR_RANGE_FROM_DATA)) {
		struct nlattr *nest;

		nest = mnl_attr_nest_start(nlh, NFTA_RANGE_FROM_DATA);
		mnl_attr_put(nlh, NFTA_DATA_VALUE, range->data_from.len,
			     range->data_from.val);
		mnl_attr_nest_end(nlh, nest);
	}
	if (e->flags & (1 << NFTNL_EXPR_RANGE_TO_DATA)) {
		struct nlattr *nest;

		nest = mnl_attr_nest_start(nlh, NFTA_RANGE_TO_DATA);
		mnl_attr_put(nlh, NFTA_DATA_VALUE, range->data_to.len,
			     range->data_to.val);
		mnl_attr_nest_end(nlh, nest);
	}
}

static int
nftnl_expr_range_parse(struct nftnl_expr *e, struct nlattr *attr)
{
	struct nftnl_expr_range *range = nftnl_expr_data(e);
	struct nlattr *tb[NFTA_RANGE_MAX+1] = {};
	int ret = 0;

	if (mnl_attr_parse_nested(attr, nftnl_expr_range_cb, tb) < 0)
		return -1;

	if (tb[NFTA_RANGE_SREG]) {
		range->sreg = ntohl(mnl_attr_get_u32(tb[NFTA_RANGE_SREG]));
		e->flags |= (1 << NFTA_RANGE_SREG);
	}
	if (tb[NFTA_RANGE_OP]) {
		range->op = ntohl(mnl_attr_get_u32(tb[NFTA_RANGE_OP]));
		e->flags |= (1 << NFTA_RANGE_OP);
	}
	if (tb[NFTA_RANGE_FROM_DATA]) {
		ret = nftnl_parse_data(&range->data_from,
				       tb[NFTA_RANGE_FROM_DATA], NULL);
		e->flags |= (1 << NFTA_RANGE_FROM_DATA);
	}
	if (tb[NFTA_RANGE_TO_DATA]) {
		ret = nftnl_parse_data(&range->data_to,
				       tb[NFTA_RANGE_TO_DATA], NULL);
		e->flags |= (1 << NFTA_RANGE_TO_DATA);
	}

	return ret;
}

static const char *expr_range_str[] = {
	[NFT_RANGE_EQ]	= "eq",
	[NFT_RANGE_NEQ]	= "neq",
};

static const char *range2str(uint32_t op)
{
	if (op > NFT_RANGE_NEQ)
		return "unknown";

	return expr_range_str[op];
}

static int nftnl_expr_range_snprintf(char *buf, size_t remain,
				     uint32_t flags, const struct nftnl_expr *e)
{
	struct nftnl_expr_range *range = nftnl_expr_data(e);
	int offset = 0, ret;

	ret = snprintf(buf, remain, "%s reg %u ",
		       range2str(range->op), range->sreg);
	SNPRINTF_BUFFER_SIZE(ret, remain, offset);

	ret = nftnl_data_reg_snprintf(buf + offset, remain, &range->data_from,
				      0, DATA_VALUE);
	SNPRINTF_BUFFER_SIZE(ret, remain, offset);

	ret = snprintf(buf + offset, remain, " ");
	SNPRINTF_BUFFER_SIZE(ret, remain, offset);

	ret = nftnl_data_reg_snprintf(buf + offset, remain, &range->data_to,
				      0, DATA_VALUE);
	SNPRINTF_BUFFER_SIZE(ret, remain, offset);

	ret = snprintf(buf + offset, remain, " ");
	SNPRINTF_BUFFER_SIZE(ret, remain, offset);

	return offset;
}

static struct attr_policy range_attr_policy[__NFTNL_EXPR_RANGE_MAX] = {
	[NFTNL_EXPR_RANGE_SREG]      = { .maxlen = sizeof(uint32_t) },
	[NFTNL_EXPR_RANGE_OP]        = { .maxlen = sizeof(uint32_t) },
	[NFTNL_EXPR_RANGE_FROM_DATA] = { .maxlen = NFT_DATA_VALUE_MAXLEN },
	[NFTNL_EXPR_RANGE_TO_DATA]   = { .maxlen = NFT_DATA_VALUE_MAXLEN },
};

struct expr_ops expr_ops_range = {
	.name		= "range",
	.alloc_len	= sizeof(struct nftnl_expr_range),
	.nftnl_max_attr	= __NFTNL_EXPR_RANGE_MAX - 1,
	.attr_policy	= range_attr_policy,
	.set		= nftnl_expr_range_set,
	.get		= nftnl_expr_range_get,
	.parse		= nftnl_expr_range_parse,
	.build		= nftnl_expr_range_build,
	.output		= nftnl_expr_range_snprintf,
};
