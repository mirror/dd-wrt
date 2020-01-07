/*
 * netlink/route/netconf.h		rtnetlink netconf layer
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2017 David Ahern <dsa@cumulusnetworks.com>
 */

#ifndef NETCONF_H_
#define NETCONF_H_

#ifdef __cplusplus
extern "C" {
#endif

struct rtnl_netconf;

int rtnl_netconf_alloc_cache(struct nl_sock *sk, struct nl_cache **result);

struct rtnl_netconf *rtnl_netconf_get_by_idx(struct nl_cache *cache, int family,
					     int ifindex);
struct rtnl_netconf *rtnl_netconf_get_all(struct nl_cache *cache,
					  int family);
struct rtnl_netconf *rtnl_netconf_get_default(struct nl_cache *cache,
					      int family);
void rtnl_netconf_put(struct rtnl_netconf *nc);

int rtnl_netconf_get_family(struct rtnl_netconf *nc, int *val);
int rtnl_netconf_get_ifindex(struct rtnl_netconf *nc, int *val);
int rtnl_netconf_get_forwarding(struct rtnl_netconf *nc, int *val);
int rtnl_netconf_get_mc_forwarding(struct rtnl_netconf *nc, int *val);
int rtnl_netconf_get_rp_filter(struct rtnl_netconf *nc, int *val);
int rtnl_netconf_get_proxy_neigh(struct rtnl_netconf *nc, int *val);
int rtnl_netconf_get_ignore_routes_linkdown(struct rtnl_netconf *nc, int *val);
int rtnl_netconf_get_input(struct rtnl_netconf *nc, int *val);

#ifdef __cplusplus
}
#endif

#endif
