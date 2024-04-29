/*
 **************************************************************************
 * Copyright (c) 2018, The Linux Foundation.  All rights reserved.
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
 * @file ecm_classifier_mark_public.h
 *	ECM acceleration engine (AE) classifier.
 */

/**
 * @addtogroup ecm_classifier_mark_subsystem
 * @{
 */

/**
 * Mark types used to register specific external module callbacks to the ECM.
 * Each type of callback can process the packets based on its need.
 */
enum ecm_classifier_mark_types {
	ECM_CLASSIFIER_MARK_TYPE_L2_ENCAP,	/**< L2 encapsulation mark type. */
	ECM_CLASSIFIER_MARK_TYPE_MAX		/**< Indicates the last item. */
};

typedef enum /** @cond */ ecm_classifier_mark_types /** @endcond */ ecm_classifier_mark_type_t;

/**
 * Result values of the external inspection module to the ECM's mark classifier.
 * Based on the result the mark classifier takes the action for the inspected connection.
 */
enum ecm_classifier_mark_results {
	ECM_CLASSIFIER_MARK_RESULT_NOT_YET,		/**< Classifier has not decided yet; try again later. */
	ECM_CLASSIFIER_MARK_RESULT_NOT_RELEVANT,	/**< Classifier is not relevant to this connection. */
	ECM_CLASSIFIER_MARK_RESULT_SUCCESS,		/**< Inspection completed successfully. */
};

typedef enum /** @cond */ ecm_classifier_mark_results /** @endcond */ ecm_classifier_mark_result_t;

/**
 * Callback function which extracts a mark value from the SKB and writes it to the mark parameter.
 */
typedef ecm_classifier_mark_result_t (*ecm_classifier_mark_get_callback_t)(struct sk_buff *skb, uint32_t *mark);

/**
 * Callback function called from the mark classifier's IPv4 sync_to
 * function to update external modules.
 */
typedef void (*ecm_classifier_mark_sync_to_ipv4_callback_t)(uint32_t mark,
							__be32 src_ip, int src_port,
							__be32 dest_ip, int dest_port,
							int protocol);

/**
 * Callback function called from the mark classifier's IPv6 sync_to
 * function to update external modules.
 */
typedef void (*ecm_classifier_mark_sync_to_ipv6_callback_t)(uint32_t mark,
							struct in6_addr *src_ip, int src_port,
							struct in6_addr *dest_ip, int dest_port,
							int protocol);

/**
 * Registers for callback functions called from the external modules.
 *
 * @param	type		The classifier mark type.
 * @param	mark_get	The classifier mark get callback pointer.
 * @param	sync_to_ipv4	The classifier IPv4 synchronization callback pointer.
 * @param	sync_to_ipv6	The classifier IPv6 synchronization callback pointer.
 *
 * @return
 * The status of the callback registration operation.
 */
extern int ecm_classifier_mark_register_callbacks(ecm_classifier_mark_type_t type, ecm_classifier_mark_get_callback_t mark_get,
						ecm_classifier_mark_sync_to_ipv4_callback_t sync_to_ipv4,
						ecm_classifier_mark_sync_to_ipv6_callback_t sync_to_ipv6);

/**
 * Unregisters callback functions called from the external modules.
 *
 * @param	type	The classifier mark type.
 *
 * @return
 * None.
 */
extern void ecm_classifier_mark_unregister_callbacks(ecm_classifier_mark_type_t type);

/**
 * @}
 */
