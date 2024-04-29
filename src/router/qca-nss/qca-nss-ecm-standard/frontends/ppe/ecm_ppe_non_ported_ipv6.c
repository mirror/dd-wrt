/*
 **************************************************************************
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
#define DEBUG_LEVEL ECM_PPE_NON_PORTED_IPV6_DEBUG_LEVEL

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
#include "ecm_ppe_non_ported_ipv6.h"
#include "ecm_ppe_ipv6.h"
#include "ecm_ppe_common.h"
#include "ecm_front_end_common.h"

static int ecm_ppe_non_ported_ipv6_accelerated_count;		/* Number of Non-Ported connections currently offloaded */

/*
 * ecm_ppe_non_ported_ipv6_handle_flush()
 *	Handle situation if destroy comes before setting up rule
 *	feci->lock is caller's responsibility.
 */
static void ecm_ppe_non_ported_ipv6_handle_flush(struct ecm_front_end_connection_instance *feci)
{
	struct ecm_db_connection_instance *ci = feci->ci;
	DEBUG_ASSERT(feci->accel_mode == ECM_FRONT_END_ACCELERATION_MODE_ACCEL_PENDING, "%px: Unexpected mode: %d\n", ci, feci->accel_mode);

	feci->stats.flush_happened = false;

	/*
	 * Increment the no-action counter. Our connection was decelerated on us with no action occurring.
	 */
	feci->stats.no_action_seen++;

	spin_lock_bh(&ecm_ppe_ipv6_lock);
	_ecm_ppe_ipv6_accel_pending_clear(feci, ECM_FRONT_END_ACCELERATION_MODE_DECEL);
	spin_unlock_bh(&ecm_ppe_ipv6_lock);
}

/*
 * ecm_ppe_non_ported_ipv6_accelerate_done()
 *	Acceleration rule pushed successfully
 *	feci->lock is caller's responsibility.
 */
static bool ecm_ppe_non_ported_ipv6_accelerate_done(struct ecm_front_end_connection_instance *feci)
{
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
	spin_lock_bh(&ecm_ppe_ipv6_lock);

	ecm_ppe_non_ported_ipv6_accelerated_count++;	/* Protocol specific counter */
	ecm_ppe_ipv6_accelerated_count++;		/* General running counter */

	if (!_ecm_ppe_ipv6_accel_pending_clear(feci, ECM_FRONT_END_ACCELERATION_MODE_ACCEL)) {
		/*
		 * Increment the no-action counter, this is reset if offload action is seen
		 */
		DEBUG_TRACE("%px: success(IPv6/non-ported): feci->accel_mode: %d\n", feci, feci->accel_mode);
		feci->stats.no_action_seen++;
		spin_unlock_bh(&ecm_ppe_ipv6_lock);
		return true;
	}

	spin_unlock_bh(&ecm_ppe_ipv6_lock);
	DEBUG_TRACE("%px: fail(IPv6/non-ported): feci->accel_mode: %d\n", feci, feci->accel_mode);

	return false;
}

/*
 * ecm_ppe_non_ported_ipv6_connection_accelerate()
 *	Accelerate a connection
 */
static void ecm_ppe_non_ported_ipv6_connection_accelerate(struct ecm_front_end_connection_instance *feci,
								struct ecm_classifier_process_response *pr, bool is_l2_encap,
								struct nf_conn *ct, struct sk_buff *skb)
{
	uint16_t regen_occurrances;
	int32_t from_ifaces_first;
	int32_t to_ifaces_first;
	struct ecm_db_iface_instance *from_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX];
	struct ecm_db_iface_instance *to_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX];
	struct ecm_db_iface_instance *from_ppe_iface;
	struct ecm_db_iface_instance *to_ppe_iface;
	int32_t from_ppe_iface_id;
	int32_t to_ppe_iface_id;
	uint8_t from_ppe_iface_address[ETH_ALEN];
	uint8_t to_ppe_iface_address[ETH_ALEN];
	struct ppe_drv_v6_rule_create *pd6rc;
	struct ecm_classifier_instance *assignments[ECM_CLASSIFIER_TYPES];
	int aci_index;
	int assignment_count;
	ppe_drv_ret_t ppe_tx_status;
	int32_t list_index;
	int32_t interface_type_counts[ECM_DB_IFACE_TYPE_COUNT];
	bool rule_invalid;
	ip_addr_t src_ip;
	ip_addr_t dest_ip;
	bool is_defunct = false;
	ecm_front_end_acceleration_mode_t result_mode;
#if defined(ECM_INTERFACE_GRE_TAP_ENABLE) || defined(ECM_INTERFACE_GRE_TUN_ENABLE)
	struct net_device *dev;
#endif

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
	if (!ecm_ppe_ipv6_accel_pending_set(feci)) {
		DEBUG_TRACE("%px: Acceleration not permitted: %px\n", feci, feci->ci);
		return;
	}

	pd6rc = (struct ppe_drv_v6_rule_create *)kzalloc(sizeof(struct ppe_drv_v6_rule_create), GFP_ATOMIC | __GFP_NOWARN);
	if (!pd6rc) {
		DEBUG_WARN("%px: no memory for ppe ipv6 message structure instance: %px\n", feci, feci->ci);
		ecm_ppe_ipv6_accel_pending_clear(feci, ECM_FRONT_END_ACCELERATION_MODE_DECEL);
		return;
	}

	pd6rc->valid_flags = 0;
	pd6rc->rule_flags = 0;

	/*
	 * Initialize VLAN tag information
	 */
	pd6rc->vlan_rule.primary_vlan.ingress_vlan_tag = ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED;
	pd6rc->vlan_rule.primary_vlan.egress_vlan_tag = ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED;
	pd6rc->vlan_rule.secondary_vlan.ingress_vlan_tag = ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED;
	pd6rc->vlan_rule.secondary_vlan.egress_vlan_tag = ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED;

	/*
	 * Get the interface lists of the connection, we must have at least one interface in the list to continue
	 */
	from_ifaces_first = ecm_db_connection_interfaces_get_and_ref(feci->ci, from_ifaces, ECM_DB_OBJ_DIR_FROM);
	if (from_ifaces_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
		DEBUG_WARN("%px: Accel attempt failed - no interfaces in from_interfaces list!\n", feci);
		goto non_ported_accel_bad_rule;
	}

	to_ifaces_first = ecm_db_connection_interfaces_get_and_ref(feci->ci, to_ifaces, ECM_DB_OBJ_DIR_TO);
	if (to_ifaces_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
		DEBUG_WARN("%px: Accel attempt failed - no interfaces in to_interfaces list!\n", feci);
		ecm_db_connection_interfaces_deref(from_ifaces, from_ifaces_first);
		goto non_ported_accel_bad_rule;
	}

	/*
	 * First interface in each must be a known ppe interface
	 */
	from_ppe_iface = from_ifaces[from_ifaces_first];
	to_ppe_iface = to_ifaces[to_ifaces_first];
	from_ppe_iface_id = ecm_ppe_common_get_ae_iface_id_by_netdev_id(ecm_db_iface_interface_identifier_get(from_ppe_iface));
	to_ppe_iface_id = ecm_ppe_common_get_ae_iface_id_by_netdev_id(ecm_db_iface_interface_identifier_get(to_ppe_iface));
	if ((from_ppe_iface_id < 0) || (to_ppe_iface_id < 0)) {
		ecm_db_connection_interfaces_deref(from_ifaces, from_ifaces_first);
		ecm_db_connection_interfaces_deref(to_ifaces, to_ifaces_first);
		DEBUG_TRACE("%px: from_ppe_iface_id: %d, to_ppe_iface_id: %d\n", feci, from_ppe_iface_id, to_ppe_iface_id);
		goto non_ported_accel_bad_rule;
	}

	/*
	 * Get PPE interface ID of the top interface in heirarchy
	 */
	from_ppe_iface = from_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX - 1];
	to_ppe_iface = to_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX - 1];
	pd6rc->top_rule.rx_if = ecm_ppe_common_get_ae_iface_id_by_netdev_id(ecm_db_iface_interface_identifier_get(from_ppe_iface));
	pd6rc->top_rule.tx_if = ecm_ppe_common_get_ae_iface_id_by_netdev_id(ecm_db_iface_interface_identifier_get(to_ppe_iface));
	if ((pd6rc->top_rule.rx_if < 0) || (pd6rc->top_rule.tx_if < 0)) {
		ecm_db_connection_interfaces_deref(from_ifaces, from_ifaces_first);
		ecm_db_connection_interfaces_deref(to_ifaces, to_ifaces_first);
		DEBUG_TRACE("%px: Accel attempt failed - TOP interfaces for 'from'(%d) and 'to'(%d) interfaces list!\n", feci, pd6rc->top_rule.rx_if, pd6rc->top_rule.tx_if);
		goto non_ported_accel_bad_rule;
	}

	/*
	 * Set interface numbers involved in accelerating this connection.
	 * These are the outer facing addresses from the heirarchy interface lists we got above.
	 * These may be overridden later if we detect special interface types e.g. ipsec.
	 */
	pd6rc->conn_rule.rx_if = from_ppe_iface_id;
	pd6rc->conn_rule.tx_if = to_ppe_iface_id;

	/*
	 * Set up the flow and return qos tags
	 */
	if (pr->process_actions & ECM_CLASSIFIER_PROCESS_ACTION_QOS_TAG) {
		pd6rc->qos_rule.flow_qos_tag = (uint32_t)pr->flow_qos_tag;
		pd6rc->qos_rule.return_qos_tag = (uint32_t)pr->return_qos_tag;
		pd6rc->valid_flags |= PPE_DRV_V6_VALID_FLAG_QOS;
	}

	/*
	 * Set the mtu values. These values will be overwritten if the flow is
	 * a specific tunnel type.
	 */
	pd6rc->conn_rule.flow_mtu = (uint32_t)ecm_db_connection_iface_mtu_get(feci->ci, ECM_DB_OBJ_DIR_FROM);
	pd6rc->conn_rule.return_mtu = (uint32_t)ecm_db_connection_iface_mtu_get(feci->ci, ECM_DB_OBJ_DIR_TO);

	/*
	 * We know that each outward facing interface is known to the PPE and so this connection could be accelerated.
	 * However the lists may also specify other interesting details that must be included in the creation command,
	 * for example, ethernet MAC, VLAN tagging or PPPoE session information.
	 * We get this information by walking from the outer to the innermost interface for each list and examine the interface types.
	 *
	 * Start with the 'from' (src) side.
	 * NOTE: The lists may contain a complex heirarchy of similar type of interface e.g. multiple vlans or tunnels within tunnels.
	 * This PPE cannot handle that - there is no way to describe this in the rule - if we see multiple types that would conflict we have to abort.
	 */
	DEBUG_TRACE("%px: Examine from/src heirarchy list\n", feci);
	memset(interface_type_counts, 0, sizeof(interface_type_counts));
	rule_invalid = false;
	for (list_index = from_ifaces_first; !rule_invalid && (list_index < ECM_DB_IFACE_HEIRARCHY_MAX); list_index++) {
		struct ecm_db_iface_instance *ii;
		uint32_t iface_id, ae_iface_id;
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
#if defined(ECM_INTERFACE_GRE_TAP_ENABLE) || defined(ECM_INTERFACE_GRE_TUN_ENABLE)
		ip_addr_t saddr;
		ip_addr_t daddr;
#endif
		ii = from_ifaces[list_index];
		ii_type = ecm_db_iface_type_get(ii);
		ii_name = ecm_db_interface_type_to_string(ii_type);
		iface_id = ecm_db_iface_interface_identifier_get(ii);
		ae_iface_id = ecm_ppe_common_get_ae_iface_id_by_netdev_id(ecm_db_iface_interface_identifier_get(ii));
		DEBUG_TRACE("%px: list_index: %d, ii: %px, type: %d (%s) ae_iface_id(%d)\n", feci, list_index, ii, ii_type, ii_name, ae_iface_id);

		if (ae_iface_id < 0) {
			DEBUG_TRACE("%px: PPE doesn't support iface_id:(%d) type:%d(%s) interface",
					feci, iface_id, ii_type, ii_name);
			ecm_db_connection_interfaces_deref(from_ifaces, from_ifaces_first);
			ecm_db_connection_interfaces_deref(to_ifaces, to_ifaces_first);
			goto non_ported_accel_bad_rule;
		}

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

			ecm_db_iface_bridge_address_get(ii, from_ppe_iface_address);
			DEBUG_TRACE("%px: Bridge - mac: %pM\n", feci, from_ppe_iface_address);
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

			ecm_db_iface_ovs_bridge_address_get(ii, from_ppe_iface_address);
			DEBUG_TRACE("%px: OVS Bridge - mac: %pM\n", feci, from_ppe_iface_address);
#else
			rule_invalid = true;
			DEBUG_TRACE("%px: OVS Bridge - not supported\n", feci);
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
#ifdef ECM_INTERFACE_GRE_TAP_ENABLE
			dev = dev_get_by_index(&init_net, ecm_db_iface_interface_identifier_get(ii));
			if (dev) {
				if (dev->priv_flags_ext & IFF_EXT_GRE_V6_TAP) {
					ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_FROM, saddr);
					ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_TO, daddr);
					if (!ecm_interface_tunnel_mtu_update(saddr, daddr, ECM_DB_IFACE_TYPE_GRE_TAP, (uint32_t *)&(pd6rc->conn_rule.flow_mtu))) {
						rule_invalid = true;
						DEBUG_WARN("%px: Unable to get mtu value for the GRE TAP interface\n", feci);
					}
				}
				dev_put(dev);
			}
#endif
			/*
			 * Can only handle one MAC, the first outermost mac.
			 */
			ecm_db_iface_ethernet_address_get(ii, from_ppe_iface_address);
			DEBUG_TRACE("%px: Ethernet - mac: %pM\n", feci, from_ppe_iface_address);
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
			 * Copy pppoe session info to the creation structure.
			 */
			ecm_db_iface_pppoe_session_info_get(ii, &pppoe_info);
			pd6rc->pppoe_rule.flow_session.session_id = pppoe_info.pppoe_session_id;
			memcpy(pd6rc->pppoe_rule.flow_session.server_mac, pppoe_info.remote_mac, ETH_ALEN);
			pd6rc->valid_flags |= PPE_DRV_V6_VALID_FLAG_PPPOE_FLOW;


			DEBUG_TRACE("%px: PPPoE - session: %x, remote_mac: %pM\n", feci,
					pd6rc->pppoe_rule.flow_session.session_id,
					pd6rc->pppoe_rule.flow_session.server_mac);
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
				pd6rc->vlan_rule.primary_vlan.ingress_vlan_tag = vlan_value;
			} else {
				pd6rc->vlan_rule.secondary_vlan.ingress_vlan_tag = vlan_value;
			}
			pd6rc->valid_flags |= PPE_DRV_V6_VALID_FLAG_VLAN;

			/*
			 * If we have not yet got an ethernet mac then take this one (very unlikely as mac should have been propagated to the slave (outer) device
			 */
			memcpy(from_ppe_iface_address, vlan_info.address, ETH_ALEN);
			DEBUG_TRACE("%px: vlan tag: %x\n", feci, vlan_value);
#else
			rule_invalid = true;
			DEBUG_TRACE("%px: VLAN - unsupported\n", feci);
#endif
			break;

		case ECM_DB_IFACE_TYPE_IPSEC_TUNNEL:
#ifndef ECM_INTERFACE_IPSEC_ENABLE
			rule_invalid = true;
			DEBUG_TRACE("%px: IPSEC - unsupported\n", feci);
#endif
			break;

		case ECM_DB_IFACE_TYPE_TUNIPIP6:
#ifdef ECM_INTERFACE_TUNIPIP6_ENABLE
			/*
			 * TODO: Clear QOS_VALID to prevent outer rule from overwriting
			 * inner flow's QoS classification.
			 */
			DEBUG_TRACE("%px: Ignoring check for TUNIPIP6: %d (%s)\n", feci, ii_type, ii_name);
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
		goto non_ported_accel_bad_rule;
	}

	/*
	 * Now examine the TO / DEST heirarchy list to construct the destination part of the rule
	 */
	DEBUG_TRACE("%px: Examine to/dest heirarchy list\n", feci);
	memset(interface_type_counts, 0, sizeof(interface_type_counts));
	rule_invalid = false;
	for (list_index = to_ifaces_first; !rule_invalid && (list_index < ECM_DB_IFACE_HEIRARCHY_MAX); list_index++) {
		struct ecm_db_iface_instance *ii;
		uint32_t iface_id, ae_iface_id;
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
		iface_id = ecm_db_iface_interface_identifier_get(ii);
		ae_iface_id = ecm_ppe_common_get_ae_iface_id_by_netdev_id(ecm_db_iface_interface_identifier_get(ii));
		DEBUG_TRACE("%px: list_index: %d, ii: %px(%d), type: %d (%s), ae_iface_id(%d)\n",
				feci, list_index, ii, iface_id, ii_type, ii_name, ae_iface_id);

		if (ae_iface_id < 0) {
			DEBUG_TRACE("%px: PPE doesn't support iface_id:(%d) type:%d(%s) interface",
					feci, iface_id, ii_type, ii_name);
			ecm_db_connection_interfaces_deref(from_ifaces, from_ifaces_first);
			ecm_db_connection_interfaces_deref(to_ifaces, to_ifaces_first);
			goto non_ported_accel_bad_rule;
		}

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

			ecm_db_iface_bridge_address_get(ii, to_ppe_iface_address);
			DEBUG_TRACE("%px: Bridge - mac: %pM\n", feci, to_ppe_iface_address);
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

			ecm_db_iface_ovs_bridge_address_get(ii, to_ppe_iface_address);
			DEBUG_TRACE("%px: OVS Bridge - mac: %pM\n", feci, to_ppe_iface_address);
#else
			rule_invalid = true;
			DEBUG_TRACE("%px: OVS Bridge - not supported\n", feci);
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
			ecm_db_iface_ethernet_address_get(ii, to_ppe_iface_address);
			DEBUG_TRACE("%px: Ethernet - mac: %pM\n", feci, to_ppe_iface_address);
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
			 * Copy pppoe session info to the creation structure.
			 */
			ecm_db_iface_pppoe_session_info_get(ii, &pppoe_info);
			pd6rc->pppoe_rule.return_session.session_id = pppoe_info.pppoe_session_id;
			memcpy(pd6rc->pppoe_rule.return_session.server_mac, pppoe_info.remote_mac, ETH_ALEN);
			pd6rc->valid_flags |= PPE_DRV_V6_VALID_FLAG_PPPOE_RETURN;

			DEBUG_TRACE("%px: PPPoE - session: %x, remote_mac: %pM\n", feci,
					pd6rc->pppoe_rule.return_session.session_id,
					pd6rc->pppoe_rule.return_session.server_mac);
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
				pd6rc->vlan_rule.primary_vlan.egress_vlan_tag = vlan_value;
			} else {
				pd6rc->vlan_rule.secondary_vlan.egress_vlan_tag = vlan_value;
			}

			pd6rc->valid_flags |= PPE_DRV_V6_VALID_FLAG_VLAN;

			/*
			 * If we have not yet got an ethernet mac then take this one (very unlikely as mac should have been propagated to the slave (outer) device
			 */
			memcpy(to_ppe_iface_address, vlan_info.address, ETH_ALEN);
			DEBUG_TRACE("%px: vlan tag: %x\n", feci, vlan_value);
#else
			rule_invalid = true;
			DEBUG_TRACE("%px: VLAN - unsupported\n", feci);
#endif
			break;

		case ECM_DB_IFACE_TYPE_IPSEC_TUNNEL:
#ifndef ECM_INTERFACE_IPSEC_ENABLE
			rule_invalid = true;
			DEBUG_TRACE("%px: IPSEC - unsupported\n", feci);
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
		goto non_ported_accel_bad_rule;
	}

	/*
	 * Routed or bridged?
	 */
	if (ecm_db_connection_is_routed_get(feci->ci)) {
		pd6rc->rule_flags |= PPE_DRV_V6_RULE_FLAG_ROUTED_FLOW;
	} else {
		pd6rc->rule_flags |= PPE_DRV_V6_RULE_FLAG_BRIDGE_FLOW;
	}

	if (ecm_interface_src_check || ecm_db_connection_is_pppoe_bridged_get(feci->ci)) {
		DEBUG_INFO("%px: Source interface check is done\n", feci);
	}

#ifdef ECM_CLASSIFIER_DSCP_ENABLE
	/*
	 * DSCP information?
	 */
	if (pr->process_actions & ECM_CLASSIFIER_PROCESS_ACTION_DSCP) {
		pd6rc->dscp_rule.flow_dscp = pr->flow_dscp;
		pd6rc->dscp_rule.return_dscp = pr->return_dscp;
		pd6rc->rule_flags |= PPE_DRV_V6_RULE_FLAG_DSCP_MARKING;
		pd6rc->valid_flags |= PPE_DRV_V6_VALID_FLAG_DSCP_MARKING;
	}
#endif

	if (ecm_ppe_ipv6_vlan_passthrough_enable && !ecm_db_connection_is_routed_get(feci->ci) &&
		(pd6rc->vlan_rule.primary_vlan.ingress_vlan_tag == ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED) &&
		(pd6rc->vlan_rule.primary_vlan.egress_vlan_tag == ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED)) {
		int vlan_present = 0;
		vlan_present = skb_vlan_tag_present(skb);
		if (vlan_present) {
			uint32_t vlan_value;
			vlan_value = (ETH_P_8021Q << 16) | skb_vlan_tag_get(skb);
			pd6rc->vlan_rule.primary_vlan.ingress_vlan_tag = vlan_value;
			pd6rc->vlan_rule.primary_vlan.egress_vlan_tag = vlan_value;
			pd6rc->valid_flags |= PPE_DRV_V6_VALID_FLAG_VLAN;
		}
	}

#ifdef ECM_CLASSIFIER_OVS_ENABLE
	/*
	 * Copy both primary and secondary (if exist) VLAN tags.
	 */
	if (pr->process_actions & ECM_CLASSIFIER_PROCESS_ACTION_OVS_VLAN_TAG) {
		pd6rc->vlan_rule.primary_vlan.ingress_vlan_tag = pr->ingress_vlan_tag[0];
		pd6rc->vlan_rule.primary_vlan.egress_vlan_tag = pr->egress_vlan_tag[0];
		pd6rc->valid_flags |= PPE_DRV_V6_VALID_FLAG_VLAN;
	}

	if (pr->process_actions & ECM_CLASSIFIER_PROCESS_ACTION_OVS_VLAN_QINQ_TAG) {
		pd6rc->vlan_rule.secondary_vlan.ingress_vlan_tag = pr->ingress_vlan_tag[1];
		pd6rc->vlan_rule.secondary_vlan.egress_vlan_tag = pr->egress_vlan_tag[1];
	}
#endif

	/*
	 * Set protocol
	 */
	pd6rc->tuple.protocol = (int32_t)ecm_db_connection_protocol_get(feci->ci);

	/*
	 * The flow_ip is where the connection established from
	 */
	ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_FROM, src_ip);
	ECM_IP_ADDR_TO_PPE_IPV6_ADDR(pd6rc->tuple.flow_ip, src_ip);

	/*
	 * The return_ip is where the connection is established to
	 */
	ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_TO, dest_ip);
	ECM_IP_ADDR_TO_PPE_IPV6_ADDR(pd6rc->tuple.return_ip, dest_ip);

	/*
	 * Same approach as above for port information
	 */
	pd6rc->tuple.flow_ident = ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_FROM);
	pd6rc->tuple.return_ident = ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_TO_NAT);

	/*
	 * Get mac addresses.
	 * The src_mac is the mac address of the node that established the connection.
	 * This will work whether the from_node is LAN (egress) or WAN (ingress).
	 */
	ecm_db_connection_node_address_get(feci->ci, ECM_DB_OBJ_DIR_FROM, (uint8_t *)pd6rc->conn_rule.flow_mac);

	/*
	 * The dest_mac is the mac address of the node that the connection is esatblished to.
	 */
	ecm_db_connection_node_address_get(feci->ci, ECM_DB_OBJ_DIR_TO_NAT, (uint8_t *)pd6rc->conn_rule.return_mac);

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
		 * should be received via this object and copied to the accel engine's create object (pd6rc).
		*/
		aci = assignments[aci_index];
		DEBUG_TRACE("%px: sync from: %px, type: %d\n", feci, aci, aci->type_get(aci));
		aci->sync_from_v6(aci, &ecrc);
	}
	ecm_db_connection_assignments_release(assignment_count, assignments);

	/*
	 * Release the interface lists
	 */
	ecm_db_connection_interfaces_deref(from_ifaces, from_ifaces_first);
	ecm_db_connection_interfaces_deref(to_ifaces, to_ifaces_first);

	DEBUG_INFO("%px: NON_PORTED Accelerate connection %px\n"
			"Protocol: %d\n"
			"from_mtu: %u\n"
			"to_mtu: %u\n"
			"from_ip: " ECM_IP_ADDR_OCTAL_FMT ":%d\n"
			"to_ip: " ECM_IP_ADDR_OCTAL_FMT ":%d\n"
			"from_mac: %pM\n"
			"to_mac: %pM\n"
			"src_iface_num: %u\n"
			"dest_iface_num: %u\n"
			"src_nexthop_num: %u\n"
			"dest_nexthop_num: %u\n"
			"ingress_inner_vlan_tag: %x\n"
			"egress_inner_vlan_tag: %x\n"
			"ingress_outer_vlan_tag: %x\n"
			"egress_outer_vlan_tag: %x\n"
			"rule_flags: %x\n"
			"valid_flags: %x\n"
			"flow_pppoe_session_id: %u\n"
			"flow_pppoe_remote_mac: %pM\n"
			"return_pppoe_session_id: %u\n"
			"return_pppoe_remote_mac: %pM\n"
			"flow_qos_tag: %x (%u)\n"
			"return_qos_tag: %x (%u)\n"
			"flow_dscp: %x\n"
			"return_dscp: %x\n"
			"conn_rule.rx_if: %d (from iface first:%s)\n"
			"conn_rule.tx_if: %d (to iface first:%s)\n",
			feci,
			feci->ci,
			pd6rc->tuple.protocol,
			pd6rc->conn_rule.flow_mtu,
			pd6rc->conn_rule.return_mtu,
			ECM_IP_ADDR_TO_OCTAL(src_ip), pd6rc->tuple.flow_ident,
			ECM_IP_ADDR_TO_OCTAL(dest_ip), pd6rc->tuple.return_ident,
			pd6rc->conn_rule.flow_mac,
			pd6rc->conn_rule.return_mac,
			pd6rc->conn_rule.rx_if,
			pd6rc->conn_rule.tx_if,
			pd6rc->top_rule.rx_if,
			pd6rc->top_rule.tx_if,
			pd6rc->vlan_rule.primary_vlan.ingress_vlan_tag,
			pd6rc->vlan_rule.primary_vlan.egress_vlan_tag,
			pd6rc->vlan_rule.secondary_vlan.ingress_vlan_tag,
			pd6rc->vlan_rule.secondary_vlan.egress_vlan_tag,
			pd6rc->rule_flags,
			pd6rc->valid_flags,
			pd6rc->pppoe_rule.flow_session.session_id,
			pd6rc->pppoe_rule.flow_session.server_mac,
			pd6rc->pppoe_rule.return_session.session_id,
			pd6rc->pppoe_rule.return_session.server_mac,
			pd6rc->qos_rule.flow_qos_tag, pd6rc->qos_rule.flow_qos_tag,
			pd6rc->qos_rule.return_qos_tag, pd6rc->qos_rule.return_qos_tag,
			pd6rc->dscp_rule.flow_dscp,
			pd6rc->dscp_rule.return_dscp,
			pd6rc->conn_rule.rx_if, (from_ifaces[from_ifaces_first])->name,
			pd6rc->conn_rule.tx_if, (to_ifaces[to_ifaces_first])->name);

	/*
	 * Now that the rule has been constructed we re-compare the generation occurrance counter.
	 * If there has been a change then we abort because the rule may have been created using
	 * unstable data - especially if another thread has begun regeneration of the connection state.
	 * NOTE: This does not prevent a regen from being flagged immediately after this line of code either,
	 * or while the acceleration rule is in flight to the ppe.
	 * This is only to check for consistency of rule state - not that the state is stale.
	 * Remember that the connection is marked as "accel pending state" so if a regen is flagged immediately
	 * after this check passes, the connection will be decelerated and refreshed very quickly.
	 */
	if (regen_occurrances != ecm_db_connection_regeneration_occurrances_get(feci->ci)) {
		DEBUG_INFO("%px: connection:%px regen occurred - aborting accel rule.\n", feci, feci->ci);
		ecm_ppe_ipv6_accel_pending_clear(feci, ECM_FRONT_END_ACCELERATION_MODE_DECEL);
		kfree(pd6rc);
		return;
	}

	/*
	 * Ref the connection before issuing an PPE rule
	 * NOTE: remember that this will also implicitly hold the feci.
	 */
	ecm_db_connection_ref(feci->ci);

	/*
	 * We are about to issue the command, record the time of transmission
	 */
	spin_lock_bh(&feci->lock);
	feci->stats.cmd_time_begun = jiffies;
	spin_unlock_bh(&feci->lock);

	pd6rc->rule_flags |= PPE_DRV_V6_RULE_FLAG_RETURN_VALID | PPE_DRV_V6_RULE_FLAG_FLOW_VALID;

	if (feci->fe_info.front_end_flags & ECM_FRONT_END_ENGINE_FLAG_PPE_DS) {
		pd6rc->rule_flags |= PPE_DRV_V6_RULE_FLAG_DS_FLOW;
	}

	if (feci->fe_info.front_end_flags & ECM_FRONT_END_ENGINE_FLAG_PPE_VP) {
		pd6rc->rule_flags |= PPE_DRV_V6_RULE_FLAG_VP_FLOW;
	}

	DEBUG_TRACE("%px: ECM IPv6 Non-Ported Rule ready to be pushed in PPE:%px\n", feci, feci->ci);

	/*
	 * Call the rule create function
	 */
	ppe_tx_status = ppe_drv_v6_create(pd6rc);
	if (ppe_tx_status == PPE_DRV_RET_SUCCESS) {
		ecm_ppe_ipv6_accel_done_time_update(feci);

		/*
		 * Take feci->lock till completion of rule create in ECM.
		 */
		spin_lock_bh(&feci->lock);

		/*
		 * Reset the driver_fail count - transmission was okay here.
		 */
		feci->stats.driver_fail = 0;

		if (feci->stats.flush_happened) {
			DEBUG_TRACE("flush happened (IPv6/non-ported)\n");
			ecm_ppe_non_ported_ipv6_handle_flush(feci);

			/*
			 * Release feci->lock taken above.
			 */
			spin_unlock_bh(&feci->lock);

			ecm_db_connection_deref(feci->ci);
			kfree(pd6rc);
			return;
		}

		if (!ecm_ppe_non_ported_ipv6_accelerate_done(feci)) {
			/*
			 * Connection couldn't be accelerated successfully, as decelerate was pending.
			 */

			DEBUG_INFO("%px: Decelerate was pending(IPv6/non-ported) %p\n", feci, feci->ci);

			/*
			 * Check if the pending decelerate was done with the defunct process.
			 * If it was, set the is_defunct flag of the feci to false for re-try.
			 */
			if (feci->is_defunct) {
				is_defunct = feci->is_defunct;
				feci->is_defunct = false;
			}

			/*
			 * Release feci->lock taken above.
			 */
			spin_unlock_bh(&feci->lock);

			/*
			 * If the pending decelerate was done through defunct process, we should
			 * re-try it here with the same defunct function, because the purpose of that
			 * process is to remove the connection from the database as well after decelerating it.
			 */
			if (is_defunct) {
				ecm_db_connection_make_defunct(feci->ci);
			} else {
				feci->decelerate(feci);
			}

			ecm_db_connection_deref(feci->ci);
			kfree(pd6rc);
			return;
		}

		/*
		 * Release feci->lock taken above.
		 */
		spin_unlock_bh(&feci->lock);

		ecm_db_connection_deref(feci->ci);
		DEBUG_TRACE("%px: ppe_drv_v6_create() non-ported success with ret=%d\n", pd6rc, ppe_tx_status);
		kfree(pd6rc);
		return;
	}

	/*
	 * Creation command failed (specific reason ignored).
	 */
	ecm_ppe_ipv6_accel_done_time_update(feci);

	/*
	 * Take feci->lock till completion of rule create in ECM.
	 */
	spin_lock_bh(&feci->lock);

	/*
	 * Creation command failed (specific reason ignored).
	 */
	DEBUG_ASSERT(feci->accel_mode == ECM_FRONT_END_ACCELERATION_MODE_ACCEL_PENDING, "%px: Unexpected mode: %d\n", feci->ci, feci->accel_mode);
	feci->stats.ae_nack++;
	feci->stats.ae_nack_total++;
	if (feci->stats.ae_nack >= feci->stats.ae_nack_limit) {
		/*
		 * Too many PPE rejections
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

	spin_lock_bh(&ecm_ppe_ipv6_lock);
	_ecm_ppe_ipv6_accel_pending_clear(feci, result_mode);
	spin_unlock_bh(&ecm_ppe_ipv6_lock);

	/*
	 * Release feci->lock taken above.
	 */
	spin_unlock_bh(&feci->lock);

	/*
	 * Release the connection.
	 */
	ecm_db_connection_deref(feci->ci);
	DEBUG_TRACE("%px: ppe_drv_v6_create() failed with ret=%d\n", feci, ppe_tx_status);
	kfree(pd6rc);
	return;

non_ported_accel_bad_rule:

	kfree(pd6rc);

	/*
	 * Jump to here when rule data is bad and an offload command cannot be constructed
	 */
	DEBUG_WARN("%px: Accel failed for non-ported IPv6 flow - bad rule\n", feci);
	ecm_ppe_ipv6_accel_pending_clear(feci, ECM_FRONT_END_ACCELERATION_MODE_FAIL_RULE);
}

/*
 * ecm_ppe_non_ported_ipv6_connection_destroy_done()
 *	Callback for handling destroy ack/nack calls.
 *	feci->lock is caller's responsibility.
 */
static void ecm_ppe_non_ported_ipv6_connection_destroy_done(struct ecm_front_end_connection_instance *feci)
{
	/*
	 * Drop decel pending counter
	 */
	spin_lock_bh(&ecm_ppe_ipv6_lock);
	ecm_ppe_ipv6_pending_decel_count--;
	DEBUG_ASSERT(ecm_ppe_ipv6_pending_decel_count >= 0, "Bad decel pending counter\n");
	spin_unlock_bh(&ecm_ppe_ipv6_lock);

	/*
	 * If decel is not still pending then it's possible that the PPE ended acceleration by some other reason e.g. flush
	 * In which case we cannot rely on the response we get here.
	 */
	if (feci->accel_mode != ECM_FRONT_END_ACCELERATION_MODE_DECEL_PENDING) {
		return;
	}

	feci->accel_mode = ECM_FRONT_END_ACCELERATION_MODE_DECEL;

	/*
	 * If connection became defunct then set mode so that no further accel/decel attempts occur.
	 */
	if (feci->is_defunct) {
		feci->accel_mode = ECM_FRONT_END_ACCELERATION_MODE_FAIL_DEFUNCT;
	}

	/*
	 * Ported acceleration ends
	 */
	spin_lock_bh(&ecm_ppe_ipv6_lock);
	ecm_ppe_non_ported_ipv6_accelerated_count--;	/* Protocol specific counter */
	DEBUG_ASSERT(ecm_ppe_non_ported_ipv6_accelerated_count >= 0, "Bad udp accel counter\n");
	ecm_ppe_ipv6_accelerated_count--;		/* General running counter */
	DEBUG_ASSERT(ecm_ppe_ipv6_accelerated_count >= 0, "Bad accel counter\n");
	spin_unlock_bh(&ecm_ppe_ipv6_lock);
}

/*
 * ecm_ppe_non_ported_ipv6_connection_decelerate_send()
 *	Send decelerate call to PPE
 */
static bool ecm_ppe_non_ported_ipv6_connection_decelerate_send(struct ecm_front_end_connection_instance *feci)
{
	struct ppe_drv_v6_rule_destroy pd6rd;
	ip_addr_t src_ip;
	ip_addr_t dest_ip;
	ppe_drv_ret_t status;
	bool ret;

	/*
	 * Increment the decel pending counter
	 */
	spin_lock_bh(&ecm_ppe_ipv6_lock);
	ecm_ppe_ipv6_pending_decel_count++;
	spin_unlock_bh(&ecm_ppe_ipv6_lock);

	pd6rd.tuple.protocol = (int32_t)ecm_db_connection_protocol_get(feci->ci);

	/*
	 * Get addressing information
	 */
	ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_FROM, src_ip);
	ECM_IP_ADDR_TO_PPE_IPV6_ADDR(pd6rd.tuple.flow_ip, src_ip);
	ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_TO_NAT, dest_ip);
	ECM_IP_ADDR_TO_PPE_IPV6_ADDR(pd6rd.tuple.return_ip, dest_ip);
	pd6rd.tuple.flow_ident = ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_FROM);
	pd6rd.tuple.return_ident = ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_TO_NAT);

	/*
	 * Take a ref to the feci->ci so that it will persist until we get a response from the PPE.
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
	 * Destroy the PPE connection cache entry.
	 */
	status = ppe_drv_v6_destroy(&pd6rd);
	if (status == PPE_DRV_RET_SUCCESS) {
		/*
		 * Take feci->lock till completion of rule destroy in ECM.
		 */
		spin_lock_bh(&feci->lock);
		feci->stats.driver_fail = 0;
		ecm_ppe_non_ported_ipv6_connection_destroy_done(feci);

		/*
		 * Release the feci->lock taken above.
		 */
		spin_unlock_bh(&feci->lock);

		/*
		 * Record command duration
		 */
		ecm_ppe_ipv6_decel_done_time_update(feci);

		ecm_db_connection_deref(feci->ci);
		return true;
	}

	/*
	 * PPE Deceleration failed and may need retry.
	 */
	ret = ecm_front_end_destroy_failure_handle(feci);

	/*
	 * Record command duration
	 */
	ecm_ppe_ipv6_decel_done_time_update(feci);

	spin_lock_bh(&ecm_ppe_ipv6_lock);
	ecm_ppe_ipv6_pending_decel_count--;
	DEBUG_ASSERT(ecm_ppe_ipv6_pending_decel_count >= 0, "Bad decel pending counter\n");
	spin_unlock_bh(&ecm_ppe_ipv6_lock);

	ecm_db_connection_deref(feci->ci);
	return ret;
}

/*
 * ecm_ppe_non_ported_ipv6_connection_decelerate()
 *	Decelerate a connection
 */
static bool ecm_ppe_non_ported_ipv6_connection_decelerate(struct ecm_front_end_connection_instance *feci)
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

	return ecm_ppe_non_ported_ipv6_connection_decelerate_send(feci);
}

/*
 * ecm_ppe_non_ported_ipv6_connection_defunct_callback()
 *	Callback to be called when a non-ported connection has become defunct.
 */
bool ecm_ppe_non_ported_ipv6_connection_defunct_callback(void *arg, int *accel_mode)
{
	bool ret;
	struct ecm_front_end_connection_instance *feci = (struct ecm_front_end_connection_instance *)arg;

	DEBUG_CHECK_MAGIC(feci, ECM_FRONT_END_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", feci);

	/*
	 * Check if the connection can be defuncted.
	 */
	spin_lock_bh(&feci->lock);
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
	ret = ecm_ppe_non_ported_ipv6_connection_decelerate_send(feci);

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
 * ecm_ppe_non_ported_ipv6_connection_accel_ceased()
 *	PPE has indicated that acceleration has stopped.
 *
 * NOTE: This is called in response to an PPE self-initiated termination of acceleration.
 * This must NOT be called because the ECM terminated the acceleration.
 */
static void ecm_ppe_non_ported_ipv6_connection_accel_ceased(struct ecm_front_end_connection_instance *feci)
{
	DEBUG_CHECK_MAGIC(feci, ECM_FRONT_END_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", feci);

	DEBUG_INFO("%px: accel ceased\n", feci);

	spin_lock_bh(&feci->lock);

	/*
	 * If we are in accel-pending state then the PPE has issued a flush out-of-order
	 * with the ACK/NACK we are actually waiting for.
	 * To work around this we record a "flush has already happened" and will action it when we finally get that ACK/NACK.
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
	 * Non-Ported acceleration ends
	 */
	spin_lock_bh(&ecm_ppe_ipv6_lock);
	ecm_ppe_non_ported_ipv6_accelerated_count--;	/* Protocol specific counter */
	DEBUG_ASSERT(ecm_ppe_non_ported_ipv6_accelerated_count >= 0, "Bad non-ported accel counter\n");
	ecm_ppe_ipv6_accelerated_count--;		/* General running counter */
	DEBUG_ASSERT(ecm_ppe_ipv6_accelerated_count >= 0, "Bad accel counter\n");
	spin_unlock_bh(&ecm_ppe_ipv6_lock);
}

#ifdef ECM_STATE_OUTPUT_ENABLE
/*
 * ecm_ppe_non_ported_ipv6_connection_state_get()
 *	Return the state of this Non ported front end instance
 */
static int ecm_ppe_non_ported_ipv6_connection_state_get(struct ecm_front_end_connection_instance *feci, struct ecm_state_file_instance *sfi)
{
	DEBUG_CHECK_MAGIC(feci, ECM_FRONT_END_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", feci);

	return ecm_front_end_common_connection_state_get(feci, sfi, "ppe_v6.non_ported");
}
#endif

/*
 * ecm_ppe_non_ported_ipv6_connection_instance_alloc()
 *	Create a front end instance specific for non-ported connection
 */
struct ecm_front_end_connection_instance *ecm_ppe_non_ported_ipv6_connection_instance_alloc(
									uint32_t flags,
									int protocol,
									struct ecm_db_connection_instance **nci)
{
	struct ecm_front_end_connection_instance *feci;
	struct ecm_db_connection_instance *ci;
	bool can_accel = (flags & ECM_FRONT_END_ENGINE_FLAG_CAN_ACCEL);

	if (ecm_ppe_ipv6_is_conn_limit_reached()) {
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
		DEBUG_WARN("Non-Ported Front end alloc failed\n");
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
	feci->accel_engine = ECM_FRONT_END_ENGINE_PPE;
	spin_lock_bh(&ecm_ppe_ipv6_lock);
	feci->stats.no_action_seen_limit = ecm_ppe_ipv6_no_action_limit_default;
	feci->stats.driver_fail_limit = ecm_ppe_ipv6_driver_fail_limit_default;
	feci->stats.ae_nack_limit = ecm_ppe_ipv6_nack_limit_default;
	spin_unlock_bh(&ecm_ppe_ipv6_lock);

	/*
	 * Copy reference to connection - no need to ref ci as ci maintains a ref to this instance instead (this instance persists for as long as ci does)
	 */
	feci->ci = ci;
	feci->ip_version = 6;
	feci->protocol = protocol;

	/*
	 * Populate the methods and callbacks
	 */
	feci->accelerate = ecm_ppe_non_ported_ipv6_connection_accelerate;
	feci->decelerate = ecm_ppe_non_ported_ipv6_connection_decelerate;
	feci->accel_ceased = ecm_ppe_non_ported_ipv6_connection_accel_ceased;
#ifdef ECM_STATE_OUTPUT_ENABLE
	feci->state_get = ecm_ppe_non_ported_ipv6_connection_state_get;
#endif
	feci->ae_interface_number_by_dev_get = ecm_ppe_common_get_interface_number_by_dev;
	feci->ae_interface_number_by_dev_type_get = ecm_ppe_common_get_interface_number_by_dev_type;
	feci->ae_interface_type_get = ecm_ppe_common_get_interface_type;
	feci->regenerate = ecm_ppe_common_connection_regenerate;
	feci->defunct = ecm_ppe_non_ported_ipv6_connection_defunct_callback;

	feci->get_stats_bitmap = ecm_ppe_common_dummy_get_stats_bitmap;
	feci->set_stats_bitmap = ecm_ppe_common_dummy_set_stats_bitmap;
	feci->fe_info.front_end_flags = flags;

	return feci;
}

/*
 * ecm_ppe_non_ported_ipv6_debugfs_init()
 */
bool ecm_ppe_non_ported_ipv6_debugfs_init(struct dentry *dentry)
{
	if (!debugfs_create_u32("non_ported_accelerated_count", S_IRUGO, dentry,
					(u32 *)&ecm_ppe_non_ported_ipv6_accelerated_count)) {
		DEBUG_ERROR("Failed to create ecm ppe ipv6 non_ported_accelerated_count file in debugfs\n");
		return false;
	}

	return true;
}
