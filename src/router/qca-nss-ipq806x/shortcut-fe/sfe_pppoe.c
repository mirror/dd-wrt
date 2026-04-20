/*
 * sfe_pppoe.c
 *     API for shortcut forwarding engine PPPoE flows
 *
 * Copyright (c) 2021-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <linux/skbuff.h>
#include <linux/if_pppox.h>
#include <linux/ppp_defs.h>
#include <asm/unaligned.h>

#include "sfe_debug.h"
#include "sfe_api.h"
#include "sfe.h"
#include "sfe_pppoe.h"

/*
 * sfe_pppoe_br_accel_mode controls how to accelerate PPPoE bridge flow.
 * - SFE_PPPOE_BR_ACCEL_MODE_EN_5T: 5-tuple (src_ip, dest_ip, src_port, dest_port, protocol) acceleration
 * - SFE_PPPOE_BR_ACCEL_MODE_EN_3T: 3-tuple (src_ip, dest_ip, PPPoE session id) acceleration
 * - SFE_PPPOE_BR_ACCEL_MODE_DISABLED: No acceleration
 */
static sfe_pppoe_br_accel_mode_t sfe_pppoe_br_accel_mode __read_mostly = SFE_PPPOE_BR_ACCEL_MODE_EN_5T;

/*
 * sfe_pppoe_get_br_accel_mode()
 *	Gets PPPoE bridge acceleration mode
 */
sfe_pppoe_br_accel_mode_t sfe_pppoe_get_br_accel_mode(void)
{
	return sfe_pppoe_br_accel_mode;
}
EXPORT_SYMBOL(sfe_pppoe_get_br_accel_mode);

/*
 * sfe_pppoe_set_br_accel_mode()
 *	Sets PPPoE bridge acceleration mode
 */
int sfe_pppoe_set_br_accel_mode(sfe_pppoe_br_accel_mode_t mode)
{
	if (mode >= SFE_PPPOE_BR_ACCEL_MODE_MAX) {
		return -1;
	}

	sfe_pppoe_br_accel_mode = mode;
	return 0;
}

/*
 * sfe_pppoe_add_header()
 *	Add PPPoE header.
 *
 * skb->data will point to PPPoE header after the function
 */
void sfe_pppoe_add_header(struct sk_buff *skb, u16 pppoe_session_id, u16 ppp_protocol)
{
	struct pppoe_hdr *ph;
	unsigned char *pp;
	unsigned int data_len;

	/*
	 * Insert the PPP header protocol
	 */
	pp = __skb_push(skb, 2);
	put_unaligned_be16(ppp_protocol, pp);

	data_len = skb->len;

	ph = (struct pppoe_hdr *)__skb_push(skb, sizeof(*ph));
	skb_reset_network_header(skb);

	/*
	 * Headers in skb will look like in below sequence
	 *	| PPPoE hdr(6 bytes) | PPP hdr (2 bytes) | L3 hdr |
	 *
	 *	The length field in the PPPoE header indicates the length of the PPPoE payload which
	 *	consists of a 2-byte PPP header plus a skb->len.
	 */
	ph->ver = 1;
	ph->type = 1;
	ph->code = 0;
	ph->sid = htons(pppoe_session_id);
	ph->length = htons(data_len);
	skb->protocol = htons(ETH_P_PPP_SES);
}

/*
 * sfe_pppoe_parse_hdr()
 *	Parse PPPoE header
 *
 * Returns true if the packet is good for further processing.
 */
bool sfe_pppoe_parse_hdr(struct sk_buff *skb, struct sfe_l2_info *l2_info)
{
	unsigned int len;
	int pppoe_len;
	struct sfe_ppp_hdr *ppp;
	struct pppoe_hdr *ph = pppoe_hdr(skb);

	/*
	 * Check that we have space for PPPoE header here.
	 */
	if (unlikely(!pskb_may_pull(skb, (sizeof(struct pppoe_hdr) + sizeof(struct sfe_ppp_hdr))))) {
		DEBUG_TRACE("%px: packet too short for PPPoE header\n", skb);
		return false;
	}

	len = skb->len;
	pppoe_len = ntohs(ph->length);
	if (unlikely(len < pppoe_len)) {
		DEBUG_TRACE("%px: len: %u is too short to %u\n", skb, len, pppoe_len);
		return false;
	}

	ppp = (struct sfe_ppp_hdr *)((u8*)ph + sizeof(*ph));

	/*
	 * Converting PPP protocol values to ether type protocol values
	 */
	switch(ntohs(ppp->protocol)) {
	case PPP_IP:
		sfe_l2_protocol_set(l2_info, ETH_P_IP);
		break;

	case PPP_IPV6:
		sfe_l2_protocol_set(l2_info, ETH_P_IPV6);
		break;

	case PPP_LCP:
		DEBUG_TRACE("%px: LCP packets are not supported in SFE\n", skb);
		return false;

	default:
		DEBUG_TRACE("%px: Unsupported protocol : %d in PPP header\n", skb, ntohs(ppp->protocol));
		return false;
	}

	sfe_l2_parse_flag_set(l2_info, SFE_L2_PARSE_FLAGS_PPPOE_INGRESS);
	sfe_l2_pppoe_session_id_set(l2_info, ntohs(ph->sid));

	/*
	 * strip PPPoE header
	 */
	__skb_pull(skb, (sizeof(struct pppoe_hdr) + sizeof(struct sfe_ppp_hdr)));
	skb_reset_network_header(skb);

	return true;
}

/*
 * sfe_pppoe_undo_parse()
 *	undo changes done to skb during PPPoE parsing
 */
void sfe_pppoe_undo_parse(struct sk_buff *skb, struct sfe_l2_info *l2_info)
{
	if (sfe_l2_parse_flag_check(l2_info, SFE_L2_PARSE_FLAGS_PPPOE_INGRESS)) {
		skb->protocol = htons(ETH_P_PPP_SES);
		__skb_push(skb, (sizeof(struct pppoe_hdr) + sizeof(struct sfe_ppp_hdr)));
	}
}
