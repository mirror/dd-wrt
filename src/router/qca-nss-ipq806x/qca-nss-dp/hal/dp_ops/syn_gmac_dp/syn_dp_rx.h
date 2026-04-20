/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 *
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

#ifndef __NSS_DP_SYN_DP_RX__
#define __NSS_DP_SYN_DP_RX__

#define SYN_DP_NAPI_BUDGET_RX		64
#define SYN_DP_RX_DESC_SIZE		128	/* Rx Descriptors needed in the descriptor pool/queue */
#define SYN_DP_RX_DESC_MAX_INDEX	(SYN_DP_RX_DESC_SIZE - 1)

/*
 * Size of sk_buff is 184B, which requires 3 cache lines
 * in ARM core (Each cache line is of size 64B). napi_gro_receive
 * and skb_put are majorly using variables from sk_buff structure
 * which falls on either first or third cache lines. So, prefetching
 * first and third cache line provides better performance.
 */
#define SYN_DP_RX_SKB_CACHE_LINE1	64
#define SYN_DP_RX_SKB_CACHE_LINE3	128

/*
 * syn_dp_rx_buf
 */
struct syn_dp_rx_buf {
	struct sk_buff *skb;	/* Buffer pointer populated to Rx/Tx dma desc */
	size_t map_addr_virt;	/* Virtual address of buffer populated to Rx/Tx dma desc */
};

/*
 * syn_dp_info_rx
 */
struct syn_dp_info_rx {
	struct napi_struct napi_rx;	/* Rx NAPI */
	void __iomem *mac_base;		/* MAC base for register read/write */
	struct dma_desc_rx *rx_desc;	/* start address of RX descriptors ring or
					   chain, this is used by the driver */
	uint32_t busy_rx_desc_cnt;	/* Number of Rx Descriptors owned by
					   DMA at any given time */
	uint32_t rx_refill_idx;		/* index of the rx descriptor owned by DMA */
	uint32_t rx_idx;		/* index of the rx descriptor next available with driver */
	struct syn_dp_rx_buf rx_buf_pool[SYN_DP_RX_DESC_SIZE];
					/* Rx skb pool helping RX DMA descriptors */
	struct nss_dp_hal_gmac_stats_rx rx_stats;
					/* GMAC driver Rx statistics */
	struct net_device *netdev;	/* Net-device corresponding to the GMAC */
	struct device *dev;		/* Platform device corresponding to the GMAC */
	struct sk_buff *head;		/* Head of the skb list in case of Scatter-Gather frame */
	struct sk_buff *tail;		/* Tail of the skb list in case of Scatter-Gather frame */
	bool page_mode;			/* page_mode: true for nr_frag and false for fraglist */
	uint32_t alloc_buf_len;		/* Skb alloc length, depends based on page/fraglist mode */
	uint32_t prev_len;		/* Stores frame_length of previous descriptor */
};

/*
 * syn_dp_rx_inc_index()
 * 	Increment Rx descriptor index
 */
static inline int syn_dp_rx_inc_index(uint32_t index, uint32_t inc)
{
	return ((index + inc) & SYN_DP_RX_DESC_MAX_INDEX);
}

#endif /*  __NSS_DP_SYN_DP_RX__ */
