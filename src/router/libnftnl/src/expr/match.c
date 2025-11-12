/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) 2012 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This code has been sponsored by Sophos Astaro <http://www.sophos.com>
 */

#include "internal.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>	/* for memcpy */
#include <arpa/inet.h>
#include <errno.h>
#include <libmnl/libmnl.h>

#include <linux/netfilter/nf_tables.h>
#include <linux/netfilter/nf_tables_compat.h>

#include <libnftnl/expr.h>
#include <libnftnl/rule.h>

/* From include/linux/netfilter/x_tables.h */
#define XT_EXTENSION_MAXNAMELEN 29

struct nftnl_expr_match {
	char		name[XT_EXTENSION_MAXNAMELEN];
	uint32_t	rev;
	uint32_t	data_len;
	const void	*data;
};

static int
nftnl_expr_match_set(struct nftnl_expr *e, uint16_t type,
			 const void *data, uint32_t data_len)
{
	struct nftnl_expr_match *mt = nftnl_expr_data(e);

	switch(type) {
	case NFTNL_EXPR_MT_NAME:
		snprintf(mt->name, sizeof(mt->name), "%.*s", data_len,
			 (const char *)data);
		break;
	case NFTNL_EXPR_MT_REV:
		memcpy(&mt->rev, data, data_len);
		break;
	case NFTNL_EXPR_MT_INFO:
		if (e->flags & (1 << NFTNL_EXPR_MT_INFO))
			xfree(mt->data);

		mt->data = data;
		mt->data_len = data_len;
		break;
	}
	return 0;
}

static const void *
nftnl_expr_match_get(const struct nftnl_expr *e, uint16_t type,
			uint32_t *data_len)
{
	struct nftnl_expr_match *mt = nftnl_expr_data(e);

	switch(type) {
	case NFTNL_EXPR_MT_NAME:
		*data_len = sizeof(mt->name);
		return mt->name;
	case NFTNL_EXPR_MT_REV:
		*data_len = sizeof(mt->rev);
		return &mt->rev;
	case NFTNL_EXPR_MT_INFO:
		*data_len = mt->data_len;
		return mt->data;
	}
	return NULL;
}

static int nftnl_expr_match_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NFTA_MATCH_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case NFTA_MATCH_NAME:
		if (mnl_attr_validate(attr, MNL_TYPE_NUL_STRING) < 0)
			abi_breakage();
		break;
	case NFTA_MATCH_REV:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			abi_breakage();
		break;
	case NFTA_MATCH_INFO:
		if (mnl_attr_validate(attr, MNL_TYPE_BINARY) < 0)
			abi_breakage();
		break;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static void
nftnl_expr_match_build(struct nlmsghdr *nlh, const struct nftnl_expr *e)
{
	struct nftnl_expr_match *mt = nftnl_expr_data(e);

	if (e->flags & (1 << NFTNL_EXPR_MT_NAME))
		mnl_attr_put_strz(nlh, NFTA_MATCH_NAME, mt->name);
	if (e->flags & (1 << NFTNL_EXPR_MT_REV))
		mnl_attr_put_u32(nlh, NFTA_MATCH_REV, htonl(mt->rev));
	if (e->flags & (1 << NFTNL_EXPR_MT_INFO))
		mnl_attr_put(nlh, NFTA_MATCH_INFO, mt->data_len, mt->data);
}

static int nftnl_expr_match_parse(struct nftnl_expr *e, struct nlattr *attr)
{
	struct nftnl_expr_match *match = nftnl_expr_data(e);
	struct nlattr *tb[NFTA_MATCH_MAX+1] = {};

	if (mnl_attr_parse_nested(attr, nftnl_expr_match_cb, tb) < 0)
		return -1;

	if (tb[NFTA_MATCH_NAME]) {
		snprintf(match->name, XT_EXTENSION_MAXNAMELEN, "%s",
			 mnl_attr_get_str(tb[NFTA_MATCH_NAME]));

		match->name[XT_EXTENSION_MAXNAMELEN-1] = '\0';
		e->flags |= (1 << NFTNL_EXPR_MT_NAME);
	}

	if (tb[NFTA_MATCH_REV]) {
		match->rev = ntohl(mnl_attr_get_u32(tb[NFTA_MATCH_REV]));
		e->flags |= (1 << NFTNL_EXPR_MT_REV);
	}

	if (tb[NFTA_MATCH_INFO]) {
		uint32_t len = mnl_attr_get_payload_len(tb[NFTA_MATCH_INFO]);
		void *match_data;

		if (e->flags & (1 << NFTNL_EXPR_MT_INFO))
			xfree(match->data);

		match_data = calloc(1, len);
		if (match_data == NULL)
			return -1;

		memcpy(match_data, mnl_attr_get_payload(tb[NFTA_MATCH_INFO]), len);

		match->data = match_data;
		match->data_len = len;

		e->flags |= (1 << NFTNL_EXPR_MT_INFO);
	}

	return 0;
}

static int
nftnl_expr_match_snprintf(char *buf, size_t len,
			  uint32_t flags, const struct nftnl_expr *e)
{
	struct nftnl_expr_match *match = nftnl_expr_data(e);

	return snprintf(buf, len, "name %s rev %u ", match->name, match->rev);
}

static void nftnl_expr_match_free(const struct nftnl_expr *e)
{
	struct nftnl_expr_match *match = nftnl_expr_data(e);

	xfree(match->data);
}

static struct attr_policy match_attr_policy[__NFTNL_EXPR_MT_MAX] = {
	[NFTNL_EXPR_MT_NAME] = { .maxlen = XT_EXTENSION_MAXNAMELEN },
	[NFTNL_EXPR_MT_REV]  = { .maxlen = sizeof(uint32_t) },
	[NFTNL_EXPR_MT_INFO] = { .maxlen = 0 },
};

struct expr_ops expr_ops_match = {
	.name		= "match",
	.alloc_len	= sizeof(struct nftnl_expr_match),
	.nftnl_max_attr	= __NFTNL_EXPR_MT_MAX - 1,
	.attr_policy	= match_attr_policy,
	.free		= nftnl_expr_match_free,
	.set		= nftnl_expr_match_set,
	.get		= nftnl_expr_match_get,
	.parse		= nftnl_expr_match_parse,
	.build		= nftnl_expr_match_build,
	.output		= nftnl_expr_match_snprintf,
};
