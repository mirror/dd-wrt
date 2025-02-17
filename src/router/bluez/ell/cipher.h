/*
 * Embedded Linux library
 * Copyright (C) 2015  Intel Corporation
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __ELL_CIPHER_H
#define __ELL_CIPHER_H

#ifdef __cplusplus
extern "C" {
#endif

struct l_cipher;

enum l_cipher_type {
	L_CIPHER_AES = 0,
	L_CIPHER_AES_CBC,
	L_CIPHER_AES_CTR,
	L_CIPHER_ARC4,
	L_CIPHER_DES,
	L_CIPHER_DES_CBC,
	L_CIPHER_DES3_EDE_CBC,
	L_CIPHER_RC2_CBC,
};

struct l_cipher *l_cipher_new(enum l_cipher_type type,
				const void *key, size_t key_length);

void l_cipher_free(struct l_cipher *cipher);

bool l_cipher_encrypt(struct l_cipher *cipher,
			const void *in, void *out, size_t len);
bool l_cipher_encryptv(struct l_cipher *cipher,
				const struct iovec *in, size_t in_cnt,
				const struct iovec *out, size_t out_cnt);

bool l_cipher_decrypt(struct l_cipher *cipher,
			const void *in, void *out, size_t len);
bool l_cipher_decryptv(struct l_cipher *cipher,
				const struct iovec *in, size_t in_cnt,
				const struct iovec *out, size_t out_cnt);

bool l_cipher_set_iv(struct l_cipher *cipher, const uint8_t *iv,
			size_t iv_length);

struct l_aead_cipher;

enum l_aead_cipher_type {
	L_AEAD_CIPHER_AES_CCM = 0,
	L_AEAD_CIPHER_AES_GCM,
};

struct l_aead_cipher *l_aead_cipher_new(enum l_aead_cipher_type type,
					const void *key, size_t key_length,
					size_t tag_length);

void l_aead_cipher_free(struct l_aead_cipher *cipher);

bool l_aead_cipher_encrypt(struct l_aead_cipher *cipher,
				const void *in, size_t in_len,
				const void *ad, size_t ad_len,
				const void *nonce, size_t nonce_len,
				void *out, size_t out_len);

bool l_aead_cipher_decrypt(struct l_aead_cipher *cipher,
				const void *in, size_t in_len,
				const void *ad, size_t ad_len,
				const void *nonce, size_t nonce_len,
				void *out, size_t out_len);

bool l_cipher_is_supported(enum l_cipher_type type);
bool l_aead_cipher_is_supported(enum l_aead_cipher_type type);

#ifdef __cplusplus
}
#endif

#endif /* __ELL_CIPHER_H */
