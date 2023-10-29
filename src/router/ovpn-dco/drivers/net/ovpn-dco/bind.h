/* SPDX-License-Identifier: GPL-2.0-only */
/*  OVPN -- OpenVPN protocol accelerator for Linux
 *  Copyright (C) 2012-2023 OpenVPN, Inc.
 *  All rights reserved.
 *  Author: James Yonan <james@openvpn.net>
 */

#ifndef _NET_OVPN_DCO_OVPNBIND_H_
#define _NET_OVPN_DCO_OVPNBIND_H_

#include "addr.h"
#include "rcu.h"

#include <net/ip.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>

struct ovpn_peer;

struct ovpn_bind {
	struct ovpn_sockaddr sa;  /* remote sockaddr */

	union {
		struct in_addr ipv4;
		struct in6_addr ipv6;
	} local;

	struct rcu_head rcu;
};

static inline bool ovpn_bind_skb_src_match(const struct ovpn_bind *bind, struct sk_buff *skb)
{
	const unsigned short family = skb_protocol_to_family(skb);
	const struct ovpn_sockaddr *sa = &bind->sa;

	if (unlikely(!bind))
		return false;

	if (unlikely(sa->in4.sin_family != family))
		return false;

	switch (family) {
	case AF_INET:
		if (unlikely(sa->in4.sin_addr.s_addr != ip_hdr(skb)->saddr))
			return false;

		if (unlikely(sa->in4.sin_port != udp_hdr(skb)->source))
			return false;
		break;
	case AF_INET6:
		if (unlikely(!ipv6_addr_equal(&sa->in6.sin6_addr, &ipv6_hdr(skb)->saddr)))
			return false;

		if (unlikely(sa->in6.sin6_port != udp_hdr(skb)->source))
			return false;
		break;
	default:
		return false;
	}

	return true;
}

struct ovpn_bind *ovpn_bind_from_sockaddr(const struct sockaddr_storage *sa);
void ovpn_bind_reset(struct ovpn_peer *peer, struct ovpn_bind *bind);

#endif /* _NET_OVPN_DCO_OVPNBIND_H_ */
