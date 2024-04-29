/*
 **************************************************************************
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
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
 * @file ecm_ae_classifier_public.h
 *	ECM acceleration engine (AE) classifier.
 */

#ifndef __ECM_AE_CLASSIFIER_PUBLIC_H__
#define __ECM_AE_CLASSIFIER_PUBLIC_H__

/**
 * @addtogroup ecm_ae_classifier_subsystem
 * @{
 */

#define ECM_AE_CLASSIFIER_FLOW_ROUTED		(1 << 0)	/**< Flow is routed. */
#define ECM_AE_CLASSIFIER_FLOW_MULTICAST	(1 << 1)	/**< Flow is multicast. */

/**
 * @brief External AE classifier flags.
 *
 */
#define ECM_AE_CLASSIFIER_FLAG_EXTERNAL_AE_REGISTERED	(1 << 0)	/**< External AE is registered flag. */
#define ECM_AE_CLASSIFIER_FLAG_FALLBACK_ENABLE		(1 << 1)	/**< AE fallback enable flag. */

/**
 *	External AE classifier returns these types to ECM per flow.
 */
typedef enum /** @cond */ ecm_ae_classifier_result /** @endcond */ {
	ECM_AE_CLASSIFIER_RESULT_NSS,		/**< Accelerate the flow in NSS */
	ECM_AE_CLASSIFIER_RESULT_PPE,		/**< Accelerate the flow in PPE */
	ECM_AE_CLASSIFIER_RESULT_SFE,		/**< Accelerate the flow in SFE */
	ECM_AE_CLASSIFIER_RESULT_PPE_VP,	/**< Accelerate the WIFI-flow in PPE with VP path */
	ECM_AE_CLASSIFIER_RESULT_PPE_DS,	/**< Accelerate the WIFI-flow in PPE with DS path */
	ECM_AE_CLASSIFIER_RESULT_NONE,		/**< Do not accelerate the flow */
	ECM_AE_CLASSIFIER_RESULT_NOT_YET,	/**< Acceleration engine hasn't been decided yet */
	ECM_AE_CLASSIFIER_RESULT_DONT_CARE,	/**< External module doesn't care about the selected AE */
} ecm_ae_classifier_result_t;

/**
 * Data structure which is filled and passed to external module
 * for the acceleration engine decision.
 */
struct ecm_ae_classifier_info {
	union {
		__be32 v4_addr;			/**< IPv4 address in host order. */
		struct in6_addr v6_addr;	/**< IPv6 address in host order. */
	} src;					/**< Source address. */
	union {
		__be32 v4_addr;			/**< IPv4 address in host order. */
		struct in6_addr v6_addr;	/**< IPv6 address in host order. */
	} dest;					/**< Destination address. */
	uint16_t dst_port;			/**< Destination port in host order. */
	uint16_t src_port;			/**< Source port port in host order. */
	uint8_t protocol;			/**< Next protocol header number. */
	uint8_t ip_ver;				/**< IP version 4 or 6. */
	uint16_t flag;				/**< Flow type flag. */
};

/**
* ECM AE classifier results.
*/
typedef ecm_ae_classifier_result_t (*ecm_ae_classifier_get_t)(struct ecm_ae_classifier_info *info);

/**
 * Data structure for acceleration engine classifier operations.
 */
struct ecm_ae_classifier_ops {
	ecm_ae_classifier_get_t ae_get;	/**< Get the acceleration engine classifier. */
	uint32_t ae_flags;			/**< AE flags */
};

/**
 * Indicates whether the IPv4 connection is being decelerated in the AE.
 *
 * @param	src_ip		The source IP address.
 * @param	src_port	The source port.
 * @param	dest_ip		The destination IP address.
 * @param	dest_port	The destination port.
 * @param	protocol	The protocol.
 *
 * @return
 * True if decelerated; false if not.
 */
bool ecm_ae_classifier_decelerate_v4_connection(__be32 src_ip, int src_port,
						__be32 dest_ip, int dest_port, int protocol);

/**
 * Indicates whether the IPv6 connection is being decelerated in the AE.
 *
 * @param	src_ip		The source IP address.
 * @param	src_port	The source port.
 * @param	dest_ip		The destination IP address.
 * @param	dest_port	The destination port.
 * @param	protocol	The protocol.
 *
 * @return
 * True if decelerated; false if not.
 */
bool ecm_ae_classifier_decelerate_v6_connection(struct in6_addr src_ip, int src_port,
						struct in6_addr dest_ip, int dest_port, int protocol);

/**
 * Registers a client with the AE classifier.
 *
 * @param	ops		The acceleration engine classifier operations.
 *
 * @return
 * None.
 */
void ecm_ae_classifier_ops_register(struct ecm_ae_classifier_ops *ops);

/**
 * Unregisters a client with the AE classifier.
 *
 * @return
 * None.
 */
void ecm_ae_classifier_ops_unregister(void);

/**
 * @}
 */

#endif /* __ECM_AE_CLASSIFIER_PUBLIC_H__ */
