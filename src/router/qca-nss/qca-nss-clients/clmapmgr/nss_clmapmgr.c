/*
 **************************************************************************
 * Copyright (c) 2019-2020 The Linux Foundation. All rights reserved.
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
 * nss_clmapmgr.c
 *  This file implements client for CLient map manager.
 */

#include <linux/version.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/if_ether.h>
#include <linux/if_vlan.h>

#include <nss_api_if.h>
#include <nss_dynamic_interface.h>

#include "nss_clmapmgr_private.h"
#include "nss_clmapmgr.h"
#include "nss_eogremgr.h"

#define NSS_CLMAPMGR_CMD_MAX_RETRY_COUNT 3
#define NSS_CLMAP_MAX_HEADROOM NSS_EOGREMGR_MAX_HEADROOM

/*
 * nss_clmapmgr_dev_xmit()
 *	Netdev ops function to send packet to NSS.
 */
static netdev_tx_t nss_clmapmgr_dev_xmit(struct sk_buff *skb, struct net_device *dev)
{
	nss_tx_status_t status;
	int if_number;
	struct nss_ctx_instance *clmap_ctx;
	struct nss_clmapmgr_priv_t *priv = netdev_priv(dev);

	if_number = priv->nss_if_number_us;
	if (unlikely(if_number <= 0)) {
		nss_clmapmgr_info("%px: clmapmgr dev is not registered with nss\n", dev);
		goto fail;
	}

	clmap_ctx = nss_clmap_get_ctx();
	if (unlikely(!clmap_ctx)) {
		nss_clmapmgr_info("%px: NSS clmapmgr context not found.\n", dev);
		goto fail;
	}

	status = nss_clmap_tx_buf(clmap_ctx, skb, (uint32_t)if_number);
	if (unlikely(status != NSS_TX_SUCCESS)) {
		if (likely(status == NSS_TX_FAILURE_QUEUE)) {
			nss_clmapmgr_warning("%px: netdev :%px queue is full", dev, dev);
			if (!netif_queue_stopped(dev)) {
				netif_stop_queue(dev);
			}
			nss_clmapmgr_warning("%px: (CLMAP packet) Failed to xmit the packet because of tx queue full, status: %d\n", dev, status);
			return NETDEV_TX_BUSY;
		}
		nss_clmapmgr_info("%px: NSS clmapmgr could not send packet to NSS %d\n", dev, if_number);
		goto fail;
	}

	return NETDEV_TX_OK;

fail:
	dev->stats.tx_dropped++;
	dev_kfree_skb_any(skb);
	return NETDEV_TX_OK;
}

/*
 * nss_clmapmgr_dev_stats64()
 *	Netdev ops function to retrieve stats.
 */
void nss_clmapmgr_dev_stats64(struct net_device *dev,
						struct rtnl_link_stats64 *stats)
{
	struct nss_clmapmgr_priv_t *priv;

	if (!stats) {
		nss_clmapmgr_warning("%px: invalid rtnl structure\n", dev);
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

}

/*
 * nss_clmapmgr_dev_init()
 *	Netdev ops function to intialize netdevice.
 */
static int nss_clmapmgr_dev_init(struct net_device *dev)
{
	dev->mtu = ETH_DATA_LEN;
	dev->needed_headroom = NSS_CLMAP_MAX_HEADROOM;
	return 0;
}

/*
 * nss_clmapmgr_dev_open()
 *	Netdev ops function to open netdevice.
 */
static int nss_clmapmgr_dev_open(struct net_device *dev)
{
	netif_start_queue(dev);
	return 0;
}

/*
 * nss_clmapmgr_dev_close()
 *	Netdevice ops function to close netdevice.
 */
static int nss_clmapmgr_dev_close(struct net_device *dev)
{
	netif_stop_queue(dev);
	return 0;
}

/*
 * clmap netdevice ops
 */
static const struct net_device_ops nss_clmapmgr_ops = {
	.ndo_init = nss_clmapmgr_dev_init,
	.ndo_open = nss_clmapmgr_dev_open,
	.ndo_stop = nss_clmapmgr_dev_close,
	.ndo_set_mac_address = eth_mac_addr,
	.ndo_validate_addr = eth_validate_addr,
	.ndo_start_xmit = nss_clmapmgr_dev_xmit,
	.ndo_get_stats64 = nss_clmapmgr_dev_stats64,
};

/*
 * nss_clmapmgr_setup()
 */
static void nss_clmapmgr_setup(struct net_device *dev)
{
	char name[IFNAMSIZ] = {0};

	strlcpy(name, "nssclmap%d", IFNAMSIZ);
	memcpy(dev->name, name, IFNAMSIZ);
	dev->netdev_ops = &nss_clmapmgr_ops;
	eth_hw_addr_random(dev);
}

/*
 * nss_clmapmgr_ds_exception()
 *	Client map manager ds exception handler to receive packet from NSS.
 */
static void nss_clmapmgr_ds_exception(struct net_device *dev, struct sk_buff *skb,
				       __attribute__((unused)) struct napi_struct *napi)
{
	/*
	 * Note: preheader needs to be processed by the user
	 * before processing the ethernet packet
	 */
	skb->protocol = eth_type_trans(skb, dev);
	netif_receive_skb(skb);
}

/*
 * nss_clmapmgr_us_exception()
 *	Client map manager us exception handler to receive packet from NSS.
 */
static void nss_clmapmgr_us_exception(struct net_device *dev, struct sk_buff *skb,
				       __attribute__((unused)) struct napi_struct *napi)
{
	/*
	 * This is an error packet and needs to be dropped.
	 */
	nss_clmapmgr_warning("%px: upstream packet got exceptioned, dropping the packet..", dev);
	dev_kfree_skb_any(skb);
}

/*
 * nss_clmapmgr_event_receive()
 *	Event Callback to receive events from NSS
 */
static void nss_clmapmgr_event_receive(void *if_ctx, struct nss_cmn_msg *cmsg)
{
	struct net_device *dev = (struct net_device *)if_ctx;
	struct nss_clmapmgr_priv_t *priv;
	struct nss_clmap_msg *clmsg = (struct nss_clmap_msg *)cmsg;
	struct nss_clmap_stats_msg *stats = &clmsg->msg.stats;
	struct rtnl_link_stats64 *netdev_stats;
	enum nss_dynamic_interface_type interface_type;
	uint64_t dropped = 0;

	dev_hold(dev);
	priv = netdev_priv(dev);
	netdev_stats = &priv->stats;

	switch (clmsg->cm.type) {
	case NSS_CLMAP_MSG_TYPE_SYNC_STATS:
		interface_type = nss_dynamic_interface_get_type(nss_clmap_get_ctx(), clmsg->cm.interface);
		if (interface_type == NSS_DYNAMIC_INTERFACE_TYPE_CLMAP_US) {
			netdev_stats->tx_packets += stats->node_stats.tx_packets;
			netdev_stats->tx_bytes += stats->node_stats.tx_bytes;
			dropped += stats->dropped_macdb_lookup_failed;
			dropped += stats->dropped_invalid_packet_size;
			dropped += stats->dropped_low_hroom;
		} else if (interface_type == NSS_DYNAMIC_INTERFACE_TYPE_CLMAP_DS) {
			netdev_stats->rx_packets += stats->node_stats.rx_packets;
			netdev_stats->rx_bytes += stats->node_stats.rx_bytes;
			dropped += stats->dropped_pbuf_alloc_failed;
			dropped += stats->dropped_linear_failed;
			dropped += stats->shared_packet_count;
			dropped += stats->ethernet_frame_error;
		}
		dropped += stats->dropped_next_node_queue_full;
		netdev_stats->tx_dropped += dropped;
		if (interface_type == NSS_DYNAMIC_INTERFACE_TYPE_CLMAP_DS) {
			netdev_stats->rx_dropped += nss_cmn_rx_dropped_sum(&stats->node_stats);
		}
		break;

	default:
		nss_clmapmgr_info("%px: Unknown Event from NSS\n", dev);
		break;
	}

	dev_put(dev);
}

/*
 * nss_clmapmgr_us_get_if_num
 * 	return us NSS interface number
 */
int nss_clmapmgr_us_get_if_num(struct net_device *dev)
{
	struct nss_clmapmgr_priv_t *priv;

	if (!dev) {
		nss_clmapmgr_info("Netdev is NULL");
		return -1;
	}

	priv = (struct nss_clmapmgr_priv_t *)netdev_priv(dev);
	return priv->nss_if_number_us;
}
EXPORT_SYMBOL(nss_clmapmgr_us_get_if_num);

/*
 * nss_clmapmgr_ds_get_if_num
 * 	return ds NSS interface number
 */
int nss_clmapmgr_ds_get_if_num(struct net_device *dev)
{
	struct nss_clmapmgr_priv_t *priv;

	if (!dev) {
		nss_clmapmgr_info("Netdev is NULL");
		return -1;
	}

	priv = (struct nss_clmapmgr_priv_t *)netdev_priv(dev);
	return priv->nss_if_number_ds;
}
EXPORT_SYMBOL(nss_clmapmgr_ds_get_if_num);

/*
 * nss_clmapmgr_mac_add()
 * 	API to send notification to NSS to add the MAC entry.
 */
nss_clmapmgr_status_t nss_clmapmgr_mac_add(struct net_device *dev, struct nss_clmapmgr_msg *clmapmsg)
{
	struct nss_clmap_msg req;
	struct nss_clmap_mac_msg *mmsg;
	int us_if, next_ifnum;
	struct nss_ctx_instance *nss_ctx = NULL;
	nss_tx_status_t status;

	if (!dev) {
		nss_clmapmgr_info("Netdev is NULL !!\n");
		return NSS_CLMAPMGR_ERR_BAD_PARAM;
	}

	if (!clmapmsg) {
		nss_clmapmgr_info("%px: nss_clmapmgr_msg is NULL !!\n", dev);
		return NSS_CLMAPMGR_ERR_BAD_PARAM;
	}

	/*
	 * Get Interface number, based on tunnel type
	 */
	switch (clmapmsg->tunnel_type) {
	case NSS_CLMAPMGR_TUNNEL_EOGRE:
		/*
		 * For EoGRE tunnel, the next node for the packet from clmap node in NSS
		 * would be GRE inner node. Get the GRE inner interface.
		 */
		next_ifnum = nss_eogremgr_get_if_num_inner(clmapmsg->tunnel_id);
		if (next_ifnum < 0) {
			nss_clmapmgr_info("%px: No NSS interface registered for the tunnel id: %d\n", dev, clmapmsg->tunnel_id);
			return NSS_CLMAPMGR_ERR_TUNNEL_NOT_FOUND;
		}
		break;
	default:
		nss_clmapmgr_info("%px: Invalid tunnel type: %d\n", dev, clmapmsg->tunnel_type);
		return NSS_CLMAPMGR_ERR_BAD_PARAM;
	}

	nss_ctx = nss_clmap_get_ctx();

	/*
	 * Check if upstream clmap interface is registered with NSS.
	 */
	us_if = nss_cmn_get_interface_number_by_dev_and_type(dev, NSS_DYNAMIC_INTERFACE_TYPE_CLMAP_US);
	if (us_if < 0) {
		nss_clmapmgr_info("%px: Net device is not registered with nss\n", dev);
		dev_put(dev);
		return NSS_CLMAPMGR_ERR_NETDEV_UNKNOWN;
	}

	memset(&req, 0, sizeof(struct nss_clmap_msg));
	mmsg = &req.msg.mac_add;

	/*
	 * Set mac_add message.
	 */
	memcpy(mmsg->mac_addr, clmapmsg->mac_addr, ETH_ALEN);
	mmsg->flags = clmapmsg->flags;
	mmsg->vlan_id = clmapmsg->vlan_id;
	mmsg->needed_headroom = clmapmsg->needed_headroom;
	mmsg->nexthop_ifnum = next_ifnum;
	nss_clmap_msg_init(&req, us_if, NSS_CLMAP_MSG_TYPE_MAC_ADD, sizeof(struct nss_clmap_mac_msg), NULL, NULL);
	status = nss_clmap_tx_msg_sync(nss_ctx, &req);
	if (status != NSS_TX_SUCCESS) {
		nss_clmapmgr_warning("%px: nss clmap mac add command error:%d if_num: %d\n", dev, status, us_if);
		dev_put(dev);
		return NSS_CLMAPMGR_ERR_MAC_ADD_FAILED;
	}

	return NSS_CLMAPMGR_SUCCESS;
}
EXPORT_SYMBOL(nss_clmapmgr_mac_add);

/*
 * nss_clmapmgr_mac_remove()
 * 	API to send notification to NSS to delete the MAC entry.
 */
nss_clmapmgr_status_t nss_clmapmgr_mac_remove(struct net_device *dev, uint8_t *mac_addr)
{
	struct nss_clmap_msg req;
	struct nss_clmap_mac_msg *mmsg;
	int us_if;
	struct nss_ctx_instance *nss_ctx = NULL;
	nss_tx_status_t status;

	if (!dev) {
		nss_clmapmgr_info("Netdev is NULL !!\n");
		return NSS_CLMAPMGR_ERR_BAD_PARAM;
	}

	if (!mac_addr) {
		nss_clmapmgr_info("%px: mac address is NULL !!\n", dev);
		return NSS_CLMAPMGR_ERR_BAD_PARAM;
	}

	nss_ctx = nss_clmap_get_ctx();

	/*
	 * Check if upstream clmap interface is registered with NSS
	 */
	us_if = nss_cmn_get_interface_number_by_dev_and_type(dev, NSS_DYNAMIC_INTERFACE_TYPE_CLMAP_US);
	if (us_if < 0) {
		nss_clmapmgr_info("%px: Net device is not registered with nss\n", dev);
		dev_put(dev);
		return NSS_CLMAPMGR_ERR_NETDEV_UNKNOWN;
	}

	memset(&req, 0, sizeof(struct nss_clmap_msg));
	mmsg = &req.msg.mac_add;

	/*
	 * Set mac_del message. Only MAC address is required, the other
	 * fields as set to 0.
	 *
	 */
	memcpy(mmsg->mac_addr, mac_addr, ETH_ALEN);
	nss_clmap_msg_init(&req, us_if, NSS_CLMAP_MSG_TYPE_MAC_DEL, sizeof(struct nss_clmap_mac_msg), NULL, NULL);
	status = nss_clmap_tx_msg_sync(nss_ctx, &req);
	if (status != NSS_TX_SUCCESS) {
		nss_clmapmgr_warning("%px: NSS clmap mac del command error:%d if_num: %d\n", dev, status, us_if);
		dev_put(dev);
		return NSS_CLMAPMGR_ERR_MAC_DEL_FAILED;
	}

	return NSS_CLMAPMGR_SUCCESS;
}
EXPORT_SYMBOL(nss_clmapmgr_mac_remove);

/*
 * nss_clmapmgr_mac_flush()
 * 	API to send notification to NSS to flush MAC entry.
 */
nss_clmapmgr_status_t nss_clmapmgr_mac_flush(struct net_device *dev, uint32_t tunnel_id, nss_clmapmgr_tunnel_type_t tunnel_type)
{
	struct nss_clmap_msg req;
	struct nss_clmap_flush_mac_msg *mmsg = &req.msg.mac_flush;
	int us_if, next_ifnum;
	struct nss_ctx_instance *nss_ctx = NULL;
	nss_tx_status_t status;

	if (!dev) {
		nss_clmapmgr_info("Netdev is NULL !!\n");
		return NSS_CLMAPMGR_ERR_BAD_PARAM;
	}

	switch (tunnel_type) {
	case NSS_CLMAPMGR_TUNNEL_EOGRE:
		/*
		 * Get GRE inner interface number
		 */
		next_ifnum = nss_eogremgr_get_if_num_inner(tunnel_id);
		if (next_ifnum < 0) {
			nss_clmapmgr_info("%px: No NSS interface registered for the tunnel id: %d\n", dev, tunnel_id);
			return NSS_CLMAPMGR_ERR_TUNNEL_NOT_FOUND;
		}
		break;
	default:
		nss_clmapmgr_info("%px: Invalid tunnel type: %d\n", dev, tunnel_type);
		return NSS_CLMAPMGR_ERR_BAD_PARAM;
	}

	nss_ctx = nss_clmap_get_ctx();

	/*
	 * Check if upstream clmap interface is registered with NSS
	 */
	us_if = nss_cmn_get_interface_number_by_dev_and_type(dev, NSS_DYNAMIC_INTERFACE_TYPE_CLMAP_US);
	if (us_if < 0) {
		nss_clmapmgr_info("%px: Net device is not registered with nss\n", dev);
		dev_put(dev);
		return NSS_CLMAPMGR_ERR_NETDEV_UNKNOWN;
	}

	/*
	 * Set mac_flush message
	 */
	mmsg->nexthop_ifnum = next_ifnum;
	nss_clmap_msg_init(&req, us_if, NSS_CLMAP_MSG_TYPE_MAC_FLUSH, sizeof(struct nss_clmap_mac_msg), NULL, NULL);
	status = nss_clmap_tx_msg_sync(nss_ctx, &req);
	if (status != NSS_TX_SUCCESS) {
		nss_clmapmgr_warning("%px: NSS clmap mac flush command error:%d if_num: %d\n", dev, status, us_if);
		dev_put(dev);
		return NSS_CLMAPMGR_ERR_MAC_FLUSH_FAILED;
	}

	return NSS_CLMAPMGR_SUCCESS;
}
EXPORT_SYMBOL(nss_clmapmgr_mac_flush);

/*
 * nss_clmapmgr_netdev_enable()
 *	Call back to enable NSS for clmap device.
 */
int nss_clmapmgr_netdev_enable(struct net_device *dev)
{
	struct nss_clmap_msg req;
	int us_if, ds_if;
	struct nss_ctx_instance *nss_ctx = NULL;
	struct nss_clmapmgr_priv_t *priv;
	nss_tx_status_t status;

	if (!dev) {
		nss_clmapmgr_info("Netdev is NULL !!\n");
		return NOTIFY_DONE;
	}

	nss_ctx = nss_clmap_get_ctx();

	/*
	 * Check if upstream clmap interface is registered with NSS
	 */
	us_if = nss_cmn_get_interface_number_by_dev_and_type(dev, NSS_DYNAMIC_INTERFACE_TYPE_CLMAP_US);
	if (us_if < 0) {
		nss_clmapmgr_info("%px: Net device is not registered with nss\n", dev);
		goto release_ref;
	}

	/*
	 * Check if downstream clmap interface is registered with NSS
	 */
	ds_if = nss_cmn_get_interface_number_by_dev_and_type(dev, NSS_DYNAMIC_INTERFACE_TYPE_CLMAP_DS);
	if (ds_if < 0) {
		nss_clmapmgr_info("%px: Net device is not registered with nss\n", dev);
		goto release_ref;
	}

	/*
	 * Send enable session command for upstream interface
	 */
	nss_clmap_msg_init(&req, us_if, NSS_CLMAP_MSG_TYPE_INTERFACE_ENABLE, 0, NULL, NULL);
	status = nss_clmap_tx_msg_sync(nss_ctx, &req);
	if (status != NSS_TX_SUCCESS) {
		nss_clmapmgr_warning("%px: NSS clmap enable command error:%d if_num: %d\n", dev, status, us_if);
		goto release_ref;
	}

	/*
	 * Send enable session command for downstream interface
	 */
	nss_clmap_msg_init(&req, ds_if, NSS_CLMAP_MSG_TYPE_INTERFACE_ENABLE, 0, NULL, NULL);
	status = nss_clmap_tx_msg_sync(nss_ctx, &req);
	if (status != NSS_TX_SUCCESS) {
		nss_clmapmgr_warning("%px: NSS clmap enable command error:%d if_num: %d\n", dev, status, ds_if);
		goto disable_us;
	}

	/*
	 * Open the netdev to accept packets
	 */
	priv = (struct nss_clmapmgr_priv_t *)netdev_priv(dev);
	priv->clmap_enabled = true;
	nss_clmapmgr_dev_open(dev);

	return NOTIFY_OK;

disable_us:
	nss_clmap_msg_init(&req, us_if, NSS_CLMAP_MSG_TYPE_INTERFACE_DISABLE, 0, NULL, NULL);
	status = nss_clmap_tx_msg_sync(nss_ctx, &req);
	if (status != NSS_TX_SUCCESS) {
		nss_clmapmgr_warning("%px: NSS clmap enable command error:%d if_num: %d\n", dev, status, us_if);
	}

release_ref:
	return NOTIFY_DONE;
}
EXPORT_SYMBOL(nss_clmapmgr_netdev_enable);

/*
 * nss_clmapmgr_netdev_disable()
 *	Call back to disable clmap interface in NSS.
 */
int nss_clmapmgr_netdev_disable(struct net_device *dev)
{
	struct nss_clmap_msg req;
	int us_if, ds_if;
	struct nss_ctx_instance *nss_ctx = NULL;
	struct nss_clmapmgr_priv_t *priv;
	nss_tx_status_t status;

	if (!dev) {
		nss_clmapmgr_info("Netdev is NULL !!\n");
		return NOTIFY_DONE;
	}

	nss_ctx = nss_clmap_get_ctx();

	/*
	 * Check if upstream clmap interface is registered with NSS
	 */
	us_if = nss_cmn_get_interface_number_by_dev_and_type(dev, NSS_DYNAMIC_INTERFACE_TYPE_CLMAP_US);
	if (us_if < 0) {
		nss_clmapmgr_info("%px: Net device is not registered with NSS\n", dev);
		goto release_ref;
	}

	/*
	 * Check if downstream clmap interface is registered with NSS
	 */
	ds_if = nss_cmn_get_interface_number_by_dev_and_type(dev, NSS_DYNAMIC_INTERFACE_TYPE_CLMAP_DS);
	if (ds_if < 0) {
		nss_clmapmgr_info("%px: Net device is not registered with NSS\n", dev);
		goto release_ref;
	}

	/*
	 * Send disable session command for upstream interface
	 */
	nss_clmap_msg_init(&req, us_if, NSS_CLMAP_MSG_TYPE_INTERFACE_DISABLE, 0, NULL, NULL);
	status = nss_clmap_tx_msg_sync(nss_ctx, &req);
	if (status != NSS_TX_SUCCESS) {
		nss_clmapmgr_warning("%px: NSS clmap disable command error:%d if_num: %d\n", dev, status, us_if);
		goto release_ref;
	}

	/*
	 * Send disable session command for downstream interface
	 */
	nss_clmap_msg_init(&req, ds_if, NSS_CLMAP_MSG_TYPE_INTERFACE_DISABLE, 0, NULL, NULL);
	status = nss_clmap_tx_msg_sync(nss_ctx, &req);
	if (status != NSS_TX_SUCCESS) {
		nss_clmapmgr_warning("%px: NSS clmap disable command error:%d if_num: %d\n", dev, status, ds_if);
		goto enable_us;
	}

	/*
	 * Close the netdev
	 */
	priv = (struct nss_clmapmgr_priv_t *)netdev_priv(dev);
	priv->clmap_enabled = false;
	nss_clmapmgr_dev_close(dev);

	return NOTIFY_OK;

enable_us:
	nss_clmap_msg_init(&req, us_if, NSS_CLMAP_MSG_TYPE_INTERFACE_ENABLE, 0, NULL, NULL);
	status = nss_clmap_tx_msg_sync(nss_ctx, &req);
	if (status != NSS_TX_SUCCESS) {
		nss_clmapmgr_warning("%px: NSS clmap disable command error:%d if_num: %d\n", dev, status, us_if);
	}

release_ref:
	return NOTIFY_DONE;
}
EXPORT_SYMBOL(nss_clmapmgr_netdev_disable);

/*
 * nss_clmapmgr_dev_event()
 *	Netdevice notifier call back function.
 */
static int nss_clmapmgr_dev_event(struct notifier_block  *nb,
		unsigned long event, void  *dev)
{
	struct net_device *netdev;
	netdev = netdev_notifier_info_to_dev(dev);

	switch (event) {
	case NETDEV_UP:
		return nss_clmapmgr_netdev_enable(netdev);

	case NETDEV_DOWN:
		return nss_clmapmgr_netdev_disable(netdev);

	default:
		break;
	}

	return NOTIFY_DONE;
}

/*
 * nss_clmapmgr_destroy_us_interface()
 *	Destroy upstream clmap interface.
 */
static nss_clmapmgr_status_t nss_clmapmgr_destroy_us_interface(struct net_device *dev, int interface_num)
{
	nss_tx_status_t status;
	int retry = 0;

	if (!nss_clmap_unregister(interface_num)) {
		nss_clmapmgr_warning("%px: clmap NSS upstream interface unregister failed\n.", dev);
		return NSS_CLMAPMGR_ERR_NSSIF_UNREGISTER_FAILED;
	}

dealloc_us:
	status = nss_dynamic_interface_dealloc_node(interface_num, NSS_DYNAMIC_INTERFACE_TYPE_CLMAP_US);
	if (status != NSS_TX_SUCCESS) {
		nss_clmapmgr_info("%px: clmap dealloc node failure for interface_num = %d\n", dev, interface_num);
		if (++retry <= NSS_CLMAPMGR_CMD_MAX_RETRY_COUNT) {
			goto dealloc_us;
		}

		nss_clmapmgr_error("%px: fatal Error, failed to dealloc upstream clmap NSS interface.\n", dev);
		return NSS_CLMAPMGR_ERR_NSSIF_DEALLOC_FAILED;
	}

	return NSS_CLMAPMGR_SUCCESS;
}

/*
 * nss_clmapmgr_destroy_ds_interface()
 *	Destroy downstream clmap interface.
 */
static nss_clmapmgr_status_t nss_clmapmgr_destroy_ds_interface(struct net_device *dev, int interface_num)
{
	nss_tx_status_t status;
	int retry = 0;

	if (!nss_clmap_unregister(interface_num)) {
		nss_clmapmgr_warning("%px: clmap NSS downstream interface unregister failed\n.", dev);
		return NSS_CLMAPMGR_ERR_NSSIF_UNREGISTER_FAILED;
	}

dealloc_ds:
	status = nss_dynamic_interface_dealloc_node(interface_num, NSS_DYNAMIC_INTERFACE_TYPE_CLMAP_DS);
	if (status != NSS_TX_SUCCESS) {
		nss_clmapmgr_info("%px: clmap dealloc node failure for ds_if = %d\n", dev, interface_num);
		if (++retry <= NSS_CLMAPMGR_CMD_MAX_RETRY_COUNT) {
			goto dealloc_ds;
		}

		nss_clmapmgr_error("%px: fatal Error, failed to dealloc downstream clmap NSS interface.\n", dev);
		return NSS_CLMAPMGR_ERR_NSSIF_DEALLOC_FAILED;
	}

	return NSS_CLMAPMGR_SUCCESS;
}

/*
 * nss_clmapmgr_decongestion_callback()
 * 	Wakeup netif queue if we were stopped by start_xmit
 */
static void nss_clmapmgr_decongestion_callback(void *arg) {
	struct net_device *dev = arg;
	struct nss_clmapmgr_priv_t *priv;

	priv = (struct nss_clmapmgr_priv_t *)netdev_priv(dev);
	if (unlikely(!priv->clmap_enabled)) {
		return;
	}

	if (netif_queue_stopped(dev)) {
		netif_wake_queue(dev);
	}
}

/*
 * nss_clmapmgr_netdev_destroy()
 * 	API for destroying a netdevice.
 * 	Note: User needs to flush all MAC entries in the clmap before destroying the clmap netdevice
 */
nss_clmapmgr_status_t nss_clmapmgr_netdev_destroy(struct net_device *dev)
{
	int us_if, ds_if;
	nss_clmapmgr_status_t ret;

	netif_tx_disable(dev);

	/*
	 * Deregister decongestion callback
	 */
	if (nss_cmn_unregister_queue_decongestion(nss_clmap_get_ctx(), nss_clmapmgr_decongestion_callback) != NSS_CB_UNREGISTER_SUCCESS) {
		nss_clmapmgr_info("%px: failed to unregister decongestion callback\n", dev);
	}

	/*
	 * Check if upstream clmap interface is registered with NSS
	 */
	us_if = nss_cmn_get_interface_number_by_dev_and_type(dev, NSS_DYNAMIC_INTERFACE_TYPE_CLMAP_US);
	if (us_if < 0) {
		nss_clmapmgr_info("%px: Net device is not registered with NSS\n", dev);
		return NSS_CLMAPMGR_ERR_NETDEV_UNKNOWN;
	}

	/*
	 * Check if downstream clmap interface is registered with NSS
	 */
	ds_if = nss_cmn_get_interface_number_by_dev_and_type(dev, NSS_DYNAMIC_INTERFACE_TYPE_CLMAP_DS);
	if (ds_if < 0) {
		nss_clmapmgr_info("%px: Net device is not registered with NSS\n", dev);
		return NSS_CLMAPMGR_ERR_NETDEV_UNKNOWN;
	}

	ret = nss_clmapmgr_destroy_us_interface(dev, us_if);
	if (ret != NSS_CLMAPMGR_SUCCESS) {
		nss_clmapmgr_warning("%px: failed to destroy clmap upstream interface: %d\n", dev, us_if);
		return ret;
	}

	ret = nss_clmapmgr_destroy_ds_interface(dev, ds_if);
	if (ret != NSS_CLMAPMGR_SUCCESS) {
		nss_clmapmgr_warning("%px: failed to destroy clmap downstream interface: %d\n", dev, ds_if);
		return ret;
	}

	nss_clmapmgr_info("%px: deleted clmap instance, us_if = %d ds_if = %d\n",
			dev, us_if, ds_if);

	unregister_netdev(dev);
	free_netdev(dev);
	return NSS_CLMAPMGR_SUCCESS;
}
EXPORT_SYMBOL(nss_clmapmgr_netdev_destroy);

/*
 * nss_clmapmgr_netdev_create()
 *	User API to create clmap interface
 */
struct net_device *nss_clmapmgr_netdev_create(void)
{
	struct nss_ctx_instance *nss_ctx;
	struct net_device *dev = NULL;
	struct nss_clmapmgr_priv_t *priv;
	nss_tx_status_t status;
	uint32_t features = 0;
	int32_t us_if, ds_if;
	int ret = -1, retry = 0;

	dev = alloc_etherdev(sizeof(struct nss_clmapmgr_priv_t));
	if (!dev) {
		nss_clmapmgr_warning("Allocation of netdev failed\n");
		return NULL;
	}

	nss_clmapmgr_setup(dev);

	/*
	 * Register net_device
	 */
	ret = register_netdev(dev);
	if (ret) {
		nss_clmapmgr_warning("%px: Netdevice registration failed\n", dev);
		free_netdev(dev);
		return NULL;
	}

	/*
	 * Create NSS clmap downstream dynamic interface
	 */
	ds_if = nss_dynamic_interface_alloc_node(NSS_DYNAMIC_INTERFACE_TYPE_CLMAP_DS);
	if (ds_if < 0) {
		nss_clmapmgr_warning("%px: NSS dynamic interface alloc failed for clmap downstream\n", dev);
		goto deregister_netdev;
	}

	/*
	 * Create NSS clmap upstream dynamic interface
	 */
	us_if = nss_dynamic_interface_alloc_node(NSS_DYNAMIC_INTERFACE_TYPE_CLMAP_US);
	if (us_if < 0) {
		nss_clmapmgr_warning("%px: NSS dynamic interface alloc failed for clmap upstream\n", dev);
		goto dealloc_ds_node;
	}

	priv = (struct nss_clmapmgr_priv_t *)netdev_priv(dev);
	priv->clmap_enabled = false;
	priv->nss_if_number_us = us_if;
	priv->nss_if_number_ds = ds_if;

	/*
	 * Register downstream clmap interface with NSS
	 */
	nss_ctx = nss_clmap_register(ds_if,
				NSS_DYNAMIC_INTERFACE_TYPE_CLMAP_DS,
				nss_clmapmgr_ds_exception,
	 			nss_clmapmgr_event_receive,
				dev,
				features);
	if (!nss_ctx) {
		nss_clmapmgr_info("%px: nss_clmap_register failed for downstream interface\n", dev);
		goto dealloc_us_node;
	}

	/*
	 * Register upstream clmap interface with NSS
	 */
	nss_ctx = nss_clmap_register(us_if,
				NSS_DYNAMIC_INTERFACE_TYPE_CLMAP_US,
				nss_clmapmgr_us_exception,
	 			nss_clmapmgr_event_receive,
				dev,
				features);
	if (!nss_ctx) {
		nss_clmapmgr_info("%px: nss_clmap_register failed for upstream interface\n", dev);
		goto unregister_ds;
	}

	/*
	 * Register decongestion callback
	 */
	if (nss_cmn_register_queue_decongestion(nss_clmap_get_ctx(), nss_clmapmgr_decongestion_callback, dev) != NSS_CB_REGISTER_SUCCESS) {
		nss_clmapmgr_warning("%px: failed to register decongestion callback\n", dev);
		goto unregister_us;
	}

	/*
	 * Success
	 */
	nss_clmapmgr_info("%px: nss_clmap_register() successful. nss_ctx = %px\n", dev, nss_ctx);
	return dev;

unregister_us:
	nss_clmap_unregister(us_if);

unregister_ds:
	nss_clmap_unregister(ds_if);

dealloc_us_node:
	status = nss_dynamic_interface_dealloc_node(us_if, NSS_DYNAMIC_INTERFACE_TYPE_CLMAP_US);
	if (status != NSS_TX_SUCCESS) {
		if (++retry <= NSS_CLMAPMGR_CMD_MAX_RETRY_COUNT) {
			goto dealloc_us_node;
		}
		nss_clmapmgr_error("%px: fatal Error, Unable to dealloc the node[%d] in the NSS FW!\n", dev, us_if);
	}

	retry = 0;
dealloc_ds_node:
	status = nss_dynamic_interface_dealloc_node(ds_if, NSS_DYNAMIC_INTERFACE_TYPE_CLMAP_DS);
	if (status != NSS_TX_SUCCESS) {
		if (++retry <= NSS_CLMAPMGR_CMD_MAX_RETRY_COUNT) {
			goto dealloc_ds_node;
		}
		nss_clmapmgr_error("%px: fatal Error, Unable to dealloc the node[%d] in the NSS FW!\n", dev, ds_if);
	}

deregister_netdev:
	unregister_netdev(dev);
	free_netdev(dev);

	return NULL;
}
EXPORT_SYMBOL(nss_clmapmgr_netdev_create);

/*
 * Linux Net device Notifier
 */
static struct notifier_block nss_clmapmgr_notifier = {
	.notifier_call = nss_clmapmgr_dev_event,
};

/*
 * nss_clmapmgr_dev_init_module()
 *	Client map module init function
 */
static int __init nss_clmapmgr_dev_init_module(void)
{
#ifdef CONFIG_OF
	/*
	 * If the node is not compatible, don't do anything.
	 */
	if (!of_find_node_by_name(NULL, "nss-common")) {
		return 0;
	}
#endif
	register_netdevice_notifier(&nss_clmapmgr_notifier);

	return 0;
}

/*
 * nss_clmapmgr_exit_module
 *	Client map module exit function
 */
static void __exit nss_clmapmgr_exit_module(void)
{
#ifdef CONFIG_OF
	/*
	 * If the node is not compatible, don't do anything.
	 */
	if (!of_find_node_by_name(NULL, "nss-common")) {
		return;
	}
#endif
	/*
	 * Unregister clmap interfaces created
	 */
	unregister_netdevice_notifier(&nss_clmapmgr_notifier);
}

module_init(nss_clmapmgr_dev_init_module);
module_exit(nss_clmapmgr_exit_module);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS client map manager");
