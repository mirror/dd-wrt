/* Copyright (c) 2015-2020 The Linux Foundation. All rights reserved.
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
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0)
#include <crypto/sha.h>
#else
#include <crypto/sha1.h>
#include <crypto/sha2.h>
#endif
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
	struct nss_cryptoapi_ctx *ctx;

	if (strncmp("nss-", crypto_tfm_alg_driver_name(tfm), 4))
		return -EINVAL;

	/* Get the nss_cryptoapi context stored in skcipher */
	ctx = crypto_skcipher_ctx(sk);
	BUG_ON(!ctx);
	NSS_CRYPTOAPI_VERIFY_MAGIC(ctx);

	*sid = ctx->sid;
	return 0;
}
EXPORT_SYMBOL(nss_cryptoapi_skcipher_ctx2session);

/*
 * nss_cryptoapi_skcipher_init()
 * 	Cryptoapi skcipher init function.
 */
int nss_cryptoapi_skcipher_init(struct crypto_skcipher *tfm)
{
	struct nss_cryptoapi_ctx *ctx = crypto_skcipher_ctx(tfm);

	BUG_ON(!ctx);

	memset(ctx, 0, sizeof(struct nss_cryptoapi_ctx));

	NSS_CRYPTOAPI_SET_MAGIC(ctx);
	ctx->user = g_cryptoapi.user;
	ctx->stats.init++;
	ctx->sid = NSS_CRYPTO_SESSION_MAX;
	init_completion(&ctx->complete);

	return 0;
}

/*
 * nss_cryptoapi_skcipher_exit()
 * 	Cryptoapi skcipher exit function.
 */
void nss_cryptoapi_skcipher_exit(struct crypto_skcipher *tfm)
{
	struct nss_cryptoapi_ctx *ctx = crypto_skcipher_ctx(tfm);
	int ret;

	BUG_ON(!ctx);
	NSS_CRYPTOAPI_VERIFY_MAGIC(ctx);

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

	if (!atomic_sub_and_test(1, &ctx->refcnt)) {
		/*
		 * We need to wait for any outstanding packet using this ctx.
		 * Once the last packet get processed, reference count will become
		 * 0 this ctx. We will wait for the reference to go down to 0.
		 */
		ret = wait_for_completion_timeout(&ctx->complete, NSS_CRYPTOAPI_REQ_TIMEOUT_TICKS);
		WARN_ON(!ret);
	}

	if (ctx->sid != NSS_CRYPTO_SESSION_MAX) {
		nss_crypto_session_free(ctx->user, ctx->sid);
		debugfs_remove_recursive(ctx->dentry);
		ctx->sid = NSS_CRYPTO_SESSION_MAX;
	}

	NSS_CRYPTOAPI_CLEAR_MAGIC(ctx);
}

/*
 * nss_cryptoapi_skcipher_setkey()
 * 	Cryptoapi setkey routine for aes.
 */
int nss_cryptoapi_skcipher_setkey(struct crypto_skcipher *cipher, const u8 *key, unsigned int keylen)
{
	struct crypto_tfm *tfm = crypto_skcipher_tfm(cipher);
	struct nss_cryptoapi_ctx *ctx = crypto_skcipher_ctx(cipher);
	struct nss_crypto_session_data data = {0};
	int status;

	/*
	 * Validate magic number - init should be called before setkey
	 */
	NSS_CRYPTOAPI_VERIFY_MAGIC(ctx);

	ctx->info = nss_cryptoapi_cra_name2info(crypto_tfm_alg_name(tfm), keylen, 0);
	if (!ctx->info) {
		return -EINVAL;
	}

	ctx->iv_size = crypto_skcipher_ivsize(cipher);

	if (ctx->info->cipher_mode == NSS_CRYPTOAPI_CIPHER_MODE_CTR_RFC3686) {
		keylen = keylen - CTR_RFC3686_NONCE_SIZE;
		memcpy(ctx->ctx_iv, key + keylen, CTR_RFC3686_NONCE_SIZE);
		ctx->ctx_iv[3] = ntohl(0x1);
		ctx->iv_size += CTR_RFC3686_NONCE_SIZE + sizeof(uint32_t);
	}

	/*
	 * Fill NSS crypto session data
	 */
	data.algo = ctx->info->algo;
	data.cipher_key = key;

	if (data.algo >= NSS_CRYPTO_CMN_ALGO_MAX)
		return -ERANGE;

	if (ctx->sid != NSS_CRYPTO_SESSION_MAX) {
		nss_crypto_session_free(ctx->user, ctx->sid);
		debugfs_remove_recursive(ctx->dentry);
		ctx->sid = NSS_CRYPTO_SESSION_MAX;
	}

	status = nss_crypto_session_alloc(ctx->user, &data, &ctx->sid);
	if (status < 0) {
		nss_cfi_err("%px: Unable to allocate crypto session(%d)\n", ctx, status);
		return status;
	}

	nss_cryptoapi_add_ctx2debugfs(ctx);
	atomic_set(&ctx->active, 1);
	atomic_set(&ctx->refcnt, 1);
	return 0;
}

/*
 * nss_cryptoapi_skcipher_done()
 * 	Cipher operation completion callback function
 */
void nss_cryptoapi_skcipher_done(void *app_data, struct nss_crypto_hdr *ch, uint8_t status)
{
	struct skcipher_request *req = app_data;
	struct crypto_skcipher *cipher = crypto_skcipher_reqtfm(req);
	struct nss_cryptoapi_ctx *ctx = crypto_skcipher_ctx(cipher);
	int error;

	BUG_ON(!ch);
	/*
	 * Check cryptoapi context magic number.
	 */
	NSS_CRYPTOAPI_VERIFY_MAGIC(ctx);

	/*
	 * For skcipher decryption case, the last block of encrypted data is used as
	 * an IV for the next data
	 */
	if (ch->op == NSS_CRYPTO_OP_DIR_ENC) {
		nss_cryptoapi_copy_iv(ctx, req->dst, req->iv, ch->iv_len);
	}

	/*
	 * Free crypto hdr
	 */
	nss_crypto_hdr_free(ctx->user, ch);

	nss_cfi_dbg("data dump after transformation\n");
	nss_cfi_dbg_data(sg_virt(req->dst), req->cryptlen, ' ');

	/*
	 * Check if there is any error reported by hardware
	 */
	error = nss_cryptoapi_status2error(ctx, status);
	ctx->stats.completed++;

	/*
	 * Decrement cryptoapi reference
	 */
	nss_cryptoapi_ref_dec(ctx);
	skcipher_request_complete(req, error);
}

/*
 * nss_cryptoapi_skcipher_encrypt()
 * 	Crytoapi encrypt for AES and 3DES algorithms.
 */
int nss_cryptoapi_skcipher_encrypt(struct skcipher_request *req)
{
	struct nss_cryptoapi_info info = {.op_dir = NSS_CRYPTO_OP_DIR_ENC};
	struct crypto_skcipher *cipher = crypto_skcipher_reqtfm(req);
	struct nss_cryptoapi_ctx *ctx = crypto_skcipher_ctx(cipher);
	struct crypto_tfm *tfm = req->base.tfm;
	struct scatterlist *cur;
	int tot_len = 0;
	int i;

	/*
	 * Check cryptoapi context magic number.
	 */
	NSS_CRYPTOAPI_VERIFY_MAGIC(ctx);

	/*
	 * Check if cryptoapi context is active or not
	 */
	if (!atomic_read(&ctx->active))
		return -EINVAL;

	if (sg_nents(req->src) != sg_nents(req->dst)) {
		ctx->stats.failed_req++;
		return -EINVAL;
	}

	/*
	 * Block size not aligned.
	 * AES-CTR requires only a one-byte block size alignment.
	 */
	if (!IS_ALIGNED(req->cryptlen, crypto_tfm_alg_blocksize(tfm)) && ctx->info->blk_align) {
		ctx->stats.failed_align++;
		return -EFAULT;
	}

	/*
	 * Fill the request information structure
	 */
	info.iv = req->iv;
	info.src.nsegs = sg_nents(req->src);
	info.dst.nsegs = sg_nents(req->dst);
	info.op_dir = NSS_CRYPTO_OP_DIR_ENC;
	info.cb = nss_cryptoapi_skcipher_done;
	info.iv_size = ctx->iv_size;
	info.src.first_sg = req->src;
	info.dst.first_sg = req->dst;
	info.dst.last_sg = sg_last(req->dst, info.dst.nsegs);

	/* out and in length will be same as ablk does only encrypt/decryt operation */
	info.total_in_len = info.total_out_len = req->cryptlen;
	info.in_place = (req->src == req->dst) ? true : false;

	/*
	 * The exact length of data that needs to be ciphered for an ABLK
	 * request is stored in req->cryptlen. Hence we may have to reduce
	 * the DMA length to what is specified in req->cryptlen and later
	 * restore the length of scatterlist back to its original value.
	 */
	for_each_sg(req->src, cur, info.src.nsegs, i) {
		if (!cur)
			break;

		tot_len += cur->length;
		if (!sg_next(cur))
			break;
	}

	/*
	 * We only support (2^16 - 1) length.
	 */
	if (tot_len > U16_MAX) {
		ctx->stats.failed_len++;
		return -EFBIG;
	}

	info.src.last_sg = cur;
	info.ahash_skip = tot_len - req->cryptlen;

	if (!atomic_inc_not_zero(&ctx->refcnt))
		return -ENOENT;

	return nss_cryptoapi_transform(ctx, &info, (void *)req, false);
}

/*
 * nss_cryptoapi_skcipher_decrypt()
 * 	Crytoapi decrypt for AES and 3DES CBC algorithms.
 */
int nss_cryptoapi_skcipher_decrypt(struct skcipher_request *req)
{
	struct nss_cryptoapi_info info = {.op_dir = NSS_CRYPTO_OP_DIR_DEC};
	struct crypto_skcipher *cipher = crypto_skcipher_reqtfm(req);
	struct nss_cryptoapi_ctx *ctx = crypto_skcipher_ctx(cipher);
	struct crypto_tfm *tfm = req->base.tfm;
	struct scatterlist *cur;
	int tot_len = 0;
	int i;

	/*
	 * Check cryptoapi context magic number.
	 */
	NSS_CRYPTOAPI_VERIFY_MAGIC(ctx);

	/*
	 * Check if cryptoapi context is active or not
	 */
	if (!atomic_read(&ctx->active))
		return -EINVAL;

	if (sg_nents(req->src) != sg_nents(req->dst)) {
		ctx->stats.failed_req++;
		return -EINVAL;
	}

	/*
	 * Block size not aligned
	 */
	if (!IS_ALIGNED(req->cryptlen, crypto_tfm_alg_blocksize(tfm)) && ctx->info->blk_align) {
		ctx->stats.failed_align++;
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
	info.iv = req->iv;
	info.src.nsegs = sg_nents(req->src);
	info.dst.nsegs = sg_nents(req->dst);
	info.iv_size = ctx->iv_size;
	info.op_dir = NSS_CRYPTO_OP_DIR_DEC;
	info.cb = nss_cryptoapi_skcipher_done;
	info.src.first_sg = req->src;
	info.dst.first_sg = req->dst;
	info.dst.last_sg = sg_last(req->dst, info.dst.nsegs);

	/* out and in length will be same as ablk does only encrypt/decryt operation */
	info.total_in_len = info.total_out_len = req->cryptlen;
	info.in_place = (req->src == req->dst) ? true : false;

	/*
	 * The exact length of data that needs to be ciphered for an ABLK
	 * request is stored in req->cryptlen. Hence we may have to reduce
	 * the DMA length to what is specified in req->cryptlen and later
	 * restore the length of scatterlist back to its original value.
	 */
	for_each_sg(req->src, cur, info.src.nsegs, i) {
		tot_len += cur->length;
		if (!sg_next(cur))
			break;
	}

	/*
	 * We only support (2^16 - 1) length.
	 */
	if (tot_len > U16_MAX) {
		ctx->stats.failed_len++;
		return -EFBIG;
	}

	info.ahash_skip = tot_len - req->cryptlen;
	info.src.last_sg = cur;

	if (!atomic_inc_not_zero(&ctx->refcnt))
		return -ENOENT;

	return nss_cryptoapi_transform(ctx, &info, (void *)req, false);
}
