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
#define DEBUG_LEVEL ECM_CMN_MULTICAST_IPV6_DEBUG_LEVEL

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

#ifdef ECM_FRONT_END_NSS_ENABLE
#include <nss_api_if.h>
#include "ecm_nss_ipv6.h"
#include "ecm_nss_multicast_ipv6.h"
#include "ecm_nss_common.h"
#else
#include "ecm_sfe_multicast_ipv6.h"
#endif
#include "ecm_front_end_common.h"
#include "ecm_ipv6.h"
#include "ecm_ae_classifier_public.h"
#include "ecm_ae_classifier.h"

/*
 * General operational control
 */
int ecm_front_end_ipv6_mc_stopped = 0;	/* When non-zero further traffic will not be processed */

/*
 * ecm_multicast_ipv6_interface_heirarchy_construct()
 *   Create destination multicast interface heirachy for Route/Bridge connection flow
 */
static int ecm_multicast_ipv6_interface_heirarchy_construct(struct ecm_front_end_connection_instance *feci,
								struct ecm_db_iface_instance *interfaces, struct net_device *in_dev,
								struct net_device *brdev, ip_addr_t packet_src_addr, ip_addr_t packet_dest_addr, uint8_t max_if,
								uint32_t *dst_dev, int32_t *to_interface_first, uint8_t *src_node_addr, bool is_routed,
								struct sk_buff *skb)
{
	int32_t iface_instance_cnt;

	if (is_routed) {
		iface_instance_cnt = ecm_interface_multicast_heirarchy_construct_routed(feci, interfaces, in_dev, packet_src_addr,
											packet_dest_addr, max_if, dst_dev, to_interface_first,
											false, NULL, skb);
	} else {
		iface_instance_cnt = ecm_interface_multicast_heirarchy_construct_bridged(feci, interfaces, brdev,
											 packet_src_addr, packet_dest_addr, max_if,
											 dst_dev, to_interface_first, src_node_addr, NULL, skb);
	}
	return iface_instance_cnt;
}

/*
 * ecm_multicast_ipv6_connection_regenerate()
 *	Re-generate a connection.
 *
 * Re-generating a connection involves re-evaluating the interface lists in case interface heirarchies have changed.
 * It also involves the possible triggering of classifier re-evaluation but only if all currently assigned
 * classifiers permit this operation.
 */
static void ecm_multicast_ipv6_connection_regenerate(struct ecm_db_connection_instance *ci, ecm_tracker_sender_type_t sender,
							struct net_device *out_dev, struct net_device *in_dev, struct net_device *in_dev_nat)
{
	int i;
	bool reclassify_allowed;
	struct ecm_db_iface_instance *from_list[ECM_DB_IFACE_HEIRARCHY_MAX];
	int32_t from_list_first;
	struct ecm_db_iface_instance *from_nat_list[ECM_DB_IFACE_HEIRARCHY_MAX];
	int32_t from_nat_list_first;
	ip_addr_t ip_src_addr;
	ip_addr_t ip_dest_addr;
	ip_addr_t ip_src_addr_nat;
	int protocol;
	bool is_routed;
	uint8_t src_node_addr[ETH_ALEN];
	uint8_t dest_node_addr[ETH_ALEN];
	uint8_t src_node_addr_nat[ETH_ALEN];
	int assignment_count;
	struct ecm_classifier_instance *assignments[ECM_CLASSIFIER_TYPES];
	struct ecm_front_end_connection_instance *feci;
	__be16 layer4hdr[2] = {0, 0};
	__be16 port = 0;

	/*
	 * Update the interface lists - these may have changed, e.g. LAG path change etc.
	 * NOTE: We never have to change the usual mapping->host->node_iface arrangements for each side of the connection (to/from sides)
	 * This is because if these interfaces change then the connection is dead anyway.
	 * But a LAG slave might change the heirarchy the connection is using but the LAG master is still sane.
	 * If any of the new interface heirarchies cannot be created then simply set empty-lists as this will deny
	 * acceleration and ensure that a bad rule cannot be created.
	 * IMPORTANT: The 'sender' defines who has sent the packet that triggered this re-generation
	 */
	protocol = ecm_db_connection_protocol_get(ci);
	is_routed = ecm_db_connection_is_routed_get(ci);

	ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_FROM, ip_src_addr);
	ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_FROM_NAT, ip_src_addr_nat);
	ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_TO, ip_dest_addr);
	ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_FROM, src_node_addr);
	ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_TO, dest_node_addr);
	ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_FROM_NAT, src_node_addr_nat);

	port = (__be16)(ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_FROM));
	layer4hdr[0] = htons(port);
	port = (__be16)(ecm_db_connection_port_get(ci, ECM_DB_OBJ_DIR_TO));
	layer4hdr[1] = htons(port);

	feci = ecm_db_connection_front_end_get_and_ref(ci);

	DEBUG_TRACE("%px: Update the 'from' interface heirarchy list\n", ci);
	from_list_first = ecm_interface_multicast_from_heirarchy_construct(feci, from_list, ip_dest_addr, ip_src_addr, 6, protocol, in_dev, is_routed, in_dev, src_node_addr, dest_node_addr, layer4hdr, NULL);
	if (from_list_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
		goto ecm_multicast_ipv6_retry_regen;
	}

	ecm_db_connection_interfaces_reset(ci, from_list, from_list_first, ECM_DB_OBJ_DIR_FROM);
	ecm_db_connection_interfaces_deref(from_list, from_list_first);

	DEBUG_TRACE("%px: Update the 'from NAT' interface heirarchy list\n", ci);
	from_nat_list_first = ecm_interface_multicast_from_heirarchy_construct(feci, from_nat_list, ip_dest_addr, ip_src_addr_nat, 4, protocol, in_dev_nat, is_routed, in_dev_nat, src_node_addr_nat, dest_node_addr, layer4hdr, NULL);
	if (from_nat_list_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
		goto ecm_multicast_ipv6_retry_regen;
	}

	ecm_db_connection_interfaces_reset(ci, from_nat_list, from_nat_list_first, ECM_DB_OBJ_DIR_FROM_NAT);
	ecm_db_connection_interfaces_deref(from_nat_list, from_nat_list_first);


	ecm_front_end_connection_deref(feci);

	/*
	 * Get list of assigned classifiers to reclassify.
	 * Remember: This also includes our default classifier too.
	 */
	assignment_count = ecm_db_connection_classifier_assignments_get_and_ref(ci, assignments);

	/*
	 * All of the assigned classifiers must permit reclassification.
	 */
	reclassify_allowed = true;
	for (i = 0; i < assignment_count; ++i) {
		DEBUG_TRACE("%px: Calling to reclassify: %px, type: %d\n", ci, assignments[i], assignments[i]->type_get(assignments[i]));
		if (!assignments[i]->reclassify_allowed(assignments[i])) {
			DEBUG_TRACE("%px: reclassify denied: %px, by type: %d\n", ci, assignments[i], assignments[i]->type_get(assignments[i]));
			reclassify_allowed = false;
			break;
		}
	}

	/*
	 * Now we action any classifier re-classify
	 */
	if (!reclassify_allowed) {
		/*
		 * Regeneration came to a successful conclusion even though reclassification was denied
		 */
		DEBUG_WARN("%px: re-classify denied\n", ci);
		goto ecm_multicast_ipv6_regen_done;
	}

	/*
	 * Reclassify
	 */
	DEBUG_INFO("%px: reclassify\n", ci);
	if (!ecm_classifier_reclassify(ci, assignment_count, assignments)) {
		/*
		 * We could not set up the classifiers to reclassify, it is safer to fail out and try again next time
		 */
		DEBUG_WARN("%px: Regeneration: reclassify failed\n", ci);
		goto ecm_multicast_ipv6_regen_done;
	}
	DEBUG_INFO("%px: reclassify success\n", ci);

ecm_multicast_ipv6_regen_done:

	/*
	 * Release the assignments
	 */
	ecm_db_connection_assignments_release(assignment_count, assignments);

	/*
	 * Re-generation of state is successful.
	 */
	ecm_db_connection_regeneration_completed(ci);

	return;

ecm_multicast_ipv6_retry_regen:
	ecm_front_end_connection_deref(feci);
	ecm_db_connection_regeneration_failed(ci);
	return;
}

/*
 * ecm_multicast_ipv6_node_establish_and_ref()
 *	Returns a reference to a node, possibly creating one if necessary.
 *
 * The given_node_addr will be used if provided.
 *
 * Returns NULL on failure.
 *
 * TODO: This function should be removed and the one in the ecm_ipv6.c file should be used
 *	instead of this one when the multicast code is fixed to use the new interface hierarchy
 *	construction model.
 */
static struct ecm_db_node_instance *ecm_multicast_ipv6_node_establish_and_ref(struct ecm_front_end_connection_instance *feci,
							struct net_device *dev, ip_addr_t addr,
							struct ecm_db_iface_instance *interface_list[], int32_t interface_list_first,
							uint8_t *given_node_addr, struct sk_buff *skb)
{
	struct ecm_db_node_instance *ni;
	struct ecm_db_node_instance *nni;
	struct ecm_db_iface_instance *ii;
	int i;
	bool done;
	uint8_t node_addr[ETH_ALEN];
#ifdef ECM_INTERFACE_L2TPV2_ENABLE
	ip_addr_t remote_ip;
#endif

	DEBUG_INFO("Establish node for " ECM_IP_ADDR_OCTAL_FMT "\n", ECM_IP_ADDR_TO_OCTAL(addr));

	/*
	 * The node is the datalink address, typically a MAC address.
	 * However the node address to use is not always obvious and depends on the interfaces involved.
	 * For example if the interface is PPPoE then we use the MAC of the PPPoE server as we cannot use normal ARP resolution.
	 * Not all hosts have a node address, where there is none, a suitable alternative should be located and is typically based on 'addr'
	 * or some other datalink session information.
	 * It should be, at a minimum, something that ties the host with the interface.
	 *
	 * Iterate from 'inner' to 'outer' interfaces - discover what the node is.
	 */
	memset(node_addr, 0, ETH_ALEN);
	done = false;
	if (given_node_addr) {
		memcpy(node_addr, given_node_addr, ETH_ALEN);
		done = true;
		DEBUG_TRACE("Using given node address: %pM\n", node_addr);
	}
	for (i = ECM_DB_IFACE_HEIRARCHY_MAX - 1; (!done) && (i >= interface_list_first); i--) {
		ecm_db_iface_type_t type;
		ip_addr_t gw_addr = ECM_IP_ADDR_NULL;
		bool on_link = false;
#ifdef ECM_INTERFACE_PPPOE_ENABLE
		struct ecm_db_interface_info_pppoe pppoe_info;
#endif
#ifdef ECM_INTERFACE_L2TPV2_ENABLE
		struct ecm_db_interface_info_pppol2tpv2 pppol2tpv2_info;
#endif
		type = ecm_db_iface_type_get(interface_list[i]);
		DEBUG_INFO("Lookup node address, interface @ %d is type: %d\n", i, type);

		switch (type) {

		case ECM_DB_IFACE_TYPE_PPPOE:
#ifdef ECM_INTERFACE_PPPOE_ENABLE
			/*
			 * Node address is the address of the remote PPPoE server
			 */
			ecm_db_iface_pppoe_session_info_get(interface_list[i], &pppoe_info);
			memcpy(node_addr, pppoe_info.remote_mac, ETH_ALEN);
			done = true;
			break;
#else
			DEBUG_TRACE("PPPoE interface unsupported\n");
			return NULL;
#endif

		case ECM_DB_IFACE_TYPE_SIT:
		case ECM_DB_IFACE_TYPE_TUNIPIP6:
			done = true;
			break;

		case ECM_DB_IFACE_TYPE_PPPOL2TPV2:
#ifdef ECM_INTERFACE_L2TPV2_ENABLE
			ecm_db_iface_pppol2tpv2_session_info_get(interface_list[i], &pppol2tpv2_info);
			ECM_HIN4_ADDR_TO_IP_ADDR(remote_ip, pppol2tpv2_info.ip.daddr);
			if (unlikely(!ecm_interface_mac_addr_get(remote_ip, node_addr, &on_link, gw_addr))) {
				DEBUG_TRACE("Failed to obtain mac for host " ECM_IP_ADDR_OCTAL_FMT "\n", ECM_IP_ADDR_TO_OCTAL(addr));
				return NULL;
			}
			done = true;
			break;
#else
			DEBUG_TRACE("PPPoL2TPV2 interface unsupported\n");
			return NULL;
#endif
		case ECM_DB_IFACE_TYPE_VLAN:
#ifdef ECM_INTERFACE_VLAN_ENABLE
			/*
			 * VLAN handled same along with bridge etc.
			 */
#else
			DEBUG_TRACE("VLAN interface unsupported\n");
			return NULL;
#endif
		case ECM_DB_IFACE_TYPE_ETHERNET:
		case ECM_DB_IFACE_TYPE_LAG:
		case ECM_DB_IFACE_TYPE_BRIDGE:
#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
		case ECM_DB_IFACE_TYPE_OVS_BRIDGE:
#endif
			if (!ecm_interface_mac_addr_get(addr, node_addr, &on_link, gw_addr)) {
				DEBUG_TRACE("Failed to obtain mac for host " ECM_IP_ADDR_OCTAL_FMT "\n", ECM_IP_ADDR_TO_OCTAL(addr));

				/*
				 * If there is a gw_addr found during the lookup, use that address
				 * for neighbour solicitation request.
				 */
				if (!ECM_IP_ADDR_IS_NULL(gw_addr)) {
					ECM_IP_ADDR_COPY(addr, gw_addr);
				}

				if (!ecm_ipv6_dev_has_ipaddr(dev) && ecm_front_end_is_bridge_port(dev)) {
					struct net_device *master;
					master = ecm_interface_get_and_hold_dev_master(dev);
					if (!master) {
						DEBUG_WARN("Expected a master\n");
						return NULL;
					}

					ecm_interface_send_neighbour_solicitation(master, addr);
					dev_put(master);
				} else {
					ecm_interface_send_neighbour_solicitation(dev, addr);
				}
				return NULL;
			}
			if (is_multicast_ether_addr(node_addr)) {
				DEBUG_TRACE("multicast node address for host " ECM_IP_ADDR_OCTAL_FMT ", node_addr: %pM\n", ECM_IP_ADDR_TO_OCTAL(addr), node_addr);
				return NULL;
			}

			done = true;
			break;
		default:
			/*
			 * Don't know how to handle these.
			 * Just copy some part of the address for now, but keep iterating the interface list
			 * in the hope something recognisable will be seen!
			 * GGG TODO We really need to roll out support for all interface types we can deal with ASAP :-(
			 */
			memcpy(node_addr, (uint8_t *)addr, ETH_ALEN);
		}
	}
	if (!done) {
		DEBUG_INFO("Failed to establish node for " ECM_IP_ADDR_OCTAL_FMT "\n", ECM_IP_ADDR_TO_OCTAL(addr));
		return NULL;
	}

	/*
	 * Establish iface
	 */
	ii = ecm_interface_establish_and_ref(feci, dev, skb);
	if (!ii) {
		DEBUG_WARN("Failed to establish iface\n");
		return NULL;
	}

	/*
	 * Locate the node
	 */
	ni = ecm_db_node_find_and_ref(node_addr, ii);
	if (ni) {
		DEBUG_TRACE("%px: node established\n", ni);
		ecm_db_iface_deref(ii);
		return ni;
	}

	/*
	 * No node - create one
	 */
	nni = ecm_db_node_alloc();
	if (!nni) {
		DEBUG_WARN("Failed to establish node\n");
		ecm_db_iface_deref(ii);
		return NULL;
	}

	/*
	 * Add node into the database, atomically to avoid races creating the same thing
	 */
	spin_lock_bh(&ecm_ipv6_lock);
	ni = ecm_db_node_find_and_ref(node_addr, ii);
	if (ni) {
		spin_unlock_bh(&ecm_ipv6_lock);
		ecm_db_node_deref(nni);
		ecm_db_iface_deref(ii);
		return ni;
	}

	ecm_db_node_add(nni, ii, node_addr, NULL, nni);
	spin_unlock_bh(&ecm_ipv6_lock);

	/*
	 * Don't need iface instance now
	 */
	ecm_db_iface_deref(ii);

	DEBUG_TRACE("%px: node established\n", nni);
	return nni;
}

/*
 * ecm_multicast_ipv6_connection_process()
 *	Process a UDP Multicast packet
 */
unsigned int ecm_multicast_ipv6_connection_process(struct net_device *out_dev,
							struct net_device *in_dev,
							uint8_t *src_node_addr,
							uint8_t *dest_node_addr,
							bool can_accel, bool is_routed, struct sk_buff *skb,
							struct ecm_tracker_ip_header *iph,
							struct nf_conn *ct, ecm_tracker_sender_type_t sender,
							struct nf_conntrack_tuple *orig_tuple, struct nf_conntrack_tuple *reply_tuple)
{
	struct udphdr *udp_hdr;
	struct udphdr udp_hdr_buff;
	struct in6_addr origin6;
	struct in6_addr group6;
	struct ecm_db_multicast_tuple_instance *tuple_instance;
	int src_port;
	int dest_port;
	int src_port_nat = 0;
	struct net_device *in_dev_nat = NULL;
	struct ecm_db_connection_instance *ci;
	ecm_db_direction_t ecm_dir = ECM_DB_DIRECTION_NON_NAT;
	ip_addr_t match_addr;
	struct ecm_classifier_instance *assignments[ECM_CLASSIFIER_TYPES];
	int aci_index;
	int32_t mc_if_cnt;
	int assignment_count;
	ecm_db_timer_group_t ci_orig_timer_group;
	struct ecm_classifier_process_response prevalent_pr;
	ip_addr_t ip_src_addr;
	ip_addr_t ip_dest_addr;
	ip_addr_t ip_src_addr_nat;
	uint32_t mc_dest_if[ECM_DB_MULTICAST_IF_MAX];
	bool br_dev_found_in_mfc = false;
	int protocol = (int)orig_tuple->dst.protonum;
	__be16 *layer4hdr = NULL;
	struct net_device *out_dev_master = NULL;
	struct net_device *l3_br_dev = NULL;
	ip_addr_t from_nat_mac_lookup;

	if (protocol != IPPROTO_UDP) {
		DEBUG_WARN("Invalid Protocol %d in skb %px\n", protocol, skb);
		return NF_ACCEPT;
	}

	/*
	 * Extract UDP header to obtain port information
	 */
	udp_hdr = ecm_tracker_udp_check_header_and_read(skb, iph, &udp_hdr_buff);
	if (unlikely(!udp_hdr)) {
		DEBUG_WARN("Invalid UDP header in skb %px\n", skb);
		return NF_ACCEPT;
	}

	/*
	 * Return if source dev is any tunnel type
	 *
	 * Acceleration for multicast packet which is sent to
	 * or received from VxLAN tunnel net device should be skipped.
	 */
	if ((in_dev->type == ECM_ARPHRD_IPSEC_TUNNEL_TYPE) ||
	    (in_dev->type == ARPHRD_SIT) || (in_dev->type == ARPHRD_PPP) ||
	    (in_dev->type == ARPHRD_TUNNEL6) ||
	    (netif_is_vxlan(in_dev)) || (netif_is_vxlan(out_dev))) {
		DEBUG_TRACE("in_dev: %px, in_type: %d, out_dev: %px, out_type: %d",
				in_dev, in_dev->type, out_dev, out_dev->type);
		return NF_ACCEPT;
	}

	/*
	 * Now extract information, if we have conntrack then use that (which would already be in the tuples)
	 */
	if (unlikely(!ct)) {
		orig_tuple->src.u.udp.port = udp_hdr->source;
		orig_tuple->dst.u.udp.port = udp_hdr->dest;
		reply_tuple->src.u.udp.port = udp_hdr->dest;
		reply_tuple->dst.u.udp.port = udp_hdr->source;
	}

	layer4hdr = (__be16*)udp_hdr;
	/*
	 * Extract transport port information
	 * Refer to the ecm_ipv6_process() for information on how we extract this information.
	 */
	src_port = ntohs(orig_tuple->src.u.udp.port);
	dest_port = ntohs(orig_tuple->dst.u.udp.port);
	src_port_nat = ntohs(reply_tuple->dst.u.udp.port);

	ECM_NIN6_ADDR_TO_IP_ADDR(ip_src_addr, orig_tuple->src.u3.in6);
	ECM_NIN6_ADDR_TO_IP_ADDR(ip_dest_addr, orig_tuple->dst.u3.in6);
	ECM_NIN6_ADDR_TO_IP_ADDR(ip_src_addr_nat, reply_tuple->dst.u3.in6);
	ECM_IP_ADDR_COPY(from_nat_mac_lookup, ip_src_addr_nat);

	/*
	 * Query MFC/Bridge Snooper to access the destination interface list.
	 */
	ECM_IP_ADDR_TO_NIN6_ADDR(origin6, ip_src_addr);
	ECM_IP_ADDR_TO_NIN6_ADDR(group6, ip_dest_addr);

	/*
	 * Initialize in_dev_nat to default in_dev.
	 */
	in_dev_nat = in_dev;

	memset(mc_dest_if, 0, sizeof(mc_dest_if));
	mc_if_cnt =  ip6mr_find_mfc_entry(&init_net, &origin6, &group6, ECM_DB_MULTICAST_IF_MAX, mc_dest_if);

	/*
	 * Skip PPP interface
	 */
	if (ecm_interface_multicast_is_iface_type(mc_dest_if, mc_if_cnt, ARPHRD_PPP)) {
		DEBUG_TRACE("%px: Packet is of type PPP; skip it\n", skb);
		return NF_ACCEPT;
	}

	if (is_routed) {
		/*
		 * This is a routed flow, hence look for a valid MFC rule
		 */
		if (mc_if_cnt <= 0){
			DEBUG_WARN("Not found a valid vif count %d\n", mc_if_cnt);
			return NF_ACCEPT;
		}
	}

	/*
	 * For "bridge + route" scenario, we can receive packet from both bridge and route hook.
	 * If MFC returns valid interface count (mc_if_cnt), then we need to check for "bridge + route"
	 * scenario.
	 */
	if (mc_if_cnt > 0) {
		is_routed = true;

		/*
		 * Check for the presence of a bridge device in the destination
		 * interface list given to us by MFC
		 */
		br_dev_found_in_mfc = ecm_interface_multicast_check_for_br_dev(mc_dest_if, mc_if_cnt);

		/*
		 * We are processing a routed multicast flow.
		 * Check if the source interface is a bridge device. If this is the case,
		 * this flow could be a "bridge + route" flow.
		 * So, we query the bridge device as well for possible joinees
		 */
		if (ecm_front_end_is_bridge_device(in_dev)
#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
				|| ecm_front_end_is_ovs_bridge_device(in_dev)
#endif
		   ) {
			int32_t mc_if_cnt_bridge;
			uint32_t dst_dev_bridge[ECM_DB_MULTICAST_IF_MAX];

			l3_br_dev = in_dev;
			memset(dst_dev_bridge, 0, sizeof(dst_dev_bridge));
			mc_if_cnt_bridge = mc_bridge_ipv6_get_if(in_dev, &origin6, &group6, ECM_DB_MULTICAST_IF_MAX, dst_dev_bridge);
			if (mc_if_cnt_bridge <= 0) {
				DEBUG_WARN("%px: No bridge ports have joined multicast group\n", ci);
				goto process_packet;
			}

			/*
			 * Check for max interface limit.
			 */
			if (mc_if_cnt == ECM_DB_MULTICAST_IF_MAX) {
				DEBUG_WARN("Interface count reached max limit: %d. Could not handle the connection", mc_if_cnt);
				goto done;
			}

			/*
			 * Update the dst_dev with in_dev as it is a bridge + route case
			 */
			mc_dest_if[mc_if_cnt++] = in_dev->ifindex;
			br_dev_found_in_mfc = true;
		}

		goto process_packet;
	}

	/*
	 * Packet flow is pure bridge. Try to query the snooper for the destination
	 * interface list
	 */
	out_dev_master =  ecm_interface_get_and_hold_dev_master(out_dev);
	DEBUG_ASSERT(out_dev_master, "Expected a master\n");
	mc_if_cnt = mc_bridge_ipv6_get_if(out_dev_master, &origin6, &group6, ECM_DB_MULTICAST_IF_MAX, mc_dest_if);
	if (mc_if_cnt <= 0) {
		DEBUG_WARN("Not found a valid MCS if count %d\n", mc_if_cnt);
		goto done;
	}

	/*
	 * The source interface could have joined the group as well.
	 * In such cases, the destination interface list returned by
	 * the snooper would include the source interface as well.
	 * We need to filter the source interface from the list in such cases.
	 */
	mc_if_cnt = ecm_interface_multicast_check_for_src_ifindex(mc_dest_if, mc_if_cnt, in_dev->ifindex);
	if (mc_if_cnt <= 0) {
		DEBUG_WARN("Not found a valid MCS if count %d\n", mc_if_cnt);
		goto done;
	}

process_packet:
	/*
	 * Work out if this packet involves NAT or not.
	 * If it does involve NAT then work out if this is an ingressing or egressing packet.
	 */
	if (!ECM_IP_ADDR_MATCH(ip_src_addr, ip_src_addr_nat)) {
		struct in6_addr nat_dev_saddr, nat_dev_daddr;
		ip_addr_t ip_nat_dev_saddr;

		/*
		 * Egressing NAT
		 * ip_src_addr_nat could be dummy address, we will get ipv6 address of out_dev
		 * and map it to from_nat_mac_lookup
		 */
		in_dev_nat = out_dev;

		ECM_IP_ADDR_TO_NIN6_ADDR(nat_dev_daddr, ip_dest_addr);
		ipv6_dev_get_saddr(dev_net(in_dev_nat), in_dev_nat, &nat_dev_daddr, 0, &nat_dev_saddr);
		ECM_NIN6_ADDR_TO_IP_ADDR(ip_nat_dev_saddr, nat_dev_saddr);
		ECM_IP_ADDR_COPY(from_nat_mac_lookup, ip_nat_dev_saddr);
	}

	DEBUG_TRACE("UDP src: " ECM_IP_ADDR_OCTAL_FMT ":%d, dest: " ECM_IP_ADDR_OCTAL_FMT ":%d\n",
			ECM_IP_ADDR_TO_OCTAL(ip_src_addr), src_port, ECM_IP_ADDR_TO_OCTAL(ip_dest_addr), dest_port);

	/*
	 * Look up a connection
	 */
	ci = ecm_db_connection_find_and_ref(ip_src_addr, ip_dest_addr, IPPROTO_UDP, src_port, dest_port);

	/*
	 * If there is no existing connection then create a new one.
	 */
	if (unlikely(!ci)) {
		struct ecm_db_mapping_instance *mi[ECM_DB_OBJ_DIR_MAX];
		struct ecm_db_node_instance *ni[ECM_DB_OBJ_DIR_MAX];
		struct ecm_classifier_default_instance *dci;
		struct ecm_front_end_connection_instance *feci;
		struct ecm_db_connection_instance *nci;
		struct ecm_db_iface_instance *from_list[ECM_DB_IFACE_HEIRARCHY_MAX];
		struct ecm_db_iface_instance *to_list;
		struct ecm_db_iface_instance *to_list_single;
		struct ecm_db_iface_instance *to_list_temp[ECM_DB_IFACE_HEIRARCHY_MAX];
		struct ecm_db_iface_instance *from_nat_list[ECM_DB_IFACE_HEIRARCHY_MAX];
		ecm_classifier_type_t classifier_type;
		int32_t from_nat_list_first;
		int32_t *to_list_first;
		int32_t *to_first;
		int32_t from_list_first;
		int32_t interface_idx_cnt = 0;
		int vif;
		int ret;
		char dest_mac_addr[6];
		ecm_ae_classifier_result_t ae_result;
		ecm_ae_classifier_get_t ae_get;

		DEBUG_TRACE("New UDP connection from " ECM_IP_ADDR_OCTAL_FMT ":%u to " ECM_IP_ADDR_OCTAL_FMT ":%u\n",
				ECM_IP_ADDR_TO_OCTAL(ip_src_addr), src_port, ECM_IP_ADDR_TO_OCTAL(ip_dest_addr), dest_port);

		/*
		 * Before we attempt to create the connection are we being terminated?
		 */
		spin_lock_bh(&ecm_ipv6_lock);
		if (ecm_ipv6_terminate_pending) {
			spin_unlock_bh(&ecm_ipv6_lock);
			DEBUG_WARN("Terminating\n");

			/*
			 * As we are terminating we just allow the packet to pass - it's no longer our concern
			 */
			goto done;
		}
		spin_unlock_bh(&ecm_ipv6_lock);

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
						  src_port, dest_port, protocol, 6,
						  is_routed, true,
						  &ae_info);
			ae_result = ae_get(&ae_info);
		}
		rcu_read_unlock();

		DEBUG_TRACE("front end type: %d ae_result: %d\n", ecm_front_end_type_get(), ae_result);

		/*
		 * Which AE can be used for this flow.
		 * When NSS enabled, use NSS only, otherwise use SFE instead
		 * 1. If NSS/SFE or DONT_CARE, allocate NSS/SFE ipv6 multicast connection instance
		 * 2. If NONE, allocate NSS/SFE ipv4 multicast connection instance with
		 *    can_accel flag set to false. By doing this we will not try to re-evaluate this flow again.
		 * 3. If NOT_YET, the connection will not be allocated in the database and the next flow will be
		 *    re-evaluated.
		 * 4. If any other type, return NF_ACCEPT.
		 */
		switch (ae_result) {
#ifdef ECM_FRONT_END_NSS_ENABLE
		case ECM_AE_CLASSIFIER_RESULT_NSS:
		case ECM_AE_CLASSIFIER_RESULT_DONT_CARE:
			if (ecm_dir == ECM_DB_DIRECTION_EGRESS_NAT) {
				goto done;
			}
			feci = ecm_nss_multicast_ipv6_connection_instance_alloc(can_accel, &nci);
			break;

		case ECM_AE_CLASSIFIER_RESULT_NONE:
			if (ecm_dir == ECM_DB_DIRECTION_EGRESS_NAT) {
				goto done;
			}
			feci = ecm_nss_multicast_ipv6_connection_instance_alloc(false, &nci);
			break;
#else
		case ECM_AE_CLASSIFIER_RESULT_SFE:
		case ECM_AE_CLASSIFIER_RESULT_DONT_CARE:
			feci = ecm_sfe_multicast_ipv6_connection_instance_alloc(can_accel, &nci);
			break;

		case ECM_AE_CLASSIFIER_RESULT_NONE:
			feci = ecm_sfe_multicast_ipv6_connection_instance_alloc(false, &nci);
			break;
#endif
		case ECM_AE_CLASSIFIER_RESULT_NOT_YET:
			DEBUG_TRACE("AE classifier hasn't decided yet for the acceleration engine\n");
			goto done;

		default:
			DEBUG_WARN("unexpected ae_result: %d\n", ae_result);
			goto done;
		}

		if (!feci) {
			DEBUG_WARN("Failed to allocate front end\n");
			goto done;
		}

		/*
		 * Create a tuple instance
		 */
		tuple_instance = ecm_db_multicast_tuple_instance_alloc(ip_src_addr, ip_dest_addr, src_port, dest_port);
		if (!tuple_instance) {
			ecm_front_end_connection_deref(feci);
			ecm_db_connection_deref(nci);
			DEBUG_WARN("Failed to allocate tuple instance\n");
			goto done;
		}
		/*
		 * Create Destination MAC address using IP multicast destination address
		 */
		ecm_translate_multicast_mac(ip_dest_addr, dest_mac_addr);

		/*
		 * Get the src and destination mappings
		 * For this we also need the interface lists which we also set upon the new connection while we are at it.
		 * GGG TODO rework terms of "src/dest" - these need to be named consistently as from/to as per database terms.
		 * GGG TODO The empty list checks should not be needed, mapping_establish_and_ref() should fail out if there is no list anyway.
		 */
		DEBUG_TRACE("%px: Create the 'from' interface heirarchy list\n", nci);
		from_list_first = ecm_interface_multicast_from_heirarchy_construct(feci, from_list, ip_dest_addr, ip_src_addr, 6, IPPROTO_UDP, in_dev, is_routed, in_dev, src_node_addr, dest_node_addr, layer4hdr, skb);
		if (from_list_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
			ecm_front_end_connection_deref(feci);
			ecm_db_connection_deref(nci);
			ecm_db_multicast_tuple_instance_deref(tuple_instance);
			DEBUG_WARN("Failed to obtain 'from' heirarchy list\n");
			goto done;
		}
		ecm_db_connection_interfaces_reset(nci, from_list, from_list_first, ECM_DB_OBJ_DIR_FROM);

		DEBUG_TRACE("%px: Create source node\n", nci);
		ni[ECM_DB_OBJ_DIR_FROM] = ecm_multicast_ipv6_node_establish_and_ref(feci, in_dev, ip_src_addr, from_list, from_list_first, src_node_addr, skb);
		ecm_db_connection_interfaces_deref(from_list, from_list_first);
		if (!ni[ECM_DB_OBJ_DIR_FROM]) {
			ecm_front_end_connection_deref(feci);
			ecm_db_connection_deref(nci);
			ecm_db_multicast_tuple_instance_deref(tuple_instance);
			DEBUG_WARN("Failed to establish source node\n");
			goto done;
		}

		DEBUG_TRACE("%px: Create source mapping\n", nci);
		mi[ECM_DB_OBJ_DIR_FROM] = ecm_ipv6_mapping_establish_and_ref(ip_src_addr, src_port);
		if (!mi[ECM_DB_OBJ_DIR_FROM]) {
			ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_FROM]);
			ecm_front_end_connection_deref(feci);
			ecm_db_connection_deref(nci);
			ecm_db_multicast_tuple_instance_deref(tuple_instance);
			DEBUG_WARN("Failed to establish src mapping\n");
			goto done;
		}

		DEBUG_TRACE("%px: Create the 'to' interface heirarchy list\n", nci);

		to_list = (struct ecm_db_iface_instance *)kzalloc(ECM_DB_TO_MCAST_INTERFACES_SIZE, GFP_ATOMIC | __GFP_NOWARN);
		if (!to_list) {
			ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_FROM]);
			ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_FROM]);
			ecm_front_end_connection_deref(feci);
			ecm_db_connection_deref(nci);
			ecm_db_multicast_tuple_instance_deref(tuple_instance);
			goto done;
		}

		to_list_first = (int32_t *)kzalloc(sizeof(int32_t *) * ECM_DB_MULTICAST_IF_MAX, GFP_ATOMIC | __GFP_NOWARN);
		if (!to_list_first) {
			ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_FROM]);
			ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_FROM]);
			ecm_front_end_connection_deref(feci);
			ecm_db_connection_deref(nci);
			ecm_db_multicast_tuple_instance_deref(tuple_instance);
			kfree(to_list);
			goto done;
		}

		/*
		 * Create the multicast 'to' interface heirarchy
		 */
		DEBUG_TRACE("%px: Create the multicast  'to' interface heirarchy list\n", nci);
		for (vif = 0; vif < ECM_DB_MULTICAST_IF_MAX; vif++) {
			to_first = ecm_db_multicast_if_first_get_at_index(to_list_first, vif);
			*to_first = ECM_DB_IFACE_HEIRARCHY_MAX;
		}

		interface_idx_cnt = ecm_multicast_ipv6_interface_heirarchy_construct(feci, to_list, in_dev, out_dev_master, ip_src_addr,
										ip_dest_addr, mc_if_cnt, mc_dest_if, to_list_first, src_node_addr,  is_routed, skb);
		if (interface_idx_cnt == 0) {
			DEBUG_WARN("Failed to obtain 'to' heirarchy list\n");
			ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_FROM]);
			ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_FROM]);
			ecm_front_end_connection_deref(feci);
			ecm_db_connection_deref(nci);
			ecm_db_multicast_tuple_instance_deref(tuple_instance);
			kfree(to_list);
			kfree(to_list_first);
			goto done;
		}

		ret = ecm_db_multicast_connection_to_interfaces_reset(nci, to_list, to_list_first);
		if (ret < 0) {
			for (vif = 0; vif < ECM_DB_MULTICAST_IF_MAX; vif++) {
				to_list_single = ecm_db_multicast_if_heirarchy_get(to_list, vif);
				ecm_db_multicast_copy_if_heirarchy(to_list_temp, to_list_single);
				to_first = ecm_db_multicast_if_first_get_at_index(to_list_first, vif);
				ecm_db_connection_interfaces_deref(to_list_temp, *to_first);
			}

			ecm_front_end_connection_deref(feci);
			ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_FROM]);
			ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_FROM]);
			ecm_db_connection_deref(nci);
			ecm_db_multicast_tuple_instance_deref(tuple_instance);
			kfree(to_list);
			kfree(to_list_first);
			DEBUG_WARN("Failed to obtain 'to' heirarchy list\n");
			goto done;
		}

		/*
		 * De-ref the destination interface list
		 */
		for (vif = 0; vif < ECM_DB_MULTICAST_IF_MAX; vif++) {
			to_list_single = ecm_db_multicast_if_heirarchy_get(to_list, vif);
			ecm_db_multicast_copy_if_heirarchy(to_list_temp, to_list_single);
			to_first = ecm_db_multicast_if_first_get_at_index(to_list_first, vif);
			ecm_db_connection_interfaces_deref(to_list_temp, *to_first);
		}

		DEBUG_TRACE("%px: Create dest node\n", nci);
		ecm_db_multicast_copy_if_heirarchy(to_list_temp, to_list);
		ni[ECM_DB_OBJ_DIR_TO] = ecm_multicast_ipv6_node_establish_and_ref(feci, out_dev, ip_dest_addr, to_list_temp, *to_list_first, dest_mac_addr, skb);
		if (!ni[ECM_DB_OBJ_DIR_TO]) {
			ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_FROM]);
			ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_FROM]);
			ecm_front_end_connection_deref(feci);
			ecm_db_connection_deref(nci);
			ecm_db_multicast_tuple_instance_deref(tuple_instance);
			kfree(to_list);
			kfree(to_list_first);
			DEBUG_WARN("Failed to establish dest node\n");
			goto done;
		}
		ni[ECM_DB_OBJ_DIR_TO_NAT] = ni[ECM_DB_OBJ_DIR_TO];

		DEBUG_TRACE("%px: Create dest mapping\n", nci);
		mi[ECM_DB_OBJ_DIR_TO] = ecm_ipv6_mapping_establish_and_ref(ip_dest_addr, dest_port);
		if (!mi[ECM_DB_OBJ_DIR_TO]) {
			ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_TO]);
			ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_FROM]);
			ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_FROM]);
			ecm_front_end_connection_deref(feci);
			ecm_db_connection_deref(nci);
			ecm_db_multicast_tuple_instance_deref(tuple_instance);
			kfree(to_list);
			kfree(to_list_first);
			DEBUG_WARN("Failed to establish dest mapping\n");
			goto done;
		}
		mi[ECM_DB_OBJ_DIR_TO_NAT] = mi[ECM_DB_OBJ_DIR_TO];

		DEBUG_TRACE("%px: Create the 'from NAT' interface heirarchy list\n", nci);
		from_nat_list_first = ecm_interface_multicast_from_heirarchy_construct(feci, from_nat_list, ip_dest_addr, from_nat_mac_lookup, 6, protocol, in_dev_nat, is_routed, in_dev_nat, src_node_addr, dest_node_addr, layer4hdr, skb);
		if (from_nat_list_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
			ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_FROM]);
			ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_TO]);
			ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_TO]);
			ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_FROM]);
			ecm_front_end_connection_deref(feci);
			ecm_db_connection_deref(nci);
			ecm_db_multicast_tuple_instance_deref(tuple_instance);
			kfree(to_list);
			kfree(to_list_first);
			DEBUG_WARN("Failed to obtain 'from NAT' heirarchy list\n");
			goto done;
		}
		ecm_db_connection_interfaces_reset(nci, from_nat_list, from_nat_list_first, ECM_DB_OBJ_DIR_FROM_NAT);

		ni[ECM_DB_OBJ_DIR_FROM_NAT] = ecm_multicast_ipv6_node_establish_and_ref(feci, in_dev_nat, from_nat_mac_lookup, from_nat_list, from_nat_list_first, src_node_addr, skb);
		ecm_db_connection_interfaces_deref(from_nat_list, from_nat_list_first);
		if (!ni[ECM_DB_OBJ_DIR_FROM_NAT]) {
			ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_FROM]);
			ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_TO]);
			ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_FROM]);
			ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_TO]);
			ecm_front_end_connection_deref(feci);
			ecm_db_connection_deref(nci);
			ecm_db_multicast_tuple_instance_deref(tuple_instance);
			kfree(to_list);
			kfree(to_list_first);
			DEBUG_WARN("Failed to obtain 'from NAT' node\n");
			goto done;
		}

		DEBUG_TRACE("%px: Create from NAT mapping\n", nci);
		mi[ECM_DB_OBJ_DIR_FROM_NAT] = ecm_ipv6_mapping_establish_and_ref(ip_src_addr_nat, src_port_nat);

		if (!mi[ECM_DB_OBJ_DIR_FROM_NAT]) {
			ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_FROM]);
			ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_TO]);
			ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_FROM_NAT]);
			ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_TO]);
			ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_FROM]);
			ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_FROM_NAT]);
			ecm_front_end_connection_deref(feci);
			ecm_db_connection_deref(nci);
			ecm_db_multicast_tuple_instance_deref(tuple_instance);
			kfree(to_list);
			kfree(to_list_first);
			DEBUG_WARN("Failed to establish from nat mapping\n");
			goto done;
		}

		/*
		 * Every connection also needs a default classifier
		 */
		dci = ecm_classifier_default_instance_alloc(nci, IPPROTO_UDP, ecm_dir, src_port, dest_port);
		if (!dci) {
			ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_TO]);
			ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_TO]);
			ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_FROM]);
			ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_FROM]);
			ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_FROM_NAT]);
			ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_FROM_NAT]);
			ecm_front_end_connection_deref(feci);
			ecm_db_connection_deref(nci);
			ecm_db_multicast_tuple_instance_deref(tuple_instance);
			kfree(to_list);
			kfree(to_list_first);
			DEBUG_WARN("Failed to allocate default classifier\n");
			goto done;
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
				dci->base.deref((struct ecm_classifier_instance *)dci);
				ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_TO]);
				ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_TO]);
				ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_FROM]);
				ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_FROM]);
				ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_FROM_NAT]);
				ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_FROM_NAT]);
				ecm_front_end_connection_deref(feci);
				ecm_db_connection_deref(nci);
				ecm_db_multicast_tuple_instance_deref(tuple_instance);
				kfree(to_list);
				kfree(to_list_first);
				DEBUG_WARN("Failed to allocate classifiers assignments\n");
				goto done;
			}
		}

		ecm_db_front_end_instance_ref_and_set(nci, feci);

		/*
		 * Now add the connection into the database.
		 * NOTE: In an SMP situation such as ours there is a possibility that more than one packet for the same
		 * connection is being processed simultaneously.
		 * We *could* end up creating more than one connection instance for the same actual connection.
		 * To guard against this we now perform a mutex'd lookup of the connection + add once more - another cpu may have created it before us.
		 */
		spin_lock_bh(&ecm_ipv6_lock);
		ci = ecm_db_connection_find_and_ref(ip_src_addr, ip_dest_addr, IPPROTO_UDP, src_port, dest_port);
		if (ci) {
			/*
			 * Another cpu created the same connection before us - use the one we just found
			 */
			spin_unlock_bh(&ecm_ipv6_lock);
			ecm_db_multicast_tuple_instance_deref(tuple_instance);
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
					6, IPPROTO_UDP, ecm_dir,
					NULL /* final callback */,
					tg, is_routed, nci);

			/*
			 * Add the tuple instance and attach it with connection instance
			 */
			ecm_db_multicast_tuple_instance_add(tuple_instance, nci);
			spin_unlock_bh(&ecm_ipv6_lock);

			ecm_db_multicast_tuple_instance_deref(tuple_instance);
			ci = nci;
			/*
			 * Update the outdev_master or indev_master in ti
			 * Dereference: ecm_db_connection_deref()
			 */
			if (!is_routed) {
				ecm_db_multicast_tuple_instance_set_and_hold_l2_br_dev(tuple_instance, out_dev_master);
			} else if (l3_br_dev) {
				ecm_db_multicast_tuple_instance_set_and_hold_l3_br_dev(tuple_instance, l3_br_dev);
			}

			DEBUG_INFO("%px: New UDP connection created\n", ci);
		}

		/*
		 * No longer need referenecs to the objects we created
		 */
		dci->base.deref((struct ecm_classifier_instance *)dci);
		ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_TO]);
		ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_TO]);
		ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_FROM]);
		ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_FROM]);
		ecm_db_mapping_deref(mi[ECM_DB_OBJ_DIR_FROM_NAT]);
		ecm_db_node_deref(ni[ECM_DB_OBJ_DIR_FROM_NAT]);
		ecm_front_end_connection_deref(feci);
		kfree(to_list);
		kfree(to_list_first);

	} else {
		bool is_dest_interface_list_empty, routed;

		is_dest_interface_list_empty = ecm_db_multicast_connection_to_interfaces_set_check(ci);
		routed = ecm_db_connection_is_routed_get(ci);

		/*
		 * Check if there was an update to the 'routed' status for an existing flow.
		 * This can happen if the flow is a bridge+route flow, and the MFC rule was not added
		 * at the time the flow was originally created when the packet was processed by the
		 * bridge hook. In this case, we defunct the flow to re-create it again.
		 */
		if (routed != is_routed) {
			ecm_db_connection_make_defunct(ci);
			ecm_db_connection_deref(ci);
			goto done;
		}

		/*
		 * At this point the feci->accel_mode is DECEL because the
		 * MC connection has expired and we had received a callback from MFC which had freed the
		 * multicast destination interface heirarchy. In this case, we reconstruct the multicast
		 * destination interface heirarchy and re-accelerate the connection.
		 */
		if (!is_dest_interface_list_empty) {
			struct ecm_db_iface_instance *to_list;
			struct ecm_db_iface_instance *to_list_temp[ECM_DB_IFACE_HEIRARCHY_MAX];
			struct ecm_db_iface_instance *to_list_single;
			int32_t *to_list_first;
			int32_t *to_first;
			int32_t i, interface_idx_cnt;
			int ret;
			struct ecm_front_end_connection_instance *feci;

			to_list = (struct ecm_db_iface_instance *)kzalloc(ECM_DB_TO_MCAST_INTERFACES_SIZE, GFP_ATOMIC | __GFP_NOWARN);
			if (!to_list) {
				ecm_db_connection_deref(ci);
				goto done;
			}

			to_list_first = (int32_t *)kzalloc(sizeof(int32_t *) * ECM_DB_MULTICAST_IF_MAX, GFP_ATOMIC | __GFP_NOWARN);
			if (!to_list_first) {
				ecm_db_connection_deref(ci);
				kfree(to_list);
				goto done;
			}

			for (i = 0; i < ECM_DB_MULTICAST_IF_MAX; i++) {
				to_first = ecm_db_multicast_if_first_get_at_index(to_list_first, i);
				*to_first = ECM_DB_IFACE_HEIRARCHY_MAX;
			}

			feci = ecm_db_connection_front_end_get_and_ref(ci);
			interface_idx_cnt = ecm_multicast_ipv6_interface_heirarchy_construct(feci, to_list, in_dev, out_dev_master,\
					ip_src_addr, ip_dest_addr, mc_if_cnt,\
					mc_dest_if, to_list_first, src_node_addr,
					is_routed, skb);
			ecm_front_end_connection_deref(feci);
			if (interface_idx_cnt == 0) {
				DEBUG_WARN("Failed to reconstruct 'to mc' heirarchy list\n");
				ecm_db_connection_deref(ci);
				kfree(to_list);
				kfree(to_list_first);
				goto done;
			}

			ret = ecm_db_multicast_connection_to_interfaces_reset(ci, to_list, to_list_first);

			/*
			 * De-ref the destination interface list
			 */
			for (i = 0; i < ECM_DB_MULTICAST_IF_MAX; i++) {
				to_list_single = ecm_db_multicast_if_heirarchy_get(to_list, i);
				ecm_db_multicast_copy_if_heirarchy(to_list_temp, to_list_single);
				to_first = ecm_db_multicast_if_first_get_at_index(to_list_first, i);
				ecm_db_connection_interfaces_deref(to_list_temp, *to_first);
			}

			kfree(to_list);
			kfree(to_list_first);

			/*
			 * If ret is less than zero than connection reset could not find memory for
			 * to_mcast_interfaces. Deref the CI and retrun.
			 */
			if (ret < 0) {
				ecm_db_connection_deref(ci);
				goto done;
			}
		}
	}

	/*
	 * if a bridge dev is present in the MFC destination then set the
	 * ECM_DB_MULTICAST_CONNECTION_BRIDGE_DEV_SET_FLAG in tuple_instance
	 */
	if (br_dev_found_in_mfc) {
		struct ecm_db_multicast_tuple_instance *tuple_instance;
		tuple_instance = ecm_db_multicast_connection_find_and_ref(ip_src_addr, ip_dest_addr);
		if (!tuple_instance) {
			ecm_db_connection_deref(ci);
			goto done;
		}

		ecm_db_multicast_tuple_instance_flags_set(tuple_instance, ECM_DB_MULTICAST_CONNECTION_BRIDGE_DEV_SET_FLAG);
		ecm_db_multicast_connection_deref(tuple_instance);
	}

#if defined(CONFIG_NET_CLS_ACT) && defined(ECM_CLASSIFIER_DSCP_IGS) && defined(ECM_FRONT_END_NSS_ENABLE)
	/*
	 * Check if IGS feature is enabled or not.
	 */
	if (unlikely(ecm_interface_igs_enabled)) {
		struct ecm_front_end_connection_instance *feci = ecm_db_connection_front_end_get_and_ref(ci);
		if (feci->accel_engine == ECM_FRONT_END_ENGINE_NSS) {
			if (!ecm_nss_common_igs_acceleration_is_allowed(feci, skb)) {
				DEBUG_WARN("%px: Multicast IPv6 IGS acceleration denied.\n", ci);
				ecm_front_end_connection_deref(feci);
				ecm_db_connection_deref(ci);
				goto done;
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
		goto done;
	}

	/*
	 * Identify which side of the connection is sending
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
		ecm_multicast_ipv6_connection_regenerate(ci, sender, out_dev, in_dev, in_dev_nat);
	}

	/*
	 * Iterate the assignments and call to process!
	 * Policy implemented:
	 * 1. Classifiers that say they are not relevant are unassigned and not actioned further.
	 * 2. Any drop command from any classifier is honoured.
	 * 3. All classifiers must action acceleration for accel to be honoured, any classifiers not sure of their relevance will stop acceleration.
	 * 4. Only the highest priority classifier, that actions it, will have its qos tag honoured.
	 * 5. Only the highest priority classifier, that actions it, will have its timer group honoured.
	 */
	DEBUG_TRACE("%px: process begin, skb: %px\n", ci, skb);
	prevalent_pr.drop = false;
	prevalent_pr.flow_qos_tag = skb->priority;
	prevalent_pr.return_qos_tag = skb->priority;
	prevalent_pr.accel_mode = ECM_CLASSIFIER_ACCELERATION_MODE_ACCEL;
	prevalent_pr.timer_group = ci_orig_timer_group = ecm_db_connection_timer_group_get(ci);
	prevalent_pr.process_actions = 0;
	assignment_count = ecm_db_connection_classifier_assignments_get_and_ref(ci, assignments);
	for (aci_index = 0; aci_index < assignment_count; ++aci_index) {
		struct ecm_classifier_process_response aci_pr;
		struct ecm_classifier_instance *aci;

		aci = assignments[aci_index];
		DEBUG_TRACE("%px: process: %px, type: %d\n", ci, aci, aci->type_get(aci));
		aci->process(aci, sender, iph, skb, &aci_pr);
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

#ifdef ECM_CLASSIFIER_OVS_ENABLE
		if (aci_pr.process_actions & ECM_CLASSIFIER_PROCESS_ACTION_OVS_MCAST_DENY_ACCEL) {
			/*
			 * We do not accelerate if flow verification with OVS fails.
			 */
			DEBUG_TRACE("%px: wants deny acceleration: %px, type: %d, skb: %px\n", ci, aci, aci->type_get(aci), skb);
			prevalent_pr.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_OVS_MCAST_DENY_ACCEL;
		}
#endif

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

#ifdef ECM_CLASSIFIER_OVS_ENABLE
		if (aci_pr.process_actions & ECM_CLASSIFIER_PROCESS_ACTION_OVS_VLAN_TAG) {
			int i;

			prevalent_pr.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_OVS_VLAN_TAG;

			/*
			 * Set primary ingress VLAN tag
			 */
			prevalent_pr.ingress_vlan_tag[0] = aci_pr.ingress_vlan_tag[0];

			for (i = 0; i < ECM_DB_MULTICAST_IF_MAX; i++) {
				prevalent_pr.egress_mc_vlan_tag[i][0] = ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED;
				prevalent_pr.egress_mc_vlan_tag[i][1] = ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED;

				/*
				 * Set primary egress VLAN tag
				 */
				if (aci_pr.egress_mc_vlan_tag[i][0] != ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED) {
					prevalent_pr.egress_mc_vlan_tag[i][0] = aci_pr.egress_mc_vlan_tag[i][0];
				}
			}
		}

		if (aci_pr.process_actions & ECM_CLASSIFIER_PROCESS_ACTION_OVS_VLAN_QINQ_TAG) {
			int i;

			prevalent_pr.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_OVS_VLAN_QINQ_TAG;

			/*
			 * Set secondary ingress VLAN tag
			 */
			prevalent_pr.ingress_vlan_tag[1] = aci_pr.ingress_vlan_tag[1];

			for (i = 0; i < ECM_DB_MULTICAST_IF_MAX; i++) {
				/*
				 * Set secondary egress VLAN tag
				 */
				if (aci_pr.egress_mc_vlan_tag[i][1] != ECM_FRONT_END_VLAN_ID_NOT_CONFIGURED) {
					prevalent_pr.egress_mc_vlan_tag[i][1] = aci_pr.egress_mc_vlan_tag[i][1];
				}
			}
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
		goto done;
	}

#ifdef ECM_CLASSIFIER_OVS_ENABLE
	/*
	 * Defunct the connection if OVS classifer return deny acceleration.
	 */
	if (prevalent_pr.process_actions & ECM_CLASSIFIER_PROCESS_ACTION_OVS_MCAST_DENY_ACCEL) {
		ecm_db_connection_make_defunct(ci);
		ecm_db_connection_deref(ci);
		goto done;
	}
#endif
	ecm_db_connection_data_totals_update(ci, (sender == ECM_TRACKER_SENDER_TYPE_SRC)? true : false, skb->len, 1);

	/*
	 * Assign qos tag
	 * GGG TODO Should we use sender to identify whether to use flow or return qos tag?
	 */
	skb->priority = prevalent_pr.flow_qos_tag;
	DEBUG_TRACE("%px: skb priority: %u\n", ci, skb->priority);

	/*
	 * Accelerate?
	 */
	if (prevalent_pr.accel_mode == ECM_CLASSIFIER_ACCELERATION_MODE_ACCEL) {
		struct ecm_front_end_connection_instance *feci;
		DEBUG_TRACE("%px: accel\n", ci);
		feci = ecm_db_connection_front_end_get_and_ref(ci);
		feci->accelerate(feci, &prevalent_pr, false, NULL, skb);
		ecm_front_end_connection_deref(feci);
	}
	ecm_db_connection_deref(ci);

done:
	if (out_dev_master) {
		dev_put(out_dev_master);
	}
	return NF_ACCEPT;
}
