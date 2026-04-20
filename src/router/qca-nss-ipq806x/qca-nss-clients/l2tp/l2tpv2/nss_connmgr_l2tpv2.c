/*
 **************************************************************************
 * Copyright (c) 2015-2017, 2019-2021 The Linux Foundation. All rights reserved.
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
 * nss_connnmgr_l2tpv2.c
 *
 * This file is the NSS l2tpv2 tunnel module
 */

#include <linux/types.h>
#include <linux/ip.h>
#include <linux/rcupdate.h>
#include <linux/module.h>
#include <linux/of.h>
#include <net/ipv6.h>
#include <linux/rwlock_types.h>
#include <linux/if_pppox.h>
#include <linux/hashtable.h>
#include <linux/inetdevice.h>
#include <linux/l2tp.h>
#include <linux/if_arp.h>
#include <linux/version.h>
#include <linux/sysctl.h>
#include <net/route.h>
#include <linux/refcount.h>
#include <linux/../../net/l2tp/l2tp_core.h>

#include <nss_api_if.h>
#include <nss_dynamic_interface.h>
#include "nss_connmgr_l2tpv2.h"
#include "nss_l2tpv2_stats.h"
#include "nss_l2tpmgr.h"

#define L2TP_HDRFLAG_T     0x8000
#define L2TP_HDRFLAG_L     0x4000
#define L2TP_HDRFLAG_S     0x0800
#define L2TP_HDRFLAG_O     0x0200
#define L2TP_HDRFLAG_P     0x0100
#define L2TP_HDR_MIN_LEN   6
#define PPP_HDR_LEN        4
#define BYTES_PER_SHORT    2

/*
 * NSS l2tpv2 debug macros
 */
#if (NSS_L2TP_DEBUG_LEVEL < 1)
#define nss_connmgr_l2tpv2_assert(fmt, args...)
#else
#define nss_connmgr_l2tpv2_assert(c)  BUG_ON(!(c));
#endif

#if defined(CONFIG_DYNAMIC_DEBUG)

/*
 * Compile messages for dynamic enable/disable
 */
#define nss_connmgr_l2tpv2_warning(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define nss_connmgr_l2tpv2_info(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define nss_connmgr_l2tpv2_trace(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#else

/*
 * Statically compile messages at different levels
 */
#if (NSS_L2TP_DEBUG_LEVEL < 2)
#define nss_connmgr_l2tpv2_warning(s, ...)
#else
#define nss_connmgr_l2tpv2_warning(s, ...) pr_warn("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_L2TP_DEBUG_LEVEL < 3)
#define nss_connmgr_l2tpv2_info(s, ...)
#else
#define nss_connmgr_l2tpv2_info(s, ...)   pr_notice("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_L2TP_DEBUG_LEVEL < 4)
#define nss_connmgr_l2tpv2_trace(s, ...)
#else
#define nss_connmgr_l2tpv2_trace(s, ...)  pr_info("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif
#endif

#define NUM_L2TP_CHANNELS_IN_PPP_NETDEVICE  1
#define HASH_BUCKET_SIZE 2  /* ( 2^ HASH_BUCKET_SIZE ) == 4 */
#define L2TP_SYSCTL_STR_LEN_MAX 2*IFNAMSIZ + 1 /* Size is determined to accomodate two netdevice names and a space in between */

static DEFINE_HASHTABLE(l2tpv2_session_data_hash_table, HASH_BUCKET_SIZE);
static int ip_ttl_max = 255;

#if defined(NSS_L2TP_IPSEC_BIND_BY_NETDEV)
static char l2tpoipsec_config[L2TP_SYSCTL_STR_LEN_MAX];
static struct ctl_table_header *ctl_tbl_hdr; /* l2tpv2 sysctl */
#endif
static struct l2tpmgr_ipsecmgr_cb __rcu ipsecmgr_cb;

/*
 * nss_connmgr_l2tpv2_msg_cb()
 *	L2TP message callback.
 */
static void nss_connmgr_l2tpv2_msg_cb(void *app_data, struct nss_l2tpv2_msg *msg) {
	struct nss_cmn_msg *ncm = &msg->cm;
	if (ncm->response != NSS_CMN_RESPONSE_ACK) {
		nss_connmgr_l2tpv2_warning("Recevied NACK for L2TP message type = %d from NSS\n", ncm->type);
	}

	nss_connmgr_l2tpv2_trace("Recevied ACK for L2TP message type = %d from NSS\n", ncm->type);
}

/*
 * nss_connmgr_l2tpv2_get_session()
 *	retrieve ppp over l2tpv2 session associated with this netdevice if any
 *
 */
static struct l2tp_session *nss_connmgr_l2tpv2_get_session(struct net_device *dev)
{
	struct ppp_channel *ppp_chan[1] = {NULL};
	int px_proto;
	struct sock *sk;
	struct l2tp_session *session = NULL;

	/*
	 * check whether the interface is of type PPP
	 */
	if (dev->type != ARPHRD_PPP || !(dev->flags & IFF_POINTOPOINT)) {
		nss_connmgr_l2tpv2_info("netdevice is not a PPP tunnel type\n");
		return NULL;
	}

	if (ppp_is_multilink(dev)) {
		nss_connmgr_l2tpv2_info("channel is multilink PPP\n");
		return NULL;
	}

	if (ppp_hold_channels(dev, ppp_chan, 1) != 1) {
		nss_connmgr_l2tpv2_info("hold channel for netdevice failed\n");
		return NULL;
	}

	if (!ppp_chan[0]) {
		nss_connmgr_l2tpv2_info("channel don't have a ppp_channel\n");
		goto err_nss_connmgr_l2tpv2_get_session_1;
	}

	px_proto = ppp_channel_get_protocol(ppp_chan[0]);
	if (px_proto != PX_PROTO_OL2TP) {
		nss_connmgr_l2tpv2_info("session socket is not of type PX_PROTO_OL2TP\n");
		goto err_nss_connmgr_l2tpv2_get_session_1;
	}

	sk = (struct sock *)ppp_chan[0]->private;
	if (!sk) {
		nss_connmgr_l2tpv2_info("session socket is NULL\n");
		goto err_nss_connmgr_l2tpv2_get_session_1;
	}
	sock_hold(sk);

	/*
	 * This is very unlikely, But check the socket is connected state
	 */
	if (unlikely(sock_flag(sk, SOCK_DEAD) || !(sk->sk_state & PPPOX_CONNECTED))) {
		nss_connmgr_l2tpv2_info("l2tpv2 session sock is either dead or not in connected state\n");
		goto err_nss_connmgr_l2tpv2_get_session_2;
	}

	session = (struct l2tp_session *)(sk->sk_user_data);
	if (!session) {
		nss_connmgr_l2tpv2_info("l2tpv2 session is null\n");
		goto err_nss_connmgr_l2tpv2_get_session_2;
	}

	session_hold(session);

err_nss_connmgr_l2tpv2_get_session_2:
	sock_put(sk);

err_nss_connmgr_l2tpv2_get_session_1:
	ppp_release_channels(ppp_chan, 1);
	return session;
}

/*
 * nss_connmgr_l2tpv2_get_all_proto_data()
 *	get all information on ppp, l2tpv2, udp and ip protocol
 */
static struct nss_connmgr_l2tpv2_session_data
*nss_connmgr_l2tpv2_get_all_proto_data(struct net_device *dev,
				       struct l2tp_session *session)
{
	struct nss_connmgr_l2tpv2_session_data *l2tpv2_session_data = NULL;
	struct l2tp_tunnel *tunnel = NULL;
	struct inet_sock *inet;
	struct nss_connmgr_l2tpv2_data *data;
	struct udp_sock *udp;

	l2tpv2_session_data = kmalloc(sizeof(struct nss_connmgr_l2tpv2_session_data),
				      GFP_KERNEL);
	if (!l2tpv2_session_data) {
		nss_connmgr_l2tpv2_info("failed to allocate l2tpv2_session_data\n");
		session_put(session);
		return NULL;
	}

	tunnel = session->tunnel;
	if (unlikely(!tunnel)) {
		nss_connmgr_l2tpv2_info("tunnel is null for session %px\n", session);
		goto err_nss_connmgr_l2tpv2_get_data_1;
	}
	tunnel_hold(tunnel);

	data = &l2tpv2_session_data->data;

	if (tunnel->version != L2TP_V_2)  {
		nss_connmgr_l2tpv2_info("l2tp version is not 2\n");
		goto err_nss_connmgr_l2tpv2_get_data_2;
	}

	if (!tunnel->sock) {
		nss_connmgr_l2tpv2_info("tunnel sock is NULL\n");
		goto err_nss_connmgr_l2tpv2_get_data_2;
	}
	sock_hold(tunnel->sock);

	/*
	 * Get Tunnel Info
	 */
	data->l2tpv2.tunnel.tunnel_id = tunnel->tunnel_id;
	data->l2tpv2.tunnel.peer_tunnel_id = tunnel->peer_tunnel_id;

	nss_connmgr_l2tpv2_info("vers=%u tun id=%u peer_id=%u ncap=%u\n",
	       tunnel->version, tunnel->tunnel_id,
	       tunnel->peer_tunnel_id,  tunnel->encap);
	/*
	 * Get session info
	 */
	data->l2tpv2.session.session_id = session->session_id;
	data->l2tpv2.session.peer_session_id = session->peer_session_id;
	data->l2tpv2.session.hdr_len = session->hdr_len;
	data->l2tpv2.session.reorder_timeout = session->reorder_timeout;
	data->l2tpv2.session.recv_seq = session->recv_seq;
	data->l2tpv2.session.send_seq = session->send_seq;

	nss_connmgr_l2tpv2_info("sess %u, peer=%u nr=%u ns=%u hdr_len=%u timeout=%x"
	       " recv_seq=%x send_seq=%x\n",
	       session->session_id,  session->peer_session_id, session->nr,
	       session->ns, session->hdr_len,
	       session->reorder_timeout, session->recv_seq,
	       session->send_seq);

	/*
	 * tunnel->sock->sk_no_check/sk_no_check_tx is set to true
	 * if UDP checksum is not needed for the L2TP socket.
	 */
	data->l2tpv2.tunnel.udp_csum = !tunnel->sock->sk_no_check_tx;

	inet = inet_sk(tunnel->sock);

	/*
	 * UDP header exist ?
	 */
	if (tunnel->encap != L2TP_ENCAPTYPE_UDP) {
		nss_connmgr_l2tpv2_info("encap type is not UDP\n");
		goto err_nss_connmgr_l2tpv2_get_data_3;

	}

	if (tunnel->sock->sk_protocol != IPPROTO_UDP) {
		nss_connmgr_l2tpv2_info("Wrong protocol %u\n",
				       tunnel->sock->sk_protocol);
		goto err_nss_connmgr_l2tpv2_get_data_3;
	}

	if (tunnel->sock->sk_state != TCP_ESTABLISHED) {
		nss_connmgr_l2tpv2_info("Udp socket is not in established" \
				       " state %u\n", tunnel->sock->sk_state);
		goto err_nss_connmgr_l2tpv2_get_data_3;
	}

	udp = udp_sk(tunnel->sock);
	if (!udp) {
		nss_connmgr_l2tpv2_info("There is no udp socket !!! ?\n");
		goto err_nss_connmgr_l2tpv2_get_data_3;
	}

	data->udp.dport = inet->inet_dport;
	data->udp.sport = inet->inet_sport;

	nss_connmgr_l2tpv2_info("udp sport=0x%x dport=0x%x\n", ntohs(inet->inet_sport), ntohs(inet->inet_dport));

	/*
	 * ip data
	 */
	if (inet->sk.sk_family == AF_INET) {
		data->ip.v4.saddr.s_addr = inet->inet_saddr;
		data->ip.v4.daddr.s_addr = inet->inet_daddr;
	} else {
		nss_connmgr_l2tpv2_info("Only Ipv4 protocol supports l2tpv2 packet's outer IP\n");
		goto err_nss_connmgr_l2tpv2_get_data_3;
	}

	nss_connmgr_l2tpv2_info("saddr 0x%x daddr = 0x%x\n",  ntohl(inet->inet_saddr), ntohl(inet->inet_daddr));

	/*
	 * fill in extra data. This can be used to get reference to
	 * netdevice
	 */
	dev_hold(dev);
	l2tpv2_session_data->dev = dev;
	strlcpy(session->ifname, dev->name, IFNAMSIZ);

	/*
	 * There is no need for protecting simultaneous addition &
	 * deletion of l2tpv2_session_data as all netdev notifier chain
	 * call back is called  with rtln mutex lock.
	 */
	hash_add_rcu(l2tpv2_session_data_hash_table,
		     &l2tpv2_session_data->hash_list,
		     dev->ifindex);

	sock_put(tunnel->sock);
	tunnel_put(tunnel);
	session_put(session);
	return l2tpv2_session_data;

err_nss_connmgr_l2tpv2_get_data_3:
	sock_put(tunnel->sock);

err_nss_connmgr_l2tpv2_get_data_2:
	tunnel_put(tunnel);

err_nss_connmgr_l2tpv2_get_data_1:
	kfree(l2tpv2_session_data);
	session_put(session);
	return NULL;
}

/*
 * nss_connmgr_l2tpv2_exception()
 *	Exception handler registered to NSS for handling l2tpv2 pkts
 *
 */
static void nss_connmgr_l2tpv2_exception(struct net_device *dev,
				       struct sk_buff *skb,
				       __attribute__((unused)) struct napi_struct *napi)

{
	const struct iphdr *iph_outer, *iph_inner;
	struct nss_connmgr_l2tpv2_session_data *ptr;
	uint16_t *l2tp_hdr;
	uint16_t l2tp_flags;
	int l2tp_hdr_len = L2TP_HDR_MIN_LEN;
	__be32 tunnel_local_ip;
	struct rtable *rt;
	struct net_device *in_dev;

	/* discard L2 header */
	skb_pull(skb, sizeof(struct ethhdr));
	skb_reset_mac_header(skb);

	iph_outer = (const struct iphdr *)skb->data;

	rcu_read_lock();
	hash_for_each_possible_rcu(l2tpv2_session_data_hash_table, ptr,
				   hash_list, dev->ifindex) {
		if (ptr->dev == dev) {
			tunnel_local_ip = ptr->data.ip.v4.saddr.s_addr;
			rcu_read_unlock();

			if (iph_outer->version == 4 && iph_outer->saddr == tunnel_local_ip) { /*pkt is encapsulated */
				skb_pull(skb, sizeof(struct iphdr));
				skb_pull(skb, sizeof(struct udphdr));
				l2tp_hdr = (uint16_t *)skb->data;
				l2tp_flags = ntohs(*l2tp_hdr);
				if (unlikely(l2tp_flags & L2TP_HDRFLAG_L)) {
					l2tp_hdr_len += 2;
				}
				if (unlikely(l2tp_flags & L2TP_HDRFLAG_S)) {
					l2tp_hdr_len += 4;
				}
				if (unlikely(l2tp_flags & L2TP_HDRFLAG_O)) {
					uint16_t  offset_pad = ntohs(*(l2tp_hdr + l2tp_hdr_len/BYTES_PER_SHORT));
					l2tp_hdr_len += 2 + offset_pad;
				}

				skb_pull(skb, l2tp_hdr_len); /* pull l2tp header */
				skb_pull(skb, PPP_HDR_LEN);  /* pull ppp header */

				iph_inner = (const struct iphdr *)skb->data;
				if (iph_inner->version == 4) {
					skb->protocol = htons(ETH_P_IP);
				} else if (iph_inner->version == 6) {
					skb->protocol = htons(ETH_P_IPV6);
				} else {
					nss_connmgr_l2tpv2_warning("not able to handle this pkt, so freeing\n");
					dev_kfree_skb_any(skb);
					return;
				}
				skb_reset_network_header(skb);
				skb_reset_transport_header(skb);
				skb->ip_summed = CHECKSUM_NONE;
				skb->pkt_type = PACKET_HOST;
				skb->dev = dev;

				/*
				 * set skb_iif
				 */
				rt = ip_route_output(&init_net, iph_inner->saddr, 0, 0, 0);
				if (unlikely(IS_ERR(rt))) {
					nss_connmgr_l2tpv2_warning("Martian packets !!!");
				} else {
					in_dev = rt->dst.dev;
					ip_rt_put(rt);
					if (likely(in_dev)) {
						skb->skb_iif = in_dev->ifindex;
					} else {
						nss_connmgr_l2tpv2_warning("could not find incoming interface\n");
					}
				}

				dev_queue_xmit(skb);
				return;

			} else  { /* pkt is decapsulated */
				if (iph_outer->version == 4) {
					skb->protocol = htons(ETH_P_IP);
				} else if (iph_outer->version == 6) {
					skb->protocol = htons(ETH_P_IPV6);
				} else {
					nss_connmgr_l2tpv2_info("pkt may be a control packet\n");
				}
				skb_reset_network_header(skb);
				skb->pkt_type = PACKET_HOST;
				skb->skb_iif = dev->ifindex;
				skb->ip_summed = CHECKSUM_NONE;
				skb->dev = dev;
				netif_receive_skb(skb);
				return;
			}
		}
	}
	rcu_read_unlock();

	nss_connmgr_l2tpv2_warning("not able to handle this pkt, so freeing\n");
	dev_kfree_skb_any(skb);
}

/*
 * nss_connmgr_l2tpv2_event_receive()
 *	Event Callback to receive events from NSS
 */
static void nss_connmgr_l2tpv2_event_receive(void *if_ctx, struct nss_l2tpv2_msg *tnlmsg)
{
	struct net_device *netdev = if_ctx;

	switch (tnlmsg->cm.type) {
	case NSS_L2TPV2_MSG_SYNC_STATS:
		nss_l2tpv2_update_dev_stats(netdev, (struct nss_l2tpv2_sync_session_stats_msg *)&tnlmsg->msg.stats);
		break;

	default:
		nss_connmgr_l2tpv2_info("Unknown Event from NSS\n");
		break;
	}
}

/*
 * nss_connmgr_l2tpv2_bind_ipsec_by_ipaddr()
 * 	Bind L2TP tunnel with IPsec(xfrm) based on IP Address
 */
static void nss_connmgr_l2tpv2_bind_ipsec_by_ipaddr(struct nss_ctx_instance *nss_ctx, struct nss_connmgr_l2tpv2_data *l2tpv2_data, uint32_t l2tp_ifnum)
{
	struct nss_l2tpv2_msg l2tpv2msg;
	nss_tx_status_t status;
	struct nss_l2tpv2_bind_ipsec_if_msg *l2tpv2_bind_ipsec_msg;
	int32_t ipsec_ifnum = -1;
	get_ipsec_ifnum_by_ip_addr_callback_t ipsec_cb;

	/*
	 * Check if the L2TP interface is applied over an IPsec (XFRM) interface by querying the IPsec
	 * client by using the L2TP tunnel IPv4 source/destination addresses.
	 */
	rcu_read_lock();
	ipsec_cb = rcu_dereference(ipsecmgr_cb.get_ifnum_by_ip_addr);
	ipsec_ifnum = ipsec_cb ? ipsec_cb(IPVERSION, &l2tpv2_data->ip.v4.saddr.s_addr, &l2tpv2_data->ip.v4.daddr.s_addr) : -1;
	rcu_read_unlock();

	if (ipsec_ifnum < 0) {
		nss_connmgr_l2tpv2_info("%px: Invalid IPsec interface no.(0x%x) based on local & remote IP-address\n", nss_ctx, ipsec_ifnum);
		return;
	}

	/*
	 * For, l2tpoipsec, send the command to bind the l2tp session with the IPsec interface.
	 */
	memset(&l2tpv2msg, 0, sizeof(struct nss_l2tpv2_msg));
	nss_l2tpv2_msg_init(&l2tpv2msg, l2tp_ifnum, NSS_L2TPV2_MSG_BIND_IPSEC_IF,
			sizeof(struct nss_l2tpv2_bind_ipsec_if_msg), (void *)nss_connmgr_l2tpv2_msg_cb, NULL);
	l2tpv2_bind_ipsec_msg = &l2tpv2msg.msg.bind_ipsec_if_msg;
	l2tpv2_bind_ipsec_msg->ipsec_ifnum = ipsec_ifnum;

	status = nss_l2tpv2_tx(nss_ctx, &l2tpv2msg);
	if (status != NSS_TX_SUCCESS) {
		/*
		 * TODO: Add retry logic. Currently it sends a warning message to user.
		 * In case of bind fails, then we don't bring down L2TP tunnel, instead we give a warning log
		 * to user, as this introduces a potential risk of not having a per packet check for source
		 * interface number in firmware.
		 */
		nss_connmgr_l2tpv2_warning("%px: L2TPv2 interface binding with IPSec interface(0x%x) failed.\n", nss_ctx, ipsec_ifnum);
		return;
	}

	nss_connmgr_l2tpv2_info("%px: L2TPv2 interface is bound to IPsec interface with if_num(0x%x)\n", nss_ctx, ipsec_ifnum);
}

/*
 * nss_connmgr_l2tpv2_dev_up()
 *	pppol2tpv2 interface's up event handler
 */
static int nss_connmgr_l2tpv2_dev_up(struct net_device *dev)
{
	uint32_t if_number;
	struct l2tp_session *session = NULL;
	struct nss_connmgr_l2tpv2_session_data *l2tpv2_session_data = NULL;
	struct nss_connmgr_l2tpv2_data *data;
	nss_tx_status_t status;
	struct nss_ctx_instance *nss_ctx;
	uint32_t features = 0; /* features denote the skb types supported by this interface */
	struct nss_l2tpv2_msg  l2tpv2msg;
	struct nss_l2tpv2_session_create_msg *l2tpv2cfg;

	session = nss_connmgr_l2tpv2_get_session(dev);
	if (!session) {
		return NOTIFY_DONE;
	}

	l2tpv2_session_data = nss_connmgr_l2tpv2_get_all_proto_data(dev, session);
	if (l2tpv2_session_data  == NULL) {
		return NOTIFY_DONE;
	}

	/*
	 * Create nss dynamic interface and register
	 */
	if_number = nss_dynamic_interface_alloc_node(NSS_DYNAMIC_INTERFACE_TYPE_L2TPV2);
	if (if_number == -1) {
		nss_connmgr_l2tpv2_info("Request interface number failed\n");
		return NOTIFY_DONE;
	}
	nss_connmgr_l2tpv2_info("nss_dynamic_interface_alloc_node() sucessful. if_number = %d\n", if_number);

	if (!nss_is_dynamic_interface(if_number)) {
		nss_connmgr_l2tpv2_warning("Invalid NSS dynamic I/F number %d\n", if_number);
		return NOTIFY_BAD;
	}

	/*
	 * Register l2tpv2  tunnel with NSS
	 */
	nss_ctx = nss_register_l2tpv2_if(if_number,
				       nss_connmgr_l2tpv2_exception,
				       nss_connmgr_l2tpv2_event_receive,
				       dev,
				       features);

	if (!nss_ctx) {
		status = nss_dynamic_interface_dealloc_node(if_number, NSS_DYNAMIC_INTERFACE_TYPE_L2TPV2);
		if (status != NSS_TX_SUCCESS) {
			nss_connmgr_l2tpv2_info("Unable to dealloc the node[%d] in the NSS fw!\n", if_number);
		}
		nss_connmgr_l2tpv2_info("nss_register_l2tpv2_if failed\n");
		return NOTIFY_BAD;
	}
	nss_connmgr_l2tpv2_info("%px: nss_register_l2tpv2_if() successful\n", nss_ctx);

	data = &l2tpv2_session_data->data;

	memset(&l2tpv2msg, 0, sizeof(struct nss_l2tpv2_msg));
	l2tpv2cfg = &l2tpv2msg.msg.session_create_msg;
	l2tpv2cfg->local_tunnel_id = data->l2tpv2.tunnel.tunnel_id;
	l2tpv2cfg->peer_tunnel_id = data->l2tpv2.tunnel.peer_tunnel_id;
	l2tpv2cfg->local_session_id = data->l2tpv2.session.session_id;
	l2tpv2cfg->peer_session_id = data->l2tpv2.session.peer_session_id;
	l2tpv2cfg->sip = ntohl(data->ip.v4.saddr.s_addr);
	l2tpv2cfg->dip = ntohl(data->ip.v4.daddr.s_addr);
	l2tpv2cfg->reorder_timeout = jiffies_to_msecs(data->l2tpv2.session.reorder_timeout);
	l2tpv2cfg->sport = ntohs(data->udp.sport);
	l2tpv2cfg->dport = ntohs(data->udp.dport);

	/*
	 *  all other fields
	 */
	l2tpv2cfg->recv_seq = data->l2tpv2.session.recv_seq;
	l2tpv2cfg->oip_ttl = ip_ttl_max;
	l2tpv2cfg->udp_csum = data->l2tpv2.tunnel.udp_csum;

	nss_connmgr_l2tpv2_info("%px: l2tpv2 info\n", nss_ctx);
	nss_connmgr_l2tpv2_info("%px: tunnel_id %d peer_tunnel_id %d session_id %d peer_session_id %d\n", nss_ctx,
											l2tpv2cfg->local_tunnel_id,
											l2tpv2cfg->peer_tunnel_id,
											l2tpv2cfg->local_session_id,
											l2tpv2cfg->peer_session_id);
	nss_connmgr_l2tpv2_info("%px: saddr 0x%x daddr 0x%x sport 0x%x  dport 0x%x\n", nss_ctx,
									l2tpv2cfg->sip, l2tpv2cfg->dip, l2tpv2cfg->sport, l2tpv2cfg->dport);
	nss_connmgr_l2tpv2_info("Sending l2tpv2 i/f up command to NSS %px\n", nss_ctx);

	nss_l2tpv2_msg_init(&l2tpv2msg, if_number, NSS_L2TPV2_MSG_SESSION_CREATE, sizeof(struct nss_l2tpv2_session_create_msg), NULL, NULL);

	status = nss_l2tpv2_tx(nss_ctx, &l2tpv2msg);
	if (status != NSS_TX_SUCCESS) {
		nss_unregister_l2tpv2_if(if_number);
		status = nss_dynamic_interface_dealloc_node(if_number, NSS_DYNAMIC_INTERFACE_TYPE_L2TPV2);
		if (status != NSS_TX_SUCCESS) {
			nss_connmgr_l2tpv2_warning("%px: Unable to dealloc the node[%d] in the NSS fw!\n", nss_ctx, if_number);
		}
		nss_connmgr_l2tpv2_warning("%px: nss l2tp session creation command error %d\n", nss_ctx, status);
		return NOTIFY_BAD;
	}

	nss_connmgr_l2tpv2_info("%px: nss_l2tpv2_tx() CREATE successful\n", nss_ctx);

	/*
	 * Check if we need to bind the L2TP to an IPsec interface. This is required as per RFC3193
	 */
	nss_connmgr_l2tpv2_bind_ipsec_by_ipaddr(nss_ctx, data, if_number);

	return NOTIFY_DONE;
}

/*
 * nss_connmgr_l2tpv2_dev_down()
 *	pppol2tpv2 interface's down event handler
 */
static int nss_connmgr_l2tpv2_dev_down(struct net_device *dev)
{
	struct nss_connmgr_l2tpv2_session_data *ptr;
	struct hlist_node *tmp;
	struct nss_l2tpv2_msg  l2tpv2msg;
	struct nss_l2tpv2_session_destroy_msg *l2tpv2cfg;
	int if_number;
	nss_tx_status_t status;

	/*
	 * check whether the interface is of type PPP
	 */
	if (dev->type != ARPHRD_PPP || !(dev->flags & IFF_POINTOPOINT)) {
		nss_connmgr_l2tpv2_info("netdevice is not a l2tpv2/l2tpv3 tunnel type\n");
		return NOTIFY_DONE;
	}

	/*
	 * Check if l2tpv2 is registered ?
	 */
	if_number = nss_cmn_get_interface_number_by_dev(dev);
	if (if_number < 0) {
		nss_connmgr_l2tpv2_info("Net device:%px is not registered with nss\n", dev);
		return NOTIFY_DONE;
	}

	hash_for_each_possible_safe(l2tpv2_session_data_hash_table, ptr,
				    tmp, hash_list, dev->ifindex) {
		if (ptr->dev == dev) {
			dev_put(dev);
			hash_del_rcu(&ptr->hash_list);
			synchronize_rcu();

			memset(&l2tpv2msg, 0, sizeof(struct nss_l2tpv2_msg));
			l2tpv2cfg = &l2tpv2msg.msg.session_destroy_msg;
			l2tpv2cfg->local_tunnel_id = ptr->data.l2tpv2.tunnel.tunnel_id;
			l2tpv2cfg->local_session_id = ptr->data.l2tpv2.session.session_id;
			kfree(ptr);

			nss_l2tpv2_msg_init(&l2tpv2msg, if_number, NSS_L2TPV2_MSG_SESSION_DESTROY, sizeof(struct nss_l2tpv2_session_destroy_msg), NULL, NULL);
			status = nss_l2tpv2_tx(nss_l2tpv2_get_context(), &l2tpv2msg);
			if (status != NSS_TX_SUCCESS) {
				nss_connmgr_l2tpv2_info("l2tpv2 session destroy command failed, if_number = %d\n", if_number);
				return NOTIFY_BAD;
			}

			nss_unregister_l2tpv2_if(if_number);
			status = nss_dynamic_interface_dealloc_node(if_number, NSS_DYNAMIC_INTERFACE_TYPE_L2TPV2);
			if (status != NSS_TX_SUCCESS) {
				nss_connmgr_l2tpv2_info("l2tpv2 dealloc node failure for if_number=%d\n", if_number);
				return NOTIFY_BAD;
			}
			nss_connmgr_l2tpv2_info("deleting l2tpv2session, if_number %d, tunnel_id %d, session_id %d\n",
									if_number, l2tpv2cfg->local_tunnel_id,  l2tpv2cfg->local_session_id);
			break;
		}
	}

	return NOTIFY_DONE;
}

/*
 * nss_connmgr_l2tpv2_dev_event()
 *	Net device notifier for nss l2tpv2 module
 */
static int nss_connmgr_l2tpv2_dev_event(struct notifier_block  *nb,
		unsigned long event, void  *dev)
{
	struct net_device *netdev;
	netdev = netdev_notifier_info_to_dev(dev);

	switch (event) {
	case NETDEV_UP:
		return nss_connmgr_l2tpv2_dev_up(netdev);

	case NETDEV_DOWN:
		return nss_connmgr_l2tpv2_dev_down(netdev);

	default:
		break;
	}

	return NOTIFY_DONE;
}

/*
 * nss_connmgr_l2tpv2_get_data()
 *	return 0 on success.
 */
int nss_connmgr_l2tpv2_get_data(struct net_device *dev, struct nss_connmgr_l2tpv2_data *data)
{
	struct nss_connmgr_l2tpv2_session_data *ptr;
	if (!data) {
		nss_connmgr_l2tpv2_info("nss_connmgr_l2tpv2_data ptr is null\n");
		return -EINVAL;
	}

	rcu_read_lock();
	hash_for_each_possible_rcu(l2tpv2_session_data_hash_table, ptr,
				   hash_list, dev->ifindex) {
		if (ptr->dev == dev) {
			memcpy(data, &ptr->data, sizeof(struct nss_connmgr_l2tpv2_data));
			rcu_read_unlock();
			return 0;
		}
	}
	rcu_read_unlock();

	return -ENODEV;
}
EXPORT_SYMBOL(nss_connmgr_l2tpv2_get_data);

/*
 * nss_connmgr_l2tpv2_does_connmgr_track()
 *	return 0 on success.
 */
int nss_connmgr_l2tpv2_does_connmgr_track(const struct net_device *dev)
{
	struct nss_connmgr_l2tpv2_session_data *ptr;

	rcu_read_lock();
	hash_for_each_possible_rcu(l2tpv2_session_data_hash_table, ptr,
				   hash_list, dev->ifindex) {
		if (ptr->dev == dev) {
			rcu_read_unlock();
			return 0;
		}
	}
	rcu_read_unlock();

	return -ENODEV;
}
EXPORT_SYMBOL(nss_connmgr_l2tpv2_does_connmgr_track);

/*
 * l2tpmgr_register_ipsecmgr_callback_by_ipaddr()
 *	Register IPSecmgr callback.
 */
void l2tpmgr_register_ipsecmgr_callback_by_ipaddr(struct l2tpmgr_ipsecmgr_cb *cb)
{
	get_ipsec_ifnum_by_ip_addr_callback_t ipsec_get_ifnum_by_ip_addr;

	rcu_read_lock();
	ipsec_get_ifnum_by_ip_addr = rcu_dereference(ipsecmgr_cb.get_ifnum_by_ip_addr);
	if (ipsec_get_ifnum_by_ip_addr) {
		rcu_read_unlock();
		nss_connmgr_l2tpv2_info("%px: IPSecmgr Callback get_ifnum_by_ip_addr is already registered\n", cb);
		return;
	}
	rcu_read_unlock();

	if (cb->get_ifnum_by_ip_addr == NULL) {
		nss_connmgr_l2tpv2_warning("%px: IPSecmgr Callback get_ifnum_by_ip_addr is NULL\n", cb);
		return;
	}

	rcu_assign_pointer(ipsecmgr_cb.get_ifnum_by_ip_addr, cb->get_ifnum_by_ip_addr);
	synchronize_rcu();
}
EXPORT_SYMBOL(l2tpmgr_register_ipsecmgr_callback_by_ipaddr);

/*
 * l2tpmgr_unregister_ipsecmgr_callback_by_ipaddr
 *	Unregister callback.
 */
void l2tpmgr_unregister_ipsecmgr_callback_by_ipaddr(void)
{
	rcu_assign_pointer(ipsecmgr_cb.get_ifnum_by_ip_addr, NULL);
	synchronize_rcu();
}
EXPORT_SYMBOL(l2tpmgr_unregister_ipsecmgr_callback_by_ipaddr);

#if defined(NSS_L2TP_IPSEC_BIND_BY_NETDEV)
/*
 * l2tpmgr_register_ipsecmgr_callback_by_netdev()
 *	Register IPSecmgr callback.
 */
void l2tpmgr_register_ipsecmgr_callback_by_netdev(struct l2tpmgr_ipsecmgr_cb *cb)
{
	get_ipsec_ifnum_by_dev_callback_t ipsec_get_ifnum_by_dev;

	rcu_read_lock();
	ipsec_get_ifnum_by_dev = rcu_dereference(ipsecmgr_cb.get_ifnum_by_dev);
	if (ipsec_get_ifnum_by_dev) {
		rcu_read_unlock();
		nss_connmgr_l2tpv2_info("%px: IPSecmgr Callback get_ifnum_by_dev is already registered\n", cb);
		return;
	}
	rcu_read_unlock();

	if (cb->get_ifnum_by_dev == NULL) {
		nss_connmgr_l2tpv2_warning("%px: IPSecmgr Callback get_ifnum_by_dev is NULL\n", cb);
		return;
	}

	rcu_assign_pointer(ipsecmgr_cb.get_ifnum_by_dev, cb->get_ifnum_by_dev);
	synchronize_rcu();
}
EXPORT_SYMBOL(l2tpmgr_register_ipsecmgr_callback_by_netdev);

/*
 * l2tpmgr_unregister_ipsecmgr_callback_by_netdev
 *	Unregister callback.
 */
void l2tpmgr_unregister_ipsecmgr_callback_by_netdev(void)
{
	rcu_assign_pointer(ipsecmgr_cb.get_ifnum_by_dev, NULL);
	synchronize_rcu();
}
EXPORT_SYMBOL(l2tpmgr_unregister_ipsecmgr_callback_by_netdev);

/*
 * nss_connmgr_l2tpv2_proc_handler()
 *	Read and write handler for sysctl.
 */
static int nss_connmgr_l2tpv2_proc_handler(struct ctl_table *ctl,
					  int write, void __user *buffer,
					  size_t *lenp, loff_t *ppos)
{
	char *l2tp_device_name, *ipsec_device_name;
	char *input_str = l2tpoipsec_config;
	int32_t l2tp_ifnum, ipsec_ifnum;
	struct net_device *l2tpdev, *ipsecdev;
	nss_tx_status_t status;
	struct nss_l2tpv2_msg l2tpv2msg;
	get_ipsec_ifnum_by_dev_callback_t ipsec_cb;
	struct nss_l2tpv2_bind_ipsec_if_msg *l2tpv2_bind_ipsec_msg;
	struct nss_ctx_instance *nss_ctx = nss_l2tpv2_get_context();
	int ret = proc_dostring(ctl, write, buffer, lenp, ppos);
	struct nss_connmgr_l2tpv2_session_data *ptr;

	if (!write) {
		nss_connmgr_l2tpv2_info("command to write is echo <l2tp-device-name> <ipsec-device-name> > <filename>\n");
		return ret;
	}

	l2tp_device_name = strsep(&input_str, " ");
	ipsec_device_name = strsep(&input_str, "\0");
	if (!l2tp_device_name) {
		nss_connmgr_l2tpv2_info("L2TP device name not found\n");
		nss_connmgr_l2tpv2_info("command is echo <l2tp-device-name> <ipsec-device-name> > <filename>\n");
		return -EINVAL;
	}

	if (!ipsec_device_name) {
		nss_connmgr_l2tpv2_info("IPsec device name not found\n");
		nss_connmgr_l2tpv2_info("command is echo <l2tp-device-name> <ipsec-device-name> > <filename>\n");
		return -EINVAL;
	}

	l2tpdev = dev_get_by_name(&init_net, l2tp_device_name);
	if (!l2tpdev) {
		nss_connmgr_l2tpv2_info("Cannot find the netdevice associated with %s\n", l2tp_device_name);
		return -EINVAL;
	}

	rcu_read_lock();
	hash_for_each_possible_rcu(l2tpv2_session_data_hash_table, ptr,
				   hash_list, l2tpdev->ifindex) {
		if (ptr->dev != l2tpdev) {
			continue;
		}

		if (ptr->data.l2tpv2.tunnel.udp_csum) {
			nss_connmgr_l2tpv2_info("Enabling UDP checksum in L2TP packet is not supported for l2tpoipsec flow\n");
		}
	}
	rcu_read_unlock();

	ipsecdev = dev_get_by_name(&init_net, ipsec_device_name);
	if (!ipsecdev) {
		nss_connmgr_l2tpv2_info("Cannot find the netdevice associated with %s\n", ipsec_device_name);
		dev_put(l2tpdev);
		return -EINVAL;
	}

	/*
	 * Get NSS ifnum for L2TP interface.
	 */
	l2tp_ifnum = nss_cmn_get_interface_number_by_dev(l2tpdev);
	if (l2tp_ifnum == -1) {
		nss_connmgr_l2tpv2_info("Cannot find the NSS interface associated with %s\n", l2tp_device_name);
		ret = -ENODEV;
		goto exit;
	}

	rcu_read_lock();
	ipsec_cb = rcu_dereference(ipsecmgr_cb.get_ifnum_by_dev);
	if (!ipsec_cb) {
		rcu_read_unlock();
		nss_connmgr_l2tpv2_info("Callback to get IPsec tun device not registered");
		ret = -EPERM;
		goto exit;
	}

	/*
	 * Get NSS ifnum for IPsec interface.
	 */
	ipsec_ifnum = ipsec_cb(ipsecdev);
	rcu_read_unlock();
	if (ipsec_ifnum == -1) {
		nss_connmgr_l2tpv2_info("Cannot find the NSS interface associated with %s\n", ipsec_device_name);
		ret = -ENODEV;
		goto exit;
	}

	/*
	 * Send the command to bind the l2tp session with the IPsec interface.
	 */
	memset(&l2tpv2msg, 0, sizeof(struct nss_l2tpv2_msg));
	nss_l2tpv2_msg_init(&l2tpv2msg, l2tp_ifnum, NSS_L2TPV2_MSG_BIND_IPSEC_IF, sizeof(struct nss_l2tpv2_bind_ipsec_if_msg), (void *)nss_connmgr_l2tpv2_msg_cb, NULL);
	l2tpv2_bind_ipsec_msg = &l2tpv2msg.msg.bind_ipsec_if_msg;
	l2tpv2_bind_ipsec_msg->ipsec_ifnum = ipsec_ifnum;
	status = nss_l2tpv2_tx(nss_ctx, &l2tpv2msg);
	if (status != NSS_TX_SUCCESS) {
		nss_connmgr_l2tpv2_info("%px IPSec interface bind failed\n", nss_ctx);
		ret = -EAGAIN;
	}

exit:
	dev_put(l2tpdev);
	dev_put(ipsecdev);
	return ret;
}

/*
 * nss_connmgr_l2tpv2_table
 */
static struct ctl_table nss_connmgr_l2tpv2_table[] = {
	{
		.procname	= "l2tpoipsec",
		.data		= &l2tpoipsec_config,
		.maxlen		= L2TP_SYSCTL_STR_LEN_MAX,
		.mode		= 0644,
		.proc_handler	= &nss_connmgr_l2tpv2_proc_handler,
	},
	{ }
};

/*
 * nss_connmgr_l2tpv2_dir
 */
static struct ctl_table nss_connmgr_l2tpv2_dir[] = {
	{
		.procname		= "l2tpv2",
		.mode			= 0555,
		.child			= nss_connmgr_l2tpv2_table,
	},
	{ }
};

/*
 * nss_connmgr_l2tpv2_sysroot sysctl root dir
 */
static struct ctl_table nss_connmgr_l2tpv2_sysroot[] = {
	{
		.procname		= "nss",
		.mode			= 0555,
		.child			= nss_connmgr_l2tpv2_dir,
	},
	{ }
};
#endif

/*
 * Linux Net device Notifier
 */
struct notifier_block nss_connmgr_l2tpv2_notifier = {
	.notifier_call = nss_connmgr_l2tpv2_dev_event,
};

/*
 * nss_connmgr_l2tpv2_init_module()
 *	Tunnel l2tpv2 module init function
 */
int __init nss_connmgr_l2tpv2_init_module(void)
{
#ifdef CONFIG_OF
	/*
	 * If the node is not compatible, don't do anything.
	 */
	if (!of_find_node_by_name(NULL, "nss-common")) {
		return 0;
	}
#endif
#if defined(NSS_L2TP_IPSEC_BIND_BY_NETDEV)
	ctl_tbl_hdr = register_sysctl("dev/nss/l2tpv2", nss_connmgr_l2tpv2_table);
	if (!ctl_tbl_hdr) {
		nss_connmgr_l2tpv2_info("Unable to register sysctl table for L2TP conn mgr\n");
		return -EFAULT;
	}

#endif
	/*
	 * Initialize ipsecmgr callback.
	 */
	rcu_assign_pointer(ipsecmgr_cb.get_ifnum_by_dev, NULL);
	rcu_assign_pointer(ipsecmgr_cb.get_ifnum_by_ip_addr, NULL);
	synchronize_rcu();
	register_netdevice_notifier(&nss_connmgr_l2tpv2_notifier);
	return 0;
}

/*
 * nss_connmgr_l2tpv2_exit_module
 *	Tunnel l2tpv2 module exit function
 */
void __exit nss_connmgr_l2tpv2_exit_module(void)
{
#ifdef CONFIG_OF
	/*
	 * If the node is not compatible, don't do anything.
	 */
	if (!of_find_node_by_name(NULL, "nss-common")) {
		return;
	}
#endif
	unregister_netdevice_notifier(&nss_connmgr_l2tpv2_notifier);
}

module_init(nss_connmgr_l2tpv2_init_module);
module_exit(nss_connmgr_l2tpv2_exit_module);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS l2tpv2 over ppp offload manager");
