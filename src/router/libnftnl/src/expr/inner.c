/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) 2012-2022 by Pablo Neira Ayuso <pablo@netfilter.org>
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

struct nftnl_expr_inner {
	uint32_t	type;
	uint32_t	flags;
	uint32_t	hdrsize;
	struct nftnl_expr *expr;
};

static void nftnl_expr_inner_free(const struct nftnl_expr *e)
{
	struct nftnl_expr_inner *inner = nftnl_expr_data(e);

	if (inner->expr)
		nftnl_expr_free(inner->expr);
}

static int
nftnl_expr_inner_set(struct nftnl_expr *e, uint16_t type,
		     const void *data, uint32_t data_len)
{
	struct nftnl_expr_inner *inner = nftnl_expr_data(e);

	switch(type) {
	case NFTNL_EXPR_INNER_TYPE:
		memcpy(&inner->type, data, data_len);
		break;
	case NFTNL_EXPR_INNER_FLAGS:
		memcpy(&inner->flags, data, data_len);
		break;
	case NFTNL_EXPR_INNER_HDRSIZE:
		memcpy(&inner->hdrsize, data, data_len);
		break;
	case NFTNL_EXPR_INNER_EXPR:
		if (inner->expr)
			nftnl_expr_free(inner->expr);

		inner->expr = (void *)data;
		break;
	}
	return 0;
}

static const void *
nftnl_expr_inner_get(const struct nftnl_expr *e, uint16_t type,
		     uint32_t *data_len)
{
	struct nftnl_expr_inner *inner = nftnl_expr_data(e);

	switch(type) {
	case NFTNL_EXPR_INNER_FLAGS:
		*data_len = sizeof(inner->flags);
		return &inner->flags;
	case NFTNL_EXPR_INNER_TYPE:
		*data_len = sizeof(inner->type);
		return &inner->type;
	case NFTNL_EXPR_INNER_HDRSIZE:
		*data_len = sizeof(inner->hdrsize);
		return &inner->hdrsize;
	case NFTNL_EXPR_INNER_EXPR:
		return inner->expr;
	}
	return NULL;
}

static void
nftnl_expr_inner_build(struct nlmsghdr *nlh, const struct nftnl_expr *e)
{
	struct nftnl_expr_inner *inner = nftnl_expr_data(e);
	struct nlattr *nest;

	mnl_attr_put_u32(nlh, NFTA_INNER_NUM, htonl(0));
	if (e->flags & (1 << NFTNL_EXPR_INNER_TYPE))
		mnl_attr_put_u32(nlh, NFTA_INNER_TYPE, htonl(inner->type));
	if (e->flags & (1 << NFTNL_EXPR_INNER_FLAGS))
		mnl_attr_put_u32(nlh, NFTA_INNER_FLAGS, htonl(inner->flags));
	if (e->flags & (1 << NFTNL_EXPR_INNER_HDRSIZE))
		mnl_attr_put_u32(nlh, NFTA_INNER_HDRSIZE, htonl(inner->hdrsize));
	if (e->flags & (1 << NFTNL_EXPR_INNER_EXPR)) {
		nest = mnl_attr_nest_start(nlh, NFTA_INNER_EXPR);
		nftnl_expr_build_payload(nlh, inner->expr);
		mnl_attr_nest_end(nlh, nest);
	}
}

static int nftnl_inner_parse_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NFTA_INNER_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case NFTA_INNER_NUM:
	case NFTA_INNER_TYPE:
	case NFTA_INNER_HDRSIZE:
	case NFTA_INNER_FLAGS:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			abi_breakage();
		break;
	case NFTA_INNER_EXPR:
		if (mnl_attr_validate(attr, MNL_TYPE_NESTED) < 0)
			abi_breakage();
		break;
	}

	tb[type] = attr;

	return MNL_CB_OK;
}

static int
nftnl_expr_inner_parse(struct nftnl_expr *e, struct nlattr *attr)
{
	struct nftnl_expr_inner *inner = nftnl_expr_data(e);
	struct nlattr *tb[NFTA_INNER_MAX + 1] = {};
	struct nftnl_expr *expr;
	int err;

	err = mnl_attr_parse_nested(attr, nftnl_inner_parse_cb, tb);
	if (err < 0)
		return err;

	if (tb[NFTA_INNER_HDRSIZE]) {
		inner->hdrsize =
			ntohl(mnl_attr_get_u32(tb[NFTA_INNER_HDRSIZE]));
		e->flags |= (1 << NFTNL_EXPR_INNER_HDRSIZE);
	}
	if (tb[NFTA_INNER_FLAGS]) {
		inner->flags =
			ntohl(mnl_attr_get_u32(tb[NFTA_INNER_FLAGS]));
		e->flags |= (1 << NFTNL_EXPR_INNER_FLAGS);
	}
	if (tb[NFTA_INNER_TYPE]) {
		inner->type =
			ntohl(mnl_attr_get_u32(tb[NFTA_INNER_TYPE]));
		e->flags |= (1 << NFTNL_EXPR_INNER_TYPE);
	}
	if (tb[NFTA_INNER_EXPR]) {
		expr = nftnl_expr_parse(tb[NFTA_INNER_EXPR]);
		if (!expr)
			return -1;

		if (inner->expr)
			nftnl_expr_free(inner->expr);

		inner->expr = expr;
		e->flags |= (1 << NFTNL_EXPR_INNER_EXPR);
	}

	return 0;
}

static int
nftnl_expr_inner_snprintf(char *buf, size_t remain, uint32_t flags,
			  const struct nftnl_expr *e)
{
	struct nftnl_expr_inner *inner = nftnl_expr_data(e);
	uint32_t offset = 0;
	int ret;

	ret = snprintf(buf, remain, "type %u hdrsize %u flags %x [",
		       inner->type, inner->hdrsize, inner->flags);
	SNPRINTF_BUFFER_SIZE(ret, remain, offset);

	ret = snprintf(buf + offset, remain, " %s ", inner->expr->ops->name);
	SNPRINTF_BUFFER_SIZE(ret, remain, offset);

	ret = nftnl_expr_snprintf(buf + offset, remain, inner->expr,
				  NFTNL_OUTPUT_DEFAULT, 0);
	SNPRINTF_BUFFER_SIZE(ret, remain, offset);

	ret = snprintf(buf + offset, remain, "] ");
	SNPRINTF_BUFFER_SIZE(ret, remain, offset);

	return offset;
}

static struct attr_policy inner_attr_policy[__NFTNL_EXPR_INNER_MAX] = {
	[NFTNL_EXPR_INNER_TYPE]    = { .maxlen = sizeof(uint32_t) },
	[NFTNL_EXPR_INNER_FLAGS]   = { .maxlen = sizeof(uint32_t) },
	[NFTNL_EXPR_INNER_HDRSIZE] = { .maxlen = sizeof(uint32_t) },
	[NFTNL_EXPR_INNER_EXPR]    = { .maxlen = 0 },
};

struct expr_ops expr_ops_inner = {
	.name		= "inner",
	.alloc_len	= sizeof(struct nftnl_expr_inner),
	.nftnl_max_attr	= __NFTNL_EXPR_INNER_MAX - 1,
	.attr_policy	= inner_attr_policy,
	.free		= nftnl_expr_inner_free,
	.set		= nftnl_expr_inner_set,
	.get		= nftnl_expr_inner_get,
	.parse		= nftnl_expr_inner_parse,
	.build		= nftnl_expr_inner_build,
	.output		= nftnl_expr_inner_snprintf,
};
