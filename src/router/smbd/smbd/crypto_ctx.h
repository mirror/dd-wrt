/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *   Copyright (C) 2019 Samsung Electronics Co., Ltd.
 */

#ifndef __CRYPTO_CTX_H__
#define __CRYPTO_CTX_H__

#include <crypto/hash.h>
#include <crypto/aead.h>

enum {
	CRYPTO_SHASH_HMACMD5	= 0,
	CRYPTO_SHASH_HMACSHA256,
	CRYPTO_SHASH_CMACAES,
	CRYPTO_SHASH_SHA512,
	CRYPTO_SHASH_MD4,
	CRYPTO_SHASH_MD5,
	CRYPTO_SHASH_MAX,
};

enum {
	CRYPTO_AEAD_AES128_GCM = 16,
	CRYPTO_AEAD_AES128_CCM,
	CRYPTO_AEAD_MAX,
};

enum {
	CRYPTO_BLK_ECBDES	= 32,
	CRYPTO_BLK_MAX,
};

struct smbd_crypto_ctx {
	struct list_head		list;

	struct shash_desc		*desc[CRYPTO_SHASH_MAX];
	struct crypto_aead		*ccmaes[CRYPTO_AEAD_MAX];
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0)
	struct blkcipher_desc		*blk_desc[CRYPTO_BLK_MAX];
#endif
};

#define CRYPTO_HMACMD5(c)	((c)->desc[CRYPTO_SHASH_HMACMD5])
#define CRYPTO_HMACSHA256(c)	((c)->desc[CRYPTO_SHASH_HMACSHA256])
#define CRYPTO_CMACAES(c)	((c)->desc[CRYPTO_SHASH_CMACAES])
#define CRYPTO_SHA512(c)	((c)->desc[CRYPTO_SHASH_SHA512])
#define CRYPTO_MD4(c)		((c)->desc[CRYPTO_SHASH_MD4])
#define CRYPTO_MD5(c)		((c)->desc[CRYPTO_SHASH_MD5])

#define CRYPTO_HMACMD5_TFM(c)	((c)->desc[CRYPTO_SHASH_HMACMD5]->tfm)
#define CRYPTO_HMACSHA256_TFM(c)\
				((c)->desc[CRYPTO_SHASH_HMACSHA256]->tfm)
#define CRYPTO_CMACAES_TFM(c)	((c)->desc[CRYPTO_SHASH_CMACAES]->tfm)
#define CRYPTO_SHA512_TFM(c)	((c)->desc[CRYPTO_SHASH_SHA512]->tfm)
#define CRYPTO_MD4_TFM(c)	((c)->desc[CRYPTO_SHASH_MD4]->tfm)
#define CRYPTO_MD5_TFM(c)	((c)->desc[CRYPTO_SHASH_MD5]->tfm)

#define CRYPTO_GCM(c)		((c)->ccmaes[CRYPTO_AEAD_AES128_GCM])
#define CRYPTO_CCM(c)		((c)->ccmaes[CRYPTO_AEAD_AES128_CCM])

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0)
#define CRYPTO_ECBDES(c)	((c)->blk_desc[CRYPTO_BLK_ECBDES])
#define CRYPTO_ECBDES_TFM(c)	((c)->blk_desc[CRYPTO_BLK_ECBDES]->tfm)
#endif

void smbd_release_crypto_ctx(struct smbd_crypto_ctx *ctx);

struct smbd_crypto_ctx *smbd_crypto_ctx_find_hmacmd5(void);
struct smbd_crypto_ctx *smbd_crypto_ctx_find_hmacsha256(void);
struct smbd_crypto_ctx *smbd_crypto_ctx_find_cmacaes(void);
struct smbd_crypto_ctx *smbd_crypto_ctx_find_sha512(void);
struct smbd_crypto_ctx *smbd_crypto_ctx_find_md4(void);
struct smbd_crypto_ctx *smbd_crypto_ctx_find_md5(void);

struct smbd_crypto_ctx *smbd_crypto_ctx_find_gcm(void);
struct smbd_crypto_ctx *smbd_crypto_ctx_find_ccm(void);

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0)
struct smbd_crypto_ctx *smbd_crypto_ctx_find_ecbdes(void);
#endif

void smbd_crypto_destroy(void);
int smbd_crypto_create(void);

#endif /* __CRYPTO_CTX_H__ */
