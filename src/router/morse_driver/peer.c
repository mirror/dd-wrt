/*
 * Copyright 2024 Morse Micro
 */
#include <linux/types.h>
#include <linux/list.h>
#include <linux/ieee80211.h>
#include <net/mac80211.h>

#include "morse.h"
#include "mac.h"
#include "peer.h"
#include "debug.h"

/* Limit age of pre-assoc peers */
#define PRE_ASSOC_PEER_AGE_LIMIT_MS	(100)

#define MORSE_PEER_DBG(_m, _f, _a...)	morse_dbg(FEATURE_ID_DEFAULT, _m, _f, ##_a)
#define MORSE_PEER_INFO(_m, _f, _a...)	morse_info(FEATURE_ID_DEFAULT, _m, _f, ##_a)
#define MORSE_PEER_WARN(_m, _f, _a...)	morse_warn(FEATURE_ID_DEFAULT, _m, _f, ##_a)
#define MORSE_PEER_ERR(_m, _f, _a...)	morse_err(FEATURE_ID_DEFAULT, _m, _f, ##_a)
#define MORSE_PEER_WARN_ON(_a)		MORSE_WARN_ON(FEATURE_ID_DEFAULT, _a)

static bool has_peer_expired(const struct morse_pre_assoc_peer *peer)
{
	u32 age_limit = msecs_to_jiffies(PRE_ASSOC_PEER_AGE_LIMIT_MS);

	return time_after(jiffies, peer->last_used + age_limit);
}

void morse_pre_assoc_peer_list_init(struct morse *mors)
{
	if (!mors) {
		MORSE_PEER_WARN_ON(1);
		return;
	}

	INIT_LIST_HEAD(&mors->pre_assoc_peers.list);
	spin_lock_init(&mors->pre_assoc_peers.lock);
}

void morse_pre_assoc_peer_list_vif_take(struct morse *mors)
{
	if (!mors) {
		MORSE_PEER_WARN_ON(1);
		return;
	}

	spin_lock_bh(&mors->pre_assoc_peers.lock);
	mors->pre_assoc_peers.n_ifaces_using++;
	MORSE_PEER_DBG(mors, "%s: in use by %d ifaces", __func__,
		       mors->pre_assoc_peers.n_ifaces_using);
	spin_unlock_bh(&mors->pre_assoc_peers.lock);
}

void morse_pre_assoc_peer_list_vif_release(struct morse *mors)
{
	struct morse_pre_assoc_peer *peer;
	struct morse_pre_assoc_peer *tmp;

	if (!mors) {
		MORSE_PEER_WARN_ON(1);
		return;
	}

	spin_lock_bh(&mors->pre_assoc_peers.lock);
	mors->pre_assoc_peers.n_ifaces_using--;
	MORSE_PEER_WARN_ON(mors->pre_assoc_peers.n_ifaces_using < 0);
	if (mors->pre_assoc_peers.n_ifaces_using > 0)
		goto exit; /* There are still interfaces using this list */

	list_for_each_entry_safe(peer, tmp, &mors->pre_assoc_peers.list, list) {
		list_del(&peer->list);
		kfree(peer);
	}

exit:
	MORSE_PEER_DBG(mors, "%s: in use by %d ifaces", __func__,
		       mors->pre_assoc_peers.n_ifaces_using);
	spin_unlock_bh(&mors->pre_assoc_peers.lock);
}

int morse_pre_assoc_peer_delete(struct morse *mors, const u8 *addr)
{
	struct morse_pre_assoc_peer *peer, *tmp;

	if (!mors || !addr) {
		MORSE_PEER_WARN_ON(1);
		return -EINVAL;
	}

	spin_lock_bh(&mors->pre_assoc_peers.lock);
	list_for_each_entry_safe(peer, tmp, &mors->pre_assoc_peers.list, list) {
		if (memcmp(peer->addr, addr, sizeof(peer->addr)) == 0) {
			list_del(&peer->list);
			kfree(peer);
			spin_unlock_bh(&mors->pre_assoc_peers.lock);
			MORSE_PEER_DBG(mors, "%s: %pM removed", __func__, addr);
			return 0;
		}

		if (has_peer_expired(peer)) {
			/* Take the opportunity to remove any expired peers */
			list_del(&peer->list);
			kfree(peer);
			MORSE_PEER_DBG(mors, "%s: %pM expired", __func__, addr);
		}
	}
	spin_unlock_bh(&mors->pre_assoc_peers.lock);

	MORSE_PEER_DBG(mors, "%s: %pM not found", __func__, addr);
	return -ENXIO;
}

int morse_pre_assoc_peer_get_last_rx_bw_mhz(struct morse *mors, const u8 *addr)
{
	int bw_mhz = -1;
	struct morse_pre_assoc_peer *peer, *tmp;

	if (!mors || !addr) {
		MORSE_PEER_WARN_ON(1);
		return -EINVAL;
	}

	if (is_broadcast_ether_addr(addr) || is_multicast_ether_addr(addr))
		return -1;

	spin_lock_bh(&mors->pre_assoc_peers.lock);
	list_for_each_entry_safe(peer, tmp, &mors->pre_assoc_peers.list, list) {
		if (memcmp(peer->addr, addr, sizeof(peer->addr)) == 0) {
			bw_mhz = peer->last_rx_bw_mhz;
			peer->last_used = jiffies;
			break;
		}

		if (has_peer_expired(peer)) {
			/* Take the opportunity to remove any expired peers */
			list_del(&peer->list);
			kfree(peer);
			MORSE_PEER_DBG(mors, "%s: %pM expired", __func__, addr);
		}
	}
	spin_unlock_bh(&mors->pre_assoc_peers.lock);

	if (bw_mhz > 0)
		MORSE_PEER_DBG(mors, "%s: peer %pM found [rx bw:%d MHz]", __func__, addr, bw_mhz);
	else
		MORSE_PEER_DBG(mors, "%s: peer %pM not found", __func__, addr);

	return bw_mhz;
}

static void morse_update_peer_rx_info(const struct morse *mors, struct morse_pre_assoc_peer *peer,
				morse_rate_code_t rc)
{
	u8 bw_mhz = morse_ratecode_bw_index_to_s1g_bw_mhz(morse_ratecode_bw_index_get(rc));

	if (bw_mhz == peer->last_rx_bw_mhz)
		return; /* Nothing to update */

	peer->last_rx_bw_mhz = bw_mhz;
	MORSE_PEER_DBG(mors, "%s: peer %pM updated [rx bw:%u MHz]", __func__,
		       peer->addr, peer->last_rx_bw_mhz);
}

int morse_pre_assoc_peer_update_rx_info(struct morse *mors, const u8 *addr, morse_rate_code_t rc)
{
	struct morse_pre_assoc_peer *peer, *tmp;

	if (!mors || !addr)
		return -EINVAL;

	if (is_broadcast_ether_addr(addr) || is_multicast_ether_addr(addr)) {
		MORSE_PEER_DBG(mors, "%s: ignoring broadcast/multicast addr", __func__);
		return 0;
	}

	spin_lock_bh(&mors->pre_assoc_peers.lock);
	list_for_each_entry_safe(peer, tmp, &mors->pre_assoc_peers.list, list) {
		if (memcmp(peer->addr, addr, sizeof(peer->addr)) == 0) {
			morse_update_peer_rx_info(mors, peer, rc);
			peer->last_used = jiffies;
			spin_unlock_bh(&mors->pre_assoc_peers.lock);
			return 0;
		}

		if (has_peer_expired(peer)) {
			/* Take the opportunity to remove any expired peers */
			list_del(&peer->list);
			kfree(peer);
			MORSE_PEER_DBG(mors, "%s: %pM expired", __func__, addr);
		}
	}
	spin_unlock_bh(&mors->pre_assoc_peers.lock);

	/* Allocate memory for a new peer */
	peer = kzalloc(sizeof(*peer), GFP_KERNEL);
	if (!peer) {
		MORSE_PEER_ERR(mors, "%s: no memory for peer %pM", __func__, addr);
		return -ENOMEM;
	}
	MORSE_PEER_DBG(mors, "%s: peer %pM added", __func__, addr);
	memcpy(peer->addr, addr, sizeof(peer->addr));
	morse_update_peer_rx_info(mors, peer, rc);
	peer->last_used = jiffies;

	/* Add new peer to the list */
	spin_lock_bh(&mors->pre_assoc_peers.lock);
	list_add_tail(&peer->list, &mors->pre_assoc_peers.list);
	spin_unlock_bh(&mors->pre_assoc_peers.lock);

	return 0;
}
