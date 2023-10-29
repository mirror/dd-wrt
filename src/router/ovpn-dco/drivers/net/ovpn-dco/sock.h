/* SPDX-License-Identifier: GPL-2.0-only */
/*  OpenVPN data channel accelerator
 *
 *  Copyright (C) 2020-2023 OpenVPN, Inc.
 *
 *  Author:	James Yonan <james@openvpn.net>
 *		Antonio Quartulli <antonio@openvpn.net>
 */

#ifndef _NET_OVPN_DCO_SOCK_H_
#define _NET_OVPN_DCO_SOCK_H_

#include <linux/net.h>
#include <linux/kref.h>
#include <linux/ptr_ring.h>
#include <net/sock.h>

#include "peer.h"

struct ovpn_struct;

/**
 * struct ovpn_socket - a kernel socket referenced in the ovpn-dco code
 */
struct ovpn_socket {
	union {
		/** @ovpn: the VPN session object owning this socket (UDP only) */
		struct ovpn_struct *ovpn;

		/* TCP only */
		struct {
			/** @peer: the unique peer transmitting over this socket (TCP only) */
			struct ovpn_peer *peer;
			struct ptr_ring recv_ring;
		};
	};

	/** @sock: the kernel socket */
	struct socket *sock;

	/** @refcount: amount of contexts currently referencing this object */
	struct kref refcount;

	/** @rcu: member used to schedule RCU destructor callback */
	struct rcu_head rcu;
};

struct ovpn_struct *ovpn_from_udp_sock(struct sock *sk);

void ovpn_socket_release_kref(struct kref *kref);

static inline void ovpn_socket_put(struct ovpn_socket *sock)
{
	kref_put(&sock->refcount, ovpn_socket_release_kref);
}

struct ovpn_socket *ovpn_socket_new(struct socket *sock, struct ovpn_peer *peer);

#endif /* _NET_OVPN_DCO_SOCK_H_ */
