/*
 ******************************************************************************
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * ****************************************************************************
 */

#ifndef __NSS_L2TPV2_STATS_H
#define __NSS_L2TPV2_STATS_H

/*
 * l2tpv2 debug stats
 */
enum nss_l2tpv2_stats_session {
	NSS_L2TPV2_STATS_SESSION_RX_PPP_LCP_PKTS,	/* Number of ppp lcp packets received */
	NSS_L2TPV2_STATS_SESSION_RX_EXP_DATA_PKTS,	/* Number of RX exceptioned packets */
	NSS_L2TPV2_STATS_SESSION_ENCAP_PBUF_ALLOC_FAIL_PKTS,	/* Number of times packet buffer allocation failed during encap */
	NSS_L2TPV2_STATS_SESSION_DECAP_PBUF_ALLOC_FAIL_PKTS,	/* Number of times packet buffer allocation failed during decap */
	NSS_L2TPV2_STATS_SESSION_MAX
};

struct nss_l2tpv2_stats_session_debug {
	uint64_t stats[NSS_L2TPV2_STATS_SESSION_MAX];
	int32_t if_index;
	uint32_t if_num; /* nss interface number */
	bool valid;
};

/*
 * l2tpv2 statistics APIs
 */
extern void nss_l2tpv2_stats_dentry_create(void);

#endif /* __NSS_L2TPV2_STATS_H */
