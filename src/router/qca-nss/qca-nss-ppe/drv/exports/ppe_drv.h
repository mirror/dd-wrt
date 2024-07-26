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
 * @file ppe_drv.h
 *	NSS PPE driver definitions.
 */

#ifndef _PPE_DRV_H_
#define _PPE_DRV_H_

/**
 * @addtogroup ppe_drv_subsystem
 * @{
 */
#include <linux/module.h>
#include <linux/if_ether.h>
#include "ppe_drv_iface.h"
#include <linux/netdevice.h>

/*
 * FSE flags
 */
#define PPE_DRV_FSE_IPV4 0x00000001	/**< Indicate if flow is IPv4 to FSE. */
#define PPE_DRV_FSE_IPV6 0x00000002	/**< Indicate if flow is IPv6 to FSE. */
#define PPE_DRV_FSE_DS 0x00000004	/**< Indicate if flow is Direct switch. */

/*
 * PPE INT PRI Min/Max values
 */
#define PPE_DRV_INT_PRI_MIN 0
#define PPE_DRV_INT_PRI_MAX 15

#define PPE_DRV_MHT_SWITCH_ID	1			/**< MHT switch ID */

/*
 * PPE Assist feature flags
 */
#define PPE_DRV_ASSIST_FEATURE_RFS      0x00000001      /* PPE Assist feature to configure RFS */
#define PPE_DRV_ASSIST_FEATURE_PRIORITY 0x00000002      /* PPE Assist feature to configure Priority */

#define PPE_DRV_SERVICE_CLASS_IS_VALID(sc)	((sc >= PPE_DRV_SAWF_SC_START) && (sc <= PPE_DRV_SAWF_SC_END))

/*
 * PPE QOS VALID FLAGS
 */
#define PPE_DRV_VALID_FLAG_FLOW_PPE_QOS		0x01	/**< PPE QOS is enabled in flow direction. */
#define PPE_DRV_VALID_FLAG_RETURN_PPE_QOS	0x02	/**< PPE QOS is enabled in return direction. */

/*
 * ACL/POLICER VALID FLAGS
 */
#define PPE_DRV_VALID_FLAG_FLOW_ACL		0x01	/**< ACL is enabled in flow direction. */
#define PPE_DRV_VALID_FLAG_RETURN_ACL		0x02	/**< ACL is enabled in return direction. */
#define PPE_DRV_VALID_FLAG_FLOW_POLICER		0x01	/**< Policer is enabled in flow direction. */
#define PPE_DRV_VALID_FLAG_RETURN_POLICER	0x02	/**< Policer is enabled in return direction. */

/*
 * ppe_drv_ip_type
 *	Types of IP addresses handled
 */
enum ppe_drv_ip_type {
	PPE_DRV_IP_TYPE_V4,	/**< IPv4 unicast IP-type. */
	PPE_DRV_IP_TYPE_V6,	/**< IPv6 unicast IP-type. */
	PPE_DRV_IP_TYPE_MC_V4,	/**< IPv4 multicast IP-type. */
	PPE_DRV_IP_TYPE_MC_V6	/**< IPv6 multicast IP-type. */
};

/*
 * ppe_drv_sawf_sc_type
 *	Total types of service class
 */
typedef enum ppe_drv_sawf_sc_type {
	PPE_DRV_SAWF_SC_NONE = 0,	/**< Invalid service class. */
	PPE_DRV_SAWF_SC_START,		/**< SAWF service class start. */
	PPE_DRV_SAWF_SC_END = 128,	/**< SAWF service class end. */
	PPE_DRV_SAWF_SC_MAX,		/**< Maximum number of SAWF service classes */
} ppe_drv_sawf_sc_t;

/*
 * ppe_drv_tree_id_type
 *	Type for different Tree-ID configuration
 */
typedef enum ppe_drv_tree_id_type {
	PPE_DRV_TREE_ID_TYPE_NONE = 0,	/**< Normal processing */
	PPE_DRV_TREE_ID_TYPE_SAWF,	/**< SAWF usecase */
	PPE_DRV_TREE_ID_TYPE_WIFI_TID,	/**< HLOS TID usecase */
	PPE_DRV_TREE_ID_TYPE_SCS,	/**< SCS usecase */
	PPE_DRV_TREE_ID_TYPE_MLO_ASSIST,	/**< MLO usecase */
	PPE_DRV_TREE_ID_TYPE_MAX = 16,	/**< Maximum number of supported types */
} ppe_drv_tree_id_type_t;

/*
 * ppe_drv_fse_tuple
 *	fse tuple
 */
struct ppe_drv_fse_tuple {
	uint32_t src_ip[4];		/**< Flow IP address. */
	uint32_t src_port;		/**< Flow identifier (e.g., TCP or UDP port). */
	uint32_t dest_ip[4];		/**< Return IP address. */
	uint32_t dest_port;		/**< Return identifier (e.g., TCP or UDP port). */
	uint8_t protocol;		/**< Protocol number. */
};

/*
 * ppe_drv_fse_rule_info
 *	Information to pass from PPE to FSE module
 */
struct ppe_drv_fse_rule_info {
	struct ppe_drv_fse_tuple tuple;		/**< 5 tuple information. */
	struct net_device *dev;			/**< VAP netdevice. */
	uint32_t flags;				/**< Info flag */
	uint8_t vp_num;			/**< Virtual port number. */
};

/*
 * ppe_drv_fse_ops
 *	FSE operations
 */
struct ppe_drv_fse_ops {
	bool (*create_fse_rule)(struct ppe_drv_fse_rule_info *finfo);	/**< Function pointer to create FSE rules. */
	bool (*destroy_fse_rule)(struct ppe_drv_fse_rule_info *finfo);	/**< Function pointer to destroy FSE rules. */
};

/**
 * ppe_drv_pppoe_session
 *	Information for PPPoE session.
 */
struct ppe_drv_pppoe_session {
	uint16_t session_id;				/**< Session id */
	uint8_t server_mac[ETH_ALEN];			/**< Server MAC address */
};

/**
 * ppe_drv_pppoe_rule
 *	Information for PPPoE connection rules.
 */
struct ppe_drv_pppoe_rule {
	struct ppe_drv_pppoe_session flow_session;		/**< Flow PPPoE session */
	struct ppe_drv_pppoe_session return_session;		/**< Return PPPoE session */
};

/**
 * ppe_drv_dscp_rule
 *	Information for DSCP connection rules.
 */
struct ppe_drv_dscp_rule {
	uint8_t flow_dscp;		/**< Egress DSCP value for the flow direction. */
	uint8_t return_dscp;		/**< Egress DSCP value for the return direction. */
};

/**
 * ppe_drv_vlan_info
 *	Information for ingress and egress VLANs.
 */
struct ppe_drv_vlan_info {
	uint32_t ingress_vlan_tag;	/**< VLAN tag for the ingress packets. */
	uint32_t egress_vlan_tag;	/**< VLAN tag for egress packets. */
};

/**
 * ppe_drv_vlan_rule
 *	Information for VLAN connection rules.
 */
struct ppe_drv_vlan_rule {
	struct ppe_drv_vlan_info primary_vlan;		/* Primary VLAN info */
	struct ppe_drv_vlan_info secondary_vlan;	/* Secondary VLAN info */
};

/**
 * ppe_drv_qos_rule
 *	Information for QoS connection rules.
 */
struct ppe_drv_qos_rule {
	uint32_t flow_qos_tag;		/**< QoS tag associated with this rule for the flow direction. */
	uint32_t return_qos_tag;	/**< QoS tag associated with this rule for the return direction. */
	uint8_t flow_int_pri;		/**< PPE INT_PRI corresponding to flow_qos_tag when PPE Qdisc is configured. */
	uint8_t return_int_pri;		/**< PPE INT_PRI corresponding to return_qos_tag when PPE Qdisc is configured. */
	uint8_t qos_valid_flags;	/**< FLAGS to identify PPE QOS. */
	uint8_t reserved[1];		/**< Reserved; padding for alignment. */
};

/**
 * ppe_drv_top_if_rule
 *	Information for top interface in hierarchy.
 */
struct ppe_drv_top_if_rule {
	ppe_drv_iface_t rx_if;		/**< Top PPE interface for from direction */
	ppe_drv_iface_t tx_if;		/**< Top PPE interface for return direction */
};

/**
 * ppe_drv_service_class_rule
 *	Service class related information.
 */
struct ppe_drv_service_class_rule {
	uint32_t flow_mark;		/**< SAWF metadata information in flow direction. */
	uint32_t return_mark;		/**< SAWF metadata information in return direction. */
	uint8_t flow_service_class;	/**< Service class id in flow direction. */
	uint8_t return_service_class;	/**< Service class id in return direction. */
};

/**
 * ppe_drv_wifi_rule
 *	Wi-Fi metadata and DS related information.
 */
struct ppe_drv_wifi_mdata_rule {
	uint32_t flow_mark;		/**< Wi-Fi metadata information in flow direction. */
	uint32_t return_mark;		/**< Wi-Fi metadata information in return direction. */
	uint32_t flow_ds_node_mdata;	/**< DS metadata in flow direction. */
	uint32_t return_ds_node_mdata;	/**< DS metadata in return direction. */
};

/*
 * ppe_drv_nsm_queue_drop_stats
 *	Per-queue stats to be send to NSM.
 */
struct ppe_drv_nsm_queue_drop_stats {
	uint64_t drop_packets;		/**< Drop packets for given queue and item. */
	uint64_t drop_bytes;		/**< Drop bytes for given queue and item. */
};

/*
 * ppe_drv_nsm_flow_stats
 *	Per-flow stats to be send to NSM.
 */
struct ppe_drv_nsm_flow_stats {
	uint64_t rx_packets;		/**< Packets received on the ethernet port. */
	uint64_t rx_bytes;		/**< Bytes received on the ethernet port */
};

/*
 * ppe_drv_nsm_sawf_sc_stats
 *	Per-service class stats to be send to NSM.
 */
struct ppe_drv_nsm_sawf_sc_stats {
	uint64_t rx_packets;		/**< Packets recieved on the ethernet port. */
	uint64_t rx_bytes;		/**< Bytes recieved on the ethernet port. */
	uint16_t flow_count;		/**< Number of flows per service class. */
};

/*
 * ppe_drv_nsm_stats
 *	Information to be send to NSM.
 */
struct ppe_drv_nsm_stats {
	struct ppe_drv_nsm_sawf_sc_stats sawf_sc_stats;		/**< Per-service class stats. */
	struct ppe_drv_nsm_flow_stats flow_stats;		/**< Per-flow stats. */
	struct ppe_drv_nsm_queue_drop_stats queue_stats;	/**< Per-queue stats. */
};

/*
 * ppe_drv_acl_policer_rule_type
 *	Rule type to indicate flow+acl or flow+policer combination.
 */
typedef enum ppe_drv_acl_policer_rule_type {
	PPE_DRV_RULE_TYPE_FLOW_ACL,		/**< Flow needs to be combined with ACL. */
	PPE_DRV_RULE_TYPE_FLOW_POLICER,		/**< Flow needs to be combined with Policer. */
} ppe_drv_acl_policer_rule_t;

/*
 * ppe_drv_acl_policer_rule
 *	ACL/POLICER rule ID for each direction
 *
 * Note: This is used when the packets matching the flow need to be rate-limited (policed)
 * 	or filtered by an ACL rule, post flow processing.
 */
struct ppe_drv_acl_policer_rule {
	ppe_drv_acl_policer_rule_t type;	/**< Whether ACL or Policer rule? */
	bool pkt_noedit;			/**< Is packet edit required. */
	union {
		struct {
			uint8_t flags;			/**< Valid flag. */
			uint32_t flow_acl_id;		/**< ACL rule ID in flow direction. */
			uint32_t return_acl_id;		/**< ACL rule ID in return direction. */
		} acl;
		struct {
			uint8_t flags;			/**< Valid flag. */
			uint32_t flow_policer_id;	/**< Policer ID in flow direction. */
			uint32_t return_policer_id;	/**< Policer ID in return direction. */
		} policer;
	} rule_id;
};

/*
 * ppe_drv_stats_sync_reason
 *	Stats sync reasons.
 */
enum ppe_drv_stats_sync_reason {
	PPE_DRV_STATS_SYNC_REASON_STATS,	/* Sync is to synchronize stats */
	PPE_DRV_STATS_SYNC_REASON_FLUSH,	/* Sync is to flush a connection entry */
	PPE_DRV_STATS_SYNC_REASON_EVICT,	/* Sync is to evict a connection entry */
	PPE_DRV_STATS_SYNC_REASON_DESTROY,	/* Sync is to destroy a connection entry */
};

/*
 * ppe_drv_notifier_ops_priority
 *	Priority order for client manager
 *	To-do: Introduce new file for ppe-drv notifier
 */
enum ppe_drv_notifier_ops_priority {
	PPE_DRV_NOTIFIER_PRI_1 = 1,		/* Client manager notifier having priority one */
	PPE_DRV_NOTIFIER_PRI_2 = 2,		/* Client manager notifier having priority two*/
	PPE_DRV_NOTIFIER_MAX,		/* Notifier maximum priority */
};

/*
 * ppe_drv_notifier_event
 *	PPE drv notifier events.
 */
enum ppe_drv_notifier_event {
	PPE_DRV_EVENT_INALID,		/* Invalid event */
	PPE_DRV_EVENT_CHANGEUPPER,	/* Change upper event */
	PPE_DRV_EVENT_MAX,		/* Maximum event */
};

struct ppe_drv_notifier_ops;

/**
 * ppe_drv_notifier_fn_t
 *	Function pointer for notifier
 */
typedef int (*ppe_drv_notifier_fn_t)(struct ppe_drv_notifier_ops *nb,
				int event, struct netdev_notifier_info *info);
/**
 * ppe_drv_notifier_ops
 *	Notifier operations registered with different clients
 */
struct ppe_drv_notifier_ops {
	ppe_drv_notifier_fn_t notifier_call;		/* Notifier callback registered with ppe drv */
	int priority;					/* Priority of notifier callback */
	struct list_head entry;				/* List head */
};

/**
 * enum ppe_drv_ret
 *	PPE return status
 */
typedef enum ppe_drv_ret {
	PPE_DRV_RET_SUCCESS = 0,			/**< Success */
	PPE_DRV_POLICER_CREATE_FAIL,			/**< Create Fail */
	PPE_DRV_POLICER_DESTROY_FAIL,			/**< Destroy Fail */
	PPE_DRV_POLICER_DESTROY_SUCCESS,		/**< Destroy success */
	PPE_DRV_RET_IFACE_INVALID,			/**< Failure due to Invalid PPE interface */
	PPE_DRV_RET_FAILURE_NOT_SUPPORTED,		/**< Failure due to unsupported feature */
	PPE_DRV_RET_FAILURE_NO_RESOURCE,		/**< Failure due to out of resource */
	PPE_DRV_RET_FAILURE_INVALID_PARAM,		/**< Failure due to invalid parameter */
	PPE_DRV_RET_PORT_NOT_FOUND,			/**< Port not found */
	PPE_DRV_RET_VSI_NOT_FOUND,			/**< VSI not found */
	PPE_DRV_RET_L3_IF_NOT_FOUND,			/**< L3_IF not found */
	PPE_DRV_RET_PORT_ALLOC_FAIL,			/**< Port allocation fails */
	PPE_DRV_RET_L3_IF_ALLOC_FAIL,			/**< L3_IF allocation fails */
	PPE_DRV_RET_L3_IF_PORT_ATTACH_FAIL,		/**< L3_IF PORT attach fails */
	PPE_DRV_RET_L3_IF_TTL_DEC_BYPASS_FAIL,		/**< L3_IF TTL decrement bypass fails */
	PPE_DRV_RET_VSI_ALLOC_FAIL,			/**< VSI allocation fails */
	PPE_DRV_RET_MAC_ADDR_CLEAR_CFG_FAIL,		/**< Mac address clear configuration fails */
	PPE_DRV_RET_MAC_ADDR_SET_CFG_FAIL,		/**< Mac address set configuration fails */
	PPE_DRV_RET_MTU_CFG_FAIL,			/**< MTU configuration fails */
	PPE_DRV_RET_DEL_MAC_FDB_FAIL,			/**< Failed to delete FDB entry by MAC */
	PPE_DRV_RET_STA_MOVE_FAIL,			/**< Failed to configure station movement */
	PPE_DRV_RET_NEW_ADDR_LRN_FAIL,			/**< Failed to configure new address learning */
	PPE_DRV_RET_FDB_FLUSH_VSI_FAIL,			/**< Failed to flush FDB entries by VSI */
	PPE_DRV_RET_MEM_IF_INVALID_PORT,		/**< Failed to find port for member interface */
	PPE_DRV_RET_STP_STATE_FAIL,			/**< Failed to set STP state on the bridge port */
	PPE_DRV_RET_IFACE_L3_IF_FAIL,			/**< Failed to find L3_IF for the interface */
	PPE_DRV_RET_PPPOE_ALLOC_FAIL,			/**< PPPOE session allocation failure */
	PPE_DRV_RET_L3_IF_PPPOE_FAIL,			/**< PPPOE session not attached to L3_IF */
	PPE_DRV_RET_L3_IF_PPPOE_SET_FAIL,		/**< Failed to set PPPOE session information in L3_IF */
	PPE_DRV_RET_BASE_IFACE_NOT_FOUND,		/**< Base interface not found */
	PPE_DRV_RET_VLAN_TPID_FAIL,			/**< VLAN TPID not found */
	PPE_DRV_RET_PORT_ROLE_FAIL,			/**< Port role configuration failed */
	PPE_DRV_RET_INGRESS_VLAN_FAIL,			/**< Ingress vlan configuration failed */
	PPE_DRV_RET_EGRESS_VLAN_FAIL,			/**< Egress vlan configuration failed */
	PPE_DRV_RET_VLAN_INGRESS_DEL_FAIL,		/**< Ingress vlan deletion configuration failed */
	PPE_DRV_RET_VLAN_EGRESS_DEL_FAIL,		/**< Egress vlan deletion configuration failed */
	PPE_DRV_RET_FAILURE_INVALID_HIERARCHY,		/**< Failure due to invalid hierarchy */
	PPE_DRV_RET_FAILURE_SNAT_DNAT_SIMUL,		/**< Failure due to both snat and dnat requested */
	PPE_DRV_RET_FAILURE_NOT_BRIDGE_SLAVES,		/**< Failure due to from and to interfaces not in same bridge */
	PPE_DRV_RET_FAILURE_IFACE_PORT_MAP,
	PPE_DRV_RET_FAILURE_CREATE_COLLISSION,		/**< Failure due to create collision */
	PPE_DRV_RET_FAILURE_CREATE_OOM,			/**< Failure due to memory allocation failed */
	PPE_DRV_RET_FAILURE_FLOW_ADD_FAIL,		/**< Failure due to flow addition failed in hardware */
	PPE_DRV_RET_FAILURE_DESTROY_NO_CONN,		/**< Failure due to connection not found in hardware */
	PPE_DRV_RET_FAILURE_NO_MATCHING_CONN,		/**< Failure due to connection not found in hardware */
	PPE_DRV_RET_FAILURE_DESTROY_FAIL,		/**< Failure due to connection not found in hardware */
	PPE_DRV_RET_FAILURE_FLUSH_FAIL,			/**< Flush failure */
	PPE_DRV_RET_FAILURE_BRIDGE_NAT,			/**< Failure due to Bridge + NAT flows */
	PPE_DRV_RET_QUEUE_CFG_FAIL,			/**< Failure in queue configuration */
	PPE_DRV_RET_FAILURE_TUN_CE_ADD_FAILURE,		/**< Failure in adding tunnel connection entry */
	PPE_DRV_RET_FAILURE_TUN_CE_DEL_FAILURE,		/**< Failure in removing tunnel connection entry */
	PPE_DRV_RET_INVALID_VP_NUM,			/**< Invalid VP number */
	PPE_DRV_RET_TUN_ADD_CE_NULL,			/**< Add connection entry callback is NULL */
	PPE_DRV_RET_INVALID_EIP_SERVICE,		/**< Invalid inline EIP service */
	PPE_DRV_RET_FAILURE_DUMMY_RULE,			/**< Failed to push rule to PPE for passive VP */
	PPE_DRV_RET_INVALID_USER_TYPE,			/**< Failed to push rule to PPE for DS flow when user type is not DS */
	PPE_DRV_RET_ADD_FDB_FAIL,			/**< Failed to add new fdb entry */
	PPE_DRV_RET_SET_FDB_AGEING_TIME_FAIL,		/**< Failed to set Ageing time */
	PPE_DRV_RET_SET_MIRROR_FAIL,			/**<Failed to set Mirror interface */
	PPE_DRV_RET_SET_MIRROR_IN_FAIL,			/**< Failed to set Mirror ingress */
	PPE_DRV_RET_SET_MIRROR_EG_FAIL,			/**< Failed to set Mirror egress */
	PPE_DRV_RET_FLUSH_FDB_BY_PORT_FAIL,		/**< Failed to flush FDB by port */
	PPE_DRV_RET_FLUSH_FDB_FAIL,			/**< Failed to flush all FDB */
	PPE_DRV_RET_SET_MIRROR_ANALYSIS_FAIL,		/**< Failed to set Mirror analysis port */
	PPE_DRV_RET_GET_MIRROR_ANALYSIS_FAIL,		/**< Failed to get Mirror analysis port */
	PPE_DRV_RET_GET_MIRROR_ANALYSIS_NO_PORT,	/**< No port is set for mirror analysis */
	PPE_DRV_RET_QOS_QUEUE_CFG_FAIL,			/**< QoS queue configuration failed. */
	PPE_DRV_RET_QOS_SCHEDULER_CFG_FAIL,		/**< QoS scheduler configuration failed. */
	PPE_DRV_RET_QOS_SHAPER_CFG_FAIL,		/**< QoS shaper configuration failed. */
	PPE_DRV_RET_QOS_PORT_CFG_FAIL,			/**< QoS port configuration failed. */
	PPE_DRV_RET_PORT_NO_OFFLOAD,			/**< Offload is disabled on the PPE port */
	PPE_DRV_RET_ACL_RULE_INVALID,			/**< ACL rule invalid */
	PPE_DRV_RET_ACL_RULE_ADD_FAIL,			/**< Failed to add ACL rule */
	PPE_DRV_RET_ACL_RULE_BIND_FAIL,			/**< Failed to bind ACL rule to src */
	PPE_DRV_RET_POLICER_RULE_BIND_FAIL,		/**< Failed to bind Policer rule */
	PPE_DRV_RET_BASE_DEV_NOT_FOUND,			/**< Base Device not found */
	PPE_DRV_RET_BASE_PORT_NOT_FOUND,		/**< Base Port not found */
	PPE_DRV_RET_FAILURE_FLOW_CONFIGURE_FAIL,	/**< Failure due to FSE flow configuration failed */
	PPE_DRV_RET_NO_TOP_RX_IF,			/**< Failure due to no corresponding top interface */
	PPE_DRV_RET_INVALID_DEV_TYPE,			/**< Invalid port netdev type */
} ppe_drv_ret_t;

/**
 * ppe_drv_get_dentry
 *	Get PPE debugfs entry.
 *
 * @return
 * ppe dentry.
 */
struct dentry *ppe_drv_get_dentry(void);

/**
 * ppe_drv_queue_from_core
 *	Provide queue for a specific core.
 *
 * @param[in] core core_id.
 *
 * @return
 * queue-ID for a specific core or -1 for incorrect core ID.
 */
int16_t ppe_drv_queue_from_core(uint8_t core);

/**
 * ppe_drv_core2queue_mapping
 *	Provide core to queue mapping.
 *
 * @param[in] core core_id.
 * @param[in] queue_id  queue_id.
 *
 * @return
 * none.
 */
void ppe_drv_core2queue_mapping(uint8_t core, uint8_t queue_id);

/*
 * ppe_drv_nsm_queue_stats_update()
 *	Update stats in NSM for given queue id.
 *
 * @param[IN] nsm_stats		Pointer to stats structure in NSM.
 * @param[IN] queue_id		Queue ID corresponding to which stats are needed.
 * @param[IN] item_id		Drop Item ID for the Queue.
 *
 * @return
 * Status of the API.
 */
extern bool ppe_drv_nsm_queue_stats_update(struct ppe_drv_nsm_stats *nsm_stats, uint32_t queue_id, uint8_t item_id);

/**
 * ppe_drv_fse_ops_unregister
 *	Unregister fse ops with ppe driver.
 *
 * @return
 * none.
 */
void ppe_drv_fse_ops_unregister(void);

/**
 * ppe_drv_fse_ops_register
 *	Register fse ops with ppe driver.
 *
 * @param[IN] ops Pointer to FSE operation structure in PPE.
 *
 * @return
 * true or false.
 */
bool ppe_drv_fse_ops_register(struct ppe_drv_fse_ops *ops);

/**
 * ppe_drv_fse_feature_enable
 *	Enable PPE-FSE feature.
 *
 * @return
 * none.
 */
void ppe_drv_fse_feature_enable(void);

/**
 * ppe_drv_fse_feature_disable
 *	Disable PPE-FSE feature.
 *
 * @return
 * none.
 */
void ppe_drv_fse_feature_disable(void);

/**
 * ppe_drv_nsm_sawf_sc_stats_read()
 *	Update stats in NSM for given service class
 *
 * @param[IN] nsm_stats		Pointer to stats structure in NSM.
 * @param[IN] service_class	Service class corresponding to which stats are needed.
 *
 * @return
 * Status of the API.
 */
extern bool ppe_drv_nsm_sawf_sc_stats_read(struct ppe_drv_nsm_stats *nsm_stats, uint8_t service_class);

/**
 * ppe_drv_notifier_register
 *	Register notifier ops with ppe driver.
 *
 * @param[IN] ops pointer to notifier structure in PPE.
 *
 * @return
 * none.
 */
extern void ppe_drv_notifier_ops_register(struct ppe_drv_notifier_ops *notifier_ops);

/**
 * ppe_drv_notifier_unregister
 *	Register notfier ops with ppe driver.
 *
 * @param[IN] ops pointer to notifier structure in PPE.
 *
 * @return
 * none.
 */
extern void ppe_drv_notifier_ops_unregister(struct ppe_drv_notifier_ops *notifier_ops);

/**
 * ppe_drv_is_mht_dev
 *	Check the ppe iface MHT switch port flag.
 *
 * @datatypes
 * net_device
 *
 * @param[in] dev	netdevice pointer.
 *
 * @return
 * true or false
 */
bool ppe_drv_is_mht_dev(struct net_device *dev);
/*
 * ppe_drv_mht_port_from_fdb()
 *	Get the port id corresponding to the destination
 *	mac address and vid
 */
int32_t ppe_drv_mht_port_from_fdb(uint8_t *dmac, uint16_t vid);

/**
 * ppe_drv_ds_map_node_to_queue
 *	Provides node to queue mapping.
 *
 * @param[in] node_id	node id.
 * @param[in] queue_id	queue_id.
 *
 * @return
 * none.
 */
void ppe_drv_ds_map_node_to_queue(uint8_t node_id, uint8_t queue_id);
#endif /* _PPE_DRV_H_ */
