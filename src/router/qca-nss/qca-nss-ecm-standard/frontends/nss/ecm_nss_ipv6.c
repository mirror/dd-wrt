/*
 **************************************************************************
 * Copyright (c) 2014-2021 The Linux Foundation. All rights reserved.
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
#define DEBUG_LEVEL ECM_NSS_IPV6_DEBUG_LEVEL

#include <nss_api_if.h>
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
#include "ecm_nss_common.h"
#include "ecm_nss_ported_ipv6.h"
#ifdef ECM_MULTICAST_ENABLE
#include "ecm_nss_multicast_ipv6.h"
#endif
#ifdef ECM_NON_PORTED_SUPPORT_ENABLE
#include "ecm_nss_non_ported_ipv6.h"
#endif
#include "ecm_front_end_common.h"
#include "ecm_front_end_ipv6.h"
#include "ecm_ipv6.h"

#define ECM_NSS_IPV6_STATS_SYNC_PERIOD msecs_to_jiffies(1000)
#define ECM_NSS_IPV6_STATS_SYNC_UDELAY 4000	/* Delay for 4ms */

int ecm_nss_ipv6_no_action_limit_default = 250;		/* Default no-action limit. */
int ecm_nss_ipv6_driver_fail_limit_default = 250;		/* Default driver fail limit. */
int ecm_nss_ipv6_nack_limit_default = 250;			/* Default nack limit. */
int ecm_nss_ipv6_accelerated_count = 0;			/* Total offloads */
int ecm_nss_ipv6_pending_accel_count = 0;			/* Total pending offloads issued to the NSS / awaiting completion */
int ecm_nss_ipv6_pending_decel_count = 0;			/* Total pending deceleration requests issued to the NSS / awaiting completion */
int ecm_nss_ipv6_vlan_passthrough_enable = 0;		/* VLAN passthrough feature enable or disable flag */

/*
 * Limiting the acceleration of connections.
 *
 * By default there is no acceleration limiting.
 * This means that when ECM has more connections (that can be accelerated) than the acceleration
 * engine will allow the ECM will continue to try to accelerate.
 * In this scenario the acceleration engine will begin removal of existing rules to make way for new ones.
 * When the accel_limit_mode is set to FIXED ECM will not permit more rules to be issued than the engine will allow.
 */
uint32_t ecm_nss_ipv6_accel_limit_mode = ECM_FRONT_END_ACCEL_LIMIT_MODE_UNLIMITED;

/*
 * Locking of the classifier - concurrency control for file global parameters.
 * NOTE: It is safe to take this lock WHILE HOLDING a feci->lock.  The reverse is NOT SAFE.
 */
DEFINE_SPINLOCK(ecm_nss_ipv6_lock);			/* Protect against SMP access between netfilter, events and private threaded function. */

/*
 * Workqueue for the connection sync
 */
struct workqueue_struct *ecm_nss_ipv6_workqueue;
struct delayed_work ecm_nss_ipv6_work;
struct nss_ipv6_msg *ecm_nss_ipv6_sync_req_msg;
static unsigned long int ecm_nss_ipv6_next_req_time;
static unsigned long int ecm_nss_ipv6_roll_check_jiffies;
static unsigned long int ecm_nss_ipv6_stats_request_success = 0;	/* Number of success stats request */
static unsigned long int ecm_nss_ipv6_stats_request_fail = 0;		/* Number of failed stats request */
static unsigned long int ecm_nss_ipv6_stats_request_nack = 0;		/* Number of NACK'd stats request */

/*
 * NSS driver linkage
 */
struct nss_ctx_instance *ecm_nss_ipv6_nss_ipv6_mgr = NULL;

static unsigned long ecm_nss_ipv6_accel_cmd_time_avg_samples = 0;	/* Sum of time taken for the set of accel command samples, used to compute average time for an accel command to complete */
static unsigned long ecm_nss_ipv6_accel_cmd_time_avg_set = 1;	/* How many samples in the set */
static unsigned long ecm_nss_ipv6_decel_cmd_time_avg_samples = 0;	/* Sum of time taken for the set of accel command samples, used to compute average time for an accel command to complete */
static unsigned long ecm_nss_ipv6_decel_cmd_time_avg_set = 1;	/* How many samples in the set */

/*
 * Debugfs dentry object.
 */
static struct dentry *ecm_nss_ipv6_dentry;

/*
 * ecm_nss_ipv6_accel_done_time_update()
 *	Record how long the command took to complete, updating average samples
 */
void ecm_nss_ipv6_accel_done_time_update(struct ecm_front_end_connection_instance *feci)
{
	unsigned long delta;

	/*
	 * How long did it take the command to complete?
	 */
	spin_lock_bh(&feci->lock);
	feci->stats.cmd_time_completed = jiffies;
	delta = feci->stats.cmd_time_completed - feci->stats.cmd_time_begun;
	spin_unlock_bh(&feci->lock);

	spin_lock_bh(&ecm_nss_ipv6_lock);
	ecm_nss_ipv6_accel_cmd_time_avg_samples += delta;
	ecm_nss_ipv6_accel_cmd_time_avg_set++;
	spin_unlock_bh(&ecm_nss_ipv6_lock);
}

/*
 * ecm_nss_ipv6_deccel_done_time_update()
 *	Record how long the command took to complete, updating average samples
 */
void ecm_nss_ipv6_decel_done_time_update(struct ecm_front_end_connection_instance *feci)
{
	unsigned long delta;

	/*
	 * How long did it take the command to complete?
	 */
	spin_lock_bh(&feci->lock);
	feci->stats.cmd_time_completed = jiffies;
	delta = feci->stats.cmd_time_completed - feci->stats.cmd_time_begun;
	spin_unlock_bh(&feci->lock);

	spin_lock_bh(&ecm_nss_ipv6_lock);
	ecm_nss_ipv6_decel_cmd_time_avg_samples += delta;
	ecm_nss_ipv6_decel_cmd_time_avg_set++;
	spin_unlock_bh(&ecm_nss_ipv6_lock);
}

/*
 * ecm_nss_ipv6_process_one_conn_sync_msg()
 *	Process one connection sync message.
 */
static inline void ecm_nss_ipv6_process_one_conn_sync_msg(struct nss_ipv6_conn_sync *sync)
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

	ECM_NSS_IPV6_ADDR_TO_IP_ADDR(flow_ip, sync->flow_ip);
	ECM_NSS_IPV6_ADDR_TO_IP_ADDR(return_ip, sync->return_ip);

	/*
	 * Look up ecm connection with a view to synchronising the connection, classifier and data tracker.
	 * Note that we use _xlate versions for destination - for egressing connections this would be the wan IP address,
	 * but for ingressing this would be the LAN side (non-nat'ed) address and is what we need for lookup of our connection.
	 */
	DEBUG_INFO("%px: NSS Sync, lookup connection using\n" \
			"Protocol: %d\n" \
			"src_addr: " ECM_IP_ADDR_OCTAL_FMT ":%d\n" \
			"dest_addr: " ECM_IP_ADDR_OCTAL_FMT ":%d\n",
			sync,
			(int)sync->protocol,
			ECM_IP_ADDR_TO_OCTAL(flow_ip), (int)sync->flow_ident,
			ECM_IP_ADDR_TO_OCTAL(return_ip), (int)sync->return_ident);

	ci = ecm_db_connection_find_and_ref(flow_ip, return_ip, sync->protocol, (int)sync->flow_ident, (int)sync->return_ident);
	if (!ci) {
		DEBUG_TRACE("%px: NSS Sync: no connection\n", sync);
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
#ifdef ECM_MULTICAST_ENABLE
		if (ecm_ip_addr_is_multicast(return_ip)) {
			/*
			 * The amount of data *sent* by the ECM multicast connection 'from' side is the amount the NSS has *received* in the 'flow' direction.
			 */
			ecm_db_multicast_connection_data_totals_update(ci, true, sync->flow_rx_byte_count, sync->flow_rx_packet_count);
			ecm_db_multicast_connection_data_totals_update(ci, true, sync->return_rx_byte_count, sync->return_rx_packet_count);
			ecm_db_multicast_connection_interface_heirarchy_stats_update(ci, sync->flow_rx_byte_count, sync->flow_rx_packet_count);

			/*
			 * Update interface statistics
			 */
			ecm_interface_multicast_stats_update(ci, sync->flow_tx_packet_count, sync->flow_tx_byte_count, sync->flow_rx_packet_count, sync->flow_rx_byte_count,
										sync->return_tx_packet_count, sync->return_tx_byte_count, sync->return_rx_packet_count, sync->return_rx_byte_count);

			ECM_IP_ADDR_TO_NIN6_ADDR(origin6, flow_ip);
			ECM_IP_ADDR_TO_NIN6_ADDR(group6, return_ip);

			/*
			 * Update IP multicast routing cache stats
			 */
			ip6mr_mfc_stats_update(&init_net, &origin6, &group6, sync->flow_rx_packet_count,
								 sync->flow_rx_byte_count, sync->flow_rx_packet_count, sync->flow_rx_byte_count);
		} else {

			/*
			 * The amount of data *sent* by the ECM connection 'from' side is the amount the NSS has *received* in the 'flow' direction.
			 */
			ecm_db_connection_data_totals_update(ci, true, sync->flow_rx_byte_count, sync->flow_rx_packet_count);

			/*
			 * The amount of data *sent* by the ECM connection 'to' side is the amount the NSS has *received* in the 'return' direction.
			 */
			ecm_db_connection_data_totals_update(ci, false, sync->return_rx_byte_count, sync->return_rx_packet_count);

			/*
			 * Update interface statistics
			 */
			ecm_interface_stats_update(ci, sync->flow_tx_packet_count, sync->flow_tx_byte_count, sync->flow_rx_packet_count, sync->flow_rx_byte_count,
						sync->return_tx_packet_count, sync->return_tx_byte_count, sync->return_rx_packet_count, sync->return_rx_byte_count);
		}

		/*
		 * As packets have been accelerated we have seen some action.
		 */
		ecm_front_end_connection_action_seen(feci);
#else
		/*
		 * The amount of data *sent* by the ECM connection 'from' side is the amount the NSS has *received* in the 'flow' direction.
		 */
		ecm_db_connection_data_totals_update(ci, true, sync->flow_rx_byte_count, sync->flow_rx_packet_count);

		/*
		 * The amount of data *sent* by the ECM connection 'to' side is the amount the NSS has *received* in the 'return' direction.
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
#endif
	}

	switch(sync->reason) {
	case NSS_IPV6_RULE_SYNC_REASON_DESTROY:
		/*
		 * This is the final sync from the NSS for a connection whose acceleration was
		 * terminated by the ecm.
		 * NOTE: We take no action here since that is performed by the destroy message ack.
		 */
		DEBUG_INFO("%px: ECM initiated final sync seen: %d\n", ci, sync->reason);
		break;
	case NSS_IPV6_RULE_SYNC_REASON_FLUSH:
	case NSS_IPV6_RULE_SYNC_REASON_EVICT:
		/*
		 * NSS has ended acceleration without instruction from the ECM.
		 */
		DEBUG_INFO("%px: NSS Initiated final sync seen: %d\n", ci, sync->reason);

		/*
		 * NSS Decelerated the connection
		 */
		feci->accel_ceased(feci);
		break;
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

#ifdef ECM_MULTICAST_ENABLE
			/*
			 * Update the neighbour entry for destination IP address
			 */
			if (!ecm_ip_addr_is_multicast(return_ip)) {
				neigh = ecm_interface_ipv6_neigh_get(feci, ECM_DB_OBJ_DIR_TO,  return_ip);
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
#else
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
#endif
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
		DEBUG_WARN("%px: NSS Sync: no conntrack connection\n", sync);
		return;
	}

	ct = nf_ct_tuplehash_to_ctrack(h);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 9, 0))
	NF_CT_ASSERT(ct->timeout.data == (unsigned long)ct);
#endif
	DEBUG_TRACE("%px: NSS Sync: conntrack connection\n", ct);

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
		spin_lock_bh(&ct->lock);
		if (ct->proto.tcp.seen[flow_dir].td_maxwin < sync->flow_max_window) {
			ct->proto.tcp.seen[flow_dir].td_maxwin = sync->flow_max_window;
		}
		if ((int32_t)(ct->proto.tcp.seen[flow_dir].td_end - sync->flow_end) < 0) {
			ct->proto.tcp.seen[flow_dir].td_end = sync->flow_end;
		}
		if ((int32_t)(ct->proto.tcp.seen[flow_dir].td_maxend - sync->flow_max_end) < 0) {
			ct->proto.tcp.seen[flow_dir].td_maxend = sync->flow_max_end;
		}
		if (ct->proto.tcp.seen[return_dir].td_maxwin < sync->return_max_window) {
			ct->proto.tcp.seen[return_dir].td_maxwin = sync->return_max_window;
		}
		if ((int32_t)(ct->proto.tcp.seen[return_dir].td_end - sync->return_end) < 0) {
			ct->proto.tcp.seen[return_dir].td_end = sync->return_end;
		}
		if ((int32_t)(ct->proto.tcp.seen[return_dir].td_maxend - sync->return_max_end) < 0) {
			ct->proto.tcp.seen[return_dir].td_maxend = sync->return_max_end;
		}
		spin_unlock_bh(&ct->lock);
		break;
	case IPPROTO_UDP:
		/*
		 * In Linux connection track, UDP flow has two timeout values:
		 * /proc/sys/net/netfilter/nf_conntrack_udp_timeout:
		 * 	this is for uni-direction UDP flow, normally its value is 60 seconds
		 * /proc/sys/net/netfilter/nf_conntrack_udp_timeout_stream:
		 * 	this is for bi-direction UDP flow, normally its value is 180 seconds
		 *
		 * Linux will update timer of UDP flow to stream timeout once it seen packets
		 * in reply direction. But if flow is accelerated by NSS or SFE, Linux won't
		 * see any packets. So we have to do the same thing in our stats sync message.
		 */
		if (!test_bit(IPS_ASSURED_BIT, &ct->status) && acct) {
			u_int64_t reply_pkts = atomic64_read(&acct[IP_CT_DIR_REPLY].packets);

			if (reply_pkts != 0) {
				struct nf_conntrack_l4proto *l4proto __maybe_unused;
				unsigned int *timeouts;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0))
				l4proto = __nf_ct_l4proto_find(AF_INET6, IPPROTO_UDP);
				timeouts = nf_ct_timeout_lookup(&init_net, ct, l4proto);

				spin_lock_bh(&ct->lock);
				ct->timeout.expires = jiffies + timeouts[UDP_CT_REPLIED];
				spin_unlock_bh(&ct->lock);
#else
				timeouts = nf_ct_timeout_lookup(ct);
				if (!timeouts) {
					struct nf_udp_net *un = nf_udp_pernet(nf_ct_net(ct));
					timeouts = un->timeouts;
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
 * ecm_nss_ipv6_net_dev_callback()
 *	Callback handler from the NSS.
 */
static void ecm_nss_ipv6_net_dev_callback(void *app_data, struct nss_ipv6_msg *nim)
{
	struct nss_ipv6_conn_sync *sync;
	/*
	 * Only respond to sync messages
	 */
	if (nim->cm.type != NSS_IPV6_RX_CONN_STATS_SYNC_MSG) {
		return;
	}
	sync = &nim->msg.conn_stats;
	ecm_nss_ipv6_process_one_conn_sync_msg(sync);
}

/*
 * ecm_nss_ipv6_connection_sync_many_callback()
 *	Callback for conn_sync_many request message
 */
static void ecm_nss_ipv6_connection_sync_many_callback(void *app_data, struct nss_ipv6_msg *nim)
{
	struct nss_ipv6_conn_sync_many_msg *nicsm = &nim->msg.conn_stats_many;
	int i;

	/*
	 * If ECM is terminating, don't process this last stats
	 */
	if (ecm_ipv6_terminate_pending) {
		return;
	}

	if (nim->cm.response == NSS_CMN_RESPONSE_ACK) {
		for (i = 0; i < nicsm->count; i++) {
			ecm_nss_ipv6_process_one_conn_sync_msg(&nicsm->conn_sync[i]);
		}
		ecm_nss_ipv6_sync_req_msg->msg.conn_stats_many.index = nicsm->next;
	} else {
		/*
		 * We get a NACK from FW, which should not happen, restart the request
		 */
		DEBUG_WARN("IPv6 conn stats request failed, restarting\n");
		ecm_nss_ipv6_stats_request_nack++;
		ecm_nss_ipv6_sync_req_msg->msg.conn_stats_many.index = 0;
	}
	queue_delayed_work(ecm_nss_ipv6_workqueue, &ecm_nss_ipv6_work, 0);
}

/*
 * ecm_nss_ipv6_stats_sync_req_work()
 *	Schedule delayed work to process connection stats and request next sync
 */
static void ecm_nss_ipv6_stats_sync_req_work(struct work_struct *work)
{
	/*
	 * Prepare a nss_ipv6_msg with CONN_STATS_SYNC_MANY request
	 */
	struct nss_ipv6_conn_sync_many_msg *nicsm_req = &ecm_nss_ipv6_sync_req_msg->msg.conn_stats_many;
	nss_tx_status_t nss_tx_status;
	int retry = 3;
	unsigned long int current_jiffies;

	spin_lock_bh(&ecm_nss_ipv6_lock);
	if (ecm_nss_ipv6_accelerated_count == 0) {
		spin_unlock_bh(&ecm_nss_ipv6_lock);
		DEBUG_TRACE("There is no accelerated IPv6 connection\n");
		goto reschedule;
	}
	spin_unlock_bh(&ecm_nss_ipv6_lock);

	msleep_interruptible(ECM_NSS_IPV6_STATS_SYNC_UDELAY / 1000);

	/*
	 * If index is 0, we are starting a new round, but if we still have time remain
	 * in this round, sleep until it ends
	 */
	if (nicsm_req->index == 0) {
		current_jiffies = jiffies;

		if (time_is_after_jiffies(ecm_nss_ipv6_roll_check_jiffies))  {
			ecm_nss_ipv6_next_req_time = jiffies + ECM_NSS_IPV6_STATS_SYNC_PERIOD;
		}

		if (time_after(ecm_nss_ipv6_next_req_time, current_jiffies)) {
			msleep_interruptible(jiffies_to_msecs(ecm_nss_ipv6_next_req_time - current_jiffies));
		}
		ecm_nss_ipv6_roll_check_jiffies = jiffies;
		ecm_nss_ipv6_next_req_time = ecm_nss_ipv6_roll_check_jiffies + ECM_NSS_IPV6_STATS_SYNC_PERIOD;
	}

	while (retry) {
		if (ecm_ipv6_terminate_pending) {
			return;
		}
		nss_tx_status = nss_ipv6_tx_with_size(ecm_nss_ipv6_nss_ipv6_mgr, ecm_nss_ipv6_sync_req_msg, PAGE_SIZE);
		if (nss_tx_status == NSS_TX_SUCCESS) {
			ecm_nss_ipv6_stats_request_success++;
			return;
		}
		ecm_nss_ipv6_stats_request_fail++;
		retry--;
		DEBUG_TRACE("TX_NOT_OKAY, try again later\n");
		usleep_range(100, 200);
	}

reschedule:
	/*
	 * TX failed after retries, reschedule ourselves with fresh start
	 */
	nicsm_req->count = 0;
	nicsm_req->index = 0;
	queue_delayed_work(ecm_nss_ipv6_workqueue, &ecm_nss_ipv6_work, ECM_NSS_IPV6_STATS_SYNC_PERIOD);
}

/*
 * ecm_nss_ipv6_get_accel_limit_mode()
 */
static int ecm_nss_ipv6_get_accel_limit_mode(void *data, u64 *val)
{
	*val = ecm_nss_ipv6_accel_limit_mode;

	return 0;
}

/*
 * ecm_nss_ipv6_set_accel_limit_mode()
 */
static int ecm_nss_ipv6_set_accel_limit_mode(void *data, u64 val)
{
	DEBUG_TRACE("ecm_nss_ipv6_accel_limit_mode = %x\n", (int)val);

	/*
	 * Check that only valid bits are set.
	 * It's fine for no bits to be set as that suggests no modes are wanted.
	 */
	if (val && (val ^ (ECM_FRONT_END_ACCEL_LIMIT_MODE_FIXED | ECM_FRONT_END_ACCEL_LIMIT_MODE_UNLIMITED))) {
		DEBUG_WARN("ecm_nss_ipv6_accel_limit_mode = %x bad\n", (int)val);
		return -EINVAL;
	}

	ecm_nss_ipv6_accel_limit_mode = (int)val;

	return 0;
}

/*
 * Debugfs attribute for accel limit mode.
 */
DEFINE_SIMPLE_ATTRIBUTE(ecm_nss_ipv6_accel_limit_mode_fops, ecm_nss_ipv6_get_accel_limit_mode, ecm_nss_ipv6_set_accel_limit_mode, "%llu\n");

/*
 * ecm_nss_ipv6_get_accel_cmd_average_millis()
 */
static ssize_t ecm_nss_ipv6_get_accel_cmd_avg_millis(struct file *file,
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
	spin_lock_bh(&ecm_nss_ipv6_lock);
	samples = ecm_nss_ipv6_accel_cmd_time_avg_samples;
	set = ecm_nss_ipv6_accel_cmd_time_avg_set;
	ecm_nss_ipv6_accel_cmd_time_avg_samples /= ecm_nss_ipv6_accel_cmd_time_avg_set;
	ecm_nss_ipv6_accel_cmd_time_avg_set = 1;
	avg = ecm_nss_ipv6_accel_cmd_time_avg_samples;
	spin_unlock_bh(&ecm_nss_ipv6_lock);

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
static struct file_operations ecm_nss_ipv6_accel_cmd_avg_millis_fops = {
	.read = ecm_nss_ipv6_get_accel_cmd_avg_millis,
};

/*
 * ecm_nss_ipv6_get_decel_cmd_average_millis()
 */
static ssize_t ecm_nss_ipv6_get_decel_cmd_avg_millis(struct file *file,
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
	spin_lock_bh(&ecm_nss_ipv6_lock);
	samples = ecm_nss_ipv6_decel_cmd_time_avg_samples;
	set = ecm_nss_ipv6_decel_cmd_time_avg_set;
	ecm_nss_ipv6_decel_cmd_time_avg_samples /= ecm_nss_ipv6_decel_cmd_time_avg_set;
	ecm_nss_ipv6_decel_cmd_time_avg_set = 1;
	avg = ecm_nss_ipv6_decel_cmd_time_avg_samples;
	spin_unlock_bh(&ecm_nss_ipv6_lock);

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
static struct file_operations ecm_nss_ipv6_decel_cmd_avg_millis_fops = {
	.read = ecm_nss_ipv6_get_decel_cmd_avg_millis,
};

/*
 * ecm_nss_ipv6_get_stats_request_counter()
 */
static ssize_t ecm_nss_ipv6_get_stats_request_counter(struct file *file,
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
			ecm_nss_ipv6_stats_request_success, ecm_nss_ipv6_stats_request_fail,
			ecm_nss_ipv6_stats_request_nack);
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
static struct file_operations ecm_nss_ipv6_stats_request_counter_fops = {
	.read = ecm_nss_ipv6_get_stats_request_counter,
};

/*
 * ecm_nss_ipv6_sync_queue_init
 *	Initialize the workqueue for ipv6 stats sync
 */
static bool ecm_nss_ipv6_sync_queue_init(void)
{
	struct nss_ipv6_conn_sync_many_msg *nicsm;

	/*
	 * Setup the connection sync msg/work/workqueue
	 */
	ecm_nss_ipv6_sync_req_msg = kzalloc(PAGE_SIZE, GFP_KERNEL);
	if (!ecm_nss_ipv6_sync_req_msg) {
		return false;
	}

	/*
	 * Register the conn_sync_many message callback
	 */
	nss_ipv6_conn_sync_many_notify_register(ecm_nss_ipv6_connection_sync_many_callback);

	nss_ipv6_msg_init(ecm_nss_ipv6_sync_req_msg, NSS_IPV6_RX_INTERFACE,
		NSS_IPV6_TX_CONN_STATS_SYNC_MANY_MSG,
		sizeof(struct nss_ipv6_conn_sync_many_msg),
		ecm_nss_ipv6_connection_sync_many_callback,
		NULL);

	nicsm = &ecm_nss_ipv6_sync_req_msg->msg.conn_stats_many;

	/*
	 * Start with index 0 and calculate the size of the conn stats array
	 */
	nicsm->index = 0;
	nicsm->size = PAGE_SIZE;

	ecm_nss_ipv6_workqueue = create_singlethread_workqueue("ecm_nss_ipv6_workqueue");
	if (!ecm_nss_ipv6_workqueue) {
		nss_ipv6_conn_sync_many_notify_unregister();
		kfree(ecm_nss_ipv6_sync_req_msg);
		return false;
	}
	INIT_DELAYED_WORK(&ecm_nss_ipv6_work, ecm_nss_ipv6_stats_sync_req_work);
	queue_delayed_work(ecm_nss_ipv6_workqueue, &ecm_nss_ipv6_work, ECM_NSS_IPV6_STATS_SYNC_PERIOD);

	return true;
}

/*
 * ecm_nss_ipv6_sync_queue_exit
 *	 the workqueue for ipv6 stats sync
 */
static void ecm_nss_ipv6_sync_queue_exit(void)
{
	/*
	 * Unregister the conn_sync_many message callback
	 * Otherwise nss will call our callback which does not exist anymore
	 */
	nss_ipv6_conn_sync_many_notify_unregister();

	/*
	 * Cancel the conn sync req work and destroy workqueue
	 */
	cancel_delayed_work_sync(&ecm_nss_ipv6_work);
	destroy_workqueue(ecm_nss_ipv6_workqueue);
	kfree(ecm_nss_ipv6_sync_req_msg);
}

/*
 * ecm_nss_ipv6_init()
 */
int ecm_nss_ipv6_init(struct dentry *dentry)
{
	int result = -1;

	if (!ecm_front_end_is_feature_supported(ECM_FE_FEATURE_NSS)) {
		DEBUG_INFO("NSS IPv6 is disabled\n");
		return 0;
	}

	DEBUG_INFO("ECM NSS IPv6 init\n");

	ecm_nss_ipv6_dentry = debugfs_create_dir("ecm_nss_ipv6", dentry);
	if (!ecm_nss_ipv6_dentry) {
		DEBUG_ERROR("Failed to create ecm nss ipv6 directory in debugfs\n");
		return result;
	}

	if (!ecm_debugfs_create_u32("no_action_limit_default", S_IRUGO | S_IWUSR, ecm_nss_ipv6_dentry,
					(u32 *)&ecm_nss_ipv6_no_action_limit_default)) {
		DEBUG_ERROR("Failed to create ecm nss ipv6 no_action_limit_default file in debugfs\n");
		goto task_cleanup;
	}

	if (!ecm_debugfs_create_u32("driver_fail_limit_default", S_IRUGO | S_IWUSR, ecm_nss_ipv6_dentry,
					(u32 *)&ecm_nss_ipv6_driver_fail_limit_default)) {
		DEBUG_ERROR("Failed to create ecm nss ipv6 driver_fail_limit_default file in debugfs\n");
		goto task_cleanup;
	}

	if (!ecm_debugfs_create_u32("nack_limit_default", S_IRUGO | S_IWUSR, ecm_nss_ipv6_dentry,
					(u32 *)&ecm_nss_ipv6_nack_limit_default)) {
		DEBUG_ERROR("Failed to create ecm nss ipv6 nack_limit_default file in debugfs\n");
		goto task_cleanup;
	}

	if (!ecm_debugfs_create_u32("accelerated_count", S_IRUGO, ecm_nss_ipv6_dentry,
					(u32 *)&ecm_nss_ipv6_accelerated_count)) {
		DEBUG_ERROR("Failed to create ecm nss ipv6 accelerated_count file in debugfs\n");
		goto task_cleanup;
	}

	if (!ecm_debugfs_create_u32("pending_accel_count", S_IRUGO, ecm_nss_ipv6_dentry,
					(u32 *)&ecm_nss_ipv6_pending_accel_count)) {
		DEBUG_ERROR("Failed to create ecm nss ipv6 pending_accel_count file in debugfs\n");
		goto task_cleanup;
	}

	if (!ecm_debugfs_create_u32("pending_decel_count", S_IRUGO, ecm_nss_ipv6_dentry,
					(u32 *)&ecm_nss_ipv6_pending_decel_count)) {
		DEBUG_ERROR("Failed to create ecm nss ipv6 pending_decel_count file in debugfs\n");
		goto task_cleanup;
	}

	if (!debugfs_create_file("accel_limit_mode", S_IRUGO | S_IWUSR, ecm_nss_ipv6_dentry,
					NULL, &ecm_nss_ipv6_accel_limit_mode_fops)) {
		DEBUG_ERROR("Failed to create ecm nss ipv6 accel_limit_mode file in debugfs\n");
		goto task_cleanup;
	}

	if (!debugfs_create_file("accel_cmd_avg_millis", S_IRUGO, ecm_nss_ipv6_dentry,
					NULL, &ecm_nss_ipv6_accel_cmd_avg_millis_fops)) {
		DEBUG_ERROR("Failed to create ecm nss ipv6 accel_cmd_avg_millis file in debugfs\n");
		goto task_cleanup;
	}

	if (!debugfs_create_file("decel_cmd_avg_millis", S_IRUGO, ecm_nss_ipv6_dentry,
					NULL, &ecm_nss_ipv6_decel_cmd_avg_millis_fops)) {
		DEBUG_ERROR("Failed to create ecm nss ipv6 decel_cmd_avg_millis file in debugfs\n");
		goto task_cleanup;
	}

	if (!ecm_nss_ported_ipv6_debugfs_init(ecm_nss_ipv6_dentry)) {
		DEBUG_ERROR("Failed to create ecm ported files in debugfs\n");
		goto task_cleanup;
	}

	if (!debugfs_create_file("stats_request_counter", S_IRUGO, ecm_nss_ipv6_dentry,
					NULL, &ecm_nss_ipv6_stats_request_counter_fops)) {
		DEBUG_ERROR("Failed to create ecm nss ipv6 stats_request_counter file in debugfs\n");
		goto task_cleanup;
	}

	if (!ecm_debugfs_create_u32("vlan_passthrough_set", S_IRUGO | S_IWUSR, ecm_nss_ipv6_dentry,
					(u32 *)&ecm_nss_ipv6_vlan_passthrough_enable)) {
		DEBUG_ERROR("Failed to create ecm nss ipv6 vlan passthrough file in debugfs\n");
		goto task_cleanup;
	}

#ifdef ECM_NON_PORTED_SUPPORT_ENABLE
	if (!ecm_nss_non_ported_ipv6_debugfs_init(ecm_nss_ipv6_dentry)) {
		DEBUG_ERROR("Failed to create ecm non-ported files in debugfs\n");
		goto task_cleanup;
	}
#endif

#ifdef ECM_MULTICAST_ENABLE
	if (!ecm_nss_multicast_ipv6_debugfs_init(ecm_nss_ipv6_dentry)) {
		DEBUG_ERROR("Failed to create ecm multicast files in debugfs\n");
		goto task_cleanup;
	}
#endif
	/*
	 * Register this module with the Linux NSS Network driver.
	 * Notify manager should be registered before the netfilter hooks. Because there
	 * is a possibility that the ECM can try to send acceleration messages to the
	 * acceleration engine without having an acceleration engine manager.
	 */
	ecm_nss_ipv6_nss_ipv6_mgr = nss_ipv6_notify_register(ecm_nss_ipv6_net_dev_callback, NULL);

#ifdef ECM_MULTICAST_ENABLE
	result = ecm_nss_multicast_ipv6_init(ecm_nss_ipv6_dentry);
	if (result < 0) {
		DEBUG_ERROR("Failed to init ecm ipv6 multicast frontend\n");
		goto task_cleanup_1;
	}
#endif

	if (!ecm_nss_ipv6_sync_queue_init()) {
		DEBUG_ERROR("Failed to create ecm ipv6 connection sync workqueue\n");
		goto task_cleanup_2;
	}

	return 0;

task_cleanup_2:
#ifdef ECM_MULTICAST_ENABLE
	ecm_nss_multicast_ipv6_exit();
task_cleanup_1:
#endif
	nss_ipv6_notify_unregister();

task_cleanup:
	debugfs_remove_recursive(ecm_nss_ipv6_dentry);
	return result;
}

/*
 * ecm_nss_ipv6_exit()
 */
void ecm_nss_ipv6_exit(void)
{
	if (!ecm_front_end_is_feature_supported(ECM_FE_FEATURE_NSS)) {
		DEBUG_INFO("NSS IPv6 is disabled\n");
		return;
	}

	DEBUG_INFO("ECM NSS IPv6 Module exit\n");

	/*
	 * Unregister from the Linux NSS Network driver
	 */
	nss_ipv6_notify_unregister();

	/*
	 * Remove the debugfs files recursively.
	 */
	if (ecm_nss_ipv6_dentry) {
		debugfs_remove_recursive(ecm_nss_ipv6_dentry);
	}

#ifdef ECM_MULTICAST_ENABLE
	ecm_nss_multicast_ipv6_exit();
#endif

	/*
	 * Clean up the stats sync queue/work
	 */
	ecm_nss_ipv6_sync_queue_exit();
}
