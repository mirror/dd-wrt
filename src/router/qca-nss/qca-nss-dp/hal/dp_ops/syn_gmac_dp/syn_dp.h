/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef __NSS_DP_SYN_DP__
#define __NSS_DP_SYN_DP__

#include "syn_dma_desc.h"
#include "syn_dp_rx.h"
#include "syn_dp_tx.h"

/*
 * The driver uses kernel DMA constructs that assume an architecture
 * where the view of physical addresses is consistent between SoC and
 * IO device(SynopsysGMAC).
 * Note that this may not be compatible for platforms where this
 * assumption is not true, for example IO devices with IOMMU support.
 */
#if defined(CONFIG_ARM_SMMU) || \
	defined(CONFIG_IOMMU_SUPPORT)
#error "Build Error: Platform is enabled with IOMMU/SMMU support."
#endif

#define SYN_DP_MINI_JUMBO_FRAME_MTU	1978
#define SYN_DP_MAX_DESC_BUFF_LEN	0x1FFF	/* Max size of buffer that can be programed into one field of desc */
#define SYN_DP_SKB_ALLOC_SIZE		(SYN_DP_MINI_JUMBO_FRAME_MTU + NET_IP_ALIGN)
#define SYN_DP_SKB_HEADROOM		128
#define SYN_DP_PAGE_MODE_SKB_SIZE	256	/* SKB head buffer size for page mode */
#define SYN_DP_QUEUE_INDEX		0	/* Only one Tx DMA channel 0 enabled */

/*
 * syn_dp_info
 *	Synopysys GMAC Dataplane information
 */
struct syn_dp_info {
	struct syn_dp_info_rx dp_info_rx;
	struct syn_dp_info_tx dp_info_tx;
	void __iomem *mac_base;
	dma_addr_t rx_desc_dma_addr;
	dma_addr_t tx_desc_dma_addr;
	int napi_added;
};

/*
 * GMAC TX/Rx APIs
 */
int syn_dp_cfg_rx_setup_rings(struct syn_dp_info *dev_info);
void syn_dp_cfg_rx_cleanup_rings(struct syn_dp_info *dev_info);
int syn_dp_cfg_tx_setup_rings(struct syn_dp_info *dev_info);
void syn_dp_cfg_tx_cleanup_rings(struct syn_dp_info *dev_info);

int syn_dp_rx(struct syn_dp_info_rx *rx_info, int budget);
int syn_dp_rx_refill(struct syn_dp_info_rx *rx_info);
int syn_dp_rx_refill_page_mode(struct syn_dp_info_rx *rx_info);
int syn_dp_tx(struct syn_dp_info_tx *tx_info, struct sk_buff *skb);
int syn_dp_tx_complete(struct syn_dp_info_tx *tx_info, int budget);

#endif /*  __NSS_DP_SYN_DP__ */
