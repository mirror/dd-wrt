/* Copyright (c) 2017-2018, 2020 The Linux Foundation. All rights reserved.
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

/**
 * nss_crypto_ahash.c
 *	Interface to communicate Native Linux crypto framework specific data
 *	to Crypto core specific data
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/random.h>
#include <linux/scatterlist.h>
#include <linux/moduleparam.h>
#include <linux/spinlock.h>
#include <asm/cmpxchg.h>
#include <linux/delay.h>
#include <linux/crypto.h>
#include <linux/debugfs.h>

#include <crypto/aes.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0)
#include <crypto/sha.h>
#else
#include <crypto/sha1.h>
#include <crypto/sha2.h>
#endif
#include <crypto/hash.h>
#include <crypto/algapi.h>
#include <crypto/scatterwalk.h>
#include <crypto/internal/hash.h>

#include <nss_api_if.h>
#include <nss_crypto_if.h>
#include <nss_cfi_if.h>
#include "nss_cryptoapi_private.h"

extern struct nss_cryptoapi gbl_ctx;

static uint8_t dummy_key[NSS_CRYPTO_MAX_KEYLEN_AES] = {
	0x60, 0x3d, 0xeb, 0x10,
	0x15, 0xca, 0x71, 0xbe,
	0x2b, 0x73, 0xae, 0xf0,
	0x85, 0x7d, 0x77, 0x81,
	0x1f, 0x35, 0x2c, 0x07,
	0x3b, 0x61, 0x08, 0xd7,
	0x2d, 0x98, 0x10, 0xa3,
	0x09, 0x14, 0xdf, 0xf4
};

static uint8_t dummy_iv[NSS_CRYPTO_MAX_IVLEN_AES] = {
	0x00, 0x01, 0x02, 0x03,
	0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b,
	0x0c, 0x0d, 0x0e, 0x0f
};

struct nss_cryptoapi_ahash_info {
	struct nss_crypto_params *params;
	nss_crypto_comp_t cb_fn;
};

/*
 * nss_cryptoapi_ahash_cra_init()
 *	Cryptoapi ahash init function.
 */
int nss_cryptoapi_ahash_cra_init(struct crypto_tfm *tfm)
{
	struct nss_cryptoapi_ctx *ctx = crypto_tfm_ctx(tfm);
	struct crypto_ahash *ahash = __crypto_ahash_cast(tfm);

	nss_cfi_assert(ctx);

	ctx->sid = NSS_CRYPTO_MAX_IDXS;
	ctx->queued = 0;
	ctx->completed = 0;
	ctx->queue_failed = 0;
	ctx->fallback_req = 0;
	atomic_set(&ctx->refcnt, 0);

	/*
	 * set this tfm reqsize to the transform specific structure
	 */
	crypto_ahash_set_reqsize(ahash, sizeof(struct nss_cryptoapi_req_ctx));
	nss_cryptoapi_set_magic(ctx);

	return 0;
};

/*
 * nss_cryptoapi_ahash_cra_exit()
 *	Cryptoapi ahash exit function.
 */
void nss_cryptoapi_ahash_cra_exit(struct crypto_tfm *tfm)
{
	struct nss_cryptoapi_ctx *ctx = crypto_tfm_ctx(tfm);
	struct nss_cryptoapi *sc = &gbl_ctx;
	nss_crypto_status_t status;

	nss_cfi_assert(ctx);

	/*
	 * When .setkey haven't been called and no session is created.
	 *	The refcnt should be 0 when this .exit is called.
	 */
	if (!atomic_read(&ctx->refcnt)) {
		nss_cfi_info("Key haven't been set and no session is created, just exit\n");
		goto clear_magic;
	}

	if (!atomic_dec_and_test(&ctx->refcnt)) {
		nss_cfi_err("Process done is not completed, while exit is called\n");
		nss_cfi_assert(false);
	}

	/*
	 * When NSS_CRYPTO_MAX_IDXS is set, it means that we didn't create any sessions
	 */
	if (ctx->sid == NSS_CRYPTO_MAX_IDXS)
		goto clear_magic;

	nss_cryptoapi_debugfs_del_session(ctx);

	status = nss_crypto_session_free(sc->crypto, ctx->sid);
	if (status != NSS_CRYPTO_STATUS_OK)
		nss_cfi_err("unable to free session: idx %d\n", ctx->sid);

clear_magic:
	nss_cryptoapi_clear_magic(ctx);
};

/*
 * nss_cryptoapi_ahash_setkey()
 * 	Cryptoapi setkey routine for hmac(sha1).
 */
int nss_cryptoapi_ahash_setkey(struct crypto_ahash *ahash, const u8 *key, unsigned int keylen)
{
	struct crypto_tfm *tfm = &ahash->base;
	struct nss_cryptoapi_ctx *ctx = crypto_tfm_ctx(&ahash->base);
	struct nss_cryptoapi *sc = &gbl_ctx;
	struct nss_crypto_key auth;
	struct nss_crypto_key cipher = { .algo = NSS_CRYPTO_CIPHER_AES_CBC };
// 	uint32_t flag = CRYPTO_TFM_RES_BAD_KEY_LEN;
	nss_crypto_status_t status;
	uint32_t algo_keylen;

	nss_cryptoapi_verify_magic(ctx);

	/*
	 * The ref count will be decremented in nss_cryptoapi_ahash_cra_exit().
	 */
	if (atomic_cmpxchg(&ctx->refcnt, 0, 1)) {
		nss_cfi_err("reusing context, setkey is already called\n");
		return -EINVAL;
	}

	/*
	 * check for the algorithm
	 */
	if (!strncmp("nss-hmac-sha1", crypto_tfm_alg_driver_name(tfm), CRYPTO_MAX_ALG_NAME)) {
		auth.algo = NSS_CRYPTO_AUTH_SHA1_HMAC;
		algo_keylen = NSS_CRYPTO_MAX_KEYLEN_SHA1;
	} else if (!strncmp("nss-hmac-sha256", crypto_tfm_alg_driver_name(tfm), CRYPTO_MAX_ALG_NAME)) {
		auth.algo = NSS_CRYPTO_AUTH_SHA256_HMAC;
		algo_keylen = NSS_CRYPTO_MAX_KEYLEN_SHA256;
	} else {
		nss_cfi_err("Unsupport AHASH algo(%s)\n", crypto_tfm_alg_driver_name(tfm));
		goto fail;
	}

	/*
	 * Check the provided key length against the
	 * algorithm's supported key length.
	 */
	if (keylen > algo_keylen) {
		nss_cfi_err("Bad Auth key_len(%d)\n", keylen);
		goto fail;
	}

	auth.key = (uint8_t *)key;
	auth.key_len = keylen;

	/*
	 * Crypto session is created with AES-CBC cipher.
	 * But during transformation, the cipher data length
	 * is set zero for pure auth.
	 */
	cipher.key = (uint8_t *)&dummy_key;
	cipher.key_len = NSS_CRYPTO_MAX_KEYLEN_AES;

	status = nss_crypto_session_alloc(sc->crypto, &cipher, &auth, &ctx->sid);
	if (status != NSS_CRYPTO_STATUS_OK) {
		nss_cfi_err("nss_crypto_session_alloc failed - status: %d\n", status);
		ctx->sid = NSS_CRYPTO_MAX_IDXS;
// 		flag = CRYPTO_TFM_RES_BAD_FLAGS;
		goto fail;
	}

	nss_cryptoapi_debugfs_add_session(sc, ctx);

	nss_cfi_info("session id created: %d\n", ctx->sid);

	ctx->cip_alg = NSS_CRYPTO_CIPHER_AES_CBC;
	ctx->auth_alg = auth.algo;

	return 0;

fail:
	/*
	 * Ref count should not be decremented here so that once a setkey
	 * fails for a context, a new setkey should occur in a different
	 * context while the old one gets freed.
	 */
// 	crypto_ahash_set_flags(ahash, flag);
	return -EINVAL;
}

/*
 * nss_cryptoapi_ahash_checkaddr()
 * 	Cryptoapi: obtain sg to virtual address mapping.
 * 	Check for multiple sg in src
 */
static int nss_cryptoapi_ahash_checkaddr(struct ahash_request *req)
{
	struct scatterlist *sg = req->src;
	struct scatterlist *sg_nxt = sg_next(sg);

	/*
	 * Fragmented memory is not supported
	 * return error, if caller sends multiple sg of fragmented
	 * memory space for src.
	 */
	while (sg_nxt) {
		if (page_address(sg_page(sg)) + sg->offset + sg_dma_len(sg)
				!= page_address(sg_page(sg_nxt)) + sg_nxt->offset) {
			nss_cfi_err("Only single sg supported: src invalid\n");
			return -EINVAL;
		}
		sg = sg_nxt;
		sg_nxt = sg_next(sg_nxt);
	}

	/*
	 * If the size of data is more than 65K reject transformation
	 */
	if (req->nbytes > NSS_CRYPTOAPI_MAX_DATA_LEN) {
		nss_cfi_err("Buffer length exceeded limit\n");
		return -EINVAL;
	}

	return 0;
}

/*
 * nss_cryptoapi_sha_hmac_done()
 *	Hash request completion callback function
 */
static void nss_cryptoapi_sha_hmac_done(struct nss_crypto_buf *buf)
{
	struct nss_cryptoapi_ctx *ctx;
	struct ahash_request *req;
	struct nss_cryptoapi_req_ctx *rctx;
	int err = 0;
	uint8_t *hw_hmac;

	nss_cfi_assert(buf);

	req = (struct ahash_request *)nss_crypto_get_cb_ctx(buf);

	ctx = crypto_tfm_ctx(req->base.tfm);
	nss_cryptoapi_verify_magic(ctx);

	rctx = ahash_request_ctx(req);

	hw_hmac = nss_crypto_get_hash_addr(buf);
	memcpy(rctx->digest, hw_hmac, rctx->diglen);

	if (req->result)
		memcpy(req->result, rctx->digest, rctx->diglen);

	nss_crypto_buf_free(gbl_ctx.crypto, buf);

	/*
	 * Passing always pass in case of encrypt.
	 *	Perhaps whenever core crypto invloke callback routine, it is always pass.
	 */
	req->base.complete(&req->base, err);

	nss_cfi_assert(atomic_read(&ctx->refcnt));
	atomic_dec(&ctx->refcnt);
	ctx->completed++;
}

/*
 * nss_cryptoapi_sha_hmac_transform()
 *	Crytoapi common routine for hash transform operations.
 */
struct nss_crypto_buf *nss_cryptoapi_sha_hmac_transform(struct ahash_request *req, struct nss_cryptoapi_ahash_info *info)
{
	struct nss_cryptoapi_ctx *ctx = crypto_tfm_ctx(req->base.tfm);
	struct nss_cryptoapi_req_ctx *rctx = ahash_request_ctx(req);
	struct nss_cryptoapi *sc = &gbl_ctx;
	struct nss_crypto_buf *buf;
	nss_crypto_status_t status;

	nss_cfi_assert(ctx);

	nss_cfi_dbg("src_vaddr: 0x%px, dst_vaddr: 0x%px\n",
			sg_virt(req->src), req->result);

	/*
	 * No cipher, all data is for auth. So skip is set to 0
	 */
	info->params->cipher_skip = 0;
	info->params->auth_skip = 0;

	if (nss_cryptoapi_ahash_checkaddr(req)) {
		nss_cfi_err("Invalid address!\n");
		return NULL;
	}

	/*
	 * Update the crypto session data
	 */
	status = nss_crypto_session_update(sc->crypto, ctx->sid, info->params);
	if (status != NSS_CRYPTO_STATUS_OK) {
		nss_cfi_err("Invalid crypto session parameters\n");
		return NULL;
	}

	/*
	 * Allocate crypto buf
	 */
	buf = nss_crypto_buf_alloc(sc->crypto);
	if (!buf) {
		nss_cfi_err("not able to allocate crypto buffer\n");
		return NULL;
	}

	/*
	 * set crypto buffer callback
	 */
	nss_crypto_set_cb(buf, info->cb_fn, req);
	nss_crypto_set_session_idx(buf, ctx->sid);

	memcpy(nss_crypto_get_ivaddr(buf), &dummy_iv, NSS_CRYPTO_MAX_IVLEN_AES);

	/*
	 * Fill Cipher and Auth len
	 */
	nss_crypto_set_data(buf, sg_virt(req->src), sg_virt(req->src),
		req->nbytes + rctx->diglen);
	nss_crypto_set_transform_len(buf, 0, req->nbytes);

	nss_cfi_dbg("cipher_len: %d, auth_len: %d, "
			"cipher_skip: %d, auth_skip: %d\n",
			buf->cipher_len, buf->auth_len,
			info->params->cipher_skip, info->params->auth_skip);
	nss_cfi_dbg("before transformation\n");
	nss_cfi_dbg_data(sg_virt(req->src), auth_len, ' ');

	return buf;
}

/*
 * nss_cryptoapi_ahash_init()
 *	Cryptoapi ahash request .init operation.
 *	Initialize the private context for the specific request.
 */
int nss_cryptoapi_ahash_init(struct ahash_request *req)
{
	struct crypto_ahash *ahash = crypto_ahash_reqtfm(req);
	struct crypto_tfm *tfm = crypto_ahash_tfm(ahash);
	struct nss_cryptoapi_ctx *ctx = crypto_tfm_ctx(tfm);
	struct nss_cryptoapi_req_ctx *rctx = ahash_request_ctx(req);

	nss_cryptoapi_verify_magic(ctx);

	if (ctx->sid >= NSS_CRYPTO_MAX_IDXS) {
		nss_cfi_err("Invalid session\n");
		return -EINVAL;
	}

	if (nss_crypto_get_auth(ctx->sid) != ctx->auth_alg) {
		nss_cfi_err("Invalid Auth Algo for session id: %d\n", ctx->sid);
		return -EINVAL;
	}

	/*
	 * Initialze the private context
	 */
	rctx->is_used = false;
	rctx->diglen = crypto_ahash_digestsize(ahash);
	memset(rctx->digest, 0, ARRAY_SIZE(rctx->digest));

	return 0;
};

/*
 * nss_cryptoapi_ahash_update()
 *	Cryptoapi ahash .update operation for a specific request.
 *	Accept user's data and post the data to NSS FW for transformation.
 *	Not return the hash result to user
 */
int nss_cryptoapi_ahash_update(struct ahash_request *req)
{
	struct nss_crypto_params params = { .req_type = NSS_CRYPTO_REQ_TYPE_AUTH };
	struct nss_cryptoapi_ahash_info info = {.cb_fn = nss_cryptoapi_sha_hmac_done,
						.params = &params};
	struct nss_cryptoapi_ctx *ctx = crypto_tfm_ctx(req->base.tfm);
	struct nss_cryptoapi_req_ctx *rctx = ahash_request_ctx(req);
	struct nss_cryptoapi *sc = &gbl_ctx;
	struct nss_crypto_buf *buf;

	nss_cryptoapi_verify_magic(ctx);

	if (ctx->sid >= NSS_CRYPTO_MAX_IDXS) {
		nss_cfi_err("Invalid session\n");
		return -EINVAL;
	}

	if (nss_crypto_get_auth(ctx->sid) != ctx->auth_alg) {
		nss_cfi_err("Invalid Auth Algo for session id: %d\n", ctx->sid);
		return -EINVAL;
	}

	/*
	 * .update can only be called one time for one request
	 */
	nss_cfi_assert(rctx->is_used == false);

	buf = nss_cryptoapi_sha_hmac_transform(req, &info);
	if (!buf) {
		nss_cfi_err("Invalid parameters\n");
		return -EINVAL;
	}

	/*
	 * Send the buffer to CORE layer for processing
	 */
	if (nss_crypto_transform_payload(sc->crypto, buf) != NSS_CRYPTO_STATUS_OK) {
		nss_cfi_info("Not enough resources with driver\n");
		nss_crypto_buf_free(sc->crypto, buf);
		ctx->queue_failed++;
		return -EINVAL;
	}

	rctx->is_used = true;
	ctx->queued++;
	atomic_inc(&ctx->refcnt);

	return -EINPROGRESS;
}

/*
 * nss_cryptoapi_ahash_final()
 *	Cryptoapi ahash .final operation for a specific request.
 *	Copy the hash result to user
 */
int nss_cryptoapi_ahash_final(struct ahash_request *req)
{
	struct crypto_ahash *ahash = crypto_ahash_reqtfm(req);
	struct crypto_tfm *tfm = crypto_ahash_tfm(ahash);
	struct nss_cryptoapi_ctx *ctx = crypto_tfm_ctx(tfm);
	struct nss_cryptoapi_req_ctx *rctx = ahash_request_ctx(req);

	nss_cryptoapi_verify_magic(ctx);

	if (ctx->sid >= NSS_CRYPTO_MAX_IDXS) {
		nss_cfi_err("Invalid session\n");
		return -EINVAL;
	}

	if (nss_crypto_get_auth(ctx->sid) != ctx->auth_alg) {
		nss_cfi_err("Invalid Auth Algo for session id: %d\n", ctx->sid);
		return -EINVAL;
	}

	nss_cfi_assert(req->result);
	memcpy(req->result, rctx->digest, crypto_ahash_digestsize(ahash));
	rctx->is_used = false;

	return 0;
}

/*
 * nss_cryptoapi_ahash_digest()
 *	Cryptoapi ahash request .digest operation.
 */
int nss_cryptoapi_ahash_digest(struct ahash_request *req)
{
	struct nss_crypto_params params = { .req_type = NSS_CRYPTO_REQ_TYPE_AUTH };
	struct nss_cryptoapi_ahash_info info = {.cb_fn = nss_cryptoapi_sha_hmac_done,
						.params = &params};
	struct nss_cryptoapi *sc = &gbl_ctx;
	struct nss_cryptoapi_ctx *ctx = crypto_tfm_ctx(req->base.tfm);
	struct nss_cryptoapi_req_ctx *rctx = ahash_request_ctx(req);
	struct nss_crypto_buf *buf;

	if (nss_cryptoapi_ahash_init(req) != NSS_CRYPTO_STATUS_OK)
		return -EINVAL;

	buf = nss_cryptoapi_sha_hmac_transform(req, &info);
	if (!buf) {
		nss_cfi_err("Invalid parameters\n");
		return -EINVAL;
	}

	if (nss_crypto_transform_payload(sc->crypto, buf) != NSS_CRYPTO_STATUS_OK) {
		nss_cfi_info("Not enough resources with driver\n");
		nss_crypto_buf_free(sc->crypto, buf);
		ctx->queue_failed++;
		return -EINVAL;
	}

	rctx->is_used = true;
	ctx->queued++;
	atomic_inc(&ctx->refcnt);

	return -EINPROGRESS;
}

/*
 * nss_cryptoapi_ahash_export()
 *	Cryptoapi ahash .export operation.
 *	Used to export data to other transform.
 *	Not support now, just ASSERT to notify users.
 */
int nss_cryptoapi_ahash_export(struct ahash_request *req, void *out)
{
	nss_cfi_assert(false);
	return 0;
};

/*
 * nss_cryptoapi_ahash_import()
 *	Cryptoapi ahash .import operation.
 *	Used to import data from other transform.
 *	Not support now, just ASSERT to notify users.
 */
int nss_cryptoapi_ahash_import(struct ahash_request *req, const void *in)
{
	nss_cfi_assert(false);
	return 0;
}
