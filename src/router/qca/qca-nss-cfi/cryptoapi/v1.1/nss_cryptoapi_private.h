/* Copyright (c) 2015-2018, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 *
 */

#ifndef __NSS_CRYPTOAPI_PRIVATE_H
#define __NSS_CRYPTOAPI_PRIVATE_H

/**
 * nss_cryptoapi.h
 *	Cryptoapi (Linux Crypto API framework) specific nss cfi header file
 */

#define NSS_CRYPTOAPI_MAX_DATA_LEN	((uint16_t) -1)
#define nss_cryptoapi_sg_has_frags(s)	sg_next(s)

#define NSS_CRYPTOAPI_DEBUGFS_NAME_SZ	64
#define NSS_CRYPTOAPI_MAGIC		0x7FED

/*
 * Key lengths for supported algorithms
 */
enum nss_cryptoapi_keylen {
	NSS_CRYPTOAPI_KEYLEN_AES128 = 16,
	NSS_CRYPTOAPI_KEYLEN_AES256 = 32,
	NSS_CRYPTOAPI_KEYLEN_AES192 = 24,
	NSS_CRYPTOAPI_KEYLEN_3DES = 24,
};

enum nss_cryptoapi_fallback_type {
	NSS_CRYPTOAPI_ENCRYPT,
	NSS_CRYPTOAPI_DECRYPT,
};

/*
 * Cryptoapi sg virtual addresses used during Encryption/Decryption operations
 */
struct nss_cryptoapi_addr {
	uint8_t *src;
	uint8_t *dst;
	uint8_t *iv;
	uint8_t *start;
};

/*
 * Framework specific handle this will be used to communicate framework
 * specific data to Core specific data
 */
struct nss_cryptoapi {
	nss_crypto_handle_t crypto;		/* crypto handle */
	struct dentry *root_dentry;
	struct dentry *stats_dentry;
	atomic_t registered;			/* Registration flag */
};

struct nss_cryptoapi_ctx {
	uint64_t queued;
	uint64_t completed;
	uint64_t queue_failed;
	struct dentry *session_dentry;
	struct crypto_tfm *sw_tfm;
	uint32_t sid;
	atomic_t refcnt;
	uint16_t authsize;
	uint16_t magic;
	uint32_t ctx_iv[AES_BLOCK_SIZE/sizeof(uint32_t)];
	bool fallback_req;
	enum nss_crypto_cipher cip_alg;
	enum nss_crypto_auth auth_alg;
	enum nss_crypto_req_type op;
};

static inline void nss_cryptoapi_verify_magic(struct nss_cryptoapi_ctx *ctx)
{
	BUG_ON(unlikely(ctx->magic != NSS_CRYPTOAPI_MAGIC));
}

static inline void nss_cryptoapi_set_magic(struct nss_cryptoapi_ctx *ctx)
{
	ctx->magic = NSS_CRYPTOAPI_MAGIC;
}

static inline void nss_cryptoapi_clear_magic(struct nss_cryptoapi_ctx *ctx)
{
	ctx->magic = 0;
}

static inline bool nss_cryptoapi_is_decrypt(struct nss_cryptoapi_ctx *ctx)
{
	return ctx->op & NSS_CRYPTO_REQ_TYPE_DECRYPT;
}

/*
 * nss_cryptoapi_check_unalign()
 *	Cryptoapi verify if length is aligned to boundary.
 */
static inline bool nss_cryptoapi_check_unalign(uint32_t len, uint32_t boundary)
{
	return !!(len & (boundary - 1));
}

/*
 * function prototypes
 */

/* Debug fs */
void nss_cryptoapi_debugfs_add_stats(struct dentry *parent, struct nss_cryptoapi_ctx *session_ctx);
void nss_cryptoapi_debugfs_add_session(struct nss_cryptoapi *gbl_ctx, struct nss_cryptoapi_ctx *session_ctx);
void nss_cryptoapi_debugfs_del_session(struct nss_cryptoapi_ctx *session_ctx);
void nss_cryptoapi_debugfs_init(struct nss_cryptoapi *gbl_ctx);
void nss_cryptoapi_debugfs_exit(struct nss_cryptoapi *gbl_ctx);

/* AEAD */
int nss_cryptoapi_aead_init(struct crypto_aead *aead);
void nss_cryptoapi_aead_exit(struct crypto_aead *aead);
int nss_cryptoapi_aead_aes_setkey(struct crypto_aead *tfm, const u8 *key, unsigned int keylen);
int nss_cryptoapi_sha1_3des_setkey(struct crypto_aead *tfm, const u8 *key, unsigned int keylen);
int nss_cryptoapi_sha256_3des_setkey(struct crypto_aead *tfm, const u8 *key, unsigned int keylen);

int nss_cryptoapi_aead_setauthsize(struct crypto_aead *authenc, unsigned int authsize);
int nss_cryptoapi_aead_aes_encrypt(struct aead_request *req);
int nss_cryptoapi_aead_aes_decrypt(struct aead_request *req);

int nss_cryptoapi_sha1_3des_encrypt(struct aead_request *req);
int nss_cryptoapi_sha1_3des_decrypt(struct aead_request *req);

int nss_cryptoapi_sha256_3des_encrypt(struct aead_request *req);
int nss_cryptoapi_sha256_3des_decrypt(struct aead_request *req);

/* ABLKCIPHER */
int nss_cryptoapi_skcipher_init(struct crypto_skcipher *tfm);
void nss_cryptoapi_skcipher_exit(struct crypto_skcipher *tfm);
int nss_cryptoapi_ablk_aes_setkey(struct crypto_skcipher *cipher, const u8 *key, unsigned int len);
int nss_cryptoapi_3des_cbc_setkey(struct crypto_skcipher *cipher, const u8 *key, unsigned int len);

int nss_cryptoapi_ablk_aes_encrypt(struct skcipher_request *req);
int nss_cryptoapi_ablk_aes_decrypt(struct skcipher_request *req);

int nss_cryptoapi_3des_cbc_encrypt(struct skcipher_request *req);
int nss_cryptoapi_3des_cbc_decrypt(struct skcipher_request *req);

#endif /* __NSS_CRYPTOAPI_PRIVATE_H */

