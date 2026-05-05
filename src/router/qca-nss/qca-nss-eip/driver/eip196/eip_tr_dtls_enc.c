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
#include <linux/slab.h>
#include <linux/completion.h>
#include <net/ip.h>
#include <net/dsfield.h>

#include "eip_priv.h"

/*
 * eip_tr_dtls_enc_len()
 *	Calculates the total encap length aligned to crypto block length.
 */
static inline uint16_t eip_tr_dtls_enc_len(struct eip_tr *tr, struct sk_buff *skb)
{
	uint16_t payload_len = skb->len;
	uint16_t actual_len;
	uint16_t total_len;

	/*
	 * No padding required for cipher with block length 1.
	 */
	if (tr->blk_len == 1) {
		total_len = payload_len + tr->digest_len;
		return total_len;
	}

	actual_len = payload_len + tr->digest_len + EIP_TR_DTLS_TRAILER;
	total_len = ALIGN(actual_len, tr->blk_len);
	EIP_TR_DTLS_SKB_CB(skb)->pad_len = total_len - actual_len + EIP_TR_DTLS_TRAILER;

	return total_len;
}

/*
 * eip_tr_dtls_enc_udp()
 *	Encapsulate UDP header.
 */
static inline void eip_tr_dtls_enc_udp(struct eip_tr *tr, struct sk_buff *skb, __be16 len)
{
	struct eip_tr_dtls *dtls = &tr->dtls;
	struct udphdr *udph;
	bool is_udplite;

	/*
	 * UDPlite checksum will be calculated by EDMA.
	 */
	is_udplite = (dtls->protocol != IPPROTO_UDP);
	if (is_udplite) {
		len = (tr->tr_flags & EIP_TR_DTLS_FLAG_UDPLITE_CSUM) ? htons(sizeof(struct udphdr)) : 0;
	}

	/*
	 * insert UDP header.
	 */
	udph = (struct udphdr *)skb_push(skb, sizeof(struct udphdr));
	skb_reset_transport_header(skb);

	udph->source = dtls->src_port;
	udph->dest = dtls->dst_port;
	udph->len = len;
	udph->check = 0;
	skb->ip_summed = CHECKSUM_PARTIAL;
}

/*
 * eip_tr_dtls_enc_v4()
 *	Outer IPv4 header addition before encapsulation.
 */
static void eip_tr_dtls_enc_v4(struct eip_tr *tr, struct sk_buff *skb)
{
	bool capwap = !!(tr->tr_flags | EIP_TR_DTLS_FLAG_CAPWAP);
	struct eip_tr_dtls *dtls = &tr->dtls;
	struct eip_ctx *ctx = tr->ctx;
	uint16_t payload_len;
	struct iphdr *iph;
	uint16_t udp_len;
	__be16 tos;

	/*
	 * Calculate payload length including encrypted data length and fixed encapsulation header length.
	 * Pad length is update in SKB CB for later use.
	 */
	payload_len = eip_tr_dtls_enc_len(tr, skb) + dtls->fixed_len;
	tos = (tr->tr_flags & EIP_TR_DTLS_FLAG_CP_TOS) ? skb->priority : dtls->tos;

	if (likely(capwap)) {
		struct eip_capwap_hdr *capwap_hdr = (struct eip_capwap_hdr *)skb_push(skb, sizeof(*capwap_hdr));

		capwap_hdr->preamble = 0x1;
		capwap_hdr->reseved[0] = capwap_hdr->reseved[1] = capwap_hdr->reseved[2] = 0;
	}

	/*
	 * insert UDP header.
	 */
	udp_len = payload_len - sizeof(struct iphdr);
	eip_tr_dtls_enc_udp(tr, skb, htons(udp_len));

	/*
	 * Insert IPv4 header.
	 */
	iph = (struct iphdr *)skb_push(skb, sizeof(struct iphdr));
	skb_reset_network_header(skb);
	skb_set_transport_header(skb, sizeof(struct iphdr));
	skb->protocol = htons(ETH_P_IP);

	iph->version = IPVERSION;
	iph->ihl = sizeof(struct iphdr) >> 2;
	iph->tos = tos;
	iph->tot_len = htons(payload_len);
	iph->id = htons(ctx->ip_id++);
	iph->frag_off = dtls->ip_df;
	iph->ttl = dtls->ttl;
	iph->protocol = dtls->protocol;
	iph->saddr = dtls->src_ip[0];
	iph->daddr = dtls->dst_ip[0];

	/*
	 * Update the header checksum
	 */
	iph->check = 0;
	iph->check = ip_fast_csum((unsigned char *)iph, iph->ihl);
}

/*
 * eip_tr_dtls_enc_v6()
 *	Outer IPv6 header addition before encapsulation.
 */
static void eip_tr_dtls_enc_v6(struct eip_tr *tr, struct sk_buff *skb)
{
	bool capwap = !!(tr->tr_flags | EIP_TR_DTLS_FLAG_CAPWAP);
	struct eip_tr_dtls *dtls = &tr->dtls;
	uint16_t payload_len;
	struct ipv6hdr *ip6h;
	uint16_t udp_len;
	__be16 tos;

	/*
	 * Calculate payload length including encrypted data length and fixed encapsulation header length.
	 * Pad length is update in SKB CB for later use.
	 */
	payload_len = eip_tr_dtls_enc_len(tr, skb) + dtls->fixed_len;
	tos = (tr->tr_flags & EIP_TR_DTLS_FLAG_CP_TOS) ? skb->priority : dtls->tos;

	if (likely(capwap)) {
		struct eip_capwap_hdr *capwap = (struct eip_capwap_hdr *)skb_push(skb, sizeof(struct eip_capwap_hdr));

		capwap->preamble = 0x1;
		capwap->reseved[0] = capwap->reseved[1] = capwap->reseved[2] = 0;
	}

	/*
	 * Insert UDP header.
	 */
	udp_len = payload_len - sizeof(struct ipv6hdr);
	eip_tr_dtls_enc_udp(tr, skb, htons(udp_len));

	/*
	 * Insert IPv6 header.
	 */
	ip6h = (struct ipv6hdr *)skb_push(skb, sizeof(struct ipv6hdr));
	skb_reset_network_header(skb);
	skb->protocol = htons(ETH_P_IPV6);

	ip6_flow_hdr(ip6h, tos, 0);
	ip6h->payload_len = htons(udp_len);
	ip6h->nexthdr = dtls->protocol;
	ip6h->hop_limit = dtls->ttl;

	memcpy(ip6h->saddr.s6_addr32, dtls->src_ip, sizeof(ip6h->saddr.s6_addr32));
	memcpy(ip6h->daddr.s6_addr32, dtls->dst_ip, sizeof(ip6h->daddr.s6_addr32));
}

/*
 * eip_tr_dtls_enc_err()
 *	Hardware error callback API for Encapsulation.
 */
void eip_tr_dtls_enc_err(struct eip_tr *tr, struct eip_hw_desc *hw, struct eip_sw_desc *sw, uint16_t cle_err, uint16_t tr_err)
{
	struct eip_tr_dtls *dtls = &tr->dtls;
	struct eip_ctx *ctx = tr->ctx;
	uint32_t err;

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
 * eip_tr_dtls_enc_done()
 *	Hardware completion callback API for Encapsulation.
 */
void eip_tr_dtls_enc_done(struct eip_tr *tr, struct eip_hw_desc *hw, struct eip_sw_desc *sw)
{
	uint16_t out_len = EIP_HW_RES_DATA_LEN(hw->token[0]);
	struct eip_tr_dtls *dtls = &tr->dtls;
	struct eip_ctx *ctx = tr->ctx;
	struct sk_buff *skb;

	/*
	 * After successful encapsulation SKB data length needs to be increased.
	 */
	skb = eip_req2skb(sw->req);
	skb_put(skb, out_len - skb->len);

	/*
	 * Call the client callback.
	 */
	dtls->ops.cb(dtls->ops.app_data, sw->req);

	kmem_cache_free(ctx->tk_cache, sw->tk);
	kmem_cache_free(ctx->sw_cache, sw);
	eip_tr_deref(tr);
}

/*
 * eip_tr_dtls_enc()
 *	DTLS Encapsulation.
 */
int eip_tr_dtls_enc(struct eip_tr *tr, struct sk_buff *skb)
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
		pr_debug("%px: Failed to allocate token.\n", tr);
		return -ENOMEM;
	}

	/*
	 * Allocate SW descriptor.
	 */
	sw = kmem_cache_alloc(ctx->sw_cache, GFP_NOWAIT | __GFP_NOWARN);
	if (!sw) {
		pr_debug("%px: Failed to allocate SW descriptor.\n", tr);
		kmem_cache_free(ctx->tk_cache, tk);
		return -ENOMEM;
	}

	EIP_TR_PRE_PROCESS(tr, &tr->dtls.ops, skb);

	/*
	 * Fill TK params
	 */
	tk_params.tr = tr;
	tk_params.eip_req = (eip_req_t)skb;

	/*
	 * Fill DTLS encapsulation token for hardware.
	 */
	tk_words = eip_tr_fill_token(tk, &tr->dtls.ops, &tk_params);
	WARN_ON(tk_words > sizeof(*tk));
	eip_dmac_clean_range(tk, tk + 1);

	dma_tx = skb_is_nonlinear(skb) ? eip_dma_tx_nonlinear_skb : eip_dma_tx_linear_skb;

	/*
	 * Fill software descriptor.
	 * Dereference: eip_tr_dtls_enc_done() / eip_tr_dtls_enc_err()
	 */
	sw->tr = eip_tr_ref(tr);

	sw->tk = tk;
	sw->tk_hdr = tk_params.tk_hdr;
	sw->tk_words = tk_words;
	sw->tk_addr = virt_to_phys(tk);
	sw->tr_addr_type = tr->tr_addr_type;

	sw->req = skb;
	sw->hw_svc = EIP_HW_CMD_HWSERVICE_LAC;
	sw->comp = eip_tr_dtls_enc_done;
	sw->err_comp = eip_tr_dtls_enc_err;

	dma = &ctx->dma[smp_processor_id()];

	status = dma_tx(dma, sw, skb);
	if (status < 0) {
		dma->stats.tx_error++;
		goto fail_tx; /* FIXME: For DTLS we need to enqueue packet back */
	}

	return 0;

fail_tx:
	eip_tr_deref(tr);
	kmem_cache_free(ctx->sw_cache, sw);
	kmem_cache_free(ctx->tk_cache, tk);
	return status;
}
EXPORT_SYMBOL(eip_tr_dtls_enc);

/*
 * eip_tr_dtls_enc_cmn_init()
 *	Initialize the transform record for DTLS encapsulation.
 */
void eip_tr_dtls_enc_cmn_init(struct eip_tr *tr, struct eip_tr_info *info, const struct eip_svc_entry *algo)
{
	struct eip_tr_info_dtls *dtls = &info->dtls;
	struct eip_tr_dtls *tr_dtls = &tr->dtls;
	struct eip_tr_base *crypto = &info->base;
	uint32_t *crypt_words = tr->hw_words;
	uint32_t *tr_words = tr->hw_words;
	uint8_t bypass_len = 0;
	uint8_t seq_offset;
	uint8_t addr_len;
	uint32_t size;
	uint32_t mode;

	tr->dtls.ops.tk_fill = algo->enc_tk_fill;

	/*
	 * First two words are Control words.
	 */
	tr_words[0] = algo->ctrl_words_0;
	tr_words[1] = algo->ctrl_words_1;

	/*
	 * Enable encap specific fields in control words 0.
	 */
	tr_words[0] |= EIP_TK_CTRL_OP_HMAC_ENC;

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
	 * DTLS specific fields for EIP96.
	 * - Version in Host order (1 word)
	 * - Sequence number start (2 words/48bit sequence)
	 */
	*crypt_words++ = EIP_TR_DTLS_VERSION(dtls->version);
	*crypt_words++ = EIP_TR_DTLS_SEQ_START;
	*crypt_words++ = EIP_TR_DTLS_EPOCH(ntohs(dtls->epoch));
	seq_offset = crypt_words - tr_words - 2;

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
	tr_words[1] |= EIP_TR_CTRL_SEQ_NUM_OFFSET_EN;
	tr_words[1] |= EIP_TR_CTRL_SEQ_NUM_OFFSET(seq_offset);
	tr_words[1] |= EIP_TR_CTRL_SEQ_NUM_STORE;

	/*
	 * Store the seq no offset, icv, iv and cipher block length
	 * to be used during token fill
	 */
	tr_dtls->seq_offset = seq_offset;
	tr->digest_len = algo->auth_digest_len;
	tr->blk_len = algo->cipher_blk_len;
	tr->iv_len = algo->iv_len;

	/*
	 * Configure DTLS encapsulation specific details in TR
	 */
	tr_dtls->ip_version = dtls->ip_ver;
	tr_dtls->tos = dtls->ip_dscp;
	tr_dtls->ip_df = dtls->ip_df ? htons(IP_DF) : 0;
	tr_dtls->ttl = dtls->ip_ttl;
	tr_dtls->src_port = dtls->src_port;
	tr_dtls->dst_port = dtls->dst_port;

	/*
	 * Configure mode specific details in TR
	 * We bypass the pre-filled outer header (IP/IP+UDP)
	 * There is no post processing for encap
	 */
	tr_dtls->ops.post = NULL;
	mode = dtls->flags & (EIP_TR_DTLS_FLAG_UDPLITE | EIP_TR_DTLS_FLAG_CAPWAP | EIP_TR_DTLS_FLAG_IPV6);

	switch (mode) {
	case (EIP_TR_DTLS_FLAG_UDPLITE | EIP_TR_DTLS_FLAG_IPV6):
	case (EIP_TR_DTLS_FLAG_IPV6):
	case (EIP_TR_DTLS_FLAG_UDPLITE):
		WARN_ON(1);
		break;

	case (EIP_TR_DTLS_FLAG_CAPWAP | EIP_TR_DTLS_FLAG_IPV6 | EIP_TR_DTLS_FLAG_UDPLITE):
		bypass_len = sizeof(struct ipv6hdr) + sizeof(struct udphdr) + sizeof(struct eip_capwap_hdr);
		tr_dtls->ops.pre = eip_tr_dtls_enc_v6;
		tr_dtls->protocol = IPPROTO_UDPLITE;
		addr_len = sizeof(struct in6_addr);
		break;

	case (EIP_TR_DTLS_FLAG_CAPWAP | EIP_TR_DTLS_FLAG_IPV6):
		bypass_len = sizeof(struct ipv6hdr) + sizeof(struct udphdr) + sizeof(struct eip_capwap_hdr);
		tr_dtls->ops.pre = eip_tr_dtls_enc_v6;
		tr_dtls->protocol = IPPROTO_UDP;
		addr_len = sizeof(struct in6_addr);
		break;

	case (EIP_TR_DTLS_FLAG_CAPWAP | EIP_TR_DTLS_FLAG_UDPLITE):
		bypass_len = sizeof(struct iphdr) + sizeof(struct udphdr) + sizeof(struct eip_capwap_hdr);
		tr_dtls->ops.pre = eip_tr_dtls_enc_v4;
		tr_dtls->protocol = IPPROTO_UDPLITE;
		addr_len = sizeof(uint32_t);
		break;

	case (EIP_TR_DTLS_FLAG_CAPWAP):	/* DTLS + CAPWAP IPv4 tunnel */
		bypass_len = sizeof(struct iphdr) + sizeof(struct udphdr) + sizeof(struct eip_capwap_hdr);
		tr_dtls->ops.pre = eip_tr_dtls_enc_v4;
		tr_dtls->protocol = IPPROTO_UDP;
		addr_len = sizeof(uint32_t);
		break;

	default:	/* DTLS IPv4 tunnel */
		bypass_len = sizeof(struct iphdr) + sizeof(struct udphdr);
		tr_dtls->ops.pre = eip_tr_dtls_enc_v4;
		tr_dtls->protocol = IPPROTO_UDP;
		addr_len = sizeof(uint32_t);
		break;
	}

	tr_dtls->bypass_len = bypass_len;	/* Software headers to bypass */
	tr_dtls->fixed_len = bypass_len + tr->iv_len + sizeof(struct eip_dtls_hdr);	/* Total overhead */
	memcpy(tr_dtls->src_ip, dtls->src_ip, addr_len);
	memcpy(tr_dtls->dst_ip, dtls->dst_ip, addr_len);
}
