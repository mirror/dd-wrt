/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) 2012-2013 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This code has been sponsored by Sophos Astaro <http://www.sophos.com>
 */

#include "internal.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h> /* for memcpy */
#include <arpa/inet.h>
#include <errno.h>
#include <libmnl/libmnl.h>
#include <linux/netfilter/nf_tables.h>
#include <libnftnl/expr.h>
#include <libnftnl/rule.h>

struct nftnl_expr_byteorder {
	enum nft_registers	sreg;
	enum nft_registers	dreg;
	enum nft_byteorder_ops	op;
	unsigned int		len;
	unsigned int		size;
};

static int
nftnl_expr_byteorder_set(struct nftnl_expr *e, uint16_t type,
			  const void *data, uint32_t data_len)
{
	struct nftnl_expr_byteorder *byteorder = nftnl_expr_data(e);

	switch(type) {
	case NFTNL_EXPR_BYTEORDER_SREG:
		memcpy(&byteorder->sreg, data, data_len);
		break;
	case NFTNL_EXPR_BYTEORDER_DREG:
		memcpy(&byteorder->dreg, data, data_len);
		break;
	case NFTNL_EXPR_BYTEORDER_OP:
		memcpy(&byteorder->op, data, data_len);
		break;
	case NFTNL_EXPR_BYTEORDER_LEN:
		memcpy(&byteorder->len, data, data_len);
		break;
	case NFTNL_EXPR_BYTEORDER_SIZE:
		memcpy(&byteorder->size, data, data_len);
		break;
	}
	return 0;
}

static const void *
nftnl_expr_byteorder_get(const struct nftnl_expr *e, uint16_t type,
			    uint32_t *data_len)
{
	struct nftnl_expr_byteorder *byteorder = nftnl_expr_data(e);

	switch(type) {
	case NFTNL_EXPR_BYTEORDER_SREG:
		*data_len = sizeof(byteorder->sreg);
		return &byteorder->sreg;
	case NFTNL_EXPR_BYTEORDER_DREG:
		*data_len = sizeof(byteorder->dreg);
		return &byteorder->dreg;
	case NFTNL_EXPR_BYTEORDER_OP:
		*data_len = sizeof(byteorder->op);
		return &byteorder->op;
	case NFTNL_EXPR_BYTEORDER_LEN:
		*data_len = sizeof(byteorder->len);
		return &byteorder->len;
	case NFTNL_EXPR_BYTEORDER_SIZE:
		*data_len = sizeof(byteorder->size);
		return &byteorder->size;
	}
	return NULL;
}

static int nftnl_expr_byteorder_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NFTA_BYTEORDER_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case NFTA_BYTEORDER_SREG:
	case NFTA_BYTEORDER_DREG:
	case NFTA_BYTEORDER_OP:
	case NFTA_BYTEORDER_LEN:
	case NFTA_BYTEORDER_SIZE:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			abi_breakage();
		break;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static void
nftnl_expr_byteorder_build(struct nlmsghdr *nlh, const struct nftnl_expr *e)
{
	struct nftnl_expr_byteorder *byteorder = nftnl_expr_data(e);

	if (e->flags & (1 << NFTNL_EXPR_BYTEORDER_SREG)) {
		mnl_attr_put_u32(nlh, NFTA_BYTEORDER_SREG,
				 htonl(byteorder->sreg));
	}
	if (e->flags & (1 << NFTNL_EXPR_BYTEORDER_DREG)) {
		mnl_attr_put_u32(nlh, NFTA_BYTEORDER_DREG,
				 htonl(byteorder->dreg));
	}
	if (e->flags & (1 << NFTNL_EXPR_BYTEORDER_OP)) {
		mnl_attr_put_u32(nlh, NFTA_BYTEORDER_OP,
				 htonl(byteorder->op));
	}
	if (e->flags & (1 << NFTNL_EXPR_BYTEORDER_LEN)) {
		mnl_attr_put_u32(nlh, NFTA_BYTEORDER_LEN,
				 htonl(byteorder->len));
	}
	if (e->flags & (1 << NFTNL_EXPR_BYTEORDER_SIZE)) {
		mnl_attr_put_u32(nlh, NFTA_BYTEORDER_SIZE,
				 htonl(byteorder->size));
	}
}

static int
nftnl_expr_byteorder_parse(struct nftnl_expr *e, struct nlattr *attr)
{
	struct nftnl_expr_byteorder *byteorder = nftnl_expr_data(e);
	struct nlattr *tb[NFTA_BYTEORDER_MAX+1] = {};
	int ret = 0;

	if (mnl_attr_parse_nested(attr, nftnl_expr_byteorder_cb, tb) < 0)
		return -1;

	if (tb[NFTA_BYTEORDER_SREG]) {
		byteorder->sreg =
			ntohl(mnl_attr_get_u32(tb[NFTA_BYTEORDER_SREG]));
		e->flags |= (1 << NFTNL_EXPR_BYTEORDER_SREG);
	}
	if (tb[NFTA_BYTEORDER_DREG]) {
		byteorder->dreg =
			ntohl(mnl_attr_get_u32(tb[NFTA_BYTEORDER_DREG]));
		e->flags |= (1 << NFTNL_EXPR_BYTEORDER_DREG);
	}
	if (tb[NFTA_BYTEORDER_OP]) {
		byteorder->op =
			ntohl(mnl_attr_get_u32(tb[NFTA_BYTEORDER_OP]));
		e->flags |= (1 << NFTNL_EXPR_BYTEORDER_OP);
	}
	if (tb[NFTA_BYTEORDER_LEN]) {
		byteorder->len =
			ntohl(mnl_attr_get_u32(tb[NFTA_BYTEORDER_LEN]));
		e->flags |= (1 << NFTNL_EXPR_BYTEORDER_LEN);
	}
	if (tb[NFTA_BYTEORDER_SIZE]) {
		byteorder->size =
			ntohl(mnl_attr_get_u32(tb[NFTA_BYTEORDER_SIZE]));
		e->flags |= (1 << NFTNL_EXPR_BYTEORDER_SIZE);
	}

	return ret;
}

static const char *expr_byteorder_str[] = {
	[NFT_BYTEORDER_HTON] = "hton",
	[NFT_BYTEORDER_NTOH] = "ntoh",
};

static const char *bo2str(uint32_t type)
{
	if (type > NFT_BYTEORDER_HTON)
		return "unknown";

	return expr_byteorder_str[type];
}

static int
nftnl_expr_byteorder_snprintf(char *buf, size_t remain,
			      uint32_t flags, const struct nftnl_expr *e)
{
	struct nftnl_expr_byteorder *byteorder = nftnl_expr_data(e);
	int offset = 0, ret;

	ret = snprintf(buf, remain, "reg %u = %s(reg %u, %u, %u) ",
		       byteorder->dreg, bo2str(byteorder->op),
		       byteorder->sreg, byteorder->size, byteorder->len);
	SNPRINTF_BUFFER_SIZE(ret, remain, offset);

	return offset;
}

static struct attr_policy byteorder_attr_policy[__NFTNL_EXPR_BYTEORDER_MAX] = {
	[NFTNL_EXPR_BYTEORDER_DREG] = { .maxlen = sizeof(uint32_t) },
	[NFTNL_EXPR_BYTEORDER_SREG] = { .maxlen = sizeof(uint32_t) },
	[NFTNL_EXPR_BYTEORDER_OP]   = { .maxlen = sizeof(uint32_t) },
	[NFTNL_EXPR_BYTEORDER_LEN]  = { .maxlen = sizeof(uint32_t) },
	[NFTNL_EXPR_BYTEORDER_SIZE] = { .maxlen = sizeof(uint32_t) },
};

struct expr_ops expr_ops_byteorder = {
	.name		= "byteorder",
	.alloc_len	= sizeof(struct nftnl_expr_byteorder),
	.nftnl_max_attr	= __NFTNL_EXPR_BYTEORDER_MAX - 1,
	.attr_policy	= byteorder_attr_policy,
	.set		= nftnl_expr_byteorder_set,
	.get		= nftnl_expr_byteorder_get,
	.parse		= nftnl_expr_byteorder_parse,
	.build		= nftnl_expr_byteorder_build,
	.output		= nftnl_expr_byteorder_snprintf,
};
