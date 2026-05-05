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
 * eip_tk_ipsec_encauth_cbc()
 *	Fill tokens for ipsec outbound traffic.
 */
uint8_t eip_tk_ipsec_encauth_cbc(struct eip_tk *tk, struct eip_tk_params *tk_params)
{
	struct sk_buff *skb = eip_req2skb(tk_params->eip_req);
	struct eip_tr *tr = tk_params->tr;
	struct eip_tr_ipsec *ipsec = &tr->ipsec;
	uint32_t *tk_hdr = &tk_params->tk_hdr;
	uint32_t *tk_word = tk->words;
	uint8_t bypass_len = ipsec->bypass_len;
	uint16_t auth_len = sizeof(struct ip_esp_hdr) + tr->iv_len;
	uint16_t data_len = skb->len;
	uint8_t pad_len = EIP_TR_IPSEC_SKB_CB(skb)->pad_len;
	uint8_t proto = EIP_TR_IPSEC_SKB_CB(skb)->ip_proto;
	uint8_t digest_len = tr->digest_len;

	/*
	 * Fill the encap instructions without ESN
	 */
	*tk_word++ = EIP_TK_INST_BYPASS | bypass_len;				/* Bypass outer header, IP / (IP+UDP) */
	*tk_word++ = EIP_TK_INST_ESP_HDR_HMAC | auth_len;			/* Pure hash, ESP hdr & IV added by EIP196 */
	*tk_word++ = EIP_TK_INST_ENC_HMAC_IPSEC | (data_len - bypass_len);	/* Encryption + Hash */
	*tk_word++ = EIP_TK_INST_PAD_ENC_HMAC | \
				 EIP_TK_IPSEC_ESP_NXT_HDR(proto) | \
				 (pad_len + EIP_TK_IPSEC_ESP_TRAILER_LEN);			/* Add trailer and ecrypt */
	*tk_word++ = EIP_TK_INST_HMAC_ADD | digest_len;                         /* Insert generated hash */
	*tk_word++ = EIP_TK_INST_CHK_SEQ_NO_ROLLOVER;				/* Verify seq no rollover */
	*tk_word++ = EIP_TK_INST_SEQ_NO_AND_MASK_UPDT |				/* TODO: modify origin to support ESN */
		EIP_TK_INST_CONTEXT_ACCESS_LEN(ipsec->ctx_num_words) |
		(ipsec->seq_offset);       					/* Update seq no in context record */

	/*
	 * Fill token header
	 */
	*tk_hdr |= EIP_HW_TOKEN_HDR_IPSEC_CMN | EIP_HW_TOKEN_HDR_IV_PRNG;
	*tk_hdr |= EIP_HW_TOKEN_HDR_OUTBOUND | data_len;

	/*
	 * Total token words.
	 */
	return (tk_word - tk->words);
}
EXPORT_SYMBOL(eip_tk_ipsec_encauth_cbc);

/*
 * eip_tk_ipsec_authdec_cbc()
 *      Fill tokens for ipsec inbound traffic.
 */
uint8_t eip_tk_ipsec_authdec_cbc(struct eip_tk *tk, struct eip_tk_params *tk_params)
{
	struct sk_buff *skb = eip_req2skb(tk_params->eip_req);
	struct eip_tr *tr = tk_params->tr;
	struct eip_tr_ipsec *ipsec = &tr->ipsec;
	uint32_t *tk_hdr = &tk_params->tk_hdr;
	uint32_t *tk_word = tk->words;
	uint8_t bypass_len = ipsec->bypass_len;
	uint16_t auth_len = sizeof(struct ip_esp_hdr) + tr->iv_len;
	uint32_t* iv_word = (uint32_t*)&(skb->data[auth_len]);
	uint16_t data_len = skb->len;
	uint8_t digest_len = tr->digest_len;
	uint16_t cipher_len = data_len - bypass_len - auth_len - digest_len;

	/*
	 * We construct IV here
	 */
	*tk_word++ = iv_word[0];
	*tk_word++ = iv_word[1];
	*tk_word++ = iv_word[2];
	*tk_word++ = iv_word[3];

	/*
	 * Fill the decapsulation instructions without ESN
	 */
	*tk_word++ = EIP_TK_INST_BYPASS | bypass_len;			/* Tun: no bypass, Trans:IP hdr */
	*tk_word++ = EIP_TK_INST_ESP_HDR_CHK | auth_len;      		/* Pure hash */
	*tk_word++ = EIP_TK_INST_HMAC_DEC | cipher_len;			/* Hash + Decrypt */
	*tk_word++ = EIP_TK_INST_HMAC_GET_IPSEC | digest_len;		/* Fetch hash */
	*tk_word++ = EIP_TK_INST_ICV_SPI_SEQ_NO_CHK | digest_len;	/* Verify hash, SPI, seq no */
	*tk_word++ = EIP_TK_INST_SEQ_NO_AND_MASK_UPDT |			/* TODO: modify origin to support ESN */
		EIP_TK_INST_CONTEXT_ACCESS_LEN(ipsec->ctx_num_words) |
		ipsec->seq_offset;					/* Update seq no and mask in context record */

	/*
	 * Fill token header
	 */
	*tk_hdr |= EIP_HW_TOKEN_HDR_IPSEC_IV16;
	*tk_hdr |= EIP_HW_TOKEN_HDR_INBOUND | data_len;

	/*
	 * Total token words.
	 */
        return (tk_word - tk->words);
}
EXPORT_SYMBOL(eip_tk_ipsec_authdec_cbc);

/*
 * eip_tk_ipsec_encauth_gcm_rfc()
 *      Fill tokens for ipsec outbound traffic.
 */
uint8_t eip_tk_ipsec_encauth_gcm_rfc(struct eip_tk *tk, struct eip_tk_params *tk_params)
{
	struct sk_buff *skb = eip_req2skb(tk_params->eip_req);
	struct eip_tr *tr = tk_params->tr;
	struct eip_tr_ipsec *ipsec = &tr->ipsec;
	uint32_t *tk_hdr = &tk_params->tk_hdr;
	uint32_t *tk_word = tk->words;
	uint8_t bypass_len = ipsec->bypass_len;
	uint16_t auth_len = sizeof(struct ip_esp_hdr);
	uint16_t iv_len = tr->iv_len;
	uint16_t data_len = skb->len;
	uint8_t y0_idx = bypass_len + auth_len + iv_len;
	uint8_t pad_len = EIP_TR_IPSEC_SKB_CB(skb)->pad_len;
	uint8_t proto = EIP_TR_IPSEC_SKB_CB(skb)->ip_proto;
	uint16_t digest_len = tr->digest_len;

	/*
	 * Fill the encap instructions without ESN
	 */
	*tk_word++ = EIP_TK_INST_BYPASS | bypass_len;                           /* Bypass outer header, IP / (IP+UDP) */
	*tk_word++ = EIP_TK_INST_ESP_HDR_GCM | auth_len;			/* Insert ESP header and authenticate */
	*tk_word++ = EIP_TK_INST_INS_IV_GCM | iv_len;				/* Insert IV without authentication */
	*tk_word++ = EIP_TK_INST_SCH_RM_Y0_OUT | y0_idx;			/* Remove Y0 */
	*tk_word++ = EIP_TK_INST_ADD_DATA | AES_BLOCK_SIZE;			/* Insert and enc block of zero */
	*tk_word++ = EIP_TK_INST_ENC_HMAC_IPSEC | (data_len - bypass_len);	/* Encryption and hash */
	*tk_word++ = EIP_TK_INST_PAD_ENC_HMAC |
		EIP_TK_IPSEC_ESP_NXT_HDR(proto) |
		(pad_len + EIP_TK_IPSEC_ESP_TRAILER_LEN);			/* Insert ESP pad & trailer, enc+auth */
	*tk_word++ = EIP_TK_INST_HMAC_ADD | digest_len;				/* Insert ICV */
	*tk_word++ = EIP_TK_INST_DEC_CHK_PAD;                                   /* Verify seq no rollover */
	*tk_word++ = EIP_TK_INST_SEQ_NO_AND_MASK_UPDT |				/* TODO: modify origin to support ESN */
		EIP_TK_INST_CONTEXT_ACCESS_LEN(ipsec->ctx_num_words) |
		ipsec->seq_offset;						/* Update seq no in context record */

	/*
	 * Fill token control data.
	 */
	*tk_hdr |= EIP_HW_TOKEN_HDR_IPSEC_CMN | EIP_HW_TOKEN_HDR_IV_PRNG;
	*tk_hdr |= (EIP_HW_TOKEN_HDR_OUTBOUND | data_len);

	/*
	 * Total token words.
	 */
	return (tk_word - tk->words);
}
EXPORT_SYMBOL(eip_tk_ipsec_encauth_gcm_rfc);

/*
 * eip_tk_ipsec_authdec_gcm_rfc()
 *      Fill tokens for ipsec inbound traffic.
 */
uint8_t eip_tk_ipsec_authdec_gcm_rfc(struct eip_tk *tk, struct eip_tk_params *tk_params)
{
	struct sk_buff *skb = eip_req2skb(tk_params->eip_req);
	struct eip_tr *tr = tk_params->tr;
	struct eip_tr_ipsec *ipsec = &tr->ipsec;
	uint32_t *tk_hdr = &tk_params->tk_hdr;
	uint32_t *tk_word = tk->words;
	uint8_t bypass_len = ipsec->bypass_len;
	uint16_t auth_len = sizeof(struct ip_esp_hdr);
	uint16_t iv_len = tr->iv_len;
	uint16_t data_len = skb->len;
	uint8_t digest_len = tr->digest_len;
	uint16_t cipher_len = data_len - bypass_len - auth_len - iv_len - digest_len;

	/*
	 * Fill the decapsulation instructions without ESN
	 */
	*tk_word++ = EIP_TK_INST_BYPASS | bypass_len;                           /* Tun: no bypass, Trans:IP hdr */
	*tk_word++ = EIP_TK_INST_ESP_HDR_CHK_GCM | auth_len;			/* Authenticate and remove ESP header */
	*tk_word++ = EIP_TK_INST_REM_IV_GCM | iv_len;				/* Retrieve IV */
	*tk_word++ = EIP_TK_INST_SCH_RM_Y0_OUT | bypass_len;			/* Remove Y0 */
	*tk_word++ = EIP_TK_INST_ADD_DATA | AES_BLOCK_SIZE;			/* Insert and enc block of zero */
	*tk_word++ = EIP_TK_INST_HMAC_DEC | cipher_len;				/* Hash + Decrypt */
	*tk_word++ = EIP_TK_INST_HMAC_GET_IPSEC | digest_len;			/* Retrieve ICV */
	*tk_word++ = EIP_TK_INST_ICV_SPI_SEQ_NO_CHK | digest_len;		/* Verify ICV, SPI, seq no */
	*tk_word++ = EIP_TK_INST_SEQ_NO_AND_MASK_UPDT |				/* TODO: modify origin to support ESN */
		EIP_TK_INST_CONTEXT_ACCESS_LEN(ipsec->ctx_num_words) |
		ipsec->seq_offset;						/* Update seq no and mask in context record */

	/*
	 * Fill token header
	 */
	*tk_hdr |= EIP_HW_TOKEN_HDR_IPSEC_CMN;
	*tk_hdr |= EIP_HW_TOKEN_HDR_INBOUND | data_len;

	/*
	 * Total token words.
	 */
	return (tk_word - tk->words);
}
EXPORT_SYMBOL(eip_tk_ipsec_authdec_gcm_rfc);
