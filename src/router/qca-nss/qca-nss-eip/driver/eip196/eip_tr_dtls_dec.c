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
#include <linux/slab.h>
#include <linux/completion.h>
#include <net/dsfield.h>

#include "eip_priv.h"

/*
 * eip_tr_dtls_dec_err()
 *	Hardware error callback API for Decapsulation.
 */
void eip_tr_dtls_dec_err(struct eip_tr *tr, struct eip_hw_desc *hw, struct eip_sw_desc *sw, uint16_t cle_err, uint16_t tr_err)
{
	struct eip_tr_dtls *dtls = &tr->dtls;
	struct eip_ctx *ctx = tr->ctx;
	int err;

	/*
	 * Call the client callback.
	 */
	err = ((tr_err << EIP_TR_ERR_SHIFT) | cle_err);
	dtls->ops.err_cb(dtls->ops.app_data, sw->req, err);

	kmem_cache_free(ctx->tk_cache, sw->tk);
	kmem_cache_free(ctx->sw_cache, sw);
	eip_tr_deref(tr);
}

/*
 * eip_tr_dtls_dec_done()
 *	Hardware completion callback API for Decapsulation.
 */
void eip_tr_dtls_dec_done(struct eip_tr *tr, struct eip_hw_desc *hw, struct eip_sw_desc *sw)
{
	uint16_t out_len = EIP_HW_RES_DATA_LEN(hw->token[0]);
	struct eip_tr_dtls *dtls = &tr->dtls;
	struct eip_ctx *ctx = tr->ctx;
	struct sk_buff *skb;

	/*
	 * After successful Transformation SKB data length needs to be decreased.
	 */
	skb = eip_req2skb(sw->req);
	pskb_trim(skb, out_len);
	skb_reset_transport_header(skb);

	/*
	 * Call the client callback.
	 */
	dtls->ops.cb(dtls->ops.app_data, sw->req);

	kmem_cache_free(ctx->tk_cache, sw->tk);
	kmem_cache_free(ctx->sw_cache, sw);
	eip_tr_deref(tr);
}

/*
 * eip_tr_dtls_dec()
 *	DTLS Decapsulation.
 */
int eip_tr_dtls_dec(struct eip_tr *tr, struct sk_buff *skb)
{
	int (*dma_tx)(struct eip_dma *dma, struct eip_sw_desc *sw, struct sk_buff *skb);
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
	 * Allocate SW descriptor.
	 */
	sw = kmem_cache_alloc(ctx->sw_cache, GFP_NOWAIT | __GFP_NOWARN);
	if (!sw) {
		pr_err("%px: Failed to allocate SW descriptor.\n", tr);
		kmem_cache_free(ctx->tk_cache, tk);
		return -ENOMEM;
	}

	/*
	 * CAPWAP needs outer TOS value.
	 * Store it in SKB before removing header.
	 */
	skb->priority = (skb->protocol == htons(ETH_P_IP)) ? ip_hdr(skb)->tos : ipv6_get_dsfield(ipv6_hdr(skb));
	skb_pull(skb, tr->dtls.remove_len);

	/*
	 * Fill TK params
	 */
	tk_params.tr = tr;
	tk_params.eip_req = (eip_req_t)skb;

	/*
	 * Fill token for decryption and hash for DTLS
	 */
	tk_words = eip_tr_fill_token(tk, &tr->dtls.ops, &tk_params);
	WARN_ON(tk_words > sizeof(*tk));
	eip_dmac_clean_range(tk, tk + 1);

	dma_tx = skb_is_nonlinear(skb) ? eip_dma_tx_nonlinear_skb : eip_dma_tx_linear_skb;

	/*
	 * Fill software descriptor.
	 * Dereference: eip_tr_dtls_dec_done() / eip_tr_dtls_dec_err()
	 */
	sw->tr = eip_tr_ref(tr);

	sw->tk = tk;
	sw->tk_hdr = tk_params.tk_hdr;
	sw->tk_words = tk_words;
	sw->tk_addr = virt_to_phys(tk);
	sw->tr_addr_type = tr->tr_addr_type;

	sw->req = skb;
	sw->hw_svc = EIP_HW_CMD_HWSERVICE_LAC;

	sw->comp = eip_tr_dtls_dec_done;
	sw->err_comp = eip_tr_dtls_dec_err;

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
	kmem_cache_free(ctx->tk_cache, tk);
	return status;
}
EXPORT_SYMBOL(eip_tr_dtls_dec);

/*
 * eip_tr_dtls_get_replay_words()
 */
static uint32_t eip_tr_dtls_get_replay_words(enum eip_dtls_replay replay)
{
	switch (replay) {
	case EIP_DTLS_REPLAY_NONE:
		return 0;
	case EIP_DTLS_REPLAY_64:
		return 2;
	case EIP_DTLS_REPLAY_128:
	default:
		return 4;
	}
}

/*
 * eip_tr_dtls_get_replay_seq_mask()
 */
static uint32_t eip_tr_dtls_get_replay_seq_mask(enum eip_dtls_replay replay)
{
	switch (replay) {
	case EIP_DTLS_REPLAY_NONE:
		return 0;
	case EIP_DTLS_REPLAY_64:
		return EIP_TR_DTLS_CTX0_SEQNO_MASK64;
	case EIP_DTLS_REPLAY_128:
	default:
		return EIP_TR_DTLS_CTX0_SEQNO_MASK128;
	}
}

/*
 * eip_tr_dtls_dec_cmn_init()
 *      Initialize the transform record for dtls Decapsulation.
 */
void eip_tr_dtls_dec_cmn_init(struct eip_tr *tr, struct eip_tr_info *info, const struct eip_svc_entry *algo)
{
	struct eip_tr_info_dtls *dtls = &info->dtls;
	struct eip_tr_dtls *tr_dtls = &tr->dtls;
	struct eip_tr_base *crypto = &info->base;
	uint32_t *crypt_words = tr->hw_words;
	uint32_t *tr_words = tr->hw_words;
	uint8_t seq_offset = 0;
	uint8_t mask_sz;
	uint32_t size;
	uint32_t mode;


	tr->dtls.ops.tk_fill = algo->dec_tk_fill;

	/*
	 * First two words are Control words.
	 */
	tr_words[0] = algo->ctrl_words_0;
	tr_words[1] = algo->ctrl_words_1;

	/*
	 * Enable DTLS Decap specific fields in control words.
	 */
	tr_words[0] |= EIP_TK_CTRL_OP_DEC_HMAC;
	tr_words[0] |= eip_tr_dtls_get_replay_seq_mask(dtls->replay);
	tr_words[1] |= EIP_TR_DTLS_CTX1_PRE_CRYPTO_DECAP;

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
	 * Leave the space for inner & outer digest / GHASH key.
	 * For GCM we store only GHASH key of size auth_block_len
	 * and for other algo we store inner & outer digest each
	 * of size auth_state_len.
	 */
	size = algo->auth_state_len ? (algo->auth_state_len * 2) : algo->auth_block_len;
	crypt_words += (size / sizeof(uint32_t));

	/*
	 * DTLS specific fields for EIP96.
	 */
	*crypt_words++ = EIP_TR_DTLS_VERSION(dtls->version);	/* Version */
	*crypt_words++ = 0x0;	/* Sequence number */
	*crypt_words++ = EIP_TR_DTLS_EPOCH(ntohs(dtls->epoch)); /* b0-15 is Sequence & B16-31 is epoch */

	/*
	 * Sequence update should not be done when window check is disabled.
	 */
	if (dtls->replay != EIP_DTLS_REPLAY_NONE)
		seq_offset = crypt_words - tr_words - 2;

	/*
	 * Set all bits to '1' in mask.
	 */
	mask_sz = eip_tr_dtls_get_replay_words(dtls->replay);
	while (mask_sz--)
		*crypt_words++ = 0xFFFFFFFF;

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
	 * Store the seq no offset, icv length and cipher block length
	 * to be used during token fill
	 */
	tr_dtls->seq_offset = seq_offset;
	tr->digest_len = algo->auth_digest_len;
	tr->blk_len = algo->cipher_blk_len;
	tr->iv_len = algo->iv_len;

	/*
	 * Assign the pre & post procesing callback for IP header
	 */
	tr_dtls->ops.pre = NULL;
	tr_dtls->ops.post = NULL;
	tr_dtls->bypass_len = 0;
	tr_dtls->remove_len = 0;

	mode = dtls->flags & (EIP_TR_DTLS_FLAG_UDPLITE | EIP_TR_DTLS_FLAG_CAPWAP | EIP_TR_DTLS_FLAG_IPV6);

	switch (mode) {
	case (EIP_TR_DTLS_FLAG_IPV6):
	case (EIP_TR_DTLS_FLAG_UDPLITE):
	case (EIP_TR_DTLS_FLAG_UDPLITE | EIP_TR_DTLS_FLAG_IPV6):
		WARN_ON(1);
		break;

	case (EIP_TR_DTLS_FLAG_CAPWAP | EIP_TR_DTLS_FLAG_IPV6 | EIP_TR_DTLS_FLAG_UDPLITE):
		tr_dtls->remove_len = sizeof(struct ipv6hdr) + sizeof(struct udphdr) + sizeof(struct eip_capwap_hdr);
		break;

	case (EIP_TR_DTLS_FLAG_CAPWAP | EIP_TR_DTLS_FLAG_IPV6):
		tr_dtls->remove_len = sizeof(struct ipv6hdr) + sizeof(struct udphdr) + sizeof(struct eip_capwap_hdr);
		break;

	case (EIP_TR_DTLS_FLAG_CAPWAP | EIP_TR_DTLS_FLAG_UDPLITE):
		tr_dtls->remove_len = sizeof(struct iphdr) + sizeof(struct udphdr) + sizeof(struct eip_capwap_hdr);
		break;

	case (EIP_TR_DTLS_FLAG_CAPWAP):	/* DTLS-CAPWAP IPv4 tunnel */
		tr_dtls->remove_len = sizeof(struct iphdr) + sizeof(struct udphdr) + sizeof(struct eip_capwap_hdr);
		break;

	default:	/* DTLS IPv4 tunnel */
		tr_dtls->remove_len = sizeof(struct iphdr) + sizeof(struct udphdr);	/* Headers to be removed by software */
		break;
	}
}
