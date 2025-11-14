/*
 **************************************************************************
 * Copyright (c) 2015, 2018-2021, The Linux Foundation.  All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
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

#ifdef ECM_INTERFACE_MAP_T_ENABLE
#include <nat46-core.h>
#endif

#ifdef ECM_INTERFACE_IPSEC_ENABLE
#ifdef ECM_INTERFACE_IPSEC_GLUE_LAYER_SUPPORT_ENABLE
#include "nss_ipsec_cmn.h"
#else
#include "nss_ipsec.h"
#endif
#endif

#ifdef ECM_INTERFACE_VXLAN_ENABLE
#include <net/vxlan.h>
#endif

#include <net/xfrm.h>

/*
 * This macro converts ECM ip_addr_t to NSS IPv6 address
 */
#define ECM_IP_ADDR_TO_NSS_IPV6_ADDR(nss6, ipaddrt) \
	{ \
		ecm_type_check_ae_ipv6(nss6); \
		ecm_type_check_ecm_ip_addr(ipaddrt); \
		nss6[0] = ipaddrt[3]; \
		nss6[1] = ipaddrt[2]; \
		nss6[2] = ipaddrt[1]; \
		nss6[3] = ipaddrt[0]; \
	}

/*
 * This macro converts NSS IPv6 address to ECM ip_addr_t
 */
#define ECM_NSS_IPV6_ADDR_TO_IP_ADDR(ipaddrt, nss6) \
	{ \
		ecm_type_check_ecm_ip_addr(ipaddrt); \
		ecm_type_check_ae_ipv6(nss6); \
		ipaddrt[0] = nss6[3]; \
		ipaddrt[1] = nss6[2]; \
		ipaddrt[2] = nss6[1]; \
		ipaddrt[3] = nss6[0]; \
	}

/*
 * ecm_nss_common_get_interface_number_by_dev()
 *	Returns the acceleration engine interface number based on the net_device object.
 */
static inline int32_t ecm_nss_common_get_interface_number_by_dev(struct net_device *dev)
{
	/*
	 * nss_interface_num for all IPsec tunnels will always be the one specific to acceleration engine.
	 */
	if (dev->type == ECM_ARPHRD_IPSEC_TUNNEL_TYPE) {
		return NSS_IPSEC_CMN_INTERFACE;
	}

#ifdef ECM_INTERFACE_RAWIP_ENABLE
	if (dev->type == ARPHRD_RAWIP) {
		return nss_rmnet_rx_get_ifnum(dev);
	}
#endif

	return nss_cmn_get_interface_number_by_dev(dev);
}

/*
 * ecm_nss_common_get_interface_number_by_dev_type()
 *	Returns the acceleration engine interface number based on the net_device object and type.
 */
static inline int32_t ecm_nss_common_get_interface_number_by_dev_type(struct net_device *dev, uint32_t type)
{
	/*
	 * nss_interface_num for all IPsec tunnels will always be the one specific to acceleration engine.
	 */
	if ((dev->type == ECM_ARPHRD_IPSEC_TUNNEL_TYPE) && !type) {
		return NSS_IPSEC_CMN_INTERFACE;
	}

#ifdef ECM_INTERFACE_VXLAN_ENABLE
	/*
	 * VxLAN outer packets are send to VxLAN base node.
	 */
	if (netif_is_vxlan(dev)) {
		if (!type) {
			return NSS_VXLAN_INTERFACE;
		}
		type = NSS_DYNAMIC_INTERFACE_TYPE_VXLAN_INNER;
	}
#endif

	return nss_cmn_get_interface_number_by_dev_and_type(dev, type);
}

/*
 * ecm_nss_common_connection_regenerate()
 *	Re-generate a specific connection in NSS front end
 */
static inline void ecm_nss_common_connection_regenerate(struct ecm_front_end_connection_instance *feci, struct ecm_db_connection_instance *ci)
{
	/*
	 * Flag the connection as needing re-generation.
	 * Re-generation occurs when we next see traffic OR an acceleration engine sync for this connection.
	 * Refer to front end protocol specific process() functions.
	 */
	ecm_db_connection_regeneration_needed(ci);
}

/*
 * ecm_nss_common_get_interface_type()
 *	Gets the NSS interface type based on some features.
 *
 * TODO: For now the feature is the IP version and the net device type. It can be changed
 * in the future for other needs.
 */
static inline int32_t ecm_nss_common_get_interface_type(struct ecm_front_end_connection_instance *feci, struct net_device *dev)
{
	switch (dev->type) {
	case ARPHRD_SIT:
#ifdef ECM_INTERFACE_SIT_ENABLE
		if (feci->ip_version == 4) {
			return NSS_DYNAMIC_INTERFACE_TYPE_TUN6RD_OUTER;
		}

		if (feci->ip_version == 6) {
			return NSS_DYNAMIC_INTERFACE_TYPE_TUN6RD_INNER;
		}
#endif
		break;

#ifdef ECM_INTERFACE_TUNIPIP6_ENABLE
	case ARPHRD_TUNNEL6:
		if (feci->ip_version == 4) {
			return NSS_DYNAMIC_INTERFACE_TYPE_TUNIPIP6_INNER;
		}

		if (feci->ip_version == 6) {
			return NSS_DYNAMIC_INTERFACE_TYPE_TUNIPIP6_OUTER;
		}
#endif
		break;

#ifdef ECM_INTERFACE_GRE_TAP_ENABLE
	case ARPHRD_ETHER:
		/*
		 * If device is not GRETAP then return NONE.
		 */
		if (!(dev->priv_flags_ext & (IFF_EXT_GRE_V4_TAP | IFF_EXT_GRE_V6_TAP))) {
			break;
		}
#if __has_attribute(__fallthrough__)
		__attribute__((__fallthrough__));
#endif
#endif
#ifdef ECM_INTERFACE_GRE_TUN_ENABLE
	case ARPHRD_IPGRE:
	case ARPHRD_IP6GRE:
#endif
#if defined(ECM_INTERFACE_GRE_TAP_ENABLE) || defined(ECM_INTERFACE_GRE_TUN_ENABLE)
		if (feci->protocol == IPPROTO_GRE) {
			return NSS_DYNAMIC_INTERFACE_TYPE_GRE_OUTER;
		}

		return NSS_DYNAMIC_INTERFACE_TYPE_GRE_INNER;
#endif
	case ARPHRD_NONE:
#ifdef ECM_INTERFACE_MAP_T_ENABLE
		if (is_map_t_dev(dev)) {
			if (feci->ip_version == 4) {
				return NSS_DYNAMIC_INTERFACE_TYPE_MAP_T_INNER;
			}

			if (feci->ip_version == 6) {
				return NSS_DYNAMIC_INTERFACE_TYPE_MAP_T_OUTER;
			}
		}
#endif
		break;
	case ARPHRD_PPP:
#ifdef ECM_INTERFACE_PPTP_ENABLE
		if (dev->priv_flags_ext & IFF_EXT_PPP_PPTP) {
			if (feci->protocol == IPPROTO_GRE) {
				return NSS_DYNAMIC_INTERFACE_TYPE_PPTP_OUTER;
			}

			return NSS_DYNAMIC_INTERFACE_TYPE_PPTP_INNER;
		}
#endif
		break;
	default:
		break;
	}

	/*
	 * By default.
	 */
	return NSS_DYNAMIC_INTERFACE_TYPE_NONE;
}

#ifdef ECM_INTERFACE_IPSEC_ENABLE
/*
 * ecm_nss_common_ipsec_get_ifnum()
 *     Get ipsec specific interface number appended with coreid
 */
static inline int32_t ecm_nss_common_ipsec_get_ifnum(int32_t ifnum)
{
#ifdef ECM_INTERFACE_IPSEC_GLUE_LAYER_SUPPORT_ENABLE
	return nss_ipsec_cmn_get_ifnum_with_coreid(ifnum);
#else
	return nss_ipsec_get_ifnum(ifnum);
#endif
}
#endif

#if defined(CONFIG_NET_CLS_ACT) && defined(ECM_CLASSIFIER_DSCP_IGS)
/*
 * ecm_nss_common_igs_acceleration_is_allowed()
 *	Return true, if flow acceleration is allowed for an IGS interface.
 */
static inline bool ecm_nss_common_igs_acceleration_is_allowed(struct ecm_front_end_connection_instance *feci,
		struct sk_buff *skb)
{
	struct net_device *to_dev;
	struct ecm_db_iface_instance *to_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX];
	enum ip_conntrack_info ctinfo;
	int to_ifaces_first;
	uint32_t list_index;
	bool do_accel = true;

	/*
	 * Get the interface lists of the connection and check if any interface in the list
	 * has ingress qdisc attached to it.
	 */
	to_ifaces_first = ecm_db_connection_interfaces_get_and_ref(feci->ci, to_ifaces, ECM_DB_OBJ_DIR_TO);
	if (to_ifaces_first == ECM_DB_IFACE_HEIRARCHY_MAX) {
		DEBUG_WARN("%px: Accel attempt failed - no interfaces in to_interfaces list!\n", feci);
		return false;
	}

	/*
	 * Reject the flow acceleration if the egress device has ingress qdisc
	 * attached to it. At these interfaces, the acceleration is only allowed
	 * when any packet is recieved over them (so that we can able to get the
	 * correct ingress qostag values from it and fill them in the acceleration rules).
	 */
	for (list_index = to_ifaces_first; list_index < ECM_DB_IFACE_HEIRARCHY_MAX; list_index++) {
		struct ecm_db_iface_instance *ii;

		ii = to_ifaces[list_index];
		to_dev = dev_get_by_index(&init_net, ecm_db_iface_interface_identifier_get(ii));
		if (unlikely(!to_dev)) {
			DEBUG_TRACE("%px: No valid device found for %d index.\n",
					feci, ecm_db_iface_interface_identifier_get(ii));
			continue;
		}

		/*
		 * Check whether ingress qdisc is attached to the egress device or not.
		 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0))
		if (likely(!(to_dev->ingress_cl_list))) {
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0))
		if (likely(!(to_dev->miniq_ingress))) {
#else
		if (likely(!(to_dev->tcx_ingress))) {
#endif
			dev_put(to_dev);
			continue;
		}

		/*
		 * Reject the connection if both side packets are not yet seen.
		 */
		nf_ct_get(skb, &ctinfo);
		if ((ctinfo != IP_CT_ESTABLISHED) &&
				(ctinfo != IP_CT_ESTABLISHED_REPLY)) {
			DEBUG_INFO("%px: New flow at ingress device, "
					"rejecting the acceleration.\n", feci);

			/*
			 * Deny the acceleration as both side packets are not yet seen.
			 */
			do_accel = false;
		}
		dev_put(to_dev);
		break;
	}

	/*
	 * Release the resources.
	 */
	ecm_db_connection_interfaces_deref(to_ifaces, to_ifaces_first);
	return do_accel;
}
#endif

#ifdef ECM_CLASSIFIER_PCC_ENABLE
/*
 * ecm_nss_common_fill_mirror_info()
 *	Fill Mirror information.
 */
static inline bool ecm_nss_common_fill_mirror_info(struct ecm_classifier_process_response *pr,
		nss_if_num_t *flow_mirror_ifnum, nss_if_num_t *return_mirror_ifnum)
{
	struct net_device *mirror_dev;

	/*
	 * Initialize both mirror interfaces to invalid.
	 */
	*flow_mirror_ifnum = -1;
	*return_mirror_ifnum = -1;

	if (pr->flow_mirror_ifindex > 0) {
		mirror_dev = dev_get_by_index(&init_net, pr->flow_mirror_ifindex);
		if (!mirror_dev) {
			DEBUG_ERROR("Invalid mirror flow index number: %d\n", pr->flow_mirror_ifindex);
			return false;
		}

		*flow_mirror_ifnum = ecm_nss_common_get_interface_number_by_dev_type(mirror_dev,
				NSS_DYNAMIC_INTERFACE_TYPE_MIRROR);
		if (*flow_mirror_ifnum < 0) {
			DEBUG_ERROR("Invalid mirror interface: %s\n", mirror_dev->name);
			dev_put(mirror_dev);
			return false;
		}
		dev_put(mirror_dev);
	}

	if (pr->return_mirror_ifindex > 0) {
		mirror_dev = dev_get_by_index(&init_net, pr->return_mirror_ifindex);
		if (!mirror_dev) {
			DEBUG_ERROR("Invalid mirror return index number: %d\n", pr->return_mirror_ifindex);
			return false;
		}

		*return_mirror_ifnum = ecm_nss_common_get_interface_number_by_dev_type(mirror_dev,
				NSS_DYNAMIC_INTERFACE_TYPE_MIRROR);
		if (*return_mirror_ifnum < 0) {
			DEBUG_ERROR("Invalid mirror interface: %s\n", mirror_dev->name);
			dev_put(mirror_dev);
			return false;
		}
		dev_put(mirror_dev);
	}

	return true;
}
#endif

bool ecm_nss_ipv6_is_conn_limit_reached(void);
bool ecm_nss_ipv4_is_conn_limit_reached(void);
bool ecm_nss_feature_check(struct sk_buff *skb, struct ecm_tracker_ip_header *ip_hdr);

/*
 * ecm_nss_common_dummy_get_stats_bitmap()
 */
static inline uint32_t ecm_nss_common_dummy_get_stats_bitmap(struct ecm_front_end_connection_instance *feci, ecm_db_obj_dir_t dir)
{
	return 0;
}

/*
 * ecm_nss_common_dummy_set_stats_bitmap()
 */
static inline void ecm_nss_common_dummy_set_stats_bitmap(struct ecm_front_end_connection_instance *feci, ecm_db_obj_dir_t dir, uint8_t bit)
{

}
