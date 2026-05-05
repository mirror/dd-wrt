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
#include <crypto/aes.h>

#include "eip_priv.h"

/*
 * eip_tk_dtls_encauth_cbc()
 *      Fill tokens for dtls outbound traffic.
 */
uint8_t eip_tk_dtls_encauth_cbc(struct eip_tk *tk, struct eip_tk_params *tk_params)
{
	struct eip_tr *tr = tk_params->tr;
	struct sk_buff *skb = eip_req2skb(tk_params->eip_req);
	struct eip_tr_dtls *dtls = &tr->dtls;

	/*
	 * TODO: Remove use of CB. Instead use structure in param to accommodate extra info.
	 */
	struct eip_tr_dtls_skb_cb *cb = EIP_TR_DTLS_SKB_CB(skb);
	uint32_t *tk_hdr = &tk_params->tk_hdr;
	uint8_t bypass_len = dtls->bypass_len;
	uint8_t digest_len = tr->digest_len;
	uint8_t pad_len = cb->pad_len;
	uint32_t *tk_word = tk->words;
	uint16_t len, clen;

	len = skb->len;
	clen = len - bypass_len;

	/*
	 * Fill the encap instructions without ESN
	 */
	*tk_word++ = EIP_TK_INST_BYPASS | bypass_len;		/* Bypass outer headers */
	*tk_word++ = EIP_TK_INST_EPOCH_SEQ_HMAC;		/* Hash epoch & upper 16 bit Sequence number */
	*tk_word++ = EIP_TK_INST_LO_SEQ_HMAC;			/* Hash lower 32bit sequence number */
	*tk_word++ = EIP_TK_INST_TYPE_HMAC_N_OUT | EIP_TK_PKT_TYPE(EIP_DTLS_TYPE_APP_DATA);
								/* Hash type and write to output */
	*tk_word++ = EIP_TK_INST_VER_HMAC_N_OUT;		/* Hash version and write to output */
	*tk_word++ = EIP_TK_INST_EPOCH_SEQ_OUT;			/* Write epoch & seq to output */
	*tk_word++ = EIP_TK_INST_LO_SEQ_OUT;			/* Write lower seq to output */
	*tk_word++ = EIP_TK_INST_DATA_LEN_HMAC;			/* Hash payload length field */
	*tk_word++ = EIP_TK_INST_DATA | htons(clen);
	*tk_word++ = EIP_TK_INST_FRAG_LEN_OUT;			/* Write data fragment length */
	*tk_word++ = EIP_TK_INST_DATA | htons(clen + tr->iv_len + digest_len + pad_len);
	*tk_word++ = EIP_TK_INST_IV_OUT | tr->iv_len;		/* Write IV to output */
	*tk_word++ = EIP_TK_INST_DATA_HMAC_ENC_OUT | clen;	/* Hash-encrypt and write payload to output */
	*tk_word++ = EIP_TK_INST_HMAC_OUT | digest_len;		/* Encrypt generated HMAC tag and write to output */
	*tk_word++ = EIP_TK_INST_PAD_ENC_OUT | pad_len;		/* Encrypt padding and write to output */
	*tk_word++ = EIP_TK_INST_SEQ64_NO_UPDT | dtls->seq_offset;	/* Update sequence number */

	/*
	 * Fill token header
	 */
	*tk_hdr |= EIP_HW_TOKEN_HDR_DTLS_CMN | EIP_HW_TOKEN_HDR_IV_PRNG;
	*tk_hdr |= EIP_HW_TOKEN_HDR_OUTBOUND | len;

	/*
	 * Total token words.
	 */
	return (tk_word - tk->words);
}

/*
 * eip_tk_dtls_authdec_cbc()
 *      Fill tokens for dtls inbound traffic.
 */
uint8_t eip_tk_dtls_authdec_cbc(struct eip_tk *tk, struct eip_tk_params *tk_params)
{
	struct eip_tr *tr = tk_params->tr;
	struct sk_buff *skb = eip_req2skb(tk_params->eip_req);
	struct eip_tr_dtls *dtls = &tr->dtls;
	uint32_t *tk_hdr = &tk_params->tk_hdr;
	void *tail = skb->data + skb->len;
	uint8_t digest_len = tr->digest_len;
	uint8_t blk_len = tr->blk_len;
	uint32_t *tk_word = tk->words;
	uint16_t len, clen, copylen;

	len = skb->len;
	clen = len - sizeof(struct eip_dtls_hdr) - tr->iv_len - digest_len - EIP_TK_DTLS_PAD_LEN_SZ;

	/*
	 * To decrypt last byte containing padding length. Hardware splits the operation in two parts:
	 * 1. Decrypt last two block of data excluding last 4 bytes.
	 * 2. Now decrypt last 4 byte and last byte is stored internally for length calculation.
	 */
	copylen = (blk_len * 2) - 4;

	/*
	 * Fill the decapsulation instructions
	 */
	*tk_word++ = EIP_TK_INST_REM_OUT_WORD;	/* Schedule Remove of last word */
	*tk_word++ = EIP_TK_INST_2BLK_DEC | copylen;/* Decrypt last 2 block - 4 bytes */
	memcpy(tk_word, tail - (blk_len * 2), copylen);
	tk_word += (copylen / sizeof(uint32_t));

	*tk_word++ = EIP_TK_INST_WORD_DEC_OUT;		/* Decrypt and write last 4 bytes */
	*tk_word++ = *(uint32_t *)(tail - 4);
	*tk_word++ = EIP_TK_INST_TYPE_N_VER_UPDT | EIP_TK_INST_LAST_BIT;
							/* Read type & version from input and store in context */
	*tk_word++ = EIP_TK_INST_EPOCH_N_HSEQ_HMAC;	/* Hash epoch and higher sequence from packet */
	*tk_word++ = EIP_TK_INST_LSEQ_HMAC;		/* hash lower sequence from packet */
	*tk_word++ = EIP_TK_INST_TYPE_N_VER_HMAC;	/* Hash type & version from context stored in previous instrunction */
	*tk_word++ = EIP_TK_INST_ENC_LEN_UPDT | clen;	/* Hash internal payload length */
	*tk_word++ = EIP_TK_INST_FRAG_LEN_REM_STORE;	/* Remove dtls frag length from input */
	*tk_word++ = EIP_TK_INST_IV_UPDT | tr->iv_len;	/* Copy Packet IV to context */
	*tk_word++ = EIP_TK_INST_INPUT_HMAC | EIP_TK_IRR_LEN(digest_len);
							/* Internal pointer to MAC */
	*tk_word++ = EIP_TK_INST_DATA_DEC_HMAC_N_OUT;	/* Decrypt-hmac payload and write to output */
	*tk_word++ = EIP_TK_INST_HMAC_DEC_N_OUT | (digest_len + EIP_TK_DTLS_PAD_LEN_SZ);
								/* Decrypt MAC */
	*tk_word++ = EIP_TK_INST_SEQ_PAD_HMAC_CHK | digest_len;	/* inbound checks */

	if (dtls->seq_offset)
		*tk_word++ = EIP_TK_INST_SEQ_UPDT | dtls->seq_offset;	/* Update sequence number */

	/*
	 * Fill token header
	 */
	*tk_hdr |= EIP_HW_TOKEN_HDR_DTLS_CMN;
	*tk_hdr |= EIP_HW_TOKEN_HDR_INBOUND | len;

	/*
	 * Total token words.
	 */
	return (tk_word - tk->words);
}

/*
 * eip_tk_dtls_encauth_gcm()
 *      Fill tokens for dtls outbound traffic.
 */
uint8_t eip_tk_dtls_encauth_gcm(struct eip_tk *tk, struct eip_tk_params *tk_params)
{
	struct eip_tr *tr = tk_params->tr;
	struct sk_buff *skb = eip_req2skb(tk_params->eip_req);
	struct eip_tr_dtls *dtls = &tr->dtls;
	uint32_t *tk_hdr = &tk_params->tk_hdr;

	uint8_t bypass_len = dtls->bypass_len;
	uint8_t digest_len = tr->digest_len;
	uint8_t pkt_type = EIP_DTLS_TYPE_APP_DATA;
	uint32_t *tk_word = tk->words;
	uint16_t len, dlen, clen;
	uint8_t y0_idx;

	y0_idx = bypass_len + sizeof(struct eip_dtls_hdr) + tr->iv_len;
	len = skb->len;
	dlen = len - bypass_len;	/* Payload length to encrypt */
	clen = dlen + tr->iv_len + digest_len; /* DTLS fragment length */

	/*
	 * Fill the encap instructions without ESN
	 */
	*tk_word++ = EIP_TK_INST_BYPASS | bypass_len;		/* Bypass outer headers */
	*tk_word++ = EIP_TK_INST_EPOCH_SEQ_HMAC;		/* Hash epoch & upper 16 bit Sequence number */
	*tk_word++ = EIP_TK_INST_LO_SEQ_HMAC;			/* Hash lower 32bit sequence number */
	*tk_word++ = EIP_TK_INST_TYPE_HMAC_N_OUT | EIP_TK_PKT_TYPE(pkt_type);
								/* Hash type and write to output */
	*tk_word++ = EIP_TK_INST_VER_HMAC_N_OUT;		/* Hash version and write to output */
	*tk_word++ = EIP_TK_INST_EPOCH_SEQ_OUT;			/* Write epoch & seq to output */
	*tk_word++ = EIP_TK_INST_LO_SEQ_OUT;			/* Write lower seq to output */
	*tk_word++ = EIP_TK_INST_DATA_LEN_HMAC | EIP_TK_INST_LAST_BIT;	/* Hash payload length field */
	*tk_word++ = EIP_TK_INST_DATA | htons(dlen);
	*tk_word++ = EIP_TK_INST_FRAG_LEN_OUT;			/* Write data fragment length */
	*tk_word++ = EIP_TK_INST_DATA | htons(clen);
	*tk_word++ = EIP_TK_INST_INS_IV_GCM | tr->iv_len;	/* Write IV to output */
	*tk_word++ = EIP_TK_INST_SCH_RM_Y0_OUT | y0_idx;	/* Schedule removal of y0 */
	*tk_word++ = EIP_TK_INST_ADD_DATA | AES_BLOCK_SIZE;	/* Insert and enc block of zero */
	*tk_word++ = EIP_TK_INST_DATA_ENC_HMAC_OUT | dlen;	/* Encrypt-hash and write payload to output */
	*tk_word++ = EIP_TK_INST_HMAC_ADD | digest_len;		/* Write HMAC tag to output */
	*tk_word++ = EIP_TK_INST_CHK_SEQ_NO_ROLLOVER;			/* Verify seq no rollover */
	*tk_word++ = EIP_TK_INST_SEQ64_NO_UPDT | dtls->seq_offset;	/* Update sequence number */

	/*
	 * Fill token header
	 */
	*tk_hdr |= EIP_HW_TOKEN_HDR_DTLS_CMN | EIP_HW_TOKEN_HDR_IV_PRNG;
	*tk_hdr |= EIP_HW_TOKEN_HDR_OUTBOUND | len;

	/*
	 * Total token words.
	 */
	return (tk_word - tk->words);
}

/*
 * eip_tk_dtls_authdec_gcm()
 *      Fill tokens for dtls inbound traffic.
 */
uint8_t eip_tk_dtls_authdec_gcm(struct eip_tk *tk, struct eip_tk_params *tk_params)
{
	struct eip_tr *tr = tk_params->tr;
	struct sk_buff *skb = eip_req2skb(tk_params->eip_req);
	struct eip_tr_dtls *dtls = &tr->dtls;
	uint32_t *tk_hdr = &tk_params->tk_hdr;
	uint8_t digest_len = tr->digest_len;
	uint32_t *tk_word = tk->words;
	uint16_t len, dlen;

	len = skb->len;
	dlen = len - sizeof(struct eip_dtls_hdr) - tr->iv_len - digest_len;

	*tk_word++ = EIP_TK_INST_TYPE_N_VER_UPDT;	/* Read type & version from input and store in context */
	*tk_word++ = EIP_TK_INST_EPOCH_N_HSEQ_HMAC;	/* Hash epoch and higher sequence from packet */
	*tk_word++ = EIP_TK_INST_LSEQ_HMAC;		/* hash lower sequence from packet */
	*tk_word++ = EIP_TK_INST_FRAG_LEN_REM;		/* Remove dtls frag length from input */
	*tk_word++ = EIP_TK_INST_REM_IV_GCM | tr->iv_len;	/* Remove & Copy Packet IV to context */
	*tk_word++ = EIP_TK_INST_TYPE_N_VER_HMAC;	/* Hash type & version from context stored in previous instrunction */
	*tk_word++ = EIP_TK_INST_DATA_LEN_HMAC | EIP_TK_INST_LAST_BIT;	/* Hash payload length field */
	*tk_word++ = EIP_TK_INST_DATA | htons(dlen);
	*tk_word++ = EIP_TK_INST_SCH_RM_Y0_OUT;			/* Schedule removal of y0 */
	*tk_word++ = EIP_TK_INST_ADD_DATA | AES_BLOCK_SIZE;	/* Insert and enc block of zero */
	*tk_word++ = EIP_TK_INST_HMAC_DEC | dlen;		/* hmac-decrypt payload and write to output */
        *tk_word++ = EIP_TK_INST_HMAC_GET_IPSEC | digest_len;	/* Retrieve ICV */
	*tk_word++ = EIP_TK_INST_SEQ_HMAC_CHK | digest_len;	/* inbound checks */

	if (dtls->seq_offset)
		*tk_word++ = EIP_TK_INST_SEQ_UPDT | dtls->seq_offset;	/* Update sequence number */

	/*
	 * Fill token header
	 */
	*tk_hdr |= EIP_HW_TOKEN_HDR_DTLS_CMN;
	*tk_hdr |= EIP_HW_TOKEN_HDR_INBOUND | len;

	/*
	 * Total token words.
	 */
	return (tk_word - tk->words);
}
