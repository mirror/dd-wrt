/*
 **************************************************************************
 * Copyright (c) 2015-2017 The Linux Foundation. All rights reserved.
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
#include <net/route.h>
#include <linux/../../net/l2tp/l2tp_core.h>

#include <nss_api_if.h>
#include <nss_dynamic_interface.h>
#include "nss_connmgr_l2tpv2.h"
#include "nss_l2tpv2_stats.h"

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

static DEFINE_HASHTABLE(l2tpv2_session_data_hash_table, HASH_BUCKET_SIZE);
static int ip_ttl_max = 255;

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
		nss_connmgr_l2tpv2_info("tunnel is null for session %p\n", session);
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
	data->l2tpv2.session.offset = 0;
	data->l2tpv2.session.hdr_len = session->hdr_len;
	data->l2tpv2.session.reorder_timeout = session->reorder_timeout;
	data->l2tpv2.session.recv_seq = session->recv_seq;
	data->l2tpv2.session.send_seq = session->send_seq;

	nss_connmgr_l2tpv2_info("sess %u, peer=%u nr=%u ns=%u off=%u  hdr_len=%u timeout=%x"
	       " recv_seq=%x send_seq=%x\n",
	       session->session_id,  session->peer_session_id, session->nr,
	       session->ns, 0, session->hdr_len,
	       session->reorder_timeout, session->recv_seq,
	       session->send_seq);

#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3, 16, 0))
	data->l2tpv2.tunnel.udp_csum = tunnel->sock->sk_no_check;
#else
	data->l2tpv2.tunnel.udp_csum = tunnel->sock->sk_no_check_tx;
#endif

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
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3, 8, 0))
	struct hlist_node *node;
#endif
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
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3, 8, 0))
				   node,
#endif
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
	nss_connmgr_l2tpv2_info("%p: nss_register_l2tpv2_if() successful\n", nss_ctx);

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

	nss_connmgr_l2tpv2_info("%p: l2tpv2 info\n", nss_ctx);
	nss_connmgr_l2tpv2_info("%p: tunnel_id %d peer_tunnel_id %d session_id %d peer_session_id %d\n", nss_ctx,
											l2tpv2cfg->local_tunnel_id,
											l2tpv2cfg->peer_tunnel_id,
											l2tpv2cfg->local_session_id,
											l2tpv2cfg->peer_session_id);
	nss_connmgr_l2tpv2_info("%p: saddr 0x%x daddr 0x%x sport 0x%x  dport 0x%x\n", nss_ctx,
									l2tpv2cfg->sip, l2tpv2cfg->dip, l2tpv2cfg->sport, l2tpv2cfg->dport);
	nss_connmgr_l2tpv2_info("Sending l2tpv2 i/f up command to NSS %p\n", nss_ctx);

	nss_l2tpv2_msg_init(&l2tpv2msg, if_number, NSS_L2TPV2_MSG_SESSION_CREATE, sizeof(struct nss_l2tpv2_session_create_msg), NULL, NULL);

	status = nss_l2tpv2_tx(nss_ctx, &l2tpv2msg);
	if (status != NSS_TX_SUCCESS) {
		nss_unregister_l2tpv2_if(if_number);
		status = nss_dynamic_interface_dealloc_node(if_number, NSS_DYNAMIC_INTERFACE_TYPE_L2TPV2);
		if (status != NSS_TX_SUCCESS) {
			nss_connmgr_l2tpv2_warning("%p: Unable to dealloc the node[%d] in the NSS fw!\n", nss_ctx, if_number);
		}
		nss_connmgr_l2tpv2_warning("%p: nss l2tp session creation command error %d\n", nss_ctx, status);
		return NOTIFY_BAD;
	}

	nss_connmgr_l2tpv2_info("%p: nss_l2tpv2_tx() successful\n", nss_ctx);

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
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3, 8, 0))
	struct hlist_node *node;
#endif
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
		nss_connmgr_l2tpv2_info("Net device:%p is not registered with nss\n", dev);
		return NOTIFY_DONE;
	}

	hash_for_each_possible_safe(l2tpv2_session_data_hash_table, ptr,
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3, 8, 0))
				    node,
#endif
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
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3, 10, 0))
	netdev = (struct net_device *)dev;
#else
	netdev = netdev_notifier_info_to_dev(dev);
#endif

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
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3, 8, 0))
	struct hlist_node  *node;
#endif
	if (!data) {
		nss_connmgr_l2tpv2_info("nss_connmgr_l2tpv2_data ptr is null\n");
		return -EINVAL;
	}

	rcu_read_lock();
	hash_for_each_possible_rcu(l2tpv2_session_data_hash_table, ptr,
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3, 8, 0))
				   node,
#endif
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
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3, 8, 0))
	struct hlist_node  *node;
#endif

	rcu_read_lock();
	hash_for_each_possible_rcu(l2tpv2_session_data_hash_table, ptr,
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3, 8, 0))
				   node,
#endif
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
