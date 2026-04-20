/*
 * Copyright (c) 2012, 2015-2017, 2019-2020 The Linux Foundation. All rights reserved.
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
#include <linux/netfilter.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <net/ip.h>
#include <net/inet_ecn.h>
#include <linux/netfilter_bridge.h>
#include <linux/igmp.h>

#include "mc_snooping.h"
#include "mc_private.h"
#include "mc_api.h"
#include "mc_osdep.h"

/* mc_br_port_get
 *	get bridge port by ifindex
 */
static struct net_bridge_port *mc_br_port_get(int ifindex)
{
	struct net_device *dev = NULL;
	struct net_bridge_port *bp = NULL;

	dev = dev_get_by_index(&init_net, ifindex);

	if (dev) {
		bp = os_br_port_get(dev);
		dev_put(dev);
	}
	return bp;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 3, 0))

/* mc_pre_routing_hook
 *	prerouting hook
 */
static unsigned int mc_pre_routing_hook(void *priv,
				    struct sk_buff *skb,
				    const struct nf_hook_state *state)
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 13, 0))
static unsigned int mc_pre_routing_hook(const struct nf_hook_ops *ops, struct sk_buff *skb,
					const struct net_device *in, const struct net_device *out,
					int(*okfn)(struct sk_buff *))
#else
static unsigned int mc_pre_routing_hook(unsigned int hooknum, struct sk_buff *skb,
					const struct net_device *in, const struct net_device *out,
					int(*okfn)(struct sk_buff *))
#endif
{
	struct mc_struct *mc;
	struct ethhdr *eh = eth_hdr(skb);
	struct net_bridge_port *port;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 3, 0))
	struct net_device *in = state->in;
#endif
	u8 dscp;

	rcu_read_lock();

	if (skb->pkt_type != PACKET_HOST ||
		(port = os_br_port_get(in)) == NULL)
		goto out;

	mc = MC_DEV(port->br->dev);
	if (!mc)
		goto out;

	dscp = MC_DSCP(mc->dscp) & (~INET_ECN_MASK);

	switch (ntohs(skb->protocol)) {
	case ETH_P_IP:
		{
			const struct iphdr *iph = ip_hdr(skb);
			if (ipv4_is_multicast(iph->daddr) && (!mc->enable_retag ||
						ipv4_get_dsfield(iph) == dscp)) {
				ip_eth_mc_map(iph->daddr, eh->h_dest);

				if (mc->debug && printk_ratelimit()) {
					MC_PRINT("Decap the group %pI4 back to %pM\n", &iph->daddr, eh->h_dest);
				}
				skb->pkt_type = PACKET_MULTICAST;
			}
		}
		break;
#ifdef MC_SUPPORT_MLD
	case ETH_P_IPV6:
		{
			struct ipv6hdr *iph6 = ipv6_hdr(skb);
			if (ipv6_addr_is_multicast(&iph6->daddr) && (!mc->enable_retag ||
						ipv6_get_dsfield(iph6) == dscp)) {
				ipv6_eth_mc_map(&iph6->daddr, eh->h_dest);

				if (mc->debug && printk_ratelimit()) {
					MC_PRINT("Decap the group %pI6 back to %pM\n", &iph6->daddr, eh->h_dest);
				}
				skb->pkt_type = PACKET_MULTICAST;
			}
		}
		break;
#endif
	}
out:
	rcu_read_unlock();
	return NF_ACCEPT;
}

/* mc_is_ipv4_report_or_leave
 *	if the skb is a ipv4 report or leave message
 */
static bool mc_is_ipv4_report_or_leave(struct sk_buff *skb)
{
	__be32 len, offset;
	struct iphdr *iph;
	struct igmphdr *ih;

	if (unlikely(!pskb_may_pull(skb, sizeof(struct iphdr))))
		return false;

	iph = ip_hdr(skb);
	if (iph->ihl < 5 || iph->version != 4)
		return false;

	if (iph->protocol != IPPROTO_IGMP)
		return false;

	len = ntohs(iph->tot_len);
	if (skb->len < len || len < (iph->ihl*4))
		return false;

	offset = skb_network_offset(skb) + ip_hdrlen(skb);
	if (unlikely(!pskb_may_pull(skb, offset + sizeof(*ih))))
		return false;

	skb_set_transport_header(skb, offset);
	ih = igmp_hdr(skb);

	switch (ih->type) {
	case IGMP_HOST_MEMBERSHIP_REPORT:
	case IGMPV2_HOST_MEMBERSHIP_REPORT:
	case IGMPV3_HOST_MEMBERSHIP_REPORT:
	case IGMP_HOST_LEAVE_MESSAGE:
		return true;
	}

	return false;
}

#ifdef MC_SUPPORT_MLD
/* mc_is_ipv6_report_or_leave
 *	skb is ipv6 report or leave message
 */
static bool mc_is_ipv6_report_or_leave(struct sk_buff *skb)
{
	struct ipv6hdr *ip6h;
	struct icmp6hdr *icmp6h;
	u8 nexthdr;
	unsigned len;
	int offset;

	if (unlikely(!pskb_may_pull(skb, sizeof(*ip6h))))
		return false;

	ip6h = ipv6_hdr(skb);
	/*
	* We're interested in MLD messages only.
	*	- Version is 6
	*	- MLD has always Router Alert hop-by-hop
	*	option
	*	- But we do not support
	*	jumbrograms.
	*/
	if (ip6h->version != 6
		|| ip6h->nexthdr != IPPROTO_HOPOPTS
		|| ip6h->payload_len ==	0)
		return false;
	len = ntohs(ip6h->payload_len);
	if (unlikely(skb->len < len))
		return false;

	nexthdr = ip6h->nexthdr;
	offset = mc_ipv6_skip_exthdr(skb, sizeof(*ip6h), &nexthdr);
	if (offset < 0 || nexthdr != IPPROTO_ICMPV6)
		return false;

	/* Okay, we found ICMPv6 header */
	if (unlikely(!pskb_may_pull(skb, offset + sizeof(*icmp6h))))
		return false;

	skb_set_transport_header(skb, offset);
	icmp6h = icmp6_hdr(skb);
	switch (icmp6h->icmp6_type) {
	case ICMPV6_MGM_REPORT:
	case ICMPV6_MLD2_REPORT:
		return true;
	}
	return false;
}
#endif

/* mc_is_report_or_leave
 *	skb is a report or leave message
 */
static bool mc_is_report_or_leave(struct sk_buff *skb)
{
	switch (ntohs(skb->protocol)) {
	case ETH_P_IP:
		return mc_is_ipv4_report_or_leave(skb);
#ifdef MC_SUPPORT_MLD
	case ETH_P_IPV6:
		return mc_is_ipv6_report_or_leave(skb);
#endif
	}
	return false;

}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 3, 0))

/* mc_forward_hook
 *	forward hook to the linux kernel
 */
static unsigned int mc_forward_hook(void *priv,
				    struct sk_buff *skb,
				    const struct nf_hook_state *state)
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 13, 0))
static unsigned int mc_forward_hook(const struct nf_hook_ops *ops, struct sk_buff *skb,
		const struct net_device *in, const struct net_device *out,
		int(*okfn)(struct sk_buff *))
#else
static unsigned int mc_forward_hook(unsigned int hooknum, struct sk_buff *skb,
		const struct net_device *in, const struct net_device *out,
		int(*okfn)(struct sk_buff *))
#endif
{
	struct mc_struct *mc;
	struct hlist_head *rhead = NULL;
	struct net_bridge_port *port;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 3, 0))
	struct net_device *in = state->in;
	struct net_device *out = state->out;
#endif

	rcu_read_lock();

	if (skb->pkt_type != PACKET_MULTICAST)
		goto accept;

	if ((port = os_br_port_get(in)) == NULL)
		goto accept;

	if ((port = os_br_port_get(out)) == NULL)
		goto accept;

	mc = MC_DEV(port->br->dev);
	if (!mc || !mc->started)
		goto accept;

	if(!mc_is_report_or_leave(skb))
		goto accept;

	/* Report/Leave forward */
	if (mc->rp.type == MC_RTPORT_DEFAULT) {
		if (ntohs(skb->protocol) == ETH_P_IP)
			rhead = &mc->rp.igmp_rlist;
#ifdef MC_SUPPORT_MLD
		else
			rhead = &mc->rp.mld_rlist;
#endif
		if (rhead == NULL)
			goto accept;

		if (!hlist_empty(rhead)) {
			struct mc_querier_entry *qe;
			struct hlist_node *h;

			os_hlist_for_each_entry_rcu(qe, h, rhead, rlist) {
				if (qe->dev == out)
					goto accept;
			}
		}
		goto drop;
	} else if (mc->rp.type == MC_RTPORT_SPECIFY) {
		port = mc_br_port_get(mc->rp.ifindex);
		if (!port || port->dev != out)
			goto drop;
	} else if (mc->rp.type == MC_RTPORT_DROP) {
		goto drop;
	}
accept:
	rcu_read_unlock();
	return NF_ACCEPT;
drop:
	rcu_read_unlock();
	return NF_DROP;
}

static struct nf_hook_ops mc_hook_ops[] __read_mostly = {
	{
		.pf = NFPROTO_BRIDGE,
		.priority = 1,
		.hooknum = NF_BR_PRE_ROUTING,
		.hook = mc_pre_routing_hook,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 4, 0))
		.owner = THIS_MODULE,
#endif
	},
	{
		.pf = NFPROTO_BRIDGE,
		.priority = 1,
		.hooknum = NF_BR_FORWARD,
		.hook = mc_forward_hook,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 4, 0))
		.owner = THIS_MODULE,
#endif
	}
};

/* mc_netfilter_init
 *	function module init
 */
int __init mc_netfilter_init(void)
{
	int ret = 0;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	ret = nf_register_net_hook(&init_net,&mc_hook_ops[0]);
	ret |= nf_register_net_hook(&init_net,&mc_hook_ops[1]);
#else
	ret = nf_register_hook(&mc_hook_ops[0]);
	ret |= nf_register_hook(&mc_hook_ops[1]);
#endif
	return ret;
}

/* mc_netfilter_exit
 *	function module exit
 */
void mc_netfilter_exit(void)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	nf_unregister_net_hook(&init_net,&mc_hook_ops[0]);
	nf_unregister_net_hook(&init_net,&mc_hook_ops[1]);
#else
	nf_unregister_hook(&mc_hook_ops[0]);
	nf_unregister_hook(&mc_hook_ops[1]);
#endif
}

