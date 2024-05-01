/*
 * sfe_vlan.h
 *	Shortcut flow acceleration for 802.1AD/802.1Q flow
 *
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef __SFE_VLAN_H
#define __SFE_VLAN_H

#include <linux/if_vlan.h>

/*
 * sfe_vlan_check_and_parse_tag()
 *
 * case 1: QinQ frame (e.g. outer tag = 88a80032, inner tag = 81000001):
 * When entering this function:
 * ----+-----------------+-----|-----+-----------+-----+---------
 *     |DMAC    |SMAC    |88|a8|00|32|81|00|00|01|08|00|45|00|
 * ----+-----------------+-----A-----+-----------+-----+---------
 *                            skb->data
 *   skb->protocol = ntohs(ETH_P_8021AD)
 *   skb->vlan_proto = 0
 *   skb->vlan_tci = 0
 *   skb->vlan_present = 0
 * When exiting:
 * ----+-----------------+-----------+-----------+-----+---------
 *     |DMAC    |SMAC    |88|a8|00|32|81|00|00|01|08|00|45|00|
 * ----+-----------------+-----------+-----------+-----A---------
 *                                                    skb->data
 *   skb->protocol = ntohs(ETH_P_IP)
 *   skb->vlan_proto = 0
 *   skb->vlan_tci = 0
 *   skb->vlan_present = 0
 *   l2_info->vlan_hdr_cnt = 2
 *   l2_info->vlan_hdr[0].tpid = ntohs(ETH_P_8021AD)
 *   l2_info->vlan_hdr[0].tci = 0x0032
 *   l2_info->vlan_hdr[1].tpid = ntohs(ETH_P_8021Q)
 *   l2_info->vlan_hdr[1].tci = 0x0001
 *   l2_info->protocol = ETH_P_IP
 *
 * case 2: 802.1Q frame (e.g. the tag is 81000001):
 * When entering this function:
 * ----+-----------------+-----|-----+-----+---------
 *     |DMAC    |SMAC    |81|00|00|01|08|00|45|00|
 * ----+-----------------+-----A-----+-----+---------
 *                            skb->data
 *   skb->protocol = ntohs(ETH_P_8021Q)
 *   skb->vlan_proto = 0
 *   skb->vlan_tci = 0
 *   skb->vlan_present = 0
 * When exiting:
 * ----+-----------------+-----------+-----+---------
 *     |DMAC    |SMAC    |81|00|00|01|08|00|45|00|
 * ----+-----------------+-----------+-----A---------
 *                                        skb->data
 *   skb->protocol = ntohs(ETH_P_IP)
 *   skb->vlan_proto = 0
 *   skb->vlan_tci = 0
 *   skb->vlan_present = 0
 *   l2_info->vlan_hdr_cnt = 1
 *   l2_info->vlan_hdr[0].tpid = ntohs(ETH_P_8021Q)
 *   l2_info->vlan_hdr[0].tci = 0x0001
 *   l2_info->protocol = ETH_P_IP
 *
 * case 3: untagged frame
 * When entering this function:
 * ----+-----------------+-----|---------------------
 *     |DMAC    |SMAC    |08|00|45|00|
 * ----+-----------------+-----A---------------------
 *                            skb->data
 *   skb->protocol = ntohs(ETH_P_IP)
 *   skb->vlan_proto = 0
 *   skb->vlan_tci = 0
 *   skb->vlan_present = 0
 * When exiting:
 * ----+-----------------+-----|---------------------
 *     |DMAC    |SMAC    |08|00|45|00|
 * ----+-----------------+-----A---------------------
 *                            skb->data
 *   skb->protocol = ntohs(ETH_P_IP)
 *   skb->vlan_proto = 0
 *   skb->vlan_tci = 0
 *   skb->vlan_present = 0
 *   l2_info->vlan_hdr_cnt = 0
 *   l2_info->protocol = ETH_P_IP
 */
static inline bool sfe_vlan_check_and_parse_tag(struct sk_buff *skb, struct sfe_l2_info *l2_info)
{
	struct vlan_hdr *vhdr;

	while ((skb->protocol == htons(ETH_P_8021AD) || skb->protocol == htons(ETH_P_8021Q)) &&
			l2_info->vlan_hdr_cnt < SFE_MAX_VLAN_DEPTH) {
		if (unlikely(!pskb_may_pull(skb, VLAN_HLEN))) {
			return false;
		}
		vhdr = (struct vlan_hdr *)skb->data;
		l2_info->vlan_hdr[l2_info->vlan_hdr_cnt].tpid = skb->protocol;
		l2_info->vlan_hdr[l2_info->vlan_hdr_cnt].tci = ntohs(vhdr->h_vlan_TCI);
		skb->protocol = vhdr->h_vlan_encapsulated_proto;
		l2_info->vlan_hdr_cnt++;
		/*
		 * strip VLAN header
		 */
		__skb_pull(skb, VLAN_HLEN);
		skb_reset_network_header(skb);
	}

	l2_info->protocol = htons(skb->protocol);
	return true;
}

/*
 * sfe_vlan_undo_parse()
 *      Restore some skb fields which are modified when parsing VLAN tags.
 */
static inline void sfe_vlan_undo_parse(struct sk_buff *skb, struct sfe_l2_info *l2_info)
{

	if ((l2_info->vlan_hdr_cnt == 0)) {
		return;
	}

	if ((sfe_l2_parse_flag_check(l2_info, SFE_L2_PARSE_FLAGS_VLAN_HW_TAG_SET))) {
		skb->vlan_present = 1;
		return;
	}

	if ((sfe_l2_parse_flag_check(l2_info, SFE_L2_PARSE_FLAGS_VLAN_LINUX_UNTAGGED))) {
		return;
	}

	skb->protocol = l2_info->vlan_hdr[0].tpid;
	__skb_push(skb, l2_info->vlan_hdr_cnt * VLAN_HLEN);
}

static inline bool sfe_vlan_dev_check_and_parse_tag(struct sk_buff *skb, struct sfe_l2_info *l2_info)
{
	struct net_device *vlan_dev = skb->dev;
	struct sfe_l2_info l2_info_temp;
	l2_info_temp.vlan_hdr_cnt = 0;

	/*
	 * Vlan tag would be present for ports where VLAN filtering is enabled.
	 * Read the vlan tag from skb header vlan field.
	 */
	if (skb_vlan_tag_present(skb)) {
		l2_info_temp.vlan_hdr[l2_info_temp.vlan_hdr_cnt].tci = skb_vlan_tag_get(skb);
		l2_info_temp.vlan_hdr[l2_info_temp.vlan_hdr_cnt].tpid = skb->vlan_proto;
		l2_info_temp.vlan_hdr_cnt++;
		__vlan_hwaccel_clear_tag(skb);
		sfe_l2_parse_flag_set(l2_info, SFE_L2_PARSE_FLAGS_VLAN_HW_TAG_SET);
	}

	/*
	 * VLAN processing is done by linux and hence skb->dev is updated to
	 * corresponding vlan dev, so extract the vlan info from vlan dev.
	 */
	while (is_vlan_dev(vlan_dev)) {
		if (unlikely(l2_info_temp.vlan_hdr_cnt > SFE_MAX_VLAN_DEPTH)) {
			if ((sfe_l2_parse_flag_check(l2_info, SFE_L2_PARSE_FLAGS_VLAN_HW_TAG_SET))) {
				skb->vlan_present = 1;
			}
			return false;
		}

		l2_info_temp.vlan_hdr[l2_info_temp.vlan_hdr_cnt].tci = vlan_dev_vlan_id(vlan_dev);
		l2_info_temp.vlan_hdr[l2_info_temp.vlan_hdr_cnt].tpid = vlan_dev_vlan_proto(vlan_dev);
		l2_info_temp.vlan_hdr_cnt++;
		vlan_dev = vlan_dev_next_dev(vlan_dev);
	}

	while ((l2_info_temp.vlan_hdr_cnt > 0) && l2_info->vlan_hdr_cnt < SFE_MAX_VLAN_DEPTH) {
		l2_info_temp.vlan_hdr_cnt--;
		l2_info->vlan_hdr[l2_info->vlan_hdr_cnt].tci = l2_info_temp.vlan_hdr[l2_info_temp.vlan_hdr_cnt].tci;
		l2_info->vlan_hdr[l2_info->vlan_hdr_cnt].tpid = l2_info_temp.vlan_hdr[l2_info_temp.vlan_hdr_cnt].tpid;
		l2_info->vlan_hdr_cnt++;
		sfe_l2_parse_flag_set(l2_info, SFE_L2_PARSE_FLAGS_VLAN_LINUX_UNTAGGED);
	}

	l2_info->protocol = htons(skb->protocol);
	return true;
}

/*
 * sfe_vlan_validate_ingress_tag()
 * 	Validates ingress packet VLAN tag.
 * 	Returns:
 * 	TRUE if validation is successful and we can accept the packet.
 * 	FALSE if packet is not valid and sfe cannot process the packet.
 */
static inline bool sfe_vlan_validate_ingress_tag(
		struct sk_buff *skb, u8 count, struct sfe_vlan_hdr *vlan_hdr, struct sfe_l2_info *l2_info, u8 vlan_filter_flags)
{
	u8 i;

	if (likely(!sfe_is_l2_feature_enabled())) {
		return true;
	}

	if (unlikely(count != l2_info->vlan_hdr_cnt)) {
#ifdef SFE_BRIDGE_VLAN_FILTERING_ENABLE
		/*
		 * Check if default PVID is enabled at ingress, using Bridge VLAN Filter rule.
		 * If yes, then we may accept the packet if one less ingress VLAN tag is found.
		 */
		if (!((vlan_filter_flags & SFE_VLAN_FILTER_FLAG_INGRESS_PVID) && (count == l2_info->vlan_hdr_cnt+1))) {
			return false;
		}
#else
		return false;
#endif
	}

	for (i = 0; i < l2_info->vlan_hdr_cnt; i++) {
		if (unlikely(vlan_hdr[i].tpid != l2_info->vlan_hdr[i].tpid)) {
			return false;
		}

		if (unlikely((vlan_hdr[i].tci & VLAN_VID_MASK) !=
			     (l2_info->vlan_hdr[i].tci & VLAN_VID_MASK))) {
			return false;
		}
	}

	return true;
}

/*
 * sfe_vlan_add_tag()
 *      Add VLAN tags at skb->data.
 *      Normally, it is called just before adding 14-byte Ethernet header.
 *
 *      This function does not update skb->mac_header so later code
 *      needs to call skb_reset_mac_header()/skb_reset_mac_len() to
 *      get correct skb->mac_header/skb->mac_len.
 *
 *      It assumes:
 *      - skb->protocol is set
 *      - skb has enough headroom to write VLAN tags
 *      - 0 < count <= SFE_MAX_VLAN_DEPTH
 *
 * When entering (e.g. skb->protocol = ntohs(ETH_P_IP) or ntohs(ETH_P_PPP_SES)):
 *  -------------------------------+---------------------
 *                                 |45|00|...
 *  -------------------------------A---------------------
 *                                skb->data
 *  -------------------------------v-----------------+-----+----------
 *                                 |11|00|xx|xx|xx|xx|00|21|45|00|...
 *  -------------------------------+-----------------+-----+----------
 *
 * When exiting (e.g. to add outer/inner tag = 88a80032/81000001):
 *  -------------+-----------+-----+---------------------
 *         |00|32|81|00|00|01|08|00|45|00|05|d8|....
 *  -------A-----+-----------+-----+---------------------
 *        skb->data
 *  -------v-----+-----------+-----+-----------------+-----+----------
 *         |00|32|81|00|00|01|88|64|11|00|xx|xx|xx|xx|00|21|45|00|
 *  -------------+-----------+-----+-----------------+-----+----------
 *  skb->protocol = ntohs(ETH_P_8021AD)
 */
static inline void sfe_vlan_add_tag(struct sk_buff *skb, int count, struct sfe_vlan_hdr *vlan)
{
	struct vlan_hdr *vhdr;
	int i;
	vlan += (count - 1);

	for (i = 0; i < count; i++) {
		vhdr = (struct vlan_hdr *)skb_push(skb, VLAN_HLEN);
		vhdr->h_vlan_TCI = htons(vlan->tci);
		vhdr->h_vlan_encapsulated_proto = skb->protocol;
		skb->protocol = vlan->tpid;
		vlan--;
	}
}

#endif /* __SFE_VLAN_H */
