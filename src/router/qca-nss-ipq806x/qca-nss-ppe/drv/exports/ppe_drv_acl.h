/*
 * Copyright (c) 2023-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef _PPE_DRV_ACL_H_
#define _PPE_DRV_ACL_H_

/*
 * TODO: Separate internal and external export files
 */

#include <linux/if_ether.h>
#include <ppe_drv.h>

/*
 * General macros.
 */
#define PPE_DRV_ACL_SLICE_PER_ROW 8	/**< Number of ACL slices per row. */
#define PPE_DRV_ACL_MAX_ROW 64		/**< Number of ACL rows. */
#define PPE_DRV_ACL_MAX_ACTION 512	/**< Number of ACL actions. */
#define PPE_DRV_ACL_RULE_CHAIN_MAX 8	/**< Number of maximum ACL slices which can be chained together. */
#define PPE_DRV_ACL_HW_INDEX_MAX 1024	/**< Number of maximum ACL hw indexes. */

/*
 * Rule flags.
 */
#define PPE_DRV_ACL_VLAN_FLAG_VSI_VALID		0x00000001	/**< ACL rule VSI valid is enabled. */
#define PPE_DRV_ACL_VLAN_FLAG_VSI 		0x00000002	/**< ACL rule VSI is valid. */
#define PPE_DRV_ACL_VLAN_FLAG_STAG		0x00000004	/**< ACL rule S-tag format field. */
#define PPE_DRV_ACL_VLAN_FLAG_CTAG		0x00000008	/**< ACL rule C-tag format field. */
#define PPE_DRV_ACL_VLAN_FLAG_SVID		0x00000010	/**< ACL rule S-VID format field. */
#define PPE_DRV_ACL_VLAN_FLAG_CVID		0x00000020	/**< ACL rule C-VID format field. */
#define PPE_DRV_ACL_VLAN_FLAG_SPCP		0x00000040	/**< ACL rule S-PCP match value. */
#define PPE_DRV_ACL_VLAN_FLAG_CPCP		0x00000080	/**< ACL rule C-PCP match value. */
#define PPE_DRV_ACL_VLAN_FLAG_SDEI		0x00000100	/**< ACL rule S-DEI match field. */
#define PPE_DRV_ACL_VLAN_FLAG_CDEI		0x00000200	/**< ACL rule C-DEI match field. */

#define PPE_DRV_ACL_L2_MISC_FLAG_PPPOE 		0x00000001	/**< ACL rule PPPOE session ID field. */
#define PPE_DRV_ACL_L2_MISC_FLAG_L2PROTO	0x00000002	/**< ACL rule L2 protocol field. */
#define PPE_DRV_ACL_L2_FLAG_SVID		0x00000004	/**< ACL rule L2 S-VID field. */

#define PPE_DRV_ACL_DST_IPV4_FLAG_PKTTYPE	0x00000001	/**< ACL rule destination IPv4 packet type. */
#define PPE_DRV_ACL_DST_IPV4_FLAG_L3FRAG	0x00000002	/**< ACL rule destination IPv4 L3 fragmentation flag. */
#define PPE_DRV_ACL_DST_IPV4_FLAG_L4PORT	0x00000004	/**< ACL rule destination port with IPv4 packet. */
#define PPE_DRV_ACL_DST_IPV4_FLAG_IP		0x00000008	/**< ACL rule destination IPv4 field. */

#define PPE_DRV_ACL_SRC_IPV4_FLAG_PKTTYPE	0x00000001	/**< ACL rule source IPv4 packet type. */
#define PPE_DRV_ACL_SRC_IPV4_FLAG_L3FRAG	0x00000002	/**< ACL rule source IPv4 L3 fragmentation flag. */
#define PPE_DRV_ACL_SRC_IPV4_FLAG_L4PORT	0x00000004	/**< ACL rule source port with IPv4 packet. */
#define PPE_DRV_ACL_SRC_IPV4_FLAG_IP		0x00000008	/**< ACL rule source IPv4 field. */

#define PPE_DRV_ACL_DST_IPV6_FLAG_PKTTYPE	0x00000001	/**< ACL rule destination IPv6 packet type. */
#define PPE_DRV_ACL_DST_IPV6_FLAG_L3FRAG	0x00000002	/**< ACL rule destination IPv6 L3 fragmentation flag. */
#define PPE_DRV_ACL_DST_IPV6_FLAG_L4PORT	0x00000004	/**< ACL rule destination port with IPv6 packet. */
#define PPE_DRV_ACL_DST_IPV6_FLAG_IP		0x00000008	/**< ACL rule destination IPv6 field. */

#define PPE_DRV_ACL_SRC_IPV6_FLAG_PKTTYPE	0x00000001	/**< ACL rule source IPv6 packet type. */
#define PPE_DRV_ACL_SRC_IPV6_FLAG_L3FRAG	0x00000002	/**< ACL rule source IPv6 L3 fragmentation flag. */
#define PPE_DRV_ACL_SRC_IPV6_FLAG_L4PORT	0x00000004	/**< ACL rule source port with IPv6 packet. */
#define PPE_DRV_ACL_SRC_IPV6_FLAG_IP		0x00000008	/**< ACL rule source IPv6 field. */

#define PPE_DRV_ACL_IP_MISC_FLAG_L3FRAG 	0x00000001	/**< ACL rule with L3 fragmentation flag. */
#define PPE_DRV_ACL_IP_MISC_FLAG_OTHEXT_HDR	0x00000002	/**< ACL rule with other extended header. */
#define PPE_DRV_ACL_IP_MISC_FLAG_FRAG_HDR	0x00000004	/**< ACL rule with fragment header. */
#define PPE_DRV_ACL_IP_MISC_FLAG_MOB_HDR	0x00000008	/**< ACL rule with mobility header. */
#define PPE_DRV_ACL_IP_MISC_FLAG_ESP_HDR	0x00000010	/**< ACL rule with ESP header. */
#define PPE_DRV_ACL_IP_MISC_FLAG_AH_HDR		0x00000020	/**< ACL rule with AH header. */
#define PPE_DRV_ACL_IP_MISC_FLAG_TTLHOP		0x00000040	/**< ACL rule with TTL/hop-limit match. */
#define PPE_DRV_ACL_IP_MISC_FLAG_L3STATE	0x00000080	/**< ACL rule with L3 state flag. */
#define PPE_DRV_ACL_IP_MISC_FLAG_TCPFLAG	0x00000100	/**< ACL rule with TCP flags. */
#define PPE_DRV_ACL_IP_MISC_FLAG_L31STFRAG	0x00000200	/**< ACL rule with L3 first fragment. */
#define PPE_DRV_ACL_IP_MISC_FLAG_DSCPTC		0x00000400	/**< ACL rule with DSCP or TC value. */
#define PPE_DRV_ACL_IP_MISC_FLAG_L4PROTO	0x00000800	/**< ACL rule with L4 protocol number. */
#define PPE_DRV_ACL_IP_MISC_FLAG_L3LEN		0x00001000	/**< ACL rule with specific L3 length. */

/*
 * ACL action flags.
 */
#define PPE_DRV_ACL_ACTION_FLAG_CC 		0x00000001	/**< ACL action for CPU code change. */
#define PPE_DRV_ACL_ACTION_FLAG_SYN_TOGGLE	0x00000002	/**< ACL action for SYN_TOGGLE bit change. */
#define PPE_DRV_ACL_ACTION_FLAG_SC		0x00000004	/**< ACL action for service code change. */
#define PPE_DRV_ACL_ACTION_FLAG_QID		0x00000008	/**< ACL action for queue ID change. */
#define PPE_DRV_ACL_ACTION_FLAG_ENQUEUE_PRI	0x00000010	/**< ACL action for enqueue priority change. */
#define PPE_DRV_ACL_ACTION_FLAG_CTAG_DEI	0x00000020	/**< ACL action for C-DEI change. */
#define PPE_DRV_ACL_ACTION_FLAG_CTAG_PCP	0x00000040	/**< ACL action for C-PCP change. */
#define PPE_DRV_ACL_ACTION_FLAG_STAG_DEI	0x00000080	/**< ACL action for S-DEI change. */
#define PPE_DRV_ACL_ACTION_FLAG_STAG_PCP	0x00000100	/**< ACL action for C-PCP change. */
#define PPE_DRV_ACL_ACTION_FLAG_DSCP_TC		0x00000200	/**< ACL action for DSCP or TC change. */
#define PPE_DRV_ACL_ACTION_FLAG_CVID		0x00000400	/**< ACL action for C-VID change. */
#define PPE_DRV_ACL_ACTION_FLAG_SVID		0x00000800	/**< ACL action for S-VID change. */
#define PPE_DRV_ACL_ACTION_FLAG_DST_INFO	0x00001000	/**< ACL action for destination change. */
#define PPE_DRV_ACL_ACTION_FLAG_BYPASS_BITMAP	0x00002000	/**< ACL action with bypass bitmap. */
#define PPE_DRV_ACL_ACTION_FLAG_POLICER_INDEX	0x00004000	/**< ACL action with policer. */
#define PPE_DRV_ACL_ACTION_FLAG_FWD_CMD		0x00008000	/**< ACL action with forward command - fwd/drop/redirect/copy. */
#define PPE_DRV_ACL_ACTION_FLAG_INT_DP		0x00010000	/**< ACL action for internal drop precedence change. */
#define PPE_DRV_ACL_ACTION_FLAG_STAG_FMT_TAGGED	0x00020000	/**< ACL action for S-tag format change. */
#define PPE_DRV_ACL_ACTION_FLAG_CTAG_FMT_TAGGED	0x00040000	/**< ACL action for C-tag format change. */
#define PPE_DRV_ACL_ACTION_FLAG_MIRROR_EN	0x00080000	/**< ACL action for mirroring. */
#define PPE_DRV_ACL_ACTION_FLAG_METADATA_EN	0x00100000	/**< ACL action with Metadata enable. */

struct ppe_drv_acl_ctx;

/*
 * ppe_drv_acl_slice_type
 *	ACL slice types.
 */
enum ppe_drv_acl_slice_type {
	PPE_DRV_ACL_SLICE_TYPE_DST_MAC,		/**< ACL destination MAC slice type. */
	PPE_DRV_ACL_SLICE_TYPE_SRC_MAC,		/**< ACL source MAC slice type. */
	PPE_DRV_ACL_SLICE_TYPE_VLAN,		/**< ACL VLAN slice type. */
	PPE_DRV_ACL_SLICE_TYPE_L2_MISC,		/**< ACL L2 miscellaneous slice type. */
	PPE_DRV_ACL_SLICE_TYPE_DST_IPV4,	/**< ACL destination IPv4 slice type. */
	PPE_DRV_ACL_SLICE_TYPE_SRC_IPV4,	/**< ACL source IPv4 slice type. */
	PPE_DRV_ACL_SLICE_TYPE_DST_IPV6_0,	/**< ACL destination IPv6 0 slice type. */
	PPE_DRV_ACL_SLICE_TYPE_DST_IPV6_1,	/**< ACL destination IPv6 1 slice type. */
	PPE_DRV_ACL_SLICE_TYPE_DST_IPV6_2,	/**< ACL destination IPv6 2 slice type. */
	PPE_DRV_ACL_SLICE_TYPE_SRC_IPV6_0,	/**< ACL source IPv6 0 slice type. */
	PPE_DRV_ACL_SLICE_TYPE_SRC_IPV6_1,	/**< ACL source IPv6 1 slice type. */
	PPE_DRV_ACL_SLICE_TYPE_SRC_IPV6_2,	/**< ACL source IPv6 2 slice type. */
	PPE_DRV_ACL_SLICE_TYPE_IP_MISC,		/**< ACL IP miscellaneous slice type. */
	PPE_DRV_ACL_SLICE_TYPE_UDF_012,		/**< ACL UDF 012 slice type. */
	PPE_DRV_ACL_SLICE_TYPE_UDF_123,		/**< ACL UDF 123 slice type. */
	PPE_DRV_ACL_SLICE_TYPE_MAX,		/**< ACL slice type max. */
};

/*
 * ppe_drv_acl_src_type
 *	ACL source types.
 */
typedef enum ppe_drv_acl_src_type {
	PPE_DRV_ACL_SRC_TYPE_PORT_BITMAP,	/**< ACL source type bitmap. */
	PPE_DRV_ACL_SRC_TYPE_PORT_NUM,		/**< ACL source type port number. */
	PPE_DRV_ACL_SRC_TYPE_SC,		/**< ACL source type service code. */
	PPE_DRV_ACL_SRC_TYPE_DEST_L3		/**< ACL source type destination L3 interface. */
} ppe_drv_acl_src_type_t;

/*
 * ppe_drv_acl_dst_type
 *	ACL destination types.
 */
typedef enum ppe_drv_acl_dst_type {
	PPE_DRV_ACL_DST_TYPE_PORT_BITMAP,	/**< ACL destination type port bitmap. */
	PPE_DRV_ACL_DST_TYPE_NEXTHOP,		/**< ACL destination type nexthop index. */
	PPE_DRV_ACL_DST_TYPE_PORT_ID,		/**< ACL destination type port number. */
} ppe_drv_acl_dst_type_t;

/*
 * ppe_drv_acl_vtag_fmt
 *	VLAN tag format.
 */
typedef enum ppe_drv_acl_vtag_fmt {
	PPE_DRV_ACL_VTAG_FMT_TAG,		/**< ACL VLAN tag format. */
	PPE_DRV_ACL_VTAG_FMT_PRI_TAG,		/**< ACL VLAN priority tag format. */
	PPE_DRV_ACL_VTAG_FMT_UNTAG,		/**< ACL VLAN untag format. */
} ppe_drv_acl_vtag_fmt_t;

/*
 * ppe_drv_acl_pkt_type
 *	ACL packet types.
 */
typedef enum ppe_drv_acl_pkt_type {
	PPE_DRV_ACL_PKT_TYPE_TCP = 0,		/**< ACL TCP packet type. */
	PPE_DRV_ACL_PKT_TYPE_UDP = 1,		/**< ACL UDP packet type. */
	PPE_DRV_ACL_PKT_TYPE_UDPLITE = 3,	/**< ACL UDP-Lite packet type. */
	PPE_DRV_ACL_PKT_TYPE_ARP = 5,		/**< ACL ARP packet type. */
	PPE_DRV_ACL_PKT_TYPE_ICMP = 7		/**< ACL ICMP packet type. */
} ppe_drv_acl_pkt_type_t;

/*
 * ppe_drv_acl_ip_ver
 *	ACL rule IP version.
 */
typedef enum ppe_drv_acl_ip_ver {
	PPE_DRV_ACL_IPV4,			/**< ACL IPv4 packet. */
	PPE_DRV_ACL_IPV6			/**< ACL IPv6 packet. */
} ppe_drv_acl_ip_ver_t;

/*
 * ppe_drv_acl_fwd_cmd
 *	ACL action command
 */
typedef enum ppe_drv_acl_fwd_cmd {
	PPE_DRV_ACL_FWD_CMD_FWD,		/**< ACL forward command - forward. */
	PPE_DRV_ACL_FWD_CMD_DROP,		/**< ACL forward command - drop. */
	PPE_DRV_ACL_FWD_CMD_COPY,		/**< ACL forward command - copy to CPU. */
	PPE_DRV_ACL_FWD_CMD_REDIR		/**< ACL forward command - redirect to CPU. */
} ppe_drv_acl_fwd_cmd_t;

/*
 * ppe_drv_acl_dp
 *	ACL drop precedence
 */
typedef enum ppe_drv_acl_dp {
	PPE_DRV_ACL_DP_0,			/**< ACL drop precedence 0. */
	PPE_DRV_ACL_DP_1,			/**< ACL drop precedence 1. */
	PPE_DRV_ACL_DP_2,			/**< ACL drop precedence 2. */
	PPE_DRV_ACL_DP_3,			/**< ACL drop precedence 3. */
} ppe_drv_acl_dp_t;

/*
 * ppe_drv_acl_ipo
 *	ACL IPO type
 */
typedef enum ppe_drv_acl_ipo {
	PPE_DRV_ACL_IPO,			/**< ACL IPO rule. */
	PPE_DRV_ACL_PREIPO,			/**< ACL pre-IPO rule. */
} ppe_drv_acl_ipo_t;

/*
 * ppe_drv_acl_flow_bind
 *	Structure for flow and ACL binding.
 */
struct ppe_drv_acl_flow_bind {
	/*
	 * Input: ACL rule ID.
	 */
	uint16_t id;				/**< ACL rule ID. */

	/*
	 * Output: Associated service code.
	 */
	uint8_t sc;				/**< returned service code. */
};

/*
 * ppe_drv_acl_rule_match_cmn
 *	Rule info common to all the rules.
 */
struct ppe_drv_acl_rule_match_cmn {
	bool post_routing_en;		/**< Post routing enable flag. */
	uint8_t qos_res_pre;		/**< QOS resolution precedence. */
	uint8_t res_chain;		/**< Chain resolution. */
};

/*
 * ppe_drv_acl_mac
 *	MAC based ACL rule.
 */
struct ppe_drv_acl_mac {
	/*
	 * Rule
	 */
	uint8_t mac[ETH_ALEN];		/**< MAC address for match */

	uint8_t inverse_en;		/**< MAC address match with inverse logic. */
	uint8_t fake_mac_hdr;		/**< Fake mac header flag. */
	uint8_t snap_flag;		/**< SNAP flag. */
	uint8_t ethernet_flag;		/**< Ethernet flag. */
	uint8_t ip_ver;			/**< IP version. */
	uint8_t ip_nonip;		/**< IP no-IP flag. */

	/*
	 * Mask
	 */
	uint8_t fake_mac_hdr_mask;	/**< Fake mac header flag mask. */
	uint8_t snap_flag_mask;		/**< SNAP flag mask. */
	uint8_t ethernet_flag_mask;	/**< Ethernet flag mask. */
	uint8_t ip_ver_mask;		/**< IP version mask. */
	uint8_t ip_nonip_mask;		/**< IP no-IP flag mask. */
	uint8_t mac_mask[ETH_ALEN];	/**< MAC address mask. */
};

/*
 * ppe_drv_acl_vlan
 *	VLAN based ACL rule
 */
struct ppe_drv_acl_vlan {
	uint8_t vsi_valid;			/**< Match based on valid VSI. */
	uint8_t vsi;				/**< VSI value for match. */
	ppe_drv_acl_vtag_fmt_t stag_fmt;	/**< S-tag format. */
	ppe_drv_acl_vtag_fmt_t ctag_fmt;	/**< C-tag format. */
	uint8_t sdei_en;			/**< S-dei based VLAN match. */
	uint8_t spcp;				/**< SPCP based VLAN match. */
	uint16_t svid;				/**< SVID VLAN match. */
	uint8_t cdei_en;			/**< C-dei based VLAN match. */
	uint8_t cpcp;				/**< CPCP based VLAN match. */
	uint16_t cvid_min;			/**< CVID VLAN match. */

	uint8_t inverse_en;			/**< VLAN field match with inverse logic. */
	uint8_t range_en;			/**< VID range match. */
	uint8_t fake_mac_hdr;			/**< Fake mac header flag. */
	uint8_t snap_flag;			/**< SNAP flag. */
	uint8_t ethernet_flag;			/**< Ethernet flag. */
	uint8_t ip_ver;				/**< IP version. */
	uint8_t ip_nonip;			/**< IP no-IP flag. */

	/*
	 * Mask
	 */
	uint8_t fake_mac_hdr_mask;		/**< Fake mac header flag mask. */
	uint8_t snap_flag_mask;			/**< SNAP flag mask. */
	uint8_t ethernet_flag_mask;		/**< Ethernet flag mask. */
	uint8_t ip_ver_mask;			/**< IP version mask. */
	uint8_t ip_nonip_mask;			/**< IP no-IP flag mask. */
	uint8_t vsi_valid_mask;			/**< VSI valid mask. */
	uint8_t vsi_mask;			/**< VSI value mask. */
	uint8_t sdei_en_mask;			/**< S-dei mask. */
	uint8_t spcp_mask;			/**< SPCP mask. */
	uint16_t svid_mask;			/**< SVID mask. */
	uint8_t cdei_en_mask;			/**< C-dei mask. */
	uint8_t cpcp_mask;			/**< CPCP mask. */
	uint16_t cvid_mask_max;			/**< CVID mask or max for range. */
};

/*
 * ppe_drv_acl_l2_misc
 *
 */
struct ppe_drv_acl_l2_misc {
	uint16_t pppoe_session_id;		/**< PPPOE session ID match. */
	uint16_t l2_proto;			/**< Ether type based match. */
	uint16_t svid_min;			/**< SVID range match. */

	uint8_t inverse_en;			/**< L2 field match with inverse logic. */
	uint8_t range_en;			/**< SVID range match. */
	uint8_t fake_mac_hdr;			/**< Fake mac header flag. */
	uint8_t snap_flag;			/**< SNAP flag. */
	uint8_t ethernet_flag;			/**< Ethernet flag. */
	uint8_t ip_ver;				/**< IP version. */
	uint8_t ip_nonip;			/**< IP no-IP flag. */

	/*
	 * Mask
	 */
	uint8_t fake_mac_hdr_mask;		/**< Fake mac header flag mask. */
	uint8_t snap_flag_mask;			/**< SNAP flag mask. */
	uint8_t ethernet_flag_mask;		/**< Ethernet flag mask. */
	uint8_t ip_ver_mask;			/**< IP version mask. */
	uint8_t ip_nonip_mask;			/**< IP no-IP flag mask. */
	uint16_t pppoe_session_id_mask;		/**< PPPOE session ID mask. */
	uint16_t l2_proto_mask;			/**< Ether type mask. */
	uint16_t svid_mask_max;			/**< SVID max value for range. */
};

/*
 * ppe_drv_acl_ipv4
 *
 */
struct ppe_drv_acl_ipv4 {
	ppe_drv_acl_pkt_type_t pkt_type;	/**< Packet type based match. */
	uint8_t l3_fragment_flag;		/**< L3 fragment flag match. */
	uint16_t l4_port_min;			/**< L4 port based match. */
	uint32_t ip;				/**< IPv4 address match. */

	uint8_t inverse_en;			/**< IPv4 match with inverse logic. */
	uint8_t range_en;			/**< L4 port range match. */
	uint8_t ip_nonip;			/**< IP no-IP flag. */

	/*
	 * Mask
	 */
	uint8_t ip_nonip_mask;			/**< IP no-IP flag mask. */
	uint8_t pkt_type_mask;			/**< packet type mask. */
	uint8_t l3_fragment_flag_mask;		/**< L3 fragment flag mask. */
	uint32_t ip_mask;			/**< IPv4 address mask. */
	uint16_t l4_port_mask_max;		/**< L4 port mask or max value for range. */
};

/*
 * ppe_drv_acl_ipv6
 *
 */
struct ppe_drv_acl_ipv6 {
	ppe_drv_acl_pkt_type_t pkt_type;	/**< Packet type based match. */
	uint8_t l3_fragment_flag;		/**< L3 fragment flag match. */
	uint32_t ip_l32;			/**< Lower 32 bit IP address. */
	union {
		uint16_t ip_u16;		/**< Upper 16 bit IP address. */
		uint16_t l4_port_min;		/**< L4 port based match. */
	};

	uint8_t inverse_en;			/**< IPv6 match with inverse logic. */
	uint8_t range_en;			/**< L4 port range match. */

	/*
	 * Mask
	 */
	uint8_t pkt_type_mask;			/**< packet type mask. */
	uint8_t l3_fragment_flag_mask;		/**< L3 fragment flag mask. */
	uint32_t ip_l32_mask;			/**< Lower 32 bit IP address mask. */
	union {
		uint16_t ip_u16_mask;		/**< Upper 16 bit IP address mask. */
		uint16_t l4_port_mask_max;	/**< L4 port mask or max for range. */
	};
};

/*
 * ppe_drv_acl_ip_misc
 *
 */
struct ppe_drv_acl_ip_misc {
	ppe_drv_acl_ip_ver_t ip_ver;		/**< IP version based match. */
	uint8_t l3_fragment_flag;		/**< L3 fragment flag match. */
	uint8_t other_extension_hdr_flag;	/**< Other extension header flag match. */
	uint8_t frag_hdr_flag;			/**< Frag header flag match. */
	uint8_t mobility_hdr_flag;		/**< Mobility header flag match. */
	uint8_t esp_hdr_flag;			/**< ESP header flag match. */
	uint8_t ah_hdr_flag;			/**< AH header flag match. */
	uint8_t hop_limit;			/**< TTL/Hop-limit based match. */
	uint8_t l3_state_option_flag;		/**< L3 state option flag match. */
	uint8_t tcp_flags;			/**< TCP flags based match. */
	uint8_t l3_1st_fragment_flag;		/**< L3 first fragment flag match. */
	uint8_t l3_dscp_tc;			/**< L3 DSCP/TC based match. */
	uint8_t l3_v4proto_v6nexthdr;		/**< L4 proto/nexth-header based match. */
	uint16_t l3_length;			/**< L3 length based match. */

	uint8_t inverse_en;			/**< L3 fields match with inverse logic. */
	uint8_t range_en;			/**< L3 length range match. */

	/*
	 * Mask
	 */
	uint8_t ip_ver_mask;			/**< IP version mask. */
	uint8_t l3_fragment_flag_mask;		/**< L3 fragment flag mask. */
	uint8_t other_extension_hdr_flag_mask;	/**< Other extension header flag mask. */
	uint8_t frag_hdr_flag_mask;		/**< Frag header flag mask. */
	uint8_t mobility_hdr_flag_mask;		/**< Mobility header flag mask. */
	uint8_t esp_hdr_flag_mask;		/**< ESP header flag mask. */
	uint8_t ah_hdr_flag_mask;		/**< AH header flag mask. */
	uint8_t hop_limit_mask;			/**< TTL/Hop-limit based mask. */
	uint8_t l3_state_option_flag_mask;	/**< L3 state option flag mask. */
	uint8_t tcp_flags_mask;			/**< TCP flags mask. */
	uint8_t l3_1st_fragment_flag_mask;	/**< L3 first fragment flag mask. */
	uint8_t l3_dscp_tc_mask;		/**< L3 DSCP/TC mask. */
	uint8_t l3_v4proto_v6nexthdr_mask;	/**< L4 proto/nexth-header mask. */
	uint16_t l3_length_mask_max;		/**< L3 length mask or max for range. */
};

/*
 * ppe_drv_acl_udf
 *
 */
struct ppe_drv_acl_udf {
	uint8_t udf_a_valid;			/**< First UDF valid for match. */
	uint8_t udf_b_valid;			/**< Second UDF valid for match. */
	uint8_t udf_c_valid;			/**< Third UDF valid for match. */
	uint16_t udf_a_min;			/**< First UDF value for match. */
	uint16_t udf_b;				/**< Second UDF value for match. */
	uint16_t udf_c;				/**< Third UDF value for match. */

	uint8_t inverse_en;			/**< UDF match with inverse logic. */
	uint8_t range_en;			/**< First UDF range match. */
	uint8_t ip_ver;				/**< IP version based match. */
	uint8_t ip_nonip;			/**< IP no-IP flag match. */

	/*
	 * Mask
	 */
	uint8_t ip_ver_mask;			/**< IP version mask. */
	uint8_t ip_nonip_mask;			/**< IP no-IP flag mask. */
	uint8_t udf_a_valid_mask;		/**< First UDF valid mask. */
	uint8_t udf_b_valid_mask;		/**< Second UDF valid mask. */
	uint8_t udf_c_valid_mask;		/**< Third UDF valid mask. */
	uint16_t udf_a_mask_max;		/**< First UDF value mask. */
	uint16_t udf_b_mask;			/**< Second UDF value mask. */
	uint16_t udf_c_mask;			/**< Third UDF value mask. */
};

/*
 * ppe_drv_acl_rule_match_one
 *	Single ACL rule info
 */
struct ppe_drv_acl_rule_match_one {
	bool valid;					/**< Slice valid. */
	uint32_t flags;					/**< Slice flag. */
	uint8_t sub_rule_cnt;				/**< Number of sub rules. */
	enum ppe_drv_acl_slice_type type;		/**< Slice type. */
	union {
		struct ppe_drv_acl_mac smac;		/**< SMAC rule slice. */
		struct ppe_drv_acl_mac dmac;		/**< DMAC rule slice. */
		struct ppe_drv_acl_vlan vlan;		/**< VLAN rule slice. */
		struct ppe_drv_acl_l2_misc l2;		/**< L2 field slice. */
		struct ppe_drv_acl_ipv4 sip_v4;		/**< Source IPv4 rule slice. */
		struct ppe_drv_acl_ipv4 dip_v4;		/**< Destination IPv4 rule slice. */
		struct ppe_drv_acl_ipv6 dip_v6_0;	/**< Destination IPv6 rule slice 0. */
		struct ppe_drv_acl_ipv6 dip_v6_1;	/**< Destination IPv6 rule slice 1. */
		struct ppe_drv_acl_ipv6 dip_v6_2;	/**< Destination IPv6 rule slice 2. */
		struct ppe_drv_acl_ipv6 sip_v6_0;	/**< Source IPv6 rule slice 0. */
		struct ppe_drv_acl_ipv6 sip_v6_1;	/**< Source IPv6 rule slice 1. */
		struct ppe_drv_acl_ipv6 sip_v6_2;	/**< Source IPv6 rule slice 2. */
		struct ppe_drv_acl_ip_misc ip;		/**< L3 field slice. */
		struct ppe_drv_acl_udf udf_012;		/**< UDF slice 0. */
		struct ppe_drv_acl_udf udf_123;		/**< UDF slice 1. */
	} rule;
};

/*
 * ppe_drv_acl_action
 *	ACL rule action
 */
struct ppe_drv_acl_action {
	/*
	 * Action values.
	 */
	uint8_t cpu_code;			/**< CPU code change action. */
	uint8_t syn_toggle;			/**< SYN_TOGGLE bit change action. */
	uint8_t service_code;			/**< Service code change action. */
	uint8_t qid;				/**< QID change action. */
	uint8_t enqueue_pri;			/**< Enqueue priority change action. */
	uint8_t ctag_dei;			/**< C-DEI change action. */
	uint8_t ctag_pcp;			/**< C-PCP change action. */
	uint8_t stag_dei;			/**< S-DEI change action. */
	uint8_t stag_pcp;			/**< S-PCP change action. */
	uint8_t dscp_tc;			/**< DSCP change action. */
	uint16_t cvid;				/**< C-VID change action. */
	uint16_t svid;				/**< S-VID change action. */
	ppe_drv_acl_dst_type_t dest_type;	/**< Destination type. */
	uint16_t dest_info;			/**< Destination change action. */
	uint16_t policer_index;			/**< Policer action. */
	uint32_t bypass_bitmap;			/**< Bypass bitmap. */
	ppe_drv_acl_fwd_cmd_t fwd_cmd;		/**< Forward command action. */
	enum ppe_drv_acl_dp int_dp;		/**< Internal drop precedence action. */
	ppe_drv_acl_vtag_fmt_t ctag_fmt;	/**< C-tag format action. */
	ppe_drv_acl_vtag_fmt_t stag_fmt;	/**< S-tag format action. */

	/*
	 * Action control flags.
	 */
	uint32_t flags;				/**< Action flags. */
};

/*
 * ppe_drv_acl_rule
 *	ACL rule and action information.
 */
struct ppe_drv_acl_rule {
	struct ppe_drv_acl_rule_match_cmn cmn;					/**< Common fields for slices. */
	struct ppe_drv_acl_rule_match_one chain[PPE_DRV_ACL_SLICE_TYPE_MAX];	/**< Array of ACL slices. */
	struct ppe_drv_acl_action action;					/**< ACL action information. */
	ppe_drv_acl_src_type_t stype;						/**< Source type for ACL binding. */
	uint8_t src;								/**< ACL source binding. */
};

/*
 * ppe_drv_acl_hw_info
 *	ACL hardware rule information
 */
struct ppe_drv_acl_hw_info {
	uint16_t hw_rule_id;		/**< ACL hardware rule ID. */
	uint16_t hw_list_id;		/**< ACL hardware list ID. */
	uint16_t hw_num_slices;		/**< Number of slices occupied. */
};

/**
 * ppe_drv_acl_metadata
 *	ACL rule metadata for packet processing based on ACL rule.
 *	This is a common structure for sending metadata from PPE driver
 *	to PPE rule module for different use cases like mirror, redirect etc.
 */
struct ppe_drv_acl_metadata {
	uint16_t acl_hw_index;		/**< ACL hardware index got from EDMA descriptor. */
	uint16_t cpu_code;		/**< CPU code from EDMA descriptor. */
	uint16_t acl_id;		/**< ACL software index. */
};

/**
 * ppe_drv_acl_rule_callback_t
 *	Function callback type for ACL tagged packet processing.
 *
 * @datatypes
 * ppe_drv_acl_metadata
 *
 * @param[IN] app_data		Pointer to void context from caller.
 * @param[IN] skb		skb pointer.
 * @param[IN] acl_info		ACL information.
 *
 * @return
 * true for success and false for failure.
 */
typedef bool (*ppe_drv_acl_rule_callback_t)(void *app_data, struct sk_buff *skb, struct ppe_drv_acl_metadata *acl_info);

/**
 * ppe_drv_acl_flow_callback_t
 *	Function callback type for flow add and flow delete.
 *
 * @datatypes
 * ppe_drv_acl_flow_bind
 *
 * @param[IN] app_data		Pointer to void context from caller.
 * @param[IN] info 		Flow information object.
 *
 * @return
 * true for success and false for failure.
 */
typedef bool (*ppe_drv_acl_flow_callback_t)(void *app_data, struct ppe_drv_acl_flow_bind *info);

/**
 * ppe_drv_acl_mirror_core_select_cb_t
 *	Function callback type for mirror core selection.
 *
 * @param[IN] core_id		core id for mirrored packets.
 * @param[IN] app_data		Pointer to void context from caller.
 *
 * @return
 * true for success and false for failure.
 */
typedef void (*ppe_drv_acl_mirror_core_select_cb_t)(uint8_t core_id, void *app_data);

/*
 * ppe_drv_acl_process_callback_t
 *	Funtion callback type for processing based on ACL rule.
 */
typedef bool (*ppe_drv_acl_process_callback_t)(void *app_data, struct sk_buff *skb, void *acl_metadata);

/**
 * ppe_drv_acl_sc_return()
 *      Return a flow ACL service code.
 *
 * @param[IN] sc	Service code to be returned to free pool.
 *
 * @return
 * void
 */
void ppe_drv_acl_sc_return(uint8_t sc);

/*
 * ppe_drv_acl_sc_get()
 *      Reserve an unused flow ACL service code.
 *
 * @return
 * Service code
 */
uint8_t ppe_drv_acl_sc_get(void);

/*
 * ppe_drv_acl_get_hw_stats()
 *	Return hardware stats for an ACL rule.
 *
 * @datatype
 * struct ppe_drv_acl_ctx
 *
 * @param[OUT] pkts	Returned hardware packet counters.
 * @param[OUT] bytes	Returned hardware byte counters.
 *
 * @return
 * void
 */
void ppe_drv_acl_get_hw_stats(struct ppe_drv_acl_ctx *ctx, uint64_t *pkts, uint64_t *bytes);

/*
 * ppe_drv_acl_hw_info_get()
 *	Return hardware information associated with an ACL rule.
 *
 * @datatype
 * struct ppe_drv_acl_ctx
 * struct ppe_drv_acl_hw_info
 *
 * @param[IN]  ctx		ACL rule context pointer.
 * @param[OUT] hw_info		Returned hardware rule information.
 *
 * @return
 * void
 */
void ppe_drv_acl_hw_info_get(struct ppe_drv_acl_ctx *ctx, struct ppe_drv_acl_hw_info *hw_info);

/**
 * ppe_drv_acl_rule_unregister_cb
 *	Callback unregistration for ACL rule process callback.
 *
 * @return
 * void
 */
void ppe_drv_acl_rule_unregister_cb(void);

/**
 * ppe_drv_acl_rule_register_cb
 *	Register the callback for ACL rule processing.
 *
 * @datatypes
 * ppe_drv_acl_rule_callback_t
 *
 * @param[IN] acl_cb		Pointer to acl callback.
 * @param[IN] app_data		Pointer to void context from caller.
 *
 * @return
 * void.
 */
void ppe_drv_acl_rule_register_cb(ppe_drv_acl_rule_callback_t acl_cb, void *app_data);

/**
 * ppe_drv_acl_flow_unregister_cb
 *	Callback unregistration for flow + ACL combination.
 *
 * @return
 * void
 */
void ppe_drv_acl_flow_unregister_cb(void);

/**
 * ppe_drv_acl_flow_register_cb
 *	Register the callback for flow add and delete
 *
 * @datatypes
 * ppe_drv_acl_flow_callback_t
 * ppe_drv_acl_flow_callback_t
 *
 * @param[IN] add_cb		Pointer to add callback.
 * @param[IN] del_cb		Pointer to delete callback.
 * @param[IN] app_data 		Pointer to void context from caller.
 *
 * @return
 * true for success and false for failure.
 */
void ppe_drv_acl_flow_register_cb(ppe_drv_acl_flow_callback_t add_cb, ppe_drv_acl_flow_callback_t del_cb, void *app_data);

/**
 * ppe_drv_acl_mirror_core_select_unregister_cb
 *	Unegister the mirror core selection API with PPE driver.
 *
 * @return
 * None.
 */
void ppe_drv_acl_mirror_core_select_unregister_cb(void);

/**
 * ppe_drv_acl_mirror_core_select_register_cb
 *	Register the mirror core selection API with PPE driver.
 *
 * @datatypes
 * ppe_drv_acl_mirror_core_select_cb_t
 *
 * @param[IN] cb		Mirror core register callback into DP.
 * @param[IN] app_data		Pointer to void context from caller.
 *
 * @return
 * none.
 */
void ppe_drv_acl_mirror_core_select_register_cb(ppe_drv_acl_mirror_core_select_cb_t cb, void *app_data);

/*
 * ppe_drv_acl_destroy()
 *	Destroy an ACL rule.
 */
void ppe_drv_acl_destroy(struct ppe_drv_acl_ctx *ctx);

/**
 * ppe_drv_acl_rule_configure()
 *	Configure slices for ACL rule.
 *
 * @datatypes
 * ppe_drv_acl_ctx
 * ppe_drv_acl_rule
 *
 * @param[IN] ctx		Pointer to ACL context.
 * @param[IN] info		Pointer to ACL info object.
 *
 * @return
 * Status of configuration operation.
 */
ppe_drv_ret_t ppe_drv_acl_configure(struct ppe_drv_acl_ctx *ctx, struct ppe_drv_acl_rule *info);

/*
 * ppe_drv_acl_alloc()
 *	Allocate slices for ACL rules.
 *
 * @datatypes
 * ppe_drv_acl_ipo_t
 *
 * @param[IN] type		ACL IPO type.
 * @param[IN] num_slices	Number of slices needed for a specific ACL rule.
 * @param[IN] pri		Priority of the rule.
 *
 * @return
 * Status of configuration operation.
 */
struct ppe_drv_acl_ctx *ppe_drv_acl_alloc(ppe_drv_acl_ipo_t type, uint8_t num_slices, uint16_t pri);

/**
 * ppe_drv_acl_enable_mirror_capture_core
 *	Enable capture core for mirrored packets.
 *
 * @param[IN] core_id		Capture core to enable.
 *
 * @return
 * Success or failure of the API.
 */
bool ppe_drv_acl_enable_mirror_capture_core(uint8_t core_id);

/**
 * ppe_drv_acl_process_skbuff
 *	Process packets with a valid ACL id.
 *
 * @datatypes
 * ppe_drv_acl_metadata
 * sk_buff
 *
 * @param[IN] acl_info		ACL rule related information.
 * @param[IN] skb		Socket buffer pointer.
 *
 * @return
 * Status of process operation.
 */
bool ppe_drv_acl_process_skbuff(struct ppe_drv_acl_metadata *acl_info, struct sk_buff *skb);

#endif
