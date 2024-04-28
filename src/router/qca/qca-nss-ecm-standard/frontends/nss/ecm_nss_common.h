/*
 **************************************************************************
 * Copyright (c) 2015, 2018-2019, The Linux Foundation.  All rights reserved.
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

/*
 * Some constants used with constructing NSS acceleration rules.
 * GGG TODO These should be provided by the NSS driver itself!
 */
#define ECM_NSS_CONNMGR_VLAN_ID_NOT_CONFIGURED 0xFFF
#define ECM_NSS_CONNMGR_VLAN_MARKING_NOT_CONFIGURED 0xFFFF

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
		if (dev->priv_flags & IFF_PPP_PPTP) {
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
