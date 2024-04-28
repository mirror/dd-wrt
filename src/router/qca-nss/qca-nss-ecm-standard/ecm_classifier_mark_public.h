/*
 **************************************************************************
 * Copyright (c) 2018, The Linux Foundation.  All rights reserved.
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

/*
 * Mark types are used to register specific external mdoule callbacks to ECM.
 * Each type of callback can process the packets based on its need.
 */
enum ecm_classifier_mark_types {
	ECM_CLASSIFIER_MARK_TYPE_L2_ENCAP,
	ECM_CLASSIFIER_MARK_TYPE_MAX
};
typedef enum ecm_classifier_mark_types ecm_classifier_mark_type_t;

/*
 * Result values of the external inspection module to the ECM's mark classifier.
 * Based on the result the mark classifier takes the action for the inspected connection.
 */
enum ecm_classifier_mark_results {
	ECM_CLASSIFIER_MARK_RESULT_NOT_YET,		/* Classifier has not decided yet - try again later */
	ECM_CLASSIFIER_MARK_RESULT_NOT_RELEVANT,	/* Classifier is not relevant to this connection */
	ECM_CLASSIFIER_MARK_RESULT_SUCCESS,		/* Inspection is completed successfully */
};
typedef enum ecm_classifier_mark_results ecm_classifier_mark_result_t;

/*
 * Callback function which extracts a mark value from the skb and write it to the mark parameter.
 */
typedef ecm_classifier_mark_result_t (*ecm_classifier_mark_get_callback_t)(struct sk_buff *skb, uint32_t *mark);

/*
 * Callback functions which are called from the mark classifier's IPv4 and IPv6 sync_to
 * functions to update the external modules.
 */
typedef void (*ecm_classifier_mark_sync_to_ipv4_callback_t)(uint32_t mark,
							    __be32 src_ip, int src_port,
							    __be32 dest_ip, int dest_port,
							    int protocol);
typedef void (*ecm_classifier_mark_sync_to_ipv6_callback_t)(uint32_t mark,
							    struct in6_addr *src_ip, int src_port,
							    struct in6_addr *dest_ip, int dest_port,
							    int protocol);

/*
 * Register/Unregister callback functions which are called from the external modules.
 */
extern int ecm_classifier_mark_register_callbacks(ecm_classifier_mark_type_t type, ecm_classifier_mark_get_callback_t mark_get,
						  ecm_classifier_mark_sync_to_ipv4_callback_t sync_to_ipv4,
						  ecm_classifier_mark_sync_to_ipv6_callback_t sync_to_ipv6);
extern void ecm_classifier_mark_unregister_callbacks(ecm_classifier_mark_type_t type);

