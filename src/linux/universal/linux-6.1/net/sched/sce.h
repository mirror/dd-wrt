// SPDX-License-Identifier: GPL-2.0

#ifndef __SCE_H
#define __SCE_H

/*
 *  Support header for the Some Congestion Experienced (SCE) ECT_1 codepoint
 *  ( https://tools.ietf.org/html/draft-morton-taht-tsvwg-sce-00 )
 *
 *  Copyright (C) 2019 Jonathan Morton <chromatix99@gmail.com>
 *  Copyright (C) 2019 David Reed <dpreed@deepplum.com>
 *  Copyright (C) 2019 Michael D. Täht <dave.taht@gmail.com>
 *  Copyright (C) 2019 Pete Heist <pete@heistp.net>
 *  Copyright (C) 2019 Toke Høiland-Jørgensen <toke@toke.dk>
 *  Copyright (C) 2019 Jason Ackley <jason@ackley.net>
*/

static inline int INET_ECN_is_sce(__u8 dsfield)
{
	return (dsfield & INET_ECN_MASK) == INET_ECN_ECT_1;
}

static inline int IP_ECN_set_sce(struct iphdr *iph)
{
	u8 ecn = ipv4_get_dsfield(iph) & INET_ECN_MASK;

	if (ecn != INET_ECN_ECT_0)
		return 0;

	ipv4_change_dsfield(iph, ~INET_ECN_MASK, INET_ECN_ECT_1);

	return 1;
}

static inline int IP6_ECN_set_sce(struct sk_buff *skb, struct ipv6hdr *iph)
{
	u8 ecn = ipv6_get_dsfield(iph) & INET_ECN_MASK;

	if (ecn != INET_ECN_ECT_0)
		return 0;

	ipv6_change_dsfield(iph, ~INET_ECN_MASK, INET_ECN_ECT_1);

	return 1;
}

static inline int INET_ECN_set_sce(struct sk_buff *skb)
{
	switch (skb->protocol) {
	case cpu_to_be16(ETH_P_IP):
		if (skb_network_header(skb) + sizeof(struct iphdr) <=
		    skb_tail_pointer(skb))
			return IP_ECN_set_sce(ip_hdr(skb));
		break;

	case cpu_to_be16(ETH_P_IPV6):
		if (skb_network_header(skb) + sizeof(struct ipv6hdr) <=
		    skb_tail_pointer(skb))
			return IP6_ECN_set_sce(skb, ipv6_hdr(skb));
		break;
	}

	return 0;
}

#endif /* __SCE_H */
