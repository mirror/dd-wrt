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
#include <libnftnl/rule.h>
#include <libnftnl/expr.h>

struct nftnl_expr_lookup {
	enum nft_registers	sreg;
	enum nft_registers	dreg;
	char			*set_name;
	uint32_t		set_id;
	uint32_t		flags;
};

static int
nftnl_expr_lookup_set(struct nftnl_expr *e, uint16_t type,
			  const void *data, uint32_t data_len)
{
	struct nftnl_expr_lookup *lookup = nftnl_expr_data(e);

	switch(type) {
	case NFTNL_EXPR_LOOKUP_SREG:
		memcpy(&lookup->sreg, data, data_len);
		break;
	case NFTNL_EXPR_LOOKUP_DREG:
		memcpy(&lookup->dreg, data, data_len);
		break;
	case NFTNL_EXPR_LOOKUP_SET:
		lookup->set_name = strdup((const char *)data);
		if (!lookup->set_name)
			return -1;
		break;
	case NFTNL_EXPR_LOOKUP_SET_ID:
		memcpy(&lookup->set_id, data, data_len);
		break;
	case NFTNL_EXPR_LOOKUP_FLAGS:
		memcpy(&lookup->flags, data, data_len);
		break;
	}
	return 0;
}

static const void *
nftnl_expr_lookup_get(const struct nftnl_expr *e, uint16_t type,
			 uint32_t *data_len)
{
	struct nftnl_expr_lookup *lookup = nftnl_expr_data(e);

	switch(type) {
	case NFTNL_EXPR_LOOKUP_SREG:
		*data_len = sizeof(lookup->sreg);
		return &lookup->sreg;
	case NFTNL_EXPR_LOOKUP_DREG:
		*data_len = sizeof(lookup->dreg);
		return &lookup->dreg;
	case NFTNL_EXPR_LOOKUP_SET:
		*data_len = strlen(lookup->set_name) + 1;
		return lookup->set_name;
	case NFTNL_EXPR_LOOKUP_SET_ID:
		*data_len = sizeof(lookup->set_id);
		return &lookup->set_id;
	case NFTNL_EXPR_LOOKUP_FLAGS:
		*data_len = sizeof(lookup->flags);
		return &lookup->flags;
	}
	return NULL;
}

static int nftnl_expr_lookup_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NFTA_LOOKUP_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case NFTA_LOOKUP_SREG:
	case NFTA_LOOKUP_DREG:
	case NFTA_LOOKUP_SET_ID:
	case NFTA_LOOKUP_FLAGS:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			abi_breakage();
		break;
	case NFTA_LOOKUP_SET:
		if (mnl_attr_validate(attr, MNL_TYPE_STRING) < 0)
			abi_breakage();
		break;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static void
nftnl_expr_lookup_build(struct nlmsghdr *nlh, const struct nftnl_expr *e)
{
	struct nftnl_expr_lookup *lookup = nftnl_expr_data(e);

	if (e->flags & (1 << NFTNL_EXPR_LOOKUP_SREG))
		mnl_attr_put_u32(nlh, NFTA_LOOKUP_SREG, htonl(lookup->sreg));
	if (e->flags & (1 << NFTNL_EXPR_LOOKUP_DREG))
		mnl_attr_put_u32(nlh, NFTA_LOOKUP_DREG, htonl(lookup->dreg));
	if (e->flags & (1 << NFTNL_EXPR_LOOKUP_SET))
		mnl_attr_put_strz(nlh, NFTA_LOOKUP_SET, lookup->set_name);
	if (e->flags & (1 << NFTNL_EXPR_LOOKUP_SET_ID))
		mnl_attr_put_u32(nlh, NFTA_LOOKUP_SET_ID,
				 htonl(lookup->set_id));
	if (e->flags & (1 << NFTNL_EXPR_LOOKUP_FLAGS))
		mnl_attr_put_u32(nlh, NFTA_LOOKUP_FLAGS, htonl(lookup->flags));
}

static int
nftnl_expr_lookup_parse(struct nftnl_expr *e, struct nlattr *attr)
{
	struct nftnl_expr_lookup *lookup = nftnl_expr_data(e);
	struct nlattr *tb[NFTA_LOOKUP_MAX+1] = {};
	int ret = 0;

	if (mnl_attr_parse_nested(attr, nftnl_expr_lookup_cb, tb) < 0)
		return -1;

	if (tb[NFTA_LOOKUP_SREG]) {
		lookup->sreg = ntohl(mnl_attr_get_u32(tb[NFTA_LOOKUP_SREG]));
		e->flags |= (1 << NFTNL_EXPR_LOOKUP_SREG);
	}
	if (tb[NFTA_LOOKUP_DREG]) {
		lookup->dreg = ntohl(mnl_attr_get_u32(tb[NFTA_LOOKUP_DREG]));
		e->flags |= (1 << NFTNL_EXPR_LOOKUP_DREG);
	}
	if (tb[NFTA_LOOKUP_SET]) {
		lookup->set_name =
			strdup(mnl_attr_get_str(tb[NFTA_LOOKUP_SET]));
		if (!lookup->set_name)
			return -1;
		e->flags |= (1 << NFTNL_EXPR_LOOKUP_SET);
	}
	if (tb[NFTA_LOOKUP_SET_ID]) {
		lookup->set_id =
			ntohl(mnl_attr_get_u32(tb[NFTA_LOOKUP_SET_ID]));
		e->flags |= (1 << NFTNL_EXPR_LOOKUP_SET_ID);
	}
	if (tb[NFTA_LOOKUP_FLAGS]) {
		lookup->flags = ntohl(mnl_attr_get_u32(tb[NFTA_LOOKUP_FLAGS]));
		e->flags |= (1 << NFTNL_EXPR_LOOKUP_FLAGS);
	}

	return ret;
}

static int
nftnl_expr_lookup_snprintf(char *buf, size_t remain,
			   uint32_t flags, const struct nftnl_expr *e)
{
	struct nftnl_expr_lookup *l = nftnl_expr_data(e);
	int offset = 0, ret;

	ret = snprintf(buf, remain, "reg %u set %s ", l->sreg, l->set_name);
	SNPRINTF_BUFFER_SIZE(ret, remain, offset);

	if (e->flags & (1 << NFTNL_EXPR_LOOKUP_DREG)) {
		ret = snprintf(buf + offset, remain, "dreg %u ", l->dreg);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);
	}

	if (e->flags & (1 << NFTNL_EXPR_LOOKUP_FLAGS)) {
		ret = snprintf(buf + offset, remain, "0x%x ", l->flags);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);
	}

	return offset;
}

static void nftnl_expr_lookup_free(const struct nftnl_expr *e)
{
	struct nftnl_expr_lookup *lookup = nftnl_expr_data(e);

	xfree(lookup->set_name);
}

static struct attr_policy lookup_attr_policy[__NFTNL_EXPR_LOOKUP_MAX] = {
	[NFTNL_EXPR_LOOKUP_SREG]   = { .maxlen = sizeof(uint32_t) },
	[NFTNL_EXPR_LOOKUP_DREG]   = { .maxlen = sizeof(uint32_t) },
	[NFTNL_EXPR_LOOKUP_SET]    = { .maxlen = NFT_SET_MAXNAMELEN },
	[NFTNL_EXPR_LOOKUP_SET_ID] = { .maxlen = sizeof(uint32_t) },
	[NFTNL_EXPR_LOOKUP_FLAGS]  = { .maxlen = sizeof(uint32_t) },
};

struct expr_ops expr_ops_lookup = {
	.name		= "lookup",
	.alloc_len	= sizeof(struct nftnl_expr_lookup),
	.nftnl_max_attr	= __NFTNL_EXPR_LOOKUP_MAX - 1,
	.attr_policy	= lookup_attr_policy,
	.free		= nftnl_expr_lookup_free,
	.set		= nftnl_expr_lookup_set,
	.get		= nftnl_expr_lookup_get,
	.parse		= nftnl_expr_lookup_parse,
	.build		= nftnl_expr_lookup_build,
	.output		= nftnl_expr_lookup_snprintf,
};
