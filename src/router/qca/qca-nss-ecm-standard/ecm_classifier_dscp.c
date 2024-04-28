/*
 **************************************************************************
 * Copyright (c) 2014-2016, The Linux Foundation.  All rights reserved.
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

#include <linux/version.h>
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/icmp.h>
#include <linux/debugfs.h>
#include <linux/kthread.h>
#include <linux/pkt_sched.h>
#include <linux/string.h>
#include <net/route.h>
#include <net/ip.h>
#include <net/tcp.h>
#include <asm/unaligned.h>
#include <asm/uaccess.h>	/* for put_user */
#include <net/ipv6.h>
#include <linux/inet.h>
#include <linux/in.h>
#include <linux/udp.h>
#include <linux/tcp.h>

#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_bridge.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_l4proto.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/ipv4/nf_conntrack_ipv4.h>
#include <net/netfilter/ipv4/nf_defrag_ipv4.h>
#include <linux/netfilter/xt_dscp.h>
#include <net/netfilter/nf_conntrack_dscpremark_ext.h>

/*
 * Debug output levels
 * 0 = OFF
 * 1 = ASSERTS / ERRORS
 * 2 = 1 + WARN
 * 3 = 2 + INFO
 * 4 = 3 + TRACE
 */
#define DEBUG_LEVEL ECM_CLASSIFIER_DSCP_DEBUG_LEVEL

#include "ecm_types.h"
#include "ecm_db_types.h"
#include "ecm_state.h"
#include "ecm_tracker.h"
#include "ecm_classifier.h"
#include "ecm_front_end_types.h"
#include "ecm_tracker_udp.h"
#include "ecm_tracker_tcp.h"
#include "ecm_db.h"
#include "ecm_classifier_dscp.h"

/*
 * Magic numbers
 */
#define ECM_CLASSIFIER_DSCP_INSTANCE_MAGIC 0xFA43

/*
 * struct ecm_classifier_dscp_instance
 * 	State to allow tracking of dynamic qos for a connection
 */
struct ecm_classifier_dscp_instance {
	struct ecm_classifier_instance base;			/* Base type */

	struct ecm_classifier_dscp_instance *next;		/* Next classifier state instance (for accouting and reporting purposes) */
	struct ecm_classifier_dscp_instance *prev;		/* Next classifier state instance (for accouting and reporting purposes) */

	uint32_t ci_serial;					/* RO: Serial of the connection */
	struct ecm_classifier_process_response process_response;/* Last process response computed */

	int refs;						/* Integer to trap we never go negative */
#if (DEBUG_LEVEL > 0)
	uint16_t magic;
#endif
};

/*
 * Operational control
 */
static int ecm_classifier_dscp_enabled = 1;			/* Operational behaviour */

/*
 * Management thread control
 */
static bool ecm_classifier_dscp_terminate_pending = false;	/* True when the user wants us to terminate */

/*
 * Debugfs dentry object.
 */
static struct dentry *ecm_classifier_dscp_dentry;

/*
 * Locking of the classifier structures
 */
static DEFINE_SPINLOCK(ecm_classifier_dscp_lock);			/* Protect SMP access. */

/*
 * List of our classifier instances
 */
static struct ecm_classifier_dscp_instance *ecm_classifier_dscp_instances = NULL;
								/* list of all active instances */
static int ecm_classifier_dscp_count = 0;			/* Tracks number of instances allocated */

/*
 * ecm_classifier_dscp_ref()
 *	Ref
 */
static void ecm_classifier_dscp_ref(struct ecm_classifier_instance *ci)
{
	struct ecm_classifier_dscp_instance *cdscpi;
	cdscpi = (struct ecm_classifier_dscp_instance *)ci;

	DEBUG_CHECK_MAGIC(cdscpi, ECM_CLASSIFIER_DSCP_INSTANCE_MAGIC, "%p: magic failed\n", cdscpi);
	spin_lock_bh(&ecm_classifier_dscp_lock);
	cdscpi->refs++;
	DEBUG_TRACE("%p: cdscpi ref %d\n", cdscpi, cdscpi->refs);
	DEBUG_ASSERT(cdscpi->refs > 0, "%p: ref wrap\n", cdscpi);
	spin_unlock_bh(&ecm_classifier_dscp_lock);
}

/*
 * ecm_classifier_dscp_deref()
 *	Deref
 */
static int ecm_classifier_dscp_deref(struct ecm_classifier_instance *ci)
{
	struct ecm_classifier_dscp_instance *cdscpi;
	cdscpi = (struct ecm_classifier_dscp_instance *)ci;

	DEBUG_CHECK_MAGIC(cdscpi, ECM_CLASSIFIER_DSCP_INSTANCE_MAGIC, "%p: magic failed\n", cdscpi);
	spin_lock_bh(&ecm_classifier_dscp_lock);
	cdscpi->refs--;
	DEBUG_ASSERT(cdscpi->refs >= 0, "%p: refs wrapped\n", cdscpi);
	DEBUG_TRACE("%p: DSCP classifier deref %d\n", cdscpi, cdscpi->refs);
	if (cdscpi->refs) {
		int refs = cdscpi->refs;
		spin_unlock_bh(&ecm_classifier_dscp_lock);
		return refs;
	}

	/*
	 * Object to be destroyed
	 */
	ecm_classifier_dscp_count--;
	DEBUG_ASSERT(ecm_classifier_dscp_count >= 0, "%p: ecm_classifier_dscp_count wrap\n", cdscpi);

	/*
	 * UnLink the instance from our list
	 */
	if (cdscpi->next) {
		cdscpi->next->prev = cdscpi->prev;
	}
	if (cdscpi->prev) {
		cdscpi->prev->next = cdscpi->next;
	} else {
		DEBUG_ASSERT(ecm_classifier_dscp_instances == cdscpi, "%p: list bad %p\n", cdscpi, ecm_classifier_dscp_instances);
		ecm_classifier_dscp_instances = cdscpi->next;
	}
	cdscpi->next = NULL;
	cdscpi->prev = NULL;
	spin_unlock_bh(&ecm_classifier_dscp_lock);

	/*
	 * Final
	 */
	DEBUG_INFO("%p: Final DSCP classifier instance\n", cdscpi);
	kfree(cdscpi);

	return 0;
}

/*
 * ecm_classifier_dscp_process()
 *	Process new data for connection
 */
static void ecm_classifier_dscp_process(struct ecm_classifier_instance *aci, ecm_tracker_sender_type_t sender,
						struct ecm_tracker_ip_header *ip_hdr, struct sk_buff *skb,
						struct ecm_classifier_process_response *process_response)
{
	struct ecm_classifier_dscp_instance *cdscpi;
	ecm_classifier_relevence_t relevance;
	struct ecm_db_connection_instance *ci = NULL;
	struct ecm_front_end_connection_instance *feci;
	ecm_front_end_acceleration_mode_t accel_mode;
	int protocol;
	uint32_t became_relevant = 0;
	struct nf_conn *ct;
	enum ip_conntrack_info ctinfo;
	struct nf_ct_dscpremark_ext *dscpcte;
	uint32_t flow_qos_tag;
	uint32_t return_qos_tag;
	uint8_t flow_dscp;
	uint8_t return_dscp;
	bool dscp_marked = false;

	cdscpi = (struct ecm_classifier_dscp_instance *)aci;
	DEBUG_CHECK_MAGIC(cdscpi, ECM_CLASSIFIER_DSCP_INSTANCE_MAGIC, "%p: magic failed\n", cdscpi);

	/*
	 * Are we yet to decide if this instance is relevant to the connection?
	 */
	spin_lock_bh(&ecm_classifier_dscp_lock);
	relevance = cdscpi->process_response.relevance;

	/*
	 * Are we relevant?
	 */
	if (relevance == ECM_CLASSIFIER_RELEVANCE_NO) {
		/*
		 * Lock still held
		 */
		goto dscp_classifier_out;
	}

	/*
	 * Yes or maybe relevant.
	 *
	 * Need to decide our relevance to this connection.
	 * We are only relevent to a connection iff:
	 * 1. We are enabled.
	 * 2. Connection can be accelerated.
	 * 3. Connection has a ct, ct has a dscp remark extension and the rule is validated.
	 * Any other condition and we are not and will stop analysing this connection.
	 */
	if (!ecm_classifier_dscp_enabled) {
		/*
		 * Lock still held
		 */
		cdscpi->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_NO;
		goto dscp_classifier_out;
	}
	spin_unlock_bh(&ecm_classifier_dscp_lock);

	/*
	 * Can we accelerate?
	 */
	ci = ecm_db_connection_serial_find_and_ref(cdscpi->ci_serial);
	if (!ci) {
		DEBUG_TRACE("%p: No ci found for %u\n", cdscpi, cdscpi->ci_serial);
		spin_lock_bh(&ecm_classifier_dscp_lock);
		cdscpi->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_NO;
		goto dscp_classifier_out;
	}
	feci = ecm_db_connection_front_end_get_and_ref(ci);
	accel_mode = feci->accel_state_get(feci);
	feci->deref(feci);
	protocol = ecm_db_connection_protocol_get(ci);
	ecm_db_connection_deref(ci);
	if (ECM_FRONT_END_ACCELERATION_NOT_POSSIBLE(accel_mode)) {
		spin_lock_bh(&ecm_classifier_dscp_lock);
		cdscpi->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_NO;
		goto dscp_classifier_out;
	}

	/*
	 * Is there a valid conntrack?
	 */
	ct = nf_ct_get(skb, &ctinfo);
	if (!ct) {
		spin_lock_bh(&ecm_classifier_dscp_lock);
		cdscpi->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_NO;
		goto dscp_classifier_out;
	}

	/*
	 * Is there a DSCPREMARK extension?
	 */
	spin_lock_bh(&ct->lock);
	dscpcte = nf_ct_dscpremark_ext_find(ct);
	if (!dscpcte) {
		spin_unlock_bh(&ct->lock);
		spin_lock_bh(&ecm_classifier_dscp_lock);
		cdscpi->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_NO;
		goto dscp_classifier_out;
	}

	/*
	 * Was a DSCP rule enabled for the flow using the iptables 'DSCP'
	 * target?
	 */
	if (nf_conntrack_dscpremark_ext_get_dscp_rule_validity(ct)
				== NF_CT_DSCPREMARK_EXT_RULE_VALID) {
		dscp_marked = true;
	}

	/*
	 * Extract the priority and DSCP from skb and store into ct extension
	 * for each direction.
	 *
	 * For TCP flows, we would have the values for both the directions by
	 * the time the connection is established. For UDP flows, we copy
	 * over the values from one direction to another if we find the
	 * values for the other direction not set, which would be due to one
	 * of the following.
	 * a. We might not have seen a packet in the opposite direction
	 * b. There were no explicitly configured priority/DSCP for the opposite
	 *    direction.
	 *
	 */
	if (sender == ECM_TRACKER_SENDER_TYPE_SRC) {
		/*
		 * Record latest flow
		 */
		flow_qos_tag = skb->priority;
		dscpcte->flow_priority = flow_qos_tag;
		flow_dscp = ip_hdr->ds >> XT_DSCP_SHIFT;	/* NOTE: XT_DSCP_SHIFT is okay for V4 and V6 */
		dscpcte->flow_dscp = flow_dscp;

		/*
		 * Get the other side ready to return our PR
		 */
		if (protocol == IPPROTO_TCP) {
			return_qos_tag = dscpcte->reply_priority;
			return_dscp = dscpcte->reply_dscp;
		} else {
			/*
			 * Copy over the flow direction QoS
			 * and DSCP if the reply direction
			 * values are not set.
			 */
			if (dscpcte->reply_priority == 0) {
				return_qos_tag = flow_qos_tag;
			} else {
				return_qos_tag = dscpcte->reply_priority;
			}

			if (dscpcte->reply_dscp == 0) {
				return_dscp = flow_dscp;
			} else {
				return_dscp = dscpcte->reply_dscp;
			}
		}
		DEBUG_TRACE("Flow DSCP: %x Flow priority: %d, Return DSCP: %x Return priority: %d\n",
				dscpcte->flow_dscp, dscpcte->flow_priority, return_dscp, return_qos_tag);
	} else {
		/*
		 * Record latest return
		 */
		return_qos_tag = skb->priority;
		dscpcte->reply_priority = return_qos_tag;
		return_dscp = ip_hdr->ds >> XT_DSCP_SHIFT;	/* NOTE: XT_DSCP_SHIFT is okay for V4 and V6 */
		dscpcte->reply_dscp = return_dscp;

		/*
		 * Get the other side ready to return our PR
		 */
		if (protocol == IPPROTO_TCP) {
			flow_qos_tag = dscpcte->flow_priority;
			flow_dscp = dscpcte->flow_dscp;
		} else {
			/*
			 * Copy over the return direction QoS
			 * and DSCP if the flow direction
			 * values are not set.
			 */
			if (dscpcte->flow_priority == 0) {
				flow_qos_tag = return_qos_tag;
			} else {
				flow_qos_tag = dscpcte->flow_priority;
			}

			if (dscpcte->flow_dscp == 0) {
				flow_dscp = return_dscp;
			} else {
				flow_dscp = dscpcte->flow_dscp;
			}
		}
		DEBUG_TRACE("Return DSCP: %x Return priority: %d, Flow DSCP: %x Flow priority: %d\n",
				dscpcte->reply_dscp, dscpcte->reply_priority, flow_dscp, flow_qos_tag);
	}
	spin_unlock_bh(&ct->lock);

	/*
	 * We are relevant to the connection
	 */
	became_relevant = ecm_db_time_get();

	spin_lock_bh(&ecm_classifier_dscp_lock);
	cdscpi->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_YES;
	cdscpi->process_response.became_relevant = became_relevant;

	cdscpi->process_response.process_actions = ECM_CLASSIFIER_PROCESS_ACTION_QOS_TAG;
	cdscpi->process_response.flow_qos_tag = flow_qos_tag;
	cdscpi->process_response.return_qos_tag = return_qos_tag;

	/*
	 * Check if we need to set DSCP
	 */
	if (dscp_marked) {
		cdscpi->process_response.flow_dscp = flow_dscp;
		cdscpi->process_response.return_dscp = return_dscp;
		cdscpi->process_response.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_DSCP;
	}

dscp_classifier_out:

	/*
	 * Return our process response
	 */
	*process_response = cdscpi->process_response;
	spin_unlock_bh(&ecm_classifier_dscp_lock);
}

/*
 * ecm_classifier_dscp_sync_to_v4()
 *	Front end is pushing accel engine state to us
 */
static void ecm_classifier_dscp_sync_to_v4(struct ecm_classifier_instance *aci, struct ecm_classifier_rule_sync *sync)
{
	struct ecm_classifier_dscp_instance *cdscpi __attribute__((unused));

	cdscpi = (struct ecm_classifier_dscp_instance *)aci;
	DEBUG_CHECK_MAGIC(cdscpi, ECM_CLASSIFIER_DSCP_INSTANCE_MAGIC, "%p: magic failed", cdscpi);
}

/*
 * ecm_classifier_dscp_sync_from_v4()
 *	Front end is retrieving accel engine state from us
 */
static void ecm_classifier_dscp_sync_from_v4(struct ecm_classifier_instance *aci, struct ecm_classifier_rule_create *ecrc)
{
	struct ecm_classifier_dscp_instance *cdscpi __attribute__((unused));

	cdscpi = (struct ecm_classifier_dscp_instance *)aci;
	DEBUG_CHECK_MAGIC(cdscpi, ECM_CLASSIFIER_DSCP_INSTANCE_MAGIC, "%p: magic failed", cdscpi);
}

/*
 * ecm_classifier_dscp_sync_to_v6()
 *	Front end is pushing accel engine state to us
 */
static void ecm_classifier_dscp_sync_to_v6(struct ecm_classifier_instance *aci, struct ecm_classifier_rule_sync *sync)
{
	struct ecm_classifier_dscp_instance *cdscpi __attribute__((unused));

	cdscpi = (struct ecm_classifier_dscp_instance *)aci;
	DEBUG_CHECK_MAGIC(cdscpi, ECM_CLASSIFIER_DSCP_INSTANCE_MAGIC, "%p: magic failed", cdscpi);
}

/*
 * ecm_classifier_dscp_sync_from_v6()
 *	Front end is retrieving accel engine state from us
 */
static void ecm_classifier_dscp_sync_from_v6(struct ecm_classifier_instance *aci, struct ecm_classifier_rule_create *ecrc)
{
	struct ecm_classifier_dscp_instance *cdscpi __attribute__((unused));

	cdscpi = (struct ecm_classifier_dscp_instance *)aci;
	DEBUG_CHECK_MAGIC(cdscpi, ECM_CLASSIFIER_DSCP_INSTANCE_MAGIC, "%p: magic failed", cdscpi);
}

/*
 * ecm_classifier_dscp_type_get()
 *	Get type of classifier this is
 */
static ecm_classifier_type_t ecm_classifier_dscp_type_get(struct ecm_classifier_instance *ci)
{
	struct ecm_classifier_dscp_instance *cdscpi;
	cdscpi = (struct ecm_classifier_dscp_instance *)ci;

	DEBUG_CHECK_MAGIC(cdscpi, ECM_CLASSIFIER_DSCP_INSTANCE_MAGIC, "%p: magic failed\n", cdscpi);
	return ECM_CLASSIFIER_TYPE_DSCP;
}

/*
 * ecm_classifier_dscp_last_process_response_get()
 *	Get result code returned by the last process call
 */
static void ecm_classifier_dscp_last_process_response_get(struct ecm_classifier_instance *ci,
							struct ecm_classifier_process_response *process_response)
{
	struct ecm_classifier_dscp_instance *cdscpi;

	cdscpi = (struct ecm_classifier_dscp_instance *)ci;
	DEBUG_CHECK_MAGIC(cdscpi, ECM_CLASSIFIER_DSCP_INSTANCE_MAGIC, "%p: magic failed\n", cdscpi);

	spin_lock_bh(&ecm_classifier_dscp_lock);
	*process_response = cdscpi->process_response;
	spin_unlock_bh(&ecm_classifier_dscp_lock);
}

/*
 * ecm_classifier_dscp_reclassify_allowed()
 *	Indicate if reclassify is allowed
 */
static bool ecm_classifier_dscp_reclassify_allowed(struct ecm_classifier_instance *ci)
{
	struct ecm_classifier_dscp_instance *cdscpi;
	cdscpi = (struct ecm_classifier_dscp_instance *)ci;
	DEBUG_CHECK_MAGIC(cdscpi, ECM_CLASSIFIER_DSCP_INSTANCE_MAGIC, "%p: magic failed\n", cdscpi);

	return true;
}

/*
 * ecm_classifier_dscp_reclassify()
 *	Reclassify
 */
static void ecm_classifier_dscp_reclassify(struct ecm_classifier_instance *ci)
{
	struct ecm_classifier_dscp_instance *cdscpi;
	cdscpi = (struct ecm_classifier_dscp_instance *)ci;
	DEBUG_CHECK_MAGIC(cdscpi, ECM_CLASSIFIER_DSCP_INSTANCE_MAGIC, "%p: magic failed\n", cdscpi);

	/*
	 * Revert back to MAYBE relevant - we will evaluate when we get the next process() call.
	 */
	spin_lock_bh(&ecm_classifier_dscp_lock);
	cdscpi->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_MAYBE;
	spin_unlock_bh(&ecm_classifier_dscp_lock);
}

#ifdef ECM_STATE_OUTPUT_ENABLE
/*
 * ecm_classifier_dscp_state_get()
 *	Return state
 */
static int ecm_classifier_dscp_state_get(struct ecm_classifier_instance *ci, struct ecm_state_file_instance *sfi)
{
	int result;
	struct ecm_classifier_dscp_instance *cdscpi;
	struct ecm_classifier_process_response process_response;

	cdscpi = (struct ecm_classifier_dscp_instance *)ci;
	DEBUG_CHECK_MAGIC(cdscpi, ECM_CLASSIFIER_DSCP_INSTANCE_MAGIC, "%p: magic failed", cdscpi);

	if ((result = ecm_state_prefix_add(sfi, "dscp"))) {
		return result;
	}

	spin_lock_bh(&ecm_classifier_dscp_lock);
	process_response = cdscpi->process_response;
	spin_unlock_bh(&ecm_classifier_dscp_lock);

	/*
	 * Output our last process response
	 */
	if ((result = ecm_classifier_process_response_state_get(sfi, &process_response))) {
		return result;
	}

	return ecm_state_prefix_remove(sfi);
}
#endif

/*
 * ecm_classifier_dscp_instance_alloc()
 *	Allocate an instance of the DSCP classifier
 */
struct ecm_classifier_dscp_instance *ecm_classifier_dscp_instance_alloc(struct ecm_db_connection_instance *ci)
{
	struct ecm_classifier_dscp_instance *cdscpi;

	/*
	 * Allocate the instance
	 */
	cdscpi = (struct ecm_classifier_dscp_instance *)kzalloc(sizeof(struct ecm_classifier_dscp_instance), GFP_ATOMIC | __GFP_NOWARN);
	if (!cdscpi) {
		DEBUG_WARN("Failed to allocate DSCP instance\n");
		return NULL;
	}

	DEBUG_SET_MAGIC(cdscpi, ECM_CLASSIFIER_DSCP_INSTANCE_MAGIC);
	cdscpi->refs = 1;
	cdscpi->base.process = ecm_classifier_dscp_process;
	cdscpi->base.sync_from_v4 = ecm_classifier_dscp_sync_from_v4;
	cdscpi->base.sync_to_v4 = ecm_classifier_dscp_sync_to_v4;
	cdscpi->base.sync_from_v6 = ecm_classifier_dscp_sync_from_v6;
	cdscpi->base.sync_to_v6 = ecm_classifier_dscp_sync_to_v6;
	cdscpi->base.type_get = ecm_classifier_dscp_type_get;
	cdscpi->base.last_process_response_get = ecm_classifier_dscp_last_process_response_get;
	cdscpi->base.reclassify_allowed = ecm_classifier_dscp_reclassify_allowed;
	cdscpi->base.reclassify = ecm_classifier_dscp_reclassify;
#ifdef ECM_STATE_OUTPUT_ENABLE
	cdscpi->base.state_get = ecm_classifier_dscp_state_get;
#endif
	cdscpi->base.ref = ecm_classifier_dscp_ref;
	cdscpi->base.deref = ecm_classifier_dscp_deref;
	cdscpi->ci_serial = ecm_db_connection_serial_get(ci);
	cdscpi->process_response.process_actions = 0;
	cdscpi->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_MAYBE;

	spin_lock_bh(&ecm_classifier_dscp_lock);

	/*
	 * Final check if we are pending termination
	 */
	if (ecm_classifier_dscp_terminate_pending) {
		spin_unlock_bh(&ecm_classifier_dscp_lock);
		DEBUG_INFO("%p: Terminating\n", ci);
		kfree(cdscpi);
		return NULL;
	}

	/*
	 * Link the new instance into our list at the head
	 */
	cdscpi->next = ecm_classifier_dscp_instances;
	if (ecm_classifier_dscp_instances) {
		ecm_classifier_dscp_instances->prev = cdscpi;
	}
	ecm_classifier_dscp_instances = cdscpi;

	/*
	 * Increment stats
	 */
	ecm_classifier_dscp_count++;
	DEBUG_ASSERT(ecm_classifier_dscp_count > 0, "%p: ecm_classifier_dscp_count wrap\n", cdscpi);
	spin_unlock_bh(&ecm_classifier_dscp_lock);

	DEBUG_INFO("DSCP instance alloc: %p\n", cdscpi);
	return cdscpi;
}
EXPORT_SYMBOL(ecm_classifier_dscp_instance_alloc);

/*
 * ecm_classifier_dscp_init()
 */
int ecm_classifier_dscp_init(struct dentry *dentry)
{
	DEBUG_INFO("DSCP classifier Module init\n");

	ecm_classifier_dscp_dentry = debugfs_create_dir("ecm_classifier_dscp", dentry);
	if (!ecm_classifier_dscp_dentry) {
		DEBUG_ERROR("Failed to create ecm dscp directory in debugfs\n");
		return -1;
	}

	debugfs_create_u32("enabled", S_IRUGO | S_IWUSR, ecm_classifier_dscp_dentry,
					(u32 *)&ecm_classifier_dscp_enabled);

	return 0;
}
EXPORT_SYMBOL(ecm_classifier_dscp_init);

/*
 * ecm_classifier_dscp_exit()
 */
void ecm_classifier_dscp_exit(void)
{
	DEBUG_INFO("DSCP classifier Module exit\n");

	spin_lock_bh(&ecm_classifier_dscp_lock);
	ecm_classifier_dscp_terminate_pending = true;
	spin_unlock_bh(&ecm_classifier_dscp_lock);

	/*
	 * Remove the debugfs files recursively.
	 */
	if (ecm_classifier_dscp_dentry) {
		debugfs_remove_recursive(ecm_classifier_dscp_dentry);
	}
}
EXPORT_SYMBOL(ecm_classifier_dscp_exit);
