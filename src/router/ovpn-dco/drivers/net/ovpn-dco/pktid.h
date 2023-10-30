/* SPDX-License-Identifier: GPL-2.0-only */
/*  OpenVPN data channel accelerator
 *
 *  Copyright (C) 2020-2023 OpenVPN, Inc.
 *
 *  Author:	Antonio Quartulli <antonio@openvpn.net>
 *		James Yonan <james@openvpn.net>
 */

#ifndef _NET_OVPN_DCO_OVPNPKTID_H_
#define _NET_OVPN_DCO_OVPNPKTID_H_

#include "main.h"

/* When the OpenVPN protocol is run in AEAD mode, use
 * the OpenVPN packet ID as the AEAD nonce:
 *
 *    00000005 521c3b01 4308c041
 *    [seq # ] [  nonce_tail   ]
 *    [     12-byte full IV    ] -> NONCE_SIZE
 *    [4-bytes                   -> NONCE_WIRE_SIZE
 *    on wire]
 */

/* OpenVPN nonce size */
#define NONCE_SIZE 12
/* amount of bytes of the nonce received from user space */
#define NONCE_TAIL_SIZE 8

/* OpenVPN nonce size reduced by 8-byte nonce tail -- this is the
 * size of the AEAD Associated Data (AD) sent over the wire
 * and is normally the head of the IV (depends on the specific algorithm)
 */
#define NONCE_WIRE_SIZE (NONCE_SIZE - sizeof(struct ovpn_nonce_tail))

/* If no packets received for this length of time, set a backtrack floor
 * at highest received packet ID thus far.
 */
#define PKTID_RECV_EXPIRE (30 * HZ)

/* Last 8 bytes of AEAD nonce
 * Provided by userspace and usually derived from
 * key material generated during TLS handshake
 */
struct ovpn_nonce_tail {
	u8 u8[NONCE_TAIL_SIZE];
};

/* Packet-ID state for transmitter */
struct ovpn_pktid_xmit {
	atomic64_t seq_num;
	struct ovpn_tcp_linear *tcp_linear;
};

/* replay window sizing in bytes = 2^REPLAY_WINDOW_ORDER */
#define REPLAY_WINDOW_ORDER 8

#define REPLAY_WINDOW_BYTES BIT(REPLAY_WINDOW_ORDER)
#define REPLAY_WINDOW_SIZE  (REPLAY_WINDOW_BYTES * 8)
#define REPLAY_INDEX(base, i) (((base) + (i)) & (REPLAY_WINDOW_SIZE - 1))

/* Packet-ID state for receiver.
 * Other than lock member, can be zeroed to initialize.
 */
struct ovpn_pktid_recv {
	/* "sliding window" bitmask of recent packet IDs received */
	u8 history[REPLAY_WINDOW_BYTES];
	/* bit position of deque base in history */
	unsigned int base;
	/* extent (in bits) of deque in history */
	unsigned int extent;
	/* expiration of history in jiffies */
	unsigned long expire;
	/* highest sequence number received */
	u32 id;
	/* highest time stamp received */
	u32 time;
	/* we will only accept backtrack IDs > id_floor */
	u32 id_floor;
	unsigned int max_backtrack;
	/* protects entire pktd ID state */
	spinlock_t lock;
};

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0)
#ifndef atomic64_fetch_add_unless
static inline int atomic64_fetch_add_unless(atomic64_t *v, long long a, long long u)
{
	return atomic64_add_unless(v, a, u);
}
#endif
#endif


/* Get the next packet ID for xmit */
static inline int ovpn_pktid_xmit_next(struct ovpn_pktid_xmit *pid, u32 *pktid)
{
	const s64 seq_num = atomic64_fetch_add_unless(&pid->seq_num, 1,
						      0x100000000LL);
	/* when the 32bit space is over, we return an error because the packet ID is used to create
	 * the cipher IV and we do not want to re-use the same value more than once
	 */
	if (unlikely(seq_num == 0x100000000LL))
		return -ERANGE;

	*pktid = (u32)seq_num;

	return 0;
}

/* Write 12-byte AEAD IV to dest */
static inline void ovpn_pktid_aead_write(const u32 pktid,
					 const struct ovpn_nonce_tail *nt,
					 unsigned char *dest)
{
	*(__force __be32 *)(dest) = htonl(pktid);
	BUILD_BUG_ON(4 + sizeof(struct ovpn_nonce_tail) != NONCE_SIZE);
	memcpy(dest + 4, nt->u8, sizeof(struct ovpn_nonce_tail));
}

void ovpn_pktid_xmit_init(struct ovpn_pktid_xmit *pid);
void ovpn_pktid_recv_init(struct ovpn_pktid_recv *pr);

int ovpn_pktid_recv(struct ovpn_pktid_recv *pr, u32 pkt_id, u32 pkt_time);

#endif /* _NET_OVPN_DCO_OVPNPKTID_H_ */
