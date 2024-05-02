/*
 **************************************************************************
 * Copyright (c) 2015, 2021, The Linux Foundation.  All rights reserved.
 * Copyright (c) 2022-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * @file ecm_classifier_pcc_public.h
 *	ECM PCC classifier.
 */

/**
 * @addtogroup ecm_classifier_pcc_subsystem
 * @{
 */

/**
 * Structure used to synchronise a classifier instance with the state as presented by the acceleration engine (AE).
 */
enum ecm_classifier_pcc_results {
	ECM_CLASSIFIER_PCC_RESULT_NOT_YET,	/**< Acceleration is neither permitted nor denied; try again later. */
	ECM_CLASSIFIER_PCC_RESULT_DENIED,	/**< Acceleration is denied for this connection. */
	ECM_CLASSIFIER_PCC_RESULT_PERMITTED,	/**< Acceleration is permitted for this connection. */
};
typedef enum /** @cond */ ecm_classifier_pcc_results /** @endcond */ ecm_classifier_pcc_result_t;

/**
 * Feature flags.
 *
 * The registrant can request multiple features to be enabled on the connection, using a bitmap of features
 * with the 'ecm_classifier_pcc_info' object.
 *
 * @note1hang Currently only the 'mirror' feature is supported.
 */
enum ecm_classifier_pcc_feature_flags {
	ECM_CLASSIFIER_PCC_FEATURE_NONE,
	ECM_CLASSIFIER_PCC_FEATURE_MIRROR = 0x1,		/**< Set by the registrant if mirroring is needed on the connection. */
	ECM_CLASSIFIER_PCC_FEATURE_ACL = 0x2,			/**< Set by the registrant if ACL is needed on the connection. */
	ECM_CLASSIFIER_PCC_FEATURE_POLICER = 0x4,		/**< Set by the registrant if POLICER is needed on the connection. */
	ECM_CLASSIFIER_PCC_FEATURE_ACL_EGRESS_DEV = 0x8,	/**< Set by the registrant if ACL mirroring based on egress dev is needed on the connection. */
};

/**
 * ecm_classifier_pcc_ap_info
 *	Policer or ACL ID to be used with the rule during post-flow processing stage
 */
struct ecm_classifier_pcc_ap_info {
	uint16_t flow_ap_index;			/**< Flow Policer/ACL ID */
	uint16_t return_ap_index;		/**< Return Policer/ACL ID */
};

/**
 * Mirror netdevices to be used for mirroring an offloaded flow.
 */
struct ecm_classifier_pcc_mirror_info {
	struct net_device *tuple_mirror_dev;		/**< Tuple direction mirror netdevice. */
	struct net_device *tuple_ret_mirror_dev;	/**< Tuple return direction mirror netdevice. */
};

/**
 * Additional n-tuple information to be passed to PCC clients for rule match.
 */
struct ecm_classifier_pcc_dev_info {
	struct net_device *in_dev;			/**< In netdevice. */
	struct net_device *out_dev;			/**< Out netdevice. */
};

/*
 * Output parameters from PCC clients to PCC classifier.
 */
struct ecm_classifier_pcc_registrant_output {
	struct ecm_classifier_pcc_mirror_info mirror;	/**< Mirror netdevice information to be used for mirroring an offloaded flow. */
	struct ecm_classifier_pcc_ap_info ap_info;	/**< Policer or ACL ID. */
};

/*
 * Input parameters for PCC clients.
 */
struct ecm_classifier_pcc_registrant_input {
	struct ecm_classifier_pcc_dev_info dev_info;	/**< Flow source and destination netdev info. */
};

/**
 * Feature information related to the connection, returned by registrant's callback.
 */
struct ecm_classifier_pcc_info {
	uint32_t feature_flags;						/**< Bitmap of requested features (ecm_classifier_pcc_feature_ids). */
	struct ecm_classifier_pcc_registrant_input input_params;	/**< Input parameters to the PCC classifier registrants. */
	struct ecm_classifier_pcc_registrant_output output_params;	/**< Output parameters from the PCC registrancts to classifier. */
};

struct ecm_classifier_pcc_registrant;

typedef void (*ecm_classifier_pcc_ref_method_t)(struct ecm_classifier_pcc_registrant *r);
typedef void (*ecm_classifier_pcc_deref_method_t)(struct ecm_classifier_pcc_registrant *r);
typedef ecm_classifier_pcc_result_t (*ecm_classifier_pcc_okay_to_accel_v4_method_t)(struct ecm_classifier_pcc_registrant *r, uint8_t *src_mac, __be32 src_ip, int src_port, uint8_t *dest_mac, __be32 dest_ip, int dest_port, int protocol);
typedef ecm_classifier_pcc_result_t (*ecm_classifier_pcc_okay_to_accel_v6_method_t)(struct ecm_classifier_pcc_registrant *r, uint8_t *src_mac, struct in6_addr *src_ip, int src_port, uint8_t *dest_mac, struct in6_addr *dest_ip, int dest_port, int protocol);

/**
 * Queries the customer's PCC registrant when ECM is processing the packet (IPv4).
 *
 * @par Customers can (optionally) also share additional connection properties to enable for the
 * connection using the cinfo object that is passed to them.
 *
 * @param	r		External registrant.
 * @param	src_mac		Source MAC address.
 * @param	src_ip		Source IP address.
 * @param	src_port	Source port.
 * @param	dest_mac	Destination MAC address.
 * @param	dest_ip		Destination IP address.
 * @param	dest_port	Destination port.
 * @param	cinfo		cinfo pointer (allocated by the ECM).
 *
 * @return
 * The return value is used to decide whether acceleration is needed for the connection or not.
 *
 * @note1hang Currently only one connection property named 'mirror' is supported.
 */
typedef ecm_classifier_pcc_result_t
 (*ecm_classifier_pcc_get_accel_info_v4_method_t)(struct ecm_classifier_pcc_registrant *r,
		  uint8_t *src_mac, __be32 src_ip, int src_port,
		  uint8_t *dest_mac, __be32 dest_ip, int dest_port,
		  int protocol, struct ecm_classifier_pcc_info *cinfo);

/**
 * Queries the customer's PCC registrant when ECM is processing the packet (IPv6).
 *
 * @par Customers can (optionally) also share additional connection properties to enable for the
 * connection using the cinfo object that is passed to them.
 *
 * @param	r		External registrant.
 * @param	src_mac		Source MAC address.
 * @param	src_ip		Source IP address.
 * @param	src_port	Source port.
 * @param	dest_mac	Destination MAC address.
 * @param	dest_ip		Destination IP address.
 * @param	dest_port	Destination port.
 * @param	cinfo		cinfo pointer (allocated by the ECM).
 *
 * @return
 * The return value is used to decide whether acceleration is needed for the connection or not.
 *
 * @note1hang Currently only one connection property named 'mirror' is supported.
 */
typedef ecm_classifier_pcc_result_t
 (*ecm_classifier_pcc_get_accel_info_v6_method_t)(struct ecm_classifier_pcc_registrant *r,
		uint8_t *src_mac, struct in6_addr *src_ip, int src_port,
		uint8_t *dest_mac, struct in6_addr *dest_ip, int dest_port,
		int protocol, struct ecm_classifier_pcc_info *cinfo);

/**
 * Used by customer parental control code to register their existance with the ECM PCC classifier.
 */
struct ecm_classifier_pcc_registrant {
	uint16_t version;
			/**< Customer Parental Controls (CPC) supplies 1 for this field. */

	struct ecm_classifier_pcc_registrant *pcc_next;
			/**< ECM PCC use. */
	struct ecm_classifier_pcc_registrant *pcc_prev;
			/**< ECM PCC use. */
	uint32_t pcc_flags;
			/**< ECM PCC use. */

	/**
	 * CPC sets this to 1 initially when registering with ECM.
	 * PCC takes its own private 'ref' for the registrant so after
	 * registering the CPC should 'deref' the initial '1'.
	 * CPC MUST NOT deallocate this structure until the ref_count is
	 * dropped to zero by deref() calls.
	 */
	atomic_t ref_count;

	struct module *this_module;
			/**< Pointer to the registrants module. */

	ecm_classifier_pcc_ref_method_t ref;
			/**< When called the ref_count is incremented by 1. */

	/**
	 * When called the ref_count is decremented by 1.
	 * When ref_count becomes 0 no further calls will be made to
	 * this registrant.
	 */
	ecm_classifier_pcc_deref_method_t deref;

	ecm_classifier_pcc_okay_to_accel_v4_method_t okay_to_accel_v4;
			/**< ECM PCC asks the CPC if the given connection is okay to accelerate. */
	ecm_classifier_pcc_okay_to_accel_v6_method_t okay_to_accel_v6;
			/**< ECM PCC asks the CPC if the given connection is okay to accelerate. */
	ecm_classifier_pcc_get_accel_info_v4_method_t get_accel_info_v4;
			/**< ECM PCC asks the CPC for the acceleration information. */
	ecm_classifier_pcc_get_accel_info_v6_method_t get_accel_info_v6;
			/**< ECM PCC asks the CPC for the acceleration information. */
};

/**
 * Registers a PCC client with the ECM.
 *
 * @param	r	External registrant.
 *
 * @return
 * The status of the PCC registration operation.
 */
extern int ecm_classifier_pcc_register(struct ecm_classifier_pcc_registrant *r);

/**
 * Unregisters a PCC client with the ECM.
 *
 * @return
 * None.
 */
extern void ecm_classifier_pcc_unregister_begin(struct ecm_classifier_pcc_registrant *r);

/**
 * Allows acceleration (IPv4).
 *
 * @param	src_mac		The source MAC address.
 * @param	src_ip		The source IP address.
 * @param	src_port	The source port.
 * @param	dest_mac	The destination MAC address.
 * @param	dest_ip		The destination IP address.
 * @param	dest_port	The destination port.
 * @param	protocol	The protocol.
 *
 * @return
 * None.
 */
extern void ecm_classifier_pcc_permit_accel_v4(uint8_t *src_mac, __be32 src_ip, int src_port, uint8_t *dest_mac, __be32 dest_ip, int dest_port, int protocol);

/**
 * Denies acceleration (IPv4).
 *
 * @param	src_mac		The source MAC address.
 * @param	src_ip		The source IP address.
 * @param	src_port	The source port.
 * @param	dest_mac	The destination MAC address.
 * @param	dest_ip		The destination IP address.
 * @param	dest_port	The destination port.
 * @param	protocol	The protocol.
 *
 * @return
 * None.
 */
extern void ecm_classifier_pcc_deny_accel_v4(uint8_t *src_mac, __be32 src_ip, int src_port, uint8_t *dest_mac, __be32 dest_ip, int dest_port, int protocol);

/**
 * Decelerates connections on a particular net device.
 *
 * @param	dev		The source net device.
 *
 * @return
 * None.
 */
extern void ecm_classifier_pcc_decel_by_dev(struct net_device *dev);

/**
 * Decelerates an existing IPv4 ECM connection.
 *
 * @param	src_mac		The source MAC address.
 * @param	src_ip		The source IP address.
 * @param	src_port	The source port.
 * @param	dest_mac	The destination MAC address.
 * @param	dest_ip		The destination IP address.
 * @param	dest_port	The destination port.
 * @param	protocol	The protocol.
 *
 * @return
 * True if decelerated; false if not.
 */
extern bool ecm_classifier_pcc_decel_v4(uint8_t *src_mac, __be32 src_ip, int src_port,
		uint8_t *dest_mac, __be32 dest_ip, int dest_port, int protocol);

/**
 * Allows acceleration (IPv6).
 *
 * @param	src_mac		The source MAC address.
 * @param	src_ip		The source IP address.
 * @param	src_port	The source port.
 * @param	dest_mac	The destination MAC address.
 * @param	dest_ip		The destination IP address.
 * @param	dest_port	The destination port.
 * @param	protocol	The protocol.
 *
 * @return
 * None.
 */
extern void ecm_classifier_pcc_permit_accel_v6(uint8_t *src_mac, struct in6_addr *src_ip, int src_port, uint8_t *dest_mac, struct in6_addr *dest_ip, int dest_port, int protocol);

/**
 * Denies acceleration (IPv6).
 *
 * @param	src_mac		The source MAC address.
 * @param	src_ip		The source IP address.
 * @param	src_port	The source port.
 * @param	dest_mac	The destination MAC address.
 * @param	dest_ip		The destination IP address.
 * @param	dest_port	The destination port.
 * @param	protocol	The protocol.
 *
 * @return
 * None.
 */
extern void ecm_classifier_pcc_deny_accel_v6(uint8_t *src_mac, struct in6_addr *src_ip, int src_port, uint8_t *dest_mac, struct in6_addr *dest_ip, int dest_port, int protocol);

/**
 * Decelerates an existing IPv6 ECM connection.
 *
 * @param	src_mac		The source MAC address.
 * @param	src_ip		The source IP address.
 * @param	src_port	The source port.
 * @param	dest_mac	The destination MAC address.
 * @param	dest_ip		The destination IP address.
 * @param	dest_port	The destination port.
 * @param	protocol	The protocol.
 *
 * @return
 * True if decelerated; false if not.
 */
extern bool ecm_classifier_pcc_decel_v6(uint8_t *src_mac, struct in6_addr *src_ip,
		int src_port, uint8_t *dest_mac, struct in6_addr *dest_ip,
		int dest_port, int protocol);

/**
 * @}
 */
