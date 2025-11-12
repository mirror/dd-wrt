/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) 2016 by Laura Garcia <nevola@gmail.com>
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

struct nftnl_expr_ng {
	enum nft_registers	dreg;
	unsigned int		modulus;
	enum nft_ng_types	type;
	unsigned int		offset;
};

static int
nftnl_expr_ng_set(struct nftnl_expr *e, uint16_t type,
		  const void *data, uint32_t data_len)
{
	struct nftnl_expr_ng *ng = nftnl_expr_data(e);

	switch (type) {
	case NFTNL_EXPR_NG_DREG:
		memcpy(&ng->dreg, data, data_len);
		break;
	case NFTNL_EXPR_NG_MODULUS:
		memcpy(&ng->modulus, data, data_len);
		break;
	case NFTNL_EXPR_NG_TYPE:
		memcpy(&ng->type, data, data_len);
		break;
	case NFTNL_EXPR_NG_OFFSET:
		memcpy(&ng->offset, data, data_len);
		break;
	default:
		return -1;
	}
	return 0;
}

static const void *
nftnl_expr_ng_get(const struct nftnl_expr *e, uint16_t type,
		  uint32_t *data_len)
{
	struct nftnl_expr_ng *ng = nftnl_expr_data(e);

	switch (type) {
	case NFTNL_EXPR_NG_DREG:
		*data_len = sizeof(ng->dreg);
		return &ng->dreg;
	case NFTNL_EXPR_NG_MODULUS:
		*data_len = sizeof(ng->modulus);
		return &ng->modulus;
	case NFTNL_EXPR_NG_TYPE:
		*data_len = sizeof(ng->type);
		return &ng->type;
	case NFTNL_EXPR_NG_OFFSET:
		*data_len = sizeof(ng->offset);
		return &ng->offset;
	}
	return NULL;
}

static int nftnl_expr_ng_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NFTA_NG_MAX) < 0)
		return MNL_CB_OK;

	switch (type) {
	case NFTA_NG_DREG:
	case NFTA_NG_MODULUS:
	case NFTA_NG_TYPE:
	case NFTA_NG_OFFSET:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			abi_breakage();
		break;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static void
nftnl_expr_ng_build(struct nlmsghdr *nlh, const struct nftnl_expr *e)
{
	struct nftnl_expr_ng *ng = nftnl_expr_data(e);

	if (e->flags & (1 << NFTNL_EXPR_NG_DREG))
		mnl_attr_put_u32(nlh, NFTA_NG_DREG, htonl(ng->dreg));
	if (e->flags & (1 << NFTNL_EXPR_NG_MODULUS))
		mnl_attr_put_u32(nlh, NFTA_NG_MODULUS, htonl(ng->modulus));
	if (e->flags & (1 << NFTNL_EXPR_NG_TYPE))
		mnl_attr_put_u32(nlh, NFTA_NG_TYPE, htonl(ng->type));
	if (e->flags & (1 << NFTNL_EXPR_NG_OFFSET))
		mnl_attr_put_u32(nlh, NFTA_NG_OFFSET, htonl(ng->offset));
}

static int
nftnl_expr_ng_parse(struct nftnl_expr *e, struct nlattr *attr)
{
	struct nftnl_expr_ng *ng = nftnl_expr_data(e);
	struct nlattr *tb[NFTA_NG_MAX+1] = {};
	int ret = 0;

	if (mnl_attr_parse_nested(attr, nftnl_expr_ng_cb, tb) < 0)
		return -1;

	if (tb[NFTA_NG_DREG]) {
		ng->dreg = ntohl(mnl_attr_get_u32(tb[NFTA_NG_DREG]));
		e->flags |= (1 << NFTNL_EXPR_NG_DREG);
	}
	if (tb[NFTA_NG_MODULUS]) {
		ng->modulus = ntohl(mnl_attr_get_u32(tb[NFTA_NG_MODULUS]));
		e->flags |= (1 << NFTNL_EXPR_NG_MODULUS);
	}
	if (tb[NFTA_NG_TYPE]) {
		ng->type = ntohl(mnl_attr_get_u32(tb[NFTA_NG_TYPE]));
		e->flags |= (1 << NFTNL_EXPR_NG_TYPE);
	}
	if (tb[NFTA_NG_OFFSET]) {
		ng->offset = ntohl(mnl_attr_get_u32(tb[NFTA_NG_OFFSET]));
		e->flags |= (1 << NFTNL_EXPR_NG_OFFSET);
	}

	return ret;
}

static int
nftnl_expr_ng_snprintf(char *buf, size_t remain,
		       uint32_t flags, const struct nftnl_expr *e)
{
	struct nftnl_expr_ng *ng = nftnl_expr_data(e);
	int offset = 0, ret;

	switch (ng->type) {
	case NFT_NG_INCREMENTAL:
		ret = snprintf(buf, remain, "reg %u = inc mod %u ",
			       ng->dreg, ng->modulus);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);
		break;
	case NFT_NG_RANDOM:
		ret = snprintf(buf, remain, "reg %u = random mod %u ",
			       ng->dreg, ng->modulus);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);
		break;
	default:
		return 0;
	}

	if (ng->offset) {
		ret = snprintf(buf + offset, remain, "offset %u ", ng->offset);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);
	}

	return offset;
}

static struct attr_policy numgen_attr_policy[__NFTNL_EXPR_NG_MAX] = {
	[NFTNL_EXPR_NG_DREG]    = { .maxlen = sizeof(uint32_t) },
	[NFTNL_EXPR_NG_MODULUS] = { .maxlen = sizeof(uint32_t) },
	[NFTNL_EXPR_NG_TYPE]    = { .maxlen = sizeof(uint32_t) },
	[NFTNL_EXPR_NG_OFFSET]  = { .maxlen = sizeof(uint32_t) },
};

struct expr_ops expr_ops_ng = {
	.name		= "numgen",
	.alloc_len	= sizeof(struct nftnl_expr_ng),
	.nftnl_max_attr	= __NFTNL_EXPR_NG_MAX - 1,
	.attr_policy	= numgen_attr_policy,
	.set		= nftnl_expr_ng_set,
	.get		= nftnl_expr_ng_get,
	.parse		= nftnl_expr_ng_parse,
	.build		= nftnl_expr_ng_build,
	.output		= nftnl_expr_ng_snprintf,
};
