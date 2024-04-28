/*
 **************************************************************************
 * Copyright (c) 2015, The Linux Foundation.  All rights reserved.
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
 * Structure used to synchronise a classifier instance with the state as presented by the accel engine
 */
enum ecm_classifier_pcc_results {
	ECM_CLASSIFIER_PCC_RESULT_NOT_YET,		/* Accel is neither permitted nor denied just yet - try again later */
	ECM_CLASSIFIER_PCC_RESULT_DENIED,		/* Accel is denied for this connection */
	ECM_CLASSIFIER_PCC_RESULT_PERMITTED,		/* Accel is permitted for this connection */
};
typedef enum ecm_classifier_pcc_results ecm_classifier_pcc_result_t;

struct ecm_classifier_pcc_registrant;

typedef void (*ecm_classifier_pcc_ref_method_t)(struct ecm_classifier_pcc_registrant *r);
typedef void (*ecm_classifier_pcc_deref_method_t)(struct ecm_classifier_pcc_registrant *r);
typedef ecm_classifier_pcc_result_t (*ecm_classifier_pcc_okay_to_accel_v4_method_t)(struct ecm_classifier_pcc_registrant *r, uint8_t *src_mac, __be32 src_ip, int src_port, uint8_t *dest_mac, __be32 dest_ip, int dest_port, int protocol);
typedef ecm_classifier_pcc_result_t (*ecm_classifier_pcc_okay_to_accel_v6_method_t)(struct ecm_classifier_pcc_registrant *r, uint8_t *src_mac, struct in6_addr *src_ip, int src_port, uint8_t *dest_mac, struct in6_addr *dest_ip, int dest_port, int protocol);

/*
 * struct ecm_classifier_pcc_registrant
 *	Used by customer parental control code to register their existance with the ECM PCC classifier
 */
struct ecm_classifier_pcc_registrant {
	uint16_t version;					/* Customer Parental Controls (CPC) supplies 1 for this field. */

	struct ecm_classifier_pcc_registrant *pcc_next;		/* ECM PCC use */
	struct ecm_classifier_pcc_registrant *pcc_prev;		/* ECM PCC use */
	uint32_t pcc_flags;					/* ECM PCC use */

	atomic_t ref_count;					/* CPC sets this to 1 initially when registering with ECM.
								 * PCC takes its own private 'ref' for the registrant so after registering the CPC should 'deref' the initial '1'.
								 * CPC MUST NOT deallocate this structure until the ref_count is dropped to zero by deref() calls
								 */
	struct module *this_module;				/* Pointer to the registrants module */

	ecm_classifier_pcc_ref_method_t ref;			/* When called the ref_count is incremented by 1 */
	ecm_classifier_pcc_deref_method_t deref;		/* When called the ref_count is decremented by 1.
								 * When ref_count becomes 0 no further calls will be made upon this registrant
								 */
	ecm_classifier_pcc_okay_to_accel_v4_method_t okay_to_accel_v4;
								/* ECM PCC asks the CPC if the given connection is okay to accelerate */
	ecm_classifier_pcc_okay_to_accel_v6_method_t okay_to_accel_v6;
								/* ECM PCC asks the CPC if the given connection is okay to accelerate */
};

extern int ecm_classifier_pcc_register(struct ecm_classifier_pcc_registrant *r);
extern void ecm_classifier_pcc_unregister_begin(struct ecm_classifier_pcc_registrant *r);

extern void ecm_classifier_pcc_permit_accel_v4(uint8_t *src_mac, __be32 src_ip, int src_port, uint8_t *dest_mac, __be32 dest_ip, int dest_port, int protocol);
extern void ecm_classifier_pcc_deny_accel_v4(uint8_t *src_mac, __be32 src_ip, int src_port, uint8_t *dest_mac, __be32 dest_ip, int dest_port, int protocol);

extern void ecm_classifier_pcc_permit_accel_v6(uint8_t *src_mac, struct in6_addr *src_ip, int src_port, uint8_t *dest_mac, struct in6_addr *dest_ip, int dest_port, int protocol);
extern void ecm_classifier_pcc_deny_accel_v6(uint8_t *src_mac, struct in6_addr *src_ip, int src_port, uint8_t *dest_mac, struct in6_addr *dest_ip, int dest_port, int protocol);

