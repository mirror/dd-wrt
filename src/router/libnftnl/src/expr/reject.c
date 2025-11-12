/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) 2013 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This code has been sponsored by Sophos Astaro <http://www.sophos.com>
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

struct nftnl_expr_reject {
	uint32_t		type;
	uint8_t			icmp_code;
};

static int nftnl_expr_reject_set(struct nftnl_expr *e, uint16_t type,
				    const void *data, uint32_t data_len)
{
	struct nftnl_expr_reject *reject = nftnl_expr_data(e);

	switch(type) {
	case NFTNL_EXPR_REJECT_TYPE:
		memcpy(&reject->type, data, data_len);
		break;
	case NFTNL_EXPR_REJECT_CODE:
		memcpy(&reject->icmp_code, data, data_len);
		break;
	}
	return 0;
}

static const void *
nftnl_expr_reject_get(const struct nftnl_expr *e, uint16_t type,
			 uint32_t *data_len)
{
	struct nftnl_expr_reject *reject = nftnl_expr_data(e);

	switch(type) {
	case NFTNL_EXPR_REJECT_TYPE:
		*data_len = sizeof(reject->type);
		return &reject->type;
	case NFTNL_EXPR_REJECT_CODE:
		*data_len = sizeof(reject->icmp_code);
		return &reject->icmp_code;
	}
	return NULL;
}

static int nftnl_expr_reject_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NFTA_REJECT_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case NFTA_REJECT_TYPE:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			abi_breakage();
		break;
	case NFTA_REJECT_ICMP_CODE:
		if (mnl_attr_validate(attr, MNL_TYPE_U8) < 0)
			abi_breakage();
		break;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static void
nftnl_expr_reject_build(struct nlmsghdr *nlh, const struct nftnl_expr *e)
{
	struct nftnl_expr_reject *reject = nftnl_expr_data(e);

	if (e->flags & (1 << NFTNL_EXPR_REJECT_TYPE))
		mnl_attr_put_u32(nlh, NFTA_REJECT_TYPE, htonl(reject->type));
	if (e->flags & (1 << NFTNL_EXPR_REJECT_CODE))
		mnl_attr_put_u8(nlh, NFTA_REJECT_ICMP_CODE, reject->icmp_code);
}

static int
nftnl_expr_reject_parse(struct nftnl_expr *e, struct nlattr *attr)
{
	struct nftnl_expr_reject *reject = nftnl_expr_data(e);
	struct nlattr *tb[NFTA_REJECT_MAX+1] = {};

	if (mnl_attr_parse_nested(attr, nftnl_expr_reject_cb, tb) < 0)
		return -1;

	if (tb[NFTA_REJECT_TYPE]) {
		reject->type = ntohl(mnl_attr_get_u32(tb[NFTA_REJECT_TYPE]));
		e->flags |= (1 << NFTNL_EXPR_REJECT_TYPE);
	}
	if (tb[NFTA_REJECT_ICMP_CODE]) {
		reject->icmp_code = mnl_attr_get_u8(tb[NFTA_REJECT_ICMP_CODE]);
		e->flags |= (1 << NFTNL_EXPR_REJECT_CODE);
	}

	return 0;
}

static int
nftnl_expr_reject_snprintf(char *buf, size_t len,
			   uint32_t flags, const struct nftnl_expr *e)
{
	struct nftnl_expr_reject *reject = nftnl_expr_data(e);

	return snprintf(buf, len, "type %u code %u ",
			reject->type, reject->icmp_code);
}

static struct attr_policy reject_attr_policy[__NFTNL_EXPR_REJECT_MAX] = {
	[NFTNL_EXPR_REJECT_TYPE] = { .maxlen = sizeof(uint32_t) },
	[NFTNL_EXPR_REJECT_CODE] = { .maxlen = sizeof(uint8_t) },
};

struct expr_ops expr_ops_reject = {
	.name		= "reject",
	.alloc_len	= sizeof(struct nftnl_expr_reject),
	.nftnl_max_attr	= __NFTNL_EXPR_REJECT_MAX - 1,
	.attr_policy	= reject_attr_policy,
	.set		= nftnl_expr_reject_set,
	.get		= nftnl_expr_reject_get,
	.parse		= nftnl_expr_reject_parse,
	.build		= nftnl_expr_reject_build,
	.output		= nftnl_expr_reject_snprintf,
};
