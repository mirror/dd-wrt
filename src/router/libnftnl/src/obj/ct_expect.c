/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) 2019 by Stéphane Veyret <sveyret@gmail.com>
 */

#include <arpa/inet.h>
#include <errno.h>

#include <libmnl/libmnl.h>

#include "obj.h"

static int nftnl_obj_ct_expect_set(struct nftnl_obj *e, uint16_t type,
				   const void *data, uint32_t data_len)
{
	struct nftnl_obj_ct_expect *exp = nftnl_obj_data(e);

	switch (type) {
	case NFTNL_OBJ_CT_EXPECT_L3PROTO:
		memcpy(&exp->l3proto, data, data_len);
		break;
	case NFTNL_OBJ_CT_EXPECT_L4PROTO:
		memcpy(&exp->l4proto, data, data_len);
		break;
	case NFTNL_OBJ_CT_EXPECT_DPORT:
		memcpy(&exp->dport, data, data_len);
		break;
	case NFTNL_OBJ_CT_EXPECT_TIMEOUT:
		memcpy(&exp->timeout, data, data_len);
		break;
	case NFTNL_OBJ_CT_EXPECT_SIZE:
		memcpy(&exp->size, data, data_len);
		break;
	}
	return 0;
}

static const void *nftnl_obj_ct_expect_get(const struct nftnl_obj *e,
					   uint16_t type, uint32_t *data_len)
{
	struct nftnl_obj_ct_expect *exp = nftnl_obj_data(e);

	switch (type) {
	case NFTNL_OBJ_CT_EXPECT_L3PROTO:
		*data_len = sizeof(exp->l3proto);
		return &exp->l3proto;
	case NFTNL_OBJ_CT_EXPECT_L4PROTO:
		*data_len = sizeof(exp->l4proto);
		return &exp->l4proto;
	case NFTNL_OBJ_CT_EXPECT_DPORT:
		*data_len = sizeof(exp->dport);
		return &exp->dport;
	case NFTNL_OBJ_CT_EXPECT_TIMEOUT:
		*data_len = sizeof(exp->timeout);
		return &exp->timeout;
	case NFTNL_OBJ_CT_EXPECT_SIZE:
		*data_len = sizeof(exp->size);
		return &exp->size;
	}
	return NULL;
}

static int nftnl_obj_ct_expect_cb(const struct nlattr *attr, void *data)
{
	int type = mnl_attr_get_type(attr);
	const struct nlattr **tb = data;

	if (mnl_attr_type_valid(attr, NFTA_CT_EXPECT_MAX) < 0)
		return MNL_CB_OK;

	switch (type) {
	case NFTA_CT_EXPECT_L3PROTO:
		if (mnl_attr_validate(attr, MNL_TYPE_U16) < 0)
			abi_breakage();
		break;
	case NFTA_CT_EXPECT_L4PROTO:
		if (mnl_attr_validate(attr, MNL_TYPE_U8) < 0)
			abi_breakage();
		break;
	case NFTA_CT_EXPECT_DPORT:
		if (mnl_attr_validate(attr, MNL_TYPE_U16) < 0)
			abi_breakage();
		break;
	case NFTA_CT_EXPECT_TIMEOUT:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			abi_breakage();
		break;
	case NFTA_CT_EXPECT_SIZE:
		if (mnl_attr_validate(attr, MNL_TYPE_U8) < 0)
			abi_breakage();
		break;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static void
nftnl_obj_ct_expect_build(struct nlmsghdr *nlh, const struct nftnl_obj *e)
{
	struct nftnl_obj_ct_expect *exp = nftnl_obj_data(e);

	if (e->flags & (1 << NFTNL_OBJ_CT_EXPECT_L3PROTO))
		mnl_attr_put_u16(nlh, NFTA_CT_EXPECT_L3PROTO, htons(exp->l3proto));
	if (e->flags & (1 << NFTNL_OBJ_CT_EXPECT_L4PROTO))
		mnl_attr_put_u8(nlh, NFTA_CT_EXPECT_L4PROTO, exp->l4proto);
	if (e->flags & (1 << NFTNL_OBJ_CT_EXPECT_DPORT))
		mnl_attr_put_u16(nlh, NFTA_CT_EXPECT_DPORT, htons(exp->dport));
	if (e->flags & (1 << NFTNL_OBJ_CT_EXPECT_TIMEOUT))
		mnl_attr_put_u32(nlh, NFTA_CT_EXPECT_TIMEOUT, exp->timeout);
	if (e->flags & (1 << NFTNL_OBJ_CT_EXPECT_SIZE))
		mnl_attr_put_u8(nlh, NFTA_CT_EXPECT_SIZE, exp->size);
}

static int
nftnl_obj_ct_expect_parse(struct nftnl_obj *e, struct nlattr *attr)
{
	struct nftnl_obj_ct_expect *exp = nftnl_obj_data(e);
	struct nlattr *tb[NFTA_CT_EXPECT_MAX + 1] = {};

	if (mnl_attr_parse_nested(attr, nftnl_obj_ct_expect_cb, tb) < 0)
		return -1;

	if (tb[NFTA_CT_EXPECT_L3PROTO]) {
		exp->l3proto = ntohs(mnl_attr_get_u16(tb[NFTA_CT_EXPECT_L3PROTO]));
		e->flags |= (1 << NFTNL_OBJ_CT_EXPECT_L3PROTO);
	}
	if (tb[NFTA_CT_EXPECT_L4PROTO]) {
		exp->l4proto = mnl_attr_get_u8(tb[NFTA_CT_EXPECT_L4PROTO]);
		e->flags |= (1 << NFTNL_OBJ_CT_EXPECT_L4PROTO);
	}
	if (tb[NFTA_CT_EXPECT_DPORT]) {
		exp->dport = ntohs(mnl_attr_get_u16(tb[NFTA_CT_EXPECT_DPORT]));
		e->flags |= (1 << NFTNL_OBJ_CT_EXPECT_DPORT);
	}
	if (tb[NFTA_CT_EXPECT_TIMEOUT]) {
		exp->timeout = mnl_attr_get_u32(tb[NFTA_CT_EXPECT_TIMEOUT]);
		e->flags |= (1 << NFTNL_OBJ_CT_EXPECT_TIMEOUT);
	}
	if (tb[NFTA_CT_EXPECT_SIZE]) {
		exp->size = mnl_attr_get_u8(tb[NFTA_CT_EXPECT_SIZE]);
		e->flags |= (1 << NFTNL_OBJ_CT_EXPECT_SIZE);
	}

	return 0;
}

static int nftnl_obj_ct_expect_snprintf(char *buf, size_t remain,
					uint32_t flags,
					const struct nftnl_obj *e)
{
	struct nftnl_obj_ct_expect *exp = nftnl_obj_data(e);
	int ret = 0, offset = 0;

	if (e->flags & (1 << NFTNL_OBJ_CT_EXPECT_L3PROTO)) {
		ret = snprintf(buf + offset, remain,
			       "family %d ", exp->l3proto);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);
	}
	if (e->flags & (1 << NFTNL_OBJ_CT_EXPECT_L4PROTO)) {
		ret = snprintf(buf + offset, remain,
			       "protocol %d ", exp->l4proto);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);
	}
	if (e->flags & (1 << NFTNL_OBJ_CT_EXPECT_DPORT)) {
		ret = snprintf(buf + offset, remain,
			       "dport %d ", exp->dport);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);
	}
	if (e->flags & (1 << NFTNL_OBJ_CT_EXPECT_TIMEOUT)) {
		ret = snprintf(buf + offset, remain,
			       "timeout %d ", exp->timeout);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);
	}
	if (e->flags & (1 << NFTNL_OBJ_CT_EXPECT_SIZE)) {
		ret = snprintf(buf + offset, remain, "size %d ", exp->size);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);
	}

	buf[offset] = '\0';
	return offset;
}

static struct attr_policy
obj_ct_expect_attr_policy[__NFTNL_OBJ_CT_EXPECT_MAX] = {
	[NFTNL_OBJ_CT_EXPECT_L3PROTO]	= { .maxlen = sizeof(uint16_t) },
	[NFTNL_OBJ_CT_EXPECT_L4PROTO]	= { .maxlen = sizeof(uint8_t) },
	[NFTNL_OBJ_CT_EXPECT_DPORT]	= { .maxlen = sizeof(uint16_t) },
	[NFTNL_OBJ_CT_EXPECT_TIMEOUT]	= { .maxlen = sizeof(uint32_t) },
	[NFTNL_OBJ_CT_EXPECT_SIZE]	= { .maxlen = sizeof(uint8_t) },
};

struct obj_ops obj_ops_ct_expect = {
	.name		= "ct_expect",
	.type		= NFT_OBJECT_CT_EXPECT,
	.alloc_len	= sizeof(struct nftnl_obj_ct_expect),
	.nftnl_max_attr	= __NFTNL_OBJ_CT_EXPECT_MAX - 1,
	.attr_policy	= obj_ct_expect_attr_policy,
	.set		= nftnl_obj_ct_expect_set,
	.get		= nftnl_obj_ct_expect_get,
	.parse		= nftnl_obj_ct_expect_parse,
	.build		= nftnl_obj_ct_expect_build,
	.output		= nftnl_obj_ct_expect_snprintf,
};
