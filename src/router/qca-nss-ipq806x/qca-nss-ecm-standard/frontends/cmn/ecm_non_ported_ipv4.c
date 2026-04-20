/*
 **************************************************************************
 * Copyright (c) 2014-2021 The Linux Foundation. All rights reserved.
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
#include <linux/debugfs.h>
#include <linux/pkt_sched.h>
#include <linux/string.h>
#include <net/route.h>
#include <net/ip.h>
#include <net/tcp.h>
#include <net/addrconf.h>
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
#define DEBUG_LEVEL ECM_CMN_NON_PORTED_IPV4_DEBUG_LEVEL

#ifdef ECM_FRONT_END_NSS_ENABLE
#include <nss_api_if.h>
#endif
#ifdef ECM_FRONT_END_SFE_ENABLE
#include <sfe_api.h>
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
#include "ecm_classifier_default.h"
#include "ecm_interface.h"
#ifdef ECM_FRONT_END_NSS_ENABLE
#include "ecm_nss_non_ported_ipv4.h"
#include "ecm_nss_ipv4.h"
#include "ecm_nss_common.h"
#endif
#ifdef ECM_FRONT_END_SFE_ENABLE
#include "ecm_sfe_non_ported_ipv4.h"
#include "ecm_sfe_ipv4.h"
#include "ecm_sfe_common.h"
#endif
#ifdef ECM_FRONT_END_PPE_ENABLE
#include "ecm_ppe_non_ported_ipv4.h"
#include "ecm_ppe_ipv4.h"
#include "ecm_ppe_common.h"
#endif
#include "ecm_front_end_common.h"
#include "ecm_ipv4.h"
#include "ecm_ae_classifier_public.h"
#include "ecm_ae_classifier.h"
#include "ecm_stats_v4.h"

/*
 * ecm_non_ported_ipv4_is_protocol_supported()
 *	Return true if IPv4 protocol is supported in non-ported acceleration.
 */
static inline bool ecm_non_ported_ipv4_is_protocol_supported(int protocol)
{
	switch (protocol) {
	case IPPROTO_IPV6:
	case IPPROTO_ESP:
#if defined(ECM_INTERFACE_PPTP_ENABLE) || defined(ECM_INTERFACE_GRE_TAP_ENABLE) || defined(ECM_INTERFACE_GRE_TUN_ENABLE)
	case IPPROTO_GRE:
#endif
	case IPPROTO_RAW:
#ifdef ECM_INTERFACE_L2TPV3_ENABLE
	case IPPROTO_L2TP:
#endif
		return true;
	}
	return false;
}

/*
 * ecm_non_ported_ipv4_process()
 *	Process a protocol that does not have port based identifiers
 */
unsigned int ecm_non_ported_ipv4_process(struct net_device *out_dev, struct net_device *out_dev_nat,
							struct net_device *in_dev, struct net_device *in_dev_nat,
							uint8_t *src_node_addr, uint8_t *src_node_addr_nat,
							uint8_t *dest_node_addr, uint8_t *dest_node_addr_nat,
							bool can_accel, bool is_routed, bool is_l2_encap, struct sk_buff *skb,
							struct ecm_tracker_ip_header *ip_hdr,
							struct nf_conn *ct, ecm_tracker_sender_type_t sender, ecm_db_direction_t ecm_dir,
							struct nf_conntrack_tuple *orig_tuple, struct nf_conntrack_tuple *reply_tuple,
							ip_addr_t ip_src_addr, ip_addr_t ip_dest_addr, ip_addr_t ip_src_addr_nat, ip_addr_t ip_dest_addr_nat,
							uint16_t l2_encap_proto)
{
	struct ecm_db_connection_instance *ci;
	struct ecm_front_end_connection_instance *feci;
	int protocol;
	int src_port;
	int src_port_nat;
	int dest_port;
	int dest_port_nat;
	ip_addr_t match_addr;
	struct ecm_classifier_instance *assignments[ECM_CLASSIFIER_TYPES];
	int aci_index;
	int assignment_count;
	ecm_db_timer_group_t ci_orig_timer_group;
	struct ecm_classifier_process_response prevalent_pr;
	bool pppoe_bridged = false;
	uint32_t flags = can_accel ? ECM_FRONT_END_ENGINE_FLAG_CAN_ACCEL : 0;

	/*
	 * Look up a connection.
	 */
	protocol = (int)orig_tuple->dst.protonum;
	src_port = 0;
	src_port_nat = 0;
	dest_port = 0;
	dest_port_nat = 0;

	/*
	 * 3-tuple acceleration for PPPoE bridged flow?
	 */
	if (unlikely(!is_routed &&
			(l2_encap_proto == ETH_P_PPP_SES) &&
			ecm_front_end_ppppoe_br_accel_3tuple())) {
		struct pppoe_hdr *ph;
		uint32_t l2_encap_len = ecm_front_end_l2_encap_header_len(l2_encap_proto);

		/*
		 * Get PPPoE session id from skb.
		 */
		ecm_front_end_push_l2_encap_header(skb, l2_encap_len);
		ph = pppoe_hdr(skb);
		src_port = ntohs(ph->sid);
		protocol = IPPROTO_RAW;
		src_port_nat = src_port;
		dest_port = src_port;
		dest_port_nat = src_port;
		ecm_front_end_pull_l2_encap_header(skb, l2_encap_len);
		pppoe_bridged = true;
		DEBUG_TRACE("PPPoE bridged flow: session ID=%#x skb=%px\n", ntohs(ph->sid), skb);
	}

	if(!ecm_non_ported_ipv4_is_protocol_supported(protocol)) {
		DEBUG_TRACE("Unsupported non-ported protocol: %d, do not process.\n", protocol);
		ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_NON_PORTED, ECM_STATS_V4_EXCEPTION_NON_PORTED_PROTOCOL_UNSUPPORTED);
		return NF_ACCEPT;
	}

	DEBUG_TRACE("Non ported src: " ECM_IP_ADDR_DOT_FMT "(" ECM_IP_ADDR_DOT_FMT "), dest: " ECM_IP_ADDR_DOT_FMT "(" ECM_IP_ADDR_DOT_FMT "), dir %d\n",
				ECM_IP_ADDR_TO_DOT(ip_src_addr), ECM_IP_ADDR_TO_DOT(ip_src_addr_nat), ECM_IP_ADDR_TO_DOT(ip_dest_addr),
				ECM_IP_ADDR_TO_DOT(ip_dest_addr_nat), ecm_dir);

	ci = ecm_db_connection_find_and_ref(ip_src_addr, ip_dest_addr, protocol, src_port, dest_port);

	/*
	 * If there is no existing connection then create a new one.
	 */
	if (unlikely(!ci)) {
		struct ecm_db_mapping_instance *mi[ECM_DB_OBJ_DIR_MAX];
		struct ecm_db_node_instance *ni[ECM_DB_OBJ_DIR_MAX];
		struct ecm_classifier_default_instance *dci;
		struct ecm_db_connection_instance *nci;
		ecm_classifier_type_t classifier_type;
		int32_t to_list_first;
		struct ecm_db_iface_instance *to_list[ECM_DB_IFACE_HEIRARCHY_MAX];
		int32_t to_nat_list_first;
		struct ecm_db_iface_instance *to_nat_list[ECM_DB_IFACE_HEIRARCHY_MAX];
		int32_t from_list_first;
		struct ecm_db_iface_instance *from_list[ECM_DB_IFACE_HEIRARCHY_MAX];
		int32_t from_nat_list_first;
		struct ecm_db_iface_instance *from_nat_list[ECM_DB_IFACE_HEIRARCHY_MAX];
		struct ecm_front_end_interface_construct_instance efeici;
		ecm_ae_classifier_result_t ae_result;
		ecm_ae_classifier_get_t ae_get;
		int i;

		DEBUG_INFO("New non-ported connection from " ECM_IP_ADDR_DOT_FMT ":%u to " ECM_IP_ADDR_DOT_FMT ":%u protocol: %d\n",
				ECM_IP_ADDR_TO_DOT(ip_src_addr), src_port, ECM_IP_ADDR_TO_DOT(ip_dest_addr), dest_port, protocol);

		/*
		 * Before we attempt to create the connection are we being terminated?
		 */
		spin_lock_bh(&ecm_ipv4_lock);
		if (ecm_ipv4_terminate_pending) {
			spin_unlock_bh(&ecm_ipv4_lock);
			DEBUG_WARN("Terminating\n");
			ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_NON_PORTED, ECM_STATS_V4_EXCEPTION_NON_PORTED_ECM_IN_TERMINATING_STATE);

			/*
			 * As we are terminating we just allow the packet to pass - it's no longer our concern
			 */
			return NF_ACCEPT;
		}
		spin_unlock_bh(&ecm_ipv4_lock);

		/*
		 * Check if an external AE classifier is registered.
		 * If it is not registered at all or unregistered at runtime
		 * a dummy callback will return ECM_AE_CLASSIFIER_RESULT_DONT_CARE.
		 */
		rcu_read_lock();
		ae_get = rcu_dereference(ae_ops.ae_get);
		if (!ae_get) {
			ae_result = ECM_AE_CLASSIFIER_RESULT_DONT_CARE;
		} else {
			struct ecm_ae_classifier_info ae_info;
			ecm_ae_classifier_select_info_fill(ip_src_addr, ip_dest_addr,
						  src_port, dest_port, protocol, 4,
						  is_routed, false,
						  &ae_info);
			ae_result = ae_get(&ae_info);
		}
		rcu_read_unlock();

		DEBUG_TRACE("front end type: %d ae_result: %d\n", ecm_front_end_type_get(), ae_result);

		/*
		 * Which AE can be used for this flow.
		 * 1. If NSS, allocate NSS ipv4 non-ported connection instance
		 * 2. If SFE, allocate SFE ipv4 non-ported connection instance
		 * 3. If PPE/PPE-VP/PPE-DS, allocate PPE ipv4 non-ported connection instance
		 * 4. If NOT_YET, the connection will not be allocated in the database and the next flow will be
		 *    re-evaluated.
		 * 5. If NONE, allocate non-ported connection instance based on the precedence array with
		 *    can_accel flag set to false. By doing this we will not try to re-evaluate this flow again.
		 * 6. If DONT_CARE, select the AE from the precdence array which is in the highest priority index.
		 * 7. If any other type, return NF_ACCEPT.
		 */
		switch (ae_result) {
#ifdef ECM_FRONT_END_NSS_ENABLE
		case ECM_AE_CLASSIFIER_RESULT_NSS:
			if (!ecm_nss_feature_check(skb, ip_hdr)) {
				DEBUG_WARN("Unsupported feature found for NSS acceleration\n");
				ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_NON_PORTED, ECM_STATS_V4_EXCEPTION_NON_PORTED_NSS_ACCEL_NOT_SUPPORTED);
				return NF_ACCEPT;
			}

			feci = ecm_nss_non_ported_ipv4_connection_instance_alloc(flags, protocol, &nci);
			goto feci_alloc_check;
#endif
#ifdef ECM_FRONT_END_SFE_ENABLE
		case ECM_AE_CLASSIFIER_RESULT_SFE:
			feci = ecm_sfe_non_ported_ipv4_connection_instance_alloc(flags, protocol, &nci);
			goto feci_alloc_check;
#endif
#ifdef ECM_FRONT_END_PPE_ENABLE
		case ECM_AE_CLASSIFIER_RESULT_PPE_DS:
			if (!ecm_ppe_feature_check(skb, ip_hdr)) {
				DEBUG_WARN("Unsupported feature found for PPE acceleration\n");
				ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_NON_PORTED, ECM_STATS_V4_EXCEPTION_NON_PORTED_PPE_DS_ACCEL_NOT_SUPPORTED);
				return NF_ACCEPT;
			}

			flags |= ECM_FRONT_END_ENGINE_FLAG_PPE_DS;
			feci = ecm_ppe_non_ported_ipv4_connection_instance_alloc(flags, protocol, &nci);
			goto feci_alloc_check;

		case ECM_AE_CLASSIFIER_RESULT_PPE_VP:
			if (!ecm_ppe_feature_check(skb, ip_hdr)) {
				DEBUG_WARN("Unsupported feature found for PPE acceleration\n");
				ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_NON_PORTED, ECM_STATS_V4_EXCEPTION_NON_PORTED_PPE_VP_ACCEL_NOT_SUPPORTED);
				return NF_ACCEPT;
			}

			flags |= ECM_FRONT_END_ENGINE_FLAG_PPE_VP;
			feci = ecm_ppe_non_ported_ipv4_connection_instance_alloc(flags, protocol, &nci);
			goto feci_alloc_check;

		case ECM_AE_CLASSIFIER_RESULT_PPE:
			if (!ecm_ppe_feature_check(skb, ip_hdr)) {
				DEBUG_WARN("Unsupported feature found for PPE acceleration\n");
				ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_NON_PORTED, ECM_STATS_V4_EXCEPTION_NON_PORTED_PPE_ACCEL_NOT_SUPPORTED);
				return NF_ACCEPT;
			}

			feci = ecm_ppe_non_ported_ipv4_connection_instance_alloc(flags, protocol, &nci);
			goto feci_alloc_check;
#endif
		case ECM_AE_CLASSIFIER_RESULT_NOT_YET:
			ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_NON_PORTED, ECM_STATS_V4_EXCEPTION_NON_PORTED_AE_NOT_ASSIGNED);
			return NF_ACCEPT;

		case ECM_AE_CLASSIFIER_RESULT_NONE:
			/*
			 * Let the precedence array select the AE type and add it
			 * to the database without accelerating the flow.
			 */
			flags &= ~ECM_FRONT_END_ENGINE_FLAG_CAN_ACCEL;
			goto precedence_alloc;

		case ECM_AE_CLASSIFIER_RESULT_DONT_CARE:
			/*
			 * External module doesn't care about which AE is selected.
			 * So, allocate one from the AE precedence array.
			 */
			goto precedence_alloc;

		default:
			DEBUG_WARN("unexpected ae_result: %d\n", ae_result);
			ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_NON_PORTED, ECM_STATS_V4_EXCEPTION_NON_PORTED_UNKNOWN_AE_TYPE);
			return NF_ACCEPT;
		}

precedence_alloc:
		for (i = 0; i <= ECM_AE_PRECEDENCE_MAX; i++) {
			if (ae_precedence[i].ae_type == ECM_FRONT_END_ENGINE_MAX) {
				DEBUG_WARN("None of the AE types in the precedence array could allocate the front end instance\n");
				ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_NON_PORTED, ECM_STATS_V4_EXCEPTION_NON_PORTED_PRECEDENCE_ALLOC_FAIL);
				return NF_ACCEPT;
			}

			/*
			 * Allocate a frontend instance for the type selected in the precedence array.
			 * Do a feature check. If the AE doesn't support it, try the next one in the array.
			 */
			if (!ecm_front_end_common_feature_check(ae_precedence[i].ae_type, skb, ip_hdr, is_routed)) {
				DEBUG_WARN("Unsupported feature found for the selected AE: %d\n", ae_precedence[i].ae_type);
				continue;
			}

			/*
			 * If allocation fails for the selected AE, try the next one.
			 */
			feci = ae_precedence[i].non_ported_ipv4_alloc(flags, protocol, &nci);
			if (!feci) {
				DEBUG_WARN("Failed to allocate front end instance\n");
				continue;
			}
			goto feci_alloc_done;
		}

feci_alloc_check:
		if (!feci) {
			ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_NON_PORTED, ECM_STATS_V4_EXCEPTION_NON_PORTED_FRONTEND_ALLOC_FAIL);
			DEBUG_WARN("Failed to allocate front end\n");
			return NF_ACCEPT;
		}

feci_alloc_done:
		if (!ecm_front_end_ipv4_interface_construct_set_and_hold(skb, sender, ecm_dir, is_routed,
							in_dev, out_dev,
							ip_src_addr, ip_src_addr_nat,
							ip_dest_addr, ip_dest_addr_nat,
							&efeici)) {
			DEBUG_WARN("ECM front end ipv4 interface construct set failed for routed traffic\n");
			ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_NON_PORTED, ECM_STATS_V4_EXCEPTION_NON_PORTED_FRONTEND_CONSTRUCTION_FAIL);
			goto fail_1;
		}

		/*
		 * Get the src and destination mappings.
		 * For this we also need the interface lists which we also set upon the new connection while we are at it.
		 * GGG TODO rework terms of "src/dest" - these need to be named consistently as from/to as per database terms.
		 * GGG TODO The empty list checks should not be needed, mapping_establish_and_ref() should fail out if there is no list anyway.
		 */
		DEBUG_TRACE("%px: Create the 'from' interface heirarchy list\n", nci);
		from_list_first = ecm_interface_heirarchy_construct(feci, from_list, efeici.from_dev, efeici.from_other_dev, ip_dest_addr, efeici.from_mac_lookup_ip_addr, ip_src_addr, 4, protocol, in_dev, is_routed, in_dev, src_node_addr, dest_node_addr, NULL, skb, NULL);
		if (from_list_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
			DEBUG_WARN("Failed to obtain 'from' heirarchy list\n");
			ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_NON_PORTED, ECM_STATS_V4_EXCEPTION_NON_PORTED_FROM_HIERARCHY_CREATION_FAIL);
			goto fail_2;
		}
		ecm_db_connection_interfaces_reset(nci, from_list, from_list_first, ECM_DB_OBJ_DIR_FROM);

		DEBUG_TRACE("%px: Create source node\n", nci);
		ni[ECM_DB_OBJ_DIR_FROM] = ecm_ipv4_node_establish_and_ref(feci, efeici.from_dev, efeici.from_mac_lookup_ip_addr, from_list, from_list_first, src_node_addr, skb);
		ecm_db_connection_interfaces_deref(from_list, from_list_first);
		if (!ni[ECM_DB_OBJ_DIR_FROM]) {
			DEBUG_WARN("Failed to establish source node\n");
			ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_NON_PORTED, ECM_STATS_V4_EXCEPTION_NON_PORTED_FROM_NODE_FAIL);
			goto fail_2;
		}

		DEBUG_TRACE("%px: Create source mapping\n", nci);
		mi[ECM_DB_OBJ_DIR_FROM] = ecm_ipv4_mapping_establish_and_ref(ip_src_addr, src_port);
		if (!mi[ECM_DB_OBJ_DIR_FROM]) {
			DEBUG_WARN("Failed to establish src mapping\n");
			ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_NON_PORTED, ECM_STATS_V4_EXCEPTION_NON_PORTED_FROM_MAPPING_FAIL);
			goto fail_3;
		}

		DEBUG_TRACE("%px: Create the 'to' interface heirarchy list\n", nci);
		to_list_first = ecm_interface_heirarchy_construct(feci, to_list, efeici.to_dev, efeici.to_other_dev, ip_src_addr, efeici.to_mac_lookup_ip_addr, ip_dest_addr, 4, protocol, out_dev, is_routed, in_dev, dest_node_addr, src_node_addr, NULL, skb, NULL);
		if (to_list_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
			DEBUG_WARN("Failed to obtain 'to' heirarchy list\n");
			ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_NON_PORTED, ECM_STATS_V4_EXCEPTION_NON_PORTED_TO_HIERARCHY_CREATION_FAIL);
			goto fail_4;
		}
		ecm_db_connection_interfaces_reset(nci, to_list, to_list_first, ECM_DB_OBJ_DIR_TO);

		DEBUG_TRACE("%px: Create dest node\n", nci);
		ni[ECM_DB_OBJ_DIR_TO] = ecm_ipv4_node_establish_and_ref(feci, efeici.to_dev, efeici.to_mac_lookup_ip_addr, to_list, to_list_first, dest_node_addr, skb);
		ecm_db_connection_interfaces_deref(to_list, to_list_first);
		if (!ni[ECM_DB_OBJ_DIR_TO]) {
			DEBUG_WARN("Failed to establish dest node\n");
			ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_NON_PORTED, ECM_STATS_V4_EXCEPTION_NON_PORTED_TO_NODE_FAIL);
			goto fail_4;
		}

		DEBUG_TRACE("%px: Create dest mapping\n", nci);
		mi[ECM_DB_OBJ_DIR_TO] = ecm_ipv4_mapping_establish_and_ref(ip_dest_addr, dest_port);
		if (!mi[ECM_DB_OBJ_DIR_TO]) {
			DEBUG_WARN("Failed to establish dest mapping\n");
			ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_NON_PORTED, ECM_STATS_V4_EXCEPTION_NON_PORTED_TO_MAPPING_FAIL);
			goto fail_5;
		}

		/*
		 * Get the src and destination NAT mappings
		 * For this we also need the interface lists which we also set upon the new connection while we are at it.
		 * GGG TODO rework terms of "src/dest" - these need to be named consistently as from/to as per database terms.
		 * GGG TODO The empty list checks should not be needed, mapping_establish_and_ref() should fail out if there is no list anyway.
		 */

		/*
		 * NOTE: For SIT tunnels use the in_dev instead of in_dev_nat
		 */
		DEBUG_TRACE("%px: Create the 'from NAT' interface heirarchy list\n", nci);
		if ((protocol == IPPROTO_IPV6) || (protocol == IPPROTO_ESP)) {
			from_nat_list_first = ecm_interface_heirarchy_construct(feci, from_nat_list, efeici.from_nat_dev, efeici.from_nat_other_dev, ip_dest_addr, efeici.from_nat_mac_lookup_ip_addr, ip_src_addr_nat, 4, protocol, in_dev, is_routed, in_dev, src_node_addr_nat, dest_node_addr_nat, NULL, skb, NULL);
		} else {
			from_nat_list_first = ecm_interface_heirarchy_construct(feci, from_nat_list, efeici.from_nat_dev, efeici.from_nat_other_dev, ip_dest_addr, efeici.from_nat_mac_lookup_ip_addr, ip_src_addr_nat, 4, protocol, in_dev_nat, is_routed, in_dev, src_node_addr_nat, dest_node_addr_nat, NULL, skb, NULL);
		}

		if (from_nat_list_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
			DEBUG_WARN("Failed to obtain 'from NAT' heirarchy list\n");
			ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_NON_PORTED, ECM_STATS_V4_EXCEPTION_NON_PORTED_FROM_NAT_HIERARCHY_CREATION_FAIL);
			goto fail_6;
		}
		ecm_db_connection_interfaces_reset(nci, from_nat_list, from_nat_list_first, ECM_DB_OBJ_DIR_FROM_NAT);

		DEBUG_TRACE("%px: Create source nat node\n", nci);
		ni[ECM_DB_OBJ_DIR_FROM_NAT] = ecm_ipv4_node_establish_and_ref(feci, efeici.from_nat_dev, efeici.from_nat_mac_lookup_ip_addr, from_nat_list, from_nat_list_first, src_node_addr_nat, skb);
		ecm_db_connection_interfaces_deref(from_nat_list, from_nat_list_first);
		if (!ni[ECM_DB_OBJ_DIR_FROM_NAT]) {
			DEBUG_WARN("Failed to establish source nat node\n");
			ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_NON_PORTED, ECM_STATS_V4_EXCEPTION_NON_PORTED_FROM_NAT_NODE_FAIL);
			goto fail_6;
		}

		mi[ECM_DB_OBJ_DIR_FROM_NAT] = ecm_ipv4_mapping_establish_and_ref(ip_src_addr_nat, src_port_nat);
		if (!mi[ECM_DB_OBJ_DIR_FROM_NAT]) {
			DEBUG_WARN("Failed to establish src nat mapping\n");
			ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_NON_PORTED, ECM_STATS_V4_EXCEPTION_NON_PORTED_FROM_NAT_MAPPING_FAIL);
			goto fail_7;
		}

		DEBUG_TRACE("%px: Create the 'to NAT' interface heirarchy list\n", nci);
		to_nat_list_first = ecm_interface_heirarchy_construct(feci, to_nat_list, efeici.to_nat_dev, efeici.to_nat_other_dev, ip_src_addr, efeici.to_nat_mac_lookup_ip_addr, ip_dest_addr_nat, 4, protocol, out_dev_nat, is_routed, in_dev, dest_node_addr_nat, src_node_addr_nat, NULL, skb, NULL);
		if (to_nat_list_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
			DEBUG_WARN("Failed to obtain 'to NAT' heirarchy list\n");
			ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_NON_PORTED, ECM_STATS_V4_EXCEPTION_NON_PORTED_TO_NAT_HIERARCHY_CREATION_FAIL);
			goto fail_8;
		}
		ecm_db_connection_interfaces_reset(nci, to_nat_list, to_nat_list_first, ECM_DB_OBJ_DIR_TO_NAT);

		DEBUG_TRACE("%px: Create dest nat node\n", nci);
		ni[ECM_DB_OBJ_DIR_TO_NAT] = ecm_ipv4_node_establish_and_ref(feci, efeici.to_nat_dev, efeici.to_nat_mac_lookup_ip_addr, to_nat_list, to_nat_list_first, dest_node_addr_nat, skb);

		ecm_db_connection_interfaces_deref(to_nat_list, to_nat_list_first);
		if (!ni[ECM_DB_OBJ_DIR_TO_NAT]) {
			DEBUG_WARN("Failed to establish dest nat node\n");
			ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_NON_PORTED, ECM_STATS_V4_EXCEPTION_NON_PORTED_TO_NAT_NODE_FAIL);
			goto fail_8;
		}

		mi[ECM_DB_OBJ_DIR_TO_NAT] = ecm_ipv4_mapping_establish_and_ref(ip_dest_addr_nat, dest_port_nat);
		if (!mi[ECM_DB_OBJ_DIR_TO_NAT]) {
			DEBUG_WARN("Failed to establish dest mapping\n");
			ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_NON_PORTED, ECM_STATS_V4_EXCEPTION_NON_PORTED_TO_NAT_MAPPING_FAIL);
			goto fail_9;
		}

		/*
		 * Every connection also needs a default classifier
		 */
		dci = ecm_classifier_default_instance_alloc(nci, protocol, ecm_dir, src_port, dest_port);
		if (!dci) {
			DEBUG_WARN("Failed to allocate default classifier\n");
			ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_NON_PORTED, ECM_STATS_V4_EXCEPTION_NON_PORTED_DEFAULT_CLASSIFIER_ALLOC_FAIL);
			goto fail_10;
		}
		ecm_db_connection_classifier_assign(nci, (struct ecm_classifier_instance *)dci);

		/*
		 * Every connection starts with a full complement of classifiers assigned.
		 * NOTE: Default classifier is a special case considered previously
		 */
		for (classifier_type = ECM_CLASSIFIER_TYPE_DEFAULT + 1; classifier_type < ECM_CLASSIFIER_TYPES; ++classifier_type) {
			struct ecm_classifier_instance *aci = ecm_classifier_assign_classifier(nci, classifier_type);
			if (aci) {
				aci->deref(aci);
			} else {
				DEBUG_WARN("Failed to allocate classifiers assignments\n");
				ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_NON_PORTED, ECM_STATS_V4_EXCEPTION_NON_PORTED_CLASSIFIER_ASSIGN_FAIL);
				goto fail_11;
			}
		}

		ecm_db_front_end_instance_ref_and_set(nci, feci);

		ecm_db_connection_l2_encap_proto_set(nci, l2_encap_proto);

		/*
		 * Now add the connection into the database.
		 * NOTE: In an SMP situation such as ours there is a possibility that more than one packet for the same
		 * connection is being processed simultaneously.
		 * We *could* end up creating more than one connection instance for the same actual connection.
		 * To guard against this we now perform a mutex'd lookup of the connection + add once more - another cpu may have created it before us.
		 */
		spin_lock_bh(&ecm_ipv4_lock);
		ci = ecm_db_connection_find_and_ref(ip_src_addr, ip_dest_addr, protocol, src_port, dest_port);
		if (ci) {
			/*
			 * Another cpu created the same connection before us - use the one we just found
			 */
			spin_unlock_bh(&ecm_ipv4_lock);
			ecm_db_connection_deref(nci);
		} else {
			struct ecm_tracker_instance *ti;
			ecm_db_timer_group_t tg;
			ecm_tracker_sender_state_t src_state;
			ecm_tracker_sender_state_t dest_state;
			ecm_tracker_connection_state_t state;

			/*
			 * Ask tracker for timer group to set the connection to initially.
			 */
			ti = dci->tracker_get_and_ref(dci);
			ti->state_get(ti, &src_state, &dest_state, &state, &tg);
			ti->deref(ti);

			/*
			 * Add the new connection we created into the database
			 * NOTE: assign to a short timer group for now - it is the assigned classifiers responsibility to do this
			 */
			ecm_db_connection_add(nci, mi, ni,
					4, protocol, ecm_dir,
					NULL /* final callback */,
					tg, is_routed, nci);

			spin_unlock_bh(&ecm_ipv4_lock);

			ci = nci;
			DEBUG_INFO("%px: New Non-ported protocol %d connection created\n", ci, protocol);
		}

		if (pppoe_bridged) {
			ecm_db_connection_flag_set(ci, ECM_DB_CONNECTION_FLAGS_PPPOE_BRIDGE);
		}

		/*
		 * No longer need referenecs to the objects we created
		 */
		dci->base.deref((struct ecm_classifier_instance *)dci);
		ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_TO_NAT]);
		ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_TO_NAT]);
		ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_FROM_NAT]);
		ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_FROM_NAT]);
		ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_TO]);
		ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_TO]);
		ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_FROM]);
		ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_FROM]);
		ecm_front_end_ipv4_interface_construct_netdev_put(&efeici);
		ecm_front_end_connection_deref(feci);

		goto done;
fail_11:
		dci->base.deref((struct ecm_classifier_instance *)dci);
fail_10:
		ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_TO_NAT]);
fail_9:
		ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_TO_NAT]);
fail_8:
		ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_FROM_NAT]);
fail_7:
		ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_FROM_NAT]);
fail_6:
		ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_TO]);
fail_5:
		ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_TO]);
fail_4:
		ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_FROM]);
fail_3:
		ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_FROM]);
fail_2:
		ecm_front_end_ipv4_interface_construct_netdev_put(&efeici);
fail_1:
		ecm_front_end_connection_deref(feci);
		ecm_db_connection_deref(nci);
		return NF_ACCEPT;
done:
		;
	}

	/*
	 * Check if AE switch is needed.
	 */
	if (ecm_front_end_connection_check_and_switch_to_next_ae(ci->feci)) {
		DEBUG_TRACE("%px: new AE type: %d\n", ci, ci->feci->accel_engine);
	}

#if defined(CONFIG_NET_CLS_ACT) && defined(ECM_CLASSIFIER_DSCP_IGS) && defined(ECM_FRONT_END_NSS_ENABLE)
	/*
	 * Check if IGS feature is enabled or not.
	 */
	if (unlikely(ecm_interface_igs_enabled)) {
		feci = ecm_db_connection_front_end_get_and_ref(ci);
		if (feci->accel_engine == ECM_FRONT_END_ENGINE_NSS) {
			if (!ecm_nss_common_igs_acceleration_is_allowed(feci, skb)) {
				DEBUG_WARN("%px: Non-ported IPv4 IGS acceleration denied\n", ci);
				ecm_front_end_connection_deref(feci);
				ecm_db_connection_deref(ci);
				ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_NON_PORTED, ECM_STATS_V4_EXCEPTION_NON_PORTED_IGS_ACCEL_DENIED);
				return NF_ACCEPT;
			}
		}
		ecm_front_end_connection_deref(feci);
	}
#endif

	/*
	 * Keep connection alive as we have seen activity
	 */
	if (!ecm_db_connection_defunct_timer_touch(ci)) {
		ecm_db_connection_deref(ci);
		ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_NON_PORTED, ECM_STATS_V4_EXCEPTION_NON_PORTED_DB_CONN_TIMER_EXPIRED);
		return NF_ACCEPT;
	}

	/*
	 * Identify which side of the connection is sending.
	 * NOTE: This may be different than what sender is at the moment
	 * given the connection we have located.
	 */
	ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_FROM, match_addr);
	if (ECM_IP_ADDR_MATCH(ip_src_addr, match_addr)) {
		sender = ECM_TRACKER_SENDER_TYPE_SRC;
	} else {
		sender = ECM_TRACKER_SENDER_TYPE_DEST;
	}

	/*
	 * Do we need to action generation change?
	 */
	if (unlikely(ecm_db_connection_regeneration_required_check(ci))) {
		ecm_ipv4_connection_regenerate(ci, sender, out_dev, out_dev_nat, in_dev, in_dev_nat, NULL, skb);
	}

	/*
	 * Increment the slow path packet counter.
	 */
	feci = ecm_db_connection_front_end_get_and_ref(ci);
	spin_lock_bh(&feci->lock);
	feci->stats.slow_path_packets++;
	spin_unlock_bh(&feci->lock);
	ecm_front_end_connection_deref(feci);

	/*
	 * Iterate the assignments and call to process!
	 * Policy implemented:
	 * 1. Classifiers that say they are not relevant are unassigned and not actioned further.
	 * 2. Any drop command from any classifier is honoured.
	 * 3. Accel is never allowed for non-ported type connections.
	 * 4. Only the highest priority classifier, that actions it, will have its qos tag honoured.
	 * 5. Only the highest priority classifier, that actions it, will have its timer group honoured.
	 */
	DEBUG_TRACE("%px: process begin, skb: %px\n", ci, skb);
	prevalent_pr.process_actions = 0;
	prevalent_pr.drop = false;
	prevalent_pr.flow_qos_tag = skb->priority;
	prevalent_pr.return_qos_tag = skb->priority;
	prevalent_pr.accel_mode = ECM_CLASSIFIER_ACCELERATION_MODE_ACCEL;
	prevalent_pr.timer_group = ci_orig_timer_group = ecm_db_connection_timer_group_get(ci);

	assignment_count = ecm_db_connection_classifier_assignments_get_and_ref(ci, assignments);
	for (aci_index = 0; aci_index < assignment_count; ++aci_index) {
		struct ecm_classifier_process_response aci_pr;
		struct ecm_classifier_instance *aci;

		aci = assignments[aci_index];
		DEBUG_TRACE("%px: process: %px, type: %d\n", ci, aci, aci->type_get(aci));
		aci->process(aci, sender, ip_hdr, skb, &aci_pr);
		DEBUG_TRACE("%px: aci_pr: process actions: %x, became relevant: %u, relevance: %d, drop: %d, "
				"flow_qos_tag: %u, return_qos_tag: %u, accel_mode: %x, timer_group: %d\n",
				ci, aci_pr.process_actions, aci_pr.became_relevant, aci_pr.relevance, aci_pr.drop,
				aci_pr.flow_qos_tag, aci_pr.return_qos_tag, aci_pr.accel_mode, aci_pr.timer_group);

		if (aci_pr.relevance == ECM_CLASSIFIER_RELEVANCE_NO) {
			ecm_classifier_type_t aci_type;

			/*
			 * This classifier can be unassigned - PROVIDED it is not the default classifier
			 */
			aci_type = aci->type_get(aci);
			if (aci_type == ECM_CLASSIFIER_TYPE_DEFAULT) {
				continue;
			}

			DEBUG_INFO("%px: Classifier not relevant, unassign: %d", ci, aci_type);
			ecm_db_connection_classifier_unassign(ci, aci);
			continue;
		}

		/*
		 * Yes or Maybe relevant.
		 */
		if (aci_pr.process_actions & ECM_CLASSIFIER_PROCESS_ACTION_DROP) {
			/*
			 * Drop command from any classifier is actioned.
			 */
			DEBUG_TRACE("%px: wants drop: %px, type: %d, skb: %px\n", ci, aci, aci->type_get(aci), skb);
			prevalent_pr.drop |= aci_pr.drop;
		}

		/*
		 * Accel mode permission
		 */
		if (aci_pr.relevance == ECM_CLASSIFIER_RELEVANCE_MAYBE) {
			/*
			 * Classifier not sure of its relevance - cannot accel yet
			 */
			DEBUG_TRACE("%px: accel denied by maybe: %px, type: %d\n", ci, aci, aci->type_get(aci));
			prevalent_pr.accel_mode = ECM_CLASSIFIER_ACCELERATION_MODE_NO;
		} else {
			if (aci_pr.process_actions & ECM_CLASSIFIER_PROCESS_ACTION_ACCEL_MODE) {
				if (aci_pr.accel_mode == ECM_CLASSIFIER_ACCELERATION_MODE_NO) {
					DEBUG_TRACE("%px: accel denied: %px, type: %d\n", ci, aci, aci->type_get(aci));
					prevalent_pr.accel_mode = ECM_CLASSIFIER_ACCELERATION_MODE_NO;
				}
				/* else yes or don't care about accel */
			}
		}

		/*
		 * Timer group (the last classifier i.e. the highest priority one) will 'win'
		 */
		if (aci_pr.process_actions & ECM_CLASSIFIER_PROCESS_ACTION_TIMER_GROUP) {
			DEBUG_TRACE("%px: timer group: %px, type: %d, group: %d\n", ci, aci, aci->type_get(aci), aci_pr.timer_group);
			prevalent_pr.timer_group = aci_pr.timer_group;
		}

		/*
		 * Qos tag (the last classifier i.e. the highest priority one) will 'win'
		 */
		if (aci_pr.process_actions & ECM_CLASSIFIER_PROCESS_ACTION_QOS_TAG) {
			DEBUG_TRACE("%px: aci: %px, type: %d, flow qos tag: %u, return qos tag: %u\n",
					ci, aci, aci->type_get(aci), aci_pr.flow_qos_tag, aci_pr.return_qos_tag);
			prevalent_pr.flow_qos_tag = aci_pr.flow_qos_tag;
			prevalent_pr.return_qos_tag = aci_pr.return_qos_tag;
			prevalent_pr.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_QOS_TAG;
		}

#ifdef ECM_CLASSIFIER_DSCP_ENABLE
#ifdef ECM_CLASSIFIER_DSCP_IGS
		/*
		 * Ingress QoS tag
		 */
		if (aci_pr.process_actions & ECM_CLASSIFIER_PROCESS_ACTION_IGS_QOS_TAG) {
			DEBUG_TRACE("%px: aci: %px, type: %d, ingress flow qos tag: %u, ingress return qos tag: %u\n",
					ci, aci, aci->type_get(aci), aci_pr.igs_flow_qos_tag, aci_pr.igs_return_qos_tag);
			prevalent_pr.igs_flow_qos_tag = aci_pr.igs_flow_qos_tag;
			prevalent_pr.igs_return_qos_tag = aci_pr.igs_return_qos_tag;
			prevalent_pr.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_IGS_QOS_TAG;
		}
#endif
		/*
		 * If any classifier denied DSCP remarking then that overrides every classifier
		 */
		if (aci_pr.process_actions & ECM_CLASSIFIER_PROCESS_ACTION_DSCP_DENY) {
			DEBUG_TRACE("%px: aci: %px, type: %d, DSCP remark denied\n",
					ci, aci, aci->type_get(aci));
			prevalent_pr.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_DSCP_DENY;
			prevalent_pr.process_actions &= ~ECM_CLASSIFIER_PROCESS_ACTION_DSCP;
		}

		/*
		 * DSCP remark action, but only if it has not been denied by any classifier
		 */
		if (aci_pr.process_actions & ECM_CLASSIFIER_PROCESS_ACTION_DSCP) {
			if (!(prevalent_pr.process_actions & ECM_CLASSIFIER_PROCESS_ACTION_DSCP_DENY)) {
				DEBUG_TRACE("%px: aci: %px, type: %d, DSCP remark wanted, flow_dscp: %u, return dscp: %u\n",
						ci, aci, aci->type_get(aci), aci_pr.flow_dscp, aci_pr.return_dscp);
				prevalent_pr.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_DSCP;
				prevalent_pr.flow_dscp = aci_pr.flow_dscp;
				prevalent_pr.return_dscp = aci_pr.return_dscp;
			}
		}

		if (aci_pr.process_actions & ECM_CLASSIFIER_PROCESS_ACTION_MARK) {
			DEBUG_TRACE("%px: aci: %px, type: %d, flow mark: %u, return mark: %u\n",
					ci, aci, aci->type_get(aci), aci_pr.flow_mark, aci_pr.return_mark);
			prevalent_pr.flow_mark = aci_pr.flow_mark;
			prevalent_pr.return_mark = aci_pr.return_mark;
			prevalent_pr.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_MARK;
		}
#endif

#ifdef ECM_CLASSIFIER_EMESH_ENABLE
		/*
		 * E-Mesh Service Prioritization is Valid
		 */
		if (aci_pr.process_actions & ECM_CLASSIFIER_PROCESS_ACTION_EMESH_SP_FLOW) {
			DEBUG_TRACE("%px: aci: %px, type: %d, E-Mesh Service Prioritization is valid\n",
					ci, aci, aci->type_get(aci));
			prevalent_pr.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_EMESH_SP_FLOW;
		}
#endif

#ifdef ECM_CLASSIFIER_PCC_ENABLE
		if (aci_pr.process_actions & ECM_CLASSIFIER_PROCESS_ACTION_ACL_ENABLED) {
			DEBUG_TRACE("%px: aci: %px, type: %d, flow: %d"
					" return: %d\n",
					ci, aci, aci->type_get(aci),
					aci_pr.rule_id.acl.flow_acl_id,
					aci_pr.rule_id.acl.return_acl_id);
			prevalent_pr.rule_id.acl.flow_acl_id = aci_pr.rule_id.acl.flow_acl_id;
			prevalent_pr.rule_id.acl.return_acl_id = aci_pr.rule_id.acl.return_acl_id;
			prevalent_pr.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_ACL_ENABLED;
		}

		if (aci_pr.process_actions & ECM_CLASSIFIER_PROCESS_ACTION_POLICER_ENABLED) {
			DEBUG_TRACE("%px: aci: %px, type: %d, flow: %d"
					" return: %d\n",
					ci, aci, aci->type_get(aci),
					aci_pr.rule_id.policer.flow_policer_id,
					aci_pr.rule_id.policer.return_policer_id);
			prevalent_pr.rule_id.policer.flow_policer_id = aci_pr.rule_id.policer.flow_policer_id;
			prevalent_pr.rule_id.policer.return_policer_id = aci_pr.rule_id.policer.return_policer_id;
			prevalent_pr.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_POLICER_ENABLED;
		}
#endif
	}

	ecm_db_connection_assignments_release(assignment_count, assignments);

	/*
	 * Change timer group?
	 */
	if (ci_orig_timer_group != prevalent_pr.timer_group) {
		DEBUG_TRACE("%px: change timer group from: %d to: %d\n", ci, ci_orig_timer_group, prevalent_pr.timer_group);
		ecm_db_connection_defunct_timer_reset(ci, prevalent_pr.timer_group);
	}

	/*
	 * Drop?
	 */
	if (prevalent_pr.drop) {
		DEBUG_TRACE("%px: drop: %px\n", ci, skb);
		ecm_db_connection_data_totals_update_dropped(ci, (sender == ECM_TRACKER_SENDER_TYPE_SRC)? true : false, skb->len, 1);
		ecm_db_connection_deref(ci);
		ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_NON_PORTED, ECM_STATS_V4_EXCEPTION_NON_PORTED_DROP_BY_CLASSIFIER);
		return NF_ACCEPT;
	}
	ecm_db_connection_data_totals_update(ci, (sender == ECM_TRACKER_SENDER_TYPE_SRC)? true : false, skb->len, 1);

	/*
	 * Assign qos tag
	 * GGG TODO Should we use sender to identify whether to use flow or return qos tag?
	 */
	skb->priority = prevalent_pr.flow_qos_tag;
	DEBUG_TRACE("%px: skb priority: %u\n", ci, skb->priority);

#ifdef ECM_INTERFACE_SIT_ENABLE
#ifdef CONFIG_IPV6_SIT_6RD
	/*
	 * SIT tunnel acceleration needs create a rule to the acceleration engine if the
	 *	tunnel's dest ip address is empty,it will get dest ip and the embedded ipv6's dest ip
	 *	address in the packet and send them to the nss firmware to accelerate the
	 *	traffic on the tun6rd interface.
	 */
	if (protocol == IPPROTO_IPV6
			&& prevalent_pr.accel_mode == ECM_CLASSIFIER_ACCELERATION_MODE_ACCEL) {
		DEBUG_TRACE("%px: accel\n", ci);
		feci = ecm_db_connection_front_end_get_and_ref(ci);
#ifdef ECM_FRONT_END_NSS_ENABLE
		if (feci->accel_engine == ECM_FRONT_END_ENGINE_NSS) {
			ecm_nss_non_ported_ipv4_sit_set_peer(feci, skb);
		}
#endif

#ifdef ECM_FRONT_END_SFE_ENABLE
		if (feci->accel_engine == ECM_FRONT_END_ENGINE_SFE) {
			ecm_sfe_non_ported_ipv4_sit_set_peer(feci, skb);
		}
#endif

#ifdef ECM_FRONT_END_PPE_ENABLE
		if (feci->accel_engine == ECM_FRONT_END_ENGINE_PPE) {
			ecm_ppe_non_ported_ipv4_sit_set_peer(feci, skb);
		}
#endif

		ecm_front_end_connection_deref(feci);
	}
#endif
#endif
	/*
	 * Accelerate?
	 */
	if (prevalent_pr.accel_mode == ECM_CLASSIFIER_ACCELERATION_MODE_ACCEL) {
		DEBUG_TRACE("%px: accel\n", ci);
		feci = ecm_db_connection_front_end_get_and_ref(ci);
		feci->accelerate(feci, &prevalent_pr, is_l2_encap, ct, skb);
		ecm_front_end_connection_deref(feci);
	}
	ecm_db_connection_deref(ci);

	return NF_ACCEPT;
}
