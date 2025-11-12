#include "internal.h"

#include <time.h>
#include <endian.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <netinet/in.h>
#include <errno.h>
#include <inttypes.h>

#include <libmnl/libmnl.h>
#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nf_tables.h>
#include <linux/netfilter.h>
#include <linux/netfilter_arp.h>

#include <libnftnl/flowtable.h>

struct nftnl_flowtable {
	struct list_head	head;
	const char		*name;
	const char		*table;
	int			family;
	uint32_t		hooknum;
	int32_t			prio;
	uint32_t		size;
	struct nftnl_str_array	dev_array;
	uint32_t		ft_flags;
	uint32_t		use;
	uint32_t		flags;
	uint64_t		handle;
};

EXPORT_SYMBOL(nftnl_flowtable_alloc);
struct nftnl_flowtable *nftnl_flowtable_alloc(void)
{
	return calloc(1, sizeof(struct nftnl_flowtable));
}

EXPORT_SYMBOL(nftnl_flowtable_free);
void nftnl_flowtable_free(const struct nftnl_flowtable *c)
{
	if (c->flags & (1 << NFTNL_FLOWTABLE_NAME))
		xfree(c->name);
	if (c->flags & (1 << NFTNL_FLOWTABLE_TABLE))
		xfree(c->table);
	if (c->flags & (1 << NFTNL_FLOWTABLE_DEVICES))
		nftnl_str_array_clear((struct nftnl_str_array *)&c->dev_array);
	xfree(c);
}

EXPORT_SYMBOL(nftnl_flowtable_is_set);
bool nftnl_flowtable_is_set(const struct nftnl_flowtable *c, uint16_t attr)
{
	return c->flags & (1 << attr);
}

EXPORT_SYMBOL(nftnl_flowtable_unset);
void nftnl_flowtable_unset(struct nftnl_flowtable *c, uint16_t attr)
{
	if (!(c->flags & (1 << attr)))
		return;

	switch (attr) {
	case NFTNL_FLOWTABLE_NAME:
		xfree(c->name);
		break;
	case NFTNL_FLOWTABLE_TABLE:
		xfree(c->table);
		break;
	case NFTNL_FLOWTABLE_HOOKNUM:
	case NFTNL_FLOWTABLE_PRIO:
	case NFTNL_FLOWTABLE_USE:
	case NFTNL_FLOWTABLE_FAMILY:
	case NFTNL_FLOWTABLE_FLAGS:
	case NFTNL_FLOWTABLE_HANDLE:
		break;
	case NFTNL_FLOWTABLE_DEVICES:
		nftnl_str_array_clear(&c->dev_array);
		break;
	default:
		return;
	}

	c->flags &= ~(1 << attr);
}

static uint32_t nftnl_flowtable_validate[NFTNL_FLOWTABLE_MAX + 1] = {
	[NFTNL_FLOWTABLE_HOOKNUM]	= sizeof(uint32_t),
	[NFTNL_FLOWTABLE_PRIO]		= sizeof(int32_t),
	[NFTNL_FLOWTABLE_FAMILY]	= sizeof(uint32_t),
	[NFTNL_FLOWTABLE_SIZE]		= sizeof(uint32_t),
	[NFTNL_FLOWTABLE_FLAGS]		= sizeof(uint32_t),
	[NFTNL_FLOWTABLE_HANDLE]	= sizeof(uint64_t),
};

EXPORT_SYMBOL(nftnl_flowtable_set_data);
int nftnl_flowtable_set_data(struct nftnl_flowtable *c, uint16_t attr,
			     const void *data, uint32_t data_len)
{
	nftnl_assert_attr_exists(attr, NFTNL_FLOWTABLE_MAX);
	nftnl_assert_validate(data, nftnl_flowtable_validate, attr, data_len);

	switch(attr) {
	case NFTNL_FLOWTABLE_NAME:
		return nftnl_set_str_attr(&c->name, &c->flags,
					  attr, data, data_len);
	case NFTNL_FLOWTABLE_TABLE:
		return nftnl_set_str_attr(&c->table, &c->flags,
					  attr, data, data_len);
		break;
	case NFTNL_FLOWTABLE_HOOKNUM:
		memcpy(&c->hooknum, data, sizeof(c->hooknum));
		break;
	case NFTNL_FLOWTABLE_PRIO:
		memcpy(&c->prio, data, sizeof(c->prio));
		break;
	case NFTNL_FLOWTABLE_FAMILY:
		memcpy(&c->family, data, sizeof(c->family));
		break;
	case NFTNL_FLOWTABLE_DEVICES:
		if (nftnl_str_array_set(&c->dev_array, data) < 0)
			return -1;
		break;
	case NFTNL_FLOWTABLE_SIZE:
		memcpy(&c->size, data, sizeof(c->size));
		break;
	case NFTNL_FLOWTABLE_FLAGS:
		memcpy(&c->ft_flags, data, sizeof(c->ft_flags));
		break;
	case NFTNL_FLOWTABLE_HANDLE:
		memcpy(&c->handle, data, sizeof(c->handle));
		break;
	}
	c->flags |= (1 << attr);
	return 0;
}

void nftnl_flowtable_set(struct nftnl_flowtable *c, uint16_t attr, const void *data) __visible;
void nftnl_flowtable_set(struct nftnl_flowtable *c, uint16_t attr, const void *data)
{
	nftnl_flowtable_set_data(c, attr, data, nftnl_flowtable_validate[attr]);
}

EXPORT_SYMBOL(nftnl_flowtable_set_u32);
void nftnl_flowtable_set_u32(struct nftnl_flowtable *c, uint16_t attr, uint32_t data)
{
	nftnl_flowtable_set_data(c, attr, &data, sizeof(uint32_t));
}

EXPORT_SYMBOL(nftnl_flowtable_set_s32);
void nftnl_flowtable_set_s32(struct nftnl_flowtable *c, uint16_t attr, int32_t data)
{
	nftnl_flowtable_set_data(c, attr, &data, sizeof(int32_t));
}

EXPORT_SYMBOL(nftnl_flowtable_set_str);
int nftnl_flowtable_set_str(struct nftnl_flowtable *c, uint16_t attr, const char *str)
{
	return nftnl_flowtable_set_data(c, attr, str, strlen(str) + 1);
}

EXPORT_SYMBOL(nftnl_flowtable_set_u64);
void nftnl_flowtable_set_u64(struct nftnl_flowtable *c, uint16_t attr, uint64_t data)
{
	nftnl_flowtable_set_data(c, attr, &data, sizeof(uint64_t));
}

EXPORT_SYMBOL(nftnl_flowtable_set_array);
int nftnl_flowtable_set_array(struct nftnl_flowtable *c, uint16_t attr,
			      const char **data)
{
	return nftnl_flowtable_set_data(c, attr, data, 0);
}

EXPORT_SYMBOL(nftnl_flowtable_get_data);
const void *nftnl_flowtable_get_data(const struct nftnl_flowtable *c,
				     uint16_t attr, uint32_t *data_len)
{
	if (!(c->flags & (1 << attr)))
		return NULL;

	switch(attr) {
	case NFTNL_FLOWTABLE_NAME:
		*data_len = strlen(c->name) + 1;
		return c->name;
	case NFTNL_FLOWTABLE_TABLE:
		*data_len = strlen(c->table) + 1;
		return c->table;
	case NFTNL_FLOWTABLE_HOOKNUM:
		*data_len = sizeof(uint32_t);
		return &c->hooknum;
	case NFTNL_FLOWTABLE_PRIO:
		*data_len = sizeof(int32_t);
		return &c->prio;
	case NFTNL_FLOWTABLE_FAMILY:
		*data_len = sizeof(int32_t);
		return &c->family;
	case NFTNL_FLOWTABLE_DEVICES:
		*data_len = 0;
		return c->dev_array.array;
	case NFTNL_FLOWTABLE_SIZE:
		*data_len = sizeof(int32_t);
		return &c->size;
	case NFTNL_FLOWTABLE_FLAGS:
		*data_len = sizeof(int32_t);
		return &c->ft_flags;
	case NFTNL_FLOWTABLE_HANDLE:
		*data_len = sizeof(uint64_t);
		return &c->handle;
	}
	return NULL;
}

EXPORT_SYMBOL(nftnl_flowtable_get);
const void *nftnl_flowtable_get(const struct nftnl_flowtable *c, uint16_t attr)
{
	uint32_t data_len;
	return nftnl_flowtable_get_data(c, attr, &data_len);
}

EXPORT_SYMBOL(nftnl_flowtable_get_str);
const char *nftnl_flowtable_get_str(const struct nftnl_flowtable *c, uint16_t attr)
{
	return nftnl_flowtable_get(c, attr);
}

EXPORT_SYMBOL(nftnl_flowtable_get_u32);
uint32_t nftnl_flowtable_get_u32(const struct nftnl_flowtable *c, uint16_t attr)
{
	uint32_t data_len = 0;
	const uint32_t *val = nftnl_flowtable_get_data(c, attr, &data_len);

	nftnl_assert(val, attr, data_len == sizeof(uint32_t));

	return val ? *val : 0;
}

EXPORT_SYMBOL(nftnl_flowtable_get_u64);
uint64_t nftnl_flowtable_get_u64(const struct nftnl_flowtable *c, uint16_t attr)
{
	uint32_t data_len = 0;
	const uint64_t *val = nftnl_flowtable_get_data(c, attr, &data_len);

	nftnl_assert(val, attr, data_len == sizeof(uint64_t));

	return val ? *val : 0;
}

EXPORT_SYMBOL(nftnl_flowtable_get_s32);
int32_t nftnl_flowtable_get_s32(const struct nftnl_flowtable *c, uint16_t attr)
{
	uint32_t data_len = 0;
	const int32_t *val = nftnl_flowtable_get_data(c, attr, &data_len);

	nftnl_assert(val, attr, data_len == sizeof(int32_t));

	return val ? *val : 0;
}

EXPORT_SYMBOL(nftnl_flowtable_get_array);
const char *const *nftnl_flowtable_get_array(const struct nftnl_flowtable *c, uint16_t attr)
{
	uint32_t data_len;
	const char * const *val = nftnl_flowtable_get_data(c, attr, &data_len);

	nftnl_assert(val, attr, attr == NFTNL_FLOWTABLE_DEVICES);

	return val;
}

EXPORT_SYMBOL(nftnl_flowtable_nlmsg_build_payload);
void nftnl_flowtable_nlmsg_build_payload(struct nlmsghdr *nlh,
					 const struct nftnl_flowtable *c)
{
	struct nlattr *nest = NULL;
	int i;

	if (c->flags & (1 << NFTNL_FLOWTABLE_TABLE))
		mnl_attr_put_strz(nlh, NFTA_FLOWTABLE_TABLE, c->table);
	if (c->flags & (1 << NFTNL_FLOWTABLE_NAME))
		mnl_attr_put_strz(nlh, NFTA_FLOWTABLE_NAME, c->name);

	if (c->flags & (1 << NFTNL_FLOWTABLE_HOOKNUM) ||
	    c->flags & (1 << NFTNL_FLOWTABLE_PRIO) ||
	    c->flags & (1 << NFTNL_FLOWTABLE_DEVICES))
		nest = mnl_attr_nest_start(nlh, NFTA_FLOWTABLE_HOOK);

	if (c->flags & (1 << NFTNL_FLOWTABLE_HOOKNUM))
		mnl_attr_put_u32(nlh, NFTA_FLOWTABLE_HOOK_NUM, htonl(c->hooknum));
	if (c->flags & (1 << NFTNL_FLOWTABLE_PRIO))
		mnl_attr_put_u32(nlh, NFTA_FLOWTABLE_HOOK_PRIORITY, htonl(c->prio));

	if (c->flags & (1 << NFTNL_FLOWTABLE_DEVICES)) {
		struct nlattr *nest_dev;
		const char *dev;

		nest_dev = mnl_attr_nest_start(nlh, NFTA_FLOWTABLE_HOOK_DEVS);
		nftnl_str_array_foreach(dev, &c->dev_array, i)
			mnl_attr_put_strz(nlh, NFTA_DEVICE_NAME, dev);
		mnl_attr_nest_end(nlh, nest_dev);
	}

	if (nest)
		mnl_attr_nest_end(nlh, nest);

	if (c->flags & (1 << NFTNL_FLOWTABLE_FLAGS))
		mnl_attr_put_u32(nlh, NFTA_FLOWTABLE_FLAGS, htonl(c->ft_flags));
	if (c->flags & (1 << NFTNL_FLOWTABLE_USE))
		mnl_attr_put_u32(nlh, NFTA_FLOWTABLE_USE, htonl(c->use));
	if (c->flags & (1 << NFTNL_FLOWTABLE_HANDLE))
		mnl_attr_put_u64(nlh, NFTA_FLOWTABLE_HANDLE, htobe64(c->handle));
}

static int nftnl_flowtable_parse_attr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NFTA_FLOWTABLE_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case NFTA_FLOWTABLE_NAME:
	case NFTA_FLOWTABLE_TABLE:
		if (mnl_attr_validate(attr, MNL_TYPE_STRING) < 0)
			abi_breakage();
		break;
	case NFTA_FLOWTABLE_HOOK:
		if (mnl_attr_validate(attr, MNL_TYPE_NESTED) < 0)
			abi_breakage();
		break;
	case NFTA_FLOWTABLE_FLAGS:
	case NFTA_FLOWTABLE_USE:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			abi_breakage();
		break;
	case NFTA_FLOWTABLE_HANDLE:
		if (mnl_attr_validate(attr, MNL_TYPE_U64) < 0)
			abi_breakage();
		break;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static int nftnl_flowtable_parse_hook_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NFTA_FLOWTABLE_HOOK_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case NFTA_FLOWTABLE_HOOK_NUM:
	case NFTA_FLOWTABLE_HOOK_PRIORITY:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			abi_breakage();
		break;
	case NFTA_FLOWTABLE_HOOK_DEVS:
		if (mnl_attr_validate(attr, MNL_TYPE_NESTED) < 0)
			abi_breakage();
		break;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static int nftnl_flowtable_parse_hook(struct nlattr *attr, struct nftnl_flowtable *c)
{
	struct nlattr *tb[NFTA_FLOWTABLE_HOOK_MAX + 1] = {};
	int ret;

	if (mnl_attr_parse_nested(attr, nftnl_flowtable_parse_hook_cb, tb) < 0)
		return -1;

	if (tb[NFTA_FLOWTABLE_HOOK_NUM]) {
		c->hooknum = ntohl(mnl_attr_get_u32(tb[NFTA_FLOWTABLE_HOOK_NUM]));
		c->flags |= (1 << NFTNL_FLOWTABLE_HOOKNUM);
	}
	if (tb[NFTA_FLOWTABLE_HOOK_PRIORITY]) {
		c->prio = ntohl(mnl_attr_get_u32(tb[NFTA_FLOWTABLE_HOOK_PRIORITY]));
		c->flags |= (1 << NFTNL_FLOWTABLE_PRIO);
	}
	if (tb[NFTA_FLOWTABLE_HOOK_DEVS]) {
		ret = nftnl_parse_devs(&c->dev_array,
				       tb[NFTA_FLOWTABLE_HOOK_DEVS]);
		if (ret < 0)
			return -1;
		c->flags |= (1 << NFTNL_FLOWTABLE_DEVICES);
	}

	return 0;
}

EXPORT_SYMBOL(nftnl_flowtable_nlmsg_parse);
int nftnl_flowtable_nlmsg_parse(const struct nlmsghdr *nlh, struct nftnl_flowtable *c)
{
	struct nlattr *tb[NFTA_FLOWTABLE_MAX + 1] = {};
	struct nfgenmsg *nfg = mnl_nlmsg_get_payload(nlh);
	int ret = 0;

	if (mnl_attr_parse(nlh, sizeof(*nfg), nftnl_flowtable_parse_attr_cb, tb) < 0)
		return -1;

	if (tb[NFTA_FLOWTABLE_NAME]) {
		if (c->flags & (1 << NFTNL_FLOWTABLE_NAME))
			xfree(c->name);
		c->name = strdup(mnl_attr_get_str(tb[NFTA_FLOWTABLE_NAME]));
		if (!c->name)
			return -1;
		c->flags |= (1 << NFTNL_FLOWTABLE_NAME);
	}
	if (tb[NFTA_FLOWTABLE_TABLE]) {
		if (c->flags & (1 << NFTNL_FLOWTABLE_TABLE))
			xfree(c->table);
		c->table = strdup(mnl_attr_get_str(tb[NFTA_FLOWTABLE_TABLE]));
		if (!c->table)
			return -1;
		c->flags |= (1 << NFTNL_FLOWTABLE_TABLE);
	}
	if (tb[NFTA_FLOWTABLE_HOOK]) {
		ret = nftnl_flowtable_parse_hook(tb[NFTA_FLOWTABLE_HOOK], c);
		if (ret < 0)
			return ret;
	}
	if (tb[NFTA_FLOWTABLE_FLAGS]) {
		c->ft_flags = ntohl(mnl_attr_get_u32(tb[NFTA_FLOWTABLE_FLAGS]));
		c->flags |= (1 << NFTNL_FLOWTABLE_FLAGS);
	}
	if (tb[NFTA_FLOWTABLE_USE]) {
		c->use = ntohl(mnl_attr_get_u32(tb[NFTA_FLOWTABLE_USE]));
		c->flags |= (1 << NFTNL_FLOWTABLE_USE);
	}
	if (tb[NFTA_FLOWTABLE_HANDLE]) {
		c->handle = be64toh(mnl_attr_get_u64(tb[NFTA_FLOWTABLE_HANDLE]));
		c->flags |= (1 << NFTNL_FLOWTABLE_HANDLE);
	}

	c->family = nfg->nfgen_family;
	c->flags |= (1 << NFTNL_FLOWTABLE_FAMILY);

	return ret;
}

static const char *nftnl_hooknum2str(int family, int hooknum)
{
	switch (family) {
	case NFPROTO_IPV4:
	case NFPROTO_IPV6:
	case NFPROTO_INET:
	case NFPROTO_BRIDGE:
		switch (hooknum) {
		case NF_INET_PRE_ROUTING:
			return "prerouting";
		case NF_INET_LOCAL_IN:
			return "input";
		case NF_INET_FORWARD:
			return "forward";
		case NF_INET_LOCAL_OUT:
			return "output";
		case NF_INET_POST_ROUTING:
			return "postrouting";
		}
		break;
	case NFPROTO_ARP:
		switch (hooknum) {
		case NF_ARP_IN:
			return "input";
		case NF_ARP_OUT:
			return "output";
		case NF_ARP_FORWARD:
			return "forward";
		}
		break;
	case NFPROTO_NETDEV:
		switch (hooknum) {
		case NF_NETDEV_INGRESS:
			return "ingress";
		}
		break;
	}
	return "unknown";
}

EXPORT_SYMBOL(nftnl_flowtable_parse);
int nftnl_flowtable_parse(struct nftnl_flowtable *c, enum nftnl_parse_type type,
			  const char *data, struct nftnl_parse_err *err)
{
	errno = EOPNOTSUPP;
	return -1;
}

EXPORT_SYMBOL(nftnl_flowtable_parse_file);
int nftnl_flowtable_parse_file(struct nftnl_flowtable *c,
			       enum nftnl_parse_type type,
			       FILE *fp, struct nftnl_parse_err *err)
{
	errno = EOPNOTSUPP;
	return -1;
}

static int nftnl_flowtable_snprintf_default(char *buf, size_t remain,
					    const struct nftnl_flowtable *c)
{
	int ret, offset = 0, i;
	const char *dev;

	ret = snprintf(buf, remain, "flow table %s %s use %u size %u flags %x",
		       c->table, c->name, c->use, c->size, c->ft_flags);
	SNPRINTF_BUFFER_SIZE(ret, remain, offset);

	if (c->flags & (1 << NFTNL_FLOWTABLE_HOOKNUM)) {
		ret = snprintf(buf + offset, remain, " hook %s prio %d ",
			       nftnl_hooknum2str(c->family, c->hooknum),
			       c->prio);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);

		if (c->flags & (1 << NFTNL_FLOWTABLE_DEVICES)) {
			ret = snprintf(buf + offset, remain, " dev { ");
			SNPRINTF_BUFFER_SIZE(ret, remain, offset);

			nftnl_str_array_foreach(dev, &c->dev_array, i) {
				ret = snprintf(buf + offset, remain, " %s ",
					       dev);
				SNPRINTF_BUFFER_SIZE(ret, remain, offset);
			}
			ret = snprintf(buf + offset, remain, " } ");
			SNPRINTF_BUFFER_SIZE(ret, remain, offset);
		}
	}

	return offset;
}

static int nftnl_flowtable_cmd_snprintf(char *buf, size_t remain,
					const struct nftnl_flowtable *c,
					uint32_t cmd, uint32_t type,
					uint32_t flags)
{
	int ret, offset = 0;

	if (type != NFTNL_OUTPUT_DEFAULT)
		return -1;

	ret = nftnl_flowtable_snprintf_default(buf + offset, remain, c);
	SNPRINTF_BUFFER_SIZE(ret, remain, offset);
	return offset;
}

EXPORT_SYMBOL(nftnl_flowtable_snprintf);
int nftnl_flowtable_snprintf(char *buf, size_t size, const struct nftnl_flowtable *c,
			 uint32_t type, uint32_t flags)
{
	if (size)
		buf[0] = '\0';

	return nftnl_flowtable_cmd_snprintf(buf, size, c, nftnl_flag2cmd(flags),
					    type, flags);
}

static int nftnl_flowtable_do_snprintf(char *buf, size_t size, const void *c,
				   uint32_t cmd, uint32_t type, uint32_t flags)
{
	return nftnl_flowtable_snprintf(buf, size, c, type, flags);
}

EXPORT_SYMBOL(nftnl_flowtable_fprintf);
int nftnl_flowtable_fprintf(FILE *fp, const struct nftnl_flowtable *c,
			    uint32_t type, uint32_t flags)
{
	return nftnl_fprintf(fp, c, NFTNL_CMD_UNSPEC, type, flags,
			   nftnl_flowtable_do_snprintf);
}

struct nftnl_flowtable_list {
	struct list_head list;
};

EXPORT_SYMBOL(nftnl_flowtable_list_alloc);
struct nftnl_flowtable_list *nftnl_flowtable_list_alloc(void)
{
	struct nftnl_flowtable_list *list;

	list = calloc(1, sizeof(struct nftnl_flowtable_list));
	if (list == NULL)
		return NULL;

	INIT_LIST_HEAD(&list->list);

	return list;
}

EXPORT_SYMBOL(nftnl_flowtable_list_free);
void nftnl_flowtable_list_free(struct nftnl_flowtable_list *list)
{
	struct nftnl_flowtable *s, *tmp;

	list_for_each_entry_safe(s, tmp, &list->list, head) {
		list_del(&s->head);
		nftnl_flowtable_free(s);
	}
	xfree(list);
}

EXPORT_SYMBOL(nftnl_flowtable_list_is_empty);
int nftnl_flowtable_list_is_empty(const struct nftnl_flowtable_list *list)
{
	return list_empty(&list->list);
}

EXPORT_SYMBOL(nftnl_flowtable_list_add);
void nftnl_flowtable_list_add(struct nftnl_flowtable *s,
			      struct nftnl_flowtable_list *list)
{
	list_add(&s->head, &list->list);
}

EXPORT_SYMBOL(nftnl_flowtable_list_add_tail);
void nftnl_flowtable_list_add_tail(struct nftnl_flowtable *s,
				   struct nftnl_flowtable_list *list)
{
	list_add_tail(&s->head, &list->list);
}

EXPORT_SYMBOL(nftnl_flowtable_list_del);
void nftnl_flowtable_list_del(struct nftnl_flowtable *s)
{
	list_del(&s->head);
}

EXPORT_SYMBOL(nftnl_flowtable_list_foreach);
int nftnl_flowtable_list_foreach(struct nftnl_flowtable_list *flowtable_list,
				 int (*cb)(struct nftnl_flowtable *t, void *data), void *data)
{
	struct nftnl_flowtable *cur, *tmp;
	int ret;

	list_for_each_entry_safe(cur, tmp, &flowtable_list->list, head) {
		ret = cb(cur, data);
		if (ret < 0)
			return ret;
	}
	return 0;
}
