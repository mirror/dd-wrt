/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) 2018 by Pablo Neira Ayuso <pablo@netfilter.org>
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

struct nftnl_expr_tunnel {
	enum nft_tunnel_keys	key;
	enum nft_registers	dreg;
};

static int nftnl_expr_tunnel_set(struct nftnl_expr *e, uint16_t type,
				 const void *data, uint32_t data_len)
{
	struct nftnl_expr_tunnel *tunnel = nftnl_expr_data(e);

	switch(type) {
	case NFTNL_EXPR_TUNNEL_KEY:
		memcpy(&tunnel->key, data, data_len);
		break;
	case NFTNL_EXPR_TUNNEL_DREG:
		memcpy(&tunnel->dreg, data, data_len);
		break;
	}
	return 0;
}

static const void *
nftnl_expr_tunnel_get(const struct nftnl_expr *e, uint16_t type,
		       uint32_t *data_len)
{
	struct nftnl_expr_tunnel *tunnel = nftnl_expr_data(e);

	switch(type) {
	case NFTNL_EXPR_TUNNEL_KEY:
		*data_len = sizeof(tunnel->key);
		return &tunnel->key;
	case NFTNL_EXPR_TUNNEL_DREG:
		*data_len = sizeof(tunnel->dreg);
		return &tunnel->dreg;
	}
	return NULL;
}

static int nftnl_expr_tunnel_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NFTA_TUNNEL_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case NFTA_TUNNEL_KEY:
	case NFTA_TUNNEL_DREG:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			abi_breakage();
		break;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static void
nftnl_expr_tunnel_build(struct nlmsghdr *nlh, const struct nftnl_expr *e)
{
	struct nftnl_expr_tunnel *tunnel = nftnl_expr_data(e);

	if (e->flags & (1 << NFTNL_EXPR_TUNNEL_KEY))
		mnl_attr_put_u32(nlh, NFTA_TUNNEL_KEY, htonl(tunnel->key));
	if (e->flags & (1 << NFTNL_EXPR_TUNNEL_DREG))
		mnl_attr_put_u32(nlh, NFTA_TUNNEL_DREG, htonl(tunnel->dreg));
}

static int
nftnl_expr_tunnel_parse(struct nftnl_expr *e, struct nlattr *attr)
{
	struct nftnl_expr_tunnel *tunnel = nftnl_expr_data(e);
	struct nlattr *tb[NFTA_TUNNEL_MAX + 1] = {};

	if (mnl_attr_parse_nested(attr, nftnl_expr_tunnel_cb, tb) < 0)
		return -1;

	if (tb[NFTA_TUNNEL_KEY]) {
		tunnel->key = ntohl(mnl_attr_get_u32(tb[NFTA_TUNNEL_KEY]));
		e->flags |= (1 << NFTNL_EXPR_TUNNEL_KEY);
	}
	if (tb[NFTA_TUNNEL_DREG]) {
		tunnel->dreg = ntohl(mnl_attr_get_u32(tb[NFTA_TUNNEL_DREG]));
		e->flags |= (1 << NFTNL_EXPR_TUNNEL_DREG);
	}

	return 0;
}

static const char *tunnel_key2str_array[NFT_TUNNEL_MAX + 1] = {
	[NFT_TUNNEL_PATH]	= "path",
	[NFT_TUNNEL_ID]		= "id",
};

static const char *tunnel_key2str(uint8_t key)
{
	if (key <= NFT_TUNNEL_MAX)
		return tunnel_key2str_array[key];

	return "unknown";
}

static int
nftnl_expr_tunnel_snprintf(char *buf, size_t len,
			 uint32_t flags, const struct nftnl_expr *e)
{
	struct nftnl_expr_tunnel *tunnel = nftnl_expr_data(e);

	if (e->flags & (1 << NFTNL_EXPR_TUNNEL_DREG)) {
		return snprintf(buf, len, "load %s => reg %u ",
				tunnel_key2str(tunnel->key), tunnel->dreg);
	}
	return 0;
}

static struct attr_policy tunnel_attr_policy[__NFTNL_EXPR_TUNNEL_MAX] = {
	[NFTNL_EXPR_TUNNEL_KEY]  = { .maxlen = sizeof(uint32_t) },
	[NFTNL_EXPR_TUNNEL_DREG] = { .maxlen = sizeof(uint32_t) },
};

struct expr_ops expr_ops_tunnel = {
	.name		= "tunnel",
	.alloc_len	= sizeof(struct nftnl_expr_tunnel),
	.nftnl_max_attr	= __NFTNL_EXPR_TUNNEL_MAX - 1,
	.attr_policy	= tunnel_attr_policy,
	.set		= nftnl_expr_tunnel_set,
	.get		= nftnl_expr_tunnel_get,
	.parse		= nftnl_expr_tunnel_parse,
	.build		= nftnl_expr_tunnel_build,
	.output		= nftnl_expr_tunnel_snprintf,
};
