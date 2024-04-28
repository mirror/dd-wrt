/*
 **************************************************************************
 * Copyright (c) 2015-2018 The Linux Foundation.  All rights reserved.
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
#include <linux/ppp_defs.h>
#include <linux/mroute.h>

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
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(4, 2, 0))
#include <net/netfilter/nf_conntrack_zones.h>
#else
#include <linux/netfilter/nf_conntrack_zones_common.h>
#endif
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_timeout.h>
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
#define DEBUG_LEVEL ECM_SFE_IPV4_DEBUG_LEVEL

#include <sfe_drv.h>

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
#ifdef ECM_CLASSIFIER_NL_ENABLE
#include "ecm_classifier_nl.h"
#endif
#include "ecm_interface.h"
#include "ecm_sfe_ported_ipv4.h"
#ifdef ECM_MULTICAST_ENABLE
#include "ecm_sfe_multicast_ipv4.h"
#endif
#ifdef ECM_NON_PORTED_SUPPORT_ENABLE
#include "ecm_sfe_non_ported_ipv4.h"
#endif
#include "ecm_front_end_common.h"
#include "ecm_front_end_ipv4.h"

int ecm_sfe_ipv4_no_action_limit_default = 250;		/* Default no-action limit. */
int ecm_sfe_ipv4_driver_fail_limit_default = 250;		/* Default driver fail limit. */
int ecm_sfe_ipv4_nack_limit_default = 250;			/* Default nack limit. */
int ecm_sfe_ipv4_accelerated_count = 0;			/* Total offloads */
int ecm_sfe_ipv4_pending_accel_count = 0;			/* Total pending offloads issued to the SFE / awaiting completion */
int ecm_sfe_ipv4_pending_decel_count = 0;			/* Total pending deceleration requests issued to the SFE / awaiting completion */

/*
 * Limiting the acceleration of connections.
 *
 * By default there is no acceleration limiting.
 * This means that when ECM has more connections (that can be accelerated) than the acceleration
 * engine will allow the ECM will continue to try to accelerate.
 * In this scenario the acceleration engine will begin removal of existing rules to make way for new ones.
 * When the accel_limit_mode is set to FIXED ECM will not permit more rules to be issued than the engine will allow.
 */
uint32_t ecm_sfe_ipv4_accel_limit_mode = ECM_FRONT_END_ACCEL_LIMIT_MODE_UNLIMITED;

/*
 * Locking of the classifier - concurrency control for file global parameters.
 * NOTE: It is safe to take this lock WHILE HOLDING a feci->lock.  The reverse is NOT SAFE.
 */
DEFINE_SPINLOCK(ecm_sfe_ipv4_lock);			/* Protect against SMP access between netfilter, events and private threaded function. */

/*
 * Management thread control
 */
bool ecm_sfe_ipv4_terminate_pending = false;		/* True when the user has signalled we should quit */

/*
 * SFE driver linkage
 */
struct sfe_drv_ctx_instance *ecm_sfe_ipv4_drv_mgr = NULL;

static unsigned long ecm_sfe_ipv4_accel_cmd_time_avg_samples = 0;	/* Sum of time taken for the set of accel command samples, used to compute average time for an accel command to complete */
static unsigned long ecm_sfe_ipv4_accel_cmd_time_avg_set = 1;	/* How many samples in the set */
static unsigned long ecm_sfe_ipv4_decel_cmd_time_avg_samples = 0;	/* Sum of time taken for the set of accel command samples, used to compute average time for an accel command to complete */
static unsigned long ecm_sfe_ipv4_decel_cmd_time_avg_set = 1;	/* How many samples in the set */

#ifdef CONFIG_XFRM
static int ecm_sfe_ipv4_reject_acceleration_for_ipsec;		/* Don't accelerate IPSEC traffic */
#endif

/*
 * Debugfs dentry object.
 */
static struct dentry *ecm_sfe_ipv4_dentry;

/*
 * ecm_sfe_ipv4_node_establish_and_ref()
 *	Returns a reference to a node, possibly creating one if necessary.
 *
 * The given_node_addr will be used if provided.
 *
 * Returns NULL on failure.
 */
struct ecm_db_node_instance *ecm_sfe_ipv4_node_establish_and_ref(struct ecm_front_end_connection_instance *feci,
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

	DEBUG_INFO("Establish node for " ECM_IP_ADDR_DOT_FMT "\n", ECM_IP_ADDR_TO_DOT(addr));

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
#ifdef ECM_INTERFACE_PPPOE_ENABLE
		struct ecm_db_interface_info_pppoe pppoe_info;
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

		case ECM_DB_IFACE_TYPE_VLAN:
#ifdef ECM_INTERFACE_VLAN_ENABLE
			/*
			 * VLAN handled same along with ethernet, lag, bridge etc.
			 */
#else
			DEBUG_TRACE("VLAN interface unsupported\n");
			return NULL;
#endif
		case ECM_DB_IFACE_TYPE_ETHERNET:
		case ECM_DB_IFACE_TYPE_LAG:
		case ECM_DB_IFACE_TYPE_BRIDGE:
		case ECM_DB_IFACE_TYPE_IPSEC_TUNNEL:
			if (!ecm_interface_mac_addr_get_no_route(dev, addr, node_addr)) {
				ip_addr_t gw_addr = ECM_IP_ADDR_NULL;

				/*
				 * Try one more time with gateway ip address if it exists.
				 */
				if (!ecm_interface_find_gateway(addr, gw_addr)) {
					DEBUG_WARN("Node establish failed, there is no gateway address for 2nd mac lookup try\n");
					return NULL;
				}

				DEBUG_TRACE("Have a gw address " ECM_IP_ADDR_DOT_FMT "\n", ECM_IP_ADDR_TO_DOT(gw_addr));

				if (ecm_interface_mac_addr_get_no_route(dev, gw_addr, node_addr)) {
					DEBUG_TRACE("Found the mac address for gateway\n");
					goto done;
				}

				ecm_interface_send_arp_request(dev, addr, false, gw_addr);

				DEBUG_TRACE("failed to obtain any node address for host " ECM_IP_ADDR_DOT_FMT "\n", ECM_IP_ADDR_TO_DOT(addr));

				/*
				 * Unable to get node address at this time.
				 */
				return NULL;
			}
done:
			if (is_multicast_ether_addr(node_addr)) {
				DEBUG_TRACE("multicast node address for host " ECM_IP_ADDR_DOT_FMT ", node_addr: %pM\n", ECM_IP_ADDR_TO_DOT(addr), node_addr);
				return NULL;
			}

			/*
			 * Because we are iterating from inner to outer interface, this interface is the
			 * innermost one that has a node address - take this one.
			 */
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
		DEBUG_INFO("Failed to establish node for " ECM_IP_ADDR_DOT_FMT "\n", ECM_IP_ADDR_TO_DOT(addr));
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
		DEBUG_TRACE("%p: node established\n", ni);
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
	spin_lock_bh(&ecm_sfe_ipv4_lock);
	ni = ecm_db_node_find_and_ref(node_addr, ii);
	if (ni) {
		spin_unlock_bh(&ecm_sfe_ipv4_lock);
		ecm_db_node_deref(nni);
		ecm_db_iface_deref(ii);
		return ni;
	}

	ecm_db_node_add(nni, ii, node_addr, NULL, nni);
	spin_unlock_bh(&ecm_sfe_ipv4_lock);

	/*
	 * Don't need iface instance now
	 */
	ecm_db_iface_deref(ii);

	DEBUG_TRACE("%p: node established\n", nni);
	return nni;
}

/*
 * ecm_sfe_ipv4_host_establish_and_ref()
 *	Returns a reference to a host, possibly creating one if necessary.
 *
 * Returns NULL on failure.
 */
struct ecm_db_host_instance *ecm_sfe_ipv4_host_establish_and_ref(ip_addr_t addr)
{
	struct ecm_db_host_instance *hi;
	struct ecm_db_host_instance *nhi;

	DEBUG_INFO("Establish host for " ECM_IP_ADDR_DOT_FMT "\n", ECM_IP_ADDR_TO_DOT(addr));

	/*
	 * Locate the host
	 */
	hi = ecm_db_host_find_and_ref(addr);
	if (hi) {
		DEBUG_TRACE("%p: host established\n", hi);
		return hi;
	}

	/*
	 * No host - create one
	 */
	nhi = ecm_db_host_alloc();
	if (!nhi) {
		DEBUG_WARN("Failed to establish host\n");
		return NULL;
	}

	/*
	 * Add host into the database, atomically to avoid races creating the same thing
	 */
	spin_lock_bh(&ecm_sfe_ipv4_lock);
	hi = ecm_db_host_find_and_ref(addr);
	if (hi) {
		spin_unlock_bh(&ecm_sfe_ipv4_lock);
		ecm_db_host_deref(nhi);
		return hi;
	}

	ecm_db_host_add(nhi, addr, true, NULL, nhi);

	spin_unlock_bh(&ecm_sfe_ipv4_lock);

	DEBUG_TRACE("%p: host established\n", nhi);
	return nhi;
}

/*
 * ecm_sfe_ipv4_mapping_establish_and_ref()
 *	Returns a reference to a mapping, possibly creating one if necessary.
 *
 * Returns NULL on failure.
 */
struct ecm_db_mapping_instance *ecm_sfe_ipv4_mapping_establish_and_ref(ip_addr_t addr, int port)
{
	struct ecm_db_mapping_instance *mi;
	struct ecm_db_mapping_instance *nmi;
	struct ecm_db_host_instance *hi;

	DEBUG_INFO("Establish mapping for " ECM_IP_ADDR_DOT_FMT ":%d\n", ECM_IP_ADDR_TO_DOT(addr), port);

	/*
	 * Locate the mapping
	 */
	mi = ecm_db_mapping_find_and_ref(addr, port);
	if (mi) {
		DEBUG_TRACE("%p: mapping established\n", mi);
		return mi;
	}

	/*
	 * No mapping - establish host existence
	 */
	hi = ecm_sfe_ipv4_host_establish_and_ref(addr);
	if (!hi) {
		DEBUG_WARN("Failed to establish host\n");
		return NULL;
	}

	/*
	 * Create mapping instance
	 */
	nmi = ecm_db_mapping_alloc();
	if (!nmi) {
		ecm_db_host_deref(hi);
		DEBUG_WARN("Failed to establish mapping\n");
		return NULL;
	}

	/*
	 * Add mapping into the database, atomically to avoid races creating the same thing
	 */
	spin_lock_bh(&ecm_sfe_ipv4_lock);
	mi = ecm_db_mapping_find_and_ref(addr, port);
	if (mi) {
		spin_unlock_bh(&ecm_sfe_ipv4_lock);
		ecm_db_mapping_deref(nmi);
		ecm_db_host_deref(hi);
		return mi;
	}

	ecm_db_mapping_add(nmi, hi, port, NULL, nmi);

	spin_unlock_bh(&ecm_sfe_ipv4_lock);

	/*
	 * Don't need the host instance now - the mapping maintains a reference to it now.
	 */
	ecm_db_host_deref(hi);

	/*
	 * Return the mapping instance
	 */
	DEBUG_INFO("%p: mapping established\n", nmi);
	return nmi;
}

/*
 * ecm_sfe_ipv4_accel_done_time_update()
 *	Record how long the command took to complete, updating average samples
 */
void ecm_sfe_ipv4_accel_done_time_update(struct ecm_front_end_connection_instance *feci)
{
	unsigned long delta;

	/*
	 * How long did it take the command to complete?
	 */
	spin_lock_bh(&feci->lock);
	feci->stats.cmd_time_completed = jiffies;
	delta = feci->stats.cmd_time_completed - feci->stats.cmd_time_begun;
	spin_unlock_bh(&feci->lock);

	spin_lock_bh(&ecm_sfe_ipv4_lock);
	ecm_sfe_ipv4_accel_cmd_time_avg_samples += delta;
	ecm_sfe_ipv4_accel_cmd_time_avg_set++;
	spin_unlock_bh(&ecm_sfe_ipv4_lock);
}

/*
 * ecm_sfe_ipv4_deccel_done_time_update()
 *	Record how long the command took to complete, updating average samples
 */
void ecm_sfe_ipv4_decel_done_time_update(struct ecm_front_end_connection_instance *feci)
{
	unsigned long delta;

	/*
	 * How long did it take the command to complete?
	 */
	spin_lock_bh(&feci->lock);
	feci->stats.cmd_time_completed = jiffies;
	delta = feci->stats.cmd_time_completed - feci->stats.cmd_time_begun;
	spin_unlock_bh(&feci->lock);

	spin_lock_bh(&ecm_sfe_ipv4_lock);
	ecm_sfe_ipv4_decel_cmd_time_avg_samples += delta;
	ecm_sfe_ipv4_decel_cmd_time_avg_set++;
	spin_unlock_bh(&ecm_sfe_ipv4_lock);
}

/*
 * ecm_sfe_ipv4_connection_regenerate()
 *	Re-generate a connection.
 *
 * Re-generating a connection involves re-evaluating the interface lists in case interface heirarchies have changed.
 * It also involves the possible triggering of classifier re-evaluation but only if all currently assigned
 * classifiers permit this operation.
 */
void ecm_sfe_ipv4_connection_regenerate(struct ecm_db_connection_instance *ci, ecm_tracker_sender_type_t sender,
							struct net_device *out_dev, struct net_device *out_dev_nat,
							struct net_device *in_dev, struct net_device *in_dev_nat,
							__be16 *layer4hdr, struct sk_buff *skb)
{
	int i;
	bool reclassify_allowed;
	int32_t to_list_first;
	struct ecm_db_iface_instance *to_list[ECM_DB_IFACE_HEIRARCHY_MAX];
	int32_t to_nat_list_first;
	struct ecm_db_iface_instance *to_nat_list[ECM_DB_IFACE_HEIRARCHY_MAX];
	int32_t from_list_first;
	struct ecm_db_iface_instance *from_list[ECM_DB_IFACE_HEIRARCHY_MAX];
	int32_t from_nat_list_first;
	struct ecm_db_iface_instance *from_nat_list[ECM_DB_IFACE_HEIRARCHY_MAX];
	ip_addr_t ip_src_addr;
	ip_addr_t ip_dest_addr;
	ip_addr_t ip_src_addr_nat;
	ip_addr_t ip_dest_addr_nat;
	int protocol;
	bool is_routed;
	uint8_t src_node_addr[ETH_ALEN];
	uint8_t dest_node_addr[ETH_ALEN];
	uint8_t src_node_addr_nat[ETH_ALEN];
	uint8_t dest_node_addr_nat[ETH_ALEN];
	int assignment_count;
	struct ecm_classifier_instance *assignments[ECM_CLASSIFIER_TYPES];
	struct ecm_front_end_connection_instance *feci;
	struct ecm_front_end_interface_construct_instance efeici;
	 ecm_db_direction_t ecm_dir;

	DEBUG_INFO("%p: re-gen needed\n", ci);

	/*
	 * We may need to swap the devices around depending on who the sender of the packet that triggered the re-gen is
	 */
	if (sender == ECM_TRACKER_SENDER_TYPE_DEST) {
		struct net_device *tmp_dev;

		/*
		 * This is a packet sent by the destination of the connection, i.e. it is a packet issued by the 'from' side of the connection.
		 */
		DEBUG_TRACE("%p: Re-gen swap devs\n", ci);
		tmp_dev = out_dev;
		out_dev = in_dev;
		in_dev = tmp_dev;

		tmp_dev = out_dev_nat;
		out_dev_nat = in_dev_nat;
		in_dev_nat = tmp_dev;
	}

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
	ecm_dir = ecm_db_connection_direction_get(ci);

	ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_FROM, ip_src_addr);
	ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_FROM_NAT, ip_src_addr_nat);

	ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_TO, ip_dest_addr);
	ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_TO_NAT, ip_dest_addr_nat);

	ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_FROM, src_node_addr);
	ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_FROM_NAT, src_node_addr_nat);

	ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_TO, dest_node_addr);
	ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_TO_NAT, dest_node_addr_nat);

	feci = ecm_db_connection_front_end_get_and_ref(ci);

	if (!ecm_front_end_ipv4_interface_construct_set_and_hold(skb, sender, ecm_dir, is_routed,
							in_dev, out_dev,
							ip_src_addr, ip_src_addr_nat,
							ip_dest_addr, ip_dest_addr_nat,
							&efeici)) {

		DEBUG_WARN("ECM front end ipv4 interface construct set failed for regeneration\n");
		goto ecm_ipv4_retry_regen;
	}

	DEBUG_TRACE("%p: Update the 'from' interface heirarchy list\n", ci);
	from_list_first = ecm_interface_heirarchy_construct(feci, from_list, efeici.from_dev, efeici.from_other_dev, ip_dest_addr, efeici.from_mac_lookup_ip_addr, ip_src_addr, 4, protocol, in_dev, is_routed, in_dev, src_node_addr, dest_node_addr, layer4hdr, skb);
	if (from_list_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
		ecm_front_end_ipv4_interface_construct_netdev_put(&efeici);
		goto ecm_ipv4_retry_regen;
	}

	ecm_db_connection_interfaces_reset(ci, from_list, from_list_first, ECM_DB_OBJ_DIR_FROM);
	ecm_db_connection_interfaces_deref(from_list, from_list_first);

	DEBUG_TRACE("%p: Update the 'from NAT' interface heirarchy list\n", ci);
	from_nat_list_first = ecm_interface_heirarchy_construct(feci, from_nat_list, efeici.from_nat_dev, efeici.from_nat_other_dev, ip_dest_addr, efeici.from_nat_mac_lookup_ip_addr, ip_src_addr_nat, 4, protocol, in_dev_nat, is_routed, in_dev_nat, src_node_addr_nat, dest_node_addr_nat, layer4hdr, skb);
	if (from_nat_list_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
		ecm_front_end_ipv4_interface_construct_netdev_put(&efeici);
		goto ecm_ipv4_retry_regen;
	}

	ecm_db_connection_interfaces_reset(ci, from_nat_list, from_nat_list_first, ECM_DB_OBJ_DIR_FROM_NAT);
	ecm_db_connection_interfaces_deref(from_nat_list, from_nat_list_first);

	DEBUG_TRACE("%p: Update the 'to' interface heirarchy list\n", ci);
	to_list_first = ecm_interface_heirarchy_construct(feci, to_list, efeici.to_dev, efeici.to_other_dev, ip_src_addr, efeici.to_mac_lookup_ip_addr, ip_dest_addr, 4, protocol, out_dev, is_routed, in_dev, dest_node_addr, src_node_addr, layer4hdr, skb);
	if (to_list_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
		ecm_front_end_ipv4_interface_construct_netdev_put(&efeici);
		goto ecm_ipv4_retry_regen;
	}

	ecm_db_connection_interfaces_reset(ci, to_list, to_list_first, ECM_DB_OBJ_DIR_TO);
	ecm_db_connection_interfaces_deref(to_list, to_list_first);

	DEBUG_TRACE("%p: Update the 'to NAT' interface heirarchy list\n", ci);
	to_nat_list_first = ecm_interface_heirarchy_construct(feci, to_nat_list, efeici.to_nat_dev, efeici.to_nat_other_dev, ip_src_addr, efeici.to_nat_mac_lookup_ip_addr, ip_dest_addr_nat, 4, protocol, out_dev_nat, is_routed, in_dev, dest_node_addr_nat, src_node_addr_nat, layer4hdr, skb);
	if (to_nat_list_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
		ecm_front_end_ipv4_interface_construct_netdev_put(&efeici);
		goto ecm_ipv4_retry_regen;
	}

	ecm_front_end_ipv4_interface_construct_netdev_put(&efeici);

	feci->deref(feci);
	ecm_db_connection_interfaces_reset(ci, to_nat_list, to_nat_list_first, ECM_DB_OBJ_DIR_TO_NAT);
	ecm_db_connection_interfaces_deref(to_nat_list, to_nat_list_first);

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
		DEBUG_TRACE("%p: Calling to reclassify: %p, type: %d\n", ci, assignments[i], assignments[i]->type_get(assignments[i]));
		if (!assignments[i]->reclassify_allowed(assignments[i])) {
			DEBUG_TRACE("%p: reclassify denied: %p, by type: %d\n", ci, assignments[i], assignments[i]->type_get(assignments[i]));
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
		DEBUG_WARN("%p: re-classify denied\n", ci);
		goto ecm_ipv4_regen_done;
	}

	/*
	 * Reclassify
	 */
	DEBUG_INFO("%p: reclassify\n", ci);
	if (!ecm_classifier_reclassify(ci, assignment_count, assignments)) {
		/*
		 * We could not set up the classifiers to reclassify, it is safer to fail out and try again next time
		 */
		DEBUG_WARN("%p: Regeneration: reclassify failed\n", ci);
		goto ecm_ipv4_regen_done;
	}
	DEBUG_INFO("%p: reclassify success\n", ci);

ecm_ipv4_regen_done:

	/*
	 * Release the assignments
	 */
	ecm_db_connection_assignments_release(assignment_count, assignments);

	/*
	 * Re-generation of state is successful.
	 */
	ecm_db_connection_regeneration_completed(ci);
	return;

ecm_ipv4_retry_regen:
	feci->deref(feci);
	ecm_db_connection_regeneration_failed(ci);
	return;
}

/*
 * ecm_sfe_ipv4_ip_process()
 *	Process IP datagram skb
 */
static unsigned int ecm_sfe_ipv4_ip_process(struct net_device *out_dev, struct net_device *in_dev,
							uint8_t *src_node_addr, uint8_t *dest_node_addr,
							bool can_accel, bool is_routed, bool is_l2_encap, struct sk_buff *skb)
{
	struct ecm_tracker_ip_header ip_hdr;
        struct nf_conn *ct;
        enum ip_conntrack_info ctinfo;
	struct nf_conntrack_tuple orig_tuple;
	struct nf_conntrack_tuple reply_tuple;
	ecm_db_direction_t ecm_dir;
	ecm_tracker_sender_type_t sender;
	ip_addr_t ip_src_addr;
	ip_addr_t ip_dest_addr;
	ip_addr_t ip_src_addr_nat;
	ip_addr_t ip_dest_addr_nat;
	struct net_device *out_dev_nat;
	struct net_device *in_dev_nat;
	uint8_t *src_node_addr_nat;
	uint8_t *dest_node_addr_nat;

	/*
	 * Obtain the IP header from the skb
	 */
	if (!ecm_tracker_ip_check_header_and_read(&ip_hdr, skb)) {
		DEBUG_WARN("Invalid ip header in skb %p\n", skb);
		return NF_ACCEPT;
	}

	if (ip_hdr.fragmented) {
		DEBUG_TRACE("skb %p is fragmented\n", skb);
		return NF_ACCEPT;
	}

#ifdef CONFIG_XFRM
	/*
	 * If skb_dst(skb)->xfrm is not null, packet is to be encrypted by ipsec, we can't accelerate it.
	 * If skb->sp is not null, packet is decrypted by ipsec. We only accelerate it when configuration didn't reject ipsec.
	 */
	if (unlikely((skb_dst(skb) && skb_dst(skb)->xfrm) || \
		     (ecm_sfe_ipv4_reject_acceleration_for_ipsec && skb_ext_exist(skb, SKB_EXT_SEC_PATH)))) {
		DEBUG_TRACE("skip local ipsec flows\n");
		return NF_ACCEPT;
	}
#endif

	/*
	 * Extract information, if we have conntrack then use that info as far as we can.
	 */
        ct = nf_ct_get(skb, &ctinfo);
	if (unlikely(!ct)) {
		DEBUG_TRACE("%p: no ct\n", skb);
		ECM_IP_ADDR_TO_NIN4_ADDR(orig_tuple.src.u3.ip, ip_hdr.src_addr);
		ECM_IP_ADDR_TO_NIN4_ADDR(orig_tuple.dst.u3.ip, ip_hdr.dest_addr);
		orig_tuple.dst.protonum = ip_hdr.protocol;
		reply_tuple.src.u3.ip = orig_tuple.dst.u3.ip;
		reply_tuple.dst.u3.ip = orig_tuple.src.u3.ip;
		sender = ECM_TRACKER_SENDER_TYPE_SRC;
	} else {
		if (unlikely(ctinfo == IP_CT_UNTRACKED)) {
			DEBUG_TRACE("%p: ct: untracked\n", skb);
			return NF_ACCEPT;
		}

		/*
		 * If the conntrack connection is using a helper (i.e. Application Layer Gateway)
		 * then acceleration is denied (connection needs assistance from HLOS to function)
		 */
		if (nfct_help(ct)) {
			DEBUG_TRACE("%p: Connection has helper\n", ct);
			can_accel = false;
		}

		/*
		 * Extract conntrack connection information
		 */
		DEBUG_TRACE("%p: ct: %p, ctinfo: %x\n", skb, ct, ctinfo);
		orig_tuple = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple;
		reply_tuple = ct->tuplehash[IP_CT_DIR_REPLY].tuple;
		if (IP_CT_DIR_ORIGINAL == CTINFO2DIR(ctinfo)) {
			sender = ECM_TRACKER_SENDER_TYPE_SRC;
		} else {
			sender = ECM_TRACKER_SENDER_TYPE_DEST;
		}

		/*
		 * Is this a related connection?
		 */
		if ((ctinfo == IP_CT_RELATED) || (ctinfo == IP_CT_RELATED_REPLY)) {
			/*
			 * ct is related to the packet at hand.
			 * We can use the IP src/dest information and the direction information.
			 * We cannot use the protocol information from the ct (typically the packet at hand is ICMP error that is related to the ct we have here).
			 */
			orig_tuple.dst.protonum = ip_hdr.protocol;
			DEBUG_TRACE("%p: related ct, actual protocol: %u\n", skb, orig_tuple.dst.protonum);
		}
	}

#ifdef ECM_MULTICAST_ENABLE
	/*
	 * Check for a multicast Destination address here.
	 */
	ECM_NIN4_ADDR_TO_IP_ADDR(ip_dest_addr, orig_tuple.dst.u3.ip);
	if (ecm_ip_addr_is_multicast(ip_dest_addr)) {
		DEBUG_TRACE("Multicast, Processing: %p\n", skb);
		return ecm_sfe_multicast_ipv4_connection_process(out_dev,
				in_dev,
				src_node_addr,
				dest_node_addr,
				can_accel, is_routed, skb,
				&ip_hdr,
				ct, sender,
				&orig_tuple, &reply_tuple);
	}
#endif

	/*
	 * Work out if this packet involves NAT or not.
	 * If it does involve NAT then work out if this is an ingressing or egressing packet.
	 */
	if (orig_tuple.src.u3.ip != reply_tuple.dst.u3.ip) {
		/*
		 * Egressing NAT
		 */
		ecm_dir = ECM_DB_DIRECTION_EGRESS_NAT;
	} else if (orig_tuple.dst.u3.ip != reply_tuple.src.u3.ip) {
		/*
		 * Ingressing NAT
		 */
		ecm_dir = ECM_DB_DIRECTION_INGRESS_NAT;
	} else if (is_routed) {
		/*
		 * Non-NAT
		 */
		ecm_dir = ECM_DB_DIRECTION_NON_NAT;
	} else {
		/*
		 * Bridged
		 */
		ecm_dir = ECM_DB_DIRECTION_BRIDGED;
	}

	DEBUG_TRACE("IP Packet ORIGINAL src: %pI4 ORIGINAL dst: %pI4 protocol: %u, sender: %d ecm_dir: %d\n", &orig_tuple.src.u3.ip, &orig_tuple.dst.u3.ip, orig_tuple.dst.protonum, sender, ecm_dir);

	/*
	 * Get IP addressing information.  This same logic is applied when extracting port information too.
	 * This is tricky to do as what we are after is src and destination addressing that is non-nat but we also need the nat information too.
	 * INGRESS connections have their conntrack information reversed!
	 * We have to keep in mind the connection direction AND the packet direction in order to be able to work out what is what.
	 *
	 * ip_src_addr and ip_dest_addr MUST always be the NON-NAT endpoint addresses and reflect PACKET direction and not connection direction 'dir'.
	 *
	 * Examples 1 through 4 cater for NAT and NON-NAT in the INGRESS or EGRESS cases.
	 *
	 * Example 1:
	 * An 'original' direction packet to an egress connection from br-lan:192.168.0.133:12345 to eth0:80.132.221.34:80 via NAT'ing router mapping eth0:10.10.10.30:33333 looks like:
	 *	orig_tuple->src == 192.168.0.133:12345		This becomes ip_src_addr
	 *	orig_tuple->dst == 80.132.221.34:80		This becomes ip_dest_addr
	 *	reply_tuple->src == 80.132.221.34:80		This becomes ip_dest_addr_nat
	 *	reply_tuple->dest == 10.10.10.30:33333		This becomes ip_src_addr_nat
	 *
	 *	in_dev would be br-lan - i.e. the device of ip_src_addr
	 *	out_dev would be eth0 - i.e. the device of ip_dest_addr
	 *	in_dev_nat would be eth0 - i.e. out_dev, the device of ip_src_addr_nat
	 *	out_dev_nat would be eth0 - i.e. out_dev, the device of ip_dest_addr_nat
	 *
	 *	From a Node address perspective we are at position X in the following topology:
	 *	LAN_PC======BR-LAN___ETH0====X====WAN_PC
	 *
	 *	src_node_addr refers to node address of of ip_src_addr_nat
	 *	src_node_addr_nat is set to src_node_addr
	 *	src_node_addr is then set to NULL as there is no node address available here for ip_src_addr
	 *
	 *	dest_node_addr refers to node address of ip_dest_addr
	 *	dest_node_addr_nat is the node of ip_dest_addr_nat which is the same as dest_node_addr
	 *
	 * Example 2:
	 * However an 'original' direction packet to an ingress connection from eth0:80.132.221.34:3321 to a LAN host (e.g. via DMZ) br-lan@192.168.0.133:12345 via NAT'ing router mapping eth0:10.10.10.30:12345 looks like:
	 *	orig_tuple->src == 80.132.221.34:3321		This becomes ip_src_addr
	 *	orig_tuple->dst == 10.10.10.30:12345		This becomes ip_dest_addr_nat
	 *	reply_tuple->src == 192.168.0.133:12345		This becomes ip_dest_addr
	 *	reply_tuple->dest == 80.132.221.34:3321		This becomes ip_src_addr_nat
	 *
	 *	in_dev would be eth0 - i.e. the device of ip_src_addr
	 *	out_dev would be br-lan - i.e. the device of ip_dest_addr
	 *	in_dev_nat would be eth0 - i.e. in_dev, the device of ip_src_addr_nat
	 *	out_dev_nat would be eth0 - i.e. in_dev, the device of ip_dest_addr_nat
	 *
	 *	From a Node address perspective we are at position X in the following topology:
	 *	LAN_PC===X===BR-LAN___ETH0========WAN_PC
	 *
	 *	src_node_addr refers to node address of br-lan which is not useful
	 *	src_node_addr_nat AND src_node_addr become NULL
	 *
	 *	dest_node_addr refers to node address of ip_dest_addr
	 *	dest_node_addr_nat is set to NULL
	 *
	 * When dealing with reply packets this confuses things even more.  Reply packets to the above two examples are as follows:
	 *
	 * Example 3:
	 * A 'reply' direction packet to the egress connection above:
	 *	orig_tuple->src == 192.168.0.133:12345		This becomes ip_dest_addr
	 *	orig_tuple->dst == 80.132.221.34:80		This becomes ip_src_addr
	 *	reply_tuple->src == 80.132.221.34:80		This becomes ip_src_addr_nat
	 *	reply_tuple->dest == 10.10.10.30:33333		This becomes ip_dest_addr_nat
	 *
	 *	in_dev would be eth0 - i.e. the device of ip_src_addr
	 *	out_dev would be br-lan - i.e. the device of ip_dest_addr
	 *	in_dev_nat would be eth0 - i.e. in_dev, the device of ip_src_addr_nat
	 *	out_dev_nat would be eth0 - i.e. in_dev, the device of ip_dest_addr_nat
	 *
	 *	From a Node address perspective we are at position X in the following topology:
	 *	LAN_PC===X===BR-LAN___ETH0========WAN_PC
	 *
	 *	src_node_addr refers to node address of br-lan which is not useful
	 *	src_node_addr_nat AND src_node_addr become NULL
	 *
	 *	dest_node_addr refers to node address of ip_dest_addr
	 *	dest_node_addr_nat is set to NULL
	 *
	 * Example 4:
	 * A 'reply' direction packet to the ingress connection above:
	 *	orig_tuple->src == 80.132.221.34:3321		This becomes ip_dest_addr
	 *	orig_tuple->dst == 10.10.10.30:12345		This becomes ip_src_addr_nat
	 *	reply_tuple->src == 192.168.0.133:12345		This becomes ip_src_addr
	 *	reply_tuple->dest == 80.132.221.34:3321		This becomes ip_dest_addr_nat
	 *
	 *	in_dev would be br-lan - i.e. the device of ip_src_addr
	 *	out_dev would be eth0 - i.e. the device of ip_dest_addr
	 *	in_dev_nat would be eth0 - i.e. out_dev, the device of ip_src_addr_nat
	 *	out_dev_nat would be eth0 - i.e. out_dev, the device of ip_dest_addr_nat
	 *
	 *	From a Node address perspective we are at position X in the following topology:
	 *	LAN_PC======BR-LAN___ETH0====X====WAN_PC
	 *
	 *	src_node_addr refers to node address of ip_src_addr_nat
	 *	src_node_addr_nat is set to src_node_addr
	 *	src_node_addr becomes NULL
	 *
	 *	dest_node_addr refers to node address of ip_dest_addr
	 *	dest_node_addr_nat is set to dest_node_addr also.
	 *
	 * The following examples are for BRIDGED cases:
	 *
	 * Example 5:
	 * An 'original' direction packet to an bridged connection from eth1:192.168.0.133:12345 to eth2:192.168.0.244:80 looks like:
	 *	orig_tuple->src == 192.168.0.133:12345		This becomes ip_src_addr
	 *	orig_tuple->dst == 192.168.0.244:80		This becomes ip_dest_addr
	 *	reply_tuple->src == 192.168.0.244:80		This becomes ip_dest_addr_nat
	 *	reply_tuple->dest == 192.168.0.133:12345	This becomes ip_src_addr_nat
	 *
	 *	in_dev would be eth1 - i.e. the device of ip_src_addr
	 *	out_dev would be eth2 - i.e. the device of ip_dest_addr
	 *	in_dev_nat would be eth1 - i.e. in_dev, the device of ip_src_addr_nat
	 *	out_dev_nat would be eth2 - i.e. out_dev, the device of ip_dest_addr_nat
	 *
	 *	From a Node address perspective we are at position X in the following topology:
	 *	LAN PC======ETH1___ETH2====X====LAN PC
	 *
	 *	src_node_addr refers to node address of ip_src_addr
	 *	src_node_addr_nat is set to src_node_addr
	 *
	 *	dest_node_addr refers to node address of ip_dest_addr
	 *	dest_node_addr_nat is set to dest_node_addr
	 *
	 * Example 6:
	 * An 'reply' direction packet to the bridged connection above:
	 *	orig_tuple->src == 192.168.0.133:12345		This becomes ip_dest_addr
	 *	orig_tuple->dst == 192.168.0.244:80		This becomes ip_src_addr
	 *	reply_tuple->src == 192.168.0.244:80		This becomes ip_src_addr_nat
	 *	reply_tuple->dest == 192.168.0.133:12345	This becomes ip_dest_addr_nat
	 *
	 *	in_dev would be eth2 - i.e. the device of ip_src_addr
	 *	out_dev would be eth1 - i.e. the device of ip_dest_addr
	 *	in_dev_nat would be eth2 - i.e. in_dev, the device of ip_src_addr_nat
	 *	out_dev_nat would be eth1 - i.e. out_dev, the device of ip_dest_addr_nat
	 *
	 *	From a Node address perspective we are at position X in the following topology:
	 *	LAN PC===X===ETH1___ETH2========LAN PC
	 *
	 *	src_node_addr refers to node address of ip_src_addr
	 *	src_node_addr_nat is set to src_node_addr
	 *
	 *	dest_node_addr refers to node address of ip_dest_addr
	 *	dest_node_addr_nat is set to dest_node_addr
	 */
	if (sender == ECM_TRACKER_SENDER_TYPE_SRC) {
		if ((ecm_dir == ECM_DB_DIRECTION_EGRESS_NAT) || (ecm_dir == ECM_DB_DIRECTION_NON_NAT)) {
			/*
			 * Example 1
			 */
			ECM_NIN4_ADDR_TO_IP_ADDR(ip_src_addr, orig_tuple.src.u3.ip);
			ECM_NIN4_ADDR_TO_IP_ADDR(ip_dest_addr, orig_tuple.dst.u3.ip);
			ECM_NIN4_ADDR_TO_IP_ADDR(ip_dest_addr_nat, reply_tuple.src.u3.ip);
			ECM_NIN4_ADDR_TO_IP_ADDR(ip_src_addr_nat, reply_tuple.dst.u3.ip);

			in_dev_nat = out_dev;
			out_dev_nat = out_dev;

			src_node_addr_nat = src_node_addr;
			src_node_addr = NULL;

			dest_node_addr_nat = dest_node_addr;
		} else if (ecm_dir == ECM_DB_DIRECTION_INGRESS_NAT) {
			/*
			 * Example 2
			 */
			ECM_NIN4_ADDR_TO_IP_ADDR(ip_src_addr, orig_tuple.src.u3.ip);
			ECM_NIN4_ADDR_TO_IP_ADDR(ip_dest_addr_nat, orig_tuple.dst.u3.ip);
			ECM_NIN4_ADDR_TO_IP_ADDR(ip_dest_addr, reply_tuple.src.u3.ip);
			ECM_NIN4_ADDR_TO_IP_ADDR(ip_src_addr_nat, reply_tuple.dst.u3.ip);

			in_dev_nat = in_dev;
			out_dev_nat = in_dev;

			src_node_addr = NULL;
			src_node_addr_nat = NULL;

			dest_node_addr_nat = NULL;
		} else if (ecm_dir == ECM_DB_DIRECTION_BRIDGED) {
			/*
			 * Example 5
			 */
			ECM_NIN4_ADDR_TO_IP_ADDR(ip_src_addr, orig_tuple.src.u3.ip);
			ECM_NIN4_ADDR_TO_IP_ADDR(ip_dest_addr, orig_tuple.dst.u3.ip);
			ECM_NIN4_ADDR_TO_IP_ADDR(ip_dest_addr_nat, reply_tuple.src.u3.ip);
			ECM_NIN4_ADDR_TO_IP_ADDR(ip_src_addr_nat, reply_tuple.dst.u3.ip);

			in_dev_nat = in_dev;
			out_dev_nat = out_dev;

			src_node_addr_nat = src_node_addr;

			dest_node_addr_nat = dest_node_addr;
		} else {
			DEBUG_ASSERT(false, "Unhandled ecm_dir: %d\n", ecm_dir);
		}
	} else {
		if ((ecm_dir == ECM_DB_DIRECTION_EGRESS_NAT) || (ecm_dir == ECM_DB_DIRECTION_NON_NAT)) {
			/*
			 * Example 3
			 */
			ECM_NIN4_ADDR_TO_IP_ADDR(ip_dest_addr, orig_tuple.src.u3.ip);
			ECM_NIN4_ADDR_TO_IP_ADDR(ip_src_addr, orig_tuple.dst.u3.ip);
			ECM_NIN4_ADDR_TO_IP_ADDR(ip_src_addr_nat, reply_tuple.src.u3.ip);
			ECM_NIN4_ADDR_TO_IP_ADDR(ip_dest_addr_nat, reply_tuple.dst.u3.ip);

			in_dev_nat  = in_dev;
			out_dev_nat = in_dev;

			src_node_addr = NULL;
			src_node_addr_nat = NULL;

			dest_node_addr_nat = NULL;
		} else if (ecm_dir == ECM_DB_DIRECTION_INGRESS_NAT) {
			/*
			 * Example 4
			 */
			ECM_NIN4_ADDR_TO_IP_ADDR(ip_dest_addr, orig_tuple.src.u3.ip);
			ECM_NIN4_ADDR_TO_IP_ADDR(ip_src_addr_nat, orig_tuple.dst.u3.ip);
			ECM_NIN4_ADDR_TO_IP_ADDR(ip_src_addr, reply_tuple.src.u3.ip);
			ECM_NIN4_ADDR_TO_IP_ADDR(ip_dest_addr_nat, reply_tuple.dst.u3.ip);

			in_dev_nat = out_dev;
			out_dev_nat = out_dev;

			src_node_addr_nat = src_node_addr;
			src_node_addr = NULL;

			dest_node_addr_nat = dest_node_addr;
		} else if (ecm_dir == ECM_DB_DIRECTION_BRIDGED) {
			/*
			 * Example 6
			 */
			ECM_NIN4_ADDR_TO_IP_ADDR(ip_dest_addr, orig_tuple.src.u3.ip);
			ECM_NIN4_ADDR_TO_IP_ADDR(ip_src_addr, orig_tuple.dst.u3.ip);
			ECM_NIN4_ADDR_TO_IP_ADDR(ip_src_addr_nat, reply_tuple.src.u3.ip);
			ECM_NIN4_ADDR_TO_IP_ADDR(ip_dest_addr_nat, reply_tuple.dst.u3.ip);

			in_dev_nat  = in_dev;
			out_dev_nat = out_dev;

			src_node_addr_nat = src_node_addr;

			dest_node_addr_nat = dest_node_addr;
		} else {
			DEBUG_ASSERT(false, "Unhandled ecm_dir: %d\n", ecm_dir);
		}
	}

	/*
	 * Non-unicast source or destination packets are ignored
	 * NOTE: Only need to check the non-nat src/dest addresses here.
	 */
	if (unlikely(ecm_ip_addr_is_non_unicast(ip_dest_addr))) {
		DEBUG_TRACE("skb %p non-unicast daddr " ECM_IP_ADDR_DOT_FMT "\n", skb, ECM_IP_ADDR_TO_DOT(ip_dest_addr));
		return NF_ACCEPT;
	}
	if (unlikely(ecm_ip_addr_is_non_unicast(ip_src_addr))) {
		DEBUG_TRACE("skb %p non-unicast saddr " ECM_IP_ADDR_DOT_FMT "\n", skb, ECM_IP_ADDR_TO_DOT(ip_src_addr));
		return NF_ACCEPT;
	}

	/*
	 * Process IP specific protocol
	 * TCP and UDP are the most likliest protocols.
	 */
	if (likely(orig_tuple.dst.protonum == IPPROTO_TCP) || likely(orig_tuple.dst.protonum == IPPROTO_UDP)) {
		return ecm_sfe_ported_ipv4_process(out_dev, out_dev_nat,
				in_dev, in_dev_nat,
				src_node_addr, src_node_addr_nat,
				dest_node_addr, dest_node_addr_nat,
				can_accel, is_routed, is_l2_encap, skb,
				&ip_hdr,
				ct, sender, ecm_dir,
				&orig_tuple, &reply_tuple,
				ip_src_addr, ip_dest_addr, ip_src_addr_nat, ip_dest_addr_nat);
	}
#ifdef ECM_NON_PORTED_SUPPORT_ENABLE
	return ecm_sfe_non_ported_ipv4_process(out_dev, out_dev_nat,
				in_dev, in_dev_nat,
				src_node_addr, src_node_addr_nat,
				dest_node_addr, dest_node_addr_nat,
				can_accel, is_routed, is_l2_encap, skb,
				&ip_hdr,
				ct, sender, ecm_dir,
				&orig_tuple, &reply_tuple,
				ip_src_addr, ip_dest_addr, ip_src_addr_nat, ip_dest_addr_nat);
#else
	return NF_ACCEPT;
#endif
}

/*
 * ecm_sfe_ipv4_post_routing_hook()
 *	Called for IP packets that are going out to interfaces after IP routing stage.
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
static unsigned int ecm_sfe_ipv4_post_routing_hook(void *priv,
				struct sk_buff *skb,
				const struct nf_hook_state *nhs)
{
	struct net_device *out = nhs->out;
#elif (LINUX_VERSION_CODE <= KERNEL_VERSION(3, 6, 0))
static unsigned int ecm_sfe_ipv4_post_routing_hook(unsigned int hooknum,
				struct sk_buff *skb,
				const struct net_device *in_unused,
				const struct net_device *out,
				int (*okfn)(struct sk_buff *))
{
#else
static unsigned int ecm_sfe_ipv4_post_routing_hook(const struct nf_hook_ops *ops,
				struct sk_buff *skb,
				const struct net_device *in_unused,
				const struct net_device *out,
				int (*okfn)(struct sk_buff *))
{
#endif
	struct net_device *in;
	bool can_accel = true;
	unsigned int result;

	DEBUG_TRACE("%p: Routing: %s\n", out, out->name);

	if (ecm_front_end_acceleration_rejected(skb)) {
		DEBUG_TRACE("Acceleration rejected\n");
		return NF_ACCEPT;
	}

	/*
	 * If operations have stopped then do not process packets
	 */
	spin_lock_bh(&ecm_sfe_ipv4_lock);
	if (unlikely(ecm_front_end_ipv4_stopped)) {
		spin_unlock_bh(&ecm_sfe_ipv4_lock);
		DEBUG_TRACE("Front end stopped\n");
		return NF_ACCEPT;
	}
	spin_unlock_bh(&ecm_sfe_ipv4_lock);

	/*
	 * Don't process broadcast or multicast
	 */
	if (skb->pkt_type == PACKET_BROADCAST) {
		DEBUG_TRACE("Broadcast, ignoring: %p\n", skb);
		return NF_ACCEPT;
	}

#ifndef ECM_MULTICAST_ENABLE
	if (skb->pkt_type == PACKET_MULTICAST) {
		DEBUG_TRACE("Multicast, ignoring: %p\n", skb);
		return NF_ACCEPT;
	}
#endif

#ifdef ECM_INTERFACE_PPPOE_ENABLE
	/*
	 * skip l2tp/pptp because we don't accelerate them
	 */
	if (ecm_interface_is_l2tp_pptp(skb, out)) {
		return NF_ACCEPT;
	}
#endif

	/*
	 * Identify interface from where this packet came
	 */
	in = dev_get_by_index(&init_net, skb->skb_iif);
	if (unlikely(!in)) {
		/*
		 * Locally sourced packets are not processed in ECM.
		 */
		return NF_ACCEPT;
	}

	DEBUG_TRACE("Post routing process skb %p, out: %p (%s), in: %p (%s)\n", skb, out, out->name, in, in->name);
	result = ecm_sfe_ipv4_ip_process((struct net_device *)out, in, NULL, NULL,
							can_accel, true, false, skb);
	dev_put(in);
	return result;
}

/*
 * ecm_sfe_ipv4_stats_sync_callback()
 *	Callback handler from the sfe driver.
 */
static void ecm_sfe_ipv4_stats_sync_callback(void *app_data, struct sfe_ipv4_msg *nim)
{
	struct sfe_ipv4_conn_sync *sync;
	struct nf_conntrack_tuple_hash *h;
	struct nf_conntrack_tuple tuple;
	struct nf_conn *ct;
	struct nf_conn_counter *acct;
	struct ecm_db_connection_instance *ci;
	struct ecm_front_end_connection_instance *feci;
	struct neighbour *neigh;
	ip_addr_t flow_ip;
	ip_addr_t return_ip_xlate;
	ip_addr_t return_ip;
	struct ecm_classifier_instance *assignments[ECM_CLASSIFIER_TYPES];
	int aci_index;
	int assignment_count;
	struct ecm_classifier_rule_sync class_sync;
	int flow_dir;
	int return_dir;

	/*
	 * Only respond to sync messages
	 */
	if (nim->cm.type != SFE_RX_CONN_STATS_SYNC_MSG) {
		DEBUG_TRACE("Ignoring nim: %p - not sync: %d", nim, nim->cm.type);
		return;
	}
	sync = &nim->msg.conn_stats;

	/*
	 * Look up ecm connection with a view to synchronising the connection, classifier and data tracker.
	 * Note that we use _xlate versions for destination - for egressing connections this would be the wan IP address,
	 * but for ingressing this would be the LAN side (non-nat'ed) address and is what we need for lookup of our connection.
	 */
	DEBUG_INFO("%p: SFE Sync, lookup connection using\n"
			"Protocol: %d\n" \
			"src_addr: %pI4n:%d\n" \
			"dest_addr: %pI4n:%d\n",
			sync,
			(int)sync->protocol,
			&sync->flow_ip, (int)sync->flow_ident,
			&sync->return_ip_xlate, (int)sync->return_ident_xlate);

	ECM_NIN4_ADDR_TO_IP_ADDR(flow_ip, sync->flow_ip);
	ECM_NIN4_ADDR_TO_IP_ADDR(return_ip_xlate, sync->return_ip_xlate);
	ECM_NIN4_ADDR_TO_IP_ADDR(return_ip, sync->return_ip);

#ifdef ECM_MULTICAST_ENABLE
	/*
	 * Check for multicast flow
	 */
	if (ecm_ip_addr_is_multicast(return_ip)) {
		ci = ecm_db_connection_find_and_ref(flow_ip, return_ip, sync->protocol, (int)ntohs(sync->flow_ident), (int)ntohs(sync->return_ident));
	} else {
		ci = ecm_db_connection_find_and_ref(flow_ip, return_ip_xlate, sync->protocol, (int)ntohs(sync->flow_ident), (int)ntohs(sync->return_ident_xlate));
	}
#else
	ci = ecm_db_connection_find_and_ref(flow_ip, return_ip_xlate, sync->protocol, (int)ntohs(sync->flow_ident), (int)ntohs(sync->return_ident_xlate));
#endif
	if (!ci) {
		DEBUG_TRACE("%p: SFE Sync: no connection\n", sync);
		goto sync_conntrack;
	}
	DEBUG_TRACE("%p: Sync conn %p\n", sync, ci);

	/*
	 * Keep connection alive and updated
	 */
	if (!ecm_db_connection_defunct_timer_touch(ci)) {
		ecm_db_connection_deref(ci);
		goto sync_conntrack;
	}

	/*
	 * Get the front end instance
	 */
	feci = ecm_db_connection_front_end_get_and_ref(ci);

	if (sync->flow_tx_packet_count || sync->return_tx_packet_count) {
		DEBUG_TRACE("%p: flow_rx_packet_count: %u, flow_rx_byte_count: %u, return_rx_packet_count: %u, return_rx_byte_count: %u\n",
				ci, sync->flow_rx_packet_count, sync->flow_rx_byte_count, sync->return_rx_packet_count, sync->return_rx_byte_count);
		DEBUG_TRACE("%p: flow_tx_packet_count: %u, flow_tx_byte_count: %u, return_tx_packet_count: %u, return_tx_byte_count: %u\n",
				ci, sync->flow_tx_packet_count, sync->flow_tx_byte_count, sync->return_tx_packet_count, sync->return_tx_byte_count);
#ifdef ECM_MULTICAST_ENABLE
		if (ecm_ip_addr_is_multicast(return_ip)) {
			/*
			 * The amount of data *sent* by the ECM multicast connection 'from' side is the amount the SFE has *received* in the 'flow' direction.
			 */
			ecm_db_multicast_connection_data_totals_update(ci, true, sync->flow_rx_byte_count, sync->flow_rx_packet_count);
			ecm_db_multicast_connection_data_totals_update(ci, false, sync->return_rx_byte_count, sync->return_rx_packet_count);
			ecm_db_multicast_connection_interface_heirarchy_stats_update(ci, sync->flow_rx_byte_count, sync->flow_rx_packet_count);

			/*
			 * As packets have been accelerated we have seen some action.
			 */
			feci->action_seen(feci);

			/*
			 * Update IP multicast routing cache stats
			 */
			ipmr_mfc_stats_update(&init_net, htonl(flow_ip[0]), htonl(return_ip[0]), sync->flow_rx_packet_count,
										 sync->flow_rx_byte_count, sync->flow_rx_packet_count, sync->flow_rx_byte_count);
		} else {
			/*
			 * The amount of data *sent* by the ECM connection 'from' side is the amount the SFE has *received* in the 'flow' direction.
			 */
			ecm_db_connection_data_totals_update(ci, true, sync->flow_rx_byte_count, sync->flow_rx_packet_count);

			/*
			 * The amount of data *sent* by the ECM connection 'to' side is the amount the SFE has *received* in the 'return' direction.
			 */
			ecm_db_connection_data_totals_update(ci, false, sync->return_rx_byte_count, sync->return_rx_packet_count);

			/*
			 * As packets have been accelerated we have seen some action.
			 */
			feci->action_seen(feci);

		}

#else
		/*
		 * The amount of data *sent* by the ECM connection 'from' side is the amount the SFE has *received* in the 'flow' direction.
		 */
		ecm_db_connection_data_totals_update(ci, true, sync->flow_rx_byte_count, sync->flow_rx_packet_count);

		/*
		 * The amount of data *sent* by the ECM connection 'to' side is the amount the SFE has *received* in the 'return' direction.
		 */
		ecm_db_connection_data_totals_update(ci, false, sync->return_rx_byte_count, sync->return_rx_packet_count);

		/*
		 * As packets have been accelerated we have seen some action.
		 */
		feci->action_seen(feci);

#endif
	}

	/*
	 * Copy the sync data to the classifier sync structure to
	 * update the classifiers' stats.
	 */
	class_sync.flow_tx_packet_count = sync->flow_tx_packet_count;
	class_sync.return_tx_packet_count = sync->return_tx_packet_count;
	class_sync.reason = sync->reason;

	/*
	 * Sync assigned classifiers
	 */
	assignment_count = ecm_db_connection_classifier_assignments_get_and_ref(ci, assignments);
	for (aci_index = 0; aci_index < assignment_count; ++aci_index) {
		struct ecm_classifier_instance *aci;
		aci = assignments[aci_index];
		DEBUG_TRACE("%p: sync to: %p, type: %d\n", feci, aci, aci->type_get(aci));
		aci->sync_to_v4(aci, &class_sync);
	}
	ecm_db_connection_assignments_release(assignment_count, assignments);

	switch(sync->reason) {
	case SFE_RULE_SYNC_REASON_DESTROY:
		/*
		 * This is the final sync from the SFE for a connection whose acceleration was
		 * terminated by the ecm.
		 * NOTE: We take no action here since that is performed by the destroy message ack.
		 */
		DEBUG_INFO("%p: ECM initiated final sync seen: %d\n", ci, sync->reason);

		/*
		 * If there is no tx/rx packets to update the other linux subsystems, we shouldn't continue
		 * for the sync message which comes as a final sync for the ECM initiated destroy request.
		 * Because this means the connection is not active for sometime and adding this delta time
		 * to the conntrack timeout will update it eventhough there is no traffic for this connection.
		 */
		if (!sync->flow_tx_packet_count && !sync->return_tx_packet_count) {
			feci->deref(feci);
			ecm_db_connection_deref(ci);
			return;
		}
		break;
	case SFE_RULE_SYNC_REASON_FLUSH:
	case SFE_RULE_SYNC_REASON_EVICT:
		/*
		 * SFE has ended acceleration without instruction from the ECM.
		 */
		DEBUG_INFO("%p: SFE Initiated final sync seen: %d cause:%d\n", ci, sync->reason, sync->cause);

		/*
		 * SFE Decelerated the connection
		 */
		feci->accel_ceased(feci);
		break;
	default:
		if (ecm_db_connection_is_routed_get(ci)) {
			/*
			 * Update the neighbour entry for source IP address
			 */
			neigh = ecm_interface_ipv4_neigh_get(flow_ip);
			if (!neigh) {
				DEBUG_WARN("Neighbour entry for %pI4n not found\n", &sync->flow_ip);
			} else {
				if (sync->flow_tx_packet_count) {
					DEBUG_TRACE("Neighbour entry event send for %pI4n: %p\n", &sync->flow_ip, neigh);
					neigh_event_send(neigh, NULL);
				}

				neigh_release(neigh);
			}

#ifdef ECM_MULTICAST_ENABLE
			/*
			 * Update the neighbour entry for destination IP address
			 */
			if (!ecm_ip_addr_is_multicast(return_ip)) {
				neigh = ecm_interface_ipv4_neigh_get(return_ip);
				if (!neigh) {
					DEBUG_WARN("Neighbour entry for %pI4n not found\n", &sync->return_ip);
				} else {
					if (sync->return_tx_packet_count) {
						DEBUG_TRACE("Neighbour entry event send for %pI4n: %p\n", &sync->return_ip, neigh);
						neigh_event_send(neigh, NULL);
					}

					neigh_release(neigh);
				}
			}
#else
			/*
			 * Update the neighbour entry for destination IP address
			 */
			neigh = ecm_interface_ipv4_neigh_get(return_ip);
			if (!neigh) {
				DEBUG_WARN("Neighbour entry for %pI4n not found\n", &sync->return_ip);
			} else {
				if (sync->return_tx_packet_count) {
					DEBUG_TRACE("Neighbour entry event send for %pI4n: %p\n", &sync->return_ip, neigh);
					neigh_event_send(neigh, NULL);
				}

				neigh_release(neigh);
			}
#endif
		}
	}

	/*
	 * If connection should be re-generated then we need to force a deceleration
	 */
	if (unlikely(ecm_db_connection_regeneration_required_peek(ci))) {
		DEBUG_TRACE("%p: Connection generation changing, terminating acceleration", ci);
		feci->decelerate(feci);
	}

	feci->deref(feci);
	ecm_db_connection_deref(ci);

sync_conntrack:
	;

	/*
	 * Create a tuple so as to be able to look up a conntrack connection
	 */
	memset(&tuple, 0, sizeof(tuple));
	tuple.src.u3.ip = sync->flow_ip;
	tuple.src.u.all = sync->flow_ident;
	tuple.src.l3num = AF_INET;

	tuple.dst.u3.ip = sync->return_ip;
	tuple.dst.dir = IP_CT_DIR_ORIGINAL;
	tuple.dst.protonum = (uint8_t)sync->protocol;
	tuple.dst.u.all = sync->return_ident;

	DEBUG_TRACE("Conntrack sync, lookup conntrack connection using\n"
			"Protocol: %d\n"
			"src_addr: %pI4:%d\n"
			"dest_addr: %pI4:%d\n",
			(int)tuple.dst.protonum,
			&tuple.src.u3.ip, (int)(ntohs(tuple.src.u.all)),
			&tuple.dst.u3.ip, (int)(ntohs(tuple.dst.u.all)));

	/*
	 * Look up conntrack connection
	 */
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(4, 2, 0))
	h = nf_conntrack_find_get(&init_net, NF_CT_DEFAULT_ZONE, &tuple);
#else
	h = nf_conntrack_find_get(&init_net, &nf_ct_zone_dflt, &tuple);
#endif
	if (!h) {
		DEBUG_WARN("%p: SFE Sync: no conntrack connection\n", sync);
		return;
	}

	ct = nf_ct_tuplehash_to_ctrack(h);
	DEBUG_TRACE("%p: SFE Sync: conntrack connection\n", ct);

	ecm_front_end_flow_and_return_directions_get(ct, flow_ip, 4, &flow_dir, &return_dir);

	/*
	 * Only update if this is not a fixed timeout
	 */
	if (!test_bit(IPS_FIXED_TIMEOUT_BIT, &ct->status)) {
		unsigned long int delta_jiffies;

		/*
		 * Convert ms ticks from the SFE to jiffies. We know that inc_ticks is small
		 * and we expect HZ to be small too so we can multiply without worrying about
		 * wrap-around problems.  We add a rounding constant to ensure that the different
		 * time bases don't cause truncation errors.
		 */
		delta_jiffies = ((sync->inc_ticks * HZ) + (MSEC_PER_SEC / 2)) / MSEC_PER_SEC;

		spin_lock_bh(&ct->lock);
		ct->timeout += delta_jiffies;
		spin_unlock_bh(&ct->lock);
	}

#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3,6,0))
	acct = nf_conn_acct_find(ct);
#else
	acct = nf_conn_acct_find(ct)->counter;
#endif
	if (acct) {
		spin_lock_bh(&ct->lock);
		atomic64_add(sync->flow_rx_packet_count, &acct[flow_dir].packets);
		atomic64_add(sync->flow_rx_byte_count, &acct[flow_dir].bytes);

		atomic64_add(sync->return_rx_packet_count, &acct[return_dir].packets);
		atomic64_add(sync->return_rx_byte_count, &acct[return_dir].bytes);
		spin_unlock_bh(&ct->lock);
	}

	switch (sync->protocol) {
	case IPPROTO_TCP:
		spin_lock_bh(&ct->lock);
		if (ct->proto.tcp.seen[flow_dir].td_maxwin < sync->flow_max_window) {
			ct->proto.tcp.seen[flow_dir].td_maxwin = sync->flow_max_window;
		}
		if ((int32_t)(ct->proto.tcp.seen[flow_dir].td_end - sync->flow_end) < 0) {
			ct->proto.tcp.seen[flow_dir].td_end = sync->flow_end;
		}
		if ((int32_t)(ct->proto.tcp.seen[flow_dir].td_maxend - sync->flow_max_end) < 0) {
			ct->proto.tcp.seen[flow_dir].td_maxend = sync->flow_max_end;
		}
		if (ct->proto.tcp.seen[return_dir].td_maxwin < sync->return_max_window) {
			ct->proto.tcp.seen[return_dir].td_maxwin = sync->return_max_window;
		}
		if ((int32_t)(ct->proto.tcp.seen[return_dir].td_end - sync->return_end) < 0) {
			ct->proto.tcp.seen[return_dir].td_end = sync->return_end;
		}
		if ((int32_t)(ct->proto.tcp.seen[return_dir].td_maxend - sync->return_max_end) < 0) {
			ct->proto.tcp.seen[return_dir].td_maxend = sync->return_max_end;
		}
		spin_unlock_bh(&ct->lock);
		break;
	case IPPROTO_UDP:
		/*
		 * In Linux connection track, UDP flow has two timeout values:
		 * /proc/sys/net/netfilter/nf_conntrack_udp_timeout:
		 * 	this is for uni-direction UDP flow, normally its value is 60 seconds
		 * /proc/sys/net/netfilter/nf_conntrack_udp_timeout_stream:
		 * 	this is for bi-direction UDP flow, normally its value is 180 seconds
		 *
		 * Linux will update timer of UDP flow to stream timeout once it seen packets
		 * in reply direction. But if flow is accelerated by NSS or SFE, Linux won't
		 * see any packets. So we have to do the same thing in our stats sync message.
		 */
		if (!test_bit(IPS_ASSURED_BIT, &ct->status) && acct) {
			u_int64_t reply_pkts = atomic64_read(&acct[IP_CT_DIR_REPLY].packets);

			if (reply_pkts != 0) {
				unsigned int *timeouts;

				set_bit(IPS_SEEN_REPLY_BIT, &ct->status);
				set_bit(IPS_ASSURED_BIT, &ct->status);

				timeouts = nf_ct_timeout_lookup(ct);

				/* Copy of udp_get_timeouts in kernel */
				if (!timeouts)
					timeouts = nf_udp_pernet(nf_ct_net(ct))->timeouts;

				spin_lock_bh(&ct->lock);
				ct->timeout = jiffies + timeouts[UDP_CT_REPLIED];
				spin_unlock_bh(&ct->lock);
			}
		}
		break;
	}

	/*
	 * Release connection
	 */
	nf_ct_put(ct);
}

/*
 * struct nf_hook_ops ecm_sfe_ipv4_netfilter_hooks[]
 *	Hooks into netfilter packet monitoring points.
 */
static struct nf_hook_ops ecm_sfe_ipv4_netfilter_hooks[] __read_mostly = {
	/*
	 * Post routing hook is used to monitor packets going to interfaces that are NOT bridged in some way, e.g. packets to the WAN.
	 */
	{
		.hook           = ecm_sfe_ipv4_post_routing_hook,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 4, 0))
		.owner          = THIS_MODULE,
#endif
		.pf             = PF_INET,
		.hooknum        = NF_INET_POST_ROUTING,
		.priority       = NF_IP_PRI_NAT_SRC + 1,
	},
};

/*
 * ecm_sfe_ipv4_get_accel_limit_mode()
 */
static int ecm_sfe_ipv4_get_accel_limit_mode(void *data, u64 *val)
{
	*val = ecm_sfe_ipv4_accel_limit_mode;

	return 0;
}

/*
 * ecm_sfe_ipv4_set_accel_limit_mode()
 */
static int ecm_sfe_ipv4_set_accel_limit_mode(void *data, u64 val)
{
	DEBUG_TRACE("ecm_sfe_ipv4_accel_limit_mode = %x\n", (int)val);

	/*
	 * Check that only valid bits are set.
	 * It's fine for no bits to be set as that suggests no modes are wanted.
	 */
	if (val && (val ^ (ECM_FRONT_END_ACCEL_LIMIT_MODE_FIXED | ECM_FRONT_END_ACCEL_LIMIT_MODE_UNLIMITED))) {
		DEBUG_WARN("ecm_sfe_ipv4_accel_limit_mode = %x bad\n", (int)val);
		return -EINVAL;
	}

	ecm_sfe_ipv4_accel_limit_mode = (int)val;

	return 0;
}

/*
 * Debugfs attribute for accel limit mode.
 */
DEFINE_SIMPLE_ATTRIBUTE(ecm_sfe_ipv4_accel_limit_mode_fops, ecm_sfe_ipv4_get_accel_limit_mode, ecm_sfe_ipv4_set_accel_limit_mode, "%llu\n");

/*
 * ecm_sfe_ipv4_get_accel_cmd_avg_millis()
 */
static ssize_t ecm_sfe_ipv4_get_accel_cmd_avg_millis(struct file *file,
								char __user *user_buf,
								size_t sz, loff_t *ppos)
{
	unsigned long set;
	unsigned long samples;
	unsigned long avg;
	char *buf;
	int ret;

	buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (!buf) {
		return -ENOMEM;
	}

	/*
	 * Operate under our locks.
	 * Compute the average of the samples taken and seed the next set of samples with the result of this one.
	 */
	spin_lock_bh(&ecm_sfe_ipv4_lock);
	samples = ecm_sfe_ipv4_accel_cmd_time_avg_samples;
	set = ecm_sfe_ipv4_accel_cmd_time_avg_set;
	ecm_sfe_ipv4_accel_cmd_time_avg_samples /= ecm_sfe_ipv4_accel_cmd_time_avg_set;
	ecm_sfe_ipv4_accel_cmd_time_avg_set = 1;
	avg = ecm_sfe_ipv4_accel_cmd_time_avg_samples;
	spin_unlock_bh(&ecm_sfe_ipv4_lock);

	/*
	 * Convert average jiffies to milliseconds
	 */
	avg *= 1000;
	avg /= HZ;

	ret = snprintf(buf, (ssize_t)PAGE_SIZE, "avg=%lu\tsamples=%lu\tset_size=%lu\n", avg, samples, set);
	if (ret < 0) {
		kfree(buf);
		return -EFAULT;
	}

	ret = simple_read_from_buffer(user_buf, sz, ppos, buf, ret);
	kfree(buf);
	return ret;
}

/*
 * File operations for accel command average time.
 */
static struct file_operations ecm_sfe_ipv4_accel_cmd_avg_millis_fops = {
	.read = ecm_sfe_ipv4_get_accel_cmd_avg_millis,
};

/*
 * ecm_sfe_ipv4_get_decel_average_millis()
 */
static ssize_t ecm_sfe_ipv4_get_decel_cmd_avg_millis(struct file *file,
								char __user *user_buf,
								size_t sz, loff_t *ppos)
{
	unsigned long set;
	unsigned long samples;
	unsigned long avg;
	char *buf;
	int ret;

	buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (!buf) {
		return -ENOMEM;
	}

	/*
	 * Operate under our locks.
	 * Compute the average of the samples taken and seed the next set of samples with the result of this one.
	 */
	spin_lock_bh(&ecm_sfe_ipv4_lock);
	samples = ecm_sfe_ipv4_decel_cmd_time_avg_samples;
	set = ecm_sfe_ipv4_decel_cmd_time_avg_set;
	ecm_sfe_ipv4_decel_cmd_time_avg_samples /= ecm_sfe_ipv4_decel_cmd_time_avg_set;
	ecm_sfe_ipv4_decel_cmd_time_avg_set = 1;
	avg = ecm_sfe_ipv4_decel_cmd_time_avg_samples;
	spin_unlock_bh(&ecm_sfe_ipv4_lock);

	/*
	 * Convert average jiffies to milliseconds
	 */
	avg *= 1000;
	avg /= HZ;

	ret = snprintf(buf, (ssize_t)PAGE_SIZE, "avg=%lu\tsamples=%lu\tset_size=%lu\n", avg, samples, set);
	if (ret < 0) {
		kfree(buf);
		return -EFAULT;
	}

	ret = simple_read_from_buffer(user_buf, sz, ppos, buf, ret);
	kfree(buf);
	return ret;
}

/*
 * File operations for decel command average time.
 */
static struct file_operations ecm_sfe_ipv4_decel_cmd_avg_millis_fops = {
	.read = ecm_sfe_ipv4_get_decel_cmd_avg_millis,
};

/*
 * ecm_sfe_ipv4_init()
 */
int ecm_sfe_ipv4_init(struct dentry *dentry)
{
	int result = -1;

	DEBUG_INFO("ECM SFE IPv4 init\n");

	ecm_sfe_ipv4_dentry = debugfs_create_dir("ecm_sfe_ipv4", dentry);
	if (!ecm_sfe_ipv4_dentry) {
		DEBUG_ERROR("Failed to create ecm sfe ipv4 directory in debugfs\n");
		return result;
	}

#ifdef CONFIG_XFRM
	debugfs_create_u32("reject_acceleration_for_ipsec", S_IRUGO | S_IWUSR, ecm_sfe_ipv4_dentry,
					(u32 *)&ecm_sfe_ipv4_reject_acceleration_for_ipsec) ;
#endif

	debugfs_create_u32("no_action_limit_default", S_IRUGO | S_IWUSR, ecm_sfe_ipv4_dentry,
					(u32 *)&ecm_sfe_ipv4_no_action_limit_default);

	debugfs_create_u32("driver_fail_limit_default", S_IRUGO | S_IWUSR, ecm_sfe_ipv4_dentry,
					(u32 *)&ecm_sfe_ipv4_driver_fail_limit_default);

	debugfs_create_u32("nack_limit_default", S_IRUGO | S_IWUSR, ecm_sfe_ipv4_dentry,
					(u32 *)&ecm_sfe_ipv4_nack_limit_default);

	debugfs_create_u32("accelerated_count", S_IRUGO, ecm_sfe_ipv4_dentry,
					(u32 *)&ecm_sfe_ipv4_accelerated_count);

	debugfs_create_u32("pending_accel_count", S_IRUGO, ecm_sfe_ipv4_dentry,
					(u32 *)&ecm_sfe_ipv4_pending_accel_count);

	debugfs_create_u32("pending_decel_count", S_IRUGO, ecm_sfe_ipv4_dentry,
					(u32 *)&ecm_sfe_ipv4_pending_decel_count);

	if(!debugfs_create_file("accel_limit_mode", S_IRUGO | S_IWUSR, ecm_sfe_ipv4_dentry,
					NULL, &ecm_sfe_ipv4_accel_limit_mode_fops)) {
		DEBUG_ERROR("Failed to create ecm sfe ipv4 accel_limit_mode file in debugfs\n");
		goto task_cleanup;
	}

	if (!debugfs_create_file("accel_cmd_avg_millis", S_IRUGO, ecm_sfe_ipv4_dentry,
					NULL, &ecm_sfe_ipv4_accel_cmd_avg_millis_fops)) {
		DEBUG_ERROR("Failed to create ecm sfe ipv4 accel_cmd_avg_millis file in debugfs\n");
		goto task_cleanup;
	}

	if (!debugfs_create_file("decel_cmd_avg_millis", S_IRUGO, ecm_sfe_ipv4_dentry,
					NULL, &ecm_sfe_ipv4_decel_cmd_avg_millis_fops)) {
		DEBUG_ERROR("Failed to create ecm sfe ipv4 decel_cmd_avg_millis file in debugfs\n");
		goto task_cleanup;
	}

	if (!ecm_sfe_ported_ipv4_debugfs_init(ecm_sfe_ipv4_dentry)) {
		DEBUG_ERROR("Failed to create ecm ported files in debugfs\n");
		goto task_cleanup;
	}

#ifdef ECM_NON_PORTED_SUPPORT_ENABLE
	if (!ecm_sfe_non_ported_ipv4_debugfs_init(ecm_sfe_ipv4_dentry)) {
		DEBUG_ERROR("Failed to create ecm non-ported files in debugfs\n");
		goto task_cleanup;
	}
#endif

#ifdef ECM_MULTICAST_ENABLE
	if (!ecm_sfe_multicast_ipv4_debugfs_init(ecm_sfe_ipv4_dentry)) {
		DEBUG_ERROR("Failed to create ecm multicast files in debugfs\n");
		goto task_cleanup;
	}
#endif
	/*
	 * Register this module with the simulated sfe driver.
	 * Notify manager should be registered before the netfilter hooks. Because there
	 * is a possibility that the ECM can try to send acceleration messages to the
	 * acceleration engine without having an acceleration engine manager.
	 */
	ecm_sfe_ipv4_drv_mgr = sfe_drv_ipv4_notify_register(ecm_sfe_ipv4_stats_sync_callback, NULL);

	/*
	 * Register netfilter hooks
	 */
	result = nf_register_net_hooks(&init_net, ecm_sfe_ipv4_netfilter_hooks, ARRAY_SIZE(ecm_sfe_ipv4_netfilter_hooks));
	if (result < 0) {
		DEBUG_ERROR("Can't register netfilter hooks.\n");
		sfe_drv_ipv4_notify_unregister();
		goto task_cleanup;
	}

#ifdef ECM_MULTICAST_ENABLE
	ecm_sfe_multicast_ipv4_init();
#endif
	return 0;

task_cleanup:

	debugfs_remove_recursive(ecm_sfe_ipv4_dentry);
	return result;
}
EXPORT_SYMBOL(ecm_sfe_ipv4_init);

/*
 * ecm_sfe_ipv4_exit()
 */
void ecm_sfe_ipv4_exit(void)
{
	DEBUG_INFO("ECM SFE IPv4 Module exit\n");

	spin_lock_bh(&ecm_sfe_ipv4_lock);
	ecm_sfe_ipv4_terminate_pending = true;
	spin_unlock_bh(&ecm_sfe_ipv4_lock);

	/*
	 * Stop the network stack hooks
	 */
	nf_unregister_net_hooks(&init_net, ecm_sfe_ipv4_netfilter_hooks,
				ARRAY_SIZE(ecm_sfe_ipv4_netfilter_hooks));

	/*
	 * Unregister from the simulated sfe driver
	 */
	sfe_drv_ipv4_notify_unregister();

	/*
	 * Remove the debugfs files recursively.
	 */
	if (ecm_sfe_ipv4_dentry) {
		debugfs_remove_recursive(ecm_sfe_ipv4_dentry);
	}

#ifdef ECM_MULTICAST_ENABLE
	ecm_sfe_multicast_ipv4_exit();
#endif
}
EXPORT_SYMBOL(ecm_sfe_ipv4_exit);
