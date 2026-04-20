/*
 **************************************************************************
 * Copyright (c) 2017-2020 The Linux Foundation. All rights reserved.
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

/*
 * nss_connnmgr_gre.c
 *  This file implements client for GRE.
 */

#include <linux/version.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <net/ip_tunnels.h>
#include <net/ip6_tunnel.h>
#include <linux/if_ether.h>
#include <net/gre.h>
#include <net/ip.h>

#include <nss_api_if.h>
#include <nss_dynamic_interface.h>
#include <nss_cmn.h>
#include "nss_connmgr_gre_public.h"
#include "nss_connmgr_gre.h"

#define MAX_RETRY_COUNT 100
#define MAX_WIFI_HEADROOM 66

/*
 * GRE connection manager context structure
 */
struct nss_connmgr_gre_context {
	struct list_head list;		/* List of GRE interface instances */
	spinlock_t lock;		/* Lock to protect list */
} gre_connmgr_ctx;

/*
 * GRE interface instance
 */
struct nss_gre_iface_instance {
	struct list_head list;			/* List of GRE interface instances */
	struct net_device *dev;			/* GRE netdevice */
	struct nss_connmgr_gre_cfg gre_cfg;	/* GRE configuration */
	enum nss_connmgr_gre_iftype gre_iftype;	/* GRE interface type */
	uint32_t inner_ifnum;			/* GRE inner dynamic interface */
	uint32_t outer_ifnum;			/* GRE outer dynamic interface */
};

/*
 * Unaligned infra in nss is disabled by default
 */
static bool enable_unalign = 1;
module_param(enable_unalign, bool, 0);

/*
 * nss_connmgr_gre_is_gre()
 *	Check whether device is of type GRE Tap or GRE Tun.
 */
static bool nss_connmgr_gre_is_gre(struct net_device *dev)
{
	if ((dev->type == ARPHRD_IPGRE) ||
	      (dev->type == ARPHRD_IP6GRE) || ((dev->type == ARPHRD_ETHER) &&
	      (dev->priv_flags_ext & (IFF_EXT_GRE_V4_TAP | IFF_EXT_GRE_V6_TAP)))) {
		return true;
	}

	return false;
}

/*
 * nss_connmgr_gre_alloc_instance()
 *	Allocate GRE interface instance.
 */
static struct nss_gre_iface_instance *nss_connmgr_gre_alloc_instance(struct net_device *dev)
{
	struct nss_gre_iface_instance *ngii;

	if (!nss_connmgr_gre_is_gre(dev)) {
		nss_connmgr_gre_warning("%px: dev is not a GRE interface\n", dev);
		return NULL;
	}

	ngii = kzalloc(sizeof(*ngii), GFP_KERNEL);
	if (!ngii)
		return NULL;

	INIT_LIST_HEAD(&ngii->list);
	ngii->dev = dev;
	return ngii;
}

/*
 * nss_connmgr_gre_free_instance()
 *	Free GRE interface instance.
 */
static void nss_connmgr_gre_free_instance(struct nss_gre_iface_instance *ngii)
{
	spin_lock(&gre_connmgr_ctx.lock);
	ngii->dev = NULL;

	if (!list_empty(&ngii->list))
		list_del(&ngii->list);

	spin_unlock(&gre_connmgr_ctx.lock);
	kfree(ngii);
}

/*
 * nss_connmgr_gre_find_instance()
 *	Find GRE interface instance from list.
 */
static struct nss_gre_iface_instance *nss_connmgr_gre_find_instance(struct net_device *dev)
{
	struct nss_gre_iface_instance *ngii;

	if (!nss_connmgr_gre_is_gre(dev)) {
		nss_connmgr_gre_warning("%px: dev is not a GRE interface\n", dev);
		return NULL;
	}

	/*
	 * Check if dev instance is in the list
	 */
	spin_lock(&gre_connmgr_ctx.lock);
	list_for_each_entry(ngii, &gre_connmgr_ctx.list, list) {
		if (ngii->dev == dev) {
			spin_unlock(&gre_connmgr_ctx.lock);
			return ngii;
		}
	}

	spin_unlock(&gre_connmgr_ctx.lock);
	return NULL;
}

/*
 * nss_connmgr_gre_dev_change_mtu()
 *	Netdev ops function to modify MTU of netdevice.
 */
static int nss_connmgr_gre_dev_change_mtu(struct net_device *dev, int new_mtu)
{
	if (new_mtu < 68 ||
	    new_mtu > 0xFFF8 - dev->needed_headroom) {
		return -EINVAL;
	}

	dev->mtu = new_mtu;
	return 0;
}

/*
 * nss_connmgr_gre_dev_init()
 *	Netdev ops function to intialize netdevice.
 */
static int nss_connmgr_gre_dev_init(struct net_device *dev)
{
	int i;
	nss_connmgr_gre_priv_t *priv = netdev_priv(dev);
	int32_t append = priv->pad_len + priv->gre_hlen;

	dev->tstats = alloc_percpu(struct pcpu_sw_netstats);
	if (!dev->tstats) {
		return -ENOMEM;
	}

	for_each_possible_cpu(i) {
		struct pcpu_sw_netstats *stats;
		stats = per_cpu_ptr(dev->tstats, i);
		u64_stats_init(&stats->syncp);
	}

	if ((dev->priv_flags_ext & IFF_EXT_GRE_V4_TAP) || (dev->type == ARPHRD_IPGRE)) {
		dev->needed_headroom = sizeof(struct iphdr) + sizeof(struct ethhdr) + MAX_WIFI_HEADROOM + append;
		dev->mtu = ETH_DATA_LEN - sizeof(struct iphdr) - append;
		dev->features |= NETIF_F_NETNS_LOCAL | NETIF_F_SG | NETIF_F_FRAGLIST | NETIF_F_HIGHDMA;
		dev->hw_features |= NETIF_F_SG | NETIF_F_FRAGLIST | NETIF_F_HIGHDMA;
		return 0;
	}

	/*
	 * Ipv6
	 */
	dev->needed_headroom = sizeof(struct ipv6hdr) + sizeof(struct ethhdr) + MAX_WIFI_HEADROOM + append;
	dev->mtu = ETH_DATA_LEN - sizeof(struct ipv6hdr) - append;
	if (dev->mtu < IPV6_MIN_MTU) {
		dev->mtu = IPV6_MIN_MTU;
	}

	dev->features |= NETIF_F_NETNS_LOCAL;
	return 0;
}

/*
 * nss_connmgr_gre_dev_uninit()
 *	Netdev ops function to unintialize netdevice.
 */
static void nss_connmgr_gre_dev_uninit(struct net_device *dev)
{
	free_percpu(dev->tstats);
	return;
}

/*
 * nss_connmgr_gre_dev_xmit()
 *	Netdev ops function to send packet to NSS.
 */
static netdev_tx_t nss_connmgr_gre_dev_xmit(struct sk_buff *skb, struct net_device *dev)
{
	nss_tx_status_t status;
	int if_number;
	struct nss_ctx_instance *gre_ctx;
	nss_connmgr_gre_priv_t *priv = netdev_priv(dev);

	if_number = priv->nss_if_number_inner;
	if (unlikely(if_number <= 0)) {
		nss_connmgr_gre_info("%px: GRE dev is not registered with nss\n", dev);
		goto fail;
	}

	gre_ctx = nss_gre_get_context();
	if (unlikely(!gre_ctx)) {
		nss_connmgr_gre_info("%px: NSS GRE context not found for if_number %d\n", dev, if_number);
		goto fail;
	}

	/*
	 * Make room for needed headroom and un-share
	 * the SKB if it is cloned.
	 */
	if (skb_cow_head(skb, dev->needed_headroom)) {
		nss_connmgr_gre_info("%px: NSS GRE insufficient headroom\n", dev);
		goto fail;
	}

	status = nss_gre_tx_buf(gre_ctx, if_number, skb);
	if (unlikely(status != NSS_TX_SUCCESS)) {
		nss_connmgr_gre_info("%px: NSS GRE could not send packet to NSS %d\n", dev, if_number);
		goto fail;
	}

	return NETDEV_TX_OK;

fail:
	dev->stats.tx_dropped++;
	dev_kfree_skb_any(skb);
	return NETDEV_TX_OK;
}

/*
 * nss_connmgr_gre_get_dev_stats64()
 *	To get the netdev stats
 */
static struct rtnl_link_stats64 *nss_connmgr_gre_get_dev_stats64(struct net_device *dev,
						struct rtnl_link_stats64 *tot)
{
	uint64_t rx_packets, rx_bytes, tx_packets, tx_bytes;
	unsigned int start;
	int i;
	for_each_possible_cpu(i) {
		const struct pcpu_sw_netstats *tstats = per_cpu_ptr(dev->tstats, i);

		do {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 15, 0))
			start = u64_stats_fetch_begin_bh(&tstats->syncp);
#else
			start = u64_stats_fetch_begin(&tstats->syncp);
#endif
			rx_packets = u64_stats_read(&tstats->rx_packets);
			tx_packets = u64_stats_read(&tstats->tx_packets);
			rx_bytes = u64_stats_read(&tstats->rx_bytes);
			tx_bytes = u64_stats_read(&tstats->tx_bytes);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 15, 0))
		} while (u64_stats_fetch_retry_bh(&tstats->syncp, start));
#else
		} while (u64_stats_fetch_retry(&tstats->syncp, start));
#endif

		tot->rx_packets += rx_packets;
		tot->tx_packets += tx_packets;
		tot->rx_bytes   += rx_bytes;
		tot->tx_bytes   += tx_bytes;

		tot->rx_dropped = dev->stats.rx_dropped;
		tot->tx_dropped = dev->stats.tx_dropped;
	}

	return tot;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 6, 0))
/*
 * nss_connmgr_gre_dev_stats64()
 *	Netdev ops function to retrieve stats for kernel version < 4.6
 */
static struct rtnl_link_stats64 *nss_connmgr_gre_dev_stats64(struct net_device *dev,
						struct rtnl_link_stats64 *tot)
{
	return nss_connmgr_gre_get_dev_stats64(dev, tot);
}
#else
/*
 * nss_connmgr_gre_dev_stats64()
 *	Netdev ops function to retrieve stats
 */
static void nss_connmgr_gre_dev_stats64(struct net_device *dev,
						struct rtnl_link_stats64 *tot)
{
	nss_connmgr_gre_get_dev_stats64(dev, tot);
}
#endif

/*
 * nss_connmgr_gre_dev_open()
 *	Netdev ops function to open netdevice.
 */
int nss_connmgr_gre_dev_open(struct net_device *dev)
{
	struct nss_ctx_instance *nss_ctx;
	struct nss_gre_msg req;
	struct nss_gre_linkup_msg *linkup = &req.msg.linkup;
	struct nss_gre_linkdown_msg *linkdown = &req.msg.linkdown;
	nss_tx_status_t status;
	int32_t inner_if, outer_if;

	inner_if = nss_cmn_get_interface_number_by_dev_and_type(dev, NSS_DYNAMIC_INTERFACE_TYPE_GRE_INNER);
	if (inner_if < 0) {
		return -EINVAL;
	}

	outer_if = nss_cmn_get_interface_number_by_dev_and_type(dev, NSS_DYNAMIC_INTERFACE_TYPE_GRE_OUTER);
	if (outer_if < 0) {
		return -EINVAL;
	}

	nss_ctx = nss_gre_get_context();

	memset(&req, 0, sizeof(struct nss_gre_msg));

	/*
	 * Open inner interface
	 */
	linkup->if_number = inner_if;
	nss_gre_msg_init(&req, inner_if, NSS_IF_OPEN, sizeof(struct nss_gre_linkup_msg), NULL, NULL);

	status = nss_gre_tx_msg_sync(nss_ctx, &req);
	if (status != NSS_TX_SUCCESS) {
		nss_connmgr_gre_info("%px: open failed for inner interface %s", dev, dev->name);
		return -EFAULT;
	}

	/*
	 * Open outer interface
	 */
	linkup->if_number = outer_if;
	nss_gre_msg_init(&req, outer_if, NSS_IF_OPEN, sizeof(struct nss_gre_linkup_msg), NULL, NULL);

	status = nss_gre_tx_msg_sync(nss_ctx, &req);
	if (status != NSS_TX_SUCCESS) {
		nss_connmgr_gre_info("%px: open failed for outer interface %s", dev, dev->name);
		linkdown->if_number = inner_if;
		nss_gre_msg_init(&req, inner_if, NSS_IF_CLOSE, sizeof(struct nss_gre_linkdown_msg), NULL, NULL);
		status = nss_gre_tx_msg_sync(nss_ctx, &req);
		if (status != NSS_TX_SUCCESS) {
			nss_connmgr_gre_info("%px: close failed for inner interface %s", dev, dev->name);
		}
		return -EFAULT;
	}

	netif_start_queue(dev);
	return 0;
}
EXPORT_SYMBOL(nss_connmgr_gre_dev_open);

/*
 * nss_connmgr_gre_dev_close()
 *	Netdevice ops function to close netdevice.
 */
int nss_connmgr_gre_dev_close(struct net_device *dev)
{
	struct nss_ctx_instance *nss_ctx;
	struct nss_gre_msg req;
	struct nss_gre_linkdown_msg *linkdown = &req.msg.linkdown;
	struct nss_gre_linkup_msg *linkup = &req.msg.linkup;
	nss_tx_status_t status;
	int32_t inner_if, outer_if;

	netif_stop_queue(dev);

	inner_if = nss_cmn_get_interface_number_by_dev_and_type(dev, NSS_DYNAMIC_INTERFACE_TYPE_GRE_INNER);
	if (inner_if < 0) {
		nss_connmgr_gre_info("%px: close failed for interface %s, inner interface: %d not valid", dev, dev->name, inner_if);
		return -EINVAL;
	}

	outer_if = nss_cmn_get_interface_number_by_dev_and_type(dev, NSS_DYNAMIC_INTERFACE_TYPE_GRE_OUTER);
	if (outer_if < 0) {
		nss_connmgr_gre_info("%px: close failed for interface %s, outer interface: %d not valid", dev, dev->name, inner_if);
		return -EINVAL;
	}

	nss_ctx = nss_gre_get_context();

	memset(&req, 0, sizeof(struct nss_gre_msg));

	/*
	 * Close inner interface
	 */
	linkdown->if_number = inner_if;
	nss_gre_msg_init(&req, inner_if, NSS_IF_CLOSE, sizeof(struct nss_gre_linkdown_msg), NULL, NULL);

	status = nss_gre_tx_msg_sync(nss_ctx, &req);
	if (status != NSS_TX_SUCCESS) {
		nss_connmgr_gre_info("%px: close failed for inner interface %s", dev, dev->name);
		return -EFAULT;
	}

	/*
	 * Close outer interface
	 */
	linkdown->if_number = outer_if;
	nss_gre_msg_init(&req, outer_if, NSS_IF_CLOSE, sizeof(struct nss_gre_linkdown_msg), NULL, NULL);

	status = nss_gre_tx_msg_sync(nss_ctx, &req);
	if (status != NSS_TX_SUCCESS) {
		nss_connmgr_gre_info("%px: close failed for outer interface %s", dev, dev->name);
		linkup->if_number = inner_if;
		nss_gre_msg_init(&req, inner_if, NSS_IF_OPEN, sizeof(struct nss_gre_linkup_msg), NULL, NULL);
		status = nss_gre_tx_msg_sync(nss_ctx, &req);
		if (status != NSS_TX_SUCCESS) {
			nss_connmgr_gre_info("%px: open failed for inner interface %s", dev, dev->name);
		}
		return -EFAULT;
	}

	return 0;
}
EXPORT_SYMBOL(nss_connmgr_gre_dev_close);

/*
 * Tap net device ops
 */
static const struct net_device_ops nss_connmgr_gre_tap_ops = {
	.ndo_init = nss_connmgr_gre_dev_init,
	.ndo_uninit = nss_connmgr_gre_dev_uninit,
	.ndo_open = nss_connmgr_gre_dev_open,
	.ndo_stop = nss_connmgr_gre_dev_close,
	.ndo_set_mac_address = eth_mac_addr,
	.ndo_validate_addr = eth_validate_addr,
	.ndo_change_mtu	= nss_connmgr_gre_dev_change_mtu,
	.ndo_start_xmit = nss_connmgr_gre_dev_xmit,
	.ndo_get_stats64 = nss_connmgr_gre_dev_stats64,
};

/*
 * Tun netdevice ops
 */
static const struct net_device_ops nss_connmgr_gre_tun_ops = {
	.ndo_init = nss_connmgr_gre_dev_init,
	.ndo_uninit = nss_connmgr_gre_dev_uninit,
	.ndo_open = nss_connmgr_gre_dev_open,
	.ndo_stop = nss_connmgr_gre_dev_close,
	.ndo_change_mtu	= nss_connmgr_gre_dev_change_mtu,
	.ndo_start_xmit = nss_connmgr_gre_dev_xmit,
	.ndo_get_stats64 = nss_connmgr_gre_dev_stats64,
};

/*
 * nss_connmgr_gre_tun_setup()
 */
static void nss_connmgr_gre_tun_setup(struct net_device *dev)
{
	dev->addr_len = 4;
	dev->flags = IFF_NOARP | IFF_POINTOPOINT;
	dev->priv_flags	&= ~IFF_XMIT_DST_RELEASE;
	dev->netdev_ops = &nss_connmgr_gre_tun_ops;
}

/*
 * nss_connmgr_gre_tap_setup()
 */
static void nss_connmgr_gre_tap_setup(struct net_device *dev)
{
	dev->netdev_ops = &nss_connmgr_gre_tap_ops;
	eth_hw_addr_random(dev);
}

/*
 * nss_connmgr_gre_prepare_config_cmd()
 *	Retrieve info from netdevie and fill it in config message to NSS.
 */
static int32_t nss_connmgr_gre_prepare_config_cmd(struct net_device *dev,
						      struct nss_gre_msg *req,
						      struct net_device **next_dev,
						      bool hold)
{
	struct nss_gre_config_msg *cmsg = &req->msg.cmsg;

	if ((dev->type == ARPHRD_ETHER) && (dev->priv_flags_ext & IFF_EXT_GRE_V4_TAP)) {
		cmsg->mode = NSS_GRE_MODE_TAP;
		cmsg->ip_type = NSS_GRE_IP_IPV4;
		if (enable_unalign) {
			cmsg->flags |= NSS_GRE_CONFIG_USE_UNALIGNED;
		}
		return nss_connmgr_gre_v4_get_config(dev, req, next_dev, hold);
	}

	if ((dev->type == ARPHRD_ETHER) && (dev->priv_flags_ext & IFF_EXT_GRE_V6_TAP)) {
		cmsg->mode = NSS_GRE_MODE_TAP;
		cmsg->ip_type = NSS_GRE_IP_IPV6;
		if (enable_unalign) {
			cmsg->flags |= NSS_GRE_CONFIG_USE_UNALIGNED;
		}
		return nss_connmgr_gre_v6_get_config(dev, req, next_dev, hold);
	}

	if (dev->type == ARPHRD_IPGRE) {
		cmsg->mode = NSS_GRE_MODE_TUN;
		cmsg->ip_type = NSS_GRE_IP_IPV4;
		return nss_connmgr_gre_v4_get_config(dev, req, next_dev, hold);
	}

	if (dev->type == ARPHRD_IP6GRE) {
		cmsg->mode = NSS_GRE_MODE_TUN;
		cmsg->ip_type = NSS_GRE_IP_IPV6;
		return nss_connmgr_gre_v6_get_config(dev, req, next_dev, hold);
	}

	return GRE_ERR_NOT_GRE_NETDEV;
}

/*
 * nss_connmgr_gre_tap_inner_exception()
 * 	Exception handler for GRETAP inner device
 *
 * These are the packets exceptioned after decapsulation,
 * and we can send the packet to linux after modifying
 * some skb fields.
 */
static void nss_connmgr_gre_tap_inner_exception(struct net_device *dev, struct sk_buff *skb,
					  __attribute__((unused)) struct napi_struct *napi)
{

	struct ethhdr *eth_hdr = (struct ethhdr *)skb->data;

	nss_connmgr_gre_trace("%px: eth_hdr->h_proto: %d\n", dev, eth_hdr->h_proto);

	if (likely(ntohs(eth_hdr->h_proto) >= ETH_P_802_3_MIN)) {
		switch (ntohs(eth_hdr->h_proto)) {
		case ETH_P_IP:
		case ETH_P_IPV6:
			/*
		 	 * These are decapped packets.
		 	 */
			skb->protocol = eth_type_trans(skb, dev);
			netif_receive_skb(skb);
			return;
		default:
			break;
		}
	}

	/*
	 * These are decapped and exceptioned non IP packets.
	 */
	skb->protocol = eth_type_trans(skb, dev);
	netif_receive_skb(skb);
	return;
}

/*
 * nss_connmgr_gre_tap_outer_exception()
 * 	Exception handler for GRETAP outer device
 *
 * These are the packets exceptioned after encapsulation,
 * and we need to remove the GRE header of the packet and
 * pass it to linux.
 */
static void nss_connmgr_gre_tap_outer_exception(struct net_device *dev, struct sk_buff *skb,
					  __attribute__((unused)) struct napi_struct *napi)
{

	struct ethhdr *eth_hdr;

	eth_hdr = (struct ethhdr *)skb->data;
	nss_connmgr_gre_trace("%px: eth_hdr->h_proto: %d\n", dev, eth_hdr->h_proto);
	if (likely(ntohs(eth_hdr->h_proto) >= ETH_P_802_3_MIN)) {
		switch (ntohs(eth_hdr->h_proto)) {
		case ETH_P_IP:
			return nss_connmgr_gre_tap_v4_outer_exception(dev, skb);
		case ETH_P_IPV6:
			return nss_connmgr_gre_tap_v6_outer_exception(dev, skb);
		default:
			nss_connmgr_gre_warning("%px: invalid skb received:%px with protocol: %d. Freeing the skb.\n", dev, skb, ntohs(eth_hdr->h_proto));
			dev_kfree_skb_any(skb);
		}
	}
}

/*
 * nss_connmgr_gre_tun_inner_exception()
 * 	Exception handler for GRETUN inner device
 *
 * These are the packets exceptioned after decapsulation,
 * and we can send the packet to linux after modifying
 * some skb fields.
 */
static void nss_connmgr_gre_tun_inner_exception(struct net_device *dev, struct sk_buff *skb,
					  __attribute__((unused)) struct napi_struct *napi)
{
	struct iphdr *iph;

	iph = (struct iphdr *)skb->data;
	switch (iph->version) {
	case 4:
		skb->protocol = htons(ETH_P_IP);
		break;
	case 6:
		skb->protocol = htons(ETH_P_IPV6);
		break;
	default:
		nss_connmgr_gre_warning("%px: wrong IP version set to skb:%px\n", dev, skb);
		dev_kfree_skb_any(skb);
		return;
	}

	skb->pkt_type = PACKET_HOST;
	skb->dev = dev;
	netif_receive_skb(skb);
}

/*
 * nss_connmgr_gre_tun_outer_exception()
 * 	Exception handler for GRETUN outer device
 *
 * These are the packets exceptioned after encapsulation,
 * and we need to remove the GRE header of the packet and
 * pass it to linux.
 */
static void nss_connmgr_gre_tun_outer_exception(struct net_device *dev, struct sk_buff *skb,
					  __attribute__((unused)) struct napi_struct *napi)
{
	struct iphdr *iph;

	iph = (struct iphdr *)skb->data;
	switch (iph->version) {
	case 4:
		return nss_connmgr_gre_tun_v4_outer_exception(dev, skb);
	case 6:
		return nss_connmgr_gre_tun_v6_outer_exception(dev, skb);
	default:
		nss_connmgr_gre_warning("%px: wrong IP version set to skb:%px\n", dev, skb);
		dev_kfree_skb_any(skb);
		break;
	}
}

/*
 * nss_connmgr_gre_rx_pkt()
 *	GRE Tap function to receive packet from NSS.
 */
static void nss_connmgr_gre_rx_pkt(struct net_device *dev, struct sk_buff *skb,
				       __attribute__((unused)) struct napi_struct *napi)
{
	skb->dev = dev;
	skb->protocol = eth_type_trans(skb, dev);
	netif_receive_skb(skb);
}

/*
 * nss_connmgr_gre_event_receive()
 *	Event Callback to receive events from NSS
 */
static void nss_connmgr_gre_event_receive(void *if_ctx, struct nss_gre_msg *tnlmsg)
{
	struct net_device *dev = if_ctx;
	struct nss_cmn_node_stats *stats = &tnlmsg->msg.sstats.node_stats;
	struct pcpu_sw_netstats *tstats;
	enum nss_dynamic_interface_type interface_type;

	switch (tnlmsg->cm.type) {
	case NSS_GRE_MSG_SESSION_STATS:
		interface_type = nss_dynamic_interface_get_type(nss_gre_get_context(), tnlmsg->cm.interface);
		tstats = this_cpu_ptr(dev->tstats);
		u64_stats_update_begin(&tstats->syncp);
		if (interface_type == NSS_DYNAMIC_INTERFACE_TYPE_GRE_INNER) {
			u64_stats_add(&tstats->tx_packets, stats->tx_packets);
			u64_stats_add(&tstats->tx_bytes, stats->tx_bytes);
		} else if (interface_type == NSS_DYNAMIC_INTERFACE_TYPE_GRE_OUTER) {
			u64_stats_add(&tstats->rx_packets, stats->rx_packets);
			u64_stats_add(&tstats->rx_bytes, stats->rx_bytes);
		}
		u64_stats_update_end(&tstats->syncp);
		dev->stats.rx_dropped += nss_cmn_rx_dropped_sum(stats);
		break;

	case NSS_GRE_MSG_BASE_STATS:
		break;

	default:
		nss_connmgr_gre_info("%px: Unknown Event from NSS\n", dev);
		break;
	}
}

/*
 * nss_connmgr_gre_make_name()
 *	Generate a name for netdevice if user does not provide one.
 */
static void nss_connmgr_gre_make_name(struct nss_connmgr_gre_cfg *cfg, char *name)
{
	switch (cfg->mode) {
	case GRE_MODE_TUN:
		strlcpy(name, "tun-%d", IFNAMSIZ);
		break;
	case GRE_MODE_TAP:
		strlcpy(name, "tap-%d", IFNAMSIZ);
		break;
	default:
		break;
	}
}

/*
 * __nss_connmgr_gre_create_interface()
 *	Creates GRE Tap/Tun netdevice and configure GRE node in NSS.
 *	This should be called after acquiring rtnl_lock().
 */
static struct net_device *__nss_connmgr_gre_create_interface(struct nss_connmgr_gre_cfg *cfg,
							     enum nss_connmgr_gre_err_codes *err_code)
{
	struct nss_ctx_instance *nss_ctx;
	struct net_device *dev = NULL;
	struct net_device *next_dev_inner = NULL;
	struct net_device *next_dev_outer = NULL;
	struct nss_gre_iface_instance *ngii;
	struct nss_gre_msg req;
	struct nss_gre_config_msg *cmsg = &req.msg.cmsg;
	nss_connmgr_gre_priv_t *priv;
	nss_tx_status_t status;
	uint32_t features = 0;
	int32_t inner_if, outer_if;
	char name[IFNAMSIZ] = {0};
	int ret = -1, retry, next_if_num_inner = 0, next_if_num_outer = 0;

	if (cfg->name) {
		strlcpy(name, cfg->name, IFNAMSIZ);
	} else {
		nss_connmgr_gre_make_name(cfg, name);
	}

	switch (cfg->mode) {
	case GRE_MODE_TUN:
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 17, 0))
		dev = alloc_netdev(sizeof(nss_connmgr_gre_priv_t), name, nss_connmgr_gre_tun_setup);
#else
		dev = alloc_netdev(sizeof(nss_connmgr_gre_priv_t), name, NET_NAME_UNKNOWN, nss_connmgr_gre_tun_setup);
#endif

		if (!dev) {
			nss_connmgr_gre_warning("Allocation of netdev failed\n");
			*err_code = GRE_ERR_ALLOC_NETDEV;
			return NULL;
		}

		if (cfg->is_ipv6) {
			dev->type = ARPHRD_IP6GRE;
			ret = nss_connmgr_gre_v6_set_config(dev, cfg);
		} else {
			dev->type = ARPHRD_IPGRE;
			ret = nss_connmgr_gre_v4_set_config(dev, cfg);
		}

		break;

	case GRE_MODE_TAP:
		dev = alloc_etherdev(sizeof(nss_connmgr_gre_priv_t));
		if (!dev) {
			nss_connmgr_gre_warning("Allocation of netdev failed\n");
			*err_code = GRE_ERR_ALLOC_NETDEV;
			return NULL;
		}

		nss_connmgr_gre_tap_setup(dev);

		if (cfg->is_ipv6) {
			dev->priv_flags_ext |= IFF_EXT_GRE_V6_TAP;
			ret = nss_connmgr_gre_v6_set_config(dev, cfg);
		} else {
			dev->priv_flags_ext |= IFF_EXT_GRE_V4_TAP;
			ret = nss_connmgr_gre_v4_set_config(dev, cfg);
		}
		break;

	default:
		nss_connmgr_gre_warning("Please Specify gre mode\n");
		*err_code = GRE_ERR_INVALID_MODE;
		goto release_ref;
	}

	if (ret) {
		nss_connmgr_gre_warning("%px: gre interface configuration failed\n", dev);
		*err_code = ret;
		goto release_ref;
	}

	/*
	 * Set name
	 */
	memcpy(dev->name, name, IFNAMSIZ);

	/*
	 * Create config cmd for acceleration engine
	 */
	memset(&req, 0, sizeof(struct nss_gre_msg));
	ret = nss_connmgr_gre_prepare_config_cmd(dev, &req, &next_dev_inner, true);
	if (ret) {
		nss_connmgr_gre_warning("%px: gre get config failed\n", dev);
		*err_code = ret;
		goto release_ref;
	}

	/*
	 *  Replace MAC addr and next node with user provided entries.
	 */
	if (cfg->use_mac_hdr) {
		memcpy(cmsg->src_mac, cfg->src_mac, ETH_ALEN);
		memcpy(cmsg->dest_mac, cfg->dest_mac, ETH_ALEN);
		cmsg->flags |= NSS_GRE_CONFIG_SET_MAC;
	}

	/*
	 * If next_dev is NULL, then the set MAC
	 * flag can not be set. Because the packet
	 * will be forwarded to ipv4_rx/ipv6_rx
	 */
	if (!cfg->next_dev) {
		cmsg->flags &= ~NSS_GRE_CONFIG_SET_MAC;
	}

	/*
	 * By now, we should have valid MAC addresses
	 */
	if (!is_valid_ether_addr((const u8 *)cmsg->src_mac) ||
	    !is_valid_ether_addr((const u8 *)cmsg->dest_mac)) {
		nss_connmgr_gre_warning("%px: Could not find MAC address for src/dest IP\n", dev);
		*err_code = GRE_ERR_INVALID_MAC;
		goto release_ref;
	}

	/*
	 * Configure the inner nexthop ifnum
	 */
	if (cfg->next_dev) {
		/*
		 * Release hold on GRE inner's nexthop that we have found,
		 * since it has been passed explicitly via the cfg
		 */
		if (next_dev_inner) {
			dev_put(next_dev_inner);
		}

		dev_hold(cfg->next_dev);

		/*
		 * TODO: Use the dynamic interface type for the inner's nexthop, in addition to the next_dev.
		 */
		next_if_num_inner = nss_cmn_get_interface_number_by_dev(cfg->next_dev);
		next_dev_inner = cfg->next_dev;
		if (next_if_num_inner < 0) {
			nss_connmgr_gre_warning("%px: Next dev inner device= %s is not registered with ae engine\n",
						dev, cfg->next_dev->name);
			*err_code = GRE_ERR_NEXT_NODE_UNREG_IN_AE;
			goto release_ref;
		}

		cmsg->flags |= NSS_GRE_CONFIG_NEXT_NODE_AVAILABLE;
	}

	/*
	 * Configure the outer nexthop ifnum
	 */
	if (cfg->next_dev_outer) {
		dev_hold(cfg->next_dev_outer);

		/*
		 * Verify if dynamic interface type is in range.
		 */
		if (cfg->outer_nss_if_type >= NSS_DYNAMIC_INTERFACE_TYPE_MAX) {
			nss_connmgr_gre_warning("%px: invalid cfg, outer nexthop type %d is not in range\n",
				       dev, cfg->outer_nss_if_type);
			goto release_ref;
		}

		next_if_num_outer = nss_cmn_get_interface_number_by_dev_and_type(cfg->next_dev_outer, cfg->outer_nss_if_type);
		next_dev_outer = cfg->next_dev_outer;

		if (next_if_num_outer < 0) {
			nss_connmgr_gre_warning("%px: Next dev outer device %s with dynamic if num %d not registered with NSS\n",
					dev, cfg->next_dev_outer->name, next_if_num_outer);
			*err_code = GRE_ERR_NEXT_NODE_UNREG_IN_AE;
			goto release_ref;
		}

		/*
		 * Get the NSS ctx for the outer next hop
		 */
		nss_ctx = nss_dynamic_interface_get_nss_ctx_by_type(cfg->outer_nss_if_type);
		if (!nss_ctx) {
			nss_connmgr_gre_warning("Could not get NSS context for type : %d\n", cfg->outer_nss_if_type);
			goto release_ref;
		}

		/*
		 * Append the core-id for the outer next hop ifnum
		 */
		next_if_num_outer = nss_cmn_append_core_id(nss_ctx, next_if_num_outer);
		if (!next_if_num_outer) {
			nss_connmgr_gre_warning("%px: Could not get interface number with core ID for outer nexthop device %s with core ID.\n",
				       nss_ctx, next_dev_outer->name);
			goto release_ref;
		}

		cmsg->flags |= NSS_GRE_CONFIG_NEXT_NODE_AVAILABLE;
	}

	/*
	 * By now, we should have a valid next node for either inner or outer
	 */
	if (!(cmsg->flags & NSS_GRE_CONFIG_NEXT_NODE_AVAILABLE)) {
		nss_connmgr_gre_warning("%px: Next dev is not available\n", dev);
		*err_code = GRE_ERR_NO_NEXT_NETDEV;
		goto release_ref;
	}

	if (cfg->add_padding) {
		cmsg->flags |= NSS_GRE_CONFIG_SET_PADDING;
	}

	if (cfg->copy_metadata) {
		cmsg->flags |= NSS_GRE_CONFIG_COPY_METADATA;
		cmsg->metadata_size = sizeof(struct nss_wifi_append_statsv2_metahdr);
	}

	/*
	 * Set per packet DSCP configuration if needed
	 */
	if (cfg->dscp_valid) {
		cmsg->flags |= NSS_GRE_CONFIG_DSCP_VALID;
	}

	/*
	 * Register net_device
	 */
	ret = register_netdevice(dev);
	if (ret) {
		*err_code = GRE_ERR_NETDEV_REG_FAILED;
		nss_connmgr_gre_warning("%px: Netdevice registration failed\n", dev);
		goto release_ref;
	}

	/*
	 * Create GRE interface instance
	 */
	ngii = nss_connmgr_gre_alloc_instance(dev);
	if (!ngii) {
		nss_connmgr_gre_warning("%px: GRE interfacen intance creation failed\n", dev);
		*err_code = GRE_ERR_ALLOC_GRE_INSTANCE;
		goto unregister_netdev;
	}

	/*
	 * Create nss outer dynamic interface
	 */
	outer_if = nss_dynamic_interface_alloc_node(NSS_DYNAMIC_INTERFACE_TYPE_GRE_OUTER);
	if (outer_if < 0) {
		nss_connmgr_gre_warning("%px: Request interface number failed\n", dev);
		*err_code = GRE_ERR_DYNMAIC_IFACE_CREATE;
		goto free_gre_instance;
	}

	ngii->outer_ifnum = outer_if;
	priv = (nss_connmgr_gre_priv_t *)netdev_priv(dev);
	priv->next_dev_outer = next_dev_outer;

	/*
	 * Create nss inner dynamic interface
	 */
	inner_if = nss_dynamic_interface_alloc_node(NSS_DYNAMIC_INTERFACE_TYPE_GRE_INNER);
	if (inner_if < 0) {
		nss_connmgr_gre_warning("%px: Request interface number failed\n", dev);
		*err_code = GRE_ERR_DYNMAIC_IFACE_CREATE;
		goto free_gre_instance;
	}

	ngii->inner_ifnum = inner_if;
	priv->next_dev_inner = next_dev_inner;
	priv->nss_if_number_inner = inner_if;

	/*
	 * Register outer gre tunnel with NSS
	 */
	nss_ctx = nss_gre_register_if(outer_if,
				NSS_DYNAMIC_INTERFACE_TYPE_GRE_OUTER,
				nss_connmgr_gre_rx_pkt,
	 			nss_connmgr_gre_event_receive,
				dev,
				features);
	if (!nss_ctx) {
		nss_connmgr_gre_info("%px: nss_register_gre_if failed\n", dev);
		*err_code = GRE_ERR_GRE_IFACE_REG;
		goto dealloc_inner_node;
	}

	/*
	 * Register inner gre tunnel with NSS
	 */
	nss_ctx = nss_gre_register_if(inner_if,
				NSS_DYNAMIC_INTERFACE_TYPE_GRE_INNER,
				nss_connmgr_gre_rx_pkt,
	 			nss_connmgr_gre_event_receive,
				dev,
				features);
	if (!nss_ctx) {
		nss_connmgr_gre_info("%px: nss_register_gre_if failed\n", dev);
		*err_code = GRE_ERR_GRE_IFACE_REG;
		goto dealloc_inner_node;
	}

	nss_connmgr_gre_info("%px: nss_register_gre_if() custom successful. nss_ctx = %px. inner_if: %d, outer_if: %d\n", dev, nss_ctx, inner_if, outer_if);

	/*
	 * Send encap config to AE
	 */
	cmsg->flags &= ~NSS_GRE_CONFIG_NEXT_NODE_AVAILABLE;

	/*
	 * Configure nexthop to inner node if available
	 */
	if (next_if_num_inner) {
		cmsg->next_node_if_num = next_if_num_inner;
		cmsg->flags |= NSS_GRE_CONFIG_NEXT_NODE_AVAILABLE;
	}
	cmsg->sibling_if_num = outer_if;
	nss_gre_msg_init(&req, inner_if, NSS_GRE_MSG_ENCAP_CONFIGURE, sizeof(struct nss_gre_config_msg), NULL, NULL);
	nss_connmgr_gre_info("%px: NSS_GRE_MSG_ENCAP_CONFIGURE:: nss_ctx = %px, flags:0x%x, ikey:0x%x, okey:0x%x, mode:%x, next_node_if_num:%u, sibling_if_num:%u, ttl:%u, tos:%u, metadata_size:%u\n",
			dev, nss_ctx, cmsg->flags, cmsg->ikey, cmsg->okey, cmsg->mode, cmsg->next_node_if_num, cmsg->sibling_if_num, cmsg->ttl, cmsg->tos, cmsg->metadata_size);
	status = nss_gre_tx_msg_sync(nss_ctx, &req);
	if (status != NSS_TX_SUCCESS) {
		*err_code = GRE_ERR_AE_CONFIG_FAILED;
		nss_connmgr_gre_info("%px: Send Encap config to AE failed\n", next_dev_inner);
		goto unregister_nss_interface;
	}

	/*
	 * Send decap config to AE
	 */
	cmsg->flags &= ~NSS_GRE_CONFIG_NEXT_NODE_AVAILABLE;

	/*
	 * Configure nexthop to outer node if available
	 */
	if (next_if_num_outer) {
		cmsg->next_node_if_num = next_if_num_outer;
		cmsg->flags |= NSS_GRE_CONFIG_NEXT_NODE_AVAILABLE;
	}
	cmsg->sibling_if_num = inner_if;
	nss_gre_msg_init(&req, outer_if, NSS_GRE_MSG_DECAP_CONFIGURE, sizeof(struct nss_gre_config_msg), NULL, NULL);
	nss_connmgr_gre_info("%px: NSS_GRE_MSG_DECAP_CONFIGURE:: nss_ctx = %px, flags:0x%x, ikey:0x%x, okey:0x%x, mode:%x, next_node_if_num:%u, sibling_if_num:%u, ttl:%u, tos:%u, metadata_size:%u\n",
			dev, nss_ctx, cmsg->flags, cmsg->ikey, cmsg->okey, cmsg->mode, cmsg->next_node_if_num, cmsg->sibling_if_num, cmsg->ttl, cmsg->tos, cmsg->metadata_size);
	status = nss_gre_tx_msg_sync(nss_ctx, &req);
	if (status != NSS_TX_SUCCESS) {
		*err_code = GRE_ERR_AE_CONFIG_FAILED;
		nss_connmgr_gre_info("%px: Send decap config to AE failed\n", next_dev_outer);
		goto unregister_nss_interface;
	}

	/*
	 * Set vap next hop if next_dev is configured
	 * TODO: Do this in a more generic manner by checking the dynamic interface type of next_dev
	 */
	if (cfg->next_dev) {
		ret = nss_connmgr_gre_set_wifi_next_hop(cfg->next_dev);
		if (ret) {
			nss_connmgr_gre_info("%px: Setting next hop of wifi vdev failed\n", dev);
			*err_code = ret;
			goto unregister_nss_interface;
		}
	}

	memcpy(&ngii->gre_cfg, cfg, sizeof(*cfg));
	ngii->gre_iftype = NSS_CONNMGR_GRE_IFTYPE_CUSTOM_GRE;

	spin_lock(&gre_connmgr_ctx.lock);
	list_add(&ngii->list, &gre_connmgr_ctx.list);
	spin_unlock(&gre_connmgr_ctx.lock);

	/*
	 * Success
	 */
	*err_code = GRE_SUCCESS;
	return dev;

unregister_nss_interface:
	nss_gre_unregister_if(inner_if);
	nss_gre_unregister_if(outer_if);

	retry = 0;
dealloc_inner_node:
	status = nss_dynamic_interface_dealloc_node(inner_if, NSS_DYNAMIC_INTERFACE_TYPE_GRE_INNER);
	if (status != NSS_TX_SUCCESS) {
		if (++retry <= MAX_RETRY_COUNT) {
			goto dealloc_inner_node;
		}
		nss_connmgr_gre_error("%px: Fatal Error, Unable to dealloc the node[%d] in the NSS FW!\n", dev, inner_if);
	}

	retry = 0;
dealloc_outer_node:
	status = nss_dynamic_interface_dealloc_node(outer_if, NSS_DYNAMIC_INTERFACE_TYPE_GRE_OUTER);
	if (status != NSS_TX_SUCCESS) {
		if (++retry <= MAX_RETRY_COUNT) {
			goto dealloc_outer_node;
		}
		nss_connmgr_gre_error("%px: Fatal Error, Unable to dealloc the node[%d] in the NSS FW!\n", dev, outer_if);
	}

free_gre_instance:
	nss_connmgr_gre_free_instance(ngii);

unregister_netdev:
	unregister_netdevice(dev);

release_ref:
	if (next_dev_inner) {
		dev_put(next_dev_inner);
	}

	if (next_dev_outer) {
		dev_put(next_dev_outer);
	}

	return dev;
}

/*
 * nss_connmgr_gre_destroy_inner_interface()
 *	Destroy inner GRE Tap/Tun interface.
 */
static enum nss_connmgr_gre_err_codes nss_connmgr_gre_destroy_inner_interface(struct net_device *dev, int interface_num)
{
	struct nss_gre_msg req;
	struct nss_gre_deconfig_msg *dmsg;
	nss_tx_status_t status;
	int retry = 0;

	memset(&req, 0, sizeof(struct nss_gre_msg));
	dmsg = &req.msg.dmsg;
	dmsg->if_number = interface_num;

deconfig_inner:
	nss_gre_msg_init(&req, interface_num, NSS_GRE_MSG_ENCAP_DECONFIGURE, sizeof(struct nss_gre_deconfig_msg), NULL, NULL);
	status = nss_gre_tx_msg_sync(nss_gre_get_context(), &req);
	if (status != NSS_TX_SUCCESS) {
		nss_connmgr_gre_info("%px: gre instance deconfigure command failed, interface_num = %d\n", dev, interface_num);

		if (++retry <= MAX_RETRY_COUNT) {
			goto deconfig_inner;
		}

		nss_connmgr_gre_error("%px: Fatal Error, failed to send GRE deconfig command to NSS\n", dev);
		return GRE_ERR_AE_DECONFIG_FAILED;
	}
	retry = 0;

	nss_gre_unregister_if(interface_num);

dealloc_inner:
	status = nss_dynamic_interface_dealloc_node(interface_num, NSS_DYNAMIC_INTERFACE_TYPE_GRE_INNER);
	if (status != NSS_TX_SUCCESS) {
		nss_connmgr_gre_info("%px: gre dealloc node failure for interface_num = %d\n", dev, interface_num);

		if (++retry <= MAX_RETRY_COUNT) {
			goto dealloc_inner;
		}

		nss_connmgr_gre_error("%px: Fatal Error, failed to send GRE dealloc command to NSS\n", dev);
		return GRE_ERR_DYNMAIC_IFACE_DESTROY;
	}

	return GRE_SUCCESS;
}

/*
 * nss_connmgr_gre_destroy_outer_interface()
 *	Destroy outer GRE Tap/Tun interface.
 */
static enum nss_connmgr_gre_err_codes nss_connmgr_gre_destroy_outer_interface(struct net_device *dev, int interface_num)
{
	struct nss_gre_msg req;
	struct nss_gre_deconfig_msg *dmsg;
	nss_tx_status_t status;
	int retry = 0;

	memset(&req, 0, sizeof(struct nss_gre_msg));
	dmsg = &req.msg.dmsg;
	dmsg->if_number = interface_num;

deconfig_outer:
	nss_gre_msg_init(&req, interface_num, NSS_GRE_MSG_DECAP_DECONFIGURE, sizeof(struct nss_gre_deconfig_msg), NULL, NULL);
	status = nss_gre_tx_msg_sync(nss_gre_get_context(), &req);
	if (status != NSS_TX_SUCCESS) {
		nss_connmgr_gre_info("%px: gre instance deconfigure command failed, interface_num = %d\n", dev, interface_num);

		if (++retry <= MAX_RETRY_COUNT) {
			goto deconfig_outer;
		}

		nss_connmgr_gre_error("%px: Fatal Error, failed to send GRE deconfig command to NSS\n", dev);
		return GRE_ERR_AE_DECONFIG_FAILED;
	}

	nss_gre_unregister_if(interface_num);
	retry = 0;

dealloc_outer:
	status = nss_dynamic_interface_dealloc_node(interface_num, NSS_DYNAMIC_INTERFACE_TYPE_GRE_OUTER);
	if (status != NSS_TX_SUCCESS) {
		nss_connmgr_gre_info("%px: gre dealloc node failure for inner_if = %d\n", dev, interface_num);

		if (++retry <= MAX_RETRY_COUNT) {
			goto dealloc_outer;
		}

		nss_connmgr_gre_error("%px: Fatal Error, failed to send GRE dealloc command to NSS\n", dev);
		return GRE_ERR_DYNMAIC_IFACE_DESTROY;
	}

	return GRE_SUCCESS;
}

/*
 * __nss_connmgr_gre_destroy_interface()
 *	Destroy GRE Tap/Tun netdevice. Acquire rtnl_lock() before calling this
 *	function.
 */
static enum nss_connmgr_gre_err_codes __nss_connmgr_gre_destroy_interface(struct net_device *dev)
{
	struct nss_gre_iface_instance *ngii;
	nss_connmgr_gre_priv_t *priv;
	enum nss_connmgr_gre_err_codes ret;

	netif_tx_disable(dev);

	ngii = nss_connmgr_gre_find_instance(dev);
	if(!ngii) {
		nss_connmgr_gre_warning("%px: GRE interface instance is not found.\n", dev);
		return GRE_ERR_NO_GRE_INSTANCE;
	}

	/*
	 * Decrement ref to next_dev
	 */
	priv = (nss_connmgr_gre_priv_t *)netdev_priv(dev);
	if (priv->next_dev_inner) {
		dev_put(priv->next_dev_inner);
	}

	if (priv->next_dev_outer) {
		dev_put(priv->next_dev_outer);
	}

	ret = nss_connmgr_gre_destroy_inner_interface(dev, ngii->inner_ifnum);
	if (ret != GRE_SUCCESS) {
		nss_connmgr_gre_warning("%px: failed to destroy inner interface: %d\n", dev, ngii->inner_ifnum);
		return ret;
	}

	ret = nss_connmgr_gre_destroy_outer_interface(dev, ngii->outer_ifnum);
	if (ret != GRE_SUCCESS) {
		nss_connmgr_gre_warning("%px: failed to destroy outer interface: %d\n", dev, ngii->outer_ifnum);
		return ret;
	}

	nss_connmgr_gre_info("%px: deleted gre instance, inner_if = %d outer_if = %d\n",
			dev, ngii->inner_ifnum, ngii->outer_ifnum);

	nss_connmgr_gre_free_instance(ngii);
	unregister_netdevice(dev);
	return GRE_SUCCESS;
}

/*
 * nss_connmgr_gre_validate_config()
 *	No support for CSUM, SEQ number.
 */
static bool nss_connmgr_gre_validate_config(struct nss_connmgr_gre_cfg *cfg)
{
	/*
	 * TODO:Disallow key for standard GRE TAP/TUN.
	 */
	if (cfg->iseq_valid || cfg->oseq_valid || cfg->icsum_valid || cfg->ocsum_valid) {
		nss_connmgr_gre_info("Bad config, seq and csum are not supported.\n");
		return false;
	}

	/*
	 * Validate DSCP and ToS inherit flags.
	 */
	if (cfg->dscp_valid && cfg->tos_inherit) {
		nss_connmgr_gre_info("Bad config, Both DSCP and ToS inherit flags are set.\n");
		return false;
	}

	return true;
}

/*
 * nss_connmgr_gre_dev_up()
 *	Netdevice notifier call back to configure NSS for GRE Tap/Tun device.
 */
static int nss_connmgr_gre_dev_up(struct net_device *dev)
{
	struct nss_gre_msg req;
	struct nss_gre_config_msg *cmsg = &req.msg.cmsg;
	int inner_if, outer_if;
	uint32_t features = 0;
	struct nss_ctx_instance *nss_ctx = NULL;
	nss_tx_status_t status;
	struct net_device *next_dev = NULL;

	/*
	 * If GRE interface instance is found return, dev is Custom GRE interface type.
	 */
	if (nss_connmgr_gre_find_instance(dev)) {
		nss_connmgr_gre_info("%px: Custom GRE interface is up.\n", dev);
		return NOTIFY_DONE;
	}

	/*
	 * Create config cmd for acceleration engine
	 */
	memset(&req, 0, sizeof(struct nss_gre_msg));
	if (nss_connmgr_gre_prepare_config_cmd(dev, &req, &next_dev, false)) {
		nss_connmgr_gre_info("%px: gre tunnel get config failed\n", dev);
		return NOTIFY_DONE;
	}

	/*
	 * Ignore set_mac and next_dev flags for generic GRE
	 */
	cmsg->flags &= ~(NSS_GRE_CONFIG_SET_MAC | NSS_GRE_CONFIG_NEXT_NODE_AVAILABLE);

	/*
	 * Create nss outer dynamic interface
	 */
	outer_if = nss_dynamic_interface_alloc_node(NSS_DYNAMIC_INTERFACE_TYPE_GRE_OUTER);
	if (outer_if < 0) {
		nss_connmgr_gre_warning("%px: Request interface number failed\n", dev);
		return NOTIFY_DONE;
	}

	/*
	 * Create nss inner dynamic interface
	 */
	inner_if = nss_dynamic_interface_alloc_node(NSS_DYNAMIC_INTERFACE_TYPE_GRE_INNER);
	if (inner_if < 0) {
		nss_connmgr_gre_warning("%px: Request interface number failed\n", dev);
		goto dealloc_outer;
	}

	/*
	 * Register gre inner/outer interface with NSS
	 */
	if ((dev->type == ARPHRD_IPGRE) || (dev->type == ARPHRD_IP6GRE)) {
		/*
		 * GRE Tunnel mode
		 */
		nss_ctx = nss_gre_register_if(inner_if,
				NSS_DYNAMIC_INTERFACE_TYPE_GRE_INNER,
				nss_connmgr_gre_tun_inner_exception,
				nss_connmgr_gre_event_receive,
				dev,
				features);

		if (!nss_ctx) {
			nss_connmgr_gre_info("%px: nss_register_gre_if failed\n", dev);
			goto dealloc_inner;
		}

		nss_ctx = nss_gre_register_if(outer_if,
				NSS_DYNAMIC_INTERFACE_TYPE_GRE_OUTER,
				nss_connmgr_gre_tun_outer_exception,
				nss_connmgr_gre_event_receive,
				dev,
				features);
	} else {
		/*
		 * GRE Tap mode
		 */
		nss_ctx = nss_gre_register_if(inner_if,
				NSS_DYNAMIC_INTERFACE_TYPE_GRE_INNER,
				nss_connmgr_gre_tap_inner_exception,
				nss_connmgr_gre_event_receive,
				dev,
				features);

		if (!nss_ctx) {
			nss_connmgr_gre_info("%px: nss_register_gre_if failed\n", dev);
			goto dealloc_inner;
		}

		nss_ctx = nss_gre_register_if(outer_if,
				NSS_DYNAMIC_INTERFACE_TYPE_GRE_OUTER,
				nss_connmgr_gre_tap_outer_exception,
				nss_connmgr_gre_event_receive,
				dev,
				features);
	}

	if (!nss_ctx) {
		nss_connmgr_gre_info("%px: nss_register_gre_if failed\n", dev);
		goto unregister_inner;
	}

	nss_connmgr_gre_info("%px: nss_register_gre_if() standard successful. nss_ctx = %px. inner_if: %d, outer_if: %d\n", dev, nss_ctx, inner_if, outer_if);

	/*
	 * Send configure command for inner interface
	 */
	cmsg->sibling_if_num = outer_if;
	nss_gre_msg_init(&req, inner_if, NSS_GRE_MSG_ENCAP_CONFIGURE, sizeof(struct nss_gre_config_msg), NULL, NULL);
	nss_connmgr_gre_info("%px: NSS_GRE_MSG_ENCAP_CONFIGURE:: nss_ctx = %px, flags:0x%x, ikey:0x%x, okey:0x%x, mode:%x, next_node_if_num:%u, sibling_if_num:%u, ttl:%u, tos:%u, metadata_size:%u\n",
			dev, nss_ctx, cmsg->flags, cmsg->ikey, cmsg->okey, cmsg->mode, cmsg->next_node_if_num, cmsg->sibling_if_num, cmsg->ttl, cmsg->tos, cmsg->metadata_size);
	status = nss_gre_tx_msg_sync(nss_ctx, &req);
	if (status != NSS_TX_SUCCESS) {
		nss_connmgr_gre_warning("%px: nss gre std configure command error %d\n", dev, status);
		goto unregister_outer;
	}

	/*
	 * Send configure command for outer interface
	 */
	cmsg->sibling_if_num = inner_if;
	nss_gre_msg_init(&req, outer_if, NSS_GRE_MSG_DECAP_CONFIGURE, sizeof(struct nss_gre_config_msg), NULL, NULL);
	nss_connmgr_gre_info("%px: NSS_GRE_MSG_DECAP_CONFIGURE:: nss_ctx = %px, flags:0x%x, ikey:0x%x, okey:0x%x, mode:%x, next_node_if_num:%u, sibling_if_num:%u, ttl:%u, tos:%u, metadata_size:%u\n",
			dev, nss_ctx, cmsg->flags, cmsg->ikey, cmsg->okey, cmsg->mode, cmsg->next_node_if_num, cmsg->sibling_if_num, cmsg->ttl, cmsg->tos, cmsg->metadata_size);
	status = nss_gre_tx_msg_sync(nss_ctx, &req);
	if (status != NSS_TX_SUCCESS) {
		nss_connmgr_gre_warning("%px: nss gre std configure command error %d\n", dev, status);
		goto unregister_outer;
	}

	/*
	 * Open the netdev to accept packets
	 */
	if (nss_connmgr_gre_dev_open(dev)) {
		nss_connmgr_gre_warning("%px: nss gre std device up command failed %d\n", dev, status);
		goto unregister_outer;
	}

	return NOTIFY_DONE;

unregister_outer:
	nss_gre_unregister_if(outer_if);

unregister_inner:
	nss_gre_unregister_if(inner_if);

dealloc_inner:
	status = nss_dynamic_interface_dealloc_node(inner_if, NSS_DYNAMIC_INTERFACE_TYPE_GRE_INNER);
	if (status != NSS_TX_SUCCESS) {
		nss_connmgr_gre_warning("%px: Unable to dealloc the node[%d] in the NSS fw!\n", dev, inner_if);
	}

dealloc_outer:
	status = nss_dynamic_interface_dealloc_node(outer_if, NSS_DYNAMIC_INTERFACE_TYPE_GRE_OUTER);
	if (status != NSS_TX_SUCCESS) {
		nss_connmgr_gre_warning("%px: Unable to dealloc the node[%d] in the NSS fw!\n", dev, outer_if);
	}

	return NOTIFY_DONE;
}

/*
 * nss_connmgr_gre_dev_down()
 *	Netdevice notifier call back to destroy GRE interface in NSS.
 */
static int nss_connmgr_gre_dev_down(struct net_device *dev)
{
	struct nss_gre_msg req;
	struct nss_gre_deconfig_msg *dmsg;
	int inner_if, outer_if;
	nss_tx_status_t status;

	/*
	 * If GRE interface instance is found return, dev is Custom GRE interface type.
	 */
	if (nss_connmgr_gre_find_instance(dev)) {
		nss_connmgr_gre_info("%px: Custom GRE interface is down.\n", dev);
		return NOTIFY_DONE;
	}

	/*
	 * Check if gre inner interface is registered with NSS
	 */
	inner_if = nss_cmn_get_interface_number_by_dev_and_type(dev, NSS_DYNAMIC_INTERFACE_TYPE_GRE_INNER);
	if (inner_if < 0) {
		nss_connmgr_gre_info("%px: Net device is not registered with nss\n", dev);
		return NOTIFY_DONE;
	}

	outer_if = nss_cmn_get_interface_number_by_dev_and_type(dev, NSS_DYNAMIC_INTERFACE_TYPE_GRE_OUTER);
	if (outer_if < 0) {
		nss_connmgr_gre_info("%px: Net device is not registered with nss\n", dev);
		return NOTIFY_DONE;
	}

	memset(&req, 0, sizeof(struct nss_gre_msg));
	dmsg = &req.msg.dmsg;
	dmsg->if_number = inner_if;
	nss_gre_msg_init(&req, inner_if, NSS_GRE_MSG_ENCAP_DECONFIGURE, sizeof(struct nss_gre_deconfig_msg), NULL, NULL);
	status = nss_gre_tx_msg_sync(nss_gre_get_context(), &req);
	if (status != NSS_TX_SUCCESS) {
		nss_connmgr_gre_info("%px: gre instance deconfigure command fo encap failed, inner_if = %d\n", dev, inner_if);
		return NOTIFY_DONE;
	}

	dmsg->if_number = outer_if;
	nss_gre_msg_init(&req, outer_if, NSS_GRE_MSG_DECAP_DECONFIGURE, sizeof(struct nss_gre_deconfig_msg), NULL, NULL);
	status = nss_gre_tx_msg_sync(nss_gre_get_context(), &req);
	if (status != NSS_TX_SUCCESS) {
		nss_connmgr_gre_info("%px: gre instance deconfigure command for decap failed, inner_if = %d\n", dev, inner_if);
		return NOTIFY_DONE;
	}

	if (nss_connmgr_gre_dev_close(dev)) {
		nss_connmgr_gre_info("%px: gre instance device close command failed, inner_if = %d\n", dev, inner_if);
		return NOTIFY_DONE;
	}

	nss_gre_unregister_if(inner_if);
	status = nss_dynamic_interface_dealloc_node(inner_if, NSS_DYNAMIC_INTERFACE_TYPE_GRE_INNER);
	if (status != NSS_TX_SUCCESS) {
		nss_connmgr_gre_info("%px: gre dealloc node failure for inner_if = %d\n", dev, inner_if);
		return NOTIFY_DONE;
	}
	nss_connmgr_gre_info("%px: deleting gre instance, inner_if = %d\n", dev, inner_if);

	nss_gre_unregister_if(outer_if);
	status = nss_dynamic_interface_dealloc_node(outer_if, NSS_DYNAMIC_INTERFACE_TYPE_GRE_OUTER);
	if (status != NSS_TX_SUCCESS) {
		nss_connmgr_gre_info("%px: gre dealloc node failure for outer_if = %d\n", dev, outer_if);
		return NOTIFY_DONE;
	}
	nss_connmgr_gre_info("%px: deleted gre instance, outer_if = %d\n", dev, outer_if);

	return NOTIFY_DONE;
}

/*
 * nss_connmgr_gre_dev_event()
 *	Netdevice notifier call back function.
 */
static int nss_connmgr_gre_dev_event(struct notifier_block  *nb,
		unsigned long event, void  *dev)
{
	struct net_device *netdev;
	netdev = netdev_notifier_info_to_dev(dev);

	switch (event) {
	case NETDEV_UP:
		return nss_connmgr_gre_dev_up(netdev);

	case NETDEV_DOWN:
		return nss_connmgr_gre_dev_down(netdev);

	default:
		break;
	}

	return NOTIFY_DONE;
}

/*
 * Linux Net device Notifier
 */
static struct notifier_block nss_connmgr_gre_notifier = {
	.notifier_call = nss_connmgr_gre_dev_event,
};

/*
 * nss_connmgr_gre_dev_init_module()
 *	Tunnel gre module init function
 */
static int __init nss_connmgr_gre_dev_init_module(void)
{
#ifdef CONFIG_OF
	/*
	 * If the node is not compatible, don't do anything.
	 */
	if (!of_find_node_by_name(NULL, "nss-common")) {
		return 0;
	}
#endif
	INIT_LIST_HEAD(&gre_connmgr_ctx.list);
	spin_lock_init(&gre_connmgr_ctx.lock);
	register_netdevice_notifier(&nss_connmgr_gre_notifier);

	return 0;
}

/*
 * nss_connmgr_gre_exit_module
 *	Tunnel gre module exit function
 */
static void __exit nss_connmgr_gre_exit_module(void)
{
	struct nss_gre_iface_instance *ngii, *n;
#ifdef CONFIG_OF
	/*
	 * If the node is not compatible, don't do anything.
	 */
	if (!of_find_node_by_name(NULL, "nss-common")) {
		return;
	}
#endif
	/*
	 * Unregister GRE interfaces created and delete interface
	 * instances.
	 */
	list_for_each_entry_safe(ngii, n, &gre_connmgr_ctx.list, list) {
		rtnl_lock();
		__nss_connmgr_gre_destroy_interface(ngii->dev);
		rtnl_unlock();
	}

	unregister_netdevice_notifier(&nss_connmgr_gre_notifier);
}

/*
 * nss_connmgr_gre_set_next_hop()
 *	Function changes next hop of wifi_vdev to NSS_GRE_INTERFACE
 */
int nss_connmgr_gre_set_wifi_next_hop(struct net_device *wifi_vdev)
{
	nss_tx_status_t status;
	void *ctx;
	int ifnumber;

	if (!wifi_vdev) {
		nss_connmgr_gre_info("wifi interface is NULL\n");
		return GRE_ERR_NO_NEXT_NETDEV;
	}

	ifnumber = nss_cmn_get_interface_number_by_dev(wifi_vdev);
	if (ifnumber < 0) {
		nss_connmgr_gre_info("%px: wifi interface is not recognized by NSS\n", wifi_vdev);
		return GRE_ERR_NEXT_NODE_UNREG_IN_AE;
	}

	ctx = nss_wifili_get_context();
	status = nss_wifi_vdev_set_next_hop(ctx, ifnumber, NSS_GRE_INTERFACE);
	if (status != NSS_TX_SUCCESS) {
		nss_connmgr_gre_info("%px: wifi drv api failed to set next hop\n", wifi_vdev);
		return GRE_ERR_AE_SET_NEXT_HOP;
	}

	return GRE_SUCCESS;
}

/*
 * nss_connmgr_gre_get_hlen()
 *	Calculates GRE header length
 */
uint32_t nss_connmgr_gre_get_hlen(struct nss_connmgr_gre_cfg *cfg)
{
	uint32_t size = 4;	/* minimum size of GRE packet */

	if (cfg->okey_valid) {
		size += 4;
	}

	if (cfg->oseq_valid) {
		size += 4;
	}

	if (cfg->ocsum_valid) {
		size += 4;
	}

	return size;
}

/*
 * nss_connmgr_gre_set_gre_flags()
 *	Map User configured flags to tunnel specific flag.
 */
void nss_connmgr_gre_set_gre_flags(struct nss_connmgr_gre_cfg *cfg,
					   uint16_t *o_flags, uint16_t *i_flags)

{
	if (cfg->ikey_valid) {
		*i_flags |= TUNNEL_KEY;
	}

	if (cfg->okey_valid) {
		*o_flags |= TUNNEL_KEY;
	}

	if (cfg->iseq_valid) {
		*i_flags |= TUNNEL_SEQ;
	}

	if (cfg->oseq_valid) {
		*o_flags |= TUNNEL_SEQ;
	}

	if (cfg->icsum_valid) {
		*i_flags |= TUNNEL_CSUM;
	}

	if (cfg->ocsum_valid) {
		*o_flags |= TUNNEL_CSUM;
	}
}

/*
 * nss_connmgr_gre_get_nss_config_flags()
 *	Map tunnel specific flags to NSS flags.
 */
uint32_t nss_connmgr_gre_get_nss_config_flags(uint16_t o_flags, uint16_t i_flags,
						  uint8_t tos, uint8_t ttl,
						  uint16_t frag_off)
{
	uint32_t gre_flags = 0;

	if (i_flags & TUNNEL_KEY) {
		gre_flags |= NSS_GRE_CONFIG_IKEY_VALID;
	}

	if (o_flags & TUNNEL_KEY) {
		gre_flags |= NSS_GRE_CONFIG_OKEY_VALID;
	}

	if (i_flags & TUNNEL_SEQ) {
		gre_flags |= NSS_GRE_CONFIG_ISEQ_VALID;
	}

	if (o_flags & TUNNEL_SEQ) {
		gre_flags |= NSS_GRE_CONFIG_OSEQ_VALID;
	}

	if (i_flags & TUNNEL_CSUM) {
		gre_flags |= NSS_GRE_CONFIG_ICSUM_VALID;
	}

	if (o_flags & TUNNEL_CSUM) {
		gre_flags |= NSS_GRE_CONFIG_OCSUM_VALID;
	}

	if (tos & 0x1) {
		gre_flags |= NSS_GRE_CONFIG_TOS_INHERIT;
	}

	if (!ttl) {
		gre_flags |= NSS_GRE_CONFIG_TTL_INHERIT;
	}

	if (frag_off == htons(IP_DF)) {
		gre_flags |= NSS_GRE_CONFIG_SET_DF;
	}

	return gre_flags;
}

 /*
  * nss_connmgr_gre_destroy_interface()
  *	User API to delete interface
  */
enum nss_connmgr_gre_err_codes nss_connmgr_gre_destroy_interface(struct net_device *dev)
{
	enum nss_connmgr_gre_err_codes ret;

	if (!dev) {
		nss_connmgr_gre_info("Please specifiy valid interface to be deleted\n");
		return GRE_ERR_NO_NETDEV;
	}

	if (in_interrupt()) {
		nss_connmgr_gre_info("%px: nss_connmgr_gre_destroy_interface() called in interrupt context\n", dev);
		return GRE_ERR_IN_INTERRUPT_CTX;
	}

	rtnl_lock();
	ret = __nss_connmgr_gre_destroy_interface(dev);
	rtnl_unlock();

	/*
	 * free_netdev() should be called outside of rtnl lock
	 */
	if (!ret) {
		free_netdev(dev);
	}

	return ret;
}
EXPORT_SYMBOL(nss_connmgr_gre_destroy_interface);

/*
 * nss_connmgr_gre_create_interface()
 *	User API to create gre standard interface
 */
struct net_device *nss_connmgr_gre_create_interface(struct nss_connmgr_gre_cfg *cfg,
						    enum nss_connmgr_gre_err_codes *err_code)
{
	struct net_device *dev;

	if ((!cfg) || (!err_code)) {
		nss_connmgr_gre_info("parameter to this function should not be NULL\n");
		return NULL;
	}

	if (in_interrupt()) {
		nss_connmgr_gre_info("nss_connmgr_gre_create_interface() called in interrupt context\n");
		*err_code = GRE_ERR_IN_INTERRUPT_CTX;
		return NULL;
	}

	if (!nss_connmgr_gre_validate_config(cfg)) {
		*err_code = GRE_ERR_UNSUPPORTED_CFG;
		return NULL;
	}

	rtnl_lock();
	dev = __nss_connmgr_gre_create_interface(cfg, err_code);
	rtnl_unlock();

	if (*err_code == GRE_SUCCESS) {
		return dev;
	}

	if (dev) {
		free_netdev(dev);
	}

	return NULL;
}
EXPORT_SYMBOL(nss_connmgr_gre_create_interface);

module_init(nss_connmgr_gre_dev_init_module);
module_exit(nss_connmgr_gre_exit_module);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS gre offload manager");
