/*
 **************************************************************************
 * Copyright (c) 2014-2017, The Linux Foundation.  All rights reserved.
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
#include <linux/module.h>
#include <linux/of.h>
#include <linux/debugfs.h>
#include <linux/inet.h>
#include <linux/etherdevice.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/ip.h>
#include <net/ipv6.h>
#include <net/ip6_route.h>
#include <net/route.h>

/*
 * Debug output levels
 * 0 = OFF
 * 1 = ASSERTS / ERRORS
 * 2 = 1 + WARN
 * 3 = 2 + INFO
 * 4 = 3 + TRACE
 */
#define DEBUG_LEVEL ECM_FRONT_END_IPV6_DEBUG_LEVEL

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
#include "ecm_front_end_ipv6.h"

/*
 * General operational control
 */
int ecm_front_end_ipv6_stopped = 0;	/* When non-zero further traffic will not be processed */

/*
 * ecm_front_end_ipv6_interface_construct_ip_addr_set()
 *	Sets the IP addresses.
 *
 * Sets the ip address fields of the ecm_front_end_interface_construct_instance
 * with the given ip addresses.
 */
static void ecm_front_end_ipv6_interface_construct_ip_addr_set(struct ecm_front_end_interface_construct_instance *efeici,
							ip_addr_t from_mac_lookup, ip_addr_t to_mac_lookup)
{
	ECM_IP_ADDR_COPY(efeici->from_mac_lookup_ip_addr, from_mac_lookup);
	ECM_IP_ADDR_COPY(efeici->to_mac_lookup_ip_addr, to_mac_lookup);
}

/*
 * ecm_front_end_ipv6_interface_construct_netdev_set()
 *	Sets the net devices.
 *
 * Sets the net device fields of the ecm_front_end_interface_construct_instance
 * with the given net devices.
 */
static void ecm_front_end_ipv6_interface_construct_netdev_set(struct ecm_front_end_interface_construct_instance *efeici,
							struct net_device *from, struct net_device *from_other,
							struct net_device *to, struct net_device *to_other)
{
	efeici->from_dev = from;
	efeici->from_other_dev = from_other;
	efeici->to_dev = to;
	efeici->to_other_dev = to_other;
}

/*
 * ecm_front_end_ipv6_interface_construct_netdev_put()
 *	Release the references of the net devices.
 */
void ecm_front_end_ipv6_interface_construct_netdev_put(struct ecm_front_end_interface_construct_instance *efeici)
{
	dev_put(efeici->from_dev);
	dev_put(efeici->from_other_dev);
	dev_put(efeici->to_dev);
	dev_put(efeici->to_other_dev);
}

/*
 * ecm_front_end_ipv6_interface_construct_netdev_hold()
 *	Holds the references of the netdevices.
 */
void ecm_front_end_ipv6_interface_construct_netdev_hold(struct ecm_front_end_interface_construct_instance *efeici)
{
	dev_hold(efeici->from_dev);
	dev_hold(efeici->from_other_dev);
	dev_hold(efeici->to_dev);
	dev_hold(efeici->to_other_dev);
}

/*
 * ecm_front_end_ipv6_interface_construct_set_and_hold()
 *	Sets the IPv6 ECM front end interface construct instance,
 *	and holds the net devices.
 */
bool ecm_front_end_ipv6_interface_construct_set_and_hold(struct sk_buff *skb, ecm_tracker_sender_type_t sender, ecm_db_direction_t ecm_dir, bool is_routed,
							struct net_device *in_dev, struct net_device *out_dev,
							ip_addr_t ip_src_addr, ip_addr_t ip_dest_addr,
							struct ecm_front_end_interface_construct_instance *efeici)
{
	struct dst_entry *dst = skb_dst(skb);
	struct rt6_info *rt = (struct rt6_info *)dst;
	ip_addr_t rt_dst_addr;
	struct net_device *rt_iif_dev = NULL;
	struct net_device *from = NULL;
	struct net_device *from_other = NULL;
	struct net_device *to = NULL;
	struct net_device *to_other = NULL;
	ip_addr_t from_mac_lookup;
	ip_addr_t to_mac_lookup;

	/*
	 * Set the rt_dst_addr with the destination IP address by default.
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

		ECM_IP_ADDR_COPY(from_mac_lookup, ip_src_addr);
		ECM_IP_ADDR_COPY(to_mac_lookup, ip_dest_addr);
	} else {
		/*
		 * Routed
		 */
		if (!rt) {
			DEBUG_WARN("rt6_info is NULL\n");
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

		DEBUG_TRACE("in_dev: %s\n", in_dev->name);
		DEBUG_TRACE("out_dev: %s\n", out_dev->name);
		DEBUG_TRACE("dst->dev: %s\n", dst->dev->name);
		DEBUG_TRACE("rt_iif_dev: %s\n", rt_iif_dev->name);
		DEBUG_TRACE("%p: rt6i_dst.addr: %pi6\n", rt, &rt->rt6i_dst.addr);
		DEBUG_TRACE("%p: rt6i_src.addr: %pi6\n", rt, &rt->rt6i_src.addr);
		DEBUG_TRACE("%p: rt6i_gateway: %pi6\n", rt, &rt->rt6i_gateway);
		DEBUG_TRACE("%p: rt6i_idev: %s\n", rt, rt->rt6i_idev->dev->name);
		DEBUG_TRACE("%p: skb->dev: %s\n", rt, skb->dev->name);

		DEBUG_INFO("ip_src_addr: " ECM_IP_ADDR_OCTAL_FMT "\n", ECM_IP_ADDR_TO_OCTAL(ip_src_addr));
		DEBUG_INFO("ip_dest_addr: " ECM_IP_ADDR_OCTAL_FMT "\n", ECM_IP_ADDR_TO_OCTAL(ip_dest_addr));

		/*
		 * If the destination host is behind a gateway, use the gateway address as destination
		 * for routed connections.
		 */
		if (!ECM_IP_ADDR_MATCH(rt->rt6i_dst.addr.in6_u.u6_addr32, rt->rt6i_gateway.in6_u.u6_addr32) || (rt->rt6i_flags & RTF_GATEWAY)) {
			if (!ECM_IP_ADDR_IS_NULL(rt->rt6i_gateway.in6_u.u6_addr32)) {
				ECM_NIN6_ADDR_TO_IP_ADDR(rt_dst_addr, rt->rt6i_gateway);
			}
		}

		from = rt_iif_dev;
		from_other = dst->dev;
		to = dst->dev;
		to_other = rt_iif_dev;

		ECM_IP_ADDR_COPY(from_mac_lookup, ip_src_addr);
		ECM_IP_ADDR_COPY(to_mac_lookup, rt_dst_addr);
	}

	ecm_front_end_ipv6_interface_construct_netdev_set(efeici, from, from_other,
								to, to_other);

	ecm_front_end_ipv6_interface_construct_netdev_hold(efeici);

	ecm_front_end_ipv6_interface_construct_ip_addr_set(efeici, from_mac_lookup, to_mac_lookup);

	/*
	 * Release the iff_dev which was hold by the dev_get_by_index() call.
	 */
	if (rt_iif_dev) {
		dev_put(rt_iif_dev);
	}

	return true;
}

/*
 * ecm_front_end_ipv6_stop()
 */
void ecm_front_end_ipv6_stop(int num)
{
	ecm_front_end_ipv6_stopped = num;
}

/*
 * ecm_front_end_ipv6_init()
 */
int ecm_front_end_ipv6_init(struct dentry *dentry)
{
	debugfs_create_u32("front_end_ipv6_stop", S_IRUGO | S_IWUSR, dentry,
					(u32 *)&ecm_front_end_ipv6_stopped);

	switch (ecm_front_end_type_get()) {
	case ECM_FRONT_END_TYPE_NSS:
		return ecm_nss_ipv6_init(dentry);
	case ECM_FRONT_END_TYPE_SFE:
		return ecm_sfe_ipv6_init(dentry);
	default:
		DEBUG_ERROR("Failed to init ipv6 front end\n");
		return -1;
	}
}

/*
 * ecm_front_end_ipv6_exit()
 */
void ecm_front_end_ipv6_exit(void)
{
	switch (ecm_front_end_type_get()) {
	case ECM_FRONT_END_TYPE_NSS:
		ecm_nss_ipv6_exit();
		break;
	case ECM_FRONT_END_TYPE_SFE:
		ecm_sfe_ipv6_exit();
		break;
	default:
		DEBUG_ERROR("Failed to exit from front end\n");
		break;
	}
}

