// SPDX-License-Identifier: GPL-2.0
/*  OpenVPN data channel accelerator
 *
 *  Copyright (C) 2019-2023 OpenVPN, Inc.
 *
 *  Author:	Antonio Quartulli <antonio@openvpn.net>
 */

#include "main.h"
#include "bind.h"
#include "ovpn.h"
#include "ovpnstruct.h"
#include "peer.h"
#include "proto.h"
#include "skb.h"
#include "udp.h"

#include <linux/inetdevice.h>
#include <linux/skbuff.h>
#include <linux/socket.h>
#include <net/addrconf.h>
#include <net/dst_cache.h>
#include <net/route.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 20, 0)
#include <net/ipv6_stubs.h>
#endif
#include <net/udp_tunnel.h>

/**
 * ovpn_udp_encap_recv() - Start processing a received UDP packet.
 * If the first byte of the payload is DATA_V2, the packet is further processed,
 * otherwise it is forwarded to the UDP stack for delivery to user space.
 *
 * @sk: the socket the packet was received on
 * @skb: the sk_buff containing the actual packet
 *
 * Return codes:
 *  0 : we consumed or dropped packet
 * >0 : skb should be passed up to userspace as UDP (packet not consumed)
 * <0 : skb should be resubmitted as proto -N (packet not consumed)
 */
static int ovpn_udp_encap_recv(struct sock *sk, struct sk_buff *skb)
{
	struct ovpn_peer *peer = NULL;
	struct ovpn_struct *ovpn;
	u32 peer_id;
	u8 opcode;
	int ret;

	ovpn = ovpn_from_udp_sock(sk);
	if (unlikely(!ovpn)) {
		net_err_ratelimited("%s: cannot obtain ovpn object from UDP socket\n", __func__);
		goto drop;
	}

	/* Make sure the first 4 bytes of the skb data buffer after the UDP header are accessible.
	 * They are required to fetch the OP code, the key ID and the peer ID.
	 */
	if (unlikely(!pskb_may_pull(skb, sizeof(struct udphdr) + 4))) {
		net_dbg_ratelimited("%s: packet too small\n", __func__);
		goto drop;
	}

	opcode = ovpn_opcode_from_skb(skb, sizeof(struct udphdr));
	if (likely(opcode == OVPN_DATA_V2)) {
		peer_id = ovpn_peer_id_from_skb(skb, sizeof(struct udphdr));
		/* some OpenVPN server implementations send data packets with the peer-id set to
		 * undef. In this case we skip the peer lookup by peer-id and we try with the
		 * transport address
		 */
		if (peer_id != OVPN_PEER_ID_UNDEF) {
			peer = ovpn_peer_lookup_id(ovpn, peer_id);
			if (!peer) {
				net_err_ratelimited("%s: received data from unknown peer (id: %d)\n",
						   __func__, peer_id);
				goto drop;
			}

			/* check if this peer changed it's IP address and update state */
			ovpn_peer_float(peer, skb);
		}
	}

	if (!peer) {
		/* might be a control packet or a data packet with undef peer-id */
		peer = ovpn_peer_lookup_transp_addr(ovpn, skb);
		if (unlikely(!peer)) {
			if (opcode != OVPN_DATA_V2) {
				netdev_dbg(ovpn->dev,
					   "%s: control packet from unknown peer, sending to userspace",
					   __func__);
				return 1;
			}

			netdev_dbg(ovpn->dev,
				   "%s: received data with undef peer-id from unknown source\n",
				   __func__);
			goto drop;
		}
	}

	/* At this point we know the packet is from a configured peer.
	 * DATA_V2 packets are handled in kernel space, the rest goes to user space.
	 *
	 * Return 1 to instruct the stack to let the packet bubble up to userspace
	 */
	if (unlikely(opcode != OVPN_DATA_V2)) {
		ovpn_peer_put(peer);
		return 1;
	}

	/* pop off outer UDP header */
	__skb_pull(skb, sizeof(struct udphdr));

	ret = ovpn_recv(ovpn, peer, skb);
	if (unlikely(ret < 0)) {
		net_err_ratelimited("%s: cannot handle incoming packet from peer %d: %d\n",
				    __func__, peer->id, ret);
		goto drop;
	}

	/* should this be a non DATA_V2 packet, ret will be >0 and this will instruct the UDP
	 * stack to continue processing this packet as usual (i.e. deliver to user space)
	 */
	return ret;

drop:
	if (peer)
		ovpn_peer_put(peer);
	kfree_skb(skb);
	return 0;
}

static int ovpn_udp4_output(struct ovpn_struct *ovpn, struct ovpn_bind *bind,
			    struct dst_cache *cache, struct sock *sk,
			    struct sk_buff *skb)
{
	struct rtable *rt;
	struct flowi4 fl = {
		.saddr = bind->local.ipv4.s_addr,
		.daddr = bind->sa.in4.sin_addr.s_addr,
		.fl4_sport = inet_sk(sk)->inet_sport,
		.fl4_dport = bind->sa.in4.sin_port,
		.flowi4_proto = sk->sk_protocol,
		.flowi4_mark = sk->sk_mark,
	};
	int ret;

	local_bh_disable();
	rt = dst_cache_get_ip4(cache, &fl.saddr);
	if (rt)
		goto transmit;

	if (unlikely(!inet_confirm_addr(sock_net(sk), NULL, 0, fl.saddr, RT_SCOPE_HOST))) {
		/* we may end up here when the cached address is not usable anymore.
		 * In this case we reset address/cache and perform a new look up
		 */
		fl.saddr = 0;
		bind->local.ipv4.s_addr = 0;
		dst_cache_reset(cache);
	}

	rt = ip_route_output_flow(sock_net(sk), &fl, sk);
	if (IS_ERR(rt) && PTR_ERR(rt) == -EINVAL) {
		fl.saddr = 0;
		bind->local.ipv4.s_addr = 0;
		dst_cache_reset(cache);

		rt = ip_route_output_flow(sock_net(sk), &fl, sk);
	}

	if (IS_ERR(rt)) {
		ret = PTR_ERR(rt);
		net_dbg_ratelimited("%s: no route to host %pISpc: %d\n", ovpn->dev->name,
				    &bind->sa.in4, ret);
		goto err;
	}
	dst_cache_set_ip4(cache, &rt->dst, fl.saddr);

transmit:
	udp_tunnel_xmit_skb(rt, sk, skb, fl.saddr, fl.daddr, 0,
			    ip4_dst_hoplimit(&rt->dst), 0, fl.fl4_sport,
			    fl.fl4_dport, false, sk->sk_no_check_tx);
	ret = 0;
err:
	local_bh_enable();
	return ret;
}

#if IS_ENABLED(CONFIG_IPV6)
static int ovpn_udp6_output(struct ovpn_struct *ovpn, struct ovpn_bind *bind,
			    struct dst_cache *cache, struct sock *sk,
			    struct sk_buff *skb)
{
	struct dst_entry *dst;
	int ret;

	struct flowi6 fl = {
		.saddr = bind->local.ipv6,
		.daddr = bind->sa.in6.sin6_addr,
		.fl6_sport = inet_sk(sk)->inet_sport,
		.fl6_dport = bind->sa.in6.sin6_port,
		.flowi6_proto = sk->sk_protocol,
		.flowi6_mark = sk->sk_mark,
		.flowi6_oif = bind->sa.in6.sin6_scope_id,
	};

	local_bh_disable();
	dst = dst_cache_get_ip6(cache, &fl.saddr);
	if (dst)
		goto transmit;

	if (unlikely(!ipv6_chk_addr(sock_net(sk), &fl.saddr, NULL, 0))) {
		/* we may end up here when the cached address is not usable anymore.
		 * In this case we reset address/cache and perform a new look up
		 */
		fl.saddr = in6addr_any;
		bind->local.ipv6 = in6addr_any;
		dst_cache_reset(cache);
	}

	dst = ipv6_stub->ipv6_dst_lookup_flow(sock_net(sk), sk, &fl, NULL);
	if (IS_ERR(dst)) {
		ret = PTR_ERR(dst);
		net_dbg_ratelimited("%s: no route to host %pISpc: %d\n", ovpn->dev->name,
				    &bind->sa.in6, ret);
		goto err;
	}
	dst_cache_set_ip6(cache, dst, &fl.saddr);

transmit:
	udp_tunnel6_xmit_skb(dst, sk, skb, skb->dev, &fl.saddr, &fl.daddr, 0,
			     ip6_dst_hoplimit(dst), 0, fl.fl6_sport,
			     fl.fl6_dport, udp_get_no_check6_tx(sk));
	ret = 0;
err:
	local_bh_enable();
	return ret;
}
#endif

/* Transmit skb utilizing kernel-provided UDP tunneling framework.
 *
 * rcu_read_lock should be held on entry.
 * On return, the skb is consumed.
 */
static int ovpn_udp_output(struct ovpn_struct *ovpn, struct ovpn_bind *bind,
			   struct dst_cache *cache, struct sock *sk,
			   struct sk_buff *skb)
{
	int ret;

	ovpn_rcu_lockdep_assert_held();

	/* set sk to null if skb is already orphaned */
	if (!skb->destructor)
		skb->sk = NULL;

	/* always permit openvpn-created packets to be (outside) fragmented */
	skb->ignore_df = 1;

	switch (bind->sa.in4.sin_family) {
	case AF_INET:
		ret = ovpn_udp4_output(ovpn, bind, cache, sk, skb);
		break;
#if IS_ENABLED(CONFIG_IPV6)
	case AF_INET6:
		ret = ovpn_udp6_output(ovpn, bind, cache, sk, skb);
		break;
#endif
	default:
		ret = -EAFNOSUPPORT;
		break;
	}

	return ret;
}

void ovpn_udp_send_skb(struct ovpn_struct *ovpn, struct ovpn_peer *peer,
		       struct sk_buff *skb)
{
	struct ovpn_bind *bind;
	struct socket *sock;
	int ret = -1;

	skb->dev = ovpn->dev;
	/* no checksum performed at this layer */
	skb->ip_summed = CHECKSUM_NONE;

	/* get socket info */
	sock = peer->sock->sock;
	if (unlikely(!sock)) {
		net_warn_ratelimited("%s: no sock for remote peer\n", __func__);
		goto out;
	}

	rcu_read_lock();
	/* get binding */
	bind = rcu_dereference(peer->bind);
	if (unlikely(!bind)) {
		net_warn_ratelimited("%s: no bind for remote peer\n", __func__);
		goto out_unlock;
	}

	/* crypto layer -> transport (UDP) */
	ret = ovpn_udp_output(ovpn, bind, &peer->dst_cache, sock->sk, skb);

out_unlock:
	rcu_read_unlock();
out:
	if (ret < 0)
		kfree_skb(skb);
}

/* Set UDP encapsulation callbacks */
int ovpn_udp_socket_attach(struct socket *sock, struct ovpn_struct *ovpn)
{
	struct udp_tunnel_sock_cfg cfg = {
		.sk_user_data = ovpn,
		.encap_type = UDP_ENCAP_OVPNINUDP,
		.encap_rcv = ovpn_udp_encap_recv,
	};
	struct ovpn_socket *old_data;

	/* sanity check */
	if (sock->sk->sk_protocol != IPPROTO_UDP) {
		netdev_err(ovpn->dev, "%s: expected UDP socket\n", __func__);
		return -EINVAL;
	}

	/* make sure no pre-existing encapsulation handler exists */
	rcu_read_lock();
	old_data = rcu_dereference_sk_user_data(sock->sk);
	rcu_read_unlock();
	if (old_data) {
		if (old_data->ovpn == ovpn) {
			netdev_dbg(ovpn->dev,
				   "%s: provided socket already owned by this interface\n",
				   __func__);
			return -EALREADY;
		}

		netdev_err(ovpn->dev, "%s: provided socket already taken by other user\n",
			   __func__);
		return -EBUSY;
	}

	setup_udp_tunnel_sock(sock_net(sock->sk), sock, &cfg);

	return 0;
}

/* Detach socket from encapsulation handler and/or other callbacks */
void ovpn_udp_socket_detach(struct socket *sock)
{
	struct udp_tunnel_sock_cfg cfg = { };

	setup_udp_tunnel_sock(sock_net(sock->sk), sock, &cfg);
}
