/* Copyright (c) 2017-2020 The Linux Foundation. All rights reserved.
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
#include <crypto/scatterwalk.h>
#include <crypto/internal/hash.h>
#include <crypto/internal/skcipher.h>

#include <nss_api_if.h>
#include <nss_crypto_cmn.h>
#include <nss_cfi_if.h>
#include <nss_crypto_api.h>
#include <nss_crypto_hdr.h>
#include <nss_crypto_defines.h>
#include "nss_cryptoapi_private.h"

extern struct nss_cryptoapi g_cryptoapi;

#if !defined NSS_CFI_DEBUG

#define NSS_CRYPTOAPI_AHASH_VERIFY_MAGIC(rctx)
#define NSS_CRYPTOAPI_AHASH_SET_MAGIC(rctx)
#define NSS_CRYPTOAPI_AHASH_CLEAR_MAGIC(rctx)

#else

#define NSS_CRYPTOAPI_AHASH_VERIFY_MAGIC(rctx)  do {		\
	struct nss_cryptoapi_req_ctx *__req_ctx = (rctx);	\
	BUG_ON(__req_ctx->magic != NSS_CRYPTOAPI_AHASH_MAGIC);	\
} while(0)

#define NSS_CRYPTOAPI_AHASH_SET_MAGIC(rctx) do {		\
	struct nss_cryptoapi_req_ctx *__req_ctx = (rctx);	\
	__req_ctx->magic = NSS_CRYPTOAPI_AHASH_MAGIC;		\
} while(0)

#define NSS_CRYPTOAPI_AHASH_CLEAR_MAGIC(rctx) do {		\
	struct nss_cryptoapi_req_ctx *__req_ctx = (rctx);	\
	__req_ctx->magic = 0;					\
} while(0)

#endif /* NSS_CFI_DEBUG */

/*
 * nss_cryptoapi_ahash_ctx2session()
 *	Cryptoapi function to get the session ID for an AHASH
 */
int nss_cryptoapi_ahash_ctx2session(struct crypto_ahash *ahash, uint32_t *sid)
{
	struct crypto_tfm *tfm = crypto_ahash_tfm(ahash);
	struct nss_cryptoapi_ctx *ctx = crypto_ahash_ctx(ahash);

	if (strncmp("nss-", crypto_tfm_alg_driver_name(tfm), 4))
		return -EINVAL;

	*sid = ctx->sid;

	return 0;
}
EXPORT_SYMBOL(nss_cryptoapi_ahash_ctx2session);

/*
 * nss_cryptoapi_ahash_cra_init()
 *	Cryptoapi ahash init function.
 */
int nss_cryptoapi_ahash_cra_init(struct crypto_tfm *tfm)
{
	struct nss_cryptoapi_ctx *ctx = crypto_tfm_ctx(tfm);
	struct crypto_ahash *ahash = __crypto_ahash_cast(tfm);

	BUG_ON(!ctx);

	memset(ctx, 0, sizeof(struct nss_cryptoapi_ctx));
	NSS_CRYPTOAPI_SET_MAGIC(ctx);

	ctx->user = g_cryptoapi.user;
	ctx->sid = NSS_CRYPTO_SESSION_MAX;
	ctx->stats.init++;
	init_completion(&ctx->complete);

	/*
	 * set this tfm reqsize to the transform specific structure
	 */
	crypto_ahash_set_reqsize(ahash, sizeof(struct nss_cryptoapi_req_ctx));

	return 0;
};

/*
 * nss_cryptoapi_ahash_cra_exit()
 *	Cryptoapi ahash exit function.
 */
void nss_cryptoapi_ahash_cra_exit(struct crypto_tfm *tfm)
{
	struct nss_cryptoapi_ctx *ctx = crypto_tfm_ctx(tfm);
	int ret;

	BUG_ON(!ctx);
	NSS_CRYPTOAPI_VERIFY_MAGIC(ctx);
	ctx->stats.exit++;

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
};

/*
 * nss_cryptoapi_ahash_setkey()
 *	Cryptoapi setkey routine for hmac(sha1).
 */
int nss_cryptoapi_ahash_setkey(struct crypto_ahash *ahash, const u8 *key, unsigned int keylen)
{
	struct crypto_tfm *tfm = crypto_ahash_tfm(ahash);
	struct nss_cryptoapi_ctx *ctx = crypto_tfm_ctx(&ahash->base);
	struct nss_crypto_session_data data = {0};
	int status;

	/*
	 * Validate magic number - init should be called before setkey
	 */
	NSS_CRYPTOAPI_VERIFY_MAGIC(ctx);

	ctx->info = nss_cryptoapi_cra_name2info(crypto_tfm_alg_name(tfm), 0, crypto_ahash_digestsize(ahash));
	if (!ctx->info) {
		return -EINVAL;
	}

	/*
	 * Fill NSS crypto session data
	 */
	data.algo = ctx->info->algo;
	data.auth_key = key;
	data.auth_keylen = keylen;

	if (data.algo >= NSS_CRYPTO_CMN_ALGO_MAX)
		return -ERANGE;

	if (ctx->sid != NSS_CRYPTO_SESSION_MAX) {
		nss_crypto_session_free(ctx->user, ctx->sid);
		debugfs_remove_recursive(ctx->dentry);
		ctx->sid = NSS_CRYPTO_SESSION_MAX;
	}

	status = nss_crypto_session_alloc(ctx->user, &data, &ctx->sid);
	if (status < 0) {
		nss_cfi_warn("%px: Unable to allocate crypto session(%d)\n", ctx, status);
		return status;
	}

	nss_cryptoapi_add_ctx2debugfs(ctx);
	atomic_set(&ctx->active, 1);
	atomic_set(&ctx->refcnt, 1);

	return 0;
}

/*
 * nss_cryptoapi_ahash_done()
 *	Hash request completion callback function
 */
void nss_cryptoapi_ahash_done(void *app_data, struct nss_crypto_hdr *ch, uint8_t status)
{
	struct ahash_request *req = app_data;
	struct crypto_ahash *ahash = crypto_ahash_reqtfm(req);
	struct nss_cryptoapi_ctx *ctx = crypto_ahash_ctx(ahash);
	struct nss_cryptoapi_req_ctx *rctx = ahash_request_ctx(req);

	uint8_t *hw_hmac;
	int error;

	BUG_ON(!ch);

	/*
	 * Check cryptoapi context magic number.
	 */
	NSS_CRYPTOAPI_VERIFY_MAGIC(ctx);

	rctx = ahash_request_ctx(req);
	hw_hmac = nss_crypto_hdr_get_hmac(ch);
	memcpy(rctx->digest, hw_hmac, rctx->diglen);

	if (req->result)
		memcpy(req->result, rctx->digest, rctx->diglen);

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
	ahash_request_complete(req, error);
}

/*
 * nss_cryptoapi_ahash_init()
 *	Cryptoapi ahash request .init operation.
 *	Initialize the private context for the specific request.
 */
int nss_cryptoapi_ahash_init(struct ahash_request *req)
{
	struct nss_cryptoapi_ctx *ctx __attribute__((unused)) = crypto_tfm_ctx(req->base.tfm);
	struct nss_cryptoapi_req_ctx *rctx = ahash_request_ctx(req);
	struct crypto_ahash *ahash = crypto_ahash_reqtfm(req);
	struct crypto_tfm *tfm = crypto_ahash_tfm(ahash);
	struct nss_crypto_session_data data = {0};
	int status;

	/*
	 * Check cryptoapi context magic number.
	 */
	NSS_CRYPTOAPI_VERIFY_MAGIC(ctx);

	if (ctx->sid == NSS_CRYPTO_SESSION_MAX) {
		/*
		 * Set the key length to zero as the need is
		 * to search for a non-keyed hash algo.
		 */
		ctx->info = nss_cryptoapi_cra_name2info(crypto_tfm_alg_name(tfm), 0, 0);
		if (!ctx->info) {
			return -EINVAL;
		}

		/*
		 * Fill NSS crypto session data
		 */
		data.algo = ctx->info->algo;

		if (data.algo >= NSS_CRYPTO_CMN_ALGO_MAX)
			return -ERANGE;

		status = nss_crypto_session_alloc(ctx->user, &data, &ctx->sid);
		if (status < 0) {
			nss_cfi_err("%px: Unable to allocate crypto session(%d)\n", ctx, status);
			return status;
		}

		nss_cryptoapi_add_ctx2debugfs(ctx);
		atomic_set(&ctx->active, 1);
		atomic_set(&ctx->refcnt, 1);
	}

	/*
	 * Initialze the private context
	 */
	rctx->msg_count = 0;
	rctx->buf_count = 0;
	rctx->diglen = crypto_ahash_digestsize(ahash);
	memset(rctx->digest, 0, ARRAY_SIZE(rctx->digest));

	NSS_CRYPTOAPI_AHASH_SET_MAGIC(rctx);
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
	struct nss_cryptoapi_info info = {.op_dir = NSS_CRYPTO_OP_DIR_AUTH};
	struct crypto_ahash *ahash = crypto_ahash_reqtfm(req);
	struct nss_cryptoapi_ctx *ctx = crypto_ahash_ctx(ahash);
	struct nss_cryptoapi_req_ctx *rctx = ahash_request_ctx(req);
	struct scatterlist *cur;
	uint32_t tot_len = 0;
	int i = 0;

	/*
	 * Check cryptoapi context magic number.
	 */
	NSS_CRYPTOAPI_VERIFY_MAGIC(ctx);
	NSS_CRYPTOAPI_AHASH_VERIFY_MAGIC(rctx);

	/*
	 * Check if cryptoapi context is active or not
	 */
	if (!atomic_read(&ctx->active))
		return -EINVAL;

	/*
	 * .update can only be called one time for one request
	 */
	WARN_ON(rctx->buf_count > 0);

	if (rctx->buf_count > 0)
		return -EPERM;

	rctx->buf_count = 1;

	/*
	 * Fill the request information structure
	 */
	info.src.nsegs = sg_nents(req->src);
	info.dst.nsegs = sg_nents(req->src);
	info.cb = nss_cryptoapi_ahash_done;
	info.hmac_len = crypto_ahash_digestsize(ahash);
	info.src.first_sg = req->src;
	info.src.last_sg = sg_last(req->src, info.src.nsegs);
	info.dst.first_sg = req->src;
	info.dst.last_sg = sg_last(req->src, info.src.nsegs);
	info.in_place = true;
	info.iv = NULL;
	info.iv_size = 0;
	info.skip = 0;

	/*
	 * The exact length of data that needs to be authenticated for an AHASH
	 * request is stored in req->nbytes. Hence we may have to reduce
	 * the DMA length to what is specified in req->nbytes and later
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

	info.ahash_skip = tot_len - req->nbytes;
	info.src.last_sg = cur;

	if (!atomic_inc_not_zero(&ctx->refcnt))
		return -ENOENT;

	return nss_cryptoapi_transform(ctx, &info, (void *)req, true);
}

/*
 * nss_cryptoapi_ahash_final()
 *	Cryptoapi ahash .final operation for a specific request.
 *	Copy the hash result to user
 */
int nss_cryptoapi_ahash_final(struct ahash_request *req)
{
	struct nss_cryptoapi_ctx *ctx __attribute__((unused)) = crypto_tfm_ctx(req->base.tfm);
	struct crypto_ahash *ahash = crypto_ahash_reqtfm(req);
	struct nss_cryptoapi_req_ctx *rctx = ahash_request_ctx(req);

	/*
	 * Check cryptoapi context magic number.
	 */
	NSS_CRYPTOAPI_VERIFY_MAGIC(ctx);
	NSS_CRYPTOAPI_AHASH_VERIFY_MAGIC(rctx);

	BUG_ON(!req->result);
	memcpy(req->result, rctx->digest, crypto_ahash_digestsize(ahash));

	return 0;
}

/*
 * nss_cryptoapi_ahash_digest()
 *	Cryptoapi ahash request .digest operation.
 */
int nss_cryptoapi_ahash_digest(struct ahash_request *req)
{
	if (nss_cryptoapi_ahash_init(req) < 0)
		return -EINVAL;

	return nss_cryptoapi_ahash_update(req);
}

/*
 * nss_cryptoapi_ahash_export()
 *	Cryptoapi ahash .export operation.
 *	Used to export data to other transform.
 *
 * Note: This API is not supported.
 */
int nss_cryptoapi_ahash_export(struct ahash_request *req, void *out)
{
	struct nss_cryptoapi_ctx *ctx __attribute__((unused)) = crypto_tfm_ctx(req->base.tfm);

	nss_cfi_warn("%px: ahash .export is not supported", ctx);
	return -ENOSYS;
};

/*
 * nss_cryptoapi_ahash_import()
 *	Cryptoapi ahash .import operation.
 *	Used to import data from other transform.
 *
 * Note: This API is not supported.
 */
int nss_cryptoapi_ahash_import(struct ahash_request *req, const void *in)
{
	struct nss_cryptoapi_ctx *ctx __attribute__((unused)) = crypto_tfm_ctx(req->base.tfm);

	nss_cfi_warn("%px: ahash .import is not supported", ctx);
	return -ENOSYS;
}
