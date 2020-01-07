/*
 * netlink/route/link/ppp.h		PPP Interface
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2016 Jonas Johansson <jonasj76@gmail.com>
 */

#ifndef NETLINK_LINK_PPP_H_
#define NETLINK_LINK_PPP_H_

#include <netlink/netlink.h>
#include <netlink/route/link.h>

#ifdef __cplusplus
extern "C" {
#endif

extern struct rtnl_link	*rtnl_link_ppp_alloc(void);
extern int		rtnl_link_ppp_set_fd(struct rtnl_link *, int32_t);
extern int		rtnl_link_ppp_get_fd(struct rtnl_link *, int32_t *);

#ifdef __cplusplus
}
#endif

#endif
