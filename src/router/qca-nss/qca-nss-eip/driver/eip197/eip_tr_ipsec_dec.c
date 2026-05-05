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
#include <linux/slab.h>
#include <linux/completion.h>

#include "eip_priv.h"

/*
 * eip_tr_ipsec_get_replay_words()
 */
static uint32_t eip_tr_ipsec_get_replay_words(enum eip_ipsec_replay replay)
{
	switch (replay) {
		case EIP_IPSEC_REPLAY_NONE:
			return 0;
		case EIP_IPSEC_REPLAY_32:
			return 1;
		case EIP_IPSEC_REPLAY_64:
			return 2;
		case EIP_IPSEC_REPLAY_128:
			return 4;
		case EIP_IPSEC_REPLAY_384:
		default:
			return 12;
	}
}

/*
 * eip_tr_ipsec_get_replay_seq_mask()
 */
static uint32_t eip_tr_ipsec_get_replay_seq_mask(enum eip_ipsec_replay replay)
{
	switch (replay) {
		case EIP_IPSEC_REPLAY_NONE:
			return 0;
		case EIP_IPSEC_REPLAY_32:
			return EIP_TR_IPSEC_SEQ_NUM_MASK_32;
		case EIP_IPSEC_REPLAY_64:
			return EIP_TR_IPSEC_SEQ_NUM_MASK_64;
		case EIP_IPSEC_REPLAY_128:
			return EIP_TR_IPSEC_SEQ_NUM_MASK_128;
		case EIP_IPSEC_REPLAY_384:
		default:
			return EIP_TR_IPSEC_SEQ_NUM_MASK_384;
	}
}

/*
 * eip_tr_ipsec_dec_cmn_init()
 *	Initialize the transform record for ipsec Decapsulation.
 */
void eip_tr_ipsec_dec_cmn_init(struct eip_tr *tr, struct eip_tr_info *info, const struct eip_svc_entry *algo)
{
	struct eip_tr_info_ipsec *ipsec = &info->ipsec;
	struct eip_tr_base *crypto = &info->base;
	uint32_t *crypt_words = tr->hw_words;
	uint32_t *tr_words = tr->hw_words;
	uint8_t seq_num_offst;
	uint8_t mask_sz;
	uint32_t size;

	bool esn = ipsec->sa_flags & EIP_TR_IPSEC_FLAG_ESN;
	bool natt = ipsec->sa_flags & EIP_TR_IPSEC_FLAG_UDP;
	bool tun = ipsec->sa_flags & EIP_TR_IPSEC_FLAG_TUNNEL;
	bool ipv6 = ipsec->sa_flags & EIP_TR_IPSEC_FLAG_IPV6;

	/*
	 * First two words are Control words.
	 */
	tr_words[0] = algo->ctrl_words_0;
	tr_words[1] = algo->ctrl_words_1;

	/*
	 * Enable IPsec specific fields in control words 0.
	 */
	tr_words[0] |= eip_tr_ipsec_get_replay_seq_mask(ipsec->replay);
	tr_words[0] |= EIP_TR_IPSEC_EXT_SEQ_NUM(esn);
	tr_words[1] |= EIP_TR_IPSEC_PAD_TYPE;

	/*
	 * Crypto variable words starts from third words.
	 */
	crypt_words = &tr_words[2];

	/*
	 * Fill cipher key.
	 */
	memcpy(crypt_words, crypto->cipher.key_data, crypto->cipher.key_len);
	crypt_words += (crypto->cipher.key_len /  sizeof(uint32_t));

	/*
	 * Leave the space for inner & outer digest / GHASH key
	 * For GCM we store only GHASH key of size auth_block_len
	 * and for other algo we store inner & outer digest each
	 * of size auth_state_len
	 */
	size = algo->auth_state_len ? (algo->auth_state_len * 2) : algo->auth_block_len;
	crypt_words += (size / sizeof(uint32_t));

	/*
	 * IPsec specific fields for EIP96.
	 * - SPI in Host order (1 word)
	 * - Sequence number (2 words for ESN else 1 word)
	 * - Sequence mask to support window size.
	 */
	*crypt_words++ = ntohl(ipsec->spi_idx);
	*crypt_words++ = 0x0;
	seq_num_offst = crypt_words - tr_words - 1;
	if (esn)
		*crypt_words++ = 0;

	/*
	 * Set all bits to '1' in mask. For 32bit we need to set extra word to '1'.
	 */
	mask_sz = eip_tr_ipsec_get_replay_words(ipsec->replay);
	mask_sz += ipsec->replay == EIP_IPSEC_REPLAY_32 ? 1 : 0;
	while (mask_sz--) {
		*crypt_words++ = 0xFFFFFFFF;
	}

	/*
	 * Fill nonce.
	 */
	if (crypto->nonce)
		*crypt_words++ = crypto->nonce;

	/*
	 * Done with EIP crypto context words.
	 * Update relative information in control words.
	 */
	tr_words[0] |= EIP_TR_CTRL_CONTEXT_WORDS(crypt_words - tr_words - 2);

	/*
	 * Configure IPsec decapsulation specfic field in TR.
	 */
	tr_words[64] = tun ? EIP_TR_IPSEC_DECAP_TUNNEL_TOKEN_HDR : EIP_TR_IPSEC_DECAP_TRANSPORT_TOKEN_HDR;

	tr_words[65] = EIP_TR_IPSEC_IPHDR_PROC;
	tr_words[65] |= EIP_TR_IPSEC_EXT_SEQ_NUM_PROC(esn);
	if (ipv6) {
		tr_words[65] |= EIP_TR_IPSEC_IPV6_EN;
	}

	/*
	 * Redirect packet to inline chanel after encapsulation.
	 * TODO: Round robin ring
	 */
	if (tr->svc == EIP_SVC_HYBRID_IPSEC) {
		struct eip_ctx *ctx = tr->ctx;
		tr_words[65] |= EIP_TR_REDIR_EN;
		tr_words[65] |= EIP_TR_REDIR_IFACE(ctx->dma[1].ring_id);;
		pr_debug("%px: Redirection enable to ring(%u) for decap record\n", tr, ctx->dma[0].ring_id);
	}

	tr_words[66] = EIP_TR_IPSEC_TTL(ipsec->ip_ttl);
	tr_words[66]|= (algo->cipher_blk_len / 2);

	tr_words[68] = EIP_TR_IPSEC_OHDR_PROTO(eip_tr_ipsec_get_ohdr(ipsec->sa_flags));
	tr_words[68] |= EIP_TR_IPSEC_ICV_SIZE(ipsec->icv_len);
	tr_words[68] |= EIP_TR_IPSEC_IV_SIZE(algo->iv_len);

	/*
	 * Configure outer UDP port information if natt is enabled.
	 */
	if (natt) {
		tr_words[69] = ipsec->src_port;
		tr_words[69] |= (ipsec->dst_port << 16);
	}

	/*
	 * token verify instruction
	 * Enable Sequence number verification if window is enabled
	 */
	tr_words[70] = EIP_TR_IPSEC_DECAP_TOKEN_VERIFY;
	tr_words[70] |= EIP_TR_IPSEC_DECAP_TOKEN_VERIFY_PAD;
	tr_words[70] |= EIP_TR_IPSEC_DECAP_TOKEN_VERIFY_HMAC | ipsec->icv_len;
	tr_words[70] |= ipsec->replay ? EIP_TR_IPSEC_DECAP_TOKEN_VERIFY_SEQ : 0;

	/*
	 * token context instruction
	 * append sequence number location
	 */
	tr_words[71] = esn ? EIP_TR_IPSEC_DECAP_ESN_TOKEN_INST : EIP_TR_IPSEC_DECAP_TOKEN_INST;
	tr_words[71] |= seq_num_offst;

	/*
	 * Update instruction with number of words to hold window and sequence number.
	 */
	if (ipsec->replay != EIP_IPSEC_REPLAY_NONE) {
		uint8_t seq_words = esn + 1 + eip_tr_ipsec_get_replay_words(ipsec->replay);
		tr_words[71] |= EIP_TR_IPSEC_DECAP_TOKEN_INST_SEQ_UPDATE(seq_words);
	}
}

/*
 * eip_tr_ipsec_dec()
 *	IPsec Decapsulation.
 */
int eip_tr_ipsec_dec(struct eip_tr *tr, struct sk_buff *skb)
{
	int (*dma_tx)(struct eip_dma *dma, struct eip_sw_desc *sw, struct sk_buff *skb);
	struct eip_ctx *ctx = tr->ctx;
	struct eip_sw_desc *sw;
	struct eip_dma *dma;
	int status = 0;

	dma_tx = skb_is_nonlinear(skb) ? eip_dma_hy_tx_nonlinear_skb : eip_dma_hy_tx_linear_skb;

	/*
	 * Allocate SW descriptor.
	 */
	sw = kmem_cache_alloc(ctx->sw_cache, GFP_NOWAIT | __GFP_NOWARN);
	if (!sw) {
		pr_err("%px: Failed to allocate SW descriptor.\n", tr);
		return -ENOMEM;
	}

	/*
	 * Fill software descriptor.
	 * Dereference: eip_tr_ipsec_tx_done
	 */
	sw->tr = eip_tr_ref(tr);
	sw->tk = NULL;
	sw->tk_hdr = EIP_HW_TOKEN_HDR_EXTENDED;
	sw->tk_addr = 0;
	sw->tr_addr_type = tr->tr_addr_type;
	sw->tk_words = 0;
	sw->hw_svc = EIP_HW_CMD_HWSERVICE_LIP;
	sw->req = skb;

	/*
	 * Push the fake mac header for inline.
	 * Set handler to trasmit completion as packet will be redirected to inline.
	 */
	skb_push(skb, sizeof(struct ethhdr));
	sw->tk_hdr |= skb->len;
	sw->comp = &eip_tr_ipsec_tx_done;
	sw->err_comp = NULL; /* There is no error in Transmit completion */

	dma = &ctx->dma[smp_processor_id()];

	status = dma_tx(dma, sw, skb);
	if (status < 0) {
		dma->stats.tx_error++;
		goto fail_tx;
	}

	return 0;

fail_tx:
	eip_tr_deref(tr);
	kmem_cache_free(ctx->sw_cache, sw);
	return status;
}
EXPORT_SYMBOL(eip_tr_ipsec_dec);
