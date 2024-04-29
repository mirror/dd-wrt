/*
 **************************************************************************
 * Copyright (c) 2014-2021 The Linux Foundation. All rights reserved.
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
#include <linux/pkt_sched.h>
#include <linux/string.h>
#include <net/ip6_route.h>
#include <net/ip6_fib.h>
#include <net/ipv6.h>
#include <net/route.h>
#include <net/ip_fib.h>
#include <net/ip.h>
#include <net/tcp.h>
#include <net/addrconf.h>
#include <asm/unaligned.h>
#include <asm/uaccess.h>	/* for put_user */
#include <linux/inet.h>
#include <linux/in6.h>
#include <linux/in.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <linux/kernel.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/socket.h>
#include <linux/wireless.h>
#include <net/gre.h>

#if defined(ECM_DB_XREF_ENABLE) && defined(ECM_BAND_STEERING_ENABLE)
#include <linux/if_bridge.h>
#endif
#include <linux/inetdevice.h>
#if defined(ECM_INTERFACE_TUNIPIP6_ENABLE) || defined(ECM_INTERFACE_SIT_ENABLE)
#include <net/ip_tunnels.h>
#endif
#include <net/ip6_tunnel.h>
#include <net/addrconf.h>
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
#include <linux/netfilter_ipv6/ip6_tables.h>
#include <net/netfilter/ipv4/nf_conntrack_ipv4.h>
#include <net/netfilter/ipv4/nf_defrag_ipv4.h>
#ifdef ECM_INTERFACE_VLAN_ENABLE
#include <linux/../../net/8021q/vlan.h>
#include <linux/if_vlan.h>
#endif
#ifdef ECM_INTERFACE_PPP_ENABLE
#include <linux/if_pppox.h>
#ifdef ECM_INTERFACE_L2TPV2_ENABLE
#include <linux/if_pppol2tp.h>
#endif
#ifdef ECM_INTERFACE_PPTP_ENABLE
#include <linux/netfilter/nf_conntrack_proto_gre.h>
#endif
#endif
#ifdef ECM_INTERFACE_MAP_T_ENABLE
#include <nat46-core.h>
#endif
#ifdef ECM_INTERFACE_VXLAN_ENABLE
#include <net/vxlan.h>
#endif
#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
#include <ovsmgr.h>
#endif
#ifdef ECM_INTERFACE_MACVLAN_ENABLE
#include <linux/if_macvlan.h>
#endif

/*
 * Debug output levels
 * 0 = OFF
 * 1 = ASSERTS / ERRORS
 * 2 = 1 + WARN
 * 3 = 2 + INFO
 * 4 = 3 + TRACE
 */
#define DEBUG_LEVEL ECM_INTERFACE_DEBUG_LEVEL

#ifdef ECM_MULTICAST_ENABLE
#include <mc_ecm.h>
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
#include "ecm_front_end_ipv4.h"
#ifdef ECM_IPV6_ENABLE
#include "ecm_front_end_ipv6.h"
#endif
#include "ecm_interface.h"
#include "exports/ecm_interface_ipsec.h"
#ifdef ECM_INTERFACE_OVPN_ENABLE
#include "ecm_interface_ovpn.h"
#endif
#include "ecm_front_end_common.h"

/*
 * Wifi event handler structure.
 */
struct ecm_interface_wifi_event {
	struct task_struct *thread;
	struct socket *sock;
};

static struct ecm_interface_wifi_event __ewn;

#ifdef ECM_INTERFACE_IPSEC_GLUE_LAYER_SUPPORT_ENABLE
/*
 * Get ipsecmgr tunnel netdevice method
 */
static struct ecm_interface_ipsec_callback ecm_interface_ipsec_cb;
#endif

/*
 * Locking - concurrency control
 */
static DEFINE_SPINLOCK(ecm_interface_lock);			/* Protect against SMP access between netfilter, events and private threaded function. */

/*
 * Management thread control
 */
static bool ecm_interface_terminate_pending = false;		/* True when the user has signalled we should quit */

/*
 * Source interface check flag.
 *	If it is enabled, the acceleration engine will check the flow's interface to see
 *	whether it matches with the rule's source interface or not.
 */
int ecm_interface_src_check;

#if 0 // defined(CONFIG_NET_CLS_ACT) && defined(ECM_CLASSIFIER_DSCP_IGS)
/*
 * IGS enabled flag.
 *	If it is enabled, the acceleration engine will deny the acceleration for the new
 *	connnection, if the egress interface has ingress qdisc enabled over it.
 */
int ecm_interface_igs_enabled;
#endif

static struct ctl_table_header *ecm_interface_ctl_table_header;	/* Sysctl table header */

#ifdef ECM_INTERFACE_OVPN_ENABLE
/*
 * Callback structure to support OVPN offload.
 */
static struct ecm_interface_ovpn ovpn;

/*
 * ecm_interface_ovpn_register
 */
int ecm_interface_ovpn_register(struct ecm_interface_ovpn *ovpn_cb)
{
	spin_lock_bh(&ecm_interface_lock);
	if (ovpn.ovpn_update_route) {
		spin_unlock_bh(&ecm_interface_lock);
		DEBUG_ERROR("OVPN callbacks are registered\n");
		return -1;
	}

	ovpn.ovpn_update_route = ovpn_cb->ovpn_update_route;
	ovpn.ovpn_get_ifnum = ovpn_cb->ovpn_get_ifnum;
	spin_unlock_bh(&ecm_interface_lock);

	return 0;
}
EXPORT_SYMBOL(ecm_interface_ovpn_register);

/*
 * ecm_interface_ovpn_unregister
 */
void ecm_interface_ovpn_unregister(void)
{
	spin_lock_bh(&ecm_interface_lock);
	ovpn.ovpn_update_route = NULL;
	ovpn.ovpn_get_ifnum = NULL;
	spin_unlock_bh(&ecm_interface_lock);
}
EXPORT_SYMBOL(ecm_interface_ovpn_unregister);

/*
 * ecm_interface_ovpn_get_ifnum
 */
static int ecm_interface_ovpn_get_ifnum(struct net_device *dev, struct sk_buff *skb, struct net_device **tun_dev)
{
	int ret = -1;

	DEBUG_TRACE("Calling registered function to get OVPN ifnum.\n");

	spin_lock_bh(&ecm_interface_lock);
	if (likely(ovpn.ovpn_get_ifnum)) {
		ret = ovpn.ovpn_get_ifnum(dev, skb, tun_dev);
	}
	spin_unlock_bh(&ecm_interface_lock);

	return ret;
}

/*
 * ecm_interface_ovpn_update_route
 */
static void ecm_interface_ovpn_update_route(struct net_device *dev, uint32_t *from_addr, uint32_t *to_addr, int version)
{
	DEBUG_TRACE("Calling registered function to update OVPN route entry.\n");

	spin_lock_bh(&ecm_interface_lock);
	if (likely(ovpn.ovpn_update_route)) {
		ovpn.ovpn_update_route(dev, from_addr, to_addr, version);
	}
	spin_unlock_bh(&ecm_interface_lock);
}
#endif

/*
 * ecm_interface_get_and_hold_ipsec_tun_netdev()
 *	Returns the nss tunnel interface net_dev
 */
struct net_device *ecm_interface_get_and_hold_ipsec_tun_netdev(struct net_device *dev, struct sk_buff *skb, int32_t *interface_type)
{
	struct net_device *ipsec_dev = NULL;
#ifdef ECM_INTERFACE_IPSEC_GLUE_LAYER_SUPPORT_ENABLE
	spin_lock_bh(&ecm_interface_lock);
	if (!ecm_interface_ipsec_cb.tunnel_get_and_hold) {
		spin_unlock_bh(&ecm_interface_lock);
		DEBUG_WARN("IPSec glue module is not loaded yet\n");
		return NULL;
	}

	ipsec_dev = ecm_interface_ipsec_cb.tunnel_get_and_hold(dev, skb, interface_type);
	spin_unlock_bh(&ecm_interface_lock);
#endif
	return ipsec_dev;
}

/*
 * ecm_interface_get_and_hold_dev_master()
 *	Returns the master device of a net device if any.
 */
struct net_device *ecm_interface_get_and_hold_dev_master(struct net_device *dev)
{
	struct net_device *master;

#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
	if (ecm_interface_is_ovs_bridge_port(dev)) {
		master = ovsmgr_dev_get_master(dev);
		if (!master) {
			return NULL;
		}

		dev_hold(master);
		return master;
	}
#endif
	rcu_read_lock();
	master = netdev_master_upper_dev_get_rcu(dev);
	if (!master) {
		rcu_read_unlock();
		return NULL;
	}
	dev_hold(master);
	rcu_read_unlock();

	return master;
}
EXPORT_SYMBOL(ecm_interface_get_and_hold_dev_master);

/*
 * ecm_interface_vlan_real_dev()
 *	Return immediate VLAN net device or Physical device pointer
 */
static inline struct net_device *ecm_interface_vlan_real_dev(struct net_device *vlan_dev)
{
#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 11, 0))
	struct vlan_dev_priv *vlan = vlan_dev_priv(vlan_dev);
	return vlan->real_dev;
#else
	return vlan_dev_next_dev(vlan_dev);
#endif
}

#ifdef ECM_INTERFACE_VLAN_ENABLE
#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 11, 0))
static void ecm_interface_vlan_dev_update_accel_stats(struct net_device *dev,
						      struct rtnl_link_stats64 *nlstats)
{
	struct vlan_dev_priv *vlan = vlan_dev_priv(dev);
	struct vlan_pcpu_stats *stats;

	if (!is_vlan_dev(dev))
		return;

	stats = this_cpu_ptr(vlan->vlan_pcpu_stats);

	u64_stats_update_begin(&stats->syncp);
	u64_stats_add(&stats->rx_packets, nlstats->rx_packets);
	u64_stats_add(&stats->rx_bytes, nlstats->rx_bytes);
	u64_stats_add(&stats->tx_packets, nlstats->tx_packets);
	u64_stats_add(&stats->tx_bytes, nlstats->tx_bytes);
	u64_stats_update_end(&stats->syncp);
}
#endif
#endif

/*
 * ecm_interface_dev_find_by_local_addr_ipv4()
 *	Return a hold to the device for the given local IP address. Returns NULL on failure.
 */
static struct net_device *ecm_interface_dev_find_by_local_addr_ipv4(ip_addr_t addr)
{
	__be32 be_addr;
	struct net_device *dev;

	ECM_IP_ADDR_TO_NIN4_ADDR(be_addr, addr);
	dev = ip_dev_find(&init_net, be_addr);
	return dev;
}

#ifdef ECM_IPV6_ENABLE
/*
 * ecm_interface_dev_find_by_local_addr_ipv6()
 *	Return a hold to the device for the given local IP address.  Returns NULL on failure.
 */
static struct net_device *ecm_interface_dev_find_by_local_addr_ipv6(ip_addr_t addr)
{
	struct in6_addr addr6;
	struct net_device *dev;

	ECM_IP_ADDR_TO_NIN6_ADDR(addr6, addr);
	dev = (struct net_device *)ecm_interface_ipv6_dev_find_and_hold(&init_net, &addr6, 1);
	return dev;
}
#endif

/*
 * ecm_interface_dev_find_by_local_addr()
 *	Return the device on which the local address resides.
 *
 * Returns a hold to the device or NULL on failure.
 */
struct net_device *ecm_interface_dev_find_by_local_addr(ip_addr_t addr)
{
	if (ECM_IP_ADDR_IS_V4(addr)) {
		DEBUG_TRACE("Locate dev for " ECM_IP_ADDR_DOT_FMT "\n", ECM_IP_ADDR_TO_DOT(addr));
		return ecm_interface_dev_find_by_local_addr_ipv4(addr);
	}

	DEBUG_TRACE("Locate dev for " ECM_IP_ADDR_OCTAL_FMT "\n", ECM_IP_ADDR_TO_OCTAL(addr));
#ifdef ECM_IPV6_ENABLE
	return ecm_interface_dev_find_by_local_addr_ipv6(addr);
#else
	return NULL;
#endif
}
EXPORT_SYMBOL(ecm_interface_dev_find_by_local_addr);

/*
 * ecm_interface_dev_find_by_addr()
 *	Return the net device on which the given IP address resides.  Returns NULL on faiure.
 *
 * NOTE: The device may be the device upon which has a default gateway to reach the address.
 * from_local_addr is true when the device was found by a local address search.
 */
struct net_device *ecm_interface_dev_find_by_addr(ip_addr_t addr, bool *from_local_addr)
{
	struct ecm_interface_route ecm_rt;
	struct net_device *dev;
	struct dst_entry *dst;

	if (ECM_IP_ADDR_IS_V4(addr)) {
		DEBUG_TRACE("find net device for address: " ECM_IP_ADDR_DOT_FMT "\n", ECM_IP_ADDR_TO_DOT(addr));
	} else {
		DEBUG_TRACE("find net device for address: " ECM_IP_ADDR_OCTAL_FMT "\n", ECM_IP_ADDR_TO_OCTAL(addr));
	}

	/*
	 * Is the address a local IP?
	 */
	dev = ecm_interface_dev_find_by_local_addr(addr);
	if (dev) {
		 *from_local_addr = true;
		DEBUG_TRACE("address is local: %px (%s)\n", dev, dev->name);
		return dev;
	}

	DEBUG_TRACE("address is not local\n");

	/*
	 * Try a route to the address instead
	 * NOTE: This will locate a route entry in the route destination *cache*.
	 */
	if (!ecm_interface_find_route_by_addr(addr, &ecm_rt)) {
		DEBUG_WARN("no route found\n");
		return NULL;
	}

	*from_local_addr = false;
	dst = ecm_rt.dst;
	dev = dst->dev;
	dev_hold(dev);
	ecm_interface_route_release(&ecm_rt);
	DEBUG_TRACE("address uses dev: %px(%s)\n", dev, dev->name);
	return dev;
}
EXPORT_SYMBOL(ecm_interface_dev_find_by_addr);

#ifdef ECM_IPV6_ENABLE
/*
 * ecm_interface_mac_addr_get_ipv6()
 *	Return mac for an IPv6 address
 *
 * GGG TODO Need to make sure this also works for local IP addresses too.
 */
static bool ecm_interface_mac_addr_get_ipv6(ip_addr_t addr, uint8_t *mac_addr, bool *on_link, ip_addr_t gw_addr)
{
	struct in6_addr daddr;
	struct ecm_interface_route ecm_rt;
	struct neighbour *neigh;
	struct rt6_info *rt;
	struct dst_entry *dst;

	/*
	 * Get the MAC address that corresponds to IP address given.
	 * We look up the rt6_info entries and, from its neighbour structure, obtain the hardware address.
	 * This means we will also work if the neighbours are routers too.
	 */
	ECM_IP_ADDR_TO_NIN6_ADDR(daddr, addr);
	if (!ecm_interface_find_route_by_addr(addr, &ecm_rt)) {
		*on_link = false;
		return false;
	}
	DEBUG_ASSERT(!ecm_rt.v4_route, "Did not locate a v6 route!\n");

	/*
	 * Is this destination on link or off-link via a gateway?
	 */
	rt = ecm_rt.rt.rtv6;
	if (!ECM_IP_ADDR_MATCH(rt->rt6i_dst.addr.in6_u.u6_addr32, rt->rt6i_gateway.in6_u.u6_addr32) || (rt->rt6i_flags & RTF_GATEWAY)) {
		*on_link = false;
		ECM_NIN6_ADDR_TO_IP_ADDR(gw_addr, rt->rt6i_gateway)
	} else {
		*on_link = true;
	}

	rcu_read_lock();
	dst = ecm_rt.dst;

	neigh = dst_neigh_lookup(dst, &daddr);
	if (!neigh) {
		neigh = neigh_lookup(&nd_tbl, &daddr, dst->dev);
	}

	if (!neigh) {
		rcu_read_unlock();
		ecm_interface_route_release(&ecm_rt);
		DEBUG_WARN("No neigh reference\n");
		return false;
	}
	if (!(neigh->nud_state & NUD_VALID)) {
		rcu_read_unlock();
		neigh_release(neigh);
		ecm_interface_route_release(&ecm_rt);
		DEBUG_WARN("NUD invalid\n");
		return false;
	}
	if (!neigh->dev) {
		rcu_read_unlock();
		neigh_release(neigh);
		ecm_interface_route_release(&ecm_rt);
		DEBUG_WARN("Neigh dev invalid\n");
		return false;
	}

	/*
	 * If neigh->dev is a loopback then addr is a local address in which case we take the MAC from given device
	 */
	if (neigh->dev->flags & IFF_LOOPBACK) {
		// GGG TODO Create an equivalent logic to that for ipv4, maybe need to create an ip6_dev_find()?
		DEBUG_TRACE("local address " ECM_IP_ADDR_OCTAL_FMT " (found loopback)\n", ECM_IP_ADDR_TO_OCTAL(addr));
		eth_zero_addr(mac_addr);
	} else {
		ether_addr_copy(mac_addr, neigh->ha);
	}
	rcu_read_unlock();
	neigh_release(neigh);
	ecm_interface_route_release(&ecm_rt);

	DEBUG_TRACE(ECM_IP_ADDR_OCTAL_FMT " maps to %pM\n", ECM_IP_ADDR_TO_OCTAL(addr), mac_addr);
	return true;
}

/*
 * ecm_interface_find_gateway_ipv6()
 *	Finds the ipv6 gateway ip address of a given ipv6 address.
 */
static bool ecm_interface_find_gateway_ipv6(ip_addr_t addr, ip_addr_t gw_addr)
{
	struct ecm_interface_route ecm_rt;
	struct rt6_info *rt;

	/*
	 * Find the ipv6 route of the given ip address to look up
	 * whether we have a gateway to reach to that ip address or not.
	 */
	if (!ecm_interface_find_route_by_addr(addr, &ecm_rt)) {
		return false;
	}
	DEBUG_ASSERT(!ecm_rt.v4_route, "Did not locate a v6 route!\n");
	DEBUG_TRACE("Found route\n");

	/*
	 * Is this destination reachable via a gateway?
	 */
	rt = ecm_rt.rt.rtv6;
	if (ECM_IP_ADDR_MATCH(rt->rt6i_dst.addr.in6_u.u6_addr32, rt->rt6i_gateway.in6_u.u6_addr32) && !(rt->rt6i_flags & RTF_GATEWAY)) {
		ecm_interface_route_release(&ecm_rt);
		return false;
	}

	ECM_NIN6_ADDR_TO_IP_ADDR(gw_addr, rt->rt6i_gateway)
	ecm_interface_route_release(&ecm_rt);
	return true;
}
#endif

/*
 * ecm_interface_find_gateway_ipv4()
 *	Finds the ipv4 gateway address of a given ipv4 address.
 */
static bool ecm_interface_find_gateway_ipv4(ip_addr_t addr, ip_addr_t gw_addr)
{
	struct ecm_interface_route ecm_rt;
	struct rtable *rt;

	/*
	 * Find the ipv4 route of the given ip address to look up
	 * whether we have a gateway to reach to that ip address or not.
	 */
	if (!ecm_interface_find_route_by_addr(addr, &ecm_rt)) {
		return false;
	}
	DEBUG_ASSERT(ecm_rt.v4_route, "Did not locate a v4 route!\n");
	DEBUG_TRACE("Found route\n");

	/*
	 * Is this destination reachable via a gateway?
	 */
	rt = ecm_rt.rt.rtv4;
	if (!rt->rt_uses_gateway && !(rt->rt_flags & RTF_GATEWAY)) {
		ecm_interface_route_release(&ecm_rt);
		return false;
	}
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 2, 0))
	ECM_NIN4_ADDR_TO_IP_ADDR(gw_addr, rt->rt_gateway)
#else
	ECM_NIN4_ADDR_TO_IP_ADDR(gw_addr, rt->rt_gw4)
#endif
	ecm_interface_route_release(&ecm_rt);
	return true;
}

/*
 * ecm_interface_find_gateway()
 *	Finds the gateway ip address of a given ECM ip address type.
 */
bool ecm_interface_find_gateway(ip_addr_t addr, ip_addr_t gw_addr)
{
	if (ECM_IP_ADDR_IS_V4(addr)) {
		return ecm_interface_find_gateway_ipv4(addr, gw_addr);
	}

#ifdef ECM_IPV6_ENABLE
	return ecm_interface_find_gateway_ipv6(addr, gw_addr);
#else
	return false;
#endif
}
EXPORT_SYMBOL(ecm_interface_find_gateway);

/*
 * ecm_interface_mac_addr_get_ipv4()
 *	Return mac for an IPv4 address
 */
static bool ecm_interface_mac_addr_get_ipv4(ip_addr_t addr, uint8_t *mac_addr, bool *on_link, ip_addr_t gw_addr)
{
	struct neighbour *neigh;
	struct ecm_interface_route ecm_rt;
	struct rtable *rt;
	struct dst_entry *dst;
	__be32 ipv4_addr;

	/*
	 * Get the MAC address that corresponds to IP address given.
	 * We look up the rtable entries and, from its neighbour structure, obtain the hardware address.
	 * This means we will also work if the neighbours are routers too.
	 * We also locate the MAC if the address is a local host address.
	 */
	ECM_IP_ADDR_TO_NIN4_ADDR(ipv4_addr, addr);
	if (!ecm_interface_find_route_by_addr(addr, &ecm_rt)) {
		*on_link = false;
		return false;
	}
	DEBUG_ASSERT(ecm_rt.v4_route, "Did not locate a v4 route!\n");
	DEBUG_TRACE("Found route\n");

	/*
	 * Is this destination on link or off-link via a gateway?
	 */
	rt = ecm_rt.rt.rtv4;
	if (rt->rt_uses_gateway || (rt->rt_flags & RTF_GATEWAY)) {
		*on_link = false;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 2, 0))
		ECM_NIN4_ADDR_TO_IP_ADDR(gw_addr, rt->rt_gateway)
#else
		ECM_NIN4_ADDR_TO_IP_ADDR(gw_addr, rt->rt_gw4)
#endif
	} else {
		*on_link = true;
	}

	/*
	 * Get the neighbour entry for the address
	 */
	rcu_read_lock();
	dst = ecm_rt.dst;

	neigh = dst_neigh_lookup(dst, &ipv4_addr);
	if (!neigh) {
		neigh = neigh_lookup(&arp_tbl, &ipv4_addr, dst->dev);
	}
	if (!neigh) {
		rcu_read_unlock();
		ecm_interface_route_release(&ecm_rt);
		DEBUG_WARN("no neigh\n");
		return false;
	}
	if (!(neigh->nud_state & NUD_VALID)) {
		rcu_read_unlock();
		neigh_release(neigh);
		ecm_interface_route_release(&ecm_rt);
		DEBUG_WARN("neigh nud state is not valid\n");
		return false;
	}
	if (!neigh->dev) {
		rcu_read_unlock();
		neigh_release(neigh);
		ecm_interface_route_release(&ecm_rt);
		DEBUG_WARN("neigh has no device\n");
		return false;
	}

	/*
	 * If the device is loopback this will be because the address is a local address
	 * In this case locate the device that has this local address and get its mac.
	 */
	if (neigh->dev->type == ARPHRD_LOOPBACK) {
		struct net_device *dev;

		DEBUG_TRACE("%pI4 finds loopback device, dev: %px (%s)\n", &ipv4_addr, neigh->dev, neigh->dev->name);
		rcu_read_unlock();
		neigh_release(neigh);
		ecm_interface_route_release(&ecm_rt);

		/*
		 * Lookup the device that has this IP address assigned
		 */
		dev = ip_dev_find(&init_net, ipv4_addr);
		if (!dev) {
			DEBUG_WARN("Unable to locate dev for: %pI4\n", &ipv4_addr);
			return false;
		}
		memcpy(mac_addr, dev->dev_addr, (size_t)dev->addr_len);
		DEBUG_TRACE("is local addr: %pI4, mac: %pM, dev ifindex: %d, dev: %px (%s), dev_type: %d\n",
				&ipv4_addr, mac_addr, dev->ifindex, dev, dev->name, dev->type);
		dev_put(dev);
		return true;
	}

	if (!(neigh->dev->flags & IFF_NOARP)) {
		ether_addr_copy(mac_addr, neigh->ha);
	} else {
		DEBUG_TRACE("non-arp device: %px (%s, type: %d) to reach %pI4\n", neigh->dev, neigh->dev->name, neigh->dev->type, &ipv4_addr);
		eth_zero_addr(mac_addr);
	}
	DEBUG_TRACE("addr: %pI4, mac: %pM, iif: %d, neigh dev ifindex: %d, dev: %px (%s), dev_type: %d\n",
			&ipv4_addr, mac_addr, rt->rt_iif, neigh->dev->ifindex, neigh->dev, neigh->dev->name, neigh->dev->type);

	rcu_read_unlock();
	neigh_release(neigh);
	ecm_interface_route_release(&ecm_rt);
	return true;
}

/*
 * ecm_interface_mac_addr_get()
 *	Return the mac address for the given IP address.  Returns false on failure.
 */
bool ecm_interface_mac_addr_get(ip_addr_t addr, uint8_t *mac_addr, bool *on_link, ip_addr_t gw_addr)
{
	if (ECM_IP_ADDR_IS_V4(addr)) {
		return ecm_interface_mac_addr_get_ipv4(addr, mac_addr, on_link, gw_addr);
	}

#ifdef ECM_IPV6_ENABLE
	return ecm_interface_mac_addr_get_ipv6(addr, mac_addr, on_link, gw_addr);
#else
	return false;
#endif
}
EXPORT_SYMBOL(ecm_interface_mac_addr_get);

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 11, 0))
static void ecm_interface_br_dev_update_stats(struct net_device *dev,
					      struct rtnl_link_stats64 *nlstats)
{
	struct pcpu_sw_netstats *tstats;

	/* Is this a bridge? */
	if (!(dev->priv_flags & IFF_EBRIDGE))
		return;

	tstats = this_cpu_ptr(dev->tstats);

	u64_stats_update_begin(&tstats->syncp);
	u64_stats_add(&tstats->rx_packets, nlstats->rx_packets);
	u64_stats_add(&tstats->rx_bytes, nlstats->rx_bytes);
	u64_stats_add(&tstats->tx_packets, nlstats->tx_packets);
	u64_stats_add(&tstats->tx_bytes, nlstats->tx_bytes);
	u64_stats_update_end(&tstats->syncp);
}
#endif

#ifdef ECM_IPV6_ENABLE
/* ecm_interface_ipv6_dev_find_and_hold()
 *	Find (and hold) net device that has the given address.
 *	Or NULL on failure.
 */
struct net_device *ecm_interface_ipv6_dev_find_and_hold(struct net *net, struct in6_addr *addr,
							int strict)
{
	struct inet6_ifaddr *ifp;
	struct net_device *dev;

	ifp = ipv6_get_ifaddr(net, addr, NULL, strict);
	if (!ifp)
		return NULL;

	if (!ifp->idev) {
		in6_ifa_put(ifp);
		return NULL;
	}

	dev = ifp->idev->dev;
	if (dev)
		dev_hold(dev);

	in6_ifa_put(ifp);

	return dev;
}

/*
 * ecm_interface_mac_addr_get_ipv6_no_route()
 *	Finds the mac address of a node from its ip address reachable via
 * the given device. It looks up the mac address in the neighbour entries.
 * It doesn't do any route lookup to find the dst entry.
 */
static bool ecm_interface_mac_addr_get_ipv6_no_route(struct net_device *dev, ip_addr_t addr, uint8_t *mac_addr)
{
	struct in6_addr daddr;
	struct neighbour *neigh;
	struct net_device *local_dev;

	memset(mac_addr, 0, ETH_ALEN);

	/*
	 * Get the MAC address that corresponds to IP address given.
	 */
	ECM_IP_ADDR_TO_NIN6_ADDR(daddr, addr);
	local_dev = ecm_interface_ipv6_dev_find_and_hold(&init_net, &daddr, 1);
	if (local_dev) {
		DEBUG_TRACE("%pi6 is a local address\n", &daddr);
		memcpy(mac_addr, dev->dev_addr, ETH_ALEN);
		dev_put(local_dev);
		return true;
	}

	rcu_read_lock();
	neigh = neigh_lookup(&nd_tbl, &daddr, dev);
	if (!neigh) {
		rcu_read_unlock();
		DEBUG_WARN("No neigh reference\n");
		return false;
	}
	if (!(neigh->nud_state & NUD_VALID)) {
		neigh_release(neigh);
		rcu_read_unlock();
		DEBUG_WARN("NUD invalid\n");
		return false;
	}
	if (!neigh->dev) {
		neigh_release(neigh);
		rcu_read_unlock();
		DEBUG_WARN("Neigh dev invalid\n");
		return false;
	}

	if (neigh->dev->flags & IFF_NOARP) {
		neigh_release(neigh);
		rcu_read_unlock();
		DEBUG_TRACE("dest MAC is zero: %pM\n", mac_addr);
		return true;
	}

	memcpy(mac_addr, neigh->ha, (size_t)neigh->dev->addr_len);
	neigh_release(neigh);
	rcu_read_unlock();
	DEBUG_TRACE(ECM_IP_ADDR_OCTAL_FMT " maps to %pM\n", ECM_IP_ADDR_TO_OCTAL(addr), mac_addr);
	return true;
}
#endif

/*
 * ecm_interface_mac_addr_get_ipv4_no_route()
 *	Finds the mac address of a node from its ip address reachable via
 * the given device. It looks up the mac address in the neighbour entries.
 * It doesn't do any route lookup to find the dst entry.
 */
static bool ecm_interface_mac_addr_get_ipv4_no_route(struct net_device *dev, ip_addr_t ip_addr, uint8_t *mac_addr)
{
	struct neighbour *neigh;
	__be32 be_addr;
	struct net_device *local_dev;

	memset(mac_addr, 0, ETH_ALEN);

	ECM_IP_ADDR_TO_NIN4_ADDR(be_addr, ip_addr);
	local_dev = ip_dev_find(&init_net, be_addr);
	if (local_dev) {
		DEBUG_TRACE("%pI4n is a local address\n", &be_addr);
		memcpy(mac_addr, dev->dev_addr, ETH_ALEN);
		dev_put(local_dev);
		return true;
	}

	rcu_read_lock();
	neigh = neigh_lookup(&arp_tbl, &be_addr, dev);
	if (!neigh) {
		rcu_read_unlock();
		DEBUG_WARN("no neigh\n");
		return false;
	}
	if (!(neigh->nud_state & NUD_VALID)) {
		neigh_release(neigh);
		rcu_read_unlock();
		DEBUG_WARN("neigh nud state is not valid\n");
		return false;
	}
	if (!neigh->dev) {
		neigh_release(neigh);
		rcu_read_unlock();
		DEBUG_WARN("neigh has no device\n");
		return false;
	}

	if (neigh->dev->flags & IFF_NOARP) {
		neigh_release(neigh);
		rcu_read_unlock();
		DEBUG_TRACE("dest MAC is zero: %pM\n", mac_addr);
		return true;
	}

	memcpy(mac_addr, neigh->ha, (size_t)neigh->dev->addr_len);
	neigh_release(neigh);
	rcu_read_unlock();
	DEBUG_TRACE("dest MAC: %pM\n", mac_addr);
	return true;

}

/*
 * ecm_interface_mac_addr_get_no_route()
 *	Return the mac address for the given IP address reacahble via the given device.
 * Return false on failure, true on success.
 */
bool ecm_interface_mac_addr_get_no_route(struct net_device *dev, ip_addr_t addr, uint8_t *mac_addr)
{
	if (ECM_IP_ADDR_IS_V4(addr)) {
		return ecm_interface_mac_addr_get_ipv4_no_route(dev, addr, mac_addr);
	}

#ifdef ECM_IPV6_ENABLE
	return ecm_interface_mac_addr_get_ipv6_no_route(dev, addr, mac_addr);
#else
	return false;
#endif
}
EXPORT_SYMBOL(ecm_interface_mac_addr_get_no_route);

#ifdef ECM_MULTICAST_ENABLE

/*
 * ecm_interface_multicast_dest_list_find_if()
 *	Searches for a given device in a list of interface indices
 *
 *	dev		Pointer to the net device to search for
 *	max_if		Number of valid interfaces in the destination interface list
 *	dest_if_list	The destination interface list
 */
static bool ecm_interface_multicast_dest_list_find_if(struct net_device *dev, uint8_t max_if, uint32_t *dest_if_list)
{
	struct net_device *dest_dev = NULL;
	uint32_t *dst_if_index;
	int index;

	for (index = 0; index < max_if; index++) {
		dst_if_index = ecm_db_multicast_if_first_get_at_index(dest_if_list, index);
		dest_dev = dev_get_by_index(&init_net, *dst_if_index);
		if (!dest_dev) {
			DEBUG_WARN("Found invalid if_index %d at index %d\n", *dst_if_index, index);
			continue;
		}

		if (dest_dev == dev) {
			dev_put(dest_dev);
			return true;
		}

		dev_put(dest_dev);
	}

	return false;
}

/*
 * ecm_interface_multicast_check_for_br_dev()
 *	Find a bridge dev is present or not in an
 *	array of Ifindexs
 */
bool ecm_interface_multicast_check_for_br_dev(uint32_t dest_if[], uint8_t max_if)
{
	struct net_device *br_dev;
	int i;

	for (i = 0; i < max_if; i++) {
		br_dev = dev_get_by_index(&init_net, dest_if[i]);
		if (!br_dev) {
			/*
			 * Interface got deleted; but is yet to be updated in MFC table
			 */
			DEBUG_WARN("Could not find a valid netdev here\n");
			continue;
		}

		if (ecm_front_end_is_bridge_device(br_dev)
#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
			|| ecm_front_end_is_ovs_bridge_device(br_dev)
#endif
				) {
			dev_put(br_dev);
			return true;
		}
		dev_put(br_dev);
	}
	return false;
}
EXPORT_SYMBOL(ecm_interface_multicast_check_for_br_dev);

#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
/*
 * ecm_interface_multicast_check_for_ovs_br_dev()
 *	Check if OVS bridge dev exists in given list of interfaces.
 */
bool ecm_interface_multicast_check_for_ovs_br_dev(uint32_t dest_if[], uint8_t max_if)
{
	int i;

	for (i = 0; i < max_if; i++) {
		struct net_device *br_dev;

		br_dev = dev_get_by_index(&init_net, dest_if[i]);
		if (!br_dev) {
			/*
			 * Interface got deleted; but is yet to be updated in MFC table
			 */
			DEBUG_WARN("Could not find a valid netdev for interface: %d\n", dest_if[i]);
			continue;
		}

		if (ecm_front_end_is_ovs_bridge_device(br_dev)) {
			dev_put(br_dev);
			return true;
		}
		dev_put(br_dev);
	}
	return false;
}
#endif

/*
 * ecm_interface_multicast_is_iface_type()
 *	Checks if interface of type exist in mc_if_index
 */
bool ecm_interface_multicast_is_iface_type(int32_t mc_if_index[], int32_t max_if_index, unsigned short type)
{
	int32_t i;
	struct net_device *dev;

	for (i = 0; i < max_if_index; i++) {

		if (!mc_if_index[i]) {
			break;
		}

		dev = dev_get_by_index(&init_net, mc_if_index[i]);
		if (!dev) {
			DEBUG_WARN("Could not find a valid interface with index = %d\n", mc_if_index[i]);
			continue;
		}

		if (dev->type == type) {
			DEBUG_TRACE("Interface dev = %s of type %u\n", dev->name,  type);
			dev_put(dev);
			return true;
		}

		dev_put(dev);
	}

	return false;
}

/*
 * ecm_interface_multicast_filter_src_interface()
 *	Filter the source interface from the list.
 */
int32_t ecm_interface_multicast_filter_src_interface(struct ecm_db_connection_instance *ci, uint32_t *mc_dst_if_index)
{
	struct ecm_db_iface_instance *ii;
	struct ecm_db_iface_instance *from_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX];
	ecm_db_iface_type_t ii_type;
	int32_t from_ifaces_first;
	int32_t from_iface_identifier;
	int32_t if_index = ECM_DB_IFACE_HEIRARCHY_MAX;

	/*
	 * Get the interface lists of the connection, we must have at least one interface in the list to continue
	 */
	from_ifaces_first = ecm_db_connection_interfaces_get_and_ref(ci, from_ifaces, ECM_DB_OBJ_DIR_FROM);
	if (from_ifaces_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
		return if_index;
	}

	ii = from_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX - 1];
	ii_type = ecm_db_iface_type_get(ii);
	if ((ii_type == ECM_DB_IFACE_TYPE_BRIDGE) || (ii_type == ECM_DB_IFACE_TYPE_OVS_BRIDGE)) {
		ii = from_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX - 2];
	}

	from_iface_identifier = ecm_db_iface_interface_identifier_get(ii);
	if_index = ecm_interface_multicast_check_for_src_ifindex(mc_dst_if_index, ECM_DB_IFACE_HEIRARCHY_MAX, from_iface_identifier);
	ecm_db_connection_interfaces_deref(from_ifaces, from_ifaces_first);
	return if_index;
}

/*
 * ecm_interface_multicast_check_for_src_if_index()
 *	Find if a source netdev ifindex is matching with list of
 *	multicast destination netdev ifindex. If find a match then
 *	returns a new list of destination netdev ifindex excluding
 *	the ifindex of source netdev.
 */
int32_t ecm_interface_multicast_check_for_src_ifindex(int32_t mc_if_index[], int32_t max_if_index, int32_t if_num)
{
	int32_t i;
	int32_t valid_index;

	for (i = 0, valid_index = 0; i < max_if_index; i++) {
		if (mc_if_index[i] == 0) {
			break;
		}

		if (mc_if_index[i] != if_num) {
			mc_if_index[valid_index] = mc_if_index[i];
			valid_index++;
			continue;
		}
	}

	return valid_index;
}
EXPORT_SYMBOL(ecm_interface_multicast_check_for_src_ifindex);
#endif

#ifdef ECM_INTERFACE_VXLAN_ENABLE
/*
 * ecm_interface_vxlan_type_get()
 *	Function to get VxLAN interface type i.e. inner/outer.
 *	Returns 0 for outer and 1 for inner.
 */
uint32_t ecm_interface_vxlan_type_get(struct sk_buff *skb, struct vxlan_dev *vxlan_tun)
{
	ip_addr_t saddr, daddr, vx_addr, packet_addr;
	struct net_device *local_dev;
	struct net_device *lower_dev = NULL;
	union vxlan_addr *src_ip, *remote_ip;
	bool packet_type_v4 = true;
	bool tunnel_type_v4 = true;

	if (!skb) {
		return -1;
	}

	switch (ntohs(skb->protocol)) {
	case ETH_P_IP:
		ECM_NIN4_ADDR_TO_IP_ADDR(saddr, ip_hdr(skb)->saddr);
		ECM_NIN4_ADDR_TO_IP_ADDR(daddr, ip_hdr(skb)->daddr);
		break;
	case ETH_P_IPV6:
		packet_type_v4 = false;
		ECM_NIN6_ADDR_TO_IP_ADDR(saddr, ipv6_hdr(skb)->saddr);
		ECM_NIN6_ADDR_TO_IP_ADDR(daddr, ipv6_hdr(skb)->daddr);
		break;
	default:
		DEBUG_WARN("%px: Unknown skb protocol.\n", skb);
		return -1;
	}

	lower_dev = dev_get_by_index(&init_net, vxlan_tun->default_dst.remote_ifindex);
	if (lower_dev) {
		local_dev = ecm_interface_dev_find_by_local_addr(saddr);
		if (lower_dev == local_dev) {
			dev_put(local_dev);
			dev_put(lower_dev);
			DEBUG_TRACE("%p: VxLAN outer interface type.\n", skb);
			return 0;
		}

		if (local_dev) {
			dev_put(local_dev);
		}

		dev_put(lower_dev);
		DEBUG_TRACE("%p: VxLAN inner interface type.\n", skb);
		return 1;
	}

	/*
	 * Need to verify packet vs tunnel even if the tunnel is v6 and packet is v4,
	 * One may assume that the packet must be inner since their type doesn't match. However, we need to verify that the tunnel has valid IP address
	 * So, we shouldn't accelerate traffic if tunnel doesn't have valid IP address.
	 */
	DEBUG_TRACE("%p: lower device is not set, check with source IP address.\n", skb);
	src_ip = &vxlan_tun->cfg.saddr;
	if (src_ip->sa.sa_family == AF_INET) {
		if (src_ip->sin.sin_addr.s_addr != htonl(INADDR_ANY)) {
			ECM_NIN4_ADDR_TO_IP_ADDR(vx_addr, src_ip->sin.sin_addr.s_addr);
			ECM_IP_ADDR_COPY(packet_addr, saddr);
		} else {
			DEBUG_TRACE("%p: Src IP addr is not set. Check with remote IP addr.\n", skb);
			remote_ip = &vxlan_tun->cfg.remote_ip;
			if (remote_ip->sin.sin_addr.s_addr == htonl(INADDR_ANY)) {
				DEBUG_TRACE("%p: Src/Remote IP addr are not set. Cannot determine tunnel direction.\n", skb);
				return -1;
			}
			ECM_NIN4_ADDR_TO_IP_ADDR(vx_addr, remote_ip->sin.sin_addr.s_addr);
			ECM_IP_ADDR_COPY(packet_addr, daddr);
		}
	} else {
		tunnel_type_v4 = false;
		if (!ipv6_addr_any(&src_ip->sin6.sin6_addr)) {
			ECM_NIN6_ADDR_TO_IP_ADDR(vx_addr, src_ip->sin6.sin6_addr);
			ECM_IP_ADDR_COPY(packet_addr, saddr);
		} else {
			DEBUG_TRACE("%p: Src IP addr is not set. Check with remote IP addr.\n", skb);
			remote_ip = &vxlan_tun->cfg.remote_ip;
			if (ipv6_addr_any(&remote_ip->sin6.sin6_addr)) {
				DEBUG_TRACE("%p: Src/Remote IP addr are not set. Cannot determine tunnel direction.\n", skb);
				return -1;
			}
			ECM_NIN6_ADDR_TO_IP_ADDR(vx_addr, remote_ip->sin6.sin6_addr);
			ECM_IP_ADDR_COPY(packet_addr, daddr);
		}
	}

	/*
	 * If packet and tunnel in different INET, it must be inner direction
	 */
	if (tunnel_type_v4 != packet_type_v4) {
		DEBUG_TRACE("%p: VxLAN inner interface type.\n", skb);
		return 1;
	}

	if (ECM_IP_ADDR_MATCH(vx_addr, packet_addr)) {
		DEBUG_TRACE("%p: VxLAN outer interface type.\n", skb);
		return 0;
	}

	DEBUG_TRACE("%p: VxLAN inner interface type.\n", skb);
	return 1;
}
#endif

/*
 * ecm_interface_addr_find_route_by_addr_ipv4()
 *	Return the route for the given IP address.  Returns NULL on failure.
 */
static bool ecm_interface_find_route_by_addr_ipv4(ip_addr_t addr, struct ecm_interface_route *ecm_rt)
{
	__be32 be_addr;

	/*
	 * Get a route to the given IP address, this will allow us to also find the interface
	 * it is using to communicate with that IP address.
	 */
	ECM_IP_ADDR_TO_NIN4_ADDR(be_addr, addr);
	ecm_rt->rt.rtv4 = ip_route_output(&init_net, be_addr, 0, 0, 0);
	if (IS_ERR(ecm_rt->rt.rtv4)) {
		DEBUG_TRACE("No output route to: %pI4n\n", &be_addr);
		return false;
	}
	DEBUG_TRACE("Output route to: %pI4n is: %px\n", &be_addr, ecm_rt->rt.rtv4);
	ecm_rt->dst = (struct dst_entry *)ecm_rt->rt.rtv4;
	ecm_rt->v4_route = true;
	return true;
}

#ifdef ECM_IPV6_ENABLE
struct rt6_info *ecm_interface_ipv6_route_lookup(struct net *netf, struct in6_addr *addr)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 17, 0))
	return rt6_lookup(netf, addr, NULL, 0, 0);
#else
	return rt6_lookup(netf, addr, NULL, 0, NULL, 0);
#endif
}

/*
 * ecm_interface_addr_find_route_by_addr_ipv6()
 *	Return the route for the given IP address.  Returns NULL on failure.
 */
static bool ecm_interface_find_route_by_addr_ipv6(ip_addr_t addr, struct ecm_interface_route *ecm_rt)
{
	struct in6_addr naddr;

	ECM_IP_ADDR_TO_NIN6_ADDR(naddr, addr);

	/*
	 * Get a route to the given IP address, this will allow us to also find the interface
	 * it is using to communicate with that IP address.
	 */
	ecm_rt->rt.rtv6 = ecm_interface_ipv6_route_lookup(&init_net, &naddr);
	if (!ecm_rt->rt.rtv6) {
		DEBUG_TRACE("No output route to: " ECM_IP_ADDR_OCTAL_FMT "\n", ECM_IP_ADDR_TO_OCTAL(addr));
		return NULL;
	}
	DEBUG_TRACE("Output route to: " ECM_IP_ADDR_OCTAL_FMT " is: %px\n", ECM_IP_ADDR_TO_OCTAL(addr), ecm_rt->rt.rtv6);
	ecm_rt->dst = (struct dst_entry *)ecm_rt->rt.rtv6;
	ecm_rt->v4_route = false;
	return true;
}
#endif

/*
 * ecm_interface_addr_find_route_by_addr()
 *	Return the route (in the given parameter) for the given IP address.  Returns false on failure.
 *
 * Route is the device on which the addr is reachable, which may be loopback for local addresses.
 *
 * Returns true if the route was able to be located.  The route must be released using ecm_interface_route_release().
 */
bool ecm_interface_find_route_by_addr(ip_addr_t addr, struct ecm_interface_route *ecm_rt)
{
	if (ECM_IP_ADDR_IS_V4(addr)) {
		DEBUG_TRACE("Locate dev for " ECM_IP_ADDR_DOT_FMT "\n", ECM_IP_ADDR_TO_DOT(addr));
		return ecm_interface_find_route_by_addr_ipv4(addr, ecm_rt);
	}

	DEBUG_TRACE("Locate dev for " ECM_IP_ADDR_OCTAL_FMT "\n", ECM_IP_ADDR_TO_OCTAL(addr));
#ifdef ECM_IPV6_ENABLE
	return ecm_interface_find_route_by_addr_ipv6(addr, ecm_rt);
#else
	return false;
#endif
}
EXPORT_SYMBOL(ecm_interface_find_route_by_addr);

/*
 * ecm_interface_route_release()
 *	Release an ecm route
 */
void ecm_interface_route_release(struct ecm_interface_route *rt)
{
	dst_release(rt->dst);
}
EXPORT_SYMBOL(ecm_interface_route_release);

#ifdef ECM_IPV6_ENABLE
/*
 * ecm_interface_send_neighbour_solicitation()
 *	Issue an IPv6 Neighbour soliciation request.
 */
void ecm_interface_send_neighbour_solicitation(struct net_device *dev, ip_addr_t addr)
{
	struct in6_addr dst_addr, src_addr;
	struct in6_addr mc_dst_addr;
	struct rt6_info *rt6i;
	struct neighbour *neigh;
	struct net *netf = dev_net(dev);
	int ret;

	/*
	 * Find source and destination addresses in Linux format. We need
	 * mcast destination address as well.
	 */
	ECM_IP_ADDR_TO_NIN6_ADDR(dst_addr, addr);
	addrconf_addr_solict_mult(&dst_addr, &mc_dst_addr);
	ret = ipv6_dev_get_saddr(netf, dev, &mc_dst_addr, 0, &src_addr);

	/*
	 * Find the route entry
	 */
	rt6i = ecm_interface_ipv6_route_lookup(netf, &dst_addr);
	if (!rt6i) {
		DEBUG_TRACE("IPv6 Route lookup failure for destination IPv6 address " ECM_IP_ADDR_OCTAL_FMT "\n", ECM_IP_ADDR_TO_OCTAL(addr));
		return;
	}

	/*
	 * Find the neighbor entry
	 */
	neigh = rt6i->dst.ops->neigh_lookup(&rt6i->dst, NULL, &dst_addr);
	if (IS_ERR(neigh)) {
		DEBUG_TRACE("Neighbour lookup failure for destination IPv6 address " ECM_IP_ADDR_OCTAL_FMT "\n", ECM_IP_ADDR_TO_OCTAL(addr));
		dst_release(&rt6i->dst);
		return;
	}

	/*
	 * Issue a Neighbour soliciation request
	 */
	DEBUG_TRACE("Issue Neighbour solicitation request\n");
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 10, 0))
	ndisc_send_ns(dev, &dst_addr, &mc_dst_addr, &src_addr);
#else
	ndisc_send_ns(dev, &dst_addr, &mc_dst_addr, &src_addr, 0);
#endif
	neigh_release(neigh);
	dst_release(&rt6i->dst);
}
EXPORT_SYMBOL(ecm_interface_send_neighbour_solicitation);
#endif

/*
 * ecm_interface_send_arp_request()
 *	Issue and ARP request.
 */
void ecm_interface_send_arp_request(struct net_device *dest_dev, ip_addr_t dest_addr, bool on_link, ip_addr_t gw_addr)
{
	/*
	 * Possible ARP does not know the address yet
	 */
	struct neighbour *neigh;
	__be32 ipv4_addr;

	/*
	 * Convert the ECM IP address type to network order IPv4 address.
	 */
	ECM_IP_ADDR_TO_NIN4_ADDR(ipv4_addr, dest_addr);

	/*
	 * If we have a GW for this address, then we have to send ARP request to the GW
	 */
	if (!on_link && !ECM_IP_ADDR_IS_NULL(gw_addr)) {
		ECM_IP_ADDR_TO_NIN4_ADDR(ipv4_addr, gw_addr);
	}

	/*
	 * If we don't have this neighbor, create it before sending the arp request,
	 * so that when we receive the arp reply we update the neigh entry.
	 */
	neigh = neigh_lookup(&arp_tbl, &ipv4_addr, dest_dev);
	if (!neigh) {
		neigh = neigh_create(&arp_tbl, &ipv4_addr, dest_dev);
		if (IS_ERR(neigh)) {
			DEBUG_WARN("Unable to create ARP request neigh for %pI4\n", &ipv4_addr);
			return;
		}
	}

	DEBUG_TRACE("Send ARP for %pI4\n", &ipv4_addr);
	neigh_event_send(neigh, NULL);
	neigh_release(neigh);
}
EXPORT_SYMBOL(ecm_interface_send_arp_request);

/*
 * ecm_interface_ipv4_neigh_get()
 *	Returns neighbour reference for a given IP address which must be released when you are done with it.
 *
 * Returns NULL on fail.
 */
struct neighbour *ecm_interface_ipv4_neigh_get(ip_addr_t addr)
{
	struct neighbour *neigh;
	struct rtable *rt;
	struct dst_entry *dst;
	__be32 ipv4_addr;

	ECM_IP_ADDR_TO_NIN4_ADDR(ipv4_addr, addr);
	rt = ip_route_output(&init_net, ipv4_addr, 0, 0, 0);
	if (IS_ERR(rt)) {
		return NULL;
	}
	dst = (struct dst_entry *)rt;
	neigh = dst_neigh_lookup(dst, &ipv4_addr);
	ip_rt_put(rt);
	return neigh;
}

#ifdef ECM_IPV6_ENABLE
/*
 * ecm_interface_ipv6_neigh_get()
 *	Returns neighbour reference for a given IP address which must be released when you are done with it.
 *
 * Returns NULL on fail.
 */
struct neighbour *ecm_interface_ipv6_neigh_get(struct ecm_front_end_connection_instance *feci, ecm_db_obj_dir_t dir, ip_addr_t addr)
{
	struct neighbour *neigh;
	struct in6_addr ipv6_addr;
	int32_t ifaces_first, iface_idx;
	struct net_device *netdev;
	struct ecm_db_iface_instance *ifaces[ECM_DB_IFACE_HEIRARCHY_MAX];

	ifaces_first = ecm_db_connection_interfaces_get_and_ref(feci->ci, ifaces, dir);
	if (ifaces_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
		return NULL;
	}

	iface_idx = ecm_db_iface_interface_identifier_get(ifaces[ECM_DB_IFACE_HEIRARCHY_MAX-1]);
	netdev = dev_get_by_index(&init_net, iface_idx);
	if (!netdev) {
		ecm_db_connection_interfaces_deref(ifaces, ifaces_first);
		return NULL;
	}

	ECM_IP_ADDR_TO_NIN6_ADDR(ipv6_addr, addr);
	neigh = neigh_lookup(&nd_tbl, &ipv6_addr, netdev);

	dev_put(netdev);
	ecm_db_connection_interfaces_deref(ifaces, ifaces_first);

	return neigh;
}
#endif

/*
 * ecm_interface_is_pptp()
 *	skip pptp tunnel encapsulated traffic
 *
 * ECM does not handle PPTP,
 * this function detects packets of that type so they can be skipped over to improve their throughput.
 */
bool ecm_interface_is_pptp(struct sk_buff *skb, const struct net_device *out)
{
#ifdef ECM_INTERFACE_PPTP_ENABLE
	struct net_device *in;

	/*
	 * skip first pass of l2tp/pptp tunnel encapsulated traffic
	 */
	if (out->type == ARPHRD_PPP) {
		if (out->priv_flags_ext & IFF_EXT_PPP_PPTP) {
			return true;
		}
	}

	in = dev_get_by_index(&init_net, skb->skb_iif);
	if (!in) {
		return true;
	}

	if (in->type == ARPHRD_PPP) {
		if (in->priv_flags_ext & IFF_EXT_PPP_PPTP) {
			dev_put(in);
			return true;
		}
	}

	dev_put(in);
#endif
	return false;
}

/*
 * ecm_interface_is_l2tp_packet_by_version()
 *	Check version of l2tp tunnel encapsulated traffic
 *
 * ECM does not handle l2tp,
 * this function detects packets of that type so they can be skipped over to improve their throughput.
 */
bool ecm_interface_is_l2tp_packet_by_version(struct sk_buff *skb, const struct net_device *out, int ver)
{
#ifdef ECM_INTERFACE_L2TPV2_PPTP_ENABLE
	uint32_t flag = 0;
	struct net_device *in;

	switch (ver) {
	case 2:
		flag = IFF_EXT_PPP_L2TPV2;
		break;
	case 3:
		flag = IFF_EXT_PPP_L2TPV3;
		break;
	default:
		break;
	}

	/*
	 * skip first pass of l2tp/pptp tunnel encapsulated traffic
	 */
	if (out->priv_flags_ext & flag) {
		return true;
	}

	in = dev_get_by_index(&init_net, skb->skb_iif);
	if (!in) {
		return true;
	}

	if (in->priv_flags_ext & flag) {
		dev_put(in);
		return true;
	}

	dev_put(in);
#endif
	return false;
}

/*
 * ecm_interface_is_l2tp_pptp()
 *	skip l2tp/pptp tunnel encapsulated traffic
 *
 * ECM does not handle L2TP or PPTP encapsulated packets,
 * this function detects packets of that type so they can be skipped over to improve their throughput.
 */
bool ecm_interface_is_l2tp_pptp(struct sk_buff *skb, const struct net_device *out)
{
#ifdef ECM_INTERFACE_L2TPV2_PPTP_ENABLE
	struct net_device *in;

	/*
	 * skip first pass of l2tp/pptp tunnel encapsulated traffic
	 */
	if (out->priv_flags_ext & (IFF_EXT_PPP_L2TPV2 | IFF_EXT_PPP_L2TPV3 |
			       IFF_EXT_PPP_PPTP)) {
		return true;
	}

	in = dev_get_by_index(&init_net, skb->skb_iif);
	if (!in) {
		return true;
	}

	if (in->priv_flags_ext & (IFF_EXT_PPP_L2TPV2 | IFF_EXT_PPP_L2TPV3 |
			      IFF_EXT_PPP_PPTP)) {
		dev_put(in);
		return true;
	}

	dev_put(in);
#endif
	return false;
}

#ifdef ECM_INTERFACE_VLAN_ENABLE
/*
 * ecm_interface_vlan_interface_establish()
 *	Returns a reference to a iface of the VLAN type, possibly creating one if necessary.
 * Returns NULL on failure or a reference to interface.
 */
static struct ecm_db_iface_instance *ecm_interface_vlan_interface_establish(struct ecm_db_interface_info_vlan *type_info,
							char *dev_name, int32_t dev_interface_num, int32_t ae_interface_num, int32_t mtu)
{
	struct ecm_db_iface_instance *nii;
	struct ecm_db_iface_instance *ii;

	DEBUG_INFO("Establish VLAN iface: %s with address: %pM, vlan tag: %u, vlan_tpid: %x MTU: %d, if num: %d, accel engine if id: %d\n",
			dev_name, type_info->address, type_info->vlan_tag, type_info->vlan_tpid, mtu, dev_interface_num, ae_interface_num);

	/*
	 * Locate the iface
	 */
	ii = ecm_db_iface_find_and_ref_vlan(type_info->address, type_info->vlan_tag, type_info->vlan_tpid);
	if (ii) {
		DEBUG_TRACE("%px: iface established\n", ii);
		return ii;
	}

	/*
	 * No iface - create one
	 */
	nii = ecm_db_iface_alloc();
	if (!nii) {
		DEBUG_WARN("Failed to establish iface\n");
		return NULL;
	}

	/*
	 * Add iface into the database, atomically to avoid races creating the same thing
	 */
	spin_lock_bh(&ecm_interface_lock);
	ii = ecm_db_iface_find_and_ref_vlan(type_info->address, type_info->vlan_tag, type_info->vlan_tpid);
	if (ii) {
		spin_unlock_bh(&ecm_interface_lock);
		ecm_db_iface_deref(nii);
		return ii;
	}
	ecm_db_iface_add_vlan(nii, type_info->address, type_info->vlan_tag, type_info->vlan_tpid, dev_name,
			mtu, dev_interface_num, ae_interface_num, NULL, nii);
	spin_unlock_bh(&ecm_interface_lock);

	DEBUG_TRACE("%px: vlan iface established\n", nii);
	return nii;
}
#endif

#ifdef ECM_INTERFACE_MACVLAN_ENABLE
/*
 * ecm_interface_macvlan_interface_establish()
 *	Returns a reference to a iface of the MACVLAN type, possibly creating one if necessary.
 * Returns NULL on failure or a reference to interface.
 */
static struct ecm_db_iface_instance *ecm_interface_macvlan_interface_establish(struct ecm_db_interface_info_macvlan *type_info,
							char *dev_name, int32_t dev_interface_num, int32_t ae_interface_num, int32_t mtu)
{
	struct ecm_db_iface_instance *nii;
	struct ecm_db_iface_instance *ii;

	DEBUG_INFO("Establish MACVLAN iface: %s with address: %pM, MTU: %d, if num: %d, accel engine if id: %d\n",
			dev_name, type_info->address, mtu, dev_interface_num, ae_interface_num);

	/*
	 * Locate the iface
	 */
	ii = ecm_db_iface_find_and_ref_macvlan(type_info->address);
	if (ii) {
		DEBUG_TRACE("%px: iface established\n", ii);
		return ii;
	}

	/*
	 * No iface - create one
	 */
	nii = ecm_db_iface_alloc();
	if (!nii) {
		DEBUG_WARN("Failed to establish iface\n");
		return NULL;
	}

	/*
	 * Add iface into the database, atomically to avoid races creating the same thing
	 */
	spin_lock_bh(&ecm_interface_lock);
	ii = ecm_db_iface_find_and_ref_macvlan(type_info->address);
	if (ii) {
		spin_unlock_bh(&ecm_interface_lock);
		ecm_db_iface_deref(nii);
		return ii;
	}
	ecm_db_iface_add_macvlan(nii, type_info->address, dev_name,
			mtu, dev_interface_num, ae_interface_num, NULL, nii);
	spin_unlock_bh(&ecm_interface_lock);

	DEBUG_TRACE("%px: MACVLAN iface established\n", nii);
	return nii;
}
#endif

#if defined(ECM_INTERFACE_OVS_BRIDGE_ENABLE) && defined(ECM_MULTICAST_ENABLE)
/*
 * ecm_interface_multicast_ovs_to_interface_get_and_ref()
 *	Populate ov_ ports/bridge device from multicast 'to' list.
 *	Returns the number of ovs port count.
 */
int ecm_interface_multicast_ovs_to_interface_get_and_ref(struct ecm_db_connection_instance *ci, struct net_device **to_ovs_port,
							struct net_device **to_ovs_brdev)
{
	struct net_device *dev;
	struct ecm_db_iface_instance *to_mc_ifaces;
	int32_t *to_mc_ifaces_first, *to_iface_first, if_cnt, i;
	int ovs_port_cnt = 0;

	if_cnt = ecm_db_multicast_connection_to_interfaces_get_and_ref_all(ci, &to_mc_ifaces, &to_mc_ifaces_first);
	if (!if_cnt) {
		DEBUG_WARN("%px: Not able to find 'to' interfaces", ci);
		return 0;
	}

	/*
	 * The 'to' interfaces ports can be part of different OVS bridges.
	 * ovs-br1-(eth0, eth1, eth2)
	 * ovs-br2-(eth3)
	 * ovs-br3 (eth4, eth5)
	 *
	 * to_ovs_port: eth0, eth1, eth2, eth3. eth4. eth5
	 * to_ovs_brdev: ovs-br1, ovs-br1, ovs-br1, ovs-br2, ovs-br3, ovs-br3
	 */
	for (i = 0; i < ECM_DB_MULTICAST_IF_MAX; i++) {
		struct ecm_db_iface_instance *ii_temp;
		int32_t j;

		/*
		 * Find interface list, skip if invalid.
		 */
		to_iface_first = ecm_db_multicast_if_first_get_at_index(to_mc_ifaces_first, i);
		if (*to_iface_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
			continue;
		}

		/*
		 * We need to find 'to' multicast port to
		 * update the OVS statistics.
		 */
		ii_temp = ecm_db_multicast_if_heirarchy_get(to_mc_ifaces, i);
		for (j = ECM_DB_IFACE_HEIRARCHY_MAX - 1; j >= *to_iface_first; j--) {
			struct net_device *br_dev;
			struct ecm_db_iface_instance **ifaces;
			struct ecm_db_iface_instance *to_iface;
			struct ecm_db_iface_instance *ii_single;

			ii_single = ecm_db_multicast_if_instance_get_at_index(ii_temp, j);
			ifaces = (struct ecm_db_iface_instance **)ii_single;
			to_iface = *ifaces;
			dev = dev_get_by_index(&init_net, ecm_db_iface_interface_identifier_get(to_iface));
			if (unlikely(!dev)) {
				DEBUG_WARN("%px: Failed to get net device with %d index\n", ci, j);
				continue;
			}

			if (!ecm_interface_is_ovs_bridge_port(dev)) {
				DEBUG_TRACE("%px: %s_dev: %s at %d index is not an OVS bridge port\n", ci, ecm_db_obj_dir_strings[ECM_DB_OBJ_DIR_TO], dev->name, j);
				dev_put(dev);
				continue;
			}

			br_dev = ovsmgr_dev_get_master(dev);
			DEBUG_ASSERT(br_dev, "%px: master dev for the OVS port:%s is NULL\n", ci, dev->name);
			to_ovs_port[ovs_port_cnt] = dev;
			to_ovs_brdev[ovs_port_cnt] = br_dev;
			DEBUG_TRACE("%px: %s_dev: %s at %d index is an OVS bridge port. OVS bridge: %s\n", ci, ecm_db_obj_dir_strings[ECM_DB_OBJ_DIR_TO], dev->name, j, br_dev->name);
			ovs_port_cnt++;
			break;
		}
	}

	ecm_db_multicast_connection_to_interfaces_deref_all(to_mc_ifaces, to_mc_ifaces_first);
	return ovs_port_cnt;
}
#endif

#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
/*
 * ecm_interface_is_ovs_bridge_port()
 *	Returns true if dev is OpenVswitch (OVS) bridge port.
 */
bool ecm_interface_is_ovs_bridge_port(const struct net_device *dev)
{
	/*
	 * Check if dev is OVS bridge port.
	 */
	return !!(dev->priv_flags & IFF_OVS_DATAPATH);
}
#endif

/*
 * ecm_interface_bridge_interface_establish()
 *	Returns a reference to a iface of the BRIDGE type, possibly creating one if necessary.
 * Returns NULL on failure or a reference to interface.
 */
static struct ecm_db_iface_instance *ecm_interface_bridge_interface_establish(struct ecm_db_interface_info_bridge *type_info,
							char *dev_name, int32_t dev_interface_num, int32_t ae_interface_num, int32_t mtu)
{
	struct ecm_db_iface_instance *nii;
	struct ecm_db_iface_instance *ii;

	DEBUG_INFO("Establish BRIDGE iface: %s with address: %pM, MTU: %d, if num: %d, accel engine if id: %d\n",
			dev_name, type_info->address, mtu, dev_interface_num, ae_interface_num);

	/*
	 * Locate the iface
	 */
	ii = ecm_db_iface_find_and_ref_bridge(type_info->address, dev_interface_num);
	if (ii) {
		DEBUG_TRACE("%px: iface established\n", ii);
		return ii;
	}

	/*
	 * No iface - create one
	 */
	nii = ecm_db_iface_alloc();
	if (!nii) {
		DEBUG_WARN("Failed to establish iface\n");
		return NULL;
	}

	/*
	 * Add iface into the database, atomically to avoid races creating the same thing
	 */
	spin_lock_bh(&ecm_interface_lock);
	ii = ecm_db_iface_find_and_ref_bridge(type_info->address, dev_interface_num);
	if (ii) {
		spin_unlock_bh(&ecm_interface_lock);
		ecm_db_iface_deref(nii);
		return ii;
	}
	ecm_db_iface_add_bridge(nii, type_info->address, dev_name,
			mtu, dev_interface_num, ae_interface_num, NULL, nii);
	spin_unlock_bh(&ecm_interface_lock);

	DEBUG_TRACE("%px: bridge iface established\n", nii);
	return nii;
}

#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
/*
 * ecm_interface_ovs_bridge_interface_establish()
 *	Returns a reference to a iface of the OVS BRIDGE type, possibly creating one if necessary.
 * Returns NULL on failure or a reference to interface.
 */
static struct ecm_db_iface_instance *ecm_interface_ovs_bridge_interface_establish(struct ecm_db_interface_info_ovs_bridge *type_info,
							char *dev_name, int32_t dev_interface_num, int32_t ae_interface_num, int32_t mtu)
{
	struct ecm_db_iface_instance *nii;
	struct ecm_db_iface_instance *ii;

	DEBUG_INFO("Establish OVS BRIDGE iface: %s with address: %pM, MTU: %d, if num: %d, accel engine if id: %d\n",
			dev_name, type_info->address, mtu, dev_interface_num, ae_interface_num);

	/*
	 * Locate the iface
	 */
	ii = ecm_db_iface_find_and_ref_ovs_bridge(type_info->address, dev_interface_num);
	if (ii) {
		DEBUG_TRACE("%px: iface established\n", ii);
		return ii;
	}

	/*
	 * No iface - create one
	 */
	nii = ecm_db_iface_alloc();
	if (!nii) {
		DEBUG_WARN("Failed to establish iface\n");
		return NULL;
	}

	/*
	 * Add iface into the database, atomically to avoid races creating the same thing
	 */
	spin_lock_bh(&ecm_interface_lock);
	ii = ecm_db_iface_find_and_ref_ovs_bridge(type_info->address, dev_interface_num);
	if (ii) {
		spin_unlock_bh(&ecm_interface_lock);
		ecm_db_iface_deref(nii);
		return ii;
	}
	ecm_db_iface_add_ovs_bridge(nii, type_info->address, dev_name,
			mtu, dev_interface_num, ae_interface_num, NULL, nii);
	spin_unlock_bh(&ecm_interface_lock);

	DEBUG_TRACE("%px: OVS bridge iface established\n", nii);
	return nii;
}
#endif

#ifdef ECM_INTERFACE_BOND_ENABLE
/*
 * ecm_interface_lag_interface_establish()
 *	Returns a reference to a iface of the LAG type, possibly creating one if necessary.
 * Returns NULL on failure or a reference to interface.
 */
static struct ecm_db_iface_instance *ecm_interface_lag_interface_establish(struct ecm_db_interface_info_lag *type_info,
							char *dev_name, int32_t dev_interface_num, int32_t ae_interface_num, int32_t mtu)
{
	struct ecm_db_iface_instance *nii;
	struct ecm_db_iface_instance *ii;

	DEBUG_INFO("Establish LAG iface: %s with address: %pM, MTU: %d, if num: %d, accel engine if id: %d\n",
			dev_name, type_info->address, mtu, dev_interface_num, ae_interface_num);

	/*
	 * Locate the iface
	 */
	ii = ecm_db_iface_find_and_ref_lag(type_info->address);
	if (ii) {
		DEBUG_TRACE("%px: iface established\n", ii);
		return ii;
	}

	/*
	 * No iface - create one
	 */
	nii = ecm_db_iface_alloc();
	if (!nii) {
		DEBUG_WARN("Failed to establish iface\n");
		return NULL;
	}

	/*
	 * Add iface into the database, atomically to avoid races creating the same thing
	 */
	spin_lock_bh(&ecm_interface_lock);
	ii = ecm_db_iface_find_and_ref_lag(type_info->address);
	if (ii) {
		spin_unlock_bh(&ecm_interface_lock);
		ecm_db_iface_deref(nii);
		return ii;
	}
	ecm_db_iface_add_lag(nii, type_info->address, dev_name,
			mtu, dev_interface_num, ae_interface_num, NULL, nii);
	spin_unlock_bh(&ecm_interface_lock);

	DEBUG_TRACE("%px: lag iface established\n", nii);
	return nii;
}
#endif

/*
 * ecm_interface_ethernet_interface_establish()
 *	Returns a reference to a iface of the ETHERNET type, possibly creating one if necessary.
 * Returns NULL on failure or a reference to interface.
 */
static struct ecm_db_iface_instance *ecm_interface_ethernet_interface_establish(struct ecm_db_interface_info_ethernet *type_info,
							char *dev_name, int32_t dev_interface_num, int32_t ae_interface_num, int32_t mtu)
{
	struct ecm_db_iface_instance *nii;
	struct ecm_db_iface_instance *ii;

	DEBUG_INFO("Establish ETHERNET iface: %s with address: %pM, MTU: %d, if num: %d, accel engine if id: %d\n",
			dev_name, type_info->address, mtu, dev_interface_num, ae_interface_num);

	/*
	 * Locate the iface
	 */
	ii = ecm_db_iface_ifidx_find_and_ref_ethernet(type_info->address, dev_interface_num, ae_interface_num);

	if (ii) {
		DEBUG_TRACE("%px: iface established\n", ii);
		return ii;
	}

	/*
	 * No iface - create one
	 */
	nii = ecm_db_iface_alloc();
	if (!nii) {
		DEBUG_WARN("Failed to establish iface\n");
		return NULL;
	}

	/*
	 * Add iface into the database, atomically to avoid races creating the same thing
	 */
	spin_lock_bh(&ecm_interface_lock);
	ii = ecm_db_iface_ifidx_find_and_ref_ethernet(type_info->address, dev_interface_num, ae_interface_num);
	if (ii) {
		spin_unlock_bh(&ecm_interface_lock);
		ecm_db_iface_deref(nii);
		return ii;
	}
	ecm_db_iface_add_ethernet(nii, type_info->address, dev_name,
			mtu, dev_interface_num, ae_interface_num, NULL, nii);
	spin_unlock_bh(&ecm_interface_lock);

	DEBUG_TRACE("%px: ethernet iface established\n", nii);
	return nii;
}

#ifdef ECM_INTERFACE_PPPOE_ENABLE
/*
 * ecm_interface_pppoe_interface_establish()
 *	Returns a reference to a iface of the PPPoE type, possibly creating one if necessary.
 * Returns NULL on failure or a reference to interface.
 */
static struct ecm_db_iface_instance *ecm_interface_pppoe_interface_establish(struct ecm_db_interface_info_pppoe *type_info,
							char *dev_name, int32_t dev_interface_num, int32_t ae_interface_num, int32_t mtu)
{
	struct ecm_db_iface_instance *nii;
	struct ecm_db_iface_instance *ii;

	DEBUG_INFO("Establish PPPoE iface: %s with session id: %u, remote mac: %pM, MTU: %d, if num: %d, accel engine if id: %d\n",
			dev_name, type_info->pppoe_session_id, type_info->remote_mac, mtu, dev_interface_num, ae_interface_num);

	/*
	 * Locate the iface
	 */
	ii = ecm_db_iface_find_and_ref_pppoe(type_info->pppoe_session_id, type_info->remote_mac);
	if (ii) {
		DEBUG_TRACE("%px: iface established\n", ii);
		return ii;
	}

	/*
	 * No iface - create one
	 */
	nii = ecm_db_iface_alloc();
	if (!nii) {
		DEBUG_WARN("Failed to establish iface\n");
		return NULL;
	}

	/*
	 * Add iface into the database, atomically to avoid races creating the same thing
	 */
	spin_lock_bh(&ecm_interface_lock);
	ii = ecm_db_iface_find_and_ref_pppoe(type_info->pppoe_session_id, type_info->remote_mac);
	if (ii) {
		spin_unlock_bh(&ecm_interface_lock);
		ecm_db_iface_deref(nii);
		return ii;
	}
	ecm_db_iface_add_pppoe(nii, type_info->pppoe_session_id, type_info->remote_mac, dev_name,
			mtu, dev_interface_num, ae_interface_num, NULL, nii);
	spin_unlock_bh(&ecm_interface_lock);

	DEBUG_TRACE("%px: pppoe iface established\n", nii);
	return nii;
}
#endif

#ifdef ECM_INTERFACE_MAP_T_ENABLE
/*
 * ecm_interface_map_t_interface_establish()
 *	Returns a reference to a iface of the PPPoE type, possibly creating one if necessary.
 * Returns NULL on failure or a reference to interface.
 */
static struct ecm_db_iface_instance *ecm_interface_map_t_interface_establish(struct ecm_db_interface_info_map_t *type_info,
							char *dev_name, int32_t dev_interface_num, int32_t ae_interface_num, int32_t mtu)
{
	struct ecm_db_iface_instance *nii;
	struct ecm_db_iface_instance *ii;

	DEBUG_TRACE("Establish MAP-T iface: %s   MTU: %d, if num: %d, accel engine if id: %d\n",
			dev_name, mtu, dev_interface_num, ae_interface_num);

	/*
	 * Locate the iface
	 */
	ii = ecm_db_iface_find_and_ref_map_t(type_info->if_index, ae_interface_num);
	if (ii) {
		DEBUG_TRACE("%px: iface established\n", ii);
		return ii;
	}

	/*
	 * No iface - create one
	 */
	nii = ecm_db_iface_alloc();
	if (!nii) {
		DEBUG_WARN("Failed to establish iface\n");
		return NULL;
	}

	/*
	 * Add iface into the database, atomically to avoid races creating the same thing
	 */
	spin_lock_bh(&ecm_interface_lock);
	ii = ecm_db_iface_find_and_ref_map_t(type_info->if_index, ae_interface_num);
	if (ii) {
		spin_unlock_bh(&ecm_interface_lock);
		ecm_db_iface_deref(nii);
		return ii;
	}
	ecm_db_iface_add_map_t(nii, type_info, dev_name,
			mtu, dev_interface_num, ae_interface_num, NULL, nii);
	spin_unlock_bh(&ecm_interface_lock);

	DEBUG_TRACE("%px: map_t iface established\n", nii);
	return nii;
}
#endif

#ifdef ECM_INTERFACE_L2TPV2_ENABLE
/*
 * ecm_interface_pppol2tpv2_interface_establish()
 *	Returns a reference to a iface of the PPPoL2TPV2 type, possibly creating one if necessary.
 *	Returns NULL on failure or a reference to interface.
 */
static struct ecm_db_iface_instance *ecm_interface_pppol2tpv2_interface_establish(struct ecm_db_interface_info_pppol2tpv2 *type_info,
							char *dev_name, int32_t dev_interface_num, int32_t ae_interface_num, int32_t mtu)
{
	struct ecm_db_iface_instance *nii;
	struct ecm_db_iface_instance *ii;

	DEBUG_INFO("Establish PPPol2tp iface: %s with tunnel id=%u session id %u\n", dev_name, type_info->l2tp.tunnel.tunnel_id,
				type_info->l2tp.session.session_id);
	/*
	 * Locate the iface
	 */
	ii = ecm_db_iface_find_and_ref_pppol2tpv2(type_info->l2tp.tunnel.tunnel_id, type_info->l2tp.session.session_id);
	if (ii) {
		DEBUG_TRACE("%px: iface established\n", ii);
		ecm_db_iface_update_ae_interface_identifier(ii, ae_interface_num);
		return ii;
	}

	/*
	 * No iface - create one
	 */
	nii = ecm_db_iface_alloc();
	if (!nii) {
		DEBUG_WARN("Failed to establish iface\n");
		return NULL;
	}

	/*
	 * Add iface into the database, atomically to avoid races creating the same thing
	 */
	spin_lock_bh(&ecm_interface_lock);
	ii = ecm_db_iface_find_and_ref_pppol2tpv2(type_info->l2tp.tunnel.tunnel_id, type_info->l2tp.session.session_id);
	if (ii) {
		spin_unlock_bh(&ecm_interface_lock);
		ecm_db_iface_deref(nii);
		ecm_db_iface_update_ae_interface_identifier(ii, ae_interface_num);
		return ii;
	}

	ecm_db_iface_add_pppol2tpv2(nii, type_info, dev_name, mtu, dev_interface_num, ae_interface_num, NULL, nii);
	spin_unlock_bh(&ecm_interface_lock);

	DEBUG_TRACE("%px: pppol2tpv2 iface established\n", nii);
	return nii;
}

#endif

#ifdef ECM_INTERFACE_PPTP_ENABLE
/*
 * ecm_interface_pptp_interface_establish()
 *	Returns a reference to a iface of the PPTP type, possibly creating one if necessary.
 *	Returns NULL on failure or a reference to interface.
 */
static struct ecm_db_iface_instance *ecm_interface_pptp_interface_establish(struct ecm_db_interface_info_pptp *type_info,
							char *dev_name, int32_t dev_interface_num, int32_t ae_interface_num, int32_t mtu)
{
	struct ecm_db_iface_instance *nii;
	struct ecm_db_iface_instance *ii;

	DEBUG_INFO("Establish PPTP iface: %s with local call id %u peer call id %u\n", dev_name, type_info->src_call_id,
				type_info->dst_call_id);
	/*
	 * Locate the iface
	 */
	ii = ecm_db_iface_find_and_ref_pptp(type_info->src_call_id, type_info->dst_call_id, ae_interface_num);
	if (ii) {
		DEBUG_TRACE("%px: iface established\n", ii);
		ecm_db_iface_update_ae_interface_identifier(ii, ae_interface_num);
		return ii;
	}

	/*
	 * No iface - create one
	 */
	nii = ecm_db_iface_alloc();
	if (!nii) {
		DEBUG_WARN("Failed to establish iface\n");
		return NULL;
	}

	/*
	 * Add iface into the database, atomically to avoid races creating the same thing
	 */
	spin_lock_bh(&ecm_interface_lock);
	ii = ecm_db_iface_find_and_ref_pptp(type_info->src_call_id, type_info->dst_call_id, ae_interface_num);
	if (ii) {
		spin_unlock_bh(&ecm_interface_lock);
		ecm_db_iface_deref(nii);
		ecm_db_iface_update_ae_interface_identifier(ii, ae_interface_num);
		return ii;
	}

	ecm_db_iface_add_pptp(nii, type_info, dev_name, mtu, dev_interface_num, ae_interface_num, NULL, nii);
	spin_unlock_bh(&ecm_interface_lock);

	DEBUG_TRACE("%px: pptp iface established\n", nii);
	return nii;
}
#endif

#ifdef ECM_INTERFACE_GRE_TUN_ENABLE
/*
 * ecm_interface_gre_tun_interface_establish()
 *	Returns a reference to a iface of the gre type, possibly creating one if necessary.
 *	Returns NULL on failure or a reference to interface.
 */
static struct ecm_db_iface_instance *ecm_interface_gre_tun_interface_establish(struct ecm_db_interface_info_gre_tun *type_info,
							char *dev_name, int32_t dev_interface_num, int32_t ae_interface_num, int32_t mtu)
{
	struct ecm_db_iface_instance *nii;
	struct ecm_db_iface_instance *ii;

	DEBUG_TRACE("Establish GRE TUN iface: %s   MTU: %d, if num: %d, accel engine if id: %d\n",
			dev_name, mtu, dev_interface_num, ae_interface_num);

	/*
	 * Locate the iface
	 */
	ii = ecm_db_iface_find_and_ref_gre_tun(type_info->if_index, ae_interface_num);
	if (ii) {
		DEBUG_TRACE("%px: iface established\n", ii);
		return ii;
	}

	/*
	 * No iface - create one
	 */
	nii = ecm_db_iface_alloc();
	if (!nii) {
		DEBUG_WARN("Failed to establish iface\n");
		return NULL;
	}

	/*
	 * Add iface into the database, atomically to avoid races creating the same thing
	 */
	spin_lock_bh(&ecm_interface_lock);
	ii = ecm_db_iface_find_and_ref_gre_tun(type_info->if_index, ae_interface_num);
	if (ii) {
		spin_unlock_bh(&ecm_interface_lock);
		ecm_db_iface_deref(nii);
		return ii;
	}
	ecm_db_iface_add_gre_tun(nii, type_info, dev_name,
			mtu, dev_interface_num, ae_interface_num, NULL, nii);
	spin_unlock_bh(&ecm_interface_lock);

	DEBUG_TRACE("%px: gre iface established\n", nii);
	return nii;
}
#endif

/*
 * ecm_interface_unknown_interface_establish()
 *	Returns a reference to a iface of the UNKNOWN type, possibly creating one if necessary.
 * Returns NULL on failure or a reference to interface.
 */
static struct ecm_db_iface_instance *ecm_interface_unknown_interface_establish(struct ecm_db_interface_info_unknown *type_info,
							char *dev_name, int32_t dev_interface_num, int32_t ae_interface_num, int32_t mtu)
{
	struct ecm_db_iface_instance *nii;
	struct ecm_db_iface_instance *ii;

	DEBUG_INFO("Establish UNKNOWN iface: %s with os_specific_ident: %u, MTU: %d, if num: %d, accel engine if id: %d\n",
			dev_name, type_info->os_specific_ident, mtu, dev_interface_num, ae_interface_num);

	/*
	 * Locate the iface
	 */
	ii = ecm_db_iface_find_and_ref_unknown(type_info->os_specific_ident);
	if (ii) {
		DEBUG_TRACE("%px: iface established\n", ii);
		return ii;
	}

	/*
	 * No iface - create one
	 */
	nii = ecm_db_iface_alloc();
	if (!nii) {
		DEBUG_WARN("Failed to establish iface\n");
		return NULL;
	}

	/*
	 * Add iface into the database, atomically to avoid races creating the same thing
	 */
	spin_lock_bh(&ecm_interface_lock);
	ii = ecm_db_iface_find_and_ref_unknown(type_info->os_specific_ident);
	if (ii) {
		spin_unlock_bh(&ecm_interface_lock);
		ecm_db_iface_deref(nii);
		return ii;
	}
	ecm_db_iface_add_unknown(nii, type_info->os_specific_ident, dev_name,
			mtu, dev_interface_num, ae_interface_num, NULL, nii);
	spin_unlock_bh(&ecm_interface_lock);

	DEBUG_TRACE("%px: unknown iface established\n", nii);
	return nii;
}

/*
 * ecm_interface_loopback_interface_establish()
 *	Returns a reference to a iface of the LOOPBACK type, possibly creating one if necessary.
 * Returns NULL on failure or a reference to interface.
 */
static struct ecm_db_iface_instance *ecm_interface_loopback_interface_establish(struct ecm_db_interface_info_loopback *type_info,
							char *dev_name, int32_t dev_interface_num, int32_t ae_interface_num, int32_t mtu)
{
	struct ecm_db_iface_instance *nii;
	struct ecm_db_iface_instance *ii;

	DEBUG_INFO("Establish LOOPBACK iface: %s with os_specific_ident: %u, MTU: %d, if num: %d, accel engine if id: %d\n",
			dev_name, type_info->os_specific_ident, mtu, dev_interface_num, ae_interface_num);

	/*
	 * Locate the iface
	 */
	ii = ecm_db_iface_find_and_ref_loopback(type_info->os_specific_ident);
	if (ii) {
		DEBUG_TRACE("%px: iface established\n", ii);
		return ii;
	}

	/*
	 * No iface - create one
	 */
	nii = ecm_db_iface_alloc();
	if (!nii) {
		DEBUG_WARN("Failed to establish iface\n");
		return NULL;
	}

	/*
	 * Add iface into the database, atomically to avoid races creating the same thing
	 */
	spin_lock_bh(&ecm_interface_lock);
	ii = ecm_db_iface_find_and_ref_loopback(type_info->os_specific_ident);
	if (ii) {
		spin_unlock_bh(&ecm_interface_lock);
		ecm_db_iface_deref(nii);
		return ii;
	}
	ecm_db_iface_add_loopback(nii, type_info->os_specific_ident, dev_name,
			mtu, dev_interface_num, ae_interface_num, NULL, nii);
	spin_unlock_bh(&ecm_interface_lock);

	DEBUG_TRACE("%px: loopback iface established\n", nii);
	return nii;
}

#ifdef ECM_INTERFACE_IPSEC_ENABLE
/*
 * ecm_interface_ipsec_tunnel_interface_establish()
 *	Returns a reference to a iface of the IPSEC_TUNNEL type, possibly creating one if necessary.
 * Returns NULL on failure or a reference to interface.
 *
 * NOTE: GGG TODO THIS NEEDS TO TAKE A PROPER APPROACH TO IPSEC TUNNELS USING ENDPOINT ADDRESSING AS THE TYPE INFO KEYS
 */
static struct ecm_db_iface_instance *ecm_interface_ipsec_tunnel_interface_establish(struct ecm_db_interface_info_ipsec_tunnel *type_info,
							char *dev_name, int32_t dev_interface_num, int32_t ae_interface_num, int32_t mtu)
{
	struct ecm_db_iface_instance *nii;
	struct ecm_db_iface_instance *ii;

	DEBUG_INFO("Establish IPSEC_TUNNEL iface: %s with os_specific_ident: %u, MTU: %d, if num: %d, accel engine if id: %d\n",
			dev_name, type_info->os_specific_ident, mtu, dev_interface_num, ae_interface_num);

	/*
	 * Locate the iface
	 */
	ii = ecm_db_iface_find_and_ref_ipsec_tunnel(type_info->os_specific_ident, ae_interface_num);
	if (ii) {
		DEBUG_TRACE("%px: iface established\n", ii);
		return ii;
	}

	/*
	 * No iface - create one
	 */
	nii = ecm_db_iface_alloc();
	if (!nii) {
		DEBUG_WARN("Failed to establish iface\n");
		return NULL;
	}

	/*
	 * Add iface into the database, atomically to avoid races creating the same thing
	 */
	spin_lock_bh(&ecm_interface_lock);
	ii = ecm_db_iface_find_and_ref_ipsec_tunnel(type_info->os_specific_ident, ae_interface_num);
	if (ii) {
		spin_unlock_bh(&ecm_interface_lock);
		ecm_db_iface_deref(nii);
		return ii;
	}
	ecm_db_iface_add_ipsec_tunnel(nii, type_info->os_specific_ident, dev_name,
			mtu, dev_interface_num, ae_interface_num, NULL, nii);
	spin_unlock_bh(&ecm_interface_lock);

	DEBUG_TRACE("%px: ipsec_tunnel iface established\n", nii);
	return nii;
}
#endif

#ifdef ECM_INTERFACE_SIT_ENABLE
#ifdef CONFIG_IPV6_SIT_6RD
/*
 * ecm_interface_sit_interface_establish()
 *	Returns a reference to a iface of the SIT type, possibly creating one if necessary.
 * Returns NULL on failure or a reference to interface.
 */
static struct ecm_db_iface_instance *ecm_interface_sit_interface_establish(struct ecm_db_interface_info_sit *type_info,
							char *dev_name, int32_t dev_interface_num, int32_t ae_interface_num, int32_t mtu)
{
	struct ecm_db_iface_instance *nii;
	struct ecm_db_iface_instance *ii;

	DEBUG_INFO("Establish SIT iface: %s with saddr: " ECM_IP_ADDR_OCTAL_FMT ", daddr: " ECM_IP_ADDR_OCTAL_FMT ", MTU: %d, if num: %d, accel engine if id: %d\n",
			dev_name, ECM_IP_ADDR_TO_OCTAL(type_info->saddr), ECM_IP_ADDR_TO_OCTAL(type_info->daddr), mtu, dev_interface_num, ae_interface_num);

	/*
	 * Locate the iface
	 */
	ii = ecm_db_iface_find_and_ref_sit(type_info->saddr, type_info->daddr, ae_interface_num);
	if (ii) {
		DEBUG_TRACE("%px: iface established\n", ii);
		return ii;
	}

	/*
	 * No iface - create one
	 */
	nii = ecm_db_iface_alloc();
	if (!nii) {
		DEBUG_WARN("Failed to establish iface\n");
		return NULL;
	}

	/*
	 * Add iface into the database, atomically to avoid races creating the same thing
	 */
	spin_lock_bh(&ecm_interface_lock);
	ii = ecm_db_iface_find_and_ref_sit(type_info->saddr, type_info->daddr, ae_interface_num);
	if (ii) {
		spin_unlock_bh(&ecm_interface_lock);
		ecm_db_iface_deref(nii);
		return ii;
	}
	ecm_db_iface_add_sit(nii, type_info, dev_name, mtu, dev_interface_num,
			ae_interface_num, NULL, nii);
	spin_unlock_bh(&ecm_interface_lock);

	DEBUG_TRACE("%px: sit iface established\n", nii);
	return nii;
}
#endif
#endif

#ifdef ECM_INTERFACE_TUNIPIP6_ENABLE
#ifdef ECM_IPV6_ENABLE
/*
 * ecm_interface_tunipip6_interface_establish()
 *	Returns a reference to a iface of the TUNIPIP6 type, possibly creating one if necessary.
 * Returns NULL on failure or a reference to interface.
 */
static struct ecm_db_iface_instance *ecm_interface_tunipip6_interface_establish(struct ecm_db_interface_info_tunipip6 *type_info,
							char *dev_name, int32_t dev_interface_num, int32_t ae_interface_num, int32_t mtu)
{
	struct ecm_db_iface_instance *nii;
	struct ecm_db_iface_instance *ii;

	DEBUG_INFO("Establish TUNIPIP6 iface: %s with saddr: " ECM_IP_ADDR_OCTAL_FMT ", daddr: " ECM_IP_ADDR_OCTAL_FMT ", MTU: %d, if num: %d, accel engine if id: %d\n",
			dev_name, ECM_IP_ADDR_TO_OCTAL(type_info->saddr), ECM_IP_ADDR_TO_OCTAL(type_info->daddr), mtu, dev_interface_num, ae_interface_num);

	/*
	 * Locate the iface
	 */
	ii = ecm_db_iface_find_and_ref_tunipip6(type_info->saddr, type_info->daddr, ae_interface_num);
	if (ii) {
		DEBUG_TRACE("%px: iface established\n", ii);
		return ii;
	}

	/*
	 * No iface - create one
	 */
	nii = ecm_db_iface_alloc();
	if (!nii) {
		DEBUG_WARN("Failed to establish iface\n");
		return NULL;
	}

	/*
	 * Add iface into the database, atomically to avoid races creating the same thing
	 */
	spin_lock_bh(&ecm_interface_lock);
	ii = ecm_db_iface_find_and_ref_tunipip6(type_info->saddr, type_info->daddr, ae_interface_num);
	if (ii) {
		spin_unlock_bh(&ecm_interface_lock);
		ecm_db_iface_deref(nii);
		return ii;
	}
	ecm_db_iface_add_tunipip6(nii, type_info, dev_name, mtu, dev_interface_num,
			ae_interface_num, NULL, nii);
	spin_unlock_bh(&ecm_interface_lock);

	DEBUG_TRACE("%px: tunipip6 iface established\n", nii);
	return nii;
}
#endif
#endif

#ifdef ECM_INTERFACE_RAWIP_ENABLE
/*
 * ecm_interface_rawip_interface_establish()
 *	Returns a reference to a iface of the RAWIP type, possibly creating one if necessary.
 * Returns NULL on failure or a reference to interface.
 */
static struct ecm_db_iface_instance *ecm_interface_rawip_interface_establish(struct ecm_db_interface_info_rawip *type_info,
							char *dev_name, int32_t dev_interface_num, int32_t ae_interface_num, int32_t mtu)
{
	struct ecm_db_iface_instance *nii;
	struct ecm_db_iface_instance *ii;

	DEBUG_INFO("Establish RAWIP iface: %s with address: %pM, MTU: %d, if num: %d, accel engine if id: %d\n",
			dev_name, type_info->address, mtu, dev_interface_num, ae_interface_num);

	/*
	 * Locate the iface
	 */
	ii = ecm_db_iface_find_and_ref_rawip(type_info->address);
	if (ii) {
		DEBUG_TRACE("%px: RAWIP iface already established\n", ii);
		return ii;
	}

	/*
	 * No iface - create one
	 */
	nii = ecm_db_iface_alloc();
	if (!nii) {
		DEBUG_WARN("Failed to establish RAWIP iface\n");
		return NULL;
	}

	/*
	 * Add iface into the database, atomically to avoid races creating the same thing
	 */
	spin_lock_bh(&ecm_interface_lock);
	ii = ecm_db_iface_find_and_ref_rawip(type_info->address);
	if (ii) {
		spin_unlock_bh(&ecm_interface_lock);
		ecm_db_iface_deref(nii);
		return ii;
	}
	ecm_db_iface_add_rawip(nii, type_info->address, dev_name,
			mtu, dev_interface_num, ae_interface_num, NULL, nii);
	spin_unlock_bh(&ecm_interface_lock);

	DEBUG_TRACE("%px: RAWIP iface established\n", nii);
	return nii;
}
#endif

#ifdef ECM_INTERFACE_OVPN_ENABLE
/*
 * ecm_interface_ovpn_interface_establish()
 *	Returns reference to iface of the OVPN type.
 */
static struct ecm_db_iface_instance *ecm_interface_ovpn_interface_establish(struct ecm_db_interface_info_ovpn *type_info,
							char *dev_name, int32_t dev_interface_num, int32_t mtu)
{
	struct ecm_db_iface_instance *nii;
	struct ecm_db_iface_instance *ii;

	DEBUG_INFO("Establish OVPN iface: %s with ae_interface_num : %d, MTU: %d, if num: %d\n",
			dev_name, type_info->tun_ifnum, mtu, dev_interface_num);

	/*
	 * Locate the iface
	 */
	ii = ecm_db_iface_find_and_ref_ovpn(type_info->tun_ifnum);
	if (ii) {
		DEBUG_TRACE("%px: iface established\n", ii);
		return ii;
	}

	/*
	 * No iface - create one
	 */
	nii = ecm_db_iface_alloc();
	if (!nii) {
		DEBUG_WARN("Failed to establish iface\n");
		return NULL;
	}

	/*
	 * Add iface into the database, atomically to avoid races creating the same thing
	 */
	spin_lock_bh(&ecm_interface_lock);
	ii = ecm_db_iface_find_and_ref_ovpn(type_info->tun_ifnum);
	if (ii) {
		spin_unlock_bh(&ecm_interface_lock);
		ecm_db_iface_deref(nii);
		return ii;
	}

	ecm_db_iface_add_ovpn(nii, type_info, dev_name, mtu, dev_interface_num, NULL, nii);
	spin_unlock_bh(&ecm_interface_lock);

	DEBUG_TRACE("%px: ovpn iface established\n", nii);
	return nii;
}
#endif

#ifdef ECM_INTERFACE_VXLAN_ENABLE
/*
 * ecm_interface_vxlan_interface_establish()
 *	Returns a reference to a iface of the VxLAN type, possibly creating one if necessary.
 * Returns NULL on failure or a reference to interface.
 */
static struct ecm_db_iface_instance *ecm_interface_vxlan_interface_establish(struct ecm_db_interface_info_vxlan *type_info,
							char *dev_name, int32_t dev_interface_num, int32_t ae_interface_num, int32_t mtu)
{
	struct ecm_db_iface_instance *nii;
	struct ecm_db_iface_instance *ii;

	DEBUG_INFO("Establish VxLAN iface: %s with vxlan id: %u, MTU: %d, if num: %d, if_type: %d, accel engine if id: %d\n",
			dev_name, type_info->vni, mtu, dev_interface_num, type_info->if_type, ae_interface_num);

	/*
	 * Locate the iface
	 */
	ii = ecm_db_iface_find_and_ref_vxlan(type_info->vni, type_info->if_type);
	if (ii) {
		DEBUG_TRACE("%px: vxlan iface established\n", ii);
		/*
		 * Update the accel engine interface identifier, just in case it was changed.
		 */
		ecm_db_iface_update_ae_interface_identifier(ii, ae_interface_num);
		return ii;
	}

	/*
	 * No iface - create one
	 */
	nii = ecm_db_iface_alloc();
	if (!nii) {
		DEBUG_WARN("Failed to establish iface\n");
		return NULL;
	}

	/*
	 * Add iface into the database, atomically to avoid races creating the same thing
	 */
	spin_lock_bh(&ecm_interface_lock);
	ii = ecm_db_iface_find_and_ref_vxlan(type_info->vni, type_info->if_type);
	if (ii) {
		spin_unlock_bh(&ecm_interface_lock);
		ecm_db_iface_deref(nii);
		ecm_db_iface_update_ae_interface_identifier(ii, ae_interface_num);
		DEBUG_TRACE("%px: vxlan iface established\n", ii);
		return ii;
	}
	ecm_db_iface_add_vxlan(nii, type_info->vni, type_info->if_type, dev_name, mtu,
			dev_interface_num, ae_interface_num, NULL, nii);
	spin_unlock_bh(&ecm_interface_lock);

	DEBUG_TRACE("%px: vxlan iface established\n", nii);
	return nii;
}
#endif

/*
 * ecm_interface_tunnel_mtu_update()
 *	Update mtu if the flow is a tunneled packet.
 */
bool ecm_interface_tunnel_mtu_update(ip_addr_t saddr, ip_addr_t daddr, ecm_db_iface_type_t type, int32_t *mtu)
{
	struct net_device *src_dev;
	struct net_device *dest_dev;
	bool ret = true;

	/*
	 * Check if source IP is local address
	 */
	src_dev = ecm_interface_dev_find_by_local_addr(saddr);
	dest_dev = ecm_interface_dev_find_by_local_addr(daddr);

	switch (type) {

	case ECM_DB_IFACE_TYPE_OVPN:
		if (src_dev) {
			*mtu = src_dev->mtu;
		} else if (dest_dev) {
			*mtu = dest_dev->mtu;
		} else {
			return false;
		}
		break;

	case ECM_DB_IFACE_TYPE_PPTP:
	case ECM_DB_IFACE_TYPE_PPPOL2TPV2:
	case ECM_DB_IFACE_TYPE_GRE_TUN:
	case ECM_DB_IFACE_TYPE_GRE_TAP:
	case ECM_DB_IFACE_TYPE_VXLAN:
	case ECM_DB_IFACE_TYPE_IPSEC_TUNNEL:
		if (src_dev) {
			*mtu = src_dev->mtu;
		} else {
			ret = false;
			goto done;
		}
		break;

	default:
		ret = false;
		DEBUG_WARN("Tunnel type doesn't need to update MTU value\n");
		break;
	}

done:
	if (src_dev) {
		dev_put(src_dev);
	}

	if (dest_dev) {
		dev_put(dest_dev);
	}

	return ret;
}

#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
/*
 * ecm_interface_ovs_bridge_port_dev_get_and_ref()
 *	Looks up the slave port in the bridge devices port list.
 */
static struct net_device *ecm_interface_ovs_bridge_port_dev_get_and_ref(struct sk_buff *skb, struct net_device *br_dev,
								ip_addr_t src_ip, ip_addr_t dst_ip, int ip_version,
								int protocol, bool is_routed, uint8_t *smac,
								uint8_t *dmac, __be16 *layer4hdr,
								struct ecm_front_end_ovs_params *op)
{
	struct ovsmgr_dp_flow flow;
	struct ovsmgr_dp_flow return_flow;
	struct net_device *dev;

	memset(&flow, 0, sizeof(flow));

	flow.indev = br_dev;
	flow.outdev = NULL;
	flow.tuple.ip_version = ip_version;
	flow.tuple.protocol = protocol;
	flow.is_routed = is_routed;

	if (!smac) {
		ether_addr_copy(flow.smac, br_dev->dev_addr);
	}

	ether_addr_copy(flow.dmac, dmac);

	/*
	 * Consider a routing flow
	 * eth1-ovsbr1----->ovsbr2-eth2
	 * The 2 ovs data path rules should look like the following,
	 * 1. ingress port:eth1, egress_port:ovsbr1
	 * 2. ingress_port:ovsbr2, egress_port:eth2
	 *
	 * Copy the multicast mac address, if src_ip is multicast.
	 * During multicast 'from' hierarchy creation, the ECM
	 * copies the source MAC as multicast MAC as the
	 * reverse direction rule is not present in the ovs
	 * data path rule set.
	 */
	if (ecm_ip_addr_is_multicast(src_ip)) {
		struct ethhdr *skb_eth_hdr;

		skb_eth_hdr = eth_hdr(skb);
		ether_addr_copy(flow.smac, dmac);
		ether_addr_copy(flow.dmac, skb_eth_hdr->h_dest);

		if (protocol == IPPROTO_UDP) {
			struct udphdr *udp_hdr = (struct udphdr *)layer4hdr;

			flow.tuple.src_port = udp_hdr->source;
			flow.tuple.dst_port = udp_hdr->dest;
		} else {
			DEBUG_WARN("%px: Protocol is not UDP\n", skb);
			return NULL;
		}

		if (ip_version == 4) {
			ECM_IP_ADDR_TO_NIN4_ADDR(flow.tuple.ipv4.src, dst_ip);
			ECM_IP_ADDR_TO_NIN4_ADDR(flow.tuple.ipv4.dst, src_ip);
			DEBUG_TRACE("%px: br_dev: %s, src_addr: %pI4, dest_addr: %pI4, ip_version: %d, protocol: %d (sp:%d, dp:%d) (smac:%pM, dmac:%pM)\n",
					skb, br_dev->name, &flow.tuple.ipv4.src, &flow.tuple.ipv4.dst,
					ip_version, protocol, flow.tuple.src_port, flow.tuple.dst_port, flow.smac, flow.dmac);
		} else {
			ECM_IP_ADDR_TO_NIN6_ADDR(flow.tuple.ipv6.src, dst_ip);
			ECM_IP_ADDR_TO_NIN6_ADDR(flow.tuple.ipv6.dst, src_ip);
			DEBUG_TRACE("%px: br_dev: %s, src_addr: %pI6, dest_addr: %pI6, ip_version: %d, protocol: %d (sp:%d, dp:%d) (smac:%pM, dmac:%pM)\n",
					skb, br_dev->name, &flow.tuple.ipv6.src, &flow.tuple.ipv6.dst,
					ip_version, protocol, flow.tuple.src_port, flow.tuple.dst_port, flow.smac, flow.dmac);
		}

		goto port_find;
	}

	/*
	 * OVS parameters are not passed explicitly for the following cases:
	 * 1. IPv6 flows
	 * 2. IPv4/IPv6 non-ported flows
	 * 3. Multicast flows.
	 * 4. SFE flows
	 */
	if (!op) {
		if (protocol == IPPROTO_TCP) {
			struct tcphdr *tcp_hdr = (struct tcphdr *)layer4hdr;

			flow.tuple.src_port = tcp_hdr->source;
			flow.tuple.dst_port = tcp_hdr->dest;
		} else if (protocol == IPPROTO_UDP) {
			struct udphdr *udp_hdr = (struct udphdr *)layer4hdr;

			flow.tuple.src_port = udp_hdr->source;
			flow.tuple.dst_port = udp_hdr->dest;
		} else if (protocol == IPPROTO_GRE) {
			DEBUG_TRACE("%px: Protocol is GRE\n", skb);
		} else {
			DEBUG_WARN("%px: Protocol is not udp/tcp/gre\n", skb);
			return NULL;
		}

		if (ip_version == 4) {
			ECM_IP_ADDR_TO_NIN4_ADDR(flow.tuple.ipv4.src, src_ip);
			ECM_IP_ADDR_TO_NIN4_ADDR(flow.tuple.ipv4.dst, dst_ip);
			DEBUG_TRACE("%px: br_dev: %s, src_addr: %pI4, dest_addr: %pI4, ip_version: %d, protocol: %d (sp:%d, dp:%d) (smac:%pM, dmac:%pM)\n",
					skb, br_dev->name, &flow.tuple.ipv4.src, &flow.tuple.ipv4.dst,
					ip_version, protocol, flow.tuple.src_port, flow.tuple.dst_port, flow.smac, flow.dmac);
		} else {
			ECM_IP_ADDR_TO_NIN6_ADDR(flow.tuple.ipv6.src, src_ip);
			ECM_IP_ADDR_TO_NIN6_ADDR(flow.tuple.ipv6.dst, dst_ip);
			DEBUG_TRACE("%px: br_dev: %s, src_addr: %pI6, dest_addr: %pI6, ip_version: %d, protocol: %d (sp:%d, dp:%d) (smac:%pM, dmac:%pM)\n",
					skb, br_dev->name, &flow.tuple.ipv6.src, &flow.tuple.ipv6.dst,
					ip_version, protocol, flow.tuple.src_port, flow.tuple.dst_port, flow.smac, flow.dmac);
		}

		goto port_find;
	}

	/*
	 * We use OVS params for IPv4 NSS unicast flows.
	 */
	if ((protocol == IPPROTO_TCP) || (protocol == IPPROTO_UDP)) {
		flow.tuple.src_port = htons(op->src_port);
		flow.tuple.dst_port = htons(op->dest_port);
	} else {
		DEBUG_WARN("%px: Protocol is not udp/tcp\n", skb);
		return NULL;
	}

	if (ip_version == 4) {
		ECM_IP_ADDR_TO_NIN4_ADDR(flow.tuple.ipv4.src, op->src_ip);
		ECM_IP_ADDR_TO_NIN4_ADDR(flow.tuple.ipv4.dst, op->dest_ip);
		DEBUG_TRACE("%px: br_dev: %s, src_addr: %pI4, dest_addr: %pI4, ip_version: %d, protocol: %d (sp:%d, dp:%d) (smac:%pM, dmac:%pM)\n",
				skb, br_dev->name, &flow.tuple.ipv4.src, &flow.tuple.ipv4.dst,
				ip_version, protocol, flow.tuple.src_port, flow.tuple.dst_port, flow.smac, flow.dmac);
	} else {
		ECM_IP_ADDR_TO_NIN6_ADDR(flow.tuple.ipv6.src, op->src_ip);
		ECM_IP_ADDR_TO_NIN6_ADDR(flow.tuple.ipv6.dst, op->dest_ip);
		DEBUG_TRACE("%px: br_dev: %s, src_addr: %pI6, dest_addr: %pI6, ip_version: %d, protocol: %d (sp:%d, dp:%d) (smac:%pM, dmac:%pM)\n",
				skb, br_dev->name, &flow.tuple.ipv6.src, &flow.tuple.ipv6.dst,
				ip_version, protocol, flow.tuple.src_port, flow.tuple.dst_port, flow.smac, flow.dmac);
	}

port_find:
	dev = ovsmgr_port_find(skb, br_dev, &flow);
	if (dev) {
		DEBUG_TRACE("OVS egress port dev: %s\n", dev->name);
		return dev;
	}

	/*
	 * Handle Multicast flows separately.
	 */
	if (ecm_ip_addr_is_multicast(src_ip)) {
		dev = ovsmgr_port_find_by_mac(skb, br_dev, &flow);
		if (!dev) {
			DEBUG_WARN("%px: Couldn't find OVS bridge port for Multicast flow.\n", skb);
			return NULL;
		}

		return dev;
	}

	/*
	 * Find by MAC addresses using return flow
	 */
	return_flow.indev = NULL;
	return_flow.outdev = br_dev;
	return_flow.tuple.ip_version = flow.tuple.ip_version;
	return_flow.tuple.protocol = flow.tuple.protocol;
	return_flow.is_routed = flow.is_routed;

	ether_addr_copy(return_flow.smac, flow.dmac);
	ether_addr_copy(return_flow.dmac, flow.smac);
	return_flow.tuple.src_port = flow.tuple.dst_port;
	return_flow.tuple.dst_port = flow.tuple.src_port;
	if (ip_version == 4) {
		return_flow.tuple.ipv4.src = flow.tuple.ipv4.dst;
		return_flow.tuple.ipv4.dst = flow.tuple.ipv4.src;
		DEBUG_TRACE("%px: br_dev = %s, src_addr: %pI4, dest_addr: %pI4, ip_version: %d, protocol: %d (sp:%d, dp:%d) (smac:%pM, dmac:%pM)\n",
				skb, br_dev->name, &return_flow.tuple.ipv4.src,
				&return_flow.tuple.ipv4.dst, return_flow.tuple.ip_version,
				return_flow.tuple.protocol, return_flow.tuple.src_port, return_flow.tuple.dst_port,
				return_flow.smac, return_flow.dmac);
	} else {
		memcpy(&return_flow.tuple.ipv6.src, &flow.tuple.ipv6.dst, sizeof(return_flow.tuple.ipv6.src));
		memcpy(&return_flow.tuple.ipv6.dst, &flow.tuple.ipv6.src, sizeof(return_flow.tuple.ipv6.dst));
		DEBUG_TRACE("%px: br_dev = %s, src_addr: %pI6, dest_addr: %pI6, ip_version: %d, protocol: %d (sp:%d, dp:%d) (smac:%pM, dmac:%pM)\n",
				skb, br_dev->name, &return_flow.tuple.ipv4.src,
				&return_flow.tuple.ipv4.dst, return_flow.tuple.ip_version,
				return_flow.tuple.protocol, return_flow.tuple.src_port, return_flow.tuple.dst_port,
				return_flow.smac, return_flow.dmac);
	}

	dev = ovsmgr_port_find_by_mac(skb, br_dev, &return_flow);
	if (!dev) {
		DEBUG_WARN("%px: Couldn't find OVS bridge port\n", skb);
		return NULL;
	}

	return dev;
}
#endif

#ifdef ECM_INTERFACE_MACVLAN_ENABLE
/*
 * ecm_interface_macvlan_mode_is_valid()
 *	Check if the macvlan interface allowed for acceleration.
 */
static bool ecm_interface_macvlan_mode_is_valid(struct net_device *dev)
{
	enum macvlan_mode mode = macvlan_get_mode(dev);

	/*
	 * Allow acceleration for only "Private" mode.
	 */
	if (mode == MACVLAN_MODE_PRIVATE) {
		return true;
	}

	DEBUG_WARN("%px: MACVLAN dev: %s, MACVLAN mode: %d is not supported for acceleration\n", dev,
			dev->name, mode);
	return false;
}
#endif

/*
 * ecm_interface_establish_and_ref()
 *	Establish an interface instance for the given interface detail.
 */
struct ecm_db_iface_instance *ecm_interface_establish_and_ref(struct ecm_front_end_connection_instance *feci,
								struct net_device *dev, struct sk_buff *skb)
{
	int32_t dev_interface_num;
	char *dev_name;
	int32_t dev_type;
	int32_t dev_mtu;
	int32_t ae_interface_num;
	struct ecm_db_iface_instance *ii;
	int32_t interface_type __attribute__((unused));
	union {
		struct ecm_db_interface_info_ethernet ethernet;		/* type == ECM_DB_IFACE_TYPE_ETHERNET */
#ifdef ECM_INTERFACE_VLAN_ENABLE
		struct ecm_db_interface_info_vlan vlan;			/* type == ECM_DB_IFACE_TYPE_VLAN */
#endif
#ifdef ECM_INTERFACE_MACVLAN_ENABLE
		struct ecm_db_interface_info_macvlan macvlan;		/* type == ECM_DB_IFACE_TYPE_MACVLAN */
#endif
#ifdef ECM_INTERFACE_BOND_ENABLE
		struct ecm_db_interface_info_lag lag;			/* type == ECM_DB_IFACE_TYPE_LAG */
#endif
		struct ecm_db_interface_info_bridge bridge;		/* type == ECM_DB_IFACE_TYPE_BRIDGE */
#ifdef ECM_INTERFACE_PPPOE_ENABLE
		struct ecm_db_interface_info_pppoe pppoe;		/* type == ECM_DB_IFACE_TYPE_PPPOE */
#endif
#ifdef ECM_INTERFACE_L2TPV2_ENABLE
		struct ecm_db_interface_info_pppol2tpv2 pppol2tpv2;		/* type == ECM_DB_IFACE_TYPE_PPPOL2TPV2 */
#endif
#ifdef ECM_INTERFACE_PPTP_ENABLE
		struct ecm_db_interface_info_pptp pptp;			/* type == ECM_DB_IFACE_TYPE_PPTP */
#endif
#ifdef ECM_INTERFACE_MAP_T_ENABLE
		struct ecm_db_interface_info_map_t map_t;		/* type == ECM_DB_IFACE_TYPE_MAP_T */
#endif
#ifdef ECM_INTERFACE_GRE_TUN_ENABLE
		struct ecm_db_interface_info_gre_tun gre_tun;			/* type == ECM_DB_IFACE_TYPE_GRE */
#endif
		struct ecm_db_interface_info_unknown unknown;		/* type == ECM_DB_IFACE_TYPE_UNKNOWN */
		struct ecm_db_interface_info_loopback loopback;		/* type == ECM_DB_IFACE_TYPE_LOOPBACK */
#ifdef ECM_INTERFACE_IPSEC_ENABLE
		struct ecm_db_interface_info_ipsec_tunnel ipsec_tunnel;	/* type == ECM_DB_IFACE_TYPE_IPSEC_TUNNEL */
#endif
#ifdef ECM_INTERFACE_SIT_ENABLE
		struct ecm_db_interface_info_sit sit;			/* type == ECM_DB_IFACE_TYPE_SIT */
#endif
#ifdef ECM_INTERFACE_TUNIPIP6_ENABLE
#ifdef ECM_IPV6_ENABLE
		struct ecm_db_interface_info_tunipip6 tunipip6;		/* type == ECM_DB_IFACE_TYPE_TUNIPIP6 */
#endif
#endif
#ifdef ECM_INTERFACE_RAWIP_ENABLE
		struct ecm_db_interface_info_rawip rawip;		/* type == ECM_DB_IFACE_TYPE_RAWIP */
#endif
#ifdef ECM_INTERFACE_OVPN_ENABLE
		struct ecm_db_interface_info_ovpn ovpn;			/* type == ECM_DB_IFACE_TYPE_OVPN */
#endif
#ifdef ECM_INTERFACE_VXLAN_ENABLE
		struct ecm_db_interface_info_vxlan vxlan;		/* type == ECM_DB_IFACE_TYPE_VXLAN */
#endif
#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
		struct ecm_db_interface_info_ovs_bridge ovsb;		/* type == ECM_DB_IFACE_TYPE_OVS_BRIDGE */
#endif
	} type_info;

#ifdef ECM_INTERFACE_GRE_TUN_ENABLE
	struct ip_tunnel *gre4_tunnel;
	struct ip6_tnl *gre6_tunnel;
#endif
#ifdef ECM_INTERFACE_PPP_ENABLE
	int channel_count;
	struct ppp_channel *ppp_chan[1];
	int channel_protocol;
#ifdef ECM_INTERFACE_PPTP_ENABLE
	int protocol = IPPROTO_IP;
	struct pptp_opt opt;
	struct iphdr *v4_hdr = NULL;
	if (skb) {
		v4_hdr = ip_hdr(skb);
		protocol = v4_hdr->protocol;
	}
#endif
#endif
	/*
	 * Get basic information about the given device
	 */
	dev_interface_num = dev->ifindex;
	dev_name = dev->name;
	dev_type = dev->type;
	dev_mtu = dev->mtu;

	/*
	 * Does the accel engine recognise this interface?
	 */
	ae_interface_num = feci->ae_interface_number_by_dev_get(dev);

	DEBUG_TRACE("%px: Establish interface instance for device: %px is type: %d, name: %s, ifindex: %d, ae_if: %d, mtu: %d\n",
			feci, dev, dev_type, dev_name, dev_interface_num, ae_interface_num, dev_mtu);

	/*
	 * Extract from the device more type-specific information
	 */
	if (dev_type == ARPHRD_ETHER) {

		/*
		 * If MAC address is zeros, do nothing.
		 */
		if (is_zero_ether_addr(dev->dev_addr)) {
			DEBUG_WARN("%px: Net device %px MAC address is all zeros\n", feci, dev);
			return NULL;
		}

		/*
		 * Ethernet - but what sub type?
		 */

#ifdef ECM_INTERFACE_VLAN_ENABLE
		/*
		 * VLAN?
		 */
		if (is_vlan_dev(dev)) {
			/*
			 * VLAN master
			 * GGG No locking needed here, ASSUMPTION is that real_dev is held for as long as we have dev.
			 */
			ether_addr_copy(type_info.vlan.address, dev->dev_addr);
			type_info.vlan.vlan_tag = vlan_dev_vlan_id(dev);
			type_info.vlan.vlan_tpid = ntohs(vlan_dev_vlan_proto(dev));
			DEBUG_TRACE("%px: Net device: %px is VLAN, mac: %pM, vlan_id: %x vlan_tpid: %x\n",
					feci, dev, type_info.vlan.address, type_info.vlan.vlan_tag, type_info.vlan.vlan_tpid);

			/*
			 * Establish this type of interface
			 */
			ii = ecm_interface_vlan_interface_establish(&type_info.vlan, dev_name, dev_interface_num, ae_interface_num, dev_mtu);
			goto identifier_update;
		}
#endif

#ifdef ECM_INTERFACE_MACVLAN_ENABLE
		/*
		 * MACVLAN?
		 */
		if (netif_is_macvlan(dev)) {
			if (ecm_interface_macvlan_mode_is_valid(dev)) {
				ether_addr_copy(type_info.macvlan.address, dev->dev_addr);
				DEBUG_TRACE("%px: Net device: %px is MACVLAN, mac: %pM\n",
						feci, dev, type_info.macvlan.address);

				/*
				 * Establish this type of interface
				 */
				ii = ecm_interface_macvlan_interface_establish(&type_info.macvlan, dev_name, dev_interface_num, ae_interface_num, dev_mtu);
				goto identifier_update;
			}

			DEBUG_WARN("%px: Net device %px MACVLAN mode is not supported.\n", feci, dev);
			return NULL;
		}
#endif

#ifdef ECM_INTERFACE_VXLAN_ENABLE
		/*
		 * VxLAN?
		 */
		if (netif_is_vxlan(dev)) {
			u32 vni;
			struct vxlan_dev *vxlan_tun;
			ip_addr_t vxlan_saddr, vxlan_daddr;

			/*
			 * VxLAN
			 */
			vxlan_tun = netdev_priv(dev);
			vni = vxlan_get_vni(vxlan_tun);
			DEBUG_TRACE("%px: Net device: %px is VxLAN, mac: %pM, vni: %d\n",
					feci, dev, dev->dev_addr, vni);
			interface_type = ecm_interface_vxlan_type_get(skb, vxlan_tun);
			if (interface_type < 0) {
				DEBUG_TRACE("%p: VxLAN tunnel direction cannot be found: %d\n", feci, interface_type);
				return NULL;
			}

			ae_interface_num = feci->ae_interface_number_by_dev_type_get(dev, interface_type);
			DEBUG_TRACE("%px: VxLAN netdevice interface ae_interface_num: %d, interface_type: %d\n",
					feci, ae_interface_num, interface_type);

			type_info.vxlan.vni = vni;
			type_info.vxlan.if_type = interface_type;

			/*
			 * Copy IP addresses from skb
			 */
			if (ip_hdr(skb)->version == IPVERSION) {
				ECM_NIN4_ADDR_TO_IP_ADDR(vxlan_saddr, ip_hdr(skb)->saddr);
				ECM_NIN4_ADDR_TO_IP_ADDR(vxlan_daddr, ip_hdr(skb)->daddr);
			} else {
				ECM_NIN6_ADDR_TO_IP_ADDR(vxlan_saddr, ipv6_hdr(skb)->saddr);
				ECM_NIN6_ADDR_TO_IP_ADDR(vxlan_daddr, ipv6_hdr(skb)->daddr);
			}

			if (ecm_interface_tunnel_mtu_update(vxlan_saddr, vxlan_daddr,
						ECM_DB_IFACE_TYPE_VXLAN, &dev_mtu)) {
				DEBUG_TRACE("%px: VxLAN netdevice mtu updated: %d\n", feci, dev_mtu);
			}

			/*
			 * Establish this type of interface
			 */
			ii = ecm_interface_vxlan_interface_establish(&type_info.vxlan, dev_name, dev_interface_num, ae_interface_num, dev_mtu);
			goto identifier_update;
		}
#endif

		/*
		 * BRIDGE?
		 */
		if (ecm_front_end_is_bridge_device(dev)) {
			/*
			 * Bridge
			 */
			ether_addr_copy(type_info.bridge.address, dev->dev_addr);

			DEBUG_TRACE("%px: Net device: %px is BRIDGE, mac: %pM\n",
					feci, dev, type_info.bridge.address);

			/*
			 * Establish this type of interface
			 */
			ii = ecm_interface_bridge_interface_establish(&type_info.bridge, dev_name, dev_interface_num, ae_interface_num, dev_mtu);
			goto identifier_update;
		}

		/*
		 * OVS BRIDGE?
		 */
#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
		if (ovsmgr_is_ovs_master(dev)) {
			/*
			 * OVS Bridge
			 */
			ether_addr_copy(type_info.ovsb.address, dev->dev_addr);

			DEBUG_TRACE("%px: Net device: %px is OVS BRIDGE, mac: %pM\n",
					feci, dev, type_info.ovsb.address);

			/*
			 * Establish this type of interface
			 */
			ii = ecm_interface_ovs_bridge_interface_establish(&type_info.ovsb, dev_name, dev_interface_num, ae_interface_num, dev_mtu);
			goto identifier_update;
		}
#endif

#ifdef ECM_INTERFACE_BOND_ENABLE
		/*
		 * LAG?
		 */
		if (ecm_front_end_is_lag_master(dev)) {
			/*
			 * Link aggregation
			 */
			ether_addr_copy(type_info.lag.address, dev->dev_addr);

			DEBUG_TRACE("%px: Net device: %px is LAG, mac: %pM\n",
					feci, dev, type_info.lag.address);

			/*
			 * Establish this type of interface
			 */
			ii = ecm_interface_lag_interface_establish(&type_info.lag, dev_name, dev_interface_num, ae_interface_num, dev_mtu);
			goto identifier_update;
		}
#endif

#ifdef ECM_INTERFACE_GRE_TAP_ENABLE
		/*
		 * GRE TAP?
		 */
		if (dev->priv_flags_ext & (IFF_EXT_GRE_V4_TAP | IFF_EXT_GRE_V6_TAP)) {
			interface_type = feci->ae_interface_type_get(feci, dev);
			ae_interface_num = feci->ae_interface_number_by_dev_type_get(dev, interface_type);

			/*
			 * GRE TAP interface is handled as ethernet interface, however it is possible
			 * that the acceleration engine may not be ready yet to handle the connection.
			 * In this case the acceleration engine interface is not found for this type and
			 * we should wait until it is ready.
			 */
			if (ae_interface_num < 0) {
				DEBUG_TRACE("%px: GRE TAP interface is not ready yet. Interface type: %d\n", feci, interface_type);
				return NULL;
			}
		}
#endif
		/*
		 * ETHERNET!
		 * Just plain ethernet it seems
		 */
		ether_addr_copy(type_info.ethernet.address, dev->dev_addr);
		DEBUG_TRACE("%px: Net device: %px is ETHERNET, mac: %pM\n",
				feci, dev, type_info.ethernet.address);

		/*
		 * Establish this type of interface
		 */
		ii = ecm_interface_ethernet_interface_establish(&type_info.ethernet, dev_name, dev_interface_num, ae_interface_num, dev_mtu);

identifier_update:
		if (ii) {
			/*
			 * An interface identifier/ifindex can be change after network restart. Below
			 * functtion will check interface_identifier present in 'ii' with new dev_interface_num.
			 * If differ then update new ifindex and update the interface identifier hash table.
			 */
			ecm_db_iface_identifier_hash_table_entry_check_and_update(ii, dev_interface_num);
		}

		return ii;
	}

	/*
	 * LOOPBACK?
	 */
	if (dev_type == ARPHRD_LOOPBACK) {
		DEBUG_TRACE("%px: Net device: %px is LOOPBACK type: %d\n", feci, dev, dev_type);
		type_info.loopback.os_specific_ident = dev_interface_num;
		ii = ecm_interface_loopback_interface_establish(&type_info.loopback, dev_name, dev_interface_num, ae_interface_num, dev_mtu);
		return ii;
	}

#ifdef ECM_INTERFACE_IPSEC_ENABLE
	/*
	 * IPSEC?
	 */
	if (dev_type == ECM_ARPHRD_IPSEC_TUNNEL_TYPE) {

#ifdef ECM_INTERFACE_IPSEC_GLUE_LAYER_SUPPORT_ENABLE
		struct net_device *ipsec_dev;
		ip_addr_t saddr, daddr;

		DEBUG_TRACE("Net device: %px is IPSec tunnel type: %d\n", dev, dev_type);

		ipsec_dev = ecm_interface_get_and_hold_ipsec_tun_netdev(dev, skb, &interface_type);
		if (!ipsec_dev) {
			DEBUG_WARN("Failed to find NSS IPSec dev for: %s and type: %d\n", dev->name, dev_type);
			return NULL;
		}

		ae_interface_num = feci->ae_interface_number_by_dev_type_get(ipsec_dev, interface_type);
		if (ae_interface_num < 0) {
			DEBUG_TRACE("IPSec interface %s is not ready yet\n", ipsec_dev->name);
			dev_put(ipsec_dev);
			return NULL;
		}

		DEBUG_TRACE("Obtained IPSec device is %s, and it's interface num is %d\n", ipsec_dev->name,
				ae_interface_num);
		dev_put(ipsec_dev);

		/*
		 * Copy IP addresses from skb
		 */
		if (ip_hdr(skb)->version == IPVERSION) {
			ECM_NIN4_ADDR_TO_IP_ADDR(saddr, ip_hdr(skb)->saddr);
			ECM_NIN4_ADDR_TO_IP_ADDR(daddr, ip_hdr(skb)->daddr);
		} else {
			ECM_NIN6_ADDR_TO_IP_ADDR(saddr, ipv6_hdr(skb)->saddr);
			ECM_NIN6_ADDR_TO_IP_ADDR(daddr, ipv6_hdr(skb)->daddr);
		}

		ecm_interface_tunnel_mtu_update(saddr, daddr, ECM_DB_IFACE_TYPE_IPSEC_TUNNEL, &dev_mtu);
#endif
		type_info.ipsec_tunnel.os_specific_ident = dev_interface_num;

		/*
		 * Override the MTU size in the decap direction in case of IPSec tunnel.
		 * This will apply to IPsec->WAN rule.
		 * TODO: Move this override to accelerate function.
		 */
#ifdef ECM_XFRM_ENABLE
		if (ip_hdr(skb)->version == IPVERSION) {
			if ((ip_hdr(skb)->protocol == IPPROTO_ESP) ||
			    ((ip_hdr(skb)->protocol == IPPROTO_UDP) &&
			     (IPCB(skb)->flags & IPSKB_XFRM_TRANSFORMED))) {
				dev_mtu = ECM_DB_IFACE_MTU_MAX;
			}
		} else {
			if (ipv6_hdr(skb)->nexthdr == IPPROTO_ESP) {
				dev_mtu = ECM_DB_IFACE_MTU_MAX;
			}
		}
#else
		if (ip_hdr(skb)->version == IPVERSION) {
			if ((ip_hdr(skb)->protocol == IPPROTO_ESP) ||
			    ((ip_hdr(skb)->protocol == IPPROTO_UDP) &&
			     (udp_hdr(skb)->dest == htons(4500)))) {
				dev_mtu = ECM_DB_IFACE_MTU_MAX;
			}
		} else {
			if (ipv6_hdr(skb)->nexthdr == IPPROTO_ESP) {
				dev_mtu = ECM_DB_IFACE_MTU_MAX;
			}
		}
#endif

		ii = ecm_interface_ipsec_tunnel_interface_establish(&type_info.ipsec_tunnel, dev_name, dev_interface_num, ae_interface_num, dev_mtu);
		if (ii) {
			ecm_db_iface_identifier_hash_table_entry_check_and_update(ii, dev_interface_num);
		}
		return ii;
	}
#endif

#ifdef ECM_INTERFACE_MAP_T_ENABLE
	if (dev_type == ARPHRD_NONE) {
		if (is_map_t_dev(dev)) {
			type_info.map_t.if_index = dev_interface_num;
			interface_type = feci->ae_interface_type_get(feci, dev);
			ae_interface_num = feci->ae_interface_number_by_dev_type_get(dev, interface_type);

			if (ae_interface_num < 0) {
				DEBUG_TRACE("%px: MAP-T interface is not ready yet\n", feci);
				return NULL;
			}

			ii = ecm_interface_map_t_interface_establish(&type_info.map_t, dev_name, dev_interface_num, ae_interface_num, dev_mtu);
			return ii;
		}

	}
#endif

#ifdef ECM_INTERFACE_SIT_ENABLE
#ifdef CONFIG_IPV6_SIT_6RD
	/*
	 * SIT (6-in-4)?
	 */
	if (dev_type == ARPHRD_SIT) {
		struct ip_tunnel *tunnel;
		struct ip_tunnel_6rd_parm *ip6rd;
		const struct iphdr  *tiph;

		DEBUG_TRACE("%px: Net device: %px is SIT (6-in-4) type: %d\n", feci, dev, dev_type);

		tunnel = (struct ip_tunnel*)netdev_priv(dev);
		ip6rd =  &tunnel->ip6rd;

		/*
		 * Get the Tunnel device IP header info
		 */
		tiph = &tunnel->parms.iph ;

		ECM_NIN4_ADDR_TO_IP_ADDR(type_info.sit.saddr, tiph->saddr);
		ECM_NIN4_ADDR_TO_IP_ADDR(type_info.sit.daddr, tiph->daddr);
		type_info.sit.ttl = tiph->ttl;
		type_info.sit.tos = tiph->tos;

		interface_type = feci->ae_interface_type_get(feci, dev);
		ae_interface_num = feci->ae_interface_number_by_dev_type_get(dev, interface_type);

		ii = ecm_interface_sit_interface_establish(&type_info.sit, dev_name, dev_interface_num, ae_interface_num, dev_mtu);
		return ii;
	}
#endif
#endif

#ifdef ECM_INTERFACE_TUNIPIP6_ENABLE
#ifdef ECM_IPV6_ENABLE
	/*
	 * IPIP6 Tunnel?
	 */
	if (dev_type == ARPHRD_TUNNEL6) {
		struct ip6_tnl *tunnel;
		struct flowi6 *fl6;

		DEBUG_TRACE("%px: Net device: %px is TUNIPIP6 type: %d\n", feci, dev, dev_type);

		/*
		 * Get the tunnel device flow information (discover the output path of the tunnel)
		 */
		tunnel = (struct ip6_tnl *)netdev_priv(dev);
		fl6 = &tunnel->fl.u.ip6;

		ECM_NIN6_ADDR_TO_IP_ADDR(type_info.tunipip6.saddr, fl6->saddr);
		ECM_NIN6_ADDR_TO_IP_ADDR(type_info.tunipip6.daddr, fl6->daddr);
		type_info.tunipip6.hop_limit = tunnel->parms.hop_limit;
		type_info.tunipip6.flags = ntohl(tunnel->parms.flags);
		type_info.tunipip6.flowlabel = fl6->flowlabel;  /* flow Label In kernel is stored in big endian format */

		interface_type = feci->ae_interface_type_get(feci, dev);
		ae_interface_num = feci->ae_interface_number_by_dev_type_get(dev, interface_type);

		if (ae_interface_num < 0) {
			DEBUG_TRACE("%px: TUNIPIP6 interface is not ready yet\n", feci);
			return NULL;
		}

		ii = ecm_interface_tunipip6_interface_establish(&type_info.tunipip6, dev_name,
								dev_interface_num, ae_interface_num, dev_mtu);
		return ii;
	}
#endif
#endif

#ifdef ECM_INTERFACE_GRE_TUN_ENABLE
	if ((dev_type == ARPHRD_IPGRE) || (dev_type == ARPHRD_IP6GRE)) {
		type_info.gre_tun.if_index = dev_interface_num;
		if (dev_type == ARPHRD_IPGRE) {
			gre4_tunnel = netdev_priv(dev);
			if (!gre4_tunnel) {
				DEBUG_WARN("%px: failed to obtain node address for host. GREv4 tunnel not ready\n", feci);
				return NULL;
			}
			ECM_NIN4_ADDR_TO_IP_ADDR(type_info.gre_tun.local_ip, gre4_tunnel->parms.iph.saddr);
			ECM_NIN4_ADDR_TO_IP_ADDR(type_info.gre_tun.remote_ip, gre4_tunnel->parms.iph.daddr);
		} else {
			gre6_tunnel = netdev_priv(dev);
			if (!gre6_tunnel) {
				DEBUG_WARN("%px: failed to obtain node address for host. GREv6 tunnel not ready\n", feci);
				return NULL;
			}
			ECM_NIN6_ADDR_TO_IP_ADDR(type_info.gre_tun.local_ip, gre6_tunnel->parms.laddr);
			ECM_NIN6_ADDR_TO_IP_ADDR(type_info.gre_tun.remote_ip, gre6_tunnel->parms.raddr);
		}

		interface_type = feci->ae_interface_type_get(feci, dev);
		ae_interface_num = feci->ae_interface_number_by_dev_type_get(dev, interface_type);
		if (ae_interface_num < 0) {
			DEBUG_TRACE("%px: GRE TUN interface is not ready yet. Interface type: %d\n", feci, interface_type);
			return NULL;
		}

		ii = ecm_interface_gre_tun_interface_establish(&type_info.gre_tun, dev_name, dev_interface_num, ae_interface_num, dev_mtu);
		if (ii) {
			/*
			 * The ifindex of a virtual netdevice like a GRE tunnel session can change if it is destroyed
			 * and comes up again. Detect if the ifindex has changed and update it if required
			 */
			ecm_db_iface_identifier_hash_table_entry_check_and_update(ii, dev_interface_num);
		}
		return ii;
	}
#endif

#ifdef ECM_INTERFACE_RAWIP_ENABLE
	/*
	 * RAWIP type?
	 */
	if (dev_type == ARPHRD_RAWIP) {
		/*
		 * Copy netdev address to the type info.
		 */
		ether_addr_copy(type_info.rawip.address, dev->dev_addr);
		DEBUG_TRACE("%px: Net device: %px is RAWIP, MAC addr: %pM\n",
			       feci, dev, type_info.rawip.address);

		/*
		 * Establish this type of interface
		 */
		ii = ecm_interface_rawip_interface_establish(&type_info.rawip, dev_name, dev_interface_num, ae_interface_num, dev_mtu);
		return ii;
	}
#endif
#ifdef ECM_INTERFACE_OVPN_ENABLE
	/*
	 * OVPN Tunnel?
	 */
	if ((dev_type == ARPHRD_NONE) && (dev->priv_flags_ext & IFF_EXT_TUN_TAP)) {
		struct net_device *tun_dev = NULL;
		ip_addr_t saddr, daddr;

		DEBUG_TRACE("Net device: %px is OVPN type: %d\n", dev, dev_type);

		ae_interface_num = ecm_interface_ovpn_get_ifnum(dev, skb, &tun_dev);
		if ((ae_interface_num <= 0) || !tun_dev) {
			DEBUG_WARN("%px: Couldn't get OVPN acceleration interface number\n", dev);
			return NULL;
		}

		type_info.ovpn.tun_ifnum = ae_interface_num;

		/*
		 * Copy IP addresses from skb
		 */
		if (ip_hdr(skb)->version == IPVERSION) {
			ECM_NIN4_ADDR_TO_IP_ADDR(saddr, ip_hdr(skb)->saddr);
			ECM_NIN4_ADDR_TO_IP_ADDR(daddr, ip_hdr(skb)->daddr);
		} else {
			ECM_NIN6_ADDR_TO_IP_ADDR(saddr, ipv6_hdr(skb)->saddr);
			ECM_NIN6_ADDR_TO_IP_ADDR(daddr, ipv6_hdr(skb)->daddr);
		}

		ecm_interface_tunnel_mtu_update(saddr, daddr, ECM_DB_IFACE_TYPE_OVPN, &dev_mtu);
		ii = ecm_interface_ovpn_interface_establish(&type_info.ovpn, tun_dev->name, tun_dev->ifindex, dev_mtu);
		return ii;
	}
#endif
	/*
	 * If this is NOT PPP then it is unknown to the ecm
	 */
	if (dev_type != ARPHRD_PPP) {
		DEBUG_TRACE("%px: Net device: %px is UNKNOWN type: %d\n", feci, dev, dev_type);
		type_info.unknown.os_specific_ident = dev_interface_num;

		/*
		 * Establish this type of interface
		 */
		ii = ecm_interface_unknown_interface_establish(&type_info.unknown, dev_name, dev_interface_num, ae_interface_num, dev_mtu);
		return ii;
	}

#ifndef ECM_INTERFACE_PPP_ENABLE
	/*
	 * PPP Support is NOT provided for.
	 * Interface is therefore unknown
	 */
	DEBUG_TRACE("%px: Net device: %px is UNKNOWN (PPP Unsupported) type: %d\n", feci, dev, dev_type);
	type_info.unknown.os_specific_ident = dev_interface_num;

	/*
	 * Establish this type of interface
	 */
	ii = ecm_interface_unknown_interface_establish(&type_info.unknown, dev_name, dev_interface_num, ae_interface_num, dev_mtu);
	return ii;
#else

#ifdef ECM_INTERFACE_L2TPV2_ENABLE
	/*
	 * ppp_xmit lock is held by linux kernel for l2tp packet in transmit
	 * direction. we need to check for l2tp packet and avoid calls to
	 * ppp_is_multilink() and ppp_hold_channels() which acquire same lock
	 */

	if ((dev->priv_flags_ext & IFF_EXT_PPP_L2TPV2) && ppp_is_xmit_locked(dev)) {
		if (skb && (skb->skb_iif == dev->ifindex)) {
			struct pppol2tp_common_addr info;

			if (__ppp_is_multilink(dev) > 0) {
				DEBUG_TRACE("%px: Net device: %px is MULTILINK PPP - Unknown to the ECM\n", feci, dev);
				type_info.unknown.os_specific_ident = dev_interface_num;

				/*
				 * Establish this type of interface
				 */
				ii = ecm_interface_unknown_interface_establish(&type_info.unknown, dev_name, dev_interface_num, ae_interface_num, dev_mtu);
				return ii;
			}
			channel_count = __ppp_hold_channels(dev, ppp_chan, 1);
			if (channel_count != 1) {
				DEBUG_TRACE("%px: Net device: %px PPP has %d channels - ECM cannot handle this (interface becomes Unknown type)\n",
					    feci, dev, channel_count);
				type_info.unknown.os_specific_ident = dev_interface_num;

				/*
				 * Establish this type of interface
				 */
				ii = ecm_interface_unknown_interface_establish(&type_info.unknown, dev_name, dev_interface_num, ae_interface_num, dev_mtu);
				return ii;
			}

			if (pppol2tp_channel_addressing_get(ppp_chan[0], &info)) {
				ppp_release_channels(ppp_chan, 1);
				return NULL;
			}

			type_info.pppol2tpv2.l2tp.tunnel.tunnel_id = info.local_tunnel_id;
			type_info.pppol2tpv2.l2tp.tunnel.peer_tunnel_id = info.remote_tunnel_id;
			type_info.pppol2tpv2.l2tp.session.session_id = info.local_session_id;
			type_info.pppol2tpv2.l2tp.session.peer_session_id = info.remote_session_id;
			type_info.pppol2tpv2.udp.sport = ntohs(info.local_addr.sin_port);
			type_info.pppol2tpv2.udp.dport = ntohs(info.remote_addr.sin_port);
			type_info.pppol2tpv2.ip.saddr = ntohl(info.local_addr.sin_addr.s_addr);
			type_info.pppol2tpv2.ip.daddr = ntohl(info.remote_addr.sin_addr.s_addr);

			/*
			 * Release the channel.  Note that next_dev is still (correctly) held.
			 */
			ppp_release_channels(ppp_chan, 1);

			DEBUG_TRACE("%px: Net device: %px PPPo2L2TP session: %d,n", feci, dev, type_info.pppol2tpv2.l2tp.session.peer_session_id);

			/*
			 * Establish this type of interface
			 */
			ii = ecm_interface_pppol2tpv2_interface_establish(&type_info.pppol2tpv2, dev_name, dev_interface_num, ae_interface_num, dev_mtu);
			return ii;
		}
	}
#endif

#ifdef ECM_INTERFACE_PPTP_ENABLE
	if ((protocol == IPPROTO_GRE) && skb && v4_hdr && (dev->priv_flags_ext & IFF_EXT_PPP_PPTP)) {
		struct gre_base_hdr *gre_hdr;
		uint16_t proto;
		int ret;

		skb_pull(skb, sizeof(struct iphdr));
		gre_hdr = (struct gre_base_hdr *)(skb->data);
		proto = ntohs(gre_hdr->protocol);
		if ((gre_hdr->flags & GRE_VERSION) == ECM_GRE_VERSION_1) {
			ecm_gre_hdr_pptp *hdr = (ecm_gre_hdr_pptp *)gre_hdr;

			ret = pptp_session_find(&opt, hdr->call_id, v4_hdr->daddr);
			if (ret < 0) {
				skb_push(skb, sizeof(struct iphdr));
				DEBUG_WARN("%px: PPTP session info not found\n", feci);
				return NULL;
			}

			/*
			 * Get PPTP session info
			 */
			type_info.pptp.src_call_id = opt.src_addr.call_id;
			type_info.pptp.dst_call_id = opt.dst_addr.call_id;
			type_info.pptp.src_ip = ntohl(opt.src_addr.sin_addr.s_addr);
			type_info.pptp.dst_ip = ntohl(opt.dst_addr.sin_addr.s_addr);

			skb_push(skb, sizeof(struct iphdr));

			interface_type = feci->ae_interface_type_get(feci, dev);
			ae_interface_num = feci->ae_interface_number_by_dev_type_get(dev, interface_type);

			/*
			 * Establish this type of interface
			 */
			ii = ecm_interface_pptp_interface_establish(&type_info.pptp, dev_name, dev_interface_num, ae_interface_num, dev_mtu);
			if (ii) {
				/*
				 * The ifindex of a virtual netdevice like a PPTP session can change if it is destroyed
				 * and comes up again. Detect if the ifindex has changed and update it if required
				 */
				ecm_db_iface_identifier_hash_table_entry_check_and_update(ii, dev_interface_num);
			}
			return ii;
		}

		skb_push(skb, sizeof(struct iphdr));

		DEBUG_TRACE("%px: Unknown GRE protocol\n", feci);
		type_info.unknown.os_specific_ident = dev_interface_num;

		/*
		 * Establish this type of interface
		 */
		ii = ecm_interface_unknown_interface_establish(&type_info.unknown, dev_name, dev_interface_num, ae_interface_num, dev_mtu);
		return ii;
	}
#endif
	/*
	 * PPP - but what is the channel type?
	 * First: If this is multi-link then we do not support it
	 */
	if (ppp_is_multilink(dev) > 0) {
		DEBUG_TRACE("%px: Net device: %px is MULTILINK PPP - Unknown to the ECM\n", feci, dev);
		type_info.unknown.os_specific_ident = dev_interface_num;

		/*
		 * Establish this type of interface
		 */
		ii = ecm_interface_unknown_interface_establish(&type_info.unknown, dev_name, dev_interface_num, ae_interface_num, dev_mtu);
		return ii;
	}

	DEBUG_TRACE("%px: Net device: %px is PPP\n", feci, dev);

	/*
	 * Get the PPP channel and then enquire what kind of channel it is
	 * NOTE: Not multilink so only one channel to get.
	 */
	channel_count = ppp_hold_channels(dev, ppp_chan, 1);
	if (channel_count != 1) {
		DEBUG_TRACE("%px: Net device: %px PPP has %d channels - ECM cannot handle this (interface becomes Unknown type)\n",
				feci, dev, channel_count);
		type_info.unknown.os_specific_ident = dev_interface_num;

		/*
		 * Establish this type of interface
		 */
		ii = ecm_interface_unknown_interface_establish(&type_info.unknown, dev_name, dev_interface_num, ae_interface_num, dev_mtu);
		return ii;
	}

	/*
	 * Get channel protocol type
	 * NOTE: Not all PPP channels support channel specific methods.
	 */
	channel_protocol = ppp_channel_get_protocol(ppp_chan[0]);

#ifdef ECM_INTERFACE_L2TPV2_ENABLE
	if (channel_protocol == PX_PROTO_OL2TP) {
		struct pppol2tp_common_addr info;

		if (pppol2tp_channel_addressing_get(ppp_chan[0], &info)) {
			ppp_release_channels(ppp_chan, 1);
			return NULL;
		}

		type_info.pppol2tpv2.l2tp.tunnel.tunnel_id = info.local_tunnel_id;
		type_info.pppol2tpv2.l2tp.tunnel.peer_tunnel_id = info.remote_tunnel_id;
		type_info.pppol2tpv2.l2tp.session.session_id = info.local_session_id;
		type_info.pppol2tpv2.l2tp.session.peer_session_id = info.remote_session_id;
		type_info.pppol2tpv2.udp.sport = ntohs(info.local_addr.sin_port);
		type_info.pppol2tpv2.udp.dport = ntohs(info.remote_addr.sin_port);
		type_info.pppol2tpv2.ip.saddr = ntohl(info.local_addr.sin_addr.s_addr);
		type_info.pppol2tpv2.ip.daddr = ntohl(info.remote_addr.sin_addr.s_addr);

		/*
		 * Release the channel.  Note that next_dev is still (correctly) held.
		 */
		ppp_release_channels(ppp_chan, 1);

		DEBUG_TRACE("%px: Net device: %px PPPo2L2TP session: %d,n", feci, dev, type_info.pppol2tpv2.l2tp.session.peer_session_id);

		/*
		 * Establish this type of interface
		 */
		ii = ecm_interface_pppol2tpv2_interface_establish(&type_info.pppol2tpv2, dev_name, dev_interface_num, ae_interface_num, dev_mtu);
		return ii;
	}
#endif
#ifdef ECM_INTERFACE_PPPOE_ENABLE
	if (channel_protocol == PX_PROTO_OE) {
		struct pppoe_opt addressing;

		/*
		 * PPPoE channel
		 */
		DEBUG_TRACE("%px: Net device: %px PPP channel is PPPoE\n", feci, dev);

		/*
		 * Get PPPoE session information and the underlying device it is using.
		 */
		if (pppoe_channel_addressing_get(ppp_chan[0], &addressing)) {
			DEBUG_WARN("%px: failed to get PPPoE addressing info\n", feci);
			ppp_release_channels(ppp_chan, 1);
			return NULL;
		}

		type_info.pppoe.pppoe_session_id = (uint16_t)ntohs((uint16_t)addressing.pa.sid);
		memcpy(type_info.pppoe.remote_mac, addressing.pa.remote, ETH_ALEN);
		dev_put(addressing.dev);

		/*
		 * Release the channel.  Note that next_dev is still (correctly) held.
		 */
		ppp_release_channels(ppp_chan, 1);

		DEBUG_TRACE("%px: Net device: %px PPPoE session: %x, remote mac: %pM\n",
			    feci, dev, type_info.pppoe.pppoe_session_id, type_info.pppoe.remote_mac);

		/*
		 * Establish this type of interface
		 */
		ii = ecm_interface_pppoe_interface_establish(&type_info.pppoe, dev_name, dev_interface_num, ae_interface_num, dev_mtu);
		return ii;
	}
#endif

#ifdef ECM_INTERFACE_PPTP_ENABLE
	if (channel_protocol == PX_PROTO_PPTP) {

		pptp_channel_addressing_get(&opt, ppp_chan[0]);

		/*
		 * Get PPTP session info
		 */
		type_info.pptp.src_call_id = opt.src_addr.call_id;
		type_info.pptp.dst_call_id = opt.dst_addr.call_id;
		type_info.pptp.src_ip = ntohl(opt.src_addr.sin_addr.s_addr);
		type_info.pptp.dst_ip = ntohl(opt.dst_addr.sin_addr.s_addr);

		DEBUG_TRACE("%px: Net device: %px PPTP source call id: %d\n", feci, dev, type_info.pptp.src_call_id);
		ppp_release_channels(ppp_chan, 1);

		interface_type = feci->ae_interface_type_get(feci, dev);
		ae_interface_num = feci->ae_interface_number_by_dev_type_get(dev, interface_type);

		/*
		 * Establish this type of interface
		 */
		ii = ecm_interface_pptp_interface_establish(&type_info.pptp, dev_name, dev_interface_num, ae_interface_num, dev_mtu);
		if (ii) {
			/*
			 * The ifindex of a virtual netdevice like a PPTP session can change if it is destroyed
			 * and comes up again. Detect if the ifindex has changed and update it if required
			 */
			ecm_db_iface_identifier_hash_table_entry_check_and_update(ii, dev_interface_num);
		}
		return ii;
	}
#endif
	DEBUG_TRACE("%px: Net device: %px PPP channel protocol: %d - Unknown to the ECM\n", feci, dev, channel_protocol);
	type_info.unknown.os_specific_ident = dev_interface_num;

	/*
	 * Release the channel
	 */
	ppp_release_channels(ppp_chan, 1);

	/*
	 * Establish this type of interface
	 */
	ii = ecm_interface_unknown_interface_establish(&type_info.unknown, dev_name, dev_interface_num, ae_interface_num, dev_mtu);
	return ii;
#endif
}
EXPORT_SYMBOL(ecm_interface_establish_and_ref);

#ifdef ECM_MULTICAST_ENABLE
/*
 * ecm_interface_multicast_heirarchy_construct_single()
 *	Create and return an interface heirarchy for a single interface for a multicast connection
 *
 *	src_addr	IP source address
 *	dest_addr	IP Destination address/Group Address
 *	interface	Pointer to a single multicast interface heirarchy
 *	given_dest_dev	Netdev pointer for destination interface
 *	br_slave_dev	Netdev pointer to a bridge slave device. It could be NULL in case of pure
 *			routed flow without any bridge interface in destination dev list.
 *	skb             sk_buff
 */
static uint32_t ecm_interface_multicast_heirarchy_construct_single(struct ecm_front_end_connection_instance *feci, ip_addr_t src_addr,
								   ip_addr_t dest_addr, struct ecm_db_iface_instance *interface,
								   struct net_device *given_dest_dev, struct net_device *br_slave_dev,
								   uint8_t *src_node_addr, bool is_routed, __be16 *layer4hdr, struct sk_buff *skb)
{
	struct ecm_db_iface_instance *to_list_single[ECM_DB_IFACE_HEIRARCHY_MAX];
	struct ecm_db_iface_instance **ifaces;
	struct ecm_db_iface_instance *ii_temp;
	struct net_device *dest_dev;
	int32_t current_interface_index;
	int32_t interfaces_cnt = 0;
	int32_t dest_dev_type;

	dest_dev = given_dest_dev;
	dev_hold(dest_dev);
	dest_dev_type = dest_dev->type;
	current_interface_index = ECM_DB_IFACE_HEIRARCHY_MAX;

	while (current_interface_index > 0) {
		struct ecm_db_iface_instance *ii;
		struct net_device *next_dev;

		/*
		 * Get the ecm db interface instance for the device at hand
		 */
		ii = ecm_interface_establish_and_ref(feci, dest_dev, skb);
		interfaces_cnt++;

		/*
		 * If the interface could not be established then we abort
		 */
		if (!ii) {
			DEBUG_WARN("Failed to establish interface: %px, name: %s\n", dest_dev, dest_dev->name);
			goto fail;
		}

		/*
		 * Record the interface instance into the *ifaces
		 */
		current_interface_index--;
		ii_temp = ecm_db_multicast_if_instance_get_at_index(interface, current_interface_index);
		ifaces = (struct ecm_db_iface_instance **)ii_temp;
		*ifaces = ii;

		/*
		 * Now we have to figure out what the next device will be (in the transmission path)
		 */
		do {
#ifdef ECM_INTERFACE_PPP_ENABLE
			int channel_count;
			struct ppp_channel *ppp_chan[1];
			int channel_protocol;
			struct pppoe_opt addressing;
#endif
			DEBUG_TRACE("Net device: %px is type: %d, name: %s\n", dest_dev, dest_dev_type, dest_dev->name);
			next_dev = NULL;

			if (dest_dev_type == ARPHRD_ETHER) {
				/*
				 * Ethernet - but what sub type?
				 */
#ifdef ECM_INTERFACE_GRE_TAP_ENABLE
				if (dest_dev->priv_flags_ext & (IFF_EXT_GRE_V4_TAP | IFF_EXT_GRE_V6_TAP)) {
					DEBUG_TRACE("%px: Acceleration not supported for GRE tap flows\n", dest_dev);
					dev_put(dest_dev);
					ecm_db_multicast_copy_if_heirarchy(to_list_single, interface);
					ecm_db_connection_interfaces_deref(to_list_single, current_interface_index);
					return ECM_DB_IFACE_HEIRARCHY_MAX;
				}
#endif
				/*
				 * VLAN?
				 */
				if (is_vlan_dev(dest_dev)) {
					/*
					 * VLAN master
					 * No locking needed here, ASSUMPTION is that real_dev is held for as long as we have dev.
					 */
					next_dev = ecm_interface_vlan_real_dev(dest_dev);
					dev_hold(next_dev);
					DEBUG_TRACE("Net device: %px is VLAN, slave dev: %px (%s)\n",
							dest_dev, next_dev, next_dev->name);
					break;
				}

				/*
				 * LINUX_BRIDGE/OVS_BRIDGE?
				 */
				if (ecm_front_end_is_bridge_device(dest_dev)
#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
					|| ecm_front_end_is_ovs_bridge_device(dest_dev)
#endif
				   ) {
					if (!br_slave_dev) {
						goto fail;
					}

					if (!ecm_front_end_is_bridge_port(br_slave_dev)
#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
					&& !ecm_interface_is_ovs_bridge_port(br_slave_dev)
#endif
					) {
						DEBUG_ASSERT(NULL, "%px: expected only bridge slave here\n", interface);
						goto fail;
					}

					next_dev = br_slave_dev;
					DEBUG_TRACE("Net device: %px is BRIDGE, next_dev: %px (%s)\n", dest_dev, next_dev, next_dev->name);
					dev_hold(next_dev);
					break;
				}

#ifdef ECM_INTERFACE_MACVLAN_ENABLE
				/*
				 * MAC-VLAN?
				 */
				if (netif_is_macvlan(dest_dev)) {
					if (ecm_interface_macvlan_mode_is_valid(dest_dev)) {
						next_dev = macvlan_dev_real_dev(dest_dev);
						dev_hold(next_dev);
						DEBUG_TRACE("%px: Net device: %px is MAC-VLAN, slave dev: %px (%s)\n",
								feci, dest_dev, next_dev, next_dev->name);
						break;
					}
				}
#endif

#ifdef ECM_INTERFACE_BOND_ENABLE
				/*
				 * LAG?
				 */
				if (ecm_front_end_is_lag_master(dest_dev)) {
					/*
					 * Link aggregation
					 * Figure out which slave device of the link aggregation will be used to reach the destination.
					 */
					uint32_t src_addr_32 = 0;
					uint32_t dest_addr_32 = 0;
					struct in6_addr src_addr6;
					struct in6_addr dest_addr6;
					uint8_t src_mac_addr[ETH_ALEN];
					uint8_t dest_mac_addr[ETH_ALEN];

					memset(src_mac_addr, 0, ETH_ALEN);
					memset(dest_mac_addr, 0, ETH_ALEN);

					if (ECM_IP_ADDR_IS_V4(src_addr)) {
						ECM_IP_ADDR_TO_NIN4_ADDR(src_addr_32, src_addr);
						ECM_IP_ADDR_TO_NIN4_ADDR(dest_addr_32, dest_addr);
					}

					if (!is_routed) {
						memcpy(src_mac_addr, src_node_addr, ETH_ALEN);
					} else {
						struct net_device *dest_dev_master;

						/*
						 * Use appropriate source MAC address for routed packets
						 */
						dest_dev_master = ecm_interface_get_and_hold_dev_master(dest_dev);
						if (dest_dev_master) {
							memcpy(src_mac_addr, dest_dev_master->dev_addr, ETH_ALEN);
							dev_put(dest_dev_master);
						} else {
							memcpy(src_mac_addr, dest_dev->dev_addr, ETH_ALEN);
						}
					}

					/*
					 * Create Destination MAC address using IP multicast destination address
					 */
					ecm_translate_multicast_mac(dest_addr, dest_mac_addr);

					if (ECM_IP_ADDR_IS_V4(src_addr)) {
						next_dev = bond_get_tx_dev(NULL, src_mac_addr, dest_mac_addr,
									   &src_addr_32, &dest_addr_32,
									   htons((uint16_t)ETH_P_IP), dest_dev, layer4hdr);
					} else {
						ECM_IP_ADDR_TO_NIN6_ADDR(src_addr6, src_addr);
						ECM_IP_ADDR_TO_NIN6_ADDR(dest_addr6, dest_addr);
						next_dev = bond_get_tx_dev(NULL, src_mac_addr, dest_mac_addr,
									   src_addr6.s6_addr, dest_addr6.s6_addr,
									   htons((uint16_t)ETH_P_IPV6), dest_dev, layer4hdr);
					}

					if (!(next_dev && netif_carrier_ok(next_dev))) {
						DEBUG_WARN("Unable to obtain LAG output slave device\n");
						goto fail;
					}

					dev_hold(next_dev);
					DEBUG_TRACE("Net device: %px is LAG, slave dev: %px (%s)\n", dest_dev, next_dev, next_dev->name);
					break;
				}
#endif

				/*
				 * ETHERNET!
				 * Just plain ethernet it seems.
				 */
				DEBUG_TRACE("Net device: %px is ETHERNET\n", dest_dev);
				break;
			}

			/*
			 * LOOPBACK?
			 */
			if (dest_dev_type == ARPHRD_LOOPBACK) {
				DEBUG_TRACE("Net device: %px is LOOPBACK type: %d\n", dest_dev, dest_dev_type);
				break;
			}

			/*
			 * IPSEC?
			 */
			if (dest_dev_type == ECM_ARPHRD_IPSEC_TUNNEL_TYPE) {
				DEBUG_TRACE("Net device: %px is IPSec tunnel type: %d\n", dest_dev, dest_dev_type);
				/*
				 * TODO Figure out the next device the tunnel is using...
				 */
				break;
			}

			/*
			 * SIT (6-in-4)?
			 */
			if (dest_dev_type == ARPHRD_SIT) {
				DEBUG_TRACE("Net device: %px is SIT (6-in-4) type: %d\n", dest_dev, dest_dev_type);
				/*
				 * TODO Figure out the next device the tunnel is using...
				 */
				break;
			}

			/*
			 * IPIP6 Tunnel?
			 */
			if (dest_dev_type == ARPHRD_TUNNEL6) {
				DEBUG_TRACE("Net device: %px is TUNIPIP6 type: %d\n", dest_dev, dest_dev_type);
				/*
				 * TODO Figure out the next device the tunnel is using...
				 */
				break;
			}

			/*
			 * If this is NOT PPP then it is unknown to the ecm and we cannot figure out it's next device.
			 */
			if (dest_dev_type != ARPHRD_PPP) {
				DEBUG_TRACE("Net device: %px is UNKNOWN type: %d\n", dest_dev, dest_dev_type);
				break;
			}

#ifndef ECM_INTERFACE_PPP_ENABLE
			DEBUG_TRACE("Net device: %px is UNKNOWN (PPP Unsupported) type: %d\n", dest_dev, dest_dev_type);
#else
			/*
			 * PPP - but what is the channel type?
			 * First: If this is multi-link then we do not support it
			 */
			if (ppp_is_multilink(dest_dev) > 0) {
				DEBUG_TRACE("Net device: %px is MULTILINK PPP - Unknown to the ECM\n", dest_dev);
				break;
			}

			DEBUG_TRACE("Net device: %px is PPP\n", dest_dev);

			/*
			 * Get the PPP channel and then enquire what kind of channel it is
			 * NOTE: Not multilink so only one channel to get.
			 */
			channel_count = ppp_hold_channels(dest_dev, ppp_chan, 1);
			if (channel_count != 1) {
				DEBUG_TRACE("Net device: %px PPP has %d channels - Unknown to the ECM\n",
						dest_dev, channel_count);
				break;
			}

			/*
			 * Get channel protocol type
			 * NOTE: Not all PPP channels support channel specific methods.
			 */
			channel_protocol = ppp_channel_get_protocol(ppp_chan[0]);
			if (channel_protocol != PX_PROTO_OE) {
				DEBUG_TRACE("Net device: %px PPP channel protocol: %d - Unknown to the ECM\n",
						dest_dev, channel_protocol);

				/*
				 * Release the channel
				 */
				ppp_release_channels(ppp_chan, 1);

				break;
			}

			/*
			 * PPPoE channel
			 */
			DEBUG_TRACE("Net device: %px PPP channel is PPPoE\n", dest_dev);

			/*
			 * Get PPPoE session information and the underlying device it is using.
			 */
			if (pppoe_channel_addressing_get(ppp_chan[0], &addressing)) {
				DEBUG_WARN("%px: failed to get PPPoE addressing info\n", dest_dev);
				ppp_release_channels(ppp_chan, 1);
				break;
			}

			/*
			 * Copy the dev hold into this, we will release the hold later
			 */
			next_dev = addressing.dev;

			/*
			 * Release the channel.  Note that next_dev is still (correctly) held.
			 */
			ppp_release_channels(ppp_chan, 1);
#endif
		} while (false);

		/*
		 * No longer need dest_dev as it may become next_dev
		 */
		dev_put(dest_dev);

		/*
		 * Check out the next_dev, if any
		 */
		if (!next_dev) {
			int32_t i __attribute__((unused));
			DEBUG_INFO("Completed interface heirarchy construct with first interface @: %d\n", current_interface_index);
#if DEBUG_LEVEL > 1
			ecm_db_multicast_copy_if_heirarchy(to_list_single, interface);
			for (i = current_interface_index; i < ECM_DB_IFACE_HEIRARCHY_MAX; ++i) {
				DEBUG_TRACE("\tInterface @ %d: %px, type: %d, name: %s\n", \
						i, to_list_single[i], ecm_db_iface_type_get(to_list_single[i]), \
						ecm_db_interface_type_to_string(ecm_db_iface_type_get(to_list_single[i])));
			}
#endif
			return current_interface_index;
		}

		/*
		 * dest_dev becomes next_dev
		 */
		dest_dev = next_dev;
		dest_dev_type = dest_dev->type;
	}

fail:
	dev_put(dest_dev);

	ecm_db_multicast_copy_if_heirarchy(to_list_single, interface);
	ecm_db_connection_interfaces_deref(to_list_single, current_interface_index);
	return ECM_DB_IFACE_HEIRARCHY_MAX;
}

/*
 * ecm_interface_hierarchy_delete()
 *	Delete hierarchy of the requested interfaces.
 */
static inline void ecm_interface_hierarchy_delete(struct ecm_db_iface_instance *interfaces,
							uint32_t *interface_first_base,
							int valid_if)
{
	struct ecm_db_iface_instance *to_list_single[ECM_DB_IFACE_HEIRARCHY_MAX];
	struct ecm_db_iface_instance *ifaces;
	int i;

	for (i = 0; i < valid_if; i++) {
		ifaces = ecm_db_multicast_if_heirarchy_get(interfaces, i);
		ecm_db_multicast_copy_if_heirarchy(to_list_single, ifaces);
		ecm_db_connection_interfaces_deref(to_list_single, interface_first_base[i]);
	}
}

/*
 * ecm_interface_multicast_heirarchy_construct_routed()
 *	Create destination interface heirarchy for a routed multicast connectiona
 *
 *	interfaces	Pointer to the 2-D array of multicast interface heirarchies
 *	in_dev		Pointer to the source netdev
 *	packet_src_addr	Source IP of the multicast flow
 *	packet_dest_addr Group(dest) IP of the multicast flow
 *	max_dst		Maximum number of netdev joined the multicast group
 *	dst_if_index_base An array of if index joined the multicast group
 *	interface_first_base An array of the index of the first interface in the list
 */
int32_t ecm_interface_multicast_heirarchy_construct_routed(struct ecm_front_end_connection_instance *feci,
								struct ecm_db_iface_instance *interfaces,
								struct net_device *in_dev,
								ip_addr_t packet_src_addr,
								ip_addr_t packet_dest_addr, uint8_t max_if,
								uint32_t *dst_if_index_base,
								uint32_t *interface_first_base, bool mfc_update,
								__be16 *layer4hdr, struct sk_buff *skb)
{
	struct ecm_db_iface_instance *ifaces;
	struct net_device *dest_dev = NULL;
	struct net_device *br_dev_src = NULL;
	uint32_t *dst_if_index;
	uint32_t *interface_first;
	uint32_t br_if;
	uint32_t valid_if;
	int32_t if_num;
	int32_t dest_dev_type;
	int if_index;
	int ii_cnt;
	int total_ii_count = 0;
	bool src_dev_is_bridge = false, dest_dev_is_br_dev_src = false;

	DEBUG_TRACE("Construct interface heirarchy for dest_addr: " ECM_IP_ADDR_DOT_FMT " src_addr: " ECM_IP_ADDR_DOT_FMT "total destination ifs %d\n",
			ECM_IP_ADDR_TO_DOT(packet_dest_addr), ECM_IP_ADDR_TO_DOT(packet_src_addr), max_if);

	/*
	 * Check if the source net_dev is a bridge slave.
	 *
	 * TODO: We are already considering ingress bridge device and
	 * adding it to dst_dev in ecm_nss_multicast_ipv4_connection_process().
	 * Check if this can be removed.
	 */
	if (in_dev && !mfc_update) {
		if (ecm_front_end_is_bridge_port(in_dev)
#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
				|| ecm_interface_is_ovs_bridge_port(in_dev)
#endif
		   ) {
			br_dev_src = ecm_interface_get_and_hold_dev_master(in_dev);
			if (!br_dev_src) {
				DEBUG_WARN("Expected a master\n");
				return 0;
			}

			/*
			 * Source netdev is part of a bridge. First make sure that this bridge
			 * is not already part of the dest_if list given by MFC. If not, we
			 * include the bridge in the dest if list to check for any bridge
			 * slaves interested in the group.
			 */
			if (!ecm_interface_multicast_dest_list_find_if(br_dev_src, max_if, dst_if_index_base)) {

				src_dev_is_bridge = true;

				/*
				 * Increase the max_if by one. This will enable us to query MCS
				 * for any bridge slaves that may be interested in the group.
				 */
				max_if++;
			}
		}
	}

	ii_cnt = 0;
	br_if = if_num = 0;

	/*
	 * This loop is for creating the destination interface hierarchy list.
	 * We take the destination interface array we got from MFC (in form of ifindex array)
	 * as input for this.
	 */
	for (if_index = 0, valid_if = 0; if_index < max_if; if_index++) {
		dst_if_index = ecm_db_multicast_if_first_get_at_index(dst_if_index_base, if_index);
		if (*dst_if_index == ECM_INTERFACE_LOOPBACK_DEV_INDEX) {
			continue;
		}

		dest_dev_is_br_dev_src = false;
		dest_dev = dev_get_by_index(&init_net, *dst_if_index);
		if (!dest_dev) {
			if (!src_dev_is_bridge) {
				/*
				 * If already constructed any interface heirarchies before hitting
				 * this error condition then Deref all interface heirarchies.
				 */
				if (valid_if > 0) {
					ecm_interface_hierarchy_delete(interfaces, interface_first_base, valid_if);
				}

				goto fail1;
			}

			dest_dev = br_dev_src;

			/*
			 * In some cases when WAN interface is added to bridge and traffic is downstream,
			 * the bridge device is part of the destination list from MFC, and at the same time
			 * 'src_dev_is_bridge' will be true as well. In such cases we will need to release
			 * the hold on the bridge device separately for dest_dev and br_dev_src.
			 * Setting this flag to true indicates that this is not the case,
			 * and that releasing the hold once is enough
			 */
			dest_dev_is_br_dev_src = true;
		}

		dest_dev_type = dest_dev->type;
		if (ecm_front_end_is_bridge_device(dest_dev)
#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
			|| ecm_front_end_is_ovs_bridge_device(dest_dev)
#endif
		   ) {
			struct net_device *mc_br_slave_dev = NULL;
			uint32_t mc_max_dst = ECM_DB_MULTICAST_IF_MAX;
			uint32_t mc_dst_if_index[ECM_DB_MULTICAST_IF_MAX];

			if (ECM_IP_ADDR_IS_V4(packet_src_addr)) {
				if_num = mc_bridge_ipv4_get_if(dest_dev, htonl((packet_src_addr[0])), htonl(packet_dest_addr[0]), mc_max_dst, mc_dst_if_index);
			} else {
#ifdef ECM_IPV6_ENABLE
				struct in6_addr origin6;
				struct in6_addr group6;
				ECM_IP_ADDR_TO_NIN6_ADDR(origin6, packet_src_addr);
				ECM_IP_ADDR_TO_NIN6_ADDR(group6, packet_dest_addr);
				if_num = mc_bridge_ipv6_get_if(dest_dev, &origin6, &group6, mc_max_dst, mc_dst_if_index);
#else
				DEBUG_WARN("IPv6 support not enabled\n");
				if_num = -1;
#endif
			}

			if ((if_num < 0) || (if_num > ECM_DB_MULTICAST_IF_MAX)) {
				DEBUG_WARN("MCS is not ready\n");

				/*
				 * If already constructed any interface heirarchies before hitting
				 * this error condition then Deref all interface heirarchies.
				 */
				if (valid_if > 0) {
					ecm_interface_hierarchy_delete(interfaces, interface_first_base, valid_if);
				}

				goto fail2;
			}

			if (in_dev && !mfc_update) {
				if_num = ecm_interface_multicast_check_for_src_ifindex(mc_dst_if_index, if_num, in_dev->ifindex);
			}

			for (br_if = 0; br_if < if_num; br_if++) {
				int total_if = valid_if + br_if;

				mc_br_slave_dev = dev_get_by_index(&init_net, mc_dst_if_index[br_if]);
				if (!mc_br_slave_dev) {
					continue;
				}

				if (total_if > ECM_DB_MULTICAST_IF_MAX) {
					ecm_interface_hierarchy_delete(interfaces, interface_first_base, total_if);
					dev_put(mc_br_slave_dev);
					goto fail2;
				}

				ifaces = ecm_db_multicast_if_heirarchy_get(interfaces, total_if);
				/*
				 * Construct a single interface heirarchy of a multicast dev.
				 */
				ii_cnt = ecm_interface_multicast_heirarchy_construct_single(feci, packet_src_addr, packet_dest_addr, ifaces, dest_dev, mc_br_slave_dev, NULL, true, layer4hdr, skb);
				if (ii_cnt == ECM_DB_IFACE_HEIRARCHY_MAX) {

					/*
					 * If already constructed any interface heirarchies before hitting
					 * this error condition then Deref all interface heirarchies.
					 */
					if (total_if > 0) {
						ecm_interface_hierarchy_delete(interfaces, interface_first_base, total_if);
					}

					dev_put(mc_br_slave_dev);
					goto fail2;
				}

				interface_first = ecm_db_multicast_if_first_get_at_index(interface_first_base, total_if);
				*interface_first = ii_cnt;
				total_ii_count += ii_cnt;
				dev_put(mc_br_slave_dev);
			}

			valid_if += br_if;
		} else {

			DEBUG_ASSERT(valid_if < ECM_DB_MULTICAST_IF_MAX, "Bad array index size %d\n", valid_if);
			ifaces = ecm_db_multicast_if_heirarchy_get(interfaces, valid_if);
			/*
			 * Construct a single interface heirarchy of a multicast dev.
			 */
			ii_cnt = ecm_interface_multicast_heirarchy_construct_single(feci, packet_src_addr, packet_dest_addr, ifaces, dest_dev, NULL, NULL, true, layer4hdr, skb);
			if (ii_cnt == ECM_DB_IFACE_HEIRARCHY_MAX) {

				/*
				 * If already constructed any interface heirarchies before hitting
				 * this error condition then Deref all interface heirarchies.
				 */
				if (valid_if > 0) {
					ecm_interface_hierarchy_delete(interfaces, interface_first_base, valid_if);
				}

				goto fail2;
			}

			interface_first = ecm_db_multicast_if_first_get_at_index(interface_first_base, valid_if);
			*interface_first = ii_cnt;
			total_ii_count += ii_cnt;
			valid_if++;
		}

		if (!dest_dev_is_br_dev_src) {
			dev_put(dest_dev);
		}
	}

	if (br_dev_src) {
		dev_put(br_dev_src);
	}

	return total_ii_count;

fail2:
	if (!dest_dev_is_br_dev_src) {
		dev_put(dest_dev);
	}

fail1:
	if (br_dev_src) {
		dev_put(br_dev_src);
	}

	return 0;
}
EXPORT_SYMBOL(ecm_interface_multicast_heirarchy_construct_routed);

/*
 * ecm_interface_multicast_heirarchy_construct_bridged()
 *	This function called when the Hyfi bridge snooper has IGMP/IMLD updates, this function
 *	creates destination interface heirarchy for a bridged multicast connection.
 *
 *	interfaces	Pointer to the 2-D array of multicast interface heirarchies
 *	dest_dev	Pointer to the destination dev, here dest_dev is always a bridge type
 *	packet_src_addr	Source IP of the multicast flow
 *	packet_dest_addr Group(dest) IP of the multicast flow
 *	mc_max_dst	Maximum number of bridge slaves joined the multicast group
 *	mc_dst_if_index_base An array of if index joined the multicast group
 *	interface_first_base An array of the index of the first interface in the list
 */
int32_t ecm_interface_multicast_heirarchy_construct_bridged(struct ecm_front_end_connection_instance *feci,
						     struct ecm_db_iface_instance *interfaces, struct net_device *dest_dev,
						     ip_addr_t packet_src_addr, ip_addr_t packet_dest_addr, uint8_t mc_max_dst,
						     int *mc_dst_if_index_base, uint32_t *interface_first_base, uint8_t *src_node_addr,
						     __be16 *layer4hdr, struct sk_buff *skb)
{
	struct ecm_db_iface_instance *to_list_single[ECM_DB_IFACE_HEIRARCHY_MAX];
	struct ecm_db_iface_instance *ifaces;
	struct net_device *mc_br_slave_dev = NULL;
	uint32_t *interface_first;
	int *mc_dst_if_index;
	int valid_if;
	int ii_cnt = 0;
	int br_if;
	int total_ii_cnt = 0;

	/*
	 * Go through the newly joined interface index one by one and
	 * create an interface heirarchy for each valid interface.
	 */
	for (br_if = 0, valid_if = 0; br_if < mc_max_dst; br_if++) {
		mc_dst_if_index = (int *)ecm_db_multicast_if_num_get_at_index(mc_dst_if_index_base, br_if);
		mc_br_slave_dev = dev_get_by_index(&init_net, *mc_dst_if_index);
		if (!mc_br_slave_dev) {

			/*
			 * If already constructed any interface heirarchies before hitting
			 * this error condition then Deref all interface heirarchies.
			 */
			if (valid_if > 0) {
				int i;
				for (i = 0; i < valid_if; i++) {
					ifaces = ecm_db_multicast_if_heirarchy_get(interfaces, i);
					ecm_db_multicast_copy_if_heirarchy(to_list_single, ifaces);
					ecm_db_connection_interfaces_deref(to_list_single, interface_first_base[i]);
				}
			}

			/*
			 * If valid netdev not found, Return 0
			 */
			return 0;
		}

		if (valid_if > ECM_DB_MULTICAST_IF_MAX) {
			int i;

			/*
			 * If already constructed any interface heirarchies before hitting
			 * this error condition then Deref all interface heirarchies.
			 */
			for (i = 0; i < valid_if; i++) {
				ifaces = ecm_db_multicast_if_heirarchy_get(interfaces, i);
				ecm_db_multicast_copy_if_heirarchy(to_list_single, ifaces);
				ecm_db_connection_interfaces_deref(to_list_single, interface_first_base[i]);
			}

			dev_put(mc_br_slave_dev);
			return 0;
		}

		ifaces = ecm_db_multicast_if_heirarchy_get(interfaces, valid_if);

		/*
		 * Construct a single interface heirarchy of a multicast dev.
		 */
		ii_cnt = ecm_interface_multicast_heirarchy_construct_single(feci, packet_src_addr, packet_dest_addr, ifaces, dest_dev, mc_br_slave_dev, src_node_addr, false, layer4hdr, skb);
		if (ii_cnt == ECM_DB_IFACE_HEIRARCHY_MAX) {

			/*
			 * If already constructed any interface heirarchies before hitting
			 * this error condition then Deref all interface heirarchies.
			 */
			if (valid_if > 0) {
			int i;
				for (i = 0; i < valid_if; i++) {
					ifaces = ecm_db_multicast_if_heirarchy_get(interfaces, i);
					ecm_db_multicast_copy_if_heirarchy(to_list_single, ifaces);
					ecm_db_connection_interfaces_deref(to_list_single, interface_first_base[i]);
				}
			}

			dev_put(mc_br_slave_dev);
			return 0;
		}

		interface_first = ecm_db_multicast_if_first_get_at_index(interface_first_base, valid_if);
		*interface_first = ii_cnt;
		total_ii_cnt += ii_cnt;
		valid_if++;
		dev_put(mc_br_slave_dev);
	}

	return valid_if;
}
EXPORT_SYMBOL(ecm_interface_multicast_heirarchy_construct_bridged);

/*
 * ecm_interface_multicast_get_next_node_mac_address()
 *	Get the MAC address of the next node for multicast flows
 *
 * TODO: This function will be removed when the multicast flow code
 * is fixed to use the new interface hierarchy construction model.
 *
 */
static bool ecm_interface_multicast_get_next_node_mac_address(
	ip_addr_t dest_addr, struct net_device *dest_dev, int ip_version,
	uint8_t *mac_addr)
{
	bool on_link;
	ip_addr_t gw_addr = ECM_IP_ADDR_NULL;

	if (!ecm_interface_mac_addr_get(dest_addr, mac_addr, &on_link, gw_addr)) {
		if (ip_version == 4) {
			DEBUG_WARN("Unable to obtain MAC address for " ECM_IP_ADDR_DOT_FMT "\n",
				ECM_IP_ADDR_TO_DOT(dest_addr));
			ecm_interface_send_arp_request(dest_dev, dest_addr, on_link, gw_addr);
		}
#ifdef ECM_IPV6_ENABLE
		if (ip_version == 6) {
			DEBUG_WARN("Unable to obtain MAC address for " ECM_IP_ADDR_OCTAL_FMT "\n",
				ECM_IP_ADDR_TO_OCTAL(dest_addr));
			ecm_interface_send_neighbour_solicitation(dest_dev, dest_addr);
		}
#endif
		return false;
	}

	return true;
}
#endif

/*
 * ecm_interface_get_next_node_mac_address()
 *	Get the MAC address of the next node
 */
static bool ecm_interface_get_next_node_mac_address(ip_addr_t dest_addr,
					struct net_device *dest_dev,
					int ip_version, uint8_t *mac_addr)
{
	ip_addr_t gw_addr = ECM_IP_ADDR_NULL;
	bool on_link = true;

	if (ecm_interface_mac_addr_get_no_route(dest_dev, dest_addr, mac_addr)) {
		return true;
	}

	/*
	 * MAC address look up failed. The host IP address may not be in the
	 * neighbour table. So, let's send an ARP or neighbour solicitation
	 * request to this host IP address, so in the subsequent lookups it can be
	 * found.
	 *
	 * If we have a gateway address, try one more time with that address.
	 * If it fails, send the request with the current dest_addr or
	 * found gateway address.
	 */
	if (ecm_interface_find_gateway(dest_addr, gw_addr)) {
		on_link = false;
		if (ecm_interface_mac_addr_get_no_route(dest_dev, gw_addr, mac_addr)) {
			DEBUG_TRACE("Found the mac address for the gateway\n");
			return true;
		}
	}

	if (ip_version == 4) {
		DEBUG_WARN("Unable to obtain MAC address for " ECM_IP_ADDR_DOT_FMT " send ARP request\n",
				ECM_IP_ADDR_TO_DOT(dest_addr));
		ecm_interface_send_arp_request(dest_dev, dest_addr, on_link, gw_addr);
	}

#ifdef ECM_IPV6_ENABLE
	if (ip_version == 6) {
		DEBUG_WARN("Unable to obtain MAC address for " ECM_IP_ADDR_OCTAL_FMT  " send solicitation request\n",
				ECM_IP_ADDR_TO_OCTAL(dest_addr));
		ecm_interface_send_neighbour_solicitation(dest_dev, dest_addr);
	}
#endif

	return false;
}

/*
 * ecm_interface_should_update_egress_device_bridged()
 *	Determine if the egress port should be re-evaluated in the bridged case
 *
 * This will be done if:
 * - The egress port is the one provided from the front-end
 * - The egress port is not a bridge, but is a slave of the bridge
 * - Not routed
 *
 * If these conditions hold, this function will hold a reference to the bridge
 * port and return it to the caller.  Otherwise no reference will be held and
 * it will return NULL.
 */
static struct net_device *ecm_interface_should_update_egress_device_bridged(
	struct net_device *given_dest_dev, struct net_device *dest_dev,
	bool is_routed)
{
	struct net_device *bridge;

	/*
	 * Determine if we should attempt to fetch the bridge device
	 */
	if (!given_dest_dev || is_routed || (dest_dev != given_dest_dev) ||
		ecm_front_end_is_bridge_device(given_dest_dev))
		return NULL;

	bridge = ecm_interface_get_and_hold_dev_master(given_dest_dev);

	if (!bridge)
		return NULL;

	if (!ecm_front_end_is_bridge_device(bridge)) {
		/*
		 * Master is not a bridge - free the reference and return
		 */
		dev_put(bridge);
		return NULL;
	}

	/*
	 * Reference is held to bridge and must be freed by caller
	 */
	return bridge;
}

static inline bool ecm_interface_is_tunnel_endpoint(struct sk_buff *skb, struct net_device *dev, int ip_version, int protocol)
{
	if (ip_version == 4) {
		if (protocol == IPPROTO_IPV6) {
			return true;
		}

#ifdef ECM_XFRM_ENABLE
		if (dev->type == ECM_ARPHRD_IPSEC_TUNNEL_TYPE && protocol == IPPROTO_UDP &&
				(IPCB(skb)->flags & IPSKB_XFRM_TRANSFORMED)) {
			return true;
		}
#else
		if (protocol == IPPROTO_UDP && udp_hdr(skb)->dest == htons(4500)) {
			return true;
		}
#endif
	}

	if (ip_version == 6 && protocol == IPPROTO_IPIP) {
		return true;
	}

	if (protocol == IPPROTO_GRE || protocol == IPPROTO_ESP) {
		return true;
	}

#ifdef ECM_INTERFACE_OVPN_ENABLE
	if (dev->type == ARPHRD_NONE && dev->priv_flags_ext & IFF_EXT_TUN_TAP) {
		return true;
	}
#endif

	return false;
}

/*
 * ecm_interface_heirarchy_construct()
 *	Construct an interface heirarchy.
 *
 * Using the given addressing, locate the interface heirarchy used to emit packets to that destination.
 * This is the heirarchy of interfaces a packet would transit to emit from the device.
 *
 * We will use the given src/dest devices when is_routed is false.
 * When is_routed is true we will use the construct and the other devices (which is the src device of the
 * construct device) whcih were obtained from the skb's route field and passed to this function..
 *
 * For example, with this network arrangement:
 *
 * PPPoE--VLAN--BRIDGE--BRIDGE_PORT(LAG_MASTER)--LAG_SLAVE_0--10.22.33.11
 *
 * Given the packet_dest_addr IP address 10.22.33.11 this will create an interface heirarchy (in interracfes[]) of:
 * LAG_SLAVE_0 @ [ECM_DB_IFACE_HEIRARCHY_MAX - 5]
 * LAG_MASTER @ [ECM_DB_IFACE_HEIRARCHY_MAX - 4]
 * BRIDGE @ [ECM_DB_IFACE_HEIRARCHY_MAX - 3]
 * VLAN @ [ECM_DB_IFACE_HEIRARCHY_MAX - 2]
 * PPPOE @ [ECM_DB_IFACE_HEIRARCHY_MAX - 1]
 * The value returned is (ECM_DB_IFACE_HEIRARCHY_MAX - 5)
 *
 * IMPORTANT: This function will return any known interfaces in the database, when interfaces do not exist in the database
 * they will be created and added automatically to the database.
 */
int32_t ecm_interface_heirarchy_construct(struct ecm_front_end_connection_instance *feci,
						struct ecm_db_iface_instance *interfaces[],
						struct net_device *const_if, struct net_device *other_if,
						ip_addr_t lookup_src_addr,
						ip_addr_t lookup_dest_addr,
						ip_addr_t real_dest_addr,
						int ip_version, int packet_protocol,
						struct net_device *given_dest_dev,
						bool is_routed, struct net_device *given_src_dev,
						uint8_t *dest_node_addr, uint8_t *src_node_addr,
						__be16 *layer4hdr, struct sk_buff *skb,
						struct ecm_front_end_ovs_params *op)
{
	int protocol;
	ip_addr_t src_addr;
	ip_addr_t dest_addr;
	struct net_device *dest_dev;
	char *dest_dev_name;
	int32_t dest_dev_type;
	struct net_device *src_dev;
	char *src_dev_name;
	int32_t src_dev_type;
	int32_t current_interface_index;
	bool from_local_addr;
	bool next_dest_addr_valid;
	bool next_dest_node_addr_valid = false;
	ip_addr_t next_dest_addr;
	uint8_t next_dest_node_addr[ETH_ALEN] = {0};
	struct net_device *bridge;
	struct net_device *top_dev = NULL;
	uint32_t serial = ecm_db_connection_serial_get(feci->ci);

	/*
	 * Get a big endian of the IPv4 address we have been given as our starting point.
	 */
	protocol = packet_protocol;
	ECM_IP_ADDR_COPY(src_addr, lookup_src_addr);
	ECM_IP_ADDR_COPY(dest_addr, lookup_dest_addr);

	if (ip_version == 4) {
		DEBUG_TRACE("%px: Construct interface heirarchy for from src_addr: " ECM_IP_ADDR_DOT_FMT " to dest_addr: " ECM_IP_ADDR_DOT_FMT ", protocol: %d (serial %u)\n",
				feci, ECM_IP_ADDR_TO_DOT(src_addr), ECM_IP_ADDR_TO_DOT(dest_addr), protocol,
				serial);
#ifdef ECM_IPV6_ENABLE
	} else if (ip_version == 6) {
		DEBUG_TRACE("%px: Construct interface heirarchy for from src_addr: " ECM_IP_ADDR_OCTAL_FMT " to dest_addr: " ECM_IP_ADDR_OCTAL_FMT ", protocol: %d (serial %u)\n",
				feci, ECM_IP_ADDR_TO_OCTAL(src_addr), ECM_IP_ADDR_TO_OCTAL(dest_addr), protocol,
				serial);
#endif
	} else {
		DEBUG_WARN("%px: Wrong IP protocol: %d\n", feci, ip_version);
		return ECM_DB_IFACE_HEIRARCHY_MAX;
	}

	/*
	 * Get device to reach the given destination address.
	 * If the heirarchy is for a routed connection we must use the devices obtained from the skb's route information..
	 * If the heirarchy is NOT for a routed connection we try the given_dest_dev.
	 */
	from_local_addr = false;
	if (!is_routed) {
		dest_dev = given_dest_dev;
		dev_hold(dest_dev);
	} else {
		dest_dev = ecm_interface_dev_find_by_local_addr(dest_addr);
		if (dest_dev) {
			from_local_addr = true;
		} else {
			dest_dev = const_if;
			dev_hold(dest_dev);
		}
	}

	/*
	 * If the address is a local address and protocol is an IP tunnel
	 * then this connection is a tunnel endpoint made to this device.
	 * In which case we circumvent all proper procedure and just hack the devices to make stuff work.
	 *
	 * TODO THIS MUST BE FIXED - WE MUST USE THE INTERFACE HIERARCHY FOR ITS INTENDED PURPOSE TO
	 * PARSE THE DEVICES AND WORK OUT THE PROPER INTERFACES INVOLVED.
	 * E.G. IF WE TRIED TO RUN A TUNNEL OVER A VLAN OR QINQ THIS WILL BREAK AS WE DON'T DISCOVER THAT HIERARCHY
	 */
	if (dest_dev && from_local_addr) {
		if (ecm_interface_is_tunnel_endpoint(skb, given_dest_dev, ip_version, protocol)) {
			dev_put(dest_dev);
			dest_dev = given_dest_dev;
			if (dest_dev) {
				dev_hold(dest_dev);
				if (ip_version == 4) {
					DEBUG_TRACE("%px: HACK: %s tunnel packet with dest_addr: " ECM_IP_ADDR_DOT_FMT " uses dev: %px(%s)\n", feci, "IPV4", ECM_IP_ADDR_TO_DOT(dest_addr), dest_dev, dest_dev->name);
				} else {
					DEBUG_TRACE("%px: HACK: %s tunnel packet with dest_addr: " ECM_IP_ADDR_OCTAL_FMT " uses dev: %px(%s)\n", feci, "IPV6", ECM_IP_ADDR_TO_OCTAL(dest_addr), dest_dev, dest_dev->name);
				}
			}
		}
	}

#ifdef ECM_INTERFACE_L2TPV2_ENABLE

	/*
	 * if the address is a local address and indev=l2tp.
	 */
	if ((given_src_dev->type == ARPHRD_PPP) && (given_src_dev->priv_flags_ext & IFF_EXT_PPP_L2TPV2) && ppp_is_xmit_locked(given_src_dev)) {
		if (!dst_xfrm(skb_dst(skb)) || (dest_dev->type != ECM_ARPHRD_IPSEC_TUNNEL_TYPE)) {
			dev_put(dest_dev);
			dest_dev = given_dest_dev;
			if (dest_dev) {
				dev_hold(dest_dev);
				DEBUG_TRACE("%px: l2tp packet tunnel packet with dest_addr: " ECM_IP_ADDR_OCTAL_FMT " uses dev: %px(%s)\n", feci, ECM_IP_ADDR_TO_OCTAL(dest_addr), dest_dev, dest_dev->name);
			}
		}
	}
#endif

#ifdef ECM_INTERFACE_PPTP_ENABLE
	/*
	 * if the address is a local address and indev=PPTP.
	 */
	if (protocol == IPPROTO_GRE && given_dest_dev && (given_dest_dev->priv_flags_ext & IFF_EXT_PPP_PPTP)){
		dev_put(dest_dev);
		dest_dev = given_dest_dev;
		if (dest_dev) {
			dev_hold(dest_dev);
			DEBUG_TRACE("%px: PPTP packet tunnel packet with dest_addr: " ECM_IP_ADDR_OCTAL_FMT " uses dev: %px(%s)\n", feci, ECM_IP_ADDR_TO_OCTAL(dest_addr), dest_dev, dest_dev->name);
		}
	}
#endif

#ifdef ECM_INTERFACE_VXLAN_ENABLE
	/*
	 * if the address is a local address and indev=VxLAN.
	 */
	if (from_local_addr &&
	    given_dest_dev &&
	    (given_dest_dev->type == ARPHRD_ETHER) &&
	    (netif_is_vxlan(given_dest_dev))) {
		dev_put(dest_dev);
		dest_dev = given_dest_dev;
		if (dest_dev) {
			dev_hold(dest_dev);
			DEBUG_TRACE("%px: VxLAN tunnel packet with dest_addr: " ECM_IP_ADDR_OCTAL_FMT " uses dev: %px(%s)\n", feci, ECM_IP_ADDR_TO_OCTAL(dest_addr), dest_dev, dest_dev->name);
		}
	}
#endif

	if (!dest_dev) {
		DEBUG_WARN("%px: dest_addr: " ECM_IP_ADDR_OCTAL_FMT " - cannot locate device\n", feci, ECM_IP_ADDR_TO_OCTAL(dest_addr));
		return ECM_DB_IFACE_HEIRARCHY_MAX;
	}
	dest_dev_name = dest_dev->name;
	dest_dev_type = dest_dev->type;

	/*
	 * Get device to reach the given source address.
	 * If the heirarchy is for a routed connection we must use the devices obtained from the skb's route information..
	 * If the heirarchy is NOT for a routed connection we try the given_src_dev.
	 */
	from_local_addr = false;
	if (!is_routed) {
		src_dev = given_src_dev;
		dev_hold(src_dev);
	} else {
		src_dev = ecm_interface_dev_find_by_local_addr(src_addr);
		if (src_dev) {
			from_local_addr = true;
		} else {
			src_dev = other_if;
			dev_hold(src_dev);
		}
	}

	/*
	 * If the address is a local address and protocol is an IP tunnel
	 * then this connection is a tunnel endpoint made to this device.
	 * In which case we circumvent all proper procedure and just hack the devices to make stuff work.
	 *
	 * TODO THIS MUST BE FIXED - WE MUST USE THE INTERFACE HIERARCHY FOR ITS INTENDED PURPOSE TO
	 * PARSE THE DEVICES AND WORK OUT THE PROPER INTERFACES INVOLVED.
	 * E.G. IF WE TRIED TO RUN A TUNNEL OVER A VLAN OR QINQ THIS WILL BREAK AS WE DON'T DISCOVER THAT HIERARCHY
	 */
	if (src_dev && from_local_addr) {
		if (ecm_interface_is_tunnel_endpoint(skb, given_src_dev, ip_version, protocol)) {
			dev_put(src_dev);
			src_dev = given_src_dev;
			if (src_dev) {
				dev_hold(src_dev);
				if (ip_version == 4) {
					DEBUG_TRACE("%px: HACK: %s tunnel packet with src_addr: " ECM_IP_ADDR_DOT_FMT " uses dev: %px(%s)\n", feci, "IPV4", ECM_IP_ADDR_TO_DOT(src_addr), src_dev, src_dev->name);
				} else {
					DEBUG_TRACE("%px: HACK: %s tunnel packet with src_addr: " ECM_IP_ADDR_OCTAL_FMT " uses dev: %px(%s)\n", feci, "IPV6", ECM_IP_ADDR_TO_OCTAL(src_addr), src_dev, src_dev->name);
				}
			}
		}
	}

	if (!src_dev) {
		DEBUG_WARN("%px: src_addr: " ECM_IP_ADDR_OCTAL_FMT " - cannot locate device\n", feci, ECM_IP_ADDR_TO_OCTAL(src_addr));
		dev_put(dest_dev);
		return ECM_DB_IFACE_HEIRARCHY_MAX;
	}
	src_dev_name = src_dev->name;
	src_dev_type = src_dev->type;

	/*
	 * Check if source and dest dev are same.
	 */
	if (src_dev == dest_dev) {
		bool skip = false;

		DEBUG_TRACE("%px: Protocol is :%d source dev and dest dev are same\n", feci, protocol);

		switch (ip_version) {
		case 4:
			/*
			 * For bridge flow we may hit this condition, and we will fail to create
			 * interface hierarchy for IPSEC passthrough / UDP Encapsulated IPSEC traffic. Hence making
			 * the check specific to routed flow in case of IPSEC passthrough traffic.
			 */
			if ((protocol == IPPROTO_IPV6) || ((protocol == IPPROTO_ESP) && is_routed)) {
				skip = true;
				break;
			}

#ifdef ECM_XFRM_ENABLE
			if (given_src_dev->type == ECM_ARPHRD_IPSEC_TUNNEL_TYPE && protocol == IPPROTO_UDP &&
					(IPCB(skb)->flags & IPSKB_XFRM_TRANSFORMED)) {
				skip = true;
				break;
			}
#else
			if (is_routed && ((protocol == IPPROTO_UDP) && (udp_hdr(skb)->dest == htons(4500)))) {
				skip = true;
				break;
			}
#endif
			break;

		case 6:
			/*
			 * For bridge flow we may hit this condition, and we will fail to create
			 * interface hierarchy for IPSEC passthrough / UDP Encapsulated IPSEC traffic. Hence making
			 * the check specific to routed flow in case of IPSEC passthrough traffic.
			 */
			if ((protocol == IPPROTO_IPIP) || ((protocol == IPPROTO_ESP) && is_routed)) {
				skip = true;
				break;
			}

			break;

		default:
			DEBUG_WARN("%px: IP version = %d, Protocol = %d: Corrupted packet entered ecm\n", feci, ip_version, protocol);
			skip = true;
			break;
		}

		if (skip) {
			/*
			 * This happens from the input hook
			 * We do not want to create a connection entry for this
			 * TODO YES WE DO.
			 * TODO THIS CONCERNS ME AS THIS SHOULD BE CAUGHT MUCH
			 * EARLIER IN THE FRONT END IF POSSIBLE TO AVOID PERFORMANCE PENALTIES.
			 * WE HAVE DONE A TREMENDOUS AMOUT OF WORK TO GET TO THIS POINT.
			 * WE WILL ABORT HERE AND THIS WILL BE REPEATED FOR EVERY PACKET.
			 * IN KEEPING WITH THE ECM DESIGN IT IS BETTER TO CREATE A CONNECTION AND RECORD IN THE HIERARCHY
			 * ENOUGH INFORMATION TO ENSURE THAT ACCELERATION IS NOT BROKEN / DOES NOT OCCUR AT ALL.
			 * THAT WAY WE DO A HEAVYWEIGHT ESTABLISHING OF A CONNECTION ONCE AND NEVER AGAIN...
			 */
			dev_put(src_dev);
			dev_put(dest_dev);
			return ECM_DB_IFACE_HEIRARCHY_MAX;
		}
	}

	bridge = ecm_interface_should_update_egress_device_bridged(
		given_dest_dev, dest_dev, is_routed);

	if (bridge) {
		struct net_device *new_dest_dev;
		new_dest_dev = br_port_dev_get(bridge, dest_node_addr, skb, serial);
		if (new_dest_dev) {
			dev_put(dest_dev);
			if (new_dest_dev != given_dest_dev) {
				DEBUG_INFO("%px: Adjusted port for %pM is %s (given was %s)\n",
					feci, dest_node_addr, new_dest_dev->name,
					given_dest_dev->name);

				dest_dev = new_dest_dev;
				dest_dev_name = dest_dev->name;
				dest_dev_type = dest_dev->type;
			}
		}
		dev_put(bridge);
	}

	next_dest_addr_valid = true;
	ECM_IP_ADDR_COPY(next_dest_addr, dest_addr);

	/*
	 * Iterate until we are done or get to the max number of interfaces we can record.
	 * NOTE: current_interface_index tracks the position of the first interface position in interfaces[]
	 * because we add from the end first_interface grows downwards.
	 */
	current_interface_index = ECM_DB_IFACE_HEIRARCHY_MAX;
	while (current_interface_index > 0) {
		struct ecm_db_iface_instance *ii;
		struct net_device *next_dev;
		/*
		 * Get the ecm db interface instance for the device at hand
		 */
		ii = ecm_interface_establish_and_ref(feci, dest_dev, skb);

		/*
		 * If the interface could not be established then we abort
		 */
		if (!ii) {
			DEBUG_WARN("%px: Failed to establish interface: %px, name: %s\n", feci, dest_dev, dest_dev_name);
			dev_put(src_dev);
			dev_put(dest_dev);

			/*
			 * Release the interfaces heirarchy we constructed to this point.
			 */
			ecm_db_connection_interfaces_deref(interfaces, current_interface_index);
			return ECM_DB_IFACE_HEIRARCHY_MAX;
		}

		/*
		 * Record the interface instance into the interfaces[]
		 */
		current_interface_index--;
		interfaces[current_interface_index] = ii;

		/*
		 * Now we have to figure out what the next device will be (in the transmission path) the skb
		 * will use to emit to the destination address.
		 */
		do {
#ifdef ECM_INTERFACE_PPP_ENABLE
			int channel_count;
			struct ppp_channel *ppp_chan[1];
			int channel_protocol;
#ifdef ECM_INTERFACE_PPPOE_ENABLE
			struct pppoe_opt addressing;
#endif
#endif

			DEBUG_TRACE("%px: Net device: %px is type: %d, name: %s\n", feci, dest_dev, dest_dev_type, dest_dev_name);
			next_dev = NULL;

			if (dest_dev_type == ARPHRD_ETHER) {
				/*
				 * Ethernet - but what sub type?
				 */

#ifdef ECM_INTERFACE_VLAN_ENABLE
				/*
				 * VLAN?
				 */
				if (is_vlan_dev(dest_dev)) {
					/*
					 * VLAN master
					 * No locking needed here, ASSUMPTION is that real_dev is held for as long as we have dev.
					 */
					next_dev = ecm_interface_vlan_real_dev(dest_dev);
					dev_hold(next_dev);
					DEBUG_TRACE("%px: Net device: %px is VLAN, slave dev: %px (%s)\n",
							feci, dest_dev, next_dev, next_dev->name);
					if (current_interface_index == (ECM_DB_IFACE_HEIRARCHY_MAX - 1)) {
						top_dev = dest_dev;
					}
					break;
				}
#endif

#ifdef ECM_INTERFACE_MACVLAN_ENABLE
				/*
				 * MAC-VLAN?
				 */
				if (netif_is_macvlan(dest_dev)) {
					if (ecm_interface_macvlan_mode_is_valid(dest_dev)) {
						next_dev = macvlan_dev_real_dev(dest_dev);
						dev_hold(next_dev);
						DEBUG_TRACE("%px: Net device: %px is MAC-VLAN, slave dev: %px (%s)\n",
								feci, dest_dev, next_dev, next_dev->name);

						/*
						 * We need to take the master_dev's MAC address during
						 * NSS rule push. During VLAN/MACVLAN the master_dev for the
						 * physical interface will be set to NULL but still we need the
						 * VLAN/MACVLAN dev's MAC address while pushing the rule. We identify
						 * the same using top_dev variable.
						 */
						if (current_interface_index == (ECM_DB_IFACE_HEIRARCHY_MAX - 1)) {
							top_dev = dest_dev;
						}
						break;
					}

					DEBUG_WARN("%px: Net device %px MACVLAN mode is not supported.\n", feci, dest_dev);
					goto done;
				}
#endif

				/*
				 * BRIDGE?
				 */
				if (ecm_front_end_is_bridge_device(dest_dev)) {
					/*
					 * Bridge
					 * Figure out which port device the skb will go to using the dest_addr.
					 */
					uint8_t mac_addr[ETH_ALEN];

					if (next_dest_node_addr_valid) {
						memcpy(mac_addr, next_dest_node_addr, ETH_ALEN);
					} else if (!next_dest_addr_valid) {
						dev_put(src_dev);
						dev_put(dest_dev);

						/*
						 * Release the interfaces heirarchy we constructed to this point.
						 */
						ecm_db_connection_interfaces_deref(interfaces, current_interface_index);
						return ECM_DB_IFACE_HEIRARCHY_MAX;
					} else {
						ip_addr_t look_up_addr;
						struct net_device *tmp_dev;
						ECM_IP_ADDR_COPY(look_up_addr, dest_addr);
						/*
						 * If this is a local IP address, this means the interface hierarchy is being created for
						 * TO_NAT or FROM_NAT direction. In these cases, since the IP address is a local IP address,
						 * MAC lookup will find the local interface's MAC address which cannot be used for the slave port
						 * look up in the forwarding database. So, we use the src_ip address for MAC look up.
						 * For TO_NAT direction, src_ip address is the sender host's IP address. For FROM_NAT direction
						 * src_ip address is the destination host's IP address.
						 * FROM_NAT ---- > HOST (src_ip)
						 * TO_NAT <----- HOST (src_ip)
						 */
						tmp_dev = ecm_interface_dev_find_by_local_addr(dest_addr);
						if (tmp_dev) {
							ECM_IP_ADDR_COPY(look_up_addr, src_addr);
							dev_put(tmp_dev);
						}

						if (!ecm_interface_get_next_node_mac_address(look_up_addr, dest_dev, ip_version, mac_addr)) {
							DEBUG_WARN("%px: Unable to find the host MAC address connected to the Linux bridge\n", feci);
							goto done;
						}
					}

					next_dev = br_port_dev_get(dest_dev,
						mac_addr, skb, serial);

					if (!next_dev) {
						DEBUG_WARN("%px: Unable to obtain output port for: %pM\n", feci, mac_addr);
						goto done;
					}

					DEBUG_TRACE("%px: Net device: %px is BRIDGE, next_dev: %px (%s)\n", feci, dest_dev, next_dev, next_dev->name);

					if (current_interface_index == (ECM_DB_IFACE_HEIRARCHY_MAX - 1)) {
						top_dev = dest_dev;
					}
					break;
				}

#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
				if (ovsmgr_is_ovs_master(dest_dev)) {
					ip_addr_t look_up_addr;
					uint8_t mac_addr[ETH_ALEN];
					struct net_device *tmp_dev;
					ECM_IP_ADDR_COPY(look_up_addr, dest_addr);

					/*
					 * If this is a local IP address, this means the interface hierarchy is being created for
					 * TO_NAT or FROM_NAT direction. In these cases, since the IP address is a local IP address,
					 * MAC lookup will find the local interface's MAC address which cannot be used for the slave port
					 * look up in the forwarding database. So, we use the src_ip address for MAC look up.
					 * For TO_NAT direction, src_ip address is the sender host's IP address. For FROM_NAT direction
					 * src_ip address is the destination host's IP address.
					 * FROM_NAT ---- > HOST (src_ip)
					 * TO_NAT <----- HOST (src_ip)
					 */
					tmp_dev = ecm_interface_dev_find_by_local_addr(dest_addr);
					if (tmp_dev) {
						ECM_IP_ADDR_COPY(look_up_addr, src_addr);
						dev_put(tmp_dev);
					}

					if (!ecm_interface_get_next_node_mac_address(look_up_addr, dest_dev, ip_version, mac_addr)) {
						DEBUG_WARN("%px: Unable to find the host MAC address connected to the OVS bridge\n", feci);
						goto done;
					}

					next_dev = ecm_interface_ovs_bridge_port_dev_get_and_ref(skb, dest_dev, src_addr, dest_addr, ip_version, protocol,
											 is_routed, src_node_addr, mac_addr, layer4hdr, op);
					if (!next_dev) {
						DEBUG_WARN("%px: Unable to obtain OVS output port for: %pM\n", feci, mac_addr);
						goto done;
					}

					DEBUG_TRACE("%px: Net device: %px is OVS BRIDGE, next_dev: %px (%s)\n", feci, dest_dev, next_dev, next_dev->name);

					if (current_interface_index == (ECM_DB_IFACE_HEIRARCHY_MAX - 1)) {
						top_dev = dest_dev;
					}
					break;
				}
#endif

#ifdef ECM_INTERFACE_BOND_ENABLE
				/*
				 * LAG?
				 */
				if (ecm_front_end_is_lag_master(dest_dev)) {
					/*
					 * Link aggregation
					 * Figure out whiich slave device of the link aggregation will be used to reach the destination.
					 */
					uint32_t src_addr_32 = 0;
					uint32_t dest_addr_32 = 0;
					struct in6_addr src_addr6;
					struct in6_addr dest_addr6;
					uint8_t src_mac_addr[ETH_ALEN];
					uint8_t dest_mac_addr[ETH_ALEN];

					memset(src_mac_addr, 0, ETH_ALEN);
					memset(dest_mac_addr, 0, ETH_ALEN);

					if (ip_version == 4) {
						ECM_IP_ADDR_TO_NIN4_ADDR(src_addr_32, src_addr);
						ECM_IP_ADDR_TO_NIN4_ADDR(dest_addr_32, real_dest_addr);
					}

					if (!is_routed) {
						memcpy(src_mac_addr, src_node_addr, ETH_ALEN);
						memcpy(dest_mac_addr, dest_node_addr, ETH_ALEN);
					} else {
						struct net_device *master_dev;

						/*
						 * Use appropriate source MAC address for routed packets and
						 * find proper interface to find the destination mac address and
						 * from which to issue ARP or neighbour solicitation packet.
						 */
						master_dev = ecm_interface_get_and_hold_dev_master(dest_dev);
						if (master_dev) {
							memcpy(src_mac_addr, master_dev->dev_addr, ETH_ALEN);
						} else {
							master_dev = dest_dev;
							if (top_dev) {
								master_dev = top_dev;
							}
							memcpy(src_mac_addr, master_dev->dev_addr, ETH_ALEN);
							dev_hold(master_dev);
						}

						/*
						 * Determine destination MAC address for this routed packet
						 */
						if (next_dest_node_addr_valid) {
							memcpy(dest_mac_addr, next_dest_node_addr, ETH_ALEN);
						} else if (!next_dest_addr_valid) {
							dev_put(src_dev);
							dev_put(dest_dev);
							dev_put(master_dev);
							ecm_db_connection_interfaces_deref(interfaces, current_interface_index);
							return ECM_DB_IFACE_HEIRARCHY_MAX;
						} else {
							if (!ecm_interface_mac_addr_get_no_route(master_dev, dest_addr, dest_mac_addr)) {
								ip_addr_t gw_addr = ECM_IP_ADDR_NULL;
								/*
								 * Try one more time with gateway ip address if it exists.
								 */
								if (!ecm_interface_find_gateway(dest_addr, gw_addr)) {
									goto lag_fail;
								}

								if (ip_version == 4) {
									DEBUG_TRACE("%px: Have a gw address " ECM_IP_ADDR_DOT_FMT "\n", feci, ECM_IP_ADDR_TO_DOT(gw_addr));
								}
#ifdef ECM_IPV6_ENABLE

								if (ip_version == 6) {
									DEBUG_TRACE("%px: Have a gw address " ECM_IP_ADDR_OCTAL_FMT "\n", feci, ECM_IP_ADDR_TO_OCTAL(gw_addr));
								}
#endif
								if (ecm_interface_mac_addr_get_no_route(master_dev, gw_addr, dest_mac_addr)) {
									DEBUG_TRACE("%px: Found the mac address for gateway\n", feci);
									dev_put(master_dev);
									goto lag_success;
								}

								if (ip_version == 4) {
									ecm_interface_send_arp_request(master_dev, dest_addr, false, gw_addr);

									DEBUG_WARN("%px: Unable to obtain any MAC address for " ECM_IP_ADDR_DOT_FMT "\n", feci, ECM_IP_ADDR_TO_DOT(dest_addr));
								}
#ifdef ECM_IPV6_ENABLE
								/*
								 * If there is a gw on the link, send the neighbor solicitation
								 * message to that address.
								 */
								if (!ECM_IP_ADDR_IS_NULL(gw_addr)) {
									ECM_IP_ADDR_COPY(dest_addr, gw_addr);
								}

								if (ip_version == 6) {
									ecm_interface_send_neighbour_solicitation(master_dev, dest_addr);

									DEBUG_WARN("%px: Unable to obtain any MAC address for " ECM_IP_ADDR_OCTAL_FMT "\n", feci, ECM_IP_ADDR_TO_OCTAL(dest_addr));
								}
#endif
lag_fail:
								dev_put(src_dev);
								dev_put(dest_dev);
								dev_put(master_dev);

								ecm_db_connection_interfaces_deref(interfaces, current_interface_index);
								return ECM_DB_IFACE_HEIRARCHY_MAX;
							}
						}
						dev_put(master_dev);
					}
lag_success:
					if (ip_version == 4) {
						next_dev = bond_get_tx_dev(NULL, src_mac_addr, dest_mac_addr,
										&src_addr_32, &dest_addr_32,
										htons((uint16_t)ETH_P_IP), dest_dev, layer4hdr);
					} else if (ip_version == 6) {
						ECM_IP_ADDR_TO_NIN6_ADDR(src_addr6, src_addr);
						ECM_IP_ADDR_TO_NIN6_ADDR(dest_addr6, real_dest_addr);
						next_dev = bond_get_tx_dev(NULL, src_mac_addr, dest_mac_addr,
									   src_addr6.s6_addr, dest_addr6.s6_addr,
									   htons((uint16_t)ETH_P_IPV6), dest_dev, layer4hdr);
					}

					if (next_dev && netif_carrier_ok(next_dev)) {
						dev_hold(next_dev);
					} else {
						DEBUG_WARN("%px: Unable to obtain LAG output slave device\n", feci);
						dev_put(src_dev);
						dev_put(dest_dev);

						/*
						 * Release the interfaces heirarchy we constructed to this point.
						 */
						ecm_db_connection_interfaces_deref(interfaces, current_interface_index);
						return ECM_DB_IFACE_HEIRARCHY_MAX;
					}

					DEBUG_TRACE("%px: Net device: %px is LAG, slave dev: %px (%s)\n", feci, dest_dev, next_dev, next_dev->name);
					break;
				}
#endif

				/*
				 * ETHERNET!
				 * Just plain ethernet it seems.
				 */
				DEBUG_TRACE("%px: Net device: %px is ETHERNET\n", feci, dest_dev);
				break;
			}

#ifdef ECM_INTERFACE_GRE_TUN_ENABLE
			/*
			 * GRE Tunnel?
			 */
			if ((dest_dev_type == ARPHRD_IPGRE) || (dest_dev_type == ARPHRD_IP6GRE)) {
				DEBUG_TRACE("%px: Net device: %px is GRE Tunnel type: %d\n", feci, dest_dev, dest_dev_type);
				break;
			}
#endif

			/*
			 * LOOPBACK?
			 */
			if (dest_dev_type == ARPHRD_LOOPBACK) {
				DEBUG_TRACE("%px: Net device: %px is LOOPBACK type: %d\n", feci, dest_dev, dest_dev_type);
				break;
			}

			/*
			 * IPSEC?
			 */
			if (dest_dev_type == ECM_ARPHRD_IPSEC_TUNNEL_TYPE) {
				DEBUG_TRACE("%px: Net device: %px is IPSec tunnel type: %d\n", feci, dest_dev, dest_dev_type);
				/* TODO Figure out the next device the tunnel is using... */
				break;
			}

			/*
			 * SIT (6-in-4)?
			 */
			if (dest_dev_type == ARPHRD_SIT) {
				DEBUG_TRACE("%px: Net device: %px is SIT (6-in-4) type: %d\n", feci, dest_dev, dest_dev_type);
				/* TODO Figure out the next device the tunnel is using... */
				break;
			}

			/*
			 * IPIP6 Tunnel?
			 */
			if (dest_dev_type == ARPHRD_TUNNEL6) {
				DEBUG_TRACE("%px: Net device: %px is TUNIPIP6 type: %d\n", feci, dest_dev, dest_dev_type);
				/* TODO Figure out the next device the tunnel is using... */
				break;
			}

#ifdef ECM_INTERFACE_MAP_T_ENABLE
			/*
			 * MAP-T xlate ?
			 */
			if (dest_dev_type == ARPHRD_NONE) {
				if (is_map_t_dev(dest_dev)) {
					DEBUG_TRACE("%px: Net device: %px is MAP-T type: %d\n", feci, dest_dev, dest_dev_type);
					break;
				}
			}
#endif

#ifdef ECM_INTERFACE_RAWIP_ENABLE
			/*
			 * RAWIP?
			 * If it is RAWIP type, there is no next device.
			 */
			if (dest_dev_type == ARPHRD_RAWIP) {
				DEBUG_TRACE("%px: Net device: %px is RAWIP type: %d\n", feci, dest_dev, dest_dev_type);
				break;
			}
#endif
#ifdef ECM_INTERFACE_OVPN_ENABLE
			/*
			 * OVPN ?
			 */
			if ((dest_dev_type == ARPHRD_NONE) && (dest_dev->priv_flags_ext & IFF_EXT_TUN_TAP)) {
				DEBUG_TRACE("Net device: %px is OVPN, device name: %s\n", dest_dev, dest_dev->name);
				break;
			}
#endif
			/*
			 * If this is NOT PPP then it is unknown to the ecm and we cannot figure out it's next device.
			 */
			if (dest_dev_type != ARPHRD_PPP) {
				DEBUG_TRACE("%px: Net device: %px is UNKNOWN type: %d\n", feci, dest_dev, dest_dev_type);
				break;
			}

#ifndef ECM_INTERFACE_PPP_ENABLE
			DEBUG_TRACE("%px: Net device: %px is UNKNOWN (PPP Unsupported) type: %d\n", feci, dest_dev, dest_dev_type);
#else
			DEBUG_TRACE("%px: Net device: %px is PPP\n", feci, dest_dev);

#ifdef ECM_INTERFACE_L2TPV2_ENABLE
			if ((given_src_dev->priv_flags_ext & IFF_EXT_PPP_L2TPV2) && ppp_is_xmit_locked(given_src_dev)) {
				if (skb->skb_iif == dest_dev->ifindex) {
					DEBUG_TRACE("%px: Net device: %px PPP channel is PPPoL2TPV2\n", feci, dest_dev);
					break;
				}
			}
#endif

#ifdef ECM_INTERFACE_PPTP_ENABLE
			if (protocol == IPPROTO_GRE && dest_dev && (dest_dev->priv_flags_ext & IFF_EXT_PPP_PPTP)) {
				DEBUG_TRACE("%px: Net device: %px PPP channel is PPTP\n", feci, dest_dev);
				break;
			}
#endif
			/*
			 * PPP - but what is the channel type?
			 * First: If this is multi-link then we do not support it
			 */
			if (ppp_is_multilink(dest_dev) > 0) {
				DEBUG_TRACE("%px: Net device: %px is MULTILINK PPP - Unknown to the ECM\n", feci, dest_dev);
				break;
			}

			/*
			 * Get the PPP channel and then enquire what kind of channel it is
			 * NOTE: Not multilink so only one channel to get.
			 */
			channel_count = ppp_hold_channels(dest_dev, ppp_chan, 1);
			if (channel_count != 1) {
				DEBUG_TRACE("%px: Net device: %px PPP has %d channels - Unknown to the ECM\n",
						feci, dest_dev, channel_count);
				break;
			}

			/*
			 * Get channel protocol type
			 * NOTE: Not all PPP channels support channel specific methods.
			 */
			channel_protocol = ppp_channel_get_protocol(ppp_chan[0]);

#ifdef ECM_INTERFACE_L2TPV2_ENABLE
			if (channel_protocol == PX_PROTO_OL2TP) {

				/*
				 * PPPoL2TPV2 channel
				 */
				ppp_release_channels(ppp_chan, 1);
				DEBUG_TRACE("%px: Net device: %px PPP channel is PPPoL2TPV2\n", feci, dest_dev);

				/*
				 * Release the channel.  Note that next_dev not held.
				 */
				break;
			}
#endif
#ifdef ECM_INTERFACE_PPPOE_ENABLE
			if (channel_protocol == PX_PROTO_OE) {
				/*
				 * PPPoE channel
				 */
				DEBUG_TRACE("%px: Net device: %px PPP channel is PPPoE\n", feci, dest_dev);

				/*
				 * Get PPPoE session information and the underlying device it is using.
				 */
				if (pppoe_channel_addressing_get(ppp_chan[0], &addressing)) {
					DEBUG_WARN("%px: failed to get PPPoE addressing info\n", feci);
					ppp_release_channels(ppp_chan, 1);
					break;
				}

				/*
				 * Copy the dev hold into this, we will release the hold later
				 */
				next_dev = addressing.dev;
				next_dest_addr_valid = false;
				next_dest_node_addr_valid = true;
				memcpy(next_dest_node_addr, addressing.pa.remote, ETH_ALEN);

				/*
				 * Release the channel.  Note that next_dev is still (correctly) held.
				 */
				ppp_release_channels(ppp_chan, 1);
				break;
			}
#endif

			DEBUG_TRACE("%px: Net device: %px PPP channel protocol: %d - Unknown to the ECM\n",
				    feci, dest_dev, channel_protocol);

			/*
			 * Release the channel
			 */
			ppp_release_channels(ppp_chan, 1);

#endif
		} while (false);

		/*
		 * No longer need dest_dev as it may become next_dev
		 */
		dev_put(dest_dev);

		/*
		 * Check out the next_dev, if any
		 */
		if (!next_dev) {
			int32_t i __attribute__((unused));
			DEBUG_INFO("%px: Completed interface heirarchy construct with first interface @: %d\n", feci, current_interface_index);
#if DEBUG_LEVEL > 1
			for (i = current_interface_index; i < ECM_DB_IFACE_HEIRARCHY_MAX; ++i) {
				DEBUG_TRACE("\t%px: Interface @ %d: %px, type: %d, name: %s\n",
						feci, i, interfaces[i], ecm_db_iface_type_get(interfaces[i]), ecm_db_interface_type_to_string(ecm_db_iface_type_get(interfaces[i])));

			}
#endif

			/*
			 * Release src_dev now
			 */
			dev_put(src_dev);
			return current_interface_index;
		}

		/*
		 * dest_dev becomes next_dev
		 */
		dest_dev = next_dev;
		dest_dev_name = dest_dev->name;
		dest_dev_type = dest_dev->type;
	}

	DEBUG_WARN("%px: Too many interfaces: %d\n", feci, current_interface_index);
	DEBUG_ASSERT(current_interface_index == 0, "%px: Bad logic handling current_interface_index: %d\n", feci, current_interface_index);

done:
	dev_put(src_dev);
	dev_put(dest_dev);

	/*
	 * Release the interfaces heirarchy we constructed to this point.
	 */
	ecm_db_connection_interfaces_deref(interfaces, current_interface_index);
	return ECM_DB_IFACE_HEIRARCHY_MAX;
}
EXPORT_SYMBOL(ecm_interface_heirarchy_construct);

#ifdef ECM_MULTICAST_ENABLE
/*
 * ecm_interface_multicast_from_heirarchy_construct()
 *	Construct an interface heirarchy.
 *
 * TODO: This function will be removed later and ecm_interface_heirarchy_construct() function
 *	 will be used when the multicast code is fixed to use the new interface hierarchy
 *	 construction model which uses the skb's route information instead of doing
 *	 the route lookup based on the IP addresses.
 *
 * Using the given addressing, locate the interface heirarchy used to emit packets to that destination.
 * This is the heirarchy of interfaces a packet would transit to emit from the device.
 *
 * We will use the given src/dest devices when is_routed is false.
 * When is_routed is true we will try routing tables first, failing back to any given.
 *
 * For example, with this network arrangement:
 *
 * PPPoE--VLAN--BRIDGE--BRIDGE_PORT(LAG_MASTER)--LAG_SLAVE_0--10.22.33.11
 *
 * Given the packet_dest_addr IP address 10.22.33.11 this will create an interface heirarchy (in interracfes[]) of:
 * LAG_SLAVE_0 @ [ECM_DB_IFACE_HEIRARCHY_MAX - 5]
 * LAG_MASTER @ [ECM_DB_IFACE_HEIRARCHY_MAX - 4]
 * BRIDGE @ [ECM_DB_IFACE_HEIRARCHY_MAX - 3]
 * VLAN @ [ECM_DB_IFACE_HEIRARCHY_MAX - 2]
 * PPPOE @ [ECM_DB_IFACE_HEIRARCHY_MAX - 1]
 * The value returned is (ECM_DB_IFACE_HEIRARCHY_MAX - 5)
 *
 * IMPORTANT: This function will return any known interfaces in the database, when interfaces do not exist in the database
 * they will be created and added automatically to the database.
 */
int32_t ecm_interface_multicast_from_heirarchy_construct(struct ecm_front_end_connection_instance *feci,
						struct ecm_db_iface_instance *interfaces[],
						ip_addr_t packet_src_addr,
						ip_addr_t packet_dest_addr,
						int ip_version, int packet_protocol,
						struct net_device *given_dest_dev,
						bool is_routed, struct net_device *given_src_dev,
						uint8_t *dest_node_addr, uint8_t *src_node_addr,
						__be16 *layer4hdr, struct sk_buff *skb)
{
	int protocol;
	ip_addr_t src_addr;
	ip_addr_t dest_addr;
	struct net_device *dest_dev;
	char *dest_dev_name;
	int32_t dest_dev_type;
	struct net_device *src_dev;
	char *src_dev_name;
	int32_t src_dev_type;
	int32_t current_interface_index;
	bool from_local_addr;
	bool next_dest_addr_valid;
	bool next_dest_node_addr_valid = false;
	ip_addr_t next_dest_addr;
	uint8_t next_dest_node_addr[ETH_ALEN] = {0};
	struct net_device *bridge;
	uint32_t serial = ecm_db_connection_serial_get(feci->ci);

	/*
	 * Get a big endian of the IPv4 address we have been given as our starting point.
	 */
	protocol = packet_protocol;
	ECM_IP_ADDR_COPY(src_addr, packet_src_addr);
	ECM_IP_ADDR_COPY(dest_addr, packet_dest_addr);

	if (ip_version == 4) {
		DEBUG_TRACE("Construct interface heirarchy for from src_addr: " ECM_IP_ADDR_DOT_FMT " to dest_addr: " ECM_IP_ADDR_DOT_FMT ", protocol: %d (serial %u)\n",
				ECM_IP_ADDR_TO_DOT(src_addr), ECM_IP_ADDR_TO_DOT(dest_addr), protocol,
				serial);
#ifdef ECM_IPV6_ENABLE
	} else if (ip_version == 6) {
		DEBUG_TRACE("Construct interface heirarchy for from src_addr: " ECM_IP_ADDR_OCTAL_FMT " to dest_addr: " ECM_IP_ADDR_OCTAL_FMT ", protocol: %d (serial %u)\n",
				ECM_IP_ADDR_TO_OCTAL(src_addr), ECM_IP_ADDR_TO_OCTAL(dest_addr), protocol,
				serial);
#endif
	} else {
		DEBUG_WARN("Wrong IP protocol: %d\n", ip_version);
		return ECM_DB_IFACE_HEIRARCHY_MAX;
	}

	/*
	 * Get device to reach the given destination address.
	 * If the heirarchy is for a routed connection we must try route lookup first, falling back to any given_dest_dev.
	 * If the heirarchy is NOT for a routed connection we try the given_dest_dev first, followed by routed lookup.
	 */
	from_local_addr = false;
	if (is_routed) {
		dest_dev = ecm_interface_dev_find_by_addr(dest_addr, &from_local_addr);
		if (!dest_dev && given_dest_dev) {
			/*
			 * Fall back to any given
			 */
			dest_dev = given_dest_dev;
			dev_hold(dest_dev);
		}
	} else if (given_dest_dev) {
		dest_dev = given_dest_dev;
		dev_hold(dest_dev);
	} else {
		/*
		 * Fall back to routed look up
		 */
		dest_dev = ecm_interface_dev_find_by_addr(dest_addr, &from_local_addr);
	}

	/*
	 * GGG ALERT: If the address is a local address and protocol is an IP tunnel
	 * then this connection is a tunnel endpoint made to this device.
	 * In which case we circumvent all proper procedure and just hack the devices to make stuff work.
	 * GGG TODO THIS MUST BE FIXED - WE MUST USE THE INTERFACE HIERARCHY FOR ITS INTENDED PURPOSE TO
	 * PARSE THE DEVICES AND WORK OUT THE PROPER INTERFACES INVOLVED.
	 * E.G. IF WE TRIED TO RUN A TUNNEL OVER A VLAN OR QINQ THIS WILL BREAK AS WE DON'T DISCOVER THAT HIERARCHY
	 */
	if (dest_dev && from_local_addr) {
		if (((ip_version == 4) && (protocol == IPPROTO_IPV6)) ||
				((ip_version == 6) && (protocol == IPPROTO_IPIP))) {
			dev_put(dest_dev);
			dest_dev = given_dest_dev;
			if (dest_dev) {
				dev_hold(dest_dev);
				if (ip_version == 4) {
					DEBUG_TRACE("HACK: %s tunnel packet with dest_addr: " ECM_IP_ADDR_DOT_FMT " uses dev: %px(%s)\n", "IPV6", ECM_IP_ADDR_TO_DOT(dest_addr), dest_dev, dest_dev->name);
				} else {
					DEBUG_TRACE("HACK: %s tunnel packet with dest_addr: " ECM_IP_ADDR_OCTAL_FMT " uses dev: %px(%s)\n", "IPIP", ECM_IP_ADDR_TO_OCTAL(dest_addr), dest_dev, dest_dev->name);
				}
			}
		}
	}

#ifdef ECM_INTERFACE_L2TPV2_ENABLE
	/*
	 * if the address is a local address and indev=l2tp.
	 */
	if ((given_src_dev->type == ARPHRD_PPP) && (given_src_dev->priv_flags_ext & IFF_EXT_PPP_L2TPV2) && ppp_is_xmit_locked(given_src_dev)) {
		if (!dst_xfrm(skb_dst(skb)) || (dest_dev->type != ECM_ARPHRD_IPSEC_TUNNEL_TYPE)) {
			dev_put(dest_dev);
			dest_dev = given_dest_dev;
			if (dest_dev) {
				dev_hold(dest_dev);
				DEBUG_TRACE("l2tp packet tunnel packet with dest_addr: " ECM_IP_ADDR_OCTAL_FMT " uses dev: %px(%s)\n", ECM_IP_ADDR_TO_OCTAL(dest_addr), dest_dev, dest_dev->name);
			}
		}
	}
#endif

#ifdef ECM_INTERFACE_PPTP_ENABLE
	/*
	 * if the address is a local address and indev=PPTP.
	 */
	if (protocol == IPPROTO_GRE && given_dest_dev && given_dest_dev->type == ARPHRD_PPP) {
		dev_put(dest_dev);
		dest_dev = given_dest_dev;
		if (dest_dev) {
			dev_hold(dest_dev);
			DEBUG_TRACE("PPTP packet tunnel packet with dest_addr: " ECM_IP_ADDR_OCTAL_FMT " uses dev: %px(%s)\n", ECM_IP_ADDR_TO_OCTAL(dest_addr), dest_dev, dest_dev->name);
		}
	}
#endif

	if (!dest_dev) {
		DEBUG_WARN("dest_addr: " ECM_IP_ADDR_OCTAL_FMT " - cannot locate device\n", ECM_IP_ADDR_TO_OCTAL(dest_addr));
		return ECM_DB_IFACE_HEIRARCHY_MAX;
	}
	dest_dev_name = dest_dev->name;
	dest_dev_type = dest_dev->type;

	/*
	 * Get device to reach the given source address.
	 * If the heirarchy is for a routed connection we must try route lookup first, falling back to any given_src_dev.
	 * If the heirarchy is NOT for a routed connection we try the given_src_dev first, followed by routed lookup.
	 */
	from_local_addr = false;
	if (is_routed) {
		src_dev = ecm_interface_dev_find_by_addr(src_addr, &from_local_addr);
		if (!src_dev && given_src_dev) {
			/*
			 * Fall back to any given
			 */
			src_dev = given_src_dev;
			dev_hold(src_dev);
		}
	} else if (given_src_dev) {
		src_dev = given_src_dev;
		dev_hold(src_dev);
	} else {
		/*
		 * Fall back to routed look up
		 */
		src_dev = ecm_interface_dev_find_by_addr(src_addr, &from_local_addr);
	}

	/*
	 * GGG ALERT: If the address is a local address and protocol is an IP tunnel
	 * then this connection is a tunnel endpoint made to this device.
	 * In which case we circumvent all proper procedure and just hack the devices to make stuff work.
	 * GGG TODO THIS MUST BE FIXED - WE MUST USE THE INTERFACE HIERARCHY FOR ITS INTENDED PURPOSE TO
	 * PARSE THE DEVICES AND WORK OUT THE PROPER INTERFACES INVOLVED.
	 * E.G. IF WE TRIED TO RUN A TUNNEL OVER A VLAN OR QINQ THIS WILL BREAK AS WE DON'T DISCOVER THAT HIERARCHY
	 */
	if (src_dev && from_local_addr) {
		if (((ip_version == 4) && (protocol == IPPROTO_IPV6)) ||
				((ip_version == 6) && (protocol == IPPROTO_IPIP))) {
			dev_put(src_dev);
			src_dev = given_src_dev;
			if (src_dev) {
				dev_hold(src_dev);
				if (ip_version == 4) {
					DEBUG_TRACE("HACK: %s tunnel packet with src_addr: " ECM_IP_ADDR_DOT_FMT " uses dev: %px(%s)\n", "IPV6", ECM_IP_ADDR_TO_DOT(src_addr), src_dev, src_dev->name);
				} else {
					DEBUG_TRACE("HACK: %s tunnel packet with src_addr: " ECM_IP_ADDR_OCTAL_FMT " uses dev: %px(%s)\n", "IPIP", ECM_IP_ADDR_TO_OCTAL(src_addr), src_dev, src_dev->name);
				}
			}
		}
	}

#ifdef ECM_INTERFACE_L2TPV2_ENABLE
	/*
	 * if the address is a local address and indev=l2tp.
	 */
	if (skb && skb->sk && (skb->sk->sk_protocol == IPPROTO_UDP) && (udp_sk(skb->sk)->encap_type == UDP_ENCAP_L2TPINUDP)) {
		if (dest_dev != given_src_dev) {
			dev_put(src_dev);
			src_dev = given_src_dev;
			if (src_dev) {
				dev_hold(src_dev);
				DEBUG_TRACE("l2tp tunnel packet with src_addr: " ECM_IP_ADDR_OCTAL_FMT " uses dev: %px(%s)\n", ECM_IP_ADDR_TO_OCTAL(src_addr), src_dev, src_dev->name);
			}
		}
	}
#endif

	if (!src_dev) {
		DEBUG_WARN("src_addr: " ECM_IP_ADDR_OCTAL_FMT " - cannot locate device\n", ECM_IP_ADDR_TO_OCTAL(src_addr));
		dev_put(dest_dev);
		return ECM_DB_IFACE_HEIRARCHY_MAX;
	}
	src_dev_name = src_dev->name;
	src_dev_type = src_dev->type;

	/*
	 * Check if source and dest dev are same.
	 * For the forwarded flows which involve tunnels this will happen when called from input hook.
	 */
	if (src_dev == dest_dev) {
		DEBUG_TRACE("Protocol is :%d source dev and dest dev are same\n", protocol);
		if (((ip_version == 4) && ((protocol == IPPROTO_IPV6) || (protocol == IPPROTO_ESP)))
				|| ((ip_version == 6) && (protocol == IPPROTO_IPIP))) {
			/*
			 * This happens from the input hook
			 * We do not want to create a connection entry for this
			 * GGG TODO YES WE DO.
			 * GGG TODO THIS CONCERNS ME AS THIS SHOULD BE CAUGHT MUCH
			 * EARLIER IN THE FRONT END IF POSSIBLE TO AVOID PERFORMANCE PENALTIES.
			 * WE HAVE DONE A TREMENDOUS AMOUT OF WORK TO GET TO THIS POINT.
			 * WE WILL ABORT HERE AND THIS WILL BE REPEATED FOR EVERY PACKET.
			 * IN KEEPING WITH THE ECM DESIGN IT IS BETTER TO CREATE A CONNECTION AND RECORD IN THE HIERARCHY
			 * ENOUGH INFORMATION TO ENSURE THAT ACCELERATION IS NOT BROKEN / DOES NOT OCCUR AT ALL.
			 * THAT WAY WE DO A HEAVYWEIGHT ESTABLISHING OF A CONNECTION ONCE AND NEVER AGAIN...
			 */
			dev_put(src_dev);
			dev_put(dest_dev);
			return ECM_DB_IFACE_HEIRARCHY_MAX;
		}
	}

	bridge = ecm_interface_should_update_egress_device_bridged(
		given_dest_dev, dest_dev, is_routed);

	if (bridge) {
		struct net_device *new_dest_dev;
		new_dest_dev = br_port_dev_get(bridge, dest_node_addr, skb, serial);
		if (new_dest_dev) {
			dev_put(dest_dev);
			if (new_dest_dev != given_dest_dev) {
				DEBUG_INFO("Adjusted port for %pM is %s (given was %s)\n",
					dest_node_addr, new_dest_dev->name,
					given_dest_dev->name);

				dest_dev = new_dest_dev;
				dest_dev_name = dest_dev->name;
				dest_dev_type = dest_dev->type;
			}
		}
		dev_put(bridge);
	}

	next_dest_addr_valid = true;
	next_dest_node_addr_valid = false;
	ECM_IP_ADDR_COPY(next_dest_addr, dest_addr);

	/*
	 * Iterate until we are done or get to the max number of interfaces we can record.
	 * NOTE: current_interface_index tracks the position of the first interface position in interfaces[]
	 * because we add from the end first_interface grows downwards.
	 */
	current_interface_index = ECM_DB_IFACE_HEIRARCHY_MAX;
	while (current_interface_index > 0) {
		struct ecm_db_iface_instance *ii;
		struct net_device *next_dev;
		/*
		 * Get the ecm db interface instance for the device at hand
		 */
		ii = ecm_interface_establish_and_ref(feci, dest_dev, skb);

		/*
		 * If the interface could not be established then we abort
		 */
		if (!ii) {
			DEBUG_WARN("Failed to establish interface: %px, name: %s\n", dest_dev, dest_dev_name);
			dev_put(src_dev);
			dev_put(dest_dev);

			/*
			 * Release the interfaces heirarchy we constructed to this point.
			 */
			ecm_db_connection_interfaces_deref(interfaces, current_interface_index);
			return ECM_DB_IFACE_HEIRARCHY_MAX;
		}

		/*
		 * Record the interface instance into the interfaces[]
		 */
		current_interface_index--;
		interfaces[current_interface_index] = ii;

		/*
		 * Now we have to figure out what the next device will be (in the transmission path) the skb
		 * will use to emit to the destination address.
		 */
		do {
#ifdef ECM_INTERFACE_PPP_ENABLE
			int channel_count;
			struct ppp_channel *ppp_chan[1];
			int channel_protocol;
#ifdef ECM_INTERFACE_PPPOE_ENABLE
			struct pppoe_opt addressing;
#endif
#endif

			DEBUG_TRACE("Net device: %px is type: %d, name: %s\n", dest_dev, dest_dev_type, dest_dev_name);
			next_dev = NULL;

			if (dest_dev_type == ARPHRD_ETHER) {
				/*
				 * Ethernet - but what sub type?
				 */

#ifdef ECM_INTERFACE_GRE_TAP_ENABLE
				if (dest_dev->priv_flags_ext & (IFF_EXT_GRE_V4_TAP | IFF_EXT_GRE_V6_TAP)) {
					DEBUG_TRACE("%px: Acceleration not supported for GRE tap flows.\n", dest_dev);
					dev_put(src_dev);
					dev_put(dest_dev);
					ecm_db_connection_interfaces_deref(interfaces, current_interface_index);
					return ECM_DB_IFACE_HEIRARCHY_MAX;
				}
#endif

#ifdef ECM_INTERFACE_VLAN_ENABLE
				/*
				 * VLAN?
				 */
				if (is_vlan_dev(dest_dev)) {
					/*
					 * VLAN master
					 * No locking needed here, ASSUMPTION is that real_dev is held for as long as we have dev.
					 */
					next_dev = ecm_interface_vlan_real_dev(dest_dev);
					dev_hold(next_dev);
					DEBUG_TRACE("Net device: %px is VLAN, slave dev: %px (%s)\n",
							dest_dev, next_dev, next_dev->name);
					break;
				}
#endif

#ifdef ECM_INTERFACE_MACVLAN_ENABLE
				/*
				 * MAC-VLAN?
				 */
				if (netif_is_macvlan(dest_dev)) {
					if (ecm_interface_macvlan_mode_is_valid(dest_dev)) {
						next_dev = macvlan_dev_real_dev(dest_dev);
						dev_hold(next_dev);
						DEBUG_TRACE("%px: Net device: %px is MAC-VLAN, slave dev: %px (%s)\n",
								feci, dest_dev, next_dev, next_dev->name);
						break;
					}
				}
#endif

				/*
				 * BRIDGE?
				 */
				if (ecm_front_end_is_bridge_device(dest_dev)) {
					/*
					 * Bridge
					 * Figure out which port device the skb will go to using the dest_addr.
					 */
					uint8_t mac_addr[ETH_ALEN];

					if (next_dest_node_addr_valid) {
						memcpy(mac_addr, next_dest_node_addr, ETH_ALEN);
					} else if (!next_dest_addr_valid) {
						dev_put(src_dev);
						dev_put(dest_dev);

						/*
						 * Release the interfaces heirarchy we constructed to this point.
						 */
						ecm_db_connection_interfaces_deref(interfaces, current_interface_index);
						return ECM_DB_IFACE_HEIRARCHY_MAX;
					} else {
						if (!ecm_interface_multicast_get_next_node_mac_address(next_dest_addr, dest_dev, ip_version, mac_addr)) {
							dev_put(src_dev);
							dev_put(dest_dev);

							/*
							 * Release the interfaces heirarchy we constructed to this point.
							 */
							ecm_db_connection_interfaces_deref(interfaces, current_interface_index);
							return ECM_DB_IFACE_HEIRARCHY_MAX;
						}
					}
					next_dev = br_port_dev_get(dest_dev,
						mac_addr, skb, serial);
					if (!next_dev) {
						DEBUG_WARN("Unable to obtain output port for: %pM\n", mac_addr);
						dev_put(src_dev);
						dev_put(dest_dev);

						/*
						 * Release the interfaces heirarchy we constructed to this point.
						 */
						ecm_db_connection_interfaces_deref(interfaces, current_interface_index);
						return ECM_DB_IFACE_HEIRARCHY_MAX;
					}
					DEBUG_TRACE("Net device: %px is BRIDGE, next_dev: %px (%s)\n", dest_dev, next_dev, next_dev->name);
					break;
				}

#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
				/*
				 * OVS_BRIDGE?
				 */
				if (ecm_front_end_is_ovs_bridge_device(dest_dev)) {
					/*
					 * Bridge
					 * Figure out which port device the skb will go to using the dest_addr.
					 */
					uint8_t mac_addr[ETH_ALEN];

					if (!ecm_interface_multicast_get_next_node_mac_address(next_dest_addr, dest_dev, ip_version, mac_addr)) {
						dev_put(src_dev);
						dev_put(dest_dev);

						/*
						 * Release the interfaces hierarchy we constructed to this point.
						 */
						ecm_db_connection_interfaces_deref(interfaces, current_interface_index);
						return ECM_DB_IFACE_HEIRARCHY_MAX;
					}
					next_dev = ecm_interface_ovs_bridge_port_dev_get_and_ref(skb, dest_dev, src_addr, dest_addr, ip_version, protocol,
											 is_routed, src_node_addr, mac_addr, layer4hdr, NULL);
					if (!next_dev) {
						DEBUG_WARN("Unable to obtain output port for: %pM\n", mac_addr);
						dev_put(src_dev);
						dev_put(dest_dev);

						/*
						 * Release the interfaces hierarchy we constructed to this point.
						 */
						ecm_db_connection_interfaces_deref(interfaces, current_interface_index);
						return ECM_DB_IFACE_HEIRARCHY_MAX;
					}
					DEBUG_TRACE("Net device: %px is BRIDGE, next_dev: %px (%s)\n", dest_dev, next_dev, next_dev->name);
					break;
				}
#endif

#ifdef ECM_INTERFACE_BOND_ENABLE
				/*
				 * LAG?
				 */
				if (ecm_front_end_is_lag_master(dest_dev)) {
					/*
					 * Link aggregation
					 * Figure out whiich slave device of the link aggregation will be used to reach the destination.
					 */
					bool dest_on_link = false;
					ip_addr_t dest_gw_addr = ECM_IP_ADDR_NULL;
					uint32_t src_addr_32 = 0;
					uint32_t dest_addr_32 = 0;
					struct in6_addr src_addr6;
					struct in6_addr dest_addr6;
					uint8_t src_mac_addr[ETH_ALEN];
					uint8_t dest_mac_addr[ETH_ALEN];
					struct net_device *master_dev = NULL;

					memset(src_mac_addr, 0, ETH_ALEN);
					memset(dest_mac_addr, 0, ETH_ALEN);

					if (ip_version == 4) {
						ECM_IP_ADDR_TO_NIN4_ADDR(src_addr_32, src_addr);
						ECM_IP_ADDR_TO_NIN4_ADDR(dest_addr_32, dest_addr);
					}

					if (!is_routed) {
						memcpy(src_mac_addr, src_node_addr, ETH_ALEN);
						memcpy(dest_mac_addr, dest_node_addr, ETH_ALEN);
					} else {
						struct net_device *dest_dev_master;

						/*
						 * Use appropriate source MAC address for routed packets
						 */
						dest_dev_master = ecm_interface_get_and_hold_dev_master(dest_dev);
						if (dest_dev_master) {
							memcpy(src_mac_addr, dest_dev_master->dev_addr, ETH_ALEN);
						} else {
							memcpy(src_mac_addr, dest_dev->dev_addr, ETH_ALEN);
						}

						/*
						 * Determine destination MAC address for this routed packet
						 */
						if (next_dest_node_addr_valid) {
							memcpy(dest_mac_addr, next_dest_node_addr, ETH_ALEN);
						} else if (!next_dest_addr_valid) {
							dev_put(src_dev);
							dev_put(dest_dev);
							if (dest_dev_master) {
								dev_put(dest_dev_master);
							}

							ecm_db_connection_interfaces_deref(interfaces, current_interface_index);
							return ECM_DB_IFACE_HEIRARCHY_MAX;
						} else {
							if (!ecm_interface_mac_addr_get(dest_addr, dest_mac_addr,
										&dest_on_link, dest_gw_addr)) {

								/*
								 * Find proper interfce from which to issue ARP
								 * or neighbour solicitation packet.
								 */
								if (dest_dev_master) {
									master_dev = dest_dev_master;
								} else {
									master_dev = dest_dev;
								}

								dev_hold(master_dev);

								if (dest_dev_master) {
									dev_put(dest_dev_master);
								}

								if (ip_version == 4) {
									DEBUG_WARN("Unable to obtain MAC address for " ECM_IP_ADDR_DOT_FMT "\n", ECM_IP_ADDR_TO_DOT(dest_addr));
									ecm_interface_send_arp_request(dest_dev, dest_addr, dest_on_link, dest_gw_addr);
								}
#ifdef ECM_IPV6_ENABLE
								/*
								 * If there is a gw on the link, send the neighbor solicitation
								 * message to that address.
								 */
								if (!ECM_IP_ADDR_IS_NULL(dest_gw_addr)) {
									ECM_IP_ADDR_COPY(dest_addr, dest_gw_addr);
								}

								if (ip_version == 6) {
									DEBUG_WARN("Unable to obtain MAC address for " ECM_IP_ADDR_OCTAL_FMT "\n", ECM_IP_ADDR_TO_OCTAL(dest_addr));
									ecm_interface_send_neighbour_solicitation(master_dev, dest_addr);
								}
#endif
								dev_put(src_dev);
								dev_put(dest_dev);
								dev_put(master_dev);
								ecm_db_connection_interfaces_deref(interfaces, current_interface_index);
								return ECM_DB_IFACE_HEIRARCHY_MAX;
							}
						}

						if (dest_dev_master) {
							dev_put(dest_dev_master);
						}
					}

					if (ip_version == 4) {
						next_dev = bond_get_tx_dev(NULL, src_mac_addr, dest_mac_addr,
										&src_addr_32, &dest_addr_32,
										htons((uint16_t)ETH_P_IP), dest_dev, layer4hdr);
					} else if (ip_version == 6) {
						ECM_IP_ADDR_TO_NIN6_ADDR(src_addr6, src_addr);
						ECM_IP_ADDR_TO_NIN6_ADDR(dest_addr6, dest_addr);
						next_dev = bond_get_tx_dev(NULL, src_mac_addr, dest_mac_addr,
									   src_addr6.s6_addr, dest_addr6.s6_addr,
									   htons((uint16_t)ETH_P_IPV6), dest_dev, layer4hdr);
					}

					if (next_dev && netif_carrier_ok(next_dev)) {
						dev_hold(next_dev);
					} else {
						DEBUG_WARN("Unable to obtain LAG output slave device\n");
						dev_put(src_dev);
						dev_put(dest_dev);

						/*
						 * Release the interfaces heirarchy we constructed to this point.
						 */
						ecm_db_connection_interfaces_deref(interfaces, current_interface_index);
						return ECM_DB_IFACE_HEIRARCHY_MAX;
					}

					DEBUG_TRACE("Net device: %px is LAG, slave dev: %px (%s)\n", dest_dev, next_dev, next_dev->name);
					break;
				}
#endif

				/*
				 * ETHERNET!
				 * Just plain ethernet it seems.
				 */
				DEBUG_TRACE("Net device: %px is ETHERNET\n", dest_dev);
				break;
			}

			/*
			 * LOOPBACK?
			 */
			if (dest_dev_type == ARPHRD_LOOPBACK) {
				DEBUG_TRACE("Net device: %px is LOOPBACK type: %d\n", dest_dev, dest_dev_type);
				break;
			}

			/*
			 * IPSEC?
			 */
			if (dest_dev_type == ECM_ARPHRD_IPSEC_TUNNEL_TYPE) {
				DEBUG_TRACE("Net device: %px is IPSec tunnel type: %d\n", dest_dev, dest_dev_type);
				/* TODO Figure out the next device the tunnel is using... */
				break;
			}

			/*
			 * SIT (6-in-4)?
			 */
			if (dest_dev_type == ARPHRD_SIT) {
				DEBUG_TRACE("Net device: %px is SIT (6-in-4) type: %d\n", dest_dev, dest_dev_type);
				/* TODO Figure out the next device the tunnel is using... */
				break;
			}

			/*
			 * IPIP6 Tunnel?
			 */
			if (dest_dev_type == ARPHRD_TUNNEL6) {
				DEBUG_TRACE("Net device: %px is TUNIPIP6 type: %d\n", dest_dev, dest_dev_type);
				/* TODO Figure out the next device the tunnel is using... */
				break;
			}

			/*
			 * If this is NOT PPP then it is unknown to the ecm and we cannot figure out it's next device.
			 */
			if (dest_dev_type != ARPHRD_PPP) {
				DEBUG_TRACE("Net device: %px is UNKNOWN type: %d\n", dest_dev, dest_dev_type);
				break;
			}

#ifndef ECM_INTERFACE_PPP_ENABLE
			DEBUG_TRACE("Net device: %px is UNKNOWN (PPP Unsupported) type: %d\n", dest_dev, dest_dev_type);
#else
			DEBUG_TRACE("Net device: %px is PPP\n", dest_dev);

#ifdef ECM_INTERFACE_L2TPV2_ENABLE
			if ((given_src_dev->priv_flags_ext & IFF_EXT_PPP_L2TPV2) && ppp_is_xmit_locked(given_src_dev)) {
				if (skb->skb_iif == dest_dev->ifindex) {
					DEBUG_TRACE("Net device: %px PPP channel is PPPoL2TPV2\n", dest_dev);
					break;
				}
			}
#endif

#ifdef ECM_INTERFACE_PPTP_ENABLE
			if (protocol == IPPROTO_GRE && dest_dev && dest_dev->type == ARPHRD_PPP) {
				DEBUG_TRACE("Net device: %px PPP channel is PPTP\n", dest_dev);
				break;
			}
#endif
			/*
			 * PPP - but what is the channel type?
			 * First: If this is multi-link then we do not support it
			 */
			if (ppp_is_multilink(dest_dev) > 0) {
				DEBUG_TRACE("Net device: %px is MULTILINK PPP - Unknown to the ECM\n", dest_dev);
				break;
			}

			/*
			 * Get the PPP channel and then enquire what kind of channel it is
			 * NOTE: Not multilink so only one channel to get.
			 */
			channel_count = ppp_hold_channels(dest_dev, ppp_chan, 1);
			if (channel_count != 1) {
				DEBUG_TRACE("Net device: %px PPP has %d channels - Unknown to the ECM\n",
						dest_dev, channel_count);
				break;
			}

			/*
			 * Get channel protocol type
			 * NOTE: Not all PPP channels support channel specific methods.
			 */
			channel_protocol = ppp_channel_get_protocol(ppp_chan[0]);

#ifdef ECM_INTERFACE_L2TPV2_ENABLE
			if (channel_protocol == PX_PROTO_OL2TP) {

				/*
				 * PPPoL2TPV2 channel
				 */
				ppp_release_channels(ppp_chan, 1);
				DEBUG_TRACE("Net device: %px PPP channel is PPPoL2TPV2\n", dest_dev);

				/*
				 * Release the channel.  Note that next_dev not held.
				 */
				break;
			}
#endif
#ifdef ECM_INTERFACE_PPPOE_ENABLE
			if (channel_protocol == PX_PROTO_OE) {
				/*
				 * PPPoE channel
				 */
				DEBUG_TRACE("Net device: %px PPP channel is PPPoE\n", dest_dev);

				/*
				 * Get PPPoE session information and the underlying device it is using.
				 */
				if (pppoe_channel_addressing_get(ppp_chan[0], &addressing)) {
					DEBUG_WARN("%px: failed to get PPPoE addressing info\n", dest_dev);
					ppp_release_channels(ppp_chan, 1);
					break;
				}

				/*
				 * Copy the dev hold into this, we will release the hold later
				 */
				next_dev = addressing.dev;
				next_dest_addr_valid = false;
				next_dest_node_addr_valid = true;
				memcpy(next_dest_node_addr, addressing.pa.remote, ETH_ALEN);

				/*
				 * Release the channel.  Note that next_dev is still (correctly) held.
				 */
				ppp_release_channels(ppp_chan, 1);
				break;
			}
#endif

			DEBUG_TRACE("Net device: %px PPP channel protocol: %d - Unknown to the ECM\n",
				    dest_dev, channel_protocol);

			/*
			 * Release the channel
			 */
			ppp_release_channels(ppp_chan, 1);

#endif
		} while (false);

		/*
		 * No longer need dest_dev as it may become next_dev
		 */
		dev_put(dest_dev);

		/*
		 * Check out the next_dev, if any
		 */
		if (!next_dev) {
			int32_t i __attribute__((unused));
			DEBUG_INFO("Completed interface heirarchy construct with first interface @: %d\n", current_interface_index);
#if DEBUG_LEVEL > 1
			for (i = current_interface_index; i < ECM_DB_IFACE_HEIRARCHY_MAX; ++i) {
				DEBUG_TRACE("\tInterface @ %d: %px, type: %d, name: %s\n",
						i, interfaces[i], ecm_db_iface_type_get(interfaces[i]), ecm_db_interface_type_to_string(ecm_db_iface_type_get(interfaces[i])));

			}
#endif

			/*
			 * Release src_dev now
			 */
			dev_put(src_dev);
			return current_interface_index;
		}

		/*
		 * dest_dev becomes next_dev
		 */
		dest_dev = next_dev;
		dest_dev_name = dest_dev->name;
		dest_dev_type = dest_dev->type;
	}

	DEBUG_WARN("Too many interfaces: %d\n", current_interface_index);
	DEBUG_ASSERT(current_interface_index == 0, "Bad logic handling current_interface_index: %d\n", current_interface_index);
	dev_put(src_dev);
	dev_put(dest_dev);

	/*
	 * Release the interfaces heirarchy we constructed to this point.
	 */
	ecm_db_connection_interfaces_deref(interfaces, current_interface_index);
	return ECM_DB_IFACE_HEIRARCHY_MAX;
}
EXPORT_SYMBOL(ecm_interface_multicast_from_heirarchy_construct);
#endif

#ifdef ECM_INTERFACE_OVPN_ENABLE
static void ecm_interface_ovpn_stats_update(struct net_device *dev, ip_addr_t from_addr, ip_addr_t to_addr)
{
	struct in6_addr from_addr6, to_addr6;

	if (ECM_IP_ADDR_IS_V4(from_addr)) {
		__be32 ip_from_addr, ip_to_addr;

		DEBUG_TRACE("IPv4 Address: " ECM_IP_ADDR_DOT_FMT " : " ECM_IP_ADDR_DOT_FMT "\n", ECM_IP_ADDR_TO_DOT(from_addr), ECM_IP_ADDR_TO_DOT(to_addr));
		ECM_IP_ADDR_TO_NIN4_ADDR(ip_from_addr, from_addr);
		ECM_IP_ADDR_TO_NIN4_ADDR(ip_to_addr, to_addr);
		ecm_interface_ovpn_update_route(dev, &ip_from_addr, &ip_to_addr, 4);
		return;
	}

	DEBUG_TRACE("IPv6 Address: " ECM_IP_ADDR_OCTAL_FMT " : " ECM_IP_ADDR_OCTAL_FMT "\n", ECM_IP_ADDR_TO_OCTAL(from_addr), ECM_IP_ADDR_TO_OCTAL(to_addr));
	ECM_IP_ADDR_TO_NIN6_ADDR(from_addr6, from_addr);
	ECM_IP_ADDR_TO_NIN6_ADDR(to_addr6, to_addr);
	ecm_interface_ovpn_update_route(dev, (uint32_t *)&from_addr6, (uint32_t *)&to_addr6, 6);
}
#endif

/*
 * ecm_interface_list_stats_update()
 *	Given an interface list, walk the interfaces and update the stats for certain types.
 */
static void ecm_interface_list_stats_update(int iface_list_first, struct ecm_db_iface_instance *iface_list[],
					uint8_t *mac_addr, bool is_mcast_to_if, uint32_t tx_packets, uint32_t tx_bytes, uint32_t rx_packets,
					uint32_t rx_bytes, bool is_ported, struct ecm_db_connection_instance *ci, ecm_db_obj_dir_t dir)
{
	int list_index;
#ifdef ECM_INTERFACE_MACVLAN_ENABLE
	bool update_mcast_rx_stats = false;
#endif

	uint32_t stats_bitmap = ci->feci->get_stats_bitmap(ci->feci, dir);

	for (list_index = iface_list_first; (list_index < ECM_DB_IFACE_HEIRARCHY_MAX); list_index++) {
		struct ecm_db_iface_instance *ii;
		ecm_db_iface_type_t ii_type;
		char *ii_name;
		struct net_device *dev;
		struct rtnl_link_stats64 stats;

		ii = iface_list[list_index];
		ii_type = ecm_db_iface_type_get(ii);
		ii_name = ecm_db_interface_type_to_string(ii_type);
		DEBUG_TRACE("list_index: %d, ii: %px, type: %d (%s)\n", list_index, ii, ii_type, ii_name);

		/*
		 * Locate real device in system
		 */
		dev = dev_get_by_index(&init_net, ecm_db_iface_interface_identifier_get(ii));
		if (!dev) {
			DEBUG_WARN("Could not locate interface\n");
			continue;
		}
		DEBUG_TRACE("found dev: %px (%s)\n", dev, dev->name);

		if (likely(!is_mcast_to_if)) {

			/*
			 * Skip bridge forwarding table update if SFE L2 feature is disabled and the flow is route+bridge
			 */
			if (ci->feci->accel_engine == ECM_FRONT_END_ENGINE_SFE) {
				if (ci->is_routed && !(stats_bitmap & BIT(ECM_DB_IFACE_TYPE_BRIDGE))) {
					goto skip_bridge_refresh;
				}
			}

			/*
			 * Refresh the bridge forward table entry if the port is a bridge port.
			 * Refresh if the ci is a 3-tuple PPPoE bridge flow.
			 * Note: A bridge port can be of different interface type, e.g VLAN, ethernet.
			 * This check, therefore, should be performed for all interface types.
			 */
			if (is_valid_ether_addr(mac_addr) && ecm_front_end_is_bridge_port(dev) && rx_packets) {

				if (is_ported || ecm_db_connection_is_pppoe_bridged_get(ci)) {
					DEBUG_TRACE("Update bridge fdb entry for mac: %pM\n", mac_addr);
					/*
					 * Update the existing fdb entry's timestamp only.
					 */
					br_fdb_entry_refresh(dev, mac_addr, 0);
				}

				DEBUG_TRACE("Update bridge fdb entry for mac: %pM\n", mac_addr);

#ifdef ECM_INTERFACE_VXLAN_ENABLE
				/*
				 * Update the VxLAN bridge fdb entries.
				 * The VxLAN fdb entries need to be updated only when the acceleration engine is PPE.
				 * When the acceleration engine is NSS, the refresh is done by the Vxlanmgr with the help of NSS firmware.
				 * When the acceleration engine is SFE, the refresh is done by the host itself.
				 */
				if (is_ported && (stats_bitmap & BIT(ECM_DB_IFACE_TYPE_VXLAN))) {
					struct ecm_db_interface_info_vxlan vxlan_info;
					struct vxlan_dev *priv;

					ecm_db_iface_vxlan_info_get(ii, &vxlan_info);
					priv = netdev_priv(dev);
					DEBUG_TRACE("Update VXLAN bridge fdb entry for mac: %pM\n", mac_addr);
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(4, 5, 7))
					vxlan_fdb_update_mac(priv, mac_addr);
#else
					vxlan_fdb_update_mac(priv, mac_addr, vxlan_info.vni);
#endif
				}
#endif
			}
		}

skip_bridge_refresh:

		memset(&stats, 0, sizeof(stats));

		/*
		 * Update stats for interfaces supported by both NSS and SFE
		 */
		switch (ii_type) {
#ifdef ECM_INTERFACE_MACVLAN_ENABLE
		case ECM_DB_IFACE_TYPE_MACVLAN:
			DEBUG_INFO("MACVLAN\n");

			if (ci->feci->accel_engine == ECM_FRONT_END_ENGINE_SFE) {
				if (!(stats_bitmap & BIT(ECM_DB_IFACE_TYPE_MACVLAN))) {
					dev_put(dev);
					continue;
				}
			}

			stats.rx_packets = rx_packets;
			stats.rx_bytes = rx_bytes;
			stats.tx_packets = tx_packets;
			stats.tx_bytes = tx_bytes;
#ifdef ECM_MULTICAST_ENABLE
			/*
			 * Update multicast rx statistics only for
			 * 'from' interface.
			 */
			update_mcast_rx_stats = (!is_mcast_to_if &&
						ecm_db_multicast_connection_to_interfaces_set_check(ci));
#endif
			macvlan_offload_stats_update(dev, &stats, update_mcast_rx_stats);

			/*
			 * Continue updating stats for other interface.
			 */
			dev_put(dev);
			continue;
#endif

		case ECM_DB_IFACE_TYPE_BRIDGE:
			DEBUG_INFO("BRIDGE\n");
			if (ci->feci->accel_engine == ECM_FRONT_END_ENGINE_SFE) {
				if (!(stats_bitmap & BIT(ECM_DB_IFACE_TYPE_BRIDGE))) {
					dev_put(dev);
					continue;
				}
			}

			stats.rx_packets = rx_packets;
			stats.rx_bytes = rx_bytes;
			stats.tx_packets = tx_packets;
			stats.tx_bytes = tx_bytes;
#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 11, 0))
			ecm_interface_br_dev_update_stats(dev, &stats);
#else
			br_dev_update_stats(dev, &stats);
#endif
			dev_put(dev);
			continue;

#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
		case ECM_DB_IFACE_TYPE_OVS_BRIDGE:
			DEBUG_INFO("OVS BRIDGE\n");
			if (ci->feci->accel_engine == ECM_FRONT_END_ENGINE_SFE) {
				if (!(stats_bitmap & BIT(ECM_DB_IFACE_TYPE_OVS_BRIDGE))) {
					dev_put(dev);
					continue;
				}
			}
			ovsmgr_bridge_interface_stats_update(dev, rx_packets, rx_bytes, tx_packets, tx_bytes);
			dev_put(dev);
			continue;
#endif

#ifdef ECM_INTERFACE_VLAN_ENABLE
		case ECM_DB_IFACE_TYPE_VLAN:
			DEBUG_INFO("VLAN\n");
			/*
			 * Update vlan device with stats from SFE AE only when
			 * SFE's l2_feature_support is enabled.
			 */
			if (ci->feci->accel_engine == ECM_FRONT_END_ENGINE_SFE) {
				if (!(stats_bitmap & BIT(ECM_DB_IFACE_TYPE_VLAN))) {
					dev_put(dev);
					continue;
				}
			}
			stats.rx_packets = rx_packets;
			stats.rx_bytes = rx_bytes;
			stats.tx_packets = tx_packets;
			stats.tx_bytes = tx_bytes;
#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 11, 0))
			ecm_interface_vlan_dev_update_accel_stats(dev, &stats);
#else
			__vlan_dev_update_accel_stats(dev, &stats);
#endif
			dev_put(dev);
			continue;
#endif

#ifdef ECM_INTERFACE_PPPOE_ENABLE
		case ECM_DB_IFACE_TYPE_PPPOE:
			DEBUG_INFO("PPPOE\n");
			if (ci->feci->accel_engine == ECM_FRONT_END_ENGINE_SFE) {
				if (!(stats_bitmap & BIT(ECM_DB_IFACE_TYPE_PPPOE))) {
					dev_put(dev);
					continue;
				}
			}
			ppp_update_stats(dev, rx_packets, rx_bytes, tx_packets, tx_bytes, 0, 0, 0, 0);
			dev_put(dev);
			continue;
#endif

		default:
			break;
		}

		/*
		 * Update stats for the interfaces supported by only the NSS acceleration engine.
		 */
		if (ci->feci->accel_engine != ECM_FRONT_END_ENGINE_NSS ) {
			DEBUG_TRACE("Invalid stats update for iface type=%d\n", ii_type);
			dev_put(dev);
			continue;
		}

		switch (ii_type) {
#ifdef ECM_INTERFACE_OVPN_ENABLE
		case ECM_DB_IFACE_TYPE_OVPN: {
			ip_addr_t from_addr, to_addr;

			DEBUG_INFO("OVPN\n");
			ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_FROM, from_addr);
			ecm_db_connection_address_get(ci, ECM_DB_OBJ_DIR_TO, to_addr);
			ecm_interface_ovpn_stats_update(dev, from_addr, to_addr);
		}
		break;
#endif
		default:
			/*
			 * TODO: Extend it accordingly
			 */
			break;
		}
		dev_put(dev);
	}
}

/*
 * ecm_interface_stats_update()
 *	Using the interface lists for the given connection, update the interface statistics for each.
 *
 * 'from' here is wrt the connection 'from' side.  Likewise with 'to'.
 * TX is wrt what the interface has transmitted.  RX is what the interface has received.
 */
void ecm_interface_stats_update(struct ecm_db_connection_instance *ci,
						uint32_t from_tx_packets, uint32_t from_tx_bytes, uint32_t from_rx_packets, uint32_t from_rx_bytes,
						uint32_t to_tx_packets, uint32_t to_tx_bytes, uint32_t to_rx_packets, uint32_t to_rx_bytes)
{
	struct ecm_db_iface_instance *from_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX];
	struct ecm_db_iface_instance *to_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX];
	int from_ifaces_first;
	int to_ifaces_first;
	uint8_t mac_addr[ETH_ALEN];
	bool is_ported = false;
	uint8_t protocol = ecm_db_connection_protocol_get(ci);

	/*
	 * Set is_ported flag based on protocol.
	 */
	if ((protocol == IPPROTO_UDP) || (protocol == IPPROTO_TCP)) {
		is_ported = true;
	}

	/*
	 * Iterate the 'from' side interfaces and update statistics and state for the real HLOS interfaces
	 * from_tx_packets / bytes: the amount transmitted by the 'from' interface
	 * from_rx_packets / bytes: the amount received by the 'from' interface
	 */
	DEBUG_INFO("%px: Update from interface stats\n", ci);
	from_ifaces_first = ecm_db_connection_interfaces_get_and_ref(ci, from_ifaces, ECM_DB_OBJ_DIR_FROM);
	ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_FROM, mac_addr);
	ecm_interface_list_stats_update(from_ifaces_first, from_ifaces, mac_addr, false, from_tx_packets, from_tx_bytes, from_rx_packets, from_rx_bytes, is_ported, ci, ECM_DB_OBJ_DIR_FROM);
	ecm_db_connection_interfaces_deref(from_ifaces, from_ifaces_first);

	/*
	 * Iterate the 'to' side interfaces and update statistics and state for the real HLOS interfaces
	 * to_tx_packets / bytes: the amount transmitted by the 'to' interface
	 * to_rx_packets / bytes: the amount received by the 'to' interface
	 */
	DEBUG_INFO("%px: Update to interface stats\n", ci);
	to_ifaces_first = ecm_db_connection_interfaces_get_and_ref(ci, to_ifaces, ECM_DB_OBJ_DIR_TO);
	ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_TO, mac_addr);
	ecm_interface_list_stats_update(to_ifaces_first, to_ifaces, mac_addr, false, to_tx_packets, to_tx_bytes, to_rx_packets, to_rx_bytes, is_ported, ci, ECM_DB_OBJ_DIR_TO);
	ecm_db_connection_interfaces_deref(to_ifaces, to_ifaces_first);
}
EXPORT_SYMBOL(ecm_interface_stats_update);

#ifdef ECM_MULTICAST_ENABLE
/*
 * ecm_interface_multicast_stats_update()
 *	Using the interface lists for the given connection, update the interface statistics for each.
 *
 * 'from interface' here is the connection 'from' side.  Likewise with 'to interface'.
 * TX is wrt what the interface has transmitted.  RX is what the interface has received.
 */
void ecm_interface_multicast_stats_update(struct ecm_db_connection_instance *ci, uint32_t from_tx_packets, uint32_t from_tx_bytes,
				   uint32_t from_rx_packets, uint32_t from_rx_bytes, uint32_t to_tx_packets, uint32_t to_tx_bytes,
				   uint32_t to_rx_packets, uint32_t to_rx_bytes)
{
	struct ecm_db_iface_instance *from_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX];
	struct ecm_db_iface_instance *to_list_single[ECM_DB_IFACE_HEIRARCHY_MAX];
	struct ecm_db_iface_instance *to_ifaces;
	struct ecm_db_iface_instance *ii_temp;
	int from_ifaces_first;
	int *to_ifaces_first;
	int if_index;
	int ret;
	uint8_t mac_addr[ETH_ALEN];
	bool is_ported = false;
	uint8_t protocol = ecm_db_connection_protocol_get(ci);

	/*
	 * Set is_ported flag based on protocol
	 */
	if (protocol == IPPROTO_UDP) {
		is_ported = true;
	}

	/*
	 * Iterate the 'from' side interfaces and update statistics and state for the real HLOS interfaces
	 * from_tx_packets / bytes: the amount transmitted by the 'from' interface
	 * from_rx_packets / bytes: the amount received by the 'from' interface
	 */
	DEBUG_INFO("%px: Update from interface stats\n", ci);
	from_ifaces_first = ecm_db_connection_interfaces_get_and_ref(ci, from_ifaces, ECM_DB_OBJ_DIR_FROM);
	ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_FROM, mac_addr);
	ecm_interface_list_stats_update(from_ifaces_first, from_ifaces, mac_addr, false, 0, 0, from_rx_packets, from_rx_bytes, is_ported, ci, ECM_DB_OBJ_DIR_FROM);
	ecm_db_connection_interfaces_deref(from_ifaces, from_ifaces_first);

	/*
	 * Iterate the 'to' side interfaces and update statistics and state for the real HLOS interfaces
	 * to_tx_packets / bytes: the amount transmitted by the 'to' interface
	 * to_rx_packets / bytes: the amount received by the 'to' interface
	 */
	DEBUG_INFO("%px: Update to interface stats\n", ci);

	/*
	 * This function allocates the memory for temporary destination interface heirarchies.
	 * This memory needs to be free at the end.
	 */
	ret = ecm_db_multicast_connection_to_interfaces_get_and_ref_all(ci, &to_ifaces, &to_ifaces_first);
	if (ret == 0) {
		DEBUG_WARN("%px: Get and ref to all multicast detination interface heirarchies failed\n", ci);
		return;
	}

	for (if_index = 0; if_index < ECM_DB_MULTICAST_IF_MAX; if_index++) {
		if (to_ifaces_first[if_index] < ECM_DB_IFACE_HEIRARCHY_MAX) {
			ii_temp = ecm_db_multicast_if_heirarchy_get(to_ifaces, if_index);
			ecm_db_multicast_copy_if_heirarchy(to_list_single, ii_temp);
			ecm_interface_list_stats_update(to_ifaces_first[if_index], to_list_single, mac_addr, true, from_tx_packets, from_tx_bytes, 0, 0, is_ported, ci, ECM_DB_OBJ_DIR_TO);
		}
	}

	ecm_db_multicast_connection_to_interfaces_deref_all(to_ifaces, to_ifaces_first);
}
EXPORT_SYMBOL(ecm_interface_multicast_stats_update);
#endif

/*
 * ecm_interface_regenerate_connections()
 *	Cause regeneration of all connections that are using the specified interface.
 */
static void ecm_interface_regenerate_connections(struct ecm_db_iface_instance *ii)
{
#ifdef ECM_DB_XREF_ENABLE
	int dir;
	struct ecm_db_connection_instance *ci[ECM_DB_OBJ_DIR_MAX];
	struct ecm_db_connection_instance *ci_mcast __attribute__ ((unused));
#endif
	char name[IFNAMSIZ];

	ecm_db_iface_interface_name_get(ii, name);
	DEBUG_TRACE("Regenerate connections using interface: %px (%s)\n", ii, name);

#ifndef ECM_DB_XREF_ENABLE
	/*
	 * An interface has changed, re-generate the connections to ensure all state is updated.
	 */
	ecm_db_regeneration_needed();
#else
	for (dir = 0; dir < ECM_DB_OBJ_DIR_MAX; dir++) {
		ci[dir] = ecm_db_iface_connections_get_and_ref_first(ii, dir);
	}

	/*
	 * If the interface has NO connections then we re-generate all.
	 */
	if (!ci[ECM_DB_OBJ_DIR_FROM] && !ci[ECM_DB_OBJ_DIR_TO] && !ci[ECM_DB_OBJ_DIR_FROM_NAT] && !ci[ECM_DB_OBJ_DIR_TO_NAT]) {
		ecm_db_regeneration_needed();
		DEBUG_TRACE("%px: Regenerate (ALL) COMPLETE\n", ii);
		return;
	}

#ifdef ECM_DB_XREF_ENABLE
	for (dir = 0; dir < ECM_DB_OBJ_DIR_MAX; dir++) {
		/*
		 * Re-generate all connections associated with this interface
		 */
		DEBUG_TRACE("%px: Regenerate %s direction connections\n", ii, ecm_db_obj_dir_strings[dir]);
		while (ci[dir]) {
			struct ecm_db_connection_instance *cin;
			cin = ecm_db_connection_iface_get_and_ref_next(ci[dir], dir);

			DEBUG_TRACE("%px: Regenerate: %px", ii, ci[dir]);
			ecm_db_connection_regenerate(ci[dir]);
			ecm_db_connection_deref(ci[dir]);
			ci[dir] = cin;
		}
	}
#endif

#ifdef ECM_MULTICAST_ENABLE
	/*
	 * Multicasts would not have recorded in the lists above.
	 * Our only way to re-gen those is to iterate all multicasts.
	 * GGG TODO This will be optimised in a future release.
	 */
	ci_mcast = ecm_db_connections_get_and_ref_first();
	while (ci_mcast) {
		struct ecm_db_connection_instance *cin;

		/*
		 * Multicast and NOT flagged for re-gen?
		 */
		if (ecm_db_multicast_connection_to_interfaces_set_check(ci_mcast)
				&& ecm_db_connection_regeneration_required_peek(ci_mcast)) {
			ecm_db_connection_regenerate(ci_mcast);
		}

		cin = ecm_db_connection_get_and_ref_next(ci_mcast);
		ecm_db_connection_deref(ci_mcast);
		ci_mcast = cin;
	}
#endif

#endif
	DEBUG_TRACE("%px: Regenerate COMPLETE\n", ii);
}

/*
 * ecm_interface_dev_regenerate_connections()
 *	Cause regeneration of all connections that are using the specified interface.
 */
void ecm_interface_dev_regenerate_connections(struct net_device *dev)
{
	struct ecm_db_iface_instance *ii;

	DEBUG_INFO("Regenerate connections for: %px (%s)\n", dev, dev->name);

	/*
	 * If the interface is known to us then we will get it returned by this
	 * function and process it accordingly.
	 */
	ii = ecm_db_iface_find_and_ref_by_interface_identifier(dev->ifindex);
	if (!ii) {
		DEBUG_WARN("%px: No interface instance could be established for this dev\n", dev);
		return;
	}
	ecm_interface_regenerate_connections(ii);
	DEBUG_TRACE("%px: Regenerate for %px: COMPLETE\n", dev, ii);
	ecm_db_iface_deref(ii);
}

/*
 * ecm_interface_defunct_connections()
 *	Cause defunct of all connections that are using the specified interface.
 */
static void ecm_interface_defunct_connections(struct ecm_db_iface_instance *ii)
{
#ifndef ECM_DB_XREF_ENABLE
	ecm_db_connection_defunct_all();
#else
	int dir;
	struct ecm_db_connection_instance *ci[ECM_DB_OBJ_DIR_MAX];
	struct ecm_db_connection_instance *ci_mcast __attribute__ ((unused));
	char name[IFNAMSIZ];

	ecm_db_iface_interface_name_get(ii, name);
	DEBUG_TRACE("defunct connections using interface: %px (%s)\n", ii, name);

	for (dir = 0; dir < ECM_DB_OBJ_DIR_MAX; dir++) {
		ci[dir] = ecm_db_iface_connections_get_and_ref_first(ii, dir);
	}

	/*
	 * Defunct ALL if all the four connection instances are NULL
	 */
	if (!ci[ECM_DB_OBJ_DIR_FROM] && !ci[ECM_DB_OBJ_DIR_TO] &&
			!ci[ECM_DB_OBJ_DIR_FROM_NAT] && !ci[ECM_DB_OBJ_DIR_TO_NAT]) {
		ecm_db_connection_defunct_all();
		DEBUG_TRACE("%px: Defunct (ALL) COMPLETE\n", ii);
		return;
	}

	/*
	 * Defunct all connections associated with this interface
	 */
	for (dir = 0; dir < ECM_DB_OBJ_DIR_MAX; dir++) {
		DEBUG_TRACE("%px: Defunct %s direction connections\n", ii, ecm_db_obj_dir_strings[dir]);
		while (ci[dir]) {
			struct ecm_db_connection_instance *cin;
			cin = ecm_db_connection_iface_get_and_ref_next(ci[dir], dir);

			DEBUG_TRACE("%px: Defunct: %px", ii, ci[dir]);
			ecm_db_connection_make_defunct(ci[dir]);
			ecm_db_connection_deref(ci[dir]);
			ci[dir] = cin;
		}
	}
#endif
	DEBUG_TRACE("%px: Defunct COMPLETE\n", ii);
}

/*
 * ecm_interface_dev_defunct_connections()
 *	Cause defunct of all connections that are using the specified interface.
 */
void ecm_interface_dev_defunct_connections(struct net_device *dev)
{
	struct ecm_db_iface_instance *ii;

	DEBUG_INFO("defunct connections for: %px (%s)\n", dev, dev->name);

	/*
	 * Filter interface instances matching dev and defunct connections
	 */
	ii = ecm_db_interfaces_get_and_ref_first();
	while (ii) {
		struct ecm_db_iface_instance *iin;

		/*
		 * Defunct connections if ii is representing dev.
		 */
		if (dev->ifindex == ecm_db_iface_interface_identifier_get(ii)) {
			ecm_interface_defunct_connections(ii);
			DEBUG_TRACE("%px: defunct for %px: COMPLETE\n", dev, ii);
			ecm_db_iface_deref(ii);
			return;
		}

		/*
		 * Find next interface in the list
		 */
		iin = ecm_db_interface_get_and_ref_next(ii);
		ecm_db_iface_deref(ii);
		ii = iin;
	}
}

/*
 * ecm_interface_mtu_change()
 *	MTU of interface has changed
 */
static void ecm_interface_mtu_change(struct net_device *dev)
{
	int mtu;
	struct ecm_db_iface_instance *ii;

	mtu = dev->mtu;
	DEBUG_INFO("%px (%s): MTU Change to: %d\n", dev, dev->name, mtu);

	/*
	 * Filter interface instances matching dev and regenerate connections
	 */
	ii = ecm_db_interfaces_get_and_ref_first();
	while (ii) {
		struct ecm_db_iface_instance *iin;

		/*
		 * Defunct connections if ii is representing dev, otherwise
		 * skip to the next ii.
		 */
		if (dev->ifindex != ecm_db_iface_interface_identifier_get(ii)) {
			goto next;
		}

		/*
		 * Change the mtu
		 */
		ecm_db_iface_mtu_reset(ii, mtu);

		if (!netif_is_bond_slave(dev)) {
			ecm_interface_regenerate_connections(ii);
		} else {
			struct net_device *master = NULL;
			master = ecm_interface_get_and_hold_dev_master(dev);
			DEBUG_ASSERT(master, "Expected a master\n");
			ecm_interface_dev_regenerate_connections(master);
			dev_put(master);
		}

		DEBUG_TRACE("%px: Regenerate for (%s) belong to iface %px COMPLETE\n", dev, dev->name, ii);
next:
		iin = ecm_db_interface_get_and_ref_next(ii);
		ecm_db_iface_deref(ii);
		ii = iin;
	}
}

/*
 * ecm_interface_netdev_notifier_callback()
 *	Netdevice notifier callback to inform us of change of state of a netdevice
 */
static int ecm_interface_netdev_notifier_callback(struct notifier_block *this, unsigned long event, void *ptr)
{
	struct net_device *dev = netdev_notifier_info_to_dev(ptr);
	struct net_device *master = NULL;

	DEBUG_INFO("Net device notifier for: %px, name: %s, event: %lx\n", dev, dev->name, event);

	switch (event) {
	case NETDEV_DOWN:
		DEBUG_INFO("Net device: %px, DOWN\n", dev);
		if (netif_is_bond_slave(dev)) {
			master = ecm_interface_get_and_hold_dev_master(dev);
			DEBUG_ASSERT(master, "Expected a master\n");
			ecm_interface_dev_defunct_connections(master);
			dev_put(master);
		} else {
			ecm_interface_dev_defunct_connections(dev);
		}
		break;

	case NETDEV_CHANGE:
		DEBUG_INFO("Net device: %px, CHANGE\n", dev);
		if (!netif_carrier_ok(dev)) {
			DEBUG_INFO("Net device: %px, CARRIER BAD\n", dev);
			if (netif_is_bond_slave(dev)) {
				master = ecm_interface_get_and_hold_dev_master(dev);
				DEBUG_ASSERT(master, "Expected a master.\n");
				ecm_interface_dev_defunct_connections(master);
				dev_put(master);
			} else {
				ecm_interface_dev_defunct_connections(dev);
			}
		}
		break;

	case NETDEV_CHANGEMTU:
		DEBUG_INFO("Net device: %px, MTU CHANGE\n", dev);
		ecm_interface_mtu_change(dev);
		break;

	default:
		DEBUG_TRACE("Net device: %px, UNHANDLED: %lx\n", dev, event);
		break;
	}

	return NOTIFY_DONE;
}

/*
 * ecm_interface_node_connections_defunct_by_type()
 *	Defunct the connections on this node based on the specific event.
 */
void ecm_interface_node_connections_defunct_by_type(uint8_t *mac, int ip_version, ecm_db_connection_defunct_type_t type)
{
	struct ecm_db_node_instance *ni = NULL;

	if (unlikely(!mac)) {
		DEBUG_WARN("mac address is null\n");
		return;
	}

	/*
	 * Disable frontend processing until defunct function call is completed.
	 */
	ecm_front_end_ipv4_stop(1);
#ifdef ECM_IPV6_ENABLE
	ecm_front_end_ipv6_stop(1);
#endif
	ni = ecm_db_node_chain_get_and_ref_first(mac);
	while (ni) {
		struct ecm_db_node_instance *nin;

		if (ecm_db_node_is_mac_addr_equal(ni, mac)) {
			int dir;
			/*
			 * FROM and TO directions are enough to destroy all the connections.
			 * FROM_NAT and TO_NAT have the same list of connections.
			 */
			for (dir = 0; dir <= ECM_DB_OBJ_DIR_TO; dir++) {
				/*
				 * If there is connection on this node, call the defunct function.
				 */
				if (ecm_db_node_get_connections_count(ni, dir)) {
					ecm_db_traverse_node_connection_list_and_defunct(ni, dir, ip_version, type);
				}
			}
			/*
			 * node was found and connections are destroyed, we are done.
			 */
			ecm_db_node_deref(ni);
			break;
		}

		/*
		 * Get next node in the chain
		 */
		nin = ecm_db_node_chain_get_and_ref_next(ni);
		ecm_db_node_deref(ni);
		ni = nin;
	}

	/*
	 * Re-enable frontend processing.
	 */
	ecm_front_end_ipv4_stop(0);
#ifdef ECM_IPV6_ENABLE
	ecm_front_end_ipv6_stop(0);
#endif
}

/*
 * ecm_interface_node_connections_defunct()
 *	Defunct the connections on this node.
 */
void ecm_interface_node_connections_defunct(uint8_t *mac, int ip_version)
{
	ecm_interface_node_connections_defunct_by_type(mac, ip_version, ECM_DB_CONNECTION_DEFUNCT_TYPE_IGNORE);
}

/*
 * struct notifier_block ecm_interface_netdev_notifier
 *	Registration for net device changes of state.
 */
static struct notifier_block ecm_interface_netdev_notifier __read_mostly = {
	.notifier_call		= ecm_interface_netdev_notifier_callback,
};

#if defined(ECM_DB_XREF_ENABLE) && defined(ECM_BAND_STEERING_ENABLE)
/*
 * ecm_interfae_node_br_fdb_notify_event()
 *	This is a callback for "bridge fdb update event. It is called
 *	When a MAC address is moved to another interface.
 *
 */
static int ecm_interface_node_br_fdb_notify_event(struct notifier_block *nb,
					       unsigned long event,
					       void *data)
{
	struct br_fdb_event *fe = (struct br_fdb_event *)data;

	/*
	 * Check if original and current devs are not NULL.
	 */
	if (!fe->orig_dev || !fe->dev) {
		return NOTIFY_DONE;
	}

	/*
	 * If the old and new devs are the same, we don't need to handle this event.
	 */
	if (fe->orig_dev == fe->dev) {
		return NOTIFY_DONE;
	}

	DEBUG_TRACE("%px: FDB notify event for moved MAC addr: %pM\n", fe, fe->addr);
	ecm_interface_node_connections_defunct(fe->addr, ECM_DB_IP_VERSION_IGNORE);

	return NOTIFY_DONE;
}

static struct notifier_block ecm_interface_node_br_fdb_update_nb = {
	.notifier_call = ecm_interface_node_br_fdb_notify_event,
};

/*
 * ecm_interface_node_br_fdb_delete_event()
 *	Callback for FDB delete/ageing timeout events.
 */
static int ecm_interface_node_br_fdb_delete_event(struct notifier_block *nb,
					       unsigned long event,
					       void *data)
{
	struct br_fdb_event *fe = (struct br_fdb_event *)data;

	if ((event != BR_FDB_EVENT_DEL) || fe->is_local) {
		DEBUG_WARN("%px: local fdb or not deleting event, ignore\n", fe);
		return NOTIFY_DONE;
	}

	DEBUG_TRACE("%px: FDB delete event for MAC addr: %pM\n", fe, fe->addr);
	ecm_interface_node_connections_defunct(fe->addr, ECM_DB_IP_VERSION_IGNORE);

	return NOTIFY_DONE;
}

static struct notifier_block ecm_interface_node_br_fdb_delete_nb = {
	.notifier_call = ecm_interface_node_br_fdb_delete_event,
};
#endif

#ifdef ECM_MULTICAST_ENABLE
/*
 * ecm_interface_multicast_find_outdated_iface_instances()
 *
 *	Called in the case of Routing/Bridging Multicast update events.
 *
 *	This function takes a list of ifindex for the connection which was received
 *	from MFC or bridge snooper, compares it against the existing list of interfaces
 *	in the DB connection, and extracts the list of those interfaces that have left
 *	the multicast group.
 *
 *	ci		A DB connection instance.
 *	mc_updates	Part of return Information. The function will mark the index of those
 *			interfaces in the DB connection 'to_mcast_interfaces' array that have
 *			left the group, in the mc_updates->if_leave_idx array. The caller uses this
 *			information to delete those outdated interface heirarchies from the
 *			connection.
 *	is_bridged	True if the function called due to bridge multicast snooper update event.
 *	dst_dev		Holds the netdevice ifindex number of the new list of interfaces as reported
 *			by the update from MFC or Bridge snooper.
 *	max_to_dev	Size of the array 'dst_dev'
 *
 *	Return true if outdated interfaces found
 */
static bool ecm_interface_multicast_find_outdated_iface_instances(struct ecm_db_connection_instance *ci, struct ecm_multicast_if_update *mc_updates,
						   uint32_t flags, bool is_br_snooper, uint32_t *mc_dst_if_index, uint32_t max_to_dev,
						   struct net_device *brdev)
{
	struct ecm_db_iface_instance *mc_ifaces;
	struct ecm_db_iface_instance *ii_temp;
	struct ecm_db_iface_instance *ii_single;
	struct ecm_db_iface_instance **ifaces;
	struct ecm_db_iface_instance *to_iface;
	int32_t *to_iface_first;
	int32_t *mc_ifaces_first;
	uint32_t *dst_if_index;
	ecm_db_iface_type_t ii_type;
	int32_t heirarchy_index;
	int32_t if_index;
	int32_t if_cnt = 0;
	int found = 0;
	int ii;
	int ret;
	int32_t ifaces_identifier;

	ret = ecm_db_multicast_connection_to_interfaces_get_and_ref_all(ci, &mc_ifaces, &mc_ifaces_first);
	if (ret == 0) {
		DEBUG_WARN("%px: multicast interfaces ref fail!\n", ci);
		return false;
	}

	/*
	 * Loop through the current interface list in the DB
	 * connection 'to_mcast_interfaces' array
	 */
	for (heirarchy_index = 0; heirarchy_index < ECM_DB_MULTICAST_IF_MAX; heirarchy_index++) {
		found = 0;
		to_iface_first = ecm_db_multicast_if_first_get_at_index(mc_ifaces_first, heirarchy_index);

		/*
		 * Invalid interface entry, skip
		 */
		if (*to_iface_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
			continue;
		}

		ii_temp = ecm_db_multicast_if_heirarchy_get(mc_ifaces, heirarchy_index);
		ii_single = ecm_db_multicast_if_instance_get_at_index(ii_temp, ECM_DB_IFACE_HEIRARCHY_MAX - 1);
		ifaces = (struct ecm_db_iface_instance **)ii_single;
		to_iface = *ifaces;
		ii_type = ecm_db_iface_type_get(to_iface);

		if ((ii_type == ECM_DB_IFACE_TYPE_BRIDGE) || (ii_type == ECM_DB_IFACE_TYPE_OVS_BRIDGE)) {
			/*
			 * If the update was received from MFC, do not consider entries in the
			 * interface list that are part of a bridge/ovs_bridge. The bridge/ovs_bridge entries will be
			 * taken care by the Bridge Snooper Callback
			 *
			 * TODO: Check if an assert is needed for the flag
			 * ECM_DB_MULTICAST_CONNECTION_BRIDGE_DEV_SET_FLAG to be set, if is_br_snooper is false.
			 */
			if (!is_br_snooper && (flags & ECM_DB_MULTICAST_CONNECTION_BRIDGE_DEV_SET_FLAG)) {
				continue;
			}

			/*
			 * If update was received from Bridge snooper.
			 * Check for the correct bridge interface in the 'to' list
			 * as ECM support multibridge multicast.
			 */
			if (is_br_snooper) {
				struct net_device *to_dev;

				to_dev = dev_get_by_index(&init_net, ecm_db_iface_interface_identifier_get(to_iface));
				if (to_dev) {
					if (to_dev->ifindex != brdev->ifindex) {
						dev_put(to_dev);
						continue;
					}
					dev_put(to_dev);
				}
			}
		} else if (is_br_snooper) {
			/*
			 * If the update was received from bridge snooper, do not consider entries in the
			 * interface list that are not part of a bridge or ovs bridge.
			 */
			continue;
		}

		/*
		 * Try to find a match in the newly received interface list, for any of
		 * the interface instance in the heirarchy. If found, it means that this
		 * interface has not left the group. If not found, it means that this
		 * interface has left the group.
		 */
		for (ii = ECM_DB_IFACE_HEIRARCHY_MAX - 1; ii >= *to_iface_first; ii--) {
			ii_single = ecm_db_multicast_if_instance_get_at_index(ii_temp, ii);
			ifaces = (struct ecm_db_iface_instance **)ii_single;
			to_iface = *ifaces;

			ii_type = ecm_db_iface_type_get(to_iface);
			ifaces_identifier = ecm_db_iface_interface_identifier_get(to_iface);
			for (if_index = 0; if_index < max_to_dev; if_index++) {
				dst_if_index = ecm_db_multicast_if_num_get_at_index(mc_dst_if_index, if_index);
				if (*dst_if_index == ifaces_identifier) {
					found = 1;
					break;
				}
			}
			if (found) {
				break;
			}
		}

		/*
		 * We did not find a match for the interface in the present list. So mark
		 * if as one that has left the group.
		 */
		if (!found) {
			if_cnt++;
			mc_updates->if_leave_idx[heirarchy_index] = 1;
		}
	}

	ecm_db_multicast_connection_to_interfaces_deref_all(mc_ifaces, mc_ifaces_first);
	mc_updates->if_leave_cnt = if_cnt;
	return (if_cnt > 0);
}

/*
 * ecm_interface_multicast_find_new_iface_instances()
 *
 *	Called in the case of Routing/Bridging Multicast update events.
 *
 *	This function takes a list of ifindex for the connection which was received
 *	from MFC or bridge snooper, compares it against the existing list of interfaces
 *	in the DB connection, and extracts the list of the new joinees for the multicast
 *	group.
 *
 *	ci		A DB connection instance.
 *	mc_updates	Part of return Information. The function will mark the index of those
 *			interfaces in the 'dst_dev' array that have joined the group, in the
 *			mc_updates->if_join_idx array. The caller uses this information to add the new
 *			interface heirarchies into the connection.
 *	dst_dev		Holds the netdevice ifindex number of the new list of interfaces as reported
 *			by the update from MFC or Bridge snooper.
 *	max_to_dev	Size of the array 'dst_dev'
 *
 *	Return true if new joinees found
 */
static bool ecm_interface_multicast_find_new_iface_instances(struct ecm_db_connection_instance *ci,
					struct ecm_multicast_if_update *mc_updates, uint32_t *mc_dst_if_index, uint32_t max_to_dev)
{
	struct ecm_db_iface_instance *mc_ifaces;
	struct ecm_db_iface_instance *ii_temp;
	struct ecm_db_iface_instance *ii_single;
	struct ecm_db_iface_instance **ifaces;
	int32_t *mc_ifaces_first;
	int32_t *to_list_first;
	int32_t heirarchy_index;
	int32_t if_index;
	int32_t if_cnt = 0;
	int found = 0;
	int ii;
	int ret;
	uint32_t *dst_if_index;
	int32_t ifaces_identifier;
	struct ecm_db_iface_instance *to_list;

	ret = ecm_db_multicast_connection_to_interfaces_get_and_ref_all(ci, &mc_ifaces, &mc_ifaces_first);
	if (ret == 0) {
		DEBUG_WARN("%px: multicast interfaces ref fail!\n", ci);
		return false;
	}

	/*
	 * Loop through the new interface list 'dst_dev'
	 */
	for (if_index = 0; if_index < max_to_dev; if_index++) {
		found = 0;
		dst_if_index = ecm_db_multicast_if_num_get_at_index(mc_dst_if_index, if_index);
		if (*dst_if_index == 0) {
			continue;
		}

		for (heirarchy_index = 0; heirarchy_index < ECM_DB_MULTICAST_IF_MAX ; heirarchy_index++) {
			to_list_first = ecm_db_multicast_if_first_get_at_index(mc_ifaces_first, heirarchy_index);

			/*
			 * Invalid interface entry, skip
			 */
			if (*to_list_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
				continue;
			}

			ii_temp = ecm_db_multicast_if_heirarchy_get(mc_ifaces, heirarchy_index);

			/*
			 * Try to find a match for this ifindex (dst_dev[if_index]), in any of the
			 * interface instance in the heirarchy. If not found, it means that this
			 * ifindex has joined the group. If found, it means that this ifindex was
			 * already part of the list of destination interfaces.
			 */
			for (ii = ECM_DB_IFACE_HEIRARCHY_MAX - 1; ii >= *to_list_first; ii--) {
				ii_single = ecm_db_multicast_if_instance_get_at_index(ii_temp, ii);
				ifaces = (struct ecm_db_iface_instance **)ii_single;
				to_list = *ifaces;
				ifaces_identifier = ecm_db_iface_interface_identifier_get(to_list);
				if (*dst_if_index == ifaces_identifier) {
					found =  1;
					break;
				}
			}

			if (found) {
				break;
			}
		}

		/*
		 * We did not find a match for the interface in the present list. So mark
		 * it as one that has joined the group.
		 */
		if (!found) {

			/*
			 * Store the if index of the new joinee
			 */
			mc_updates->join_dev[if_cnt] = *dst_if_index;

			/*
			 * Identify a new vacant slot in the 'to_mcast_interfaces' to place
			 * the new interface
			 */
			for (heirarchy_index = 0; heirarchy_index < ECM_DB_MULTICAST_IF_MAX ; heirarchy_index++) {
				to_list_first = ecm_db_multicast_if_first_get_at_index(mc_ifaces_first, heirarchy_index);
				if (*to_list_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
					mc_updates->if_join_idx[heirarchy_index] = 1;
					break;
				}
			}

			if_cnt++;
		}
	}

	ecm_db_multicast_connection_to_interfaces_deref_all(mc_ifaces, mc_ifaces_first);
	mc_updates->if_join_cnt = if_cnt;

	return (if_cnt > 0);
}

/*
 * ecm_interface_multicast_find_updates_to_iface_list()
 *	Process IGMP/MLD updates either from MFC or bridge snooper. Identity the interfaces
 *	that have left the group and new interfaces that have joined the group.
 *
 * The function returns true if there was any update necessary to the current destination
 * interface list
 */
bool ecm_interface_multicast_find_updates_to_iface_list(struct ecm_db_connection_instance *ci, struct ecm_multicast_if_update *mc_updates,
						uint32_t flags, bool is_br_snooper, uint32_t *mc_dst_if_index, uint32_t max_to_dev,
						struct net_device *brdev)
{
	bool join;
	bool leave;

	/*
	 * Find destination interfaces that have left the group
	 */
	leave = ecm_interface_multicast_find_outdated_iface_instances(ci, mc_updates, flags, is_br_snooper, mc_dst_if_index, max_to_dev, brdev);

	/*
	 * Find new destination interfaces that have joined the group
	 */
	join = ecm_interface_multicast_find_new_iface_instances(ci, mc_updates, mc_dst_if_index, max_to_dev);

	return (leave || join);
}
EXPORT_SYMBOL(ecm_interface_multicast_find_updates_to_iface_list);
#endif

#ifdef ECM_DB_XREF_ENABLE
/*
 * ecm_interface_neigh_mac_update_notify_event()
 *	Neighbour mac address change handler.
 */
static int ecm_interface_neigh_mac_update_notify_event(struct notifier_block *nb,
						       unsigned long val,
						       void *data)
{
	struct neigh_mac_update *nmu = (struct neigh_mac_update *)data;

	/*
	 * If the old and new mac addresses are equal, do nothing.
	 * This case shouldn't happen.
	 */
	if (!ecm_mac_addr_equal(nmu->old_mac, nmu->update_mac)) {
		DEBUG_TRACE("old and new mac addresses are equal: %pM\n", nmu->old_mac);
		return NOTIFY_DONE;
	}

	/*
	 * If the old mac is zero, do nothing. When a host joins the arp table first
	 * time, its old mac comes as zero. We shouldn't handle this case, because
	 * there is not any connection in ECM db with zero mac.
	 */
	if (is_zero_ether_addr(nmu->old_mac)) {
		DEBUG_WARN("old mac is zero\n");
		return NOTIFY_DONE;
	}

	DEBUG_TRACE("old mac: %pM new mac: %pM\n", nmu->old_mac, nmu->update_mac);

	DEBUG_INFO("neigh mac update notify for node %pM\n", nmu->old_mac);
	ecm_interface_node_connections_defunct((uint8_t *)nmu->old_mac, ECM_DB_IP_VERSION_IGNORE);

	return NOTIFY_DONE;
}

/*
 * struct notifier_block ecm_interface_neigh_mac_update_nb
 *	Registration for neighbour mac address update.
 */
static struct notifier_block ecm_interface_neigh_mac_update_nb = {
	.notifier_call = ecm_interface_neigh_mac_update_notify_event,
};
#endif

/*
 * ecm_interface_wifi_event_iwevent
 *	wireless event handler
 */
static int ecm_interface_wifi_event_iwevent(int ifindex, unsigned char *buf, size_t len)
{
	struct iw_event iwe_buf, *iwe = &iwe_buf;
	char *pos, *end;

	pos = buf;
	end = buf + len;
	while (pos + IW_EV_LCP_LEN <= end) {

		/*
		 * Copy the base data structure to get iwe->len
		 */
		memcpy(&iwe_buf, pos, IW_EV_LCP_LEN);

		/*
		 * Check that len is valid and that we have that much in the buffer.
		 *
		 */
		if (iwe->len < IW_EV_LCP_LEN) {
			return -1;
		}

		if ((iwe->len > sizeof (struct iw_event)) || (iwe->len + pos) > end) {
			return -1;
		}

		/*
		 * Do the copy again with the full length.
		 */
		memcpy(&iwe_buf, pos, iwe->len);

		if (iwe->cmd == IWEVREGISTERED) {
			DEBUG_INFO("STA %pM joining\n", (uint8_t *)iwe->u.addr.sa_data);
			ecm_interface_node_connections_defunct_by_type((uint8_t *)iwe->u.addr.sa_data, ECM_DB_IP_VERSION_IGNORE,
								ECM_DB_CONNECTION_DEFUNCT_TYPE_STA_JOIN);
		} else if (iwe->cmd == IWEVEXPIRED) {
			DEBUG_INFO("STA %pM leaving\n", (uint8_t *)iwe->u.addr.sa_data);
			ecm_interface_node_connections_defunct((uint8_t *)iwe->u.addr.sa_data, ECM_DB_IP_VERSION_IGNORE);
		} else {
			DEBUG_INFO("iwe->cmd is %d for STA %pM\n", iwe->cmd, (unsigned char *) iwe->u.addr.sa_data);
		}

		pos += iwe->len;
	}

	return 0;
}

/*
 * ecm_interface_wifi_event_newlink
 *	Link event handler
 */
static int ecm_interface_wifi_event_newlink(struct ifinfomsg *ifi, unsigned char *buf, size_t len)
{
	struct rtattr *attr;
	int attrlen, rta_len;

	DEBUG_TRACE("Event from interface %d\n", ifi->ifi_index);

	attrlen = len;
	attr = (struct rtattr *) buf;
	rta_len = RTA_ALIGN(sizeof(struct rtattr));

	while (RTA_OK(attr, attrlen)) {
		if (attr->rta_type == IFLA_WIRELESS) {
			ecm_interface_wifi_event_iwevent(ifi->ifi_index, ((char *) attr) + rta_len, attr->rta_len - rta_len);
		}
		attr = RTA_NEXT(attr, attrlen);
	}

	return 0;
}

/*
 * ecm_interface_wifi_event_handler
 *	Netlink event handler
 */
static int ecm_interface_wifi_event_handler(unsigned char *buf, int len)
{
	struct nlmsghdr *nlh;
	struct ifinfomsg *ifi;
	int left;

	nlh = (struct nlmsghdr *) buf;
	left = len;

	while (NLMSG_OK(nlh, left)) {
		switch (nlh->nlmsg_type) {
		case RTM_NEWLINK:
		case RTM_DELLINK:
			if (NLMSG_PAYLOAD(nlh, 0) < sizeof(struct ifinfomsg)) {
				DEBUG_INFO("invalid netlink message\n");
				break;
			}

			ifi = NLMSG_DATA(nlh);
			DEBUG_INFO("ifi->ifi_family: %d\n", ifi->ifi_family);
			if (ifi->ifi_family != AF_BRIDGE) {
				ecm_interface_wifi_event_newlink(ifi, (u8 *)ifi + NLMSG_ALIGN(sizeof(struct ifinfomsg)),
					NLMSG_PAYLOAD(nlh, sizeof(struct ifinfomsg)));
			}
			break;
		}

		nlh = NLMSG_NEXT(nlh, left);
	}

	return 0;
}

/*
 * ecm_interface_wifi_event_rx
 *	Receive netlink message from socket
 */
static int ecm_interface_wifi_event_rx(struct socket *sock, struct sockaddr_nl *addr, unsigned char *buf, int len)
{
	struct msghdr msg;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0))
	struct iovec  iov;
	mm_segment_t oldfs;
	int size;
#else
	struct kvec iov;
#endif

	iov.iov_base = buf;
	iov.iov_len  = len;

	msg.msg_flags = 0;
	msg.msg_name  = addr;
	msg.msg_namelen = sizeof(struct sockaddr_nl);
	msg.msg_control = NULL;
	msg.msg_controllen = 0;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0))
	oldfs = get_fs();
	set_fs(KERNEL_DS);
	iov_iter_init(&msg.msg_iter, READ, &iov, 1, len);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 7, 0))
	size = sock_recvmsg(sock, &msg, len, msg.msg_flags);
#else
	size = sock_recvmsg(sock, &msg, msg.msg_flags);
#endif
	set_fs(oldfs);

	return size;
#else
	return kernel_recvmsg(sock, &msg, &iov, 1, iov.iov_len, 0);
#endif
}

/*
 * ecm_interface_wifi_event_thread
 */
static void ecm_interface_wifi_event_thread(void)
{
	int err;
	int size;
	struct sockaddr_nl saddr;
	unsigned char buf[512];
	int len = sizeof(buf);

	kernel_sigaction(SIGKILL, SIG_DFL);
	err = sock_create(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE, &__ewn.sock);
	if (err < 0) {
		DEBUG_ERROR("failed to create sock\n");
		goto exit1;
	}

	memset(&saddr, 0, sizeof(saddr));
	saddr.nl_family = AF_NETLINK;
	saddr.nl_groups = RTNLGRP_LINK;
	saddr.nl_pid    = current->pid;

	err = __ewn.sock->ops->bind(__ewn.sock, (struct sockaddr *)&saddr, sizeof(struct sockaddr));
	if (err < 0) {
		DEBUG_ERROR("failed to bind sock\n");
		goto exit2;
	}

	DEBUG_INFO("ecm_interface_wifi_event thread started\n");
	while (!kthread_should_stop()) {
		size = ecm_interface_wifi_event_rx(__ewn.sock, &saddr, buf, len);
		DEBUG_TRACE("got a netlink msg with len %d\n", size);

		if (signal_pending(current))
			break;

		if (size < 0) {
			DEBUG_WARN("netlink rx error\n");
		} else {
			ecm_interface_wifi_event_handler(buf, size);
		}
	}

	DEBUG_INFO("ecm_interface_wifi_event thread stopped\n");
exit2:
	sock_release(__ewn.sock);
exit1:
	__ewn.sock = NULL;

	return;
}

/*
 * ecm_interface_wifi_event_start()
 */
int ecm_interface_wifi_event_start(void)
{
	if (__ewn.thread) {
		return 0;
	}

	__ewn.thread = kthread_run((void *)ecm_interface_wifi_event_thread, NULL, "ECM_wifi_event");
	if (IS_ERR(__ewn.thread)) {
		DEBUG_ERROR("Unable to start kernel thread\n");
		return -ENOMEM;
	}

	return 0;
}

/*
 * ecm_interface_wifi_event_stop()
 */
int ecm_interface_wifi_event_stop(void)
{
	int err;

	if (__ewn.thread == NULL) {
		return 0;
	}

	DEBUG_INFO("kill ecm_interface_wifi_event thread\n");

	send_sig(SIGKILL, __ewn.thread, 1);
	err = kthread_stop(__ewn.thread);
	__ewn.thread = NULL;

	return err;
}

#if 0 //defined(CONFIG_NET_CLS_ACT) && defined(ECM_CLASSIFIER_DSCP_IGS)
/*
 * ecm_interface_igs_enabled_handler()
 *	IGS enabled check sysctl node handler.
 */
static int ecm_interface_igs_enabled_handler(struct ctl_table *ctl, int write, void __user *buffer,
		 size_t *lenp, loff_t *ppos)
{
	int ret;
	int current_value;

	/*
	 * Take the current value
	 */
	current_value = ecm_interface_igs_enabled;

	/*
	 * Write the variable with user input
	 */
	ret = proc_dointvec(ctl, write, buffer, lenp, ppos);
	if (ret || (!write)) {
		return ret;
	}

	/*
	 * If the IGS feature is not supported in the selected frontend,
	 * just return.
	 */
	if (!ecm_front_end_is_feature_supported(ECM_FE_FEATURE_IGS)) {
		DEBUG_WARN("IGS is not supported by front-end\n");
		return -EINVAL;
	}

	if ((ecm_interface_igs_enabled != 1) && (ecm_interface_igs_enabled != 0)) {
		DEBUG_WARN("Invalid input. Valid values 0/1\n");
		ecm_interface_igs_enabled = current_value;
		return -EINVAL;
	}
	return 0;
}
#endif

/*
 * ecm_interface_src_check_handler()
 *	Source interface check sysctl node handler.
 */
static int ecm_interface_src_check_handler(struct ctl_table *ctl, int write, void __user *buffer, size_t *lenp, loff_t *ppos)
{
	int ret;
	int current_value;

	/*
	 * Take the current value
	 */
	current_value = ecm_interface_src_check;

	/*
	 * Write the variable with user input
	 */
	ret = proc_dointvec(ctl, write, buffer, lenp, ppos);
	if (ret || (!write)) {
		return ret;
	}

	/*
	 * If the src iface check feature is not supported in the selected frontend,
	 * just return.
	 */
	if (!ecm_front_end_is_feature_supported(ECM_FE_FEATURE_SRC_IF_CHECK)) {
		DEBUG_WARN("Source interface check is not supported by front-end\n");
		return -EINVAL;
	}

	if ((ecm_interface_src_check != 1) && (ecm_interface_src_check != 0)) {
		DEBUG_WARN("Invalid input. Valid values 0/1\n");
		ecm_interface_src_check = current_value;
		return -EINVAL;
	}

	return ret;
}

static struct ctl_table ecm_interface_table[] = {
	{
		.procname		= "src_interface_check",
		.data			= &ecm_interface_src_check,
		.maxlen			= sizeof(int),
		.mode			= 0644,
		.proc_handler		= &ecm_interface_src_check_handler,
	},
#if 0 //defined(CONFIG_NET_CLS_ACT) && defined(ECM_CLASSIFIER_DSCP_IGS)
	{
		.procname		= "igs_enabled",
		.data			= &ecm_interface_igs_enabled,
		.maxlen			= sizeof(int),
		.mode			= 0644,
		.proc_handler		= &ecm_interface_igs_enabled_handler,
	},
#endif
	{ }
};

static struct ctl_table ecm_interface_root_dir[] = {
	{
		.procname		= "ecm",
		.mode			= 0555,
		.child			= ecm_interface_table,
	},
	{ }
};

static struct ctl_table ecm_interface_root[] = {
	{
		.procname		= "net",
		.mode			= 0555,
		.child			= ecm_interface_root_dir,
	},
	{ }
};

#ifdef ECM_INTERFACE_IPSEC_GLUE_LAYER_SUPPORT_ENABLE
/*
 * ecm_interface_ipsec_register_callbacks()
 *	Register callbacks
 */
void ecm_interface_ipsec_register_callbacks(struct ecm_interface_ipsec_callback *cb)
{
	spin_lock_bh(&ecm_interface_lock);
	memcpy(&ecm_interface_ipsec_cb, cb, sizeof(struct ecm_interface_ipsec_callback));
	spin_unlock_bh(&ecm_interface_lock);
}
EXPORT_SYMBOL(ecm_interface_ipsec_register_callbacks);

/*
 * ecm_interface_ipsec_unregister_callbacks
 *	Unregister callbacks
 */
void ecm_interface_ipsec_unregister_callbacks(void)
{
	spin_lock_bh(&ecm_interface_lock);
	memset(&ecm_interface_ipsec_cb, 0, sizeof(struct ecm_interface_ipsec_callback));
	spin_unlock_bh(&ecm_interface_lock);
}
EXPORT_SYMBOL(ecm_interface_ipsec_unregister_callbacks);

#endif

#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE

/*
 * ecm_interface_ovs_defunct_masked_tuple()
 *	Make defunct based on masked fields
 */
static void ecm_interface_ovs_defunct_masked_tuple(struct ovsmgr_dp_flow *flow)
{
	ip_addr_t src_addr;
	ip_addr_t dest_addr;
	bool is_routed = false;

	bool smac_valid = !is_zero_ether_addr(flow->smac);
	bool dmac_valid = !is_zero_ether_addr(flow->dmac);

	if (flow->tuple.ip_version == 4) {
		ECM_NIN4_ADDR_TO_IP_ADDR(src_addr, flow->tuple.ipv4.src);
		ECM_NIN4_ADDR_TO_IP_ADDR(dest_addr, flow->tuple.ipv4.dst);
	} else {
		ECM_NIN6_ADDR_TO_IP_ADDR(src_addr, flow->tuple.ipv6.src);
		ECM_NIN6_ADDR_TO_IP_ADDR(dest_addr, flow->tuple.ipv6.dst);
	}

	if (netif_is_ovs_master(flow->indev) || netif_is_ovs_master(flow->outdev)) {
		is_routed = true;
	}

	/*
	 * This function attempts to defunct connections which match
	 * given OVS flow.  Some of the parameters in flow are masked so the
	 * logic used here is to match:
	 *  if (A & B == A), where B is flow parameter and A is CI parameter.
	 *  A, B can be MAC, IP, port, protocol.
	 *
	 * OVS datapath flow can be routed or bridged flow.  To identify bridge vs routed
	 * flow->{indev/outdev} is checked if it is OVS bridge interface and smac/dmac is
	 * matching interface MAC address.  Otherwise flow is considered to be bridge flows.
	 *
	 * This function and the underlying functions (called functions) implement above logic
	 * to match parameters.  SMAC/DMAC is expected to be non-zero, if they are 0 (any) then
	 * rest of the parameters are used to filter out ECM connections using OVS classifier Hash bucket.
	 * Otherwise node list is used to identify the connections originated from it and they are defunct.
	 * In this implementation, worst case is SMAC and DMAC are 0 and all OVS connections need to be
	 * parsed.
	 *
	 * Below is the sequence of checks and how ECM connections are filtered:
	 *
	 * 1. if SMAC and DMAC are invalid, defunct flows by masked matching (ip_version, sip, sport, protocol, dip, dport)
	 *
	 * 2. SMAC is valid (non zero) && outdev is OVS bridge:  defunction connections originated from nodes matching smac.
	 *		a. Check if outdev is OVS bridge interface and dmac is matching bridge MAC.
	 *		   If yes then defunct connections by matching {ip_version, sip, dip, protocol, sport, dport},
	 *		   dmac in CI is ignored (any destination node address).
	 *
	 * 3. DMAC is valid (non-zero) && indev is OVS bridge: - defunction connections originated to nodes matching dmac.
	 *		a. Check if indev is OVS bridge interface and smac is matching bridge MAC.
	 *		   If yes then defunct connections by matching {ip_version, ip, dip, protocol, sport, dport},
	 *		   smac in CI is ignored (any destination node address).
	 *
	 * 4. if either indev OR outdev is ovs_bridge: Then defunct connections through OVS classifier hash bucket by matching:
	 *		    {ip_version, sip, dip, protocol, sport, dport}
	 *
	 * 5. if SMAC is valid : Then delete bridge flows originating from SMAC by matching:
	 *		   {ip_version, dmac, sip, dip, protocol, sport, dport}
	 *
	 * 6. if DMAC is valid : Then delete bridge flows originating to DMAC by matching:
	 *		   {ip_version, smac, sip, dip, protocol, sport, dport}
	 *
	 * 7. If any of the above conndition is not true:  defunct flows by masked matching (ip_version, sip, sport, protocol, dip, dport)
	 */

	/*
	 * case 1:  Defunct by classifier if smac and dmac are invalid
	 */
	if (!smac_valid && !dmac_valid) {
		goto classify_defunct;
	}

	/*
	 * case 2: If outdev is ovs_br, find all connection FROM smac and MATCH
	 */
	if (smac_valid && netif_is_ovs_master(flow->outdev)) {
		if (unlikely(dmac_valid && !ECM_MAC_ADDR_MATCH(flow->outdev->dev_addr, flow->dmac))) {
			DEBUG_WARN("%px: Defunct routed connections FROM %pM to the %s (OVS bridge) Failed\n", flow, flow->smac,
								flow->outdev->name);
			return;
		}

		DEBUG_TRACE("%px: Defunct routed connections by masked 7 tuple FROM %pM to the %s (OVS bridge)\n", flow,
							flow->smac, flow->outdev->name);

		ecm_db_node_ovs_connections_masked_defunct(flow->tuple.ip_version, flow->smac, true, src_addr,
							   ntohs(flow->tuple.src_port), flow->dmac, false, dest_addr,
							   ntohs(flow->tuple.dst_port), flow->tuple.protocol,
							   ECM_DB_OBJ_DIR_FROM, true);
		return;
	}

	/*
	 * case 3: If indev is ovs_br, find all connection from TO  dmac and match.
	 */
	if (dmac_valid && netif_is_ovs_master(flow->indev)) {
		if (unlikely(smac_valid && !ECM_MAC_ADDR_MATCH(flow->indev->dev_addr, flow->smac))) {
			DEBUG_WARN("%px: Defunct routed connections TO %pM from %s (OVS bridge), Failed\n", flow,
				   flow->dmac, flow->indev->name);
			return;
		}

		DEBUG_TRACE("%px: Defunct routed the connections by masked 7 tuple TO %pM from %s (OVS bridge)\n", flow,
			    flow->dmac, flow->indev->name);

		ecm_db_node_ovs_connections_masked_defunct(flow->tuple.ip_version, flow->smac, false, src_addr,
							   ntohs(flow->tuple.src_port), flow->dmac, true, dest_addr,
							   ntohs(flow->tuple.dst_port), flow->tuple.protocol,
							   ECM_DB_OBJ_DIR_TO, true);

		return;
	}

	/*
	 * case 4: If smac or dmac is not valid and either of indev or outdev is ovs-bridge device, defunct by 5 tuple
	 */
	if (is_routed) {
		goto classify_defunct;
	}

	/*
	 * case 5: if smac is valid, match bridged connection FROM SMAC
	 */
	if (smac_valid) {
		DEBUG_TRACE("%px: Defunct Bridged flows by masked  7 tuple (smac valid) indev = %s, outdev = %s, smac:%pM, dmac:%pM, "
								"proto=%d, sport=%d, dport=%d\n",
					flow, flow->indev->name, flow->outdev->name, flow->smac, flow->dmac,
					flow->tuple.protocol, ntohs(flow->tuple.src_port), ntohs(flow->tuple.dst_port));

		ecm_db_node_ovs_connections_masked_defunct(flow->tuple.ip_version, flow->smac, true, src_addr,
							   ntohs(flow->tuple.src_port), flow->dmac,
							   dmac_valid, dest_addr,
							   ntohs(flow->tuple.dst_port), flow->tuple.protocol,
							   ECM_DB_OBJ_DIR_FROM, false);
		return;
	}

	/*
	 * case 6: if dmac is valid, match bridged connection TO DMAC
	 */
	if (dmac_valid) {
		DEBUG_TRACE("%px: Defunct Bridged flows by masked 7 tuple (dmac valid) indev = %s, outdev = %s, smac:%pM, dmac:%pM, "
								"proto=%d, sport=%d, dport=%d\n",
					flow, flow->indev->name, flow->outdev->name, flow->smac, flow->dmac,
					flow->tuple.protocol, ntohs(flow->tuple.src_port), ntohs(flow->tuple.dst_port));

		ecm_db_node_ovs_connections_masked_defunct(flow->tuple.ip_version, flow->smac, false, src_addr,
							   ntohs(flow->tuple.src_port), flow->dmac, true,  dest_addr,
							   ntohs(flow->tuple.dst_port), flow->tuple.protocol,
							   ECM_DB_OBJ_DIR_TO, false);

		return;
	}

classify_defunct:

	/*
	 * case 7: If dmac and smac are NULL, defunct connections by matching 5 tuple info
	 */
	DEBUG_TRACE("%px: Defunct flows by 5 tuple (smac & dmac invalid) indev = %s, outdev = %s, smac:%pM, dmac:%pM, "
					     "proto=%d, sport=%d, dport=%d is_routed=%d\n",
				flow, flow->indev->name, flow->outdev->name, flow->smac, flow->dmac,
				flow->tuple.protocol, ntohs(flow->tuple.src_port), ntohs(flow->tuple.dst_port), is_routed);

	ecm_db_connection_defunct_by_classifier(flow->tuple.ip_version, src_addr, ntohs(flow->tuple.src_port), dest_addr,
						ntohs(flow->tuple.dst_port), flow->tuple.protocol, is_routed, ECM_CLASSIFIER_TYPE_OVS);

}

/*
 * ecm_interface_ovs_flow_defunct_connections()
 *	This event can be triggered when the OVS flow is deleted due to flow timeout.
 *	Defunct the connections based on the OVS flow information.
 */
static void ecm_interface_ovs_flow_defunct_connections(struct ovsmgr_dp_flow *flow)
{
	ip_addr_t src_ip = ECM_IP_ADDR_NULL;
	ip_addr_t dest_ip = ECM_IP_ADDR_NULL;

	/*
	 * Delete by flow rule.
	 */

	if (flow->tuple.ip_version == 4) {
		DEBUG_TRACE("IPv4: Src: %pI4:%d protocol: %d Dst: %pI4:%d\n",
			   &flow->tuple.ipv4.src, ntohs(flow->tuple.src_port),
			   flow->tuple.protocol,
			   &flow->tuple.ipv4.dst, ntohs(flow->tuple.dst_port));

		if (flow->tuple.ipv4.src) {
			ECM_NIN4_ADDR_TO_IP_ADDR(src_ip, flow->tuple.ipv4.src);
		}

		if (flow->tuple.ipv4.dst) {
			ECM_NIN4_ADDR_TO_IP_ADDR(dest_ip, flow->tuple.ipv4.dst);
		}
	} else if (flow->tuple.ip_version == 6) {
		DEBUG_TRACE("IPv6: Src: %pI6:%d protocol: %d Dst: %pI6:%d\n",
			   &flow->tuple.ipv6.src, ntohs(flow->tuple.src_port),
			   flow->tuple.protocol,
			   &flow->tuple.ipv6.dst, ntohs(flow->tuple.dst_port));
		ECM_NIN6_ADDR_TO_IP_ADDR(src_ip, flow->tuple.ipv6.src);
		ECM_NIN6_ADDR_TO_IP_ADDR(dest_ip, flow->tuple.ipv6.dst);
	} else {
		DEBUG_WARN("%px: Unsupported IP version: %d\n", flow, flow->tuple.ip_version);
		return;
	}

	/*
	 * For multicast flows, ovs manager will set source ip
	 * as 0.0.0.0, we need to get the 'ci' from multicast
	 * destination ip
	 */
	if (ecm_ip_addr_is_multicast(dest_ip)) {
#ifdef ECM_MULTICAST_ENABLE
		ip_addr_t grp_ip;
		struct ecm_db_connection_instance *ci;
		struct ecm_db_multicast_tuple_instance *ti;

		/*
		 * Get the for the group in the tuple_instance table. As OVS
		 * does not support "source specific multicast" there can be
		 * only one tuple entry for a group.
		 */
		ti = ecm_db_multicast_connection_get_and_ref_first(dest_ip);
		if (!ti) {
			DEBUG_WARN("%px: no multicast tuple entry found\n", flow);
			return;
		}

		/*
		 * Force destruction of the connection by making it defunct
		 */
		while (ti) {
			struct ecm_db_multicast_tuple_instance *ti_next;

			ecm_db_multicast_tuple_instance_group_ip_get(ti, grp_ip);
			if (ECM_IP_ADDR_MATCH(grp_ip, dest_ip)) {
				ci = ecm_db_multicast_connection_get_from_tuple(ti);
				ecm_db_connection_make_defunct(ci);
			}
			ti_next = ecm_db_multicast_connection_get_and_ref_next(ti);
			ecm_db_multicast_connection_deref(ti);
			ti = ti_next;
		}
#endif
		return;
	}

	/*
	 * For unicast if 5-tuple is not valid, then delete the flows by
	 * {smac/dmac and indev/outdev}
	 */
	if ((flow->tuple.protocol == IPPROTO_TCP || flow->tuple.protocol == IPPROTO_UDP) &&
			flow->tuple.src_port && flow->tuple.dst_port &&
			!ECM_IP_ADDR_IS_NULL(src_ip) && !ECM_IP_ADDR_IS_NULL(dest_ip)) {
		struct ecm_db_connection_instance *ci;

		/*
		 * Delete the flows by using 5 tuple
		 */
		DEBUG_TRACE("%px: Delete flow by 5 tuple: indev = %s, outdev = %s, smac:%pM, dmac:%pM, "
					"proto=%d, sport=%d, dport=%d\n",
					flow, flow->indev->name, flow->outdev->name, flow->smac, flow->dmac,
					flow->tuple.protocol, ntohs(flow->tuple.src_port), ntohs(flow->tuple.dst_port));

		/*
		 * Delete the flows by using 5-tuple parameters.
		 */
		ci = ecm_db_connection_from_ovs_flow_get_and_ref(flow);
		if (!ci) {
			DEBUG_WARN("%px: OVS flow not found in ECM database, Try to match by mask\n", flow);
			goto no_ci;
		}
		DEBUG_INFO("%px: Connection defunct %px\n", flow, ci);

		/*
		 * Force destruction of the connection by making it defunct
		 */
		ecm_db_connection_make_defunct(ci);
		ecm_db_connection_deref(ci);
		return;
	}

no_ci:

	/*
	 * if all tuple values are 0, defunct by interface
	 */
	if (unlikely(is_zero_ether_addr(flow->smac) && is_zero_ether_addr(flow->dmac))) {
		bool sip_valid, dip_valid;

		if (flow->tuple.ip_version == 4) {
			sip_valid = !!flow->tuple.ipv4.src;
			dip_valid = !!flow->tuple.ipv4.dst;
		} else {
			sip_valid = !ipv6_addr_any((const struct in6_addr *)&flow->tuple.ipv6.src);
			dip_valid = !ipv6_addr_any((const struct in6_addr *)&flow->tuple.ipv6.dst);
		}

		if (sip_valid || dip_valid || flow->tuple.src_port || flow->tuple.dst_port || flow->tuple.protocol) {
			goto defunct_by_masked_tuple;
		}

		if (flow->indev && !netif_is_ovs_master(flow->indev)) {
			DEBUG_TRACE("%px: Defunct all flows by indev=%s\n", flow, flow->indev->name);
			ecm_interface_dev_defunct_connections(flow->indev);
		}

		if (flow->outdev && !netif_is_ovs_master(flow->outdev)) {
			DEBUG_TRACE("%px: Defunct all flows by outdev=%s\n", flow, flow->outdev->name);
			ecm_interface_dev_defunct_connections(flow->outdev);
		}

		DEBUG_TRACE("%px: Delete flow is called with all tuple 0\n", flow);
		return;
	}

defunct_by_masked_tuple:

	DEBUG_TRACE("%px: Delete flow by 5 or 7 tuple masks: indev = %s, outdev = %s, smac:%pM, dmac:%pM, "
		    "proto=%d, sport=%d, dport=%d\n",
		    flow, flow->indev->name, flow->outdev->name, flow->smac, flow->dmac,
		    flow->tuple.protocol, ntohs(flow->tuple.src_port), ntohs(flow->tuple.dst_port));

	ecm_interface_ovs_defunct_masked_tuple(flow);
}

#ifdef ECM_MULTICAST_ENABLE
/*
 * ecm_interface_multicast_ovs_flow_update_connections()
 *	Update the connections based on the OVS flow information.
 *
 *	This event is triggered by OVS when a new OVS port joins an
 *	already existing OVS multicast flow. We rely on this event rather
 *	than the MCS update event when an OVS port joins an existing flow.
 *	This is because when the MCS event is received, the OVS may not
 *	have yet updated its flow entry to add the new port,
 *	so ECM will not be able to query OVS to validate the new flow entry
 */
static void ecm_interface_multicast_ovs_flow_update_connections(struct ovsmgr_dp_flow *flow)
{
	ip_addr_t ip_dest_addr;
	struct ecm_db_connection_instance *ci;
	struct ecm_db_multicast_tuple_instance *ti;
	struct ecm_front_end_connection_instance *feci;
	struct net_device *brdev;

	if (flow->tuple.ip_version == 4) {
		ECM_NIN4_ADDR_TO_IP_ADDR(ip_dest_addr, flow->tuple.ipv4.dst);
		DEBUG_TRACE("IPv4: Src: %pI4:%d protocol: %d Dst: %pI4:%d\n",
			   &flow->tuple.ipv4.src, flow->tuple.src_port,
			   flow->tuple.protocol,
			   &flow->tuple.ipv4.dst, flow->tuple.dst_port);
	} else if (flow->tuple.ip_version == 6) {
		ECM_NIN6_ADDR_TO_IP_ADDR(ip_dest_addr, flow->tuple.ipv6.dst);
		DEBUG_TRACE("IPv6: Src: %pI6:%d protocol: %d Dst: %pI6:%d\n",
			   &flow->tuple.ipv6.src, flow->tuple.src_port,
			   flow->tuple.protocol,
			   &flow->tuple.ipv6.dst, flow->tuple.dst_port);
	} else {
		DEBUG_WARN("%px: Unsupported IP version: %d\n", flow, flow->tuple.ip_version);
		return;
	}

	if (!ecm_ip_addr_is_multicast(ip_dest_addr)) {
		DEBUG_WARN("%px: Change notification is supported only for multicast flows\n", flow);
		return;
	}

	/*
	 * Get the OVS bridge device.
	 */
	if (ecm_front_end_is_ovs_bridge_device(flow->outdev)) {
		brdev = flow->outdev;
	} else if (ecm_interface_is_ovs_bridge_port(flow->outdev)) {
		brdev = ovsmgr_dev_get_master(flow->outdev);
		if (!brdev) {
			DEBUG_WARN("%px: Master device for OVS port: %s is NULL!\n", flow, flow->outdev->name);
			return;
		}
	} else {
		DEBUG_WARN("%px: egress port: %s is neither OVS bridge nor OVS port\n", flow, flow->outdev->name);
		return;
	}

	/*
	 * Get the first entry for the group in the tuple_instance table,
	 */
	ti = ecm_db_multicast_connection_get_and_ref_first(ip_dest_addr);
	if (!ti) {
		DEBUG_WARN("%px: no multicast tuple entry found\n", flow);
		return;
	}

	ci = ecm_db_multicast_connection_get_from_tuple(ti);
	feci = ecm_db_connection_front_end_get_and_ref(ci);

	/*
	 * The source IP address in the OVS flow passed to us is always found
	 * to be NULL. So, update the all multicast connections for this group address.
	 */
	if (feci->multicast_update) {
		feci->multicast_update(ip_dest_addr, brdev);
	}

	ecm_front_end_connection_deref(feci);
	ecm_db_multicast_connection_deref(ti);
}
#endif

/*
 * ecm_interface_ovs_notifier_callback()
 *	Netdevice notifier callback to inform us of change of state of a netdevice
 */
static int ecm_interface_ovs_notifier_callback(struct notifier_block *nb, unsigned long event, void *data)
{
	struct ovsmgr_notifiers_info *ovs_info = (struct ovsmgr_notifiers_info *)data;
	struct ovsmgr_dp_port_info *port;

	DEBUG_INFO("OVS notifier event: %lu\n", event);

	switch(event) {
	case OVSMGR_DP_PORT_DEL:
		port = ovs_info->port;
		ecm_interface_dev_defunct_connections(port->dev);
		break;
	case OVSMGR_DP_FLOW_DEL:
		ecm_interface_ovs_flow_defunct_connections(ovs_info->flow);
		break;
	case OVSMGR_DP_FLOW_TBL_FLUSH:
		ecm_db_connection_defunct_all();
		break;
	case OVSMGR_DP_FLOW_CHANGE:
#ifdef ECM_MULTICAST_ENABLE
		ecm_interface_multicast_ovs_flow_update_connections(ovs_info->flow);
#endif
		break;
	}

	return NOTIFY_DONE;
}

/*
 * struct notifier_block ecm_interface_ovs_notifier
 *	Registration for OVS events
 */
static struct notifier_block ecm_interface_ovs_notifier __read_mostly = {
	.notifier_call = ecm_interface_ovs_notifier_callback,
};
#endif

/*
 * ecm_interface_init()
 */
int ecm_interface_init(void)
{
	int result;
	DEBUG_INFO("ECM Interface init\n");

	/*
	 * Register sysctl table.
	 */
	ecm_interface_ctl_table_header = register_sysctl_table(ecm_interface_root);

	result = register_netdevice_notifier(&ecm_interface_netdev_notifier);
	if (result != 0) {
		DEBUG_ERROR("Failed to register netdevice notifier %d\n", result);
		unregister_sysctl_table(ecm_interface_ctl_table_header);
		return result;
	}
#if defined(ECM_DB_XREF_ENABLE) && defined(ECM_BAND_STEERING_ENABLE)
	/*
	 * If the bridge feature is supported in the selected frontend,
	 * register the  FDB event handlers.
	 */
	if (ecm_front_end_is_feature_supported(ECM_FE_FEATURE_BRIDGE)) {
		br_fdb_update_register_notify(&ecm_interface_node_br_fdb_update_nb);
		br_fdb_register_notify(&ecm_interface_node_br_fdb_delete_nb);
	}
#endif
#ifdef ECM_DB_XREF_ENABLE
	neigh_mac_update_register_notify(&ecm_interface_neigh_mac_update_nb);
#endif
#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
	ovsmgr_notifier_register(&ecm_interface_ovs_notifier);
#endif
	ecm_interface_wifi_event_start();

	return 0;
}
EXPORT_SYMBOL(ecm_interface_init);

/*
 * ecm_interface_exit()
 */
void ecm_interface_exit(void)
{
	DEBUG_INFO("ECM Interface exit\n");

	spin_lock_bh(&ecm_interface_lock);
	ecm_interface_terminate_pending  = true;
	spin_unlock_bh(&ecm_interface_lock);

	unregister_netdevice_notifier(&ecm_interface_netdev_notifier);
#ifdef ECM_DB_XREF_ENABLE
	neigh_mac_update_unregister_notify(&ecm_interface_neigh_mac_update_nb);
#endif

#if defined(ECM_DB_XREF_ENABLE) && defined(ECM_BAND_STEERING_ENABLE)
	/*
	 * If the bridge feature is supported in the selected frontend,
	 * unregister the  FDB event handlers.
	 */
	if (ecm_front_end_is_feature_supported(ECM_FE_FEATURE_BRIDGE)) {
		br_fdb_update_unregister_notify(&ecm_interface_node_br_fdb_update_nb);
		br_fdb_unregister_notify(&ecm_interface_node_br_fdb_delete_nb);
	}
#endif
	ecm_interface_wifi_event_stop();

#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
	ovsmgr_notifier_unregister(&ecm_interface_ovs_notifier);
#endif
	/*
	 * Unregister sysctl table.
	 */
	if (ecm_interface_ctl_table_header) {
		unregister_sysctl_table(ecm_interface_ctl_table_header);
	}
}
EXPORT_SYMBOL(ecm_interface_exit);
