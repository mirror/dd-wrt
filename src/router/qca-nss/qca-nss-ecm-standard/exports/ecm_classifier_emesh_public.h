/*
 **************************************************************************
 * Copyright (c) 2020, The Linux Foundation.  All rights reserved.
 * Copyright (c) 2022-2023, Qualcomm Innovation Center, Inc. All rights reserved.
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
 **************************************************************************
 */

/**
 * @file ecm_classifier_emesh_public.h
 *	ECM E-mesh classifier subsystem.
 */

#ifndef __ECM_CLASSIFIER_EMESH_PUBLIC_H__
#define __ECM_CLASSIFIER_EMESH_PUBLIC_H__

/**
 * @addtogroup ecm_classifier_emesh_subsystem
 * @{
 */

#define ECM_CLASSIFIER_EMESH_SAWF_SVID_VALID		0x1
#define ECM_CLASSIFIER_EMESH_SAWF_DSCP_VALID		0x2
#define ECM_CLASSIFIER_EMESH_SAWF_VLAN_PCP_VALID	0x4
#define ECM_CLASSIFIER_EMESH_SAWF_DSCPCTE_VALID		0x8
#define ECM_CLASSIFIER_EMESH_SAWF_INVALID_MSDUQ         0xffffffff
#define ECM_CLASSIFIER_EMESH_SAWF_INVALID_SVID		0xffffffff

/**
 * State of the connection while informing 5-tuple
 * information to FSE (Flow search engine) via register callback.
 */
enum ecm_classifier_fse_connection_state {
	ECM_CLASSIFIER_SAWF_FSE_CONNECTION_STATE_ACCEL,		/**< FSE callback at the time of connection acceleration. */
	ECM_CLASSIFIER_SAWF_FSE_CONNECTION_STATE_DECEL,		/**< FSE callback at the time of connection deceleration. */
	ECM_CLASSIFIER_SAWF_FSE_CONNECTION_STATE_MAX		/**< Indicates the last item. */
};
typedef enum ecm_classifier_fse_connection_state ecm_classifier_fse_connection_state_t;

/**
 * This structure collects 5-tuple information along with SAWF metadata and
 * informs WLAN driver to update FSE(Flow Search Engine) entry via registered callback.
 */
struct ecm_classifier_fse_info {
	union {
		__be32 v4_addr;			/**< Source IPv4 address. */
		struct in6_addr v6_addr;	/**< Source IPv6 address. */
	} src;
	union {
		__be32 v4_addr;			/**< Destination IPv4 address. */
		struct in6_addr v6_addr;	/**< Destination IPv6 address. */
	} dest;
	uint16_t ip_version;			/**< IP version. */
	uint16_t src_port;			/**< Source port. */
	uint16_t dest_port;			/**< Destination port. */
	uint16_t protocol;			/**< Protocol number. */
	uint8_t src_mac[ETH_ALEN];			/**< Source MAC. */
	uint8_t dest_mac[ETH_ALEN];			/**< Destination MAC. */
	uint32_t fw_svc_info;			/**< Forward SAWF info. */
	uint32_t rv_svc_info;			/**< Reverse SAWF info. */
	struct net_device *src_dev;		/**< Source dev. */
	struct net_device *dest_dev;		/**< Destination dev. */
};

/**
 * ecm_classifier_emesh_sawf_flow_info
 *
 * @netdev : Netdevice
 * @peer_mac : Destination peer mac address
 * @service_id : Service class id
 * @dscp : Differentiated Services Code Point
 * @rule_id : Rule id
 * @sawf_rule_type: Rule type
 */
struct ecm_classifier_emesh_sawf_flow_info {
	struct net_device *netdev;
	uint8_t *peer_mac;
	uint32_t service_id;
	uint32_t dscp;
	uint32_t vlan_pcp;
	uint32_t valid_flag;
	uint32_t rule_id;
	uint8_t sawf_rule_type;
};

/**
 * Structure collecting mesh latency params to send it to wlan driver
 * via registered callback.
 */
struct ecm_classifer_emesh_sawf_mesh_latency_params {
	struct net_device *dst_dev;		/**< Destination net dev. */
	struct net_device *src_dev;		/**< Source net dev. */
	uint8_t *peer_mac;			/**< Peer MAC. */
	uint32_t service_interval_dl;		/**< Service interval DL. */
	uint32_t burst_size_dl;			/**< Burst size DL. */
	uint32_t service_interval_ul;		/**< Service interval UL. */
	uint32_t burst_size_ul;			/**< Burst size UL. */
	uint16_t priority;			/**< Priority. */
	uint8_t accel_or_decel;			/**< Time of the callback. */
};

/**
 * Structure collecting sawf param to send to wlan driver
 * via registered callback at connection sync.
 */
struct ecm_classifer_emesh_sawf_sync_params {
	struct net_device *dest_dev;		/**< Destination net dev. */
	struct net_device *src_dev;		/**< Source net dev. */
	uint8_t dest_mac[ETH_ALEN];		/**< Destination MAC. */
	uint8_t src_mac[ETH_ALEN];		/**< Source MAC. */
	uint8_t fwd_service_id;			/**< Forward Service class ID. */
	uint8_t rev_service_id;			/**< Reverse Service class ID. */
	uint32_t fwd_mark_metadata;		/**< Forward mark metadata. */
	uint32_t rev_mark_metadata;		/**< Reverse mark metadata. */
	uint8_t add_or_sub;			/**< Add or Subtract a Flow */
};

/**
 * Mesh latency configuration update callback function to which MSCS client will register.
 */
typedef void (*ecm_classifier_emesh_callback_t)(struct ecm_classifer_emesh_sawf_mesh_latency_params *mesh_params);

/**
 * MSDUQ callback to which emesh-sawf will register.
 */
typedef uint32_t (*ecm_classifier_emesh_msduq_callback_t)(struct ecm_classifier_emesh_sawf_flow_info *sawf_flow_info);

/**
 * SAWF params sync callback function pointer.
 */
typedef void (*ecm_classifier_emesh_sawf_conn_params_sync_callback_t)(struct ecm_classifer_emesh_sawf_sync_params *sawf_sync_params);

/**
 * FSE flow update callback to which emesh-sawf will register.
 */
typedef bool (*ecm_classifier_emesh_fse_flow_callback_t)(struct ecm_classifier_fse_info *fse_info);

/**
 * Data structure for easy mesh-sawf classifier callbacks.
 */
struct ecm_classifier_emesh_sawf_callbacks {
	ecm_classifier_emesh_callback_t update_peer_mesh_latency_params;
						/**< Parameters for peer mesh latency. */
	ecm_classifier_emesh_msduq_callback_t update_service_id_get_msduq;
						/**< Get msduq for SAWF classifier. */
	ecm_classifier_emesh_fse_flow_callback_t update_fse_flow_info;
						/**< Update fse flow callback. */
	ecm_classifier_emesh_sawf_conn_params_sync_callback_t sawf_conn_sync;
						/**< Sync SAWF parameters. */
};

/**
 * Registers a client for EMESH-SAWF callbacks.
 *
 * @param	mesh_cb	EMESH-SAWF callback pointer.
 *
 * @return
 * The status of the callback registration operation.
 */
int ecm_classifier_emesh_latency_config_callback_register(struct ecm_classifier_emesh_sawf_callbacks *mesh_cb);

/**
 * Unregisters a client from EMESH-SAWF callbacks.
 *
 * @return
 * None.
 */
void ecm_classifier_emesh_latency_config_callback_unregister(void);

/**
 * Registers msduq EMESH-SAWF callback.
 *
 * @param	mesh_cb	EMESH-SAWF callback pointer.
 *
 * @return
 * The status of the callback registration operation.
 */
int ecm_classifier_emesh_sawf_msduq_callback_register(struct ecm_classifier_emesh_sawf_callbacks *mesh_cb);

/**
 * Unregisters msduq EMESH-SAWF callback.
 *
 * @return
 * None.
 */
void ecm_classifier_emesh_sawf_msduq_callback_unregister(void);

/**
 * Registers EMESH-SAWF connection sync callback.
 *
 * @param	mesh_cb	EMESH-SAWF callback pointer.
 *
 * @return
 * The status of the callback registration operation.
 */
int ecm_classifier_emesh_sawf_conn_sync_callback_register(struct ecm_classifier_emesh_sawf_callbacks *mesh_cb);

/**
 * Unregisters EMESH-SAWF connection sync callback.
 *
 * @return
 * None.
 */
void ecm_classifier_emesh_sawf_conn_sync_callback_unregister(void);

/**
 * Registers EMESH-SAWF fse flow update callback.
 *
 * @param	mesh_cb	EMESH-SAWF callback pointer.
 *
 * @return
 * The status of the callback registration operation.
 */
int ecm_classifier_emesh_sawf_update_fse_flow_callback_register(struct ecm_classifier_emesh_sawf_callbacks *mesh_cb);

/**
 * Unregisters EMESH-SAWF fse flow update callback.
 *
 * @return
 * None.
 */
void ecm_classifier_emesh_sawf_update_fse_flow_callback_unregister(void);

/**
 * @}
 */

#endif /* __ECM_CLASSIFIER_EMESH_PUBLIC_H__ */
