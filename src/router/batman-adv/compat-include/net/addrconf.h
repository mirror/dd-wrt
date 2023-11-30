/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_NET_ADDRCONF_H_
#define _NET_BATMAN_ADV_COMPAT_NET_ADDRCONF_H_

#include <linux/version.h>
#include_next <net/addrconf.h>

#if LINUX_VERSION_IS_LESS(5, 1, 0)

static inline int batadv_ipv6_mc_check_mld(struct sk_buff *skb)
{
	return ipv6_mc_check_mld(skb, NULL);
}

#define ipv6_mc_check_mld(skb) \
	batadv_ipv6_mc_check_mld(skb)

#endif /* LINUX_VERSION_IS_LESS(5, 1, 0) */

#if LINUX_VERSION_IS_LESS(5, 13, 0)

static bool batadv_mcast_mla_is_duplicate(u8 *mcast_addr,
					  struct hlist_head *mcast_list);

static inline int
compat_batadv_mcast_mla_softif_get_ipv6(struct net_device *dev,
				 struct hlist_head *mcast_list,
				 struct batadv_mcast_mla_flags *flags,
				 u8 *mcast_addr,
				struct batadv_hw_addr *new,
				struct inet6_dev *in6_dev)

{
	struct ifmcaddr6 *pmc6;
	int ret = 0;

	if (flags->tvlv_flags & BATADV_MCAST_WANT_ALL_IPV6)
		return 0;

	rcu_read_lock();

	in6_dev = __in6_dev_get(dev);
	if (!in6_dev) {
		rcu_read_unlock();
		return 0;
	}

	read_lock_bh(&in6_dev->lock);
	for (pmc6 = in6_dev->mc_list; pmc6; pmc6 = pmc6->next) {
		if (IPV6_ADDR_MC_SCOPE(&pmc6->mca_addr) <
		    IPV6_ADDR_SCOPE_LINKLOCAL)
			continue;

		if (flags->tvlv_flags & BATADV_MCAST_WANT_ALL_UNSNOOPABLES &&
		    ipv6_addr_is_ll_all_nodes(&pmc6->mca_addr))
			continue;

		if (!(flags->tvlv_flags & BATADV_MCAST_WANT_NO_RTR6) &&
		    IPV6_ADDR_MC_SCOPE(&pmc6->mca_addr) >
		    IPV6_ADDR_SCOPE_LINKLOCAL)
			continue;

		ipv6_eth_mc_map(&pmc6->mca_addr, mcast_addr);

		if (batadv_mcast_mla_is_duplicate(mcast_addr, mcast_list))
			continue;

		new = kmalloc(sizeof(*new), GFP_ATOMIC);
		if (!new) {
			ret = -ENOMEM;
			break;
		}

		ether_addr_copy(new->addr, mcast_addr);
		hlist_add_head(&new->list, mcast_list);
		ret++;
	}
	read_unlock_bh(&in6_dev->lock);
	rcu_read_unlock();

	return ret;
}

#define ifmcaddr6 \
		net_device *orig_dev = dev; \
		return compat_batadv_mcast_mla_softif_get_ipv6(orig_dev, \
							       mcast_list, \
							       flags, \
							       mcast_addr, \
							       new = NULL, \
							       in6_dev = NULL); \
	} \
	static inline int \
	__unused_batadv_mcast_mla_softif_get_ipv6(struct net_device *dev, \
					     struct hlist_head *mcast_list, \
					     struct batadv_mcast_mla_flags *flags) \
	{ \
		struct batadv_hw_addr *new; \
		struct inet6_dev *in6_dev; \
		u8 mcast_addr[ETH_ALEN]; \
		struct ifmcaddr6

#endif /* LINUX_VERSION_IS_LESS(5, 13, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_NET_ADDRCONF_H_ */
