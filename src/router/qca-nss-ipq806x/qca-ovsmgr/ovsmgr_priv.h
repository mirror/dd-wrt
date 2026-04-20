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

/*
 * ovsmgr_priv.h
 */
#ifndef __OVSMGR_PRIV__H
#define __OVSMGR_PRIV__H

#include <linux/version.h>

#define OVSMGR_DEBUG_LVL_ERROR 1	/* Turn on debug for an error. */
#define OVSMGR_DEBUG_LVL_WARN 2		/* Turn on debug for a warning. */
#define OVSMGR_DEBUG_LVL_INFO 3		/* Turn on debug for information. */
#define OVSMGR_DEBUG_LVL_TRACE 4	/* Turn on debug for trace. */

/*
 * Maximum number of VLAN's per port
 */
#define OVSMGR_PORT_VLAN_MAX_CNT 32

/*
 * Compile messages for dynamic enable/disable
 */
#define ovsmgr_error(s, ...) do { \
	if (net_ratelimit()) {  \
		pr_alert("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__);  \
	} \
} while (0)

#if defined(CONFIG_DYNAMIC_DEBUG)
/*
 * Compile messages for dynamic enable/disable
 */
#define ovsmgr_info(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define ovsmgr_trace(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define ovsmgr_warn(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)

#else
/*
 * Statically compile messages at different levels
 */
#if (OVSMGR_DEBUG_LEVEL >= OVSMGR_DEBUG_LVL_INFO)
#define ovsmgr_info(s, ...) pr_notice("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__);
#else
#define ovsmgr_info(s, ...)
#endif

#if (OVSMGR_DEBUG_LEVEL >= OVSMGR_DEBUG_LVL_TRACE)
#define ovsmgr_trace(s, ...) pr_info("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__);
#else
#define ovsmgr_trace(s, ...)
#endif

#if (OVSMGR_DEBUG_LEVEL >= OVSMGR_DEBUG_LVL_WARN)
#define ovsmgr_warn(s, ...) pr_warn("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__);
#else
#define ovsmgr_warn(s, ...)
#endif
#endif /* !CONFIG_DYNAMIC_DEBUG */

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0))
#define OVSMGR_KEY_VLAN_TCI(key) (key)->eth.tci
#else
#define OVSMGR_KEY_VLAN_TCI(key) (key)->eth.vlan.tci
#define OVSMGR_KEY_CVLAN_TCI(key) (key)->eth.cvlan.tci
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0))
#define VLAN_TAG_PRESENT VLAN_CFI_MASK
#endif

/*
 * ovsmgr_stats
 *	Datapath statistics maintained by OVS manager
 */
struct ovsmgr_stats {
	atomic64_t pkts_from_ovs_dp;	/* Number of packets received from OVS datapath */
	atomic64_t pkts_fwd_pre_flow;	/* Number of packets forwarded to pre flow hook functions*/
	atomic64_t pkts_fwd_post_flow;	/* Number of packets forwarded to post flow hook functions*/
};

/*
 * ovsmgr_ctx
 *	OVS manager context
 */
struct ovsmgr_ctx {
	struct list_head dp_list;	/* List of data paths */
	struct list_head dp_hooks;	/* List of registered hooks */
	rwlock_t lock;			/* Lock to access datapath list */
	struct dentry *dentry;		/* Debugfs entry for OVS */
	struct dentry *dentry_file;	/* Debugfs file entry for OVS */
	struct ovsmgr_stats stats;	/* Pakcet processing statistics. */
};

/*
 * ovsmgr_vlan_info
 *	OVS VLAN information for a given port
 */
struct ovsmgr_vlan_info {
	struct vlan_hdr vlan;	/* VLAN configuration for this port */
	int ref_cnt;
};

/*
 * ovsmgr_dp_port
 *	Datapath port details
 */
struct ovsmgr_dp_port {
	struct list_head node;				/* To add to port_list */
	struct net_device *dev;				/* Port netdev  */
	struct net_device *master_dev;			/* Master netdev */
	struct ovsmgr_vlan_info vlan_info[OVSMGR_PORT_VLAN_MAX_CNT];	/* VLAN configuration for this port */
	char master_name[IFNAMSIZ];			/* Master device name */
	void *vport;					/* OVS datapath port context */
	int vport_num;					/* OVS datapath port number */
	enum ovs_vport_type vport_type;			/* OVS datapath port type */
	bool add_notified;				/* Set to true if port add notifier is sent */
};

/*
 * ovsmgr_dp
 *	Datapath details
 */
struct ovsmgr_dp {
	struct list_head node;		/* To add to dp_list */
	struct list_head port_list;	/* List of bridge ports */
	struct net_device *dev;		/* netdev representing data path */
	void *dp;			/* OVS data path context */
};

/*
 * ovsmgr_dp_ctx
 *	OVS manager context
 */
struct ovsmgr_dp_ctx {
	struct list_head dp_list;	/* List of data paths */
	rwlock_t lock;			/* Lock to access dp_list */
};

extern struct ovsmgr_ctx ovsmgr_ctx;

int ovsmgr_dp_init(void);
void ovsmgr_dp_exit(void);
int ovsmgr_notifiers_call(struct ovsmgr_notifiers_info *info, unsigned long val);
void ovsmgr_dp_bridge_interface_stats_update(struct net_device *dev,
					  uint32_t rx_packets, uint32_t rx_bytes,
					  uint32_t tx_packets, uint32_t tx_bytes);
void ovsmgr_dp_flow_stats_update(struct ovsmgr_dp_flow *flow, struct ovsmgr_dp_flow_stats *stats);
struct net_device *ovsmgr_dp_port_dev_find(struct sk_buff *skb,
					       struct net_device *dev, struct ovsmgr_dp_flow *flow);
struct net_device *ovsmgr_dp_port_dev_find_by_mac(struct sk_buff *skb, struct net_device *dev,
							struct ovsmgr_dp_flow *flow);
struct net_device *ovsmgr_dp_dev_get_master(struct net_device *dev);
bool ovsmgr_dp_dev_is_master(struct net_device *dev);
enum ovsmgr_flow_status ovsmgr_dp_flow_info_get(struct ovsmgr_dp_flow *flow,
						     struct sk_buff *skb, struct ovsmgr_flow_info *ofi);

#endif /* __OVSMGR_PRIV__H */
