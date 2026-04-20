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
/**
 * @file ppe_acl.h
 *	NSS PPE ACL definitions.
 */

#ifndef _PPE_ACL_H_
#define _PPE_ACL_H_

#include <linux/if.h>
#include <linux/if_ether.h>

#define PPE_ACL_PRI_NOMINAL 1
#define PPE_ACL_INVALID_HW_INDEX 0xFFFF
#define PPE_ACL_HW_INDEX_MAX 1024
#define PPE_ACL_GROUP_MAX 2
#define PPE_ACL_GROUP_DEFAULT 0

/**
 * ACL rule ID
 */
typedef int16_t ppe_acl_rule_id_t;

/**
 * ppe_acl_rule_match_type
 *	ACL rule types.
 */
typedef enum ppe_acl_rule_match_type {
	PPE_ACL_RULE_MATCH_TYPE_INVALID,		/**< Invalid rule type. */
	PPE_ACL_RULE_MATCH_TYPE_SMAC,			/**< SMAC rule type. */
	PPE_ACL_RULE_MATCH_TYPE_DMAC,			/**< DMAC rule type. */
	PPE_ACL_RULE_MATCH_TYPE_SVID,			/**< SVID rule type. */
	PPE_ACL_RULE_MATCH_TYPE_CVID,			/**< CVID rule type. */
	PPE_ACL_RULE_MATCH_TYPE_SPCP,			/**< SPCP rule type. */
	PPE_ACL_RULE_MATCH_TYPE_CPCP,			/**< CPCP rule type. */
	PPE_ACL_RULE_MATCH_TYPE_PPPOE_SESS,		/**< PPPOE session rule type. */
	PPE_ACL_RULE_MATCH_TYPE_ETHER_TYPE,		/**< Ether type rule */
	PPE_ACL_RULE_MATCH_TYPE_L3_1ST_FRAG,		/**< First IP fragment rule type. */
	PPE_ACL_RULE_MATCH_TYPE_IP_LEN,		/**< IP length rule type. */
	PPE_ACL_RULE_MATCH_TYPE_TTL_HOPLIMIT,		/**< TTL/hop-limit rule type. */
	PPE_ACL_RULE_MATCH_TYPE_DSCP_TC,		/**< DSCP/traffic-class rule type. */
	PPE_ACL_RULE_MATCH_TYPE_PROTO_NEXTHDR,	/**< L4 proto rule type. */
	PPE_ACL_RULE_MATCH_TYPE_TCP_FLAG,		/**< TCP flags rule type. */
	PPE_ACL_RULE_MATCH_TYPE_SIP,			/**< Source IP rule type. */
	PPE_ACL_RULE_MATCH_TYPE_DIP,			/**< Destination rule type. */
	PPE_ACL_RULE_MATCH_TYPE_SPORT,		/**< L4 source port rule type. */
	PPE_ACL_RULE_MATCH_TYPE_DPORT,		/**< L4 destination port rule type. */
	PPE_ACL_RULE_MATCH_TYPE_IP_GEN,		/**< General IP fields rule type. */
	PPE_ACL_RULE_MATCH_TYPE_UDF,			/**< User Defined fields (UDF) rule type. */
	PPE_ACL_RULE_MATCH_TYPE_DEFAULT,		/**< Default rule type. */
	PPE_ACL_RULE_MATCH_TYPE_MAX,			/**< Maximum rule type. */
} ppe_acl_rule_match_type_t;

/*
 * Rule type valid flag
 */
#define PPE_ACL_RULE_MATCH_TYPE_SMAC_VALID		(1 << PPE_ACL_RULE_MATCH_TYPE_SMAC)
#define PPE_ACL_RULE_MATCH_TYPE_DMAC_VALID		(1 << PPE_ACL_RULE_MATCH_TYPE_DMAC)
#define PPE_ACL_RULE_MATCH_TYPE_SVID_VALID		(1 << PPE_ACL_RULE_MATCH_TYPE_SVID)
#define PPE_ACL_RULE_MATCH_TYPE_CVID_VALID		(1 << PPE_ACL_RULE_MATCH_TYPE_CVID)
#define PPE_ACL_RULE_MATCH_TYPE_SPCP_VALID		(1 << PPE_ACL_RULE_MATCH_TYPE_SPCP)
#define PPE_ACL_RULE_MATCH_TYPE_CPCP_VALID		(1 << PPE_ACL_RULE_MATCH_TYPE_CPCP)
#define PPE_ACL_RULE_MATCH_TYPE_PPPOE_SESS_VALID	(1 << PPE_ACL_RULE_MATCH_TYPE_PPPOE_SESS)
#define PPE_ACL_RULE_MATCH_TYPE_ETHER_TYPE_VALID	(1 << PPE_ACL_RULE_MATCH_TYPE_ETHER_TYPE)
#define PPE_ACL_RULE_MATCH_TYPE_L3_1ST_FRAG_VALID	(1 << PPE_ACL_RULE_MATCH_TYPE_L3_1ST_FRAG)
#define PPE_ACL_RULE_MATCH_TYPE_IP_LEN_VALID		(1 << PPE_ACL_RULE_MATCH_TYPE_IP_LEN)
#define PPE_ACL_RULE_MATCH_TYPE_TTL_HOPLIMIT_VALID	(1 << PPE_ACL_RULE_MATCH_TYPE_TTL_HOPLIMIT)
#define PPE_ACL_RULE_MATCH_TYPE_DSCP_TC_VALID		(1 << PPE_ACL_RULE_MATCH_TYPE_DSCP_TC)
#define PPE_ACL_RULE_MATCH_TYPE_PROTO_NEXTHDR_VALID	(1 << PPE_ACL_RULE_MATCH_TYPE_PROTO_NEXTHDR)
#define PPE_ACL_RULE_MATCH_TYPE_TCP_FLAG_VALID	(1 << PPE_ACL_RULE_MATCH_TYPE_TCP_FLAG)
#define PPE_ACL_RULE_MATCH_TYPE_SIP_VALID		(1 << PPE_ACL_RULE_MATCH_TYPE_SIP)
#define PPE_ACL_RULE_MATCH_TYPE_DIP_VALID		(1 << PPE_ACL_RULE_MATCH_TYPE_DIP)
#define PPE_ACL_RULE_MATCH_TYPE_SPORT_VALID		(1 << PPE_ACL_RULE_MATCH_TYPE_SPORT)
#define PPE_ACL_RULE_MATCH_TYPE_DPORT_VALID		(1 << PPE_ACL_RULE_MATCH_TYPE_DPORT)
#define PPE_ACL_RULE_MATCH_TYPE_IP_GEN_VALID		(1 << PPE_ACL_RULE_MATCH_TYPE_IP_GEN)
#define PPE_ACL_RULE_MATCH_TYPE_UDF_VALID		(1 << PPE_ACL_RULE_MATCH_TYPE_UDF)
#define PPE_ACL_RULE_MATCH_TYPE_DEFAULT_VALID		(1 << PPE_ACL_RULE_MATCH_TYPE_DEFAULT)

/*
 * ACL rule flag common
 */
#define PPE_ACL_RULE_CMN_FLAG_SNAP			0x00000001	/**< Rule common flag for SNAP packets. */
#define PPE_ACL_RULE_CMN_FLAG_FAKE_MAC			0x00000002	/**< Rule common flag for packets with fake mac header. */
#define PPE_ACL_RULE_CMN_FLAG_ETHERNET			0x00000004	/**< Rule common flag for ACL match for ethernet packet. */
#define PPE_ACL_RULE_CMN_FLAG_IPV4			0x00000008	/**< Rule common flag for ACL match for IPv4 packets. */
#define PPE_ACL_RULE_CMN_FLAG_IPV6			0x00000010	/**< Rule common flag for ACL match for IPv6 packets. */
#define PPE_ACL_RULE_CMN_FLAG_NON_IP			0x00000020	/**< Rule common flag for ACL match for non-IP packets. */
#define PPE_ACL_RULE_CMN_FLAG_PRI_EN			0x00000040	/**< Rule common flag to indicate priority configuration. */
#define PPE_ACL_RULE_CMN_FLAG_POST_RT_EN		0x00000080	/**< Rule common flag to enable match for post routing fields. */
#define PPE_ACL_RULE_CMN_FLAG_OUTER_HDR_MATCH		0x00000100	/**< Rule common flag to enable match on tunnel outer headers fields. */
#define PPE_ACL_RULE_CMN_FLAG_NO_RULEID			0x00000200	/**< Rule common flag to indicate rule ID is not passed by caller. */
#define PPE_ACL_RULE_CMN_FLAG_FLOW_QOS_OVERRIDE		0x00000400	/**< Rule common flag to override QOS parameters from flow entry. */
#define PPE_ACL_RULE_CMN_FLAG_METADATA_EN		0x00000800	/**< Rule common flag to indicate METADATA ENABLE in ACL rule. */
#define PPE_ACL_RULE_CMN_FLAG_GROUP_EN			0x00001000	/**< Rule common flag to indicate priority configuration. */

/*
 * ACL rule flag general - applicable for each rule separately.
 */
#define PPE_ACL_RULE_GEN_FLAG_INVERSE_EN        0x00000001				/**< General flag for inverse matching. */
#define PPE_ACL_RULE_GEN_FLAG_LAST		PPE_ACL_RULE_GEN_FLAG_INVERSE_EN	/**< Last general flag. */

/*
 * VLAN ACL rule flag
 */
#define PPE_ACL_RULE_FLAG_DEI_EN		(PPE_ACL_RULE_GEN_FLAG_LAST << 1)	/**< Rule match with DEI bit in 1p header. */
#define PPE_ACL_RULE_FLAG_SVID_RANGE 		(PPE_ACL_RULE_GEN_FLAG_LAST << 2)	/**< Rule match with S-VID range. */
#define PPE_ACL_RULE_FLAG_VID_RANGE		(PPE_ACL_RULE_GEN_FLAG_LAST << 3)	/**< Rule match with C-VID range. */
#define PPE_ACL_RULE_FLAG_VID_MASK		(PPE_ACL_RULE_GEN_FLAG_LAST << 4)	/**< Rule match with VID mask. */
#define PPE_ACL_RULE_FLAG_PCP_MASK		(PPE_ACL_RULE_GEN_FLAG_LAST << 5)	/**< Rule match with 802.1p mask. */

/*
 * L3 length rule flag.
 */
#define PPE_ACL_RULE_FLAG_IPLEN_MASK		(PPE_ACL_RULE_GEN_FLAG_LAST << 1)	/**< Rule match with L3 length mask. */
#define PPE_ACL_RULE_FLAG_IPLEN_RANGE		(PPE_ACL_RULE_GEN_FLAG_LAST << 2)	/**< Rule match with L3 length range. */

/*
 * MAC ACL rule flag.
 */
#define PPE_ACL_RULE_FLAG_MAC_MASK 		(PPE_ACL_RULE_GEN_FLAG_LAST << 1)	/**< Rule match with MAC mask. */

/*
 * PPPOE session rule flag.
 */
#define PPE_ACL_RULE_FLAG_PPPOE_MASK		(PPE_ACL_RULE_GEN_FLAG_LAST << 1)	/**< Rule match with PPPOE session mask. */

/*
 * Ethertype ACL rule flag.
 */
#define PPE_ACL_RULE_FLAG_ETHTYPE_MASK		(PPE_ACL_RULE_GEN_FLAG_LAST << 1)	/**< Rule match with ethertype mask. */

/*
 * TTL/HOPLIMIT rule flag.
 */
#define PPE_ACL_RULE_FLAG_TTL_HOPLIMIT_MASK	(PPE_ACL_RULE_GEN_FLAG_LAST << 1)	/**< Rule match with TTL/hop-limit mask. */

/*
 * DSCP/TC rule flag.
 */
#define PPE_ACL_RULE_FLAG_DSCP_TC_MASK		(PPE_ACL_RULE_GEN_FLAG_LAST << 1)	/**< Rule match with DSCP/TC mask. */

/*
 * L4 protocol rule flag.
 */
#define PPE_ACL_RULE_FLAG_PROTO_NEXTHDR_MASK	(PPE_ACL_RULE_GEN_FLAG_LAST << 1)	/**< Rule match with proto/next-header mask. */

/*
 * IP general flags.
 */
#define PPE_ACL_RULE_FLAG_L3_FRAG		(PPE_ACL_RULE_GEN_FLAG_LAST << 1)	/**< Rule match with IP fragment header. */
#define PPE_ACL_RULE_FLAG_ESP_HDR		(PPE_ACL_RULE_GEN_FLAG_LAST << 2)	/**< Rule match with ESP header. */
#define PPE_ACL_RULE_FLAG_AH_HDR		(PPE_ACL_RULE_GEN_FLAG_LAST << 3)	/**< Rule match with auth header. */
#define PPE_ACL_RULE_FLAG_MOBILITY_HDR		(PPE_ACL_RULE_GEN_FLAG_LAST << 4)	/**< Rule match with mobility header. */
#define PPE_ACL_RULE_FLAG_OTHER_EXT_HDR		(PPE_ACL_RULE_GEN_FLAG_LAST << 5)	/**< Rule match with other IPv6 header. */
#define PPE_ACL_RULE_FLAG_FRAG_HDR		(PPE_ACL_RULE_GEN_FLAG_LAST << 6)	/**< Rule match with fragment IPv6 header. */
#define PPE_ACL_RULE_FLAG_IPV4_OPTION		(PPE_ACL_RULE_GEN_FLAG_LAST << 7)	/**< Rule match with IPv4 option. */

/*
 * Tcp-flags rule flag.
 */
#define PPE_ACL_RULE_FLAG_TCP_FLG_MASK		(PPE_ACL_RULE_GEN_FLAG_LAST << 1)	/**< Rule match with TCP flag mask. */

/*
 * IP rule flag.
 */
#define PPE_ACL_RULE_FLAG_SIP_MASK		(PPE_ACL_RULE_GEN_FLAG_LAST << 1)	/**< Rule match with source IP mask. */
#define PPE_ACL_RULE_FLAG_DIP_MASK		(PPE_ACL_RULE_GEN_FLAG_LAST << 2)	/**< Rule match with destination IP mask. */

/*
 * L4 port rule flag.
 */
#define PPE_ACL_RULE_FLAG_SPORT_MASK		(PPE_ACL_RULE_GEN_FLAG_LAST << 1)	/**< Rule match with source port mask. */
#define PPE_ACL_RULE_FLAG_SPORT_RANGE		(PPE_ACL_RULE_GEN_FLAG_LAST << 2)	/**< Rule match with source port range. */
#define PPE_ACL_RULE_FLAG_DPORT_MASK		(PPE_ACL_RULE_GEN_FLAG_LAST << 3)	/**< Rule match with destination port mask. */
#define PPE_ACL_RULE_FLAG_DPORT_RANGE		(PPE_ACL_RULE_GEN_FLAG_LAST << 4)	/**< Rule match with destination port range. */

/*
 * UDF rule flag.
 */
#define PPE_ACL_RULE_FLAG_UDFA_MASK 		(PPE_ACL_RULE_GEN_FLAG_LAST << 1)	/**< Rule match with user defined field - a mask. */
#define PPE_ACL_RULE_FLAG_UDFA_RANGE		(PPE_ACL_RULE_GEN_FLAG_LAST << 2)	/**< Rule match with user defined field - a range. */
#define PPE_ACL_RULE_FLAG_UDFB_MASK		(PPE_ACL_RULE_GEN_FLAG_LAST << 3)	/**< Rule match with user defined field - b mask. */
#define PPE_ACL_RULE_FLAG_UDFB_RANGE		(PPE_ACL_RULE_GEN_FLAG_LAST << 4)	/**< Rule match with user defined field - b range. */
#define PPE_ACL_RULE_FLAG_UDFC_MASK		(PPE_ACL_RULE_GEN_FLAG_LAST << 5)	/**< Rule match with user defined field - c mask. */
#define PPE_ACL_RULE_FLAG_UDFD_MASK		(PPE_ACL_RULE_GEN_FLAG_LAST << 6)	/**< Rule match with user defined field - d mask. */

/*
 * ACL action flags.
 */
#define PPE_ACL_RULE_ACTION_FLAG_SERVICE_CODE_EN		0x00000001	/**< Rule action to change service code. */
#define PPE_ACL_RULE_ACTION_FLAG_QID_EN				0x00000002	/**< Rule action to change queue ID. */
#define PPE_ACL_RULE_ACTION_FLAG_ENQUEUE_PRI_CHANGE_EN		0x00000004	/**< Rule action to change enqueue priority. */
#define PPE_ACL_RULE_ACTION_FLAG_CTAG_DEI_CHANGE_EN		0x00000008	/**< Rule action to change C-DEI bit in 1p. */
#define PPE_ACL_RULE_ACTION_FLAG_CTAG_PCP_CHANGE_EN		0x00000010	/**< Rule action to change C-PCP marking. */
#define PPE_ACL_RULE_ACTION_FLAG_STAG_DEI_CHANGE_EN		0x00000020	/**< Rule action to change S-DEI bit in 1p. */
#define PPE_ACL_RULE_ACTION_FLAG_STAG_PCP_CHANGE_EN		0x00000040	/**< Rule action to change S-PCP marking. */
#define PPE_ACL_RULE_ACTION_FLAG_DSCP_TC_CHANGE_EN		0x00000080	/**< Rule action to change DSCP/traffic-class in IP header. */
#define PPE_ACL_RULE_ACTION_FLAG_CVID_CHANGE_EN			0x00000100	/**< Rule action to change C-VID. */
#define PPE_ACL_RULE_ACTION_FLAG_SVID_CHANGE_EN			0x00000200	/**< Rule action to change S-VID. */
#define PPE_ACL_RULE_ACTION_FLAG_DEST_INFO_CHANGE_EN		0x00000400	/**< Rule action to change destination. */
#define PPE_ACL_RULE_ACTION_FLAG_MIRROR_EN			0x00000800	/**< Rule action to enable mirroring. */
#define PPE_ACL_RULE_ACTION_FLAG_CTAG_FMT_TAGGED		0x00001000	/**< Rule action to change C-tag format. */
#define PPE_ACL_RULE_ACTION_FLAG_STAG_FMT_TAGGED		0x00002000	/**< Rule action to change S-tag format. */
#define PPE_ACL_RULE_ACTION_FLAG_REDIR_TO_CORE_EN		0x00004000	/**< Rule action to enable redirection to a specific core. */
#define PPE_ACL_RULE_ACTION_FLAG_FW_CMD				0x00008000	/**< Rule action to enable special forwarding - forward/drop/copu/redirect. */
#define PPE_ACL_RULE_ACTION_FLAG_POLICER_EN			0x00010000	/**< Rule action to enable police. */
#define PPE_ACL_RULE_ACTION_FLAG_REDIR_EDIT_EN			0x00020000	/**< Rule action to enable redirect with packet editing enable. */

/**
 * ppe_acl_rule_vlan_fmt
 *	VLAN format
 */
typedef enum ppe_acl_vlan_fmt {
	PPE_ACL_VLAN_FMT_TAGGED,		/**< VLAN tagged format. */
	PPE_ACL_VLAN_FMT_PRI_TAGGED,		/**< VLAN priority tagged format. */
	PPE_ACL_VLAN_FRM_UNTAGGED,		/**< VLAN untagged format. */
} ppe_acl_vlan_fmt_t;

/**
 * ppe_acl_rule_ip_type
 *	ACL rule IP type.
 */
typedef enum ppe_acl_ip_type {
	PPE_ACL_IP_TYPE_V4,			/**< Packet with IPv4 header. */
	PPE_ACL_IP_TYPE_V6			/**< Packet with IPv6 header. */
} ppe_acl_ip_type_t;

/**
 * ppe_acl_fwd_cmd
 *	PPE ACL forward action
 */
typedef enum ppe_acl_fwd_cmd {
	PPE_ACL_FWD_CMD_FWD,			/**< Forward command to allow regular forwarding. */
	PPE_ACL_FWD_CMD_DROP,			/**< Forward command to drop matching packets. */
	PPE_ACL_FWD_CMD_COPY,			/**< Forward command to copy the matching packets. */
	PPE_ACL_FWD_CMD_REDIR,			/**< Forward command to redirecting the matching packets. */
} ppe_acl_fwd_cmd_t;

/**
 * ppe_acl_rule_src_type
 *	PPE ACL rule source tyoe
 */
typedef enum ppe_acl_rule_src_type {
	PPE_ACL_RULE_SRC_TYPE_DEV = 1,		/**< Rule source is a net-device. */
	PPE_ACL_RULE_SRC_TYPE_SC,		/**< Rule source is a service code. */
	PPE_ACL_RULE_SRC_TYPE_FLOW		/**< Rule source is flow. */
} ppe_acl_rule_src_type_t;

/**
 * ppe_acl_ret
 *	ACL return code
 */
typedef enum ppe_acl_ret {
	PPE_ACL_RET_SUCCESS = 0,			/**< Success */
	PPE_ACL_RET_CREATE_FAIL_OOM,			/**< Rule create failed due to out of memory. */
	PPE_ACL_RET_CREATE_FAIL_MAX_SLICES,		/**< Rule create failed because rule require more than max slices. */
	PPE_ACL_RET_CREATE_FAIL_DRV_ALLOC,		/**< Rule create failed due to ppe-drv context alloc failure. */
	PPE_ACL_RET_CREATE_FAIL_RULE_CONFIG,		/**< Rule create failed due to invalid configuration. */
	PPE_ACL_RET_CREATE_FAIL_INVALID_SRC,		/**< Rule create failed due to invalid source. */
	PPE_ACL_RET_CREATE_FAIL_INVALID_CMN,		/**< Rule create failed due to invalid common rule. */
	PPE_ACL_RET_CREATE_FAIL_RULE_PARSE,		/**< Rule create failed due to rule parse failure. */
	PPE_ACL_RET_CREATE_FAIL_SC_ALLOC,		/**< Rule create failed due to service code allocation failure. */
	PPE_ACL_RET_CREATE_FAIL_ACTION_CONFIG,		/**< Rule create failed due to invalid action configuration. */
	PPE_ACL_RET_CREATE_FAIL_INVALID_ID,		/**< Rule create failed due to invalid rule ID. */
	PPE_ACL_RET_DESTROY_FAIL_INVALID_ID,		/**< Rule destroy failed due to invalid rule ID. */
} ppe_acl_ret_t;

/**
 * ppe_acl_rule_match_mac
 *	MAC based ACL rule.
 */
struct ppe_acl_rule_match_mac {
	/*
	 * Rule
	 */
	uint8_t mac[ETH_ALEN];			/**< MAC address to match. */
	uint8_t mac_mask[ETH_ALEN];		/**< MAC address mask to be used with match. */
};

/**
 * ppe_acl_rule_match_vlan
 *	VLAN based ACL rule.
 */
struct ppe_acl_rule_match_vlan {
	ppe_acl_vlan_fmt_t tag_fmt;		/**< VLAN tag format for ACL match. */
	uint16_t vid_min;			/**< VLAN ID for ACL match. */
	uint16_t vid_mask_max;			/**< VLAN ID mask or maximum value for range. */
};

/**
 * ppe_acl_rule_match_1p
 *	802.1p based ACL rule.
 */
struct ppe_acl_rule_match_1p {
	uint8_t pcp;				/**< PCP value for ACL match. */
	uint8_t pcp_mask;			/**< PCP mask for ACL match. */
};

/**
 * ppe_acl_rule_match_pppoe_session
 *	PPPOE session based ACL rule
 */
struct ppe_acl_rule_match_pppoe_session {
	uint16_t pppoe_session_id;		/**< PPPOE session ID for ACL match. */
	uint16_t pppoe_session_id_mask;		/**< PPPOE session ID mask for ACL match. */
};

/**
 * ppe_acl_rule_match_ether_type
 *	Ether-type based ACL rule.
 */
struct ppe_acl_rule_match_ether_type {
	uint16_t l2_proto;			/**< Ether type value for ACL match. */
	uint16_t l2_proto_mask;			/**< Ether type mask for ACL match. */
};

/**
 * ppe_acl_rule_match_ip
 *	IP based ACL rule
 */
struct ppe_acl_rule_match_ip {
	ppe_acl_ip_type_t ip_type;		/**< IP type for IP based ACL rule. */
	uint32_t ip[4];				/**< IP address for ACL match. */
	uint32_t ip_mask[4];			/**< IP address mask for ACL match. */
};

/**
 * ppe_acl_rule_match_l4_port
 *	L4 port based ACL rule
 */
struct ppe_acl_rule_match_l4_port {
	uint16_t l4_port_min;			/**< L4 port value for ACL match. */
	uint16_t l4_port_max_mask;		/**< L4 port mask or maximum value for range. */
};

/**
 * ppe_acl_rule_match_1st_frag
 *	ACL rule for first IP fragment
 */
struct ppe_acl_rule_match_1st_frag {
	uint8_t reserved;			/**< Reserved field - not used. */
};

/**
 * ppe_acl_rule_match_tcp_flag
 *	TCP flags based ACL rule.
 */
struct ppe_acl_rule_match_tcp_flag {
	uint8_t tcp_flags;			/**< TCP flags for ACL match. */
	uint8_t tcp_flags_mask;			/**< TCP flags mask for ACL match. */
};

/**
 * ppe_acl_rule_match_l4_proto
 *	L4 proto based ACL rule.
 */
struct ppe_acl_rule_match_l4_proto {
	uint8_t l3_v4proto_v6nexthdr;		/**< L4 proto for ACL match. */
	uint8_t l3_v4proto_v6nexthdr_mask;	/**< L4 proto mask for ACL match. */
};

/**
 * ppe_acl_rule_match_dscp_tc
 *	DSCP/TC based ACL rule.
 */
struct ppe_acl_rule_match_dscp_tc {
	uint8_t l3_dscp_tc;			/**< DSCP/TC value for ACL match. */
	uint8_t l3_dscp_tc_mask;		/**< DSCP/TC mask for ACL match. */
};


/**
 * ppe_acl_rule_match_l3_len
 *	ACL rule based on IP length.
 */
struct ppe_acl_rule_match_l3_len {
	uint16_t l3_length_min;			/**< L3 length for ACL match. */
	uint16_t l3_length_mask_max;		/**< L3 length mask of maximum value for range. */
};

/**
 * ppe_acl_rule_match_ttl_hop
 *	TTL/HOP limit based ACL rule.
 */
struct ppe_acl_rule_match_ttl_hop {
	uint8_t hop_limit;			/**< TTL/hop-limit for ACL match. */
	uint8_t hop_limit_mask;			/**< TTL/hop-limit mask for ACL match. */
};

/**
 * ppe_acl_rule_match_udf
 *	UDF based ACL rule
 */
struct ppe_acl_rule_match_udf {
	uint16_t udf_a_min;			/**< UDF-A value for ACL match. */
	uint16_t udf_b_min;			/**< UDF-B value for ACL match. */
	uint16_t udf_c;				/**< UDF-C value for ACL match. */
	uint16_t udf_d;				/**< UDF-D value for ACL match. */
	uint16_t udf_a_mask_max;		/**< UDF-A mask for ACL match. */
	uint16_t udf_b_mask_max;		/**< UDF-B mask for ACL match. */
	uint16_t udf_c_mask;			/**< UDF-C mask for ACL match. */
	uint16_t udf_d_mask;			/**< UDF-D mask for ACL match. */
	uint8_t udf_a_valid;			/**< UDF-A valid for ACL match. */
	uint8_t udf_b_valid;			/**< UDF-B valid for ACL match. */
	uint8_t udf_c_valid;			/**< UDF-C valid for ACL match. */
	uint8_t udf_d_valid;			/**< UDF-D valid for ACL match. */
};

/**
 * ppe_acl_rule_match_default
 *	Default ACL rule.
 */
struct ppe_acl_rule_match_default {
	uint8_t reserved;			/**< Reserved field - not used. */
};

/**
 * ppe_acl_rule_action
 *	ACL rule action
 */
struct ppe_acl_rule_action {
	/*
	 * Action values.
	 */
	uint8_t service_code;			/**< Changed service code. */
	uint8_t qid;				/**< Changed queue ID. */
	uint8_t enqueue_pri;			/**< Changed enqueue priority. */
	uint8_t ctag_pcp;			/**< Changed CPCP. */
	uint8_t stag_pcp;			/**< Changed SPCP. */
	uint8_t dscp_tc;			/**< Changed DSCP/TC. */
	uint16_t cvid;				/**< Changed CVID. */
	uint16_t svid;				/**< Changed SVID. */
	union {
		char dev_name[IFNAMSIZ];	/**< Changed destination device. */
	} dst;
	uint8_t redir_core;			/**< Redirect core. */
	uint16_t policer_id;			/**< Policer ID. */
	ppe_acl_fwd_cmd_t fwd_cmd;		/**< Forward action for matched packets. */

	/*
	 * Action control flags.
	 */
	uint32_t flags;				/**< Action control flags. */
};

/**
 * ppe_acl_rule_match_one
 *	Single ACL rule info
 */
struct ppe_acl_rule_match_one {
	uint32_t rule_flags;				/**< Rule flags. */
	union {
		struct ppe_acl_rule_match_mac smac;	/**< Rule with source-MAC address match. */
		struct ppe_acl_rule_match_mac dmac;	/**< Rule with destination-MAC address match. */
		struct ppe_acl_rule_match_vlan cvid;	/**< Rule with C-VLAN ID match. */
		struct ppe_acl_rule_match_vlan svid;	/**< Rule with S-VLAN ID match. */
		struct ppe_acl_rule_match_1p cpcp;	/**< Rule with C-PCP match. */
		struct ppe_acl_rule_match_1p spcp;	/**< Rule with S-PCP match. */
		struct ppe_acl_rule_match_pppoe_session pppoe_sess;
							/**< Rule with PPPOE session ID match. */
		struct ppe_acl_rule_match_ether_type ether_type;
							/**< Rule with ethertype match. */
		struct ppe_acl_rule_match_ip sip;	/**< Rule with source IP match. */
		struct ppe_acl_rule_match_ip dip;	/**< Rule with destination IP match. */
		struct ppe_acl_rule_match_l4_port sport;
							/**< Rule with source port match. */
		struct ppe_acl_rule_match_l4_port dport;
							/**< Rule with destination port match. */
		struct ppe_acl_rule_match_1st_frag first_frag;
							/**< Rule with first frag match. */
		struct ppe_acl_rule_match_tcp_flag tcp_flag;
							/**< Rule with tcp flag match. */
		struct ppe_acl_rule_match_l4_proto proto_nexthdr;
							/**< Rule with L4 proto match. */
		struct ppe_acl_rule_match_dscp_tc dscp_tc;
							/**< Rule with DSCP/traffic-class match. */
		struct ppe_acl_rule_match_l3_len l3_len;
							/**< Rule with L3 length match. */
		struct ppe_acl_rule_match_ttl_hop ttl_hop;
							/**< Rule with TTL/hop-limit match. */
		struct ppe_acl_rule_match_udf udf;	/**< Rule with user-defined match. */
		struct ppe_acl_rule_match_default def;	/**< Rule with default match. */
	} rule;
};

/**
 * ppe_acl_rule_flow_policer
 *	ACL rule info for flow policer.
 *
 * TODO: Move this to internal header file.
 */
struct ppe_acl_rule_flow_policer {
	/*
	 * Request
	 */
	uint16_t hw_policer_idx;			/**< Policer index to be used for flow + policer combination. */
	ppe_acl_rule_id_t rule_id;			/**< ACL rule ID associated with flow + policer. */
	bool pkt_noedit;				/**< Indication if policing is required in packet non-edit mode */

	/*
	 * Response
	 */
	uint8_t sc;					/**< Flow service code to be used to bind flow with ACL. */
	ppe_acl_ret_t ret;				/**< Return status. */
};

/**
 * ppe_acl_rule_match_cmn
 *	Common fields for ACL rule
 */
struct ppe_acl_rule_match_cmn {
	uint32_t cmn_flags;				/**< Rule match common flag. */
	uint16_t pri;					/**< ACL rule priority. */
	uint8_t group;					/**< ACL rule chain resolution. */
};

/**
 * ppe_acl_rule
 *	Multiple ACL rules.
 */
struct ppe_acl_rule {
	/*
	 * Request
	 */
	ppe_acl_rule_id_t rule_id;			/**< Rule ID. */
	uint32_t valid_flags;				/**< Bits indicating valid rule types in the rule. */
	struct ppe_acl_rule_match_cmn cmn;		/**< Common match rule. */
	struct ppe_acl_rule_match_one rules[PPE_ACL_RULE_MATCH_TYPE_MAX];
							/**< Multiple single rule. */
	struct ppe_acl_rule_action action;		/**< ACL action object. */
	ppe_acl_rule_src_type_t stype;			/**< ACL source type. */
	union {
		uint8_t sc;				/**< ACL source a service code. */
		char dev_name[IFNAMSIZ];		/**< ACL source dev name. */
	} src;

	/*
	 * Response
	 */
	ppe_acl_ret_t ret;				/**< ACL return type. */

};

/*
 * TOOD: Move flow_policer APIs to intenral .h file.
 */

/**
 * ppe_acl_rule_process_callback_t
 *	External callback to be registered with ACL for certain actions.
 */
typedef bool (*ppe_acl_rule_process_callback_t)(void *app_data, void *skb);

/**
 * ppe_acl_rule_flow_policer_create()
 *	Create ACL rule for a flow & policer combination. This is API is used
 *	when user want to enable policer per flow and provide necessary details
 *	in rule create via classifier in connection manager.
 *
 * @datatypes
 * ppe_acl_rule_flow_policer
 *
 * @param[IN] rule 	ACL rule information.
 *
 * @return
 * Status of flow policer create rule operation.
 */
ppe_acl_ret_t ppe_acl_rule_flow_policer_create(struct ppe_acl_rule_flow_policer *rule);

/**
 * ppe_acl_rule_destroy()
 *	Destroy ACL rule in PPE.
 *
 * @datatypes
 * ppe_acl_rule_id_t
 *
 * @param[IN] id	ACL rule ID which needs to be deleted.
 *
 * @return
 * Status of rule destroy operation.
 */
ppe_acl_ret_t ppe_acl_rule_destroy(ppe_acl_rule_id_t id);

/**
 * ppe_acl_rule_create()
 *	Create ACL rule in PPE.
 *
 * @datatypes
 * ppe_acl_rule
 *
 * @param[IN] rule		ACL rule information.
 *
 * @return
 * Status of rule create operation.
 */
ppe_acl_ret_t ppe_acl_rule_create(struct ppe_acl_rule *rule);

/**
 * ppe_acl_rule_get_acl_hw_index()
 *	Get ACL hardware index.
 *
 * @datatypes
 * ppe_acl_rule_id_t
 *
 * @param[IN] acl_id		ACL rule ID.
 *
 * @return
 * Return the hw index.
 */
uint16_t ppe_acl_rule_get_acl_hw_index(ppe_acl_rule_id_t acl_id);

/**
 * ppe_acl_rule_ref()
 *	Take a reference on an ACL rule.
 *
 * @datatypes
 * ppe_acl_rule_id_t
 *
 * @param[IN] acl_id		ACL rule ID.
 *
 * @return
 * True or False.
 */
bool ppe_acl_rule_ref(ppe_acl_rule_id_t acl_id);

/**
 * ppe_acl_rule_deref()
 *	Dereference on an ACL rule.
 *
 * @datatypes
 * ppe_acl_rule_id_t
 *
 * @param[IN] acl_id		ACL rule ID.
 *
 * @return
 * True or False.
 */
bool ppe_acl_rule_deref(ppe_acl_rule_id_t acl_id);

/**
 * ppe_acl_rule_callback_register()
 *	Register callbacks for ACL rules for certain actions to be taken.
 *
 * @datatypes
 * ppe_acl_rule_id_t
 * ppe_acl_rule_callback_t_cb
 *
 * @param[IN] acl_id		ACL rule information.
 * @param[IN] cb		callback.
 * @param[IN] app_data		app_data.
 *
 * @return
 * Status of register operation.
 */
bool ppe_acl_rule_callback_register(ppe_acl_rule_id_t acl_id, ppe_acl_rule_process_callback_t cb, void *app_data);

/**
 * ppe_acl_rule_callback_unregister()
 *	Unregister callbacks for ACL rules.
 *
 * @datatypes
 * ppe_acl_rule_id_t
 *
 * @param[IN] acl_id		ACL rule information.
 *
 * @return
 * void.
 */
void ppe_acl_rule_callback_unregister(ppe_acl_rule_id_t acl_id);

#endif /* _PPE_ACL_H_ */
