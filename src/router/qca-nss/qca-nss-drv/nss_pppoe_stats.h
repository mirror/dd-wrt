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

#ifndef __NSS_PPPOE_STATS_H
#define __NSS_PPPOE_STATS_H

/*
 * PPPoE statistics
 */
enum nss_pppoe_stats_session {
	NSS_PPPOE_STATS_RX_PACKETS,
	NSS_PPPOE_STATS_RX_BYTES,
	NSS_PPPOE_STATS_TX_PACKETS,
	NSS_PPPOE_STATS_TX_BYTES,
	NSS_PPPOE_STATS_SESSION_WRONG_VERSION_OR_TYPE,
	NSS_PPPOE_STATS_SESSION_WRONG_CODE,
	NSS_PPPOE_STATS_SESSION_UNSUPPORTED_PPP_PROTOCOL,
	NSS_PPPOE_STATS_SESSION_MAX
};

/*
 * PPPoE session stats structure for debug interface
 */
struct nss_pppoe_stats_session_debug {
	uint64_t stats[NSS_PPPOE_STATS_SESSION_MAX];
				/* stats for the session */
	int32_t if_index;	/* net device index for the session */
	uint32_t if_num;	/* nss interface number */
	bool valid;		/* dynamic interface valid flag */
};

/*
 * PPPoE statistics APIs
 */
extern void nss_pppoe_stats_dentry_create(void);

#endif /* __NSS_PPPOE_STATS_H */
