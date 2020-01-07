/*
 * netlink/route/act/gact.h	gact action
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2016 Sushma Sitaram <sushma.sitaram@intel.com>
 */

#ifndef NETLINK_GACT_H_
#define NETLINK_GACT_H_

#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/route/action.h>
#include <linux/tc_act/tc_gact.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int rtnl_gact_set_action(struct rtnl_act *act, int action);
extern int rtnl_gact_get_action(struct rtnl_act *act);

#ifdef __cplusplus
}
#endif

#endif
