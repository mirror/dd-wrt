/*
 **************************************************************************
 * Copyright (c) 2019-2020, The Linux Foundation.  All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

/**
 * @file ecm_classifier_ovs_public.h
 *	ECM OVM classifier.
 */

/**
 * @addtogroup ecm_classifier_ovs_subsystem
 * @{
 */

/**
 * Result values of the external inspection module to the ECM's OVS classifier.
 * Based on the result the OVS classifier takes the action for the inspected connection.
 */
enum ecm_classifier_ovs_results {
	ECM_CLASSIFIER_OVS_RESULT_ALLOW_VLAN_ACCEL,		/**< VLAN process succeeded. */
	ECM_CLASSIFIER_OVS_RESULT_ALLOW_VLAN_QINQ_ACCEL,	/**< VLAN process succeeded for QinQ. */
	ECM_CLASSIFIER_OVS_RESULT_ALLOW_ACCEL,			/**< No VLAN present, just accelerate. */
	ECM_CLASSIFIER_OVS_RESULT_DENY_ACCEL_EGRESS,		/**< Flow egress is not allowed for acceleration. */
	ECM_CLASSIFIER_OVS_RESULT_DENY_ACCEL			/**< Do not accelerate. */
};

typedef enum /** @cond */ ecm_classifier_ovs_results /** @endcond */ ecm_classifier_ovs_result_t;

/**
 * OVS classifier process response.
 */
struct ecm_classifier_ovs_process_response {
	uint32_t dscp;				/**< Used by the routed connections. */
	uint32_t flow_dscp;			/**< Bridge connection's flow DSCP value. */
	uint32_t return_dscp;			/**< Bridge connection's return DSCP value. */
	struct vlan_hdr ingress_vlan[2];	/**< Ingress VLAN header. */
	struct vlan_hdr egress_vlan[2];		/**< Egress VLAN header. */
};


/**
 * Callback function that processes the connection information.
 */
typedef ecm_classifier_ovs_result_t (*ecm_classifier_ovs_process_callback_t)(struct ovsmgr_dp_flow *flow,
									struct sk_buff *skb,
									struct ecm_classifier_ovs_process_response *resp);

/**
 * OVS callbacks.
 */
struct ecm_classifier_ovs_callbacks {
	ecm_classifier_ovs_process_callback_t ovs_process;	/**< OVS process callback. */
};

/**
 * Registers callback functions called from external modules.
 *
 * @param	callbacks	The callbacks pointer.
 *
 * @return
 * The status of the callback registration operation.
 */
int ecm_classifier_ovs_register_callbacks(struct ecm_classifier_ovs_callbacks *callbacks);

/**
 * Unregisters callback functions called from external modules.
 *
 * @return
 * None.
 */
void ecm_classifier_ovs_unregister_callbacks(void);

/**
 * @}
 */
