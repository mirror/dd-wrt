/*
 **************************************************************************
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
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
#include <net/ip6_route.h>
#include <net/ip6_fib.h>
#include <net/addrconf.h>
#include <net/ipv6.h>
#include <net/tcp.h>
#include <asm/unaligned.h>
#include <asm/uaccess.h>	/* for put_user */
#include <net/ipv6.h>
#include <linux/inet.h>
#include <linux/in6.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <linux/ppp_defs.h>
#include <linux/mroute6.h>
#include <linux/vmalloc.h>

#include <linux/inetdevice.h>
#include <linux/if_arp.h>
#include <linux/netfilter_ipv6.h>
#include <linux/netfilter_bridge.h>
#include <linux/if_bridge.h>
#include <net/arp.h>
#ifdef ECM_INTERFACE_VXLAN_ENABLE
#include <net/vxlan.h>
#endif
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_acct.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_l4proto.h>
#include <linux/netfilter/nf_conntrack_zones_common.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_timeout.h>
#include <net/netfilter/ipv6/nf_conntrack_ipv6.h>
#include <net/netfilter/ipv6/nf_defrag_ipv6.h>
#ifdef ECM_INTERFACE_VLAN_ENABLE
#include <linux/../../net/8021q/vlan.h>
#include <linux/if_vlan.h>
#endif

/*
 * Debug output levels
 * 0 = OFF
 * 1 = ASSERTS / ERRORS
 * 2 = 1 + WARN
 * 3 = 2 + INFO
 * 4 = 3 + TRACE
 */
#define DEBUG_LEVEL ECM_PPE_IPV6_DEBUG_LEVEL

#ifdef ECM_MULTICAST_ENABLE
#include <mc_ecm.h>
#endif

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
#ifdef ECM_CLASSIFIER_NL_ENABLE
#include "ecm_classifier_nl.h"
#endif
#include "ecm_interface.h"
#include "ecm_ppe_common.h"
#include "ecm_ppe_ported_ipv6.h"
#ifdef ECM_NON_PORTED_SUPPORT_ENABLE
#include "ecm_ppe_non_ported_ipv6.h"
#endif
#include "ecm_front_end_common.h"
#include "ecm_front_end_ipv6.h"
#include "ecm_ipv6.h"

#define ECM_PPE_IPV6_STATS_SYNC_PERIOD msecs_to_jiffies(60)
			/* Stats sync happens every 60ms, such that max of 2K conn are synced in 1sec; (1000ms / 60ms) * 128 = ~2K */
#define ECM_PPE_IPV6_STATS_SYNC_COUNT 128	/* Sync max of 128 connections from ppe */

int ecm_ppe_ipv6_no_action_limit_default = 250;		/* Default no-action limit. */
int ecm_ppe_ipv6_driver_fail_limit_default = 250;		/* Default driver fail limit. */
int ecm_ppe_ipv6_nack_limit_default = 1;			/* Default nack limit. */
int ecm_ppe_ipv6_accelerated_count = 0;			/* Total offloads */
int ecm_ppe_ipv6_pending_accel_count = 0;			/* Total pending offloads issued to the PPE / awaiting completion */
int ecm_ppe_ipv6_pending_decel_count = 0;			/* Total pending deceleration requests issued to the PPE / awaiting completion */
int ecm_ppe_ipv6_vlan_passthrough_enable = 0;           /* VLAN passthrough feature enable or disable flag */

/*
 * Limiting the acceleration of connections.
 *
 * By default there is no acceleration limiting.
 * This means that when ECM has more connections (that can be accelerated) than the acceleration
 * engine will allow the ECM will continue to try to accelerate.
 * In this scenario the acceleration engine will begin removal of existing rules to make way for new ones.
 * When the accel_limit_mode is set to FIXED ECM will not permit more rules to be issued than the engine will allow.
 */
uint32_t ecm_ppe_ipv6_accel_limit_mode = ECM_FRONT_END_ACCEL_LIMIT_MODE_UNLIMITED;

/*
 * Locking of the classifier - concurrency control for file global parameters.
 * NOTE: It is safe to take this lock WHILE HOLDING a feci->lock. The reverse is NOT SAFE.
 */
DEFINE_SPINLOCK(ecm_ppe_ipv6_lock);			/* Protect against SMP access between netfilter, events and private threaded function. */

/*
 * Workqueue for the connection sync
 */
struct workqueue_struct *ecm_ppe_ipv6_workqueue;
struct delayed_work ecm_ppe_ipv6_work;
static unsigned long int ecm_ppe_ipv6_stats_request_success = 0;	/* Number of success stats request */
static unsigned long int ecm_ppe_ipv6_stats_request_fail = 0;		/* Number of failed stats request */
static unsigned long int ecm_ppe_ipv6_stats_request_nack = 0;		/* Number of NACK'd stats request */

static unsigned long ecm_ppe_ipv6_accel_cmd_time_avg_samples = 0;	/* Sum of time taken for the set of accel command samples, used to compute average time for an accel command to complete */
static unsigned long ecm_ppe_ipv6_accel_cmd_time_avg_set = 1;	/* How many samples in the set */
static unsigned long ecm_ppe_ipv6_decel_cmd_time_avg_samples = 0;	/* Sum of time taken for the set of accel command samples, used to compute average time for an accel command to complete */
static unsigned long ecm_ppe_ipv6_decel_cmd_time_avg_set = 1;	/* How many samples in the set */

/*
 * Debugfs dentry object.
 */
static struct dentry *ecm_ppe_ipv6_dentry;

/*
 * ecm_ppe_ipv6_accel_done_time_update()
 *	Record how long the command took to complete, updating average samples
 */
void ecm_ppe_ipv6_accel_done_time_update(struct ecm_front_end_connection_instance *feci)
{
	unsigned long delta;

	/*
	 * How long did it take the command to complete?
	 */
	spin_lock_bh(&feci->lock);
	feci->stats.cmd_time_completed = jiffies;
	delta = feci->stats.cmd_time_completed - feci->stats.cmd_time_begun;
	spin_unlock_bh(&feci->lock);

	spin_lock_bh(&ecm_ppe_ipv6_lock);
	ecm_ppe_ipv6_accel_cmd_time_avg_samples += delta;
	ecm_ppe_ipv6_accel_cmd_time_avg_set++;
	spin_unlock_bh(&ecm_ppe_ipv6_lock);
}

/*
 * ecm_ppe_ipv6_deccel_done_time_update()
 *	Record how long the command took to complete, updating average samples
 */
void ecm_ppe_ipv6_decel_done_time_update(struct ecm_front_end_connection_instance *feci)
{
	unsigned long delta;

	/*
	 * How long did it take the command to complete?
	 */
	spin_lock_bh(&feci->lock);
	feci->stats.cmd_time_completed = jiffies;
	delta = feci->stats.cmd_time_completed - feci->stats.cmd_time_begun;
	spin_unlock_bh(&feci->lock);

	spin_lock_bh(&ecm_ppe_ipv6_lock);
	ecm_ppe_ipv6_decel_cmd_time_avg_samples += delta;
	ecm_ppe_ipv6_decel_cmd_time_avg_set++;
	spin_unlock_bh(&ecm_ppe_ipv6_lock);
}

/*
 * ecm_ppe_ipv6_process_one_conn_sync_msg()
 *	Process one connection sync message.
 */
static inline void ecm_ppe_ipv6_process_one_conn_sync_msg(struct ppe_drv_v6_conn_sync *sync)
{
	struct nf_conntrack_tuple_hash *h;
	struct nf_conntrack_tuple tuple;
	struct nf_conn *ct;
	struct nf_conn_counter *acct;
	struct ecm_db_connection_instance *ci;
	struct ecm_front_end_connection_instance *feci;
	struct neighbour *neigh;
	struct ecm_classifier_instance *assignments[ECM_CLASSIFIER_TYPES];
	int aci_index;
	int assignment_count;
	ip_addr_t flow_ip;
	ip_addr_t return_ip;
	struct in6_addr group6 __attribute__((unused));
	struct in6_addr origin6 __attribute__((unused));
	struct ecm_classifier_rule_sync class_sync;
	int flow_dir;
	int return_dir;
	unsigned long int delta_jiffies;
	int elapsed;

	ECM_PPE_IPV6_ADDR_TO_IP_ADDR(flow_ip, sync->flow_ip);
	ECM_PPE_IPV6_ADDR_TO_IP_ADDR(return_ip, sync->return_ip);

	/*
	 * Look up ecm connection with a view to synchronising the connection, classifier and data tracker.
	 * Note that we use _xlate versions for destination - for egressing connections this would be the wan IP address,
	 * but for ingressing this would be the LAN side (non-nat'ed) address and is what we need for lookup of our connection.
	 */
	DEBUG_INFO("%px: PPE Sync, lookup connection using\n" \
			"Protocol: %d\n" \
			"src_addr: " ECM_IP_ADDR_OCTAL_FMT ":%d\n" \
			"dest_addr: " ECM_IP_ADDR_OCTAL_FMT ":%d\n",
			sync,
			(int)sync->protocol,
			ECM_IP_ADDR_TO_OCTAL(flow_ip), (int)sync->flow_ident,
			ECM_IP_ADDR_TO_OCTAL(return_ip), (int)sync->return_ident);

	ci = ecm_db_connection_find_and_ref(flow_ip, return_ip, sync->protocol, (int)sync->flow_ident, (int)sync->return_ident);
	if (!ci) {
		DEBUG_TRACE("%px: PPE Sync: no connection\n", sync);
		return;
	}

	DEBUG_TRACE("%px: Sync conn %px\n", sync, ci);

	/*
	 * Copy the sync data to the classifier sync structure to
	 * update the classifiers' stats.
	 */
	class_sync.tx_packet_count[ECM_CONN_DIR_FLOW] = sync->flow_tx_packet_count;
	class_sync.tx_byte_count[ECM_CONN_DIR_FLOW] = sync->flow_tx_byte_count;
	class_sync.rx_packet_count[ECM_CONN_DIR_FLOW] = sync->flow_rx_packet_count;
	class_sync.rx_byte_count[ECM_CONN_DIR_FLOW] = sync->flow_rx_byte_count;
	class_sync.tx_packet_count[ECM_CONN_DIR_RETURN] = sync->return_tx_packet_count;
	class_sync.tx_byte_count[ECM_CONN_DIR_RETURN] = sync->return_tx_byte_count;
	class_sync.rx_packet_count[ECM_CONN_DIR_RETURN] = sync->return_rx_packet_count;
	class_sync.rx_byte_count[ECM_CONN_DIR_RETURN] = sync->return_rx_byte_count;
	class_sync.reason = sync->reason;

	/*
	 * Sync assigned classifiers
	 */
	assignment_count = ecm_db_connection_classifier_assignments_get_and_ref(ci, assignments);
	for (aci_index = 0; aci_index < assignment_count; ++aci_index) {
		struct ecm_classifier_instance *aci;
		aci = assignments[aci_index];
		DEBUG_TRACE("%px: sync to: %px, type: %d\n", ci, aci, aci->type_get(aci));
		aci->sync_to_v6(aci, &class_sync);
	}
	ecm_db_connection_assignments_release(assignment_count, assignments);

	/*
	 * Get the elapsed time since the last sync and add this elapsed time
	 * to the conntrack's timeout while updating it. If the return value is
	 * a negative value which means the timer is not in a valid state, just
	 * return here and do not update the defunct timer and the conntrack.
	 */
	elapsed = ecm_db_connection_elapsed_defunct_timer(ci);
	if (elapsed < 0) {
		ecm_db_connection_deref(ci);
		return;
	}
	delta_jiffies = elapsed * HZ;

	/*
	 * Keep connection alive and updated
	 */
	if (!ecm_db_connection_defunct_timer_touch(ci)) {
		ecm_db_connection_deref(ci);
		goto sync_conntrack;
	}

	/*
	 * Get the front end instance
	 */
	feci = ecm_db_connection_front_end_get_and_ref(ci);

	if (sync->flow_tx_packet_count || sync->return_tx_packet_count) {
		DEBUG_TRACE("%px: flow_rx_packet_count: %u, flow_rx_byte_count: %u, return_rx_packet_count: %u, , return_rx_byte_count: %u\n",
				ci, sync->flow_rx_packet_count, sync->flow_rx_byte_count, sync->return_rx_packet_count, sync->return_rx_byte_count);
		DEBUG_TRACE("%px: flow_tx_packet_count: %u, flow_tx_byte_count: %u, return_tx_packet_count: %u, return_tx_byte_count: %u\n",
				ci, sync->flow_tx_packet_count, sync->flow_tx_byte_count, sync->return_tx_packet_count, sync->return_tx_byte_count);
		/*
		 * The amount of data *sent* by the ECM connection 'from' side is the amount the PPE has *received* in the 'flow' direction.
		 */
		ecm_db_connection_data_totals_update(ci, true, sync->flow_rx_byte_count, sync->flow_rx_packet_count);

		/*
		 * The amount of data *sent* by the ECM connection 'to' side is the amount the PPE has *received* in the 'return' direction.
		 */
		ecm_db_connection_data_totals_update(ci, false, sync->return_rx_byte_count, sync->return_rx_packet_count);

		/*
		 * As packets have been accelerated we have seen some action.
		 */
		ecm_front_end_connection_action_seen(feci);

		/*
		 * Update interface statistics
		 */
		ecm_interface_stats_update(ci, sync->flow_tx_packet_count, sync->flow_tx_byte_count, sync->flow_rx_packet_count, sync->flow_rx_byte_count,
						sync->return_tx_packet_count, sync->return_tx_byte_count, sync->return_rx_packet_count, sync->return_rx_byte_count);
	}

	switch(sync->reason) {
	case PPE_DRV_STATS_SYNC_REASON_FLUSH:
		/*
		 * PPE has ended acceleration without instruction from the ECM.
		 */
		DEBUG_INFO("%px: PPE initiated final sync seen due to reason: Flush from PPE for %px\n", sync, feci);

		/*
		 * PPE Decelerated the connection
		 */
		feci->accel_ceased(feci);
		break;

	case PPE_DRV_STATS_SYNC_REASON_DESTROY:
		/*
		 * This is the final sync from the PPE for a connection whose acceleration was
		 * terminated by the ecm.
		 * NOTE: We take no action here since that is performed by the destroy message ack.
		 */
		DEBUG_INFO("%px: ECM initiated final sync seen: %d\n", ci, sync->reason);

		/*
		 * If there is no tx/rx packets to update the other linux subsystems, we shouldn't continue
		 * for the sync message which comes as a final sync for the ECM initiated destroy request.
		 * Because this means the connection is not active for sometime and adding this delta time
		 * to the conntrack timeout will update it eventhough there is no traffic for this connection.
		 * When the CT is in destroy status, find ct could cause ct
		 * destroyed again
		 */
		if ((!sync->flow_tx_packet_count && !sync->return_tx_packet_count)
				|| (ci->flags & ECM_DB_CONNECTION_FLAGS_DEFUNCT_CT_DESTROYED)) {
			ecm_front_end_connection_deref(feci);
			ecm_db_connection_deref(ci);
			return;
		}
		break;

	case PPE_DRV_STATS_SYNC_REASON_STATS:
	default:
		if (ecm_db_connection_is_routed_get(ci)) {
			/*
			 * Update the neighbour entry for source IP address
			 */
			neigh = ecm_interface_ipv6_neigh_get(feci, ECM_DB_OBJ_DIR_FROM, flow_ip);
			if (!neigh) {
				DEBUG_WARN("Neighbour entry for " ECM_IP_ADDR_OCTAL_FMT " not found\n", ECM_IP_ADDR_TO_OCTAL(flow_ip));
			} else {
				if (sync->flow_tx_packet_count) {
					DEBUG_TRACE("Neighbour entry event send for " ECM_IP_ADDR_OCTAL_FMT ": %px\n", ECM_IP_ADDR_TO_OCTAL(flow_ip), neigh);
					neigh_event_send(neigh, NULL);
				}

				neigh_release(neigh);
			}

			/*
			 * Update the neighbour entry for destination IP address
			 */
			neigh = ecm_interface_ipv6_neigh_get(feci, ECM_DB_OBJ_DIR_TO, return_ip);
			if (!neigh) {
				DEBUG_WARN("Neighbour entry for " ECM_IP_ADDR_OCTAL_FMT " not found\n", ECM_IP_ADDR_TO_OCTAL(return_ip));
			} else {
				if (sync->return_tx_packet_count) {
					DEBUG_TRACE("Neighbour entry event send for " ECM_IP_ADDR_OCTAL_FMT ": %px\n", ECM_IP_ADDR_TO_OCTAL(return_ip), neigh);
					neigh_event_send(neigh, NULL);
				}

				neigh_release(neigh);
			}
		}
	}

	/*
	 * If connection should be re-generated then we need to force a deceleration
	 */
	if (unlikely(ecm_db_connection_regeneration_required_peek(ci))) {
		DEBUG_TRACE("%px: Connection generation changing, terminating acceleration", ci);
		feci->decelerate(feci);
	}

	ecm_front_end_connection_deref(feci);
	ecm_db_connection_deref(ci);

sync_conntrack:
	;

	/*
	 * Create a tuple so as to be able to look up a conntrack connection
	 */
	memset(&tuple, 0, sizeof(tuple));
	ECM_IP_ADDR_TO_NIN6_ADDR(tuple.src.u3.in6, flow_ip)
	tuple.src.u.all = (__be16)htons(sync->flow_ident);
	tuple.src.l3num = AF_INET6;

	ECM_IP_ADDR_TO_NIN6_ADDR(tuple.dst.u3.in6, return_ip);
	tuple.dst.dir = IP_CT_DIR_ORIGINAL;
	tuple.dst.protonum = (uint8_t)sync->protocol;
	tuple.dst.u.all = (__be16)htons(sync->return_ident);

	DEBUG_TRACE("Conntrack sync, lookup conntrack connection using\n"
			"Protocol: %d\n"
			"src_addr: " ECM_IP_ADDR_OCTAL_FMT ":%d\n"
			"dest_addr: " ECM_IP_ADDR_OCTAL_FMT ":%d\n",
			(int)tuple.dst.protonum,
			ECM_IP_ADDR_TO_OCTAL(flow_ip), (int)ntohs(tuple.src.u.all),
			ECM_IP_ADDR_TO_OCTAL(return_ip), (int)ntohs(tuple.dst.u.all));

	/*
	 * Look up conntrack connection
	 */
	h = nf_conntrack_find_get(&init_net, &nf_ct_zone_dflt, &tuple);
	if (!h) {
		DEBUG_WARN("%px: PPE Sync: no conntrack connection\n", sync);
		return;
	}

	ct = nf_ct_tuplehash_to_ctrack(h);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 9, 0))
	NF_CT_ASSERT(ct->timeout.data == (unsigned long)ct);
#endif
	DEBUG_TRACE("%px: PPE Sync: conntrack connection\n", ct);

	ecm_front_end_flow_and_return_directions_get(ct, flow_ip, 6, &flow_dir, &return_dir);

	/*
	 * Only update if this is not a fixed timeout
	 * delta_jiffies is the elapsed time since the last sync of this connection.
	 */
	if (!test_bit(IPS_FIXED_TIMEOUT_BIT, &ct->status)) {
		spin_lock_bh(&ct->lock);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 9, 0))
		ct->timeout.expires += delta_jiffies;
#else
		ct->timeout += delta_jiffies;
#endif
		spin_unlock_bh(&ct->lock);
	}

	acct = nf_conn_acct_find(ct)->counter;
	if (acct) {
		spin_lock_bh(&ct->lock);
		atomic64_add(sync->flow_rx_packet_count, &acct[flow_dir].packets);
		atomic64_add(sync->flow_rx_byte_count, &acct[flow_dir].bytes);

		atomic64_add(sync->return_rx_packet_count, &acct[return_dir].packets);
		atomic64_add(sync->return_rx_byte_count, &acct[return_dir].bytes);
		spin_unlock_bh(&ct->lock);
	}

	switch (sync->protocol) {
	case IPPROTO_TCP:
		break;
	case IPPROTO_UDP:
		/*
		 * In Linux connection track, UDP flow has two timeout values:
		 * /proc/sys/net/netfilter/nf_conntrack_udp_timeout:
		 *	this is for uni-direction UDP flow, normally its value is 60 seconds
		 * /proc/sys/net/netfilter/nf_conntrack_udp_timeout_stream:
		 *	this is for bi-direction UDP flow, normally its value is 180 seconds
		 *
		 * Linux will update timer of UDP flow to stream timeout once it seen packets
		 * in reply direction. But if flow is accelerated by PPE or SFE, Linux won't
		 * see any packets. So we have to do the same thing in our stats sync message.
		 */
		if (!test_bit(IPS_ASSURED_BIT, &ct->status) && acct) {
			u_int64_t reply_pkts = atomic64_read(&acct[IP_CT_DIR_REPLY].packets);

			if (reply_pkts != 0) {
				struct nf_conntrack_l4proto *l4proto __maybe_unused;
				unsigned int *timeouts;

				set_bit(IPS_SEEN_REPLY_BIT, &ct->status);
				set_bit(IPS_ASSURED_BIT, &ct->status);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0))
				l4proto = __nf_ct_l4proto_find(AF_INET6, IPPROTO_UDP);
				timeouts = nf_ct_timeout_lookup(&init_net, ct, l4proto);

				spin_lock_bh(&ct->lock);
				ct->timeout.expires = jiffies + timeouts[UDP_CT_REPLIED];
				spin_unlock_bh(&ct->lock);
#else
				timeouts = nf_ct_timeout_lookup(ct);
				if (!timeouts) {
					timeouts = udp_get_timeouts(nf_ct_net(ct));
				}

				spin_lock_bh(&ct->lock);
				ct->timeout = jiffies + timeouts[UDP_CT_REPLIED];
				spin_unlock_bh(&ct->lock);
#endif
			}
		}
		break;
	}

	/*
	 * Release connection
	 */
	nf_ct_put(ct);
}

/*
 * ecm_ppe_ipv6_stats_callback()
 *	Callback handler from the PPE.
 */
static void ecm_ppe_ipv6_stats_callback(void *app_data, struct ppe_drv_v6_conn_sync *conn_sync)
{
	ecm_ppe_ipv6_process_one_conn_sync_msg(conn_sync);
}

/*
 * ecm_ppe_ipv6_stats_sync_req_work()
 *	Schedule delayed work to process connection stats and request next sync
 */
static void ecm_ppe_ipv6_stats_sync_req_work(struct work_struct *work)
{
	/*
	 * Prepare a ppe_ipv6_msg with CONN_STATS_SYNC_MANY request
	 */
	int i;
	struct ppe_drv_v6_conn_sync_many *cn_sync;

	spin_lock_bh(&ecm_ppe_ipv6_lock);
	if (ecm_ppe_ipv6_accelerated_count == 0) {
		spin_unlock_bh(&ecm_ppe_ipv6_lock);
		DEBUG_TRACE("There is no accelerated IPv6 connection\n");
		goto reschedule;
	}
	spin_unlock_bh(&ecm_ppe_ipv6_lock);

	cn_sync = vzalloc(sizeof(struct ppe_drv_v6_conn_sync) * ECM_PPE_IPV6_STATS_SYNC_COUNT);
	if (!cn_sync) {
		DEBUG_WARN("Memory allocation failed for ppe_drv_v6_conn_sync_many\n");
		goto reschedule;
	}

	ppe_drv_v6_conn_sync_many(cn_sync, ECM_PPE_IPV6_STATS_SYNC_COUNT);

	/*
	 * If ECM is terminating, don't process this last stats
	 */
	if (ecm_ipv6_terminate_pending) {
		DEBUG_TRACE("ecm_ipv6_terminate_pending\n");
		vfree(cn_sync);
		return;
	}

	for (i = 0; i < cn_sync->count; i++) {
		ecm_ppe_ipv6_process_one_conn_sync_msg(&cn_sync->conn_sync[i]);
	}

	vfree(cn_sync);

reschedule:
	queue_delayed_work(ecm_ppe_ipv6_workqueue, &ecm_ppe_ipv6_work, ECM_PPE_IPV6_STATS_SYNC_PERIOD);
}

/*
 * ecm_ppe_ipv6_get_accel_limit_mode()
 */
static int ecm_ppe_ipv6_get_accel_limit_mode(void *data, u64 *val)
{
	*val = ecm_ppe_ipv6_accel_limit_mode;

	return 0;
}

/*
 * ecm_ppe_ipv6_set_accel_limit_mode()
 */
static int ecm_ppe_ipv6_set_accel_limit_mode(void *data, u64 val)
{
	DEBUG_TRACE("ecm_ppe_ipv6_accel_limit_mode = %x\n", (int)val);

	/*
	 * Check that only valid bits are set.
	 * It's fine for no bits to be set as that suggests no modes are wanted.
	 */
	if (val && (val ^ (ECM_FRONT_END_ACCEL_LIMIT_MODE_FIXED | ECM_FRONT_END_ACCEL_LIMIT_MODE_UNLIMITED))) {
		DEBUG_WARN("ecm_ppe_ipv6_accel_limit_mode = %x bad\n", (int)val);
		return -EINVAL;
	}

	ecm_ppe_ipv6_accel_limit_mode = (int)val;

	return 0;
}

/*
 * Debugfs attribute for accel limit mode.
 */
DEFINE_SIMPLE_ATTRIBUTE(ecm_ppe_ipv6_accel_limit_mode_fops, ecm_ppe_ipv6_get_accel_limit_mode, ecm_ppe_ipv6_set_accel_limit_mode, "%llu\n");

/*
 * ecm_ppe_ipv6_get_accel_cmd_average_millis()
 */
static ssize_t ecm_ppe_ipv6_get_accel_cmd_avg_millis(struct file *file,
								char __user *user_buf,
								size_t sz, loff_t *ppos)
{
	unsigned long set;
	unsigned long samples;
	unsigned long avg;
	char *buf;
	int ret;

	buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (!buf) {
		return -ENOMEM;
	}

	/*
	 * Operate under our locks.
	 * Compute the average of the samples taken and seed the next set of samples with the result of this one.
	 */
	spin_lock_bh(&ecm_ppe_ipv6_lock);
	samples = ecm_ppe_ipv6_accel_cmd_time_avg_samples;
	set = ecm_ppe_ipv6_accel_cmd_time_avg_set;
	ecm_ppe_ipv6_accel_cmd_time_avg_samples /= ecm_ppe_ipv6_accel_cmd_time_avg_set;
	ecm_ppe_ipv6_accel_cmd_time_avg_set = 1;
	avg = ecm_ppe_ipv6_accel_cmd_time_avg_samples;
	spin_unlock_bh(&ecm_ppe_ipv6_lock);

	/*
	 * Convert average jiffies to milliseconds
	 */
	avg *= 1000;
	avg /= HZ;

	ret = snprintf(buf, (ssize_t)PAGE_SIZE, "avg=%lu\tsamples=%lu\tset_size=%lu\n", avg, samples, set);
	if (ret < 0) {
		kfree(buf);
		return -EFAULT;
	}

	ret = simple_read_from_buffer(user_buf, sz, ppos, buf, ret);
	kfree(buf);
	return ret;
}

/*
 * File operations for accel command average time.
 */
static struct file_operations ecm_ppe_ipv6_accel_cmd_avg_millis_fops = {
	.read = ecm_ppe_ipv6_get_accel_cmd_avg_millis,
};

/*
 * ecm_ppe_ipv6_get_decel_cmd_average_millis()
 */
static ssize_t ecm_ppe_ipv6_get_decel_cmd_avg_millis(struct file *file,
								char __user *user_buf,
								size_t sz, loff_t *ppos)
{
	unsigned long set;
	unsigned long samples;
	unsigned long avg;
	char *buf;
	int ret;

	buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (!buf) {
		return -ENOMEM;
	}

	/*
	 * Operate under our locks.
	 * Compute the average of the samples taken and seed the next set of samples with the result of this one.
	 */
	spin_lock_bh(&ecm_ppe_ipv6_lock);
	samples = ecm_ppe_ipv6_decel_cmd_time_avg_samples;
	set = ecm_ppe_ipv6_decel_cmd_time_avg_set;
	ecm_ppe_ipv6_decel_cmd_time_avg_samples /= ecm_ppe_ipv6_decel_cmd_time_avg_set;
	ecm_ppe_ipv6_decel_cmd_time_avg_set = 1;
	avg = ecm_ppe_ipv6_decel_cmd_time_avg_samples;
	spin_unlock_bh(&ecm_ppe_ipv6_lock);

	/*
	 * Convert average jiffies to milliseconds
	 */
	avg *= 1000;
	avg /= HZ;

	ret = snprintf(buf, (ssize_t)PAGE_SIZE, "avg=%lu\tsamples=%lu\tset_size=%lu\n", avg, samples, set);
	if (ret < 0) {
		kfree(buf);
		return -EFAULT;
	}

	ret = simple_read_from_buffer(user_buf, sz, ppos, buf, ret);
	kfree(buf);
	return ret;
}

/*
 * File operations for decel command average time.
 */
static struct file_operations ecm_ppe_ipv6_decel_cmd_avg_millis_fops = {
	.read = ecm_ppe_ipv6_get_decel_cmd_avg_millis,
};

/*
 * ecm_ppe_ipv6_get_stats_request_counter()
 */
static ssize_t ecm_ppe_ipv6_get_stats_request_counter(struct file *file,
								char __user *user_buf,
								size_t sz, loff_t *ppos)
{
	char *buf;
	int ret;

	buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (!buf) {
		return -ENOMEM;
	}

	ret = snprintf(buf, (ssize_t)PAGE_SIZE, "success=%lu\tfail=%lu\tnack=%lu\t\n",
			ecm_ppe_ipv6_stats_request_success, ecm_ppe_ipv6_stats_request_fail,
			ecm_ppe_ipv6_stats_request_nack);
	if (ret < 0) {
		kfree(buf);
		return -EFAULT;
	}

	ret = simple_read_from_buffer(user_buf, sz, ppos, buf, ret);
	kfree(buf);
	return ret;
}

/*
 * File operations for decel command average time.
 */
static struct file_operations ecm_ppe_ipv6_stats_request_counter_fops = {
	.read = ecm_ppe_ipv6_get_stats_request_counter,
};

/*
 * ecm_ppe_ipv6_sync_queue_init
 *	Initialize the workqueue for ipv6 stats sync
 */
static bool ecm_ppe_ipv6_sync_queue_init(void)
{
	ecm_ppe_ipv6_workqueue = create_singlethread_workqueue("ecm_ppe_ipv6_workqueue");
	if (!ecm_ppe_ipv6_workqueue) {
		return false;
	}

	INIT_DELAYED_WORK(&ecm_ppe_ipv6_work, ecm_ppe_ipv6_stats_sync_req_work);
	queue_delayed_work(ecm_ppe_ipv6_workqueue, &ecm_ppe_ipv6_work, ECM_PPE_IPV6_STATS_SYNC_PERIOD);

	return true;
}

/*
 * ecm_ppe_ipv6_sync_queue_exit
 *	 the workqueue for ipv6 stats sync
 */
static void ecm_ppe_ipv6_sync_queue_exit(void)
{
	/*
	 * Cancel the conn sync req work and destroy workqueue
	 */
	cancel_delayed_work_sync(&ecm_ppe_ipv6_work);
	destroy_workqueue(ecm_ppe_ipv6_workqueue);
}

/*
 * ecm_ppe_ipv6_init()
 */
int ecm_ppe_ipv6_init(struct dentry *dentry)
{
	int result = -1;

	if (!ecm_front_end_is_feature_supported(ECM_FE_FEATURE_PPE)) {
		DEBUG_INFO("PPE IPv6 is disabled\n");
		return 0;
	}

	DEBUG_INFO("ECM PPE IPv6 init\n");

	ecm_ppe_ipv6_dentry = debugfs_create_dir("ecm_ppe_ipv6", dentry);
	if (!ecm_ppe_ipv6_dentry) {
		DEBUG_ERROR("Failed to create ecm ppe ipv6 directory in debugfs\n");
		return result;
	}

	if (!debugfs_create_u32("no_action_limit_default", S_IRUGO | S_IWUSR, ecm_ppe_ipv6_dentry,
					(u32 *)&ecm_ppe_ipv6_no_action_limit_default)) {
		DEBUG_ERROR("Failed to create ecm ppe ipv6 no_action_limit_default file in debugfs\n");
		goto task_cleanup_1;
	}

	if (!debugfs_create_u32("driver_fail_limit_default", S_IRUGO | S_IWUSR, ecm_ppe_ipv6_dentry,
					(u32 *)&ecm_ppe_ipv6_driver_fail_limit_default)) {
		DEBUG_ERROR("Failed to create ecm ppe ipv6 driver_fail_limit_default file in debugfs\n");
		goto task_cleanup_1;
	}

	if (!debugfs_create_u32("nack_limit_default", S_IRUGO | S_IWUSR, ecm_ppe_ipv6_dentry,
					(u32 *)&ecm_ppe_ipv6_nack_limit_default)) {
		DEBUG_ERROR("Failed to create ecm ppe ipv6 nack_limit_default file in debugfs\n");
		goto task_cleanup_1;
	}

	if (!debugfs_create_u32("accelerated_count", S_IRUGO, ecm_ppe_ipv6_dentry,
					(u32 *)&ecm_ppe_ipv6_accelerated_count)) {
		DEBUG_ERROR("Failed to create ecm ppe ipv6 accelerated_count file in debugfs\n");
		goto task_cleanup_1;
	}

	if (!debugfs_create_u32("pending_accel_count", S_IRUGO, ecm_ppe_ipv6_dentry,
					(u32 *)&ecm_ppe_ipv6_pending_accel_count)) {
		DEBUG_ERROR("Failed to create ecm ppe ipv6 pending_accel_count file in debugfs\n");
		goto task_cleanup_1;
	}

	if (!debugfs_create_u32("pending_decel_count", S_IRUGO, ecm_ppe_ipv6_dentry,
					(u32 *)&ecm_ppe_ipv6_pending_decel_count)) {
		DEBUG_ERROR("Failed to create ecm ppe ipv6 pending_decel_count file in debugfs\n");
		goto task_cleanup_1;
	}

	if (!debugfs_create_file("accel_limit_mode", S_IRUGO | S_IWUSR, ecm_ppe_ipv6_dentry,
					NULL, &ecm_ppe_ipv6_accel_limit_mode_fops)) {
		DEBUG_ERROR("Failed to create ecm ppe ipv6 accel_limit_mode file in debugfs\n");
		goto task_cleanup_1;
	}

	if (!debugfs_create_file("accel_cmd_avg_millis", S_IRUGO, ecm_ppe_ipv6_dentry,
					NULL, &ecm_ppe_ipv6_accel_cmd_avg_millis_fops)) {
		DEBUG_ERROR("Failed to create ecm ppe ipv6 accel_cmd_avg_millis file in debugfs\n");
		goto task_cleanup_1;
	}

	if (!debugfs_create_file("decel_cmd_avg_millis", S_IRUGO, ecm_ppe_ipv6_dentry,
					NULL, &ecm_ppe_ipv6_decel_cmd_avg_millis_fops)) {
		DEBUG_ERROR("Failed to create ecm ppe ipv6 decel_cmd_avg_millis file in debugfs\n");
		goto task_cleanup_1;
	}

	if (!ecm_ppe_ported_ipv6_debugfs_init(ecm_ppe_ipv6_dentry)) {
		DEBUG_ERROR("Failed to create ecm ported files in debugfs\n");
		goto task_cleanup_1;
	}

	if (!debugfs_create_file("stats_request_counter", S_IRUGO, ecm_ppe_ipv6_dentry,
					NULL, &ecm_ppe_ipv6_stats_request_counter_fops)) {
		DEBUG_ERROR("Failed to create ecm ppe ipv6 stats_request_counter file in debugfs\n");
		goto task_cleanup_1;
	}

	if (!debugfs_create_u32("vlan_passthrough_set", S_IRUGO | S_IWUSR, ecm_ppe_ipv6_dentry,
					(u32 *)&ecm_ppe_ipv6_vlan_passthrough_enable)) {
		DEBUG_ERROR("Failed to create ecm ppe ipv6 vlan passthrough file in debugfs\n");
		goto task_cleanup_1;
	}

#ifdef ECM_NON_PORTED_SUPPORT_ENABLE
	if (!ecm_ppe_non_ported_ipv6_debugfs_init(ecm_ppe_ipv6_dentry)) {
		DEBUG_ERROR("Failed to create ecm non-ported files in debugfs\n");
		goto task_cleanup_1;
	}
#endif

	/*
	 * Register this module with the Linux PPE Network driver.
	 * Notify manager should be registered before the netfilter hooks. Because there
	 * is a possibility that the ECM can try to send acceleration messages to the
	 * acceleration engine without having an acceleration engine manager.
	 */
	/* TODO: Invoke PPE stats register api */
	if (!ppe_drv_v6_stats_callback_register(ecm_ppe_ipv6_stats_callback, NULL)) {
		DEBUG_ERROR("Failed to register stats callback\n");
		goto task_cleanup_1;
	}

	if (!ecm_ppe_ipv6_sync_queue_init()) {
		DEBUG_ERROR("Failed to create ecm ipv6 connection sync workqueue\n");
		goto task_cleanup_2;
	}

	return 0;

task_cleanup_2:
	ppe_drv_v6_stats_callback_unregister();
task_cleanup_1:
	debugfs_remove_recursive(ecm_ppe_ipv6_dentry);
	return result;
}
EXPORT_SYMBOL(ecm_ppe_ipv6_init);

/*
 * ecm_ppe_ipv6_exit()
 */
void ecm_ppe_ipv6_exit(void)
{
	if (!ecm_front_end_is_feature_supported(ECM_FE_FEATURE_PPE)) {
		DEBUG_INFO("PPE IPv6 is disabled\n");
		return;
	}

	DEBUG_INFO("ECM PPE IPv6 Module exit\n");

	/*
	 * Unregister from the Linux PPE Network driver
	 */
	ppe_drv_v6_stats_callback_unregister();

	/*
	 * Remove the debugfs files recursively.
	 */
	if (ecm_ppe_ipv6_dentry) {
		debugfs_remove_recursive(ecm_ppe_ipv6_dentry);
	}

	/*
	 * Clean up the stats sync queue/work
	 */
	ecm_ppe_ipv6_sync_queue_exit();
}
EXPORT_SYMBOL(ecm_ppe_ipv6_exit);
