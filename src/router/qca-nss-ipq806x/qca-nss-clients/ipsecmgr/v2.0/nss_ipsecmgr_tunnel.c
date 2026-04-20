/*
 **************************************************************************
 * Copyright (c) 2017-2021, The Linux Foundation. All rights reserved.
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

#include <crypto/aead.h>
#include <crypto/skcipher.h>
#include <crypto/internal/hash.h>

#include <nss_api_if.h>
#include <nss_ipsec_cmn.h>
#include <nss_ipsecmgr.h>
#include <nss_cryptoapi.h>

#include "nss_ipsecmgr_ref.h"
#include "nss_ipsecmgr_flow.h"
#include "nss_ipsecmgr_sa.h"
#include "nss_ipsecmgr_ctx.h"
#include "nss_ipsecmgr_tunnel.h"
#include "nss_ipsecmgr_priv.h"

/*
 * nss_ipsecmgr_tunnel_open()
 *	open the tunnel for usage
 */
static int nss_ipsecmgr_tunnel_open(struct net_device *dev)
{
	struct nss_ipsecmgr_tunnel *tun __attribute__((unused)) = netdev_priv(dev);

	netif_start_queue(dev);
	return 0;
}

/*
 * nss_ipsecmgr_tunnel_stop()
 *	stop the IPsec tunnel
 */
static int nss_ipsecmgr_tunnel_stop(struct net_device *dev)
{
	struct nss_ipsecmgr_tunnel *tun __attribute__((unused)) = netdev_priv(dev);

	netif_stop_queue(dev);
	return 0;
}

/*
 * nss_ipsecmgr_tunnel_tx()
 *	tunnel transmit function
 */
static netdev_tx_t nss_ipsecmgr_tunnel_tx(struct sk_buff *skb, struct net_device *dev)
{
	struct nss_ipsecmgr_tunnel *tun = netdev_priv(dev);
	struct nss_ipsec_cmn_flow_tuple f_tuple = {{0}};
	struct nss_ipsec_cmn_mdata_encap *mdata;
	struct nss_ipsec_cmn_sa_tuple *sa_tuple;
	struct nss_ctx_instance *nss_ctx;
	struct nss_ipsecmgr_flow *flow;
	struct nss_ipsecmgr_ctx *ctx;
	struct nss_ipsecmgr_sa *sa;
	bool expand_skb = false;
	uint16_t data_len;
	int nhead, ntail;
	uint32_t ifnum;

	nhead = dev->needed_headroom;
	ntail = dev->needed_tailroom;

	/*
	 * Check if skb is shared
	 */
	if (unlikely(skb_shared(skb))) {
		skb = skb_unshare(skb, in_atomic() ? GFP_ATOMIC : GFP_KERNEL);
		if (!skb)
			return NETDEV_TX_OK;
	}

	/*
	 * Currently IPsec only supports tunnel mode hence non-IP frames are freed.
	 * In transport mode we will encapsulate based on the skb->protocol
	 */
	switch (ip_hdr(skb)->version) {
	case IPVERSION:
		nss_ipsecmgr_flow_ipv4_inner2tuple(ip_hdr(skb), &f_tuple);
		break;

	case 6:
		nss_ipsecmgr_flow_ipv6_inner2tuple(ipv6_hdr(skb), &f_tuple);
		break;

	default:
		nss_ipsecmgr_warn("%px: Non-IP packet for encapsulation", dev);
		goto free;
	}

	/*
	 * Linearize the nonlinear SKB.
	 */
	if (skb_linearize(skb)) {
		nss_ipsecmgr_trace("%s: unable to Linearize SKB\n", dev->name);
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
	 * This packet is ready for NSS transformation. We will now insert the
	 * metadata on top of the IP header and expand data area to cover tailroom for NSS.
	 */
	data_len = skb->len;
	skb_put(skb, ntail);
	mdata = nss_ipsecmgr_tunnel_push_mdata(skb);

	read_lock_bh(&ipsecmgr_drv->lock);

	ctx = nss_ipsecmgr_ctx_find(tun, NSS_IPSEC_CMN_CTX_TYPE_MDATA_INNER);
	if (!ctx) {
		read_unlock_bh(&ipsecmgr_drv->lock);
		nss_ipsecmgr_warn("%px: failed to find inner metdata context for TX\n", tun);
		goto free;
	}

	/*
	 * Check if default SA is configured in tunnel. If found, fill sa_tuple in
	 * metadata to be used for encap.
	 */
	sa = tun->tx_sa;
	if (sa) {
		sa_tuple = &sa->state.tuple;
		goto fill_mdata;
	}

	/*
	 * We search the flow if this has been programmed prior to sending the packets. If flow
	 * is found, we send corresponding sa_tuple as metadata for encap, else drop the packet.
	 */
	flow = nss_ipsecmgr_flow_find(ipsecmgr_drv->flow_db, &f_tuple);
	if (!flow) {
		read_unlock_bh(&ipsecmgr_drv->lock);
		nss_ipsecmgr_trace("%px, failed to find flow for TX", tun);
		goto free;
	}

	sa_tuple = &flow->state.sa;

fill_mdata:
	mdata->flags = 0;
	mdata->seq_num = 0;
	mdata->sa = *sa_tuple;
	mdata->data_len = data_len;

	ifnum = ctx->ifnum;
	nss_ctx = ctx->nss_ctx;
	read_unlock_bh(&ipsecmgr_drv->lock);

	/*
	 * We are not expecting the packet to host; hence scrub it.
	 * In case of exception the packet will be encapsulated which
	 * is different from the one that went for transformation.
	 *
	 * Note: The SKB will be orphaned at this point. Ideally, this should
	 * be charged to NSS pool which allow the accounting of all such buffers.
	 */
	skb_scrub_packet(skb, true);

	/*
	 * Send the packet down;
	 * TODO: Use stop queue and start queue to restart in case of
	 * queue full condition
	 */
	if (nss_ipsec_cmn_tx_buf(nss_ctx, skb, ifnum) != 0) {
		goto free;
	}

	return NETDEV_TX_OK;

free:
	dev_kfree_skb_any(skb);
	return NETDEV_TX_OK;
}

/*
 * nss_ipsecmgr_tunnel_get_stats64()
 *	Get device statistics
 */
static struct rtnl_link_stats64 *nss_ipsecmgr_tunnel_get_stats64(struct net_device *dev, struct rtnl_link_stats64 *stats)
{
	struct nss_ipsecmgr_tunnel *tun = netdev_priv(dev);
	struct list_head *head = &tun->ctx_db;
	struct nss_ipsecmgr_ctx *ctx;

	memset(stats, 0, sizeof(*stats));

	read_lock_bh(&ipsecmgr_drv->lock);
	list_for_each_entry(ctx, head, list) {
		nss_ipsecmgr_ctx_stats_read(ctx, stats);
	}

	read_unlock_bh(&ipsecmgr_drv->lock);
	return stats;
}

/*
 * nss_ipsecmgr_tunnel_stats64()
 *	Sync statistics to linux
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 11, 0))
static struct rtnl_link_stats64 *nss_ipsecmgr_tunnel_stats64(struct net_device *dev, struct rtnl_link_stats64 *stats)
{
	return nss_ipsecmgr_tunnel_get_stats64(dev, stats);
}
#else
static void nss_ipsecmgr_tunnel_stats64(struct net_device *dev, struct rtnl_link_stats64 *stats)
{
	nss_ipsecmgr_tunnel_get_stats64(dev, stats);
}
#endif

/*
 * nss_ipsecmgr_tunnel_mtu_update()
 *	Update tunnel max MTU
 */
static void nss_ipsecmgr_tunnel_mtu_update(struct list_head *head)
{
	struct nss_ipsecmgr_tunnel *tun;
	uint16_t max_mtu = 0;
	bool update_mtu = false;

	write_lock_bh(&ipsecmgr_drv->lock);
	list_for_each_entry(tun, head, list) {
		if (tun->dev->mtu > max_mtu)
			max_mtu = tun->dev->mtu;
	}

	if (ipsecmgr_drv->max_mtu != max_mtu) {
		ipsecmgr_drv->max_mtu = max_mtu;
		update_mtu = true;
	}

	write_unlock_bh(&ipsecmgr_drv->lock);

#ifdef NSS_IPSECMGR_PPE_SUPPORT
	/*
	 * Set PPE inline port's MTU.
	 * TODO: this needs to move to Virtual Port
	 */
	if (ipsecmgr_drv->ipsec_inline && update_mtu) {
		struct nss_ipsecmgr_ctx *ctx;
		uint32_t redir_ifnum;

		read_lock_bh(&ipsecmgr_drv->lock);
		ctx = nss_ipsecmgr_ctx_find(netdev_priv(ipsecmgr_drv->dev), NSS_IPSEC_CMN_CTX_TYPE_REDIR);
		if (!ctx) {
			read_unlock_bh(&ipsecmgr_drv->lock);
			nss_ipsecmgr_warn("Unable to find REDIR interface for %s", ipsecmgr_drv->dev->name);
			return;
		}
		redir_ifnum = ctx->ifnum;
		read_unlock_bh(&ipsecmgr_drv->lock);

		nss_ipsecmgr_info("Updating mtu for %s as %d", ipsecmgr_drv->dev->name, max_mtu);
		nss_ipsec_cmn_ppe_mtu_update(ipsecmgr_drv->nss_ctx, redir_ifnum, max_mtu, max_mtu);
	}
#endif
}

/*
 * nss_ipsecmgr_tunnel_mtu()
 *	Change device MTU
 */
static int nss_ipsecmgr_tunnel_mtu(struct net_device *dev, int mtu)
{
	dev->mtu = mtu;
	nss_ipsecmgr_tunnel_mtu_update(&ipsecmgr_drv->tun_db);
	return 0;
}

/* NSS IPsec tunnel operation */
static const struct net_device_ops ipsecmgr_dev_ops = {
	.ndo_open = nss_ipsecmgr_tunnel_open,
	.ndo_stop = nss_ipsecmgr_tunnel_stop,
	.ndo_start_xmit = nss_ipsecmgr_tunnel_tx,
	.ndo_get_stats64 = nss_ipsecmgr_tunnel_stats64,
	.ndo_change_mtu = nss_ipsecmgr_tunnel_mtu,
};

/*
 * nss_ipsecmgr_tunnel_free()
 *	free an existing IPsec tunnel interface
 */
static void nss_ipsecmgr_tunnel_free(struct net_device *dev)
{
	struct nss_ipsecmgr_tunnel *tun = netdev_priv(dev);
	struct nss_ipsecmgr_ref *ref, *tmp;
	struct list_head free_refs;

	nss_ipsecmgr_info("IPsec tunnel device(%s) freed\n", dev->name);

	INIT_LIST_HEAD(&free_refs);

	/*
	 * Remove context(s) from the tunnel reference tree if it has been
	 * added
	 */
	write_lock_bh(&ipsecmgr_drv->lock);
	if (!nss_ipsecmgr_ref_is_empty(&tun->ref)) {
		nss_ipsecmgr_ref_del(&tun->ref, &free_refs);
	}

	write_unlock_bh(&ipsecmgr_drv->lock);

	list_for_each_entry_safe(ref, tmp, &free_refs, node) {
		ref->free(ref);
	}

	free_netdev(dev);
}

/*
 * nss_ipsecmgr_tunnel_free_work()
 *	Drain all pending refs for free
 */
static void nss_ipsecmgr_tunnel_free_work(struct work_struct *work)
{
	struct nss_ipsecmgr_tunnel *tun = container_of(work, struct nss_ipsecmgr_tunnel, free_work);
	struct nss_ipsecmgr_ref *ref, *tmp;
	struct list_head tmp_head;
	bool is_locked;

	INIT_LIST_HEAD(&tmp_head);

	write_lock_bh(&ipsecmgr_drv->lock);
	list_splice_tail_init(&tun->free_refs, &tmp_head);
	write_unlock_bh(&ipsecmgr_drv->lock);

	is_locked = rtnl_trylock();
	list_for_each_entry_safe(ref, tmp, &tmp_head, node) {
		ref->free(ref);
	}

	if (is_locked)
		rtnl_unlock();
}

/*
 * nss_ipsecmgr_tunnel_free_ref()
 *	Unregister IPsec tunnel interface
 */
static void nss_ipsecmgr_tunnel_free_ref(struct nss_ipsecmgr_ref *ref)
{
	struct nss_ipsecmgr_tunnel *tun = container_of(ref, struct nss_ipsecmgr_tunnel, ref);

	/*
	 * The unregister should start here but the expectation is that the free would
	 * happen when the reference count goes down to '0'
	 */
	if (tun->dev->reg_state == NETREG_REGISTERED) {
		nss_ipsecmgr_tunnel_mtu_update(&ipsecmgr_drv->tun_db);
		rtnl_is_locked() ? unregister_netdevice(tun->dev) : unregister_netdev(tun->dev);
	}
}

/*
 * nss_ipsecmgr_tunnel_del_ref()
 *	Delete IPsec tunnel reference
 */
static void nss_ipsecmgr_tunnel_del_ref(struct nss_ipsecmgr_ref *ref)
{
	struct nss_ipsecmgr_tunnel *tun = container_of(ref, struct nss_ipsecmgr_tunnel, ref);
	list_del(&tun->list);
}

/*
 * nss_ipsecmr_dev_setup()
 *	setup the IPsec tunnel
 */
static void nss_ipsecmgr_tunnel_setup(struct net_device *dev)
{
	dev->addr_len = ETH_ALEN;
	dev->mtu = NSS_IPSECMGR_TUN_MTU(ETH_DATA_LEN);

	dev->hard_header_len = NSS_IPSECMGR_TUN_MAX_HDR_LEN;
	dev->needed_headroom = NSS_IPSECMGR_TUN_HEADROOM;
	dev->needed_tailroom = NSS_IPSECMGR_TUN_TAILROOM;

	dev->type = NSS_IPSEC_CMN_ARPHRD_IPSEC;

	dev->ethtool_ops = NULL;
	dev->header_ops = NULL;
	dev->netdev_ops = &ipsecmgr_dev_ops;

#if (LINUX_VERSION_CODE <= KERNEL_VERSION(4, 11, 8))
	dev->destructor = nss_ipsecmgr_tunnel_free;
#else
	dev->priv_destructor = nss_ipsecmgr_tunnel_free;
#endif

	/*
	 * Get the MAC address from the ethernet device
	 */
	eth_random_addr((u8 *) dev->dev_addr);

	memset(dev->broadcast, 0xff, dev->addr_len);
	memcpy(dev->perm_addr, dev->dev_addr, dev->addr_len);
}

/*
 * nss_ipsecmgr_tunnel_del()
 *	delete an existing IPsec tunnel
 */
void nss_ipsecmgr_tunnel_del(struct net_device *dev)
{
	struct nss_ipsecmgr_tunnel *tun = netdev_priv(dev);

	debugfs_remove_recursive(tun->dentry);

	/*
	 * Flush all associated SA(s) and flow(s) with the tunnel
	 */
	write_lock_bh(&ipsecmgr_drv->lock);
	nss_ipsecmgr_ref_del(&tun->ref, &tun->free_refs);
	write_unlock_bh(&ipsecmgr_drv->lock);

	schedule_work(&tun->free_work);
	flush_work(&tun->free_work);
}
EXPORT_SYMBOL(nss_ipsecmgr_tunnel_del);

/*
 * nss_ipsecmgr_tunnel_add()
 *	add a IPsec pseudo tunnel device
 */
struct net_device *nss_ipsecmgr_tunnel_add(struct nss_ipsecmgr_callback *cb)
{
	struct nss_ipsecmgr_ctx *inner, *mdata_inner;
	struct nss_ipsecmgr_ctx *outer, *mdata_outer;
	struct nss_ipsecmgr_tunnel *tun;
	struct net_device *skb_dev;
	struct net_device *dev;
	int status;

	dev = alloc_netdev(sizeof(*tun), NSS_IPSECMGR_TUN_NAME, NET_NAME_ENUM, nss_ipsecmgr_tunnel_setup);
	if (!dev) {
		nss_ipsecmgr_error("unable to allocate a tunnel device\n");
		return NULL;
	}

	skb_dev = cb->skb_dev;
	tun = netdev_priv(dev);

	tun->dev = dev;
	nss_ipsecmgr_ref_init(&tun->ref, nss_ipsecmgr_tunnel_del_ref, nss_ipsecmgr_tunnel_free_ref);

	INIT_LIST_HEAD(&tun->list);
	INIT_LIST_HEAD(&tun->free_refs);
	INIT_WORK(&tun->free_work, nss_ipsecmgr_tunnel_free_work);

	nss_ipsecmgr_db_init(&tun->ctx_db);

	memcpy(&tun->cb, cb, sizeof(tun->cb));

	/*
	 * Use HLOS netdev if it is loaded in the callback context;
	 * else use the NSS netdev
	 */
	if (!skb_dev)
		tun->cb.skb_dev = dev;

	/*
	 * Inner context allocation
	 */
	inner = nss_ipsecmgr_ctx_alloc(tun,
					NSS_IPSEC_CMN_CTX_TYPE_INNER,
					NSS_DYNAMIC_INTERFACE_TYPE_IPSEC_CMN_INNER,
					nss_ipsecmgr_ctx_rx_inner,
					nss_ipsecmgr_ctx_rx_stats,
					NSS_IPSEC_CMN_FEATURE_INLINE_ACCEL);
	if (!inner) {
		nss_ipsecmgr_warn("%px: failed to allocate context inner\n", tun);
		goto free;
	}

	nss_ipsecmgr_ctx_attach(&tun->ctx_db, inner);

	/*
	 * Inner Metadata context allocation
	 */
	mdata_inner = nss_ipsecmgr_ctx_alloc(tun,
					NSS_IPSEC_CMN_CTX_TYPE_MDATA_INNER,
					NSS_DYNAMIC_INTERFACE_TYPE_IPSEC_CMN_MDATA_INNER,
					nss_ipsecmgr_ctx_rx_inner,
					nss_ipsecmgr_ctx_rx_stats,
					0);
	if (!mdata_inner) {
		nss_ipsecmgr_warn("%px: failed to allocate context metadata inner\n", tun);
		goto free;
	}

	nss_ipsecmgr_ctx_attach(&tun->ctx_db, mdata_inner);
	/*
	 * Outer context allocation
	 */
	outer = nss_ipsecmgr_ctx_alloc(tun,
					NSS_IPSEC_CMN_CTX_TYPE_OUTER,
					NSS_DYNAMIC_INTERFACE_TYPE_IPSEC_CMN_OUTER,
					nss_ipsecmgr_ctx_rx_outer,
					nss_ipsecmgr_ctx_rx_stats,
					NSS_IPSEC_CMN_FEATURE_INLINE_ACCEL);
	if (!outer) {
		nss_ipsecmgr_warn("%px: failed to allocate context outer\n", tun);
		goto free;
	}

	nss_ipsecmgr_ctx_attach(&tun->ctx_db, outer);
	/*
	 * Outer metadata context allocation
	 */
	mdata_outer = nss_ipsecmgr_ctx_alloc(tun,
					NSS_IPSEC_CMN_CTX_TYPE_MDATA_OUTER,
					NSS_DYNAMIC_INTERFACE_TYPE_IPSEC_CMN_MDATA_OUTER,
					nss_ipsecmgr_ctx_rx_outer,
					nss_ipsecmgr_ctx_rx_stats,
					0);
	if (!mdata_outer) {
		nss_ipsecmgr_warn("%px: failed to allocate context metadata outer\n", tun);
		goto free;
	}

	nss_ipsecmgr_ctx_attach(&tun->ctx_db, mdata_outer);

	/*
	 * We need to setup the exception interface number for inner & outer;
	 * The exception interface is used by the NSS to send the packet back
	 * to host when there is a rule miss after the processing (inner or outer).
	 * In case of inner any exception after its processing must come with the
	 * outer interface number. Similarly, for outer we have to do the opposite
	 */
	nss_ipsecmgr_ctx_set_except(inner, outer->ifnum);
	nss_ipsecmgr_ctx_set_except(mdata_inner, outer->ifnum);
	nss_ipsecmgr_ctx_set_except(outer, inner->ifnum);
	nss_ipsecmgr_ctx_set_except(mdata_outer, inner->ifnum);

	/*
	 * We need to setup the sibling interface number for inner & outer;
	 * The sibling interface is used by the NSS to configure SA on sibling.
	 */
	nss_ipsecmgr_ctx_set_sibling(inner, mdata_inner->ifnum);
	nss_ipsecmgr_ctx_set_sibling(outer, mdata_outer->ifnum);

	if (!nss_ipsecmgr_ctx_config(inner)) {
		nss_ipsecmgr_warn("%px: failed to configure inner context\n", tun);
		goto free;
	}

	if (!nss_ipsecmgr_ctx_config(mdata_inner)) {
		nss_ipsecmgr_warn("%px: failed to configure metadata inner context\n", tun);
		goto free;
	}

	if (!nss_ipsecmgr_ctx_config(outer)) {
		nss_ipsecmgr_warn("%px: failed to configure outer context\n", tun);
		goto free;
	}

	if (!nss_ipsecmgr_ctx_config(mdata_outer)) {
		nss_ipsecmgr_warn("%px: failed to configure metadata outer context\n", tun);
		goto free;
	}

	status = rtnl_is_locked() ? register_netdevice(dev) : register_netdev(dev);
	if (status < 0) {
		nss_ipsecmgr_warn("%px: register net dev failed :%s\n", tun, dev->name);
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(4, 11, 8))
		goto free;
#else
		/*
		 * Later kernels invoke the destructor upon failure
		 */
		return NULL;
#endif
	}

	write_lock_bh(&ipsecmgr_drv->lock);
	list_add(&tun->list, &ipsecmgr_drv->tun_db);
	write_unlock_bh(&ipsecmgr_drv->lock);

	nss_ipsecmgr_tunnel_mtu(dev, skb_dev ? skb_dev->mtu : dev->mtu);

	/*
	 * Create debugfs entry for tunnel and its child context(s)
	 */
	tun->dentry = debugfs_create_dir(dev->name, ipsecmgr_drv->dentry);
	if (tun->dentry) {
		debugfs_create_file("inner", S_IRUGO, tun->dentry, inner, &ipsecmgr_ctx_file_ops);
		debugfs_create_file("mdata_inner", S_IRUGO, tun->dentry, mdata_inner, &ipsecmgr_ctx_file_ops);
		debugfs_create_file("outer", S_IRUGO, tun->dentry, outer, &ipsecmgr_ctx_file_ops);
		debugfs_create_file("mdata_outer", S_IRUGO, tun->dentry, mdata_outer, &ipsecmgr_ctx_file_ops);
	}

	return dev;
free:
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(4, 11, 8))
	dev->destructor(dev);
#else
	dev->priv_destructor(dev);
#endif
	return NULL;
}
EXPORT_SYMBOL(nss_ipsecmgr_tunnel_add);
