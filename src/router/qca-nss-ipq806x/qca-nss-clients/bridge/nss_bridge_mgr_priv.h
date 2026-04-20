/*
 **************************************************************************
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

#ifndef _NSS_BRIDGE_MGR_PRIV_H_
#define _NSS_BRIDGE_MGR_PRIV_H_

#if (NSS_BRIDGE_MGR_DEBUG_LEVEL < 1)
#define nss_bridge_mgr_assert(fmt, args...)
#else
#define nss_bridge_mgr_assert(c) BUG_ON(!(c))
#endif /* NSS_BRIDGE_MGR_DEBUG_LEVEL */

/*
 * Compile messages for dynamic enable/disable
 */
#define nss_bridge_mgr_always(s, ...) pr_alert(s, ##__VA_ARGS__)

#if defined(CONFIG_DYNAMIC_DEBUG)
#define nss_bridge_mgr_warn(s, ...) \
		pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define nss_bridge_mgr_info(s, ...) \
		pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define nss_bridge_mgr_trace(s, ...) \
		pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#else /* CONFIG_DYNAMIC_DEBUG */
/*
 * Statically compile messages at different levels
 */
#if (NSS_BRIDGE_MGR_DEBUG_LEVEL < 2)
#define nss_bridge_mgr_warn(s, ...)
#else
#define nss_bridge_mgr_warn(s, ...) \
		pr_warn("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_BRIDGE_MGR_DEBUG_LEVEL < 3)
#define nss_bridge_mgr_info(s, ...)
#else
#define nss_bridge_mgr_info(s, ...) \
		pr_notice("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_BRIDGE_MGR_DEBUG_LEVEL < 4)
#define nss_bridge_mgr_trace(s, ...)
#else
#define nss_bridge_mgr_trace(s, ...) \
		pr_info("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif
#endif /* CONFIG_DYNAMIC_DEBUG */

/*
 * nss interface check
 */
#define NSS_BRIDGE_MGR_PHY_PORT_MIN 1
#define NSS_BRIDGE_MGR_PHY_PORT_MAX 6
#define NSS_BRIDGE_MGR_IF_IS_TYPE_PHYSICAL(if_num) \
	(((if_num) >= NSS_BRIDGE_MGR_PHY_PORT_MIN) && \
	((if_num) <= NSS_BRIDGE_MGR_PHY_PORT_MAX))

#if defined(NSS_BRIDGE_MGR_PPE_SUPPORT)
#define NSS_BRIDGE_MGR_SWITCH_ID	0
#define NSS_BRIDGE_MGR_SPANNING_TREE_ID	0
#define NSS_BRIDGE_MGR_DISABLE_PPE_EXCEPTION	0
#define NSS_BRIDGE_MGR_ENABLE_PPE_EXCEPTION	1

#define NSS_BRIDGE_MGR_ACL_DEV_ID 0
#define NSS_BRIDGE_MGR_ACL_LIST_ID 61
#define NSS_BRIDGE_MGR_ACL_LIST_PRIORITY 0
#define NSS_BRIDGE_MGR_ACL_RULE_NR 1
#define NSS_BRIDGE_MGR_ACL_FRAG_RULE_ID 0
#define NSS_BRIDGE_MGR_ACL_FIN_RULE_ID 1
#define NSS_BRIDGE_MGR_ACL_SYN_RULE_ID 2
#define NSS_BRIDGE_MGR_ACL_RST_RULE_ID 3

#endif

/*
 * bridge manager context structure
 */
struct nss_bridge_mgr_context {
	struct list_head list;		/* List of bridge instance */
	spinlock_t lock;		/* Lock to protect bridge instance */
#if defined(NSS_BRIDGE_MGR_PPE_SUPPORT)
	int32_t wan_if_num;		/* WAN interface number */
	char wan_ifname[IFNAMSIZ];	/* WAN interface name */
	struct ctl_table_header *nss_bridge_mgr_header;	/* bridge sysctl */
#endif
};

/*
 * bridge manager private structure
 */
struct nss_bridge_pvt {
	struct list_head list;			/* List of bridge instance */
	struct net_device *dev;			/* Bridge netdevice */
	uint32_t ifnum;				/* Dynamic interface for bridge */
#if defined(NSS_BRIDGE_MGR_PPE_SUPPORT)
	uint32_t vsi;				/* VSI set for bridge */
	uint32_t port_vsi[NSS_BRIDGE_MGR_PHY_PORT_MAX];	/* port VSI set for physical interfaces	*/
	uint32_t lag_ports[NSS_BRIDGE_MGR_PHY_PORT_MAX];	/* List of slave ports in LAG */
	int bond_slave_num;			/* Total number of bond devices added into
						   bridge device */
	bool wan_if_enabled;			/* Is WAN interface enabled? */
	int32_t wan_if_num;			/* WAN interface number, if enabled */
#endif
	uint32_t mtu;				/* MTU for bridge */
	uint8_t dev_addr[ETH_ALEN];		/* MAC address for bridge */
};

int nss_bridge_mgr_register_br(struct net_device *dev);
int nss_bridge_mgr_unregister_br(struct net_device *dev);
struct nss_bridge_pvt *nss_bridge_mgr_find_instance(struct net_device *dev);
int nss_bridge_mgr_join_bridge(struct net_device *dev, struct nss_bridge_pvt *br);
int nss_bridge_mgr_leave_bridge(struct net_device *dev, struct nss_bridge_pvt *br);
void nss_bridge_mgr_ovs_init(void);
void nss_bridge_mgr_ovs_exit(void);

#endif
