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

#include <libmnl/libmnl.h>
#include <libnftnl/object.h>

#include "internal.h"
#include "obj.h"

static int
nftnl_obj_counter_set(struct nftnl_obj *e, uint16_t type,
			  const void *data, uint32_t data_len)
{
	struct nftnl_obj_counter *ctr = nftnl_obj_data(e);

	switch(type) {
	case NFTNL_OBJ_CTR_BYTES:
		memcpy(&ctr->bytes, data, data_len);
		break;
	case NFTNL_OBJ_CTR_PKTS:
		memcpy(&ctr->pkts, data, data_len);
		break;
	}
	return 0;
}

static const void *
nftnl_obj_counter_get(const struct nftnl_obj *e, uint16_t type,
			  uint32_t *data_len)
{
	struct nftnl_obj_counter *ctr = nftnl_obj_data(e);

	switch(type) {
	case NFTNL_OBJ_CTR_BYTES:
		*data_len = sizeof(ctr->bytes);
		return &ctr->bytes;
	case NFTNL_OBJ_CTR_PKTS:
		*data_len = sizeof(ctr->pkts);
		return &ctr->pkts;
	}
	return NULL;
}

static int nftnl_obj_counter_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NFTA_COUNTER_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case NFTA_COUNTER_BYTES:
	case NFTA_COUNTER_PACKETS:
		if (mnl_attr_validate(attr, MNL_TYPE_U64) < 0)
			abi_breakage();
		break;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static void
nftnl_obj_counter_build(struct nlmsghdr *nlh, const struct nftnl_obj *e)
{
	struct nftnl_obj_counter *ctr = nftnl_obj_data(e);

	if (e->flags & (1 << NFTNL_OBJ_CTR_BYTES))
		mnl_attr_put_u64(nlh, NFTA_COUNTER_BYTES, htobe64(ctr->bytes));
	if (e->flags & (1 << NFTNL_OBJ_CTR_PKTS))
		mnl_attr_put_u64(nlh, NFTA_COUNTER_PACKETS, htobe64(ctr->pkts));
}

static int
nftnl_obj_counter_parse(struct nftnl_obj *e, struct nlattr *attr)
{
	struct nftnl_obj_counter *ctr = nftnl_obj_data(e);
	struct nlattr *tb[NFTA_COUNTER_MAX+1] = {};

	if (mnl_attr_parse_nested(attr, nftnl_obj_counter_cb, tb) < 0)
		return -1;

	if (tb[NFTA_COUNTER_BYTES]) {
		ctr->bytes = be64toh(mnl_attr_get_u64(tb[NFTA_COUNTER_BYTES]));
		e->flags |= (1 << NFTNL_OBJ_CTR_BYTES);
	}
	if (tb[NFTA_COUNTER_PACKETS]) {
		ctr->pkts = be64toh(mnl_attr_get_u64(tb[NFTA_COUNTER_PACKETS]));
		e->flags |= (1 << NFTNL_OBJ_CTR_PKTS);
	}

	return 0;
}

static int nftnl_obj_counter_snprintf(char *buf, size_t len, uint32_t flags,
				      const struct nftnl_obj *e)
{
	struct nftnl_obj_counter *ctr = nftnl_obj_data(e);

	return snprintf(buf, len, "pkts %"PRIu64" bytes %"PRIu64" ",
			ctr->pkts, ctr->bytes);
}

static struct attr_policy obj_ctr_attr_policy[__NFTNL_OBJ_CTR_MAX] = {
	[NFTNL_OBJ_CTR_BYTES]	= { .maxlen = sizeof(uint64_t) },
	[NFTNL_OBJ_CTR_PKTS]	= { .maxlen = sizeof(uint64_t) },
};

struct obj_ops obj_ops_counter = {
	.name		= "counter",
	.type		= NFT_OBJECT_COUNTER,
	.alloc_len	= sizeof(struct nftnl_obj_counter),
	.nftnl_max_attr	= __NFTNL_OBJ_CTR_MAX - 1,
	.attr_policy	= obj_ctr_attr_policy,
	.set		= nftnl_obj_counter_set,
	.get		= nftnl_obj_counter_get,
	.parse		= nftnl_obj_counter_parse,
	.build		= nftnl_obj_counter_build,
	.output		= nftnl_obj_counter_snprintf,
};
