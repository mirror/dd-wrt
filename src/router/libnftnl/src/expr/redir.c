/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) 2014 by Arturo Borrero Gonzalez <arturo@debian.org>
 */

#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <errno.h>
#include <inttypes.h>

#include <linux/netfilter/nf_tables.h>

#include "internal.h"
#include <libmnl/libmnl.h>
#include <libnftnl/expr.h>
#include <libnftnl/rule.h>

struct nftnl_expr_redir {
	enum nft_registers sreg_proto_min;
	enum nft_registers sreg_proto_max;
	uint32_t	flags;
};

static int
nftnl_expr_redir_set(struct nftnl_expr *e, uint16_t type,
			const void *data, uint32_t data_len)
{
	struct nftnl_expr_redir *redir = nftnl_expr_data(e);

	switch (type) {
	case NFTNL_EXPR_REDIR_REG_PROTO_MIN:
		memcpy(&redir->sreg_proto_min, data, data_len);
		break;
	case NFTNL_EXPR_REDIR_REG_PROTO_MAX:
		memcpy(&redir->sreg_proto_max, data, data_len);
		break;
	case NFTNL_EXPR_REDIR_FLAGS:
		memcpy(&redir->flags, data, data_len);
		break;
	}
	return 0;
}

static const void *
nftnl_expr_redir_get(const struct nftnl_expr *e, uint16_t type,
			uint32_t *data_len)
{
	struct nftnl_expr_redir *redir = nftnl_expr_data(e);

	switch (type) {
	case NFTNL_EXPR_REDIR_REG_PROTO_MIN:
		*data_len = sizeof(redir->sreg_proto_min);
		return &redir->sreg_proto_min;
	case NFTNL_EXPR_REDIR_REG_PROTO_MAX:
		*data_len = sizeof(redir->sreg_proto_max);
		return &redir->sreg_proto_max;
	case NFTNL_EXPR_REDIR_FLAGS:
		*data_len = sizeof(redir->flags);
		return &redir->flags;
	}
	return NULL;
}

static int nftnl_expr_redir_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NFTA_REDIR_MAX) < 0)
		return MNL_CB_OK;

	switch (type) {
	case NFTA_REDIR_REG_PROTO_MIN:
	case NFTA_REDIR_REG_PROTO_MAX:
	case NFTA_REDIR_FLAGS:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			abi_breakage();
		break;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static void
nftnl_expr_redir_build(struct nlmsghdr *nlh, const struct nftnl_expr *e)
{
	struct nftnl_expr_redir *redir = nftnl_expr_data(e);

	if (e->flags & (1 << NFTNL_EXPR_REDIR_REG_PROTO_MIN))
		mnl_attr_put_u32(nlh, NFTA_REDIR_REG_PROTO_MIN,
				 htobe32(redir->sreg_proto_min));
	if (e->flags & (1 << NFTNL_EXPR_REDIR_REG_PROTO_MAX))
		mnl_attr_put_u32(nlh, NFTA_REDIR_REG_PROTO_MAX,
				 htobe32(redir->sreg_proto_max));
	if (e->flags & (1 << NFTNL_EXPR_REDIR_FLAGS))
		mnl_attr_put_u32(nlh, NFTA_REDIR_FLAGS, htobe32(redir->flags));
}

static int
nftnl_expr_redir_parse(struct nftnl_expr *e, struct nlattr *attr)
{
	struct nftnl_expr_redir *redir = nftnl_expr_data(e);
	struct nlattr *tb[NFTA_REDIR_MAX + 1] = {};

	if (mnl_attr_parse_nested(attr, nftnl_expr_redir_cb, tb) < 0)
		return -1;

	if (tb[NFTA_REDIR_REG_PROTO_MIN]) {
		redir->sreg_proto_min =
			ntohl(mnl_attr_get_u32(tb[NFTA_REDIR_REG_PROTO_MIN]));
		e->flags |= (1 << NFTNL_EXPR_REDIR_REG_PROTO_MIN);
	}
	if (tb[NFTA_REDIR_REG_PROTO_MAX]) {
		redir->sreg_proto_max =
			ntohl(mnl_attr_get_u32(tb[NFTA_REDIR_REG_PROTO_MAX]));
		e->flags |= (1 << NFTNL_EXPR_REDIR_REG_PROTO_MAX);
	}
	if (tb[NFTA_REDIR_FLAGS]) {
		redir->flags = be32toh(mnl_attr_get_u32(tb[NFTA_REDIR_FLAGS]));
		e->flags |= (1 << NFTNL_EXPR_REDIR_FLAGS);
	}

	return 0;
}

static int
nftnl_expr_redir_snprintf(char *buf, size_t remain,
			  uint32_t flags, const struct nftnl_expr *e)
{
	int ret, offset = 0;
	struct nftnl_expr_redir *redir = nftnl_expr_data(e);

	if (nftnl_expr_is_set(e, NFTNL_EXPR_REDIR_REG_PROTO_MIN)) {
		ret = snprintf(buf + offset, remain, "proto_min reg %u ",
			       redir->sreg_proto_min);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);
	}

	if (nftnl_expr_is_set(e, NFTNL_EXPR_REDIR_REG_PROTO_MAX)) {
		ret = snprintf(buf + offset, remain, "proto_max reg %u ",
			       redir->sreg_proto_max);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);
	}

	if (nftnl_expr_is_set(e, NFTNL_EXPR_REDIR_FLAGS)) {
		ret = snprintf(buf + offset, remain, "flags 0x%x ",
			       redir->flags);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);
	}

	return offset;
}

static struct attr_policy redir_attr_policy[__NFTNL_EXPR_REDIR_MAX] = {
	[NFTNL_EXPR_REDIR_REG_PROTO_MIN] = { .maxlen = sizeof(uint32_t) },
	[NFTNL_EXPR_REDIR_REG_PROTO_MAX] = { .maxlen = sizeof(uint32_t) },
	[NFTNL_EXPR_REDIR_FLAGS]         = { .maxlen = sizeof(uint32_t) },
};

struct expr_ops expr_ops_redir = {
	.name		= "redir",
	.alloc_len	= sizeof(struct nftnl_expr_redir),
	.nftnl_max_attr	= __NFTNL_EXPR_REDIR_MAX - 1,
	.attr_policy	= redir_attr_policy,
	.set		= nftnl_expr_redir_set,
	.get		= nftnl_expr_redir_get,
	.parse		= nftnl_expr_redir_parse,
	.build		= nftnl_expr_redir_build,
	.output		= nftnl_expr_redir_snprintf,
};
