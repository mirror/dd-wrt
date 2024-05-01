/* Copyright (c) 2021, The Linux Foundation. All rights reserved.
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

#include <linux/version.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/if.h>
#include <linux/list.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <net/xfrm.h>
#include <net/protocol.h>
#include <linux/inetdevice.h>
#include <net/addrconf.h>
#include <linux/netfilter.h>

#include <nss_api_if.h>
#include <nss_ipsec.h>
#include <nss_ipsecmgr.h>

#include "nss_ipsec_xfrm_tunnel.h"
#include "nss_ipsec_xfrm_sa.h"
#include "nss_ipsec_xfrm_flow.h"
#include "nss_ipsec_xfrm.h"

/*
 * nss_ipsec_xfrm_tunnel_free_work()
 *	Tunnel Free work function.
 */
static void nss_ipsec_xfrm_tunnel_free_work(struct work_struct *work)
{
	struct nss_ipsec_xfrm_tunnel *tun = container_of(work, struct nss_ipsec_xfrm_tunnel, free_work);
	struct nss_ipsec_xfrm_drv *drv = tun->drv;

	/*
	 * There should be no default_outer xfrm_state assoicated,
	 * with a tunnel that is getting freed.
	 */
	BUG_ON(atomic_read(&tun->default_spi));
	BUG_ON(!tun->dev);

	nss_ipsec_xfrm_info("%p: free tunnel: nss_dev %s", tun, tun->dev->name);
	nss_ipsecmgr_tunnel_del(tun->dev);

	atomic64_inc(&drv->stats.tun_freed);

	if (atomic_dec_and_test(&drv->num_tunnels)) {
		complete(&drv->complete);
	}

	kfree(tun);
}

/*
 * nss_ipsec_xfrm_tunnel_final()
 *	Called when the last held reference to the tunnel is released.
 */
static void nss_ipsec_xfrm_tunnel_final(struct kref *kref)
{
	struct nss_ipsec_xfrm_tunnel *tun = container_of(kref, struct nss_ipsec_xfrm_tunnel, ref);
	nss_ipsec_xfrm_trace("%p:Tunnel destroyed (%s) \n", tun, tun->dev->name);

	schedule_work(&tun->free_work);
}

/*
 * nss_ipsec_xfrm_tunnel_rx_inner(()
 *	Inner exception callback
 */
static void nss_ipsec_xfrm_tunnel_rx_inner(void *app_data, struct sk_buff *skb)
{
	struct nss_ipsec_xfrm_tunnel *tun = app_data;
	struct nss_ipsec_xfrm_sa *default_sa;
	struct nss_ipsec_xfrm_drv *drv = tun->drv;

	nss_ipsec_xfrm_trace("%p: Packet(%px) exceptioned after decapsulation; ip_version %d\n", tun, skb, ip_hdr(skb)->version);

	/*
	 * No secpath should be exisitng with the exception packet.
	 * XXX Should we add a BUG_ON() here?
	 */
	if (secpath_exists(skb)) {
		nss_ipsec_xfrm_err("%p: Exceptioned packet(%px) already has a SP loaded; dropping\n", tun, skb);
		goto drop;

	}

	/*
	 * sec_path is needed for IN/FWD IPSec policy check. sec_path needs the xfrm_state
	 * that was used to decrypt the packet.
	 * We feed the secpath with default_outer SA. Note that during rekey transition,
	 * this might not be the actual SA that was used to decrypt this packet.
	 * But it should be fine, as the old and new rekey SAs would share the same properties
	 * and mode.
	 */
	default_sa = nss_ipsec_xfrm_sa_ref_by_spi(atomic_read(&tun->default_spi), tun->family);
	if (!default_sa) {
		nss_ipsec_xfrm_err("%p: Exceptioned packet(%px) no default_outer SA assoicated; dropping\n", tun, skb);
		goto drop;
	}

	/*
	 * Allocate and setup the SP in SKB
	 */
	if (!nss_ipsec_xfrm_sa_sp_set(default_sa, skb)) {
		nss_ipsec_xfrm_err("%p: Failed to handle exception(%px) after decap; No SP(%p)\n", tun, skb, default_sa);
		goto err;
	}

	atomic64_inc(&drv->stats.inner_except);
	nss_ipsec_xfrm_sa_deref(default_sa);

	netif_receive_skb(skb);
	return;
err:
	nss_ipsec_xfrm_sa_deref(default_sa);
drop:
	atomic64_inc(&drv->stats.inner_drop);
	dev_kfree_skb_any(skb);
	return;
}

/*
 * nss_ipsec_xfrm_tunnel_rx_outer()
 *	Outer exception callback
 */
static void nss_ipsec_xfrm_tunnel_rx_outer(void *app_data, struct sk_buff *skb)
{
	int (*local_out)(struct net *, struct sock *, struct sk_buff *);
	struct nss_ipsec_xfrm_tunnel *tun = app_data;
	struct nss_ipsec_xfrm_drv *drv = tun->drv;
	struct dst_entry *dst;

	nss_ipsec_xfrm_trace("%p: Packet(%px) exceptioned after encapsulation; ip_version %d\n", tun, skb, ip_hdr(skb)->version);

	if (ip_hdr(skb)->version == IPVERSION) {
		struct iphdr *iph = ip_hdr(skb);

		struct rtable *rt = ip_route_output(&init_net, iph->daddr, iph->saddr, 0, 0);
		if (unlikely(IS_ERR(rt))) {
			nss_ipsec_xfrm_warn("%px: Failed to handle ipv4 exception after encap; No route\n", skb);
			goto drop;
		}

		dst = &rt->dst;
		local_out = ip_local_out;
		IPCB(skb)->flags |= IPSKB_XFRM_TRANSFORMED;
	} else {
		struct ipv6hdr *ip6h = ipv6_hdr(skb);
		struct flowi6 fl6;

		memset(&fl6, 0, sizeof(struct flowi6));
		memcpy(&fl6.daddr, &ip6h->daddr, sizeof(fl6.daddr));
		memcpy(&fl6.saddr, &ip6h->saddr, sizeof(fl6.saddr));

		dst = ip6_route_output(&init_net, NULL, &fl6);
		if (unlikely(IS_ERR(dst))) {
			nss_ipsec_xfrm_warn("%px: Failed to handle ipv6 exception after encap; No route\n", skb);
			goto drop;
		}

		local_out = ip6_local_out;
		IP6CB(skb)->flags |= IP6SKB_XFRM_TRANSFORMED;
	}

	/*
	 * Sets the 'dst' entry for SKB and sends the packet out directly to the physical
	 * device associated with the IPsec tunnel interface.
	 */
	skb_dst_set(skb, dst);
	skb->ip_summed = CHECKSUM_COMPLETE;

	local_out(&init_net, NULL, skb);
	atomic64_inc(&drv->stats.outer_except);
	return;

drop:
	dev_kfree_skb_any(skb);
	atomic64_inc(&drv->stats.outer_drop);
	return;
}

/*
 * nss_ipsec_xfrm_tunnel_proc_event()
 *	NSS xfrm event cb registered with ipsecmgr
 *	Used for syncing per-SA stats.
 */
static void nss_ipsec_xfrm_tunnel_proc_event(void *app_data, struct nss_ipsecmgr_event *event)
{
	struct nss_ipsec_xfrm_tunnel *tun =  app_data;

	/*
	 * We only handle stats event for now
	 */
	if (event->type != NSS_IPSECMGR_EVENT_SA_STATS) {
		return;
	}

	nss_ipsec_xfrm_update_stats(tun->drv, &event->data.stats);
}

/*
 * nss_ipsec_xfrm_tunnel_deref()
 *	Put tunnel ref. Schedules free if the last reference was released.
 *	Caller needs to hold the write lock, as this could be the last reference
 *	that is getting put.
 */
void nss_ipsec_xfrm_tunnel_deref(struct nss_ipsec_xfrm_tunnel *tun)
{
	kref_put(&tun->ref, nss_ipsec_xfrm_tunnel_final);
}

/*
 * nss_ipsec_xfrm_tunnel_ref
 *	Hold tunnel ref.
 */
struct nss_ipsec_xfrm_tunnel * nss_ipsec_xfrm_tunnel_ref(struct nss_ipsec_xfrm_tunnel *tun)
{
	kref_get(&tun->ref);
	return tun;
}

/*
 * nss_ipsec_xfrm_tunnel_match()
 *	Returns true if the supplied IP addreses matches the tunnel's
 *	remote and local addresses
 */
bool nss_ipsec_xfrm_tunnel_match(struct nss_ipsec_xfrm_tunnel *tun, xfrm_address_t *l, xfrm_address_t *r, uint16_t family)
{
	uint8_t status = 0;

	if (family == AF_INET) {
		nss_ipsec_xfrm_trace("%p: family %d remote %pI4 local %pI4", tun, family, &r->a4, &l->a4);
		status += (tun->local.a4 ^ l->a4);
		status += (tun->remote.a4 ^ r->a4);
		return !status;
	}

	nss_ipsec_xfrm_trace( "%p: family %d remote %pI6 local %pI6", tun, family, r->a6, l->a6);
	status += xfrm6_addr_equal(&tun->local, l);
	status += xfrm6_addr_equal(&tun->remote, r);
	return !status;
}

/*
 * nss_ipsec_xfrm_tunnel_dealloc()
 *	Free a tunnel object.
 */
void nss_ipsec_xfrm_tunnel_dealloc(struct nss_ipsec_xfrm_tunnel *tun)
{
	struct nss_ipsec_xfrm_drv *drv = tun->drv;

	atomic64_inc(&drv->stats.tun_dealloced);
	nss_ipsec_xfrm_tunnel_deref(tun);
}

/*
 * nss_ipsec_xfrm_tunnel_alloc()
 *	Creates and initializes a new tunnel obj
 */
struct nss_ipsec_xfrm_tunnel *nss_ipsec_xfrm_tunnel_alloc(struct nss_ipsec_xfrm_drv *drv, xfrm_address_t *local,
								xfrm_address_t *remote, uint16_t family)
{
	struct nss_ipsecmgr_callback ipsec_cb = {0};
	struct nss_ipsec_xfrm_tunnel *tun;
	struct rtable *rt;
	struct rt6_info *rt6;
	struct dst_entry *dst;
	uint8_t ttl_hop_limit;
	uint16_t overhead;
	uint32_t mtu;

	switch (family) {
	case AF_INET:
		 rt = ip_route_output(&init_net, remote->a4, 0, 0, 0);
		 if (IS_ERR(rt)) {
			 nss_ipsec_xfrm_err("%p:Failed to allocate tunnel; No IPv4 dst found\n", drv);
			 return NULL;
		 }

		 dst = &rt->dst;
		 ttl_hop_limit = ip4_dst_hoplimit(dst);
		 overhead = NSS_IPSEC_XFRM_TUNNEL_V4_MAX_OVERHEAD;
		 break;

	case AF_INET6:
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
		rt6 = rt6_lookup(&init_net, &remote->in6, NULL, 0, 0);
#else
		rt6 = rt6_lookup(&init_net, &remote->in6, NULL, 0, 0, 0);
#endif
		if (!rt6) {
			nss_ipsec_xfrm_err("Failed to allocate tunnel; No IPv6 dst found\n");
			return NULL;
		}

		dst = &rt6->dst;
		ttl_hop_limit = ip6_dst_hoplimit(dst);
		overhead = NSS_IPSEC_XFRM_TUNNEL_V6_MAX_OVERHEAD;
		break;

	default:
		nss_ipsec_xfrm_err("%p:Failed to allocate tunnel; bad family type (%u)\n", drv, family);
		return NULL;
	}

	mtu = dst_mtu(dst) - overhead;
	dst_release(dst);

	tun = kzalloc(sizeof(struct nss_ipsec_xfrm_tunnel), GFP_KERNEL);
	if (!tun) {
		nss_ipsec_xfrm_err("Failed to create tunnel; Out of memory\n");
		return NULL;
	}

	ipsec_cb.except_cb = nss_ipsec_xfrm_tunnel_rx_outer;
	ipsec_cb.data_cb = nss_ipsec_xfrm_tunnel_rx_inner;
	ipsec_cb.event_cb = nss_ipsec_xfrm_tunnel_proc_event;
	ipsec_cb.app_data = tun;

	/*
	 * Create IPsec manager tunnel netdevice
	 */
	tun->dev = nss_ipsecmgr_tunnel_add(&ipsec_cb);
	if (!tun->dev) {
		nss_ipsec_xfrm_err("%p: Failed to program tunnel(%p) in IPsec manager\n", drv, tun);
		kfree(tun);
		return NULL;
	}

	tun->ttl = ttl_hop_limit;
	tun->family = family;
	tun->drv = drv;

	/*
	 * Copy the addresses for future comparision
	 */
	memcpy(&tun->remote, remote, sizeof(xfrm_address_t));
	memcpy(&tun->local, local, sizeof(xfrm_address_t));

	/*
	 * Tunnel reference held; will be released in free or deref
	 */
	kref_init(&tun->ref);
	atomic64_inc(&drv->stats.tun_alloced);
	atomic_inc(&drv->num_tunnels);
	INIT_WORK(&tun->free_work, nss_ipsec_xfrm_tunnel_free_work);

	/*
	 * Update the tunnel MTU for NSS netdevice
	 */
	rtnl_lock();
	dev_set_mtu(tun->dev, mtu);
	rtnl_unlock();

	nss_ipsec_xfrm_trace("%p: XFRM offload tunnel created\n", tun);
	return tun;
}
