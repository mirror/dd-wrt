/* SPDX-License-Identifier: LGPL-2.1-only */
/*
 * Copyright (c) 2022 MaxLinear, Inc.
 */

#ifndef NETLINK_LINK_BRIDGE_INFO_H_
#define NETLINK_LINK_BRIDGE_INFO_H_

#include <netlink/netlink.h>
#include <netlink/route/link.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void rtnl_link_bridge_set_vlan_filtering(struct rtnl_link *link,
						uint8_t vlan_filtering);
extern int rtnl_link_bridge_get_vlan_filtering(struct rtnl_link *link,
					       uint8_t *vlan_filtering);

extern void rtnl_link_bridge_set_vlan_protocol(struct rtnl_link *link,
					       uint16_t vlan_protocol);
extern int rtnl_link_bridge_get_vlan_protocol(struct rtnl_link *link,
					      uint16_t *vlan_protocol);

extern void rtnl_link_bridge_set_vlan_stats_enabled(struct rtnl_link *link,
						    uint8_t vlan_stats_enabled);
extern int rtnl_link_bridge_get_vlan_stats_enabled(struct rtnl_link *link,
						   uint8_t *vlan_stats_enabled);

#ifdef __cplusplus
}
#endif

#endif
