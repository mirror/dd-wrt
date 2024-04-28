/*
 **************************************************************************
 * Copyright (c) 2015-2018, The Linux Foundation. All rights reserved.
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

#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/rwlock_types.h>
#include <linux/hashtable.h>
#include <linux/inetdevice.h>
#include <linux/ip.h>
#include <net/ipv6.h>
#include <linux/if_arp.h>
#include <net/route.h>
#include <linux/if_pppox.h>
#include <net/ip.h>

#ifdef CONFIG_OF
#include <linux/of.h>
#endif

#include <nss_api_if.h>
#include <nss_dynamic_interface.h>
#include "nss_connmgr_pptp.h"

#define PPP_HDR_LEN        4

/*
 * NSS pptp debug macros
 */
#if (NSS_PPTP_DEBUG_LEVEL < 1)
#define nss_connmgr_pptp_assert(fmt, args...)
#else
#define nss_connmgr_pptp_assert(c) if (!(c)) { BUG_ON(!(c)); }
#endif

#if defined(CONFIG_DYNAMIC_DEBUG)
/*
 * Compile messages for dynamic enable/disable
 */
#define nss_connmgr_pptp_warning(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define nss_connmgr_pptp_info(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define nss_connmgr_pptp_trace(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else

/*
 * Statically compile messages at different levels
 */
#if (NSS_PPTP_DEBUG_LEVEL < 2)
#define nss_connmgr_pptp_warning(s, ...)
#else
#define nss_connmgr_pptp_warning(s, ...) pr_warn("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_PPTP_DEBUG_LEVEL < 3)
#define nss_connmgr_pptp_info(s, ...)
#else
#define nss_connmgr_pptp_info(s, ...)   pr_notice("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_PPTP_DEBUG_LEVEL < 4)
#define nss_connmgr_pptp_trace(s, ...)
#else
#define nss_connmgr_pptp_trace(s, ...)  pr_info("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif
#endif

#define NUM_PPTP_CHANNELS_IN_PPP_NETDEVICE  1
#define HASH_BUCKET_SIZE 2  /* ( 2^ HASH_BUCKET_SIZE ) == 4 */

static DEFINE_HASHTABLE(pptp_session_table, HASH_BUCKET_SIZE);

/*
 * nss_connmgr_pptp_client_xmit()
 * 	PPTP GRE seq/ack offload callback handler. Sends SKB to NSS firmware.
 * 	Note: RCU lock is already held by caller.
 */
static int nss_connmgr_pptp_client_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct nss_connmgr_pptp_session_entry *session_info;
	struct nss_ctx_instance *nss_pptp_ctx;
	nss_tx_status_t status;
	int host_inner_if;;

	/*
	 * Check if pptp host inner I/F is registered ?
	 */
	host_inner_if = nss_cmn_get_interface_number_by_dev_and_type(dev, NSS_DYNAMIC_INTERFACE_TYPE_PPTP_HOST_INNER);
	if (host_inner_if < 0) {
		nss_connmgr_pptp_warning("%p: Net device is not registered\n", dev);
		return -1;
	}

	nss_pptp_ctx = nss_pptp_get_context();
	if (!nss_pptp_ctx) {
		nss_connmgr_pptp_warning("%p: NSS PPTP context not found for if_number %d\n", dev, host_inner_if);
		return -1;
	}

	hash_for_each_possible_rcu(pptp_session_table, session_info,
				    hash_list, dev->ifindex) {
		if (session_info->dev != dev) {
			continue;
		}

		status = nss_pptp_tx_buf(nss_pptp_ctx, host_inner_if, skb);
		if (status == NSS_TX_SUCCESS) {
			/*
			 * Found a match for a session and successfully posted
			 * packet to firmware. Retrun success.
			 */
			nss_connmgr_pptp_info("%p: NSS FW tx success if_number %d\n", dev, host_inner_if);
			return 0;
		}

		nss_connmgr_pptp_info("%p: NSS FW tx failed if_number %d\n", dev, host_inner_if);
		return -1;
	}

	/* Return error, Could not find a match for session */
	return -1;
}

/*
 * nss_connmgr_pptp_get_session()
 *	Retrieve pptp session associated with this netdevice if any
 */
static int nss_connmgr_pptp_get_session(struct net_device *dev, struct pptp_opt *opt)
{
	struct ppp_channel *channel[1] = {NULL};
	int px_proto;
	int ppp_ch_count;

	/*
	 * check whether the interface is of type PPP
	 */
	if (dev->type != ARPHRD_PPP || !(dev->priv_flags & IFF_PPP_PPTP)) {
		nss_connmgr_pptp_info("%p: netdevice is not a PPP tunnel type\n", dev);
		return -1;
	}

	if (ppp_is_multilink(dev)) {
		nss_connmgr_pptp_warning("%p: channel is multilink PPP\n", dev);
		return -1;
	}

	ppp_ch_count = ppp_hold_channels(dev, channel, 1);
	nss_connmgr_pptp_info("%p: PPP hold channel ret %d\n", dev, ppp_ch_count);
	if (ppp_ch_count != 1) {
		nss_connmgr_pptp_warning("%p: hold channel for netdevice failed\n", dev);
		return -1;
	}

	if (!channel[0]) {
		nss_connmgr_pptp_warning("%p: channel don't have a ppp_channel\n", dev);
		return -1;
	}

	px_proto = ppp_channel_get_protocol(channel[0]);
	if (px_proto != PX_PROTO_PPTP) {
		nss_connmgr_pptp_warning("%p: session socket is not of type PX_PROTO_PPTP\n", dev);
		ppp_release_channels(channel, 1);
		return -1;
	}

	pptp_channel_addressing_get(opt, channel[0]);

	ppp_release_channels(channel, 1);
	return 0;
}

/*
 * nss_connmgr_add_pptp_session()
 *	Add PPTP session entry into Hash table
 */
static struct nss_connmgr_pptp_session_entry *nss_connmgr_add_pptp_session(struct net_device *dev, struct pptp_opt *session)

{
	struct nss_connmgr_pptp_session_entry *pptp_session_data = NULL;
	struct nss_connmgr_pptp_session_info *data;
	struct net_device *physical_dev;

	pptp_session_data = kmalloc(sizeof(struct nss_connmgr_pptp_session_entry),
				      GFP_KERNEL);
	if (!pptp_session_data) {
		nss_connmgr_pptp_warning("%p: failed to allocate pptp_session_data\n", dev);
		return NULL;
	}

	data = &pptp_session_data->data;

	/*
	 * Get session info
	 */
	data->src_call = session->src_addr.call_id;
	data->dst_call = session->dst_addr.call_id;
	data->src_ip = session->src_addr.sin_addr.s_addr;
	data->dst_ip = session->dst_addr.sin_addr.s_addr;

	nss_connmgr_pptp_info("%p: src_call_id=%u peer_call_id=%u\n", dev, data->src_call, data->dst_call);

	/*
	 * This netdev hold will be released when netdev
	 * down event arrives and session goes down.
	 */
	dev_hold(dev);
	pptp_session_data->dev = dev;

	/*
	 * Note: ip_dev_find does a hold on the physical device,
	 * which is released when PPTP session goes down
	 */
	physical_dev = ip_dev_find(&init_net, data->src_ip);
	if (!physical_dev) {
		nss_connmgr_pptp_warning("%p: couldn't find a phycal dev %s\n", dev, dev->name);
		dev_put(dev);
		kfree(pptp_session_data);
		return NULL;
	}

	pptp_session_data->phy_dev = physical_dev;

	/*
	 * There is no need for protecting simultaneous addition &
	 * deletion of pptp_session_data as all netdev notifier chain
	 * call back is called with rtln mutex lock.
	 */
	hash_add_rcu(pptp_session_table,
		&pptp_session_data->hash_list,
		dev->ifindex);

	return pptp_session_data;
}

/*
 * nss_connmgr_pptp_event_receive()
 *	Event Callback to receive events from NSS
 */
static void nss_connmgr_pptp_event_receive(void *if_ctx, struct nss_pptp_msg *tnlmsg)
{
	struct nss_connmgr_pptp_session_entry *session_info = (struct nss_connmgr_pptp_session_entry *)if_ctx;
	struct net_device *netdev = session_info->dev;
	struct nss_pptp_sync_session_stats_msg *sync_stats;
	uint32_t if_type;

	switch (tnlmsg->cm.type) {
	case NSS_PPTP_MSG_SYNC_STATS:
		if (!netdev) {
			return;
		}

		nss_connmgr_pptp_info("%p: Update PPP stats for PPTP netdev %p\n", session_info, netdev);
		sync_stats = (struct nss_pptp_sync_session_stats_msg *)&tnlmsg->msg.stats;
		dev_hold(netdev);

		if_type = nss_dynamic_interface_get_type(nss_pptp_get_context(), tnlmsg->cm.interface);

		if (if_type == NSS_DYNAMIC_INTERFACE_TYPE_PPTP_OUTER) {
			ppp_update_stats(netdev,
				 (unsigned long)sync_stats->node_stats.rx_packets,
				 (unsigned long)sync_stats->node_stats.rx_bytes,
				 0, 0, 0, 0, 0, 0);
		} else {

			ppp_update_stats(netdev, 0, 0,
				 (unsigned long)sync_stats->node_stats.tx_packets,
				 (unsigned long)sync_stats->node_stats.tx_bytes,
				  0, 0, 0, 0);
		}

		dev_put(netdev);
		break;

	default:
		nss_connmgr_pptp_warning("%p: Unknown Event from NSS\n", session_info);
		break;
	}
}

/*
 * nss_connmgr_pptp_decap_exception()
 *	Exception handler registered to NSS for handling pptp pkts
 */
static void nss_connmgr_pptp_decap_exception(struct net_device *dev,
				       struct sk_buff *skb,
				       __attribute__((unused)) struct napi_struct *napi)

{
	struct iphdr *iph_outer;
	struct nss_connmgr_pptp_session_entry *session_info;
	struct flowi4 fl4;
	struct nss_pptp_gre_hdr *gre_hdr;
	__be32 tunnel_local_ip;
	__be32 tunnel_peer_ip;
	struct rtable *rt;

	/* discard L2 header */
	skb_pull(skb, sizeof(struct ethhdr));
	skb_reset_mac_header(skb);

	iph_outer = (struct iphdr *)skb->data;

	rcu_read_lock();
	hash_for_each_possible_rcu(pptp_session_table, session_info,
				   hash_list, dev->ifindex) {
		if (session_info->dev != dev) {
			continue;
		}

		tunnel_local_ip = session_info->data.src_ip;
		tunnel_peer_ip = session_info->data.dst_ip;
		rcu_read_unlock();
		if ((iph_outer->version == 4) && (iph_outer->protocol == IPPROTO_GRE) &&
			(iph_outer->saddr == tunnel_local_ip) && (iph_outer->daddr == tunnel_peer_ip)) { /*pkt is encapsulated */

			/*
			 * Pull the outer IP header and confirm the packet is a PPTP GRE Packet
			 */
			skb_pull(skb, sizeof(struct iphdr));
			gre_hdr = (struct nss_pptp_gre_hdr *)skb->data;
			if ((ntohs(gre_hdr->protocol) != NSS_PPTP_GRE_PROTO) &&
				(gre_hdr->flags_ver == NSS_PPTP_GRE_VER)) {
				nss_connmgr_pptp_warning("%p, Not PPTP_GRE_PROTO, so freeing\n", dev);
				dev_kfree_skb_any(skb);
				return;
			}

			skb_push(skb, sizeof(struct iphdr));

			/*
			 * This is a PPTP encapsulated packet that has been exceptioned to host from NSS.
			 * We can send it directly to the physical device
			 */
			rt = ip_route_output_ports(&init_net, &fl4, NULL, tunnel_peer_ip,
					tunnel_local_ip, 0, 0, IPPROTO_GRE, RT_TOS(0), 0);
			if (unlikely(IS_ERR(rt))) {
				nss_connmgr_pptp_warning("%p: Martian packets, drop\n", dev);
				nss_connmgr_pptp_warning("%p: No route or out dev, drop packet...\n", dev);
				dev_kfree_skb_any(skb);
				return;
			}

			if (likely(rt->dst.dev)) {
				nss_connmgr_pptp_info("%p: dst route dev is %s\n", session_info, rt->dst.dev->name);
			} else {
				nss_connmgr_pptp_warning("%p: No out dev, drop packet...\n", dev);
				dev_kfree_skb_any(skb);
			}

			/*
			 * Sets the 'dst' entry for SKB, reset the IP and Transport
			 * Header and sends the packet out directly to the physical
			 * device associated with the PPTP tunnel interface.
			 */
			skb->dev = dev;
			skb_dst_drop(skb);
			skb_dst_set(skb, &rt->dst);
			skb->ip_summed = CHECKSUM_COMPLETE;

			skb_reset_network_header(skb);
			skb_set_transport_header(skb, iph_outer->ihl*4);
			skb->skb_iif = dev->ifindex;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 4, 0))
			ip_local_out(skb);
#else
			ip_local_out(&init_net, skb->sk, skb);
#endif
			return;
		}
	}
	rcu_read_unlock();
	nss_connmgr_pptp_warning("%p: unable to find session for PPTP exception packet from %s, so freeing\n", dev, dev->name);
	dev_kfree_skb_any(skb);
}

/*
 * nss_connmgr_pptp_encap_exception()
 *	Exception handler registered to NSS for handling pptp pkts
 */
static void nss_connmgr_pptp_encap_exception(struct net_device *dev,
				       struct sk_buff *skb,
				       __attribute__((unused)) struct napi_struct *napi)

{
	struct iphdr *iph_outer;
	struct nss_connmgr_pptp_session_entry *session_info;
	__be32 tunnel_local_ip;
	__be32 tunnel_peer_ip;

	/* discard L2 header */
	skb_pull(skb, sizeof(struct ethhdr));
	skb_reset_mac_header(skb);

	iph_outer = (struct iphdr *)skb->data;

	rcu_read_lock();
	hash_for_each_possible_rcu(pptp_session_table, session_info,
				   hash_list, dev->ifindex) {
		if (session_info->dev != dev) {
			continue;
		}

		tunnel_local_ip = session_info->data.src_ip;
		tunnel_peer_ip = session_info->data.dst_ip;
		rcu_read_unlock();

		if (iph_outer->version == 4) {
			skb->protocol = htons(ETH_P_IP);
		} else if (iph_outer->version == 6) {
			skb->protocol = htons(ETH_P_IPV6);
		} else {
			nss_connmgr_pptp_info("%p: pkt may be a control packet\n", dev);
		}

		skb_reset_network_header(skb);
		skb->pkt_type = PACKET_HOST;
		skb->skb_iif = dev->ifindex;
		skb->ip_summed = CHECKSUM_NONE;
		skb->dev = dev;
		nss_connmgr_pptp_info("%p: send decapsulated packet through network stack", dev);
		netif_receive_skb(skb);
		return;
	}
	rcu_read_unlock();
	nss_connmgr_pptp_warning("%p: unable to find session for PPTP exception packet from %s, so freeing\n", dev, dev->name);
	dev_kfree_skb_any(skb);
}

/*
 * nss_connmgr_pptp_dev_up()
 *	pppopptp interface's up event handler
 */
static int nss_connmgr_pptp_dev_up(struct net_device *dev)
{
	struct pptp_opt opt;
	struct nss_connmgr_pptp_session_entry *session_info = NULL;
	struct nss_connmgr_pptp_session_info *data;
	nss_tx_status_t status;
	struct nss_ctx_instance *nss_ctx;
	uint32_t features = 0;
	int32_t inner_if, outer_if, host_inner_if;
	struct nss_pptp_msg  pptpmsg;
	struct nss_pptp_session_configure_msg *pptpcfg;
	int ret;

	ret = nss_connmgr_pptp_get_session(dev, &opt);
	if (ret < 0) {
		return NOTIFY_DONE;
	}

	/*
	 * Create nss dynamic interface and register
	 */
	inner_if = nss_dynamic_interface_alloc_node(NSS_DYNAMIC_INTERFACE_TYPE_PPTP_INNER);
	if (inner_if < 0) {
		nss_connmgr_pptp_warning("%p: Request inner interface number failed\n", dev);
		return NOTIFY_DONE;
	}

	if (!nss_is_dynamic_interface(inner_if)) {
		nss_connmgr_pptp_warning("%p: Invalid NSS dynamic I/F number %d\n", dev, inner_if);
		goto inner_fail;
	}

	outer_if = nss_dynamic_interface_alloc_node(NSS_DYNAMIC_INTERFACE_TYPE_PPTP_OUTER);
	if (outer_if < 0) {
		nss_connmgr_pptp_warning("%p: Request outer interface number failed\n", dev);
		goto inner_fail;
	}

	if (!nss_is_dynamic_interface(outer_if)) {
		nss_connmgr_pptp_warning("%p: Invalid NSS dynamic I/F number %d\n", dev, outer_if);
		goto outer_fail;
	}

	host_inner_if = nss_dynamic_interface_alloc_node(NSS_DYNAMIC_INTERFACE_TYPE_PPTP_HOST_INNER);
	if (host_inner_if < 0) {
		nss_connmgr_pptp_warning("%p: Request host inner interface number failed\n", dev);
		goto outer_fail;
	}

	if (!nss_is_dynamic_interface(host_inner_if)) {
		nss_connmgr_pptp_warning("%p: Invalid NSS dynamic I/F number %d\n", dev, host_inner_if);
		goto host_inner_fail;
	}

	session_info = nss_connmgr_add_pptp_session(dev, &opt);
	if (!session_info) {
		nss_connmgr_pptp_warning("%p: PPTP session add failed\n", dev);
		goto host_inner_fail;
	}

	/*
	 * Register pptp tunnel inner interface with NSS
	 */
	nss_ctx = nss_register_pptp_if(inner_if,
				NSS_DYNAMIC_INTERFACE_TYPE_PPTP_INNER,
				nss_connmgr_pptp_encap_exception,
				nss_connmgr_pptp_event_receive,
				dev,
				features,
				session_info);

	if (!nss_ctx) {
		nss_connmgr_pptp_warning("%p: nss_register_pptp_if failed for inner\n", dev);
		goto register_inner_if_fail;
	}

	nss_connmgr_pptp_info("%p: inner interface registration successful\n", nss_ctx);

	/*
	 * Register pptp tunnel outer interface with NSS
	 */
	nss_ctx = nss_register_pptp_if(outer_if,
				NSS_DYNAMIC_INTERFACE_TYPE_PPTP_OUTER,
				nss_connmgr_pptp_decap_exception,
				nss_connmgr_pptp_event_receive,
				dev,
				features,
				session_info);

	if (!nss_ctx) {
		nss_connmgr_pptp_warning("%p: nss_register_pptp_if failed for outer\n", dev);
		goto register_outer_if_fail;
	}

	nss_connmgr_pptp_info("%p: outer interface registration successful\n", nss_ctx);

	/*
	 * Register pptp tunnel inner interface with NSS
	 */
	nss_ctx = nss_register_pptp_if(host_inner_if,
				NSS_DYNAMIC_INTERFACE_TYPE_PPTP_HOST_INNER,
				nss_connmgr_pptp_encap_exception,
				nss_connmgr_pptp_event_receive,
				dev,
				features,
				session_info);

	if (!nss_ctx) {
		nss_connmgr_pptp_warning("%p: nss_register_pptp_if failed for host inner\n", dev);
		goto register_host_inner_if_fail;
	}

	nss_connmgr_pptp_info("%p: host inner interface registration successful\n", nss_ctx);

	/*
	 * Initialize and configure inner I/F.
	 */
	data = &session_info->data;

	memset(&pptpmsg, 0, sizeof(struct nss_pptp_msg));
	pptpcfg = &pptpmsg.msg.session_configure_msg;

	/*
	 * The call id is already in host byte order,
	 * therefore no need to do ntohs() for call id.
	 */
	pptpcfg->src_call_id = data->src_call;
	pptpcfg->dst_call_id = data->dst_call;

	/*
	 * Convert IP addresses from nework byte order
	 * to host byte order before posting to firmware.
	 */
	pptpcfg->sip = ntohl(data->src_ip);
	pptpcfg->dip = ntohl(data->dst_ip);

	/*
	 * Populate the sibling interfaces.
	 */
	pptpcfg->sibling_ifnum_pri = outer_if;
	pptpcfg->sibling_ifnum_aux = host_inner_if;

	nss_connmgr_pptp_info("%p: pptp info\n", dev);
	nss_connmgr_pptp_info("%p: local_call_id %d peer_call_id %d\n", dev,
									pptpcfg->src_call_id,
									pptpcfg->dst_call_id);
	nss_connmgr_pptp_info("%p: saddr 0x%x daddr 0x%x \n", dev, pptpcfg->sip, pptpcfg->dip);

	nss_pptp_msg_init(&pptpmsg, inner_if, NSS_PPTP_MSG_SESSION_CONFIGURE, sizeof(struct nss_pptp_session_configure_msg), NULL, NULL);

	status = nss_pptp_tx_msg_sync(nss_ctx, &pptpmsg);
	if (status != NSS_TX_SUCCESS) {
		nss_connmgr_pptp_warning("%p: nss pptp session creation command error %d\n", dev, status);
		goto tx_msg_fail;
	}
	nss_connmgr_pptp_info("%p: nss_pptp_tx() successful for inner\n", dev);

	/*
	 * Initialize and configure outer I/F.
	 */
	pptpcfg->sibling_ifnum_pri = inner_if;
	pptpcfg->sibling_ifnum_aux = host_inner_if;

	nss_pptp_msg_init(&pptpmsg, outer_if, NSS_PPTP_MSG_SESSION_CONFIGURE, sizeof(struct nss_pptp_session_configure_msg), NULL, NULL);

	status = nss_pptp_tx_msg_sync(nss_ctx, &pptpmsg);
	if (status != NSS_TX_SUCCESS) {
		nss_connmgr_pptp_warning("%p: nss pptp session creation command error %d\n", dev, status);
		goto tx_msg_fail;
	}
	nss_connmgr_pptp_info("%p: nss_pptp_tx() successful for outer\n", dev);

	/*
	 * Initialize and configure host inner I/F.
	 */
	pptpcfg->sibling_ifnum_pri = outer_if;
	pptpcfg->sibling_ifnum_aux = inner_if;

	nss_pptp_msg_init(&pptpmsg, host_inner_if, NSS_PPTP_MSG_SESSION_CONFIGURE, sizeof(struct nss_pptp_session_configure_msg), NULL, NULL);

	status = nss_pptp_tx_msg_sync(nss_ctx, &pptpmsg);
	if (status != NSS_TX_SUCCESS) {
		nss_connmgr_pptp_warning("%p: nss pptp session creation command error %d\n", dev, status);
		goto tx_msg_fail;
	}
	nss_connmgr_pptp_info("%p: nss_pptp_tx() successful for host inner\n", dev);

	/*
	 * Enable the offload mode for Linux PPTP kernel driver. After this
	 * all PPTP GRE packets will go through the NSS FW.
	 */
	pptp_session_enable_offload_mode(data->dst_call, data->dst_ip);

	return NOTIFY_DONE;

tx_msg_fail:
	nss_unregister_pptp_if(host_inner_if);
register_host_inner_if_fail:
	nss_unregister_pptp_if(outer_if);
register_outer_if_fail:
	nss_unregister_pptp_if(inner_if);
register_inner_if_fail:
		dev_put(dev); /* We are accessing dev later */
		dev_put(session_info->phy_dev);
		hash_del_rcu(&session_info->hash_list);
		synchronize_rcu();
		kfree(session_info);
host_inner_fail:
	status = nss_dynamic_interface_dealloc_node(host_inner_if, NSS_DYNAMIC_INTERFACE_TYPE_PPTP_HOST_INNER);
	if (status != NSS_TX_SUCCESS) {
		nss_connmgr_pptp_warning("%p: Unable to dealloc the node[%d] in the NSS fw!\n", dev, host_inner_if);
	}
outer_fail:
	status = nss_dynamic_interface_dealloc_node(inner_if, NSS_DYNAMIC_INTERFACE_TYPE_PPTP_OUTER);
	if (status != NSS_TX_SUCCESS) {
		nss_connmgr_pptp_warning("%p: Unable to dealloc the node[%d] in the NSS fw!\n", dev, outer_if);
	}
inner_fail:
	status = nss_dynamic_interface_dealloc_node(inner_if, NSS_DYNAMIC_INTERFACE_TYPE_PPTP_INNER);
	if (status != NSS_TX_SUCCESS) {
		nss_connmgr_pptp_warning("%p: Unable to dealloc the node[%d] in the NSS fw!\n", dev, inner_if);
	}

	return NOTIFY_DONE;
}

/*
 * nss_connmgr_pptp_dev_down()
 *	pptp interface's down event handler
 */
static int nss_connmgr_pptp_dev_down(struct net_device *dev)
{
	struct nss_connmgr_pptp_session_entry *session_info;
	struct nss_connmgr_pptp_session_entry *session_found = NULL;
	struct hlist_node *tmp;

	struct nss_pptp_msg pptpmsg;
	struct nss_pptp_session_deconfigure_msg *pptpcfg;
	nss_tx_status_t status;
	int32_t inner_if, outer_if, host_inner_if;

	/*
	 * check whether the interface is of type PPP
	 */
	if (dev->type != ARPHRD_PPP || !(dev->priv_flags & IFF_PPP_PPTP)) {
		nss_connmgr_pptp_info("%p: netdevice is not a pptp tunnel type\n", dev);
		return NOTIFY_DONE;
	}

	/*
	 * Check if pptp inner I/F is registered ?
	 */
	inner_if = nss_cmn_get_interface_number_by_dev_and_type(dev, NSS_DYNAMIC_INTERFACE_TYPE_PPTP_INNER);
	if (inner_if < 0) {
		nss_connmgr_pptp_warning("%p: outer I/F is not registered\n", dev);
		return NOTIFY_DONE;
	}

	/*
	 * Check if pptp outer I/F is registered ?
	 */
	outer_if = nss_cmn_get_interface_number_by_dev_and_type(dev, NSS_DYNAMIC_INTERFACE_TYPE_PPTP_OUTER);
	if (outer_if < 0) {
		nss_connmgr_pptp_warning("%p: inner I/F is not registered\n", dev);
		return NOTIFY_DONE;
	}

	/*
	 * Check if pptp host inner I/F is registered ?
	 */
	host_inner_if = nss_cmn_get_interface_number_by_dev_and_type(dev, NSS_DYNAMIC_INTERFACE_TYPE_PPTP_HOST_INNER);
	if (host_inner_if < 0) {
		nss_connmgr_pptp_warning("%p: Net device is not registered\n", dev);
		return NOTIFY_DONE;
	}

	hash_for_each_possible_safe(pptp_session_table, session_info,
				    tmp, hash_list, dev->ifindex) {
		if (session_info->dev != dev) {
			continue;
		}

		session_found = session_info;
		break;
	}

	if (!session_found) {
		nss_connmgr_pptp_warning("%p: pptp session is not found for this device", dev);
		return NOTIFY_DONE;
	}

	/*
	 * Disable the pptp offload mode. This will allow all PPTP GRE packets
	 * to go through linux PPTP kernel module.
	 */
	pptp_session_disable_offload_mode(session_info->data.dst_call, session_info->data.dst_ip);
	dev_put(dev);
	dev_put(session_info->phy_dev);
	hash_del_rcu(&session_info->hash_list);
	synchronize_rcu();

	memset(&pptpmsg, 0, sizeof(struct nss_pptp_msg));
	pptpcfg = &pptpmsg.msg.session_deconfigure_msg;
	pptpcfg->src_call_id = session_info->data.src_call;

	/*
	 * Deconfigure all I/Fs.
	 */
	nss_pptp_msg_init(&pptpmsg, inner_if, NSS_PPTP_MSG_SESSION_DECONFIGURE, sizeof(struct nss_pptp_session_deconfigure_msg), NULL, NULL);
	status = nss_pptp_tx_msg_sync(nss_pptp_get_context(), &pptpmsg);
	if (status != NSS_TX_SUCCESS) {
		nss_connmgr_pptp_warning("%p: pptp session destroy command failed, if_number = %d\n", dev, inner_if);
		goto fail;
	}

	nss_pptp_msg_init(&pptpmsg, outer_if, NSS_PPTP_MSG_SESSION_DECONFIGURE, sizeof(struct nss_pptp_session_deconfigure_msg), NULL, NULL);
	status = nss_pptp_tx_msg_sync(nss_pptp_get_context(), &pptpmsg);
	if (status != NSS_TX_SUCCESS) {
		nss_connmgr_pptp_warning("%p: pptp session destroy command failed, if_number = %d\n", dev, outer_if);
		goto fail;
	}

	nss_pptp_msg_init(&pptpmsg, host_inner_if, NSS_PPTP_MSG_SESSION_DECONFIGURE, sizeof(struct nss_pptp_session_deconfigure_msg), NULL, NULL);
	status = nss_pptp_tx_msg_sync(nss_pptp_get_context(), &pptpmsg);
	if (status != NSS_TX_SUCCESS) {
		nss_connmgr_pptp_warning("%p: pptp session destroy command failed, if_number = %d\n", dev, host_inner_if);
		goto fail;
	}

	/*
	 * Unregister all the I/Fs.
	 */
	nss_unregister_pptp_if(inner_if);
	nss_unregister_pptp_if(outer_if);
	nss_unregister_pptp_if(host_inner_if);

	/*
	 * Dealloc all the I/Fs.
	 */
	status = nss_dynamic_interface_dealloc_node(inner_if, NSS_DYNAMIC_INTERFACE_TYPE_PPTP_INNER);
	if (status != NSS_TX_SUCCESS) {
		nss_connmgr_pptp_warning("%p: pptp dealloc node failure for inner if_number=%d\n", dev, inner_if);
		goto fail;
	}

	status = nss_dynamic_interface_dealloc_node(outer_if, NSS_DYNAMIC_INTERFACE_TYPE_PPTP_OUTER);
	if (status != NSS_TX_SUCCESS) {
		nss_connmgr_pptp_warning("%p: pptp dealloc node failure for outer if_number=%d\n", dev, outer_if);
		goto fail;
	}

	status = nss_dynamic_interface_dealloc_node(host_inner_if, NSS_DYNAMIC_INTERFACE_TYPE_PPTP_HOST_INNER);
	if (status != NSS_TX_SUCCESS) {
		nss_connmgr_pptp_warning("%p: pptp dealloc node failure for host inner if_number=%d\n", dev, host_inner_if);
		goto fail;
	}

	nss_connmgr_pptp_info("%p: deleting pptpsession, if_number %d, local_call_id %d, peer_call_id %d\n", dev,
					dev->ifindex, session_info->data.src_call,  session_info->data.dst_call);

fail:
	kfree(session_info);
	return NOTIFY_DONE;

}

/*
 * nss_connmgr_pptp_dev_event()
 *	Net device notifier for nss pptp module
 */
static int nss_connmgr_pptp_dev_event(struct notifier_block  *nb,
		unsigned long event, void  *dev)
{
	struct net_device *netdev = netdev_notifier_info_to_dev(dev);

	switch (event) {
	case NETDEV_UP:
		nss_connmgr_pptp_info("%p: netdevice '%s' UP event\n", netdev, netdev->name);
		return nss_connmgr_pptp_dev_up(netdev);

	case NETDEV_DOWN:
		nss_connmgr_pptp_info("%p: netdevice '%s' Down event\n", netdev, netdev->name);
		return nss_connmgr_pptp_dev_down(netdev);

	default:
		break;
	}

	return NOTIFY_DONE;
}

/*
 * Linux Net device Notifier
 */
struct notifier_block nss_connmgr_pptp_notifier = {
	.notifier_call = nss_connmgr_pptp_dev_event,
};

/*
 * nss_connmgr_pptp_init_module()
 *	Tunnel pptp module init function
 */
int __init nss_connmgr_pptp_init_module(void)
{
#ifdef CONFIG_OF
	/*
	 * If the node is not compatible, don't do anything.
	 */
	if (!of_find_node_by_name(NULL, "nss-common")) {
		return 0;
	}
#endif
	register_netdevice_notifier(&nss_connmgr_pptp_notifier);
	pptp_register_gre_seq_offload_callback(nss_connmgr_pptp_client_xmit);
	return 0;
}

/*
 * nss_connmgr_pptp_exit_module
 *	Tunnel pptp module exit function
 */
void __exit nss_connmgr_pptp_exit_module(void)
{
	pptp_unregister_gre_seq_offload_callback();
#ifdef CONFIG_OF
	/*
	 * If the node is not compatible, don't do anything.
	 */
	if (!of_find_node_by_name(NULL, "nss-common")) {
		return;
	}
#endif
	unregister_netdevice_notifier(&nss_connmgr_pptp_notifier);
}

module_init(nss_connmgr_pptp_init_module);
module_exit(nss_connmgr_pptp_exit_module);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS pptp offload manager");
