/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: ISC
 */

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kref.h>
#include <linux/crypto.h>
#include <linux/slab.h>
#include <linux/completion.h>

#include <crypto/md5.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
#include <crypto/sha.h>
#else
#include <crypto/sha1.h>
#include <crypto/sha2.h>
#endif
#include <crypto/sha3.h>
#include <crypto/aes.h>

#include "eip_priv.h"

/*
 * Function prototypes.
 */
bool eip_tr_ahash_init(struct eip_tr *tr, struct eip_tr_info *info, const struct eip_svc_entry *algo);

/*
 * Global constant algorith information.
 */
static const struct eip_svc_entry ahash_algo_list[] = {
	/*
	 * *******************************
	 * Async non-keyed HASH digests. *
	 * *******************************
	 */
	{
		.name = "eip-md5",
		.enc_tk_fill = NULL,
		.dec_tk_fill = NULL,
		.auth_tk_fill = &eip_tk_auth,
		.auth_block_len = MD5_HMAC_BLOCK_SIZE,
		.auth_state_len = 0,
		.auth_digest_len = MD5_DIGEST_SIZE,
		.tr_init = &eip_tr_ahash_init,
		.ctrl_words_0 = EIP_HW_CTX_ALGO_MD5,
		.ctrl_words_1 = 0x0,
	},
	{
		.name = "eip-sha1",
		.enc_tk_fill = NULL,
		.dec_tk_fill = NULL,
		.auth_tk_fill = &eip_tk_auth,
		.auth_block_len = SHA1_BLOCK_SIZE,
		.auth_state_len = 0,
		.auth_digest_len = SHA1_DIGEST_SIZE,
		.tr_init = &eip_tr_ahash_init,
		.ctrl_words_0 = EIP_HW_CTX_ALGO_SHA1,
		.ctrl_words_1 = 0x0,
	},
	{
		.name = "eip-sha224",
		.enc_tk_fill = NULL,
		.dec_tk_fill = NULL,
		.auth_tk_fill = &eip_tk_auth,
		.auth_block_len = SHA224_BLOCK_SIZE,
		.auth_state_len = 0,
		.auth_digest_len = SHA224_DIGEST_SIZE,
		.tr_init = &eip_tr_ahash_init,
		.ctrl_words_0 = EIP_HW_CTX_ALGO_SHA224,
		.ctrl_words_1 = 0x0,
	},
	{
		.name = "eip-sha256",
		.enc_tk_fill = NULL,
		.dec_tk_fill = NULL,
		.auth_tk_fill = &eip_tk_auth,
		.auth_block_len = SHA256_BLOCK_SIZE,
		.auth_state_len = 0,
		.auth_digest_len = SHA256_DIGEST_SIZE,
		.tr_init = &eip_tr_ahash_init,
		.ctrl_words_0 = EIP_HW_CTX_ALGO_SHA256,
		.ctrl_words_1 = 0x0,
	},
	{
		.name = "eip-sha384",
		.enc_tk_fill = NULL,
		.dec_tk_fill = NULL,
		.auth_tk_fill = &eip_tk_auth,
		.auth_block_len = SHA384_BLOCK_SIZE,
		.auth_state_len = 0,
		.auth_digest_len = SHA384_DIGEST_SIZE,
		.tr_init = &eip_tr_ahash_init,
		.ctrl_words_0 = EIP_HW_CTX_ALGO_SHA384,
		.ctrl_words_1 = 0x0,
	},
	{
		.name = "eip-sha512",
		.enc_tk_fill = NULL,
		.dec_tk_fill = NULL,
		.auth_tk_fill = &eip_tk_auth,
		.auth_block_len = SHA512_BLOCK_SIZE,
		.auth_state_len = 0,
		.auth_digest_len = SHA512_DIGEST_SIZE,
		.tr_init = &eip_tr_ahash_init,
		.ctrl_words_0 = EIP_HW_CTX_ALGO_SHA512,
		.ctrl_words_1 = 0x0,
	},

	/*
	 * ******************************
	 * Aynsc keyed HMAC digests.	*
	 * ******************************
	 */
	{
		.name = "eip-hmac-md5",
		.enc_tk_fill = NULL,
		.dec_tk_fill = NULL,
		.auth_tk_fill = &eip_tk_auth,
		.auth_block_len = MD5_HMAC_BLOCK_SIZE,
		.auth_state_len = EIP_HW_PAD_KEYSZ_MD5,
		.auth_digest_len = MD5_DIGEST_SIZE,
		.tr_init = &eip_tr_ahash_init,
		.ctrl_words_0 = EIP_HW_CTX_ALGO_MD5 | EIP_HW_CTX_AUTH_MODE_HMAC |
			EIP_HW_CTX_SIZE_8WORDS,
		.ctrl_words_1 = 0x0,
	},
	{
		.name = "eip-hmac-sha1",
		.enc_tk_fill = NULL,
		.dec_tk_fill = NULL,
		.auth_tk_fill = &eip_tk_auth,
		.auth_block_len = SHA1_BLOCK_SIZE,
		.auth_state_len = EIP_HW_PAD_KEYSZ_SHA1,
		.auth_digest_len = SHA1_DIGEST_SIZE,
		.tr_init = &eip_tr_ahash_init,
		.ctrl_words_0 = EIP_HW_CTX_ALGO_SHA1 | EIP_HW_CTX_AUTH_MODE_HMAC |
			EIP_HW_CTX_SIZE_10WORDS,
		.ctrl_words_1 = 0x0,
	},
	{
		.name = "eip-hmac-sha224",
		.enc_tk_fill = NULL,
		.dec_tk_fill = NULL,
		.auth_tk_fill = &eip_tk_auth,
		.auth_block_len = SHA224_BLOCK_SIZE,
		.auth_state_len = EIP_HW_PAD_KEYSZ_SHA224,
		.auth_digest_len = SHA224_DIGEST_SIZE,
		.tr_init = &eip_tr_ahash_init,
		.ctrl_words_0 = EIP_HW_CTX_ALGO_SHA224 | EIP_HW_CTX_AUTH_MODE_HMAC |
			EIP_HW_CTX_SIZE_16WORDS,
		.ctrl_words_1 = 0x0,
	},
	{
		.name = "eip-hmac-sha256",
		.enc_tk_fill = NULL,
		.dec_tk_fill = NULL,
		.auth_tk_fill = &eip_tk_auth,
		.auth_block_len = SHA256_BLOCK_SIZE,
		.auth_state_len = EIP_HW_PAD_KEYSZ_SHA256,
		.auth_digest_len = SHA256_DIGEST_SIZE,
		.tr_init = &eip_tr_ahash_init,
		.ctrl_words_0 = EIP_HW_CTX_ALGO_SHA256 | EIP_HW_CTX_AUTH_MODE_HMAC |
			EIP_HW_CTX_SIZE_16WORDS,
		.ctrl_words_1 = 0x0,
	},
	{
		.name = "eip-hmac-sha384",
		.enc_tk_fill = NULL,
		.dec_tk_fill = NULL,
		.auth_tk_fill = &eip_tk_auth,
		.auth_block_len = SHA384_BLOCK_SIZE,
		.auth_state_len = EIP_HW_PAD_KEYSZ_SHA384,
		.auth_digest_len = SHA384_DIGEST_SIZE,
		.tr_init = &eip_tr_ahash_init,
		.ctrl_words_0 = EIP_HW_CTX_ALGO_SHA384 | EIP_HW_CTX_AUTH_MODE_HMAC |
			EIP_HW_CTX_SIZE_32WORDS,
		.ctrl_words_1 = 0x0,
	},
	{
		.name = "eip-hmac-sha512",
		.enc_tk_fill = NULL,
		.dec_tk_fill = NULL,
		.auth_tk_fill = &eip_tk_auth,
		.auth_block_len = SHA512_BLOCK_SIZE,
		.auth_state_len = EIP_HW_PAD_KEYSZ_SHA512,
		.auth_digest_len = SHA512_DIGEST_SIZE,
		.tr_init = &eip_tr_ahash_init,
		.ctrl_words_0 = EIP_HW_CTX_ALGO_SHA512 | EIP_HW_CTX_AUTH_MODE_HMAC |
			EIP_HW_CTX_SIZE_32WORDS,
		.ctrl_words_1 = 0x0,
	},

};

/*
 * eip_tr_ahash_tx_complete()
 *	Free Resources held during transmit.
 *
 * TR should not be used after calling this function.
 */
static inline void eip_tr_ahash_tx_complete(struct eip_tr *tr, struct eip_sw_desc *sw)
{
	struct eip_ctx *ctx = tr->ctx;

	/*
	 * free the memory associated with the operation.
	 */
	kmem_cache_free(ctx->tk_cache, sw->tk);
	kmem_cache_free(ctx->sw_cache, sw);

	/*
	 * Reference: eip_tr_ahash_digest()
	 */
	eip_tr_deref(tr);
}

/*
 * eip_tr_ahash_digest_err()
 *	Error in Digest DMA operation.
 */
static void eip_tr_ahash_digest_err(struct eip_tr *tr, struct eip_hw_desc *hw, struct eip_sw_desc *sw, uint16_t cle_err, uint16_t tr_err)
{
	/*
	 * Digest computation must not fail unless there is any programming error.
	 */
	pr_err("%px: Digest completed with cle_err(%u) tr_err(%u)\n", tr, cle_err, tr_err);
	BUG_ON(1);
}

/*
 * eip_tr_ahash_digest_done()
 *	Finish Digest DMA operation.
 */
static void eip_tr_ahash_digest_done(struct eip_tr *tr, struct eip_hw_desc *hw, struct eip_sw_desc *sw)
{
	struct completion *completion = sw->req;

	/*
	 * Free transmit resources.
	 */
	eip_tr_ahash_tx_complete(tr, sw);

	pr_debug("%px: Digest completed\n", tr);
	complete(completion);
}

/*
 * eip_tr_ahash_init()
 *	Initialize the transform record for Crypto AHASH.
 */
bool eip_tr_ahash_init(struct eip_tr *tr, struct eip_tr_info *info, const struct eip_svc_entry *algo)
{
	struct eip_tr_info_crypto *crypto = &info->crypto;
	struct eip_tr_base *base = &info->base;
	uint32_t *tr_words = tr->hw_words;
	uint32_t size;

	if (base->cipher.key_len) {
		pr_err("%px: Cipher key length must be zero for ahash (%u)\n", tr, base->cipher.key_len);
		return false;
	}

	/*
	 * We use small record for aead.
	 */
	tr->tr_addr_type = EIP_HW_CTX_TYPE_SMALL;
	tr->tr_addr_type |= virt_to_phys(tr->hw_words);
	tr->digest_len = algo->auth_digest_len;
	tr->ctrl_words[0] = algo->ctrl_words_0;
	tr->ctrl_words[1] = algo->ctrl_words_1;

	tr->crypto.enc.tk_fill = NULL;
	tr->crypto.enc.cb = NULL;
	tr->crypto.enc.err_cb = NULL;
	tr->crypto.enc.app_data = NULL;
	tr->crypto.dec.tk_fill = NULL;
	tr->crypto.dec.cb = NULL;
	tr->crypto.dec.err_cb = NULL;
	tr->crypto.dec.app_data = NULL;
	tr->crypto.auth.tk_fill = algo->auth_tk_fill;
	tr->crypto.auth.cb = crypto->auth_cb;
	tr->crypto.auth.err_cb = crypto->auth_err_cb;
	tr->crypto.auth.app_data = crypto->app_data;

	/*
	 * For crypto, Control words are in tokens.
	 */
	*tr_words++ = 0;
	*tr_words++ = 0;

	/*
	 * Non-keyed algorithm.
	 */
	if (!base->auth.key_len) {
		/*
		 * Flush record memory.
		 * Driver must not make any update in transform records words after this.
		 * EIP197 will do ipad/opad update.
		 */
		eip_dmac_clean_range(tr->hw_words, tr->hw_words + EIP_HW_CTX_SIZE_SMALL_WORDS);
		return true;
	}

	/*
	 * Leave the space for inner & outer digest only for HMAC.
	 */
	size = algo->auth_state_len;
	tr_words += (size / sizeof(uint32_t));
	tr_words += (size / sizeof(uint32_t));

	/*
	 * Flush record memory.
	 * Driver must not make any update in transform records words after this.
	 * EIP197 will do ipad/opad update.
	 */
	eip_dmac_clean_range(tr->hw_words, tr->hw_words+ EIP_HW_CTX_SIZE_SMALL_WORDS);

	/*
	 * HMAC Inner and Outer Digest Precalculation
	 */
	if (!eip_tr_ahash_key2digest(tr, info, algo)) {
		pr_err("%px: Digest computation failed for algo(%s)\n", tr, algo->name);
		return false;
	}

	return true;
}

/*
 * eip_tr_ahash_get_svc()
 * 	Get algo database for ahash.
 */
const struct eip_svc_entry *eip_tr_ahash_get_svc(void)
{
	return ahash_algo_list;
}

/*
 * eip_tr_ahash_get_svc_len()
 * 	Get algo database length for ahash.
 */
size_t eip_tr_ahash_get_svc_len(void)
{
	return ARRAY_SIZE(ahash_algo_list);
}

/*
 * eip_tr_ahash_digest()
 *	Generate the digest keys for hash.
 */
bool eip_tr_ahash_key2digest(struct eip_tr *tr, struct eip_tr_info *info, const struct eip_svc_entry *algo)
{
	struct eip_tr_base *base = &info->base;
	struct eip_ctx *ctx = tr->ctx;
	uint32_t tk_hdr = 0;
	struct eip_sw_desc *sw;
	struct scatterlist sg;
	struct eip_dma *dma;
	struct eip_tk *tk;
	uint8_t pad_offst;
	void *key_data;
	uint32_t data_len;
	uint32_t tk_words;

	DECLARE_COMPLETION_ONSTACK(completion);

	/*
	 * Sanity check.
	 */
	if (!tr->digest_len || !base->auth.key_len) {
		pr_err("%px: Invalid hmac parameter, digest_len(%u) key len(%u)\n", tr,
				tr->digest_len, base->auth.key_len);
		return false;
	}

	/*
	 * Allocate inner & outer padded keys for precalculation.
	 * Each key will be of block length.
	 */
	data_len = base->auth.key_len;
	key_data = kzalloc(data_len, GFP_KERNEL);
	if (!key_data) {
		pr_err("%px: Failed to allocate key data\n", tr);
		return false;
	}

	memcpy(key_data, base->auth.key_data, base->auth.key_len);

	/*
	 * Initialize data scatterlist for DMA operation.
	 */
	sg_init_one(&sg, key_data, data_len);

	/*
	 * Allocate HW token.
	 */
	tk = kmem_cache_alloc(ctx->tk_cache, GFP_KERNEL);
	if (!tk) {
		pr_err("%px: Failed to allocate Token\n", tr);
		goto fail_alloc_tk;
	}

	/*
	 * Allocate SW descriptor.
	 */
	sw = kmem_cache_alloc(ctx->sw_cache, GFP_KERNEL);
	if (!sw) {
		pr_err("%px: Failed to allocate SW descriptor\n", tr);
		goto fail_alloc_desc;
	}

	/*
	 * Fill token for digest.
	 */
	pad_offst = EIP_HW_CTRL_WORDS + (base->cipher.key_len / sizeof(uint32_t));
	tk_words = eip_tk_digest(tk, tr, &sg, &tk_hdr, pad_offst, algo->auth_state_len / sizeof(uint32_t));

	/*
	 * Flush & invalidate token memory for device. We should not access token after this.
	 */
	eip_dmac_clean_range(tk, tk + 1);

	/*
	 * Fill software descriptor.
	 */
	sw->tr = eip_tr_ref(tr);
	sw->tk = tk;
	sw->comp = &eip_tr_ahash_digest_done;
	sw->err_comp = &eip_tr_ahash_digest_err;
	sw->tk_hdr = tk_hdr;
	sw->tk_addr = virt_to_phys(tk);
	sw->tr_addr_type = tr->tr_addr_type;
	sw->tk_words = tk_words;
	sw->hw_svc = EIP_HW_CMD_HWSERVICE_LAC;
	sw->req = &completion;

	/*
	 * Schedule the transformation.
	 * There is potential chance that DMA fails to schedule this request during peak
	 * traffic. In that case we need to reschedule the digest calculation.
	 */
	dma = &ctx->ep->la[smp_processor_id()];

retry:
	if (eip_dma_tx_sg(dma, sw, &sg, &sg)) {
		pr_warn("%px: DMA is busy for digest schedule\n", tr);

		/*
		 * TODO: Find better way to handle this scenario than sleeping.
		 */
		usleep_range(1000, 1000);
		goto retry;
	}

	/*
	 * Wait for digest computation. We only free data memory here.
	 * TK & SW free is done in completion.
	 */
	pr_debug("%px: Waiting for Digest completion\n", tr);
	wait_for_completion(&completion);
	kfree(key_data);
	return true;

fail_alloc_desc:
	kmem_cache_free(ctx->tk_cache, tk);
fail_alloc_tk:
	kfree(key_data);
	return false;
}

/*
 * eip_tr_ahash_done()
 *	Hardware callback API for authentication.
 */
static void eip_tr_ahash_done(struct eip_tr *tr, struct eip_hw_desc *hw, struct eip_sw_desc *sw)
{
	eip_req_t eip_req = sw->req;
	struct eip_ctx *ctx = tr->ctx;
	void *app_data = tr->crypto.auth.app_data;
	eip_tr_callback_t cb = tr->crypto.auth.cb;

	/*
	 * Free token and sw cache.
	 */
	kmem_cache_free(ctx->tk_cache, sw->tk);
	kmem_cache_free(ctx->sw_cache, sw);

	/*
	 * Reference: eip_tr_skcipher_enc()
	 */
	eip_tr_deref(tr);

	/*
	 * Call the client callback.
	 */
	cb(app_data, eip_req);
}

/*
 * eip_tr_ahash_err()
 *	Hardware callback API for error in authentication.
 */
static void eip_tr_ahash_err(struct eip_tr *tr, struct eip_hw_desc *hw, struct eip_sw_desc *sw, uint16_t cle_err, uint16_t tr_err)
{
	eip_req_t eip_req = sw->req;
	struct eip_ctx *ctx = tr->ctx;
	void *app_data = tr->crypto.auth.app_data;
	eip_tr_err_callback_t cb = tr->crypto.auth.err_cb;
	int err = 0;

	/*
	 * Free token and sw cache.
	 */
	kmem_cache_free(ctx->tk_cache, sw->tk);
	kmem_cache_free(ctx->sw_cache, sw);

	/*
	 * Reference: eip_tr_skcipher_enc()
	 */
	eip_tr_deref(tr);

	/*
	 * Classify hardware errors and convert to linux errors.
	 */
	err = eip_tr_classify_err(cle_err, tr_err);

	/*
	 * Call the client callback.
	 */
	cb(app_data, eip_req, err);
}

/*
 * eip_tr_ahash_auth()
 *	Authentication API.
 */
int eip_tr_ahash_auth(struct eip_tr *tr, struct ahash_request *req, struct scatterlist *res)
{
	struct eip_tk_params tk_params = {0};
	struct eip_ctx *ctx = tr->ctx;
	struct eip_sw_desc *sw;
	struct eip_dma *dma;
	struct eip_tk *tk;
	uint32_t tk_words;
	int status = 0;

	/*
	 * Allocate HW token.
	 */
	tk = kmem_cache_alloc(ctx->tk_cache, GFP_NOWAIT | __GFP_NOWARN);
	if (!tk) {
		pr_err("%px: Failed to allocate token.\n", tr);
		return -ENOMEM;
	}

	/*
	 * Fill TK params
	 */
	tk_params.tr = tr;
	tk_params.eip_req = req;

	/*
	 * Fill token for hmac and decryption.
	 */
	tk_words = eip_tr_fill_token(tk, &tr->crypto.auth, &tk_params);

	eip_dmac_clean_range(tk, tk + 1);

	/*
	 * Allocate SW descriptor.
	 */
	sw = kmem_cache_alloc(ctx->sw_cache, GFP_NOWAIT | __GFP_NOWARN);
	if (!sw) {
		pr_err("%px: Failed to allocate SW descriptor.\n", tr);
		status = -ENOMEM;
		goto fail_sw_alloc;
	}

	/*
	 * Fill software descriptor.
	 */
	sw->tr = eip_tr_ref(tr);
	sw->tk = tk;
	sw->comp = &eip_tr_ahash_done;
	sw->err_comp = &eip_tr_ahash_err;
	sw->tk_hdr = tk_params.tk_hdr;
	sw->tk_addr = virt_to_phys(tk);
	sw->tr_addr_type = tr->tr_addr_type;
	sw->tk_words = tk_words;
	sw->hw_svc = EIP_HW_CMD_HWSERVICE_LAC;
	sw->req = req;

	dma = &ctx->dma[smp_processor_id()];

	status = eip_dma_tx_sg(dma, sw, req->src, res);
	if (status < 0) {
		dma->stats.tx_error++;
		goto fail_tx;
	}

	return 0;
fail_tx:
	eip_tr_deref(tr);
	kmem_cache_free(ctx->sw_cache, sw);
fail_sw_alloc:
	kmem_cache_free(ctx->tk_cache, tk);
	return status;
}
EXPORT_SYMBOL(eip_tr_ahash_auth);
