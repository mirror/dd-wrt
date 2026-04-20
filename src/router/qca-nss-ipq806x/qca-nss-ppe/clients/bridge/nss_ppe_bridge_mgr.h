/*
 **************************************************************************
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

#ifndef _NSS_PPE_BRIDGE_MGR_H_
#define _NSS_PPE_BRIDGE_MGR_H_

#if (NSS_PPE_BRIDGE_MGR_DEBUG_LEVEL < 1)
#define nss_ppe_bridge_mgr_assert(fmt, args...)
#else
#define nss_ppe_bridge_mgr_assert(c) BUG_ON(!(c))
#endif /* NSS_PPE_BRIDGE_MGR_DEBUG_LEVEL */

/*
 * Compile messages for dynamic enable/disable
 */
#define nss_ppe_bridge_mgr_always(s, ...) pr_alert(s, ##__VA_ARGS__)

#if defined(CONFIG_DYNAMIC_DEBUG)
#define nss_ppe_bridge_mgr_warn(s, ...) \
		pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define nss_ppe_bridge_mgr_info(s, ...) \
		pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define nss_ppe_bridge_mgr_trace(s, ...) \
		pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)

#else /* CONFIG_DYNAMIC_DEBUG */
/*
 * Statically compile messages at different levels
 */
#if (NSS_PPE_BRIDGE_MGR_DEBUG_LEVEL < 2)
#define nss_ppe_bridge_mgr_warn(s, ...)
#else
#define nss_ppe_bridge_mgr_warn(s, ...) \
		pr_warn("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_PPE_BRIDGE_MGR_DEBUG_LEVEL < 3)
#define nss_ppe_bridge_mgr_info(s, ...)
#else
#define nss_ppe_bridge_mgr_info(s, ...) \
		prnotice("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_PPE_BRIDGE_MGR_DEBUG_LEVEL < 4)
#define nss_ppe_bridge_mgr_trace(s, ...)
#else
#define nss_ppe_bridge_mgr_trace(s, ...) \
		pr_info("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif
#endif /* CONFIG_DYNAMIC_DEBUG */

/*
 * Maximum PHY_PORT number
 */
#define NSS_PPE_BRIDGE_MGR_PHY_PORT_MAX 6

#define NSS_PPE_BRIDGE_MGR_SWITCH_ID 0
#define NSS_PPE_BRIDGE_MGR_SPANNING_TREE_ID 0

/*
 * struct nss_ppe_brdige_mgr_context
 *	bridge manager context structure
 */
struct nss_ppe_bridge_mgr_context {
	struct list_head list;		/* List of bridge instance */
	spinlock_t lock;		/* Lock to protect bridge instance */
	struct net_device *wan_netdev;		/* WAN interface netdevice */

	char wan_ifname[IFNAMSIZ];	/* WAN interface name */
	struct ctl_table_header *nss_ppe_bridge_mgr_header;	/* bridge sysctl */
};

/*
 * struct nss_ppe_bridge_mgr_pvt
 *	bridge manager private structure
 */
struct nss_ppe_bridge_mgr_pvt {
	struct list_head list;			/* List of bridge instance */
	struct net_device *dev;			/* Bridge netdevice */
	struct ppe_drv_iface *iface;		/* PPE bridge iface */
	int bond_slave_num;			/* Total number of bond devices added into
						   bridge device */
	bool wan_if_enabled;			/* Is WAN interface enabled? */
	bool fdb_lrn_enabled;			/* Keep track of FDB Learning status */
	struct net_device *wan_netdev;		/* WAN interface netdevice */
	uint32_t mtu;				/* MTU for bridge */
	uint8_t dev_addr[ETH_ALEN];		/* MAC address for bridge */
	atomic64_t bridge_vlan_iface_cnt;		/* Number of VLAN interfaces over bridge */
};



struct nss_ppe_bridge_mgr_pvt *nss_ppe_bridge_mgr_find_instance(struct net_device *dev);
int nss_ppe_bridge_mgr_leave_bridge(struct net_device *dev, struct net_device *bridge_dev);
int nss_ppe_bridge_mgr_join_bridge(struct net_device *dev, struct net_device *bridge_dev);
int nss_ppe_bridge_mgr_unregister_br(struct net_device *dev);
int nss_ppe_bridge_mgr_register_br(struct net_device *dev);
void nss_ppe_bridge_mgr_ovs_init(void);
void nss_ppe_bridge_mgr_ovs_exit(void);

#endif
