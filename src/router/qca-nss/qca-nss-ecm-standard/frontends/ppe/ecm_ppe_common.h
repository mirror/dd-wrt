/*
 **************************************************************************
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifdef ECM_INTERFACE_MAP_T_ENABLE
#include <nat46-core.h>
#endif

#include <ppe_drv.h>
#include <ppe_drv_v4.h>
#include <ppe_drv_v6.h>

#include <net/xfrm.h>

/*
 * This macro converts ECM ip_addr_t to PPE IPv6 address
 */
#define ECM_IP_ADDR_TO_PPE_IPV6_ADDR(ppe6, ipaddrt) \
	{ \
		ecm_type_check_ae_ipv6(ppe6); \
		ecm_type_check_ecm_ip_addr(ipaddrt); \
		ppe6[0] = ipaddrt[3]; \
		ppe6[1] = ipaddrt[2]; \
		ppe6[2] = ipaddrt[1]; \
		ppe6[3] = ipaddrt[0]; \
	}

/*
 * This macro converts PPE IPv6 address to ECM ip_addr_t
 */
#define ECM_PPE_IPV6_ADDR_TO_IP_ADDR(ipaddrt, ppe6) \
	{ \
		ecm_type_check_ecm_ip_addr(ipaddrt); \
		ecm_type_check_ae_ipv6(ppe6); \
		ipaddrt[0] = ppe6[3]; \
		ipaddrt[1] = ppe6[2]; \
		ipaddrt[2] = ppe6[1]; \
		ipaddrt[3] = ppe6[0]; \
	}

/*
 * ecm_ppe_common_get_ae_iface_id_by_netdev_id()
 *	Gets the PPE interface id from the netdevice interface index.
 */
static inline int32_t ecm_ppe_common_get_ae_iface_id_by_netdev_id(int32_t ifindex)
{
	struct net_device *dev;
	int32_t ae_iface_id;

	dev = dev_get_by_index(&init_net, ifindex);
	if (!dev) {
		DEBUG_WARN("unable to find net device with %d ifindex\n", ifindex);
		return -1;
	}

	ae_iface_id = ppe_drv_iface_idx_get_by_dev(dev);

	dev_put(dev);

	return ae_iface_id;
}

/*
 * ecm_ppe_common_get_interface_type()
 * 	PPE doesn't support the concept of "type" for interfaces.
 */
static inline int32_t ecm_ppe_common_get_interface_type(struct ecm_front_end_connection_instance *feci, struct net_device *dev)
{
	DEBUG_WARN("%px: PPE doesn't have interface TYPE supported\n", dev);
	return -1;
}

/*
 * ecm_ppe_common_get_interface_number_by_dev()
 *	Returns the acceleration engine interface number based on the net_device object.
 */
static inline int32_t ecm_ppe_common_get_interface_number_by_dev(struct net_device *dev)
{
	return dev->ifindex;
}

/*
 * ecm_ppe_common_get_interface_number_by_dev_type()
 * 	PPE doesn't support the concept of "type" for interfaces.
 */
static inline int32_t ecm_ppe_common_get_interface_number_by_dev_type(struct net_device *dev, uint32_t type)
{
	return ecm_ppe_common_get_interface_number_by_dev(dev);
}

/*
 * ecm_ppe_common_connection_regenerate()
 *	Re-generate a specific connection in PPE front end
 */
static inline void ecm_ppe_common_connection_regenerate(struct ecm_front_end_connection_instance *feci, struct ecm_db_connection_instance *ci)
{
	/*
	 * Flag the connection as needing re-generation.
	 * Re-generation occurs when we next see traffic OR an acceleration engine sync for this connection.
	 * Refer to front end protocol specific process() functions.
	 */
	ecm_db_connection_regeneration_needed(ci);
}

/*
 * ecm_ppe_feature_check()
 *	Check some specific features for PPE acceleration
 */
static inline bool ecm_ppe_feature_check(struct sk_buff *skb, struct ecm_tracker_ip_header *ip_hdr)
{
	/*
	 * Check if this is xfrm flow and can be accelerated via PPE.
	 */
#ifdef CONFIG_XFRM
	if (!dev_net(skb->dev)->xfrm.policy_count[XFRM_POLICY_OUT]) {
		goto not_xfrm;
	}

	/*
	 * Packet seen after output transformation. We use the IPCB(skb) to check
	 * for this condition. No custom code should mangle the IPCB: skb->cb area,
	 * while the packet is traversing through the INET layer.
	 *
	 * Accelerate outer flow through PPE.
	 */
	if (ip_hdr->is_v4) {
		if ((IPCB(skb)->flags & IPSKB_XFRM_TRANSFORMED)) {
			DEBUG_TRACE("%px: Packet has undergone xfrm transformation\n", skb);
			return true;
		}
	} else if (IP6CB(skb)->flags & IP6SKB_XFRM_TRANSFORMED) {
		DEBUG_TRACE("%px: Packet has undergone xfrm transformation\n", skb);
		return true;
	}

	if (ip_hdr->protocol == IPPROTO_ESP) {
		DEBUG_TRACE("%px: ESP Passthrough packet\n", skb);
		goto not_xfrm;
	}

	/*
	 * skb's sp is set for decapsulated packet.
	 * Dont accelerate inner flow.
	 */
	if (secpath_exists(skb)) {
		DEBUG_TRACE("%px: Packet has undergone xfrm decapsulation((%d)\n", skb, ip_hdr->protocol);
		return false;
	}

	/*
	 * dst->xfrm is valid for lan to wan plain packet
	 */
	if (skb_dst(skb) && skb_dst(skb)->xfrm) {
		DEBUG_TRACE("%px: Plain text packet destined for xfrm(%d)\n", skb, ip_hdr->protocol);
		return false;
	}

not_xfrm:
#endif

	/*
	 * TODO: Should we add some features to be rejected in PPE frontend?
	 */

	return true;
}

/*
 * ecm_ppe_common_dummy_get_stats_bitmap()
 */
static inline uint32_t ecm_ppe_common_dummy_get_stats_bitmap(struct ecm_front_end_connection_instance *feci, ecm_db_obj_dir_t dir)
{
	return 0;
}

/*
 * ecm_ppe_common_dummy_set_stats_bitmap()
 */
static inline void ecm_ppe_common_dummy_set_stats_bitmap(struct ecm_front_end_connection_instance *feci, ecm_db_obj_dir_t dir, uint8_t bit)
{

}

bool ecm_ppe_ipv6_is_conn_limit_reached(void);
bool ecm_ppe_ipv4_is_conn_limit_reached(void);
