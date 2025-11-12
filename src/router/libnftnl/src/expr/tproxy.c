/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2018 Máté Eckl <ecklm94@gmail.com>
 */

#include "internal.h"

#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <libmnl/libmnl.h>
#include <linux/netfilter/nf_tables.h>
#include <libnftnl/expr.h>
#include <libnftnl/rule.h>

struct nftnl_expr_tproxy {
	enum nft_registers sreg_addr;
	enum nft_registers sreg_port;
	int                family;
};

static int
nftnl_expr_tproxy_set(struct nftnl_expr *e, uint16_t type,
		      const void *data, uint32_t data_len)
{
	struct nftnl_expr_tproxy *tproxy = nftnl_expr_data(e);

	switch(type) {
	case NFTNL_EXPR_TPROXY_FAMILY:
		memcpy(&tproxy->family, data, data_len);
		break;
	case NFTNL_EXPR_TPROXY_REG_ADDR:
		memcpy(&tproxy->sreg_addr, data, data_len);
		break;
	case NFTNL_EXPR_TPROXY_REG_PORT:
		memcpy(&tproxy->sreg_port, data, data_len);
		break;
	}

	return 0;
}

static const void *
nftnl_expr_tproxy_get(const struct nftnl_expr *e, uint16_t type,
		      uint32_t *data_len)
{
	struct nftnl_expr_tproxy *tproxy = nftnl_expr_data(e);

	switch(type) {
	case NFTNL_EXPR_TPROXY_FAMILY:
		*data_len = sizeof(tproxy->family);
		return &tproxy->family;
	case NFTNL_EXPR_TPROXY_REG_ADDR:
		*data_len = sizeof(tproxy->sreg_addr);
		return &tproxy->sreg_addr;
	case NFTNL_EXPR_TPROXY_REG_PORT:
		*data_len = sizeof(tproxy->sreg_port);
		return &tproxy->sreg_port;
	}
	return NULL;
}

static int nftnl_expr_tproxy_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NFTA_TPROXY_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case NFTA_TPROXY_FAMILY:
	case NFTA_TPROXY_REG_ADDR:
	case NFTA_TPROXY_REG_PORT:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			abi_breakage();
		break;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static int
nftnl_expr_tproxy_parse(struct nftnl_expr *e, struct nlattr *attr)
{
	struct nftnl_expr_tproxy *tproxy = nftnl_expr_data(e);
	struct nlattr *tb[NFTA_TPROXY_MAX + 1] = {};

	if (mnl_attr_parse_nested(attr, nftnl_expr_tproxy_cb, tb) < 0)
		return -1;

	if (tb[NFTA_TPROXY_FAMILY]) {
		tproxy->family = ntohl(mnl_attr_get_u32(tb[NFTA_TPROXY_FAMILY]));
		e->flags |= (1 << NFTNL_EXPR_TPROXY_FAMILY);
	}
	if (tb[NFTA_TPROXY_REG_ADDR]) {
		tproxy->sreg_addr =
			ntohl(mnl_attr_get_u32(tb[NFTA_TPROXY_REG_ADDR]));
		e->flags |= (1 << NFTNL_EXPR_TPROXY_REG_ADDR);
	}
	if (tb[NFTA_TPROXY_REG_PORT]) {
		tproxy->sreg_port =
			ntohl(mnl_attr_get_u32(tb[NFTA_TPROXY_REG_PORT]));
		e->flags |= (1 << NFTNL_EXPR_TPROXY_REG_PORT);
	}

	return 0;
}

static void
nftnl_expr_tproxy_build(struct nlmsghdr *nlh, const struct nftnl_expr *e)
{
	struct nftnl_expr_tproxy *tproxy = nftnl_expr_data(e);

	if (e->flags & (1 << NFTNL_EXPR_TPROXY_FAMILY))
		mnl_attr_put_u32(nlh, NFTA_TPROXY_FAMILY, htonl(tproxy->family));

	if (e->flags & (1 << NFTNL_EXPR_TPROXY_REG_ADDR))
		mnl_attr_put_u32(nlh, NFTA_TPROXY_REG_ADDR,
				 htonl(tproxy->sreg_addr));

	if (e->flags & (1 << NFTNL_EXPR_TPROXY_REG_PORT))
		mnl_attr_put_u32(nlh, NFTA_TPROXY_REG_PORT,
				 htonl(tproxy->sreg_port));
}

static int
nftnl_expr_tproxy_snprintf(char *buf, size_t remain,
			uint32_t flags, const struct nftnl_expr *e)
{
	struct nftnl_expr_tproxy *tproxy = nftnl_expr_data(e);
	int offset = 0, ret = 0;

	if (tproxy->family != NFTA_TPROXY_UNSPEC) {
		ret = snprintf(buf + offset, remain, "%s ",
			       nftnl_family2str(tproxy->family));
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);
	}

	if (e->flags & (1 << NFTNL_EXPR_TPROXY_REG_ADDR)) {
		ret = snprintf(buf + offset, remain,
			       "addr reg %u ", tproxy->sreg_addr);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);
	}

	if (e->flags & (1 << NFTNL_EXPR_TPROXY_REG_PORT)) {
		ret = snprintf(buf + offset, remain,
			       "port reg %u ", tproxy->sreg_port);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);
	}

	return offset;
}

static struct attr_policy tproxy_attr_policy[__NFTNL_EXPR_TPROXY_MAX] = {
	[NFTNL_EXPR_TPROXY_FAMILY]   = { .maxlen = sizeof(uint32_t) },
	[NFTNL_EXPR_TPROXY_REG_ADDR] = { .maxlen = sizeof(uint32_t) },
	[NFTNL_EXPR_TPROXY_REG_PORT] = { .maxlen = sizeof(uint32_t) },
};

struct expr_ops expr_ops_tproxy = {
	.name		= "tproxy",
	.alloc_len	= sizeof(struct nftnl_expr_tproxy),
	.nftnl_max_attr	= __NFTNL_EXPR_TPROXY_MAX - 1,
	.attr_policy	= tproxy_attr_policy,
	.set		= nftnl_expr_tproxy_set,
	.get		= nftnl_expr_tproxy_get,
	.parse		= nftnl_expr_tproxy_parse,
	.build		= nftnl_expr_tproxy_build,
	.output		= nftnl_expr_tproxy_snprintf,
};
