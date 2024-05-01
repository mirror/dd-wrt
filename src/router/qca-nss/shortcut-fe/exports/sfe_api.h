/*
 * sfe_api.h
 *	 SFE exported function headers for SFE engine.
 *
 * Copyright (c) 2015,2016, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * @file sfe_api.h
 *	SFE exported function headers for the SFE engine.
 */

#ifndef __SFE_API_H
#define __SFE_API_H

/**
 * @addtogroup nss_sfe_definitions
 * @{
 */

#define SFE_MAX_VLAN_DEPTH 2			/**< Maximum VLAN depth. */
#define SFE_VLAN_ID_NOT_CONFIGURED 0xfff	/**< VLAN ID not configured. */
#define SFE_INVALID_VLAN_PCP 0xff		/**< VLAN PCP remark is invalid for SAWF (Service Aware Wi-Fi). */
#define SFE_MAX_SERVICE_CLASS_ID 0x80		/**< Maximum service class ID. */
#define SFE_INVALID_SERVICE_CLASS_ID 0xff	/**< Service class ID not valid. */
#define SFE_SERVICE_CLASS_STATS_MAX_RETRY 100	/**< Maximum retries for fetching service class statistics. */
#define SFE_INVALID_MSDUQ 0xff			/**< Invalid MAC Service Data Unit Queue. */
#define SFE_MC_IF_MAX 16

#define SFE_SPECIAL_INTERFACE_BASE 0x7f00	/**< Special interface base number. */
#define SFE_SPECIAL_INTERFACE_IPV4 (SFE_SPECIAL_INTERFACE_BASE + 1)	/**< Interface number for IPv4. */
#define SFE_SPECIAL_INTERFACE_IPV6 (SFE_SPECIAL_INTERFACE_BASE + 2)	/**< Interface number for IPv6. */
#define SFE_SPECIAL_INTERFACE_IPSEC (SFE_SPECIAL_INTERFACE_BASE + 3)	/**< Interface number for IPSec. */
#define SFE_SPECIAL_INTERFACE_L2TP (SFE_SPECIAL_INTERFACE_BASE + 4)	/**< Interface number for L2TP. */
#define SFE_SPECIAL_INTERFACE_PPTP (SFE_SPECIAL_INTERFACE_BASE + 5)	/**< Interface number for PPTP. */

/**
 * SAWF_metadata information placement in mark field.
 */
#define SFE_SAWF_VALID_TAG              0xAA    /**< Valid SAWF tag value. */
#define SFE_SAWF_TAG_SHIFT              24      /**< Number of bit shifts for SAWF tag. */
#define SFE_SAWF_SERVICE_CLASS_SHIFT    16      /**< Number of bit shifts for SAWF service class ID. */
#define SFE_SAWF_SERVICE_CLASS_MASK     0xff    /**< Mask for SAWF service class ID. */
#define SFE_SAWF_MSDUQ_MASK             0xffff  /**< Mask for SAWF msduq. */

/**
 * SAWF_metadata extraction.
 */
#define SFE_GET_SAWF_TAG(sawf_meta)             (sawf_meta >> SFE_SAWF_TAG_SHIFT)
				/**< Get tag field in SAWF meta. */
#define SFE_SAWF_TAG_IS_VALID(tag)              ((tag == SFE_SAWF_VALID_TAG) ? true : false)
				/**< Check if tag is a valid SAWF tag. */

/**
 * MHT port information placement in mark field.
 */
#define SFE_MHT_VALID_TAG		0xBB	/**< Valid MHT tag value. */
#define SFE_MHT_TAG_SHIFT		24	/**< Number of bit shifts for MHT tag. */

#define SFE_MHT_MAX_ACCELERATION_RETRY	256	/**< Maximum retry for SFE acceleration for MHT. */
/**
* @}
*/

/**
 * @addtogroup nss_sfe_flags
 * @{
 */

/*
 * Rule creation and rule update flags.
 */
#define SFE_RULE_CREATE_FLAG_NO_SEQ_CHECK (1<<0)		/**< Do not perform TCP sequence number checks. */
#define SFE_RULE_CREATE_FLAG_BRIDGE_FLOW  (1<<1)		/**< Rule is for a pure bridge forwarding flow. */
#define SFE_RULE_CREATE_FLAG_ROUTED       (1<<2)		/**< Rule is for a routed connection. */
#define SFE_RULE_CREATE_FLAG_DSCP_MARKING (1<<3)		/**< Rule has DSCP marking configured. */
#define SFE_RULE_CREATE_FLAG_NO_SRC_IDENT (1<<4)		/**< Zero out the source identifier. */
#define SFE_RULE_CREATE_FLAG_UNUSED0	  (1<<5)		/**< Rule flag unused 0. */
#define SFE_RULE_CREATE_FLAG_UNUSED1	  (1<<6)		/**< Rule flag unused 1. */
#define SFE_RULE_CREATE_FLAG_L2_ENCAP     (1<<7)		/**< Consists of an encapsulating protocol that carries an IPv4 payload within it. */
#define SFE_RULE_CREATE_FLAG_USE_FLOW_BOTTOM_INTERFACE (1<<8)	/**< Use flow interface number instead of top interface. */
#define SFE_RULE_CREATE_FLAG_USE_RETURN_BOTTOM_INTERFACE (1<<9) /**< Use return interface number instead of top interface. */
#define SFE_RULE_CREATE_FLAG_FLOW_SRC_INTERFACE_CHECK  (1<<10)  /**< Check source interface on the flow direction. */
#define SFE_RULE_CREATE_FLAG_RETURN_SRC_INTERFACE_CHECK  (1<<11)
								/**< Check source interface on the return direction. */
#define SFE_RULE_CREATE_FLAG_FLOW_TRANSMIT_FAST (1<<12)		/**< Original flow can be transmitted fast. */
#define SFE_RULE_CREATE_FLAG_RETURN_TRANSMIT_FAST (1<<13)	/**< Return flow can be transmitted fast. */
#define SFE_RULE_CREATE_FLAG_FLOW_SRC_INTERFACE_CHECK_NO_FLUSH  (1<<14)
								/**< Check source interface on the flow direction but do not flush the connection. */
#define SFE_RULE_CREATE_FLAG_RETURN_SRC_INTERFACE_CHECK_NO_FLUSH  (1<<15)
								/**< Check source interface on the return direction but do not flush the connection. */
#define SFE_RULE_CREATE_FLAG_FLOW_L2_DISABLE (1<<16)		/**< Disable L2 flow processing in original flow. */
#define SFE_RULE_CREATE_FLAG_RETURN_L2_DISABLE (1<<17)		/**< Disable L2 flow processing in return flow. */
#define SFE_RULE_CREATE_FLAG_BRIDGE_VLAN_PASSTHROUGH  (1<<18)	/**< Bridge VLAN passthrough. */

/*
 * Rule creation validity flags.
 */
#define SFE_RULE_CREATE_CONN_VALID         (1<<0)	/**< IPv4 connection is valid. */
#define SFE_RULE_CREATE_TCP_VALID          (1<<1)	/**< TCP protocol fields are valid. */
#define SFE_RULE_CREATE_PPPOE_DECAP_VALID  (1<<2)	/**< PPPoE decapsulation fields are valid. */
#define SFE_RULE_CREATE_PPPOE_ENCAP_VALID  (1<<3)	/**< PPPoE encapsulation fields are valid. */
#define SFE_RULE_CREATE_QOS_VALID          (1<<4)	/**< QoS fields are valid. */
#define SFE_RULE_CREATE_VLAN_VALID         (1<<5)	/**< VLAN fields are valid. */
#define SFE_RULE_CREATE_DSCP_MARKING_VALID (1<<6)	/**< DSCP marking fields are valid. */
#define SFE_RULE_CREATE_VLAN_MARKING_VALID (1<<7)	/**< VLAN marking fields are valid. */
#define SFE_RULE_CREATE_DIRECTION_VALID    (1<<8)	/**< Acceleration direction is valid. */
#define SFE_RULE_CREATE_SRC_MAC_VALID      (1<<9)	/**< Source MAC address is valid. */
#define SFE_RULE_CREATE_MARK_VALID         (1<<10)	/**< SKB marking fields are valid. */
#define SFE_RULE_CREATE_TRUSTSEC_VALID     (1<<11)	/**< Trustsec fields are valid. */
#define SFE_RULE_CREATE_QDISC_RULE_VALID   (1<<12)	/**< QDISC rule is valid. */
#define SFE_RULE_CREATE_VLAN_FILTER_VALID  (1<<13)	/**< Bridge VLAN Filter rule is valid. */

/*
 * Multicast rule flags
 */
#define SFE_MC_RULE_CREATE_FLAG_MC_UPDATE	(1<<0)	/**< Multicast rule update. */
#define SFE_MC_RULE_CREATE_FLAG_MC_EMESH_SP	(1<<1)
		/**< Mark multicast rule as E-MESH Service Prioritization valid. */

/*
 * Multicast rule validity flags
 */
#define SFE_MC_RULE_CREATE_FLAG_QOS_VALID		(1<<0)
		/**< QoS fields are valid. */
#define SFE_MC_RULE_CREATE_FLAG_DSCP_MARKING_VALID	(1<<1)
		/**< DSCP fields are valid. */
#define SFE_MC_RULE_CREATE_FLAG_INGRESS_VLAN_VALID	(1<<2)
		/**< Ingress VLAN fields are valid. */
#define SFE_MC_RULE_CREATE_FLAG_INGRESS_PPPOE		(1<<3)
		/**< Ingress PPPoE fields are valid. */
#define SFE_MC_RULE_CREATE_FLAG_IGS_VALID		(1<<4)
		/**< Ingress shaping fields are valid. */

/*
 * Per-interface rule flags for a multicast connection (to be used with the rule_flags
 * field of sfe_ipv4_mc_if_rule structure).
 */
#define SFE_MC_RULE_CREATE_IF_FLAG_BRIDGE_FLOW 0x01
		/**< Multicast connection rule is created for a bridge flow. */
#define SFE_MC_RULE_CREATE_IF_FLAG_ROUTED_FLOW 0x02
		/**< Multicast connection rule is created for a routed flow. */
#define SFE_MC_RULE_CREATE_IF_FLAG_JOIN 0x04
		/**< Interface has joined the flow. */
#define SFE_MC_RULE_CREATE_IF_FLAG_LEAVE 0x08
		/**< Interface has left the flow. */

/*
 * Per-interface valid flags for a multicast connection (to be used with the valid_flags
 * field of sfe_ipv4_mc_if_rule structure).
 */
#define SFE_MC_RULE_CREATE_IF_FLAG_VLAN_VALID 0x01
		/**< VLAN fields are valid. */
#define SFE_MC_RULE_CREATE_IF_FLAG_PPPOE_VALID 0x02
		/**< PPPoE fields are valid. */
#define SFE_MC_RULE_CREATE_IF_FLAG_NAT_VALID 0x4
		/**< Interface is configured with the source NAT. */

/*
 * Source MAC address validity flags; used with the mac_valid_flags field in the sfe_ipv4_src_mac_rule structure.
 */
#define SFE_SRC_MAC_FLOW_VALID 0x01
		/**< MAC address for the flow interface is valid. */
#define SFE_SRC_MAC_RETURN_VALID 0x02
		/**< MAC address for the return interface is valid. */

/*
 * Ingress/egress trustsec validity flags; used with the CREATE_TRUSTSEC_VALID and sfe_trustsec_rule.
 */
#define SFE_TRUSTSEC_INGRESS_SGT_VALID 0x01
		/**< Ingress SGT for the flow interface is valid. */
#define SFE_TRUSTSEC_EGRESS_SGT_VALID 0x02

/*
 * Qdisc interface validity flags; used with the qdisc_valid_flags  field in the sfe_qdisc_rule structure.
 */
#define SFE_QDISC_RULE_FLOW_VALID 0x01
		/**< Qdisc interface for the flow interface is valid. */
#define SFE_QDISC_RULE_RETURN_VALID 0x02
		/**< Qdisc interface for the return interface is valid. */
#define SFE_QDISC_RULE_FLOW_PPE_QDISC_FAST_XMIT 0x04
		/**< Fast transmit via PPE Qdisc for the flow interface is valid. */
#define SFE_QDISC_RULE_RETURN_PPE_QDISC_FAST_XMIT 0x08
		/**< Fast transmit via PPE Qdisc for the return interface is valid. */

/*
 * Bridge VLAN filter flags; used with SFE_RULE_CREATE_VLAN_FILTER_VALID and sfe_vlan_filter_rule structure.
 */
#define SFE_VLAN_FILTER_FLAG_VALID 		(1<<0) 	/**< VLAN filter is valid in the rule.  */
#define SFE_VLAN_FILTER_FLAG_INGRESS_PVID 	(1<<1)	/**< Add VLAN header at ingress for untagged packets. */
#define SFE_VLAN_FILTER_FLAG_EGRESS_UNTAGGED	(1<<2)	/**< Strip VLAN header associated with this VID at egress. */

/*
 * SAWF mark validity flags
 */
#define SFE_SAWF_MARK_FLOW_VALID        (1<<0)
		/**< SAWF mark in the flow direction is valid. */
#define SFE_SAWF_MARK_RETURN_VALID      (1<<1)
		/**< SAWF mark in the return direction is valid. */

/**
 * @}
 */

/**
 * @addtogroup nss_sfe_datatypes
 * @{
 */

/*
 * 32/64-bit pointer types.
 */
#ifdef __LP64__
typedef uint64_t sfe_ptr_t; /**< 64-bit pointer. */
#else
typedef uint32_t sfe_ptr_t; /**< 32-bit pointer. */
#endif

/**
* Synchronize reason enum.
*/
typedef enum /** @cond */ sfe_rule_sync_reason /** @endcond */ {
	SFE_RULE_SYNC_REASON_STATS,	/**< Synchronize statistics. */
	SFE_RULE_SYNC_REASON_FLUSH,	/**< Synchronize to flush an entry. */
	SFE_RULE_SYNC_REASON_EVICT,	/**< Synchronize to evict an entry. */
	SFE_RULE_SYNC_REASON_DESTROY	/**< Synchronize to destroy an entry (requested by the connection manager). */

} sfe_rule_sync_reason_t;

/**
 * Tx command status.
 */
typedef enum {
	SFE_TX_SUCCESS = 0,		/**< Success. */
	SFE_TX_FAILURE,			/**< Failure other than descriptor not available. */
	SFE_TX_FAILURE_QUEUE,		/**< failure due to descriptor not available. */
	SFE_TX_FAILURE_NOT_READY,	/**< Failure due to SFE state uninitialized. */
	SFE_TX_FAILURE_TOO_LARGE,	/**< Command is too large to fit in one message. */
	SFE_TX_FAILURE_TOO_SHORT,	/**< Command or packet is shorter than expected. */
	SFE_TX_FAILURE_NOT_SUPPORTED,	/**< Command or packet not accepted for forwarding. */
	SFE_TX_FAILURE_BAD_PARAM,	/**< Failure due to bad parameters. */
	SFE_TX_FAILURE_NOT_ENABLED,	/**< Failure due to SFE not enabled. */
} sfe_tx_status_t;

/**
 * Common response types.
 */
enum sfe_cmn_response {
	SFE_CMN_RESPONSE_ACK,		/**< Message acknowledged. */
	SFE_CMN_RESPONSE_EVERSION,	/**< Version error. */
	SFE_CMN_RESPONSE_EINTERFACE,	/**< Interface error. */
	SFE_CMN_RESPONSE_ELENGTH,	/**< Length error. */
	SFE_CMN_RESPONSE_EMSG,		/**< Message error. */
	SFE_CMM_RESPONSE_NOTIFY,	/**< Message independant of request. */
	SFE_CMN_RESPONSE_LAST		/**< Indicates the last item. */
};

/**
 * IPv4 bridge/route rule messages.
 */
enum sfe_message_types {
	SFE_TX_CREATE_RULE_MSG,		/**< IPv4/IPv6 create rule message. */
	SFE_TX_DESTROY_RULE_MSG,	/**< IPv4/IPv6 destroy rule message. */
	SFE_RX_CONN_STATS_SYNC_MSG,	/**< IPv4/IPv6 connection statistics synchronize message. */
	SFE_TX_CONN_STATS_SYNC_MANY_MSG,/**< IPv4/IPv6 connection statistics synchronize many message. */
	SFE_TUN6RD_ADD_UPDATE_PEER,	/**< Add/update peer for 6RD tunnel. */
	SFE_TX_CREATE_MULTICAST_RULE_MSG,	/**< IPv4/IPv6 create multicast rule message. */
	SFE_TX_DESTROY_MULTICAST_RULE_MSG,     /**< IPv4/IPv6 destroy multicast rule message. */
	SFE_MAX_MSG_TYPES,		/**< IPv4/IPv6 message max type number. */
};

/*
 * Connection mark types.
 */
enum sfe_connection_mark_type {
	SFE_CONNECTION_MARK_TYPE_CONNMARK,      /**< Conntrack mark. */
	SFE_CONNECTION_MARK_TYPE_SAWFMARK,      /**< SAWF mark. */
	SFE_CONNECTION_MARK_TYPE_MAX            /**< Indicates the last item. */
};

/**
 * Connection mark structure.
 */
struct sfe_connection_mark {
	int protocol;				/**< Protocol number. */
	__be32 src_ip[4];			/**< Source IP address. */
	__be32 dest_ip[4];			/**< Destination IP address. */
	__be16 src_port;			/**< Source port number. */
	__be16 dest_port;			/**< Destination port number. */
	u32 flow_mark;				/**< Mark to be updated for the flow direction. */
	u32 return_mark;			/**< Mark to be updated for the return direction. */
	u8 flow_svc_id;			/**< Service class in the flow direction. */
	u8 return_svc_id;		/**< Service class in the return direction. */
	u32 flags;				/**< State of marks. */
	enum sfe_connection_mark_type type;	/**< Type of the marking. */
};

/**
 * Common message structure.
 */
struct sfe_cmn_msg {
	u16 version;			/**< Version ID for the main message format. */
	u16 interface;			/**< Primary key for all messages. */
	enum sfe_cmn_response response;	/**< Primary response. */
	u32 type;			/**< Decentralized request ID used to match response ID. */
	u32 error;			/**< Decentralized specific error message; response == EMSG. */
	sfe_ptr_t cb;			/**< Callback pointer. */
	sfe_ptr_t app_data;		/**< Application data. */
	u32 len;			/**< Length of the message excluding this header. */
};

/**
 * Common 5-tuple structure.
 */
struct sfe_ipv4_5tuple {
	__be32 flow_ip;		/**< Flow IP address. */
	__be32 return_ip;	/**< Return IP address. */
	__be16 flow_ident;	/**< Flow identifier, e.g., TCP/UDP port. */
	__be16 return_ident;	/**< Return identifier, e.g., TCP/UDP port. */
	u8 protocol;		/**< Protocol number. */
	u8 reserved[3];		/**< Reserved; padding for alignment. */
};

/**
 * IPv4 connection rule structure.
 */
struct sfe_ipv4_connection_rule {
	u8 flow_mac[6];			/**< Flow MAC address. */
	u8 return_mac[6];		/**< Return MAC address. */
	s32 flow_interface_num;		/**< Flow interface number. */
	s32 return_interface_num;	/**< Return interface number. */
	s32 flow_top_interface_num;	/**< Top flow interface number. */
	s32 return_top_interface_num;	/**< Top return interface number. */
	u32 flow_mtu;			/**< Flow interface`s MTU. */
	u32 return_mtu;			/**< Return interface`s MTU. */
	__be32 flow_ip_xlate;		/**< Translated flow IP address. */
	__be32 return_ip_xlate;		/**< Translated return IP address. */
	__be16 flow_ident_xlate;	/**< Translated flow identifier, e.g., port. */
	__be16 return_ident_xlate;	/**< Translated return identifier, e.g., port. */
};

/**
 * TCP connection rule structure.
 */
struct sfe_protocol_tcp_rule {
	u32 flow_max_window;	/**< Flow direction's largest seen window. */
	u32 return_max_window;	/**< Return direction's largest seen window. */
	u32 flow_end;		/**< Flow direction's largest seen sequence + segment length. */
	u32 return_end;		/**< Return direction's largest seen sequence + segment length. */
	u32 flow_max_end;	/**< Flow direction's largest seen ack + max(1, win). */
	u32 return_max_end;	/**< Return direction's largest seen ack + max(1, win). */
	u8 flow_window_scale;	/**< Flow direction's window scaling factor. */
	u8 return_window_scale;	/**< Return direction's window scaling factor. */
	u16 reserved;		/**< Reserved; padding for alignment. */
};

/**
 * sfe_pppoe_br_accel_mode_t
 *	PPPoE bridge acceleration modes.
 */
typedef enum {
	SFE_PPPOE_BR_ACCEL_MODE_DISABLED,       /**< No acceleration */
	SFE_PPPOE_BR_ACCEL_MODE_EN_5T,          /**< 5-tuple (src_ip, dest_ip, src_port, dest_port, protocol) acceleration */
	SFE_PPPOE_BR_ACCEL_MODE_EN_3T,          /**< 3-tuple (src_ip, dest_ip, pppoe session id) acceleration */
	SFE_PPPOE_BR_ACCEL_MODE_MAX             /**< Indicates the last item */
} __attribute__ ((__packed__)) sfe_pppoe_br_accel_mode_t;

/**
 * PPPoE connection rules structure.
 */
struct sfe_pppoe_rule {
	u16 flow_pppoe_session_id;		/**< Flow direction`s PPPoE session ID. */
	u8 flow_pppoe_remote_mac[ETH_ALEN];	/**< Flow direction`s PPPoE server MAC address. */
	u16 return_pppoe_session_id;		/**< Return direction's PPPoE session ID. */
	u8 return_pppoe_remote_mac[ETH_ALEN];	/**< Return direction's PPPoE server MAC address. */
};

/**
 * Information for source MAC address rules.
 */
struct sfe_src_mac_rule {
	uint32_t mac_valid_flags;	/**< MAC address validity flags. */
	uint16_t flow_src_mac[3];	/**< Source MAC address for the flow direction. */
	uint16_t return_src_mac[3];	/**< Source MAC address for the return direction. */
};

/**
 * QoS connection rule structure.
 */
struct sfe_qos_rule {
	u32 flow_qos_tag;	/**< QoS tag associated with this rule for flow direction. */
	u32 return_qos_tag;	/**< QoS tag associated with this rule for return direction. */
	u8 flow_int_pri;	/**< PPE INT_PRI corresponding to flow direction when PPE Qdisc is configured. */
	u8 return_int_pri;	/**< PPE INT_PRI corresponding to return direction when PPE Qdisc is configured. */
};

/**
* Mark rule structure.
*/
struct sfe_mark_rule {
	u32 flow_mark;		/**< SKB mark associated with this rule for flow direction. */
	u32 return_mark;	/**< SKB mark associated with this rule for return direction. */
};

/**
 * DSCP connection rule structure.
 */
struct sfe_dscp_rule {
	u8 flow_dscp;		/**< Egress DSCP value for flow direction. */
	u8 return_dscp;		/**< Egress DSCP value for return direction. */
	u8 reserved[2];		/**< Reserved; padding for alignment. */
};

/**
 * VLAN connection rule structure.
 */
struct sfe_vlan_rule {
	u32 ingress_vlan_tag;	/**< VLAN tag for ingress packets. */
	u32 egress_vlan_tag;	/**< VLAN tag for egress packets. */
};

/**
 * VLAN filter connection rule structure.
 * 	TODO: Combine flow/return information into one structure.
 */
struct sfe_vlan_filter_rule {
	u32 ingress_vlan_tag;	/**< VLAN tag for ingress packets. */
	u32 egress_vlan_tag;	/**< VLAN tag for egress packets. */
	u8 ingress_flags;	/**< VLAN flags at ingress. */
	u8 egress_flags;	/**< VLAN flags at egress. */
	u8 reserved[2];		/**< Reserved; padding for alignment. */
};

/**
 * Trustsec connection rule structure.
 */
struct sfe_trustsec_rule {
	u16 ingress_sgt;	/**< Trustsec SGT for ingress packets. */
	u16 egress_sgt;		/**< Trustsec SGT for egress packets. */
	u8 sgt_valid_flags;	/**< SGT validity flags. */
};

/**
 * Acceleration direction rule structure.
 * Sometimes it is useful to accelerate traffic in one direction and not in another.
 */
struct sfe_acceleration_direction_rule {
	u8 flow_accel;		/**< Accelerate in flow direction. */
	u8 return_accel;	/**< Accelerate in return direction. */
	u8 reserved[2];		/**< Reserved; padding for alignment. */
};

/**
 * Service class rule information in both directions.
 */
struct sfe_service_class_rule {
	uint32_t flow_mark;		/**< SAWF metadata information in flow direction. */
	uint32_t return_mark;		/**< SAWF metadata information in return direction. */
	uint8_t flow_svc_id;		/**< Service class id in flow direction. */
	uint8_t return_svc_id;		/**< Service class id in return direction. */
};

/**
 * Qdisc information for both the directions.
 *
 * In case of a single qdisc applied by user in the flow or return interface hierarchy, use this rule to
 * indicate the netdev on which the qdisc is applied. This enables full SFE offload for the direction including
 * the qdisc processing.
 */
struct sfe_qdisc_rule {
	uint32_t valid_flags;       	/**< Qdisc interface validity flags. */
	s32 flow_qdisc_interface;	/**< Netdevice for flow direction on which qdisc is applied. */
	s32 return_qdisc_interface;	/**< Netdevice for return direction on which qdisc is applied. */
};

/**
 * IPv4 rule create submessage structure.
 */
struct sfe_ipv4_rule_create_msg {
	/* Request */
	u32 valid_flags;				/**< Bit flags associated with paramater validity. */
	u32 rule_flags;					/**< Bit flags associated with the rule. */

	struct sfe_ipv4_5tuple tuple;			/**< Holds values of 5-tuple. */

	struct sfe_ipv4_connection_rule conn_rule;	/**< Basic connection-specific data. */
	struct sfe_protocol_tcp_rule tcp_rule;		/**< TCP-related acceleration parameters. */
	struct sfe_pppoe_rule pppoe_rule;		/**< PPPoE-related acceleration parameters. */
	struct sfe_qos_rule qos_rule;			/**< QoS-related acceleration parameters. */
	struct sfe_src_mac_rule src_mac_rule;		/**< Source MAC address rule. */
	struct sfe_mark_rule mark_rule;			/**< SKB mark-related acceleration parameters. */
	struct sfe_dscp_rule dscp_rule;			/**< DSCP-related acceleration parameters. */
	struct sfe_vlan_rule vlan_primary_rule;		/**< Primary VLAN-related acceleration parameters. */
	struct sfe_vlan_rule vlan_secondary_rule;	/**< Secondary VLAN-related acceleration parameters. */
	struct sfe_trustsec_rule trustsec_rule;		/**< Trustsec-related acceleration parameters. */

#ifdef CONFIG_XFRM
	struct sfe_acceleration_direction_rule direction_rule;
							/**< Direction related acceleration parameters. */
#endif
	/* Response */
	struct sfe_service_class_rule sawf_rule;
							/**< Service class related information */
	struct sfe_qdisc_rule qdisc_rule;		/**< Qdisc indication per direction */
	struct sfe_vlan_filter_rule flow_vlan_filter_rule;
							/**< TO-direction VLAN Filter related acceleration parameters. */
	struct sfe_vlan_filter_rule return_vlan_filter_rule;
							/**< FROM-direction VLAN Filter related acceleration parameters. */
	u32 index;					/**< Slot ID for cache statistics to host OS. */
};

/**
 * IPv4 rule destroy submessage structure.
 */
struct sfe_ipv4_rule_destroy_msg {
	struct sfe_ipv4_5tuple tuple;	/**< Holds values of 5-tuple. */
};

/*
 * Multicast destination interface entry
 */
struct sfe_ipv4_mc_device_entry {
	uint32_t rule_flags;		/**< Bit flag of rules indicating PPPOE session, Vlan. */
	uint32_t valid_flags;		/**< Valid Bit flags associated with the rule of vlan/pppoe. */
	uint32_t xlate_src_ip;		/**< nated source ip address. */
	uint32_t xlate_src_ident;	/**< nated source port number. */
	uint32_t egress_vlan_tag[SFE_MAX_VLAN_DEPTH]; /**< VLAN tag stack for the ingress packets. */
	uint32_t if_mtu;                        /**< Interface MTU */
	uint16_t if_mac[3];                     /**< Interface MAC address */
	u16 pppoe_session_id;			/**< PPPOE header offset */
	u8 pppoe_remote_mac[ETH_ALEN];		/**< PPPoE remote mac address. */
	u8 egress_vlan_hdr_cnt;			/**< vlan header number. */
	u8  reserved[1];			/**< Reserved; padding for alignment. */
	uint32_t if_num;			/**< Dest interface number (virtual or physical). */
	struct sfe_qdisc_rule qdisc_rule;		/**< Qdisc indication per direction */
};

/**
 * sfe_ipv4_mc_rule_create_msg
 *      IPv4 multicast rule for creating sub-messages.
 */
struct sfe_ipv4_mc_rule_create_msg {
	/* Request */
	u32 valid_flags;				/**< Bit flags associated with paramater validity. */
	u32 rule_flags;					/**< Bit flags associated with the rule. */

	struct sfe_ipv4_5tuple tuple;           /**< Holds values of the 5 tuple. */
	struct sfe_ipv4_connection_rule conn_rule;	/**< Basic connection-specific data. */
	struct sfe_pppoe_rule pppoe_rule;		/**< PPPoE-related acceleration parameters. */
	struct sfe_qos_rule qos_rule;			/**< QoS-related acceleration parameters. */
	struct sfe_src_mac_rule src_mac_rule;		/**< Source MAC address rule. */
	struct sfe_mark_rule mark_rule;			/**< SKB mark-related acceleration parameters. */
	struct sfe_dscp_rule dscp_rule;			/**< DSCP-related acceleration parameters. */
	struct sfe_vlan_rule vlan_primary_rule;		/**< Primary VLAN-related acceleration parameters. */
	struct sfe_vlan_rule vlan_secondary_rule;	/**< Secondary VLAN-related acceleration parameters. */
#ifdef CONFIG_XFRM
	struct sfe_acceleration_direction_rule direction_rule;
							/**< Direction related acceleration parameters. */
#endif
	struct sfe_service_class_rule sawf_rule;	/**< Service class related information. */
	uint16_t dest_mac[3];                   /**< Destination multicast MAC address. */
	uint16_t if_cnt;                      /**< Number of destination interfaces. */
	struct sfe_ipv4_mc_device_entry if_rule[SFE_MC_IF_MAX];	/**< Per-interface information. */
};

/**
 * sfe_ipv4_mc_rule_destroy_msg
 *      IPv4 multicast rule for destroy sub-messages.
 */
struct sfe_ipv4_mc_rule_destroy_msg {
	struct sfe_ipv4_5tuple tuple;           /**< Holds values of the 5 tuple. */
};

/**
 * The SFE IPv4 rule sync structure.
 */
struct sfe_ipv4_conn_sync {
	u32 index;			/**< Slot ID for cache statistics to host OS. */
	u8 protocol;			/**< Protocol number. */
	__be32 flow_ip;			/**< Flow IP address. */
	__be32 flow_ip_xlate;		/**< Translated flow IP address. */
	__be16 flow_ident;		/**< Flow identifier, e.g., port. */
	__be16 flow_ident_xlate;	/**< Translated flow identifier, e.g., port. */
	u32 flow_max_window;		/**< Flow direction's largest seen window. */
	u32 flow_end;			/**< Flow direction's largest seen sequence + segment length. */
	u32 flow_max_end;		/**< Flow direction's largest seen ack + max(1, win). */
	u32 flow_rx_packet_count;	/**< Flow interface's Rx packet count. */
	u32 flow_rx_byte_count;		/**< Flow interface's Rx byte count. */
	u32 flow_tx_packet_count;	/**< Flow interface's Tx packet count. */
	u32 flow_tx_byte_count;		/**< Flow interface's Tx byte count. */
	u16 flow_pppoe_session_id;	/**< Flow interface`s PPPoE session ID. */
	u16 flow_pppoe_remote_mac[3];	/**< Flow interface's PPPoE remote server MAC address (if present). */
	__be32 return_ip;		/**< Return IP address. */
	__be32 return_ip_xlate;		/**< Translated return IP address */
	__be16 return_ident;		/**< Return identifier, e.g., port. */
	__be16 return_ident_xlate;	/**< Translated return identifier, e.g., port. */
	u32 return_max_window;		/**< Return direction's largest seen window. */
	u32 return_end;			/**< Return direction's largest seen sequence + segment length. */
	u32 return_max_end;		/**< Return direction's largest seen ack + max(1, win). */
	u32 return_rx_packet_count;	/**< Return interface's Rx packet count. */
	u32 return_rx_byte_count;	/**< Return interface's Rx byte count. */
	u32 return_tx_packet_count;	/**< Return interface's Tx packet count. */
	u32 return_tx_byte_count;	/**< Return interface's Tx byte count. */
	u16 return_pppoe_session_id;	/**< Return interface`s PPPoE session ID. */
	u16 return_pppoe_remote_mac[3];	/**< Return interface's PPPoE remote server MAC address (if present). */
	u32 inc_ticks;			/**< Number of ticks since the last sync. */
	u32 reason;			/**< Synchronization reason. */

	u8 flags;			/**< Bit flags associated with the rule. */
	u32 qos_tag;			/**< QoS tag. */
	u32 cause;			/**< Flush cause. */
};

/**
 * Information for a multiple IPv4 connection statistics synchronization message.
 */
struct sfe_ipv4_conn_sync_many_msg {
	/*
	 * Request
	 */
	uint16_t index;		/**< Request connection statistics from the index. */
	uint16_t size;		/**< Buffer size of this message. */

	/*
	 * Response
	 */
	uint16_t next;		/**< Firmware response for the next connection to be requested. */
	uint16_t count;		/**< Number of synchronized connections included in this message. */
	struct sfe_ipv4_conn_sync conn_sync[];	/**< Array for the statistics. */
};

/**
 * Message structure to send/receive IPv4 bridge/route commands
 */
struct sfe_ipv4_msg {
	struct sfe_cmn_msg cm;					/**< Message header. */
	union {
		struct sfe_ipv4_rule_create_msg rule_create;	/**< Rule create message. */
		struct sfe_ipv4_rule_destroy_msg rule_destroy;	/**< Rule destroy message. */
		struct sfe_ipv4_conn_sync conn_stats;		/**< Connection statistics synchronization message. */
		struct sfe_ipv4_conn_sync_many_msg conn_stats_many;
					/**< Many connections' statistics synchronization message. */
		struct sfe_ipv4_mc_rule_create_msg mc_rule_create;/**< MC rule create message. */
		struct sfe_ipv4_mc_rule_destroy_msg mc_rule_destroy; /**<MC rule destroy message. */
	} msg;							/**< IPv4 message. */
};

/**
 * @}
 */

/**
 * @addtogroup nss_sfe_functions
 * @{
 */

/**
 * IPv4 message received callback.
 */
typedef void (*sfe_ipv4_msg_callback_t)(void *app_data, struct sfe_ipv4_msg *msg);

/**
 * @}
 */

/**
 * @addtogroup nss_sfe_datatypes
 * @{
 */

/**
 * IPv6 5-tuple structure.
 */
struct sfe_ipv6_5tuple {
	__be32 flow_ip[4];	/**< Flow IP address. */
	__be32 return_ip[4];	/**< Return IP address. */
	__be16 flow_ident;	/**< Flow identifier, e.g.,TCP/UDP port. */
	__be16 return_ident;	/**< Return identifier, e.g., TCP/UDP port. */
	u8  protocol;		/**< Protocol number. */
	u8  reserved[3];	/**< Reserved; padding for alignment. */
};

/**
 * IPv6 connection rule structure.
 */
struct sfe_ipv6_connection_rule {
	u8 flow_mac[6];			/**< Flow MAC address. */
	u8 return_mac[6];		/**< Return MAC address. */
	s32 flow_interface_num;		/**< Flow interface number. */
	s32 return_interface_num;	/**< Return interface number. */
	s32 flow_top_interface_num;	/**< Top flow interface number. */
	s32 return_top_interface_num;	/**< Top return interface number. */
	u32 flow_mtu;			/**< Flow interface's MTU. */
	u32 return_mtu;			/**< Return interface's MTU. */
	__be32 flow_ip_xlate[4];		/**< Translated flow IP address. */
	__be32 return_ip_xlate[4];		/**< Translated return IP address. */
	__be16 flow_ident_xlate;	/**< Translated flow identifier, e.g., port. */
	__be16 return_ident_xlate;	/**< Translated return identifier, e.g., port. */
};

/**
 * IPv6 rule create submessage structure.
 */
struct sfe_ipv6_rule_create_msg {
	/*
	 * Request
	 */
	u32 valid_flags;				/**< Bit flags associated with parameter validity. */
	u32 rule_flags;					/**< Bit flags associated with the rule. */
	struct sfe_ipv6_5tuple tuple;			/**< Holds values of the sfe_ipv6_5tuple tuple. */
	struct sfe_ipv6_connection_rule conn_rule;	/**< Basic connection-specific data. */
	struct sfe_protocol_tcp_rule tcp_rule;		/**< Protocol-related acceleration parameters. */
	struct sfe_pppoe_rule pppoe_rule;		/**< PPPoE-related acceleration parameters. */
	struct sfe_qos_rule qos_rule;			/**< QoS-related acceleration parameters. */
	struct sfe_src_mac_rule src_mac_rule;		/**< Source MAC address rule. */
	struct sfe_mark_rule mark_rule;			/**< SKB mark-related acceleration parameters. */
	struct sfe_dscp_rule dscp_rule;			/**< DSCP-related acceleration parameters. */
	struct sfe_vlan_rule vlan_primary_rule;		/**< VLAN-related acceleration parameters. */
	struct sfe_vlan_rule vlan_secondary_rule;	/**< VLAN-related acceleration parameters. */
	struct sfe_trustsec_rule trustsec_rule;		/**< Trustsec-related acceleration parameters. */

#ifdef CONFIG_XFRM
	struct sfe_acceleration_direction_rule direction_rule;
							/**< Direction-related acceleration parameters. */
#endif
	/*
	 * Response
	 */
	struct sfe_service_class_rule sawf_rule;	/**< Service class related information. */
	struct sfe_qdisc_rule qdisc_rule;		/**< Qdisc indication per direction */
	struct sfe_vlan_filter_rule flow_vlan_filter_rule;
							/**< TO-direction VLAN Filter related acceleration parameters. */
	struct sfe_vlan_filter_rule return_vlan_filter_rule;
							/**< FROM-direction VLAN Filter related acceleration parameters. */
	u32 index;					/**< Slot ID for cache statistics to host OS. */
};

/**
 * IPv6 rule destroy submessage structure.
 */
struct sfe_ipv6_rule_destroy_msg {
	struct sfe_ipv6_5tuple tuple;	/**< Holds values of the sfe_ipv6_5tuple tuple */
};

/*
 * Multicast destination interface entry
 */
struct sfe_ipv6_mc_device_entry {
	uint32_t rule_flags;		/**< Bit flag of rules indicating PPPOE session, Vlan. */
	uint32_t valid_flags;		/**< Valid Bit flags associated with the rule of vlan/pppoe. */
	uint32_t xlate_src_ip[4];	/**< nated source ip address. */
	uint32_t xlate_src_ident;	/**< nated source port number. */

	uint32_t egress_vlan_tag[SFE_MAX_VLAN_DEPTH]; /**< VLAN tag stack for the ingress packets. */
	uint32_t if_mtu;                        /* Interface MTU */
	uint32_t if_num;		/**< Dest interface number (virtual or physical). */
	uint16_t if_mac[3];                     /* Interface MAC address */
	u16 pppoe_session_id;			/* PPPOE header offset */
	u8 pppoe_remote_mac[ETH_ALEN];
	u8 egress_vlan_hdr_cnt;
	u8  reserved[1];			/**< Reserved; padding for alignment. */
	struct sfe_qdisc_rule qdisc_rule;		/**< Qdisc indication per direction */
};

/**
 * sfe_ipv6_mc_rule_create_msg
 *      IPv6 multicast rule for creating sub-messages.
 */
struct sfe_ipv6_mc_rule_create_msg {
	/* Request */
	u32 valid_flags;				/**< Bit flags associated with paramater validity. */
	u32 rule_flags;					/**< Bit flags associated with the rule. */

	struct sfe_ipv6_5tuple tuple;           /**< Holds values of the 5 tuple. */
	struct sfe_ipv6_connection_rule conn_rule;	/**< Basic connection-specific data. */
	struct sfe_pppoe_rule pppoe_rule;		/**< PPPoE-related acceleration parameters. */
	struct sfe_qos_rule qos_rule;			/**< QoS-related acceleration parameters. */
	struct sfe_src_mac_rule src_mac_rule;		/**< Source MAC address rule. */
	struct sfe_mark_rule mark_rule;			/**< SKB mark-related acceleration parameters. */
	struct sfe_dscp_rule dscp_rule;			/**< DSCP-related acceleration parameters. */
	struct sfe_vlan_rule vlan_primary_rule;		/**< Primary VLAN-related acceleration parameters. */
	struct sfe_vlan_rule vlan_secondary_rule;	/**< Secondary VLAN-related acceleration parameters. */
#ifdef CONFIG_XFRM
	struct sfe_acceleration_direction_rule direction_rule;
							/**< Direction-related acceleration parameters. */
#endif
	struct sfe_service_class_rule sawf_rule;	/**< Service class related information. */
	uint16_t dest_mac[3];                   /**< Destination multicast MAC address. */
	uint16_t if_cnt;                      /**< Number of destination interfaces. */
	struct sfe_ipv6_mc_device_entry if_rule[SFE_MC_IF_MAX];	/**< Per-interface information. */

};

/**
 * sfe_ipv6_mc_rule_destroy_msg
 *      IPv6 multicast rule for destroy sub-messages.
 */
struct sfe_ipv6_mc_rule_destroy_msg {
	struct sfe_ipv6_5tuple tuple;           /**< Holds values of the 5 tuple. */
};

/**
 * SFE IPv6 rule sync structure.
 */
struct sfe_ipv6_conn_sync {
	u32 index;			/**< Slot ID for cache statistics to host OS. */
	u8 protocol;			/**< Protocol number. */
	__be32 flow_ip[4];		/**< Flow IP address. */
	__be32 flow_ip_xlate[4];		/**< Translated flow IP address. */
	__be16 flow_ident;		/**< Flow identifier, e.g., port. */
	__be16 flow_ident_xlate;	/**< Translated flow identifier, e.g., port. */
	u32 flow_max_window;		/**< Flow direction's largest seen window. */
	u32 flow_end;			/**< Flow direction's largest seen sequence + segment length. */
	u32 flow_max_end;		/**< Flow direction's largest seen ack + max(1, win). */
	u32 flow_rx_packet_count;	/**< Flow interface's Rx packet count. */
	u32 flow_rx_byte_count;		/**< Flow interface's Rx byte count. */
	u32 flow_tx_packet_count;	/**< Flow interface's Tx packet count. */
	u32 flow_tx_byte_count;		/**< Flow interface's Tx byte count. */
	u16 flow_pppoe_session_id;	/**< Flow interface`s PPPoE session ID. */
	u16 flow_pppoe_remote_mac[3];	/**< Flow interface's PPPoE remote server MAC address (if present). */
	__be32 return_ip[4];		/**< Return IP address. */
	__be32 return_ip_xlate[4];		/**< Translated return IP address. */
	__be16 return_ident;		/**< Return identifer, e.g., port. */
	__be16 return_ident_xlate;	/**< Translated return identifier, e.g., port. */
	u32 return_max_window;		/**< Return direction's largest seen window. */
	u32 return_end;			/**< Return direction's largest seen sequence + segment length. */
	u32 return_max_end;		/**< Return direction's largest seen ack + max(1, win). */
	u32 return_rx_packet_count;	/**< Return interface's Rx packet count. */
	u32 return_rx_byte_count;	/**< Return interface's Rx byte count. */
	u32 return_tx_packet_count;	/**< Return interface's Tx packet count. */
	u32 return_tx_byte_count;	/**< Return interface's Tx byte count. */
	u16 return_pppoe_session_id;	/**< Return interface`s PPPoE session ID. */
	u16 return_pppoe_remote_mac[3];	/**< Return interface's PPPoE remote server MAC address (if present). */
	u32 inc_ticks;			/**< Number of ticks since the last sync. */
	u32 reason;			/**< Sync reason. */
	u8 flags;			/**< Bit flags associated with the rule. */
	u32 qos_tag;			/**< QoS tag. */
	u32 cause;			/**< Flush cause associated with the rule. */
};

/**
 * Information for a multiple IPv6 connection statistics synchronization message.
 */
struct sfe_ipv6_conn_sync_many_msg {
	/*
	 * Request:
	 */
	uint16_t index;		/**< Request connection statistics from the index. */
	uint16_t size;		/**< Buffer size of this message. */

	/*
	 * Response:
	 */
	uint16_t next;		/**< Firmware response for the next connection to be requested. */
	uint16_t count;		/**< Number of synchronized connections included in this message. */
	struct sfe_ipv6_conn_sync conn_sync[];	/**< Array for the statistics. */
};

/**
 * Message structure to send/receive IPv6 bridge/route commands.
 */
struct sfe_ipv6_msg {
	struct sfe_cmn_msg cm;		/**< Message header. */
	union {
		struct sfe_ipv6_rule_create_msg rule_create;
					/**< Rule create message. */
		struct sfe_ipv6_rule_destroy_msg rule_destroy;
					/**< Rule destroy message. */
		struct sfe_ipv6_conn_sync conn_stats;
					/**< Statistics synchronization message. */
		struct sfe_ipv6_conn_sync_many_msg conn_stats_many;
					/**< Many Connections' statistics synchronizaion message. */
		struct sfe_ipv6_mc_rule_create_msg mc_rule_create;/**< MC rule create message. */
		struct sfe_ipv6_mc_rule_destroy_msg mc_rule_destroy; /**<MC rule destroy message. */

	} msg;				/**< IPv6 message. */
};

/**
 * @}
 */

/**
 * @addtogroup nss_sfe_functions
 * @{
 */

/**
 * IPv6 message received callback.
 */
typedef void (*sfe_ipv6_msg_callback_t)(void *app_data, struct sfe_ipv6_msg *msg);

/**
 * @}
 */

/**
 * @addtogroup nss_sfe_datatypes
 * @{
 */

/**
 * 6rd tunnel peer address.
 */
struct sfe_tun6rd_set_peer_msg {
	__be32 ipv6_address[4];	/**< The peer's IPv6 address. */
	__be32 dest;		/**< The peer's IPv4 address. */
};

/**
 * Message structure to send/receive 6rd tunnel messages.
 */
struct sfe_tun6rd_msg {
	struct sfe_cmn_msg cm;				/**< Message header. */
	union {
		struct sfe_tun6rd_set_peer_msg peer;	/**< Add or update peer message. */
	} msg;						/**< 6RD tunnel message. */
};

/**
 * SFE context instance.
 */
struct sfe_ctx_instance {
	int not_used;	/**< Not used. */
};

/**
 * @}
 */

/**
 * @addtogroup nss_sfe_functions
 * @{
 */

/**
 * Copy the IPv4 statistics for the given service class.
 *
 * @param	sid	Service class ID
 * @param	bytes	Pointer to where byte count should be written.
 * @param	packets	Pointer to where packet count should be written.
 *
 * @return
 * True if successful, false if maximum retries exceeded; bool.
 */
extern bool sfe_service_class_stats_get(uint8_t sid, uint64_t *bytes, uint64_t *packets);

/**
 * Gets the maximum number of IPv4 connections supported by the SFE acceleration engine.
 *
 * @return
 * The maximum number of connections that can be accelerated by the SFE.
 */
int sfe_ipv4_max_conn_count(void);

/**
 * Transmits an IPv4 message to the SFE.
 *
 * @param	sfe_ctx		SFE context.
 * @param	msg		The IPv4 message.
 *
 * @return
 * The status of the Tx operation (#sfe_tx_status_t).
 */
extern sfe_tx_status_t sfe_ipv4_tx(struct sfe_ctx_instance *sfe_ctx, struct sfe_ipv4_msg *msg);

/**
 * Registers a notifier callback for IPv4 messages from the SFE.
 *
 * @param	one_rule_cb		The callback pointer for one rule.
 * @param	many_rules_cb		The callback pointer for many rules.
 * @param	app_data	The application context for this message.
 *
 * @return
 * The SFE context (#sfe_ctx_instance).
 */
extern struct sfe_ctx_instance *sfe_ipv4_notify_register(sfe_ipv4_msg_callback_t one_rule_cb,
		sfe_ipv4_msg_callback_t many_rules_cb,void *app_data);

/**
 * Unregisters a notifier callback for IPv4 messages from the SFE.
 *
 * @return
 * None.
 */
extern void sfe_ipv4_notify_unregister(void);

/**
 * Initializes an IPv4 message.
 *
 * @param	nim		The IPv4 message pointer.
 * @param	if_num		The interface number.
 * @param	type		The type of the message.
 * @param	len		The length of the message.
 * @param	cb		The message callback.
 * @param	app_data	The application context for this message.
 *
 */
extern void sfe_ipv4_msg_init(struct sfe_ipv4_msg *nim, u16 if_num, u32 type, u32 len,
			sfe_ipv4_msg_callback_t cb, void *app_data);

/**
 * Gets the maximum number of IPv6 connections supported by the SFE acceleration engine.
 *
 * @return
 *  The maximum number of connections that can be accelerated by the SFE; integer.
 */
int sfe_ipv6_max_conn_count(void);

/**
 * Transmits an IPv6 message to the SFE.
 *
 * @param	sfe_ctx		The SFE context.
 * @param	msg		The IPv6 message.
 *
 * @return
 * The status of the Tx operation (#sfe_tx_status_t).
 */
extern sfe_tx_status_t sfe_ipv6_tx(struct sfe_ctx_instance *sfe_ctx, struct sfe_ipv6_msg *msg);

/**
 * Registers a notifier callback for IPv6 messages from the SFE.
 *
 * @param	one_rule_cb		The callback pointer for one rule.
 * @param	many_rules_cb		The callback pointer for many rules.
 *
 * @return
 * The SFE context (#sfe_ctx_instance).
 */
extern struct sfe_ctx_instance *sfe_ipv6_notify_register(sfe_ipv6_msg_callback_t one_rule_cb,
		sfe_ipv6_msg_callback_t many_rules_cb,void *app_data);

/**
 * Unregisters a notifier callback for IPv6 messages from the SFE.
 */
extern void sfe_ipv6_notify_unregister(void);

/**
 * Initializes an IPv6 message.
 *
 * @param	nim		The IPv6 message pointer.
 * @param	if_num		The interface number.
 * @param	type		The type of the message.
 * @param	len		The length of the message.
 * @param	cb		The message callback.
 * @param	app_data	The application context for this message.
 *
 * @return
 * None.
 */
extern void sfe_ipv6_msg_init(struct sfe_ipv6_msg *nim, u16 if_num, u32 type, u32 len,
			sfe_ipv6_msg_callback_t cb, void *app_data);

/**
 * Transmits a 6rd tunnel message to the SFE.
 *
 * @param	sfe_ctx		The SFE context pointer.
 * @param	msg		The 6rd tunnel message pointer.
 *
 * @return
 * The status of the Tx operation (#sfe_tx_status_t).
 */
sfe_tx_status_t sfe_tun6rd_tx(struct sfe_ctx_instance *sfe_ctx, struct sfe_tun6rd_msg *msg);

/**
 * Initializes a 6rd tunnel message.
 *
 * @param	ncm		The 6rd tunnel message pointer.
 * @param	if_num		The interface number.
 * @param	type		The type of the message.
 * @param	len		The length of the message.
 * @param	cb		The message callback.
 * @param	app_data	The application context for this message.
 *
 * @return
 * None.
 */
void sfe_tun6rd_msg_init(struct sfe_tun6rd_msg *ncm, u16 if_num, u32 type,  u32 len,
			 void *cb, void *app_data);

/**
 * Indicates whether the l2 feature flag is enabled or disabled.
 *
 * @return
 * True if enabled; false if disabled.
 */
bool sfe_is_l2_feature_enabled(void);

/**
 * Updates mark values of an IPv4 connection.
 *
 * @param	mark		The mark object.
 *
 * @return
 * None.
 */
void sfe_ipv4_mark_rule_update(struct sfe_connection_mark *mark);

/**
 * Updates mark values of an IPv6 connection.
 *
 * @param	mark		The mark object.
 *
 * @return
 * None.
 */
void sfe_ipv6_mark_rule_update(struct sfe_connection_mark *mark);

/**
 * Gets the acceleration mode of PPPoE bridge.
 *
 * @return
 * The acceleration mode.
 */
sfe_pppoe_br_accel_mode_t sfe_pppoe_get_br_accel_mode(void);

/**
 * Flow statistics connection create callback.
 *
 * @param	ip_version	The IP version of the connection, must equal 4 for IPv4 or 6 for IPv6.
 * @param	protocol	The protocol number of the connection.
 * @param	orig_src_ip	A pointer to the source IP in the original direction. If IPv6, a pointer to an array of length 4.
 * @param	orig_src_port	Source port number in the original direcion.
 * @param	orig_dst_ip	A pointer to the destination IP address in the original direction. If IPv6, a pointer to an array of length 4.
 * @param	orig_dst_port	Destination port number in the original direcion.
 * @param	repl_src_ip	A pointer to the source IP address in the reply direction. If IPv6, a pointer to an array of length 4.
 * @param	repl_src_port	Source port number in the reply direcion.
 * @param	repl_dst_ip	A pointer to the destination IP address in the reply direction. If IPv6, a pointer to an array of length 4.
 * @param	repl_dst_port	Destination port number in the reply direcion.
 * @param	orig_conn	Pointer to where the original direction connection entry pointer will be written.
 * @param	repl_conn	Pointer to where the reply direction connection entry pointer will be written.
 */
typedef void (*sfe_fls_conn_create_t)(uint8_t ip_version,
					uint8_t protocol,
					uint32_t *orig_src_ip,
					uint16_t orig_src_port,
					uint32_t *orig_dest_ip,
					uint16_t orig_dest_port,
					uint32_t *repl_src_ip,
					uint16_t repl_src_port,
					uint32_t *repl_dest_ip,
					uint16_t repl_dest_port,
					void **orig_conn,
					void **repl_conn);

/**
 * Flow statistics connection delete callback.
 *
 * @param	conn	The entry to be deleted.
 */
typedef void (*sfe_fls_conn_delete_t)(void *conn);

/**
 * Flow statisticics connection statistics update callback.
 *
 * @param	conn	The entry to be updated.
 * @param	skb	The sk_buff to use for statistics.
 */
typedef bool (*sfe_fls_conn_stats_update_t)(void *conn, struct sk_buff *skb);

/**
 * Registers flow statistics methods with SFE.
 *
 * @param	create_cb		Create connection callback.
 * @param	delete_cb		Delete connection callback.
 * @param	stats_update_cb		Update statistics callback.
 *
 * @return
 * None.
 */
void sfe_fls_register(sfe_fls_conn_create_t create_cb, sfe_fls_conn_delete_t delete_cb, sfe_fls_conn_stats_update_t stats_update_cb);

/**
 * Unregisters flow statistics methods from SFE.
 *
 * @return
 * None.
 */
void sfe_fls_unregister(void);

/**
 * @}
 */

#endif /* __SFE_API_H */
