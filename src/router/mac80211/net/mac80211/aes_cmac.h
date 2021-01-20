/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright 2008, Jouni Malinen <j@w1.fi>
 */

#ifndef AES_CMAC_H
#define AES_CMAC_H

#include <linux/crypto.h>

struct crypto_cipher *ieee80211_aes_cmac_key_setup(const u8 key[],
						   size_t key_len);
void ieee80211_aes_cmac(struct crypto_cipher *tfm, const u8 *aad,
			const u8 *data, size_t data_len, u8 *mic);
void ieee80211_aes_cmac_256(struct crypto_cipher *tfm, const u8 *aad,
			    const u8 *data, size_t data_len, u8 *mic);
void ieee80211_aes_cmac_key_free(struct crypto_cipher *tfm);

#endif /* AES_CMAC_H */
