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
 * nss_cryptoapi_aead.c
 * 	Interface to communicate Native Linux crypto framework specific data
 * 	to Crypto core specific data
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/random.h>
#include <crypto/internal/aead.h>
#include <linux/moduleparam.h>
#include <linux/spinlock.h>
#include <asm/cmpxchg.h>
#include <linux/delay.h>
#include <linux/crypto.h>
#include <linux/rtnetlink.h>
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

/*
 * nss_cryptoapi_aead_ctx2session()
 *	Cryptoapi function to get the session ID for an AEAD
 */
int nss_cryptoapi_aead_ctx2session(struct crypto_aead *aead, uint32_t *sid)
{
	struct crypto_tfm *tfm = crypto_aead_tfm(aead);
	struct nss_cryptoapi_ctx *ctx = crypto_aead_ctx(aead);

	if (strncmp("nss-", crypto_tfm_alg_driver_name(tfm), 4))
		return -EINVAL;

	ctx = crypto_aead_ctx(aead);
	*sid = ctx->sid;

	return 0;
}
EXPORT_SYMBOL(nss_cryptoapi_aead_ctx2session);

/*
 * nss_cryptoapi_aead_init()
 * 	Cryptoapi aead init function.
 */
int nss_cryptoapi_aead_init(struct crypto_aead *aead)
{
	struct crypto_tfm *tfm = crypto_aead_tfm(aead);
	struct nss_cryptoapi_ctx *ctx = crypto_tfm_ctx(tfm);
	struct crypto_aead *sw_tfm;
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
	sw_tfm = crypto_alloc_aead(crypto_tfm_alg_name(tfm), 0, CRYPTO_ALG_ASYNC | CRYPTO_ALG_NEED_FALLBACK);
	if (IS_ERR(sw_tfm)) {
		ctx->stats.failed_init++;
		nss_cfi_err("Unable to allocate fallback for aead:%s\n", crypto_tfm_alg_name(tfm));
		return 0;
	}

	/*
	 * set this tfm reqsize same to fallback tfm
	 */
	crypto_aead_set_reqsize(aead, crypto_aead_reqsize(sw_tfm));
	ctx->sw_tfm = crypto_aead_tfm(sw_tfm);

	return 0;
}

/*
 * nss_cryptoapi_aead_exit()
 * 	Cryptoapi aead exit function.
 */
void nss_cryptoapi_aead_exit(struct crypto_aead *aead)
{
	struct crypto_tfm *tfm = crypto_aead_tfm(aead);
	struct nss_cryptoapi_ctx *ctx = crypto_tfm_ctx(tfm);
	int ret;

	BUG_ON(!ctx);

	if (ctx->sw_tfm) {
		crypto_free_aead(__crypto_aead_cast(ctx->sw_tfm));
		ctx->sw_tfm = NULL;
	}

	ctx->stats.exit++;

	/*
	 * When fallback_req is set, it means that we didn't allocate
	 * session from qca-nss-crypto, it maybe uses software crypto.
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
 * nss_cryptoapi_aead_setkey_noauth()
 * 	Cryptoapi setkey routine for aead algorithms with no authentication key.
 */
int nss_cryptoapi_aead_setkey_noauth(struct crypto_aead *aead, const u8 *key, unsigned int keylen)
{
	struct crypto_tfm *tfm = crypto_aead_tfm(aead);
	struct nss_cryptoapi_ctx *ctx = crypto_tfm_ctx(tfm);
	struct nss_crypto_session_data data = {0};
	const uint8_t *nonce = NULL;
	uint16_t nonce_sz;
	int32_t status;

	/*
	 * Validate magic number - init should be called before setkey
	 */
	NSS_CRYPTOAPI_VERIFY_MAGIC(ctx);

	ctx->info = nss_cryptoapi_cra_name2info(crypto_tfm_alg_name(tfm), keylen, 0);
	if (!ctx->info) {
		nss_cfi_err("%p: Unable to find algorithm with keylen\n", ctx);
		crypto_aead_set_flags(aead, CRYPTO_TFM_RES_BAD_KEY_LEN);
		return -ENOENT;
	}

	if (ctx->info->algo >= NSS_CRYPTO_CMN_ALGO_MAX)
		return -ERANGE;

	nonce_sz = ctx->info->nonce;
	ctx->iv_size = crypto_aead_ivsize(aead) + nonce_sz + sizeof(uint32_t);

	/* Last 4 bytes of the key are nonce in RFC4106 */
	if (ctx->info->cipher_mode != NSS_CRYPTOAPI_CIPHER_MODE_GCM) {
		nonce = key + (keylen - nonce_sz);
		memcpy(ctx->ctx_iv, nonce, nonce_sz);
	}

	ctx->ctx_iv[3] = ntohl(0x1);

	data.algo = ctx->info->algo;
	data.cipher_key = key;
	data.sec_key = false;
	data.nonce = nonce;

	status = nss_crypto_session_alloc(ctx->user, &data, &ctx->sid);
	if (status < 0) {
		nss_cfi_err("%p: Unable to allocate crypto session(%d)\n", ctx, status);
		crypto_aead_set_flags(aead, CRYPTO_TFM_RES_BAD_FLAGS);
		return status;
	}

	nss_cryptoapi_add_ctx2debugfs(ctx);
	atomic_set(&ctx->active, 1);
	atomic_set(&ctx->refcnt, 1);
	return 0;
}

/*
 * nss_cryptoapi_aead_setkey()
 * 	Cryptoapi setkey routine for aead (aes/des/sha) algorithms.
 */
int nss_cryptoapi_aead_setkey(struct crypto_aead *aead, const u8 *key, unsigned int keylen)
{
	struct crypto_tfm *tfm = crypto_aead_tfm(aead);
	struct nss_cryptoapi_ctx *ctx = crypto_tfm_ctx(tfm);
	struct crypto_authenc_keys keys = {0};
	struct nss_crypto_session_data data = {0};
	int32_t status, ret;

	/*
	 * Validate magic number - init should be called before setkey
	 */
	NSS_CRYPTOAPI_VERIFY_MAGIC(ctx);

	/*
	 * Extract and cipher and auth keys
	 */
	if (crypto_authenc_extractkeys(&keys, key, keylen) != 0) {
		nss_cfi_err("%p: Unable to extract keys\n", ctx);
		crypto_aead_set_flags(aead, CRYPTO_TFM_RES_BAD_KEY_LEN);
		return -EIO;
	}

	ctx->info = nss_cryptoapi_cra_name2info(crypto_tfm_alg_name(tfm), keys.enckeylen, crypto_aead_maxauthsize(aead));
	if (!ctx->info) {
		nss_cfi_err("%p: Unable to find algorithm with keylen\n", ctx);
		crypto_aead_set_flags(aead, CRYPTO_TFM_RES_BAD_KEY_LEN);
		return -ENOENT;
	}

	if (ctx->info->algo >= NSS_CRYPTO_CMN_ALGO_MAX)
		return -ERANGE;

	ctx->iv_size = crypto_aead_ivsize(aead);

	if (ctx->info->cipher_mode == NSS_CRYPTOAPI_CIPHER_MODE_CTR_RFC3686) {
		keys.enckeylen = keys.enckeylen - CTR_RFC3686_NONCE_SIZE;
		memcpy(ctx->ctx_iv, keys.enckey + keys.enckeylen, CTR_RFC3686_NONCE_SIZE);
		ctx->ctx_iv[3] = ntohl(0x1);
		ctx->iv_size += CTR_RFC3686_NONCE_SIZE + sizeof(uint32_t);
	}

	/*
	 * Validate if auth key length exceeds what we support
	 */
	if (keys.authkeylen > ctx->info->auth_blocksize) {
		nss_cfi_err("%p: Auth keylen(%d) exceeds supported\n", ctx, keys.authkeylen);
		crypto_aead_set_flags(aead, CRYPTO_TFM_RES_BAD_KEY_LEN);
		return -EINVAL;
	}

	/*
	 * If an algorithm in not enabled in hardware, fallback to software
	 */
	if (ctx->sw_tfm) {
		ctx->fallback_req = true;
		ctx->sid = NSS_CRYPTO_MAX_IDXS;

		/*
		 * Set flags for fallback tfm
		 */
		crypto_tfm_clear_flags(ctx->sw_tfm, CRYPTO_TFM_REQ_MASK);
		crypto_tfm_set_flags(ctx->sw_tfm, crypto_aead_get_flags(aead) & CRYPTO_TFM_REQ_MASK);

		/*
		 * Set the key for fallback tfm */
		ret = crypto_aead_setkey(__crypto_aead_cast(ctx->sw_tfm), key, keylen);
		if (ret) {
			nss_cfi_err("Setkey failed for software fallback\n");
			crypto_aead_set_flags(aead, crypto_tfm_get_flags(ctx->sw_tfm));
		}

		return ret;
	}

	data.algo = ctx->info->algo;
	data.cipher_key = keys.enckey;
	data.auth_key = keys.authkey;
	data.auth_keylen = keys.authkeylen;
	data.sec_key = false;

	status = nss_crypto_session_alloc(ctx->user, &data, &ctx->sid);
	if (status < 0) {
		nss_cfi_err("%p: Unable to allocate crypto session(%d)\n", ctx, status);
		crypto_aead_set_flags(aead, CRYPTO_TFM_RES_BAD_FLAGS);
		return status;
	}

	nss_cryptoapi_add_ctx2debugfs(ctx);
	atomic_set(&ctx->active, 1);
	atomic_set(&ctx->refcnt, 1);
	return 0;
}

/*
 * nss_cryptoapi_aead_setauthsize()
 * 	Cryptoapi set authsize funtion.
 */
int nss_cryptoapi_aead_setauthsize(struct crypto_aead *authenc, unsigned int authsize)
{
	struct nss_cryptoapi_ctx *ctx = crypto_aead_ctx(authenc);

	NSS_CRYPTOAPI_VERIFY_MAGIC(ctx);
	ctx->authsize = (uint16_t)authsize;

	if (ctx->sw_tfm)
		crypto_aead_setauthsize(__crypto_aead_cast(ctx->sw_tfm), authsize);

	return 0;
}

/*
 * nss_cryptoapi_aead_done()
 * 	Cipher/Auth encrypt request completion callback function
 */
void nss_cryptoapi_aead_done(void *app_data, struct nss_crypto_hdr *ch, uint8_t status)
{
	struct aead_request *req = (struct aead_request *)app_data;
	struct crypto_aead *aead = crypto_aead_reqtfm(req);
	struct nss_cryptoapi_ctx *ctx = crypto_aead_ctx(aead);
	int error;

	BUG_ON(!ch);

	/*
	 * check cryptoapi context magic number.
	 */
	NSS_CRYPTOAPI_VERIFY_MAGIC(ctx);
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
	aead_request_complete(req, error);
}

/*
 * nss_cryptoapi_aead_echainiv_tx_proc()
 *	Fill echainiv information structure
 */
void nss_cryptoapi_aead_echainiv_tx_proc(struct nss_cryptoapi_ctx *ctx, struct aead_request *req,
					struct nss_cryptoapi_info *info, bool encrypt)
{
	struct crypto_aead *aead = crypto_aead_reqtfm(req);

	memcpy(req->iv, sg_virt(req->src) + req->assoclen, crypto_aead_ivsize(aead));

	info->iv = req->iv;
	info->nsegs = sg_nents(req->src);
	info->cb = nss_cryptoapi_aead_done;
	info->iv_size = ctx->iv_size;
	info->hmac_len = crypto_aead_authsize(aead);
	info->ahash_skip = encrypt ? crypto_aead_authsize(aead) : 0;
	info->first_sg = req->src;
	info->last_sg = sg_last(req->src, info->nsegs);
	info->skip = req->assoclen + crypto_aead_ivsize(aead);
}

/*
 * nss_cryptoapi_aead_seqiv_tx_proc()
 *	Fill seqiv information structure
 */
void nss_cryptoapi_aead_seqiv_tx_proc(struct nss_cryptoapi_ctx *ctx, struct aead_request *req,
					struct nss_cryptoapi_info *info, bool encrypt)
{
	struct crypto_aead *aead = crypto_aead_reqtfm(req);

	memcpy(req->iv, sg_virt(req->src) + req->assoclen, crypto_aead_ivsize(aead));

	/*
	 * Fill the request information structure
	 * Note: For CTR mode, IV size will be set to AES_BLOCK_SIZE.
	 * This is because linux gives iv size as 8 while we need to
	 * alloc 16 bytes in crypto hdr to accomodate:
	 * - 4 bytes of nonce
	 * - 8 bytes of IV
	 * - 4 bytes of initial counter
	 */
	info->iv = req->iv;
	info->nsegs = sg_nents(req->src);
	info->cb = nss_cryptoapi_aead_done;
	info->iv_size = ctx->iv_size;
	info->hmac_len = crypto_aead_authsize(aead);
	info->ahash_skip = encrypt ? crypto_aead_authsize(aead) : 0;
	info->first_sg = req->src;
	info->last_sg = sg_last(req->src, info->nsegs);
	info->skip = req->assoclen + crypto_aead_ivsize(aead);

	/*
	 * For gcm RFC4106, IV is not part of authentication
	 */
	if (ctx->info->cipher_mode == NSS_CRYPTOAPI_CIPHER_MODE_GCM_RFC4106)
		info->auth = req->assoclen;
}

/*
 * nss_cryptoapi_aead_tx_proc()
 *	Fill authenc information structure
 */
void nss_cryptoapi_aead_tx_proc(struct nss_cryptoapi_ctx *ctx, struct aead_request *req,
				struct nss_cryptoapi_info *info, bool encrypt)
{
	struct crypto_aead *aead = crypto_aead_reqtfm(req);

	info->iv = req->iv;
	info->nsegs = sg_nents(req->src);
	info->cb = nss_cryptoapi_aead_done;
	info->iv_size = ctx->iv_size;
	info->hmac_len = crypto_aead_authsize(aead);
	info->ahash_skip = encrypt ? crypto_aead_authsize(aead) : 0;
	info->first_sg = req->src;
	info->last_sg = sg_last(req->src, info->nsegs);
	info->skip = req->assoclen;

	/*
	 * This api is called from tcrypt, for GCM case assoclen include
	 * IV also, while IV is not part of authentication as per RFC4106
	 */
	if (ctx->info->cipher_mode == NSS_CRYPTOAPI_CIPHER_MODE_GCM_RFC4106)
		info->auth = req->assoclen - crypto_aead_ivsize(aead);
	 else if (ctx->info->cipher_mode == NSS_CRYPTOAPI_CIPHER_MODE_GCM)
		info->auth = req->assoclen;
}

/*
 * nss_cryptoapi_aead_encrypt()
 * 	Crytoapi common encrypt for (AES/3DES) (SHA1/SHA256 with CBC) algorithms
 */
int nss_cryptoapi_aead_encrypt(struct aead_request *req)
{
	struct nss_cryptoapi_info info = {.op_dir = NSS_CRYPTO_OP_DIR_ENC_AUTH};
	struct crypto_aead *aead = crypto_aead_reqtfm(req);
	struct nss_cryptoapi_ctx *ctx = crypto_aead_ctx(aead);
	int error;

	/*
	 * Check cryptoapi context magic number
	 */
	NSS_CRYPTOAPI_VERIFY_MAGIC(ctx);

	if (ctx->fallback_req) {
		struct crypto_aead *orig_tfm = crypto_aead_reqtfm(req);

		BUG_ON(!ctx->sw_tfm);
		aead_request_set_tfm(req, __crypto_aead_cast(ctx->sw_tfm));
		ctx->stats.queued++;

		error = crypto_aead_encrypt(req);
		if (!error)
			ctx->stats.completed++;

		aead_request_set_tfm(req, orig_tfm);
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

	BUG_ON(!ctx->info->aead_tx_proc);
	ctx->info->aead_tx_proc(ctx, req, &info, true);

	if (!atomic_inc_not_zero(&ctx->refcnt))
		return -ENOENT;

	return nss_cryptoapi_transform(ctx, &info, (void *)req, false);
}

/*
 * nss_cryptoapi_aead_decrypt()
 * 	Crytoapi common decrypt for (AES/3DES) (SHA1/SHA256 with CBC) algorithms
 */
int nss_cryptoapi_aead_decrypt(struct aead_request *req)
{
	struct nss_cryptoapi_info info = {.op_dir = NSS_CRYPTO_OP_DIR_AUTH_DEC};
	struct crypto_aead *aead = crypto_aead_reqtfm(req);
	struct nss_cryptoapi_ctx *ctx = crypto_aead_ctx(aead);
	int error;

	/*
	 * Check cryptoapi context magic number.
	 */
	NSS_CRYPTOAPI_VERIFY_MAGIC(ctx);

	if (ctx->fallback_req) {
		struct crypto_aead *orig_tfm = crypto_aead_reqtfm(req);

		BUG_ON(!ctx->sw_tfm);
		aead_request_set_tfm(req, __crypto_aead_cast(ctx->sw_tfm));
		ctx->stats.queued++;

		error = crypto_aead_decrypt(req);
		if (!error)
			ctx->stats.completed++;

		aead_request_set_tfm(req, orig_tfm);
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

	BUG_ON(!ctx->info->aead_tx_proc);
	ctx->info->aead_tx_proc(ctx, req, &info, false);

	if (!atomic_inc_not_zero(&ctx->refcnt))
		return -ENOENT;

	return nss_cryptoapi_transform(ctx, &info, (void *)req, false);
}
