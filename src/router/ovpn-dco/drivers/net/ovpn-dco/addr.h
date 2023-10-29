/* SPDX-License-Identifier: GPL-2.0-only */
/*  OpenVPN data channel accelerator
 *
 *  Copyright (C) 2020-2023 OpenVPN, Inc.
 *
 *  Author:	James Yonan <james@openvpn.net>
 *		Antonio Quartulli <antonio@openvpn.net>
 */

#ifndef _NET_OVPN_DCO_OVPNADDR_H_
#define _NET_OVPN_DCO_OVPNADDR_H_

#include "crypto.h"

#include <linux/jhash.h>
#include <linux/in.h>
#include <linux/in6.h>
#include <net/ipv6.h>

/* our basic transport layer address */
struct ovpn_sockaddr {
	union {
		struct sockaddr_in in4;
		struct sockaddr_in6 in6;
	};
};

/* Translate skb->protocol value to AF_INET or AF_INET6 */
static inline unsigned short skb_protocol_to_family(const struct sk_buff *skb)
{
	switch (skb->protocol) {
	case htons(ETH_P_IP):
		return AF_INET;
	case htons(ETH_P_IPV6):
		return AF_INET6;
	default:
		return 0;
	}
}

#endif /* _NET_OVPN_DCO_OVPNADDR_H_ */
