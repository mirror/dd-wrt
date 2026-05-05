/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: ISC
 */

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kref.h>
#include <linux/slab.h>
#include <linux/completion.h>

#include "eip_priv.h"

/*
 * eip_tr_ipsec_enc_cmn_init()
 *	Initialize the transform record for ipsec encapsulation.
 */
void eip_tr_ipsec_enc_cmn_init(struct eip_tr *tr, struct eip_tr_info *info, const struct eip_svc_entry *algo)
{
	struct eip_tr_info_ipsec *ipsec = &info->ipsec;
	struct eip_tr_base *crypto = &info->base;
	uint32_t *crypt_words = tr->hw_words;
	uint32_t *tr_words = tr->hw_words;
	uint8_t seq_num_offst;
	uint32_t size;

	bool esn = ipsec->sa_flags & EIP_TR_IPSEC_FLAG_ESN;
	bool natt = ipsec->sa_flags & EIP_TR_IPSEC_FLAG_UDP;
	bool copy_dscp = ipsec->sa_flags & EIP_TR_IPSEC_FLAG_CP_TOS;
	bool copy_df = ipsec->sa_flags & EIP_TR_IPSEC_FLAG_CP_DF;
	bool ipv6 = ipsec->sa_flags & EIP_TR_IPSEC_FLAG_IPV6;

	/*
	 * First two words are Control words.
	 */
	tr_words[0] = algo->ctrl_words_0;
	tr_words[1] = algo->ctrl_words_1;

	/*
	 * Enable IPsec specific fields in control words 0.
	 */
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
	seq_num_offst = crypt_words - tr_words - 1;
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
	tr_words[1] |= EIP_TR_IPSEC_SEQ_NUM_OFFSET_EN;
	tr_words[1] |= EIP_TR_IPSEC_SEQ_NUM_OFFSET(seq_num_offst);
	tr_words[1] |= EIP_TR_IPSEC_SEQ_NUM_STORE;

	/*
	 * Configure IPsec encapsulation specfic field in TR.
	 */
	if (!ipv6) {
		uint32_t csum;

		tr_words[56] = ipsec->src_ip[0];
		tr_words[60] = ipsec->dst_ip[0];

		/*
		 * Checksum require for IPv4.
		 */
		csum = (ipsec->src_ip[0] & 0xFFFF) + (ipsec->src_ip[0] >> 16);
		csum += (ipsec->dst_ip[0] & 0xFFFF) + (ipsec->dst_ip[0] >> 16);
		tr_words[57] = (csum & 0xFFFF) + (csum >> 16);

		tr_words[65] = 0;
	} else {
		tr_words[56] = ipsec->src_ip[0];
		tr_words[57] = ipsec->src_ip[1];
		tr_words[58] = ipsec->src_ip[2];
		tr_words[59] = ipsec->src_ip[3];

		tr_words[60] = ipsec->dst_ip[0];
		tr_words[61] = ipsec->dst_ip[1];
		tr_words[62] = ipsec->dst_ip[2];
		tr_words[63] = ipsec->dst_ip[3];

		tr_words[65] = EIP_TR_IPSEC_IPV6_EN;
	}

	/*
	 * Encap token header.
	 */
	tr_words[64] = EIP_TR_IPSEC_ENCAP_TOKEN_HDR;
	tr_words[64] |= EIP_TR_IPSEC_ENCAP_TOKEN_HDR_IV;

	/*
	 * Outer DF: 0 = Copy from inner, 1 = clear df, 2 = set df.
	 * DSCP En: 0 = Copy from inner, 1 = use default.
	 */
	tr_words[65] |= EIP_TR_IPSEC_DF(copy_df ? 0 : (ipsec->ip_df ? 2 : 1));
	tr_words[65] |= EIP_TR_IPSEC_DSCP_COPY_EN(!copy_dscp);
	tr_words[65] |= EIP_TR_IPSEC_IPHDR_PROC;
	tr_words[65] |= EIP_TR_IPSEC_EXT_SEQ_NUM_PROC(esn);

	/*
	 * Redirect packet to inline chanel after encapsulation.
	 */
	if (tr->svc == EIP_SVC_HYBRID_IPSEC) {
		tr_words[65] |= EIP_TR_REDIR_EN;
		tr_words[65] |= EIP_TR_REDIR_IFACE(EIP_HW_INLINE_RING);;
		pr_debug("%px: Redirection enable to inline(%u) for encap record\n", tr, EIP_HW_INLINE_RING);
	}

	tr_words[66] = copy_dscp ? 0 : EIP_TR_IPSEC_DSCP(ipsec->ip_dscp);
	tr_words[66]|= EIP_TR_IPSEC_TTL(ipsec->ip_ttl);
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

	tr_words[70] = EIP_TR_IPSEC_ENCAP_TOKEN_VERIFY;
	tr_words[71] = esn ? EIP_TR_IPSEC_ENCAP_ESN_TOKEN_INST : EIP_TR_IPSEC_ENCAP_TOKEN_INST;
	tr_words[71] |= seq_num_offst;
}

/*
 * eip_tr_ipsec_enc()
 *	IPsec Encapsulation.
 */
int eip_tr_ipsec_enc(struct eip_tr *tr, struct sk_buff *skb)
{
	int (*dma_tx)(struct eip_dma *dma, struct eip_sw_desc *sw, struct sk_buff *skb);
	struct eip_tr_stats *tr_stats = this_cpu_ptr(tr->stats_pcpu);
	struct eip_ctx *ctx = tr->ctx;
	struct eip_sw_desc *sw;
	struct eip_dma *dma;
	int status = 0;

	dma_tx = skb_is_nonlinear(skb) ? eip_dma_hy_tx_nonlinear_skb : eip_dma_hy_tx_linear_skb;

	/*
	 * check whether the packet length is within
	 * max jumbo size limit supported by the HW.
	 */
	if (unlikely(skb->len > EIP_MAX_PKT_LEN)) {
		pr_err("%px: packet size(%d) exceeds max supported limit (%d)\n", skb, skb->len, EIP_MAX_PKT_LEN);
		tr_stats->tx_error_len++;
		return -EINVAL;
	}

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
	 * Dereference: eip_tr_ipsec_tx_done()
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
EXPORT_SYMBOL(eip_tr_ipsec_enc);

