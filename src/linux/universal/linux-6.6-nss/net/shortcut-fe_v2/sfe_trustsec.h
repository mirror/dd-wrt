/*
 * sfe_trustsec.h
 *	Shortcut flow acceleration for trustsec
 *
 * Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef __SFE_TRUSTSEC_H
#define __SFE_TRUSTSEC_H

#include <linux/if_trustsec.h>

/*
 * sfe_trustsec_proto_hdr
 *	Protocol header after trustsec header
 */
struct sfe_trustsec_proto_hdr {
	u16 protocol;
};

/*
 * sfe_trustsec_check_and_parse_sgt()
 *	Check packet for trustsec header.
 */
static inline bool sfe_trustsec_check_and_parse_sgt(struct sk_buff *skb, struct sfe_l2_info *l2_info)
{
	struct trustsec_hdr *thdr = trustsec_get_hdr(skb);
	struct sfe_trustsec_proto_hdr *phdr;

	/*
	 * Check that we have space for trustsec header header here.
	 */
	if (unlikely(!pskb_may_pull(skb, (sizeof(struct trustsec_hdr) + sizeof(struct sfe_trustsec_proto_hdr))))) {
		DEBUG_TRACE("%px: packet too short for trustsec header\n", skb);
		return false;
	}

	if (thdr->meta_hdr_ver != TRUSTSEC_META_HDR_VERSION) {
		DEBUG_TRACE("%px: version: %u is different than supported one\n", skb, thdr->meta_hdr_ver);
		return false;
	}

	if (thdr->meta_hdr_len != TRUSTSEC_META_HDR_LEN) {
		DEBUG_TRACE("%px: trustsec header length: %u is different than supported one\n",
											skb, thdr->meta_hdr_len);
		return false;
	}

	if (ntohs(thdr->payload_len) != TRUSTSEC_PAYLOAD_LEN) {
		DEBUG_TRACE("%px: length: %u is different than supported one\n", skb, thdr->payload_len);
		return false;
	}

	if (ntohs(thdr->payload_opt_type) != TRUSTSEC_PAYLOAD_OPT_TYPE) {
		DEBUG_TRACE("%px: option type: %u is different than supported one\n", skb, thdr->payload_opt_type);
		return false;
	}

	phdr = (struct sfe_trustsec_proto_hdr *)((u8*)thdr + sizeof(*thdr));

	/*
	 * Converting PPP protocol values to ether type protocol values
	 */
	switch(ntohs(phdr->protocol)) {
	case ETH_P_IP:
		skb->protocol = htons(ETH_P_IP);
		sfe_l2_protocol_set(l2_info, ETH_P_IP);
		break;

	case ETH_P_IPV6:
		skb->protocol = htons(ETH_P_IPV6);
		sfe_l2_protocol_set(l2_info, ETH_P_IPV6);
		break;

	case ETH_P_PPP_SES:
		skb->protocol = htons(ETH_P_PPP_SES);
		break;

	default:
		DEBUG_TRACE("%px: Unsupported protocol : %d in trustsec header\n", skb, ntohs(phdr->protocol));
		return false;
	}

	sfe_l2_trustsec_sgt_set(l2_info, ntohs(thdr->sgt));
	sfe_l2_parse_flag_set(l2_info, SFE_L2_PARSE_FLAGS_TRUSTSEC_INGRESS);

	/*
	 * strip trustsec header
	 */
	__skb_pull(skb, sizeof(struct trustsec_hdr) + sizeof(struct sfe_trustsec_proto_hdr));
	skb_reset_network_header(skb);
	return true;
}

/*
 * sfe_trustsec_undo_parse()
 *      Restore some skb fields which are modified when parsing trustsec header.
 */
static inline void sfe_trustsec_undo_parse(struct sk_buff *skb, struct sfe_l2_info *l2_info)
{
	if (sfe_l2_parse_flag_check(l2_info, SFE_L2_PARSE_FLAGS_TRUSTSEC_INGRESS)) {
		__skb_push(skb, sizeof(struct trustsec_hdr) + sizeof(struct sfe_trustsec_proto_hdr));
		skb->protocol = (htons(ETH_P_TRSEC));
	}
}

/*
 * sfe_trustsec_validate_ingress_sgt()
 *      Validate ingress packet's SGT tag
 */
static inline bool sfe_trustsec_validate_ingress_sgt(
					struct sk_buff *skb, bool sgt_valid,
					struct sfe_trustsec_hdr *ts_hdr,
					struct sfe_l2_info *l2_info)
{
	if (likely(!sfe_is_l2_feature_enabled())) {
		return true;
	}

	if (unlikely(sgt_valid != sfe_l2_parse_flag_check(l2_info, SFE_L2_PARSE_FLAGS_TRUSTSEC_INGRESS))) {
		return false;
	}

	if (!sgt_valid) {
		return true;
	}

	return (l2_info->trustsec_sgt == ts_hdr->sgt);
}

/*
 * sfe_trustsec_add_sgt()
 *      Add trustsec tags at skb->data.
 *
 * skb->data will point to trustsec header after the function
 */
static inline void sfe_trustsec_add_sgt(struct sk_buff *skb, struct sfe_trustsec_hdr *ts_hdr)
{
	unsigned char *pp;

	/*
	 * Insert the proto header for the next protocol
	 */
	pp = __skb_push(skb, 2);
	put_unaligned(skb->protocol, pp);

	trustsec_insert_hdr(skb, htons(ts_hdr->sgt));
}

#endif /* __SFE_TRUSTSEC_H */
