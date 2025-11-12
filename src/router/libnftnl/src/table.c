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
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <errno.h>

#include <libmnl/libmnl.h>
#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nf_tables.h>

#include <libnftnl/table.h>

struct nftnl_table {
	struct list_head head;

	const char	*name;
	uint32_t	family;
	uint32_t	table_flags;
	uint64_t 	handle;
	uint32_t	use;
	uint32_t	flags;
	uint32_t	owner;
	struct {
		void		*data;
		uint32_t	len;
	} user;
};

EXPORT_SYMBOL(nftnl_table_alloc);
struct nftnl_table *nftnl_table_alloc(void)
{
	return calloc(1, sizeof(struct nftnl_table));
}

EXPORT_SYMBOL(nftnl_table_free);
void nftnl_table_free(const struct nftnl_table *t)
{
	if (t->flags & (1 << NFTNL_TABLE_NAME))
		xfree(t->name);
	if (t->flags & (1 << NFTNL_TABLE_USERDATA))
		xfree(t->user.data);

	xfree(t);
}

EXPORT_SYMBOL(nftnl_table_is_set);
bool nftnl_table_is_set(const struct nftnl_table *t, uint16_t attr)
{
	return t->flags & (1 << attr);
}

EXPORT_SYMBOL(nftnl_table_unset);
void nftnl_table_unset(struct nftnl_table *t, uint16_t attr)
{
	if (!(t->flags & (1 << attr)))
		return;

	switch (attr) {
	case NFTNL_TABLE_NAME:
		xfree(t->name);
		break;
	case NFTNL_TABLE_USERDATA:
		xfree(t->user.data);
		break;
	case NFTNL_TABLE_FLAGS:
	case NFTNL_TABLE_HANDLE:
	case NFTNL_TABLE_FAMILY:
	case NFTNL_TABLE_USE:
	case NFTNL_TABLE_OWNER:
		break;
	}
	t->flags &= ~(1 << attr);
}

static uint32_t nftnl_table_validate[NFTNL_TABLE_MAX + 1] = {
	[NFTNL_TABLE_FLAGS]	= sizeof(uint32_t),
	[NFTNL_TABLE_FAMILY]	= sizeof(uint32_t),
	[NFTNL_TABLE_HANDLE]	= sizeof(uint64_t),
	[NFTNL_TABLE_USE]	= sizeof(uint32_t),
	[NFTNL_TABLE_OWNER]	= sizeof(uint32_t),
};

EXPORT_SYMBOL(nftnl_table_set_data);
int nftnl_table_set_data(struct nftnl_table *t, uint16_t attr,
			 const void *data, uint32_t data_len)
{
	nftnl_assert_attr_exists(attr, NFTNL_TABLE_MAX);
	nftnl_assert_validate(data, nftnl_table_validate, attr, data_len);

	switch (attr) {
	case NFTNL_TABLE_NAME:
		return nftnl_set_str_attr(&t->name, &t->flags,
					  attr, data, data_len);
	case NFTNL_TABLE_HANDLE:
		memcpy(&t->handle, data, sizeof(t->handle));
		break;
	case NFTNL_TABLE_FLAGS:
		memcpy(&t->table_flags, data, sizeof(t->table_flags));
		break;
	case NFTNL_TABLE_FAMILY:
		memcpy(&t->family, data, sizeof(t->family));
		break;
	case NFTNL_TABLE_USE:
		memcpy(&t->use, data, sizeof(t->use));
		break;
	case NFTNL_TABLE_USERDATA:
		if (t->flags & (1 << NFTNL_TABLE_USERDATA))
			xfree(t->user.data);

		t->user.data = malloc(data_len);
		if (!t->user.data)
			return -1;
		memcpy(t->user.data, data, data_len);
		t->user.len = data_len;
		break;
	case NFTNL_TABLE_OWNER:
		memcpy(&t->owner, data, sizeof(t->owner));
		break;
	}
	t->flags |= (1 << attr);
	return 0;
}

void nftnl_table_set(struct nftnl_table *t, uint16_t attr, const void *data) __visible;
void nftnl_table_set(struct nftnl_table *t, uint16_t attr, const void *data)
{
	nftnl_table_set_data(t, attr, data, nftnl_table_validate[attr]);
}

EXPORT_SYMBOL(nftnl_table_set_u32);
void nftnl_table_set_u32(struct nftnl_table *t, uint16_t attr, uint32_t val)
{
	nftnl_table_set_data(t, attr, &val, sizeof(uint32_t));
}

EXPORT_SYMBOL(nftnl_table_set_u64);
void nftnl_table_set_u64(struct nftnl_table *t, uint16_t attr, uint64_t val)
{
	nftnl_table_set_data(t, attr, &val, sizeof(uint64_t));
}

EXPORT_SYMBOL(nftnl_table_set_u8);
void nftnl_table_set_u8(struct nftnl_table *t, uint16_t attr, uint8_t val)
{
	nftnl_table_set_data(t, attr, &val, sizeof(uint8_t));
}

EXPORT_SYMBOL(nftnl_table_set_str);
int nftnl_table_set_str(struct nftnl_table *t, uint16_t attr, const char *str)
{
	return nftnl_table_set_data(t, attr, str, strlen(str) + 1);
}

EXPORT_SYMBOL(nftnl_table_get_data);
const void *nftnl_table_get_data(const struct nftnl_table *t, uint16_t attr,
				 uint32_t *data_len)
{
	if (!(t->flags & (1 << attr)))
		return NULL;

	switch(attr) {
	case NFTNL_TABLE_NAME:
		*data_len = strlen(t->name) + 1;
		return t->name;
	case NFTNL_TABLE_HANDLE:
		*data_len = sizeof(uint64_t);
		return &t->handle;
	case NFTNL_TABLE_FLAGS:
		*data_len = sizeof(uint32_t);
		return &t->table_flags;
	case NFTNL_TABLE_FAMILY:
		*data_len = sizeof(uint32_t);
		return &t->family;
	case NFTNL_TABLE_USE:
		*data_len = sizeof(uint32_t);
		return &t->use;
	case NFTNL_TABLE_USERDATA:
		*data_len = t->user.len;
		return t->user.data;
	case NFTNL_TABLE_OWNER:
		*data_len = sizeof(uint32_t);
		return &t->owner;
	}
	return NULL;
}

EXPORT_SYMBOL(nftnl_table_get);
const void *nftnl_table_get(const struct nftnl_table *t, uint16_t attr)
{
	uint32_t data_len;
	return nftnl_table_get_data(t, attr, &data_len);
}

EXPORT_SYMBOL(nftnl_table_get_u32);
uint32_t nftnl_table_get_u32(const struct nftnl_table *t, uint16_t attr)
{
	const void *ret = nftnl_table_get(t, attr);
	return ret == NULL ? 0 : *((uint32_t *)ret);
}

EXPORT_SYMBOL(nftnl_table_get_u64);
uint64_t nftnl_table_get_u64(const struct nftnl_table *t, uint16_t attr)
{
	const void *ret = nftnl_table_get(t, attr);
	return ret == NULL ? 0 : *((uint64_t *)ret);
}

EXPORT_SYMBOL(nftnl_table_get_u8);
uint8_t nftnl_table_get_u8(const struct nftnl_table *t, uint16_t attr)
{
	const void *ret = nftnl_table_get(t, attr);
	return ret == NULL ? 0 : *((uint8_t *)ret);
}

EXPORT_SYMBOL(nftnl_table_get_str);
const char *nftnl_table_get_str(const struct nftnl_table *t, uint16_t attr)
{
	return nftnl_table_get(t, attr);
}

EXPORT_SYMBOL(nftnl_table_nlmsg_build_payload);
void nftnl_table_nlmsg_build_payload(struct nlmsghdr *nlh, const struct nftnl_table *t)
{
	if (t->flags & (1 << NFTNL_TABLE_NAME))
		mnl_attr_put_strz(nlh, NFTA_TABLE_NAME, t->name);
	if (t->flags & (1 << NFTNL_TABLE_HANDLE))
		mnl_attr_put_u64(nlh, NFTA_TABLE_HANDLE, htobe64(t->handle));
	if (t->flags & (1 << NFTNL_TABLE_FLAGS))
		mnl_attr_put_u32(nlh, NFTA_TABLE_FLAGS, htonl(t->table_flags));
	if (t->flags & (1 << NFTNL_TABLE_USERDATA))
		mnl_attr_put(nlh, NFTA_TABLE_USERDATA, t->user.len, t->user.data);
}

static int nftnl_table_parse_attr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NFTA_TABLE_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case NFTA_TABLE_NAME:
		if (mnl_attr_validate(attr, MNL_TYPE_STRING) < 0)
			abi_breakage();
		break;
	case NFTA_TABLE_HANDLE:
		if (mnl_attr_validate(attr, MNL_TYPE_U64) < 0)
			abi_breakage();
		break;
	case NFTA_TABLE_FLAGS:
	case NFTA_TABLE_USE:
	case NFTA_TABLE_OWNER:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			abi_breakage();
		break;
	case NFTA_TABLE_USERDATA:
		if (mnl_attr_validate(attr, MNL_TYPE_BINARY) < 0)
			abi_breakage();
		break;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

EXPORT_SYMBOL(nftnl_table_nlmsg_parse);
int nftnl_table_nlmsg_parse(const struct nlmsghdr *nlh, struct nftnl_table *t)
{
	struct nlattr *tb[NFTA_TABLE_MAX+1] = {};
	struct nfgenmsg *nfg = mnl_nlmsg_get_payload(nlh);
	int ret;

	if (mnl_attr_parse(nlh, sizeof(*nfg), nftnl_table_parse_attr_cb, tb) < 0)
		return -1;

	if (tb[NFTA_TABLE_NAME]) {
		if (t->flags & (1 << NFTNL_TABLE_NAME))
			xfree(t->name);
		t->name = strdup(mnl_attr_get_str(tb[NFTA_TABLE_NAME]));
		if (!t->name)
			return -1;
		t->flags |= (1 << NFTNL_TABLE_NAME);
	}
	if (tb[NFTA_TABLE_FLAGS]) {
		t->table_flags = ntohl(mnl_attr_get_u32(tb[NFTA_TABLE_FLAGS]));
		t->flags |= (1 << NFTNL_TABLE_FLAGS);
	}
	if (tb[NFTA_TABLE_USE]) {
		t->use = ntohl(mnl_attr_get_u32(tb[NFTA_TABLE_USE]));
		t->flags |= (1 << NFTNL_TABLE_USE);
	}
	if (tb[NFTA_TABLE_HANDLE]) {
		t->handle = be64toh(mnl_attr_get_u64(tb[NFTA_TABLE_HANDLE]));
		t->flags |= (1 << NFTNL_TABLE_HANDLE);
	}
	if (tb[NFTA_TABLE_USERDATA]) {
		ret = nftnl_table_set_data(t, NFTNL_TABLE_USERDATA,
			mnl_attr_get_payload(tb[NFTA_TABLE_USERDATA]),
			mnl_attr_get_payload_len(tb[NFTA_TABLE_USERDATA]));
		if (ret < 0)
			return ret;
	}
	if (tb[NFTA_TABLE_OWNER]) {
		t->owner = ntohl(mnl_attr_get_u32(tb[NFTA_TABLE_OWNER]));
		t->flags |= (1 << NFTNL_TABLE_OWNER);
	}

	t->family = nfg->nfgen_family;
	t->flags |= (1 << NFTNL_TABLE_FAMILY);

	return 0;
}

EXPORT_SYMBOL(nftnl_table_parse);
int nftnl_table_parse(struct nftnl_table *t, enum nftnl_parse_type type,
		    const char *data, struct nftnl_parse_err *err)
{
	errno = EOPNOTSUPP;

	return -1;
}

EXPORT_SYMBOL(nftnl_table_parse_file);
int nftnl_table_parse_file(struct nftnl_table *t, enum nftnl_parse_type type,
			 FILE *fp, struct nftnl_parse_err *err)
{
	errno = EOPNOTSUPP;

	return -1;
}

static int nftnl_table_snprintf_default(char *buf, size_t size,
					const struct nftnl_table *t)
{
	return snprintf(buf, size, "table %s %s flags %x use %d handle %llu",
			t->name, nftnl_family2str(t->family),
			t->table_flags, t->use, (unsigned long long)t->handle);
}

static int nftnl_table_cmd_snprintf(char *buf, size_t remain,
				    const struct nftnl_table *t, uint32_t cmd,
				    uint32_t type, uint32_t flags)
{
	int ret, offset = 0;

	if (type != NFTNL_OUTPUT_DEFAULT)
		return -1;

	ret = nftnl_table_snprintf_default(buf + offset, remain, t);
	SNPRINTF_BUFFER_SIZE(ret, remain, offset);
	return offset;
}

EXPORT_SYMBOL(nftnl_table_snprintf);
int nftnl_table_snprintf(char *buf, size_t size, const struct nftnl_table *t,
			 uint32_t type, uint32_t flags)
{
	if (size)
		buf[0] = '\0';

	return nftnl_table_cmd_snprintf(buf, size, t, nftnl_flag2cmd(flags), type,
				      flags);
}

static int nftnl_table_do_snprintf(char *buf, size_t size, const void *t,
				   uint32_t cmd, uint32_t type, uint32_t flags)
{
	return nftnl_table_snprintf(buf, size, t, type, flags);
}

EXPORT_SYMBOL(nftnl_table_fprintf);
int nftnl_table_fprintf(FILE *fp, const struct nftnl_table *t, uint32_t type,
			uint32_t flags)
{
	return nftnl_fprintf(fp, t, NFTNL_CMD_UNSPEC, type, flags,
			   nftnl_table_do_snprintf);
}

struct nftnl_table_list {
	struct list_head list;
};

EXPORT_SYMBOL(nftnl_table_list_alloc);
struct nftnl_table_list *nftnl_table_list_alloc(void)
{
	struct nftnl_table_list *list;

	list = calloc(1, sizeof(struct nftnl_table_list));
	if (list == NULL)
		return NULL;

	INIT_LIST_HEAD(&list->list);

	return list;
}

EXPORT_SYMBOL(nftnl_table_list_free);
void nftnl_table_list_free(struct nftnl_table_list *list)
{
	struct nftnl_table *r, *tmp;

	list_for_each_entry_safe(r, tmp, &list->list, head) {
		list_del(&r->head);
		nftnl_table_free(r);
	}
	xfree(list);
}

EXPORT_SYMBOL(nftnl_table_list_is_empty);
int nftnl_table_list_is_empty(const struct nftnl_table_list *list)
{
	return list_empty(&list->list);
}

EXPORT_SYMBOL(nftnl_table_list_add);
void nftnl_table_list_add(struct nftnl_table *r, struct nftnl_table_list *list)
{
	list_add(&r->head, &list->list);
}

EXPORT_SYMBOL(nftnl_table_list_add_tail);
void nftnl_table_list_add_tail(struct nftnl_table *r, struct nftnl_table_list *list)
{
	list_add_tail(&r->head, &list->list);
}

EXPORT_SYMBOL(nftnl_table_list_del);
void nftnl_table_list_del(struct nftnl_table *t)
{
	list_del(&t->head);
}

EXPORT_SYMBOL(nftnl_table_list_foreach);
int nftnl_table_list_foreach(struct nftnl_table_list *table_list,
			   int (*cb)(struct nftnl_table *t, void *data),
			   void *data)
{
	struct nftnl_table *cur, *tmp;
	int ret;

	list_for_each_entry_safe(cur, tmp, &table_list->list, head) {
		ret = cb(cur, data);
		if (ret < 0)
			return ret;
	}
	return 0;
}

struct nftnl_table_list_iter {
	const struct nftnl_table_list	*list;
	struct nftnl_table		*cur;
};

EXPORT_SYMBOL(nftnl_table_list_iter_create);
struct nftnl_table_list_iter *
nftnl_table_list_iter_create(const struct nftnl_table_list *l)
{
	struct nftnl_table_list_iter *iter;

	iter = calloc(1, sizeof(struct nftnl_table_list_iter));
	if (iter == NULL)
		return NULL;

	iter->list = l;
	if (nftnl_table_list_is_empty(l))
		iter->cur = NULL;
	else
		iter->cur = list_entry(l->list.next, struct nftnl_table, head);

	return iter;
}

EXPORT_SYMBOL(nftnl_table_list_iter_next);
struct nftnl_table *nftnl_table_list_iter_next(struct nftnl_table_list_iter *iter)
{
	struct nftnl_table *r = iter->cur;

	if (r == NULL)
		return NULL;

	/* get next table, if any */
	iter->cur = list_entry(iter->cur->head.next, struct nftnl_table, head);
	if (&iter->cur->head == iter->list->list.next)
		return NULL;

	return r;
}

EXPORT_SYMBOL(nftnl_table_list_iter_destroy);
void nftnl_table_list_iter_destroy(const struct nftnl_table_list_iter *iter)
{
	xfree(iter);
}
