/*
 * route.h            libnl routing module
 *
 * Copyright (c) 2003-2005 Thomas Graf <tgraf@suug.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 *
 */

#ifndef NETLINK_ROUTE_H_
#define NETLINK_ROUTE_H_

#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/addr.h>
#include <netlink/data.h>

/**
 * Available link mask flags (link.l_mask)
 */

#define ROUTE_HAS_FAMILY    0x000001
#define ROUTE_HAS_DST_LEN   0x000002
#define ROUTE_HAS_SRC_LEN   0x000004
#define ROUTE_HAS_TOS       0x000008
#define ROUTE_HAS_TABLE     0x000010
#define ROUTE_HAS_PROTOCOL  0x000020
#define ROUTE_HAS_SCOPE     0x000040
#define ROUTE_HAS_TYPE      0x000080
#define ROUTE_HAS_FLAGS     0x000100
#define ROUTE_HAS_DST       0x000200
#define ROUTE_HAS_SRC       0x000400
#define ROUTE_HAS_IIF       0x000800
#define ROUTE_HAS_OIF       0x001000
#define ROUTE_HAS_GATEWAY   0x002000
#define ROUTE_HAS_PRIO      0x004000
#define ROUTE_HAS_PREF_SRC  0x008000
#define ROUTE_HAS_METRICS   0x010000
#define ROUTE_HAS_MULTIPATH 0x020000
#define ROUTE_HAS_REALM     0x040000
#define ROUTE_HAS_CACHEINFO 0x080000
#define ROUTE_HAS_SESSION   0x100000
#define ROUTE_HAS_PROTOINFO 0x200000

#define NEXTHOP_HAS_FLAGS   0x000001
#define NEXTHOP_HAS_HOPS    0x000002
#define NEXTHOP_HAS_IFINDEX 0x000004
#define NEXTHOP_HAS_GATEWAY 0x000008

struct rtnl_nexthop
{
	uint8_t		rtnh_flags;
	uint8_t		rtnh_hops;
	/* 2 bytes spare */
	uint32_t	rtnh_ifindex;
	struct nl_addr	rtnh_gateway;
	uint32_t	rtnh_mask;

	struct rtnl_nexthop *rtnh_next;
};

/**
 * Routing Caching Information
 * @ingroup route
 */
struct rtnl_rtcacheinfo
{
	uint32_t	rtci_clntref;
	uint32_t	rtci_last_use;
	uint32_t	rtci_expires;
	uint32_t	rtci_error;
	uint32_t	rtci_used;
	uint32_t	rtci_id;
	uint32_t	rtci_ts;
	uint32_t	rtci_tsage;
};

/**
 * Route
 * @ingroup route
 */
struct rtnl_route
{
	/** Common header required by cache */
	NLHDR_COMMON

	uint8_t			rt_family;
	uint8_t			rt_dst_len;
	uint8_t			rt_src_len;
	uint8_t			rt_tos;
	uint8_t			rt_table;
	uint8_t			rt_protocol;
	uint8_t			rt_scope;
	uint8_t			rt_type;
	uint32_t		rt_flags;
	struct nl_addr		rt_dst;
	struct nl_addr		rt_src;
	char			rt_iif[IFNAMSIZ+1];
	uint32_t		rt_oif;
	struct nl_addr		rt_gateway;
	uint32_t		rt_prio;
	uint32_t		rt_metrics[RTAX_MAX];
	uint32_t		rt_metrics_mask;
	struct nl_addr		rt_pref_src;
	struct rtnl_nexthop *	rt_nexthops;
	uint32_t		rt_realm;
	struct rtnl_rtcacheinfo	rt_cacheinfo;
	struct nl_data		rt_session;
	struct nl_data		rt_protoinfo;
	uint32_t		rt_flag_mask;
	uint32_t		rt_mask;
};


extern struct nl_cache_ops rtnl_route_ops;

/**
 * Initialize a routing cache structure.
 * @ingroup route
 * @code
 * struct nl_cache cache = RTNL_INIT_ROUTE_CACHE();
 * @endcode
 */
#define RTNL_INIT_ROUTE_CACHE() {              \
    .c_type = RTNL_ROUTE,                      \
    .c_type_size = sizeof(struct rtnl_route),  \
    .c_ops = &rtnl_route_ops,                  \
}


/**
 * Initialize a route strcuture.
 * @ingroup route
 * @code
 * struct rtnl_route r = RTNL_INIT_ROUTE();
 * @endcode
 */
#define RTNL_INIT_ROUTE() {                    \
    .ce_type = RTNL_ROUTE,                     \
    .ce_size = sizeof(struct rtnl_route),      \
}

extern char *	rtnl_scope2str_r(int, char *, size_t);
extern char *	rtnl_scope2str(int);
extern int	rtnl_str2scope(const char *);

extern void	rtnl_route_set_table(struct rtnl_route *, int);
extern void	rtnl_route_set_scope(struct rtnl_route *, int);
extern void	rtnl_route_set_tos(struct rtnl_route *r, int);
extern void	rtnl_route_set_realm(struct rtnl_route *, int);
extern void	rtnl_route_set_protocol(struct rtnl_route *, int);
extern void	rtnl_route_set_prio(struct rtnl_route *, int);
extern void	rtnl_route_set_family(struct rtnl_route *, int);
extern void	rtnl_route_set_type(struct rtnl_route *, int);
extern void	rtnl_route_set_dst_len(struct rtnl_route *, int);
extern void	rtnl_route_set_src_len(struct rtnl_route *, int);
extern void	rtnl_route_set_flags(struct rtnl_route *, int);
extern void	rtnl_route_unset_flags(struct rtnl_route *, int);
extern int	rtnl_route_set_metric(struct rtnl_route *, int, uint32_t);
extern int	rtnl_route_unset_metric(struct rtnl_route *, int);
extern int	rtnl_route_set_dst(struct rtnl_route *, struct nl_addr *);
extern int	rtnl_route_set_dst_str(struct rtnl_route *, const char *);
extern int	rtnl_route_set_src(struct rtnl_route *, struct nl_addr *);
extern int	rtnl_route_set_src_str(struct rtnl_route *, const char *);
extern int	rtnl_route_set_gateway(struct rtnl_route *, struct nl_addr *);
extern int	rtnl_route_set_gateway_str(struct rtnl_route *, const char *);
extern int	rtnl_route_set_pref_src(struct rtnl_route *, struct nl_addr *);
extern int	rtnl_route_set_pref_src_str(struct rtnl_route *, const char *);
extern void	rtnl_route_set_oif(struct rtnl_route *, int);
extern int	rtnl_route_set_oif_name(struct rtnl_route *, struct nl_cache *,
					const char *);
extern void	rtnl_route_set_iif(struct rtnl_route *, const char *);

#endif
