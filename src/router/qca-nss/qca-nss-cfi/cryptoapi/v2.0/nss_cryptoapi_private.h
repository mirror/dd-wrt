/* Copyright (c) 2015-2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#define NSS_CRYPTOAPI_DEBUGFS_MAX_NAME 64
#define NSS_CRYPTOAPI_DEBUGFS_MAX_CTX_ENTRY 5
#define NSS_CRYPTOAPI_DEBUGFS_MAX_STATS_ENTRY 11
#define NSS_CRYPTOAPI_MAGIC 0x7FED
#define NSS_CRYPTOAPI_AHASH_MAGIC 0x7FEF
#define NSS_CRYPTOAPI_HDR_POOL_SZ 1024
#define NSS_CRYPTOAPI_DEFAULT_HDR_SZ 512
#define NSS_CRYPTOAPI_TIMEOUT 100 /* msecs */
#define NSS_CRYPTOAPI_MAX_IV (AES_BLOCK_SIZE/sizeof(uint32_t))
#define NSS_CRYPTOAPI_REQ_TIMEOUT_TICKS msecs_to_jiffies(NSS_CRYPTOAPI_REQ_TIMEO_SECS * MSEC_PER_SEC)

/*
 * Cipher mode
 */
enum nss_cryptoapi_cipher_mode {
	NSS_CRYPTOAPI_CIPHER_MODE_NONE = 0,		/* NULL cipher mode */
	NSS_CRYPTOAPI_CIPHER_MODE_ECB,			/* Electronic Code Block */
	NSS_CRYPTOAPI_CIPHER_MODE_CBC,			/* Cipher Block Chaining */
	NSS_CRYPTOAPI_CIPHER_MODE_GCM,
	NSS_CRYPTOAPI_CIPHER_MODE_CTR_RFC3686,		/* Counter Mode */
	NSS_CRYPTOAPI_CIPHER_MODE_GCM_RFC4106,			/* Galois Counter Mode */
	NSS_CRYPTOAPI_CIPHER_MODE_MAX
};

/*
 * Authentication mode
 */
enum nss_cryptoapi_auth_mode {
	NSS_CRYPTOAPI_AUTH_MODE_NONE = 0,	/* NULL Authentication */
	NSS_CRYPTOAPI_AUTH_MODE_HASH,		/* Hash message digest */
	NSS_CRYPTOAPI_AUTH_MODE_HMAC,		/* Keyed Hash message digest */
	NSS_CRYPTOAPI_AUTH_MODE_GMAC,		/* Keyed Galois message digest */
	NSS_CRYPTOAPI_AUTH_MODE_MAX
};

/*
 * CryptoAPI software context
 */
struct nss_cryptoapi {
	struct nss_crypto_user_ctx ctx;			/* Crypto user context */
	struct nss_crypto_user *user;			/* Crypto user handle */
	struct dentry *root;				/* Root debugfs entry */
	struct nss_cryptoapi_algo_info *algo_info;	/* algorithm table */
	atomic_t registered;				/* Registration flag */
};

/*
 * Ahash algorithm req specific context
 */
struct nss_cryptoapi_req_ctx {
	uint64_t msg_count;				/* Message count for this context */

	enum nss_cryptoapi_auth_mode auth_mode;		/* Auth mode */
	struct nss_cryptoapi_algo_info *info;		/* Algorithm specific info */

	uint32_t buf_count;				/* Buffer input count */
	uint32_t diglen;				/* Digest length */
	uint8_t digest[SHA512_DIGEST_SIZE];		/* Storage for resultant digest */

	uint16_t magic;					/* Magic */
};

/*
 * CryptoAPI statistics
 */
struct nss_cryptoapi_stats {
	uint64_t queued;				/* Packets queued for completion */
	uint64_t completed;				/* Packet completed */
	uint64_t init;					/* Init invoked */
	uint64_t exit;					/* Exit invoked */
	uint64_t failed_init;				/* Init failed */
	uint64_t failed_exit;				/* Exit failed */
	uint64_t failed_fallback;			/* Fallback */
	uint64_t failed_queue;				/* Packets failing enqueue */
	uint64_t failed_nomem;				/* Packets failing due to no memory */
	uint64_t failed_req;				/* Packets failing due incorrect request */
	uint64_t failed_align;				/* Packets failing alignment checks */
	uint64_t failed_len;				/* Packets failing Supported length checks */
	uint64_t error[NSS_CRYPTO_CMN_RESP_ERROR_MAX];	/* Other packet response errors */
};

/*
 * CryptoAPI context
 */
struct nss_cryptoapi_ctx {
	struct nss_cryptoapi_stats stats;	/* Statistics */
	struct nss_cryptoapi_algo_info *info;	/* Algorithm info */
	struct dentry *dentry;			/* Debugfs entry */
	struct nss_crypto_user *user;		/* Crypto user */
	struct crypto_tfm *sw_tfm;		/* SW fallback context */

	struct completion complete;		/* Completion object for outstanding packets */

	atomic_t active;			/* ctx status(active/inactive) */
	atomic_t refcnt;			/* ctx refcnt */

	uint32_t sid;				/* Session index */
	uint32_t ctx_iv[NSS_CRYPTOAPI_MAX_IV];	/* Initial IV for the context */

	uint16_t iv_size;			/* IV size */
	uint16_t authsize;			/* HMAC length */
	uint16_t magic;				/* Magic */
	bool fallback_req;			/* SW fallback required for algorithm */
};

/*
 * nss_cryptoapi_seg_info
 *	Structure to store the information specific to the scatterlist
 */
struct nss_cryptoapi_seg_info {
	struct scatterlist *first_sg;	/* First SG in the list */
	struct scatterlist *last_sg;	/* Last SG in the list */
	size_t nsegs;			/* Number of segments in the list */
};

/*
 * Request specific structure
 */
struct nss_cryptoapi_info {
	nss_crypto_req_callback_t cb;		/* Callback function */
	struct nss_cryptoapi_seg_info src;	/* Segment information for src SG list */
	struct nss_cryptoapi_seg_info dst;	/* Segment information for dst SG list */
	bool in_place;				/* Set to TRUE if src and dst are same */
	unsigned int total_in_len;		/* Total input length for the transformation */
	unsigned int total_out_len;		/* Total output length of the transformation */
	void *iv;				/* IV to use for transform */
	uint8_t skip;				/* Cipher skip */
	uint8_t auth;				/* auth len */
	uint8_t iv_size;			/* IV size */
	uint8_t hmac_len;			/* HMAC length */
	uint8_t ahash_skip;			/* AHASH data length */
	enum nss_crypto_op_dir op_dir;		/* Operation direction */
};

typedef void (*nss_cryptoapi_aead_tx_proc_method_t)(struct nss_cryptoapi_ctx *ctx,
		struct aead_request *req, struct nss_cryptoapi_info *info, bool encrypt);

/*
 * NSS Crypto specific structure to store the details of supported algorithms
 */
struct nss_cryptoapi_algo_info {
	const char *cra_name;				/* CryptoAPI name */
	uint16_t cipher_keylen;				/* Cipher key length */
	uint16_t digest_sz;				/* Authentication digest size */
	uint16_t auth_blocksize;			/* Authentication block length */
	uint16_t nonce;					/* nonce value for counter mode */
	enum nss_crypto_cmn_algo algo;			/* NSS crypto algorithm ID */
	enum nss_cryptoapi_cipher_mode cipher_mode;	/* Cipher mode */
	enum nss_cryptoapi_auth_mode auth_mode;		/* Authentication mode */
	nss_cryptoapi_aead_tx_proc_method_t aead_tx_proc; /* Tx processing function */
	bool blk_align;					/* Allow buffer length unaligned to block */
};

#if defined NSS_CFI_DEBUG

/*
 * nss_cryptoapi_verify_magic()
 *	verify magic
 */
static inline void nss_cryptoapi_verify_magic(struct nss_cryptoapi_ctx *ctx)
{
	BUG_ON(unlikely(ctx->magic != NSS_CRYPTOAPI_MAGIC));
}

/*
 * nss_cryptoapi_set_magic()
 *	set magic
 */
static inline void nss_cryptoapi_set_magic(struct nss_cryptoapi_ctx *ctx)
{
	ctx->magic = NSS_CRYPTOAPI_MAGIC;
}

/*
 * nss_cryptoapi_clear_magic()
 *	clear magic
 */
static inline void nss_cryptoapi_clear_magic(struct nss_cryptoapi_ctx *ctx)
{
	ctx->magic = 0;
}

#define NSS_CRYPTOAPI_VERIFY_MAGIC(ctx) nss_cryptoapi_verify_magic(ctx)
#define NSS_CRYPTOAPI_SET_MAGIC(ctx) nss_cryptoapi_set_magic(ctx)
#define NSS_CRYPTOAPI_CLEAR_MAGIC(ctx) nss_cryptoapi_clear_magic(ctx)
#else
#define NSS_CRYPTOAPI_VERIFY_MAGIC(ctx)
#define NSS_CRYPTOAPI_SET_MAGIC(ctx)
#define NSS_CRYPTOAPI_CLEAR_MAGIC(ctx)
#endif /* !NSS_CFI_DEBUG */


/*
 * function prototypes
 */
extern struct nss_cryptoapi g_cryptoapi;
extern void nss_cryptoapi_ref_dec(struct nss_cryptoapi_ctx *ctx);
extern int nss_cryptoapi_status2error(struct nss_cryptoapi_ctx *ctx, uint8_t status);
extern struct nss_cryptoapi_algo_info *nss_cryptoapi_cra_name2info(const char *cra_name, uint16_t enc_keylen, uint16_t digest_sz);
extern int nss_cryptoapi_transform(struct nss_cryptoapi_ctx *ctx, struct nss_cryptoapi_info *info, void *app_data, bool ahash);
/*
 * Debug fs
 */
extern void nss_cryptoapi_add_ctx2debugfs(struct nss_cryptoapi_ctx *ctx);

/*
 * AEAD
 */
extern int nss_cryptoapi_aead_init(struct crypto_aead *aead);
extern void nss_cryptoapi_aead_exit(struct crypto_aead *aead);
extern int nss_cryptoapi_aead_setkey(struct crypto_aead *tfm, const u8 *key, unsigned int keylen);
extern int nss_cryptoapi_aead_setkey_noauth(struct crypto_aead *tfm, const u8 *key, unsigned int keylen);
extern int nss_cryptoapi_aead_setauthsize(struct crypto_aead *authenc, unsigned int authsize);
extern int nss_cryptoapi_aead_encrypt(struct aead_request *req);
extern int nss_cryptoapi_aead_decrypt(struct aead_request *req);
extern void nss_cryptoapi_aead_echainiv_tx_proc(struct nss_cryptoapi_ctx *ctx, struct aead_request *req,
				struct nss_cryptoapi_info *info, bool encrypt);
extern void nss_cryptoapi_aead_seqiv_tx_proc(struct nss_cryptoapi_ctx *ctx, struct aead_request *req,
				struct nss_cryptoapi_info *info, bool encrypt);
extern void nss_cryptoapi_aead_tx_proc(struct nss_cryptoapi_ctx *ctx, struct aead_request *req,
				struct nss_cryptoapi_info *info, bool encrypt);

/*
 * ABLKCIPHER
 */
extern int nss_cryptoapi_ablkcipher_init(struct crypto_tfm *tfm);
extern void nss_cryptoapi_ablkcipher_exit(struct crypto_tfm *tfm);
extern int nss_cryptoapi_ablk_setkey(struct crypto_ablkcipher *cipher, const u8 *key, unsigned int len);
extern int nss_cryptoapi_ablk_encrypt(struct ablkcipher_request *req);
extern int nss_cryptoapi_ablk_decrypt(struct ablkcipher_request *req);
extern void nss_cryptoapi_copy_iv(struct nss_cryptoapi_ctx *ctx, struct scatterlist *sg, uint8_t *iv, uint8_t iv_len);

/*
 * AHASH
 */
extern int nss_cryptoapi_ahash_cra_init(struct crypto_tfm *tfm);
extern void nss_cryptoapi_ahash_cra_exit(struct crypto_tfm *tfm);
extern int nss_cryptoapi_ahash_setkey(struct crypto_ahash *ahash, const u8 *key, unsigned int keylen);
extern int nss_cryptoapi_ahash_init(struct ahash_request *req);
extern int nss_cryptoapi_ahash_update(struct ahash_request *req);
extern int nss_cryptoapi_ahash_final(struct ahash_request *req);
extern int nss_cryptoapi_ahash_digest(struct ahash_request *req);
extern int nss_cryptoapi_ahash_export(struct ahash_request *req, void *out);
extern int nss_cryptoapi_ahash_import(struct ahash_request *req, const void *in);
#endif /* !__NSS_CRYPTOAPI_PRIVATE_H */
