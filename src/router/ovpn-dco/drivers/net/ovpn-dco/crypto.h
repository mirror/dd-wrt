/* SPDX-License-Identifier: GPL-2.0-only */
/*  OpenVPN data channel accelerator
 *
 *  Copyright (C) 2020-2023 OpenVPN, Inc.
 *
 *  Author:	James Yonan <james@openvpn.net>
 *		Antonio Quartulli <antonio@openvpn.net>
 */

#ifndef _NET_OVPN_DCO_OVPNCRYPTO_H_
#define _NET_OVPN_DCO_OVPNCRYPTO_H_

#include "main.h"
#include "pktid.h"

#include <uapi/linux/ovpn_dco.h>
#include <linux/skbuff.h>

struct ovpn_peer;
struct ovpn_crypto_key_slot;

/* info needed for both encrypt and decrypt directions */
struct ovpn_key_direction {
	const u8 *cipher_key;
	size_t cipher_key_size;
	const u8 *nonce_tail; /* only needed for GCM modes */
	size_t nonce_tail_size; /* only needed for GCM modes */
};

/* all info for a particular symmetric key (primary or secondary) */
struct ovpn_key_config {
	enum ovpn_cipher_alg cipher_alg;
	u8 key_id;
	struct ovpn_key_direction encrypt;
	struct ovpn_key_direction decrypt;
};

/* used to pass settings from netlink to the crypto engine */
struct ovpn_peer_key_reset {
	enum ovpn_key_slot slot;
	struct ovpn_key_config key;
};

struct ovpn_crypto_key_slot {
	u8 key_id;

	struct crypto_aead *encrypt;
	struct crypto_aead *decrypt;
	struct ovpn_nonce_tail nonce_tail_xmit;
	struct ovpn_nonce_tail nonce_tail_recv;

	struct ovpn_pktid_recv pid_recv ____cacheline_aligned_in_smp;
	struct ovpn_pktid_xmit pid_xmit ____cacheline_aligned_in_smp;
	struct kref refcount;
	struct rcu_head rcu;
};

struct ovpn_crypto_state {
	struct ovpn_crypto_key_slot __rcu *primary;
	struct ovpn_crypto_key_slot __rcu *secondary;

	/* protects primary and secondary slots */
	struct mutex mutex;
};

static inline bool ovpn_crypto_key_slot_hold(struct ovpn_crypto_key_slot *ks)
{
	return kref_get_unless_zero(&ks->refcount);
}

static inline void ovpn_crypto_state_init(struct ovpn_crypto_state *cs)
{
	RCU_INIT_POINTER(cs->primary, NULL);
	RCU_INIT_POINTER(cs->secondary, NULL);
	mutex_init(&cs->mutex);
}

static inline struct ovpn_crypto_key_slot *
ovpn_crypto_key_id_to_slot(const struct ovpn_crypto_state *cs, u8 key_id)
{
	struct ovpn_crypto_key_slot *ks;

	if (unlikely(!cs))
		return NULL;

	rcu_read_lock();
	ks = rcu_dereference(cs->primary);
	if (ks && ks->key_id == key_id) {
		if (unlikely(!ovpn_crypto_key_slot_hold(ks)))
			ks = NULL;
		goto out;
	}

	ks = rcu_dereference(cs->secondary);
	if (ks && ks->key_id == key_id) {
		if (unlikely(!ovpn_crypto_key_slot_hold(ks)))
			ks = NULL;
		goto out;
	}

	/* when both key slots are occupied but no matching key ID is found, ks has to be reset to
	 * NULL to avoid carrying a stale pointer
	 */
	ks = NULL;
out:
	rcu_read_unlock();

	return ks;
}

static inline struct ovpn_crypto_key_slot *
ovpn_crypto_key_slot_primary(const struct ovpn_crypto_state *cs)
{
	struct ovpn_crypto_key_slot *ks;

	rcu_read_lock();
	ks = rcu_dereference(cs->primary);
	if (unlikely(ks && !ovpn_crypto_key_slot_hold(ks)))
		ks = NULL;
	rcu_read_unlock();

	return ks;
}

void ovpn_crypto_key_slot_release(struct kref *kref);

static inline void ovpn_crypto_key_slot_put(struct ovpn_crypto_key_slot *ks)
{
	kref_put(&ks->refcount, ovpn_crypto_key_slot_release);
}

int ovpn_crypto_state_reset(struct ovpn_crypto_state *cs,
			    const struct ovpn_peer_key_reset *pkr);

void ovpn_crypto_key_slot_delete(struct ovpn_crypto_state *cs,
				 enum ovpn_key_slot slot);

void ovpn_crypto_state_release(struct ovpn_crypto_state *cs);

void ovpn_crypto_key_slots_swap(struct ovpn_crypto_state *cs);

void ovpn_crypto_kill_primary(struct ovpn_crypto_state *cs);

#endif /* _NET_OVPN_DCO_OVPNCRYPTO_H_ */
