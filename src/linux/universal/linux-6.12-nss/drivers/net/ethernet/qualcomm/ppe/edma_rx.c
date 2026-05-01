// SPDX-License-Identifier: GPL-2.0-only
/* Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 */

/* Provides APIs to alloc Rx Buffers, reap the buffers, receive and
 * process linear and Scatter Gather packets.
 */

#include <linux/dma-mapping.h>
#include <linux/etherdevice.h>
#include <linux/irqreturn.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/platform_device.h>
#include <linux/printk.h>
#include <linux/regmap.h>

#include "edma.h"
#include "edma_cfg_rx.h"
#include "edma_port.h"
#include "ppe.h"
#include "ppe_regs.h"

static int edma_rx_alloc_buffer_list(struct edma_rxfill_ring *rxfill_ring, int alloc_count)
{
	struct edma_rxfill_stats *rxfill_stats = &rxfill_ring->rxfill_stats;
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	u32 rx_alloc_size = rxfill_ring->alloc_size;
	struct regmap *regmap = ppe_dev->regmap;
	bool page_mode = rxfill_ring->page_mode;
	struct edma_rxfill_desc *rxfill_desc;
	u32 buf_len = rxfill_ring->buf_len;
	struct device *dev = ppe_dev->dev;
	u16 prod_idx, start_idx;
	u16 num_alloc = 0;
	u32 reg;

	prod_idx = rxfill_ring->prod_idx;
	start_idx = prod_idx;

	while (likely(alloc_count--)) {
		dma_addr_t buff_addr;
		struct sk_buff *skb;
		struct page *pg;

		rxfill_desc = EDMA_RXFILL_DESC(rxfill_ring, prod_idx);

		skb = dev_alloc_skb(rx_alloc_size);
		if (unlikely(!skb)) {
			u64_stats_update_begin(&rxfill_stats->syncp);
			++rxfill_stats->alloc_failed;
			u64_stats_update_end(&rxfill_stats->syncp);
			break;
		}

		skb_reserve(skb, EDMA_RX_SKB_HEADROOM + NET_IP_ALIGN);

		if (likely(!page_mode)) {
			buff_addr = dma_map_single(dev, skb->data, rx_alloc_size, DMA_FROM_DEVICE);
			if (dma_mapping_error(dev, buff_addr)) {
				dev_dbg(dev, "edma_context:%p Unable to dma for non page mode",
					edma_ctx);
				dev_kfree_skb_any(skb);
				break;
			}
		} else {
			pg = alloc_page(GFP_ATOMIC);
			if (unlikely(!pg)) {
				u64_stats_update_begin(&rxfill_stats->syncp);
				++rxfill_stats->page_alloc_failed;
				u64_stats_update_end(&rxfill_stats->syncp);
				dev_kfree_skb_any(skb);
				dev_dbg(dev, "edma_context:%p Unable to allocate page",
					edma_ctx);
				break;
			}

			buff_addr = dma_map_page(dev, pg, 0, PAGE_SIZE, DMA_FROM_DEVICE);
			if (dma_mapping_error(dev, buff_addr)) {
				dev_dbg(dev, "edma_context:%p Mapping error for page mode",
					edma_ctx);
				__free_page(pg);
				dev_kfree_skb_any(skb);
				break;
			}

			skb_fill_page_desc(skb, 0, pg, 0, PAGE_SIZE);
		}

		EDMA_RXFILL_BUFFER_ADDR_SET(rxfill_desc, buff_addr);

		EDMA_RXFILL_OPAQUE_LO_SET(rxfill_desc, skb);
#ifdef __LP64__
		EDMA_RXFILL_OPAQUE_HI_SET(rxfill_desc, skb);
#endif
		EDMA_RXFILL_PACKET_LEN_SET(rxfill_desc,
					   (u32)(buf_len) & EDMA_RXFILL_BUF_SIZE_MASK);
		prod_idx = (prod_idx + 1) & EDMA_RX_RING_SIZE_MASK;
		num_alloc++;
	}

	if (likely(num_alloc)) {
		reg = EDMA_BASE_OFFSET + EDMA_REG_RXFILL_PROD_IDX(rxfill_ring->ring_id);
		regmap_write(regmap, reg, prod_idx);
		rxfill_ring->prod_idx = prod_idx;
	}

	return num_alloc;
}

/**
 * edma_rx_alloc_buffer - EDMA Rx alloc buffer.
 * @rxfill_ring: EDMA Rxfill ring
 * @alloc_count: Number of rings to alloc
 *
 * Alloc Rx buffers for RxFill ring.
 *
 * Return the number of rings allocated.
 */
int edma_rx_alloc_buffer(struct edma_rxfill_ring *rxfill_ring, int alloc_count)
{
	return edma_rx_alloc_buffer_list(rxfill_ring, alloc_count);
}

/* Mark ip_summed appropriately in the skb as per the L3/L4 checksum
 * status in descriptor.
 */
static void edma_rx_checksum_verify(struct edma_rxdesc_pri *rxdesc_pri,
				    struct sk_buff *skb)
{
	u8 pid = EDMA_RXDESC_PID_GET(rxdesc_pri);

	skb_checksum_none_assert(skb);

	if (likely(EDMA_RX_PID_IS_IPV4(pid))) {
		if (likely(EDMA_RXDESC_L3CSUM_STATUS_GET(rxdesc_pri)) &&
		    likely(EDMA_RXDESC_L4CSUM_STATUS_GET(rxdesc_pri)))
			skb->ip_summed = CHECKSUM_UNNECESSARY;
	} else if (likely(EDMA_RX_PID_IS_IPV6(pid))) {
		if (likely(EDMA_RXDESC_L4CSUM_STATUS_GET(rxdesc_pri)))
			skb->ip_summed = CHECKSUM_UNNECESSARY;
	}
}

static void edma_rx_process_last_segment(struct edma_rxdesc_ring *rxdesc_ring,
					 struct edma_rxdesc_pri *rxdesc_pri,
					 struct sk_buff *skb)
{
	bool page_mode = rxdesc_ring->rxfill->page_mode;
	struct edma_port_pcpu_stats *pcpu_stats;
	struct edma_port_rx_stats *rx_stats;
	struct edma_port_priv *port_dev;
	struct sk_buff *skb_head;
	struct net_device *dev;
	u32 pkt_length;

	/* Get packet length. */
	pkt_length = EDMA_RXDESC_PACKET_LEN_GET(rxdesc_pri);

	skb_head = rxdesc_ring->head;
	dev = skb_head->dev;

	/* Check Rx checksum offload status. */
	if (likely(dev->features & NETIF_F_RXCSUM))
		edma_rx_checksum_verify(rxdesc_pri, skb_head);

	/* Get stats for the netdevice. */
	port_dev = netdev_priv(dev);
	pcpu_stats = &port_dev->pcpu_stats;
	rx_stats = this_cpu_ptr(pcpu_stats->rx_stats);

	if (unlikely(page_mode)) {
		if (unlikely(!pskb_may_pull(skb_head, ETH_HLEN))) {
			/* Discard the SKB that we have been building,
			 * in addition to the SKB linked to current descriptor.
			 */
			dev_kfree_skb_any(skb_head);
			rxdesc_ring->head = NULL;
			rxdesc_ring->last = NULL;
			rxdesc_ring->pdesc_head = NULL;

			u64_stats_update_begin(&rx_stats->syncp);
			rx_stats->rx_nr_frag_headroom_err++;
			u64_stats_update_end(&rx_stats->syncp);

			return;
		}
	}

	if (unlikely(!pskb_pull(skb_head, EDMA_RXDESC_DATA_OFFSET_GET(rxdesc_ring->pdesc_head)))) {
		dev_kfree_skb_any(skb_head);
		rxdesc_ring->head = NULL;
		rxdesc_ring->last = NULL;
		rxdesc_ring->pdesc_head = NULL;

		u64_stats_update_begin(&rx_stats->syncp);
		rx_stats->rx_nr_frag_headroom_err++;
		u64_stats_update_end(&rx_stats->syncp);

		return;
	}

	u64_stats_update_begin(&rx_stats->syncp);
	rx_stats->rx_pkts++;
	rx_stats->rx_bytes += skb_head->len;
	rx_stats->rx_nr_frag_pkts += (u64)page_mode;
	rx_stats->rx_fraglist_pkts += (u64)(!page_mode);
	u64_stats_update_end(&rx_stats->syncp);

	pr_debug("edma_context:%p skb:%p Jumbo pkt_length:%u\n",
		 edma_ctx, skb_head, skb_head->len);

	skb_head->protocol = eth_type_trans(skb_head, dev);

	/* Send packet up the stack. */
	if (dev->features & NETIF_F_GRO)
		napi_gro_receive(&rxdesc_ring->napi, skb_head);
	else
		netif_receive_skb(skb_head);

	rxdesc_ring->head = NULL;
	rxdesc_ring->last = NULL;
	rxdesc_ring->pdesc_head = NULL;
}

static void edma_rx_handle_frag_list(struct edma_rxdesc_ring *rxdesc_ring,
				     struct edma_rxdesc_pri *rxdesc_pri,
				     struct sk_buff *skb)
{
	u32 pkt_length;

	/* Get packet length. */
	pkt_length = EDMA_RXDESC_PACKET_LEN_GET(rxdesc_pri);
	pr_debug("edma_context:%p skb:%p fragment pkt_length:%u\n",
		 edma_ctx, skb, pkt_length);

	if (!(rxdesc_ring->head)) {
		skb_put(skb, pkt_length);
		rxdesc_ring->head = skb;
		rxdesc_ring->last = NULL;
		rxdesc_ring->pdesc_head = rxdesc_pri;

		return;
	}

	/* Append it to the fraglist of head if this is second frame
	 * If not second frame append to tail.
	 */
	skb_put(skb, pkt_length);
	if (!skb_has_frag_list(rxdesc_ring->head))
		skb_shinfo(rxdesc_ring->head)->frag_list = skb;
	else
		rxdesc_ring->last->next = skb;

	rxdesc_ring->last = skb;
	rxdesc_ring->last->next = NULL;
	rxdesc_ring->head->len += pkt_length;
	rxdesc_ring->head->data_len += pkt_length;
	rxdesc_ring->head->truesize += skb->truesize;

	/* If there are more segments for this packet,
	 * then we have nothing to do. Otherwise process
	 * last segment and send packet to stack.
	 */
	if (EDMA_RXDESC_MORE_BIT_GET(rxdesc_pri))
		return;

	edma_rx_process_last_segment(rxdesc_ring, rxdesc_pri, skb);
}

static void edma_rx_handle_nr_frags(struct edma_rxdesc_ring *rxdesc_ring,
				    struct edma_rxdesc_pri *rxdesc_pri,
				    struct sk_buff *skb)
{
	skb_frag_t *frag = NULL;
	u32 pkt_length;

	/* Get packet length. */
	pkt_length = EDMA_RXDESC_PACKET_LEN_GET(rxdesc_pri);
	pr_debug("edma_context:%p skb:%p fragment pkt_length:%u\n",
		 edma_ctx, skb, pkt_length);

	if (!(rxdesc_ring->head)) {
		skb->len = pkt_length;
		skb->data_len = pkt_length;
		skb->truesize = SKB_TRUESIZE(PAGE_SIZE);
		rxdesc_ring->head = skb;
		rxdesc_ring->last = NULL;
		rxdesc_ring->pdesc_head = rxdesc_pri;

		return;
	}

	frag = &skb_shinfo(skb)->frags[0];

	/* Append current frag at correct index as nr_frag of parent. */
	skb_add_rx_frag(rxdesc_ring->head, skb_shinfo(rxdesc_ring->head)->nr_frags,
			skb_frag_page(frag), 0, pkt_length, PAGE_SIZE);
	skb_shinfo(skb)->nr_frags = 0;

	/* Free the SKB after we have appended its frag page to the head skb. */
	dev_kfree_skb_any(skb);

	/* If there are more segments for this packet,
	 * then we have nothing to do. Otherwise process
	 * last segment and send packet to stack.
	 */
	if (EDMA_RXDESC_MORE_BIT_GET(rxdesc_pri))
		return;

	edma_rx_process_last_segment(rxdesc_ring, rxdesc_pri, skb);
}

static bool edma_rx_handle_linear_packets(struct edma_rxdesc_ring *rxdesc_ring,
					  struct edma_rxdesc_pri *rxdesc_pri,
					  struct sk_buff *skb)
{
	bool page_mode = rxdesc_ring->rxfill->page_mode;
	struct edma_port_pcpu_stats *pcpu_stats;
	struct edma_port_rx_stats *rx_stats;
	struct edma_port_priv *port_dev;
	skb_frag_t *frag = NULL;
	u32 pkt_length;

	/* Get stats for the netdevice. */
	port_dev = netdev_priv(skb->dev);
	pcpu_stats = &port_dev->pcpu_stats;
	rx_stats = this_cpu_ptr(pcpu_stats->rx_stats);

	/* Get packet length. */
	pkt_length = EDMA_RXDESC_PACKET_LEN_GET(rxdesc_pri);

	if (likely(!page_mode)) {
		skb_put(skb, pkt_length);
		goto send_to_stack;
	}

	/* Handle linear packet in page mode. */
	frag = &skb_shinfo(skb)->frags[0];
	skb_add_rx_frag(skb, 0, skb_frag_page(frag), 0, pkt_length, PAGE_SIZE);

	/* Pull ethernet header into SKB data area for header processing. */
	if (unlikely(!pskb_may_pull(skb, ETH_HLEN))) {
		u64_stats_update_begin(&rx_stats->syncp);
		rx_stats->rx_nr_frag_headroom_err++;
		u64_stats_update_end(&rx_stats->syncp);
		dev_kfree_skb_any(skb);

		return false;
	}

send_to_stack:

	__skb_pull(skb, EDMA_RXDESC_DATA_OFFSET_GET(rxdesc_pri));

	/* Check Rx checksum offload status. */
	if (likely(skb->dev->features & NETIF_F_RXCSUM))
		edma_rx_checksum_verify(rxdesc_pri, skb);

	u64_stats_update_begin(&rx_stats->syncp);
	rx_stats->rx_pkts++;
	rx_stats->rx_bytes += pkt_length;
	rx_stats->rx_nr_frag_pkts += (u64)page_mode;
	u64_stats_update_end(&rx_stats->syncp);

	skb->protocol = eth_type_trans(skb, skb->dev);
	if (skb->dev->features & NETIF_F_GRO)
		napi_gro_receive(&rxdesc_ring->napi, skb);
	else
		netif_receive_skb(skb);

	netdev_dbg(skb->dev, "edma_context:%p, skb:%p pkt_length:%u\n",
		   edma_ctx, skb, skb->len);

	return true;
}

static struct net_device *edma_rx_get_src_dev(struct edma_rxdesc_stats *rxdesc_stats,
					      struct edma_rxdesc_pri *rxdesc_pri,
					      struct sk_buff *skb)
{
	u32 src_info = EDMA_RXDESC_SRC_INFO_GET(rxdesc_pri);
	struct edma_hw_info *hw_info = edma_ctx->hw_info;
	struct net_device *ndev = NULL;
	u8 src_port_num;

	/* Check src_info. */
	if (likely((src_info & EDMA_RXDESC_SRCINFO_TYPE_MASK)
	    == EDMA_RXDESC_SRCINFO_TYPE_PORTID)) {
		src_port_num = src_info & EDMA_RXDESC_PORTNUM_BITS;
	} else {
		if (net_ratelimit()) {
			pr_warn("Invalid src info_type:0x%x. Drop skb:%p\n",
				(src_info & EDMA_RXDESC_SRCINFO_TYPE_MASK), skb);
		}

		u64_stats_update_begin(&rxdesc_stats->syncp);
		++rxdesc_stats->src_port_inval_type;
		u64_stats_update_end(&rxdesc_stats->syncp);

		return NULL;
	}

	/* Packet with PP source. */
	if (likely(src_port_num <= hw_info->max_ports)) {
		if (unlikely(src_port_num < EDMA_START_IFNUM)) {
			if (net_ratelimit())
				pr_warn("Port number error :%d. Drop skb:%p\n",
					src_port_num, skb);

			u64_stats_update_begin(&rxdesc_stats->syncp);
			++rxdesc_stats->src_port_inval;
			u64_stats_update_end(&rxdesc_stats->syncp);

			return NULL;
		}

		/* Get netdev for this port using the source port
		 * number as index into the netdev array. We need to
		 * subtract one since the indices start form '0' and
		 * port numbers start from '1'.
		 */
		ndev = edma_ctx->netdev_arr[src_port_num - 1];
	}

	if (likely(ndev))
		return ndev;

	if (net_ratelimit())
		pr_warn("Netdev Null src_info_type:0x%x src port num:%d Drop skb:%p\n",
			(src_info & EDMA_RXDESC_SRCINFO_TYPE_MASK),
			src_port_num, skb);

	u64_stats_update_begin(&rxdesc_stats->syncp);
	++rxdesc_stats->src_port_inval_netdev;
	u64_stats_update_end(&rxdesc_stats->syncp);

	return NULL;
}

static int edma_rx_reap(struct edma_rxdesc_ring *rxdesc_ring, int budget)
{
	struct edma_rxdesc_stats *rxdesc_stats = &rxdesc_ring->rxdesc_stats;
	u32 alloc_size = rxdesc_ring->rxfill->alloc_size;
	bool page_mode = rxdesc_ring->rxfill->page_mode;
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct edma_rxdesc_pri *next_rxdesc_pri;
	struct regmap *regmap = ppe_dev->regmap;
	struct device *dev = ppe_dev->dev;
	u32 prod_idx, cons_idx, end_idx;
	u32 work_to_do, work_done = 0;
	struct sk_buff *next_skb;
	u32 work_leftover, reg;

	/* Get Rx ring producer and consumer indices. */
	cons_idx = rxdesc_ring->cons_idx;

	if (likely(rxdesc_ring->work_leftover > EDMA_RX_MAX_PROCESS)) {
		work_to_do = rxdesc_ring->work_leftover;
	} else {
		reg = EDMA_BASE_OFFSET + EDMA_REG_RXDESC_PROD_IDX(rxdesc_ring->ring_id);
		regmap_read(regmap, reg, &prod_idx);
		prod_idx = prod_idx & EDMA_RXDESC_PROD_IDX_MASK;
		work_to_do = EDMA_DESC_AVAIL_COUNT(prod_idx,
						   cons_idx, EDMA_RX_RING_SIZE);
		rxdesc_ring->work_leftover = work_to_do;
	}

	if (work_to_do > budget)
		work_to_do = budget;

	rxdesc_ring->work_leftover -= work_to_do;
	end_idx = (cons_idx + work_to_do) & EDMA_RX_RING_SIZE_MASK;
	next_rxdesc_pri = EDMA_RXDESC_PRI_DESC(rxdesc_ring, cons_idx);

	/* Get opaque from RXDESC. */
	next_skb = (struct sk_buff *)EDMA_RXDESC_OPAQUE_GET(next_rxdesc_pri);

	work_leftover = work_to_do & (EDMA_RX_MAX_PROCESS - 1);
	while (likely(work_to_do--)) {
		struct edma_rxdesc_pri *rxdesc_pri;
		struct net_device *ndev;
		struct sk_buff *skb;
		dma_addr_t dma_addr;

		skb = next_skb;
		rxdesc_pri = next_rxdesc_pri;
		dma_addr = EDMA_RXDESC_BUFFER_ADDR_GET(rxdesc_pri);

		if (!page_mode)
			dma_unmap_single(dev, dma_addr, alloc_size,
					 DMA_TO_DEVICE);
		else
			dma_unmap_page(dev, dma_addr, PAGE_SIZE, DMA_TO_DEVICE);

		/* Update consumer index. */
		cons_idx = (cons_idx + 1) & EDMA_RX_RING_SIZE_MASK;

		/* Get the next Rx descriptor. */
		next_rxdesc_pri = EDMA_RXDESC_PRI_DESC(rxdesc_ring, cons_idx);

		/* Handle linear packets or initial segments first. */
		if (likely(!(rxdesc_ring->head))) {
			ndev = edma_rx_get_src_dev(rxdesc_stats, rxdesc_pri, skb);
			if (unlikely(!ndev)) {
				dev_kfree_skb_any(skb);
				goto next_rx_desc;
			}

			/* Update skb fields for head skb. */
			skb->dev = ndev;
			skb->skb_iif = ndev->ifindex;

			/* Handle linear packets. */
			if (likely(!EDMA_RXDESC_MORE_BIT_GET(rxdesc_pri))) {
				next_skb =
					(struct sk_buff *)EDMA_RXDESC_OPAQUE_GET(next_rxdesc_pri);

				if (unlikely(!
					     edma_rx_handle_linear_packets(rxdesc_ring,
									   rxdesc_pri, skb)))
					dev_kfree_skb_any(skb);

				goto next_rx_desc;
			}
		}

		next_skb = (struct sk_buff *)EDMA_RXDESC_OPAQUE_GET(next_rxdesc_pri);

		/* Handle scatter frame processing for first/middle/last segments. */
		page_mode ? edma_rx_handle_nr_frags(rxdesc_ring, rxdesc_pri, skb) :
			edma_rx_handle_frag_list(rxdesc_ring, rxdesc_pri, skb);

next_rx_desc:
		/* Update work done. */
		work_done++;

		/* Check if we can refill EDMA_RX_MAX_PROCESS worth buffers,
		 * if yes, refill and update index before continuing.
		 */
		if (unlikely(!(work_done & (EDMA_RX_MAX_PROCESS - 1)))) {
			reg = EDMA_BASE_OFFSET + EDMA_REG_RXDESC_CONS_IDX(rxdesc_ring->ring_id);
			regmap_write(regmap, reg, cons_idx);
			rxdesc_ring->cons_idx = cons_idx;
			edma_rx_alloc_buffer_list(rxdesc_ring->rxfill, EDMA_RX_MAX_PROCESS);
		}
	}

	/* Check if we need to refill and update
	 * index for any buffers before exit.
	 */
	if (unlikely(work_leftover)) {
		reg = EDMA_BASE_OFFSET + EDMA_REG_RXDESC_CONS_IDX(rxdesc_ring->ring_id);
		regmap_write(regmap, reg, cons_idx);
		rxdesc_ring->cons_idx = cons_idx;
		edma_rx_alloc_buffer_list(rxdesc_ring->rxfill, work_leftover);
	}

	return work_done;
}

/**
 * edma_rx_napi_poll - EDMA Rx napi poll.
 * @napi: NAPI structure
 * @budget: Rx NAPI budget
 *
 * EDMA RX NAPI handler to handle the NAPI poll.
 *
 * Return the number of packets processed.
 */
int edma_rx_napi_poll(struct napi_struct *napi, int budget)
{
	struct edma_rxdesc_ring *rxdesc_ring = (struct edma_rxdesc_ring *)napi;
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct regmap *regmap = ppe_dev->regmap;
	int work_done = 0;
	u32 status, reg;

	do {
		work_done += edma_rx_reap(rxdesc_ring, budget - work_done);
		if (likely(work_done >= budget))
			return work_done;

		/* Check if there are more packets to process. */
		reg = EDMA_BASE_OFFSET + EDMA_REG_RXDESC_INT_STAT(rxdesc_ring->ring_id);
		regmap_read(regmap, reg, &status);
		status = status & EDMA_RXDESC_RING_INT_STATUS_MASK;
	} while (likely(status));

	napi_complete(napi);

	reg = EDMA_BASE_OFFSET + EDMA_REG_RXDESC_INT_MASK(rxdesc_ring->ring_id);
	regmap_write(regmap, reg, edma_ctx->intr_info.intr_mask_rx);

	return work_done;
}

/**
 * edma_rx_handle_irq - EDMA Rx handle irq.
 * @irq: Interrupt to handle
 * @ctx: Context
 *
 * Process RX IRQ and schedule NAPI.
 *
 * Return IRQ_HANDLED(1) on success.
 */
irqreturn_t edma_rx_handle_irq(int irq, void *ctx)
{
	struct edma_rxdesc_ring *rxdesc_ring = (struct edma_rxdesc_ring *)ctx;
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct regmap *regmap = ppe_dev->regmap;
	u32 reg;

	if (likely(napi_schedule_prep(&rxdesc_ring->napi))) {
		/* Disable RxDesc interrupt. */
		reg = EDMA_BASE_OFFSET + EDMA_REG_RXDESC_INT_MASK(rxdesc_ring->ring_id);
		regmap_write(regmap, reg, EDMA_MASK_INT_DISABLE);
		__napi_schedule(&rxdesc_ring->napi);
	}

	return IRQ_HANDLED;
}
