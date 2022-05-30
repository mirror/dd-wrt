/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Software WEP encryption implementation
 * Copyright 2002, Jouni Malinen <jkmaline@cc.hut.fi>
 * Copyright 2003, Instant802 Networks, Inc.
 */

#ifndef WEP_H
#define WEP_H

#include <linux/skbuff.h>
#include <linux/types.h>
#include "ieee80211_i.h"
#include "key.h"

static void ieee80211_wep_init(struct ieee80211_local *local);
static int ieee80211_wep_encrypt_data(struct arc4_ctx *ctx, u8 *rc4key,
				size_t klen, u8 *data, size_t data_len);
static int ieee80211_wep_encrypt(struct ieee80211_local *local,
			  struct sk_buff *skb,
			  const u8 *key, int keylen, int keyidx);
static int ieee80211_wep_decrypt_data(struct arc4_ctx *ctx, u8 *rc4key,
			       size_t klen, u8 *data, size_t data_len);

static ieee80211_rx_result
ieee80211_crypto_wep_decrypt(struct ieee80211_rx_data *rx);
static ieee80211_tx_result
ieee80211_crypto_wep_encrypt(struct ieee80211_tx_data *tx);

#endif /* WEP_H */
