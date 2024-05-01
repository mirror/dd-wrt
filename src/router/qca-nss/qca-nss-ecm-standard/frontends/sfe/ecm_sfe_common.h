/*
 * Copyright (c) 2015, 2018, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 */

#include "ecm_sfe_common_public.h"

/*
 * Export the callback object for frontend usage.
 */
extern struct ecm_sfe_common_callbacks ecm_sfe_cb;

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

void ecm_sfe_common_fast_xmit_set(uint32_t *rule_flags, uint32_t *valid_flags, struct sfe_qdisc_rule *qdisc_rule, struct ecm_db_iface_instance *from_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX], struct ecm_db_iface_instance *to_ifaces[ECM_DB_IFACE_HEIRARCHY_MAX], int32_t from_interfaces_first, int32_t to_interfaces_first);

/*
 * ecm_sfe_common_dummy_get_stats_bitmap()
 */
static inline uint32_t ecm_sfe_common_dummy_get_stats_bitmap(struct ecm_front_end_connection_instance *feci, ecm_db_obj_dir_t dir)
{
	return 0;
}

/*
 * ecm_sfe_common_dummy_set_stats_bitmap()
 */
static inline void ecm_sfe_common_dummy_set_stats_bitmap(struct ecm_front_end_connection_instance *feci, ecm_db_obj_dir_t dir, uint8_t bit)
{

}

bool ecm_sfe_ipv4_is_conn_limit_reached(void);
bool ecm_sfe_ipv6_is_conn_limit_reached(void);
bool ecm_sfe_common_is_l2_iface_supported(ecm_db_iface_type_t ii_type, int cur_heirarchy_index, int first_heirarchy_index);

void ecm_sfe_common_init_fe_info(struct ecm_front_end_common_fe_info *info);
void ecm_sfe_common_update_rule(struct ecm_front_end_connection_instance *feci, enum ecm_rule_update_type type, void *arg);
void ecm_sfe_common_tuple_set(struct ecm_front_end_connection_instance *feci,
			      int32_t from_iface_id, int32_t to_iface_id,
			      struct ecm_sfe_common_tuple *tuple);
bool ecm_sfe_feature_check(struct sk_buff *skb, struct ecm_tracker_ip_header *ip_hdr, bool is_routed);
#ifdef ECM_BRIDGE_VLAN_FILTERING_ENABLE
void ecm_sfe_common_ipv4_vlan_filter_set(struct ecm_db_connection_instance *ci, struct sfe_ipv4_rule_create_msg *nircm);
void ecm_sfe_common_ipv6_vlan_filter_set(struct ecm_db_connection_instance *ci, struct sfe_ipv6_rule_create_msg *nircm);
#endif
#ifdef ECM_MHT_ENABLE
bool ecm_sfe_common_get_mht_port_id(struct ecm_front_end_connection_instance *feci,
				    struct ecm_db_iface_instance *from_sfe_iface,
				    struct ecm_db_iface_instance *to_sfe_iface,
				    u32 *valid_flags, struct sfe_mark_rule *mark_rule);
#endif
