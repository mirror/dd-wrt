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

#ifndef __NSS_DP_SYN_DP_TX__
#define __NSS_DP_SYN_DP_TX__

#define SYN_DP_NAPI_BUDGET_TX		64
#define SYN_DP_TX_DESC_SIZE		1024	/* Tx Descriptors needed in the descriptor pool/queue */
#define SYN_DP_TX_DESC_MAX_INDEX	(SYN_DP_TX_DESC_SIZE - 1)
#define SYN_DP_TX_INVALID_DESC_INDEX	SYN_DP_TX_DESC_SIZE

/*
 * syn_dp_tx_buf
 */
struct syn_dp_tx_buf {
	struct sk_buff *skb;	/* Buffer pointer populated to Tx dma desc */
	uint32_t len;		/* Length of the buffer provided to descriptor */
	size_t shinfo_addr_virt;	/* Buffer address to prefetch the shinfo
						during Tx complete*/
};

/*
 * syn_dp_info_tx
 */
struct syn_dp_info_tx {
	struct napi_struct napi_tx;	/* Tx NAPI */
	void __iomem *mac_base;		/* MAC base for register read/write */
	struct dma_desc_tx *tx_desc;	/* start address of TX descriptors ring or
						chain, this is used by the driver */
	uint32_t busy_tx_desc_cnt;	/* Number of Tx Descriptors owned by
						DMA at any given time */
	uint32_t tx_comp_idx;		/* index of the tx descriptor owned by DMA */
	uint32_t tx_idx;		/* index of the tx descriptor next available with driver */
	struct syn_dp_tx_buf tx_buf_pool[SYN_DP_TX_DESC_SIZE];
					/* Tx skb pool helping TX DMA descriptors */
	struct nss_dp_hal_gmac_stats_tx tx_stats;
					/* GMAC driver Tx statistics */
	struct net_device *netdev;	/* Net-device corresponding to the GMAC */
	struct device *dev;		/* Platform device corresponding to the GMAC */
	struct sk_buff *skb_free_list[SYN_DP_NAPI_BUDGET_TX];
					/* Array to hold SKBs before free during Tx completion */
	size_t shinfo_addr_virt[SYN_DP_NAPI_BUDGET_TX];
					/* Array to hold SKB end pointer to be
						prefetched during Tx completion */
};

/*
 * syn_dp_tx_inc_index()
 * 	Increment Tx descriptor index
 */
static inline uint32_t syn_dp_tx_inc_index(uint32_t index, uint32_t inc)
{
	return ((index + inc) & SYN_DP_TX_DESC_MAX_INDEX);
}

/*
 * syn_dp_tx_comp_desc_get()
 * 	Get the Tx completed descriptor
 */
static inline struct dma_desc_tx *syn_dp_tx_comp_desc_get(struct syn_dp_info_tx *tx_info)
{
	return tx_info->tx_desc + tx_info->tx_comp_idx;
}

/*
 * syn_dp_tx_comp_index_get()
 * 	Get the Tx completion index
 */
static inline uint32_t syn_dp_tx_comp_index_get(struct syn_dp_info_tx *tx_info)
{
	return tx_info->tx_comp_idx;
}

#endif /*  __NSS_DP_SYN_DP_TX__ */
