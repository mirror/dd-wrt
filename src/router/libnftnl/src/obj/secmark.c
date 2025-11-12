/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) 2012-2016 by Pablo Neira Ayuso <pablo@netfilter.org>
 */

#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <errno.h>
#include <inttypes.h>

#include <linux/netfilter/nf_tables.h>

#include "internal.h"
#include <libmnl/libmnl.h>
#include <libnftnl/object.h>

#include "obj.h"

static int nftnl_obj_secmark_set(struct nftnl_obj *e, uint16_t type,
				const void *data, uint32_t data_len)
{
	struct nftnl_obj_secmark *secmark = nftnl_obj_data(e);

	switch (type) {
	case NFTNL_OBJ_SECMARK_CTX:
		snprintf(secmark->ctx, sizeof(secmark->ctx), "%s", (const char *)data);
		break;
	}
	return 0;
}

static const void *nftnl_obj_secmark_get(const struct nftnl_obj *e,
					uint16_t type, uint32_t *data_len)
{
	struct nftnl_obj_secmark *secmark = nftnl_obj_data(e);

	switch (type) {
	case NFTNL_OBJ_SECMARK_CTX:
		*data_len = strlen(secmark->ctx);
		return secmark->ctx;
	}
	return NULL;
}

static int nftnl_obj_secmark_cb(const struct nlattr *attr, void *data)
{
	const struct nftnl_obj_secmark *secmark = NULL;
	int type = mnl_attr_get_type(attr);
	const struct nlattr **tb = data;

	if (mnl_attr_type_valid(attr, NFTA_SECMARK_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case NFTA_SECMARK_CTX:
		if (mnl_attr_validate(attr, MNL_TYPE_STRING) < 0)
			abi_breakage();
		if (mnl_attr_get_payload_len(attr) >= sizeof(secmark->ctx))
			abi_breakage();
		break;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static void
nftnl_obj_secmark_build(struct nlmsghdr *nlh, const struct nftnl_obj *e)
{
	struct nftnl_obj_secmark *secmark = nftnl_obj_data(e);

	if (e->flags & (1 << NFTNL_OBJ_SECMARK_CTX))
		mnl_attr_put_str(nlh, NFTA_SECMARK_CTX, secmark->ctx);
}

static int
nftnl_obj_secmark_parse(struct nftnl_obj *e, struct nlattr *attr)
{
	struct nftnl_obj_secmark *secmark = nftnl_obj_data(e);
	struct nlattr *tb[NFTA_SECMARK_MAX + 1] = {};

	if (mnl_attr_parse_nested(attr, nftnl_obj_secmark_cb, tb) < 0)
		return -1;

	if (tb[NFTA_SECMARK_CTX]) {
		snprintf(secmark->ctx, sizeof(secmark->ctx), "%s",
			 mnl_attr_get_str(tb[NFTA_SECMARK_CTX]));
		e->flags |= (1 << NFTNL_OBJ_SECMARK_CTX);
	}

	return 0;
}

static int nftnl_obj_secmark_snprintf(char *buf, size_t len,
				       uint32_t flags,
				       const struct nftnl_obj *e)
{
	struct nftnl_obj_secmark *secmark = nftnl_obj_data(e);

	return snprintf(buf, len, "context %s ", secmark->ctx);
}

static struct attr_policy obj_secmark_attr_policy[__NFTNL_OBJ_SECMARK_MAX] = {
	[NFTNL_OBJ_SECMARK_CTX]	= { .maxlen = NFT_SECMARK_CTX_MAXLEN },
};

struct obj_ops obj_ops_secmark = {
	.name		= "secmark",
	.type		= NFT_OBJECT_SECMARK,
	.alloc_len	= sizeof(struct nftnl_obj_secmark),
	.nftnl_max_attr	= __NFTNL_OBJ_SECMARK_MAX - 1,
	.attr_policy	= obj_secmark_attr_policy,
	.set		= nftnl_obj_secmark_set,
	.get		= nftnl_obj_secmark_get,
	.parse		= nftnl_obj_secmark_parse,
	.build		= nftnl_obj_secmark_build,
	.output		= nftnl_obj_secmark_snprintf,
};
