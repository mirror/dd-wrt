/*
 **************************************************************************
 * Copyright (c) 2014-2017, 2020, The Linux Foundation. All rights reserved.
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

/* nss_ipsecmgr.c
 *	NSS to HLOS IPSec Manager
 */
#include <linux/version.h>
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/of.h>
#include <linux/ipv6.h>
#include <linux/skbuff.h>
#include <linux/module.h>
#include <linux/bitops.h>
#include <linux/netdevice.h>
#include <linux/rtnetlink.h>
#include <linux/etherdevice.h>
#include <linux/vmalloc.h>
#include <linux/debugfs.h>
#include <linux/atomic.h>
#include <net/protocol.h>
#include <net/route.h>
#include <net/ip6_route.h>
#include <net/esp.h>
#include <net/xfrm.h>
#include <net/icmp.h>

#include <nss_api_if.h>
#include <nss_ipsec.h>
#include <nss_ipsecmgr.h>

#include "nss_ipsecmgr_priv.h"
#include <nss_tstamp.h>

extern bool nss_cmn_get_nss_enabled(void);

struct nss_ipsecmgr_drv *ipsecmgr_ctx;

static bool gen_pmtu_error = true;
module_param(gen_pmtu_error, bool, 0644);
MODULE_PARM_DESC(gen_pmtu_error, "Support generation of PMTU error packet");

/*
 **********************
 * Helper Functions
 **********************
 */

/*
 * nss_ipsecmgr_ref_no_update()
 *     dummy functions for object owner when there is no update
 */
static void nss_ipsecmgr_ref_no_update(struct nss_ipsecmgr_priv *priv, struct nss_ipsecmgr_ref *child, struct nss_ipsec_msg *nim)
{
	nss_ipsecmgr_trace("ref_no_update triggered for child (%px)\n", child);
	return;
}

/*
 * nss_ipsecmgr_ref_no_free()
 * 	dummy functions for object owner when there is no free
 */
static void nss_ipsecmgr_ref_no_free(struct nss_ipsecmgr_priv *priv, struct nss_ipsecmgr_ref *ref)
{
	nss_ipsecmgr_trace("%px:ref_no_free triggered\n", ref);
	return;
}

/*
 * nss_ipsecmgr_ref_no_overhead()
 *	dummy functions for object owner when there is no overhead
 */
static uint32_t nss_ipsecmgr_ref_no_overhead(struct nss_ipsecmgr_ref *ref)
{
	nss_ipsecmgr_trace("%px:ref_get_no_overhead triggered\n", ref);
	return 0;
}

/*
 * nss_ipsecmgr_ref_set_overhead()
 *	set the overhead function for reference object
 */
void nss_ipsecmgr_ref_set_overhead(struct nss_ipsecmgr_ref *ref, nss_ipsecmgr_ref_overhead_t overhead)
{
	ref->overhead = overhead;
}

/*
 * nss_ipsecmgr_ref_init()
 * 	initiaize the reference object
 */
void nss_ipsecmgr_ref_init(struct nss_ipsecmgr_ref *ref, nss_ipsecmgr_ref_update_t update, nss_ipsecmgr_ref_free_t free)
{
	INIT_LIST_HEAD(&ref->head);
	INIT_LIST_HEAD(&ref->node);

	ref->id = 0;
	ref->parent = NULL;
	ref->update = update ? update : nss_ipsecmgr_ref_no_update;
	ref->free = free ? free : nss_ipsecmgr_ref_no_free;
	ref->overhead = nss_ipsecmgr_ref_no_overhead;
}

/*
 * nss_ipsecmgr_ref_add()
 * 	add child reference to parent chain
 */
void nss_ipsecmgr_ref_add(struct nss_ipsecmgr_ref *child, struct nss_ipsecmgr_ref *parent)
{
	/*
	 * if child is already part of an existing chain then remove it before
	 * adding it to the new one. In case this is a new entry then the list
	 * init during alloc would ensure that the "del_init" operation results
	 * in a no-op
	 */
	list_del_init(&child->node);
	list_add(&child->node, &parent->head);

	child->parent = parent;
}

/*
 * nss_ipsecmgr_ref_update()
 * 	update the "ref" object and link it to the parent
 */
void nss_ipsecmgr_ref_update(struct nss_ipsecmgr_priv *priv, struct nss_ipsecmgr_ref *child, struct nss_ipsec_msg *nim)
{
	struct nss_ipsecmgr_ref *entry;

	child->id++;
	child->update(priv, child, nim);

	/*
	 * If, there are references to associated with this
	 * object then notify them about the change. This allows
	 * the "ref" objects to trigger notifications to NSS for
	 * updates to SA
	 */
	list_for_each_entry(entry, &child->head, node) {
		nss_ipsecmgr_ref_update(priv, entry, nim);
	}
}

/*
 * nss_ipsecmgr_ref_overhead()
 *	Get the SA overhead of the reference passed
 *
 * note: ideally this will trigger a chain of callbacks till
 * the SA
 */
uint32_t nss_ipsecmgr_ref_overhead(struct nss_ipsecmgr_ref *ref)
{
	if (!ref->parent)
		return ref->overhead(ref);

	return nss_ipsecmgr_ref_overhead(ref->parent);
}

/*
 * nss_ipsecmgr_ref_free()
 * 	Free all references from the "ref" object
 *
 * Note: If, the "ref" has child references then it
 * will walk the child reference chain first and issue
 * free for each of the associated "child ref" objects.
 * At the end it will invoke free for the "parent" ref
 * object.
 *
 * +-------+   +-------+   +-------+
 * |  SA1  +--->   SA2 +--->  SA3  |
 * +---+---+   +---+---+   +-------+
 *     |
 * +---V---+   +-------+   +-------+
 * | Flow1 +---> Sub1  +---> Flow4 |
 * +-------+   +---+---+   +-------+
 *                 |
 *             +---v---+
 *             | Flow2 |
 *             +---+---+
 *                 |
 *             +---v---+
 *             | Flow3 |
 *             +-------+
 */
void nss_ipsecmgr_ref_free(struct nss_ipsecmgr_priv *priv, struct nss_ipsecmgr_ref *ref)
{
	struct nss_ipsecmgr_ref *entry;

	while (!list_empty(&ref->head)) {
		entry = list_first_entry(&ref->head, struct nss_ipsecmgr_ref, node);
		nss_ipsecmgr_ref_free(priv, entry);
	}

	list_del_init(&ref->node);
	ref->free(priv, ref);
}

/*
 * nss_ipsecmgr_ref_is_child()
 * 	return true if the child is direct sibling of parent
 */
bool nss_ipsecmgr_ref_is_child(struct nss_ipsecmgr_ref *child, struct nss_ipsecmgr_ref *parent)
{
	struct nss_ipsecmgr_ref *entry;

	list_for_each_entry(entry, &parent->head, node) {
		if (entry == child) {
			return true;
		}
	}

	return false;
}

/*
 **********************
 * Netdev ops
 **********************
 */

/*
 * nss_ipsecmgr_tunnel_open()
 * 	open the tunnel for usage
 */
static int nss_ipsecmgr_tunnel_open(struct net_device *dev)
{
	struct nss_ipsecmgr_priv *priv;

	priv = netdev_priv(dev);

	netif_start_queue(dev);

	return 0;
}

/*
 * nss_ipsecmgr_tunnel_stop()
 * 	stop the IPsec tunnel
 */
static int nss_ipsecmgr_tunnel_stop(struct net_device *dev)
{
	struct nss_ipsecmgr_priv *priv;

	priv = netdev_priv(dev);

	netif_stop_queue(dev);

	return 0;
}

/*
 * nss_ipsecmgr_tunnel_tx()
 * 	tunnel transmit function
 */
static netdev_tx_t nss_ipsecmgr_tunnel_tx(struct sk_buff *skb, struct net_device *dev)
{
	struct nss_ipsecmgr_flow_data flow_data = {0};
	bool process_mtu = gen_pmtu_error;
	struct nss_ipsecmgr_priv *priv;
	bool expand_skb = false;
	int nhead, ntail;
	bool tstamp_skb;

	priv = netdev_priv(dev);
	nhead = dev->needed_headroom;
	ntail = dev->needed_tailroom;

	tstamp_skb = skb_shinfo(skb)->tx_flags & SKBTX_HW_TSTAMP;

	/*
	 * Check if skb is non-linear
	 */
	if (skb_is_nonlinear(skb)) {
		nss_ipsecmgr_trace("%s: NSS IPSEC does not support fragments %px\n", dev->name, skb);
		goto free;
	}

	/*
	 * Check if skb is shared
	 */
	if (unlikely(skb_shared(skb))) {
		nss_ipsecmgr_trace("%s: Shared skb is not supported: %px\n", dev->name, skb);
		goto free;
	}

	/*
	 * Check if packet is given starting from network header
	 */
	if (skb->data != skb_network_header(skb)) {
		nss_ipsecmgr_trace("%s: 'Skb data is not starting from IP header\n", dev->name);
		goto free;
	}

	/*
	 * For all these cases
	 * - create a writable copy of buffer
	 * - increase the head room
	 * - increase the tail room
	 */
	if (skb_cloned(skb) || (skb_headroom(skb) < nhead) || (skb_tailroom(skb) < ntail)) {
		expand_skb = true;
	}

	if (expand_skb && pskb_expand_head(skb, nhead, ntail, GFP_KERNEL)) {
		nss_ipsecmgr_trace("%s: unable to expand buffer\n", dev->name);
		goto free;
	}

	/*
	 * Before proceeding check for the following conditions
	 * For IPv4 packet if DF bit is set then process_mtu
	 */
	if (process_mtu && (skb->protocol == htons(ETH_P_IP)))
		process_mtu = !!(ip_hdr(skb)->frag_off & htons(IP_DF));

	/*
	 * check whether the IPsec encapsulation can be offloaded to NSS
	 *	- if the flow matches a subnet rule, then a new flow rule
	 *		is added to NSS.
	 *	- if the flow doesn't match any subnet, then the packet
	 * 		is dropped
	 */
	if (!nss_ipsecmgr_flow_offload(priv, skb, &flow_data)) {
		nss_ipsecmgr_warn("%px:failed to accelerate flow\n", dev);
		goto free;
	}

	/*
	 * Check if pre-fragmentation is not enabled or already a fragment.
	 * then send the buffer on its way to NSS
	 */
	if (process_mtu && nss_ipsecmgr_flow_process_pmtu(priv, skb, &flow_data))
		goto free;

	/*
	 * If the packet needs to timestamped. Send to
	 * timestamping NSS module.
	 */
	if (unlikely(tstamp_skb)) {
		if (nss_tstamp_tx_buf(ipsecmgr_ctx->nss_ctx, skb, NSS_IPSEC_ENCAP_IF_NUMBER))
			goto free;

		return NETDEV_TX_OK;
	}

	/*
	 * Send the packet down
	 */
	if (nss_ipsec_tx_buf(skb, ipsecmgr_ctx->encap_ifnum) != 0) {
		/*
		 * TODO: NEED TO STOP THE QUEUE
		 */
		goto free;
	}

	return NETDEV_TX_OK;

free:
	dev_kfree_skb_any(skb);
	return NETDEV_TX_OK;
}

/*
 * nss_ipsecmgr_tunnel_stats()
 * 	get tunnel statistics
 */
void nss_ipsecmgr_tunnel_stats64(struct net_device *dev, struct rtnl_link_stats64 *stats)
{
	struct nss_ipsecmgr_priv *priv = netdev_priv(dev);

	memset(stats, 0, sizeof(struct rtnl_link_stats64));

	/*
	 * trigger a stats update chain
	 */
	read_lock_bh(&ipsecmgr_ctx->lock);
	memcpy(stats, &priv->stats, sizeof(struct rtnl_link_stats64));
	read_unlock_bh(&ipsecmgr_ctx->lock);
}

/*
 * nss_ipsecmgr_change_mtu()
 * 	change MTU size of IPsec tunnel device
 */
static int32_t nss_ipsecmgr_change_mtu(struct net_device *dev, int32_t mtu)
{
	dev->mtu = mtu;
	return 0;
}

/* NSS IPsec tunnel operation */
static const struct net_device_ops nss_ipsecmgr_tunnel_ops = {
	.ndo_open = nss_ipsecmgr_tunnel_open,
	.ndo_stop = nss_ipsecmgr_tunnel_stop,
	.ndo_start_xmit = nss_ipsecmgr_tunnel_tx,
	.ndo_get_stats64 = nss_ipsecmgr_tunnel_stats64,
	.ndo_change_mtu = nss_ipsecmgr_change_mtu,
};

/*
 * nss_ipsecmgr_tunnel_free()
 * 	free an existing IPsec tunnel interface
 */
static void nss_ipsecmgr_tunnel_free(struct net_device *dev)
{
	nss_ipsecmgr_info("IPsec tunnel device(%s) freed\n", dev->name);

	free_netdev(dev);
}

/*
 * nss_ipsecmr_setup_tunnel()
 * 	setup the IPsec tunnel
 */
static void nss_ipsecmgr_tunnel_setup(struct net_device *dev)
{
	dev->addr_len = ETH_ALEN;
	dev->mtu = NSS_IPSECMGR_TUN_MTU(ETH_DATA_LEN);

	dev->hard_header_len = NSS_IPSECMGR_TUN_MAX_HDR_LEN;
	dev->needed_headroom = NSS_IPSECMGR_TUN_HEADROOM;
	dev->needed_tailroom = NSS_IPSECMGR_TUN_TAILROOM;

	dev->type = NSS_IPSEC_ARPHRD_IPSEC;

	dev->ethtool_ops = NULL;
	dev->header_ops = NULL;
	dev->netdev_ops = &nss_ipsecmgr_tunnel_ops;

	dev->priv_destructor = nss_ipsecmgr_tunnel_free;

	/*
	 * get the MAC address from the ethernet device
	 */
	random_ether_addr(dev->dev_addr);

	memset(dev->broadcast, 0xff, dev->addr_len);
	memcpy(dev->perm_addr, dev->dev_addr, dev->addr_len);
}

/*
 * nss_ipsecmgr_tunnel_get_callback()
 * 	get the callback entry for the dev index
 *
 * Note: this is typically expected in the RX BH path
 */
static struct nss_ipsecmgr_callback_entry *nss_ipsecmgr_tunnel_get_callback(int dev_index)
{
	struct nss_ipsecmgr_callback_db *cb_db = &ipsecmgr_ctx->cb_db;
	struct nss_ipsecmgr_callback_entry *cb_entry;

	if (!atomic_read(&cb_db->num_entries))
		return NULL;

	BUG_ON(!in_atomic());

	/*
	 * search the callback database to find if there
	 * are any registered callback for the net_device
	 */
	read_lock(&ipsecmgr_ctx->lock);				/* lock */
	list_for_each_entry(cb_entry, &cb_db->entries, node) {
		if (cb_entry->dev_index == dev_index) {
			read_unlock(&ipsecmgr_ctx->lock);	/* unlock */
			return cb_entry;
		}
	}

	read_unlock(&ipsecmgr_ctx->lock);			/* unlock */

	return NULL;
}

/*
 * nss_ipsecmgr_tunnel_get_dev()
 *	Get the net_device associated with the packet.
 */
static struct net_device *nss_ipsecmgr_tunnel_get_dev(struct sk_buff *skb)
{
	struct nss_ipsecmgr_sa_entry *sa_entry;
	struct nss_ipsec_tuple tuple;
	struct nss_ipsecmgr_ref *ref;
	struct nss_ipsecmgr_key key;
	struct net_device *dev;
	struct dst_entry *dst;
	struct ip_esp_hdr *esph;
	size_t hdr_sz = 0;

	skb_reset_mac_header(skb);
	skb_reset_network_header(skb);

	switch (ip_hdr(skb)->version) {
	case IPVERSION:
	{
		struct iphdr *iph = ip_hdr(skb);
		struct rtable *rt;
		bool is_encap;

		hdr_sz = sizeof(struct iphdr);
		skb->protocol = cpu_to_be16(ETH_P_IP);

		/*
		 * Set the transport header skb pointer
		 */
		skb_set_transport_header(skb, hdr_sz);

		is_encap = (iph->protocol == IPPROTO_ESP);
		if (!is_encap && (iph->protocol == IPPROTO_UDP)) {
			if (udp_hdr(skb)->dest == NSS_IPSECMGR_NATT_PORT_DATA) {
				hdr_sz += sizeof(struct udphdr);
				is_encap = true;
			}
		}

		if (!is_encap) {
			rt = ip_route_output(&init_net, iph->saddr, 0, 0, 0);
			if (IS_ERR(rt)) {
				return NULL;
			}

			dst = (struct dst_entry *)rt;
			dev = dst->dev;
			dst_release(dst);
			goto done;
		}

		nss_ipsecmgr_v4_hdr2tuple(iph, &tuple);
		break;
	}

	case 6:
	{
		struct ipv6hdr *ip6h = ipv6_hdr(skb);
		struct flowi6 fl6;

		hdr_sz = sizeof(struct ipv6hdr);
		skb->protocol = cpu_to_be16(ETH_P_IPV6);

		nss_ipsecmgr_v6_hdr2tuple(ip6h, &tuple);

		if (tuple.proto_next_hdr != IPPROTO_ESP) {
			memset(&fl6, 0, sizeof(fl6));
			memcpy(&fl6.daddr, &ip6h->saddr, sizeof(fl6.daddr));

			dst = ip6_route_output(&init_net, NULL, &fl6);
			if (IS_ERR(dst)) {
				return NULL;
			}

			dev = dst->dev;
			dst_release(dst);
			goto done;
		}

		if (ip6h->nexthdr == NEXTHDR_FRAGMENT) {
			hdr_sz += sizeof(struct frag_hdr);
		}
		break;
	}
	default:
		nss_ipsecmgr_warn("%px:could not get dev for the flow\n", skb);
		return NULL;
	}

	skb_set_transport_header(skb, hdr_sz);
	esph = ip_esp_hdr(skb);
	tuple.esp_spi = ntohl(esph->spi);

	nss_ipsecmgr_sa_tuple2key(&tuple, &key);

	ref = nss_ipsecmgr_sa_lookup(&key);
	if (!ref) {
		nss_ipsecmgr_trace("unable to find SA (%px)\n", skb);
		return NULL;
	}

	sa_entry = container_of(ref, struct nss_ipsecmgr_sa_entry, ref);
	dev = sa_entry->priv->dev;
done:

	skb->pkt_type = PACKET_HOST;
	skb->skb_iif = dev->ifindex;
	skb->dev = dev;
	return dev;
}

/*
 * nss_ipsecmgr_tunnel_rx()
 *	receive NSS exception packets
 */
static void nss_ipsecmgr_tunnel_rx(struct net_device *dummy, struct sk_buff *skb, __attribute((unused)) struct napi_struct *napi)
{
	struct nss_ipsecmgr_callback_entry *cb_entry;
	nss_ipsecmgr_data_cb_t cb_fn;
	struct net_device *dev;

	BUG_ON(dummy == NULL);
	BUG_ON(skb == NULL);

	dev = nss_ipsecmgr_tunnel_get_dev(skb);
	if (unlikely(!dev)) {
		nss_ipsecmgr_trace("cannot find a dev(%px)\n", skb);
		dev_kfree_skb_any(skb);
		return;
	}

	dev_hold(dev);

	/*
	 * search the device in the registered callback table;
	 * if there is match then load the callback for indication
	 */
	cb_entry = nss_ipsecmgr_tunnel_get_callback(dev->ifindex);
	if (!cb_entry) {
		netif_receive_skb(skb);
		goto done;
	}

	cb_fn = cb_entry->data;
	BUG_ON(!cb_fn);

	cb_fn(cb_entry->app_data, skb);
done:
	dev_put(dev);
}

/*
 * nss_ipsecmgr_tunnel_update_stats()
 *	Update tunnel stats
 */
static void nss_ipsecmgr_tunnel_update_stats(struct nss_ipsecmgr_priv *priv, struct nss_ipsec_msg *nim)
{
	struct rtnl_link_stats64 *tun_stats;
	struct nss_ipsec_sa_stats *pkts;

	tun_stats = &priv->stats;
	pkts = &nim->msg.stats.sa;

	if (nim->type == NSS_IPSEC_TYPE_ENCAP) {
		/*
		 * update tunnel specific stats
		 */
		tun_stats->tx_bytes += pkts->bytes;
		tun_stats->tx_packets += pkts->count;

		tun_stats->tx_dropped += pkts->no_headroom;
		tun_stats->tx_dropped += pkts->no_tailroom;
		tun_stats->tx_dropped += pkts->no_resource;
		tun_stats->tx_dropped += pkts->fail_queue;
		tun_stats->tx_dropped += pkts->fail_hash;
		tun_stats->tx_dropped += pkts->fail_replay;
		return;
	}

	/*
	 * update tunnel specific stats
	 */
	if (nim->type == NSS_IPSEC_TYPE_ENCAP) {
		tun_stats->tx_bytes += pkts->bytes;
		tun_stats->tx_packets += pkts->count;

		tun_stats->tx_dropped += pkts->no_headroom;
		tun_stats->tx_dropped += pkts->no_tailroom;
		tun_stats->tx_dropped += pkts->no_resource;
		tun_stats->tx_dropped += pkts->fail_queue;
		tun_stats->tx_dropped += pkts->fail_hash;
		tun_stats->tx_dropped += pkts->fail_replay;
		return;
	}

	tun_stats->rx_bytes += pkts->bytes;
	tun_stats->rx_packets += pkts->count;

	tun_stats->rx_dropped += pkts->no_headroom;
	tun_stats->rx_dropped += pkts->no_tailroom;
	tun_stats->rx_dropped += pkts->no_resource;
	tun_stats->rx_dropped += pkts->fail_queue;
	tun_stats->rx_dropped += pkts->fail_hash;
	tun_stats->rx_dropped += pkts->fail_replay;
}

/*
 * nss_ipsecmgr_tunnel_notify()
 * 	asynchronous event reception
 */
static void nss_ipsecmgr_tunnel_notify(__attribute((unused))void *app_data, struct nss_ipsec_msg *nim)
{
	struct nss_ipsecmgr_node_stats *drv_stats;
	struct nss_ipsec_node_stats *node_stats;
	struct nss_ipsecmgr_sa_stats *sa_stats;
	struct nss_ipsecmgr_event stats_event;
	struct nss_ipsecmgr_sa_entry *sa;
	struct nss_ipsec_sa_stats *pkts;
	struct nss_ipsecmgr_priv *priv;
	nss_ipsecmgr_event_cb_t cb_fn;
	struct nss_ipsecmgr_ref *ref;
	struct nss_ipsecmgr_key key;
	bool reset_fail_hash;
	struct net_device *dev;

	BUG_ON(nim == NULL);

	/*
	 * this holds the ref_cnt for the device
	 */
	dev = dev_get_by_index(&init_net, nim->tunnel_id);
	if (!dev) {
		nss_ipsecmgr_info("event received on deallocated I/F (%d)\n", nim->tunnel_id);
		return;
	}

	priv = netdev_priv(dev);

	switch (nim->cm.type) {
	case NSS_IPSEC_MSG_TYPE_SYNC_SA_STATS:

		/*
		 * prepare and lookup sa based on selector sent from nss
		 */
		nss_ipsecmgr_sa_tuple2key(&nim->tuple, &key);

		write_lock(&ipsecmgr_ctx->lock);

		ref = nss_ipsecmgr_sa_lookup(&key);
		if (!ref) {
			write_unlock(&ipsecmgr_ctx->lock);
			nss_ipsecmgr_trace("event received on deallocated SA tunnel:(%d)\n", nim->tunnel_id);
			goto done;
		}

		sa = container_of(ref, struct nss_ipsecmgr_sa_entry, ref);

		/*
		 * update sa stats in the local database
		 */
		nss_ipsecmgr_sa_stats_update(nim, sa);

		sa_stats = &stats_event.data.stats;

		/*
		 * update tunnel stats
		 */
		sa_stats->fail_hash_alarm = false;
		nss_ipsecmgr_tunnel_update_stats(priv, nim);

		if ((nim->type == NSS_IPSEC_TYPE_DECAP) &&
				sa->fail_hash_thresh) {
			pkts = &nim->msg.stats.sa;

			/*
			 * If the fail_hash_count is zero and packet count is
			 * non-zero. It indicates that the continuous hash
			 * failure was a transient state hence reset the count
			 */
			reset_fail_hash = (!pkts->fail_hash_cont &&
					pkts->count);
			sa->pkts.fail_hash_cont = reset_fail_hash ? 0
				: (sa->pkts.fail_hash_cont +
						pkts->fail_hash_cont);

			/*
			 * Check the fail_hash_cont hash crossed the threshold,
			 * if yes set the alarm.
			 */
			if (sa->pkts.fail_hash_cont >= sa->fail_hash_thresh) {
				sa_stats->fail_hash_alarm = true;
				sa->pkts.fail_hash_cont -= sa->fail_hash_thresh;
			}
		}

		memcpy(&sa_stats->sa, &sa->sa_info,
				sizeof(struct nss_ipsecmgr_sa));
		sa_stats->crypto_index = sa->nim.msg.rule.data.crypto_index;
		write_unlock(&ipsecmgr_ctx->lock);

		/*
		 * if event callback is available then post the statistics using the callback function
		 */
		cb_fn = priv->cb.event;
		if (cb_fn) {
			stats_event.type = NSS_IPSECMGR_EVENT_SA_STATS;

			/*
			 * copy stats and SA information
			 */
			sa_stats->seq_num = nim->msg.stats.sa.seq_num;

			sa_stats->esn_enabled = nim->msg.stats.sa.esn_enabled;
			sa_stats->window_max = nim->msg.stats.sa.window_max;
			sa_stats->window_size = nim->msg.stats.sa.window_size;

			sa_stats->pkts.count = nim->msg.stats.sa.count;
			sa_stats->pkts.bytes = nim->msg.stats.sa.bytes;

			cb_fn(priv->cb.app_data, &stats_event);
		}

		break;

	case NSS_IPSEC_MSG_TYPE_SYNC_NODE_STATS:

		drv_stats = &ipsecmgr_ctx->enc_stats;
		if (unlikely(nim->type == NSS_IPSEC_TYPE_DECAP)) {
			drv_stats = &ipsecmgr_ctx->dec_stats;
		}

		node_stats = &nim->msg.stats.node;
		drv_stats->enqueued += node_stats->enqueued;
		drv_stats->completed += node_stats->completed;
		drv_stats->linearized += node_stats->linearized;
		drv_stats->exceptioned += node_stats->exceptioned;
		drv_stats->fail_enqueue += node_stats->fail_enqueue;

		break;

	default:
		break;
	}
done:
	dev_put(dev);
}

/*
 * nss_ipsecmgr_node_stats_read()
 * 	read node statistics
 */
static ssize_t nss_ipsecmgr_node_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	struct nss_ipsecmgr_node_stats *enc_stats = &ipsecmgr_ctx->enc_stats;
	struct nss_ipsecmgr_node_stats *dec_stats = &ipsecmgr_ctx->dec_stats;
	ssize_t ret = 0;
	char *local;
	int len;

	local = vmalloc(NSS_IPSECMGR_MAX_BUF_SZ);

	len = 0;
	len += snprintf(local + len, NSS_IPSECMGR_MAX_BUF_SZ - len, "\tencap_enqueued: %lld\n", enc_stats->enqueued);
	len += snprintf(local + len, NSS_IPSECMGR_MAX_BUF_SZ - len, "\tencap_completed: %lld\n", enc_stats->completed);
	len += snprintf(local + len, NSS_IPSECMGR_MAX_BUF_SZ - len, "\tencap_exceptioned: %lld\n", enc_stats->exceptioned);
	len += snprintf(local + len, NSS_IPSECMGR_MAX_BUF_SZ - len, "\tencap_enqueue_failed: %lld\n", enc_stats->fail_enqueue);

	len += snprintf(local + len, NSS_IPSECMGR_MAX_BUF_SZ - len, "\tdecap_enqueued: %lld\n", dec_stats->enqueued);
	len += snprintf(local + len, NSS_IPSECMGR_MAX_BUF_SZ - len, "\tdecap_completed: %lld\n", dec_stats->completed);
	len += snprintf(local + len, NSS_IPSECMGR_MAX_BUF_SZ - len, "\tdecap_exceptioned: %lld\n", dec_stats->exceptioned);
	len += snprintf(local + len, NSS_IPSECMGR_MAX_BUF_SZ - len, "\tdecap_enqueue_failed: %lld\n", dec_stats->fail_enqueue);

	ret = simple_read_from_buffer(ubuf, sz, ppos, local, len + 1);

	vfree(local);

	return ret;
}

/*
 * file operation structure instance
 */
static const struct file_operations node_stats_op = {
	.open = simple_open,
	.llseek = default_llseek,
	.read = nss_ipsecmgr_node_stats_read,
};

/*
 * nss_ipsecmgr_tunnel_add()
 * 	add a IPsec pseudo tunnel device
 */
struct net_device *nss_ipsecmgr_tunnel_add(struct nss_ipsecmgr_callback *cb)
{
	struct nss_ipsecmgr_callback_db *cb_db = &ipsecmgr_ctx->cb_db;
	struct nss_ipsecmgr_priv *priv;
	struct net_device *dev;
	int status;

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 17, 0)
	dev = alloc_netdev(sizeof(struct nss_ipsecmgr_priv), NSS_IPSECMGR_TUN_NAME, nss_ipsecmgr_tunnel_setup);
#else
	dev = alloc_netdev(sizeof(struct nss_ipsecmgr_priv), NSS_IPSECMGR_TUN_NAME, NET_NAME_ENUM, nss_ipsecmgr_tunnel_setup);
#endif
	if (!dev) {
		nss_ipsecmgr_error("unable to allocate a tunnel device\n");
		return NULL;
	}

	priv = netdev_priv(dev);
	priv->dev = dev;

	INIT_LIST_HEAD(&priv->cb.node);
	priv->cb.dev_index = dev->ifindex;
	priv->cb.app_data = cb->ctx;
	priv->cb.data = cb->data_fn;
	priv->cb.event = cb->event_fn;

	status = rtnl_is_locked() ? register_netdevice(dev) : register_netdev(dev);
	if (status < 0) {
		nss_ipsecmgr_error("register net dev failed :%s\n", dev->name);
		goto fail;
	}

	/*
	 * only register if data callback is available
	 */
	if (cb->data_fn){
		write_lock_bh(&ipsecmgr_ctx->lock);
		list_add(&priv->cb.node, &cb_db->entries);
		atomic_inc(&cb_db->num_entries);
		write_unlock_bh(&ipsecmgr_ctx->lock);
	}

	return dev;
fail:
	free_netdev(dev);
	return NULL;
}
EXPORT_SYMBOL(nss_ipsecmgr_tunnel_add);

/*
 * nss_ipsecmgr_del_tunnel()
 * 	delete an existing IPsec tunnel
 */
bool nss_ipsecmgr_tunnel_del(struct net_device *dev)
{
	struct nss_ipsecmgr_callback_db *cb_db = &ipsecmgr_ctx->cb_db;
	struct nss_ipsecmgr_priv *priv = netdev_priv(dev);

	write_lock_bh(&ipsecmgr_ctx->lock);
	atomic_dec(&cb_db->num_entries);
	list_del(&priv->cb.node);
	write_unlock_bh(&ipsecmgr_ctx->lock);

	nss_ipsecmgr_sa_flush_all(priv);

	/*
	 * The unregister should start here but the expectation is that the free would
	 * happen when the reference count goes down to '0'
	 */
	rtnl_is_locked() ? unregister_netdevice(dev) : unregister_netdev(dev);

	return true;
}
EXPORT_SYMBOL(nss_ipsecmgr_tunnel_del);

/*
 * nss_ipsecmgr_tunnel_update_callback()
 * 	update the callback databse with the new device index
 *
 * Note: the callback is database that holds callback functions w.r.t a
 * device index. The tunnel_add would typically load this with the device
 * index of the tunnel. Overtime the caller can decide to update this with
 * different device index. In cases where the IPsec stack typically creates
 * a tunnel device of its own (KLIPS), the callback would then get mapped to
 * these devices instead of the IPsec manager created tunnel netdevice
 */
void nss_ipsecmgr_tunnel_update_callback(struct net_device *tun, struct net_device *cur)
{
	struct nss_ipsecmgr_callback_db *cb_db = &ipsecmgr_ctx->cb_db;
	struct nss_ipsecmgr_callback_entry *cb_entry;
	int tun_dev_index = tun->ifindex;

	if (!atomic_read(&cb_db->num_entries))
		return;

	/*
	 * search the old device index in callback table
	 * and replace it with the new index
	 */
	write_lock_bh(&ipsecmgr_ctx->lock);
	list_for_each_entry(cb_entry, &cb_db->entries, node) {
		if (cb_entry->dev_index == tun_dev_index) {
			cb_entry->dev_index = cur->ifindex;
			break;
		}
	}

	write_unlock_bh(&ipsecmgr_ctx->lock);
}
EXPORT_SYMBOL(nss_ipsecmgr_tunnel_update_callback);

static const struct net_device_ops nss_ipsecmgr_ipsec_ndev_ops;

/*
 * nss_ipsecmgr_dummy_netdevice_setup()
 *	Setup function for dummy netdevice.
 */
static void nss_ipsecmgr_dummy_netdevice_setup(struct net_device *dev)
{
}

/*
 * flow file operation structure instance
 */
static const struct file_operations flow_stats_op = {
	.open = simple_open,
	.llseek = default_llseek,
	.read = nss_ipsecmgr_flow_stats_read,
};

/*
 * per flow file operation structure instance
 */
static const struct file_operations per_flow_stats_op = {
	.open = simple_open,
	.llseek = default_llseek,
	.read = nss_ipsecmgr_per_flow_stats_read,
	.write = nss_ipsecmgr_per_flow_stats_write,
};

/*
 * SA file operation structure instance
 */
static const struct file_operations sa_stats_op = {
	.open = simple_open,
	.llseek = default_llseek,
	.read = nss_ipsecmgr_sa_stats_read,
};

/*
 * subnet file operation structure instance
 */
static const struct file_operations subnet_stats_op = {
	.open = simple_open,
	.llseek = default_llseek,
	.read = nss_ipsecmgr_netmask_stats_read,
};

/*
 * nss_ipsecmgr_init_stats_debugfs()
 *	Initialize the debugfs tree.
 */
static int nss_ipsecmgr_init_stats_debugfs(struct dentry *stats_root)
{
	if (!debugfs_create_file("subnet", S_IRUGO, stats_root, (uint32_t *)NULL, &subnet_stats_op)) {
		nss_ipsecmgr_error("Debugfs file creation failed for subnet\n");
		return -1;
	}

	if (!debugfs_create_file("sa", S_IRUGO, stats_root, NULL, &sa_stats_op)) {
		nss_ipsecmgr_error("Debugfs file creation failed for SA\n");
		return -1;
	}

	if (!debugfs_create_file("flow", S_IRUGO, stats_root, NULL, &flow_stats_op)) {
		nss_ipsecmgr_error("Debugfs file creation failed for flow\n");
		return -1;
	}

	if (!debugfs_create_file("per_flow", S_IRWXUGO, stats_root, NULL, &per_flow_stats_op)) {
		nss_ipsecmgr_error("Debugfs file creation failed for per flow\n");
		return -1;
	}

	if (!debugfs_create_file("node", S_IRUGO, stats_root, NULL, &node_stats_op)) {
		nss_ipsecmgr_error("Debugfs file creation failed for per node\n");
		return -1;
	}

	return 0;
}

#if defined NSS_IPSECMGR_PMTU_SUPPORT

/*
 * nss_ipsecmgr_esp4_rcv()
 *	IPv4 Receive handler for ESP protocol
 */
static int nss_ipsecmgr_tunnel_rx_esp4(struct sk_buff *skb)
{
	/*
	 * TODO:This can potentially receive ESP packets in when
	 * the outer ESP rule is flushed. In which case the DECAP
	 * packets entering linux must be bounced through offload
	 */
	dev_kfree_skb_any(skb);
	return 0;
}

/*
 * nss_ipsecmgr_esp4_err()
 *	IPv4 Error handler for ESP protocol
 */
static void nss_ipsecmgr_tunnel_error_esp4(struct sk_buff *skb, uint32_t mtu)
{
	struct nss_ipsecmgr_sa_entry *sa_entry;
	struct nss_ipsecmgr_key key = { {0} };
	struct nss_ipsecmgr_sa_v4 sa = {0};
	struct nss_ipsecmgr_priv *priv;
	struct nss_ipsecmgr_ref *ref;
	struct ip_esp_hdr *esph;
	struct iphdr *iph;

	/*
	 * If the ICMP error is not PMTU then return
	 */
	if (icmp_hdr(skb)->type != ICMP_DEST_UNREACH)
		return;

	if (icmp_hdr(skb)->code != ICMP_FRAG_NEEDED)
		return;

	/*
	 * Skb data now points to the IP header present in the
	 * IMCP payload. It will be of the packet which generated the
	 * PMTU error. Extract the ESP header from the payload.
	 */
	iph = (struct iphdr *)skb->data;
	esph = (struct ip_esp_hdr *)(skb->data + (iph->ihl << 2));

	sa.src_ip = ntohl(iph->saddr);
	sa.dst_ip = ntohl(iph->daddr);
	sa.spi_index = ntohl(esph->spi);

	nss_ipsecmgr_v4_sa2key(&sa, &key);

	/*
	 * Get the SA corresponding to the ESP flow
	 */
	read_lock(&ipsecmgr_ctx->lock);
	ref = nss_ipsecmgr_sa_lookup(&key);
	if (!ref) {
		read_unlock(&ipsecmgr_ctx->lock);
		nss_ipsecmgr_trace("unable to find SA (%px)\n", skb);
		return;
	}

	sa_entry = container_of(ref, struct nss_ipsecmgr_sa_entry, ref);
	priv = sa_entry->priv;
	BUG_ON(!priv);

	atomic_set(&priv->outer_dst_mtu, mtu);
	read_unlock(&ipsecmgr_ctx->lock);

	/*
	 * update new mtu for this flow
	 */
	BUG_ON(!dev_net(skb->dev));
	ipv4_update_pmtu(skb, dev_net(skb->dev), mtu, 0, 0, IPPROTO_ESP, 0);
	return;
}

/*
 * protocol handler for IPv4 ESP
 */
static const struct net_protocol nss_ipsecmgr_proto_esp4 = {
	.handler     = nss_ipsecmgr_tunnel_rx_esp4,
	.err_handler = nss_ipsecmgr_tunnel_error_esp4,
	.netns_ok    = 1,
};

/*
 * nss_ipsecmgr_esp6_rcv()
 *	IPV6 Receive handler for ESP protocol
 */
static int nss_ipsecmgr_tunnel_rx_esp6(struct sk_buff *skb)
{
	/*
	 * TODO:This can potentially receive ESP packets in when
	 * the outer ESP rule is flushed. In which case the DECAP
	 * packets entering linux must be bounced through offload
	 */
	dev_kfree_skb_any(skb);
	return 0;
}

/*
 * nss_ipsecmgr_esp6_err()
 *	IPV6 Error handler for ESP protocol
 */
static void nss_ipsecmgr_tunnel_error_esp6(struct sk_buff *skb, struct inet6_skb_parm *opt, uint8_t type,
								uint8_t code, int32_t offset, uint32_t mtu)
{
	struct nss_ipsecmgr_sa_entry *sa_entry;
	struct nss_ipsecmgr_sa_v6 sa = { {0} };
	struct nss_ipsecmgr_key key = { {0} };
	struct nss_ipsecmgr_priv *priv;
	struct nss_ipsecmgr_ref *ref;
	struct ip_esp_hdr *esph;
	struct ipv6hdr *iph;

	/*
	 * If the ICMP type is not PMTU return
	 */
	if (type != ICMPV6_PKT_TOOBIG)
		return;

	/*
	 * Skb data now points to the IP header present in the
	 * IMCP payload. It will be of the packet which generated the
	 * PMTU error. Extract the ESP header from the payload.
	 */
	esph = (struct ip_esp_hdr *)(skb->data + offset);
	iph = (struct ipv6hdr *)skb->data;

	/*
	 * Get the selectors from IPv6 header. Compose the key and
	 * get the decap SA
	 */
	nss_ipsecmgr_v6addr_ntoh(iph->daddr.s6_addr32, sa.dst_ip);
	nss_ipsecmgr_v6addr_ntoh(iph->saddr.s6_addr32, sa.src_ip);
	sa.spi_index = ntohl(esph->spi);

	nss_ipsecmgr_v6_sa2key(&sa, &key);

	read_lock(&ipsecmgr_ctx->lock);
	ref = nss_ipsecmgr_sa_lookup(&key);
	if (!ref) {
		read_unlock(&ipsecmgr_ctx->lock);
		nss_ipsecmgr_info("%px: unable to find SA\n", skb);
		return;
	}

	sa_entry = container_of(ref, struct nss_ipsecmgr_sa_entry, ref);
	priv = sa_entry->priv;
	BUG_ON(!priv);

	atomic_set(&priv->outer_dst_mtu, ntohl(mtu));
	read_unlock(&ipsecmgr_ctx->lock);

	/*
	 * Update the PMTU
	 */
	BUG_ON(!dev_net(skb->dev));
	ip6_update_pmtu(skb, dev_net(skb->dev), mtu, 0, 0);
	return;
}

/*
 * protocol handler for IPv6 ESP
 */
static struct inet6_protocol nss_ipsecmgr_proto_esp6 = {
	.handler        =       nss_ipsecmgr_tunnel_rx_esp6,
	.err_handler    =       nss_ipsecmgr_tunnel_error_esp6,
	.flags          =       INET6_PROTO_NOPOLICY,
};
#endif

/*
 * nss_ipsecmgr_init()
 *	module init
 */
static int __init nss_ipsecmgr_init(void)
{
	int status;

	if (!nss_cmn_get_nss_enabled()) {
		nss_ipsecmgr_info_always("NSS is not enabled in this platform\n");
		return 0;
	}

	ipsecmgr_ctx = vzalloc(sizeof(struct nss_ipsecmgr_drv));
	if (!ipsecmgr_ctx) {
		nss_ipsecmgr_info_always("Allocating ipsecmgr context failed\n");
		return 0;
	}

	ipsecmgr_ctx->nss_ctx = nss_ipsec_get_context();
	if (!ipsecmgr_ctx->nss_ctx) {
		nss_ipsecmgr_info_always("Getting NSS Context failed\n");
		goto free;
	}

	ipsecmgr_ctx->data_ifnum = nss_ipsec_get_data_interface();
	ipsecmgr_ctx->encap_ifnum = nss_ipsec_get_encap_interface();
	ipsecmgr_ctx->decap_ifnum = nss_ipsec_get_decap_interface();

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 17, 0)
	ipsecmgr_ctx->ndev = alloc_netdev(0, NSS_IPSECMGR_DEFAULT_TUN_NAME, nss_ipsecmgr_dummy_netdevice_setup);
#else
	ipsecmgr_ctx->ndev = alloc_netdev(0, NSS_IPSECMGR_DEFAULT_TUN_NAME, NET_NAME_UNKNOWN, nss_ipsecmgr_dummy_netdevice_setup);
#endif
	if (!ipsecmgr_ctx->ndev) {
		nss_ipsecmgr_info_always("Ipsec: Could not allocate ipsec net_device\n");
		goto free;
	}

	ipsecmgr_ctx->ndev->netdev_ops = &nss_ipsecmgr_ipsec_ndev_ops;

	status = rtnl_is_locked() ? register_netdevice(ipsecmgr_ctx->ndev) : register_netdev(ipsecmgr_ctx->ndev);
	if (status) {
		nss_ipsecmgr_info_always("IPsec: Could not register ipsec net_device\n");
		goto netdev_free;
	}

	rwlock_init(&ipsecmgr_ctx->lock);
	nss_ipsecmgr_init_sa_db(&ipsecmgr_ctx->sa_db);
	nss_ipsecmgr_init_netmask_db(&ipsecmgr_ctx->net_db);
	nss_ipsecmgr_init_flow_db(&ipsecmgr_ctx->flow_db);
	nss_ipsecmgr_init_callback_db(&ipsecmgr_ctx->cb_db);

	nss_ipsec_data_register(ipsecmgr_ctx->data_ifnum, nss_ipsecmgr_tunnel_rx, ipsecmgr_ctx->ndev, 0);
	nss_ipsec_notify_register(ipsecmgr_ctx->encap_ifnum, nss_ipsecmgr_tunnel_notify, NULL);
	nss_ipsec_notify_register(ipsecmgr_ctx->decap_ifnum, nss_ipsecmgr_tunnel_notify, NULL);

	/*
	 * initialize debugfs.
	 */
	ipsecmgr_ctx->dentry = debugfs_create_dir("qca-nss-ipsecmgr", NULL);
	if (!ipsecmgr_ctx->dentry) {
		nss_ipsecmgr_info_always("Creating debug directory failed\n");
		goto unregister_dev;

	}

	ipsecmgr_ctx->stats_dentry = debugfs_create_dir("stats", ipsecmgr_ctx->dentry);
	if (!ipsecmgr_ctx->stats_dentry) {
		debugfs_remove_recursive(ipsecmgr_ctx->dentry);
		nss_ipsecmgr_info("Creating debug directory failed\n");
		goto unregister_dev;

	}

	/*
	 * Create debugfs entries for SA, flow and subnet
	 */
	if (nss_ipsecmgr_init_stats_debugfs(ipsecmgr_ctx->stats_dentry)) {
		nss_ipsecmgr_info("Creating debug tree failed\n");
		debugfs_remove_recursive(ipsecmgr_ctx->dentry);
		goto unregister_dev;

	}

#if defined NSS_IPSECMGR_PMTU_SUPPORT

	/*
	 * Register a ESP protocol handler only when XFRM is not loaded
	 */
	status = inet_add_protocol(&nss_ipsecmgr_proto_esp4, IPPROTO_ESP);
	if (status < 0) {
		nss_ipsecmgr_warn("%px:%d in Registering ESP4 Handler\n",
				ipsecmgr_ctx->nss_ctx, status);
	}

	status = inet6_add_protocol(&nss_ipsecmgr_proto_esp6, IPPROTO_ESP);
	if (status < 0) {
		nss_ipsecmgr_warn("%px:%d in Registering ESP6 Handler\n",
				ipsecmgr_ctx->nss_ctx, status);
	}
#endif

	init_completion(&ipsecmgr_ctx->complete);
	sema_init(&ipsecmgr_ctx->sem, 1);
	atomic_set(&ipsecmgr_ctx->seq_num, 0);

	nss_ipsecmgr_info_always("NSS IPsec manager loaded: %s\n", NSS_CLIENT_BUILD_ID);
	return 0;

unregister_dev:
	rtnl_is_locked() ? unregister_netdevice(ipsecmgr_ctx->ndev) : unregister_netdev(ipsecmgr_ctx->ndev);

netdev_free:
	free_netdev(ipsecmgr_ctx->ndev);

free:
	vfree(ipsecmgr_ctx);
	ipsecmgr_ctx = NULL;

	return 0;
}

/*
 * nss_ipsecmgr_exit()
 * 	module exit
 */
static void __exit nss_ipsecmgr_exit(void)
{
	if (!ipsecmgr_ctx) {
		nss_ipsecmgr_info_always("Invalid ipsecmgr Context\n");
		return;
	}

	if (!ipsecmgr_ctx->nss_ctx) {
		nss_ipsecmgr_info_always("Invalid NSS Context\n");
		vfree(ipsecmgr_ctx);
		ipsecmgr_ctx = NULL;
		return;
	}

	/*
	 * Unregister the callbacks from the HLOS as we are no longer
	 * interested in exception data & async messages
	 */
	nss_ipsec_data_unregister(ipsecmgr_ctx->nss_ctx, ipsecmgr_ctx->data_ifnum);

	nss_ipsec_notify_unregister(ipsecmgr_ctx->nss_ctx, ipsecmgr_ctx->encap_ifnum);
	nss_ipsec_notify_unregister(ipsecmgr_ctx->nss_ctx, ipsecmgr_ctx->decap_ifnum);

	if (ipsecmgr_ctx->ndev) {
		rtnl_is_locked() ? unregister_netdevice(ipsecmgr_ctx->ndev) : unregister_netdev(ipsecmgr_ctx->ndev);
	}

	/*
	 * Remove debugfs directory and entries below that.
	 */
	if (ipsecmgr_ctx->dentry) {
		debugfs_remove_recursive(ipsecmgr_ctx->dentry);
	}

	/*
	 * Free the ipsecmgr ctx
	 */
	vfree(ipsecmgr_ctx);
	ipsecmgr_ctx = NULL;

	nss_ipsecmgr_info_always("NSS IPsec manager unloaded\n");

}

MODULE_LICENSE("Dual BSD/GPL");

module_init(nss_ipsecmgr_init);
module_exit(nss_ipsecmgr_exit);
