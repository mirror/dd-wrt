/*
 **************************************************************************
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
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
 * nss_ovpnmgr_tun.c
 */
#include <linux/etherdevice.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/if_tun.h>
#include <linux/ip.h>
#include <net/ip.h>
#include <net/udp.h>
#include <net/ip6_checksum.h>
#include <linux/ipv6.h>
#include <linux/crypto.h>

#include <nss_api_if.h>
#include <nss_qvpn.h>
#include "nss_ovpnmgr.h"
#include "nss_ovpnmgr_crypto.h"
#include "nss_ovpnmgr_tun.h"
#include "nss_ovpnmgr_app.h"
#include "nss_ovpnmgr_debugfs.h"
#include "nss_ovpnmgr_priv.h"
#include "nss_ovpnmgr_route.h"

#if NSS_OVPNMGR_DEBUG_ENABLE_PKT_DUMP
/*
 * nss_ovpnmgr_tun_pkt_dump()
 *	Dump packet.
 */
static void nss_ovpnmgr_tun_pkt_dump(struct sk_buff *skb, char *hdr, uint16_t offset, uint16_t len)
{
	nss_ovpnmgr_info("%s:\n", hdr);
	print_hex_dump_bytes("", DUMP_PREFIX_OFFSET, skb->data + offset, len);
}
#define NSS_OVPNMGR_TUN_PKT_DUMP(skb, hdr, offset, len) nss_ovpnmgr_tun_pkt_dump(skb, hdr, offset, len)
#else
#define NSS_OVPNMGR_TUN_PKT_DUMP(skb, hdr, offset, len)
#endif

/*
 * nss_ovpnmgr_tun_ipv4_forward()
 *	Transmit IPv4 encapsulated packet exceptioned from NSS.
 */
static void nss_ovpnmgr_tun_ipv4_forward(struct nss_ovpnmgr_tun *tun, struct sk_buff *skb)
{
	struct nss_ovpnmgr_app *app = tun->app;
	struct udphdr *udph;
	struct rtable *rt;
	struct iphdr *iph;

	skb->protocol = htons(ETH_P_IP);
	skb_reset_network_header(skb);
	iph = ip_hdr(skb);

	rt = ip_route_output(dev_net(app->dev), iph->daddr, iph->saddr, 0, 0);
	if (unlikely(IS_ERR(rt))) {
		nss_ovpnmgr_warn("%p: Failed to find IPv4 route.\n", skb);
		tun->outer.stats.host_pkt_drop++;
		dev_kfree_skb_any(skb);
		return;
	}

	/*
	 * Reset skb->dst.
	 */
	skb_dst_drop(skb);
	skb_dst_set(skb, &rt->dst);
	/*
	 * Set packet type as host generated and set skb->dev to tun/tap device.
	 */
	skb->pkt_type = PACKET_HOST;
	skb->dev = app->dev;

	/*
	 * Check IPv4 header length.
	 */
	if (ip_hdrlen(skb) != sizeof(*iph)) {
		nss_ovpnmgr_warn("%p: IPv4 header length is incorrect.\n", skb);
		tun->outer.stats.host_pkt_drop++;
		dev_kfree_skb_any(skb);
		return;
	}

	/*
	 * Set transport header.
	 */
	skb_set_transport_header(skb, ip_hdrlen(skb));
	skb->skb_iif = app->dev->ifindex;

	if (iph->protocol != IPPROTO_UDP) {
		nss_ovpnmgr_warn("%p: Encapuslation protocol is not UDP.\n", skb);
		tun->outer.stats.host_pkt_drop++;
		dev_kfree_skb_any(skb);
		return;
	}

	udph = udp_hdr(skb);
	udp_set_csum(0, skb, iph->saddr, iph->daddr, ntohs(udph->len));
	ip_local_out(&init_net, NULL, skb);
}

/*
 * nss_ovpnmgr_tun_ipv6_forward()
 *	Transmit IPv6 encapsulated packet exceptioned from NSS.
 */
static void nss_ovpnmgr_tun_ipv6_forward(struct nss_ovpnmgr_tun *tun, struct sk_buff *skb)
{
	struct nss_ovpnmgr_app *app = tun->app;
	struct rt6_info *rt6;
	struct ipv6hdr *ip6h;

	skb_reset_network_header(skb);
	skb->protocol = htons(ETH_P_IPV6);
	ip6h = ipv6_hdr(skb);

	rt6 = rt6_lookup(dev_net(app->dev), &ip6h->daddr, &ip6h->saddr, 0, 0);
	if (!rt6) {
		nss_ovpnmgr_warn("%p: Failed to find IPv6 route.\n", skb);
		tun->outer.stats.host_pkt_drop++;
		dev_kfree_skb_any(skb);
		return;
	}

	/*
	 * Reset skb->dst.
	 */
	skb_dst_drop(skb);
	skb_dst_set(skb, &rt6->dst);

	/*
	 * Set packet type as host generated and set skb->dev to tun/tap device.
	 */
	skb->pkt_type = PACKET_HOST;
	skb->dev = app->dev;

	/*
	 * Set transport header.
	 */
	skb_set_transport_header(skb, sizeof(*ip6h));
	skb->skb_iif = app->dev->ifindex;

	if (ip6h->nexthdr != IPPROTO_UDP) {
		nss_ovpnmgr_warn("%p: Encapuslation protocol is not UDP.\n", skb);
		tun->outer.stats.host_pkt_drop++;
		dev_kfree_skb_any(skb);
		return;
	}

	skb->ip_summed = CHECKSUM_PARTIAL;
	ip6_local_out(&init_net, NULL, skb);
}

/*
 * nss_ovpnmgr_tun_rx_outer()
 *	Handles encapsulated packets received from NSS firmware
 */
static void nss_ovpnmgr_tun_rx_outer(struct net_device *dev, struct sk_buff *skb, struct napi_struct *napi)
{
	struct nss_ovpnmgr_tun *tun = netdev_priv(dev);

	skb_reset_network_header(skb);

	switch (ip_hdr(skb)->version) {
	case IPVERSION:
		/*
		 * Dump ip_hdr(20) + udp_hdr(8) + ovpn_hdr(4)
		 */
		NSS_OVPNMGR_TUN_PKT_DUMP(skb, "IPv4 packet from NSS(Outer)", 0, 32);
		nss_ovpnmgr_tun_ipv4_forward(tun, skb);
		break;
	case 6:
		/*
		 * Dump ipv6_hdr(40) + udp_hdr(8) + ovpn_hdr(4)
		 */
		NSS_OVPNMGR_TUN_PKT_DUMP(skb, "IPv6 packet from NSS(Outer)", 0, 52);
		nss_ovpnmgr_tun_ipv6_forward(tun, skb);
		break;
	default:
		/*
		 * Dump initial 20 bytes to analyze junk packet.
		 */
		NSS_OVPNMGR_TUN_PKT_DUMP(skb, "Junk packet from NSS(Outer)", 0, 20);
		tun->outer.stats.host_pkt_drop++;
		dev_kfree_skb_any(skb);
	}
}

/*
 * nss_ovpnmgr_tun_rx_inner()
 *	Handles decapsulated packets received from NSS firmware
 */
static void nss_ovpnmgr_tun_rx_inner(struct net_device *dev, struct sk_buff *skb, struct napi_struct *napi)
{
	struct nss_ovpnmgr_tun *tun = netdev_priv(dev);
	struct nss_ovpnmgr_app *app = tun->app;

	skb_reset_mac_header(skb);
	skb_reset_network_header(skb);
	skb_reset_transport_header(skb);

	switch (ip_hdr(skb)->version) {
	case IPVERSION:
		/*
		 * Dump ip_hdr(20) + transport header(8)
		 */
		NSS_OVPNMGR_TUN_PKT_DUMP(skb, "IPv4 packet from NSS(Inner)", 0, 28);
		skb->protocol = htons(ETH_P_IP);
		break;
	case 6:
		/*
		 * Dump ipv6_hdr(40) + transport header(8)
		 */
		NSS_OVPNMGR_TUN_PKT_DUMP(skb, "IPv6 packet from NSS(Inner)", 0, 48);
		skb->protocol = htons(ETH_P_IPV6);
		break;
	default:
		/*
		 * Dump maximum control packet size(28)
		 */
		NSS_OVPNMGR_TUN_PKT_DUMP(skb, "OCC/PING(OVPN) packet from NSS(Inner)", 0, 28);
		nss_ovpnmgr_app_forward(app, tun, skb);
		return;
	}

	skb->ip_summed = CHECKSUM_NONE;
	skb->pkt_type = PACKET_HOST;
	skb->dev = app->dev;
	skb->skb_iif = app->dev->ifindex;
	netif_receive_skb(skb);
}

/*
 * nss_ovpnmgr_tun_stats_update()
 *	Update statistics.
 */
static void nss_ovpnmgr_tun_stats_update(struct nss_ovpnmgr_tun *tun, struct rtnl_link_stats64 *stats)
{
	struct nss_ovpnmgr_tun_ctx_stats *inner_stats, *outer_stats;
	int i;

	/*
	 * This API is called under lock.
	 */

	inner_stats = &tun->inner.stats;
	outer_stats = &tun->outer.stats;

	stats->rx_packets += inner_stats->rx_packets;
	stats->rx_bytes += inner_stats->rx_bytes;
	stats->tx_packets += outer_stats->tx_packets;
	stats->tx_bytes += outer_stats->tx_bytes;

	for (i = 0; i < NSS_MAX_NUM_PRI; i++) {
		stats->rx_dropped += inner_stats->rx_dropped[i];
		stats->tx_dropped += outer_stats->rx_dropped[i];
	}

	for (i = 1; i < NSS_CRYPTO_CMN_RESP_ERROR_MAX; i++) {
		stats->rx_dropped += inner_stats->fail_crypto[i];
		stats->tx_dropped += outer_stats->fail_crypto[i];
	}

	for (i = 0; i < NSS_QVPN_PKT_DROP_EVENT_MAX; i++) {
		stats->rx_dropped += inner_stats->fail_offload[i];
		stats->tx_dropped += outer_stats->fail_offload[i];
	}
}

/*
 * nss_ovpnmgr_tun_update_node_stats()
 *	Update node statistics.
 */
static void nss_ovpnmgr_tun_update_node_stats(struct nss_ovpnmgr_tun *tun, struct nss_qvpn_stats_sync_msg *stats, bool inner)
{
	struct nss_ovpnmgr_tun_stats *tun_stats = &tun->stats;

	if (inner) {
		tun_stats->tun_read_bytes += stats->node_stats.rx_bytes;
		tun_stats->link_write_bytes += stats->node_stats.tx_bytes;
	} else {
		tun_stats->link_read_bytes += stats->node_stats.rx_bytes;
		tun_stats->tun_write_bytes += stats->node_stats.tx_bytes;
		tun_stats->link_read_bytes_auth += stats->node_stats.tx_bytes;
		tun_stats->link_read_bytes_auth += stats->exception_event[NSS_QVPN_EXCEPTION_EVENT_RX_DATA_PKT];
	}
}

/*
 * nss_ovpnmgr_tun_event_inner()
 *	Handles events/messages from NSS firmware
 */
static void nss_ovpnmgr_tun_event_inner(void *app_data, struct nss_cmn_msg *ncm)
{
	struct nss_ovpnmgr_tun_ctx_stats *ctx_stats;
	struct nss_ovpnmgr_tun *tun = app_data;
	struct nss_qvpn_msg *nom = (struct nss_qvpn_msg *)ncm;
	struct nss_qvpn_stats_sync_msg *stats = &nom->msg.stats;
	struct nss_ovpnmgr_app *app = tun->app;
	struct net_device *dev;
	int i;

	if (nom->cm.type != NSS_QVPN_MSG_TYPE_SYNC_STATS) {
		nss_ovpnmgr_warn("%p: Unsupported event received from NSS: %d\n", tun, nom->cm.type);
		return;
	}

	write_lock(&ovpnmgr_ctx.lock);

	ctx_stats = &tun->inner.stats;

	ctx_stats->rx_packets += stats->node_stats.rx_packets;
	ctx_stats->rx_bytes += stats->node_stats.rx_bytes;

	ctx_stats->tx_packets += stats->node_stats.tx_packets;
	ctx_stats->tx_bytes += stats->node_stats.tx_bytes;

	for (i = 0; i < NSS_MAX_NUM_PRI; i++) {
		ctx_stats->rx_dropped[i] += stats->node_stats.rx_dropped[i];
	}

	for (i = 0; i < NSS_CRYPTO_CMN_RESP_ERROR_MAX; i++) {
		ctx_stats->fail_crypto[i] += stats->crypto_resp_error[i];
	}

	for (i = 0; i < NSS_QVPN_PKT_DROP_EVENT_MAX; i++) {
		ctx_stats->fail_offload[i] += stats->pkt_drop_event[i];
	}

	for (i = 0 ; i < NSS_QVPN_EXCEPTION_EVENT_MAX; i++) {
		ctx_stats->exception[i] += stats->exception_event[i];
	}

	dev = app->dev;
	nss_ovpnmgr_tun_update_node_stats(tun, stats, true);

	write_unlock(&ovpnmgr_ctx.lock);
}

/*
 * nss_ovpnmgr_tun_event_outer()
 *	Handles events/messages from NSS firmware
 */
static void nss_ovpnmgr_tun_event_outer(void *app_data, struct nss_cmn_msg *ncm)
{
	struct nss_ovpnmgr_tun_ctx_stats *ctx_stats;
	struct nss_ovpnmgr_tun *tun = app_data;
	struct nss_qvpn_msg *nom = (struct nss_qvpn_msg *)ncm;
	struct nss_qvpn_stats_sync_msg *stats = &nom->msg.stats;
	struct nss_ovpnmgr_app *app = tun->app;
	struct net_device *dev;
	int i;

	if (nom->cm.type != NSS_QVPN_MSG_TYPE_SYNC_STATS) {
		nss_ovpnmgr_warn("%p: Unsupported event received from NSS: %d\n", tun, nom->cm.type);
		return;
	}

	write_lock(&ovpnmgr_ctx.lock);

	ctx_stats = &tun->outer.stats;

	ctx_stats->rx_packets += stats->node_stats.rx_packets;
	ctx_stats->rx_bytes += stats->node_stats.rx_bytes;

	ctx_stats->tx_packets += stats->node_stats.tx_packets;
	ctx_stats->tx_bytes += stats->node_stats.tx_bytes;

	for (i = 0; i < NSS_MAX_NUM_PRI; i++) {
		ctx_stats->rx_dropped[i] += stats->node_stats.rx_dropped[i];
	}

	for (i = 0; i < NSS_CRYPTO_CMN_RESP_ERROR_MAX; i++) {
		ctx_stats->fail_crypto[i] += stats->crypto_resp_error[i];
	}

	for (i = 0; i < NSS_QVPN_PKT_DROP_EVENT_MAX; i++) {
		ctx_stats->fail_offload[i] += stats->pkt_drop_event[i];
	}

	for (i = 0 ; i < NSS_QVPN_EXCEPTION_EVENT_MAX; i++) {
		ctx_stats->exception[i] += stats->exception_event[i];
	}

	dev = app->dev;
	nss_ovpnmgr_tun_update_node_stats(tun, stats, false);

	write_unlock(&ovpnmgr_ctx.lock);
}

/*
 * nss_ovpnmgr_tun_ctx_deinit()
 *	De-initialize tunnel node.
 */
static int nss_ovpnmgr_tun_ctx_deinit(struct nss_ovpnmgr_tun_ctx *ctx)
{
	nss_qvpn_unregister_if(ctx->ifnum);

	if (nss_dynamic_interface_dealloc_node(ctx->ifnum, ctx->di_type) != NSS_TX_SUCCESS) {
		nss_ovpnmgr_warn("%p: failed to dealloc OVPN inner DI\n", ctx);
	}

	/* Unregister Encryption */
	nss_ovpnmgr_crypto_ctx_free(&ctx->active);
	nss_ovpnmgr_crypto_ctx_free(&ctx->expiring);

	return 0;
}

/*
 * nss_ovpnmgr_tun_deconfig()
 *	Deconfigure inner tunnel node.
 */
static int nss_ovpnmgr_tun_deconfig(struct nss_ovpnmgr_tun_ctx *ctx, struct nss_ovpnmgr_tun *tun)
{
	struct nss_qvpn_msg nom;
	struct nss_qvpn_tunnel_config_msg *msg = &nom.msg.tunnel_config;
	enum nss_qvpn_error_type resp;
	nss_tx_status_t status;

	/*
	 * TODO: Need to add support to retry if the failure is due to queueu full.
	 */
	status = nss_qvpn_tx_msg_sync(tun->nss_ctx, &nom, ctx->ifnum, NSS_QVPN_MSG_TYPE_TUNNEL_DECONFIGURE,
			sizeof(*msg), &resp);
	if (status != NSS_TX_SUCCESS) {
		nss_ovpnmgr_warn("%p: failed to configure outer interface, resp=%d, status = %d\n", tun, resp, status);
		return status == NSS_TX_FAILURE_QUEUE ? -EBUSY : -EINVAL;
	}

	return 0;
}

/*
 * ovpnmgr_dev_ops
 */
static const struct net_device_ops ovpnmgr_dev_ops = {
	/*
	 * ovpn netdev is created for transmitting control packets in data channel
	 * to application. No addtional functionality is implemented.
	 */
};

/*
 * nss_ovpnmgr_tun_free()
 *	Delete OVPN tunnel.
 */
static void nss_ovpnmgr_tun_free(struct net_device *dev)
{
	struct nss_ovpnmgr_tun *tun = netdev_priv(dev);

	nss_ovpnmgr_tun_deconfig(&tun->inner, tun);
	nss_ovpnmgr_tun_deconfig(&tun->outer, tun);
	nss_ovpnmgr_tun_ctx_deinit(&tun->inner);
	nss_ovpnmgr_tun_ctx_deinit(&tun->outer);
	free_netdev(dev);
}

/*
 * nss_ovpnmgr_tun_dev_setup()
 *	Setup function for dummy netdevice.
 */
static void nss_ovpnmgr_tun_dev_setup(struct net_device *dev)
{
	dev->needed_headroom = NSS_OVPNMGR_TUN_HEADROOM;
	dev->needed_tailroom = NSS_OVPNMGR_TUN_TAILROOM;

	dev->header_ops = NULL;
	dev->netdev_ops = &ovpnmgr_dev_ops;
	dev->ethtool_ops = NULL;
	dev->destructor = nss_ovpnmgr_tun_free;
}

/*
 * nss_ovpnmgr_tun_alloc()
 *	Allocte an OVPN tunnel instance.
 */
static struct nss_ovpnmgr_tun *nss_ovpnmgr_tun_alloc(void)
{
	struct nss_ovpnmgr_tun *tun;
	struct net_device *dev;

	dev = alloc_netdev(sizeof(*tun), NSS_OVPNMGR_TUN_NAME, NET_NAME_ENUM, nss_ovpnmgr_tun_dev_setup);
	if (!dev) {
		nss_ovpnmgr_warn("unable to allocate ovpn netdev\n");
		return NULL;
	}

	tun = netdev_priv(dev);
	tun->dev = dev;

	INIT_LIST_HEAD(&tun->list);
	INIT_LIST_HEAD(&tun->route_list);

	tun->nss_ctx = nss_qvpn_get_context();
	if (unlikely(!tun->nss_ctx)) {
		nss_ovpnmgr_warn("%p: Failed to get NSS OVPN context.\n", tun);
		return NULL;
	}

	tun->inner.nss_ctx = tun->outer.nss_ctx = tun->nss_ctx;
	return tun;
}

/*
 * nss_ovpnmgr_tun_init_qvpn_config()
 *	Initialize QVPN configuration data structure.
 */
static void nss_ovpnmgr_tun_init_qvpn_config(struct nss_ovpnmgr_tun *tun, struct nss_qvpn_tunnel_config_msg *qvpn_cfg)
{
	memset(qvpn_cfg, 0, sizeof(*qvpn_cfg));

	if (tun->tun_cfg.flags & NSS_OVPNMGR_HDR_FLAG_IPV6) {
		nss_ovpnmgr_ipv6_addr_ntohl(qvpn_cfg->hdr_cfg.dst_ip, tun->tun_hdr.dst_ip);
		nss_ovpnmgr_ipv6_addr_ntohl(qvpn_cfg->hdr_cfg.src_ip, tun->tun_hdr.src_ip);
		qvpn_cfg->hdr_cfg.hdr_flags = NSS_QVPN_HDR_FLAG_IPV6;
	} else {
		qvpn_cfg->hdr_cfg.src_ip[0] = ntohl(tun->tun_hdr.src_ip[0]);
		qvpn_cfg->hdr_cfg.dst_ip[0] = ntohl(tun->tun_hdr.dst_ip[0]);
	}

	qvpn_cfg->hdr_cfg.src_port = ntohs(tun->tun_hdr.src_port);
	qvpn_cfg->hdr_cfg.dst_port = ntohs(tun->tun_hdr.dst_port);
	qvpn_cfg->hdr_cfg.hdr_flags |= NSS_QVPN_HDR_FLAG_L4_UDP;
	qvpn_cfg->hdr_cfg.seqnum_size = 4;
	qvpn_cfg->hdr_cfg.anti_replay_alg = NSS_QVPN_ANTI_REPLAY_ALG_REPLAY_WINDOW;
	qvpn_cfg->hdr_cfg.session_id_offset = 0;
	qvpn_cfg->hdr_cfg.session_id_size = 1;
	qvpn_cfg->hdr_cfg.vpn_hdr_head_offset = 0;
	qvpn_cfg->hdr_cfg.vpn_hdr_tail_size = 0;
}

/*
 * nss_ovpnmgr_tun_inner_config()
 *	Configure inner tunnel node.
 */
static int nss_ovpnmgr_tun_inner_config(struct nss_ovpnmgr_tun *tun)
{
	struct nss_qvpn_msg nom;
	struct nss_qvpn_tunnel_config_msg *qvpn_cfg = &nom.msg.tunnel_config;
	enum nss_qvpn_error_type resp;
	nss_tx_status_t status;
	int32_t total_cmds = 1;

	nss_ovpnmgr_tun_init_qvpn_config(tun, qvpn_cfg);

	qvpn_cfg->sibling_if = tun->outer.ifnum;

	/*
	 * QVPN commands for OVPN encapsulation.
	 *	NSS_QVPN_CMDS_TYPE_ADD_VPN_HDR, NSS_QVPN_CMDS_TYPE_ANTI_REPLAY,
	 *	NSS_QVPN_CMDS_TYPE_ENCRYPT, NSS_QVPN_CMDS_TYPE_ADD_L3_L4_HDR
	 */
	qvpn_cfg->cmd[0] = NSS_QVPN_CMDS_TYPE_ADD_VPN_HDR;

	if (!(tun->tun_cfg.flags & NSS_OVPNMGR_HDR_FLAG_NO_REPLAY)) {
		qvpn_cfg->cmd[total_cmds++] = NSS_QVPN_CMDS_TYPE_ANTI_REPLAY;
	}

	if (tun->inner.active.crypto_idx != U16_MAX) {
		qvpn_cfg->cmd[total_cmds++] = NSS_QVPN_CMDS_TYPE_ENCRYPT;
		qvpn_cfg->cmd_profile = NSS_QVPN_PROFILE_CRYPTO_ENCAP;
	} else {
		qvpn_cfg->cmd_profile = NSS_QVPN_PROFILE_ENCAP;
	}

	qvpn_cfg->cmd[total_cmds++] = NSS_QVPN_CMDS_TYPE_ADD_L3_L4_HDR;
	qvpn_cfg->total_cmds = total_cmds;

	/*
	 * Crypto configuration.
	 */
	qvpn_cfg->crypto_key.crypto_idx = tun->inner.active.crypto_idx;

	if (tun->tun_cfg.flags & NSS_OVPNMGR_HDR_FLAG_DATA_V2) {
		uint32_t *session_id = (uint32_t *)qvpn_cfg->hdr_cfg.vpn_hdr_head;

		*session_id = htonl(((NSS_OVPNMGR_TUN_DATA_V2 << NSS_OVPNMGR_TUN_OPCODE_SHIFT) |
					tun->inner.active.key_id) << NSS_OVPNMGR_TUN_PEER_ID_SHIFT |
					(tun->tun_cfg.peer_id & 0xFFFFFF));
		/*
		 * [op+kid|peer-id|HMAC Len|IV|SNO|Inner Packet]
		 * [1|3|20-32|16-24-32]
		 */
		qvpn_cfg->crypto_cfg.hmac_offset = 4;
		qvpn_cfg->hdr_cfg.vpn_hdr_head_size = 4;
	} else {
		uint8_t *session_id = (uint8_t *)qvpn_cfg->hdr_cfg.vpn_hdr_head;

		*session_id = (NSS_OVPNMGR_TUN_DATA_V1 << NSS_OVPNMGR_TUN_OPCODE_SHIFT) | tun->inner.active.key_id;
		/*
		 * [op+kid|HMAC Len|IV|SNO|Inner Packet]
		 * [1|20-32|16-24-32]
		 */
		qvpn_cfg->crypto_cfg.hmac_offset = 1;
		qvpn_cfg->hdr_cfg.vpn_hdr_head_size = 1;
	}

	memcpy(qvpn_cfg->crypto_key.session_id, qvpn_cfg->hdr_cfg.vpn_hdr_head, 1);
	qvpn_cfg->crypto_cfg.hmac_len = tun->inner.active.hash_len;
	qvpn_cfg->crypto_cfg.iv_len = tun->inner.active.iv_len;


	qvpn_cfg->hdr_cfg.vpn_hdr_head_size += qvpn_cfg->crypto_cfg.hmac_len + qvpn_cfg->crypto_cfg.iv_len +
						qvpn_cfg->hdr_cfg.seqnum_size;
	qvpn_cfg->crypto_cfg.iv_offset = qvpn_cfg->crypto_cfg.hmac_offset + qvpn_cfg->crypto_cfg.hmac_len;
	qvpn_cfg->crypto_cfg.auth_offset = qvpn_cfg->crypto_cfg.iv_offset;
	qvpn_cfg->crypto_cfg.cipher_op_offset = qvpn_cfg->crypto_cfg.iv_offset + qvpn_cfg->crypto_cfg.iv_len;
	qvpn_cfg->crypto_cfg.cipher_blk_size = tun->outer.active.blk_len;

	qvpn_cfg->hdr_cfg.seqnum_offset = qvpn_cfg->crypto_cfg.iv_offset + qvpn_cfg->crypto_cfg.iv_len;

	switch (tun->inner.active.crypto_type) {
	case NSS_OVPNMGR_CRYPTO_TYPE_AEAD:
		qvpn_cfg->crypto_cfg.crypto_mode = NSS_QVPN_CRYPTO_MODE_ENC_AUTH;
		qvpn_cfg->crypto_cfg.iv_type = NSS_QVPN_IV_TYPE_DYNAMIC_RAND;
		qvpn_cfg->crypto_cfg.pad_type = NSS_QVPN_PAD_TYPE_PKCS7;
		break;
	case NSS_OVPNMGR_CRYPTO_TYPE_ABLK:
		qvpn_cfg->crypto_cfg.crypto_mode = NSS_QVPN_CRYPTO_MODE_ENC;
		qvpn_cfg->crypto_cfg.iv_type = NSS_QVPN_IV_TYPE_DYNAMIC_RAND;
		qvpn_cfg->crypto_cfg.pad_type = NSS_QVPN_PAD_TYPE_PKCS7;
		break;
	case NSS_OVPNMGR_CRYPTO_TYPE_AHASH:
		qvpn_cfg->crypto_cfg.crypto_mode = NSS_QVPN_CRYPTO_MODE_AUTH;
		qvpn_cfg->crypto_cfg.iv_type = NSS_QVPN_IV_TYPE_NONE;
		qvpn_cfg->crypto_cfg.pad_type = NSS_QVPN_PAD_TYPE_NONE;
		break;
	default:
		qvpn_cfg->crypto_cfg.crypto_mode = NSS_QVPN_CRYPTO_MODE_NONE;
		qvpn_cfg->crypto_cfg.iv_type = NSS_QVPN_IV_TYPE_NONE;
		qvpn_cfg->crypto_cfg.pad_type = NSS_QVPN_PAD_TYPE_NONE;
	}

	status = nss_qvpn_tx_msg_sync(tun->nss_ctx,  &nom, tun->inner.ifnum, NSS_QVPN_MSG_TYPE_TUNNEL_CONFIGURE,
					sizeof(*qvpn_cfg), &resp);
	if (status != NSS_TX_SUCCESS) {
		nss_ovpnmgr_warn("%p: failed to configure inner interface, resp = %d, status = %d\n", tun, resp, status);
		return status == NSS_TX_FAILURE_QUEUE ? -EBUSY : -EINVAL;
	}

	return 0;
}

/*
 * nss_ovpnmgr_tun_outer_config()
 *	Configure outer tunnel node.
 */
static int nss_ovpnmgr_tun_outer_config(struct nss_ovpnmgr_tun *tun)
{
	struct nss_qvpn_msg nom;
	struct nss_qvpn_tunnel_config_msg *qvpn_cfg = &nom.msg.tunnel_config;
	enum nss_qvpn_error_type resp;
	nss_tx_status_t status;
	int32_t total_cmds = 0;

	nss_ovpnmgr_tun_init_qvpn_config(tun, qvpn_cfg);

	qvpn_cfg->sibling_if = tun->inner.ifnum;

	/*
	 * QVPN commands for OVPN encapsulation.
	 *	NSS_QVPN_CMDS_TYPE_DECRYPT, NSS_QVPN_CMDS_TYPE_ANTI_REPLAY,
	 *	NSS_QVPN_CMDS_TYPE_REMOVE_L3_L4_HDR, NSS_QVPN_CMDS_TYPE_REMOVE_VPN_HDR
	 */

	if (tun->inner.active.crypto_idx != U16_MAX) {
		qvpn_cfg->cmd[total_cmds++] = NSS_QVPN_CMDS_TYPE_DECRYPT;
		qvpn_cfg->cmd_profile = NSS_QVPN_PROFILE_CRYPTO_DECAP;
	} else {
		qvpn_cfg->cmd_profile = NSS_QVPN_PROFILE_DECAP;
	}

	if (!(tun->tun_cfg.flags & NSS_OVPNMGR_HDR_FLAG_NO_REPLAY)) {
		qvpn_cfg->cmd[total_cmds++] = NSS_QVPN_CMDS_TYPE_ANTI_REPLAY;
	}

	qvpn_cfg->cmd[total_cmds++] = NSS_QVPN_CMDS_TYPE_REMOVE_L3_L4_HDR;
	qvpn_cfg->cmd[total_cmds++] = NSS_QVPN_CMDS_TYPE_REMOVE_VPN_HDR;
	qvpn_cfg->total_cmds = total_cmds;

	/*
	 * Crypto configuration.
	 */
	qvpn_cfg->crypto_key.crypto_idx = tun->outer.active.crypto_idx;

	if (tun->tun_cfg.flags & NSS_OVPNMGR_HDR_FLAG_PEER_DATA_V2) {
		uint32_t *session_id = (uint32_t *)qvpn_cfg->hdr_cfg.vpn_hdr_head;

		nss_ovpnmgr_info("Peer transmits V2 data packets\n");

		*session_id = htonl(((NSS_OVPNMGR_TUN_DATA_V2 << NSS_OVPNMGR_TUN_OPCODE_SHIFT) |
					tun->outer.active.key_id) << NSS_OVPNMGR_TUN_PEER_ID_SHIFT |
					(tun->tun_cfg.peer_id & 0xFFFFFF));
		/*
		 * [op+kid|peer-id|HMAC Len|IV|SNO|Inner Packet]
		 * [1|3|20-32|16-24-32]
		 */
		qvpn_cfg->crypto_cfg.hmac_offset = 4;
		qvpn_cfg->hdr_cfg.vpn_hdr_head_size = 4;
	} else {
		uint8_t *session_id = (uint8_t *)qvpn_cfg->hdr_cfg.vpn_hdr_head;

		*session_id = (NSS_OVPNMGR_TUN_DATA_V1 << NSS_OVPNMGR_TUN_OPCODE_SHIFT) | tun->outer.active.key_id;
		/*
		 * [op+kid|HMAC Len|IV|SNO|Inner Packet]
		 * [1|20-32|16-24-32]
		 */
		qvpn_cfg->crypto_cfg.hmac_offset = 1;
		qvpn_cfg->hdr_cfg.vpn_hdr_head_size = 1;
	}

	memcpy(qvpn_cfg->crypto_key.session_id, qvpn_cfg->hdr_cfg.vpn_hdr_head, 1);

	qvpn_cfg->crypto_cfg.hmac_len = tun->outer.active.hash_len;
	qvpn_cfg->crypto_cfg.iv_len = tun->outer.active.iv_len;

	qvpn_cfg->hdr_cfg.vpn_hdr_head_size += qvpn_cfg->crypto_cfg.hmac_len + qvpn_cfg->crypto_cfg.iv_len +
						qvpn_cfg->hdr_cfg.seqnum_size;
	qvpn_cfg->crypto_cfg.iv_offset = qvpn_cfg->crypto_cfg.hmac_offset + qvpn_cfg->crypto_cfg.hmac_len;
	qvpn_cfg->crypto_cfg.auth_offset = qvpn_cfg->crypto_cfg.iv_offset;
	qvpn_cfg->crypto_cfg.cipher_op_offset = qvpn_cfg->crypto_cfg.iv_offset + qvpn_cfg->crypto_cfg.iv_len;
	qvpn_cfg->crypto_cfg.cipher_blk_size = tun->outer.active.blk_len;

	switch (tun->outer.active.crypto_type) {
	case NSS_OVPNMGR_CRYPTO_TYPE_AEAD:
		qvpn_cfg->crypto_cfg.crypto_mode = NSS_QVPN_CRYPTO_MODE_AUTH_DEC;
		qvpn_cfg->crypto_cfg.iv_type = NSS_QVPN_IV_TYPE_DYNAMIC_RAND;
		qvpn_cfg->crypto_cfg.pad_type = NSS_QVPN_PAD_TYPE_PKCS7;
		break;
	case NSS_OVPNMGR_CRYPTO_TYPE_ABLK:
		qvpn_cfg->crypto_cfg.crypto_mode = NSS_QVPN_CRYPTO_MODE_DEC;
		qvpn_cfg->crypto_cfg.iv_type = NSS_QVPN_IV_TYPE_DYNAMIC_RAND;
		qvpn_cfg->crypto_cfg.pad_type = NSS_QVPN_PAD_TYPE_PKCS7;
		break;
	case NSS_OVPNMGR_CRYPTO_TYPE_AHASH:
		qvpn_cfg->crypto_cfg.crypto_mode = NSS_QVPN_CRYPTO_MODE_AUTH;
		qvpn_cfg->crypto_cfg.iv_type = NSS_QVPN_IV_TYPE_NONE;
		qvpn_cfg->crypto_cfg.pad_type = NSS_QVPN_PAD_TYPE_NONE;
		break;
	default:
		qvpn_cfg->crypto_cfg.crypto_mode = NSS_QVPN_CRYPTO_MODE_NONE;
		qvpn_cfg->crypto_cfg.iv_type = NSS_QVPN_IV_TYPE_NONE;
		qvpn_cfg->crypto_cfg.pad_type = NSS_QVPN_PAD_TYPE_NONE;
	}

	status = nss_qvpn_tx_msg_sync(tun->nss_ctx, &nom, tun->outer.ifnum, NSS_QVPN_MSG_TYPE_TUNNEL_CONFIGURE,
					sizeof(*qvpn_cfg), &resp);
	if (status != NSS_TX_SUCCESS) {
		nss_ovpnmgr_warn("%p: failed to configure outer interface, resp = %d, status = %d\n", tun, resp, status);
		return status == NSS_TX_FAILURE_QUEUE ? -EBUSY : -EINVAL;
	}

	return 0;
}

/*
 * nss_ovpnmgr_tun_ctx_inner_init()
 *	Initialize inner tunnel context.
 */
static int nss_ovpnmgr_tun_ctx_inner_init(struct nss_ovpnmgr_tun *tun,
		struct nss_ovpnmgr_crypto_config *crypto_cfg)
{
	int ret;

	/*
	 * Register Encryption.
	 */
	if (nss_ovpnmgr_crypto_ctx_alloc(&tun->inner.active, crypto_cfg, &crypto_cfg->encrypt)) {
		nss_ovpnmgr_error("%p: Failed to register Encryption session\n", tun);
		return -EINVAL;
	}

	/* Send message to NSS FW */
	tun->inner.di_type = NSS_DYNAMIC_INTERFACE_TYPE_QVPN_INNER;
	tun->inner.ifnum = nss_dynamic_interface_alloc_node(tun->inner.di_type);
	if (tun->inner.ifnum < 0) {
		nss_ovpnmgr_error("%p: Failed to alloc OVPN inner dynamic interface\n", tun);
		ret = -ENXIO;
		goto free_crypto;
	}

	if (!nss_qvpn_register_if(tun->inner.ifnum, nss_ovpnmgr_tun_rx_inner,
				nss_ovpnmgr_tun_event_inner, tun->dev, 0, tun)) {
		nss_ovpnmgr_warn("%p: Failed to register OVPN inner interface %d\n", tun, tun->inner.ifnum);
		ret = -EIO;
		goto free_ifnum;
	}

	return 0;

free_ifnum:
	if (nss_dynamic_interface_dealloc_node(tun->inner.ifnum, tun->inner.di_type)) {
		nss_ovpnmgr_warn("%p: Failed to dealloc inner DI.\n", tun);
	}

free_crypto:
	/* Unregister Encryption */
	nss_ovpnmgr_crypto_ctx_free(&tun->inner.active);
	return ret;
}

/*
 * nss_ovpnmgr_tun_ctx_outer_init()
 *	Initialize outer tunnel context.
 */
static int nss_ovpnmgr_tun_ctx_outer_init(struct nss_ovpnmgr_tun *tun, struct nss_ovpnmgr_crypto_config *crypto_cfg)
{
	int ret;

	/*
	 * Register Encryption.
	 */
	if (nss_ovpnmgr_crypto_ctx_alloc(&tun->outer.active, crypto_cfg, &crypto_cfg->decrypt)) {
		nss_ovpnmgr_error("%p: Failed to register Encryption session\n", tun);
		return -EINVAL;
	}

	/* Send message to NSS FW */
	tun->outer.di_type = NSS_DYNAMIC_INTERFACE_TYPE_QVPN_OUTER;
	tun->outer.ifnum = nss_dynamic_interface_alloc_node(tun->outer.di_type);
	if (tun->outer.ifnum < 0) {
		nss_ovpnmgr_error("%p: Failed to alloc OVPN outer dynamic interface\n", tun);
		ret = -ENXIO;
		goto free_crypto;
	}

	if (!nss_qvpn_register_if(tun->outer.ifnum, nss_ovpnmgr_tun_rx_outer, nss_ovpnmgr_tun_event_outer,
				tun->dev, 0, tun)) {
		nss_ovpnmgr_warn("%p: Failed to register OVPN outer interface %d\n", tun, tun->outer.ifnum);
		ret = -EIO;
		goto free_ifnum;
	}

	return 0;

free_ifnum:
	if (nss_dynamic_interface_dealloc_node(tun->outer.ifnum, tun->outer.di_type)) {
		nss_ovpnmgr_warn("%p: Failed to dealloc outer dynamic node.\n", tun);
	}

free_crypto:
	/* Unregister Decryption */
	nss_ovpnmgr_crypto_ctx_free(&tun->outer.active);
	return ret;
}

/*
 * nss_ovpnmgr_tun_config_verify()
 *	Verify tunnel configuration.
 */
static int nss_ovpnmgr_tun_config_verify(struct nss_ovpnmgr_tun_tuple *tun_hdr, struct nss_ovpnmgr_tun_config *tun_cfg)
{
	if (tun_cfg->flags & NSS_OVPNMGR_HDR_FLAG_L4_PROTO_TCP) {
		nss_ovpnmgr_warn("%p: OVPN data channel offload over TCP is not supported\n", tun_hdr);
		return -EINVAL;
	}

	if (tun_cfg->flags & NSS_OVPNMGR_HDR_FLAG_FRAG) {
		nss_ovpnmgr_warn("%p: Fragmentation specified by OVPN protocol is not supported\n", tun_hdr);
		return -EINVAL;
	}

	if (tun_cfg->flags & NSS_OVPNMGR_HDR_FLAG_PID_LONG_FMT) {
		nss_ovpnmgr_warn("%p: PID Long format is not supported\n", tun_hdr);
		return -EINVAL;
	}

	if (tun_cfg->flags & NSS_OVPNMGR_HDR_FLAG_SHARED_KEY) {
		nss_ovpnmgr_warn("%p: Crypto shared key is not supported\n", tun_hdr);
		return -EINVAL;
	}

	if (tun_cfg->flags & NSS_OVPNMGR_HDR_FLAG_COPY_TOS) {
		nss_ovpnmgr_warn("%p: Copy TOS is not supported\n", tun_hdr);
		return -EINVAL;
	}

	if (!tun_hdr->dst_ip[0] || !tun_hdr->src_ip[0] || !tun_hdr->src_port || !tun_hdr->dst_port) {
		nss_ovpnmgr_warn("%p: IP/UDP Tunnel parameters are invalid.\n", tun_hdr);
		return -EINVAL;
	}
	return 0;
}

/*
 * nss_ovpnmgr_tun_config_dump()
 *	Dump configuration details.
 */
static void nss_ovpnmgr_tun_config_dump(struct net_device *app_dev, struct nss_ovpnmgr_tun_tuple *tun_hdr,
			struct nss_ovpnmgr_tun_config *tun_cfg, struct nss_ovpnmgr_crypto_config *crypto_cfg)
{
	nss_ovpnmgr_info("%p: app_dev = %s\n", app_dev, app_dev->name);
	nss_ovpnmgr_info("flags = %x, peer_id = %u\n", tun_cfg->flags, tun_cfg->peer_id);

	if (tun_cfg->flags & NSS_OVPNMGR_HDR_FLAG_IPV6) {
		nss_ovpnmgr_info("Remote tunnel IP = %pI6c\n", tun_hdr->dst_ip);
		nss_ovpnmgr_info("local tunnel IP = %pI6c\n", tun_hdr->src_ip);
		nss_ovpnmgr_info("local tunnel Port = %d\n", tun_hdr->src_port);
		nss_ovpnmgr_info("remote tunnel Port = %d\n", tun_hdr->dst_port);
	} else {
		nss_ovpnmgr_info("Remote tunnel IP = %pI4\n", &tun_hdr->dst_ip[0]);
		nss_ovpnmgr_info("local tunnel IP = %pI4\n", &tun_hdr->src_ip[0]);
		nss_ovpnmgr_info("local tunnel Port = %d\n", tun_hdr->src_port);
		nss_ovpnmgr_info("remote tunnel Port = %d\n", tun_hdr->dst_port);
	}
	nss_ovpnmgr_info("Crypto: algo = %d, cipher_keylen = %d, hmac_keylen = %d\n",
			crypto_cfg->algo, crypto_cfg->encrypt.cipher_keylen,
			crypto_cfg->encrypt.hmac_keylen);
}

/*
 * nss_ovpnmgr_tun_get_stats()
 *	Update offload tunnel statistics
 */
void nss_ovpnmgr_tun_get_stats(struct net_device *app_dev, struct rtnl_link_stats64 *stats)
{
	struct nss_ovpnmgr_app *app;
	struct nss_ovpnmgr_tun *tun;

	write_lock_bh(&ovpnmgr_ctx.lock);

	app = nss_ovpnmgr_app_find(app_dev);
	if (!app) {
		write_unlock_bh(&ovpnmgr_ctx.lock);
		return;
	}

	/*
	 * Get per tunnel statistics.
	 */
	list_for_each_entry(tun, &app->tun_list, list) {
		nss_ovpnmgr_tun_stats_update(tun, stats);
	}

	write_unlock_bh(&ovpnmgr_ctx.lock);
}

/*
 * nss_ovpnmgr_tun_route_update()
 *	Update route state
 */
void nss_ovpnmgr_tun_route_update(uint32_t tunnel_id, uint32_t *from_addr, uint32_t *to_addr, int version)
{
	struct nss_ovpnmgr_route_tuple rt;
	struct nss_ovpnmgr_tun *tun;
	struct net_device *tun_dev;
	int addr_size;

	tun_dev = dev_get_by_index(&init_net, tunnel_id);
	if (!tun_dev) {
		nss_ovpnmgr_warn("Couldn't find tunnel with tunnel_id = %d\n", tunnel_id);
		return;
	}

	tun = netdev_priv(tun_dev);

	addr_size = version == IPVERSION ? sizeof(rt.ip_addr[0]) : sizeof(rt.ip_addr);

	memset(&rt, 0, sizeof(rt));

	rt.ip_version = version;
	memcpy(rt.ip_addr, from_addr, addr_size);

	write_lock_bh(&ovpnmgr_ctx.lock);
	/*
	 * Set route active for from_addr.
	 */
	if (!nss_ovpnmgr_route_set_active(&tun->route_list, &rt)) {
		goto done;
	}

	/*
	 * Route is not found for from_addr, set route active for to_addr.
	 */
	memcpy(rt.ip_addr, to_addr, addr_size);

	nss_ovpnmgr_route_set_active(&tun->route_list, &rt);
done:
	write_unlock_bh(&ovpnmgr_ctx.lock);
	dev_put(tun_dev);
}
EXPORT_SYMBOL(nss_ovpnmgr_tun_route_update);

/*
 * nss_ovpnmgr_tun_stats_get()
 *	Get statistics of tunnel
 */
int nss_ovpnmgr_tun_stats_get(uint32_t tunnel_id, struct nss_ovpnmgr_tun_stats *stats)
{
	struct nss_ovpnmgr_tun *tun;
	struct net_device *tun_dev;

	tun_dev = dev_get_by_index(&init_net, tunnel_id);
	if (!tun_dev) {
		nss_ovpnmgr_warn("Couldn't find tunnel with tunnel_id = %d\n", tunnel_id);
		return -EINVAL;
	}

	tun = netdev_priv(tun_dev);

	write_lock_bh(&ovpnmgr_ctx.lock);
	stats->tun_read_bytes  = tun->stats.tun_read_bytes;
	stats->tun_write_bytes = tun->stats.tun_write_bytes;
	stats->link_read_bytes = tun->stats.link_read_bytes;
	stats->link_read_bytes_auth = tun->stats.link_read_bytes_auth;
	stats->link_write_bytes = tun->stats.link_write_bytes;
	memset(&tun->stats, 0, sizeof(tun->stats));
	write_unlock_bh(&ovpnmgr_ctx.lock);

	dev_put(tun_dev);
	return 0;
}
EXPORT_SYMBOL(nss_ovpnmgr_tun_stats_get);

/*
 * nss_ovpnmgr_tun_tx()
 *	Transmit packet to NSS firmware for encapsulation/decapsulation.
 */
int nss_ovpnmgr_tun_tx(uint32_t tunnel_id, struct nss_ovpnmgr_metadata *mdata, struct sk_buff *skb)
{
	struct nss_ovpnmgr_tun_ctx_stats *stats;
	struct nss_ovpnmgr_tun *tun;
	struct net_device *tun_dev;
	nss_tx_status_t status;
	bool expand_skb = false;
	uint32_t ifnum;
	uint16_t nhead;
	uint16_t ntail;
	int err;

	tun_dev = dev_get_by_index(&init_net, tunnel_id);
	if (!tun_dev) {
		nss_ovpnmgr_warn("%p: Couldn't find tunnel with tunnel_id = %d\n", skb, mdata->tunnel_id);
		return -EINVAL;
	}

	tun = netdev_priv(tun_dev);
	nss_ovpnmgr_info("%p: metadata flags (0x%x)\n", tun, mdata->flags);

	nhead = tun_dev->needed_headroom;
	ntail = tun_dev->needed_tailroom;

	read_lock_bh(&ovpnmgr_ctx.lock);
	if (mdata->flags & NSS_OVPNMGR_METADATA_FLAG_PKT_DECAP) {
		ifnum = tun->outer.ifnum;
		stats = &tun->outer.stats;
	} else {
		ifnum = tun->inner.ifnum;
		stats = &tun->inner.stats;
	}
	read_unlock_bh(&ovpnmgr_ctx.lock);

	/*
	 * Check if skb is shared
	 */
	if (unlikely(skb_shared(skb))) {
		skb = skb_unshare(skb, in_atomic() ? GFP_ATOMIC : GFP_KERNEL);
		if (!skb) {
			nss_ovpnmgr_trace("%p: unable to expand skb, dev = %s\n", tun, tun_dev->name);
			err = -ENOSPC;
			goto free;
		}
	}

	if (skb_cloned(skb) || (skb_headroom(skb) < nhead) || (skb_tailroom(skb) < ntail)) {
		expand_skb = true;
	}

	if (expand_skb && pskb_expand_head(skb, nhead, ntail, in_atomic() ? GFP_ATOMIC : GFP_KERNEL)) {
		nss_ovpnmgr_trace("%p: unable to expand skb, dev = %s\n", tun, tun_dev->name);
		err = -ENOMEM;
		goto free;
	}

	/*
	 * Dump IPv4/v6 header (40) + transport header (8) + ovpn header (4)
	 */
	NSS_OVPNMGR_TUN_PKT_DUMP(skb, "Pkt to NSS", 0, 52);

	status = nss_qvpn_tx_buf(tun->nss_ctx, ifnum, skb);
	if (status != NSS_TX_SUCCESS) {
		nss_ovpnmgr_trace("%p: Packet offload failed, status = %d\n", tun, status);
		err = (status == NSS_TX_FAILURE_QUEUE) ? -EBUSY : -EINVAL;
		goto free;
	}

	dev_put(tun_dev);
	return 0;
free:
	write_lock_bh(&ovpnmgr_ctx.lock);
	stats->host_pkt_drop++;
	write_unlock_bh(&ovpnmgr_ctx.lock);

	dev_put(tun_dev);
	return err;
}
EXPORT_SYMBOL(nss_ovpnmgr_tun_tx);

/*
 * nss_ovpnmgr_tun_del()
 *	Delete tunnel.
 */
int nss_ovpnmgr_tun_del(uint32_t tunnel_id)
{
	struct nss_ovpnmgr_route *route, *n;
	struct nss_ovpnmgr_tun *tun;
	struct net_device *tun_dev;

	tun_dev = dev_get_by_index(&init_net, tunnel_id);
	if (!tun_dev) {
		nss_ovpnmgr_warn("Couldn't find tunnel: tunnel_id = %u\n\n", tunnel_id);
		return -EINVAL;
	}

	tun = netdev_priv(tun_dev);

	nss_ovpnmgr_info("%p: Deleting Tunnel, tunnel_id = %d\n", tun, tun->tunnel_id);

	write_lock_bh(&ovpnmgr_ctx.lock);
	/*
	 * Delete Routes from the tunnel.
	 */
	list_for_each_entry_safe(route, n, &tun->route_list, list) {
		list_del(&route->list);
		kfree(route);
	}

	/*
	 * Remove this tunnel from tunnel list.
	 */
	list_del(&tun->list);
	write_unlock_bh(&ovpnmgr_ctx.lock);

	/*
	 * Release tun_dev before freeing tunnel.
	 */
	dev_put(tun_dev);
	/*
	 * Unregister ovpn netdevice.
	 */
	rtnl_is_locked() ? unregister_netdevice(tun_dev) : unregister_netdev(tun_dev);
	return 0;
}
EXPORT_SYMBOL(nss_ovpnmgr_tun_del);

/*
 * nss_ovpnmgr_tun_add()
 *	Add new tunnel.
 */
uint32_t nss_ovpnmgr_tun_add(struct net_device *app_dev,
			struct nss_ovpnmgr_tun_tuple *tun_hdr,
			struct nss_ovpnmgr_tun_config *tun_cfg,
			struct nss_ovpnmgr_crypto_config *crypto_cfg)
{
	struct nss_ovpnmgr_app *app;
	struct nss_ovpnmgr_tun *tun;
	struct net_device *tun_dev;
	int status;

	nss_ovpnmgr_tun_config_dump(app_dev, tun_hdr, tun_cfg, crypto_cfg);

	/*
	 * Veryify tunnel configuration parameters.
	 */
	status = nss_ovpnmgr_tun_config_verify(tun_hdr, tun_cfg);
	if (status < 0) {
		nss_ovpnmgr_warn("%p: Tunnel configuration parameters are invalid, status=%d.\n", app_dev, status);
		return 0;
	}

	read_lock_bh(&ovpnmgr_ctx.lock);
	app = nss_ovpnmgr_app_find(app_dev);
	if (!app) {
		read_unlock_bh(&ovpnmgr_ctx.lock);
		nss_ovpnmgr_warn("%p: Couldn't find app for app_dev=%s\n", app_dev, app_dev->name);
		return 0;
	}

	/*
	 * Hold the device until we add the tunnel.
	 */
	dev_hold(app_dev);
	read_unlock_bh(&ovpnmgr_ctx.lock);

	tun = nss_ovpnmgr_tun_alloc();
	if (!tun) {
		nss_ovpnmgr_warn("%p: Tunnel allocation failed.\n", app);
		dev_put(app_dev);
		return 0;
	}

	tun_dev = tun->dev;

	/*
	 * Copy tunnel header parameters.
	 */
	memcpy(&tun->tun_hdr, tun_hdr, sizeof(*tun_hdr));
	tun->tun_cfg.flags = tun_cfg->flags;

	/*
	 * Initialize tunnel inner context.
	 */
	if (nss_ovpnmgr_tun_ctx_inner_init(tun, crypto_cfg)) {
		nss_ovpnmgr_error("%p: Failed to initialize inner context.\n", app);
		goto free_tun;
	}

	/*
	 * Initialize tunnel outer context.
	 */
	if (nss_ovpnmgr_tun_ctx_outer_init(tun, crypto_cfg)) {
		nss_ovpnmgr_error("%p: Failed to initialize outer context.\n", app);
		goto free_inner;
	}

	/*
	 * Configure inner node.
	 */
	if (nss_ovpnmgr_tun_inner_config(tun)) {
		nss_ovpnmgr_warn("%p: inner node tunnel configuration failed\n", tun);
		goto free_outer;
	}

	/*
	 * Configure outer node.
	 */
	if (nss_ovpnmgr_tun_outer_config(tun)) {
		nss_ovpnmgr_warn("%p: Outer node tunnel configuration failed\n", tun);
		goto free_inner_config;
	}

	/*
	 * Register netdev.
	 */
	status = rtnl_is_locked() ? register_netdevice(tun_dev) : register_netdev(tun_dev);
	if (status < 0) {
		nss_ovpnmgr_warn("register net dev failed :%s\n", tun_dev->name);
		goto free_outer_config;
	}

	/*
	 * Assign ifindex as tunnel_id as it is unique.
	 */
	tun->tunnel_id = tun_dev->ifindex;
	tun->app = app;

	write_lock_bh(&ovpnmgr_ctx.lock);
	list_add(&tun->list, &app->tun_list);
	write_unlock_bh(&ovpnmgr_ctx.lock);

	dev_put(app_dev);
	return tun->tunnel_id;

free_outer_config:
	nss_ovpnmgr_tun_deconfig(&tun->outer, tun);
free_inner_config:
	nss_ovpnmgr_tun_deconfig(&tun->inner, tun);
free_outer:
	nss_ovpnmgr_tun_ctx_deinit(&tun->outer);
free_inner:
	nss_ovpnmgr_tun_ctx_deinit(&tun->inner);
free_tun:
	free_netdev(tun_dev);
	dev_put(app_dev);

	return 0;
}
EXPORT_SYMBOL(nss_ovpnmgr_tun_add);
