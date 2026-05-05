/*
 * Copyright (c) 2022-2023, Qualcomm Innovation Center, Inc. All rights reserved.
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
static bool eip_tr_ipsec_aes_cbc_init(struct eip_tr *tr, struct eip_tr_info *info, const struct eip_svc_entry *algo);
static bool eip_tr_ipsec_des_cbc_init(struct eip_tr *tr, struct eip_tr_info *info, const struct eip_svc_entry *algo);
static bool eip_tr_ipsec_aes_gcm_init(struct eip_tr *tr, struct eip_tr_info *info, const struct eip_svc_entry *algo);

/*
 * Global constant algorith information.
 */
static const struct eip_svc_entry ipsec_algo_list[] = {
	{
		.name = "eip-aes-cbc-sha1-hmac",
		.enc_tk_fill = &eip_tk_ipsec_encauth_cbc,
		.dec_tk_fill = &eip_tk_ipsec_authdec_cbc,
		.tr_init = eip_tr_ipsec_aes_cbc_init,
		.auth_digest_len = SHA1_DIGEST_SIZE,
		.auth_block_len = SHA1_BLOCK_SIZE,
		.auth_state_len = EIP_HW_PAD_KEYSZ_SHA1,
		.ctrl_words_0 = EIP_HW_CTX_WITH_KEY | EIP_HW_CTX_ALGO_SHA1 |
			EIP_HW_CTX_AUTH_MODE_HMAC | EIP_TR_IPSEC_SPI | EIP_TR_IPSEC_SEQ_NUM,
		.ctrl_words_1 = EIP_HW_CTX_CIPHER_MODE_CBC,
		.iv_len = AES_BLOCK_SIZE,
		.cipher_blk_len = AES_BLOCK_SIZE
	},
	{
		.name = "eip-aes-cbc-md5-hmac",
		.enc_tk_fill = &eip_tk_ipsec_encauth_cbc,
		.dec_tk_fill = &eip_tk_ipsec_authdec_cbc,
		.tr_init = eip_tr_ipsec_aes_cbc_init,
		.auth_digest_len = MD5_DIGEST_SIZE,
		.auth_block_len = MD5_HMAC_BLOCK_SIZE,
		.auth_state_len = EIP_HW_PAD_KEYSZ_MD5,
		.ctrl_words_0 = EIP_HW_CTX_WITH_KEY | EIP_HW_CTX_ALGO_MD5 |
			EIP_HW_CTX_AUTH_MODE_HMAC | EIP_TR_IPSEC_SPI | EIP_TR_IPSEC_SEQ_NUM,
		.ctrl_words_1 = EIP_HW_CTX_CIPHER_MODE_CBC,
		.iv_len = AES_BLOCK_SIZE,
		.cipher_blk_len = AES_BLOCK_SIZE
	},
	{
		.name = "eip-aes-cbc-sha256-hmac",
		.enc_tk_fill = &eip_tk_ipsec_encauth_cbc,
		.dec_tk_fill = &eip_tk_ipsec_authdec_cbc,
		.tr_init = eip_tr_ipsec_aes_cbc_init,
		.auth_digest_len = SHA256_DIGEST_SIZE,
		.auth_block_len = SHA256_BLOCK_SIZE,
		.auth_state_len = EIP_HW_PAD_KEYSZ_SHA256,
		.ctrl_words_0 = EIP_HW_CTX_WITH_KEY | EIP_HW_CTX_ALGO_SHA256 |
			EIP_HW_CTX_AUTH_MODE_HMAC | EIP_TR_IPSEC_SPI | EIP_TR_IPSEC_SEQ_NUM,
		.ctrl_words_1 = EIP_HW_CTX_CIPHER_MODE_CBC,
		.iv_len = AES_BLOCK_SIZE,
		.cipher_blk_len = AES_BLOCK_SIZE
	},
	{
		.name = "eip-3des-cbc-sha1-hmac",
		.enc_tk_fill = &eip_tk_ipsec_encauth_cbc,
		.dec_tk_fill = &eip_tk_ipsec_authdec_cbc,
		.tr_init = eip_tr_ipsec_des_cbc_init,
		.auth_digest_len = SHA1_DIGEST_SIZE,
		.auth_block_len = SHA1_BLOCK_SIZE,
		.auth_state_len = EIP_HW_PAD_KEYSZ_SHA1,
		.ctrl_words_0 = EIP_HW_CTX_WITH_KEY | EIP_HW_CTX_ALGO_SHA1 |
			EIP_HW_CTX_AUTH_MODE_HMAC | EIP_TR_IPSEC_SPI | EIP_TR_IPSEC_SEQ_NUM,
		.ctrl_words_1 = EIP_HW_CTX_CIPHER_MODE_CBC,
		.iv_len = DES3_EDE_BLOCK_SIZE,
		.cipher_blk_len = DES3_EDE_BLOCK_SIZE
	},
	{
		.name = "eip-3des-cbc-md5-hmac",
		.enc_tk_fill = &eip_tk_ipsec_encauth_cbc,
		.dec_tk_fill = &eip_tk_ipsec_authdec_cbc,
		.tr_init = eip_tr_ipsec_des_cbc_init,
		.auth_digest_len = MD5_DIGEST_SIZE,
		.auth_block_len = MD5_HMAC_BLOCK_SIZE,
		.auth_state_len = EIP_HW_PAD_KEYSZ_MD5,
		.ctrl_words_0 = EIP_HW_CTX_WITH_KEY | EIP_HW_CTX_ALGO_MD5 |
			EIP_HW_CTX_AUTH_MODE_HMAC | EIP_TR_IPSEC_SPI | EIP_TR_IPSEC_SEQ_NUM,
		.ctrl_words_1 = EIP_HW_CTX_CIPHER_MODE_CBC,
		.iv_len = DES3_EDE_BLOCK_SIZE,
		.cipher_blk_len = DES3_EDE_BLOCK_SIZE
	},
	{
		.name = "eip-3des-cbc-sha256-hmac",
		.enc_tk_fill = &eip_tk_ipsec_encauth_cbc,
		.dec_tk_fill = &eip_tk_ipsec_authdec_cbc,
		.tr_init = eip_tr_ipsec_des_cbc_init,
		.auth_digest_len = SHA256_DIGEST_SIZE,
		.auth_block_len = SHA256_BLOCK_SIZE,
		.auth_state_len = EIP_HW_PAD_KEYSZ_SHA256,
		.ctrl_words_0 = EIP_HW_CTX_WITH_KEY | EIP_HW_CTX_ALGO_SHA256 |
			EIP_HW_CTX_AUTH_MODE_HMAC | EIP_TR_IPSEC_SPI | EIP_TR_IPSEC_SEQ_NUM,
		.ctrl_words_1 = EIP_HW_CTX_CIPHER_MODE_CBC,
		.iv_len = DES3_EDE_BLOCK_SIZE,
		.cipher_blk_len = DES3_EDE_BLOCK_SIZE
	},
	{
		.name = "eip-aes-gcm-rfc4106",
		.enc_tk_fill = &eip_tk_ipsec_encauth_gcm_rfc,
		.dec_tk_fill = &eip_tk_ipsec_authdec_gcm_rfc,
		.tr_init = eip_tr_ipsec_aes_gcm_init,
		.auth_digest_len = GHASH_DIGEST_SIZE,
		.auth_block_len = GHASH_BLOCK_SIZE,
		.auth_state_len = 0,
		.ctrl_words_0 = EIP_HW_CTX_WITH_KEY | EIP_HW_CTX_ALGO_GHASH |
			EIP_HW_CTX_AUTH_MODE_GMAC | EIP_TR_IPSEC_SPI | EIP_TR_IPSEC_SEQ_NUM,
		.ctrl_words_1 = EIP_HW_CTX_CIPHER_MODE_GCM | EIP_TR_IPSEC_CONTROL_IV(0x1) |
			EIP_TR_IPSEC_IV_FORMAT(0x1),
		.iv_len = 8,
		.cipher_blk_len = 4 /* IPsec ESP requires dword aligned */
	}
	/*
	 * TODO: Add support for null cipher
	 */
};

/*
 * eip_tr_ipsec_verify()
 *	IPsec specific validation
 */
static bool eip_tr_ipsec_verify(struct eip_tr *tr, struct eip_tr_info *info, const struct eip_svc_entry *algo)
{
	struct eip_tr_info_ipsec *ipsec = &info->ipsec;
	bool enc;

	enc = ipsec->sa_flags & EIP_TR_IPSEC_FLAG_ENC;
	if (!enc && (ipsec->sa_flags & EIP_TR_IPSEC_FLAG_ESN) && (ipsec->replay == EIP_IPSEC_REPLAY_NONE)) {
		pr_warn("%px: Anti-replay should be enabled with ESN.\n", tr);
		return false;
	}
	return true;
}

/*
 * eip_tr_ipsec_cmn_init()
 *	Initialize the transform record for ipsec.
 */
static void eip_tr_ipsec_cmn_init(struct eip_tr *tr, struct eip_tr_info *info, const struct eip_svc_entry *algo)
{
	struct eip_tr_info_ipsec *ipsec = &info->ipsec;
	uint32_t *tr_words = tr->hw_words;

	/*
	 * We first do common initialization and then algo specific initialization.
	 */
	if (ipsec->sa_flags & EIP_TR_IPSEC_FLAG_ENC) {
		eip_tr_ipsec_enc_cmn_init(tr, info, algo);
	} else {
		eip_tr_ipsec_dec_cmn_init(tr, info, algo);
	}

	/*
	 * We use large record for ipsec.
	 */
	tr->tr_addr_type = EIP_HW_CTX_TYPE_LARGE;
	tr->tr_addr_type |= virt_to_phys(tr->hw_words);
	tr->tr_flags = ipsec->sa_flags;
	tr->ctrl_words[0] = tr_words[0];
	tr->ctrl_words[1] = tr_words[1];
	tr->nonce = info->base.nonce;

	tr->ipsec.ops.cb = ipsec->cb;
	tr->ipsec.ops.err_cb = ipsec->err_cb;
	tr->ipsec.ops.app_data = ipsec->app_data;
}

/*
 * eip_tr_ipsec_aes_cbc_init()
 *	Initialize the transform record for ipsec.
 */
static bool eip_tr_ipsec_aes_cbc_init(struct eip_tr *tr, struct eip_tr_info *info, const struct eip_svc_entry *algo)
{
	struct eip_tr_base *crypto = &info->base;
	uint32_t *tr_words = tr->hw_words;

	/*
	 * We first do common initialization and then algo specific initialization.
	 */
	if (!eip_tr_ipsec_verify(tr, info, algo)) {
		pr_err("%px: TR verification failed.\n", tr);
		return false;
	}

	eip_tr_ipsec_cmn_init(tr, info, algo);

	switch(crypto->cipher.key_len) {
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
 * eip_tr_ipsec_des_cbc_init()
 *	Initialize the transform record for ipsec.
 */
static bool eip_tr_ipsec_des_cbc_init(struct eip_tr *tr, struct eip_tr_info *info, const struct eip_svc_entry *algo)
{
	struct eip_tr_base *crypto = &info->base;
	uint32_t *tr_words = tr->hw_words;

	/*
	 * We first do common initialization and then algo specific initialization.
	 */
	if (!eip_tr_ipsec_verify(tr, info, algo)) {
		pr_err("%px: TR verification failed.\n", tr);
		return false;
	}

	eip_tr_ipsec_cmn_init(tr, info, algo);

	switch(crypto->cipher.key_len) {
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
 * eip_tr_ipsec_aes_gcm_init()
 *	Initialize the transform record for ipsec.
 */
static bool eip_tr_ipsec_aes_gcm_init(struct eip_tr *tr, struct eip_tr_info *info, const struct eip_svc_entry *algo)
{
	struct eip_tr_base *base = &info->base;
	uint8_t authkey[AES_BLOCK_SIZE] = {0};
	uint32_t *tr_words = tr->hw_words;
	uint32_t *doffset = NULL;
	uint32_t *key = NULL;
	uint8_t auth_key_idx;
	int err = 0;

	/*
	 * We first do common initialization and then algo specific initialization.
	 */
	if (!eip_tr_ipsec_verify(tr, info, algo)) {
		pr_err("%px: TR verification failed.\n", tr);
		return false;
	}

	eip_tr_ipsec_cmn_init(tr, info, algo);

	switch(base->cipher.key_len) {
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
		pr_err("%px: Invalid key length(%u)\n", tr, base->cipher.key_len);
		return false;
	}

	/*
	 * Generate and fill ghash key.
	 * Here we are using software for generation of the keys
	 */
	err = eip_tr_genkey(tr, info, (uint8_t *)authkey, algo->auth_block_len);
	if (err) {
		pr_warn("Failed to generate authentication key for GCM with err <%x>\n", err);
		return false;
	}

	auth_key_idx = EIP_HW_CTRL_WORDS + (base->cipher.key_len / sizeof(uint32_t));
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
 * eip_tr_ipsec_get_svc()
 *	Get algo database for ipsec.
 */
const struct eip_svc_entry *eip_tr_ipsec_get_svc(void)
{
	return ipsec_algo_list;
}

/*
 * eip_tr_ipsec_get_svc_len()
 *	Get algo database length for ipsec.
 */
size_t eip_tr_ipsec_get_svc_len(void)
{
	return ARRAY_SIZE(ipsec_algo_list);
}
