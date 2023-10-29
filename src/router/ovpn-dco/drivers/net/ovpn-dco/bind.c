// SPDX-License-Identifier: GPL-2.0
/*  OpenVPN data channel accelerator
 *
 *  Copyright (C) 2012-2023 OpenVPN, Inc.
 *
 *  Author:	James Yonan <james@openvpn.net>
 *		Antonio Quartulli <antonio@openvpn.net>
 */

#include "ovpn.h"
#include "bind.h"
#include "peer.h"

#include <linux/in.h>
#include <linux/in6.h>
#include <linux/socket.h>
#include <linux/types.h>

/* Given a remote sockaddr, compute the skb hash
 * and get a dst_entry so we can send packets to the remote.
 * Called from process context or softirq (must be indicated with
 * process_context bool).
 */
struct ovpn_bind *ovpn_bind_from_sockaddr(const struct sockaddr_storage *ss)
{
	struct ovpn_bind *bind;
	size_t sa_len;

	if (ss->ss_family == AF_INET)
		sa_len = sizeof(struct sockaddr_in);
	else if (ss->ss_family == AF_INET6)
		sa_len = sizeof(struct sockaddr_in6);
	else
		return ERR_PTR(-EAFNOSUPPORT);

	bind = kzalloc(sizeof(*bind), GFP_ATOMIC);
	if (unlikely(!bind))
		return ERR_PTR(-ENOMEM);

	memcpy(&bind->sa, ss, sa_len);

	return bind;
}

static void ovpn_bind_release_rcu(struct rcu_head *head)
{
	struct ovpn_bind *bind = container_of(head, struct ovpn_bind, rcu);

	kfree(bind);
}

void ovpn_bind_reset(struct ovpn_peer *peer, struct ovpn_bind *new)
{
	struct ovpn_bind *old;

	spin_lock_bh(&peer->lock);
	old = rcu_replace_pointer(peer->bind, new, true);
	spin_unlock_bh(&peer->lock);

	if (old)
		call_rcu(&old->rcu, ovpn_bind_release_rcu);
}
