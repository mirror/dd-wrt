/*
 * Copyright (c) 2012 The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2, as published by the Free Software Foundation.
*/

#ifndef _MC_NETFILTER_H_
#define _MC_NETFILTER_H_

int __init mc_netfilter_init(void);
void mc_netfilter_exit(void);

#ifndef HYFI_MC_STANDALONE_NF
#include <linux/version.h>
#include <linux/netfilter.h>
#include <linux/netdevice.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
unsigned int mc_pre_routing_hook(void *priv,
                                 struct sk_buff *skb,
                                 const struct nf_hook_state *state);

unsigned int mc_forward_hook(void *priv,
                             struct sk_buff *skb,
                             const struct nf_hook_state *state);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 13, 0))
unsigned int mc_pre_routing_hook(const struct nf_hook_ops *ops, struct sk_buff *skb,
        const struct net_device *in, const struct net_device *out,
        int(*okfn)(struct sk_buff *));

unsigned int mc_forward_hook(const struct nf_hook_ops *ops, struct sk_buff *skb,
        const struct net_device *in, const struct net_device *out,
        int(*okfn)(struct sk_buff *));
#else
unsigned int mc_pre_routing_hook(unsigned int hooknum, struct sk_buff *skb,
        const struct net_device *in, const struct net_device *out,
        int(*okfn)(struct sk_buff *));

unsigned int mc_forward_hook(unsigned int hooknum, struct sk_buff *skb,
        const struct net_device *in, const struct net_device *out,
        int(*okfn)(struct sk_buff *));
#endif
#endif

#endif
