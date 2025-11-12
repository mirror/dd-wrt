/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) 2016 by Pablo Neira Ayuso <pablo@netfilter.org>
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

struct nftnl_expr_last {
	uint64_t	msecs;
	uint32_t	set;
};

static int nftnl_expr_last_set(struct nftnl_expr *e, uint16_t type,
				const void *data, uint32_t data_len)
{
	struct nftnl_expr_last *last = nftnl_expr_data(e);

	switch (type) {
	case NFTNL_EXPR_LAST_MSECS:
		memcpy(&last->msecs, data, data_len);
		break;
	case NFTNL_EXPR_LAST_SET:
		memcpy(&last->set, data, data_len);
		break;
	}
	return 0;
}

static const void *nftnl_expr_last_get(const struct nftnl_expr *e,
					uint16_t type, uint32_t *data_len)
{
	struct nftnl_expr_last *last = nftnl_expr_data(e);

	switch (type) {
	case NFTNL_EXPR_LAST_MSECS:
		*data_len = sizeof(last->msecs);
		return &last->msecs;
	case NFTNL_EXPR_LAST_SET:
		*data_len = sizeof(last->set);
		return &last->set;
	}
	return NULL;
}

static int nftnl_expr_last_cb(const struct nlattr *attr, void *data)
{
	int type = mnl_attr_get_type(attr);
	const struct nlattr **tb = data;

	if (mnl_attr_type_valid(attr, NFTA_LAST_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case NFTA_LAST_MSECS:
		if (mnl_attr_validate(attr, MNL_TYPE_U64) < 0)
			abi_breakage();
		break;
	case NFTA_LAST_SET:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			abi_breakage();
		break;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static void
nftnl_expr_last_build(struct nlmsghdr *nlh, const struct nftnl_expr *e)
{
	struct nftnl_expr_last *last = nftnl_expr_data(e);

	if (e->flags & (1 << NFTNL_EXPR_LAST_MSECS))
		mnl_attr_put_u64(nlh, NFTA_LAST_MSECS, htobe64(last->msecs));
	if (e->flags & (1 << NFTNL_EXPR_LAST_SET))
		mnl_attr_put_u32(nlh, NFTA_LAST_SET, htonl(last->set));
}

static int
nftnl_expr_last_parse(struct nftnl_expr *e, struct nlattr *attr)
{
	struct nftnl_expr_last *last = nftnl_expr_data(e);
	struct nlattr *tb[NFTA_LAST_MAX + 1] = {};

	if (mnl_attr_parse_nested(attr, nftnl_expr_last_cb, tb) < 0)
		return -1;

	if (tb[NFTA_LAST_MSECS]) {
		last->msecs = be64toh(mnl_attr_get_u64(tb[NFTA_LAST_MSECS]));
		e->flags |= (1 << NFTNL_EXPR_LAST_MSECS);
	}
	if (tb[NFTA_LAST_SET]) {
		last->set = ntohl(mnl_attr_get_u32(tb[NFTA_LAST_SET]));
		e->flags |= (1 << NFTNL_EXPR_LAST_SET);
	}

	return 0;
}

static int nftnl_expr_last_snprintf(char *buf, size_t len,
				       uint32_t flags,
				       const struct nftnl_expr *e)
{
	struct nftnl_expr_last *last = nftnl_expr_data(e);

	if (!last->set)
		return snprintf(buf, len, "never ");

	return snprintf(buf, len, "%"PRIu64" ", last->msecs);
}

static struct attr_policy last_attr_policy[__NFTNL_EXPR_LAST_MAX] = {
	[NFTNL_EXPR_LAST_MSECS] = { .maxlen = sizeof(uint64_t) },
	[NFTNL_EXPR_LAST_SET]   = { .maxlen = sizeof(uint32_t) },
};

struct expr_ops expr_ops_last = {
	.name		= "last",
	.alloc_len	= sizeof(struct nftnl_expr_last),
	.nftnl_max_attr	= __NFTNL_EXPR_LAST_MAX - 1,
	.attr_policy	= last_attr_policy,
	.set		= nftnl_expr_last_set,
	.get		= nftnl_expr_last_get,
	.parse		= nftnl_expr_last_parse,
	.build		= nftnl_expr_last_build,
	.output		= nftnl_expr_last_snprintf,
};
