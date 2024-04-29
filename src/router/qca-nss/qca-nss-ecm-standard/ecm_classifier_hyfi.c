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
#include <linux/kthread.h>
#include <linux/debugfs.h>
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

#include <hyfi_ecm.h>
#include <hyfi_hash.h>

/*
 * Debug output levels
 * 0 = OFF
 * 1 = ASSERTS / ERRORS
 * 2 = 1 + WARN
 * 3 = 2 + INFO
 * 4 = 3 + TRACE
 */
#define DEBUG_LEVEL ECM_CLASSIFIER_HYFI_DEBUG_LEVEL

#include "ecm_types.h"
#include "ecm_db_types.h"
#include "ecm_state.h"
#include "ecm_tracker.h"
#include "ecm_classifier.h"
#include "ecm_front_end_types.h"
#include "ecm_tracker_udp.h"
#include "ecm_tracker_tcp.h"
#include "ecm_db.h"
#include "ecm_classifier_hyfi.h"
#include "ecm_front_end_ipv4.h"
#include "ecm_interface.h"
#include "ecm_front_end_common.h"

/*
 * Magic numbers
 */
#define ECM_CLASSIFIER_HYFI_INSTANCE_MAGIC 0xFE25

/*
 * Definitions
 */
#define ECM_CLASSIFIER_HYFI_STATE_INIT              ( 1 << 0 )
#define ECM_CLASSIFIER_HYFI_STATE_REGISTERED	    ( 1 << 1 )
#define ECM_CLASSIFIER_HYFI_STATE_IGNORE            ( 1 << 2 )

/*
 * struct ecm_classifier_hyfi_instance
 * 	State to allow tracking of dynamic qos for a connection
 */
struct ecm_classifier_hyfi_instance {
	struct ecm_classifier_instance base;			/* Base type */

	struct ecm_classifier_hyfi_instance *next;		/* Next classifier state instance (for accouting and reporting purposes) */
	struct ecm_classifier_hyfi_instance *prev;		/* Next classifier state instance (for accouting and reporting purposes) */

	uint32_t ci_serial;					/* RO: Serial of the connection */
	struct ecm_classifier_process_response process_response;/* Last process response computed */

	uint32_t hyfi_state;
	struct hyfi_ecm_flow_data_t flow;

	int refs;						/* Integer to trap we never go negative */
#if (DEBUG_LEVEL > 0)
	uint16_t magic;
#endif
	bool multi_bridge_flow;
	char bridge_name[IFNAMSIZ]; /* Destination bridge device of this hyfi instance */
};

/*
 * Listener for db events
 */
struct ecm_db_listener_instance *ecm_classifier_hyfi_li = NULL;

/*
 * Operational control - defaults to false (disabled)
 */
static int ecm_classifier_hyfi_enabled;		/* Operational behaviour */

/*
 * Management thread control
 */
static bool ecm_classifier_hyfi_terminate_pending = false;		/* True when the user wants us to terminate */

/*
 * Debugfs dentry object.
 */
static struct dentry *ecm_classifier_hyfi_dentry;

/*
 * Locking of the classifier structures
 */
static DEFINE_SPINLOCK(ecm_classifier_hyfi_lock);			/* Protect SMP access. */

/*
 * List of our classifier instances
 */
static struct ecm_classifier_hyfi_instance *ecm_classifier_hyfi_instances = NULL;
									/* list of all active instances */
static int ecm_classifier_hyfi_count = 0;					/* Tracks number of instances allocated */

/*
 * ecm_classifier_hyfi_ref()
 *	Ref
 */
static void ecm_classifier_hyfi_ref(struct ecm_classifier_instance *ci)
{
	struct ecm_classifier_hyfi_instance *chfi;
	chfi = (struct ecm_classifier_hyfi_instance *)ci;

	DEBUG_CHECK_MAGIC(chfi, ECM_CLASSIFIER_HYFI_INSTANCE_MAGIC, "%px: magic failed\n", chfi);
	spin_lock_bh(&ecm_classifier_hyfi_lock);
	chfi->refs++;
	DEBUG_TRACE("%px: chfi ref %d\n", chfi, chfi->refs);
	DEBUG_ASSERT(chfi->refs > 0, "%px: ref wrap\n", chfi);
	spin_unlock_bh(&ecm_classifier_hyfi_lock);
}

/*
 * ecm_classifier_hyfi_deref()
 *	Deref
 */
static int ecm_classifier_hyfi_deref(struct ecm_classifier_instance *ci)
{
	struct ecm_classifier_hyfi_instance *chfi;
	chfi = (struct ecm_classifier_hyfi_instance *)ci;

	DEBUG_CHECK_MAGIC(chfi, ECM_CLASSIFIER_HYFI_INSTANCE_MAGIC, "%px: magic failed\n", chfi);
	spin_lock_bh(&ecm_classifier_hyfi_lock);
	chfi->refs--;
	DEBUG_ASSERT(chfi->refs >= 0, "%px: refs wrapped\n", chfi);
	DEBUG_TRACE("%px: HyFi classifier deref %d\n", chfi, chfi->refs);
	if (chfi->refs) {
		int refs = chfi->refs;
		spin_unlock_bh(&ecm_classifier_hyfi_lock);
		return refs;
	}

	/*
	 * Object to be destroyed
	 */
	ecm_classifier_hyfi_count--;
	DEBUG_ASSERT(ecm_classifier_hyfi_count >= 0, "%px: ecm_classifier_hyfi_count wrap\n", chfi);

	/*
	 * UnLink the instance from our list
	 */
	if (chfi->next) {
		chfi->next->prev = chfi->prev;
	}
	if (chfi->prev) {
		chfi->prev->next = chfi->next;
	} else {
		DEBUG_ASSERT(ecm_classifier_hyfi_instances == chfi, "%px: list bad %px\n", chfi, ecm_classifier_hyfi_instances);
		ecm_classifier_hyfi_instances = chfi->next;
	}
	chfi->next = NULL;
	chfi->prev = NULL;

	spin_unlock_bh(&ecm_classifier_hyfi_lock);

	/*
	 * Final
	 */
	DEBUG_INFO("%px: Final HyFi classifier instance\n", chfi);
	kfree(chfi);

	return 0;
}

/*
 * ecm_classifier_hyfi_process()
 *	Process new data for connection
 */
static void ecm_classifier_hyfi_process(struct ecm_classifier_instance *aci, ecm_tracker_sender_type_t sender,
						struct ecm_tracker_ip_header *ip_hdr, struct sk_buff *skb,
						struct ecm_classifier_process_response *process_response)
{
	struct ecm_classifier_hyfi_instance *chfi;
	ecm_classifier_relevence_t relevance;
	bool enabled;
	struct ecm_db_connection_instance *ci;
	struct ecm_front_end_connection_instance *feci;
	ecm_front_end_acceleration_mode_t accel_mode;
	uint32_t became_relevant = 0;
	uint32_t flag = 0;

	chfi = (struct ecm_classifier_hyfi_instance *)aci;
	DEBUG_CHECK_MAGIC(chfi, ECM_CLASSIFIER_HYFI_INSTANCE_MAGIC, "%px: magic failed\n", chfi);

	/*
	 * Are we yet to decide if this instance is relevant to the connection?
	 */
	spin_lock_bh(&ecm_classifier_hyfi_lock);
	relevance = chfi->process_response.relevance;
	if (relevance != ECM_CLASSIFIER_RELEVANCE_MAYBE) {
		/*
		 * We already know
		 * NOTE: Lock still held
		 */
		goto hyfi_classifier_out;
	}

	enabled = ecm_classifier_hyfi_enabled;
	spin_unlock_bh(&ecm_classifier_hyfi_lock);

	/*
	 * Need to decide our relevance to this connection.
	 * If classifier is enabled and the front end says it can accel then we are "relevant".
	 * Any other condition and we are not and will stop analysing this connection.
	 */
	relevance = ECM_CLASSIFIER_RELEVANCE_NO;
	ci = ecm_db_connection_serial_find_and_ref(chfi->ci_serial);
	if (!ci) {
		DEBUG_TRACE("%px: No ci found for %u\n", chfi, chfi->ci_serial);
		accel_mode = ECM_FRONT_END_ACCELERATION_MODE_FAIL_DENIED;
	} else {
		feci = ecm_db_connection_front_end_get_and_ref(ci);
		accel_mode = ecm_front_end_connection_accel_state_get(feci);
		ecm_front_end_connection_deref(feci);
		ecm_db_connection_deref(ci);
	}

	if (enabled && ECM_FRONT_END_ACCELERATION_POSSIBLE(accel_mode) &&
		hyfi_ecm_bridge_attached(chfi->bridge_name)) {
		relevance = ECM_CLASSIFIER_RELEVANCE_YES;
		became_relevant = ecm_db_time_get();
	}

	spin_lock_bh(&ecm_classifier_hyfi_lock);
	chfi->process_response.relevance = relevance;
	chfi->process_response.became_relevant = became_relevant;

hyfi_classifier_out:
	;

	/*
	 * Return our process response
	 */
	*process_response = chfi->process_response;
	if (relevance == ECM_CLASSIFIER_RELEVANCE_NO) {
		goto hyfi_classifier_done;
	}

	/*
	 * Fast path, already accelerated or ignored
	 */
	if (chfi->hyfi_state & (ECM_CLASSIFIER_HYFI_STATE_REGISTERED | ECM_CLASSIFIER_HYFI_STATE_IGNORE)) {
		if (chfi->hyfi_state & ECM_CLASSIFIER_HYFI_STATE_REGISTERED) {
			DEBUG_INFO("%px: Regen of Flow serial: %d Flow hash: 0x%02x (@%lu)\n",
				aci, chfi->flow.ecm_serial, chfi->flow.hash, jiffies);
		}
		goto hyfi_classifier_done;
	}

	/*
	 * Compute the hashes in both forward and reverse directions
	 */
	if (unlikely(hyfi_hash_skbuf(skb, &chfi->flow.hash, &flag,
		&chfi->flow.priority, &chfi->flow.seq)))
		goto hyfi_classifier_done;

	if (unlikely(hyfi_hash_skbuf_reverse(skb, &chfi->flow.reverse_hash)))
		goto hyfi_classifier_done;

	chfi->flow.ecm_serial = chfi->ci_serial;
	if (flag & ECM_HYFI_IS_IPPROTO_UDP)
		hyfi_ecm_set_flag(&chfi->flow, ECM_HYFI_IS_IPPROTO_UDP);
	else
		hyfi_ecm_clear_flag(&chfi->flow, ECM_HYFI_IS_IPPROTO_UDP);

	memcpy(&chfi->flow.sa, eth_hdr(skb)->h_source, ETH_ALEN);
	memcpy(&chfi->flow.da, eth_hdr(skb)->h_dest, ETH_ALEN);

	DEBUG_INFO("%px: Flow serial: %d\nFlow hash: 0x%02x, priority 0x%08x, "
			"flag: %d\nSA: %pM\nDA: %pM (@%lu)\n\n",
			aci, chfi->flow.ecm_serial, chfi->flow.hash,
			chfi->flow.priority, chfi->flow.flag,
			chfi->flow.sa, chfi->flow.da, jiffies);

	chfi->hyfi_state = ECM_CLASSIFIER_HYFI_STATE_REGISTERED;

hyfi_classifier_done:
	;

	spin_unlock_bh(&ecm_classifier_hyfi_lock);
}

/*
 * ecm_classifier_hyfi_get_intf_id()
 *	Get the interface ID from the level of the
 *	connection hierarchy that matches the interface selected by HyFi.
 *
 * If the hierarchy contains a bridge, the correct interface will be in the level
 * of the hierarchy after the bridge.
 * If the hierarchy doesn't contain a bridge, look for an interface that has its
 * master set to the HyFi bridge.
 *
 * If neither of these conditions hold, this hierarchy can't be relevant to HyFi,
 * and return -1.
 */
static int32_t ecm_classifier_hyfi_get_intf_id(struct ecm_db_connection_instance *ci,
	ecm_db_obj_dir_t dir)
{
	int32_t intf_first;
	int32_t system_index = -1;
	struct ecm_db_iface_instance *interfaces[ECM_DB_IFACE_HEIRARCHY_MAX];
	int32_t i;
	char name[IFNAMSIZ];

	intf_first = ecm_db_connection_interfaces_get_and_ref(ci, interfaces, dir);

	if (intf_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
		DEBUG_WARN("%px: Error fetching interfaces\n", ci);
		return -1;
	}

	/*
	 * Search from innermost to outermost interface
	 */
	for (i = ECM_DB_IFACE_HEIRARCHY_MAX - 1; i >= intf_first; i--) {
		int32_t index;
		if (ecm_db_iface_type_get(interfaces[i]) == ECM_DB_IFACE_TYPE_BRIDGE) {
			/*
			 * Found the bridge - next interface should be the one we want
			 */
			if (i <= intf_first) {
				DEBUG_WARN("%px: Found bridge at position %d, but first interface "
					"is %d, can't fetch HyFi relevant interface\n", ci,
					i, intf_first);
				break;
			}

			index = ecm_db_iface_interface_identifier_get(interfaces[i-1]);

			if (!hyfi_ecm_is_port_on_hyfi_bridge(index)) {
				DEBUG_TRACE("%px: Found non-Hy-Fi bridge, ignoring\n", ci);
				break;
			}

			ecm_db_iface_interface_name_get(interfaces[i-1], &name[0]);
			DEBUG_TRACE("%px: Found bridge: '%s' interface is %d (%s)\n", ci,
				ecm_db_obj_dir_strings[dir], index, name);
			system_index = index;
			break;
		}

		/*
		 * Bridge not found yet - check if this interface belongs to the bridge
		 */
		index = ecm_db_iface_interface_identifier_get(interfaces[i]);
		if (hyfi_ecm_is_port_on_hyfi_bridge(index)) {
			ecm_db_iface_interface_name_get(interfaces[i], &name[0]);
			DEBUG_TRACE("%px: Found bridge port: '%s' interface is %d (%s)\n", ci,
				ecm_db_obj_dir_strings[dir], index,
				name);
			system_index = index;
			break;
		}

		/*
		 * Match not found - keep searching
		 */
	}

	ecm_db_connection_interfaces_deref(interfaces, intf_first);

	return system_index;
}

/* ecm_classifier_hyfi_get_bridge_name()
 *	Get the bridge name from the connection hierarchy
 */
static void ecm_classifier_hyfi_get_bridge_name(struct ecm_db_connection_instance *ci,
		char *name_buffer, ecm_db_obj_dir_t dir)
{
	int32_t intf_first;
	struct ecm_db_iface_instance *interfaces[ECM_DB_IFACE_HEIRARCHY_MAX];
	int32_t i;

	intf_first = ecm_db_connection_interfaces_get_and_ref(ci, interfaces, dir);

	if (intf_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
		DEBUG_WARN("%px: Error fetching bridge name\n", ci);
		return;
	}

	for (i = intf_first; i < ECM_DB_IFACE_HEIRARCHY_MAX; i++) {
		struct net_device *dev;
		dev = dev_get_by_index(&init_net, ecm_db_iface_interface_identifier_get(interfaces[i]));

		if (dev && ecm_front_end_is_bridge_port(dev)) {
			struct net_device *master;
			master = ecm_interface_get_and_hold_dev_master(dev);
			if (master) {
				strlcpy(name_buffer, master->name, IFNAMSIZ);
				dev_put(master);
				dev_put(dev);
				break;
			}
		}
		if (dev)
			dev_put(dev);
	}

	ecm_db_connection_interfaces_deref(interfaces, intf_first);
}

/*
 * ecm_classifier_hyfi_sync_to_v4()
 *	Front end is pushing accel engine state to us
 */
static void ecm_classifier_hyfi_sync_to_v4(struct ecm_classifier_instance *aci, struct ecm_classifier_rule_sync *sync)
{
	struct ecm_classifier_hyfi_instance *chfi;
	struct ecm_db_connection_instance *ci;
	uint64_t from_packets = 0;
	uint64_t from_bytes = 0;
	uint64_t to_packets = 0;
	uint64_t to_bytes = 0;
	uint64_t from_packets_dropped = 0;
	uint64_t from_bytes_dropped = 0;
	uint64_t to_packets_dropped = 0;
	uint64_t to_bytes_dropped = 0;
	int32_t ret_fwd, ret_rev;
	uint32_t time_now;
	uint32_t time_elapsed_fwd = 0;
	uint32_t time_elapsed_rev = 0;
	uint8_t flow_dest_addr[ETH_ALEN];
	uint64_t fwd_bytes, rev_bytes, fwd_packets, rev_packets;
	bool should_keep_on_fdb_update_fwd = false;
	bool should_keep_on_fdb_update_rev = false;
	int32_t to_system_index;
	int32_t from_system_index;

	if (sync->reason != ECM_FRONT_END_IPV4_RULE_SYNC_REASON_STATS) {
		DEBUG_TRACE("%px: Update not due to stats: %d\n",
			aci, sync->reason);
		return;
	}

	chfi = (struct ecm_classifier_hyfi_instance *)aci;
	DEBUG_CHECK_MAGIC(chfi, ECM_CLASSIFIER_HYFI_INSTANCE_MAGIC, "%px: magic failed", chfi);

	if (chfi->hyfi_state &
			(ECM_CLASSIFIER_HYFI_STATE_IGNORE)) {
		return;
	}

	ci = ecm_db_connection_serial_find_and_ref(chfi->ci_serial);
	if (!ci) {
		DEBUG_TRACE("%px: No ci found for %u\n", chfi, chfi->ci_serial);
		return;
	}

	/*
	 * Update the stats on Hy-Fi side
	 */
	to_system_index = ecm_classifier_hyfi_get_intf_id(ci, ECM_DB_OBJ_DIR_TO);
	from_system_index = ecm_classifier_hyfi_get_intf_id(ci, ECM_DB_OBJ_DIR_FROM);

	ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_TO, &flow_dest_addr[0]);
	ecm_db_connection_data_stats_get(ci, &from_bytes, &to_bytes,
			&from_packets, &to_packets,
			&from_bytes_dropped, &to_bytes_dropped,
			&from_packets_dropped, &to_packets_dropped);

	DEBUG_INFO("UPDATE STATS: Flow serial: %d\npriority 0x%08x, flag: %d\nSA: %pM\nDA: %pM\n\n",
			chfi->flow.ecm_serial, chfi->flow.priority, chfi->flow.flag,
			chfi->flow.sa, chfi->flow.da);

	time_now = jiffies;
	/*
	 * Make sure destination address still matches
	 */
	if (memcmp(&flow_dest_addr[0], &chfi->flow.da[0], ETH_ALEN)) {
		DEBUG_INFO("UPDATE STATS: Direction of flow is "
			"reverse of expectation\n");
		fwd_bytes = (to_bytes - to_bytes_dropped);
		fwd_packets = (to_packets - to_packets_dropped);
		rev_bytes = (from_bytes - from_bytes_dropped);
		rev_packets = (from_packets - from_packets_dropped);
	} else {
		fwd_bytes = (from_bytes - from_bytes_dropped);
		fwd_packets = (from_packets - from_packets_dropped);
		rev_bytes = (to_bytes - to_bytes_dropped);
		rev_packets = (to_packets - to_packets_dropped);
	}

	/*
	 * Update forward direction 'from' bytes
	 */
	ret_fwd = hyfi_ecm_update_stats(&chfi->flow, chfi->flow.hash,
		&chfi->flow.da[0], &chfi->flow.sa[0],
		fwd_bytes, fwd_packets, time_now,
		&should_keep_on_fdb_update_fwd,
		&time_elapsed_fwd, chfi->bridge_name);
	DEBUG_INFO("ret_fwd %d Forward hash: 0x%02x, from_bytes = %lld, from_packets = %lld, "
		"num_bytes_dropped = %lld, num_packets_dropped = %lld\n",
		ret_fwd, chfi->flow.hash, from_bytes, from_packets,
		from_bytes_dropped, from_packets_dropped);

	/*
	 * Update reverse direction 'to' bytes
	 */
	ret_rev = hyfi_ecm_update_stats(&chfi->flow, chfi->flow.reverse_hash,
		&chfi->flow.sa[0], &chfi->flow.da[0],
		rev_bytes, rev_packets, time_now,
		&should_keep_on_fdb_update_rev,
		&time_elapsed_rev, chfi->bridge_name);

	DEBUG_INFO("ret_rev %d Reverse hash: 0x%02x, to_bytes = %lld, to_packets = %lld, "
		"num_bytes_dropped = %lld, num_packets_dropped = %lld\n",
		ret_rev, chfi->flow.reverse_hash, to_bytes, to_packets,
		to_bytes_dropped, to_packets_dropped);

	chfi->flow.last_update = time_now;
	chfi->flow.last_elapsed_time = time_elapsed_fwd >= time_elapsed_rev ?
		time_elapsed_fwd : time_elapsed_rev;

	if (!time_elapsed_fwd) {
		if (should_keep_on_fdb_update_fwd) {
			hyfi_ecm_set_flag(&chfi->flow,
				ECM_HYFI_SHOULD_KEEP_ON_FDB_UPDATE_FWD);
		} else {
			hyfi_ecm_clear_flag(&chfi->flow,
				ECM_HYFI_SHOULD_KEEP_ON_FDB_UPDATE_FWD);
		}
	}

	if (!time_elapsed_rev) {
		if (should_keep_on_fdb_update_rev) {
			hyfi_ecm_set_flag(&chfi->flow,
				ECM_HYFI_SHOULD_KEEP_ON_FDB_UPDATE_REV);
		} else {
			hyfi_ecm_clear_flag(&chfi->flow,
				ECM_HYFI_SHOULD_KEEP_ON_FDB_UPDATE_REV);
		}
	}

	if (!hyfi_ecm_port_matches(&chfi->flow, to_system_index,
			 from_system_index, chfi->bridge_name)) {
		struct ecm_front_end_connection_instance *feci;
		unsigned long start_time, end_time;

		feci = ecm_db_connection_front_end_get_and_ref(ci);
		start_time = feci->stats.cmd_time_begun;
		end_time = feci->stats.cmd_time_completed;
		if (chfi->flow.cmd_time_begun != start_time &&
			chfi->flow.cmd_time_completed != end_time) {

			/*
			 * Only do another regenerate operation if the timestamps have
			 * changed (indicating that the front-end has completed the
			 * previously requested operation)
			 */
			feci->regenerate(feci, ci);
			DEBUG_INFO("%px: Mismatch for %u egress port between HyFi and "
				"ECM,\nregenerate start %lu, end %lu, "
				"previous start %lu, end %lu (@%u)\n",
				ci, chfi->flow.ecm_serial, start_time, end_time,
				chfi->flow.cmd_time_begun, chfi->flow.cmd_time_completed,
				time_now);
			chfi->flow.cmd_time_begun = start_time;
			chfi->flow.cmd_time_completed = end_time;
		} else {
			DEBUG_INFO("%px: Mismatch for %u egress port between HyFi and "
				"ECM,\nbut previous regeneration in progress start "
				" %lu, end %lu (@%u)\n",
				ci, chfi->flow.ecm_serial, start_time, end_time, time_now);
		}
		ecm_front_end_connection_deref(feci);
	}

	ecm_db_connection_deref(ci);

	if (ret_fwd < 0 || ret_rev < 0) {
		DEBUG_ERROR_RATELIMITED("%px: Error updating stats", aci);
		return;
	}

	/*
	 * Only need to be interested in forward or reverse direction
	 */
	if (ret_fwd == 0 || ret_rev == 0) {
		chfi->hyfi_state = ECM_CLASSIFIER_HYFI_STATE_REGISTERED;
	} else if (ret_fwd == 2 || ret_rev == 2) {
		/*
		 * Not attached, may be interested in the future
		 */
		chfi->hyfi_state = ECM_CLASSIFIER_HYFI_STATE_INIT;
	} else {
		/*
		 * Not interested
		 */
		chfi->hyfi_state = ECM_CLASSIFIER_HYFI_STATE_IGNORE;
	}
}

/*
 * ecm_classifier_hyfi_sync_from_v4()
 *	Front end is retrieving accel engine state from us
 */
static void ecm_classifier_hyfi_sync_from_v4(struct ecm_classifier_instance *aci, struct ecm_classifier_rule_create *ecrc)
{
	struct ecm_classifier_hyfi_instance *chfi;

	chfi = (struct ecm_classifier_hyfi_instance *)aci;
	DEBUG_CHECK_MAGIC(chfi, ECM_CLASSIFIER_HYFI_INSTANCE_MAGIC, "%px: magic failed", chfi);
}

/*
 * ecm_classifier_hyfi_sync_to_v6()
 *	Front end is pushing accel engine state to us
 */
static void ecm_classifier_hyfi_sync_to_v6(struct ecm_classifier_instance *aci, struct ecm_classifier_rule_sync *sync)
{
	/* Same processing as v4 */
	ecm_classifier_hyfi_sync_to_v4(aci, sync);
}

/*
 * ecm_classifier_hyfi_sync_from_v6()
 *	Front end is retrieving accel engine state from us
 */
static void ecm_classifier_hyfi_sync_from_v6(struct ecm_classifier_instance *aci, struct ecm_classifier_rule_create *ecrc)
{
	struct ecm_classifier_hyfi_instance *chfi;

	chfi = (struct ecm_classifier_hyfi_instance *)aci;
	DEBUG_CHECK_MAGIC(chfi, ECM_CLASSIFIER_HYFI_INSTANCE_MAGIC, "%px: magic failed", chfi);

}

/*
 * ecm_classifier_hyfi_type_get()
 *	Get type of classifier this is
 */
static ecm_classifier_type_t ecm_classifier_hyfi_type_get(struct ecm_classifier_instance *ci)
{
	struct ecm_classifier_hyfi_instance *chfi;
	chfi = (struct ecm_classifier_hyfi_instance *)ci;

	DEBUG_CHECK_MAGIC(chfi, ECM_CLASSIFIER_HYFI_INSTANCE_MAGIC, "%px: magic failed\n", chfi);
	return ECM_CLASSIFIER_TYPE_HYFI;
}

/*
 * ecm_classifier_hyfi_last_process_response_get()
 *	Get result code returned by the last process call
 */
static void ecm_classifier_hyfi_last_process_response_get(struct ecm_classifier_instance *ci,
							struct ecm_classifier_process_response *process_response)
{
	struct ecm_classifier_hyfi_instance *chfi;

	chfi = (struct ecm_classifier_hyfi_instance *)ci;
	DEBUG_CHECK_MAGIC(chfi, ECM_CLASSIFIER_HYFI_INSTANCE_MAGIC, "%px: magic failed\n", chfi);

	spin_lock_bh(&ecm_classifier_hyfi_lock);
	*process_response = chfi->process_response;
	spin_unlock_bh(&ecm_classifier_hyfi_lock);
}

/*
 * ecm_classifier_hyfi_reclassify_allowed()
 *	Indicate if reclassify is allowed
 */
static bool ecm_classifier_hyfi_reclassify_allowed(struct ecm_classifier_instance *ci)
{
	struct ecm_classifier_hyfi_instance *chfi;
	chfi = (struct ecm_classifier_hyfi_instance *)ci;
	DEBUG_CHECK_MAGIC(chfi, ECM_CLASSIFIER_HYFI_INSTANCE_MAGIC, "%px: magic failed\n", chfi);

	return true;
}

/*
 * ecm_classifier_hyfi_reclassify()
 *	Reclassify
 */
static void ecm_classifier_hyfi_reclassify(struct ecm_classifier_instance *ci)
{
	struct ecm_classifier_hyfi_instance *chfi;
	chfi = (struct ecm_classifier_hyfi_instance *)ci;
	DEBUG_CHECK_MAGIC(chfi, ECM_CLASSIFIER_HYFI_INSTANCE_MAGIC, "%px: magic failed\n", chfi);
}

#ifdef ECM_STATE_OUTPUT_ENABLE
/*
 * ecm_classifier_hyfi_state_get()
 *	Return state
 */
static int ecm_classifier_hyfi_state_get(struct ecm_classifier_instance *ci, struct ecm_state_file_instance *sfi)
{
	int result;
	struct ecm_classifier_hyfi_instance *chfi;
	struct ecm_classifier_process_response process_response;

	chfi = (struct ecm_classifier_hyfi_instance *)ci;
	DEBUG_CHECK_MAGIC(chfi, ECM_CLASSIFIER_HYFI_INSTANCE_MAGIC, "%px: magic failed", chfi);

	if ((result = ecm_state_prefix_add(sfi, "hyfi"))) {
		return result;
	}

	spin_lock_bh(&ecm_classifier_hyfi_lock);
	process_response = chfi->process_response;
	spin_unlock_bh(&ecm_classifier_hyfi_lock);

	/*
	 * Output our last process response
	 */
	if ((result = ecm_classifier_process_response_state_get(sfi, &process_response))) {
		return result;
	}

	return ecm_state_prefix_remove(sfi);
}
#endif

static void ecm_classifier_hyfi_should_keep_connection(
	struct ecm_classifier_instance *aci, struct ecm_db_connection_defunct_info *info)
{
	struct ecm_classifier_hyfi_instance *chfi;
	/*
	 * If the event is STA join, classifer does not care about the
	 * the connection defunct, so we return.
	 */
	if (info->type == ECM_DB_CONNECTION_DEFUNCT_TYPE_STA_JOIN) {
		return;
	}

	chfi = (struct ecm_classifier_hyfi_instance *)aci;
	DEBUG_CHECK_MAGIC(chfi, ECM_CLASSIFIER_HYFI_INSTANCE_MAGIC,
		"%px: magic failed", chfi);

	if (chfi->hyfi_state &
			(ECM_CLASSIFIER_HYFI_STATE_IGNORE)) {
		/* HyFi doesn't care if connection deleted */
		return;
	}

	info->should_keep_connection = hyfi_ecm_should_keep(&chfi->flow, info->mac, chfi->bridge_name);
}

/*
 * ecm_classifier_hyfi_instance_alloc()
 *	Allocate an instance of the HyFi classifier
 */
struct ecm_classifier_hyfi_instance *ecm_classifier_hyfi_instance_alloc(struct ecm_db_connection_instance *ci)
{
	struct ecm_classifier_hyfi_instance *chfi;
	char to_bridge[IFNAMSIZ];
	char from_bridge[IFNAMSIZ];

	/*
	 * Allocate the instance
	 */
	chfi = (struct ecm_classifier_hyfi_instance *)kzalloc(sizeof(struct ecm_classifier_hyfi_instance), GFP_ATOMIC | __GFP_NOWARN);
	if (!chfi) {
		DEBUG_WARN("Failed to allocate HyFi instance\n");
		return NULL;
	}

	DEBUG_SET_MAGIC(chfi, ECM_CLASSIFIER_HYFI_INSTANCE_MAGIC);
	chfi->refs = 1;
	chfi->base.process = ecm_classifier_hyfi_process;
	chfi->base.sync_from_v4 = ecm_classifier_hyfi_sync_from_v4;
	chfi->base.sync_to_v4 = ecm_classifier_hyfi_sync_to_v4;
	chfi->base.sync_from_v6 = ecm_classifier_hyfi_sync_from_v6;
	chfi->base.sync_to_v6 = ecm_classifier_hyfi_sync_to_v6;
	chfi->base.type_get = ecm_classifier_hyfi_type_get;
	chfi->base.last_process_response_get = ecm_classifier_hyfi_last_process_response_get;
	chfi->base.reclassify_allowed = ecm_classifier_hyfi_reclassify_allowed;
	chfi->base.reclassify = ecm_classifier_hyfi_reclassify;
	chfi->base.should_keep_connection =
		ecm_classifier_hyfi_should_keep_connection;
#ifdef ECM_STATE_OUTPUT_ENABLE
	chfi->base.state_get = ecm_classifier_hyfi_state_get;
#endif
	chfi->base.ref = ecm_classifier_hyfi_ref;
	chfi->base.deref = ecm_classifier_hyfi_deref;
	chfi->ci_serial = ecm_db_connection_serial_get(ci);
	chfi->process_response.process_actions = 0;
	chfi->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_MAYBE;

	/*
	 * Find and save the bridge name. This will be passed to hyfi module later
	 */
	to_bridge[0] = 0;
	from_bridge[0] = 0;
	ecm_classifier_hyfi_get_bridge_name(ci, to_bridge, ECM_DB_OBJ_DIR_TO);
	ecm_classifier_hyfi_get_bridge_name(ci, from_bridge, ECM_DB_OBJ_DIR_FROM);

	if (!strlen(to_bridge) || !strlen(from_bridge)) {
		/* one of the bridge name is null, typical
		 * routed connection. Consider valid bridge*/
		if (strlen(to_bridge)) {
			strlcpy(chfi->bridge_name, to_bridge, IFNAMSIZ);
		} else if (strlen(from_bridge)) {
			strlcpy(chfi->bridge_name, from_bridge, IFNAMSIZ);
		}
	} else if (!strncmp(to_bridge, from_bridge, IFNAMSIZ)) {
		/* Pure bridge connection. Consider any one bridge */
		strlcpy(chfi->bridge_name, to_bridge, IFNAMSIZ);
	} else {
		/* multi-bridge connection */
		chfi->multi_bridge_flow = true;

		/* TODO: Currently we are not supporting
		 * multi-bridge connection, stats wont be
		 * updated for multi-bridge flow */
	}

	/*
	 * Init Hy-Fi state
	 */
	chfi->hyfi_state = ECM_CLASSIFIER_HYFI_STATE_INIT;

	spin_lock_bh(&ecm_classifier_hyfi_lock);

	/*
	 * Final check if we are pending termination
	 */
	if (ecm_classifier_hyfi_terminate_pending) {
		spin_unlock_bh(&ecm_classifier_hyfi_lock);
		DEBUG_INFO("%px: Terminating\n", ci);
		kfree(chfi);
		return NULL;
	}

	/*
	 * Link the new instance into our list at the head
	 */
	chfi->next = ecm_classifier_hyfi_instances;
	if (ecm_classifier_hyfi_instances) {
		ecm_classifier_hyfi_instances->prev = chfi;
	}
	ecm_classifier_hyfi_instances = chfi;

	/*
	 * Increment stats
	 */
	ecm_classifier_hyfi_count++;
	DEBUG_ASSERT(ecm_classifier_hyfi_count > 0, "%px: ecm_classifier_hyfi_count wrap\n", chfi);
	spin_unlock_bh(&ecm_classifier_hyfi_lock);

	DEBUG_INFO("HyFi instance alloc: %px\n", chfi);
	return chfi;
}
EXPORT_SYMBOL(ecm_classifier_hyfi_instance_alloc);

/*
 * ecm_classifier_hyfi_connection_added()
 *	Invoked when a connection is added to the DB
 */
static void ecm_classifier_hyfi_connection_added(void *arg, struct ecm_db_connection_instance *ci)
{
#if (DEBUG_LEVEL > 2)
	uint32_t serial = ecm_db_connection_serial_get(ci);
	DEBUG_INFO("%px: HYFI LISTENER: added conn with serial: %u\n", ci, serial);
#endif
}

/*
 * ecm_classifier_hyfi_connection_removed()
 *	Invoked when a connection is removed from the DB
 */
static void ecm_classifier_hyfi_connection_removed(void *arg, struct ecm_db_connection_instance *ci)
{
	struct ecm_classifier_instance *aci;
	struct ecm_classifier_hyfi_instance *chfi;
	uint32_t serial = ecm_db_connection_serial_get(ci);

	DEBUG_INFO("%px: HYFI LISTENER: removed conn with serial: %u\n", ci, serial);

	/*
	 * Only handle events if there is an HyFi classifier attached
	 */
	aci = ecm_db_connection_assigned_classifier_find_and_ref(ci, ECM_CLASSIFIER_TYPE_HYFI);
	if (!aci) {
		DEBUG_TRACE("%px: Connection removed ignored"
			" - no HyFi classifier\n", ci);
		return;
	}

	chfi = (struct ecm_classifier_hyfi_instance *)aci;
	DEBUG_INFO("%px: removed conn with serial: %u, "
		"hash 0x%x, rev hash 0x%x\n",
		aci, serial, chfi->flow.hash, chfi->flow.reverse_hash);

	/*
	 * Mark as decelerated
	 */
	hyfi_ecm_decelerate(chfi->flow.hash, serial, &chfi->flow.da[0], chfi->bridge_name);
	hyfi_ecm_decelerate(chfi->flow.reverse_hash, serial, &chfi->flow.sa[0], chfi->bridge_name);

	aci->deref(aci);
}

/*
 * ecm_classifier_hyfi_set_set_command()
 *	Set hyfi command to accel/decel connection.
 */
static ssize_t ecm_classifier_hyfi_set_command(struct file *file,
						const char __user *user_buf,
						size_t sz, loff_t *ppos)
{
#define ECM_CLASSIFIER_HYFI_SET_IP_COMMAND_FIELDS 2
	char *cmd_buf;
	int field_count;
	char *field_ptr;
	char *fields[ECM_CLASSIFIER_HYFI_SET_IP_COMMAND_FIELDS];
	char cmd;
	uint32_t serial;
	struct ecm_db_connection_instance *ci;
	struct ecm_front_end_connection_instance *feci;
	struct ecm_classifier_hyfi_instance *chfi;
	struct ecm_classifier_instance *classi;

	/*
	 * Check if we are enabled
	 */
	spin_lock_bh(&ecm_classifier_hyfi_lock);
	if (!ecm_classifier_hyfi_enabled) {
		spin_unlock_bh(&ecm_classifier_hyfi_lock);
		return -EINVAL;
	}
	spin_unlock_bh(&ecm_classifier_hyfi_lock);

	/*
	 * buf is formed as:
	 * [0]   [1]
	 * <CMD>/<SERIAL>
	 * CMD:
	 *	s = Decelerate based on <SERIAL> number given.
	 */
	cmd_buf = (char *)kzalloc(sz + 1, GFP_ATOMIC);
	if (!cmd_buf) {
		return -ENOMEM;
	}

	sz = simple_write_to_buffer(cmd_buf, sz, ppos, user_buf, sz);

	/*
	 * Split the buffer into its fields
	 */
	field_count = 0;
	field_ptr = cmd_buf;
	fields[field_count] = strsep(&field_ptr, "/");
	while (fields[field_count] != NULL) {
		DEBUG_TRACE("FIELD %d: %s\n", field_count, fields[field_count]);
		field_count++;
		if (field_count == ECM_CLASSIFIER_HYFI_SET_IP_COMMAND_FIELDS) {
			break;
		}
		fields[field_count] = strsep(&field_ptr, "/");
	}

	if (field_count != ECM_CLASSIFIER_HYFI_SET_IP_COMMAND_FIELDS) {
		DEBUG_WARN("invalid field count %d\n", field_count);
		kfree(cmd_buf);
		return -EINVAL;
	}

	if (!sscanf(fields[0], "%c", &cmd)) {
		DEBUG_WARN("invalid cmd\n");
		kfree(cmd_buf);
		return -EINVAL;
	}
	if (!sscanf(fields[1], "%u", &serial)) {
		DEBUG_WARN("invalid serial\n");
		kfree(cmd_buf);
		return -EINVAL;
	}

	kfree(cmd_buf);

	/*
	 * Locate the connection using the serial or tuple given
	 */
	switch (cmd) {
	case 's':
		DEBUG_TRACE("Lookup connection using serial: %u\n", serial);
		ci = ecm_db_connection_serial_find_and_ref(serial);
		break;
	default:
		DEBUG_WARN("invalid cmd %c\n", cmd);
		return -EINVAL;
	}

	if (!ci) {
		DEBUG_WARN("database connection not found\n");
		return -ENOMEM;
	}
	DEBUG_TRACE("Connection found: %px\n", ci);

	/*
	 * Get the Hy-Fi classifier instance
	 */
	classi = ecm_db_connection_assigned_classifier_find_and_ref(
		ci, ECM_CLASSIFIER_TYPE_HYFI);

	if (!classi) {
		DEBUG_WARN("%px: No Hy-Fi classifier instance\n", ci);
		ecm_db_connection_deref(ci);
		return -ENOMEM;
	}

	chfi = (struct ecm_classifier_hyfi_instance *)classi;

	/*
	 * Now action the command
	 */
	switch (cmd) {
	case 's':
	case 'f':

		/*
		 * Regenerate the connection
		 */
		feci = ecm_db_connection_front_end_get_and_ref(ci);

		/*
		 * Store the begin / end timestamps here (from the previous
		 * operation). Will not attempt another regen if
		 * ports don't match until this regenerate is completed (which
		 * can be detected by a change in the timestamps).
		 */
		chfi->flow.cmd_time_begun = feci->stats.cmd_time_begun;
		chfi->flow.cmd_time_completed = feci->stats.cmd_time_completed;
		feci->regenerate(feci, ci);
		ecm_front_end_connection_deref(feci);

		DEBUG_TRACE("%px: Force regeneration %u start_time %lu end_time"
			" %lu (@%lu)\n", ci, serial,
			chfi->flow.cmd_time_begun,
			chfi->flow.cmd_time_completed, jiffies);
		break;
	}
	classi->deref(classi);
	ecm_db_connection_deref(ci);

	return sz;
}

/*
 * File operations for hyfi classifier command.
 */
static struct file_operations ecm_classifier_hyfi_cmd_fops = {
	.write = ecm_classifier_hyfi_set_command,
	.llseek = generic_file_llseek,
	.owner = THIS_MODULE
};

/*
 * ecm_classifier_hyfi_rules_init()
 */
int ecm_classifier_hyfi_rules_init(struct dentry *dentry)
{
	DEBUG_INFO("HyFi classifier Module init\n");

	ecm_classifier_hyfi_dentry = debugfs_create_dir("ecm_classifier_hyfi", dentry);
	if (!ecm_classifier_hyfi_dentry) {
		DEBUG_ERROR("Failed to create ecm hyfi classifier directory in debugfs\n");
		goto classifier_task_cleanup;
	}

	debugfs_create_u32("enabled", S_IRUGO | S_IWUSR, ecm_classifier_hyfi_dentry,
					(u32 *)&ecm_classifier_hyfi_enabled);

	if (!debugfs_create_file("cmd", S_IWUSR, ecm_classifier_hyfi_dentry,
					NULL, &ecm_classifier_hyfi_cmd_fops)) {
		DEBUG_ERROR("Failed to create ecm hyfi classifier cmd file in debugfs\n");
		goto classifier_task_cleanup;
	}

	/*
	 * Allocate listener instance to listen for db events
	 */
	ecm_classifier_hyfi_li = ecm_db_listener_alloc();
	if (!ecm_classifier_hyfi_li) {
		DEBUG_ERROR("Failed to allocate listener\n");
		goto classifier_task_cleanup;
	}

	/*
	 * Add the listener into the database
	 * NOTE: Ref the thread count for the listener
	 */
	ecm_db_listener_add(ecm_classifier_hyfi_li,
			NULL /* ecm_classifier_hyfi_iface_added */,
			NULL /* ecm_classifier_hyfi_iface_removed */,
			NULL /* ecm_classifier_hyfi_node_added */,
			NULL /* ecm_classifier_hyfi_node_removed */,
			NULL /* ecm_classifier_hyfi_host_added */,
			NULL /* ecm_classifier_hyfi_host_removed */,
			NULL /* ecm_classifier_hyfi_mapping_added */,
			NULL /* ecm_classifier_hyfi_mapping_removed */,
			ecm_classifier_hyfi_connection_added,
			ecm_classifier_hyfi_connection_removed,
			NULL /* ecm_classifier_hyfi_listener_final */,
			ecm_classifier_hyfi_li);

	return 0;

classifier_task_cleanup:

	debugfs_remove_recursive(ecm_classifier_hyfi_dentry);
	return -1;
}
EXPORT_SYMBOL(ecm_classifier_hyfi_rules_init);

/*
 * ecm_classifier_hyfi_rules_exit()
 */
void ecm_classifier_hyfi_rules_exit(void)
{
	DEBUG_INFO("HyFi classifier Module exit\n");

	spin_lock_bh(&ecm_classifier_hyfi_lock);
	ecm_classifier_hyfi_terminate_pending = true;
	spin_unlock_bh(&ecm_classifier_hyfi_lock);

	/*
	 * Release our ref to the listener.
	 * This will cause it to be unattached to the db listener list.
	 * NOTE: Our thread refs will be released on final callback when
	 * we know there will be no more callbacks to it.
	 */
	if (ecm_classifier_hyfi_li) {
		ecm_db_listener_deref(ecm_classifier_hyfi_li);
		ecm_classifier_hyfi_li = NULL;
	}

	/*
	 * Remove the debugfs files recursively.
	 */
	if (ecm_classifier_hyfi_dentry) {
		debugfs_remove_recursive(ecm_classifier_hyfi_dentry);
	}
}
EXPORT_SYMBOL(ecm_classifier_hyfi_rules_exit);
