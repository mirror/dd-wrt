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
#include <ppe_drv_port.h>

#include <net/xfrm.h>

#ifdef ECM_INTERFACE_VXLAN_ENABLE
#include <nss_ppe_vxlanmgr.h>
#endif

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
 * ecm_ppe_common_get_port_id_by_netdev_id()
 *	Gets the PPE port id from the netdevice interface index.
 */
static inline int32_t ecm_ppe_common_get_port_id_by_netdev_id(int32_t ifindex)
{
	struct net_device *dev;
	int32_t port_id;

	dev = dev_get_by_index(&init_net, ifindex);
	if (!dev) {
		DEBUG_WARN("unable to find net device with %d ifindex\n", ifindex);
		return PPE_DRV_PORT_ID_INVALID;
	}

	port_id = ppe_drv_port_num_from_dev(dev);

	dev_put(dev);

	return port_id;
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

bool ecm_ppe_feature_check(struct sk_buff *skb, struct ecm_tracker_ip_header *ip_hdr);
bool ecm_ppe_ipv6_is_conn_limit_reached(void);
bool ecm_ppe_ipv4_is_conn_limit_reached(void);

#ifdef ECM_INTERFACE_VXLAN_ENABLE
int ecm_ppe_ported_get_vxlan_ppe_dev_index(struct ecm_front_end_connection_instance *feci, struct ecm_db_iface_instance *ii,
											ecm_db_obj_dir_t dir, enum nss_ppe_vxlanmgr_vp_creation *vp_status);
#endif
