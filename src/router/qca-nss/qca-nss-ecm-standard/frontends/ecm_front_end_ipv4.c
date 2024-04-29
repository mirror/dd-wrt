/*
 **************************************************************************
 * Copyright (c) 2015-2016, 2020-2021 The Linux Foundation. All rights reserved.
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
#include <linux/module.h>
#include <linux/of.h>
#include <linux/debugfs.h>
#include <linux/inet.h>
#include <linux/etherdevice.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/ip.h>
#include <net/ipv6.h>
#include <net/route.h>

/*
 * Debug output levels
 * 0 = OFF
 * 1 = ASSERTS / ERRORS
 * 2 = 1 + WARN
 * 3 = 2 + INFO
 * 4 = 3 + TRACE
 */
#define DEBUG_LEVEL ECM_FRONT_END_IPV4_DEBUG_LEVEL

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
#include "ecm_interface.h"
#include "ecm_ipv4.h"

/*
 * General operational control
 */
int ecm_front_end_ipv4_stopped = 0;	/* When non-zero further traffic will not be processed */

/*
 * ecm_front_end_ipv4_interface_construct_ip_addr_set()
 *	Sets the IP addresses.
 *
 * Sets the ip address fields of the ecm_front_end_interface_construct_instance
 * with the given ip addresses.
 */
static void ecm_front_end_ipv4_interface_construct_ip_addr_set(struct ecm_front_end_interface_construct_instance *efeici,
							ip_addr_t from_mac_lookup, ip_addr_t to_mac_lookup,
							ip_addr_t from_nat_mac_lookup, ip_addr_t to_nat_mac_lookup)
{
	ECM_IP_ADDR_COPY(efeici->from_mac_lookup_ip_addr, from_mac_lookup);
	ECM_IP_ADDR_COPY(efeici->to_mac_lookup_ip_addr, to_mac_lookup);
	ECM_IP_ADDR_COPY(efeici->from_nat_mac_lookup_ip_addr, from_nat_mac_lookup);
	ECM_IP_ADDR_COPY(efeici->to_nat_mac_lookup_ip_addr, to_nat_mac_lookup);
}

/*
 * ecm_front_end_ipv4_interface_construct_netdev_set()
 *	Sets the net devices.
 *
 * Sets the net device fields of the ecm_front_end_interface_construct_instance
 * with the given net devices.
 */
static void ecm_front_end_ipv4_interface_construct_netdev_set(struct ecm_front_end_interface_construct_instance *efeici,
							struct net_device *from, struct net_device *from_other,
							struct net_device *to, struct net_device *to_other,
							struct net_device *from_nat, struct net_device *from_nat_other,
							struct net_device *to_nat, struct net_device *to_nat_other)
{
	efeici->from_dev = from;
	efeici->from_other_dev = from_other;
	efeici->to_dev = to;
	efeici->to_other_dev = to_other;
	efeici->from_nat_dev = from_nat;
	efeici->from_nat_other_dev = from_nat_other;
	efeici->to_nat_dev = to_nat;
	efeici->to_nat_other_dev = to_nat_other;
}

/*
 * ecm_front_end_ipv4_interface_construct_netdev_put()
 *	Release the references of the net devices.
 */
void ecm_front_end_ipv4_interface_construct_netdev_put(struct ecm_front_end_interface_construct_instance *efeici)
{
	dev_put(efeici->from_dev);
	dev_put(efeici->from_other_dev);
	dev_put(efeici->to_dev);
	dev_put(efeici->to_other_dev);
	dev_put(efeici->from_nat_dev);
	dev_put(efeici->from_nat_other_dev);
	dev_put(efeici->to_nat_dev);
	dev_put(efeici->to_nat_other_dev);
}

/*
 * ecm_front_end_ipv4_interface_construct_netdev_hold()
 *	Holds the references of the netdevices.
 */
void ecm_front_end_ipv4_interface_construct_netdev_hold(struct ecm_front_end_interface_construct_instance *efeici)
{
	dev_hold(efeici->from_dev);
	dev_hold(efeici->from_other_dev);
	dev_hold(efeici->to_dev);
	dev_hold(efeici->to_other_dev);
	dev_hold(efeici->from_nat_dev);
	dev_hold(efeici->from_nat_other_dev);
	dev_hold(efeici->to_nat_dev);
	dev_hold(efeici->to_nat_other_dev);
}

/*
 * ecm_front_end_ipv4_interface_construct_set_and_hold()
 *	Sets the IPv4 ECM front end interface construct instance,
 *	and holds the net devices.
 */
bool ecm_front_end_ipv4_interface_construct_set_and_hold(struct sk_buff *skb, ecm_tracker_sender_type_t sender, ecm_db_direction_t ecm_dir, bool is_routed,
							struct net_device *in_dev, struct net_device *out_dev,
							ip_addr_t ip_src_addr, ip_addr_t ip_src_addr_nat,
							ip_addr_t ip_dest_addr, ip_addr_t ip_dest_addr_nat,
							struct ecm_front_end_interface_construct_instance *efeici)
{
	struct dst_entry *dst = skb_dst(skb);
	struct rtable *rt = (struct rtable *)dst;
	struct net_device *rt_iif_dev = NULL;
	ip_addr_t rt_dst_addr;
	struct net_device *from = NULL;
	struct net_device *from_other = NULL;
	struct net_device *to = NULL;
	struct net_device *to_other = NULL;
	struct net_device *from_nat = NULL;
	struct net_device *from_nat_other = NULL;
	struct net_device *to_nat = NULL;
	struct net_device *to_nat_other = NULL;
	struct net_device *dst_dev = NULL;
	ip_addr_t from_mac_lookup;
	ip_addr_t to_mac_lookup;
	ip_addr_t from_nat_mac_lookup;
	ip_addr_t to_nat_mac_lookup;
	bool gateway = false;
	bool dst_dev_override = false;

	/*
	 * Set the rt_dst_addr with the destination IP address by default.
	 * If there is a gateway IP address, this will be overwritten with the gateway IP address.
	 */
	ECM_IP_ADDR_COPY(rt_dst_addr, ip_dest_addr);

	if (!is_routed) {
		/*
		 * Bridged
		 */
		from = in_dev;
		from_other = in_dev;
		to = out_dev;
		to_other = out_dev;
	} else {
		__be32 rt_gw4;

		if (!rt) {
			DEBUG_WARN("rtable is NULL\n");
			return false;
		}

		/*
		 * If the flow is routed, extract the route information from the skb.
		 * Print the extracted information for debug purpose.
		 */
		rt_iif_dev = dev_get_by_index(&init_net, skb->skb_iif);
		if (!rt_iif_dev) {
			DEBUG_WARN("No rt_iif dev\n");
			return false;
		}
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(5, 2, 0))
		rt_gw4 = rt->rt_gateway;
#else
		rt_gw4 = rt->rt_gw4;
#endif
		dst_dev = dst->dev;

#ifdef ECM_XFRM_ENABLE
		/*
		 * If the dst is an xfrm dst, then override the dst_dev.
		*/
		if (dst_xfrm(dst)) {
			int32_t if_type;
			struct net_device *xfrm_dst_dev = ecm_interface_get_and_hold_ipsec_tun_netdev(NULL, skb, &if_type);
			/*
			 * If we reach here and are unable to find the tunnel netdevice,
			 * then return failure.
			 */
			if (!xfrm_dst_dev) {
				if (rt_iif_dev) {
					dev_put(rt_iif_dev);
				}
				return false;
			}

			dst_dev = xfrm_dst_dev;
			dst_dev_override = true;
		}

#endif
		DEBUG_TRACE("dst->dev: %s dst_dev: %s\n", dst->dev->name, dst_dev->name);
		DEBUG_TRACE("%px: rt gateway: %pI4\n", rt, &rt_gw4);

		DEBUG_INFO("ip_src_addr" ECM_IP_ADDR_DOT_FMT "\n", ECM_IP_ADDR_TO_DOT(ip_src_addr));
		DEBUG_INFO("ip_src_addr_nat" ECM_IP_ADDR_DOT_FMT "\n", ECM_IP_ADDR_TO_DOT(ip_src_addr_nat));
		DEBUG_INFO("ip_dest_addr" ECM_IP_ADDR_DOT_FMT "\n", ECM_IP_ADDR_TO_DOT(ip_dest_addr));
		DEBUG_INFO("ip_dest_addr_nat" ECM_IP_ADDR_DOT_FMT "\n", ECM_IP_ADDR_TO_DOT(ip_dest_addr_nat));

		if (rt->rt_uses_gateway || (rt->rt_flags & RTF_GATEWAY)) {
			/*
			 * Overwrite the rt_dst_addr with the gateway IP address. The destination host is
			 * behind a gateway.
			 */
			DEBUG_TRACE("Gateway address will be looked up overwrite the rt_dst_addr\n");
			ECM_NIN4_ADDR_TO_IP_ADDR(rt_dst_addr, rt_gw4)
			gateway = true;
		}

		DEBUG_INFO("rt_dst_addr" ECM_IP_ADDR_DOT_FMT "\n", ECM_IP_ADDR_TO_DOT(rt_dst_addr));

		/*
		 * Initialize the interfaces with defaults.
		 * rt_iif_dev is the interface which the packet comes in to the system,
		 * dst->dev is the interface which the packet goes out from the system.
		 */
		from = rt_iif_dev;
		from_other = dst_dev;
		to = dst_dev;
		to_other = rt_iif_dev;
	}

	/*
	 * Just in case the is_routed flag comes as 0, but the ecm_dir is different than
	 * bridge flow, we check the netdevices here before setting the efeici fields.
	 *
	 * TODO: Why ecm_dir comes as non-bridge flow, even though the is_routed flag is bridged?
	 */
	if (ecm_dir != ECM_DB_DIRECTION_BRIDGED) {
		if (!dst || !dst_dev || !rt_iif_dev) {
			DEBUG_WARN("Traffic is not bridged but the netdevs are not valid\n");
			if (rt_iif_dev) {
				dev_put(rt_iif_dev);
			}
			if (dst_dev_override) {
				dev_put(dst_dev);
			}
			return false;
		}
	}

	/*
	 * Initialize the mac lookup ip addresses with defaults.
	 */
	ECM_IP_ADDR_COPY(from_mac_lookup, ip_src_addr);
	ECM_IP_ADDR_COPY(to_mac_lookup, rt_dst_addr);
	ECM_IP_ADDR_COPY(from_nat_mac_lookup, ip_src_addr_nat);

	/*
	 * If we have a gateway IP address we should use it for the
	 * to_nat_mac_lookup IP address.
	 * Note that in hairpin NAT the destination IP address and the destination
	 * NAT IP addresses are different than each other. Because of this we cannot
	 * use the rt_dst_addr for to_nat_mac_lookup as well. In a normal routing
	 * traffic they are equal.
	 */
	if (gateway) {
		ECM_IP_ADDR_COPY(to_nat_mac_lookup, rt_dst_addr);
	} else {
		ECM_IP_ADDR_COPY(to_nat_mac_lookup, ip_dest_addr_nat);
	}

	/*
	 * Based on the flow and connection direction, set the NAT'd net devices.
	 * The above IP address settings are valid for each flow and connection direction case.
	 */
	if (sender == ECM_TRACKER_SENDER_TYPE_SRC) {
		if (ecm_dir == ECM_DB_DIRECTION_EGRESS_NAT) {
			from_nat = dst_dev;
			from_nat_other = rt_iif_dev;
			to_nat = dst_dev;
			to_nat_other = rt_iif_dev;
		} else if (ecm_dir == ECM_DB_DIRECTION_NON_NAT) {
			from_nat = rt_iif_dev;
			from_nat_other = dst_dev;
			to_nat = dst_dev;
			to_nat_other = rt_iif_dev;
		} else if (ecm_dir == ECM_DB_DIRECTION_INGRESS_NAT) {
			from_nat = rt_iif_dev;
			from_nat_other = dst_dev;
			to_nat = rt_iif_dev;
			to_nat_other = dst_dev;
		} else if (ecm_dir == ECM_DB_DIRECTION_BRIDGED) {
			from_nat = in_dev;
			from_nat_other = in_dev;
			to_nat = out_dev;
			to_nat_other = out_dev;
		} else {
			DEBUG_ASSERT(false, "Unhandled ecm_dir: %d\n", ecm_dir);
		}
	} else {
		if (ecm_dir == ECM_DB_DIRECTION_EGRESS_NAT) {
			from_nat = rt_iif_dev;
			from_nat_other = dst_dev;
			to_nat = rt_iif_dev;
			to_nat_other = dst_dev;
		} else if (ecm_dir == ECM_DB_DIRECTION_NON_NAT) {
			from_nat = rt_iif_dev;
			from_nat_other = dst_dev;
			to_nat = dst_dev;
			to_nat_other = rt_iif_dev;
		} else if (ecm_dir == ECM_DB_DIRECTION_INGRESS_NAT) {
			from_nat = dst_dev;
			from_nat_other = rt_iif_dev;
			to_nat = dst_dev;
			to_nat_other = rt_iif_dev;
		} else if (ecm_dir == ECM_DB_DIRECTION_BRIDGED) {
			from_nat = in_dev;
			from_nat_other = in_dev;
			to_nat = out_dev;
			to_nat_other = out_dev;
		} else {
			DEBUG_ASSERT(false, "Unhandled ecm_dir: %d\n", ecm_dir);
		}
	}

	ecm_front_end_ipv4_interface_construct_netdev_set(efeici, from, from_other,
								to, to_other,
								from_nat, from_nat_other,
								to_nat, to_nat_other);

	ecm_front_end_ipv4_interface_construct_netdev_hold(efeici);

	ecm_front_end_ipv4_interface_construct_ip_addr_set(efeici, from_mac_lookup, to_mac_lookup,
								from_nat_mac_lookup, to_nat_mac_lookup);

	if (dst_dev_override) {
		dev_put(dst_dev);
	}

	/*
	 * Release the iff_dev which was hold by the dev_get_by_index() call.
	 */
	if (rt_iif_dev) {
		dev_put(rt_iif_dev);
	}

	return true;
}

/*
 * ecm_front_end_ipv4_stop()
 */
void ecm_front_end_ipv4_stop(int num)
{
	ecm_front_end_ipv4_stopped = num;
}

/*
 * ecm_front_end_ipv4_init()
 */
int ecm_front_end_ipv4_init(struct dentry *dentry)
{
	debugfs_create_u32("front_end_ipv4_stop", S_IRUGO | S_IWUSR, dentry,
			   (u32 *)&ecm_front_end_ipv4_stopped);

	return ecm_ipv4_init(dentry);
}

/*
 * ecm_front_end_ipv4_exit()
 */
void ecm_front_end_ipv4_exit(void)
{
	ecm_ipv4_exit();
}
