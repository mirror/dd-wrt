/*
 **************************************************************************
 * Copyright (c) 2022, 2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <linux/version.h>
#include <linux/types.h>
#include <linux/ip.h>
#include <net/ipv6.h>
#include <linux/tcp.h>
#include <linux/module.h>
#include <linux/inet.h>
#include <linux/etherdevice.h>
#include <net/netfilter/nf_conntrack_l4proto.h>

#define DEBUG_LEVEL ECM_PPE_COMMON_DEBUG_LEVEL

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
#include "ecm_interface.h"
#include "ecm_ppe_common.h"
#include "ecm_front_end_common.h"
#include "ecm_ppe_ipv6.h"
#include "ecm_ppe_ipv4.h"
#ifdef ECM_INTERFACE_VXLAN_ENABLE
#include <net/vxlan.h>
#endif

#ifdef ECM_OPENWRT_SUPPORT
extern int nf_ct_tcp_no_window_check;
#endif

#ifdef ECM_IPV6_ENABLE
/*
 * ecm_ppe_ipv6_is_conn_limit_reached()
 *	Connection limit is reached or not ?
 */
bool ecm_ppe_ipv6_is_conn_limit_reached(void)
{
#if !defined(ECM_FRONT_END_CONN_LIMIT_ENABLE)
	return false;
#endif

	if (likely(!((ecm_front_end_is_feature_supported(ECM_FE_FEATURE_CONN_LIMIT)) && ecm_front_end_conn_limit))) {
		return false;
	}

	spin_lock_bh(&ecm_ppe_ipv6_lock);
	if (ecm_ppe_ipv6_accelerated_count == PPE_DRV_V6_MAX_CONN_COUNT) {
		spin_unlock_bh(&ecm_ppe_ipv6_lock);
		DEBUG_INFO("ECM DB connection limit %d reached, for PPE frontend \
			   new flows cannot be accelerated.\n",
			   ecm_ppe_ipv6_accelerated_count);
		return true;
	}
	spin_unlock_bh(&ecm_ppe_ipv6_lock);
	return false;
}
#endif

/*
 * ecm_ppe_ipv4_is_conn_limit_reached()
 *	Connection limit is reached or not ?
 */
bool ecm_ppe_ipv4_is_conn_limit_reached(void)
{
#if !defined(ECM_FRONT_END_CONN_LIMIT_ENABLE)
	return false;
#endif

	if (likely(!((ecm_front_end_is_feature_supported(ECM_FE_FEATURE_CONN_LIMIT)) && ecm_front_end_conn_limit))) {
		return false;
	}

	spin_lock_bh(&ecm_ppe_ipv4_lock);
	if (ecm_ppe_ipv4_accelerated_count == PPE_DRV_V4_MAX_CONN_COUNT) {
		spin_unlock_bh(&ecm_ppe_ipv4_lock);
		DEBUG_INFO("ECM DB connection limit %d reached, for PPE frontend \
			   new flows cannot be accelerated.\n",
			   ecm_ppe_ipv4_accelerated_count);
		return true;
	}
	spin_unlock_bh(&ecm_ppe_ipv4_lock);
	return false;
}

#ifdef ECM_INTERFACE_VXLAN_ENABLE
/*
 * ecm_ppe_ported_get_vxlan_ppe_dev_index()
 *	Check and get whether the given ecm iface instance of type VXLAN virtual port is created in PPE.
 */
int ecm_ppe_ported_get_vxlan_ppe_dev_index(struct ecm_front_end_connection_instance *feci, struct ecm_db_iface_instance *ii,
											ecm_db_obj_dir_t dir, enum nss_ppe_vxlanmgr_vp_creation *vp_status)
{
	int if_index = -1;
	uint32_t remote_ip[4] = {0};
	struct ecm_db_interface_info_vxlan vxlan_info = {0};
	struct net_device *dev;
	struct vxlan_config *cfg;
	struct vxlan_dev *priv;
	union vxlan_addr *src_ip;
	uint8_t ip_type;

	dev = dev_get_by_index(&init_net, ecm_db_iface_interface_identifier_get(ii));
	if (!dev) {
		DEBUG_TRACE("%px: VXLAN: could not get the dev", feci);
		return -1;
	}

	priv = netdev_priv(dev);
	cfg = &priv->cfg;
	src_ip = &cfg->saddr;
	if (src_ip->sa.sa_family == AF_INET) {
		ip_type = AF_INET;
	} else {
		ip_type = AF_INET6;
	}

	ecm_db_iface_vxlan_info_get(ii, &vxlan_info);
	if (!vxlan_info.if_type) {
		ip_addr_t addr = {0};

		DEBUG_TRACE("%px: VXLAN: It is an outer rule", feci);
		ecm_db_connection_address_get(feci->ci, ECM_DB_OBJ_DIR_TO_NAT, addr);
		if (ip_type == AF_INET) {
			ECM_IP_ADDR_TO_NIN4_ADDR(remote_ip[0], addr);
		} else {
			uint32_t temp[4] = {0};

			ECM_IP_ADDR_TO_PPE_IPV6_ADDR(temp, addr);
			remote_ip[0] = ntohl(temp[0]);
			remote_ip[1] = ntohl(temp[1]);
			remote_ip[2] = ntohl(temp[2]);
			remote_ip[3] = ntohl(temp[3]);
		}
	} else {
		uint8_t mac_addr[ETH_ALEN] = {0};
		union vxlan_addr rip = {0};

		DEBUG_TRACE("%px: VXLAN: It is an inner rule", feci);
		ecm_db_connection_node_address_get(feci->ci, dir, mac_addr);
		if (vxlan_find_remote_ip(priv, mac_addr, priv->cfg.vni, &rip) < 0) {
			DEBUG_WARN("%px: VXLAN: failed to get the remote IP from kernel", feci);
			dev_put(dev);
			return -1;
		}

		if (ip_type == AF_INET) {
			remote_ip[0] = rip.sin.sin_addr.s_addr;
			DEBUG_TRACE("%px: VXLAN: rip:%pI4h", feci, remote_ip);
		} else {
			memcpy(remote_ip, &rip.sin6.sin6_addr, sizeof(struct in6_addr));
			DEBUG_TRACE("%px: VXLAN: rip:%pI6h", feci, &remote_ip[0]);
		}
	}

	*vp_status = nss_ppe_vxlanmgr_get_ifindex_and_vp_status(dev, remote_ip, ip_type, &if_index);

	DEBUG_TRACE("%px: VXLAN: netdev:%s if_index:%d vp_status:%u\n", feci,dev->name, if_index, *vp_status);

	dev_put(dev);

	return if_index;
}
#endif

/*
 * ecm_ppe_feature_check()
 *	Check some specific features for PPE acceleration
 */
bool ecm_ppe_feature_check(struct sk_buff *skb, struct ecm_tracker_ip_header *ip_hdr)
{
	bool inner = 0;

#ifdef ECM_OPENWRT_SUPPORT
	if (ip_hdr->protocol == IPPROTO_TCP) {
#if 1 //(LINUX_VERSION_CODE < KERNEL_VERSION(5, 13, 0))
		uint32_t tcp_no_window_check = nf_ct_tcp_no_window_check;
#else
		struct nf_conn *ct;
		enum ip_conntrack_info ctinfo;
		struct nf_tcp_net *tn;
		uint32_t tcp_no_window_check;

		ct = nf_ct_get(skb, &ctinfo);

		/*
		 * If there is no ct, we shouldn't care about the conntrack's TCP window check.
		 * Returning false here will break OVS bridge flow acceleration in PPE, since OVS bridge
		 * flows do not have ct.
		 */
		if (unlikely(!ct)) {
			DEBUG_TRACE("%px: No Conntrack found for packet\n", skb);
			return true;
		}

		tn = nf_tcp_pernet(nf_ct_net(ct));
		tcp_no_window_check = tn->tcp_no_window_check;
#endif
		if(unlikely(tcp_no_window_check == 0)) {
			DEBUG_TRACE("%px: TCP window check feature not supported for PPE; skip it\n", skb);
			return false;
		}
	}
#endif

	if (ecm_front_end_is_xfrm_flow(skb, ip_hdr, &inner)) {
#ifdef ECM_XFRM_ENABLE
		struct net_device *ipsec_dev;
		int32_t interface_type;

		/*
		 * Dont accelerate inner flow.
		 */
		if (inner) {
			DEBUG_TRACE("%px xfrm inner flow is not supported for PPE; skip it\n", skb);
			return false;
		}

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
