/* SPDX-License-Identifier: GPL-2.0-only */
/*  OpenVPN data channel accelerator
 *
 *  Copyright (C) 2020-2023 OpenVPN, Inc.
 *
 *  Author:	Antonio Quartulli <antonio@openvpn.net>
 *		James Yonan <james@openvpn.net>
 */

#ifndef _NET_OVPN_DCO_SKB_H_
#define _NET_OVPN_DCO_SKB_H_

#include <linux/in.h>
#include <linux/in6.h>
#include <linux/ip.h>
#include <linux/skbuff.h>
#include <linux/socket.h>
#include <linux/types.h>

#define OVPN_SKB_CB(skb) ((struct ovpn_skb_cb *)&((skb)->cb))

struct ovpn_skb_cb {
	union {
		struct in_addr ipv4;
		struct in6_addr ipv6;
	} local;
	sa_family_t sa_fam;
};

/* Return IP protocol version from skb header.
 * Return 0 if protocol is not IPv4/IPv6 or cannot be read.
 */
static inline __be16 ovpn_ip_check_protocol(struct sk_buff *skb)
{
	__be16 proto = 0;

	/* skb could be non-linear,
	 * make sure IP header is in non-fragmented part
	 */
	if (!pskb_network_may_pull(skb, sizeof(struct iphdr)))
		return 0;

	if (ip_hdr(skb)->version == 4)
		proto = htons(ETH_P_IP);
	else if (ip_hdr(skb)->version == 6)
		proto = htons(ETH_P_IPV6);

	return proto;
}

#endif /* _NET_OVPN_DCO_SKB_H_ */
