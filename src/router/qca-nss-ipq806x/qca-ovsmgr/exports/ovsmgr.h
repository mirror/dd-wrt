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
 * ovsmgr.h
 */
#ifndef __OVSMGR__H
#define __OVSMGR__H

#include <linux/if_vlan.h>

/*
 * Notifiers
 */
#define OVSMGR_DP_ADD 1
#define OVSMGR_DP_DEL 2
#define OVSMGR_DP_BR_ADD 3
#define OVSMGR_DP_BR_DEL 4
#define OVSMGR_DP_PORT_ADD 5
#define OVSMGR_DP_PORT_DEL 6
#define OVSMGR_DP_FLOW_ADD 7
#define OVSMGR_DP_FLOW_DEL 8
#define OVSMGR_DP_FLOW_CHANGE 9
#define OVSMGR_DP_FLOW_TBL_FLUSH 10
#define OVSMGR_DP_VLAN_ADD 11
#define OVSMGR_DP_VLAN_DEL 12

/*
 * Hook numbers
 */
#define OVSMGR_DP_HOOK_PRE_FLOW_PROC  1	/* Call hook function before flow search */
#define OVSMGR_DP_HOOK_POST_FLOW_PROC 2	/* Call hook function after flow is found */

/*
 * Signature to validate if skb->cb is populated by OVS manager
 */
#define OVSMGR_SKB_SIGNATURE 0xAEAD1246

#define OVSMGR_INVALID_DSCP 0xFFFFFFFF

enum ovsmgr_flow_status {
	OVSMGR_FLOW_STATUS_ALLOW_ACCEL,		/* Allow the acceleration of the flow, no VLAN operation */
	OVSMGR_FLOW_STATUS_DENY_ACCEL,		/* Deny the acceleration of the flow */
	OVSMGR_FLOW_STATUS_DENY_ACCEL_EGRESS,	/* outdev in flow is not allowed for acceleration */
	OVSMGR_FLOW_STATUS_ALLOW_VLAN_ACCEL,	/* Allow the acceleration of the flow, single VLAN operation */
	OVSMGR_FLOW_STATUS_ALLOW_VLAN_QINQ_ACCEL,
						/* Allow the acceleration of the flow, QinQ VLAN operation */
	OVSMGR_FLOW_STATUS_UNKNOWN		/* Flow status is unknown */
};

/*
 * ovsmgr_skb_cb
 *	skb->cb details updated by OVS manager before forwarding the packet post
 *	flow hook.
 */
struct ovsmgr_skb_cb {
	uint32_t ovs_sig;
	void *flow;
};

#define OVSMGR_OVS_CB(skb) ((struct ovsmgr_skb_cb *)(skb)->cb)

/*
 * Hook function type
 */
typedef unsigned int (*ovsmgr_dp_hookfn_t)(struct sk_buff *skb, struct net_device *out);

/*
 * ovsmgr_dp_hook_ops
 *	Datapath hook options
 */
struct ovsmgr_dp_hook_ops {
	struct list_head list;
	int protocol;			/* IP protocol interested in */
	int hook_num;			/* Hook number */
	ovsmgr_dp_hookfn_t hook;	/* Hook function to be called */
};

/*
 * ovsmgr_dp_tuple
 *	Datapath flow tuple
 */
struct ovsmgr_dp_tuple {
	union {
		struct {
			__be32 src;
			__be32 dst;
		} ipv4;

		struct {
			struct in6_addr src;
			struct in6_addr dst;
		} ipv6;
	};
	uint16_t src_port;	/* Source port */
	uint16_t dst_port;	/* Destination port */
	uint8_t ip_version;	/* IP version */
	uint8_t protocol;	/* IP protocol */
};

/*
 * ovsmgr_dp_flow
 *	Datapath flow details
 */
struct ovsmgr_dp_flow {
	struct net_device *indev;		/* Ingress interface */
	struct net_device *outdev;		/* Egress interface */
	uint8_t smac[ETH_ALEN];			/* Source MAC */
	uint8_t dmac[ETH_ALEN];			/* Destination MAC */
	struct vlan_hdr ingress_vlan;		/* Ingress VLAN header */
	struct ovsmgr_dp_tuple tuple;		/* Flow tuple */
	bool is_routed;				/* Routed flag, set when flow is routed */
};

/*
 * ovsmgr_dp_flow_stats
 *	Datapath flow statistics
 */
struct ovsmgr_dp_flow_stats {
	uint32_t pkts;	/* Number of received packets matching the flow */
	uint32_t bytes;	/* Number of received bytes matching the flow */
};

/*
 * ovsmgr_flow_info
 *	OVS Flow information for the given key.
 *	ingress[0]/egress[0] represents inner VLAN header details
 *	ingress[1]/egress[1] represents outer VLAN header details
 *	0 value represents no vlan operations required
 *	dscp contains DSCP to be marked for given flow key.
 */
struct ovsmgr_flow_info {
	uint32_t dscp;			/* DSCP mark */
	struct vlan_hdr ingress[2];	/* Ingress VLAN details */
	struct vlan_hdr egress[2];	/* Egress VLAN details */
};

/*
 * ovsmgr_dp_port_info
 *	Port notification structure
 */
struct ovsmgr_dp_port_info {
	struct net_device *master;	/* OVS bridge interface */
	struct net_device *dev;		/* OVS bridge port interface */
};

/*
 * ovsmgr_dp_port_vlan_info
 *	Port VLAN notification structure
 */
struct ovsmgr_dp_port_vlan_info {
	struct vlan_hdr vh;
	struct net_device *master;
	struct net_device *dev;
};

/*
 * ovsmgr_notifiers_info
 *	Notifiers structure
 */
struct ovsmgr_notifiers_info {
	union {
		struct net_device *dev;			/* Netdev for bridge add/del notification */
		struct ovsmgr_dp_port_info *port;	/* Port details for port add/del notification */
		struct ovsmgr_dp_port_vlan_info *vlan;	/* Port VLAN details for VLAN add/del notification */
		struct ovsmgr_dp_flow *flow;		/* Flow details for datapath flow add/del notification */
	};
};

void ovsmgr_notifier_register(struct notifier_block *nb);
void ovsmgr_notifier_unregister(struct notifier_block *nb);
int ovsmgr_dp_hook_register(struct ovsmgr_dp_hook_ops *ops);
void ovsmgr_dp_hook_unregister(struct ovsmgr_dp_hook_ops *ops);
void ovsmgr_bridge_interface_stats_update(struct net_device *dev,
					  uint32_t rx_packets, uint32_t rx_bytes,
					  uint32_t tx_packets, uint32_t tx_bytes);
void ovsmgr_flow_stats_update(struct ovsmgr_dp_flow *flow, struct ovsmgr_dp_flow_stats *stats);
struct net_device *ovsmgr_port_find(struct sk_buff *skb, struct net_device *dev, struct ovsmgr_dp_flow *flow);
struct net_device *ovsmgr_port_find_by_mac(struct sk_buff *skb, struct net_device *dev, struct ovsmgr_dp_flow *flow);
enum ovsmgr_flow_status ovsmgr_flow_info_get(struct ovsmgr_dp_flow *flow,
					      struct sk_buff *skb, struct ovsmgr_flow_info *ofi);
struct net_device *ovsmgr_dev_get_master(struct net_device *dev);
bool ovsmgr_is_ovs_master(struct net_device *dev);

#endif /* __OVSMGR__H */
