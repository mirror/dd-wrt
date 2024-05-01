/*
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef __ECM_STATS_V6_H
#define __ECM_STATS_V6_H

/*
 * Enum for common IPv6 flow exceptions.
 */
enum ecm_stats_v6_exception_cmn_events {
	ECM_STATS_V6_EXCEPTION_FRONT_END_STOPPED,
			/* Number of IPv6 packets ignored as front end is stopped */
	ECM_STATS_V6_EXCEPTION_BCAST_PACKET_IGNORED,
			/* Number of IPv6 broadcast packets ignored */
	ECM_STATS_V6_EXCEPTION_PPTP_DISABLED,
			/* Number of IPv6 packets ignored as PPTP accelaration failed */
	ECM_STATS_V6_EXCEPTION_L2TPV2_DISABLED,
			/* Number of IPv6 packets ignored as L2TPv2 accelaration failed */
	ECM_STATS_V6_EXCEPTION_L2TPV3_UNSUPPORTED_INTERFACE,
			/* Number of IPv6 packets ignored as L2TPv3 pseudowire interface (ex: PPP) not supported */
	ECM_STATS_V6_EXCEPTION_LOCAL_PACKETS_IGNORED,
			/* Number of IPv6 locally formed packets ignored */
	ECM_STATS_V6_EXCEPTION_BRIDGE_PACKET_WRONG_HOOK,
			/* Number of IPv6 bridge packets ignored */
	ECM_STATS_V6_EXCEPTION_OVS_DISABLED,
			/* Number of IPv6 OVS packets ignored */
	ECM_STATS_V6_EXCEPTION_BRIDGE_FRONT_END_STOPPED,
			/* Number of IPv6 bridge packets ignored as front end is stopped */
	ECM_STATS_V6_EXCEPTION_BRIDGE_BCAST_PACKET_IGNORED,
			/* Number of IPv6 bridge broadcast packets ignored */
	ECM_STATS_V6_EXCEPTION_BRIDGE_PPTP_ACCEL_FAIL,
			/* Number of IPv6 bridge packets ignored as PPTP accelaration failed */
	ECM_STATS_V6_EXCEPTION_BRIDGE_MALFORMED_IP_HEADER,
			/* Number of malformed IPv6 header bridge packets ignored */
	ECM_STATS_V6_EXCEPTION_BRIDGE_NOT_IP_PPPOE_PACKETS,
			/* Number of bridge IPv6 header packets ignored as not PPPOE */
	ECM_STATS_V6_EXCEPTION_BRIDGE_MASTER_NOT_FOUND,
			/* Number of bridge IPv6 bridge packets ignored as not PPPOE */
	ECM_STATS_V6_EXCEPTION_BRIDGE_LOCAL_PACKETS_IGNORED,
			/* Number of IPv6 bridge locally formed packets ignored */
	ECM_STATS_V6_EXCEPTION_BRIDGE_PORT_NOT_FOUND,
			/* Number of IPv6 bridge packets ignored as port not found*/
	ECM_STATS_V6_EXCEPTION_BRIDGE_HAIRPIN_NOT_ENABLED,
			/* Number of IPv6 bridge packets ignored as hairpin not enabled */
	ECM_STATS_V6_EXCEPTION_ROUTED_PACKET_WRONG_HOOK,
			/* Number of IPv6 bridge routed packets ignored */
	ECM_STATS_V6_EXCEPTION_BRIDGE_DEST_MAC_NOT_FOUND,
			/* Number of IPv6 bridge packets ignored as destination MAC not found*/
	ECM_STATS_V6_EXCEPTION_BRIDGE_MAC_ENTRY_NOT_FOUND,
			/* Number of IPv6 bridge packets ignored as MAC entry not found */
	ECM_STATS_V6_EXCEPTION_BRIDGE_PPPOE_ACCEL_DISABLED,
			/* Number of IPv6 bridge packets ignored as PPPOE accel disabled */
	ECM_STATS_V6_EXCEPTION_BRIDGE_PROTO_NOT_PPPOE,
			/* Number of IPv6 bridge packets ignored as protocol not PPPOE*/
	ECM_STATS_V6_EXCEPTION_BRIDGE_PPPOE_MALFORMED_HEADER,
			/* Number of IPv6 bridge packets ignored as PPPOE header malformed */
	ECM_STATS_V6_EXCEPTION_BRIDGE_PPPOE_MCAST_ACCEL_NOT_SUPPORTED,
			/* Number of IPv6 bridge packets ignored as PPPOE Multicat accel not supported*/
	ECM_STATS_V6_EXCEPTION_MALFORMED_IP_HEADER,
			/* Number of malformed IPv6 header packets ignored */
	ECM_STATS_V6_EXCEPTION_NON_IPV6_HDR,
			/* Number of IPv6 packets ignored with not IPv6 header */
	ECM_STATS_V6_EXCEPTION_FRAGMENTED_PACKETS,
			/* Number of IPv6 fragmented packets ignored */
	ECM_STATS_V6_EXCEPTION_CONNTRACK_IN_DYING_STATE,
			/* Number of IPv6 packets ignored as conntrack is in dying state */
	ECM_STATS_V6_EXCEPTION_UNTRACKED_CONNTRACK,
			/* Number of IPv6 packets ignored conntrack is untracked */
	ECM_STATS_V6_EXCEPTION_CONN_HAS_HELPER,
			/* Number of IPv6 packets ignored as acceleration not permitted */
	ECM_STATS_V6_EXCEPTION_UNSUPPORTED_GRE_PROTOCOL,
			/* Number of IPv6 packets ignored GRE accelaration not supported */
	ECM_STATS_V6_EXCEPTION_MCAST_STOPPED,
			/* Number of IPv6 packets ignored as multicast is disabled by frontend */
	ECM_STATS_V6_EXCEPTION_MCAST_NOT_SUPPORTED,
			/* Number of IPv6 Packets ignored as multicast is unsupported by selected fronend */
	ECM_STATS_V6_EXCEPTION_MCAST_FEATURE_DISABLED,
			/* Number of IPv6 packets ignored multicast feature is disabled */
	ECM_STATS_V6_EXCEPTION_DEST_IP_NOT_UCAST,
			/* Number of IPv6 packets ignored IP destination IP is not unicast */
	ECM_STATS_V6_EXCEPTION_SRC_IP_NOT_UCAST,
			/* Number of IPv6 packets ignored as source IP is not unicast */
	ECM_STATS_V6_EXCEPTION_NON_PORTED_NOT_SUPPORTED,
			/* Number of IPv6 packets ignored as nonported feature not supported */
	ECM_STATS_V6_EXCEPTION_NON_PORTED_DISABLED,
			/* Number of IPv6 packets ignored as nonported is disabled */
	ECM_STATS_V6_EXCEPTION_ROUTE_IN_IFF_NO_OFFLOAD,
			/* Number of routed IPv6 packet ignored as input interface marked with no-offload */
	ECM_STATS_V6_EXCEPTION_ROUTE_OUT_IFF_NO_OFFLOAD,
			/* Number of routed IPv6 packet ignored as output interface marked with no-offload */
	ECM_STATS_V6_EXCEPTION_BRIDGE_IN_IFF_NO_OFFLOAD,
			/* Number of bridged IPv6 packet ignored as input interface marked with no-offload */
	ECM_STATS_V6_EXCEPTION_BRIDGE_OUT_IFF_NO_OFFLOAD,
			/* Number of bridged IPv6 packet ignored as output interface marked with no-offload */
	ECM_STATS_V6_EXCEPTION_UNSUPPORTED_L2TPV3_PROTOCOL,
			/* Number of IPv6 packets ignored as unsupported L2TPv3 protocol */
	ECM_STATS_V6_EXCEPTION_CMN_MAX
			/*Maximum common exceptions for IPv6 flows*/
};

/*
 * Enum for ported IPv6 flow exceptions.
 */
enum ecm_stats_v6_exception_ported_events {
	ECM_STATS_V6_EXCEPTION_PORTED_TCP_NOT_CONFIRM,
			/* Number of IPv6 packets ignord in ported flow as TCP connection is not confirmed */
	ECM_STATS_V6_EXCEPTION_PORTED_TCP_NOT_ESTAB,
			/* Number of IPv6 packets ignored in ported flow as TCP connection is in unestablished */
	ECM_STATS_V6_EXCEPTION_PORTED_TCP_MALFORMED_HEADER,
			/* Number of IPv6 packets ignored in ported flow  as TCP header is malformed */
	ECM_STATS_V6_EXCEPTION_PORTED_TCP_DENIED_PORT,
			/* Number of IPv6 packets ignored in ported flow as TCP port is denied */
	ECM_STATS_V6_EXCEPTION_PORTED_UDP_CONN_NOT_CONFIRM,
			/* Number of IPv6 packets ignored in ported flow as UDP connection is unconfirmed */
	ECM_STATS_V6_EXCEPTION_PORTED_MALFORMED_UDP_HEADER,
			/* Number of IPv6 packets ignored in ported flow as UDP header is malformed */
	ECM_STATS_V6_EXCEPTION_PORTED_UDP_DENIED_PORT,
			/* Number of IPv6 packets ignored in ported flow as UDP port is denied */
	ECM_STATS_V6_EXCEPTION_PORTED_PROTOCOL_UNSUPPORTED,
			/* Number of IPv6 packets ignored in ported flow as protocol is unsupported */
	ECM_STATS_V6_EXCEPTION_PORTED_ECM_IN_TERMINATING_STATE,
			/* Number of IPv6 packets ignored in ported flow as ECM is in terminating state */
	ECM_STATS_V6_EXCEPTION_PORTED_TCP_IN_TERMINATING_STATE,
			/* Number of IPv6 packets ignored in ported flow as TCP connection is in terminating state */
	ECM_STATS_V6_EXCEPTION_PORTED_NSS_ACCEL_NOT_SUPPORTED,
			/* Number of IPv6 packets ignored in ported flow as NSS accelaration is not supported */
	ECM_STATS_V6_EXCEPTION_PORTED_SFE_ACCEL_NOT_SUPPORTED,
			/* Number of IPv6 packets ignored in ported flow as SFE accelaration is not supported */
	ECM_STATS_V6_EXCEPTION_PORTED_PPE_DS_ACCEL_NOT_SUPPORTED,
			/* Number of IPv6 packets ignored in ported flow as PPE DS accelaration is not supported */
	ECM_STATS_V6_EXCEPTION_PORTED_PPE_VP_ACCEL_NOT_SUPPORTED,
			/* Number of IPv6 packets ignored in ported flow as PPE VP accelaration is not supported */
	ECM_STATS_V6_EXCEPTION_PORTED_PPE_ACCEL_NOT_SUPPORTED,
			/* Number of IPv6 packets ignored in ported flow as PPE accelaration is not supported */
	ECM_STATS_V6_EXCEPTION_PORTED_AE_TYPE_NOT_ASSIGNED,
			/* Number of IPv6 packets ignored in ported flow as AE classifier classification not assigned */
	ECM_STATS_V6_EXCEPTION_PORTED_UNKNOWN_AE_TYPE,
			/* Number of IPv6 packets ignored in ported flow as unknown classifier */
	ECM_STATS_V6_EXCEPTION_PORTED_PRECEDENCE_ALLOC_FAIL,
			/* Number of IPv6 packets ignored in ported flow as classifier allocation failed */
	ECM_STATS_V6_EXCEPTION_PORTED_FRONTEND_ALLOC_FAIL,
			/* Number of IPv6 packets ignored in ported flow as frontend allocation failed */
	ECM_STATS_V6_EXCEPTION_PORTED_FRONTEND_CONSTRUCT_FAIL,
			/* Number of IPv6 packets ignored in ported flow as frontend construct failed */
	ECM_STATS_V6_EXCEPTION_PORTED_FROM_HIERARCHY_CREATION_FAIL,
			/* Number of IPv6 packets ignored in ported flow as from hierarchy creation failed */
	ECM_STATS_V6_EXCEPTION_PORTED_FROM_NODE_FAIL,
			/* Number of IPv6 packets ignored in ported flow as source node creation failed */
	ECM_STATS_V6_EXCEPTION_PORTED_FROM_MAPPING_FAIL,
			/* Number of IPv6 packets ignored in ported flow as source mapping allocation failed */
	ECM_STATS_V6_EXCEPTION_PORTED_TO_HIERARCHY_CREATION_FAIL,
			/* Number of IPv6 packets ignored in ported flow as to hierarchy creation failed */
	ECM_STATS_V6_EXCEPTION_PORTED_TO_NODE_FAIL,
			/* Number of IPv6 packets ignored in ported flow as destination node creation failed */
	ECM_STATS_V6_EXCEPTION_PORTED_TO_MAPPING_FAIL,
			/* Number of IPv6 packets ignored in ported flow as destination mapping creation failed */
	ECM_STATS_V6_EXCEPTION_PORTED_FROM_NAT_HIERARCHY_CREATION_FAIL,
			/* Number of IPv6 packets ignored in the ported flow as from nat hierarchy creation failed */
	ECM_STATS_V6_EXCEPTION_PORTED_FROM_NAT_NODE_FAIL,
			/* Number of IPv6 packets ignored in the ported flow as source nat node creation failed */
	ECM_STATS_V6_EXCEPTION_PORTED_FROM_NAT_MAPPING_FAIL,
			/* Number of IPv6 packets ignored in the ported flow as source nat mapping creation failed */
	ECM_STATS_V6_EXCEPTION_PORTED_TO_NAT_HIERARCHY_CREATION_FAIL,
			/* Number of IPv6 packets ignored in the ported flow as to NAT hierarcy creation failed */
	ECM_STATS_V6_EXCEPTION_PORTED_TO_NAT_NODE_FAIL,
			/* Number of IPv6 packets ignored in the ported flow as destination NAT node creation failed */
	ECM_STATS_V6_EXCEPTION_PORTED_TO_NAT_MAPPING_FAIL,
			/* Number of IPv6 packets ignored in the ported flow as destination NAT mapping creation failed */
	ECM_STATS_V6_EXCEPTION_PORTED_VLAN_FILTER_ADD_FAIL,
			/* Number of IPv6 packets ignored in ported flow as VLAN filter addition failed */
	ECM_STATS_V6_EXCEPTION_PORTED_DEFAULT_CLASSIFIER_ALLOC_FAIL,
			/* Number of IPv6 packets ignored in ported flow as default classifier allocation failed */
	ECM_STATS_V6_EXCEPTION_PORTED_CLASSIFIER_ASSIGN_FAIL,
			/* Number of IPv6 packets ignored in ported flow as classifier assign failed */
	ECM_STATS_V6_EXCEPTION_PORTED_IGNORE_BRIDGE_PACKETS,
			/* Number of IPv6 packets ignored in ported flow as bridge packets */
	ECM_STATS_V6_EXCEPTION_PORTED_IGS_ACCEL_DENIED,
			/* Number of IPv6 packets ignored in ported flow as IGS accelaration ignored */
	ECM_STATS_V6_EXCEPTION_PORTED_TOUCH_TIMER_NOT_SET,
			/* Number of IPv6 packets ignored in ported flow as touch timer not set */
	ECM_STATS_V6_EXCEPTION_PORTED_SRC_NAT_IP_CHANGED,
			/* Number of IPv6 packets ignored in ported flow as source nat ip changed */
	ECM_STATS_V6_EXCEPTION_PORTED_DB_CONN_TIMER_EXPIRED,
			/* Number of IPv6 packets ignored in ported flow as bridge DB connection timer expired */
	ECM_STATS_V6_EXCEPTION_PORTED_DROP_BY_CLASSIFIER,
			/* Number of IPv6 packets ignored in ported flow as dropped by classifier */
	ECM_STATS_V6_EXCEPTION_PORTED_MAX
			/*Maximum ported exceptions for IPv6 flows*/
};

/*
 * Enum for non-ported IPv6 flow exceptions.
 */
#ifdef ECM_NON_PORTED_SUPPORT_ENABLE
enum ecm_stats_v6_exception_non_ported_events {
	ECM_STATS_V6_EXCEPTION_NON_PORTED_NAT_ACCEL_NOT_SUPPORTED,
			/* Number of IPv6 packets ignored in non ported nat accelaration not supported */
	ECM_STATS_V6_EXCEPTION_NON_PORTED_PROTOCOL_UNSUPPORTED,
			/* Number of IPv6 packets ignored in non ported flow as TCP connection is not confirmed */
	ECM_STATS_V6_EXCEPTION_NON_PORTED_ECM_IN_TERMINATING_STATE,
			/* Number of IPv6 packets ignored in non ported flow as ECM is in terminating state */
	ECM_STATS_V6_EXCEPTION_NON_PORTED_NSS_ACCEL_NOT_SUPPORTED,
			/* Number of IPv6 packets ignored in non ported flow as NSS accelaration is not supported */
	ECM_STATS_V6_EXCEPTION_NON_PORTED_PPE_DS_ACCEL_NOT_SUPPORTED,
			/* Number of IPv6 packets ignored in non ported flow as PPE DS accelaration is not supported */
	ECM_STATS_V6_EXCEPTION_NON_PORTED_PPE_VP_ACCEL_NOT_SUPPORTED,
			/* Number of IPv6 packets ignored in non ported flow as PPE VP accelaration is not supported */
	ECM_STATS_V6_EXCEPTION_NON_PORTED_PPE_ACCEL_NOT_SUPPORTED,
			/* Number of IPv6 packets ignored in non ported flow as PPE accelaration is not supported */
	ECM_STATS_V6_EXCEPTION_NON_PORTED_AE_TYPE_NOT_ASSIGNED,
			/* Number of IPv6 packets ignored in non ported flow as AE classifier classification not assigned */
	ECM_STATS_V6_EXCEPTION_NON_PORTED_UNKNOWN_AE_TYPE,
			/* Number of IPv6 packets ignored in non ported flow as unknown classifier */
	ECM_STATS_V6_EXCEPTION_NON_PORTED_PRECEDENCE_ALLOC_FAIL,
			/* Number of IPv6 packets ignored in non ported flow as classifier allocation failed */
	ECM_STATS_V6_EXCEPTION_NON_PORTED_FRONTEND_ALLOC_FAIL,
			/* Number of IPv6 packets ignored in non ported flow as frontend allocation failed */
	ECM_STATS_V6_EXCEPTION_NON_PORTED_FRONTEND_CONSTRUCTION_FAIL,
			/* Number of IPv6 packets ignored in non ported flow as frontend construct failed */
	ECM_STATS_V6_EXCEPTION_NON_PORTED_FROM_HIERARCHY_CREATION_FAIL,
			/* Number of IPv6 packets ignored in non ported flow as from hierarchy creation failed */
	ECM_STATS_V6_EXCEPTION_NON_PORTED_FROM_NODE_FAIL,
			/* Number of IPv6 packets ignored in non ported flow as source node creation failed */
	ECM_STATS_V6_EXCEPTION_NON_PORTED_FROM_MAPPING_FAIL,
			/* Number of IPv6 packets ignored in non ported flow as source mapping allocation failed */
	ECM_STATS_V6_EXCEPTION_NON_PORTED_TO_HIERARCHY_CREATION_FAIL,
			/* Number of IPv6 packets ignored in non ported flow as to hierarchy creation failed */
	ECM_STATS_V6_EXCEPTION_NON_PORTED_TO_NODE_FAIL,
			/* Number of IPv6 packets ignored in non ported flow as destination node creation failed */
	ECM_STATS_V6_EXCEPTION_NON_PORTED_TO_MAPPING_FAIL,
			/* Number of IPv6 packets ignored in non ported flow as destination mapping creation failed */
	ECM_STATS_V6_EXCEPTION_NON_PORTED_DEFAULT_CLASSIFIER_ALLOC_FAIL,
			/* Number of IPv6 packets ignored in non ported flow as default classifier allocation failed */
	ECM_STATS_V6_EXCEPTION_NON_PORTED_CLASSIFIER_ASSIGN_FAIL,
			/* Number of IPv6 packets ignored in non ported flow as classifier assign failed */
	ECM_STATS_V6_EXCEPTION_NON_PORTED_IGS_ACCEL_DENIED,
			/* Number of IPv6 packets ignored in non ported flow as IGS accelaration ignored */
	ECM_STATS_V6_EXCEPTION_NON_PORTED_DB_CONN_TIMER_EXPIRED,
			/* Number of IPv6 packets ignored in non ported flow as bridge DB connection timer expired */
	ECM_STATS_V6_EXCEPTION_NON_PORTED_DROP_BY_CLASSIFIER,
			/* Number of IPv6 packets ignored in non ported flow as dropped by classifier */
	ECM_STATS_V6_EXCEPTION_NON_PORTED_MAX
			/*Maximum non-ported exceptions for IPv6 flows*/
};
#endif

/*
 * Wrapper enum to identify the stats.
 * This is added to avoid using extern for the stats variable.
 */
typedef enum ecm_stats_v6_type {
	ECM_STATS_V6_EXCEPTION_CMN,
	ECM_STATS_V6_EXCEPTION_PORTED,
	ECM_STATS_V6_EXCEPTION_NON_PORTED,
	ECM_STATS_V6_EXCEPTION_MAX
} ecm_stats_v6_type_t;

/*
 * Wrapper for the ECM V6 stats.
 */
struct ecm_stats_v6 {
	atomic64_t ecm_stats_v6_exception_cmn[ECM_STATS_V6_EXCEPTION_CMN_MAX];
	atomic64_t ecm_stats_v6_exception_ported[ECM_STATS_V6_EXCEPTION_PORTED_MAX];
#ifdef ECM_NON_PORTED_SUPPORT_ENABLE
	atomic64_t ecm_stats_v6_exception_non_ported[ECM_STATS_V6_EXCEPTION_NON_PORTED_MAX];
#endif
};

void ecm_stats_v6_inc(ecm_stats_v6_type_t stat_type, int stat_idx);
int ecm_stats_v6_debugfs_init(struct dentry *dentry);

#endif
