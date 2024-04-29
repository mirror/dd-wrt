/*
 **************************************************************************
 * Copyright (c) 2015-2021 The Linux Foundation. All rights reserved.
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
#include <linux/kthread.h>
#include <linux/pkt_sched.h>
#include <linux/debugfs.h>
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

#include <linux/inetdevice.h>
#include <linux/if_arp.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_bridge.h>
#include <linux/if_bridge.h>
#include <net/arp.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_acct.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_l4proto.h>
#include <net/netfilter/nf_conntrack_zones.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/ipv4/nf_conntrack_ipv4.h>
#include <net/netfilter/ipv4/nf_defrag_ipv4.h>
#include <linux/netfilter/nf_conntrack_tftp.h>
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
#define DEBUG_LEVEL ECM_SFE_PORTED_IPV4_DEBUG_LEVEL

#include <sfe_api.h>

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
#include "ecm_sfe_ported_ipv4.h"
#include "ecm_sfe_ipv4.h"
#include "ecm_sfe_common.h"
#include "ecm_front_end_common.h"


static int ecm_sfe_ported_ipv4_accelerated_count[ECM_FRONT_END_PORTED_PROTO_MAX] = {0};
						/* Array of Number of TCP and UDP connections currently offloaded */

/*
 * Expose what should be a static flag in the TCP connection tracker.
 */
#ifdef ECM_OPENWRT_SUPPORT
extern int nf_ct_tcp_no_window_check;
#endif
extern int nf_ct_tcp_be_liberal;

/*
 * ecm_sfe_ported_ipv4_connection_callback()
 *	Callback for handling create ack/nack calls.
 */
static void ecm_sfe_ported_ipv4_connection_callback(void *app_data, struct sfe_ipv4_msg *nim)
{
	struct sfe_ipv4_rule_create_msg * __attribute__((unused)) nircm = &nim->msg.rule_create;
	uint32_t serial = (uint32_t)(ecm_ptr_t)app_data;
	struct ecm_db_connection_instance *ci;
	struct ecm_front_end_connection_instance *feci;
	ecm_front_end_acceleration_mode_t result_mode;
	bool is_defunct = false;

	/*
	 * Is this a response to a create message?
	 */
	if (nim->cm.type != SFE_TX_CREATE_RULE_MSG) {
		DEBUG_ERROR("%px: ported create callback with improper type: %d, serial: %u\n", nim, nim->cm.type, serial);
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

	/*
	 * Record command duration
	 */
	ecm_sfe_ipv4_accel_done_time_update(feci);

	/*
	 * Dump some useful trace information.
	 */
	DEBUG_TRACE("%px: accelerate response for connection: %px, serial: %u\n", feci, feci->ci, serial);
	DEBUG_TRACE("%px: rule_flags: %x, valid_flags: %x\n", feci, nircm->rule_flags, nircm->valid_flags);
	DEBUG_TRACE("%px: flow_ip: %pI4n:%d\n", feci, &nircm->tuple.flow_ip, nircm->tuple.flow_ident);
	DEBUG_TRACE("%px: return_ip: %pI4n:%d\n", feci, &nircm->tuple.return_ip, nircm->tuple.return_ident);
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

		spin_lock_bh(&ecm_sfe_ipv4_lock);
		_ecm_sfe_ipv4_accel_pending_clear(feci, result_mode);
		spin_unlock_bh(&ecm_sfe_ipv4_lock);

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
		 * Increment the no-action counter.  Our connection was decelerated on us with no action occurring.
		 */
		feci->stats.no_action_seen++;

		spin_lock_bh(&ecm_sfe_ipv4_lock);
		_ecm_sfe_ipv4_accel_pending_clear(feci, ECM_FRONT_END_ACCELERATION_MODE_DECEL);
		spin_unlock_bh(&ecm_sfe_ipv4_lock);

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
	 * Decelerate may have been attempted while we were "pending accel" and
	 * this function will return true if that was the case.
	 * If decelerate was pending then we need to begin deceleration :-(
	 */
	spin_lock_bh(&ecm_sfe_ipv4_lock);

	ecm_sfe_ported_ipv4_accelerated_count[feci->ported_accelerated_count_index]++;	/* Protocol specific counter */
	ecm_sfe_ipv4_accelerated_count++;		/* General running counter */

	if (!_ecm_sfe_ipv4_accel_pending_clear(feci, ECM_FRONT_END_ACCELERATION_MODE_ACCEL)) {
		/*
		 * Increment the no-action counter, this is reset if offload action is seen
		 */
		feci->stats.no_action_seen++;

		spin_unlock_bh(&ecm_sfe_ipv4_lock);
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

	spin_unlock_bh(&ecm_sfe_ipv4_lock);
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
 * ecm_sfe_ported_ipv4_connection_accelerate()
 *	Accelerate a connection
 */
static void ecm_sfe_ported_ipv4_connection_accelerate(struct ecm_front_end_connection_instance *feci,
									struct ecm_classifier_process_response *pr, bool is_l2_encap,
									struct nf_conn *ct, struct sk_buff *skb)
{
	uint16_t regen_occurrances;
	int protocol;
	int32_t from_ifaces_first;
	int32_t to_ifaces_first;
	struct ecm_db_iface_instance *from_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX];
	struct ecm_db_iface_instance *to_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX];
	struct ecm_db_iface_instance *from_sfe_iface;
	struct ecm_db_iface_instance *to_sfe_iface;
	int32_t from_sfe_iface_id;
	int32_t to_sfe_iface_id;
	uint8_t from_sfe_iface_address[ETH_ALEN];
	uint8_t to_sfe_iface_address[ETH_ALEN];
	ip_addr_t addr;
	struct sfe_ipv4_msg *nim;
	struct sfe_ipv4_rule_create_msg *nircm;
	struct ecm_classifier_instance *assignments[ECM_CLASSIFIER_TYPES];
	int aci_index;
	int assignment_count;
	sfe_tx_status_t sfe_tx_status;
	int32_t list_index;
	int32_t interface_type_counts[ECM_DB_IFACE_TYPE_COUNT];
	bool rule_invalid;
	uint8_t dest_mac_xlate[ETH_ALEN];
	ecm_db_direction_t ecm_dir;
	ecm_front_end_acceleration_mode_t result_mode;
	struct net *net = nf_ct_net(ct);
	struct nf_tcp_net *tn = nf_tcp_pernet(net);
	struct ecm_classifier_instance *aci;
	struct ecm_classifier_rule_create ecrc;
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
	 * Test if acceleration is permitted
	 */
	if (!ecm_sfe_ipv4_accel_pending_set(feci)) {
		DEBUG_TRACE("%px: Acceleration not permitted: %px skb=%px\n", feci, feci->ci, skb);
		return;
	}

	nim = (struct sfe_ipv4_msg *)kzalloc(sizeof(struct sfe_ipv4_msg), GFP_ATOMIC | __GFP_NOWARN);
	if (!nim) {
		DEBUG_WARN("%px: no memory for sfe ipv4 message structure instance: %px\n", feci, feci->ci);
		ecm_sfe_ipv4_accel_pending_clear(feci, ECM_FRONT_END_ACCELERATION_MODE_DECEL);
		return;
	}

	/*
	 * Okay construct an accel command.
	 * Initialise creation structure.
	 * NOTE: We leverage the app_data void pointer to be our 32 bit connection serial number.
	 * When we get it back we re-cast it to a uint32 and do a faster connection lookup.
	 */
	sfe_ipv4_msg_init(nim, SFE_SPECIAL_INTERFACE_IPV4, SFE_TX_CREATE_RULE_MSG,
			sizeof(struct sfe_ipv4_rule_create_msg),
			ecm_sfe_ported_ipv4_connection_callback,
			(void *)(ecm_ptr_t)ecm_db_connection_serial_get(feci->ci));

	nircm = &nim->msg.rule_create;
	nircm->valid_flags = 0;
	nircm->rule_flags = 0;

	/*
	 * Initialize VLAN tag information
	 */
	nircm->vlan_primary_rule.ingress_vlan_tag = SFE_VLAN_ID_NOT_CONFIGURED;
	nircm->vlan_primary_rule.egress_vlan_tag = SFE_VLAN_ID_NOT_CONFIGURED;
	nircm->vlan_secondary_rule.ingress_vlan_tag = SFE_VLAN_ID_NOT_CONFIGURED;
	nircm->vlan_secondary_rule.egress_vlan_tag = SFE_VLAN_ID_NOT_CONFIGURED;

	/*
	 * Get the interface lists of the connection, we must have at least one interface in the list to continue
	 */
	from_ifaces_first = ecm_db_connection_interfaces_get_and_ref(feci->ci, from_ifaces, ECM_DB_OBJ_DIR_FROM);
	if (from_ifaces_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
		DEBUG_WARN("%px: Accel attempt failed - no interfaces in from_interfaces list!\n", feci);
		goto ported_accel_bad_rule;
	}

	to_ifaces_first = ecm_db_connection_interfaces_get_and_ref(feci->ci, to_ifaces, ECM_DB_OBJ_DIR_TO);
	if (to_ifaces_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
		DEBUG_WARN("%px: Accel attempt failed - no interfaces in to_interfaces list!\n", feci);
		ecm_db_connection_interfaces_deref(from_ifaces, from_ifaces_first);
		goto ported_accel_bad_rule;
	}

	/*
	 * First interface in each must be a known sfe interface
	 */
	from_sfe_iface = from_ifaces[from_ifaces_first];
	to_sfe_iface = to_ifaces[to_ifaces_first];
	from_sfe_iface_id = ecm_db_iface_interface_identifier_get(from_sfe_iface);
	to_sfe_iface_id = ecm_db_iface_interface_identifier_get(to_sfe_iface);
	if ((from_sfe_iface_id < 0) || (to_sfe_iface_id < 0)) {
		DEBUG_TRACE("%px: from_sfe_iface_id: %d, to_sfe_iface_id: %d\n", feci, from_sfe_iface_id, to_sfe_iface_id);
		ecm_db_connection_interfaces_deref(from_ifaces, from_ifaces_first);
		ecm_db_connection_interfaces_deref(to_ifaces, to_ifaces_first);
		goto ported_accel_bad_rule;
	}

	/*
	 * New rule being created
	 */
	nircm->valid_flags |= SFE_RULE_CREATE_CONN_VALID;

	/*
	 * Set interface numbers involved in accelerating this connection.
	 * These are the outer facing addresses from the heirarchy interface lists we got above.
	 * These may be overridden later if we detect special interface types e.g. ipsec.
	 */
	nircm->conn_rule.flow_interface_num = from_sfe_iface_id;
	nircm->conn_rule.return_interface_num = to_sfe_iface_id;

	/*
	 * Check which side of the connection can support L2 acceleration.
	 * The check is done only for the routed flows and if the L2 feature is enabled.
	 */
	if (sfe_is_l2_feature_enabled() && ecm_db_connection_is_routed_get(feci->ci)) {
		rcu_read_lock();
		l2_accel_check = rcu_dereference(ecm_sfe_cb.l2_accel_check);
		if (l2_accel_check) {
			struct ecm_sfe_common_tuple l2_accel_tuple;

			ecm_sfe_common_tuple_set(feci, from_sfe_iface_id, to_sfe_iface_id, &l2_accel_tuple);

			l2_accel_bits = l2_accel_check(&l2_accel_tuple);
		}
		rcu_read_unlock();
	}

	/*
	 * Set interface numbers involved in accelerating this connection.
	 * These are the inner facing addresses from the heirarchy interface lists we got above.
	 */
	nim->msg.rule_create.conn_rule.flow_top_interface_num = ecm_db_iface_interface_identifier_get(from_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX-1]);
	nim->msg.rule_create.conn_rule.return_top_interface_num = ecm_db_iface_interface_identifier_get(to_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX-1]);

	/*
	 * We know that each outward facing interface is known to the SFE and so this connection could be accelerated.
	 * However the lists may also specify other interesting details that must be included in the creation command,
	 * for example, ethernet MAC, VLAN tagging or PPPoE session information.
	 * We get this information by walking from the outer to the innermost interface for each list and examine the interface types.
	 *
	 * Start with the 'from' (src) side.
	 * NOTE: The lists may contain a complex heirarchy of similar type of interface e.g. multiple vlans or tunnels within tunnels.
	 * This SFE cannot handle that - there is no way to describe this in the rule - if we see multiple types that would conflict we have to abort.
	 */
	DEBUG_TRACE("%px: Examine from/src heirarchy list. skb=%px\n", feci, skb);
	memset(interface_type_counts, 0, sizeof(interface_type_counts));
	rule_invalid = false;
	for (list_index = from_ifaces_first; !rule_invalid && (list_index < ECM_DB_IFACE_HEIRARCHY_MAX); list_index++) {
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
#ifdef ECM_INTERFACE_VXLAN_ENABLE
		struct ecm_db_interface_info_vxlan vxlan_info;
#endif
		ii = from_ifaces[list_index];
		ii_type = ecm_db_iface_type_get(ii);
		ii_name = ecm_db_interface_type_to_string(ii_type);

		DEBUG_TRACE("%px: list_index: %d, ii: %px (%s %d %d), type: %d (%s)\n",
				feci, list_index, ii,
				ii->name, ii->interface_identifier, ii->ae_interface_identifier,
				ii_type, ii_name);

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

			if (ecm_sfe_common_is_l2_iface_supported(ECM_DB_IFACE_TYPE_BRIDGE, list_index, from_ifaces_first) && (l2_accel_bits & ECM_SFE_COMMON_FLOW_L2_ACCEL_ALLOWED)) {
				nircm->rule_flags |= SFE_RULE_CREATE_FLAG_USE_FLOW_BOTTOM_INTERFACE;
				feci->set_stats_bitmap(feci, ECM_DB_OBJ_DIR_FROM, ECM_DB_IFACE_TYPE_BRIDGE);
			}

			ecm_db_iface_bridge_address_get(ii, from_sfe_iface_address);
			if (is_valid_ether_addr(from_sfe_iface_address)) {
				ether_addr_copy((uint8_t *)nircm->src_mac_rule.flow_src_mac, from_sfe_iface_address);
				nircm->src_mac_rule.mac_valid_flags |= SFE_SRC_MAC_FLOW_VALID;
				nircm->valid_flags |= SFE_RULE_CREATE_SRC_MAC_VALID;
			}
			DEBUG_TRACE("%px: Bridge - mac: %pM\n", feci, from_sfe_iface_address);
			break;

		case ECM_DB_IFACE_TYPE_OVS_BRIDGE:
#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
			DEBUG_TRACE("%px: OVS Bridge\n", feci);
			if (interface_type_counts[ii_type] != 0) {
				/*
				 * Cannot cascade bridges
				 */
				rule_invalid = true;
				DEBUG_TRACE("%px: OVS Bridge - ignore additional\n", feci);
				break;
			}

			if (ecm_sfe_common_is_l2_iface_supported(ECM_DB_IFACE_TYPE_OVS_BRIDGE, list_index, from_ifaces_first) && (l2_accel_bits & ECM_SFE_COMMON_FLOW_L2_ACCEL_ALLOWED)) {
				nircm->rule_flags |= SFE_RULE_CREATE_FLAG_USE_FLOW_BOTTOM_INTERFACE;
				feci->set_stats_bitmap(feci, ECM_DB_OBJ_DIR_FROM, ECM_DB_IFACE_TYPE_OVS_BRIDGE);
			}

			ecm_db_iface_ovs_bridge_address_get(ii, from_sfe_iface_address);
			if (is_valid_ether_addr(from_sfe_iface_address)) {
				ether_addr_copy((uint8_t *)nircm->src_mac_rule.flow_src_mac, from_sfe_iface_address);
				nircm->src_mac_rule.mac_valid_flags |= SFE_SRC_MAC_FLOW_VALID;
				nircm->valid_flags |= SFE_RULE_CREATE_SRC_MAC_VALID;
			}
			DEBUG_TRACE("%px: OVS Bridge - mac: %pM\n", feci, from_sfe_iface_address);
#else
			rule_invalid = true;
#endif
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
			ecm_db_iface_ethernet_address_get(ii, from_sfe_iface_address);
			DEBUG_TRACE("%px: Ethernet - mac: %pM\n", feci, from_sfe_iface_address);
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

			nircm->pppoe_rule.flow_pppoe_session_id = pppoe_info.pppoe_session_id;
			memcpy(nircm->pppoe_rule.flow_pppoe_remote_mac, pppoe_info.remote_mac, ETH_ALEN);
			nircm->valid_flags |= SFE_RULE_CREATE_PPPOE_DECAP_VALID;
			nircm->rule_flags |= SFE_RULE_CREATE_FLAG_USE_FLOW_BOTTOM_INTERFACE;

			DEBUG_TRACE("%px: PPPoE - session: %x, mac: %pM\n", feci,
					nircm->pppoe_rule.flow_pppoe_session_id,
					nircm->pppoe_rule.flow_pppoe_remote_mac);
#else
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
				nircm->vlan_primary_rule.ingress_vlan_tag = vlan_value;
			} else {
				nircm->vlan_secondary_rule.ingress_vlan_tag = vlan_value;
			}
			nircm->valid_flags |= SFE_RULE_CREATE_VLAN_VALID;

			if (sfe_is_l2_feature_enabled() && (l2_accel_bits & ECM_SFE_COMMON_FLOW_L2_ACCEL_ALLOWED)) {
				nircm->rule_flags |= SFE_RULE_CREATE_FLAG_USE_FLOW_BOTTOM_INTERFACE;
				feci->set_stats_bitmap(feci, ECM_DB_OBJ_DIR_FROM, ECM_DB_IFACE_TYPE_VLAN);

				if (is_valid_ether_addr(vlan_info.address)) {
					ether_addr_copy((uint8_t *)nircm->src_mac_rule.flow_src_mac, vlan_info.address);
					nircm->src_mac_rule.mac_valid_flags |= SFE_SRC_MAC_FLOW_VALID;
					nircm->valid_flags |= SFE_RULE_CREATE_SRC_MAC_VALID;
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
		case ECM_DB_IFACE_TYPE_MACVLAN:
#ifdef ECM_INTERFACE_MACVLAN_ENABLE
			if (ecm_sfe_common_is_l2_iface_supported(ECM_DB_IFACE_TYPE_MACVLAN, list_index, from_ifaces_first) && (l2_accel_bits & ECM_SFE_COMMON_FLOW_L2_ACCEL_ALLOWED)) {
				nircm->rule_flags |= SFE_RULE_CREATE_FLAG_USE_FLOW_BOTTOM_INTERFACE;
				feci->set_stats_bitmap(feci, ECM_DB_OBJ_DIR_FROM, ECM_DB_IFACE_TYPE_MACVLAN);
			}

			ecm_db_iface_macvlan_address_get(ii, from_sfe_iface_address);
			ether_addr_copy((uint8_t *)nircm->src_mac_rule.flow_src_mac, from_sfe_iface_address);
			nircm->src_mac_rule.mac_valid_flags |= SFE_SRC_MAC_FLOW_VALID;
			nircm->valid_flags |= SFE_RULE_CREATE_SRC_MAC_VALID;

			DEBUG_TRACE("%px: Macvlan - mac: %pM\n", feci, from_sfe_iface_address);
#else
			rule_invalid = true;
			DEBUG_TRACE("%px: MACVLAN - unsupported\n", feci);
#endif
			break;

		case ECM_DB_IFACE_TYPE_IPSEC_TUNNEL:
#ifdef ECM_INTERFACE_IPSEC_ENABLE
			DEBUG_TRACE("%px: IPSEC\n", feci);
			if (interface_type_counts[ii_type] != 0) {
				/*
				 * Can only support one ipsec
				 */
				rule_invalid = true;
				DEBUG_TRACE("%px: IPSEC - additional unsupported\n", feci);
				break;
			}
#else
			rule_invalid = true;
			DEBUG_TRACE("%px: IPSEC - unsupported\n", feci);
#endif
			break;

		case ECM_DB_IFACE_TYPE_LAG:
#ifdef ECM_INTERFACE_BOND_ENABLE
			if (sfe_is_l2_feature_enabled() && (l2_accel_bits & ECM_SFE_COMMON_FLOW_L2_ACCEL_ALLOWED)) {
				nircm->rule_flags |= SFE_RULE_CREATE_FLAG_USE_FLOW_BOTTOM_INTERFACE;
				/*
				 * LAG device gets its stats by summing up all stats of its
				 * slaves (i.e. 'ethx' devices). Please see bond_get_stats()
				 * in linux-5.4/drivers/net/bonding/bond_main.c.
				 * So no need to call feci->set_stats_bitmap().
				 */

				ecm_db_iface_lag_address_get(ii, from_sfe_iface_address);
				if (is_valid_ether_addr(from_sfe_iface_address)) {
					ether_addr_copy((uint8_t *)nircm->src_mac_rule.flow_src_mac, from_sfe_iface_address);
					nircm->src_mac_rule.mac_valid_flags |= SFE_SRC_MAC_FLOW_VALID;
					nircm->valid_flags |= SFE_RULE_CREATE_SRC_MAC_VALID;
				}
				DEBUG_TRACE("%px: LAG - mac: %pM\n", feci, from_sfe_iface_address);
			}
#else
			rule_invalid = true;
			DEBUG_TRACE("%px: LAG - unsupported\n", feci);
#endif
			break;

		case ECM_DB_IFACE_TYPE_VXLAN:
#ifdef ECM_INTERFACE_VXLAN_ENABLE
			ecm_db_iface_vxlan_info_get(ii, &vxlan_info);

			/*
			 * For VxLAN device, 4-tuple connection rule is added only for outer flow,
			 * where source port is set to zero.
			 */
			if (!vxlan_info.if_type) {
				nircm->rule_flags |= SFE_RULE_CREATE_NO_SRC_IDENT;
			}
#else
			rule_invalid = true;
			DEBUG_TRACE("%px: VXLAN - unsupported\n", feci);
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
		DEBUG_WARN("%px: from/src Rule invalid\n", feci);
		ecm_db_connection_interfaces_deref(from_ifaces, from_ifaces_first);
		ecm_db_connection_interfaces_deref(to_ifaces, to_ifaces_first);
		goto ported_accel_bad_rule;
	}

	/*
	 * Now examine the TO / DEST heirarchy list to construct the destination part of the rule
	 */
	DEBUG_TRACE("%px: Examine to/dest heirarchy list\n", feci);
	memset(interface_type_counts, 0, sizeof(interface_type_counts));
	rule_invalid = false;
	for (list_index = to_ifaces_first; !rule_invalid && (list_index < ECM_DB_IFACE_HEIRARCHY_MAX); list_index++) {
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

		ii = to_ifaces[list_index];
		ii_type = ecm_db_iface_type_get(ii);
		ii_name = ecm_db_interface_type_to_string(ii_type);

		DEBUG_TRACE("%px: list_index: %d, ii: %px (%s %d %d), type: %d (%s)\n",
				feci, list_index, ii,
				ii->name, ii->interface_identifier, ii->ae_interface_identifier,
				ii_type, ii_name);

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

			if (ecm_sfe_common_is_l2_iface_supported(ECM_DB_IFACE_TYPE_BRIDGE, list_index, to_ifaces_first) && (l2_accel_bits & ECM_SFE_COMMON_RETURN_L2_ACCEL_ALLOWED)) {
				nircm->rule_flags |= SFE_RULE_CREATE_FLAG_USE_RETURN_BOTTOM_INTERFACE;
				feci->set_stats_bitmap(feci, ECM_DB_OBJ_DIR_TO, ECM_DB_IFACE_TYPE_BRIDGE);
			}

			ecm_db_iface_bridge_address_get(ii, to_sfe_iface_address);
			if (is_valid_ether_addr(to_sfe_iface_address)) {
				ether_addr_copy((uint8_t *)nircm->src_mac_rule.return_src_mac, to_sfe_iface_address);
				nircm->src_mac_rule.mac_valid_flags |= SFE_SRC_MAC_RETURN_VALID;
				nircm->valid_flags |= SFE_RULE_CREATE_SRC_MAC_VALID;
			}

			DEBUG_TRACE("%px: Bridge - mac: %pM\n", feci, to_sfe_iface_address);
			break;

		case ECM_DB_IFACE_TYPE_OVS_BRIDGE:
#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
			DEBUG_TRACE("%px: OVS Bridge\n", feci);
			if (interface_type_counts[ii_type] != 0) {
				/*
				 * Cannot cascade bridges
				 */
				rule_invalid = true;
				DEBUG_TRACE("%px: OVS Bridge - ignore additional\n", feci);
				break;
			}

			if (ecm_sfe_common_is_l2_iface_supported(ECM_DB_IFACE_TYPE_OVS_BRIDGE, list_index, to_ifaces_first) && (l2_accel_bits & ECM_SFE_COMMON_RETURN_L2_ACCEL_ALLOWED)) {
				nircm->rule_flags |= SFE_RULE_CREATE_FLAG_USE_RETURN_BOTTOM_INTERFACE;
				feci->set_stats_bitmap(feci, ECM_DB_OBJ_DIR_TO, ECM_DB_IFACE_TYPE_OVS_BRIDGE);
			}

			ecm_db_iface_ovs_bridge_address_get(ii, to_sfe_iface_address);
			if (is_valid_ether_addr(to_sfe_iface_address)) {
				ether_addr_copy((uint8_t *)nircm->src_mac_rule.flow_src_mac, to_sfe_iface_address);
				nircm->src_mac_rule.mac_valid_flags |= SFE_SRC_MAC_RETURN_VALID;
				nircm->valid_flags |= SFE_RULE_CREATE_SRC_MAC_VALID;
			}
			DEBUG_TRACE("%px: OVS Bridge - mac: %pM\n", feci, to_sfe_iface_address);
#else
			rule_invalid = true;
#endif
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
			nircm->pppoe_rule.return_pppoe_session_id = pppoe_info.pppoe_session_id;
			memcpy(nircm->pppoe_rule.return_pppoe_remote_mac, pppoe_info.remote_mac, ETH_ALEN);
			nircm->valid_flags |= SFE_RULE_CREATE_PPPOE_ENCAP_VALID;
			nircm->rule_flags |= SFE_RULE_CREATE_FLAG_USE_RETURN_BOTTOM_INTERFACE;

			DEBUG_TRACE("%px: PPPoE - session: %x, mac: %pM\n", feci,
				    nircm->pppoe_rule.return_pppoe_session_id,
				    nircm->pppoe_rule.return_pppoe_remote_mac);
#else
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
			if (interface_type_counts[ii_type] == 0) {
				nircm->vlan_primary_rule.egress_vlan_tag = vlan_value;
			} else {
				nircm->vlan_secondary_rule.egress_vlan_tag = vlan_value;
			}
			nircm->valid_flags |= SFE_RULE_CREATE_VLAN_VALID;

			if (sfe_is_l2_feature_enabled() && (l2_accel_bits & ECM_SFE_COMMON_RETURN_L2_ACCEL_ALLOWED)) {
				nircm->rule_flags |= SFE_RULE_CREATE_FLAG_USE_RETURN_BOTTOM_INTERFACE;
				feci->set_stats_bitmap(feci, ECM_DB_OBJ_DIR_TO, ECM_DB_IFACE_TYPE_VLAN);

				if (is_valid_ether_addr(vlan_info.address)) {
					ether_addr_copy((uint8_t *)nircm->src_mac_rule.return_src_mac, vlan_info.address);
					nircm->src_mac_rule.mac_valid_flags |= SFE_SRC_MAC_RETURN_VALID;
					nircm->valid_flags |= SFE_RULE_CREATE_SRC_MAC_VALID;
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

		case ECM_DB_IFACE_TYPE_MACVLAN:
#ifdef ECM_INTERFACE_MACVLAN_ENABLE
			if (ecm_sfe_common_is_l2_iface_supported(ECM_DB_IFACE_TYPE_MACVLAN, list_index, to_ifaces_first) && (l2_accel_bits & ECM_SFE_COMMON_RETURN_L2_ACCEL_ALLOWED)) {
				nircm->rule_flags |= SFE_RULE_CREATE_FLAG_USE_RETURN_BOTTOM_INTERFACE;
				feci->set_stats_bitmap(feci, ECM_DB_OBJ_DIR_TO, ECM_DB_IFACE_TYPE_MACVLAN);
			}

			ecm_db_iface_macvlan_address_get(ii, to_sfe_iface_address);
			ether_addr_copy((uint8_t *)nircm->src_mac_rule.return_src_mac, to_sfe_iface_address);
			nircm->src_mac_rule.mac_valid_flags |= SFE_SRC_MAC_RETURN_VALID;
			nircm->valid_flags |= SFE_RULE_CREATE_SRC_MAC_VALID;

			DEBUG_TRACE("%px: Macvlan - mac: %pM\n", feci, to_sfe_iface_address);
#else
			rule_invalid = true;
			DEBUG_TRACE("%px: MACVLAN - unsupported\n", feci);
#endif
			break;

		case ECM_DB_IFACE_TYPE_IPSEC_TUNNEL:
#ifdef ECM_INTERFACE_IPSEC_ENABLE
			DEBUG_TRACE("%px: IPSEC\n", feci);
			if (interface_type_counts[ii_type] != 0) {
				/*
				 * Can only support one ipsec
				 */
				rule_invalid = true;
				DEBUG_TRACE("%px: IPSEC - additional unsupported\n", feci);
				break;
			}
#else
			rule_invalid = true;
			DEBUG_TRACE("%px: IPSEC - unsupported\n", feci);
#endif
			break;

		case ECM_DB_IFACE_TYPE_LAG:
#ifdef ECM_INTERFACE_BOND_ENABLE
			if (sfe_is_l2_feature_enabled() && (l2_accel_bits & ECM_SFE_COMMON_RETURN_L2_ACCEL_ALLOWED)) {
				nircm->rule_flags |= SFE_RULE_CREATE_FLAG_USE_RETURN_BOTTOM_INTERFACE;
				/*
				 * LAG device gets its stats by summing up all stats of its
				 * slaves (i.e. 'ethx' devices). Please see bond_get_stats()
				 * in linux-5.4/drivers/net/bonding/bond_main.c.
				 * So no need to call feci->set_stats_bitmap().
				 */

				ecm_db_iface_lag_address_get(ii, to_sfe_iface_address);
				if (is_valid_ether_addr(to_sfe_iface_address)) {
					ether_addr_copy((uint8_t *)nircm->src_mac_rule.return_src_mac, to_sfe_iface_address);
					nircm->src_mac_rule.mac_valid_flags |= SFE_SRC_MAC_RETURN_VALID;
					nircm->valid_flags |= SFE_RULE_CREATE_SRC_MAC_VALID;
				}
				DEBUG_TRACE("%px: LAG - mac: %pM\n", feci, to_sfe_iface_address);
			}
#else
			rule_invalid = true;
			DEBUG_TRACE("%px: LAG - unsupported\n", feci);
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
		ecm_db_connection_interfaces_deref(from_ifaces, from_ifaces_first);
		ecm_db_connection_interfaces_deref(to_ifaces, to_ifaces_first);
		goto ported_accel_bad_rule;
	}

	/*
	 * Routed or bridged?
	 */
	if (ecm_db_connection_is_routed_get(feci->ci)) {
		nircm->rule_flags |= SFE_RULE_CREATE_FLAG_ROUTED;
	} else {
		nircm->rule_flags |= SFE_RULE_CREATE_FLAG_BRIDGE_FLOW;
		if (is_l2_encap) {
			nircm->rule_flags |= SFE_RULE_CREATE_FLAG_L2_ENCAP;
		}
	}

	if (ecm_interface_src_check) {
		DEBUG_INFO("%px: Source interface check flag is enabled\n", feci);
		nircm->rule_flags |= SFE_RULE_CREATE_FLAG_FLOW_SRC_INTERFACE_CHECK;
		nircm->rule_flags |= SFE_RULE_CREATE_FLAG_RETURN_SRC_INTERFACE_CHECK;
	}

	/*
	 * Enable source interface check without flushing the rule for this flow to re-inject the packet to
	 * the network stack in SFE driver after the first pass of the packet coming with the L2 interface.
	 * In the second pass, the packet will come to SFE with the L3 interface. If there are more than 3 interfaces
	 * in the hierarchy, the packet will be re-injected to the stack until the flows input interface matches with the
	 * rule's match_dev.
	 */
	if (!(l2_accel_bits & ECM_SFE_COMMON_FLOW_L2_ACCEL_ALLOWED)) {
		nircm->rule_flags |= SFE_RULE_CREATE_FLAG_FLOW_SRC_INTERFACE_CHECK;
		nircm->rule_flags |= SFE_RULE_CREATE_FLAG_FLOW_SRC_INTERFACE_CHECK_NO_FLUSH;
	}

	if (!(l2_accel_bits & ECM_SFE_COMMON_RETURN_L2_ACCEL_ALLOWED)) {
		nircm->rule_flags |= SFE_RULE_CREATE_FLAG_RETURN_SRC_INTERFACE_CHECK;
		nircm->rule_flags |= SFE_RULE_CREATE_FLAG_RETURN_SRC_INTERFACE_CHECK_NO_FLUSH;
	}

	/*
	 * Set up the flow and return qos tags
	 */
	if (pr->process_actions & ECM_CLASSIFIER_PROCESS_ACTION_QOS_TAG) {
		nircm->qos_rule.flow_qos_tag = (uint32_t)pr->flow_qos_tag;
		nircm->qos_rule.return_qos_tag = (uint32_t)pr->return_qos_tag;
		nircm->valid_flags |= SFE_RULE_CREATE_QOS_VALID;
	}

#if defined ECM_CLASSIFIER_DSCP_ENABLE || defined ECM_CLASSIFIER_EMESH_ENABLE
	/*
	 * DSCP information?
	 */
	if (pr->process_actions & ECM_CLASSIFIER_PROCESS_ACTION_DSCP) {
		nircm->dscp_rule.flow_dscp = pr->flow_dscp;
		nircm->dscp_rule.return_dscp = pr->return_dscp;
		nircm->rule_flags |= SFE_RULE_CREATE_FLAG_DSCP_MARKING;
		nircm->valid_flags |= SFE_RULE_CREATE_DSCP_MARKING_VALID;
	}

	/*
	 * Set up the flow and return mark.
	 */
	if (pr->process_actions & ECM_CLASSIFIER_PROCESS_ACTION_MARK) {
		nircm->mark_rule.flow_mark = (uint32_t)pr->flow_mark;
		nircm->mark_rule.return_mark = (uint32_t)pr->return_mark;
		nircm->valid_flags |= SFE_RULE_CREATE_MARK_VALID;
	}
#endif

#ifdef ECM_CLASSIFIER_EMESH_ENABLE
	/*
	 * SAWF information
	 */
	if (pr->process_actions & ECM_CLASSIFIER_PROCESS_ACTION_EMESH_SAWF_TAG) {
		nircm->sawf_rule.flow_mark = pr->flow_sawf_metadata;
		nircm->sawf_rule.return_mark = pr->return_sawf_metadata;
	}

	/*
	 * VLAN pcp remark set in SAWF classifer, we modify the pcp value in VLAN tag
	 * and send the update VLAN tag to SFE.
	 */
	if (pr->process_actions & ECM_CLASSIFIER_PROCESS_ACTION_EMESH_SAWF_VLAN_PCP_REMARK) {
		if (pr->flow_vlan_pcp != SFE_INVALID_VLAN_PCP &&
				nircm->vlan_primary_rule.egress_vlan_tag != SFE_VLAN_ID_NOT_CONFIGURED) {
			nircm->vlan_primary_rule.egress_vlan_tag &= ~VLAN_PRIO_MASK;
			nircm->vlan_primary_rule.egress_vlan_tag |= pr->flow_vlan_pcp << VLAN_PRIO_SHIFT;
		}

		if (pr->return_vlan_pcp != SFE_INVALID_VLAN_PCP &&
				nircm->vlan_primary_rule.ingress_vlan_tag != SFE_VLAN_ID_NOT_CONFIGURED) {
			nircm->vlan_primary_rule.ingress_vlan_tag &= ~VLAN_PRIO_MASK;
			nircm->vlan_primary_rule.ingress_vlan_tag |= pr->return_vlan_pcp << VLAN_PRIO_SHIFT;
		}
	}
#endif

#ifdef ECM_CLASSIFIER_OVS_ENABLE
	if (ecm_front_end_is_feature_supported(ECM_FE_FEATURE_OVS_VLAN)) {
		/*
		 * Copy the both primary and secondary (if exist) VLAN tags.
		 */
		if (pr->process_actions & ECM_CLASSIFIER_PROCESS_ACTION_OVS_VLAN_TAG) {
			nircm->vlan_primary_rule.ingress_vlan_tag = pr->ingress_vlan_tag[0];
			nircm->vlan_primary_rule.egress_vlan_tag = pr->egress_vlan_tag[0];
			nircm->valid_flags |= SFE_RULE_CREATE_VLAN_VALID;
		}

		if (pr->process_actions & ECM_CLASSIFIER_PROCESS_ACTION_OVS_VLAN_QINQ_TAG) {
			nircm->vlan_secondary_rule.ingress_vlan_tag = pr->ingress_vlan_tag[1];
			nircm->vlan_secondary_rule.egress_vlan_tag = pr->egress_vlan_tag[1];
		}
	}
#endif

	protocol = ecm_db_connection_protocol_get(feci->ci);

	/*
	 * Set protocol
	 */
	nircm->tuple.protocol = (int32_t)protocol;

	/*
	 * The flow_ip is where the connection established from
	 */
	ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_FROM, addr);
	ECM_IP_ADDR_TO_NIN4_ADDR(nircm->tuple.flow_ip, addr);

	/*
	 * The return_ip is where the connection is established to, however, in the case of ingress
	 * the return_ip would be the routers WAN IP - i.e. the NAT'ed version.
	 * Getting the NAT'ed version here works for ingress or egress packets, for egress
	 * the NAT'ed version would be the same as the normal address
	 */
	ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_TO_NAT, addr);
	ECM_IP_ADDR_TO_NIN4_ADDR(nircm->tuple.return_ip, addr);

	/*
	 * When the packet is forwarded to the next interface get the address the source IP of the
	 * packet should be translated to.  For egress this is the NAT'ed from address.
	 * This also works for ingress as the NAT'ed version of the WAN host would be the same as non-NAT'ed
	 */
	ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_FROM_NAT, addr);
	ECM_IP_ADDR_TO_NIN4_ADDR(nircm->conn_rule.flow_ip_xlate, addr);

	/*
	 * The destination address is what the destination IP is translated to as it is forwarded to the next interface.
	 * For egress this would yield the normal wan host and for ingress this would correctly NAT back to the LAN host
	 */
	ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_TO, addr);
	ECM_IP_ADDR_TO_NIN4_ADDR(nircm->conn_rule.return_ip_xlate, addr);

	/*
	 * Same approach as above for port information
	 */
	nircm->tuple.flow_ident = htons(ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_FROM));
	nircm->tuple.return_ident = htons(ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_TO_NAT));
	nircm->conn_rule.flow_ident_xlate = htons(ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_FROM_NAT));
	nircm->conn_rule.return_ident_xlate = htons(ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_TO));

	/*
	 * Get mac addresses.
	 * The src_mac is the mac address of the node that established the connection.
	 * This will work whether the from_node is LAN (egress) or WAN (ingress).
	 */
	ecm_db_connection_node_address_get(feci->ci, ECM_DB_OBJ_DIR_FROM, (uint8_t *)nircm->conn_rule.flow_mac);

	/*
	 * The dest_mac is more complex.  For egress it is the node address of the 'to' side of the connection.
	 * For ingress it is the node adress of the NAT'ed 'to' IP.
	 * Essentially it is the MAC of node associated with create.dest_ip and this is "to nat" side.
	 */
	ecm_db_connection_node_address_get(feci->ci, ECM_DB_OBJ_DIR_TO_NAT, (uint8_t *)nircm->conn_rule.return_mac);

	/*
	 * The dest_mac_xlate is the mac address to replace the pkt.dst_mac when a packet is sent to->from
	 * For bridged connections this does not change.
	 * For routed connections this is the mac of the 'to' node side of the connection.
	 */
	if (ecm_db_connection_is_routed_get(feci->ci)) {
		ecm_db_connection_node_address_get(feci->ci, ECM_DB_OBJ_DIR_TO, dest_mac_xlate);
	} else {
		/*
		 * Bridge flows preserve the MAC addressing
		 */
		memcpy(dest_mac_xlate, (uint8_t *)nircm->conn_rule.return_mac, ETH_ALEN);
	}

	/*
	 * Refer to the Example 2 and 3 in ecm_sfe_ipv4_ip_process() function for egress
	 * and ingress NAT'ed cases. In these cases, the destination node is the one which has the
	 * ip_dest_addr. So, above we get the mac address of this host and use that mac address
	 * for the destination node address in NAT'ed cases.
	 */
	ecm_dir = ecm_db_connection_direction_get(feci->ci);
	if ((ecm_dir == ECM_DB_DIRECTION_INGRESS_NAT) || (ecm_dir == ECM_DB_DIRECTION_EGRESS_NAT)) {
		memcpy(nircm->conn_rule.return_mac, dest_mac_xlate, ETH_ALEN);
	}

	/*
	 * Get MTU information
	 */
	nircm->conn_rule.flow_mtu = (uint32_t)ecm_db_connection_iface_mtu_get(feci->ci, ECM_DB_OBJ_DIR_FROM);
	nircm->conn_rule.return_mtu = (uint32_t)ecm_db_connection_iface_mtu_get(feci->ci, ECM_DB_OBJ_DIR_TO);

	if (protocol == IPPROTO_TCP) {
		/*
		 * Need window scaling and remarking information if available
		 * Start by looking up the conntrack connection
		 */
		if (!ct) {
			/*
			 * No conntrack so no need to check window sequence space
			 */
			DEBUG_TRACE("%px: TCP Accel no ct from conn %px to get window data\n", feci, feci->ci);
			nircm->rule_flags |= SFE_RULE_CREATE_FLAG_NO_SEQ_CHECK;
		} else {
			int flow_dir;
			int return_dir;

			ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_FROM, addr);
			ecm_front_end_flow_and_return_directions_get(ct, addr, 4, &flow_dir, &return_dir);

			DEBUG_TRACE("%px: TCP Accel Get window data from ct %px for conn %px\n", feci, ct, feci->ci);
			spin_lock_bh(&ct->lock);
			nircm->tcp_rule.flow_window_scale = ct->proto.tcp.seen[flow_dir].td_scale;
			nircm->tcp_rule.flow_max_window = ct->proto.tcp.seen[flow_dir].td_maxwin;
			nircm->tcp_rule.flow_end = ct->proto.tcp.seen[flow_dir].td_end;
			nircm->tcp_rule.flow_max_end = ct->proto.tcp.seen[flow_dir].td_maxend;
			nircm->tcp_rule.return_window_scale = ct->proto.tcp.seen[return_dir].td_scale;
			nircm->tcp_rule.return_max_window = ct->proto.tcp.seen[return_dir].td_maxwin;
			nircm->tcp_rule.return_end = ct->proto.tcp.seen[return_dir].td_end;
			nircm->tcp_rule.return_max_end = ct->proto.tcp.seen[return_dir].td_maxend;
#ifdef ECM_OPENWRT_SUPPORT
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0))
			if (tn->tcp_be_liberal || tn->tcp_no_window_check
#else
			if (tn->tcp_be_liberal || nf_ct_tcp_no_window_check
#endif
#else
			if (tn->tcp_be_liberal
#endif
					|| (ct->proto.tcp.seen[flow_dir].flags & IP_CT_TCP_FLAG_BE_LIBERAL)
					|| (ct->proto.tcp.seen[return_dir].flags & IP_CT_TCP_FLAG_BE_LIBERAL)) {
				nircm->rule_flags |= SFE_RULE_CREATE_FLAG_NO_SEQ_CHECK;
			}
			spin_unlock_bh(&ct->lock);
		}

		nircm->valid_flags |= SFE_RULE_CREATE_TCP_VALID;
	}

	/*
	 * Sync our creation command from the assigned classifiers to get specific additional creation rules.
	 * NOTE: These are called in ascending order of priority and so the last classifier (highest) shall
	 * override any preceding classifiers.
	 * This also gives the classifiers a chance to see that acceleration is being attempted.
	 *
	 * sync_from_v4 is avoided before accelerating a connection for EMESH classifier as it has
	 * callbacks registered with WLAN driver. If SFE fails to create a rule, EMESH
	 * classifier should not trigger these callbacks.
	 */
	assignment_count = ecm_db_connection_classifier_assignments_get_and_ref(feci->ci, assignments);
	for (aci_index = 0; aci_index < assignment_count; ++aci_index) {
		/*
		 * NOTE: The current classifiers do not sync anything to the underlying accel engines.
		 * In the future, if any of the classifiers wants to pass any parameter, these parameters
		 * should be received via this object and copied to the accel engine's create object (nircm).
		 */
		aci = assignments[aci_index];
#ifdef ECM_CLASSIFIER_EMESH_ENABLE
		if ((aci->type_get(aci)) != ECM_CLASSIFIER_TYPE_EMESH) {
			DEBUG_TRACE("%px: sync from: %px, type: %d\n", feci, aci, aci->type_get(aci));
			aci->sync_from_v4(aci, &ecrc);
		}
#else
		DEBUG_TRACE("%px: sync from: %px, type: %d\n", feci, aci, aci->type_get(aci));
		aci->sync_from_v4(aci, &ecrc);
#endif
	}
	ecm_db_connection_assignments_release(assignment_count, assignments);

	/*
	 * Configure qdisc rule and fast xmit settings in the rule
	 */
	if (sfe_is_l2_feature_enabled()) {
		ecm_sfe_common_fast_xmit_set(&nircm->rule_flags, &nircm->valid_flags, &nircm->qdisc_rule, from_ifaces, to_ifaces, from_ifaces_first, to_ifaces_first);
	}

	/*
	 * Release the interface lists
	 */
	ecm_db_connection_interfaces_deref(from_ifaces, from_ifaces_first);
	ecm_db_connection_interfaces_deref(to_ifaces, to_ifaces_first);

	DEBUG_INFO("%px: Ported Accelerate connection %px\n"
			"Protocol: %d\n"
			"from_mtu: %u\n"
			"to_mtu: %u\n"
			"from_ip: %pI4n:%d\n"
			"to_ip: %pI4n:%d\n"
			"from_ip_xlate: %pI4n:%d\n"
			"to_ip_xlate: %pI4n:%d\n"
			"from_mac: %pM\n"
			"to_mac: %pM\n"
			"from_src_mac: %pM\n"
			"to_src_mac: %pM\n"
			"from_iface_num: %u\n"
			"to_iface_num: %u\n"
			"from_top_interface_num: %d\n"
			"to_top_interface_num: %d\n"
			"primary_ingress_vlan_tag: %x\n"
			"primary_egress_vlan_tag: %x\n"
			"secondary_ingress_vlan_tag: %x\n"
			"secondary_egress_vlan_tag: %x\n"
			"flags: rule=%x valid=%x src_mac_valid=%x\n"
			"return_pppoe_session_id: %u\n"
			"return_pppoe_remote_mac: %pM\n"
			"flow_pppoe_session_id: %u\n"
			"flow_pppoe_remote_mac: %pM\n"
			"flow_qos_tag: %x (%u)\n"
			"return_qos_tag: %x (%u)\n"
			"flow_window_scale: %u\n"
			"flow_max_window: %u\n"
			"flow_end: %u\n"
			"flow_max_end: %u\n"
			"return_window_scale: %u\n"
			"return_max_window: %u\n"
			"return_end: %u\n"
			"return_max_end: %u\n"
			"flow_dscp: %x\n"
			"return_dscp: %x\n"
			"flow_mark:%x\n"
			"return_mark:%x\n",
			feci,
			feci->ci,
			nircm->tuple.protocol,
			nircm->conn_rule.flow_mtu,
			nircm->conn_rule.return_mtu,
			&nircm->tuple.flow_ip, ntohs(nircm->tuple.flow_ident),
			&nircm->tuple.return_ip, ntohs(nircm->tuple.return_ident),
			&nircm->conn_rule.flow_ip_xlate, ntohs(nircm->conn_rule.flow_ident_xlate),
			&nircm->conn_rule.return_ip_xlate, ntohs(nircm->conn_rule.return_ident_xlate),
			nircm->conn_rule.flow_mac,
			nircm->conn_rule.return_mac,
			nircm->src_mac_rule.flow_src_mac,
			nircm->src_mac_rule.return_src_mac,
			nircm->conn_rule.flow_interface_num,
			nircm->conn_rule.return_interface_num,
			nircm->conn_rule.flow_top_interface_num,
			nircm->conn_rule.return_top_interface_num,
			nircm->vlan_primary_rule.ingress_vlan_tag,
			nircm->vlan_primary_rule.egress_vlan_tag,
			nircm->vlan_secondary_rule.ingress_vlan_tag,
			nircm->vlan_secondary_rule.egress_vlan_tag,
			nircm->rule_flags,
			nircm->valid_flags,
			nircm->src_mac_rule.mac_valid_flags,
			nircm->pppoe_rule.return_pppoe_session_id,
			nircm->pppoe_rule.return_pppoe_remote_mac,
			nircm->pppoe_rule.flow_pppoe_session_id,
			nircm->pppoe_rule.flow_pppoe_remote_mac,
			nircm->qos_rule.flow_qos_tag, nircm->qos_rule.flow_qos_tag,
			nircm->qos_rule.return_qos_tag, nircm->qos_rule.return_qos_tag,
			nircm->tcp_rule.flow_window_scale,
			nircm->tcp_rule.flow_max_window,
			nircm->tcp_rule.flow_end,
			nircm->tcp_rule.flow_max_end,
			nircm->tcp_rule.return_window_scale,
			nircm->tcp_rule.return_max_window,
			nircm->tcp_rule.return_end,
			nircm->tcp_rule.return_max_end,
			nircm->dscp_rule.flow_dscp,
			nircm->dscp_rule.return_dscp,
			nircm->mark_rule.flow_mark,
			nircm->mark_rule.return_mark);

	if (protocol == IPPROTO_TCP) {

		DEBUG_INFO("flow_window_scale: %u\n"
			"flow_max_window: %u\n"
			"flow_end: %u\n"
			"flow_max_end: %u\n"
			"return_window_scale: %u\n"
			"return_max_window: %u\n"
			"return_end: %u\n"
			"return_max_end: %u\n",
			nircm->tcp_rule.flow_window_scale,
			nircm->tcp_rule.flow_max_window,
			nircm->tcp_rule.flow_end,
			nircm->tcp_rule.flow_max_end,
			nircm->tcp_rule.return_window_scale,
			nircm->tcp_rule.return_max_window,
			nircm->tcp_rule.return_end,
			nircm->tcp_rule.return_max_end);
	}

	/*
	 * Now that the rule has been constructed we re-compare the generation occurrance counter.
	 * If there has been a change then we abort because the rule may have been created using
	 * unstable data - especially if another thread has begun regeneration of the connection state.
	 * NOTE: This does not prevent a regen from being flagged immediately after this line of code either,
	 * or while the acceleration rule is in flight to the nss.
	 * This is only to check for consistency of rule state - not that the state is stale.
	 * Remember that the connection is marked as "accel pending state" so if a regen is flagged immediately
	 * after this check passes, the connection will be decelerated and refreshed very quickly.
	 */
	if (regen_occurrances != ecm_db_connection_regeneration_occurrances_get(feci->ci)) {
		DEBUG_INFO("%px: connection:%px regen occurred - aborting accel rule.\n", feci, feci->ci);
		ecm_sfe_ipv4_accel_pending_clear(feci, ECM_FRONT_END_ACCELERATION_MODE_DECEL);
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
	sfe_tx_status = sfe_ipv4_tx(ecm_sfe_ipv4_mgr, nim);
	if (sfe_tx_status == SFE_TX_SUCCESS) {
		/*
		 * Reset the driver_fail count - transmission was okay here.
		 */
		spin_lock_bh(&feci->lock);
		feci->stats.driver_fail = 0;
		spin_unlock_bh(&feci->lock);

		kfree(nim);

		/*
		 * For emesh classifier sync_from_v4 to be called after rule is successfully created.
		 */
#ifdef ECM_CLASSIFIER_EMESH_ENABLE
		aci = ecm_db_connection_assigned_classifier_find_and_ref(feci->ci, ECM_CLASSIFIER_TYPE_EMESH);
		if (aci) {
			ecrc.skb = skb;
			DEBUG_TRACE("%px: sync from: %px, type: %d\n", feci, aci, aci->type_get(aci));
			aci->sync_from_v4(aci, &ecrc);
			aci->deref(aci);
		}
#endif
		return;
	}

	kfree(nim);

	/*
	 * Release that ref!
	 */
	ecm_db_connection_deref(feci->ci);

	/*
	 * TX failed
	 */
	spin_lock_bh(&feci->lock);
	DEBUG_ASSERT(feci->accel_mode == ECM_FRONT_END_ACCELERATION_MODE_ACCEL_PENDING, "%px: Accel mode unexpected: %d\n", feci, feci->accel_mode);
	feci->stats.driver_fail_total++;
	feci->stats.driver_fail++;
	if (feci->stats.driver_fail >= feci->stats.driver_fail_limit) {
		DEBUG_WARN("%px: Accel failed - driver fail limit\n", feci);
		result_mode = ECM_FRONT_END_ACCELERATION_MODE_FAIL_DRIVER;
	} else {
		result_mode = ECM_FRONT_END_ACCELERATION_MODE_DECEL;
	}

	spin_lock_bh(&ecm_sfe_ipv4_lock);
	_ecm_sfe_ipv4_accel_pending_clear(feci, result_mode);
	spin_unlock_bh(&ecm_sfe_ipv4_lock);

	spin_unlock_bh(&feci->lock);
	return;

ported_accel_bad_rule:
	;

	/*
	 * Jump to here when rule data is bad and an offload command cannot be constructed
	 */
	DEBUG_WARN("%px: Accel failed - bad rule\n", feci);
	ecm_sfe_ipv4_accel_pending_clear(feci, ECM_FRONT_END_ACCELERATION_MODE_FAIL_RULE);
}

/*
 * ecm_sfe_ported_ipv4_connection_destroy_callback()
 *	Callback for handling destroy ack/nack calls.
 */
static void ecm_sfe_ported_ipv4_connection_destroy_callback(void *app_data, struct sfe_ipv4_msg *nim)
{
	struct sfe_ipv4_rule_destroy_msg * __attribute__((unused))nirdm = &nim->msg.rule_destroy;
	uint32_t serial = (uint32_t)(ecm_ptr_t)app_data;
	struct ecm_db_connection_instance *ci;
	struct ecm_front_end_connection_instance *feci;

	/*
	 * Is this a response to a destroy message?
	 */
	if (nim->cm.type != SFE_TX_DESTROY_RULE_MSG) {
		DEBUG_ERROR("%px: ported destroy callback with improper type: %d\n", nim, nim->cm.type);
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

	/*
	 * Record command duration
	 */
	ecm_sfe_ipv4_decel_done_time_update(feci);

	/*
	 * Dump some useful trace information.
	 */
	DEBUG_TRACE("%px: decelerate response for connection: %px\n", feci, feci->ci);
	DEBUG_TRACE("%px: flow_ip: %pI4n:%d\n", feci, &nirdm->tuple.flow_ip, nirdm->tuple.flow_ident);
	DEBUG_TRACE("%px: return_ip: %pI4n:%d\n", feci, &nirdm->tuple.return_ip, nirdm->tuple.return_ident);
	DEBUG_TRACE("%px: protocol: %d\n", feci, nirdm->tuple.protocol);

	/*
	 * Drop decel pending counter
	 */
	spin_lock_bh(&ecm_sfe_ipv4_lock);
	ecm_sfe_ipv4_pending_decel_count--;
	DEBUG_ASSERT(ecm_sfe_ipv4_pending_decel_count >= 0, "Bad decel pending counter\n");
	spin_unlock_bh(&ecm_sfe_ipv4_lock);

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
	 * Ported acceleration ends
	 */
	spin_lock_bh(&ecm_sfe_ipv4_lock);
	ecm_sfe_ported_ipv4_accelerated_count[feci->ported_accelerated_count_index]--;	/* Protocol specific counter */
	DEBUG_ASSERT(ecm_sfe_ported_ipv4_accelerated_count[feci->ported_accelerated_count_index] >= 0, "Bad udp accel counter\n");
	ecm_sfe_ipv4_accelerated_count--;		/* General running counter */
	DEBUG_ASSERT(ecm_sfe_ipv4_accelerated_count >= 0, "Bad accel counter\n");
	spin_unlock_bh(&ecm_sfe_ipv4_lock);

	/*
	 * Release the connections.
	 */
	ecm_front_end_connection_deref(feci);
	ecm_db_connection_deref(ci);
}

/*
 * ecm_sfe_ported_ipv4_connection_decelerate_msg_send()
 *	Prepares and sends a decelerate message to acceleration engine.
 */
static bool ecm_sfe_ported_ipv4_connection_decelerate_msg_send(struct ecm_front_end_connection_instance *feci)
{
	struct sfe_ipv4_msg *nim;
	struct sfe_ipv4_rule_destroy_msg *nirdm;
	ip_addr_t addr;
	sfe_tx_status_t sfe_tx_status;
	bool ret;

	DEBUG_CHECK_MAGIC(feci, ECM_FRONT_END_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", feci);

	nim = (struct sfe_ipv4_msg *)kzalloc(sizeof(struct sfe_ipv4_msg), GFP_ATOMIC | __GFP_NOWARN);
	if (!nim) {
		DEBUG_WARN("%px: no memory for sfe ipv4 message structure instance: %px\n", feci, feci->ci);
		return false;
	}

	/*
	 * Increment the decel pending counter
	 */
	spin_lock_bh(&ecm_sfe_ipv4_lock);
	ecm_sfe_ipv4_pending_decel_count++;
	spin_unlock_bh(&ecm_sfe_ipv4_lock);

	/*
	 * Prepare deceleration message
	 */
	sfe_ipv4_msg_init(nim, SFE_SPECIAL_INTERFACE_IPV4, SFE_TX_DESTROY_RULE_MSG,
			sizeof(struct sfe_ipv4_rule_destroy_msg),
			ecm_sfe_ported_ipv4_connection_destroy_callback,
			(void *)(ecm_ptr_t)ecm_db_connection_serial_get(feci->ci));

	nirdm = &nim->msg.rule_destroy;
	nirdm->tuple.protocol = (int32_t)ecm_db_connection_protocol_get(feci->ci);

	/*
	 * Get addressing information
	 */
	ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_FROM, addr);
	ECM_IP_ADDR_TO_NIN4_ADDR(nirdm->tuple.flow_ip, addr);
	ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_TO_NAT, addr);
	ECM_IP_ADDR_TO_NIN4_ADDR(nirdm->tuple.return_ip, addr);
	nirdm->tuple.flow_ident = htons(ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_FROM));
	nirdm->tuple.return_ident = htons(ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_TO_NAT));

	DEBUG_INFO("%px: Ported Connection %px decelerate\n"
			"protocol: %d\n"
			"src_ip: %pI4:%d\n"
			"dest_ip: %pI4:%d\n",
			feci, feci->ci, nirdm->tuple.protocol,
			&nirdm->tuple.flow_ip, nirdm->tuple.flow_ident,
			&nirdm->tuple.return_ip, nirdm->tuple.return_ident);

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
	sfe_tx_status = sfe_ipv4_tx(ecm_sfe_ipv4_mgr, nim);
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
	spin_lock_bh(&ecm_sfe_ipv4_lock);
	ecm_sfe_ipv4_pending_decel_count--;
	DEBUG_ASSERT(ecm_sfe_ipv4_pending_decel_count >= 0, "Bad decel pending counter\n");
	spin_unlock_bh(&ecm_sfe_ipv4_lock);

	return ret;
}

/*
 * ecm_sfe_ported_ipv4_connection_decelerate()
 *     Decelerate a connection
 */
static bool ecm_sfe_ported_ipv4_connection_decelerate(struct ecm_front_end_connection_instance *feci)
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

	return ecm_sfe_ported_ipv4_connection_decelerate_msg_send(feci);
}

/*
 * ecm_sfe_ported_ipv4_connection_defunct_callback()
 *	Callback to be called when a ported connection has become defunct.
 */
bool ecm_sfe_ported_ipv4_connection_defunct_callback(void *arg, int *accel_mode)
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

	if (!ecm_front_end_common_connection_decelerate_accel_mode_check(feci)) {
		*accel_mode = feci->accel_mode;
		spin_unlock_bh(&feci->lock);
		return false;
	}
	feci->accel_mode = ECM_FRONT_END_ACCELERATION_MODE_DECEL_PENDING;
	spin_unlock_bh(&feci->lock);

	/*
	 * If none of the cases matched above, this means the connection is in the
	 * accel mode, so we force a deceleration.
	 * NOTE: If the mode is accel pending then the decel will be actioned when that is completed.
	 */
	ret = ecm_sfe_ported_ipv4_connection_decelerate_msg_send(feci);

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
 * ecm_sfe_ported_ipv4_connection_accel_ceased()
 *	SFE has indicated that acceleration has stopped.
 *
 * NOTE: This is called in response to an SFE self-initiated termination of acceleration.
 * This must NOT be called because the ECM terminated the acceleration.
 */
static void ecm_sfe_ported_ipv4_connection_accel_ceased(struct ecm_front_end_connection_instance *feci)
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
	 * Ported acceleration ends
	 */
	spin_lock_bh(&ecm_sfe_ipv4_lock);
	ecm_sfe_ported_ipv4_accelerated_count[feci->ported_accelerated_count_index]--;	/* Protocol specific counter */
	DEBUG_ASSERT(ecm_sfe_ported_ipv4_accelerated_count[feci->ported_accelerated_count_index] >= 0, "Bad ported accel counter\n");
	ecm_sfe_ipv4_accelerated_count--;		/* General running counter */
	DEBUG_ASSERT(ecm_sfe_ipv4_accelerated_count >= 0, "Bad accel counter\n");
	spin_unlock_bh(&ecm_sfe_ipv4_lock);
}

#ifdef ECM_STATE_OUTPUT_ENABLE
/*
 * ecm_sfe_ported_ipv4_connection_state_get()
 *	Return state of this ported front end instance
 */
static int ecm_sfe_ported_ipv4_connection_state_get(struct ecm_front_end_connection_instance *feci, struct ecm_state_file_instance *sfi)
{
	DEBUG_CHECK_MAGIC(feci, ECM_FRONT_END_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", feci);

	return ecm_front_end_common_connection_state_get(feci, sfi, "sfe_v4.ported");
}
#endif

/*
 * ecm_sfe_ported_ipv4_connection_set();
 *	Sets the SFE IPv4 ported connection's fields.
 */
void ecm_sfe_ported_ipv4_connection_set(struct ecm_front_end_connection_instance *feci, uint32_t flags)
{
	feci->accel_engine = ECM_FRONT_END_ENGINE_SFE;
	feci->stats.no_action_seen_limit = ecm_sfe_ipv4_no_action_limit_default;
	feci->stats.driver_fail_limit = ecm_sfe_ipv4_driver_fail_limit_default;
	feci->stats.ae_nack_limit = ecm_sfe_ipv4_nack_limit_default;
	feci->accelerate = ecm_sfe_ported_ipv4_connection_accelerate;
	feci->decelerate = ecm_sfe_ported_ipv4_connection_decelerate;
	feci->accel_ceased = ecm_sfe_ported_ipv4_connection_accel_ceased;
#ifdef ECM_STATE_OUTPUT_ENABLE
	feci->state_get = ecm_sfe_ported_ipv4_connection_state_get;
#endif
	feci->ae_interface_number_by_dev_get = ecm_sfe_common_get_interface_number_by_dev;
	feci->ae_interface_number_by_dev_type_get = ecm_sfe_common_get_interface_number_by_dev_type;
	feci->ae_interface_type_get = ecm_sfe_common_get_interface_type;
	feci->regenerate = ecm_sfe_common_connection_regenerate;
	feci->defunct = ecm_sfe_ported_ipv4_connection_defunct_callback;

	ecm_sfe_common_init_fe_info(&feci->fe_info);

	feci->get_stats_bitmap = ecm_front_end_common_get_stats_bitmap;
	feci->set_stats_bitmap = ecm_front_end_common_set_stats_bitmap;
	feci->fe_info.front_end_flags = flags;

	/*
	 * Just in case this function is called while switching AE to SFE
	 * let's reset the failure stats which was increased by the old AE.
	 */
        feci->stats.driver_fail_total = 0;
        feci->stats.driver_fail = 0;
        feci->stats.ae_nack = 0;
        feci->stats.ae_nack_total = 0;
}

/*
 * ecm_sfe_ported_ipv4_connection_instance_alloc()
 *	Create a front end instance specific for ported connection
 */
struct ecm_front_end_connection_instance *ecm_sfe_ported_ipv4_connection_instance_alloc(
								uint32_t accel_flags,
								int protocol,
								struct ecm_db_connection_instance **nci)
{
	struct ecm_front_end_connection_instance *feci;
	struct ecm_db_connection_instance *ci;
	bool can_accel = (accel_flags & ECM_FRONT_END_ENGINE_FLAG_CAN_ACCEL);

	if (ecm_sfe_ipv4_is_conn_limit_reached()) {
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
		ecm_db_connection_deref(ci);
		DEBUG_WARN("Ported Front end alloc failed\n");
		return NULL;
	}

	/*
	 * Refs is 1 for the creator of the connection
	 */
	feci->refs = 1;
	DEBUG_SET_MAGIC(feci, ECM_FRONT_END_CONNECTION_INSTANCE_MAGIC);
	spin_lock_init(&feci->lock);

	feci->can_accel = can_accel;
	feci->accel_mode = (can_accel)? ECM_FRONT_END_ACCELERATION_MODE_DECEL : ECM_FRONT_END_ACCELERATION_MODE_FAIL_DENIED;

	/*
	 * Copy reference to connection - no need to ref ci as ci maintains a ref to this instance instead (this instance persists for as long as ci does)
	 */
	feci->ci = ci;

	feci->ip_version = 4;

	feci->protocol = protocol;

	feci->update_rule = ecm_sfe_common_update_rule;
	ecm_sfe_ported_ipv4_connection_set(feci, accel_flags);

	if (protocol == IPPROTO_TCP) {
		feci->ported_accelerated_count_index = ECM_FRONT_END_PORTED_PROTO_TCP;
	} else if (protocol == IPPROTO_UDP) {
		feci->ported_accelerated_count_index = ECM_FRONT_END_PORTED_PROTO_UDP;
	} else {
		DEBUG_WARN("%px: Wrong protocol: %d\n", feci, protocol);
		DEBUG_CLEAR_MAGIC(feci);
		kfree(feci);
		ecm_db_connection_deref(ci);
		return NULL;
	}

	return feci;
}

/*
 * ecm_sfe_ported_ipv4_debugfs_init()
 */
bool ecm_sfe_ported_ipv4_debugfs_init(struct dentry *dentry)
{
	debugfs_create_u32("udp_accelerated_count", S_IRUGO, dentry,
						&ecm_sfe_ported_ipv4_accelerated_count[ECM_FRONT_END_PORTED_PROTO_UDP]);

	debugfs_create_u32("tcp_accelerated_count", S_IRUGO, dentry,
					&ecm_sfe_ported_ipv4_accelerated_count[ECM_FRONT_END_PORTED_PROTO_TCP]);

	return true;
}
