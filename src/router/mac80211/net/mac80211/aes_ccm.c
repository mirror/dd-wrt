/*
 * Copyright 2003-2004, Instant802 Networks, Inc.
 * Copyright 2005-2006, Devicescape Software, Inc.
 *
 * Rewrite: Copyright (C) 2013 Linaro Ltd <ard.biesheuvel@linaro.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/err.h>
#include <crypto/aead.h>
#include <crypto/aes.h>

#include <net/mac80211.h>
#include "key.h"
#include "aes_ccm.h"

static void aes_ccm_prepare(struct crypto_cipher *tfm, u8 *b_0, u8 *aad, u8 *s_0,
			    u8 *a, u8 *b)
{
	int i;

	crypto_cipher_encrypt_one(tfm, b, b_0);

	/* Extra Authenticate-only data (always two AES blocks) */
	for (i = 0; i < AES_BLOCK_SIZE; i++)
		aad[i] ^= b[i];
	crypto_cipher_encrypt_one(tfm, b, aad);

	aad += AES_BLOCK_SIZE;

	for (i = 0; i < AES_BLOCK_SIZE; i++)
		aad[i] ^= b[i];
	crypto_cipher_encrypt_one(tfm, a, aad);

	/* Mask out bits from auth-only-b_0 */
	b_0[0] &= 0x07;

	/* S_0 is used to encrypt T (= MIC) */
	b_0[14] = 0;
	b_0[15] = 0;
	crypto_cipher_encrypt_one(tfm, s_0, b_0);
}


void ieee80211_aes_ccm_encrypt(struct crypto_cipher *tfm, u8 *b_0, u8 *aad,
			       u8 *data, size_t data_len, u8 *mic,
			       size_t mic_len)
{
	int i, j, last_len, num_blocks;
	u8 b[AES_BLOCK_SIZE];
	u8 s_0[AES_BLOCK_SIZE];
	u8 e[AES_BLOCK_SIZE];
	u8 *pos, *cpos;

	num_blocks = DIV_ROUND_UP(data_len, AES_BLOCK_SIZE);
	last_len = data_len % AES_BLOCK_SIZE;
	aes_ccm_prepare(tfm, b_0, aad, s_0, b, b);

	/* Process payload blocks */
	pos = data;
	cpos = data;
	for (j = 1; j <= num_blocks; j++) {
		int blen = (j == num_blocks && last_len) ?
			last_len : AES_BLOCK_SIZE;

		/* Authentication followed by encryption */
		for (i = 0; i < blen; i++)
			b[i] ^= pos[i];
		crypto_cipher_encrypt_one(tfm, b, b);

		b_0[14] = (j >> 8) & 0xff;
		b_0[15] = j & 0xff;
		crypto_cipher_encrypt_one(tfm, e, b_0);
		for (i = 0; i < blen; i++)
			*cpos++ = *pos++ ^ e[i];
	}

	for (i = 0; i < mic_len; i++)
		mic[i] = b[i] ^ s_0[i];
}

int ieee80211_aes_ccm_decrypt(struct crypto_cipher *tfm, u8 *b_0, u8 *aad,
			      u8 *data, size_t data_len, u8 *mic,
			      size_t mic_len)
{
	int i, j, last_len, num_blocks;
	u8 *pos, *cpos;
	u8 a[AES_BLOCK_SIZE];
	u8 b[AES_BLOCK_SIZE];
	u8 s_0[AES_BLOCK_SIZE];

	num_blocks = DIV_ROUND_UP(data_len, AES_BLOCK_SIZE);
	last_len = data_len % AES_BLOCK_SIZE;
	aes_ccm_prepare(tfm, b_0, aad, s_0, a, b);

	/* Process payload blocks */
	cpos = data;
	pos = data;
	for (j = 1; j <= num_blocks; j++) {
		int blen = (j == num_blocks && last_len) ?
			last_len : AES_BLOCK_SIZE;

		/* Decryption followed by authentication */
		b_0[14] = (j >> 8) & 0xff;
		b_0[15] = j & 0xff;
		crypto_cipher_encrypt_one(tfm, b, b_0);
		for (i = 0; i < blen; i++) {
			*pos = *cpos++ ^ b[i];
			a[i] ^= *pos++;
		}
		crypto_cipher_encrypt_one(tfm, a, a);
	}

	for (i = 0; i < mic_len; i++) {
		if ((mic[i] ^ s_0[i]) != a[i])
			return -1;
	}

	return 0;
}

struct crypto_cipher *ieee80211_aes_key_setup_encrypt(const u8 key[],
						      size_t key_len,
						      size_t mic_len)
{
	struct crypto_cipher *tfm;

	tfm = crypto_alloc_cipher("aes", 0, CRYPTO_ALG_ASYNC);
	if (!IS_ERR(tfm))
		crypto_cipher_setkey(tfm, key, key_len);
	else 
		printk(KERN_WARNING "%s: failed\n",__func__);

	return tfm;
}


void ieee80211_aes_key_free(struct crypto_cipher *tfm)
{
	crypto_free_cipher(tfm);
}
