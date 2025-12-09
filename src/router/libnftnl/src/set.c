/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) 2012-2013 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This code has been sponsored by Sophos Astaro <http://www.sophos.com>
 */
#include "internal.h"

#include <time.h>
#include <endian.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <limits.h>
#include <errno.h>

#include <libmnl/libmnl.h>
#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nf_tables.h>

#include <libnftnl/set.h>
#include <libnftnl/expr.h>

EXPORT_SYMBOL(nftnl_set_alloc);
struct nftnl_set *nftnl_set_alloc(void)
{
	struct nftnl_set *s;

	s = calloc(1, sizeof(struct nftnl_set));
	if (s == NULL)
		return NULL;

	INIT_LIST_HEAD(&s->element_list);
	INIT_LIST_HEAD(&s->expr_list);
	return s;
}

EXPORT_SYMBOL(nftnl_set_free);
void nftnl_set_free(const struct nftnl_set *s)
{
	struct nftnl_set_elem *elem, *tmp;
	struct nftnl_expr *expr, *next;

	if (s->flags & (1 << NFTNL_SET_TABLE))
		xfree(s->table);
	if (s->flags & (1 << NFTNL_SET_NAME))
		xfree(s->name);
	if (s->flags & (1 << NFTNL_SET_USERDATA))
		xfree(s->user.data);

	list_for_each_entry_safe(expr, next, &s->expr_list, head) {
		list_del(&expr->head);
		nftnl_expr_free(expr);
	}

	list_for_each_entry_safe(elem, tmp, &s->element_list, head) {
		list_del(&elem->head);
		nftnl_set_elem_free(elem);
	}

	xfree(s->type);
	xfree(s);
}

EXPORT_SYMBOL(nftnl_set_is_set);
bool nftnl_set_is_set(const struct nftnl_set *s, uint16_t attr)
{
	return s->flags & (1 << attr);
}

EXPORT_SYMBOL(nftnl_set_unset);
void nftnl_set_unset(struct nftnl_set *s, uint16_t attr)
{
	struct nftnl_expr *expr, *tmp;

	if (!(s->flags & (1 << attr)))
		return;

	switch (attr) {
	case NFTNL_SET_TABLE:
		xfree(s->table);
		break;
	case NFTNL_SET_NAME:
		xfree(s->name);
		break;
	case NFTNL_SET_HANDLE:
	case NFTNL_SET_FLAGS:
	case NFTNL_SET_KEY_TYPE:
	case NFTNL_SET_KEY_LEN:
	case NFTNL_SET_DATA_TYPE:
	case NFTNL_SET_DATA_LEN:
	case NFTNL_SET_OBJ_TYPE:
	case NFTNL_SET_FAMILY:
	case NFTNL_SET_ID:
	case NFTNL_SET_POLICY:
	case NFTNL_SET_DESC_SIZE:
	case NFTNL_SET_DESC_CONCAT:
	case NFTNL_SET_TIMEOUT:
	case NFTNL_SET_GC_INTERVAL:
	case NFTNL_SET_COUNT:
		break;
	case NFTNL_SET_USERDATA:
		xfree(s->user.data);
		break;
	case NFTNL_SET_EXPR:
	case NFTNL_SET_EXPRESSIONS:
		list_for_each_entry_safe(expr, tmp, &s->expr_list, head) {
			list_del(&expr->head);
			nftnl_expr_free(expr);
		}
		break;
	default:
		return;
	}

	s->flags &= ~(1 << attr);
}

static uint32_t nftnl_set_validate[NFTNL_SET_MAX + 1] = {
	[NFTNL_SET_HANDLE]		= sizeof(uint64_t),
	[NFTNL_SET_FLAGS]		= sizeof(uint32_t),
	[NFTNL_SET_KEY_TYPE]		= sizeof(uint32_t),
	[NFTNL_SET_KEY_LEN]		= sizeof(uint32_t),
	[NFTNL_SET_DATA_TYPE]	= sizeof(uint32_t),
	[NFTNL_SET_DATA_LEN]		= sizeof(uint32_t),
	[NFTNL_SET_OBJ_TYPE]		= sizeof(uint32_t),
	[NFTNL_SET_FAMILY]		= sizeof(uint32_t),
	[NFTNL_SET_ID]			= sizeof(uint32_t),
	[NFTNL_SET_POLICY]		= sizeof(uint32_t),
	[NFTNL_SET_DESC_SIZE]	= sizeof(uint32_t),
	[NFTNL_SET_TIMEOUT]		= sizeof(uint64_t),
	[NFTNL_SET_GC_INTERVAL]	= sizeof(uint32_t),
	[NFTNL_SET_COUNT]	= sizeof(uint32_t),
};

EXPORT_SYMBOL(nftnl_set_set_data);
int nftnl_set_set_data(struct nftnl_set *s, uint16_t attr, const void *data,
		       uint32_t data_len)
{
	struct nftnl_expr *expr, *tmp;

	nftnl_assert_attr_exists(attr, NFTNL_SET_MAX);
	nftnl_assert_validate(data, nftnl_set_validate, attr, data_len);

	switch(attr) {
	case NFTNL_SET_TABLE:
		return nftnl_set_str_attr(&s->table, &s->flags,
					  attr, data, data_len);
	case NFTNL_SET_NAME:
		return nftnl_set_str_attr(&s->name, &s->flags,
					  attr, data, data_len);
	case NFTNL_SET_HANDLE:
		memcpy(&s->handle, data, sizeof(s->handle));
		break;
	case NFTNL_SET_FLAGS:
		memcpy(&s->set_flags, data, sizeof(s->set_flags));
		break;
	case NFTNL_SET_KEY_TYPE:
		memcpy(&s->key_type, data, sizeof(s->key_type));
		break;
	case NFTNL_SET_KEY_LEN:
		memcpy(&s->key_len, data, sizeof(s->key_len));
		break;
	case NFTNL_SET_DATA_TYPE:
		memcpy(&s->data_type, data, sizeof(s->data_type));
		break;
	case NFTNL_SET_DATA_LEN:
		memcpy(&s->data_len, data, sizeof(s->data_len));
		break;
	case NFTNL_SET_OBJ_TYPE:
		memcpy(&s->obj_type, data, sizeof(s->obj_type));
		break;
	case NFTNL_SET_FAMILY:
		memcpy(&s->family, data, sizeof(s->family));
		break;
	case NFTNL_SET_ID:
		memcpy(&s->id, data, sizeof(s->id));
		break;
	case NFTNL_SET_POLICY:
		memcpy(&s->policy, data, sizeof(s->policy));
		break;
	case NFTNL_SET_DESC_SIZE:
		memcpy(&s->desc.size, data, sizeof(s->desc.size));
		break;
	case NFTNL_SET_DESC_CONCAT:
		if (data_len > sizeof(s->desc.field_len))
			return -1;

		memcpy(&s->desc.field_len, data, data_len);
		for (s->desc.field_count = 0;
		     s->desc.field_count < NFT_REG32_COUNT;
		     s->desc.field_count++) {
			if (!s->desc.field_len[s->desc.field_count])
				break;
		}
		break;
	case NFTNL_SET_TIMEOUT:
		memcpy(&s->timeout, data, sizeof(s->timeout));
		break;
	case NFTNL_SET_GC_INTERVAL:
		memcpy(&s->gc_interval, data, sizeof(s->gc_interval));
		break;
	case NFTNL_SET_COUNT:
		memcpy(&s->elemcount, data, sizeof(s->elemcount));
		break;
	case NFTNL_SET_USERDATA:
		if (s->flags & (1 << NFTNL_SET_USERDATA))
			xfree(s->user.data);

		s->user.data = malloc(data_len);
		if (!s->user.data)
			return -1;
		memcpy(s->user.data, data, data_len);
		s->user.len = data_len;
		break;
	case NFTNL_SET_EXPR:
		list_for_each_entry_safe(expr, tmp, &s->expr_list, head) {
			list_del(&expr->head);
			nftnl_expr_free(expr);
		}

		expr = (void *)data;
		list_add(&expr->head, &s->expr_list);
		break;
	}
	s->flags |= (1 << attr);
	return 0;
}

int nftnl_set_set(struct nftnl_set *s, uint16_t attr, const void *data) __visible;
int nftnl_set_set(struct nftnl_set *s, uint16_t attr, const void *data)
{
	return nftnl_set_set_data(s, attr, data, nftnl_set_validate[attr]);
}

EXPORT_SYMBOL(nftnl_set_set_u32);
void nftnl_set_set_u32(struct nftnl_set *s, uint16_t attr, uint32_t val)
{
	nftnl_set_set_data(s, attr, &val, sizeof(uint32_t));
}

EXPORT_SYMBOL(nftnl_set_set_u64);
void nftnl_set_set_u64(struct nftnl_set *s, uint16_t attr, uint64_t val)
{
	nftnl_set_set_data(s, attr, &val, sizeof(uint64_t));
}

EXPORT_SYMBOL(nftnl_set_set_str);
int nftnl_set_set_str(struct nftnl_set *s, uint16_t attr, const char *str)
{
	return nftnl_set_set_data(s, attr, str, strlen(str) + 1);
}

EXPORT_SYMBOL(nftnl_set_get_data);
const void *nftnl_set_get_data(const struct nftnl_set *s, uint16_t attr,
			       uint32_t *data_len)
{
	struct nftnl_expr *expr;

	if (!(s->flags & (1 << attr)))
		return NULL;

	switch(attr) {
	case NFTNL_SET_TABLE:
		*data_len = strlen(s->table) + 1;
		return s->table;
	case NFTNL_SET_NAME:
		*data_len = strlen(s->name) + 1;
		return s->name;
	case NFTNL_SET_HANDLE:
		*data_len = sizeof(uint64_t);
		return &s->handle;
	case NFTNL_SET_FLAGS:
		*data_len = sizeof(uint32_t);
		return &s->set_flags;
	case NFTNL_SET_KEY_TYPE:
		*data_len = sizeof(uint32_t);
		return &s->key_type;
	case NFTNL_SET_KEY_LEN:
		*data_len = sizeof(uint32_t);
		return &s->key_len;
	case NFTNL_SET_DATA_TYPE:
		*data_len = sizeof(uint32_t);
		return &s->data_type;
	case NFTNL_SET_DATA_LEN:
		*data_len = sizeof(uint32_t);
		return &s->data_len;
	case NFTNL_SET_OBJ_TYPE:
		*data_len = sizeof(uint32_t);
		return &s->obj_type;
	case NFTNL_SET_FAMILY:
		*data_len = sizeof(uint32_t);
		return &s->family;
	case NFTNL_SET_ID:
		*data_len = sizeof(uint32_t);
		return &s->id;
	case NFTNL_SET_POLICY:
		*data_len = sizeof(uint32_t);
		return &s->policy;
	case NFTNL_SET_DESC_SIZE:
		*data_len = sizeof(uint32_t);
		return &s->desc.size;
	case NFTNL_SET_DESC_CONCAT:
		*data_len = s->desc.field_count;
		return s->desc.field_len;
	case NFTNL_SET_TIMEOUT:
		*data_len = sizeof(uint64_t);
		return &s->timeout;
	case NFTNL_SET_GC_INTERVAL:
		*data_len = sizeof(uint32_t);
		return &s->gc_interval;
	case NFTNL_SET_COUNT:
		*data_len = sizeof(uint32_t);
		return &s->elemcount;
	case NFTNL_SET_USERDATA:
		*data_len = s->user.len;
		return s->user.data;
	case NFTNL_SET_EXPR:
		list_for_each_entry(expr, &s->expr_list, head)
			break;
		return expr;
	}
	return NULL;
}

EXPORT_SYMBOL(nftnl_set_get);
const void *nftnl_set_get(const struct nftnl_set *s, uint16_t attr)
{
	uint32_t data_len;
	return nftnl_set_get_data(s, attr, &data_len);
}

EXPORT_SYMBOL(nftnl_set_get_str);
const char *nftnl_set_get_str(const struct nftnl_set *s, uint16_t attr)
{
	return nftnl_set_get(s, attr);
}

EXPORT_SYMBOL(nftnl_set_get_u32);
uint32_t nftnl_set_get_u32(const struct nftnl_set *s, uint16_t attr)
{
	uint32_t data_len;
	const uint32_t *val = nftnl_set_get_data(s, attr, &data_len);

	nftnl_assert(val, attr, data_len == sizeof(uint32_t));

	return val ? *val : 0;
}

EXPORT_SYMBOL(nftnl_set_get_u64);
uint64_t nftnl_set_get_u64(const struct nftnl_set *s, uint16_t attr)
{
	uint32_t data_len;
	const uint64_t *val = nftnl_set_get_data(s, attr, &data_len);

	nftnl_assert(val, attr, data_len == sizeof(uint64_t));

	return val ? *val : 0;
}

struct nftnl_set *nftnl_set_clone(const struct nftnl_set *set)
{
	struct nftnl_set *newset;
	struct nftnl_set_elem *elem, *newelem;

	newset = nftnl_set_alloc();
	if (newset == NULL)
		return NULL;

	memcpy(newset, set, sizeof(*set));

	if (set->flags & (1 << NFTNL_SET_TABLE)) {
		newset->table = strdup(set->table);
		if (!newset->table)
			goto err;
	}
	if (set->flags & (1 << NFTNL_SET_NAME)) {
		newset->name = strdup(set->name);
		if (!newset->name)
			goto err;
	}

	INIT_LIST_HEAD(&newset->element_list);
	list_for_each_entry(elem, &set->element_list, head) {
		newelem = nftnl_set_elem_clone(elem);
		if (newelem == NULL)
			goto err;

		list_add_tail(&newelem->head, &newset->element_list);
	}

	newset->type = NULL;

	return newset;
err:
	nftnl_set_free(newset);
	return NULL;
}

static void nftnl_set_nlmsg_build_desc_size_payload(struct nlmsghdr *nlh,
						    struct nftnl_set *s)
{
	mnl_attr_put_u32(nlh, NFTA_SET_DESC_SIZE, htonl(s->desc.size));
}

static void nftnl_set_nlmsg_build_desc_concat_payload(struct nlmsghdr *nlh,
						      struct nftnl_set *s)
{
	struct nlattr *nest;
	int i;

	nest = mnl_attr_nest_start(nlh, NFTA_SET_DESC_CONCAT);
	for (i = 0; i < NFT_REG32_COUNT && i < s->desc.field_count; i++) {
		struct nlattr *nest_elem;

		nest_elem = mnl_attr_nest_start(nlh, NFTA_LIST_ELEM);
		mnl_attr_put_u32(nlh, NFTA_SET_FIELD_LEN,
				 htonl(s->desc.field_len[i]));
		mnl_attr_nest_end(nlh, nest_elem);
	}
	mnl_attr_nest_end(nlh, nest);
}

static void
nftnl_set_nlmsg_build_desc_payload(struct nlmsghdr *nlh, struct nftnl_set *s)
{
	struct nlattr *nest;

	nest = mnl_attr_nest_start(nlh, NFTA_SET_DESC);

	if (s->flags & (1 << NFTNL_SET_DESC_SIZE))
		nftnl_set_nlmsg_build_desc_size_payload(nlh, s);
	if (s->flags & (1 << NFTNL_SET_DESC_CONCAT))
		nftnl_set_nlmsg_build_desc_concat_payload(nlh, s);

	mnl_attr_nest_end(nlh, nest);
}

EXPORT_SYMBOL(nftnl_set_nlmsg_build_payload);
void nftnl_set_nlmsg_build_payload(struct nlmsghdr *nlh, struct nftnl_set *s)
{
	int num_exprs = 0;

	if (s->flags & (1 << NFTNL_SET_TABLE))
		mnl_attr_put_strz(nlh, NFTA_SET_TABLE, s->table);
	if (s->flags & (1 << NFTNL_SET_NAME))
		mnl_attr_put_strz(nlh, NFTA_SET_NAME, s->name);
	if (s->flags & (1 << NFTNL_SET_HANDLE))
		mnl_attr_put_u64(nlh, NFTA_SET_HANDLE, htobe64(s->handle));
	if (s->flags & (1 << NFTNL_SET_FLAGS))
		mnl_attr_put_u32(nlh, NFTA_SET_FLAGS, htonl(s->set_flags));
	if (s->flags & (1 << NFTNL_SET_KEY_TYPE))
		mnl_attr_put_u32(nlh, NFTA_SET_KEY_TYPE, htonl(s->key_type));
	if (s->flags & (1 << NFTNL_SET_KEY_LEN))
		mnl_attr_put_u32(nlh, NFTA_SET_KEY_LEN, htonl(s->key_len));
	/* These are only used to map matching -> action (1:1) */
	if (s->flags & (1 << NFTNL_SET_DATA_TYPE))
		mnl_attr_put_u32(nlh, NFTA_SET_DATA_TYPE, htonl(s->data_type));
	if (s->flags & (1 << NFTNL_SET_DATA_LEN))
		mnl_attr_put_u32(nlh, NFTA_SET_DATA_LEN, htonl(s->data_len));
	if (s->flags & (1 << NFTNL_SET_OBJ_TYPE))
		mnl_attr_put_u32(nlh, NFTA_SET_OBJ_TYPE, htonl(s->obj_type));
	if (s->flags & (1 << NFTNL_SET_ID))
		mnl_attr_put_u32(nlh, NFTA_SET_ID, htonl(s->id));
	if (s->flags & (1 << NFTNL_SET_POLICY))
		mnl_attr_put_u32(nlh, NFTA_SET_POLICY, htonl(s->policy));
	if (s->flags & (1 << NFTNL_SET_DESC_SIZE | 1 << NFTNL_SET_DESC_CONCAT))
		nftnl_set_nlmsg_build_desc_payload(nlh, s);
	if (s->flags & (1 << NFTNL_SET_TIMEOUT))
		mnl_attr_put_u64(nlh, NFTA_SET_TIMEOUT, htobe64(s->timeout));
	if (s->flags & (1 << NFTNL_SET_GC_INTERVAL))
		mnl_attr_put_u32(nlh, NFTA_SET_GC_INTERVAL, htonl(s->gc_interval));
	if (s->flags & (1 << NFTNL_SET_USERDATA))
		mnl_attr_put(nlh, NFTA_SET_USERDATA, s->user.len, s->user.data);
	if (!list_empty(&s->expr_list)) {
		struct nftnl_expr *expr;

		list_for_each_entry(expr, &s->expr_list, head)
			num_exprs++;

		if (num_exprs == 1) {
			struct nlattr *nest1;

			nest1 = mnl_attr_nest_start(nlh, NFTA_SET_EXPR);
			list_for_each_entry(expr, &s->expr_list, head)
				nftnl_expr_build_payload(nlh, expr);

			mnl_attr_nest_end(nlh, nest1);
		} else if (num_exprs > 1) {
			struct nlattr *nest1, *nest2;

			nest1 = mnl_attr_nest_start(nlh, NFTA_SET_EXPRESSIONS);
			list_for_each_entry(expr, &s->expr_list, head) {
				nest2 = mnl_attr_nest_start(nlh, NFTA_LIST_ELEM);
				nftnl_expr_build_payload(nlh, expr);
				mnl_attr_nest_end(nlh, nest2);
			}
			mnl_attr_nest_end(nlh, nest1);
		}
	}
}

EXPORT_SYMBOL(nftnl_set_add_expr);
void nftnl_set_add_expr(struct nftnl_set *s, struct nftnl_expr *expr)
{
	list_add_tail(&expr->head, &s->expr_list);
}

EXPORT_SYMBOL(nftnl_set_expr_foreach);
int nftnl_set_expr_foreach(const struct nftnl_set *s,
			   int (*cb)(struct nftnl_expr *e, void *data),
			   void *data)
{
	struct nftnl_expr *cur, *tmp;
	int ret;

	list_for_each_entry_safe(cur, tmp, &s->expr_list, head) {
		ret = cb(cur, data);
		if (ret < 0)
			return ret;
	}
	return 0;
}

static int nftnl_set_parse_attr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NFTA_SET_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case NFTA_SET_TABLE:
	case NFTA_SET_NAME:
	case NFTA_SET_TYPE:
		if (mnl_attr_validate(attr, MNL_TYPE_STRING) < 0)
			abi_breakage();
		break;
	case NFTA_SET_HANDLE:
		if (mnl_attr_validate(attr, MNL_TYPE_U64) < 0)
			abi_breakage();
		break;
	case NFTA_SET_FLAGS:
	case NFTA_SET_KEY_TYPE:
	case NFTA_SET_KEY_LEN:
	case NFTA_SET_DATA_TYPE:
	case NFTA_SET_DATA_LEN:
	case NFTA_SET_ID:
	case NFTA_SET_POLICY:
	case NFTA_SET_GC_INTERVAL:
	case NFTA_SET_COUNT:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			abi_breakage();
		break;
	case NFTA_SET_USERDATA:
		if (mnl_attr_validate(attr, MNL_TYPE_BINARY) < 0)
			abi_breakage();
		break;
	case NFTA_SET_TIMEOUT:
		if (mnl_attr_validate(attr, MNL_TYPE_U64) < 0)
			abi_breakage();
		break;
	case NFTA_SET_DESC:
	case NFTA_SET_EXPR:
	case NFTA_SET_EXPRESSIONS:
		if (mnl_attr_validate(attr, MNL_TYPE_NESTED) < 0)
			abi_breakage();
		break;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static int
nftnl_set_desc_concat_field_parse_attr_cb(const struct nlattr *attr, void *data)
{
	int type = mnl_attr_get_type(attr);
	struct nftnl_set *s = data;

	if (type != NFTA_SET_FIELD_LEN)
		return MNL_CB_OK;

	if (mnl_attr_validate(attr, MNL_TYPE_U32))
		return MNL_CB_ERROR;

	s->desc.field_len[s->desc.field_count] = ntohl(mnl_attr_get_u32(attr));
	s->desc.field_count++;

	return MNL_CB_OK;
}

static int
nftnl_set_desc_concat_parse_attr_cb(const struct nlattr *attr, void *data)
{
	int type = mnl_attr_get_type(attr);
	struct nftnl_set *s = data;

	if (type != NFTA_LIST_ELEM)
		return MNL_CB_OK;

	return mnl_attr_parse_nested(attr,
				     nftnl_set_desc_concat_field_parse_attr_cb,
				     s);
}

static int nftnl_set_desc_parse_attr_cb(const struct nlattr *attr, void *data)
{
	int type = mnl_attr_get_type(attr), err;
	struct nftnl_set *s = data;

	if (mnl_attr_type_valid(attr, NFTA_SET_DESC_MAX) < 0)
		return MNL_CB_OK;

	switch (type) {
	case NFTA_SET_DESC_SIZE:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0) {
			abi_breakage();
			break;
		}

		s->desc.size = ntohl(mnl_attr_get_u32(attr));
		s->flags |= (1 << NFTNL_SET_DESC_SIZE);
		break;
	case NFTA_SET_DESC_CONCAT:
		err = mnl_attr_parse_nested(attr,
					    nftnl_set_desc_concat_parse_attr_cb,
					    s);
		if (err != MNL_CB_OK)
			abi_breakage();

		s->flags |= (1 << NFTNL_SET_DESC_CONCAT);
		break;
	default:
		break;
	}

	return MNL_CB_OK;
}

static int nftnl_set_desc_parse(struct nftnl_set *s, const struct nlattr *attr)
{
	return mnl_attr_parse_nested(attr, nftnl_set_desc_parse_attr_cb, s);
}

EXPORT_SYMBOL(nftnl_set_nlmsg_parse);
int nftnl_set_nlmsg_parse(const struct nlmsghdr *nlh, struct nftnl_set *s)
{
	struct nlattr *tb[NFTA_SET_MAX+1] = {};
	struct nfgenmsg *nfg = mnl_nlmsg_get_payload(nlh);
	struct nftnl_expr *expr, *next;
	int ret;

	if (mnl_attr_parse(nlh, sizeof(*nfg), nftnl_set_parse_attr_cb, tb) < 0)
		return -1;

	if (nftnl_parse_str_attr(tb[NFTA_SET_TABLE], NFTNL_SET_TABLE,
				 &s->table, &s->flags) < 0)
		return -1;
	if (nftnl_parse_str_attr(tb[NFTA_SET_NAME], NFTNL_SET_NAME,
				 &s->name, &s->flags) < 0)
		return -1;
	if (tb[NFTA_SET_HANDLE]) {
		s->handle = be64toh(mnl_attr_get_u64(tb[NFTA_SET_HANDLE]));
		s->flags |= (1 << NFTNL_SET_HANDLE);
	}
	if (tb[NFTA_SET_FLAGS]) {
		s->set_flags = ntohl(mnl_attr_get_u32(tb[NFTA_SET_FLAGS]));
		s->flags |= (1 << NFTNL_SET_FLAGS);
	}
	if (tb[NFTA_SET_KEY_TYPE]) {
		s->key_type = ntohl(mnl_attr_get_u32(tb[NFTA_SET_KEY_TYPE]));
		s->flags |= (1 << NFTNL_SET_KEY_TYPE);
	}
	if (tb[NFTA_SET_KEY_LEN]) {
		s->key_len = ntohl(mnl_attr_get_u32(tb[NFTA_SET_KEY_LEN]));
		s->flags |= (1 << NFTNL_SET_KEY_LEN);
	}
	if (tb[NFTA_SET_DATA_TYPE]) {
		s->data_type = ntohl(mnl_attr_get_u32(tb[NFTA_SET_DATA_TYPE]));
		s->flags |= (1 << NFTNL_SET_DATA_TYPE);
	}
	if (tb[NFTA_SET_DATA_LEN]) {
		s->data_len = ntohl(mnl_attr_get_u32(tb[NFTA_SET_DATA_LEN]));
		s->flags |= (1 << NFTNL_SET_DATA_LEN);
	}
	if (tb[NFTA_SET_OBJ_TYPE]) {
		s->obj_type = ntohl(mnl_attr_get_u32(tb[NFTA_SET_OBJ_TYPE]));
		s->flags |= (1 << NFTNL_SET_OBJ_TYPE);
	}
	if (tb[NFTA_SET_ID]) {
		s->id = ntohl(mnl_attr_get_u32(tb[NFTA_SET_ID]));
		s->flags |= (1 << NFTNL_SET_ID);
	}
	if (tb[NFTA_SET_POLICY]) {
		s->policy = ntohl(mnl_attr_get_u32(tb[NFTA_SET_POLICY]));
		s->flags |= (1 << NFTNL_SET_POLICY);
	}
	if (tb[NFTA_SET_TIMEOUT]) {
		s->timeout = be64toh(mnl_attr_get_u64(tb[NFTA_SET_TIMEOUT]));
		s->flags |= (1 << NFTNL_SET_TIMEOUT);
	}
	if (tb[NFTA_SET_GC_INTERVAL]) {
		s->gc_interval = ntohl(mnl_attr_get_u32(tb[NFTA_SET_GC_INTERVAL]));
		s->flags |= (1 << NFTNL_SET_GC_INTERVAL);
	}
	if (tb[NFTA_SET_USERDATA]) {
		ret = nftnl_set_set_data(s, NFTNL_SET_USERDATA,
			mnl_attr_get_payload(tb[NFTA_SET_USERDATA]),
			mnl_attr_get_payload_len(tb[NFTA_SET_USERDATA]));
		if (ret < 0)
			return ret;
	}
	if (tb[NFTA_SET_DESC]) {
		ret = nftnl_set_desc_parse(s, tb[NFTA_SET_DESC]);
		if (ret < 0)
			return ret;
	}
	if (tb[NFTA_SET_EXPR]) {
		expr = nftnl_expr_parse(tb[NFTA_SET_EXPR]);
		if (!expr)
			goto out_set_expr;

		list_add(&expr->head, &s->expr_list);
		s->flags |= (1 << NFTNL_SET_EXPR);
	} else if (tb[NFTA_SET_EXPRESSIONS]) {
		struct nlattr *attr;

		mnl_attr_for_each_nested(attr, tb[NFTA_SET_EXPRESSIONS]) {
			if (mnl_attr_get_type(attr) != NFTA_LIST_ELEM)
				goto out_set_expr;

			expr = nftnl_expr_parse(attr);
			if (expr == NULL)
				goto out_set_expr;

			list_add_tail(&expr->head, &s->expr_list);
		}
		s->flags |= (1 << NFTNL_SET_EXPRESSIONS);
	}

	if (tb[NFTA_SET_TYPE]) {
		xfree(s->type);
		s->type = strdup(mnl_attr_get_str(tb[NFTA_SET_TYPE]));
	}

	if (tb[NFTA_SET_COUNT]) {
		s->elemcount = ntohl(mnl_attr_get_u32(tb[NFTA_SET_COUNT]));
		s->flags |= (1 << NFTNL_SET_COUNT);
	}

	s->family = nfg->nfgen_family;
	s->flags |= (1 << NFTNL_SET_FAMILY);

	return 0;
out_set_expr:
	list_for_each_entry_safe(expr, next, &s->expr_list, head) {
		list_del(&expr->head);
		nftnl_expr_free(expr);
	}

	return -1;
}

EXPORT_SYMBOL(nftnl_set_parse);
int nftnl_set_parse(struct nftnl_set *s, enum nftnl_parse_type type,
		  const char *data, struct nftnl_parse_err *err)
{
	errno = EOPNOTSUPP;

	return -1;
}

EXPORT_SYMBOL(nftnl_set_parse_file);
int nftnl_set_parse_file(struct nftnl_set *s, enum nftnl_parse_type type,
		       FILE *fp, struct nftnl_parse_err *err)
{
	errno = EOPNOTSUPP;

	return -1;
}

static int nftnl_set_snprintf_default(char *buf, size_t remain,
				      const struct nftnl_set *s,
				      uint32_t type, uint32_t flags)
{
	struct nftnl_set_elem *elem;
	int ret, offset = 0;

	ret = snprintf(buf, remain, "%s %s %x",
			s->name, s->table, s->set_flags);
	SNPRINTF_BUFFER_SIZE(ret, remain, offset);

	if (s->flags & (1 << NFTNL_SET_TIMEOUT)) {
		ret = snprintf(buf + offset, remain, " timeout %"PRIu64"ms",
			       s->timeout);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);
	}

	if (s->flags & (1 << NFTNL_SET_GC_INTERVAL)) {
		ret = snprintf(buf + offset, remain, " gc_interval %ums",
			       s->gc_interval);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);
	}

	if (s->flags & (1 << NFTNL_SET_POLICY)) {
		ret = snprintf(buf + offset, remain, " policy %u", s->policy);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);
	}

	if (s->flags & (1 << NFTNL_SET_DESC_SIZE)) {
		ret = snprintf(buf + offset, remain, " size %u", s->desc.size);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);
	}

	if (s->type) {
		ret = snprintf(buf + offset, remain, " backend %s", s->type);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);
	}

	if (s->elemcount) {
		ret = snprintf(buf + offset, remain, " count %u", s->elemcount);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);
	}

	/* Empty set? Skip printinf of elements */
	if (list_empty(&s->element_list))
		return offset;

	ret = snprintf(buf + offset, remain, "\n");
	SNPRINTF_BUFFER_SIZE(ret, remain, offset);

	list_for_each_entry(elem, &s->element_list, head) {
		ret = snprintf(buf + offset, remain, "\t");
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);

		ret = nftnl_set_elem_snprintf_default(buf + offset, remain,
						      elem);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);
	}

	return offset;
}

static int nftnl_set_cmd_snprintf(char *buf, size_t remain,
				  const struct nftnl_set *s, uint32_t cmd,
				  uint32_t type, uint32_t flags)
{
	uint32_t inner_flags = flags;
	int ret, offset = 0;

	if (type != NFTNL_OUTPUT_DEFAULT)
		return -1;

	/* prevent set_elems to print as events */
	inner_flags &= ~NFTNL_OF_EVENT_ANY;

	ret = nftnl_set_snprintf_default(buf + offset, remain, s, type,
					 inner_flags);
	SNPRINTF_BUFFER_SIZE(ret, remain, offset);
	return offset;
}

EXPORT_SYMBOL(nftnl_set_snprintf);
int nftnl_set_snprintf(char *buf, size_t size, const struct nftnl_set *s,
		       uint32_t type, uint32_t flags)
{
	if (size)
		buf[0] = '\0';

	return nftnl_set_cmd_snprintf(buf, size, s, nftnl_flag2cmd(flags), type,
				    flags);
}

static int nftnl_set_do_snprintf(char *buf, size_t size, const void *s,
				 uint32_t cmd, uint32_t type, uint32_t flags)
{
	return nftnl_set_snprintf(buf, size, s, type, flags);
}

EXPORT_SYMBOL(nftnl_set_fprintf);
int nftnl_set_fprintf(FILE *fp, const struct nftnl_set *s, uint32_t type,
		      uint32_t flags)
{
	return nftnl_fprintf(fp, s, NFTNL_CMD_UNSPEC, type, flags,
			   nftnl_set_do_snprintf);
}

EXPORT_SYMBOL(nftnl_set_elem_add);
void nftnl_set_elem_add(struct nftnl_set *s, struct nftnl_set_elem *elem)
{
	list_add_tail(&elem->head, &s->element_list);
}

#define SET_NAME_HSIZE		512

struct nftnl_set_list {
	struct list_head list;
	struct hlist_head name_hash[SET_NAME_HSIZE];
};

EXPORT_SYMBOL(nftnl_set_list_alloc);
struct nftnl_set_list *nftnl_set_list_alloc(void)
{
	struct nftnl_set_list *list;
	int i;

	list = calloc(1, sizeof(struct nftnl_set_list));
	if (list == NULL)
		return NULL;

	INIT_LIST_HEAD(&list->list);
	for (i = 0; i < SET_NAME_HSIZE; i++)
		INIT_HLIST_HEAD(&list->name_hash[i]);

	return list;
}

EXPORT_SYMBOL(nftnl_set_list_free);
void nftnl_set_list_free(struct nftnl_set_list *list)
{
	struct nftnl_set *s, *tmp;

	list_for_each_entry_safe(s, tmp, &list->list, head) {
		list_del(&s->head);
		hlist_del(&s->hnode);
		nftnl_set_free(s);
	}
	xfree(list);
}

EXPORT_SYMBOL(nftnl_set_list_is_empty);
int nftnl_set_list_is_empty(const struct nftnl_set_list *list)
{
	return list_empty(&list->list);
}

static uint32_t djb_hash(const char *key)
{
	uint32_t i, hash = 5381;

	for (i = 0; i < strlen(key); i++)
		hash = ((hash << 5) + hash) + key[i];

	return hash;
}

EXPORT_SYMBOL(nftnl_set_list_add);
void nftnl_set_list_add(struct nftnl_set *s, struct nftnl_set_list *list)
{
	int key = djb_hash(s->name) % SET_NAME_HSIZE;

	hlist_add_head(&s->hnode, &list->name_hash[key]);
	list_add(&s->head, &list->list);
}

EXPORT_SYMBOL(nftnl_set_list_add_tail);
void nftnl_set_list_add_tail(struct nftnl_set *s, struct nftnl_set_list *list)
{
	int key = djb_hash(s->name) % SET_NAME_HSIZE;

	hlist_add_head(&s->hnode, &list->name_hash[key]);
	list_add_tail(&s->head, &list->list);
}

EXPORT_SYMBOL(nftnl_set_list_del);
void nftnl_set_list_del(struct nftnl_set *s)
{
	list_del(&s->head);
	hlist_del(&s->hnode);
}

EXPORT_SYMBOL(nftnl_set_list_foreach);
int nftnl_set_list_foreach(struct nftnl_set_list *set_list,
			 int (*cb)(struct nftnl_set *t, void *data), void *data)
{
	struct nftnl_set *cur, *tmp;
	int ret;

	list_for_each_entry_safe(cur, tmp, &set_list->list, head) {
		ret = cb(cur, data);
		if (ret < 0)
			return ret;
	}
	return 0;
}

struct nftnl_set_list_iter {
	const struct nftnl_set_list	*list;
	struct nftnl_set		*cur;
};

EXPORT_SYMBOL(nftnl_set_list_iter_create);
struct nftnl_set_list_iter *
nftnl_set_list_iter_create(const struct nftnl_set_list *l)
{
	struct nftnl_set_list_iter *iter;

	iter = calloc(1, sizeof(struct nftnl_set_list_iter));
	if (iter == NULL)
		return NULL;

	iter->list = l;
	if (nftnl_set_list_is_empty(l))
		iter->cur = NULL;
	else
		iter->cur = list_entry(l->list.next, struct nftnl_set, head);

	return iter;
}

EXPORT_SYMBOL(nftnl_set_list_iter_cur);
struct nftnl_set *
nftnl_set_list_iter_cur(const struct nftnl_set_list_iter *iter)
{
	return iter->cur;
}

EXPORT_SYMBOL(nftnl_set_list_iter_next);
struct nftnl_set *nftnl_set_list_iter_next(struct nftnl_set_list_iter *iter)
{
	struct nftnl_set *s = iter->cur;

	if (s == NULL)
		return NULL;

	/* get next rule, if any */
	iter->cur = list_entry(iter->cur->head.next, struct nftnl_set, head);
	if (&iter->cur->head == iter->list->list.next)
		return NULL;

	return s;
}

EXPORT_SYMBOL(nftnl_set_list_iter_destroy);
void nftnl_set_list_iter_destroy(const struct nftnl_set_list_iter *iter)
{
	xfree(iter);
}

EXPORT_SYMBOL(nftnl_set_list_lookup_byname);
struct nftnl_set *
nftnl_set_list_lookup_byname(struct nftnl_set_list *set_list, const char *set)
{
	int key = djb_hash(set) % SET_NAME_HSIZE;
	struct hlist_node *n;
	struct nftnl_set *s;

	hlist_for_each_entry(s, n, &set_list->name_hash[key], hnode) {
		if (!strcmp(set, s->name))
			return s;
	}
	return NULL;
}

int nftnl_set_lookup_id(struct nftnl_expr *e,
		      struct nftnl_set_list *set_list, uint32_t *set_id)
{
	const char *set_name;
	struct nftnl_set *s;

	set_name = nftnl_expr_get_str(e, NFTNL_EXPR_LOOKUP_SET);
	if (set_name == NULL)
		return 0;

	s = nftnl_set_list_lookup_byname(set_list, set_name);
	if (s == NULL)
		return 0;

	*set_id = nftnl_set_get_u32(s, NFTNL_SET_ID);
	return 1;
}
