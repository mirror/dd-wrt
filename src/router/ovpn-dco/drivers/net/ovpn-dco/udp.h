/* SPDX-License-Identifier: GPL-2.0-only */
/*  OpenVPN data channel accelerator
 *
 *  Copyright (C) 2019-2023 OpenVPN, Inc.
 *
 *  Author:	Antonio Quartulli <antonio@openvpn.net>
 */

#ifndef _NET_OVPN_DCO_UDP_H_
#define _NET_OVPN_DCO_UDP_H_

#include "peer.h"
#include "ovpnstruct.h"

#include <linux/net.h>
#include <linux/skbuff.h>
#include <linux/types.h>
#include <net/sock.h>

int ovpn_udp_socket_attach(struct socket *sock, struct ovpn_struct *ovpn);
void ovpn_udp_socket_detach(struct socket *sock);
void ovpn_udp_send_skb(struct ovpn_struct *ovpn, struct ovpn_peer *peer,
		       struct sk_buff *skb);

#endif /* _NET_OVPN_DCO_UDP_H_ */
