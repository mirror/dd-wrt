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

/*
 * Global list.
 * All instances are inserted into global list - this allows easy iteration of all instances of a particular type.
 * The list is doubly linked for fast removal.  The list is in no particular order.
 */
struct ecm_db_iface_instance *ecm_db_interfaces = NULL;

/*
 * Interface hash table
 */
#define ECM_DB_IFACE_HASH_SLOTS 8
static struct ecm_db_iface_instance *ecm_db_iface_table[ECM_DB_IFACE_HASH_SLOTS];
                                                        /* Slots of the interface hash table */
static int ecm_db_iface_table_lengths[ECM_DB_IFACE_HASH_SLOTS];
                                                        /* Tracks how long each chain is */
#define ECM_DB_IFACE_ID_HASH_SLOTS 8
static struct ecm_db_iface_instance *ecm_db_iface_id_table[ECM_DB_IFACE_ID_HASH_SLOTS];
                                                        /* Slots of the interface id hash table */
static int ecm_db_iface_id_table_lengths[ECM_DB_IFACE_ID_HASH_SLOTS];
                                                        /* Tracks how long each chain is */
static int ecm_db_iface_count = 0;			/* Number of interfaces allocated */

/*
 * Interface flags
 */
#define ECM_DB_IFACE_FLAGS_INSERTED 1			/* Interface is inserted into connection database tables */

/*
 * ecm_db_interface_type_names[]
 *	Array that maps the interface type to a string
 */
static char *ecm_db_interface_type_names[ECM_DB_IFACE_TYPE_COUNT] = {
	"ETHERNET",
	"PPPoE",
	"LINK-AGGREGATION",
	"VLAN",
	"BRIDGE",
	"LOOPBACK",
	"IPSEC_TUNNEL",
	"UNKNOWN",
	"SIT",
	"TUNIPIP6",
	"PPPoL2TPV2",
	"PPTP",
	"MAP_T",
	"GRE_TUN",
	"GRE_TAP",
	"RAWIP",
	"OVPN",
	"VxLAN",
	"OVS_BRIDGE",
	"MACVLAN"
};

/*
 * _ecm_db_iface_count_get()
 *	Return the iface count (lockless).
 */
int _ecm_db_iface_count_get(void)
{
	return ecm_db_iface_count;
}

/*
 * ecm_db_interface_type_to_string()
 *	Return a string buffer containing the type name of the interface
 */
char *ecm_db_interface_type_to_string(ecm_db_iface_type_t type)
{
	DEBUG_ASSERT((type >= 0) && (type < ECM_DB_IFACE_TYPE_COUNT), "Invalid type: %d\n", type);
	return ecm_db_interface_type_names[(int)type];
}
EXPORT_SYMBOL(ecm_db_interface_type_to_string);

/*
 * ecm_db_iface_type_get()
 *	Return type of interface
 */
ecm_db_iface_type_t ecm_db_iface_type_get(struct ecm_db_iface_instance *ii)
{
	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed", ii);
	return ii->type;
}
EXPORT_SYMBOL(ecm_db_iface_type_get);

/*
 * ecm_db_iface_id_generate_hash_index()
 *	Calculate the hash index based on interface identifier.
 */
static inline ecm_db_iface_id_hash_t ecm_db_iface_id_generate_hash_index(int32_t interface_id)
{
	uint32_t hash_val;

	hash_val = (uint32_t)jhash_1word((uint32_t)interface_id, ecm_db_jhash_rnd);
	return (ecm_db_iface_id_hash_t)(hash_val & (ECM_DB_IFACE_ID_HASH_SLOTS - 1));
}

#ifdef ECM_DB_ADVANCED_STATS_ENABLE
/*
 * ecm_db_iface_data_stats_get()
 *	Return data stats for the instance
 */
static void ecm_db_iface_data_stats_get(struct ecm_db_iface_instance *ii, uint64_t *from_data_total, uint64_t *to_data_total,
						uint64_t *from_packet_total, uint64_t *to_packet_total,
						uint64_t *from_data_total_dropped, uint64_t *to_data_total_dropped,
						uint64_t *from_packet_total_dropped, uint64_t *to_packet_total_dropped)
{
	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed", ii);
	spin_lock_bh(&ecm_db_lock);
	if (from_data_total) {
		*from_data_total = ii->from_data_total;
	}
	if (to_data_total) {
		*to_data_total = ii->to_data_total;
	}
	if (from_packet_total) {
		*from_packet_total = ii->from_packet_total;
	}
	if (to_packet_total) {
		*to_packet_total = ii->to_packet_total;
	}
	if (from_data_total_dropped) {
		*from_data_total_dropped = ii->from_data_total_dropped;
	}
	if (to_data_total_dropped) {
		*to_data_total_dropped = ii->to_data_total_dropped;
	}
	if (from_packet_total_dropped) {
		*from_packet_total_dropped = ii->from_packet_total_dropped;
	}
	if (to_packet_total_dropped) {
		*to_packet_total_dropped = ii->to_packet_total_dropped;
	}
	spin_unlock_bh(&ecm_db_lock);
}
#endif

#ifdef ECM_STATE_OUTPUT_ENABLE
/*
 * ecm_db_iface_state_get_base()
 *	Get the basic state for an interface object
 */
static int ecm_db_iface_state_get_base(struct ecm_db_iface_instance *ii, struct ecm_state_file_instance *sfi)
{
	int result;
#ifdef ECM_DB_XREF_ENABLE
	int node_count;
#endif
	uint32_t time_added;
	int32_t interface_identifier;
	int32_t ae_interface_identifier;
	char name[IFNAMSIZ];
	int32_t mtu;
	ecm_db_iface_type_t type;
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

	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);
	DEBUG_TRACE("%px: Open iface msg\n", ii);

	if ((result = ecm_state_prefix_add(sfi, "iface"))) {
		return result;
	}

#ifdef ECM_DB_XREF_ENABLE
	node_count = ecm_db_iface_node_count_get(ii);
#endif
	time_added = ii->time_added;
	type = ii->type;
	interface_identifier = ii->interface_identifier;
	ae_interface_identifier = ii->ae_interface_identifier;
	spin_lock_bh(&ecm_db_lock);
	strlcpy(name, ii->name, IFNAMSIZ);
	mtu = ii->mtu;
	spin_unlock_bh(&ecm_db_lock);

#ifdef ECM_DB_ADVANCED_STATS_ENABLE
	ecm_db_iface_data_stats_get(ii, &from_data_total, &to_data_total,
			&from_packet_total, &to_packet_total,
			&from_data_total_dropped, &to_data_total_dropped,
			&from_packet_total_dropped, &to_packet_total_dropped);

	if ((result = ecm_db_adv_stats_state_write(sfi, from_data_total, to_data_total,
			from_packet_total, to_packet_total, from_data_total_dropped,
			to_data_total_dropped, from_packet_total_dropped,
			to_packet_total_dropped))) {
		return result;
	}
#endif

	if ((result = ecm_state_write(sfi, "type", "%d", type))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "name", "%s", name))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "time_added", "%u", time_added))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "mtu", "%d", mtu))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "interface_identifier", "%d", interface_identifier))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "ae_interface_identifier", "%d", ae_interface_identifier))) {
		return result;
	}

#ifdef ECM_DB_XREF_ENABLE
	if ((result = ecm_state_write(sfi, "nodes", "%d", node_count))) {
		return result;
	}
#endif

	return ecm_state_prefix_remove(sfi);
}

/*
 * ecm_db_iface_ethernet_state_get()
 * 	Return interface type specific state
 */
static int ecm_db_iface_ethernet_state_get(struct ecm_db_iface_instance *ii, struct ecm_state_file_instance *sfi)
{
	int result;
	uint8_t address[ETH_ALEN];

	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);
	spin_lock_bh(&ecm_db_lock);
	memcpy(address, ii->type_info.ethernet.address, ETH_ALEN);
	spin_unlock_bh(&ecm_db_lock);

	if ((result = ecm_state_prefix_add(sfi, "ethernet"))) {
		return result;
	}

	if ((result = ecm_db_iface_state_get_base(ii, sfi))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "address", "%pM", address))) {
		return result;
	}

	return ecm_state_prefix_remove(sfi);
}

#ifdef ECM_INTERFACE_BOND_ENABLE
/*
 * ecm_db_iface_lag_state_get()
 * 	Return interface type specific state
 */
static int ecm_db_iface_lag_state_get(struct ecm_db_iface_instance *ii, struct ecm_state_file_instance *sfi)
{
	int result;
	uint8_t address[ETH_ALEN];

	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);
	spin_lock_bh(&ecm_db_lock);
	memcpy(address, ii->type_info.lag.address, ETH_ALEN);
	spin_unlock_bh(&ecm_db_lock);

	if ((result = ecm_state_prefix_add(sfi, "lag"))) {
		return result;
	}
	if ((result = ecm_db_iface_state_get_base(ii, sfi))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "address", "%pM", address))) {
		return result;
	}

	return ecm_state_prefix_remove(sfi);
}
#endif

/*
 * ecm_db_iface_bridge_state_get()
 * 	Return interface type specific state
 */
static int ecm_db_iface_bridge_state_get(struct ecm_db_iface_instance *ii, struct ecm_state_file_instance *sfi)
{
	int result;
	uint8_t address[ETH_ALEN];

	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);
	spin_lock_bh(&ecm_db_lock);
	memcpy(address, ii->type_info.bridge.address, ETH_ALEN);
	spin_unlock_bh(&ecm_db_lock);

	if ((result = ecm_state_prefix_add(sfi, "bridge"))) {
		return result;
	}
	if ((result = ecm_db_iface_state_get_base(ii, sfi))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "address", "%pM", address))) {
		return result;
	}

	return ecm_state_prefix_remove(sfi);
}

#ifdef ECM_INTERFACE_MACVLAN_ENABLE
/*
 * ecm_db_iface_macvlan_state_get()
 * 	Return interface type specific state
 */
static int ecm_db_iface_macvlan_state_get(struct ecm_db_iface_instance *ii, struct ecm_state_file_instance *sfi)
{
	int result;
	uint8_t address[ETH_ALEN];

	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);
	spin_lock_bh(&ecm_db_lock);
	memcpy(address, ii->type_info.macvlan.address, ETH_ALEN);
	spin_unlock_bh(&ecm_db_lock);

	if ((result = ecm_state_prefix_add(sfi, "macvlan"))) {
		return result;
	}

	if ((result = ecm_db_iface_state_get_base(ii, sfi))) {
		goto done;
	}

	if ((result = ecm_state_write(sfi, "address", "%pM", address))) {
		goto done;
	}

done:
	ecm_state_prefix_remove(sfi);
	return result;
}
#endif

#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
/*
 * ecm_db_iface_ovs_bridge_state_get()
 * 	Return interface type specific state
 */
static int ecm_db_iface_ovs_bridge_state_get(struct ecm_db_iface_instance *ii, struct ecm_state_file_instance *sfi)
{
	int result;
	uint8_t address[ETH_ALEN];

	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);
	spin_lock_bh(&ecm_db_lock);
	memcpy(address, ii->type_info.ovsb.address, ETH_ALEN);
	spin_unlock_bh(&ecm_db_lock);

	if ((result = ecm_state_prefix_add(sfi, "ovs_bridge"))) {
		return result;
	}
	if ((result = ecm_db_iface_state_get_base(ii, sfi))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "address", "%pM", address))) {
		return result;
	}

	return ecm_state_prefix_remove(sfi);
}
#endif

#ifdef ECM_INTERFACE_VLAN_ENABLE
/*
 * ecm_db_iface_vlan_state_get()
 * 	Return interface type specific state
 */
static int ecm_db_iface_vlan_state_get(struct ecm_db_iface_instance *ii, struct ecm_state_file_instance *sfi)
{
	int result;
	uint8_t address[ETH_ALEN];
	uint16_t vlan_tag;
	uint16_t vlan_tpid;

	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);
	spin_lock_bh(&ecm_db_lock);
	memcpy(address, ii->type_info.vlan.address, ETH_ALEN);
	vlan_tag = ii->type_info.vlan.vlan_tag;
	vlan_tpid = ii->type_info.vlan.vlan_tpid;
	spin_unlock_bh(&ecm_db_lock);

	if ((result = ecm_state_prefix_add(sfi, "vlan"))) {
		return result;
	}
	if ((result = ecm_db_iface_state_get_base(ii, sfi))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "address", "%pM", address))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "tag", "%x", vlan_tag))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "tpid", "%x", vlan_tpid))) {
		return result;
	}

	return ecm_state_prefix_remove(sfi);
}
#endif

#ifdef ECM_INTERFACE_PPPOE_ENABLE
/*
 * ecm_db_iface_pppoe_state_get()
 * 	Return interface type specific state
 */
static int ecm_db_iface_pppoe_state_get(struct ecm_db_iface_instance *ii, struct ecm_state_file_instance *sfi)
{
	int result;
	uint16_t pppoe_session_id;
	uint8_t remote_mac[ETH_ALEN];

	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);
	spin_lock_bh(&ecm_db_lock);
	pppoe_session_id = ii->type_info.pppoe.pppoe_session_id;
	memcpy(remote_mac, ii->type_info.pppoe.remote_mac, ETH_ALEN);
	spin_unlock_bh(&ecm_db_lock);

	if ((result = ecm_state_prefix_add(sfi, "pppoe"))) {
		return result;
	}
	if ((result = ecm_db_iface_state_get_base(ii, sfi))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "remote_mac", "%pM", remote_mac))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "session_id", "%u", pppoe_session_id))) {
		return result;
	}

	return ecm_state_prefix_remove(sfi);
}
#endif

#ifdef ECM_INTERFACE_MAP_T_ENABLE
/*
 * ecm_db_iface_map_t_state_get()
 *	Return interface type specific state
 */
static int ecm_db_iface_map_t_state_get(struct ecm_db_iface_instance *ii, struct ecm_state_file_instance *sfi)
{
	int result;
	int32_t if_index;

	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);
	spin_lock_bh(&ecm_db_lock);
	if_index = ii->type_info.map_t.if_index;
	spin_unlock_bh(&ecm_db_lock);

	if ((result = ecm_state_prefix_add(sfi, "map_t"))) {
		return result;
	}
	if ((result = ecm_db_iface_state_get_base(ii, sfi))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "if_index", "%d", if_index))) {
		return result;
	}

	return ecm_state_prefix_remove(sfi);
}
#endif

#ifdef ECM_INTERFACE_GRE_TUN_ENABLE
/*
 * ecm_db_iface_gre_tun_state_get
 * 	Return interface type specific state
 */
static int ecm_db_iface_gre_tun_state_get(struct ecm_db_iface_instance *ii, struct ecm_state_file_instance *sfi)
{
	int result;
	int32_t if_index;
	ip_addr_t local_ip, remote_ip;
	char local_ipaddress[ECM_IP_ADDR_STR_BUFF_SIZE];
	char remote_ipaddress[ECM_IP_ADDR_STR_BUFF_SIZE];

	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);
	spin_lock_bh(&ecm_db_lock);
	if_index = ii->type_info.gre_tun.if_index;
	memcpy(local_ip, ii->type_info.gre_tun.local_ip, sizeof(ip_addr_t));
	memcpy(remote_ip, ii->type_info.gre_tun.remote_ip, sizeof(ip_addr_t));
	spin_unlock_bh(&ecm_db_lock);

	ecm_ip_addr_to_string(local_ipaddress, local_ip);
	ecm_ip_addr_to_string(remote_ipaddress, remote_ip);

	if ((result = ecm_state_prefix_add(sfi, "gre_tun"))) {
		return result;
	}
	if ((result = ecm_db_iface_state_get_base(ii, sfi))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "if_index", "%d", if_index))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "local_ip", "%s", local_ipaddress))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "remote_ip", "%s", remote_ipaddress))) {
		return result;
	}

	return ecm_state_prefix_remove(sfi);
}
#endif

#ifdef ECM_INTERFACE_L2TPV2_ENABLE

/*
 * ecm_db_iface_pppol2tpv2_state_get()
 *	Return interface type specific state
 */
static int ecm_db_iface_pppol2tpv2_state_get(struct ecm_db_iface_instance *ii, struct ecm_state_file_instance *sfi)
{
	int result;
	struct ecm_db_interface_info_pppol2tpv2 type_info;

	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);
	spin_lock_bh(&ecm_db_lock);
	memcpy(&type_info, &ii->type_info, sizeof(struct ecm_db_interface_info_pppol2tpv2));
	spin_unlock_bh(&ecm_db_lock);

	if ((result = ecm_state_prefix_add(sfi, "pppol2tpv2"))) {
		return result;
	}

	if ((result = ecm_db_iface_state_get_base(ii, sfi))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "local_tunnel_id", "%u", type_info.l2tp.tunnel.tunnel_id))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "local_session_id", "%u", type_info.l2tp.session.session_id))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "peer_tunnnel_id", "%u", type_info.l2tp.tunnel.peer_tunnel_id))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "peer_session_id", "%u", type_info.l2tp.session.peer_session_id))) {
		return result;
	}

	return ecm_state_prefix_remove(sfi);
}

#endif

#ifdef ECM_INTERFACE_PPTP_ENABLE
/*
 * ecm_db_iface_pptp_state_get()
 *	Return interface type specific state
 */
static int ecm_db_iface_pptp_state_get(struct ecm_db_iface_instance *ii, struct ecm_state_file_instance *sfi)
{
	int result;
	struct ecm_db_interface_info_pptp type_info;

	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);
	spin_lock_bh(&ecm_db_lock);
	memcpy(&type_info, &ii->type_info, sizeof(struct ecm_db_interface_info_pptp));
	spin_unlock_bh(&ecm_db_lock);

	result = ecm_state_prefix_add(sfi, "pptp");
	if (result) {
		return result;
	}

	result = ecm_db_iface_state_get_base(ii, sfi);
	if (result) {
		return result;
	}

	result = ecm_state_write(sfi, "local_call_id", "%u", type_info.src_call_id);
	if (result) {
		return result;
	}

	result = ecm_state_write(sfi, "peer_call_id", "%u", type_info.dst_call_id);
	if (result) {
		return result;
	}

	return ecm_state_prefix_remove(sfi);
}
#endif

/*
 * ecm_db_iface_unknown_state_get()
 * 	Return interface type specific state
 */
static int ecm_db_iface_unknown_state_get(struct ecm_db_iface_instance *ii, struct ecm_state_file_instance *sfi)
{
	int result;
	uint32_t os_specific_ident;

	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);
	spin_lock_bh(&ecm_db_lock);
	os_specific_ident = ii->type_info.unknown.os_specific_ident;
	spin_unlock_bh(&ecm_db_lock);

	if ((result = ecm_state_prefix_add(sfi, "pppoe"))) {
		return result;
	}
	if ((result = ecm_db_iface_state_get_base(ii, sfi))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "os_specific_ident", "%u", os_specific_ident))) {
		return result;
	}

	return ecm_state_prefix_remove(sfi);
}

/*
 * ecm_db_iface_loopback_state_get()
 * 	Return interface type specific state
 */
static int ecm_db_iface_loopback_state_get(struct ecm_db_iface_instance *ii, struct ecm_state_file_instance *sfi)
{
	int result;
	uint32_t os_specific_ident;

	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);
	spin_lock_bh(&ecm_db_lock);
	os_specific_ident = ii->type_info.loopback.os_specific_ident;
	spin_unlock_bh(&ecm_db_lock);

	if ((result = ecm_state_prefix_add(sfi, "loopback"))) {
		return result;
	}
	if ((result = ecm_db_iface_state_get_base(ii, sfi))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "os_specific_ident", "%u", os_specific_ident))) {
		return result;
	}

	return ecm_state_prefix_remove(sfi);
}

#ifdef ECM_INTERFACE_IPSEC_ENABLE
/*
 * ecm_db_iface_ipsec_tunnel_state_get()
 * 	Return interface type specific state
 *
 * GGG TODO Output state on ipsec tunnel specific data
 */
static int ecm_db_iface_ipsec_tunnel_state_get(struct ecm_db_iface_instance *ii, struct ecm_state_file_instance *sfi)
{
	int result;
	uint32_t os_specific_ident;

	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);
	spin_lock_bh(&ecm_db_lock);
	os_specific_ident = ii->type_info.ipsec_tunnel.os_specific_ident;
	spin_unlock_bh(&ecm_db_lock);

	if ((result = ecm_state_prefix_add(sfi, "ipsec"))) {
		return result;
	}
	if ((result = ecm_db_iface_state_get_base(ii, sfi))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "os_specific_ident", "%u", os_specific_ident))) {
		return result;
	}

	return ecm_state_prefix_remove(sfi);
}
#endif

#ifdef ECM_INTERFACE_TUNIPIP6_ENABLE
#ifdef ECM_IPV6_ENABLE
/*
 * ecm_db_iface_tunipip6_state_get()
 * 	Return interface type specific state
 */
static int ecm_db_iface_tunipip6_state_get(struct ecm_db_iface_instance *ii, struct ecm_state_file_instance *sfi)
{
	int result;

	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);

	/*
	 * TODO: tunipip6 specific information needs to be added.
	 */
	if ((result = ecm_state_prefix_add(sfi, "tunipip6"))) {
		return result;
	}
	if ((result = ecm_db_iface_state_get_base(ii, sfi))) {
		return result;
	}

	return ecm_state_prefix_remove(sfi);
}
#endif
#endif

#ifdef ECM_INTERFACE_SIT_ENABLE
/*
 * ecm_db_iface_sit_state_get()
 * 	Return interface type specific state
 */
static int ecm_db_iface_sit_state_get(struct ecm_db_iface_instance *ii, struct ecm_state_file_instance *sfi)
{
	int result;

	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);

	/*
	 * TODO: SIT (6rd) specific information needs to be added.
	 */
	if ((result = ecm_state_prefix_add(sfi, "sit"))) {
		return result;
	}
	if ((result = ecm_db_iface_state_get_base(ii, sfi))) {
		return result;
	}

	return ecm_state_prefix_remove(sfi);
}
#endif

#ifdef ECM_INTERFACE_RAWIP_ENABLE
/*
 * ecm_db_iface_rawip_state_get()
 * 	Return interface type specific state
 */
static int ecm_db_iface_rawip_state_get(struct ecm_db_iface_instance *ii, struct ecm_state_file_instance *sfi)
{
	int result;
	uint8_t address[ETH_ALEN];

	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);
	spin_lock_bh(&ecm_db_lock);
	memcpy(address, ii->type_info.rawip.address, ETH_ALEN);
	spin_unlock_bh(&ecm_db_lock);

	if ((result = ecm_state_prefix_add(sfi, "rawip"))) {
		return result;
	}

	if ((result = ecm_db_iface_state_get_base(ii, sfi))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "address", "%pM", address))) {
		return result;
	}

	return ecm_state_prefix_remove(sfi);
}
#endif

#ifdef ECM_INTERFACE_OVPN_ENABLE
/*
 * ecm_db_iface_ovpn_state_get()
 *	Return OVPN interface state
 */
static int ecm_db_iface_ovpn_state_get(struct ecm_db_iface_instance *ii, struct ecm_state_file_instance *sfi)
{
	int result;
	struct ecm_db_interface_info_ovpn type_info;

	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);
	spin_lock_bh(&ecm_db_lock);
	memcpy(&type_info, &ii->type_info, sizeof(struct ecm_db_interface_info_ovpn));
	spin_unlock_bh(&ecm_db_lock);

	result = ecm_state_prefix_add(sfi, "ovpn");
	if (result) {
		return result;
	}

	result = ecm_db_iface_state_get_base(ii, sfi);
	if (result) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "tun_ifnum", "%u", type_info.tun_ifnum))) {
		return result;
	}

	return ecm_state_prefix_remove(sfi);
}
#endif

#ifdef ECM_INTERFACE_VXLAN_ENABLE
/*
 * ecm_db_iface_vxlan_state_get()
 *	Return interface type specific state
 */
static int ecm_db_iface_vxlan_state_get(struct ecm_db_iface_instance *ii, struct ecm_state_file_instance *sfi)
{
	int result;
	uint32_t vni;

	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);
	spin_lock_bh(&ecm_db_lock);
	vni = ii->type_info.vxlan.vni;
	spin_unlock_bh(&ecm_db_lock);

	if ((result = ecm_state_prefix_add(sfi, "vxlan"))) {
		return result;
	}

	if ((result = ecm_db_iface_state_get_base(ii, sfi))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "vni", "%d", vni))) {
		return result;
	}

	return ecm_state_prefix_remove(sfi);
}
#endif

/*
 * ecm_db_iface_state_get()
 *	Obtain state for the interface.
 *
 * State specific to the interface type will be returned.
 */
int ecm_db_iface_state_get(struct ecm_state_file_instance *sfi, struct ecm_db_iface_instance *ii)
{
	int result;

	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);

	if ((result = ecm_state_prefix_add(sfi, "iface"))) {
		return result;
	}

	if ((result = ii->state_get(ii, sfi))) {
		return result;
	}

	return ecm_state_prefix_remove(sfi);

}
EXPORT_SYMBOL(ecm_db_iface_state_get);

/*
 * ecm_db_iface_hash_table_lengths_get()
 *	Return hash table length
 */
int ecm_db_iface_hash_table_lengths_get(int index)
{
	int length;

	DEBUG_ASSERT((index >= 0) && (index < ECM_DB_IFACE_HASH_SLOTS), "Bad protocol: %d\n", index);
	spin_lock_bh(&ecm_db_lock);
	length = ecm_db_iface_table_lengths[index];
	spin_unlock_bh(&ecm_db_lock);
	return length;
}
EXPORT_SYMBOL(ecm_db_iface_hash_table_lengths_get);

/*
 * ecm_db_iface_hash_index_get_next()
 * Given a hash index, return the next one OR return -1 for no more hash indicies to return.
 */
int ecm_db_iface_hash_index_get_next(int index)
{
	index++;
	if (index >= ECM_DB_IFACE_HASH_SLOTS) {
		return -1;
	}
	return index;
}
EXPORT_SYMBOL(ecm_db_iface_hash_index_get_next);

/*
 * ecm_db_iface_hash_index_get_first()
 * Return first hash index
 */
int ecm_db_iface_hash_index_get_first(void)
{
	return 0;
}
EXPORT_SYMBOL(ecm_db_iface_hash_index_get_first);
#endif

/*
 * _ecm_db_iface_ref()
 */
void _ecm_db_iface_ref(struct ecm_db_iface_instance *ii)
{
	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);
	ii->refs++;
	DEBUG_TRACE("%px: iface ref %d\n", ii, ii->refs);
	DEBUG_ASSERT(ii->refs > 0, "%px: ref wrap\n", ii);
}

/*
 * ecm_db_iface_ref()
 */
void ecm_db_iface_ref(struct ecm_db_iface_instance *ii)
{
	spin_lock_bh(&ecm_db_lock);
	_ecm_db_iface_ref(ii);
	spin_unlock_bh(&ecm_db_lock);
}
EXPORT_SYMBOL(ecm_db_iface_ref);

/*
 * ecm_db_iface_deref()
 *	Deref a interface instance, removing it from the database on the last ref release
 */
int ecm_db_iface_deref(struct ecm_db_iface_instance *ii)
{
	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);

	/*
	 * Decrement reference count
	 */
	spin_lock_bh(&ecm_db_lock);
	ii->refs--;
	DEBUG_TRACE("%px: iface deref %d\n", ii, ii->refs);
	DEBUG_ASSERT(ii->refs >= 0, "%px: ref wrap\n", ii);

	if (ii->refs > 0) {
		int refs = ii->refs;
		spin_unlock_bh(&ecm_db_lock);
		return refs;
	}

#ifdef ECM_DB_XREF_ENABLE
	DEBUG_ASSERT((ii->nodes == NULL) && (ii->node_count == 0), "%px: nodes not null\n", ii);
#endif

	/*
	 * Remove from database if inserted
	 */
	if (!ii->flags & ECM_DB_IFACE_FLAGS_INSERTED) {
		spin_unlock_bh(&ecm_db_lock);
	} else {
		struct ecm_db_listener_instance *li;

		/*
		 * Remove from the global list
		 */
		if (!ii->prev) {
			DEBUG_ASSERT(ecm_db_interfaces == ii, "%px: interface table bad\n", ii);
			ecm_db_interfaces = ii->next;
		} else {
			ii->prev->next = ii->next;
		}
		if (ii->next) {
			ii->next->prev = ii->prev;
		}
		ii->prev = NULL;
		ii->next = NULL;

		/*
		 * Link out of hash table
		 */
		if (!ii->hash_prev) {
			DEBUG_ASSERT(ecm_db_iface_table[ii->hash_index] == ii, "%px: hash table bad got %px for hash index %u\n", ii, ecm_db_iface_table[ii->hash_index], ii->hash_index);
			ecm_db_iface_table[ii->hash_index] = ii->hash_next;
		} else {
			ii->hash_prev->hash_next = ii->hash_next;
		}
		if (ii->hash_next) {
			ii->hash_next->hash_prev = ii->hash_prev;
		}
		ii->hash_next = NULL;
		ii->hash_prev = NULL;
		ecm_db_iface_table_lengths[ii->hash_index]--;
		DEBUG_ASSERT(ecm_db_iface_table_lengths[ii->hash_index] >= 0, "%px: invalid table len %d\n", ii, ecm_db_iface_table_lengths[ii->hash_index]);

		/*
		 * Link out of interface identifier hash table
		 */
		if (!ii->iface_id_hash_prev) {
			DEBUG_ASSERT(ecm_db_iface_id_table[ii->iface_id_hash_index] == ii, "%px: hash table bad got %px for hash index %u\n", ii, ecm_db_iface_id_table[ii->iface_id_hash_index], ii->iface_id_hash_index);
			ecm_db_iface_id_table[ii->iface_id_hash_index] = ii->iface_id_hash_next;
		} else {
			ii->iface_id_hash_prev->iface_id_hash_next = ii->iface_id_hash_next;
		}
		if (ii->iface_id_hash_next) {
			ii->iface_id_hash_next->iface_id_hash_prev = ii->iface_id_hash_prev;
		}
		ii->iface_id_hash_next = NULL;
		ii->iface_id_hash_prev = NULL;
		ecm_db_iface_id_table_lengths[ii->iface_id_hash_index]--;
		DEBUG_ASSERT(ecm_db_iface_id_table_lengths[ii->iface_id_hash_index] >= 0, "%px: invalid table len %d\n", ii, ecm_db_iface_id_table_lengths[ii->iface_id_hash_index]);
		spin_unlock_bh(&ecm_db_lock);

		/*
		 * Throw removed event to listeners
		 */
		DEBUG_TRACE("%px: Throw iface removed event\n", ii);
		li = ecm_db_listeners_get_and_ref_first();
		while (li) {
			struct ecm_db_listener_instance *lin;
			if (li->iface_removed) {
				li->iface_removed(li->arg, ii);
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
	if (ii->final) {
		ii->final(ii->arg);
	}

	/*
	 * We can now destroy the instance
	 */
	DEBUG_CLEAR_MAGIC(ii);
	kfree(ii);

	/*
	 * Decrease global interface count
	 */
	spin_lock_bh(&ecm_db_lock);
	ecm_db_iface_count--;
	DEBUG_ASSERT(ecm_db_iface_count >= 0, "%px: iface count wrap\n", ii);
	spin_unlock_bh(&ecm_db_lock);

	return 0;
}
EXPORT_SYMBOL(ecm_db_iface_deref);

/*
 * ecm_db_iface_ae_interface_identifier_get()
 *	Return the accel engine interface number of this ecm interface
 */
int32_t ecm_db_iface_ae_interface_identifier_get(struct ecm_db_iface_instance *ii)
{
	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed", ii);
	return ii->ae_interface_identifier;
}
EXPORT_SYMBOL(ecm_db_iface_ae_interface_identifier_get);

/*
 * ecm_db_iface_ae_interface_identifier_set()
 *	Sets accel engine  interface number of this ecm interface
 */
void ecm_db_iface_ae_interface_identifier_set(struct ecm_db_iface_instance *ii, uint32_t num)
{
	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed", ii);
	ii->ae_interface_identifier = num;
}
EXPORT_SYMBOL(ecm_db_iface_ae_interface_identifier_set);

/*
 * ecm_db_iface_interface_identifier_get()
 *	Return the interface number of this ecm interface
 */
int32_t ecm_db_iface_interface_identifier_get(struct ecm_db_iface_instance *ii)
{
	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed", ii);
	return ii->interface_identifier;
}
EXPORT_SYMBOL(ecm_db_iface_interface_identifier_get);

/*
 * ecm_db_iface_interface_name_get()
 *	Return the interface name of this ecm interface
 *
 * name_buffer should be at least of size IFNAMSIZ
 */
void ecm_db_iface_interface_name_get(struct ecm_db_iface_instance *ii, char *name_buffer)
{
	DEBUG_CHECK_MAGIC(ii,
		ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed", ii);
	strlcpy(name_buffer, ii->name, IFNAMSIZ);
}
EXPORT_SYMBOL(ecm_db_iface_interface_name_get);

/*
 * ecm_db_iface_mtu_reset()
 *	Reset the mtu
 */
int32_t ecm_db_iface_mtu_reset(struct ecm_db_iface_instance *ii, int32_t mtu)
{
	int32_t mtu_old;
	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed", ii);
	spin_lock_bh(&ecm_db_lock);
	mtu_old = ii->mtu;
	ii->mtu = mtu;
	spin_unlock_bh(&ecm_db_lock);
	DEBUG_INFO("%px: Mtu change from %d to %d\n", ii, mtu_old, mtu);

	return mtu_old;
}
EXPORT_SYMBOL(ecm_db_iface_mtu_reset);

/*
 * ecm_db_interfaces_get_and_ref_first()
 *	Obtain a ref to the first iface instance, if any
 */
struct ecm_db_iface_instance *ecm_db_interfaces_get_and_ref_first(void)
{
	struct ecm_db_iface_instance *ii;
	spin_lock_bh(&ecm_db_lock);
	ii = ecm_db_interfaces;
	if (ii) {
		_ecm_db_iface_ref(ii);
	}
	spin_unlock_bh(&ecm_db_lock);
	return ii;
}
EXPORT_SYMBOL(ecm_db_interfaces_get_and_ref_first);

/*
 * ecm_db_interface_get_and_ref_next()
 *	Return the next iface in the list given a iface
 */
struct ecm_db_iface_instance *ecm_db_interface_get_and_ref_next(struct ecm_db_iface_instance *ii)
{
	struct ecm_db_iface_instance *iin;
	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed", ii);
	spin_lock_bh(&ecm_db_lock);
	iin = ii->next;
	if (iin) {
		_ecm_db_iface_ref(iin);
	}
	spin_unlock_bh(&ecm_db_lock);
	return iin;
}
EXPORT_SYMBOL(ecm_db_interface_get_and_ref_next);

#ifdef ECM_INTERFACE_SIT_ENABLE
/*
 * ecm_db_iface_generate_hash_index_sit()
 * 	Calculate the hash index.
 */
static inline ecm_db_iface_hash_t ecm_db_iface_generate_hash_index_sit(ip_addr_t saddr, ip_addr_t daddr)
{
	uint32_t tuple1;
	uint32_t tuple2;
	uint32_t hash_val;

	ECM_IP_ADDR_HASH(tuple1, saddr);
	ECM_IP_ADDR_HASH(tuple2, daddr);
	hash_val = (uint32_t)jhash_2words(tuple1, tuple2, ecm_db_jhash_rnd);
	return (ecm_db_iface_hash_t)(hash_val & (ECM_DB_IFACE_HASH_SLOTS - 1));
}
#endif

#ifdef ECM_INTERFACE_TUNIPIP6_ENABLE
#ifdef ECM_IPV6_ENABLE
/*
 * ecm_db_iface_generate_hash_index_tunipip6()
 * 	Calculate the hash index.
 */
static inline ecm_db_iface_hash_t ecm_db_iface_generate_hash_index_tunipip6(ip_addr_t saddr, ip_addr_t daddr)
{
	uint32_t tuple1;
	uint32_t tuple2;
	uint32_t hash_val;

	ECM_IP_ADDR_HASH(tuple1, saddr);
	ECM_IP_ADDR_HASH(tuple2, daddr);
	hash_val = (uint32_t)jhash_2words(tuple1, tuple2, ecm_db_jhash_rnd);
	return (ecm_db_iface_hash_t)(hash_val & (ECM_DB_IFACE_HASH_SLOTS - 1));
}
#endif
#endif

#ifdef ECM_INTERFACE_OVPN_ENABLE
/*
 * ecm_db_iface_generate_hash_index_ovpn()
 * 	Calculate the hash index.
 */
static inline ecm_db_iface_hash_t ecm_db_iface_generate_hash_index_ovpn(int32_t tun_ifnum)
{
	uint32_t hash_val;

	hash_val = (uint32_t)jhash_1word((uint32_t)tun_ifnum, ecm_db_jhash_rnd);
	return (ecm_db_iface_hash_t)(hash_val & (ECM_DB_IFACE_HASH_SLOTS - 1));
}
#endif

/*
 * ecm_db_iface_generate_hash_index_ethernet()
 * 	Calculate the hash index.
 */
static inline ecm_db_iface_hash_t ecm_db_iface_generate_hash_index_ethernet(uint8_t *address)
{
	uint32_t hash_val;
	hash_val = (uint32_t)jhash(address, 6, ecm_db_jhash_rnd);
	return (ecm_db_iface_hash_t)(hash_val & (ECM_DB_IFACE_HASH_SLOTS - 1));
}

#ifdef ECM_INTERFACE_PPPOE_ENABLE
/*
 * ecm_db_iface_generate_hash_index_pppoe()
 * 	Calculate the hash index.
 */
static inline ecm_db_iface_hash_t ecm_db_iface_generate_hash_index_pppoe(uint16_t pppoe_session_id)
{
	uint32_t hash_val;
	hash_val = (uint32_t)jhash_1word((uint32_t)pppoe_session_id, ecm_db_jhash_rnd);
	return (ecm_db_iface_hash_t)(hash_val & (ECM_DB_IFACE_HASH_SLOTS - 1));
}
#endif

#ifdef ECM_INTERFACE_L2TPV2_ENABLE
/*
 * ecm_db_iface_generate_hash_index_pppol2tpv2()
 *	Calculate the hash index.
 */
static inline ecm_db_iface_hash_t ecm_db_iface_generate_hash_index_pppol2tpv2(uint32_t pppol2tpv2_tunnel_id, uint32_t pppol2tpv2_session_id)
{
	uint32_t hash_val;
	hash_val = (uint32_t)jhash_2words(pppol2tpv2_tunnel_id, pppol2tpv2_session_id, ecm_db_jhash_rnd);
	return (ecm_db_iface_hash_t)(hash_val & (ECM_DB_IFACE_HASH_SLOTS - 1));
}

#endif

#ifdef ECM_INTERFACE_PPTP_ENABLE
/*
 * ecm_db_iface_generate_hash_index_pptp()
 *	Calculate the hash index.
 */
static inline ecm_db_iface_hash_t ecm_db_iface_generate_hash_index_pptp(uint16_t pptp_src_call_id, uint16_t pptp_dst_call_id)
{
	uint32_t hash_val;
	hash_val = (uint32_t)jhash_2words(pptp_src_call_id, pptp_dst_call_id, ecm_db_jhash_rnd);
	return (ecm_db_iface_hash_t)(hash_val & (ECM_DB_IFACE_HASH_SLOTS - 1));
}
#endif

#ifdef ECM_INTERFACE_MAP_T_ENABLE
/*
 * ecm_db_iface_generate_hash_index_map_t()
 *	Calculate the hash index.
 */
static inline ecm_db_iface_hash_t ecm_db_iface_generate_hash_index_map_t(int if_index)
{
	uint32_t hash_val;
	hash_val = (uint32_t)jhash_1word(if_index, ecm_db_jhash_rnd);
	return (ecm_db_iface_hash_t)(hash_val & (ECM_DB_IFACE_HASH_SLOTS - 1));
}
#endif

#ifdef ECM_INTERFACE_GRE_TUN_ENABLE
/*
 * ecm_db_iface_generate_hash_index_gre_tun()
 * 	Calculate the hash index.
 */
static inline ecm_db_iface_hash_t ecm_db_iface_generate_hash_index_gre_tun(int if_index)
{
	uint32_t hash_val;
	hash_val = (uint32_t)jhash_1word(if_index, ecm_db_jhash_rnd);
	return (ecm_db_iface_hash_t)(hash_val & (ECM_DB_IFACE_HASH_SLOTS - 1));
}
#endif

/*
 * ecm_db_iface_generate_hash_index_unknown()
 * 	Calculate the hash index.
 */
static inline ecm_db_iface_hash_t ecm_db_iface_generate_hash_index_unknown(uint32_t os_specific_ident)
{
	uint32_t hash_val;
	hash_val = (uint32_t)jhash_1word(os_specific_ident, ecm_db_jhash_rnd);
	return (ecm_db_iface_hash_t)(hash_val & (ECM_DB_IFACE_HASH_SLOTS - 1));
}

/*
 * ecm_db_iface_generate_hash_index_loopback()
 * 	Calculate the hash index.
 */
static inline ecm_db_iface_hash_t ecm_db_iface_generate_hash_index_loopback(uint32_t os_specific_ident)
{
	uint32_t hash_val;
	hash_val = (uint32_t)jhash_1word(os_specific_ident, ecm_db_jhash_rnd);
	return (ecm_db_iface_hash_t)(hash_val & (ECM_DB_IFACE_HASH_SLOTS - 1));
}

#ifdef ECM_INTERFACE_IPSEC_ENABLE
/*
 * ecm_db_iface_generate_hash_index_ipsec_tunnel()
 * 	Calculate the hash index.
 * GGG TODO Flesh this out using actual tunnel endpoint keys
 */
static inline ecm_db_iface_hash_t ecm_db_iface_generate_hash_index_ipsec_tunnel(uint32_t os_specific_ident)
{
	uint32_t hash_val;
	hash_val = (uint32_t)jhash_1word(os_specific_ident, ecm_db_jhash_rnd);
	return (ecm_db_iface_hash_t)(hash_val & (ECM_DB_IFACE_HASH_SLOTS - 1));
}
#endif

#ifdef ECM_INTERFACE_VXLAN_ENABLE
/*
 * ecm_db_iface_generate_hash_index_vxlan()
 *	Calculate the hash index based on VxLAN network identifier and interface type.
 */
static inline ecm_db_iface_hash_t ecm_db_iface_generate_hash_index_vxlan(uint32_t vni, uint32_t if_type)
{
	uint32_t hash_val;
	hash_val = (uint32_t)jhash_2words(vni, if_type, ecm_db_jhash_rnd);
	return (ecm_db_iface_hash_t)(hash_val & (ECM_DB_IFACE_HASH_SLOTS - 1));
}
#endif

/*
 * ecm_db_iface_ethernet_address_get()
 *	Obtain the ethernet address for an ethernet interface
 */
void ecm_db_iface_ethernet_address_get(struct ecm_db_iface_instance *ii, uint8_t *address)
{
	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed", ii);
	DEBUG_ASSERT(ii->type == ECM_DB_IFACE_TYPE_ETHERNET, "%px: Bad type, expected ethernet, actual: %d\n", ii, ii->type);
	spin_lock_bh(&ecm_db_lock);
	ether_addr_copy(address, ii->type_info.ethernet.address);
	spin_unlock_bh(&ecm_db_lock);
}
EXPORT_SYMBOL(ecm_db_iface_ethernet_address_get);

/*
 * ecm_db_iface_bridge_address_get()
 *	Obtain the ethernet address for a bridge interface
 */
void ecm_db_iface_bridge_address_get(struct ecm_db_iface_instance *ii, uint8_t *address)
{
	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed", ii);
	DEBUG_ASSERT(ii->type == ECM_DB_IFACE_TYPE_BRIDGE, "%px: Bad type, expected bridge, actual: %d\n", ii, ii->type);
	spin_lock_bh(&ecm_db_lock);
	ether_addr_copy(address, ii->type_info.bridge.address);
	spin_unlock_bh(&ecm_db_lock);
}
EXPORT_SYMBOL(ecm_db_iface_bridge_address_get);

#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
/*
 * ecm_db_iface_ovs_bridge_address_get()
 *	Obtain the ethernet address for a ovs bridge interface
 */
void ecm_db_iface_ovs_bridge_address_get(struct ecm_db_iface_instance *ii, uint8_t *address)
{
	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed", ii);
	DEBUG_ASSERT(ii->type == ECM_DB_IFACE_TYPE_OVS_BRIDGE, "%px: Bad type, expected ovs bridge, actual: %d\n", ii, ii->type);
	spin_lock_bh(&ecm_db_lock);
	ether_addr_copy(address, ii->type_info.ovsb.address);
	spin_unlock_bh(&ecm_db_lock);
}
#endif

#ifdef ECM_INTERFACE_BOND_ENABLE
/*
 * ecm_db_iface_lag_address_get()
 *	Obtain the ethernet address for a LAG interface
 */
void ecm_db_iface_lag_address_get(struct ecm_db_iface_instance *ii, uint8_t *address)
{
	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed", ii);
	DEBUG_ASSERT(ii->type == ECM_DB_IFACE_TYPE_LAG, "%px: Bad type, expected LAG, actual: %d\n", ii, ii->type);
	spin_lock_bh(&ecm_db_lock);
	ether_addr_copy(address, ii->type_info.lag.address);
	spin_unlock_bh(&ecm_db_lock);
}
EXPORT_SYMBOL(ecm_db_iface_lag_address_get);
#endif

/*
 * _ecm_db_iface_identifier_hash_table_insert_entry()
 *	Calculate the hash index based on updated interface_identifier, and
 *	re-insert into interface identifier chain.
 *
 *	Note: Must take ecm_db_lock before calling this.
 */
static void _ecm_db_iface_identifier_hash_table_insert_entry(struct ecm_db_iface_instance *ii, int32_t interface_identifier)
{
	ecm_db_iface_id_hash_t iface_id_hash_index;

	/*
	 * Compute hash chain for insertion
	 */
	iface_id_hash_index = ecm_db_iface_id_generate_hash_index(interface_identifier);
	ii->iface_id_hash_index = iface_id_hash_index;

	/*
	 * Insert into interface identifier chain
	 */
	ii->iface_id_hash_next = ecm_db_iface_id_table[iface_id_hash_index];
	if (ecm_db_iface_id_table[iface_id_hash_index]) {
		ecm_db_iface_id_table[iface_id_hash_index]->iface_id_hash_prev = ii;
	}

	ecm_db_iface_id_table[iface_id_hash_index] = ii;
	ecm_db_iface_id_table_lengths[iface_id_hash_index]++;
	DEBUG_ASSERT(ecm_db_iface_id_table_lengths[iface_id_hash_index] > 0, "%px: invalid iface id table len %d\n", ii, ecm_db_iface_id_table_lengths[iface_id_hash_index]);
}

/*
 * _ecm_db_iface_identifier_hash_table_remove_entry()
 * 	Remove an entry of a given interface instance from interface identifier chain.
 *
 *	Note: Must take ecm_db_lock before calling this.
 */
static void _ecm_db_iface_identifier_hash_table_remove_entry(struct ecm_db_iface_instance *ii)
{
	/*
	 * Remove from database if inserted
	 */
	if (!ii->flags & ECM_DB_IFACE_FLAGS_INSERTED) {
		return;
	}

	/*
	 * Link out of interface identifier hash table
	 */
	if (!ii->iface_id_hash_prev) {
		DEBUG_ASSERT(ecm_db_iface_id_table[ii->iface_id_hash_index] == ii, "%px: hash table bad got %px for hash index %u\n", ii, ecm_db_iface_id_table[ii->iface_id_hash_index], ii->iface_id_hash_index);
		ecm_db_iface_id_table[ii->iface_id_hash_index] = ii->iface_id_hash_next;
	} else {
		ii->iface_id_hash_prev->iface_id_hash_next = ii->iface_id_hash_next;
	}

	if (ii->iface_id_hash_next) {
		ii->iface_id_hash_next->iface_id_hash_prev = ii->iface_id_hash_prev;
	}

	ii->iface_id_hash_next = NULL;
	ii->iface_id_hash_prev = NULL;
	ecm_db_iface_id_table_lengths[ii->iface_id_hash_index]--;
	DEBUG_ASSERT(ecm_db_iface_id_table_lengths[ii->iface_id_hash_index] >= 0, "%px: invalid table len %d\n", ii, ecm_db_iface_id_table_lengths[ii->iface_id_hash_index]);
}

/*
 * ecm_db_iface_identifier_hash_table_entry_check_and_update()
 * 	Update the hash table entry of interface identifier hash table.
 * 	First remove the 'ii' from curent hash index position, re-calculate new hash and re-insert
 * 	the 'ii' at new hash index position into interface identifier hash table.
 */
void ecm_db_iface_identifier_hash_table_entry_check_and_update(struct ecm_db_iface_instance *ii, int32_t new_interface_identifier)
{
	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed", ii);
	spin_lock_bh(&ecm_db_lock);
	if (ii->interface_identifier == new_interface_identifier) {
		spin_unlock_bh(&ecm_db_lock);
		return;
	}

	DEBUG_TRACE("%px: interface ifindex has changed Old %d, New %d \n", ii, ii->interface_identifier, new_interface_identifier);
	_ecm_db_iface_identifier_hash_table_remove_entry(ii);
	ii->interface_identifier = new_interface_identifier;
	_ecm_db_iface_identifier_hash_table_insert_entry(ii, new_interface_identifier);
	spin_unlock_bh(&ecm_db_lock);
}
EXPORT_SYMBOL(ecm_db_iface_identifier_hash_table_entry_check_and_update);

/*
 * ecm_db_iface_find_and_ref_by_interface_identifier()
 *	Return an interface based on a hlos interface identifier
 */
struct ecm_db_iface_instance *ecm_db_iface_find_and_ref_by_interface_identifier(int32_t interface_id)
{
	ecm_db_iface_id_hash_t hash_index;
	struct ecm_db_iface_instance *ii;

	DEBUG_TRACE("Lookup database iface with interface_id %d\n", interface_id);

	/*
	 * Compute the hash chain index and prepare to walk the chain
	 */
	hash_index = ecm_db_iface_id_generate_hash_index(interface_id);

	/*
	 * Iterate the chain looking for a host with matching details
	 */
	spin_lock_bh(&ecm_db_lock);
	ii = ecm_db_iface_id_table[hash_index];
	while (ii) {
		if (ii->interface_identifier == interface_id) {
			_ecm_db_iface_ref(ii);
			spin_unlock_bh(&ecm_db_lock);
			DEBUG_TRACE("iface found %px\n", ii);
			return ii;
		}

		/*
		 * Try next
		 */
		ii = ii->iface_id_hash_next;
	}
	spin_unlock_bh(&ecm_db_lock);
	DEBUG_TRACE("Iface not found\n");
	return NULL;
}
EXPORT_SYMBOL(ecm_db_iface_find_and_ref_by_interface_identifier);

/*
 * ecm_db_iface_ifidx_find_and_ref_ethernet()
 *	Return an interface based on a MAC address and interface hlos interface identifier
 */
struct ecm_db_iface_instance *ecm_db_iface_ifidx_find_and_ref_ethernet(uint8_t *address, int32_t ifidx, int32_t ae_interface_num)
{
	ecm_db_iface_hash_t hash_index;
	struct ecm_db_iface_instance *ii;

	DEBUG_TRACE("Lookup ethernet iface with addr %pM\n", address);

	/*
	 * Compute the hash chain index and prepare to walk the chain
	 */
	hash_index = ecm_db_iface_generate_hash_index_ethernet(address);

	/*
	 * Iterate the chain looking for a host with matching details
	 */
	spin_lock_bh(&ecm_db_lock);
	ii = ecm_db_iface_table[hash_index];
	while (ii) {
		if ((ii->type != ECM_DB_IFACE_TYPE_ETHERNET)
		    || memcmp(ii->type_info.ethernet.address, address, ETH_ALEN)
		    || (ii->interface_identifier != ifidx)
		    || (ii->ae_interface_identifier != ae_interface_num)) {
			ii = ii->hash_next;
			continue;
		}

		_ecm_db_iface_ref(ii);
		spin_unlock_bh(&ecm_db_lock);
		DEBUG_TRACE("iface found %px\n", ii);
		return ii;
	}
	spin_unlock_bh(&ecm_db_lock);
	DEBUG_TRACE("Iface not found\n");
	return NULL;
}
EXPORT_SYMBOL(ecm_db_iface_ifidx_find_and_ref_ethernet);

#ifdef ECM_INTERFACE_VLAN_ENABLE
/*
 * ecm_db_iface_vlan_info_get()
 *	Get vlan interface specific information
 */
void ecm_db_iface_vlan_info_get(struct ecm_db_iface_instance *ii, struct ecm_db_interface_info_vlan *vlan_info)
{
	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed", ii);
	DEBUG_ASSERT(ii->type == ECM_DB_IFACE_TYPE_VLAN, "%px: Bad type, expected vlan, actual: %d\n", ii, ii->type);
	spin_lock_bh(&ecm_db_lock);
	ether_addr_copy(vlan_info->address, ii->type_info.vlan.address);
	vlan_info->vlan_tag = ii->type_info.vlan.vlan_tag;
	vlan_info->vlan_tpid = ii->type_info.vlan.vlan_tpid;
	spin_unlock_bh(&ecm_db_lock);
}
EXPORT_SYMBOL(ecm_db_iface_vlan_info_get);

/*
 * ecm_db_iface_find_and_ref_vlan()
 *	Lookup and return a iface reference if any
 */
struct ecm_db_iface_instance *ecm_db_iface_find_and_ref_vlan(uint8_t *address, uint16_t vlan_tag, uint16_t vlan_tpid)
{
	ecm_db_iface_hash_t hash_index;
	struct ecm_db_iface_instance *ii;

	DEBUG_TRACE("Lookup vlan iface with addr %pM, vlan tag: %x vlan tpid: %x\n", address, vlan_tag, vlan_tpid);

	/*
	 * Compute the hash chain index and prepare to walk the chain
	 */
	hash_index = ecm_db_iface_generate_hash_index_ethernet(address);

	/*
	 * Iterate the chain looking for a host with matching details
	 */
	spin_lock_bh(&ecm_db_lock);
	ii = ecm_db_iface_table[hash_index];
	while (ii) {
		if ((ii->type != ECM_DB_IFACE_TYPE_VLAN) || (ii->type_info.vlan.vlan_tag != vlan_tag)
				|| (ii->type_info.vlan.vlan_tpid != vlan_tpid)
				|| memcmp(ii->type_info.vlan.address, address, ETH_ALEN)) {
			ii = ii->hash_next;
			continue;
		}

		_ecm_db_iface_ref(ii);
		spin_unlock_bh(&ecm_db_lock);
		DEBUG_TRACE("iface found %px\n", ii);
		return ii;
	}
	spin_unlock_bh(&ecm_db_lock);
	DEBUG_TRACE("Iface not found\n");
	return NULL;
}
EXPORT_SYMBOL(ecm_db_iface_find_and_ref_vlan);
#endif

#ifdef ECM_INTERFACE_MACVLAN_ENABLE
/*
 * ecm_db_iface_find_and_ref_macvlan()
 *	Lookup and return a iface reference if any
 */
struct ecm_db_iface_instance *ecm_db_iface_find_and_ref_macvlan(uint8_t *address)
{
	ecm_db_iface_hash_t hash_index;
	struct ecm_db_iface_instance *ii;

	DEBUG_TRACE("Lookup macvlan iface with addr %pM\n", address);

	/*
	 * Compute the hash chain index and prepare to walk the chain
	 */
	hash_index = ecm_db_iface_generate_hash_index_ethernet(address);

	/*
	 * Iterate the chain looking for a host with matching details
	 */
	spin_lock_bh(&ecm_db_lock);
	ii = ecm_db_iface_table[hash_index];
	while (ii) {
		if ((ii->type != ECM_DB_IFACE_TYPE_MACVLAN) || !ether_addr_equal(ii->type_info.macvlan.address, address)) {
			ii = ii->hash_next;
			continue;
		}

		_ecm_db_iface_ref(ii);
		spin_unlock_bh(&ecm_db_lock);
		DEBUG_TRACE("iface found %px\n", ii);
		return ii;
	}
	spin_unlock_bh(&ecm_db_lock);
	DEBUG_TRACE("Iface not found\n");
	return NULL;
}

/*
 * ecm_db_iface_macvlan_address_get()
 *	Obtain the ethernet address for a macvlan interface
 */
void ecm_db_iface_macvlan_address_get(struct ecm_db_iface_instance *ii, uint8_t *address)
{
	spin_lock_bh(&ecm_db_lock);
	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed", ii);
	DEBUG_ASSERT(ii->type == ECM_DB_IFACE_TYPE_MACVLAN, "%px: Bad type, expected macvlan, actual: %d\n", ii, ii->type);
	ether_addr_copy(address, ii->type_info.macvlan.address);
	spin_unlock_bh(&ecm_db_lock);
}
#endif

#ifdef ECM_INTERFACE_VXLAN_ENABLE
/*
 * ecm_db_iface_vxlan_info_get()
 *     Get vxlan interface specific information
 */
void ecm_db_iface_vxlan_info_get(struct ecm_db_iface_instance *ii, struct ecm_db_interface_info_vxlan *vxlan_info)
{
	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%p: magic failed", ii);
	DEBUG_ASSERT(ii->type == ECM_DB_IFACE_TYPE_VXLAN, "%p: Bad type, expected vxlan, actual: %d\n", ii, ii->type);
	spin_lock_bh(&ecm_db_lock);
	vxlan_info->vni = ii->type_info.vxlan.vni;
	vxlan_info->if_type = ii->type_info.vxlan.if_type;
	spin_unlock_bh(&ecm_db_lock);
}

/*
 * ecm_db_iface_find_and_ref_vxlan()
 *	Lookup and return a iface reference if any
 */
struct ecm_db_iface_instance *ecm_db_iface_find_and_ref_vxlan(uint32_t vni, uint32_t type)
{
	ecm_db_iface_hash_t hash_index;
	struct ecm_db_iface_instance *ii;

	DEBUG_TRACE("Lookup vxlan iface with vxlan id: %d & if_type: %d\n", vni, type);

	/*
	 * Compute the hash chain index and prepare to walk the chain
	 */
	hash_index = ecm_db_iface_generate_hash_index_vxlan(vni, type);

	/*
	 * Iterate the chain looking for a host with matching details
	 */
	spin_lock_bh(&ecm_db_lock);
	ii = ecm_db_iface_table[hash_index];
	while (ii) {
		if ((ii->type != ECM_DB_IFACE_TYPE_VXLAN)
			|| (ii->type_info.vxlan.vni != vni)
			|| (ii->type_info.vxlan.if_type != type)) {
			ii = ii->hash_next;
			continue;
		}

		_ecm_db_iface_ref(ii);
		spin_unlock_bh(&ecm_db_lock);
		DEBUG_TRACE("iface found %px\n", ii);
		return ii;
	}
	spin_unlock_bh(&ecm_db_lock);
	DEBUG_TRACE("Iface not found\n");
	return NULL;
}
#endif

/*
 * ecm_db_iface_find_and_ref_bridge()
 *	Lookup and return a iface reference if any
 */
struct ecm_db_iface_instance *ecm_db_iface_find_and_ref_bridge(uint8_t *address, int32_t if_num)
{
	ecm_db_iface_hash_t hash_index;
	struct ecm_db_iface_instance *ii;

	DEBUG_TRACE("Lookup bridge iface with addr %pM\n", address);

	/*
	 * Compute the hash chain index and prepare to walk the chain
	 */
	hash_index = ecm_db_iface_generate_hash_index_ethernet(address);

	/*
	 * Iterate the chain looking for a host with matching details
	 */
	spin_lock_bh(&ecm_db_lock);
	ii = ecm_db_iface_table[hash_index];
	while (ii) {
		if ((ii->type != ECM_DB_IFACE_TYPE_BRIDGE)
			|| memcmp(ii->type_info.bridge.address, address, ETH_ALEN)
			|| ii->interface_identifier != if_num) {

			ii = ii->hash_next;
			continue;
		}

		_ecm_db_iface_ref(ii);
		spin_unlock_bh(&ecm_db_lock);
		DEBUG_TRACE("iface found %px\n", ii);
		return ii;
	}
	spin_unlock_bh(&ecm_db_lock);
	DEBUG_TRACE("Iface not found\n");
	return NULL;
}
EXPORT_SYMBOL(ecm_db_iface_find_and_ref_bridge);

#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
/*
 * ecm_db_iface_find_and_ref_ovs_bridge()
 *	Lookup and return a iface reference if any
 */
struct ecm_db_iface_instance *ecm_db_iface_find_and_ref_ovs_bridge(uint8_t *address, int32_t if_num)
{
	ecm_db_iface_hash_t hash_index;
	struct ecm_db_iface_instance *ii;

	DEBUG_TRACE("Lookup OVS bridge iface with addr %pM\n", address);

	/*
	 * Compute the hash chain index and prepare to walk the chain
	 */
	hash_index = ecm_db_iface_generate_hash_index_ethernet(address);

	/*
	 * Iterate the chain looking for a host with matching details
	 */
	spin_lock_bh(&ecm_db_lock);
	ii = ecm_db_iface_table[hash_index];
	while (ii) {
		if ((ii->type != ECM_DB_IFACE_TYPE_OVS_BRIDGE)
			|| memcmp(ii->type_info.ovsb.address, address, ETH_ALEN)
			|| (ii->interface_identifier != if_num)) {
			ii = ii->hash_next;
			continue;
		}

		_ecm_db_iface_ref(ii);
		spin_unlock_bh(&ecm_db_lock);
		DEBUG_TRACE("iface found %px\n", ii);
		return ii;
	}
	spin_unlock_bh(&ecm_db_lock);
	DEBUG_TRACE("Iface not found\n");
	return NULL;
}
#endif

#ifdef ECM_INTERFACE_BOND_ENABLE
/*
 * ecm_db_iface_find_and_ref_lag()
 *	Lookup and return a iface reference if any
 */
struct ecm_db_iface_instance *ecm_db_iface_find_and_ref_lag(uint8_t *address)
{
	ecm_db_iface_hash_t hash_index;
	struct ecm_db_iface_instance *ii;

	DEBUG_TRACE("Lookup lag iface with addr %pM\n", address);

	/*
	 * Compute the hash chain index and prepare to walk the chain
	 */
	hash_index = ecm_db_iface_generate_hash_index_ethernet(address);

	/*
	 * Iterate the chain looking for a host with matching details
	 */
	spin_lock_bh(&ecm_db_lock);
	ii = ecm_db_iface_table[hash_index];
	while (ii) {
		if ((ii->type != ECM_DB_IFACE_TYPE_LAG) || memcmp(ii->type_info.lag.address, address, ETH_ALEN)) {
			ii = ii->hash_next;
			continue;
		}

		_ecm_db_iface_ref(ii);
		spin_unlock_bh(&ecm_db_lock);
		DEBUG_TRACE("iface found %px\n", ii);
		return ii;
	}
	spin_unlock_bh(&ecm_db_lock);
	DEBUG_TRACE("Iface not found\n");
	return NULL;
}
EXPORT_SYMBOL(ecm_db_iface_find_and_ref_lag);
#endif

#ifdef ECM_INTERFACE_PPPOE_ENABLE
/*
 * ecm_db_iface_pppoe_session_info_get()
 *	Get pppoe interface specific information
 */
void ecm_db_iface_pppoe_session_info_get(struct ecm_db_iface_instance *ii, struct ecm_db_interface_info_pppoe *pppoe_info)
{
	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed", ii);
	DEBUG_ASSERT(ii->type == ECM_DB_IFACE_TYPE_PPPOE, "%px: Bad type, expected pppoe, actual: %d\n", ii, ii->type);
	spin_lock_bh(&ecm_db_lock);
	ether_addr_copy(pppoe_info->remote_mac, ii->type_info.pppoe.remote_mac);
	pppoe_info->pppoe_session_id = ii->type_info.pppoe.pppoe_session_id;
	spin_unlock_bh(&ecm_db_lock);
}

EXPORT_SYMBOL(ecm_db_iface_pppoe_session_info_get);

/*
 * ecm_db_iface_find_and_ref_pppoe()
 *	Lookup and return a iface reference if any
 */
struct ecm_db_iface_instance *ecm_db_iface_find_and_ref_pppoe(uint16_t pppoe_session_id, uint8_t *remote_mac)
{
	ecm_db_iface_hash_t hash_index;
	struct ecm_db_iface_instance *ii;

	DEBUG_TRACE("Lookup pppoe iface with addr %x\n", pppoe_session_id);

	/*
	 * Compute the hash chain index and prepare to walk the chain
	 */
	hash_index = ecm_db_iface_generate_hash_index_pppoe(pppoe_session_id);

	/*
	 * Iterate the chain looking for a host with matching details
	 */
	spin_lock_bh(&ecm_db_lock);
	ii = ecm_db_iface_table[hash_index];
	while (ii) {
		if ((ii->type != ECM_DB_IFACE_TYPE_PPPOE)
				|| (ii->type_info.pppoe.pppoe_session_id != pppoe_session_id)
				|| memcmp(ii->type_info.pppoe.remote_mac, remote_mac, ETH_ALEN)) {
			ii = ii->hash_next;
			continue;
		}

		_ecm_db_iface_ref(ii);
		spin_unlock_bh(&ecm_db_lock);
		DEBUG_TRACE("iface found %px\n", ii);
		return ii;
	}
	spin_unlock_bh(&ecm_db_lock);
	DEBUG_TRACE("Iface not found\n");
	return NULL;
}
EXPORT_SYMBOL(ecm_db_iface_find_and_ref_pppoe);
#endif

/*
 * ecm_db_iface_update_ae_interface_identifier()
 *	update ae_interface_identifier in iface instance.
 */
void ecm_db_iface_update_ae_interface_identifier(struct ecm_db_iface_instance *ii, int32_t ae_interface_identifier)
{
	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed", ii);

	spin_lock_bh(&ecm_db_lock);
	if (ii->ae_interface_identifier == ae_interface_identifier) {
		spin_unlock_bh(&ecm_db_lock);
		return;
	}
	ii->ae_interface_identifier = ae_interface_identifier;
	spin_unlock_bh(&ecm_db_lock);
}
EXPORT_SYMBOL(ecm_db_iface_update_ae_interface_identifier);

#ifdef ECM_INTERFACE_L2TPV2_ENABLE
/*
 * ecm_db_iface_pppol2tpv2_session_info_get
 *	get l2tpv2 specific info
 */
void ecm_db_iface_pppol2tpv2_session_info_get(struct ecm_db_iface_instance *ii, struct ecm_db_interface_info_pppol2tpv2 *pppol2tpv2_info)
{
	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed", ii);
	DEBUG_ASSERT(ii->type == ECM_DB_IFACE_TYPE_PPPOL2TPV2, "%px: Bad type, expected pppol2tpv2, actual: %d\n", ii, ii->type);
	spin_lock_bh(&ecm_db_lock);
	memcpy(pppol2tpv2_info, &ii->type_info.pppol2tpv2, sizeof(struct ecm_db_interface_info_pppol2tpv2));
	spin_unlock_bh(&ecm_db_lock);
}
EXPORT_SYMBOL(ecm_db_iface_pppol2tpv2_session_info_get);

/*
 * ecm_db_iface_find_and_ref_pppol2tpv2()
 *	Lookup and return a iface reference if any
 */
struct ecm_db_iface_instance *ecm_db_iface_find_and_ref_pppol2tpv2(uint32_t pppol2tpv2_tunnel_id, uint32_t pppol2tpv2_session_id)
{
	ecm_db_iface_hash_t hash_index;
	struct ecm_db_iface_instance *ii;

	/*
	 * Compute the hash chain index and prepare to walk the chain
	 */
	hash_index = ecm_db_iface_generate_hash_index_pppol2tpv2(pppol2tpv2_tunnel_id, pppol2tpv2_session_id);

	DEBUG_TRACE("Lookup pppol2tpv2 iface with local_tunnel_id = %d, local_session_id = %d, hash = 0x%x\n", pppol2tpv2_tunnel_id,
									pppol2tpv2_session_id, hash_index);

	/*
	 * Iterate the chain looking for a host with matching details
	 */
	spin_lock_bh(&ecm_db_lock);
	ii = ecm_db_iface_table[hash_index];

	while (ii) {
		if ((ii->type != ECM_DB_IFACE_TYPE_PPPOL2TPV2)
				|| (ii->type_info.pppol2tpv2.l2tp.session.session_id != pppol2tpv2_session_id)
				|| (ii->type_info.pppol2tpv2.l2tp.tunnel.tunnel_id != pppol2tpv2_tunnel_id)) {
			ii = ii->hash_next;
			continue;
		}

		_ecm_db_iface_ref(ii);
		spin_unlock_bh(&ecm_db_lock);
		DEBUG_TRACE("iface found %px\n", ii);
		return ii;
	}
	spin_unlock_bh(&ecm_db_lock);

	DEBUG_TRACE("Iface not found\n");
	return NULL;
}
EXPORT_SYMBOL(ecm_db_iface_find_and_ref_pppol2tpv2);

#endif

#ifdef ECM_INTERFACE_PPTP_ENABLE
/*
 * ecm_db_iface_pptp_session_info_get
 *	get pptp specific info
 */
void ecm_db_iface_pptp_session_info_get(struct ecm_db_iface_instance *ii, struct ecm_db_interface_info_pptp *pptp_info)
{
	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed", ii);
	DEBUG_ASSERT(ii->type == ECM_DB_IFACE_TYPE_PPTP, "%px: Bad type, expected pptp, actual: %d\n", ii, ii->type);
	spin_lock_bh(&ecm_db_lock);
	memcpy(pptp_info, &ii->type_info.pptp, sizeof(struct ecm_db_interface_info_pptp));
	spin_unlock_bh(&ecm_db_lock);
}
EXPORT_SYMBOL(ecm_db_iface_pptp_session_info_get);

/*
 * ecm_db_iface_find_and_ref_pptp()
 *	Lookup and return a iface reference if any
 */
struct ecm_db_iface_instance *ecm_db_iface_find_and_ref_pptp(uint32_t pptp_src_call_id, uint32_t pptp_dst_call_id, int32_t ae_interface_num)
{
	ecm_db_iface_hash_t hash_index;
	struct ecm_db_iface_instance *ii;

	/*
	 * Compute the hash chain index and prepare to walk the chain
	 */
	hash_index = ecm_db_iface_generate_hash_index_pptp(pptp_src_call_id, pptp_dst_call_id);

	DEBUG_TRACE("Lookup pptp iface with local_call_id = %d, remote_call_id = %d, hash = 0x%x\n", pptp_src_call_id,
									pptp_dst_call_id, hash_index);

	/*
	 * Iterate the chain looking for a host with matching details
	 */
	spin_lock_bh(&ecm_db_lock);
	ii = ecm_db_iface_table[hash_index];

	while (ii) {
		if ((ii->type != ECM_DB_IFACE_TYPE_PPTP)
				|| (ii->type_info.pptp.src_call_id != pptp_src_call_id)
				|| (ii->type_info.pptp.dst_call_id != pptp_dst_call_id)
				|| (ii->ae_interface_identifier != ae_interface_num)) {
			ii = ii->hash_next;
			continue;
		}

		_ecm_db_iface_ref(ii);
		spin_unlock_bh(&ecm_db_lock);
		DEBUG_TRACE("iface found %px\n", ii);
		return ii;
	}
	spin_unlock_bh(&ecm_db_lock);

	DEBUG_TRACE("Iface not found\n");
	return NULL;
}
EXPORT_SYMBOL(ecm_db_iface_find_and_ref_pptp);
#endif

#ifdef ECM_INTERFACE_MAP_T_ENABLE
/*
 * ecm_db_iface_map_t_info_get
 *	get map_t specific info
 */
void ecm_db_iface_map_t_info_get(struct ecm_db_iface_instance *ii, struct ecm_db_interface_info_map_t *map_t_info)
{
	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed", ii);
	DEBUG_ASSERT(ii->type == ECM_DB_IFACE_TYPE_MAP_T, "%px: Bad type, expected map_t, actual: %d\n", ii, ii->type);
	spin_lock_bh(&ecm_db_lock);
	memcpy(map_t_info, &ii->type_info.map_t, sizeof(struct ecm_db_interface_info_map_t));
	spin_unlock_bh(&ecm_db_lock);
}
EXPORT_SYMBOL(ecm_db_iface_map_t_info_get);

/*
 * ecm_db_iface_find_and_ref_map_t()
 *	Lookup and return a iface reference if any
 */
struct ecm_db_iface_instance *ecm_db_iface_find_and_ref_map_t(int if_index, int32_t ae_interface_num)
{
	ecm_db_iface_hash_t hash_index;
	struct ecm_db_iface_instance *ii;

	DEBUG_TRACE("Lookup map_t iface with if_index = %d\n", if_index);

	/*
	 * Compute the hash chain index and prepare to walk the chain
	 */
	hash_index = ecm_db_iface_generate_hash_index_map_t(if_index);

	/*
	 * Iterate the chain looking for a host with matching details
	 */
	spin_lock_bh(&ecm_db_lock);
	ii = ecm_db_iface_table[hash_index];

	while (ii) {
		if ((ii->type != ECM_DB_IFACE_TYPE_MAP_T)
				|| (ii->type_info.map_t.if_index != if_index)
				|| (ii->ae_interface_identifier != ae_interface_num)) {
			ii = ii->hash_next;
			continue;
		}

		_ecm_db_iface_ref(ii);
		spin_unlock_bh(&ecm_db_lock);
		DEBUG_TRACE("%px: iface found\n", ii);
		return ii;
	}
	spin_unlock_bh(&ecm_db_lock);

	DEBUG_TRACE("Iface not found\n");
	return NULL;
}
EXPORT_SYMBOL(ecm_db_iface_find_and_ref_map_t);

#endif

#ifdef ECM_INTERFACE_GRE_TUN_ENABLE
/*
 * ecm_db_iface_gre_tun_info_get
 * 	Get gre specific info
 */
void ecm_db_iface_gre_tun_info_get(struct ecm_db_iface_instance *ii, struct ecm_db_interface_info_gre_tun *gre_tun_info)
{
	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed", ii);
	DEBUG_ASSERT(ii->type == ECM_DB_IFACE_TYPE_GRE_TUN, "%px: Bad type, expected gre, actual: %d\
			n", ii, ii->type);
	spin_lock_bh(&ecm_db_lock);
	memcpy(gre_tun_info, &ii->type_info.gre_tun, sizeof(struct ecm_db_interface_info_gre_tun));
	spin_unlock_bh(&ecm_db_lock);
}
EXPORT_SYMBOL(ecm_db_iface_gre_tun_info_get);

/*
 * ecm_db_iface_find_and_ref_gre()
 * 	Lookup and return a iface reference if any
 */
struct ecm_db_iface_instance *ecm_db_iface_find_and_ref_gre_tun(int if_index, int32_t ae_interface_num)
{
	ecm_db_iface_hash_t hash_index;
	struct ecm_db_iface_instance *ii;

	DEBUG_TRACE("Lookup gre iface with if_index = %d\n", if_index);

	/*
	 * Compute the hash chain index and prepare to walk the chain
	 */
	hash_index = ecm_db_iface_generate_hash_index_gre_tun(if_index);

	/*
	 * Iterate the chain looking for a host with matching details
	 */
	spin_lock_bh(&ecm_db_lock);
	ii = ecm_db_iface_table[hash_index];

	while (ii) {
		if ((ii->type != ECM_DB_IFACE_TYPE_GRE_TUN)
				|| (ii->type_info.gre_tun.if_index != if_index)
				|| (ii->ae_interface_identifier != ae_interface_num)) {
			ii = ii->hash_next;
			continue;
		}

		_ecm_db_iface_ref(ii);
		spin_unlock_bh(&ecm_db_lock);
		DEBUG_TRACE("%px: iface found\n", ii);
		return ii;
	}
	spin_unlock_bh(&ecm_db_lock);

	DEBUG_TRACE("Iface not found\n");
	return NULL;
}
EXPORT_SYMBOL(ecm_db_iface_find_and_ref_gre_tun);

#endif
/*
 * ecm_db_iface_find_and_ref_unknown()
 *	Lookup and return a iface reference if any
 */
struct ecm_db_iface_instance *ecm_db_iface_find_and_ref_unknown(uint32_t os_specific_ident)
{
	ecm_db_iface_hash_t hash_index;
	struct ecm_db_iface_instance *ii;

	DEBUG_TRACE("Lookup unknown iface with addr %x (%u)\n", os_specific_ident, os_specific_ident);

	/*
	 * Compute the hash chain index and prepare to walk the chain
	 */
	hash_index = ecm_db_iface_generate_hash_index_unknown(os_specific_ident);

	/*
	 * Iterate the chain looking for a host with matching details
	 */
	spin_lock_bh(&ecm_db_lock);
	ii = ecm_db_iface_table[hash_index];
	while (ii) {
		if ((ii->type != ECM_DB_IFACE_TYPE_UNKNOWN) || (ii->type_info.unknown.os_specific_ident != os_specific_ident)) {
			ii = ii->hash_next;
			continue;
		}

		_ecm_db_iface_ref(ii);
		spin_unlock_bh(&ecm_db_lock);
		DEBUG_TRACE("iface found %px\n", ii);
		return ii;
	}
	spin_unlock_bh(&ecm_db_lock);
	DEBUG_TRACE("Iface not found\n");
	return NULL;
}
EXPORT_SYMBOL(ecm_db_iface_find_and_ref_unknown);

/*
 * ecm_db_iface_find_and_ref_loopback()
 *	Lookup and return a iface reference if any
 */
struct ecm_db_iface_instance *ecm_db_iface_find_and_ref_loopback(uint32_t os_specific_ident)
{
	ecm_db_iface_hash_t hash_index;
	struct ecm_db_iface_instance *ii;

	DEBUG_TRACE("Lookup loopback iface with addr %x (%u)\n", os_specific_ident, os_specific_ident);

	/*
	 * Compute the hash chain index and prepare to walk the chain
	 */
	hash_index = ecm_db_iface_generate_hash_index_loopback(os_specific_ident);

	/*
	 * Iterate the chain looking for a host with matching details
	 */
	spin_lock_bh(&ecm_db_lock);
	ii = ecm_db_iface_table[hash_index];
	while (ii) {
		if ((ii->type != ECM_DB_IFACE_TYPE_LOOPBACK) || (ii->type_info.loopback.os_specific_ident != os_specific_ident)) {
			ii = ii->hash_next;
			continue;
		}

		_ecm_db_iface_ref(ii);
		spin_unlock_bh(&ecm_db_lock);
		DEBUG_TRACE("iface found %px\n", ii);
		return ii;
	}
	spin_unlock_bh(&ecm_db_lock);
	DEBUG_TRACE("Iface not found\n");
	return NULL;
}
EXPORT_SYMBOL(ecm_db_iface_find_and_ref_loopback);

#ifdef ECM_INTERFACE_IPSEC_ENABLE
/*
 * ecm_db_iface_find_and_ref_ipsec_tunnel()
 *	Lookup and return a iface reference if any.
 * GGG TODO Flesh this out using tunnel endpoint keys
 */
struct ecm_db_iface_instance *ecm_db_iface_find_and_ref_ipsec_tunnel(uint32_t os_specific_ident, int32_t ae_interface_num)
{
	ecm_db_iface_hash_t hash_index;
	struct ecm_db_iface_instance *ii;

	DEBUG_TRACE("Lookup ipsec_tunnel iface with addr %x (%u)\n", os_specific_ident, os_specific_ident);

	/*
	 * Compute the hash chain index and prepare to walk the chain
	 */
	hash_index = ecm_db_iface_generate_hash_index_ipsec_tunnel(os_specific_ident);

	/*
	 * Iterate the chain looking for a host with matching details
	 */
	spin_lock_bh(&ecm_db_lock);
	ii = ecm_db_iface_table[hash_index];
	while (ii) {
		if ((ii->type != ECM_DB_IFACE_TYPE_IPSEC_TUNNEL)
				|| (ii->type_info.ipsec_tunnel.os_specific_ident != os_specific_ident)
				|| (ii->ae_interface_identifier != ae_interface_num)) {
			ii = ii->hash_next;
			continue;
		}

		_ecm_db_iface_ref(ii);
		spin_unlock_bh(&ecm_db_lock);
		DEBUG_TRACE("iface found %px\n", ii);
		return ii;
	}
	spin_unlock_bh(&ecm_db_lock);
	DEBUG_TRACE("Iface not found\n");
	return NULL;
}
EXPORT_SYMBOL(ecm_db_iface_find_and_ref_ipsec_tunnel);
#endif

#ifdef ECM_INTERFACE_SIT_ENABLE
/*
 * ecm_db_iface_find_and_ref_sit()
 *	Lookup and return a iface reference if any
 */
struct ecm_db_iface_instance *ecm_db_iface_find_and_ref_sit(ip_addr_t saddr, ip_addr_t daddr, int32_t ae_interface_num)
{
	ecm_db_iface_hash_t hash_index;
	struct ecm_db_iface_instance *ii;

	DEBUG_TRACE("Lookup sit (6-in-4) iface with saddr: " ECM_IP_ADDR_OCTAL_FMT ", daddr: " ECM_IP_ADDR_OCTAL_FMT "\n",
			ECM_IP_ADDR_TO_OCTAL(saddr), ECM_IP_ADDR_TO_OCTAL(daddr));

	/*
	 * Compute the hash chain index and prepare to walk the chain
	 */
	hash_index = ecm_db_iface_generate_hash_index_sit(saddr, daddr);

	/*
	 * Iterate the chain looking for a host with matching details
	 */
	spin_lock_bh(&ecm_db_lock);
	ii = ecm_db_iface_table[hash_index];
	while (ii) {
		if ((ii->type != ECM_DB_IFACE_TYPE_SIT)
				|| !ECM_IP_ADDR_MATCH(ii->type_info.sit.saddr, saddr)
				|| !ECM_IP_ADDR_MATCH(ii->type_info.sit.daddr, daddr)
				|| (ii->ae_interface_identifier != ae_interface_num)) {
			ii = ii->hash_next;
			continue;
		}

		_ecm_db_iface_ref(ii);
		spin_unlock_bh(&ecm_db_lock);
		DEBUG_TRACE("iface found %px\n", ii);
		return ii;
	}
	spin_unlock_bh(&ecm_db_lock);
	DEBUG_TRACE("Iface not found\n");
	return NULL;
}
EXPORT_SYMBOL(ecm_db_iface_find_and_ref_sit);
#endif

#ifdef ECM_INTERFACE_TUNIPIP6_ENABLE
#ifdef ECM_IPV6_ENABLE
/*
 * ecm_db_iface_find_and_ref_tunipip6()
 *	Lookup and return a iface reference if any
 */
struct ecm_db_iface_instance *ecm_db_iface_find_and_ref_tunipip6(ip_addr_t saddr, ip_addr_t daddr, int32_t ae_interface_num)
{
	ecm_db_iface_hash_t hash_index;
	struct ecm_db_iface_instance *ii;

	DEBUG_TRACE("Lookup TUNIPIP6 iface with saddr: " ECM_IP_ADDR_OCTAL_FMT ", daddr: " ECM_IP_ADDR_OCTAL_FMT "\n",
			ECM_IP_ADDR_TO_OCTAL(saddr), ECM_IP_ADDR_TO_OCTAL(daddr));

	/*
	 * Compute the hash chain index and prepare to walk the chain
	 */
	hash_index = ecm_db_iface_generate_hash_index_tunipip6(saddr, daddr);

	/*
	 * Iterate the chain looking for a host with matching details
	 */
	spin_lock_bh(&ecm_db_lock);
	ii = ecm_db_iface_table[hash_index];
	while (ii) {
		if ((ii->type != ECM_DB_IFACE_TYPE_TUNIPIP6)
			|| !ECM_IP_ADDR_MATCH(ii->type_info.tunipip6.saddr, saddr)
			|| !ECM_IP_ADDR_MATCH(ii->type_info.tunipip6.daddr, daddr)
			|| (ii->ae_interface_identifier != ae_interface_num)) {
			ii = ii->hash_next;
			continue;
		}

		_ecm_db_iface_ref(ii);
		spin_unlock_bh(&ecm_db_lock);
		DEBUG_TRACE("iface found %px\n", ii);
		return ii;
	}
	spin_unlock_bh(&ecm_db_lock);
	DEBUG_TRACE("Iface not found\n");
	return NULL;
}
EXPORT_SYMBOL(ecm_db_iface_find_and_ref_tunipip6);
#endif
#endif

#ifdef ECM_INTERFACE_RAWIP_ENABLE
/*
 * ecm_db_iface_find_and_ref_rawip()
 *	Lookup and return a iface reference if any
 */
struct ecm_db_iface_instance *ecm_db_iface_find_and_ref_rawip(uint8_t *address)
{
	ecm_db_iface_hash_t hash_index;
	struct ecm_db_iface_instance *ii;

	DEBUG_TRACE("Lookup RAWIP iface with addr %pM\n", address);

	/*
	 * Compute the hash chain index and prepare to walk the chain
	 * We can use the same hash function of ethernet interface.
	 */
	hash_index = ecm_db_iface_generate_hash_index_ethernet(address);

	/*
	 * Iterate the chain looking for an iface with matching details
	 */
	spin_lock_bh(&ecm_db_lock);
	ii = ecm_db_iface_table[hash_index];
	while (ii) {
		if ((ii->type != ECM_DB_IFACE_TYPE_RAWIP)
		    || memcmp(ii->type_info.rawip.address, address, ETH_ALEN)) {
			ii = ii->hash_next;
			continue;
		}

		_ecm_db_iface_ref(ii);
		spin_unlock_bh(&ecm_db_lock);
		DEBUG_TRACE("%px: RAWIP iface found\n", ii);
		return ii;
	}
	spin_unlock_bh(&ecm_db_lock);
	DEBUG_TRACE("RAWIP iface not found\n");
	return NULL;
}
EXPORT_SYMBOL(ecm_db_iface_find_and_ref_rawip);
#endif

#ifdef ECM_INTERFACE_OVPN_ENABLE
/*
 * ecm_db_iface_find_and_ref_ovpn()
 *	Lookup and return OVPN iface reference
 */
struct ecm_db_iface_instance *ecm_db_iface_find_and_ref_ovpn(int32_t tun_ifnum)
{
	ecm_db_iface_hash_t hash_index;
	struct ecm_db_iface_instance *ii;

	DEBUG_TRACE("Lookup OVPN iface with ifnum: %d\n", tun_ifnum);

	/*
	 * Compute the hash chain index and prepare to walk the chain
	 */
	hash_index = ecm_db_iface_generate_hash_index_ovpn(tun_ifnum);

	/*
	 * Iterate the chain looking for a host with matching details
	 */
	spin_lock_bh(&ecm_db_lock);
	ii = ecm_db_iface_table[hash_index];
	while (ii) {
		if ((ii->type != ECM_DB_IFACE_TYPE_OVPN)
				|| ii->type_info.ovpn.tun_ifnum != tun_ifnum) {
			ii = ii->hash_next;
			continue;
		}

		_ecm_db_iface_ref(ii);
		spin_unlock_bh(&ecm_db_lock);
		DEBUG_TRACE("iface found %px\n", ii);
		return ii;
	}
	spin_unlock_bh(&ecm_db_lock);
	DEBUG_TRACE("Iface not found\n");
	return NULL;
}
EXPORT_SYMBOL(ecm_db_iface_find_and_ref_ovpn);
#endif

#ifdef ECM_DB_XREF_ENABLE
/*
 * ecm_db_iface_connections_get_and_ref_first()
 *	Return a reference to the first connection made for this iface on the specified direction.
 */
struct ecm_db_connection_instance *
ecm_db_iface_connections_get_and_ref_first(struct ecm_db_iface_instance *ii,
					   ecm_db_obj_dir_t dir)
{
	struct ecm_db_connection_instance *ci;

	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed", ii);

	spin_lock_bh(&ecm_db_lock);
	ci = ii->connections[dir];
	if (ci) {
		_ecm_db_connection_ref(ci);
	}
	spin_unlock_bh(&ecm_db_lock);

	return ci;
}
EXPORT_SYMBOL(ecm_db_iface_connections_get_and_ref_first);

/*
 * ecm_db_iface_nodes_get_and_ref_first()
 *	Return a reference to the first node made from this iface
 */
struct ecm_db_node_instance *ecm_db_iface_nodes_get_and_ref_first(struct ecm_db_iface_instance *ii)
{
	struct ecm_db_node_instance *ni;

	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed", ii);

	spin_lock_bh(&ecm_db_lock);
	ni = ii->nodes;
	if (ni) {
		_ecm_db_node_ref(ni);
	}
	spin_unlock_bh(&ecm_db_lock);

	return ni;
}
EXPORT_SYMBOL(ecm_db_iface_nodes_get_and_ref_first);

/*
 * ecm_db_iface_node_count_get()
 *	Return the number of nodes to this iface
 */
int ecm_db_iface_node_count_get(struct ecm_db_iface_instance *ii)
{
	int count;

	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);

	spin_lock_bh(&ecm_db_lock);
	count = ii->node_count;
	spin_unlock_bh(&ecm_db_lock);
	return count;
}
EXPORT_SYMBOL(ecm_db_iface_node_count_get);
#endif

/*
 * ecm_db_iface_add_to_db()
 *	Adds the created iface to the database lists.
 */
static inline void ecm_db_iface_add_to_db(struct ecm_db_iface_instance *ii,  ecm_db_iface_hash_t hash_index)
{
	ecm_db_iface_id_hash_t iface_id_hash_index;
	struct ecm_db_listener_instance *li;

	ii->hash_index = hash_index;

	iface_id_hash_index = ecm_db_iface_id_generate_hash_index(ii->interface_identifier);
	ii->iface_id_hash_index = iface_id_hash_index;

	/*
	 * Add into the global list
	 */
	spin_lock_bh(&ecm_db_lock);
	ii->flags |= ECM_DB_IFACE_FLAGS_INSERTED;
	ii->prev = NULL;
	ii->next = ecm_db_interfaces;
	if (ecm_db_interfaces) {
		ecm_db_interfaces->prev = ii;
	}
	ecm_db_interfaces = ii;

	/*
	 * Insert into chain
	 */
	ii->hash_prev = NULL;
	ii->hash_next = ecm_db_iface_table[hash_index];
	if (ecm_db_iface_table[hash_index]) {
		ecm_db_iface_table[hash_index]->hash_prev = ii;
	}
	ecm_db_iface_table[hash_index] = ii;
	ecm_db_iface_table_lengths[hash_index]++;
	DEBUG_ASSERT(ecm_db_iface_table_lengths[hash_index] > 0, "%px: invalid table len %d\n", ii, ecm_db_iface_table_lengths[hash_index]);

	DEBUG_INFO("%px: interface inserted at hash index %u, hash prev is %px, type: %d\n", ii, ii->hash_index, ii->hash_prev, ii->type);

	/*
	 * Insert into interface identifier chain
	 */
	ii->iface_id_hash_prev = NULL;
	ii->iface_id_hash_next = ecm_db_iface_id_table[iface_id_hash_index];
	if (ecm_db_iface_id_table[iface_id_hash_index]) {
		ecm_db_iface_id_table[iface_id_hash_index]->iface_id_hash_prev = ii;
	}
	ecm_db_iface_id_table[iface_id_hash_index] = ii;
	ecm_db_iface_id_table_lengths[iface_id_hash_index]++;
	DEBUG_ASSERT(ecm_db_iface_id_table_lengths[iface_id_hash_index] > 0, "%px: invalid iface id table len %d\n", ii, ecm_db_iface_id_table_lengths[iface_id_hash_index]);

	/*
	 * Set time of addition
	 */
	ii->time_added = ecm_db_time;
	spin_unlock_bh(&ecm_db_lock);

	/*
	 * Throw add event to the listeners
	 */
	DEBUG_TRACE("%px: Throw iface added event\n", ii);
	li = ecm_db_listeners_get_and_ref_first();
	while (li) {
		struct ecm_db_listener_instance *lin;
		if (li->iface_added) {
			li->iface_added(li->arg, ii);
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
 * ecm_db_iface_add_ethernet()
 *	Add a iface instance into the database
 */
void ecm_db_iface_add_ethernet(struct ecm_db_iface_instance *ii, uint8_t *address, char *name, int32_t mtu,
					int32_t interface_identifier, int32_t ae_interface_identifier,
					ecm_db_iface_final_callback_t final, void *arg)
{
	ecm_db_iface_hash_t hash_index;
	struct ecm_db_interface_info_ethernet *type_info;

	spin_lock_bh(&ecm_db_lock);
	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);
	DEBUG_ASSERT(address, "%px: address null\n", ii);
#ifdef ECM_DB_XREF_ENABLE
	DEBUG_ASSERT((ii->nodes == NULL) && (ii->node_count == 0), "%px: nodes not null\n", ii);
#endif
	DEBUG_ASSERT(!(ii->flags & ECM_DB_IFACE_FLAGS_INSERTED), "%px: inserted\n", ii);
	DEBUG_ASSERT(name, "%px: no name given\n", ii);
	spin_unlock_bh(&ecm_db_lock);

	/*
	 * Record general info
	 */
	ii->type = ECM_DB_IFACE_TYPE_ETHERNET;
#ifdef ECM_STATE_OUTPUT_ENABLE
	ii->state_get = ecm_db_iface_ethernet_state_get;
#endif
	ii->arg = arg;
	ii->final = final;
	strlcpy(ii->name, name, IFNAMSIZ);
	ii->mtu = mtu;
	ii->interface_identifier = interface_identifier;
	ii->ae_interface_identifier = ae_interface_identifier;

	/*
	 * Type specific info
	 */
	type_info = &ii->type_info.ethernet;
	memcpy(type_info->address, address, ETH_ALEN);

	/*
	 * Compute hash chain for insertion
	 */
	hash_index = ecm_db_iface_generate_hash_index_ethernet(address);

	ecm_db_iface_add_to_db(ii, hash_index);

}
EXPORT_SYMBOL(ecm_db_iface_add_ethernet);

#ifdef ECM_INTERFACE_BOND_ENABLE
/*
 * ecm_db_iface_add_lag()
 *	Add a iface instance into the database
 */
void ecm_db_iface_add_lag(struct ecm_db_iface_instance *ii, uint8_t *address, char *name, int32_t mtu,
					int32_t interface_identifier, int32_t ae_interface_identifier,
					ecm_db_iface_final_callback_t final, void *arg)
{
	ecm_db_iface_hash_t hash_index;
	struct ecm_db_interface_info_lag *type_info;

	spin_lock_bh(&ecm_db_lock);
	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);
	DEBUG_ASSERT(address, "%px: address null\n", ii);
#ifdef ECM_DB_XREF_ENABLE
	DEBUG_ASSERT((ii->nodes == NULL) && (ii->node_count == 0), "%px: nodes not null\n", ii);
#endif
	DEBUG_ASSERT(!(ii->flags & ECM_DB_IFACE_FLAGS_INSERTED), "%px: inserted\n", ii);
	DEBUG_ASSERT(name, "%px: no name given\n", ii);
	spin_unlock_bh(&ecm_db_lock);

	/*
	 * Record general info
	 */
	ii->type = ECM_DB_IFACE_TYPE_LAG;
#ifdef ECM_STATE_OUTPUT_ENABLE
	ii->state_get = ecm_db_iface_lag_state_get;
#endif
	ii->arg = arg;
	ii->final = final;
	strlcpy(ii->name, name, IFNAMSIZ);
	ii->mtu = mtu;
	ii->interface_identifier = interface_identifier;
	ii->ae_interface_identifier = ae_interface_identifier;

	/*
	 * Type specific info
	 */
	type_info = &ii->type_info.lag;
	memcpy(type_info->address, address, ETH_ALEN);

	/*
	 * Compute hash chain for insertion
	 */
	hash_index = ecm_db_iface_generate_hash_index_ethernet(address);

	ecm_db_iface_add_to_db(ii, hash_index);
}
EXPORT_SYMBOL(ecm_db_iface_add_lag);
#endif

/*
 * ecm_db_iface_add_bridge()
 *	Add a iface instance into the database
 */
void ecm_db_iface_add_bridge(struct ecm_db_iface_instance *ii, uint8_t *address, char *name, int32_t mtu,
					int32_t interface_identifier, int32_t ae_interface_identifier,
					ecm_db_iface_final_callback_t final, void *arg)
{
	ecm_db_iface_hash_t hash_index;
	struct ecm_db_interface_info_bridge *type_info;

	spin_lock_bh(&ecm_db_lock);
	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);
	DEBUG_ASSERT(address, "%px: address null\n", ii);
#ifdef ECM_DB_XREF_ENABLE
	DEBUG_ASSERT((ii->nodes == NULL) && (ii->node_count == 0), "%px: nodes not null\n", ii);
#endif
	DEBUG_ASSERT(!(ii->flags & ECM_DB_IFACE_FLAGS_INSERTED), "%px: inserted\n", ii);
	DEBUG_ASSERT(name, "%px: no name given\n", ii);
	spin_unlock_bh(&ecm_db_lock);

	/*
	 * Record general info
	 */
	ii->type = ECM_DB_IFACE_TYPE_BRIDGE;
#ifdef ECM_STATE_OUTPUT_ENABLE
	ii->state_get = ecm_db_iface_bridge_state_get;
#endif
	ii->arg = arg;
	ii->final = final;
	strlcpy(ii->name, name, IFNAMSIZ);
	ii->mtu = mtu;
	ii->interface_identifier = interface_identifier;
	ii->ae_interface_identifier = ae_interface_identifier;

	/*
	 * Type specific info
	 */
	type_info = &ii->type_info.bridge;
	memcpy(type_info->address, address, ETH_ALEN);

	/*
	 * Compute hash chain for insertion
	 */
	hash_index = ecm_db_iface_generate_hash_index_ethernet(address);

	ecm_db_iface_add_to_db(ii, hash_index);
}
EXPORT_SYMBOL(ecm_db_iface_add_bridge);

#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
/*
 * ecm_db_iface_add_ovs_bridge()
 *	Add a iface instance into the database
 */
void ecm_db_iface_add_ovs_bridge(struct ecm_db_iface_instance *ii, uint8_t *address, char *name, int32_t mtu,
					int32_t interface_identifier, int32_t ae_interface_identifier,
					ecm_db_iface_final_callback_t final, void *arg)
{
	ecm_db_iface_hash_t hash_index;
	struct ecm_db_interface_info_ovs_bridge *type_info;

	spin_lock_bh(&ecm_db_lock);
	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);
	DEBUG_ASSERT(address, "%px: address null\n", ii);
#ifdef ECM_DB_XREF_ENABLE
	DEBUG_ASSERT((ii->nodes == NULL) && (ii->node_count == 0), "%px: nodes not null\n", ii);
#endif
	DEBUG_ASSERT(!(ii->flags & ECM_DB_IFACE_FLAGS_INSERTED), "%px: inserted\n", ii);
	DEBUG_ASSERT(name, "%px: no name given\n", ii);
	spin_unlock_bh(&ecm_db_lock);

	/*
	 * Record general info
	 */
	ii->type = ECM_DB_IFACE_TYPE_OVS_BRIDGE;
#ifdef ECM_STATE_OUTPUT_ENABLE
	ii->state_get = ecm_db_iface_ovs_bridge_state_get;
#endif
	ii->arg = arg;
	ii->final = final;
	strlcpy(ii->name, name, IFNAMSIZ);
	ii->mtu = mtu;
	ii->interface_identifier = interface_identifier;
	ii->ae_interface_identifier = ae_interface_identifier;

	/*
	 * Type specific info
	 */
	type_info = &ii->type_info.ovsb;
	memcpy(type_info->address, address, ETH_ALEN);

	/*
	 * Compute hash chain for insertion
	 */
	hash_index = ecm_db_iface_generate_hash_index_ethernet(address);

	ecm_db_iface_add_to_db(ii, hash_index);
}
#endif

#ifdef ECM_INTERFACE_MACVLAN_ENABLE
/*
 * ecm_db_iface_add_macvlan()
 *	Add a iface instance into the database
 */
void ecm_db_iface_add_macvlan(struct ecm_db_iface_instance *ii, uint8_t *address, char *name, int32_t mtu,
					int32_t interface_identifier, int32_t ae_interface_identifier,
					ecm_db_iface_final_callback_t final, void *arg)
{
	ecm_db_iface_hash_t hash_index;
	struct ecm_db_interface_info_macvlan *type_info;

	spin_lock_bh(&ecm_db_lock);
	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);
	DEBUG_ASSERT(address, "%px: address null\n", ii);
#ifdef ECM_DB_XREF_ENABLE
	DEBUG_ASSERT((ii->nodes == NULL) && (ii->node_count == 0), "%px: nodes not null\n", ii);
#endif
	DEBUG_ASSERT(!(ii->flags & ECM_DB_IFACE_FLAGS_INSERTED), "%px: inserted\n", ii);
	DEBUG_ASSERT(name, "%px: no name given\n", ii);
	spin_unlock_bh(&ecm_db_lock);

	/*
	 * Record general info
	 */
	ii->type = ECM_DB_IFACE_TYPE_MACVLAN;
#ifdef ECM_STATE_OUTPUT_ENABLE
	ii->state_get = ecm_db_iface_macvlan_state_get;
#endif
	ii->arg = arg;
	ii->final = final;
	strlcpy(ii->name, name, IFNAMSIZ);
	ii->mtu = mtu;
	ii->interface_identifier = interface_identifier;
	ii->ae_interface_identifier = ae_interface_identifier;

	/*
	 * Type specific info
	 */
	type_info = &ii->type_info.macvlan;
	ether_addr_copy(type_info->address, address);

	/*
	 * Compute hash chain for insertion
	 */
	hash_index = ecm_db_iface_generate_hash_index_ethernet(address);

	ecm_db_iface_add_to_db(ii, hash_index);
}
#endif

#ifdef ECM_INTERFACE_VLAN_ENABLE
/*
 * ecm_db_iface_add_vlan()
 *	Add a iface instance into the database
 */
void ecm_db_iface_add_vlan(struct ecm_db_iface_instance *ii, uint8_t *address, uint16_t vlan_tag, uint16_t vlan_tpid, char *name, int32_t mtu,
					int32_t interface_identifier, int32_t ae_interface_identifier,
					ecm_db_iface_final_callback_t final, void *arg)
{
	ecm_db_iface_hash_t hash_index;
	struct ecm_db_interface_info_vlan *type_info;

	spin_lock_bh(&ecm_db_lock);
	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);
	DEBUG_ASSERT(address, "%px: address null\n", ii);
#ifdef ECM_DB_XREF_ENABLE
	DEBUG_ASSERT((ii->nodes == NULL) && (ii->node_count == 0), "%px: nodes not null\n", ii);
#endif
	DEBUG_ASSERT(!(ii->flags & ECM_DB_IFACE_FLAGS_INSERTED), "%px: inserted\n", ii);
	DEBUG_ASSERT(name, "%px: no name given\n", ii);
	spin_unlock_bh(&ecm_db_lock);

	/*
	 * Record general info
	 */
	ii->type = ECM_DB_IFACE_TYPE_VLAN;
#ifdef ECM_STATE_OUTPUT_ENABLE
	ii->state_get = ecm_db_iface_vlan_state_get;
#endif
	ii->arg = arg;
	ii->final = final;
	strlcpy(ii->name, name, IFNAMSIZ);
	ii->mtu = mtu;
	ii->interface_identifier = interface_identifier;
	ii->ae_interface_identifier = ae_interface_identifier;

	/*
	 * Type specific info
	 */
	type_info = &ii->type_info.vlan;
	type_info->vlan_tag = vlan_tag;
	type_info->vlan_tpid = vlan_tpid;
	memcpy(type_info->address, address, ETH_ALEN);

	/*
	 * Compute hash chain for insertion
	 */
	hash_index = ecm_db_iface_generate_hash_index_ethernet(address);

	ecm_db_iface_add_to_db(ii, hash_index);
}
EXPORT_SYMBOL(ecm_db_iface_add_vlan);
#endif

#ifdef ECM_INTERFACE_MAP_T_ENABLE
/*
 * ecm_db_iface_add_map_t()
 *	Add a iface instance into the database
 */
void ecm_db_iface_add_map_t(struct ecm_db_iface_instance *ii, struct ecm_db_interface_info_map_t *map_t_info,
					char *name, int32_t mtu, int32_t interface_identifier,
					int32_t ae_interface_identifier, ecm_db_iface_final_callback_t final,
					void *arg)
{
	ecm_db_iface_hash_t hash_index;
	struct ecm_db_interface_info_map_t *type_info;

	spin_lock_bh(&ecm_db_lock);
	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);
#ifdef ECM_DB_XREF_ENABLE
	DEBUG_ASSERT((ii->nodes == NULL) && (ii->node_count == 0), "%px: nodes not null\n", ii);
#endif
	DEBUG_ASSERT(!(ii->flags & ECM_DB_IFACE_FLAGS_INSERTED), "%px: inserted\n", ii);
	DEBUG_ASSERT(name, "%px: no name given\n", ii);
	spin_unlock_bh(&ecm_db_lock);

	/*
	 * Record general info
	 */
	ii->type = ECM_DB_IFACE_TYPE_MAP_T;
#ifdef ECM_STATE_OUTPUT_ENABLE
	ii->state_get = ecm_db_iface_map_t_state_get;
#endif
	ii->arg = arg;
	ii->final = final;
	strlcpy(ii->name, name, IFNAMSIZ);
	ii->mtu = mtu;
	ii->interface_identifier = interface_identifier;
	ii->ae_interface_identifier = ae_interface_identifier;

	/*
	 * Type specific info
	 */
	type_info = &ii->type_info.map_t;
	memcpy(type_info, map_t_info, sizeof(struct ecm_db_interface_info_map_t));

	/*
	 * Compute hash chain for insertion
	 */
	hash_index = ecm_db_iface_generate_hash_index_map_t(type_info->if_index);

	ecm_db_iface_add_to_db(ii, hash_index);
}
EXPORT_SYMBOL(ecm_db_iface_add_map_t);
#endif

#ifdef ECM_INTERFACE_GRE_TUN_ENABLE
/*
 * ecm_db_iface_add_gre_tun()
 * 	Add a iface instance into the database
 */
void ecm_db_iface_add_gre_tun(struct ecm_db_iface_instance *ii, struct ecm_db_interface_info_gre_tun *gre_tun_info,
				char *name, int32_t mtu, int32_t interface_identifier,
				int32_t ae_interface_identifier, ecm_db_iface_final_callback_t final,
				void *arg)
{
	ecm_db_iface_hash_t hash_index;
	struct ecm_db_interface_info_gre_tun *type_info;

	spin_lock_bh(&ecm_db_lock);
	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);
#ifdef ECM_DB_XREF_ENABLE
	DEBUG_ASSERT((ii->nodes == NULL) && (ii->node_count == 0), "%px: nodes not null\n", ii);
#endif
	DEBUG_ASSERT(!(ii->flags & ECM_DB_IFACE_FLAGS_INSERTED), "%px: inserted\n", ii);
	DEBUG_ASSERT(name, "%px: no name given\n", ii);
	spin_unlock_bh(&ecm_db_lock);

	/*
	 * Record general info
	 */
	ii->type = ECM_DB_IFACE_TYPE_GRE_TUN;
#ifdef ECM_STATE_OUTPUT_ENABLE
	ii->state_get = ecm_db_iface_gre_tun_state_get;
#endif
	ii->arg = arg;
	ii->final = final;
	strlcpy(ii->name, name, IFNAMSIZ);
	ii->mtu = mtu;
	ii->interface_identifier = interface_identifier;
	ii->ae_interface_identifier = ae_interface_identifier;

	/*
	 * Type specific info
	 */
	type_info = &ii->type_info.gre_tun;
	memcpy(type_info, gre_tun_info, sizeof(struct ecm_db_interface_info_gre_tun));

	/*
	 * Compute hash chain for insertion
	 */
	hash_index = ecm_db_iface_generate_hash_index_gre_tun(type_info->if_index);

	ecm_db_iface_add_to_db(ii, hash_index);
}
EXPORT_SYMBOL(ecm_db_iface_add_gre_tun);
#endif

#ifdef ECM_INTERFACE_PPPOE_ENABLE
/*
 * ecm_db_iface_add_pppoe()
 *	Add a iface instance into the database
 */
void ecm_db_iface_add_pppoe(struct ecm_db_iface_instance *ii, uint16_t pppoe_session_id, uint8_t *remote_mac,
					char *name, int32_t mtu, int32_t interface_identifier,
					int32_t ae_interface_identifier, ecm_db_iface_final_callback_t final,
					void *arg)
{
	ecm_db_iface_hash_t hash_index;
	struct ecm_db_interface_info_pppoe *type_info;

	spin_lock_bh(&ecm_db_lock);
	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);
#ifdef ECM_DB_XREF_ENABLE
	DEBUG_ASSERT((ii->nodes == NULL) && (ii->node_count == 0), "%px: nodes not null\n", ii);
#endif
	DEBUG_ASSERT(!(ii->flags & ECM_DB_IFACE_FLAGS_INSERTED), "%px: inserted\n", ii);
	DEBUG_ASSERT(name, "%px: no name given\n", ii);
	spin_unlock_bh(&ecm_db_lock);

	/*
	 * Record general info
	 */
	ii->type = ECM_DB_IFACE_TYPE_PPPOE;
#ifdef ECM_STATE_OUTPUT_ENABLE
	ii->state_get = ecm_db_iface_pppoe_state_get;
#endif
	ii->arg = arg;
	ii->final = final;
	strlcpy(ii->name, name, IFNAMSIZ);
	ii->mtu = mtu;
	ii->interface_identifier = interface_identifier;
	ii->ae_interface_identifier = ae_interface_identifier;

	/*
	 * Type specific info
	 */
	type_info = &ii->type_info.pppoe;
	type_info->pppoe_session_id = pppoe_session_id;
	memcpy(type_info->remote_mac, remote_mac, ETH_ALEN);

	/*
	 * Compute hash chain for insertion
	 */
	hash_index = ecm_db_iface_generate_hash_index_pppoe(pppoe_session_id);

	ecm_db_iface_add_to_db(ii, hash_index);
}
EXPORT_SYMBOL(ecm_db_iface_add_pppoe);
#endif

#ifdef ECM_INTERFACE_L2TPV2_ENABLE
/*
 * ecm_db_iface_add_pppol2tpv2()
 *	Add a iface instance into the database
 */
void ecm_db_iface_add_pppol2tpv2(struct ecm_db_iface_instance *ii, struct ecm_db_interface_info_pppol2tpv2 *pppol2tpv2_info,
					char *name, int32_t mtu, int32_t interface_identifier,
					int32_t ae_interface_identifier, ecm_db_iface_final_callback_t final,
					void *arg)
{
	ecm_db_iface_hash_t hash_index;
	struct ecm_db_interface_info_pppol2tpv2 *type_info;

	spin_lock_bh(&ecm_db_lock);
	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);
#ifdef ECM_DB_XREF_ENABLE
	DEBUG_ASSERT((ii->nodes == NULL) && (ii->node_count == 0), "%px: nodes not null\n", ii);
#endif
	DEBUG_ASSERT(!(ii->flags & ECM_DB_IFACE_FLAGS_INSERTED), "%px: inserted\n", ii);
	DEBUG_ASSERT(name, "%px: no name given\n", ii);
	spin_unlock_bh(&ecm_db_lock);

	/*
	 * Record general info
	 */
	ii->type = ECM_DB_IFACE_TYPE_PPPOL2TPV2;
#ifdef ECM_STATE_OUTPUT_ENABLE
	ii->state_get = ecm_db_iface_pppol2tpv2_state_get;
#endif
	ii->arg = arg;
	ii->final = final;
	strlcpy(ii->name, name, IFNAMSIZ);
	ii->mtu = mtu;
	ii->interface_identifier = interface_identifier;
	ii->ae_interface_identifier = ae_interface_identifier;

	/*
	 * Type specific info
	 */
	type_info = &ii->type_info.pppol2tpv2;
	memcpy(type_info, pppol2tpv2_info, sizeof(struct ecm_db_interface_info_pppol2tpv2));

	/*
	 * Compute hash chain for insertion
	 */
	hash_index = ecm_db_iface_generate_hash_index_pppol2tpv2(type_info->l2tp.tunnel.tunnel_id,
							  type_info->l2tp.session.session_id);
	ecm_db_iface_add_to_db(ii, hash_index);
}
EXPORT_SYMBOL(ecm_db_iface_add_pppol2tpv2);

#endif

#ifdef ECM_INTERFACE_PPTP_ENABLE
/*
 * ecm_db_iface_add_pptp()
 *	Add a iface instance into the database
 */
void ecm_db_iface_add_pptp(struct ecm_db_iface_instance *ii, struct ecm_db_interface_info_pptp *pptp_info,
					char *name, int32_t mtu, int32_t interface_identifier,
					int32_t ae_interface_identifier, ecm_db_iface_final_callback_t final,
					void *arg)
{
	ecm_db_iface_hash_t hash_index;
	struct ecm_db_interface_info_pptp *type_info;

	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);
	spin_lock_bh(&ecm_db_lock);
#ifdef ECM_DB_XREF_ENABLE
	DEBUG_ASSERT((ii->nodes == NULL) && (ii->node_count == 0), "%px: nodes not null\n", ii);
#endif
	DEBUG_ASSERT(!(ii->flags & ECM_DB_IFACE_FLAGS_INSERTED), "%px: inserted\n", ii);
	DEBUG_ASSERT(name, "%px: no name given\n", ii);
	spin_unlock_bh(&ecm_db_lock);

	/*
	 * Record general info
	 */
	ii->type = ECM_DB_IFACE_TYPE_PPTP;
#ifdef ECM_STATE_OUTPUT_ENABLE
	ii->state_get = ecm_db_iface_pptp_state_get;
#endif
	ii->arg = arg;
	ii->final = final;
	strlcpy(ii->name, name, IFNAMSIZ);
	ii->mtu = mtu;
	ii->interface_identifier = interface_identifier;
	ii->ae_interface_identifier = ae_interface_identifier;

	/*
	 * Type specific info
	 */
	type_info = &ii->type_info.pptp;
	memcpy(type_info, pptp_info, sizeof(struct ecm_db_interface_info_pptp));

	/*
	 * Compute hash chain for insertion
	 */
	hash_index = ecm_db_iface_generate_hash_index_pptp(type_info->src_call_id,
							  type_info->dst_call_id);
	ecm_db_iface_add_to_db(ii, hash_index);
}
EXPORT_SYMBOL(ecm_db_iface_add_pptp);
#endif

/*
 * ecm_db_iface_add_unknown()
 *	Add a iface instance into the database
 */
void ecm_db_iface_add_unknown(struct ecm_db_iface_instance *ii, uint32_t os_specific_ident, char *name, int32_t mtu,
					int32_t interface_identifier, int32_t ae_interface_identifier,
					ecm_db_iface_final_callback_t final, void *arg)
{
	ecm_db_iface_hash_t hash_index;
	struct ecm_db_interface_info_unknown *type_info;

	spin_lock_bh(&ecm_db_lock);
	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);
#ifdef ECM_DB_XREF_ENABLE
	DEBUG_ASSERT((ii->nodes == NULL) && (ii->node_count == 0), "%px: nodes not null\n", ii);
#endif
	DEBUG_ASSERT(!(ii->flags & ECM_DB_IFACE_FLAGS_INSERTED), "%px: inserted\n", ii);
	DEBUG_ASSERT(name, "%px: no name given\n", ii);
	spin_unlock_bh(&ecm_db_lock);

	/*
	 * Record general info
	 */
	ii->type = ECM_DB_IFACE_TYPE_UNKNOWN;
#ifdef ECM_STATE_OUTPUT_ENABLE
	ii->state_get = ecm_db_iface_unknown_state_get;
#endif
	ii->arg = arg;
	ii->final = final;
	strlcpy(ii->name, name, IFNAMSIZ);
	ii->mtu = mtu;
	ii->interface_identifier = interface_identifier;
	ii->ae_interface_identifier = ae_interface_identifier;

	/*
	 * Type specific info
	 */
	type_info = &ii->type_info.unknown;
	type_info->os_specific_ident = os_specific_ident;

	/*
	 * Compute hash chain for insertion
	 */
	hash_index = ecm_db_iface_generate_hash_index_unknown(os_specific_ident);

	ecm_db_iface_add_to_db(ii, hash_index);
}
EXPORT_SYMBOL(ecm_db_iface_add_unknown);

/*
 * ecm_db_iface_add_loopback()
 *	Add a iface instance into the database
 */
void ecm_db_iface_add_loopback(struct ecm_db_iface_instance *ii, uint32_t os_specific_ident, char *name, int32_t mtu,
					int32_t interface_identifier, int32_t ae_interface_identifier,
					ecm_db_iface_final_callback_t final, void *arg)
{
	ecm_db_iface_hash_t hash_index;
	struct ecm_db_interface_info_loopback *type_info;

	spin_lock_bh(&ecm_db_lock);
	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);
#ifdef ECM_DB_XREF_ENABLE
	DEBUG_ASSERT((ii->nodes == NULL) && (ii->node_count == 0), "%px: nodes not null\n", ii);
#endif
	DEBUG_ASSERT(!(ii->flags & ECM_DB_IFACE_FLAGS_INSERTED), "%px: inserted\n", ii);
	DEBUG_ASSERT(name, "%px: no name given\n", ii);
	spin_unlock_bh(&ecm_db_lock);

	/*
	 * Record general info
	 */
	ii->type = ECM_DB_IFACE_TYPE_LOOPBACK;
#ifdef ECM_STATE_OUTPUT_ENABLE
	ii->state_get = ecm_db_iface_loopback_state_get;
#endif
	ii->arg = arg;
	ii->final = final;
	strlcpy(ii->name, name, IFNAMSIZ);
	ii->mtu = mtu;
	ii->interface_identifier = interface_identifier;
	ii->ae_interface_identifier = ae_interface_identifier;

	/*
	 * Type specific info
	 */
	type_info = &ii->type_info.loopback;
	type_info->os_specific_ident = os_specific_ident;

	/*
	 * Compute hash chain for insertion
	 */
	hash_index = ecm_db_iface_generate_hash_index_loopback(os_specific_ident);

	ecm_db_iface_add_to_db(ii, hash_index);
}
EXPORT_SYMBOL(ecm_db_iface_add_loopback);

#ifdef ECM_INTERFACE_SIT_ENABLE
/*
 * ecm_db_iface_sit_daddr_is_null()
 *	The sit addr is null or not
 */
bool ecm_db_iface_sit_daddr_is_null(struct ecm_db_iface_instance *ii)
{
	return ii->type_info.sit.daddr[0] == 0;
}
EXPORT_SYMBOL(ecm_db_iface_sit_daddr_is_null);

/*
 * ecm_db_iface_add_sit()
 *	Add a iface instance into the database
 */
void ecm_db_iface_add_sit(struct ecm_db_iface_instance *ii, struct ecm_db_interface_info_sit *type_info, char *name, int32_t mtu,
					int32_t interface_identifier, int32_t ae_interface_identifier,
					ecm_db_iface_final_callback_t final, void *arg)
{
	ecm_db_iface_hash_t hash_index;

	spin_lock_bh(&ecm_db_lock);
	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);
#ifdef ECM_DB_XREF_ENABLE
	DEBUG_ASSERT((ii->nodes == NULL) && (ii->node_count == 0), "%px: nodes not null\n", ii);
#endif
	DEBUG_ASSERT(!(ii->flags & ECM_DB_IFACE_FLAGS_INSERTED), "%px: inserted\n", ii);
	DEBUG_ASSERT(name, "%px: no name given\n", ii);
	spin_unlock_bh(&ecm_db_lock);

	/*
	 * Record general info
	 */
	ii->type = ECM_DB_IFACE_TYPE_SIT;
#ifdef ECM_STATE_OUTPUT_ENABLE
	ii->state_get = ecm_db_iface_sit_state_get;
#endif
	ii->arg = arg;
	ii->final = final;
	strlcpy(ii->name, name, IFNAMSIZ);
	ii->mtu = mtu;
	ii->interface_identifier = interface_identifier;
	ii->ae_interface_identifier = ae_interface_identifier;

	/*
	 * Type specific info to be copied
	 */
	ii->type_info.sit = *type_info;

	/*
	 * Compute hash chain for insertion
	 */
	hash_index = ecm_db_iface_generate_hash_index_sit(type_info->saddr, type_info->daddr);

	ecm_db_iface_add_to_db(ii, hash_index);
}
EXPORT_SYMBOL(ecm_db_iface_add_sit);
#endif

#ifdef ECM_INTERFACE_TUNIPIP6_ENABLE
#ifdef ECM_IPV6_ENABLE
/*
 * ecm_db_iface_add_tunipip6()
 *	Add a iface instance into the database
 */
void ecm_db_iface_add_tunipip6(struct ecm_db_iface_instance *ii, struct ecm_db_interface_info_tunipip6 *type_info, char *name, int32_t mtu,
					int32_t interface_identifier, int32_t ae_interface_identifier,
					ecm_db_iface_final_callback_t final, void *arg)
{
	ecm_db_iface_hash_t hash_index;

	spin_lock_bh(&ecm_db_lock);
	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);
#ifdef ECM_DB_XREF_ENABLE
	DEBUG_ASSERT((ii->nodes == NULL) && (ii->node_count == 0), "%px: nodes not null\n", ii);
#endif
	DEBUG_ASSERT(!(ii->flags & ECM_DB_IFACE_FLAGS_INSERTED), "%px: inserted\n", ii);
	DEBUG_ASSERT(name, "%px: no name given\n", ii);
	spin_unlock_bh(&ecm_db_lock);

	/*
	 * Record general info
	 */
	ii->type = ECM_DB_IFACE_TYPE_TUNIPIP6;
#ifdef ECM_STATE_OUTPUT_ENABLE
	ii->state_get = ecm_db_iface_tunipip6_state_get;
#endif
	ii->arg = arg;
	ii->final = final;
	strlcpy(ii->name, name, IFNAMSIZ);
	ii->mtu = mtu;
	ii->interface_identifier = interface_identifier;
	ii->ae_interface_identifier = ae_interface_identifier;

	/*
	 * Type specific info to be copied
	 */
	ii->type_info.tunipip6 = *type_info;

	/*
	 * Compute hash chain for insertion
	 */
	hash_index = ecm_db_iface_generate_hash_index_tunipip6(type_info->saddr, type_info->daddr);

	ecm_db_iface_add_to_db(ii, hash_index);
}
EXPORT_SYMBOL(ecm_db_iface_add_tunipip6);
#endif
#endif

#ifdef ECM_INTERFACE_IPSEC_ENABLE
/*
 * ecm_db_iface_add_ipsec_tunnel()
 *	Add a iface instance into the database
 *
 * GGG TODO This needs to take ipsec tunnel endpoint information etc. something very appropriate for ipsec tunnels, anyhow.
 */
void ecm_db_iface_add_ipsec_tunnel(struct ecm_db_iface_instance *ii, uint32_t os_specific_ident, char *name, int32_t mtu,
					int32_t interface_identifier, int32_t ae_interface_identifier,
					ecm_db_iface_final_callback_t final, void *arg)
{
	ecm_db_iface_hash_t hash_index;
	struct ecm_db_interface_info_ipsec_tunnel *type_info;

	spin_lock_bh(&ecm_db_lock);
	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);
#ifdef ECM_DB_XREF_ENABLE
	DEBUG_ASSERT((ii->nodes == NULL) && (ii->node_count == 0), "%px: nodes not null\n", ii);
#endif
	DEBUG_ASSERT(!(ii->flags & ECM_DB_IFACE_FLAGS_INSERTED), "%px: inserted\n", ii);
	DEBUG_ASSERT(name, "%px: no name given\n", ii);
	spin_unlock_bh(&ecm_db_lock);

	/*
	 * Record general info
	 */
	ii->type = ECM_DB_IFACE_TYPE_IPSEC_TUNNEL;
#ifdef ECM_STATE_OUTPUT_ENABLE
	ii->state_get = ecm_db_iface_ipsec_tunnel_state_get;
#endif
	ii->arg = arg;
	ii->final = final;
	strlcpy(ii->name, name, IFNAMSIZ);
	ii->mtu = mtu;
	ii->interface_identifier = interface_identifier;
	ii->ae_interface_identifier = ae_interface_identifier;

	/*
	 * Type specific info
	 */
	type_info = &ii->type_info.ipsec_tunnel;
	type_info->os_specific_ident = os_specific_ident;

	/*
	 * Compute hash chain for insertion
	 */
	hash_index = ecm_db_iface_generate_hash_index_ipsec_tunnel(os_specific_ident);

	ecm_db_iface_add_to_db(ii, hash_index);
}
EXPORT_SYMBOL(ecm_db_iface_add_ipsec_tunnel);
#endif

#ifdef ECM_INTERFACE_RAWIP_ENABLE
/*
 * ecm_db_iface_add_rawip()
 *	Add a iface instance into the database
 */
void ecm_db_iface_add_rawip(struct ecm_db_iface_instance *ii, uint8_t *address, char *name, int32_t mtu,
					int32_t interface_identifier, int32_t ae_interface_identifier,
					ecm_db_iface_final_callback_t final, void *arg)
{
	ecm_db_iface_hash_t hash_index;
	struct ecm_db_interface_info_rawip *type_info;

	spin_lock_bh(&ecm_db_lock);
	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);
	DEBUG_ASSERT(address, "%px: address null\n", ii);
#ifdef ECM_DB_XREF_ENABLE
	DEBUG_ASSERT((ii->nodes == NULL) && (ii->node_count == 0), "%px: nodes not null\n", ii);
#endif
	DEBUG_ASSERT(!(ii->flags & ECM_DB_IFACE_FLAGS_INSERTED), "%px: inserted\n", ii);
	DEBUG_ASSERT(name, "%px: no name given\n", ii);
	spin_unlock_bh(&ecm_db_lock);

	/*
	 * Record general info
	 */
	ii->type = ECM_DB_IFACE_TYPE_RAWIP;
#ifdef ECM_STATE_OUTPUT_ENABLE
	ii->state_get = ecm_db_iface_rawip_state_get;
#endif
	ii->arg = arg;
	ii->final = final;
	strlcpy(ii->name, name, IFNAMSIZ);
	ii->mtu = mtu;
	ii->interface_identifier = interface_identifier;
	ii->ae_interface_identifier = ae_interface_identifier;

	/*
	 * Type specific info
	 */
	type_info = &ii->type_info.rawip;
	memcpy(type_info->address, address, ETH_ALEN);

	/*
	 * Compute hash chain for insertion
	 */
	hash_index = ecm_db_iface_generate_hash_index_ethernet(address);

	ecm_db_iface_add_to_db(ii, hash_index);
}
EXPORT_SYMBOL(ecm_db_iface_add_rawip);
#endif

#ifdef ECM_INTERFACE_OVPN_ENABLE
/*
 * ecm_db_iface_add_ovpn()
 *	Add OVPN interface instance into the database
 */
void ecm_db_iface_add_ovpn(struct ecm_db_iface_instance *ii,
				struct ecm_db_interface_info_ovpn *type_info, char *name,
				int32_t mtu, int32_t interface_identifier,
				ecm_db_iface_final_callback_t final, void *arg)
{
	ecm_db_iface_hash_t hash_index;

	spin_lock_bh(&ecm_db_lock);
	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);
#ifdef ECM_DB_XREF_ENABLE
	DEBUG_ASSERT((ii->nodes == NULL) && (ii->node_count == 0), "%px: nodes not null\n", ii);
#endif
	DEBUG_ASSERT(!(ii->flags & ECM_DB_IFACE_FLAGS_INSERTED), "%px: inserted\n", ii);
	DEBUG_ASSERT(name, "%px: no name given\n", ii);
	spin_unlock_bh(&ecm_db_lock);

	/*
	 * Record general info
	 */
	ii->type = ECM_DB_IFACE_TYPE_OVPN;
#ifdef ECM_STATE_OUTPUT_ENABLE
	ii->state_get = ecm_db_iface_ovpn_state_get;
#endif
	ii->arg = arg;
	ii->final = final;
	strlcpy(ii->name, name, IFNAMSIZ);
	ii->mtu = mtu;
	ii->interface_identifier = interface_identifier;
	ii->ae_interface_identifier = type_info->tun_ifnum;

	/*
	 * Type specific info to be copied
	 */
	ii->type_info.ovpn = *type_info;

	/*
	 * Compute hash chain for insertion
	 */
	hash_index = ecm_db_iface_generate_hash_index_ovpn(type_info->tun_ifnum);

	ecm_db_iface_add_to_db(ii, hash_index);
}
EXPORT_SYMBOL(ecm_db_iface_add_ovpn);
#endif

#ifdef ECM_INTERFACE_VXLAN_ENABLE
/*
 * ecm_db_iface_add_vxlan()
 *	Add a iface instance into the database
 */
void ecm_db_iface_add_vxlan(struct ecm_db_iface_instance *ii, uint32_t vni, uint32_t if_type,
					char *name, int32_t mtu, int32_t interface_identifier,
					int32_t ae_interface_identifier,
					ecm_db_iface_final_callback_t final, void *arg)
{
	ecm_db_iface_hash_t hash_index;
	struct ecm_db_interface_info_vxlan *type_info;

	spin_lock_bh(&ecm_db_lock);
	DEBUG_CHECK_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC, "%px: magic failed\n", ii);
#ifdef ECM_DB_XREF_ENABLE
	DEBUG_ASSERT((ii->nodes == NULL) && (ii->node_count == 0), "%px: nodes not null\n", ii);
#endif
	DEBUG_ASSERT(!(ii->flags & ECM_DB_IFACE_FLAGS_INSERTED), "%px: inserted\n", ii);
	DEBUG_ASSERT(name, "%px: no name given\n", ii);
	spin_unlock_bh(&ecm_db_lock);

	/*
	 * Record general info
	 */
	ii->type = ECM_DB_IFACE_TYPE_VXLAN;
#ifdef ECM_STATE_OUTPUT_ENABLE
	ii->state_get = ecm_db_iface_vxlan_state_get;
#endif
	ii->arg = arg;
	ii->final = final;
	strlcpy(ii->name, name, IFNAMSIZ);
	ii->mtu = mtu;
	ii->interface_identifier = interface_identifier;
	ii->ae_interface_identifier = ae_interface_identifier;

	/*
	 * Type specific info
	 */
	type_info = &ii->type_info.vxlan;
	type_info->vni = vni;
	type_info->if_type = if_type;

	/*
	 * Compute hash chain for insertion
	 */
	hash_index = ecm_db_iface_generate_hash_index_vxlan(vni, if_type);

	ecm_db_iface_add_to_db(ii, hash_index);
}
#endif

/*
 * ecm_db_iface_alloc()
 *	Allocate a iface instance
 */
struct ecm_db_iface_instance *ecm_db_iface_alloc(void)
{
	struct ecm_db_iface_instance *ii;

	ii = (struct ecm_db_iface_instance *)kzalloc(sizeof(struct ecm_db_iface_instance), GFP_ATOMIC | __GFP_NOWARN);
	if (!ii) {
		DEBUG_WARN("Alloc failed\n");
		return NULL;
	}

	ii->refs = 1;
	DEBUG_SET_MAGIC(ii, ECM_DB_IFACE_INSTANCE_MAGIC);

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
		kfree(ii);
		return NULL;
	}

	ecm_db_iface_count++;
	spin_unlock_bh(&ecm_db_lock);

	DEBUG_TRACE("iface created %px\n", ii);
	return ii;
}
EXPORT_SYMBOL(ecm_db_iface_alloc);

/*
 * ecm_db_iface_init()
 */
bool ecm_db_iface_init(struct dentry *dentry)
{
	if (!ecm_debugfs_create_u32("iface_count", S_IRUGO, dentry,
					(u32 *)&ecm_db_iface_count)) {
		DEBUG_ERROR("Failed to create ecm db iface count file in debugfs\n");
		return false;
	}

	return true;
}
