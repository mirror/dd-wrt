/* Copyright (c) 2015-2019 The Linux Foundation. All rights reserved.
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
 * nss_cryptoapi_ablk.c
 * 	Interface to communicate Native Linux crypto framework specific data
 * 	to Crypto core specific data
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/random.h>
#include <linux/moduleparam.h>
#include <linux/spinlock.h>
#include <asm/cmpxchg.h>
#include <linux/delay.h>
#include <linux/crypto.h>
#include <linux/debugfs.h>
#include <linux/completion.h>

#include <crypto/aes.h>
#include <crypto/des.h>
#include <crypto/sha.h>
#include <crypto/hash.h>
#include <crypto/algapi.h>
#include <crypto/aead.h>
#include <crypto/authenc.h>
#include <crypto/ctr.h>
#include <crypto/internal/skcipher.h>

#include <nss_api_if.h>
#include <nss_crypto_cmn.h>
#include <nss_cfi_if.h>
#include <nss_crypto_api.h>
#include <nss_crypto_hdr.h>
#include <nss_crypto_defines.h>
#include <nss_cryptoapi.h>
#include "nss_cryptoapi_private.h"

extern struct nss_cryptoapi g_cryptoapi;

/*
 * nss_cryptoapi_skcipher_ctx2session()
 *	Cryptoapi function to get the session ID for an skcipher
 */
int nss_cryptoapi_skcipher_ctx2session(struct crypto_skcipher *sk, uint32_t *sid)
{
	struct crypto_tfm *tfm = crypto_skcipher_tfm(sk);
	struct crypto_ablkcipher **actx, *ablk;
	struct ablkcipher_tfm *ablk_tfm;
	struct nss_cryptoapi_ctx *ctx;

	if (strncmp("nss-", crypto_tfm_alg_driver_name(tfm), 4))
		return -EINVAL;

	/* Get the ablkcipher from the skcipher */
	actx = crypto_skcipher_ctx(sk);
	if (!actx || !(*actx))
		return -EINVAL;

	/*
	 * The ablkcipher now obtained is a wrapper around the actual
	 * ablkcipher that is created when the skcipher is created.
	 * Hence we derive the required ablkcipher through ablkcipher_tfm.
	 */
	ablk_tfm = crypto_ablkcipher_crt(*actx);
	if (!ablk_tfm)
		return -EINVAL;

	ablk = ablk_tfm->base;
	if (!ablk)
		return -EINVAL;

	/* Get the nss_cryptoapi context stored in the ablkcipher */
	ctx = crypto_ablkcipher_ctx(ablk);

	BUG_ON(!ctx);
	NSS_CRYPTOAPI_VERIFY_MAGIC(ctx);

	*sid = ctx->sid;
	return 0;
}
EXPORT_SYMBOL(nss_cryptoapi_skcipher_ctx2session);

/*
 * nss_cryptoapi_ablkcipher_init()
 * 	Cryptoapi ablkcipher init function.
 */
int nss_cryptoapi_ablkcipher_init(struct crypto_tfm *tfm)
{
	struct nss_cryptoapi_ctx *ctx = crypto_tfm_ctx(tfm);
	struct crypto_ablkcipher *sw_tfm;
	bool need_fallback;

	BUG_ON(!ctx);
	NSS_CRYPTOAPI_SET_MAGIC(ctx);

	memset(ctx, 0, sizeof(struct nss_cryptoapi_ctx));

	ctx->user = g_cryptoapi.user;
	ctx->stats.init++;
	init_completion(&ctx->complete);

	need_fallback = crypto_tfm_alg_flags(tfm) & CRYPTO_ALG_NEED_FALLBACK;
	if (!need_fallback)
		return 0;

	/*
	 * Alloc fallback transform for future use
	 */
	sw_tfm = crypto_alloc_ablkcipher(crypto_tfm_alg_name(tfm), 0, CRYPTO_ALG_ASYNC | CRYPTO_ALG_NEED_FALLBACK);
	if (IS_ERR(sw_tfm)) {
		nss_cfi_err("%p: unable to allocate SW_TFM(%s)\n", ctx, crypto_tfm_alg_name(tfm));
		return -ENOMEM;
	}

	/*
	 * set this tfm reqsize same to fallback tfm
	 */
	tfm->crt_ablkcipher.reqsize = crypto_ablkcipher_reqsize(sw_tfm);
	ctx->sw_tfm = crypto_ablkcipher_tfm(sw_tfm);

	return 0;
}

/*
 * nss_cryptoapi_ablkcipher_exit()
 * 	Cryptoapi ablkcipher exit function.
 */
void nss_cryptoapi_ablkcipher_exit(struct crypto_tfm *tfm)
{
	struct nss_cryptoapi_ctx *ctx = crypto_tfm_ctx(tfm);
	int ret;

	BUG_ON(!ctx);
	NSS_CRYPTOAPI_VERIFY_MAGIC(ctx);

	if (ctx->sw_tfm) {
		crypto_free_ablkcipher(__crypto_ablkcipher_cast(ctx->sw_tfm));
		ctx->sw_tfm = NULL;
	}

	ctx->stats.exit++;

	/*
	 * When fallback_req is set, it means that fallback tfm was used
	 * we didn't create any sessions.
	 */
	if (ctx->fallback_req) {
		ctx->stats.failed_fallback++;
		return;
	}

	if (!atomic_read(&ctx->active)) {
		ctx->stats.failed_exit++;
		return;
	}

	/*
	 * Mark cryptoapi context as inactive
	 */
	atomic_set(&ctx->active, 0);

	nss_crypto_session_free(ctx->user, ctx->sid);

	if (!atomic_sub_and_test(1, &ctx->refcnt)) {
		/*
		 * We need to wait for any outstanding packet using this ctx.
		 * Once the last packet get processed, reference count will become
		 * 0 this ctx. We will wait for the reference to go down to 0.
		 */
		ret = wait_for_completion_timeout(&ctx->complete, NSS_CRYPTOAPI_REQ_TIMEOUT_TICKS);
		WARN_ON(!ret);
	}

	ctx->sid = NSS_CRYPTO_SESSION_MAX;

	debugfs_remove_recursive(ctx->dentry);
	NSS_CRYPTOAPI_CLEAR_MAGIC(ctx);
}

/*
 * nss_cryptoapi_ablk_setkey()
 * 	Cryptoapi setkey routine for aes.
 */
int nss_cryptoapi_ablk_setkey(struct crypto_ablkcipher *cipher, const u8 *key, unsigned int keylen)
{
	struct crypto_tfm *tfm = crypto_ablkcipher_tfm(cipher);
	struct nss_cryptoapi_ctx *ctx = crypto_tfm_ctx(tfm);
	struct nss_crypto_session_data data = {0};
	int status;

	/*
	 * Validate magic number - init should be called before setkey
	 */
	NSS_CRYPTOAPI_VERIFY_MAGIC(ctx);

	ctx->info = nss_cryptoapi_cra_name2info(crypto_tfm_alg_name(tfm), keylen, 0);
	if (!ctx->info) {
		crypto_ablkcipher_set_flags(cipher, CRYPTO_TFM_RES_BAD_KEY_LEN);
		return -EINVAL;
	}

	ctx->iv_size = crypto_ablkcipher_ivsize(cipher);

	if (ctx->info->cipher_mode == NSS_CRYPTOAPI_CIPHER_MODE_CTR_RFC3686) {
		keylen = keylen - CTR_RFC3686_NONCE_SIZE;
		memcpy(ctx->ctx_iv, key + keylen, CTR_RFC3686_NONCE_SIZE);
		ctx->ctx_iv[3] = ntohl(0x1);
		ctx->iv_size += CTR_RFC3686_NONCE_SIZE + sizeof(uint32_t);
	}

	/*
	 * Fallback to software if an algorithm in not enabled in hardware.
	 */
	if (ctx->sw_tfm) {
		ctx->fallback_req = true;

		/*
		 * set flag to fallback tfm
		 */
		crypto_tfm_clear_flags(ctx->sw_tfm, CRYPTO_TFM_REQ_MASK);
		crypto_tfm_set_flags(ctx->sw_tfm, crypto_ablkcipher_get_flags(cipher) & CRYPTO_TFM_REQ_MASK);

		/*
		 * Set the key for fallback tfm
		 */
		status = crypto_ablkcipher_setkey(__crypto_ablkcipher_cast(ctx->sw_tfm), key, keylen);
		if (status) {
			nss_cfi_err("Failed to set key to the sw crypto");
			crypto_ablkcipher_set_flags(cipher, crypto_tfm_get_flags(ctx->sw_tfm));
		}

		return status;
	}

	/*
	 * Fill NSS crypto session data
	 */
	data.algo = ctx->info->algo;
	data.cipher_key = key;

	if (data.algo >= NSS_CRYPTO_CMN_ALGO_MAX)
		return -ERANGE;

	status = nss_crypto_session_alloc(ctx->user, &data, &ctx->sid);
	if (status < 0) {
		nss_cfi_err("%p: Unable to allocate crypto session(%d)\n", ctx, status);
		crypto_ablkcipher_set_flags(cipher, CRYPTO_TFM_RES_BAD_FLAGS);
		return status;
	}

	nss_cryptoapi_add_ctx2debugfs(ctx);
	atomic_set(&ctx->active, 1);
	atomic_set(&ctx->refcnt, 1);
	return 0;
}

/*
 * nss_cryptoapi_ablkcipher_done()
 * 	Cipher operation completion callback function
 */
void nss_cryptoapi_ablkcipher_done(void *app_data, struct nss_crypto_hdr *ch, uint8_t status)
{
	struct ablkcipher_request *req = app_data;
	struct nss_cryptoapi_ctx *ctx = crypto_tfm_ctx(req->base.tfm);
	int error;

	BUG_ON(!ch);

	/*
	 * Check cryptoapi context magic number.
	 */
	NSS_CRYPTOAPI_VERIFY_MAGIC(ctx);

	/*
	 * Free crypto hdr
	 */
	nss_crypto_hdr_free(ctx->user, ch);

	nss_cfi_dbg("data dump after transformation\n");
	nss_cfi_dbg_data(sg_virt(req->dst), req->nbytes, ' ');

	/*
	 * Check if there is any error reported by hardware
	 */
	error = nss_cryptoapi_status2error(ctx, status);
	ctx->stats.completed++;

	/*
	 * Decrement cryptoapi reference
	 */
	nss_cryptoapi_ref_dec(ctx);
	req->base.complete(&req->base, error);
}

/*
 * nss_cryptoapi_ablk_encrypt()
 * 	Crytoapi encrypt for AES and 3DES algorithms.
 */
int nss_cryptoapi_ablk_encrypt(struct ablkcipher_request *req)
{
	struct nss_cryptoapi_info info = {.op_dir = NSS_CRYPTO_OP_DIR_ENC};
	struct crypto_ablkcipher *cipher = crypto_ablkcipher_reqtfm(req);
	struct nss_cryptoapi_ctx *ctx = crypto_ablkcipher_ctx(cipher);
	struct crypto_tfm *tfm = req->base.tfm;
	struct scatterlist *cur;
	int tot_len = 0;
	int error;
	int i;

	/*
	 * Check cryptoapi context magic number.
	 */
	NSS_CRYPTOAPI_VERIFY_MAGIC(ctx);

	if (ctx->fallback_req) {
		struct crypto_ablkcipher *orig_tfm = crypto_ablkcipher_reqtfm(req);

		/*
		 * Set new fallback tfm to the request
		 */
		BUG_ON(!ctx->sw_tfm);
		ablkcipher_request_set_tfm(req, __crypto_ablkcipher_cast(ctx->sw_tfm));
		ctx->stats.queued++;

		error = crypto_ablkcipher_encrypt(req);
		if (!error)
			ctx->stats.completed++;

		/*
		 * Set original tfm to the request
		 */
		ablkcipher_request_set_tfm(req, orig_tfm);
		return error;
	}

	/*
	 * Check if cryptoapi context is active or not
	 */
	if (!atomic_read(&ctx->active))
		return -EINVAL;

	if (req->src != req->dst) {
		ctx->stats.failed_req++;
		return -EINVAL;
	}

	/*
	 * Block size not aligned.
	 * AES-CTR requires only a one-byte block size alignment.
	 */
	if (!IS_ALIGNED(req->nbytes, crypto_tfm_alg_blocksize(tfm)) &&
			ctx->info->cipher_mode != NSS_CRYPTOAPI_CIPHER_MODE_CTR_RFC3686) {
		ctx->stats.failed_align++;
		crypto_ablkcipher_set_flags(cipher, CRYPTO_TFM_RES_BAD_BLOCK_LEN);
		return -EFAULT;
	}

	/*
	 * Fill the request information structure
	 */
	info.iv = req->info;
	info.nsegs = sg_nents(req->src);
	info.op_dir = NSS_CRYPTO_OP_DIR_ENC;
	info.cb = nss_cryptoapi_ablkcipher_done;
	info.iv_size = ctx->iv_size;
	info.first_sg = req->src;

	/*
	 * The exact length of data that needs to be ciphered for an ABLK
	 * request is stored in req->nbytes. Hence we may have to reduce
	 * the DMA length to what is specified in req->nbytes and later
	 * restore the length of scatterlist back to its original value.
	 */
	for_each_sg(req->src, cur, info.nsegs, i) {
		if (!cur)
			break;

		tot_len += cur->length;
		if (!sg_next(cur))
			break;
	}

	info.last_sg = cur;
	info.ahash_skip = tot_len - req->nbytes;

	if (!atomic_inc_not_zero(&ctx->refcnt))
		return -ENOENT;

	return nss_cryptoapi_transform(ctx, &info, (void *)req, false);
}

/*
 * nss_cryptoapi_ablk_decrypt()
 * 	Crytoapi decrypt for AES and 3DES CBC algorithms.
 */
int nss_cryptoapi_ablk_decrypt(struct ablkcipher_request *req)
{
	struct nss_cryptoapi_info info = {.op_dir = NSS_CRYPTO_OP_DIR_DEC};
	struct crypto_ablkcipher *cipher = crypto_ablkcipher_reqtfm(req);
	struct nss_cryptoapi_ctx *ctx = crypto_ablkcipher_ctx(cipher);
	struct crypto_tfm *tfm = req->base.tfm;
	struct scatterlist *cur;
	int tot_len = 0;
	int error;
	int i;

	/*
	 * Check cryptoapi context magic number.
	 */
	NSS_CRYPTOAPI_VERIFY_MAGIC(ctx);

	if (ctx->fallback_req) {
		struct crypto_ablkcipher *orig_tfm = crypto_ablkcipher_reqtfm(req);


		/*
		 * Set new fallback tfm to the request
		 */
		BUG_ON(!ctx->sw_tfm);
		ablkcipher_request_set_tfm(req, __crypto_ablkcipher_cast(ctx->sw_tfm));
		ctx->stats.queued++;

		error = crypto_ablkcipher_decrypt(req);
		if (!error)
			ctx->stats.completed++;

		/*
		 * Set original tfm to the request
		 */
		ablkcipher_request_set_tfm(req, orig_tfm);
		return error;
	}

	/*
	 * Check if cryptoapi context is active or not
	 */
	if (!atomic_read(&ctx->active))
		return -EINVAL;

	if (req->src != req->dst) {
		ctx->stats.failed_req++;
		return -EINVAL;
	}

	/*
	 * Block size not aligned
	 */
	if (!IS_ALIGNED(req->nbytes, crypto_tfm_alg_blocksize(tfm))) {
		ctx->stats.failed_align++;
		crypto_ablkcipher_set_flags(cipher, CRYPTO_TFM_RES_BAD_BLOCK_LEN);
		return -EFAULT;
	}

	/*
	 * Fill the request information structure
	 * Note: For CTR mode, IV size will be set to AES_BLOCK_SIZE.
	 * This is because linux gives iv size as 8 while we need to alloc 16 bytes
	 * in crypto hdr to accomodate
	 * - 4 bytes of nonce
	 * - 8 bytes of IV
	 * - 4 bytes of initial counter
	 */
	info.iv = req->info;
	info.nsegs = sg_nents(req->src);
	info.iv_size = ctx->iv_size;
	info.op_dir = NSS_CRYPTO_OP_DIR_DEC;
	info.cb = nss_cryptoapi_ablkcipher_done;
	info.first_sg = req->src;

	/*
	 * The exact length of data that needs to be ciphered for an ABLK
	 * request is stored in req->nbytes. Hence we may have to reduce
	 * the DMA length to what is specified in req->nbytes and later
	 * restore the length of scatterlist back to its original value.
	 */
	for_each_sg(req->src, cur, info.nsegs, i) {
		tot_len += cur->length;
		if (!sg_next(cur))
			break;
	}

	info.ahash_skip = tot_len - req->nbytes;
	info.last_sg = cur;

	if (!atomic_inc_not_zero(&ctx->refcnt))
		return -ENOENT;

	return nss_cryptoapi_transform(ctx, &info, (void *)req, false);
}
