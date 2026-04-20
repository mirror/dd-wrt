/*
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
/*
 * nss_trustsec_rx_stats.h
 *      NSS TRUSTSEC RX statistics header file.
 */

#ifndef __NSS_TRUSTSEC_RX_STATS_H
#define __NSS_TRUSTSEC_RX_STATS_H

/*
 * Trustsec TX statistics
 */
enum nss_trustsec_rx_stats {
	NSS_TRUSTSEC_RX_STATS_UNKNOWN_ETH_TYPE,
					/* Number of packets with unknown ethernet type */
	NSS_TRUSTSEC_RX_STATS_UNKNOWN_PKT,
					/* Number of packets with unknown IP packet*/
	NSS_TRUSTSEC_RX_STATS_UNKNOWN_DEST,
					/* Number of packets with unknown destination */
	NSS_TRUSTSEC_RX_STATS_IP_PARSE_FAILED,
					/* Number of packets with IP parse failed */
	NSS_TRUSTSEC_RX_STATS_WRONG_L4_TYPE,
					/* Number of packets with wrong L4 type */
	NSS_TRUSTSEC_RX_STATS_MAX
};

/*
 * Trustsec TX statistics APIs
 */
extern void nss_trustsec_rx_stats_sync(struct nss_ctx_instance *nss_ctx, struct nss_trustsec_rx_stats_sync_msg *ntsm);
extern void nss_trustsec_rx_stats_dentry_create(void);

#endif /* __NSS_TRUSTSEC_RX_STATS_H */
