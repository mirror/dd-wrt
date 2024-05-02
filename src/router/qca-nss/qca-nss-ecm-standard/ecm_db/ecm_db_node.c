/*
 **************************************************************************
 * Copyright (c) 2014-2018, 2020-2021 The Linux Foundation. All rights reserved.
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

/*
 * Global list.
 * All instances are inserted into global list - this allows easy iteration of all instances of a particular type.
 * The list is doubly linked for fast removal.  The list is in no particular order.
 */
struct ecm_db_node_instance *ecm_db_nodes = NULL;

/*
 * Node hash table
 */
#define ECM_DB_NODE_HASH_SLOTS 32768
static struct ecm_db_node_instance **ecm_db_node_table;
							/* Slots of the node hash table */
static int *ecm_db_node_table_lengths;
							/* Tracks how long each chain is */
static int ecm_db_node_count = 0;			/* Number of nodes allocated */

/*
 * Node flags
 */
#define ECM_DB_NODE_FLAGS_INSERTED 1			/* Node is inserted into connection database tables */

/*
 * ecm_db_node_generate_hash_index()
 * 	Calculate the hash index.
 */
static inline ecm_db_node_hash_t ecm_db_node_generate_hash_index(uint8_t *address)
{
	uint32_t hash_val;

	hash_val = (uint32_t)jhash(address, 6, ecm_db_jhash_rnd);
	hash_val &= (ECM_DB_NODE_HASH_SLOTS - 1);

	return (ecm_db_node_hash_t)hash_val;
}

/*
 * _ecm_db_node_count_get()
 *	Return the node count (lockless).
 */
int _ecm_db_node_count_get(void)
{
	return ecm_db_node_count;
}

/*
 * _ecm_db_node_ref()
 */
void _ecm_db_node_ref(struct ecm_db_node_instance *ni)
{
	DEBUG_CHECK_MAGIC(ni, ECM_DB_NODE_INSTANCE_MAGIC, "%px: magic failed\n", ni);
	ni->refs++;
	DEBUG_TRACE("%px: node ref %d\n", ni, ni->refs);
	DEBUG_ASSERT(ni->refs > 0, "%px: ref wrap\n", ni);
}

/*
 * ecm_db_node_ref()
 */
void ecm_db_node_ref(struct ecm_db_node_instance *ni)
{
	spin_lock_bh(&ecm_db_lock);
	_ecm_db_node_ref(ni);
	spin_unlock_bh(&ecm_db_lock);
}
EXPORT_SYMBOL(ecm_db_node_ref);

#ifdef ECM_DB_ADVANCED_STATS_ENABLE
/*
 * ecm_db_node_data_stats_get()
 *	Return data stats for the instance
 */
void ecm_db_node_data_stats_get(struct ecm_db_node_instance *ni, uint64_t *from_data_total, uint64_t *to_data_total,
						uint64_t *from_packet_total, uint64_t *to_packet_total,
						uint64_t *from_data_total_dropped, uint64_t *to_data_total_dropped,
						uint64_t *from_packet_total_dropped, uint64_t *to_packet_total_dropped)
{
	DEBUG_CHECK_MAGIC(ni, ECM_DB_NODE_INSTANCE_MAGIC, "%px: magic failed", ni);
	spin_lock_bh(&ecm_db_lock);
	if (from_data_total) {
		*from_data_total = ni->from_data_total;
	}
	if (to_data_total) {
		*to_data_total = ni->to_data_total;
	}
	if (from_packet_total) {
		*from_packet_total = ni->from_packet_total;
	}
	if (to_packet_total) {
		*to_packet_total = ni->to_packet_total;
	}
	if (from_data_total_dropped) {
		*from_data_total_dropped = ni->from_data_total_dropped;
	}
	if (to_data_total_dropped) {
		*to_data_total_dropped = ni->to_data_total_dropped;
	}
	if (from_packet_total_dropped) {
		*from_packet_total_dropped = ni->from_packet_total_dropped;
	}
	if (to_packet_total_dropped) {
		*to_packet_total_dropped = ni->to_packet_total_dropped;
	}
	spin_unlock_bh(&ecm_db_lock);
}
EXPORT_SYMBOL(ecm_db_node_data_stats_get);
#endif

/*
 * ecm_db_node_adress_get()
 *	Return address
 */
void ecm_db_node_adress_get(struct ecm_db_node_instance *ni, uint8_t *address_buffer)
{
	DEBUG_CHECK_MAGIC(ni, ECM_DB_NODE_INSTANCE_MAGIC, "%px: magic failed", ni);
	memcpy(address_buffer, ni->address, ETH_ALEN);
}
EXPORT_SYMBOL(ecm_db_node_adress_get);

/*
 * ecm_db_nodes_get_and_ref_first()
 *	Obtain a ref to the first node instance, if any
 */
struct ecm_db_node_instance *ecm_db_nodes_get_and_ref_first(void)
{
	struct ecm_db_node_instance *ni;
	spin_lock_bh(&ecm_db_lock);
	ni = ecm_db_nodes;
	if (ni) {
		_ecm_db_node_ref(ni);
	}
	spin_unlock_bh(&ecm_db_lock);
	return ni;
}
EXPORT_SYMBOL(ecm_db_nodes_get_and_ref_first);

/*
 * ecm_db_node_get_and_ref_next()
 *	Return the next node in the list given a node
 */
struct ecm_db_node_instance *ecm_db_node_get_and_ref_next(struct ecm_db_node_instance *ni)
{
	struct ecm_db_node_instance *nin;
	DEBUG_CHECK_MAGIC(ni, ECM_DB_NODE_INSTANCE_MAGIC, "%px: magic failed", ni);
	spin_lock_bh(&ecm_db_lock);
	nin = ni->next;
	if (nin) {
		_ecm_db_node_ref(nin);
	}
	spin_unlock_bh(&ecm_db_lock);
	return nin;
}
EXPORT_SYMBOL(ecm_db_node_get_and_ref_next);

/*
 * ecm_db_node_deref()
 *	Deref a node.  Removing it on the last ref and destroying it.
 */
int ecm_db_node_deref(struct ecm_db_node_instance *ni)
{
#if (DEBUG_LEVEL >= 1)
	int dir;
#endif
	DEBUG_CHECK_MAGIC(ni, ECM_DB_NODE_INSTANCE_MAGIC, "%px: magic failed\n", ni);

	spin_lock_bh(&ecm_db_lock);
	ni->refs--;
	DEBUG_TRACE("%px: node deref %d\n", ni, ni->refs);
	DEBUG_ASSERT(ni->refs >= 0, "%px: ref wrap\n", ni);

	if (ni->refs > 0) {
		int refs = ni->refs;
		spin_unlock_bh(&ecm_db_lock);
		return refs;
	}

#ifdef ECM_DB_XREF_ENABLE
#if (DEBUG_LEVEL >= 1)
	for (dir = 0; dir < ECM_DB_OBJ_DIR_MAX; dir++) {
		DEBUG_ASSERT((ni->connections[dir] == NULL) && (ni->connections_count[dir] == 0), "%px: %s connections not null\n", ni, ecm_db_obj_dir_strings[dir]);
	}
#endif
#endif

	/*
	 * Remove from database if inserted
	 */
	if (!ni->flags & ECM_DB_NODE_FLAGS_INSERTED) {
		spin_unlock_bh(&ecm_db_lock);
	} else {
		struct ecm_db_listener_instance *li;

		/*
		 * Remove from the global list
		 */
		if (!ni->prev) {
			DEBUG_ASSERT(ecm_db_nodes == ni, "%px: node table bad\n", ni);
			ecm_db_nodes = ni->next;
		} else {
			ni->prev->next = ni->next;
		}
		if (ni->next) {
			ni->next->prev = ni->prev;
		}
		ni->prev = NULL;
		ni->next = NULL;

		/*
		 * Link out of hash table
		 */
		if (!ni->hash_prev) {
			DEBUG_ASSERT(ecm_db_node_table[ni->hash_index] == ni, "%px: hash table bad\n", ni);
			ecm_db_node_table[ni->hash_index] = ni->hash_next;
		} else {
			ni->hash_prev->hash_next = ni->hash_next;
		}
		if (ni->hash_next) {
			ni->hash_next->hash_prev = ni->hash_prev;
		}
		ni->hash_next = NULL;
		ni->hash_prev = NULL;
		ecm_db_node_table_lengths[ni->hash_index]--;
		DEBUG_ASSERT(ecm_db_node_table_lengths[ni->hash_index] >= 0, "%px: invalid table len %d\n", ni, ecm_db_node_table_lengths[ni->hash_index]);

#ifdef ECM_DB_XREF_ENABLE
		/*
		 * Unlink it from the iface node list
		 */
		if (!ni->node_prev) {
			DEBUG_ASSERT(ni->iface->nodes == ni, "%px: nodes table bad\n", ni);
			ni->iface->nodes = ni->node_next;
		} else {
			ni->node_prev->node_next = ni->node_next;
		}
		if (ni->node_next) {
			ni->node_next->node_prev = ni->node_prev;
		}
		ni->node_next = NULL;
		ni->node_prev = NULL;
		ni->iface->node_count--;
#endif

		spin_unlock_bh(&ecm_db_lock);

		/*
		 * Throw removed event to listeners
		 */
		DEBUG_TRACE("%px: Throw node removed event\n", ni);
		li = ecm_db_listeners_get_and_ref_first();
		while (li) {
			struct ecm_db_listener_instance *lin;
			if (li->node_removed) {
				li->node_removed(li->arg, ni);
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
	if (ni->final) {
		ni->final(ni->arg);
	}

	/*
	 * Now release the iface instance if the node had one
	 */
	if (ni->iface) {
		ecm_db_iface_deref(ni->iface);
	}

	/*
	 * We can now destroy the instance
	 */
	DEBUG_CLEAR_MAGIC(ni);
	kfree(ni);

	/*
	 * Decrease global node count
	 */
	spin_lock_bh(&ecm_db_lock);
	ecm_db_node_count--;
	DEBUG_ASSERT(ecm_db_node_count >= 0, "%px: node count wrap\n", ni);
	spin_unlock_bh(&ecm_db_lock);

	return 0;
}
EXPORT_SYMBOL(ecm_db_node_deref);

/*
 * ecm_db_node_is_mac_addr_equal()
 *	Compares the node's mac address with the given mac address.
 */
bool ecm_db_node_is_mac_addr_equal(struct ecm_db_node_instance *ni, uint8_t *address)
{
	DEBUG_CHECK_MAGIC(ni, ECM_DB_NODE_INSTANCE_MAGIC, "%px: magic failed", ni);

	if (ecm_mac_addr_equal(ni->address, address)) {
		return false;
	}

	return true;
}
EXPORT_SYMBOL(ecm_db_node_is_mac_addr_equal);

/*
 * ecm_db_node_find_and_ref()
 *	Lookup and return a node reference if any
 */
struct ecm_db_node_instance *ecm_db_node_find_and_ref(uint8_t *address, struct ecm_db_iface_instance *ii)
{
	ecm_db_node_hash_t hash_index;
	struct ecm_db_node_instance *ni;

	DEBUG_TRACE("Lookup node with addr %pMi and iface %px\n", address, ii);

	/*
	 * Compute the hash chain index and prepare to walk the chain
	 */
	hash_index = ecm_db_node_generate_hash_index(address);

	/*
	 * Iterate the chain looking for a host with matching details
	 */
	spin_lock_bh(&ecm_db_lock);
	ni = ecm_db_node_table[hash_index];
	while (ni) {
		if (memcmp(ni->address, address, ETH_ALEN)) {
			ni = ni->hash_next;
			continue;
		}

		if (ni->iface != ii) {
			ni = ni->hash_next;
			continue;
		}

		_ecm_db_node_ref(ni);
		spin_unlock_bh(&ecm_db_lock);
		DEBUG_TRACE("node found %px\n", ni);
		return ni;
	}
	spin_unlock_bh(&ecm_db_lock);
	DEBUG_TRACE("Node not found\n");
	return NULL;
}
EXPORT_SYMBOL(ecm_db_node_find_and_ref);

/*
 * ecm_db_node_chain_get_and_ref_first()
 *	Gets and refs the first node in the chain of that mac address.
 */
struct ecm_db_node_instance *ecm_db_node_chain_get_and_ref_first(uint8_t *address)
{
	ecm_db_node_hash_t hash_index;
	struct ecm_db_node_instance *ni;

	DEBUG_TRACE("Get the first node with addr %pMi in the chain\n", address);

	/*
	 * Compute the hash chain index.
	 */
	hash_index = ecm_db_node_generate_hash_index(address);

	spin_lock_bh(&ecm_db_lock);
	ni = ecm_db_node_table[hash_index];
	if (ni) {
		_ecm_db_node_ref(ni);
	}
	spin_unlock_bh(&ecm_db_lock);

	return ni;
}
EXPORT_SYMBOL(ecm_db_node_chain_get_and_ref_first);

/*
 * ecm_db_node_chain_get_and_ref_next()
 *	Gets and refs the next node in the chain..
 */
struct ecm_db_node_instance *ecm_db_node_chain_get_and_ref_next(struct ecm_db_node_instance *ni)
{
	struct ecm_db_node_instance *nin;
	DEBUG_CHECK_MAGIC(ni, ECM_DB_NODE_INSTANCE_MAGIC, "%px: magic failed", ni);

	spin_lock_bh(&ecm_db_lock);
	nin = ni->hash_next;
	if (nin) {
		_ecm_db_node_ref(nin);
	}
	spin_unlock_bh(&ecm_db_lock);
	return nin;
}
EXPORT_SYMBOL(ecm_db_node_chain_get_and_ref_next);

/*
 * ecm_db_node_iface_get_and_ref()
 */
struct ecm_db_iface_instance *ecm_db_node_iface_get_and_ref(struct ecm_db_node_instance *ni)
{
	DEBUG_CHECK_MAGIC(ni, ECM_DB_NODE_INSTANCE_MAGIC, "%px: magic failed\n", ni);

	spin_lock_bh(&ecm_db_lock);
	_ecm_db_iface_ref(ni->iface);
	spin_unlock_bh(&ecm_db_lock);
	return ni->iface;
}
EXPORT_SYMBOL(ecm_db_node_iface_get_and_ref);

/*
 * ecm_db_node_add()
 *	Add a node instance into the database
 */
void ecm_db_node_add(struct ecm_db_node_instance *ni, struct ecm_db_iface_instance *ii, uint8_t *address,
					ecm_db_node_final_callback_t final, void *arg)
{
#if (DEBUG_LEVEL >= 1)
	int dir;
#endif
	ecm_db_node_hash_t hash_index;
	struct ecm_db_listener_instance *li;

	spin_lock_bh(&ecm_db_lock);
	DEBUG_CHECK_MAGIC(ni, ECM_DB_NODE_INSTANCE_MAGIC, "%px: magic failed\n", ni);
	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);
	DEBUG_ASSERT(address, "%px: address null\n", ni);
	DEBUG_ASSERT((ni->iface == NULL), "%px: iface not null\n", ni);
	DEBUG_ASSERT(!(ni->flags & ECM_DB_NODE_FLAGS_INSERTED), "%px: inserted\n", ni);
#ifdef ECM_DB_XREF_ENABLE
#if (DEBUG_LEVEL >= 1)
	for (dir = 0; dir < ECM_DB_OBJ_DIR_MAX; dir++) {
		DEBUG_ASSERT((ni->connections[dir] == NULL) && (ni->connections_count[dir] == 0), "%px: %s connections not null\n", ni, ecm_db_obj_dir_strings[dir]);
	}
#endif
#endif
	spin_unlock_bh(&ecm_db_lock);

	memcpy(ni->address, address, ETH_ALEN);
	ni->arg = arg;
	ni->final = final;

	/*
	 * Compute hash chain for insertion
	 */
	hash_index = ecm_db_node_generate_hash_index(address);
	ni->hash_index = hash_index;

	/*
	 * Node takes a ref to the iface
	 */
	ecm_db_iface_ref(ii);
	ni->iface = ii;

	/*
	 * Add into the global list
	 */
	spin_lock_bh(&ecm_db_lock);
	ni->flags |= ECM_DB_NODE_FLAGS_INSERTED;
	ni->prev = NULL;
	ni->next = ecm_db_nodes;
	if (ecm_db_nodes) {
		ecm_db_nodes->prev = ni;
	}
	ecm_db_nodes = ni;

	/*
	 * Insert into the hash chain
	 */
	ni->hash_prev = NULL;
	ni->hash_next = ecm_db_node_table[hash_index];
	if (ecm_db_node_table[hash_index]) {
		ecm_db_node_table[hash_index]->hash_prev = ni;
	}
	ecm_db_node_table[hash_index] = ni;
	ecm_db_node_table_lengths[hash_index]++;
	DEBUG_ASSERT(ecm_db_node_table_lengths[hash_index] > 0, "%px: invalid table len %d\n", ni, ecm_db_node_table_lengths[hash_index]);

	/*
	 * Set time of add
	 */
	ni->time_added = ecm_db_time;

#ifdef ECM_DB_XREF_ENABLE
	/*
	 * Insert node into the iface nodes list
	 */
	ni->node_prev = NULL;
	ni->node_next = ii->nodes;
	if (ii->nodes) {
		ii->nodes->node_prev = ni;
	}
	ii->nodes = ni;
	ii->node_count++;
#endif
	spin_unlock_bh(&ecm_db_lock);

	/*
	 * Throw add event to the listeners
	 */
	DEBUG_TRACE("%px: Throw node added event\n", ni);
	li = ecm_db_listeners_get_and_ref_first();
	while (li) {
		struct ecm_db_listener_instance *lin;
		if (li->node_added) {
			li->node_added(li->arg, ni);
		}

		/*
		 * Get next listener
		 */
		lin = ecm_db_listener_get_and_ref_next(li);
		ecm_db_listener_deref(li);
		li = lin;
	}
}
EXPORT_SYMBOL(ecm_db_node_add);

/*
 * ecm_db_node_state_get()
 *	Prepare a node message
 */
int ecm_db_node_state_get(struct ecm_state_file_instance *sfi, struct ecm_db_node_instance *ni)
{
	int result;
	char address[ECM_MAC_ADDR_STR_BUFF_SIZE];
#ifdef ECM_DB_XREF_ENABLE
	int dir;
	int connections_count[ECM_DB_OBJ_DIR_MAX];
#endif
	uint32_t time_added;
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

	DEBUG_TRACE("Prep node msg for %px\n", ni);

	/*
	 * Create a small xml stats block for our managed node, like:
	 * <node address="" hosts="" time_added="" from_data_total="" to_data_total="" />
	 *
	 * Extract information from the node for inclusion into the message
	 */
#ifdef ECM_DB_XREF_ENABLE
	spin_lock_bh(&ecm_db_lock);
	for (dir = 0; dir < ECM_DB_OBJ_DIR_MAX; dir++) {
		connections_count[dir] = ni->connections_count[dir];
	}
	spin_unlock_bh(&ecm_db_lock);
#endif
	time_added = ni->time_added;
	snprintf(address, sizeof(address), "%pM", ni->address);

#ifdef ECM_DB_ADVANCED_STATS_ENABLE
	ecm_db_node_data_stats_get(ni, &from_data_total, &to_data_total,
			&from_packet_total, &to_packet_total,
			&from_data_total_dropped, &to_data_total_dropped,
			&from_packet_total_dropped, &to_packet_total_dropped);

#endif

	if ((result = ecm_state_prefix_add(sfi, "node"))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "address", "%s", address))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "time_added", "%u", time_added))) {
		return result;
	}
#ifdef ECM_DB_XREF_ENABLE
	for (dir = 0; dir < ECM_DB_OBJ_DIR_MAX; dir++) {
		/*
		 * ECM_DB_NODE_CONN_COUNT_STR_SIZE is the size of "FROM_NAT_connections_count"
		 * string which can be the max length of these strings
		 */
		char name[ECM_DB_NODE_CONN_COUNT_STR_SIZE];
		snprintf(name, ECM_DB_NODE_CONN_COUNT_STR_SIZE, "%s_connections_count", ecm_db_obj_dir_strings[dir]);
		if ((result = ecm_state_write(sfi, name, "%d", connections_count[dir]))) {
			return result;
		}
	}
#endif
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
EXPORT_SYMBOL(ecm_db_node_state_get);

/*
 * ecm_db_node_hash_table_lengths_get()
 *	Return hash table length
 */
int ecm_db_node_hash_table_lengths_get(int index)
{
	int length;

	DEBUG_ASSERT((index >= 0) && (index < ECM_DB_NODE_HASH_SLOTS), "Bad protocol: %d\n", index);
	spin_lock_bh(&ecm_db_lock);
	length = ecm_db_node_table_lengths[index];
	spin_unlock_bh(&ecm_db_lock);
	return length;
}
EXPORT_SYMBOL(ecm_db_node_hash_table_lengths_get);

/*
 * ecm_db_node_hash_index_get_next()
 * Given a hash index, return the next one OR return -1 for no more hash indicies to return.
 */
int ecm_db_node_hash_index_get_next(int index)
{
	index++;
	if (index >= ECM_DB_NODE_HASH_SLOTS) {
		return -1;
	}
	return index;
}
EXPORT_SYMBOL(ecm_db_node_hash_index_get_next);

/*
 * ecm_db_node_hash_index_get_first()
 * Return first hash index
 */
int ecm_db_node_hash_index_get_first(void)
{
	return 0;
}
EXPORT_SYMBOL(ecm_db_node_hash_index_get_first);

/*
 * ecm_db_node_get_connections_count()
 *	Returns the connections count on the node in the given direction.
 */
int ecm_db_node_get_connections_count(struct ecm_db_node_instance *ni, ecm_db_obj_dir_t dir)
{
	DEBUG_CHECK_MAGIC(ni, ECM_DB_NODE_INSTANCE_MAGIC, "%px: magic failed\n", ni);

	return ni->connections_count[dir];
}

/*
 * ecm_db_node_alloc()
 *	Allocate a node instance
 */
struct ecm_db_node_instance *ecm_db_node_alloc(void)
{
	struct ecm_db_node_instance *ni;

	ni = (struct ecm_db_node_instance *)kzalloc(sizeof(struct ecm_db_node_instance), GFP_ATOMIC | __GFP_NOWARN);
	if (!ni) {
		DEBUG_WARN("Alloc failed\n");
		return NULL;
	}

	ni->refs = 1;
	DEBUG_SET_MAGIC(ni, ECM_DB_NODE_INSTANCE_MAGIC);

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
		kfree(ni);
		return NULL;
	}

	ecm_db_node_count++;
	spin_unlock_bh(&ecm_db_lock);

	DEBUG_TRACE("Node created %px\n", ni);
	return ni;
}
EXPORT_SYMBOL(ecm_db_node_alloc);

#ifdef ECM_DB_XREF_ENABLE
/*
 * ecm_db_node_connections_get_and_ref_first()
 *	Obtain a ref to the first connection instance of node on this direction, if any
 */
static inline struct ecm_db_connection_instance *
ecm_db_node_connections_get_and_ref_first(struct ecm_db_node_instance *node,
					  ecm_db_obj_dir_t dir)
{
	struct ecm_db_connection_instance *ci;
	DEBUG_CHECK_MAGIC(node, ECM_DB_NODE_INSTANCE_MAGIC, "%px: magic failed", node);
	spin_lock_bh(&ecm_db_lock);
	ci = node->connections[dir];
	if (ci) {
		_ecm_db_connection_ref(ci);
	}
	spin_unlock_bh(&ecm_db_lock);
	return ci;
}

/*
 * ecm_db_node_connection_get_and_ref_next()
 *	Return the next connection in the specified direction of given a connection
 */
static inline struct ecm_db_connection_instance *
ecm_db_node_connection_get_and_ref_next(struct ecm_db_connection_instance *ci,
					ecm_db_obj_dir_t dir)
{
	struct ecm_db_connection_instance *cin;
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", ci);
	spin_lock_bh(&ecm_db_lock);
	cin = ci->node_next[dir];
	if (cin) {
		_ecm_db_connection_ref(cin);
	}
	spin_unlock_bh(&ecm_db_lock);
	return cin;
}

/*
 * ecm_db_should_keep_connection()
 *	check if any classifier believes this connection should
 *	be kept
 */
static bool ecm_db_should_keep_connection(
	struct ecm_db_connection_instance *ci, struct ecm_db_connection_defunct_info *info)
{
	int assignment_count;
	int aci_index;
	struct ecm_classifier_instance *assignments[ECM_CLASSIFIER_TYPES];

	/*
	 * In case of STA join event, we need to keep all connections apart from
	 * the connections with SAWF classifier. So we will make should_keep_connection
	 * flag true by default, and let the relevent classifier take the decision.
	 */
	info->should_keep_connection = (info->type == ECM_DB_CONNECTION_DEFUNCT_TYPE_STA_JOIN) ? true : false;

	assignment_count =
		ecm_db_connection_classifier_assignments_get_and_ref(ci, assignments);
	for (aci_index = 0; aci_index < assignment_count; ++aci_index) {
		struct ecm_classifier_instance *aci;
		aci = assignments[aci_index];
		if (aci->should_keep_connection) {
			aci->should_keep_connection(aci, info);
		}
	}
	ecm_db_connection_assignments_release(assignment_count, assignments);

	return info->should_keep_connection;
}

/*
 * ecm_db_traverse_node_connection_list_and_defunct()
 *	traverse a node in the specified direction and calls ecm_db_connection_make_defunct()
 *	for each entry.  If ip_version is valid (non-zero), then defunct the
 *	connections matching version.
 */
void ecm_db_traverse_node_connection_list_and_defunct(
	struct ecm_db_node_instance *node, ecm_db_obj_dir_t dir, int ip_version,
	ecm_db_connection_defunct_type_t type)
{
	struct ecm_db_connection_instance *ci = NULL;
	struct ecm_db_connection_defunct_info info;

	memcpy(info.mac, node->address, ETH_ALEN);
	info.type = type;

	/*
	 * Iterate all from connections
	 */
	ci = ecm_db_node_connections_get_and_ref_first(node, dir);
	while (ci) {
		struct ecm_db_connection_instance *cin;

		if (!ecm_db_should_keep_connection(ci, &info)) {
			if (ip_version != ECM_DB_IP_VERSION_IGNORE && (ecm_db_connection_ip_version_get(ci) != ip_version)) {
				DEBUG_TRACE("%px: keeping connection, ip_version mismatch %d\n", ci, ci->serial);
				goto keep_node_conn;
			}

			DEBUG_TRACE("%px: defunct %d\n", ci, ci->serial);
			ecm_db_connection_make_defunct(ci);
		} else {
			DEBUG_TRACE("%px: keeping connection %d\n", ci, ci->serial);
		}
keep_node_conn:
		cin = ecm_db_node_connection_get_and_ref_next(ci, dir);
		ecm_db_connection_deref(ci);
		ci = cin;
	}
	DEBUG_INFO("%px: Defuncting from node connection list complete\n", node);
}

#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE

/*
 * ecm_db_node_ovs_connections_masked_defunct()
 *	Destroy connections created on the node
 */
void ecm_db_node_ovs_connections_masked_defunct(int ip_ver, uint8_t *src_mac, bool src_mac_check, ip_addr_t src_addr_mask,
							uint16_t src_port_mask, uint8_t *dest_mac, bool dest_mac_check,
							ip_addr_t dest_addr_mask, uint16_t dest_port_mask,
							int proto_mask, ecm_db_obj_dir_t dir, bool is_routed)
{
	struct ecm_db_node_instance *ni;
	uint8_t smac[ETH_ALEN], dmac[ETH_ALEN];
	uint8_t *mac;
	ip_addr_t sip, dip;
	uint16_t sport, dport;
	int proto;
	int cnt = 0;
	char *direction = NULL;

	mac = (dir == ECM_DB_OBJ_DIR_FROM) ? src_mac : dest_mac;

	ni = ecm_db_node_chain_get_and_ref_first(mac);
	if (!ni) {
		DEBUG_WARN("Unable to find first instance node\n");
		return;
	}

	/*
	 * Iterate through all node instances
	 */
	while (ni) {
		struct ecm_db_connection_instance *ci;
		struct ecm_db_node_instance *nni;

		DEBUG_CHECK_MAGIC(ni, ECM_DB_NODE_INSTANCE_MAGIC, "%px: magic failed", ni);

		if (!ecm_db_node_is_mac_addr_equal(ni, mac)) {
			nni = ecm_db_node_chain_get_and_ref_next(ni);
			ecm_db_node_deref(ni);
			ni = nni;
			continue;
		}

		ci = ecm_db_node_connections_get_and_ref_first(ni, dir);
		while (ci) {
			struct ecm_db_connection_instance *cin;

			DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", ci);

			/*
			 * Skip routed CI for brided flows
			 * Skip bridged CI for routed flows
			 */
			if (is_routed != ecm_db_connection_is_routed_get(ci)) {
				goto next_ci;
			}

			/*
			 * Check IP version
			 */
			if (ip_ver != ECM_DB_IP_VERSION_IGNORE && (ecm_db_connection_ip_version_get(ci) != ip_ver)) {
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
			 *
			 */
			ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_FROM, smac);
			ecm_db_connection_node_address_get(ci,  ECM_DB_OBJ_DIR_TO, dmac);
			ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_FROM, sip);
			ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_TO, dip);
			sport = ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_FROM);
			dport = ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_TO);

			/*
			 * 1. For topology A, B, C, D  if drop rule is added in br-home or br-wan
			 * 2. For topoloy  A, B  if drop rule is added in br-wan
			 * Match in flow direction
			 */

			if ((!src_mac_check || ECM_MAC_ADDR_MATCH(smac, src_mac)) &&
			    (!dest_mac_check || ECM_MAC_ADDR_MATCH(dmac, dest_mac)) &&
			    ECM_IP_ADDR_MASK_MATCH(sip, src_addr_mask) &&
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
			if ((!src_mac_check || ECM_MAC_ADDR_MATCH(dmac, src_mac)) &&
			    (!dest_mac_check || ECM_MAC_ADDR_MATCH(smac, dest_mac)) &&
			    ECM_IP_ADDR_MASK_MATCH(dip, src_addr_mask) &&
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

			ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_FROM_NAT, smac);
			ecm_db_connection_node_address_get(ci,  ECM_DB_OBJ_DIR_TO_NAT, dmac);
			ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_FROM_NAT, sip);
			ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_TO_NAT, dip);
			sport = ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_FROM_NAT);
			dport = ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_TO_NAT);

			/*
			 * 1. For topoloy  C, D  if drop rule is added in br-wan
			 * Match in flow direction
			 */
			if ((!src_mac_check || ECM_MAC_ADDR_MATCH(smac, src_mac)) &&
			    (!dest_mac_check || ECM_MAC_ADDR_MATCH(dmac, dest_mac)) &&
			    ECM_IP_ADDR_MASK_MATCH(sip, src_addr_mask) &&
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
			if ((!src_mac_check || ECM_MAC_ADDR_MATCH(dmac, src_mac)) &&
			    (!dest_mac_check || ECM_MAC_ADDR_MATCH(smac, dest_mac)) &&
			    ECM_IP_ADDR_MASK_MATCH(dip, src_addr_mask) &&
			    ECM_IP_ADDR_MASK_MATCH(sip, dest_addr_mask) &&
			    ECM_PORT_MASK_MATCH(dport, src_port_mask) &&
			    ECM_PORT_MASK_MATCH(sport, dest_port_mask)) {
				direction = "reverse (nat)";
				goto defunct_conn;
			}

			goto next_ci;

defunct_conn:
			cnt++;

			DEBUG_TRACE("%px: Defuncting 7 tuple %s connection\n", ci, is_routed ? "routed" : "bridged");
			if (ECM_IP_ADDR_IS_V4(src_addr_mask)) {
				DEBUG_TRACE("%px: Defunct CI masked 7 tuple match(%s) smac=%pM(%d) src=" ECM_IP_ADDR_DOT_FMT " sport=%d "
					    "dmac=%pM(%d) dest=" ECM_IP_ADDR_DOT_FMT ", dport=%d, proto=%d cnt=%d\n", ci, direction, smac,
					    src_mac_check, ECM_IP_ADDR_TO_DOT(sip), sport, dmac, dest_mac_check,
					    ECM_IP_ADDR_TO_DOT(dip), dport, proto, cnt);
			} else {
				DEBUG_TRACE("%px: Defunct CI masked 7 tuple match(%s) src=%pM(%d)" ECM_IP_ADDR_OCTAL_FMT " sport=%d "
					    "dmac=%pM(%d) dest=" ECM_IP_ADDR_OCTAL_FMT ", dport=%d, proto=%d, cnt=%d\n", ci, direction,
					    smac, src_mac_check, ECM_IP_ADDR_TO_OCTAL(sip), sport, dmac, dest_mac_check,
					    ECM_IP_ADDR_TO_OCTAL(dip), dport, proto, cnt);
			}

			ecm_db_connection_make_defunct(ci);
next_ci:
			cin = ecm_db_node_connection_get_and_ref_next(ci, dir);

			ecm_db_connection_deref(ci);
			ci = cin;
		}

		ecm_db_node_deref(ni);
		break;
	}

	DEBUG_TRACE("Completed OVS 7 tuple %s connections (cnt=%d) masked defunct\n",  is_routed ? "routed" : "bridged", cnt);

	if (ECM_IP_ADDR_IS_V4(src_addr_mask)) {
		DEBUG_TRACE("Defunct request by masked 7 tuple smac_mask=%pM(%d) src_mask=" ECM_IP_ADDR_DOT_FMT " sport_mask=%d, "
			    "dmac_mask=%pM(%d) dest_mask=" ECM_IP_ADDR_DOT_FMT " dport_mask=%d, proto_mask=%d\n",  src_mac,
			    src_mac_check, ECM_IP_ADDR_TO_DOT(src_addr_mask), src_port_mask, dest_mac, dest_mac_check,
			    ECM_IP_ADDR_TO_DOT(dest_addr_mask), dest_port_mask, proto_mask);
	} else {
		DEBUG_TRACE("Defunct request by masked 7 tuple smac_mask=%pM(%d) src_mask=" ECM_IP_ADDR_OCTAL_FMT " sport_mask=%d "
			    "dmac_mask=%pM(%d) dest_mask=" ECM_IP_ADDR_OCTAL_FMT " dport_mask=%d, proto_mask=%d cnt=%d\n",
			    src_mac, src_mac_check, ECM_IP_ADDR_TO_OCTAL(src_addr_mask), src_port_mask, dest_mac, dest_mac_check,
			    ECM_IP_ADDR_TO_OCTAL(dest_addr_mask), dest_port_mask, proto_mask, cnt);
	}

}

/*
 * ecm_db_node_ovs_routed_connections_defunct()
 *	Destroy the routed connections created on the node in the given
 *	direction which is related to the ovs_br interface.
 */
void ecm_db_node_ovs_routed_connections_defunct(uint8_t *node_mac, struct net_device *ovs_br, int ip_version, ecm_db_obj_dir_t dir)
{
	struct ecm_db_iface_instance *ii;
	struct ecm_db_node_instance *ni;
	struct ecm_db_connection_instance *ci;

	ii =  ecm_db_iface_find_and_ref_by_interface_identifier(ovs_br->ifindex);
	if (!ii) {
		DEBUG_WARN("%px: Unable to find OVS bridge iface instance\n", ovs_br);
		return;
	}

	/*
	 * Find the node instance which has the node_mac and related to the ovs_br.
	 * Nodes are stored in the database with their related interface instances.
	 */
	ni = ecm_db_node_find_and_ref(node_mac, ii);
	if(!ni) {
		DEBUG_WARN("%px: Unable to find node instance related to %pM and %s\n", ovs_br, node_mac, ovs_br->name);
		ecm_db_iface_deref(ii);
		return;
	}

	/*
	 * Iterate all routed connections on this node in the dir direction.
	 */
	ci = ecm_db_node_connections_get_and_ref_first(ni, dir);
	while (ci) {
		struct ecm_db_connection_instance *cin;

		if (ecm_db_connection_is_routed_get(ci) && (ecm_db_connection_ip_version_get(ci) == ip_version)) {
			DEBUG_TRACE("%px: Defuncting connection %p\n", ovs_br, ci);
			ecm_db_connection_make_defunct(ci);
		}

		cin = ecm_db_node_connection_get_and_ref_next(ci, dir);
		ecm_db_connection_deref(ci);
		ci = cin;
	}

	ecm_db_node_deref(ni);
	ecm_db_iface_deref(ii);

	DEBUG_TRACE("%px: Completed OVS routed connection defunct\n", ovs_br);
}

/*
 * ecm_db_traverse_snode_dnode_connection_list_and_defunct()
 *	Defunct connections between node1 (sni) and node2 (which has dmac address)
 */
void ecm_db_traverse_snode_dnode_connection_list_and_defunct(
	struct ecm_db_node_instance *sni, uint8_t *dmac, int ip_version, ecm_db_obj_dir_t dir)
{
	struct ecm_db_connection_instance *ci = NULL;

	if (dir != ECM_DB_OBJ_DIR_FROM && dir != ECM_DB_OBJ_DIR_TO) {
		DEBUG_WARN("Direction is incorrect: %d\n", dir);
		return;
	}

	/*
	 * Iterate all connection instances which are sent or received from
	 * given node instannce (sni).
	 */
	ci = ecm_db_node_connections_get_and_ref_first(sni, dir);
	while (ci) {
		struct ecm_db_connection_instance *cin;
		struct ecm_db_node_instance *dni;

		/*
		 * Find the connection instance which match given MAC address (dmac)
		 */
		if (dir == ECM_DB_OBJ_DIR_FROM) {
			/*
			 * Direction is FROM, then sni is source node.
			 * find the node in TO direction.
			 */
			dni = ci->node[ECM_DB_OBJ_DIR_TO];
		} else {
			/*
			 * Direction is TO, then sni is destination node.
			 * find the node in FROM direction.
			 */
			dni = ci->node[ECM_DB_OBJ_DIR_FROM];
		}

		/*
		 * if the MAC address of node (dni) match MAC address (dmac)
		 * then delete the connection instance.
		 */
		if (ecm_db_node_is_mac_addr_equal(dni, dmac)) {
			if (ip_version != ECM_DB_IP_VERSION_IGNORE && (ecm_db_connection_ip_version_get(ci) != ip_version)) {
				DEBUG_TRACE("%px: keeping connection, ip_version mismatch %d\n", ci, ci->serial);
				goto keep_sni_conn;
			}

			DEBUG_TRACE("%px: defunct %d\n", ci, ci->serial);
			ecm_db_connection_make_defunct(ci);
		}
keep_sni_conn:
		cin = ecm_db_node_connection_get_and_ref_next(ci, dir);
		ecm_db_connection_deref(ci);
		ci = cin;
	}
	DEBUG_INFO("%px: Defuncting from node connection list complete\n", sni);
}
#endif
#endif

/*
 * ecm_db_node_init()
 */
bool ecm_db_node_init(struct dentry *dentry)
{
	if (!ecm_debugfs_create_u32("node_count", S_IRUGO, dentry,
					(u32 *)&ecm_db_node_count)) {
		DEBUG_ERROR("Failed to create ecm db node count file in debugfs\n");
		return false;
	}

	ecm_db_node_table = vzalloc(sizeof(struct ecm_db_node_instance *) * ECM_DB_NODE_HASH_SLOTS);
	if (!ecm_db_node_table) {
		DEBUG_ERROR("Failed to allocate virtual memory for ecm_db_node_table\n");
		return false;
	}

	ecm_db_node_table_lengths = vzalloc(sizeof(int) * ECM_DB_NODE_HASH_SLOTS);
	if (!ecm_db_node_table_lengths) {
		DEBUG_ERROR("Failed to allocate virtual memory for ecm_db_node_table_lengths\n");
		vfree(ecm_db_node_table);
		return false;
	}

	return true;
}

/*
 * ecm_db_node_exit()
 */
void ecm_db_node_exit(void)
{
	vfree(ecm_db_node_table_lengths);
	vfree(ecm_db_node_table);
}
