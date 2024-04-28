/*
 **************************************************************************
 * Copyright (c) 2015, 2018, The Linux Foundation.  All rights reserved.
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

#ifdef CONFIG_XFRM
/*
 * Which type of ipsec process traffic need.
 */
enum ecm_sfe_ipsec_state {
	ECM_SFE_IPSEC_STATE_NONE = 0,
	ECM_SFE_IPSEC_STATE_TO_ENCRYPT,
	ECM_SFE_IPSEC_STATE_WAS_DECRYPTED
};
#endif

/*
 * This macro converts ECM ip_addr_t to SFE IPv6 address
 */
#define ECM_IP_ADDR_TO_SFE_IPV6_ADDR(sfe6, ipaddrt) \
	{ \
		ecm_type_check_ae_ipv6(sfe6); \
		ecm_type_check_ecm_ip_addr(ipaddrt); \
		sfe6[0] = htonl(ipaddrt[3]); \
		sfe6[1] = htonl(ipaddrt[2]); \
		sfe6[2] = htonl(ipaddrt[1]); \
		sfe6[3] = htonl(ipaddrt[0]); \
	}

/*
 * This macro converts SFE IPv6 address to ECM ip_addr_t
 */
#define ECM_SFE_IPV6_ADDR_TO_IP_ADDR(ipaddrt, sfe6) \
	{ \
		ecm_type_check_ecm_ip_addr(ipaddrt); \
		ecm_type_check_ae_ipv6(sfe6); \
		ipaddrt[0] = ntohl(sfe6[3]); \
		ipaddrt[1] = ntohl(sfe6[2]); \
		ipaddrt[2] = ntohl(sfe6[1]); \
		ipaddrt[3] = ntohl(sfe6[0]); \
	}

/*
 * ecm_sfe_common_get_interface_number_by_dev()
 *	Returns the acceleration engine interface number based on the net_device object.
 */
static inline int32_t ecm_sfe_common_get_interface_number_by_dev(struct net_device *dev)
{
	/*
	 * sfe_interface_num for all IPsec tunnels will always be the one specific to acceleration engine.
	 */
	if (dev->type == ECM_ARPHRD_IPSEC_TUNNEL_TYPE) {
		return SFE_SPECIAL_INTERFACE_IPSEC;
	}

	return dev->ifindex;
}

/*
 * ecm_sfe_common_get_interface_number_by_dev_type()
 *	Returns the acceleration engine interface number based on the net_device object and type.
 */
static inline int32_t ecm_sfe_common_get_interface_number_by_dev_type(struct net_device *dev, uint32_t type)
{

	return ecm_sfe_common_get_interface_number_by_dev(dev);
}

/*
 * ecm_sfe_common_connection_regenerate()
 *	Re-generate a specific connection in SFE front end
 */
static inline void ecm_sfe_common_connection_regenerate(struct ecm_front_end_connection_instance *feci, struct ecm_db_connection_instance *ci)
{
	/*
	 * Flag the connection as needing re-generation.
	 * Re-generation occurs when we next see traffic OR an acceleration engine sync for this connection.
	 * Refer to front end protocol specific process() functions.
	 */
	ecm_db_connection_regeneration_needed(ci);

	/*
	 * If the connection is accelerated then force deceleration.
	 * Under normal circumstances deceleration would occur on the next sync received,
	 * however, there is a situation where a sync may not occur if, say, a cable has been pulled.
	 * The acceleration engine would see no further traffic to trigger sending a sync and so
	 * re-generation would not occur.
	 * The connection would stall and no-regeneration would happen leaving the connection in bad state.
	 * NOTE: We can just call decelerate() upon the front end - if its not accelerated this will have no effect.
	 */
	feci->decelerate(feci);
}

/*
 * ecm_sfe_common_get_interface_type()
 *	Gets the SFE interface type based on some features.
 *
 * NOTE: There is no type for SFE now. This function is implemented just to
 * use it for the feci callback.
 */
static inline int32_t ecm_sfe_common_get_interface_type(struct ecm_front_end_connection_instance *feci, struct net_device *dev)
{
	/*
	 * By default return 0. SFE driver doesn't have any interface type.
	 */
	return 0;
}
