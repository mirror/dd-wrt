/*
 **************************************************************************
 * Copyright (c) 2015, 2016, The Linux Foundation. All rights reserved.
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
 * nss_portifmgr.c
 *	NSS to HLOS Port Interface manager
 */
#include <linux/module.h>
#include <linux/etherdevice.h>
#include <linux/if_arp.h>
#include <linux/rtnetlink.h>
#include <nss_api_if.h>
#include <nss_gmac_api_if.h>

#include <nss_portifmgr.h>

#define NSS_PORTIFMGR_EXTRA_HEADER_SIZE	4

/*
 * Features enabled for portifmgr, note we are not able to enable HW_CSUM
 * related features because GMAC HW can't do checksum offloading when
 * proprietary header inserted
 */
#define NSS_PORTIFMGR_SUPPORTED_FEATURES (NETIF_F_HIGHDMA | NETIF_F_SG | NETIF_F_FRAGLIST)

#if (NSS_PORTIFMGR_DEBUG_LEVEL < 1)
#define nss_portifmgr_assert(fmt, args...)
#else
#define nss_portifmgr_assert(c) BUG_ON(!(c))
#endif /* NSS_PORTIFMGR_DEBUG_LEVEL */

/*
 * Compile messages for dynamic enable/disable
 */
#if defined(CONFIG_DYNAMIC_DEBUG)
#define nss_portifmgr_warn(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define nss_portifmgr_info(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define nss_portifmgr_trace(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#else /* CONFIG_DYNAMIC_DEBUG */
/*
 * Statically compile messages at different levels
 */
#if (NSS_PORTIFMGR_DEBUG_LEVEL < 2)
#define nss_portifmgr_warn(s, ...)
#else
#define nss_portifmgr_warn(s, ...) pr_warn("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_PORTIFMGR_DEBUG_LEVEL < 3)
#define nss_portifmgr_info(s, ...)
#else
#define nss_portifmgr_info(s, ...)   pr_notice("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_PORTIFMGR_DEBUG_LEVEL < 4)
#define nss_portifmgr_trace(s, ...)
#else
#define nss_portifmgr_trace(s, ...)  pr_info("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif
#endif /* CONFIG_DYNAMIC_DEBUG */


#ifdef NSS_PORTIFMGR_REF_AP148
/*
 * This holds all the netdev created on the switch ports
 */
struct net_device *ndev_list[NSS_PORTID_MAX_SWITCH_PORT];
#endif

/*
 * nss_portifmgr_receive_pkt()
 *	Receives a pkt from NSS
 */
static void nss_portifmgr_receive_pkt(struct net_device *dev, struct sk_buff *skb, struct napi_struct *napi)
{
	struct nss_portifmgr_priv *priv;

	/* SKB NETIF START */
	dev_hold(dev);
	priv = netdev_priv(dev);

	skb->dev = dev;
	skb->protocol = eth_type_trans(skb, dev);

	nss_portifmgr_trace("%s: Rx on if_num %d, packet len %d, CSUM %d\n",
			__func__, priv->if_num, skb->len, skb->ip_summed);

	(void)netif_receive_skb(skb);
	/* SKB NETIF END */
	dev_put(dev);
}

/*
 * nss_portifmgr_open()
 *	Netdev's open call.
 */
static int nss_portifmgr_open(struct net_device *dev)
{
	struct nss_portifmgr_priv *priv = (struct nss_portifmgr_priv *)netdev_priv(dev);

	if (!priv->nss_ctx) {
		nss_portifmgr_warn("%p: %s registration to NSS not completed yet\n", dev, dev->name);
		return -EAGAIN;
	}
	netif_start_queue(dev);
	return 0;
}

/*
 * nss_portifmgr_close()
 *	Netdev's close call.
 */
static int nss_portifmgr_close(struct net_device *dev)
{
	netif_stop_queue(dev);
	return 0;
}

/*
 * nss_portifmgr_start_xmit()
 *	Transmit skb to NSS FW over portid if_num.
 */
static netdev_tx_t nss_portifmgr_start_xmit(struct sk_buff *skb, struct net_device *dev)
{

	struct net_device_stats *stats = &dev->stats;
	struct nss_portifmgr_priv *priv;
	const unsigned char *dest = skb->data;
	struct sk_buff *tx_skb = skb;
	nss_tx_status_t status;

	priv = netdev_priv(dev);

	/*
	 * Drop if trying to xmit when registration to NSS is not completed yet.
	 */
	if (!priv->nss_ctx) {
		nss_portifmgr_warn("%p: sending without ctx: if_num %d port_id %d\n",
					dev, priv->port_id, priv->if_num);
		goto drop;
	}

	if (skb_headroom(skb) < NSS_PORTIFMGR_EXTRA_HEADER_SIZE) {
		nss_portifmgr_warn("%p: headroom not enough skb: %p\n", dev, skb);
		goto drop;
	}

	/*
	 * For a multicast/broadcast skb, linux is trying to send the same
	 * skb->data to all interfaces. But we are going to add port header
	 * inside the eth_header. The 2nd interface will get a skb that eth_hdr
	 * is already have a portid header. We need to make a copy to avoid this
	 */
	if (!is_unicast_ether_addr(dest)) {
		tx_skb = skb_copy(skb, GFP_KERNEL);
		kfree_skb(skb);
		if (!tx_skb) {
			nss_portifmgr_warn("%p: failed to copy skb\n", dev);
			stats->tx_dropped++;
			return NETDEV_TX_OK;
		}
	}

	status = nss_portid_if_tx_data(priv->nss_ctx, tx_skb, priv->if_num);
	if (likely(status == NSS_TX_SUCCESS)) {
		return NETDEV_TX_OK;
	}
	nss_portifmgr_warn("%p: portid interface failed to xmit the packet : %d\n",
								dev, status);

drop:
	kfree_skb(tx_skb);
	stats->tx_dropped++;
	return NETDEV_TX_OK;
}

/*
 * nss_portifmgr_get_stats()
 *	Netdev get stats function to get port stats
 */
static struct rtnl_link_stats64 *nss_portifmgr_get_stats(struct net_device *dev, struct rtnl_link_stats64 *stats)
{
	struct nss_portifmgr_priv *priv = (struct nss_portifmgr_priv *)netdev_priv(dev);
	BUG_ON(priv == NULL);

	nss_portid_get_stats(priv->if_num, stats);
	return stats;
}

/*
 * nss_portifmgr_change_mtu()
 *	Netdev change mtu function
 */
int nss_portifmgr_change_mtu(struct net_device *dev, int new_mtu)
{
	struct nss_portifmgr_priv *priv = (struct nss_portifmgr_priv *)netdev_priv(dev);

	if ((new_mtu + NSS_PORTIFMGR_EXTRA_HEADER_SIZE) > priv->real_dev->mtu) {
		return -ERANGE;
	}

	dev->mtu = new_mtu;
	return 0;
}

/*
 * nss_portifmgr_netdev_ops
 *	Netdev operations.
 */
static const struct net_device_ops nss_portifmgr_netdev_ops = {
	.ndo_open		= nss_portifmgr_open,
	.ndo_stop		= nss_portifmgr_close,
	.ndo_start_xmit		= nss_portifmgr_start_xmit,
	.ndo_set_mac_address	= eth_mac_addr,
	.ndo_change_mtu		= nss_portifmgr_change_mtu,
	.ndo_get_stats64	= nss_portifmgr_get_stats,
};

/*
 * nss_portifmgr_create_if()
 *	API to create netdev and dynamic interfaces that mapped to switch port and gmac
 */
struct net_device *nss_portifmgr_create_if(int port_id, int gmac_id, const char *name)
{
	struct net_device *ndev, *real_dev;
	struct nss_ctx_instance *nss_ctx;
	struct nss_portifmgr_priv *priv;
	int err, if_num;
	nss_tx_status_t status;

	real_dev = nss_gmac_get_netdev_by_macid(gmac_id);
	if (!real_dev) {
		nss_portifmgr_warn("Underlying gmac%d netdevice does not exist!\n", gmac_id);
		return NULL;
	}

	ndev = alloc_etherdev(sizeof(struct nss_portifmgr_priv));
	if (!ndev) {
		nss_portifmgr_warn("Error allocating netdev\n");
		return NULL;
	}

	/*
	 * Setup net_device
	 */
	ndev->netdev_ops = &nss_portifmgr_netdev_ops;
	ndev->needed_headroom = NSS_PORTIFMGR_EXTRA_HEADER_SIZE;
	ndev->features |= NSS_PORTIFMGR_SUPPORTED_FEATURES;
	ndev->hw_features |= NSS_PORTIFMGR_SUPPORTED_FEATURES;
	ndev->vlan_features |= NSS_PORTIFMGR_SUPPORTED_FEATURES;
	ndev->wanted_features |= NSS_PORTIFMGR_SUPPORTED_FEATURES;
	ndev->mtu = real_dev->mtu - NSS_PORTIFMGR_EXTRA_HEADER_SIZE;
	strlcpy(ndev->name, name, IFNAMSIZ);

	/*
	 * Setup temp mac address, this can be changed with ifconfig later
	 */
	memcpy(ndev->dev_addr, "\x00\x03\x7f\x11\x22\x00", ETH_ALEN);
	ndev->dev_addr[5] = 0x10 + port_id;

	err = register_netdev(ndev);
	if (err) {
		nss_portifmgr_warn("Register_netdev() fail with error :%d\n", err);
		free_netdev(ndev);
		return NULL;
	}

	if_num = nss_dynamic_interface_alloc_node(NSS_DYNAMIC_INTERFACE_TYPE_PORTID);
	if (if_num < 0) {
		nss_portifmgr_warn("Dynamic interface alloc failed\n");
		goto fail;
	}

	/*
	 * Register port interface with NSS
	 */
	nss_ctx = nss_portid_register_port_if(if_num, port_id, ndev, nss_portifmgr_receive_pkt);
	if (!nss_ctx) {
		goto fail2;
	}

	/*
	 * Fill in information in the netdev priv structure
	 */
	priv = netdev_priv(ndev);
	priv->nss_ctx = nss_ctx;
	priv->real_dev = real_dev;
	priv->if_num = if_num;
	priv->port_id = port_id;

	/*
	 * Send msg to configure port in NSS FW
	 */
	status = nss_portid_tx_configure_port_if_msg(nss_ctx, if_num, port_id, gmac_id);
	if (status == NSS_TX_SUCCESS) {
		dev_hold(real_dev);
		return ndev;
	}

	/*
	 * Failure handling
	 */
	nss_portid_unregister_port_if(if_num);
fail2:
	(void)nss_dynamic_interface_dealloc_node(if_num, NSS_DYNAMIC_INTERFACE_TYPE_PORTID);

fail:
	unregister_netdev(ndev);
	free_netdev(ndev);
	return NULL;
}
EXPORT_SYMBOL(nss_portifmgr_create_if);

/*
 * nss_portifmgr_destroy_if()
 *	API to destroy netdev and the dynamic interfaces associated with it
 */
void nss_portifmgr_destroy_if(struct net_device *ndev)
{
	struct nss_portifmgr_priv *priv = netdev_priv(ndev);
	nss_tx_status_t status;

	/*
	 * Release ref for the real gmac net_device
	 */
	dev_put(priv->real_dev);

	/*
	 * Unconfigure port
	 */
	status = nss_portid_tx_unconfigure_port_if_msg(priv->nss_ctx, priv->if_num, priv->port_id);
	if (status != NSS_TX_SUCCESS) {
		nss_portifmgr_warn("%p: destroy if_num %d failed\n", priv->nss_ctx, priv->if_num);
	}

	/*
	 * Unregister from NSS
	 */
	nss_portid_unregister_port_if(priv->if_num);

	/*
	 * Release dynamic interface
	 */
	(void)nss_dynamic_interface_dealloc_node(priv->if_num, NSS_DYNAMIC_INTERFACE_TYPE_PORTID);

	/*
	 * Unregister net_device
	 */
	rtnl_is_locked() ? unregister_netdevice(ndev) : unregister_netdev(ndev);
}
EXPORT_SYMBOL(nss_portifmgr_destroy_if);

#ifdef NSS_PORTIFMGR_REF_AP148
/*
 * nss_portifmgr_create_interfaces_ap148()
 *	Sample function to create port interfaces on AP148 ref design
 */
void nss_portifmgr_create_interfaces_ap148(void)
{
	int i = 0;
	ndev_list[i] = nss_portifmgr_create_if(1, 2, "lan%d");
	if (!ndev_list[i]) {
		nss_portifmgr_warn("Creating lan0 interface on port 1 failed\n");
		return;
	}

	ndev_list[++i] = nss_portifmgr_create_if(2, 2, "lan%d");
	if (!ndev_list[i]) {
		nss_portifmgr_warn("Creating lan1 interface on port 2 failed\n");
		return;
	}

	ndev_list[++i] = nss_portifmgr_create_if(3, 2, "lan%d");
	if (!ndev_list[i]) {
		nss_portifmgr_warn("Creating lan2 interface on port 3 failed\n");
		return;
	}
}

/*
 * nss_portifmgr_destroy_all_interfaces()
 *	Destroy all the created interfaces
 */
void nss_portifmgr_destroy_all_interfaces(void)
{
	int i;

	for (i = 0; i < NSS_PORTID_MAX_SWITCH_PORT; i++) {
		if (ndev_list[i]) {
			nss_portifmgr_destroy_if(ndev_list[i]);
			ndev_list[i] = NULL;
		}
	}
}
#endif

/*
 * nss_portifmgr_init_module()
 *	portifmgr module init function
 */
int __init nss_portifmgr_init_module(void)
{
	pr_info("module (platform - IPQ806x , Build %s) loaded\n",
			 NSS_CLIENT_BUILD_ID);

#ifdef NSS_PORTIFMGR_REF_AP148
	nss_portifmgr_create_interfaces_ap148();
#endif
	return 0;
}

/*
 * nss_portifmgr_exit_module()
 *	portifmgr module exit function
 */
void __exit nss_portifmgr_exit_module(void)
{
#ifdef NSS_PORTIFMGR_REF_AP148
	nss_portifmgr_destroy_all_interfaces();
#endif
	pr_info("module unloaded\n");
}

module_init(nss_portifmgr_init_module);
module_exit(nss_portifmgr_exit_module);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS port interface manager");
