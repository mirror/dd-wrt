/*
 **************************************************************************
 * Copyright (c) 2014-2021 The Linux Foundation.  All rights reserved.
 * Copyright (c) 2022-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include <net/ip_tunnels.h>
#include <net/route.h>
#include <net/ip.h>
#include <net/tcp.h>
#include <net/addrconf.h>
#include <net/xfrm.h>
#include <asm/unaligned.h>
#include <asm/uaccess.h>	/* for put_user */
#include <net/ipv6.h>
#include <linux/inet.h>
#include <linux/in.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <linux/ppp_defs.h>
#include <linux/mroute.h>

#include <net/ip6_tunnel.h>
#include <linux/inetdevice.h>
#include <linux/if_arp.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_bridge.h>
#include <linux/if_bridge.h>
#include <net/arp.h>
#ifdef ECM_INTERFACE_VXLAN_ENABLE
#include <net/vxlan.h>
#endif
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_acct.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_l4proto.h>
#include <linux/netfilter/nf_conntrack_zones_common.h>
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
#define DEBUG_LEVEL ECM_CMN_IPV4_DEBUG_LEVEL

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
#include "ecm_front_end_common.h"
#include "ecm_front_end_ipv4.h"
#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
#include <ovsmgr.h>
#endif
#include "ecm_ported_ipv4.h"
#ifdef ECM_NON_PORTED_SUPPORT_ENABLE
#include "ecm_non_ported_ipv4.h"
#endif
#include "ecm_multicast_ipv4.h"
#include "ecm_stats_v4.h"

/*
 * Locking of the classifier - concurrency control for file global parameters.
 * NOTE: It is safe to take this lock WHILE HOLDING a feci->lock.  The reverse is NOT SAFE.
 */
DEFINE_SPINLOCK(ecm_ipv4_lock);			/* Protect against SMP access between netfilter, events and private threaded function. */

/*
 * Management thread control
 */
bool ecm_ipv4_terminate_pending = false;		/* True when the user has signalled we should quit */

/*
 * ecm_ipv4_dev_has_ipaddr()
 *	Returns true if dev has an IPv4 address
 */
bool ecm_ipv4_dev_has_ipaddr(struct net_device *dev)
{
	struct in_device *in_dev;
	const struct in_ifaddr *ifa;

	rcu_read_lock();
	in_dev = __in_dev_get_rcu(dev);
	if (!in_dev) {
		rcu_read_unlock();
		DEBUG_TRACE("%s: dev->ip_ptr is NULL\n", dev->name);
		return false;
	}

	ifa = rcu_dereference(in_dev->ifa_list);
	if (!ifa) {
		rcu_read_unlock();
		DEBUG_TRACE("%s: dev->ip_ptr->ifa_list is NULL\n", dev->name);
		return false;
	}
	rcu_read_unlock();

	return true;
}

/*
 * ecm_ipv4_node_establish_and_ref()
 *	Returns a reference to a node, possibly creating one if necessary.
 *
 * The given_node_addr will be used if provided.
 *
 * Returns NULL on failure.
 */
struct ecm_db_node_instance *ecm_ipv4_node_establish_and_ref(struct ecm_front_end_connection_instance *feci,
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
#if defined(ECM_INTERFACE_L2TPV2_ENABLE) || defined(ECM_INTERFACE_PPTP_ENABLE)
	ip_addr_t local_ip, remote_ip;
#endif

#if defined(ECM_INTERFACE_VXLAN_ENABLE) || defined(ECM_INTERFACE_L2TPV2_ENABLE) || defined(ECM_INTERFACE_PPTP_ENABLE)
	struct net_device *local_dev;
#endif

#if defined(ECM_INTERFACE_MAP_T_ENABLE) || defined(ECM_INTERFACE_GRE_TUN_ENABLE) || defined(ECM_XFRM_ENABLE)
	struct net_device *in;
#endif

#ifdef ECM_INTERFACE_GRE_TUN_ENABLE
	struct ip_tunnel *gre4_tunnel;
	struct ip6_tnl *gre6_tunnel;
	ip_addr_t local_gre_tun_ip;
#endif

	DEBUG_INFO("%px: Establish node for " ECM_IP_ADDR_DOT_FMT "\n", feci, ECM_IP_ADDR_TO_DOT(addr));

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
		DEBUG_TRACE("%px: Using given node address: %pM\n", feci, node_addr);
	}

	for (i = ECM_DB_IFACE_HEIRARCHY_MAX - 1; (!done) && (i >= interface_list_first); i--) {
		ecm_db_iface_type_t type;
#ifdef ECM_INTERFACE_PPPOE_ENABLE
		struct ecm_db_interface_info_pppoe pppoe_info;
#endif
#ifdef ECM_INTERFACE_L2TPV2_ENABLE
		struct ecm_db_interface_info_pppol2tpv2 pppol2tpv2_info;
#endif
#ifdef ECM_INTERFACE_PPTP_ENABLE
		struct ecm_db_interface_info_pptp pptp_info;
#endif
		struct net_device *mac_dev;

		type = ecm_db_iface_type_get(interface_list[i]);
		DEBUG_INFO("%px: Lookup node address, interface @ %d is type: %d\n", feci, i, type);

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
			DEBUG_TRACE("%px:PPPoE interface unsupported\n", feci);
			return NULL;
#endif
		case ECM_DB_IFACE_TYPE_SIT:
		case ECM_DB_IFACE_TYPE_TUNIPIP6:
			done = true;
			break;

		case ECM_DB_IFACE_TYPE_PPPOL2TPV2:
#ifdef ECM_INTERFACE_L2TPV2_ENABLE
			ecm_db_iface_pppol2tpv2_session_info_get(interface_list[i], &pppol2tpv2_info);
			ECM_HIN4_ADDR_TO_IP_ADDR(local_ip, pppol2tpv2_info.ip.saddr);
			ECM_HIN4_ADDR_TO_IP_ADDR(remote_ip, pppol2tpv2_info.ip.daddr);
			DEBUG_TRACE("%px: local=" ECM_IP_ADDR_DOT_FMT " remote=" ECM_IP_ADDR_DOT_FMT " addr=" ECM_IP_ADDR_DOT_FMT "\n", feci,
			       ECM_IP_ADDR_TO_DOT(local_ip), ECM_IP_ADDR_TO_DOT(remote_ip), ECM_IP_ADDR_TO_DOT(addr));

			local_dev = ecm_interface_dev_find_by_local_addr(local_ip);
			if (!local_dev) {
				DEBUG_WARN("%px: Failed to find local netdevice of l2tp tunnel for " ECM_IP_ADDR_DOT_FMT "\n", feci, ECM_IP_ADDR_TO_DOT(local_ip));
				return NULL;
			}

			DEBUG_TRACE("%px: local_dev found is %s\n", feci, local_dev->name);

			/*
			 * If the local_dev is a PPP device, we support only PPPoE devices.
			 */
			if (local_dev->type == ARPHRD_PPP) {
#ifndef ECM_INTERFACE_PPPOE_ENABLE
				DEBUG_TRACE("%px: l2tp over netdevice %s unsupported\n", feci, local_dev->name);
				dev_put(local_dev);
				return NULL;
#else
				if (!ecm_interface_mac_addr_get_pppoe(local_dev, node_addr)) {
					DEBUG_WARN("%px: Unable to get any PPPoE device MAC address\n", feci);
					dev_put(local_dev);
					return NULL;
				}

				dev_put(local_dev);
				done = true;
				break;
#endif
			}

#ifdef ECM_INTERFACE_RAWIP_ENABLE
			/*
			 * If the local dev is Rmnet device, no need to do a MAC lookup.
			 */
			if (local_dev->type == ARPHRD_RAWIP) {
				dev_put(local_dev);
				done = true;
				break;
			}
#endif
			if (ECM_IP_ADDR_MATCH(local_ip, addr)) {
				if (unlikely(!ecm_interface_mac_addr_get_no_route(local_dev, local_ip, node_addr))) {
					DEBUG_WARN("%px: failed to obtain node address for " ECM_IP_ADDR_DOT_FMT "\n", feci, ECM_IP_ADDR_TO_DOT(local_ip));
					dev_put(local_dev);
					return NULL;
				}

			} else {
				/*
				 * Try to get the host mac address from the
				 * neigbour list.
				 */
				if (unlikely(!ecm_interface_mac_addr_get_no_route(local_dev, remote_ip, node_addr))) {
					ip_addr_t gw_addr = ECM_IP_ADDR_NULL;

					/*
					 * If the host is behind a route device, use the gateway's mac
					 * address instead.
					 */
					if (ecm_interface_find_gateway(remote_ip, NULL, gw_addr)) {
						DEBUG_TRACE("%px: Have a gw address " ECM_IP_ADDR_DOT_FMT "\n", feci, ECM_IP_ADDR_TO_DOT(gw_addr));
						if (!ecm_interface_mac_addr_get_no_route(local_dev, gw_addr, node_addr)) {
							DEBUG_TRACE("%px: Failed to find the mac address for gateway\n", feci);
							dev_put(local_dev);
							return NULL;
						}
					} else {
						DEBUG_WARN("%px: failed to obtain node address for host " ECM_IP_ADDR_DOT_FMT "\n", feci, ECM_IP_ADDR_TO_DOT(remote_ip));
						dev_put(local_dev);
						return NULL;
					}
				}
			}

			dev_put(local_dev);
			done = true;
			break;
#else
			DEBUG_TRACE("%px: PPPoL2TPV2 interface unsupported\n", feci);
			return NULL;
#endif
		case ECM_DB_IFACE_TYPE_PPTP:
#ifdef ECM_INTERFACE_PPTP_ENABLE
			ecm_db_iface_pptp_session_info_get(interface_list[i], &pptp_info);
			ECM_HIN4_ADDR_TO_IP_ADDR(local_ip, pptp_info.src_ip);
			ECM_HIN4_ADDR_TO_IP_ADDR(remote_ip, pptp_info.dst_ip);
			DEBUG_TRACE("%px: local=" ECM_IP_ADDR_DOT_FMT " remote=" ECM_IP_ADDR_DOT_FMT " addr=" ECM_IP_ADDR_DOT_FMT "\n", feci,
			       ECM_IP_ADDR_TO_DOT(local_ip), ECM_IP_ADDR_TO_DOT(remote_ip), ECM_IP_ADDR_TO_DOT(addr));

			local_dev = ecm_interface_dev_find_by_local_addr(local_ip);

			if (!local_dev) {
				DEBUG_WARN("%px: Failed to find local netdevice of pptp tunnel for " ECM_IP_ADDR_DOT_FMT "\n", feci, ECM_IP_ADDR_TO_DOT(local_ip));
				return NULL;
			}

			DEBUG_TRACE("%px: local_dev found is %s\n", feci, local_dev->name);

			/*
			 * If the local_dev is a PPP device, we support only PPPoE devices.
			 */
			if (local_dev->type == ARPHRD_PPP) {
#ifndef ECM_INTERFACE_PPPOE_ENABLE
				DEBUG_TRACE("%px: PPTP over netdevice %s unsupported\n", feci, local_dev->name);
				dev_put(local_dev);
				return NULL;
#else
				if (!ecm_interface_mac_addr_get_pppoe(local_dev, node_addr)) {
					DEBUG_WARN("%px: Unable to get any PPPoE device MAC address\n", feci);
					dev_put(local_dev);
					return NULL;
				}

				dev_put(local_dev);
				done = true;
				break;
#endif
			}

			if (ECM_IP_ADDR_MATCH(local_ip, addr)) {
				if (unlikely(!ecm_interface_mac_addr_get_no_route(local_dev, local_ip, node_addr))) {
					DEBUG_WARN("%px: failed to obtain node address for " ECM_IP_ADDR_DOT_FMT "\n", feci, ECM_IP_ADDR_TO_DOT(local_ip));
					dev_put(local_dev);
					return NULL;
				}

			} else {
				if (unlikely(!ecm_interface_mac_addr_get_no_route(local_dev, remote_ip, node_addr))) {
					ip_addr_t gw_addr = ECM_IP_ADDR_NULL;

					if (!ecm_interface_find_gateway(remote_ip, NULL, gw_addr)) {
						DEBUG_WARN("%px: failed to obtain Gateway address for host " ECM_IP_ADDR_DOT_FMT "\n", feci, ECM_IP_ADDR_TO_DOT(remote_ip));
						dev_put(local_dev);
						return NULL;
					}

					if (ECM_IP_ADDR_MATCH(gw_addr, remote_ip)) {
						DEBUG_WARN("%px: host ip address match with gw address " ECM_IP_ADDR_DOT_FMT "\n", feci, ECM_IP_ADDR_TO_DOT(remote_ip));
						dev_put(local_dev);
						return NULL;
					}

					if (!ecm_interface_mac_addr_get_no_route(local_dev, gw_addr, node_addr)) {
						DEBUG_WARN("%px: failed to obtain node address for host " ECM_IP_ADDR_DOT_FMT "\n", feci, ECM_IP_ADDR_TO_DOT(gw_addr));
						dev_put(local_dev);
						return NULL;
					}
				}
			}

			dev_put(local_dev);
			done = true;
			break;
#else
			DEBUG_TRACE("%px: PPTP interface unsupported\n", feci);
			return NULL;
#endif
		case ECM_DB_IFACE_TYPE_MAP_T:
#ifdef ECM_INTERFACE_MAP_T_ENABLE
			in = dev_get_by_index(&init_net, skb->skb_iif);
			if (!in) {
				DEBUG_WARN("%px: failed to obtain node address for host " ECM_IP_ADDR_DOT_FMT "\n", feci, ECM_IP_ADDR_TO_DOT(addr));
				return NULL;
			}
			memcpy(node_addr, in->dev_addr, ETH_ALEN);
			dev_put(in);
			done = true;
			break;
#else
			DEBUG_TRACE("%px: MAP-T interface unsupported\n", feci);
			return NULL;
#endif
		case ECM_DB_IFACE_TYPE_GRE_TUN:
#ifdef ECM_INTERFACE_GRE_TUN_ENABLE
			in = dev_get_by_index(&init_net, skb->skb_iif);
			if (!in) {
				DEBUG_WARN("%px: failed to obtain node address for host " ECM_IP_ADDR_DOT_FMT "\n", feci, ECM_IP_ADDR_TO_DOT(addr));
				return NULL;
			}

			switch(in->type) {
			case ARPHRD_IPGRE:
				gre4_tunnel = netdev_priv(in);
				if (!gre4_tunnel) {
					dev_put(in);
					DEBUG_WARN("%px: failed to obtain node address for host. GREv4 tunnel not initialized\n", feci);
					return NULL;
				}
				ECM_NIN4_ADDR_TO_IP_ADDR(local_gre_tun_ip, gre4_tunnel->parms.iph.saddr);
				dev_put(in);
				in = ecm_interface_dev_find_by_local_addr(local_gre_tun_ip);
				if (!in) {
					DEBUG_WARN("%px: failed to obtain node address for host " ECM_IP_ADDR_DOT_FMT "\n", feci, ECM_IP_ADDR_TO_DOT(local_gre_tun_ip));
					return NULL;
				}
				break;

			case ARPHRD_IP6GRE:
				gre6_tunnel = netdev_priv(in);
				if (!gre6_tunnel) {
					dev_put(in);
					DEBUG_WARN("%px: failed to obtain node address for host. GREv6 tunnel not initialized\n", feci);
					return NULL;
				}
				ECM_NIN6_ADDR_TO_IP_ADDR(local_gre_tun_ip, gre6_tunnel->parms.laddr);
				dev_put(in);
				in = ecm_interface_dev_find_by_local_addr(local_gre_tun_ip);
				if (!in) {
					DEBUG_WARN("%px: failed to obtain node address for host " ECM_IP_ADDR_OCTAL_FMT "\n", feci, ECM_IP_ADDR_TO_OCTAL(local_gre_tun_ip));
					return NULL;
				}
				break;

			default:
				DEBUG_TRACE("%px: establish node with physical netdev: %s\n", feci, in->name);
			}
			memcpy(node_addr, in->dev_addr, ETH_ALEN);
			dev_put(in);
			done = true;
			break;
#else
			DEBUG_TRACE("%px: GRE Tunnel interface unsupported\n", feci);
			return NULL;
#endif
		case ECM_DB_IFACE_TYPE_IPSEC_TUNNEL:
#ifdef ECM_XFRM_ENABLE
			if (dst_xfrm(skb_dst(skb))) {
				ether_addr_copy(node_addr, dev->dev_addr);
				done = true;
				break;
			}

			if (secpath_exists(skb)) {
				in = dev_get_by_index(&init_net, skb->skb_iif);
				if (!in) {
					DEBUG_WARN("%px: failed to obtain node address for host " ECM_IP_ADDR_DOT_FMT "\n", feci, ECM_IP_ADDR_TO_DOT(addr));
					return NULL;
				}

				ether_addr_copy(node_addr, in->dev_addr);
				dev_put(in);
				done = true;
				break;
			}
#if __has_attribute(__fallthrough__)
			__attribute__((__fallthrough__));
#endif
#endif
		case ECM_DB_IFACE_TYPE_ETHERNET:
#ifdef ECM_INTERFACE_VLAN_ENABLE
		case ECM_DB_IFACE_TYPE_VLAN:
#endif
		case ECM_DB_IFACE_TYPE_LAG:
		case ECM_DB_IFACE_TYPE_BRIDGE:
#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
		case ECM_DB_IFACE_TYPE_OVS_BRIDGE:
#endif
			/*
			 * If dev is a bridge port, we should use the bridge device for the MAC lookup and ARP request.
			 * For brouting case where dev is a bridge port and also is a routing interface (so dev has an
			 * IP address), we should use dev itself for the MAC lookup and ARP request.
			 */
			if (ecm_front_end_is_bridge_port(dev) && !ecm_ipv4_dev_has_ipaddr(dev)) {
				DEBUG_TRACE("%s is a bridge port\n", dev->name);
				mac_dev = ecm_interface_get_and_hold_dev_master(dev);
				if(!mac_dev) {
					DEBUG_WARN("%px: No master for %s, failed to obtain any node address for host " ECM_IP_ADDR_DOT_FMT "\n", feci, dev->name, ECM_IP_ADDR_TO_DOT(addr));
					return NULL;
				}
			} else {
				dev_hold(dev);
				mac_dev = dev;
				DEBUG_TRACE("%s is not a bridge port\n", dev->name);
			}

			if (!ecm_interface_mac_addr_get_no_route(mac_dev, addr, node_addr)) {
				ip_addr_t gw_addr = ECM_IP_ADDR_NULL;
				bool on_link = true;

				/*
				 * Check if we have a gateway address. If yes, first we will try to get the MAC address
				 * of that gateway. If it fails, we will send ARP request to that address
				 * to find the node MAC address while processing the subsequent packets.
				 */
				if (ecm_interface_find_gateway(addr, NULL, gw_addr)) {
					DEBUG_TRACE("%px: Have a gw address " ECM_IP_ADDR_DOT_FMT "\n", feci, ECM_IP_ADDR_TO_DOT(gw_addr));
					if (ecm_interface_mac_addr_get_no_route(mac_dev, gw_addr, node_addr)) {
						DEBUG_TRACE("%px: Found the mac address for gateway\n", feci);
						dev_put(mac_dev);
						goto done;
					}
					on_link = false;
				}

				ecm_interface_send_arp_request(mac_dev, addr, on_link, gw_addr);

				DEBUG_WARN("%px: failed to obtain any node address for host " ECM_IP_ADDR_DOT_FMT "\n", feci, ECM_IP_ADDR_TO_DOT(addr));
				dev_put(mac_dev);

				/*
				 * Unable to get node address at this time.
				 */
				return NULL;
			}
			dev_put(mac_dev);
done:
			if (is_multicast_ether_addr(node_addr)) {
				DEBUG_TRACE("%px: multicast node address for host " ECM_IP_ADDR_DOT_FMT ", node_addr: %pM\n", feci, ECM_IP_ADDR_TO_DOT(addr), node_addr);
				return NULL;
			}

			/*
			 * Because we are iterating from inner to outer interface, this interface is the
			 * innermost one that has a node address - take this one.
			 */
			done = true;
			break;

		case ECM_DB_IFACE_TYPE_RAWIP:
#ifdef ECM_INTERFACE_RAWIP_ENABLE
			done = true;
			break;
#else
			DEBUG_TRACE("%px: RAWIP interface unsupported\n", feci);
			return NULL;
#endif
		case ECM_DB_IFACE_TYPE_OVPN:
#ifdef ECM_INTERFACE_OVPN_ENABLE
			done = true;
			break;
#else
			DEBUG_TRACE("%px: OVPN interface unsupported\n", feci);
			return NULL;
#endif
		case ECM_DB_IFACE_TYPE_VXLAN:
#ifdef ECM_INTERFACE_VXLAN_ENABLE
			local_dev = ecm_interface_dev_find_by_local_addr(addr);
			if (!local_dev) {
				DEBUG_WARN("%px: Failed to find local netdevice of VxLAN tunnel for " ECM_IP_ADDR_DOT_FMT "\n",
						feci, ECM_IP_ADDR_TO_DOT(addr));
				return NULL;
			}

			if (!ecm_interface_mac_addr_get_no_route(local_dev, addr, node_addr)) {
				DEBUG_WARN("%px: Couldn't find mac address for local dev\n", feci);
				dev_put(local_dev);
				return NULL;
			}
			DEBUG_TRACE("%px: Found the mac address for local dev\n", feci);
			dev_put(local_dev);
			done = true;
			break;
#else
			DEBUG_TRACE("%px: VXLAN interface unsupported\n", feci);
			return NULL;
#endif
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
		DEBUG_WARN("%px: Failed to establish node for " ECM_IP_ADDR_DOT_FMT "\n", feci, ECM_IP_ADDR_TO_DOT(addr));
		return NULL;
	}

	/*
	 *  Establish iface
	 */
	ii = ecm_interface_establish_and_ref(feci, dev, skb);
	if (!ii) {
		DEBUG_WARN("%px: Failed to establish iface\n", feci);
		return NULL;
	}

	/*
	 * Locate the node
	 */
	ni = ecm_db_node_find_and_ref(node_addr, ii);
	if (ni) {
		DEBUG_TRACE("%px: node established: %px\n", feci, ni);
		ecm_db_iface_deref(ii);
		return ni;
	}

	/*
	 * No node - create one
	 */
	nni = ecm_db_node_alloc();
	if (!nni) {
		DEBUG_WARN("%px: Failed to establish node\n", feci);
		ecm_db_iface_deref(ii);
		return NULL;
	}

	/*
	 * Add node into the database, atomically to avoid races creating the same thing
	 */
	spin_lock_bh(&ecm_ipv4_lock);
	ni = ecm_db_node_find_and_ref(node_addr, ii);
	if (ni) {
		spin_unlock_bh(&ecm_ipv4_lock);
		ecm_db_node_deref(nni);
		ecm_db_iface_deref(ii);
		return ni;
	}

	ecm_db_node_add(nni, ii, node_addr, NULL, nni);
	spin_unlock_bh(&ecm_ipv4_lock);

	/*
	 * Don't need iface instance now
	 */
	ecm_db_iface_deref(ii);

	DEBUG_TRACE("%px: node (%px) established, node address: %pM\n", feci, nni, node_addr);
	return nni;
}

/*
 * ecm_ipv4_host_establish_and_ref()
 *	Returns a reference to a host, possibly creating one if necessary.
 *
 * Returns NULL on failure.
 */
struct ecm_db_host_instance *ecm_ipv4_host_establish_and_ref(ip_addr_t addr)
{
	struct ecm_db_host_instance *hi;
	struct ecm_db_host_instance *nhi;

	DEBUG_INFO("Establish host for " ECM_IP_ADDR_DOT_FMT "\n", ECM_IP_ADDR_TO_DOT(addr));

	/*
	 * Locate the host
	 */
	hi = ecm_db_host_find_and_ref(addr);
	if (hi) {
		DEBUG_TRACE("%px: host established\n", hi);
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
	spin_lock_bh(&ecm_ipv4_lock);
	hi = ecm_db_host_find_and_ref(addr);
	if (hi) {
		spin_unlock_bh(&ecm_ipv4_lock);
		ecm_db_host_deref(nhi);
		return hi;
	}

	ecm_db_host_add(nhi, addr, true, NULL, nhi);

	spin_unlock_bh(&ecm_ipv4_lock);

	DEBUG_TRACE("%px: host established\n", nhi);
	return nhi;
}

/*
 * ecm_ipv4_mapping_establish_and_ref()
 *	Returns a reference to a mapping, possibly creating one if necessary.
 *
 * Returns NULL on failure.
 */
struct ecm_db_mapping_instance *ecm_ipv4_mapping_establish_and_ref(ip_addr_t addr, int port)
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
		DEBUG_TRACE("%px: mapping established\n", mi);
		return mi;
	}

	/*
	 * No mapping - establish host existence
	 */
	hi = ecm_ipv4_host_establish_and_ref(addr);
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
	spin_lock_bh(&ecm_ipv4_lock);
	mi = ecm_db_mapping_find_and_ref(addr, port);
	if (mi) {
		spin_unlock_bh(&ecm_ipv4_lock);
		ecm_db_mapping_deref(nmi);
		ecm_db_host_deref(hi);
		return mi;
	}

	ecm_db_mapping_add(nmi, hi, port, NULL, nmi);

	spin_unlock_bh(&ecm_ipv4_lock);

	/*
	 * Don't need the host instance now - the mapping maintains a reference to it now.
	 */
	ecm_db_host_deref(hi);

	/*
	 * Return the mapping instance
	 */
	DEBUG_INFO("%px: mapping established\n", nmi);
	return nmi;
}

/*
 * ecm_ipv4_connection_regenerate()
 *	Re-generate a connection.
 *
 * Re-generating a connection involves re-evaluating the interface lists in case interface heirarchies have changed.
 * It also involves the possible triggering of classifier re-evaluation but only if all currently assigned
 * classifiers permit this operation.
 */
void ecm_ipv4_connection_regenerate(struct ecm_db_connection_instance *ci, ecm_tracker_sender_type_t sender,
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
	struct ecm_front_end_ovs_params *from_ovs_params = NULL;
	struct ecm_front_end_ovs_params *to_ovs_params = NULL;
	struct ecm_front_end_ovs_params *from_nat_ovs_params = NULL;
	struct ecm_front_end_ovs_params *to_nat_ovs_params = NULL;

	DEBUG_INFO("%px: re-gen needed\n", ci);

	/*
	 * We may need to swap the devices around depending on who the sender of the packet that triggered the re-gen is
	 */
	if (sender == ECM_TRACKER_SENDER_TYPE_DEST) {
		struct net_device *tmp_dev;

		/*
		 * This is a packet sent by the destination of the connection, i.e. it is a packet issued by the 'from' side of the connection.
		 */
		DEBUG_TRACE("%px: Re-gen swap devs\n", ci);
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

	if ((protocol == IPPROTO_TCP) || (protocol == IPPROTO_UDP)) {
		int src_port, src_port_nat, dest_port, dest_port_nat;
		struct ecm_front_end_ovs_params ovs_params[ECM_DB_OBJ_DIR_MAX];

		src_port = ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_FROM);
		src_port_nat = ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_FROM_NAT);
		dest_port = ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_TO);
		dest_port_nat = ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_TO_NAT);

		ecm_front_end_fill_ovs_params(ovs_params,
				      ip_src_addr, ip_src_addr_nat,
				      ip_dest_addr, ip_dest_addr_nat,
				      src_port, src_port_nat,
				      dest_port, dest_port_nat, ecm_dir);

		from_ovs_params = &ovs_params[ECM_DB_OBJ_DIR_FROM];
		to_ovs_params = &ovs_params[ECM_DB_OBJ_DIR_TO];
		from_nat_ovs_params = &ovs_params[ECM_DB_OBJ_DIR_FROM_NAT];
		to_nat_ovs_params = &ovs_params[ECM_DB_OBJ_DIR_TO_NAT];
	}

	DEBUG_TRACE("%px: Update the 'from' interface heirarchy list\n", ci);
	from_list_first = ecm_interface_heirarchy_construct(feci, from_list, efeici.from_dev, efeici.from_other_dev, ip_dest_addr, efeici.from_mac_lookup_ip_addr, ip_src_addr, 4, protocol, in_dev, is_routed, in_dev, src_node_addr, dest_node_addr, layer4hdr, skb, from_ovs_params);
	if (from_list_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
		ecm_front_end_ipv4_interface_construct_netdev_put(&efeici);
		goto ecm_ipv4_retry_regen;
	}

	ecm_db_connection_interfaces_reset(ci, from_list, from_list_first, ECM_DB_OBJ_DIR_FROM);
	ecm_db_connection_interfaces_deref(from_list, from_list_first);

	DEBUG_TRACE("%px: Update the 'from NAT' interface heirarchy list\n", ci);
	if ((protocol == IPPROTO_IPV6) || (protocol == IPPROTO_ESP)) {
		from_nat_list_first = ecm_interface_heirarchy_construct(feci, from_nat_list, efeici.from_nat_dev, efeici.from_nat_other_dev, ip_dest_addr, efeici.from_nat_mac_lookup_ip_addr, ip_src_addr_nat, 4, protocol, in_dev, is_routed, in_dev, src_node_addr_nat, dest_node_addr_nat, layer4hdr, skb, from_nat_ovs_params);
	} else {
		from_nat_list_first = ecm_interface_heirarchy_construct(feci, from_nat_list, efeici.from_nat_dev, efeici.from_nat_other_dev, ip_dest_addr, efeici.from_nat_mac_lookup_ip_addr, ip_src_addr_nat, 4, protocol, in_dev_nat, is_routed, in_dev_nat, src_node_addr_nat, dest_node_addr_nat, layer4hdr, skb, from_nat_ovs_params);
	}

	if (from_nat_list_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
		ecm_front_end_ipv4_interface_construct_netdev_put(&efeici);
		goto ecm_ipv4_retry_regen;
	}

	ecm_db_connection_interfaces_reset(ci, from_nat_list, from_nat_list_first, ECM_DB_OBJ_DIR_FROM_NAT);
	ecm_db_connection_interfaces_deref(from_nat_list, from_nat_list_first);

	DEBUG_TRACE("%px: Update the 'to' interface heirarchy list\n", ci);
	to_list_first = ecm_interface_heirarchy_construct(feci, to_list, efeici.to_dev, efeici.to_other_dev, ip_src_addr, efeici.to_mac_lookup_ip_addr, ip_dest_addr, 4, protocol, out_dev, is_routed, in_dev, dest_node_addr, src_node_addr, layer4hdr, skb, to_ovs_params);
	if (to_list_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
		ecm_front_end_ipv4_interface_construct_netdev_put(&efeici);
		goto ecm_ipv4_retry_regen;
	}

	ecm_db_connection_interfaces_reset(ci, to_list, to_list_first, ECM_DB_OBJ_DIR_TO);
	ecm_db_connection_interfaces_deref(to_list, to_list_first);

	DEBUG_TRACE("%px: Update the 'to NAT' interface heirarchy list\n", ci);
	to_nat_list_first = ecm_interface_heirarchy_construct(feci, to_nat_list, efeici.to_nat_dev, efeici.to_nat_other_dev, ip_src_addr, efeici.to_nat_mac_lookup_ip_addr, ip_dest_addr_nat, 4, protocol, out_dev_nat, is_routed, in_dev, dest_node_addr_nat, src_node_addr_nat, layer4hdr, skb, to_nat_ovs_params);
	if (to_nat_list_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
		ecm_front_end_ipv4_interface_construct_netdev_put(&efeici);
		goto ecm_ipv4_retry_regen;
	}

	ecm_front_end_ipv4_interface_construct_netdev_put(&efeici);

	ecm_front_end_connection_deref(feci);
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
		goto ecm_ipv4_regen_done;
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
		goto ecm_ipv4_regen_done;
	}
	DEBUG_INFO("%px: reclassify success\n", ci);

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
	ecm_front_end_connection_deref(feci);
	ecm_db_connection_regeneration_failed(ci);
	return;
}

/*
 * ecm_ipv4_ip_process()
 *	Process IP datagram skb
 */
unsigned int ecm_ipv4_ip_process(struct net_device *out_dev, struct net_device *in_dev,
							uint8_t *src_node_addr, uint8_t *dest_node_addr,
							bool can_accel, bool is_routed, bool is_l2_encap, struct sk_buff *skb, uint16_t l2_encap_proto)
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
	uint8_t protonum;

	/*
	 * Obtain the IP header from the skb
	 */
	if (!ecm_tracker_ip_check_header_and_read(&ip_hdr, skb)) {
		DEBUG_WARN("Invalid ip header in skb %px\n", skb);
		ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_MALFORMED_IP_HEADER);
		return NF_ACCEPT;
	}

	/*
	 * Process only IPv4 packets
	 */
	if (!ip_hdr.is_v4) {
		DEBUG_TRACE("Not an IPv4 packet, skb %px\n", skb);
		ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_NON_IPV4_HDR);
		return NF_ACCEPT;
	}

	if (ip_hdr.fragmented) {
		DEBUG_TRACE("skb %px is fragmented\n", skb);
		ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_FRAGMENTED_PACKETS);
		return NF_ACCEPT;
	}

	/*
	 * Extract information, if we have conntrack then use that info as far as we can.
	 */
	ct = nf_ct_get(skb, &ctinfo);
	if (unlikely(!ct)) {
		DEBUG_TRACE("%px: no ct\n", skb);
		ECM_IP_ADDR_TO_NIN4_ADDR(orig_tuple.src.u3.ip, ip_hdr.src_addr);
		ECM_IP_ADDR_TO_NIN4_ADDR(orig_tuple.dst.u3.ip, ip_hdr.dest_addr);
		orig_tuple.dst.protonum = ip_hdr.protocol;
		reply_tuple.src.u3.ip = orig_tuple.dst.u3.ip;
		reply_tuple.dst.u3.ip = orig_tuple.src.u3.ip;
		sender = ECM_TRACKER_SENDER_TYPE_SRC;
	} else {
		/*
		 * Do not process the packet, if the conntrack is in dying state.
		 */
		if (unlikely(test_bit(IPS_DYING_BIT, &ct->status))) {
			DEBUG_WARN("%px: ct: %px is in dying state\n", skb, ct);
			ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_CONNTRACK_IN_DYING_STATE);
			return NF_ACCEPT;
		}

		/*
		 * Fake untracked conntrack objects were removed on 4.12 kernel version
		 * and onwards.
		 * So, for the newer kernels, instead of comparing the ct with the percpu
		 * fake conntrack, we can check the ct status.
		 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0))
		if (unlikely(ct == nf_ct_untracked_get())) {
#else
		if (unlikely(ctinfo == IP_CT_UNTRACKED)) {
#endif
#ifdef ECM_INTERFACE_VXLAN_ENABLE
			/*
			 * If the conntrack connection is set as untracked,
			 * ECM will accept and process only VxLAN outer flows,
			 * otherwise such flows will not be processed by ECM.
			 *
			 * E.g. In the following network arramgement,
			 * Eth1 ---> Bridge ---> VxLAN0(Bridge Port) ---> Eth0(WAN)
			 * The packets from VxLAN0 to Eth0 will be routed.
			 *
			 * netif_is_vxlan API is used to identify the VxLAN device &
			 * is_routed flag is used to identify the outer flow.
			 */
			if (is_routed && netif_is_vxlan(in_dev)) {
				DEBUG_TRACE("%px: Untracked CT for VxLAN\n", skb);
				ECM_IP_ADDR_TO_NIN4_ADDR(orig_tuple.src.u3.ip, ip_hdr.src_addr);
				ECM_IP_ADDR_TO_NIN4_ADDR(orig_tuple.dst.u3.ip, ip_hdr.dest_addr);
				orig_tuple.dst.protonum = ip_hdr.protocol;
				reply_tuple.src.u3.ip = orig_tuple.dst.u3.ip;
				reply_tuple.dst.u3.ip = orig_tuple.src.u3.ip;
				sender = ECM_TRACKER_SENDER_TYPE_SRC;
				ct = NULL;
				goto vxlan_done;
			}
#endif
			DEBUG_TRACE("%px: ct: untracked\n", skb);
			ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_UNTRACKED_CONNTRACK);
			return NF_ACCEPT;
		}

		/*
		 * If the conntrack connection is using a helper (i.e. Application Layer Gateway)
		 * then acceleration is denied (connection needs assistance from HLOS to function)
		 */
		if (nfct_help(ct)) {
			DEBUG_TRACE("%px: Connection has helper\n", ct);
			can_accel = false;
			ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_CONN_HAS_HELPER);
		}

		/*
		 * Extract conntrack connection information
		 */
		DEBUG_TRACE("%px: ct: %px, ctinfo: %x\n", skb, ct, ctinfo);
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
			DEBUG_TRACE("%px: related ct, actual protocol: %u\n", skb, orig_tuple.dst.protonum);
		}
#ifdef ECM_INTERFACE_VXLAN_ENABLE
vxlan_done:
		;
#endif
	}

	/*
	 * Check if we can accelerate the GRE protocol.
	 */
	if (ip_hdr.protocol == IPPROTO_GRE) {
		uint16_t offset = ip_hdr.headers[ECM_TRACKER_IP_PROTOCOL_TYPE_GRE].offset;
		if (!ecm_front_end_gre_proto_is_accel_allowed(in_dev, out_dev, skb, &orig_tuple, &reply_tuple, 4, offset)) {
			DEBUG_WARN("%px: GRE protocol is not allowed\n", skb);
			ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_UNSUPPORTED_GRE_PROTOCOL);
			return NF_ACCEPT;
		}
	}

	/*
	 * Check if we can accelerate L2TPv3 protocol.
	 */
	if (ip_hdr.protocol == IPPROTO_L2TP) {
		if (!ecm_front_end_l2tp_proto_is_accel_allowed(in_dev, out_dev)) {
			DEBUG_WARN("%px: L2TPv3 protocol is not allowed\n", skb);
			ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_UNSUPPORTED_L2TPV3_PROTOCOL);
			return NF_ACCEPT;
		}
	}

	/*
	 * Check for a multicast Destination address here.
	 */
	ECM_NIN4_ADDR_TO_IP_ADDR(ip_dest_addr, orig_tuple.dst.u3.ip);
	if (ecm_ip_addr_is_multicast(ip_dest_addr)) {
#ifdef ECM_MULTICAST_ENABLE
		if (unlikely(ecm_front_end_ipv4_mc_stopped)) {
			DEBUG_TRACE("%px: Multicast disabled by ecm_front_end_ipv4_mc_stopped = %d\n", skb, ecm_front_end_ipv4_mc_stopped);
			ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_MCAST_STOPPED);
			return NF_ACCEPT;
		}

		if (unlikely(!ecm_front_end_is_feature_supported(ECM_FE_FEATURE_MULTICAST))) {
			DEBUG_TRACE("%px: Multicast ipv4 acceleration is not supported on the selected frontend\n", skb);
			ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_MCAST_NOT_SUPPORTED);
			return NF_ACCEPT;
		}

		DEBUG_TRACE("%px: CMN Multicast, Processing\n", skb);
		return ecm_multicast_ipv4_connection_process(out_dev, in_dev, src_node_addr, dest_node_addr,
									can_accel, is_routed, skb, &ip_hdr, ct, sender,
									&orig_tuple, &reply_tuple);
#else
		ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_MCAST_FEATURE_DISABLED);
		return NF_ACCEPT;
#endif
	}

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

	/*
	 * Is ecm_dir consistent with is_routed flag?
	 * In SNAT and hairpin NAT scenario, while accessing the LAN side server with its private
	 * IP address from another client in the same LAN, the packets come through the bridge post routing hook
	 * have the WAN interface IP address as the SNAT address. Then in the above ecm_dir calculation,
	 * it is calculated as ECM_DB_DIRECTION_EGRESS_NAT. So, we shouldn't accelerate the flow this time
	 * and wait for the packet to pass through the post routing hook.
	 *
	 */
	if (!is_routed && (ecm_dir != ECM_DB_DIRECTION_BRIDGED)) {
		DEBUG_TRACE("Packet comes from bridge post routing hook but ecm_dir is not bridge\n");
		ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_BRIDGE_ECM_DIR_MISMATCH);
		return NF_ACCEPT;
	}

	/*
	 * If PPPoE bridged flows are to be handled with 3-tuple rule, set protocol to IPPROTO_RAW.
	 */
	protonum = orig_tuple.dst.protonum;
	if (unlikely(!is_routed && (l2_encap_proto == ETH_P_PPP_SES))) {
		/*
		 * Check if PPPoE bridge acceleration is 3-tuple based.
		 */
		if (ecm_front_end_ppppoe_br_accel_3tuple()) {
			DEBUG_TRACE("3-tuple acceleration is enabled for PPPoE bridged flows\n");
			protonum = IPPROTO_RAW;
		}
	}
	DEBUG_TRACE("IP Packet ORIGINAL src: %pI4 ORIGINAL dst: %pI4 protocol: %u, sender: %d ecm_dir: %d\n",
			&orig_tuple.src.u3.ip, &orig_tuple.dst.u3.ip, protonum, sender, ecm_dir);

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
		if (ecm_dir == ECM_DB_DIRECTION_EGRESS_NAT) {
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
		} else if ((ecm_dir == ECM_DB_DIRECTION_BRIDGED) || (ecm_dir == ECM_DB_DIRECTION_NON_NAT)) {
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
		if (ecm_dir == ECM_DB_DIRECTION_EGRESS_NAT) {
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
		} else if ((ecm_dir == ECM_DB_DIRECTION_BRIDGED) || (ecm_dir == ECM_DB_DIRECTION_NON_NAT)) {
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
		DEBUG_TRACE("skb %px non-unicast daddr " ECM_IP_ADDR_DOT_FMT "\n", skb, ECM_IP_ADDR_TO_DOT(ip_dest_addr));
		ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_DEST_IP_NOT_UCAST);
		return NF_ACCEPT;
	}
	if (unlikely(ecm_ip_addr_is_non_unicast(ip_src_addr))) {
		DEBUG_TRACE("skb %px non-unicast saddr " ECM_IP_ADDR_DOT_FMT "\n", skb, ECM_IP_ADDR_TO_DOT(ip_src_addr));
		ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_SRC_IP_NOT_UCAST);
		return NF_ACCEPT;
	}

	/*
	 * Process IP specific protocol
	 * TCP and UDP are the most likliest protocols.
	 */
	if (likely(protonum == IPPROTO_TCP) || likely(protonum == IPPROTO_UDP)) {
		return ecm_ported_ipv4_process(out_dev, out_dev_nat,
				in_dev, in_dev_nat,
				src_node_addr, src_node_addr_nat,
				dest_node_addr, dest_node_addr_nat,
				can_accel, is_routed, is_l2_encap, skb,
				&ip_hdr,
				ct, sender, ecm_dir,
				&orig_tuple, &reply_tuple,
				ip_src_addr, ip_dest_addr, ip_src_addr_nat, ip_dest_addr_nat, l2_encap_proto);
	}
#ifdef ECM_NON_PORTED_SUPPORT_ENABLE
	if (unlikely(!ecm_front_end_is_feature_supported(ECM_FE_FEATURE_NON_PORTED))) {
		DEBUG_TRACE("%px: Non-ported ipv4 acceleration is not supported on the selected frontend\n", skb);
		ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_NON_PORTED_NOT_SUPPORTED);
		return NF_ACCEPT;
	}

	return ecm_non_ported_ipv4_process(out_dev, out_dev_nat,
				in_dev, in_dev_nat,
				src_node_addr, src_node_addr_nat,
				dest_node_addr, dest_node_addr_nat,
				can_accel, is_routed, is_l2_encap, skb,
				&ip_hdr,
				ct, sender, ecm_dir,
				&orig_tuple, &reply_tuple,
				ip_src_addr, ip_dest_addr, ip_src_addr_nat, ip_dest_addr_nat, l2_encap_proto);
#else
	ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_NON_PORTED_DISABLED);
	return NF_ACCEPT;
#endif
}

/*
 * ecm_ipv4_is_bridge_pkt()
 *	Return true if pkt is from bridge flow.
 *	If in/out dev is a bridge port and the other dev is a master
 *	of the same bridge port dev, then consider it a bridge flow packet 
 *	and return true.
 */
static bool ecm_ipv4_is_bridge_pkt(struct net_device *in,
		struct net_device *out)
{
	struct net_device *lower = NULL;
	struct net_device *upper = NULL;
	struct net_device *bridge = NULL;

	if (in->priv_flags & IFF_BRIDGE_PORT) {
		lower = in;
		bridge = out;
	} else if (out->priv_flags & IFF_BRIDGE_PORT) {
		lower = out;
		bridge = in;
	}

	if (!lower)
		return false;

	rcu_read_lock();
	upper = netdev_master_upper_dev_get_rcu(lower);
	rcu_read_unlock();
	return upper && (upper == bridge) && !ecm_ipv4_dev_has_ipaddr(lower);
}

/*
 * ecm_ipv4_post_routing_hook()
 *	Called for IP packets that are going out to interfaces after IP routing stage.
 */
static unsigned int ecm_ipv4_post_routing_hook(void *priv,
				struct sk_buff *skb,
				const struct nf_hook_state *nhs)
{
	struct net_device *out = nhs->out;
	struct net_device *in;
	bool can_accel = true;
	unsigned int result;

	DEBUG_TRACE("%px: IPv4 CMN Routing: %s skb=%px\n", out, out->name, skb);

	/*
	 * Skip flow with out interface marked for don't offload.
	 */
	if (out->priv_flags_ext & IFF_EXT_HW_NO_OFFLOAD) {
		ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_ROUTE_OUT_IFF_NO_OFFLOAD);
		return NF_ACCEPT;
	}

	/*
	 * If operations have stopped then do not process packets
	 */
	spin_lock_bh(&ecm_ipv4_lock);
	if (unlikely(ecm_front_end_ipv4_stopped)) {
		spin_unlock_bh(&ecm_ipv4_lock);
		DEBUG_TRACE("Front end stopped\n");
		ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_FRONT_END_STOPPED);
		return NF_ACCEPT;
	}
	spin_unlock_bh(&ecm_ipv4_lock);

	/*
	 * Don't process broadcast or multicast
	 */
	if (skb->pkt_type == PACKET_BROADCAST) {
		DEBUG_TRACE("Broadcast, ignoring: %px\n", skb);
		ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_BCAST_PACKET_IGNORED);
		return NF_ACCEPT;
	}

#ifndef ECM_INTERFACE_PPTP_ENABLE
	/*
	 * skip pptp because we don't accelerate them
	 */
	if (ecm_interface_is_pptp(skb, out)) {
		ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_PPTP_DISABLED);
		return NF_ACCEPT;
	}
#endif

#ifndef ECM_INTERFACE_L2TPV2_ENABLE
	/*
	 * skip l2tpv2 because we don't accelerate them
	 */
	if (ecm_interface_is_l2tp_packet_by_version(skb, out, 2)) {
		ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_L2TPV2_DISABLED);
		return NF_ACCEPT;
	}
#endif

	/*
	 * skip l2tpv3 over PPP interface because we don't accelerate them
	 */
	if (ecm_interface_is_l2tp_packet_by_version(skb, out, 3)) {
		ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_L2TPV3_UNSUPPORTED_INTERFACE);
		return NF_ACCEPT;
	}

	/*
	 * Identify interface from where this packet came
	 */
	in = dev_get_by_index(&init_net, skb->skb_iif);
	if (unlikely(!in)) {
		/*
		 * Locally sourced packets are not processed in ECM.
		 */
		ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_LOCAL_PACKETS_IGNORED);
		return NF_ACCEPT;
	}

	/*
	 * Skip flow with source interface marked for don't offload.
	 */
	if (in->priv_flags_ext & IFF_EXT_HW_NO_OFFLOAD) {
		dev_put(in);
		ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_ROUTE_IN_IFF_NO_OFFLOAD);
		return NF_ACCEPT;
	}

	/*
	 * Skip bridge flow packet
	 */
	if (ecm_ipv4_is_bridge_pkt(in, out)) {
		DEBUG_TRACE("Bridge flow, ignoring: %px\n", skb);
		dev_put(in);
		ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_BRIDGE_PACKET_WRONG_HOOK);
		return NF_ACCEPT;
	}
#ifndef ECM_INTERFACE_OVS_BRIDGE_ENABLE
	/*
	 * skip OpenVSwitch flows because we don't accelerate them
	 */
	if (netif_is_ovs_master(out) || netif_is_ovs_master(in)) {
		dev_put(in);
		ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_OVS_DISABLED);
		return NF_ACCEPT;
	}
#endif

	DEBUG_TRACE("CMN Post routing process skb %px, out: %px (%s), in: %px (%s)\n", skb, out, out->name, in, in->name);
	result = ecm_ipv4_ip_process((struct net_device *)out, in, NULL, NULL,
							can_accel, true, false, skb, 0);
	dev_put(in);
	return result;
}

/*
 * ecm_ipv4_pppoe_bridge_process()
 *	Called for PPPoE session packets that are going
 *	out to one of the bridge physical interfaces.
 */
static unsigned int ecm_ipv4_pppoe_bridge_process(struct net_device *out,
						     struct net_device *in,
						     struct ethhdr *skb_eth_hdr,
						     bool can_accel,
						     struct sk_buff *skb)
{
	struct ecm_tracker_ip_header ip_hdr;
	unsigned int result = NF_ACCEPT;
	struct pppoe_hdr *ph = pppoe_hdr(skb);
	uint16_t ppp_proto = *(uint16_t *)ph->tag;
	uint32_t encap_header_len = 0;

	ppp_proto = ntohs(ppp_proto);
	if (ppp_proto != PPP_IP) {
		ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_BRIDGE_PROTO_NOT_PPPOE);
		return NF_ACCEPT;
	}

	encap_header_len = ecm_front_end_l2_encap_header_len(ntohs(skb->protocol));
	ecm_front_end_pull_l2_encap_header(skb, encap_header_len);
	skb->protocol = htons(ETH_P_IP);

	if (!ecm_tracker_ip_check_header_and_read(&ip_hdr, skb)) {
		DEBUG_WARN("Invalid ip header in skb %px\n", skb);
		ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_BRIDGE_PPPOE_MALFORMED_HEADER);
		goto skip_ipv4_process;
	}

	/*
	 * Return if destination IP address is multicast address.
	 */
	if (ecm_ip_addr_is_multicast(ip_hdr.dest_addr)) {
		DEBUG_WARN("Multicast acceleration is not support in PPPoE bridge %px\n", skb);
		ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_BRIDGE_PPPOE_MCAST_ACCEL_NOT_SUPPORTED);
		goto skip_ipv4_process;
	}

	result = ecm_ipv4_ip_process(out, in, skb_eth_hdr->h_source,
					 skb_eth_hdr->h_dest, can_accel,
					 false, true, skb, ETH_P_PPP_SES);
skip_ipv4_process:
	ecm_front_end_push_l2_encap_header(skb, encap_header_len);
	skb->protocol = htons(ETH_P_PPP_SES);

	return result;
}

/*
 * ecm_ipv4_vlan_bridge_process()
 *	called for double vlan packets that are going
 *	out to one of the bridge physical interfaces.
 */
static unsigned int ecm_ipv4_vlan_bridge_process(struct net_device *out,
						     struct net_device *in,
						     struct ethhdr *skb_eth_hdr,
						     bool can_accel,
						     struct sk_buff *skb)
{
	unsigned int result = NF_ACCEPT;
	struct vlan_ethhdr *skb_vlan_hdr = vlan_eth_hdr(skb);
	uint32_t encap_header_len = 0;

	encap_header_len = ecm_front_end_l2_encap_header_len(ntohs(skb->protocol));
	ecm_front_end_pull_l2_encap_header(skb, encap_header_len);
	skb->protocol = skb_vlan_hdr->h_vlan_encapsulated_proto;

	if (ntohs(skb->protocol) == ETH_P_PPP_SES) {

		/*
		 * Check if PPPoE bridge acceleration is disabled.
		 */
		if (ecm_front_end_ppppoe_br_accel_disabled()) {
			DEBUG_TRACE("skb: %px, PPPoE bridge flow acceleration is disabled\n", skb);
			goto skip_ipv4_vlan_process;
		}

		result = ecm_ipv4_pppoe_bridge_process((struct net_device *)out, in, skb_eth_hdr, can_accel, skb);
		goto skip_ipv4_vlan_process;
	}

	result = ecm_ipv4_ip_process(out, in, skb_eth_hdr->h_source,
					 skb_eth_hdr->h_dest, can_accel,
					 false, true, skb, ETH_P_8021Q);
skip_ipv4_vlan_process:
	ecm_front_end_push_l2_encap_header(skb, encap_header_len);
	skb->protocol = htons(ETH_P_8021Q);

	return result;
}

/*
 * ecm_ipv4_bridge_post_routing_hook()
 *	Called for packets that are going out to one of the bridge physical interfaces.
 *
 * These may have come from another bridged interface or from a non-bridged interface.
 * Conntrack information may be available or not if this skb is bridged.
 */
static unsigned int ecm_ipv4_bridge_post_routing_hook(void *priv,
					struct sk_buff *skb,
					const struct nf_hook_state *nhs)
{
	struct net_device *out = nhs->out;
	struct ethhdr *skb_eth_hdr;
	uint16_t eth_type;
	struct net_device *bridge;
	struct net_device *in;
	bool can_accel = true;
	unsigned int result = NF_ACCEPT;
	struct net_device *dest_dev __maybe_unused;
	uint16_t vid __maybe_unused;

	DEBUG_TRACE("%px: IPv4 CMN Bridge: %s skb=%px\n", out, out->name, skb);

	/*
	 * Skip flow with out interface marked for don't offload.
	 */
	if (out->priv_flags_ext & IFF_EXT_HW_NO_OFFLOAD) {
		ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_BRIDGE_OUT_IFF_NO_OFFLOAD);
		return NF_ACCEPT;
	}

	/*
	 * If operations have stopped then do not process packets
	 */
	spin_lock_bh(&ecm_ipv4_lock);
	if (unlikely(ecm_front_end_ipv4_stopped)) {
		spin_unlock_bh(&ecm_ipv4_lock);
		DEBUG_TRACE("Front end stopped\n");
		ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_BRIDGE_FRONT_END_STOPPED);
		return NF_ACCEPT;
	}
	spin_unlock_bh(&ecm_ipv4_lock);

	/*
	 * Don't process broadcast or multicast
	 */
	if (skb->pkt_type == PACKET_BROADCAST) {
		DEBUG_TRACE("Broadcast, ignoring: %px\n", skb);
		ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_BRIDGE_BCAST_PACKET_IGNORED);
		return NF_ACCEPT;
	}

	/*
	 * skip l2tp/pptp because we don't accelerate them
	 */
	if (ecm_interface_is_l2tp_pptp(skb, out)) {
		ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_BRIDGE_PPTP_ACCEL_FAIL);
		return NF_ACCEPT;
	}

	/*
	 * Check packet is an IP Ethernet packet
	 */
	skb_eth_hdr = eth_hdr(skb);
	if (!skb_eth_hdr) {
		DEBUG_TRACE("%px: Not Eth\n", skb);
		ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_BRIDGE_MALFORMED_IP_HEADER);
		return NF_ACCEPT;
	}
	eth_type = ntohs(skb_eth_hdr->h_proto);
	if (unlikely((eth_type != ETH_P_IP) && (eth_type != ETH_P_PPP_SES) && (eth_type != ETH_P_8021Q))) {
		DEBUG_TRACE("%px: Not IP/PPPoE/VLAN session: %d\n", skb, eth_type);
		ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_BRIDGE_NOT_IP_PPPOE_PACKETS);
		return NF_ACCEPT;
	}

	if (eth_type == ETH_P_8021Q) {
		struct vlan_ethhdr *skb_vlan_hdr;
		uint16_t vlan_encap_proto;
		skb_vlan_hdr = vlan_eth_hdr(skb);
		vlan_encap_proto = ntohs(skb_vlan_hdr->h_vlan_encapsulated_proto);
		if (unlikely((vlan_encap_proto != ETH_P_IP) && (vlan_encap_proto != ETH_P_PPP_SES))) {
			DEBUG_TRACE("%px: Not IP/PPPoE session: %d\n", skb, vlan_encap_proto);
			ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_BRIDGE_NOT_IP_PPPOE_PACKETS);
			return NF_ACCEPT;
		}
	}

	/*
	 * Identify interface from where this packet came.
	 * There are three scenarios to consider here:
	 * 1. Packet came from a local source.
	 *	Ignore - local is not handled.
	 * 2. Packet came from a routed path.
	 *	Ignore - it was handled in INET post routing.
	 * 3. Packet is bridged from another port.
	 *	Process.
	 *
	 * Begin by identifying case 1.
	 * NOTE: We are given 'out' (which we implicitly know is a bridge port) so out interface's master is the 'bridge'.
	 */
	bridge = ecm_interface_get_and_hold_dev_master((struct net_device *)out);
	if (!bridge) {
		DEBUG_WARN("Expected bridge\n");
		ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_BRIDGE_MASTER_NOT_FOUND);
		return NF_ACCEPT;
	}

	in = dev_get_by_index(&init_net, skb->skb_iif);
	if  (!in) {
		/*
		 * Case 1.
		 */
		DEBUG_TRACE("Local traffic: %px, ignoring traffic to bridge: %px (%s) \n", skb, bridge, bridge->name);
		dev_put(bridge);
		ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_BRIDGE_LOCAL_PACKETS_IGNORED);
		return NF_ACCEPT;
	}

	/*
	 * Skip flow with interface marked for don't offload.
	 */
	if (in->priv_flags_ext & IFF_EXT_HW_NO_OFFLOAD) {
		dev_put(bridge);
		dev_put(in);
		ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_BRIDGE_IN_IFF_NO_OFFLOAD);
		return NF_ACCEPT;
	}

	dev_put(in);

	/*
	 * Case 2:
	 *	For routed packets the skb will have the src mac matching the bridge mac.
	 * Case 3:
	 *	If the packet was not local (case 1) or routed (case 2) then
	 *	we process. There is an exception to case 2: when hairpin mode
	 *	is enabled, we process.
	 */

	/*
	 * Pass in NULL (for skb) and 0 for cookie since doing FDB lookup only
	 */
	in = br_port_dev_get(bridge, skb_eth_hdr->h_source, NULL, 0);
	if (!in) {
		DEBUG_TRACE("skb: %px, no in device for bridge: %px (%s)\n", skb, bridge, bridge->name);
		dev_put(bridge);
		ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_BRIDGE_PORT_NOT_FOUND);
		return NF_ACCEPT;
	}

	/*
	 * This flag needs to be checked in slave port(eth0/ath0)
	 * and not on master interface(br-lan). Hairpin flag can be
	 * enabled/disabled for ports individually.
	 */
	if (in == out) {
		if (!br_is_hairpin_enabled(in)) {
			DEBUG_TRACE("skb: %px, bridge: %px (%s), ignoring"
					"the packet, hairpin not enabled"
					"on port %px (%s)\n", skb, bridge,
					bridge->name, out, out->name);
			ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_BRIDGE_HAIRPIN_NOT_ENABLED);
			goto skip_ipv4_bridge_flow;
		}
		DEBUG_TRACE("skb: %px, bridge: %px (%s), hairpin enabled on port"
				"%px (%s)\n", skb, bridge, bridge->name, out, out->name);
	}

	/*
	 * Case 2: Routed trafffic would be handled by the INET post routing.
	 */
	if (!ecm_mac_addr_equal(skb_eth_hdr->h_source, bridge->dev_addr)) {
		DEBUG_TRACE("skb: %px, Ignoring routed packet to bridge: %px (%s)\n", skb, bridge, bridge->name);
		ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_ROUTED_PACKET_WRONG_HOOK);
		goto skip_ipv4_bridge_flow;
	}

	if (!is_multicast_ether_addr(skb_eth_hdr->h_dest)) {
		/*
		 * Process the packet, if we have this destination mac address in the fdb table.
		 */
#ifdef ECM_BRIDGE_VLAN_FILTERING_ENABLE
		/*
		 * TODO: Enhance br_fdb_has_entry() to make it VLAN filter aware.
		 */
		rcu_read_lock();
		dest_dev = br_fdb_find_vid_by_mac(bridge, skb_eth_hdr->h_dest, &vid);
		if (!dest_dev) {
			DEBUG_TRACE("skb: %px, br_fdb_find_vid_by_mac() returned NULL dest_dev for dest_mac(%pM) %px (%s) vid=%d\n",
					skb, skb_eth_hdr->h_dest, bridge, bridge->name, vid);
			rcu_read_unlock();
			ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_BRIDGE_DEST_MAC_NOT_FOUND);
			goto skip_ipv4_bridge_flow;
		}
		dev_put(dest_dev);
		rcu_read_unlock();
#else
		if (!br_fdb_has_entry((struct net_device *)out, skb_eth_hdr->h_dest, 0)) {
			DEBUG_WARN("skb: %px, No fdb entry for this mac address %pM in the bridge: %px (%s)\n",
					skb, skb_eth_hdr->h_dest, bridge, bridge->name);
			ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_BRIDGE_MAC_ENTRY_NOT_FOUND);
			goto skip_ipv4_bridge_flow;
		}
#endif
	}

	DEBUG_TRACE("CMN Bridge process skb: %px, bridge: %px (%s), In: %px (%s), Out: %px (%s)\n",
			skb, bridge, bridge->name, in, in->name, out, out->name);

	if (unlikely(eth_type == ETH_P_8021Q)) {
		result = ecm_ipv4_vlan_bridge_process((struct net_device *)out, in, skb_eth_hdr, can_accel, skb);
		goto skip_ipv4_bridge_flow;
	}

	if (unlikely(eth_type == ETH_P_PPP_SES)) {

		/*
		 * Check if PPPoE bridge acceleration is disabled.
		 */
		if (ecm_front_end_ppppoe_br_accel_disabled()) {
			DEBUG_TRACE("skb: %px, PPPoE bridge flow acceleration is disabled\n", skb);
			ecm_stats_v4_inc(ECM_STATS_V4_EXCEPTION_CMN, ECM_STATS_V4_EXCEPTION_BRIDGE_PPPOE_ACCEL_DISABLED);
			goto skip_ipv4_bridge_flow;
		}

		result = ecm_ipv4_pppoe_bridge_process((struct net_device *)out, in, skb_eth_hdr, can_accel, skb);
		goto skip_ipv4_bridge_flow;
	}
	result = ecm_ipv4_ip_process((struct net_device *)out, in,
				skb_eth_hdr->h_source, skb_eth_hdr->h_dest, can_accel, false, false, skb, 0);
skip_ipv4_bridge_flow:
	dev_put(in);
	dev_put(bridge);
	return result;
}

/*
 * struct nf_hook_ops ecm_ipv4_netfilter_routing_hooks[]
 *	Hooks into netfilter routing packet monitoring points.
 */
static struct nf_hook_ops ecm_ipv4_netfilter_routing_hooks[] __read_mostly = {
	/*
	 * Post routing hook is used to monitor packets going to interfaces that are NOT bridged in some way, e.g. packets to the WAN.
	 */
	{
		.hook           = ecm_ipv4_post_routing_hook,
		.pf             = PF_INET,
		.hooknum        = NF_INET_POST_ROUTING,
		.priority       = NF_IP_PRI_NAT_SRC + 1,
	},
};

/*
 * struct nf_hook_ops ecm_ipv4_netfilter_bridge_hooks[]
 *	Hooks into netfilter bridge packet monitoring points.
 */
static struct nf_hook_ops ecm_ipv4_netfilter_bridge_hooks[] __read_mostly = {
	/*
	 * The bridge post routing hook monitors packets going to interfaces that are part of a bridge arrangement.
	 * For example Wireles LAN (WLAN) and Wired LAN (LAN).
	 */
	{
		.hook		= ecm_ipv4_bridge_post_routing_hook,
		.pf		= PF_BRIDGE,
		.hooknum	= NF_BR_POST_ROUTING,
		.priority	= NF_BR_PRI_FILTER_OTHER,
	},
};

#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
/*
 * ecm_ipv4_ovs_dp_process()
 *      Process OVS IPv4 bridged packets.
 */
unsigned int ecm_ipv4_ovs_dp_process(struct sk_buff *skb, struct net_device *out)
{
	struct ethhdr *skb_eth_hdr;
	bool can_accel = true;
	struct net_device *in;

	/*
	 * If operations have stopped then do not process packets
	 */
	spin_lock_bh(&ecm_ipv4_lock);
	if (unlikely(ecm_front_end_ipv4_stopped)) {
		spin_unlock_bh(&ecm_ipv4_lock);
		DEBUG_TRACE("Front end stopped\n");
		return 1;
	}
	spin_unlock_bh(&ecm_ipv4_lock);

	/*
	 * Don't process broadcast.
	 */
	if (skb->pkt_type == PACKET_BROADCAST) {
		DEBUG_TRACE("Broadcast, ignoring: %px\n", skb);
		return 1;
	}

	/*
	 * Don't process multicast packet if frontend does not support it
	 */
	if (skb->pkt_type == PACKET_MULTICAST) {
		if (!ecm_front_end_is_feature_supported(ECM_FE_FEATURE_MULTICAST)) {
			DEBUG_TRACE("Frontend does not support mcast, ignoring: %px\n", skb);
			return 1;
		}
	}

	if (skb->protocol != ntohs(ETH_P_IP)) {
		DEBUG_WARN("%px: Wrong skb protocol: %d", skb, skb->protocol);
		return 1;
	}

	skb_eth_hdr = eth_hdr(skb);
	if (!skb_eth_hdr) {
		DEBUG_WARN("%px: Not Eth\n", skb);
		return 1;
	}

	in = dev_get_by_index(&init_net, skb->skb_iif);
	if (!in) {
		DEBUG_WARN("%px: No in device\n", skb);
		return 1;
	}

	DEBUG_TRACE("%px: in: %s out: %s skb->protocol: %x\n", skb, in->name, out->name, skb->protocol);

	if (netif_is_ovs_master(in)) {
		if (!ecm_mac_addr_equal(skb_eth_hdr->h_dest, in->dev_addr)) {
			DEBUG_TRACE("%px: in is bridge and mac address equals to packet dest, flow is routed, ignore \n", skb);
			dev_put(in);
			return 1;
		}
	}

	if (netif_is_ovs_master(out)) {
		if (!ecm_mac_addr_equal(skb_eth_hdr->h_source, out->dev_addr)) {
			DEBUG_TRACE("%px: out is bridge and mac address equals to packet source, flow is routed, ignore \n", skb);
			dev_put(in);
			return 1;
		}
	}

	ecm_ipv4_ip_process((struct net_device *)out, in,
			skb_eth_hdr->h_source, skb_eth_hdr->h_dest, can_accel, false, false, skb, ETH_P_IP);

	dev_put(in);

	return 0;
}

static struct ovsmgr_dp_hook_ops ecm_ipv4_dp_hooks = {
	.protocol = 4,
	.hook_num = OVSMGR_DP_HOOK_POST_FLOW_PROC,
	.hook = ecm_ipv4_ovs_dp_process,
};
#endif

/*
 * ecm_ipv4_init()
 */
int ecm_ipv4_init(struct dentry *dentry)
{
	int result;

	DEBUG_INFO("ECM CMN IPv4 init\n");

#ifdef ECM_FRONT_END_NSS_ENABLE
	result = ecm_nss_ipv4_init(dentry);
	if (result < 0) {
		DEBUG_ERROR("Can't initialize NSS ipv4\n");
		return result;
	}
#endif
#ifdef ECM_FRONT_END_PPE_ENABLE
	result = ecm_ppe_ipv4_init(dentry);
	if (result < 0) {
		DEBUG_ERROR("Can't initialize PPE ipv4\n");
		goto ppe_ipv4_failed;
	}
#endif
	result = ecm_sfe_ipv4_init(dentry);
	if (result < 0) {
		DEBUG_ERROR("Can't initialize SFE ipv4\n");
		goto sfe_ipv4_failed;
	}

	/*
	 * Register netfilter routing hooks
	 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 13, 0))
	result = nf_register_hooks(ecm_ipv4_netfilter_routing_hooks, ARRAY_SIZE(ecm_ipv4_netfilter_routing_hooks));
#else
	result = nf_register_net_hooks(&init_net, ecm_ipv4_netfilter_routing_hooks, ARRAY_SIZE(ecm_ipv4_netfilter_routing_hooks));
#endif
	if (result < 0) {
		DEBUG_ERROR("Can't register common netfilter routing hooks.\n");
		goto nf_register_failed_1;
	}

	/*
	 * Register netfilter bridge hooks, if the frontend type supports it. SFE only mode doesn't support it.
	 */
	if (ecm_front_end_is_feature_supported(ECM_FE_FEATURE_BRIDGE)) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 13, 0))
		result = nf_register_hooks(ecm_ipv4_netfilter_bridge_hooks, ARRAY_SIZE(ecm_ipv4_netfilter_bridge_hooks));
#else
		result = nf_register_net_hooks(&init_net, ecm_ipv4_netfilter_bridge_hooks, ARRAY_SIZE(ecm_ipv4_netfilter_bridge_hooks));
#endif
		if (result < 0) {
			DEBUG_ERROR("Can't register common netfilter bridge hooks.\n");
			goto nf_register_failed_2;
		}
	}

	/*
	 * Register OVS bridge DP hookup when AE frontend supports bridge processing
	 */
#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
	if (ecm_front_end_is_feature_supported(ECM_FE_FEATURE_OVS_BRIDGE)) {
		ovsmgr_dp_hook_register(&ecm_ipv4_dp_hooks);
	}
#endif

	return 0;

nf_register_failed_2:
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 13, 0))
	nf_unregister_hooks(ecm_ipv4_netfilter_routing_hooks, ARRAY_SIZE(ecm_ipv4_netfilter_routing_hooks));
#else
	nf_unregister_net_hooks(&init_net, ecm_ipv4_netfilter_routing_hooks, ARRAY_SIZE(ecm_ipv4_netfilter_routing_hooks));
#endif
nf_register_failed_1:
	ecm_sfe_ipv4_exit();

sfe_ipv4_failed:
#ifdef ECM_FRONT_END_PPE_ENABLE
	ecm_ppe_ipv4_exit();
ppe_ipv4_failed:
#endif

#ifdef ECM_FRONT_END_NSS_ENABLE
	ecm_nss_ipv4_exit();
#endif
	return result;
}

/*
 * ecm_ipv4_exit()
 */
void ecm_ipv4_exit(void)
{
	DEBUG_INFO("ECM CMN IPv4 Module exit\n");

	spin_lock_bh(&ecm_ipv4_lock);
	ecm_ipv4_terminate_pending = true;
	spin_unlock_bh(&ecm_ipv4_lock);

	/*
	 * Unregister the netfilter bridge hooks, if the frontend type supports it. SFE only mode doesn't support it.
	 */
	if (ecm_front_end_is_feature_supported(ECM_FE_FEATURE_BRIDGE)) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 13, 0))
		nf_unregister_hooks(ecm_ipv4_netfilter_bridge_hooks, ARRAY_SIZE(ecm_ipv4_netfilter_bridge_hooks));
#else
		nf_unregister_net_hooks(&init_net, ecm_ipv4_netfilter_bridge_hooks, ARRAY_SIZE(ecm_ipv4_netfilter_bridge_hooks));
#endif
	}

	/*
	 * Unregister the netfilter routing hooks.
	 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 13, 0))
	nf_unregister_hooks(ecm_ipv4_netfilter_routing_hooks, ARRAY_SIZE(ecm_ipv4_netfilter_routing_hooks));
#else
	nf_unregister_net_hooks(&init_net, ecm_ipv4_netfilter_routing_hooks, ARRAY_SIZE(ecm_ipv4_netfilter_routing_hooks));
#endif

	/*
	 * Unregister OVS bridge DP hook
	 */
#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
	if (ecm_front_end_is_feature_supported(ECM_FE_FEATURE_OVS_BRIDGE)) {
		ovsmgr_dp_hook_unregister(&ecm_ipv4_dp_hooks);
	}
#endif

	ecm_sfe_ipv4_exit();
#ifdef ECM_FRONT_END_PPE_ENABLE
	ecm_ppe_ipv4_exit();
#endif
#ifdef ECM_FRONT_END_NSS_ENABLE
	ecm_nss_ipv4_exit();
#endif
}
