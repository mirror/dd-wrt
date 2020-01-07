/*
 * netlink/route/link/xfrmi.h		XFRMI interface
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2019 Eyal Birger <eyal.birger@gmail.com>
 *
 * Based on netlink/route/link/ipvti.h
 */

#ifndef NETLINK_LINK_XFRMI_H_
#define NETLINK_LINK_XFRMI_H_

#include <netlink/netlink.h>
#include <netlink/route/link.h>

#ifdef __cplusplus
extern "C" {
#endif
	extern struct rtnl_link *rtnl_link_xfrmi_alloc(void);

	extern int rtnl_link_is_xfrmi(struct rtnl_link *link);

	extern int rtnl_link_xfrmi_set_link(struct rtnl_link *link,  uint32_t index);
	extern int rtnl_link_xfrmi_get_link(struct rtnl_link *link, uint32_t *out_link);

	extern int rtnl_link_xfrmi_set_if_id(struct rtnl_link *link, uint32_t if_id);
	extern int rtnl_link_xfrmi_get_if_id(struct rtnl_link *link, uint32_t *out_if_id);

#ifdef __cplusplus
}
#endif

#endif
