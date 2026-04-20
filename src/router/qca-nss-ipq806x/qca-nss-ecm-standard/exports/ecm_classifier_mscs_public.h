/*
 **************************************************************************
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
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
 * @file ecm_classifier_mscs_public.h
 *	ECM mobile switching central server classifer subsystem.
 */

#ifndef __ECM_CLASSIFIER_MSCS_PUBLIC_H__
#define __ECM_CLASSIFIER_MSCS_PUBLIC_H__

/**
 * @addtogroup ecm_classifier_mscs_subsystem
 * @{
 */

#define ECM_CLASSIFIER_MSCS_INVALID_QOS_TAG 0xF	/**< MSCS invalid QoS tag value. */

/**
 * MSCS priority update result based on Wi-Fi MSCS peer lookup.
 */
enum ecm_classifier_mscs_results {
	ECM_CLASSIFIER_MSCS_RESULT_UPDATE_PRIORITY,	/**< MSCS priority update allowed. */
	ECM_CLASSIFIER_MSCS_RESULT_UPDATE_INVALID_TAG,	/**< MSCS priority update invalid. */
	ECM_CLASSIFIER_MSCS_RESULT_DENY_PRIORITY,	/**< MSCS priority not allowed for flow and return direction. */
};

typedef enum /** @cond */ ecm_classifier_mscs_results /** @endcond */ ecm_classifier_mscs_result_t;

/**
 * Structure containing params to get MSCS priority.
 */
struct ecm_classifier_mscs_get_priority_info {
	struct net_device *dst_dev;	/**< Destination net dev. */
	struct net_device *src_dev;	/**< Source net dev. */
	uint8_t *src_mac;		/**< Source MAC address. */
	uint8_t *dst_mac;		/**< Destination MAC address. */
	struct sk_buff *skb;		/**< SKB pointer. */
};

/**
 * Structure containing params for SCS rule match info.
 */
struct ecm_classifier_mscs_rule_match_info {
	struct net_device *dst_dev;	/**< Destination net dev. */
	struct net_device *src_dev;	/**< Source net dev. */
	uint8_t *src_mac;		/**< Source MAC. */
	uint8_t *dst_mac;		/**< Destination MAC. */
	uint32_t rule_id;		/**< Rule ID. */
};

/**
 * Callback to which MSCS clients will register.
 */
typedef int (*ecm_classifier_mscs_process_callback_t)(struct ecm_classifier_mscs_get_priority_info *priority_info);
typedef bool (*ecm_classifier_mscs_scs_priority_callback_t)(struct ecm_classifier_mscs_rule_match_info *rule_match_info);

/**
 * Data structure for MSCS classifier callbacks.
 */
struct ecm_classifier_mscs_callbacks {
	ecm_classifier_mscs_process_callback_t get_peer_priority;	/**< Callback to get the peer priority. */
	ecm_classifier_mscs_scs_priority_callback_t update_skb_priority;/**< Callback to update the skb priority. */
};

/**
 * Registers a client for MSCS callbacks.
 *
 * @param mscs_cb MSCS context pointer.
 *
 * @return
 * The status of the callback registration operation.
 */
int ecm_classifier_mscs_callback_register(struct ecm_classifier_mscs_callbacks *mscs_cb);

/**
 * Unregisters a client from MSCS callbacks.
 *
 * @return
 * None.
 */
void ecm_classifier_mscs_callback_unregister(void);

/**
 * @}
 */

#endif /* __ECM_CLASSIFIER_MSCS_PUBLIC_H__ */
