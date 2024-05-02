/*
 **************************************************************************
 * Copyright (c) 2015, 2016, 2020-2021, The Linux Foundation. All rights reserved.
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
#include <linux/module.h>
#include <linux/of.h>
#include <linux/debugfs.h>
#include <linux/inet.h>
#include <linux/etherdevice.h>
#include <linux/inetdevice.h>
#include <linux/sysctl.h>
#include <net/netfilter/nf_conntrack.h>
#ifdef ECM_CLASSIFIER_DSCP_ENABLE
#include <linux/netfilter/xt_dscp.h>
#include <net/netfilter/nf_conntrack_dscpremark_ext.h>
#endif
#include <net/ip.h>
#include <net/ipv6.h>
#include <net/addrconf.h>
#include <net/gre.h>
#include <net/xfrm.h>
#if (LINUX_VERSION_CODE > KERNEL_VERSION(6, 6, 0))
#include <net/tcx.h>
#endif
#include <linux/hashtable.h>
#include <net/sch_generic.h>
#ifdef ECM_FRONT_END_PPE_ENABLE
#include <ppe_drv.h>
#endif

/*
 * Debug output levels
 * 0 = OFF
 * 1 = ASSERTS / ERRORS
 * 2 = 1 + WARN
 * 3 = 2 + INFO
 * 4 = 3 + TRACE
 */
#define DEBUG_LEVEL ECM_FRONT_END_COMMON_DEBUG_LEVEL

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
#include "ecm_front_end_common.h"
#include "ecm_interface.h"
#include "ecm_ae_classifier.h"

#ifdef ECM_FRONT_END_NSS_ENABLE
#include <nss_api_if.h>
#include "ecm_nss_ipv4.h"
#include "ecm_nss_ipv6.h"
#include "ecm_nss_common.h"
#include "ecm_nss_ported_ipv4.h"
#include "ecm_nss_ported_ipv6.h"
#ifdef ECM_NON_PORTED_SUPPORT_ENABLE
#include "ecm_nss_non_ported_ipv4.h"
#include "ecm_nss_non_ported_ipv6.h"
#endif
#endif

#ifdef ECM_FRONT_END_SFE_ENABLE
#include <sfe_api.h>
#include "ecm_sfe_ipv4.h"
#include "ecm_sfe_ipv6.h"
#include "ecm_sfe_common.h"
#include "ecm_sfe_ported_ipv4.h"
#include "ecm_sfe_ported_ipv6.h"
#ifdef ECM_NON_PORTED_SUPPORT_ENABLE
#include "ecm_sfe_non_ported_ipv4.h"
#include "ecm_sfe_non_ported_ipv6.h"
#endif
#endif

#ifdef ECM_FRONT_END_PPE_ENABLE
#include "ecm_ppe_ipv4.h"
#include "ecm_ppe_ipv6.h"
#include "ecm_ppe_common.h"
#include "ecm_ppe_ported_ipv4.h"
#include "ecm_ppe_ported_ipv6.h"
#ifdef ECM_NON_PORTED_SUPPORT_ENABLE
#include "ecm_ppe_non_ported_ipv4.h"
#include "ecm_ppe_non_ported_ipv6.h"
#endif
#endif

#ifdef ECM_FRONT_END_FSE_ENABLE
/*
 * Callback object for ECM frontend interaction with wlan driver to add/delete FSE rules.
 */
struct ecm_front_end_fse_callbacks *ecm_fe_fse_cb = NULL;
#endif

/*
 * Sysctl table header
 */
static struct ctl_table_header *ecm_front_end_ctl_tbl_hdr;

/*
 * Flag to limit the number of DB connections at any point to the maximum number
 * that can be accelerated by NSS. This may need to be enabled for low memory
 * platforms to control memory allocated by ECM databases.
 */
unsigned int ecm_front_end_conn_limit = 0;
#ifdef ECM_FRONT_END_PPE_ENABLE
unsigned int ecm_front_end_ppe_fse_enable = 1;
#endif

#define ECM_FRONT_END_DENIED_PORTS_HASH_BITS 6
#define ECM_FRONT_END_DENIED_PORTS_HTABLE_SIZE (1 << ECM_FRONT_END_DENIED_PORTS_HASH_BITS)

/*
 * Denied acceleration port hash tables and port counts in the tables.
 */
static DEFINE_HASHTABLE(ecm_front_end_udp_denied_ports, ECM_FRONT_END_DENIED_PORTS_HASH_BITS);
static DEFINE_HASHTABLE(ecm_front_end_tcp_denied_ports, ECM_FRONT_END_DENIED_PORTS_HASH_BITS);
static atomic_t ecm_udp_denied_port_count;
static atomic_t ecm_tcp_denied_port_count;

/*
 * Hash node in the denied ports hash list.
 */
struct ecm_denied_port_node {
	struct hlist_node hnode;
	uint32_t port;
};

/*
 * Predefined frontend and feature support map.
 */
uint32_t ecm_fe_feature_list[ECM_FRONT_END_TYPE_MAX] = {
	/* Auto: This type will never be selected */
	0,

	/* NSS type */
	ECM_FE_FEATURE_NSS | ECM_FE_FEATURE_NON_PORTED | ECM_FE_FEATURE_BRIDGE |
	ECM_FE_FEATURE_MULTICAST | ECM_FE_FEATURE_BONDING | ECM_FE_FEATURE_IGS |
	ECM_FE_FEATURE_SRC_IF_CHECK | ECM_FE_FEATURE_CONN_LIMIT |
	ECM_FE_FEATURE_DSCP_ACTION | ECM_FE_FEATURE_XFRM | ECM_FE_FEATURE_OVS_BRIDGE |
	ECM_FE_FEATURE_OVS_VLAN,

	/* SFE type */
	ECM_FE_FEATURE_SFE | ECM_FE_FEATURE_NON_PORTED | ECM_FE_FEATURE_CONN_LIMIT |
	ECM_FE_FEATURE_OVS_BRIDGE | ECM_FE_FEATURE_OVS_VLAN | ECM_FE_FEATURE_BRIDGE |
	ECM_FE_FEATURE_BONDING | ECM_FE_FEATURE_SRC_IF_CHECK | ECM_FE_FEATURE_MULTICAST,

	/* PPE */
#ifndef ECM_PPE_SOURCE_INTERFACE_CHECK_ENABLE
	ECM_FE_FEATURE_PPE | ECM_FE_FEATURE_BRIDGE | ECM_FE_FEATURE_NON_PORTED |
	ECM_FE_FEATURE_CONN_LIMIT,
#else
	ECM_FE_FEATURE_PPE | ECM_FE_FEATURE_BRIDGE | ECM_FE_FEATURE_NON_PORTED |
	ECM_FE_FEATURE_CONN_LIMIT | ECM_FE_FEATURE_SRC_IF_CHECK,
#endif

	/* NSS_SFE type */
	ECM_FE_FEATURE_NSS | ECM_FE_FEATURE_SFE | ECM_FE_FEATURE_NON_PORTED | ECM_FE_FEATURE_BRIDGE |
	ECM_FE_FEATURE_MULTICAST | ECM_FE_FEATURE_BONDING | ECM_FE_FEATURE_IGS |
	ECM_FE_FEATURE_SRC_IF_CHECK | ECM_FE_FEATURE_CONN_LIMIT |
	ECM_FE_FEATURE_DSCP_ACTION | ECM_FE_FEATURE_XFRM | ECM_FE_FEATURE_OVS_BRIDGE |
	ECM_FE_FEATURE_OVS_VLAN,

	/*
	 * PPE_SFE type
	 * TODO: Handle the features which are not supported by PPE, if the AE is selected as PPE.
	 */
#ifndef ECM_PPE_SOURCE_INTERFACE_CHECK_ENABLE
	ECM_FE_FEATURE_SFE | ECM_FE_FEATURE_NON_PORTED | ECM_FE_FEATURE_CONN_LIMIT |
	ECM_FE_FEATURE_OVS_BRIDGE | ECM_FE_FEATURE_OVS_VLAN | ECM_FE_FEATURE_BRIDGE |
	ECM_FE_FEATURE_BONDING | ECM_FE_FEATURE_PPE | ECM_FE_FEATURE_MULTICAST,
#else
	ECM_FE_FEATURE_SFE | ECM_FE_FEATURE_NON_PORTED | ECM_FE_FEATURE_CONN_LIMIT |
	ECM_FE_FEATURE_OVS_BRIDGE | ECM_FE_FEATURE_OVS_VLAN | ECM_FE_FEATURE_BRIDGE |
	ECM_FE_FEATURE_BONDING | ECM_FE_FEATURE_PPE | ECM_FE_FEATURE_MULTICAST | ECM_FE_FEATURE_SRC_IF_CHECK,
#endif
};

struct ecm_ae_precedence ae_precedence[ECM_AE_PRECEDENCE_MAX + 1];

/*
 * ecm_front_end_set_ae_alloc_methods()
 *	Set the AE front-end alloc methods at the precedence array index.
 */
void ecm_front_end_set_ae_alloc_methods(struct ecm_ae_precedence *precedence)
{
	switch (precedence->ae_type) {
#ifdef ECM_FRONT_END_SFE_ENABLE
	case ECM_FRONT_END_ENGINE_SFE:
		precedence->ported_ipv4_alloc = ecm_sfe_ported_ipv4_connection_instance_alloc;
		precedence->ported_ipv6_alloc = ecm_sfe_ported_ipv6_connection_instance_alloc;
#ifdef ECM_NON_PORTED_SUPPORT_ENABLE
		precedence->non_ported_ipv4_alloc = ecm_sfe_non_ported_ipv4_connection_instance_alloc;
		precedence->non_ported_ipv6_alloc = ecm_sfe_non_ported_ipv6_connection_instance_alloc;
#endif
		break;
#endif
#ifdef ECM_FRONT_END_NSS_ENABLE
	case ECM_FRONT_END_ENGINE_NSS:
		precedence->ported_ipv4_alloc = ecm_nss_ported_ipv4_connection_instance_alloc;
		precedence->ported_ipv6_alloc = ecm_nss_ported_ipv6_connection_instance_alloc;
#ifdef ECM_NON_PORTED_SUPPORT_ENABLE
		precedence->non_ported_ipv4_alloc = ecm_nss_non_ported_ipv4_connection_instance_alloc;
		precedence->non_ported_ipv6_alloc = ecm_nss_non_ported_ipv6_connection_instance_alloc;
#endif
		break;
#endif
#ifdef ECM_FRONT_END_PPE_ENABLE
	case ECM_FRONT_END_ENGINE_PPE:
		precedence->ported_ipv4_alloc = ecm_ppe_ported_ipv4_connection_instance_alloc;
		precedence->ported_ipv6_alloc = ecm_ppe_ported_ipv6_connection_instance_alloc;
#ifdef ECM_NON_PORTED_SUPPORT_ENABLE
		precedence->non_ported_ipv4_alloc = ecm_ppe_non_ported_ipv4_connection_instance_alloc;
		precedence->non_ported_ipv6_alloc = ecm_ppe_non_ported_ipv6_connection_instance_alloc;
#endif
		break;
#endif
	default:
		DEBUG_WARN("precedence->ae_type: %d is not supported yet", precedence->ae_type);
	}
}

/*
 * ecm_front_end_is_feature_supported()
 *	Checks if the given feature is supported in the selected frontend.
 */
bool ecm_front_end_is_feature_supported(enum ecm_fe_feature feature)
{
	enum ecm_front_end_type type = ecm_front_end_type_get();

	return !!(ecm_fe_feature_list[type] & feature);
}

/*
 * ecm_front_end_is_xfrm_flow()
 *	Returns true if the flow is an xfrm flow and identifies if the flow is xfrm inner.
 */
bool ecm_front_end_is_xfrm_flow(struct sk_buff *skb, struct ecm_tracker_ip_header *ip_hdr, bool *inner)
{
#ifdef CONFIG_XFRM
	struct dst_entry *dst;
	struct net *net;

	net = dev_net(skb->dev);
	if (likely(!net->xfrm.policy_count[XFRM_POLICY_OUT])) {
		return false;
	}

	/*
	 * Packet seen after output transformation. We use the IPCB(skb) to check
	 * for this condition. No custom code should mangle the IPCB: skb->cb area,
	 * while the packet is traversing through the INET layer.
	 */
	if (ip_hdr->is_v4) {
		if ((IPCB(skb)->flags & IPSKB_XFRM_TRANSFORMED)) {
			DEBUG_TRACE("%px: Packet has undergone xfrm transformation\n", skb);
			*inner = false;
			return true;
		}
	} else if (IP6CB(skb)->flags & IP6SKB_XFRM_TRANSFORMED) {
		DEBUG_TRACE("%px: Packet has undergone xfrm transformation\n", skb);
		*inner = false;
		return true;
	}

	if (ip_hdr->protocol == IPPROTO_ESP) {
		DEBUG_TRACE("%px: ESP Passthrough packet\n", skb);
		return false;
	}

	/*
	 * skb's sp is set for decapsulated packet
	 */
	if (secpath_exists(skb)) {
		DEBUG_TRACE("%px: Packet has undergone xfrm decapsulation((%d)\n", skb, ip_hdr->protocol);
		*inner = true;
		return true;
	}

	/*
	 * dst->xfrm is valid for lan to wan plain packet
	 */
	dst = skb_dst(skb);
	if (dst && dst->xfrm) {
		DEBUG_TRACE("%px: Plain text packet destined for xfrm(%d)\n", skb, ip_hdr->protocol);
		*inner = true;
		return true;
	}
#endif

	return false;
}

/*
 * ecm_front_end_feature_check()
 *	Check some specific features for front end acceleration
 */
bool ecm_front_end_feature_check(struct sk_buff *skb, struct ecm_tracker_ip_header *ip_hdr)
{
	bool inner = 0;

	if (ecm_front_end_is_xfrm_flow(skb, ip_hdr, &inner)) {
#ifdef ECM_XFRM_ENABLE
		struct net_device *ipsec_dev;
		int32_t interface_type;

		/*
		 * Check if the transformation for this flow
		 * is done by AE. If yes, then try to accelerate.
		 */
		ipsec_dev = ecm_interface_get_and_hold_ipsec_tun_netdev(NULL, skb, &interface_type);
		if (!ipsec_dev) {
			DEBUG_TRACE("%px xfrm flow not managed by NSS; skip it\n", skb);
			return false;
		}
		dev_put(ipsec_dev);
#else
		DEBUG_TRACE("%px xfrm flow, but accel is disabled; skip it\n", skb);
		return false;
#endif
	}

	return true;
}

/*
 * ecm_front_end_common_feature_check()
 *	Check if the selected AE supports the flow.
 */
bool ecm_front_end_common_feature_check(enum ecm_front_end_engine ae_type,
					struct sk_buff *skb,
					struct ecm_tracker_ip_header *iph,
					bool is_routed)
{
	switch (ae_type) {
#ifdef ECM_FRONT_END_SFE_ENABLE
	case ECM_FRONT_END_ENGINE_SFE:
		if (!ecm_sfe_feature_check(skb, iph, is_routed)) {
			return false;
		}

		return true;
#endif
#ifdef ECM_FRONT_END_NSS_ENABLE
	case ECM_FRONT_END_ENGINE_NSS:
		if (!ecm_nss_feature_check(skb, iph)) {
			return false;
		}

		return true;
#endif
#ifdef ECM_FRONT_END_PPE_ENABLE
	case ECM_FRONT_END_ENGINE_PPE:
		if (!ecm_ppe_feature_check(skb, iph)) {
			return false;
		}

		return true;
#endif
	default:
		DEBUG_WARN("ae_type: %d is not supported yet, feature check failed", ae_type);
	}

	return false;
}

#ifdef ECM_INTERFACE_BOND_ENABLE
/*
 * ecm_front_end_bond_notifier_stop()
 */
void ecm_front_end_bond_notifier_stop(int num)
{
	if (ecm_front_end_is_feature_supported(ECM_FE_FEATURE_BONDING)) {
		ecm_bond_notifier_stop(num);
	}
}

/*
 * ecm_front_end_bond_notifier_init()
 */
int ecm_front_end_bond_notifier_init(struct dentry *dentry)
{
	if (ecm_front_end_is_feature_supported(ECM_FE_FEATURE_BONDING)) {
		return ecm_bond_notifier_init(dentry);
	}

	return 0;
}

/*
 * ecm_front_end_bond_notifier_exit()
 */
void ecm_front_end_bond_notifier_exit(void)
{
	if (ecm_front_end_is_feature_supported(ECM_FE_FEATURE_BONDING)) {
		ecm_bond_notifier_exit();
	}
}
#endif

#ifdef ECM_STATE_OUTPUT_ENABLE
/*
 * ecm_front_end_common_connection_state_get()
 *	Return state of the front end instance.
 */
int ecm_front_end_common_connection_state_get(struct ecm_front_end_connection_instance *feci,
					      struct ecm_state_file_instance *sfi,
					      char *conn_type)
{
	int result;
	bool can_accel;
	ecm_front_end_acceleration_mode_t accel_mode;
	struct ecm_front_end_connection_mode_stats stats;
	char *ae_selection_done = "precedence-array";

	spin_lock_bh(&feci->lock);
	can_accel = feci->can_accel;
	accel_mode = feci->accel_mode;

	if (feci->fe_info.front_end_flags & ECM_FRONT_END_ENGINE_FLAG_SAWF_CHANGE_AE_TYPE_DONE) {
		ae_selection_done = "sawf-classifier";
	} else if (feci->fe_info.front_end_flags & ECM_FRONT_END_ENGINE_FLAG_AE_SELECTOR_ENABLED) {
		ae_selection_done = "ae-selector";
	}

	memcpy(&stats, &feci->stats, sizeof(struct ecm_front_end_connection_mode_stats));
	spin_unlock_bh(&feci->lock);

	if ((result = ecm_state_prefix_add(sfi, conn_type))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "can_accel", "%d", can_accel))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "accel_mode", "%d", accel_mode))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "decelerate_pending", "%d", stats.decelerate_pending))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "flush_happened_total", "%d", stats.flush_happened_total))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "no_action_seen_total", "%d", stats.no_action_seen_total))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "no_action_seen", "%d", stats.no_action_seen))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "no_action_seen_limit", "%d", stats.no_action_seen_limit))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "driver_fail_total", "%d", stats.driver_fail_total))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "driver_fail", "%d", stats.driver_fail))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "driver_fail_limit", "%d", stats.driver_fail_limit))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "ae_nack_total", "%d", stats.ae_nack_total))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "ae_nack", "%d", stats.ae_nack))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "ae_nack_limit", "%d", stats.ae_nack_limit))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "slow_path_packets", "%llu", stats.slow_path_packets))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "ae_selection_done", "%s", ae_selection_done))) {
		return result;
	}
	return ecm_state_prefix_remove(sfi);
}
#endif

/*
 * ecm_front_end_l2tp_proto_is_accel_allowed
 *	Allow L2TPV3 acceleration for l2tpv3 tunnel on ethernet interfaces
 */
bool ecm_front_end_l2tp_proto_is_accel_allowed(struct net_device *indev, struct net_device *outdev)
{
#ifdef ECM_INTERFACE_L2TPV3_ENABLE
	/*
	 * L2TPv3 acceleration is allowed for l2tp-ethX interfaces
	 */
	if ((indev->priv_flags_ext & IFF_EXT_ETH_L2TPV3)
		|| (outdev->priv_flags_ext & IFF_EXT_ETH_L2TPV3)) {
		return true;
	}
#endif
	return false;
}

/*
 * ecm_front_end_gre_proto_is_accel_allowed()
 *	Handle the following GRE cases:
 *
 * 1. PPTP locally terminated - allow acceleration
 * 2. PPTP pass through - do not allow acceleration
 * 3. GRE V4 or V6 TAP - allow acceleration
 * 4. GRE V4 or V6 TUN - allow acceleration
 * 5. NVGRE locally terminated - do not allow acceleration
 * 6. GRE pass through NAT - do not allow acceleration
 * 7. NVGRE pass through - do not allow acceleration
 * 8. GRE pass through non NAT - allow acceleration
 */
bool ecm_front_end_gre_proto_is_accel_allowed(struct net_device *indev,
							     struct net_device *outdev,
							     struct sk_buff *skb,
							     struct nf_conntrack_tuple *orig_tuple,
							     struct nf_conntrack_tuple *reply_tuple,
							     int ip_version, uint16_t offset)
{
	struct net_device *dev;
	struct gre_base_hdr *greh;

	skb_pull(skb, offset);
	greh = (struct gre_base_hdr *)(skb->data);
	skb_push(skb, offset);

	if ((greh->flags & GRE_VERSION) == ECM_GRE_VERSION_1) {
		/*
		 * Case 1: PPTP locally terminated
		 */
		if (ecm_interface_is_pptp(skb, outdev)) {
			DEBUG_TRACE("%px: PPTP GRE locally terminated - allow acceleration\n", skb);
			return true;
		}

		/*
		 * Case 2: PPTP pass through
		 */
		DEBUG_TRACE("%px: PPTP GRE pass through - do not allow acceleration\n", skb);
		return false;
	}

	if ((greh->flags & GRE_VERSION) != ECM_GRE_VERSION_0) {
		DEBUG_WARN("%px: Unknown GRE version - do not allow acceleration\n", skb);
		return false;
	}

	/*
	 * Case 3: GRE V4 or V6 TAP
	 */
	if ((indev->priv_flags_ext & (IFF_EXT_GRE_V4_TAP | IFF_EXT_GRE_V6_TAP))
		|| (outdev->priv_flags_ext & (IFF_EXT_GRE_V4_TAP | IFF_EXT_GRE_V6_TAP))) {
#ifdef ECM_INTERFACE_GRE_TAP_ENABLE
		DEBUG_TRACE("%px: GRE IPv%d TAP flow - allow acceleration\n", skb, ip_version);
		return true;
#else
		DEBUG_TRACE("%px: GRE IPv%d TAP feature is disabled - do not allow acceleration\n", skb, ip_version);
		return false;
#endif
	}

	/*
	 * Case 4: GRE V4 or V6 TUN
	 */
	if ((indev->type == ARPHRD_IPGRE) || (outdev->type == ARPHRD_IPGRE)
		|| (indev->type == ARPHRD_IP6GRE) || (outdev->type == ARPHRD_IP6GRE)) {
#ifdef ECM_INTERFACE_GRE_TUN_ENABLE
		DEBUG_TRACE("%px: GRE IPv%d TUN flow - allow acceleration\n", skb, ip_version);
		return true;
#else
		DEBUG_TRACE("%px: GRE IPv%d TUN feature is disabled - do not allow acceleration\n", skb, ip_version);
		return false;
#endif
	}

	/*
	 * Case 5: NVGRE locally terminated
	 *
	 * Check both source and dest interface.
	 * If either is locally terminated, we cannot accelerate.
	 */
	if (ip_version == 4) {
		dev = ip_dev_find(&init_net, orig_tuple->src.u3.ip);
		if (dev) {
			/*
			 * Source IP address is local
			 */
			dev_put(dev);
			DEBUG_TRACE("%px: NVGRE locally terminated (src) - do not allow acceleration\n", skb);
			return false;
		}

		dev = ip_dev_find(&init_net, orig_tuple->dst.u3.ip);
		if (dev) {
			/*
			 * Destination IP address is local
			 */
			dev_put(dev);
			DEBUG_TRACE("%px: NVGRE locally terminated (dest) - do not allow acceleration\n", skb);
			return false;
		}

		/*
		 * Case 6: GRE pass through NAT
		 *
		 */
		if (orig_tuple->src.u3.ip != reply_tuple->dst.u3.ip || orig_tuple->dst.u3.ip != reply_tuple->src.u3.ip) {
			DEBUG_TRACE("%px: GRE IPv%d pass through NAT - do not allow acceleration\n", skb, ip_version);
			return false;
		}
	} else {
#ifdef ECM_IPV6_ENABLE
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 9, 0))
		dev = ipv6_dev_find(&init_net, &(orig_tuple->src.u3.in6), 1);
#else
		dev = ipv6_dev_find(&init_net, &(orig_tuple->src.u3.in6), NULL);
		dev_hold(dev);
#endif
		if (dev) {
			/*
			 * Source IP address is local
			 */
			dev_put(dev);
			DEBUG_TRACE("%px: NVGRE locally terminated (src) - do not allow acceleration\n", skb);
			return false;
		}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 9, 0))
		dev = ipv6_dev_find(&init_net, &(orig_tuple->dst.u3.in6), 1);
#else
		dev = ipv6_dev_find(&init_net, &(orig_tuple->dst.u3.in6), NULL);
		dev_hold(dev);
#endif
		if (dev) {
			/*
			 * Destination IP address is local
			 */
			dev_put(dev);
			DEBUG_TRACE("%px: NVGRE locally terminated (dest) - do not allow acceleration\n", skb);
			return false;
		}

#else
		DEBUG_TRACE("%px: IPv6 support not enabled\n", skb);
		return false;
#endif
	}

	/*
	 * Case 7: NVGRE pass through
	 */
	if (greh->flags & GRE_KEY) {
		DEBUG_TRACE("%px: NVGRE pass through - do not allow acceleration\n", skb);
		return false;
	}

	/*
	 * Case 8: GRE pass through non NAT
	 */
	DEBUG_TRACE("%px: GRE IPv%d pass through non NAT - allow acceleration\n", skb, ip_version);
	return true;
}

#ifdef ECM_CLASSIFIER_DSCP_ENABLE
/*
 * ecm_front_end_tcp_set_dscp_ext()
 *	Sets the DSCP remark extension.
 */
void ecm_front_end_tcp_set_dscp_ext(struct nf_conn *ct,
					      struct ecm_tracker_ip_header *iph,
					      struct sk_buff *skb,
					      ecm_tracker_sender_type_t sender)
{
	struct nf_ct_dscpremark_ext *dscpcte;

	/*
	 * Extract the priority and DSCP from skb and store into ct extension for each direction.
	 */
	spin_lock_bh(&ct->lock);
	dscpcte = nf_ct_dscpremark_ext_find(ct);
	if (dscpcte) {
		if (sender == ECM_TRACKER_SENDER_TYPE_SRC) {
			dscpcte->flow_priority = skb->priority;
			dscpcte->flow_mark = skb->mark;
			dscpcte->flow_dscp = iph->ds >> XT_DSCP_SHIFT;
			dscpcte->flow_set_flags = NF_CT_DSCPREMARK_EXT_PRIO | NF_CT_DSCPREMARK_EXT_DSCP | NF_CT_DSCPREMARK_EXT_MARK;
			DEBUG_TRACE("%px: sender: %d flow priority: %d flow dscp: %d flow_mark: %d flow_set_flags: 0x%x\n",
				    ct, sender, dscpcte->flow_priority, dscpcte->flow_dscp, dscpcte->flow_mark, dscpcte->flow_set_flags);
		} else {
			dscpcte->reply_priority =  skb->priority;
			dscpcte->reply_mark =  skb->mark;
			dscpcte->reply_dscp = iph->ds >> XT_DSCP_SHIFT;
			dscpcte->return_set_flags = NF_CT_DSCPREMARK_EXT_PRIO | NF_CT_DSCPREMARK_EXT_DSCP | NF_CT_DSCPREMARK_EXT_MARK;
			DEBUG_TRACE("%px: sender: %d reply priority: %d reply dscp: %d reply_mark: %d return_set_flags: 0x%x\n",
				    ct, sender, dscpcte->reply_priority, dscpcte->reply_dscp, dscpcte->reply_mark, dscpcte->return_set_flags);
		}
	}
	spin_unlock_bh(&ct->lock);
}
#endif

/*
 * ecm_front_end_fill_ovs_params()
 *	Set the OVS flow lookup parameters.
 *
 */
void ecm_front_end_fill_ovs_params(struct ecm_front_end_ovs_params ovs_params[],
					ip_addr_t ip_src_addr, ip_addr_t ip_src_addr_nat,
					ip_addr_t ip_dest_addr, ip_addr_t ip_dest_addr_nat,
					int src_port, int src_port_nat,
					int dest_port, int dest_port_nat, ecm_db_direction_t ecm_dir)
{
	DEBUG_INFO("ip_src_addr " ECM_IP_ADDR_DOT_FMT "\n", ECM_IP_ADDR_TO_DOT(ip_src_addr));
	DEBUG_INFO("ip_src_addr_nat " ECM_IP_ADDR_DOT_FMT "\n", ECM_IP_ADDR_TO_DOT(ip_src_addr_nat));
	DEBUG_INFO("ip_dest_addr " ECM_IP_ADDR_DOT_FMT "\n", ECM_IP_ADDR_TO_DOT(ip_dest_addr));
	DEBUG_INFO("ip_dest_addr_nat " ECM_IP_ADDR_DOT_FMT "\n", ECM_IP_ADDR_TO_DOT(ip_dest_addr_nat));
	DEBUG_INFO("src_port: %d, src_port_nat: %d, dest_port:%d, dest_port_nat:%d\n", src_port, src_port_nat, dest_port, dest_port_nat);

	/*
	 * A routed flow can go through ECM in 4 different NAT cases.
	 *
	 * 1. SNAT:
	 * PC1 -------------> eth1-ovs-br1--->ovs-br2-eth2 -------------> PC2
	 *			FROM		TO
	 *					TO_NAT
	 *					FROM_NAT
	 *
	 * ip_src_addr				ip_src_addr_nat		ip_dest_addr/ip_dest_addr_nat
	 *
	 *
	 * 2. DNAT:
	 * PC1 <------------- eth1-ovs-br1<---ovs-br2-eth2 <------------- PC2
	 *			TO		FROM
	 *					FROM_NAT
	 *					TO_NAT
	 *
	 * ip_dest_addr				ip_dest_addr_nat	ip_src_addr/ip_src_addr_nat
	 *
	 *
	 * 3. Non-NAT - Egress:
	 * PC1 -------------> eth1-ovs-br1--->ovs-br2-eth2 -------------> PC2
	 *			FROM		TO
	 *			FROM_NAT	TO_NAT
	 *
	 * ip_src_addr/ip_src_addr_nat					ip_dest_addr/ip_dest_addr_nat
	 *
	 *
	 * 4. Non-NAT - Ingress:
	 * PC1 <------------- eth1-ovs-br1<---ovs-br2-eth2 <------------- PC2
	 *			TO		FROM
	 *			TO_NAT		FROM_NAT
	 *
	 * ip_dest_addr/ip_dest_addr_nat				ip_src_addr/ip_src_addr_nat
	 */

	/*
	 * To look-up the FROM and TO ports, in all NAT cases we use the same IP/port combinations.
	 */
	ECM_IP_ADDR_COPY(ovs_params[ECM_DB_OBJ_DIR_FROM].src_ip, ip_dest_addr_nat);
	ECM_IP_ADDR_COPY(ovs_params[ECM_DB_OBJ_DIR_FROM].dest_ip, ip_src_addr);
	ovs_params[ECM_DB_OBJ_DIR_FROM].src_port = dest_port_nat;
	ovs_params[ECM_DB_OBJ_DIR_FROM].dest_port = src_port;

	ECM_IP_ADDR_COPY(ovs_params[ECM_DB_OBJ_DIR_TO].src_ip, ip_src_addr_nat);
	ECM_IP_ADDR_COPY(ovs_params[ECM_DB_OBJ_DIR_TO].dest_ip, ip_dest_addr);
	ovs_params[ECM_DB_OBJ_DIR_TO].src_port = src_port_nat;
	ovs_params[ECM_DB_OBJ_DIR_TO].dest_port = dest_port;

	if (ecm_dir == ECM_DB_DIRECTION_EGRESS_NAT) {
		/*
		 * For SNAT case (EGRESS_NAT), we use specific IP/port combinations to look-up
		 * the FROM_NAT and TO_NAT ports.
		 */
		ECM_IP_ADDR_COPY(ovs_params[ECM_DB_OBJ_DIR_FROM_NAT].src_ip, ip_src_addr_nat);
		ECM_IP_ADDR_COPY(ovs_params[ECM_DB_OBJ_DIR_FROM_NAT].dest_ip, ip_dest_addr_nat);
		ovs_params[ECM_DB_OBJ_DIR_FROM_NAT].src_port = src_port_nat;
		ovs_params[ECM_DB_OBJ_DIR_FROM_NAT].dest_port = dest_port_nat;

		ECM_IP_ADDR_COPY(ovs_params[ECM_DB_OBJ_DIR_TO_NAT].src_ip, ip_src_addr_nat);
		ECM_IP_ADDR_COPY(ovs_params[ECM_DB_OBJ_DIR_TO_NAT].dest_ip, ip_dest_addr_nat);
		ovs_params[ECM_DB_OBJ_DIR_TO_NAT].src_port = src_port_nat;
		ovs_params[ECM_DB_OBJ_DIR_TO_NAT].dest_port = dest_port_nat;
	} else if (ecm_dir == ECM_DB_DIRECTION_INGRESS_NAT) {
		/*
		 * For DNAT case (INGRESS_NAT), we use specific IP/port combinations to look-up
		 * the FROM_NAT and TO_NAT ports.
		 */
		ECM_IP_ADDR_COPY(ovs_params[ECM_DB_OBJ_DIR_FROM_NAT].src_ip, ip_dest_addr_nat);
		ECM_IP_ADDR_COPY(ovs_params[ECM_DB_OBJ_DIR_FROM_NAT].dest_ip, ip_src_addr_nat);
		ovs_params[ECM_DB_OBJ_DIR_FROM_NAT].src_port = dest_port_nat;
		ovs_params[ECM_DB_OBJ_DIR_FROM_NAT].dest_port = src_port_nat;

		ECM_IP_ADDR_COPY(ovs_params[ECM_DB_OBJ_DIR_TO_NAT].src_ip, ip_dest_addr_nat);
		ECM_IP_ADDR_COPY(ovs_params[ECM_DB_OBJ_DIR_TO_NAT].dest_ip, ip_src_addr_nat);
		ovs_params[ECM_DB_OBJ_DIR_TO_NAT].src_port = dest_port_nat;
		ovs_params[ECM_DB_OBJ_DIR_TO_NAT].dest_port = src_port_nat;
	} else {
		/*
		 * For Non-NAT case, we use the same IP/port combinations of FROM and TO to look-up
		 * the FROM_NAT and TO_NAT ports.
		 */
		ECM_IP_ADDR_COPY(ovs_params[ECM_DB_OBJ_DIR_FROM_NAT].src_ip, ip_dest_addr_nat);
		ECM_IP_ADDR_COPY(ovs_params[ECM_DB_OBJ_DIR_FROM_NAT].dest_ip, ip_src_addr);
		ovs_params[ECM_DB_OBJ_DIR_FROM_NAT].src_port = dest_port_nat;
		ovs_params[ECM_DB_OBJ_DIR_FROM_NAT].dest_port = src_port;

		ECM_IP_ADDR_COPY(ovs_params[ECM_DB_OBJ_DIR_TO_NAT].src_ip, ip_src_addr_nat);
		ECM_IP_ADDR_COPY(ovs_params[ECM_DB_OBJ_DIR_TO_NAT].dest_ip, ip_dest_addr);
		ovs_params[ECM_DB_OBJ_DIR_TO_NAT].src_port = src_port_nat;
		ovs_params[ECM_DB_OBJ_DIR_TO_NAT].dest_port = dest_port;
	}
}

/*
 * ecm_front_end_get_slow_packet_count()
 *	Gets the slow path packet count for the given connection.
 */
uint64_t ecm_front_end_get_slow_packet_count(struct ecm_front_end_connection_instance *feci)
{
	uint64_t slow_pkts;

	spin_lock_bh(&feci->lock);
	slow_pkts = feci->stats.slow_path_packets;
	spin_unlock_bh(&feci->lock);
	return slow_pkts;
}

#ifdef ECM_FRONT_END_PPE_ENABLE
/*
 * ecm_front_end_ppe_fse_enable_limit_handler()
 *	Sysctl to enable/disable FSE programming through PPE.
 */
int ecm_front_end_ppe_fse_enable_handler(struct ctl_table *ctl, int write, void __user *buffer, size_t *lenp, loff_t *ppos)
{
	int ret;

	/*
	 * Write the variable with user input
	 */
	ret = proc_dointvec(ctl, write, buffer, lenp, ppos);
	if (ret || (!write)) {
		/*
		 * Return failure.
		 */
		return ret;
	}

        if ((ecm_front_end_ppe_fse_enable != 0) &&
			(ecm_front_end_ppe_fse_enable != 1)) {
		DEBUG_WARN("Invalid input. Valid values 0/1\n");
		return -EINVAL;
	}

	if (ecm_front_end_ppe_fse_enable == 0) {
		ppe_drv_fse_feature_disable();
		return ret;
	}

	ppe_drv_fse_feature_enable();
	return ret;
}
#endif

/*
 * ecm_front_end_db_conn_limit_handler()
 *	Database connection limit sysctl node handler.
 */
int ecm_front_end_db_conn_limit_handler(struct ctl_table *ctl, int write, void __user *buffer, size_t *lenp, loff_t *ppos)
{
	int ret;
	int current_value;

	/*
	 * Take the current value
	 */
	current_value = ecm_front_end_conn_limit;

	/*
	 * Write the variable with user input
	 */
	ret = proc_dointvec(ctl, write, buffer, lenp, ppos);
	if (ret || (!write)) {
		/*
		 * Return failure.
		 */
		return ret;
	}

	if ((ecm_front_end_conn_limit != 0) &&
			(ecm_front_end_conn_limit != 1)) {
		DEBUG_WARN("Invalid input. Valid values 0/1\n");
		ecm_front_end_conn_limit = current_value;
		return -EINVAL;
	}

	return ret;
}

/*
 * ecm_front_end_denied_ports_read()
 *	Reads the denied ports from the denied ports array and prints.
 */
static void ecm_front_end_denied_ports_read(void __user *buffer, size_t *lenp, loff_t *ppos, struct hlist_head *denied_ports)
{
	char *read_buf;
	int i, len;
	size_t bytes = 0;
	struct hlist_node *pnode;

	/*
	 * (64 * 8) bytes for the buffer size is sufficient to write
	 * the array including the spaces and new line characters.
	 */
	read_buf = kzalloc(ECM_FRONT_END_DENIED_PORTS_HTABLE_SIZE * 8 * sizeof(char), GFP_KERNEL);
	if (!read_buf) {
		DEBUG_ERROR("Failed to alloc buffer to print denied port array\n");
		return;
	}

	for (i = 0; i < ECM_FRONT_END_DENIED_PORTS_HTABLE_SIZE; i++) {
		hlist_for_each(pnode, &denied_ports[i]) {
			struct ecm_denied_port_node *p = hlist_entry(pnode, struct ecm_denied_port_node, hnode);
			len = scnprintf(read_buf + bytes, 8, "%d ", p->port);
			if (!len) {
				DEBUG_ERROR("failed to read from buffer %d\n", p->port);
				kfree(read_buf);
				return;
			}
			bytes += len;
		}
	}

	/*
	 * Add new line character at the end.
	 */
	len = scnprintf(read_buf + bytes, 4, "\n");
	bytes += len;

	bytes = simple_read_from_buffer(buffer, *lenp, ppos, read_buf, bytes);
	*lenp = bytes;
	kfree(read_buf);
}

/*
 * ecm_front_end_is_port_in_denied_list()
 *	Checks if the port is in the given port list.
 */
static inline bool ecm_front_end_is_port_in_denied_list(int port, struct hlist_head *denied_ports)
{
	struct hlist_node *pnode;
	uint32_t hash = hash_32(port, ECM_FRONT_END_DENIED_PORTS_HTABLE_SIZE);

	hlist_for_each(pnode, &denied_ports[hash]) {
		struct ecm_denied_port_node *p = hlist_entry(pnode, struct ecm_denied_port_node, hnode);
		if (p->port == port) {
			return true;
		}
	}
	return false;
}

/*
 * ecm_front_end_denied_ports_handler()
 *	Proc handler function for denied ports read/write operation.
 */
static int ecm_front_end_denied_ports_handler(int write, void __user *buffer, size_t *lenp, loff_t *ppos, struct hlist_head *denied_ports, bool is_udp)
{

	char *buf;
	char *pfree;
	char *token;
	int count, port;
	long int val;

	if (!write) {
		ecm_front_end_denied_ports_read(buffer, lenp, ppos, denied_ports);
		return 0;
	}

	buf = kzalloc((ECM_FRONT_END_DENIED_PORTS_HTABLE_SIZE * 8) * sizeof(char), GFP_KERNEL);
	if (!buf) {
		return -ENOMEM;
	}

	pfree = buf;
	count = *lenp;
	if (count > (ECM_FRONT_END_DENIED_PORTS_HTABLE_SIZE * 8 * sizeof(char))) {
		count = ECM_FRONT_END_DENIED_PORTS_HTABLE_SIZE * 8 * sizeof(char);
	}

	if (copy_from_user(buf, buffer, count)) {
		kfree(pfree);
		return -EFAULT;
	}

	token = strsep(&buf, " ");
	if (strlen(token) != 3) {
		DEBUG_ERROR("cmd should be add or del\n");
		kfree(pfree);
		return -EINVAL;
	}

	if (!strncmp(token, "add", 3)) {
		while (buf) {
			token = strsep(&buf, " ");
			if (kstrtol(token, 10, &val)) {
				DEBUG_ERROR("%s is not a number\n", token);
				kfree(pfree);
				return -EINVAL;
			}
			if (sscanf(token, "%d", &port)) {
				struct ecm_denied_port_node *p;
				uint32_t hash;

				if (port < 0 || port > 65535) {
					DEBUG_ERROR("port %d is not between (0-65535)\n", port);
					kfree(pfree);
					return -EINVAL;
				}
				if (ecm_front_end_is_port_in_denied_list(port, denied_ports)) {
					DEBUG_WARN("port: %d is already in the port list\n", port);
					continue;
				}

				p = vmalloc(sizeof(struct ecm_denied_port_node));
				if (!p) {
					DEBUG_ERROR("unable to allocate memory for port node: %d\n", port);
					return -ENOMEM;
				}

				hash = hash_32(port, ECM_FRONT_END_DENIED_PORTS_HTABLE_SIZE);

				p->port = port;
				INIT_HLIST_NODE(&p->hnode);
				hlist_add_head(&p->hnode, &denied_ports[hash]);
				if (is_udp) {
					atomic_inc(&ecm_udp_denied_port_count);
				} else {
					atomic_inc(&ecm_tcp_denied_port_count);
				}
			}
		}
	} else if (!strncmp(token, "del", 3)) {
		while (buf) {
			token = strsep(&buf, " ");
			if (kstrtol(token, 10, &val)) {
				DEBUG_ERROR("%s is not a number\n", token);
				kfree(pfree);
				return -EINVAL;
			}
			if (sscanf(token, "%d", &port)) {
				struct hlist_node *pnode, *temp;
				uint32_t hash;

				if (port < 0 || port > 65535) {
					DEBUG_ERROR("port %d is not between (0-65535)\n", port);
					kfree(pfree);
					return -EINVAL;
				}

				hash = hash_32(port, ECM_FRONT_END_DENIED_PORTS_HTABLE_SIZE);
				hlist_for_each_safe(pnode, temp, &denied_ports[hash]) {
					struct ecm_denied_port_node *p = hlist_entry(pnode, struct ecm_denied_port_node, hnode);
					if (p->port == port) {
						hlist_del(&p->hnode);
						vfree(p);
						if (is_udp) {
							atomic_dec(&ecm_udp_denied_port_count);
						} else {
							atomic_dec(&ecm_tcp_denied_port_count);
						}
					}
				}
			}
		}
	} else {
		DEBUG_ERROR("invalid command: %s\n", token);
	}

	kfree(pfree);
	return count;
}

/*
 * ecm_front_end_udp_denied_ports_handler()
 *	Proc handler function for UDP denied ports read/write operation.
 */
static int ecm_front_end_udp_denied_ports_handler(struct ctl_table *ctl, int write, void __user *buffer, size_t *lenp, loff_t *ppos)
{
	/*
	 * Usage:
	 *	Add ports to the list:
	 *	echo add 53 67 146 > /proc/sys/net/ecm/udp_denied_ports
	 *
	 *	Delete ports from the list:
	 *	echo del 67 > /proc/sys/net/ecm/udp_denied_ports
	 *
	 *	Dump the list to the console:
	 *	cat /proc/sys/net/ecm/udp_denied_ports
	 */
	return ecm_front_end_denied_ports_handler(write, buffer, lenp, ppos, ecm_front_end_udp_denied_ports, true);
}

/*
 * ecm_front_end_tcp_denied_ports_handler()
 *	Proc handler function for TCP denied ports read/write operation.
 */
static int ecm_front_end_tcp_denied_ports_handler(struct ctl_table *ctl, int write, void __user *buffer, size_t *lenp, loff_t *ppos)
{
	/*
	 * Usage:
	 *	Add ports to the list:
	 *	echo add 53 67 146 > /proc/sys/net/ecm/tcp_denied_ports
	 *
	 *	Delete ports from the list:
	 *	echo del 67 > /proc/sys/net/ecm/tcp_denied_ports
	 *
	 *	Dump the list to the console:
	 *	cat /proc/sys/net/ecm/tcp_denied_ports
	 */
	return ecm_front_end_denied_ports_handler(write, buffer, lenp, ppos, ecm_front_end_tcp_denied_ports, false);
}

static struct ctl_table ecm_front_end_sysctl_tbl[] = {
	{
		.procname	= "front_end_conn_limit",
		.data		= &ecm_front_end_conn_limit,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &ecm_front_end_db_conn_limit_handler,
	},
#ifdef ECM_FRONT_END_PPE_ENABLE
	{
		.procname	= "ppe_fse_enable",
		.data		= &ecm_front_end_ppe_fse_enable,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &ecm_front_end_ppe_fse_enable_handler,
	},
#endif
	{
		.procname	= "udp_denied_ports",
		.data		= &ecm_front_end_udp_denied_ports,
		.maxlen		= sizeof(int) * ECM_FRONT_END_DENIED_PORTS_HTABLE_SIZE,
		.mode		= 0644,
		.proc_handler	= &ecm_front_end_udp_denied_ports_handler,
	},
	{
		.procname	= "tcp_denied_ports",
		.data		= &ecm_front_end_tcp_denied_ports,
		.maxlen		= sizeof(int) * ECM_FRONT_END_DENIED_PORTS_HTABLE_SIZE,
		.mode		= 0644,
		.proc_handler	= &ecm_front_end_tcp_denied_ports_handler,
	},
	{}
};

/*
 * ecm_front_end_common_sysctl_register()
 *	Function to register sysctl node during front end init
 */
void ecm_front_end_common_sysctl_register()
{
	/*
	 * Register sysctl table.
	 */
	ecm_front_end_ctl_tbl_hdr = register_sysctl("net/ecm", ecm_front_end_sysctl_tbl);
#ifdef ECM_FRONT_END_SFE_ENABLE
	if (ecm_front_end_ctl_tbl_hdr) {
		ecm_sfe_sysctl_tbl_init();
	}
#endif
}

/*
 * ecm_front_end_common_sysctl_unregister()
 *	Function to unregister sysctl node during front end exit
 */
void ecm_front_end_common_sysctl_unregister()
{
	/*
	 * Unregister sysctl table.
	 */
	if (ecm_front_end_ctl_tbl_hdr) {
#ifdef ECM_FRONT_END_SFE_ENABLE
		ecm_sfe_sysctl_tbl_exit();
#endif
		unregister_sysctl_table(ecm_front_end_ctl_tbl_hdr);
	}
}

/*
 * ecm_front_end_connection_accel_state_get()
 *      Get acceleration state
 */
ecm_front_end_acceleration_mode_t ecm_front_end_connection_accel_state_get(struct ecm_front_end_connection_instance *feci)
{
	ecm_front_end_acceleration_mode_t state;

	DEBUG_CHECK_MAGIC(feci, ECM_FRONT_END_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", feci);
	spin_lock_bh(&feci->lock);
	state = feci->accel_mode;
	spin_unlock_bh(&feci->lock);
	return state;
}

/*
 * ecm_front_end_connection_action_seen()
 *      Acceleration action / activity has been seen for this connection.
 *
 * NOTE: Call the action_seen() method when the AE has demonstrated that it has offloaded some data for a connection.
 */
void ecm_front_end_connection_action_seen(struct ecm_front_end_connection_instance *feci)
{
	DEBUG_CHECK_MAGIC(feci, ECM_FRONT_END_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", feci);
	DEBUG_INFO("%px: Action seen\n", feci);
	spin_lock_bh(&feci->lock);
	feci->stats.no_action_seen = 0;
	spin_unlock_bh(&feci->lock);
}

/*
 * ecm_front_end_connection_ref()
 *      Ref a connection front end instance
 */
void ecm_front_end_connection_ref(struct ecm_front_end_connection_instance *feci)
{
	DEBUG_CHECK_MAGIC(feci, ECM_FRONT_END_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", feci);
	spin_lock_bh(&feci->lock);
	feci->refs++;
	DEBUG_TRACE("%px: feci ref %d\n", feci, feci->refs);
	DEBUG_ASSERT(feci->refs > 0, "%px: ref wrap\n", feci);
	spin_unlock_bh(&feci->lock);
}

/*
 * ecm_front_end_connection_deref()
 *      Deref a connection front end instance
 */
int ecm_front_end_connection_deref(struct ecm_front_end_connection_instance *feci)
{
	DEBUG_CHECK_MAGIC(feci, ECM_FRONT_END_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", feci);

	spin_lock_bh(&feci->lock);
	feci->refs--;
	DEBUG_ASSERT(feci->refs >= 0, "%px: ref wrap\n", feci);

	if (feci->refs > 0) {
		int refs = feci->refs;
		spin_unlock_bh(&feci->lock);
		DEBUG_TRACE("%px: feci deref %d\n", feci, refs);
		return refs;
	}
	spin_unlock_bh(&feci->lock);

	/*
	* We can now destroy the instance
	*/
	DEBUG_TRACE("%px: feci final\n", feci);
	DEBUG_CLEAR_MAGIC(feci);
	kfree(feci);
	return 0;
}

#ifdef ECM_NON_PORTED_SUPPORT_ENABLE
/*
 * ecm_front_end_non_ported_ipv6_connection_update()
 *	Update the non-ported IPv6 feci instance fields.
 */
static void ecm_front_end_non_ported_ipv6_connection_update(struct ecm_front_end_connection_instance *feci,
							enum ecm_front_end_engine ae_type)
{
	DEBUG_ASSERT(spin_is_locked(&feci->lock), "%px: feci lock is not held\n", feci);

	switch (ae_type) {
#ifdef ECM_FRONT_END_NSS_ENABLE
	case ECM_FRONT_END_ENGINE_NSS:
		DEBUG_ASSERT(NULL, "%px: cannot switch to NSS from any other AEs\n", feci);
		break;
#endif
#ifdef ECM_FRONT_END_PPE_ENABLE
	case ECM_FRONT_END_ENGINE_PPE:
		DEBUG_ASSERT(NULL, "%px: cannot switch to PPE from any other AEs\n", feci);
		break;
#endif
#ifdef ECM_FRONT_END_SFE_ENABLE
	case ECM_FRONT_END_ENGINE_SFE:
		ecm_sfe_non_ported_ipv6_connection_set(feci, 0);
		break;
#endif
	default:
		DEBUG_WARN("%px: unknown AE type to update\n", feci);
		break;
	}
}

/*
 * ecm_front_end_non_ported_ipv4_connection_update()
 *	Update the non-ported IPv4 feci instance fields.
 */
static void ecm_front_end_non_ported_ipv4_connection_update(struct ecm_front_end_connection_instance *feci,
							enum ecm_front_end_engine ae_type)
{
	DEBUG_ASSERT(spin_is_locked(&feci->lock), "%px: feci lock is not held\n", feci);

	switch (ae_type) {
#ifdef ECM_FRONT_END_NSS_ENABLE
	case ECM_FRONT_END_ENGINE_NSS:
		DEBUG_ASSERT(NULL, "%px: cannot switch to NSS from any other AEs\n", feci);
		break;
#endif
#ifdef ECM_FRONT_END_PPE_ENABLE
	case ECM_FRONT_END_ENGINE_PPE:
		DEBUG_ASSERT(NULL, "%px: cannot switch to PPE from any other AEs\n", feci);
		break;
#endif
#ifdef ECM_FRONT_END_SFE_ENABLE
	case ECM_FRONT_END_ENGINE_SFE:
		ecm_sfe_non_ported_ipv4_connection_set(feci, 0);
		break;
#endif
	default:
		DEBUG_WARN("%px: unknown AE type to update\n", feci);
		break;
	}
}
#endif

/*
 * ecm_front_end_ported_ipv6_connection_update()
 *	Update the ported IPv6 feci instance fields.
 */
static void ecm_front_end_ported_ipv6_connection_update(struct ecm_front_end_connection_instance *feci,
							enum ecm_front_end_engine ae_type)
{
	DEBUG_ASSERT(spin_is_locked(&feci->lock), "%px: feci lock is not held\n", feci);

	switch (ae_type) {
#ifdef ECM_FRONT_END_NSS_ENABLE
	case ECM_FRONT_END_ENGINE_NSS:
		DEBUG_ASSERT(NULL, "%px: cannot switch to NSS from any other AEs\n", feci);
		break;
#endif
#ifdef ECM_FRONT_END_PPE_ENABLE
	case ECM_FRONT_END_ENGINE_PPE:
		ecm_ppe_ported_ipv6_connection_set(feci, feci->fe_info.front_end_flags);
		break;
#endif
#ifdef ECM_FRONT_END_SFE_ENABLE
	case ECM_FRONT_END_ENGINE_SFE:
		ecm_sfe_ported_ipv6_connection_set(feci, feci->fe_info.front_end_flags);
		break;
#endif
	default:
		DEBUG_WARN("%px: unknown AE type to update\n", feci);
		break;
	}
}

/*
 * ecm_front_end_ported_ipv4_connection_update()
 *	Update the ported IPv4 feci instance fields.
 */
static void ecm_front_end_ported_ipv4_connection_update(struct ecm_front_end_connection_instance *feci,
							enum ecm_front_end_engine ae_type)
{
	DEBUG_ASSERT(spin_is_locked(&feci->lock), "%px: feci lock is not held\n", feci);

	switch (ae_type) {
#ifdef ECM_FRONT_END_NSS_ENABLE
	case ECM_FRONT_END_ENGINE_NSS:
		DEBUG_ASSERT(NULL, "%px: cannot switch to NSS from any other AEs\n", feci);
		break;
#endif
#ifdef ECM_FRONT_END_PPE_ENABLE
	case ECM_FRONT_END_ENGINE_PPE:
		ecm_ppe_ported_ipv4_connection_set(feci, feci->fe_info.front_end_flags);
		break;
#endif
#ifdef ECM_FRONT_END_SFE_ENABLE
	case ECM_FRONT_END_ENGINE_SFE:
		ecm_sfe_ported_ipv4_connection_set(feci, feci->fe_info.front_end_flags);
		break;
#endif
	default:
		DEBUG_WARN("%px: unknown AE type to update\n", feci);
		break;
	}
}

/*
 * ecm_front_end_connection_limit_reached()
 *	Check connection limit.
 */
bool ecm_front_end_connection_limit_reached(enum ecm_front_end_engine ae_type, int ip_version)
{
	switch (ae_type) {
#ifdef ECM_FRONT_END_NSS_ENABLE
	case ECM_FRONT_END_ENGINE_NSS:
		if (ip_version == 4 && ecm_nss_ipv4_is_conn_limit_reached()) {
			return true;
		}

		if (ip_version == 6 && ecm_nss_ipv6_is_conn_limit_reached()) {
			return true;
		}
		break;
#endif
#ifdef ECM_FRONT_END_SFE_ENABLE
	case ECM_FRONT_END_ENGINE_SFE:
		if (ip_version == 4 && ecm_sfe_ipv4_is_conn_limit_reached()) {
			return true;
		}

		if (ip_version == 6 && ecm_sfe_ipv6_is_conn_limit_reached()) {
			return true;
		}
		break;
#endif
#ifdef ECM_FRONT_END_PPE_ENABLE
	case ECM_FRONT_END_ENGINE_PPE:
		if (ip_version == 4 && ecm_ppe_ipv4_is_conn_limit_reached()) {
			return true;
		}

		if (ip_version == 6 && ecm_ppe_ipv6_is_conn_limit_reached()) {
			return true;
		}
		break;
#endif
	default:
		DEBUG_WARN("wrong ae_type: %d\n", ae_type);
	}

	return false;
}

/*
 * ecm_front_end_connection_check_and_switch_to_next_ae()
 *	Switches the AE type of the front end instance.
 */
bool ecm_front_end_connection_check_and_switch_to_next_ae(struct ecm_front_end_connection_instance *feci)
{
	int i;
	int new_ae_type = ECM_AE_PRECEDENCE_MAX;

	DEBUG_CHECK_MAGIC(feci, ECM_FRONT_END_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", feci);

	DEBUG_TRACE("%px: Frontend switch from AE type %d\n", feci, feci->accel_engine);

	spin_lock_bh(&feci->lock);

	if (feci->is_defunct) {
		spin_unlock_bh(&feci->lock);
		DEBUG_TRACE("%px: AE switch can't be done for defuncted flow\n", feci);
		return false;
	}

	/*
	 * check if acceleration engine needs to be changed
	 * if change_ae_type flag is set then it means that sawf wants to change the ae
	 */
	if (feci->fe_info.front_end_flags & ECM_FRONT_END_ENGINE_FLAG_SAWF_CHANGE_AE_TYPE) {
		new_ae_type = feci->next_accel_engine;
		feci->fe_info.front_end_flags &= ~ ECM_FRONT_END_ENGINE_FLAG_SAWF_CHANGE_AE_TYPE;
		feci->fe_info.front_end_flags |= ECM_FRONT_END_ENGINE_FLAG_SAWF_CHANGE_AE_TYPE_DONE;
		goto change_ae;
	}

	/*
	 * Check the accel_mode of the existing connection.
	 * If it is set to one of the FAIL modes, this means that, we tried to accelerate
	 * the connection with the selected AE and it is not accepted. We can switch to the next AE
	 * if there is one.
	 */
	if (!ECM_FRONT_END_ACCELERATION_FAILED(feci->accel_mode)
			|| (feci->accel_mode == ECM_FRONT_END_ACCELERATION_MODE_FAIL_DENIED)) {
		spin_unlock_bh(&feci->lock);
		DEBUG_TRACE("%px: AE switch is not possible yet, accel mode %d\n", feci, feci->accel_mode);
		return false;
	}

	/*
	 * If an external AE classifier is registered, we may not want to dependent on the
	 * precedence array for our next AE selection. SFE is the default option if fallback
	 * is enabled. Registrant may have requested fallback to be disabled, in case of a
	 * flow acceleration failure to PPE. Hence query the AE classifier for the fallback setting first
	 */
	if (ecm_ae_classifier_is_external(&ae_ops) &&
		!(feci->fe_info.front_end_flags & ECM_FRONT_END_ENGINE_FLAG_AE_PRECEDENCE)) {
		if (ecm_ae_classifier_is_fallback_enabled(&ae_ops)) {
			if (feci->accel_engine == ECM_FRONT_END_ENGINE_SFE) {
				spin_unlock_bh(&feci->lock);
				DEBUG_TRACE("%px: Already trying in SFE mode. Not falling back to any other AE.\n", feci);
				return false;
			}

			new_ae_type = ECM_FRONT_END_ENGINE_SFE;
			DEBUG_TRACE("%px: Fallback to SFE in AE classifier mode.\n", feci);
			goto change_ae;
		} else {
			spin_unlock_bh(&feci->lock);
			DEBUG_TRACE("%px: Not falling back to any other AE in AE classifier mode.\n", feci);
			return false;
		}
	}

	/*
	 * Find the next AE type in the precedence array.
	 */
	for (i = 0; i < ECM_AE_PRECEDENCE_MAX; i++) {
		if (ae_precedence[i].ae_type == feci->accel_engine) {
			/*
			 * Check if the next AE in the precedence array is valid.
			 */
			new_ae_type = ae_precedence[++i].ae_type;
			if (new_ae_type == ECM_FRONT_END_ENGINE_MAX) {
				spin_unlock_bh(&feci->lock);
				DEBUG_TRACE("%px: There is no next AE to switch\n", feci);
				return false;
			}
			break;
		}
	}

change_ae:
	/*
	 * Check if this new AE has space for a new connection.
	 */
	if (ecm_front_end_connection_limit_reached(new_ae_type, feci->ip_version)) {
		spin_unlock_bh(&feci->lock);
		DEBUG_TRACE("%px: AE type: %d reached its connection limit\n", feci, new_ae_type);
		return false;
	}

	switch (feci->ip_version) {
	case 4:
		if ((feci->protocol == IPPROTO_UDP) || (feci->protocol == IPPROTO_TCP)) {
			ecm_front_end_ported_ipv4_connection_update(feci, new_ae_type);
		} else {
#ifdef ECM_NON_PORTED_SUPPORT_ENABLE
			ecm_front_end_non_ported_ipv4_connection_update(feci, new_ae_type);
#else
			spin_unlock_bh(&feci->lock);
			DEBUG_ERROR("%px: ECM non ported support is disabled\n", feci);
			return false;
#endif
		}
		break;

	case 6:
		if ((feci->protocol == IPPROTO_UDP) || (feci->protocol == IPPROTO_TCP)) {
			ecm_front_end_ported_ipv6_connection_update(feci, new_ae_type);
		} else {
#ifdef ECM_NON_PORTED_SUPPORT_ENABLE
			ecm_front_end_non_ported_ipv6_connection_update(feci, new_ae_type);
#else
			spin_unlock_bh(&feci->lock);
			DEBUG_ERROR("%px: ECM non ported support is disabled\n", feci);
			return false;
#endif
		}
		break;

	default:
		spin_unlock_bh(&feci->lock);
		DEBUG_ERROR("%px: Unexpected IP version: %d\n", feci, feci->ip_version);
		return false;
	}

	feci->accel_mode = ECM_FRONT_END_ACCELERATION_MODE_DECEL;
	spin_unlock_bh(&feci->lock);
	DEBUG_TRACE("%px: Frontend switch to AE type %d\n", feci, feci->accel_engine);
	return true;
}

/*
 * ecm_front_end_common_get_stats_bitmap()
 *	Get bit map
 */
uint32_t ecm_front_end_common_get_stats_bitmap(struct ecm_front_end_connection_instance *feci, ecm_db_obj_dir_t dir)
{
	DEBUG_CHECK_MAGIC(feci, ECM_FRONT_END_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", feci);

	switch (dir) {
	case ECM_DB_OBJ_DIR_FROM:
		return feci->fe_info.from_stats_bitmap;

	case ECM_DB_OBJ_DIR_TO:
		return feci->fe_info.to_stats_bitmap;

	default:
		DEBUG_WARN("Direction not handled dir=%d for get stats bitmap\n", dir);
		break;
	}

	return 0;
}

/*
 * ecm_front_end_common_set_stats_bitmap()
 *	Set bit map
 */
void ecm_front_end_common_set_stats_bitmap(struct ecm_front_end_connection_instance *feci, ecm_db_obj_dir_t dir, uint8_t bit)
{
	DEBUG_CHECK_MAGIC(feci, ECM_FRONT_END_CONNECTION_INSTANCE_MAGIC, "%px: magic failed", feci);

	switch (dir) {
	case ECM_DB_OBJ_DIR_FROM:
		feci->fe_info.from_stats_bitmap |= BIT(bit);
		break;

	case ECM_DB_OBJ_DIR_TO:
		feci->fe_info.to_stats_bitmap |= BIT(bit);
		break;
	default:
		DEBUG_WARN("Direction not handled dir=%d for set stats bitmap\n", dir);
		break;
	}
}

/*
 * ecm_front_end_check_udp_denied_ports()
 *	Checks if any given UDP port number is in the acceleration denied list.
 */
bool ecm_front_end_check_udp_denied_ports(uint16_t src_port, uint16_t dest_port)
{
	if (!atomic_read(&ecm_udp_denied_port_count)) {
		DEBUG_TRACE("No denied port for UDP\n");
		return false;
	}

	/*
	 * Check first the source port.
	 */
	if (ecm_front_end_is_port_in_denied_list(src_port, ecm_front_end_udp_denied_ports)) {
		return true;
	}

	/*
	 * Then check the dest port.
	 */
	return ecm_front_end_is_port_in_denied_list(dest_port, ecm_front_end_udp_denied_ports);
}

/*
 * ecm_front_end_check_tcp_denied_ports()
 *	Checks if any given TCP port number is in the acceleration denied list.
 */
bool ecm_front_end_check_tcp_denied_ports(uint16_t src_port, uint16_t dest_port)
{
	if (!atomic_read(&ecm_tcp_denied_port_count)) {
		DEBUG_TRACE("No denied port for TCP\n");
		return false;
	}

	/*
	 * Check first the source port.
	 */
	if (ecm_front_end_is_port_in_denied_list(src_port, ecm_front_end_tcp_denied_ports)) {
		return true;
	}

	/*
	 * Then check the dest port.
	 */
	return ecm_front_end_is_port_in_denied_list(dest_port, ecm_front_end_tcp_denied_ports);
}

/*
 * ecm_front_end_common_intf_ingress_qdisc_check()
 *      Checks if ingress qdisc is configured on the given interface
 */
bool ecm_front_end_common_intf_ingress_qdisc_check(int32_t interface_num)
{
#if defined(CONFIG_NET_CLS_ACT)
	struct net_device *dev;
	struct mini_Qdisc *miniq;

	dev = dev_get_by_index(&init_net, interface_num);
	if (!dev) {
		DEBUG_INFO("device-ifindex[%d] is not present\n", interface_num);
		return false;
	}

	BUG_ON(!rcu_read_lock_bh_held());
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0))
	miniq = rcu_dereference_bh(dev->miniq_ingress);
#else
	struct bpf_mprog_entry *entry = rcu_dereference_bh(dev->tcx_ingress);
	miniq = entry ? tcx_entry(entry)->miniq : NULL;
#endif
	if (miniq) {
		DEBUG_INFO("Ingress Qdisc is present for device[%s]\n", dev->name);
		dev_put(dev);
		return true;
	}

	DEBUG_INFO("Ingress Qdisc is not present for device[%s]\n", dev->name);
	dev_put(dev);
#endif
	return false;
}

/*
 * ecm_front_end_common_intf_qdisc_check()
 *      Checks if qdisc is configured on the given interface
 */
bool ecm_front_end_common_intf_qdisc_check(int32_t interface_num, bool *is_ppeq)
{
	struct net_device *dev;
        struct netdev_queue *txq;
        struct Qdisc *q;
        int i;
#if defined(CONFIG_NET_CLS_ACT) && defined(CONFIG_NET_EGRESS)
	struct mini_Qdisc *miniq;
#endif

	*is_ppeq = false;
	dev = dev_get_by_index(&init_net, interface_num);
	if (!dev) {
		DEBUG_INFO("device-ifindex[%d] is not present\n", interface_num);
		return false;
	}

	BUG_ON(!rcu_read_lock_bh_held());
	for (i = 0; i < dev->real_num_tx_queues; i++) {
		txq = netdev_get_tx_queue(dev, i);
		q = rcu_dereference_bh(txq->qdisc);
		if ((!q) || (!q->enqueue)) {
			continue;
		}

#ifdef ECM_FRONT_END_PPE_QOS_ENABLE
		if (q->flags & TCQ_F_NSS) {
			DEBUG_INFO("PPE Qdisc is present for device[%s]\n", dev->name);
			*is_ppeq = true;
                }
#endif

		DEBUG_INFO("Qdisc is present for device[%s]\n", dev->name);
		dev_put(dev);
		return true;
	}

#if defined(CONFIG_NET_CLS_ACT) && defined(CONFIG_NET_EGRESS)
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0))
	miniq = rcu_dereference_bh(dev->miniq_egress);
#else
	struct bpf_mprog_entry *entry = rcu_dereference_bh(dev->tcx_egress);
	miniq = entry ? tcx_entry(entry)->miniq : NULL;
#endif
	if (miniq) {
		DEBUG_INFO("Egress needed\n");
		dev_put(dev);
		return true;
	}
#endif

	DEBUG_WARN("%px Qdisc is not present for device[%s]\n", dev, dev->name);
	dev_put(dev);
	return false;
}

#ifdef ECM_FRONT_END_FSE_ENABLE
/*
 * ecm_front_end_fse_info_get()
 *	Get the FSE info from ECM frontend connection instance.
 */
bool ecm_front_end_fse_info_get(struct ecm_front_end_connection_instance *feci, struct ecm_front_end_fse_info *fse_info)
{
	ip_addr_t src_ip;
	ip_addr_t dest_ip;

	/*
	 * Get destination net device from 'TO' side of ecm interface hierarchy.
	 */
	fse_info->dest_dev = ecm_db_connection_first_iface_dev_get_and_ref(feci->ci, ECM_DB_OBJ_DIR_TO);
	if (!fse_info->dest_dev) {
		DEBUG_WARN("%px: Failed to get net device with %d dir\n", feci, ECM_DB_OBJ_DIR_TO);
		return false;
	}

	dev_put(fse_info->dest_dev);

	/*
	 * Get source net device from 'FROM' side of interface hierarchy.
	 */
	fse_info->src_dev = ecm_db_connection_first_iface_dev_get_and_ref(feci->ci, ECM_DB_OBJ_DIR_FROM);
	if (!fse_info->src_dev) {
		DEBUG_WARN("%px: Failed to get net device with %d dir\n", feci, ECM_DB_OBJ_DIR_FROM);
		return false;
	}

	dev_put(fse_info->src_dev);

	/*
	 * Get the 5 tuple information from front end connection instance.
	 */
	fse_info->ip_version = ecm_db_connection_ip_version_get(feci->ci);
	fse_info->protocol = ecm_db_connection_protocol_get(feci->ci);
	fse_info->src_port = ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_FROM);
	fse_info->dest_port = ecm_db_connection_port_get(feci->ci, ECM_DB_OBJ_DIR_TO);
	ecm_db_connection_node_address_get(feci->ci, ECM_DB_OBJ_DIR_FROM, fse_info->src_mac);
	ecm_db_connection_node_address_get(feci->ci, ECM_DB_OBJ_DIR_TO, fse_info->dest_mac);
	ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_FROM, src_ip);
	ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_TO, dest_ip);

	if (fse_info->ip_version == 4) {
		ECM_IP_ADDR_TO_NIN4_ADDR(fse_info->src.v4_addr, src_ip);
		ECM_IP_ADDR_TO_NIN4_ADDR(fse_info->dest.v4_addr, dest_ip);
	} else if (fse_info->ip_version == 6) {
		ECM_IP_ADDR_TO_NIN6_ADDR(fse_info->src.v6_addr, src_ip);
		ECM_IP_ADDR_TO_NIN6_ADDR(fse_info->dest.v6_addr, dest_ip);
	}

	return true;
}

/*
 * ecm_front_end_fse_callbacks_register()
 *	Registers ECM FSE common callbacks.
 */
int ecm_front_end_fse_callbacks_register(struct ecm_front_end_fse_callbacks *fse_cb)
{
	if (ecm_fe_fse_cb) {
		DEBUG_ERROR("ECM FSE callbacks are already registered\n");
		return -1;
	}
	rcu_assign_pointer(ecm_fe_fse_cb, fse_cb);
	synchronize_rcu();

	return 0;
}
EXPORT_SYMBOL(ecm_front_end_fse_callbacks_register);

/*
 * ecm_front_end_fse_callbacks_unregister()
 *	Unregisters ECM FSE common callbacks.
 */
void ecm_front_end_fse_callbacks_unregister(void)
{
	rcu_assign_pointer(ecm_fe_fse_cb, NULL);
	synchronize_rcu();
}
EXPORT_SYMBOL(ecm_front_end_fse_callbacks_unregister);
#endif

/*
 * ecm_front_end_is_ae_type_feature_supported()
 *	checks whether selected acceleration engine's featue is supported.
 */
bool ecm_front_end_is_ae_type_feature_supported(ecm_ae_classifier_result_t ae_type, struct sk_buff *skb,
									struct ecm_tracker_ip_header *iph)
{
	switch (ae_type) {
#ifdef ECM_FRONT_END_NSS_ENABLE
	case ECM_AE_CLASSIFIER_RESULT_NSS:
		if ((ecm_front_end_feature_check(skb, iph)) &&
					(ecm_front_end_is_feature_supported(ECM_FE_FEATURE_NSS))) {
			return true;
		}
		break;
#endif
#ifdef ECM_FRONT_END_SFE_ENABLE
	case ECM_AE_CLASSIFIER_RESULT_SFE:
		if ((ecm_front_end_feature_check(skb, iph)) &&
					(ecm_front_end_is_feature_supported(ECM_FE_FEATURE_SFE))) {
			return true;
		}
		break;
#endif
#ifdef ECM_FRONT_END_PPE_ENABLE
	case ECM_AE_CLASSIFIER_RESULT_PPE:
	case ECM_AE_CLASSIFIER_RESULT_PPE_DS:
	case ECM_AE_CLASSIFIER_RESULT_PPE_VP:
		if ((ecm_front_end_feature_check(skb, iph)) &&
					(ecm_front_end_is_feature_supported(ECM_FE_FEATURE_PPE))) {
			return true;
		}
		break;
#endif
	default:
		DEBUG_WARN("unexpected ae type: %d\n", ae_type);
	}
	return false;
}

/*
 * ecm_front_end_ae_type_to_supported_ae_engine()
 *	maps ae type to corresponding engine
 *	sets required front end flags for the given mode
 *	returns the supported acceleration engine for the selected ae type
 */
enum ecm_front_end_engine ecm_front_end_ae_type_to_supported_ae_engine(uint32_t *flags,
									ecm_ae_classifier_result_t ae_type)
{
	switch (ae_type) {
#ifdef ECM_FRONT_END_NSS_ENABLE
	case ECM_AE_CLASSIFIER_RESULT_NSS:
		return ECM_FRONT_END_ENGINE_NSS;
#endif
#ifdef ECM_FRONT_END_SFE_ENABLE
	case ECM_AE_CLASSIFIER_RESULT_SFE:
		return ECM_FRONT_END_ENGINE_SFE;
#endif
#ifdef ECM_FRONT_END_PPE_ENABLE
	case ECM_AE_CLASSIFIER_RESULT_PPE_DS:
		*flags |= ECM_FRONT_END_ENGINE_FLAG_PPE_DS;
		return ECM_FRONT_END_ENGINE_PPE;
	case ECM_AE_CLASSIFIER_RESULT_PPE_VP:
		*flags |= ECM_FRONT_END_ENGINE_FLAG_PPE_VP;
		return ECM_FRONT_END_ENGINE_PPE;
	case ECM_AE_CLASSIFIER_RESULT_PPE:
		return ECM_FRONT_END_ENGINE_PPE;
#endif
	default:
		DEBUG_WARN("unexpected ae type: %d\n", ae_type);
		return ECM_FRONT_END_ENGINE_MAX;
	}
}

/*
 * ecm_front_end_accel_engine_to_ae_type()
 *	returns possible corresponding ae type
 */
ecm_ae_classifier_result_t ecm_front_end_accel_engine_to_ae_type(enum ecm_front_end_engine accel_engine)
{
	switch (accel_engine) {
#ifdef ECM_FRONT_END_NSS_ENABLE
	case ECM_FRONT_END_ENGINE_NSS:
		return ECM_AE_CLASSIFIER_RESULT_NSS;
#endif
#ifdef ECM_FRONT_END_SFE_ENABLE
	case ECM_FRONT_END_ENGINE_SFE:
		return ECM_AE_CLASSIFIER_RESULT_SFE;
#endif
#ifdef ECM_FRONT_END_PPE_ENABLE
	case ECM_FRONT_END_ENGINE_PPE:
		return ECM_AE_CLASSIFIER_RESULT_PPE;
#endif
	default:
		DEBUG_WARN("unexpected acceleration engine: %d\n", accel_engine);
		return ECM_AE_CLASSIFIER_RESULT_NONE;
	}
}
