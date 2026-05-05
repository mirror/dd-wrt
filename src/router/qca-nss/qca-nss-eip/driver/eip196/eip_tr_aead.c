/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: ISC
 */

#include <linux/version.h>
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
#include <crypto/gcm.h>
#include <crypto/ghash.h>

#include "eip_priv.h"

/*
 * Function prototypes.
 */
bool eip_tr_aead_init(struct eip_tr *tr, struct eip_tr_info *info, const struct eip_svc_entry *algo);
bool eip_tr_aead_gcm_init(struct eip_tr *tr, struct eip_tr_info *info, const struct eip_svc_entry *algo);

/*
 * Global constant algorith information.
 */
static const struct eip_svc_entry aead_algo_list[] = {
	{
		.name = "eip-aes-cbc-md5-hmac",
		.enc_tk_fill = &eip_tk_encauth_cbc,
		.dec_tk_fill = &eip_tk_authdec_cbc,
		.tr_init = eip_tr_aead_init,
		.auth_block_len = MD5_HMAC_BLOCK_SIZE,
		.auth_digest_len = MD5_DIGEST_SIZE,
		.auth_state_len = EIP_HW_PAD_KEYSZ_MD5,
		.ctrl_words_0 = EIP_HW_CTX_WITH_KEY | EIP_HW_CTX_ALGO_MD5 |
			EIP_HW_CTX_AUTH_MODE_HMAC,
		.ctrl_words_1 = EIP_HW_CTX_CIPHER_MODE_CBC,
	},
	{
		.name = "eip-aes-cbc-sha1-hmac",
		.enc_tk_fill = &eip_tk_encauth_cbc,
		.dec_tk_fill = &eip_tk_authdec_cbc,
		.tr_init = eip_tr_aead_init,
		.auth_digest_len = SHA1_DIGEST_SIZE,
		.auth_block_len = SHA1_BLOCK_SIZE,
		.auth_state_len = EIP_HW_PAD_KEYSZ_SHA1,
		.ctrl_words_0 = EIP_HW_CTX_WITH_KEY | EIP_HW_CTX_ALGO_SHA1 |
			EIP_HW_CTX_AUTH_MODE_HMAC,
		.ctrl_words_1 = EIP_HW_CTX_CIPHER_MODE_CBC,
	},
	{
		.name = "eip-aes-cbc-sha256-hmac",
		.enc_tk_fill = &eip_tk_encauth_cbc,
		.dec_tk_fill = &eip_tk_authdec_cbc,
		.tr_init = eip_tr_aead_init,
		.auth_block_len = SHA256_BLOCK_SIZE,
		.auth_digest_len = SHA256_DIGEST_SIZE,
		.auth_state_len = EIP_HW_PAD_KEYSZ_SHA256,
		.ctrl_words_0 = EIP_HW_CTX_WITH_KEY | EIP_HW_CTX_ALGO_SHA256 |
			EIP_HW_CTX_AUTH_MODE_HMAC,
		.ctrl_words_1 = EIP_HW_CTX_CIPHER_MODE_CBC,
	},
	{
		.name = "eip-aes-ctr-rfc3686-md5-hmac",
		.enc_tk_fill = &eip_tk_encauth_ctr_rfc,
		.dec_tk_fill = &eip_tk_authdec_ctr_rfc,
		.tr_init = eip_tr_aead_init,
		.auth_block_len = MD5_HMAC_BLOCK_SIZE,
		.auth_digest_len = MD5_DIGEST_SIZE,
		.auth_state_len = EIP_HW_PAD_KEYSZ_MD5,
		.ctrl_words_0 = EIP_HW_CTX_WITH_KEY | EIP_HW_CTX_ALGO_MD5 |
			EIP_HW_CTX_AUTH_MODE_HMAC,
		.ctrl_words_1 = EIP_HW_CTX_CIPHER_MODE_CTR,
	},
	{
		.name = "eip-aes-cbc-sha384-hmac",
		.enc_tk_fill = &eip_tk_encauth_cbc,
		.dec_tk_fill = &eip_tk_authdec_cbc,
		.tr_init = eip_tr_aead_init,
		.auth_block_len = SHA384_BLOCK_SIZE,
		.auth_digest_len = SHA384_DIGEST_SIZE,
		.auth_state_len = EIP_HW_PAD_KEYSZ_SHA384,
		.ctrl_words_0 = EIP_HW_CTX_WITH_KEY | EIP_HW_CTX_ALGO_SHA384 |
			EIP_HW_CTX_AUTH_MODE_HMAC,
		.ctrl_words_1 = EIP_HW_CTX_CIPHER_MODE_CBC,
	},
	{
		.name = "eip-aes-cbc-sha512-hmac",
		.enc_tk_fill = &eip_tk_encauth_cbc,
		.dec_tk_fill = &eip_tk_authdec_cbc,
		.tr_init = eip_tr_aead_init,
		.auth_block_len = SHA512_BLOCK_SIZE,
		.auth_digest_len = SHA512_DIGEST_SIZE,
		.auth_state_len = EIP_HW_PAD_KEYSZ_SHA512,
		.ctrl_words_0 = EIP_HW_CTX_WITH_KEY | EIP_HW_CTX_ALGO_SHA512 |
			EIP_HW_CTX_AUTH_MODE_HMAC,
		.ctrl_words_1 = EIP_HW_CTX_CIPHER_MODE_CBC,
	},
	{
		.name = "eip-aes-ctr-rfc3686-sha1-hmac",
		.enc_tk_fill = &eip_tk_encauth_ctr_rfc,
		.dec_tk_fill = &eip_tk_authdec_ctr_rfc,
		.tr_init = eip_tr_aead_init,
		.auth_block_len = SHA1_BLOCK_SIZE,
		.auth_digest_len = SHA1_DIGEST_SIZE,
		.auth_state_len = EIP_HW_PAD_KEYSZ_SHA1,
		.ctrl_words_0 = EIP_HW_CTX_WITH_KEY | EIP_HW_CTX_ALGO_SHA1 |
			EIP_HW_CTX_AUTH_MODE_HMAC,
		.ctrl_words_1 = EIP_HW_CTX_CIPHER_MODE_CTR,
	},
	{
		.name = "eip-aes-ctr-rfc3686-sha256-hmac",
		.enc_tk_fill = &eip_tk_encauth_ctr_rfc,
		.dec_tk_fill = &eip_tk_authdec_ctr_rfc,
		.tr_init = eip_tr_aead_init,
		.auth_block_len = SHA256_BLOCK_SIZE,
		.auth_digest_len = SHA256_DIGEST_SIZE,
		.auth_state_len = EIP_HW_PAD_KEYSZ_SHA256,
		.ctrl_words_0 = EIP_HW_CTX_WITH_KEY | EIP_HW_CTX_ALGO_SHA256 |
			EIP_HW_CTX_AUTH_MODE_HMAC,
		.ctrl_words_1 = EIP_HW_CTX_CIPHER_MODE_CTR,
	},
	{
		.name = "eip-aes-ctr-rfc3686-sha384-hmac",
		.enc_tk_fill = &eip_tk_encauth_ctr_rfc,
		.dec_tk_fill = &eip_tk_authdec_ctr_rfc,
		.tr_init = eip_tr_aead_init,
		.auth_block_len = SHA384_BLOCK_SIZE,
		.auth_digest_len = SHA384_DIGEST_SIZE,
		.auth_state_len = EIP_HW_PAD_KEYSZ_SHA384,
		.ctrl_words_0 = EIP_HW_CTX_WITH_KEY | EIP_HW_CTX_ALGO_SHA384 |
			EIP_HW_CTX_AUTH_MODE_HMAC,
		.ctrl_words_1 = EIP_HW_CTX_CIPHER_MODE_CTR,
	},
	{
		.name = "eip-aes-ctr-rfc3686-sha512-hmac",
		.enc_tk_fill = &eip_tk_encauth_ctr_rfc,
		.dec_tk_fill = &eip_tk_authdec_ctr_rfc,
		.tr_init = eip_tr_aead_init,
		.auth_block_len = SHA512_BLOCK_SIZE,
		.auth_digest_len = SHA512_DIGEST_SIZE,
		.auth_state_len = EIP_HW_PAD_KEYSZ_SHA512,
		.ctrl_words_0 = EIP_HW_CTX_WITH_KEY | EIP_HW_CTX_ALGO_SHA512 |
			EIP_HW_CTX_AUTH_MODE_HMAC,
		.ctrl_words_1 = EIP_HW_CTX_CIPHER_MODE_CTR,
	},
	{
		.name = "eip-aes-gcm",
		.enc_tk_fill = &eip_tk_encauth_gcm,
		.dec_tk_fill = &eip_tk_authdec_gcm,
		.tr_init = eip_tr_aead_gcm_init,
		.auth_block_len = GHASH_BLOCK_SIZE,
		.auth_digest_len = GHASH_DIGEST_SIZE,
		.auth_state_len = 0,
		.ctrl_words_0 = EIP_HW_CTX_WITH_KEY | EIP_HW_CTX_ALGO_GHASH |
			EIP_HW_CTX_AUTH_MODE_GMAC,
		.ctrl_words_1 = EIP_HW_CTX_CIPHER_MODE_GCM,
	},
	{
		.name = "eip-aes-gcm-rfc4106",
		.enc_tk_fill = &eip_tk_encauth_gcm_rfc,
		.dec_tk_fill = &eip_tk_authdec_gcm_rfc,
		.tr_init = eip_tr_aead_gcm_init,
		.auth_block_len = GHASH_BLOCK_SIZE,
		.auth_digest_len = GHASH_DIGEST_SIZE,
		.auth_state_len = 0,
		.ctrl_words_0 = EIP_HW_CTX_WITH_KEY | EIP_HW_CTX_ALGO_GHASH |
			EIP_HW_CTX_AUTH_MODE_GMAC,
		.ctrl_words_1 = EIP_HW_CTX_CIPHER_MODE_GCM,
	},
	{
		.name = "eip-3des-cbc-md5-hmac",
		.enc_tk_fill = &eip_tk_encauth_cbc,
		.dec_tk_fill = &eip_tk_authdec_cbc,
		.tr_init = eip_tr_aead_init,
		.auth_block_len = MD5_HMAC_BLOCK_SIZE,
		.auth_digest_len = MD5_DIGEST_SIZE,
		.auth_state_len = EIP_HW_PAD_KEYSZ_MD5,
		.ctrl_words_0 = EIP_HW_CTX_WITH_KEY | EIP_HW_CTX_ALGO_MD5 |
			EIP_HW_CTX_AUTH_MODE_HMAC | EIP_HW_CTX_ALGO_3DES,
		.ctrl_words_1 = EIP_HW_CTX_CIPHER_MODE_CBC,
	},
	{
		.name = "eip-3des-cbc-sha1-hmac",
		.enc_tk_fill = &eip_tk_encauth_cbc,
		.dec_tk_fill = &eip_tk_authdec_cbc,
		.tr_init = eip_tr_aead_init,
		.auth_block_len = SHA1_BLOCK_SIZE,
		.auth_digest_len = SHA1_DIGEST_SIZE,
		.auth_state_len = EIP_HW_PAD_KEYSZ_SHA1,
		.ctrl_words_0 = EIP_HW_CTX_WITH_KEY | EIP_HW_CTX_ALGO_SHA1 |
			EIP_HW_CTX_AUTH_MODE_HMAC | EIP_HW_CTX_ALGO_3DES,
		.ctrl_words_1 = EIP_HW_CTX_CIPHER_MODE_CBC,
	},
	{
		.name = "eip-3des-cbc-sha256-hmac",
		.enc_tk_fill = &eip_tk_encauth_cbc,
		.dec_tk_fill = &eip_tk_authdec_cbc,
		.tr_init = eip_tr_aead_init,
		.auth_block_len = SHA256_BLOCK_SIZE,
		.auth_digest_len = SHA256_DIGEST_SIZE,
		.auth_state_len = EIP_HW_PAD_KEYSZ_SHA256,
		.ctrl_words_0 = EIP_HW_CTX_WITH_KEY | EIP_HW_CTX_ALGO_SHA256 |
			EIP_HW_CTX_AUTH_MODE_HMAC | EIP_HW_CTX_ALGO_3DES,
		.ctrl_words_1 = EIP_HW_CTX_CIPHER_MODE_CBC,
	},
};

/*
 * eip_tr_aead_tx_complete()
 *	Free Resources held during transmit.
 *
 * TR should not be used after calling this function.
 */
static inline void eip_tr_aead_tx_complete(struct eip_tr *tr, struct eip_sw_desc *sw)
{
	struct eip_ctx *ctx = tr->ctx;

	/*
	 * Free token and sw_cache.
	 */
	kmem_cache_free(ctx->tk_cache, sw->tk);
	kmem_cache_free(ctx->sw_cache, sw);

	/*
	 * Reference: eip_tr_aead_encauth().
	 */
	eip_tr_deref(tr);
}

/*
 * eip_tr_aead_enc_err()
 *	Hardware callback API for error in encryption.
 */
static void eip_tr_aead_enc_err(struct eip_tr *tr, struct eip_hw_desc *hw, struct eip_sw_desc *sw, uint16_t cle_err, uint16_t tr_err)
{
	eip_tr_err_callback_t cb = tr->crypto.enc.err_cb;
	void *app_data = tr->crypto.enc.app_data;
	eip_req_t eip_req = sw->req;
	int err = 0;

	/*
	 * Free transmit resources.
	 */
	eip_tr_aead_tx_complete(tr, sw);

	/*
	 * Classify hw errors and convert to linux errors.
	 */
	err = eip_tr_classify_err(cle_err, tr_err);

	/*
	 * Call the client callback.
	 */
	cb(app_data, eip_req, err);
}

/*
 * eip_tr_aead_dec_err()
 *	Hardware callback API for error in decryption.
 */
static void eip_tr_aead_dec_err(struct eip_tr *tr, struct eip_hw_desc *hw, struct eip_sw_desc *sw, uint16_t cle_err, uint16_t tr_err)
{
	eip_tr_err_callback_t cb = tr->crypto.dec.err_cb;
	void *app_data = tr->crypto.dec.app_data;
	eip_req_t eip_req = sw->req;
	int err = 0;

	/*
	 * Free transmit resources.
	 */
	eip_tr_aead_tx_complete(tr, sw);

	/*
	 * Classify hw errors and convert to linux errors.
	 */
	err = eip_tr_classify_err(cle_err, tr_err);

	/*
	 * Call the client callback.
	 */
	cb(app_data, eip_req, err);
}

/*
 * eip_tr_aead_enc_done()
 *	Hardware callback API for encryption.
 */
static void eip_tr_aead_enc_done(struct eip_tr *tr, struct eip_hw_desc *hw, struct eip_sw_desc *sw)
{
	eip_tr_callback_t cb = tr->crypto.enc.cb;
	void *app_data = tr->crypto.enc.app_data;
	eip_req_t eip_req = sw->req;

	/*
	 * Free transmit resources.
	 */
	eip_tr_aead_tx_complete(tr, sw);

	/*
	 * Call the client callback.
	 */
	cb(app_data, eip_req);
}

/*
 * eip_tr_aead_dec_done()
 *	Hardware callback API for Decryption.
 */
static void eip_tr_aead_dec_done(struct eip_tr *tr, struct eip_hw_desc *hw, struct eip_sw_desc *sw)
{
	eip_tr_callback_t cb = tr->crypto.dec.cb;
	void *app_data = tr->crypto.dec.app_data;
	eip_req_t eip_req = sw->req;

	/*
	 * Free transmit resources.
	 */
	eip_tr_aead_tx_complete(tr, sw);

	/*
	 * Call the client callback.
	 */
	cb(app_data, eip_req);
}

/*
 * eip_tr_aead_init()
 *	Initialize the transform record for Crypto AEAD.
 */
bool eip_tr_aead_init(struct eip_tr *tr, struct eip_tr_info *info, const struct eip_svc_entry *algo)
{
	struct eip_tr_info_crypto *crypto = &info->crypto;
	struct eip_tr_base *base = &info->base;
	uint32_t *tr_words = tr->hw_words;
	uint8_t ctx_sz;
	uint32_t size;

	/*
	 * We use large record for aead.
	 */
	tr->tr_addr_type = EIP_HW_CTX_TYPE_LARGE;
	tr->tr_addr_type |= virt_to_phys(tr->hw_words);
	tr->digest_len = algo->auth_digest_len;
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
	 * Leave the space for inner & outer digest.
	 */
	size = algo->auth_state_len;
	tr_words += (size / sizeof(uint32_t));
	tr_words += (size / sizeof(uint32_t));

	/*
	 * Fill control words.
	 */
	tr->ctrl_words[0] = algo->ctrl_words_0;
	tr->ctrl_words[1] = algo->ctrl_words_1;
	ctx_sz = tr_words - tr->hw_words - EIP_HW_MAX_CTRL;

	if (tr->ctrl_words[0] & EIP_HW_CTX_ALGO_3DES) {
		tr->ctrl_words[0] |= EIP_TR_CTRL_CONTEXT_WORDS(ctx_sz);
		goto done;
	}

	switch(base->cipher.key_len) {
	case AES_KEYSIZE_128:
		tr->ctrl_words[0] |= (EIP_HW_CTX_ALGO_AES128 | EIP_TR_CTRL_CONTEXT_WORDS(ctx_sz));
		break;
	case AES_KEYSIZE_192:
		tr->ctrl_words[0] |= (EIP_HW_CTX_ALGO_AES192 | EIP_TR_CTRL_CONTEXT_WORDS(ctx_sz));
		break;
	case AES_KEYSIZE_256:
		tr->ctrl_words[0] |= (EIP_HW_CTX_ALGO_AES256 | EIP_TR_CTRL_CONTEXT_WORDS(ctx_sz));
		break;
	default:
		pr_err("%px: Invalid key length(%u)\n", tr, base->cipher.key_len);
		return false;
	}

done:
	/*
	 * Flush record memory.
	 * Driver must not make any update in transform records words after this.
	 * EIP196 will do ipad/opad update.
	 */
	eip_dmac_clean_range(tr->hw_words, tr->hw_words + EIP_HW_CTX_SIZE_LARGE_WORDS);

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
 * eip_tr_aead_gcm_init()
 *	Initialize the transform record for GCM AEAD.
 */
bool eip_tr_aead_gcm_init(struct eip_tr *tr, struct eip_tr_info *info, const struct eip_svc_entry *algo)
{
	struct eip_tr_info_crypto *crypto = &info->crypto;
	struct eip_tr_base *base = &info->base;
	uint8_t authkey[AES_BLOCK_SIZE] = {0};
	uint32_t *tr_words = tr->hw_words;
	uint32_t *doffset = NULL;
	uint32_t *key = NULL;
	uint8_t ctx_sz;
	int err = 0;

	/*
	 * We use large record for aead.
	 */
	tr->tr_addr_type = EIP_HW_CTX_TYPE_LARGE;
	tr->tr_addr_type |= virt_to_phys(tr->hw_words);
	tr->digest_len = algo->auth_digest_len;
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
	 * Leave space for ghash key
	 */
	doffset = tr_words;
	tr_words += (algo->auth_block_len / sizeof(uint32_t));

	/*
	 * Fill control words.
	 */
	tr->ctrl_words[0] = algo->ctrl_words_0;
	tr->ctrl_words[1] = algo->ctrl_words_1;
	ctx_sz = tr_words - tr->hw_words - EIP_HW_MAX_CTRL;

	switch (base->cipher.key_len) {
	case AES_KEYSIZE_128:
		tr->ctrl_words[0] |= (EIP_HW_CTX_ALGO_AES128 | EIP_TR_CTRL_CONTEXT_WORDS(ctx_sz));
		break;
	case AES_KEYSIZE_192:
		tr->ctrl_words[0] |= (EIP_HW_CTX_ALGO_AES192 | EIP_TR_CTRL_CONTEXT_WORDS(ctx_sz));
		break;
	case AES_KEYSIZE_256:
		tr->ctrl_words[0] |= (EIP_HW_CTX_ALGO_AES256 | EIP_TR_CTRL_CONTEXT_WORDS(ctx_sz));
		break;
	default:
		pr_err("%px: Invalid key length(%u)\n", tr, base->cipher.key_len);
		return false;
	}

	/*
	 * Generate and fill ghash key
	 * Here we are using software for generation of the keys
	 */
	err = eip_tr_genkey(tr, info, (uint8_t *)authkey, algo->auth_block_len);
	if (err) {
		pr_warn("Failed to generate authentication key for GCM with err <%x>\n", err);
		return false;
	}

	key = (uint32_t *)authkey;
	doffset[0] = ntohl(key[0]);
	doffset[1] = ntohl(key[1]);
	doffset[2] = ntohl(key[2]);
	doffset[3] = ntohl(key[3]);

	/*
	 * Flush record memory.
	 * Driver must not make any update in transform records words after this.
	 */
	eip_dmac_clean_range(tr->hw_words, tr->hw_words + EIP_HW_CTX_SIZE_LARGE_WORDS);

	return true;
}

/*
 * eip_tr_aead_get_svc()
 *	Get algo database for aead.
 */
const struct eip_svc_entry *eip_tr_aead_get_svc(void)
{
	return aead_algo_list;
}

/*
 * eip_tr_aead_get_svc_len()
 *	Get algo database length for aead.
 */
size_t eip_tr_aead_get_svc_len(void)
{
	return ARRAY_SIZE(aead_algo_list);
}

/*
 * eip_tr_aead_encauth()
 *	Encryption followed by authentication.
 */
int eip_tr_aead_encauth(struct eip_tr *tr, struct aead_request *req)
{
	struct eip_tk_params tk_params = {0};
	struct eip_ctx *ctx = tr->ctx;
	struct eip_sw_desc *sw;
	struct eip_dma *dma;
	struct eip_tk *tk;
	uint32_t tk_words;
	int status = 0;

	/*
	 * Fill TK params
	 */
	tk_params.tr = tr;
	tk_params.eip_req = req;

	/*
	 * Allocate HW token.
	 */
	tk = kmem_cache_alloc(ctx->tk_cache, GFP_NOWAIT | __GFP_NOWARN);
	if (!tk) {
		pr_err("%px: Failed to allocate token.\n", tr);
		return -ENOMEM;
	}

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
	sw->comp = &eip_tr_aead_enc_done;
	sw->err_comp = &eip_tr_aead_enc_err;
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
EXPORT_SYMBOL(eip_tr_aead_encauth);

/*
 * eip_tr_aead_authdec()
 *	Authentication followed by decryption.
 */
int eip_tr_aead_authdec(struct eip_tr *tr, struct aead_request *req)
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
	sw->comp = &eip_tr_aead_dec_done;
	sw->err_comp = &eip_tr_aead_dec_err;
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
EXPORT_SYMBOL(eip_tr_aead_authdec);
