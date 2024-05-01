/*
 * Copyright (c) 2013, 2015, 2020 The Linux Foundation. All rights reserved.
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

#ifndef MC_OSDEP_H_
#define MC_OSDEP_H_

#include <linux/version.h>
#include <linux/netfilter_bridge.h>
#include <br_private.h>

int br_pass_frame_up(struct sk_buff *skb, bool promisc);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
static inline int os_br_pass_frame_up(struct sk_buff *skb)
{
	return br_pass_frame_up(skb, false);
}
#else
static inline int os_br_pass_frame_up(struct sk_buff *skb)
{
	struct net_device *indev, *brdev = BR_INPUT_SKB_CB(skb)->brdev;
	struct net_bridge *br = netdev_priv(brdev);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
	struct pcpu_sw_netstats *brstats = this_cpu_ptr(br->stats);
#else
	struct br_cpu_netstats *brstats = this_cpu_ptr(br->stats);
#endif

	u64_stats_update_begin(&brstats->syncp);
	brstats->rx_packets++;
	brstats->rx_bytes += skb->len;
	u64_stats_update_end(&brstats->syncp);

	indev = skb->dev;
	skb->dev = brdev;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0))
	br_drop_fake_rtable(skb);
#endif

	return NF_HOOK(NFPROTO_BRIDGE, NF_BR_LOCAL_IN, skb, indev, NULL,
		   netif_receive_skb);
}
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 3, 0))
#include <linux/moduleparam.h>
#include <linux/export.h>
#include <linux/printk.h>
/* This function should be called with rcu_read_lock held */
static inline struct net_bridge_port *os_br_port_get(const struct net_device *dev)
{
	struct net_bridge_port *br_port;

	if (!dev)
		return NULL;

	br_port = br_port_get_rcu(dev);

	return br_port;
}

static inline void os_br_forward(const struct net_bridge_port *to, struct sk_buff *skb)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	br_forward(to, skb, false, false);
#else
	br_forward(to, skb, NULL);
#endif
}

static inline void mc_ipv6_addr_copy(struct in6_addr *a1, const struct in6_addr *a2)
{
	memcpy(a1, a2, sizeof(struct in6_addr));
}

static inline int mc_ipv6_skip_exthdr(const struct sk_buff *skb, int start, u8 *nexthdrp)
{
	__be16 frag_off;
	return ipv6_skip_exthdr(skb, start, nexthdrp, &frag_off);
}

#else
#include <net/ipv6.h>
#include <linux/kernel.h>

/* This function should be called with rcu_read_lock held */
static inline struct net_bridge_port *os_br_port_get(const struct net_device *dev)
{
	struct net_bridge_port *br_port;

	if (!dev)
		return NULL;

	br_port = rcu_dereference(dev->br_port);

	return br_port;
}

static inline void os_br_forward(const struct net_bridge_port *to,
		struct sk_buff *skb)
{
	br_forward(to, skb);
}

static inline void mc_ipv6_addr_copy(struct in6_addr *a1,
		const struct in6_addr *a2)
{
	ipv6_addr_copy(a1, a2);
}

#if !(defined(CONFIG_IPV6) || defined(CONFIG_IPV6_MODULE))
#define ipv6_skip_exthdr(_x, _y, _z) (-1)
#endif

static inline int mc_ipv6_skip_exthdr(const struct sk_buff *skb, int start,
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
#define os_br_fdb_get(a, b) br_fdb_find_rcu(a, b, 0)
#else
#define os_br_fdb_get(a, b) __br_fdb_get(a, b, 0)
#endif

#else

#define os_hlist_for_each_entry_rcu(tpos, pos, head, member) \
	hlist_for_each_entry_rcu(tpos, pos, head, member)

#define os_hlist_for_each_entry_safe(tpos, pos, n, head, member) \
	(void)pos; \
	hlist_for_each_entry_safe(tpos, pos, n, head, member)

#define os_hlist_for_each_entry(tpos, pos, head, member) \
	hlist_for_each_entry(tpos, pos, head, member)

#define os_br_fdb_get(a, b) __br_fdb_get(a, b)
#endif


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
#define compare_ether_addr(a, b)		 !ether_addr_equal(a, b)
#endif


static inline struct net_bridge_port *mc_bridge_get_dst(const struct net_bridge_port *src,
							struct sk_buff **skb)

{
	const struct net_bridge *br;
	struct net_bridge_fdb_entry *dst;

	if (src) {
		/* Bridged interface */
		br = src->br;
	} else {
		/* Routed interface */
		br = netdev_priv(BR_INPUT_SKB_CB(*skb)->brdev);
	}

	dst = os_br_fdb_get((struct net_bridge *)br, eth_hdr(*skb)->h_dest);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0))
	if (dst && !test_bit(BR_FDB_LOCAL, &dst->flags))
#else
	if (dst && !dst->is_local)
#endif
		return dst->dst;

	return NULL;
}
#endif

