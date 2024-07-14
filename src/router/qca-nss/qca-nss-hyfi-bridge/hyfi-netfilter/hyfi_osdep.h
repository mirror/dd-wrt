/*
 * Copyright (c) 2013-2014, 2016, The Linux Foundation. All rights reserved.
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

#ifndef HYFI_OSDEP_H_
#define HYFI_OSDEP_H_

#include <linux/version.h>
#include <linux/netfilter_bridge.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
static inline int hyfi_handle_local_in(struct net *net, struct sock *sk, struct sk_buff *skb)
{
        return netif_receive_skb(skb);
}
#endif

static inline int hyfi_br_pass_frame_up(struct sk_buff *skb)
{
        struct net_device *indev, *brdev = BR_INPUT_SKB_CB(skb)->brdev;
        struct net_bridge *br = netdev_priv(brdev);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6,1,0))
	struct pcpu_sw_netstats *brstats = this_cpu_ptr(br->dev->tstats);
#elif ((LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)))
        struct pcpu_sw_netstats *brstats = this_cpu_ptr(br->stats);
#else
        struct br_cpu_netstats *brstats = this_cpu_ptr(br->stats);
#endif

        u64_stats_update_begin(&brstats->syncp);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6,1,0))
        u64_stats_inc(&brstats->rx_packets);
        u64_stats_add(&brstats->rx_bytes,skb->len);
#else
        brstats->rx_packets++;
        brstats->rx_bytes += skb->len;
#endif
        u64_stats_update_end(&brstats->syncp);

        indev = skb->dev;
        skb->dev = brdev;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0))
        br_drop_fake_rtable(skb);
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
        return NF_HOOK(NFPROTO_BRIDGE, NF_BR_LOCAL_IN, dev_net(indev), NULL,
                       skb, indev, NULL, hyfi_handle_local_in);
#else
        return NF_HOOK(NFPROTO_BRIDGE, NF_BR_LOCAL_IN, skb, indev, NULL,
                       netif_receive_skb);
#endif
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 3, 0))
#include <linux/moduleparam.h>
#include <linux/export.h>
static inline struct net_bridge_port *hyfi_br_port_get(const struct net_device *dev)
{
	struct net_bridge_port *br_port;

	if (!dev)
		return NULL;

	rcu_read_lock();
	br_port = br_port_get_rcu(dev);
	rcu_read_unlock();

	return br_port;
}
static inline unsigned long hyfi_updated_time_get(const struct net_bridge_fdb_entry *fdb)
{
	return fdb->updated;
}
static inline void hyfi_br_forward(const struct net_bridge_port *to, struct sk_buff *skb)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
       // fasle: packet will be received locally after forwarding
       // false: packet is locally originated
        br_forward(to, skb, false, false);
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
	br_forward(to, skb, NULL);
#endif
}
static inline void hyfi_ipv6_addr_copy(struct in6_addr *a1, const struct in6_addr *a2)
{
	memcpy(a1, a2, sizeof(struct in6_addr));
}
static inline int hyfi_ipv6_skip_exthdr(const struct sk_buff *skb, int start, u8 *nexthdrp)
{
	__be16 frag_off;
	return ipv6_skip_exthdr(skb, start, nexthdrp, &frag_off);
}
#else
#include <net/ipv6.h>
#include <linux/kernel.h>

static inline struct net_bridge_port *hyfi_br_port_get(const struct net_device *dev)
{
	struct net_bridge_port *br_port;

	if (!dev)
		return NULL;

	br_port = rcu_dereference(dev->br_port);

	return br_port;
}
static inline unsigned long hyfi_updated_time_get(
		const struct net_bridge_fdb_entry *fdb)
{
	return fdb->ageing_timer;
}
static inline void hyfi_br_forward(const struct net_bridge_port *to,
		struct sk_buff *skb)
{
	br_forward(to, skb);
}
static inline void hyfi_ipv6_addr_copy(struct in6_addr *a1,
		const struct in6_addr *a2)
{
	ipv6_addr_copy(a1, a2);
}
#if !(defined(CONFIG_IPV6) || defined(CONFIG_IPV6_MODULE))

#if 0 /* Not clear why we need this is IPV6 is disabled? */
static inline int ipv6_ext_hdr(u8 nexthdr)
{
	/*
	 * find out if nexthdr is an extension header or a protocol
	 */
	return ((nexthdr == NEXTHDR_HOP) ||
			(nexthdr == NEXTHDR_ROUTING) ||
			(nexthdr == NEXTHDR_FRAGMENT) ||
			(nexthdr == NEXTHDR_AUTH) ||
			(nexthdr == NEXTHDR_NONE) ||
			(nexthdr == NEXTHDR_DEST));
}
static inline int ipv6_skip_exthdr(const struct sk_buff *skb, int start, u8 *nexthdrp)
{
	u8 nexthdr = *nexthdrp;

	while (ipv6_ext_hdr(nexthdr)) {
		struct ipv6_opt_hdr _hdr, *hp;
		int hdrlen;

		if (nexthdr == NEXTHDR_NONE)
		return -1;

		hp = skb_header_pointer(skb, start, sizeof(_hdr), &_hdr);
		if (hp == NULL)
		return -1;

		if (nexthdr == NEXTHDR_FRAGMENT) {
			__be16 _frag_off, *fp;
			fp = skb_header_pointer(skb,
					start+offsetof(struct frag_hdr,
							frag_off),
					sizeof(_frag_off),
					&_frag_off);
			if (fp == NULL)
			return -1;

			if (ntohs(*fp) & ~0x7)
			break;

			hdrlen = 8;
		} else if (nexthdr == NEXTHDR_AUTH)
		hdrlen = (hp->hdrlen+2)<<2;
		else
		hdrlen = ipv6_optlen(hp);

		nexthdr = hp->nexthdr;
		start += hdrlen;
	}
	*nexthdrp = nexthdr;
	return start;
}
#else
#define ipv6_skip_exthdr( _x, _y, _z ) (-1)
#endif
#endif

static inline int hyfi_ipv6_skip_exthdr(const struct sk_buff *skb, int start,
		u8 *nexthdrp)
{
	return ipv6_skip_exthdr(skb, start, nexthdrp);
}
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))
#define os_hlist_for_each_entry_rcu(tpos, pos, head, member) \
	(void)pos; \
	hlist_for_each_entry_rcu(tpos, head, member)

#define os_hlist_for_each_entry_safe(tpos, pos, n, head, member) \
	(void)pos; \
	hlist_for_each_entry_safe(tpos, n, head, member)

#define os_hlist_for_each_entry(tpos, pos, head, member) \
	(void)pos; \
	hlist_for_each_entry(tpos, head, member)

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
#define os_br_fdb_get(a,b) br_fdb_find_rcu(a,b,0)
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
#define os_br_fdb_get(a,b) __br_fdb_get(a,b,0)
#endif

#else

#define os_hlist_for_each_entry_rcu(tpos, pos, head, member) \
	hlist_for_each_entry_rcu(tpos, pos, head, member)

#define os_hlist_for_each_entry_safe(tpos, pos, n, head, member) \
	(void)pos; \
	hlist_for_each_entry_safe(tpos, pos, n, head, member)

#define os_hlist_for_each_entry(tpos, pos, head, member) \
	hlist_for_each_entry(tpos, pos, head, member)

#define os_br_fdb_get(a,b) __br_fdb_get(a,b)
#endif


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
#define compare_ether_addr(a,b)      !ether_addr_equal(a,b)
#endif

#endif

