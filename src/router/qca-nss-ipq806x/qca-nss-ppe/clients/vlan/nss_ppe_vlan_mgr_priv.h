/*
 **************************************************************************
 * Copyright (c) 2017-2018, 2020-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2024, Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef _NSS_PPE_VLAN_MGR_PRIV_H_
#define _NSS_PPE_VLAN_MGR_PRIV_H_

#if (NSS_PPE_VLAN_MGR_DEBUG_LEVEL < 1)
#define nss_ppe_vlan_mgr_assert(fmt, args...)
#else
#define nss_ppe_vlan_mgr_assert(c) BUG_ON(!(c))
#endif /* NSS_PPE_VLAN_MGR_DEBUG_LEVEL */

/*
 * Compile messages for dynamic enable/disable
 */
#if defined(CONFIG_DYNAMIC_DEBUG)
#define nss_ppe_vlan_mgr_warn(s, ...) \
		pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define nss_ppe_vlan_mgr_info(s, ...) \
		pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define nss_ppe_vlan_mgr_trace(s, ...) \
		pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#else /* CONFIG_DYNAMIC_DEBUG */
/*
 * Statically compile messages at different levels
 */
#if (NSS_PPE_VLAN_MGR_DEBUG_LEVEL < 2)
#define nss_ppe_vlan_mgr_warn(s, ...)
#else
#define nss_ppe_vlan_mgr_warn(s, ...) \
		pr_warn("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_PPE_VLAN_MGR_DEBUG_LEVEL < 3)
#define nss_ppe_vlan_mgr_info(s, ...)
#else
#define nss_ppe_vlan_mgr_info(s, ...) \
		pr_notice("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_PPE_VLAN_MGR_DEBUG_LEVEL < 4)
#define nss_ppe_vlan_mgr_trace(s, ...)
#else
#define nss_ppe_vlan_mgr_trace(s, ...) \
		pr_info("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif
#endif /* CONFIG_DYNAMIC_DEBUG */

#define NSS_PPE_VLAN_MGR_INVALID_PORT -1
#define NSS_PPE_VLAN_MGR_PORT_MAX (PPE_DRV_PORTS_MAX)
#define NSS_PPE_VLAN_MGR_TYPE_SINGLE 0	/**< Single VLAN tag in message. */
#define NSS_PPE_VLAN_MGR_TYPE_DOUBLE 1	/**< Double VLAN tag in message. */
#define NSS_PPE_VLAN_MGR_TAG_CNT(v) ((v->parent) ? NSS_PPE_VLAN_MGR_TYPE_DOUBLE : NSS_PPE_VLAN_MGR_TYPE_SINGLE)
#define NSS_PPE_VLAN_MGR_PORT_ROLE_CHANGED 1

#define NSS_PPE_VLAN_MGR_WHITESPACE		" \t\v\f\n,"
#define NSS_PPE_VLAN_MGR_VLAN_AS_VP_MAX		16

/*
 * vlan client context
 */
struct nss_ppe_vlan_mgr_context {
	int ctpid;				/* Customer TPID */
	int stpid;				/* Service TPID */
	int16_t port_role[NSS_PPE_VLAN_MGR_PORT_MAX];
						/* Role of physical ports */
	struct list_head list;			/* List of vlan private instance */
	spinlock_t lock;			/* Lock to protect vlan private instance */
	struct ctl_table_header *sys_hdr;	/* "/pro/sys/nss/vlan_client" directory */
	nss_ppe_vlan_mgr_br_vlan_cb_t vlan_over_bridge_cb;	/* CB to update bridge manager */
};

/*
 * vlan manager private structure
 */
struct nss_vlan_pvt {
	struct list_head list;			/* list head */
	struct nss_vlan_pvt *parent;		/* parent vlan instance */

	/*
	 * Fields for Linux information
	 */
	int ifindex;				/* netdev ifindex */
	int16_t port[NSS_PPE_VLAN_MGR_PORT_MAX];
						/* real physical port of this vlan */
	int32_t bond_id;			/* bond interface number, if vlan is created over bond */
	uint32_t vid;				/* vid info */
	uint32_t tpid;				/* tpid info */
	uint32_t mtu;				/* mtu info */
	uint8_t dev_addr[ETH_ALEN];		/* mac address */

	/*
	 * Fields for PPE information
	 */
	struct ppe_drv_iface *iface;		/* VLAN ppe_iface info */
	struct ppe_drv_iface *bridge_iface;	/* Bridge ppe_iface info if member of a bridge */
	uint32_t ppe_cvid;			/* ppe_cvid info */
	uint32_t ppe_svid;			/* ppe_svid info */
	struct ppe_drv_vlan_xlate_info xlate_info;
						/* XLATE info */
	struct kref ref;			/* Reference count */
	bool is_vlan_as_vp_iface;		/* is VP created for this VLAN */
	struct net_device *br_net_dev;		/* base dev of bridge VLAN netdev*/
	bool is_vlan_over_bridge;		/* VLAN interface is created over bridge */
};
#endif
