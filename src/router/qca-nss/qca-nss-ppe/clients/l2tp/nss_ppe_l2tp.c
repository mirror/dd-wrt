/*
 * Copyright (c) 2023-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include <linux/module.h>
#include <linux/l2tp.h>
#include <l2tp_core.h>
#include <ppe_drv.h>
#include <nss_ppe_tun_drv.h>
#include "nss_ppe_l2tp.h"

static struct nss_ppe_l2tp l2tp_gbl;

static uint8_t encap_ecn_mode = PPE_DRV_TUN_CMN_CTX_ENCAP_ECN_NO_UPDATE;
module_param(encap_ecn_mode, byte, 0644);
MODULE_PARM_DESC(encap_ecn_mode, "Encap ECN mode 0:NO_UPDATE, 1:RFC3168_LIMIT_RFC6040_CMPAT, 2:RFC3168_FULL, 3:RFC4301_RFC6040_NORMAL");

static uint8_t decap_ecn_mode = PPE_DRV_TUN_CMN_CTX_DECAP_ECN_RFC3168_MODE;
module_param(decap_ecn_mode, byte, 0644);
MODULE_PARM_DESC(decap_ecn_mode, "Decap ECN mode 0:RFC3168, 1:RFC4301, 2:RFC6040");

static bool inherit_dscp = false;
module_param(inherit_dscp, bool, 0644);
MODULE_PARM_DESC(inherit_dscp, "DSCP 0:Dont Inherit inner, 1:Inherit inner");

static bool inherit_ttl = false;
module_param(inherit_ttl, bool, 0644);
MODULE_PARM_DESC(inherit_ttl, "TTL 0:Dont Inherit inner, 1:Inherit inner");

/*
 * nss_ppe_l2tp_src_exception()
 *	handle source VP exception packets
 */
static bool nss_ppe_l2tp_src_exception(struct ppe_vp_cb_info *info, ppe_tun_data *tun_data)
{
	struct sk_buff *skb = info->skb;
	struct net_device *dev = skb->dev;
	int ret;
	const struct iphdr *iph;

	skb_reset_network_header(skb);
	iph = (const struct iphdr *)skb->data;
	if (iph->version == IPVERSION) {
		skb->protocol = htons(ETH_P_IP);
	} else {
		skb->protocol = htons(ETH_P_IPV6);
	}

	skb->pkt_type = PACKET_HOST;
	/*
	 * Reset Skb flags
	 */
	skb->fast_xmit = 0;
	skb->fast_recycled = 0;
	skb->recycled_for_ds = 0;
	ret = netif_receive_skb(skb);
	if (ret != NET_RX_SUCCESS) {
		nss_ppe_l2tp_trace("%p: exception packet dropped\n", dev);
	}
	return true;
}

/*
 * nss_ppe_l2tp_get_session_and_tunnel_info()
 *	get session and tunnel information from netdev
 */
static bool nss_ppe_l2tp_get_session_and_tunnel_info(struct net_device *dev, struct l2tp_session **sess, struct l2tp_tunnel **tun)
{
	struct ppp_channel *ppp_chan[NSS_PPE_L2TP_CHANNEL_MAX] = {NULL};
	int px_proto;
	struct pppol2tp_common_addr info = {0};
	struct l2tp_session *session = NULL;
	struct l2tp_tunnel *tunnel = NULL;

	/*
	 * check whether the interface is of type PPP
	 */
	if (dev->type != ARPHRD_PPP || !(dev->flags & IFF_POINTOPOINT)) {
		nss_ppe_l2tp_warning("netdevice is not a PPP tunnel type\n");
		return false;
	}

	if (ppp_is_multilink(dev)) {
		nss_ppe_l2tp_warning("channel is multilink PPP\n");
		return false;
	}

	if (ppp_hold_channels(dev, ppp_chan, NSS_PPE_L2TP_CHANNEL_MAX) != NSS_PPE_L2TP_CHANNEL_MAX) {
		nss_ppe_l2tp_warning("hold channel for netdevice failed\n");
		return false;
	}

	nss_ppe_l2tp_assert(ppp_chan[0], "ppp channel list returned is NULL\n");

	px_proto = ppp_channel_get_protocol(ppp_chan[0]);
	if (px_proto != PX_PROTO_OL2TP) {
		nss_ppe_l2tp_warning("session socket is not of type PX_PROTO_OL2TP\n");
		ppp_release_channels(ppp_chan, NSS_PPE_L2TP_CHANNEL_MAX);
		return false;
	}

	if (pppol2tp_channel_addressing_get(ppp_chan[0], &info)) {
		nss_ppe_l2tp_warning("l2tp channel info get failed\n");
		ppp_release_channels(ppp_chan, NSS_PPE_L2TP_CHANNEL_MAX);
		return false;
	}

	tunnel = l2tp_tunnel_get(dev_net(dev), info.local_tunnel_id);
	if (!tunnel) {
		nss_ppe_l2tp_warning("l2tp tunnel get failed\n");
		ppp_release_channels(ppp_chan, NSS_PPE_L2TP_CHANNEL_MAX);
		return false;
	}

	if (tunnel->version != NSS_PPE_L2TP_VER_2) {
		nss_ppe_l2tp_warning("ppe doesnt support l2tp version %d offload\n", tunnel->version);
		l2tp_tunnel_dec_refcount(tunnel);
		ppp_release_channels(ppp_chan, NSS_PPE_L2TP_CHANNEL_MAX);
		return false;
	}

	session = l2tp_tunnel_get_session(tunnel, info.local_session_id);
	if (!session) {
		nss_ppe_l2tp_warning("l2tp session get from tunnel failed\n");
		l2tp_tunnel_dec_refcount(tunnel);
		ppp_release_channels(ppp_chan, NSS_PPE_L2TP_CHANNEL_MAX);
		return false;
	}

	*tun = tunnel;
	*sess = session;
	ppp_release_channels(ppp_chan, NSS_PPE_L2TP_CHANNEL_MAX);

	return true;
}

/*
 * nss_ppe_l2tp_dev_parse_param
 *	Parse l2tp session param
 */
static bool nss_ppe_l2tp_dev_parse_param(struct net_device *netdev, struct ppe_drv_tun_cmn_ctx *tun_hdr)
{
	struct l2tp_session *session = NULL;
	struct l2tp_tunnel *tunnel = NULL;
	struct inet_sock *inet;
	struct udp_sock *udp;
	uint16_t sport, dport;

	if (!nss_ppe_l2tp_get_session_and_tunnel_info(netdev, &session, &tunnel)) {
		nss_ppe_l2tp_warning("%px: l2tp session or tunnel get failed for %s\n", netdev, netdev->name);
		return false;
	}

	if (!tunnel || !session) {
		nss_ppe_l2tp_warning("%px: l2tp session/tunnel info is NULL %p, %p\n", netdev, tunnel, session);
		return false;
	}

	if (session->reorder_timeout || session->recv_seq || session->send_seq) {
		nss_ppe_l2tp_warning("PPE doesnt support sequence numbers or reordering of data packets\n");
		goto fail;
	}

	if (tunnel->encap != L2TP_ENCAPTYPE_UDP) {
		nss_ppe_l2tp_warning("encap type is not UDP\n");
		goto fail;
	}

	if (!tunnel->sock) {
		nss_ppe_l2tp_warning("tunnel sock is NULL\n");
		goto fail;
	}

	sock_hold(tunnel->sock);
	inet = inet_sk(tunnel->sock);

	if (tunnel->sock->sk_protocol != IPPROTO_UDP) {
		sock_put(tunnel->sock);
		nss_ppe_l2tp_warning("Wrong protocol %u\n", tunnel->sock->sk_protocol);
		goto fail;
	}

	if (tunnel->sock->sk_state != PPPOX_CONNECTED) {
		sock_put(tunnel->sock);
		nss_ppe_l2tp_warning("Tunnel socket is not in connected state %u\n", tunnel->sock->sk_state);
		goto fail;
	}

	udp = udp_sk(tunnel->sock);
	if (!udp) {
		sock_put(tunnel->sock);
		nss_ppe_l2tp_warning("Error getting UDP socket\n");
		goto fail;
	}

	if (inet->sk.sk_family != AF_INET) {
		sock_put(tunnel->sock);
		nss_ppe_l2tp_warning("Only Ipv4 protocol supports l2tpv2 packet's outer IP\n");
		goto fail;
	}

	sock_put(tunnel->sock);

	tun_hdr->l3.saddr[0] = inet->inet_saddr;
	tun_hdr->l3.daddr[0] = inet->inet_daddr;
	tun_hdr->l3.proto = IPPROTO_UDP;
	tun_hdr->l3.flags |= PPE_DRV_TUN_CMN_CTX_L3_IPV4;
	tun_hdr->l3.ttl = l2tp_gbl.outer_ttl;

	if (tunnel->sock->sk_no_check_tx) {
		tun_hdr->l3.flags |= PPE_DRV_TUN_CMN_CTX_L3_UDP_ZERO_CSUM_TX;
	}

	if (inherit_ttl) {
		tun_hdr->l3.flags |= PPE_DRV_TUN_CMN_CTX_L3_INHERIT_TTL;
	}

	if (inherit_dscp) {
		tun_hdr->l3.flags |=  PPE_DRV_TUN_CMN_CTX_L3_INHERIT_DSCP;
	}

	if (encap_ecn_mode <= PPE_DRV_TUN_CMN_CTX_ENCAP_ECN_RFC4301_RFC6040_NORMAL_MODE) {
		tun_hdr->l3.encap_ecn_mode = encap_ecn_mode;
	}

	if (decap_ecn_mode <= PPE_DRV_TUN_CMN_CTX_DECAP_ECN_RFC6040_MODE) {
		tun_hdr->l3.decap_ecn_mode = decap_ecn_mode;
	}

	if (!ppe_tun_l2tp_port_get(&sport, &dport)) {
		nss_ppe_l2tp_warning("Connot get port configurations from ppe tun\n");
		goto fail;
	}

	tun_hdr->tun.l2tp.sport = inet->inet_sport;
	tun_hdr->tun.l2tp.dport = inet->inet_dport;

	if ((sport != ntohs(tun_hdr->tun.l2tp.sport)) ||
			(dport != ntohs(tun_hdr->tun.l2tp.dport))) {
		nss_ppe_l2tp_warning("UDP port configuration mismatch in PPE Configured sport: %d, dport: %d,\
				      L2TP tunnel UDP port value sport: %d, dport: %d\n", sport, dport,
				      ntohs(tun_hdr->tun.l2tp.sport), ntohs(tun_hdr->tun.l2tp.dport));
		nss_ppe_l2tp_warning("Use \" echo \"src_port=<value> dest_port=<value>\" > /sys/kernel/debug/l2tp/l2tp_port \" \
					to change port sport and dport values\n");
		goto fail;
	}

	tun_hdr->tun.l2tp.tunnel_id = tunnel->tunnel_id;
	tun_hdr->tun.l2tp.peer_tunnel_id = tunnel->peer_tunnel_id;

	tun_hdr->tun.l2tp.session_id = session->session_id;
	tun_hdr->tun.l2tp.peer_session_id = session->peer_session_id;

	/*
	 * Release references taken on tunnel and session
	 */
	l2tp_session_dec_refcount(session);
	l2tp_tunnel_dec_refcount(tunnel);

	return true;

fail:
	l2tp_session_dec_refcount(session);
	l2tp_tunnel_dec_refcount(tunnel);
	return false;
}


/*
 * netif_is_l2tp()
 *	check if netdev is of type PPP tunnel(L2TP)
 */
static bool netif_is_l2tp(struct net_device *dev)
{
	if ((dev->type == ARPHRD_PPP || (dev->flags & IFF_POINTOPOINT)) &&
			(dev->priv_flags_ext & IFF_EXT_PPP_L2TPV2)) {
		return true;
	}

	return false;
}

/*
 * nss_ppe_l2tp_dev_stats_update()
 *	Update L2TP dev statistics
 */
bool nss_ppe_l2tp_dev_stats_update(struct net_device *dev, ppe_tun_hw_stats *stats, ppe_tun_data *tun_data)
{
	struct l2tp_session *session = NULL;
	struct l2tp_tunnel *tunnel = NULL;
	struct l2tp_stats l2tp_stats = {0};
	struct ppe_tun_l2tp_info *l2tp_info;

	if (!dev) {
		nss_ppe_l2tp_trace("L2TP stats update dev is NULL\n");
		return false;
	}

	if (!netif_is_l2tp(dev)) {
		nss_ppe_l2tp_trace("Not an L2TP netdevice\n");
		return false;
	}

	if (!tun_data) {
		nss_ppe_l2tp_trace("L2TP stats callback data is NULL\n");
		return false;
	}

	l2tp_info = &tun_data->l2tp_info;

	dev_hold(dev);
	tunnel = l2tp_tunnel_get(dev_net(dev), l2tp_info->tunnel_id);
	if (!tunnel) {
		nss_ppe_l2tp_warning("l2tp tunnel get failed\n");
		dev_put(dev);
		return false;
	}

	session = l2tp_tunnel_get_session(tunnel, l2tp_info->session_id);
	if (!session) {
		nss_ppe_l2tp_warning("l2tp session get from tunnel failed\n");
		l2tp_tunnel_dec_refcount(tunnel);
		dev_put(dev);
		return false;
	}

	atomic_long_set(&l2tp_stats.tx_packets, (long)stats->tx_pkt_cnt);
	atomic_long_set(&l2tp_stats.tx_bytes, (long)stats->tx_byte_cnt);
	atomic_long_set(&l2tp_stats.rx_packets, (long)stats->rx_pkt_cnt);
	atomic_long_set(&l2tp_stats.rx_bytes, (long)stats->rx_byte_cnt);
	l2tp_stats_update(tunnel, session, &l2tp_stats);

	/*
	 * Update ppp interface stats
	 */
	ppp_update_stats(dev,
			(unsigned long)stats->rx_pkt_cnt,
			(unsigned long)stats->rx_byte_cnt,
			(unsigned long)stats->tx_pkt_cnt,
			(unsigned long)stats->tx_byte_cnt,
			(unsigned long)0,
			(unsigned long)0,
			(unsigned long)stats->rx_drop_pkt_cnt,
			(unsigned long)stats->tx_drop_pkt_cnt);

	l2tp_session_dec_refcount(session);
	l2tp_tunnel_dec_refcount(tunnel);
	dev_put(dev);

	return true;
}

/*
 * nss_ppe_l2tp_dev_event()
 *      Net device notifier for l2tp module
 */
int nss_ppe_l2tp_dev_event(struct notifier_block  *nb, unsigned long event, void  *info)
{
	struct net_device *netdev = netdev_notifier_info_to_dev(info);
	bool status;
	struct ppe_drv_tun_cmn_ctx *tun_hdr = NULL;
	struct ppe_tun_excp *tun_cb = NULL;
	ppe_tun_data tun_data = {0};

	/*
	 * Proceed to handle event only if its L2TP PPP netdevice
	 */
	if (!netif_is_l2tp(netdev)) {
		return NOTIFY_DONE;
	}

	switch (event) {
	case NETDEV_UP:
		nss_ppe_l2tp_trace("%px: NETDEV_UP :event %lu name %s\n", netdev, event, netdev->name);

		tun_hdr = kzalloc(sizeof(struct ppe_drv_tun_cmn_ctx), GFP_ATOMIC);
		if (!tun_hdr) {
			nss_ppe_l2tp_warning("%px: memory allocation for tunnel %s failed\n", netdev, netdev->name);
			break;
		}

		if (!nss_ppe_l2tp_dev_parse_param(netdev, tun_hdr)) {
			nss_ppe_l2tp_warning("%px: PPE acceleration configuration failed\n", netdev);
			kfree(tun_hdr);
			break;
		}

		tun_hdr->type = PPE_DRV_TUN_CMN_CTX_TYPE_L2TP_V2;
		tun_data.l2tp_info.tunnel_id = tun_hdr->tun.l2tp.tunnel_id;
		tun_data.l2tp_info.session_id = tun_hdr->tun.l2tp.session_id;

		tun_cb = kzalloc(sizeof(struct ppe_tun_excp), GFP_ATOMIC);
		if (!tun_cb) {
			nss_ppe_l2tp_warning("%px: memory allocation for tunnel callback failed for device %s\n", netdev, netdev->name);

			kfree(tun_hdr);
			break;
		}

		tun_cb->src_excp_method = nss_ppe_l2tp_src_exception;
		tun_cb->stats_update_method = nss_ppe_l2tp_dev_stats_update;
		tun_cb->tun_data = &tun_data;

		/*
		 * Allocate tunnel and configure if its a L2TP netdevice
		 */
		status = ppe_tun_alloc(netdev, PPE_DRV_TUN_CMN_CTX_TYPE_L2TP_V2);
		if (status) {
			nss_l2tp_stats_dentry_create(netdev);
		}

		/*
		 * Configure L2TP tunnel in PPE
		 */
		if (!(ppe_tun_configure(netdev, tun_hdr, tun_cb))) {
			nss_ppe_l2tp_trace("%px: Not able to create tunnel for dev: %s\n", netdev, netdev->name);
			nss_l2tp_stats_dentry_free(netdev);
			ppe_tun_free(netdev);
		}

		kfree(tun_hdr);
		kfree(tun_cb);
		break;

	case NETDEV_DOWN:
		nss_ppe_l2tp_trace("%px: NETDEV_DOWN :event %lu name %s\n", netdev, event, netdev->name);
		/*
		 * Deconfigure and free L2TP tunnel when NETDEV_DOWN is received
		 */
		ppe_tun_deconfigure(netdev);
		ppe_tun_free(netdev);
		nss_l2tp_stats_dentry_free(netdev);
		break;

	case NETDEV_CHANGEMTU:
		nss_ppe_l2tp_trace("%px: NETDEV_CHANGEMTU :event %lu name %s\n", netdev, event, netdev->name);
		ppe_tun_mtu_set(netdev, netdev->mtu);
		break;

	default:
		nss_ppe_l2tp_trace("%px: Unhandled notifier dev %s event %x\n", netdev, netdev->name, (int)event);
		break;
	}

	return NOTIFY_DONE;
}

/*
 * nss_ppe_l2tp_stats_show()
 *	Read ppe tunnel statistics.
 */
static int nss_ppe_l2tp_stats_show(struct seq_file *m, void __attribute__((unused))*ptr)
{
	struct net_device *dev = (struct net_device *)m->private;
	uint64_t exception_packet;
	uint64_t exception_bytes;

	ppe_tun_exception_packet_get(dev, &exception_packet, &exception_bytes);
	seq_printf(m, "\n################ PPE Client l2tp Statistics Start ################\n");
	seq_printf(m, "dev: %s\n", dev->name);
	seq_printf(m, "  Exception:\n");
	seq_printf(m, "\t exception packet: %llu\n", exception_packet);
	seq_printf(m, "\t exception bytes: %llu\n", exception_bytes);
	seq_printf(m, "\n################ PPE Client l2tp Statistics End ################\n");

	return 0;
}

/*
 * nss_ppe_l2tp_stats_open()
 */
static int nss_ppe_l2tp_stats_open(struct inode *inode, struct file *file)
{
	return single_open(file, nss_ppe_l2tp_stats_show, inode->i_private);
}

/*
 * nss_ppe_l2tp_stats_ops
 *	File operations for l2tp tunnel stats
 */
static const struct file_operations nss_ppe_l2tp_stats_ops = {
	.open = nss_ppe_l2tp_stats_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release
};

/*
 * nss_l2tp_stats_dentry_create()
 *	Create dentry for a given netdevice.
 */
static bool nss_l2tp_stats_dentry_create(struct net_device *dev)
{
	char dentry_name[IFNAMSIZ];
	struct dentry *dentry;
	scnprintf(dentry_name, sizeof(dentry_name), "%s_stats", dev->name);

	dentry = debugfs_create_file(dentry_name, S_IRUGO,
			l2tp_gbl.l2tp_dentry, dev, &nss_ppe_l2tp_stats_ops);
	if (!dentry) {
		nss_ppe_l2tp_warning("%px: Debugfs file creation failed for device %s\n", dev, dev->name);
		return false;
	}

	return true;
}

/*
 * nss_l2tp_stats_dentry_free()
 *	Remove dentry for a given netdevice.
 */
static bool nss_l2tp_stats_dentry_free(struct net_device *dev)
{
	char dentry_name[IFNAMSIZ];
	struct dentry *dentry;
	scnprintf(dentry_name, sizeof(dentry_name), "%s_stats", dev->name);

	dentry = debugfs_lookup(dentry_name, l2tp_gbl.l2tp_dentry);
	if (dentry) {
		debugfs_remove(dentry);
		nss_ppe_l2tp_trace("%px: removed stats debugfs entry for dev %s", dev, dentry_name);
		return true;
	}

	nss_ppe_l2tp_trace("%px: Could not find stats debugfs entry for dev %s", dev, dentry_name);
	return false;
}

/*
 * nss_ppe_l2tp_port_write()
 * 	Update UDP port values used for L2TP
 */
static ssize_t nss_ppe_l2tp_port_write(struct file *f, const char *buffer, size_t len, loff_t *offset)
{
	ssize_t size;
	char data[50];
	int ret, src_port, dest_port;

	size = simple_write_to_buffer(data, sizeof(data), offset, buffer, len);
	if (size < 0) {
		nss_ppe_l2tp_trace("Error reading the input for l2tp port configuration");
		return size;
	}

	ret = sscanf(data, "src_port=%d dest_port=%d", &src_port, &dest_port);
	if (ret != 2) {
		printk("syntax error: please provide input in \"src_port=<value> dest_port=<value>\" format\n");
		return -EINVAL;
	}

	if (src_port < 0 || src_port > 65535 || dest_port < 0 || dest_port > 65535) {
		printk("Invalid l2tp port configuration\n");
		return -EINVAL;
	}

	if (!ppe_tun_l2tp_port_set(src_port, dest_port)) {
		printk("Port configuration failed, Make sure all L2TP tunnel instaces are down before changing port values\n");
		return -EPERM;
	}

	return len;
}

/*
 * nss_ppe_l2tp_port_read()
 * 	Get UDP port values used for L2TP
 */
static ssize_t nss_ppe_l2tp_port_read(struct file *f, char *buf, size_t count, loff_t *offset)
{
	int len;
	char lbuf[70];
	uint16_t sport, dport;

	ppe_tun_l2tp_port_get(&sport, &dport);

	len = snprintf(lbuf, sizeof(lbuf), "L2TP src port: %u  dest port: %u\n", sport, dport);

	return simple_read_from_buffer(buf, count, offset, lbuf, len);
}

/*
 * nss_ppe_l2tp_port_ops
 *	File operations for l2tp tunnel port setting
 */
static const struct file_operations nss_ppe_l2tp_port_ops = {
	.owner = THIS_MODULE,
	.write = nss_ppe_l2tp_port_write,
	.read = nss_ppe_l2tp_port_read,
};

/*
 * nss_ppe_l2tp_ttl_write()
 * Update ttl values used for L2TP
 */
static ssize_t nss_ppe_l2tp_ttl_write(struct file *f, const char *buffer, size_t len, loff_t *offset)
{
	ssize_t size;
	char data[50];
	int ret, ttl;

	size = simple_write_to_buffer(data, sizeof(data), offset, buffer, len);
	if (size < 0) {
		nss_ppe_l2tp_trace("Error reading the input for l2tp outer ttl configuration");
		return size;
	}

	ret = sscanf(data, "%d", &ttl);
	if (ret != 1) {
		printk("syntax error: please provide input in \"<value>\" format\n");
		return -EINVAL;
	}

	if (ttl < 0 || ttl > 255) {
		printk("Invalid l2tp ttl configuration\n");
		return -EINVAL;
	}

	l2tp_gbl.outer_ttl = ttl;

	return len;
}

/*
 * nss_ppe_l2tp_ttl_read()
 * Get ttl value used for L2TP
 */
static ssize_t nss_ppe_l2tp_ttl_read(struct file *f, char *buf, size_t count, loff_t *offset)
{
	int len;
	char lbuf[50];

	len = snprintf(lbuf, sizeof(lbuf), "outer TTL set = %d\n", l2tp_gbl.outer_ttl);

	return simple_read_from_buffer(buf, count, offset, lbuf, len);
}

/*
 * nss_ppe_l2tp_port_ops
 *	File operations for l2tp tunnel port setting
 */
static const struct file_operations nss_ppe_l2tp_ttl_ops = {
	.owner = THIS_MODULE,
	.write = nss_ppe_l2tp_ttl_write,
	.read = nss_ppe_l2tp_ttl_read,
};

/*
 * nss_l2tp_stats_dentry_deinit()
 *	Cleanup the debugfs tree.
 */
static void nss_ppe_l2tp_dentry_deinit(void)
{
	debugfs_remove_recursive(l2tp_gbl.l2tp_dentry);
	l2tp_gbl.l2tp_dentry = NULL;
}

/*
 * nss_ppe_l2tp_dentry_init()
 *	Create l2tp tunnel statistics debugfs entry.
 */
static bool nss_ppe_l2tp_dentry_init(void)
{
	/*
	 * Initialize debugfs directory.
	 */
	struct dentry *parent;
	struct dentry *clients;
	struct dentry *dentry;

	parent = debugfs_lookup("qca-nss-ppe", NULL);
	if (!parent) {
		nss_ppe_l2tp_warning("parent debugfs entry for qca-nss-ppe not present\n");
		return false;
	}

	clients = debugfs_lookup("clients", parent);
	if (!clients) {
		nss_ppe_l2tp_warning("clients debugfs entry inside qca-nss-ppe not present\n");
		return false;
	}

	l2tp_gbl.l2tp_dentry = debugfs_create_dir("l2tp", clients);
	if (!l2tp_gbl.l2tp_dentry) {
		nss_ppe_l2tp_warning("l2tp debugfs entry inside qca-nss-ppe/clients could not be created\n");
		return false;
	}

	dentry = debugfs_create_file("l2tp_port", 0644, l2tp_gbl.l2tp_dentry, NULL, &nss_ppe_l2tp_port_ops);
	if (!dentry) {
		debugfs_remove_recursive(l2tp_gbl.l2tp_dentry);
		nss_ppe_l2tp_warning("l2tp port configuration file entry could not be created\n");
		return false;
	}

	dentry = debugfs_create_file("l2tp_outer_ttl", 0644, l2tp_gbl.l2tp_dentry, NULL, &nss_ppe_l2tp_ttl_ops);
	if (!dentry) {
		debugfs_remove_recursive(l2tp_gbl.l2tp_dentry);
		nss_ppe_l2tp_warning("l2tp ttl configuration file entry could not be created\n");
		return false;
	}

	return true;
}

/*
 * Linux Net device Notifier
 */
struct notifier_block nss_ppe_l2tp_notifier = {
	.notifier_call = nss_ppe_l2tp_dev_event,
};

/*
 * nss_ppe_l2tp_init_module()
 *      Tunnel l2tp module init function
 */
int __init nss_ppe_l2tp_init_module(void)
{
	/*
	 * Create the debugfs directory for statistics.
	 */
	if (!nss_ppe_l2tp_dentry_init()) {
		nss_ppe_l2tp_trace("Failed to initialize debugfs\n");
		return -1;
	}

	if (encap_ecn_mode > PPE_DRV_TUN_CMN_CTX_ENCAP_ECN_RFC4301_RFC6040_NORMAL_MODE) {
		nss_ppe_l2tp_dentry_deinit();
		nss_ppe_l2tp_warning("Invalid Encap ECN mode %u\n", encap_ecn_mode);
		return -1;
	}

	if (decap_ecn_mode > PPE_DRV_TUN_CMN_CTX_DECAP_ECN_RFC6040_MODE) {
		nss_ppe_l2tp_dentry_deinit();
		nss_ppe_l2tp_warning("Invalid Decap ECN mode %u\n", decap_ecn_mode);
		return -1;
	}

	l2tp_gbl.outer_ttl = NSS_PPE_L2TP_DEFAULT_TTL;
	register_netdevice_notifier(&nss_ppe_l2tp_notifier);
	nss_ppe_l2tp_trace("l2tp PPE driver registered\n");

	return 0;
}

/*
 * nss_ppe_l2tp_exit_module()
 * Tunnel l2tp module exit function
 */
void __exit nss_ppe_l2tp_exit_module(void)
{
	/*
	 * deactivate all L2TP PPE instances.
	 */
	ppe_tun_conf_accel(PPE_DRV_TUN_CMN_CTX_TYPE_L2TP_V2, false);

	/*
	 * De-initialize debugfs.
	 */
	nss_ppe_l2tp_dentry_deinit();

	/*
	 * Unregister net device notification for standard tunnel.
	 */
	unregister_netdevice_notifier(&nss_ppe_l2tp_notifier);

	nss_ppe_l2tp_info("l2tp module unloaded\n");
}

module_init(nss_ppe_l2tp_init_module);
module_exit(nss_ppe_l2tp_exit_module);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS PPE l2tp client driver");
