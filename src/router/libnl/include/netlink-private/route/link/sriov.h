/*
 * include/netlink-private/route/link/sriov.h      SRIOV VF Info
 *
 *     This library is free software; you can redistribute it and/or
 *     modify it under the terms of the GNU Lesser General Public
 *     License as published by the Free Software Foundation version 2.1
 *     of the License.
 *
 * Copyright (c) 2016 Intel Corp. All rights reserved.
 * Copyright (c) 2016 Jef Oliver <jef.oliver@intel.com>
 */

#ifndef NETLINK_PRIV_LINK_SRIOV_H_
#define NETLINK_PRIV_LINK_SRIOV_H_

#include <netlink/netlink.h>
#include <netlink/route/link/sriov.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int rtnl_link_sriov_clone(struct rtnl_link *, struct rtnl_link *);
extern void rtnl_link_sriov_dump_details(struct rtnl_link *, struct nl_dump_params *);
extern void rtnl_link_sriov_dump_stats(struct rtnl_link *, struct nl_dump_params *);
extern int rtnl_link_sriov_fill_vflist(struct nl_msg *, struct rtnl_link *);
extern void rtnl_link_sriov_free_data(struct rtnl_link *);
extern int rtnl_link_sriov_parse_vflist(struct rtnl_link *, struct nlattr **);

#ifdef __cplusplus
}
#endif

#endif
