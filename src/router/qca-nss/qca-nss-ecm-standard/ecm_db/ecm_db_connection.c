/*
 **************************************************************************
 * Copyright (c) 2014-2021, The Linux Foundation. All rights reserved.
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
#include <linux/random.h>
#include <net/route.h>
#include <net/ip.h>
#include <net/tcp.h>
#include <asm/unaligned.h>
#include <asm/uaccess.h>	/* for put_user */
#include <net/ipv6.h>
#include <net/ip6_route.h>
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
#define DEBUG_LEVEL ECM_DB_DEBUG_LEVEL

#include "ecm_types.h"
#include "ecm_db_types.h"
#include "ecm_state.h"
#include "ecm_tracker.h"
#include "ecm_classifier.h"
#include "ecm_front_end_types.h"
#include "ecm_classifier_default.h"
#include "ecm_db.h"
#include "ecm_front_end_common.h"

/*
 * Magic number
 */
#ifdef ECM_DB_CTA_TRACK_ENABLE
#define ECM_DB_CLASSIFIER_TYPE_ASSIGNMENT_MAGIC 0xAEF4
#endif

/*
 * Global list.
 * All instances are inserted into global list - this allows easy iteration of all instances of a particular type.
 * The list is doubly linked for fast removal.  The list is in no particular order.
 */
struct ecm_db_connection_instance *ecm_db_connections = NULL;

/*
 * Connection hash table
 */
#define ECM_DB_CONNECTION_HASH_SLOTS 32768
static struct ecm_db_connection_instance **ecm_db_connection_table;
						/* Slots of the connection hash table */
static int *ecm_db_connection_table_lengths;
						/* Tracks how long each chain is */
static int ecm_db_connection_count = 0;		/* Number of connections allocated */

/*
 * Connection serial number hash table
 */
#define ECM_DB_CONNECTION_SERIAL_HASH_SLOTS 32768
struct ecm_db_connection_instance **ecm_db_connection_serial_table;
						/* Slots of the connection serial hash table */
int *ecm_db_connection_serial_table_lengths;
						/* Tracks how long each chain is */

static int ecm_db_connection_serial = 0;	/* Serial number - ensures each connection has a unique serial number.
						 * Serial numbers are used mainly by classifiers that keep their own state
						 * and can 'link' their state to the right connection using a serial number.
						 * The serial number is also used as a soft linkage to other subsystems such as NA.
						 */

/*
 * Simple stats
 */
#define ECM_DB_PROTOCOL_COUNT 256
static int ecm_db_connection_count_by_protocol[ECM_DB_PROTOCOL_COUNT];	/* Each IP protocol has its own count */

/*
 * Connection state validity
 * This counter is incremented whenever a general change is detected which requires re-generation of state for ALL connections.
 */
uint16_t ecm_db_connection_generation = 0;		/* Generation counter to detect when all connection state is considered stale and all must be re-generated */

#ifdef ECM_DB_CTA_TRACK_ENABLE
/*
 * Classifier TYPE assignment lists.
 *
 * For each type of classifier a list is kept of all connections assigned a classifier of that type.
 * This permits a classifier type to rapidly retrieve all connections with classifiers assigned to it of that type.
 *
 * NOTE: This is in addition to the basic functionality whereby a connection keeps a list of classifier instances
 * that are assigned to it in descending order of priority.
 */

/*
 * struct ecm_db_connection_classifier_type_assignment_list
 *	A list, one for each classifier type.
 */
struct ecm_db_connection_classifier_type_assignment_list {
	struct ecm_db_connection_instance *type_assignments_list;
							/* Lists of connections assigned to this type of classifier */
	int32_t type_assignment_count;			/* Number of connections in the list */
} ecm_db_connection_classifier_type_assignments[ECM_CLASSIFIER_TYPES];
							/* Each classifier type has a list of connections that are assigned to classifier instances of that type */
#endif

/*
 * _ecm_db_connection_count_get()
 *	Return the connection count (lockless).
 */
int _ecm_db_connection_count_get(void)
{
	return ecm_db_connection_count;
}

/*
 * ecm_db_connection_count_get()
 *	Return the connection count
 */
int ecm_db_connection_count_get(void)
{
	int count;

	spin_lock_bh(&ecm_db_lock);
	count = ecm_db_connection_count;
	spin_unlock_bh(&ecm_db_lock);
	return count;
}
EXPORT_SYMBOL(ecm_db_connection_count_get);

/*
 * ecm_db_connection_count_by_protocol_get()
 *	Return # connections for the given protocol
 */
int ecm_db_connection_count_by_protocol_get(int protocol)
{
	int count;

	DEBUG_ASSERT((protocol >= 0) && (protocol < ECM_DB_PROTOCOL_COUNT), "Bad protocol: %d\n", protocol);
	spin_lock_bh(&ecm_db_lock);
	count = ecm_db_connection_count_by_protocol[protocol];
	spin_unlock_bh(&ecm_db_lock);
	return count;
}
EXPORT_SYMBOL(ecm_db_connection_count_by_protocol_get);

/*
 * ecm_db_connection_l2_encap_proto_set()
 *	Sets the L2 encap protocol.
 */
void ecm_db_connection_l2_encap_proto_set(struct ecm_db_connection_instance *ci, uint16_t l2_encap_proto)
{
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", ci);

	spin_lock_bh(&ecm_db_lock);
	ci->l2_encap_proto = l2_encap_proto;
	spin_unlock_bh(&ecm_db_lock);
}

/*
 * ecm_db_connection_l2_encap_proto_get()
 *	Gets the L2 encap protocol.
 */
uint16_t ecm_db_connection_l2_encap_proto_get(struct ecm_db_connection_instance *ci)
{
	uint16_t proto;
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", ci);

	spin_lock_bh(&ecm_db_lock);
	proto = ci->l2_encap_proto;
	spin_unlock_bh(&ecm_db_lock);

	return proto;
}

/*
 * ecm_db_connection_mark_set()
 *	Sets the mark value of the connection.
 */
void ecm_db_connection_mark_set(struct ecm_db_connection_instance *ci, uint32_t mark)
{
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", ci);

	spin_lock_bh(&ecm_db_lock);
	ci->mark = mark;
	spin_unlock_bh(&ecm_db_lock);

}

/*
 * ecm_db_connection_flag_set()
 *	Sets the flag in connection instance.
 */
void ecm_db_connection_flag_set(struct ecm_db_connection_instance *ci, uint32_t flag)
{
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", ci);

	spin_lock_bh(&ecm_db_lock);
	ci->flags |= flag;
	spin_unlock_bh(&ecm_db_lock);
}

/*
 * ecm_db_connection_mark_get()
 *	Gets the mark value of the connection.
 */
uint32_t ecm_db_connection_mark_get(struct ecm_db_connection_instance *ci)
{
	uint16_t mark;
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", ci);

	spin_lock_bh(&ecm_db_lock);
	mark = ci->mark;
	spin_unlock_bh(&ecm_db_lock);

	return mark;
}

/*
 * ecm_db_connection_front_end_get_and_ref()
 *	Return ref to the front end instance of the connection
 */
struct ecm_front_end_connection_instance *ecm_db_connection_front_end_get_and_ref(struct ecm_db_connection_instance *ci)
{
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", ci);
	ecm_front_end_connection_ref(ci->feci);
	return ci->feci;
}
EXPORT_SYMBOL(ecm_db_connection_front_end_get_and_ref);

/*
 * ecm_db_connection_defunct_callback()
 *	Invoked by the expiration of the defunct_timer contained in a connection instance
 */
static void ecm_db_connection_defunct_callback(void *arg)
{
	int accel_mode;

	struct ecm_db_connection_instance *ci = (struct ecm_db_connection_instance *)arg;
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", ci);

	DEBUG_INFO("%px: defunct timer expired\n", ci);

	/*
	 * call the front end defunct to destroy the rule.
	 */
	(void)ci->feci->defunct(ci->feci, &accel_mode);

	/*
	 * when it was expired, it has been removed from  the timer list,
	 * we need release the ref when it was added to the timer list.
	 */
	ecm_db_connection_deref(ci);
}

/*
 * ecm_db_connection_elapsed_defunct_timer()
 *	Returns the elapsed time of defunct timer.
 * If the timer is already expired and not removed from the database, the
 * function returns a negative value. The caller MUST handle this return value.
 */
int ecm_db_connection_elapsed_defunct_timer(struct ecm_db_connection_instance *ci)
{
	long int expires_in;
	int elapsed;

	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", ci);

	/*
	 * Do some sanity checks.
	 * If it is not in a timer group, which means already expired, or the
	 * connection has not been fully created yet. Just return 0.
	 */
	spin_lock_bh(&ecm_db_lock);
	if (ci->defunct_timer.group == ECM_DB_TIMER_GROUPS_MAX) {
		spin_unlock_bh(&ecm_db_lock);
		return -1;
	}

	/*
	 * Already expired, but not removed from the database completely.
	 */
	expires_in = (long int)(ci->defunct_timer.timeout - ecm_db_time);
	if (expires_in < 0) {
		spin_unlock_bh(&ecm_db_lock);
		return -1;
	}

	elapsed = ecm_db_timer_groups[ci->defunct_timer.group].time - expires_in;
	spin_unlock_bh(&ecm_db_lock);

	return elapsed;
}
EXPORT_SYMBOL(ecm_db_connection_elapsed_defunct_timer);

/*
 * ecm_db_connection_defunct_timer_reset()
 *	Set/change the timer group associated with a connection.  Returns false if the connection has become defunct and the new group cannot be set for that reason.
 */
bool ecm_db_connection_defunct_timer_reset(struct ecm_db_connection_instance *ci, ecm_db_timer_group_t tg)
{
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", ci);
	return ecm_db_timer_group_entry_reset(&ci->defunct_timer, tg);
}
EXPORT_SYMBOL(ecm_db_connection_defunct_timer_reset);

/*
 * ecm_db_connection_defunct_timer_touch()
 *	Update the connections defunct timer to stop it timing out.  Returns false if the connection defunct timer has expired.
 */
bool ecm_db_connection_defunct_timer_touch(struct ecm_db_connection_instance *ci)
{
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", ci);
	return ecm_db_timer_group_entry_touch(&ci->defunct_timer);
}
EXPORT_SYMBOL(ecm_db_connection_defunct_timer_touch);

/*
 * ecm_db_connection_defunct_timer_no_touch_set()
 *	Set no touch flag in CI
 */
void ecm_db_connection_defunct_timer_no_touch_set(struct ecm_db_connection_instance *ci)
{
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%p: magic failed", ci);
	ci->timer_no_touch = true;
}

/*
 * ecm_db_connection_defunct_timer_no_touch_get()
 *	Get no touch flag in CI
 */
bool ecm_db_connection_defunct_timer_no_touch_get(struct ecm_db_connection_instance *ci)
{
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%p: magic failed", ci);
	return ci->timer_no_touch;
}

/*
 * ecm_db_connection_timer_group_get()
 *	Return the timer group id
 */
ecm_db_timer_group_t ecm_db_connection_timer_group_get(struct ecm_db_connection_instance *ci)
{
	ecm_db_timer_group_t tg;
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", ci);

	spin_lock_bh(&ecm_db_lock);
	tg = ci->defunct_timer.group;
	spin_unlock_bh(&ecm_db_lock);
	return tg;
}
EXPORT_SYMBOL(ecm_db_connection_timer_group_get);

/*
 * ecm_db_connection_make_defunct()
 *	Make connection defunct.
 */
void ecm_db_connection_make_defunct(struct ecm_db_connection_instance *ci)
{
	int accel_mode;
	bool ret;

	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", ci);

	/*
	 * Call the frontend's defunct callback function and handle the return values.
	 */
	ret = ci->feci->defunct(ci->feci, &accel_mode);

	/*
	 * If connection's defunct timer is already removed from the groups,
	 * this means that the connection is timed out and already in defunct process.
	 * So, we needn't destroy the connection any more.
	 */
	spin_lock_bh(&ecm_db_lock);
	if (ci->defunct_timer.group == ECM_DB_TIMER_GROUPS_MAX) {
		spin_unlock_bh(&ecm_db_lock);
		return;
	}

	/*
	 * If the defunct is success, first we should remove the timer and then release
	 * the last reference.
	 * If defunct fails and the connection state is in one of the fail states, we should remove the timer
	 * and release the last reference.
	 * the removal of timer will always be successful here, we needn't check the
	 * return value.
	 */
	if (ret || ECM_FRONT_END_ACCELERATION_FAILED(accel_mode)) {
		_ecm_db_timer_group_entry_remove(&ci->defunct_timer);
		spin_unlock_bh(&ecm_db_lock);
		ecm_db_connection_deref(ci);
		return;
	}

	spin_unlock_bh(&ecm_db_lock);
}
EXPORT_SYMBOL(ecm_db_connection_make_defunct);

/*
 * ecm_db_connection_data_totals_update()
 *	Update the total data (and packets) sent/received by the given host
 */
void ecm_db_connection_data_totals_update(struct ecm_db_connection_instance *ci, bool is_from, uint64_t size, uint64_t packets)
{
#ifdef ECM_DB_ADVANCED_STATS_ENABLE
	int32_t i;
#endif

	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed\n", ci);

	spin_lock_bh(&ecm_db_lock);

	if (is_from) {
		/*
		 * Update totals sent by the FROM side of connection
		 */
		ci->from_data_total += size;
		ci->from_packet_total += packets;
#ifdef ECM_DB_ADVANCED_STATS_ENABLE
		ci->mapping[ECM_DB_OBJ_DIR_FROM]->from_data_total += size;
		ci->mapping[ECM_DB_OBJ_DIR_FROM]->host->from_data_total += size;
		ci->node[ECM_DB_OBJ_DIR_FROM]->from_data_total += size;
		ci->mapping[ECM_DB_OBJ_DIR_FROM]->from_packet_total += packets;
		ci->mapping[ECM_DB_OBJ_DIR_FROM]->host->from_packet_total += packets;
		ci->node[ECM_DB_OBJ_DIR_FROM]->from_packet_total += packets;

		/*
		 * Data from the host is essentially TO the interface on which the host is reachable
		 */
		for (i = ci->interface_first[ECM_DB_OBJ_DIR_FROM]; i < ECM_DB_IFACE_HEIRARCHY_MAX; ++i) {
			ci->interfaces[ECM_DB_OBJ_DIR_FROM][i]->to_data_total += size;
			ci->interfaces[ECM_DB_OBJ_DIR_FROM][i]->to_packet_total += packets;
		}

		/*
		 * Update totals sent TO the other side of the connection
		 */
		ci->mapping[ECM_DB_OBJ_DIR_TO]->to_data_total += size;
		ci->mapping[ECM_DB_OBJ_DIR_TO]->host->to_data_total += size;
		ci->node[ECM_DB_OBJ_DIR_TO]->to_data_total += size;
		ci->mapping[ECM_DB_OBJ_DIR_TO]->to_packet_total += packets;
		ci->mapping[ECM_DB_OBJ_DIR_TO]->host->to_packet_total += packets;
		ci->node[ECM_DB_OBJ_DIR_TO]->to_packet_total += packets;

		/*
		 * Sending to the other side means FROM the interface we reach that host
		 */
		for (i = ci->interface_first[ECM_DB_OBJ_DIR_TO]; i < ECM_DB_IFACE_HEIRARCHY_MAX; ++i) {
			ci->interfaces[ECM_DB_OBJ_DIR_TO][i]->from_data_total += size;
			ci->interfaces[ECM_DB_OBJ_DIR_TO][i]->from_packet_total += packets;
		}
#endif
		spin_unlock_bh(&ecm_db_lock);
		return;
	}

	/*
	 * Update totals sent by the TO side of this connection
	 */
	ci->to_data_total += size;
	ci->to_packet_total += packets;
#ifdef ECM_DB_ADVANCED_STATS_ENABLE
	ci->mapping[ECM_DB_OBJ_DIR_TO]->from_data_total += size;
	ci->mapping[ECM_DB_OBJ_DIR_TO]->host->from_data_total += size;
	ci->node[ECM_DB_OBJ_DIR_TO]->from_data_total += size;
	ci->mapping[ECM_DB_OBJ_DIR_TO]->from_packet_total += packets;
	ci->mapping[ECM_DB_OBJ_DIR_TO]->host->from_packet_total += packets;
	ci->node[ECM_DB_OBJ_DIR_TO]->from_packet_total += packets;

	/*
	 * Data from the host is essentially TO the interface on which the host is reachable
	 */
	for (i = ci->interface_first[ECM_DB_OBJ_DIR_TO]; i < ECM_DB_IFACE_HEIRARCHY_MAX; ++i) {
		ci->interfaces[ECM_DB_OBJ_DIR_TO][i]->to_data_total += size;
		ci->interfaces[ECM_DB_OBJ_DIR_TO][i]->to_packet_total += packets;
	}

	/*
	 * Update totals sent TO the other side of the connection
	 */
	ci->mapping[ECM_DB_OBJ_DIR_FROM]->to_data_total += size;
	ci->mapping[ECM_DB_OBJ_DIR_FROM]->host->to_data_total += size;
	ci->node[ECM_DB_OBJ_DIR_FROM]->to_data_total += size;
	ci->mapping[ECM_DB_OBJ_DIR_FROM]->to_packet_total += packets;
	ci->mapping[ECM_DB_OBJ_DIR_FROM]->host->to_packet_total += packets;
	ci->node[ECM_DB_OBJ_DIR_FROM]->to_packet_total += packets;

	/*
	 * Sending to the other side means FROM the interface we reach that host
	 */
	for (i = ci->interface_first[ECM_DB_OBJ_DIR_FROM]; i < ECM_DB_IFACE_HEIRARCHY_MAX; ++i) {
		ci->interfaces[ECM_DB_OBJ_DIR_FROM][i]->from_data_total += size;
		ci->interfaces[ECM_DB_OBJ_DIR_FROM][i]->from_packet_total += packets;
	}
#endif
	spin_unlock_bh(&ecm_db_lock);
}
EXPORT_SYMBOL(ecm_db_connection_data_totals_update);

/*
 * ecm_db_connection_data_totals_update_dropped()
 *	Update the total data (and packets) sent by the given host but which we dropped
 */
void ecm_db_connection_data_totals_update_dropped(struct ecm_db_connection_instance *ci, bool is_from, uint64_t size, uint64_t packets)
{
#ifdef ECM_DB_ADVANCED_STATS_ENABLE
	int32_t i;
#endif

	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed\n", ci);

	if (is_from) {
		/*
		 * Update dropped totals sent by the FROM side
		 */
		spin_lock_bh(&ecm_db_lock);
		ci->from_data_total_dropped += size;
		ci->from_packet_total_dropped += packets;
#ifdef ECM_DB_ADVANCED_STATS_ENABLE
		ci->mapping[ECM_DB_OBJ_DIR_FROM]->from_data_total_dropped += size;
		ci->mapping[ECM_DB_OBJ_DIR_FROM]->host->from_data_total_dropped += size;
		ci->node[ECM_DB_OBJ_DIR_FROM]->from_data_total_dropped += size;
		ci->mapping[ECM_DB_OBJ_DIR_FROM]->from_packet_total_dropped += packets;
		ci->mapping[ECM_DB_OBJ_DIR_FROM]->host->from_packet_total_dropped += packets;
		ci->node[ECM_DB_OBJ_DIR_FROM]->from_packet_total_dropped += packets;

		/*
		 * Data from the host is essentially TO the interface on which the host is reachable
		 */
		for (i = ci->interface_first[ECM_DB_OBJ_DIR_FROM]; i < ECM_DB_IFACE_HEIRARCHY_MAX; ++i) {
			ci->interfaces[ECM_DB_OBJ_DIR_FROM][i]->to_data_total_dropped += size;
			ci->interfaces[ECM_DB_OBJ_DIR_FROM][i]->to_packet_total_dropped += packets;
		}
#endif
		spin_unlock_bh(&ecm_db_lock);
		return;
	}

	/*
	 * Update dropped totals sent by the TO side of this connection
	 */
	spin_lock_bh(&ecm_db_lock);
	ci->to_data_total_dropped += size;
	ci->to_packet_total_dropped += packets;
#ifdef ECM_DB_ADVANCED_STATS_ENABLE
	ci->mapping[ECM_DB_OBJ_DIR_TO]->from_data_total_dropped += size;
	ci->mapping[ECM_DB_OBJ_DIR_TO]->host->from_data_total_dropped += size;
	ci->node[ECM_DB_OBJ_DIR_TO]->from_data_total_dropped += size;
	ci->mapping[ECM_DB_OBJ_DIR_TO]->from_packet_total_dropped += packets;
	ci->mapping[ECM_DB_OBJ_DIR_TO]->host->from_packet_total_dropped += packets;
	ci->node[ECM_DB_OBJ_DIR_TO]->from_packet_total_dropped += packets;

	/*
	 * Data from the host is essentially TO the interface on which the host is reachable
	 */
	for (i = ci->interface_first[ECM_DB_OBJ_DIR_TO]; i < ECM_DB_IFACE_HEIRARCHY_MAX; ++i) {
		ci->interfaces[ECM_DB_OBJ_DIR_TO][i]->to_data_total_dropped += size;
		ci->interfaces[ECM_DB_OBJ_DIR_TO][i]->to_packet_total_dropped += packets;
	}
#endif
	spin_unlock_bh(&ecm_db_lock);
}
EXPORT_SYMBOL(ecm_db_connection_data_totals_update_dropped);

/*
 * ecm_db_connection_data_stats_get()
 *	Return data stats for the instance
 */
void ecm_db_connection_data_stats_get(struct ecm_db_connection_instance *ci, uint64_t *from_data_total, uint64_t *to_data_total,
						uint64_t *from_packet_total, uint64_t *to_packet_total,
						uint64_t *from_data_total_dropped, uint64_t *to_data_total_dropped,
						uint64_t *from_packet_total_dropped, uint64_t *to_packet_total_dropped)
{
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", ci);

	spin_lock_bh(&ecm_db_lock);
	if (from_data_total) {
		*from_data_total = ci->from_data_total;
	}
	if (to_data_total) {
		*to_data_total = ci->to_data_total;
	}
	if (from_packet_total) {
		*from_packet_total = ci->from_packet_total;
	}
	if (to_packet_total) {
		*to_packet_total = ci->to_packet_total;
	}
	if (from_data_total_dropped) {
		*from_data_total_dropped = ci->from_data_total_dropped;
	}
	if (to_data_total_dropped) {
		*to_data_total_dropped = ci->to_data_total_dropped;
	}
	if (from_packet_total_dropped) {
		*from_packet_total_dropped = ci->from_packet_total_dropped;
	}
	if (to_packet_total_dropped) {
		*to_packet_total_dropped = ci->to_packet_total_dropped;
	}
	spin_unlock_bh(&ecm_db_lock);
}
EXPORT_SYMBOL(ecm_db_connection_data_stats_get);

/*
 * ecm_db_connection_serial_get()
 *	Return serial
 */
uint32_t ecm_db_connection_serial_get(struct ecm_db_connection_instance *ci)
{
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", ci);
	return ci->serial;
}
EXPORT_SYMBOL(ecm_db_connection_serial_get);

/*
 * ecm_db_connection_address_get()
 *	Return ip address address
 */
void ecm_db_connection_address_get(struct ecm_db_connection_instance *ci, ecm_db_obj_dir_t dir, ip_addr_t addr)
{
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", ci);
	DEBUG_CHECK_MAGIC(ci->mapping[dir], ECM_DB_MAPPING_INSTANCE_MAGIC, "%px: magic failed", ci->mapping[dir]);
	DEBUG_CHECK_MAGIC(ci->mapping[dir]->host, ECM_DB_HOST_INSTANCE_MAGIC, "%px: magic failed", ci->mapping[dir]->host);
	ECM_IP_ADDR_COPY(addr, ci->mapping[dir]->host->address);
}
EXPORT_SYMBOL(ecm_db_connection_address_get);

/*
 * ecm_db_connection_port_get()
 *	Return port
 */
int ecm_db_connection_port_get(struct ecm_db_connection_instance *ci, ecm_db_obj_dir_t dir)
{
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", ci);
	DEBUG_CHECK_MAGIC(ci->mapping[dir], ECM_DB_MAPPING_INSTANCE_MAGIC, "%px: magic failed", ci->mapping[dir]);
	return ci->mapping[dir]->port;
}
EXPORT_SYMBOL(ecm_db_connection_port_get);

/*
 * ecm_db_connection_node_address_get()
 *	Return address of the node used when sending packets to the specified side.
 */
void ecm_db_connection_node_address_get(struct ecm_db_connection_instance *ci, ecm_db_obj_dir_t dir, uint8_t *address_buffer)
{
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", ci);
	memcpy(address_buffer, ci->node[dir]->address, ETH_ALEN);
}
EXPORT_SYMBOL(ecm_db_connection_node_address_get);

/*
 * ecm_db_connection_iface_name_get()
 *	Return name of interface on which the specified side may be reached
 */
void ecm_db_connection_iface_name_get(struct ecm_db_connection_instance *ci, ecm_db_obj_dir_t dir, char *name_buffer)
{
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", ci);
	strlcpy(name_buffer, ci->node[dir]->iface->name, IFNAMSIZ);
}
EXPORT_SYMBOL(ecm_db_connection_iface_name_get);

/*
 * ecm_db_connection_iface_mtu_get()
 *	Return MTU of interface on which the specified side may be reached
 */
int ecm_db_connection_iface_mtu_get(struct ecm_db_connection_instance *ci, ecm_db_obj_dir_t dir)
{
	int mtu;
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", ci);
	spin_lock_bh(&ecm_db_lock);
	mtu = ci->node[dir]->iface->mtu;
	spin_unlock_bh(&ecm_db_lock);
	return mtu;
}
EXPORT_SYMBOL(ecm_db_connection_iface_mtu_get);

/*
 * ecm_db_connection_iface_type_get()
 *	Return type of interface on which the specified side may be reached
 */
ecm_db_iface_type_t ecm_db_connection_iface_type_get(struct ecm_db_connection_instance *ci, ecm_db_obj_dir_t dir)
{
	ecm_db_iface_type_t type;

	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", ci);
	spin_lock_bh(&ecm_db_lock);
	type = ci->node[dir]->iface->type;
	spin_unlock_bh(&ecm_db_lock);
	return type;
}
EXPORT_SYMBOL(ecm_db_connection_iface_type_get);

/*
 * ecm_db_connection_regeneration_occurrances_get()
 *	Get the number of regeneration occurrances that have occurred since the connection was created.
 */
uint16_t ecm_db_connection_regeneration_occurrances_get(struct ecm_db_connection_instance *ci)
{
	uint16_t occurances;
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", ci);

	spin_lock_bh(&ecm_db_lock);
	occurances = ci->regen_occurances;
	spin_unlock_bh(&ecm_db_lock);
	return occurances;
}
EXPORT_SYMBOL(ecm_db_connection_regeneration_occurrances_get);

/*
 * ecm_db_connection_regeneration_completed()
 *	Re-generation was completed successfully
 */
void ecm_db_connection_regeneration_completed(struct ecm_db_connection_instance *ci)
{
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", ci);

	spin_lock_bh(&ecm_db_lock);

	DEBUG_ASSERT(ci->regen_in_progress, "%px: Bad call", ci);
	DEBUG_ASSERT(ci->regen_required > 0, "%px: Bad call", ci);

	/*
	 * Decrement the required counter by 1.
	 * This may mean that regeneration is still required due to another change occuring _during_ re-generation.
	 */
	ci->regen_required--;
	ci->regen_in_progress = false;
	ci->regen_success++;
	spin_unlock_bh(&ecm_db_lock);
}
EXPORT_SYMBOL(ecm_db_connection_regeneration_completed);

/*
 * ecm_db_connection_regeneration_failed()
 *	Re-generation failed
 */
void ecm_db_connection_regeneration_failed(struct ecm_db_connection_instance *ci)
{
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", ci);

	spin_lock_bh(&ecm_db_lock);

	DEBUG_ASSERT(ci->regen_in_progress, "%px: Bad call", ci);
	DEBUG_ASSERT(ci->regen_required > 0, "%px: Bad call", ci);

	/*
	 * Re-generation is no longer in progress BUT we leave the regen
	 * counter as it is so as to indicate re-generation is still needed
	 */
	ci->regen_in_progress = false;
	ci->regen_fail++;
	spin_unlock_bh(&ecm_db_lock);
}
EXPORT_SYMBOL(ecm_db_connection_regeneration_failed);

/*
 * ecm_db_connection_regeneration_required_check()
 *	Returns true if the connection needs to be re-generated.
 *
 * If re-generation is needed this will mark the connection to indicate that re-generation is needed AND in progress.
 * If the return code is TRUE the caller MUST handle the re-generation.
 * Upon re-generation completion you must call ecm_db_connection_regeneration_completed() or ecm_db_connection_regeneration_failed().
 */
bool ecm_db_connection_regeneration_required_check(struct ecm_db_connection_instance *ci)
{
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", ci);

	/*
	 * Check the global generation counter for changes
	 */
	spin_lock_bh(&ecm_db_lock);
	if (ci->generation != ecm_db_connection_generation) {
		/*
		 * Re-generation is needed
		 */
		ci->regen_occurances++;
		ci->regen_required++;

		/*
		 * Record that we have seen this change
		 */
		ci->generation = ecm_db_connection_generation;
	}

	/*
	 * If re-generation is in progress then something is handling re-generation already
	 * so we tell the caller that it cannot handle re-generation.
	 */
	if (ci->regen_in_progress) {
		spin_unlock_bh(&ecm_db_lock);
		return false;
	}

	/*
	 * Is re-generation required?
	 */
	if (ci->regen_required == 0) {
		spin_unlock_bh(&ecm_db_lock);
		return false;
	}

	/*
	 * Flag that re-generation is in progress and tell the caller to handle re-generation
	 */
	ci->regen_in_progress = true;
	spin_unlock_bh(&ecm_db_lock);
	return true;
}
EXPORT_SYMBOL(ecm_db_connection_regeneration_required_check);

/*
 * ecm_db_connection_regeneration_required_peek()
 *	Returns true if the connection needs to be regenerated.
 *
 * NOTE: The caller MUST NOT handle re-generation, the caller may use this indication
 * to determine the sanity of the connection state and whether acceleration is permitted.
 */
bool ecm_db_connection_regeneration_required_peek(struct ecm_db_connection_instance *ci)
{
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", ci);

	spin_lock_bh(&ecm_db_lock);

	/*
	 * Check the global generation counter for changes (record any change now)
	 */
	if (ci->generation != ecm_db_connection_generation) {
		/*
		 * Re-generation is needed, flag the connection as needing re-generation now.
		 */
		ci->regen_occurances++;
		ci->regen_required++;

		/*
		 * Record that we have seen this change
		 */
		ci->generation = ecm_db_connection_generation;
	}
	if (ci->regen_required == 0) {
		spin_unlock_bh(&ecm_db_lock);
		return false;
	}
	spin_unlock_bh(&ecm_db_lock);
	return true;
}
EXPORT_SYMBOL(ecm_db_connection_regeneration_required_peek);

/*
 * ecm_db_connection_regeneration_needed()
 *	Cause a specific connection to require re-generation
 *
 * NOTE: This only flags that re-generation is needed.
 * The connection will typically be re-generated when ecm_db_connection_regeneration_required_check() is invoked.
 */
void ecm_db_connection_regeneration_needed(struct ecm_db_connection_instance *ci)
{
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", ci);

	spin_lock_bh(&ecm_db_lock);
	ci->regen_occurances++;
	ci->regen_required++;
	spin_unlock_bh(&ecm_db_lock);
}
EXPORT_SYMBOL(ecm_db_connection_regeneration_needed);

/*
 * ecm_db_regeneration_needed()
 *	Bump the global generation index to cause a re-generation of all connections state.
 */
void ecm_db_regeneration_needed(void)
{
	spin_lock_bh(&ecm_db_lock);
	ecm_db_connection_generation++;
	spin_unlock_bh(&ecm_db_lock);
}
EXPORT_SYMBOL(ecm_db_regeneration_needed);

/*
 * ecm_db_connection_regenerate()
 *	Re-generate a specific connection
 */
void ecm_db_connection_regenerate(struct ecm_db_connection_instance *ci)
{
	struct ecm_front_end_connection_instance *feci;

	DEBUG_TRACE("Regenerate connection: %px\n", ci);

	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", ci);

	/*
	 * Notify front end to regenerate a connection.
	 */
	feci = ecm_db_connection_front_end_get_and_ref(ci);
	feci->regenerate(feci, ci);
	ecm_front_end_connection_deref(feci);
}
EXPORT_SYMBOL(ecm_db_connection_regenerate);

/*
 * ecm_db_connection_direction_get()
 *	Return direction of the connection.
 *
 * NOTE: an EGRESS connection means that packets being sent to mapping_to should have qos applied.
 * INGRESS means that packets being sent to mapping[ECM_DB_OBJ_DIR_FROM] should have qos applied.
 */
ecm_db_direction_t ecm_db_connection_direction_get(struct ecm_db_connection_instance *ci)
{
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", ci);
	return ci->direction;
}
EXPORT_SYMBOL(ecm_db_connection_direction_get);

/*
 * ecm_db_connection_is_routed_get()
 *	Return whether connection is a routed path or not
 */
bool ecm_db_connection_is_routed_get(struct ecm_db_connection_instance *ci)
{
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", ci);
	return ci->is_routed;
}
EXPORT_SYMBOL(ecm_db_connection_is_routed_get);

/*
 * ecm_db_connection_protocol_get()
 *	Return protocol of connection
 */
int ecm_db_connection_protocol_get(struct ecm_db_connection_instance *ci)
{
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", ci);
	return ci->protocol;
}
EXPORT_SYMBOL(ecm_db_connection_protocol_get);

/*
 * ecm_db_connection_ip_version_get()
 *	Return IP version of connection
 */
int ecm_db_connection_ip_version_get(struct ecm_db_connection_instance *ci)
{
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", ci);
	return ci->ip_version;
}
EXPORT_SYMBOL(ecm_db_connection_ip_version_get);

/*
 * ecm_db_connection_is_pppoe_bridged_get()
 *	Return whether connection is pppoe bridged or not
 */
bool ecm_db_connection_is_pppoe_bridged_get(struct ecm_db_connection_instance *ci)
{
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", ci);
	return ci->flags & ECM_DB_CONNECTION_FLAGS_PPPOE_BRIDGE;
}

/*
 * ecm_db_connection_defunct_timer_remove_and_set()
 *	Move the connection to a new timer group.
 *
 * Before setting the new group, check if the timer group is set. If it is set,
 * remove it first from the current group.
 *
 */
void ecm_db_connection_defunct_timer_remove_and_set(struct ecm_db_connection_instance *ci, ecm_db_timer_group_t tg)
{
	struct ecm_db_timer_group_entry *tge;

	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", ci);
	DEBUG_TRACE("%px: ecm_db_connection_defunct_timer_remove_and_set\n", ci);

	spin_lock_bh(&ecm_db_lock);
	tge = &ci->defunct_timer;
	if (tge->group == tg) {
		spin_unlock_bh(&ecm_db_lock);
		DEBUG_TRACE("%px: timer group is aslready equal to %d\n", ci, tg);
		return;
	}

	if (tge->group != ECM_DB_TIMER_GROUPS_MAX) {
		_ecm_db_timer_group_entry_remove(tge);
	} else {
		/*
		 * if group is ECM_DB_TIMER_GROUPS_MAX, its ref to ci was decreased
		 * need ref it again, otherwise, the ci could be free before the
		 * timer was expired
		 */
		_ecm_db_connection_ref(ci);
	}

	/*
	 * Set new group
	 */
	_ecm_db_timer_group_entry_set(tge, tg);
	spin_unlock_bh(&ecm_db_lock);
	DEBUG_TRACE("%px: New timer group is: %d\n", ci, tge->group);
}
EXPORT_SYMBOL(ecm_db_connection_defunct_timer_remove_and_set);

/*
 * _ecm_db_connection_ref()
 */
void _ecm_db_connection_ref(struct ecm_db_connection_instance *ci)
{
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", ci);
	ci->refs++;
	DEBUG_TRACE("%px: connection ref %d\n", ci, ci->refs);
	DEBUG_ASSERT(ci->refs > 0, "%px: ref wrap\n", ci);
}

/*
 * ecm_db_connection_ref()
 */
void ecm_db_connection_ref(struct ecm_db_connection_instance *ci)
{
	spin_lock_bh(&ecm_db_lock);
	_ecm_db_connection_ref(ci);
	spin_unlock_bh(&ecm_db_lock);
}
EXPORT_SYMBOL(ecm_db_connection_ref);

/*
 * ecm_db_connections_get_and_ref_first()
 *	Obtain a ref to the first connection instance, if any
 */
struct ecm_db_connection_instance *ecm_db_connections_get_and_ref_first(void)
{
	struct ecm_db_connection_instance *ci;
	spin_lock_bh(&ecm_db_lock);
	ci = ecm_db_connections;
	if (ci) {
		_ecm_db_connection_ref(ci);
	}
	spin_unlock_bh(&ecm_db_lock);
	return ci;
}
EXPORT_SYMBOL(ecm_db_connections_get_and_ref_first);

/*
 * ecm_db_connection_get_and_ref_next()
 *	Return the next connection in the list given a connection
 */
struct ecm_db_connection_instance *ecm_db_connection_get_and_ref_next(struct ecm_db_connection_instance *ci)
{
	struct ecm_db_connection_instance *cin;
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", ci);
	spin_lock_bh(&ecm_db_lock);
	cin = ci->next;
	if (cin) {
		_ecm_db_connection_ref(cin);
	}
	spin_unlock_bh(&ecm_db_lock);
	return cin;
}
EXPORT_SYMBOL(ecm_db_connection_get_and_ref_next);

#ifdef ECM_DB_CTA_TRACK_ENABLE
/*
 * _ecm_db_classifier_type_assignment_remove()
 *	Remove the connection from the classifier type assignment list (of the given type)
 */
static void _ecm_db_classifier_type_assignment_remove(struct ecm_db_connection_instance *ci, ecm_classifier_type_t ca_type)
{
	struct ecm_db_connection_classifier_type_assignment *ta;
	struct ecm_db_connection_classifier_type_assignment_list *tal;

	DEBUG_ASSERT(spin_is_locked(&ecm_db_lock), "%px: lock is not held\n", ci);

	DEBUG_TRACE("%px: Classifier type assignment remove: %d\n", ci, ca_type);
	ta = &ci->type_assignment[ca_type];
	DEBUG_CHECK_MAGIC(ta, ECM_DB_CLASSIFIER_TYPE_ASSIGNMENT_MAGIC, "%px: magic failed, ci: %px\n", ta, ci);
	DEBUG_ASSERT(ta->iteration_count == 0, "%px: iteration count: %d, type: %d\n", ci, ta->iteration_count, ca_type);

	if (ta->next) {
		struct ecm_db_connection_classifier_type_assignment *tan = &ta->next->type_assignment[ca_type];
		DEBUG_ASSERT(tan->prev == ci, "Bad list, expecting: %px, got: %px\n", ci, tan->prev);
		tan->prev = ta->prev;
	}

	tal = &ecm_db_connection_classifier_type_assignments[ca_type];
	if (ta->prev) {
		struct ecm_db_connection_classifier_type_assignment *tap = &ta->prev->type_assignment[ca_type];
		DEBUG_ASSERT(tap->next == ci, "Bad list, expecting: %px, got: %px\n", ci, tap->next);
		tap->next = ta->next;
	} else {
		/*
		 * Set new head of list
		 */
		DEBUG_ASSERT(tal->type_assignments_list == ci, "Bad head, expecting %px, got %px, type: %d\n", ci, tal->type_assignments_list, ca_type);
		tal->type_assignments_list = ta->next;
	}
	ta->next = NULL;
	ta->prev = NULL;
	ta->pending_unassign = false;

	/*
	 * Decrement assignment count
	 */
	tal->type_assignment_count--;
	DEBUG_ASSERT(tal->type_assignment_count >= 0, "Bad type assignment count: %d, type: %d\n", tal->type_assignment_count, ca_type);

	DEBUG_CLEAR_MAGIC(ta);
}
#endif

/*
 * _ecm_db_connection_classifier_unassign()
 *	Unassign a classifier and remove the classifier type
 *
 * The default classifier cannot be unassigned.
 */
static inline void _ecm_db_connection_classifier_unassign(struct ecm_db_connection_instance *ci, struct ecm_classifier_instance *cci, ecm_classifier_type_t ca_type)
{
#ifdef ECM_DB_CTA_TRACK_ENABLE
	struct ecm_db_connection_classifier_type_assignment *ta;
#endif
	DEBUG_ASSERT(spin_is_locked(&ecm_db_lock), "%px: lock is not held\n", ci);

	/*
	 * Clear the assignment.
	 */
	ci->assignments_by_type[ca_type] = NULL;

	/*
	 * Link out of assignments list
	 */
	if (cci->ca_prev) {
		cci->ca_prev->ca_next = cci->ca_next;
	} else {
		DEBUG_ASSERT(ci->assignments == cci, "%px: Bad assigmnment list, expecting: %px, got: %px", ci, cci, ci->assignments);
		ci->assignments = cci->ca_next;
	}
	if (cci->ca_next) {
		cci->ca_next->ca_prev = cci->ca_prev;
	}
	cci->ca_next = NULL;
	cci->ca_prev = NULL;

#ifdef ECM_DB_CTA_TRACK_ENABLE
	/*
	 * Remove from the classifier type assignment list
	 */
	ta = &ci->type_assignment[ca_type];
	DEBUG_CHECK_MAGIC(ta, ECM_DB_CLASSIFIER_TYPE_ASSIGNMENT_MAGIC, "%px: magic failed, ci: %px", ta, ci);
	if (ta->iteration_count > 0) {
		/*
		 * The list entry is being iterated outside of db lock being held.
		 * We cannot remove this entry since it would mess up iteration.
		 * Set the pending flag to be actioned another time
		 */
		ta->pending_unassign = true;
		return;
	}

	/*
	 * Remove the list entry
	 */
	DEBUG_INFO("%px: Remove type assignment: %d\n", ci, ca_type);
	_ecm_db_classifier_type_assignment_remove(ci, ca_type);
#endif
	cci->deref(cci);
}

/*
 * ecm_db_connection_deref()
 *	Release reference to connection.  Connection is removed from database on final deref and destroyed.
 */
int ecm_db_connection_deref(struct ecm_db_connection_instance *ci)
{
#ifdef ECM_DB_CTA_TRACK_ENABLE
	ecm_classifier_type_t ca_type;
#endif
	int32_t i;
	int32_t dir;

	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", ci);

	spin_lock_bh(&ecm_db_lock);
	ci->refs--;
	DEBUG_TRACE("%px: connection deref %d\n", ci, ci->refs);
	DEBUG_ASSERT(ci->refs >= 0, "%px: ref wrap\n", ci);

	if (ci->refs > 0) {
		int refs = ci->refs;
		spin_unlock_bh(&ecm_db_lock);
		return refs;
	}

#ifdef ECM_MULTICAST_ENABLE
	/*
	 * For multicast connections, we need to deref the
	 * associated tuple instance as well
	 */
	if (ci->ti) {
		_ecm_db_multicast_tuple_instance_deref(ci->ti);
	}
#endif
	/*
	 * Remove from database if inserted
	 */
	if (!ci->flags & ECM_DB_CONNECTION_FLAGS_INSERTED) {
		spin_unlock_bh(&ecm_db_lock);
	} else {
		struct ecm_db_listener_instance *li;
#ifdef ECM_DB_XREF_ENABLE
		struct ecm_db_iface_instance *iface[ECM_DB_OBJ_DIR_MAX];
#endif

		/*
		 * Remove it from the connection hash table
		 */
		if (!ci->hash_prev) {
			DEBUG_ASSERT(ecm_db_connection_table[ci->hash_index] == ci, "%px: hash table bad\n", ci);
			ecm_db_connection_table[ci->hash_index] = ci->hash_next;
		} else {
			ci->hash_prev->hash_next = ci->hash_next;
		}
		if (ci->hash_next) {
			ci->hash_next->hash_prev = ci->hash_prev;
		}
		ci->hash_prev = NULL;
		ci->hash_next = NULL;
		ecm_db_connection_table_lengths[ci->hash_index]--;
		DEBUG_ASSERT(ecm_db_connection_table_lengths[ci->hash_index] >= 0, "%px: invalid table len %d\n", ci, ecm_db_connection_table_lengths[ci->hash_index]);

		/*
		 * Remove it from the connection serial hash table
		 */
		if (!ci->serial_hash_prev) {
			DEBUG_ASSERT(ecm_db_connection_serial_table[ci->serial_hash_index] == ci, "%px: hash table bad\n", ci);
			ecm_db_connection_serial_table[ci->serial_hash_index] = ci->serial_hash_next;
		} else {
			ci->serial_hash_prev->serial_hash_next = ci->serial_hash_next;
		}
		if (ci->serial_hash_next) {
			ci->serial_hash_next->serial_hash_prev = ci->serial_hash_prev;
		}
		ci->serial_hash_prev = NULL;
		ci->serial_hash_next = NULL;
		ecm_db_connection_serial_table_lengths[ci->serial_hash_index]--;
		DEBUG_ASSERT(ecm_db_connection_serial_table_lengths[ci->serial_hash_index] >= 0, "%px: invalid table len %d\n", ci, ecm_db_connection_serial_table_lengths[ci->serial_hash_index]);

		/*
		 * Remove from the global list
		 */
		if (!ci->prev) {
			DEBUG_ASSERT(ecm_db_connections == ci, "%px: conn table bad\n", ci);
			ecm_db_connections = ci->next;
		} else {
			ci->prev->next = ci->next;
		}
		if (ci->next) {
			ci->next->prev = ci->prev;
		}
		ci->prev = NULL;
		ci->next = NULL;

#ifdef ECM_DB_XREF_ENABLE
		/*
		 * Remove connection from the mappings' connection list
		 */
		for (dir = 0; dir < ECM_DB_OBJ_DIR_MAX; dir++) {
			if (!ci->mapping_prev[dir]) {
				DEBUG_ASSERT(ci->mapping[dir]->connections[dir] == ci, "%px: %s conn table bad\n", ci, ecm_db_obj_dir_strings[dir]);
				ci->mapping[dir]->connections[dir] = ci->mapping_next[dir];
			} else {
				ci->mapping_prev[dir]->mapping_next[dir] = ci->mapping_next[dir];
			}
			if (ci->mapping_next[dir]) {
				ci->mapping_next[dir]->mapping_prev[dir] = ci->mapping_prev[dir];
			}
			ci->mapping_prev[dir] = NULL;
			ci->mapping_next[dir] = NULL;
		}

		/*
		 * Remove connection from the ifaces' connection list
		 * GGG TODO Deprecated. Interface lists will be used instead. To be deleted.
		 */
		for (dir = 0; dir < ECM_DB_OBJ_DIR_MAX; dir++) {
			iface[dir] = ci->node[dir]->iface;
			if (!ci->iface_prev[dir]) {
				DEBUG_ASSERT(iface[dir]->connections[dir] == ci,
					     "%px: iface %s conn table bad\n",
					     ci, ecm_db_obj_dir_strings[dir]);
				iface[dir]->connections[dir] = ci->iface_next[dir];
			} else {
				ci->iface_prev[dir]->iface_next[dir] = ci->iface_next[dir];
			}
			if (ci->iface_next[dir]) {
				ci->iface_next[dir]->iface_prev[dir] = ci->iface_prev[dir];
			}
			ci->iface_prev[dir] = NULL;
			ci->iface_next[dir] = NULL;
		}

		/*
		 * Remove connection from its nodes' connection list
		 */
		for (dir = 0; dir < ECM_DB_OBJ_DIR_MAX; dir++) {
			if (!ci->node_prev[dir]) {
				DEBUG_ASSERT(ci->node[dir]->connections[dir] == ci,
					     "%px: %s node conn table bad, got: %px\n",
					     ci, ecm_db_obj_dir_strings[dir], ci->node[dir]->connections[dir]);
				ci->node[dir]->connections[dir] = ci->node_next[dir];
			} else {
				ci->node_prev[dir]->node_next[dir] = ci->node_next[dir];
			}
			if (ci->node_next[dir]) {
				ci->node_next[dir]->node_prev[dir] = ci->node_prev[dir];
			}
			ci->node_prev[dir] = NULL;
			ci->node_next[dir] = NULL;
			ci->node[dir]->connections_count[dir]--;
			DEBUG_ASSERT(ci->node[dir]->connections_count[dir] >= 0, "%px: %s node bad count\n", ci, ecm_db_obj_dir_strings[dir]);
		}
#endif

		/*
		 * Update the counters in the mappings
		 */
		if (ci->protocol == IPPROTO_UDP) {
			for (dir = 0; dir < ECM_DB_OBJ_DIR_MAX; dir++) {
				ci->mapping[dir]->udp_count[dir]--;
			}
		} else if (ci->protocol == IPPROTO_TCP) {
			for (dir = 0; dir < ECM_DB_OBJ_DIR_MAX; dir++) {
				ci->mapping[dir]->tcp_count[dir]--;
			}
		}

		for (dir = 0; dir < ECM_DB_OBJ_DIR_MAX; dir++) {
			ci->mapping[dir]->conn_count[dir]--;
		}

		/*
		 * Assert that the defunt timer has been detached
		 */
		DEBUG_ASSERT(ci->defunct_timer.group == ECM_DB_TIMER_GROUPS_MAX, "%px: unexpected timer group %d\n", ci, ci->defunct_timer.group);

		/*
		 * Decrement protocol counter stats
		 */
		ecm_db_connection_count_by_protocol[ci->protocol]--;
		DEBUG_ASSERT(ecm_db_connection_count_by_protocol[ci->protocol] >= 0, "%px: Invalid protocol count %d\n", ci, ecm_db_connection_count_by_protocol[ci->protocol]);

		spin_unlock_bh(&ecm_db_lock);

		/*
		 * Throw removed event to listeners
		 */
		DEBUG_TRACE("%px: Throw connection removed event\n", ci);
		li = ecm_db_listeners_get_and_ref_first();
		while (li) {
			struct ecm_db_listener_instance *lin;
			if (li->connection_removed) {
				li->connection_removed(li->arg, ci);
			}

			/*
			 * Get next listener
			 */
			lin = ecm_db_listener_get_and_ref_next(li);
			ecm_db_listener_deref(li);
			li = lin;
		}
	}

#ifdef ECM_DB_CTA_TRACK_ENABLE
	/*
	 * Unlink from the "assignments by classifier type" lists.
	 *
	 * This is done whether the connection is inserted into the database or not - this is because
	 * classifier assignments take place before adding into the db.
	 *
	 * NOTE: We know that the ci is not being iterated in any of these lists because otherwise
	 * ci would be being held as part of iteration and so we would not be here!
	 * Equally we know that if the assignments_by_type[] element is non-null then it must also be in the relevant list too.
	 *
	 * Default classifier is not in the classifier type assignement list, so we should start the loop index
	 * with the first assigned classifier type.
	 */
	spin_lock_bh(&ecm_db_lock);
	for (ca_type = ECM_CLASSIFIER_TYPE_DEFAULT + 1; ca_type < ECM_CLASSIFIER_TYPES; ++ca_type) {
		struct ecm_classifier_instance *cci = ci->assignments_by_type[ca_type];
		if (!cci) {
			/*
			 * No assignment of this type, so would not be in the classifier type assignments list
			 */
			continue;
		}
		_ecm_db_connection_classifier_unassign(ci, cci, ca_type);
	}
	spin_unlock_bh(&ecm_db_lock);
#endif

	/*
	 * Throw final event
	 */
	if (ci->final) {
		ci->final(ci->arg);
	}

	/*
	 * Release instances to the objects referenced by the connection
	 */
	while (ci->assignments) {
		struct ecm_classifier_instance *classi = ci->assignments;
		ci->assignments = classi->ca_next;
		classi->deref(classi);
	}

	for (dir = 0; dir < ECM_DB_OBJ_DIR_MAX; dir++) {
		if (ci->mapping[dir]) {
			ecm_db_mapping_deref(ci->mapping[dir]);
		}
	}

	if (ci->feci) {
		ecm_front_end_connection_deref(ci->feci);
	}

	for (dir = 0; dir < ECM_DB_OBJ_DIR_MAX; dir++) {
		if (ci->node[dir]) {
			ecm_db_node_deref(ci->node[dir]);
		}
	}

	/*
	 * Remove references to the interfaces in our heirarchy lists
	 */
	for (dir = 0; dir < ECM_DB_OBJ_DIR_MAX; dir++) {
		for (i = ci->interface_first[dir]; i < ECM_DB_IFACE_HEIRARCHY_MAX; ++i) {
			DEBUG_TRACE("%px: %s interface %d remove: %px\n", ci, ecm_db_obj_dir_strings[dir], i, ci->interfaces[dir][i]);
			ecm_db_iface_deref(ci->interfaces[dir][i]);
		}
	}

#ifdef ECM_MULTICAST_ENABLE
	/*
	 * Remove references to the multicast interfaces of this connection.
	 */
	ecm_db_multicast_connection_to_interfaces_clear(ci);
#endif
	/*
	 * We can now destroy the instance
	 */
	DEBUG_CLEAR_MAGIC(ci);
	kfree(ci);

	/*
	 * Decrease global connection count
	 */
	spin_lock_bh(&ecm_db_lock);
	ecm_db_connection_count--;
	DEBUG_ASSERT(ecm_db_connection_count >= 0, "%px: connection count wrap\n", ci);
	spin_unlock_bh(&ecm_db_lock);

	return 0;
}
EXPORT_SYMBOL(ecm_db_connection_deref);

/*
 * ecm_db_connection_defunct_all()
 *	Make defunct ALL connections.
 *
 * This API is typically used in shutdown situations commanded by the user.
 * NOTE: Ensure all front ends are stopped to avoid further connections being created while this is running.
 */
void ecm_db_connection_defunct_all(void)
{
	struct ecm_db_connection_instance *ci;

	DEBUG_INFO("Defuncting all\n");

	/*
	 * Iterate all connections
	 */
	ci = ecm_db_connections_get_and_ref_first();
	while (ci) {
		struct ecm_db_connection_instance *cin;

		DEBUG_TRACE("%px: defunct\n", ci);
		ecm_db_connection_make_defunct(ci);

		cin = ecm_db_connection_get_and_ref_next(ci);
		ecm_db_connection_deref(ci);
		ci = cin;
	}
	DEBUG_INFO("Defuncting complete\n");
}
EXPORT_SYMBOL(ecm_db_connection_defunct_all);

#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
/*
 * ecm_db_connection_defunct_by_classifier()
 *	Make defunct based on masked fields
 */
void ecm_db_connection_defunct_by_classifier(int ip_ver, ip_addr_t src_addr_mask, uint16_t src_port_mask,
					     ip_addr_t dest_addr_mask, uint16_t dest_port_mask,
					     int proto_mask, bool is_routed, ecm_classifier_type_t ca_type)
{
	struct ecm_db_connection_instance *ci;
	int cnt = 0;
	char *direction = NULL;

	/*
	 * Iterate all connections
	 */
	ci = ecm_db_connections_get_and_ref_first();
	while (ci) {
		struct ecm_db_connection_instance *cin;
		struct ecm_classifier_instance *eci;
		ip_addr_t sip;
		ip_addr_t dip;
		uint16_t sport, dport;
		int proto;

		DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", ci);

		eci = ecm_db_connection_assigned_classifier_find_and_ref(ci, ca_type);
		if (!eci) {
			goto next_ci;
		}
		eci->deref(eci);

		/*
		 *  Ignore connection with wrong version
		 */
		if (ip_ver != ECM_DB_IP_VERSION_IGNORE && (ecm_db_connection_ip_version_get(ci) != ip_ver)) {
			goto next_ci;
		}

		/*
		 * Skip routed connection for bridge flow
		 * Skip bridge connection for routed flow
		 */
		if (is_routed != ecm_db_connection_is_routed_get(ci)) {
			goto next_ci;
		}

		/*
		 * Check protocol if specified
		 */
		proto = ecm_db_connection_protocol_get(ci);
		if (!ECM_PROTO_MASK_MATCH(proto, proto_mask)) {
			goto next_ci;
		}

		/*
		 * A : PCI < ------- br-home ----------bridging----------------br-wan ---->PC2
		 *
		 * B : PCI < ------- br-home ----------Routing----------------br-wan ---->PC2
		 *
		 *							DNAT
		 * C : PCI < ------- br-home ----------Routing----------------br-wan ----> PC2
		 *							SNAT
		 * D : PCI < ------- br-home ----------Routing----------------br-wan ----> PC2
		 */
		ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_FROM, sip);
		ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_TO, dip);
		sport = ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_FROM);
		dport = ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_TO);

		/*
		 * 1. For topology A, B, C, D  if drop rule is added in br-home or br-wan
		 * 2. For topoloy  A, B  if drop rule is added in br-wan
		 * Match in flow direction
		 */
		if (ECM_IP_ADDR_MASK_MATCH(sip, src_addr_mask) &&
		    ECM_IP_ADDR_MASK_MATCH(dip, dest_addr_mask) &&
		    ECM_PORT_MASK_MATCH(sport, src_port_mask) &&
		    ECM_PORT_MASK_MATCH(dport, dest_port_mask)) {
			direction = "flow";
			goto defunct_conn;
		}

		/*
		 * 1. For topology A, B, C, D  if drop rule is added in br-home or br-wan
		 * 2. For topoloy  A, B  if drop rule is added in br-wan
		 * Match in reverse direction
		 */
		if (ECM_IP_ADDR_MASK_MATCH(dip, src_addr_mask) &&
		    ECM_IP_ADDR_MASK_MATCH(sip, dest_addr_mask) &&
		    ECM_PORT_MASK_MATCH(dport, src_port_mask) &&
		    ECM_PORT_MASK_MATCH(sport, dest_port_mask)) {
			direction = "reverse";
			goto defunct_conn;
		}

		/*
		 * There is no NATing in case of bridging
		 */
		if (!is_routed) {
			goto next_ci;
		}

		ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_FROM_NAT, sip);
		ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_TO_NAT, dip);
		sport = ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_FROM_NAT);
		dport = ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_TO_NAT);

		/*
		 * 1. For topoloy  C, D  if drop rule is added in br-wan
		 * Match in flow direction
		 */
		if (ECM_IP_ADDR_MASK_MATCH(sip, src_addr_mask) &&
		    ECM_IP_ADDR_MASK_MATCH(dip, dest_addr_mask) &&
		    ECM_PORT_MASK_MATCH(sport, src_port_mask) &&
		    ECM_PORT_MASK_MATCH(dport, dest_port_mask)) {
			direction = "flow (nat)";
			goto defunct_conn;
		}

		/*
		 * 1. For topoloy  C, D  if drop rule is added in br-wan
		 * Match in reverse direction
		 */
		if (ECM_IP_ADDR_MASK_MATCH(dip, src_addr_mask) &&
		    ECM_IP_ADDR_MASK_MATCH(sip, dest_addr_mask) &&
		    ECM_PORT_MASK_MATCH(dport, src_port_mask) &&
		    ECM_PORT_MASK_MATCH(sport, dest_port_mask)) {
			direction = "reverse (nat)";
			goto defunct_conn;
		}

		goto next_ci;

defunct_conn:
		cnt++;
		if (ECM_IP_ADDR_IS_V4(src_addr_mask)) {
			DEBUG_TRACE("%px: Defunct CI masked 5 tuple match(%s) src=" ECM_IP_ADDR_DOT_FMT " sport=%d dest="
					ECM_IP_ADDR_DOT_FMT ", dport=%d, proto=%d cnt=%d\n", ci, direction,
					ECM_IP_ADDR_TO_DOT(sip), sport, ECM_IP_ADDR_TO_DOT(dip), dport, proto, cnt);
		} else {
			DEBUG_TRACE("%px: Defunct CI masked 5 tuple match(%s) src=" ECM_IP_ADDR_OCTAL_FMT " sport=%d dest="
					ECM_IP_ADDR_OCTAL_FMT ", dport=%d, proto=%d, cnt=%d\n", ci, direction,
					ECM_IP_ADDR_TO_OCTAL(sip), sport, ECM_IP_ADDR_TO_OCTAL(dip), dport, proto, cnt);
		}

		ecm_db_connection_make_defunct(ci);

next_ci:
		cin = ecm_db_connection_get_and_ref_next(ci);
		ecm_db_connection_deref(ci);
		ci = cin;
	}

	if (ECM_IP_ADDR_IS_V4(src_addr_mask)) {
		DEBUG_TRACE("%px: Defunct request by masked 5 tuple src_mask=" ECM_IP_ADDR_DOT_FMT " sport_mask=%d, dest_mask="
				ECM_IP_ADDR_DOT_FMT " dport_mask=%d, proto_mask=%d, cnt=%d\n", ci,
				ECM_IP_ADDR_TO_DOT(src_addr_mask), src_port_mask, ECM_IP_ADDR_TO_DOT(dest_addr_mask),
				dest_port_mask, proto_mask, cnt);
	} else {
		DEBUG_TRACE("%px: Defunct request by masked 5 tuple src_mask=" ECM_IP_ADDR_OCTAL_FMT " sport_mask=%d dest_mask="
				ECM_IP_ADDR_OCTAL_FMT " dport_mask=%d, proto_mask=%d cnt=%d\n", ci,
				ECM_IP_ADDR_TO_OCTAL(src_addr_mask), src_port_mask,
				ECM_IP_ADDR_TO_OCTAL(dest_addr_mask), dest_port_mask, proto_mask, cnt);
	}
}
#endif

/*
 * ecm_db_connection_defunct_by_port()
 *	 Make defunct based on source or destination port.
 */
void ecm_db_connection_defunct_by_port(int port, ecm_db_obj_dir_t dir)
{
	struct ecm_db_connection_instance *ci;

	DEBUG_INFO("Defuncting all matching connections by port %d\n", port);

	/*
	 * Iterate all connections
	 */
	ci = ecm_db_connections_get_and_ref_first();
	while (ci) {
		struct ecm_db_connection_instance *cin;
		DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed\n", ci);

		/*
		 * Flush the connection matching with given port address and flow direction
		 */
		if (port == htons(ecm_db_connection_port_get(ci, dir))) {
			DEBUG_TRACE("%px: defunct\n", ci);
			ecm_db_connection_make_defunct(ci);
		}

		cin = ecm_db_connection_get_and_ref_next(ci);
		ecm_db_connection_deref(ci);
		ci = cin;
	}
	DEBUG_INFO("Port based Defuncting complete\n");
}
EXPORT_SYMBOL(ecm_db_connection_defunct_by_port);

/*
 * ecm_db_connection_defunct_by_protocol()
 * 	Make defunct based on protocol.
 */
void ecm_db_connection_defunct_by_protocol(int protocol)
{
	struct ecm_db_connection_instance *ci;

	DEBUG_INFO("Defuncting all matching connections by protocol %d\n", protocol);

	/*
	 * Iterate all connections
	 */
	ci = ecm_db_connections_get_and_ref_first();
	while (ci) {
		struct ecm_db_connection_instance *cin;
		DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed\n", ci);

		/*
		 * Flush the connection matching with given protocol
		 */
		if (protocol == ecm_db_connection_protocol_get(ci)) {
			DEBUG_TRACE("%px: defunct\n", ci);
			ecm_db_connection_make_defunct(ci);
		}

		cin = ecm_db_connection_get_and_ref_next(ci);
		ecm_db_connection_deref(ci);
		ci = cin;
	}
	DEBUG_INFO("Protocol based defuncting complete\n");
}
EXPORT_SYMBOL(ecm_db_connection_defunct_by_protocol);

/*
 * ecm_db_connection_defunct_ip_version()
 *	Make defunct based on the IP version (IPv4 or IPv6).
 */
void ecm_db_connection_defunct_ip_version(int ip_version)
{
	struct ecm_db_connection_instance *ci;

	DEBUG_ASSERT(ip_version == 4 || ip_version == 6, "Wrong ip_version: %d\n", ip_version);

	DEBUG_INFO("Defuncting IPv%d connections\n", ip_version);

	/*
	 * Iterate all connections
	 */
	ci = ecm_db_connections_get_and_ref_first();
	while (ci) {
		struct ecm_db_connection_instance *cin;

		if (ci->ip_version == ip_version) {
			DEBUG_TRACE("%px: defunct\n", ci);
			ecm_db_connection_make_defunct(ci);
		}

		cin = ecm_db_connection_get_and_ref_next(ci);
		ecm_db_connection_deref(ci);
		ci = cin;
	}
	DEBUG_INFO("Defuncting complete for IPv%d connections\n", ip_version);
}

/*
 * ecm_db_connection_generate_hash_index()
 * 	Calculate the hash index.
 *
 * Note: The hash we produce is symmetric - i.e. we can swap the "from" and "to"
 * details without generating a different hash index!
 */
static inline ecm_db_connection_hash_t ecm_db_connection_generate_hash_index(ip_addr_t host1_addr, uint32_t host1_port, ip_addr_t host2_addr, uint32_t host2_port, int protocol)
{
	uint32_t hah1;
	uint32_t hah2;
	uint32_t ht1;
	uint32_t hash_val;

	/*
	 * The hash function only uses both host 1 address/port, host 2 address/port
	 * and protocol fields.
	 */
	ECM_IP_ADDR_HASH(hah1, host1_addr);
	ECM_IP_ADDR_HASH(hah2, host2_addr);
	ht1 = (u32)hah1 + host1_port + hah2 + host2_port + (uint32_t)protocol;
	hash_val = (uint32_t)jhash_1word(ht1, ecm_db_jhash_rnd);
	return (ecm_db_connection_hash_t)(hash_val & (ECM_DB_CONNECTION_HASH_SLOTS - 1));
}

/*
 * ecm_db_connection_generate_serial_hash_index()
 * 	Calculate the serial hash index.
 */
static inline ecm_db_connection_serial_hash_t ecm_db_connection_generate_serial_hash_index(uint32_t serial)
{
	uint32_t hash_val;
	hash_val = (uint32_t)jhash_1word(serial, ecm_db_jhash_rnd);

	return (ecm_db_connection_serial_hash_t)(hash_val & (ECM_DB_CONNECTION_SERIAL_HASH_SLOTS - 1));
}

/*
 * ecm_db_connection_find_and_ref_chain()
 *	Given a hash chain index locate the connection
 */
static struct ecm_db_connection_instance *ecm_db_connection_find_and_ref_chain(ecm_db_connection_hash_t hash_index,
											ip_addr_t host1_addr, ip_addr_t host2_addr,
											int protocol, int host1_port, int host2_port)
{
	struct ecm_db_connection_instance *ci;

	/*
	 * Iterate the chain looking for a connection with matching details
	 */
	spin_lock_bh(&ecm_db_lock);
	ci = ecm_db_connection_table[hash_index];
	while (ci) {
		/*
		 * The use of unlikely() is liberally used because under fast-hit scenarios the connection would always be at the start of a chain
		 */
		if (unlikely(ci->protocol != protocol)) {
			goto try_next;
		}

		if (unlikely(!ECM_IP_ADDR_MATCH(host1_addr, ci->mapping[ECM_DB_OBJ_DIR_FROM]->host->address))) {
			goto try_reverse;
		}

		if (unlikely(host1_port != ci->mapping[ECM_DB_OBJ_DIR_FROM]->port)) {
			goto try_reverse;
		}

		if (unlikely(!ECM_IP_ADDR_MATCH(host2_addr, ci->mapping[ECM_DB_OBJ_DIR_TO]->host->address))) {
			goto try_reverse;
		}

		if (unlikely(host2_port != ci->mapping[ECM_DB_OBJ_DIR_TO]->port)) {
			goto try_reverse;
		}

		goto connection_found;

try_reverse:
		if (unlikely(!ECM_IP_ADDR_MATCH(host1_addr, ci->mapping[ECM_DB_OBJ_DIR_TO]->host->address))) {
			goto try_next;
		}

		if (unlikely(host1_port != ci->mapping[ECM_DB_OBJ_DIR_TO]->port)) {
			goto try_next;
		}

		if (unlikely(!ECM_IP_ADDR_MATCH(host2_addr, ci->mapping[ECM_DB_OBJ_DIR_FROM]->host->address))) {
			goto try_next;
		}

		if (unlikely(host2_port != ci->mapping[ECM_DB_OBJ_DIR_FROM]->port)) {
			goto try_next;
		}

		goto connection_found;

try_next:
		ci = ci->hash_next;
	}
	spin_unlock_bh(&ecm_db_lock);
	DEBUG_TRACE("Connection not found in hash chain\n");
	return NULL;

connection_found:
	_ecm_db_connection_ref(ci);
	spin_unlock_bh(&ecm_db_lock);
	DEBUG_TRACE("Connection found %px\n", ci);
	return ci;
}

/*
 * ecm_db_connection_find_and_ref()
 *	Locate a connection instance based on addressing, protocol and optional port information.
 *
 * NOTE: For non-port based protocols then ports are expected to be -(protocol).
 */
struct ecm_db_connection_instance *ecm_db_connection_find_and_ref(ip_addr_t host1_addr, ip_addr_t host2_addr, int protocol, int host1_port, int host2_port)
{
	ecm_db_connection_hash_t hash_index;

	DEBUG_TRACE("Lookup connection " ECM_IP_ADDR_OCTAL_FMT ":%d <> " ECM_IP_ADDR_OCTAL_FMT ":%d protocol %d\n", ECM_IP_ADDR_TO_OCTAL(host1_addr), host1_port, ECM_IP_ADDR_TO_OCTAL(host2_addr), host2_port, protocol);

	/*
	 * Compute the hash chain index and prepare to walk the chain
	 */
	hash_index = ecm_db_connection_generate_hash_index(host1_addr, host1_port, host2_addr, host2_port, protocol);
	return ecm_db_connection_find_and_ref_chain(hash_index, host1_addr, host2_addr, protocol, host1_port, host2_port);
}
EXPORT_SYMBOL(ecm_db_connection_find_and_ref);

/*
 * ecm_db_connection_serial_find_and_ref()
 *	Locate a connection instance based on serial if it still exists
 */
struct ecm_db_connection_instance *ecm_db_connection_serial_find_and_ref(uint32_t serial)
{
	ecm_db_connection_serial_hash_t serial_hash_index;
	struct ecm_db_connection_instance *ci;

	DEBUG_TRACE("Lookup connection serial: %u\n", serial);

	/*
	 * Compute the hash chain index and prepare to walk the chain
	 */
	serial_hash_index = ecm_db_connection_generate_serial_hash_index(serial);

	/*
	 * Iterate the chain looking for a connection with matching serial
	 */
	spin_lock_bh(&ecm_db_lock);
	ci = ecm_db_connection_serial_table[serial_hash_index];
	while (ci) {
		/*
		 * The use of likely() is used because under fast-hit scenarios the connection would always be at the start of a chain
		 */
		if (likely(ci->serial == serial)) {
			_ecm_db_connection_ref(ci);
			spin_unlock_bh(&ecm_db_lock);
			DEBUG_TRACE("Connection found %px\n", ci);
			return ci;
		}

		ci = ci->serial_hash_next;
	}
	spin_unlock_bh(&ecm_db_lock);
	DEBUG_TRACE("Connection not found\n");
	return NULL;
}
EXPORT_SYMBOL(ecm_db_connection_serial_find_and_ref);

/*
 * ecm_db_connection_node_get_and_ref()
 *	Return node reference
 */
struct ecm_db_node_instance *ecm_db_connection_node_get_and_ref(struct ecm_db_connection_instance *ci, ecm_db_obj_dir_t dir)
{
	struct ecm_db_node_instance *ni;

	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed\n", ci);

	spin_lock_bh(&ecm_db_lock);
	ni = ci->node[dir];
	_ecm_db_node_ref(ni);
	spin_unlock_bh(&ecm_db_lock);
	return ni;
}
EXPORT_SYMBOL(ecm_db_connection_node_get_and_ref);

#ifdef ECM_DB_XREF_ENABLE
/*
 * ecm_db_connection_mapping_get_and_ref_next()
 *	Return reference to next connection in the mapping chain in the specified direction.
 */
struct ecm_db_connection_instance *ecm_db_connection_mapping_get_and_ref_next(struct ecm_db_connection_instance *ci, ecm_db_obj_dir_t dir)
{
	struct ecm_db_connection_instance *nci;

	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed\n", ci);

	spin_lock_bh(&ecm_db_lock);
	nci = ci->mapping_next[dir];
	if (nci) {
		_ecm_db_connection_ref(nci);
	}
	spin_unlock_bh(&ecm_db_lock);

	return nci;
}
EXPORT_SYMBOL(ecm_db_connection_mapping_get_and_ref_next);

/*
 * ecm_db_connection_iface_get_and_ref_next()
 *	Return reference to next connection in iface chain in the specified direction.
 */
struct ecm_db_connection_instance *ecm_db_connection_iface_get_and_ref_next(struct ecm_db_connection_instance *ci, ecm_db_obj_dir_t dir)
{
	struct ecm_db_connection_instance *nci;

	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed\n", ci);

	spin_lock_bh(&ecm_db_lock);
	nci = ci->iface_next[dir];
	if (nci) {
		_ecm_db_connection_ref(nci);
	}
	spin_unlock_bh(&ecm_db_lock);

	return nci;
}
EXPORT_SYMBOL(ecm_db_connection_iface_get_and_ref_next);
#endif

/*
 * ecm_db_connection_mapping_get_and_ref()
 * 	Return a reference to the mapping of the connection in the specified direction.
 */
struct ecm_db_mapping_instance *ecm_db_connection_mapping_get_and_ref(struct ecm_db_connection_instance *ci, ecm_db_obj_dir_t dir)
{
	struct ecm_db_mapping_instance *mi;

	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed\n", ci);

	spin_lock_bh(&ecm_db_lock);
	mi = ci->mapping[dir];
	_ecm_db_mapping_ref(mi);
	spin_unlock_bh(&ecm_db_lock);
	return mi;
}
EXPORT_SYMBOL(ecm_db_connection_mapping_get_and_ref);

/*
 * ecm_db_connection_classifier_assign()
 *	Assign a classifier to the connection assigned classifier list.
 *
 * This adds the classifier in the ci->assignments list in ascending priority order according to the classifier type.
 * Only assigned classifiers are in this list, allowing fast retrival of current assignments, avoiding the need to skip over unassigned classifiers.
 * Because there is only one of each type of classifier the classifier is also recorded in an array, the position in which is its type value.
 * This allows fast lookup based on type too.
 * Further, the connection is recorded in the classifier type assignment array too, this permits iterating of all connections that are assigned to a TYPE of classifier.
 */
void ecm_db_connection_classifier_assign(struct ecm_db_connection_instance *ci, struct ecm_classifier_instance *new_ca)
{
	struct ecm_classifier_instance *ca;
	struct ecm_classifier_instance *ca_prev;
	ecm_classifier_type_t new_ca_type;
#ifdef ECM_DB_CTA_TRACK_ENABLE
	struct ecm_db_connection_classifier_type_assignment *ta;
	struct ecm_db_connection_classifier_type_assignment_list *tal;
#endif

	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed\n", ci);

	/*
	 * Get the type (which is also used as the priority)
	 */
	new_ca_type = new_ca->type_get(new_ca);

	/*
	 * Connection holds ref to the classifier
	 */
	new_ca->ref(new_ca);

	/*
	 * Find place to insert the classifier
	 */
	spin_lock_bh(&ecm_db_lock);
	ca = ci->assignments;
	ca_prev = NULL;
	while (ca) {
		ecm_classifier_type_t ca_type;
		ca_type = ca->type_get(ca);

		/*
		 * If new ca is less important that the current assigned classifier insert here
		 */
		if (new_ca_type < ca_type) {
			break;
		}
		ca_prev = ca;
		ca = ca->ca_next;
	}

	/*
	 * Insert new_ca before ca and after ca_prev.
	 */
	new_ca->ca_prev = ca_prev;
	if (ca_prev) {
		ca_prev->ca_next = new_ca;
	} else {
		DEBUG_ASSERT(ci->assignments == ca, "%px: Bad assigmnment list, expecting: %px, got: %px\n", ci, ca, ci->assignments);
		ci->assignments = new_ca;
	}

	new_ca->ca_next = ca;
	if (ca) {
		ca->ca_prev = new_ca;
	}

	/*
	 * Insert based on type too
	 */
	DEBUG_ASSERT(ci->assignments_by_type[new_ca_type] == NULL, "%px: Only one of each type: %d may be registered, new: %px, existing, %px\n",
			ci, new_ca_type, new_ca, ci->assignments_by_type[new_ca_type]);
	ci->assignments_by_type[new_ca_type] = new_ca;

#ifdef ECM_DB_CTA_TRACK_ENABLE
	/*
	 * Default classifier will not be added to the classifier type assignment list.
	 * Only assigned classifiers can be added.
	 */
	if (new_ca_type == ECM_CLASSIFIER_TYPE_DEFAULT) {
		spin_unlock_bh(&ecm_db_lock);
		return;
	}

	/*
	 * Add the connection into the type assignment list too.
	 */
	ta = &ci->type_assignment[new_ca_type];
	if (ta->pending_unassign) {
		/*
		 * The connection is pending unassignment / removal from list, but since it has been
		 * re-assigned to the same type of classifier we can just clear the flag and avoid the removal.
		 * NOTE: pending_unassign is only ever true if the iteration count is non-zero i.e. iteration is in progress.
		 */
		DEBUG_CHECK_MAGIC(ta, ECM_DB_CLASSIFIER_TYPE_ASSIGNMENT_MAGIC, "%px: magic failed, ci: %px", ta, ci);
		DEBUG_ASSERT(ta->iteration_count != 0, "%px: Bad pending_unassign: type: %d, Iteration count zero\n", ci, new_ca_type);
		ta->pending_unassign = false;
		spin_unlock_bh(&ecm_db_lock);
		return;
	}

	/*
	 * iteration_count should be zero as there should not be a duplicate assignment of the same type.
	 * This is because if iteration_count was non-zero then pending_unassign should have been true.
	 */
	DEBUG_ASSERT(ta->iteration_count == 0, "%px: Type: %d, Iteration count not zero: %d\n", ci, new_ca_type, ta->iteration_count);

	/*
	 * Insert the connection into the classifier type assignment list, at the head
	 */
	tal = &ecm_db_connection_classifier_type_assignments[new_ca_type];
	ta->next = tal->type_assignments_list;
	ta->prev = NULL;

	/*
	 * If there is an existing head, it is no longer the head
	 */
	if (tal->type_assignments_list) {
		struct ecm_db_connection_classifier_type_assignment *talh;
		talh = &tal->type_assignments_list->type_assignment[new_ca_type];
		talh->prev = ci;
	}

	/*
	 * Set new head
	 */
	tal->type_assignments_list = ci;

	/*
	 * Set magic
	 */
	DEBUG_SET_MAGIC(ta, ECM_DB_CLASSIFIER_TYPE_ASSIGNMENT_MAGIC);

	/*
	 * Increment assignment count
	 */
	tal->type_assignment_count++;
	DEBUG_ASSERT(tal->type_assignment_count > 0, "Bad iteration count: %d\n", tal->type_assignment_count);
#endif
	spin_unlock_bh(&ecm_db_lock);
}
EXPORT_SYMBOL(ecm_db_connection_classifier_assign);

/*
 * ecm_db_connection_classifier_assignments_get_and_ref()
 *	Populate the given array with references to the currently assigned classifiers.
 *
 * This function returns the number of assignments starting from [0].
 * [0] is the lowest priority classifier, [return_val - 1] is the highest priority.
 * Release each classifier when you are done, for convenience use ecm_db_connection_assignments_release().
 *
 * NOTE: The array also contains the default classifier too which of course will always be at [0]
 *
 * WARNING: The array MUST be of size ECM_CLASSIFIER_TYPES.
 */
int ecm_db_connection_classifier_assignments_get_and_ref(struct ecm_db_connection_instance *ci, struct ecm_classifier_instance *assignments[])
{
	int aci_count;
	struct ecm_classifier_instance *aci;
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed\n", ci);

	aci_count = 0;
	spin_lock_bh(&ecm_db_lock);
	aci = ci->assignments;
	while (aci) {
		aci->ref(aci);
		assignments[aci_count++] = aci;
		aci = aci->ca_next;
	}
	spin_unlock_bh(&ecm_db_lock);
	DEBUG_ASSERT(aci_count >= 1, "%px: Must have at least default classifier!\n", ci);
	return aci_count;
}
EXPORT_SYMBOL(ecm_db_connection_classifier_assignments_get_and_ref);

/*
 * ecm_db_connection_assignments_release()
 * 	Release references to classifiers in the assignments array
 */
void ecm_db_connection_assignments_release(int assignment_count, struct ecm_classifier_instance *assignments[])
{
	int i;
	for (i = 0; i < assignment_count; ++i) {
		struct ecm_classifier_instance *aci = assignments[i];
		if (aci) {
			aci->deref(aci);
		}
	}
}
EXPORT_SYMBOL(ecm_db_connection_assignments_release);

/*
 * ecm_db_connection_assigned_classifier_find_and_ref()
 *	Return a ref to classifier of the requested type, if found
 */
struct ecm_classifier_instance *ecm_db_connection_assigned_classifier_find_and_ref(struct ecm_db_connection_instance *ci, ecm_classifier_type_t type)
{
	struct ecm_classifier_instance *ca;
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed\n", ci);
	spin_lock_bh(&ecm_db_lock);
	ca = ci->assignments_by_type[type];
	if (ca) {
		ca->ref(ca);
	}
	spin_unlock_bh(&ecm_db_lock);
	return ca;
}
EXPORT_SYMBOL(ecm_db_connection_assigned_classifier_find_and_ref);

/*
 * ecm_db_connection_classifier_unassign()
 *	Unassign a classifier
 *
 * The default classifier cannot be unassigned.
 */
void ecm_db_connection_classifier_unassign(struct ecm_db_connection_instance *ci, struct ecm_classifier_instance *cci)
{
	ecm_classifier_type_t ca_type;

	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed\n", ci);

	/*
	 * Get the type
	 */
	ca_type = cci->type_get(cci);
	DEBUG_ASSERT(ca_type != ECM_CLASSIFIER_TYPE_DEFAULT, "%px: Cannot unassign default", ci);

	if (ca_type >= ECM_CLASSIFIER_TYPES) {
		DEBUG_WARN("%px: ca_type: %d is higher than the max classifier type number: %d\n", ci, ca_type, (ECM_CLASSIFIER_TYPES - 1));
		return;
	}

	DEBUG_TRACE("%px: Unassign type: %d, classifier: %px\n", ci, ca_type, cci);

	/*
	 * NOTE: It is possible that in SMP this classifier has already been unassigned.
	 */
	spin_lock_bh(&ecm_db_lock);
	if (ci->assignments_by_type[ca_type] == NULL) {
		spin_unlock_bh(&ecm_db_lock);
		DEBUG_TRACE("%px: Classifier type: %d already unassigned\n", ci, ca_type);
		return;
	}
	_ecm_db_connection_classifier_unassign(ci, cci, ca_type);
	spin_unlock_bh(&ecm_db_lock);
}
EXPORT_SYMBOL(ecm_db_connection_classifier_unassign);

#ifdef ECM_DB_CTA_TRACK_ENABLE
/*
 * ecm_db_connection_by_classifier_type_assignment_get_and_ref_first()
 *	Return a reference to the first connection for which a classifier of the given type is associated with
 *
 * WARNING: YOU MUST NOT USE ecm_db_connection_deref() to release the references taken using this API.
 * YOU MUST use ecm_db_connection_by_classifier_type_assignment_deref(), this ensures type assignment list integrity.
 */
struct ecm_db_connection_instance *ecm_db_connection_by_classifier_type_assignment_get_and_ref_first(ecm_classifier_type_t ca_type)
{
	struct ecm_db_connection_classifier_type_assignment_list *tal;
	struct ecm_db_connection_instance *ci;

	DEBUG_ASSERT(ca_type < ECM_CLASSIFIER_TYPES, "Bad type: %d\n", ca_type);

	DEBUG_TRACE("Get and ref first connection assigned with classifier type: %d\n", ca_type);

	tal = &ecm_db_connection_classifier_type_assignments[ca_type];
	spin_lock_bh(&ecm_db_lock);
	ci = tal->type_assignments_list;
	while (ci) {
		struct ecm_db_connection_classifier_type_assignment *ta;
		ta = &ci->type_assignment[ca_type];
		DEBUG_CHECK_MAGIC(ta, ECM_DB_CLASSIFIER_TYPE_ASSIGNMENT_MAGIC, "%px: magic failed, ci: %px", ta, ci);

		if (ta->pending_unassign) {
			DEBUG_TRACE("Skip %px, pending unassign for type: %d\n", ci, ca_type);
			ci = ta->next;
			continue;
		}

		/*
		 * Take reference to this connection.
		 * NOTE: Hold both the connection and the assignment entry so that when we unlock both the connection
		 * and the type assignment list entry maintains integrity.
		 */
		_ecm_db_connection_ref(ci);
		ta->iteration_count++;
		DEBUG_ASSERT(ta->iteration_count > 0, "Bad Iteration count: %d for type: %d, connection: %px\n", ta->iteration_count, ca_type, ci);
		spin_unlock_bh(&ecm_db_lock);
		return ci;
	}
	spin_unlock_bh(&ecm_db_lock);
	return NULL;
}
EXPORT_SYMBOL(ecm_db_connection_by_classifier_type_assignment_get_and_ref_first);

/*
 * ecm_db_connection_by_classifier_type_assignment_get_and_ref_next()
 *	Return a reference to the next connection for which a classifier of the given type is associated with.
 *
 * WARNING: YOU MUST NOT USE ecm_db_connection_deref() to release the references taken using this API.
 * YOU MUST use ecm_db_connection_by_classifier_type_assignment_deref(), this ensures type assignment list integrity.
 */
struct ecm_db_connection_instance *ecm_db_connection_by_classifier_type_assignment_get_and_ref_next(struct ecm_db_connection_instance *ci, ecm_classifier_type_t ca_type)
{
	struct ecm_db_connection_classifier_type_assignment *ta;
	struct ecm_db_connection_instance *cin;

	DEBUG_ASSERT(ca_type < ECM_CLASSIFIER_TYPES, "Bad type: %d\n", ca_type);
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed\n", ci);

	DEBUG_TRACE("Get and ref next connection assigned with classifier type: %d and ci: %px\n", ca_type, ci);

	spin_lock_bh(&ecm_db_lock);
	ta = &ci->type_assignment[ca_type];
	cin = ta->next;
	while (cin) {
		struct ecm_db_connection_classifier_type_assignment *tan;

		tan = &cin->type_assignment[ca_type];
		DEBUG_CHECK_MAGIC(tan, ECM_DB_CLASSIFIER_TYPE_ASSIGNMENT_MAGIC, "%px: magic failed, ci: %px", tan, cin);

		if (tan->pending_unassign) {
			DEBUG_TRACE("Skip %px, pending unassign for type: %d\n", cin, ca_type);
			cin = tan->next;
			continue;
		}

		/*
		 * Take reference to this connection.
		 * NOTE: Hold both the connection and the assignment entry so that when we unlock both the connection
		 * and the type assignment list entry maintains integrity.
		 */
		_ecm_db_connection_ref(cin);
		tan->iteration_count++;
		DEBUG_ASSERT(tan->iteration_count > 0, "Bad Iteration count: %d for type: %d, connection: %px\n", tan->iteration_count, ca_type, cin);
		spin_unlock_bh(&ecm_db_lock);
		return cin;
	}
	spin_unlock_bh(&ecm_db_lock);
	return NULL;
}
EXPORT_SYMBOL(ecm_db_connection_by_classifier_type_assignment_get_and_ref_next);

/*
 * ecm_db_connection_by_classifier_type_assignment_deref()
 *	Release a reference to a connection while iterating a classifier type assignment list
 */
void ecm_db_connection_by_classifier_type_assignment_deref(struct ecm_db_connection_instance *ci, ecm_classifier_type_t ca_type)
{
	struct ecm_db_connection_classifier_type_assignment_list *tal;
	struct ecm_db_connection_classifier_type_assignment *ta;

	DEBUG_ASSERT(ca_type < ECM_CLASSIFIER_TYPES, "Bad type: %d\n", ca_type);
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed\n", ci);

	tal = &ecm_db_connection_classifier_type_assignments[ca_type];

	/*
	 * Drop the iteration count
	 */
	spin_lock_bh(&ecm_db_lock);
	ta = &ci->type_assignment[ca_type];
	DEBUG_CHECK_MAGIC(ta, ECM_DB_CLASSIFIER_TYPE_ASSIGNMENT_MAGIC, "%px: magic failed, ci: %px", ta, ci);
	ta->iteration_count--;
	DEBUG_ASSERT(ta->iteration_count >= 0, "Bad Iteration count: %d for type: %d, connection: %px\n", ta->iteration_count, ca_type, ci);

	/*
	 * If there are no more iterations on-going and this is pending unassign then we can remove it from the assignments list
	 */
	if (ta->pending_unassign && (ta->iteration_count == 0)) {
		DEBUG_INFO("%px: Remove type assignment: %d\n", ci, ca_type);
		_ecm_db_classifier_type_assignment_remove(ci, ca_type);
	}
	spin_unlock_bh(&ecm_db_lock);
	ecm_db_connection_deref(ci);
}
EXPORT_SYMBOL(ecm_db_connection_by_classifier_type_assignment_deref);

/*
 * ecm_db_connection_make_defunct_by_assignment_type()
 *	Make defunct all connections that are currently assigned to a classifier of the given type
 */
void ecm_db_connection_make_defunct_by_assignment_type(ecm_classifier_type_t ca_type)
{
	struct ecm_db_connection_instance *ci;

	DEBUG_INFO("Make defunct all assigned to type: %d\n", ca_type);

	ci = ecm_db_connection_by_classifier_type_assignment_get_and_ref_first(ca_type);
	while (ci) {
		struct ecm_db_connection_instance *cin;

		DEBUG_TRACE("%px: Make defunct: %d\n", ci, ca_type);
		ecm_db_connection_make_defunct(ci);

		cin = ecm_db_connection_by_classifier_type_assignment_get_and_ref_next(ci, ca_type);
		ecm_db_connection_by_classifier_type_assignment_deref(ci, ca_type);
		ci = cin;
	}
}
EXPORT_SYMBOL(ecm_db_connection_make_defunct_by_assignment_type);

/*
 * ecm_db_connection_regenerate_by_assignment_type()
 *	Cause regeneration all connections that are currently assigned to a classifier of the given type
 */
void ecm_db_connection_regenerate_by_assignment_type(ecm_classifier_type_t ca_type)
{
	struct ecm_db_connection_instance *ci;

	DEBUG_INFO("Regenerate all assigned to type: %d\n", ca_type);

	ci = ecm_db_connection_by_classifier_type_assignment_get_and_ref_first(ca_type);
	while (ci) {
		struct ecm_db_connection_instance *cin;

		DEBUG_TRACE("%px: Re-generate: %d\n", ci, ca_type);
		ecm_db_connection_regenerate(ci);

		cin = ecm_db_connection_by_classifier_type_assignment_get_and_ref_next(ci, ca_type);
		ecm_db_connection_by_classifier_type_assignment_deref(ci, ca_type);
		ci = cin;
	}
}
EXPORT_SYMBOL(ecm_db_connection_regenerate_by_assignment_type);
#endif

/*
 * ecm_db_connection_interfaces_get_and_ref()
 *	Return the interface heirarchy in the specified direction which this connection is established.
 *
 * 'interfaces' MUST be an array as large as ECM_DB_IFACE_HEIRARCHY_MAX.
 * Returns either ECM_DB_IFACE_HEIRARCHY_MAX if there are no interfaces / error.
 * Returns the index into the interfaces[] of the first interface (so "for (i = <ret val>, i < ECM_DB_IFACE_HEIRARCHY_MAX; ++i)" works)
 *
 * Each interface is referenced on return, be sure to release them individually or use ecm_db_connection_interfaces_deref() instead.
 */
int32_t ecm_db_connection_interfaces_get_and_ref(struct ecm_db_connection_instance *ci,
						 struct ecm_db_iface_instance *interfaces[],
						 ecm_db_obj_dir_t dir)
{
	int32_t n;
	int32_t i;
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed\n", ci);

	spin_lock_bh(&ecm_db_lock);
	n = ci->interface_first[dir];
	for (i = n; i < ECM_DB_IFACE_HEIRARCHY_MAX; ++i) {
		interfaces[i] = ci->interfaces[dir][i];
		_ecm_db_iface_ref(interfaces[i]);
	}
	spin_unlock_bh(&ecm_db_lock);
	return n;
}
EXPORT_SYMBOL(ecm_db_connection_interfaces_get_and_ref);

/*
 * ecm_db_connection_interfaces_deref()
 *	Release all interfaces in the given interfaces heirarchy array.
 *
 * 'first' is the number returned by one of the ecm_db_connection_xx_interfaces_get_and_ref().
 * You should NOT have released any references to any of the interfaces in the array youself, this releases them all.
 */
void ecm_db_connection_interfaces_deref(struct ecm_db_iface_instance *interfaces[], int32_t first)
{
	int32_t i;
	DEBUG_ASSERT((first >= 0) && (first <= ECM_DB_IFACE_HEIRARCHY_MAX), "Bad first: %d\n", first);

	for (i = first; i < ECM_DB_IFACE_HEIRARCHY_MAX; ++i) {
		ecm_db_iface_deref(interfaces[i]);
	}
}
EXPORT_SYMBOL(ecm_db_connection_interfaces_deref);

/*
 * ecm_db_connection_interfaces_reset()
 *	Reset the interfaces heirarchy in the specified direction with a new set of interfaces
 *
 * NOTE: This will mark the list as set even if you specify no list as a replacement.
 * This is deliberate - it's stating that there is no list :-)
 */
void ecm_db_connection_interfaces_reset(struct ecm_db_connection_instance *ci,
					struct ecm_db_iface_instance *interfaces[],
					int32_t new_first, ecm_db_obj_dir_t dir)
{
	struct ecm_db_iface_instance *old[ECM_DB_IFACE_HEIRARCHY_MAX];
	int32_t old_first;
	int32_t i;
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed\n", ci);

	/*
	 * Iterate the from interface list, removing the old and adding in the new
	 */
	spin_lock_bh(&ecm_db_lock);
	for (i = 0; i < ECM_DB_IFACE_HEIRARCHY_MAX; ++i) {
		/*
		 * Put any previous interface into the old list
		 */
		old[i] = ci->interfaces[dir][i];
		ci->interfaces[dir][i] = NULL;
		if (i < new_first) {
			continue;
		}
		ci->interfaces[dir][i] = interfaces[i];
		_ecm_db_iface_ref(ci->interfaces[dir][i]);
	}

	/*
	 * Get old first and update to new first
	 */
	old_first = ci->interface_first[dir];
	ci->interface_first[dir] = new_first;
	ci->interface_set[dir] = true;
	spin_unlock_bh(&ecm_db_lock);

	/*
	 * Release old
	 */
	ecm_db_connection_interfaces_deref(old, old_first);
}
EXPORT_SYMBOL(ecm_db_connection_interfaces_reset);

/*
 * ecm_db_connection_interfaces_get_count()
 *	Return the number of interfaces in the list
 */
int32_t ecm_db_connection_interfaces_get_count(struct ecm_db_connection_instance *ci, ecm_db_obj_dir_t dir)
{
	int32_t first;
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed\n", ci);
	spin_lock_bh(&ecm_db_lock);
	first = ci->interface_first[dir];
	spin_unlock_bh(&ecm_db_lock);
	return ECM_DB_IFACE_HEIRARCHY_MAX - first;
}
EXPORT_SYMBOL(ecm_db_connection_interfaces_get_count);

/*
 * ecm_db_connection_interfaces_set_check()
 *	Returns true if the interface list has been set - even if set to an empty list!
 */
bool ecm_db_connection_interfaces_set_check(struct ecm_db_connection_instance *ci, ecm_db_obj_dir_t dir)
{
	bool set;

	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed\n", ci);
	spin_lock_bh(&ecm_db_lock);
	set = ci->interface_set[dir];
	spin_unlock_bh(&ecm_db_lock);
	return set;
}
EXPORT_SYMBOL(ecm_db_connection_interfaces_set_check);

/*
 * ecm_db_connection_interfaces_clear()
 *	Clear down the interfaces list, marking the list as not set
 */
void ecm_db_connection_interfaces_clear(struct ecm_db_connection_instance *ci, ecm_db_obj_dir_t dir)
{
	struct ecm_db_iface_instance *discard[ECM_DB_IFACE_HEIRARCHY_MAX];
	int32_t discard_first;
	int32_t i;

	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed\n", ci);

	spin_lock_bh(&ecm_db_lock);
	for (i = ci->interface_first[dir]; i < ECM_DB_IFACE_HEIRARCHY_MAX; ++i) {
		discard[i] = ci->interfaces[dir][i];
	}

	discard_first = ci->interface_first[dir];
	ci->interface_set[dir] = false;
	ci->interface_first[dir] = ECM_DB_IFACE_HEIRARCHY_MAX;
	spin_unlock_bh(&ecm_db_lock);

	/*
	 * Release previous
	 */
	ecm_db_connection_interfaces_deref(discard, discard_first);
}
EXPORT_SYMBOL(ecm_db_connection_interfaces_clear);

/*
 * ecm_db_connection_add()
 *	Add the connection into the database.
 *
 * NOTE: The parameters are DIRECTIONAL in terms of which mapping established the connection.
 * NOTE: Dir confirms if this is an egressing or ingressing connection.
 * This applies to firewalling front ends mostly. If INGRESS then mapping[ECM_DB_OBJ_DIR_FROM] is the WAN side.
 * If EGRESS then mapping[ECM_DB_OBJ_DIR_TO] is the WAN side.
 */
void ecm_db_connection_add(struct ecm_db_connection_instance *ci,
							struct ecm_db_mapping_instance *mapping[],
							struct ecm_db_node_instance *node[],
							int ip_version,
							int protocol, ecm_db_direction_t ecm_dir,
							ecm_db_connection_final_callback_t final,
							ecm_db_timer_group_t tg, bool is_routed,
							void *arg)
{
	ecm_db_connection_hash_t hash_index;
	ecm_db_connection_serial_hash_t serial_hash_index;
	struct ecm_db_listener_instance *li;
	int dir;
#ifdef ECM_DB_XREF_ENABLE
	struct ecm_db_iface_instance *iface[ECM_DB_OBJ_DIR_MAX];
#endif

	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed\n", ci);
	for (dir = 0; dir < ECM_DB_OBJ_DIR_MAX; dir++) {
		DEBUG_CHECK_MAGIC(mapping[dir], ECM_DB_MAPPING_INSTANCE_MAGIC, "%px: %s mapping magic failed \n", mapping[dir], ecm_db_obj_dir_strings[dir]);
		DEBUG_CHECK_MAGIC(node[dir], ECM_DB_NODE_INSTANCE_MAGIC, "%px: %s node magic failed\n", node[dir], ecm_db_obj_dir_strings[dir]);
	}
	DEBUG_ASSERT((protocol >= 0) && (protocol <= 255), "%px: invalid protocol number %d\n", ci, protocol);

	spin_lock_bh(&ecm_db_lock);
	DEBUG_ASSERT(!(ci->flags & ECM_DB_CONNECTION_FLAGS_INSERTED), "%px: inserted\n", ci);
	spin_unlock_bh(&ecm_db_lock);

	/*
	 * Record owner arg and callbacks
	 */
	ci->final = final;
	ci->arg = arg;

#ifdef ECM_MULTICAST_ENABLE
	ci->ti = NULL;
#endif

	/*
	 * Ensure default classifier has been assigned this is a must to ensure minimum level of classification
	 */
	DEBUG_ASSERT(ci->assignments_by_type[ECM_CLASSIFIER_TYPE_DEFAULT], "%px: No default classifier assigned\n", ci);

	/*
	 * Connection takes references to the mappings and nodes
	 */
	for (dir = 0; dir < ECM_DB_OBJ_DIR_MAX; dir++) {
		ecm_db_mapping_ref(mapping[dir]);
		ci->mapping[dir] = mapping[dir];

		ecm_db_node_ref(node[dir]);
		ci->node[dir] = node[dir];
	}

	/*
	 * Set the protocol and routed flag
	 */
	ci->ip_version = ip_version;
	ci->protocol = protocol;
	ci->is_routed = is_routed;

	/*
	 * Set direction of connection
	 */
	ci->direction = ecm_dir;

	/*
	 * Identify which hash chain this connection will go into
	 */
	hash_index = ecm_db_connection_generate_hash_index(mapping[ECM_DB_OBJ_DIR_FROM]->host->address,
							   mapping[ECM_DB_OBJ_DIR_FROM]->port,
							   mapping[ECM_DB_OBJ_DIR_TO]->host->address,
							   mapping[ECM_DB_OBJ_DIR_TO]->port, protocol);
	ci->hash_index = hash_index;

	/*
	 * Identify which serial hash chain this connection will go into
	 */
	serial_hash_index = ecm_db_connection_generate_serial_hash_index(ci->serial);
	ci->serial_hash_index = serial_hash_index;

	/*
	 * Now we need to lock
	 */
	spin_lock_bh(&ecm_db_lock);

	/*
	 * Increment protocol counter stats
	 */
	ecm_db_connection_count_by_protocol[protocol]++;
	DEBUG_ASSERT(ecm_db_connection_count_by_protocol[protocol] > 0, "%px: Invalid protocol count %d\n", ci, ecm_db_connection_count_by_protocol[protocol]);

	/*
	 * Set time
	 */
	ci->time_added = ecm_db_time;

	/*
	 * Add connection into the global list
	 */
	ci->prev = NULL;
	ci->next = ecm_db_connections;
	if (ecm_db_connections) {
		ecm_db_connections->prev = ci;
	}
	ecm_db_connections = ci;

	/*
	 * Add this connection into the connections hash table
	 */
	ci->flags |= ECM_DB_CONNECTION_FLAGS_INSERTED;

	/*
	 * Insert connection into the connections hash table
	 */
	ci->hash_prev = NULL;
	ci->hash_next = ecm_db_connection_table[hash_index];
	if (ecm_db_connection_table[hash_index]) {
		ecm_db_connection_table[hash_index]->hash_prev = ci;
	}
	ecm_db_connection_table[hash_index] = ci;
	ecm_db_connection_table_lengths[hash_index]++;
	DEBUG_ASSERT(ecm_db_connection_table_lengths[hash_index] > 0, "%px: invalid table len %d\n", ci, ecm_db_connection_table_lengths[hash_index]);

	/*
	 * Insert connection into the connections serial hash table
	 */
	ci->serial_hash_prev = NULL;
	ci->serial_hash_next = ecm_db_connection_serial_table[serial_hash_index];
	if (ecm_db_connection_serial_table[serial_hash_index]) {
		ecm_db_connection_serial_table[serial_hash_index]->serial_hash_prev = ci;
	}
	ecm_db_connection_serial_table[serial_hash_index] = ci;
	ecm_db_connection_serial_table_lengths[serial_hash_index]++;
	DEBUG_ASSERT(ecm_db_connection_serial_table_lengths[serial_hash_index] > 0, "%px: invalid table len %d\n", ci, ecm_db_connection_serial_table_lengths[serial_hash_index]);

#ifdef ECM_DB_XREF_ENABLE
	/*
	 * Add this connection into the nodes.
	 */
	for (dir = 0; dir < ECM_DB_OBJ_DIR_MAX; dir++) {
		ci->node_prev[dir] = NULL;
		ci->node_next[dir] = node[dir]->connections[dir];
		if (node[dir]->connections[dir]) {
			node[dir]->connections[dir]->node_prev[dir] = ci;
		}
		node[dir]->connections[dir] = ci;
		node[dir]->connections_count[dir]++;
		DEBUG_ASSERT(node[dir]->connections_count[dir] > 0, "%px: invalid count for %s node connections\n", ci, ecm_db_obj_dir_strings[dir]);
	}

	/*
	 * Add this connection into the mappings
	 */
	for (dir = 0; dir < ECM_DB_OBJ_DIR_MAX; dir++) {
		ci->mapping_prev[dir] = NULL;
		ci->mapping_next[dir] = mapping[dir]->connections[dir];
		if (mapping[dir]->connections[dir]) {
			mapping[dir]->connections[dir]->mapping_prev[dir] = ci;
		}
		mapping[dir]->connections[dir] = ci;
	}

	/*
	 * Add this connection into the ifaces list of connections
	 * NOTE: There is no need to ref the iface because it will exist for as long as this connection exists
	 * due to the heirarchy of dependencies being kept by the database.
	 */
	for (dir = 0; dir < ECM_DB_OBJ_DIR_MAX; dir++) {
		iface[dir] = node[dir]->iface;
		ci->iface_prev[dir] = NULL;
		ci->iface_next[dir] = iface[dir]->connections[dir];
		if (iface[dir]->connections[dir]) {
			iface[dir]->connections[dir]->iface_prev[dir] = ci;
		}
		iface[dir]->connections[dir] = ci;
	}
#endif

	/*
	 * NOTE: The interface heirarchy lists are deliberately left empty - these are completed
	 * by the front end if it is appropriate to do so.
	 */

	/*
	 * Update the counters in the mapping
	 */
	if (protocol == IPPROTO_UDP) {
		for (dir = 0; dir < ECM_DB_OBJ_DIR_MAX; dir++) {
			mapping[dir]->udp_count[dir]++;
		}
	} else if (protocol == IPPROTO_TCP) {
		for (dir = 0; dir < ECM_DB_OBJ_DIR_MAX; dir++) {
			mapping[dir]->tcp_count[dir]++;
		}
	}

	for (dir = 0; dir < ECM_DB_OBJ_DIR_MAX; dir++) {
		mapping[dir]->conn_count[dir]++;
	}

	/*
	 * Set the generation number to match global
	 */
	ci->generation = ecm_db_connection_generation;

	spin_unlock_bh(&ecm_db_lock);

	/*
	 * Throw add event to the listeners
	 */
	DEBUG_TRACE("%px: Throw connection added event\n", ci);
	li = ecm_db_listeners_get_and_ref_first();
	while (li) {
		struct ecm_db_listener_instance *lin;
		if (li->connection_added) {
			li->connection_added(li->arg, ci);
		}

		/*
		 * Get next listener
		 */
		lin = ecm_db_listener_get_and_ref_next(li);
		ecm_db_listener_deref(li);
		li = lin;
	}

	/*
	 * Set timer group. 'ref' the connection to ensure it persists for the timer.
	 */
	ecm_db_connection_ref(ci);
	ecm_db_timer_group_entry_set(&ci->defunct_timer, tg);
}
EXPORT_SYMBOL(ecm_db_connection_add);

/*
 * ecm_db_connection_heirarchy_state_get()
 *	Output state for an interface heirarchy list.
 */
int ecm_db_connection_heirarchy_state_get(struct ecm_state_file_instance *sfi, struct ecm_db_iface_instance *interfaces[], int32_t first_interface)
{
	int result;
	int count;
	int i;
	int j;

	count = ECM_DB_IFACE_HEIRARCHY_MAX - first_interface;
	if ((result = ecm_state_write(sfi, "interface_count", "%d", count))) {
		return result;
	}

	/*
	 * Iterate the interface heirarchy list and output the information
	 */
	for (i = first_interface, j = 0; i < ECM_DB_IFACE_HEIRARCHY_MAX; ++i, ++j) {
		struct ecm_db_iface_instance *ii = interfaces[i];
		DEBUG_TRACE("Output interface @ %d: %px\n", i, ii);

		if ((result = ecm_state_prefix_index_add(sfi, j))) {
			return result;
		}
		result = ii->state_get(ii, sfi);
		if (result) {
			return result;
		}
		if ((result = ecm_state_prefix_remove(sfi))) {
			return result;
		}
	}

	return 0;
}

/*
 * ecm_db_connection_state_get()
 *	Prepare a connection message
 */
int ecm_db_connection_state_get(struct ecm_state_file_instance *sfi, struct ecm_db_connection_instance *ci)
{
	int result;
	long int expires_in;
	int sport;
	int sport_nat;
	char snode_address[ECM_MAC_ADDR_STR_BUFF_SIZE];
	char snode_address_nat[ECM_MAC_ADDR_STR_BUFF_SIZE];
	char sip_address[ECM_IP_ADDR_STR_BUFF_SIZE];
	char sip_address_nat[ECM_IP_ADDR_STR_BUFF_SIZE];
	char dnode_address[ECM_MAC_ADDR_STR_BUFF_SIZE];
	char dnode_address_nat[ECM_MAC_ADDR_STR_BUFF_SIZE];
	int dport;
	int dport_nat;
	char dip_address[ECM_IP_ADDR_STR_BUFF_SIZE];
	char dip_address_nat[ECM_IP_ADDR_STR_BUFF_SIZE];
	ecm_db_direction_t direction;
	int ip_version;
	int protocol;
	bool is_routed;
	uint32_t regen_success;
	uint32_t regen_fail;
	uint16_t regen_required;
	uint16_t regen_occurances;
	bool regen_in_progress;
	uint16_t generation;
	uint16_t global_generation;
	uint32_t time_added;
	uint32_t serial;
	uint64_t from_data_total;
	uint64_t to_data_total;
	uint64_t from_packet_total;
	uint64_t to_packet_total;
	uint64_t from_data_total_dropped;
	uint64_t to_data_total_dropped;
	uint64_t from_packet_total_dropped;
	uint64_t to_packet_total_dropped;
	struct ecm_db_host_instance *hi;
	struct ecm_db_node_instance *ni;
	int aci_index;
	int aci_count;
	ip_addr_t __attribute__((unused)) group_ip;
	struct ecm_front_end_connection_instance *feci;
	struct ecm_classifier_instance *assignments[ECM_CLASSIFIER_TYPES];
	int32_t first_interface;
	struct ecm_db_iface_instance *interfaces[ECM_DB_IFACE_HEIRARCHY_MAX];

	DEBUG_TRACE("Prep conn msg for %px\n", ci);

	/*
	 * Identify expiration
	 */
	spin_lock_bh(&ecm_db_lock);
	if (ci->defunct_timer.group == ECM_DB_TIMER_GROUPS_MAX) {
		expires_in = -1;
	} else {
		expires_in = (long int)(ci->defunct_timer.timeout - ecm_db_time);
		if (expires_in <= 0) {
			expires_in = 0;
		}
	}

	regen_success = ci->regen_success;
	regen_fail = ci->regen_fail;
	regen_required = ci->regen_required;
	regen_occurances = ci->regen_occurances;
	regen_in_progress = ci->regen_in_progress;
	generation = ci->generation;
	global_generation = ecm_db_connection_generation;

	spin_unlock_bh(&ecm_db_lock);

	/*
	 * Extract information from the connection for inclusion into the message
	 */
	sport = ci->mapping[ECM_DB_OBJ_DIR_FROM]->port;
	sport_nat = ci->mapping[ECM_DB_OBJ_DIR_FROM_NAT]->port;
	dport = ci->mapping[ECM_DB_OBJ_DIR_TO]->port;
	dport_nat = ci->mapping[ECM_DB_OBJ_DIR_TO_NAT]->port;

	hi = ci->mapping[ECM_DB_OBJ_DIR_TO]->host;
	ecm_ip_addr_to_string(dip_address, hi->address);
	ni = ci->node[ECM_DB_OBJ_DIR_TO];
	snprintf(dnode_address, sizeof(dnode_address), "%pM", ni->address);
	hi = ci->mapping[ECM_DB_OBJ_DIR_TO_NAT]->host;
	ecm_ip_addr_to_string(dip_address_nat, hi->address);

	hi = ci->mapping[ECM_DB_OBJ_DIR_FROM]->host;
	ecm_ip_addr_to_string(sip_address, hi->address);
	ni = ci->node[ECM_DB_OBJ_DIR_FROM];
	snprintf(snode_address, sizeof(snode_address), "%pM", ni->address);
	hi = ci->mapping[ECM_DB_OBJ_DIR_FROM_NAT]->host;
	ecm_ip_addr_to_string(sip_address_nat, hi->address);

	ni = ci->node[ECM_DB_OBJ_DIR_TO_NAT];
	snprintf(dnode_address_nat, sizeof(dnode_address_nat), "%pM", ni->address);

	ni = ci->node[ECM_DB_OBJ_DIR_FROM_NAT];
	snprintf(snode_address_nat, sizeof(snode_address_nat), "%pM", ni->address);

	direction = ci->direction;
	ip_version = ci->ip_version;
	protocol = ci->protocol;
	is_routed = ci->is_routed;
	time_added = ci->time_added;
	serial = ci->serial;
	ecm_db_connection_data_stats_get(ci, &from_data_total, &to_data_total,
			&from_packet_total, &to_packet_total,
			&from_data_total_dropped, &to_data_total_dropped,
			&from_packet_total_dropped, &to_packet_total_dropped);

	if ((result = ecm_state_prefix_add(sfi, "conn"))) {
		return result;
	}
	if ((result = ecm_state_prefix_index_add(sfi, serial))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "serial", "%u", serial))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "sip_address", "%s", sip_address))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "sip_address_nat", "%s", sip_address_nat))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "sport", "%d", sport))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "sport_nat", "%d", sport_nat))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "snode_address", "%s", snode_address))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "snode_address_nat", "%s", snode_address_nat))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "dip_address", "%s", dip_address))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "dip_address_nat", "%s", dip_address_nat))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "dport", "%d", dport))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "dport_nat", "%d", dport_nat))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "dnode_address", "%s", dnode_address))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "dnode_address_nat", "%s", dnode_address_nat))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "ip_version", "%d", ip_version))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "protocol", "%d", protocol))) {
		return result;
	}

	if (ecm_db_connection_is_pppoe_bridged_get(ci)) {
		/*
		 * PPPoE session ID is set in sport and dport.
		 */
		if ((result = ecm_state_write(sfi, "pppoe_session_id", "%u", sport))) {
			return result;
		}
	}

	if ((result = ecm_state_write(sfi, "is_routed", "%d", is_routed))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "expires", "%ld", expires_in))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "direction", "%d", direction))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "time_added", "%u", time_added))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "regen_success", "%u", regen_success))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "regen_fail", "%u", regen_fail))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "regen_required", "%u", regen_required))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "regen_occurances", "%u", regen_occurances))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "regen_in_progress", "%u", regen_in_progress))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "generation", "%u/%u", generation, global_generation))) {
		return result;
	}

	/*
	 * NOTE: These advanced stats are not conditional compiled.
	 * Connections always contain these stats
	 */
	if ((result = ecm_db_adv_stats_state_write(sfi, from_data_total, to_data_total,
			from_packet_total, to_packet_total, from_data_total_dropped,
			to_data_total_dropped, from_packet_total_dropped,
			to_packet_total_dropped))) {
		return result;
	}

	if ((result = ecm_state_prefix_add(sfi, "from_interfaces"))) {
		return result;
	}
	first_interface = ecm_db_connection_interfaces_get_and_ref(ci, interfaces, ECM_DB_OBJ_DIR_FROM);
	result = ecm_db_connection_heirarchy_state_get(sfi, interfaces, first_interface);
	ecm_db_connection_interfaces_deref(interfaces, first_interface);
	if (result) {
		return result;
	}
	if ((result = ecm_state_prefix_remove(sfi))) {
		return result;
	}

#ifdef ECM_MULTICAST_ENABLE
	ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_TO, group_ip);
	if (ecm_ip_addr_is_multicast(group_ip)) {
		if ((result = ecm_state_prefix_add(sfi, "to_mc_interfaces"))) {
			return result;
		}

		if ((result = ecm_db_multicast_to_interfaces_xml_state_get(ci, sfi))) {
			return result;
		}

		if ((result = ecm_state_prefix_remove(sfi))) {
			return result;
		}
	}
	else {
		if ((result = ecm_state_prefix_add(sfi, "to_interfaces"))) {
			return result;
		}

		first_interface = ecm_db_connection_interfaces_get_and_ref(ci, interfaces, ECM_DB_OBJ_DIR_TO);
		result = ecm_db_connection_heirarchy_state_get(sfi, interfaces, first_interface);
		ecm_db_connection_interfaces_deref(interfaces, first_interface);
		if (result) {
			return result;
		}

		if ((result = ecm_state_prefix_remove(sfi))) {
			return result;
		}
	}
#else
	if ((result = ecm_state_prefix_add(sfi, "to_interfaces"))) {
		return result;
	}
	first_interface = ecm_db_connection_interfaces_get_and_ref(ci, interfaces, ECM_DB_OBJ_DIR_TO);
	result = ecm_db_connection_heirarchy_state_get(sfi, interfaces, first_interface);
	ecm_db_connection_interfaces_deref(interfaces, first_interface);
	if (result) {
		return result;
	}
	if ((result = ecm_state_prefix_remove(sfi))) {
		return result;
	}
#endif

	if ((result = ecm_state_prefix_add(sfi, "from_nat_interfaces"))) {
		return result;
	}
	first_interface = ecm_db_connection_interfaces_get_and_ref(ci, interfaces, ECM_DB_OBJ_DIR_FROM_NAT);
	result = ecm_db_connection_heirarchy_state_get(sfi, interfaces, first_interface);
	ecm_db_connection_interfaces_deref(interfaces, first_interface);
	if (result) {
		return result;
	}
	if ((result = ecm_state_prefix_remove(sfi))) {
		return result;
	}

	if ((result = ecm_state_prefix_add(sfi, "to_nat_interfaces"))) {
		return result;
	}
	first_interface = ecm_db_connection_interfaces_get_and_ref(ci, interfaces, ECM_DB_OBJ_DIR_TO_NAT);
	result = ecm_db_connection_heirarchy_state_get(sfi, interfaces, first_interface);
	ecm_db_connection_interfaces_deref(interfaces, first_interface);
	if (result) {
		return result;
	}
	if ((result = ecm_state_prefix_remove(sfi))) {
		return result;
	}

	/*
	 * Output front end state
	 */
	feci = ecm_db_connection_front_end_get_and_ref(ci);
	result = feci->state_get(feci, sfi);
	ecm_front_end_connection_deref(feci);
	if (result) {
		return result;
	}

	if ((result = ecm_state_prefix_add(sfi, "classifiers"))) {
		return result;
	}

	/*
	 * Grab references to the assigned classifiers so we can produce state for them
	 */
	aci_count = ecm_db_connection_classifier_assignments_get_and_ref(ci, assignments);

	/*
	 * Iterate the assigned classifiers and provide a state record for each
	 */
	for (aci_index = 0; aci_index < aci_count; ++aci_index) {
		struct ecm_classifier_instance *aci;

		aci = assignments[aci_index];
		result = aci->state_get(aci, sfi);
		if (result) {
			ecm_db_connection_assignments_release(aci_count, assignments);
			return result;
		}
	}

	ecm_db_connection_assignments_release(aci_count, assignments);

	if ((result = ecm_state_prefix_remove(sfi))) {
		return result;
	}

	if ((result = ecm_state_prefix_remove(sfi))) {
		return result;
	}

	return ecm_state_prefix_remove(sfi);
}
EXPORT_SYMBOL(ecm_db_connection_state_get);

/*
 * ecmECM_DB_CONNECTION_HASH_SLOTS_lengths_get()
 *	Return hash table length
 */
int ecm_db_connection_hash_table_lengths_get(int index)
{
	int length;

	DEBUG_ASSERT((index >= 0) && (index < ECM_DB_CONNECTION_HASH_SLOTS), "Bad protocol: %d\n", index);
	spin_lock_bh(&ecm_db_lock);
	length = ecm_db_connection_table_lengths[index];
	spin_unlock_bh(&ecm_db_lock);
	return length;
}
EXPORT_SYMBOL(ecm_db_connection_hash_table_lengths_get);

/*
 * ecm_db_connection_hash_index_get_next()
 * Given a hash index, return the next one OR return -1 for no more hash indicies to return.
 */
int ecm_db_connection_hash_index_get_next(int index)
{
	index++;
	if (index >= ECM_DB_CONNECTION_SERIAL_HASH_SLOTS) {
		return -1;
	}
	return index;
}
EXPORT_SYMBOL(ecm_db_connection_hash_index_get_next);

/*
 * ecm_db_connection_hash_index_get_first()
 * Return first hash index
 */
int ecm_db_connection_hash_index_get_first(void)
{
	return 0;
}
EXPORT_SYMBOL(ecm_db_connection_hash_index_get_first);

#ifdef ECM_STATE_OUTPUT_ENABLE
/*
 * ecm_db_protocol_get_next()
 * Given a number, return the next one OR return -1 for no more protocol numbers to return.
 */
int ecm_db_protocol_get_next(int protocol)
{
	protocol++;
	if (protocol >= ECM_DB_PROTOCOL_COUNT) {
		return -1;
	}
	return protocol;
}
EXPORT_SYMBOL(ecm_db_protocol_get_next);

/*
 * ecm_db_protocol_get_first()
 * Return first protocol number
 */
int ecm_db_protocol_get_first(void)
{
	return 0;
}
EXPORT_SYMBOL(ecm_db_protocol_get_first);
#endif

/*
 * ecm_db_connection_alloc()
 *	Allocate a connection instance
 */
struct ecm_db_connection_instance *ecm_db_connection_alloc(void)
{
	struct ecm_db_connection_instance *ci;
	int __attribute__((unused)) i;
	unsigned int conn_count;

	/*
	 * If we have exceeded the conntrack connection limit then do not allocate new instance.
	 */
	conn_count = (unsigned int)ecm_db_connection_count_get();
	if (conn_count >= nf_conntrack_max) {
		DEBUG_WARN("ECM Connection count limit reached: db: %u, ct: %u\n", conn_count, nf_conntrack_max);
		return NULL;
	}

	/*
	 * Allocate the connection
	 */
	ci = (struct ecm_db_connection_instance *)kzalloc(sizeof(struct ecm_db_connection_instance), GFP_ATOMIC | __GFP_NOWARN);
	if (!ci) {
		DEBUG_WARN("Connection alloc failed\n");
		return NULL;
	}

	/*
	 * Initialise the defunct timer entry
	 */
	ecm_db_timer_group_entry_init(&ci->defunct_timer, ecm_db_connection_defunct_callback, ci);

	/*
	 * Refs is 1 for the creator of the connection
	 */
	ci->refs = 1;
	DEBUG_SET_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC);

	/*
	 * Initialise the interfaces from/to lists.
	 * Interfaces are added from end of array.
	 */
	ci->interface_first[ECM_DB_OBJ_DIR_FROM] = ECM_DB_IFACE_HEIRARCHY_MAX;
	ci->interface_first[ECM_DB_OBJ_DIR_TO] = ECM_DB_IFACE_HEIRARCHY_MAX;
	ci->interface_first[ECM_DB_OBJ_DIR_FROM_NAT] = ECM_DB_IFACE_HEIRARCHY_MAX;
	ci->interface_first[ECM_DB_OBJ_DIR_TO_NAT] = ECM_DB_IFACE_HEIRARCHY_MAX;

#ifdef ECM_MULTICAST_ENABLE
	for (i = 0; i < ECM_DB_MULTICAST_IF_MAX; ++i) {
		ci->to_mcast_interface_first[i] = ECM_DB_IFACE_HEIRARCHY_MAX;
	}
#endif
	/*
	 * If the master thread is terminating then we cannot create new instances
	 */
	spin_lock_bh(&ecm_db_lock);
	if (ecm_db_terminate_pending) {
		spin_unlock_bh(&ecm_db_lock);
		DEBUG_WARN("Thread terminating\n");
		kfree(ci);
		return NULL;
	}

	/*
	 * Assign runtime unique serial
	 */
	ci->serial = ecm_db_connection_serial++;

	ecm_db_connection_count++;
	DEBUG_ASSERT(ecm_db_connection_count > 0, "%px: connection count wrap\n", ci);
	spin_unlock_bh(&ecm_db_lock);

	DEBUG_TRACE("Connection created %px\n", ci);
	return ci;
}
EXPORT_SYMBOL(ecm_db_connection_alloc);

/*
 * ecm_db_connection_ipv6_from_ct_get_and_ref()
 *	Return, if any, a connection given a ct
 */
struct ecm_db_connection_instance *ecm_db_connection_ipv6_from_ct_get_and_ref(struct nf_conn *ct)
{
	struct nf_conntrack_tuple orig_tuple;
	struct nf_conntrack_tuple reply_tuple;
	ip_addr_t host1_addr;
	ip_addr_t host2_addr;
	int host1_port;
	int host2_port;
	int protocol;

	/*
	 * Look up the associated connection for this conntrack connection
	 */
	orig_tuple = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple;
	reply_tuple = ct->tuplehash[IP_CT_DIR_REPLY].tuple;
	ECM_NIN6_ADDR_TO_IP_ADDR(host1_addr, orig_tuple.src.u3.in6);
	ECM_NIN6_ADDR_TO_IP_ADDR(host2_addr, reply_tuple.src.u3.in6);
	protocol = orig_tuple.dst.protonum;
	switch (protocol) {
	case IPPROTO_TCP:
		host1_port = ntohs(orig_tuple.src.u.tcp.port);
		host2_port = ntohs(reply_tuple.src.u.tcp.port);
		break;
	case IPPROTO_UDP:
		host1_port = ntohs(orig_tuple.src.u.udp.port);
		host2_port = ntohs(reply_tuple.src.u.udp.port);
		break;
	case IPPROTO_IPIP:
	case IPPROTO_GRE:
		host1_port = 0;
		host2_port = 0;
		break;
	default:
		host1_port = -protocol;
		host2_port = -protocol;
	}

	DEBUG_TRACE("%px: lookup src: " ECM_IP_ADDR_OCTAL_FMT ":%d, "
		    "dest: " ECM_IP_ADDR_OCTAL_FMT ":%d, "
		    "protocol %d\n",
		    ct,
		    ECM_IP_ADDR_TO_OCTAL(host1_addr),
		    host1_port,
		    ECM_IP_ADDR_TO_OCTAL(host2_addr),
		    host2_port,
		    protocol);

	return ecm_db_connection_find_and_ref(host1_addr,
					      host2_addr,
					      protocol,
					      host1_port,
					      host2_port);
}

/*
 * ecm_db_connection_ipv4_from_ct_get_and_ref()
 *	Return, if any, a connection given a ct
 */
struct ecm_db_connection_instance *ecm_db_connection_ipv4_from_ct_get_and_ref(struct nf_conn *ct)
{
	struct nf_conntrack_tuple orig_tuple;
	struct nf_conntrack_tuple reply_tuple;
	ip_addr_t host1_addr;
	ip_addr_t host2_addr;
	int host1_port;
	int host2_port;
	int protocol;

	/*
	 * Look up the associated connection for this conntrack connection
	 */
	orig_tuple = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple;
	reply_tuple = ct->tuplehash[IP_CT_DIR_REPLY].tuple;
	ECM_NIN4_ADDR_TO_IP_ADDR(host1_addr, orig_tuple.src.u3.ip);
	ECM_NIN4_ADDR_TO_IP_ADDR(host2_addr, reply_tuple.src.u3.ip);
	protocol = orig_tuple.dst.protonum;
	switch (protocol) {
	case IPPROTO_TCP:
		host1_port = ntohs(orig_tuple.src.u.tcp.port);
		host2_port = ntohs(reply_tuple.src.u.tcp.port);
		break;
	case IPPROTO_UDP:
		host1_port = ntohs(orig_tuple.src.u.udp.port);
		host2_port = ntohs(reply_tuple.src.u.udp.port);
		break;
	case IPPROTO_IPV6:
	case IPPROTO_ESP:
	case IPPROTO_GRE:
		host1_port = 0;
		host2_port = 0;
		break;
	default:
		host1_port = -protocol;
		host2_port = -protocol;
	}

	DEBUG_TRACE("%px: lookup src: " ECM_IP_ADDR_DOT_FMT ":%d, "
		    "dest: " ECM_IP_ADDR_DOT_FMT ":%d, "
		    "protocol %d\n",
		    ct,
		    ECM_IP_ADDR_TO_DOT(host1_addr),
		    host1_port,
		    ECM_IP_ADDR_TO_DOT(host2_addr),
		    host2_port,
		    protocol);

	return ecm_db_connection_find_and_ref(host1_addr,
					      host2_addr,
					      protocol,
					      host1_port,
					      host2_port);
}

#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
/*
 * ecm_db_connection_from_ovs_flow_get_and_ref()
 *	Look-up a connection based on OVS flow 5-tuple information.
 */
struct ecm_db_connection_instance *ecm_db_connection_from_ovs_flow_get_and_ref(struct ovsmgr_dp_flow *flow)
{
	ip_addr_t src_addr;
	ip_addr_t dst_addr;
	int src_port;
	int dst_port;
	int protocol;

	/*
	 * Look up the associated connection for this OVS flow
	 */
	protocol = flow->tuple.protocol;
	src_port = ntohs(flow->tuple.src_port);
	dst_port = ntohs(flow->tuple.dst_port);

	if (flow->tuple.ip_version == 4) {
		ECM_NIN4_ADDR_TO_IP_ADDR(src_addr, flow->tuple.ipv4.src);
		ECM_NIN4_ADDR_TO_IP_ADDR(dst_addr, flow->tuple.ipv4.dst);
		DEBUG_TRACE("%px: OVS IPv4 flow lookup src: " ECM_IP_ADDR_DOT_FMT ":%d, "
			    "dest: " ECM_IP_ADDR_DOT_FMT ":%d, "
			    "protocol %d\n",
			    flow,
			    ECM_IP_ADDR_TO_DOT(src_addr),
			    src_port,
			    ECM_IP_ADDR_TO_DOT(dst_addr),
			    dst_port,
			    protocol);
	} else if (flow->tuple.ip_version == 6) {
		ECM_NIN6_ADDR_TO_IP_ADDR(src_addr, flow->tuple.ipv6.src);
		ECM_NIN6_ADDR_TO_IP_ADDR(dst_addr, flow->tuple.ipv6.dst);
		DEBUG_TRACE("%px: OVS IPv6 flow lookup src: " ECM_IP_ADDR_OCTAL_FMT ":%d, "
			    "dest: " ECM_IP_ADDR_OCTAL_FMT ":%d, "
			    "protocol %d\n",
			    flow,
			    ECM_IP_ADDR_TO_OCTAL(src_addr),
			    src_port,
			    ECM_IP_ADDR_TO_OCTAL(dst_addr),
			    dst_port,
			    protocol);
	} else {
		DEBUG_WARN("%px: Invalid IP version: %d\n", flow, flow->tuple.ip_version);
		return NULL;
	}

	return ecm_db_connection_find_and_ref(src_addr,
					      dst_addr,
					      protocol,
					      src_port,
					      dst_port);
}
#endif

/*
 * ecm_db_connection_decel_v4()
 *	Decelerate IPv4 connection.
 *
 * Big endian parameters apart from protocol
 */
bool ecm_db_connection_decel_v4(__be32 src_ip, int src_port,
				__be32 dest_ip, int dest_port, int protocol)
{
	ip_addr_t ecm_src_ip;
	ip_addr_t ecm_dest_ip;
	struct ecm_db_connection_instance *ci;

	/*
	 * Look up ECM connection from the given tuple
	 */
	src_port = ntohs(src_port);
	dest_port = ntohs(dest_port);
	ECM_NIN4_ADDR_TO_IP_ADDR(ecm_src_ip, src_ip);
	ECM_NIN4_ADDR_TO_IP_ADDR(ecm_dest_ip, dest_ip);

	ci = ecm_db_connection_find_and_ref(ecm_src_ip, ecm_dest_ip, protocol, src_port, dest_port);
	if (!ci) {
		DEBUG_WARN("Decel v4 Connection lookup failed."
				" Received connection tuple information: \n"
				"Protocol: %d\n"
				"src: " ECM_IP_ADDR_DOT_FMT ":%d\n"
				"dest: " ECM_IP_ADDR_DOT_FMT ":%d\n",
				protocol,
				ECM_IP_ADDR_TO_DOT(ecm_src_ip), src_port,
				ECM_IP_ADDR_TO_DOT(ecm_dest_ip), dest_port);
		return false;
	}

	DEBUG_TRACE("Decel v4, connection tuple information: \n"
			"Protocol: %d\n"
			"src: " ECM_IP_ADDR_DOT_FMT ":%d\n"
			"dest: " ECM_IP_ADDR_DOT_FMT ":%d\n",
			protocol,
			ECM_IP_ADDR_TO_DOT(ecm_src_ip), src_port,
			ECM_IP_ADDR_TO_DOT(ecm_dest_ip), dest_port);

	/*
	 * Defunct the connection.
	 */
	ecm_db_connection_make_defunct(ci);
	ecm_db_connection_deref(ci);
	return true;
}

/*
 * ecm_db_connection_decel_v6()
 *	Decelerate IPv6 connection.
 *
 * Big endian parameters apart from protocol
 *
 * NOTE: If IPv6 is not supported in ECM this function must still exist as a stub to avoid compilation problems for registrants.
 */
bool ecm_db_connection_decel_v6(struct in6_addr *src_ip, int src_port,
				struct in6_addr *dest_ip, int dest_port, int protocol)
{
#ifdef ECM_IPV6_ENABLE
	struct in6_addr in6;
	ip_addr_t ecm_src_ip;
	ip_addr_t ecm_dest_ip;
	struct ecm_db_connection_instance *ci;

	/*
	 * Look up ECM connection from the given tuple
	 */
	src_port = ntohs(src_port);
	dest_port = ntohs(dest_port);
	in6 = *src_ip;
	ECM_NIN6_ADDR_TO_IP_ADDR(ecm_src_ip, in6);
	in6 = *dest_ip;
	ECM_NIN6_ADDR_TO_IP_ADDR(ecm_dest_ip, in6);

	ci = ecm_db_connection_find_and_ref(ecm_src_ip, ecm_dest_ip, protocol, src_port, dest_port);
	if (!ci) {
		DEBUG_WARN("Decel v6, Connection lookup failed."
				" Received connection tuple information: \n"
				"Protocol: %d\n"
				"src: " ECM_IP_ADDR_OCTAL_FMT ":%d\n"
				"dest: " ECM_IP_ADDR_OCTAL_FMT ":%d\n",
				protocol,
				ECM_IP_ADDR_TO_OCTAL(ecm_src_ip), src_port,
				ECM_IP_ADDR_TO_OCTAL(ecm_dest_ip), dest_port);
		return false;
	}


	DEBUG_TRACE("Decel v6, connection tuple information: \n"
			"Protocol: %d\n"
			"src: " ECM_IP_ADDR_OCTAL_FMT ":%d\n"
			"dest: " ECM_IP_ADDR_OCTAL_FMT ":%d\n",
			protocol,
			ECM_IP_ADDR_TO_OCTAL(ecm_src_ip), src_port,
			ECM_IP_ADDR_TO_OCTAL(ecm_dest_ip), dest_port);

	/*
	 * Defunct the connection.
	 */
	ecm_db_connection_make_defunct(ci);
	ecm_db_connection_deref(ci);
	return true;
#endif
	return false;
}

/*
 * ecm_db_front_end_instance_ref_and_set()
 *	Refs and sets the front end instance of connection.
 */
void ecm_db_front_end_instance_ref_and_set(struct ecm_db_connection_instance *ci, struct ecm_front_end_connection_instance *feci)
{
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed\n", ci);

	ecm_front_end_connection_ref(feci);
	ci->feci = feci;
}
EXPORT_SYMBOL(ecm_db_front_end_instance_ref_and_set);

/*
 * ecm_db_get_connection_counts_simple()
 *	Return total of connections for each simple protocol (tcp, udp, other).  Primarily for use by the luci-bwc service.
 */
static ssize_t ecm_db_get_connection_counts_simple(struct file *file,
						char __user *user_buf,
						size_t sz, loff_t *ppos)
{
	int tcp_count;
	int udp_count;
	int other_count;
	int total_count;
	int ret;
	char *buf;

	buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (!buf) {
		return -ENOMEM;
	}

	/*
	 * Get snapshot of the protocol counts
	 */
	spin_lock_bh(&ecm_db_lock);
	tcp_count = ecm_db_connection_count_by_protocol[IPPROTO_TCP];
	udp_count = ecm_db_connection_count_by_protocol[IPPROTO_UDP];
	total_count = ecm_db_connection_count;
	other_count = total_count - (tcp_count + udp_count);
	spin_unlock_bh(&ecm_db_lock);

	ret = snprintf(buf, (ssize_t)PAGE_SIZE, "tcp %d udp %d other %d total %d\n", tcp_count, udp_count, other_count, total_count);
	if (ret < 0) {
		kfree(buf);
		return -EFAULT;
	}

	ret = simple_read_from_buffer(user_buf, sz, ppos, buf, ret);
	kfree(buf);
	return ret;
}

/*
 * File operations for simple connection counts.
 */
static struct file_operations ecm_db_connection_count_simple_fops = {
	.read = ecm_db_get_connection_counts_simple,
};

/*
 * ecm_db_connection_init()
 */
bool ecm_db_connection_init(struct dentry *dentry)
{
	debugfs_create_u32("connection_count", S_IRUGO, dentry,
					(u32 *)&ecm_db_connection_count);

	if (!debugfs_create_file("connection_count_simple", S_IRUGO, dentry,
					NULL, &ecm_db_connection_count_simple_fops)) {
		DEBUG_ERROR("Failed to create ecm db connection count simple file in debugfs\n");
		return false;
	}

	ecm_db_connection_table = vzalloc(sizeof(struct ecm_db_connection_instance *) * ECM_DB_CONNECTION_HASH_SLOTS);
	if (!ecm_db_connection_table) {
		DEBUG_ERROR("Failed to allocate virtual memory for ecm_db_connection_table\n");
		return false;
	}

	ecm_db_connection_table_lengths = vzalloc(sizeof(int) * ECM_DB_CONNECTION_HASH_SLOTS);
	if (!ecm_db_connection_table_lengths) {
		DEBUG_ERROR("Failed to allocate virtual memory for ecm_db_connection_table_lengths\n");
		vfree(ecm_db_connection_table);
		return false;
	}

	ecm_db_connection_serial_table = vzalloc(sizeof(struct ecm_db_connection_instance *) * ECM_DB_CONNECTION_SERIAL_HASH_SLOTS);
	if (!ecm_db_connection_serial_table) {
		DEBUG_ERROR("Failed to allocate virtual memory for ecm_db_connection_serial_table\n");
		vfree(ecm_db_connection_table_lengths);
		vfree(ecm_db_connection_table);
		return false;
	}

	ecm_db_connection_serial_table_lengths = vzalloc(sizeof(int) * ECM_DB_CONNECTION_SERIAL_HASH_SLOTS);
	if (!ecm_db_connection_serial_table_lengths) {
		DEBUG_ERROR("Failed to allocate virtual memory for ecm_db_connection_serial_table_lengths\n");
		vfree(ecm_db_connection_serial_table);
		vfree(ecm_db_connection_table_lengths);
		vfree(ecm_db_connection_table);
		return false;
	}

	/*
	 * Reset connection by protocol counters
	 */
	memset(ecm_db_connection_count_by_protocol, 0, sizeof(ecm_db_connection_count_by_protocol));

#ifdef ECM_DB_CTA_TRACK_ENABLE
	/*
	 * Reset classifier type assignment lists
	 */
	memset(ecm_db_connection_classifier_type_assignments, 0, sizeof(ecm_db_connection_classifier_type_assignments));
#endif
	return true;
}

/*
 * ecm_db_connection_exit()
 */
void ecm_db_connection_exit(void)
{
	vfree(ecm_db_connection_serial_table_lengths);
	vfree(ecm_db_connection_serial_table);
	vfree(ecm_db_connection_table_lengths);
	vfree(ecm_db_connection_table);
}
