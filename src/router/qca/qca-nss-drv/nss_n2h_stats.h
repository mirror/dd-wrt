/*
 ******************************************************************************
 * Copyright (c) 2017, 2019 The Linux Foundation. All rights reserved.
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

#ifndef __NSS_N2H_STATS_H
#define __NSS_N2H_STATS_H

/*
 * N2H node statistics
 */
enum nss_n2h_stats_types {
	NSS_N2H_STATS_QUEUE_DROPPED = NSS_STATS_NODE_MAX,
					/* Number of packets dropped because the exception queue is too full */
	NSS_N2H_STATS_TOTAL_TICKS,	/* Total clock ticks spend inside the N2H */
	NSS_N2H_STATS_WORST_CASE_TICKS,	/* Worst case iteration of the exception path in ticks */
	NSS_N2H_STATS_ITERATIONS,	/* Number of iterations around the N2H */

	NSS_N2H_STATS_PBUF_OCM_ALLOC_FAILS,	/* Number of pbuf ocm allocations that have failed */
	NSS_N2H_STATS_PBUF_OCM_FREE_COUNT,	/* Number of pbuf ocm free count */
	NSS_N2H_STATS_PBUF_OCM_TOTAL_COUNT,	/* Number of pbuf ocm total count */

	NSS_N2H_STATS_PBUF_DEFAULT_ALLOC_FAILS,	/* Number of pbuf default allocations that have failed */
	NSS_N2H_STATS_PBUF_DEFAULT_FREE_COUNT,	/* Number of pbuf default free count */
	NSS_N2H_STATS_PBUF_DEFAULT_TOTAL_COUNT,	/* Number of pbuf default total count */

	NSS_N2H_STATS_PAYLOAD_ALLOC_FAILS,	/* Number of pbuf allocations that have failed because there were no free payloads */
	NSS_N2H_STATS_PAYLOAD_FREE_COUNT,	/* Number of free payloads that exist */

	NSS_N2H_STATS_H2N_CONTROL_PACKETS,	/* Control packets received from HLOS */
	NSS_N2H_STATS_H2N_CONTROL_BYTES,	/* Control bytes received from HLOS */
	NSS_N2H_STATS_N2H_CONTROL_PACKETS,	/* Control packets sent to HLOS */
	NSS_N2H_STATS_N2H_CONTROL_BYTES,	/* Control bytes sent to HLOS */

	NSS_N2H_STATS_H2N_DATA_PACKETS,		/* Data packets received from HLOS */
	NSS_N2H_STATS_H2N_DATA_BYTES,		/* Data bytes received from HLOS */
	NSS_N2H_STATS_N2H_DATA_PACKETS,		/* Data packets sent to HLOS */
	NSS_N2H_STATS_N2H_DATA_BYTES,		/* Data bytes sent to HLOS */
	NSS_N2H_STATS_N2H_TOT_PAYLOADS,		/* No. of payloads in NSS */
	NSS_N2H_STATS_N2H_INTERFACE_INVALID,	/* No. of bad interface access */
	NSS_N2H_STATS_ENQUEUE_RETRIES,		/* No. of enqueue retries by N2H */

	NSS_N2H_STATS_MAX,
};

/*
 * N2H statistics APIs
 */
extern void nss_n2h_stats_sync(struct nss_ctx_instance *nss_ctx, struct nss_n2h_stats_sync *nnss);
extern void nss_n2h_stats_dentry_create(void);

#endif /* __NSS_N2H_STATS_H */
