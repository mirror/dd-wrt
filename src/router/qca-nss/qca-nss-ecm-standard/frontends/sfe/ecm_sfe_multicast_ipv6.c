/*
 **************************************************************************
 * Copyright (c) 2014-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_acct.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_l4proto.h>
#include <net/netfilter/nf_conntrack_zones.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/ipv6/nf_conntrack_ipv6.h>
#include <net/netfilter/ipv6/nf_defrag_ipv6.h>
#ifdef ECM_INTERFACE_BOND_ENABLE
#include <net/bonding.h>
#endif
#include <net/vxlan.h>
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
#define DEBUG_LEVEL ECM_SFE_MULTICAST_IPV6_DEBUG_LEVEL

#include <mc_ecm.h>

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
#include "ecm_classifier_default.h"
#include "ecm_interface.h"
#include "ecm_sfe_ipv6.h"
#include "ecm_sfe_multicast_ipv6.h"
#include "ecm_sfe_common.h"
#include "ecm_front_end_common.h"
#include "ecm_front_end_ipv6.h"

static int ecm_sfe_multicast_ipv6_accelerated_count = 0;
						/* Number of IPv6 multicast connections currently offloaded */

/*
 * ecm_sfe_multicast_ipv6_connection_update_callback()
 * 	Callback for handling Ack/Nack for update accelerate rules.
 */
static void ecm_sfe_multicast_ipv6_connection_update_callback(void *app_data, struct sfe_ipv6_msg *nim)
{
	struct sfe_ipv6_mc_rule_create_msg *nircm = &nim->msg.mc_rule_create;
	uint32_t serial = (uint32_t)(ecm_ptr_t)app_data;
	struct ecm_db_connection_instance *ci;
	struct ecm_front_end_connection_instance *feci;
	ip_addr_t flow_ip;
	ip_addr_t return_ip;

	/*TODO: If the response is NACK then decelerate the flow and flushes all rules */
	DEBUG_TRACE("%px: update callback, response received from FW : %u\n", nim, nim->cm.response);

	/*
	 * Is this a response to a create message?
	 */
	if (nim->cm.type != SFE_TX_CREATE_MULTICAST_RULE_MSG) {
		DEBUG_ERROR("%px: multicast update callback with improper type: %d, serial: %u\n", nim, nim->cm.type, serial);
		return;
	}

	/*
	 * Is this a response to a update rule message?
	 */
	if ( !(nircm->rule_flags & SFE_MC_RULE_CREATE_FLAG_MC_UPDATE)) {
		DEBUG_ERROR("%px: multicast update callback with improper type: %d, serial: %u\n", nim, nim->cm.type, serial);
		return;
	}

	/*
	 * Look up ecm connection so that we can update the status.
	 */
	ci = ecm_db_connection_serial_find_and_ref(serial);
	if (!ci) {
		DEBUG_TRACE("%px: multicast update callback, connection not found, serial: %u\n", nim, serial);
		return;
	}

	/*
	 * Release ref held for this ack/nack response.
	 * NOTE: It's okay to do this here, ci won't go away, because the ci is held as
	 * a result of the ecm_db_connection_serial_find_and_ref()
	 */
	ecm_db_connection_deref(ci);

	/*
	 * Get the front end instance
	 */
	feci = ecm_db_connection_front_end_get_and_ref(ci);
	DEBUG_CHECK_MAGIC(feci, ECM_FRONT_END_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", feci);

	ECM_SFE_IPV6_ADDR_TO_IP_ADDR(flow_ip, nircm->tuple.flow_ip);
	ECM_SFE_IPV6_ADDR_TO_IP_ADDR(return_ip, nircm->tuple.return_ip);

	/*
	 * Dump some useful trace information.
	 */
	DEBUG_TRACE("%px: Update accelerate response for connection: %px, serial: %u\n", feci, feci->ci, serial);
	DEBUG_TRACE("%px: valid_flags: %x\n", feci, nircm->valid_flags);
	DEBUG_TRACE("%px: flow_ip: " ECM_IP_ADDR_OCTAL_FMT ":%d\n", feci, ECM_IP_ADDR_TO_OCTAL(flow_ip), nircm->tuple.flow_ident);
	DEBUG_TRACE("%px: return_ip: " ECM_IP_ADDR_OCTAL_FMT ":%d\n", feci, ECM_IP_ADDR_TO_OCTAL(return_ip), nircm->tuple.return_ident);
	DEBUG_TRACE("%px: protocol: %d\n", feci, nircm->tuple.protocol);

	/*
	 * Release the connection.
	 */
	ecm_front_end_connection_deref(feci);
	ecm_db_connection_deref(ci);
	return;
}

/*
 * ecm_sfe_multicast_ipv6_connection_create_callback()
 * 	callback for handling create ack/nack calls for multicast create commands.
 */
static void ecm_sfe_multicast_ipv6_connection_create_callback(void *app_data, struct sfe_ipv6_msg *nim)
{
	struct sfe_ipv6_mc_rule_create_msg *__attribute__((unused))nircm = &nim->msg.mc_rule_create;
	uint32_t serial = (uint32_t)(ecm_ptr_t)app_data;
	struct ecm_db_connection_instance *ci;
	struct ecm_front_end_connection_instance *feci;
	ip_addr_t flow_ip;
	ip_addr_t return_ip;
	ecm_front_end_acceleration_mode_t result_mode;
	bool is_defunct = false;

	/*
	 * Is this a response to a create message?
	 */
	if (nim->cm.type != SFE_TX_CREATE_MULTICAST_RULE_MSG) {
		DEBUG_ERROR("%px: udp create callback with improper type: %d, serial: %u\n", nim, nim->cm.type, serial);
		return;
	}

	/*
	 * Look up ecm connection so that we can update the status.
	 */
	ci = ecm_db_connection_serial_find_and_ref(serial);
	if (!ci) {
		DEBUG_TRACE("%px: create callback, connection not found, serial: %u\n", nim, serial);
		return;
	}

	/*
	 * Release ref held for this ack/nack response.
	 * NOTE: It's okay to do this here, ci won't go away, because the ci is held as
	 * a result of the ecm_db_connection_serial_find_and_ref()
	 */
	ecm_db_connection_deref(ci);

	/*
	 * Get the front end instance
	 */
	feci = ecm_db_connection_front_end_get_and_ref(ci);
	DEBUG_CHECK_MAGIC(feci, ECM_FRONT_END_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", feci);

	ECM_SFE_IPV6_ADDR_TO_IP_ADDR(flow_ip, nircm->tuple.flow_ip);
	ECM_SFE_IPV6_ADDR_TO_IP_ADDR(return_ip, nircm->tuple.return_ip);

	/*
	 * Dump some useful trace information.
	 */
	DEBUG_TRACE("%px: accelerate response for connection: %px, serial: %u\n", feci, feci->ci, serial);
	DEBUG_TRACE("%px: rule_flags: %x, valid_flags: %x\n", feci, nircm->rule_flags, nircm->valid_flags);
	DEBUG_TRACE("%px: flow_ip: " ECM_IP_ADDR_OCTAL_FMT ":%d\n", feci, ECM_IP_ADDR_TO_OCTAL(flow_ip), nircm->tuple.flow_ident);
	DEBUG_TRACE("%px: return_ip: " ECM_IP_ADDR_OCTAL_FMT ":%d\n", feci, ECM_IP_ADDR_TO_OCTAL(return_ip), nircm->tuple.return_ident);
	DEBUG_TRACE("%px: protocol: %d\n", feci, nircm->tuple.protocol);

	/*
	 * Handle the creation result code.
	 */
	DEBUG_TRACE("%px: response: %d\n", feci, nim->cm.response);
	if (nim->cm.response != SFE_CMN_RESPONSE_ACK) {
		/*
		 * Creation command failed (specific reason ignored).
		 */
		DEBUG_TRACE("%px: accel nack: %d\n", feci, nim->cm.error);
		spin_lock_bh(&feci->lock);
		DEBUG_ASSERT(feci->accel_mode == ECM_FRONT_END_ACCELERATION_MODE_ACCEL_PENDING, "%px: Unexpected mode: %d\n", ci, feci->accel_mode);
		feci->stats.ae_nack++;
		feci->stats.ae_nack_total++;
		if (feci->stats.ae_nack >= feci->stats.ae_nack_limit) {
			/*
			 * Too many SFE rejections
			 */
			result_mode = ECM_FRONT_END_ACCELERATION_MODE_FAIL_ACCEL_ENGINE;
		} else {
			/*
			 * Revert to decelerated
			 */
			result_mode = ECM_FRONT_END_ACCELERATION_MODE_DECEL;
		}

		/*
		 * If connection is now defunct then set mode to ensure no further accel attempts occur
		 */
		if (feci->is_defunct) {
			result_mode = ECM_FRONT_END_ACCELERATION_MODE_FAIL_DEFUNCT;
		}
		spin_lock_bh(&ecm_sfe_ipv6_lock);
		_ecm_sfe_ipv6_accel_pending_clear(feci, result_mode);
		spin_unlock_bh(&ecm_sfe_ipv6_lock);

		spin_unlock_bh(&feci->lock);

		/*
		 * Release the connection.
		 */
		ecm_front_end_connection_deref(feci);
		ecm_db_connection_deref(ci);
		return;
	}

	spin_lock_bh(&feci->lock);
	DEBUG_ASSERT(feci->accel_mode == ECM_FRONT_END_ACCELERATION_MODE_ACCEL_PENDING, "%px: Unexpected mode: %d\n", ci, feci->accel_mode);

	/*
	 * If a flush occured before we got the ACK then our acceleration was effectively cancelled on us
	 * GGG TODO This is a workaround for a SFE message OOO quirk, this should eventually be removed.
	 */
	if (feci->stats.flush_happened) {
		feci->stats.flush_happened = false;

		/*
		 * Increment the no-action counter.  Our connectin was decelerated on us with no action occurring.
		 */
		feci->stats.no_action_seen++;
		spin_lock_bh(&ecm_sfe_ipv6_lock);
		_ecm_sfe_ipv6_accel_pending_clear(feci, ECM_FRONT_END_ACCELERATION_MODE_DECEL);
		spin_unlock_bh(&ecm_sfe_ipv6_lock);

		spin_unlock_bh(&feci->lock);

		/*
		 * Release the connection.
		 */
		ecm_front_end_connection_deref(feci);
		ecm_db_connection_deref(ci);
		return;
	}

	/*
	 * Create succeeded
	 */

	/*
	 * Clear any nack count
	 */
	feci->stats.ae_nack = 0;

	/*
	 * Clear the "accelerate pending" state and move to "accelerated" state bumping
	 * the accelerated counters to match our new state.
	 *
	 * Decelerate may have been attempted while we were accel pending.
	 * If decelerate is pending then we need to begin deceleration :-(
	 */
	spin_lock_bh(&ecm_sfe_ipv6_lock);

	ecm_sfe_multicast_ipv6_accelerated_count++;	/* Protocol specific counter */
	ecm_sfe_ipv6_accelerated_count++;		/* General running counter */

	if (!_ecm_sfe_ipv6_accel_pending_clear(feci, ECM_FRONT_END_ACCELERATION_MODE_ACCEL)) {
		/*
		 * Increement the no-action counter, this is reset if offload action is seen
		 */
		feci->stats.no_action_seen++;

		spin_unlock_bh(&ecm_sfe_ipv6_lock);
		spin_unlock_bh(&feci->lock);

		/*
		 * Release the connection.
		 */
		ecm_front_end_connection_deref(feci);
		ecm_db_connection_deref(ci);
		return;
	}

	DEBUG_INFO("%px: Decelerate was pending\n", ci);

	/*
	 * Check if the pending decelerate was done with the defunct process.
	 * If it was, set the is_defunct flag of the feci to false for re-try.
	 */
	if (feci->is_defunct) {
		is_defunct = feci->is_defunct;
		feci->is_defunct = false;
	}

	spin_unlock_bh(&ecm_sfe_ipv6_lock);
	spin_unlock_bh(&feci->lock);

	/*
	 * If the pending decelerate was done through defunct process, we should
	 * re-try it here with the same defunct function, because the purpose of that
	 * process is to remove the connection from the database as well after decelerating it.
	 */
	if (is_defunct) {
		ecm_db_connection_make_defunct(ci);
	} else {
		feci->decelerate(feci);
	}

	/*
	 * Release the connection.
	 */
	ecm_front_end_connection_deref(feci);
	ecm_db_connection_deref(ci);
}

/*
 * ecm_sfe_multicast_ipv6_connection_update_accelerate()
 * 	Push destination interface list updates for a multicast connection
 * 	to SFE
 *
 * 	This function receives a list of interfaces that have either left or joined the connection,
 * 	and sends a 'multicast update' command to SFE to inform about these interface state changes.
 */
static int ecm_sfe_multicast_ipv6_connection_update_accelerate(struct ecm_front_end_connection_instance *feci,
							       struct ecm_multicast_if_update *rp,
							       struct ecm_classifier_process_response *pr)
{
	uint16_t regen_occurrances;
	struct ecm_db_iface_instance *to_ifaces;
	struct ecm_db_iface_instance *ii_temp;
	struct ecm_db_iface_instance *ii_single;
	struct ecm_db_iface_instance **ifaces;
	struct ecm_db_iface_instance *from_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX];
	struct ecm_db_iface_instance *from_sfe_iface;
	int32_t *to_ifaces_first;
	int32_t *to_ii_first;
	struct sfe_ipv6_msg *nim;
	struct sfe_ipv6_mc_rule_create_msg *create;
	ip_addr_t addr;
	int32_t ret, vif;
	int32_t valid_vif_idx = 0;
	int32_t from_ifaces_first;
	int32_t to_sfe_iface_id = 0;
	int32_t from_sfe_iface_id = 0;
	uint8_t to_sfe_iface_address[ETH_ALEN];
	sfe_tx_status_t sfe_tx_status;
	int32_t list_index;
	int32_t to_mtu = 0;
	int32_t interface_type_counts[ECM_DB_IFACE_TYPE_COUNT];
	bool rule_invalid;
	uint8_t dest_mac[ETH_ALEN];
	uint32_t l2_accel_bits = (ECM_SFE_COMMON_FLOW_L2_ACCEL_ALLOWED | ECM_SFE_COMMON_RETURN_L2_ACCEL_ALLOWED);
	ecm_sfe_common_l2_accel_check_callback_t l2_accel_check;

	DEBUG_INFO("%px: UPDATE Accel conn: %px\n", feci, feci->ci);

	/*
	 * Get the re-generation occurrance counter of the connection.
	 * We compare it again at the end - to ensure that the rule construction has seen no generation
	 * changes during rule creation.
	 */
	regen_occurrances = ecm_db_connection_regeneration_occurrances_get(feci->ci);

	nim = (struct sfe_ipv6_msg *)kzalloc(sizeof(struct sfe_ipv6_msg), GFP_ATOMIC | __GFP_NOWARN);
	if (!nim) {
		return -1;
	}

	sfe_ipv6_msg_init(nim, SFE_SPECIAL_INTERFACE_IPV6, SFE_TX_CREATE_MULTICAST_RULE_MSG,
			sizeof(struct sfe_ipv6_mc_rule_create_msg),
			ecm_sfe_multicast_ipv6_connection_update_callback,
			(void *)(ecm_ptr_t)ecm_db_connection_serial_get(feci->ci));

	create = &nim->msg.mc_rule_create;

	create->valid_flags = 0;
	create->rule_flags = 0;

	/*
	 * Initialize VLAN tag information
	 */
	create->vlan_primary_rule.ingress_vlan_tag = SFE_VLAN_ID_NOT_CONFIGURED;
	create->vlan_primary_rule.egress_vlan_tag = SFE_VLAN_ID_NOT_CONFIGURED;
	create->vlan_secondary_rule.ingress_vlan_tag = SFE_VLAN_ID_NOT_CONFIGURED;
	create->vlan_secondary_rule.egress_vlan_tag = SFE_VLAN_ID_NOT_CONFIGURED;

	/*
	 * Populate the multicast creation structure
	 */
	from_ifaces_first = ecm_db_connection_interfaces_get_and_ref(feci->ci, from_ifaces, ECM_DB_OBJ_DIR_FROM);
	if (from_ifaces_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
		DEBUG_WARN("%px: Accel attempt failed - no interfaces in from_interfaces list!\n", feci);
		kfree(nim);
		return -1;
	}

	from_sfe_iface = from_ifaces[from_ifaces_first];
	from_sfe_iface_id = ecm_db_iface_ae_interface_identifier_get(from_sfe_iface);
	if (from_sfe_iface_id < 0) {
                DEBUG_TRACE("%px: from_sfe_iface_id: %d\n", feci, from_sfe_iface_id);
		ecm_db_connection_interfaces_deref(from_ifaces, from_ifaces_first);
		kfree(nim);
		return -1;
        }

	/*
	 * New rule being created
	 */
	create->valid_flags |= SFE_RULE_CREATE_CONN_VALID;

	/*
	 * Check which side of the connection can support L2 acceleration.
	 * The check is done only for the routed flows and if the L2 feature is enabled.
	 */
	if (sfe_is_l2_feature_enabled() && ecm_db_connection_is_routed_get(feci->ci)) {
		rcu_read_lock();
		l2_accel_check = rcu_dereference(ecm_sfe_cb.l2_accel_check);
		if (l2_accel_check) {
			struct ecm_sfe_common_tuple l2_accel_tuple;

			ecm_sfe_common_tuple_set(feci, from_sfe_iface_id, 0, &l2_accel_tuple);

			l2_accel_bits = l2_accel_check(&l2_accel_tuple);
		}
		rcu_read_unlock();
	}

	/*
	 * Set interface numbers involved in accelerating this connection.
	 * These are the outer facing addresses from the heirarchy interface lists we got above.
	 * These may be overridden later if we detect special interface types e.g. ipsec.
	 */
	create->conn_rule.flow_interface_num = from_sfe_iface_id;

	/*
	 * Set interface numbers involved in accelerating this connection.
	 * These are the inner facing addresses from the heirarchy interface lists we got above.
	 */
	create->conn_rule.flow_top_interface_num = ecm_db_iface_interface_identifier_get(from_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX-1]);

	ecm_db_connection_interfaces_deref(from_ifaces, from_ifaces_first);

	/*
	 * Construct an accel command.
	 */
	ret = ecm_db_multicast_connection_to_interfaces_get_and_ref_all(feci->ci, &to_ifaces, &to_ifaces_first);
	if (ret == 0) {
		DEBUG_WARN("%px: Accel attempt failed - no interfaces in to_interfaces list!\n", feci);
		kfree(nim);
		return -1;
	}

	/*
	 * Now examine the TO / DEST heirarchy list to construct the destination interface
	 * information
	 */
	DEBUG_TRACE("%px: Examine to/dest heirarchy list\n", feci);
	rule_invalid = false;

	/*
	 * Loop through the list of interface updates
	 */
	for (vif = 0; vif < ECM_DB_MULTICAST_IF_MAX; vif++) {
#ifdef ECM_INTERFACE_VLAN_ENABLE
		create->if_rule[vif].egress_vlan_tag[0] = ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED;
		create->if_rule[vif].egress_vlan_tag[1] = ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED;
#endif
		/*
		 * If there is no state change for an interface at this index,
		 * then ignore
		 */
		if (!(rp->if_join_idx[vif] || rp->if_leave_idx[vif])) {
			continue;
		}

		ii_temp = ecm_db_multicast_if_heirarchy_get(to_ifaces, vif);

		/*
		 * We have an update for this interface. Construct the interface information
		 */
		to_sfe_iface_id = -1;
		memset(interface_type_counts, 0, sizeof(interface_type_counts));
		to_ii_first = ecm_db_multicast_if_first_get_at_index(to_ifaces_first, vif);

		for (list_index = *to_ii_first; !rule_invalid && (list_index < ECM_DB_IFACE_HEIRARCHY_MAX); list_index++) {
			struct ecm_db_iface_instance *ii;
			ecm_db_iface_type_t ii_type;
			char *ii_name;
#ifdef ECM_INTERFACE_PPPOE_ENABLE
			struct ecm_db_interface_info_pppoe pppoe_info;
#endif
#ifdef ECM_INTERFACE_VLAN_ENABLE
			struct ecm_db_interface_info_vlan vlan_info;
			uint32_t vlan_value = 0;
			struct net_device *vlan_out_dev = NULL;
#endif

			ii_single = ecm_db_multicast_if_instance_get_at_index(ii_temp, list_index);
			ifaces = (struct ecm_db_iface_instance **)ii_single;
			ii = *ifaces;
			ii_type = ecm_db_iface_type_get(ii);
			ii_name = ecm_db_interface_type_to_string(ii_type);
			DEBUG_TRACE("%px: list_index: %d, ii: %px, type: %d (%s)\n", feci, list_index, ii, ii_type, ii_name);

			/*
			 * Extract information from this interface type if it is applicable to the rule.
			 * Conflicting information may cause accel to be unsupported.
			 */
			switch (ii_type) {
			case ECM_DB_IFACE_TYPE_BRIDGE:
				DEBUG_TRACE("%px: Bridge\n", feci);
				if (interface_type_counts[ii_type] != 0) {
					/*
					 * Cannot cascade bridges
					 */
					rule_invalid = true;
					DEBUG_TRACE("%px: Bridge - ignore additional\n", feci);
					break;
				}
				if (ecm_sfe_common_is_l2_iface_supported(ECM_DB_IFACE_TYPE_BRIDGE, list_index, *to_ifaces_first)) {
					create->rule_flags |= SFE_RULE_CREATE_FLAG_USE_RETURN_BOTTOM_INTERFACE;
					feci->set_stats_bitmap(feci, ECM_DB_OBJ_DIR_TO, ECM_DB_IFACE_TYPE_BRIDGE);
				}
				ecm_db_iface_bridge_address_get(ii, to_sfe_iface_address);
				if (is_valid_ether_addr(to_sfe_iface_address)) {
					ether_addr_copy((uint8_t *)create->src_mac_rule.return_src_mac,	to_sfe_iface_address);
					create->src_mac_rule.mac_valid_flags |=	SFE_SRC_MAC_RETURN_VALID;
					create->valid_flags |= SFE_RULE_CREATE_SRC_MAC_VALID;
				}
				DEBUG_TRACE("%px: Bridge - mac: %pM\n", feci, to_sfe_iface_address);

				break;
			case ECM_DB_IFACE_TYPE_ETHERNET:
				 DEBUG_TRACE("%px: Ethernet\n", feci);
				if (interface_type_counts[ii_type] != 0) {

					/*
					 * Ignore additional mac addresses, these are usually as a result of address propagation
					 * from bridges down to ports etc. */

					DEBUG_TRACE("%px: Ethernet - ignore additional\n", feci);
					break;
				}

				/*
				 * Can only handle one MAC, the first outermost mac.
				 */
				ecm_db_iface_ethernet_address_get(ii, to_sfe_iface_address);
				to_mtu = (uint32_t)ecm_db_connection_iface_mtu_get(feci->ci, ECM_DB_OBJ_DIR_TO);
				to_sfe_iface_id = ecm_db_iface_ae_interface_identifier_get(ii);
				if (to_sfe_iface_id < 0) {
					DEBUG_TRACE("%px: to_sfe_iface_id: %d\n", feci, to_sfe_iface_id);
					ecm_db_multicast_connection_to_interfaces_deref_all(to_ifaces, to_ifaces_first);
					kfree(nim);
					return -1;
			        }

				DEBUG_TRACE("%px: Ethernet - mac: %pM\n", feci, to_sfe_iface_address);
				break;
			case ECM_DB_IFACE_TYPE_PPPOE:
#ifdef ECM_INTERFACE_PPPOE_ENABLE
				/*
				 * More than one PPPoE in the list is not valid!
				 */
				if (interface_type_counts[ii_type] != 0) {
					DEBUG_TRACE("%px: PPPoE - additional unsupported\n", feci);
					rule_invalid = true;
					break;
				}
				/*
				 * PPPoE is supported only when L2 feature flag is enabled
				 */
				if (!sfe_is_l2_feature_enabled()) {
					DEBUG_TRACE("%px: PPPoE - unsupported\n", feci);
					break;
				}

				/*
				 * If external module decide that L2 acceleration is not allowed, we should return
				 * without setting PPPoE parameters.
				 */
				if (!(l2_accel_bits & ECM_SFE_COMMON_RETURN_L2_ACCEL_ALLOWED)) {
					DEBUG_TRACE("%px: L2 acceleration is not allowed for the PPPoE interface\n", feci);
					break;
				}

				feci->set_stats_bitmap(feci, ECM_DB_OBJ_DIR_TO, ECM_DB_IFACE_TYPE_PPPOE);

				/*
				 * Copy pppoe session info to the creation structure.
				 */
				ecm_db_iface_pppoe_session_info_get(ii, &pppoe_info);
				create->if_rule[valid_vif_idx].pppoe_session_id = pppoe_info.pppoe_session_id;
				memcpy(create->if_rule[valid_vif_idx].pppoe_remote_mac, pppoe_info.remote_mac, ETH_ALEN);
				create->if_rule[valid_vif_idx].valid_flags |= SFE_RULE_CREATE_PPPOE_ENCAP_VALID;
				create->if_rule[valid_vif_idx].rule_flags |= SFE_RULE_CREATE_FLAG_USE_RETURN_BOTTOM_INTERFACE;
				DEBUG_TRACE("%px: PPPoE - session: %x, mac:%pM\n", feci,
						create->if_rule[valid_vif_idx].pppoe_session_id,
						create->if_rule[valid_vif_idx].pppoe_remote_mac);

				create->if_rule[valid_vif_idx].valid_flags |= SFE_MC_RULE_CREATE_IF_FLAG_PPPOE_VALID;

#else
				DEBUG_TRACE("%px: PPPoE - unsupported\n", feci);
				rule_invalid = true;
#endif
				break;
			case ECM_DB_IFACE_TYPE_VLAN:
#ifdef ECM_INTERFACE_VLAN_ENABLE
				DEBUG_TRACE("%px: VLAN\n", feci);
				if (interface_type_counts[ii_type] > 1) {
					/*
					 * Can only support two vlans
					 */
					rule_invalid = true;
					DEBUG_TRACE("%px: VLAN - additional unsupported\n", feci);
					break;
				}
				ecm_db_iface_vlan_info_get(ii, &vlan_info);
				vlan_value = ((vlan_info.vlan_tpid << 16) | vlan_info.vlan_tag);
				/*
				 * Look up the vlan device and incorporate the vlan priority into the vlan_value
				 */
				vlan_out_dev = dev_get_by_index(&init_net, ecm_db_iface_interface_identifier_get(ii));
				if (vlan_out_dev) {
					vlan_value |= vlan_dev_get_egress_qos_mask(vlan_out_dev, pr->flow_qos_tag);
					dev_put(vlan_out_dev);
					vlan_out_dev = NULL;
				}
				/*
				 * Primary or secondary (QinQ) VLAN?
				 */
				create->if_rule[valid_vif_idx].egress_vlan_tag[interface_type_counts[ii_type]] = vlan_value;
				create->valid_flags |= SFE_RULE_CREATE_VLAN_VALID;
				if (sfe_is_l2_feature_enabled() && (l2_accel_bits & ECM_SFE_COMMON_RETURN_L2_ACCEL_ALLOWED)) {
					create->if_rule[valid_vif_idx].rule_flags |= SFE_RULE_CREATE_FLAG_USE_RETURN_BOTTOM_INTERFACE;
					feci->set_stats_bitmap(feci, ECM_DB_OBJ_DIR_TO, ECM_DB_IFACE_TYPE_VLAN);
					if (is_valid_ether_addr(vlan_info.address)) {
						ether_addr_copy((uint8_t *)create->src_mac_rule.return_src_mac, vlan_info.address);
						create->src_mac_rule.mac_valid_flags |= SFE_SRC_MAC_RETURN_VALID;
						create->valid_flags |= SFE_RULE_CREATE_SRC_MAC_VALID;
					}
				}
				/*
				 * If we have not yet got an ethernet mac then take this one
				 * (very unlikely as mac should have been propagated to the slave (outer) device)
				 */
				if (interface_type_counts[ECM_DB_IFACE_TYPE_ETHERNET] == 0) {
					interface_type_counts[ECM_DB_IFACE_TYPE_ETHERNET]++;
					DEBUG_TRACE("%px: VLAN use mac: %pM\n", feci, vlan_info.address);
				}
				DEBUG_TRACE("%px: vlan tag: %x\n", feci, vlan_value);
				create->if_rule[valid_vif_idx].valid_flags |= SFE_MC_RULE_CREATE_IF_FLAG_VLAN_VALID;
#else
				rule_invalid = true;
				DEBUG_TRACE("%px: VLAN - unsupported\n", feci);
#endif
				break;
			default:
				DEBUG_TRACE("%px: Ignoring: %d (%s)\n", feci, ii_type, ii_name);
			}

			/*
			 * Seen an interface of this type
			 */
			interface_type_counts[ii_type]++;
		}

		if (rule_invalid) {
			DEBUG_WARN("%px: to/dest Rule invalid\n", feci);
			ecm_db_multicast_connection_to_interfaces_deref_all(to_ifaces, to_ifaces_first);
			kfree(nim);
			return -1;
		}

		/*
		 * Is this a valid interface?
		 */
		if (to_sfe_iface_id != -1) {
			bool is_bridge;
			create->if_rule[valid_vif_idx].if_num = to_sfe_iface_id;
			create->if_rule[valid_vif_idx].if_mtu = to_mtu;
			if (rp->if_join_idx[vif]) {

				/*
				 * The interface has joined the group
				 */
				create->if_rule[valid_vif_idx].rule_flags |= SFE_MC_RULE_CREATE_IF_FLAG_JOIN;
#ifdef ECM_CLASSIFIER_OVS_ENABLE
				if (pr->process_actions & ECM_CLASSIFIER_PROCESS_ACTION_OVS_VLAN_TAG) {
					/*
					 * Set primary egress VLAN tag
					 */
					if (pr->egress_mc_vlan_tag[vif][0] != ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED) {
						create->if_rule[valid_vif_idx].egress_vlan_tag[0] = pr->egress_mc_vlan_tag[vif][0];
						create->if_rule[valid_vif_idx].valid_flags |= SFE_MC_RULE_CREATE_IF_FLAG_VLAN_VALID;

						/*
						 * Set secondary egress VLAN tag
						 */
						if (pr->egress_mc_vlan_tag[vif][1] != ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED) {
							create->if_rule[valid_vif_idx].egress_vlan_tag[1] = pr->egress_mc_vlan_tag[vif][1];
						}
					}
				}
#endif
			} else if (rp->if_leave_idx[vif]) {

				/*
				 * The interface has left the group
				 */
				create->if_rule[valid_vif_idx].rule_flags |= SFE_MC_RULE_CREATE_IF_FLAG_LEAVE;
			}

			is_bridge = !ecm_db_connection_is_routed_get(feci->ci);

			/*
			 * Do not set the ROUTED flag for pure bridged interfaces
			 */
			if (is_bridge) {
				uint8_t from_sfe_iface_address[ETH_ALEN];
				ecm_db_connection_node_address_get(feci->ci, ECM_DB_OBJ_DIR_FROM, (uint8_t *)from_sfe_iface_address);
				memcpy(create->if_rule[valid_vif_idx].if_mac, from_sfe_iface_address, ETH_ALEN);
				create->if_rule[valid_vif_idx].rule_flags |= SFE_MC_RULE_CREATE_IF_FLAG_BRIDGE_FLOW;
				create->rule_flags |= SFE_RULE_CREATE_FLAG_BRIDGE_FLOW;
			} else {
				memcpy(create->if_rule[valid_vif_idx].if_mac, to_sfe_iface_address, ETH_ALEN);
				create->if_rule[valid_vif_idx].rule_flags |= SFE_MC_RULE_CREATE_IF_FLAG_ROUTED_FLOW;
			}

			ecm_sfe_common_fast_xmit_set(&(create->if_rule[valid_vif_idx].rule_flags),
					&(create->if_rule[valid_vif_idx].valid_flags),
					&(create->if_rule[valid_vif_idx].qdisc_rule),
					from_ifaces, (struct ecm_db_iface_instance **)ii_temp, from_ifaces_first, *to_ii_first);

			valid_vif_idx++;
		}
	}

	/*
	 * Set number of interface updates in the update rule
	 */
	create->if_cnt = valid_vif_idx;

	/*
	 * Set the UPDATE flag
	 */
	create->rule_flags |= SFE_MC_RULE_CREATE_FLAG_MC_UPDATE;

	/*
	 * Set protocol
	 */
	create->tuple.protocol = IPPROTO_UDP;

	/*
	 * The src_ip is where the connection established from
	 */
	ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_FROM, addr);
	ECM_IP_ADDR_TO_SFE_IPV6_ADDR(create->tuple.flow_ip, addr);

	/*
	 * The destination address is what the destination IP is translated to as it is forwarded to the next interface.
	 * For egress this would yield the normal wan host and for ingress this would correctly NAT back to the LAN host
	 */
	ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_TO, addr);
	ECM_IP_ADDR_TO_SFE_IPV6_ADDR(create->tuple.return_ip, addr);

	/*
	 * Same approach as above for port information
	 */
	create->tuple.flow_ident = htons(ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_FROM));
	create->tuple.return_ident = htons(ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_TO));

	/*
	 * Get mac addresses.
	 * The src_mac is the mac address of the node that established the connection.
	 * This will work whether the from_node is LAN (egress) or WAN (ingress).
	 */
	ecm_db_connection_node_address_get(feci->ci, ECM_DB_OBJ_DIR_FROM, (uint8_t *)create->conn_rule.flow_mac);

	/*
	 * Destination Node(MAC) address. This address will be same for all to side intefaces
	 */
	ecm_db_connection_node_address_get(feci->ci, ECM_DB_OBJ_DIR_TO, dest_mac);
	memcpy(create->dest_mac, dest_mac, ETH_ALEN);

	ecm_db_multicast_connection_to_interfaces_deref_all(to_ifaces, to_ifaces_first);

	for (vif = 0; vif < valid_vif_idx ; vif++) {
		DEBUG_TRACE("ACCEL UPDATE %px: UDP Accelerate connection %px\n"
				"Rule flag: %x\n"
				"Vif: %d\n"
				"Protocol: %d\n"
				"from_ip: " ECM_IP_ADDR_OCTAL_FMT ":%d\n"
				"to_ip: " ECM_IP_ADDR_OCTAL_FMT ":%d\n"
				"to_mtu: %u\n"
				"to_mac: %pM\n"
				"dest_iface_num: %u\n"
				"out_vlan[0] %x\n"
				"out_vlan[1] %x\n",
				feci,
				feci->ci,
				create->if_rule[vif].rule_flags,
				vif,
				create->tuple.protocol,
				ECM_IP_ADDR_TO_OCTAL(create->tuple.flow_ip), create->tuple.flow_ident,
				ECM_IP_ADDR_TO_OCTAL(create->tuple.return_ip), create->tuple.return_ident,
				create->if_rule[vif].if_mtu,
				create->if_rule[vif].if_mac,
				create->if_rule[vif].if_num,
				create->if_rule[vif].egress_vlan_tag[0],
				create->if_rule[vif].egress_vlan_tag[1]);

	}

	/*
	 * Now that the rule has been constructed we re-compare the generation occurrance counter.
	 * If there has been a change then we abort because the rule may have been created using
	 * unstable data - especially if another thread has begun regeneration of the connection state.
	 * NOTE: This does not prevent a regen from being flagged immediately after this line of code either,
	 * or while the acceleration rule is in flight to the sfe.
	 * This is only to check for consistency of rule state - not that the state is stale.
	 */
	if (regen_occurrances != ecm_db_connection_regeneration_occurrances_get(feci->ci)) {
		DEBUG_INFO("%px: connection:%px regen occurred - aborting accel rule.\n", feci, feci->ci);
		kfree(nim);
		return -1;
	}

	/*
	 * Ref the connection before issuing an SFE rule
	 * This ensures that when the SFE responds to the command - which may even be immediately -
	 * the callback function can trust the correct ref was taken for its purpose.
	 * NOTE: remember that this will also implicitly hold the feci.
	 */
	ecm_db_connection_ref(feci->ci);

	/*
	 * We are about to issue the command, record the time of transmission
	 */
	spin_lock_bh(&feci->lock);
	feci->stats.cmd_time_begun = jiffies;
	spin_unlock_bh(&feci->lock);

	/*
	 * Call the rule create function
	 */
	sfe_tx_status = sfe_ipv6_tx(ecm_sfe_ipv6_mgr, nim);
	if (sfe_tx_status == SFE_TX_SUCCESS) {
		spin_lock_bh(&feci->lock);
		feci->stats.driver_fail = 0;		/* Reset */
		spin_unlock_bh(&feci->lock);
		kfree(nim);
		return 0;
	}

	/*
	 * Revert accel mode if necessary
	 */
	DEBUG_WARN("%px: ACCEL UPDATE attempt failed\n", feci);

	/*
	 * Release that ref!
	 */
	ecm_db_connection_deref(feci->ci);

	kfree(nim);

	/*
	 * TX failed
	 */
	spin_lock_bh(&feci->lock);
	feci->stats.driver_fail_total++;
	feci->stats.driver_fail++;
	if (feci->stats.driver_fail >= feci->stats.driver_fail_limit) {
		DEBUG_WARN("%px: Accel failed - driver fail limit\n", feci);
		feci->accel_mode = ECM_FRONT_END_ACCELERATION_MODE_FAIL_DRIVER;
	}
	spin_unlock_bh(&feci->lock);
	return -1;
}

/*
 * ecm_sfe_multicast_ipv6_connection_accelerate()
 * 	Accelerate a multicast UDP connection
 */
static void ecm_sfe_multicast_ipv6_connection_accelerate(struct ecm_front_end_connection_instance *feci,
                                                                        struct ecm_classifier_process_response *pr, bool is_l2_encap,
                                                                        struct nf_conn *ct, struct sk_buff *skb)
{
	uint16_t regen_occurrances;
	struct ecm_db_iface_instance *from_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX];
	struct ecm_db_iface_instance *from_nat_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX];
	struct ecm_db_iface_instance *from_sfe_iface;
	int32_t from_ifaces_first;
	int32_t from_nat_ifaces_first;
	struct ecm_db_iface_instance *to_ifaces;
	struct ecm_db_iface_instance *ii_temp;
	struct ecm_db_iface_instance *ii_single;
	struct ecm_db_iface_instance **ifaces;
	struct sfe_ipv6_msg *nim;
	int32_t *to_ifaces_first;
	int32_t *to_ii_first;
	int32_t from_nat_ifaces_identifier = 0;
	int32_t from_sfe_iface_id;
	int32_t to_sfe_iface_id;
	uint8_t from_sfe_iface_address[ETH_ALEN];
	uint8_t to_sfe_iface_address[ETH_ALEN];
	ip_addr_t addr;
	struct sfe_ipv6_mc_rule_create_msg *create;
	struct ecm_classifier_instance *assignments[ECM_CLASSIFIER_TYPES];
	int aci_index;
	int32_t vif, ret;
	int assignment_count;
	sfe_tx_status_t sfe_tx_status;
	int32_t list_index;
	int32_t valid_vif_idx = 0;
	int32_t interface_type_counts[ECM_DB_IFACE_TYPE_COUNT];
	uint8_t dest_mac[ETH_ALEN];
	bool rule_invalid;
	ecm_front_end_acceleration_mode_t result_mode;
	uint32_t l2_accel_bits = (ECM_SFE_COMMON_FLOW_L2_ACCEL_ALLOWED | ECM_SFE_COMMON_RETURN_L2_ACCEL_ALLOWED);
	ecm_sfe_common_l2_accel_check_callback_t l2_accel_check;

	DEBUG_CHECK_MAGIC(feci, ECM_FRONT_END_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", feci);

	/*
	 * Get the re-generation occurrance counter of the connection.
	 * We compare it again at the end - to ensure that the rule construction has seen no generation
	 * changes during rule creation.
	 */
	regen_occurrances = ecm_db_connection_regeneration_occurrances_get(feci->ci);

	/*
	 * Can this connection be accelerated at all?
	 */
	if (!ecm_sfe_ipv6_accel_pending_set(feci)) {
		DEBUG_TRACE("%px: Acceleration denied: %px\n", feci, feci->ci);
		return;
	}

	/*
	 * Construct an accel command.
	 * Initialise Multicast create structure.
	 * NOTE: We leverage the app_data void pointer to be our 32 bit connection serial number.
	 * When we get it back we re-cast it to a uint32 and do a faster connection lookup.
	 */
	nim = (struct sfe_ipv6_msg *)kzalloc(sizeof(struct sfe_ipv6_msg), GFP_ATOMIC | __GFP_NOWARN);
	if (!nim) {
		ecm_sfe_ipv6_accel_pending_clear(feci, ECM_FRONT_END_ACCELERATION_MODE_DECEL);
		return;
	}

	sfe_ipv6_msg_init(nim, SFE_SPECIAL_INTERFACE_IPV6, SFE_TX_CREATE_MULTICAST_RULE_MSG,
			sizeof(struct sfe_ipv6_mc_rule_create_msg),
			 ecm_sfe_multicast_ipv6_connection_create_callback,
			(void *)(ecm_ptr_t)ecm_db_connection_serial_get(feci->ci));

	create = &nim->msg.mc_rule_create;

	create->valid_flags = 0;
	create->rule_flags = 0;

	/*
	 * Initialize VLAN tag information
	 */
	create->vlan_primary_rule.ingress_vlan_tag = SFE_VLAN_ID_NOT_CONFIGURED;
	create->vlan_primary_rule.egress_vlan_tag = SFE_VLAN_ID_NOT_CONFIGURED;
	create->vlan_secondary_rule.ingress_vlan_tag = SFE_VLAN_ID_NOT_CONFIGURED;
	create->vlan_secondary_rule.egress_vlan_tag = SFE_VLAN_ID_NOT_CONFIGURED;

	/*
	 * Populate the multicast creation structure
	 */
	from_ifaces_first = ecm_db_connection_interfaces_get_and_ref(feci->ci, from_ifaces, ECM_DB_OBJ_DIR_FROM);
	if (from_ifaces_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
		DEBUG_WARN("%px: Accel attempt failed - no interfaces in from_interfaces list!\n", feci);
		kfree(nim);
		return;
	}

	from_sfe_iface = from_ifaces[from_ifaces_first];
	from_sfe_iface_id = ecm_db_iface_ae_interface_identifier_get(from_sfe_iface);
	if (from_sfe_iface_id < 0) {
                DEBUG_TRACE("%px: from_sfe_iface_id: %d\n", feci, from_sfe_iface_id);
		ecm_db_connection_interfaces_deref(from_ifaces, from_ifaces_first);
		kfree(nim);
		return;
        }

	/*
	 * New rule being created
	 */
	create->valid_flags |= SFE_RULE_CREATE_CONN_VALID;

	/*
	 * Check which side of the connection can support L2 acceleration.
	 * The check is done only for the routed flows and if the L2 feature is enabled.
	 */
	if (sfe_is_l2_feature_enabled() && ecm_db_connection_is_routed_get(feci->ci)) {
		rcu_read_lock();
		l2_accel_check = rcu_dereference(ecm_sfe_cb.l2_accel_check);
		if (l2_accel_check) {
			struct ecm_sfe_common_tuple l2_accel_tuple;

			ecm_sfe_common_tuple_set(feci, from_sfe_iface_id, 0, &l2_accel_tuple);

			l2_accel_bits = l2_accel_check(&l2_accel_tuple);
		}
		rcu_read_unlock();
	}

	/*
	 * Set interface numbers involved in accelerating this connection.
	 * These are the outer facing addresses from the heirarchy interface lists we got above.
	 * These may be overridden later if we detect special interface types e.g. ipsec.
	 */
	create->conn_rule.flow_interface_num = from_sfe_iface_id;

	/*
	 * Set interface numbers involved in accelerating this connection.
	 * These are the inner facing addresses from the heirarchy interface lists we got above.
	 */
	create->conn_rule.flow_top_interface_num = ecm_db_iface_interface_identifier_get(from_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX-1]);

	memset(interface_type_counts, 0, sizeof(interface_type_counts));
	rule_invalid = false;
	for (list_index = from_ifaces_first; list_index < ECM_DB_IFACE_HEIRARCHY_MAX; list_index++) {
		struct ecm_db_iface_instance *ii;
		ecm_db_iface_type_t ii_type;
		char *ii_name;

#ifdef ECM_INTERFACE_PPPOE_ENABLE
		struct ecm_db_interface_info_pppoe pppoe_info;
#endif
#ifdef ECM_INTERFACE_VLAN_ENABLE
		struct ecm_db_interface_info_vlan vlan_info;
		uint32_t vlan_value = 0;
		struct net_device *vlan_in_dev = NULL;
#endif

		ii = from_ifaces[list_index];
		ii_type = ecm_db_iface_type_get(ii);
		ii_name = ecm_db_interface_type_to_string(ii_type);
		DEBUG_TRACE("%px: list_index: %d, ii: %px, type: %d (%s)\n", feci, list_index, ii, ii_type, ii_name);

		/*
		 * Extract information from this interface type if it is applicable to the rule.
		 * Conflicting information may cause accel to be unsupported.
		 */
		switch (ii_type) {
		case ECM_DB_IFACE_TYPE_BRIDGE:
			DEBUG_TRACE("%px: Bridge\n", feci);
			if (ecm_sfe_common_is_l2_iface_supported(ECM_DB_IFACE_TYPE_BRIDGE, list_index, from_ifaces_first) && (l2_accel_bits & ECM_SFE_COMMON_FLOW_L2_ACCEL_ALLOWED)) {
				create->rule_flags |= SFE_RULE_CREATE_FLAG_USE_FLOW_BOTTOM_INTERFACE;
				feci->set_stats_bitmap(feci, ECM_DB_OBJ_DIR_FROM, ECM_DB_IFACE_TYPE_BRIDGE);
			}

			ecm_db_iface_bridge_address_get(ii, from_sfe_iface_address);
			if (is_valid_ether_addr(from_sfe_iface_address)) {
				ether_addr_copy((uint8_t *)create->src_mac_rule.flow_src_mac, from_sfe_iface_address);
				create->src_mac_rule.mac_valid_flags |= SFE_SRC_MAC_FLOW_VALID;
				create->valid_flags |= SFE_RULE_CREATE_SRC_MAC_VALID;
			}
			DEBUG_TRACE("%px: Bridge - mac: %pM\n", feci, from_sfe_iface_address);
			break;

		case ECM_DB_IFACE_TYPE_MACVLAN:
#ifdef ECM_INTERFACE_MACVLAN_ENABLE
			if (ecm_sfe_common_is_l2_iface_supported(ECM_DB_IFACE_TYPE_MACVLAN, list_index, from_ifaces_first) && (l2_accel_bits & ECM_SFE_COMMON_FLOW_L2_ACCEL_ALLOWED)) {
				create->rule_flags |= SFE_RULE_CREATE_FLAG_USE_FLOW_BOTTOM_INTERFACE;
				feci->set_stats_bitmap(feci, ECM_DB_OBJ_DIR_FROM, ECM_DB_IFACE_TYPE_MACVLAN);
			}

			ecm_db_iface_macvlan_address_get(ii, from_sfe_iface_address);
			ether_addr_copy((uint8_t *)create->src_mac_rule.flow_src_mac, from_sfe_iface_address);
			create->src_mac_rule.mac_valid_flags |= SFE_SRC_MAC_FLOW_VALID;
			create->valid_flags |= SFE_RULE_CREATE_SRC_MAC_VALID;

			DEBUG_TRACE("%px: Macvlan - mac: %pM\n", feci, from_sfe_iface_address);
#else
			rule_invalid = true;
			DEBUG_TRACE("%px: MACVLAN - unsupported\n", feci);
#endif
			break;

		case ECM_DB_IFACE_TYPE_VLAN:
#ifdef ECM_INTERFACE_VLAN_ENABLE
			if (interface_type_counts[ii_type] > 1) {
				/*
				 * Can only support two vlans
				 */
				rule_invalid = true;
				DEBUG_TRACE("%px: VLAN - additional unsupported\n", feci);
				break;
			}
			ecm_db_iface_vlan_info_get(ii, &vlan_info);
			vlan_value = ((vlan_info.vlan_tpid << 16) | vlan_info.vlan_tag);

			/*
			 * Look up the vlan device and incorporate the vlan priority into the vlan_value
			 */
			vlan_in_dev = dev_get_by_index(&init_net, ecm_db_iface_interface_identifier_get(ii));
			if (vlan_in_dev) {
				vlan_value |= vlan_dev_get_egress_qos_mask(vlan_in_dev, pr->return_qos_tag);
				dev_put(vlan_in_dev);
				vlan_in_dev = NULL;
			}

			/*
			 * Primary or secondary (QinQ) VLAN?
			 */
			if (interface_type_counts[ii_type] == 0) {
				create->vlan_primary_rule.ingress_vlan_tag = vlan_value;
			} else {
				create->vlan_secondary_rule.ingress_vlan_tag = vlan_value;
			}
			create->valid_flags |= SFE_RULE_CREATE_VLAN_VALID;

			if (sfe_is_l2_feature_enabled() && (l2_accel_bits & ECM_SFE_COMMON_FLOW_L2_ACCEL_ALLOWED)) {
				create->rule_flags |= SFE_RULE_CREATE_FLAG_USE_FLOW_BOTTOM_INTERFACE;
				feci->set_stats_bitmap(feci, ECM_DB_OBJ_DIR_FROM, ECM_DB_IFACE_TYPE_VLAN);

				if (is_valid_ether_addr(vlan_info.address)) {
					ether_addr_copy((uint8_t *)create->src_mac_rule.flow_src_mac, vlan_info.address);
					create->src_mac_rule.mac_valid_flags |= SFE_SRC_MAC_FLOW_VALID;
					create->valid_flags |= SFE_RULE_CREATE_SRC_MAC_VALID;
				}
			}

			/*
			 * If we have not yet got an ethernet mac then take this one
			 * (very unlikely as mac should have been propagated to the slave (outer) device)
			 */
			if (interface_type_counts[ECM_DB_IFACE_TYPE_ETHERNET] == 0) {
				interface_type_counts[ECM_DB_IFACE_TYPE_ETHERNET]++;
				DEBUG_TRACE("%px: VLAN use mac: %pM\n", feci, vlan_info.address);
			}
			DEBUG_TRACE("%px: vlan tag: %x\n", feci, vlan_value);
#else
			rule_invalid = true;
			DEBUG_TRACE("%px: VLAN - unsupported\n", feci);
#endif
			break;
		case ECM_DB_IFACE_TYPE_PPPOE:
#ifdef ECM_INTERFACE_PPPOE_ENABLE
			/*
			 * More than one PPPoE in the list is not valid!
			 */
			if (interface_type_counts[ii_type] != 0) {
				DEBUG_TRACE("%px: PPPoE - additional unsupported\n", feci);
				rule_invalid = true;
				break;
			}

			/*
			 * PPPoE is supported only when L2 feature flag is enabled
			 */
			if (!sfe_is_l2_feature_enabled()) {
				DEBUG_TRACE("%px: PPPoE - unsupported\n", feci);
				break;
			}

			/*
			 * If external module decide that L2 acceleration is not allowed, we should return
			 * without setting PPPoE parameters.
			 */
			if (!(l2_accel_bits & ECM_SFE_COMMON_FLOW_L2_ACCEL_ALLOWED)) {
				DEBUG_TRACE("%px: L2 acceleration is not allowed for the PPPoE interface\n", feci);
				break;
			}

			feci->set_stats_bitmap(feci, ECM_DB_OBJ_DIR_FROM, ECM_DB_IFACE_TYPE_PPPOE);

			/*
			 * Copy pppoe session info to the creation structure.
			 */
			ecm_db_iface_pppoe_session_info_get(ii, &pppoe_info);

			create->pppoe_rule.flow_pppoe_session_id = pppoe_info.pppoe_session_id;
			memcpy(create->pppoe_rule.flow_pppoe_remote_mac, pppoe_info.remote_mac, ETH_ALEN);
			create->valid_flags |= SFE_RULE_CREATE_PPPOE_DECAP_VALID;
			create->rule_flags |= SFE_RULE_CREATE_FLAG_USE_FLOW_BOTTOM_INTERFACE;

			DEBUG_TRACE("%px: PPPoE - session: %x, mac: %pM\n", feci,
					create->pppoe_rule.flow_pppoe_session_id,
					create->pppoe_rule.flow_pppoe_remote_mac);

#else
			rule_invalid = true;
#endif
			break;

		default:
			DEBUG_TRACE("%px: Ignoring: %d (%s)\n", feci, ii_type, ii_name);
		}
		interface_type_counts[ii_type]++;
	}

	ecm_db_connection_interfaces_deref(from_ifaces, from_ifaces_first);

	ret = ecm_db_multicast_connection_to_interfaces_get_and_ref_all(feci->ci, &to_ifaces, &to_ifaces_first);
	if (!ret) {
		DEBUG_WARN("%px: Accel attempt failed - no multicast interfaces in to_interfaces list!\n", feci);
		kfree(nim);
		return;
	}

	from_nat_ifaces_first = ecm_db_connection_interfaces_get_and_ref(feci->ci, from_nat_ifaces, ECM_DB_OBJ_DIR_FROM_NAT);
	from_nat_ifaces_identifier = ecm_db_iface_interface_identifier_get(from_nat_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX - 1]);
	ecm_db_connection_interfaces_deref(from_nat_ifaces, from_nat_ifaces_first);

	/*
	 * Now examine the TO / DEST heirarchy list to construct the destination part of the rule
	 */
	DEBUG_TRACE("%px: Examine to/dest heirarchy list\n", feci);
	rule_invalid = false;
	for (vif = 0; vif < ECM_DB_MULTICAST_IF_MAX; vif++) {
		int32_t found_nat_ii_match = 0;
		int32_t to_mtu = 0;
#ifdef ECM_INTERFACE_PPPOE_ENABLE
		struct ecm_db_interface_info_pppoe pppoe_info;
#endif

#ifdef ECM_INTERFACE_VLAN_ENABLE
		struct ecm_db_interface_info_vlan vlan_info;
		uint32_t vlan_value = 0;
		struct net_device *vlan_out_dev = NULL;
#endif
		to_sfe_iface_id = -1;


#ifdef ECM_INTERFACE_VLAN_ENABLE
		create->if_rule[vif].egress_vlan_tag[0] = ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED;
		create->if_rule[vif].egress_vlan_tag[1] = ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED;
#endif

		ii_temp = ecm_db_multicast_if_heirarchy_get(to_ifaces, vif);
		to_ii_first = ecm_db_multicast_if_first_get_at_index(to_ifaces_first, vif);
		memset(interface_type_counts, 0, sizeof(interface_type_counts));

		for (list_index = *to_ii_first; !rule_invalid && (list_index < ECM_DB_IFACE_HEIRARCHY_MAX); list_index++) {
			struct ecm_db_iface_instance *ii;
			int32_t ii_identifier;
			ecm_db_iface_type_t ii_type;
			char *ii_name;

			ii_single = ecm_db_multicast_if_instance_get_at_index(ii_temp, list_index);
			ifaces = (struct ecm_db_iface_instance **)ii_single;
			ii = *ifaces;
			ii_type = ecm_db_iface_type_get(ii);
			ii_name = ecm_db_interface_type_to_string(ii_type);
			ii_identifier = ecm_db_iface_interface_identifier_get(ii);

			/*
			 * Find match for NAT interface in Multicast destination interface list.
			 * If found match, set the found_nat_ii_match flag here.
			 */
			if (ii_identifier == from_nat_ifaces_identifier) {
				found_nat_ii_match = 1;
			}

			DEBUG_TRACE("%px: list_index: %d, ii: %px, type: %d (%s)\n", feci, list_index, ii, ii_type, ii_name);

			/*
			 * Extract information from this interface type if it is applicable to the rule.
			 * Conflicting information may cause accel to be unsupported.
			 */
			switch (ii_type) {
			case ECM_DB_IFACE_TYPE_BRIDGE:

				/*
				 * TODO: Find and set the bridge/route flag for this interface
				 */
				DEBUG_TRACE("%px: Bridge\n", feci);
				if (interface_type_counts[ii_type] != 0) {

					/*
					 * Cannot cascade bridges
					 */
					rule_invalid = true;
					DEBUG_TRACE("%px: Bridge - ignore additional\n", feci);
					break;
				}

				if (ecm_sfe_common_is_l2_iface_supported(ECM_DB_IFACE_TYPE_BRIDGE, list_index, *to_ifaces_first)) {
					create->rule_flags |= SFE_RULE_CREATE_FLAG_USE_RETURN_BOTTOM_INTERFACE;
					feci->set_stats_bitmap(feci, ECM_DB_OBJ_DIR_TO, ECM_DB_IFACE_TYPE_BRIDGE);
				}
				ecm_db_iface_bridge_address_get(ii, to_sfe_iface_address);
				if (is_valid_ether_addr(to_sfe_iface_address)) {
					ether_addr_copy((uint8_t *)create->src_mac_rule.return_src_mac,	to_sfe_iface_address);
					create->src_mac_rule.mac_valid_flags |=	SFE_SRC_MAC_RETURN_VALID;
					create->valid_flags |= SFE_RULE_CREATE_SRC_MAC_VALID;
				}
				DEBUG_TRACE("%px: Bridge - mac: %pM\n", feci, to_sfe_iface_address);
				break;
			case ECM_DB_IFACE_TYPE_ETHERNET:
				DEBUG_TRACE("%px: Ethernet\n", feci);
				if (interface_type_counts[ii_type] != 0) {

					/*
					 * Ignore additional mac addresses, these are usually as a result of address propagation
					 * from bridges down to ports etc.
					 */
					DEBUG_TRACE("%px: Ethernet - ignore additional\n", feci);
					break;
				}

				/*
				 * Can only handle one MAC, the first outermost mac.
				 */
				ecm_db_iface_ethernet_address_get(ii, to_sfe_iface_address);
				to_mtu = (uint32_t)ecm_db_connection_iface_mtu_get(feci->ci, ECM_DB_OBJ_DIR_TO);
				to_sfe_iface_id = ecm_db_iface_ae_interface_identifier_get(ii);
				if (to_sfe_iface_id < 0) {
					DEBUG_TRACE("%px: to_sfe_iface_id: %d\n", feci, to_sfe_iface_id);
					ecm_db_multicast_connection_to_interfaces_deref_all(to_ifaces, to_ifaces_first);
					kfree(nim);
					return;
			        }

				DEBUG_TRACE("%px: Ethernet - mac: %pM\n", feci, to_sfe_iface_address);
				break;
			case ECM_DB_IFACE_TYPE_LAG:
			{
#ifdef ECM_INTERFACE_BOND_ENABLE
				struct net_device *dev;
				DEBUG_TRACE("%px: LAG : %s\n", feci, dev->name);
				if (interface_type_counts[ii_type] != 0) {

					/*
					 * Ignore additional mac addresses, these are usually as a result of address propagation
					 * from bridges down to ports etc.
					 */
					DEBUG_TRACE("%px: LAG - ignore additional\n", feci);
					rule_invalid = true;
					break;
				}

				dev = dev_get_by_index(&init_net, ecm_db_iface_interface_identifier_get(ii));
				if (!dev) {
					DEBUG_TRACE("%px: LAG device is not present\n", feci);
					rule_invalid = true;
					break;
				}

				/* Multicast offload supported only for MLO LAG devices.
				 * Destination interface is MLO bond device itself
				 */
				if (!bond_is_mlo_device(dev)) {
					DEBUG_TRACE("%px: LAG device is not MLO device: %s\n", feci, dev->name);
					dev_put(dev);
					rule_invalid = true;
					break;
				}


				/*
				 * Can only handle one MAC, the first outermost mac.
				 */
				ecm_db_iface_lag_address_get(ii, to_sfe_iface_address);
				to_mtu = (uint32_t)ecm_db_connection_iface_mtu_get(feci->ci, ECM_DB_OBJ_DIR_TO);
				to_sfe_iface_id = ecm_db_iface_ae_interface_identifier_get(ii);
				if (to_sfe_iface_id < 0) {
					DEBUG_TRACE("%px: to_sfe_iface_id: %d\n", feci, to_sfe_iface_id);
					dev_put(dev);
					rule_invalid = true;
					break;
			        }
				DEBUG_TRACE("%px: LAG - mac: %pM, mtu %d\n", feci, to_sfe_iface_address, to_mtu);
				dev_put(dev);
#else
				DEBUG_TRACE("%px: LAG not supported\n", feci);
				rule_invalid = true;
#endif
				break;
			}
			case ECM_DB_IFACE_TYPE_PPPOE:
#ifdef ECM_INTERFACE_PPPOE_ENABLE
				/*
				 * More than one PPPoE in the list is not valid!
				 */
				if (interface_type_counts[ii_type] != 0) {
					DEBUG_TRACE("%px: PPPoE - additional unsupported\n", feci);
					rule_invalid = true;
					break;
				}
				/*
				 * PPPoE is supported only when L2 feature flag is enabled
				 */
				if (!sfe_is_l2_feature_enabled()) {
					DEBUG_TRACE("%px: PPPoE - unsupported\n", feci);
					break;
				}

				/*
				 * If external module decide that L2 acceleration is not allowed, we should return
				 * without setting PPPoE parameters.
				 */
				if (!(l2_accel_bits & ECM_SFE_COMMON_RETURN_L2_ACCEL_ALLOWED)) {
					DEBUG_TRACE("%px: L2 acceleration is not allowed for the PPPoE interface\n", feci);
					break;
				}

				feci->set_stats_bitmap(feci, ECM_DB_OBJ_DIR_TO, ECM_DB_IFACE_TYPE_PPPOE);

				/*
				 * Copy pppoe session info to the creation structure.
				 */
				ecm_db_iface_pppoe_session_info_get(ii, &pppoe_info);
				create->if_rule[valid_vif_idx].pppoe_session_id = pppoe_info.pppoe_session_id;
				memcpy(create->if_rule[valid_vif_idx].pppoe_remote_mac, pppoe_info.remote_mac, ETH_ALEN);
				create->if_rule[valid_vif_idx].valid_flags |= SFE_RULE_CREATE_PPPOE_ENCAP_VALID;
				create->if_rule[valid_vif_idx].rule_flags |= SFE_RULE_CREATE_FLAG_USE_RETURN_BOTTOM_INTERFACE;
				DEBUG_TRACE("%px: PPPoE - session: %x, mac:%pM\n", feci,
						create->if_rule[valid_vif_idx].pppoe_session_id,
						create->if_rule[valid_vif_idx].pppoe_remote_mac);

				create->if_rule[valid_vif_idx].valid_flags |= SFE_MC_RULE_CREATE_IF_FLAG_PPPOE_VALID;

#else
				DEBUG_TRACE("%px: PPPoE - unsupported\n", feci);
				rule_invalid = true;
#endif
				break;
			case ECM_DB_IFACE_TYPE_VLAN:
#ifdef ECM_INTERFACE_VLAN_ENABLE
				DEBUG_TRACE("%px: VLAN\n", feci);
				if (interface_type_counts[ii_type] > 1) {
					/*
					 * Can only support two vlans
					 */
					rule_invalid = true;
					DEBUG_TRACE("%px: VLAN - additional unsupported\n", feci);
					break;
				}
				ecm_db_iface_vlan_info_get(ii, &vlan_info);
				vlan_value = ((vlan_info.vlan_tpid << 16) | vlan_info.vlan_tag);
				/*
				 * Look up the vlan device and incorporate the vlan priority into the vlan_value
				 */
				vlan_out_dev = dev_get_by_index(&init_net, ecm_db_iface_interface_identifier_get(ii));
				if (vlan_out_dev) {
					vlan_value |= vlan_dev_get_egress_qos_mask(vlan_out_dev, pr->flow_qos_tag);
					dev_put(vlan_out_dev);
					vlan_out_dev = NULL;
				}
				/*
				 * Primary or secondary (QinQ) VLAN?
				 */
				create->if_rule[valid_vif_idx].egress_vlan_tag[interface_type_counts[ii_type]] = vlan_value;
				create->valid_flags |= SFE_RULE_CREATE_VLAN_VALID;
				if (sfe_is_l2_feature_enabled() && (l2_accel_bits & ECM_SFE_COMMON_RETURN_L2_ACCEL_ALLOWED)) {
					create->if_rule[valid_vif_idx].rule_flags |= SFE_RULE_CREATE_FLAG_USE_RETURN_BOTTOM_INTERFACE;
					feci->set_stats_bitmap(feci, ECM_DB_OBJ_DIR_TO, ECM_DB_IFACE_TYPE_VLAN);
					if (is_valid_ether_addr(vlan_info.address)) {
						ether_addr_copy((uint8_t *)create->src_mac_rule.return_src_mac, vlan_info.address);
						create->src_mac_rule.mac_valid_flags |= SFE_SRC_MAC_RETURN_VALID;
						create->valid_flags |= SFE_RULE_CREATE_SRC_MAC_VALID;
					}
				}
				/*
				 * If we have not yet got an ethernet mac then take this one
				 * (very unlikely as mac should have been propagated to the slave (outer) device)
				 */
				if (interface_type_counts[ECM_DB_IFACE_TYPE_ETHERNET] == 0) {
					interface_type_counts[ECM_DB_IFACE_TYPE_ETHERNET]++;
					DEBUG_TRACE("%px: VLAN use mac: %pM\n", feci, vlan_info.address);
				}
				DEBUG_TRACE("%px: vlan tag: %x\n", feci, vlan_value);
				create->if_rule[valid_vif_idx].valid_flags |= SFE_MC_RULE_CREATE_IF_FLAG_VLAN_VALID;

#else
				rule_invalid = true;
				DEBUG_TRACE("%px: VLAN - unsupported\n", feci);
#endif
				break;
			default:
				DEBUG_TRACE("%px: Ignoring: %d (%s)\n", feci, ii_type, ii_name);
			}

			/*
			 * Seen an interface of this type
			 */
			interface_type_counts[ii_type]++;
		}

		if (rule_invalid) {
			DEBUG_WARN("%px: to/dest Rule invalid\n", feci);
			ecm_db_multicast_connection_to_interfaces_deref_all(to_ifaces, to_ifaces_first);
			kfree(nim);
			ecm_sfe_ipv6_accel_pending_clear(feci, ECM_FRONT_END_ACCELERATION_MODE_FAIL_RULE);
			return;
		}

		/*
		 * Populate the interface details for a valid interface in the multicast destination
		 * interface list.
		 */
		if (to_sfe_iface_id != -1) {
			bool is_bridge;

			/*
			 * Set a rule for NAT if found_nat_ii_match flag is set
			 */
			if (found_nat_ii_match) {
				ip_addr_t xlate_sip;
				uint32_t xlate_src_ip[4];

				ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_FROM_NAT, xlate_sip);
				ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_FROM, addr);
				if (!ECM_IP_ADDR_MATCH(xlate_sip, addr)) {
					ECM_IP_ADDR_TO_SFE_IPV6_ADDR(xlate_src_ip, xlate_sip);
					create->if_rule[valid_vif_idx].xlate_src_ip[0] = xlate_src_ip[0];
					create->if_rule[valid_vif_idx].xlate_src_ip[1] = xlate_src_ip[1];
					create->if_rule[valid_vif_idx].xlate_src_ip[2] = xlate_src_ip[2];
					create->if_rule[valid_vif_idx].xlate_src_ip[3] = xlate_src_ip[3];
					create->if_rule[valid_vif_idx].xlate_src_ident = ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_FROM_NAT);
					create->if_rule[valid_vif_idx].valid_flags |= SFE_MC_RULE_CREATE_IF_FLAG_NAT_VALID;
				}
			}

			create->if_rule[valid_vif_idx].rule_flags |= SFE_MC_RULE_CREATE_IF_FLAG_JOIN;
			create->if_rule[valid_vif_idx].if_num = to_sfe_iface_id;
			create->if_rule[valid_vif_idx].if_mtu = to_mtu;

			is_bridge = !ecm_db_connection_is_routed_get(feci->ci);

			/*
			 * Identify if the destination interface blongs to pure bridge or routed flow.
			 */
			if (is_bridge) {
				uint8_t from_sfe_iface_address[ETH_ALEN];
				ecm_db_connection_node_address_get(feci->ci, ECM_DB_OBJ_DIR_FROM, (uint8_t *)from_sfe_iface_address);
				memcpy(create->if_rule[valid_vif_idx].if_mac, from_sfe_iface_address, ETH_ALEN);
				create->if_rule[valid_vif_idx].rule_flags |= SFE_MC_RULE_CREATE_IF_FLAG_BRIDGE_FLOW;
				create->rule_flags |= SFE_RULE_CREATE_FLAG_BRIDGE_FLOW;
			} else {
				memcpy(create->if_rule[valid_vif_idx].if_mac, to_sfe_iface_address, ETH_ALEN);
				create->if_rule[valid_vif_idx].rule_flags |= SFE_MC_RULE_CREATE_IF_FLAG_ROUTED_FLOW;
			}

			ecm_sfe_common_fast_xmit_set(&(create->if_rule[valid_vif_idx].rule_flags),
					&(create->if_rule[valid_vif_idx].valid_flags),
					&(create->if_rule[valid_vif_idx].qdisc_rule),
					from_ifaces, (struct ecm_db_iface_instance **)ii_temp, from_ifaces_first, *to_ii_first);

#ifdef ECM_CLASSIFIER_OVS_ENABLE
			if (pr->process_actions & ECM_CLASSIFIER_PROCESS_ACTION_OVS_VLAN_TAG) {
				/*
				 * Set primary egress VLAN tag
				 */
				if (pr->egress_mc_vlan_tag[vif][0] != ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED) {
					create->if_rule[valid_vif_idx].egress_vlan_tag[0] = pr->egress_mc_vlan_tag[vif][0];
					create->if_rule[valid_vif_idx].valid_flags |= SFE_MC_RULE_CREATE_IF_FLAG_VLAN_VALID;

					/*
					 * Set secondary egress VLAN tag
					 */
					if (pr->egress_mc_vlan_tag[vif][1] != ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED) {
						create->if_rule[valid_vif_idx].egress_vlan_tag[1] = pr->egress_mc_vlan_tag[vif][1];
					}
				}
			}
#endif
			valid_vif_idx++;
		}
	}

	create->if_cnt = valid_vif_idx;

	/*
	 * Set up the flow and return qos tags
	 */
	if (pr->process_actions & ECM_CLASSIFIER_PROCESS_ACTION_QOS_TAG)
	{
		create->qos_rule.flow_qos_tag = (uint32_t)pr->flow_qos_tag;
		create->qos_rule.return_qos_tag	= (uint32_t)pr->return_qos_tag;
		create->valid_flags |= SFE_RULE_CREATE_QOS_VALID;
	}

#ifdef ECM_CLASSIFIER_DSCP_ENABLE
	/*
	 * DSCP information?
	 */
	if (pr->process_actions & ECM_CLASSIFIER_PROCESS_ACTION_DSCP) {
		create->dscp_rule.flow_dscp = pr->flow_dscp;
		create->valid_flags |= SFE_MC_RULE_CREATE_FLAG_DSCP_MARKING_VALID;
	}
#endif

#ifdef ECM_CLASSIFIER_EMESH_ENABLE
	/*
	 * Mark the rule as E-MESH Service Prioritization valid.
	 */
	if (pr->process_actions & ECM_CLASSIFIER_PROCESS_ACTION_EMESH_SP_FLOW) {
		create->rule_flags |= SFE_MC_RULE_CREATE_FLAG_MC_EMESH_SP;
	}
#endif

#ifdef ECM_CLASSIFIER_OVS_ENABLE
	if (pr->process_actions & ECM_CLASSIFIER_PROCESS_ACTION_OVS_VLAN_TAG) {
		/*
		 * Set ingress VLAN tag
		 */
		if (pr->ingress_vlan_tag[0] != ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED) {
			create->vlan_primary_rule.ingress_vlan_tag = pr->ingress_vlan_tag[0];
			create->valid_flags |= SFE_MC_RULE_CREATE_FLAG_INGRESS_VLAN_VALID;
			ecm_db_multicast_tuple_set_ovs_ingress_vlan(feci->ci->ti, pr->ingress_vlan_tag);
		}
	}
#endif

	ecm_db_connection_node_address_get(feci->ci, ECM_DB_OBJ_DIR_TO, dest_mac);
	memcpy(create->dest_mac, dest_mac, ETH_ALEN);

	/*
	 * Bridge vlan passthrough
	 */
	if ((create->rule_flags & SFE_RULE_CREATE_FLAG_BRIDGE_FLOW) && (!(create->valid_flags & SFE_RULE_CREATE_VLAN_VALID))) {
		if (skb_vlan_tag_present(skb)) {
			create->rule_flags |= SFE_RULE_CREATE_FLAG_BRIDGE_VLAN_PASSTHROUGH;
		}
	}

	/*
	 * Set protocol
	 */
	create->tuple.protocol = IPPROTO_UDP;

	/*
	 * The flow_ip is where the connection established from
	 */
	ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_FROM, addr);
	ECM_IP_ADDR_TO_SFE_IPV6_ADDR(create->tuple.flow_ip, addr);

	/*
	 * The return_ip is where the connection is established to, however, in the case of ingress
	 * the return_ip would be the routers WAN IP - i.e. the NAT'ed version.
	 * Getting the NAT'ed version here works for ingress or egress packets, for egress
	 * the NAT'ed version would be the same as the normal address
	 */
	ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_TO_NAT, addr);
	ECM_IP_ADDR_TO_SFE_IPV6_ADDR(create->tuple.return_ip, addr);

	/*
	 * When the packet is forwarded to the next interface get the address the source IP of the
	 * packet should be translated to.  For egress this is the NAT'ed from address.
	 * This also works for ingress as the NAT'ed version of the WAN host would be the same as non-NAT'ed
	 */
	ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_FROM_NAT, addr);
	ECM_IP_ADDR_TO_SFE_IPV6_ADDR(create->conn_rule.flow_ip_xlate, addr);

	/*
	 * The destination address is what the destination IP is translated to as it is forwarded to the next interface.
	 * For egress this would yield the normal wan host and for ingress this would correctly NAT back to the LAN host
	 */
	ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_TO, addr);
	ECM_IP_ADDR_TO_SFE_IPV6_ADDR(create->conn_rule.return_ip_xlate, addr);

	/*
	 * Same approach as above for port information
	 */
	create->tuple.flow_ident = htons(ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_FROM));
	create->tuple.return_ident = htons(ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_TO_NAT));
	create->conn_rule.flow_ident_xlate = htons(ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_FROM_NAT));
	create->conn_rule.return_ident_xlate = htons(ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_TO));

	/*
	 * Get mac addresses.
	 * The src_mac is the mac address of the node that established the connection.
	 * This will work whether the from_node is LAN (egress) or WAN (ingress).
	 */
	ecm_db_connection_node_address_get(feci->ci, ECM_DB_OBJ_DIR_FROM, (uint8_t *)create->conn_rule.flow_mac);

	/*
	 * Sync our creation command from the assigned classifiers to get specific additional creation rules.
	 * NOTE: These are called in ascending order of priority and so the last classifier (highest) shall
	 * override any preceding classifiers.
	 * This also gives the classifiers a chance to see that acceleration is being attempted.
	 */
	assignment_count = ecm_db_connection_classifier_assignments_get_and_ref(feci->ci, assignments);
	for (aci_index = 0; aci_index < assignment_count; ++aci_index) {
		struct ecm_classifier_instance *aci;
		struct ecm_classifier_rule_create ecrc;
		/*
		 * NOTE: The current classifiers do not sync anything to the underlying accel engines.
		 * In the future, if any of the classifiers wants to pass any parameter, these parameters
		 * should be received via this object and copied to the accel engine's create object (nircm).
		*/
		aci = assignments[aci_index];
		DEBUG_TRACE("%px: sync from: %px, type: %d\n", feci, aci, aci->type_get(aci));
		aci->sync_from_v6(aci, &ecrc);
	}
	ecm_db_connection_assignments_release(assignment_count, assignments);

	/*
	 * Release the interface lists
	 */
	ecm_db_multicast_connection_to_interfaces_deref_all(to_ifaces, to_ifaces_first);

	for (vif = 0; vif < valid_vif_idx ; vif++){
		DEBUG_TRACE("%px: Multicast Accelerate connection %px\n"
			"Vif: %d\n"
			"Protocol: %d\n"
			"to_mtu: %u\n"
			"from_ip: " ECM_IP_ADDR_OCTAL_FMT ":%d\n"
			"to_ip: " ECM_IP_ADDR_OCTAL_FMT ":%d\n"
			"xlate_ip: " ECM_IP_ADDR_OCTAL_FMT ":%d\n"
			"to_mac: %pM\n"
			"dest_iface_num: %u\n"
			"in_vlan[0] %x\n"
			"in_vlan[1] %x\n"
			"out_vlan[0] %x\n"
			"out_vlan[1] %x\n",
			feci,
			feci->ci,
			vif,
			create->tuple.protocol,
			create->if_rule[vif].if_mtu,
			ECM_IP_ADDR_TO_OCTAL(create->tuple.flow_ip), create->tuple.flow_ident,
			ECM_IP_ADDR_TO_OCTAL(create->tuple.return_ip), create->tuple.return_ident,
			ECM_IP_ADDR_TO_OCTAL(create->if_rule[vif].xlate_src_ip), create->if_rule[vif].xlate_src_ident,
			create->if_rule[vif].if_mac,
			create->if_rule[vif].if_num,
			create->vlan_primary_rule.ingress_vlan_tag,
			create->vlan_secondary_rule.ingress_vlan_tag,
			create->if_rule[vif].egress_vlan_tag[0],
			create->if_rule[vif].egress_vlan_tag[1]);
	}

	/*
	 * Now that the rule has been constructed we re-compare the generation occurrance counter.
	 * If there has been a change then we abort because the rule may have been created using
	 * unstable data - especially if another thread has begun regeneration of the connection state.
	 * NOTE: This does not prevent a regen from being flagged immediately after this line of code either,
	 * or while the acceleration rule is in flight to the sfe.
	 * This is only to check for consistency of rule state - not that the state is stale.
	 * Remember that the connection is marked as "accel pending state" so if a regen is flagged immediately
	 * after this check passes, the connection will be decelerated and refreshed very quickly.
	 */
	if (regen_occurrances != ecm_db_connection_regeneration_occurrances_get(feci->ci)) {
		DEBUG_INFO("%px: connection:%px regen occurred - aborting accel rule.\n", feci, feci->ci);
		ecm_sfe_ipv6_accel_pending_clear(feci, ECM_FRONT_END_ACCELERATION_MODE_DECEL);
		kfree(nim);
		return;
	}

	/*
	 * Ref the connection before issuing an SFE rule
	 * This ensures that when the SFE responds to the command - which may even be immediately -
	 * the callback function can trust the correct ref was taken for its purpose.
	 * NOTE: remember that this will also implicitly hold the feci.
	 */
	ecm_db_connection_ref(feci->ci);

	/*
	 * We are about to issue the command, record the time of transmission
	 */
	spin_lock_bh(&feci->lock);
	feci->stats.cmd_time_begun = jiffies;
	spin_unlock_bh(&feci->lock);

	/*
	 * Call the rule create function
	 */
	sfe_tx_status = sfe_ipv6_tx(ecm_sfe_ipv6_mgr, nim);
	if (sfe_tx_status == SFE_TX_SUCCESS) {
		DEBUG_INFO("%px: Accel Tx Success\n", feci);
		/*
		 * Reset the driver_fail count - transmission was okay here.
		 */
		spin_lock_bh(&feci->lock);
		feci->stats.driver_fail = 0;	/* Reset */
		spin_unlock_bh(&feci->lock);
		kfree(nim);
		return;
	}

	DEBUG_INFO("%px: Accel Tx Failed\n", feci);
	/*
	 * Release that ref!
	 */
	ecm_db_connection_deref(feci->ci);

	/*
	 * TX failed
	 */
	spin_lock_bh(&feci->lock);
	DEBUG_ASSERT(feci->accel_mode == ECM_FRONT_END_ACCELERATION_MODE_ACCEL_PENDING, "%px: accel mode unexpected: %d\n", feci, feci->accel_mode);
	feci->stats.driver_fail_total++;
	feci->stats.driver_fail++;
	if (feci->stats.driver_fail >= feci->stats.driver_fail_limit) {
		DEBUG_WARN("%px: Accel failed - driver fail limit\n", feci);
		result_mode = ECM_FRONT_END_ACCELERATION_MODE_FAIL_DRIVER;
	} else {
		result_mode = ECM_FRONT_END_ACCELERATION_MODE_DECEL;
	}

	spin_lock_bh(&ecm_sfe_ipv6_lock);
	_ecm_sfe_ipv6_accel_pending_clear(feci, result_mode);
	spin_unlock_bh(&ecm_sfe_ipv6_lock);

	spin_unlock_bh(&feci->lock);
	kfree(nim);
	return;
}

/*
 * ecm_sfe_multicast_ipv6_connection_destroy_callback()
 *	Callback for handling destroy ack/nack calls.
 */
static void ecm_sfe_multicast_ipv6_connection_destroy_callback(void *app_data, struct sfe_ipv6_msg *nim)
{
	struct sfe_ipv6_rule_destroy_msg *nirdm = &nim->msg.rule_destroy;
	uint32_t serial = (uint32_t)(ecm_ptr_t)app_data;
	struct ecm_db_connection_instance *ci;
	struct ecm_front_end_connection_instance *feci;
	ip_addr_t flow_ip;
	ip_addr_t return_ip;

	/*
	 * Is this a response to a destroy message?
	 */
	if (nim->cm.type != SFE_TX_DESTROY_MULTICAST_RULE_MSG) {
		DEBUG_ERROR("%px: multicast destroy callback with improper type: %d\n", nim, nim->cm.type);
		return;
	}

	/*
	 * Look up ecm connection so that we can update the status.
	 */
	ci = ecm_db_connection_serial_find_and_ref(serial);
	if (!ci) {
		DEBUG_TRACE("%px: destroy callback, connection not found, serial: %u\n", nim, serial);
		return;
	}

	/*
	 * Release ref held for this ack/nack response.
	 * NOTE: It's okay to do this here, ci won't go away, because the ci is held as
	 * a result of the ecm_db_connection_serial_find_and_ref()
	 */
	ecm_db_connection_deref(ci);

	/*
	 * Get the front end instance
	 */
	feci = ecm_db_connection_front_end_get_and_ref(ci);
	DEBUG_CHECK_MAGIC(feci, ECM_FRONT_END_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", feci);

	ECM_SFE_IPV6_ADDR_TO_IP_ADDR(flow_ip, nirdm->tuple.flow_ip);
	ECM_SFE_IPV6_ADDR_TO_IP_ADDR(return_ip, nirdm->tuple.return_ip);

	/*
	 * Record command duration
	 */
	ecm_sfe_ipv6_decel_done_time_update(feci);

	/*
	 * Dump some useful trace information.
	 */
	DEBUG_TRACE("%px: decelerate response for connection: %px\n", feci, feci->ci);
	DEBUG_TRACE("%px: flow_ip: " ECM_IP_ADDR_OCTAL_FMT ":%d\n", feci, ECM_IP_ADDR_TO_OCTAL(flow_ip), nirdm->tuple.flow_ident);
	DEBUG_TRACE("%px: return_ip: " ECM_IP_ADDR_OCTAL_FMT ":%d\n", feci, ECM_IP_ADDR_TO_OCTAL(return_ip), nirdm->tuple.return_ident);
	DEBUG_TRACE("%px: protocol: %d\n", feci, nirdm->tuple.protocol);

	/*
	 * Drop decel pending counter
	 */
	spin_lock_bh(&ecm_sfe_ipv6_lock);
	ecm_sfe_ipv6_pending_decel_count--;
	DEBUG_ASSERT(ecm_sfe_ipv6_pending_decel_count >= 0, "Bad decel pending counter\n");
	spin_unlock_bh(&ecm_sfe_ipv6_lock);

	spin_lock_bh(&feci->lock);

	/*
	 * If decel is not still pending then it's possible that the SFE ended acceleration by some other reason e.g. flush
	 * In which case we cannot rely on the response we get here.
	 */
	if (feci->accel_mode != ECM_FRONT_END_ACCELERATION_MODE_DECEL_PENDING) {
		spin_unlock_bh(&feci->lock);

		/*
		 * Release the connections.
		 */
		ecm_front_end_connection_deref(feci);
		ecm_db_connection_deref(ci);
		return;
	}

	DEBUG_TRACE("%px: response: %d\n", feci, nim->cm.response);
	if (nim->cm.response != SFE_CMN_RESPONSE_ACK) {
		feci->accel_mode = ECM_FRONT_END_ACCELERATION_MODE_FAIL_DECEL;
	} else {
		feci->accel_mode = ECM_FRONT_END_ACCELERATION_MODE_DECEL;
	}

	/*
	 * If connection became defunct then set mode so that no further accel/decel attempts occur.
	 */
	if (feci->is_defunct) {
		feci->accel_mode = ECM_FRONT_END_ACCELERATION_MODE_FAIL_DEFUNCT_SHORT;
	}

	spin_unlock_bh(&feci->lock);

	/*
	 * Multicast acceleration ends
	 */
	spin_lock_bh(&ecm_sfe_ipv6_lock);
	ecm_sfe_multicast_ipv6_accelerated_count--;	/* Protocol specific counter */
	DEBUG_ASSERT(ecm_sfe_multicast_ipv6_accelerated_count >= 0, "Bad udp accel counter\n");
	ecm_sfe_ipv6_accelerated_count--;		/* General running counter */
	DEBUG_ASSERT(ecm_sfe_ipv6_accelerated_count >= 0, "Bad accel counter\n");
	spin_unlock_bh(&ecm_sfe_ipv6_lock);

	/*
	 * Release the connections.
	 */
	ecm_front_end_connection_deref(feci);
	ecm_db_connection_deref(ci);
}

/*
 * ecm_sfe_multicast_ipv6_connection_decelerate_msg_send()
 *	Prepares and sends a decelerate message to acceleration engine.
 */
static bool ecm_sfe_multicast_ipv6_connection_decelerate_msg_send(struct ecm_front_end_connection_instance *feci)
{
	struct sfe_ipv6_msg *nim;
	struct sfe_ipv6_rule_destroy_msg *nirdm;
	ip_addr_t src_ip;
	ip_addr_t dest_ip;
	sfe_tx_status_t sfe_tx_status;
	bool ret;

	DEBUG_CHECK_MAGIC(feci, ECM_FRONT_END_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", feci);

	nim = (struct sfe_ipv6_msg *)kzalloc(sizeof(struct sfe_ipv6_msg), GFP_ATOMIC | __GFP_NOWARN);
	if (!nim) {
		return false;
	}

	/*
	 * Increment the decel pending counter
	 */
	spin_lock_bh(&ecm_sfe_ipv6_lock);
	ecm_sfe_ipv6_pending_decel_count++;
	spin_unlock_bh(&ecm_sfe_ipv6_lock);

	/*
	 * Prepare deceleration message
	 */
	sfe_ipv6_msg_init(nim, SFE_SPECIAL_INTERFACE_IPV6, SFE_TX_DESTROY_MULTICAST_RULE_MSG,
			sizeof(struct sfe_ipv6_rule_destroy_msg),
			ecm_sfe_multicast_ipv6_connection_destroy_callback,
			(void *)(ecm_ptr_t)ecm_db_connection_serial_get(feci->ci));

	nirdm = &nim->msg.rule_destroy;
	nirdm->tuple.protocol = (int32_t)ecm_db_connection_protocol_get(feci->ci);

	/*
	 * Get addressing information
	 */
	ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_FROM, src_ip);
	ECM_IP_ADDR_TO_SFE_IPV6_ADDR(nirdm->tuple.flow_ip, src_ip);
	ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_TO, dest_ip);
	ECM_IP_ADDR_TO_SFE_IPV6_ADDR(nirdm->tuple.return_ip, dest_ip);
	nirdm->tuple.flow_ident = htons(ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_FROM));
	nirdm->tuple.return_ident = htons(ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_TO));

	DEBUG_INFO("%px: Mcast Connection %px decelerate\n"
			"src_ip: " ECM_IP_ADDR_OCTAL_FMT ":%d\n"
			"dest_ip: " ECM_IP_ADDR_OCTAL_FMT ":%d\n",
			feci, feci->ci,
			ECM_IP_ADDR_TO_OCTAL(src_ip), nirdm->tuple.flow_ident,
			ECM_IP_ADDR_TO_OCTAL(dest_ip), nirdm->tuple.return_ident);

	/*
	 * Right place to free the multicast destination interfaces list.
	 */
	ecm_db_multicast_connection_to_interfaces_clear(feci->ci);

	/*
	 * Take a ref to the feci->ci so that it will persist until we get a response from the SFE.
	 * NOTE: This will implicitly hold the feci too.
	 */
	ecm_db_connection_ref(feci->ci);

	/*
	 * We are about to issue the command, record the time of transmission
	 */
	spin_lock_bh(&feci->lock);
	feci->stats.cmd_time_begun = jiffies;
	spin_unlock_bh(&feci->lock);

	/*
	 * Destroy the SFE connection cache entry.
	 */
	sfe_tx_status = sfe_ipv6_tx(ecm_sfe_ipv6_mgr, nim);
	if (sfe_tx_status == SFE_TX_SUCCESS) {
		/*
		 * Reset the driver_fail count - transmission was okay here.
		 */
		spin_lock_bh(&feci->lock);
		feci->stats.driver_fail = 0;
		spin_unlock_bh(&feci->lock);
		kfree(nim);
		return true;
	}

	kfree(nim);

	/*
	 * TX failed
	 */
	ret = ecm_front_end_destroy_failure_handle(feci);

	/*
	 * Release the ref take, SFE driver did not accept our command.
	 */
	ecm_db_connection_deref(feci->ci);

	/*
	 * Could not send the request, decrement the decel pending counter
	 */
	spin_lock_bh(&ecm_sfe_ipv6_lock);
	ecm_sfe_ipv6_pending_decel_count--;
	DEBUG_ASSERT(ecm_sfe_ipv6_pending_decel_count >= 0, "Bad decel pending counter\n");
	spin_unlock_bh(&ecm_sfe_ipv6_lock);

	return ret;
}

/*
 * ecm_sfe_multicast_ipv6_connection_decelerate()
 *     Decelerate a connection
 */
static bool ecm_sfe_multicast_ipv6_connection_decelerate(struct ecm_front_end_connection_instance *feci)
{
	/*
	 * Check if accel mode is OK for the deceleration.
	 */
	spin_lock_bh(&feci->lock);
	if (!ecm_front_end_common_connection_decelerate_accel_mode_check(feci)) {
		spin_unlock_bh(&feci->lock);
		return false;
	}
	feci->accel_mode = ECM_FRONT_END_ACCELERATION_MODE_DECEL_PENDING;
	spin_unlock_bh(&feci->lock);

	return ecm_sfe_multicast_ipv6_connection_decelerate_msg_send(feci);
}

/*
 * ecm_sfe_multicast_ipv6_connection_defunct_callback()
 *	Callback to be called when a Mcast connection has become defunct.
 */
bool ecm_sfe_multicast_ipv6_connection_defunct_callback(void *arg, int *accel_mode)
{
	bool ret;
	struct ecm_front_end_connection_instance *feci = (struct ecm_front_end_connection_instance *)arg;

	DEBUG_CHECK_MAGIC(feci, ECM_FRONT_END_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", feci);

	spin_lock_bh(&feci->lock);
	/*
	 * Check if the connection can be defuncted.
	 */
	if (!ecm_front_end_common_connection_defunct_check(feci)) {
		*accel_mode = feci->accel_mode;
		spin_unlock_bh(&feci->lock);
		return false;
	}

	/*
	 * If none of the cases matched above, this means the connection is in the
	 * accel mode, so we force a deceleration.
	 * NOTE: If the mode is accel pending then the decel will be actioned when that is completed.
	 */
	if (!ecm_front_end_common_connection_decelerate_accel_mode_check(feci)) {
		*accel_mode = feci->accel_mode;
		spin_unlock_bh(&feci->lock);
		return false;
	}
	feci->accel_mode = ECM_FRONT_END_ACCELERATION_MODE_DECEL_PENDING;
	spin_unlock_bh(&feci->lock);

	ret = ecm_sfe_multicast_ipv6_connection_decelerate_msg_send(feci);

	/*
	 * Copy the accel_mode which is returned from the decelerate message function. This value
	 * will be used in the caller to decide releasing the final reference of the connection.
	 * But if this function reaches to here, the caller care about the ret. If ret is true,
	 * the reference will be released regardless of the accel_mode. If ret is false, accel_mode
	 * will be in the ACCEL state (for destroy re-try) and this state will not be used in the
	 * caller's decision. It looks for ACCEL_FAIL states.
	 */
	spin_lock_bh(&feci->lock);
	*accel_mode = feci->accel_mode;
	spin_unlock_bh(&feci->lock);

	return ret;
}

/*
 * ecm_sfe_multicast_ipv6_connection_accel_ceased()
 *	SFE has indicated that acceleration has stopped.
 *
 * NOTE: This is called in response to an SFE self-initiated termination of acceleration.
 * This must NOT be called because the ECM terminated the acceleration.
 */
static void ecm_sfe_multicast_ipv6_connection_accel_ceased(struct ecm_front_end_connection_instance *feci)
{
	DEBUG_CHECK_MAGIC(feci, ECM_FRONT_END_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", feci);

	DEBUG_INFO("%px: accel ceased\n", feci);

	spin_lock_bh(&feci->lock);

	/*
	 * If we are in accel-pending state then the SFE has issued a flush out-of-order
	 * with the ACK/NACK we are actually waiting for.
	 * To work around this we record a "flush has already happened" and will action it when we finally get that ACK/NACK.
	 * GGG TODO This should eventually be removed when the SFE honours messaging sequence.
	 */
	if (feci->accel_mode == ECM_FRONT_END_ACCELERATION_MODE_ACCEL_PENDING) {
		feci->stats.flush_happened = true;
		feci->stats.flush_happened_total++;
		spin_unlock_bh(&feci->lock);
		return;
	}

	/*
	 * If connection is no longer accelerated by the time we get here just ignore the command
	 */
	if (feci->accel_mode != ECM_FRONT_END_ACCELERATION_MODE_ACCEL) {
		spin_unlock_bh(&feci->lock);
		return;
	}

	/*
	 * If the no_action_seen counter was not reset then acceleration ended without any offload action
	 */
	if (feci->stats.no_action_seen) {
		feci->stats.no_action_seen_total++;
	}

	/*
	 * If the no_action_seen indicates successive cessations of acceleration without any offload action occuring
	 * then we fail out this connection
	 */
	if (feci->stats.no_action_seen >= feci->stats.no_action_seen_limit) {
		feci->accel_mode = ECM_FRONT_END_ACCELERATION_MODE_FAIL_NO_ACTION;
	} else {
		feci->accel_mode = ECM_FRONT_END_ACCELERATION_MODE_DECEL;
	}
	spin_unlock_bh(&feci->lock);

	/*
	 * Mcast acceleration ends
	 */
	spin_lock_bh(&ecm_sfe_ipv6_lock);
	ecm_sfe_multicast_ipv6_accelerated_count--;	/* Protocol specific counter */
	DEBUG_ASSERT(ecm_sfe_multicast_ipv6_accelerated_count >= 0, "Bad Mcast accel counter\n");
	ecm_sfe_ipv6_accelerated_count--;		/* General running counter */
	DEBUG_ASSERT(ecm_sfe_ipv6_accelerated_count >= 0, "Bad accel counter\n");
	spin_unlock_bh(&ecm_sfe_ipv6_lock);
}

#ifdef ECM_STATE_OUTPUT_ENABLE
/*
 * ecm_sfe_multicast_ipv6_connection_state_get()
 *	Return state of this multicast front end instance
 */
static int ecm_sfe_multicast_ipv6_connection_state_get(struct ecm_front_end_connection_instance *feci, struct ecm_state_file_instance *sfi)
{
	DEBUG_CHECK_MAGIC(feci, ECM_FRONT_END_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", feci);

	return ecm_front_end_common_connection_state_get(feci, sfi, "sfe_v6.multicast");
}
#endif

/*
 * ecm_sfe_multicast_ipv6_bridge_update_connections()
 * 	Update SFE with new multicast egress ports.
 */
static void ecm_sfe_multicast_ipv6_bridge_update_connections(ip_addr_t dest_ip, struct net_device *brdev)
{
	struct ecm_front_end_connection_instance *feci;
	struct ecm_db_multicast_tuple_instance *ti;
	struct ecm_db_multicast_tuple_instance *ti_next;
	struct ecm_multicast_if_update mc_sync;
	struct ecm_db_connection_instance *ci;
	struct in6_addr group6;
	struct in6_addr origin6;
	ip_addr_t grp_ip;
	ip_addr_t src_ip;
	int i, ret;
	int32_t if_num;
	uint32_t mc_dst_dev[ECM_DB_MULTICAST_IF_MAX];
	bool mc_update;
	bool is_routed;
	struct net_device *l2_br_dev, *l3_br_dev;

	ECM_IP_ADDR_TO_NIN6_ADDR(group6, dest_ip);
	ti = ecm_db_multicast_connection_get_and_ref_first(dest_ip);
	if (!ti) {
		DEBUG_WARN("no multicast tuple entry found. Group IP: " ECM_IP_ADDR_OCTAL_FMT , ECM_IP_ADDR_TO_OCTAL(dest_ip));
		return;
	}

	while (ti) {
		struct ecm_classifier_process_response aci_pr;

		memset(&aci_pr, 0, sizeof(aci_pr));

		/*
		 * We now have a 5-tuple which has been accelerated. Query the MCS bridge to receive a list
		 * of interfaces left or joined a group for a source.
		 */
		memset(mc_dst_dev, 0, sizeof(mc_dst_dev));

		/*
		 * Get the group IP address stored in tuple_instance and match this with
		 * the group IP received from MCS update callback.
		 */
		ecm_db_multicast_tuple_instance_group_ip_get(ti, grp_ip);
		if (!ECM_IP_ADDR_MATCH(grp_ip, dest_ip)) {
			goto find_next_tuple;
		}

		/*
		 * Get the source IP address for this entry for the group
		 */
		ecm_db_multicast_tuple_instance_source_ip_get(ti, src_ip);
		ECM_IP_ADDR_TO_NIN6_ADDR(origin6, src_ip);

		/*
		 * Query bridge snooper for the destination list when given the group and source
		 * if, 	if_num < 0   mc_bridge_ipv6_get_if has encountered with some error, check for next tuple.
		 * 	if_num == 0  All slaves have left the group. Deacel the flow.
		 * 	if_num > 0   An interface leave/Join the group. Process the leave/join interface request.
		 */
		if_num = mc_bridge_ipv6_get_if (brdev, &origin6, &group6, ECM_DB_MULTICAST_IF_MAX, mc_dst_dev);
		if (if_num < 0) {
			/*
			 * This may a valid case when all the interface has left a multicast group.
			 * In this case the MCS will return if_num 0, But we may have an oudated
			 * interface in multicast interface heirarchy list. At next step we have to
			 * check whether the DB instance is present or not.
			 */
			DEBUG_TRACE("No valid bridge slaves for the group/source\n");
			goto find_next_tuple;
		}

		/*
		 * Get a DB connection instance for the 5-tuple
		 */
		ci = ecm_db_multicast_connection_get_from_tuple(ti);

		/*
		 * The source interface could have joined the group as well.
		 * In such cases, the destination interface list returned by
		 * the snooper would include the source interface as well.
		 * We need to filter the source interface from the list in such cases.
		 */
		if (if_num > 0) {
			if_num = ecm_interface_multicast_filter_src_interface(ci, mc_dst_dev);
			if (if_num == ECM_DB_IFACE_HEIRARCHY_MAX) {
				DEBUG_WARN("%px: MCS Snooper Update: no interfaces in from_interfaces list!\n", ci);
				goto find_next_tuple;
			}
		}

		feci = ecm_db_connection_front_end_get_and_ref(ci);

		/*
		 * All bridge slaves has left the group. If flow is pure bridge, Deacel the connection and return
		 * If flow is routed, let MFC callback handle this.
		 *
		 * If there are no routed interfaces, then decelerate. Else
		 * we first send an update message to the firmware for the
		 * interface that have left, before issuing a decelerate
		 * at a later point via the MFC callback. This is because
		 * there might be a few seconds delay before MFC issues
		 * the delete callback
		 */
		is_routed = ecm_db_connection_is_routed_get(ci);
		if (!is_routed) {
			/*
			 * L2-only multicast: Update the flow only if the flow's bridge device matches the bridge device passed by MCS.
			 */
			if (!if_num) {
				/*
				 * Decelerate the flow since there is no active ports left
				 */
				feci->decelerate(feci);
				ecm_front_end_connection_deref(feci);
				goto find_next_tuple;
			}

			/*
			 * Decelerate the flow if the bridge device from the MCS update does not match the bridge with which flow was created.
			 */
			l2_br_dev = ecm_db_multicast_tuple_instance_get_l2_br_dev(ti);
			if (!l2_br_dev) {
				DEBUG_WARN("Not found a valid l2_br_dev in ti for bridged mc flow");
				ecm_front_end_connection_deref(feci);
				goto find_next_tuple;
			}

			if (l2_br_dev != brdev) {
				DEBUG_WARN("L2 bridge device does not match the MCS update. l2_br_dev:%s brdev:%s", l2_br_dev->name, brdev->name);
				ecm_front_end_connection_deref(feci);
				goto find_next_tuple;
			}
		} else {
			/*
			 * Check whether the bridge for which we are processing an update,
			 * is part of the the routed destination interface list for the flow.
			 * The flow could be one of the below
			 *
			 * a. WAN (upstream) <-> br-lan (downstream)
			 * b. WAN (upstream) <-> br-lan1(downstream), br-lan2 (downstream)
			 */
			int i;
			struct in6_addr ip_src;
			struct in6_addr ip_grp;
			uint32_t dst_if_cnt;
			uint32_t dst_dev[ECM_DB_MULTICAST_IF_MAX];

			/*
			 * If l3_br_dev is NULL then the we only need to check the ipmr destination list.
			 */
			l3_br_dev = ecm_db_multicast_tuple_instance_get_l3_br_dev(ti);
			if (!l3_br_dev) {
				goto process_ipmr_entry;
			}

			/*
			 * 'brdev' is already part of the multicast interface list, no need to check ipmr entry.
			 */
			if (l3_br_dev == brdev) {
				goto process_packet;
			}

process_ipmr_entry:
			memset(dst_dev, 0, sizeof(dst_dev));
			ECM_IP_ADDR_TO_NIN6_ADDR(ip_src, src_ip);
			ECM_IP_ADDR_TO_NIN6_ADDR(ip_grp, grp_ip);
			dst_if_cnt =  ip6mr_find_mfc_entry(&init_net, &ip_src, &ip_grp, ECM_DB_MULTICAST_IF_MAX, dst_dev);
			if (dst_if_cnt < 0) {
				/*
				 * Decelerate the flow since there is no active ports left
				 */
				DEBUG_WARN("Not found a valid vif count %d\n", dst_if_cnt);
				feci->decelerate(feci);
				ecm_front_end_connection_deref(feci);
				goto find_next_tuple;
			}

			/*
			 * Update should be allowed for the connection only if 'brdev' is part of ipmr destination interface list.
			 */
			for (i = 0; i < dst_if_cnt; i++) {
				if (dst_dev[i] == brdev->ifindex)
					goto process_packet;
			}

			DEBUG_WARN("brdev: %s is neither part of mcproxy configuration nor same as ingress bridge port device.\n", brdev->name);
			ecm_front_end_connection_deref(feci);
			goto find_next_tuple;
		}

process_packet:
		/*
		 * Find out changes to the destination interfaces heirarchy
		 * of the connection. We try to find out the interfaces that
		 * have joined new, and the existing interfaces in the list
		 * that have left seperately.
		 */
		memset(&mc_sync, 0, sizeof(mc_sync));
		spin_lock_bh(&ecm_sfe_ipv6_lock);
		mc_update = ecm_interface_multicast_find_updates_to_iface_list(ci, &mc_sync, 0, true, mc_dst_dev, if_num, brdev);
		spin_unlock_bh(&ecm_sfe_ipv6_lock);
		if (!mc_update) {
			/*
			 * No updates to this multicast flow. Move on to the next
			 * flow for the same group
			 */
			ecm_front_end_connection_deref(feci);
			goto find_next_tuple;
		}

		DEBUG_TRACE("BRIDGE UPDATE callback ===> leave_cnt %d, join_cnt %d\n", mc_sync.if_leave_cnt, mc_sync.if_join_cnt);

		/*
		 * Do we have any new interfaces that have joined?
		 */
		if (mc_sync.if_join_cnt) {
			struct ecm_db_iface_instance *to_list;
			uint8_t src_node_addr[ETH_ALEN];
			int32_t if_cnt, to_list_first[ECM_DB_MULTICAST_IF_MAX];
			uint32_t tuple_instance_flags;

			to_list = (struct ecm_db_iface_instance *)kzalloc(ECM_DB_TO_MCAST_INTERFACES_SIZE, GFP_ATOMIC | __GFP_NOWARN);
			if (!to_list) {
				ecm_front_end_connection_deref(feci);
				goto find_next_tuple;
			}

			/*
			 * Initialize the heirarchy's indices for the 'to_list'
			 * which will hold the interface heirarchies for the new joinees
			 */
			for (i = 0; i < ECM_DB_MULTICAST_IF_MAX; i++) {
				to_list_first[i] = ECM_DB_IFACE_HEIRARCHY_MAX;
			}

			ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_FROM, src_node_addr);

			/*
			 * Create the interface heirarchy list for the new interfaces. We append this list later to
			 * the existing list of destination interfaces.
			 */
			if_cnt = ecm_interface_multicast_heirarchy_construct_bridged(feci, to_list, brdev, src_ip, dest_ip, mc_sync.if_join_cnt, mc_sync.join_dev, to_list_first, src_node_addr, NULL, NULL);
			if (!if_cnt) {
				DEBUG_WARN("Failed to obtain 'to_mcast_update' heirarchy list\n");
				feci->decelerate(feci);
				ecm_front_end_connection_deref(feci);
				kfree(to_list);
				goto find_next_tuple;
			}

			/*
			 * Append the interface heirarchy array of the new joinees to the existing destination list
			 */
			ecm_db_multicast_connection_to_interfaces_update(ci, to_list, to_list_first, mc_sync.if_join_idx);

			/*
			 * In Routed + Bridge mode, if there is a group leave request arrives for the last
			 * slave of the bridge then MFC will clear ECM_DB_MULTICAST_CONNECTION_BRIDGE_DEV_SET_FLAG
			 * in tuple_instance. If the bridge slave joins again then we need to set the flag again
			 * in tuple_instance here.
			 */
			tuple_instance_flags = ecm_db_multicast_tuple_instance_flags_get(ti);
			if (is_routed && !(tuple_instance_flags & ECM_DB_MULTICAST_CONNECTION_BRIDGE_DEV_SET_FLAG)) {
				ecm_db_multicast_tuple_instance_flags_set(ti, ECM_DB_MULTICAST_CONNECTION_BRIDGE_DEV_SET_FLAG);
			}

			/*
			 * De-ref the updated destination interface list
			 */
			for (i = 0; i < ECM_DB_MULTICAST_IF_MAX; i++) {
				if (mc_sync.if_join_idx[i]) {
					struct ecm_db_iface_instance *to_list_single;
					struct ecm_db_iface_instance *to_list_temp[ECM_DB_IFACE_HEIRARCHY_MAX];

					to_list_single = ecm_db_multicast_if_heirarchy_get(to_list, i);
					ecm_db_multicast_copy_if_heirarchy(to_list_temp, to_list_single);
					ecm_db_connection_interfaces_deref(to_list_temp, to_list_first[i]);
				}
			}
			kfree(to_list);
		} else if (mc_sync.if_leave_cnt) {
			/*
			 * If these are the last interface set leaving the to interface
			 * list of the connection, then decelerate the connection
			 */
			int mc_to_interface_count = ecm_db_multicast_connection_to_interfaces_get_count(ci);

			if (mc_sync.if_leave_cnt == mc_to_interface_count) {
				feci->decelerate(feci);
				ecm_front_end_connection_deref(feci);
				DEBUG_INFO("%px: Decelerating the flow as there are no to interfaces in the multicast group: " ECM_IP_ADDR_OCTAL_FMT , feci, ECM_IP_ADDR_TO_OCTAL(dest_ip));
				goto find_next_tuple;
			}
		}

#ifdef ECM_CLASSIFIER_OVS_ENABLE
		/*
		 * Verify the 'to' interface list with OVS classifier.
		 */
		if (ecm_front_end_is_ovs_bridge_device(brdev) &&
			ecm_db_multicast_ovs_verify_to_list(ci, &aci_pr)) {
			/*
			 * We defunct the flow when the OVS returns "DENY_ACCEL" for the port.
			 * This can happen when the first port has joined on an OVS bridge
			 * on a flow that already exists. In this case, OVS needs to see the
			 * packet to update the flow. So we defunct the existing rule.
			 */
			DEBUG_WARN("%px: Verification of the ovs 'to_list' has failed. Hence, defunct the connection: %px\n", feci, feci->ci);
			ecm_db_connection_make_defunct(ci);
			ecm_front_end_connection_deref(feci);
			goto find_next_tuple;
		}
#endif
		/*
		 * Push the updates to SFE
		 */
		DEBUG_TRACE("%px: Update accel\n", ci);
		if ((feci->accel_mode <= ECM_FRONT_END_ACCELERATION_MODE_FAIL_DENIED) ||
				(feci->accel_mode != ECM_FRONT_END_ACCELERATION_MODE_ACCEL)) {
			DEBUG_TRACE("%px: Ignoring wrong mode accel for conn: %px\n", feci, feci->ci);
			ecm_front_end_connection_deref(feci);
			goto find_next_tuple;
		}

		ret = ecm_sfe_multicast_ipv6_connection_update_accelerate(feci, &mc_sync, &aci_pr);
		if (ret < 0) {
			feci->decelerate(feci);
			ecm_front_end_connection_deref(feci);
			goto find_next_tuple;
		}

		ecm_front_end_connection_deref(feci);

		/*
		 * Release the interfaces that may have left the connection
		 */
		ecm_db_multicast_connection_to_interfaces_leave(ci, &mc_sync);

find_next_tuple:
		ti_next = ecm_db_multicast_connection_get_and_ref_next(ti);
		ecm_db_multicast_connection_deref(ti);
		ti = ti_next;
	}
}

/*
 * ecm_sfe_multicast_ipv6_connection_instance_alloc()
 *	Create a front end instance specific for Mcast connection
 */
struct ecm_front_end_connection_instance *ecm_sfe_multicast_ipv6_connection_instance_alloc(
								bool can_accel,
								struct ecm_db_connection_instance **nci)
{
	struct ecm_front_end_connection_instance *feci;
	struct ecm_db_connection_instance *ci;

	if (ecm_sfe_ipv6_is_conn_limit_reached()) {
		DEBUG_TRACE("Reached connection limit\n");
		return NULL;
	}

	/*
	 * Now allocate the new connection
	 */
	*nci = ecm_db_connection_alloc();
	if (!*nci) {
		DEBUG_WARN("Failed to allocate connection\n");
		return NULL;
	}

	ci = *nci;

	feci = (struct ecm_front_end_connection_instance *)kzalloc(sizeof(struct ecm_front_end_connection_instance), GFP_ATOMIC | __GFP_NOWARN);
	if (!feci) {
		DEBUG_WARN("Mcast Front end alloc failed\n");
		ecm_db_connection_deref(ci);
		return NULL;
	}

	/*
	 * Refs is 1 for the creator of the connection
	 */
	feci->refs = 1;
	DEBUG_SET_MAGIC(feci, ECM_FRONT_END_CONNECTION_INSTANCE_MAGIC);
	spin_lock_init(&feci->lock);

	feci->can_accel = can_accel;
	feci->accel_mode = (can_accel) ? ECM_FRONT_END_ACCELERATION_MODE_DECEL : ECM_FRONT_END_ACCELERATION_MODE_FAIL_DENIED;
	feci->accel_engine = ECM_FRONT_END_ENGINE_SFE;
	spin_lock_bh(&ecm_sfe_ipv6_lock);
	feci->stats.no_action_seen_limit = ecm_sfe_ipv6_no_action_limit_default;
	feci->stats.driver_fail_limit = ecm_sfe_ipv6_driver_fail_limit_default;
	feci->stats.ae_nack_limit = ecm_sfe_ipv6_nack_limit_default;
	spin_unlock_bh(&ecm_sfe_ipv6_lock);

	/*
	 * Copy reference to connection - no need to ref ci as ci maintains a ref to this instance instead (this instance persists for as long as ci does)
	 */
	feci->ci = ci;

	feci->ip_version = 6;

	/*
	 * Populate the methods and callbacks
	 */
	feci->accelerate = ecm_sfe_multicast_ipv6_connection_accelerate;
	feci->decelerate = ecm_sfe_multicast_ipv6_connection_decelerate;
	feci->accel_ceased = ecm_sfe_multicast_ipv6_connection_accel_ceased;
#ifdef ECM_STATE_OUTPUT_ENABLE
	feci->state_get = ecm_sfe_multicast_ipv6_connection_state_get;
#endif
	feci->ae_interface_number_by_dev_get = ecm_sfe_common_get_interface_number_by_dev;
	feci->ae_interface_number_by_dev_type_get = ecm_sfe_common_get_interface_number_by_dev_type;
	feci->ae_interface_type_get = ecm_sfe_common_get_interface_type;
	feci->regenerate = ecm_sfe_common_connection_regenerate;
	feci->multicast_update = ecm_sfe_multicast_ipv6_bridge_update_connections;
	feci->defunct = ecm_sfe_multicast_ipv6_connection_defunct_callback;

	feci->get_stats_bitmap = ecm_front_end_common_get_stats_bitmap;
	feci->set_stats_bitmap = ecm_front_end_common_set_stats_bitmap;

	return feci;
}

/*
 * ecm_sfe_multicast_ipv6_br_update_event_callback()
 *	Callback received from bridge multicast snooper module in the
 * following events:
 * a.) Updates to a muticast group due to IGMP JOIN/LEAVE.
 * brdev: bridge netdevice
 * group: IP multicast group address
 *
 * This function does the following:
 * a.) Lookup the tuple_instance table to find any flows through the bridge for the group
 * b.) Query the MCS bridge using the source and group address, to receive a list
 *     of bridge slaves for the flow
 * c.) Process the updates by matching the received bridge slave list with existing
 *     destination interfaces heirarchy for the CI, and decide whether to delete an
 *     exiting interface heirarchy or add a new heirarchy.
 */
static void ecm_sfe_multicast_ipv6_br_update_event_callback(struct net_device *brdev, struct in6_addr *group)
{
	ip_addr_t dest_ip;
	struct in6_addr group6;

	memcpy(&group6, group, sizeof(struct in6_addr));
	ECM_NIN6_ADDR_TO_IP_ADDR(dest_ip, group6);

	DEBUG_TRACE("IP dst: " ECM_IP_ADDR_OCTAL_FMT , ECM_IP_ADDR_TO_OCTAL(dest_ip));

	ecm_sfe_multicast_ipv6_bridge_update_connections(dest_ip, brdev);
}

/*
 * ecm_sfe_multicast_ipv6_mfc_update_event_callback()
 *	Callback called by Linux kernel multicast routing module in the
 * following events:
 * a.) Updates to destination interface lists for a multicast route due to
 * IGMP JOIN/LEAVE
 * b.) Multicast route delete event
 *
 * group: IP multicast group address
 * origin: IP source for the multicast route
 * to_dev_idx[]: Array of ifindex for destination interfaces for the route
 * max_to_dev: size of the array
 * op: UPDATE or DELETE event
 *
 * This function does the following:
 * a.) Lookup the tuple_instance table to find any accelerated flows for the group/source,
 *     and access their CI
 * b.) Process the updates by matching the received destination interface list with existing
 *     destination interfaces heirarchy for the CI, and decide whether to delete an
 *     exiting interface heirarchy or add a new heirarchy.
 */
static void ecm_sfe_multicast_ipv6_mfc_update_event_callback(struct in6_addr *group, struct in6_addr *origin, uint32_t max_to_dev, uint32_t to_dev_idx[], uint8_t op)
{
	struct ecm_db_connection_instance *ci;
	struct ecm_db_multicast_tuple_instance *tuple_instance;
	struct ecm_db_multicast_tuple_instance *tuple_instance_next;
	struct ecm_multicast_if_update mc_sync;
	struct in6_addr origin6;
	struct in6_addr group6;
	int32_t vif_cnt;
	ip_addr_t src_ip;
	ip_addr_t dest_ip;
	ip_addr_t src_addr;
	ip_addr_t grp_addr;

	memcpy(&origin6, origin, sizeof(ip_addr_t));
	memcpy(&group6, group, sizeof(ip_addr_t));

	ECM_NIN6_ADDR_TO_IP_ADDR(src_ip, origin6);
	ECM_NIN6_ADDR_TO_IP_ADDR(dest_ip, group6);

	DEBUG_TRACE("IP Packet src: " ECM_IP_ADDR_OCTAL_FMT "dst: " ECM_IP_ADDR_OCTAL_FMT ,
			                                ECM_IP_ADDR_TO_OCTAL(src_ip),ECM_IP_ADDR_TO_OCTAL(dest_ip));

	/*
	 * Access the 5-tuple information from the tuple_instance table, using the
	 * source and group addresses
	 */
	tuple_instance = ecm_db_multicast_connection_find_and_ref(src_ip, dest_ip);
	if (!tuple_instance) {
		DEBUG_TRACE("MFC_EVENT: Port info is not found\n");
		return;
	}

	switch (op) {
	case IP6MR_MFC_EVENT_UPDATE:
	{
		struct ecm_front_end_connection_instance *feci;
		struct ecm_db_iface_instance *to_list;
		struct ecm_db_iface_instance *to_list_single;
		struct ecm_db_iface_instance *to_list_temp[ECM_DB_IFACE_HEIRARCHY_MAX];
		uint32_t to_list_first[ECM_DB_MULTICAST_IF_MAX];
		uint32_t i, ret;
		uint32_t mc_flags = 0;
		uint32_t tuple_instance_flag = 0;
		bool br_present_in_dest_dev_list = false;
		bool mc_update = false;

		/*
		 * Find the connections belongs to the same source and group address and
		 * apply the updates received from event handler
		 */
		while (tuple_instance) {
			struct ecm_classifier_process_response aci_pr;

			memset(&aci_pr, 0, sizeof(aci_pr));
			/*
			 * Get the source/group IP address for this multicast tuple and match
			 * with the source and group IP received from the event handler. If
			 * there is no match found, jumping to the next multicast tuple.
			 */
			ecm_db_multicast_tuple_instance_source_ip_get(tuple_instance, src_addr);
			ecm_db_multicast_tuple_instance_group_ip_get(tuple_instance, grp_addr);
			if (!(ECM_IP_ADDR_MATCH(src_addr, src_ip) && ECM_IP_ADDR_MATCH(grp_addr, dest_ip))) {
				DEBUG_TRACE("%px: Multicast tuple not matched, try next multicast tuple %d\n", tuple_instance, op);
				tuple_instance_next = ecm_db_multicast_connection_get_and_ref_next(tuple_instance);
				ecm_db_multicast_connection_deref(tuple_instance);
				tuple_instance = tuple_instance_next;
				continue;
			}

			/*
			 * Get the DB connection instance using the tuple_instance, ref to 'ci'
			 * has been already taken by ecm_db_multicast_connection_find_and_ref()
			 */
			ci = ecm_db_multicast_connection_get_from_tuple(tuple_instance);
			DEBUG_TRACE("%px: update the multicast flow: %px\n", ci, tuple_instance);

			/*
			 * Check if this multicast connection has a bridge
			 * device in multicast destination interface list.
			 */
			tuple_instance_flag = ecm_db_multicast_tuple_instance_flags_get(tuple_instance);
			if (tuple_instance_flag & ECM_DB_MULTICAST_CONNECTION_BRIDGE_DEV_SET_FLAG) {
				mc_flags |= ECM_DB_MULTICAST_CONNECTION_BRIDGE_DEV_SET_FLAG;
			}

			memset(&mc_sync, 0, sizeof(mc_sync));
			spin_lock_bh(&ecm_sfe_ipv6_lock);

			/*
			 * Find out changes to the destination interfaces heirarchy
			 * of the connection. We try to find out the interfaces that
			 * have joined new, and the existing interfaces in the list
			 * that have left seperately.
			 */
			mc_update = ecm_interface_multicast_find_updates_to_iface_list(ci, &mc_sync, mc_flags, false, to_dev_idx, max_to_dev, NULL);
			if (!mc_update) {
				DEBUG_TRACE("%px: no update, check next multicast tuple: %px\n", ci, tuple_instance);
				spin_unlock_bh(&ecm_sfe_ipv6_lock);
				tuple_instance_next = ecm_db_multicast_connection_get_and_ref_next(tuple_instance);
				ecm_db_multicast_connection_deref(tuple_instance);
				tuple_instance = tuple_instance_next;
				continue;
			}

			br_present_in_dest_dev_list = ecm_interface_multicast_check_for_br_dev(to_dev_idx, max_to_dev);
			if (!br_present_in_dest_dev_list) {
				ecm_db_multicast_tuple_instance_flags_clear(tuple_instance, ECM_DB_MULTICAST_CONNECTION_BRIDGE_DEV_SET_FLAG);
			}

			spin_unlock_bh(&ecm_sfe_ipv6_lock);
			DEBUG_TRACE("%px: MFC update callback leave_cnt %d, join_cnt %d\n", ci, mc_sync.if_leave_cnt, mc_sync.if_join_cnt);

			feci = ecm_db_connection_front_end_get_and_ref(ci);

			/*
			 * Do we have any new interfaces that have joined?
			 */
			if (mc_sync.if_join_cnt > 0) {
				to_list = (struct ecm_db_iface_instance *)kzalloc(ECM_DB_TO_MCAST_INTERFACES_SIZE, GFP_ATOMIC | __GFP_NOWARN);
				if (!to_list) {
					ecm_front_end_connection_deref(feci);
					ecm_db_multicast_connection_deref(tuple_instance);
					return;
				}

				/*
				 * Initialize the heirarchy's indices for the 'to_list'
				 * which will hold the interface heirarchies for the new joinees
				 */
				for (i = 0; i < ECM_DB_MULTICAST_IF_MAX; i++) {
					to_list_first[i] = ECM_DB_IFACE_HEIRARCHY_MAX;
				}

				/*
				 * Create the interface heirarchy list for the new interfaces. We append this list later to
				 * the existing list of destination interfaces.
				 */
				vif_cnt = ecm_interface_multicast_heirarchy_construct_routed(feci, to_list, NULL, src_ip, dest_ip, mc_sync.if_join_cnt, mc_sync.join_dev, to_list_first, true, NULL, NULL);
				if (vif_cnt == 0) {
					DEBUG_WARN("Failed to obtain 'to_mcast_update' heirarchy list\n");
					feci->decelerate(feci);
					ecm_front_end_connection_deref(feci);
					ecm_db_multicast_connection_deref(tuple_instance);
					kfree(to_list);
					return;
				}

				/*
				 * Append the interface heirarchy array of the new joinees to the existing destination list
				 */
				ecm_db_multicast_connection_to_interfaces_update(ci, to_list, to_list_first, mc_sync.if_join_idx);

				/*
				 * De-ref the updated destination interface list
				 */
				for (i = 0; i < ECM_DB_MULTICAST_IF_MAX; i++) {
					if (mc_sync.if_join_idx[i]) {
						to_list_single = ecm_db_multicast_if_heirarchy_get(to_list, i);
						ecm_db_multicast_copy_if_heirarchy(to_list_temp, to_list_single);
						ecm_db_connection_interfaces_deref(to_list_temp, to_list_first[i]);
					}
				}
				kfree(to_list);
			}

			if (br_present_in_dest_dev_list && !(tuple_instance_flag & ECM_DB_MULTICAST_CONNECTION_BRIDGE_DEV_SET_FLAG)) {
				ecm_db_multicast_tuple_instance_flags_set(tuple_instance, ECM_DB_MULTICAST_CONNECTION_BRIDGE_DEV_SET_FLAG);
			}

#ifdef ECM_CLASSIFIER_OVS_ENABLE
			/*
			 * Verify the 'to' interface list with OVS classifier.
			 */
			if (ecm_interface_multicast_check_for_ovs_br_dev(to_dev_idx, max_to_dev) &&
				ecm_db_multicast_ovs_verify_to_list(ci, &aci_pr)) {
				/*
				 * We defunct the flow when the OVS returns "DENY_ACCEL" for the port.
				 * This can happen when the first port has joined on an OVS bridge
				 * on a flow that already exists. In this case, OVS needs to see the
				 * packet to update the flow. So we defunct the existing rule.
				 */
				DEBUG_TRACE("%px: Verification of the ovs 'to_list' has failed. Hence, defunct the connection: %px\n", feci, feci->ci);
				ecm_db_connection_make_defunct(ci);
				ecm_front_end_connection_deref(feci);
				ecm_db_multicast_connection_deref(tuple_instance);
				return;
			}
#endif
			/*
			 * Push the updates to SFE
			 */
			DEBUG_TRACE("%px: Update accel %px\n", ci, tuple_instance);
			if ((feci->accel_mode <= ECM_FRONT_END_ACCELERATION_MODE_FAIL_DENIED) ||
					(feci->accel_mode != ECM_FRONT_END_ACCELERATION_MODE_ACCEL)) {
				DEBUG_TRACE("%px: Ignoring wrong mode accel for conn: %px\n", feci, feci->ci);
				ecm_front_end_connection_deref(feci);
				ecm_db_multicast_connection_deref(tuple_instance);
				return;
			}

			/*
			 * Update the new rules in FW. If returns error decelerate the connection
			 * and flush the Multicast rules.
			 */
			ret = ecm_sfe_multicast_ipv6_connection_update_accelerate(feci, &mc_sync, &aci_pr);
			if (ret < 0) {
				feci->decelerate(feci);
				ecm_front_end_connection_deref(feci);
				ecm_db_multicast_connection_deref(tuple_instance);
				return;
			}

			ecm_front_end_connection_deref(feci);

			/*
			 * Release the interfaces that may have left the connection
			 */
			ecm_db_multicast_connection_to_interfaces_leave(ci, &mc_sync);

			/*
			 * Move on to the next flow for the same source and group
			 */
			tuple_instance_next = ecm_db_multicast_connection_get_and_ref_next(tuple_instance);
			ecm_db_multicast_connection_deref(tuple_instance);
			tuple_instance = tuple_instance_next;
		}
		break;
	}
	case IP6MR_MFC_EVENT_DELETE:
	{
		struct ecm_front_end_connection_instance *feci;

		/*
		 * MFC reports that the multicast connection has died.
		 * Find the connections belongs to the same source/group address,
		 * Decelerate connections and free the frontend instance
		 */
		while (tuple_instance) {
			/*
			 * Get the source/group IP address for this multicast tuple and match
			 * with the source and group IP received from the event handler. If
			 * there is not match found, jumping to the next multicast tuple.
			 */
			ecm_db_multicast_tuple_instance_source_ip_get(tuple_instance, src_addr);
			ecm_db_multicast_tuple_instance_group_ip_get(tuple_instance, grp_addr);
			if (!(ECM_IP_ADDR_MATCH(src_addr, src_ip) && ECM_IP_ADDR_MATCH(grp_addr, dest_ip))) {
				DEBUG_TRACE("%px:Multicast tuple not matched, try next tuple %d\n", tuple_instance, op);
				tuple_instance_next = ecm_db_multicast_connection_get_and_ref_next(tuple_instance);
				ecm_db_multicast_connection_deref(tuple_instance);
				tuple_instance = tuple_instance_next;
				continue;
			}

			/*
			 * Get the DB connection instance using the tuple_instance, ref to 'ci'
			 * has been already taken by ecm_db_multicast_connection_find_and_ref()
			 */
			ci = ecm_db_multicast_connection_get_from_tuple(tuple_instance);
			DEBUG_TRACE("%px:%d delete the multicast flow: %px\n", ci, op, tuple_instance);

			/*
			 * Get the front end instance
			 */
			feci = ecm_db_connection_front_end_get_and_ref(ci);
			feci->decelerate(feci);
			ecm_front_end_connection_deref(feci);

			/*
			 * Move on to the next flow for the same source and group
			 */
			tuple_instance_next = ecm_db_multicast_connection_get_and_ref_next(tuple_instance);
			ecm_db_multicast_connection_deref(tuple_instance);
			tuple_instance = tuple_instance_next;
		}
		break;
	}
	default:
		DEBUG_TRACE("Unknow event notified from the kernel MFC module\n");
		break;
	}

	return;
}

/*
 * ecm_sfe_multicast_ipv6_debugfs_init()
 */
bool ecm_sfe_multicast_ipv6_debugfs_init(struct dentry *dentry)
{
	if (!ecm_debugfs_create_u32("multicast_accelerated_count", S_IRUGO, dentry,
						&ecm_sfe_multicast_ipv6_accelerated_count)) {
		DEBUG_ERROR("Failed to create ecm sfe ipv6 multicast_accelerated_count file in debugfs\n");
		return false;
	}

	return true;
}

/*
 * ecm_sfe_multicast_ipv6_stop()
 */
void ecm_sfe_multicast_ipv6_stop(int num)
{
	ecm_front_end_ipv6_mc_stopped = !!num;
}

/*
 * ecm_sfe_multicast_ipv6_init()
 * 	Register the callbacks for MCS snooper and MFC update
 */
int ecm_sfe_multicast_ipv6_init(struct dentry *dentry)
{
	if (!ecm_debugfs_create_u32("ecm_sfe_multicast_ipv6_stop", S_IRUGO | S_IWUSR, dentry,
					(u32 *)&ecm_front_end_ipv6_mc_stopped)) {
		DEBUG_ERROR("Failed to create ecm front end ipv6 mc stop file in debugfs\n");
		return -1;
	}

	/*
	 * Register multicast update callback to MCS snooper
	 */
	mc_bridge_ipv6_update_callback_register(ecm_sfe_multicast_ipv6_br_update_event_callback);

	/*
	 * Register multicast update callbacks to MFC
	 */
	ip6mr_register_mfc_event_offload_callback(ecm_sfe_multicast_ipv6_mfc_update_event_callback);
	return 0;
}

/*
 * ecm_sfe_multicast_ipv6_exit()
 *	Unregister the callbacks for MCS snooper and MFC update
 */
void ecm_sfe_multicast_ipv6_exit(void)
{
	/*
	 * Unregister multicast update callbacks to
	 * MFC and MCS snooper
	 */
	ip6mr_unregister_mfc_event_offload_callback();
	mc_bridge_ipv6_update_callback_deregister();
}
