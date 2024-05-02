/*
 **************************************************************************
 * Copyright (c) 2014-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2024 Qualcomm Innovation Center, Inc. All rights reserved.
 *
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

#ifdef ECM_MULTICAST_ENABLE
#define ECM_DB_MULTICAST_INSTANCE_MAGIC 0xc34a

#define ECM_DB_MULTICAST_TUPLE_INSTANCE_HASH_SLOTS 16

typedef uint32_t ecm_db_multicast_tuple_instance_hash_t;

/*
 * struct ecm_db_multicast_tuple_instance
 * 	Tuple information for an accelerated multicast connection.
 * 	This tuple information is further used to find an attached
 * 	connection for the multicast flow.
 */
struct ecm_db_multicast_tuple_instance {
	struct ecm_db_multicast_tuple_instance *next;	/* Next instance in global list */
	struct ecm_db_multicast_tuple_instance *prev;	/* Previous instance in global list */
	struct ecm_db_connection_instance *ci;	/* Pointer to the DB Connection Instance */
#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
	struct vlan_hdr ovs_ingress_vlan;	/* Ingress vlan tag for ovs bridge. */
#endif
	uint16_t src_port;	/* RO: IPv4/v6 Source Port */
	uint16_t dst_port;	/* RO: IPv4/v6 Destination Port */
	ip_addr_t src_ip;	/* RO: IPv4/v6 Source Address */
	ip_addr_t grp_ip;	/* RO: IPv4/v6 Multicast Group Address */
	uint32_t flags;		/* Flags for this instance node */
	uint32_t hash_index;	/* Hash index of this node */
	int proto;		/* RO: Protocol */
	struct net_device *l2_br_dev;		/* Bridge device for L2-only flows */
	struct net_device *l3_br_dev;		/* Bridge device for L3-only flows */
	int refs;		/* Integer to trap we never go negative */
#if (DEBUG_LEVEL > 0)
	uint16_t magic;		/* Magic value for debug */
#endif
};

/*
 * Multicast connection tuple table
 * 	This table is used to lookup a complete tuple for multicast connections
 * 	using the multicast group address
 */
static struct ecm_db_multicast_tuple_instance *ecm_db_multicast_tuple_instance_table[ECM_DB_MULTICAST_TUPLE_INSTANCE_HASH_SLOTS];

/*
 * ecm_db_multicast_connection_data_totals_update()
 *	Update the total bytes and packets sent/received by the multicast connection
 *	TODO: This function is almost similar to unicast connection_data_totals_update() except few
 *	      lines of code. The next merge should have a common logic for both unicast and multicast.
 */
void ecm_db_multicast_connection_data_totals_update(struct ecm_db_connection_instance *ci, bool is_from, uint64_t size, uint64_t packets)
{
	int32_t i;

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
EXPORT_SYMBOL(ecm_db_multicast_connection_data_totals_update);

/*
 * ecm_db_multicast_connection_interface_heirarchy_stats_update()
 * 	Traverse through the multicast destination interface heirarchy and update the stats (data and packets).
 */
void ecm_db_multicast_connection_interface_heirarchy_stats_update(struct ecm_db_connection_instance *ci, uint64_t size, uint64_t packets)
{
	struct ecm_db_iface_instance *to_mc_ifaces;
	struct ecm_db_iface_instance *ii;
	struct ecm_db_iface_instance **ifaces;
	struct ecm_db_iface_instance *ii_temp;
	int32_t *to_mc_ifaces_first;
	int heirarchy_index;
	int ret;

	ret = ecm_db_multicast_connection_to_interfaces_get_and_ref_all(ci, &to_mc_ifaces, &to_mc_ifaces_first);
	if (ret == 0) {
		DEBUG_WARN("%px: no interfaces in to_multicast_interfaces list!\n", ci);
		return;
	}

	spin_lock_bh(&ecm_db_lock);
	for (heirarchy_index = 0; heirarchy_index < ECM_DB_MULTICAST_IF_MAX; heirarchy_index++) {

		if (to_mc_ifaces_first[heirarchy_index] < ECM_DB_IFACE_HEIRARCHY_MAX) {
			ii_temp = ecm_db_multicast_if_heirarchy_get(to_mc_ifaces, heirarchy_index);
			ii_temp = ecm_db_multicast_if_instance_get_at_index(ii_temp, ECM_DB_IFACE_HEIRARCHY_MAX - 1);
			ifaces = (struct ecm_db_iface_instance **)ii_temp;
			ii = *ifaces;
			ii->to_data_total += size;
			ii->to_packet_total += packets;
		}
	}
	spin_unlock_bh(&ecm_db_lock);

	ecm_db_multicast_connection_to_interfaces_deref_all(to_mc_ifaces, to_mc_ifaces_first);
}
EXPORT_SYMBOL(ecm_db_multicast_connection_interface_heirarchy_stats_update);

/*
 * _ecm_db_multicast_tuple_instance_deref()
 * 	Deref the reference count or
 * 	Free the tuple_instance struct, when the multicast connection dies
 */
int _ecm_db_multicast_tuple_instance_deref(struct ecm_db_multicast_tuple_instance *ti)
{
	DEBUG_CHECK_MAGIC(ti, ECM_DB_MULTICAST_INSTANCE_MAGIC, "%px: magic failed", ti);
	ti->refs--;
	DEBUG_TRACE("%px: ti deref %d\n", ti, ti->refs);
	DEBUG_ASSERT(ti->refs >= 0, "%px: ref wrap\n", ti);

	if (ti->refs > 0) {
		return ti->refs;
	}

	if (ti->flags & ECM_DB_MULTICAST_TUPLE_INSTANCE_FLAGS_INSERTED) {
		if (!ti->prev) {
			DEBUG_ASSERT(ecm_db_multicast_tuple_instance_table[ti->hash_index] == ti, "%px: hash table bad\n", ti);
			ecm_db_multicast_tuple_instance_table[ti->hash_index] = ti->next;
		} else {
			ti->prev->next = ti->next;
		}

		if (ti->next) {
			ti->next->prev = ti->prev;
		}
	}

	if (ti->l2_br_dev) {
		dev_put(ti->l2_br_dev);
	}

	if (ti->l3_br_dev) {
		dev_put(ti->l3_br_dev);
	}

	DEBUG_CLEAR_MAGIC(ti);
	kfree(ti);

	return 0;
}

/*
 * ecm_db_multicast_generate_hash_index()
 * 	Calculate the hash index given a multicast group address.
 */
static inline ecm_db_multicast_tuple_instance_hash_t ecm_db_multicast_generate_hash_index(ip_addr_t address)
{
	uint32_t temp;
	uint32_t hash_val;

	if (ECM_IP_ADDR_IS_V4(address)){
		temp = (uint32_t)address[0];
	} else {
		temp = (uint32_t)address[3];
	}

	hash_val = (uint32_t)jhash_1word(temp, ecm_db_jhash_rnd);

	return (ecm_db_multicast_tuple_instance_hash_t)(hash_val & (ECM_DB_MULTICAST_TUPLE_INSTANCE_HASH_SLOTS - 1));
}

/*
 * ecm_db_multicast_connection_to_interfaces_reset()
 *	Reset the 'to' interfaces heirarchy with a new set of destination interfaces for
 *	the multicast connection
 */
int ecm_db_multicast_connection_to_interfaces_reset(struct ecm_db_connection_instance *ci, struct ecm_db_iface_instance *interfaces, int32_t *new_first)
{
	struct ecm_db_iface_instance *ii_temp;
	struct ecm_db_iface_instance *ii_single;
	struct ecm_db_iface_instance **ifaces;
	struct ecm_db_iface_instance *ii_db;
	struct ecm_db_iface_instance *ii_db_single;
	struct ecm_db_iface_instance **ifaces_db;
	int32_t *nf_p;
	int32_t heirarchy_index;
	int32_t i;
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed\n", ci);

	/*
	 * First remove all old interface hierarchies if any hierarchy
	 * uphold in the ci->to_mcast_interfaces.
	 */
	ecm_db_multicast_connection_to_interfaces_clear(ci);

	ci->to_mcast_interfaces = (struct ecm_db_iface_instance *)kzalloc(ECM_DB_TO_MCAST_INTERFACES_SIZE, GFP_ATOMIC | __GFP_NOWARN);
	if (!ci->to_mcast_interfaces) {
		DEBUG_WARN("%px: Memory is not available for to_mcast_interfaces\n", ci);
		return -1;
	}

	/*
	 * Iterate the to interface list and add the new interface hierarchies
	 */
	spin_lock_bh(&ecm_db_lock);

	for (heirarchy_index = 0; heirarchy_index < ECM_DB_MULTICAST_IF_MAX; heirarchy_index++) {
		ii_temp = ecm_db_multicast_if_heirarchy_get(interfaces, heirarchy_index);
		nf_p = ecm_db_multicast_if_first_get_at_index(new_first, heirarchy_index);

		if (*nf_p == ECM_DB_IFACE_HEIRARCHY_MAX) {
			continue;
		}

		for (i = *nf_p; i < ECM_DB_IFACE_HEIRARCHY_MAX; ++i) {

			/*
			 * Store valid dest interface list into DB connection
			 */
			ii_single = ecm_db_multicast_if_instance_get_at_index(ii_temp, i);
			ifaces = (struct ecm_db_iface_instance **)ii_single;

			ii_db = ecm_db_multicast_if_heirarchy_get(ci->to_mcast_interfaces, heirarchy_index);
			ii_db_single = ecm_db_multicast_if_instance_get_at_index(ii_db, i);
			ifaces_db = (struct ecm_db_iface_instance **)ii_db_single;

			*ifaces_db = *ifaces;
			_ecm_db_iface_ref(*ifaces_db);
		}
	}

	/*
	 * Update the first indices
	 */
	for (heirarchy_index = 0; heirarchy_index < ECM_DB_MULTICAST_IF_MAX; heirarchy_index++) {
		nf_p = ecm_db_multicast_if_first_get_at_index(new_first, heirarchy_index);
		ci->to_mcast_interface_first[heirarchy_index] = *nf_p;
	}

	ci->to_mcast_interfaces_set = true;
	spin_unlock_bh(&ecm_db_lock);

	return 0;
}
EXPORT_SYMBOL(ecm_db_multicast_connection_to_interfaces_reset);

/*
 *  ecm_db_multicast_connection_to_interfaces_update()
 *  	Merge the latest valid multicast destination interfaces into DB Connection
 *  	instance. The new list holds the updated list of interfaces for the multicast
 *  	connection, due to JOIN updates.
 */
void ecm_db_multicast_connection_to_interfaces_update(struct ecm_db_connection_instance *ci,
			struct ecm_db_iface_instance *interfaces, int32_t *mc_join_first, int32_t *mc_join_valid_idx)
{
	struct ecm_db_iface_instance *ii_temp;
	struct ecm_db_iface_instance *ii_single;
	struct ecm_db_iface_instance **ifaces;
	struct ecm_db_iface_instance *ii_db;
	struct ecm_db_iface_instance *ii_db_single;
	struct ecm_db_iface_instance **ifaces_db;
	int32_t *join_first;
	int32_t *join_idx;
	int heirarchy_index;
	int32_t if_index;
	int32_t i;

	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed\n", ci);

	/*
	 * Iterate the to interface list, adding in the new
	 */
	spin_lock_bh(&ecm_db_lock);
	for (heirarchy_index = 0, if_index = 0; heirarchy_index < ECM_DB_MULTICAST_IF_MAX; heirarchy_index++) {
		ii_temp = ecm_db_multicast_if_heirarchy_get(interfaces, if_index);
		join_first = ecm_db_multicast_if_first_get_at_index(mc_join_first, if_index);
		join_idx = ecm_db_multicast_if_num_get_at_index(mc_join_valid_idx, heirarchy_index);

		if (*join_idx == 0) {

			/*
			 * No update for the interface at this index
			 */
			continue;
		}

		/*
		 * This interface has joined the group. Add it to the list.
		 */
		if (*join_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
			if_index++;
			continue;
		}

		ci->to_mcast_interface_first[heirarchy_index] = *join_first;
		for (i = *join_first; i < ECM_DB_IFACE_HEIRARCHY_MAX; ++i) {

			/*
			 * Store valid dest interface list into DB connection
			 */
			ii_single = ecm_db_multicast_if_instance_get_at_index(ii_temp, i);
			ifaces = (struct ecm_db_iface_instance **)ii_single;
			ii_db = ecm_db_multicast_if_heirarchy_get(ci->to_mcast_interfaces, heirarchy_index);
			ii_db_single = ecm_db_multicast_if_instance_get_at_index(ii_db, i);
			ifaces_db = (struct ecm_db_iface_instance **)ii_db_single;
			*ifaces_db = *ifaces;
			_ecm_db_iface_ref(*ifaces_db);
		}
		if_index++;
	}
	spin_unlock_bh(&ecm_db_lock);
	return;
}
EXPORT_SYMBOL(ecm_db_multicast_connection_to_interfaces_update);

/*
 *  _ecm_db_multicast_tuple_instance_ref()
 * 	Increment tuple reference count by one
 */
static void _ecm_db_multicast_tuple_instance_ref(struct ecm_db_multicast_tuple_instance *ti)
{
	DEBUG_CHECK_MAGIC(ti, ECM_DB_MULTICAST_INSTANCE_MAGIC, "%px: magic failed", ti);
	ti->refs++;
	DEBUG_TRACE("%px: ti ref %d\n", ti, ti->refs);
	DEBUG_ASSERT(ti->refs > 0, "%px: ref wrap\n", ti)
}

/*
 * ecm_db_multicast_alloc_connection()
 * 	Allocate memory for the connection structure.
 */
struct ecm_db_multicast_tuple_instance *ecm_db_multicast_tuple_instance_alloc(ip_addr_t origin, ip_addr_t group, uint16_t src_port, uint16_t dst_port)
{
	struct ecm_db_multicast_tuple_instance *ti;
	ti = (struct ecm_db_multicast_tuple_instance *)kzalloc(sizeof(struct ecm_db_multicast_tuple_instance), GFP_ATOMIC | __GFP_NOWARN);
	if (!ti) {
		DEBUG_WARN("ti: Alloc failed\n");
		return NULL;
	}
	ti->src_port = src_port;
	ti->dst_port = dst_port;
	ECM_IP_ADDR_COPY(ti->src_ip, origin);
	ECM_IP_ADDR_COPY(ti->grp_ip, group);
	ti->proto = IPPROTO_UDP;
	ti->hash_index = ecm_db_multicast_generate_hash_index(group);
	ti->flags = 0;
	ti->refs = 1;
	ti->next = NULL;
	ti->prev = NULL;
	ti->l2_br_dev = NULL;
	ti->l3_br_dev = NULL;
#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
	ti->ovs_ingress_vlan.h_vlan_TCI = 0;
	ti->ovs_ingress_vlan.h_vlan_encapsulated_proto = 0;
#endif
	DEBUG_SET_MAGIC(ti, ECM_DB_MULTICAST_INSTANCE_MAGIC);

	return ti;
}
EXPORT_SYMBOL(ecm_db_multicast_tuple_instance_alloc);

/*
 * ecm_db_multicast_connection_find_and_ref()
 * 	Called by MFC event update to fetch connection from the table
 * 	This function takes a ref count for both tuple_instance and 'ci'
 *	Call ecm_db_multicast_connection_deref function for deref both
 *	'ti' and 'ci'
 */
struct ecm_db_multicast_tuple_instance *ecm_db_multicast_connection_find_and_ref(ip_addr_t origin, ip_addr_t group)
{
	ecm_db_multicast_tuple_instance_hash_t hash_index;
	struct ecm_db_multicast_tuple_instance *ti;

	/*
	 * Compute the hash chain index
	 */
	hash_index = ecm_db_multicast_generate_hash_index(group);

	spin_lock_bh(&ecm_db_lock);
	ti = ecm_db_multicast_tuple_instance_table[hash_index];

	/*
	 * Traverse through the list and find the ti
	 */
	while (ti) {
		if (!(ECM_IP_ADDR_MATCH(ti->src_ip, origin) && ECM_IP_ADDR_MATCH(ti->grp_ip, group))) {
			ti = ti->next;
			continue;
		}

		_ecm_db_multicast_tuple_instance_ref(ti);
		_ecm_db_connection_ref(ti->ci);
		spin_unlock_bh(&ecm_db_lock);
		DEBUG_TRACE("multicast tuple instance found %px\n", ti);
		return ti;
	}

	spin_unlock_bh(&ecm_db_lock);
	DEBUG_TRACE("multicast tuple instance not found\n");
	return NULL;
}
EXPORT_SYMBOL(ecm_db_multicast_connection_find_and_ref);

/*
 * ecm_db_multicast_tuple_instance_deref()
 * 	Deref the reference count or
 * 	Free the tuple_instance struct, when the multicast connection dies
 */
int ecm_db_multicast_tuple_instance_deref(struct ecm_db_multicast_tuple_instance *ti)
{
	int refs;
	spin_lock_bh(&ecm_db_lock);
	refs = _ecm_db_multicast_tuple_instance_deref(ti);
	spin_unlock_bh(&ecm_db_lock);
	return refs;
}
EXPORT_SYMBOL(ecm_db_multicast_tuple_instance_deref);

/*
 * ecm_db_multicast_connection_deref()
 * 	Deref both 'ti' and 'ci'
 * 	call this function after ecm_db_multicast_connection_find_and_ref()
 */
void ecm_db_multicast_connection_deref(struct ecm_db_multicast_tuple_instance *ti)
{
	struct ecm_db_connection_instance *ci;
	DEBUG_CHECK_MAGIC(ti, ECM_DB_MULTICAST_INSTANCE_MAGIC, "%px: magic failed", ti);

	ci = ti->ci;
	ecm_db_multicast_tuple_instance_deref(ti);
	ecm_db_connection_deref(ci);

}
EXPORT_SYMBOL(ecm_db_multicast_connection_deref);

/*
 * ecm_db_multicast_tuple_instance_add()
 * 	Add the tuple instance into the hash table. Also, attach the tuple instance
 * 	with connection instance.
 *
 * 	Note: This function takes a reference count and caller has to also call
 * 	ecm_db_multicast_tuple_instance_deref() after this function.
 */
void ecm_db_multicast_tuple_instance_add(struct ecm_db_multicast_tuple_instance *ti, struct ecm_db_connection_instance *ci)
{
	DEBUG_CHECK_MAGIC(ti, ECM_DB_MULTICAST_INSTANCE_MAGIC, "%px: magic failed", ti);

	spin_lock_bh(&ecm_db_lock);
	DEBUG_ASSERT(!(ti->flags & ECM_DB_MULTICAST_TUPLE_INSTANCE_FLAGS_INSERTED), "%px: inserted\n", ti);

	/*
	 * Attach the multicast tuple instance with the connection instance
	 */
	ci->ti = ti;
	ti->ci = ci;

	/*
	 * Take a local reference to ti
	 */
	_ecm_db_multicast_tuple_instance_ref(ti);
	ti->next = ecm_db_multicast_tuple_instance_table[ti->hash_index];
	if (ecm_db_multicast_tuple_instance_table[ti->hash_index]) {
		ecm_db_multicast_tuple_instance_table[ti->hash_index]->prev = ti;
	}

	ecm_db_multicast_tuple_instance_table[ti->hash_index] = ti;

	ti->flags |= ECM_DB_MULTICAST_TUPLE_INSTANCE_FLAGS_INSERTED;
	spin_unlock_bh(&ecm_db_lock);

}
EXPORT_SYMBOL(ecm_db_multicast_tuple_instance_add);

/*
 * ecm_db_multicast_connection_get_and_ref_first()
 * 	Return the first tuple instance from the table when given a group
 * 	Also take a ref count for 'ci', once done call ecm_db_multicast_connection_deref()
 * 	to deref both 'ti' and 'ci'
 */
struct ecm_db_multicast_tuple_instance *ecm_db_multicast_connection_get_and_ref_first(ip_addr_t group)
{
	ecm_db_multicast_tuple_instance_hash_t hash_index;
	struct ecm_db_multicast_tuple_instance *ti;

	hash_index = ecm_db_multicast_generate_hash_index(group);

	spin_lock_bh(&ecm_db_lock);
	ti = ecm_db_multicast_tuple_instance_table[hash_index];
	if (ti) {
		_ecm_db_multicast_tuple_instance_ref(ti);
		_ecm_db_connection_ref(ti->ci);
	}
	spin_unlock_bh(&ecm_db_lock);

	return ti;
}
EXPORT_SYMBOL(ecm_db_multicast_connection_get_and_ref_first);

/*
 * ecm_db_multicast_connection_get_and_ref_next()
 * 	Return the next tuple instance node and
 * 	take a ref count for 'ci', once done call ecm_db_multicast_connection_deref()
 * 	to deref both 'ti' and 'ci'
 */
struct ecm_db_multicast_tuple_instance *ecm_db_multicast_connection_get_and_ref_next(struct ecm_db_multicast_tuple_instance *ti)
{
	struct ecm_db_multicast_tuple_instance *tin;
	DEBUG_CHECK_MAGIC(ti, ECM_DB_MULTICAST_INSTANCE_MAGIC, "%px: magic failed", ti);
	spin_lock_bh(&ecm_db_lock);
	tin = ti->next;
	if (tin) {
		_ecm_db_multicast_tuple_instance_ref(tin);
		_ecm_db_connection_ref(tin->ci);
	}
	spin_unlock_bh(&ecm_db_lock);
	return tin;
}
EXPORT_SYMBOL(ecm_db_multicast_connection_get_and_ref_next);

/*
 * ecm_db_multicast_tuple_instance_source_ip_get()
 * 	This function return the source IP for a connection object
 */
void ecm_db_multicast_tuple_instance_source_ip_get(struct ecm_db_multicast_tuple_instance *ti, ip_addr_t origin)
{
	DEBUG_CHECK_MAGIC(ti, ECM_DB_MULTICAST_INSTANCE_MAGIC, "%px: magic failed", ti);
	ECM_IP_ADDR_COPY(origin, ti->src_ip);
}
EXPORT_SYMBOL(ecm_db_multicast_tuple_instance_source_ip_get);

/*
 * ecm_db_multicast_tuple_instance_group_ip_get()
 * 	This function return the group IP for a connection object
 */
void ecm_db_multicast_tuple_instance_group_ip_get(struct ecm_db_multicast_tuple_instance *ti, ip_addr_t group)
{
	DEBUG_CHECK_MAGIC(ti, ECM_DB_MULTICAST_INSTANCE_MAGIC, "%px: magic failed", ti);
	ECM_IP_ADDR_COPY(group, ti->grp_ip);
}
EXPORT_SYMBOL(ecm_db_multicast_tuple_instance_group_ip_get);

/*
 * ecm_db_multicast_tuple_instance_flags_get()
 * 	Return flags related to Multicast connection
 */
uint32_t ecm_db_multicast_tuple_instance_flags_get(struct ecm_db_multicast_tuple_instance *ti)
{
	uint32_t flags;

	DEBUG_CHECK_MAGIC(ti, ECM_DB_MULTICAST_INSTANCE_MAGIC, "%px: magic failed\n", ti);
	spin_lock_bh(&ecm_db_lock);
	flags = ti->flags;
	spin_unlock_bh(&ecm_db_lock);
	return flags;
}
EXPORT_SYMBOL(ecm_db_multicast_tuple_instance_flags_get);

/*
 * ecm_db_multicast_tuple_instance_flags_set()
 * 	Set the multicast connection flags
 */
void ecm_db_multicast_tuple_instance_flags_set(struct ecm_db_multicast_tuple_instance *ti, uint32_t flags)
{
	DEBUG_CHECK_MAGIC(ti, ECM_DB_MULTICAST_INSTANCE_MAGIC, "%px: magic failed\n", ti);

	spin_lock_bh(&ecm_db_lock);
	ti->flags |= flags;
	spin_unlock_bh(&ecm_db_lock);
}
EXPORT_SYMBOL(ecm_db_multicast_tuple_instance_flags_set);

/*
 * ecm_db_multicast_tuple_instance_flags_clear()
 * 	Clear the multicast connection flags
 */
void ecm_db_multicast_tuple_instance_flags_clear(struct ecm_db_multicast_tuple_instance *ti, uint32_t flags)
{
	DEBUG_CHECK_MAGIC(ti, ECM_DB_MULTICAST_INSTANCE_MAGIC, "%px: magic failed\n", ti);

	spin_lock_bh(&ecm_db_lock);
	ti->flags &= ~flags;
	spin_unlock_bh(&ecm_db_lock);
}
EXPORT_SYMBOL(ecm_db_multicast_tuple_instance_flags_clear);

/*
 * ecm_db_multicast_tuple_instance_set_and_hold_l2_br_dev()
 * 	Save bridge device for L2 multicast flow
 */
void ecm_db_multicast_tuple_instance_set_and_hold_l2_br_dev(struct ecm_db_multicast_tuple_instance *ti, struct net_device *l2_br_dev)
{
	DEBUG_CHECK_MAGIC(ti, ECM_DB_MULTICAST_INSTANCE_MAGIC, "%px: magic failed\n", ti);

	DEBUG_ASSERT(l2_br_dev, "Invalid argument received. Expected a l2_br_dev");

	spin_lock_bh(&ecm_db_lock);
	ti->l2_br_dev = l2_br_dev;
	dev_hold(ti->l2_br_dev);
	spin_unlock_bh(&ecm_db_lock);
}
EXPORT_SYMBOL(ecm_db_multicast_tuple_instance_set_and_hold_l2_br_dev);

/*
 * ecm_db_multicast_tuple_instance_set_and_hold_l3_br_dev()
 * 	Save bridge device for L3 multicast flow
 */
void ecm_db_multicast_tuple_instance_set_and_hold_l3_br_dev(struct ecm_db_multicast_tuple_instance *ti, struct net_device *l3_br_dev)
{
	DEBUG_CHECK_MAGIC(ti, ECM_DB_MULTICAST_INSTANCE_MAGIC, "%px: magic failed\n", ti);

	DEBUG_ASSERT(l3_br_dev, "Invalid argument received. Expected a l3_br_dev");

	spin_lock_bh(&ecm_db_lock);
	ti->l3_br_dev = l3_br_dev;
	dev_hold(ti->l3_br_dev);
	spin_unlock_bh(&ecm_db_lock);
}
EXPORT_SYMBOL(ecm_db_multicast_tuple_instance_set_and_hold_l3_br_dev);

/*
 * ecm_db_multicast_tuple_instance_get_l2_br_dev()
 * 	Return bridge device for L2 multicast flow
 */
struct net_device *ecm_db_multicast_tuple_instance_get_l2_br_dev(struct ecm_db_multicast_tuple_instance *ti)
{
	DEBUG_CHECK_MAGIC(ti, ECM_DB_MULTICAST_INSTANCE_MAGIC, "%px: magic failed\n", ti);
	return ti->l2_br_dev;
}
EXPORT_SYMBOL(ecm_db_multicast_tuple_instance_get_l2_br_dev);

/*
 * ecm_db_multicast_tuple_instance_get_l3_br_dev()
 * 	Return bridge device for L3 multicast flow
 */
struct net_device *ecm_db_multicast_tuple_instance_get_l3_br_dev(struct ecm_db_multicast_tuple_instance *ti)
{
	DEBUG_CHECK_MAGIC(ti, ECM_DB_MULTICAST_INSTANCE_MAGIC, "%px: magic failed\n", ti);
	return ti->l3_br_dev;
}
EXPORT_SYMBOL(ecm_db_multicast_tuple_instance_get_l3_br_dev);

/*
 * ecm_db_multicast_connection_to_interfaces_get_and_ref_all()
 *	Return the list of multicast destination interface heirarchies to which this connection is established.
 *	The function returns the heirarchies using the 'interface' pointer passed to it. It also returns the first
 *	index in the interface heirarchy for each of the heirarchies using the 'ifaces_first' pointer.
 *
 * NOTE: This function allocates the memory for the destination interface heirachy. This memory is expected to be
 * freed only by making a call to ecm_db_multicast_connection_interfaces_deref_all().
 *
 * The size of the buffer allocated for the heirarchies and pointed to by 'interfaces' is as large as
 * sizeof(struct ecm_db_iface_instance *) * ECM_DB_MULTICAST_IF_MAX * ECM_DB_IFACE_HEIRARCHY_MAX.
 * Returns the number of interface heirarchies in the list as a return value.
 *
 * Each interface is referenced on return, be sure to release them using ecm_db_multicast_connection_interfaces_deref_all().
 */
int32_t ecm_db_multicast_connection_to_interfaces_get_and_ref_all(struct ecm_db_connection_instance *ci,
						struct ecm_db_iface_instance **interfaces, int32_t **ifaces_first)
{
	struct ecm_db_iface_instance *heirarchy_base;
	struct ecm_db_iface_instance *heirarchy_temp;
	struct ecm_db_iface_instance *ii_single;
	struct ecm_db_iface_instance **ifaces;
	struct ecm_db_iface_instance *ii_db;
	struct ecm_db_iface_instance *ii_db_single;
	struct ecm_db_iface_instance **ifaces_db;
	int32_t *ii_first_base;
	int32_t *ii_first;
	int32_t heirarchy_index;
	int32_t ii_index;
	int32_t if_count = 0;

	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed\n", ci);

	heirarchy_base = (struct ecm_db_iface_instance *)kzalloc(ECM_DB_TO_MCAST_INTERFACES_SIZE, GFP_ATOMIC | __GFP_NOWARN);
	if (!heirarchy_base) {
		DEBUG_WARN("%px: No memory for interface hierarchies \n", ci);
		return if_count;
	}

	ii_first_base = (int32_t *)kzalloc(sizeof(int32_t *) * ECM_DB_MULTICAST_IF_MAX, GFP_ATOMIC | __GFP_NOWARN);
	if (!ii_first_base) {
		DEBUG_WARN("%px: No memory for first interface \n", ci);
		kfree(heirarchy_base);
		return if_count;
	}

	spin_lock_bh(&ecm_db_lock);
	if (!ci->to_mcast_interfaces_set) {
		spin_unlock_bh(&ecm_db_lock);
		kfree(ii_first_base);
		kfree(heirarchy_base);
		return if_count;
	}

	for (heirarchy_index = 0; heirarchy_index < ECM_DB_MULTICAST_IF_MAX; heirarchy_index++) {

		heirarchy_temp = ecm_db_multicast_if_heirarchy_get(heirarchy_base, heirarchy_index);

		if (ci->to_mcast_interface_first[heirarchy_index] < ECM_DB_IFACE_HEIRARCHY_MAX) {
			if_count++;
		}

		for (ii_index = ci->to_mcast_interface_first[heirarchy_index]; ii_index < ECM_DB_IFACE_HEIRARCHY_MAX; ++ii_index) {
			ii_db = ecm_db_multicast_if_heirarchy_get(ci->to_mcast_interfaces, heirarchy_index);
			ii_db_single = ecm_db_multicast_if_instance_get_at_index(ii_db, ii_index);
			ifaces_db = (struct ecm_db_iface_instance **)ii_db_single;

			/*
			 * Take a reference count
			 */
			_ecm_db_iface_ref(*ifaces_db);

			ii_single = ecm_db_multicast_if_instance_get_at_index(heirarchy_temp, ii_index);
			ifaces = (struct ecm_db_iface_instance **)ii_single;
			*ifaces = *ifaces_db;
		}

		ii_first = ecm_db_multicast_if_first_get_at_index(ii_first_base, heirarchy_index);
		*ii_first = ci->to_mcast_interface_first[heirarchy_index];
	}

	*interfaces = heirarchy_base;
	*ifaces_first = ii_first_base;

	spin_unlock_bh(&ecm_db_lock);
	return if_count;
}
EXPORT_SYMBOL(ecm_db_multicast_connection_to_interfaces_get_and_ref_all);

/*
 * ecm_db_multicast_connection_to_interfaces_set_check()
 *	Returns true if the multicast destination interfaces list has been set.
 */
bool ecm_db_multicast_connection_to_interfaces_set_check(struct ecm_db_connection_instance *ci)
{
	bool set;

	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed\n", ci);
	spin_lock_bh(&ecm_db_lock);
	set = ci->to_mcast_interfaces_set;
	spin_unlock_bh(&ecm_db_lock);
	return set;
}
EXPORT_SYMBOL(ecm_db_multicast_connection_to_interfaces_set_check);

/*
 * ecm_db_multicast_connection_to_interfaces_set_clear()
 *	Clear the to_mcast_interfaces_set flag if the multicast destination interfaces list has been freed.
 */
static void  _ecm_db_multicast_connection_to_interfaces_set_clear(struct ecm_db_connection_instance *ci)
{
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed\n", ci);
	ci->to_mcast_interfaces_set = false;
}

/*
 * ecm_db_multicast_connection_get_from_tuple()
 * 	Return the connection instance
 */
struct ecm_db_connection_instance *ecm_db_multicast_connection_get_from_tuple(struct ecm_db_multicast_tuple_instance *ti)
{
	DEBUG_CHECK_MAGIC(ti, ECM_DB_MULTICAST_INSTANCE_MAGIC, "%px: magic failed", ti);
	DEBUG_ASSERT(ti->ci, "%px: Bad multicast connection instance \n", ti);

	return ti->ci;
}
EXPORT_SYMBOL(ecm_db_multicast_connection_get_from_tuple);

/*
 * ecm_db_multicast_connection_to_interfaces_deref_all()
 * 	Deref all destination multicast interface heirarchies at once
 */
void ecm_db_multicast_connection_to_interfaces_deref_all(struct ecm_db_iface_instance *interfaces, int32_t *ifaces_first)
{
	struct ecm_db_iface_instance *ifaces_single;
	struct ecm_db_iface_instance *ii_temp[ECM_DB_IFACE_HEIRARCHY_MAX];
	int32_t *to_first;
	int heirarchy_index;
	DEBUG_ASSERT(interfaces, "Bad memory, multicast interfaces list has been already freed\n");
	DEBUG_ASSERT(ifaces_first, "Bad memory, multicast interfaces first has been already freed\n");

	for (heirarchy_index = 0; heirarchy_index < ECM_DB_MULTICAST_IF_MAX; heirarchy_index++) {
		to_first = ecm_db_multicast_if_first_get_at_index(ifaces_first, heirarchy_index);
		if (*to_first < ECM_DB_IFACE_HEIRARCHY_MAX) {
			ifaces_single = ecm_db_multicast_if_heirarchy_get(interfaces, heirarchy_index);
			ecm_db_multicast_copy_if_heirarchy(ii_temp, ifaces_single);
			ecm_db_connection_interfaces_deref(ii_temp, *to_first);
		}
	}

	/*
	 * Free the temporary memory allocated by ecm_db_multicast_connection_to_interfaces_get_and_ref_all()
	 */
	kfree(interfaces);
	kfree(ifaces_first);

}
EXPORT_SYMBOL(ecm_db_multicast_connection_to_interfaces_deref_all);

/*
 * _ecm_db_multicast_connection_to_interface_first_is_valid()
 * 	Check if destnation interfaces first list uphold a valid interface
 * 	first or all entries have discarded.
 */
static bool _ecm_db_multicast_connection_to_interface_first_is_valid(int32_t ifaces_first[])
{
	int heirarchy_index;

	for (heirarchy_index = 0; heirarchy_index < ECM_DB_MULTICAST_IF_MAX; heirarchy_index++) {
		if (ifaces_first[heirarchy_index] < ECM_DB_IFACE_HEIRARCHY_MAX) {
			return true;
		}
	}

	return false;
}

/*
 * ecm_db_multicast_connection_to_interfaces_clear_at_index()
 * 	Dereference and clear a interface heirarchy at 'index' position
 */
void ecm_db_multicast_connection_to_interfaces_clear_at_index(struct ecm_db_connection_instance *ci, uint32_t index)
{
	struct ecm_db_iface_instance *discard[ECM_DB_IFACE_HEIRARCHY_MAX];
	struct ecm_db_iface_instance *ifaces_db_single;
	int32_t discard_first;

	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed\n", ci);

	/*
	 * Invalid Index Value
	 */
	DEBUG_ASSERT((index < ECM_DB_MULTICAST_IF_MAX), "%px: Invalid index for multicast interface heirarchies list %u\n", ci, index);

	spin_lock_bh(&ecm_db_lock);
	if (ci->to_mcast_interface_first[index] == ECM_DB_IFACE_HEIRARCHY_MAX) {
		spin_unlock_bh(&ecm_db_lock);
		return;
	}

	ifaces_db_single = ecm_db_multicast_if_heirarchy_get(ci->to_mcast_interfaces, index);
	ecm_db_multicast_copy_if_heirarchy(discard, ifaces_db_single);

	discard_first = ci->to_mcast_interface_first[index];
	ci->to_mcast_interface_first[index] = ECM_DB_IFACE_HEIRARCHY_MAX;

	/*
	 * If this is the only valid interface hierarchy left in the list of destination
	 * interface hierarchies then clear the ci->to_mcast_interfaces_set flag here before
	 * deleting this.
	 */
	if (!_ecm_db_multicast_connection_to_interface_first_is_valid(ci->to_mcast_interface_first)) {
		ci->to_mcast_interfaces_set = false;
	}

	spin_unlock_bh(&ecm_db_lock);

	ecm_db_connection_interfaces_deref(discard, discard_first);
}
EXPORT_SYMBOL(ecm_db_multicast_connection_to_interfaces_clear_at_index);

/*
 * ecm_db_multicast_connection_to_interfaces_get_count()
 * 	Get the number of to interfaces for a connection
 */
int ecm_db_multicast_connection_to_interfaces_get_count(struct ecm_db_connection_instance *ci)
{
	int heirarchy_index, count = 0;

	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed\n", ci);

	spin_lock_bh(&ecm_db_lock);
	for (heirarchy_index = 0; heirarchy_index < ECM_DB_MULTICAST_IF_MAX; heirarchy_index++) {
		if (ci->to_mcast_interface_first[heirarchy_index] < ECM_DB_IFACE_HEIRARCHY_MAX) {
			count++;
		}
	}
	spin_unlock_bh(&ecm_db_lock);

	return count;
}
EXPORT_SYMBOL(ecm_db_multicast_connection_to_interfaces_get_count);

/*
 * ecm_db_multicast_connection_to_interfaces_clear()
 * 	Deref and clear all destination multicast interface heirarchies
 */
void ecm_db_multicast_connection_to_interfaces_clear(struct ecm_db_connection_instance *ci)
{
	int heirarchy_index;
	DEBUG_CHECK_MAGIC(ci, ECM_DB_CONNECTION_INSTANCE_MAGIC, "%px: magic failed\n", ci);

	spin_lock_bh(&ecm_db_lock);
	if (!ci->to_mcast_interfaces) {
		spin_unlock_bh(&ecm_db_lock);
		return;
	}

	_ecm_db_multicast_connection_to_interfaces_set_clear(ci);
	spin_unlock_bh(&ecm_db_lock);

	for (heirarchy_index = 0; heirarchy_index < ECM_DB_MULTICAST_IF_MAX; heirarchy_index++) {
		ecm_db_multicast_connection_to_interfaces_clear_at_index(ci, heirarchy_index);
	}

	spin_lock_bh(&ecm_db_lock);
	kfree(ci->to_mcast_interfaces);
	ci->to_mcast_interfaces = NULL;
	spin_unlock_bh(&ecm_db_lock);
}
EXPORT_SYMBOL(ecm_db_multicast_connection_to_interfaces_clear);

/*
 * ecm_db_multicast_to_interfaces_xml_state_get()
 *	Obtain XML state for the multicast destination interfaces list
 */
int ecm_db_multicast_to_interfaces_xml_state_get(struct ecm_db_connection_instance *ci, struct ecm_state_file_instance *sfi)
{
	struct ecm_db_iface_instance *mc_ifaces;
	struct ecm_db_iface_instance *mc_ifaces_single[ECM_DB_IFACE_HEIRARCHY_MAX];
	struct ecm_db_iface_instance *ii_temp;
	int32_t *mc_ifaces_first;
	int32_t *ifaces_first;
	int32_t heirarchy_index;
	int ret;

	ret = ecm_db_multicast_connection_to_interfaces_get_and_ref_all(ci, &mc_ifaces, &mc_ifaces_first);
	if (ret == 0) {
		return ret;
	}

	for (heirarchy_index = 0; heirarchy_index < ECM_DB_MULTICAST_IF_MAX; heirarchy_index++) {

		ii_temp = ecm_db_multicast_if_heirarchy_get(mc_ifaces, heirarchy_index);
		ecm_db_multicast_copy_if_heirarchy(mc_ifaces_single, ii_temp);
		ifaces_first = ecm_db_multicast_if_first_get_at_index(mc_ifaces_first, heirarchy_index);

		if (ci->to_mcast_interface_first[heirarchy_index] < ECM_DB_IFACE_HEIRARCHY_MAX) {
			ret = ecm_db_connection_heirarchy_state_get(sfi, mc_ifaces_single, *ifaces_first);
			if (ret) {
				ecm_db_multicast_connection_to_interfaces_deref_all(mc_ifaces, mc_ifaces_first);
				return ret;
			}

		}
	}
	ecm_db_multicast_connection_to_interfaces_deref_all(mc_ifaces, mc_ifaces_first);

	return ret;
}

#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
#ifdef ECM_CLASSIFIER_OVS_ENABLE
/*
 * ecm_db_multicast_ovs_verify_to_list()
 * 	Verify the 'to' interface list with OVS classifier.
 */
bool ecm_db_multicast_ovs_verify_to_list(struct ecm_db_connection_instance *ci, struct ecm_classifier_process_response *aci_pr)
{
	struct ecm_classifier_instance *aci;
	bool is_defunct = false;

	/*
	 * Get the OVS classifier instance from the connection.
	 */
	aci = ecm_db_connection_assigned_classifier_find_and_ref(ci, ECM_CLASSIFIER_TYPE_OVS);
	if (!aci) {
		DEBUG_WARN("%px: no OVS classifier\n", ci);
		return is_defunct;
	}

	aci->process(aci, ECM_TRACKER_SENDER_MAX, NULL, NULL, aci_pr);
	if (aci_pr->process_actions & ECM_CLASSIFIER_PROCESS_ACTION_OVS_MCAST_DENY_ACCEL) {
		is_defunct = true;
	}

	aci->deref(aci);
	return is_defunct;
}

/*
 * ecm_db_multicast_tuple_set_ovs_ingress_vlan()
 * 	Set ingress VLAN tag for OVS ports.
 */
void ecm_db_multicast_tuple_set_ovs_ingress_vlan(struct ecm_db_multicast_tuple_instance *ti, uint32_t *ingress_vlan_tag)
{
	DEBUG_CHECK_MAGIC(ti, ECM_DB_MULTICAST_INSTANCE_MAGIC, "%px: magic failed", ti);

	ti->ovs_ingress_vlan.h_vlan_TCI = ingress_vlan_tag[0] & 0xffff;
	ti->ovs_ingress_vlan.h_vlan_encapsulated_proto = (ingress_vlan_tag[0] >> 16) & 0xffff;
}

/*
 * ecm_db_multicast_tuple_get_ovs_ingress_vlan()
 * 	Get ingress VLAN tag for OVS ports.
 */
struct vlan_hdr ecm_db_multicast_tuple_get_ovs_ingress_vlan(struct ecm_db_multicast_tuple_instance *ti)
{
	DEBUG_CHECK_MAGIC(ti, ECM_DB_MULTICAST_INSTANCE_MAGIC, "%px: magic failed", ti);

	return ti->ovs_ingress_vlan;
}
#endif
#endif

/*
 * ecm_db_multicast_connection_to_interfaces_leave()
 * 	Remove 'to' interfaces from the connection if it has left the group.
 */
void ecm_db_multicast_connection_to_interfaces_leave(struct ecm_db_connection_instance *ci, struct ecm_multicast_if_update *mc_update)
{
	int i;

	if (!mc_update->if_leave_cnt) {
		return;
	}

	for (i = 0; i < ECM_DB_MULTICAST_IF_MAX && mc_update->if_leave_cnt; i++) {
		/*
		 * Is this entry marked? If yes, then the corresponding entry
		 * in the 'to_mcast_interfaces' array in the ci has left the
		 * connection.
		 */
		if (!mc_update->if_leave_idx[i]) {
			continue;
		}

		/*
		 * Release the interface heirarchy for this
		 * interface since it has left the group
		 */
		ecm_db_multicast_connection_to_interfaces_clear_at_index(ci, i);
		mc_update->if_leave_cnt--;
	}
}

/*
 * ecm_db_multicast_netdevs_get_index()
 * 	Gets the list of interface index  of source and destination netdevices for the mulicast flow
 */
struct ecm_db_multicast_iface_list_info ecm_db_multicast_netdevs_get_index(struct ecm_db_connection_instance *ci)
{
	int32_t to_iface_id, from_iface_id, from_ifaces_first, ret, *to_ifaces_first, *to_ii_first;
	struct ecm_db_iface_instance *ii_temp, *ii, **ifaces, *ii_single;
	struct ecm_db_iface_instance *to_ifaces, *from_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX];
	uint32_t vif;
	struct ecm_db_multicast_iface_list_info mc_ifindex =  {0};

	ret = ecm_db_multicast_connection_to_interfaces_get_and_ref_all(ci, &to_ifaces, &to_ifaces_first);
	if (!ret) {
		DEBUG_TRACE("%px: Failed to get interfaces list\n", ci);
		return mc_ifindex;
	}

	for (vif = 0; vif < ECM_DB_MULTICAST_IF_MAX; vif++) {
		ii_temp = ecm_db_multicast_if_heirarchy_get(to_ifaces, vif);
		to_ii_first = ecm_db_multicast_if_first_get_at_index(to_ifaces_first, vif);

		if (*to_ii_first < ECM_DB_IFACE_HEIRARCHY_MAX) {
			ii_single = ecm_db_multicast_if_instance_get_at_index(ii_temp, *to_ii_first);
			ifaces = (struct ecm_db_iface_instance **)ii_single;
			ii = *ifaces;
			to_iface_id = ecm_db_iface_interface_identifier_get(ii);
			DEBUG_TRACE("%x: Destination interface[%d] index: %d\n", ci->serial, mc_ifindex.dest_dev_count, to_iface_id);
			mc_ifindex.dest_ifindex[mc_ifindex.dest_dev_count++] = to_iface_id;
		}
	}
	ecm_db_multicast_connection_to_interfaces_deref_all(to_ifaces, to_ifaces_first);

	if (!mc_ifindex.dest_dev_count) {
		DEBUG_WARN("%px: Failed to get destination net devices\n", ci);
		return mc_ifindex;
	}

	from_ifaces_first = ecm_db_connection_interfaces_get_and_ref(ci, from_ifaces, ECM_DB_OBJ_DIR_FROM);

	if (from_ifaces_first != ECM_DB_IFACE_HEIRARCHY_MAX) {
		from_iface_id = ecm_db_iface_interface_identifier_get(from_ifaces[from_ifaces_first]);
		DEBUG_TRACE("%x: Source interface index: %d\n", ci->serial, from_iface_id);
		mc_ifindex.src_ifindex = from_iface_id;
		ecm_db_connection_interfaces_deref(from_ifaces, from_ifaces_first);
		return mc_ifindex;
	}

	DEBUG_WARN("%px: Failed to get source net device\n", ci);
	memset(&mc_ifindex, 0, sizeof(mc_ifindex));
	return mc_ifindex;
}
#endif
