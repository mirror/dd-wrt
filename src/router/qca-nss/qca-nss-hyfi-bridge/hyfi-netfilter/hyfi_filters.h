/*
 * Copyright (c) 2012, The Linux Foundation. All rights reserved.
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

#ifndef HYFI_FILTERS_H_
#define HYFI_FILTERS_H_

#include <linux/skbuff.h>

static inline int hyfi_is_hcp_pkt(struct sk_buff *skb)
{
	struct ethhdr *ethhdr = eth_hdr(skb);
	static const u8 ATH_OUI[3] = { 0x00, 0x03, 0x7f }; /* to be replaced with a define */

	if (unlikely(ethhdr->h_proto == htons(0x88b7))) {
		u8 *data = (u8 *) ethhdr;
		u8 *OUIPtr = data + ETH_HLEN;
		u16 ID = get_unaligned((u16*) (OUIPtr + 3));
		if (!memcmp(ATH_OUI, OUIPtr, sizeof ATH_OUI) && (htons(0x0000) == ID)) {
			return 1;
		}
	}
	return 0;
}

static inline int hyfi_hcp_frame_filter(struct sk_buff *skb,
		const struct net_device *dev)
{
	if (unlikely(hyfi_is_hcp_pkt(skb))) {
		u8 *data = (u8 *) eth_hdr(skb);
		put_unaligned(htonl(dev->ifindex), (u32*) (data + 33)); /* The number is to be replaced with a define */
		return 1;
	}

	return 0;
}

static inline int hyfi_is_ieee1905_pkt(struct sk_buff *skb)
{
	struct ethhdr *ethhdr = eth_hdr(skb);

	if (unlikely(ethhdr->h_proto == htons(0x893A))) {
		return 1;
	}

	return 0;
}

static inline int hyfi_is_ieee1901_pkt(struct sk_buff *skb)
{
	struct ethhdr *ethhdr = eth_hdr(skb);

	if (unlikely(ethhdr->h_proto == htons(0x88E1))) {
		return 1;
	}

	return 0;
}

static inline int hyfi_is_lldp_pkt(struct sk_buff *skb)
{
	struct ethhdr *ethhdr = eth_hdr(skb);

	if (unlikely(ethhdr->h_proto == htons(0x88CC))) {
		return 1;
	}

	return 0;
}

/*
 * IEEE 1905.1 message types
 */
typedef enum ieee1905MessageType_e {
	IEEE1905_MSG_TYPE_TOPOLOGY_DISCOVERY = 0,
	IEEE1905_MSG_TYPE_TOPOLOGY_NOTIFICATION,
	IEEE1905_MSG_TYPE_TOPOLOGY_QUERY,
	IEEE1905_MSG_TYPE_TOPOLOGY_RESPONSE,
	IEEE1905_MSG_TYPE_VENDOR_SPECIFIC,
	IEEE1905_MSG_TYPE_LINK_METRIC_QUERY,
	IEEE1905_MSG_TYPE_LINK_METRIC_RESPONSE,
	IEEE1905_MSG_TYPE_AP_AUTOCONFIGURATION_SEARCH,
	IEEE1905_MSG_TYPE_AP_AUTOCONFIGURATION_RESPONSE,
	IEEE1905_MSG_TYPE_AP_AUTOCONFIGURATION_WPS,
	IEEE1905_MSG_TYPE_AP_AUTOCONFIGURATION_RENEW
} ieee1905MessageType_e;

static inline int hyfi_ieee1905_msg_type(struct sk_buff *skb);
static inline int hyfi_ieee1905_frame_filter(struct sk_buff *skb,
		const struct net_device *dev)
{
	if (unlikely(hyfi_is_ieee1905_pkt(skb))) {
		u8 *data = (u8 *) eth_hdr(skb);
		u8 ifindex = (u8) (dev->ifindex);
		u8 flags, index_quot;

		if (!(hyfi_ieee1905_msg_type(skb) == IEEE1905_MSG_TYPE_TOPOLOGY_DISCOVERY ||
			hyfi_ieee1905_msg_type(skb) == IEEE1905_MSG_TYPE_TOPOLOGY_NOTIFICATION ||
			hyfi_ieee1905_msg_type(skb) == IEEE1905_MSG_TYPE_TOPOLOGY_QUERY ||
			hyfi_ieee1905_msg_type(skb) == IEEE1905_MSG_TYPE_TOPOLOGY_RESPONSE ||
			hyfi_ieee1905_msg_type(skb) == IEEE1905_MSG_TYPE_VENDOR_SPECIFIC ||
			hyfi_ieee1905_msg_type(skb) ==
			IEEE1905_MSG_TYPE_AP_AUTOCONFIGURATION_SEARCH ||
			hyfi_ieee1905_msg_type(skb) ==
			IEEE1905_MSG_TYPE_AP_AUTOCONFIGURATION_RENEW)) {
			return 1;
		}

		flags = *((u8 *) (data + sizeof(struct ethhdr) + 7));
		flags = flags & 0xc0;

		index_quot = dev->ifindex >> 8;
		if (index_quot < 64) {
			flags = flags | index_quot;
		}

		put_unaligned(ifindex,
			(u8 *) (data + sizeof(struct ethhdr) + 1));
		put_unaligned(flags,
			(u8 *) (data + sizeof(struct ethhdr) + 7));

		return 1;
	}

	return 0;
}

static inline int hyfi_ieee1905_msg_type(struct sk_buff *skb)
{
   if (unlikely(hyfi_is_ieee1905_pkt(skb))) {
       u8 *data = (u8 *) eth_hdr(skb);
       u8 *msg_ptr = data + ETH_HLEN + 2;
       u16 msg_type = get_unaligned((u16*) (msg_ptr));
                return htons(msg_type);
   }

   return -1;
}

#endif /* HYFI_FILTERS_H_ */
