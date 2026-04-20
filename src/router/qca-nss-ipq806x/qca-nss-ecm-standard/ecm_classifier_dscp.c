/*
 **************************************************************************
 * Copyright (c) 2014-2016, 2019-2021 The Linux Foundation. All rights reserved.
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
#include "ecm_front_end_common.h"

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
	bool packet_seen[ECM_CONN_DIR_MAX];			/* Per-direction packet seen flag */
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

	DEBUG_CHECK_MAGIC(cdscpi, ECM_CLASSIFIER_DSCP_INSTANCE_MAGIC, "%px: magic failed\n", cdscpi);
	spin_lock_bh(&ecm_classifier_dscp_lock);
	cdscpi->refs++;
	DEBUG_TRACE("%px: cdscpi ref %d\n", cdscpi, cdscpi->refs);
	DEBUG_ASSERT(cdscpi->refs > 0, "%px: ref wrap\n", cdscpi);
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

	DEBUG_CHECK_MAGIC(cdscpi, ECM_CLASSIFIER_DSCP_INSTANCE_MAGIC, "%px: magic failed\n", cdscpi);
	spin_lock_bh(&ecm_classifier_dscp_lock);
	cdscpi->refs--;
	DEBUG_ASSERT(cdscpi->refs >= 0, "%px: refs wrapped\n", cdscpi);
	DEBUG_TRACE("%px: DSCP classifier deref %d\n", cdscpi, cdscpi->refs);
	if (cdscpi->refs) {
		int refs = cdscpi->refs;
		spin_unlock_bh(&ecm_classifier_dscp_lock);
		return refs;
	}

	/*
	 * Object to be destroyed
	 */
	ecm_classifier_dscp_count--;
	DEBUG_ASSERT(ecm_classifier_dscp_count >= 0, "%px: ecm_classifier_dscp_count wrap\n", cdscpi);

	/*
	 * UnLink the instance from our list
	 */
	if (cdscpi->next) {
		cdscpi->next->prev = cdscpi->prev;
	}
	if (cdscpi->prev) {
		cdscpi->prev->next = cdscpi->next;
	} else {
		DEBUG_ASSERT(ecm_classifier_dscp_instances == cdscpi, "%px: list bad %px\n", cdscpi, ecm_classifier_dscp_instances);
		ecm_classifier_dscp_instances = cdscpi->next;
	}
	cdscpi->next = NULL;
	cdscpi->prev = NULL;
	spin_unlock_bh(&ecm_classifier_dscp_lock);

	/*
	 * Final
	 */
	DEBUG_INFO("%px: Final DSCP classifier instance\n", cdscpi);
	kfree(cdscpi);

	return 0;
}

/*
 * ecm_classifier_dscp_is_bidi_packet_seen()
 *	Return true if both direction packets are seen.
 */
static inline bool ecm_classifier_dscp_is_bidi_packet_seen(struct ecm_classifier_dscp_instance *cdscpi)
{
	return ((cdscpi->packet_seen[ECM_CONN_DIR_FLOW] == true) && (cdscpi->packet_seen[ECM_CONN_DIR_RETURN] == true));
}

/*
 * ecm_classifier_dscp_fill_info()
 *	Save the QoS and DSCP values in the classifier instance.
 */
static void ecm_classifier_dscp_fill_info(struct ecm_classifier_dscp_instance *cdscpi,
					 ecm_tracker_sender_type_t sender,
					 struct ecm_tracker_ip_header *ip_hdr,
					 struct sk_buff *skb)
{
	if (sender == ECM_TRACKER_SENDER_TYPE_SRC) {
		cdscpi->process_response.flow_qos_tag = skb->priority;
		cdscpi->process_response.flow_mark = skb->mark;
		cdscpi->process_response.flow_dscp = ip_hdr->ds >> XT_DSCP_SHIFT;
		cdscpi->packet_seen[ECM_CONN_DIR_FLOW] = true;
	} else {
		cdscpi->process_response.return_qos_tag = skb->priority;
		cdscpi->process_response.return_mark = skb->mark;
		cdscpi->process_response.return_dscp = ip_hdr->ds >> XT_DSCP_SHIFT;
		cdscpi->packet_seen[ECM_CONN_DIR_RETURN] = true;
	}
}

/*
 * ecm_classifier_dscp_update()
 *	Called from the frontend files to update the classifier instance.
 */
void ecm_classifier_dscp_update(struct ecm_classifier_instance *aci, enum ecm_rule_update_type type, void *arg)
{
	struct nf_ct_dscpremark_ext *dscpcte;
	struct nf_conn *ct = (struct nf_conn *)arg;
	struct ecm_classifier_dscp_instance *cdscpi = (struct ecm_classifier_dscp_instance *)aci;
	DEBUG_CHECK_MAGIC(cdscpi, ECM_CLASSIFIER_DSCP_INSTANCE_MAGIC, "%px: magic failed\n", cdscpi);

	if (type != ECM_RULE_UPDATE_TYPE_CONNMARK) {
		DEBUG_WARN("%px: unsupported update type: %d\n", aci, type);
		return;
	}

	/*
	 * Since we set the mark values from the DSCP extension, we need to update this
	 * extension as well. Because in case of a rule flush, the updated value should be read.
	 * UDP flows should also overwrite the classifier's response mark field which is set with the
	 * skb->mark.
	 */
	spin_lock_bh(&ct->lock);
	dscpcte = nf_ct_dscpremark_ext_find(ct);
	if (!dscpcte) {
		spin_unlock_bh(&ct->lock);
		DEBUG_WARN("%px: no dscp extension\n", aci);
		return;
	}
	dscpcte->flow_mark = ct->mark;
	dscpcte->reply_mark = ct->mark;
	dscpcte->flow_set_flags |= NF_CT_DSCPREMARK_EXT_MARK;
	dscpcte->return_set_flags |= NF_CT_DSCPREMARK_EXT_MARK;
	spin_unlock_bh(&ct->lock);

	cdscpi->process_response.flow_mark = ct->mark;
	cdscpi->process_response.return_mark = ct->mark;
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
	bool dscp_marked = false;
	uint64_t slow_pkts;

	cdscpi = (struct ecm_classifier_dscp_instance *)aci;
	DEBUG_CHECK_MAGIC(cdscpi, ECM_CLASSIFIER_DSCP_INSTANCE_MAGIC, "%px: magic failed\n", cdscpi);

	/*
	 * Are we yet to decide if this instance is relevant to the connection?
	 */
	spin_lock_bh(&ecm_classifier_dscp_lock);
	relevance = cdscpi->process_response.relevance;

	/*
	 * Are we relevant?
	 */
	if (relevance == ECM_CLASSIFIER_RELEVANCE_NO) {
		*process_response = cdscpi->process_response;
		spin_unlock_bh(&ecm_classifier_dscp_lock);
		return;
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
		cdscpi->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_NO;
		*process_response = cdscpi->process_response;
		spin_unlock_bh(&ecm_classifier_dscp_lock);
		return;
	}
	spin_unlock_bh(&ecm_classifier_dscp_lock);

	/*
	 * Can we accelerate?
	 */
	ci = ecm_db_connection_serial_find_and_ref(cdscpi->ci_serial);
	if (!ci) {
		DEBUG_TRACE("%px: No ci found for %u\n", cdscpi, cdscpi->ci_serial);
		spin_lock_bh(&ecm_classifier_dscp_lock);
		cdscpi->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_NO;
		goto dscp_classifier_out;
	}

	feci = ecm_db_connection_front_end_get_and_ref(ci);
	accel_mode = ecm_front_end_connection_accel_state_get(feci);
	slow_pkts = ecm_front_end_get_slow_packet_count(feci);
	ecm_front_end_connection_deref(feci);
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
		DEBUG_WARN("%px: no conntrack found\n", cdscpi);
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
		DEBUG_WARN("%px: no DSCP conntrack extension found\n", cdscpi);
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
		DEBUG_TRACE("%px: DSCP remark extension is valid\n", cdscpi);
		dscp_marked = true;
	}
	spin_unlock_bh(&ct->lock);

	/*
	 * We are relevant to the connection
	 */
	became_relevant = ecm_db_time_get();

	spin_lock_bh(&ecm_classifier_dscp_lock);
	cdscpi->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_YES;
	cdscpi->process_response.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_ACCEL_MODE;
	cdscpi->process_response.accel_mode = ECM_CLASSIFIER_ACCELERATION_MODE_ACCEL;
	cdscpi->process_response.became_relevant = became_relevant;

	if (protocol == IPPROTO_TCP) {
		/*
		 * If DSCP conntrack extension is filled in the frontend, use those values
		 * instead of waiting both direction traffic again.
		 */
		if ((dscpcte->flow_set_flags == (NF_CT_DSCPREMARK_EXT_PRIO | NF_CT_DSCPREMARK_EXT_DSCP | NF_CT_DSCPREMARK_EXT_MARK))
			&& (dscpcte->return_set_flags == (NF_CT_DSCPREMARK_EXT_PRIO | NF_CT_DSCPREMARK_EXT_DSCP | NF_CT_DSCPREMARK_EXT_MARK))) {
			/*
			 * If sender and the conntrack info direction are consistent, fill the response field
			 * with the flow/return values as it is. Otherwise reverse the assignments.
			 */
			if (((sender == ECM_TRACKER_SENDER_TYPE_SRC) && (IP_CT_DIR_ORIGINAL == CTINFO2DIR(ctinfo))) ||
				((sender == ECM_TRACKER_SENDER_TYPE_DEST) && (IP_CT_DIR_REPLY == CTINFO2DIR(ctinfo)))) {
				cdscpi->process_response.flow_qos_tag = dscpcte->flow_priority;
				cdscpi->process_response.return_qos_tag = dscpcte->reply_priority;
				cdscpi->process_response.flow_mark = dscpcte->flow_mark;
				cdscpi->process_response.return_mark = dscpcte->reply_mark;
				cdscpi->process_response.flow_dscp = dscpcte->flow_dscp;
				cdscpi->process_response.return_dscp = dscpcte->reply_dscp;
			} else {
				cdscpi->process_response.flow_qos_tag = dscpcte->reply_priority;
				cdscpi->process_response.return_qos_tag = dscpcte->flow_priority;
				cdscpi->process_response.flow_mark = dscpcte->reply_mark;
				cdscpi->process_response.return_mark = dscpcte->flow_mark;
				cdscpi->process_response.flow_dscp = dscpcte->reply_dscp;
				cdscpi->process_response.return_dscp = dscpcte->flow_dscp;
			}
			DEBUG_TRACE("%px: DSCP extension is used to set the QoS values\n", cdscpi);
			goto done;
		}

		/*
		 * Stop the processing if both side packets are already seen.
		 * Above the process response is already set to allow the acceleration.
		 */
		if (ecm_classifier_dscp_is_bidi_packet_seen(cdscpi)) {
			DEBUG_TRACE("%px: TCP bi-di packets seen\n", cdscpi);
			goto done;
		}

		/*
		 * Store the QoS and DSCP info in the classifier instance and deny the
		 * acceleration if both side info is not yet available.
		 *
		 * This setting is a backup setting for the QoS values, in case the DSCP
		 * conntrack extension is not filled. This situation happens when the conntrack
		 * table is flushed by the user with the echo f > /proc/net/nf_conntrack command.
		 * After this flush, the conntarck entry and the ECM connection entry are destroyed,
		 * but the TCP connection remains established. When the next packet comes to ECM,
		 * since there is no TCP handshake, the DSCP conntarck extension is not set. So,
		 * we need to read these values from the packet's IP header.
		 */
		ecm_classifier_dscp_fill_info(cdscpi, sender, ip_hdr, skb);
		if (!ecm_classifier_dscp_is_bidi_packet_seen(cdscpi)) {
			DEBUG_TRACE("%px: TCP both side info is not yet picked\n", cdscpi);
			cdscpi->process_response.accel_mode = ECM_CLASSIFIER_ACCELERATION_MODE_NO;
			goto dscp_classifier_out;
		}
	} else {
		/*
		 * If the acceleration delay option is enabled, we will wait
		 * until seeing both side traffic.
		 *
		 * There are 2 options:
		 * Option 1: Wait forever until to see the reply direction traffic
		 * Option 2: Wait for seeing N number of packets. If we still don't see reply,
		 * set the uni-directional values.
		 */
		if (ecm_classifier_accel_delay_pkts) {
			/*
			 * Stop the processing if both side packets are already seen.
			 * Above the process response is already set to allow the
			 * acceleration.
			 */
			if (ecm_classifier_dscp_is_bidi_packet_seen(cdscpi)) {
				DEBUG_TRACE("%px: UDP bi-di packets seen\n", cdscpi);
				goto done;
			}

			/*
			 * Store the QoS and DSCP info in the classifier instance and allow the
			 * acceleration if both side info is not yet available.
			 */
			ecm_classifier_dscp_fill_info(cdscpi, sender, ip_hdr, skb);
			if (ecm_classifier_dscp_is_bidi_packet_seen(cdscpi)) {
				DEBUG_TRACE("%px: UDP both side info is picked\n", cdscpi);
				goto done;
			}

			/*
			 * Deny the acceleration if any of the below options holds true.
			 * For option 1, we wait forever
			 * For option 2, we wait until seeing ecm_classifier_accel_delay_pkts.
			 */
			if ((ecm_classifier_accel_delay_pkts == 1) || (slow_pkts < ecm_classifier_accel_delay_pkts)) {
				DEBUG_TRACE("%px: accel_delay_pkts: %d slow_pkts: %llu accel is not allowed yet\n",
						cdscpi, ecm_classifier_accel_delay_pkts, slow_pkts);
				cdscpi->process_response.accel_mode = ECM_CLASSIFIER_ACCELERATION_MODE_NO;
				goto dscp_classifier_out;
			}
		}

		/*
		 * If we didn't see both direction traffic during the acceleration
		 * delay time, we can allow the acceleration by setting the uni-directional
		 * values to both flow and return QoS and DSCP.
		 */
		if (sender == ECM_TRACKER_SENDER_TYPE_SRC) {
			cdscpi->process_response.flow_qos_tag = skb->priority;
			cdscpi->process_response.flow_mark = skb->mark;
			cdscpi->process_response.flow_dscp = ip_hdr->ds >> XT_DSCP_SHIFT;

			/*
			 * If UDP bi-di traffic is being run, it is possible that other direction's
			 * QoS and DSCP values are also set by the subsequent packets before we push
			 * the rule to NSS. So, let's update them, if they are not set.
			 */
			if (cdscpi->process_response.return_qos_tag == 0) {
				cdscpi->process_response.return_qos_tag = skb->priority;
			}

			if (cdscpi->process_response.return_mark == 0) {
				cdscpi->process_response.return_mark = skb->mark;
			}

			if (cdscpi->process_response.return_dscp == 0) {
				cdscpi->process_response.return_dscp = ip_hdr->ds >> XT_DSCP_SHIFT;
			}

		} else {
			cdscpi->process_response.return_qos_tag = skb->priority;
			cdscpi->process_response.return_mark = skb->mark;
			cdscpi->process_response.return_dscp = ip_hdr->ds >> XT_DSCP_SHIFT;

			/*
			 * If UDP bi-di traffic is being run, it is possible that other direction's
			 * QoS and DSCP values are also set by the subsequent packets before we push
			 * the rule to NSS. So, let's update them, if they are not set.
			 */
			if (cdscpi->process_response.flow_qos_tag == 0) {
				cdscpi->process_response.flow_qos_tag = skb->priority;
			}

			if (cdscpi->process_response.flow_mark == 0) {
				cdscpi->process_response.flow_mark = skb->mark;
			}

			if (cdscpi->process_response.flow_dscp == 0) {
				cdscpi->process_response.flow_dscp = ip_hdr->ds >> XT_DSCP_SHIFT;
			}
		}

		/*
		 * If the flow and return set flags are set for the MARK, we overwrite the mark field.
		 * These values are stored in the dscp extentension in the update callback.
		 */
		if ((dscpcte->flow_set_flags & NF_CT_DSCPREMARK_EXT_MARK) &&
				(dscpcte->return_set_flags & NF_CT_DSCPREMARK_EXT_MARK)) {
			cdscpi->process_response.flow_mark = dscpcte->flow_mark;
			cdscpi->process_response.return_mark = dscpcte->reply_mark;
		}
	}
done:
	cdscpi->process_response.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_QOS_TAG;
	cdscpi->process_response.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_MARK;

#ifdef ECM_CLASSIFIER_DSCP_IGS
	/*
	 * IGS qostag values in conntrack are stored as per the direction of the flow.
	 * But ECM always create an acceleration connection rule treating packet's source
	 * address as the source of the connection irrespective of the CT's direction.
	 * So, the IGS qostag values should be appropriately filled in ECM acceleration
	 * connection rule.
	 * Scenario's example: For WAN to LAN traffic for tunnel, the CT will be created from
	 * WAN to LAN but the CI will not be created as ECM will not get the packet in this
	 * direction. CI will be created for packet from LAN to WAN.
	 */
	cdscpi->process_response.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_IGS_QOS_TAG;
	if (((sender == ECM_TRACKER_SENDER_TYPE_SRC) && (IP_CT_DIR_ORIGINAL == CTINFO2DIR(ctinfo))) ||
		((sender == ECM_TRACKER_SENDER_TYPE_DEST) && (IP_CT_DIR_REPLY == CTINFO2DIR(ctinfo)))) {
		cdscpi->process_response.igs_flow_qos_tag = dscpcte->igs_flow_qos_tag;
		cdscpi->process_response.igs_return_qos_tag = dscpcte->igs_reply_qos_tag;
	} else {
		cdscpi->process_response.igs_return_qos_tag = dscpcte->igs_flow_qos_tag;
		cdscpi->process_response.igs_flow_qos_tag = dscpcte->igs_reply_qos_tag;
	}
#endif
	/*
	 * Check if we need to set DSCP
	 */
	if (dscp_marked) {
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
	DEBUG_CHECK_MAGIC(cdscpi, ECM_CLASSIFIER_DSCP_INSTANCE_MAGIC, "%px: magic failed", cdscpi);
}

/*
 * ecm_classifier_dscp_sync_from_v4()
 *	Front end is retrieving accel engine state from us
 */
static void ecm_classifier_dscp_sync_from_v4(struct ecm_classifier_instance *aci, struct ecm_classifier_rule_create *ecrc)
{
	struct ecm_classifier_dscp_instance *cdscpi __attribute__((unused));

	cdscpi = (struct ecm_classifier_dscp_instance *)aci;
	DEBUG_CHECK_MAGIC(cdscpi, ECM_CLASSIFIER_DSCP_INSTANCE_MAGIC, "%px: magic failed", cdscpi);
}

/*
 * ecm_classifier_dscp_sync_to_v6()
 *	Front end is pushing accel engine state to us
 */
static void ecm_classifier_dscp_sync_to_v6(struct ecm_classifier_instance *aci, struct ecm_classifier_rule_sync *sync)
{
	struct ecm_classifier_dscp_instance *cdscpi __attribute__((unused));

	cdscpi = (struct ecm_classifier_dscp_instance *)aci;
	DEBUG_CHECK_MAGIC(cdscpi, ECM_CLASSIFIER_DSCP_INSTANCE_MAGIC, "%px: magic failed", cdscpi);
}

/*
 * ecm_classifier_dscp_sync_from_v6()
 *	Front end is retrieving accel engine state from us
 */
static void ecm_classifier_dscp_sync_from_v6(struct ecm_classifier_instance *aci, struct ecm_classifier_rule_create *ecrc)
{
	struct ecm_classifier_dscp_instance *cdscpi __attribute__((unused));

	cdscpi = (struct ecm_classifier_dscp_instance *)aci;
	DEBUG_CHECK_MAGIC(cdscpi, ECM_CLASSIFIER_DSCP_INSTANCE_MAGIC, "%px: magic failed", cdscpi);
}

/*
 * ecm_classifier_dscp_type_get()
 *	Get type of classifier this is
 */
static ecm_classifier_type_t ecm_classifier_dscp_type_get(struct ecm_classifier_instance *ci)
{
	struct ecm_classifier_dscp_instance *cdscpi;
	cdscpi = (struct ecm_classifier_dscp_instance *)ci;

	DEBUG_CHECK_MAGIC(cdscpi, ECM_CLASSIFIER_DSCP_INSTANCE_MAGIC, "%px: magic failed\n", cdscpi);
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
	DEBUG_CHECK_MAGIC(cdscpi, ECM_CLASSIFIER_DSCP_INSTANCE_MAGIC, "%px: magic failed\n", cdscpi);

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
	DEBUG_CHECK_MAGIC(cdscpi, ECM_CLASSIFIER_DSCP_INSTANCE_MAGIC, "%px: magic failed\n", cdscpi);

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
	DEBUG_CHECK_MAGIC(cdscpi, ECM_CLASSIFIER_DSCP_INSTANCE_MAGIC, "%px: magic failed\n", cdscpi);

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
	DEBUG_CHECK_MAGIC(cdscpi, ECM_CLASSIFIER_DSCP_INSTANCE_MAGIC, "%px: magic failed", cdscpi);

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
	cdscpi->base.update = ecm_classifier_dscp_update;
	cdscpi->ci_serial = ecm_db_connection_serial_get(ci);
	cdscpi->process_response.process_actions = 0;
	cdscpi->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_MAYBE;

	spin_lock_bh(&ecm_classifier_dscp_lock);

	/*
	 * Final check if we are pending termination
	 */
	if (ecm_classifier_dscp_terminate_pending) {
		spin_unlock_bh(&ecm_classifier_dscp_lock);
		DEBUG_INFO("%px: Terminating\n", ci);
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
	DEBUG_ASSERT(ecm_classifier_dscp_count > 0, "%px: ecm_classifier_dscp_count wrap\n", cdscpi);
	spin_unlock_bh(&ecm_classifier_dscp_lock);

	DEBUG_INFO("DSCP instance alloc: %px\n", cdscpi);
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

	if (!ecm_debugfs_create_u32("enabled", S_IRUGO | S_IWUSR, ecm_classifier_dscp_dentry,
					(u32 *)&ecm_classifier_dscp_enabled)) {
		DEBUG_ERROR("Failed to create dscp enabled file in debugfs\n");
		debugfs_remove_recursive(ecm_classifier_dscp_dentry);
		return -1;
	}

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
