/*
 **************************************************************************
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

/*
 * nss_ovpnmgr_crypto.h
 */
#ifndef __NSS_OVPNMGR_CRYPTO__H
#define __NSS_OVPNMGR_CRYPTO__H

#define NSS_OVPNMGR_CRYPTO_INVALID_KEY_ID 0xff

/*
 * Crypto key types.
 */
enum nss_ovpnmgr_crypto_key_type {
	NSS_OVPNMGR_CRYPTO_KEY_TYPE_INITIAL,	/* Initial key which becomes active during tunnel addition. */
	NSS_OVPNMGR_CRYPTO_KEY_TYPE_CURRENT,	/* Current key. */
	NSS_OVPNMGR_CRYPTO_KEY_TYPE_EXPIRING	/* Key about to expire. */
};

/*
 * NSS OVPN manager supported crypto algorithms
 */
enum nss_ovpnmgr_crypto_type {
	NSS_OVPNMGR_CRYPTO_TYPE_AEAD = 1,	/* Crypto type AEAD */
	NSS_OVPNMGR_CRYPTO_TYPE_ABLK,		/* Crypto type ABLK */
	NSS_OVPNMGR_CRYPTO_TYPE_AHASH		/* Crypto type AHASH */
};

/*
 * nss_ovpnmgr_crypto_ctx
 */
struct nss_ovpnmgr_crypto_ctx {
	uint8_t blk_len;				/* Cipher block length. */
	uint8_t hash_len;				/* Hash length. */
	uint8_t iv_len;					/* IV length. */
	uint8_t key_id;					/* Crypto key id. */
	uint16_t crypto_idx;				/* Crypto Session Index. */

	enum nss_ovpnmgr_crypto_type crypto_type;	/* Crypto type. */
	union {
		struct crypto_aead *aead;		/* Cipher + Hash */
		struct crypto_skcipher *skcipher;	/* Cipher Only */
		struct crypto_ahash *ahash;		/* Hash Only */
	} tfm;
};

int nss_ovpnmgr_crypto_ctx_alloc(struct nss_ovpnmgr_crypto_ctx *ctx,
		struct nss_ovpnmgr_crypto_config *cfg, struct nss_ovpnmgr_crypto_key *key);
void nss_ovpnmgr_crypto_ctx_free(struct nss_ovpnmgr_crypto_ctx *crypto_ctx);
#endif /* __NSS_OVPNMGR_CRYPTO__H */
