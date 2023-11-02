// SPDX-License-Identifier: GPL-2.0
/*  OpenVPN data channel accelerator
 *
 *  Copyright (C) 2020-2023 OpenVPN, Inc.
 *
 *  Author:	James Yonan <james@openvpn.net>
 *		Antonio Quartulli <antonio@openvpn.net>
 */

#include "ovpn.h"
#include "bind.h"
#include "crypto.h"
#include "peer.h"
#include "netlink.h"
#include "tcp.h"

#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/workqueue.h>

static void ovpn_peer_ping(struct timer_list *t)
{
	struct ovpn_peer *peer = from_timer(peer, t, keepalive_xmit);

	netdev_dbg(peer->ovpn->dev, "%s: sending ping to peer %u\n", __func__, peer->id);
	ovpn_keepalive_xmit(peer);
}

static void ovpn_peer_expire(struct timer_list *t)
{
	struct ovpn_peer *peer = from_timer(peer, t, keepalive_recv);

	netdev_dbg(peer->ovpn->dev, "%s: peer %u expired\n", __func__, peer->id);
	ovpn_peer_del(peer, OVPN_DEL_PEER_REASON_EXPIRED);
}

/* Construct a new peer */
static struct ovpn_peer *ovpn_peer_create(struct ovpn_struct *ovpn, u32 id)
{
	struct ovpn_peer *peer;
	int ret;

	/* alloc and init peer object */
	peer = kzalloc(sizeof(*peer), GFP_KERNEL);
	if (!peer)
		return ERR_PTR(-ENOMEM);

	peer->id = id;
	peer->halt = false;
	peer->ovpn = ovpn;

	peer->vpn_addrs.ipv4.s_addr = htonl(INADDR_ANY);
	peer->vpn_addrs.ipv6 = in6addr_any;

	RCU_INIT_POINTER(peer->bind, NULL);
	ovpn_crypto_state_init(&peer->crypto);
	spin_lock_init(&peer->lock);
	kref_init(&peer->refcount);
	ovpn_peer_stats_init(&peer->vpn_stats);
	ovpn_peer_stats_init(&peer->link_stats);

	INIT_WORK(&peer->encrypt_work, ovpn_encrypt_work);
	INIT_WORK(&peer->decrypt_work, ovpn_decrypt_work);

	ret = dst_cache_init(&peer->dst_cache, GFP_KERNEL);
	if (ret < 0) {
		netdev_err(ovpn->dev, "%s: cannot initialize dst cache\n", __func__);
		goto err;
	}

	ret = ptr_ring_init(&peer->tx_ring, OVPN_QUEUE_LEN, GFP_KERNEL);
	if (ret < 0) {
		netdev_err(ovpn->dev, "%s: cannot allocate TX ring\n", __func__);
		goto err_dst_cache;
	}

	ret = ptr_ring_init(&peer->rx_ring, OVPN_QUEUE_LEN, GFP_KERNEL);
	if (ret < 0) {
		netdev_err(ovpn->dev, "%s: cannot allocate RX ring\n", __func__);
		goto err_tx_ring;
	}

	ret = ptr_ring_init(&peer->netif_rx_ring, OVPN_QUEUE_LEN, GFP_KERNEL);
	if (ret < 0) {
		netdev_err(ovpn->dev, "%s: cannot allocate NETIF RX ring\n", __func__);
		goto err_rx_ring;
	}

	/* configure and start NAPI */
	netif_napi_add_tx_weight(ovpn->dev, &peer->napi, ovpn_napi_poll,
				 NAPI_POLL_WEIGHT);
	napi_enable(&peer->napi);

	dev_hold(ovpn->dev);

	timer_setup(&peer->keepalive_xmit, ovpn_peer_ping, 0);
	timer_setup(&peer->keepalive_recv, ovpn_peer_expire, 0);

	return peer;
err_rx_ring:
	ptr_ring_cleanup(&peer->rx_ring, NULL);
err_tx_ring:
	ptr_ring_cleanup(&peer->tx_ring, NULL);
err_dst_cache:
	dst_cache_destroy(&peer->dst_cache);
err:
	kfree(peer);
	return ERR_PTR(ret);
}

/* Reset the ovpn_sockaddr associated with a peer */
static int ovpn_peer_reset_sockaddr(struct ovpn_peer *peer, const struct sockaddr_storage *ss,
				    const u8 *local_ip)
{
	struct ovpn_bind *bind;
	size_t ip_len;

	/* create new ovpn_bind object */
	bind = ovpn_bind_from_sockaddr(ss);
	if (IS_ERR(bind))
		return PTR_ERR(bind);

	if (local_ip) {
		if (ss->ss_family == AF_INET) {
			ip_len = sizeof(struct in_addr);
		} else if (ss->ss_family == AF_INET6) {
			ip_len = sizeof(struct in6_addr);
		} else {
			netdev_warn(peer->ovpn->dev, "%s: invalid family for remote endpoint\n",
				   __func__);
			kfree(bind);
			return -EINVAL;
		}

		memcpy(&bind->local, local_ip, ip_len);
	}

	/* set binding */
	ovpn_bind_reset(peer, bind);

	return 0;
}

void ovpn_peer_float(struct ovpn_peer *peer, struct sk_buff *skb)
{
	struct sockaddr_storage ss;
	const u8 *local_ip = NULL;
	struct sockaddr_in6 *sa6;
	struct sockaddr_in *sa;
	struct ovpn_bind *bind;
	sa_family_t family;

	rcu_read_lock();
	bind = rcu_dereference(peer->bind);
	if (unlikely(!bind))
		goto unlock;

	if (likely(ovpn_bind_skb_src_match(bind, skb)))
		goto unlock;

	family = skb_protocol_to_family(skb);

	if (bind->sa.in4.sin_family == family)
		local_ip = (u8 *)&bind->local;

	switch (family) {
	case AF_INET:
		sa = (struct sockaddr_in *)&ss;
		sa->sin_family = AF_INET;
		sa->sin_addr.s_addr = ip_hdr(skb)->saddr;
		sa->sin_port = udp_hdr(skb)->source;
		break;
	case AF_INET6:
		sa6 = (struct sockaddr_in6 *)&ss;
		sa6->sin6_family = AF_INET6;
		sa6->sin6_addr = ipv6_hdr(skb)->saddr;
		sa6->sin6_port = udp_hdr(skb)->source;
		sa6->sin6_scope_id = ipv6_iface_scope_id(&ipv6_hdr(skb)->saddr, skb->skb_iif);
		break;
	default:
		goto unlock;
	}

	netdev_dbg(peer->ovpn->dev, "%s: peer %d floated to %pIScp", __func__, peer->id, &ss);
	ovpn_peer_reset_sockaddr(peer, (struct sockaddr_storage *)&ss, local_ip);
unlock:
	rcu_read_unlock();
}

static void ovpn_peer_timer_delete_all(struct ovpn_peer *peer)
{
	del_timer_sync(&peer->keepalive_xmit);
	del_timer_sync(&peer->keepalive_recv);
}

static void ovpn_peer_free(struct ovpn_peer *peer)
{
	ovpn_bind_reset(peer, NULL);
	ovpn_peer_timer_delete_all(peer);

	WARN_ON(!__ptr_ring_empty(&peer->tx_ring));
	ptr_ring_cleanup(&peer->tx_ring, NULL);
	WARN_ON(!__ptr_ring_empty(&peer->rx_ring));
	ptr_ring_cleanup(&peer->rx_ring, NULL);
	WARN_ON(!__ptr_ring_empty(&peer->netif_rx_ring));
	ptr_ring_cleanup(&peer->netif_rx_ring, NULL);

	dst_cache_destroy(&peer->dst_cache);

	dev_put(peer->ovpn->dev);

	kfree(peer);
}

static void ovpn_peer_release_rcu(struct rcu_head *head)
{
	struct ovpn_peer *peer = container_of(head, struct ovpn_peer, rcu);

	ovpn_crypto_state_release(&peer->crypto);
	ovpn_peer_free(peer);
}

void ovpn_peer_release(struct ovpn_peer *peer)
{
	napi_disable(&peer->napi);
	netif_napi_del(&peer->napi);

	if (peer->sock)
		ovpn_socket_put(peer->sock);

	call_rcu(&peer->rcu, ovpn_peer_release_rcu);
}

static void ovpn_peer_delete_work(struct work_struct *work)
{
	struct ovpn_peer *peer = container_of(work, struct ovpn_peer,
					      delete_work);
	ovpn_peer_release(peer);
	ovpn_netlink_notify_del_peer(peer);
}

/* Use with kref_put calls, when releasing refcount
 * on ovpn_peer objects.  This method should only
 * be called from process context with config_mutex held.
 */
void ovpn_peer_release_kref(struct kref *kref)
{
	struct ovpn_peer *peer = container_of(kref, struct ovpn_peer, refcount);

	INIT_WORK(&peer->delete_work, ovpn_peer_delete_work);
	queue_work(peer->ovpn->events_wq, &peer->delete_work);
}

struct ovpn_peer *ovpn_peer_new(struct ovpn_struct *ovpn, const struct sockaddr_storage *sa,
				struct socket *sock, u32 id, uint8_t *local_ip)
{
	struct ovpn_peer *peer;
	int ret;

	/* create new peer */
	peer = ovpn_peer_create(ovpn, id);
	if (IS_ERR(peer))
		return peer;

	if (sock->sk->sk_protocol == IPPROTO_UDP) {
		/* a UDP peer must have a remote endpoint */
		if (!sa) {
			ovpn_peer_release(peer);
			return ERR_PTR(-EINVAL);
		}

		/* set peer sockaddr */
		ret = ovpn_peer_reset_sockaddr(peer, sa, local_ip);
		if (ret < 0) {
			ovpn_peer_release(peer);
			return ERR_PTR(ret);
		}
	}

	peer->sock = ovpn_socket_new(sock, peer);
	if (IS_ERR(peer->sock)) {
		peer->sock = NULL;
		ovpn_peer_release(peer);
		return ERR_PTR(-ENOTSOCK);
	}

	return peer;
}

/* Configure keepalive parameters */
void ovpn_peer_keepalive_set(struct ovpn_peer *peer, u32 interval, u32 timeout)
{
	u32 delta;

	netdev_dbg(peer->ovpn->dev,
		   "%s: scheduling keepalive for peer %u: interval=%u timeout=%u\n", __func__,
		   peer->id, interval, timeout);

	peer->keepalive_interval = interval;
	if (interval > 0) {
		delta = msecs_to_jiffies(interval * MSEC_PER_SEC);
		mod_timer(&peer->keepalive_xmit, jiffies + delta);
	} else {
		del_timer(&peer->keepalive_xmit);
	}

	peer->keepalive_timeout = timeout;
	if (timeout) {
		delta = msecs_to_jiffies(timeout * MSEC_PER_SEC);
		mod_timer(&peer->keepalive_recv, jiffies + delta);
	} else {
		del_timer(&peer->keepalive_recv);
	}
}

#define ovpn_peer_index(_tbl, _key, _key_len)		\
	(jhash(_key, _key_len, 0) % HASH_SIZE(_tbl))	\

static struct ovpn_peer *ovpn_peer_lookup_vpn_addr4(struct hlist_head *head, __be32 *addr)
{
	struct ovpn_peer *tmp, *peer = NULL;

	rcu_read_lock();
	hlist_for_each_entry_rcu(tmp, head, hash_entry_addr4) {
		if (*addr != tmp->vpn_addrs.ipv4.s_addr)
			continue;

		if (!ovpn_peer_hold(tmp))
			continue;

		peer = tmp;
		break;
	}
	rcu_read_unlock();

	return peer;
}

static struct ovpn_peer *ovpn_peer_lookup_vpn_addr6(struct hlist_head *head, struct in6_addr *addr)
{
	struct ovpn_peer *tmp, *peer = NULL;
	int i;

	rcu_read_lock();
	hlist_for_each_entry_rcu(tmp, head, hash_entry_addr6) {
		for (i = 0; i < 4; i++) {
			if (addr->s6_addr32[i] != tmp->vpn_addrs.ipv6.s6_addr32[i])
				continue;
		}

		if (!ovpn_peer_hold(tmp))
			continue;

		peer = tmp;
		break;
	}
	rcu_read_unlock();

	return peer;
}

/**
 * ovpn_nexthop4() - looks up the IP of the nexthop for the given destination
 *
 * Looks up in the IPv4 system routing table the IO of the nexthop to be used
 * to reach the destination passed as argument. IF no nexthop can be found, the
 * destination itself is returned as it probably has to be used as nexthop.
 *
 * @ovpn: the private data representing the current VPN session
 * @dst: the destination to be looked up
 *
 * Return the IP of the next hop if found or the dst itself otherwise
 */
static __be32 ovpn_nexthop4(struct ovpn_struct *ovpn, __be32 dst)
{
	struct rtable *rt;
	struct flowi4 fl = {
		.daddr = dst
	};

	rt = ip_route_output_flow(dev_net(ovpn->dev), &fl, NULL);
	if (IS_ERR(rt)) {
		net_dbg_ratelimited("%s: no route to host %pI4\n", __func__, &dst);
		/* if we end up here this packet is probably going to be thrown away later */
		return dst;
	}

	if (!rt->rt_uses_gateway)
		goto out;

	dst = rt->rt_gw4;
out:
	ip_rt_put(rt);
	return dst;
}

/**
 * ovpn_nexthop6() - looks up the IPv6 of the nexthop for the given destination
 *
 * Looks up in the IPv6 system routing table the IO of the nexthop to be used
 * to reach the destination passed as argument. IF no nexthop can be found, the
 * destination itself is returned as it probably has to be used as nexthop.
 *
 * @ovpn: the private data representing the current VPN session
 * @dst: the destination to be looked up
 *
 * Return the IP of the next hop if found or the dst itself otherwise
 */
static struct in6_addr ovpn_nexthop6(struct ovpn_struct *ovpn, struct in6_addr dst)
{
#if IS_ENABLED(CONFIG_IPV6)
	struct rt6_info *rt;
	struct flowi6 fl = {
		.daddr = dst,
	};

	rt = (struct rt6_info *)ipv6_stub->ipv6_dst_lookup_flow(dev_net(ovpn->dev), NULL, &fl,
								NULL);
	if (IS_ERR(rt)) {
		net_dbg_ratelimited("%s: no route to host %pI6\n", __func__, &dst);
		/* if we end up here this packet is probably going to be thrown away later */
		return dst;
	}

	if (!(rt->rt6i_flags & RTF_GATEWAY))
		goto out;

	dst = rt->rt6i_gateway;
out:
	dst_release((struct dst_entry *)rt);
#endif
	return dst;
}

/**
 * ovpn_peer_lookup_vpn_addr() - Lookup peer to send skb to
 *
 * This function takes a tunnel packet and looks up the peer to send it to
 * after encapsulation. The skb is expected to be the in-tunnel packet, without
 * any OpenVPN related header.
 *
 * Assume that the IP header is accessible in the skb data.
 *
 * @ovpn: the private data representing the current VPN session
 * @skb: the skb to extract the destination address from
 *
 * Return the peer if found or NULL otherwise.
 */
struct ovpn_peer *ovpn_peer_lookup_vpn_addr(struct ovpn_struct *ovpn, struct sk_buff *skb,
					    bool use_src)
{
	struct ovpn_peer *tmp, *peer = NULL;
	struct hlist_head *head;
	sa_family_t sa_fam;
	struct in6_addr addr6;
	__be32 addr4;
	u32 index;

	/* in P2P mode, no matter the destination, packets are always sent to the single peer
	 * listening on the other side
	 */
	if (ovpn->mode == OVPN_MODE_P2P) {
		rcu_read_lock();
		tmp = rcu_dereference(ovpn->peer);
		if (likely(tmp && ovpn_peer_hold(tmp)))
			peer = tmp;
		rcu_read_unlock();
		return peer;
	}

	sa_fam = skb_protocol_to_family(skb);

	switch (sa_fam) {
	case AF_INET:
		if (use_src)
			addr4 = ip_hdr(skb)->saddr;
		else
			addr4 = ip_hdr(skb)->daddr;
		addr4 = ovpn_nexthop4(ovpn, addr4);

		index = ovpn_peer_index(ovpn->peers.by_vpn_addr, &addr4, sizeof(addr4));
		head = &ovpn->peers.by_vpn_addr[index];

		peer = ovpn_peer_lookup_vpn_addr4(head, &addr4);
		break;
	case AF_INET6:
		if (use_src)
			addr6 = ipv6_hdr(skb)->saddr;
		else
			addr6 = ipv6_hdr(skb)->daddr;
		addr6 = ovpn_nexthop6(ovpn, addr6);

		index = ovpn_peer_index(ovpn->peers.by_vpn_addr, &addr6, sizeof(addr6));
		head = &ovpn->peers.by_vpn_addr[index];

		peer = ovpn_peer_lookup_vpn_addr6(head, &addr6);
		break;
	}

	return peer;
}

static bool ovpn_peer_transp_match(struct ovpn_peer *peer, struct sockaddr_storage *ss)
{
	struct ovpn_bind *bind = rcu_dereference(peer->bind);
	struct sockaddr_in6 *sa6;
	struct sockaddr_in *sa4;

	if (unlikely(!bind))
		return false;

	if (ss->ss_family != bind->sa.in4.sin_family)
		return false;

	switch (ss->ss_family) {
	case AF_INET:
		sa4 = (struct sockaddr_in *)ss;
		if (sa4->sin_addr.s_addr != bind->sa.in4.sin_addr.s_addr)
			return false;
		if (sa4->sin_port != bind->sa.in4.sin_port)
			return false;
		break;
	case AF_INET6:
		sa6 = (struct sockaddr_in6 *)ss;
		if (memcmp(&sa6->sin6_addr, &bind->sa.in6.sin6_addr, sizeof(struct in6_addr)))
			return false;
		if (sa6->sin6_port != bind->sa.in6.sin6_port)
			return false;
		break;
	default:
		return false;
	}

	return true;
}

static bool ovpn_peer_skb_to_sockaddr(struct sk_buff *skb, struct sockaddr_storage *ss)
{
	struct sockaddr_in6 *sa6;
	struct sockaddr_in *sa4;

	ss->ss_family = skb_protocol_to_family(skb);
	switch (ss->ss_family) {
	case AF_INET:
		sa4 = (struct sockaddr_in *)ss;
		sa4->sin_family = AF_INET;
		sa4->sin_addr.s_addr = ip_hdr(skb)->saddr;
		sa4->sin_port = udp_hdr(skb)->source;
		break;
	case AF_INET6:
		sa6 = (struct sockaddr_in6 *)ss;
		sa6->sin6_family = AF_INET6;
		sa6->sin6_addr = ipv6_hdr(skb)->saddr;
		sa6->sin6_port = udp_hdr(skb)->source;
		break;
	default:
		return false;
	}

	return true;
}

static struct ovpn_peer *ovpn_peer_lookup_transp_addr_p2p(struct ovpn_struct *ovpn,
							  struct sockaddr_storage *ss)
{
	struct ovpn_peer *tmp, *peer = NULL;

	rcu_read_lock();
	tmp = rcu_dereference(ovpn->peer);
	if (likely(tmp && ovpn_peer_transp_match(tmp, ss) && ovpn_peer_hold(tmp)))
		peer = tmp;
	rcu_read_unlock();

	return peer;
}

struct ovpn_peer *ovpn_peer_lookup_transp_addr(struct ovpn_struct *ovpn, struct sk_buff *skb)
{
	struct ovpn_peer *peer = NULL, *tmp;
	struct sockaddr_storage ss = { 0 };
	struct hlist_head *head;
	size_t sa_len;
	bool found;
	u32 index;

	if (unlikely(!ovpn_peer_skb_to_sockaddr(skb, &ss)))
		return NULL;

	if (ovpn->mode == OVPN_MODE_P2P)
		return ovpn_peer_lookup_transp_addr_p2p(ovpn, &ss);

	switch (ss.ss_family) {
	case AF_INET:
		sa_len = sizeof(struct sockaddr_in);
		break;
	case AF_INET6:
		sa_len = sizeof(struct sockaddr_in6);
		break;
	default:
		return NULL;
	}

	index = ovpn_peer_index(ovpn->peers.by_transp_addr, &ss, sa_len);
	head = &ovpn->peers.by_transp_addr[index];

	rcu_read_lock();
	hlist_for_each_entry_rcu(tmp, head, hash_entry_transp_addr) {
		found = ovpn_peer_transp_match(tmp, &ss);
		if (!found)
			continue;

		if (!ovpn_peer_hold(tmp))
			continue;

		peer = tmp;
		break;
	}
	rcu_read_unlock();

	return peer;
}

static struct ovpn_peer *ovpn_peer_lookup_id_p2p(struct ovpn_struct *ovpn, u32 peer_id)
{
	struct ovpn_peer *tmp, *peer = NULL;

	rcu_read_lock();
	tmp = rcu_dereference(ovpn->peer);
	if (likely(tmp && tmp->id == peer_id && ovpn_peer_hold(tmp)))
		peer = tmp;
	rcu_read_unlock();

	return peer;
}

struct ovpn_peer *ovpn_peer_lookup_id(struct ovpn_struct *ovpn, u32 peer_id)
{
	struct ovpn_peer *tmp,  *peer = NULL;
	struct hlist_head *head;
	u32 index;

	if (ovpn->mode == OVPN_MODE_P2P)
		return ovpn_peer_lookup_id_p2p(ovpn, peer_id);

	index = ovpn_peer_index(ovpn->peers.by_id, &peer_id, sizeof(peer_id));
	head = &ovpn->peers.by_id[index];

	rcu_read_lock();
	hlist_for_each_entry_rcu(tmp, head, hash_entry_id) {
		if (tmp->id != peer_id)
			continue;

		if (!ovpn_peer_hold(tmp))
			continue;

		peer = tmp;
		break;
	}
	rcu_read_unlock();

	return peer;
}

void ovpn_peer_update_local_endpoint(struct ovpn_peer *peer, struct sk_buff *skb)
{
	struct ovpn_bind *bind;

	rcu_read_lock();
	bind = rcu_dereference(peer->bind);
	if (unlikely(!bind))
		goto unlock;

	switch (skb_protocol_to_family(skb)) {
	case AF_INET:
		if (unlikely(bind->local.ipv4.s_addr != ip_hdr(skb)->daddr)) {
			netdev_dbg(peer->ovpn->dev,
				   "%s: learning local IPv4 for peer %d (%pI4 -> %pI4)\n", __func__,
				   peer->id, &bind->local.ipv4.s_addr, &ip_hdr(skb)->daddr);
			bind->local.ipv4.s_addr = ip_hdr(skb)->daddr;
		}
		break;
	case AF_INET6:
		if (unlikely(memcmp(&bind->local.ipv6, &ipv6_hdr(skb)->daddr,
				    sizeof(bind->local.ipv6)))) {
			netdev_dbg(peer->ovpn->dev,
				   "%s: learning local IPv6 for peer %d (%pI6c -> %pI6c\n",
				   __func__, peer->id, &bind->local.ipv6, &ipv6_hdr(skb)->daddr);
			bind->local.ipv6 = ipv6_hdr(skb)->daddr;
		}
		break;
	default:
		break;
	}
unlock:
	rcu_read_unlock();
}

static int ovpn_peer_add_mp(struct ovpn_struct *ovpn, struct ovpn_peer *peer)
{
	struct sockaddr_storage sa = { 0 };
	struct sockaddr_in6 *sa6;
	struct sockaddr_in *sa4;
	struct ovpn_bind *bind;
	struct ovpn_peer *tmp;
	size_t salen;
	int ret = 0;
	u32 index;

	spin_lock_bh(&ovpn->peers.lock);
	/* do not add duplicates */
	tmp = ovpn_peer_lookup_id(ovpn, peer->id);
	if (tmp) {
		ovpn_peer_put(tmp);
		ret = -EEXIST;
		goto unlock;
	}

	hlist_del_init_rcu(&peer->hash_entry_transp_addr);
	bind = rcu_dereference_protected(peer->bind, true);
	/* peers connected via UDP have bind == NULL */
	if (bind) {
		switch (bind->sa.in4.sin_family) {
		case AF_INET:
			sa4 = (struct sockaddr_in *)&sa;

			sa4->sin_family = AF_INET;
			sa4->sin_addr.s_addr = bind->sa.in4.sin_addr.s_addr;
			sa4->sin_port = bind->sa.in4.sin_port;
			salen = sizeof(*sa4);
			break;
		case AF_INET6:
			sa6 = (struct sockaddr_in6 *)&sa;

			sa6->sin6_family = AF_INET6;
			sa6->sin6_addr = bind->sa.in6.sin6_addr;
			sa6->sin6_port = bind->sa.in6.sin6_port;
			salen = sizeof(*sa6);
			break;
		default:
			ret = -EPROTONOSUPPORT;
			goto unlock;
		}

		index = ovpn_peer_index(ovpn->peers.by_transp_addr, &sa, salen);
		hlist_add_head_rcu(&peer->hash_entry_transp_addr,
				   &ovpn->peers.by_transp_addr[index]);
	}

	index = ovpn_peer_index(ovpn->peers.by_id, &peer->id, sizeof(peer->id));
	hlist_add_head_rcu(&peer->hash_entry_id, &ovpn->peers.by_id[index]);

	if (peer->vpn_addrs.ipv4.s_addr != htonl(INADDR_ANY)) {
		index = ovpn_peer_index(ovpn->peers.by_vpn_addr, &peer->vpn_addrs.ipv4,
					sizeof(peer->vpn_addrs.ipv4));
		hlist_add_head_rcu(&peer->hash_entry_addr4, &ovpn->peers.by_vpn_addr[index]);
	}

	hlist_del_init_rcu(&peer->hash_entry_addr6);
	if (memcmp(&peer->vpn_addrs.ipv6, &in6addr_any, sizeof(peer->vpn_addrs.ipv6))) {
		index = ovpn_peer_index(ovpn->peers.by_vpn_addr, &peer->vpn_addrs.ipv6,
					sizeof(peer->vpn_addrs.ipv6));
		hlist_add_head_rcu(&peer->hash_entry_addr6, &ovpn->peers.by_vpn_addr[index]);
	}

unlock:
	spin_unlock_bh(&ovpn->peers.lock);

	return ret;
}

static int ovpn_peer_add_p2p(struct ovpn_struct *ovpn, struct ovpn_peer *peer)
{
	struct ovpn_peer *tmp;

	spin_lock_bh(&ovpn->lock);
	/* in p2p mode it is possible to have a single peer only, therefore the
	 * old one is released and substituted by the new one
	 */
	tmp = rcu_dereference(ovpn->peer);
	if (tmp) {
		tmp->delete_reason = OVPN_DEL_PEER_REASON_TEARDOWN;
		ovpn_peer_put(tmp);
	}

	rcu_assign_pointer(ovpn->peer, peer);
	spin_unlock_bh(&ovpn->lock);

	return 0;
}

/* assume refcounter was increased by caller */
int ovpn_peer_add(struct ovpn_struct *ovpn, struct ovpn_peer *peer)
{
	switch (ovpn->mode) {
	case OVPN_MODE_MP:
		return ovpn_peer_add_mp(ovpn, peer);
	case OVPN_MODE_P2P:
		return ovpn_peer_add_p2p(ovpn, peer);
	default:
		return -EOPNOTSUPP;
	}
}

static void ovpn_peer_unhash(struct ovpn_peer *peer, enum ovpn_del_peer_reason reason)
{
	hlist_del_init_rcu(&peer->hash_entry_id);
	hlist_del_init_rcu(&peer->hash_entry_addr4);
	hlist_del_init_rcu(&peer->hash_entry_addr6);
	hlist_del_init_rcu(&peer->hash_entry_transp_addr);

	ovpn_peer_put(peer);
	peer->delete_reason = reason;
}

static int ovpn_peer_del_mp(struct ovpn_peer *peer, enum ovpn_del_peer_reason reason)
{
	struct ovpn_peer *tmp;
	int ret = 0;

	spin_lock_bh(&peer->ovpn->peers.lock);
	tmp = ovpn_peer_lookup_id(peer->ovpn, peer->id);
	if (tmp != peer) {
		ret = -ENOENT;
		goto unlock;
	}
	ovpn_peer_unhash(peer, reason);

unlock:
	spin_unlock_bh(&peer->ovpn->peers.lock);

	if (tmp)
		ovpn_peer_put(tmp);

	return ret;
}

static int ovpn_peer_del_p2p(struct ovpn_peer *peer, enum ovpn_del_peer_reason reason)
{
	struct ovpn_peer *tmp;
	int ret = -ENOENT;

	spin_lock_bh(&peer->ovpn->lock);
	tmp = rcu_dereference(peer->ovpn->peer);
	if (tmp != peer)
		goto unlock;

	ovpn_peer_put(tmp);
	tmp->delete_reason = reason;
	RCU_INIT_POINTER(peer->ovpn->peer, NULL);
	ret = 0;

unlock:
	spin_unlock_bh(&peer->ovpn->lock);

	return ret;
}

void ovpn_peer_release_p2p(struct ovpn_struct *ovpn)
{
	struct ovpn_peer *tmp;

	rcu_read_lock();
	tmp = rcu_dereference(ovpn->peer);
	if (!tmp)
		goto unlock;

	ovpn_peer_del_p2p(tmp, OVPN_DEL_PEER_REASON_TEARDOWN);
unlock:
	rcu_read_unlock();
}

int ovpn_peer_del(struct ovpn_peer *peer, enum ovpn_del_peer_reason reason)
{
	switch (peer->ovpn->mode) {
	case OVPN_MODE_MP:
		return ovpn_peer_del_mp(peer, reason);
	case OVPN_MODE_P2P:
		return ovpn_peer_del_p2p(peer, reason);
	default:
		return -EOPNOTSUPP;
	}
}

void ovpn_peers_free(struct ovpn_struct *ovpn)
{
	struct hlist_node *tmp;
	struct ovpn_peer *peer;
	int bkt;

	spin_lock_bh(&ovpn->peers.lock);
	hash_for_each_safe(ovpn->peers.by_id, bkt, tmp, peer, hash_entry_id)
		ovpn_peer_unhash(peer, OVPN_DEL_PEER_REASON_TEARDOWN);
	spin_unlock_bh(&ovpn->peers.lock);
}
