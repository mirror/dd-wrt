/*
 **************************************************************************
 * Copyright (c) 2020, The Linux Foundation.  All rights reserved.
 * Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
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
 * This structure collects 5-tuple information to send it to
 * WLAN driver via the registered FSE (Flow Search Engine) callback.
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
};

/**
 * Mesh latency configuration update callback function to which MSCS client will register.
 */
typedef int (*ecm_classifier_emesh_callback_t)(uint8_t dest_mac[],
		uint32_t service_interval_dl, uint32_t burst_size_dl,
		uint32_t service_interval_ul, uint32_t burst_size_ul,
		uint16_t priority, uint8_t add_or_sub);

/**
 * MSDUQ callback to which emesh-sawf will register.
 */
typedef uint16_t (*ecm_classifier_emesh_msduq_callback_t)(struct net_device *out_dev,
		uint8_t dest_mac[], uint32_t service_class_id);

/**
 * SAWF UL configuration update callback function pointer.
 */
typedef void (*ecm_classifier_emesh_sawf_config_ul_callback_t)(uint8_t dest_mac[],
		uint8_t src_mac[], uint8_t fwd_service_id, uint8_t rev_service_id, uint8_t add_or_sub);

/**
 * FSE flow update callback to which emesh-sawf will register.
 */
typedef void (*ecm_classifier_emesh_fse_flow_callback_t)(void *appdata, ecm_classifier_fse_connection_state_t state);

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
	ecm_classifier_emesh_sawf_config_ul_callback_t update_sawf_ul;
						/**< Update SAWF uplink parameters. */
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
 * Registers EMESH-SAWF config uplink callback.
 *
 * @param	mesh_cb	EMESH-SAWF callback pointer.
 *
 * @return
 * The status of the callback registration operation.
 */
int ecm_classifier_emesh_sawf_config_ul_callback_register(struct ecm_classifier_emesh_sawf_callbacks *mesh_cb);

/**
 * Unregisters EMESH-SAWF config uplink callback.
 *
 * @return
 * None.
 */
void ecm_classifier_emesh_sawf_config_ul_callback_unregister(void);

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
