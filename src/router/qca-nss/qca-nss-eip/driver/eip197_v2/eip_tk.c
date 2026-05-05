/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: ISC
 */

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <crypto/gcm.h>
#include <crypto/aes.h>
#include "eip_priv.h"

/*
 * eip_tk_encauth_gcm_cmn()
 *	Fill encryption and GHASH token instructions for outbound traffic
 */
static inline uint8_t eip_tk_encauth_gcm_cmn(uint32_t *tk_word, uint8_t auth_len, uint8_t skip_len,
		uint16_t data_len, uint8_t digest_len, uint32_t *tk_hdr)
{
	uint32_t *inst  = tk_word;
	uint16_t crypt_len = data_len - auth_len - skip_len;

	/*
	 * Fill instructions.
	 */
	*inst++ = EIP_TK_INST_GCM_HMAC | auth_len;		/* pure hash */
	*inst++ = EIP_TK_INST_BYPASS | skip_len;		/* Bypass data */
	*inst++ = EIP_TK_INST_DEL_DATA | (auth_len + skip_len);	/* Remove y0 */
	*inst++ = EIP_TK_INST_ADD_DATA | AES_BLOCK_SIZE;	/* Hash + enc + store */
	*inst++ = EIP_TK_INST_ENC_GCM | crypt_len;		/* Hash + enc + store */
	*inst++ = EIP_TK_INST_NOPAD_ENC_HMAC;			/* Don't pad */
	*inst++ = EIP_TK_INST_HMAC_ADD | digest_len;		/* insert hash */

	/*
	 * Fill token header.
	 */
	*tk_hdr |= EIP_HW_TOKEN_HDR_CRYPTO_IV16;
	*tk_hdr |= (EIP_HW_TOKEN_HDR_OUTBOUND | data_len);

	return (inst - tk_word);
}

/*
 * eip_tk_authdec_gcm_cmn()
 *	Fill decryption and GHASH token instructions for inbound traffic
 */
static inline uint8_t eip_tk_authdec_gcm_cmn(uint32_t *tk_word, uint8_t auth_len, uint8_t skip_len,
		uint16_t data_len, uint8_t digest_len, uint32_t *tk_hdr)
{
	uint32_t *inst = tk_word;
	uint16_t crypt_len = data_len - auth_len - skip_len - digest_len;

	/*
	 * Fill instructions.
	 */
	*inst++ = EIP_TK_INST_GCM_HMAC | auth_len;		/* pure hash */
	*inst++ = EIP_TK_INST_BYPASS | skip_len;		/* Bypass data */
	*inst++ = EIP_TK_INST_DEL_DATA | (auth_len + skip_len);	/* Remove Y0 */
	*inst++ = EIP_TK_INST_ADD_DATA | AES_BLOCK_SIZE;	/* enc + hash */
	*inst++ = EIP_TK_INST_HMAC_DEC | crypt_len;		/* hash + dec */
	*inst++ = EIP_TK_INST_HMAC_GET | digest_len;		/* Fetch hash/pad */
	*inst++ = EIP_TK_INST_HMAC_CHK | digest_len;		/* Verify hash */

	/*
	 * Fill token header.
	 */
	*tk_hdr |= EIP_HW_TOKEN_HDR_CRYPTO_IV16;
	*tk_hdr |= (EIP_HW_TOKEN_HDR_INBOUND | data_len);

	return (inst - tk_word);
}

/*
 * eip_tk_enc_3des()
 *     Fill encryption tokens.
 */
uint8_t eip_tk_enc_3des(struct eip_tk *tk, struct eip_tk_params *tk_params)
{
	struct skcipher_request *req = eip_req2skcipher_request(tk_params->eip_req);
	struct eip_tr *tr = tk_params->tr;
	uint32_t *tk_hdr = &tk_params->tk_hdr;
	uint32_t *iv_word = (uint32_t *)req->iv;
	uint32_t *tk_word = tk->words;

	/*
	 * Fill token control info.
	 */
	*tk_word++ = tr->ctrl_words[0] | EIP_TK_CTRL_OP_ENC;
	*tk_word++ = tr->ctrl_words[1];

	/*
	 * We construct IV here.
	 */
	*tk_word++ = iv_word[0];
	*tk_word++ = iv_word[1];

	/*
	 * Fill encryption instructions.
	 */
	*tk_word++ = EIP_TK_INST_ENC | req->cryptlen;	/* Encrypt data */
	*tk_word++ = EIP_TK_INST_PAD_ENC;		/* Padding before encrypt */

	/*
	 * Fill token header.
	 */
	*tk_hdr |= EIP_HW_TOKEN_HDR_CRYPTO_IV8;
	*tk_hdr |= (EIP_HW_TOKEN_HDR_OUTBOUND | req->cryptlen);

	/*
	 * Total token words.
	 */
	return (tk_word - tk->words);
}
EXPORT_SYMBOL(eip_tk_enc_3des);

/*
 * eip_tk_dec_3des()
 *     Fill decryption tokens.
 */
uint8_t eip_tk_dec_3des(struct eip_tk *tk, struct eip_tk_params *tk_params)
{
	struct skcipher_request *req = eip_req2skcipher_request(tk_params->eip_req);
	struct eip_tr *tr = tk_params->tr;
	uint32_t *tk_hdr = &tk_params->tk_hdr;
	uint32_t *iv_word = (uint32_t *)req->iv;
	uint32_t *tk_word = tk->words;

	/*
	 * Fill token control info.
	 */
	*tk_word++ = tr->ctrl_words[0] | EIP_TK_CTRL_OP_DEC;
	*tk_word++ = tr->ctrl_words[1];

	/*
	 * We construct IV here.
	 */
	*tk_word++ = iv_word[0];
	*tk_word++ = iv_word[1];

	/*
	 * Fill decryption instructions.
	 */
	*tk_word++ = EIP_TK_INST_DEC | req->cryptlen;	/* Decrypt data */
	*tk_word++ = EIP_TK_INST_DEC_CHK_PAD;		/* Check padding */

	/*
	 * Fill token header.
	 */
	*tk_hdr |= EIP_HW_TOKEN_HDR_CRYPTO_IV8;
	*tk_hdr |= (EIP_HW_TOKEN_HDR_INBOUND | req->cryptlen);

	/*
	 * Total token words.
	 */
	return (tk_word - tk->words);
}
EXPORT_SYMBOL(eip_tk_dec_3des);

/*
 * eip_tk_enc()
 *	Fill encryption tokens.
 */
uint8_t eip_tk_enc(struct eip_tk *tk, struct eip_tk_params *tk_params)
{
	struct skcipher_request *req = eip_req2skcipher_request(tk_params->eip_req);
	struct crypto_skcipher *tfm = crypto_skcipher_reqtfm(req);
	uint32_t iv_len = crypto_skcipher_ivsize(tfm);
	uint32_t *tk_hdr = &tk_params->tk_hdr;
	struct eip_tr *tr = tk_params->tr;
	uint32_t *tk_word = tk->words;
	uint32_t zeroes[4] = {0};
	uint32_t *iv_word;

	iv_word = iv_len ? (uint32_t *)req->iv : zeroes;

	/*
	 * Fill token control info.
	 */
	*tk_word++ = tr->ctrl_words[0] | EIP_TK_CTRL_OP_ENC;
	*tk_word++ = tr->ctrl_words[1];

	/*
	 * We construct IV here.
	 */
	*tk_word++ = iv_word[0];
	*tk_word++ = iv_word[1];
	*tk_word++ = iv_word[2];
	*tk_word++ = iv_word[3];

	/*
	 * Fill encryption instructions.
	 */
	*tk_word++ = EIP_TK_INST_ENC | req->cryptlen;	/* Encrypt data */
	*tk_word++ = EIP_TK_INST_PAD_ENC;		/* Padding before encrypt */

	/*
	 * Fill token header.
	 */
	*tk_hdr |= EIP_HW_TOKEN_HDR_CRYPTO_IV16;
	*tk_hdr |= (EIP_HW_TOKEN_HDR_OUTBOUND | req->cryptlen);

	/*
	 * Total token words.
	 */
	return (tk_word - tk->words);
}
EXPORT_SYMBOL(eip_tk_enc);

/*
 * eip_tk_dec()
 *	Fill decryption tokens.
 */
uint8_t eip_tk_dec(struct eip_tk *tk, struct eip_tk_params *tk_params)
{
	struct skcipher_request *req = eip_req2skcipher_request(tk_params->eip_req);
	struct crypto_skcipher *tfm = crypto_skcipher_reqtfm(req);
	uint32_t iv_len = crypto_skcipher_ivsize(tfm);
	uint32_t *tk_hdr = &tk_params->tk_hdr;
	struct eip_tr *tr = tk_params->tr;
	uint32_t *tk_word = tk->words;
	uint32_t zeroes[4] = {0};
	uint32_t *iv_word;

	iv_word = iv_len ? (uint32_t *)req->iv : zeroes;

	/*
	 * Fill token control info.
	 */
	*tk_word++ = tr->ctrl_words[0] | EIP_TK_CTRL_OP_DEC;
	*tk_word++ = tr->ctrl_words[1];

	/*
	 * We construct IV here.
	 */
	*tk_word++ = iv_word[0];
	*tk_word++ = iv_word[1];
	*tk_word++ = iv_word[2];
	*tk_word++ = iv_word[3];

	/*
	 * Fill decryption instructions.
	 */
	*tk_word++ = EIP_TK_INST_DEC | req->cryptlen;	/* Decrypt data */
	*tk_word++ = EIP_TK_INST_DEC_CHK_PAD;		/* Check padding */

	/*
	 * Fill token header.
	 */
	*tk_hdr |= EIP_HW_TOKEN_HDR_CRYPTO_IV16;
	*tk_hdr |= (EIP_HW_TOKEN_HDR_INBOUND | req->cryptlen);

	/*
	 * Total token words.
	 */
	return (tk_word - tk->words);
}
EXPORT_SYMBOL(eip_tk_dec);

/*
 * eip_tk_enc_ctr_rfc()
 *	Fill encryption tokens.
 */
uint8_t eip_tk_enc_ctr_rfc(struct eip_tk *tk, struct eip_tk_params *tk_params)
{
	struct skcipher_request *req = eip_req2skcipher_request(tk_params->eip_req);
	struct eip_tr *tr = tk_params->tr;
	uint32_t *tk_hdr = &tk_params->tk_hdr;
	uint32_t *iv_word = (uint32_t *)req->iv;
	uint32_t *tk_word = tk->words;

	/*
	 * Fill token control info.
	 */
	*tk_word++ = tr->ctrl_words[0] | EIP_TK_CTRL_OP_ENC;
	*tk_word++ = tr->ctrl_words[1];

	/*
	 * We construct IV here from cached nonce during setkey.
	 * Byte[0]: nonce from key.
	 * Byte[1]: IV[0] from request.
	 * Byte[2]: IV[1] from request.
	 * Byte[3]: Counter set to 1.
	 */
	*tk_word++ = tr->nonce;
	*tk_word++ = iv_word[0];
	*tk_word++ = iv_word[1];
	*tk_word++ = ntohl(0x1);

	/*
	 * Fill encryption instructions.
	 */
	*tk_word++ = EIP_TK_INST_ENC | req->cryptlen;	/* Encrypt data */
	*tk_word++ = EIP_TK_INST_PAD_ENC;		/* Padding */

	/*
	 * Fill token header.
	 */
	*tk_hdr |= EIP_HW_TOKEN_HDR_CRYPTO_IV16;
	*tk_hdr |= (EIP_HW_TOKEN_HDR_OUTBOUND | req->cryptlen);

	/*
	 * Total token words.
	 */
	return (tk_word - tk->words);
}
EXPORT_SYMBOL(eip_tk_enc_ctr_rfc);

/*
 * eip_tk_dec_ctr_rfc()
 *	Fill decryption tokens.
 */
uint8_t eip_tk_dec_ctr_rfc(struct eip_tk *tk, struct eip_tk_params *tk_params)
{
	struct skcipher_request *req = eip_req2skcipher_request(tk_params->eip_req);
	uint32_t *iv_word = (uint32_t *)req->iv;
	uint32_t *tk_hdr = &tk_params->tk_hdr;
	struct eip_tr *tr = tk_params->tr;
	uint32_t *tk_word = tk->words;

	/*
	 * Fill token control info.
	 */
	*tk_word++ = tr->ctrl_words[0] | EIP_TK_CTRL_OP_DEC;
	*tk_word++ = tr->ctrl_words[1];

	/*
	 * We construct IV here from cached nonce during setkey.
	 * Byte[0]: nonce from key.
	 * Byte[1]: IV[0] from request.
	 * Byte[2]: IV[1] from request.
	 * Byte[3]: Counter set to 1.
	 */
	*tk_word++ = tr->nonce;
	*tk_word++ = iv_word[0];
	*tk_word++ = iv_word[1];
	*tk_word++ = ntohl(0x1);

	/*
	 * Fill decryption instructions.
	 */
	*tk_word++ = EIP_TK_INST_DEC | req->cryptlen;	/* Decrypt data */
	*tk_word++ = EIP_TK_INST_DEC_CHK_PAD;		/* Check padding */

	/*
	 * Fill token header.
	 */
	*tk_hdr |= EIP_HW_TOKEN_HDR_CRYPTO_IV16;
	*tk_hdr |= (EIP_HW_TOKEN_HDR_INBOUND | req->cryptlen);

	/*
	 * Total token words.
	 */
	return (tk_word - tk->words);
}
EXPORT_SYMBOL(eip_tk_dec_ctr_rfc);

/*
 * eip_tk_auth()
 *	Fill auth tokens.
 */
uint8_t eip_tk_auth(struct eip_tk *tk, struct eip_tk_params *tk_params)
{
	struct ahash_request *req = eip_req2ahash_request(tk_params->eip_req);
	struct crypto_ahash *tfm = crypto_ahash_reqtfm(req);
	struct eip_tr *tr = tk_params->tr;
	uint32_t *tk_hdr = &tk_params->tk_hdr;
	unsigned int hmac_len = crypto_ahash_digestsize(tfm);
	uint32_t *tk_word = tk->words;

	/*
	 * Fill token control info.
	 */
	*tk_word++ = tr->ctrl_words[0] | EIP_TK_CTRL_OP_HMAC_ADD;
	*tk_word++ = tr->ctrl_words[1];

	/*
	 * Fill the instructions.
	 */
	*tk_word++ = EIP_TK_INST_HASH | req->nbytes;	/* Generate hash. */
	*tk_word++ = EIP_TK_INST_HMAC_ADD | hmac_len;	/* Fetch hash. */

	/*
	 * Fill token header.
	 */
	*tk_hdr |= EIP_HW_TOKEN_HDR_CRYPTO_CMN;
	*tk_hdr |= (EIP_HW_TOKEN_HDR_OUTBOUND | req->nbytes);

	/*
	 * Total token words.
	 */
	return (tk_word - tk->words);
}
EXPORT_SYMBOL(eip_tk_auth);

/*
 * eip_tk_encauth_cbc()
 *	Fill tokens for AEAD outbound traffic.
 */
uint8_t eip_tk_encauth_cbc(struct eip_tk *tk, struct eip_tk_params *tk_params)
{
	struct aead_request *req = eip_req2aead_request(tk_params->eip_req);
	struct crypto_aead *tfm = crypto_aead_reqtfm(req);
	struct eip_tr *tr = tk_params->tr;
	uint32_t *tk_hdr = &tk_params->tk_hdr;
	uint16_t data_len = req->cryptlen + req->assoclen;
	uint16_t digest_len = crypto_aead_authsize(tfm);
	uint32_t *iv_word = (uint32_t *)req->iv;
	uint16_t auth_len = req->assoclen;
	uint32_t *tk_word = tk->words;

	/*
	 * Fill token control info.
	 */
	*tk_word++ = tr->ctrl_words[0] | EIP_TK_CTRL_OP_ENC_HMAC;
	*tk_word++ = tr->ctrl_words[1];

	/*
	 * We construct IV here.
	 */
	*tk_word++ = iv_word[0];
	*tk_word++ = iv_word[1];
	*tk_word++ = iv_word[2];
	*tk_word++ = iv_word[3];

	/*
	 * Fill the instructions.
	 */
	*tk_word++ = EIP_TK_INST_HMAC | auth_len;			/* Pure hash. */
	*tk_word++ = EIP_TK_INST_ENC_HMAC | (data_len - auth_len);	/* Encryption + Hash. */
	*tk_word++ = EIP_TK_INST_NOPAD_ENC_HMAC;			/* Don't pad before enc + hash.*/
	*tk_word++ = EIP_TK_INST_HMAC_ADD | digest_len;			/* Insert generated hash.*/

	/*
	 * Fill token header.
	 */
	*tk_hdr |= EIP_HW_TOKEN_HDR_CRYPTO_IV16;
	*tk_hdr |= (EIP_HW_TOKEN_HDR_OUTBOUND | data_len);

	/*
	 * Total token words.
	 */
	return (tk_word - tk->words);
}
EXPORT_SYMBOL(eip_tk_encauth_cbc);

/*
 * eip_tk_authdec_cbc()
 *	Fill tokens for AEAD inbound traffic.
 */
uint8_t eip_tk_authdec_cbc(struct eip_tk *tk, struct eip_tk_params *tk_params)
{
	struct aead_request *req = eip_req2aead_request(tk_params->eip_req);
	struct crypto_aead *tfm = crypto_aead_reqtfm(req);
	struct eip_tr *tr = tk_params->tr;
	uint32_t *tk_hdr = &tk_params->tk_hdr;
	uint16_t data_len = req->cryptlen + req->assoclen;
	uint16_t digest_len = crypto_aead_authsize(tfm);
	uint32_t *iv_word = (uint32_t *)req->iv;
	uint16_t auth_len = req->assoclen;
	uint32_t *tk_word = tk->words;

	/*
	 * Fill token control info.
	 */
	*tk_word++ = tr->ctrl_words[0] | EIP_TK_CTRL_OP_HMAC_DEC;
	*tk_word++ = tr->ctrl_words[1];

	/*
	 * We construct IV here.
	 */
	*tk_word++ = iv_word[0];
	*tk_word++ = iv_word[1];
	*tk_word++ = iv_word[2];
	*tk_word++ = iv_word[3];

	/*
	 * Fill instructions.
	 */
	*tk_word++ = EIP_TK_INST_HMAC | auth_len;					/* Pure hash */
	*tk_word++ = EIP_TK_INST_HMAC_DEC | (data_len - auth_len - digest_len);		/* Perform hash before decryption. */
	*tk_word++ = EIP_TK_INST_HMAC_GET | digest_len;					/* Fetch hash/pad */
	*tk_word++ = EIP_TK_INST_HMAC_CHK | digest_len;					/* Verify hash */

	/*
	 * Fill token control data.
	 */
	*tk_hdr |= EIP_HW_TOKEN_HDR_CRYPTO_IV16;
	*tk_hdr |= (EIP_HW_TOKEN_HDR_INBOUND | data_len);

	/*
	 * Total token words.
	 */
	return (tk_word - tk->words);
}
EXPORT_SYMBOL(eip_tk_authdec_cbc);

/*
 * eip_tk_encauth_ctr_rfc()
 *	Fill tokens for AEAD outbound traffic.
 */
uint8_t eip_tk_encauth_ctr_rfc(struct eip_tk *tk, struct eip_tk_params *tk_params)
{
	struct aead_request *req = eip_req2aead_request(tk_params->eip_req);
	struct crypto_aead *tfm = crypto_aead_reqtfm(req);
	struct eip_tr *tr = tk_params->tr;
	uint32_t *tk_hdr = &tk_params->tk_hdr;
	uint16_t data_len = req->cryptlen + req->assoclen;
	uint16_t digest_len = crypto_aead_authsize(tfm);
	uint32_t *iv_word = (uint32_t *)req->iv;
	uint16_t auth_len = req->assoclen;
	uint32_t *tk_word = tk->words;

	/*
	 * Fill token control info.
	 */
	*tk_word++ = tr->ctrl_words[0] | EIP_TK_CTRL_OP_ENC_HMAC;
	*tk_word++ = tr->ctrl_words[1];

	/*
	 * We construct IV here from cached nonce during setkey.
	 * Byte[0]: nonce from key.
	 * Byte[1]: IV[0] from request.
	 * Byte[2]: IV[1] from request.
	 * Byte[3]: Counter set to 1.
	 */
	*tk_word++ = tr->nonce;
	*tk_word++ = iv_word[0];
	*tk_word++ = iv_word[1];
	*tk_word++ = ntohl(0x1);

	/*
	 * Fill the encryption and hmac instructions.
	 */
	*tk_word++ = EIP_TK_INST_HMAC | auth_len;			/* Pure hash */
	*tk_word++ = EIP_TK_INST_ENC_HMAC | (data_len - auth_len);	/* Encryption + Hash */
	*tk_word++ = EIP_TK_INST_NOPAD_ENC_HMAC;			/* Don't do padding */
	*tk_word++ = EIP_TK_INST_HMAC_ADD | digest_len;			/* Insert hash */

	/*
	 * Fill token header.
	 */
	*tk_hdr |= EIP_HW_TOKEN_HDR_CRYPTO_IV16;
	*tk_hdr |= (EIP_HW_TOKEN_HDR_OUTBOUND | data_len);

	/*
	 * Total token words.
	 */
	return (tk_word - tk->words);
}
EXPORT_SYMBOL(eip_tk_encauth_ctr_rfc);

/*
 * eip_tk_authdec_ctr_rfc()
 *	Fill tokens for AEAD inbound traffic.
 */
uint8_t eip_tk_authdec_ctr_rfc(struct eip_tk *tk, struct eip_tk_params *tk_params)
{
	struct aead_request *req = eip_req2aead_request(tk_params->eip_req);
	struct crypto_aead *tfm = crypto_aead_reqtfm(req);
	struct eip_tr *tr = tk_params->tr;
	uint32_t *tk_hdr = &tk_params->tk_hdr;
	uint16_t data_len = req->cryptlen + req->assoclen;
	uint16_t digest_len = crypto_aead_authsize(tfm);
	uint32_t *iv_word = (uint32_t *)req->iv;
	uint16_t auth_len = req->assoclen;
	uint32_t *tk_word = tk->words;

	/*
	 * Fill token control info.
	 */
	*tk_word++ = tr->ctrl_words[0] | EIP_TK_CTRL_OP_HMAC_DEC;
	*tk_word++ = tr->ctrl_words[1];

	/*
	 * We construct IV here from cached nonce during setkey.
	 * Byte[0]: nonce from key.
	 * Byte[1]: IV[0] from request.
	 * Byte[2]: IV[1] from request.
	 * Byte[3]: Counter set to 1.
	 */
	*tk_word++ = tr->nonce;
	*tk_word++ = iv_word[0];
	*tk_word++ = iv_word[1];
	*tk_word++ = ntohl(0x1);

	/*
	 * Fill instructions.
	 */
	*tk_word++ = EIP_TK_INST_HMAC | auth_len;					/* Pure hash */
	*tk_word++ = EIP_TK_INST_HMAC_DEC | (data_len - auth_len - digest_len);		/* Encryption + Hash */
	*tk_word++ = EIP_TK_INST_HMAC_GET | digest_len;					/* Fetch hash/pad */
	*tk_word++ = EIP_TK_INST_HMAC_CHK | digest_len;					/* Verify hash */

	/*
	 * Fill control header.
	 */
	*tk_hdr |= EIP_HW_TOKEN_HDR_CRYPTO_IV16;
	*tk_hdr |= (EIP_HW_TOKEN_HDR_INBOUND | data_len);

	/*
	 * Total token words.
	 */
	return (tk_word - tk->words);
}
EXPORT_SYMBOL(eip_tk_authdec_ctr_rfc);

/*
 * eip_tk_digest()
 *	Fill instruction for Digest calculation.
 */
uint8_t eip_tk_digest(struct eip_tk *tk, struct eip_tr *tr, struct scatterlist *sg, uint32_t *tk_hdr,
		uint8_t ipad_offst, uint8_t pad_words)
{
	uint32_t *tk_word = tk->words;
	uint32_t *tk_ctrl;
	uint8_t opad_offst;

	/*
	 * Initialize the token for ipad & opad generation.
	 */
	tk_ctrl = tk_word;
	tk_word += 2;

	tk_ctrl[0] = EIP_HW_CTRL_LEN(EIP_HW_MAX_CTRL);
	tk_ctrl[0] |= EIP_HW_CTRL_OPTION_UNFINISH_HASH_TO_CTX;
	tk_ctrl[0] |= EIP_HW_CTRL_TOP_HMAC_ADD;
	tk_ctrl[0] |= EIP_HW_CTRL_DIGEST_TYPE;

	/*
	 * Hash algo number is filled in cached TR control word.
	 */
	tk_ctrl[0] |= (tr->ctrl_words[0] & EIP_HW_CTRL_HASH_OP_MASK);
	tk_ctrl[1] = 0x0;

	/*
	 * Fill instructiion.
	 * We first update inner digest and then outer digest. Both of having same size.
	 */
	opad_offst = ipad_offst + pad_words;

	*tk_word++ = EIP_TK_INST_DIGEST | sg->length;
	*tk_word++ = EIP_TK_INST_IPAD_GEN;
	*tk_word++ = EIP_TK_INST_NOP;
	*tk_word++ = EIP_TK_INST_OPAD_GEN;
	*tk_word++ = EIP_TK_INST_IPAD_ADD | EIP_TK_INST_CONTEXT_ACCESS_LEN(pad_words) | ipad_offst;
	*tk_word++ = EIP_TK_INST_OPAD_ADD | EIP_TK_INST_CONTEXT_ACCESS_LEN(pad_words) | opad_offst;

	/*
	 * Fill command token header.
	 */
	*tk_hdr = EIP_HW_TOKEN_HDR_DIGEST | sg->length;

	pr_debug("%px: Digest token filled for digest size(%u) pad words(%u) data len(%u)\n", tk,
			tr->digest_len, pad_words, sg->length);
	return (tk_word - tk->words);
}

/*
 * eip_tk_encauth_gcm()
 *	Fill tokens for AEAD outbound traffic.
 */
uint8_t eip_tk_encauth_gcm(struct eip_tk *tk, struct eip_tk_params *tk_params)
{
	struct aead_request *req = eip_req2aead_request(tk_params->eip_req);
	struct crypto_aead *tfm = crypto_aead_reqtfm(req);
	struct eip_tr *tr = tk_params->tr;
	uint32_t *tk_hdr = &tk_params->tk_hdr;
	uint16_t data_len = req->cryptlen + req->assoclen;
	uint16_t digest_len = crypto_aead_authsize(tfm);
	uint32_t *iv_word = (uint32_t *)req->iv;
	uint16_t auth_len = req->assoclen;
	uint32_t *tk_word = tk->words;

	/*
	 * Fill token control info.
	 */
	*tk_word++ = tr->ctrl_words[0] | EIP_TK_CTRL_OP_ENC_HMAC;
	*tk_word++ = tr->ctrl_words[1];

	/*
	 * We construct IV here. In case of GCM linux only give 12 bytes of IV.
	 * We add additional 4 bytes of counter to make it 16 bytes as this is
	 * expected from hardware.
	 * Byte[0..2]: IV[0..2] from request.
	 * Byte[3]: Counter set to 1.
	 */
	*tk_word++ = iv_word[0];
	*tk_word++ = iv_word[1];
	*tk_word++ = iv_word[2];
	*tk_word++ = ntohl(0x1);

	/*
	 * Fill instructions and token header.
	 * For non RFC case IV is included for authentication hence skip length is 0.
	 */
	tk_word += eip_tk_encauth_gcm_cmn(tk_word, auth_len, 0, data_len, digest_len, tk_hdr);

	/*
	 * Total token words.
	 */
	return (tk_word - tk->words);
}
EXPORT_SYMBOL(eip_tk_encauth_gcm);

/*
 * eip_tk_authdec_gcm()
 *	Fill tokens for AEAD inbound traffic.
 */
uint8_t eip_tk_authdec_gcm(struct eip_tk *tk, struct eip_tk_params *tk_params)
{
	struct aead_request *req = eip_req2aead_request(tk_params->eip_req);
	struct crypto_aead *tfm = crypto_aead_reqtfm(req);
	struct eip_tr *tr = tk_params->tr;
	uint32_t *tk_hdr = &tk_params->tk_hdr;
	uint16_t data_len = req->cryptlen + req->assoclen;
	uint16_t digest_len = crypto_aead_authsize(tfm);
	uint32_t *iv_word = (uint32_t *)req->iv;
	uint16_t auth_len = req->assoclen;
	uint32_t *tk_word = tk->words;

	/*
	 * Fill token control info.
	 */
	*tk_word++ = tr->ctrl_words[0] | EIP_TK_CTRL_OP_HMAC_DEC;
	*tk_word++ = tr->ctrl_words[1];

	/*
	 * We construct IV here. In case of GCM linux only give 12 bytes of IV.
	 * We add additional 4 bytes of counter to make it 16 bytes as this is
	 * expected from hardware.
	 * Byte[0..2]: IV[0..2] from request.
	 * Byte[3]: Counter set to 1.
	 */
	*tk_word++ = iv_word[0];
	*tk_word++ = iv_word[1];
	*tk_word++ = iv_word[2];
	*tk_word++ = ntohl(0x1);

	/*
	 * Fill token instructions and token header
	 * For non RFC case IV is included for authentication hence skip length is 0.
	 */
	tk_word += eip_tk_authdec_gcm_cmn(tk_word, auth_len, 0, data_len, digest_len, tk_hdr);

	/*
	 * Total token words.
	 */
	return (tk_word - tk->words);
}
EXPORT_SYMBOL(eip_tk_authdec_gcm);

/*
 * eip_tk_encauth_gcm_rfc()
 *	Fill encryption tokens followed by gmac tokens.
 */
uint8_t eip_tk_encauth_gcm_rfc(struct eip_tk *tk, struct eip_tk_params *tk_params)
{
	struct aead_request *req = eip_req2aead_request(tk_params->eip_req);
	struct crypto_aead *tfm = crypto_aead_reqtfm(req);
	struct eip_tr *tr = tk_params->tr;
	uint32_t *tk_hdr = &tk_params->tk_hdr;
	uint16_t data_len = req->cryptlen + req->assoclen;
	uint16_t digest_len = crypto_aead_authsize(tfm);
	uint32_t *iv_word = (uint32_t *)req->iv;
	uint16_t iv_len = crypto_aead_ivsize(tfm);
	uint16_t auth_len = req->assoclen - iv_len;
	uint32_t *tk_word = tk->words;

	/*
	 * Fill token control info.
	 */
	*tk_word++ = tr->ctrl_words[0] | EIP_TK_CTRL_OP_ENC_HMAC;
	*tk_word++ = tr->ctrl_words[1];

	/*
	 * We construct IV here from cached nonce during tr alloc.
	 * Byte[0]: nonce from key.
	 * Byte[1]: IV[0] from request.
	 * Byte[2]: IV[1] from request.
	 * Byte[3]: Counter set to 1.
	 */
	*tk_word++ = tr->nonce;
	*tk_word++ = iv_word[0];
	*tk_word++ = iv_word[1];
	*tk_word++ = ntohl(0x1);

	/*
	 * Fill token instructions and token header
	 * For RFC case IV is not included for authentication.
	 */
	tk_word += eip_tk_encauth_gcm_cmn(tk_word, auth_len, iv_len, data_len, digest_len, tk_hdr);

	/*
	 * Total token words.
	 */
	return (tk_word - tk->words);
}
EXPORT_SYMBOL(eip_tk_encauth_gcm_rfc);

/*
 * eip_tk_authdec_gcm_rfc()
 *	Fill gmac tokens followed by decryption tokens.
 */
uint8_t eip_tk_authdec_gcm_rfc(struct eip_tk *tk, struct eip_tk_params *tk_params)
{
	struct aead_request *req = eip_req2aead_request(tk_params->eip_req);
	struct crypto_aead *tfm = crypto_aead_reqtfm(req);
	struct eip_tr *tr = tk_params->tr;
	uint32_t *tk_hdr = &tk_params->tk_hdr;
	uint16_t data_len = req->cryptlen + req->assoclen;
	uint16_t digest_len = crypto_aead_authsize(tfm);
	uint32_t *iv_word = (uint32_t *)req->iv;
	uint16_t iv_len = crypto_aead_ivsize(tfm);
	uint16_t auth_len = req->assoclen - iv_len;
	uint32_t *tk_word = tk->words;

	/*
	 * Fill token control info.
	 */
	*tk_word++ = tr->ctrl_words[0] | EIP_TK_CTRL_OP_HMAC_DEC;
	*tk_word++ = tr->ctrl_words[1];

	/*
	 * We construct IV here from cached nonce during setkey.
	 * Byte[0]: nonce from key.
	 * Byte[1]: IV[0] from request.
	 * Byte[2]: IV[1] from request.
	 * Byte[3]: Counter set to 1.
	 */
	*tk_word++ = tr->nonce;
	*tk_word++ = iv_word[0];
	*tk_word++ = iv_word[1];
	*tk_word++ = ntohl(0x1);

	/*
	 * Fill token instructions and token header
	 * For RFC case IV is not included for authentication
	 */
	tk_word += eip_tk_authdec_gcm_cmn(tk_word, auth_len, iv_len, data_len, digest_len, tk_hdr);

	/*
	 * Total token words.
	 */
	return (tk_word - tk->words);
}
EXPORT_SYMBOL(eip_tk_authdec_gcm_rfc);
