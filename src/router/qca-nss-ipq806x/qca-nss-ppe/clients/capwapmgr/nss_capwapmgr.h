/*
 **************************************************************************
 * Copyright (c) 2014-2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef __NSS_CAPWAPMGR_H
#define __NSS_CAPWAPMGR_H

#if (NSS_CAPWAPMGR_DEBUG_LEVEL < 1)
#define nss_capwapmgr_assert(c, s, ...)
#else
#define nss_capwapmgr_assert(c, s, ...) if (!(c)) { printk(KERN_CRIT "%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__); BUG_ON(!(c)); }
#endif /* NSS_CAPWAPMGR_DEBUG_LEVEL */

/*
 * Compile messages for dynamic enable/disable
 */
#if defined(CONFIG_DYNAMIC_DEBUG)
#define nss_capwapmgr_warn(s, ...)	pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define nss_capwapmgr_info(s, ...)	pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define nss_capwapmgr_trace(s, ...)	pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else /* CONFIG_DYNAMIC_DEBUG */
/*
 * Statically compile messages at different levels
 */
#if (NSS_CAPWAPMGR_DEBUG_LEVEL < 2)
#define nss_capwapmgr_warn(s, ...)
#else
#define nss_capwapmgr_warn(s, ...)	pr_warn("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_CAPWAPMGR_DEBUG_LEVEL < 3)
#define nss_capwapmgr_info(s, ...)
#else
#define nss_capwapmgr_info(s, ...)	pr_notice("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_CAPWAPMGR_DEBUG_LEVEL < 4)
#define nss_capwapmgr_trace(s, ...)
#else
#define nss_capwapmgr_trace(s, ...)	pr_info("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif
#endif /* CONFIG_DYNAMIC_DEBUG */

/*
 * CAPWAP net device Name.
 */
#define NSS_CAPWAPMGR_NETDEV_NAME	"nsscapwap"

/*
 * NSS capwap mgr macros
 */
#define NSS_CAPWAPMGR_NORMAL_FRAME_MTU	1500

/*
 * Default CAPWAP re-assembly timeout 10 milli-seconds.
 */
#define NSS_CAPWAPMGR_REASSEMBLY_TIMEOUT 10

/*
 * Ethernet types.
 */
#define NSS_CAPWAPMGR_ETH_TYPE_MASK	0xFFFF
#define NSS_CAPWAPMGR_ETH_TYPE_TRUSTSEC	0x8909
#define NSS_CAPWAPMGR_ETH_TYPE_IPV4	ETH_P_IP
#define NSS_CAPWAPMGR_ETH_TYPE_IPV6	ETH_P_IPV6
#define NSS_CAPWAPMGR_DSCP_MAX	64

/*
 * Vlan tag not configured
 */
#define NSS_CAPWAPMGR_VLAN_TAG_NOT_CONFIGURED	0xFFF

/*
 * ACL specific parameters.
 */
#define NSS_CAPWAPMGR_ETH_TYPE_SIZE 2
#define NSS_CAPWAPMGR_ETH_HDR_OFFSET 6
#define NSS_CAPWAPMGR_IPV4_OFFSET 8
#define NSS_CAPWAPMGR_DSCP_MASK_IPV4_SHIFT 2
#define NSS_CAPWAPMGR_DSCP_MASK_IPV6_SHIFT 6
#define NSS_CAPWAPMGR_DEV_ID 0
#define NSS_CAPWAPMGR_GROUP_ID 0
#define NSS_CAPWAPMGR_RULE_NR 1

/*
 * ACL rule bind bitmap for all physical ports (1 through 6)
 */
#define NSS_CAPWAPMGR_BIND_BITMAP 0x7E

/*
 * We need 4 ACL rules - 2 rules for each v4 and v6 classification
 * of dhcp, dscp + trustsec.
 */
#define NSS_CAPWAPMGR_ACL_DSCP_RULES_PER_LIST 4

/*
 * list-id 55 and 60 reserved for this purpose.
 * TODO: Find a better approach to reserve list-id.
 */
#define NSS_CAPWAPMGR_ACL_TRUSTSEC_LIST_ID 55
#define NSS_CAPWAPMGR_ACL_TRUSTSEC_LIST_PRIO 0
#define NSS_CAPWAPMGR_ACL_TRUSTSEC_RULE_ID 0

/*
 * DSCP ACL list id and priority.
 */
#define NSS_CAPWAPMGR_ACL_DSCP_LIST_ID 60
#define NSS_CAPWAPMGR_ACL_DSCP_LIST_CNT 1
#define NSS_CAPWAPMGR_ACL_DSCP_LIST_PRIO 1

/*
 * Tx l2 tunnel edit parameters.
 */
#define NSS_CAPWAPMGR_TX_INNER_PAYLOAD_TYPE_ETHER 0
#define NSS_CAPWAPMGR_TX_INNER_PAYLOAD_TYPE_IP 1
#define NSS_CAPWAPMGR_TX_SVLAN_ENABLED 1
#define NSS_CAPWAPMGR_TX_CVLAN_ENABLED 1
#define NSS_CAPWAPMGR_TX_VLAN_OFFSET 12
#define NSS_CAPWAPMGR_TX_VLAN_TPID_MASK 0xFFFF0000
#define NSS_CAPWAPMGR_TX_VLAN_TPID_SHIFT 16
#define NSS_CAPWAPMGR_TX_VLAN_TPID_SIZE 2

#define NSS_CAPWAPMGR_TX_VLAN_TCI_MASK 0xFFFF
#define NSS_CAPWAPMGR_TX_VLAN_TCI_SIZE 2

/*
 * Number of ports to which the acl rule can be attached.
 */
#define NSS_CAPWAPMGR_ACL_TRUSTSEC_PORT_MAX 6

/*
 * CAPWAP VP MTU.
 */
#define NSS_CAPWAPMGR_VP_MTU 9216

/*
 * nss_capwapmgr_acl
 *	Object containing rule related info.
 */
struct nss_capwapmgr_acl {
	bool in_use;			/* Set when rule is in use. */
	uint8_t uid;			/* Unique ID for this rule object. */
	uint8_t list_id;		/* List on which this rule resides. */
	uint8_t rule_id;		/* Rule-id of this rule. */
	uint8_t dscp_value;		/* DSCP value */
	uint8_t dscp_mask;		/* DSCP mask */
};

/*
 * nss_capwapmgr_acl_list
 */
struct nss_capwapmgr_acl_list {
	struct nss_capwapmgr_acl rule[NSS_CAPWAPMGR_ACL_DSCP_RULES_PER_LIST];
					/* Rules on this ACL list. */
};

/*
 * nss_capwapmgr_global
 *	Global structure for capwapmgr.
 */
struct nss_capwapmgr_global {
	uint32_t count;				/* Counter for driver queue selection. */
	struct nss_capwapmgr_acl_list acl_list[NSS_CAPWAPMGR_ACL_DSCP_LIST_CNT];
						/* Set when ACL rule is in use. */
	struct nss_capwap_tunnel_stats tunneld_stats;	/* Stats from deleted capwap tunnels. */
	atomic_t trustsec_tunnel_count[NSS_CAPWAPMGR_ACL_TRUSTSEC_PORT_MAX];
						/* Number of trustsec tunnels for every physical interface */
	ppe_vp_num_t trustsec_rx_vp_num;	/* VP for handling trustsec_rx */
	struct net_device *trustsec_rx_internal_ndev;	/* Internal netdev for trustsec_rx */
	atomic_t trustsec_acl_rule_create_req;	/* Number of trustsec_rx acl create requests */
	atomic_t dscp_acl_rule_create_req;	/* Number of dscp acl rule create requests */
	bool trustsec_rx_vp_configured;		/* Flag to check if trustsec_rx vp is configured */
	bool trustsec_rx_vp_config_in_progress;	/* Flag to check if trustsec_rx vp config is in progress */
	struct dentry *capwap_dentry;		/* Dentry to enable/disable ppe to host mode */
	bool ppe2host;				/* ppe2host configuration */
};

#endif /* __NSS_CAPWAPMGR_H */
