/*
 * Copyright (c) 2022-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

/*
 * nss_ppe_vxlanmgr_priv.h
 *	VxLAN manager header
 */
#ifndef __NSS_PPE_VXLANMGR_PRIV_H
#define __NSS_PPE_VXLANMGR_PRIV_H

#include <net/vxlan.h>
#include <nss_ppe_vxlanmgr.h>

#define NSS_PPE_VXLANMGR_DST_PORT_MIN 1
#define NSS_PPE_VXLANMGR_DST_PORT_MAX 65535

struct ppe_drv_tun_cmn_ctx;

/*
 * Compile messages for dynamic enable/disable
 */
#if defined(CONFIG_DYNAMIC_DEBUG)
#define nss_ppe_vxlanmgr_warn(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define nss_ppe_vxlanmgr_info(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define nss_ppe_vxlanmgr_trace(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else /* CONFIG_DYNAMIC_DEBUG */
/*
 * Statically compile messages at different levels
 */
#if (NSS_PPE_VXLAN_MGR_DEBUG_LEVEL < 2)
#define nss_ppe_vxlanmgr_warn(s, ...)
#else
#define nss_ppe_vxlanmgr_warn(s, ...) pr_warn("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_PPE_VXLAN_MGR_DEBUG_LEVEL < 3)
#define nss_ppe_vxlanmgr_info(s, ...)
#else
#define nss_ppe_vxlanmgr_info(s, ...)   pr_notice("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_PPE_VXLAN_MGR_DEBUG_LEVEL < 4)
#define nss_ppe_vxlanmgr_trace(s, ...)
#else
#define nss_ppe_vxlanmgr_trace(s, ...)  pr_info("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif
#endif /* CONFIG_DYNAMIC_DEBUG */

#define nss_ppe_vxlanmgr_assert(c) BUG_ON(!(c))

#define NSS_PPE_VXLAN_MGR_O_DSCP_GET(x, y) (x >> y)

/*
 * VXLAN global context.
 */
struct nss_ppe_vxlanmgr_ctx {
	struct dentry *dentry;			/* per tunnel debugfs entry */
	uint16_t nack_limit;	/* nack limit */
	spinlock_t tun_lock;			/* lock to protect the tunnel list */
};

/*
 * VXLAN remote information.
 */
struct nss_ppe_vxlanmgr_remote_info {
	struct kref mac_address_ref;			/* reference count to keep track of number of clients connected to a remote */
	union vxlan_addr remote_ip;	/* Remote IP address */
	struct net_device *nss_netdev;	/* The child/dummy netdevice */
	struct net_device *pdev;	/* The parent netdevice */
	uint32_t vp_status_nack;		/* The nack limit for the remote */
	bool bridge_joined;	/* NSS netdevice is in the bridge */
};

/*
 * VXLAN RTM_NEIGH event data.
 * RTM_NEIGH event is an irq context hence the below data-structure is used to store the information
 * so that it will be used later in the work-queue.
 */
struct nss_ppe_vxlanmgr_rtm_neigh_event_data {
	struct  net_device *parent_netdev;	/* Parent/linux netdevice of the tunnel */
	union vxlan_addr rip;			/* Remote IP address received in SWITCHDEV_VXLAN_FDB_ADD_TO_DEVICE event */
	uint8_t event;	/* SWITCHDEV_VXLAN_FDB_ADD_TO_DEVICE or SWITCHDEV_VXLAN_FDB_DEL_TO_DEVICE event */
	struct list_head rtm_event_list;	/* list to maintain the RTM events */
};

/*
 * VXLAN tunnel context for each tunnel (VXLAN remote).
 */
struct nss_ppe_vxlanmgr_tun_ctx {
	uint32_t vni;				/* vnet identifier */
	struct net_device *parent_dev;			/* tunnel netdevice pointer */
	struct dentry *dentry;			/* per tunnel debugfs entry */
	uint32_t tunnel_flags;			/* vxlan tunnel flags */
	uint16_t src_port_min;			/* minimum source port */
	uint16_t src_port_max;			/* maximum source port*/
	uint16_t dest_port;			/* destination port */
	uint8_t tos;				/* tos value */
	uint8_t ttl;				/* time to live */
	struct ppe_drv_tun_cmn_ctx *tun_hdr;	/* Outer tunnel header */
	struct nss_ppe_vxlanmgr_remote_info remote_info;	/* The remote database */
	enum nss_ppe_vxlanmgr_vp_creation vp_status;	/* VP creation status for each VXLAN tunnel */
	struct nss_ppe_vxlanmgr_ctx *vxlan_ctx;	/* VXLAN global context */
	struct hlist_node node;	/* Hash node for each VXLAN PPE tunnel */
};

/*
 * struct nss_ppe_vxlanmgr_nss_dev_priv
 *	Private structure for the child/dummy nss-netdevice.
 */
struct nss_ppe_vxlanmgr_nss_dev_priv {
	int pdev_ifindex;	/* Linux-kernel/Parent netdevice index */
};

struct net_device *nss_ppe_vxlanmgr_get_parent_netdev(struct net_device *nss_dev);
void nss_ppe_vxlanmgr_all_remotes_set_mtu(struct net_device *pdev, unsigned int mtu);
void nss_ppe_vxlanmgr_all_remotes_decap_enable(struct net_device *pdev);
void nss_ppe_vxlanmgr_all_remotes_decap_disable(struct net_device *pdev);
void nss_ppe_vxlanmgr_all_remotes_leave_bridge(struct net_device *pdev);
void nss_ppe_vxlanmgr_all_remotes_join_bridge(struct net_device *pdev);
void nss_ppe_vxlanmgr_delete_all_remotes(void);
int nss_ppe_vxlanmgr_wq_init(void);
int nss_ppe_vxlanmgr_wq_exit(void);
#endif /* __NSS_VXLANMGR_PPE_PRIV_H */
