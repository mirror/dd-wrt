/*
 * Copyright (c) 2012, 2015 The Linux Foundation. All rights reserved.
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

#ifndef _MC_NETFILTER_H_
#define _MC_NETFILTER_H_

int __init mc_netfilter_init(void);
void mc_netfilter_exit(void);

#include <linux/version.h>
#include <linux/netfilter.h>
#include <linux/netdevice.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 13, 0))
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
