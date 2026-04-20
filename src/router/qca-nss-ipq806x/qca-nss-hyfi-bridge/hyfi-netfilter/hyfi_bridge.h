/*
 * Copyright (c) 2012-2014, 2016 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef HYFI_BRIDGE_H_
#define HYFI_BRIDGE_H_

#include <br_private.h>
#include "hyfi_api.h"
#include "hyfi_netfilter.h"
#include "hyfi_seamless.h"
#include "hyfi_osdep.h"

#ifndef IPPROTO_ETHERIP
#define IPPROTO_ETHERIP (97)
#endif

#define HOMEPLUG 0x88E1
#define HYFI_AGGR_REORD_FLUSH_QUOTA 2
#define HYFI_BRIDGE_MAX 4 /* max number of hyfi bridges supported */

struct hyfi_net_bridge {
	spinlock_t lock;

	spinlock_t hash_ha_lock;
	struct hlist_head hash_ha[HA_HASH_SIZE];

	spinlock_t hash_hd_lock;
	struct hlist_head hash_hd[HD_HASH_SIZE];

	u_int32_t flags;

	// Maximum age of a H-Active entry before it should be aged out
	// (in jiffies)
	u_int32_t hatbl_aging_time;
	struct timer_list hatbl_timer;
	u_int32_t ha_entry_cnt;
	pid_t event_pid;

	struct path_switch_param path_switch_param;
	void *mc;
        unsigned char multicast_router;
	struct list_head port_list;

	struct net_device *dev;
	char linux_bridge[IFNAMSIZ]; /* Linux bridge name */
	u_int8_t TSEnabled;
	u_int8_t isController;
	char colocatedIfName[IFNAMSIZ]; /* interface name of colocated dev */
};

struct hyfi_net_bridge_port {
	u_int8_t group_num;
	u_int8_t group_type;
	u_int8_t bcast_enable;
	u_int8_t port_type;

	struct list_head list;
	struct rcu_head	rcu;
	struct net_device *dev;
};

struct hyfi_net_bridge *hyfi_bridge_get(const struct net_bridge *br);
struct hyfi_net_bridge *hyfi_bridge_get_first_br(void);
struct hyfi_net_bridge *hyfi_bridge_get_by_dev(const struct net_device *dev);
struct hyfi_net_bridge *hyfi_bridge_get_by_port(const struct net_bridge_port *port);
struct hyfi_net_bridge_port *hyfi_bridge_get_port(const struct net_bridge_port *p);
struct hyfi_net_bridge_port * hyfi_bridge_get_port_by_dev(const struct net_device *dev);

/**
 * @brief Get the Hy-Fi bridge device.  This protects us during
 *        operations where Hy-Fi may be detached asynchronously.
 *
 * @pre Must be called with rcu_read_lock
 *
 * @return Pointer to hyfi_br.dev if bridge is attached, NULL
 *         otherwise
 */
struct net_device *hyfi_bridge_dev_get_rcu(const struct hyfi_net_bridge *br);

static inline bool hyfi_brmode_relay_override(
		const struct hyfi_net_bridge *hyfi_br)
{
	return (hyfi_br->flags & HYFI_BRIDGE_FLAG_MODE_RELAY_OVERRIDE) ?
			true : false;
}

static inline bool hyfi_tcp_sp(const struct hyfi_net_bridge *hyfi_br)
{
	return (hyfi_br->flags & HYFI_BRIDGE_FLAG_MODE_TCP_SP) ? true : false;
}

/**
 * @brief Determine if the bridge is in Multicast Only forwarding mode.
 *
 * In this mode, only the IEEE1901/IEEE1905.1/LLDP/HCP multicast forwarding
 * rules are applied.
 *
 * @param [in] hyfi_br  the bridge handle
 *
 * @return true if it is in APS forwarding mode; otherwise false
 */
static inline bool hyfi_bridge_is_fwmode_aps(
		const struct hyfi_net_bridge *hyfi_br)
{
	return (hyfi_br->flags & HYFI_BRIDGE_FLAG_FWMODE_MASK) == 0 ?
			true : false;
}

/**
 * @brief Determine if the bridge is in APS forwarding mode.
 *
 * @param [in] hyfi_br  the bridge handle
 *
 * @return true if it is in APS forwarding mode; otherwise false
 */
static inline bool hyfi_bridge_is_fwmode_mcast_only(
		const struct hyfi_net_bridge *hyfi_br)
{
	return (hyfi_br->flags & HYFI_BRIDGE_FLAG_FWMODE_MCAST_ONLY) ?
			true : false;
}

static inline bool hyfi_portgrp_relay(const struct hyfi_net_bridge_port *p)
{
	return (!p || (p && p->group_type == HYFI_PORTGRP_TYPE_RELAY)) ?
			true : false;
}

static inline bool hyfi_multicast_is_router(const struct hyfi_net_bridge *hyfi_br)
{
        return (hyfi_br->multicast_router > 0);
}

static inline u_int32_t hyfi_portgrp_num(const struct hyfi_net_bridge_port *p)
{
	if (!p)
		return 0;

	return (p->group_num);
}

static inline int hyfi_bridge_portgrp_relay(struct net_bridge_port *p)
{
    return hyfi_portgrp_relay(hyfi_bridge_get_port(p));
}

static inline int hyfi_bridge_should_flood(const struct hyfi_net_bridge_port *hyfi_p,
		const struct sk_buff *skb)
{
	if (hyfi_p && hyfi_p->bcast_enable)
		return 1;

	return 0;
}

#define MAX_WDSEXT_IFACE 15
struct WdsExt_ifaces {
    char ifname[IFNAMSIZ];
};

struct WdsExt_iflist {
    u_int32_t num_entries;
    struct WdsExt_ifaces iflist[ MAX_WDSEXT_IFACE ];
};

int hyfi_bridge_get_WdsExt_iface_list(struct hyfi_net_bridge *hyfi_br, struct WdsExt_iflist *wdsExtlist);
int hyfi_bridge_dev_event(struct hyfi_net_bridge *hyfi_br, unsigned long event,
		struct net_device *dev);
int hyfi_bridge_set_bridge_name(struct hyfi_net_bridge *hyfi_br, const char *br_name);
int hyfi_bridge_init_port(struct hyfi_net_bridge *hyfi_br, struct net_bridge_port *p);
int hyfi_bridge_delete_port(struct hyfi_net_bridge *hyfi_br, struct net_bridge_port *p);
int hyfi_bridge_should_deliver(const struct hyfi_net_bridge_port *src,
		const struct hyfi_net_bridge_port *dst, const struct sk_buff *skb);
struct net_bridge_port *hyfi_bridge_get_dst(const struct net_bridge_port *src,
		struct sk_buff **skb);
struct hyfi_net_bridge * hyfi_bridge_get_hyfi_bridge(const char *br_name);
struct hyfi_net_bridge * hyfi_bridge_alloc_hyfi_bridge(const char *br_name);

int hyfi_bridge_init(void);

void hyfi_bridge_fini(void);

/*
 * The following are debug macros used throughout the Hy-Fi bridge.
 * Each file that #includes this file MUST have a:
 *
 * #define DEBUG_LEVEL X
 *
 * before the inclusion of this file.
 * X is:
 * 0 = OFF
 * 1 = ASSERTS / ERRORS
 * 2 = 1 + WARN
 * 3 = 2 + INFO
 * 4 = 3 + TRACE
 * NOTE: But X is usually provided by a -D preprocessor defined in the Makefile
 */
#if (DEBUG_LEVEL < 1)
#define DEBUG_ASSERT(s, ...)
#define DEBUG_ERROR(s, ...)
#else
#define DEBUG_ASSERT(c, s, ...) if (!(c)) { pr_emerg("ASSERT: %s:%d:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__); BUG(); }
#define DEBUG_ERROR(s, ...) pr_err("%s:%d:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if defined(CONFIG_DYNAMIC_DEBUG)
/*
 * Compile messages for dynamic enable/disable
 */
#define DEBUG_WARN(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define DEBUG_INFO(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define DEBUG_TRACE(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else

/*
 * Statically compile messages at different levels
 */
#if (DEBUG_LEVEL < 2)
#define DEBUG_WARN(s, ...)
#else
#define DEBUG_WARN(s, ...) pr_warn("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if (DEBUG_LEVEL < 3)
#define DEBUG_INFO(s, ...)
#else
#define DEBUG_INFO(s, ...) pr_notice("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if (DEBUG_LEVEL < 4)
#define DEBUG_TRACE(s, ...)
#else
#define DEBUG_TRACE(s, ...) pr_info("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif
#endif

#endif /* HYFI_BRIDGE_H_ */
