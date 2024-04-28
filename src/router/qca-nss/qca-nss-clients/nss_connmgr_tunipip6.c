/*
 **************************************************************************
 * Copyright (c) 2014, 2017-2018, The Linux Foundation. All rights reserved.
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
 * nss_tunipip6.c
 *
 * This file is the NSS DS-lit and IPP6  tunnel module
 * ------------------------REVISION HISTORY-----------------------------
 * Qualcomm Atheros	    15/sep/2013		     Created
 */

#include <linux/version.h>
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/of.h>
#include <linux/tcp.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <net/ipv6.h>
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3,9,0))
#include <net/ipip.h>
#else
#include <net/ip_tunnels.h>
#endif
#include <net/ip6_tunnel.h>
#include <linux/if_arp.h>
#include <nss_api_if.h>

/*
 * NSS tunipip6 debug macros
 */
#if (NSS_TUNIPIP6_DEBUG_LEVEL < 1)
#define nss_tunipip6_assert(fmt, args...)
#else
#define nss_tunipip6_assert(c) if (!(c)) { BUG_ON(!(c)); }
#endif

#if defined(CONFIG_DYNAMIC_DEBUG)

/*
 * Compile messages for dynamic enable/disable
 */
#define nss_tunipip6_warning(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define nss_tunipip6_info(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define nss_tunipip6_trace(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#else

/*
 * Statically compile messages at different levels
 */
#if (NSS_TUNIPIP6_DEBUG_LEVEL < 2)
#define nss_tunipip6_warning(s, ...)
#else
#define nss_tunipip6_warning(s, ...) pr_warn("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_TUNIPIP6_DEBUG_LEVEL < 3)
#define nss_tunipip6_info(s, ...)
#else
#define nss_tunipip6_info(s, ...)   pr_notice("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_TUNIPIP6_DEBUG_LEVEL < 4)
#define nss_tunipip6_trace(s, ...)
#else
#define nss_tunipip6_trace(s, ...)  pr_info("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif
#endif

/*
 *  tunipip6 stats structure
 */
struct nss_tunipip6_stats {
	uint32_t rx_packets;	/* Number of received packets */
	uint32_t rx_bytes;	/* Number of received bytes */
	uint32_t tx_packets;	/* Number of transmitted packets */
	uint32_t tx_bytes;	/* Number of transmitted bytes */
};

/*
 * nss_tunipip6_encap_exception()
 *	Exception handler registered to NSS driver.
 *
 * This function is called when no rule is found for successful encapsulation.
 */
static void nss_tunipip6_encap_exception(struct net_device *dev, struct sk_buff *skb, __attribute__((unused)) struct napi_struct *napi)
{
	skb->dev = dev;
	nss_tunipip6_info("received - %d bytes name %s ver %x\n",
			skb->len, dev->name, (skb->data[0] >> 4));

	skb->protocol = htons(ETH_P_IP);
	skb_reset_network_header(skb);
	skb->pkt_type = PACKET_HOST;
	skb->skb_iif = dev->ifindex;
	skb->ip_summed = CHECKSUM_NONE;
	netif_receive_skb(skb);
}

/*
 * nss_tunipip6_decap_exception()
 *	Exception handler registered to NSS driver.
 *
 * This function is called when no rule is found for successful decapsulation.
 */
static void nss_tunipip6_decap_exception(struct net_device *dev, struct sk_buff *skb, __attribute__((unused)) struct napi_struct *napi)
{
	skb->dev = dev;
	nss_tunipip6_info("received - %d bytes name %s ver %x\n",
			skb->len, dev->name, (skb->data[0] >> 4));

	skb->protocol = htons(ETH_P_IPV6);
	skb_reset_network_header(skb);
	skb->pkt_type = PACKET_HOST;
	skb->skb_iif = dev->ifindex;
	skb->ip_summed = CHECKSUM_NONE;
	netif_receive_skb(skb);
}

/*
 *  nss_tunipip6_update_dev_stats
 *	Update the Dev stats received from NetAp
 */
static void nss_tunipip6_update_dev_stats(struct net_device *dev,
					struct nss_tunipip6_stats_sync_msg *sync_stats)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
	struct pcpu_sw_netstats stats;

	u64_stats_init(&stats.syncp);
	u64_stats_update_begin(&stats.syncp);
	u64_stats_add(&stats.rx_packets, sync_stats->node_stats.rx_packets);
	u64_stats_add(&stats.rx_bytes, sync_stats->node_stats.rx_bytes);
	u64_stats_add(&stats.tx_packets, sync_stats->node_stats.tx_packets);
	u64_stats_add(&stats.tx_bytes, sync_stats->node_stats.tx_bytes);
	u64_stats_update_end(&stats.syncp);
#else
	struct nss_tunipip6_stats stats;

	stats.rx_packets = sync_stats->node_stats.rx_packets;
	stats.rx_bytes = sync_stats->node_stats.rx_bytes;
	stats.tx_packets = sync_stats->node_stats.tx_packets;
	stats.tx_bytes = sync_stats->node_stats.tx_bytes;
#endif

	dev->stats.rx_dropped += nss_cmn_rx_dropped_sum(&sync_stats->node_stats);
	ip6_update_offload_stats(dev, (void *)&stats);

}

/*
 * nss_tunipip6_event_receive()
 *	Event Callback to receive events from NSS.
 */
void nss_tunipip6_event_receive(void *if_ctx, struct nss_tunipip6_msg *tnlmsg)
{
	struct net_device *netdev = NULL;
	netdev = (struct net_device *)if_ctx;

	switch (tnlmsg->cm.type) {
	case NSS_TUNIPIP6_RX_STATS_SYNC:
		nss_tunipip6_update_dev_stats(netdev, (struct nss_tunipip6_stats_sync_msg *)&tnlmsg->msg.stats_sync);
		break;

	default:
		nss_tunipip6_info("%s: Unknown Event from NSS", __func__);
		break;
	}
}

/*
 * nss_tunipip6_dev_up()
 *	IPIP6 Tunnel device i/f up handler
 */
int nss_tunipip6_dev_up(struct net_device *netdev)
{
	struct ip6_tnl *tunnel;
	struct nss_tunipip6_msg tnlmsg;
	struct nss_tunipip6_create_msg *tnlcfg;
	struct flowi6 *fl6;
	uint32_t fmr_number = 0;
	int inner_ifnum, outer_ifnum;
	uint32_t features = 0;
	nss_tx_status_t status;
	struct nss_ctx_instance *nss_ctx;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
	struct __ip6_tnl_fmr *fmr;
#endif

	/*
	 * Validate netdev for ipv6-in-ipv4  Tunnel
	 */
	if (netdev->type != ARPHRD_TUNNEL6 ) {
		return NOTIFY_DONE;
	}

	tunnel = (struct ip6_tnl *)netdev_priv(netdev);

	/*
	 * Find the Tunnel device flow information
	 */
	fl6 = &tunnel->fl.u.ip6;

	nss_tunipip6_trace("%p: Tunnel Param srcaddr %x:%x:%x:%x  daddr %x:%x:%x:%x\n", netdev,
			fl6->saddr.s6_addr32[0], fl6->saddr.s6_addr32[1],
			fl6->saddr.s6_addr32[2], fl6->saddr.s6_addr32[3],
			fl6->daddr.s6_addr32[0], fl6->daddr.s6_addr32[1],
			fl6->daddr.s6_addr32[2], fl6->daddr.s6_addr32[3] );
	nss_tunipip6_trace("%p: Hop limit %d\n", netdev, tunnel->parms.hop_limit);
	nss_tunipip6_trace("%p: Tunnel param flag %x  fl6.flowlabel %x\n", netdev,  tunnel->parms.flags, fl6->flowlabel);

	inner_ifnum = nss_dynamic_interface_alloc_node(NSS_DYNAMIC_INTERFACE_TYPE_TUNIPIP6_INNER);
	if (inner_ifnum < 0) {
		nss_tunipip6_warning("%p: Request interface number failed\n", netdev);
		goto inner_alloc_fail;
	}

	outer_ifnum = nss_dynamic_interface_alloc_node(NSS_DYNAMIC_INTERFACE_TYPE_TUNIPIP6_OUTER);
	if (outer_ifnum < 0) {
		nss_tunipip6_warning("%p: Request interface number failed\n", netdev);
		goto outer_alloc_fail;
	}

	/*
	 * Register ipip6 tunnel with NSS
	 */
	nss_ctx = nss_register_tunipip6_if(inner_ifnum,
					NSS_DYNAMIC_INTERFACE_TYPE_TUNIPIP6_INNER,
					nss_tunipip6_encap_exception,
					nss_tunipip6_event_receive,
					netdev,
					features);
	if (!nss_ctx) {
		nss_tunipip6_warning("%p: nss_register_tunipip6_if Failed\n", netdev);
		goto inner_reg_fail;
	}

	nss_ctx = nss_register_tunipip6_if(outer_ifnum,
					NSS_DYNAMIC_INTERFACE_TYPE_TUNIPIP6_OUTER,
					nss_tunipip6_decap_exception,
					nss_tunipip6_event_receive,
					netdev,
					features);
	if (!nss_ctx) {
		nss_tunipip6_warning("%p: nss_register_tunipip6_if Failed\n", netdev);
		goto outer_reg_fail;
	}

	nss_tunipip6_trace("%p: nss_register_tunipip6_if Success\n", netdev);

	/*
	 * Prepare The Tunnel configuration parameter to send to nss
	 */
	memset(&tnlmsg, 0, sizeof(struct nss_tunipip6_msg));
	tnlcfg = &tnlmsg.msg.tunipip6_create;

	tnlcfg->saddr[0] = ntohl(fl6->saddr.s6_addr32[0]);
	tnlcfg->saddr[1] = ntohl(fl6->saddr.s6_addr32[1]);
	tnlcfg->saddr[2] = ntohl(fl6->saddr.s6_addr32[2]);
	tnlcfg->saddr[3] = ntohl(fl6->saddr.s6_addr32[3]);
	tnlcfg->daddr[0] = ntohl(fl6->daddr.s6_addr32[0]);
	tnlcfg->daddr[1] = ntohl(fl6->daddr.s6_addr32[1]);
	tnlcfg->daddr[2] = ntohl(fl6->daddr.s6_addr32[2]);
	tnlcfg->daddr[3] = ntohl(fl6->daddr.s6_addr32[3]);
	tnlcfg->hop_limit = tunnel->parms.hop_limit;
	tnlcfg->flags = ntohl(tunnel->parms.flags);

	/*
	 * Flow Label In kernel is stored in big endian format.
	 */
	tnlcfg->flowlabel = fl6->flowlabel;
	tnlcfg->draft03 = tunnel->parms.draft03;

	/*
	 * Configure FMR table up to MAX_FMR_NUMBER, the rest will be forwarded to BR
	 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
	for (fmr = tunnel->parms.fmrs; fmr && fmr_number < NSS_TUNIPIP6_MAX_FMR_NUMBER; fmr = fmr->next, fmr_number++) {
		tnlcfg->fmr[fmr_number].ip6_prefix[0] = ntohl(fmr->ip6_prefix.s6_addr32[0]);
		tnlcfg->fmr[fmr_number].ip6_prefix[1] = ntohl(fmr->ip6_prefix.s6_addr32[1]);
		tnlcfg->fmr[fmr_number].ip6_prefix[2] = ntohl(fmr->ip6_prefix.s6_addr32[2]);
		tnlcfg->fmr[fmr_number].ip6_prefix[3] = ntohl(fmr->ip6_prefix.s6_addr32[3]);
		tnlcfg->fmr[fmr_number].ip4_prefix = ntohl(fmr->ip4_prefix.s_addr);
		tnlcfg->fmr[fmr_number].ip6_prefix_len = fmr->ip6_prefix_len;
		tnlcfg->fmr[fmr_number].ip4_prefix_len = fmr->ip4_prefix_len;
		tnlcfg->fmr[fmr_number].ea_len = fmr->ea_len;
		tnlcfg->fmr[fmr_number].offset = fmr->offset;
	}
#endif
	tnlcfg->fmr_number = fmr_number;

	/*
	 * Updating sibling_if_num for encap interface.
	 */
	tnlcfg->sibling_if_num = outer_ifnum;

	nss_tunipip6_trace("%p: Tunnel Param srcaddr %x:%x:%x:%x  daddr %x:%x:%x:%x\n", netdev,
			tnlcfg->saddr[0], tnlcfg->saddr[1],
			tnlcfg->saddr[2], tnlcfg->saddr[3],
			tnlcfg->daddr[0], tnlcfg->daddr[1],
			tnlcfg->daddr[2], tnlcfg->daddr[3] );

	/*
	 * Send configure message to encap interface.
	 */
	nss_tunipip6_msg_init(&tnlmsg, inner_ifnum, NSS_TUNIPIP6_TX_ENCAP_IF_CREATE,
			sizeof(struct nss_tunipip6_create_msg), NULL, NULL);

	nss_tunipip6_trace("%p: Sending IPIP6 tunnel i/f up command to NSS %p\n", netdev, nss_ctx);
	status = nss_tunipip6_tx(nss_ctx, &tnlmsg);
	if (status != NSS_TX_SUCCESS) {
		nss_tunipip6_warning("%p: Tunnel up command error %d\n", netdev, status);
		goto config_fail;
	}

	/*
	 * Updating sibling_if_num for decap interface.
	 */
	tnlcfg->sibling_if_num = inner_ifnum;

	/*
	 * Send configure message to decap interface.
	 */
	nss_tunipip6_msg_init(&tnlmsg, outer_ifnum, NSS_TUNIPIP6_TX_DECAP_IF_CREATE,
			sizeof(struct nss_tunipip6_create_msg), NULL, NULL);

	nss_tunipip6_trace("%p: Sending IPIP6 tunnel i/f up command to NSS %p\n", netdev, nss_ctx);
	status = nss_tunipip6_tx(nss_ctx, &tnlmsg);
	if (status != NSS_TX_SUCCESS) {
		nss_tunipip6_warning("%p: Tunnel up command error %d\n", netdev, status);
		goto config_fail;
	}
	return NOTIFY_DONE;

config_fail:
	nss_unregister_tunipip6_if(outer_ifnum);
outer_reg_fail:
	nss_unregister_tunipip6_if(inner_ifnum);
inner_reg_fail:
	status = nss_dynamic_interface_dealloc_node(outer_ifnum, NSS_DYNAMIC_INTERFACE_TYPE_TUNIPIP6_OUTER);
	if (status != NSS_TX_SUCCESS) {
		nss_tunipip6_warning("%p: Unable to dealloc the node[%d] in the NSS fw!\n", netdev, outer_ifnum);
	}
outer_alloc_fail:
	status = nss_dynamic_interface_dealloc_node(inner_ifnum, NSS_DYNAMIC_INTERFACE_TYPE_TUNIPIP6_INNER);
	if (status != NSS_TX_SUCCESS) {
		nss_tunipip6_warning("%p: Unable to dealloc the node[%d] in the NSS fw!\n", netdev, inner_ifnum);
	}
inner_alloc_fail:
	return NOTIFY_DONE;
}

/*
 * nss_tunipip6_dev_down()
 *	IPP6 Tunnel device i/f down handler
 */
int nss_tunipip6_dev_down(struct net_device *netdev)
{
	int inner_ifnum, outer_ifnum;
	nss_tx_status_t status;

	/*
	 * Validate netdev for ipv6-in-ipv4  Tunnel
	 */
	if (netdev->type != ARPHRD_TUNNEL6) {
		return NOTIFY_DONE;
	}

	/*
	 * Check if tunnel ipip6 is registered ?
	 */
	inner_ifnum = nss_cmn_get_interface_number_by_dev_and_type(netdev, NSS_DYNAMIC_INTERFACE_TYPE_TUNIPIP6_INNER);
	if (inner_ifnum < 0) {
		nss_tunipip6_warning("%p: Net device is not registered with nss\n", netdev);
		return NOTIFY_DONE;
	}

	outer_ifnum = nss_cmn_get_interface_number_by_dev_and_type(netdev, NSS_DYNAMIC_INTERFACE_TYPE_TUNIPIP6_OUTER);
	if (outer_ifnum < 0) {
		nss_tunipip6_warning("%p: Net device is not registered with nss\n", netdev);
		return NOTIFY_DONE;
	}

	/*
	 * Un-Register IPIP6 tunnel with NSS
	 */
	nss_unregister_tunipip6_if(inner_ifnum);
	nss_unregister_tunipip6_if(outer_ifnum);

	status = nss_dynamic_interface_dealloc_node(inner_ifnum, NSS_DYNAMIC_INTERFACE_TYPE_TUNIPIP6_INNER);
	if (status != NSS_TX_SUCCESS) {
		nss_tunipip6_warning("%p: Dealloc node failure\n", netdev);
		return NOTIFY_DONE;
	}

	status = nss_dynamic_interface_dealloc_node(outer_ifnum, NSS_DYNAMIC_INTERFACE_TYPE_TUNIPIP6_OUTER);
	if (status != NSS_TX_SUCCESS) {
		nss_tunipip6_warning("%p: Dealloc node failure\n", netdev);
		return NOTIFY_DONE;
	}

	return NOTIFY_DONE;
}

/*
 * nss_tunipip6_dev_event()
 *	Net device notifier for ipip6 module
 */
static int nss_tunipip6_dev_event(struct notifier_block  *nb,
		unsigned long event, void  *dev)
{
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3, 10, 0))
	struct net_device *netdev = (struct net_device *)dev;
#else
	struct net_device *netdev = netdev_notifier_info_to_dev(dev);
#endif

	switch (event) {
	case NETDEV_UP:
		nss_tunipip6_trace("%p: NETDEV_UP :event %lu name %s\n", netdev, event, netdev->name);
		return nss_tunipip6_dev_up(netdev);

	case NETDEV_DOWN:
		nss_tunipip6_trace("%p: NETDEV_DOWN :event %lu name %s\n", netdev, event, netdev->name);
		return nss_tunipip6_dev_down(netdev);

	default:
		nss_tunipip6_trace("%p: Unhandled notifier dev %s event %x\n", netdev, netdev->name, (int)event);
		break;
	}

	return NOTIFY_DONE;
}

/*
 * Linux Net device Notifier
 */
struct notifier_block nss_tunipip6_notifier = {
	.notifier_call = nss_tunipip6_dev_event,
};

/*
 * nss_tunipip6_init_module()
 *	Tunnel ipip6 module init function
 */
int __init nss_tunipip6_init_module(void)
{
#ifdef CONFIG_OF
	/*
	 * If the node is not compatible, don't do anything.
	 */
	if (!of_find_node_by_name(NULL, "nss-common")) {
		return 0;
	}
#endif
	nss_tunipip6_info("module (platform - IPQ806x , %s) loaded\n",
			  NSS_CLIENT_BUILD_ID);

	register_netdevice_notifier(&nss_tunipip6_notifier);
	nss_tunipip6_trace("Netdev Notifier registerd\n");


	return 0;
}

/*
 * nss_tunipip6_exit_module()
 *	Tunnel ipip6 module exit function
 */
void __exit nss_tunipip6_exit_module(void)
{
#ifdef CONFIG_OF

	/*
	 * If the node is not compatible, don't do anything.
	 */
	if (!of_find_node_by_name(NULL, "nss-common")) {
		return;
	}
#endif

	unregister_netdevice_notifier(&nss_tunipip6_notifier);
	nss_tunipip6_info("module unloaded\n");
}

module_init(nss_tunipip6_init_module);
module_exit(nss_tunipip6_exit_module);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS tunipip6 offload manager");
