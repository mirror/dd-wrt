/*
 * Copyright (c) 2011-2014, 2016 The Linux Foundation. All rights reserved.
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

/*
 * Hybrid bridge definitions and data types
 *
 * netlink message format:
 *  ------------------------------------------
 * | NLMSG_HEADER | HYMSG_HEADER | HYMSG_DATA |
 *  ------------------------------------------
 * HYMSG_DATA format for the different message types:
 *
 * HYFI_GET_HA_TABLE:          multiple of struct __hatbl_entry
 * HYFI_GET_HD_TABLE:          multiple of struct __hdtbl_entry
 * HYFI_UPDATE_HATBL_ENTRIES:  multiple of struct __hatbl_entry
 * HYFI_UPDATE_HDTBL_ENTRIES:  multiple of struct __hdtbl_entry
 * HYFI_ADD_HDTBL_ENTRIES:     multiple of struct __hdtbl_entry
 * HYFI_DEL_HDTBL_ENTRIES:     multiple of struct __hdtbl_entry
 * HYFI_FLUSH_HDTBL:
 * HYFI_SET_HATBL_AGING_PARAM: struct __aging_param
 * HYFI_GET_BRIDGE:            struct __hybr_info
 * HYFI_GET_PORT_LIST:         multiple of struct __brport_group
 * HYFI_SET_EVENT_INFO:        struct __event_info
 * HYFI_SET_BRIDGE_MODE:       bridge mode (HR or HC) (u32)
 * HYFI_SET_BRIDGE_FWMODE:     bridge forwarding mode (APS, Single backhaul, or
 *                                                     Multicast only) (u32)
 * HYFI_SET_BRPORT_GROUP:      group num and type (u32)
 * HYFI_FLUSH_HATBL:
 * HYFI_ADD_HATBL_ENTRIES:     multiple of struct __hatbl_entry
 * HYFI_DEL_HDTBL_ENTRIES_BYID: multiple of struct __hdtbl_entry
 * HYFI_SET_BRPORT_BCAST:      struct __brport_group
 * HYFI_SET_TCP_SP:            u_int32_t
 * HYFI_SET_BRPORT_TYPE:       struct __brport_group
 *
 *
 * netlink event format:
 *  ------------------------------------------
 * | NLMSG_HEADER | EVENT_DATA |
 *  ------------------------------------------
 * HYFI_EVENT_ADD_HA_ENTRY:       struct __hatbl_entry
 * HYFI_EVENT_DEL_HA_ENTRY:       struct __hatbl_entry
 * HYFI_EVENT_AGEOUT_HA_ENTRIES:
 * HYFI_EVENT_LINK_UP:              ifindex (u32)
 * HYFI_EVENT_LINK_DOWN:            ifindex (u32)
 * HYFI_EVENT_LINK_PORT_UP:         ssdk_port_status
 * HYFI_EVENT_LINK_PORT_DOWN:       ssdk_port_status
 * HYFI_EVENT_MAC_LEARN_ON_PORT:    struct __ssdkport_entry
 */

#ifndef _HYFI_API_H
#define _HYFI_API_H

#include <linux/types.h>


/* netlink define */
/* netlink port */
#define NETLINK_QCA_HYFI            ( 25 )
#define NETLINK_QCA_HYFI_EVENT      ( 26 )
#define NLEVENT_INVALID_PID -1

#ifndef IFNAMSIZ /* Avoid including linux/if.h due to GPL */
#define IFNAMSIZ    ( 16 )
#endif

#define HYFI_PORT_INVALID_TYPE      ( 255 )

#define HYFI_AGGR_MAX_IFACE     ( 0x3 )

/* netlink message type */
enum {
	HYFI_GET_BRIDGE = 0,
	HYFI_SET_BRIDGE_MODE, /* HR or HC */
	HYFI_SET_BRIDGE_TCP_SP,
	HYFI_SET_EVENT_PID,
	HYFI_ATTACH_BRIDGE,
	HYFI_DETACH_BRIDGE,
	HYFI_GET_SWITCH_PORT_ID,
	HYFI_SET_BRIDGE_FWMODE,  /* special forwarding mode rules */
	HYFI_SET_BRIDGE_TS_MODE,  /* traffic separation enabled */
	HYFI_VERSION_COMPATIBILITY_CHECK,   /* Version compatibility check */
	HYFI_GET_WDS_EXT_IFACE_LIST,   /* wds_ext iface list used in 4address conversion */
	HYFI_SET_BRIDGE_AP_OP_MODE,  /* check if ctrl or agent enabled */
	HYFI_SET_COLOCATED_IFNAME,

	HYFI_GET_PORT_LIST = 0x100,
	HYFI_SET_BRPORT_GROUP, /* port group number and type */
	HYFI_SET_BRPORT_BCAST,
	HYFI_SET_BRPORT_TYPE,

	HYFI_GET_HA_TABLE = 0x200,
	HYFI_ADD_HATBL_ENTRIES,
	HYFI_UPDATE_HATBL_ENTRIES,
	HYFI_UPDATE_HATBL_ENTRY,
	HYFI_SET_HATBL_AGING_PARAM,
	HYFI_FLUSH_HATBL,

	HYFI_GET_HD_TABLE = 0x300,
	HYFI_ADD_HDTBL_ENTRIES,
	HYFI_UPDATE_HDTBL_ENTRIES,
	HYFI_DEL_HDTBL_ENTRIES, /* del hd entries by mac */
	HYFI_DEL_HDTBL_ENTRIES_BYID, /* del hd entries by id */
	HYFI_FLUSH_HDTBL,

	HYFI_GET_FDB = 0x400,

	HYFI_SET_PATHSWITCH_PARAM = 0x500,

	HYFI_PSW_FIRST = 0x600, /* Advanced path switching range 0x200 - 0x300 */
	HYFI_SET_PSW_MSE_TIMEOUT = HYFI_PSW_FIRST,
	HYFI_SET_PSW_DROP_MARKERS,
	HYFI_SET_PSW_OLD_IF_QUIET_TIME,
	HYFI_SET_PSW_DUP_PKT_FLUSH_QUOTA,
	HYFI_PSW_LAST,

	HYFI_SET_SP_RULE = 0x700, /* Service priortization rule */
	HYFI_FLUSH_SP_RULES,
	HYFI_SET_MSCS_RULE,
	HYFI_SET_SCS_RULE,
};

/* Bridge mode:
 * The Hybrid bridge can be programmed in two modes:
 * 1. Relay override mode: Allow packet relaying from every port in every group to any other port in any other port.
 * 2. Group type relay mode: Packet relay decision is done according to the group type.
 */
#define HYFI_BRIDGE_MODE_RELAY_OVERRIDE        (1)

/**
 * Forwarding mode:
 * The Hybrid bridge can be programmed in one of three modes:
 * 1. Full Hy-Fi forwarding rules: Use H-Default, H-Active, and bridge port
 *                                 configurations to determine whether a
 *                                 packet is dropped and the egress port
 *                                 (on output/forward).
 * 2. No hybrid tables: Use purely the FDB for egress port selection.
 *                      Still enforce the other rules for forwarding/local
 *                      input that are based on bridge port configuration.
 * 3. Multicast only: Only apply the rules relating to IEEE1901/IEEE1905.1/
 *                    LLDP/HCP packets (and specifically multicast).
 */
#define HYFI_BRIDGE_FWMODE_APS                (0)
#define HYFI_BRIDGE_FWMODE_NO_HYBRID_TABLES   (1)
#define HYFI_BRIDGE_FWMODE_MCAST_ONLY         (2)

/* Group type:
 * A port can belong to either 2 groups:
 * 1. Relay group: Where packets are relayed from each port to every other port
 * 2. Non-relay group: Where packets are not relayed between group ports.
 *
 * Note that this has no effect in relay override bridge mode.
 */
#define HYFI_PORTGRP_TYPE_RELAY     (1)
#define HYFI_PORTGRP_INVALID		(255)	/* Uninitialized group number */

#define HYFI_BRIDGE_FLAG_MODE_RELAY_OVERRIDE    ( 1<<0 )
#define HYFI_BRIDGE_FLAG_MODE_TCP_SP            ( 1<<1 )

/**
 * H-Default and H-Active tables are not used for forwarding decisions.
 */
#define HYFI_BRIDGE_FLAG_FWMODE_NO_HYBRID_TABLES  ( 1<<2 )

/**
 * Only the IEEE1901/IEEE1905.1/LLDP/HCP multicast forwarding rules are
 * enforced.
 *
 * No other rules are enforced (including any rules relating to the
 * different types of ports).
 */
#define HYFI_BRIDGE_FLAG_FWMODE_MCAST_ONLY        ( 1<<3 )

/**
 * The mask to extract the bits of the flags that determine which bridge
 * forwarding rules are applied.
 */
#define HYFI_BRIDGE_FLAG_FWMODE_MASK ( HYFI_BRIDGE_FLAG_FWMODE_NO_HYBRID_TABLES |  \
                                       HYFI_BRIDGE_FLAG_FWMODE_MCAST_ONLY )

/* The hyInterfaceType enum is deprecated and should not be used anymore.
 * It is kept here temporarily until it will be cleaned completely.
 * The following macro will break the compilation of any user space
 * application which is trying to use this enum.
 */
#ifndef __KERNEL__
#define __HY_MISUSE_PROTECT( x )    __##x
#else
#define __HY_MISUSE_PROTECT( x )   x
#endif

enum __HY_MISUSE_PROTECT( hyInterfaceType ) {
	__HY_MISUSE_PROTECT( hyInterface_WIFI_2G ) = 0, /* Always leave W2G as '0': used as index by pcw service */
	__HY_MISUSE_PROTECT( hyInterface_WIFI_5G ),
	__HY_MISUSE_PROTECT( hyInterface_HPAV ),
	__HY_MISUSE_PROTECT( hyInterface_ETH ),
	__HY_MISUSE_PROTECT( hyInterface_WIFI_6G ),
	__HY_MISUSE_PROTECT( hyInterface_MLD ),

	__HY_MISUSE_PROTECT( hyInterface_NumberOfChildInterfaces ), /* Use this constant for arrays containing only child interfaces */

	__HY_MISUSE_PROTECT( hyInterface_HY_BRIDGE ) = __HY_MISUSE_PROTECT( hyInterface_NumberOfChildInterfaces ),

	__HY_MISUSE_PROTECT( hyInterface_NumberOfInterfaces )
};

struct __hyctl_msg_header {
	char if_name[IFNAMSIZ]; /* hybrid bridge name: hy0 */
	u_int32_t buf_len; /* not include this header */
	u_int32_t tbl_offsite; /* how many entries to skip */
	u_int32_t status;
	u_int32_t bytes_written;
	u_int32_t bytes_needed;

}__attribute__ ((packed));

#define HYFI_MSG_HDRLEN       sizeof (struct __hyctl_msg_header)
#define HYFI_MSG_DATA(p)      ((void*)(((char*)p) + NLMSG_LENGTH(0) + HYFI_MSG_HDRLEN))

/* define for the status field */
enum {
	HYFI_STATUS_SUCCESS = 0,
	HYFI_STATUS_NOT_SUPPORTED = 1,
	HYFI_STATUS_RESOURCES = 2,
	HYFI_STATUS_INVALID_PARAMETER = 3,
	HYFI_STATUS_BUFFER_OVERFLOW = 4,
	HYFI_STATUS_FAILURE = 5,
	HYFI_STATUS_NOT_FOUND = 6,
};

#define HYFI_TRAFFIC_CLASS_OTHER 0
#define HYFI_TRAFFIC_CLASS_UDP   1

struct __aging_param {
	u_int32_t aging_time;
};

struct __event_info {
	u_int32_t event_pid;
};

struct __switchport_index {
        u_int8_t mac_addr[6];
        u_int32_t vlanid;
        u_int32_t portid;
};

struct __ssdkport_entry {
	u_int8_t  addr[6];
	u_int32_t vlanid;
	u_int32_t portid;
	u_int16_t portlink;
};
struct __hybr_info {
	u_int32_t ifindex;
	u_int32_t flags;
};

struct __brport_group {
	u_int32_t ifindex;
	u_int32_t group_num;
	u_int32_t group_type;
	u_int32_t bcast_enable;
	u_int32_t port_type;
};

/* netlink event type */
enum {
	HYFI_EVENT_ADD_HA_ENTRY,
	HYFI_EVENT_DEL_HA_ENTRY,
	HYFI_EVENT_AGEOUT_HA_ENTRIES,
	HYFI_EVENT_LINK_UP,
	HYFI_EVENT_LINK_DOWN,
	HYFI_EVENT_FDB_UPDATED,
	HYFI_EVENT_LINK_PORT_UP,
	HYFI_EVENT_LINK_PORT_DOWN,
	HYFI_EVENT_MAC_LEARN_ON_PORT,
	HYFI_EVENT_BR_JOIN,
	HYFI_EVENT_BR_LEAVE,
};

struct __hfdb_entry {
	u_int8_t mac_addr[6];
	u_int8_t ifindex;
	u_int8_t is_local;
	u_int32_t ageing_timer_value;
	u_int8_t ifindex_hi;
	u_int16_t portid;
	u_int8_t pad0;
};

struct __hdtbl_entry {
	u_int8_t mac_addr[6];
	u_int8_t id[6];
	u_int32_t udp_port; /* udp port ifindex */
	u_int32_t other_port; /* other port ifindex */
	u_int32_t static_entry;
};

struct __hatbl_port_list {
	u_int32_t port; /* port ifindex */
	u_int32_t quota;
};

struct __hatbl_entry {
	u_int16_t hash;
	u_int8_t sa[6];
	u_int8_t da[6];
	u_int8_t id[6];
	u_int16_t sub_class;
	struct __hatbl_port_list port_list[HYFI_AGGR_MAX_IFACE];
	u_int32_t age; /* since last access */
	u_int32_t create_time; /* ha entry create time */
	u_int32_t num_packets;
	u_int32_t num_bytes;
	u_int32_t priority;
	u_int32_t action :3; /* drop, throttle */
	u_int32_t local :1; /* not created from HD */
	u_int32_t static_entry :1;
	u_int32_t aggr_entry :1; /* Aggregated entry */
	u_int32_t accl_entry :1; /* Accelerated entry */

	u_int32_t serial;	/* Serial number */
	u_int32_t reserved; /* Reserved */
};

struct __path_switch_param {
	u_int32_t enable_switch_markers;
};

struct __sp_rule {
	/// service prioritization rule identifier.
	u_int32_t id;
	/// add-remove filter rule bit.
	/// 1 means add 0 means delete
	u_int8_t add_delete_rule ;
	/// rule precedence – higher number means higher priority.
	u_int8_t rule_precedence;
	/// rule output the value of or method  used to select the 802.1q c-tag
	/// pcp value with which  to mark the matched packet.
	u_int8_t rule_output;
	/// rule match always true
	/// s(skip field matching) flag
	u_int8_t rule_match_always_true ;
	/// match up in 802.11 qos control flag
	u_int8_t matchup ;
	/// up in 802.11 qos control match sense flag
	u_int8_t match_up_sense ;
	/// match source mac address flag
	u_int8_t match_source_mac ;
	/// match source mac address sense
	u_int8_t match_source_mac_sense ;
	/// match destination mac address flags
	u_int8_t match_dst_mac ;
	/// destination mac address match sense flag
	u_int8_t match_dst_mac_sense ;
	/// up in 802.11 qos control
	u_int8_t user_priority;
	/// source mac address
	/// if “match source mac address” flag bit is set to one,
	/// this field shall be included, otherwise this field shall be omitted.
	u_int8_t sa[6];
	/// destination mac address
	/// if “match destination mac address” flag bit is set to one,
	/// this field shall be included, otherwise this field shall be omitted.
	u_int8_t da[6];

	/// internal field to keep track if rule is valid or not
	u_int8_t valid_qsp;

	/// Match Source IPv4 Address flag
	u_int8_t match_source_ipv4;
	/// Match Source IPv4 Address sense
	u_int8_t match_source_ipv4_sense;
	/// Match Destination IPv4 Address flags
	u_int8_t match_dst_ipv4;
	/// Destination IPv4 Address Match Sense Flag
	u_int8_t match_dst_ipv4_sense;

	/// Match Source IPv6 Address flag
	u_int8_t match_source_ipv6;
	/// Match Source IPv6 Address sense
	u_int8_t match_source_ipv6_sense;
	/// Match Destination IPv6 Address flags
	u_int8_t match_dst_ipv6;
	/// Destination IPv6 Address Match Sense Flag
	u_int8_t match_dst_ipv6Sense;

	/// Match Source port flag
	u_int8_t match_source_port;
	/// Match Source port sense
	u_int8_t match_source_port_sense;
	/// Match Destination port flags
	u_int8_t match_dst_port;
	/// Destination port Match Sense Flag
	u_int8_t match_dst_port_sense;

	/// Match protocol number or next header flag
	u_int8_t match_protocol_number;
	/// Match protocol number or next header Match sense flag
	u_int8_t match_protocol_number_sense;
	/// Match VLAN ID flags
	u_int8_t match_vlan_id;
	/// Match VLAN ID Match sense flags
	u_int8_t match_vlan_id_sense;

	/// Match dscp flags
	u_int8_t match_dscp;
	/// Match dscp Match semse flags
	u_int8_t match_dscp_sense;

	/// Source IPv4 Address
	/// If “Match Source IPv4 Address” flag bit is set to one,
	/// this field shall be included, otherwise this field shall be omitted.
	u_int32_t src_ipv4_addr;
	/// Source IPv6 Address
	/// If “Match Source IPv6 Address” flag bit is set to one,
	/// this field shall be included, otherwise this field shall be omitted.
	u_int32_t src_ipv6_addr[4];
	/// Destination IPv4 Address
	/// If “Match Destination IPv4 Address” flag bit is set to one,
	/// this field shall be included, otherwise this field shall be omitted.
	u_int32_t dst_ipv4_addr;
	/// Destination IPv6 Address
	/// If “Match Destination IPv6 Address” flag bit is set to one,
	/// this field shall be included, otherwise this field shall be omitted.
	u_int32_t dst_ipv6_addr[4];
	/// Source Port
	u_int16_t src_port;
	/// Destination Port
	u_int16_t dst_port;
	/// Protocol Number or Next Header
	u_int8_t protocol_number;
	/// VLAN ID
	u_int16_t vlan_id;
	/// DSCP value
	u_int8_t dscp;
	/// Service interval downlink
	u_int8_t service_interval_dl;
	/// Service interval uplink
	u_int8_t service_interval_ul;
	/// Burst size downlink
	uint32_t burst_size_dl;
	/// Burst size uplink
	uint32_t burst_size_ul;
};

struct __mscs_rule {
	/// service prioritization rule identifier.
	u_int32_t id;
	/// add-remove filter rule bit.
	/// 1 means add 0 means delete
	u_int8_t add_delete_rule;
	/// rule precedence – higher number means higher priority.
	u_int8_t rule_precedence;
	/// rule output the value of or method  used to select the 802.1q c-tag
	/// pcp value with which  to mark the matched packet.
	u_int8_t rule_output;
	/// match source mac address flag
	u_int8_t match_source_mac;
	/// match destination mac address flags
	u_int8_t match_dst_mac;
	/// source mac address
	/// If “Match Source IPv6 Address” flag bit is set to one,
	/// this field shall be included, otherwise this field shall be omitted.
	u_int8_t sa[6];
	/// destination mac address
	/// if “match destination mac address” flag bit is set to one,
	/// this field shall be included, otherwise this field shall be omitted.
	u_int8_t da[6];
	/// MSCS TID bitmap
	u_int8_t mscs_tid_bitmap;
};

struct __scs_rule {
	/// service prioritization rule identifier.
	u_int32_t id;
	/// add-remove filter rule bit.
	/// 1 means add 0 means delete
	u_int8_t add_delete_rule;
	/// rule precedence – higher number means higher priority.
	u_int8_t rule_precedence;
	/// rule output the value of or method  used to select the 802.1q c-tag
	/// pcp value with which  to mark the matched packet.
	u_int8_t rule_output;
	/// Match ip version type
	u_int8_t match_ip_version_type;
	/// match source mac address flag
	u_int8_t match_dst_mac;
	/// Match Destination port flags
	u_int8_t match_dst_port;
	/// Match Destination IPv4 Address flags
	u_int8_t match_dst_ipv4;
	/// match source mac address flag
	u_int8_t match_source_mac;
	/// Match Source port flag
	u_int8_t match_source_port;
	/// Match Source IPv4 Address flag
	u_int8_t match_source_ipv4;

	/// IP version type
	u_int8_t ip_version_type;

	/// destination mac address
	/// if “match destination mac address” flag bit is set to one,
	/// this field shall be included, otherwise this field shall be omitted.
	u_int8_t da[6];
	/// Destination Port
	u_int16_t dst_port;
	/// Destination IPv4 Address
	/// If “Match Destination IPv4 Address” flag bit is set to one,
	/// this field shall be included, otherwise this field shall be omitted.
	u_int32_t dst_ipv4_addr;

	/// source mac address
	/// if “match source mac address” flag bit is set to one,
	/// this field shall be included, otherwise this field shall be omitted.
	u_int8_t sa[6];
	/// Source Port
	u_int16_t src_port;
	/// Source IPv4 Address
	/// If “Match Source IPv4 Address” flag bit is set to one,
	/// this field shall be included, otherwise this field shall be omitted.
	u_int32_t src_ipv4_addr;

	/// DSCP value
	u_int8_t dscp;
};

#endif
