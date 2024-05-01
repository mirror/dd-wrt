 /*
 **************************************************************************
 * Copyright (c) 2014, 2017-2018, 2020 The Linux Foundation. All rights reserved.

 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.

 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

#include <linux/version.h>
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/of.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <net/ip.h>
#include <net/ipv6.h>
#include <linux/if.h>
#include <net/ip_tunnels.h>
#include <net/ip6_tunnel.h>
#include <nss_api_if.h>
#include "nss_connmgr_tunipip6.h"
#include "nss_connmgr_tunipip6_sysctl.h"
#include "nss_connmgr_tunipip6_priv.h"

#define NSS_TUNIPIP6_MAX_FMR 255	/* Maximum number of forward mapping rule (FMR). */

/*
 * Frag Id update is disabled by default
 */
bool frag_id_update = false;
/*
 * Creating custom ipip6 interface is disabled by default.
 */
static bool enable_custom;
module_param(enable_custom, bool, 0);

/*
 * tunipip6 global context.
 */
struct nss_tunipip6_context tunipip6_ctx;

/*
 * nss_tunipip6_alloc_instance()
 * 	Allocate tunipip6 interface instance.
 */
static struct nss_tunipip6_instance *nss_tunipip6_alloc_instance(struct net_device *dev,
					int inner_ifnum,
					int outer_ifnum)
{
	struct nss_tunipip6_instance*ntii;

	ntii = vzalloc(sizeof(*ntii));
	if (!ntii) {
		nss_tunipip6_warning("%px: Not able to allocate tunipip6 instance\n", dev);
		return NULL;
	}

	ntii->dev = dev;

	/*
	 * Create statistics dentry.
	 */
	if (!nss_tunipip6_stats_dentry_create(ntii)) {
		vfree(ntii);
		nss_tunipip6_warning("%px: Not able to create tunipip6 statistics dentry\n", dev);
		return NULL;
	}

	INIT_LIST_HEAD(&ntii->list);
	ntii->inner_ifnum = inner_ifnum;
	ntii->outer_ifnum = outer_ifnum;
	dev_hold(dev);
	return ntii;
}

/*
 * nss_tunipip6_free_instance()
 * 	Delete the tunipip6 interface instance from the list and free it.
 *
 * Note: tunnel list lock is expected to be held by the caller.
 */
static void nss_tunipip6_free_instance(struct nss_tunipip6_instance *ntii)
{
	if (!list_empty(&ntii->list)) {
		list_del(&ntii->list);
	}

	vfree(ntii);
}

/*
 * nss_tunipip6_find_instance()
 * 	Find tunipip6 interface instance from list.
 *
 * Note: tunnel list lock is expected to be held by the caller.
 */
struct nss_tunipip6_instance *nss_tunipip6_find_instance(struct net_device *dev)
{
	struct nss_tunipip6_instance *ntii;

	/*
	 * Check if dev instance is in the list
	 */
	list_for_each_entry(ntii, &tunipip6_ctx.dev_list, list) {
		if (ntii->dev == dev) {
			return ntii;
		}
	}

	return NULL;
}

/*
 * nss_tunipip6_find_and_free_instance()
 * 	Find and free the tunipip6 instance.
 */
static enum nss_connmgr_tunipip6_err_codes nss_tunipip6_find_and_free_instance(struct net_device *netdev)
{
	struct dentry *dentry;
	struct nss_tunipip6_instance *ntii;

	spin_lock_bh(&tunipip6_ctx.lock);
	ntii = nss_tunipip6_find_instance(netdev);
	if (!ntii) {
		spin_unlock_bh(&tunipip6_ctx.lock);
		nss_tunipip6_warning("%px: Not able to find tunipip6 instance for dev:%s\n", netdev, netdev->name);
		return NSS_CONNMGR_TUNIPIP6_CONTEXT_FAILURE;
	}

	dentry = ntii->dentry;
	nss_tunipip6_free_instance(ntii);
	spin_unlock_bh(&tunipip6_ctx.lock);
	debugfs_remove(dentry);
	dev_put(netdev);
	return NSS_CONNMGR_TUNIPIP6_SUCCESS;
}

/*
 * nss_tunipip6_encap_exception()
 *	Exception handler registered to NSS driver.
 *
 * This function is called when no rule is found for successful encapsulation.
 */
static void nss_tunipip6_encap_exception(struct net_device *dev, struct sk_buff *skb, __attribute__((unused)) struct napi_struct *napi)
{
	const struct net_device_ops *ops = dev->netdev_ops;
	struct netdev_queue *queue;
	struct iphdr *iph;
	struct rtable *rt;
	int cpu;

	if (unlikely(!pskb_may_pull(skb, sizeof(struct iphdr)))) {
		nss_tunipip6_warning("%px: skb: %px, pskb_may_pull failed to linearize iphdr, packet does not have a proper IPv4 header.", dev, skb);
		dev_kfree_skb_any(skb);
		return;
	}

	skb_reset_network_header(skb);

	iph = ip_hdr(skb);
	nss_tunipip6_assert(iph->version == IPVERSION);

	nss_tunipip6_info("%px: received - %d bytes name %s ver %x\n",
			skb, skb->len, dev->name, iph->version);

	rt = ip_route_output(&init_net, iph->daddr, 0, 0, 0);
	if (unlikely(IS_ERR(rt))) {
		nss_tunipip6_info("%px: Failed to find IPv4 route for dest %pI4 src %pI4\n", skb, &iph->daddr, &iph->saddr);
		dev_kfree_skb_any(skb);
		return;
	}

	nss_tunipip6_trace("%px: Route look up successful for dest_ip: %pI4 src_ip: %pI4 dest dev:%s\n",
			skb, &iph->daddr, &iph->saddr, rt->dst.dev->name);

	/*
	 * Send decap direction packets to stack using netif_receive_skb
	 * NOTE: if tunnel to tunnel traffic is enabled, then this might fail.
	 */
	if (rt->dst.dev->type != ARPHRD_TUNNEL6) {
		skb->dev = dev;
		skb->protocol = htons(ETH_P_IP);
		skb->pkt_type = PACKET_HOST;
		skb->skb_iif = dev->ifindex;
		skb->ip_summed = CHECKSUM_NONE;
		netif_receive_skb(skb);
		return;
	}

	/*
	 * Send encap direction packets to tunnel xmit using ndo_start_xmit
	 */
	skb_dst_drop(skb);
	skb_dst_set(skb, &rt->dst);

	/*
	 * Set ignore df bit to fragment the packet in kernel.
	 */
	if (!(iph->frag_off & htons(IP_DF))) {
		skb->ignore_df = true;
	}

	skb->protocol = htons(ETH_P_IP);
	skb->pkt_type = PACKET_HOST;
	skb->skb_iif = dev->ifindex;
	skb->dev = dev;
	skb->ip_summed = CHECKSUM_NONE;

	/*
	 * This is needed to acquire HARD_TX_LOCK.
	 */
	cpu = smp_processor_id();
	queue = skb_get_tx_queue(dev, skb);

	nss_tunipip6_trace("%px: skb queue mapping: %d, cpu: %d", skb, skb_get_queue_mapping(skb), cpu);

	/*
	 * Take HARD_TX_LOCK to be in sync with the kernel.
	 */
	HARD_TX_LOCK(dev, queue, cpu);

	/*
	 * Check if queue is alive
	 */
	if (unlikely(netif_xmit_frozen_or_stopped(queue))) {
		HARD_TX_UNLOCK(dev, queue);
		nss_tunipip6_trace("%px: Dropping the packet, as queue: %px is not alive", skb, queue);
		skb_dst_drop(skb);
		dev_kfree_skb_any(skb);
		return;
	}
	ops->ndo_start_xmit(skb, dev);
	HARD_TX_UNLOCK(dev, queue);
}

/*
 * nss_tunipip6_decap_exception()
 *	Exception handler registered to NSS driver.
 *
 * Exception handler registered to NSS for handling tunipip6 ipv6 pkts.
 */
static void nss_tunipip6_decap_exception(struct net_device *dev, struct sk_buff *skb, __attribute__((unused)) struct napi_struct *napi)
{
	const struct net_device_ops *ops = dev->netdev_ops;
	struct netdev_queue *queue;
	struct iphdr *iph;
	struct rtable *rt;
	int cpu;
	int8_t ver = skb->data[0] >> 4;

	nss_tunipip6_trace("%px: received - %d bytes name %s ver %x\n",
			dev, skb->len, dev->name, ver);

	nss_tunipip6_assert(ver == 6);

	if (unlikely(!pskb_may_pull(skb, sizeof(struct ipv6hdr)))) {
		nss_tunipip6_warning("%px: pskb_may_pull failed to pull ipv6 header", dev);
		dev_kfree_skb_any(skb);
		return;
	}

	skb_pull(skb, sizeof(struct ipv6hdr));

	if (unlikely(!pskb_may_pull(skb, sizeof(struct iphdr)))) {
		nss_tunipip6_warning("%px: pskb_may_pull failed to linearize iphdr, packet does not have a proper IPv4 header.", dev);
		dev_kfree_skb_any(skb);
		return;
	}

	skb_reset_network_header(skb);

	iph = ip_hdr(skb);
	nss_tunipip6_assert(iph->version == 4);

	rt = ip_route_output(&init_net, iph->daddr, 0, 0, 0);
	if (unlikely(IS_ERR(rt))) {
		nss_tunipip6_info("%px: Failed to find IPv4 route for %pI4\n", skb, &iph->daddr);
		dev_kfree_skb_any(skb);
		return;
	}

	nss_tunipip6_trace("%px: Route look up successful for dest_ip: %pI4 src_ip: %pI4\n",
			skb, &iph->daddr, &iph->saddr);

	skb_dst_drop(skb);
	skb_dst_set(skb, &rt->dst);

	skb_reset_transport_header(skb);

	/*
	 * Set ignore df bit to fragment the packet in kernel.
	 */
	if (!(iph->frag_off & htons(IP_DF))) {
		skb->ignore_df = true;
	}

	skb->protocol = htons(ETH_P_IP);
	skb->pkt_type = PACKET_HOST;
	skb->skb_iif = dev->ifindex;
	skb->dev = dev;
	skb->ip_summed = CHECKSUM_NONE;

	/*
	 * This is needed to acquire HARD_TX_LOCK.
	 */
	cpu = smp_processor_id();
	queue = skb_get_tx_queue(dev, skb);

	nss_tunipip6_trace("%px: skb queue mapping: %d, cpu: %d", skb, skb_get_queue_mapping(skb), cpu);

	/*
	 * Take HARD_TX_LOCK to be in sync with the kernel.
	 */
	HARD_TX_LOCK(dev, queue, cpu);

	/*
	 * Check if queue is alive
	 */
	if (unlikely(netif_xmit_frozen_or_stopped(queue))) {
		HARD_TX_UNLOCK(dev, queue);
		nss_tunipip6_trace("%px: Dropping the packet, as queue: %px is not alive", skb, queue);
		skb_dst_drop(skb);
		dev_kfree_skb_any(skb);
		return;
	}
	ops->ndo_start_xmit(skb, dev);
	HARD_TX_UNLOCK(dev, queue);
}

/*
 *  nss_tunipip6_update_dev_stats
 *	Update the Dev stats received from NSS
 */
static void nss_tunipip6_update_dev_stats(struct net_device *dev,
					struct nss_tunipip6_msg *tnlmsg)
{
	struct pcpu_sw_netstats stats;
	enum nss_dynamic_interface_type interface_type;
	struct nss_tunipip6_stats_sync_msg *sync_stats = (struct nss_tunipip6_stats_sync_msg *)&tnlmsg->msg.stats_sync;

	interface_type = nss_dynamic_interface_get_type(nss_tunipip6_get_context(), tnlmsg->cm.interface);

	memset(&stats, 0, sizeof(stats));
	if (interface_type == NSS_DYNAMIC_INTERFACE_TYPE_TUNIPIP6_INNER) {
		stats.tx_packets = sync_stats->node_stats.tx_packets;
		stats.tx_bytes = sync_stats->node_stats.tx_bytes;
	} else if (interface_type == NSS_DYNAMIC_INTERFACE_TYPE_TUNIPIP6_OUTER) {
		stats.rx_packets = sync_stats->node_stats.rx_packets;
		stats.rx_bytes = sync_stats->node_stats.rx_bytes;
	} else {
		nss_tunipip6_warning("%px: Invalid interface type received from NSS\n", dev);
		return;
	}

	dev->stats.rx_dropped += nss_cmn_rx_dropped_sum(&sync_stats->node_stats);

	/* TODO: Update rx_dropped stats in ip6_update_offload_stats() */
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
		/*
		 * Update netdevice statistics.
		 */
		nss_tunipip6_update_dev_stats(netdev, tnlmsg);

		/*
		 * Update NSS statistics for tunipip6.
		 */
		nss_tunipip6_stats_sync(netdev, tnlmsg);
		break;

	default:
		nss_tunipip6_info("%px: Unknown Event from NSS\n", netdev);
		break;
	}
}

/*
 * _nss_tunipip6_dyn_interface_destroy()
 * 	Destroy NSS dynamic interface.
 */
enum nss_connmgr_tunipip6_err_codes _nss_tunipip6_dyn_interface_destroy(struct net_device *netdev)
{
	int inner_ifnum, outer_ifnum;
	nss_tx_status_t status;

	/*
	 * Check if tunnel ipip6 is registered ?
	 */
	inner_ifnum = nss_cmn_get_interface_number_by_dev_and_type(netdev, NSS_DYNAMIC_INTERFACE_TYPE_TUNIPIP6_INNER);
	if (inner_ifnum < 0) {
		nss_tunipip6_warning("%px: Net device is not registered with nss inner node\n", netdev);
		return NSS_CONNMGR_TUNIPIP6_NO_DEV;
	}

	status = nss_dynamic_interface_dealloc_node(inner_ifnum, NSS_DYNAMIC_INTERFACE_TYPE_TUNIPIP6_INNER);
	if (status != NSS_TX_SUCCESS) {
		nss_tunipip6_warning("%px: Dealloc inneer node failure\n", netdev);
		return NSS_CONNMGR_TUNIPIP6_TUN_DESTROY_FAILURE;
	}

	/*
	 * Un-Register IPIP6 tunnel with NSS
	 */
	nss_unregister_tunipip6_if(inner_ifnum);

	outer_ifnum = nss_cmn_get_interface_number_by_dev_and_type(netdev, NSS_DYNAMIC_INTERFACE_TYPE_TUNIPIP6_OUTER);
	if (outer_ifnum < 0) {
		nss_tunipip6_warning("%px: Net device is not registered with nss outer node\n", netdev);
		return NSS_CONNMGR_TUNIPIP6_NO_DEV;
	}

	status = nss_dynamic_interface_dealloc_node(outer_ifnum, NSS_DYNAMIC_INTERFACE_TYPE_TUNIPIP6_OUTER);
	if (status != NSS_TX_SUCCESS) {
		nss_tunipip6_warning("%px: Dealloc outer node failure\n", netdev);
		return NSS_CONNMGR_TUNIPIP6_TUN_DESTROY_FAILURE;
	}

	/*
	 * Un-Register IPIP6 tunnel with NSS
	 */
	nss_unregister_tunipip6_if(outer_ifnum);

	return NSS_CONNMGR_TUNIPIP6_SUCCESS;
}

/*
 * nss_tunipip6_dev_parse_param()
 *	IPIP6 standard Tunnel device i/f up handler
 */
static void nss_tunipip6_dev_parse_param(struct net_device *netdev, struct nss_connmgr_tunipip6_tunnel_cfg *tnlcfg)
{
	struct ip6_tnl *tunnel;
	struct flowi6 *fl6;

	tunnel = (struct ip6_tnl *)netdev_priv(netdev);

	/*
	 * Find the Tunnel device flow information
	 */
	fl6 = &tunnel->fl.u.ip6;

	nss_tunipip6_trace("%px: Tunnel param saddr: %pI6 daddr: %pI6\n", netdev, fl6->saddr.s6_addr32, fl6->daddr.s6_addr32);
	nss_tunipip6_trace("%px: Hop limit %d\n", netdev, tunnel->parms.hop_limit);
	nss_tunipip6_trace("%px: Tunnel param flag %x  fl6.flowlabel %x\n", netdev,  tunnel->parms.flags, fl6->flowlabel);
	nss_tunipip6_trace("%px: Tunnel frag id update is %d\n", netdev, frag_id_update);

	/*
	 * Prepare The Tunnel configuration parameter to send to nss
	 */
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
	tnlcfg->ttl_inherit = false;
	tnlcfg->tos_inherit = true;
	tnlcfg->frag_id_update = frag_id_update;
	tnlcfg->fmr_max = NSS_TUNIPIP6_MAX_FMR;
	/*
	 * Flow Label In kernel is stored in big endian format.
	 */
	tnlcfg->flowlabel = fl6->flowlabel;

	/*
	 * The tunnel_type for ds-lite and map-e should be same.
	 */
	tnlcfg->tunnel_type = NSS_CONNMGR_TUNIPIP6_TUNNEL_MAPE;

#if IS_ENABLED(CONFIG_MAP_E_SUPPORT)
#ifdef DRAFT03_SUPPORT
	/*
	 * Set "tunnel_type" based on draft03.
	 */
	if (tunnel->parms.draft03) {
		tnlcfg->tunnel_type = NSS_CONNMGR_TUNIPIP6_TUNNEL_MAPE_DRAFT03;
	}
#endif
#endif
}

#if IS_ENABLED(CONFIG_MAP_E_SUPPORT)
/*
 * nss_connmgr_tunipip6_configure_fmr()
 * 	Add FMR in NSS (if present).
 */
static void nss_connmgr_tunipip6_configure_fmr(struct net_device *netdev)
{
	struct ip6_tnl *tunnel;
	struct __ip6_tnl_fmr *fmr;
	uint32_t fmr_number = 0;
	enum nss_connmgr_tunipip6_err_codes status;
	struct nss_connmgr_tunipip6_maprule_cfg mrcfg = {0};

	tunnel = (struct ip6_tnl *)netdev_priv(netdev);

	/*
	 * Configure FMR table up to NSS_TUNIPIP6_MAX_FMR, the rest will be forwarded to BR
	 */
	for (fmr = tunnel->parms.fmrs; fmr && fmr_number < NSS_TUNIPIP6_MAX_FMR; fmr = fmr->next, fmr_number++) {
		/*
		 * Prepare "rulecfg"
		 */
		mrcfg.rule_type = NSS_CONNMGR_TUNIPIP6_RULE_FMR;
		mrcfg.ipv6_prefix[0] = ntohl(fmr->ip6_prefix.s6_addr32[0]);
		mrcfg.ipv6_prefix[1] = ntohl(fmr->ip6_prefix.s6_addr32[1]);
		mrcfg.ipv6_prefix[2] = ntohl(fmr->ip6_prefix.s6_addr32[2]);
		mrcfg.ipv6_prefix[3] = ntohl(fmr->ip6_prefix.s6_addr32[3]);
		mrcfg.ipv4_prefix = ntohl(fmr->ip4_prefix.s_addr);
		mrcfg.ipv6_prefix_len = fmr->ip6_prefix_len;
		mrcfg.ipv4_prefix_len = fmr->ip4_prefix_len;
		mrcfg.ea_len = fmr->ea_len;
		mrcfg.psid_offset = fmr->offset;

		/*
		 * Call add maprule API.
		 */
		status = nss_connmgr_tunipip6_add_maprule(netdev, &mrcfg);
		if (status != NSS_CONNMGR_TUNIPIP6_SUCCESS) {
			nss_tunipip6_trace("%px: Not able to add FMR rule. IPv6 Prefix: %pI6 IPv6 Prefix Lenght: %d\n"
					"IPv4 Prefix: %pI6 IPv4 Prefix Lenght: %d EA Length: %d PSID Offset: %d\n",
					netdev, mrcfg.ipv6_prefix, mrcfg.ipv6_prefix_len,&mrcfg.ipv4_prefix,
					mrcfg.ipv4_prefix_len, mrcfg.ea_len, mrcfg.psid_offset);
		}
	}
}
#endif

/*
 * nss_connmgr_tunipip6_create_interface()
 * 	Exported API to create and configure NSS nodes.
 */
enum nss_connmgr_tunipip6_err_codes nss_connmgr_tunipip6_create_interface(struct net_device *netdev, struct nss_connmgr_tunipip6_tunnel_cfg *tnlcfg)
{
	struct nss_tunipip6_msg tnlmsg;
	struct nss_tunipip6_create_msg *tnlcreate;
	struct nss_ctx_instance *nss_ctx;
	int inner_ifnum, outer_ifnum;
	uint32_t features = 0;
	nss_tx_status_t status;
	struct nss_tunipip6_instance *ntii;

#if IS_ENABLED(CONFIG_MAP_E_SUPPORT)
#ifndef DRAFT03_SUPPORT
	if ((tnlcfg->tunnel_type == NSS_CONNMGR_TUNIPIP6_TUNNEL_4RD) ||
		tnlcfg->tunnel_type == NSS_CONNMGR_TUNIPIP6_TUNNEL_MAPE_DRAFT03) {
		nss_tunipip6_warning("Draftf03 is not supported\n");
		return NSS_CONNMGR_TUNIPIP6_TUN_CREATE_FAILURE;
	}
#endif
#endif
	/*
	 * For standard tunnel, netdev and tnlcfg validation is done
	 * inside nss_tunipip6_dev_parse_param().
	 */
	if (!enable_custom) {
		goto configure_tunnel;
	}

	if (!netdev || !tnlcfg) {
		nss_tunipip6_warning("Invalid parametes passed\n");
		return NSS_CONNMGR_TUNIPIP6_INVALID_PARAM;
	}

	/*
	 * Validate netdev for ipv6-in-ipv4  Tunnel
	 */
	if (netdev->type != ARPHRD_TUNNEL6) {
		nss_tunipip6_warning("%px: Invalid netdevice type: %d\n", netdev, netdev->type);
		return NSS_CONNMGR_TUNIPIP6_NETDEV_TYPE_FAILURE;
	}

configure_tunnel:
	if ((tnlcfg->tunnel_type != NSS_CONNMGR_TUNIPIP6_TUNNEL_4RD) &&
		(tnlcfg->tunnel_type != NSS_CONNMGR_TUNIPIP6_TUNNEL_MAPE) &&
		(tnlcfg->tunnel_type != NSS_CONNMGR_TUNIPIP6_TUNNEL_MAPE_DRAFT03)) {
		nss_tunipip6_warning("%px: Invalid tunnel type: %d\n", netdev, tnlcfg->tunnel_type);
		return NSS_CONNMGR_TUNIPIP6_TUN_NONE;
	}

	/*
	 * Prepare The Tunnel configuration parameter to send to nss
	 */
	memset(&tnlmsg, 0, sizeof(struct nss_tunipip6_msg));
	tnlcreate = &tnlmsg.msg.tunipip6_create;

	tnlcreate->saddr[0] = tnlcfg->saddr[0];
	tnlcreate->saddr[1] = tnlcfg->saddr[1];
	tnlcreate->saddr[2] = tnlcfg->saddr[2];
	tnlcreate->saddr[3] = tnlcfg->saddr[3];
	tnlcreate->daddr[0] = tnlcfg->daddr[0];
	tnlcreate->daddr[1] = tnlcfg->daddr[1];
	tnlcreate->daddr[2] = tnlcfg->daddr[2];
	tnlcreate->daddr[3] = tnlcfg->daddr[3];
	tnlcreate->hop_limit = tnlcfg->hop_limit;
	tnlcreate->flags = tnlcfg->flags;
	tnlcreate->flowlabel = tnlcfg->flowlabel;
	tnlcreate->ttl_inherit = tnlcfg->ttl_inherit;
	tnlcreate->tos_inherit = tnlcfg->tos_inherit;
	tnlcreate->frag_id_update = tnlcfg->frag_id_update;

	/*
	 * Set "draft03" based on "tunnel_type". draft03 should be
	 * set to 0 for MAPE.
	 */
	if ((tnlcfg->tunnel_type == NSS_CONNMGR_TUNIPIP6_TUNNEL_4RD) ||
			(tnlcfg->tunnel_type == NSS_CONNMGR_TUNIPIP6_TUNNEL_MAPE_DRAFT03)) {
		tnlcreate->draft03 = 1;
	}

	inner_ifnum = nss_dynamic_interface_alloc_node(NSS_DYNAMIC_INTERFACE_TYPE_TUNIPIP6_INNER);
	if (inner_ifnum < 0) {
		nss_tunipip6_warning("%px: Request interface number failed\n", netdev);
		goto inner_alloc_fail;
	}

	outer_ifnum = nss_dynamic_interface_alloc_node(NSS_DYNAMIC_INTERFACE_TYPE_TUNIPIP6_OUTER);
	if (outer_ifnum < 0) {
		nss_tunipip6_warning("%px: Request interface number failed\n", netdev);
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
		nss_tunipip6_warning("%px: nss_register_tunipip6_if Failed\n", netdev);
		goto inner_reg_fail;
	}

	nss_ctx = nss_register_tunipip6_if(outer_ifnum,
					NSS_DYNAMIC_INTERFACE_TYPE_TUNIPIP6_OUTER,
					nss_tunipip6_decap_exception,
					nss_tunipip6_event_receive,
					netdev,
					features);
	if (!nss_ctx) {
		nss_tunipip6_warning("%px: nss_register_tunipip6_if Failed\n", netdev);
		goto outer_reg_fail;
	}

	nss_tunipip6_trace("%px: nss_register_tunipip6_if Success\n", netdev);

	/*
	 * Updating sibling_if_num for encap interface.
	 */
	tnlcreate->sibling_if_num = outer_ifnum;

	nss_tunipip6_trace("%px: Tunnel Param srcaddr %x:%x:%x:%x  daddr %x:%x:%x:%x\n", netdev,
			tnlcreate->saddr[0], tnlcreate->saddr[1],
			tnlcreate->saddr[2], tnlcreate->saddr[3],
			tnlcreate->daddr[0], tnlcreate->daddr[1],
			tnlcreate->daddr[2], tnlcreate->daddr[3]);

	/*
	 * Send configure message to encap interface.
	 */
	nss_tunipip6_msg_init(&tnlmsg, inner_ifnum, NSS_TUNIPIP6_TX_ENCAP_IF_CREATE,
			sizeof(struct nss_tunipip6_create_msg), NULL, NULL);

	nss_tunipip6_trace("%px: Sending IPIP6 tunnel i/f up command to NSS %px\n", netdev, nss_ctx);
	status = nss_tunipip6_tx_sync(nss_ctx, &tnlmsg);
	if (status != NSS_TX_SUCCESS) {
		nss_tunipip6_warning("%px: Tunnel up command error %d\n", netdev, status);
		goto context_alloc_fail;
	}

	/*
	 * Updating sibling_if_num for decap interface.
	 */
	tnlcreate->sibling_if_num = inner_ifnum;

	/*
	 * Send configure message to decap interface.
	 */
	nss_tunipip6_msg_init(&tnlmsg, outer_ifnum, NSS_TUNIPIP6_TX_DECAP_IF_CREATE,
			sizeof(struct nss_tunipip6_create_msg), NULL, NULL);

	nss_tunipip6_trace("%px: Sending IPIP6 tunnel i/f up command to NSS %px\n", netdev, nss_ctx);
	status = nss_tunipip6_tx_sync(nss_ctx, &tnlmsg);
	if (status != NSS_TX_SUCCESS) {
		nss_tunipip6_warning("%px: Tunnel up command error %d\n", netdev, status);
		goto context_alloc_fail;
	}

	/*
	 * Initialize tunipip6 instance.
	 */
	ntii = nss_tunipip6_alloc_instance(netdev, inner_ifnum, outer_ifnum);
	if (!ntii) {
		nss_tunipip6_warning("%px: Not able to create tunipip6 instance\n", netdev);
		goto context_alloc_fail;
	}

	/*
	 * Add the new tunipip6 instance to the global list.
	 */
	spin_lock_bh(&tunipip6_ctx.lock);
	list_add(&ntii->list, &tunipip6_ctx.dev_list);
	spin_unlock_bh(&tunipip6_ctx.lock);

	return NSS_CONNMGR_TUNIPIP6_SUCCESS;

context_alloc_fail:
	nss_unregister_tunipip6_if(outer_ifnum);
outer_reg_fail:
	nss_unregister_tunipip6_if(inner_ifnum);
inner_reg_fail:
	status = nss_dynamic_interface_dealloc_node(outer_ifnum, NSS_DYNAMIC_INTERFACE_TYPE_TUNIPIP6_OUTER);
	if (status != NSS_TX_SUCCESS) {
		nss_tunipip6_warning("%px: Unable to dealloc the node[%d] in the NSS fw!\n", netdev, outer_ifnum);
	}
outer_alloc_fail:
	status = nss_dynamic_interface_dealloc_node(inner_ifnum, NSS_DYNAMIC_INTERFACE_TYPE_TUNIPIP6_INNER);
	if (status != NSS_TX_SUCCESS) {
		nss_tunipip6_warning("%px: Unable to dealloc the node[%d] in the NSS fw!\n", netdev, inner_ifnum);
	}
inner_alloc_fail:

	return NSS_CONNMGR_TUNIPIP6_TUN_CREATE_FAILURE;
}
EXPORT_SYMBOL(nss_connmgr_tunipip6_create_interface);

/*
 * nss_connmgr_tunipip6_destroy_interface()
 * 	Exported API to destroy NSS dynamic interface.
 */
enum nss_connmgr_tunipip6_err_codes nss_connmgr_tunipip6_destroy_interface(struct net_device *netdev)
{
	enum nss_connmgr_tunipip6_err_codes ret;

	/*
	 * Validate netdev for ipv6-in-ipv4  Tunnel
	 */
	if (netdev->type != ARPHRD_TUNNEL6) {
		return NSS_CONNMGR_TUNIPIP6_NETDEV_TYPE_FAILURE;
	}

	/*
	 * Destroy tunipip6 NSS context.
	 */
	ret = _nss_tunipip6_dyn_interface_destroy(netdev);
	if (ret != NSS_CONNMGR_TUNIPIP6_SUCCESS) {
		nss_tunipip6_warning("%px: Not able to destroy NSS context. Err: %d\n", netdev, ret);
		return ret;
	}

	/*
	 * Find and free the tunipip6 instance.
	 */
	ret = nss_tunipip6_find_and_free_instance(netdev);
	return ret;
}
EXPORT_SYMBOL(nss_connmgr_tunipip6_destroy_interface);

/*
 * nss_connmgr_tunipip6_add_maprule()
 * 	Add new BMR/FMR entry.
 */
enum nss_connmgr_tunipip6_err_codes nss_connmgr_tunipip6_add_maprule(struct net_device *netdev, struct nss_connmgr_tunipip6_maprule_cfg *rulecfg)
{
	struct nss_tunipip6_msg tnlmsg;
	struct nss_tunipip6_map_rule *rule_add;
	struct nss_ctx_instance *nss_ctx;
	nss_tx_status_t status;
	int inner_ifnum;
	int msg_type;

	if (!netdev || !rulecfg) {
		return NSS_CONNMGR_TUNIPIP6_INVALID_PARAM;
	}

	inner_ifnum = nss_cmn_get_interface_number_by_dev_and_type(netdev, NSS_DYNAMIC_INTERFACE_TYPE_TUNIPIP6_INNER);
	if (inner_ifnum < 0) {
		nss_tunipip6_warning("%px: Invalid inner interface number: %d\n", netdev, inner_ifnum);
		return NSS_CONNMGR_TUNIPIP6_NO_DEV;
	}

	switch (rulecfg->rule_type) {
	case NSS_CONNMGR_TUNIPIP6_RULE_BMR:
		msg_type = NSS_TUNIPIP6_BMR_RULE_ADD;
		break;
	case NSS_CONNMGR_TUNIPIP6_RULE_FMR:
		msg_type = NSS_TUNIPIP6_FMR_RULE_ADD;
		break;
	default:
		nss_tunipip6_warning("%p: Invalid rule type: %d", netdev, rulecfg->rule_type);
		return NSS_CONNMGR_TUNIPIP6_INVALID_RULE_TYPE;
	}

	/*
	 * Prepare the mapping rule parameter to send to nss
	 */
	memset(&tnlmsg, 0, sizeof(struct nss_tunipip6_msg));
	rule_add = &tnlmsg.msg.map_rule;

	rule_add->ip6_prefix[0] = rulecfg->ipv6_prefix[0];
	rule_add->ip6_prefix[1] = rulecfg->ipv6_prefix[1];
	rule_add->ip6_prefix[2] = rulecfg->ipv6_prefix[2];
	rule_add->ip6_prefix[3] = rulecfg->ipv6_prefix[3];
	rule_add->ip6_prefix_len = rulecfg->ipv6_prefix_len;

	rule_add->ip4_prefix = rulecfg->ipv4_prefix;
	rule_add->ip4_prefix_len = rulecfg->ipv4_prefix_len;

	rule_add->ip6_suffix[0] = rulecfg->ipv6_suffix[0];
	rule_add->ip6_suffix[1] = rulecfg->ipv6_suffix[1];
	rule_add->ip6_suffix[2] = rulecfg->ipv6_suffix[2];
	rule_add->ip6_suffix[3] = rulecfg->ipv6_suffix[3];
	rule_add->ip6_suffix_len = rulecfg->ipv6_suffix_len;

	rule_add->ea_len = rulecfg->ea_len;
	rule_add->psid_offset = rulecfg->psid_offset;

	/*
	 * Send maprule add message to encap interface.
	 */
	nss_tunipip6_msg_init(&tnlmsg, inner_ifnum, msg_type,
			sizeof(struct nss_tunipip6_map_rule), NULL, NULL);

	nss_ctx = nss_tunipip6_get_context();
	nss_tunipip6_trace("%px: Sending IPIP6 tunnel maprule, message type %d add command to NSS %p\n", netdev, msg_type, nss_ctx);
	status = nss_tunipip6_tx_sync(nss_ctx, &tnlmsg);
	if (status != NSS_TX_SUCCESS) {
		nss_tunipip6_warning("%px: Tunnel maprule add command error %d\n", netdev, status);
		return NSS_CONNMGR_TUNIPIP6_MAPRULE_ADD_FAILURE;
	}

	return NSS_CONNMGR_TUNIPIP6_SUCCESS;
}
EXPORT_SYMBOL(nss_connmgr_tunipip6_add_maprule);

/*
 * nss_connmgr_tunipip6_del_maprule()
 * 	Delete existing BMR/FMR entry.
 */
enum nss_connmgr_tunipip6_err_codes nss_connmgr_tunipip6_del_maprule(struct net_device *netdev, struct nss_connmgr_tunipip6_maprule_cfg *rulecfg)
{
	struct nss_tunipip6_msg tnlmsg;
	struct nss_tunipip6_map_rule *map_rule;
	struct nss_ctx_instance *nss_ctx;
	nss_tx_status_t status;
	int inner_ifnum;
	int msg_type;

	if (!netdev || !rulecfg) {
		return NSS_CONNMGR_TUNIPIP6_INVALID_PARAM;
	}

	inner_ifnum = nss_cmn_get_interface_number_by_dev_and_type(netdev, NSS_DYNAMIC_INTERFACE_TYPE_TUNIPIP6_INNER);
	if (inner_ifnum < 0) {
		nss_tunipip6_warning("%px: Invalid inner interface number: %d\n", netdev, inner_ifnum);
		return NSS_CONNMGR_TUNIPIP6_NO_DEV;
	}

	/*
	 * Intialize tunnel message.
	 */
	memset(&tnlmsg, 0, sizeof(struct nss_tunipip6_msg));

	/*
	 * To delete BMR, only delete message is needed.
	 */
	switch (rulecfg->rule_type) {
	case NSS_CONNMGR_TUNIPIP6_RULE_BMR:
		msg_type = NSS_TUNIPIP6_BMR_RULE_DEL;

		/*
		 * To delete BMR, only delete message is needed.
		 */
		goto del_bmr;
	case NSS_CONNMGR_TUNIPIP6_RULE_FMR:
		msg_type = NSS_TUNIPIP6_FMR_RULE_DEL;
		break;
	default:
		nss_tunipip6_warning("%p: Invalid rule type: %d", netdev, rulecfg->rule_type);
		return NSS_CONNMGR_TUNIPIP6_INVALID_RULE_TYPE;
	}

	/*
	 * Prepare the maprule rule parameter to send to NSS
	 */
	map_rule = &tnlmsg.msg.map_rule;
	map_rule->ip6_prefix[0] = rulecfg->ipv6_prefix[0];
	map_rule->ip6_prefix[1] = rulecfg->ipv6_prefix[1];
	map_rule->ip6_prefix[2] = rulecfg->ipv6_prefix[2];
	map_rule->ip6_prefix[3] = rulecfg->ipv6_prefix[3];
	map_rule->ip6_prefix_len = rulecfg->ipv6_prefix_len;

	map_rule->ip4_prefix = rulecfg->ipv4_prefix;
	map_rule->ip4_prefix_len = rulecfg->ipv4_prefix_len;

	map_rule->ip6_suffix[0] = rulecfg->ipv6_suffix[0];
	map_rule->ip6_suffix[1] = rulecfg->ipv6_suffix[1];
	map_rule->ip6_suffix[2] = rulecfg->ipv6_suffix[2];
	map_rule->ip6_suffix[3] = rulecfg->ipv6_suffix[3];
	map_rule->ip6_suffix_len = rulecfg->ipv6_suffix_len;

	map_rule->ea_len = rulecfg->ea_len;
	map_rule->psid_offset = rulecfg->psid_offset;

del_bmr:
	/*
	 * Send map rule delete message to encap interface.
	 */
	nss_tunipip6_msg_init(&tnlmsg, inner_ifnum, msg_type,
			sizeof(struct nss_tunipip6_map_rule), NULL, NULL);
	nss_ctx = nss_tunipip6_get_context();
	nss_tunipip6_trace("%px: Sending IPIP6 tunnel maprule, message type %d delete command to NSS %p\n", netdev, msg_type, nss_ctx);
	status = nss_tunipip6_tx_sync(nss_ctx, &tnlmsg);
	if (status != NSS_TX_SUCCESS) {
		nss_tunipip6_warning("%px: Tunnel maprule delete command error %d\n", netdev, status);
		return NSS_CONNMGR_TUNIPIP6_MAPRULE_DEL_FAILURE;
	}

	return NSS_CONNMGR_TUNIPIP6_SUCCESS;
}
EXPORT_SYMBOL(nss_connmgr_tunipip6_del_maprule);

/*
 * nss_connmgr_tunipip6_flush_fmr_rule()
 * 	Flush existing FMR entry.
 */
enum nss_connmgr_tunipip6_err_codes nss_connmgr_tunipip6_flush_fmr_rule(struct net_device *netdev)
{
	struct nss_tunipip6_msg tnlmsg;
	struct nss_ctx_instance *nss_ctx;
	nss_tx_status_t status;
	int inner_ifnum;

	if (!netdev) {
		return NSS_CONNMGR_TUNIPIP6_INVALID_PARAM;
	}

	/*
	 * Send fmr rule flush message to encap interface.
	 */
	inner_ifnum = nss_cmn_get_interface_number_by_dev_and_type(netdev, NSS_DYNAMIC_INTERFACE_TYPE_TUNIPIP6_INNER);
	if (inner_ifnum < 0) {
		nss_tunipip6_warning("%px: Invalid inner interface number: %d\n", netdev, inner_ifnum);
		return NSS_CONNMGR_TUNIPIP6_NO_DEV;
	}

	nss_tunipip6_msg_init(&tnlmsg, inner_ifnum, NSS_TUNIPIP6_FMR_RULE_FLUSH,
			0, NULL, NULL);

	nss_ctx = nss_tunipip6_get_context();
	nss_tunipip6_trace("%px: Sending IPIP6 tunnel FMR rule flush command to NSS %p\n", netdev, nss_ctx);
	status = nss_tunipip6_tx_sync(nss_ctx, &tnlmsg);
	if (status != NSS_TX_SUCCESS) {
		nss_tunipip6_warning("%px: FMR rule flush command error %d\n", netdev, status);
		return NSS_CONNMGR_TUNIPIP6_FMR_RULE_FLUSH_FAILURE;
	}

	return NSS_CONNMGR_TUNIPIP6_SUCCESS;
}
EXPORT_SYMBOL(nss_connmgr_tunipip6_flush_fmr_rule);

/*
 * nss_tunipip6_dev_event()
 *	Net device notifier for ipip6 module
 */
static int nss_tunipip6_dev_event(struct notifier_block  *nb,
		unsigned long event, void  *dev)
{
	struct net_device *netdev = netdev_notifier_info_to_dev(dev);
	struct nss_connmgr_tunipip6_tunnel_cfg tnlcfg = {0};

	/*
	 * Validate netdev for ipv6-in-ipv4  Tunnel
	 */
	if (netdev->type != ARPHRD_TUNNEL6 ) {
		return NOTIFY_DONE;;
	}

	switch (event) {
	case NETDEV_UP:
		nss_tunipip6_trace("%px: NETDEV_UP :event %lu name %s\n", netdev, event, netdev->name);

		/*
		 * Create NSS interface for standard tunnel. The creation of the tunnel
		 * is a three step process.
		 * 1.	'tnlcfg' is populated from the netdev parameters.
		 * 2.	Create API is called to create NSS encap and decap nodes.
		 * 3.	Configure FMR rules(if present) in NSS encap node.
		 */
		nss_tunipip6_dev_parse_param(netdev, &tnlcfg);
		if (nss_connmgr_tunipip6_create_interface(netdev, &tnlcfg) != NSS_CONNMGR_TUNIPIP6_SUCCESS) {
			nss_tunipip6_trace("%px: Not able to create tunnel for dev: %s\n", netdev, netdev->name);
			return NOTIFY_DONE;
		}
#if IS_ENABLED(CONFIG_MAP_E_SUPPORT)
		nss_connmgr_tunipip6_configure_fmr(netdev);
#endif
		break;

	case NETDEV_DOWN:
		nss_tunipip6_trace("%px: NETDEV_DOWN :event %lu name %s\n", netdev, event, netdev->name);
		nss_connmgr_tunipip6_destroy_interface(netdev);
		break;

	default:
		nss_tunipip6_trace("%px: Unhandled notifier dev %s event %x\n", netdev, netdev->name, (int)event);
		break;
	}

	return NOTIFY_DONE;
}

/*
 * nss_tunipip6_destroy_interface_all()
 * 	Destroy NSS interfaces and free instance for all tunipip6 interfaces.
 */
static void nss_tunipip6_destroy_interface_all(void)
{
	struct net_device *netdev;
	struct dentry *dentry;
	struct nss_tunipip6_instance *ntii;

	spin_lock_bh(&tunipip6_ctx.lock);
	ntii = list_first_entry_or_null(&tunipip6_ctx.dev_list, struct nss_tunipip6_instance, list);
	do {
		if (!ntii) {
			spin_unlock_bh(&tunipip6_ctx.lock);
			return;
		}

		netdev = ntii->dev;
		dentry = ntii->dentry;
		nss_tunipip6_free_instance(ntii);
		spin_unlock_bh(&tunipip6_ctx.lock);

		dev_put(netdev);
		debugfs_remove(dentry);
		_nss_tunipip6_dyn_interface_destroy(netdev);

		spin_lock_bh(&tunipip6_ctx.lock);
		ntii = list_first_entry_or_null(&tunipip6_ctx.dev_list, struct nss_tunipip6_instance, list);
	} while (ntii);
	spin_unlock_bh(&tunipip6_ctx.lock);
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

	/*
	 * Initialize lock and dev list.
	 */
	INIT_LIST_HEAD(&tunipip6_ctx.dev_list);
	spin_lock_init(&tunipip6_ctx.lock);

	/*
	 * Create the debugfs directory for statistics.
	 */
	if (!nss_tunipip6_stats_dentry_init()) {
		nss_tunipip6_trace("Failed to initialize debugfs\n");
		return -1;
	}

	/*
	 * Do not register net device notification for
	 * custom tunnel. Net device notification is used only
	 * for standard tunnel.
	 */
	if (!enable_custom) {
		register_netdevice_notifier(&nss_tunipip6_notifier);
		nss_tunipip6_trace("Netdev Notifier registerd\n");
	}

	/*
	 * Register sysctl to add/delete/flush mapping rules.
	 */
	nss_tunipip6_sysctl_register();
	nss_tunipip6_trace("Sysctl registerd\n");

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

	/*
	 * Free Host and NSS tunipip6 instances.
	 */
	nss_tunipip6_destroy_interface_all();

	/*
	 * De-initialize debugfs.
	 */
	nss_tunipip6_stats_dentry_deinit();

	/*
	 * Unregister net device notification for standard tunnel.
	 */
	if (!enable_custom) {
		unregister_netdevice_notifier(&nss_tunipip6_notifier);
		nss_tunipip6_trace("Netdev Notifier unregisterd\n");
	}

	/*
	 * Unregister sysctl.
	 */
	nss_tunipip6_sysctl_unregister();
	nss_tunipip6_trace("Sysctl unregisterd\n");

	nss_tunipip6_info("module unloaded\n");
}

module_init(nss_tunipip6_init_module);
module_exit(nss_tunipip6_exit_module);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS tunipip6 offload manager");
