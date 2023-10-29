// SPDX-License-Identifier: GPL-2.0
/*  OpenVPN data channel accelerator
 *
 *  Copyright (C) 2020-2023 OpenVPN, Inc.
 *
 *  Author:	James Yonan <james@openvpn.net>
 *		Antonio Quartulli <antonio@openvpn.net>
 */

#include "main.h"
#include "ovpn.h"
#include "peer.h"
#include "sock.h"
#include "rcu.h"
#include "tcp.h"
#include "udp.h"

#include <net/udp.h>
#include <net/udp_tunnel.h>

/* Finalize release of socket, called after RCU grace period */
static void ovpn_socket_detach(struct socket *sock)
{
	if (!sock)
		return;

	if (sock->sk->sk_protocol == IPPROTO_UDP)
		ovpn_udp_socket_detach(sock);
	else if (sock->sk->sk_protocol == IPPROTO_TCP)
		ovpn_tcp_socket_detach(sock);

	sockfd_put(sock);
}

void ovpn_socket_release_kref(struct kref *kref)
{
	struct ovpn_socket *sock = container_of(kref, struct ovpn_socket, refcount);

	ovpn_socket_detach(sock->sock);
	kfree_rcu(sock, rcu);
}

static bool ovpn_socket_hold(struct ovpn_socket *sock)
{
	return kref_get_unless_zero(&sock->refcount);
}

static struct ovpn_socket *ovpn_socket_get(struct socket *sock)
{
	struct ovpn_socket *ovpn_sock;

	rcu_read_lock();
	ovpn_sock = rcu_dereference_sk_user_data(sock->sk);
	if (!ovpn_socket_hold(ovpn_sock)) {
		pr_warn("%s: found ovpn_socket with ref = 0\n", __func__);
		ovpn_sock = NULL;
	}
	rcu_read_unlock();

	return ovpn_sock;
}

/* Finalize release of socket, called after RCU grace period */
static int ovpn_socket_attach(struct socket *sock, struct ovpn_peer *peer)
{
	int ret = -EOPNOTSUPP;

	if (!sock || !peer)
		return -EINVAL;

	if (sock->sk->sk_protocol == IPPROTO_UDP)
		ret = ovpn_udp_socket_attach(sock, peer->ovpn);
	else if (sock->sk->sk_protocol == IPPROTO_TCP)
		ret = ovpn_tcp_socket_attach(sock, peer);

	return ret;
}

struct ovpn_struct *ovpn_from_udp_sock(struct sock *sk)
{
	struct ovpn_socket *ovpn_sock;

	ovpn_rcu_lockdep_assert_held();

	if (unlikely(READ_ONCE(udp_sk(sk)->encap_type) != UDP_ENCAP_OVPNINUDP))
		return NULL;

	ovpn_sock = rcu_dereference_sk_user_data(sk);
	if (unlikely(!ovpn_sock))
		return NULL;

	/* make sure that sk matches our stored transport socket */
	if (unlikely(!ovpn_sock->sock || sk != ovpn_sock->sock->sk))
		return NULL;

	return ovpn_sock->ovpn;
}

struct ovpn_socket *ovpn_socket_new(struct socket *sock, struct ovpn_peer *peer)
{
	struct ovpn_socket *ovpn_sock;
	int ret;

	ret = ovpn_socket_attach(sock, peer);
	if (ret < 0 && ret != -EALREADY)
		return ERR_PTR(ret);

	/* if this socket is already owned by this interface, just increase the refcounter */
	if (ret == -EALREADY) {
		/* caller is expected to increase the sock refcounter before passing it to this
		 * function. For this reason we drop it if not needed, like when this socket is
		 * already owned.
		 */
		ovpn_sock = ovpn_socket_get(sock);
		sockfd_put(sock);
		return ovpn_sock;
	}

	ovpn_sock = kzalloc(sizeof(*ovpn_sock), GFP_KERNEL);
	if (!ovpn_sock)
		return ERR_PTR(-ENOMEM);

	ovpn_sock->ovpn = peer->ovpn;
	ovpn_sock->sock = sock;
	kref_init(&ovpn_sock->refcount);

	/* TCP sockets are per-peer, therefore they are linked to their unique peer */
	if (sock->sk->sk_protocol == IPPROTO_TCP) {
		ovpn_sock->peer = peer;
		ret = ptr_ring_init(&ovpn_sock->recv_ring, OVPN_QUEUE_LEN, GFP_KERNEL);
		if (ret < 0) {
			netdev_err(peer->ovpn->dev, "%s: cannot allocate TCP recv ring\n",
				   __func__);
			goto err;
		}
	}

	rcu_assign_sk_user_data(sock->sk, ovpn_sock);

	return ovpn_sock;
err:
	kfree(ovpn_sock);
	return ERR_PTR(ret);
}
