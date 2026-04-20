/*
 **************************************************************************
 * Copyright (c) 2014-2016, 2020-2021, The Linux Foundation. All rights reserved.
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
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/icmp.h>
#include <linux/debugfs.h>
#include <linux/kthread.h>
#include <linux/pkt_sched.h>
#include <linux/string.h>
#include <linux/ctype.h>
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

/*
 * Debug output levels
 * 0 = OFF
 * 1 = ASSERTS / ERRORS
 * 2 = 1 + WARN
 * 3 = 2 + INFO
 * 4 = 3 + TRACE
 */
#define DEBUG_LEVEL ECM_CLASSIFIER_DEFAULT_DEBUG_LEVEL

#include "ecm_types.h"
#include "ecm_db_types.h"
#include "ecm_state.h"
#include "ecm_tracker.h"
#include "ecm_classifier.h"
#include "ecm_front_end_types.h"
#include "ecm_tracker_datagram.h"
#include "ecm_tracker_udp.h"
#include "ecm_tracker_tcp.h"
#include "ecm_db.h"
#include "ecm_classifier_default.h"
#include "ecm_front_end_common.h"

/*
 * Magic numbers
 */
#define ECM_CLASSIFIER_DEFAULT_INTERNAL_INSTANCE_MAGIC 0x8761
#define ECM_CLASSIFIER_DEFAULT_STATE_FILE_INSTANCE_MAGIC 0x3321

/*
 * struct ecm_classifier_default_internal_instance
 * 	State to allow tracking of dynamic priority for a connection
 */
struct ecm_classifier_default_internal_instance {
	struct ecm_classifier_default_instance base;		/* Base type */

	uint32_t ci_serial;					/* RO: Serial of the connection */
	int protocol;						/* RO: Protocol of the connection */

	struct ecm_classifier_process_response process_response;
								/* Last process response computed */

	ecm_db_timer_group_t timer_group;			/* The timer group the connection should be in based on state */

	ecm_tracker_sender_type_t ingress_sender;		/* RO: Which sender is sending ingress data */
	ecm_tracker_sender_type_t egress_sender;		/* RO: Which sender is sending egress data */

	struct ecm_tracker_instance *ti;			/* RO: Tracker used for state and timer group checking. Pointer will not change so safe to access outside of lock. */
	bool packet_seen[ECM_CONN_DIR_MAX];                     /* Per-direction packet seen flag */
	int refs;						/* Integer to trap we never go negative */
#if (DEBUG_LEVEL > 0)
	uint16_t magic;
#endif
};

static  DEFINE_SPINLOCK(ecm_classifier_default_lock);			/* Concurrency control SMP access */
static int ecm_classifier_default_count = 0;			/* Tracks number of instances allocated */

/*
 * Operational control
 */
static ecm_classifier_acceleration_mode_t ecm_classifier_default_accel_mode = ECM_CLASSIFIER_ACCELERATION_MODE_ACCEL;
								/* Cause connections whose hosts are both on-link to be accelerated */
static int ecm_classifier_default_enabled = 1;		/* When disabled the qos algorithm will not be applied to skb's */

/*
 * Management thread control
 */
static bool ecm_classifier_default_terminate_pending = false;	/* True when the user wants us to terminate */

/*
 * Character device stuff - used to communicate status back to user space
 */
#define ECM_CLASSIFIER_DEFAULT_STATE_FILE_BUFFER_SIZE 1024
struct ecm_classifier_default_state_file_instance {
	struct ecm_classifier_default_internal_instance *cdii;
	bool doc_start_written;
	bool doc_end_written;
	char msg_buffer[ECM_CLASSIFIER_DEFAULT_STATE_FILE_BUFFER_SIZE];	/* Used to hold the current state message being output */
	char *msgp;							/* Points into the msg buffer as we output it piece by piece */
	int msg_len;							/* Length of the buffer still to be written out */
#if (DEBUG_LEVEL > 0)
	uint16_t magic;
#endif
};
static struct dentry *ecm_classifier_default_dentry;		/* Debugfs dentry object */

/*
 * _ecm_classifier_default_ref()
 *	Ref
 */
static void _ecm_classifier_default_ref(struct ecm_classifier_default_internal_instance *cdii)
{
	cdii->refs++;
	DEBUG_TRACE("%px: cdii ref %d\n", cdii, cdii->refs);
	DEBUG_ASSERT(cdii->refs > 0, "%px: ref wrap\n", cdii);
}

/*
 * ecm_classifier_default_ref()
 *	Ref
 */
static void ecm_classifier_default_ref(struct ecm_classifier_instance *ci)
{
	struct ecm_classifier_default_internal_instance *cdii;
	cdii = (struct ecm_classifier_default_internal_instance *)ci;

	DEBUG_CHECK_MAGIC(cdii, ECM_CLASSIFIER_DEFAULT_INTERNAL_INSTANCE_MAGIC, "%px: magic failed", cdii);
	spin_lock_bh(&ecm_classifier_default_lock);
	_ecm_classifier_default_ref(cdii);
	spin_unlock_bh(&ecm_classifier_default_lock);
}

/*
 * ecm_classifier_default_deref()
 *	Deref
 */
static int ecm_classifier_default_deref(struct ecm_classifier_instance *ci)
{
	struct ecm_classifier_default_internal_instance *cdii;
	cdii = (struct ecm_classifier_default_internal_instance *)ci;

	DEBUG_CHECK_MAGIC(cdii, ECM_CLASSIFIER_DEFAULT_INTERNAL_INSTANCE_MAGIC, "%px: magic failed", cdii);
	spin_lock_bh(&ecm_classifier_default_lock);
	cdii->refs--;
	DEBUG_ASSERT(cdii->refs >= 0, "%px: refs wrapped\n", cdii);
	DEBUG_TRACE("%px: Default classifier deref %d\n", cdii, cdii->refs);
	if (cdii->refs) {
		int refs = cdii->refs;
		spin_unlock_bh(&ecm_classifier_default_lock);
		return refs;
	}

	/*
	 * Object to be destroyed
	 */
	ecm_classifier_default_count--;
	DEBUG_ASSERT(ecm_classifier_default_count >= 0, "%px: ecm_classifier_default_count wrap\n", cdii);

	spin_unlock_bh(&ecm_classifier_default_lock);

	/*
	 * Release our tracker
	 */
	cdii->ti->deref(cdii->ti);

	/*
	 * Final
	 */
	DEBUG_INFO("%px: Final default classifier instance\n", cdii);
	kfree(cdii);

	return 0;
}

/*
 * ecm_classifier_default_ready_for_accel()
 *	Checks if the connection is ready for the acceleration.
 *
 * This function is called, if the acceleration delay feature is enabled.
 */
static bool ecm_classifier_default_ready_for_accel(
			struct ecm_classifier_default_internal_instance *cdii,
			ecm_tracker_sender_type_t sender)
{
	uint64_t slow_pkts;
	struct ecm_db_connection_instance *ci;
	struct ecm_front_end_connection_instance *feci;

	/*
	 * We delay the acceleration for TCP and UDP protocols only.
	 * Non-ported protocols mostly the outer connections of the flows.
	 * Acceleration delay is required to inspect the packets and
	 * the outer connections are generally do not need inspection.
	 */
	if (cdii->protocol != IPPROTO_TCP && cdii->protocol != IPPROTO_UDP) {
		DEBUG_TRACE("%px: Accel delay is not enabled for protocol: %d\n", cdii, cdii->protocol);
		return true;
	}

	/*
	 * Delay forever until seeing the reply packet.
	 */
	if (ecm_classifier_accel_delay_pkts == 1) {
		DEBUG_INFO("%px: Checking both direction traffic\n", cdii);

		/*
		 * Set the flow and return direction packet seen flags.
		 */
		if (sender == ECM_TRACKER_SENDER_TYPE_SRC) {
			cdii->packet_seen[ECM_CONN_DIR_FLOW] = true;
		} else {
			cdii->packet_seen[ECM_CONN_DIR_RETURN] = true;
		}

		/*
		 * Check if both direction traffic is seen.
		 */
		return cdii->packet_seen[ECM_CONN_DIR_FLOW] && cdii->packet_seen[ECM_CONN_DIR_RETURN];
	}

	/*
	 * Delay the acceleration until we see <N> number of packets.
	 * ecm_classifier_accel_delay_pkts = <N>
	 */
	ci = ecm_db_connection_serial_find_and_ref(cdii->ci_serial);
	if (!ci) {
		DEBUG_TRACE("%px: No ci found for %u\n", cdii, cdii->ci_serial);
		return false;
	}

	/*
	 * Get the packet count we have seen in the slow path so far.
	 */
	feci = ecm_db_connection_front_end_get_and_ref(ci);
	slow_pkts = ecm_front_end_get_slow_packet_count(feci);
	ecm_front_end_connection_deref(feci);
	ecm_db_connection_deref(ci);

	/*
	 * Check if we have seen slow path packets as the predefined count.
	 */
	if (slow_pkts < ecm_classifier_accel_delay_pkts) {
		DEBUG_TRACE("%px: delay the acceleration: slow packets: %llu default delay packet count: %d\n",
			    cdii, slow_pkts, ecm_classifier_accel_delay_pkts);

		/*
		 * We haven't reached the slow path packet limit.
		 * We can wait more to accelerate the connection.
		 */
		return false;
	}

	/*
	 * We waited enough time for the acceleration, we can allow it now.
	 */
	DEBUG_INFO("%px: Let the flow accel, waited enough packet\n", cdii);
	return true;
}

/*
 * ecm_classifier_default_process()
 *	Process the flow for acceleration decision.
 */
static void ecm_classifier_default_process(struct ecm_classifier_instance *aci, ecm_tracker_sender_type_t sender,
					   struct ecm_tracker_ip_header *ip_hdr, struct sk_buff *skb,
					   struct ecm_classifier_process_response *process_response)
{
	struct ecm_tracker_instance *ti;
	ecm_tracker_sender_state_t from_state;
	ecm_tracker_sender_state_t to_state;
	ecm_tracker_connection_state_t prevailing_state;
	ecm_db_timer_group_t tg;
	struct ecm_classifier_default_internal_instance *cdii = (struct ecm_classifier_default_internal_instance *)aci;
	struct nf_conn *ct;
	enum ip_conntrack_info ctinfo;

	DEBUG_CHECK_MAGIC(cdii, ECM_CLASSIFIER_DEFAULT_INTERNAL_INSTANCE_MAGIC, "%px: invalid state magic\n", cdii);

	/*
	 * Get qos result and accel mode
	 * Default classifier is rarely disabled.
	 */
	if (unlikely(!ecm_classifier_default_enabled)) {
		/*
		 * Still relevant but have no actions that need processing
		 */
		spin_lock_bh(&ecm_classifier_default_lock);
		cdii->process_response.process_actions = 0;
		*process_response = cdii->process_response;
		spin_unlock_bh(&ecm_classifier_default_lock);
		return;
	}

	/*
	 * Update connection state
	 * Compute the timer group this connection should be in.
	 * For this we need the tracker and the state to be updated.
	 * NOTE: Tracker does not need to be ref'd it will exist for as long as this default classifier instance does
	 * which is at least for the duration of this call.
	 */
	ti = cdii->ti;
	ti->state_update(ti, sender, ip_hdr, skb);
	ti->state_get(ti, &from_state, &to_state, &prevailing_state, &tg);
	spin_lock_bh(&ecm_classifier_default_lock);
	if (unlikely(cdii->timer_group != tg)) {
		/*
		 * Timer group has changed
		 */
		cdii->process_response.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_TIMER_GROUP;
		cdii->process_response.timer_group = tg;

		/*
		 * Record for future change comparisons
		 */
		DEBUG_INFO("%px: timer group changed from %d to %d\n", cdii, cdii->timer_group, tg);
		cdii->timer_group = tg;
	}

	/*
	 * Don't care?
	 */
	if (ecm_classifier_default_accel_mode == ECM_CLASSIFIER_ACCELERATION_MODE_DONT_CARE) {
		cdii->process_response.process_actions &= ~ECM_CLASSIFIER_PROCESS_ACTION_ACCEL_MODE;
		*process_response = cdii->process_response;
		spin_unlock_bh(&ecm_classifier_default_lock);
		return;
	}
	spin_unlock_bh(&ecm_classifier_default_lock);

	/*
	 * Handle non-TCP case
	 */
	if (cdii->protocol != IPPROTO_TCP) {
		if (unlikely(prevailing_state != ECM_TRACKER_CONNECTION_STATE_ESTABLISHED)) {
			DEBUG_INFO("%px: Protocol: %d is non-TCP, prevailing_state: %d\n", cdii, cdii->protocol, prevailing_state);
			spin_lock_bh(&ecm_classifier_default_lock);
			goto accel_no;
		}
		DEBUG_INFO("%px: Protocol: %d is non-TCP, prevailing_state ESTABLISHED\n", cdii, cdii->protocol);
		goto check_delay;
	}

	/*
	 * Check the TCP connection state, when the ct is NULL.
	 * ct valid case was already checked in the ecm_nss{sfe}_ported_ipv4{6}_process functions.
	 * If we are not established then we deny acceleration.
	 */
	ct = nf_ct_get(skb, &ctinfo);
	if (!ct) {
		DEBUG_TRACE("%px: No Conntrack found for packet, using ECM tracker state\n", cdii);

		if (unlikely(prevailing_state == ECM_TRACKER_CONNECTION_STATE_ESTABLISHED)) {
			DEBUG_INFO("%px: TCP prevailing_state ESTABLISHED\n", cdii);
			goto check_delay;
		}

		DEBUG_INFO("%px: TCP prevailing_state: %d\n", cdii, prevailing_state);

		spin_lock_bh(&ecm_classifier_default_lock);
		if ((prevailing_state == ECM_TRACKER_CONNECTION_STATE_FAULT) || (prevailing_state == ECM_TRACKER_CONNECTION_STATE_CLOSED)) {
			cdii->process_response.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_TIMER_GROUP_NO_TOUCH;
		}
		goto accel_no;
	}

	/*
	* If the connection is shutting down do not manage it.
	* state can not be SYN_SENT, SYN_RECV because connection is assured
	* Not managed states: FIN_WAIT, CLOSE_WAIT, LAST_ACK, TIME_WAIT, CLOSE.
	*/
	spin_lock_bh(&ct->lock);
	if (ct->proto.tcp.state != TCP_CONNTRACK_ESTABLISHED) {
		spin_unlock_bh(&ct->lock);
		DEBUG_TRACE("%px: Connection in termination state %#X\n", ct, ct->proto.tcp.state);
		spin_lock_bh(&ecm_classifier_default_lock);
		goto accel_no;
	}
	spin_unlock_bh(&ct->lock);

check_delay:
	/*
	 * Should we delay the acceleration?
	 */
	if (ecm_classifier_accel_delay_pkts) {
		if (!ecm_classifier_default_ready_for_accel(cdii, sender)) {
			DEBUG_INFO("%px: connection is not ready for accel\n", cdii);
			spin_lock_bh(&ecm_classifier_default_lock);
			goto accel_no;
		}
		DEBUG_INFO("%px: connection is ready for accel\n", cdii);
	}

	/*
	 * Return the process response
	 */
	spin_lock_bh(&ecm_classifier_default_lock);
	cdii->process_response.accel_mode = ecm_classifier_default_accel_mode;
	cdii->process_response.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_ACCEL_MODE;
	*process_response = cdii->process_response;
	spin_unlock_bh(&ecm_classifier_default_lock);
	return;

accel_no:
	cdii->process_response.accel_mode = ECM_CLASSIFIER_ACCELERATION_MODE_NO;
	cdii->process_response.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_ACCEL_MODE;
	*process_response = cdii->process_response;
	spin_unlock_bh(&ecm_classifier_default_lock);
}

/*
 * ecm_classifier_default_type_get()
 *	Get type of classifier this is
 */
static ecm_classifier_type_t ecm_classifier_default_type_get(struct ecm_classifier_instance *aci)
{
	struct ecm_classifier_default_internal_instance *cdii;
	cdii = (struct ecm_classifier_default_internal_instance *)aci;

	DEBUG_CHECK_MAGIC(cdii, ECM_CLASSIFIER_DEFAULT_INTERNAL_INSTANCE_MAGIC, "%px: magic failed", cdii);
	return ECM_CLASSIFIER_TYPE_DEFAULT;
}

/*
 * ecm_classifier_default_reclassify_allowed()
 *	Get whether reclassification is allowed
 */
static bool ecm_classifier_default_reclassify_allowed(struct ecm_classifier_instance *aci)
{
	struct ecm_classifier_default_internal_instance *cdii;
	cdii = (struct ecm_classifier_default_internal_instance *)aci;

	DEBUG_CHECK_MAGIC(cdii, ECM_CLASSIFIER_DEFAULT_INTERNAL_INSTANCE_MAGIC, "%px: magic failed", cdii);
	return true;
}

/*
 * ecm_classifier_default_reclassify()
 *	Reclassify
 */
static void ecm_classifier_default_reclassify(struct ecm_classifier_instance *aci)
{
	struct ecm_classifier_default_internal_instance *cdii;
	cdii = (struct ecm_classifier_default_internal_instance *)aci;
	DEBUG_CHECK_MAGIC(cdii, ECM_CLASSIFIER_DEFAULT_INTERNAL_INSTANCE_MAGIC, "%px: magic failed", cdii);
}

/*
 * ecm_classifier_default_last_process_response_get()
 *	Get result code returned by the last process call
 */
static void ecm_classifier_default_last_process_response_get(struct ecm_classifier_instance *aci,
							struct ecm_classifier_process_response *process_response)
{
	struct ecm_classifier_default_internal_instance *cdii;
	cdii = (struct ecm_classifier_default_internal_instance *)aci;
	DEBUG_CHECK_MAGIC(cdii, ECM_CLASSIFIER_DEFAULT_INTERNAL_INSTANCE_MAGIC, "%px: magic failed", cdii);

	spin_lock_bh(&ecm_classifier_default_lock);
	*process_response = cdii->process_response;
	spin_unlock_bh(&ecm_classifier_default_lock);
}

/*
 * ecm_classifier_default_sync_to_v4()
 *	Front end is pushing accel engine state to us
 */
static void ecm_classifier_default_sync_to_v4(struct ecm_classifier_instance *aci, struct ecm_classifier_rule_sync *sync)
{
	struct ecm_classifier_default_internal_instance *cdii __attribute__((unused));

	cdii = (struct ecm_classifier_default_internal_instance *)aci;
	DEBUG_CHECK_MAGIC(cdii, ECM_CLASSIFIER_DEFAULT_INTERNAL_INSTANCE_MAGIC, "%px: magic failed", cdii);
}

/*
 * ecm_classifier_default_sync_from_v4()
 *	Front end is retrieving accel engine state from us
 */
static void ecm_classifier_default_sync_from_v4(struct ecm_classifier_instance *aci, struct ecm_classifier_rule_create *ecrc)
{
	struct ecm_classifier_default_internal_instance *cdii __attribute__((unused));

	cdii = (struct ecm_classifier_default_internal_instance *)aci;
	DEBUG_CHECK_MAGIC(cdii, ECM_CLASSIFIER_DEFAULT_INTERNAL_INSTANCE_MAGIC, "%px: magic failed", cdii);
}

/*
 * ecm_classifier_default_sync_to_v6()
 *	Front end is pushing accel engine state to us
 */
static void ecm_classifier_default_sync_to_v6(struct ecm_classifier_instance *aci, struct ecm_classifier_rule_sync *sync)
{
	struct ecm_classifier_default_internal_instance *cdii __attribute__((unused));

	cdii = (struct ecm_classifier_default_internal_instance *)aci;
	DEBUG_CHECK_MAGIC(cdii, ECM_CLASSIFIER_DEFAULT_INTERNAL_INSTANCE_MAGIC, "%px: magic failed", cdii);
}

/*
 * ecm_classifier_default_sync_from_v6()
 *	Front end is retrieving accel engine state from us
 */
static void ecm_classifier_default_sync_from_v6(struct ecm_classifier_instance *aci, struct ecm_classifier_rule_create *ecrc)
{
	struct ecm_classifier_default_internal_instance *cdii __attribute__((unused));

	cdii = (struct ecm_classifier_default_internal_instance *)aci;
	DEBUG_CHECK_MAGIC(cdii, ECM_CLASSIFIER_DEFAULT_INTERNAL_INSTANCE_MAGIC, "%px: magic failed", cdii);
}

/*
 * ecm_classifier_tracker_get_and_ref()
 *	Obtain default classifiers tracker (usually for state tracking for the connection as it always exists for the connection)
 */
static struct ecm_tracker_instance *ecm_classifier_tracker_get_and_ref(struct ecm_classifier_default_instance *dci)
{
	struct ecm_classifier_default_internal_instance *cdii;
	struct ecm_tracker_instance *ti;

	cdii = (struct ecm_classifier_default_internal_instance *)dci;
	DEBUG_CHECK_MAGIC(cdii, ECM_CLASSIFIER_DEFAULT_INTERNAL_INSTANCE_MAGIC, "%px: magic failed", cdii);

	ti = cdii->ti;
	ti->ref(ti);
	return ti;
}

#ifdef ECM_STATE_OUTPUT_ENABLE
/*
 * ecm_classifier_default_state_get()
 *	Return state
 */
static int ecm_classifier_default_state_get(struct ecm_classifier_instance *ci, struct ecm_state_file_instance *sfi)
{
	int result;
	struct ecm_classifier_default_internal_instance *cdii;
	struct ecm_classifier_process_response process_response;
	ecm_db_timer_group_t timer_group;
	ecm_tracker_sender_type_t ingress_sender;
	ecm_tracker_sender_type_t egress_sender;

	cdii = (struct ecm_classifier_default_internal_instance *)ci;
	DEBUG_CHECK_MAGIC(cdii, ECM_CLASSIFIER_DEFAULT_INTERNAL_INSTANCE_MAGIC, "%px: magic failed", cdii);

	if ((result = ecm_state_prefix_add(sfi, "default"))) {
		return result;
	}

	spin_lock_bh(&ecm_classifier_default_lock);
	egress_sender = cdii->egress_sender;
	ingress_sender = cdii->ingress_sender;
	timer_group = cdii->timer_group;
	process_response = cdii->process_response;
	spin_unlock_bh(&ecm_classifier_default_lock);

	if ((result = ecm_state_write(sfi, "ingress_sender", "%d", ingress_sender))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "egress_sender", "%d", egress_sender))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "timer_group", "%d", timer_group))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "accel_delay_pkt_default", "%d", ecm_classifier_accel_delay_pkts))) {
		return result;
	}

	/*
	 * Output our last process response
	 */
	if ((result = ecm_classifier_process_response_state_get(sfi, &process_response))) {
		return result;
	}

	if ((result = ecm_state_prefix_add(sfi, "trackers"))) {
		return result;
	}

	/*
	 * Output our tracker state
	 */
	if ((result = cdii->ti->state_text_get(cdii->ti, sfi))) {
		return result;
	}

	if ((result = ecm_state_prefix_remove(sfi))) {
		return result;
	}

	return ecm_state_prefix_remove(sfi);
}
#endif

/*
 * ecm_classifier_default_instance_alloc()
 *	Allocate an instance of the default classifier
 */
struct ecm_classifier_default_instance *ecm_classifier_default_instance_alloc(struct ecm_db_connection_instance *ci, int protocol, ecm_db_direction_t dir, int from_port, int to_port)
{
	struct ecm_classifier_default_internal_instance *cdii;
	struct ecm_classifier_default_instance *cdi;

	/*
	 * Allocate the instance
	 */
	cdii = (struct ecm_classifier_default_internal_instance *)kzalloc(sizeof(struct ecm_classifier_default_internal_instance), GFP_ATOMIC | __GFP_NOWARN);
	if (!cdii) {
		DEBUG_WARN("Failed to allocate default instance\n");
		return NULL;
	}

	/*
	 * Allocate a tracker for state etc.
	 */
	if (protocol == IPPROTO_TCP) {
		DEBUG_TRACE("%px: Alloc tracker for TCP connection: %px\n", cdii, ci);
		cdii->ti = (struct ecm_tracker_instance *)ecm_tracker_tcp_alloc();
		if (!cdii->ti) {
			DEBUG_WARN("%px: Failed to alloc tracker\n", cdii);
			kfree(cdii);
			return NULL;
		}
		ecm_tracker_tcp_init((struct ecm_tracker_tcp_instance *)cdii->ti, ECM_TRACKER_CONNECTION_TRACKING_LIMIT_DEFAULT, 1500, 1500);
	} else if (protocol == IPPROTO_UDP) {
		DEBUG_TRACE("%px: Alloc tracker for UDP connection: %px\n", cdii, ci);
		cdii->ti = (struct ecm_tracker_instance *)ecm_tracker_udp_alloc();
		if (!cdii->ti) {
			DEBUG_WARN("%px: Failed to alloc tracker\n", cdii);
			kfree(cdii);
			return NULL;
		}
		ecm_tracker_udp_init((struct ecm_tracker_udp_instance *)cdii->ti, ECM_TRACKER_CONNECTION_TRACKING_LIMIT_DEFAULT, from_port, to_port);
	} else {
		DEBUG_TRACE("%px: Alloc tracker for non-ported connection: %px\n", cdii, ci);
		cdii->ti = (struct ecm_tracker_instance *)ecm_tracker_datagram_alloc();
		if (!cdii->ti) {
			DEBUG_WARN("%px: Failed to alloc tracker\n", cdii);
			kfree(cdii);
			return NULL;
		}
		ecm_tracker_datagram_init((struct ecm_tracker_datagram_instance *)cdii->ti, ECM_TRACKER_CONNECTION_TRACKING_LIMIT_DEFAULT);
	}

	DEBUG_SET_MAGIC(cdii, ECM_CLASSIFIER_DEFAULT_INTERNAL_INSTANCE_MAGIC);
	cdii->refs = 1;
	cdii->ci_serial = ecm_db_connection_serial_get(ci);
	cdii->protocol = protocol;

	/*
	 * We are always relevant to the connection
	 */
	cdii->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_YES;

	/*
	 * Using the connection direction identify egress and ingress host addresses
	 */
	if (dir == ECM_DB_DIRECTION_INGRESS_NAT) {
		cdii->ingress_sender = ECM_TRACKER_SENDER_TYPE_SRC;
		cdii->egress_sender = ECM_TRACKER_SENDER_TYPE_DEST;
	} else {
		cdii->egress_sender = ECM_TRACKER_SENDER_TYPE_SRC;
		cdii->ingress_sender = ECM_TRACKER_SENDER_TYPE_DEST;
	}
	DEBUG_TRACE("%px: Ingress sender = %d egress sender = %d\n", cdii, cdii->ingress_sender, cdii->egress_sender);

	/*
	 * Methods specific to the default classifier
	 */
	cdi = (struct ecm_classifier_default_instance *)cdii;
	cdi->tracker_get_and_ref = ecm_classifier_tracker_get_and_ref;

	/*
	 * Methods generic to all classifiers.
	 */
	cdi->base.process = ecm_classifier_default_process;
	cdi->base.sync_from_v4 = ecm_classifier_default_sync_from_v4;
	cdi->base.sync_to_v4 = ecm_classifier_default_sync_to_v4;
	cdi->base.sync_from_v6 = ecm_classifier_default_sync_from_v6;
	cdi->base.sync_to_v6 = ecm_classifier_default_sync_to_v6;
	cdi->base.type_get = ecm_classifier_default_type_get;
	cdi->base.reclassify_allowed = ecm_classifier_default_reclassify_allowed;
	cdi->base.reclassify = ecm_classifier_default_reclassify;
	cdi->base.last_process_response_get = ecm_classifier_default_last_process_response_get;
#ifdef ECM_STATE_OUTPUT_ENABLE
	cdi->base.state_get = ecm_classifier_default_state_get;
#endif
	cdi->base.ref = ecm_classifier_default_ref;
	cdi->base.deref = ecm_classifier_default_deref;

	spin_lock_bh(&ecm_classifier_default_lock);

	/*
	 * Final check if we are pending termination
	 */
	if (ecm_classifier_default_terminate_pending) {
		spin_unlock_bh(&ecm_classifier_default_lock);
		DEBUG_INFO("%px: Terminating\n", ci);
		cdii->ti->deref(cdii->ti);
		kfree(cdii);
		return NULL;
	}

	/*
	 * Increment stats
	 */
	ecm_classifier_default_count++;
	DEBUG_ASSERT(ecm_classifier_default_count > 0, "%px: ecm_classifier_default_count wrap\n", cdii);
	spin_unlock_bh(&ecm_classifier_default_lock);

	DEBUG_INFO("Default classifier instance alloc: %px\n", cdii);
	return cdi;
}
EXPORT_SYMBOL(ecm_classifier_default_instance_alloc);

/*
 * ecm_classifier_default_init()
 */
int ecm_classifier_default_init(struct dentry *dentry)
{
	DEBUG_INFO("Default classifier Module init\n");

	DEBUG_ASSERT(ECM_CLASSIFIER_TYPE_DEFAULT == 0, "DO NOT CHANGE DEFAULT PRIORITY");

	ecm_classifier_default_dentry = debugfs_create_dir("ecm_classifier_default", dentry);
	if (!ecm_classifier_default_dentry) {
		DEBUG_ERROR("Failed to create ecm default classifier directory in debugfs\n");
		return -1;
	}

	if (!ecm_debugfs_create_u32("enabled", S_IRUGO | S_IWUSR, ecm_classifier_default_dentry,
					(u32 *)&ecm_classifier_default_enabled)) {
		DEBUG_ERROR("Failed to create ecm deafult classifier enabled file in debugfs\n");
		debugfs_remove_recursive(ecm_classifier_default_dentry);
		return -1;
	}

	if (!ecm_debugfs_create_u32("accel_mode", S_IRUGO | S_IWUSR, ecm_classifier_default_dentry,
					(u32 *)&ecm_classifier_default_accel_mode)) {
		DEBUG_ERROR("Failed to create ecm deafult classifier accel_mode file in debugfs\n");
		debugfs_remove_recursive(ecm_classifier_default_dentry);
		return -1;
	}

	if (!ecm_debugfs_create_u32("accel_delay_pkts", S_IRUGO | S_IWUSR, ecm_classifier_default_dentry,
					(u32 *)&ecm_classifier_accel_delay_pkts)) {
		DEBUG_ERROR("Failed to create accel delay packet counts in debugfs\n");
		debugfs_remove_recursive(ecm_classifier_default_dentry);
		return -1;
	}

	return 0;
}
EXPORT_SYMBOL(ecm_classifier_default_init);

/*
 * ecm_classifier_default_exit()
 */
void ecm_classifier_default_exit(void)
{
	DEBUG_INFO("Default classifier Module exit\n");
	spin_lock_bh(&ecm_classifier_default_lock);
	ecm_classifier_default_terminate_pending = true;
	spin_unlock_bh(&ecm_classifier_default_lock);

	/*
	 * Remove the debugfs files recursively.
	 */
	if (ecm_classifier_default_dentry) {
		debugfs_remove_recursive(ecm_classifier_default_dentry);
	}
}
EXPORT_SYMBOL(ecm_classifier_default_exit);
