/*
 * Copyright (c) 2022-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * @file ppe_drv_v6.h
 *	NSS PPE IPv6 specific definitions.
 */

#ifndef _PPE_DRV_V6_H_
#define _PPE_DRV_V6_H_

/**
 * @addtogroup ppe_drv_v6_subsystem
 * @{
 */

/**
 * Converts the format of an IPv6 address from Linux to PPE-drv.
 */
#define PPE_DRV_IN6_TO_IPV6(ipv6, in6) \
{ \
	((uint32_t *)ipv6)[0] = ntohl(in6.in6_u.u6_addr32[0]); \
	((uint32_t *)ipv6)[1] = ntohl(in6.in6_u.u6_addr32[1]); \
	((uint32_t *)ipv6)[2] = ntohl(in6.in6_u.u6_addr32[2]); \
	((uint32_t *)ipv6)[3] = ntohl(in6.in6_u.u6_addr32[3]); \
}

/**
 * Converts the format of an IPv6 address from PPE-drv to Linux.
 */
#define PPE_DRV_IPV6_TO_IN6(in6, ipv6) \
{ \
	in6.in6_u.u6_addr32[0] = htonl(((uint32_t *)ipv6)[0]); \
	in6.in6_u.u6_addr32[1] = htonl(((uint32_t *)ipv6)[1]); \
	in6.in6_u.u6_addr32[2] = htonl(((uint32_t *)ipv6)[2]); \
	in6.in6_u.u6_addr32[3] = htonl(((uint32_t *)ipv6)[3]); \
}

/*
 * v6 rule flags
 */
#define PPE_DRV_V6_RULE_FLAG_BRIDGE_FLOW		0x0001	/**< Bridge Flow */
#define PPE_DRV_V6_RULE_FLAG_ROUTED_FLOW		0x0002	/**< Rule is for a routed connection */
#define PPE_DRV_V6_RULE_FLAG_DSCP_MARKING		0x0004  /**< Rule creation for DSCP marking */
#define PPE_DRV_V6_RULE_FLAG_VLAN_MARKING		0x0008	/**< Rule creation for VLAN marking */
#define PPE_DRV_V6_RULE_FLAG_FLOW_VALID			0x0010	/**< Rule creation for flow direction */
#define PPE_DRV_V6_RULE_FLAG_RETURN_VALID		0x0020	/**< Rule creation for return direction */
#define PPE_DRV_V6_RULE_FLAG_PPPOE_VALID		0x0040	/**< Rule creation for PPPoe */
#define PPE_DRV_V6_RULE_FLAG_DS_FLOW			0x0080	/**< Rule creation for DS flow */
#define PPE_DRV_V6_RULE_FLAG_VP_FLOW			0x0100	/**< Rule creation for VP flow */
#define PPE_DRV_V6_RULE_FLAG_SRC_INTERFACE_CHECK	0x0200	/**< Rule creation for source interface check */
#define PPE_DRV_V6_RULE_TO_BRIDGE_VLAN_NETDEV		0x0400  /**< VLAN over bridge in egress direction */
#define PPE_DRV_V6_RULE_FROM_BRIDGE_VLAN_NETDEV		0x0800  /**< VLAN over bridge in ingress direction */
#define PPE_DRV_V6_RULE_NOEDIT_FLOW_RULE		0x1000  /**< Noedit rule creation for flow direction */
#define PPE_DRV_V6_RULE_NOEDIT_RETURN_RULE		0x2000  /**< Noedit rule creation for return direction */

/*
 * v6 valid flags
 */
#define PPE_DRV_V6_VALID_FLAG_PPPOE_FLOW	0x0001  /**< PPPoE fields are valid for flow direction. */
#define PPE_DRV_V6_VALID_FLAG_PPPOE_RETURN	0x0002  /**< PPPoE fields are valid for return direction. */
#define PPE_DRV_V6_VALID_FLAG_VLAN		0x0004  /**< VLAN fields are valid. */
#define PPE_DRV_V6_VALID_FLAG_QOS		0x0008  /**< QoS fields are valid. */
#define PPE_DRV_V6_VALID_FLAG_DSCP_MARKING	0x0010  /**< DSCP fields are valid. */
#define PPE_DRV_V6_VALID_FLAG_SAWF		0x0020  /**< SAWF fields are valid. */
#define PPE_DRV_V6_VALID_FLAG_ACL_POLICER	0x0040  /**< ACL/Policer fields are valid. */
#define PPE_DRV_V6_VALID_FLAG_WIFI_TID		0x0080	/**< HLOS TID fields are valid. */
#define PPE_DRV_V6_VALID_FLAG_SCS		0x0100	/**< SCS fields are valid. */
#define PPE_DRV_V6_VALID_FLAG_FLOW_WIFI_MDATA	0x0200  /**< Wi-Fi flow metadata is valid. */
#define PPE_DRV_V6_VALID_FLAG_RETURN_WIFI_MDATA	0x0400  /**< Wi-Fi return metadata is valid. */
#define PPE_DRV_V6_VALID_FLAG_FLOW_WIFI_DS	0x0800  /**< Wi-Fi DS flow field is valid. */
#define PPE_DRV_V6_VALID_FLAG_RETURN_WIFI_DS	0x1000  /**< Wi-Fi DS return field is valid. */

#define PPE_DRV_V6_MAX_CONN_COUNT               1024

/**
 * ppe_drv_v6_5tuple
 *	Common 5-tuple information.
 */
struct ppe_drv_v6_5tuple {
	uint32_t flow_ip[4];		/**< Flow IP address. */
	uint32_t flow_ident;		/**< Flow identifier (e.g., TCP or UDP port). */
	uint32_t return_ip[4];		/**< Return IP address. */
	uint32_t return_ident;		/**< Return identier (e.g., TCP or UDP port). */
	uint8_t protocol;		/**< Protocol number. */
};

/**
 * ppe_drv_v6_connection_rule
 *	Information for creating a connection.
 */
struct ppe_drv_v6_connection_rule {
	uint8_t flow_mac[ETH_ALEN];		/**< Flow MAC address. */
	uint8_t return_mac[ETH_ALEN];		/**< Return MAC address. */
	uint16_t flow_mtu;			/**< MTU for the flow interface. */
	uint16_t return_mtu;			/**< MTU for the return interface. */
	ppe_drv_iface_t rx_if;			/**< From PPE interface number */
	ppe_drv_iface_t tx_if;			/**< To PPE interface number */
};

/**
 * ppe_drv_v6_rule_create
 *	PPE IPv6 rule create structure.
 */
struct ppe_drv_v6_rule_create {
	/*
	 * Request
	 */
	uint16_t valid_flags;				/**< Bit flags associated with the validity of parameters. */
	uint16_t rule_flags;				/**< Bit flags associated with the rule. */
	struct ppe_drv_v6_5tuple tuple;			/**< Holds values of the 5 tuple. */
	struct ppe_drv_v6_connection_rule conn_rule;	/**< Basic connection-specific data. */
	struct ppe_drv_pppoe_rule pppoe_rule;		/**< PPPoE-related acceleration parameters. */
	struct ppe_drv_qos_rule qos_rule;		/**< QoS-related acceleration parameters. */
	struct ppe_drv_dscp_rule dscp_rule;		/**< DSCP-related acceleration parameters. */
	struct ppe_drv_vlan_rule vlan_rule;		/**< VLAN-related acceleration parameters. */
	struct ppe_drv_top_if_rule top_rule;		/**< Parameters related to the top interface in hierarchy. */
	struct ppe_drv_service_class_rule sawf_rule;    /**< Service class related information. */
	struct ppe_drv_acl_policer_rule ap_rule;	/**< ACL/Policer rule ID information. */
	struct ppe_drv_wifi_mdata_rule wifi_rule;	/**< Wi-Fi metadata rule ID information. */
};

/**
 * ppe_drv_v6_rule_destroy
 *	PPE IPv6 rule destroy structure.
*/
struct ppe_drv_v6_rule_destroy {
        struct ppe_drv_v6_5tuple tuple;			/**< Holds values of the 5 tuple. */
};

/*
 * ppe_drv_v6_conn_sync
 *	PPE connection sync structure for one connection.
 */
struct ppe_drv_v6_conn_sync {
        uint8_t protocol;				/**< Protocol number. */
        uint32_t flow_ip[4];				/**< Flow IP address. */
        uint32_t flow_ident;				/**< Flow ident (e.g. port). */
        uint32_t flow_rx_packet_count;			/**< Flow interface's RX packet count. */
        uint32_t flow_rx_byte_count;			/**< Flow interface's RX byte count. */
        uint32_t flow_tx_packet_count;			/**< Flow interface's TX packet count. */
        uint32_t flow_tx_byte_count;			/**< Flow interface's TX byte count. */
        uint32_t return_ip[4];				/**< Return IP address. */
        uint32_t return_ident;				/**< Return ident (e.g. port). */
        uint32_t return_rx_packet_count;		/**< Return interface's RX packet count. */
        uint32_t return_rx_byte_count;			/**< Return interface's RX byte count. */
        uint32_t return_tx_packet_count;		/**< Return interface's TX packet count. */
        uint32_t return_tx_byte_count;			/**< Return interface's TX byte count. */
        enum ppe_drv_stats_sync_reason reason;		/**< Reason for the sync. */
};

/*
 * ppe_drv_v6_flow_conn_stats
 *	PPE connection stats for a single connection
 */
struct ppe_drv_v6_flow_conn_stats {
	struct ppe_drv_v6_5tuple tuple;			/**< Holds value of 5 tuple. */
	struct ppe_drv_v6_conn_sync conn_sync;		/**< Connection stats */
};

/*
 * ppe_drv_v6_conn_sync_many
 *	PPE connection sync many structure.
 */
struct ppe_drv_v6_conn_sync_many {
        uint16_t count;					/* How many conn_sync included in this sync callback */
        struct ppe_drv_v6_conn_sync *conn_sync;	/* Connection sync array */
};

/**
 * Callback function for syncing IPv6 connection stats.
 *
 * @datatypes
 * ppe_drv_v6_conn_sync
 *
 * @param[in] app_data    Pointer to the user context registered the callback.
 * @param[in] conn_sync   Pointer to the connection sync message data structure.
 */
typedef void (*ppe_drv_v6_sync_callback_t)(void *app_data, struct ppe_drv_v6_conn_sync *conn_sync);

/**
 * ppe_drv_v6_stats_callback_unregister
 *	API to unregister IPv6 connection stats sync callback.
 *
 * @return
 */
void ppe_drv_v6_stats_callback_unregister(void);

/**
 * ppe_drv_v6_stats_callback_register
 *	API to register IPv6 connection stats sync callback.
 *
 * @param[in] cb         Pointer to the callback function.
 * @param[in] app_data   Pointer to the app data which is passed with the callback.
 *
 * @return
 * Status of the register operation.
 */
bool ppe_drv_v6_stats_callback_register(ppe_drv_v6_sync_callback_t cb, void *app_data);

/**
 * ppe_drv_v6_get_conn_stats
 *	API to get a single connection stats.
 *
 * @param[in] conn_stats	Pointer to the connection stats structure.
 *
 * @return
 * Status of the stats sync.
 */
ppe_drv_ret_t ppe_drv_v6_get_conn_stats(struct ppe_drv_v6_flow_conn_stats *conn_stats);

/**
 * ppe_drv_v6_conn_sync_many
 *	API to get v6 connection stats.
 *
 * @param[in] cn_syn     Pointer to the buffer in which stats are filled.
 * @param[in] num_conn   Number of connection stats which can be filled in one iteration.
 *
 * @return
 * void
 */
void ppe_drv_v6_conn_sync_many(struct ppe_drv_v6_conn_sync_many *cn_syn, uint8_t num_conn);

/**
 * ppe_drv_v6_rfs_destroy
 * 	Destroys IPv6 RFS connection rule in PPE.
 *	This function is deprecated, pls use ppe_drv_v6_assist_rule_destroy() instead
 *
 * @datatypes
 * ppe_drv_v6_rfs_destroy
 *
 * @param[in] destroy   Pointer to the NSS PPE IPv6 destroy rule message.
 *
 * @return
 * Status of the destroy operation.
 */
ppe_drv_ret_t ppe_drv_v6_rfs_destroy(struct ppe_drv_v6_rule_destroy *destroy);

/**
 * ppe_drv_v6_rfs_create
 *      Creates IPv6 RFS connection rule in PPE.
 *	This function is deprecated, pls use ppe_drv_v6_assist_rule_create() instead
 *
 * @datatypes
 * ppe_drv_v6_rfs_create
 *
 * @param[in] create   Pointer to the NSS PPE IPv6 create rule message.
 *
 * @return
 * Status of the create operation.
 */
ppe_drv_ret_t ppe_drv_v6_rfs_create(struct ppe_drv_v6_rule_create *create);

/**
 * ppe_drv_v6_assist_rule_destroy
 *	Destroys IPv6 Assist connection rule in PPE.
 *
 * @param[in] rule_type PPE Assist rule type.
 * @param[in] destroy   Pointer to the NSS PPE IPv6 destroy rule message.
 *
 * @return
 * Status of the destroy operation.
 */
ppe_drv_ret_t ppe_drv_v6_assist_rule_destroy(struct ppe_drv_v6_rule_destroy *destroy);

/**
 * ppe_drv_v6_assist_rule_create
 *	Creates IPv6 Assist connection rule in PPE.
 *
 * @param[in] create	Pointer to the NSS PPE ASSIST IPv6 create rule message.
 * @param[in] feature 	PPE Assist feature type.
 *
 * @return
 * Status of the create operation.
 */
ppe_drv_ret_t ppe_drv_v6_assist_rule_create(struct ppe_drv_v6_rule_create *create, uint32_t feature);

/**
 * ppe_drv_v6_policer_flow_create
 *	Creates IPv6 policer rule in PPE.
 *
 * @datatypes
 * ppe_drv_v6_policer_create
 *
 * @param[in] create   Pointer to the NSS PPE IPv6 create rule message.
 *
 * @return
 * Status of the create operation.
 */
ppe_drv_ret_t ppe_drv_v6_policer_flow_create(struct ppe_drv_v6_rule_create *create);

/**
 * ppe_drv_v6_policer_flow_destroy
 *	Destroy IPv6 policer rule in PPE.
 *
 * @datatypes
 * ppe_drv_v6_policer_destroy
 *
 * @param[in] destroy   Pointer to the NSS PPE IPv6 destroy rule message.
 *
 * @return
 * Status of the destroy operation.
 */
ppe_drv_ret_t ppe_drv_v6_policer_flow_destroy(struct ppe_drv_v6_rule_destroy *destroy);

/**
 * ppe_drv_v6_destroy
 *	Destroys IPv6 connection rule in PPE.
 *
 * @datatypes
 * ppe_drv_v6_rule_destroy
 *
 * @param[in] destroy   Pointer to the NSS PPE IPv6 destroy rule message.
 *
 * @return
 * Status of the destroy operation.
 */
ppe_drv_ret_t ppe_drv_v6_destroy(struct ppe_drv_v6_rule_destroy *destroy);

/**
 * ppe_drv_v6_create
 *	Creates IPv6 connection rule in PPE.
 *
 * @datatypes
 * ppe_drv_v6_rule_create
 *
 * @param[in] create   Pointer to the NSS PPE IPv6 create rule message.
 *
 * @return
 * Status of the create operation.
 */
ppe_drv_ret_t ppe_drv_v6_create(struct ppe_drv_v6_rule_create *create);

/**
 * ppe_drv_v6_nsm_stats_update
 *	Update stats in NSM for the given 5 tuple.
 *
 * @param[in] nsm_stats		Pointer to stats maintained in NSM.
 * @param[in] tuple		5 tuple corresponding to which stats are needed.
 *
 * @return
 * Status of the API.
 */
extern bool ppe_drv_v6_nsm_stats_update(struct ppe_drv_nsm_stats *nsm_stats, struct ppe_drv_v6_5tuple *tuple);

/** @} */ /* end_addtogroup ppe_drv_v6_subsystem */

#endif /* _PPE_DRV_V6_H_ */

