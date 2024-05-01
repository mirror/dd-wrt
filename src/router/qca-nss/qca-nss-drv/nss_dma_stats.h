/*
 ******************************************************************************
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * ****************************************************************************
 */

#ifndef __NSS_DMA_STATS_H
#define __NSS_DMA_STATS_H

#include <nss_cmn.h>

/**
 * nss_dma_stats_types
 *	DMA node statistics.
 */
enum nss_dma_stats_types {
	NSS_DMA_STATS_NO_REQ = NSS_STATS_NODE_MAX,	/**< Request descriptor not available. */
	NSS_DMA_STATS_NO_DESC,				/**< DMA descriptors not available. */
	NSS_DMA_STATS_NEXTHOP,				/**< Failed to retrive next hop. */
	NSS_DMA_STATS_FAIL_NEXTHOP_QUEUE,		/**< Failed to queue next hop. */
	NSS_DMA_STATS_FAIL_LINEAR_SZ,			/**< Failed to get memory for linearization. */
	NSS_DMA_STATS_FAIL_LINEAR_ALLOC,		/**< Failed to allocate buffer for linearization. */
	NSS_DMA_STATS_FAIL_LINEAR_NO_SG,		/**< Skip linearization due to non-SG packet. */
	NSS_DMA_STATS_FAIL_SPLIT_SZ,			/**< Failed to spliting buffer into multiple buffers. */
	NSS_DMA_STATS_FAIL_SPLIT_ALLOC,			/**< Failed to allocate buffer for split. */
	NSS_DMA_STATS_FAIL_SYNC_ALLOC,			/**< Failed to allocate buffer for sending statistics. */
	NSS_DMA_STATS_FAIL_CTX_ACTIVE,			/**< Failed to queue as the node is not active. */
	NSS_DMA_STATS_FAIL_HW_E0,			/**< Failed to process in HW, error code E0. */
	NSS_DMA_STATS_FAIL_HW_E1,			/**< Failed to process in HW, error code E1. */
	NSS_DMA_STATS_FAIL_HW_E2,			/**< Failed to process in HW, error code E2. */
	NSS_DMA_STATS_FAIL_HW_E3,			/**< Failed to process in HW, error code E3. */
	NSS_DMA_STATS_FAIL_HW_E4,			/**< Failed to process in HW, error code E4. */
	NSS_DMA_STATS_FAIL_HW_E5,			/**< Failed to process in HW, error code E5. */
	NSS_DMA_STATS_FAIL_HW_E6,			/**< Failed to process in HW, error code E6. */
	NSS_DMA_STATS_FAIL_HW_E7,			/**< Failed to process in HW, error code E7. */
	NSS_DMA_STATS_FAIL_HW_E8,			/**< Failed to process in HW, error code E8. */
	NSS_DMA_STATS_FAIL_HW_E9,			/**< Failed to process in HW, error code E9. */
	NSS_DMA_STATS_FAIL_HW_E10,			/**< Failed to process in HW, error code E10. */
	NSS_DMA_STATS_FAIL_HW_E11,			/**< Failed to process in HW, error code E11. */
	NSS_DMA_STATS_FAIL_HW_E12,			/**< Failed to process in HW, error code E12. */
	NSS_DMA_STATS_FAIL_HW_E13,			/**< Failed to process in HW, error code E13. */
	NSS_DMA_STATS_FAIL_HW_E14,			/**< Failed to process in HW, error code E14. */
	NSS_DMA_STATS_FAIL_HW_E15,			/**< Failed to process in HW, error code E15. */
	NSS_DMA_STATS_MAX,				/**< Maximum message type. */
};

/*
 * DMA statistics APIs
 */
extern void nss_dma_stats_sync(struct nss_ctx_instance *nss_ctx, struct nss_dma_stats *nds);
extern void nss_dma_stats_dentry_create(void);

#endif /* __NSS_DMA_STATS_H */
