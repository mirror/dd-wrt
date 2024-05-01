/*
 **************************************************************************
 * Copyright (c) 2014-2018, 2020-2021, The Linux Foundation. All rights reserved.
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

/*
 * Global list.
 * All instances are inserted into global list - this allows easy iteration of all instances of a particular type.
 * The list is doubly linked for fast removal.  The list is in no particular order.
 */
struct ecm_db_mapping_instance *ecm_db_mappings = NULL;

/*
 * Mapping hash table
 */
#define ECM_DB_MAPPING_HASH_SLOTS 32768
static struct ecm_db_mapping_instance **ecm_db_mapping_table;
							/* Slots of the mapping hash table */
static int *ecm_db_mapping_table_lengths;
							/* Tracks how long each chain is */
static int ecm_db_mapping_count = 0;			/* Number of mappings allocated */

/*
 * Mapping flags
 */
#define ECM_DB_MAPPING_FLAGS_INSERTED 1	/* Mapping is inserted into connection database tables */

/*
 * ecm_db_mapping_generate_hash_index()
 * 	Calculate the hash index.
 */
static inline ecm_db_mapping_hash_t ecm_db_mapping_generate_hash_index(ip_addr_t address, uint32_t port)
{
	uint32_t tuple;
	uint32_t hash_val;

	ECM_IP_ADDR_HASH(tuple, address);
	hash_val = (uint32_t)jhash_2words(tuple, port, ecm_db_jhash_rnd);
	return (ecm_db_mapping_hash_t)(hash_val & (ECM_DB_MAPPING_HASH_SLOTS - 1));
}

/*
 * _ecm_db_mapping_count_get()
 *	Return the mapping count (lockless).
 */
int _ecm_db_mapping_count_get(void)
{
	return ecm_db_mapping_count;
}

/*
 * _ecm_db_mapping_ref()
 */
void _ecm_db_mapping_ref(struct ecm_db_mapping_instance *mi)
{
	DEBUG_CHECK_MAGIC(mi, ECM_DB_MAPPING_INSTANCE_MAGIC, "%px: magic failed\n", mi);
	mi->refs++;
	DEBUG_TRACE("%px: mapping ref %d\n", mi, mi->refs);
	DEBUG_ASSERT(mi->refs > 0, "%px: ref wrap\n", mi);
}

/*
 * ecm_db_mapping_ref()
 */
void ecm_db_mapping_ref(struct ecm_db_mapping_instance *mi)
{
	spin_lock_bh(&ecm_db_lock);
	_ecm_db_mapping_ref(mi);
	spin_unlock_bh(&ecm_db_lock);
}
EXPORT_SYMBOL(ecm_db_mapping_ref);

#ifdef ECM_DB_ADVANCED_STATS_ENABLE
/*
 * ecm_db_mapping_data_stats_get()
 *	Return data stats for the instance
 */
void ecm_db_mapping_data_stats_get(struct ecm_db_mapping_instance *mi, uint64_t *from_data_total, uint64_t *to_data_total,
						uint64_t *from_packet_total, uint64_t *to_packet_total,
						uint64_t *from_data_total_dropped, uint64_t *to_data_total_dropped,
						uint64_t *from_packet_total_dropped, uint64_t *to_packet_total_dropped)
{
	DEBUG_CHECK_MAGIC(mi, ECM_DB_MAPPING_INSTANCE_MAGIC, "%px: magic failed", mi);
	spin_lock_bh(&ecm_db_lock);
	if (from_data_total) {
		*from_data_total = mi->from_data_total;
	}
	if (to_data_total) {
		*to_data_total = mi->to_data_total;
	}
	if (from_packet_total) {
		*from_packet_total = mi->from_packet_total;
	}
	if (to_packet_total) {
		*to_packet_total = mi->to_packet_total;
	}
	if (from_data_total_dropped) {
		*from_data_total_dropped = mi->from_data_total_dropped;
	}
	if (to_data_total_dropped) {
		*to_data_total_dropped = mi->to_data_total_dropped;
	}
	if (from_packet_total_dropped) {
		*from_packet_total_dropped = mi->from_packet_total_dropped;
	}
	if (to_packet_total_dropped) {
		*to_packet_total_dropped = mi->to_packet_total_dropped;
	}
	spin_unlock_bh(&ecm_db_lock);
}
EXPORT_SYMBOL(ecm_db_mapping_data_stats_get);
#endif

/*
 * ecm_db_mapping_state_get()
 *	Prepare a mapping message
 */
int ecm_db_mapping_state_get(struct ecm_state_file_instance *sfi, struct ecm_db_mapping_instance *mi)
{
	int result;
	int port;
	char address[ECM_IP_ADDR_STR_BUFF_SIZE];
	int tcp_count[ECM_DB_OBJ_DIR_MAX];
	int udp_count[ECM_DB_OBJ_DIR_MAX];
	int conn_count[ECM_DB_OBJ_DIR_MAX];
	uint32_t time_added;
	struct ecm_db_host_instance *hi;
#ifdef ECM_DB_ADVANCED_STATS_ENABLE
	uint64_t from_data_total;
	uint64_t to_data_total;
	uint64_t from_packet_total;
	uint64_t to_packet_total;
	uint64_t from_data_total_dropped;
	uint64_t to_data_total_dropped;
	uint64_t from_packet_total_dropped;
	uint64_t to_packet_total_dropped;
#endif
	DEBUG_CHECK_MAGIC(mi, ECM_DB_MAPPING_INSTANCE_MAGIC, "%px: magic failed", mi);

	DEBUG_TRACE("Prep mapping msg for %px\n", mi);

	/*
	 * Create a small xml stats element for our mapping.
	 * Extract information from the mapping for inclusion into the message
	 */
	spin_lock_bh(&ecm_db_lock);
	memcpy(tcp_count, mi->tcp_count, sizeof(tcp_count));
	memcpy(udp_count, mi->udp_count, sizeof(udp_count));
	memcpy(conn_count, mi->conn_count, sizeof(conn_count));
	spin_unlock_bh(&ecm_db_lock);

	port = mi->port;
	time_added = mi->time_added;
	hi = mi->host;
	ecm_ip_addr_to_string(address, hi->address);

#ifdef ECM_DB_ADVANCED_STATS_ENABLE
	ecm_db_mapping_data_stats_get(mi, &from_data_total, &to_data_total,
			&from_packet_total, &to_packet_total,
			&from_data_total_dropped, &to_data_total_dropped,
			&from_packet_total_dropped, &to_packet_total_dropped);
#endif

	if ((result = ecm_state_prefix_add(sfi, "mapping"))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "port", "%d", port))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "from", "%d", conn_count[ECM_DB_OBJ_DIR_FROM]))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "to", "%d", conn_count[ECM_DB_OBJ_DIR_TO]))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "tcp_from", "%d", tcp_count[ECM_DB_OBJ_DIR_FROM]))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "tcp_to", "%d", tcp_count[ECM_DB_OBJ_DIR_TO]))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "udp_from", "%d", udp_count[ECM_DB_OBJ_DIR_FROM]))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "udp_to", "%d", udp_count[ECM_DB_OBJ_DIR_TO]))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "nat_from", "%d", conn_count[ECM_DB_OBJ_DIR_FROM_NAT]))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "nat_to", "%d", conn_count[ECM_DB_OBJ_DIR_TO_NAT]))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "tcp_nat_from", "%d", tcp_count[ECM_DB_OBJ_DIR_FROM_NAT]))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "tcp_nat_to", "%d", tcp_count[ECM_DB_OBJ_DIR_TO_NAT]))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "udp_nat_from", "%d", udp_count[ECM_DB_OBJ_DIR_FROM_NAT]))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "udp_nat_to", "%d", udp_count[ECM_DB_OBJ_DIR_TO_NAT]))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "address", "%s", address))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "time_added", "%u", time_added))) {
		return result;
	}

#ifdef ECM_DB_ADVANCED_STATS_ENABLE
	if ((result = ecm_db_adv_stats_state_write(sfi, from_data_total, to_data_total,
			from_packet_total, to_packet_total, from_data_total_dropped,
			to_data_total_dropped, from_packet_total_dropped,
			to_packet_total_dropped))) {
		return result;
	}
#endif

	return ecm_state_prefix_remove(sfi);
}
EXPORT_SYMBOL(ecm_db_mapping_state_get);

/*
 * ecm_db_mapping_adress_get()
 *	Return address
 */
void ecm_db_mapping_adress_get(struct ecm_db_mapping_instance *mi, ip_addr_t addr)
{
	DEBUG_CHECK_MAGIC(mi, ECM_DB_MAPPING_INSTANCE_MAGIC, "%px: magic failed", mi);
	ECM_IP_ADDR_COPY(addr, mi->host->address);
}
EXPORT_SYMBOL(ecm_db_mapping_adress_get);

/*
 * ecm_db_mapping_port_get()
 *	Return port
 */
int ecm_db_mapping_port_get(struct ecm_db_mapping_instance *mi)
{
	DEBUG_CHECK_MAGIC(mi, ECM_DB_MAPPING_INSTANCE_MAGIC, "%px: magic failed", mi);
	return mi->port;
}
EXPORT_SYMBOL(ecm_db_mapping_port_get);

/*
 * ecm_db_mappings_get_and_ref_first()
 *	Obtain a ref to the first mapping instance, if any
 */
struct ecm_db_mapping_instance *ecm_db_mappings_get_and_ref_first(void)
{
	struct ecm_db_mapping_instance *mi;
	spin_lock_bh(&ecm_db_lock);
	mi = ecm_db_mappings;
	if (mi) {
		_ecm_db_mapping_ref(mi);
	}
	spin_unlock_bh(&ecm_db_lock);
	return mi;
}
EXPORT_SYMBOL(ecm_db_mappings_get_and_ref_first);

/*
 * ecm_db_mapping_get_and_ref_next()
 *	Return the next mapping in the list given a mapping
 */
struct ecm_db_mapping_instance *ecm_db_mapping_get_and_ref_next(struct ecm_db_mapping_instance *mi)
{
	struct ecm_db_mapping_instance *min;
	DEBUG_CHECK_MAGIC(mi, ECM_DB_MAPPING_INSTANCE_MAGIC, "%px: magic failed", mi);
	spin_lock_bh(&ecm_db_lock);
	min = mi->next;
	if (min) {
		_ecm_db_mapping_ref(min);
	}
	spin_unlock_bh(&ecm_db_lock);
	return min;
}
EXPORT_SYMBOL(ecm_db_mapping_get_and_ref_next);

/*
 * ecm_db_mapping_deref()
 *	Release ref to mapping, possibly removing it from the database and destroying it.
 */
int ecm_db_mapping_deref(struct ecm_db_mapping_instance *mi)
{
#if (DEBUG_LEVEL >= 1)
	int dir;
#endif
	DEBUG_CHECK_MAGIC(mi, ECM_DB_MAPPING_INSTANCE_MAGIC, "%px: magic failed\n", mi);

	spin_lock_bh(&ecm_db_lock);
	mi->refs--;
	DEBUG_TRACE("%px: mapping deref %d\n", mi, mi->refs);
	DEBUG_ASSERT(mi->refs >= 0, "%px: ref wrap\n", mi);

	if (mi->refs > 0) {
		int refs = mi->refs;
		spin_unlock_bh(&ecm_db_lock);
		return refs;
	}

#if (DEBUG_LEVEL >= 1)
	for (dir = 0; dir < ECM_DB_OBJ_DIR_MAX; dir++) {
		DEBUG_ASSERT(!mi->tcp_count[dir] && !mi->udp_count[dir] && !mi->conn_count[dir], "%px: %s not zero: %d, %d, %d\n",
			     mi, ecm_db_obj_dir_strings[dir], mi->tcp_count[dir], mi->udp_count[dir], mi->conn_count[dir]);
#ifdef ECM_DB_XREF_ENABLE
		DEBUG_ASSERT(!mi->connections[dir], "%px: %s not null: %px\n", mi, ecm_db_obj_dir_strings[dir], mi->connections[dir]);
#endif
	}
#endif
	/*
	 * Remove from database if inserted
	 */
	if (!mi->flags & ECM_DB_MAPPING_FLAGS_INSERTED) {
		spin_unlock_bh(&ecm_db_lock);
	} else {
		struct ecm_db_listener_instance *li;

		/*
		 * Remove from the global list
		 */
		if (!mi->prev) {
			DEBUG_ASSERT(ecm_db_mappings == mi, "%px: mapping table bad\n", mi);
			ecm_db_mappings = mi->next;
		} else {
			mi->prev->next = mi->next;
		}
		if (mi->next) {
			mi->next->prev = mi->prev;
		}
		mi->prev = NULL;
		mi->next = NULL;

		/*
		 * Unlink it from the mapping hash table
		 */
		if (!mi->hash_prev) {
			DEBUG_ASSERT(ecm_db_mapping_table[mi->hash_index] == mi, "%px: hash table bad\n", mi);
			ecm_db_mapping_table[mi->hash_index] = mi->hash_next;
		} else {
			mi->hash_prev->hash_next = mi->hash_next;
		}
		if (mi->hash_next) {
			mi->hash_next->hash_prev = mi->hash_prev;
		}
		mi->hash_next = NULL;
		mi->hash_prev = NULL;
		ecm_db_mapping_table_lengths[mi->hash_index]--;
		DEBUG_ASSERT(ecm_db_mapping_table_lengths[mi->hash_index] >= 0, "%px: invalid table len %d\n", mi, ecm_db_mapping_table_lengths[mi->hash_index]);

#ifdef ECM_DB_XREF_ENABLE
		/*
		 * Unlink it from the host mapping list
		 */
		if (!mi->mapping_prev) {
			DEBUG_ASSERT(mi->host->mappings == mi, "%px: mapping table bad\n", mi);
			mi->host->mappings = mi->mapping_next;
		} else {
			mi->mapping_prev->mapping_next = mi->mapping_next;
		}
		if (mi->mapping_next) {
			mi->mapping_next->mapping_prev = mi->mapping_prev;
		}
		mi->mapping_next = NULL;
		mi->mapping_prev = NULL;

		mi->host->mapping_count--;
#endif
		spin_unlock_bh(&ecm_db_lock);

		/*
		 * Throw removed event to listeners
		 */
		DEBUG_TRACE("%px: Throw mapping removed event\n", mi);
		li = ecm_db_listeners_get_and_ref_first();
		while (li) {
			struct ecm_db_listener_instance *lin;
			if (li->mapping_removed) {
				li->mapping_removed(li->arg, mi);
			}

			/*
			 * Get next listener
			 */
			lin = ecm_db_listener_get_and_ref_next(li);
			ecm_db_listener_deref(li);
			li = lin;
		}
	}

	/*
	 * Throw final event
	 */
	if (mi->final) {
		mi->final(mi->arg);
	}

	/*
	 * Now release the host instance if the mapping had one
	 */
	if (mi->host) {
		ecm_db_host_deref(mi->host);
	}

	/*
	 * We can now destroy the instance
	 */
	DEBUG_CLEAR_MAGIC(mi);
	kfree(mi);

	/*
	 * Decrease global mapping count
	 */
	spin_lock_bh(&ecm_db_lock);
	ecm_db_mapping_count--;
	DEBUG_ASSERT(ecm_db_mapping_count >= 0, "%px: mapping count wrap\n", mi);
	spin_unlock_bh(&ecm_db_lock);

	return 0;
}
EXPORT_SYMBOL(ecm_db_mapping_deref);

/*
 * ecm_db_mapping_find_and_ref()
 *	Lookup and return a mapping reference if any.
 *
 * NOTE: For non-port based protocols the ports are expected to be -(protocol)
 */
struct ecm_db_mapping_instance *ecm_db_mapping_find_and_ref(ip_addr_t address, int port)
{
	ecm_db_mapping_hash_t hash_index;
	struct ecm_db_mapping_instance *mi;

	DEBUG_TRACE("Lookup mapping with addr " ECM_IP_ADDR_OCTAL_FMT " and port %d\n", ECM_IP_ADDR_TO_OCTAL(address), port);

	/*
	 * Compute the hash chain index and prepare to walk the chain
	 */
	hash_index = ecm_db_mapping_generate_hash_index(address, port);

	/*
	 * Iterate the chain looking for a mapping with matching details
	 */
	spin_lock_bh(&ecm_db_lock);
	mi = ecm_db_mapping_table[hash_index];
	while (mi) {
		if (mi->port != port) {
			mi = mi->hash_next;
			continue;
		}

		if (!ECM_IP_ADDR_MATCH(mi->host->address, address)) {
			mi = mi->hash_next;
			continue;
		}

		_ecm_db_mapping_ref(mi);
		spin_unlock_bh(&ecm_db_lock);
		DEBUG_TRACE("Mapping found %px\n", mi);
		return mi;
	}
	spin_unlock_bh(&ecm_db_lock);
	DEBUG_TRACE("Mapping not found\n");
	return NULL;
}
EXPORT_SYMBOL(ecm_db_mapping_find_and_ref);

#ifdef ECM_DB_XREF_ENABLE
/*
 * ecm_db_mapping_connections_get_and_ref_first()
 *	Return a reference to the first connection made on this mapping in the specified direction.
 */
struct ecm_db_connection_instance *ecm_db_mapping_connections_get_and_ref_first(struct ecm_db_mapping_instance *mi, ecm_db_obj_dir_t dir)
{
	struct ecm_db_connection_instance *ci;

	DEBUG_CHECK_MAGIC(mi, ECM_DB_MAPPING_INSTANCE_MAGIC, "%px: magic failed", mi);

	spin_lock_bh(&ecm_db_lock);
	ci = mi->connections[dir];
	if (ci) {
		_ecm_db_connection_ref(ci);
	}
	spin_unlock_bh(&ecm_db_lock);

	return ci;
}
EXPORT_SYMBOL(ecm_db_mapping_connections_get_and_ref_first);
#endif

/*
 * ecm_db_mapping_host_get_and_ref()
 */
struct ecm_db_host_instance *ecm_db_mapping_host_get_and_ref(struct ecm_db_mapping_instance *mi)
{
	DEBUG_CHECK_MAGIC(mi, ECM_DB_MAPPING_INSTANCE_MAGIC, "%px: magic failed\n", mi);

	spin_lock_bh(&ecm_db_lock);
	_ecm_db_host_ref(mi->host);
	spin_unlock_bh(&ecm_db_lock);
	return mi->host;
}
EXPORT_SYMBOL(ecm_db_mapping_host_get_and_ref);

/*
 * ecm_db_mapping_connections_total_count_get()
 *	Return the total number of connections (NAT and non-NAT) this mapping has
 */
int ecm_db_mapping_connections_total_count_get(struct ecm_db_mapping_instance *mi)
{
	int count;

	DEBUG_CHECK_MAGIC(mi, ECM_DB_MAPPING_INSTANCE_MAGIC, "%px: magic failed\n", mi);

	spin_lock_bh(&ecm_db_lock);
	count = mi->conn_count[ECM_DB_OBJ_DIR_FROM] +
		mi->conn_count[ECM_DB_OBJ_DIR_TO] +
		mi->conn_count[ECM_DB_OBJ_DIR_FROM_NAT] +
		mi->conn_count[ECM_DB_OBJ_DIR_TO_NAT];

	DEBUG_ASSERT(count >= 0, "%px: Count overflow from: %d, to: %d, nat_from: %d, nat_to: %d\n",
		     mi, mi->conn_count[ECM_DB_OBJ_DIR_FROM], mi->conn_count[ECM_DB_OBJ_DIR_TO],
		     mi->conn_count[ECM_DB_OBJ_DIR_FROM_NAT], mi->conn_count[ECM_DB_OBJ_DIR_TO_NAT]);
	spin_unlock_bh(&ecm_db_lock);
	return count;
}
EXPORT_SYMBOL(ecm_db_mapping_connections_total_count_get);

/*
 * ecm_db_mapping_add()
 *	Add a mapping instance into the database
 *
 * NOTE: The mapping will take a reference to the host instance.
 */
void ecm_db_mapping_add(struct ecm_db_mapping_instance *mi, struct ecm_db_host_instance *hi, int port,
						ecm_db_mapping_final_callback_t final, void *arg)
{
	ecm_db_mapping_hash_t hash_index;
	struct ecm_db_listener_instance *li;

	spin_lock_bh(&ecm_db_lock);
	DEBUG_CHECK_MAGIC(mi, ECM_DB_MAPPING_INSTANCE_MAGIC, "%px: magic failed\n", mi);
	DEBUG_CHECK_MAGIC(hi, ECM_DB_HOST_INSTANCE_MAGIC, "%px: magic failed\n", hi);
	DEBUG_ASSERT(!(mi->flags & ECM_DB_MAPPING_FLAGS_INSERTED), "%px: inserted\n", mi);
	DEBUG_ASSERT((hi->flags & ECM_DB_HOST_FLAGS_INSERTED), "%px: not inserted\n", hi);
	DEBUG_ASSERT(!mi->tcp_count[ECM_DB_OBJ_DIR_FROM] && !mi->tcp_count[ECM_DB_OBJ_DIR_TO] &&
		     !mi->udp_count[ECM_DB_OBJ_DIR_FROM] && !mi->udp_count[ECM_DB_OBJ_DIR_TO],
		     "%px: protocol count errors\n", mi);
#ifdef ECM_DB_XREF_ENABLE
	DEBUG_ASSERT(mi->connections[ECM_DB_OBJ_DIR_FROM] == NULL, "%px: connections not null\n", mi);
	DEBUG_ASSERT(mi->connections[ECM_DB_OBJ_DIR_TO] == NULL, "%px: connections not null\n", mi);
	DEBUG_ASSERT(!mi->conn_count[ECM_DB_OBJ_DIR_FROM] && !mi->conn_count[ECM_DB_OBJ_DIR_TO] &&
		     !mi->conn_count[ECM_DB_OBJ_DIR_FROM_NAT] && !mi->conn_count[ECM_DB_OBJ_DIR_TO_NAT],
		     "%px: connection count errors\n", mi);
#endif
	spin_unlock_bh(&ecm_db_lock);

	mi->arg = arg;
	mi->final = final;

	/*
	 * Compute hash table position for insertion
	 */
	hash_index = ecm_db_mapping_generate_hash_index(hi->address, port);
	mi->hash_index = hash_index;

	/*
	 * Record port
	 */
	mi->port = port;

	/*
	 * Mapping takes a ref to the host
	 */
	ecm_db_host_ref(hi);
	mi->host = hi;

	/*
	 * Set time
	 */
	spin_lock_bh(&ecm_db_lock);
	mi->time_added = ecm_db_time;

	/*
	 * Record the mapping is inserted
	 */
	mi->flags |= ECM_DB_MAPPING_FLAGS_INSERTED;

	/*
	 * Add into the global list
	 */
	mi->prev = NULL;
	mi->next = ecm_db_mappings;
	if (ecm_db_mappings) {
		ecm_db_mappings->prev = mi;
	}
	ecm_db_mappings = mi;

	/*
	 * Insert mapping into the mappings hash table
	 */
	mi->hash_prev = NULL;
	mi->hash_next = ecm_db_mapping_table[hash_index];
	if (ecm_db_mapping_table[hash_index]) {
		ecm_db_mapping_table[hash_index]->hash_prev = mi;
	}
	ecm_db_mapping_table[hash_index] = mi;
	ecm_db_mapping_table_lengths[hash_index]++;
	DEBUG_ASSERT(ecm_db_mapping_table_lengths[hash_index] > 0, "%px: invalid table len %d\n", hi, ecm_db_mapping_table_lengths[hash_index]);

#ifdef ECM_DB_XREF_ENABLE
	/*
	 * Insert mapping into the host mapping list
	 */
	mi->mapping_prev = NULL;
	mi->mapping_next = hi->mappings;
	if (hi->mappings) {
		hi->mappings->mapping_prev = mi;
	}
	hi->mappings = mi;
	hi->mapping_count++;
#endif
	spin_unlock_bh(&ecm_db_lock);

	/*
	 * Throw add event to the listeners
	 */
	DEBUG_TRACE("%px: Throw mapping added event\n", mi);
	li = ecm_db_listeners_get_and_ref_first();
	while (li) {
		struct ecm_db_listener_instance *lin;
		if (li->mapping_added) {
			li->mapping_added(li->arg, mi);
		}

		/*
		 * Get next listener
		 */
		lin = ecm_db_listener_get_and_ref_next(li);
		ecm_db_listener_deref(li);
		li = lin;
	}
}
EXPORT_SYMBOL(ecm_db_mapping_add);

/*
 * ecm_db_mapping_hash_table_lengths_get()
 *	Return hash table length
 */
int ecm_db_mapping_hash_table_lengths_get(int index)
{
	int length;

	DEBUG_ASSERT((index >= 0) && (index < ECM_DB_MAPPING_HASH_SLOTS), "Bad protocol: %d\n", index);
	spin_lock_bh(&ecm_db_lock);
	length = ecm_db_mapping_table_lengths[index];
	spin_unlock_bh(&ecm_db_lock);
	return length;
}
EXPORT_SYMBOL(ecm_db_mapping_hash_table_lengths_get);

/*
 * ecm_db_mapping_hash_index_get_next()
 * Given a hash index, return the next one OR return -1 for no more hash indicies to return.
 */
int ecm_db_mapping_hash_index_get_next(int index)
{
	index++;
	if (index >= ECM_DB_MAPPING_HASH_SLOTS) {
		return -1;
	}
	return index;
}
EXPORT_SYMBOL(ecm_db_mapping_hash_index_get_next);

/*
 * ecm_db_mapping_hash_index_get_first()
 * Return first hash index
 */
int ecm_db_mapping_hash_index_get_first(void)
{
	return 0;
}
EXPORT_SYMBOL(ecm_db_mapping_hash_index_get_first);

/*
 * ecm_db_mapping_alloc()
 *	Allocate a mapping instance
 */
struct ecm_db_mapping_instance *ecm_db_mapping_alloc(void)
{
	struct ecm_db_mapping_instance *mi;

	mi = (struct ecm_db_mapping_instance *)kzalloc(sizeof(struct ecm_db_mapping_instance), GFP_ATOMIC | __GFP_NOWARN);
	if (!mi) {
		DEBUG_WARN("Alloc failed\n");
		return NULL;
	}

	mi->refs = 1;
	DEBUG_SET_MAGIC(mi, ECM_DB_MAPPING_INSTANCE_MAGIC);

	/*
	 * Alloc operation must be atomic to ensure thread and module can be held
	 */
	spin_lock_bh(&ecm_db_lock);

	/*
	 * If the event processing thread is terminating then we cannot create new instances
	 */
	if (ecm_db_terminate_pending) {
		spin_unlock_bh(&ecm_db_lock);
		DEBUG_WARN("Thread terminating\n");
		kfree(mi);
		return NULL;
	}

	ecm_db_mapping_count++;
	spin_unlock_bh(&ecm_db_lock);

	DEBUG_TRACE("Mapping created %px\n", mi);
	return mi;
}
EXPORT_SYMBOL(ecm_db_mapping_alloc);

/*
 * ecm_db_mapping_init()
 */
bool ecm_db_mapping_init(struct dentry *dentry)
{
	if (!ecm_debugfs_create_u32("mapping_count", S_IRUGO, dentry,
					(u32 *)&ecm_db_mapping_count)) {
		DEBUG_ERROR("Failed to create ecm db mapping count file in debugfs\n");
		return false;
	}

	ecm_db_mapping_table = vzalloc(sizeof(struct ecm_db_mapping_instance *) * ECM_DB_MAPPING_HASH_SLOTS);
	if (!ecm_db_mapping_table) {
		DEBUG_ERROR("Failed to allocate virtual memory for ecm_db_mapping_table\n");
		return false;
	}

	ecm_db_mapping_table_lengths = vzalloc(sizeof(int) * ECM_DB_MAPPING_HASH_SLOTS);
	if (!ecm_db_mapping_table_lengths) {
		DEBUG_ERROR("Failed to allocate virtual memory for ecm_db_mapping_table_lengths\n");
		vfree(ecm_db_mapping_table);
		return false;
	}

	return true;
}

/*
 * ecm_db_mapping_exit()
 */
void ecm_db_mapping_exit(void)
{
	vfree(ecm_db_mapping_table_lengths);
	vfree(ecm_db_mapping_table);
}
