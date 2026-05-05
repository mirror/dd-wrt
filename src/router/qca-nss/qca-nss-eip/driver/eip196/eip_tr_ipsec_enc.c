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
 * eip_tr_ipsec_enc_len()
 * 	Calculates the required pad length, to be used during token fill.
 *
 * SPI, seq no, IV & ICV will be inserted by EIP196 hardware
 * Return value is the aligned length
 */
static inline uint16_t eip_tr_ipsec_enc_len(struct eip_tr *tr, struct sk_buff *skb, uint16_t payload_len)
{
	uint16_t total_len;
	uint16_t pad_len;

	total_len = ALIGN(payload_len + EIP_TK_IPSEC_ESP_TRAILER_LEN, tr->blk_len);
	pad_len = total_len - (payload_len + EIP_TK_IPSEC_ESP_TRAILER_LEN);
	EIP_TR_IPSEC_SKB_CB(skb)->pad_len = pad_len;

	return total_len;
}

/*
 * eip_tr_ipsec_enc_tun4()
 *	Outer IPv4 header addition before tunnel mode encapsulation
 */
static void eip_tr_ipsec_enc_tun4(struct eip_tr *tr, struct sk_buff *skb)
{
	struct eip_tr_ipsec *ipsec = &tr->ipsec;
	struct eip_ctx *ctx = tr->ctx;
	uint16_t payload_len;
	struct udphdr *udph;
	struct iphdr *iph;
	__be16 tos;
	__be16 df;

	/*
	 * Calculate the required pad length and update post encap payload length
	 */
	skb_reset_inner_network_header(skb);
	payload_len = eip_tr_ipsec_enc_len(tr, skb, skb->len) + ipsec->fixed_len;

	switch(ip_hdr(skb)->version) {
	case IPVERSION:
		iph = ip_hdr(skb);
		tos = (tr->tr_flags & EIP_TR_IPSEC_FLAG_CP_TOS) ? ip_hdr(skb)->tos : ipsec->tos;

		/*
		 * Check if Linux has marked the packet to ignore fragmentation mark; in that case
		 * clear any DF settings inherited for the packet
		 */
		df = (tr->tr_flags & EIP_TR_IPSEC_FLAG_CP_DF) ? iph->frag_off & htons(IP_DF) : htons(ipsec->df);
		df = !skb->ignore_df ? df : 0;
		EIP_TR_IPSEC_SKB_CB(skb)->ip_proto = IPPROTO_IPIP;
		break;

	default:
		tos = (tr->tr_flags & EIP_TR_IPSEC_FLAG_CP_TOS) ? ipv6_get_dsfield(ipv6_hdr(skb)) : ipsec->tos;
		df = !skb->ignore_df ? htons(IP_DF) : 0;
		EIP_TR_IPSEC_SKB_CB(skb)->ip_proto = IPPROTO_IPV6;
		break;
	}

	/*
	 * Add space for outer udp header for NATT and fill it
	 * UDP header checksum should be zero (rfc3948) & length should
	 * include the post encap packet length
	 */
	if(likely(ipsec->protocol == IPPROTO_UDP)) {
		udph = (struct udphdr*)skb_push(skb, sizeof(struct udphdr));
		skb_reset_transport_header(skb);

		udph->source = ipsec->src_port;
		udph->dest = ipsec->dst_port;
		udph->len = htons(payload_len - sizeof(struct iphdr));
		udph->check = 0; /* TODO: add support for user configurable checksum option */
	}

	/*
	 * Add space for outer IPv4 header and fill it
	 * Outer IPv4 header total length post encap includes outer hdr len
	 * (IP / IP+UDP) and ip payload length
	 */
	iph = (struct iphdr *)skb_push(skb, sizeof(struct iphdr));
	skb_reset_network_header(skb);
	skb_set_transport_header(skb, sizeof(struct iphdr));

	iph->version = IPVERSION;
	iph->ihl = sizeof(struct iphdr) >> 2;
	iph->tos = tos;
	iph->tot_len = htons(payload_len);
	iph->id = htons(ctx->ip_id++);
	iph->frag_off = df;
	iph->ttl = ipsec->ttl;
	iph->protocol = ipsec->protocol;
	iph->saddr = ipsec->src_ip[0];
	iph->daddr = ipsec->dst_ip[0];

	/*
	 * Update the header checksum
	 * TODO: Update checksum with modified fields only
	 */
	iph->check = 0;
	iph->check = ip_fast_csum((unsigned char *)iph, 5);
}

/*
 * eip_tr_ipsec_enc_tun6()
 *	Outer IPv6 header addition before tunnel mode encapsulation
 */
static void eip_tr_ipsec_enc_tun6(struct eip_tr *tr, struct sk_buff *skb)
{
	struct eip_tr_ipsec *ipsec = &tr->ipsec;
	struct ipv6hdr *ip6h;
	uint16_t payload_len;
	__be16 tos;

	/*
	 * Calculate the required pad length and update post encap payload length
	 */
	payload_len = eip_tr_ipsec_enc_len(tr, skb, skb->len) + ipsec->fixed_len;

	switch(ipv6_hdr(skb)->version) {
	case IPVERSION:
		tos = (tr->tr_flags & EIP_TR_IPSEC_FLAG_CP_TOS) ? ip_hdr(skb)->tos : ipsec->tos;
		EIP_TR_IPSEC_SKB_CB(skb)->ip_proto = IPPROTO_IPIP;
		break;

	default:
		ip6h = ipv6_hdr(skb);
		tos = (tr->tr_flags & EIP_TR_IPSEC_FLAG_CP_TOS) ? ipv6_get_dsfield(ip6h) : ipsec->tos;
		EIP_TR_IPSEC_SKB_CB(skb)->ip_proto = IPPROTO_IPV6;
		break;
	}

	/*
	 * Add space for outer IPv6 header and fill it
	 * Outer IPv6 header payload length post encap does not
	 * include IPv6 header len
	 */
	ip6h = skb_push(skb, sizeof(struct ipv6hdr));
	skb_reset_network_header(skb);
	skb_set_transport_header(skb, sizeof(struct ipv6hdr));

	ip6_flow_hdr(ip6h, tos, 0);

	ip6h->payload_len = htons(payload_len);
	ip6h->nexthdr = IPPROTO_ESP;
	ip6h->hop_limit = ipsec->ttl;

	memcpy(ip6h->saddr.s6_addr32, ipsec->src_ip, sizeof(ip6h->saddr.s6_addr32));
	memcpy(ip6h->daddr.s6_addr32, ipsec->dst_ip, sizeof(ip6h->daddr.s6_addr32));
}

/*
 * eip_tr_ipsec_enc_trans4()
 * 	Header update for transport mode before encapsulation
 */
static void eip_tr_ipsec_enc_trans4(struct eip_tr *tr, struct sk_buff *skb)
{
	struct eip_tr_ipsec *ipsec = &tr->ipsec;
	struct iphdr *iph = ip_hdr(skb);
	uint16_t payload_len;

	/*
	 * Calculate the required pad length and update post encap payload length
	 */
	payload_len = eip_tr_ipsec_enc_len(tr, skb, skb->len - sizeof(struct iphdr)) + ipsec->fixed_len;
	EIP_TR_IPSEC_SKB_CB(skb)->ip_proto = iph->protocol;

	/*
	 * Update total length post encap, protocol & checksum in IPv4 header
	 */
	iph->tot_len = htons(payload_len);
	iph->protocol = IPPROTO_ESP;

	/*
	 * TODO: Update checksum with modified fields only
	 */
	iph->check = 0;
	iph->check = ip_fast_csum((unsigned char *)iph, iph->ihl);

	/*
	 * TODO: handle NATT case
	 */
}

/*
 * eip_tr_ipsec_enc_trans6()
 *	Header update for transport mode before encapsulation
 */
static void eip_tr_ipsec_enc_trans6(struct eip_tr *tr, struct sk_buff *skb)
{
	struct eip_tr_ipsec *ipsec = &tr->ipsec;
	struct ipv6hdr *ip6h = ipv6_hdr(skb);
	uint16_t payload_len;

	/*
	 * Calculate the required pad length and update post encap payload length
	 */
	payload_len = eip_tr_ipsec_enc_len(tr, skb, skb->len - sizeof(struct ipv6hdr)) + ipsec->fixed_len ;
	EIP_TR_IPSEC_SKB_CB(skb)->ip_proto = ip6h->nexthdr;

	/*
	 * update total length post encap and next header in IPv6 header
	 */
	ip6h->payload_len = htons(payload_len);
	ip6h->nexthdr = IPPROTO_ESP;
}

/*
 * eip_tr_ipsec_enc_err()
 *	Hardware error callback API for Encapsulation.
 */
void eip_tr_ipsec_enc_err(struct eip_tr *tr, struct eip_hw_desc *hw, struct eip_sw_desc *sw, uint16_t cle_err, uint16_t tr_err)
{
	struct eip_tr_ipsec *ipsec = &tr->ipsec;
	struct eip_ctx *ctx = tr->ctx;
	uint32_t err;

	/*
	 * Call the client callback.
	 */
	err = ((tr_err << EIP_TR_ERR_SHIFT) | cle_err);
	ipsec->ops.err_cb(ipsec->ops.app_data, sw->req, err);

	kmem_cache_free(ctx->tk_cache, sw->tk);
	kmem_cache_free(ctx->sw_cache, sw);
	eip_tr_deref(tr);
}

/*
 * eip_tr_ipsec_enc_done()
 *	Hardware completion callback API for Encapsulation.
 */
void eip_tr_ipsec_enc_done(struct eip_tr *tr, struct eip_hw_desc *hw, struct eip_sw_desc *sw)
{
	uint16_t out_len = EIP_HW_RES_DATA_LEN(hw->token[0]);
	struct eip_tr_ipsec *ipsec = &tr->ipsec;
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
	ipsec->ops.cb(ipsec->ops.app_data, sw->req);

	kmem_cache_free(ctx->tk_cache, sw->tk);
	kmem_cache_free(ctx->sw_cache, sw);
	eip_tr_deref(tr);
}


/*
 * eip_tr_ipsec_enc()
 *	IPsec Encapsulation.
 */
int eip_tr_ipsec_enc(struct eip_tr *tr, struct sk_buff *skb)
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

	EIP_TR_PRE_PROCESS(tr, &tr->ipsec.ops, skb);

	/*
	 * Fill TK params
	 */
	tk_params.tr = tr;
	tk_params.eip_req = (eip_req_t)skb;

	/*
	 * Fill token for encryption and hash for ipsec
	 */
	tk_words = eip_tr_fill_token(tk, &tr->ipsec.ops, &tk_params);
	eip_dmac_clean_range(tk, tk + 1);

	dma_tx = skb_is_nonlinear(skb) ? eip_dma_tx_nonlinear_skb : eip_dma_tx_linear_skb;

	/*
	 * Fill software descriptor.
	 * Dereference: eip_tr_ipsec_enc_done() / eip_tr_ipsec_enc_err()
	 */
	sw->tr = eip_tr_ref(tr);

	sw->tk = tk;
	sw->tk_hdr = tk_params.tk_hdr;
	sw->tk_words = tk_words;
	sw->tk_addr = virt_to_phys(tk);
	sw->tr_addr_type = tr->tr_addr_type;

	sw->req = skb;
	sw->hw_svc = EIP_HW_CMD_HWSERVICE_LAC;
	sw->comp = eip_tr_ipsec_enc_done;
	sw->err_comp = eip_tr_ipsec_enc_err;

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
EXPORT_SYMBOL(eip_tr_ipsec_enc);

/*
 * eip_tr_ipsec_enc_cmn_init()
 *      Initialize the transform record for ipsec encapsulation.
 */
void eip_tr_ipsec_enc_cmn_init(struct eip_tr *tr, struct eip_tr_info *info, const struct eip_svc_entry *algo)
{
        struct eip_tr_info_ipsec *ipsec = &info->ipsec;
	struct eip_tr_ipsec *tr_ipsec = &tr->ipsec;
        struct eip_tr_base *crypto = &info->base;
        uint32_t *crypt_words = tr->hw_words;
        uint32_t *tr_words = tr->hw_words;
	uint8_t bypass_len = 0;
        uint8_t seq_offset;
	uint8_t addr_len;
	bool esn, natt;
        uint32_t size;
	uint32_t mode;

        esn = ipsec->sa_flags & EIP_TR_IPSEC_FLAG_ESN;
        natt = ipsec->sa_flags & EIP_TR_IPSEC_FLAG_UDP;

	tr->ipsec.ops.tk_fill = algo->enc_tk_fill;

        /*
         * First two words are Control words.
         */
        tr_words[0] = algo->ctrl_words_0;
        tr_words[1] = algo->ctrl_words_1;

        /*
         * Enable IPsec specific fields in control words 0.
         */
	tr_words[0] |= EIP_TK_CTRL_OP_ENC_HMAC;
        tr_words[0] |= EIP_TR_IPSEC_EXT_SEQ_NUM(esn);

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
         * - Sequence number start (2 words for ESN else 1 word)
         */
        *crypt_words++ = ntohl(ipsec->spi_idx);
        *crypt_words++ = 0x0;
        seq_offset = crypt_words - tr_words - 1;

        if (esn)
                *crypt_words++ = 0;

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
         * Store the details to be used during token fill
         */
        tr_ipsec->seq_offset = seq_offset;
	tr_ipsec->ctx_num_words = EIP_TR_IPSEC_CTX_WORDS_ESN(esn);
        tr->blk_len = algo->cipher_blk_len;
        tr->digest_len = ipsec->icv_len;
	tr->iv_len = algo->iv_len;

        /*
         * Configure IPsec encapsulation specific details in TR
         */
	tr_ipsec->ip_version = ipsec->ip_ver;
	tr_ipsec->tos = ipsec->ip_dscp;
	tr_ipsec->df = ipsec->ip_df;
	tr_ipsec->ttl = ipsec->ip_ttl;
	tr_ipsec->protocol = natt ? IPPROTO_UDP : IPPROTO_ESP;

	if(natt) {
		tr_ipsec->src_port = ipsec->src_port;
		tr_ipsec->dst_port = ipsec->dst_port;
		bypass_len += sizeof(struct udphdr);
	}

	/*
	 * Configure mode specific details in TR
	 * We bypass the pre-filled outer header (IP/IP+UDP)
	 * There is no post processing for encap
	 */
	tr_ipsec->ops.post = NULL;
	mode = ipsec->sa_flags & (EIP_TR_IPSEC_FLAG_TUNNEL | EIP_TR_IPSEC_FLAG_UDP | EIP_TR_IPSEC_FLAG_IPV6);

	switch (mode) {
	/*
	 * Tunnel mode
	 */
	case (EIP_TR_IPSEC_FLAG_TUNNEL | EIP_TR_IPSEC_FLAG_UDP):
	case EIP_TR_IPSEC_FLAG_TUNNEL:
		bypass_len += sizeof(struct iphdr);
		tr_ipsec->bypass_len = bypass_len;
		tr_ipsec->fixed_len = bypass_len + tr->iv_len + sizeof(struct ip_esp_hdr) + tr->digest_len;
		tr_ipsec->ops.pre = eip_tr_ipsec_enc_tun4;
		addr_len = sizeof(uint32_t);
		break;

	case (EIP_TR_IPSEC_FLAG_TUNNEL | EIP_TR_IPSEC_FLAG_IPV6):
		tr_ipsec->bypass_len = sizeof(struct ipv6hdr);
		tr_ipsec->fixed_len = tr->iv_len + sizeof(struct ip_esp_hdr) + tr->digest_len;
		tr_ipsec->ops.pre = eip_tr_ipsec_enc_tun6;
		addr_len = sizeof(struct in6_addr);
		break;
	/*
	 * Transport mode
	 */
	case EIP_TR_IPSEC_FLAG_IPV6:
		tr_ipsec->bypass_len = sizeof(struct ipv6hdr);
		tr_ipsec->fixed_len = tr->iv_len + sizeof(struct ip_esp_hdr) + tr->digest_len;
		tr_ipsec->ops.pre = eip_tr_ipsec_enc_trans6;
		addr_len = sizeof(struct in6_addr);
		break;

	case EIP_TR_IPSEC_FLAG_UDP:
		/*
		 * TODO: Add support
		 */
		BUG_ON(1);
		break;
	default:
		bypass_len += sizeof(struct iphdr);
		tr_ipsec->bypass_len = bypass_len;
		tr_ipsec->fixed_len = bypass_len + tr->iv_len + sizeof(struct ip_esp_hdr) + tr->digest_len;
		tr_ipsec->ops.pre = eip_tr_ipsec_enc_trans4;
		addr_len = sizeof(uint32_t);
		break;
	}

	memcpy(tr_ipsec->src_ip, ipsec->src_ip, addr_len);
	memcpy(tr_ipsec->dst_ip, ipsec->dst_ip, addr_len);
}
