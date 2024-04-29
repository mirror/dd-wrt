/*
 **************************************************************************
 * Copyright (c) 2016, 2018-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <linux/version.h>
#include <linux/types.h>
#include <net/ip.h>
#include <net/ipv6.h>
#include <linux/inet.h>
#include <linux/etherdevice.h>

/*
 * Debug output levels
 * 0 = OFF
 * 1 = ASSERTS / ERRORS
 * 2 = 1 + WARN
 * 3 = 2 + INFO
 * 4 = 3 + TRACE
 */
#define DEBUG_LEVEL ECM_CLASSIFIER_DEBUG_LEVEL

#include "ecm_types.h"
#include "ecm_db_types.h"
#include "ecm_state.h"
#include "ecm_tracker.h"
#include "ecm_classifier.h"
#include "ecm_front_end_types.h"
#include "ecm_db.h"
#ifdef ECM_CLASSIFIER_NL_ENABLE
#include "ecm_classifier_nl.h"
#endif
#ifdef ECM_CLASSIFIER_HYFI_ENABLE
#include "ecm_classifier_hyfi.h"
#endif
#ifdef ECM_CLASSIFIER_DSCP_ENABLE
#include "ecm_classifier_dscp.h"
#endif
#ifdef ECM_CLASSIFIER_PCC_ENABLE
#include "ecm_classifier_pcc.h"
#endif
#ifdef ECM_CLASSIFIER_MARK_ENABLE
#include "ecm_classifier_mark.h"
#endif
#ifdef ECM_CLASSIFIER_OVS_ENABLE
#include "ecm_classifier_ovs.h"
#endif
#ifdef ECM_CLASSIFIER_EMESH_ENABLE
#include "ecm_classifier_emesh.h"
#endif
#ifdef ECM_CLASSIFIER_MSCS_ENABLE
#include "ecm_classifier_mscs.h"
#endif

/*
 * Default slow path packets allowed before the acceleration
 *  0 - The feature is disabled. Acceleration starts immediately.
 *  1 - Acceleration will not start until both direction traffic is seen.
 *  N - Acceleration will not start until N packets are seen in the slow path.
 */
int ecm_classifier_accel_delay_pkts = 0;

/*
 * ecm_classifier_assign_classifier()
 *	Instantiate and assign classifier of type upon the connection, also returning it if it could be allocated.
 */
struct ecm_classifier_instance *ecm_classifier_assign_classifier(struct ecm_db_connection_instance *ci, ecm_classifier_type_t type)
{
	DEBUG_TRACE("%px: Assign classifier of type: %d\n", ci, type);
	DEBUG_ASSERT(type != ECM_CLASSIFIER_TYPE_DEFAULT, "Must never need to instantiate default type in this way");

	switch (type) {
#ifdef ECM_CLASSIFIER_PCC_ENABLE
	case ECM_CLASSIFIER_TYPE_PCC: {
		struct ecm_classifier_pcc_instance *pcci;

		pcci = ecm_classifier_pcc_instance_alloc(ci);
		if (!pcci) {
			DEBUG_TRACE("%px: Failed to create Parental Controls classifier\n", ci);
			return NULL;
		}
		DEBUG_TRACE("%px: Created Parental Controls classifier: %px\n", ci, pcci);
		ecm_db_connection_classifier_assign(ci, (struct ecm_classifier_instance *)pcci);
		return (struct ecm_classifier_instance *)pcci;
	}
#endif
#ifdef ECM_CLASSIFIER_OVS_ENABLE
	case ECM_CLASSIFIER_TYPE_OVS: {
		struct ecm_classifier_ovs_instance *ecvi;

		ecvi = ecm_classifier_ovs_instance_alloc(ci);
		if (!ecvi) {
			DEBUG_TRACE("%px: Failed to create ovs classifier\n", ci);
			return NULL;
		}
		DEBUG_TRACE("%px: Created ovs classifier: %px\n", ci, ecvi);
		ecm_db_connection_classifier_assign(ci, (struct ecm_classifier_instance *)ecvi);
		return (struct ecm_classifier_instance *)ecvi;
	}
#endif
#ifdef ECM_CLASSIFIER_NL_ENABLE
	case ECM_CLASSIFIER_TYPE_NL: {
		struct ecm_classifier_nl_instance *cnli;

		cnli = ecm_classifier_nl_instance_alloc(ci);
		if (!cnli) {
			DEBUG_TRACE("%px: Failed to create Netlink classifier\n", ci);
			return NULL;
		}
		DEBUG_TRACE("%px: Created Netlink classifier: %px\n", ci, cnli);
		ecm_db_connection_classifier_assign(ci, (struct ecm_classifier_instance *)cnli);
		return (struct ecm_classifier_instance *)cnli;
	}
#endif
#ifdef ECM_CLASSIFIER_EMESH_ENABLE
	case ECM_CLASSIFIER_TYPE_EMESH: {
		struct ecm_classifier_emesh_sawf_instance *cemi;

		cemi = ecm_classifier_emesh_sawf_instance_alloc(ci);
		if (!cemi) {
			DEBUG_TRACE("%px: Failed to create emesh classifier\n", ci);
			return NULL;
		}
		DEBUG_TRACE("%px: Created emesh classifier: %px\n", ci, cemi);
		ecm_db_connection_classifier_assign(ci, (struct ecm_classifier_instance *)cemi);
		return (struct ecm_classifier_instance *)cemi;
	}
#endif
#ifdef ECM_CLASSIFIER_DSCP_ENABLE
	case ECM_CLASSIFIER_TYPE_DSCP: {
		struct ecm_classifier_dscp_instance *cdscpi;

		cdscpi = ecm_classifier_dscp_instance_alloc(ci);
		if (!cdscpi) {
			DEBUG_TRACE("%px: Failed to create DSCP classifier\n", ci);
			return NULL;
		}
		DEBUG_TRACE("%px: Created DSCP classifier: %px\n", ci, cdscpi);
		ecm_db_connection_classifier_assign(ci, (struct ecm_classifier_instance *)cdscpi);
		return (struct ecm_classifier_instance *)cdscpi;
	}
#endif
#ifdef ECM_CLASSIFIER_HYFI_ENABLE
	case ECM_CLASSIFIER_TYPE_HYFI: {
		struct ecm_classifier_hyfi_instance *chfi;

		chfi = ecm_classifier_hyfi_instance_alloc(ci);
		if (!chfi) {
			DEBUG_TRACE("%px: Failed to create HyFi classifier\n", ci);
			return NULL;
		}
		DEBUG_TRACE("%px: Created HyFi classifier: %px\n", ci, chfi);
		ecm_db_connection_classifier_assign(ci, (struct ecm_classifier_instance *)chfi);
		return (struct ecm_classifier_instance *)chfi;
	}
#endif
#ifdef ECM_CLASSIFIER_MARK_ENABLE
	case ECM_CLASSIFIER_TYPE_MARK: {
		struct ecm_classifier_mark_instance *ecmi;

		ecmi = ecm_classifier_mark_instance_alloc(ci);
		if (!ecmi) {
			DEBUG_TRACE("%px: Failed to create mark classifier\n", ci);
			return NULL;
		}
		DEBUG_TRACE("%px: Created mark classifier: %px\n", ci, ecmi);
		ecm_db_connection_classifier_assign(ci, (struct ecm_classifier_instance *)ecmi);
		return (struct ecm_classifier_instance *)ecmi;
	}
#endif
#ifdef ECM_CLASSIFIER_MSCS_ENABLE
	case ECM_CLASSIFIER_TYPE_MSCS: {
		struct ecm_classifier_mscs_instance *ecmi;

		ecmi = ecm_classifier_mscs_instance_alloc(ci);
		if (!ecmi) {
			DEBUG_TRACE("%px: Failed to create mscs classifier\n", ci);
			return NULL;
		}
		DEBUG_TRACE("%px: Created mscs classifier: %px\n", ci, ecmi);
		ecm_db_connection_classifier_assign(ci, (struct ecm_classifier_instance *)ecmi);
		return (struct ecm_classifier_instance *)ecmi;
	}
#endif
	default:
		DEBUG_ASSERT(NULL, "%px: Unsupported type: %d\n", ci, type);
		return NULL;
	}
}

/*
 * ecm_classifier_reclassify()
 *	Signal reclassify upon the assigned classifiers.
 *
 * Classifiers that unassigned themselves we TRY to re-instantiate them.
 * Returns false if the function is not able to instantiate all missing classifiers.
 * This function does not release and references to classifiers in the assignments[].
 */
bool ecm_classifier_reclassify(struct ecm_db_connection_instance *ci, int assignment_count, struct ecm_classifier_instance *assignments[])
{
	ecm_classifier_type_t classifier_type;
	int i;
	bool full_reclassification = true;

	/*
	 * assignment_count will always be <= the number of classifier types available
	 */
	for (i = 0, classifier_type = ECM_CLASSIFIER_TYPE_DEFAULT; i < assignment_count; ++i, ++classifier_type) {
		ecm_classifier_type_t aci_type;
		struct ecm_classifier_instance *aci;

		aci = assignments[i];
		aci_type = aci->type_get(aci);
		DEBUG_TRACE("%px: Reclassify: %d\n", ci, aci_type);
		aci->reclassify(aci);

		/*
		 * If the connection has a full complement of assigned classifiers then these will match 1:1 with the classifier_type (all in same order).
		 * If not, we have to create the missing ones.
		 */
		if (aci_type == classifier_type) {
			continue;
		}

		/*
		 * Need to instantiate the missing classifier types until we get to the same type as aci_type then we are back in sync to continue reclassification
		 */
		while (classifier_type != aci_type) {
			struct ecm_classifier_instance *naci;
			DEBUG_TRACE("%px: Instantiate missing type: %d\n", ci, classifier_type);
			DEBUG_ASSERT(classifier_type < ECM_CLASSIFIER_TYPES, "Algorithm bad");

			naci = ecm_classifier_assign_classifier(ci, classifier_type);
			if (!naci) {
				full_reclassification = false;
			} else {
				naci->deref(naci);
			}

			classifier_type++;
		}
	}

	/*
	 * Add missing types
	 */
	for (; classifier_type < ECM_CLASSIFIER_TYPES; ++classifier_type) {
		struct ecm_classifier_instance *naci;
		DEBUG_TRACE("%px: Instantiate missing type: %d\n", ci, classifier_type);

		naci = ecm_classifier_assign_classifier(ci, classifier_type);
		if (!naci) {
			full_reclassification = false;
		} else {
			naci->deref(naci);
		}
	}

	DEBUG_TRACE("%px: reclassify done: %u\n", ci, full_reclassification);
	return full_reclassification;
}
