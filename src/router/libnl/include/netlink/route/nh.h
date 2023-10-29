/* SPDX-License-Identifier: LGPL-2.1-only */
/*
 * Copyright (c) 2022 Stanislav Zaikin <zstaseg@gmail.com>
 */

#ifndef NETLINK_ROUTE_NEXTHOP2_H_
#define NETLINK_ROUTE_NEXTHOP2_H_

#include <netlink/netlink.h>
#include <netlink/addr.h>

#ifdef __cplusplus
extern "C" {
#endif

struct rtnl_nh;

typedef struct nl_nh_group_info {
	uint32_t nh_id; /*!< nexthop id */
	uint8_t weight; /*!< weight in nexthop group */
} nl_nh_group_info_t;

typedef struct nl_nh_group {
	int ce_refcnt;
	unsigned size;
	nl_nh_group_info_t *entries;
} nl_nh_group_t;

extern int rtnl_nh_alloc_cache(struct nl_sock *sk, int family,
			       struct nl_cache **result);
extern struct rtnl_nh *rtnl_nh_alloc(void);
extern void rtnl_nh_put(struct rtnl_nh *);

extern struct rtnl_nh *rtnl_nh_get(struct nl_cache *cache, int nhid);

extern int rtnl_nh_set_gateway(struct rtnl_nh *, struct nl_addr *);
extern struct nl_addr *rtnl_nh_get_gateway(struct rtnl_nh *);

extern int rtnl_nh_set_fdb(struct rtnl_nh *, int value);
extern int rtnl_nh_get_fdb(struct rtnl_nh *);

extern int rtnl_nh_get_group_entry(struct rtnl_nh *, int n);
extern int rtnl_nh_get_group_size(struct rtnl_nh *);

extern int rtnl_nh_get_id(struct rtnl_nh *);
extern int rtnl_nh_get_oif(struct rtnl_nh *);

#ifdef __cplusplus
}
#endif

#endif
