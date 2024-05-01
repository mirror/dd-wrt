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
struct ecm_db_host_instance *ecm_db_hosts;

/*
 * Host hash table
 */
#define ECM_DB_HOST_HASH_SLOTS 32768
static struct ecm_db_host_instance **ecm_db_host_table;	/* Slots of the host hash table */
static int *ecm_db_host_table_lengths;			/* Tracks how long each chain is */

static int ecm_db_host_count = 0;	/* Number of hosts allocated */

/*
 * ecm_db_host_generate_hash_index()
 *	Calculate the hash index.
 */
static inline ecm_db_host_hash_t ecm_db_host_generate_hash_index(ip_addr_t address)
{
	uint32_t tuple;
	uint32_t hash_val;

	ECM_IP_ADDR_HASH(tuple, address);
	hash_val = (uint32_t)jhash_1word(tuple, ecm_db_jhash_rnd);
	return (ecm_db_host_hash_t)(hash_val & (ECM_DB_HOST_HASH_SLOTS - 1));
}

/*
 * _ecm_db_host_count_get()
 *	Return the host count (lockless).
 */
int _ecm_db_host_count_get(void)
{
	return ecm_db_host_count;
}

/*
 * _ecm_db_host_ref()
 */
void _ecm_db_host_ref(struct ecm_db_host_instance *hi)
{
	DEBUG_CHECK_MAGIC(hi, ECM_DB_HOST_INSTANCE_MAGIC, "%px: magic failed\n", hi);
	hi->refs++;
	DEBUG_TRACE("%px: host ref %d\n", hi, hi->refs);
	DEBUG_ASSERT(hi->refs > 0, "%px: ref wrap\n", hi);
}

/*
 * ecm_db_host_ref()
 */
void ecm_db_host_ref(struct ecm_db_host_instance *hi)
{
	spin_lock_bh(&ecm_db_lock);
	_ecm_db_host_ref(hi);
	spin_unlock_bh(&ecm_db_lock);
}
EXPORT_SYMBOL(ecm_db_host_ref);

/*
 * ecm_db_hosts_get_and_ref_first()
 *	Obtain a ref to the first host instance, if any
 */
struct ecm_db_host_instance *ecm_db_hosts_get_and_ref_first(void)
{
	struct ecm_db_host_instance *hi;
	spin_lock_bh(&ecm_db_lock);
	hi = ecm_db_hosts;
	if (hi) {
		_ecm_db_host_ref(hi);
	}
	spin_unlock_bh(&ecm_db_lock);
	return hi;
}
EXPORT_SYMBOL(ecm_db_hosts_get_and_ref_first);

/*
 * ecm_db_host_get_and_ref_next()
 *	Return the next host in the list given a host
 */
struct ecm_db_host_instance *ecm_db_host_get_and_ref_next(struct ecm_db_host_instance *hi)
{
	struct ecm_db_host_instance *hin;
	DEBUG_CHECK_MAGIC(hi, ECM_DB_HOST_INSTANCE_MAGIC, "%px: magic failed", hi);
	spin_lock_bh(&ecm_db_lock);
	hin = hi->next;
	if (hin) {
		_ecm_db_host_ref(hin);
	}
	spin_unlock_bh(&ecm_db_lock);
	return hin;
}
EXPORT_SYMBOL(ecm_db_host_get_and_ref_next);

#ifdef ECM_DB_ADVANCED_STATS_ENABLE
/*
 * ecm_db_host_data_stats_get()
 *	Return data stats for the instance
 */
void ecm_db_host_data_stats_get(struct ecm_db_host_instance *hi, uint64_t *from_data_total, uint64_t *to_data_total,
						uint64_t *from_packet_total, uint64_t *to_packet_total,
						uint64_t *from_data_total_dropped, uint64_t *to_data_total_dropped,
						uint64_t *from_packet_total_dropped, uint64_t *to_packet_total_dropped)
{
	DEBUG_CHECK_MAGIC(hi, ECM_DB_HOST_INSTANCE_MAGIC, "%px: magic failed", hi);
	spin_lock_bh(&ecm_db_lock);
	if (from_data_total) {
		*from_data_total = hi->from_data_total;
	}
	if (to_data_total) {
		*to_data_total = hi->to_data_total;
	}
	if (from_packet_total) {
		*from_packet_total = hi->from_packet_total;
	}
	if (to_packet_total) {
		*to_packet_total = hi->to_packet_total;
	}
	if (from_data_total_dropped) {
		*from_data_total_dropped = hi->from_data_total_dropped;
	}
	if (to_data_total_dropped) {
		*to_data_total_dropped = hi->to_data_total_dropped;
	}
	if (from_packet_total_dropped) {
		*from_packet_total_dropped = hi->from_packet_total_dropped;
	}
	if (to_packet_total_dropped) {
		*to_packet_total_dropped = hi->to_packet_total_dropped;
	}
	spin_unlock_bh(&ecm_db_lock);
}
EXPORT_SYMBOL(ecm_db_host_data_stats_get);
#endif

/*
 * ecm_db_host_address_get()
 *	Return address of host
 */
void ecm_db_host_address_get(struct ecm_db_host_instance *hi, ip_addr_t addr)
{
	DEBUG_CHECK_MAGIC(hi, ECM_DB_HOST_INSTANCE_MAGIC, "%px: magic failed", hi);
	ECM_IP_ADDR_COPY(addr, hi->address);
}
EXPORT_SYMBOL(ecm_db_host_address_get);

/*
 * ecm_db_host_on_link_get()
 *	Return on link status of host
 */
bool ecm_db_host_on_link_get(struct ecm_db_host_instance *hi)
{
	DEBUG_CHECK_MAGIC(hi, ECM_DB_HOST_INSTANCE_MAGIC, "%px: magic failed", hi);
	return hi->on_link;
}
EXPORT_SYMBOL(ecm_db_host_on_link_get);

/*
 * ecm_db_host_deref()
 *	Release a ref to a host instance, possibly causing removal from the database and destruction of the instance
 */
int ecm_db_host_deref(struct ecm_db_host_instance *hi)
{
	DEBUG_CHECK_MAGIC(hi, ECM_DB_HOST_INSTANCE_MAGIC, "%px: magic failed\n", hi);

	spin_lock_bh(&ecm_db_lock);
	hi->refs--;
	DEBUG_TRACE("%px: host deref %d\n", hi, hi->refs);
	DEBUG_ASSERT(hi->refs >= 0, "%px: ref wrap\n", hi);

	if (hi->refs > 0) {
		int refs = hi->refs;
		spin_unlock_bh(&ecm_db_lock);
		return refs;
	}

#ifdef ECM_DB_XREF_ENABLE
	DEBUG_ASSERT((hi->mappings == NULL) && (hi->mapping_count == 0), "%px: mappings not null\n", hi);
#endif

	/*
	 * Remove from database if inserted
	 */
	if (!hi->flags & ECM_DB_HOST_FLAGS_INSERTED) {
		spin_unlock_bh(&ecm_db_lock);
	} else {
		struct ecm_db_listener_instance *li;

		/*
		 * Remove from the global list
		 */
		if (!hi->prev) {
			DEBUG_ASSERT(ecm_db_hosts == hi, "%px: host table bad\n", hi);
			ecm_db_hosts = hi->next;
		} else {
			hi->prev->next = hi->next;
		}
		if (hi->next) {
			hi->next->prev = hi->prev;
		}
		hi->prev = NULL;
		hi->next = NULL;

		/*
		 * Unlink it from the host hash table
		 */
		if (!hi->hash_prev) {
			DEBUG_ASSERT(ecm_db_host_table[hi->hash_index] == hi, "%px: hash table bad\n", hi);
			ecm_db_host_table[hi->hash_index] = hi->hash_next;
		} else {
			hi->hash_prev->hash_next = hi->hash_next;
		}
		if (hi->hash_next) {
			hi->hash_next->hash_prev = hi->hash_prev;
		}
		hi->hash_next = NULL;
		hi->hash_prev = NULL;
		ecm_db_host_table_lengths[hi->hash_index]--;
		DEBUG_ASSERT(ecm_db_host_table_lengths[hi->hash_index] >= 0, "%px: invalid table len %d\n", hi, ecm_db_host_table_lengths[hi->hash_index]);

		spin_unlock_bh(&ecm_db_lock);

		/*
		 * Throw removed event to listeners
		 */
		DEBUG_TRACE("%px: Throw host removed event\n", hi);
		li = ecm_db_listeners_get_and_ref_first();
		while (li) {
			struct ecm_db_listener_instance *lin;
			if (li->host_removed) {
				li->host_removed(li->arg, hi);
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
	if (hi->final) {
		hi->final(hi->arg);
	}

	/*
	 * We can now destroy the instance
	 */
	DEBUG_CLEAR_MAGIC(hi);
	kfree(hi);

	/*
	 * Decrease global host count
	 */
	spin_lock_bh(&ecm_db_lock);
	ecm_db_host_count--;
	DEBUG_ASSERT(ecm_db_host_count >= 0, "%px: host count wrap\n", hi);
	spin_unlock_bh(&ecm_db_lock);

	return 0;
}
EXPORT_SYMBOL(ecm_db_host_deref);

/*
 * ecm_db_host_find_and_ref()
 *	Lookup and return a host reference if any
 */
struct ecm_db_host_instance *ecm_db_host_find_and_ref(ip_addr_t address)
{
	ecm_db_host_hash_t hash_index;
	struct ecm_db_host_instance *hi;

	DEBUG_TRACE("Lookup host with addr " ECM_IP_ADDR_OCTAL_FMT "\n", ECM_IP_ADDR_TO_OCTAL(address));

	/*
	 * Compute the hash chain index and prepare to walk the chain
	 */
	hash_index = ecm_db_host_generate_hash_index(address);

	/*
	 * Iterate the chain looking for a host with matching details
	 */
	spin_lock_bh(&ecm_db_lock);
	hi = ecm_db_host_table[hash_index];
	while (hi) {
		if (!ECM_IP_ADDR_MATCH(hi->address, address)) {
			hi = hi->hash_next;
			continue;
		}

		_ecm_db_host_ref(hi);
		spin_unlock_bh(&ecm_db_lock);
		DEBUG_TRACE("host found %px\n", hi);
		return hi;
	}
	spin_unlock_bh(&ecm_db_lock);
	DEBUG_TRACE("Host not found\n");
	return NULL;
}
EXPORT_SYMBOL(ecm_db_host_find_and_ref);

#ifdef ECM_DB_XREF_ENABLE
/*
 * ecm_db_host_mappings_get_and_ref_first()
 *	Return a reference to the first mapping of this host
 */
static struct ecm_db_mapping_instance *ecm_db_host_mappings_get_and_ref_first(struct ecm_db_host_instance *hi)
{
	struct ecm_db_mapping_instance *mi = NULL;

	DEBUG_CHECK_MAGIC(hi, ECM_DB_HOST_INSTANCE_MAGIC, "%p: magic failed", hi);

	spin_lock_bh(&ecm_db_lock);
	mi = hi->mappings;
	if (mi) {
		_ecm_db_mapping_ref(mi);
	}
	spin_unlock_bh(&ecm_db_lock);

	return mi;
}

/*
 * ecm_db_host_mapping_get_and_ref_next()
 *	Return the next host mapping in the list given a mapping
 */
static struct ecm_db_mapping_instance *ecm_db_host_mapping_get_and_ref_next(struct ecm_db_mapping_instance *mi)
{
	struct ecm_db_mapping_instance *nmi = NULL;

	DEBUG_CHECK_MAGIC(mi, ECM_DB_MAPPING_INSTANCE_MAGIC, "%p: magic failed", mi);

	spin_lock_bh(&ecm_db_lock);
	nmi = mi->mapping_next;
	if (nmi) {
		_ecm_db_mapping_ref(nmi);
	}
	spin_unlock_bh(&ecm_db_lock);

	return nmi;
}

/*
 * ecm_db_host_connections_defunct_by_dir()
 *	Defunct the connections associated with the IP address in the given direction.
 */
void ecm_db_host_connections_defunct_by_dir(ip_addr_t addr, ecm_db_obj_dir_t dir)
{
	struct ecm_db_host_instance *hi;
	struct ecm_db_mapping_instance *mi;

	hi = ecm_db_host_find_and_ref(addr);
	if (!hi) {
		DEBUG_WARN("Unable to find host instance\n");
		return;
	}

	DEBUG_CHECK_MAGIC(hi, ECM_DB_HOST_INSTANCE_MAGIC, "%px: magic failed\n", hi);

	mi = ecm_db_host_mappings_get_and_ref_first(hi);
	while (mi) {
		struct ecm_db_connection_instance *ci;
		struct ecm_db_mapping_instance *min;

		ci = ecm_db_mapping_connections_get_and_ref_first(mi, dir);
		while (ci) {
			struct ecm_db_connection_instance *cin;
			DEBUG_TRACE("%px: defunct %d\n", ci, ci->serial);
			ecm_db_connection_make_defunct(ci);

			cin = ecm_db_connection_mapping_get_and_ref_next(ci, dir);
			ecm_db_connection_deref(ci);
			ci = cin;
		}

		min = ecm_db_host_mapping_get_and_ref_next(mi);
		ecm_db_mapping_deref(mi);
		mi = min;
	}

	if (ECM_IP_ADDR_IS_V4(addr)) {
		DEBUG_INFO("%px: Defuncting connections %s " ECM_IP_ADDR_DOT_FMT "\n",
			   hi, ecm_db_obj_dir_strings[dir], ECM_IP_ADDR_TO_DOT(addr));
	} else {
		DEBUG_INFO("%px: Defuncting connections %s " ECM_IP_ADDR_OCTAL_FMT "\n",
			   hi, ecm_db_obj_dir_strings[dir], ECM_IP_ADDR_TO_OCTAL(addr));
	}
}

/*
 * ecm_db_host_connections_defunct_by_src_and_dest()
 *	Defunct the connections with the given source and destination IP addresses.
 */
void ecm_db_host_connections_defunct_by_src_and_dest(ip_addr_t src_addr, ip_addr_t dest_addr)
{
	struct ecm_db_host_instance *src_hi;
	struct ecm_db_mapping_instance *src_mi;

	src_hi = ecm_db_host_find_and_ref(src_addr);
	if (!src_hi) {
		DEBUG_WARN("Unable to find host instance\n");
		return;
	}

	DEBUG_CHECK_MAGIC(src_hi, ECM_DB_HOST_INSTANCE_MAGIC, "%px: magic failed\n", src_hi);

	src_mi = ecm_db_host_mappings_get_and_ref_first(src_hi);
	while (src_mi) {
		struct ecm_db_connection_instance *ci;
		struct ecm_db_mapping_instance *src_min;

		ci = ecm_db_mapping_connections_get_and_ref_first(src_mi, ECM_DB_OBJ_DIR_FROM);
		while (ci) {
			struct ecm_db_connection_instance *cin;
			ip_addr_t dst_ip;

			ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_TO, dst_ip);

			if (ECM_IP_ADDR_MATCH(dest_addr, dst_ip)) {
				DEBUG_TRACE("%px: defunct %d\n", ci, ci->serial);
				ecm_db_connection_make_defunct(ci);
			}

			cin = ecm_db_connection_mapping_get_and_ref_next(ci, ECM_DB_OBJ_DIR_FROM);
			ecm_db_connection_deref(ci);
			ci = cin;
		}

		src_min = ecm_db_host_mapping_get_and_ref_next(src_mi);
		ecm_db_mapping_deref(src_mi);
		src_mi = src_min;
	}

	if (ECM_IP_ADDR_IS_V4(src_addr)) {
		DEBUG_INFO("%px: Defuncting connections from " ECM_IP_ADDR_DOT_FMT "to " ECM_IP_ADDR_DOT_FMT "\n",
			   src_hi, ECM_IP_ADDR_TO_DOT(src_addr), ECM_IP_ADDR_TO_DOT(dest_addr));
	} else {
		DEBUG_INFO("%px: Defuncting connections from " ECM_IP_ADDR_OCTAL_FMT "to " ECM_IP_ADDR_OCTAL_FMT "\n",
			   src_hi, ECM_IP_ADDR_TO_OCTAL(src_addr), ECM_IP_ADDR_TO_OCTAL(dest_addr));
	}
}

/*
 * ecm_db_host_mapping_count_get()
 *	Return the number of mappings to this host
 */
int ecm_db_host_mapping_count_get(struct ecm_db_host_instance *hi)
{
	int count;

	DEBUG_CHECK_MAGIC(hi, ECM_DB_HOST_INSTANCE_MAGIC, "%px: magic failed\n", hi);

	spin_lock_bh(&ecm_db_lock);
	count = hi->mapping_count;
	spin_unlock_bh(&ecm_db_lock);
	return count;
}
EXPORT_SYMBOL(ecm_db_host_mapping_count_get);
#endif

/*
 * ecm_db_host_add()
 *	Add a host instance into the database
 */
void ecm_db_host_add(struct ecm_db_host_instance *hi, ip_addr_t address, bool on_link, ecm_db_host_final_callback_t final, void *arg)
{
	ecm_db_host_hash_t hash_index;
	struct ecm_db_listener_instance *li;

	spin_lock_bh(&ecm_db_lock);
	DEBUG_CHECK_MAGIC(hi, ECM_DB_HOST_INSTANCE_MAGIC, "%px: magic failed\n", hi);
	DEBUG_ASSERT(!(hi->flags & ECM_DB_HOST_FLAGS_INSERTED), "%px: inserted\n", hi);
#ifdef ECM_DB_XREF_ENABLE
	DEBUG_ASSERT((hi->mappings == NULL) && (hi->mapping_count == 0), "%px: mappings not null\n", hi);
#endif
	spin_unlock_bh(&ecm_db_lock);

	hi->arg = arg;
	hi->final = final;
	ECM_IP_ADDR_COPY(hi->address, address);
	hi->on_link = on_link;

	/*
	 * Compute hash index into which host will be added
	 */
	hash_index = ecm_db_host_generate_hash_index(address);
	hi->hash_index = hash_index;

	/*
	 * Add into the global list
	 */
	spin_lock_bh(&ecm_db_lock);
	hi->flags |= ECM_DB_HOST_FLAGS_INSERTED;
	hi->prev = NULL;
	hi->next = ecm_db_hosts;
	if (ecm_db_hosts) {
		ecm_db_hosts->prev = hi;
	}
	ecm_db_hosts = hi;

	/*
	 * Add host into the hash table
	 */
	hi->hash_prev = NULL;
	hi->hash_next = ecm_db_host_table[hash_index];
	if (ecm_db_host_table[hash_index]) {
		ecm_db_host_table[hash_index]->hash_prev = hi;
	}
	ecm_db_host_table[hash_index] = hi;
	ecm_db_host_table_lengths[hash_index]++;
	DEBUG_ASSERT(ecm_db_host_table_lengths[hash_index] > 0, "%px: invalid table len %d\n", hi, ecm_db_host_table_lengths[hash_index]);

	/*
	 * Set time of add
	 */
	hi->time_added = ecm_db_time;
	spin_unlock_bh(&ecm_db_lock);

	/*
	 * Throw add event to the listeners
	 */
	DEBUG_TRACE("%px: Throw host added event\n", hi);
	li = ecm_db_listeners_get_and_ref_first();
	while (li) {
		struct ecm_db_listener_instance *lin;
		if (li->host_added) {
			li->host_added(li->arg, hi);
		}

		/*
		 * Get next listener
		 */
		lin = ecm_db_listener_get_and_ref_next(li);
		ecm_db_listener_deref(li);
		li = lin;
	}
}
EXPORT_SYMBOL(ecm_db_host_add);

/*
 * ecm_db_host_state_get()
 *	Prepare a host message
 */
int ecm_db_host_state_get(struct ecm_state_file_instance *sfi, struct ecm_db_host_instance *hi)
{
	int result;
	char address[ECM_IP_ADDR_STR_BUFF_SIZE];
#ifdef ECM_DB_XREF_ENABLE
	int mapping_count;
#endif
	uint32_t time_added;
	bool on_link;
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

	DEBUG_TRACE("Prep host msg for %px\n", hi);

	/*
	 * Create a small xml stats element for our host.
	 * Extract information from the host for inclusion into the message
	 */
#ifdef ECM_DB_XREF_ENABLE
	mapping_count = ecm_db_host_mapping_count_get(hi);
#endif
	ecm_ip_addr_to_string(address, hi->address);
	time_added = hi->time_added;
	on_link = hi->on_link;

#ifdef ECM_DB_ADVANCED_STATS_ENABLE
	ecm_db_host_data_stats_get(hi, &from_data_total, &to_data_total,
			&from_packet_total, &to_packet_total,
			&from_data_total_dropped, &to_data_total_dropped,
			&from_packet_total_dropped, &to_packet_total_dropped);
#endif

	if ((result = ecm_state_prefix_add(sfi, "host"))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "address", "%s", address))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "time_added", "%u", time_added))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "on_link", "%d", on_link))) {
		return result;
	}

#ifdef ECM_DB_XREF_ENABLE
	if ((result = ecm_state_write(sfi, "mappings", "%d", mapping_count))) {
		return result;
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
EXPORT_SYMBOL(ecm_db_host_state_get);

/*
 * ecm_db_host_hash_table_lengths_get()
 *	Return hash table length
 */
int ecm_db_host_hash_table_lengths_get(int index)
{
	int length;

	DEBUG_ASSERT((index >= 0) && (index < ECM_DB_HOST_HASH_SLOTS), "Bad protocol: %d\n", index);
	spin_lock_bh(&ecm_db_lock);
	length = ecm_db_host_table_lengths[index];
	spin_unlock_bh(&ecm_db_lock);
	return length;
}
EXPORT_SYMBOL(ecm_db_host_hash_table_lengths_get);

/*
 * ecm_db_host_hash_index_get_next()
 * Given a hash index, return the next one OR return -1 for no more hash indicies to return.
 */
int ecm_db_host_hash_index_get_next(int index)
{
	index++;
	if (index >= ECM_DB_HOST_HASH_SLOTS) {
		return -1;
	}
	return index;
}
EXPORT_SYMBOL(ecm_db_host_hash_index_get_next);

/*
 * ecm_db_host_hash_index_get_first()
 * Return first hash index
 */
int ecm_db_host_hash_index_get_first(void)
{
	return 0;
}
EXPORT_SYMBOL(ecm_db_host_hash_index_get_first);

/*
 * ecm_db_host_alloc()
 *	Allocate a host instance
 */
struct ecm_db_host_instance *ecm_db_host_alloc(void)
{
	struct ecm_db_host_instance *hi;
	hi = (struct ecm_db_host_instance *)kzalloc(sizeof(struct ecm_db_host_instance), GFP_ATOMIC | __GFP_NOWARN);
	if (!hi) {
		DEBUG_WARN("Alloc failed\n");
		return NULL;
	}

	hi->refs = 1;
	DEBUG_SET_MAGIC(hi, ECM_DB_HOST_INSTANCE_MAGIC);

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
		kfree(hi);
		return NULL;
	}

	ecm_db_host_count++;
	spin_unlock_bh(&ecm_db_lock);

	DEBUG_TRACE("Host created %px\n", hi);
	return hi;
}
EXPORT_SYMBOL(ecm_db_host_alloc);

/*
 * ecm_db_host_init()
 */
bool ecm_db_host_init(struct dentry *dentry)
{
	if (!ecm_debugfs_create_u32("host_count", S_IRUGO, dentry,
					(u32 *)&ecm_db_host_count)) {
		DEBUG_ERROR("Failed to create ecm db host count file in debugfs\n");
		return false;;
	}

	ecm_db_host_table = vzalloc(sizeof(struct ecm_db_host_instance *) * ECM_DB_HOST_HASH_SLOTS);
	if (!ecm_db_host_table) {
		DEBUG_ERROR("Failed to allocate virtual memory for ecm_db_host_table\n");
		return false;
	}

	ecm_db_host_table_lengths = vzalloc(sizeof(int) * ECM_DB_HOST_HASH_SLOTS);
	if (!ecm_db_host_table_lengths) {
		DEBUG_ERROR("Failed to allocate virtual memory for ecm_db_host_table_lengths\n");
		vfree(ecm_db_host_table);
		return false;
	}

	return true;
}

/*
 * ecm_db_host_exit()
 */
void ecm_db_host_exit(void)
{
	vfree(ecm_db_host_table_lengths);
	vfree(ecm_db_host_table);
}
