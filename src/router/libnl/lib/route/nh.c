/* SPDX-License-Identifier: LGPL-2.1-only */
/*
 * Copyright (c) 2022 Stanislav Zaikin <zstaseg@gmail.com>
 */

#include "nl-default.h"

#include <linux/nexthop.h>

#include <netlink/route/nh.h>
#include <netlink/hashtable.h>
#include <netlink/route/nexthop.h>

#include "nl-aux-route/nl-route.h"
#include "nl-route.h"
#include "nl-priv-dynamic-core/nl-core.h"
#include "nl-priv-dynamic-core/cache-api.h"

/** @cond SKIP */
struct rtnl_nh {
	NLHDR_COMMON

	uint8_t nh_family;
	uint32_t nh_flags;

	uint32_t nh_id;
	uint32_t nh_group_type;
	nl_nh_group_t *nh_group;
	uint32_t nh_oif;
	struct nl_addr *nh_gateway;
};

#define NH_ATTR_FLAGS (1 << 0)
#define NH_ATTR_ID (1 << 1)
#define NH_ATTR_GROUP (1 << 2)
#define NH_ATTR_FLAG_BLACKHOLE (1 << 3)
#define NH_ATTR_OIF (1 << 4)
#define NH_ATTR_GATEWAY (1 << 5)
#define NH_ATTR_FLAG_GROUPS (1 << 6)
#define NH_ATTR_FLAG_FDB (1 << 8)
/** @endcond */

struct nla_policy rtnl_nh_policy[NHA_MAX + 1] = {
	[NHA_UNSPEC] = { .type = NLA_UNSPEC },
	[NHA_ID] = { .type = NLA_U32 },
	[NHA_GROUP] = { .type = NLA_NESTED },
	[NHA_GROUP_TYPE] = { .type = NLA_U16 },
	[NHA_BLACKHOLE] = { .type = NLA_UNSPEC },
	[NHA_OIF] = { .type = NLA_U32 },
};

static struct nl_cache_ops rtnl_nh_ops;
static struct nl_object_ops nh_obj_ops;

static nl_nh_group_t *rtnl_nh_grp_alloc(unsigned size)
{
	nl_nh_group_t *nhg;

	_nl_assert(size <= (unsigned)INT_MAX);

	if (!(nhg = calloc(1, sizeof(*nhg))))
		return NULL;

	nhg->size = size;

	if (!(nhg->entries = calloc(size, sizeof(*nhg->entries)))) {
		free(nhg);
		return NULL;
	}

	nhg->ce_refcnt = 1;

	return nhg;
}

static void rtnl_nh_grp_put(nl_nh_group_t *nhg)
{
	if (!nhg)
		return;

	_nl_assert(nhg->ce_refcnt > 0);

	nhg->ce_refcnt--;

	if (nhg->ce_refcnt > 0)
		return;

	free(nhg);
}

static int rtnh_nh_grp_cmp(const nl_nh_group_t *a, const nl_nh_group_t *b)
{
	unsigned i;

	_NL_CMP_SELF(a, b);
	_NL_CMP_DIRECT(a->size, b->size);
	for (i = 0; i < a->size; i++) {
		_NL_CMP_DIRECT(a->entries[i].nh_id, b->entries[i].nh_id);
		_NL_CMP_DIRECT(a->entries[i].weight, b->entries[i].weight);
	}
	return 0;
}

static int rtnh_nh_grp_clone(nl_nh_group_t *src, nl_nh_group_t **dst)
{
	nl_nh_group_t *ret;
	unsigned i;

	ret = rtnl_nh_grp_alloc(src->size);

	if (!ret)
		return -NLE_NOMEM;

	for (i = 0; i < src->size; i++) {
		ret->entries[i].nh_id = src->entries[i].nh_id;
		ret->entries[i].weight = src->entries[i].weight;
	}

	*dst = ret;

	return NLE_SUCCESS;
}

struct rtnl_nh *rtnl_nh_alloc(void)
{
	return (struct rtnl_nh *)nl_object_alloc(&nh_obj_ops);
}

static int nh_clone(struct nl_object *_src, struct nl_object *_dst)
{
	struct rtnl_nh *dst = nl_object_priv(_dst);
	struct rtnl_nh *src = nl_object_priv(_src);

	dst->nh_flags = src->nh_flags;
	dst->nh_family = src->nh_family;
	dst->nh_id = src->nh_id;
	dst->nh_oif = src->nh_oif;
	dst->ce_mask = src->ce_mask;

	if (src->nh_gateway) {
		dst->nh_gateway = nl_addr_clone(src->nh_gateway);
		if (!dst->nh_gateway) {
			return -NLE_NOMEM;
		}
	}

	if (src->nh_group) {
		if (rtnh_nh_grp_clone(src->nh_group, &dst->nh_group) < 0) {
			return -NLE_NOMEM;
		}
	}

	return 0;
}

static void nh_free(struct nl_object *obj)
{
	struct rtnl_nh *nh = nl_object_priv(obj);
	nl_addr_put(nh->nh_gateway);

	if (nh->nh_group)
		rtnl_nh_grp_put(nh->nh_group);
}

void rtnl_nh_put(struct rtnl_nh *nh)
{
	struct nl_object *obj = (struct nl_object *)nh;

	nl_object_put(obj);
}

static void nexthop_keygen(struct nl_object *obj, uint32_t *hashkey,
			   uint32_t table_sz)
{
	struct rtnl_nh *nexthop = nl_object_priv(obj);
	unsigned int lkey_sz;
	struct nexthop_hash_key {
		uint32_t nh_id;
	} __attribute__((packed)) lkey;

	lkey_sz = sizeof(lkey);
	lkey.nh_id = nexthop->nh_id;

	*hashkey = nl_hash(&lkey, lkey_sz, 0) % table_sz;

	return;
}

int rtnl_nh_set_gateway(struct rtnl_nh *nexthop, struct nl_addr *addr)
{
	if (nexthop->ce_mask & NH_ATTR_GATEWAY) {
		nl_addr_put(nexthop->nh_gateway);
	}

	nexthop->nh_gateway = nl_addr_clone(addr);
	nexthop->ce_mask |= NH_ATTR_GATEWAY;

	return 0;
}

struct nl_addr *rtnl_nh_get_gateway(struct rtnl_nh *nexthop)
{
	return nexthop->nh_gateway;
}

int rtnl_nh_set_fdb(struct rtnl_nh *nexthop, int value)
{
	if (value)
		nexthop->ce_mask |= NH_ATTR_FLAG_FDB;
	else
		nexthop->ce_mask &= ~NH_ATTR_FLAG_FDB;

	return 0;
}

int rtnl_nh_get_oif(struct rtnl_nh *nexthop)
{
	if (nexthop->ce_mask & NH_ATTR_OIF)
		return nexthop->nh_oif;

	return 0;
}

int rtnl_nh_get_fdb(struct rtnl_nh *nexthop)
{
	return nexthop->ce_mask & NH_ATTR_FLAG_FDB;
}

int rtnl_nh_get_group_entry(struct rtnl_nh *nexthop, int n)
{
	if (!(nexthop->ce_mask & NH_ATTR_GROUP) || !nexthop->nh_group)
		return -NLE_MISSING_ATTR;

	if (n < 0 || ((unsigned)n) >= nexthop->nh_group->size)
		return -NLE_INVAL;

	return nexthop->nh_group->entries[n].nh_id;
}

int rtnl_nh_get_group_size(struct rtnl_nh *nexthop)
{
	if (!(nexthop->ce_mask & NH_ATTR_GROUP) || !nexthop->nh_group)
		return -NLE_MISSING_ATTR;

	_nl_assert(nexthop->nh_group->size <= INT_MAX);

	return (int)nexthop->nh_group->size;
}

static int rtnl_nh_grp_info(unsigned size, const struct nexthop_grp *vi,
			    nl_nh_group_t **nvi)
{
	nl_nh_group_t *ret;
	unsigned i;

	if (!(ret = rtnl_nh_grp_alloc(size)))
		return -NLE_NOMEM;

	for (i = 0; i < size; i++) {
		ret->entries[i].nh_id = vi[i].id;
		ret->entries[i].weight = vi[i].weight;
	}

	*nvi = ret;
	return NLE_SUCCESS;
}

int rtnl_nh_get_id(struct rtnl_nh *nh)
{
	if (nh->ce_mask & NH_ATTR_ID)
		return nh->nh_id;

	return -NLE_INVAL;
}

static int nexthop_msg_parser(struct nl_cache_ops *ops, struct sockaddr_nl *who,
			      struct nlmsghdr *n, struct nl_parser_param *pp)
{
	_nl_auto_rtnl_nh struct rtnl_nh *nexthop = NULL;
	struct nhmsg *ifi;
	struct nlattr *tb[NHA_MAX + 1];
	int err;
	int family;

	nexthop = rtnl_nh_alloc();
	if (nexthop == NULL)
		return -NLE_NOMEM;

	nexthop->ce_msgtype = n->nlmsg_type;

	if (!nlmsg_valid_hdr(n, sizeof(*ifi)))
		return -NLE_MSG_TOOSHORT;

	ifi = nlmsg_data(n);
	family = ifi->nh_family;
	nexthop->nh_family = family;
	nexthop->nh_flags = ifi->nh_flags;
	nexthop->ce_mask = (NH_ATTR_FLAGS);

	err = nlmsg_parse(n, sizeof(*ifi), tb, NHA_MAX, rtnl_nh_policy);
	if (err < 0)
		return err;

	if (tb[NHA_ID]) {
		nexthop->nh_id = nla_get_u32(tb[NHA_ID]);
		nexthop->ce_mask |= NH_ATTR_ID;
	}

	if (tb[NHA_OIF]) {
		nexthop->nh_oif = nla_get_u32(tb[NHA_OIF]);
		nexthop->ce_mask |= NH_ATTR_OIF;
	}

	if (tb[NHA_GATEWAY]) {
		nexthop->nh_gateway =
			nl_addr_alloc_attr(tb[NHA_GATEWAY], family);
		nexthop->ce_mask |= NH_ATTR_GATEWAY;
	}

	if (tb[NHA_BLACKHOLE]) {
		nexthop->ce_mask |= NH_ATTR_FLAG_BLACKHOLE;
	}

	if (tb[NHA_GROUPS]) {
		nexthop->ce_mask |= NH_ATTR_FLAG_GROUPS;
	}

	if (tb[NHA_FDB]) {
		nexthop->ce_mask |= NH_ATTR_FLAG_FDB;
	}

	if (tb[NHA_GROUP]) {
		nl_nh_group_t *nh_group = NULL;
		const void *data;
		unsigned size;
		unsigned len;

		data = nla_data(tb[NHA_GROUP]);
		len = nla_len(tb[NHA_GROUP]);
		size = len / sizeof(struct nexthop_grp);

		err = rtnl_nh_grp_info(size, (const struct nexthop_grp *)data,
				       &nh_group);
		if (err < 0) {
			return err;
		}

		nexthop->nh_group = nh_group;
		nexthop->ce_mask |= NH_ATTR_GROUP;
	}

	return pp->pp_cb((struct nl_object *)nexthop, pp);
}

static int nexthop_request_update(struct nl_cache *cache, struct nl_sock *sk)
{
	_nl_auto_nl_msg struct nl_msg *msg = NULL;
	int family = cache->c_iarg1;
	struct nhmsg hdr = { .nh_family = family };
	int err;

	msg = nlmsg_alloc_simple(RTM_GETNEXTHOP, NLM_F_DUMP);
	if (!msg)
		return -NLE_NOMEM;

	if (nlmsg_append(msg, &hdr, sizeof(hdr), NLMSG_ALIGNTO) < 0)
		return -NLE_MSGSIZE;

	err = nl_send_auto(sk, msg);
	if (err < 0)
		return err;

	return NLE_SUCCESS;
}

static void dump_nh_group(nl_nh_group_t *group, struct nl_dump_params *dp)
{
	unsigned i;

	nl_dump(dp, " nh_grp:");
	for (i = 0; i < group->size; i++) {
		nl_dump(dp, " %u", group->entries[i].nh_id);
	}
}

static void nh_dump_line(struct nl_object *obj, struct nl_dump_params *dp)
{
	struct nl_cache *cache;
	char buf[128];
	struct rtnl_nh *nh = nl_object_priv(obj);

	cache = nl_cache_mngt_require_safe("route/nh");

	if (nh->ce_mask & NH_ATTR_ID)
		nl_dump(dp, "nhid %u", nh->nh_id);

	if (nh->ce_mask & NH_ATTR_OIF)
		nl_dump(dp, " oif %d", nh->nh_oif);

	if (nh->ce_mask & NH_ATTR_GATEWAY)
		nl_dump(dp, " via %s",
			nl_addr2str(nh->nh_gateway, buf, sizeof(buf)));

	if (nh->ce_mask & NH_ATTR_FLAG_BLACKHOLE)
		nl_dump(dp, " blackhole");

	if (nh->ce_mask & NH_ATTR_FLAG_GROUPS)
		nl_dump(dp, " groups");

	if (nh->ce_mask & NH_ATTR_GROUP)
		dump_nh_group(nh->nh_group, dp);

	if (nh->ce_mask & NH_ATTR_FLAG_FDB)
		nl_dump(dp, " fdb");

	nl_dump(dp, "\n");

	if (cache)
		nl_cache_put(cache);
}

static void nh_dump_details(struct nl_object *nh, struct nl_dump_params *dp)
{
	nh_dump_line(nh, dp);
}

static uint64_t nh_compare(struct nl_object *a, struct nl_object *b,
			   uint64_t attrs, int loose)
{
	int diff = 0;
	struct rtnl_nh *src = nl_object_priv(a);
	struct rtnl_nh *dst = nl_object_priv(b);

#define _DIFF(ATTR, EXPR) ATTR_DIFF(attrs, ATTR, a, b, EXPR)
	diff |= _DIFF(NH_ATTR_ID, src->nh_id != dst->nh_id);
	diff |= _DIFF(NH_ATTR_GATEWAY,
		      nl_addr_cmp(src->nh_gateway, dst->nh_gateway));
	diff |= _DIFF(NH_ATTR_OIF, src->nh_oif != dst->nh_oif);
	diff |= _DIFF(NH_ATTR_GROUP,
		      rtnh_nh_grp_cmp(src->nh_group, dst->nh_group));
	diff |= _DIFF(NH_ATTR_FLAG_FDB, false);
	diff |= _DIFF(NH_ATTR_FLAG_GROUPS, false);
	diff |= _DIFF(NH_ATTR_FLAG_BLACKHOLE, false);
#undef _DIFF

	return diff;
}

struct rtnl_nh *rtnl_nh_get(struct nl_cache *cache, int nhid)
{
	struct rtnl_nh *nh;

	if (cache->c_ops != &rtnl_nh_ops)
		return NULL;

	nl_list_for_each_entry(nh, &cache->c_items, ce_list) {
		if (nh->nh_id == nhid) {
			nl_object_get((struct nl_object *)nh);
			return nh;
		}
	}

	return NULL;
}

/**
 * Allocate nexthop cache and fill in all configured nexthops.
 * @arg sk		Netnexthop socket.
 * @arg family		nexthop address family or AF_UNSPEC
 * @arg result		Pointer to store resulting cache.
 * @arg flags		Flags to set in nexthop cache before filling
 *
 * Allocates and initializes a new nexthop cache. If \c sk is valid, a netnexthop
 * message is sent to the kernel requesting a full dump of all configured
 * nexthops. The returned messages are parsed and filled into the cache. If
 * the operation succeeds, the resulting cache will contain a nexthop object for
 * each nexthop configured in the kernel. If \c sk is NULL, returns 0 but the
 * cache is still empty.
 *
 * If \c family is set to an address family other than \c AF_UNSPEC the
 * contents of the cache can be limited to a specific address family.
 * Currently the following address families are supported:
 * - AF_BRIDGE
 * - AF_INET6
 *
 * @route_doc{nexthop_list, Get List of nexthops}
 * @see rtnl_nh_get()
 * @see rtnl_nh_get_by_name()
 * @return 0 on success or a negative error code.
 */
static int rtnl_nh_alloc_cache_flags(struct nl_sock *sk, int family,
				     struct nl_cache **result,
				     unsigned int flags)
{
	struct nl_cache *cache;
	int err;

	cache = nl_cache_alloc(&rtnl_nh_ops);
	if (!cache)
		return -NLE_NOMEM;

	cache->c_iarg1 = family;

	if (flags)
		nl_cache_set_flags(cache, flags);

	if (sk && (err = nl_cache_refill(sk, cache)) < 0) {
		nl_cache_free(cache);
		return err;
	}

	*result = cache;
	return 0;
}

/**
 * Allocate nexthop cache and fill in all configured nexthops.
 * @arg sk		Netnexthop socket.
 * @arg family		nexthop address family or AF_UNSPEC
 * @arg result		Pointer to store resulting cache.
 *
 * Allocates and initializes a new nexthop cache. If \c sk is valid, a netnexthop
 * message is sent to the kernel requesting a full dump of all configured
 * nexthops. The returned messages are parsed and filled into the cache. If
 * the operation succeeds, the resulting cache will contain a nexthop object for
 * each nexthop configured in the kernel. If \c sk is NULL, returns 0 but the
 * cache is still empty.
 *
 * If \c family is set to an address family other than \c AF_UNSPEC the
 * contents of the cache can be limited to a specific address family.
 * Currently the following address families are supported:
 * - AF_BRIDGE
 * - AF_INET6
 *
 * @route_doc{nexthop_list, Get List of nexthops}
 * @see rtnl_nh_get()
 * @see rtnl_nh_get_by_name()
 * @return 0 on success or a negative error code.
 */
int rtnl_nh_alloc_cache(struct nl_sock *sk, int family,
			struct nl_cache **result)
{
	return rtnl_nh_alloc_cache_flags(sk, family, result, 0);
}

static struct nl_object_ops nh_obj_ops = {
  .oo_name		= "route/nh",
  .oo_size		= sizeof(struct rtnl_nh),
  .oo_free_data		= nh_free,
  .oo_clone		= nh_clone,
  .oo_dump = {
      [NL_DUMP_LINE]	= nh_dump_line,
      [NL_DUMP_DETAILS]	= nh_dump_details,
  },
  .oo_compare		= nh_compare,
  .oo_keygen		= nexthop_keygen,
  .oo_attrs2str		= rtnl_route_nh_flags2str,
  .oo_id_attrs		= NH_ATTR_ID,
};

static struct nl_af_group nh_groups[] = {
	{ AF_UNSPEC, RTNLGRP_NEXTHOP },
	{ END_OF_GROUP_LIST },
};

static struct nl_cache_ops rtnl_nh_ops = {
  .co_name		= "route/nh",
  .co_hdrsize		= sizeof(struct nhmsg),
  .co_msgtypes		= {
          { RTM_NEWNEXTHOP, NL_ACT_NEW, "new" },
          { RTM_DELNEXTHOP, NL_ACT_DEL, "del" },
          { RTM_GETNEXTHOP, NL_ACT_GET, "get" },
          END_OF_MSGTYPES_LIST,
          },
  .co_protocol  = NETLINK_ROUTE,
  .co_groups		= nh_groups,
  .co_request_update	= nexthop_request_update,
  .co_msg_parser		= nexthop_msg_parser,
  .co_obj_ops		= &nh_obj_ops,
};

static void _nl_init nexthop_init(void)
{
	nl_cache_mngt_register(&rtnl_nh_ops);
}

static void _nl_exit nexthop_exit(void)
{
	nl_cache_mngt_unregister(&rtnl_nh_ops);
}
