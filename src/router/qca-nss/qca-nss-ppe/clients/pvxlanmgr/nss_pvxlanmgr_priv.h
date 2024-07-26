/*
 **************************************************************************
 * Copyright (c) 2019-2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

/*
 * nss_pvxlanmgr_priv.h
 *	Pvxlan manager private defines
 */
#include <linux/types.h>
#include <linux/of.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>

#include <nss_api_if.h>
#include <nss_cmn.h>

#include <ppe_drv.h>
#include <ppe_drv_v4.h>
#include <ppe_drv_v6.h>
#include <nss_dp_api_if.h>
#include <ppe_vp_public.h>
#include <ppe_drv_iface.h>
#include <ppe_acl.h>
#include <nss_pvxlan.h>
#include <nss_pvxlanmgr.h>

#define NSS_PVXLANMGR_NETDEV_NAME "nsspvxlan"

#if (NSS_PVXLANMGR_DEBUG_LEVEL < 1)
#define nss_pvxlanmgr_assert(fmt, args...)
#else
#define nss_pvxlanmgr_assert(c) if (!(c)) { BUG_ON(!(c)); }
#endif /* NSS_PVXLANMGR_DEBUG_LEVEL */

/*
 * Compile messages for dynamic enable/disable
 */
#if defined(CONFIG_DYNAMIC_DEBUG)
#define nss_pvxlanmgr_warn(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define nss_pvxlanmgr_info(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define nss_pvxlanmgr_trace(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else /* CONFIG_DYNAMIC_DEBUG */
/*
 * Statically compile messages at different levels
 */
#if (NSS_PVXLANMGR_DEBUG_LEVEL < 2)
#define nss_pvxlanmgr_warn(s, ...)
#else
#define nss_pvxlanmgr_warn(s, ...) pr_warn("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_PVXLANMGR_DEBUG_LEVEL < 3)
#define nss_pvxlanmgr_info(s, ...)
#else
#define nss_pvxlanmgr_info(s, ...)   pr_notice("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_PVXLANMGR_DEBUG_LEVEL < 4)
#define nss_pvxlanmgr_trace(s, ...)
#else
#define nss_pvxlanmgr_trace(s, ...)  pr_info("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif
#endif /* CONFIG_DYNAMIC_DEBUG */

/*
 * ACL port mask to match VxLAN port number.
 */
#define NSS_PVXLAN_ACL_PORT_MASK 0xffff

/*
 * nss_pvxlanmgr_tunnel
 *	Mapping table from tunnel-id to rule.
 */
struct nss_pvxlanmgr_tunnel {
	uint32_t tunnel_state;			/* Tunnel state. */
	union {
		struct nss_ipv4_rule_create_msg v4;	/* IPv4 rule structure. */
		struct nss_ipv6_rule_create_msg v6;	/* IPv6 rule struture. */
	} ip_rule;
	struct nss_pvxlan_rule_msg pvxlan_rule;	/* Copy of PVXLAN rule. */
	ppe_acl_rule_id_t acl_rule_id;		/* ACL rule id. */
};

/*
 * nss_pvxlanmgr_priv
 *	Private structure to store information needed by a nss_pvxlan net_device
 */
struct nss_pvxlanmgr_priv {
	struct nss_ctx_instance *pvxlan_ctx;	/* Pointer to NSS PVxLAN context. */
	struct nss_pvxlanmgr_tunnel *tunnel;	/* Pointer to tunnel data. */
	struct nss_pvxlanmgr_response *resp;	/* Response housekeeping. */
	struct rtnl_link_stats64 stats;		/* NetDev stats. */
	uint32_t if_num_host_inner;		/* Pvxlan inner interface number in NSS. */
	uint32_t if_num_outer;			/* Pvxlan outer interface number in NSS. */
	ppe_vp_num_t vp_num;			/* VP number associated with the tunnel. */
};

/*
 * Returns tunnel structure pointer from tunnel_id.
 */
struct nss_pvxlanmgr_tunnel *nss_pvxlanmgr_tunnel_get(struct net_device *dev, uint32_t tunnel_id);

nss_tx_status_t nss_pvxlanmgr_tunnel_tx_msg_disable(struct nss_ctx_instance *ctx, uint32_t if_num);

nss_tx_status_t nss_pvxlanmgr_tunnel_tx_msg_enable(struct nss_ctx_instance *ctx,
						uint32_t if_num, uint32_t sibling_if_num);
nss_tx_status_t nss_pvxlanmgr_tunnel_config_vp(struct nss_ctx_instance *ctx,
				uint32_t if_num, ppe_vp_num_t vp_num, bool ppe_to_host);
nss_tx_status_t nss_pvxlanmgr_tunnel_unconfig_vp(struct nss_ctx_instance *ctx,
				uint32_t if_num, ppe_vp_num_t vp_num);
