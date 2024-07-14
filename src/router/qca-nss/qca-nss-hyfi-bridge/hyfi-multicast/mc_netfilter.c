/*
 * Copyright (c) 2012-2016 The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2, as published by the Free Software Foundation.
 */

#define DEBUG_LEVEL HYFI_MC_DEBUG_LEVEL

#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <net/ip.h>
#include <net/inet_ecn.h>
#include <linux/netfilter_bridge.h>

#include "mc_snooping.h"
#include "mc_private.h"
#include "mc_api.h"
#include "hyfi_osdep.h"

/*TODO: cleanup for multicast hook */
#ifdef HYFI_MC_STANDALONE_NF
//#define HYFI_MC_STATIC static
#define HYFI_MC_STATIC
#else
#define HYFI_MC_STATIC
#endif

static struct net_bridge_port *mc_br_port_get(int ifindex)
{
    struct net_device *dev = NULL;
    struct net_bridge_port *bp = NULL;

    dev = dev_get_by_index(&init_net, ifindex);

    if (dev) {
		bp = hyfi_br_port_get(dev);
		dev_put(dev);
    }
    return bp;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
HYFI_MC_STATIC unsigned int mc_pre_routing_hook(void *priv,
                                                struct sk_buff *skb,
                                                const struct nf_hook_state *state)
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 13, 0))
HYFI_MC_STATIC unsigned int mc_pre_routing_hook(const struct nf_hook_ops *ops, struct sk_buff *skb,
        const struct net_device *in, const struct net_device *out,
        int(*okfn)(struct sk_buff *))
#else
HYFI_MC_STATIC unsigned int mc_pre_routing_hook(unsigned int hooknum, struct sk_buff *skb,
        const struct net_device *in, const struct net_device *out,
        int(*okfn)(struct sk_buff *))
#endif
{ 
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
    const struct net_device *in = state->in;
#endif
    struct hyfi_net_bridge *hyfi_br = hyfi_bridge_get_by_dev(in);
    struct mc_struct *mc = MC_DEV(hyfi_br);
    struct ethhdr *eh = eth_hdr(skb);
    struct net_bridge_port *port;
    u8 dscp;

    rcu_read_lock();

    if (!mc || skb->pkt_type != PACKET_HOST ||
            (port = hyfi_br_port_get(in)) == NULL)
        goto out;

    dscp = MC_DSCP(mc->dscp) & (~INET_ECN_MASK);

    switch (ntohs(skb->protocol)) {
        case ETH_P_IP:
            {
                const struct iphdr *iph = ip_hdr(skb);
                if (ipv4_is_multicast(iph->daddr) && (!mc->enable_retag || 
                            (ipv4_get_dsfield(iph) == dscp))) {
                    ip_eth_mc_map(iph->daddr, eh->h_dest);

                    if (mc->debug && printk_ratelimit()) {
                        MC_PRINT("Decap the group "MC_IP4_STR" back to "MC_MAC_STR"\n", 
                                 MC_IP4_FMT((u8 *)(&iph->daddr)) ,MC_MAC_FMT(eh->h_dest));
                    }
                    skb->pkt_type = PACKET_MULTICAST;
                }
            }
            break;
#ifdef HYBRID_MC_MLD
        case ETH_P_IPV6:
            {
                struct ipv6hdr *iph6 = ipv6_hdr(skb);
                if (ipv6_addr_is_multicast(&iph6->daddr) && (!mc->enable_retag || 
                            (ipv6_get_dsfield(iph6) == dscp))) {
                    ipv6_eth_mc_map(&iph6->daddr, eh->h_dest);

                    if (mc->debug && printk_ratelimit()) {
                        MC_PRINT("Decap the group "MC_IP6_STR" back to "MC_MAC_STR"\n", 
                                 MC_IP6_FMT((__be16 *)(&iph6->daddr)) ,MC_MAC_FMT(eh->h_dest));
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

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
HYFI_MC_STATIC unsigned int mc_forward_hook(void *priv,
                                            struct sk_buff *skb,
                                            const struct nf_hook_state *state)
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 13, 0))
HYFI_MC_STATIC unsigned int mc_forward_hook(const struct nf_hook_ops *ops, struct sk_buff *skb,
        const struct net_device *in, const struct net_device *out,
        int(*okfn)(struct sk_buff *))
#else
HYFI_MC_STATIC unsigned int mc_forward_hook(unsigned int hooknum, struct sk_buff *skb,
        const struct net_device *in, const struct net_device *out,
        int(*okfn)(struct sk_buff *))
#endif
{ 
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
    const struct net_device *in = state->in;
    const struct net_device *out = state->out;
#endif
    struct hyfi_net_bridge *hyfi_br = hyfi_bridge_get_by_dev(out);
    struct mc_struct *mc = MC_DEV(hyfi_br);
    struct hlist_head *rhead = NULL;
    struct net_bridge_port *port;
    struct mc_mdb_entry *mdb = NULL;

    rcu_read_lock();

    /* Checks are relevant for multicast packets only */
    if ((likely(!is_multicast_ether_addr(eth_hdr(skb)->h_dest))) ||
        (unlikely(is_broadcast_ether_addr(eth_hdr(skb)->h_dest)))) {
        goto accept;
    }

    if ((port = hyfi_br_port_get(in)) == NULL || !mc ){
        goto accept;
    }

    if (!MC_SKB_CB(skb)->igmp) {
        goto accept;
    }

    mdb = MC_SKB_CB(skb)->mdb;

    /* Leave filter */
    if (mdb && MC_SKB_CB(skb)->type == MC_LEAVE && 
            (atomic_read(&mdb->users) > 1))
        goto drop;

    if (MC_SKB_CB(skb)->type != MC_LEAVE && 
            MC_SKB_CB(skb)->type != MC_REPORT)
        goto accept;

    if (!mc->started)
        goto accept;

    /* Report/Leave forward */
    if (mc->rp.type == MC_RTPORT_DEFAULT) {
        if (ntohs(skb->protocol) == ETH_P_IP)
            rhead = &mc->rp.igmp_rlist;
#ifdef HYBRID_MC_MLD
        else
            rhead = &mc->rp.mld_rlist;
#endif
        if (!hlist_empty(rhead)) {
            struct mc_querier_entry *qe;
            struct hlist_node *h;

            os_hlist_for_each_entry_rcu(qe, h, rhead, rlist) {
                if (((struct net_bridge_port *)qe->port)->dev == out)
                    goto accept;
            }
            goto drop;
        }
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

#if defined (HYFI_MC_STANDALONE_NF) && defined (HYFI_MULTICAST_SUPPORT)
static struct nf_hook_ops mc_hook_ops[] __read_mostly =
{
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
#endif

int __init mc_netfilter_init(void)
{
    int ret = 0;
    //The below code are not used to register multicast hooks
    //so they are not used. TODO: cleanup this code.
#if defined (HYFI_MC_STANDALONE_NF) && defined (HYFI_MULTICAST_SUPPORT)
    ret = nf_register_hook(&mc_hook_ops[0]);
    ret |= nf_register_hook(&mc_hook_ops[1]);
#endif
    return ret;
}

void mc_netfilter_exit(void)
{
#if defined (HYFI_MC_STANDALONE_NF) && defined (HYFI_MULTICAST_SUPPORT)
    nf_unregister_hook(&mc_hook_ops[0]);
    nf_unregister_hook(&mc_hook_ops[1]);
#endif
}

