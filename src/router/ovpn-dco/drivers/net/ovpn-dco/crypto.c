// SPDX-License-Identifier: GPL-2.0
/*  OpenVPN data channel accelerator
 *
 *  Copyright (C) 2020-2023 OpenVPN, Inc.
 *
 *  Author:	James Yonan <james@openvpn.net>
 *		Antonio Quartulli <antonio@openvpn.net>
 */

#include "main.h"
#include "crypto_aead.h"
#include "crypto.h"

#include <uapi/linux/ovpn_dco.h>

static void ovpn_ks_destroy_rcu(struct rcu_head *head)
{
	struct ovpn_crypto_key_slot *ks;

	ks = container_of(head, struct ovpn_crypto_key_slot, rcu);
	ovpn_aead_crypto_key_slot_destroy(ks);
}

void ovpn_crypto_key_slot_release(struct kref *kref)
{
	struct ovpn_crypto_key_slot *ks;

	ks = container_of(kref, struct ovpn_crypto_key_slot, refcount);
	call_rcu(&ks->rcu, ovpn_ks_destroy_rcu);
}

/* can only be invoked when all peer references have been dropped (i.e. RCU
 * release routine)
 */
void ovpn_crypto_state_release(struct ovpn_crypto_state *cs)
{
	struct ovpn_crypto_key_slot *ks;

	ks = rcu_access_pointer(cs->primary);
	if (ks) {
		RCU_INIT_POINTER(cs->primary, NULL);
		ovpn_crypto_key_slot_put(ks);
	}

	ks = rcu_access_pointer(cs->secondary);
	if (ks) {
		RCU_INIT_POINTER(cs->secondary, NULL);
		ovpn_crypto_key_slot_put(ks);
	}

	mutex_destroy(&cs->mutex);
}

/* removes the primary key from the crypto context */
void ovpn_crypto_kill_primary(struct ovpn_crypto_state *cs)
{
	struct ovpn_crypto_key_slot *ks;

	mutex_lock(&cs->mutex);
	ks = rcu_replace_pointer(cs->primary, NULL, lockdep_is_held(&cs->mutex));
	ovpn_crypto_key_slot_put(ks);
	mutex_unlock(&cs->mutex);
}

/* Reset the ovpn_crypto_state object in a way that is atomic
 * to RCU readers.
 */
int ovpn_crypto_state_reset(struct ovpn_crypto_state *cs,
			    const struct ovpn_peer_key_reset *pkr)
	__must_hold(cs->mutex)
{
	struct ovpn_crypto_key_slot *old = NULL;
	struct ovpn_crypto_key_slot *new;

	lockdep_assert_held(&cs->mutex);

	new = ovpn_aead_crypto_key_slot_new(&pkr->key);
	if (IS_ERR(new))
		return PTR_ERR(new);

	switch (pkr->slot) {
	case OVPN_KEY_SLOT_PRIMARY:
		old = rcu_replace_pointer(cs->primary, new,
					  lockdep_is_held(&cs->mutex));
		break;
	case OVPN_KEY_SLOT_SECONDARY:
		old = rcu_replace_pointer(cs->secondary, new,
					  lockdep_is_held(&cs->mutex));
		break;
	default:
		goto free_key;
	}

	if (old)
		ovpn_crypto_key_slot_put(old);

	return 0;
free_key:
	ovpn_crypto_key_slot_put(new);
	return -EINVAL;
}

void ovpn_crypto_key_slot_delete(struct ovpn_crypto_state *cs,
				 enum ovpn_key_slot slot)
{
	struct ovpn_crypto_key_slot *ks = NULL;

	mutex_lock(&cs->mutex);
	switch (slot) {
	case OVPN_KEY_SLOT_PRIMARY:
		ks = rcu_replace_pointer(cs->primary, NULL,
					 lockdep_is_held(&cs->mutex));
		break;
	case OVPN_KEY_SLOT_SECONDARY:
		ks = rcu_replace_pointer(cs->secondary, NULL,
					 lockdep_is_held(&cs->mutex));
		break;
	default:
		pr_warn("Invalid slot to release: %u\n", slot);
		break;
	}
	mutex_unlock(&cs->mutex);

	if (!ks) {
		pr_debug("Key slot already released: %u\n", slot);
		return;
	}
	pr_debug("deleting key slot %u, key_id=%u\n", slot, ks->key_id);

	ovpn_crypto_key_slot_put(ks);
}

/* this swap is not atomic, but there will be a very short time frame where the
 * old_secondary key won't be available. This should not be a big deal as most
 * likely both peers are already using the new primary at this point.
 */
void ovpn_crypto_key_slots_swap(struct ovpn_crypto_state *cs)
{
	const struct ovpn_crypto_key_slot *old_primary, *old_secondary;

	mutex_lock(&cs->mutex);

	old_secondary = rcu_dereference_protected(cs->secondary,
						  lockdep_is_held(&cs->mutex));
	old_primary = rcu_replace_pointer(cs->primary, old_secondary,
					  lockdep_is_held(&cs->mutex));
	rcu_assign_pointer(cs->secondary, old_primary);

	pr_debug("key swapped: %u <-> %u\n",
		 old_primary ? old_primary->key_id : 0,
		 old_secondary ? old_secondary->key_id : 0);

	mutex_unlock(&cs->mutex);
}
