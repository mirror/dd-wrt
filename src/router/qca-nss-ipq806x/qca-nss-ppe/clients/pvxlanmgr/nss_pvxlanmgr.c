/*
 **************************************************************************
 * Copyright (c) 2019-2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2023-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * nss_pvxlanmgr.c
 *	NSS to HLOS PVxLAN manager
 */
#include "nss_pvxlanmgr_priv.h"

static bool ppe_to_host = true;

/*
 * nss_pvxlanmgr_stats_update()
 *	Update the netdev stats
 */
static void nss_pvxlanmgr_stats_update(void *app_data, struct nss_pvxlan_msg *msg)
{
	struct net_device *dev = (struct net_device *)app_data;
	struct nss_pvxlanmgr_priv *priv;
	struct nss_pvxlan_stats_msg *stats;
	struct rtnl_link_stats64 *netdev_stats;
	struct nss_cmn_msg *ncm;
	uint32_t i, if_num;
	uint64_t dropped = 0;

	ncm = (struct nss_cmn_msg *)msg;
	if_num = ncm->interface;

	stats = &msg->msg.stats;
	dev_hold(dev);
	priv = netdev_priv(dev);
	netdev_stats = &priv->stats;

	/*
	 * Only look at the tx_packets/tx_bytes for both host_inner/outer interfaces.
	 * rx_bytes/rx_packets are increased when the packet is received by the node.
	 * Therefore, it includes both transmitted/dropped packets. tx_bytes/tx_packets
	 * reflect successfully transmitted packets.
	 */
	if (if_num == priv->if_num_host_inner) {
		netdev_stats->tx_packets += stats->node_stats.tx_packets;
		for (i = 0; i < NSS_MAX_NUM_PRI; i++) {
			dropped += stats->node_stats.rx_dropped[i];
		}
		dropped += stats->mac_db_lookup_failed;
		dropped += stats->udp_encap_lookup_failed;
		netdev_stats->tx_bytes += stats->node_stats.tx_bytes;
		dropped += stats->dropped_hroom;
		dropped += stats->dropped_ver_mis;
		dropped += stats->dropped_zero_sized_packet;
		dropped += stats->dropped_next_node_queue_full;
		netdev_stats->tx_dropped += dropped;
		dev_put(dev);
		return;
	}

	netdev_stats->rx_packets += stats->node_stats.tx_packets;
	for (i = 0; i < NSS_MAX_NUM_PRI; i++) {
		dropped += stats->node_stats.rx_dropped[i];
	}
	dropped += stats->dropped_malformed;
	dropped += stats->dropped_next_node_queue_full;
	dropped += stats->dropped_pbuf_alloc_failed;
	dropped += stats->dropped_linear_failed;

	netdev_stats->rx_bytes += stats->node_stats.tx_bytes;
	netdev_stats->rx_dropped += dropped;
	dev_put(dev);
}

/*
 * nss_pvxlanmgr_start_xmit()
 *	Transmit's skb to NSS FW over PVxLAN if_num.
 *
 * Please make sure to leave headroom of NSS_PVXLAN_HEADROOM with every
 * packet so that NSS can encap eth,vlan,ip,udp,pvxlan headers.
 * Also, skb->len must include size of metaheader. Essentially skb->len is
 * size of PVxLAN Payload and metaheader.
 */
static netdev_tx_t nss_pvxlanmgr_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct net_device_stats *stats = &dev->stats;
	struct nss_pvxlanmgr_priv *priv;
	uint32_t if_num_host_inner;
	nss_tx_status_t status;
	int32_t extra_head = skb_headroom(skb) - dev->needed_headroom;

	if (unlikely(extra_head < 0)) {
		nss_pvxlanmgr_warn("%px: skb ( %px ) does not have enough headroom\n", dev, skb);
		kfree_skb(skb);
		stats->tx_dropped++;
		return NETDEV_TX_OK;
	}

	priv = netdev_priv(dev);
	if_num_host_inner = priv->if_num_host_inner;
	if (unlikely(if_num_host_inner == 0)) {
		nss_pvxlanmgr_warn("%px: (PVXLAN packet) if_num in the tunnel not set pre->if_num_host_inner %d\n", dev,
				priv->if_num_host_inner);
		kfree_skb(skb);
		stats->tx_dropped++;
		return NETDEV_TX_OK;
	}

	status = nss_pvxlan_tx_buf(priv->pvxlan_ctx, skb, if_num_host_inner);
	if (unlikely(status != NSS_TX_SUCCESS)) {
		if (status == NSS_TX_FAILURE_QUEUE) {
			nss_pvxlanmgr_warn("%px: netdev :%px queue is full", dev, dev);
			if (!netif_queue_stopped(dev)) {
				netif_stop_queue(dev);
			}
			nss_pvxlanmgr_warn("%px: (PVxLAN packet) Failed to xmit the packet because of the tx failure : %d\n", dev, status);
			return NETDEV_TX_BUSY;
		}

		nss_pvxlanmgr_warn("%px: (PVxLAN packet) Failed to xmit the packet because of non-tx failure reason : %d\n", dev, status);
		kfree_skb(skb);
		stats->tx_dropped++;
	}

	return NETDEV_TX_OK;
}

/*
 * nss_pvxlanmgr_receive_pkt()
 *	Receives a pkt from NSS
 */
static void nss_pvxlanmgr_receive_pkt(struct net_device *dev, struct sk_buff *skb, struct napi_struct *napi)
{
	/*
	 * SKB NETIF START
	 */
	dev_hold(dev);
	skb->dev = dev;
	skb->pkt_type = PACKET_OTHERHOST;
	skb->skb_iif = dev->ifindex;
	skb_reset_mac_header(skb);
	skb_reset_network_header(skb);
	skb_reset_transport_header(skb);
	netif_receive_skb(skb);

	/*
	 * SKB NETIF END
	 */
	dev_put(dev);
}

/*
 * nss_pvxlanmgr_receive_pkt_ppe_vp()
 *	Receives a pkt from PPE
 */
static bool nss_pvxlanmgr_receive_pkt_ppe_vp(struct ppe_vp_cb_info *info, void *cb_data)
{
	nss_pvxlanmgr_receive_pkt(info->skb->dev, info->skb, NULL);
	return true;
}

/*
 * nss_pvxlanmgr_get_tunnel_stats()
 *	Netdev get stats function to get tunnel stats
 */
static struct rtnl_link_stats64 *nss_pvxlanmgr_get_tunnel_stats(struct net_device *dev, struct rtnl_link_stats64 *stats)
{
	struct nss_pvxlanmgr_priv *priv;

	if (!stats) {
		nss_pvxlanmgr_warn("%px: invalid rtnl structure\n", dev);
		return stats;
	}

	dev_hold(dev);

	/*
	 * Netdev seems to be incrementing rx_dropped because we don't give IP header.
	 * So reset it as it's of no use for us.
	 */
	atomic_long_set(&dev->rx_dropped, 0);
	priv = netdev_priv(dev);
	memset(stats, 0, sizeof(struct rtnl_link_stats64));
	memcpy(stats, &priv->stats, sizeof(struct rtnl_link_stats64));
	dev_put(dev);

	return stats;
}

/*
 * nss_pvxlanmgr_dev_tunnel_stats()
 *	Netdev ops function to retrieve stats for kernel version > 4.6
 */
static void nss_pvxlanmgr_dev_tunnel_stats(struct net_device *dev, struct rtnl_link_stats64 *stats)
{
	nss_pvxlanmgr_get_tunnel_stats(dev, stats);
}

/*
 * nss_pvxlanmgr_unregister_with_nss()
 *	Internal function to unregister with NSS FW
 */
static void nss_pvxlanmgr_unregister_with_nss(uint32_t if_num)
{
	nss_pvxlanmgr_trace("%d: unregister with NSS FW\n", if_num);
	nss_pvxlan_unregister(if_num);
}

/*
 * nss_pvxlanmgr_register_with_nss()
 *	Internal function to register with NSS FW.
 */
static nss_pvxlanmgr_status_t nss_pvxlanmgr_register_with_nss(uint32_t if_num, struct net_device *dev)
{
	struct nss_ctx_instance *ctx;

	ctx = nss_pvxlan_register(if_num, nss_pvxlanmgr_receive_pkt, nss_pvxlanmgr_stats_update, dev, 0);
	if (!ctx) {
		nss_pvxlanmgr_warn("%px: %d: nss_pvxlanmgr_data_register failed\n", dev, if_num);
		return NSS_PVXLANMGR_FAILURE;
	}
	return NSS_PVXLANMGR_SUCCESS;
}

/*
 * nss_pvxlanmgr_open()
 *	Netdev's open call.
 */
static int nss_pvxlanmgr_open(struct net_device *dev)
{
	netif_start_queue(dev);
	return 0;
}

/*
 * nss_pvxlanmgr_close()
 *	Netdev's close call.
 */
static int nss_pvxlanmgr_close(struct net_device *dev)
{
	netif_stop_queue(dev);
	return 0;
}

/*
 * nss_pvxlanmgr_decongestion_callback()
 *	Wakeup netif queue if we were stopped by start_xmit
 */
static void nss_pvxlanmgr_decongestion_callback(void *arg)
{
	struct net_device *dev = arg;

	/*
	 * TODO: Add a check whether tunnel is disabled or not before waking it up.
	 */
	if (netif_queue_stopped(dev)) {
		netif_wake_queue(dev);
	}
}

/*
 * nss_pvxlanmgr_netdev_ops
 *	Netdev operations.
 */
static const struct net_device_ops nss_pvxlanmgr_netdev_ops = {
	.ndo_open		= nss_pvxlanmgr_open,
	.ndo_stop		= nss_pvxlanmgr_close,
	.ndo_start_xmit		= nss_pvxlanmgr_start_xmit,
	.ndo_set_mac_address	= eth_mac_addr,
	.ndo_get_stats64	= nss_pvxlanmgr_dev_tunnel_stats,
};

/*
 * nss_pvxlanmgr_netdev_setup()
 *	Netdev setup function.
 */
static void nss_pvxlanmgr_netdev_setup(struct net_device *dev)
{
	dev->addr_len = ETH_ALEN;
	dev->mtu = ETH_DATA_LEN;
	dev->needed_headroom = NSS_PVXLAN_HEADROOM;
	dev->type = ARPHRD_VOID;
	dev->ethtool_ops = NULL;
	dev->header_ops = NULL;
	dev->netdev_ops = &nss_pvxlanmgr_netdev_ops;
	dev->priv_destructor = NULL;

	memset(dev->dev_addr, 0x00, dev->addr_len);
	memset(dev->broadcast, 0xff, dev->addr_len);
	memcpy(dev->perm_addr, dev->dev_addr, dev->addr_len);
}

/*
 * nss_pvxlanmgr_netdev_event()
 *	Netdevice notifier for NSS PVxLAN manager module
 */
static int nss_pvxlanmgr_netdev_event(struct notifier_block  *nb, unsigned long event, void  *dev)
{
	struct net_device *netdev = (struct net_device *)dev;

	if (strstr(netdev->name, NSS_PVXLANMGR_NETDEV_NAME) == NULL) {
		return NOTIFY_DONE;
	}

	switch (event) {
	case NETDEV_UP:
		nss_pvxlanmgr_trace("%px: NETDEV_UP: event %lu name %s\n", netdev, event, netdev->name);
		nss_pvxlanmgr_netdev_enable(netdev);
		break;

	case NETDEV_DOWN:
		nss_pvxlanmgr_trace("%px: NETDEV_DOWN: event %lu name %s\n", netdev, event, netdev->name);
		nss_pvxlanmgr_netdev_disable(netdev);
		break;

	default:
		nss_pvxlanmgr_trace("%px: Unhandled notifier event %lu name %s\n", netdev, event, netdev->name);
		break;
	}

	return NOTIFY_DONE;
}

/*
 * Linux Net device Notifier
 */
static struct notifier_block nss_pvxlanmgr_netdev_notifier = {
	.notifier_call = nss_pvxlanmgr_netdev_event,
};

/*
 * nss_pvxlanmgr_get_if_num_outer()
 *	Get the if_num outer for the netdevice.
 */
uint32_t nss_pvxlanmgr_get_if_num_outer(struct net_device *dev)
{
	struct nss_pvxlanmgr_priv *priv;
	priv = netdev_priv(dev);

	return priv->if_num_outer;
}
EXPORT_SYMBOL(nss_pvxlanmgr_get_if_num_outer);

/*
 * nss_pvxlanmgr_get_if_num_host_inner()
 *	Get the host inner interface number for the netdevice.
 */
uint32_t nss_pvxlanmgr_get_if_num_host_inner(struct net_device *dev)
{
	struct nss_pvxlanmgr_priv *priv;
	priv = netdev_priv(dev);

	return priv->if_num_host_inner;
}
EXPORT_SYMBOL(nss_pvxlanmgr_get_if_num_host_inner);

/*
 * nss_pvxlanmgr_netdev_disable()
 *	API for disabling a data tunnel
 */
nss_pvxlanmgr_status_t nss_pvxlanmgr_netdev_disable(struct net_device *dev)
{
	struct nss_pvxlanmgr_priv *priv;
	nss_tx_status_t ret;

	dev_hold(dev);
	priv = netdev_priv(dev);
	nss_pvxlanmgr_info("%px: tunnel disable is being called\n", dev);
	ret = nss_pvxlanmgr_tunnel_tx_msg_disable(priv->pvxlan_ctx, priv->if_num_host_inner);
	if (ret != NSS_TX_SUCCESS) {
		nss_pvxlanmgr_warn("%px: Tunnel disable failed: %d\n", dev, ret);
		dev_put(dev);
		return ret;
	}

	ret = nss_pvxlanmgr_tunnel_tx_msg_disable(priv->pvxlan_ctx, priv->if_num_outer);
	if (ret != NSS_TX_SUCCESS) {
		nss_pvxlanmgr_warn("%px: Tunnel disable failed: %d\n", dev, ret);
		nss_pvxlanmgr_tunnel_tx_msg_enable(priv->pvxlan_ctx, priv->if_num_host_inner, priv->if_num_outer);
		dev_put(dev);
		return ret;
	}

	dev_put(dev);
	return ret;
}
EXPORT_SYMBOL(nss_pvxlanmgr_netdev_disable);

/*
 * nss_pvxlanmgr_netdev_enable()
 *	API for enabling a data tunnel
 */
nss_pvxlanmgr_status_t nss_pvxlanmgr_netdev_enable(struct net_device *dev)
{
	struct nss_pvxlanmgr_priv *priv;
	nss_tx_status_t ret;

	dev_hold(dev);
	priv = netdev_priv(dev);
	nss_pvxlanmgr_info("%px: tunnel enable is being called\n", dev);
	ret = nss_pvxlanmgr_tunnel_tx_msg_enable(priv->pvxlan_ctx, priv->if_num_host_inner, priv->if_num_outer);
	if (ret != NSS_TX_SUCCESS) {
		nss_pvxlanmgr_warn("%px: Tunnel enable failed: %d\n", dev, ret);
		dev_put(dev);
		return ret;
	}

	ret = nss_pvxlanmgr_tunnel_tx_msg_enable(priv->pvxlan_ctx, priv->if_num_outer, priv->if_num_host_inner);
	if (ret != NSS_TX_SUCCESS) {
		nss_pvxlanmgr_warn("%px: Tunnel enable failed: %d\n", dev, ret);
		nss_pvxlanmgr_tunnel_tx_msg_disable(priv->pvxlan_ctx, priv->if_num_host_inner);
		dev_put(dev);
		return ret;
	}

	dev_put(dev);
	return ret;
}
EXPORT_SYMBOL(nss_pvxlanmgr_netdev_enable);

/*
 * nss_pvxlanmgr_netdev_destroy()
 *	API for destroying a netdevice.
 *
 * The PVxLAN tunnel must be destroyed first before netdevice.
 * Even though unregistrations fail during this process, we can't stop and return.
 * That will cause a resource to never be freed. Therefore, we print warning if any
 * of the unregistration fails but continue to free the resources.
 */
nss_pvxlanmgr_status_t nss_pvxlanmgr_netdev_destroy(struct net_device *dev)
{
	struct nss_pvxlanmgr_priv *priv;
	uint32_t if_num_host_inner, if_num_outer, i;
	ppe_vp_num_t vp_num;
	nss_tx_status_t nss_status;
	struct nss_pvxlanmgr_tunnel *t;

	priv = netdev_priv(dev);
	if_num_host_inner = priv->if_num_host_inner;
	if_num_outer = priv->if_num_outer;
	vp_num = priv->vp_num;

	if (nss_cmn_unregister_queue_decongestion(priv->pvxlan_ctx, nss_pvxlanmgr_decongestion_callback) != NSS_CB_UNREGISTER_SUCCESS) {
		nss_pvxlanmgr_warn("%px: failed to unregister decongestion callback\n", dev);
	}

	/*
	 * Delete all tunnels associated with this net_device.
	 */
	for (i = 0; i < NSS_PVXLANMGR_MAX_TUNNELS; i++) {
		t = nss_pvxlanmgr_tunnel_get(dev, i);
		if (t) {
			nss_pvxlanmgr_warn("%px: tunnel %d: exist during the destruction\n", dev, i);
			nss_pvxlanmgr_tunnel_destroy(dev, i);
		}
	}

	/*
	 * Unconfigure VP associated to the netdevice.
	 */
	nss_pvxlanmgr_tunnel_unconfig_vp(priv->pvxlan_ctx, if_num_host_inner, vp_num);
	nss_pvxlanmgr_tunnel_unconfig_vp(priv->pvxlan_ctx, if_num_outer, vp_num);

	/*
	 * Deallocate dynamic interface
	 */
	nss_pvxlanmgr_unregister_with_nss(if_num_host_inner);
	nss_status = nss_dynamic_interface_dealloc_node(if_num_host_inner, NSS_DYNAMIC_INTERFACE_TYPE_PVXLAN_HOST_INNER);
	if (nss_status != NSS_TX_SUCCESS) {
		nss_pvxlanmgr_warn("%px: Dealloc of dynamic interface failed: %d\n",
			dev, if_num_host_inner);
	}

	/*
	 * Deallocate dynamic interface
	 */
	nss_pvxlanmgr_unregister_with_nss(if_num_outer);
	nss_status = nss_dynamic_interface_dealloc_node(if_num_outer, NSS_DYNAMIC_INTERFACE_TYPE_PVXLAN_OUTER);
	if (nss_status != NSS_TX_SUCCESS) {
		nss_pvxlanmgr_warn("%px: Dealloc of dynamic interface failed for tunnel: %d\n",
			dev, if_num_outer);
	}

	priv = netdev_priv(dev);
	ppe_vp_free(priv->vp_num);
	unregister_netdev(dev);
	kfree(priv->tunnel);
	free_netdev(dev);
	return NSS_PVXLANMGR_SUCCESS;
}
EXPORT_SYMBOL(nss_pvxlanmgr_netdev_destroy);

/*
 * nss_pvxlanmgr_netdev_create()
 *	API to create a PVxLAN netdev
 */
struct net_device *nss_pvxlanmgr_netdev_create()
{
	struct nss_pvxlanmgr_priv *priv;
	struct net_device *dev;
	int32_t pvxlan_if_num_host_inner, pvxlan_if_num_outer;
	int err;
	struct ppe_vp_ai vpai = {0};
	ppe_vp_num_t vp_num;
	struct nss_ctx_instance *ctx;

	dev = alloc_netdev(sizeof(struct nss_pvxlanmgr_priv),
					"nsspvxlan%d", NET_NAME_UNKNOWN, nss_pvxlanmgr_netdev_setup);
	if (!dev) {
		nss_pvxlanmgr_warn("Error allocating netdev\n");
		return NULL;
	}

	vpai.type = PPE_VP_TYPE_SW_PO;
	vpai.queue_num = edma_cfg_rx_point_offload_ring_queue_get();

	/*
	 * Check if ppe to host mode is enabled.
	 */
	if (ppe_to_host) {
		vpai.src_cb = &nss_pvxlanmgr_receive_pkt_ppe_vp;
	}

	/*
	 * Allocate a PPE VP
	 */
	vp_num = ppe_vp_alloc(dev, &vpai);
	if (vp_num == -1) {
		nss_pvxlanmgr_warn("%px: VP alloc failed", dev);
		goto fail;
	}

	priv = netdev_priv(dev);
	priv->vp_num = vp_num;
	ctx = nss_pvxlan_get_ctx();
	if (!ctx) {
		nss_pvxlanmgr_warn("%px: failed to find pvxlan context\n", dev);
		goto fail1;
	}

	priv->pvxlan_ctx = ctx;

	priv->tunnel = kzalloc(sizeof(struct nss_pvxlanmgr_tunnel) * NSS_PVXLANMGR_MAX_TUNNELS, GFP_ATOMIC);
	if (!priv->tunnel) {
		nss_pvxlanmgr_warn("%px: failed to allocate tunnel memory\n", dev);
		goto fail1;
	}

	if (nss_cmn_register_queue_decongestion(priv->pvxlan_ctx, nss_pvxlanmgr_decongestion_callback, dev) != NSS_CB_REGISTER_SUCCESS) {
		nss_pvxlanmgr_warn("%px: failed to register decongestion callback\n", dev);
		goto fail2;
	}

	pvxlan_if_num_host_inner = nss_dynamic_interface_alloc_node(NSS_DYNAMIC_INTERFACE_TYPE_PVXLAN_HOST_INNER);
	if (pvxlan_if_num_host_inner < 0) {
		nss_pvxlanmgr_warn("%px: di returned error : %d\n", dev, pvxlan_if_num_host_inner);
		goto fail3;
	}

	pvxlan_if_num_outer = nss_dynamic_interface_alloc_node(NSS_DYNAMIC_INTERFACE_TYPE_PVXLAN_OUTER);
	if (pvxlan_if_num_outer < 0) {
		nss_pvxlanmgr_warn("%px: di returned error : %d\n", dev, pvxlan_if_num_outer);
		goto fail4;
	}

	if (nss_pvxlanmgr_register_with_nss(pvxlan_if_num_host_inner, dev) != NSS_PVXLANMGR_SUCCESS) {
		nss_pvxlanmgr_warn("%d: NSS PVXLAN register with NSS failed", pvxlan_if_num_host_inner);
		goto fail5;
	}

	if (nss_pvxlanmgr_register_with_nss(pvxlan_if_num_outer, dev) != NSS_PVXLANMGR_SUCCESS) {
		nss_pvxlanmgr_warn("%d: NSS PVXLAN register with NSS failed", pvxlan_if_num_outer);
		goto fail6;
	}

	if (nss_pvxlanmgr_tunnel_config_vp(ctx, pvxlan_if_num_outer, vp_num, ppe_to_host)!= NSS_TX_SUCCESS) {
		nss_pvxlanmgr_warn("%px: NSS PVXLAN vp_num %d update failed for "
				"pvxlan outer interface %d\n", ctx, vp_num, pvxlan_if_num_outer);
		goto fail7;
	}

	if (nss_pvxlanmgr_tunnel_config_vp(ctx, pvxlan_if_num_host_inner, vp_num,  ppe_to_host)!= NSS_TX_SUCCESS) {
		nss_pvxlanmgr_warn("%px: NSS PVXLAN vp_num %d update failed for "
				"pvxlan inner interface %d\n", ctx, vp_num, pvxlan_if_num_host_inner);
		goto fail8;
	}

	err = register_netdev(dev);
	if (err) {
		nss_pvxlanmgr_warn("register_netdev() fail with error :%d\n", err);
		goto fail9;
	}

	priv->if_num_host_inner = pvxlan_if_num_host_inner;
	priv->if_num_outer = pvxlan_if_num_outer;

	return dev;

fail9:
	nss_pvxlanmgr_tunnel_unconfig_vp(ctx, pvxlan_if_num_host_inner, vp_num);
fail8:
	nss_pvxlanmgr_tunnel_unconfig_vp(ctx, pvxlan_if_num_outer, vp_num);
fail7:
	nss_pvxlanmgr_unregister_with_nss(pvxlan_if_num_outer);
fail6:
	nss_pvxlanmgr_unregister_with_nss(pvxlan_if_num_host_inner);
fail5:
	nss_dynamic_interface_dealloc_node(pvxlan_if_num_outer, NSS_DYNAMIC_INTERFACE_TYPE_PVXLAN_OUTER);
fail4:
	nss_dynamic_interface_dealloc_node(pvxlan_if_num_host_inner, NSS_DYNAMIC_INTERFACE_TYPE_PVXLAN_HOST_INNER);
fail3:
	nss_cmn_unregister_queue_decongestion(priv->pvxlan_ctx, nss_pvxlanmgr_decongestion_callback);
fail2:
	kfree(priv->tunnel);
fail1:
	ppe_vp_free(vp_num);
fail:
	free_netdev(dev);
	return NULL;
}
EXPORT_SYMBOL(nss_pvxlanmgr_netdev_create);

/*
 * nss_pvxlanmgr_exit_module()
 *	Tunnel PVxLAN module exit function
 */
static void __exit nss_pvxlanmgr_exit_module(void)
{
	int ret;
#ifdef CONFIG_OF
	/*
	 * If the node is not compatible, don't do anything.
	 */
	if (!of_find_node_by_name(NULL, "nss-common")) {
		return;
	}
#endif

	ret = unregister_netdevice_notifier(&nss_pvxlanmgr_netdev_notifier);
	if (ret) {
		nss_pvxlanmgr_warn("failed to unregister netdevice notifier: error %d\n", ret);
	}

	nss_pvxlanmgr_info("module unloaded\n");
}

/*
 * nss_pvxlanmgr_init_module()
 *	Tunnel PVxLAN module init function
 */
static int __init nss_pvxlanmgr_init_module(void)
{
	int ret;
#ifdef CONFIG_OF
	/*
	 * If the node is not compatible, don't do anything.
	 */
	if (!of_find_node_by_name(NULL, "nss-common")) {
		return 0;
	}
#endif

	nss_pvxlanmgr_info("module %s loaded\n",
			   NSS_PPE_BUILD_ID);

	ret = register_netdevice_notifier(&nss_pvxlanmgr_netdev_notifier);
	if (ret) {
		nss_pvxlanmgr_warn("failed to register netdevice notifier: error %d/n", ret);
	}

	return ret;
}

module_init(nss_pvxlanmgr_init_module);
module_exit(nss_pvxlanmgr_exit_module);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS PVxLAN manager");
