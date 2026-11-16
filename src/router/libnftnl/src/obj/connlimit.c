/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) 2025 by Fernando Fernandez Mancera <fmancera@suse.de>
 */

#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>

#include <linux/netfilter/nf_tables.h>

#include <internal.h>
#include <libmnl/libmnl.h>
#include <libnftnl/object.h>

#include "obj.h"

static int nftnl_obj_connlimit_set(struct nftnl_obj *e, uint16_t type,
				   const void *data, uint32_t data_len)
{
	struct nftnl_obj_connlimit *connlimit = nftnl_obj_data(e);

	switch(type) {
	case NFTNL_OBJ_CONNLIMIT_COUNT:
		memcpy(&connlimit->count, data, data_len);
		break;
	case NFTNL_OBJ_CONNLIMIT_FLAGS:
		memcpy(&connlimit->flags, data, data_len);
		break;
	}
	return 0;
}

static const void *nftnl_obj_connlimit_get(const struct nftnl_obj *e,
					   uint16_t type, uint32_t *data_len)
{
	struct nftnl_obj_connlimit *connlimit = nftnl_obj_data(e);

	switch (type) {
	case NFTNL_OBJ_CONNLIMIT_COUNT:
		*data_len = sizeof(connlimit->count);
		return &connlimit->count;
	case NFTNL_OBJ_CONNLIMIT_FLAGS:
		*data_len = sizeof(connlimit->flags);
		return &connlimit->flags;
	}
	return NULL;
}

static int nftnl_obj_connlimit_cb(const struct nlattr *attr, void *data)
{
	int type = mnl_attr_get_type(attr);
	const struct nlattr **tb = data;

	if (mnl_attr_type_valid(attr, NFTA_CONNLIMIT_MAX) < 0)
		return MNL_CB_OK;

	switch (type) {
	case NFTA_CONNLIMIT_COUNT:
	case NFTA_CONNLIMIT_FLAGS:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			abi_breakage();
		break;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static void nftnl_obj_connlimit_build(struct nlmsghdr *nlh,
				      const struct nftnl_obj *e)
{
	struct nftnl_obj_connlimit *connlimit = nftnl_obj_data(e);

	if (e->flags & (1 << NFTNL_OBJ_CONNLIMIT_COUNT))
		mnl_attr_put_u32(nlh, NFTA_CONNLIMIT_COUNT,
				 htonl(connlimit->count));
	if (e->flags & (1 << NFTNL_OBJ_CONNLIMIT_FLAGS))
		mnl_attr_put_u32(nlh, NFTA_CONNLIMIT_FLAGS,
				 htonl(connlimit->flags));
}

static int nftnl_obj_connlimit_parse(struct nftnl_obj *e, struct nlattr *attr)
{
	struct nftnl_obj_connlimit *connlimit = nftnl_obj_data(e);
	struct nlattr *tb[NFTA_CONNLIMIT_MAX + 1] = {};

	if (mnl_attr_parse_nested(attr, nftnl_obj_connlimit_cb, tb) < 0)
		return -1;

	if (tb[NFTA_CONNLIMIT_COUNT]) {
		connlimit->count = ntohl(mnl_attr_get_u32(tb[NFTA_CONNLIMIT_COUNT]));
		e->flags |= (1 << NFTNL_OBJ_CONNLIMIT_COUNT);
	}
	if (tb[NFTA_CONNLIMIT_FLAGS]) {
		connlimit->flags = ntohl(mnl_attr_get_u32(tb[NFTA_CONNLIMIT_FLAGS]));
		e->flags |= (1 << NFTNL_OBJ_CONNLIMIT_FLAGS);
	}

	return 0;
}

static int nftnl_obj_connlimit_snprintf(char *buf, size_t len,
					uint32_t flags,
					const struct nftnl_obj *e)
{
	struct nftnl_obj_connlimit *connlimit = nftnl_obj_data(e);

	return snprintf(buf, len, "count %u flags %x ",
			connlimit->count, connlimit->flags);
}

static struct attr_policy obj_connlimit_attr_policy[__NFTNL_OBJ_CONNLIMIT_MAX] = {
	[NFTNL_OBJ_CONNLIMIT_COUNT]	= { .maxlen = sizeof(uint32_t) },
	[NFTNL_OBJ_CONNLIMIT_FLAGS]	= { .maxlen = sizeof(uint32_t) },
};

struct obj_ops obj_ops_connlimit = {
	.name		= "connlimit",
	.type		= NFT_OBJECT_CONNLIMIT,
	.alloc_len	= sizeof(struct nftnl_obj_connlimit),
	.nftnl_max_attr	= __NFTNL_OBJ_CONNLIMIT_MAX,
	.attr_policy	= obj_connlimit_attr_policy,
	.set		= nftnl_obj_connlimit_set,
	.get		= nftnl_obj_connlimit_get,
	.parse		= nftnl_obj_connlimit_parse,
	.build		= nftnl_obj_connlimit_build,
	.output		= nftnl_obj_connlimit_snprintf,
};
