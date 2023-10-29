/* SPDX-License-Identifier: GPL-2.0-only */
/*  OpenVPN data channel accelerator
 *
 *  Copyright (C) 2020-2023 OpenVPN, Inc.
 *
 *  Author:	Antonio Quartulli <antonio@openvpn.net>
 *		James Yonan <james@openvpn.net>
 */

#ifndef _NET_OVPN_DCO_OVPNPROTO_H_
#define _NET_OVPN_DCO_OVPNPROTO_H_

#include "main.h"

#include <linux/skbuff.h>

/* Methods for operating on the initial command
 * byte of the OpenVPN protocol.
 */

/* packet opcode (high 5 bits) and key-id (low 3 bits) are combined in
 * one byte
 */
#define OVPN_KEY_ID_MASK 0x07
#define OVPN_OPCODE_SHIFT 3
#define OVPN_OPCODE_MASK 0x1F
/* upper bounds on opcode and key ID */
#define OVPN_KEY_ID_MAX (OVPN_KEY_ID_MASK + 1)
#define OVPN_OPCODE_MAX (OVPN_OPCODE_MASK + 1)
/* packet opcodes of interest to us */
#define OVPN_DATA_V1 6 /* data channel V1 packet */
#define OVPN_DATA_V2 9 /* data channel V2 packet */
/* size of initial packet opcode */
#define OVPN_OP_SIZE_V1 1
#define OVPN_OP_SIZE_V2	4
#define OVPN_PEER_ID_MASK 0x00FFFFFF
#define OVPN_PEER_ID_UNDEF 0x00FFFFFF
/* first byte of keepalive message */
#define OVPN_KEEPALIVE_FIRST_BYTE 0x2a
/* first byte of exit message */
#define OVPN_EXPLICIT_EXIT_NOTIFY_FIRST_BYTE 0x28

/**
 * Extract the OP code from the specified byte
 *
 * Return the OP code
 */
static inline u8 ovpn_opcode_from_byte(u8 byte)
{
	return byte >> OVPN_OPCODE_SHIFT;
}

/**
 * Extract the OP code from the skb head.
 *
 * Note: this function assumes that the skb head was pulled enough
 * to access the first byte.
 *
 * Return the OP code
 */
static inline u8 ovpn_opcode_from_skb(const struct sk_buff *skb, u16 offset)
{
	return ovpn_opcode_from_byte(*(skb->data + offset));
}

/**
 * Extract the key ID from the skb head.
 *
 * Note: this function assumes that the skb head was pulled enough
 * to access the first byte.
 *
 * Return the key ID
 */

static inline u8 ovpn_key_id_from_skb(const struct sk_buff *skb)
{
	return *skb->data & OVPN_KEY_ID_MASK;
}

/**
 * Extract the peer ID from the skb head.
 *
 * Note: this function assumes that the skb head was pulled enough
 * to access the first 4 bytes.
 *
 * Return the peer ID.
 */

static inline u32 ovpn_peer_id_from_skb(const struct sk_buff *skb, u16 offset)
{
	return ntohl(*(__be32 *)(skb->data + offset)) & OVPN_PEER_ID_MASK;
}

static inline u32 ovpn_opcode_compose(u8 opcode, u8 key_id, u32 peer_id)
{
	const u8 op = (opcode << OVPN_OPCODE_SHIFT) | (key_id & OVPN_KEY_ID_MASK);

	return (op << 24) | (peer_id & OVPN_PEER_ID_MASK);
}

#endif /* _NET_OVPN_DCO_OVPNPROTO_H_ */
