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

#include <nss_dp_dev.h>
#include "syn_dma_reg.h"

/*
 * syn_dp_cfg_rx_setup_desc_queue
 *	This sets up the receive Descriptor queue in ring mode.
 */
static int syn_dp_cfg_rx_setup_desc_queue(struct syn_dp_info *dev_info)
{
	struct syn_dp_info_rx *rx_info = &dev_info->dp_info_rx;
	struct dma_desc_rx *first_desc = NULL;
	struct net_device *netdev = rx_info->netdev;
	dma_addr_t dma_addr;
	netdev_dbg(netdev, "Total size of memory required for Rx Descriptors in Ring Mode = %u\n", (uint32_t)((sizeof(struct dma_desc_rx) * SYN_DP_RX_DESC_SIZE)));

	/*
	 * Allocate cacheable descriptors for Rx
	 */
	first_desc = dma_alloc_coherent(rx_info->dev,
					sizeof(struct dma_desc_rx) * SYN_DP_RX_DESC_SIZE,
					&dma_addr, GFP_KERNEL);
	if (!first_desc) {
		netdev_dbg(netdev, "Error in Rx Descriptor Memory allocation in Ring mode\n");
		return -ENOMEM;
	}

	dev_info->rx_desc_dma_addr = dma_addr;
	rx_info->rx_desc = first_desc;
	syn_dp_gmac_rx_desc_init_ring(rx_info->rx_desc, SYN_DP_RX_DESC_SIZE);

	rx_info->rx_refill_idx = 0;
	rx_info->rx_idx = 0;
	rx_info->busy_rx_desc_cnt = 0;

	netdev_dbg(netdev, "Rx Descriptors in Ring Mode: No. of descriptors = %d base = 0x%px dma = 0x%px\n",
			SYN_DP_RX_DESC_SIZE, first_desc, (void *)dev_info->rx_desc_dma_addr);

	return NSS_DP_SUCCESS;
}

/*
 * syn_dp_cfg_rx_setup_rings
 *	Perform initial setup of Rx rings
 */
int syn_dp_cfg_rx_setup_rings(struct syn_dp_info *dev_info)
{
	int err;

	err = syn_dp_cfg_rx_setup_desc_queue(dev_info);
	if (err) {
		netdev_dbg(dev_info->dp_info_rx.netdev, "nss_dp_gmac: rx descriptor setup unsuccessfull, err code: %d", err);
		return NSS_DP_FAILURE;
	}

	if (likely(!dev_info->dp_info_rx.page_mode)) {
		syn_dp_rx_refill(&dev_info->dp_info_rx);
	} else {
		syn_dp_rx_refill_page_mode(&dev_info->dp_info_rx);
	}
	syn_init_rx_desc_base(dev_info->mac_base, dev_info->rx_desc_dma_addr);

	return NSS_DP_SUCCESS;
}

/*
 * syn_dp_cfg_rx_cleanup_rings
 *	Cleanup Synopsys GMAC Rx rings
 */
void syn_dp_cfg_rx_cleanup_rings(struct syn_dp_info *dev_info)
{
	struct syn_dp_info_rx *rx_info = &dev_info->dp_info_rx;
	uint32_t rx_skb_index;
	struct dma_desc_rx *rxdesc;
	int i;
	struct sk_buff *skb;

	/*
	 * Rx Ring cleaning
	 * We are assuming that the NAPI poll was already completed.
	 * No need of a lock here since the NAPI and interrupts have been disabled now
	 */
	rx_skb_index = rx_info->rx_idx;
	for (i = 0; i < rx_info->busy_rx_desc_cnt; i++) {
		rx_skb_index = (rx_skb_index + i) & SYN_DP_RX_DESC_MAX_INDEX;
		rxdesc = rx_info->rx_desc;

		dma_unmap_single(rx_info->dev, rxdesc->buffer1,
				 rxdesc->length, DMA_FROM_DEVICE);

		skb = rx_info->rx_buf_pool[rx_skb_index].skb;
		if (unlikely(skb != NULL)) {
			dev_kfree_skb_any(skb);
			rx_info->rx_buf_pool[rx_skb_index].skb = NULL;
		}
	}

	dma_free_coherent(rx_info->dev, (sizeof(struct dma_desc_rx) * SYN_DP_RX_DESC_SIZE),
			  rx_info->rx_desc, dev_info->rx_desc_dma_addr);
	rx_info->rx_desc = NULL;
	dev_info->rx_desc_dma_addr = (dma_addr_t)0;
}
