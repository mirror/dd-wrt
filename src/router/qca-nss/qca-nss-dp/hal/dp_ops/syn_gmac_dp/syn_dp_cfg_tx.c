/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * syn_dp_cfg_tx_setup_desc_queue
 *	This sets up the transmit Descriptor queue in ring mode.
 */
static int syn_dp_cfg_tx_setup_desc_queue(struct syn_dp_info *dev_info)
{
	struct syn_dp_info_tx *tx_info = &dev_info->dp_info_tx;
	struct net_device *netdev = tx_info->netdev;
	struct dma_desc_tx *first_desc = NULL;
	dma_addr_t dma_addr;

	netdev_dbg(netdev, "Total size of memory required for Tx Descriptors in Ring Mode = %u\n", (uint32_t)((sizeof(struct dma_desc_tx) * SYN_DP_TX_DESC_SIZE)));

	first_desc = dma_alloc_coherent(tx_info->dev, sizeof(struct dma_desc_tx) * SYN_DP_TX_DESC_SIZE, &dma_addr, GFP_KERNEL);
	if (!first_desc) {
		netdev_dbg(netdev, "Error in Tx Descriptors memory allocation\n");
		return -ENOMEM;
	}

	tx_info->tx_desc = first_desc;
	dev_info->tx_desc_dma_addr = dma_addr;
	netdev_dbg(netdev, "Tx Descriptors in Ring Mode: No. of descriptors = %d base = 0x%px dma = 0x%px\n"
			, SYN_DP_TX_DESC_SIZE, first_desc, (void *)dma_addr);

	syn_dp_gmac_tx_desc_init_ring(tx_info->tx_desc, SYN_DP_TX_DESC_SIZE);

	tx_info->tx_comp_idx = 0;
	tx_info->tx_idx = 0;
	tx_info->busy_tx_desc_cnt = 0;

	return NSS_DP_SUCCESS;
}

/*
 * syn_dp_cfg_tx_setup_rings
 *	Perform initial setup of Tx rings
 */
int syn_dp_cfg_tx_setup_rings(struct syn_dp_info *dev_info)
{
	int err;

	err = syn_dp_cfg_tx_setup_desc_queue(dev_info);
	if (err) {
		netdev_dbg(dev_info->dp_info_tx.netdev, "nss_dp_gmac: tx descriptor setup unsuccessfull, err code: %d", err);
		return NSS_DP_FAILURE;
	}

	syn_init_tx_desc_base(dev_info->mac_base, dev_info->tx_desc_dma_addr);

	return NSS_DP_SUCCESS;
}

/*
 * syn_dp_cfg_tx_cleanup_rings
 *	Cleanup Synopsys GMAC Tx rings
 */
void syn_dp_cfg_tx_cleanup_rings(struct syn_dp_info *dev_info)
{
	struct syn_dp_info_tx *tx_info = &dev_info->dp_info_tx;
	uint32_t tx_skb_index;
	struct dma_desc_tx *txdesc;
	int i;
	struct sk_buff *skb;
	uint32_t busy_tx_desc_cnt = atomic_read((atomic_t *)&tx_info->busy_tx_desc_cnt);

	/*
	 * Tx Ring cleaning
	 */
	tx_skb_index = syn_dp_tx_comp_index_get(tx_info);
	for (i = 0; i < busy_tx_desc_cnt; i++) {
		tx_skb_index = syn_dp_tx_inc_index(tx_skb_index, i);
		txdesc = tx_info->tx_desc;

		skb = tx_info->tx_buf_pool[tx_skb_index].skb;
		if (unlikely(skb != NULL)) {
			dev_kfree_skb_any(skb);
			tx_info->tx_buf_pool[tx_skb_index].skb = NULL;
		}
	}

	dma_free_coherent(tx_info->dev, (sizeof(struct dma_desc_tx) * SYN_DP_TX_DESC_SIZE),
				tx_info->tx_desc, dev_info->tx_desc_dma_addr);
}
