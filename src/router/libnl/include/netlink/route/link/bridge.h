/* SPDX-License-Identifier: LGPL-2.1-only */
/*
 * Copyright (c) 2013 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_LINK_BRIDGE_H_
#define NETLINK_LINK_BRIDGE_H_

#include <netlink/netlink.h>
#include <netlink/route/link.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RTNL_LINK_BRIDGE_VLAN_BITMAP_MAX 4096
#define RTNL_LINK_BRIDGE_VLAN_BITMAP_LEN (RTNL_LINK_BRIDGE_VLAN_BITMAP_MAX / 32)

struct rtnl_link_bridge_vlan
{
	uint16_t                pvid;
	uint32_t                vlan_bitmap[RTNL_LINK_BRIDGE_VLAN_BITMAP_LEN];
	uint32_t                untagged_bitmap[RTNL_LINK_BRIDGE_VLAN_BITMAP_LEN];
};

/**
 * Bridge flags
 * @ingroup bridge
 */
enum rtnl_link_bridge_flags {
	RTNL_BRIDGE_HAIRPIN_MODE	= 0x0001,
	RTNL_BRIDGE_BPDU_GUARD		= 0x0002,
	RTNL_BRIDGE_ROOT_BLOCK		= 0x0004,
	RTNL_BRIDGE_FAST_LEAVE		= 0x0008,
	RTNL_BRIDGE_UNICAST_FLOOD	= 0x0010,
	RTNL_BRIDGE_LEARNING		= 0x0020,
	RTNL_BRIDGE_LEARNING_SYNC	= 0x0040,
};

#define RTNL_BRIDGE_HWMODE_VEB BRIDGE_MODE_VEB
#define RTNL_BRIDGE_HWMODE_VEPA BRIDGE_MODE_VEPA
#define RTNL_BRIDGE_HWMODE_MAX BRIDGE_MODE_VEPA
#define RTNL_BRIDGE_HWMODE_UNDEF BRIDGE_MODE_UNDEF

extern struct rtnl_link *rtnl_link_bridge_alloc(void);

extern int	rtnl_link_is_bridge(struct rtnl_link *);
extern int	rtnl_link_bridge_has_ext_info(struct rtnl_link *);

extern int	rtnl_link_bridge_set_port_state(struct rtnl_link *, uint8_t );
extern int	rtnl_link_bridge_get_port_state(struct rtnl_link *);

extern int	rtnl_link_bridge_set_priority(struct rtnl_link *, uint16_t);
extern int	rtnl_link_bridge_get_priority(struct rtnl_link *);

extern int	rtnl_link_bridge_set_cost(struct rtnl_link *, uint32_t);
extern int	rtnl_link_bridge_get_cost(struct rtnl_link *, uint32_t *);

extern int	rtnl_link_bridge_unset_flags(struct rtnl_link *, unsigned int);
extern int	rtnl_link_bridge_set_flags(struct rtnl_link *, unsigned int);
extern int	rtnl_link_bridge_get_flags(struct rtnl_link *);

extern int	rtnl_link_bridge_set_self(struct rtnl_link *);

extern int	rtnl_link_bridge_get_hwmode(struct rtnl_link *, uint16_t *);
extern int	rtnl_link_bridge_set_hwmode(struct rtnl_link *, uint16_t);

extern char * rtnl_link_bridge_flags2str(int, char *, size_t);
extern int	rtnl_link_bridge_str2flags(const char *);

extern char * rtnl_link_bridge_portstate2str(int, char *, size_t);
extern int  rtnl_link_bridge_str2portstate(const char *);

extern char * rtnl_link_bridge_hwmode2str(uint16_t, char *, size_t);
extern uint16_t rtnl_link_bridge_str2hwmode(const char *);

extern int	rtnl_link_bridge_add(struct nl_sock *sk, const char *name);

extern int	rtnl_link_bridge_pvid(struct rtnl_link *link);
extern int	rtnl_link_bridge_has_vlan(struct rtnl_link *link);

extern struct rtnl_link_bridge_vlan *rtnl_link_bridge_get_port_vlan(struct rtnl_link *link);
#ifdef __cplusplus
}
#endif

#endif

