/*
 * Copyright (c) 2023, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kref.h>
#include <linux/crypto.h>
#include <crypto/aead.h>
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
#include <crypto/ghash.h>
#include <crypto/gcm.h>

#include "eip_priv.h"

/*
 * Forward declaration for tr_init functions.
 */
static bool eip_tr_dtls_aes_cbc_init(struct eip_tr *tr, struct eip_tr_info *info, const struct eip_svc_entry *algo);
static bool eip_tr_dtls_des_cbc_init(struct eip_tr *tr, struct eip_tr_info *info, const struct eip_svc_entry *algo);
static bool eip_tr_dtls_aes_gcm_init(struct eip_tr *tr, struct eip_tr_info *info, const struct eip_svc_entry *algo);

/*
 * Global constant algorithm information.
 */
static const struct eip_svc_entry dtls_algo_list[] = {
	{
		.name = "eip-aes-cbc-sha1-hmac",
		.enc_tk_fill = &eip_tk_dtls_encauth_cbc,
		.dec_tk_fill = &eip_tk_dtls_authdec_cbc,
		.tr_init = eip_tr_dtls_aes_cbc_init,
		.auth_digest_len = SHA1_DIGEST_SIZE,
		.auth_block_len = SHA1_BLOCK_SIZE,
		.auth_state_len = EIP_HW_PAD_KEYSZ_SHA1,
		.ctrl_words_0 = EIP_HW_CTX_WITH_KEY | EIP_HW_CTX_ALGO_SHA1 |
			EIP_HW_CTX_AUTH_MODE_HMAC | EIP_TR_IPSEC_SPI | EIP_TR_DTLS_CTX0_SEQNO,
		.ctrl_words_1 = EIP_HW_CTX_CIPHER_MODE_CBC | EIP_TR_DTLS_CTX1_PAD_TLS | EIP_TR_CTRL_SEQ_NUM_STORE,
		.iv_len = AES_BLOCK_SIZE,
		.cipher_blk_len = AES_BLOCK_SIZE
	},
	{
		.name = "eip-aes-cbc-sha256-hmac",
		.enc_tk_fill = &eip_tk_dtls_encauth_cbc,
		.dec_tk_fill = &eip_tk_dtls_authdec_cbc,
		.tr_init = eip_tr_dtls_aes_cbc_init,
		.auth_digest_len = SHA256_DIGEST_SIZE,
		.auth_block_len = SHA256_BLOCK_SIZE,
		.auth_state_len = EIP_HW_PAD_KEYSZ_SHA256,
		.ctrl_words_0 = EIP_HW_CTX_WITH_KEY | EIP_HW_CTX_ALGO_SHA256 |
			EIP_HW_CTX_AUTH_MODE_HMAC | EIP_TR_IPSEC_SPI | EIP_TR_DTLS_CTX0_SEQNO,
		.ctrl_words_1 = EIP_HW_CTX_CIPHER_MODE_CBC | EIP_TR_DTLS_CTX1_PAD_TLS | EIP_TR_CTRL_SEQ_NUM_STORE,
		.iv_len = AES_BLOCK_SIZE,
		.cipher_blk_len = AES_BLOCK_SIZE
	},
	{
		.name = "eip-3des-cbc-sha1-hmac",
		.enc_tk_fill = &eip_tk_dtls_encauth_cbc,
		.dec_tk_fill = &eip_tk_dtls_authdec_cbc,
		.tr_init = eip_tr_dtls_des_cbc_init,
		.auth_digest_len = SHA1_DIGEST_SIZE,
		.auth_block_len = SHA1_BLOCK_SIZE,
		.auth_state_len = EIP_HW_PAD_KEYSZ_SHA1,
		.ctrl_words_0 = EIP_HW_CTX_WITH_KEY | EIP_HW_CTX_ALGO_SHA1 |
			EIP_HW_CTX_AUTH_MODE_HMAC | EIP_TR_IPSEC_SPI | EIP_TR_DTLS_CTX0_SEQNO,
		.ctrl_words_1 = EIP_HW_CTX_CIPHER_MODE_CBC | EIP_TR_DTLS_CTX1_PAD_TLS | EIP_TR_CTRL_SEQ_NUM_STORE,
		.iv_len = DES3_EDE_BLOCK_SIZE,
		.cipher_blk_len = DES3_EDE_BLOCK_SIZE
	},
	{
		.name = "eip-3des-cbc-sha256-hmac",
		.enc_tk_fill = &eip_tk_dtls_encauth_cbc,
		.dec_tk_fill = &eip_tk_dtls_authdec_cbc,
		.tr_init = eip_tr_dtls_des_cbc_init,
		.auth_digest_len = SHA256_DIGEST_SIZE,
		.auth_block_len = SHA256_BLOCK_SIZE,
		.auth_state_len = EIP_HW_PAD_KEYSZ_SHA256,
		.ctrl_words_0 = EIP_HW_CTX_WITH_KEY | EIP_HW_CTX_ALGO_SHA256 |
			EIP_HW_CTX_AUTH_MODE_HMAC | EIP_TR_IPSEC_SPI | EIP_TR_DTLS_CTX0_SEQNO,
		.ctrl_words_1 = EIP_HW_CTX_CIPHER_MODE_CBC | EIP_TR_DTLS_CTX1_PAD_TLS | EIP_TR_CTRL_SEQ_NUM_STORE,
		.iv_len = DES3_EDE_BLOCK_SIZE,
		.cipher_blk_len = DES3_EDE_BLOCK_SIZE
	},
	{
		.name = "eip-aes-gcm",
		.enc_tk_fill = &eip_tk_dtls_encauth_gcm,
		.dec_tk_fill = &eip_tk_dtls_authdec_gcm,
		.tr_init = eip_tr_dtls_aes_gcm_init,
		.auth_digest_len = GHASH_DIGEST_SIZE,
		.auth_block_len = GHASH_BLOCK_SIZE,
		.auth_state_len = 0,
		.ctrl_words_0 = EIP_HW_CTX_WITH_KEY | EIP_HW_CTX_ALGO_GHASH |
			EIP_HW_CTX_AUTH_MODE_GMAC | EIP_TR_IPSEC_SPI | EIP_TR_DTLS_CTX0_SEQNO,
		.ctrl_words_1 = EIP_HW_CTX_CIPHER_MODE_GCM | EIP_TR_CTRL_SEQ_NUM_STORE |
			EIP_TR_DTLS_CONTROL_IV(1) | EIP_TR_DTLS_IV_FORMAT(1),	/* GCM uses nonce and CTR mode */
		.iv_len = 8,
		.cipher_blk_len = 1 /* GCM doesn't require block alignment */
	}
};

/*
 * eip_tr_dtls_cmn_init()
 *	Initialize the transform record for DTLS.
 */
static void eip_tr_dtls_cmn_init(struct eip_tr *tr, struct eip_tr_info *info, const struct eip_svc_entry *algo)
{
	struct eip_tr_info_dtls *dtls = &info->dtls;
	uint32_t *tr_words = tr->hw_words;

	if (dtls->flags & EIP_TR_DTLS_FLAG_ENC)
		eip_tr_dtls_enc_cmn_init(tr, info, algo);
	else
		eip_tr_dtls_dec_cmn_init(tr, info, algo);

	/*
	 * We use large record for dtls.
	 */
	tr->tr_addr_type = EIP_HW_CTX_TYPE_LARGE;
	tr->tr_addr_type |= virt_to_phys(tr->hw_words);
	tr->tr_flags = dtls->flags;
	tr->ctrl_words[0] = tr_words[0];
	tr->ctrl_words[1] = tr_words[1];
	tr->nonce = info->base.nonce;

	tr->dtls.ops.cb = dtls->cb;
	tr->dtls.ops.err_cb = dtls->err_cb;
	tr->dtls.ops.app_data = dtls->app_data;
}

/*
 * eip_tr_dtls_aes_cbc_init()
 *	Initialize the transform record for DTLS/AES-CBC.
 */
static bool eip_tr_dtls_aes_cbc_init(struct eip_tr *tr, struct eip_tr_info *info, const struct eip_svc_entry *algo)
{
	struct eip_tr_base *crypto = &info->base;
	uint32_t *tr_words = tr->hw_words;

	/*
	 * We first do common initialization and then algo specific initialization.
	 */
	eip_tr_dtls_cmn_init(tr, info, algo);

	switch (crypto->cipher.key_len) {
	case AES_KEYSIZE_128:
		tr_words[0] |= EIP_HW_CTX_ALGO_AES128;
		tr->ctrl_words[0] |= EIP_HW_CTX_ALGO_AES128;
		break;
	case AES_KEYSIZE_192:
		tr_words[0] |= EIP_HW_CTX_ALGO_AES192;
		tr->ctrl_words[0] |= EIP_HW_CTX_ALGO_AES192;
		break;
	case AES_KEYSIZE_256:
		tr_words[0] |= EIP_HW_CTX_ALGO_AES256;
		tr->ctrl_words[0] |= EIP_HW_CTX_ALGO_AES256;
		break;
	default:
		pr_err("%px: Invalid key length(%u)\n", tr, crypto->cipher.key_len);
		return false;
	}

	/*
	 * Flush record memory.
	 * Driver must not make any update in transform records words after this.
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
 * eip_tr_dtls_des_cbc_init()
 *	Initialize the transform record for DTLS/AES-CBC.
 */
static bool eip_tr_dtls_des_cbc_init(struct eip_tr *tr, struct eip_tr_info *info, const struct eip_svc_entry *algo)
{
	struct eip_tr_base *crypto = &info->base;
	uint32_t *tr_words = tr->hw_words;

	/*
	 * We first do common initialization and then algo specific initialization.
	 */
	eip_tr_dtls_cmn_init(tr, info, algo);

	switch (crypto->cipher.key_len) {
	case DES3_EDE_KEY_SIZE:
		tr_words[0] |= EIP_HW_CTX_ALGO_3DES;
		tr->ctrl_words[0] |= EIP_HW_CTX_ALGO_3DES;
		break;
	default:
		pr_err("%px: Invalid key length(%u)\n", tr, crypto->cipher.key_len);
		return false;
	}

	/*
	 * Flush record memory.
	 * Driver must not make any update in transform records words after this.
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
 * eip_tr_dtls_aes_gcm_init()
 *	Initialize the transform record for DTLS/AES-GCM.
 */
static bool eip_tr_dtls_aes_gcm_init(struct eip_tr *tr, struct eip_tr_info *info, const struct eip_svc_entry *algo)
{
	struct eip_tr_info_dtls *dtls = &info->dtls;
	struct eip_tr_base *crypto = &info->base;
	uint8_t authkey[AES_BLOCK_SIZE] = {0};
	uint32_t *tr_words = tr->hw_words;
	uint32_t *doffset = NULL;
	uint32_t *key = NULL;
	uint8_t auth_key_idx;
	int err = 0;

	/*
	 * We first do common initialization and then algo specific initialization.
	 */
	eip_tr_dtls_cmn_init(tr, info, algo);

	/*
	 * Overide ToP, GCM uses Encrypt-Hash & hash-decrypt.
	 * Also, Decap doesn't require preprocessing.
	 */
	if (dtls->flags & EIP_TR_DTLS_FLAG_ENC) {
		tr_words[0] &= ~EIP_TK_CTRL_OP_MASK;
		tr_words[0] |= EIP_TK_CTRL_OP_ENC_HMAC;
	} else {
		tr_words[0] &= ~EIP_TK_CTRL_OP_MASK;
		tr_words[0] |= EIP_TK_CTRL_OP_HMAC_DEC;
		tr_words[1] &= ~EIP_TR_DTLS_CTX1_PRE_CRYPTO_DECAP;
	}

	switch (crypto->cipher.key_len) {
	case AES_KEYSIZE_128:
		tr_words[0] |= EIP_HW_CTX_ALGO_AES128;
		break;
	case AES_KEYSIZE_192:
		tr_words[0] |= EIP_HW_CTX_ALGO_AES192;
		break;
	case AES_KEYSIZE_256:
		tr_words[0] |= EIP_HW_CTX_ALGO_AES256;
		break;
	default:
		pr_err("%px: Invalid key length(%u)\n", tr, crypto->cipher.key_len);
		return false;
	}

	/*
	 * Reload cached control word.
	 */
	tr->ctrl_words[0] = tr_words[0];
	tr->ctrl_words[1] = tr_words[1];

	/*
	 * Generate and fill ghash key.
	 * Here we are using software for generation of the keys
	 */
	err = eip_tr_genkey(tr, info, (uint8_t *)authkey, algo->auth_block_len);
	if (err) {
		pr_warn("Failed to generate authentication key for GCM with err <%x>\n", err);
		return false;
	}

	auth_key_idx = EIP_HW_CTRL_WORDS + (crypto->cipher.key_len / sizeof(uint32_t));
	doffset = &tr_words[auth_key_idx];

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
 * eip_tr_dtls_get_svc()
 *	Get algo database for DTLS.
 */
const struct eip_svc_entry *eip_tr_dtls_get_svc(void)
{
	return dtls_algo_list;
}

/*
 * eip_tr_dtls_get_svc_len()
 *	Get algo database length for dtls.
 */
size_t eip_tr_dtls_get_svc_len(void)
{
	return ARRAY_SIZE(dtls_algo_list);
}
