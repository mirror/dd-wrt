/*
 **************************************************************************
 * Copyright (c) 2014-2021, The Linux Foundation. All rights reserved.
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

/*
 * nss_capwapmgr.c
 *	NSS to HLOS CAPWAP manager
 */
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/of.h>
#include <linux/tcp.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/stat.h>
#include <net/ipv6.h>
#include <linux/version.h>
#include <net/ip_tunnels.h>
#include <linux/if_arp.h>
#include <linux/etherdevice.h>
#include <linux/if_pppox.h>
#include <nss_api_if.h>
#include <linux/in.h>
#include <linux/debugfs.h>
#include <fal/fal_qos.h>
#include <fal/fal_acl.h>
#include <ppe_drv.h>
#include <ppe_drv_v4.h>
#include <ppe_drv_v6.h>
#include <nss_dp_api_if.h>
#include <ppe_vp_public.h>
#include <ppe_drv_iface.h>
#include <nss_dtlsmgr.h>
#include <nss_capwapmgr_public.h>
#include <nss_capwapmgr_user.h>
#include "nss_capwapmgr.h"

/*
 * This file is responsible for interacting with qca-nss-drv's
 * CAPWAP API to manage CAPWAP tunnels.
 *
 * This driver also exposes few APIs which can be used by
 * another module to perform operations on CAPWAP tunnels. However, we create
 * one netdevice for all the CAPWAP tunnels which is done at the module's
 * init time if NSS_CAPWAPMGR_ONE_NETDEV is set in the Makefile.
 *
 * If your requirement is to create one netdevice per-CAPWAP tunnel, then
 * netdevice needs to be created before CAPWAP tunnel create. Netdevice are
 * created using nss_capwapmgr_netdev_create() API.
 *
 */

/*
 * Global Structure to hold tunnel statistics and driver queue selection.
 */
static struct nss_capwapmgr_global global;

/*
 * Lock to handle acl configuration.
 */
DEFINE_SPINLOCK(nss_capwapmgr_acl_spinlock);

#if defined(NSS_CAPWAPMGR_ONE_NETDEV)
/*
 * If you want only one netdev for all the tunnels. If you don't want
 * to create one netdev for all the tunnels, then netdev must be
 * created using nss_capwapmgr_netdev_create() before every tunnel create
 * operation.
 */
static struct net_device *nss_capwapmgr_ndev = NULL;
#endif

/*
 * nss_capwapmgr_open()
 *	Netdev's open call.
 */
static int nss_capwapmgr_open(struct net_device *dev)
{
	netif_start_queue(dev);
	return 0;
}

/*
 * nss_capwapmgr_close()
 *	Netdev's close call.
 */
static int nss_capwapmgr_close(struct net_device *dev)
{
	netif_stop_queue(dev);
	return 0;
}

/*
 * nss_capwapmgr_decongestion_callback()
 *	Wakeup netif queue if we were stopped by start_xmit
 */
static void nss_capwapmgr_decongestion_callback(void *arg)
{
	struct net_device *dev = arg;

	if (netif_queue_stopped(dev)) {
		netif_wake_queue(dev);
	}
}

/*
 * nss_capwapmgr_start_xmit()
 *	Transmit's skb to NSS FW over CAPWAP if_num_inner.
 *
 * Please make sure to leave headroom of NSS_CAPWAP_HEADROOM with every
 * packet so that NSS can encap eth,vlan,ip,udp,capwap headers.
 * Also, skb->len must include size of metaheader. Essentially skb->len is
 * size of CAPWAP Payload (including wireless info sections) and metaheader.
 */
static netdev_tx_t nss_capwapmgr_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct net_device_stats *stats = &dev->stats;
	struct nss_capwapmgr_priv *priv;
	struct nss_capwap_metaheader *pre;
	uint32_t if_num_inner;
	nss_tx_status_t status;

	priv = netdev_priv(dev);
	pre = (struct nss_capwap_metaheader *)skb->data;

	if (unlikely(pre->tunnel_id > NSS_CAPWAPMGR_MAX_TUNNELS)) {
		nss_capwapmgr_warn("%px: (CAPWAP packet) tunnel-id invalid: %d\n", dev, pre->tunnel_id);
		kfree_skb(skb);
		stats->tx_dropped++;
		return NETDEV_TX_OK;
	}

	if_num_inner = priv->tunnel[pre->tunnel_id].if_num_inner;
	if (unlikely(if_num_inner == -1)) {
		nss_capwapmgr_warn("%px: (CAPWAP packet) if_num_inner in the tunnel not set pre->tunnel_id %d\n", dev,
				pre->tunnel_id);
		kfree_skb(skb);
		stats->tx_dropped++;
		return NETDEV_TX_OK;
	}

	/*
	 * We use the lowest bit in the inner flow_id to determine which Tx ring
	 * to use (drv uses queue mapping to select Tx ring).
	 *
	 * This ring distribution will in turn get used in NSS firmware
	 * for better thread distribution of encap operation.
	 */
	skb_set_queue_mapping(skb, pre->flow_id & 0x1);

	status = nss_capwap_tx_buf(priv->nss_ctx, skb, if_num_inner);
	if (unlikely(status != NSS_TX_SUCCESS)) {
		if (status == NSS_TX_FAILURE_QUEUE) {
			nss_capwapmgr_warn("%px: netdev :%px queue is full", dev, dev);
			if (!netif_queue_stopped(dev)) {
				netif_stop_queue(dev);
			}
		}

		return NETDEV_TX_BUSY;
	}

	return NETDEV_TX_OK;
}

/*
 * nss_capwapmgr_fill_up_stats()
 *	Fills up stats in netdev's stats.
 */
static void nss_capwapmgr_fill_up_stats(struct rtnl_link_stats64 *stats, struct nss_capwap_tunnel_stats *tstats)
{
	stats->rx_packets += tstats->pnode_stats.rx_packets;
	stats->rx_dropped += tstats->pnode_stats.rx_dropped;

	/* rx_fifo_errors will appear as rx overruns in ifconfig */
	stats->rx_fifo_errors += (tstats->rx_n2h_drops + tstats->rx_n2h_queue_full_drops);
	stats->rx_errors += (tstats->rx_mem_failure_drops + tstats->rx_oversize_drops + tstats->rx_frag_timeout_drops);
	stats->rx_bytes += tstats->pnode_stats.rx_bytes;

	/* tx_fifo_errors  will appear as tx overruns in ifconfig */
	stats->tx_fifo_errors += tstats->tx_queue_full_drops;
	stats->tx_errors += tstats->tx_mem_failure_drops;
	stats->tx_bytes += tstats->pnode_stats.tx_bytes;

	stats->tx_dropped += (tstats->tx_dropped_sg_ref + tstats->tx_dropped_ver_mis + tstats->tx_dropped_hroom
			 + tstats->tx_dropped_dtls + tstats->tx_dropped_nwireless);
	stats->tx_packets += tstats->pnode_stats.tx_packets;
}

/*
 * nss_capwapmgr_get_tunnel_stats()
 *	Netdev get stats function to get tunnel stats
 */
static struct rtnl_link_stats64 *nss_capwapmgr_get_tunnel_stats(struct net_device *dev, struct rtnl_link_stats64 *stats)
{
	struct nss_capwap_tunnel_stats tstats;
	int i;

	if (!stats) {
		nss_capwapmgr_warn("%px: invalid rtnl structure\n", dev);
		return stats;
	}

	/*
	 * Netdev seems to be incrementing rx_dropped because we don't give IP header.
	 * So reset it as it's of no use for us.
	 */
	atomic_long_set(&dev->rx_dropped, 0);

	memset(stats, 0, sizeof (struct rtnl_link_stats64));
	nss_capwapmgr_fill_up_stats(stats, &global.tunneld_stats);

	for (i = NSS_DYNAMIC_IF_START; i <= (NSS_DYNAMIC_IF_START + NSS_MAX_DYNAMIC_INTERFACES); i++) {
		if (nss_capwap_get_stats(i, &tstats) == false) {
			continue;
		}

		nss_capwapmgr_fill_up_stats(stats, &tstats);
	}

	return stats;
}

/*
 * nss_capwapmgr_dev_tunnel_stats()
 *	Netdev ops function to retrieve stats for kernel version > 4.6
 */
static void nss_capwapmgr_dev_tunnel_stats(struct net_device *dev, struct rtnl_link_stats64 *stats)
{
	nss_capwapmgr_get_tunnel_stats(dev, stats);
}

/*
 * nss_capwapmgr_netdev_ops
 *	Netdev operations.
 */
static const struct net_device_ops nss_capwapmgr_netdev_ops = {
	.ndo_open		= nss_capwapmgr_open,
	.ndo_stop		= nss_capwapmgr_close,
	.ndo_start_xmit		= nss_capwapmgr_start_xmit,
	.ndo_set_mac_address	= eth_mac_addr,
	.ndo_change_mtu		= eth_change_mtu,
	.ndo_get_stats64	= nss_capwapmgr_dev_tunnel_stats,
};

/*
 * nss_capwapmgr_dummy_netdev_setup()
 *	Netdev setup function.
 */
static void nss_capwapmgr_dummy_netdev_setup(struct net_device *dev)
{
	dev->addr_len = ETH_ALEN;
	dev->mtu = ETH_DATA_LEN;
	dev->needed_headroom = NSS_CAPWAP_HEADROOM;
	dev->needed_tailroom = 4;
	dev->type = ARPHRD_VOID;
	dev->ethtool_ops = NULL;
	dev->header_ops = NULL;
	dev->netdev_ops = &nss_capwapmgr_netdev_ops;
	dev->priv_destructor = NULL;

	memcpy(dev->dev_addr, "\x00\x00\x00\x00\x00\x00", dev->addr_len);
	memset(dev->broadcast, 0xff, dev->addr_len);
	memcpy(dev->perm_addr, dev->dev_addr, dev->addr_len);
}

/*
 * nss_capwapmgr_msg_event_receive()
 *	CAPWAP message callback for responses to commands sent to NSS FW
 *
 * This is command hanlder for all the messages since all we do is wake-up
 * the caller who is sending message to NSS FW.
 */
static void nss_capwapmgr_msg_event_receive(void *app_data, struct nss_capwap_msg *nim)
{
	struct net_device *dev = app_data;
	struct nss_cmn_msg *ncm = (struct nss_cmn_msg *)nim;
	struct nss_capwapmgr_response *r;
	struct nss_capwapmgr_priv *priv;
	uint32_t if_num;

	if (ncm->response == NSS_CMN_RESPONSE_NOTIFY) {
		return;
	}

	/*
	 * Since all CAPWAP messages are sync in nature we need to wake-up caller.
	 */
	if_num = ncm->interface - NSS_DYNAMIC_IF_START;
	dev_hold(dev);
	priv = netdev_priv(dev);
	r = &priv->resp[if_num];

	/*
	 * If somebody is waiting...
	 */
	if (atomic_read(&r->seq) != 0) {
		if (ncm->response != NSS_CMN_RESPONSE_ACK) {
			r->error = ncm->error;
		}

		r->response = ncm->response;
		atomic_dec(&r->seq);
		wake_up(&r->wq);
	}

	dev_put(dev);
}

/*
 * nss_capwap_remap_error()
 *	Remaps NSS FW response error to nss_capwapmgr_status_t
 */
static nss_capwapmgr_status_t nss_capwap_remap_error(nss_capwap_msg_response_t error)
{
	nss_capwapmgr_status_t status;

	switch (error) {
	case NSS_CAPWAP_ERROR_MSG_INVALID_REASSEMBLY_TIMEOUT:
		status = NSS_CAPWAPMGR_FAILURE_INVALID_REASSEMBLY_TIMEOUT;
		break;
	case NSS_CAPWAP_ERROR_MSG_INVALID_PATH_MTU:
		status = NSS_CAPWAPMGR_FAILURE_INVALID_PATH_MTU;
		break;
	case NSS_CAPWAP_ERROR_MSG_INVALID_MAX_FRAGMENT:
		status = NSS_CAPWAPMGR_FAILURE_INVALID_MAX_FRAGMENT;
		break;
	case NSS_CAPWAP_ERROR_MSG_INVALID_BUFFER_SIZE:
		status = NSS_CAPWAPMGR_FAILURE_INVALID_BUFFER_SIZE;
		break;
	case NSS_CAPWAP_ERROR_MSG_INVALID_L3_PROTO:
		status = NSS_CAPWAPMGR_FAILURE_INVALID_L3_PROTO;
		break;
	case NSS_CAPWAP_ERROR_MSG_INVALID_UDP_PROTO:
		status = NSS_CAPWAPMGR_FAILURE_INVALID_UDP_PROTO;
		break;
	case NSS_CAPWAP_ERROR_MSG_INVALID_VERSION:
		status = NSS_CAPWAPMGR_FAILURE_INVALID_VERSION;
		break;
	case NSS_CAPWAP_ERROR_MSG_TUNNEL_DISABLED:
		status = NSS_CAPWAPMGR_FAILURE_TUNNEL_DISABLED;
		break;
	case NSS_CAPWAP_ERROR_MSG_TUNNEL_ENABLED:
		status = NSS_CAPWAPMGR_FAILURE_TUNNEL_ENABLED;
		break;
	case NSS_CAPWAP_ERROR_MSG_TUNNEL_NOT_CFG:
		status = NSS_CAPWAPMGR_FAILURE_TUNNEL_NOT_CFG;
		break;
	default:
		status = NSS_CAPWAPMGR_FAILURE;
	}

	return status;
}

/*
 * nss_capwapmgr_get_tunnel()
 *	Common function to verify tunnel.
 *
 * The caller of the function should hold reference to the net device before calling.
 */
static nss_capwapmgr_status_t nss_capwapmgr_get_tunnel(struct net_device *dev, uint8_t tunnel_id, struct nss_capwapmgr_tunnel **t)
{
	struct nss_capwapmgr_priv *priv;

	if (!dev) {
		nss_capwapmgr_warn("Invalid net_device\n");
		return NSS_CAPWAPMGR_INVALID_NETDEVICE;
	}

	if (tunnel_id > NSS_CAPWAPMGR_MAX_TUNNELS) {
		nss_capwapmgr_warn("%px: tunnel_id: %d out of range (%d)\n", dev, tunnel_id, NSS_CAPWAPMGR_MAX_TUNNELS);
		return NSS_CAPWAPMGR_MAX_TUNNEL_COUNT_EXCEEDED;
	}

	priv = netdev_priv(dev);
	*t = &priv->tunnel[tunnel_id];
	if ( ((*t)->if_num_inner == -1) || ((*t)->if_num_outer == -1) ) {
		return NSS_CAPWAPMGR_FAILURE_TUNNEL_DOES_NOT_EXIST;
	}

	return NSS_CAPWAPMGR_SUCCESS;
}

/*
 * nss_capwapmgr_receive_pkt()
 *	Receives a pkt from NSS
 */
static void nss_capwapmgr_receive_pkt(struct net_device *dev, struct sk_buff *skb, struct napi_struct *napi)
{
	struct nss_capwapmgr_priv *priv;
	struct nss_capwap_metaheader *pre = (struct nss_capwap_metaheader *)skb->data;
	int32_t if_num;

	if (unlikely(skb->len < sizeof(struct nss_capwap_metaheader))) {
		nss_capwapmgr_warn("%px: skb len is short :%d", dev, skb->len);
		dev_kfree_skb_any(skb);
		return;
	}

	/*
	 * SKB NETIF START
	 */
	dev_hold(dev);
	priv = netdev_priv(dev);

	/*
	 * NSS FW sends interface number
	 */
	if_num = pre->tunnel_id;
	if (unlikely(if_num > NSS_MAX_DYNAMIC_INTERFACES)) {
		nss_capwapmgr_warn("%px: if_num %d is wrong for skb\n", dev, if_num);
		pre->tunnel_id = 0xFF;
	} else {
		/*
		 * Remap interface number to tunnel_id.
		 */
		pre->tunnel_id = priv->if_num_to_tunnel_id[if_num];
	}

	if (unlikely(pre->type & NSS_CAPWAP_PKT_TYPE_PADDED)) {
		skb_trim(skb, (skb->len - NSS_CAPWAP_PADDING));
		pre->type &= ~NSS_CAPWAP_PKT_TYPE_PADDED;
	}

	skb->dev = dev;
	skb->pkt_type = PACKET_HOST;
	skb->skb_iif = dev->ifindex;
	skb_reset_mac_header(skb);
	skb_reset_transport_header(skb);
	(void)netif_receive_skb(skb);

	/*
	 * SKB NETIF END
	 */
	dev_put(dev);
}

/*
 * nss_capwapmgr_receive_pkt_ppe_vp()
 *	Receives a pkt from ppe.
 */
static bool nss_capwapmgr_receive_pkt_ppe_vp(struct ppe_vp_cb_info *info, void *cb_data)
{
	struct sk_buff *skb = info->skb;
	struct net_device *parent_netdev = (struct net_device *)cb_data;

	nss_capwapmgr_assert(parent_dev, "Parent netdev is NULL for internal dev %p", skb->dev);
	nss_capwapmgr_receive_pkt(parent_netdev, skb, NULL);

	return true;
}

/*
 * nss_capwapmgr_register_with_nss()
 *	Internal function to register with NSS FW.
 */
static nss_capwapmgr_status_t nss_capwapmgr_register_with_nss(uint32_t interface_num, struct net_device *dev)
{
	/* features denote the skb_types supported */
	uint32_t features = 0;

	struct nss_ctx_instance *ctx = nss_capwap_data_register(interface_num, nss_capwapmgr_receive_pkt, dev, features);
	if (!ctx) {
		nss_capwapmgr_warn("%px: %d: nss_capwapmgr_data_register failed\n", dev, interface_num);
		return NSS_CAPWAPMGR_FAILURE;
	}

	return NSS_CAPWAPMGR_SUCCESS;
}

/*
 * nss_capwapmgr_unregister_with_nss()
 *	Internal function to unregister with NSS FW
 */
static void nss_capwapmgr_unregister_with_nss(uint32_t if_num)
{
	nss_capwapmgr_trace("%d: unregister with NSS FW\n", if_num);
	nss_capwap_data_unregister(if_num);
}

/*
 * nss_capwapmgr_tx_msg_sync()
 *	Waits for message to return.
 */
static nss_capwapmgr_status_t nss_capwapmgr_tx_msg_sync(struct nss_ctx_instance *ctx, struct net_device *dev, struct nss_capwap_msg *msg)
{
	struct nss_capwapmgr_priv *priv;
	struct nss_capwapmgr_response *r;
	uint32_t if_num;
	nss_capwapmgr_status_t status;

	if_num = msg->cm.interface - NSS_DYNAMIC_IF_START;
	dev_hold(dev);
	priv = netdev_priv(dev);
	r = &priv->resp[if_num];
	down(&r->sem);
	r->response = NSS_CMN_RESPONSE_ACK;

	/*
	 * Indicate that we are waiting
	 */
	atomic_set(&r->seq, 1);

	/*
	 * Call NSS driver
	 */
	status = nss_capwap_tx_msg(ctx, msg);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		up(&r->sem);
		dev_put(dev);
		return status;
	}

	if (!wait_event_timeout(r->wq, atomic_read(&r->seq) == 0, 5 * HZ)) {

		/*
		 * Indicate that we are no longer waiting
		 */
		atomic_set(&r->seq, 0);
		up(&r->sem);
		nss_capwapmgr_warn("%px: CAPWAP command msg response timeout\n", ctx);
		dev_put(dev);
		return NSS_CAPWAPMGR_FAILURE_CMD_TIMEOUT;
	}

	/*
	 * If NSS FW responded back with an error.
	 */
	if (r->response != NSS_CMN_RESPONSE_ACK) {
		up(&r->sem);
		nss_capwapmgr_warn("%px: CAPWAP command msg response : %d, error:%d\n", ctx,
				r->response, r->error);
		dev_put(dev);
		return nss_capwap_remap_error(r->error);
	}

	up(&r->sem);
	dev_put(dev);
	return status;
}

/*
 * nss_capwapmgr_create_capwap_rule()
 *	Internal function to create a CAPWAP rule
 */
static nss_capwapmgr_status_t nss_capwapmgr_create_capwap_rule(struct net_device *dev,
	uint32_t if_num, struct nss_capwap_rule_msg *msg, uint16_t type_flags)
{
	struct nss_ctx_instance *ctx = nss_capwap_get_ctx();
	struct nss_capwap_msg capwapmsg;
	struct nss_capwap_rule_msg *capwapcfg;
	nss_tx_status_t status;

	nss_capwapmgr_info("%px: ctx: CAPWAP Rule src_port: 0x%d dest_port:0x%d\n", ctx,
	    ntohl(msg->encap.src_port), ntohl(msg->encap.dest_port));

	/*
	 * Verify CAPWAP rule parameters.
	 */
	if (ntohl(msg->decap.reassembly_timeout) > NSS_CAPWAP_MAX_REASSEMBLY_TIMEOUT) {
		nss_capwapmgr_warn("%px: invalid reassem timeout: %d, max: %d\n",
			ctx, ntohl(msg->decap.reassembly_timeout), NSS_CAPWAP_MAX_REASSEMBLY_TIMEOUT);
		return NSS_CAPWAPMGR_FAILURE_INVALID_REASSEMBLY_TIMEOUT;
	}

	if (msg->decap.reassembly_timeout == 0) {

		/*
		 * 10 milli-seconds
		 */
		msg->decap.reassembly_timeout = htonl(NSS_CAPWAPMGR_REASSEMBLY_TIMEOUT);
	}

	if (ntohl(msg->decap.max_fragments) > NSS_CAPWAP_MAX_FRAGMENTS) {
		nss_capwapmgr_warn("%px: invalid fragment setting: %d, max: %d\n",
			ctx, ntohl(msg->decap.max_fragments), NSS_CAPWAP_MAX_FRAGMENTS);
		return NSS_CAPWAPMGR_FAILURE_INVALID_MAX_FRAGMENT;
	}

	if (msg->decap.max_fragments == 0) {
		msg->decap.max_fragments = htonl(NSS_CAPWAP_MAX_FRAGMENTS);
	}

	if (ntohl(msg->decap.max_buffer_size) > NSS_CAPWAP_MAX_BUFFER_SIZE) {
		nss_capwapmgr_warn("%px: invalid buffer size: %d, max: %d\n",
			ctx, ntohl(msg->decap.max_buffer_size), NSS_CAPWAP_MAX_BUFFER_SIZE);
		return NSS_CAPWAPMGR_FAILURE_INVALID_BUFFER_SIZE;
	}

	if (msg->decap.max_buffer_size == 0) {
		msg->decap.max_buffer_size = htonl(nss_capwap_get_max_buf_size(ctx));
	}

	if (ntohl(msg->encap.path_mtu) > NSS_CAPWAP_MAX_MTU) {
		nss_capwapmgr_warn("%px: invalid path_mtu: %d, max: %d\n",
			ctx, ntohl(msg->encap.path_mtu), NSS_CAPWAP_MAX_MTU);
		return NSS_CAPWAPMGR_FAILURE_INVALID_PATH_MTU;
	}

	if (msg->encap.path_mtu == 0) {
		msg->encap.path_mtu = htonl(NSS_CAPWAPMGR_NORMAL_FRAME_MTU);
	}

	msg->type_flags = type_flags;

	/*
	 * Prepare the tunnel configuration parameter to send to NSS FW
	 */
	memset(&capwapmsg, 0, sizeof(struct nss_capwap_msg));
	capwapcfg = &capwapmsg.msg.rule;
	memcpy(capwapcfg, msg, sizeof(struct nss_capwap_rule_msg));

	/*
	 * Send CAPWAP tunnel create command to NSS
	 */
	nss_capwap_msg_init(&capwapmsg, if_num, NSS_CAPWAP_MSG_TYPE_CFG_RULE,
			sizeof(struct nss_capwap_rule_msg),
			nss_capwapmgr_msg_event_receive, dev);

	status = nss_capwapmgr_tx_msg_sync(ctx, dev, &capwapmsg);
	if (status != NSS_TX_SUCCESS) {
		nss_capwapmgr_warn("%px: ctx: create encap data tunnel error %d \n", ctx, status);
		return status;
	}

	return NSS_CAPWAPMGR_SUCCESS;
}

/*
 * nss_capwapmgr_update_pppoe_rule()
 *	Updates PPPoE rule from PPPoE netdevice.
 */
static bool nss_capwapmgr_update_pppoe_rule(struct net_device *dev, struct ppe_drv_pppoe_session *pppoe_rule)
{
	struct pppoe_opt addressing;
	struct ppp_channel *channel[1] = {NULL};
	int px_proto;
	int ppp_ch_count;
	bool status = true;

	if (ppp_is_multilink(dev)) {
		nss_capwapmgr_warn("%px: channel is multilink PPP\n", dev);
		goto fail;
	}

	ppp_ch_count = ppp_hold_channels(dev, channel, 1);
	nss_capwapmgr_info("%px: PPP hold channel ret %d\n", dev, ppp_ch_count);
	if (ppp_ch_count != 1) {
		nss_capwapmgr_warn("%px: hold channel for netdevice failed\n", dev);
		goto fail;
	}

	px_proto = ppp_channel_get_protocol(channel[0]);
	if (px_proto != PX_PROTO_OE) {
		nss_capwapmgr_warn("%px: session socket is not of type PX_PROTO_OE\n", dev);
		goto fail;
	}

	if (pppoe_channel_addressing_get(channel[0], &addressing)) {
		nss_capwapmgr_warn("%px: failed to get addressing information\n", dev);
		goto fail;
	}

	/*
	 * Update the PPPoE rule information and set PPPoE valid flag.
	 */
	pppoe_rule->session_id = (uint16_t)ntohs((uint16_t)addressing.pa.sid);
	memcpy(pppoe_rule->server_mac, addressing.pa.remote, ETH_ALEN);

	nss_capwapmgr_info("%px: Update PPE PPPoE flow rule with session_id = %u, remote_mac = %pM\n",
			dev, pppoe_rule->session_id, pppoe_rule->server_mac);

	/*
	 * pppoe_channel_addressing_get returns held device.
	 */
	dev_put(addressing.dev);
	goto done;

fail:
	ppp_release_channels(channel, 1);
	status = false;
done:
	return status;
}

/*
 * nss_capwapmgr_tx_rule_destroy
 *	Function to Unconfigure the PPE tunnel encapsulation rule.
 */
static nss_capwapmgr_status_t nss_capwapmgr_tx_rule_destroy(struct nss_capwapmgr_tunnel *t, bool trustsec_enabled)
{
	fal_tunnel_id_t tunnel_id_bckup = {0};
	fal_tunnel_id_t tunnel_id = {0};
	fal_tunnel_encap_cfg_t cfg = {0};
	sw_error_t sw_err = SW_OK;
	nss_capwapmgr_status_t status = NSS_CAPWAPMGR_SUCCESS;
	uint32_t v_port;
	int32_t port;
	uint32_t dev_id = NSS_CAPWAPMGR_DEV_ID;

	/*
	 * Get the tunnel associated to the VP.
	 */
	v_port = FAL_PORT_ID(FAL_PORT_TYPE_VPORT, t->vp_num_encap);
	sw_err = fal_tunnel_encap_port_tunnelid_get(dev_id, v_port, &tunnel_id_bckup);
	if (sw_err != SW_OK) {
		nss_capwapmgr_warn("Failed to get tunnelid for tx VP %d\n",t->vp_num_encap);
		status = NSS_CAPWAPMGR_FAILURE_TUNNEL_ID_GET;
		goto done;
	}

	/*
	 * Get the tunnel encap entry associated with the tunnel.
	 */
	sw_err = fal_tunnel_encap_entry_get(dev_id, tunnel_id_bckup.tunnel_id, &cfg);
	if (sw_err != SW_OK) {
		nss_capwapmgr_warn("Failed to add tunnel encap entry\n");
		status = NSS_CAPWAPMGR_FAILURE_TUNNEL_ENCAP_ENTRY_GET;
		goto done;
	}

	/*
	 * Get the physical port associated with the VP.
	 */
	sw_err = fal_vport_physical_port_id_get(dev_id, v_port, &port);
	if(sw_err != SW_OK) {
		nss_capwapmgr_warn("Failed to get the physical port for VP %d\n",
				t->vp_num_encap);
		status  = NSS_CAPWAPMGR_FAILURE_TX_PORT_GET;
		goto fail;
	}

	/*
	 * Delete the encap entry associatd with the tunnel.
	 */
	sw_err = fal_tunnel_encap_entry_del(dev_id, tunnel_id_bckup.tunnel_id);
	if (sw_err != SW_OK) {
		nss_capwapmgr_warn("Failed to add tunnel encap entry \n");
		status = NSS_CAPWAPMGR_FAILURE_TUNNEL_ENCAP_ENTRY_DELETE;
		goto done;
	}

	/*
	 * Unbind the VP.
	 */
	sw_err = fal_vport_physical_port_id_set(dev_id, v_port, 0);
	if(sw_err != SW_OK) {
		nss_capwapmgr_warn("Failed to unbind VP %d\n",
				t->vp_num_encap);
		status  = NSS_CAPWAPMGR_FAILURE_UNBIND_VPORT;
		goto fail;
	}

	/*
	 * Unbind the tunnel associated to the VP
	 */
	sw_err = fal_tunnel_encap_port_tunnelid_set(dev_id, v_port, &tunnel_id);
	if (sw_err != SW_OK) {
		nss_capwapmgr_warn("Failed to remove tunnelid for tx %d\n", t->vp_num_encap);
		status = NSS_CAPWAPMGR_FAILURE_TUNNEL_ID_SET;
		goto fail1;
	}

	if (trustsec_enabled) {
		t->tunnel_state &= ~NSS_CAPWAPMGR_TUNNEL_STATE_TRUSTSEC_TX_CONFIGURED;
	}

	goto done;

fail1:
	fal_vport_physical_port_id_set(dev_id, v_port, port);
fail:
	fal_tunnel_encap_entry_add(dev_id, tunnel_id_bckup.tunnel_id, &cfg);
done:
	return status;
}

/*
 * nss_capwapmgr_ppe_destroy_ipv4_flow_rule
 *	Internal Function to destroy IPv4 flow rule.
 */
static nss_capwapmgr_status_t nss_capwapmgr_ppe_destroy_ipv4_flow_rule(struct nss_ipv4_create *v4)
{
	ppe_drv_ret_t ppe_status;
	struct ppe_drv_v4_rule_destroy pd4rd;
	memset(&pd4rd, 0, sizeof (struct ppe_drv_v4_rule_destroy));
	pd4rd.tuple.protocol = IPPROTO_UDP;
	pd4rd.tuple.flow_ip = v4->src_ip;
	pd4rd.tuple.flow_ident = (uint32_t)v4->src_port;
	pd4rd.tuple.return_ip = v4->dest_ip;
	pd4rd.tuple.return_ident = (uint32_t)v4->dest_port;

	ppe_status = ppe_drv_v4_destroy(&pd4rd);
	if ((ppe_status != PPE_DRV_RET_SUCCESS) && (ppe_status != PPE_DRV_RET_FAILURE_DESTROY_NO_CONN)) {
		nss_capwapmgr_warn("%px: Unconfigure ipv4 flow rule failed : %d\n", v4, ppe_status);
		return NSS_CAPWAPMGR_FAILURE_IP_DESTROY_RULE;
	}

	return NSS_CAPWAPMGR_SUCCESS;
}

/*
 * nss_capwapmgr_ppe_destroy_ipv4_rule
 *	Internal Function to destroy IPv4 connection.
 */
static nss_capwapmgr_status_t nss_capwapmgr_ppe_destroy_ipv4_rule(struct nss_capwapmgr_tunnel *t)
{
	nss_capwapmgr_status_t status = NSS_CAPWAPMGR_SUCCESS;

	status = nss_capwapmgr_ppe_destroy_ipv4_flow_rule(&t->ip_rule.v4);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		return status;
	}

	status = nss_capwapmgr_tx_rule_destroy(t, false);
	if (status!= NSS_CAPWAPMGR_SUCCESS) {
		return NSS_CAPWAPMGR_FAILURE_IP_DESTROY_RULE;
	}

	t->tunnel_state &= ~NSS_CAPWAPMGR_TUNNEL_STATE_IPRULE_CONFIGURED;
	return NSS_CAPWAPMGR_SUCCESS;
}

/*
 * nss_capwapmgr_tx_rule_create_v4
 *	Function to configure PPE tunnel encapsulation to handle
 *	Egress(AP->AC) traffic.
 */
static nss_capwapmgr_status_t nss_capwapmgr_tx_rule_create_v4(struct nss_capwapmgr_tunnel *t, struct nss_ipv4_create *v4, bool trustsec_enabled)
{
	fal_tunnel_id_t tunnel_id = {0};
	fal_tunnel_encap_cfg_t cfg = {0};
	sw_error_t sw_err = SW_OK;
	nss_capwapmgr_status_t status = NSS_CAPWAPMGR_SUCCESS;
	struct ppe_drv_iface *iface;
	uint32_t v_port;
	int32_t port_num;
	uint8_t *data;
	uint32_t dev_id = NSS_CAPWAPMGR_DEV_ID;
	uint16_t ether_type = htons(trustsec_enabled ? NSS_CAPWAPMGR_ETH_TYPE_TRUSTSEC:NSS_CAPWAPMGR_ETH_TYPE_IPV4);
	uint16_t vlan_tpid;
	uint16_t vlan_tci;

	/*
	 * Get the port associated to the source interface
	 */
	iface = ppe_drv_iface_get_by_idx(v4->src_interface_num);
	port_num = ppe_drv_iface_port_idx_get(iface);

	/*
	 * Update the tunnel id to be used for trustsec_tx.
	 */
	tunnel_id.tunnel_id_valid = true;
	tunnel_id.tunnel_id = t->tunnel_id;

	v_port = FAL_PORT_ID(FAL_PORT_TYPE_VPORT, t->vp_num_encap);
	sw_err = fal_tunnel_encap_port_tunnelid_set(dev_id, v_port, &tunnel_id);
	if (sw_err != SW_OK) {
		nss_capwapmgr_warn("Failed to set tunnelid for V4 UL VP %d\n", t->vp_num_encap);
		status = NSS_CAPWAPMGR_FAILURE_TUNNEL_ID_SET;
		goto done;
	}

	/*
	 * Bind the VP to the phyical port.
	 */
	sw_err = fal_vport_physical_port_id_set(dev_id, v_port, port_num);
	if(sw_err != SW_OK) {
		nss_capwapmgr_warn("Failed to bind trustsec VP %d to the underlying physical port %d\n",
				t->vp_num_encap, port_num);
		status  = NSS_CAPWAPMGR_FAILURE_BIND_VPORT;
		goto fail;
	}

	/*
	 * Inner payload type is IP for normal tunnels, ETHERNET for trustsec tunnel.
	 */
	cfg.payload_inner_type = trustsec_enabled ? NSS_CAPWAPMGR_TX_INNER_PAYLOAD_TYPE_ETHER:NSS_CAPWAPMGR_TX_INNER_PAYLOAD_TYPE_IP;

	cfg.tunnel_len = 0;

	/*
	 * Update destination mac address for egress traffic.
	 * v4 rule is for AC->AP direction and hence for egress traffic the destination mac will be
	 * the src mac in the rule.
	 */
	data = cfg.pkt_header.pkt_header_data;
	memcpy(data, v4->src_mac, ETH_ALEN);
	data+=ETH_ALEN;

	/*
	 * Update source mac address for egress traffic.
	 */
	memcpy(data, v4->dest_mac, ETH_ALEN);
	data+=ETH_ALEN;

	cfg.tunnel_len += 2 * ETH_ALEN;

	/*
	 * Update cvlan tag if present in the rule.
	 */
	if ((v4->in_vlan_tag[0] & 0xFFF) != 0xFFF) {
		cfg.cvlan_fmt = NSS_CAPWAPMGR_TX_CVLAN_ENABLED;
		cfg.vlan_offset = NSS_CAPWAPMGR_TX_VLAN_OFFSET;

		/*
		 * write the tpid
		 */
		vlan_tpid = htons((v4->in_vlan_tag[0] & NSS_CAPWAPMGR_TX_VLAN_TPID_MASK) >> NSS_CAPWAPMGR_TX_VLAN_TPID_SHIFT);
		memcpy(data, &vlan_tpid, NSS_CAPWAPMGR_TX_VLAN_TPID_SIZE);
		data+=NSS_CAPWAPMGR_TX_VLAN_TPID_SIZE;

		/*
		 * Write the tci
		 */
		vlan_tci = htons(v4->in_vlan_tag[0] & NSS_CAPWAPMGR_TX_VLAN_TCI_MASK);
		memcpy(data, &vlan_tci, NSS_CAPWAPMGR_TX_VLAN_TCI_SIZE);
		data+=NSS_CAPWAPMGR_TX_VLAN_TCI_SIZE;

		cfg.tunnel_len += NSS_CAPWAPMGR_TX_VLAN_TCI_SIZE + NSS_CAPWAPMGR_TX_VLAN_TPID_SIZE;
	}

	/*
	 * Update the ether_type.
	 */
	memcpy(data, &ether_type, NSS_CAPWAPMGR_ETH_TYPE_SIZE);
	cfg.tunnel_len += NSS_CAPWAPMGR_ETH_TYPE_SIZE;

	sw_err = fal_tunnel_encap_entry_add(dev_id, tunnel_id.tunnel_id, &cfg);
	if (sw_err != SW_OK) {
		nss_capwapmgr_warn("Failed to add tunnel encap entry \n");
		status = NSS_CAPWAPMGR_FAILURE_TUNNEL_ENCAP_ENTRY_ADD;
		goto fail1;
	}

	if (trustsec_enabled) {
		t->tunnel_state |= NSS_CAPWAPMGR_TUNNEL_STATE_TRUSTSEC_TX_CONFIGURED;
	}

	goto done;

fail1:
	fal_vport_physical_port_id_set(dev_id, v_port, 0);
fail:
	tunnel_id.tunnel_id_valid = false;
	fal_tunnel_encap_port_tunnelid_set(dev_id, v_port, &tunnel_id);
done:
	return status;
}

/*
 * nss_capwapmgr_tx_rule_create_v6
 *	Function to configure PPE tunnel encapsulation to handle
 *	Egress(AP->AC) traffic.
 */
static nss_capwapmgr_status_t nss_capwapmgr_tx_rule_create_v6(struct nss_capwapmgr_tunnel *t, struct nss_ipv6_create *v6, bool trustsec_enabled)
{
	fal_tunnel_id_t tunnel_id = {0};
	fal_tunnel_encap_cfg_t cfg = {0};
	sw_error_t sw_err = SW_OK;
	nss_capwapmgr_status_t status = NSS_CAPWAPMGR_SUCCESS;
	struct ppe_drv_iface *iface;
	int32_t port_num;
	uint32_t v_port;
	uint8_t *data;
	uint32_t dev_id = NSS_CAPWAPMGR_DEV_ID;
	uint16_t ether_type = htons(trustsec_enabled ? NSS_CAPWAPMGR_ETH_TYPE_TRUSTSEC:NSS_CAPWAPMGR_ETH_TYPE_IPV6);
	uint16_t vlan_tpid;
	uint16_t vlan_tci;

	/*
	 * Get the port number assocaited with the interface.
	 */
	iface = ppe_drv_iface_get_by_idx(v6->src_interface_num);
	port_num = ppe_drv_iface_port_idx_get(iface);

	/*
	 * Update the tunnel id to be used for trustsec_tx.
	 */
	tunnel_id.tunnel_id_valid = true;
	tunnel_id.tunnel_id = t->tunnel_id;

	v_port = FAL_PORT_ID(FAL_PORT_TYPE_VPORT, t->vp_num_encap);
	sw_err = fal_tunnel_encap_port_tunnelid_set(dev_id, v_port, &tunnel_id);
	if (sw_err != SW_OK) {
		nss_capwapmgr_warn("Failed to set tunnelid for UL V6 VP %d\n",t->vp_num_encap);
		status = NSS_CAPWAPMGR_FAILURE_TUNNEL_ID_SET;
		goto done;
	}

	/*
	 * Bind the VP to the phyical port.
	 */
	sw_err = fal_vport_physical_port_id_set(dev_id, v_port, port_num);
	if(sw_err != SW_OK) {
		nss_capwapmgr_warn("Failed to bind UL V6 VP %d to the underlying physical port %d\n",
				t->vp_num_encap, port_num);
		status  = NSS_CAPWAPMGR_FAILURE_BIND_VPORT;
		goto fail;
	}

	/*
	 * Inner payload type is IP.
	 */
	cfg.payload_inner_type = trustsec_enabled ? NSS_CAPWAPMGR_TX_INNER_PAYLOAD_TYPE_ETHER:NSS_CAPWAPMGR_TX_INNER_PAYLOAD_TYPE_IP;

	/*
	 * Update destination mac address for egress traffic.
	 * v6 rule is for AC->AP direction and hence for egress traffic the destination mac will be
	 * the src mac in the rule.
	 */
	data = cfg.pkt_header.pkt_header_data;
	memcpy(data, v6->src_mac, ETH_ALEN);
	data+=ETH_ALEN;

	/*
	 * Update source mac address for egress traffic.
	 */
	memcpy(data, v6->dest_mac, ETH_ALEN);
	data+=ETH_ALEN;

	cfg.tunnel_len += 2 * ETH_ALEN;

	/*
	 * Update cvlan tag if present in the rule.
	 */
	if ((v6->in_vlan_tag[0] & 0xFFF) != 0xFFF) {
		cfg.cvlan_fmt = NSS_CAPWAPMGR_TX_CVLAN_ENABLED;
		cfg.vlan_offset = NSS_CAPWAPMGR_TX_VLAN_OFFSET;

		/*
		 * write the tpid
		 */
		vlan_tpid = htons((v6->in_vlan_tag[0] & NSS_CAPWAPMGR_TX_VLAN_TPID_MASK) >> NSS_CAPWAPMGR_TX_VLAN_TPID_SHIFT);
		memcpy(data, &vlan_tpid, NSS_CAPWAPMGR_TX_VLAN_TPID_SIZE);
		data+=NSS_CAPWAPMGR_TX_VLAN_TPID_SIZE;

		/*
		 * Write the tci
		 */
		vlan_tci = htons(v6->in_vlan_tag[0] & NSS_CAPWAPMGR_TX_VLAN_TCI_MASK);
		memcpy(data, &vlan_tci, NSS_CAPWAPMGR_TX_VLAN_TCI_SIZE);
		data+=NSS_CAPWAPMGR_TX_VLAN_TCI_SIZE;

		cfg.tunnel_len += NSS_CAPWAPMGR_TX_VLAN_TCI_SIZE + NSS_CAPWAPMGR_TX_VLAN_TPID_SIZE;
	}

	/*
	 * Update the ether_type.
	 */
	memcpy(data, &ether_type, NSS_CAPWAPMGR_ETH_TYPE_SIZE);
	cfg.tunnel_len += NSS_CAPWAPMGR_ETH_TYPE_SIZE;

	sw_err = fal_tunnel_encap_entry_add(dev_id, tunnel_id.tunnel_id, &cfg);
	if (sw_err != SW_OK) {
		nss_capwapmgr_warn("Failed to add tunnel encap entry \n");
		status = NSS_CAPWAPMGR_FAILURE_TUNNEL_ENCAP_ENTRY_ADD;
		goto fail1;
	}

	if (trustsec_enabled) {
		t->tunnel_state |= NSS_CAPWAPMGR_TUNNEL_STATE_TRUSTSEC_TX_CONFIGURED;
	}

	goto done;

fail1:
	fal_vport_physical_port_id_set(dev_id, v_port, 0);
fail:
	tunnel_id.tunnel_id_valid = false;
	fal_tunnel_encap_port_tunnelid_set(dev_id, v_port, &tunnel_id);
done:
	return status;
}

/*
 * nss_capwapmgr_ppe_create_ipv4_rule
 *	Internal function to configure IPv4 connection.
 */
static nss_capwapmgr_status_t nss_capwapmgr_ppe_create_ipv4_rule(struct nss_capwapmgr_tunnel *t, struct nss_ipv4_create *v4)
{
	struct ppe_drv_v4_rule_create *pd4rc;
	ppe_drv_ret_t ppe_status;
	nss_capwapmgr_status_t status = NSS_CAPWAPMGR_SUCCESS;
	bool use_top_interface = false;

	/*
	 * Create the PPE flow rule to handle DL traffic.
	 */
	pd4rc = (struct ppe_drv_v4_rule_create *)kzalloc(sizeof(struct ppe_drv_v4_rule_create), GFP_KERNEL);
	if (!pd4rc) {
		nss_capwapmgr_warn("%px: No memory to allocate ppe ipv4 rule create structure\n", t);
		goto fail;
	}

	pd4rc->valid_flags = 0;
	pd4rc->rule_flags = 0;

	/*
	 * Copy over the 5 tuple details.
	 */
	pd4rc->tuple.protocol = (uint8_t)v4->protocol;
	pd4rc->tuple.flow_ip = v4->src_ip;
	pd4rc->tuple.flow_ident = (uint32_t)v4->src_port;
	pd4rc->tuple.return_ip = v4->dest_ip;
	pd4rc->tuple.return_ident = (uint32_t)v4->dest_port;

	/*
	 * Copy over the connection rules and set the CONN_VALID flag
	 */
	pd4rc->conn_rule.rx_if = v4->src_interface_num;
	pd4rc->conn_rule.flow_mtu = v4->from_mtu;
	pd4rc->conn_rule.flow_ip_xlate = v4->src_ip_xlate;
	pd4rc->conn_rule.flow_ident_xlate = (uint32_t)v4->src_port_xlate;
	memcpy(pd4rc->conn_rule.flow_mac, v4->src_mac, 6);

	pd4rc->conn_rule.tx_if = v4->dest_interface_num;
	pd4rc->conn_rule.return_mtu = v4->to_mtu;
	pd4rc->conn_rule.return_ip_xlate = v4->dest_ip_xlate;
	pd4rc->conn_rule.return_ident_xlate = (uint32_t)v4->dest_port_xlate;
	if (pd4rc->tuple.return_ip != pd4rc->conn_rule.return_ip_xlate ||
		pd4rc->tuple.return_ident != pd4rc->conn_rule.return_ident_xlate)  {
		memcpy(pd4rc->conn_rule.return_mac, v4->dest_mac_xlate, 6);
	} else {
		memcpy(pd4rc->conn_rule.return_mac, v4->dest_mac, 6);
	}

	pd4rc->rule_flags |= PPE_DRV_V4_RULE_FLAG_RETURN_VALID | PPE_DRV_V4_RULE_FLAG_FLOW_VALID;


	if ((v4->in_vlan_tag[0] & 0xFFF) != 0xFFF) {
		/*
		 * Copy over the VLAN tag and set the VLAN_VALID flag.
		 * IP rule direction is AC->AP, so we only update the ingress VLAN tag.
		 */
		pd4rc->vlan_rule.primary_vlan.ingress_vlan_tag = v4->in_vlan_tag[0];
		pd4rc->vlan_rule.primary_vlan.egress_vlan_tag = NSS_CAPWAPMGR_VLAN_TAG_NOT_CONFIGURED;
		pd4rc->vlan_rule.secondary_vlan.ingress_vlan_tag = NSS_CAPWAPMGR_VLAN_TAG_NOT_CONFIGURED;
		pd4rc->vlan_rule.secondary_vlan.egress_vlan_tag = NSS_CAPWAPMGR_VLAN_TAG_NOT_CONFIGURED;
		pd4rc->valid_flags |= PPE_DRV_V4_VALID_FLAG_VLAN;
		use_top_interface = true;
	}

	if (v4->flow_pppoe_if_exist) {
		/*
		 * Copy over the PPPOE rules and set PPPOE_VALID flag.
		 */
		if (!nss_capwapmgr_update_pppoe_rule(v4->top_ndev, &pd4rc->pppoe_rule.flow_session)) {
			nss_capwapmgr_warn("%px:PPPoE rule update failed\n",t);
			goto fail;
		}

		pd4rc->valid_flags |= PPE_DRV_V4_VALID_FLAG_FLOW_PPPOE;
		use_top_interface = true;
	}

	/*
	 * Copy over the qos rules and set the QOS_VALID flag.
	 */
	if (v4->flags & NSS_IPV4_CREATE_FLAG_QOS_VALID) {
		pd4rc->qos_rule.flow_qos_tag = v4->flow_qos_tag;
		pd4rc->qos_rule.return_qos_tag = v4->return_qos_tag;
		pd4rc->valid_flags |= PPE_DRV_V4_VALID_FLAG_QOS;
	}

	/*
	 * Copy over the DSCP rule parameters.
	 */
	if (v4->flags & NSS_IPV4_CREATE_FLAG_DSCP_MARKING) {
		pd4rc->dscp_rule.flow_dscp = v4->flow_dscp;
		pd4rc->dscp_rule.return_dscp = v4->return_dscp;
		pd4rc->rule_flags |= PPE_DRV_V4_RULE_FLAG_DSCP_MARKING;
		pd4rc->valid_flags |= PPE_DRV_V4_VALID_FLAG_DSCP_MARKING;
	}

	/*
	 * If Ingress VLAN tag or pppoe rule is present, set the top interface.
	 */
	if (use_top_interface) {
		pd4rc->top_rule.rx_if = ppe_drv_iface_idx_get_by_dev(v4->top_ndev);
	} else {
		pd4rc->top_rule.rx_if = v4->src_interface_num;
	}

	pd4rc->top_rule.tx_if = v4->dest_interface_num;
	ppe_status = ppe_drv_v4_create(pd4rc);
	if (ppe_status != PPE_DRV_RET_SUCCESS) {
		nss_capwapmgr_warn("%px:PPE rule create failed\n",t);
		goto fail;
	}

	/*
	 * Create PPE tunnel encap entry to handle UL traffic.
	 */
	status = nss_capwapmgr_tx_rule_create_v4(t, v4, false);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_ppe_destroy_ipv4_flow_rule(v4);
		goto fail;
	}

	/*
	 * Update the tunnel state.
	 */
	t->tunnel_state |= NSS_CAPWAPMGR_TUNNEL_STATE_IPRULE_CONFIGURED;
	goto done;

fail:
	status = NSS_CAPWAPMGR_FAILURE_IP_RULE;
done:
	kfree(pd4rc);
	return status;
}

/*
 * nss_capwapmgr_ppe_destroy_ipv6_flow_rule
 *	Internal Function to destroy IPv6 flow rule.
 */
static nss_capwapmgr_status_t nss_capwapmgr_ppe_destroy_ipv6_flow_rule(struct nss_ipv6_create *v6)
{
	ppe_drv_ret_t ppe_status;
	struct ppe_drv_v6_rule_destroy pd6rd;
	memset(&pd6rd, 0, sizeof (struct nss_ipv6_destroy));

	pd6rd.tuple.protocol = (uint8_t)v6->protocol;
	pd6rd.tuple.flow_ip[0] = v6->src_ip[0];
	pd6rd.tuple.flow_ip[1] = v6->src_ip[1];
	pd6rd.tuple.flow_ip[2] = v6->src_ip[2];
	pd6rd.tuple.flow_ip[3] = v6->src_ip[3];

	pd6rd.tuple.return_ip[0] = v6->dest_ip[0];
	pd6rd.tuple.return_ip[1] = v6->dest_ip[1];
	pd6rd.tuple.return_ip[2] = v6->dest_ip[2];
	pd6rd.tuple.return_ip[3] = v6->dest_ip[3];

	pd6rd.tuple.flow_ident = v6->src_port;
	pd6rd.tuple.return_ident = v6->dest_port;
	ppe_status = ppe_drv_v6_destroy(&pd6rd);
	if ((ppe_status != PPE_DRV_RET_SUCCESS) && (ppe_status != PPE_DRV_RET_FAILURE_DESTROY_NO_CONN)) {
		nss_capwapmgr_warn("%px: unconfigure ipv6 rule failed : %d\n", v6, ppe_status);
		return NSS_CAPWAPMGR_FAILURE_IP_DESTROY_RULE;
	}

	return NSS_CAPWAPMGR_SUCCESS;
}

/*
 * nss_capwapmgr_ppe_destroy_ipv6_rule
 *	Internal Function to destroy IPv6 connection.
 */
static nss_capwapmgr_status_t nss_capwapmgr_ppe_destroy_ipv6_rule(struct nss_capwapmgr_tunnel *t)
{
	nss_capwapmgr_status_t status = NSS_CAPWAPMGR_SUCCESS;

	status = nss_capwapmgr_ppe_destroy_ipv6_flow_rule(&t->ip_rule.v6);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		return status;
	}

	status = nss_capwapmgr_tx_rule_destroy(t, false);
	if (status!= NSS_CAPWAPMGR_SUCCESS) {
		return NSS_CAPWAPMGR_FAILURE_IP_DESTROY_RULE;
	}

	t->tunnel_state &= ~NSS_CAPWAPMGR_TUNNEL_STATE_IPRULE_CONFIGURED;
	return NSS_CAPWAPMGR_SUCCESS;
}

/*
 * nss_capwapmgr_ppe_create_ipv6_rule
 *	Internal function to configure IPv6 connection.
 */
static nss_capwapmgr_status_t nss_capwapmgr_ppe_create_ipv6_rule(struct nss_capwapmgr_tunnel *t, struct nss_ipv6_create *v6)
{
	struct ppe_drv_v6_rule_create *pd6rc;
	ppe_drv_ret_t ppe_status;
	nss_capwapmgr_status_t status = NSS_CAPWAPMGR_SUCCESS;
	bool use_top_interface = false;

	/*
	 * Create PPE flow rule to handle DL traffic.
	 */
	pd6rc = (struct ppe_drv_v6_rule_create *)kzalloc(sizeof(struct ppe_drv_v6_rule_create), GFP_KERNEL);
	if (!pd6rc) {
		nss_capwapmgr_warn("%px:No memory to allocate ppe ipv6 rule create structure\n", v6);
		goto fail;
	}

	pd6rc->valid_flags = 0;
	pd6rc->rule_flags = 0;

	/*
	 * Copy over the 5 tuple information.
	 */
	pd6rc->tuple.protocol = (uint8_t)v6->protocol;
	pd6rc->tuple.flow_ip[0] = v6->src_ip[0];
	pd6rc->tuple.flow_ip[1] = v6->src_ip[1];
	pd6rc->tuple.flow_ip[2] = v6->src_ip[2];
	pd6rc->tuple.flow_ip[3] = v6->src_ip[3];
	pd6rc->tuple.flow_ident = (uint32_t)v6->src_port;

	pd6rc->tuple.return_ip[0] = v6->dest_ip[0];
	pd6rc->tuple.return_ip[1] = v6->dest_ip[1];
	pd6rc->tuple.return_ip[2] = v6->dest_ip[2];
	pd6rc->tuple.return_ip[3] = v6->dest_ip[3];
	pd6rc->tuple.return_ident = (uint32_t)v6->dest_port;

	/*
	 * Copy over the connection rules and set the CONN_VALID flag
	 */
	pd6rc->conn_rule.rx_if = v6->src_interface_num;
	pd6rc->conn_rule.flow_mtu = v6->from_mtu;
	memcpy(pd6rc->conn_rule.flow_mac, v6->src_mac, 6);

	pd6rc->conn_rule.tx_if = v6->dest_interface_num;
	pd6rc->conn_rule.return_mtu = v6->to_mtu;
	memcpy(pd6rc->conn_rule.return_mac, v6->dest_mac, 6);

	pd6rc->rule_flags |= PPE_DRV_V6_RULE_FLAG_RETURN_VALID | PPE_DRV_V6_RULE_FLAG_FLOW_VALID;

	if ((v6->in_vlan_tag[0] & 0xFFF) != 0xFFF) {
		/*
		 * Copy over the VLAN tag and set the VLAN_VALID flag.
		 * IP rule direction is AC->AP, so we only update the ingress VLAN tag.
		 */
		pd6rc->vlan_rule.primary_vlan.ingress_vlan_tag = v6->in_vlan_tag[0];
		pd6rc->vlan_rule.primary_vlan.egress_vlan_tag = NSS_CAPWAPMGR_VLAN_TAG_NOT_CONFIGURED;
		pd6rc->vlan_rule.secondary_vlan.ingress_vlan_tag = NSS_CAPWAPMGR_VLAN_TAG_NOT_CONFIGURED;
		pd6rc->vlan_rule.secondary_vlan.egress_vlan_tag = NSS_CAPWAPMGR_VLAN_TAG_NOT_CONFIGURED;
		pd6rc->valid_flags |= PPE_DRV_V6_VALID_FLAG_VLAN;
		use_top_interface = true;
	}

	if (v6->flow_pppoe_if_exist) {
		/*
		 * Copy over the PPPOE rules and set PPPOE_VALID flag.
		 */
		if (!nss_capwapmgr_update_pppoe_rule(v6->top_ndev, &pd6rc->pppoe_rule.flow_session)) {
			nss_capwapmgr_warn("%px:PPPoE rule update failed\n",t);
			goto fail;
		}

		pd6rc->valid_flags |= PPE_DRV_V6_VALID_FLAG_PPPOE_FLOW;
		use_top_interface = true;
	}

	/*
	 * Copy over the qos rules and set the QOS_VALID flag.
	 */
	if (v6->flags & NSS_IPV6_CREATE_FLAG_QOS_VALID) {
		pd6rc->qos_rule.flow_qos_tag = v6->flow_qos_tag;
		pd6rc->qos_rule.return_qos_tag = v6->return_qos_tag;
		pd6rc->valid_flags |= PPE_DRV_V6_VALID_FLAG_QOS;
	}

	/*
	 * Copy over the DSCP rule parameters.
	 */
	if (v6->flags & NSS_IPV6_CREATE_FLAG_DSCP_MARKING) {
		pd6rc->dscp_rule.flow_dscp = v6->flow_dscp;
		pd6rc->dscp_rule.return_dscp = v6->return_dscp;
		pd6rc->rule_flags |= PPE_DRV_V6_RULE_FLAG_DSCP_MARKING;
		pd6rc->valid_flags |= PPE_DRV_V6_VALID_FLAG_DSCP_MARKING;
	}

	/*
	 * If Ingress VLAN tag or pppoe rule is present, set the top interface.
	 */
	if (use_top_interface) {
		pd6rc->top_rule.rx_if = ppe_drv_iface_idx_get_by_dev(v6->top_ndev);
	} else {
		pd6rc->top_rule.rx_if = v6->src_interface_num;
	}

	pd6rc->top_rule.tx_if = v6->dest_interface_num;
	ppe_status = ppe_drv_v6_create(pd6rc);
	if (ppe_status != PPE_DRV_RET_SUCCESS) {
		nss_capwapmgr_warn("%px:PPE rule create failed\n", v6);
		goto fail;
	}

	/*
	 * Create PPE tunnel encap entry to handle UL traffic.
	 */
	status = nss_capwapmgr_tx_rule_create_v6(t, v6, false);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_ppe_destroy_ipv6_flow_rule(v6);
		goto fail;
	}

	/*
	 * Update the tunnel state.
	 */
	t->tunnel_state |= NSS_CAPWAPMGR_TUNNEL_STATE_IPRULE_CONFIGURED;
	goto done;

fail:
	status = NSS_CAPWAPMGR_FAILURE_IP_RULE;
done:
	kfree(pd6rc);
	return status;
}

/*
 * nss_capwapmgr_tx_msg_enable_tunnel()
 *	Common function to send CAPWAP tunnel enable msg
 */
static nss_tx_status_t nss_capwapmgr_tx_msg_enable_tunnel(struct nss_ctx_instance *ctx, struct net_device *dev, uint32_t if_num, uint32_t sibling_if_num)
{
	struct nss_capwap_msg capwapmsg;
	nss_tx_status_t status;

	/*
	 * Prepare the tunnel configuration parameter to send to NSS FW
	 */
	memset(&capwapmsg, 0, sizeof(struct nss_capwap_msg));
	capwapmsg.msg.enable_tunnel.sibling_if_num = sibling_if_num;

	/*
	 * Send CAPWAP data tunnel command to NSS
	 */
	nss_capwap_msg_init(&capwapmsg, if_num, NSS_CAPWAP_MSG_TYPE_ENABLE_TUNNEL, sizeof(struct nss_capwap_enable_tunnel_msg), nss_capwapmgr_msg_event_receive, dev);

	status = nss_capwapmgr_tx_msg_sync(ctx, dev, &capwapmsg);
	if (status != NSS_TX_SUCCESS) {
		nss_capwapmgr_warn("%px: ctx: CMD: %d Tunnel error : %d \n", ctx, NSS_CAPWAP_MSG_TYPE_ENABLE_TUNNEL, status);
	}

	return status;
}

/*
 * nss_capwapmgr_tunnel_action()
 *	Common function for CAPWAP tunnel operation messages without
 *	any message data structures.
 */
static nss_tx_status_t nss_capwapmgr_tunnel_action(struct nss_ctx_instance *ctx, struct net_device *dev, uint32_t if_num, nss_capwap_msg_type_t cmd)
{
	struct nss_capwap_msg capwapmsg;
	nss_tx_status_t status;

	/*
	 * Prepare the tunnel configuration parameter to send to NSS FW
	 */
	memset(&capwapmsg, 0, sizeof(struct nss_capwap_msg));

	/*
	 * Send CAPWAP data tunnel command to NSS
	 */
	nss_capwap_msg_init(&capwapmsg, if_num, cmd, 0, nss_capwapmgr_msg_event_receive, dev);

	status = nss_capwapmgr_tx_msg_sync(ctx, dev, &capwapmsg);
	if (status != NSS_TX_SUCCESS) {
		nss_capwapmgr_warn("%px: ctx: CMD: %d Tunnel error : %d \n", ctx, cmd, status);
	}

	return status;
}

/*
 * nss_capwapmgr_tx_msg_update_vp_num()
 *	Function to send update vp message.
 */
static nss_tx_status_t nss_capwapmgr_tx_msg_update_vp_num(struct net_device *dev, uint32_t if_num, int16_t vp_num)
{
	struct nss_capwap_msg capwapmsg;
	nss_tx_status_t status;
	struct nss_capwapmgr_priv *priv = netdev_priv(dev);
	struct nss_ctx_instance *ctx = priv->nss_ctx;

	/*
	 * Prepare the tunnel configuration parameter to send to NSS FW
	 */
	memset(&capwapmsg, 0, sizeof(struct nss_capwap_msg));
	capwapmsg.msg.update_vp_num.vp_num = vp_num;

	/*
	 * Send CAPWAP data tunnel command to NSS
	 */
	nss_capwap_msg_init(&capwapmsg, if_num, NSS_CAPWAP_MSG_TYPE_UPDATE_VP_NUM, sizeof(struct nss_capwap_update_vp_num_msg), nss_capwapmgr_msg_event_receive, dev);

	status = nss_capwapmgr_tx_msg_sync(ctx, dev, &capwapmsg);
	if (status != NSS_TX_SUCCESS) {
		nss_capwapmgr_warn("%px: ctx: CMD: %d Tunnel error : %d \n", ctx, NSS_CAPWAP_MSG_TYPE_UPDATE_VP_NUM, status);
	}

	return status;
}

/*
 * nss_capwapmgr_trustsec_rx_vp_unconfig
 *	Unconfigre trustsec rx vp.
 */
static bool nss_capwapmgr_trustsec_rx_vp_unconfig(void)
{
	struct nss_ctx_instance *ctx = nss_trustsec_rx_get_ctx();
	struct nss_trustsec_rx_msg rx_msg = {0};
	struct net_device *int_ndev = NULL;
	ppe_vp_num_t vp_num;
	ppe_vp_status_t vp_status;
	nss_tx_status_t nss_status;

	/*
	 * Get the vp num and internel netdev from the global structure.
	 */
	int_ndev = global.trustsec_rx_internal_ndev;
	vp_num = global.trustsec_rx_vp_num;

	/*
	 * Free the VP interface associated with the tunnel.
	 */
	if (vp_num) {
		vp_status = ppe_vp_free(vp_num);
		if (vp_status != PPE_VP_STATUS_SUCCESS) {
			nss_capwapmgr_warn("VP Number %d: Failed to free trustsec_rx vp\n",
				vp_num);
			return false;
		}

		global.trustsec_rx_vp_num = 0;

		/*
		 * Send vp unconfig message to trustsec rx node in nss fw.
		 */
		rx_msg.msg.uncfg.num = vp_num;

		nss_trustsec_rx_msg_init(&rx_msg,
				NSS_TRUSTSEC_RX_INTERFACE,
				NSS_TRUSTSEC_RX_MSG_UNCONFIG_VP,
				sizeof(struct nss_trustsec_rx_vp_msg),
				NULL,
				NULL);

		nss_status = nss_trustsec_rx_msg_sync(ctx, &rx_msg);
		if (nss_status != NSS_TX_SUCCESS) {
			nss_capwapmgr_warn("Failed to send unconfig trustsec rx VP %d  message to nss fw", vp_num);
			return false;
		}
	}

	/*
	 * Free the internal netdevice assocaited to trustsec rx vp.
	 */
	if (int_ndev) {
		free_netdev(int_ndev);
		global.trustsec_rx_internal_ndev = NULL;
	}

	return true;
}

/*
 * nss_capwapmgr_trustsec_rx_acl_unconfig
 *	Unconfigure trustec related rules and objects.
 */
static void nss_capwapmgr_trustsec_rx_acl_unconfig(void)
{
	uint32_t dev_id = NSS_CAPWAPMGR_DEV_ID;
	uint32_t list_id = NSS_CAPWAPMGR_ACL_TRUSTSEC_LIST_ID;
	uint32_t rule_id = NSS_CAPWAPMGR_ACL_TRUSTSEC_RULE_ID;
	uint8_t rule_nr = NSS_CAPWAPMGR_RULE_NR;

	/*
	 * Delete the acl rule.
	 */
	fal_acl_rule_delete(dev_id, list_id, rule_id, rule_nr);

	/*
	 * Delete the trustsec rx acl list.
	 */
	fal_acl_list_destroy(dev_id, list_id);
}

/*
 * nss_capwapmgr_trustsec_rx_vp_config
 *	This API configures trustsec rx vp if not configured.
 */
static bool nss_capwapmgr_trustsec_rx_vp_config(void)
{
	struct ppe_vp_ai vpai = {0};
	struct nss_ctx_instance *ctx = nss_trustsec_rx_get_ctx();
	struct nss_trustsec_rx_msg rx_msg = {0};
	struct net_device *int_ndev = NULL;
	ppe_vp_num_t vp_num;
	nss_tx_status_t nss_status;

	spin_lock(&nss_capwapmgr_acl_spinlock);
	if (global.trustsec_rx_vp_configured) {
		spin_unlock(&nss_capwapmgr_acl_spinlock);
		nss_capwapmgr_info("Trusec rx vp already configured\n");
		return true;
	}

	if (global.trustsec_rx_vp_config_in_progress) {
		spin_unlock(&nss_capwapmgr_acl_spinlock);
		nss_capwapmgr_info("Trusec rx vp config in progress\n");
		return false;
	}

	global.trustsec_rx_vp_config_in_progress = true;
	spin_unlock(&nss_capwapmgr_acl_spinlock);

	int_ndev = alloc_netdev(0, "trustsecint",
				NET_NAME_ENUM, nss_capwapmgr_dummy_netdev_setup);
	if (!int_ndev) {
		nss_capwapmgr_warn("Error allocating internal netdev for trustsec_rx\n");
		return false;
	}

	vpai.type = PPE_VP_TYPE_SW_PO;
	vpai.queue_num = edma_cfg_rx_point_offload_ring_queue_get();
	vp_num = ppe_vp_alloc(int_ndev, &vpai);
	if (vp_num == -1) {
		nss_capwapmgr_warn("Trustsec rx  VP alloc failed\n");
		goto fail1;
	}

	rx_msg.msg.cfg.num = vp_num;

	nss_trustsec_rx_msg_init(&rx_msg,
			NSS_TRUSTSEC_RX_INTERFACE,
			NSS_TRUSTSEC_RX_MSG_CONFIG_VP,
			sizeof(struct nss_trustsec_rx_vp_msg),
			NULL,
			NULL);

	nss_status = nss_trustsec_rx_msg_sync(ctx, &rx_msg);
	if (nss_status != NSS_TX_SUCCESS) {
		nss_capwapmgr_warn("Failed to send config vp message to fw\n");
		goto fail2;
	}

	global.trustsec_rx_internal_ndev = int_ndev;
	global.trustsec_rx_vp_num = vp_num;

	spin_lock(&nss_capwapmgr_acl_spinlock);
	global.trustsec_rx_vp_configured = true;
	global.trustsec_rx_vp_config_in_progress = false;
	spin_unlock(&nss_capwapmgr_acl_spinlock);
	return true;
fail2:
	ppe_vp_free(vp_num);
fail1:
	free_netdev(int_ndev);
	spin_lock(&nss_capwapmgr_acl_spinlock);
	global.trustsec_rx_vp_config_in_progress = false;
	spin_unlock(&nss_capwapmgr_acl_spinlock);

	return false;
}

/*
 * nss_capwapmgr_trustsec_rx_acl_config
 *	Configuration required to handle trustsec traffic.
 */
static bool nss_capwapmgr_trustsec_rx_acl_config(void)
{
	sw_error_t sw_err;
	fal_acl_rule_t acl_rule = {0};
	uint32_t v_port;
	uint32_t dev_id = NSS_CAPWAPMGR_DEV_ID;
	uint32_t list_id = NSS_CAPWAPMGR_ACL_TRUSTSEC_LIST_ID;
	uint32_t rule_id = NSS_CAPWAPMGR_ACL_TRUSTSEC_RULE_ID;
	uint8_t rule_nr = NSS_CAPWAPMGR_RULE_NR;
	uint32_t vp_num;

	if (!nss_capwapmgr_trustsec_rx_vp_config()) {
		nss_capwapmgr_warn("Failed to configure trustsec_rx vp");
		return false;
	}

	vp_num = global.trustsec_rx_vp_num;
	/*
	 * Create the acl list to handle trustsec_traffic.
	 */
	sw_err = fal_acl_list_creat(NSS_CAPWAPMGR_DEV_ID, list_id, NSS_CAPWAPMGR_ACL_TRUSTSEC_LIST_PRIO);
	if (sw_err != SW_OK) {
		nss_capwapmgr_warn("Failed to create ACL list err:%d\n", sw_err);
		return false;
	}

	/*
	 * Valid flag is set to specify that the acl rule will match a field
	 * in the L2 header.
	 */
	FAL_FIELD_FLG_SET(acl_rule.field_flg, FAL_ACL_FIELD_MAC_ETHTYPE);

	/*
	 * Update the ethertype to match trustsec ether type(0x8909).
	 */
	acl_rule.ethtype_val = NSS_CAPWAPMGR_ETH_TYPE_TRUSTSEC;
	acl_rule.ethtype_mask = NSS_CAPWAPMGR_ETH_TYPE_MASK;

	/*
	 * Set action flag to forward the matched packets to trustsec VP.
	 */
	FAL_ACTION_FLG_SET(acl_rule.action_flg, FAL_ACL_ACTION_PERMIT);
	FAL_ACTION_FLG_SET(acl_rule.action_flg, FAL_ACL_ACTION_REDPT);
	v_port = FAL_PORT_ID(FAL_PORT_TYPE_VPORT, vp_num);
	acl_rule.ports = v_port;

	sw_err = fal_acl_rule_query(dev_id, list_id, rule_id, &acl_rule);
	if (sw_err != SW_NOT_FOUND) {
		nss_capwapmgr_warn("ACL trustsec rule already exist for list_id = %u, rule_id %u - code: %d\n",
				list_id, rule_id, sw_err);
		goto fail;
	}

	sw_err = fal_acl_rule_add(dev_id, list_id, rule_id, rule_nr, &acl_rule);
	if (sw_err) {
		nss_capwapmgr_warn("Failed to add ACL trustsec rule: %d - code: %d\n", rule_id, sw_err);
		goto fail;
	}

	return true;

fail:
	fal_acl_list_destroy(dev_id, list_id);
	return false;
}

/*
 * nss_capwapmgr_dscp_acl_deinit.
 *	Delete ACL realted tables and objects.
 */
static void nss_capwapmgr_dscp_acl_deinit(void)
{
	int i;

	for (i = 0; i < NSS_CAPWAPMGR_ACL_DSCP_LIST_CNT; i++) {
		int list_id = NSS_CAPWAPMGR_ACL_DSCP_LIST_ID + i;
		fal_acl_list_destroy(NSS_CAPWAPMGR_DEV_ID, list_id);
	}
}

/*
 * nss_capwapmgr_dscp_acl_init()
 *	Initializes ACL related tables and objects.
 */
static bool nss_capwapmgr_dscp_acl_init(void)
{
	sw_error_t rv;
	int i, j, uid = 0;

	if (!nss_capwapmgr_trustsec_rx_vp_config()) {
		nss_capwapmgr_warn("Failed to configure trustsec_rx vp");
		return false;
	}

	/*
	 * Create the ACL list we will be using for dscp prioritization.
	 */
	for (i = 0; i < NSS_CAPWAPMGR_ACL_DSCP_LIST_CNT; i++) {
		int list_id = NSS_CAPWAPMGR_ACL_DSCP_LIST_ID + i;
		rv = fal_acl_list_creat(NSS_CAPWAPMGR_DEV_ID, list_id, NSS_CAPWAPMGR_ACL_DSCP_LIST_PRIO);
		if (rv != SW_OK) {
			nss_capwapmgr_warn("Failed to create ACL list err:%d\n", rv);
			return false;
		}
	}

	/*
	 * Initialize the global ACL table.
	 */
	for (i = 0; i < NSS_CAPWAPMGR_ACL_DSCP_LIST_CNT; i++) {
		for (j = 0; j < NSS_CAPWAPMGR_ACL_DSCP_RULES_PER_LIST; j++) {
			global.acl_list[i].rule[j].uid = uid++;
			global.acl_list[i].rule[j].rule_id = j;
			global.acl_list[i].rule[j].list_id = i;
		}
	}

	return true;
}

/*
 * nss_capwapmgr_tx_trustsec_config_msg_v6
 *	Send configure message to trustsec rx node in FW.
 */
static nss_capwapmgr_status_t nss_capwapmgr_tx_trustsec_config_msg_v6(struct nss_ipv6_create *v6, uint32_t if_num)
{
	struct nss_ctx_instance *ctx = nss_trustsec_rx_get_ctx();
	struct nss_trustsec_rx_msg trustsec_rx_msg = {0};
	struct nss_trustsec_rx_configure_msg *trustsec_cfg_msg;
	nss_tx_status_t nss_status;
	nss_capwapmgr_status_t status = NSS_CAPWAPMGR_SUCCESS;

	trustsec_cfg_msg = &trustsec_rx_msg.msg.configure;

	/*
	 * Extract the 5 tuple information.
	 */
	trustsec_cfg_msg->ip_version = NSS_TRUSTSEC_RX_FLAG_IPV6;

	/*
	 * Copy the source ipv6 address and port.
	 */
	memcpy(trustsec_cfg_msg->src_ip.ip.ipv6, v6->src_ip, sizeof(uint32_t) * 4);
	trustsec_cfg_msg->src_port = v6->src_port;

	/*
	 * Copy the source ipv6 address and port.
	 */
	memcpy(trustsec_cfg_msg->dest_ip.ip.ipv6, v6->dest_ip, sizeof(uint32_t) * 4);
	trustsec_cfg_msg->dest_port = v6->dest_port;

	/*
	 * Destination interface will be the capwap_outer node.
	 */
	trustsec_cfg_msg->dest = if_num;

	nss_trustsec_rx_msg_init(&trustsec_rx_msg, NSS_TRUSTSEC_RX_INTERFACE, NSS_TRUSTSEC_RX_MSG_CONFIGURE,
		sizeof( struct nss_trustsec_rx_configure_msg), NULL, NULL);

	nss_status = nss_trustsec_rx_msg_sync(ctx, &trustsec_rx_msg);
	if (nss_status != NSS_TX_SUCCESS) {
		status = NSS_CAPWAPMGR_FAILURE_CONFIG_TRUSTSEC_RX;
	}

	return status;
}

/*
 * nss_capwapmgr_tx_trustsec_config_msg_v4
 *	Send configure message to trustsec rx node in FW.
 */
static nss_capwapmgr_status_t nss_capwapmgr_tx_trustsec_config_msg_v4(struct nss_ipv4_create *v4, uint32_t if_num)
{
	struct nss_ctx_instance *ctx = nss_trustsec_rx_get_ctx();
	struct nss_trustsec_rx_msg trustsec_rx_msg = {0};
	struct nss_trustsec_rx_configure_msg *trustsec_cfg_msg;
	nss_tx_status_t nss_status;
	nss_capwapmgr_status_t status = NSS_CAPWAPMGR_SUCCESS;

	trustsec_cfg_msg = &trustsec_rx_msg.msg.configure;
	/*
	 * Extract the 5 tuple information.
	 */
	trustsec_cfg_msg->src_ip.ip.ipv4 = v4->src_ip;
	trustsec_cfg_msg->src_port = v4->src_port;
	trustsec_cfg_msg->dest_ip.ip.ipv4 = v4->dest_ip;
	trustsec_cfg_msg->dest_port = v4->dest_port;
	trustsec_cfg_msg->ip_version = NSS_TRUSTSEC_RX_FLAG_IPV4;

	/*
	 * Destination interface will be the capwap_outer node.
	 */
	trustsec_cfg_msg->dest = if_num;

	nss_trustsec_rx_msg_init(&trustsec_rx_msg, NSS_TRUSTSEC_RX_INTERFACE, NSS_TRUSTSEC_RX_MSG_CONFIGURE,
		sizeof( struct nss_trustsec_rx_configure_msg), NULL, NULL);

	nss_status = nss_trustsec_rx_msg_sync(ctx, &trustsec_rx_msg);
	if (nss_status != NSS_TX_SUCCESS) {
		status = NSS_CAPWAPMGR_FAILURE_CONFIG_TRUSTSEC_RX;
	}

	return status;
}

/*
 * nss_capwapmgr_tx_trustsec_unconfig_msg_v6
 *	Send unconfigure message to trustsec_rx node in FW.
 */
static nss_capwapmgr_status_t nss_capwapmgr_tx_trustsec_unconfig_msg_v6(struct nss_ipv6_create *v6, uint32_t if_num)
{
	struct nss_ctx_instance *ctx = nss_trustsec_rx_get_ctx();
	struct nss_trustsec_rx_msg trustsec_rx_msg = {0};
	struct nss_trustsec_rx_unconfigure_msg *trustsec_uncfg_msg;
	nss_tx_status_t nss_status;
	nss_capwapmgr_status_t status = NSS_CAPWAPMGR_SUCCESS;

	trustsec_uncfg_msg = &trustsec_rx_msg.msg.unconfigure;
	trustsec_uncfg_msg->ip_version = NSS_TRUSTSEC_RX_FLAG_IPV6;

	/*
	 * Copy the source ipv6 address and port.
	 */
	memcpy(trustsec_uncfg_msg->src_ip.ip.ipv6, v6->src_ip, sizeof(uint32_t) * 4);
	trustsec_uncfg_msg->src_port = v6->src_port;

	/*
	 * Copy the source ipv6 address and port.
	 */
	memcpy(trustsec_uncfg_msg->dest_ip.ip.ipv6, v6->dest_ip, sizeof(uint32_t) * 4);
	trustsec_uncfg_msg->dest_port = v6->dest_port;

	/*
	 * Destination interface will be the capwap_outer node.
	 */
	trustsec_uncfg_msg->dest = if_num;

	nss_trustsec_rx_msg_init(&trustsec_rx_msg, NSS_TRUSTSEC_RX_INTERFACE, NSS_TRUSTSEC_RX_MSG_UNCONFIGURE,
		sizeof( struct nss_trustsec_rx_unconfigure_msg), NULL, NULL);

	nss_status = nss_trustsec_rx_msg_sync(ctx, &trustsec_rx_msg);
	if (nss_status != NSS_TX_SUCCESS) {
		status = NSS_CAPWAPMGR_FAILURE_CONFIG_TRUSTSEC_RX;
	}

	return status;
}
/*
 * nss_capwapmgr_tx_trustsec_unconfig_msg_v4
 *	Send unconfigure message to trustsec_rx node in FW.
 */
static nss_capwapmgr_status_t nss_capwapmgr_tx_trustsec_unconfig_msg_v4(struct nss_ipv4_create *v4, uint32_t if_num)
{
	struct nss_ctx_instance *ctx = nss_trustsec_rx_get_ctx();
	struct nss_trustsec_rx_msg trustsec_rx_msg = {0};
	struct nss_trustsec_rx_unconfigure_msg *trustsec_uncfg_msg;
	nss_tx_status_t nss_status;
	nss_capwapmgr_status_t status = NSS_CAPWAPMGR_SUCCESS;

	trustsec_uncfg_msg = &trustsec_rx_msg.msg.unconfigure;

	/*
	 * Extract the 5 tuple information.
	 */
	trustsec_uncfg_msg->src_ip.ip.ipv4 = v4->src_ip;
	trustsec_uncfg_msg->src_port = v4->src_port;
	trustsec_uncfg_msg->dest_ip.ip.ipv4 = v4->dest_ip;
	trustsec_uncfg_msg->dest_port = v4->dest_port;
	trustsec_uncfg_msg->ip_version = NSS_TRUSTSEC_RX_FLAG_IPV4;

	/*
	 * Destination interface will be the capwap_outer node.
	 */
	trustsec_uncfg_msg->dest = if_num;

	nss_trustsec_rx_msg_init(&trustsec_rx_msg, NSS_TRUSTSEC_RX_INTERFACE, NSS_TRUSTSEC_RX_MSG_UNCONFIGURE,
		sizeof( struct nss_trustsec_rx_unconfigure_msg), NULL, NULL);

	nss_status = nss_trustsec_rx_msg_sync(ctx, &trustsec_rx_msg);
	if (nss_status != NSS_TX_SUCCESS) {
		status = NSS_CAPWAPMGR_FAILURE_CONFIG_TRUSTSEC_RX;
	}

	return status;
}

/*
 * nss_capwapmgr_trustsec_rx_acl_rule_unbind
 *	Function to unbid the ACL rule handling Ingress traffic (AC->AP)
 */
static nss_capwapmgr_status_t nss_capwapmgr_trustsec_rx_acl_rule_unbind(int32_t port_num)
{
	nss_capwapmgr_status_t status = NSS_CAPWAPMGR_SUCCESS;
	sw_error_t sw_err;
	uint32_t dev_id = NSS_CAPWAPMGR_DEV_ID;
	uint32_t list_id = NSS_CAPWAPMGR_ACL_TRUSTSEC_LIST_ID;
	atomic_t *tunnel_count = &global.trustsec_tunnel_count[port_num - 1];
	atomic_t *acl_req_count = &global.trustsec_acl_rule_create_req;

	/*
	 * Unbind the acl rule if its the last tunnel to be assocaited to the specific port.
	 */
	if (atomic_dec_and_test(tunnel_count)) {
		sw_err = fal_acl_list_unbind(dev_id, list_id, FAL_ACL_DIREC_IN, FAL_ACL_BIND_PORT, port_num);
		if (sw_err != SW_OK) {
			nss_capwapmgr_warn("Failed to unbind trustsec ACL list:%d to port %d - code: %d\n", list_id,
					port_num, sw_err);
			atomic_inc(tunnel_count);
			status = NSS_CAPWAPMGR_FAILURE_UNBIND_ACL_LIST;
		}
	}

	/*
	 * Unconfigure the trustsec acl objects if this is the last tunnel using it.
	 */
	if (atomic_dec_and_test(acl_req_count)) {
		nss_capwapmgr_trustsec_rx_acl_unconfig();
	}

	return status;
}

/*
 * nss_capwapmgr_trustsec_rx_acl_rule_bind
 *	Function to bind the ACL rule to handle Ingress traffic (AC->AP)
 */
static nss_capwapmgr_status_t nss_capwapmgr_trustsec_rx_acl_rule_bind(int32_t port_num)
{
	nss_capwapmgr_status_t status = NSS_CAPWAPMGR_SUCCESS;
	sw_error_t sw_err;
	uint32_t dev_id = NSS_CAPWAPMGR_DEV_ID;
	uint32_t list_id = NSS_CAPWAPMGR_ACL_TRUSTSEC_LIST_ID;
	atomic_t *tunnel_count = &global.trustsec_tunnel_count[port_num - 1];
	atomic_t *acl_req_count = &global.trustsec_acl_rule_create_req;

	/*
	 * Configure the trustsec rx vp and acl related objects when this api
	 * is called for the first time.
	 */
	if (atomic_inc_return(acl_req_count) == 1) {
		nss_capwapmgr_trustsec_rx_acl_config();
	}

	/*
	 * Bind the acl rule for the first tunnel created on a specific port.
	 */
	if (atomic_inc_return(tunnel_count) == 1) {
		sw_err = fal_acl_list_bind(dev_id, list_id, FAL_ACL_DIREC_IN, FAL_ACL_BIND_PORT, port_num);
		if (sw_err != SW_OK) {
			nss_capwapmgr_warn("Failed to bind trustsec ACL rule:%d to port %d - code: %d\n", list_id,
					port_num, sw_err);
			atomic_dec(tunnel_count);
			status = NSS_CAPWAPMGR_FAILURE_BIND_ACL_LIST;

			/*
			 * Unconfigure trustsec rx acl objects if
			 * this is the last tunnel using it.
			 */
			if (atomic_dec_and_test(acl_req_count)) {
				nss_capwapmgr_trustsec_rx_acl_unconfig();
			}
		}
	}

	return status;
}




/*
 * nss_capwapmgr_trustsec_rule_destroy_v6
 *	Function to destroy FW and PPE rules for ipv6 trustsec tunnels.
 */
static nss_capwapmgr_status_t nss_capwapmgr_trustsec_rule_destroy_v6(struct nss_capwapmgr_tunnel *t)
{
	struct ppe_drv_iface *iface;
	int32_t port_num;
	nss_capwapmgr_status_t status = NSS_CAPWAPMGR_SUCCESS;
	struct nss_ipv6_create *v6 = &t->ip_rule.v6;
	uint32_t if_num = t->if_num_outer;

	/*
	 * Unconfig trustsec_rx node in NSS-FW.
	 */
	status = nss_capwapmgr_tx_trustsec_unconfig_msg_v6(v6, if_num);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%px: trustsec unconfigure error %d\n", v6, status);
		goto done;
	}

	/*
	 * Unbind the acl rule.
	 */
	iface = ppe_drv_iface_get_by_idx(v6->src_interface_num);
	port_num = ppe_drv_iface_port_idx_get(iface);
	status = nss_capwapmgr_trustsec_rx_acl_rule_unbind(port_num);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%px: trustsec acl rule error %d\n", v6, status);
		goto done;
	}

	/*
	 * Delete PPE tunnel rule.
	 */
	status = nss_capwapmgr_tx_rule_destroy(t, true);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%px: trustsec tunnel rule error %d\n", v6, status);
		goto done;
	}

	t->tunnel_state &= ~NSS_CAPWAPMGR_TUNNEL_STATE_IPRULE_CONFIGURED;
done:
	return status;
}

/*
 * nss_capwapmgr_trustsec_rule_create_v6
 *	Function to create FW and PPE rules to handle ipv6 trustsec tunnels.
 */
static nss_capwapmgr_status_t nss_capwapmgr_trustsec_rule_create_v6(struct nss_capwapmgr_tunnel *t, struct nss_ipv6_create *v6, uint32_t if_num)
{
	struct ppe_drv_iface *iface;
	int32_t port_num;
	nss_capwapmgr_status_t status = NSS_CAPWAPMGR_SUCCESS;

	/*
	 * Send 5 tuple info to trustsec_rx node in NSS-FW.
	 */
	status = nss_capwapmgr_tx_trustsec_config_msg_v6(v6, if_num);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%px: trustsec configure error %d\n", v6, status);
		goto done;
	}

	/*
	 * Create PPE ACL rule to handle ingress trustsec packets.
	 */
	iface = ppe_drv_iface_get_by_idx(v6->src_interface_num);
	port_num = ppe_drv_iface_port_idx_get(iface);
	status = nss_capwapmgr_trustsec_rx_acl_rule_bind(port_num);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%px: trustsec acl rule error %d\n", v6, status);
		goto fail;
	}

	/*
	 * Create PPE tunnel rule to handle egress trustsec packets.
	 */
	status = nss_capwapmgr_tx_rule_create_v6(t, v6, true);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%px: trustsec tunnel rule error %d\n", v6, status);
		goto fail1;
	}

	t->tunnel_state |= NSS_CAPWAPMGR_TUNNEL_STATE_IPRULE_CONFIGURED;
	goto done;
fail1:
	nss_capwapmgr_trustsec_rx_acl_rule_unbind(port_num);
fail:
	nss_capwapmgr_tx_trustsec_unconfig_msg_v6(v6, if_num);
done:
	return status;

	return NSS_CAPWAPMGR_SUCCESS;
}

/*
 * nss_capwapmgr_trustsec_rule_destroy_v4
 *	Function to destroy FW and PPE rules for ipv4 trustsec tunnels.
 */
static nss_capwapmgr_status_t nss_capwapmgr_trustsec_rule_destroy_v4(struct nss_capwapmgr_tunnel *t)
{
	struct ppe_drv_iface *iface;
	int32_t port_num;
	nss_capwapmgr_status_t status = NSS_CAPWAPMGR_SUCCESS;
	struct nss_ipv4_create *v4 = &t->ip_rule.v4;
	uint32_t if_num = t->if_num_outer;

	/*
	 * Unconfig trustsec_rx node in NSS-FW.
	 */
	status = nss_capwapmgr_tx_trustsec_unconfig_msg_v4(v4, if_num);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%px: trustsec unconfigure error %d\n", v4, status);
		goto done;
	}

	/*
	 * Unbind the acl rule.
	 */
	iface = ppe_drv_iface_get_by_idx(v4->src_interface_num);
	port_num = ppe_drv_iface_port_idx_get(iface);
	status = nss_capwapmgr_trustsec_rx_acl_rule_unbind(port_num);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%px: trustsec acl rule error %d\n", v4, status);
		goto done;
	}

	/*
	 * Delete PPE tunnel rule.
	 */
	status = nss_capwapmgr_tx_rule_destroy(t, true);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%px: trustsec tunnel rule error %d\n", v4, status);
		goto done;
	}

	t->tunnel_state &= ~NSS_CAPWAPMGR_TUNNEL_STATE_IPRULE_CONFIGURED;
done:
	return status;
}

/*
 * nss_capwapmgr_trustsec_rule_create_v4
 *	Function to create FW and PPE rules to handle ipv4 trustsec tunnels.
 */
static nss_capwapmgr_status_t nss_capwapmgr_trustsec_rule_create_v4(struct nss_capwapmgr_tunnel *t, struct nss_ipv4_create *v4, uint32_t if_num)
{
	struct ppe_drv_iface *iface;
	int32_t port_num;
	nss_capwapmgr_status_t status = NSS_CAPWAPMGR_SUCCESS;

	/*
	 * Send 5 tuple info to trustsec_rx node in NSS-FW.
	 */
	status = nss_capwapmgr_tx_trustsec_config_msg_v4(v4, if_num);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%px: trustsec configure error %d\n", v4, status);
		goto done;
	}

	/*
	 * Create PPE ACL rule to handle ingress trustsec packets.
	 */
	iface = ppe_drv_iface_get_by_idx(v4->src_interface_num);
	port_num = ppe_drv_iface_port_idx_get(iface);
	status = nss_capwapmgr_trustsec_rx_acl_rule_bind(port_num);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%px: trustsec acl rule error %d\n", v4, status);
		goto fail;
	}

	/*
	 * Create PPE tunnel rule to handle egress trustsec packets.
	 */
	status = nss_capwapmgr_tx_rule_create_v4(t, v4, true);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%px: trustsec tunnel rule error %d\n", v4, status);
		goto fail1;
	}

	t->tunnel_state |= NSS_CAPWAPMGR_TUNNEL_STATE_IPRULE_CONFIGURED;
	goto done;
fail1:
	nss_capwapmgr_trustsec_rx_acl_rule_unbind(port_num);
fail:
	nss_capwapmgr_tx_trustsec_unconfig_msg_v4(v4, if_num);
done:
	return status;
}

/*
 * nss_capwapmgr_tunnel_create_common()
 *	Common handling for creating IPv4 or IPv6 tunnel
 */
static nss_capwapmgr_status_t nss_capwapmgr_tunnel_create_common(struct net_device *dev, uint8_t tunnel_id,
	struct nss_ipv4_create *v4, struct nss_ipv6_create *v6, struct nss_capwap_rule_msg *capwap_rule, struct nss_dtlsmgr_config *dtls_data)
{
	bool dtls_enabled = !!(capwap_rule->enabled_features & NSS_CAPWAPMGR_FEATURE_DTLS_ENABLED);
	int32_t capwap_if_num_inner, capwap_if_num_outer, forwarding_ifnum;
	nss_capwapmgr_status_t status = NSS_CAPWAPMGR_SUCCESS;
	nss_tx_status_t nss_status = NSS_TX_SUCCESS;
	struct net_device *internal_dev_decap = NULL;
	struct net_device *internal_dev_encap = NULL;
	struct nss_capwapmgr_tunnel *t = NULL;
	struct nss_capwapmgr_priv *priv;
	uint16_t type_flags = 0;
	struct ppe_vp_ai vpai;
	ppe_vp_num_t vp_num_decap, vp_num_encap;
	uint32_t outer_trustsec_enabled = capwap_rule->enabled_features & NSS_CAPWAPMGR_FEATURE_OUTER_TRUSTSEC_ENABLED;

	if (!v4 && !v6) {
		nss_capwapmgr_warn("%px: invalid ip create rule for tunnel: %d\n", dev, tunnel_id);
		return NSS_CAPWAPMGR_INVALID_IP_RULE;
	}

	if (!(capwap_rule->l3_proto == NSS_CAPWAP_TUNNEL_IPV4 ||
		capwap_rule->l3_proto == NSS_CAPWAP_TUNNEL_IPV6)) {
		nss_capwapmgr_warn("%px: tunnel %d: wrong argument for l3_proto\n", dev, tunnel_id);
		return NSS_CAPWAPMGR_FAILURE_INVALID_L3_PROTO;
	}

	if (!(capwap_rule->which_udp == NSS_CAPWAP_TUNNEL_UDP ||
		capwap_rule->which_udp == NSS_CAPWAP_TUNNEL_UDPLite)) {
		nss_capwapmgr_warn("%px: tunnel %d: wrong argument for which_udp\n", dev, tunnel_id);
		return NSS_CAPWAPMGR_FAILURE_INVALID_UDP_PROTO;
	}

	if (dtls_enabled && !dtls_data) {
		nss_capwapmgr_warn("%px: need to supply dtls_data if DTLS is enabled\n", dev);
		return NSS_CAPWAPMGR_FAILURE_BAD_PARAM;
	}

	/*
	 * Allocate a two internal net_devices for every tunnel.
	 * One to handle UL traffice.
	 * The other to handle the DL traffic.
	 *
	 * We use PPE's tunnel encap entries to write the L2 header in UL direction.
	 * In the DL direction we use PPE's flow entries to forward the traffic from the
	 * ingress WAN port to CAPWAP VP whose queues are mapped to NSS-FW.
	 */
	internal_dev_decap = alloc_netdev(0,"capwapintd%d",
				NET_NAME_UNKNOWN, nss_capwapmgr_dummy_netdev_setup);
	if (!internal_dev_decap) {
		nss_capwapmgr_warn("Error allocating internal decap netdev\n");
		return NSS_CAPWAPMGR_FAILRUE_INTERNAL_DECAP_NETDEV_ALLOC_FAILED;
	}

	internal_dev_encap = alloc_netdev(0,"capwapinte%d",
				NET_NAME_UNKNOWN, nss_capwapmgr_dummy_netdev_setup);
	if (!internal_dev_decap) {
		nss_capwapmgr_warn("Error allocating internal encap netdev\n");
		return NSS_CAPWAPMGR_FAILRUE_INTERNAL_ENCAP_NETDEV_ALLOC_FAILED;
	}

	memset(&vpai, 0, sizeof(struct ppe_vp_ai));
	internal_dev_decap->mtu = NSS_CAPWAPMGR_VP_MTU;
	internal_dev_encap->mtu = NSS_CAPWAPMGR_VP_MTU;

	vpai.type = PPE_VP_TYPE_SW_PO;

	if (!outer_trustsec_enabled) {
		vpai.queue_num = edma_cfg_rx_point_offload_ring_queue_get();

		/*
		 * Enable ppe to host mode.
		 * This mode is not supported for trustsec enabled tunnels.
		 */
		if (global.ppe2host) {
			vpai.src_cb = &nss_capwapmgr_receive_pkt_ppe_vp;
			vpai.src_cb_data = (void*)dev;
			capwap_rule->enabled_features |= NSS_CAPWAPMGR_FEATURE_PPE_TO_HOST_ENABLED;
		}

		/*
		 * Allocate a PPE-VP to handle DL traffic.
		*/
		vp_num_decap = ppe_vp_alloc(internal_dev_decap, &vpai);
		if (vp_num_decap == -1) {
			nss_capwapmgr_warn("%px: Decap VP alloc failed", dev);
			free_netdev(internal_dev_decap);
			return NSS_CAPWAPMGR_FAILURE_DECAP_VP_ALLOC;
		}
	}

	/*
	 * Reset the queue number for the UL VP.
	 */
	vpai.queue_num = 0;
	vp_num_encap = ppe_vp_alloc(internal_dev_encap, &vpai);
	if (vp_num_encap == -1) {
		nss_capwapmgr_warn("%px: Encap VP alloc failed", dev);
		free_netdev(internal_dev_encap);
		status = NSS_CAPWAPMGR_FAILURE_ENCAP_VP_ALLOC;
		goto fail;
	}

	dev_hold(dev);

	/*
	 * We create tunnel only when the tunnel does not exist.
	 */
	status = nss_capwapmgr_get_tunnel(dev, tunnel_id, &t);
	if (status != NSS_CAPWAPMGR_FAILURE_TUNNEL_DOES_NOT_EXIST) {
		nss_capwapmgr_warn("%px: tunnel: %d get error: %d\n", dev, tunnel_id, status);
		goto fail0;
	}

	capwap_if_num_inner = nss_dynamic_interface_alloc_node(NSS_DYNAMIC_INTERFACE_TYPE_CAPWAP_HOST_INNER);
	if (capwap_if_num_inner < 0) {
		nss_capwapmgr_warn("%px: di returned error : %d\n", dev, capwap_if_num_inner);
		status = NSS_CAPWAPMGR_FAILURE_DI_ALLOC_FAILED;
		goto fail0;
	}
	t->tunnel_state |= NSS_CAPWAPMGR_TUNNEL_STATE_INNER_ALLOCATED;

	if (nss_capwapmgr_register_with_nss(capwap_if_num_inner, dev) != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%d: NSS CAPWAP register with NSS failed", capwap_if_num_inner);
		status = NSS_CAPWAPMGR_FAILURE_REGISTER_NSS;
		goto fail1;
	}

	capwap_if_num_outer = nss_dynamic_interface_alloc_node(NSS_DYNAMIC_INTERFACE_TYPE_CAPWAP_OUTER);
	if (capwap_if_num_outer < 0) {
		nss_capwapmgr_warn("%px: di returned error : %d\n", dev, capwap_if_num_outer);
		status = NSS_CAPWAPMGR_FAILURE_DI_ALLOC_FAILED;
		goto fail2;
	}
	t->tunnel_state |= NSS_CAPWAPMGR_TUNNEL_STATE_OUTER_ALLOCATED;

	if (nss_capwapmgr_register_with_nss(capwap_if_num_outer, dev) != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%d: NSS CAPWAP register with NSS failed", capwap_if_num_outer);
		status = NSS_CAPWAPMGR_FAILURE_REGISTER_NSS;
		goto fail3;
	}

	if (!outer_trustsec_enabled) {
		if (nss_capwapmgr_tx_msg_update_vp_num(dev, capwap_if_num_outer, vp_num_decap) != NSS_TX_SUCCESS) {
			nss_capwapmgr_warn("%px: %d VP number update failed %d", dev, vp_num_decap, status);
			status = NSS_CAPWAPMGR_FAILURE_UPDATE_VP_NUM;
			goto fail4;
		}

		if (nss_capwapmgr_tx_msg_update_vp_num(dev, capwap_if_num_inner, vp_num_encap) != NSS_TX_SUCCESS) {
			nss_capwapmgr_warn("%px: %d VP number update failed %d", dev, vp_num_encap, status);
			status = NSS_CAPWAPMGR_FAILURE_UPDATE_VP_NUM;
			goto fail4;
		}
	} else {
		nss_capwapmgr_info("%px: configure TrustsecTx with sgt value: %x\n", dev, capwap_rule->outer_sgt_value);
		nss_status = nss_trustsec_tx_configure_sgt(capwap_if_num_outer, vp_num_encap, capwap_rule->outer_sgt_value);
		if (nss_status != NSS_TX_SUCCESS) {
			nss_capwapmgr_warn("%px: configure trustsectx node failed\n", dev);
			status = NSS_CAPWAPMGR_FAILURE_CONFIGURE_TRUSTSEC_TX;
			goto fail4;
		}
	}

	capwap_rule->mtu_adjust = 0;
	capwap_rule->dtls_inner_if_num = 0;
	forwarding_ifnum = ppe_drv_iface_idx_get_by_dev(internal_dev_decap);

	if (dtls_enabled) {
		/*
		 * We only support the METADATA mode for pure DTLS tunnels; in CAPWAP-DTLS
		 * the offload will not send the packets starting with Metadata. We need to
		 * ensure that the user does not configure this mode accidentally.
		 */
		dtls_data->flags &= ~NSS_DTLSMGR_ENCAP_METADATA;
		dtls_data->decap.nexthop_ifnum = nss_capwap_ifnum_with_core_id(capwap_if_num_outer);

		dtls_data->vp_num_encap = vp_num_encap;
		t->dtls_dev = nss_dtlsmgr_session_create(dtls_data);
		if (!t->dtls_dev) {
			nss_capwapmgr_warn("%px: NSS DTLS node alloc failed\n", dev);
			status = NSS_CAPWAPMGR_FAILURE_DI_ALLOC_FAILED;
			goto fail4;
		}

		capwap_rule->dtls_inner_if_num = nss_dtlsmgr_get_interface(t->dtls_dev, NSS_DTLSMGR_INTERFACE_TYPE_INNER);
		forwarding_ifnum = ppe_drv_iface_idx_get_by_dev(t->dtls_dev);
		capwap_rule->mtu_adjust = nss_dtlsmgr_encap_overhead(t->dtls_dev);
	}

	/*
	 * We use type_flags to determine the correct header sizes
	 * for a frame when encaping. CAPWAP processing node in the
	 * NSS FW does not know anything about IP rule information.
	 */
	if (v4) {
		if ((v4->out_vlan_tag[0] & 0xFFF) != 0xFFF) {
			type_flags |= NSS_CAPWAP_RULE_CREATE_VLAN_CONFIGURED;
		}

		if (v4->flow_pppoe_if_exist) {
			type_flags |= NSS_CAPWAP_RULE_CREATE_PPPOE_CONFIGURED;
		}
	} else {
		if ((v6->out_vlan_tag[0] & 0xFFF) != 0xFFF) {
			type_flags |= NSS_CAPWAP_RULE_CREATE_VLAN_CONFIGURED;
		}

		if (v6->flow_pppoe_if_exist) {
			type_flags |= NSS_CAPWAP_RULE_CREATE_PPPOE_CONFIGURED;
		}
	}

	if (capwap_rule->l3_proto == NSS_CAPWAP_TUNNEL_IPV6 && capwap_rule->which_udp == NSS_CAPWAP_TUNNEL_UDPLite) {
		type_flags |= NSS_CAPWAP_ENCAP_UDPLITE_HDR_CSUM;
	}

	/*
	 * Copy over the IP rule information to capwap rule for encap side.
	 * This will avoid any confusions because IP rule direction is AC->AP and
	 * capwap encap rule direction is AP->AC.
	 */
	if (v4) {
		capwap_rule->encap.src_port = htonl(v4->dest_port);
		capwap_rule->encap.src_ip.ip.ipv4 = htonl(v4->dest_ip);

		capwap_rule->encap.dest_port = htonl(v4->src_port);
		capwap_rule->encap.dest_ip.ip.ipv4 = htonl(v4->src_ip);
	} else {
		capwap_rule->encap.src_port = htonl(v6->dest_port);
		capwap_rule->encap.src_ip.ip.ipv6[0] = htonl(v6->dest_ip[0]);
		capwap_rule->encap.src_ip.ip.ipv6[1] = htonl(v6->dest_ip[1]);
		capwap_rule->encap.src_ip.ip.ipv6[2] = htonl(v6->dest_ip[2]);
		capwap_rule->encap.src_ip.ip.ipv6[3] = htonl(v6->dest_ip[3]);

		capwap_rule->encap.dest_port = htonl(v6->src_port);
		capwap_rule->encap.dest_ip.ip.ipv6[0] = htonl(v6->src_ip[0]);
		capwap_rule->encap.dest_ip.ip.ipv6[1] = htonl(v6->src_ip[1]);
		capwap_rule->encap.dest_ip.ip.ipv6[2] = htonl(v6->src_ip[2]);
		capwap_rule->encap.dest_ip.ip.ipv6[3] = htonl(v6->src_ip[3]);
	}

	status = nss_capwapmgr_create_capwap_rule(dev, capwap_if_num_inner, capwap_rule, type_flags);
	nss_capwapmgr_info("%px: dynamic interface if_num is :%d and capwap tunnel status:%d\n", dev, capwap_if_num_inner, status);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%px: %d: CAPWAP rule create failed with status: %d", dev, capwap_if_num_inner, status);
		status = NSS_CAPWAPMGR_FAILURE_CAPWAP_RULE;
		goto fail4;
	}

	status = nss_capwapmgr_create_capwap_rule(dev, capwap_if_num_outer, capwap_rule, type_flags);
	nss_capwapmgr_info("%px: dynamic interface if_num is :%d and capwap tunnel status:%d\n", dev, capwap_if_num_outer, status);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%px: %d: CAPWAP rule create failed with status: %d", dev, capwap_if_num_outer, status);
		status = NSS_CAPWAPMGR_FAILURE_CAPWAP_RULE;
		goto fail5;
	}

	priv = netdev_priv(dev);
	t = &priv->tunnel[tunnel_id];
	t->tunnel_id = tunnel_id;
	t->vp_num_decap = vp_num_decap;
	t->vp_num_encap = vp_num_encap;

	/*
	 * For non trustsec tunnels, PPE handles 5 tuple lookup.
	 * For trustsec tunnels we use acl rule to route the trustsec traffic
	 * to nss-fw and trustsec_rx node handles 5 tuple lookup.
	 */
	if (!outer_trustsec_enabled) {
		if (v4) {
			v4->dest_interface_num = forwarding_ifnum;
			status = nss_capwapmgr_ppe_create_ipv4_rule(t, v4);
		} else {
			v6->dest_interface_num = forwarding_ifnum;
			status = nss_capwapmgr_ppe_create_ipv6_rule(t, v6);
		}
	} else {
		if (v4) {
			status = nss_capwapmgr_trustsec_rule_create_v4(t, v4, capwap_if_num_outer);
		} else {
			status = nss_capwapmgr_trustsec_rule_create_v6(t, v6, capwap_if_num_outer);
		}
	}

	if (status != NSS_CAPWAPMGR_SUCCESS) {
			nss_capwapmgr_warn("%px: IPv4/IPv6 rule create failed with status: %d", dev, status);
			goto fail6;
		}

	nss_capwapmgr_info("%px: %d: %d: CAPWAP TUNNEL CREATE DONE tunnel_id:%d (%px)\n", dev, capwap_if_num_inner, capwap_if_num_outer, tunnel_id, t);

	/*
	 * Keep a copy of rule information.
	 */
	if (v4) {
		memcpy(&t->ip_rule.v4, v4, sizeof (struct nss_ipv4_create));
	} else {
		memcpy(&t->ip_rule.v6, v6, sizeof (struct nss_ipv6_create));
	}

	memcpy(&t->capwap_rule, capwap_rule, sizeof (struct nss_capwap_rule_msg));

	/*
	 * Make it globally visible inside the netdev.
	 */
	t->internal_dev_encap = internal_dev_encap;
	t->internal_dev_decap = internal_dev_decap;
	t->if_num_inner = capwap_if_num_inner;
	t->if_num_outer = capwap_if_num_outer;
	priv->if_num_to_tunnel_id[capwap_if_num_inner] = tunnel_id;
	priv->if_num_to_tunnel_id[capwap_if_num_outer] = tunnel_id;
	t->tunnel_state |= NSS_CAPWAPMGR_TUNNEL_STATE_CONFIGURED;
	t->type_flags = type_flags;

	goto done;

fail6:
	nss_capwapmgr_tunnel_action(priv->nss_ctx, dev, capwap_if_num_outer, NSS_CAPWAP_MSG_TYPE_UNCFG_RULE);
fail5:
	if (outer_trustsec_enabled) {
		nss_trustsec_tx_unconfigure_sgt(capwap_if_num_outer, capwap_rule->outer_sgt_value);
	}
	nss_capwapmgr_tunnel_action(priv->nss_ctx, dev, capwap_if_num_inner, NSS_CAPWAP_MSG_TYPE_UNCFG_RULE);
fail4:
	nss_capwapmgr_unregister_with_nss(capwap_if_num_outer);
fail3:
	nss_dynamic_interface_dealloc_node(capwap_if_num_outer, NSS_DYNAMIC_INTERFACE_TYPE_CAPWAP_OUTER);
fail2:
	nss_capwapmgr_unregister_with_nss(capwap_if_num_inner);
fail1:
	nss_dynamic_interface_dealloc_node(capwap_if_num_inner, NSS_DYNAMIC_INTERFACE_TYPE_CAPWAP_HOST_INNER);
fail0:
	ppe_vp_free(vp_num_encap);
	free_netdev(internal_dev_encap);
fail:
	if (!outer_trustsec_enabled) {
		ppe_vp_free(vp_num_decap);
		free_netdev(internal_dev_decap);
	}

done:
	dev_put(dev);
	return status;
}

/*
 * nss_capwapmgr_tunnel_save_stats()
 *	Internal function to save tunnel stats when a tunnel is being
 *	destroyed.
 */
static void nss_capwapmgr_tunnel_save_stats(struct nss_capwap_tunnel_stats *save, struct nss_capwap_tunnel_stats *fstats)
{
	save->dtls_pkts += fstats->dtls_pkts;

	save->rx_segments += fstats->rx_segments;
	save->rx_dup_frag += fstats->rx_dup_frag;
	save->rx_oversize_drops += fstats->rx_oversize_drops;
	save->rx_frag_timeout_drops += fstats->rx_frag_timeout_drops;
	save->rx_n2h_drops += fstats->rx_n2h_drops;
	save->rx_n2h_queue_full_drops += fstats->rx_n2h_queue_full_drops;
	save->rx_mem_failure_drops += fstats->rx_mem_failure_drops;
	save->rx_csum_drops += fstats->rx_csum_drops;
	save->rx_malformed += fstats->rx_malformed;
	save->rx_frag_gap_drops += fstats->rx_frag_gap_drops;

	save->tx_segments += fstats->tx_segments;
	save->tx_queue_full_drops += fstats->tx_queue_full_drops;
	save->tx_mem_failure_drops += fstats->tx_mem_failure_drops;
	save->tx_dropped_sg_ref += fstats->tx_dropped_sg_ref;
	save->tx_dropped_ver_mis += fstats->tx_dropped_ver_mis;
	save->tx_dropped_hroom += fstats->tx_dropped_hroom;
	save->tx_dropped_dtls += fstats->tx_dropped_dtls;
	save->tx_dropped_nwireless += fstats->tx_dropped_nwireless;

	/*
	 * add pnode stats now.
	 */
	save->pnode_stats.rx_packets += fstats->pnode_stats.rx_packets;
	save->pnode_stats.rx_bytes += fstats->pnode_stats.rx_bytes;
	save->pnode_stats.rx_dropped += fstats->pnode_stats.rx_dropped;
	save->pnode_stats.tx_packets += fstats->pnode_stats.tx_packets;
	save->pnode_stats.tx_bytes += fstats->pnode_stats.tx_bytes;
}

/*
 * nss_capwapmgr_flow_rule_action()
 */
static inline nss_capwapmgr_status_t nss_capwapmgr_flow_rule_action(struct net_device *dev, uint8_t tunnel_id,
						nss_capwap_msg_type_t cmd, struct nss_capwapmgr_flow_info *flow_info)
{
	struct nss_capwapmgr_priv *priv;
	struct nss_capwap_msg capwapmsg;
	struct nss_capwap_flow_rule_msg *ncfrm;
	struct nss_capwapmgr_tunnel *t = NULL;
	nss_capwapmgr_status_t status;

	dev_hold(dev);
	status = nss_capwapmgr_get_tunnel(dev, tunnel_id, &t);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%px: can't find tunnel: %d\n", dev, tunnel_id);
		goto done;
	}

	priv = netdev_priv(dev);

	memset(&capwapmsg, 0, sizeof(struct nss_capwap_msg));
	nss_capwap_msg_init(&capwapmsg, t->if_num_outer, cmd,
		sizeof(struct nss_capwap_flow_rule_msg), nss_capwapmgr_msg_event_receive, dev);

	/*
	 * Set flow rule message
	 */
	if (cmd == NSS_CAPWAP_MSG_TYPE_FLOW_RULE_ADD) {
		ncfrm = &capwapmsg.msg.flow_rule_add;
		memcpy(&ncfrm->flow_attr, &flow_info->flow_attr, sizeof(struct nss_capwap_flow_attr));
	} else {
		ncfrm = &capwapmsg.msg.flow_rule_del;
	}
	ncfrm->protocol = flow_info->protocol;
	ncfrm->src_port = flow_info->src_port;
	ncfrm->dst_port = flow_info->dst_port;
	ncfrm->ip_version = flow_info->ip_version;
	memcpy(ncfrm->src_ip, flow_info->src_ip, sizeof(struct in6_addr));
	memcpy(ncfrm->dst_ip, flow_info->dst_ip, sizeof(struct in6_addr));

	/*
	 * Send flow rule message to NSS core
	 */
	status = nss_capwapmgr_tx_msg_sync(priv->nss_ctx, dev, &capwapmsg);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%px: send flow rule message failed with error: %d\n", dev, status);
	}

done:
	dev_put(dev);
	return status;
}

/*
 * nss_capwapmgr_netdev_up()
 *	NSS CAPWAP Tunnel device i/f up handler
 */
static int nss_capwapmgr_netdev_up(struct net_device *netdev)
{
	uint8_t i;
	for (i = 0; i < NSS_CAPWAPMGR_MAX_TUNNELS; i++) {
		(void)nss_capwapmgr_enable_tunnel(nss_capwapmgr_ndev, i);
	}

	return NOTIFY_DONE;
}

/*
 * nss_capwapmgr_netdev_down()
 *	NSS CAPWAP Tunnel device i/f up handler
 */
static int nss_capwapmgr_netdev_down(struct net_device *netdev)
{
	uint8_t i;
	for (i = 0; i < NSS_CAPWAPMGR_MAX_TUNNELS; i++) {
		(void)nss_capwapmgr_disable_tunnel(nss_capwapmgr_ndev, i);
	}

	return NOTIFY_DONE;
}

/*
 * nss_capwapmgr_netdev_event()
 *	Net device notifier for NSS CAPWAP manager module
 */
static int nss_capwapmgr_netdev_event(struct notifier_block *nb, unsigned long event, void *dev)
{
	struct net_device *netdev = (struct net_device *)dev;

	if (strstr(netdev->name, NSS_CAPWAPMGR_NETDEV_NAME) == NULL) {
		return NOTIFY_DONE;
	}

	switch (event) {
	case NETDEV_UP:
		nss_capwapmgr_trace("%px: NETDEV_UP: event %lu name %s\n", netdev, event, netdev->name);
		return nss_capwapmgr_netdev_up(netdev);

	case NETDEV_DOWN:
		nss_capwapmgr_trace("%px: NETDEV_DOWN: event %lu name %s\n", netdev, event, netdev->name);
		return nss_capwapmgr_netdev_down(netdev);

	default:
		nss_capwapmgr_trace("%px: Unhandled notifier event %lu name %s\n", netdev, event, netdev->name);
		break;
	}

	return NOTIFY_DONE;
}

/*
 * nss_capwapmgr_ppe2host_read()
 *	capwapmanager ppe2host read handler
 */
static ssize_t nss_capwapmgr_ppe2host_read(struct file *f, char *buf, size_t count, loff_t *offset)
{
	int len;
	char lbuf[26];

	len = snprintf(lbuf, sizeof(lbuf), "capwap ppe2host %s\n", (global.ppe2host) ? ("enabled") : ("disabled"));

	return simple_read_from_buffer(buf, count, offset, lbuf, len);
}

/*
 * nss_capwapmgr_ppe2host_write()
 *	capwapmanager ppe2host write handler
 */
static ssize_t nss_capwapmgr_ppe2host_write(struct file *f, const char *buffer, size_t len, loff_t *offset)
{
	ssize_t size;
	char data[16];
	bool res;
	int status;

	size = simple_write_to_buffer(data, sizeof(data), offset, buffer, len);
	if (size < 0) {
		nss_capwapmgr_warn("Error reading the input for capwap ppe2host configuration");
		return size;
	}

	status = kstrtobool(data, &res);
	if (status) {
		nss_capwapmgr_warn("Error reading the input for capwap ppe2host configuration");
		return status;
	}

	global.ppe2host = res;
	return len;
}

/*
 * nss_capwapmgr_ppe2host_file_fops
 *	File handler for configuring ppe2host
 */
const struct file_operations nss_capwapmgr_ppe2host_file_fops = {
	.owner = THIS_MODULE,
	.write = nss_capwapmgr_ppe2host_write,
	.read = nss_capwapmgr_ppe2host_read
};

/*
 * nss_capwapmgr_dentry_init()
 *	Create capwap accel_mode debugfs entry.
 */
static bool nss_capwapmgr_dentry_init(void)
{
	/*
	 * Initialize debugfs directory.
	 */
	struct dentry *parent;
	struct dentry *clients;

	parent = debugfs_lookup("qca-nss-ppe", NULL);
	if (!parent) {
		nss_capwapmgr_warn("parent debugfs entry for qca-nss-ppe not present");
		return false;
	}

	clients = debugfs_lookup("clients", parent);
	if (!clients) {
		nss_capwapmgr_warn("clients debugfs entry inside qca-nss-ppe not present");
		return false;
	}

	global.capwap_dentry = debugfs_create_dir("capwap", clients);
	if (!global.capwap_dentry) {
		nss_capwapmgr_warn("Failed to create capwap debugfs under qca-nss-ppe/clients/");
		return false;
	}

	if (!debugfs_create_file("ppe2host", (S_IRUGO | S_IWUSR), global.capwap_dentry, NULL, &nss_capwapmgr_ppe2host_file_fops)) {
		nss_capwapmgr_warn("Failed to create debugfs entry for ppe2host");
		debugfs_remove_recursive(global.capwap_dentry);
		return false;
	}

	return true;
}

/*
 * Linux Net device Notifier
 */
struct notifier_block nss_capwapmgr_netdev_notifier = {
	.notifier_call = nss_capwapmgr_netdev_event,
};

/*
 * nss_capwapmgr_netdev_create()
 *	API to create a CAPWAP netdev
 */
struct net_device *nss_capwapmgr_netdev_create()
{
	struct nss_capwapmgr_priv *priv;
	struct nss_capwapmgr_response *r;
	struct net_device *ndev;
	int i;
	int err;

	ndev = alloc_netdev(sizeof(struct nss_capwapmgr_priv),
					"nsscapwap%d", NET_NAME_UNKNOWN, nss_capwapmgr_dummy_netdev_setup);
	if (!ndev) {
		nss_capwapmgr_warn("Error allocating netdev\n");
		return NULL;
	}

	err = register_netdev(ndev);
	if (err) {
		nss_capwapmgr_warn("register_netdev() fail with error :%d\n", err);
		free_netdev(ndev);
		return NULL;
	}

	priv = netdev_priv(ndev);
	priv->nss_ctx = nss_capwap_get_ctx();
	priv->tunnel = kmalloc(sizeof(struct nss_capwapmgr_tunnel) * NSS_CAPWAPMGR_MAX_TUNNELS, GFP_ATOMIC);
	if (!priv->tunnel) {
		nss_capwapmgr_warn("%px: failed to allocate tunnel memory\n", ndev);
		goto fail1;
	}
	memset(priv->tunnel, 0, sizeof(struct nss_capwapmgr_tunnel) * NSS_CAPWAPMGR_MAX_TUNNELS);
	for (i = 0; i < NSS_CAPWAPMGR_MAX_TUNNELS; i++) {
		priv->tunnel[i].if_num_inner = -1;
		priv->tunnel[i].if_num_outer = -1;
	}

	priv->resp = kmalloc(sizeof(struct nss_capwapmgr_response) * NSS_MAX_DYNAMIC_INTERFACES, GFP_ATOMIC);
	if (!priv->resp) {
		nss_capwapmgr_warn("%px: failed to allocate tunnel response memory\n", ndev);
		goto fail2;
	}
	for (i = 0; i < NSS_MAX_DYNAMIC_INTERFACES; i++) {
		r = &priv->resp[i];
		init_waitqueue_head(&r->wq);

		/*
		 * CAPWAP interface is limited to one command per-interface.
		 */
		sema_init(&r->sem, 1);
	}

	priv->if_num_to_tunnel_id = kmalloc(sizeof(uint8_t) * NSS_MAX_DYNAMIC_INTERFACES, GFP_ATOMIC);
	if (!priv->if_num_to_tunnel_id) {
		nss_capwapmgr_warn("%px: failed to allocate if_num to tunnel_id memory\n", ndev);
		goto fail3;
	}
	memset(priv->if_num_to_tunnel_id, 0, sizeof(uint8_t) * NSS_MAX_DYNAMIC_INTERFACES);

	if (nss_cmn_register_queue_decongestion(priv->nss_ctx, nss_capwapmgr_decongestion_callback, ndev) != NSS_CB_REGISTER_SUCCESS) {
		nss_capwapmgr_warn("%px: failed to register decongestion callback\n", ndev);
		goto fail4;
	}

	return ndev;

fail4:
	kfree(priv->if_num_to_tunnel_id);
fail3:
	kfree(priv->resp);
fail2:
	kfree(priv->tunnel);
fail1:
	unregister_netdev(ndev);
	free_netdev(ndev);
	return NULL;
}
EXPORT_SYMBOL(nss_capwapmgr_netdev_create);

/*
 * nss_capwapmgr_netdev_destroy()
 *	API for destroying a netdevice.
 *
 * All the CAPWAP tunnels must be destroyed first before netdevice.
 */
nss_capwapmgr_status_t nss_capwapmgr_netdev_destroy(struct net_device *dev)
{
	rtnl_is_locked() ? unregister_netdevice(dev) : unregister_netdev(dev);
	return NSS_CAPWAPMGR_SUCCESS;
}
EXPORT_SYMBOL(nss_capwapmgr_netdev_destroy);

/*
 * nss_capwapmgr_get_dtls_netdev()
 *	API for getting the dtls netdev associated to the capwap tunnel
 *
 * The caller is expected to do a dev_put() to release the reference.
 */
struct net_device *nss_capwapmgr_get_dtls_netdev(struct net_device *capwap_dev, uint8_t tunnel_id)
{
	struct nss_capwapmgr_tunnel *t;
	nss_capwapmgr_status_t status;
	struct net_device *dtls_dev;

	dev_hold(capwap_dev);
	status = nss_capwapmgr_get_tunnel(capwap_dev, tunnel_id, &t);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%px: can't find tunnel: %d\n", capwap_dev, tunnel_id);
		dev_put(capwap_dev);
		return NULL;
	}

	dtls_dev = t->dtls_dev;
	dev_hold(dtls_dev);

	dev_put(capwap_dev);

	return dtls_dev;
}
EXPORT_SYMBOL(nss_capwapmgr_get_dtls_netdev);

/*
 * nss_capwapmgr_update_path_mtu()
 *	API for updating Path MTU
 */
nss_capwapmgr_status_t nss_capwapmgr_update_path_mtu(struct net_device *dev, uint8_t tunnel_id, uint32_t mtu)
{
	struct nss_capwapmgr_priv *priv;
	struct nss_capwap_msg capwapmsg;
	struct nss_capwapmgr_tunnel *t = NULL;
	nss_capwapmgr_status_t status;
	ppe_vp_status_t ppe_vp_status;
	struct nss_ipv4_create *v4;
	struct nss_ipv6_create *v6;
	uint32_t outer_trustsec_enabled;

	if (mtu > NSS_CAPWAP_MAX_MTU) {
		nss_capwapmgr_warn("%px: invalid path_mtu: %d, max: %d\n", dev, mtu, NSS_CAPWAP_MAX_MTU);
		return NSS_CAPWAPMGR_FAILURE_INVALID_PATH_MTU;
	}

	dev_hold(dev);
	status = nss_capwapmgr_get_tunnel(dev, tunnel_id, &t);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%px: can't find tunnel: %d\n", dev, tunnel_id);
		goto done;
	}

	outer_trustsec_enabled = t->capwap_rule.enabled_features & NSS_CAPWAPMGR_FEATURE_OUTER_TRUSTSEC_ENABLED;
	priv = netdev_priv(dev);
	nss_capwapmgr_info("%px: %d: tunnel update MTU is being called\n", dev, t->if_num_inner);

	/*
	 * Prepare the tunnel configuration parameter to send to NSS FW
	 */
	memset(&capwapmsg, 0, sizeof(struct nss_capwap_msg));

	/*
	 * Send CAPWAP data tunnel command to NSS
	 */
	nss_capwap_msg_init(&capwapmsg, t->if_num_inner, NSS_CAPWAP_MSG_TYPE_UPDATE_PATH_MTU,
		sizeof(struct nss_capwap_path_mtu_msg), nss_capwapmgr_msg_event_receive, dev);
	capwapmsg.msg.mtu.path_mtu = htonl(mtu);
	status = nss_capwapmgr_tx_msg_sync(priv->nss_ctx, dev, &capwapmsg);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%px: Update Path MTU CAPWAP tunnel error : %d \n", dev, status);
		status = NSS_CAPWAPMGR_FAILURE_CAPWAP_RULE;
		goto done;
	}

	/*
	 * Update the MTU for the CAPWAP VP.
	 */
	ppe_vp_status = ppe_vp_mtu_set(t->vp_num_decap, mtu);
	if (ppe_vp_status != PPE_VP_STATUS_SUCCESS) {
		nss_capwapmgr_warn("%px: PPE_VP mtu set failed %d\n", dev, ppe_vp_status);
		status = NSS_CAPWAPMGR_FAILURE_VP_MTU_SET;
		goto fail;
	}

	/*
	 * Fon trustsec tunnels we do not update the PPE rule.
	 */
	if (outer_trustsec_enabled) {
		goto done;
	}

	/*
	 * Delete and re-create the IPv4/IPv6 rule with the new MTU for flow and return
	 */
	if (t->tunnel_state & NSS_CAPWAPMGR_TUNNEL_STATE_IPRULE_CONFIGURED) {
		if (t->capwap_rule.l3_proto == NSS_CAPWAP_TUNNEL_IPV4) {
			status = nss_capwapmgr_ppe_destroy_ipv4_rule(t);
			if (status != NSS_CAPWAPMGR_SUCCESS) {
				goto fail1;
			}
		} else {
			status = nss_capwapmgr_ppe_destroy_ipv6_rule(t);
			if (status != NSS_CAPWAPMGR_SUCCESS) {
				goto fail1;
			}
		}
	}

	if (t->capwap_rule.l3_proto == NSS_CAPWAP_TUNNEL_IPV4) {
		v4 = &t->ip_rule.v4;
		v4->from_mtu = v4->to_mtu = mtu;
		status = nss_capwapmgr_ppe_create_ipv4_rule(t, v4);
		if (status != NSS_CAPWAPMGR_SUCCESS) {
			v4->from_mtu = v4->to_mtu = ntohl(t->capwap_rule.encap.path_mtu);
			nss_capwapmgr_warn("%px: Configure ipv4 rule failed : %d\n", dev, status);
			goto fail1;
		}
	} else {
		v6 = &t->ip_rule.v6;
		v6->from_mtu = v6->to_mtu = mtu;
		status = nss_capwapmgr_ppe_create_ipv6_rule(t, v6);
		if (status != NSS_CAPWAPMGR_SUCCESS) {
			v6->from_mtu = v6->to_mtu = ntohl(t->capwap_rule.encap.path_mtu);
			nss_capwapmgr_warn("%px: Configure ipv6 rule failed : %d\n", dev, status);
			goto fail1;
		}
	}

	t->capwap_rule.encap.path_mtu = htonl(mtu);
	goto done;

fail1:
	ppe_vp_status = ppe_vp_mtu_set(t->vp_num_decap, t->capwap_rule.encap.path_mtu);
	if (ppe_vp_status != PPE_VP_STATUS_SUCCESS) {
		nss_capwapmgr_warn("%px: Restore PPE_VP mtu failed %d\n", dev, ppe_vp_status);
	}

fail:
	nss_capwapmgr_warn("%px: Update Path MTU IP RULE tunnel error : %d \n", dev, status);
	capwapmsg.msg.mtu.path_mtu = t->capwap_rule.encap.path_mtu;
	if (nss_capwapmgr_tx_msg_sync(priv->nss_ctx, dev, &capwapmsg) != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%px: Restore Path MTU CAPWAP tunnel error : %d \n", dev, status);
	}

done:
	dev_put(dev);
	return status;
}
EXPORT_SYMBOL(nss_capwapmgr_update_path_mtu);

/*
 * nss_capwapmgr_update_dest_mac_addr()
 *	API for updating Destination Mac Addr
 */
nss_capwapmgr_status_t nss_capwapmgr_update_dest_mac_addr(struct net_device *dev, uint8_t tunnel_id, uint8_t *mac_addr)
{
	struct nss_capwapmgr_priv *priv;
	struct nss_capwapmgr_tunnel *t = NULL;
	nss_capwapmgr_status_t status = NSS_CAPWAPMGR_SUCCESS;
	struct nss_ipv4_create *v4;
	struct nss_ipv6_create *v6;
	uint8_t mac_addr_old[ETH_ALEN];
	uint32_t outer_trustsec_enabled;

	dev_hold(dev);
	status = nss_capwapmgr_get_tunnel(dev, tunnel_id, &t);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%px: can't find tunnel: %d\n", dev, tunnel_id);
		goto done;
	}

	outer_trustsec_enabled = t->capwap_rule.enabled_features & NSS_CAPWAPMGR_FEATURE_OUTER_TRUSTSEC_ENABLED;
	priv = netdev_priv(dev);
	nss_capwapmgr_info("%px: %d: tunnel update mac Addr is being called\n", dev, tunnel_id);

	/*
	 * For trustsec enabled tunnels delete and update the trustsec tx rule with new destination mac address.
	 *
	 * For non trustsec tunnels delete and re-create the IPv4/IPv6 rule with the new destination mac address
	 * for flow and return. Since the encap direction is handled by the return rule, we are updating the src_mac.
	 */
	if (outer_trustsec_enabled && (t->tunnel_state & NSS_CAPWAPMGR_TUNNEL_STATE_TRUSTSEC_TX_CONFIGURED)) {
		status = nss_capwapmgr_tx_rule_destroy(t, true);
	} else if (t->tunnel_state & NSS_CAPWAPMGR_TUNNEL_STATE_IPRULE_CONFIGURED) {
		if (t->capwap_rule.l3_proto == NSS_CAPWAP_TUNNEL_IPV4) {
			status = nss_capwapmgr_ppe_destroy_ipv4_rule(t);
		} else {
			status = nss_capwapmgr_ppe_destroy_ipv6_rule(t);
		}
	}

	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%px: Update Destination Mac for tunnel error: %d\n", dev, status);
		goto done;
	}

	if (t->capwap_rule.l3_proto == NSS_CAPWAP_TUNNEL_IPV4) {
		v4 = &t->ip_rule.v4;
		memcpy(mac_addr_old, v4->src_mac, ETH_ALEN);
		memcpy(v4->src_mac, mac_addr, ETH_ALEN);
		if (outer_trustsec_enabled) {
			status = nss_capwapmgr_tx_rule_create_v4(t, v4, true);
		} else {
			status = nss_capwapmgr_ppe_create_ipv4_rule(t, v4);
		}
		if (status != NSS_CAPWAPMGR_SUCCESS) {
			nss_capwapmgr_warn("%px: Update Destination Mac for tunnel error : %d \n", dev, status);
			memcpy(t->ip_rule.v4.src_mac, mac_addr_old, ETH_ALEN);
			goto done;
		}
	} else {
		v6 = &t->ip_rule.v6;
		memcpy(mac_addr_old, v6->src_mac, ETH_ALEN);
		memcpy(v6->src_mac, mac_addr, ETH_ALEN);
		if (outer_trustsec_enabled) {
			status = nss_capwapmgr_tx_rule_create_v6(t, v6, true);
		} else {
			status = nss_capwapmgr_ppe_create_ipv6_rule(t, &t->ip_rule.v6);
		}
		if (status != NSS_CAPWAPMGR_SUCCESS) {
			nss_capwapmgr_warn("%px: Update Destination Mac for tunnel error : %d \n", dev, status);
			memcpy(t->ip_rule.v6.src_mac, mac_addr_old, ETH_ALEN);
		}
	}

done:
	dev_put(dev);
	return status;
}
EXPORT_SYMBOL(nss_capwapmgr_update_dest_mac_addr);

/*
 * nss_capwapmgr_update_src_interface()
 *	API for updating Source Interface
 */
nss_capwapmgr_status_t nss_capwapmgr_update_src_interface(struct net_device *dev, uint8_t tunnel_id, int32_t src_interface_num)
{
	struct nss_capwapmgr_priv *priv;
	struct nss_capwapmgr_tunnel *t = NULL;
	nss_capwapmgr_status_t status;
	int32_t src_interface_num_temp;
	uint32_t outer_trustsec_enabled;

	dev_hold(dev);
	status = nss_capwapmgr_get_tunnel(dev, tunnel_id, &t);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%px: can't find tunnel: %d\n", dev, tunnel_id);
		goto done;
	}

	outer_trustsec_enabled = t->capwap_rule.enabled_features & NSS_CAPWAPMGR_FEATURE_OUTER_TRUSTSEC_ENABLED;
	priv = netdev_priv(dev);
	nss_capwapmgr_info("%px: %d: tunnel update source interface is being called\n", dev, tunnel_id);

	/*
	 * For trustsec enabled tunnels, delete and recreate trustsec tx rule with the new source interface.
	 *
	 * For non trustsec tunnels destroy/re-create the IPv4/IPv6 rule with the new Interface number for
	 * flow and return
	 */
	if (outer_trustsec_enabled && (t->tunnel_state & NSS_CAPWAPMGR_TUNNEL_STATE_TRUSTSEC_TX_CONFIGURED)) {
		status = nss_capwapmgr_tx_rule_destroy(t, true);
	} else if (t->tunnel_state & NSS_CAPWAPMGR_TUNNEL_STATE_IPRULE_CONFIGURED) {
		if (t->capwap_rule.l3_proto == NSS_CAPWAP_TUNNEL_IPV4) {
			status = nss_capwapmgr_ppe_destroy_ipv4_rule(t);
		} else {
			status = nss_capwapmgr_ppe_destroy_ipv6_rule(t);
		}
	}

	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%px: Update source interface failed with error: %d\n", dev, status);
		goto done;
	}

	if (t->capwap_rule.l3_proto == NSS_CAPWAP_TUNNEL_IPV4) {
		src_interface_num_temp = t->ip_rule.v4.src_interface_num;
		t->ip_rule.v4.src_interface_num = src_interface_num;
		if (outer_trustsec_enabled) {
			status = nss_capwapmgr_tx_rule_create_v4(t, &t->ip_rule.v4, true);
		} else {
			status = nss_capwapmgr_ppe_create_ipv4_rule(t, &t->ip_rule.v4);
		}

		if (status != NSS_CAPWAPMGR_SUCCESS) {
			nss_capwapmgr_warn("%px: configure ipv4 rule failed : %d\n", dev, status);
			t->ip_rule.v4.src_interface_num = src_interface_num_temp;
			goto done;
		}
	} else {
		src_interface_num_temp = t->ip_rule.v6.src_interface_num;
		t->ip_rule.v6.src_interface_num = src_interface_num;
		if (outer_trustsec_enabled) {
			status = nss_capwapmgr_tx_rule_create_v6(t, &t->ip_rule.v6, true);
		} else {
			status = nss_capwapmgr_ppe_create_ipv6_rule(t, &t->ip_rule.v6);
		}

		if (status != NSS_CAPWAPMGR_SUCCESS) {
			nss_capwapmgr_warn("%px: configure ipv6 rule failed : %d\n", dev, status);
			t->ip_rule.v6.src_interface_num = src_interface_num_temp;
		}
	}

done:
	dev_put(dev);
	return status;
}
EXPORT_SYMBOL(nss_capwapmgr_update_src_interface);

/*
 * nss_capwapmgr_configure_dtls
 *	Enable or disable DTLS of a capwap tunnel
 */
nss_capwapmgr_status_t nss_capwapmgr_configure_dtls(struct net_device *capwap_dev, uint8_t tunnel_id, uint8_t enable, struct nss_dtlsmgr_config *data)
{
	struct nss_capwap_msg capwapmsg_inner, capwapmsg_outer;
	nss_capwapmgr_status_t status = NSS_CAPWAPMGR_SUCCESS;
	struct nss_capwapmgr_priv *priv;
	struct nss_capwapmgr_tunnel *t;
	uint32_t ip_if_num;
	bool configured;

	dev_hold(capwap_dev);
	status = nss_capwapmgr_get_tunnel(capwap_dev, tunnel_id, &t);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%px: can't find tunnel: %d\n", capwap_dev, tunnel_id);
		dev_put(capwap_dev);
		return status;
	}

	configured = !!(t->capwap_rule.enabled_features & NSS_CAPWAPMGR_FEATURE_DTLS_ENABLED);

	/*
	 * Fail if capwap tunnel already has dtls enabled.
	 */
	if (enable && configured) {
		nss_capwapmgr_warn("%px: nothing changed for tunnel: %d\n", capwap_dev, tunnel_id);
		dev_put(capwap_dev);
		return NSS_CAPWAPMGR_FAILURE_BAD_PARAM;
	}

	/*
	 * Fail if capwap tunnel has dtls disabled and we try to disable it again.
	 */
	if (!enable && !configured) {
		nss_capwapmgr_warn("%px: nothing changed for tunnel: %d\n", capwap_dev, tunnel_id);
		dev_put(capwap_dev);
		return NSS_CAPWAPMGR_FAILURE_BAD_PARAM;
	}

	/*
	 * Check if the capwap tunnel is enabled. We operate on disabled tunnel only.
	 */
	if (t->tunnel_state & NSS_CAPWAPMGR_TUNNEL_STATE_ENABLED) {
		nss_capwapmgr_warn("%px: tunnel %d is already enabled\n", capwap_dev, tunnel_id);
		dev_put(capwap_dev);
		return NSS_CAPWAPMGR_FAILURE_TUNNEL_ENABLED;
	}

	/*
	 * Prepare DTLS configure message
	 */
	memset(&capwapmsg_inner, 0, sizeof(struct nss_capwap_msg));
	nss_capwap_msg_init(&capwapmsg_inner, t->if_num_inner, NSS_CAPWAP_MSG_TYPE_DTLS,
		sizeof(struct nss_capwap_dtls_msg), nss_capwapmgr_msg_event_receive, capwap_dev);

	memset(&capwapmsg_outer, 0, sizeof(struct nss_capwap_msg));
	nss_capwap_msg_init(&capwapmsg_outer, t->if_num_outer, NSS_CAPWAP_MSG_TYPE_DTLS,
		sizeof(struct nss_capwap_dtls_msg), nss_capwapmgr_msg_event_receive, capwap_dev);

	if (!enable) {
		nss_capwapmgr_info("%px disabling DTLS for tunnel: %d\n", capwap_dev, tunnel_id);

		ip_if_num = ppe_drv_iface_idx_get_by_dev(t->internal_dev_decap);
		capwapmsg_inner.msg.dtls.enable = 0;
		capwapmsg_inner.msg.dtls.dtls_inner_if_num = t->capwap_rule.dtls_inner_if_num;
		capwapmsg_inner.msg.dtls.mtu_adjust = 0;

		capwapmsg_outer.msg.dtls.enable = 0;
	} else {
		nss_capwapmgr_info("%px enabling DTLS for tunnel: %d\n", capwap_dev, tunnel_id);

		if (!t->capwap_rule.dtls_inner_if_num) {
			/*
			 * Create a DTLS node, we only validate caller is providing a DTLS
			 * configuration structure, the correctness of these settings are
			 * validated by dtlsmgr
			 */
			if (!data) {
				nss_capwapmgr_info("%px: dtls data required to create dtls tunnel\n", capwap_dev);
				dev_put(capwap_dev);
				return NSS_CAPWAPMGR_FAILURE_BAD_PARAM;
			}

			/*
			 * We only support the METADATA mode for pure DTLS tunnels; in CAPWAP-DTLS
			 * the offload will not send the packets starting with Metadata. We need to
			 * ensure that the user does not configure this mode accidentally.
			 */
			data->flags &= ~NSS_DTLSMGR_ENCAP_METADATA;
			data->decap.nexthop_ifnum = nss_capwap_ifnum_with_core_id(t->if_num_outer);

			data->vp_num_encap = t->vp_num_encap;
			t->dtls_dev = nss_dtlsmgr_session_create(data);
			if (!t->dtls_dev) {
				nss_capwapmgr_warn("%px: cannot create DTLS session\n", capwap_dev);
				dev_put(capwap_dev);
				return NSS_CAPWAPMGR_FAILURE_DI_ALLOC_FAILED;
			}

			/* Store the DTLS encap and decap interface numbers */
			t->capwap_rule.dtls_inner_if_num = nss_dtlsmgr_get_interface(t->dtls_dev,
										     NSS_DTLSMGR_INTERFACE_TYPE_INNER);
			t->capwap_rule.mtu_adjust = nss_dtlsmgr_encap_overhead(t->dtls_dev);
			nss_capwapmgr_info("%px: created dtls node for tunnel: %d if_num: %d mtu_adjust: %d\n",
					   capwap_dev, tunnel_id, t->capwap_rule.dtls_inner_if_num, t->capwap_rule.mtu_adjust);
		}

		ip_if_num = ppe_drv_iface_idx_get_by_dev(t->dtls_dev);

		capwapmsg_inner.msg.dtls.enable = 1;
		capwapmsg_outer.msg.dtls.enable = 1;
		capwapmsg_inner.msg.dtls.mtu_adjust = t->capwap_rule.mtu_adjust;
		capwapmsg_inner.msg.dtls.dtls_inner_if_num = t->capwap_rule.dtls_inner_if_num;
	}

	priv = netdev_priv(capwap_dev);

	/*
	 * Now configure capwap dtls
	 */
	t->tunnel_state |= NSS_CAPWAPMGR_TUNNEL_STATE_IPRULE_CONFIGURED;
	status = nss_capwapmgr_tx_msg_sync(priv->nss_ctx, capwap_dev, &capwapmsg_inner);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%px: configure DTLS failed for inner node: %d\n", capwap_dev, status);
		dev_put(capwap_dev);
		return status;
	}

	status = nss_capwapmgr_tx_msg_sync(priv->nss_ctx, capwap_dev, &capwapmsg_outer);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%px: configure DTLS failed for outer node: %d\n", capwap_dev, status);
		dev_put(capwap_dev);
		return status;
	}

	/*
	 * Recreate ipv4/v6 rules with the new interface number
	 */
	if (t->capwap_rule.l3_proto == NSS_CAPWAP_TUNNEL_IPV4) {
		if (t->tunnel_state & NSS_CAPWAPMGR_TUNNEL_STATE_IPRULE_CONFIGURED) {
			status = nss_capwapmgr_ppe_destroy_ipv4_rule(t);
			if (status != NSS_CAPWAPMGR_SUCCESS) {
				nss_capwapmgr_warn("%px: unconfigure ipv4 rule failed : %d\n", capwap_dev, status);
				dev_put(capwap_dev);
				return NSS_CAPWAPMGR_FAILURE_IP_DESTROY_RULE;
			}

			t->tunnel_state &= ~NSS_CAPWAPMGR_TUNNEL_STATE_IPRULE_CONFIGURED;
		}

		t->ip_rule.v4.dest_interface_num = ip_if_num;

		/*
		 * The 5 tuple are same as configured by capwap.
		 */
		status = nss_capwapmgr_ppe_create_ipv4_rule(t, &t->ip_rule.v4);
	} else {
		if (t->tunnel_state & NSS_CAPWAPMGR_TUNNEL_STATE_IPRULE_CONFIGURED) {
			status = nss_capwapmgr_ppe_destroy_ipv6_rule(t);
			if (status != NSS_CAPWAPMGR_SUCCESS) {
				nss_capwapmgr_warn("%px: unconfigure ipv6 rule failed : %d\n", capwap_dev, status);
				dev_put(capwap_dev);
				return NSS_CAPWAPMGR_FAILURE_IP_DESTROY_RULE;
			}

			t->tunnel_state &= ~NSS_CAPWAPMGR_TUNNEL_STATE_IPRULE_CONFIGURED;
		}

		t->ip_rule.v6.dest_interface_num = ip_if_num;

		/*
		 * The 5 tuple are same as configured by capwap.
		 */
		status = nss_capwapmgr_ppe_create_ipv6_rule(t, &t->ip_rule.v6);
	}

	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%px: configure ip rule failed : %d\n", capwap_dev, status);
		dev_put(capwap_dev);
		return NSS_CAPWAPMGR_FAILURE_IP_RULE;
	}

	if (enable) {
		t->capwap_rule.enabled_features |= NSS_CAPWAPMGR_FEATURE_DTLS_ENABLED;
	} else {
		t->capwap_rule.enabled_features &= ~NSS_CAPWAPMGR_FEATURE_DTLS_ENABLED;
	}
	dev_put(capwap_dev);
	return NSS_CAPWAPMGR_SUCCESS;
}
EXPORT_SYMBOL(nss_capwapmgr_configure_dtls);

/*
 * nss_capwapmgr_verify_dtls_rekey_param()
 *	Validate the rekey param for a DTLS tunnel and return the DTLS netdevice
 *
 *  The caller should hold the reference on the net device before calling.
 */
static inline struct net_device *nss_capwapmgr_verify_dtls_rekey_param(struct net_device *dev, uint8_t tunnel_id,
								 struct nss_dtlsmgr_config_update *udata)
{
	struct nss_capwapmgr_tunnel *t;
	nss_capwapmgr_status_t status;

	if (!udata) {
		nss_capwapmgr_info("%px: dtls session update data required\n", dev);
		return NULL;
	}

	status = nss_capwapmgr_get_tunnel(dev, tunnel_id, &t);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%px: can't find tunnel: %d\n", dev, tunnel_id);
		return NULL;
	}


	if (!(t->capwap_rule.enabled_features & NSS_CAPWAPMGR_FEATURE_DTLS_ENABLED)) {
		nss_capwapmgr_warn("%px: tunnel does not enable DTLS: %d\n", dev, tunnel_id);
		return NULL;
	}

	return t->dtls_dev;
}

/*
 * nss_capwapmgr_dtls_rekey_rx_cipher_update()
 *	Update the rx cipher key for an DTLS tunnel
 */
nss_capwapmgr_status_t nss_capwapmgr_dtls_rekey_rx_cipher_update(struct net_device *dev, uint8_t tunnel_id,
								 struct nss_dtlsmgr_config_update *udata)
{
	struct net_device *dtls_ndev;

	dev_hold(dev);
	dtls_ndev = nss_capwapmgr_verify_dtls_rekey_param(dev, tunnel_id, udata);
	dev_put(dev);

	if (!dtls_ndev) {
		goto fail;
	}

	/*
	 * Calling dtlsmgr for rekey
	 */
	if (nss_dtlsmgr_session_update_decap(dtls_ndev, udata) != NSS_DTLSMGR_OK) {
		goto fail;
	}
	return NSS_CAPWAPMGR_SUCCESS;

fail:
	nss_capwapmgr_warn("%px: tunnel: %d rekey rx cipher update failed\n", dtls_ndev, tunnel_id);
	return NSS_CAPWAPMGR_FAILURE_INVALID_DTLS_CFG;
}
EXPORT_SYMBOL(nss_capwapmgr_dtls_rekey_rx_cipher_update);

/*
 * nss_capwapmgr_dtls_rekey_tx_cipher_update()
 *	Update the tx cipher key for an DTLS tunnel
 */
nss_capwapmgr_status_t nss_capwapmgr_dtls_rekey_tx_cipher_update(struct net_device *dev, uint8_t tunnel_id,
								 struct nss_dtlsmgr_config_update *udata)
{
	struct net_device *dtls_ndev;

	dev_hold(dev);
	dtls_ndev = nss_capwapmgr_verify_dtls_rekey_param(dev, tunnel_id, udata);
	dev_put(dev);

	if (!dtls_ndev) {
		goto fail;
	}

	/*
	 * Calling dtlsmgr for rekey
	 */
	if (nss_dtlsmgr_session_update_encap(dtls_ndev, udata) != NSS_DTLSMGR_OK) {
		goto fail;
	}
	return NSS_CAPWAPMGR_SUCCESS;

fail:
	nss_capwapmgr_warn("%px: tunnel: %d rekey rx cipher update failed\n", dtls_ndev, tunnel_id);
	return NSS_CAPWAPMGR_FAILURE_INVALID_DTLS_CFG;
}
EXPORT_SYMBOL(nss_capwapmgr_dtls_rekey_tx_cipher_update);

/*
 * nss_capwapmgr_dtls_rekey_rx_cipher_switch()
 *	Switch the rx cipher key for an DTLS tunnel
 */
nss_capwapmgr_status_t nss_capwapmgr_dtls_rekey_rx_cipher_switch(struct net_device *dev, uint8_t tunnel_id)
{
	struct nss_capwapmgr_tunnel *t;
	nss_capwapmgr_status_t status = NSS_CAPWAPMGR_SUCCESS;

	dev_hold(dev);
	status = nss_capwapmgr_get_tunnel(dev, tunnel_id, &t);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%px: can't find tunnel: %d\n", dev, tunnel_id);
		goto done;
	}

	if (!(t->capwap_rule.enabled_features & NSS_CAPWAPMGR_FEATURE_DTLS_ENABLED)) {
		nss_capwapmgr_warn("%px: tunnel does not enable DTLS: %d\n", dev, tunnel_id);
		status = NSS_CAPWAPMGR_FAILURE_BAD_PARAM;
		goto done;
	}

	/*
	 * Calling dtlsmgr for rekey switch
	 */
	if (!nss_dtlsmgr_session_switch_decap(t->dtls_dev)) {
		nss_capwapmgr_warn("%px: tunnel: %d rekey rx cipher switch failed\n", t->dtls_dev, tunnel_id);
		status = NSS_CAPWAPMGR_FAILURE_INVALID_DTLS_CFG;
	}

done:
	dev_put(dev);
	return status;
}
EXPORT_SYMBOL(nss_capwapmgr_dtls_rekey_rx_cipher_switch);

/*
 * nss_capwapmgr_dtls_rekey_tx_cipher_switch()
 *	Switch the tx cipher key for an DTLS tunnel
 */
nss_capwapmgr_status_t nss_capwapmgr_dtls_rekey_tx_cipher_switch(struct net_device *dev, uint8_t tunnel_id)
{
	struct nss_capwapmgr_tunnel *t;
	nss_capwapmgr_status_t status = NSS_CAPWAPMGR_SUCCESS;

	dev_hold(dev);
	status = nss_capwapmgr_get_tunnel(dev, tunnel_id, &t);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%px: can't find tunnel: %d\n", dev, tunnel_id);
		goto done;
	}

	if (!(t->capwap_rule.enabled_features & NSS_CAPWAPMGR_FEATURE_DTLS_ENABLED)) {
		nss_capwapmgr_warn("%px: tunnel does not enable DTLS: %d\n", dev, tunnel_id);
		status = NSS_CAPWAPMGR_FAILURE_BAD_PARAM;
		goto done;
	}

	/*
	 * Calling dtlsmgr for rekey switch
	 */
	if (!nss_dtlsmgr_session_switch_encap(t->dtls_dev)) {
		nss_capwapmgr_warn("%px: tunnel: %d rekey tx cipher switch failed\n", t->dtls_dev, tunnel_id);
		status = NSS_CAPWAPMGR_FAILURE_INVALID_DTLS_CFG;
	}

done:
	dev_put(dev);
	return status;
}
EXPORT_SYMBOL(nss_capwapmgr_dtls_rekey_tx_cipher_switch);

/*
 * nss_capwapmgr_change_version()
 *	Change CAPWAP version
 */
nss_capwapmgr_status_t nss_capwapmgr_change_version(struct net_device *dev, uint8_t tunnel_id, uint8_t ver)
{
	struct nss_capwapmgr_priv *priv;
	struct nss_capwap_msg capwapmsg;
	struct nss_capwapmgr_tunnel *t = NULL;
	nss_capwapmgr_status_t status = NSS_CAPWAPMGR_SUCCESS;

	dev_hold(dev);
	status = nss_capwapmgr_get_tunnel(dev, tunnel_id, &t);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%px: can't find tunnel: %d\n", dev, tunnel_id);
		goto done;
	}

	if (ver > NSS_CAPWAP_VERSION_V2) {
		nss_capwapmgr_warn("%px: un-supported Version: %d\n", dev, ver);
		status = NSS_CAPWAPMGR_FAILURE_INVALID_VERSION;
		goto done;
	}

	priv = netdev_priv(dev);

	/*
	 * Prepare the tunnel configuration parameter to send to NSS FW
	 */
	memset(&capwapmsg, 0, sizeof(struct nss_capwap_msg));

	/*
	 * Send CAPWAP data tunnel command to NSS
	 */
	nss_capwap_msg_init(&capwapmsg, t->if_num_inner, NSS_CAPWAP_MSG_TYPE_VERSION,
		sizeof(struct nss_capwap_version_msg), nss_capwapmgr_msg_event_receive, dev);
	capwapmsg.msg.version.version = ver;
	status = nss_capwapmgr_tx_msg_sync(priv->nss_ctx, dev, &capwapmsg);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%px: Update Path MTU Tunnel error : %d \n", dev, status);
	}

done:
	dev_put(dev);
	return status;
}
EXPORT_SYMBOL(nss_capwapmgr_change_version);

/*
 * nss_capwapmgr_enable_tunnel()
 *	API for enabling a data tunnel
 */
nss_capwapmgr_status_t nss_capwapmgr_enable_tunnel(struct net_device *dev, uint8_t tunnel_id)
{
	struct nss_capwapmgr_priv *priv;
	struct nss_capwapmgr_tunnel *t = NULL;
	nss_capwapmgr_status_t status = NSS_CAPWAPMGR_SUCCESS;

	dev_hold(dev);
	status = nss_capwapmgr_get_tunnel(dev, tunnel_id, &t);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%px: can't find tunnel: %d\n", dev, tunnel_id);
		goto done;
	}

	if (t->tunnel_state & NSS_CAPWAPMGR_TUNNEL_STATE_ENABLED) {
		nss_capwapmgr_warn("%px: tunnel %d is already enabled\n", dev, tunnel_id);
		status = NSS_CAPWAPMGR_FAILURE_TUNNEL_ENABLED;
		goto done;
	}

	priv = netdev_priv(dev);
	nss_capwapmgr_info("%px: Inner:%d Outer:%d. Tunnel enable is being called\n", dev, t->if_num_inner, t->if_num_outer);

	status = nss_capwapmgr_tx_msg_enable_tunnel(priv->nss_ctx, dev, t->if_num_inner,t->if_num_outer);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		goto done;
	}

	status = nss_capwapmgr_tx_msg_enable_tunnel(priv->nss_ctx, dev, t->if_num_outer,t->if_num_inner);
	if(status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_tunnel_action(priv->nss_ctx, dev, t->if_num_inner,NSS_CAPWAP_MSG_TYPE_DISABLE_TUNNEL);
		goto done;
	}

	t->tunnel_state |= NSS_CAPWAPMGR_TUNNEL_STATE_ENABLED;

done:
	dev_put(dev);
	return status;
}
EXPORT_SYMBOL(nss_capwapmgr_enable_tunnel);

/*
 * nss_capwapmgr_disable_tunnel()
 *	API for disabling a data tunnel
 */
nss_capwapmgr_status_t nss_capwapmgr_disable_tunnel(struct net_device *dev, uint8_t tunnel_id)
{
	struct nss_capwapmgr_priv *priv;
	struct nss_capwapmgr_tunnel *t = NULL;
	nss_capwapmgr_status_t status = NSS_CAPWAPMGR_SUCCESS;

	dev_hold(dev);
	status = nss_capwapmgr_get_tunnel(dev, tunnel_id, &t);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%px: can't find tunnel: %d\n", dev, tunnel_id);
		goto done;
	}

	if (!(t->tunnel_state & NSS_CAPWAPMGR_TUNNEL_STATE_ENABLED)) {
		nss_capwapmgr_warn("%px: tunnel %d is already disabled\n", dev, tunnel_id);
		status = NSS_CAPWAPMGR_FAILURE_TUNNEL_DISABLED;
		goto done;
	}

	priv = netdev_priv(dev);
	nss_capwapmgr_info("%px: Inner:%d Outer:%d. Tunnel disable is being called\n", dev, t->if_num_inner, t->if_num_outer);

	status = nss_capwapmgr_tunnel_action(priv->nss_ctx, dev, t->if_num_inner,NSS_CAPWAP_MSG_TYPE_DISABLE_TUNNEL);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		status = NSS_CAPWAPMGR_FAILURE_TUNNEL_DISABLED;
		nss_capwapmgr_warn("%px: tunnel %d disable failed\n", dev, tunnel_id);
		goto done;
	}

	status = nss_capwapmgr_tunnel_action(priv->nss_ctx, dev, t->if_num_outer,NSS_CAPWAP_MSG_TYPE_DISABLE_TUNNEL);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%px: tunnel %d disable failed\n", dev, tunnel_id);
		nss_capwapmgr_tx_msg_enable_tunnel(priv->nss_ctx, dev, t->if_num_inner, t->if_num_outer);
		goto done;
	}

	t->tunnel_state &= ~NSS_CAPWAPMGR_TUNNEL_STATE_ENABLED;

done:
	dev_put(dev);
	return status;
}
EXPORT_SYMBOL(nss_capwapmgr_disable_tunnel);

/*
 * nss_capwapmgr_ipv4_tunnel_create()
 *	API for creating IPv4 and CAPWAP rule.
 */
nss_capwapmgr_status_t nss_capwapmgr_ipv4_tunnel_create(struct net_device *dev, uint8_t tunnel_id,
			struct nss_ipv4_create *ip_rule, struct nss_capwap_rule_msg *capwap_rule, struct nss_dtlsmgr_config *dtls_data)
{
	return nss_capwapmgr_tunnel_create_common(dev, tunnel_id, ip_rule, NULL, capwap_rule, dtls_data);
}
EXPORT_SYMBOL(nss_capwapmgr_ipv4_tunnel_create);

/*
 * nss_capwapmgr_ipv6_tunnel_create()
 *	API for creating IPv6 and CAPWAP rule.
 */
nss_capwapmgr_status_t nss_capwapmgr_ipv6_tunnel_create(struct net_device *dev, uint8_t tunnel_id,
			struct nss_ipv6_create *ip_rule, struct nss_capwap_rule_msg *capwap_rule, struct nss_dtlsmgr_config *dtls_data)
{
	return nss_capwapmgr_tunnel_create_common(dev, tunnel_id, NULL, ip_rule, capwap_rule, dtls_data);
}
EXPORT_SYMBOL(nss_capwapmgr_ipv6_tunnel_create);

/*
 * nss_capwapmgr_tunnel_destroy()
 *	API for destroying a tunnel. CAPWAP tunnel must be first disabled.
 */
nss_capwapmgr_status_t nss_capwapmgr_tunnel_destroy(struct net_device *dev, uint8_t tunnel_id)
{
	struct nss_capwap_tunnel_stats stats;
	struct nss_capwapmgr_priv *priv;
	struct nss_capwapmgr_tunnel *t = NULL;
	nss_tx_status_t nss_status = NSS_TX_SUCCESS;
	uint32_t if_num_inner, if_num_outer;
	nss_capwapmgr_status_t status = NSS_CAPWAPMGR_SUCCESS;
	ppe_vp_status_t vp_status;
	uint32_t outer_trustsec_enabled;

	dev_hold(dev);
	status = nss_capwapmgr_get_tunnel(dev, tunnel_id, &t);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%px: can't find tunnel: %d\n", dev, tunnel_id);
		goto done;
	}

	if (!(t->tunnel_state & NSS_CAPWAPMGR_TUNNEL_STATE_CONFIGURED)) {
		nss_capwapmgr_warn("%px: tunnel %d is not configured yet\n", dev, tunnel_id);
		status = NSS_CAPWAPMGR_FAILURE_TUNNEL_NOT_CFG;
		goto done;
	}

	/*
	 * We don't allow destroy operation on tunnel if it's still enabled.
	 */
	if (t->tunnel_state & NSS_CAPWAPMGR_TUNNEL_STATE_ENABLED) {
		nss_capwapmgr_warn("%px: no destroy allowed for an enabled tunnel: %d\n", dev, tunnel_id);
		status = NSS_CAPWAPMGR_FAILURE_TUNNEL_ENABLED;
		goto done;
	}

	if (!(t->capwap_rule.l3_proto == NSS_CAPWAP_TUNNEL_IPV4 ||
			t->capwap_rule.l3_proto == NSS_CAPWAP_TUNNEL_IPV6)) {
		nss_capwapmgr_warn("%px: tunnel %d: wrong argument for l3_proto\n", dev, tunnel_id);
		status = NSS_CAPWAPMGR_FAILURE_INVALID_L3_PROTO;
		goto done;
	}

	if (!(t->capwap_rule.which_udp == NSS_CAPWAP_TUNNEL_UDP ||
			t->capwap_rule.which_udp == NSS_CAPWAP_TUNNEL_UDPLite)) {
		nss_capwapmgr_warn("%px: tunnel %d: wrong argument for which_udp(%d)\n", dev, tunnel_id, t->capwap_rule.which_udp);
		status = NSS_CAPWAPMGR_FAILURE_INVALID_UDP_PROTO;
		goto done;
	}

	outer_trustsec_enabled = t->capwap_rule.enabled_features & NSS_CAPWAPMGR_FEATURE_OUTER_TRUSTSEC_ENABLED;
	priv = netdev_priv(dev);
	nss_capwapmgr_info("%px: %d: tunnel destroy is being called\n", dev, tunnel_id);

	if_num_inner = t->if_num_inner;
	if_num_outer = t->if_num_outer;

	if (priv->if_num_to_tunnel_id[if_num_inner] != tunnel_id) {
		nss_capwapmgr_warn("%px: %d: tunnel_id %d didn't match with tunnel_id :%d\n",
			dev, if_num_inner, tunnel_id, priv->if_num_to_tunnel_id[if_num_inner]);
		status = NSS_CAPWAPMGR_FAILURE_BAD_PARAM;
		goto done;
	}

	if (priv->if_num_to_tunnel_id[if_num_outer] != tunnel_id) {
		nss_capwapmgr_warn("%px: %d: tunnel_id %d didn't match with tunnel_id :%d\n",
			dev, if_num_outer, tunnel_id, priv->if_num_to_tunnel_id[if_num_outer]);
		status = NSS_CAPWAPMGR_FAILURE_BAD_PARAM;
		goto done;
	}

	if (nss_capwap_get_stats(if_num_inner, &stats)) {
		nss_capwapmgr_tunnel_save_stats(&global.tunneld_stats, &stats);
	}

	if (nss_capwap_get_stats(if_num_outer, &stats)) {
		nss_capwapmgr_tunnel_save_stats(&global.tunneld_stats, &stats);
	}

	/*
	 * Destroy IP rule first.
	 */
	if (t->tunnel_state & NSS_CAPWAPMGR_TUNNEL_STATE_IPRULE_CONFIGURED) {
		if (t->capwap_rule.l3_proto == NSS_CAPWAP_TUNNEL_IPV4) {
			if (outer_trustsec_enabled) {
				status = nss_capwapmgr_trustsec_rule_destroy_v4(t);
			} else {
				status = nss_capwapmgr_ppe_destroy_ipv4_rule(t);
			}
		} else {
			if (outer_trustsec_enabled) {
				status = nss_capwapmgr_trustsec_rule_destroy_v6(t);
			} else {
				status = nss_capwapmgr_ppe_destroy_ipv6_rule(t);
			}
		}
	}

	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%px: Unconfigure IP rule failed for tunnel: %d\n", dev, tunnel_id);
		goto done;
	}

	/*
	 * Destroy CAPWAP rule now.
	 */
	status = nss_capwapmgr_tunnel_action(priv->nss_ctx, dev, if_num_outer, NSS_CAPWAP_MSG_TYPE_UNCFG_RULE);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%px: %d: Unconfigure Inner CAPWAP rule failed for tunnel : %d\n",
			dev, if_num_outer, tunnel_id);
	}

	status = nss_capwapmgr_tunnel_action(priv->nss_ctx, dev, if_num_inner, NSS_CAPWAP_MSG_TYPE_UNCFG_RULE);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%px: %d: Unconfigure Outer CAPWAP rule failed for tunnel : %d\n",
			dev, if_num_inner, tunnel_id);
	}

	/*
	 * If trustsec is enabled, unconfigure the trustsec tx rule in NSS-FW.
	 */
	if (outer_trustsec_enabled) {
		nss_trustsec_tx_unconfigure_sgt(if_num_outer, t->capwap_rule.outer_sgt_value);
	}

	nss_capwapmgr_unregister_with_nss(if_num_outer);
	nss_capwapmgr_unregister_with_nss(if_num_inner);

	/*
	 * Deallocate dynamic interface
	 */
	if ((t->tunnel_state & NSS_CAPWAPMGR_TUNNEL_STATE_OUTER_ALLOCATED)) {
		nss_status = nss_dynamic_interface_dealloc_node(if_num_outer, NSS_DYNAMIC_INTERFACE_TYPE_CAPWAP_OUTER);
		if (nss_status != NSS_TX_SUCCESS) {
			nss_capwapmgr_warn("%px: %d: Dealloc of dynamic interface failed for tunnel : %d\n",
				dev, if_num_outer, tunnel_id);
		}

		t->tunnel_state &= ~NSS_CAPWAPMGR_TUNNEL_STATE_OUTER_ALLOCATED;
	}

	if ((t->tunnel_state & NSS_CAPWAPMGR_TUNNEL_STATE_INNER_ALLOCATED)) {
		nss_status = nss_dynamic_interface_dealloc_node(if_num_inner, NSS_DYNAMIC_INTERFACE_TYPE_CAPWAP_HOST_INNER);
		if (nss_status != NSS_TX_SUCCESS) {
			nss_capwapmgr_warn("%px: %d: Dealloc of dynamic interface failed for tunnel : %d\n",
				dev, if_num_inner, tunnel_id);
		}

		t->tunnel_state &= ~NSS_CAPWAPMGR_TUNNEL_STATE_INNER_ALLOCATED;
	}

	/*
	 * Free the VP interface associated with the tunnel.
	 */
	if (!outer_trustsec_enabled) {
		vp_status = ppe_vp_free(t->vp_num_decap);
		if (vp_status != PPE_VP_STATUS_SUCCESS) {
			nss_capwapmgr_warn("%px: VP Number %d: Failed to free the associated VP for tunnel : %d\n",
				dev, t->vp_num_decap, tunnel_id);
			status = NSS_CAPWAPMGR_FAILURE_VP_FREE;
			goto done;
		}
	}

	vp_status = ppe_vp_free(t->vp_num_encap);
	if (vp_status != PPE_VP_STATUS_SUCCESS) {
		nss_capwapmgr_warn("%px: VP Number %d: Failed to free the associated VP for tunnel : %d\n",
			dev, t->vp_num_encap, tunnel_id);
		status = NSS_CAPWAPMGR_FAILURE_VP_FREE;
		goto done;
	}

	/*
	 * Destroy DTLS node if there is one associated to this tunnel
	 */
	if (t->capwap_rule.dtls_inner_if_num) {
		if (nss_dtlsmgr_session_destroy(t->dtls_dev) != NSS_DTLSMGR_OK) {
			nss_capwapmgr_warn("%px: failed to destroy DTLS session", t->dtls_dev);
		}
	}

	/*
	 * Update the state flag to say the tunnel is unconfigured.
	 */
	t->tunnel_state &= ~NSS_CAPWAPMGR_TUNNEL_STATE_CONFIGURED;

	/*
	 * Free the internal net device associated with the tunnel.
	 */
	free_netdev(t->internal_dev_decap);
	free_netdev(t->internal_dev_encap);

	priv->if_num_to_tunnel_id[if_num_inner] = -1;
	priv->if_num_to_tunnel_id[if_num_outer] = -1;

	memset(t, 0, sizeof(struct nss_capwapmgr_tunnel));

	t->if_num_inner = -1;
	t->if_num_outer = -1;
	nss_capwapmgr_info("%px: Tunnel %d is destroyed\n", dev , tunnel_id);
	status = NSS_CAPWAPMGR_SUCCESS;

done:
	dev_put(dev);
	return status;
}
EXPORT_SYMBOL(nss_capwapmgr_tunnel_destroy);

/*
 * nss_capwapmgr_add_flow_rule()
 *	Send a capwap flow rule add message to NSS core.
 */
nss_capwapmgr_status_t nss_capwapmgr_add_flow_rule(struct net_device *dev, uint8_t tunnel_id, struct nss_capwapmgr_flow_info *flow_info)
{
	return nss_capwapmgr_flow_rule_action(dev, tunnel_id, NSS_CAPWAP_MSG_TYPE_FLOW_RULE_ADD, flow_info);
}
EXPORT_SYMBOL(nss_capwapmgr_add_flow_rule);

/*
 * nss_capwapmgr_del_flow_rule()
 *	Send a capwap flow rule del message to NSS core.
 */
nss_capwapmgr_status_t nss_capwapmgr_del_flow_rule(struct net_device *dev, uint8_t tunnel_id, struct nss_capwapmgr_flow_info *flow_info)
{
	return nss_capwapmgr_flow_rule_action(dev, tunnel_id, NSS_CAPWAP_MSG_TYPE_FLOW_RULE_DEL, flow_info);
}
EXPORT_SYMBOL(nss_capwapmgr_del_flow_rule);

/*
 * nss_capwapmgr_tunnel_stats()
 *	Gets tunnel stats from netdev
 */
nss_capwapmgr_status_t nss_capwapmgr_tunnel_stats(struct net_device *dev,
		uint8_t tunnel_id, struct nss_capwap_tunnel_stats *stats)
{
	struct nss_capwapmgr_tunnel *t = NULL;
	struct nss_capwap_tunnel_stats stats_temp;
	nss_capwapmgr_status_t status = NSS_CAPWAPMGR_SUCCESS;

	if (!stats) {
		nss_capwapmgr_warn("%px: invalid rtnl structure\n", dev);
		return NSS_CAPWAPMGR_FAILURE_BAD_PARAM;
	}

	dev_hold(dev);
	status = nss_capwapmgr_get_tunnel(dev, tunnel_id, &t);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_capwapmgr_warn("%px: can't find tunnel: %d\n", dev, tunnel_id);
		goto done;
	}

	if (!(t->tunnel_state & NSS_CAPWAPMGR_TUNNEL_STATE_CONFIGURED)) {
		nss_capwapmgr_trace("%px: tunnel: %d not configured yet\n", dev, tunnel_id);
		status = NSS_CAPWAPMGR_FAILURE_TUNNEL_NOT_CFG;
		goto done;
	}

	/*
	 * Copy the inner interface stats.
	 */
	if (nss_capwap_get_stats(t->if_num_inner, &stats_temp) == false) {
		nss_capwapmgr_warn("%px: tunnel %d not ready yet\n", dev, tunnel_id);
		status = NSS_CAPWAPMGR_FAILURE_NOT_READY;
		goto done;
	}

	stats->dtls_pkts += stats_temp.dtls_pkts;
	stats->tx_segments += stats_temp.tx_segments;
	stats->tx_queue_full_drops += stats_temp.tx_queue_full_drops;
	stats->tx_mem_failure_drops += stats_temp.tx_mem_failure_drops;
	stats->tx_dropped_sg_ref += stats_temp.tx_dropped_sg_ref;
	stats->tx_dropped_ver_mis += stats_temp.tx_dropped_ver_mis;
	stats->tx_dropped_hroom += stats_temp.tx_dropped_hroom;
	stats->tx_dropped_dtls += stats_temp.tx_dropped_dtls;
	stats->tx_dropped_nwireless += stats_temp.tx_dropped_nwireless;

	/*
	 * Pnode tx stats for Inner node.
	 */
	stats->pnode_stats.tx_packets += stats_temp.pnode_stats.tx_packets;
	stats->pnode_stats.tx_bytes += stats_temp.pnode_stats.tx_bytes;
	stats->tx_dropped_inner += stats_temp.tx_dropped_inner;

	/*
	 * Copy the outer interface stats.
	 */
	if (nss_capwap_get_stats(t->if_num_outer, &stats_temp) == false) {
		nss_capwapmgr_warn("%px: tunnel %d not ready yet\n", dev, tunnel_id);
		status = NSS_CAPWAPMGR_FAILURE_NOT_READY;
		goto done;
	}

	stats->rx_segments += stats_temp.rx_segments;
	stats->dtls_pkts += stats_temp.dtls_pkts;
	stats->rx_dup_frag += stats_temp.rx_dup_frag;
	stats->rx_oversize_drops += stats_temp.rx_oversize_drops;
	stats->rx_frag_timeout_drops += stats_temp.rx_frag_timeout_drops;
	stats->rx_n2h_drops += stats_temp.rx_n2h_drops;
	stats->rx_n2h_queue_full_drops += stats_temp.rx_n2h_queue_full_drops;
	stats->rx_mem_failure_drops += stats_temp.rx_mem_failure_drops;
	stats->rx_csum_drops += stats_temp.rx_csum_drops;
	stats->rx_malformed += stats_temp.rx_malformed;
	stats->rx_frag_gap_drops += stats_temp.rx_frag_gap_drops;

	/*
	 * Pnode rx stats for outer node.
	 */
	stats->pnode_stats.rx_packets += stats_temp.pnode_stats.rx_packets;
	stats->pnode_stats.rx_bytes += stats_temp.pnode_stats.rx_bytes;
	stats->pnode_stats.rx_dropped += stats_temp.pnode_stats.rx_dropped;

done:
	dev_put(dev);
	return status;
}
EXPORT_SYMBOL(nss_capwapmgr_tunnel_stats);

/*
 * nss_capwapmgr_dscp_rule_destroy()
 *	API to destroy previously created DSCP rule.
 */
nss_capwapmgr_status_t nss_capwapmgr_dscp_rule_destroy(uint8_t id)
{
	sw_error_t rv;
	fal_qos_cosmap_t cosmap;
	struct nss_capwapmgr_acl *acl_rule;
	uint8_t dev_id = NSS_CAPWAPMGR_DEV_ID;
	uint8_t rule_nr = NSS_CAPWAPMGR_RULE_NR;
	uint8_t group_id = NSS_CAPWAPMGR_GROUP_ID;
	uint8_t i, j, list_id, v4_rule_id, v6_rule_id, dscp_value, dscp_mask;
	atomic_t *acl_req_count = &global.dscp_acl_rule_create_req;

	for (i = 0; i < NSS_CAPWAPMGR_ACL_DSCP_LIST_CNT; i++) {
		for (j = 0; j < NSS_CAPWAPMGR_ACL_DSCP_RULES_PER_LIST; j++) {
			if (global.acl_list[i].rule[j].uid == id) {
				acl_rule = &global.acl_list[i].rule[j];
				goto found;
			}
		}
	}

	nss_capwapmgr_warn("Invalid id: %u\n", id);
	return NSS_CAPWAPMGR_FAILURE_DSCP_RULE_ID_INVALID;

found:
	if (!acl_rule->in_use) {
		nss_capwapmgr_warn("Rule matching id: %d not in use\n", id);
		return NSS_CAPWAPMGR_FAILURE_DSCP_RULE_ID_NOT_IN_USE;
	}

	dscp_value = acl_rule->dscp_value;
	dscp_mask = acl_rule->dscp_mask;

	/*
	 * Reset all classification fields on cosmap table.
	 */
	cosmap.internal_pcp = 0;
	cosmap.internal_dei = 0;
	cosmap.internal_pri = 0;
	cosmap.internal_dscp = 0;
	cosmap.internal_dp = 0;

	for (i = 0; i < NSS_CAPWAPMGR_DSCP_MAX; i++) {
		if ((i & dscp_mask) != dscp_value) {
			continue;
		}

		nss_capwapmgr_trace("dscpmap: resetting for dscp %u\n", i);
		rv = fal_qos_cosmap_dscp_set(dev_id, group_id, i, &cosmap);
		if (rv != SW_OK) {
			nss_capwapmgr_warn("Failed to reset cosmap for dscp %d - code: %d\n", i, rv);
			return NSS_CAPWAPMGR_FAILURE_DSCP_RULE_DELETE_FAILED;
		}
	}

	/*
	 * Since we use 2 ACL entries per rule (i.e. v4/v6) we multiply by
	 * two to get rule_ids.
	 */
	v4_rule_id = acl_rule->rule_id * 2;
	v6_rule_id = v4_rule_id + 1;
	list_id = NSS_CAPWAPMGR_ACL_DSCP_LIST_ID + acl_rule->list_id;

	rv = fal_acl_rule_delete(dev_id, list_id, v6_rule_id, rule_nr);
	if (rv != SW_OK) {
		nss_capwapmgr_warn("Failed to del ACL v6_rule %d from list %d - code: %d\n", v6_rule_id, list_id, rv);
		return NSS_CAPWAPMGR_FAILURE_DSCP_RULE_DELETE_FAILED;
	}

	rv = fal_acl_rule_delete(dev_id, list_id, v4_rule_id, rule_nr);
	if (rv != SW_OK) {
		nss_capwapmgr_warn("Failed to del ACL v4_rule %d from list %d - code: %d\n", v4_rule_id, list_id, rv);
		return NSS_CAPWAPMGR_FAILURE_DSCP_RULE_DELETE_FAILED;
	}

	acl_rule->in_use = false;

	if (atomic_dec_and_test(acl_req_count)) {
		nss_capwapmgr_dscp_acl_deinit();
	}

	return NSS_CAPWAPMGR_SUCCESS;
}
EXPORT_SYMBOL(nss_capwapmgr_dscp_rule_destroy);

/*
 * nss_capwapmgr_dscp_rule_create()
 *	API to prioritize packets based on DSCP.
 */
nss_capwapmgr_status_t nss_capwapmgr_dscp_rule_create(uint8_t dscp_value, uint8_t dscp_mask, uint8_t pri, uint8_t *id)
{
	sw_error_t rv;
	fal_qos_cosmap_t cosmap;
	fal_qos_cosmap_t *orig_cosmap;
	fal_acl_rule_t *acl_rule;
	uint8_t dev_id = NSS_CAPWAPMGR_DEV_ID;
	uint8_t group_id = NSS_CAPWAPMGR_GROUP_ID;
	uint8_t rule_nr = NSS_CAPWAPMGR_RULE_NR;
	uint8_t list_id, v4_rule_id, v6_rule_id;
	uint8_t lid, rid, i, j;
	int8_t err, fail_dscp;
	int8_t uid = -1;
	uint32_t v_port;
	atomic_t *acl_req_count = &global.dscp_acl_rule_create_req;

	if (atomic_inc_return(acl_req_count) == 1) {
		if(!nss_capwapmgr_dscp_acl_init()) {
			return NSS_CAPWAPMGR_FAILURE_DSCP_ACL_INIT;
		}
	}

	nss_capwapmgr_info("Setting priority %u for dscp %u mask %u\n", pri, dscp_value, dscp_mask);

	orig_cosmap = kzalloc(NSS_CAPWAPMGR_DSCP_MAX * sizeof(*orig_cosmap), GFP_KERNEL);
	if (!orig_cosmap) {
		nss_capwapmgr_warn("Failed to alloc memory for orig_cosmap\n");
		err = NSS_CAPWAPMGR_FAILURE_MEM_UNAVAILABLE;
		goto fail;
	}

	acl_rule = kzalloc(sizeof(*acl_rule), GFP_KERNEL);
	if (!acl_rule) {
		nss_capwapmgr_warn("Failed to alloc memory for acl_rule\n");
		kfree(orig_cosmap);
		err = NSS_CAPWAPMGR_FAILURE_MEM_UNAVAILABLE;
		goto fail;
	}

	/*
	 * Get an empty acl rule.
	 */
	for (i = 0; i < NSS_CAPWAPMGR_ACL_DSCP_LIST_CNT; i++) {
		for (j = 0; j < NSS_CAPWAPMGR_ACL_DSCP_RULES_PER_LIST; j++) {
			if (global.acl_list[i].rule[j].in_use) {
				continue;
			}

			uid = global.acl_list[i].rule[j].uid;
			rid = global.acl_list[i].rule[j].rule_id;
			lid = global.acl_list[i].rule[j].list_id;
			goto found;
		}
	}

found:
	if (uid < 0) {
		nss_capwapmgr_warn("No free ACL rules available\n");
		err = NSS_CAPWAPMGR_FAILURE_ACL_UNAVAILABLE;
		goto fail1;
	};

	/*
	 * Since we use 2 ACL entries per rule (i.e. v4/v6) we multiply rid by
	 * two to get rule_id.
	 */
	v4_rule_id = rid * 2;
	v6_rule_id = v4_rule_id + 1;
	list_id = NSS_CAPWAPMGR_ACL_DSCP_LIST_ID + lid;

	nss_capwapmgr_info("Using ACL rules: %d & %d from list: %d\n", v4_rule_id, v6_rule_id, list_id);

	/*
	 * Prioritize packets with the dscp value. For trustsec packets, we need to specify
	 * the location of the dscp value with ACL configuration.
	 * ACL rule always start from the L2 header. It will be trustsec header for our case.
	 * We need two user defined profile to set beginning of the
	 * Profile 0 is for start of the ethernet type.
	 * Profile 1 is for the start of the ip header.
	 */
	rv = fal_acl_udf_profile_set(dev_id, FAL_ACL_UDF_NON_IP, 0, FAL_ACL_UDF_TYPE_L3, NSS_CAPWAPMGR_ETH_HDR_OFFSET);
	if (rv != SW_OK) {
		nss_capwapmgr_warn("Failed to create UDF 0 Map - code: %d\n", rv);
		err = NSS_CAPWAPMGR_FAILURE_CREATE_UDF_PROFILE;
		goto fail1;
	}

	rv = fal_acl_udf_profile_set(dev_id, FAL_ACL_UDF_NON_IP, 1, FAL_ACL_UDF_TYPE_L3, NSS_CAPWAPMGR_IPV4_OFFSET);
	if (rv != SW_OK) {
		nss_capwapmgr_warn("Failed to create UDF 1 Map - code: %d\n", rv);
		err = NSS_CAPWAPMGR_FAILURE_CREATE_UDF_PROFILE;
		goto fail1;
	}

	acl_rule->rule_type = FAL_ACL_RULE_MAC;

	/*
	 * Sets valid flags for the acl rule.
	 * Following rules are valid:
	 * - Ethernet type
	 * - User defined field 0. Correspond to ethernet type (ipv4/ipv6)
	 * - User defined field 1. Correspond to DSCP value (dscp_value)
	 */
	FAL_FIELD_FLG_SET(acl_rule->field_flg, FAL_ACL_FIELD_MAC_ETHTYPE);
	FAL_FIELD_FLG_SET(acl_rule->field_flg, FAL_ACL_FIELD_UDF0);
	FAL_FIELD_FLG_SET(acl_rule->field_flg, FAL_ACL_FIELD_UDF1);
	FAL_ACTION_FLG_SET(acl_rule->action_flg, FAL_ACL_ACTION_PERMIT);
	FAL_ACTION_FLG_SET(acl_rule->action_flg, FAL_ACL_ACTION_ENQUEUE_PRI);

	/*
	 * Redirect trustsec + dscp packets to the trustsec VP
	 */
	FAL_ACTION_FLG_SET(acl_rule->action_flg, FAL_ACL_ACTION_REDPT);
	v_port = FAL_PORT_ID(FAL_PORT_TYPE_VPORT, global.trustsec_rx_vp_num);
	acl_rule->ports = v_port;

	/*
	 * Set common parameters for ipv4/ipv6
	 *
	 * TODO: Once the host dp trustsec patch is merged,
	 * get the trustsec ethertype from there.
	 */
	acl_rule->ethtype_val = NSS_CAPWAPMGR_ETH_TYPE_TRUSTSEC;
	acl_rule->ethtype_mask = NSS_CAPWAPMGR_ETH_TYPE_MASK;
	acl_rule->udf0_op = FAL_ACL_FIELD_MASK;
	acl_rule->udf1_op = FAL_ACL_FIELD_MASK;
	acl_rule->enqueue_pri = pri;

	/*
	 * Create ACL rule for IPv4
	 */
	acl_rule->udf0_val = NSS_CAPWAPMGR_ETH_TYPE_IPV4;
	acl_rule->udf0_mask = NSS_CAPWAPMGR_ETH_TYPE_MASK;
	acl_rule->udf1_val = dscp_value << NSS_CAPWAPMGR_DSCP_MASK_IPV4_SHIFT;
	acl_rule->udf1_mask = dscp_mask << NSS_CAPWAPMGR_DSCP_MASK_IPV4_SHIFT;

	rv = fal_acl_rule_query(dev_id, list_id, v4_rule_id, acl_rule);
	if (rv != SW_NOT_FOUND) {
		nss_capwapmgr_warn("ACL rule already exist for list_id: %u, rule_id: %u - code: %d\n", list_id, v4_rule_id, rv);
		err = NSS_CAPWAPMGR_FAILURE_ACL_RULE_ALREADY_EXIST;
		goto fail1;
	}

	rv = fal_acl_list_unbind(dev_id, list_id, FAL_ACL_DIREC_IN, FAL_ACL_BIND_PORTBITMAP, NSS_CAPWAPMGR_BIND_BITMAP);
	if (rv != SW_OK) {
		nss_capwapmgr_warn("Failed to unbind list: %d - code: %d\n", list_id, rv);
		err = NSS_CAPWAPMGR_FAILURE_ADD_ACL_RULE;
		goto fail1;
	}

	rv = fal_acl_rule_add(dev_id, list_id, v4_rule_id, rule_nr, acl_rule);
	if (rv != SW_OK) {
		nss_capwapmgr_warn("Failed to add ACL v4_rule: %d - code: %d\n", rv, v4_rule_id);
		err = NSS_CAPWAPMGR_FAILURE_ADD_ACL_RULE;
		goto fail1;
	}

	/*
	 * Create ACL rule for IPv6
	 */
	acl_rule->udf0_val = NSS_CAPWAPMGR_ETH_TYPE_IPV6;
	acl_rule->udf0_mask = NSS_CAPWAPMGR_ETH_TYPE_MASK;
	acl_rule->udf1_val = dscp_value << NSS_CAPWAPMGR_DSCP_MASK_IPV6_SHIFT;
	acl_rule->udf1_mask = dscp_mask << NSS_CAPWAPMGR_DSCP_MASK_IPV6_SHIFT;

	rv = fal_acl_rule_add(dev_id, list_id, v6_rule_id, rule_nr, acl_rule);
	if (rv != SW_OK) {
		nss_capwapmgr_warn("Failed to add ACL v6_rule: %d - code: %d\n", rv, v6_rule_id);
		err = NSS_CAPWAPMGR_FAILURE_ADD_ACL_RULE;
		goto fail2;
	}

	/*
	 * Bind list to all ethernet ports
	 */
	rv = fal_acl_list_bind(dev_id, list_id, FAL_ACL_DIREC_IN, FAL_ACL_BIND_PORTBITMAP, NSS_CAPWAPMGR_BIND_BITMAP);
	if (rv != SW_OK) {
		nss_capwapmgr_warn("Failed to bind ACL list: %d - code: %d\n", list_id, rv);
		err = NSS_CAPWAPMGR_FAILURE_BIND_ACL_LIST;
		goto fail3;
	}

	/*
	 * Set ACL as in_use and save dscp value and mask.
	 */
	global.acl_list[lid].rule[rid].in_use = true;
	global.acl_list[lid].rule[rid].dscp_value = dscp_value;
	global.acl_list[lid].rule[rid].dscp_mask = dscp_mask;

	/*
	 * Prioritize packets with the dscp value is dscp_value for non trustsec packets.
	 * These packets do not require any ACL Rule.
	 */
	cosmap.internal_pcp = 0;
	cosmap.internal_dei = 0;
	cosmap.internal_pri = pri;
	cosmap.internal_dscp = 0;
	cosmap.internal_dp = 0;
	for (i = 0; i < NSS_CAPWAPMGR_DSCP_MAX; i++) {
		if ((i & dscp_mask) != dscp_value) {
			continue;
		}

		rv = fal_qos_cosmap_dscp_get(dev_id, group_id, i, &orig_cosmap[i]);
		if (rv != SW_OK) {
			nss_capwapmgr_warn("dscpmap: failed to get cosmap for dscp %d\n", i);
			err = NSS_CAPWAPMGR_FAILURE_CONFIGURE_DSCP_MAP;
			goto fail4;
		}

		nss_capwapmgr_trace("dscpmap: setting priority %u for dscp %u\n", pri, i);
		rv = fal_qos_cosmap_dscp_set(dev_id, group_id, i, &cosmap);
		if (rv != SW_OK) {
			nss_capwapmgr_warn("Failed to configure cosmap for dscp %d - code: %d\n", i, rv);
			err = NSS_CAPWAPMGR_FAILURE_CONFIGURE_DSCP_MAP;
			goto fail4;
		}
	}

	kfree(acl_rule);
	kfree(orig_cosmap);

	*id = uid;

	return NSS_CAPWAPMGR_SUCCESS;

fail4:
	fail_dscp = i;
	for (i = 0; i < fail_dscp; i++) {
		if ((i & dscp_mask) != dscp_value) {
			continue;
		}

		nss_capwapmgr_trace("dscpmap: resetting to priority %u for dscp %u\n", orig_cosmap[i].internal_pri, i);
		rv = fal_qos_cosmap_dscp_set(dev_id, group_id, i, &orig_cosmap[i]);
		if (rv != SW_OK) {
			nss_capwapmgr_warn("Failed to reset cosmap for dscp %d - code: %d\n", i, rv);
		}
	}

fail3:
	rv = fal_acl_rule_delete(dev_id, list_id, v6_rule_id, rule_nr);
	if (rv != SW_OK) {
		nss_capwapmgr_warn("Failed to del ACL v6_rule %d from list %d - code: %d\n", v6_rule_id, list_id, rv);
	}

fail2:
	rv = fal_acl_rule_delete(dev_id, list_id, v4_rule_id, rule_nr);
	if (rv != SW_OK) {
		nss_capwapmgr_warn("Failed to del ACL v4_rule %d from list %d - code: %d\n", v4_rule_id, list_id, rv);
	}

fail1:
	kfree(orig_cosmap);
	kfree(acl_rule);

fail:
	if (atomic_dec_and_test(acl_req_count)) {
		nss_capwapmgr_dscp_acl_deinit();
	}

	return err;
}
EXPORT_SYMBOL(nss_capwapmgr_dscp_rule_create);


#if defined(NSS_CAPWAPMGR_ONE_NETDEV)
/*
 * nss_capwapmgr_get_netdev()
 *	Returns net device used.
 */
struct net_device *nss_capwapmgr_get_netdev(void)
{
	return nss_capwapmgr_ndev;
}
EXPORT_SYMBOL(nss_capwapmgr_get_netdev);
#endif

/*
 * nss_capwapmgr_init_module()
 *	Tunnel CAPWAP module init function
 */
int __init nss_capwapmgr_init_module(void)
{
#ifdef CONFIG_OF
	/*
	 * If the node is not compatible, don't do anything.
	 */
	if (!of_find_node_by_name(NULL, "nss-common")) {
		return 0;
	}
#endif
	nss_capwapmgr_info("module (platform - IPQ9574, %s) loaded\n",
			   NSS_PPE_BUILD_ID);

#if defined(NSS_CAPWAPMGR_ONE_NETDEV)
	/*
	 * In this code, we create a single netdev for all the CAPWAP
	 * tunnels.
	 */
	nss_capwapmgr_ndev = nss_capwapmgr_netdev_create();
	if (!nss_capwapmgr_ndev) {
		nss_capwapmgr_warn("Couldn't create capwap interface\n");
		return -1;
	}
#endif
	if (!nss_capwapmgr_dentry_init()) {
		nss_capwapmgr_warn("Failed to create dentry for capwap\n");
	}

	register_netdevice_notifier(&nss_capwapmgr_netdev_notifier);
	memset(&global.tunneld_stats, 0, sizeof(struct nss_capwap_tunnel_stats));

	/*
	 * ppe2host is disabled by default.
	 */
	global.ppe2host = false;

	return 0;
}

/*
 * nss_capwapmgr_exit_module()
 *	Tunnel CAPWAP module exit function
 */
void __exit nss_capwapmgr_exit_module(void)
{
#if defined(NSS_CAPWAPMGR_ONE_NETDEV)
	struct nss_capwapmgr_priv *priv;
	uint8_t i;
#endif

#ifdef CONFIG_OF
	/*
	 * If the node is not compatible, don't do anything.
	 */
	if (!of_find_node_by_name(NULL, "nss-common")) {
		return;
	}
#endif

#if defined(NSS_CAPWAPMGR_ONE_NETDEV)
	priv = netdev_priv(nss_capwapmgr_ndev);
	for (i = 0; i < NSS_CAPWAPMGR_MAX_TUNNELS; i++) {
		(void) nss_capwapmgr_disable_tunnel(nss_capwapmgr_ndev, i);
		(void) nss_capwapmgr_tunnel_destroy(nss_capwapmgr_ndev, i);
	}
	kfree(priv->if_num_to_tunnel_id);
	kfree(priv->resp);
	kfree(priv->tunnel);
	unregister_netdev(nss_capwapmgr_ndev);
	free_netdev(nss_capwapmgr_ndev);

	nss_capwapmgr_ndev = NULL;
#endif
	if (global.capwap_dentry) {
		debugfs_remove_recursive(global.capwap_dentry);
	}

	unregister_netdevice_notifier(&nss_capwapmgr_netdev_notifier);

	nss_capwapmgr_trustsec_rx_vp_unconfig();
	nss_capwapmgr_info("module unloaded\n");
}

module_init(nss_capwapmgr_init_module);
module_exit(nss_capwapmgr_exit_module);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS CAPWAP manager");
