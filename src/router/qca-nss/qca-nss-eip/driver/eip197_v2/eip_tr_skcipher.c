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
#include <crypto/des.h>

#include "eip_priv.h"

/*
 * Function prototypes.
 */
bool eip_tr_skcipher_init(struct eip_tr *tr, struct eip_tr_info *info, const struct eip_svc_entry *algo);

/*
 * Global constant algorith information.
 */
static const struct eip_svc_entry skcipher_algo_list[] = {
	{
		.name = "eip-des3_ede-cbc",
		.auth_block_len = 0,
		.auth_state_len = 0,
		.auth_digest_len = 0,
		.auth_tk_fill = NULL,
		.enc_tk_fill = &eip_tk_enc_3des,
		.dec_tk_fill = &eip_tk_dec_3des,
		.tr_init = &eip_tr_skcipher_init,
		.ctrl_words_0 = EIP_HW_CTX_WITH_KEY | EIP_HW_CTX_ALGO_3DES,
		.ctrl_words_1 = EIP_HW_CTX_CIPHER_MODE_CBC,
	},
	{
		.name = "eip-aes-cbc",
		.auth_block_len = 0,
		.auth_state_len = 0,
		.auth_digest_len = 0,
		.auth_tk_fill = NULL,
		.enc_tk_fill = &eip_tk_enc,
		.dec_tk_fill = &eip_tk_dec,
		.tr_init = &eip_tr_skcipher_init,
		.ctrl_words_0 = EIP_HW_CTX_WITH_KEY,
		.ctrl_words_1 = EIP_HW_CTX_CIPHER_MODE_CBC,
	},
	{
		.name = "eip-aes-ctr-rfc3686",
		.auth_block_len = 0,
		.auth_state_len = 0,
		.auth_digest_len = 0,
		.auth_tk_fill = NULL,
		.enc_tk_fill = &eip_tk_enc_ctr_rfc,
		.dec_tk_fill = &eip_tk_dec_ctr_rfc,
		.tr_init = &eip_tr_skcipher_init,
		.ctrl_words_0 = EIP_HW_CTX_WITH_KEY,
		.ctrl_words_1 = EIP_HW_CTX_CIPHER_MODE_CTR,
	},
	{
		.name = "eip-aes-ecb",
		.auth_block_len = 0,
		.auth_state_len = 0,
		.auth_digest_len = 0,
		.auth_tk_fill = NULL,
		.enc_tk_fill = &eip_tk_enc,
		.dec_tk_fill = &eip_tk_dec,
		.tr_init = &eip_tr_skcipher_init,
		.ctrl_words_0 = EIP_HW_CTX_WITH_KEY,
		.ctrl_words_1 = EIP_HW_CTX_CIPHER_MODE_ECB,
	},
};

/*
 * eip_tr_skcipher_init()
 *	Initialize the transform record for Crypto SKCIPHER.
 */
bool eip_tr_skcipher_init(struct eip_tr *tr, struct eip_tr_info *info, const struct eip_svc_entry *algo)
{
	struct eip_tr_info_crypto *crypto = &info->crypto;
	struct eip_tr_base *base = &info->base;
	uint32_t *tr_words = tr->hw_words;
	uint8_t ctx_sz;

	/*
	 * We use small record for skcipher.
	 */
	tr->tr_addr_type = EIP_HW_CTX_TYPE_SMALL;
	tr->tr_addr_type |= virt_to_phys(tr->hw_words);
	tr->ctrl_words[0] = algo->ctrl_words_0;
	tr->ctrl_words[1] = algo->ctrl_words_1;
	tr->nonce = base->nonce;

	tr->crypto.enc.tk_fill = algo->enc_tk_fill;
	tr->crypto.enc.cb = crypto->enc_cb;
	tr->crypto.enc.err_cb = crypto->enc_err_cb;
	tr->crypto.enc.app_data = crypto->app_data;
	tr->crypto.dec.tk_fill = algo->dec_tk_fill;
	tr->crypto.dec.cb = crypto->dec_cb;
	tr->crypto.dec.err_cb = crypto->dec_err_cb;
	tr->crypto.dec.app_data = crypto->app_data;
	tr->crypto.auth.tk_fill = NULL;
	tr->crypto.auth.cb = NULL;
	tr->crypto.auth.err_cb = NULL;
	tr->crypto.auth.app_data = NULL;

	/*
	 * For crypto, Control words are in tokens.
	 */
	*tr_words++ = 0;
	*tr_words++ = 0;

	/*
	 * Fill cipher key.
	 */
	memcpy(tr_words, base->cipher.key_data, base->cipher.key_len);
	tr_words += (base->cipher.key_len /  sizeof(uint32_t));

	/*
	 * Fill control words.
	 */
	tr->ctrl_words[0] = algo->ctrl_words_0;
	tr->ctrl_words[1] = algo->ctrl_words_1;
	ctx_sz = tr_words - tr->hw_words - EIP_HW_MAX_CTRL;

	switch(base->cipher.key_len) {
	case AES_KEYSIZE_128:
		tr->ctrl_words[0] |= (EIP_HW_CTX_ALGO_AES128 | EIP_TR_CTRL_CONTEXT_WORDS(ctx_sz));
		break;
	case AES_KEYSIZE_192:
		/*
		 * As keylen is same for AES192 and 3DES we need to handle 3DES case differently.
		 */
		if (!strncmp(base->alg_name, "eip-des3_ede-cbc", strlen("eip-des3_ede-cbc"))) {
			tr->ctrl_words[0] |= (EIP_HW_CTX_ALGO_3DES | EIP_TR_CTRL_CONTEXT_WORDS(ctx_sz));
			break;
		}

		tr->ctrl_words[0] |= (EIP_HW_CTX_ALGO_AES192 | EIP_TR_CTRL_CONTEXT_WORDS(ctx_sz));
		break;
	case AES_KEYSIZE_256:
		tr->ctrl_words[0] |= (EIP_HW_CTX_ALGO_AES256 | EIP_TR_CTRL_CONTEXT_WORDS(ctx_sz));
		break;
	default:
		pr_err("%px: Invalid cipher key_len(%u)\n", tr, base->cipher.key_len);
		return false;
	}

	/*
	 * Flush record memory.
	 * Driver must not make any update in transform records words after this.
	 * EIP197 will do ipad/opad update.
	 */
	eip_dmac_clean_range(tr->hw_words, tr->hw_words+ EIP_HW_CTX_SIZE_SMALL_WORDS);

	return true;
}

/*
 * eip_tr_skcipher_get_svc()
 *	Get algo database for skcipher.
 */
const struct eip_svc_entry *eip_tr_skcipher_get_svc(void)
{
	return skcipher_algo_list;
}

/*
 * eip_tr_skcipher_get_svc_len()
 *	Get algo database length for skcipher.
 */
size_t eip_tr_skcipher_get_svc_len(void)
{
	return ARRAY_SIZE(skcipher_algo_list);
}

/*
 * eip_tr_skcipher_done()
 *	Hardware callback API for encryption.
 */
static void eip_tr_skcipher_enc_done(struct eip_tr *tr, struct eip_hw_desc *hw, struct eip_sw_desc *sw)
{
	eip_req_t eip_req = sw->req;
	struct eip_ctx *ctx = tr->ctx;
	eip_tr_callback_t cb = tr->crypto.enc.cb;
	void *app_data = tr->crypto.enc.app_data;

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
* eip_tr_skcipher_enc_err()
*	Hardware callback API for error in encryption.
*/
static void eip_tr_skcipher_enc_err(struct eip_tr *tr, struct eip_hw_desc *hw, struct eip_sw_desc *sw, uint16_t cle_err, uint16_t tr_err)
{
	eip_req_t eip_req = sw->req;
	struct eip_ctx *ctx = tr->ctx;
	eip_tr_err_callback_t cb = tr->crypto.enc.err_cb;
	void *app_data = tr->crypto.enc.app_data;
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
* eip_tr_skcipher_done()
*	Hardware callback API for decryption.
*/
static void eip_tr_skcipher_dec_done(struct eip_tr *tr, struct eip_hw_desc *hw, struct eip_sw_desc *sw)
{
	eip_req_t eip_req = sw->req;
	struct eip_ctx *ctx = tr->ctx;
	eip_tr_callback_t cb = tr->crypto.dec.cb;
	void *app_data = tr->crypto.dec.app_data;

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
* eip_tr_skcipher_dec_err()
*	Hardware callback API for error in decryption.
*/
static void eip_tr_skcipher_dec_err(struct eip_tr *tr, struct eip_hw_desc *hw, struct eip_sw_desc *sw, uint16_t cle_err, uint16_t tr_err)
{
	eip_req_t eip_req = sw->req;
	struct eip_ctx *ctx = tr->ctx;
	eip_tr_err_callback_t cb = tr->crypto.dec.err_cb;
	void *app_data = tr->crypto.dec.app_data;
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
 * eip_tr_skcipher_enc()
 *	Encrypt API.
 */
int eip_tr_skcipher_enc(struct eip_tr *tr, struct skcipher_request *req)
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
	 * Fill token for encryption and hmac.
	 */
	tk_words = eip_tr_fill_token(tk, &tr->crypto.enc, &tk_params);

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
	sw->comp = &eip_tr_skcipher_enc_done;
	sw->err_comp = &eip_tr_skcipher_enc_err;
	sw->tk_hdr = tk_params.tk_hdr;
	sw->tk_addr = virt_to_phys(tk);
	sw->tr_addr_type = tr->tr_addr_type;
	sw->tk_words = tk_words;
	sw->hw_svc = EIP_HW_CMD_HWSERVICE_LAC;
	sw->req = req;

	dma = &ctx->dma[smp_processor_id()];

	status = eip_dma_tx_sg(dma, sw, req->src, req->dst);
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
EXPORT_SYMBOL(eip_tr_skcipher_enc);

/*
 * eip_tr_skcipher_dec()
 *	Decryption API.
 */
int eip_tr_skcipher_dec(struct eip_tr *tr, struct skcipher_request *req)
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
	tk_words = eip_tr_fill_token(tk, &tr->crypto.dec, &tk_params);

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
	sw->comp = &eip_tr_skcipher_dec_done;
	sw->err_comp = &eip_tr_skcipher_dec_err;
	sw->tk_hdr = tk_params.tk_hdr;
	sw->tk_addr = virt_to_phys(tk);
	sw->tr_addr_type = tr->tr_addr_type;
	sw->tk_words = tk_words;
	sw->hw_svc = EIP_HW_CMD_HWSERVICE_LAC;
	sw->req = req;

	dma = &ctx->dma[smp_processor_id()];

	status = eip_dma_tx_sg(dma, sw, req->src, req->dst);
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
EXPORT_SYMBOL(eip_tr_skcipher_dec);
