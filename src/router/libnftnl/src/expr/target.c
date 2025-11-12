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

struct nftnl_expr_target {
	char		name[XT_EXTENSION_MAXNAMELEN];
	uint32_t	rev;
	uint32_t	data_len;
	const void	*data;
};

static int
nftnl_expr_target_set(struct nftnl_expr *e, uint16_t type,
			 const void *data, uint32_t data_len)
{
	struct nftnl_expr_target *tg = nftnl_expr_data(e);

	switch(type) {
	case NFTNL_EXPR_TG_NAME:
		snprintf(tg->name, sizeof(tg->name), "%.*s", data_len,
			 (const char *) data);
		break;
	case NFTNL_EXPR_TG_REV:
		memcpy(&tg->rev, data, data_len);
		break;
	case NFTNL_EXPR_TG_INFO:
		if (e->flags & (1 << NFTNL_EXPR_TG_INFO))
			xfree(tg->data);

		tg->data = data;
		tg->data_len = data_len;
		break;
	}
	return 0;
}

static const void *
nftnl_expr_target_get(const struct nftnl_expr *e, uint16_t type,
			 uint32_t *data_len)
{
	struct nftnl_expr_target *tg = nftnl_expr_data(e);

	switch(type) {
	case NFTNL_EXPR_TG_NAME:
		*data_len = sizeof(tg->name);
		return tg->name;
	case NFTNL_EXPR_TG_REV:
		*data_len = sizeof(tg->rev);
		return &tg->rev;
	case NFTNL_EXPR_TG_INFO:
		*data_len = tg->data_len;
		return tg->data;
	}
	return NULL;
}

static int nftnl_expr_target_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NFTA_TARGET_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case NFTA_TARGET_NAME:
		if (mnl_attr_validate(attr, MNL_TYPE_NUL_STRING) < 0)
			abi_breakage();
		break;
	case NFTA_TARGET_REV:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			abi_breakage();
		break;
	case NFTA_TARGET_INFO:
		if (mnl_attr_validate(attr, MNL_TYPE_BINARY) < 0)
			abi_breakage();
		break;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static void
nftnl_expr_target_build(struct nlmsghdr *nlh, const struct nftnl_expr *e)
{
	struct nftnl_expr_target *tg = nftnl_expr_data(e);

	if (e->flags & (1 << NFTNL_EXPR_TG_NAME))
		mnl_attr_put_strz(nlh, NFTA_TARGET_NAME, tg->name);
	if (e->flags & (1 << NFTNL_EXPR_TG_REV))
		mnl_attr_put_u32(nlh, NFTA_TARGET_REV, htonl(tg->rev));
	if (e->flags & (1 << NFTNL_EXPR_TG_INFO))
		mnl_attr_put(nlh, NFTA_TARGET_INFO, tg->data_len, tg->data);
}

static int nftnl_expr_target_parse(struct nftnl_expr *e, struct nlattr *attr)
{
	struct nftnl_expr_target *target = nftnl_expr_data(e);
	struct nlattr *tb[NFTA_TARGET_MAX+1] = {};

	if (mnl_attr_parse_nested(attr, nftnl_expr_target_cb, tb) < 0)
		return -1;

	if (tb[NFTA_TARGET_NAME]) {
		snprintf(target->name, XT_EXTENSION_MAXNAMELEN, "%s",
			 mnl_attr_get_str(tb[NFTA_TARGET_NAME]));

		target->name[XT_EXTENSION_MAXNAMELEN-1] = '\0';
		e->flags |= (1 << NFTNL_EXPR_TG_NAME);
	}

	if (tb[NFTA_TARGET_REV]) {
		target->rev = ntohl(mnl_attr_get_u32(tb[NFTA_TARGET_REV]));
		e->flags |= (1 << NFTNL_EXPR_TG_REV);
	}

	if (tb[NFTA_TARGET_INFO]) {
		uint32_t len = mnl_attr_get_payload_len(tb[NFTA_TARGET_INFO]);
		void *target_data;

		if (target->data)
			xfree(target->data);

		target_data = calloc(1, len);
		if (target_data == NULL)
			return -1;

		memcpy(target_data, mnl_attr_get_payload(tb[NFTA_TARGET_INFO]), len);

		target->data = target_data;
		target->data_len = len;

		e->flags |= (1 << NFTNL_EXPR_TG_INFO);
	}

	return 0;
}

static int
nftnl_expr_target_snprintf(char *buf, size_t len,
			   uint32_t flags, const struct nftnl_expr *e)
{
	struct nftnl_expr_target *target = nftnl_expr_data(e);

	return snprintf(buf, len, "name %s rev %u ", target->name, target->rev);
}

static void nftnl_expr_target_free(const struct nftnl_expr *e)
{
	struct nftnl_expr_target *target = nftnl_expr_data(e);

	xfree(target->data);
}

static struct attr_policy target_attr_policy[__NFTNL_EXPR_TG_MAX] = {
	[NFTNL_EXPR_TG_NAME] = { .maxlen = XT_EXTENSION_MAXNAMELEN },
	[NFTNL_EXPR_TG_REV]  = { .maxlen = sizeof(uint32_t) },
	[NFTNL_EXPR_TG_INFO] = { .maxlen = 0 },
};

struct expr_ops expr_ops_target = {
	.name		= "target",
	.alloc_len	= sizeof(struct nftnl_expr_target),
	.nftnl_max_attr	= __NFTNL_EXPR_TG_MAX - 1,
	.attr_policy	= target_attr_policy,
	.free		= nftnl_expr_target_free,
	.set		= nftnl_expr_target_set,
	.get		= nftnl_expr_target_get,
	.parse		= nftnl_expr_target_parse,
	.build		= nftnl_expr_target_build,
	.output		= nftnl_expr_target_snprintf,
};
