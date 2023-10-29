// SPDX-License-Identifier: GPL-2.0
/*  OpenVPN data channel accelerator
 *
 *  Copyright (C) 2019-2023 OpenVPN, Inc.
 *
 *  Author:	Antonio Quartulli <antonio@openvpn.net>
 */

#include "main.h"
#include "ovpnstruct.h"
#include "ovpn.h"
#include "peer.h"
#include "proto.h"
#include "skb.h"
#include "tcp.h"

#include <linux/ptr_ring.h>
#include <linux/skbuff.h>
#include <net/tcp.h>
#include <net/route.h>

static struct proto ovpn_tcp_prot;

static int ovpn_tcp_read_sock(read_descriptor_t *desc, struct sk_buff *in_skb,
			      unsigned int in_offset, size_t in_len)
{
	struct sock *sk = desc->arg.data;
	struct ovpn_socket *sock;
	struct ovpn_skb_cb *cb;
	struct ovpn_peer *peer;
	size_t chunk, copied = 0;
	int status;
	void *data;
	u16 len;

	rcu_read_lock();
	sock = rcu_dereference_sk_user_data(sk);
	rcu_read_unlock();

	if (unlikely(!sock || !sock->peer)) {
		pr_err("ovpn: read_sock triggered for socket with no metadata\n");
		desc->error = -EINVAL;
		return 0;
	}

	peer = sock->peer;

	while (in_len > 0) {
		/* no skb allocated means that we have to read (or finish reading) the 2 bytes
		 * prefix containing the actual packet size.
		 */
		if (!peer->tcp.skb) {
			chunk = min_t(size_t, in_len, sizeof(u16) - peer->tcp.offset);
			WARN_ON(skb_copy_bits(in_skb, in_offset,
					      peer->tcp.raw_len + peer->tcp.offset, chunk) < 0);
			peer->tcp.offset += chunk;

			/* keep on reading until we got the whole packet size */
			if (peer->tcp.offset != sizeof(u16))
				goto next_read;

			len = ntohs(*(__be16 *)peer->tcp.raw_len);
			/* invalid packet length: this is a fatal TCP error */
			if (!len) {
				netdev_err(peer->ovpn->dev, "%s: received invalid packet length: %d\n",
					   __func__, len);
				desc->error = -EINVAL;
				goto err;
			}

			/* add 2 bytes to allocated space (and immediately reserve them) for packet
			 * length prepending, in case the skb has to be forwarded to userspace
			 */
			peer->tcp.skb = netdev_alloc_skb_ip_align(peer->ovpn->dev,
								  len + sizeof(u16));
			if (!peer->tcp.skb) {
				desc->error = -ENOMEM;
				goto err;
			}
			skb_reserve(peer->tcp.skb, sizeof(u16));

			peer->tcp.offset = 0;
			peer->tcp.data_len = len;
		} else {
			chunk = min_t(size_t, in_len, peer->tcp.data_len - peer->tcp.offset);

			/* extend skb to accommodate the new chunk and copy it from the input skb */
			data = skb_put(peer->tcp.skb, chunk);
			WARN_ON(skb_copy_bits(in_skb, in_offset, data, chunk) < 0);
			peer->tcp.offset += chunk;

			/* keep on reading until we get the full packet */
			if (peer->tcp.offset != peer->tcp.data_len)
				goto next_read;

			/* do not perform IP caching for TCP connections */
			cb = OVPN_SKB_CB(peer->tcp.skb);
			cb->sa_fam = AF_UNSPEC;

			/* At this point we know the packet is from a configured peer.
			 * DATA_V2 packets are handled in kernel space, the rest goes to user space.
			 *
			 * Queue skb for sending to userspace via recvmsg on the socket
			 */
			if (likely(ovpn_opcode_from_skb(peer->tcp.skb, 0) == OVPN_DATA_V2)) {
				/* hold reference to peer as requird by ovpn_recv() */
				ovpn_peer_hold(peer);
				status = ovpn_recv(peer->ovpn, peer, peer->tcp.skb);
			} else {
				/* prepend skb with packet len. this way userspace can parse
				 * the packet as if it just arrived from the remote endpoint
				 */
				void *raw_len = __skb_push(peer->tcp.skb, sizeof(u16));
				memcpy(raw_len, peer->tcp.raw_len, sizeof(u16));

				status = ptr_ring_produce_bh(&peer->sock->recv_ring, peer->tcp.skb);
				if (likely(!status))
					peer->tcp.sk_cb.sk_data_ready(sk);
			}

			/* skb not consumed - free it now */
			if (unlikely(status < 0))
				kfree_skb(peer->tcp.skb);

			peer->tcp.skb = NULL;
			peer->tcp.offset = 0;
			peer->tcp.data_len = 0;
		}
next_read:
		in_len -= chunk;
		in_offset += chunk;
		copied += chunk;
	}

	return copied;
err:
	netdev_err(peer->ovpn->dev, "cannot process incoming TCP data: %d\n", desc->error);
	ovpn_peer_del(peer, OVPN_DEL_PEER_REASON_TRANSPORT_ERROR);
	return 0;
}

static void ovpn_tcp_data_ready(struct sock *sk)
{
	struct socket *sock = sk->sk_socket;
	read_descriptor_t desc;

	if (unlikely(!sock || !sock->ops || !sock->ops->read_sock))
		return;

	desc.arg.data = sk;
	desc.error = 0;
	desc.count = 1;

	sock->ops->read_sock(sk, &desc, ovpn_tcp_read_sock);
}

static void ovpn_tcp_write_space(struct sock *sk)
{
	struct ovpn_socket *sock;

	rcu_read_lock();
	sock = rcu_dereference_sk_user_data(sk);
	rcu_read_unlock();

	if (!sock || !sock->peer)
		return;

	queue_work(sock->peer->ovpn->events_wq, &sock->peer->tcp.tx_work);
}

static bool ovpn_tcp_sock_is_readable(
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0) && !defined(EL9)
				      const struct sock *sk
#else
				      struct sock *sk
#endif
				      )

{
	struct ovpn_socket *sock;

	rcu_read_lock();
	sock = rcu_dereference_sk_user_data(sk);
	rcu_read_unlock();

	if (!sock || !sock->peer)
		return false;

	return !ptr_ring_empty_bh(&sock->recv_ring);
}

static int ovpn_tcp_recvmsg(struct sock *sk, struct msghdr *msg, size_t len,
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 16, 0)
			    int noblock,
#endif
			    int flags, int *addr_len)
{
	bool tmp = flags & MSG_DONTWAIT;
	DEFINE_WAIT_FUNC(wait, woken_wake_function);
	int ret, chunk, copied = 0;
	struct ovpn_socket *sock;
	struct sk_buff *skb;
	long timeo;

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 16, 0)
	tmp = noblock;
#endif

	if (unlikely(flags & MSG_ERRQUEUE))
		return sock_recv_errqueue(sk, msg, len, SOL_IP, IP_RECVERR);

	timeo = sock_rcvtimeo(sk, tmp);

	rcu_read_lock();
	sock = rcu_dereference_sk_user_data(sk);
	rcu_read_unlock();

	if (!sock || !sock->peer) {
		ret = -EBADF;
		goto unlock;
	}

	while (ptr_ring_empty_bh(&sock->recv_ring)) {
		if (sk->sk_shutdown & RCV_SHUTDOWN)
			return 0;

		if (sock_flag(sk, SOCK_DONE))
			return 0;

		if (!timeo) {
			ret = -EAGAIN;
			goto unlock;
		}

		add_wait_queue(sk_sleep(sk), &wait);
		sk_set_bit(SOCKWQ_ASYNC_WAITDATA, sk);
		sk_wait_event(sk, &timeo, !ptr_ring_empty_bh(&sock->recv_ring), &wait);
		sk_clear_bit(SOCKWQ_ASYNC_WAITDATA, sk);
		remove_wait_queue(sk_sleep(sk), &wait);

		/* take care of signals */
		if (signal_pending(current)) {
			ret = sock_intr_errno(timeo);
			goto unlock;
		}
	}

	while (len && (skb = __ptr_ring_peek(&sock->recv_ring))) {
		chunk = min_t(size_t, len, skb->len);
		ret = skb_copy_datagram_msg(skb, 0, msg, chunk);
		if (ret < 0) {
			pr_err("ovpn: cannot copy TCP data to userspace: %d\n", ret);
			kfree_skb(skb);
			goto unlock;
		}

		__skb_pull(skb, chunk);

		if (!skb->len) {
			/* skb was entirely consumed and can now be removed from the ring */
			__ptr_ring_discard_one(&sock->recv_ring);
			consume_skb(skb);
		}

		len -= chunk;
		copied += chunk;
	}
	ret = copied;

unlock:
	return ret ? : -EAGAIN;
}

static void ovpn_destroy_skb(void *skb)
{
	consume_skb(skb);
}

void ovpn_tcp_socket_detach(struct socket *sock)
{
	struct ovpn_socket *ovpn_sock;
	struct ovpn_peer *peer;

	if (!sock)
		return;

	rcu_read_lock();
	ovpn_sock = rcu_dereference_sk_user_data(sock->sk);
	rcu_read_unlock();

	if (!ovpn_sock->peer)
		return;

	peer = ovpn_sock->peer;

	/* restore CBs that were saved in ovpn_sock_set_tcp_cb() */
	write_lock_bh(&sock->sk->sk_callback_lock);
	sock->sk->sk_data_ready = peer->tcp.sk_cb.sk_data_ready;
	sock->sk->sk_write_space = peer->tcp.sk_cb.sk_write_space;
	sock->sk->sk_prot = peer->tcp.sk_cb.prot;
	rcu_assign_sk_user_data(sock->sk, NULL);
	write_unlock_bh(&sock->sk->sk_callback_lock);

	/* cancel any ongoing work. Done after removing the CBs so that these workers cannot be
	 * re-armed
	 */
	cancel_work_sync(&peer->tcp.tx_work);

	ptr_ring_cleanup(&ovpn_sock->recv_ring, ovpn_destroy_skb);
	ptr_ring_cleanup(&peer->tcp.tx_ring, ovpn_destroy_skb);
}

/* Try to send one skb (or part of it) over the TCP stream.
 *
 * Return 0 on success or a negative error code otherwise.
 *
 * Note that the skb is modified by putting away the data being sent, therefore
 * the caller should check if skb->len is zero to understand if the full skb was
 * sent or not.
 */
static int ovpn_tcp_send_one(struct ovpn_peer *peer, struct sk_buff *skb)
{
	struct msghdr msg = { .msg_flags = MSG_DONTWAIT | MSG_NOSIGNAL };
	struct kvec iv = { 0 };
	int ret;

	if (skb_linearize(skb) < 0) {
		net_err_ratelimited("%s: can't linearize packet\n", __func__);
		return -ENOMEM;
	}

	/* initialize iv structure now as skb_linearize() may have changed skb->data */
	iv.iov_base = skb->data;
	iv.iov_len = skb->len;

	ret = kernel_sendmsg(peer->sock->sock, &msg, &iv, 1, iv.iov_len);
	if (ret > 0) {
		__skb_pull(skb, ret);

		/* since we update per-cpu stats in process context,
		 * we need to disable softirqs
		 */
		local_bh_disable();
		dev_sw_netstats_tx_add(peer->ovpn->dev, 1, ret);
		local_bh_enable();

		return 0;
	}

	return ret;
}

/* Process packets in TCP TX queue */
static void ovpn_tcp_tx_work(struct work_struct *work)
{
	struct ovpn_peer *peer;
	struct sk_buff *skb;
	int ret;

	peer = container_of(work, struct ovpn_peer, tcp.tx_work);
	while ((skb = __ptr_ring_peek(&peer->tcp.tx_ring))) {
		ret = ovpn_tcp_send_one(peer, skb);
		if (ret < 0 && ret != -EAGAIN) {
			net_warn_ratelimited("%s: cannot send TCP packet to peer %u: %d\n", __func__,
					    peer->id, ret);
			/* in case of TCP error stop sending loop and delete peer */
			ovpn_peer_del(peer, OVPN_DEL_PEER_REASON_TRANSPORT_ERROR);
			break;
		} else if (!skb->len) {
			/* skb was entirely consumed and can now be removed from the ring */
			__ptr_ring_discard_one(&peer->tcp.tx_ring);
			consume_skb(skb);
		}

		/* give a chance to be rescheduled if needed */
		cond_resched();
	}
}

/* Put packet into TCP TX queue and schedule a consumer */
void ovpn_queue_tcp_skb(struct ovpn_peer *peer, struct sk_buff *skb)
{
	int ret;

	ret = ptr_ring_produce_bh(&peer->tcp.tx_ring, skb);
	if (ret < 0) {
		kfree_skb_list(skb);
		return;
	}

	queue_work(peer->ovpn->events_wq, &peer->tcp.tx_work);
}

/* Set TCP encapsulation callbacks */
int ovpn_tcp_socket_attach(struct socket *sock, struct ovpn_peer *peer)
{
	void *old_data;
	int ret;

	INIT_WORK(&peer->tcp.tx_work, ovpn_tcp_tx_work);

	ret = ptr_ring_init(&peer->tcp.tx_ring, OVPN_QUEUE_LEN, GFP_KERNEL);
	if (ret < 0) {
		netdev_err(peer->ovpn->dev, "cannot allocate TCP TX ring\n");
		return ret;
	}

	peer->tcp.skb = NULL;
	peer->tcp.offset = 0;
	peer->tcp.data_len = 0;

	write_lock_bh(&sock->sk->sk_callback_lock);

	/* make sure no pre-existing encapsulation handler exists */
	rcu_read_lock();
	old_data = rcu_dereference_sk_user_data(sock->sk);
	rcu_read_unlock();
	if (old_data) {
		netdev_err(peer->ovpn->dev, "provided socket already taken by other user\n");
		ret = -EBUSY;
		goto err;
	}

	/* sanity check */
	if (sock->sk->sk_protocol != IPPROTO_TCP) {
		netdev_err(peer->ovpn->dev, "provided socket is UDP but expected TCP\n");
		ret = -EINVAL;
		goto err;
	}

	/* only a fully connected socket are expected. Connection should be handled in userspace */
	if (sock->sk->sk_state != TCP_ESTABLISHED) {
		netdev_err(peer->ovpn->dev, "provided TCP socket is not in ESTABLISHED state: %d\n",
			   sock->sk->sk_state);
		ret = -EINVAL;
		goto err;
	}

	/* save current CBs so that they can be restored upon socket release */
	peer->tcp.sk_cb.sk_data_ready = sock->sk->sk_data_ready;
	peer->tcp.sk_cb.sk_write_space = sock->sk->sk_write_space;
	peer->tcp.sk_cb.prot = sock->sk->sk_prot;

	/* assign our static CBs */
	sock->sk->sk_data_ready = ovpn_tcp_data_ready;
	sock->sk->sk_write_space = ovpn_tcp_write_space;
	sock->sk->sk_prot = &ovpn_tcp_prot;

	write_unlock_bh(&sock->sk->sk_callback_lock);

	return 0;
err:
	write_unlock_bh(&sock->sk->sk_callback_lock);
	ptr_ring_cleanup(&peer->tcp.tx_ring, NULL);

	return ret;
}

int __init ovpn_tcp_init(void)
{
	/* We need to substitute the recvmsg and the sock_is_readable
	 * callbacks in the sk_prot member of the sock object for TCP
	 * sockets.
	 *
	 * However sock->sk_prot is a pointer to a static variable and
	 * therefore we can't directly modify it, otherwise every socket
	 * pointing to it will be affected.
	 *
	 * For this reason we create our own static copy and modify what
	 * we need. Then we make sk_prot point to this copy
	 * (in ovpn_tcp_socket_attach())
	 */
	ovpn_tcp_prot = tcp_prot;
	ovpn_tcp_prot.recvmsg = ovpn_tcp_recvmsg;
	ovpn_tcp_prot.sock_is_readable = ovpn_tcp_sock_is_readable;

	return 0;
}
