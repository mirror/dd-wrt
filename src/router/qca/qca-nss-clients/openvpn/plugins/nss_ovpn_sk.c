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
 * nss_ovpn_sk.c
 *	Socket implementation for OVPN.
 */

#include <linux/net.h>
#include <linux/socket.h>
#include <net/sock.h>
#include <net/udp_tunnel.h>
#include <linux/crypto.h>

#include <nss_api_if.h>
#include <nss_qvpn.h>
#include "nss_ovpn_sk.h"
#include "nss_ovpnmgr.h"
#include "nss_ovpn_sk_priv.h"

static struct proto nss_ovpn_sk_proto __read_mostly = {
	.name = "OVPN",
	.owner = THIS_MODULE,
	.obj_size = sizeof(struct nss_ovpn_sk_pinfo),
};

/*
 * nss_ovpn_sk_route_info_copy()
 *	Copy route info passed from application.
 */
static inline void nss_ovpn_sk_route_info_copy(struct nss_ovpnmgr_route_tuple *route, struct nss_ovpn_sk_route_info *route_info)
{
	route->ip_version = route_info->ip_version;
	memcpy(route->ip_addr, route_info->ip_network, sizeof(route_info->ip_network));
}

/*
 * nss_ovpn_sk_route_state_get()
 *	Get route status from OVPN manager.
 */
static int nss_ovpn_sk_route_state_get(struct socket *sock, unsigned long argp)
{
	struct nss_ovpn_sk_route_info route_info;
	struct nss_ovpnmgr_route_tuple route;
	struct nss_ovpn_sk_pinfo *pinfo = (struct nss_ovpn_sk_pinfo *)sock->sk;

	if (!pinfo->dev) {
		nss_ovpn_sk_warn("%p: Application is not registered\n", sock);
		return -EINVAL;
	}

	if (copy_from_user(&route_info, (void *)argp, sizeof(route_info))) {
		nss_ovpn_sk_warn("%p: Failed to copy route information\n", sock);
		return -EFAULT;
	}

	nss_ovpn_sk_route_info_copy(&route, &route_info);
	return nss_ovpnmgr_route_is_active(route_info.tunnel_id, &route);
}

/*
 * nss_ovpn_sk_stats_get()
 *	Get tunnel statistics.
 */
static int nss_ovpn_sk_stats_get(struct socket *sock, unsigned long argp)
{
	struct nss_ovpn_sk_tun_stats tun_stats;
	struct nss_ovpnmgr_tun_stats stats;
	struct nss_ovpn_sk_pinfo *pinfo = (struct nss_ovpn_sk_pinfo *)sock->sk;
	int ret;

	if (!pinfo->dev) {
		nss_ovpn_sk_warn("%p: Application is not registered\n", sock);
		return -EINVAL;
	}

	if (copy_from_user(&tun_stats, (void *)argp, sizeof(tun_stats))) {
		nss_ovpn_sk_warn("%p: Failed to copy tunnel statistics\n", sock);
		return -EFAULT;
	}

	ret = nss_ovpnmgr_tun_stats_get(tun_stats.tunnel_id, &stats);
	if (ret) {
		nss_ovpn_sk_warn("%p: Failed to get statistics for tunnel_id = %u\n", sock, tun_stats.tunnel_id);
		return ret;
	}

	tun_stats.tun_read_bytes  = stats.tun_read_bytes;
	tun_stats.tun_write_bytes = stats.tun_write_bytes;
	tun_stats.link_read_bytes = stats.link_read_bytes;
	tun_stats.link_read_bytes_auth = stats.link_read_bytes_auth;
	tun_stats.link_write_bytes = stats.link_write_bytes;

	if (copy_to_user((void *)argp, &tun_stats, sizeof(tun_stats))) {
		nss_ovpn_sk_warn("%p: Failed to copy statistics\n", sock);
		return -EFAULT;
	}

	return 0;
}

/*
 * nss_ovpn_sk_crypto_key_del()
 *	Delete crypto key.
 */
static int nss_ovpn_sk_crypto_key_del(struct socket *sock, unsigned long argp)
{
	struct nss_ovpn_sk_crypto_session crypto_session;
	struct nss_ovpn_sk_pinfo *pinfo = (struct nss_ovpn_sk_pinfo *)sock->sk;

	if (!pinfo->dev) {
		nss_ovpn_sk_warn("%p: Application is not registered\n", sock);
		return -EINVAL;
	}

	if (copy_from_user(&crypto_session, (void *)argp, sizeof(crypto_session))) {
		nss_ovpn_sk_warn("%p: Failed to copy crypto session\n", sock);
		return -EFAULT;
	}

	return nss_ovpnmgr_crypto_key_del(crypto_session.tunnel_id);
}

/*
 * nss_ovpn_sk_crypto_key_add()
 *	Add crypto key.
 */
static int nss_ovpn_sk_crypto_key_add(struct socket *sock, unsigned long argp)
{
	struct nss_ovpn_sk_crypto_session crypto_info;
	struct nss_ovpnmgr_crypto_config crypto_cfg;
	struct nss_ovpn_sk_pinfo *pinfo = (struct nss_ovpn_sk_pinfo *)sock->sk;

	if (!pinfo->dev) {
		nss_ovpn_sk_warn("%p: Application is not registered\n", sock);
		return -EINVAL;
	}

	if (copy_from_user(&crypto_info, (void *)argp, sizeof(crypto_info))) {
		nss_ovpn_sk_warn("%p: Failed to copy crypto key\n", sock);
		return -EFAULT;
	}

	crypto_cfg.algo = crypto_info.config.algo;
	crypto_cfg.encrypt.cipher_keylen = crypto_info.config.cipher_key_size;
	crypto_cfg.encrypt.hmac_keylen = crypto_info.config.hmac_key_size;
	crypto_cfg.decrypt.cipher_keylen = crypto_info.config.cipher_key_size;
	crypto_cfg.decrypt.hmac_keylen = crypto_info.config.hmac_key_size;

	memcpy(&crypto_cfg.encrypt, &crypto_info.encrypt, sizeof(crypto_info.encrypt));
	memcpy(&crypto_cfg.decrypt, &crypto_info.decrypt, sizeof(crypto_info.decrypt));

	nss_ovpn_sk_info("%p: Crypto Add: tunnel_id = %u, key_id = %d\n", sock, crypto_info.tunnel_id, crypto_info.key_id);
	return nss_ovpnmgr_crypto_key_add(crypto_info.tunnel_id, crypto_info.key_id, &crypto_cfg);
}

/*
 * nss_ovpn_sk_route_del()
 *	Delete route from tunnel.
 */
static int nss_ovpn_sk_route_del(struct socket *sock, unsigned long argp)
{
	struct nss_ovpn_sk_route_info route_info;
	struct nss_ovpnmgr_route_tuple route;
	struct nss_ovpn_sk_pinfo *pinfo = (struct nss_ovpn_sk_pinfo *)sock->sk;

	if (!pinfo->dev) {
		nss_ovpn_sk_warn("%p: Application is not registered\n", sock);
		return -EINVAL;
	}

	if (copy_from_user(&route_info, (void *)argp, sizeof(route_info))) {
		nss_ovpn_sk_warn("%p: Failed to copy route information\n", sock);
		return -EFAULT;
	}

	nss_ovpn_sk_route_info_copy(&route, &route_info);
	return nss_ovpnmgr_route_del(route_info.tunnel_id, &route);
}

/*
 * nss_ovpn_sk_route_add()
 *	Add route for the tunnel.
 */
static int nss_ovpn_sk_route_add(struct socket *sock, unsigned long argp)
{
	struct nss_ovpn_sk_route_info route_info;
	struct nss_ovpnmgr_route_tuple route;
	struct nss_ovpn_sk_pinfo *pinfo = (struct nss_ovpn_sk_pinfo *)sock->sk;

	if (!pinfo->dev) {
		nss_ovpn_sk_warn("%p: Application is not registered\n", sock);
		return -EINVAL;
	}

	if (copy_from_user(&route_info, (void *)argp, sizeof(route_info))) {
		nss_ovpn_sk_warn("%p: Failed to copy route information\n", sock);
		return -EFAULT;
	}

	nss_ovpn_sk_route_info_copy(&route, &route_info);
	return nss_ovpnmgr_route_add(route_info.tunnel_id, &route);
}

/*
 * nss_ovpn_sk_tun_del()
 *	Delete tunnel.
 */
static int nss_ovpn_sk_tun_del(struct socket *sock, unsigned long argp)
{
	uint32_t tunnel_id;
	struct nss_ovpn_sk_pinfo *pinfo = (struct nss_ovpn_sk_pinfo *)sock->sk;

	if (!pinfo->dev) {
		nss_ovpn_sk_warn("%p: Application is not registered\n", sock);
		return -EINVAL;
	}

	if (copy_from_user(&tunnel_id, (void *)argp, sizeof(tunnel_id))) {
		nss_ovpn_sk_warn("%p: Failed to copy tunnel id\n", sock);
		return -EFAULT;
	}

	return nss_ovpnmgr_tun_del(tunnel_id);
}

/*
 * nss_ovpn_sk_tun_add()
 *	Add tunnel.
 */
static int nss_ovpn_sk_tun_add(struct socket *sock, unsigned long argp)
{
	struct nss_ovpnmgr_crypto_config crypto_cfg;
	struct nss_ovpnmgr_tun_config tun_cfg;
	struct nss_ovpnmgr_tun_tuple tun_hdr;
	struct nss_ovpn_sk_tunnel tun_data;
	struct nss_ovpn_sk_pinfo *pinfo = (struct nss_ovpn_sk_pinfo *)sock->sk;
	struct net_device *tun_dev;
	uint32_t tunnel_id;
	int err;

	if (!pinfo->dev) {
		nss_ovpn_sk_warn("%p: Application is not registered\n", sock);
		return -EINVAL;
	}

	if (copy_from_user(&tun_data, (void *)argp, sizeof(tun_data))) {
		nss_ovpn_sk_warn("%p: Failed to copy application data\n", sock);
		return -EFAULT;
	}

	tun_cfg.flags = tun_data.ovpn.flags;
	memcpy(&tun_hdr.src_ip[0], &tun_data.tun_hdr.src_ip[0], sizeof(tun_data.tun_hdr.src_ip));
	memcpy(&tun_hdr.dst_ip[0], &tun_data.tun_hdr.dst_ip[0], sizeof(tun_data.tun_hdr.dst_ip));
	tun_hdr.src_port = tun_data.tun_hdr.src_port;
	tun_hdr.dst_port = tun_data.tun_hdr.dst_port;

	crypto_cfg.algo = tun_data.crypto.config.algo;
	crypto_cfg.encrypt.cipher_keylen = tun_data.crypto.config.cipher_key_size;
	crypto_cfg.encrypt.hmac_keylen = tun_data.crypto.config.hmac_key_size;
	crypto_cfg.decrypt.cipher_keylen = tun_data.crypto.config.cipher_key_size;
	crypto_cfg.decrypt.hmac_keylen = tun_data.crypto.config.hmac_key_size;

	memcpy(&crypto_cfg.encrypt.cipher_key, &tun_data.crypto.encrypt.cipher_key, tun_data.crypto.config.cipher_key_size);
	memcpy(&crypto_cfg.encrypt.hmac_key, &tun_data.crypto.encrypt.hmac_key, tun_data.crypto.config.hmac_key_size);
	memcpy(&crypto_cfg.decrypt.cipher_key, &tun_data.crypto.decrypt.cipher_key, tun_data.crypto.config.cipher_key_size);
	memcpy(&crypto_cfg.decrypt.hmac_key, &tun_data.crypto.decrypt.hmac_key, tun_data.crypto.config.hmac_key_size);

	tunnel_id = nss_ovpnmgr_tun_add(pinfo->dev, &tun_hdr, &tun_cfg, &crypto_cfg);
	if (!tunnel_id) {
		nss_ovpn_sk_warn("%p: Failed to add tunnel for application:%u\n", sock, pinfo->pid);
		return -EINVAL;
	}

	tun_dev = dev_get_by_index(&init_net, tunnel_id);
	if (unlikely(!tun_dev)) {
		nss_ovpn_sk_warn("%p: tun_dev is not found: tunnel_id = %u\n", sock, tunnel_id);
		nss_ovpnmgr_tun_del(tunnel_id);
		return -EFAULT;
	}

	dev_put(tun_dev);
	/*
	 * Bring up tunnel device.
	 */
	rtnl_lock();
	err = dev_open(tun_dev);
	rtnl_unlock();

	if (err) {
		nss_ovpn_sk_warn("%p: tun_dev is not found: tunnel_id = %u\n", sock, tunnel_id);
		nss_ovpnmgr_tun_del(tunnel_id);
		return -EFAULT;
	}

	/*
	 * Copy tunnel_id to application memory.
	 */
	tun_data.ovpn.tunnel_id = tunnel_id;

	if (copy_to_user((void *)argp, &tun_data, sizeof(tun_data))) {
		nss_ovpn_sk_warn("%p: Failed to copy tunnel information to application\n", sock);
		nss_ovpnmgr_tun_del(tunnel_id);
		return -EFAULT;
	}

	return 0;
}

/*
 * nss_ovpn_sk_app_dereg()
 *	Deregister application.
 */
static int nss_ovpn_sk_app_dereg(struct socket *sock, unsigned long argp)
{
	struct nss_ovpn_sk_pinfo *pinfo = (struct nss_ovpn_sk_pinfo *)sock->sk;
	int ret;

	if (!pinfo->dev) {
		nss_ovpn_sk_warn("%p: Application is not registered\n", sock);
		return -EINVAL;
	}

	nss_ovpn_sk_info("%p: Deregistering app with pid=%u\n", sock, pinfo->pid);
	ret = nss_ovpnmgr_app_del(pinfo->dev);
	if (!ret)
		pinfo->pid = 0;

	/*
	 * pinfo->dev was held during application registration.
	 * Since application is closing the socket. release the hold.
	 */
	if (pinfo->dev) {
		dev_put(pinfo->dev);
		pinfo->dev = NULL;
	}

	return  ret;
}

/*
 * nss_ovpn_sk_app_reg()
 *	Register application.
 */
static int nss_ovpn_sk_app_reg(struct socket *sock, unsigned long argp)
{
	struct nss_ovpn_sk_pinfo *pinfo = (struct nss_ovpn_sk_pinfo *)sock->sk;
	struct nss_ovpn_sk_app_inst app;
	int ret;

	if (copy_from_user(&app, (void *)argp, sizeof(app))) {
		nss_ovpn_sk_warn("%p: Failed to copy application data\n", sock);
		return -EFAULT;
	}
	nss_ovpn_sk_info("%p: pid = %u, tun_fd = %d, udp_fd = %d\n", sock, app.pid, app.tun_fd, app.udp_fd);

	pinfo->dev = dev_get_by_name(&init_net, app.tun_dev);
	if (!pinfo->dev) {
		nss_ovpn_sk_warn("%p: Couldn't find tun/tap dev=%s\n", sock, app.tun_dev);
		return -EFAULT;
	}

	ret = nss_ovpnmgr_app_add(pinfo->dev, app.app_mode, (void *)sock);
	if (ret) {
		nss_ovpn_sk_warn("%p: Failed to register application, pid=%u\n", sock, app.pid);
		dev_put(pinfo->dev);
		return ret;
	}

	/*
	 * Initialize socket private data.
	 * this will be used in rest of the socket functionality
	 */
	pinfo->pid = app.pid;
	pinfo->tun_fd = app.tun_fd;
	pinfo->udp_fd = app.udp_fd;

	/*
	 * Do not release pinfo->dev here.  It will be used
	 * by socket layer as long as the application is registered.
	 */
	return 0;
}

/*
 * nss_ovpn_sk_release()
 *	Release socket.
 */
int nss_ovpn_sk_release(struct socket *sock)
{
	struct nss_ovpn_sk_pinfo *pinfo = (struct nss_ovpn_sk_pinfo *)sock->sk;

	nss_ovpn_sk_info("%p: Releasing socket\n", sock);
	if (pinfo->pid && nss_ovpnmgr_app_del(pinfo->dev)) {
		nss_ovpn_sk_info("%p: Failed to delete App\n", sock);
	}

	/*
	 * pinfo->dev was held during application registration.
	 * Since application is closing the socket. release the hold.
	 */
	if (pinfo->dev) {
		dev_put(pinfo->dev);
		pinfo->dev = NULL;
	}

	if (sock->sk)
		sock_put(sock->sk);

	return 0;
}

/*
 * nss_ovpn_sk_sendmsg()
 *	Receive packet from application.
 */
static int nss_ovpn_sk_sendmsg(struct socket *sock, struct msghdr *msg, size_t len)
{
	struct nss_ovpn_sk_pkt_info *pkt_info_data;
	struct nss_ovpnmgr_metadata mdata;
	struct nss_ovpn_sk_pinfo *pinfo = (struct nss_ovpn_sk_pinfo *)sock->sk;
	struct sk_buff *skb = NULL;
	struct cmsghdr *cmsg;
	size_t total_len = iov_iter_count(&msg->msg_iter);
	uint8_t *data;
	int outer_hdr_offset = 0;

	if (!pinfo->dev) {
		nss_ovpn_sk_warn("%p: Application is not registered\n", sock);
		return -EINVAL;
	}

	/*
	 * Packet received from application has two parts:
	 *	1. Control message provides details of the packet.
	 *	2. Packet content.
	 * Extract control message into pkt_info.
	 */
	cmsg = CMSG_FIRSTHDR(msg);
	if (!cmsg) {
		nss_ovpn_sk_warn("%p: Control message is invalid\n", sock);
		return -EINVAL;
	}

	if (!CMSG_OK(msg, cmsg)) {
		nss_ovpn_sk_warn("%p: Incorrect message format\n", sock);
		return -EINVAL;
	}

	if (cmsg->cmsg_len < CMSG_LEN(sizeof(*pkt_info_data))) {
		nss_ovpn_sk_warn("%p: Incorrect message length\n", sock);
		return -EINVAL;
	}

	pkt_info_data = (struct nss_ovpn_sk_pkt_info *)CMSG_DATA(cmsg);
	if (!pkt_info_data) {
		nss_ovpn_sk_warn("%p: Cannot send this packet, there is no msg_control\n", sock);
		return -EFAULT;
	}

	if (pkt_info_data->flags & NSS_OVPN_SK_PKT_INFO_FLAG_DIR_DECAP) {
		if (pkt_info_data->flags & NSS_OVPN_SK_PKT_INFO_FLAG_PKT_TYPE_IPV6) {
			outer_hdr_offset = sizeof(struct ipv6hdr) + sizeof(struct udphdr);
			nss_ovpn_sk_info("%p: Decrypt: IPv6 + UDP\n", sock);
		} else {
			outer_hdr_offset = sizeof(struct iphdr) + sizeof(struct udphdr);
			nss_ovpn_sk_info("%p: Decrypt: IPv4 + UDP\n", sock);
		}
	} else {
		/*
		 * Encapsulation requires headroom + tailroom.
		 */
		outer_hdr_offset = NSS_OVPNMGR_TUN_HEADROOM + NSS_OVPNMGR_TUN_TAILROOM;
	}

	skb = dev_alloc_skb(total_len + outer_hdr_offset);
	if (unlikely(!skb)) {
		nss_ovpn_sk_warn("%p: Couldn't allocation skb\n", sock);
		return -ENOBUFS;
	}

	if (pkt_info_data->flags & NSS_OVPN_SK_PKT_INFO_FLAG_DIR_DECAP) {
		data = skb_put(skb, total_len + outer_hdr_offset);
		data += outer_hdr_offset;
	} else {
		skb_reserve(skb, NSS_OVPNMGR_TUN_HEADROOM);
		data = skb_put(skb, total_len);
	}

	if (copy_from_iter(data, total_len, &msg->msg_iter) != total_len) {
		nss_ovpn_sk_info("%p: skb copy conversion failed\n", sock);
		dev_kfree_skb_any(skb);
		return -EFAULT;
	}

	mdata.tunnel_id = pkt_info_data->tunnel_id;
	mdata.flags = pkt_info_data->flags;

	if (nss_ovpnmgr_tun_tx(mdata.tunnel_id, &mdata, skb)) {
		nss_ovpn_sk_info("%p: Packet offload failed.\n", sock);
		dev_kfree_skb_any(skb);
		return -EFAULT;
	}

	return 0;
}

/*
 * nss_ovpn_sk_recvmsg()
 *	Send packet to application.
 */
static int nss_ovpn_sk_recvmsg(struct socket *sock, struct msghdr *msg, size_t size, int flags)
{
	struct nss_ovpn_sk_pkt_info *pkt_info_data;
	struct nss_ovpnmgr_metadata pkt_info;
	int copied, ret;
	struct sk_buff *skb;
	struct cmsghdr *cmsg;
	struct sock *sk = sock->sk;

	if (flags & ~(MSG_PEEK | MSG_DONTWAIT | MSG_TRUNC | MSG_CMSG_COMPAT)) {
		nss_ovpn_sk_info("%p: Cannot send this packet to app, there is no msg_control\n", sock);
		return -EINVAL;
	}

	/*
	 * Packet received from application has two parts:
	 *	1. Control message provides details of the packet.
	 *	2. Packet content.
	 * Extract control message into pkt_info.
	 */
	cmsg = CMSG_FIRSTHDR(msg);
	if (!cmsg) {
		nss_ovpn_sk_warn("%p: Control message is invalid\n", sock);
		return -EINVAL;
	}

	if (!CMSG_OK(msg, cmsg)) {
		nss_ovpn_sk_warn("%p: Incorrect message format\n", sock);
		return -EINVAL;
	}

	if (cmsg->cmsg_len < CMSG_LEN(sizeof(*pkt_info_data))) {
		nss_ovpn_sk_warn("%p: Incorrect message length\n", sock);
		return -EINVAL;
	}

	pkt_info_data = (struct nss_ovpn_sk_pkt_info *)CMSG_DATA(cmsg);
	if (!pkt_info_data) {
		nss_ovpn_sk_warn("%p: Cannot send this packet to app, there is no msg_control.\n", sock);
		return -EINVAL;
	}

	skb = skb_recv_datagram(sk, flags, MSG_DONTWAIT, &ret);
	if (!skb) {
		nss_ovpn_sk_warn("%p: There are no packets in the queue.\n", sock);
		return -ENOBUFS;
	}

	/*
	 * Control informatoin of packet is copied in skb->cb by OVPN manager.
	 * Send control information to application.
	 */
	memcpy(&pkt_info, skb->cb, sizeof(pkt_info));
	pkt_info_data->tunnel_id = pkt_info.tunnel_id;
	pkt_info_data->flags = pkt_info.flags;
	put_cmsg(msg, SOL_IP, IP_PKTINFO, sizeof(*pkt_info_data), pkt_info_data);

	copied = skb->len;
	if (copied > size) {
		msg->msg_flags |= MSG_TRUNC;
		copied = size;
	}

	skb_reset_transport_header(skb);
	ret = skb_copy_datagram_msg(skb, 0, msg, copied);
	if (ret) {
		nss_ovpn_sk_warn("%p: Packet copy to user failed\n", sock);
		skb_free_datagram(sk, skb);
		return ret;
	}

	ret = (flags & MSG_TRUNC) ? skb->len : copied;
	nss_ovpn_sk_info("%p: Message copied with packet length=%d\n", sock, ret);
	return ret;
}

/*
 * nss_ovpn_sk_ioctl()
 *	Ioctl interface.
 */
static int nss_ovpn_sk_ioctl(struct socket *sock, unsigned int cmd, unsigned long argp)
{
	switch (cmd) {
	case NSS_OVPN_SK_SIOC_APP_REG:
		return nss_ovpn_sk_app_reg(sock, argp);
	case NSS_OVPN_SK_SIOC_APP_DEREG:
		return nss_ovpn_sk_app_dereg(sock, argp);
	case NSS_OVPN_SK_SIOC_TUN_ADD:
		return nss_ovpn_sk_tun_add(sock, argp);
	case NSS_OVPN_SK_SIOC_TUN_DEL:
		return nss_ovpn_sk_tun_del(sock, argp);
	case NSS_OVPN_SK_SIOC_ROUTE_ADD:
		return nss_ovpn_sk_route_add(sock, argp);
	case NSS_OVPN_SK_SIOC_ROUTE_DEL:
		return nss_ovpn_sk_route_del(sock, argp);
	case NSS_OVPN_SK_SIOC_ROUTE_STATE_GET:
		return nss_ovpn_sk_route_state_get(sock, argp);
	case NSS_OVPN_SK_SIOC_CRYPTO_KEY_ADD:
		return nss_ovpn_sk_crypto_key_add(sock, argp);
	case NSS_OVPN_SK_SIOC_CRYPTO_KEY_DEL:
		return nss_ovpn_sk_crypto_key_del(sock, argp);
	case NSS_OVPN_SK_SIOC_STATS_GET:
		return nss_ovpn_sk_stats_get(sock, argp);
	}

	nss_ovpn_sk_warn("%p: Invalid ioctl command: %u\n", sock, cmd);
	return -ENOIOCTLCMD;
}

static const struct proto_ops nss_ovpn_sk_proto_ops = {
	.family = PF_OVPN,
	.owner = THIS_MODULE,

	.connect = sock_no_connect,
	.socketpair = sock_no_socketpair,
	.getname = sock_no_getname,
	.ioctl = nss_ovpn_sk_ioctl,
	.listen = sock_no_listen,
	.shutdown = sock_no_shutdown,
	.getsockopt = sock_no_getsockopt,
	.mmap = sock_no_mmap,
	.sendpage = sock_no_sendpage,
	.sendmsg = nss_ovpn_sk_sendmsg,
	.recvmsg = nss_ovpn_sk_recvmsg,
	.poll = datagram_poll,
	.bind = sock_no_bind,
	.release = nss_ovpn_sk_release,
	.setsockopt = sock_no_setsockopt,
	.accept = sock_no_accept,
};

/*
 * nss_ovpn_sk_create()
 *	Create OVPN socket.
 */
static int nss_ovpn_sk_create(struct net *net, struct socket *sock, int protocol, int kern)
{
	struct nss_ovpn_sk_pinfo *pinfo;
	struct sock *sk;

	nss_ovpn_sk_info("%p: protocol = %d, sock->type = %d\n", sock, protocol, sock->type);

	if (sock->type != SOCK_DGRAM) {
		nss_ovpn_sk_warn("%p: Only SOCK_DGRAM socket is supported\n", sock);
		return -ESOCKTNOSUPPORT;
	}

	/*
	 * For now we don't have any protocol defined.
	 * We could define different protocols and generalize
	 * this implementation.
	 */
	if (protocol) {
		nss_ovpn_sk_warn("%p: Protocol specific socket is not implemented.\n", sock);
		return -EPROTONOSUPPORT;
	}

	sk = sk_alloc(net, PF_OVPN, GFP_KERNEL, &nss_ovpn_sk_proto, kern);
	if (!sk) {
		nss_ovpn_sk_warn("%p: sock instance allocation failed.\n", sock);
		return -ENOMEM;
	}

	pinfo =  (struct nss_ovpn_sk_pinfo *)sk;
	pinfo->pid = 0;
	pinfo->tun_fd = 0;
	pinfo->tun_sock = NULL;
	pinfo->udp_fd = 0;

	sock->ops = &nss_ovpn_sk_proto_ops;
	sock_init_data(sock, sk);

	sk->sk_family = PF_OVPN;
	sk->sk_destruct = NULL;

	nss_ovpn_sk_info("Socket created successfully\n");
	return 0;
}

static const struct net_proto_family nss_ovpn_sk_family = {
	.family = PF_OVPN,
	.create = nss_ovpn_sk_create,
	.owner = THIS_MODULE,
};

/*
 * nss_ovpn_sk_send()
 *	Enqueue packet to socket queue.
 */
int nss_ovpn_sk_send(struct sk_buff *skb, void *app_data)
{
	struct socket *sock = app_data;
	struct sock *sk;

	if (!app_data) {
		nss_ovpn_sk_warn("%p: app_data is NULL\n", skb);
		return -1;
	}

	if (unlikely(skb_orphan_frags(skb, GFP_ATOMIC))) {
		return -1;
	}

	sk = sock->sk;

	/*
	 * Orphan the skb - required as we might hang on to it
	 * for indefinite time.
	 */
	skb_orphan(skb);
	nf_reset(skb);

	/* Enqueue packet */
	if (sock_queue_rcv_skb(sk, skb) < 0) {
		return -1;
	}

	nss_ovpn_sk_info("%p: Packet enqueued into socket queue\n", skb);

	return 0;
}

/*
 * nss_ovpn_sk_init()
 *	Register OVPN socket family.
 */
int nss_ovpn_sk_init(void)
{
	int ret = proto_register(&nss_ovpn_sk_proto, 0);

	if (ret) {
		nss_ovpn_sk_warn("Failed to register OVPN socket protocol\n");
		return ret;
	}

	ret = sock_register(&nss_ovpn_sk_family);
	if (ret) {
		nss_ovpn_sk_warn("Failed to register OVPN socket family\n");
		proto_unregister(&nss_ovpn_sk_proto);
		return ret;
	}

	return 0;
}

/*
 * nss_ovpn_sk_cleanup()
 *	De-register OVPN socket family.
 */
void nss_ovpn_sk_cleanup(void)
{
	sock_unregister(PF_OVPN);
	proto_unregister(&nss_ovpn_sk_proto);
}

MODULE_ALIAS_NETPROTO(AF_OVPN);
