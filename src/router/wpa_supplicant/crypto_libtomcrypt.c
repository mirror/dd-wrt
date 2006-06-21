/*
 * WPA Supplicant / Crypto wrapper for LibTomCrypt (for internal TLSv1)
 * Copyright (c) 2005-2006, Jouni Malinen <jkmaline@cc.hut.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#include "includes.h"
#include <tomcrypt.h>

#include "common.h"
#include "rc4.h"
#include "crypto.h"


void md4_vector(size_t num_elem, const u8 *addr[], const size_t *len, u8 *mac)
{
	hash_state md;
	size_t i;

	md4_init(&md);
	for (i = 0; i < num_elem; i++)
		md4_process(&md, addr[i], len[i]);
	md4_done(&md, mac);
}


void des_encrypt(const u8 *clear, const u8 *key, u8 *cypher)
{
	u8 pkey[8], next, tmp;
	int i;
	symmetric_key skey;

	/* Add parity bits to the key */
	next = 0;
	for (i = 0; i < 7; i++) {
		tmp = key[i];
		pkey[i] = (tmp >> i) | next | 1;
		next = tmp << (7 - i);
	}
	pkey[i] = next | 1;

	des_setup(pkey, 8, 0, &skey);
	des_ecb_encrypt(clear, cypher, &skey);
	des_done(&skey);
}


#ifdef EAP_TLS_FUNCS
void md5_vector(size_t num_elem, const u8 *addr[], const size_t *len, u8 *mac)
{
	hash_state md;
	size_t i;

	md5_init(&md);
	for (i = 0; i < num_elem; i++)
		md5_process(&md, addr[i], len[i]);
	md5_done(&md, mac);
}


void sha1_vector(size_t num_elem, const u8 *addr[], const size_t *len, u8 *mac)
{
	hash_state md;
	size_t i;

	sha1_init(&md);
	for (i = 0; i < num_elem; i++)
		sha1_process(&md, addr[i], len[i]);
	sha1_done(&md, mac);
}


void * aes_encrypt_init(const u8 *key, size_t len)
{
	symmetric_key *skey;
	skey = malloc(sizeof(*skey));
	if (skey == NULL)
		return NULL;
	if (aes_setup(key, len, 0, skey) != CRYPT_OK) {
		free(skey);
		return NULL;
	}
	return skey;
}


void aes_encrypt(void *ctx, const u8 *plain, u8 *crypt)
{
	symmetric_key *skey = ctx;
	aes_ecb_encrypt(plain, crypt, skey);
}


void aes_encrypt_deinit(void *ctx)
{
	symmetric_key *skey = ctx;
	aes_done(skey);
	free(skey);
}


void * aes_decrypt_init(const u8 *key, size_t len)
{
	symmetric_key *skey;
	skey = malloc(sizeof(*skey));
	if (skey == NULL)
		return NULL;
	if (aes_setup(key, len, 0, skey) != CRYPT_OK) {
		free(skey);
		return NULL;
	}
	return skey;
}


void aes_decrypt(void *ctx, const u8 *crypt, u8 *plain)
{
	symmetric_key *skey = ctx;
	aes_ecb_encrypt(plain, (u8 *) crypt, skey);
}


void aes_decrypt_deinit(void *ctx)
{
	symmetric_key *skey = ctx;
	aes_done(skey);
	free(skey);
}


#ifdef CONFIG_TLS_INTERNAL

static int ltc_prng_idx = -1;
static prng_state ltc_prng_state;


struct crypto_hash {
	enum crypto_hash_alg alg;
	int error;
	union {
		hash_state md;
		hmac_state hmac;
	} u;
};


struct crypto_hash * crypto_hash_init(enum crypto_hash_alg alg, const u8 *key,
				      size_t key_len)
{
	struct crypto_hash *ctx;

	ctx = wpa_zalloc(sizeof(*ctx));
	if (ctx == NULL)
		return NULL;

	ctx->alg = alg;

	switch (alg) {
	case CRYPTO_HASH_ALG_MD5:
		if (md5_init(&ctx->u.md) != CRYPT_OK)
			goto fail;
		break;
	case CRYPTO_HASH_ALG_SHA1:
		if (sha1_init(&ctx->u.md) != CRYPT_OK)
			goto fail;
		break;
	case CRYPTO_HASH_ALG_HMAC_MD5:
		if (hmac_init(&ctx->u.hmac, find_hash("md5"), key, key_len) !=
		    CRYPT_OK)
			goto fail;
		break;
	case CRYPTO_HASH_ALG_HMAC_SHA1:
		if (hmac_init(&ctx->u.hmac, find_hash("sha1"), key, key_len) !=
		    CRYPT_OK)
			goto fail;
		break;
	default:
		goto fail;
	}

	return ctx;

fail:
	free(ctx);
	return NULL;
}

void crypto_hash_update(struct crypto_hash *ctx, const u8 *data, size_t len)
{
	if (ctx == NULL || ctx->error)
		return;

	switch (ctx->alg) {
	case CRYPTO_HASH_ALG_MD5:
		ctx->error = md5_process(&ctx->u.md, data, len) != CRYPT_OK;
		break;
	case CRYPTO_HASH_ALG_SHA1:
		ctx->error = sha1_process(&ctx->u.md, data, len) != CRYPT_OK;
		break;
	case CRYPTO_HASH_ALG_HMAC_MD5:
	case CRYPTO_HASH_ALG_HMAC_SHA1:
		ctx->error = hmac_process(&ctx->u.hmac, data, len) != CRYPT_OK;
		break;
	}
}


int crypto_hash_finish(struct crypto_hash *ctx, u8 *mac, size_t *len)
{
	int ret = 0;
	unsigned long clen;

	if (ctx == NULL)
		return -2;

	if (mac == NULL || len == NULL) {
		free(ctx);
		return 0;
	}

	if (ctx->error) {
		free(ctx);
		return -2;
	}

	switch (ctx->alg) {
	case CRYPTO_HASH_ALG_MD5:
		if (*len < 16) {
			*len = 16;
			free(ctx);
			return -1;
		}
		*len = 16;
		if (md5_done(&ctx->u.md, mac) != CRYPT_OK)
			ret = -2;
		break;
	case CRYPTO_HASH_ALG_SHA1:
		if (*len < 20) {
			*len = 20;
			free(ctx);
			return -1;
		}
		*len = 20;
		if (sha1_done(&ctx->u.md, mac) != CRYPT_OK)
			ret = -2;
		break;
	case CRYPTO_HASH_ALG_HMAC_SHA1:
		if (*len < 20) {
			*len = 20;
			free(ctx);
			return -1;
		}
		/* continue */
	case CRYPTO_HASH_ALG_HMAC_MD5:
		if (*len < 16) {
			*len = 16;
			free(ctx);
			return -1;
		}
		clen = *len;
		if (hmac_done(&ctx->u.hmac, mac, &clen) != CRYPT_OK) {
			free(ctx);
			return -1;
		}
		*len = clen;
		break;
	default:
		ret = -2;
		break;
	}

	free(ctx);

	return ret;
}


struct crypto_cipher {
	int rc4;
	union {
		symmetric_CBC cbc;
		struct {
			size_t used_bytes;
			u8 key[16];
			size_t keylen;
		} rc4;
	} u;
};


struct crypto_cipher * crypto_cipher_init(enum crypto_cipher_alg alg,
					  const u8 *iv, const u8 *key,
					  size_t key_len)
{	
	struct crypto_cipher *ctx;
	int idx, res, rc4 = 0;

	switch (alg) {
	case CRYPTO_CIPHER_ALG_AES:
		idx = find_cipher("aes");
		break;
	case CRYPTO_CIPHER_ALG_3DES:
		idx = find_cipher("3des");
		break;
	case CRYPTO_CIPHER_ALG_DES:
		idx = find_cipher("des");
		break;
	case CRYPTO_CIPHER_ALG_RC2:
		idx = find_cipher("rc2");
		break;
	case CRYPTO_CIPHER_ALG_RC4:
		idx = -1;
		rc4 = 1;
		break;
	default:
		return NULL;
	}

	ctx = wpa_zalloc(sizeof(*ctx));
	if (ctx == NULL)
		return NULL;

	if (rc4) {
		ctx->rc4 = 1;
		if (key_len > sizeof(ctx->u.rc4.key)) {
			free(ctx);
			return NULL;
		}
		ctx->u.rc4.keylen = key_len;
		memcpy(ctx->u.rc4.key, key, key_len);
	} else {
		res = cbc_start(idx, iv, key, key_len, 0, &ctx->u.cbc);
		if (res != CRYPT_OK) {
			wpa_printf(MSG_DEBUG, "LibTomCrypt: Cipher start "
				   "failed: %s", error_to_string(res));
			free(ctx);
			return NULL;
		}
	}

	return ctx;
}

int crypto_cipher_encrypt(struct crypto_cipher *ctx, const u8 *plain,
			  u8 *crypt, size_t len)
{
	int res;

	if (ctx->rc4) {
		if (plain != crypt)
			memcpy(crypt, plain, len);
		rc4_skip(ctx->u.rc4.key, ctx->u.rc4.keylen,
			 ctx->u.rc4.used_bytes, crypt, len);
		ctx->u.rc4.used_bytes += len;
		return 0;
	}

	res = cbc_encrypt(plain, crypt, len, &ctx->u.cbc);
	if (res != CRYPT_OK) {
		wpa_printf(MSG_DEBUG, "LibTomCrypt: CBC encryption "
			   "failed: %s", error_to_string(res));
		return -1;
	}
	return 0;
}


int crypto_cipher_decrypt(struct crypto_cipher *ctx, const u8 *crypt,
			  u8 *plain, size_t len)
{
	int res;

	if (ctx->rc4) {
		if (plain != crypt)
			memcpy(plain, crypt, len);
		rc4_skip(ctx->u.rc4.key, ctx->u.rc4.keylen,
			 ctx->u.rc4.used_bytes, plain, len);
		ctx->u.rc4.used_bytes += len;
		return 0;
	}

	res = cbc_decrypt(crypt, plain, len, &ctx->u.cbc);
	if (res != CRYPT_OK) {
		wpa_printf(MSG_DEBUG, "LibTomCrypt: CBC decryption "
			   "failed: %s", error_to_string(res));
		return -1;
	}

	return 0;
}


void crypto_cipher_deinit(struct crypto_cipher *ctx)
{
	if (!ctx->rc4)
		cbc_done(&ctx->u.cbc);
	free(ctx);
}


struct crypto_public_key {
	rsa_key rsa;
};


struct crypto_public_key * crypto_public_key_import(const u8 *key, size_t len)
{
	int res;
	struct crypto_public_key *pk;

	pk = wpa_zalloc(sizeof(*pk));
	if (pk == NULL)
		return NULL;

	res = rsa_import(key, len, &pk->rsa);
	if (res != CRYPT_OK) {
		wpa_printf(MSG_ERROR, "LibTomCrypt: Failed to import "
			   "public key (res=%d '%s')",
			   res, error_to_string(res));
		free(pk);
		return NULL;
	}

	if (pk->rsa.type != PK_PUBLIC) {
		wpa_printf(MSG_ERROR, "LibTomCrypt: Public key was not of "
			   "correct type");
		rsa_free(&pk->rsa);
		free(pk);
		return NULL;
	}

	return pk;
}


struct crypto_public_key * crypto_public_key_from_cert(const u8 *buf,
						       size_t len)
{
	/* No X.509 support in LibTomCrypt */
	return NULL;
}


/*
 * PKCS #1 v1.5 version of RSA encryption was unfortunately removed from
 * LibTomCrypt in v1.03, so let's maintain a local copy of the needed functions
 * here since TLSv1 uses the old version of PKCS #1. These are based on the
 * code from LibTomCrypt v1.02 that was released in public domain by
 * Tom St Denis.
 */

#define mp_count_bits(a)		ltc_mp.count_bits(a)
#define mp_unsigned_bin_size(a)		ltc_mp.unsigned_size(a)

static int pkcs_1_v15_es_encode(const unsigned char *msg, unsigned long msglen,
				unsigned long modulus_bitlen,
				prng_state *prng, int prng_idx,
				unsigned char *out, unsigned long *outlen)
{
	unsigned long modulus_bytelen, x, y, i;

	/* get modulus len */
	modulus_bytelen = (modulus_bitlen >> 3) + (modulus_bitlen & 7 ? 1 : 0);
	if (modulus_bytelen < 12)
		return CRYPT_INVALID_ARG;

	/* verify length */
	if (msglen > (modulus_bytelen - 11) || *outlen < modulus_bytelen)
		return CRYPT_PK_INVALID_SIZE;

	/* 0x00 0x02 PS 0x00 M */
	x = 0;
	out[x++] = 0x00;
	out[x++] = 0x02;
	y = modulus_bytelen - msglen - 3;
	if (prng_descriptor[prng_idx].read(out + x, y, prng) != y)
		return CRYPT_ERROR_READPRNG;
	for (i = 0; i < y; i++) {
		/*
		 * PKCS #1 v1.5 block type 02: PS is pseusorandomly generated
		 * and each octet is nonzero. Change zeroes to ones to avoid
		 * including zeroes here.
		 */
		if (out[x + i] == 0)
			out[x + i] = 1;
	}

	x += y;
	out[x++] = 0x00;
	memcpy(out + x, msg, msglen);
	*outlen = modulus_bytelen;

	return CRYPT_OK;
}


static int rsa_v15_encrypt_key(const unsigned char *in, unsigned long inlen,
			       unsigned char *out, unsigned long *outlen,
			       prng_state *prng, int prng_idx, rsa_key *key)
{
	unsigned long modulus_bitlen, modulus_bytelen, x;
	int err;

	/* valid prng? */
	err = prng_is_valid(prng_idx);
	if (err != CRYPT_OK)
		return err;

	/* get modulus len in bits */
	modulus_bitlen = mp_count_bits(key->N);

	/* outlen must be at least the size of the modulus */
	modulus_bytelen = mp_unsigned_bin_size(key->N);
	if (modulus_bytelen > *outlen)
		return CRYPT_BUFFER_OVERFLOW;

	/* pad it */
	x = *outlen;
	err = pkcs_1_v15_es_encode(in, inlen, modulus_bitlen, prng, prng_idx,
				   out, &x);
	if (err != CRYPT_OK)
		return err;

	/* encrypt it */
	return rsa_exptmod(out, x, out, outlen, PK_PUBLIC, key);
}


int crypto_public_key_encrypt_pkcs1_v15(struct crypto_public_key *key,
					const u8 *in, size_t inlen,
					u8 *out, size_t *outlen)
{
	unsigned long clen;
	int res;

	clen = *outlen;
	res = rsa_v15_encrypt_key(in, inlen, out, &clen, &ltc_prng_state,
				  ltc_prng_idx, &key->rsa);
	if (res != CRYPT_OK) {
		wpa_printf(MSG_DEBUG, "LibTomCrypt: RSA encryption failed: %s",
			   error_to_string(res));
		return -1;
	}
	*outlen = clen;

	return 0;
}


void crypto_public_key_free(struct crypto_public_key *key)
{
	if (key) {
		rsa_free(&key->rsa);
		free(key);
	}
}


int crypto_global_init(void)
{
	int res;
	u8 buf[32];

	ltc_mp = tfm_desc;
	/* TODO: only register algorithms that are really needed */
	if (register_hash(&md4_desc) < 0 ||
	    register_hash(&md5_desc) < 0 ||
	    register_hash(&sha1_desc) < 0 ||
	    register_cipher(&aes_desc) < 0 ||
	    register_cipher(&des_desc) < 0 ||
	    register_cipher(&des3_desc) < 0 ||
	    register_prng(&fortuna_desc) < 0) {
		wpa_printf(MSG_ERROR, "TLSv1: Failed to register "
			   "hash/cipher/prng functions");
		return -1;
	}
	ltc_prng_idx = find_prng("fortuna");
	if (ltc_prng_idx < 0) {
		wpa_printf(MSG_ERROR, "TLSv1: Failed to select PRNG");
		return -1;
	}
	res =  rng_make_prng(128, ltc_prng_idx, &ltc_prng_state, NULL);
	if (res != CRYPT_OK) {
		wpa_printf(MSG_ERROR, "TLSv1: Failed to initialize "
			   "PRNG: %s", error_to_string(res));
		return -1;
	}

	os_get_random(buf, sizeof(buf));
	fortuna_add_entropy(buf, sizeof(buf), &ltc_prng_state);

	return 0;
}


void crypto_global_deinit(void)
{
}


#ifdef EAP_FAST

#ifndef mp_init_multi
#define mp_init_multi                ltc_init_multi
#define mp_clear_multi               ltc_deinit_multi
#define mp_unsigned_bin_size(a)      ltc_mp.unsigned_size(a)
#define mp_to_unsigned_bin(a, b)     ltc_mp.unsigned_write(a, b)
#define mp_read_unsigned_bin(a, b, c) ltc_mp.unsigned_read(a, b, c)
#define mp_exptmod(a,b,c,d)          ltc_mp.exptmod(a,b,c,d)
#endif

int crypto_mod_exp(const u8 *base, size_t base_len,
		   const u8 *power, size_t power_len,
		   const u8 *modulus, size_t modulus_len,
		   u8 *result, size_t *result_len)
{
	void *b, *p, *m, *r;

	if (mp_init_multi(&b, &p, &m, &r, NULL) != CRYPT_OK)
		return -1;

	if (mp_read_unsigned_bin(b, (u8 *) base, base_len) != CRYPT_OK ||
	    mp_read_unsigned_bin(p, (u8 *) power, power_len) != CRYPT_OK ||
	    mp_read_unsigned_bin(m, (u8 *) modulus, modulus_len) != CRYPT_OK)
		goto fail;

	if (mp_exptmod(b, p, m, r) != CRYPT_OK)
		goto fail;

	*result_len = mp_unsigned_bin_size(r);
	if (mp_to_unsigned_bin(r, result) != CRYPT_OK)
		goto fail;

	mp_clear_multi(b, p, m, r, NULL);
	return 0;

fail:
	mp_clear_multi(b, p, m, r, NULL);
	return -1;
}

#endif /* EAP_FAST */

#endif /* CONFIG_TLS_INTERNAL */

#endif /* EAP_TLS_FUNCS */
