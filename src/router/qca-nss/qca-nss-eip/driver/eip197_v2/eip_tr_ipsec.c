/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: ISC
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
static bool eip_tr_ipsec_null_hmac_init(struct eip_tr *tr, struct eip_tr_info *info, const struct eip_svc_entry *algo);

/*
 * Global constant algorith information.
 */
static const struct eip_svc_entry ipsec_algo_list[] = {
	{
		.name = "eip-aes-cbc-sha1-hmac",
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
	},
	{
		.name = "eip-cbc-null-sha1-hmac",
		.tr_init = eip_tr_ipsec_null_hmac_init,
		.auth_digest_len = SHA1_DIGEST_SIZE,
		.auth_block_len = SHA1_BLOCK_SIZE,
		.auth_state_len = EIP_HW_PAD_KEYSZ_SHA1,
		.ctrl_words_0 = EIP_HW_CTX_WITH_KEY | EIP_HW_CTX_ALGO_SHA1 |
			EIP_HW_CTX_AUTH_MODE_HMAC | EIP_TR_IPSEC_SPI | EIP_TR_IPSEC_SEQ_NUM,
		.ctrl_words_1 = EIP_HW_CTX_CIPHER_MODE_ECB,
		.iv_len = 0,
		.cipher_blk_len = 4
	},
	{
		.name = "eip-cbc-null-sha256-hmac",
		.tr_init = eip_tr_ipsec_null_hmac_init,
		.auth_digest_len = SHA256_DIGEST_SIZE,
		.auth_block_len = SHA256_BLOCK_SIZE,
		.auth_state_len = EIP_HW_PAD_KEYSZ_SHA256,
		.ctrl_words_0 = EIP_HW_CTX_WITH_KEY | EIP_HW_CTX_ALGO_SHA256 |
			EIP_HW_CTX_AUTH_MODE_HMAC | EIP_TR_IPSEC_SPI | EIP_TR_IPSEC_SEQ_NUM,
		.ctrl_words_1 = EIP_HW_CTX_CIPHER_MODE_ECB,
		.iv_len = 0,
		.cipher_blk_len = 4
	}
};

/*
 * eip_tr_ipsec_get_ohdr()
 */
uint8_t eip_tr_ipsec_get_ohdr(uint32_t sa_flags)
{
	uint32_t val = sa_flags & (EIP_TR_IPSEC_FLAG_ENC | EIP_TR_IPSEC_FLAG_IPV6
			| EIP_TR_IPSEC_FLAG_UDP | EIP_TR_IPSEC_FLAG_TUNNEL);

	switch (val) {
	/*
	 * Encapsulation.
	 */
	case (EIP_TR_IPSEC_FLAG_ENC | EIP_TR_IPSEC_FLAG_TUNNEL | EIP_TR_IPSEC_FLAG_UDP):
		return EIP_TR_IPSEC_OHDR_PROTO_V4_TUNNEL_NATT_ENC;
	case (EIP_TR_IPSEC_FLAG_ENC | EIP_TR_IPSEC_FLAG_TUNNEL | EIP_TR_IPSEC_FLAG_IPV6):
		return EIP_TR_IPSEC_OHDR_PROTO_V6_TUNNEL_ENC;
	case (EIP_TR_IPSEC_FLAG_ENC | EIP_TR_IPSEC_FLAG_TUNNEL):
		return EIP_TR_IPSEC_OHDR_PROTO_V4_TUNNEL_ENC;
	case (EIP_TR_IPSEC_FLAG_ENC | EIP_TR_IPSEC_FLAG_UDP):
		return EIP_TR_IPSEC_OHDR_PROTO_V4_TRANSPORT_NATT_ENC;
	case (EIP_TR_IPSEC_FLAG_ENC | EIP_TR_IPSEC_FLAG_IPV6):
		return EIP_TR_IPSEC_OHDR_PROTO_V6_TRANSPORT_ENC;
	case (EIP_TR_IPSEC_FLAG_ENC):
		return EIP_TR_IPSEC_OHDR_PROTO_V4_TRANSPORT_ENC;

	/*
	 * Decapsulation.
	 */
	case (EIP_TR_IPSEC_FLAG_TUNNEL | EIP_TR_IPSEC_FLAG_UDP):
		return EIP_TR_IPSEC_OHDR_PROTO_V4_TUNNEL_NATT_DEC;
	case (EIP_TR_IPSEC_FLAG_TUNNEL | EIP_TR_IPSEC_FLAG_IPV6):
		return EIP_TR_IPSEC_OHDR_PROTO_V6_TUNNEL_DEC;
	case (EIP_TR_IPSEC_FLAG_TUNNEL):
		return EIP_TR_IPSEC_OHDR_PROTO_V4_TUNNEL_DEC;
	case (EIP_TR_IPSEC_FLAG_UDP):
		return EIP_TR_IPSEC_OHDR_PROTO_V4_TRANSPORT_NATT_DEC;
	case (EIP_TR_IPSEC_FLAG_IPV6):
		return EIP_TR_IPSEC_OHDR_PROTO_V6_TRANSPORT_DEC;
	default:
		return EIP_TR_IPSEC_OHDR_PROTO_V4_TRANSPORT_DEC;
	}
}

/*
 * eip_tr_ipsec_verify()
 * 	IPsec specific validation
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
		tr_words[0] |= EIP_TK_CTRL_OP_ENC_HMAC;
	} else {
		eip_tr_ipsec_dec_cmn_init(tr, info, algo);
		tr_words[0] |= EIP_TK_CTRL_OP_HMAC_DEC;
	}

	/*
	 * We use large record for ipsec.
	 */
	tr->tr_addr_type = EIP_HW_CTX_TYPE_LARGE;
	tr->tr_addr_type |= virt_to_phys(tr->hw_words);
	tr->tr_flags = ipsec->sa_flags;
	tr->iv_len = algo->iv_len;
	tr->blk_len = algo->cipher_blk_len;
	tr->digest_len = algo->auth_digest_len;
	tr->ctrl_words[0] = tr_words[0];
	tr->ctrl_words[1] = tr_words[1];
	tr->nonce = info->base.nonce;

	tr->ipsec.ops.tk_fill = NULL;
	tr->ipsec.ops.cb = ipsec->cb;
	tr->ipsec.ops.err_cb = ipsec->err_cb;
	tr->ipsec.app_data = ipsec->app_data;

	memcpy(tr->bypass, ipsec->ppe_mdata, sizeof(tr->bypass));
}

/*
 * eip_tr_ipsec_aes_cbc_init()
 *	Initialize the transform record for ipsec.
 */
static bool eip_tr_ipsec_aes_cbc_init(struct eip_tr *tr, struct eip_tr_info *info, const struct eip_svc_entry *algo)
{
	struct eip_tr_info_ipsec *ipsec = &info->ipsec;
	struct eip_tr_base *crypto = &info->base;
	uint32_t *tr_words = tr->hw_words;
	uint8_t enc;

	/*
	 * We first do common initialization and then algo specific initialization.
	 */
	if (!eip_tr_ipsec_verify(tr, info, algo)) {
		pr_err("%px: Common initialization failed.\n", tr);
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

	enc = !!(ipsec->sa_flags & EIP_TR_IPSEC_FLAG_ENC);
	if (enc) {
		tr_words[68] |= EIP_TR_IPSEC_ESP_PROTO(EIP_TR_IPSEC_PROTO_OUT_CBC);
	} else {
		tr_words[68] |= EIP_TR_IPSEC_ESP_PROTO(EIP_TR_IPSEC_PROTO_IN_CBC);
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
	struct eip_tr_info_ipsec *ipsec = &info->ipsec;
	struct eip_tr_base *crypto = &info->base;
	uint32_t *tr_words = tr->hw_words;
	uint8_t enc;

	/*
	 * We first do common initialization and then algo specific initialization.
	 */
	if (!eip_tr_ipsec_verify(tr, info, algo)) {
		pr_err("%px: Common initialization failed.\n", tr);
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

	enc = !!(ipsec->sa_flags & EIP_TR_IPSEC_FLAG_ENC);
	if (enc) {
		tr_words[68] |= EIP_TR_IPSEC_ESP_PROTO(EIP_TR_IPSEC_PROTO_OUT_CBC);
	} else {
		tr_words[68] |= EIP_TR_IPSEC_ESP_PROTO(EIP_TR_IPSEC_PROTO_IN_CBC);
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
	struct eip_tr_info_ipsec *ipsec = &info->ipsec;
	struct eip_tr_base *base = &info->base;
	uint8_t authkey[AES_BLOCK_SIZE] = {0};
	uint32_t *tr_words = tr->hw_words;
	uint32_t *doffset = NULL;
	uint32_t *key = NULL;
	uint8_t auth_key_idx;
	uint8_t enc;
	int err = 0;

	/*
	 * We first do common initialization and then algo specific initialization.
	 */
	if (!eip_tr_ipsec_verify(tr, info, algo)) {
		pr_err("%px: Common initialization failed.\n", tr);
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

	enc = !!(ipsec->sa_flags & EIP_TR_IPSEC_FLAG_ENC);
	if (enc) {
		tr_words[68] |= EIP_TR_IPSEC_ESP_PROTO(EIP_TR_IPSEC_PROTO_OUT_GCM);
	} else {
		tr_words[68] |= EIP_TR_IPSEC_ESP_PROTO(EIP_TR_IPSEC_PROTO_IN_GCM);
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
 * eip_tr_ipsec_null_hmac_init()
 *	Initialize the transform record for ipsec.
 */
static bool eip_tr_ipsec_null_hmac_init(struct eip_tr *tr, struct eip_tr_info *info, const struct eip_svc_entry *algo)
{
	struct eip_tr_info_ipsec *ipsec = &info->ipsec;
	uint32_t *tr_words = tr->hw_words;
	uint8_t enc;

	/*
	 * We first do common initialization and then algo specific initialization.
	 */
	if (!eip_tr_ipsec_verify(tr, info, algo)) {
		pr_err("%px: Common initialization failed.\n", tr);
		return false;
	}

	eip_tr_ipsec_cmn_init(tr, info, algo);

	/*
	 * Override common encode decode bit set in common init.
	 */
	enc = !!(ipsec->sa_flags & EIP_TR_IPSEC_FLAG_ENC);
	if (enc) {
		tr_words[0] &= ~EIP_TK_CTRL_OP_ENC_HMAC;
		tr_words[0] |= EIP_TK_CTRL_OP_HMAC_ADD;
		tr->ctrl_words[0] = tr_words[0];
		tr_words[68] |= EIP_TR_IPSEC_ESP_PROTO(EIP_TR_IPSEC_PROTO_OUT_CBC);
	} else {
		tr_words[0] &= ~EIP_TK_CTRL_OP_HMAC_DEC;
		tr_words[0] |= EIP_TK_CTRL_OP_HMAC_CHK;
		tr->ctrl_words[0] = tr_words[0];
		tr_words[68] |= EIP_TR_IPSEC_ESP_PROTO(EIP_TR_IPSEC_PROTO_IN_CBC);
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
 * eip_tr_ipsec_tx_done()
 *	Hardware tx completion callback API for Encapsulation/Decapsulation.
 */
void eip_tr_ipsec_tx_done(struct eip_tr *tr, struct eip_hw_desc *hw, struct eip_sw_desc *sw)
{
	struct sk_buff *skb = eip_req2skb(sw->req);
	struct eip_ctx *ctx = tr->ctx;

	/*
	 * Free the SKB
	 */
	skb = eip_req2skb(sw->req);

	/*
	 * Free the SKB.
	 */
	consume_skb(skb);
	kmem_cache_free(ctx->sw_cache, sw);

	/*
	 * Dereference: eip_tr_ipsec_enc/dec()
	 */
	eip_tr_deref(tr);
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
