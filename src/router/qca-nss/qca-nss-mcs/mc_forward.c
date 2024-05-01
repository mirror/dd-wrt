/*
 * Copyright (c) 2012, 2015, 2017-2020 The Linux Foundation. All rights reserved.
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

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/err.h>
#include <linux/if_ether.h>
#include <linux/slab.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/rculist.h>
#include <linux/timer.h>
#include <net/inet_ecn.h>

#include "mc_private.h"
#include "mc_snooping.h"
#include "mc_osdep.h"

static void mc_retag(void *iph, __be16 etype, __be32 dscp)
{
	__be32 _dscp = MC_DSCP(dscp);

	if (etype == ETH_P_IP)
		ipv4_copy_dscp(_dscp, iph);
#ifdef MC_SUPPORT_MLD
	else if (etype == ETH_P_IPV6)
		ipv6_copy_dscp(_dscp, iph);
#endif
}

/*
 * mc_stranslate_check_source
 *	check source for givens sources.
 */
static int mc_stranslate_check_source(int pro, void *srcs, int offset, void *iph)
{
	if (offset > MC_DEF_IP6_SIZE)
		return -1;

	if (pro == htons(ETH_P_IP))
		return (*((__be32 *)srcs + offset) == ((struct iphdr *)iph)->saddr);
#ifdef MC_SUPPORT_MLD
	else
		return (!ipv6_addr_cmp((struct in6_addr *)srcs + offset, &((struct ipv6hdr *)iph)->saddr));
#endif
	return 0;
}

/*
 * mc_translate_filter_source
 *	translation filtered by the source
 */
static int mc_translate_filter_source(struct __mc_encaptbl_dev *dev, void *ip, __be16 pro)
{
	int n = 0;

	if (!dev->in_nsrcs && !dev->ex_nsrcs)
		return 0;

	/* Check INCLUDE list first */
	for (n = 0; n < dev->in_nsrcs; n++) {
		if (mc_stranslate_check_source(pro, dev->in_srcs, n, ip))
			break;
	}
	if (n != dev->in_nsrcs)
		return 0;

	/* Then check EXCLUDE list */
	if (dev->ex_nsrcs == MC_DEF_EX_SRCS_INVAL)
		return 1;
	for (n = 0; n < dev->ex_nsrcs; n++) {
		if (mc_stranslate_check_source(pro, dev->ex_srcs, n, ip))
			break;
	}
	if (n == dev->ex_nsrcs)
		return 0;

	return 1;
}

/*
 * mc_translate_hook
 *	apply kernel rules to the skb after translation
 */
static void mc_translate_hook(struct net_bridge *br,
			  struct __mc_encaptbl_dev *encap_dev,
			  struct sk_buff *skb, int forward)
{
	struct net_bridge_port *pdst = NULL;

	memcpy(eth_hdr(skb)->h_dest, encap_dev->mac, ETH_ALEN);
	pdst = mc_bridge_get_dst(os_br_port_get(skb->dev), &skb);

	if (pdst) {
		if (pdst == (struct net_bridge_port *)-1)
			goto out;

		if (forward)
			os_br_forward(pdst, skb);
		else
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
			br_forward(pdst, skb, false, true);
#else
			br_deliver(pdst, skb);
#endif
	}
out:
	if (!pdst || pdst == (struct net_bridge_port *)-1)
		kfree_skb(skb);
}

/*
 * mc_do_translate
 *	do unicast and multicast mac address translation
 */
static int mc_do_translate(struct mc_mdb_entry *mdb, void *iph, struct sk_buff *skb, int forward)
{
	int i;
	struct net_bridge *br = netdev_priv(mdb->mc->dev);
	struct __mc_encaptbl_dev *prev = NULL;
	struct mc_struct *mc = mdb->mc;

	if (!mdb->encap_dev_cnt) {
		kfree_skb(skb);
		return 0;
	}

	if (mc->debug && printk_ratelimit()) {
		if (mdb->group.pro == htons(ETH_P_IP))
			MC_PRINT("Encap the Group %pI4 to following QCA devices:\n", &mdb->group);
		else
			MC_PRINT("Encap the Group %pI6 to following QCA devices:\n", &mdb->group);

		read_lock(&mdb->rwlock);
		for (i = 0; i < mdb->encap_dev_cnt; i++)
			MC_PRINT("	-- %pM\n", mdb->encap_dev[i].mac);
		read_unlock(&mdb->rwlock);
	}

	read_lock(&mdb->rwlock);
	for (i = 0; i < mdb->encap_dev_cnt; i++) {
		if (mc_translate_filter_source(&mdb->encap_dev[i], iph, mdb->group.pro))
			continue;
		if (prev != NULL) {
			struct sk_buff *skb2;

			if ((skb2 = skb_copy(skb, GFP_ATOMIC)) == NULL) {
				read_unlock(&mdb->rwlock);
				kfree_skb(skb);
				return -ENOMEM;
			}
			mc_translate_hook(br, prev, skb2, forward);
		}
		prev = &mdb->encap_dev[i];
	}

	if (prev) {
		if (!(skb = skb_unshare(skb, GFP_ATOMIC))) {
			read_unlock(&mdb->rwlock);
			return -EINVAL;
		}
		mc_translate_hook(br, prev, skb, forward);
		read_unlock(&mdb->rwlock);
		return 0;
	}

	read_unlock(&mdb->rwlock);
	kfree_skb(skb);
	return 0;
}

/*
 * mc_flood_hook -- apply kernel rules to send out the skb
 */
static void mc_flood_hook(__be32 ifindex, struct sk_buff *skb, int forward)
{
	struct net_device *dev;
	struct net_bridge_port *br_port;

	dev = dev_get_by_index(&init_net, ifindex);
	if (!dev) {
		kfree_skb(skb);
		return;
	}

	br_port = os_br_port_get(dev);
	if (!br_port) {
		kfree_skb(skb);
		goto out;
	}

	if (forward)
		os_br_forward(br_port, skb);
	else
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
		br_forward(br_port, skb, false, true);
#else
		br_deliver(br_port, skb);
#endif
out:
	dev_put(dev);
}

/*
 * mc_do_router_flood
 *	Flood the data to the all ports attached to a router
 *	rhead: igmp_rlist or mld_rlist for the ports query exists
 *	skb: the skb to be delivered.
 */
static int mc_do_router_flood(struct mc_mdb_entry *mdb, struct hlist_head *rhead, struct sk_buff *skb)
{
	struct mc_querier_entry *qe;
	struct hlist_node *h;
	struct sk_buff *skb2;
	int i = 0;
	struct net_bridge_port *br_port;

	os_hlist_for_each_entry_rcu(qe, h, rhead, rlist) {

		/* Avoid to send it twice to the same ports
		 * If port already exist in the mdb table list
		 * go to the next query port.
		 */
		if (mdb) {
			for (i = 0; i < mdb->flood_ifcnt; i++) {
				if (mdb->flood_ifindex[i] == qe->ifindex)
					break;
			}

			if (i < mdb->flood_ifcnt)
				continue;
		}
		if ((skb2 = skb_clone(skb, GFP_ATOMIC)) == NULL)
			return -ENOMEM;

		br_port = os_br_port_get(qe->dev);
		if (!br_port) {
			kfree_skb(skb2);
			continue;
		}
		os_br_forward(br_port, skb2);
	}

	return 0;
}

/*
 * mc_output
 *	transmit the skb to all required ports
 */
static int mc_output(struct mc_mdb_entry *mdb, struct sk_buff *skb, int forward)
{
	int i;
	__be32 prev = 0;
	struct sk_buff *skb2;
	struct mc_struct *mc = mdb->mc;

	if (unlikely(mc->debug) && printk_ratelimit()) {
		if (mdb->group.pro == htons(ETH_P_IP))
			MC_PRINT("Flood the Group %pI4 to following interfaces:\n", &mdb->group);
		else
			MC_PRINT("Flood the Group %pI6 to following interfaces:\n",&mdb->group);

		read_lock(&mdb->rwlock);
		for (i = 0; i < mdb->flood_ifcnt; i++) {
			struct net_device *dev = dev_get_by_index(&init_net, mdb->flood_ifindex[i]);

			if (dev) {
				MC_PRINT("	-- %s\n", dev->name);
				dev_put(dev);
			}
		}
		read_unlock(&mdb->rwlock);
	}

	read_lock(&mdb->rwlock);
	for (i = 0; i < mdb->flood_ifcnt; i++) {
		if (prev) {
			skb2 = skb_clone(skb, GFP_ATOMIC);
			if (!skb2) {
				read_unlock(&mdb->rwlock);
				kfree_skb(skb);
				return -ENOMEM;
			}
			mc_flood_hook(prev, skb2, forward);
		}
		prev = mdb->flood_ifindex[i];
	}

	if (prev) {
		mc_flood_hook(prev, skb, forward);
		read_unlock(&mdb->rwlock);
		return 0;
	}

	read_unlock(&mdb->rwlock);
	kfree_skb(skb);
	return 0;
}

/*
 * mc_convert
 *	dispatch the multicast packets
 */
static int mc_convert(struct mc_struct *mc, struct sk_buff *skb, int forward)
{
	__be16 etype = 0;
	struct ethhdr *eh;
	const struct iphdr *ip;

#ifdef MC_SUPPORT_MLD
	const struct ipv6hdr *ip6;
#endif
	struct mc_ip group;
	struct hlist_head *head;
	struct mc_mdb_entry *mdb;
	void *iph = NULL;
	struct sk_buff *skb2 = NULL;
	struct hlist_head *rhead = NULL;
	int is_management;
	int passup = 0;

	eh = eth_hdr(skb);
	etype = ntohs(eh->h_proto);

	if (forward && (MC_SKB_CB(skb)->igmp || MC_SKB_CB(skb)->non_snoop))
		goto out;

	switch (etype) {
	case ETH_P_IP:
		ip = ip_hdr(skb);
		if (!forward && (ip->protocol == IPPROTO_IGMP ||
			mc_find_acl_rule(&mc->igmp_acl, ip->daddr, NULL,
				eh->h_dest, MC_ACL_RULE_NON_SNOOPING)))
			goto out;

		if (!mc->convert_all && ip->protocol != IPPROTO_UDP)
			goto out;

		is_management = mc_find_acl_rule(&mc->igmp_acl, ip->daddr, NULL,
					eh->h_dest, MC_ACL_RULE_MANAGEMENT);

		memset(&group, 0, sizeof group);
		rhead = &mc->rp.igmp_rlist;
		group.u.ip4 = ip->daddr;
		group.pro = htons(ETH_P_IP);
		iph = (void *)ip;
		break;
#ifdef MC_SUPPORT_MLD
	case ETH_P_IPV6:
		ip6 = ipv6_hdr(skb);
		if (!forward && (ip6->nexthdr == IPPROTO_ICMPV6 ||
					mc_find_acl_rule(&mc->mld_acl, 0, (void *)&ip6->daddr,
						eh->h_dest, MC_ACL_RULE_NON_SNOOPING)))
			goto out;

		if (!mc->convert_all && ip6->nexthdr != IPPROTO_UDP)
			goto out;

		is_management = mc_find_acl_rule(&mc->mld_acl, 0, (void *)&ip6->daddr,
					eh->h_dest, MC_ACL_RULE_MANAGEMENT);
		rhead = &mc->rp.mld_rlist;

		memset(&group, 0, sizeof group);
		mc_ipv6_addr_copy(&group.u.ip6, &ip6->daddr);
		group.pro = htons(ETH_P_IPV6);
		iph = (void *)ip6;
		break;
#endif
	default:
		goto out;
	}

	if (forward) {
		if (mc_device_is_router(mc))
			passup = 1;
	}

	head = &mc->hash[mc_group_hash(mc->salt, group.u.ip4)];
	mdb = mc_mdb_find(head, &group);
	if (!mdb || !atomic_read(&mdb->users)) {
		if (mc->forward_policy == MC_POLICY_FLOOD || is_management)
			goto out;

		/* The packets should be forwarded to the all ports attached to
		 * a router
		 */
		if (forward && !hlist_empty(rhead))
			mc_do_router_flood(NULL, rhead, skb);

		if (passup) {
			/*multicast router is enabled, passing up for routing*/
			os_br_pass_frame_up(skb);
		} else
			kfree_skb(skb);
		return 0;
	}

	if (mc->enable_retag)
		mc_retag(iph, etype, mc->dscp);

	if (mdb->flood_ifcnt) {
		if (!(skb2 = skb_clone(skb, GFP_ATOMIC)) ||
				(mc_output(mdb, skb2, forward) < 0)) {
			kfree_skb(skb);
			return 0;
		}
	}

	if (mdb->encap_dev_cnt) {
		if (!(skb2 = skb_clone(skb, GFP_ATOMIC))) {
			kfree_skb(skb);
			return 0;
		}
		mc_do_translate(mdb, iph, skb2, forward);
	}

	/* The packets should be forwarded to the all ports attached to a router
	 */
	if (forward && !hlist_empty(rhead))
		mc_do_router_flood(mdb, rhead, skb);

	if (passup)
		os_br_pass_frame_up(skb);
	else
		kfree_skb(skb);

	return 0;
out:
	return -EINVAL;
}

/*
 * __mc_process
 *	handler of the snooper
 */
static int __mc_process(const struct net_bridge_port *src, struct sk_buff *skb)
{
	struct net_bridge *br;
	struct mc_struct *mc;
	struct net_bridge_fdb_entry *fdb;
	int  ret;

	mc = MC_DEV(BR_INPUT_SKB_CB(skb)->brdev);
	if (!mc)
		return -EINVAL;


	if (!mc->started ||
		is_broadcast_ether_addr(eth_hdr(skb)->h_dest))
		return -EINVAL;

	if (!src) {
		ret = mc_convert(mc, skb, 0);
		return ret;
	}

	br = src->br;
	fdb = os_br_fdb_get(br, eth_hdr(skb)->h_source);
	if (!fdb)
		return -EINVAL;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	if (mc_rcv(mc, skb, fdb->key.addr.addr, src->dev)) {
#else
	if (mc_rcv(mc, skb, fdb->addr.addr, src->dev)) {
#endif
		kfree_skb(skb);
		return 0;
	}

	ret = mc_convert(mc, skb, 1);
	return ret;
}

/*
 * mc_process
 *	callback of br_multicast_handle_hook from the linux kernel
 */
static int mc_process(const struct net_bridge_port *src, struct sk_buff *skb)
{
	int  ret;

	rcu_read_lock();
	ret = __mc_process(src, skb);
	rcu_read_unlock();

	return ret;
}

/*
 * mc_forward_init
 *	register mc process to kernel
 *	need protect by spin_lock
 */
int mc_forward_init(void)
{
	br_multicast_handle_hook_t *br_mc_handler;

	WARN_ON_ONCE(!rcu_read_lock_held());
	br_mc_handler = rcu_dereference(br_multicast_handle_hook);
	if (br_mc_handler) {
		if (br_mc_handler != mc_process) {
			printk("the hook has been occupied by others\n");
			return -1;
		} else
			return 0;
	}
	rcu_assign_pointer(br_multicast_handle_hook, mc_process);

	return 0;
}

/*
 * mc_forward_exit
 *	unregister to br_multicast_handle_hook
 *	need to be protected by the spin_lock
 */
void mc_forward_exit(void)
{
	br_multicast_handle_hook_t *br_mc_handler;

	WARN_ON_ONCE(!rcu_read_lock_held());
	br_mc_handler = rcu_dereference(br_multicast_handle_hook);
	if (!br_mc_handler)
		return;

	if (br_mc_handler != mc_process) {
		printk("bridge multicast handler[%p] being used by others\n", br_mc_handler);
		return;
	}

	/*The last mc need to unregister br_multicast_handle_hook*/
	if (!mc_has_more_instance())
		rcu_assign_pointer(br_multicast_handle_hook, NULL);

}

