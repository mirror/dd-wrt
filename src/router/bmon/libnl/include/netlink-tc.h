/*
 * netlink-tc.h		Local Traffic Control Interface
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_TC_PRIV_H_
#define NETLINK_TC_PRIV_H_

#include <netlink-local.h>

#define TCA_ATTR_HANDLE    0x001
#define TCA_ATTR_PARENT    0x002
#define TCA_ATTR_IFINDEX   0x004
#define TCA_ATTR_KIND      0x008
#define TCA_ATTR_FAMILY    0x010
#define TCA_ATTR_INFO      0x020
#define TCA_ATTR_OPTS      0x040
#define TCA_ATTR_STATS     0x080
#define TCA_ATTR_XSTATS    0x100
#define TCA_ATTR_MAX	  TCA_ATTR_XSTATS

extern int tca_parse(struct nlattr **, int, struct rtnl_tca *,
		     struct nla_policy *);
extern int  tca_msg_parser(struct nlmsghdr *, struct rtnl_tca *);
extern void tca_free_data(struct rtnl_tca *);
extern int  tca_dump_brief(struct rtnl_tca *, const char *,
			   struct nl_dump_params *, int);
extern int  tca_dump_full(struct rtnl_tca *, struct nl_dump_params *, int);
extern int  tca_dump_stats(struct rtnl_tca *,
				struct nl_dump_params *, int);
extern int  tca_filter(struct rtnl_tca *, struct rtnl_tca *);

extern void tca_set_ifindex(struct rtnl_tca *, int);
extern int  tca_get_ifindex(struct rtnl_tca *);
extern void tca_set_handle(struct rtnl_tca *, uint32_t);
extern uint32_t tca_get_handle(struct rtnl_tca *);
extern void tca_set_parent(struct rtnl_tca *, uint32_t);
extern uint32_t tca_get_parent(struct rtnl_tca *);
extern void tca_set_kind(struct rtnl_tca *, const char *);
extern char *tca_get_kind(struct rtnl_tca *);
extern uint64_t tca_get_stat(struct rtnl_tca *, int );

extern struct nl_msg *tca_build_msg(struct rtnl_tca *tca, int type, int flags);

static inline void *tca_priv(struct rtnl_tca *tca)
{
	return tca->tc_subdata;
}

static inline void *tca_xstats(struct rtnl_tca *tca)
{
	return tca->tc_xstats->d_data;
}

#endif
