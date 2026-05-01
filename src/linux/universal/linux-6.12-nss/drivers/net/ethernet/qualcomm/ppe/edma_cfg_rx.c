// SPDX-License-Identifier: GPL-2.0-only
/* Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 */

/* Configure rings, Buffers and NAPI for receive path along with
 * providing APIs to enable, disable, clean and map the Rx rings.
 */

#include <linux/cpumask.h>
#include <linux/dma-mapping.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/printk.h>
#include <linux/regmap.h>
#include <linux/skbuff.h>

#include "edma.h"
#include "edma_cfg_rx.h"
#include "ppe.h"
#include "ppe_regs.h"

/* EDMA Queue ID to Ring ID Table. */
#define EDMA_QID2RID_TABLE_MEM(q)	(0xb9000 + (0x4 * (q)))

/* Rx ring queue offset. */
#define EDMA_QUEUE_OFFSET(q_id)	((q_id) / EDMA_MAX_PRI_PER_CORE)

/* Rx EDMA maximum queue supported. */
#define EDMA_CPU_PORT_QUEUE_MAX(queue_start)	\
			((queue_start) + (EDMA_MAX_PRI_PER_CORE * num_possible_cpus()) - 1)

/* EDMA Queue ID to Ring ID configuration. */
#define EDMA_QID2RID_NUM_PER_REG	4

int rx_queues[] = {0, 8, 16, 24};

static u32 edma_rx_ring_queue_map[][EDMA_MAX_CORE] = {{ 0, 8, 16, 24 },
						{ 1, 9, 17, 25 },
						{ 2, 10, 18, 26 },
						{ 3, 11, 19, 27 },
						{ 4, 12, 20, 28 },
						{ 5, 13, 21, 29 },
						{ 6, 14, 22, 30 },
						{ 7, 15, 23, 31 }};

u32 edma_cfg_rx_rps_bitmap_cores = EDMA_RX_DEFAULT_BITMAP;

static int edma_cfg_rx_desc_rings_reset_queue_mapping(void)
{
	struct edma_hw_info *hw_info = edma_ctx->hw_info;
	struct edma_ring_info *rx = hw_info->rx;
	u32 i, ret;

	for (i = 0; i < rx->num_rings; i++) {
		struct edma_rxdesc_ring *rxdesc_ring;

		rxdesc_ring = &edma_ctx->rx_rings[i];

		ret = ppe_edma_ring_to_queues_config(edma_ctx->ppe_dev, rxdesc_ring->ring_id,
						     ARRAY_SIZE(rx_queues), rx_queues);
		if (ret) {
			pr_err("Error in unmapping rxdesc ring %d to PPE queue mapping to disable its backpressure configuration\n",
			       i);
			return ret;
		}
	}

	return 0;
}

static int edma_cfg_rx_desc_ring_reset_queue_priority(u32 rxdesc_ring_idx)
{
	u32 i, queue_id, ret;

	for (i = 0; i < EDMA_MAX_PRI_PER_CORE; i++) {
		queue_id = edma_rx_ring_queue_map[i][rxdesc_ring_idx];

		ret = ppe_queue_priority_set(edma_ctx->ppe_dev, queue_id, i);
		if (ret) {
			pr_err("Error in resetting %u queue's priority\n",
			       queue_id);
			return ret;
		}
	}

	return 0;
}

static int edma_cfg_rx_desc_ring_reset_queue_config(void)
{
	struct edma_hw_info *hw_info = edma_ctx->hw_info;
	struct edma_ring_info *rx = hw_info->rx;
	u32 i, ret;

	if (unlikely(rx->num_rings > num_possible_cpus())) {
		pr_err("Invalid count of rxdesc rings: %d\n",
		       rx->num_rings);
		return -EINVAL;
	}

	/* Unmap Rxdesc ring to PPE queue mapping */
	ret = edma_cfg_rx_desc_rings_reset_queue_mapping();
	if (ret)	{
		pr_err("Error in resetting Rx desc ring backpressure config\n");
		return ret;
	}

	/* Reset the priority for PPE queues mapped to Rx rings */
	for (i = 0; i < rx->num_rings; i++) {
		ret =  edma_cfg_rx_desc_ring_reset_queue_priority(i);
		if (ret)	{
			pr_err("Error in resetting ring:%d queue's priority\n",
			       i + rx->ring_start);
			return ret;
		}
	}

	return 0;
}

static int edma_cfg_rx_desc_ring_to_queue_mapping(void)
{
	struct edma_hw_info *hw_info = edma_ctx->hw_info;
	struct edma_ring_info *rx = hw_info->rx;
	u32 i;
	int ret;

	/* Rxdesc ring to PPE queue mapping */
	for (i = 0; i < rx->num_rings; i++) {
		struct edma_rxdesc_ring *rxdesc_ring;

		rxdesc_ring = &edma_ctx->rx_rings[i];

		ret = ppe_edma_ring_to_queues_config(edma_ctx->ppe_dev,
						     rxdesc_ring->ring_id,
						ARRAY_SIZE(rx_queues), rx_queues);
		if (ret) {
			pr_err("Error in configuring Rx ring to PPE queue mapping, ret: %d, id: %d\n",
			       ret, rxdesc_ring->ring_id);
			if (!edma_cfg_rx_desc_rings_reset_queue_mapping())
				pr_err("Error in resetting Rx desc ringbackpressure configurations\n");

			return ret;
		}

		pr_debug("Rx desc ring %d to PPE queue mapping for backpressure:\n",
			 rxdesc_ring->ring_id);
	}

	return 0;
}

static void edma_cfg_rx_desc_ring_configure(struct edma_rxdesc_ring *rxdesc_ring)
{
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct regmap *regmap = ppe_dev->regmap;
	u32 data, reg;

	reg = EDMA_BASE_OFFSET + EDMA_REG_RXDESC_BA(rxdesc_ring->ring_id);
	regmap_write(regmap, reg, (u32)(rxdesc_ring->pdma & EDMA_RXDESC_BA_MASK));

	reg = EDMA_BASE_OFFSET + EDMA_REG_RXDESC_PREHEADER_BA(rxdesc_ring->ring_id);
	regmap_write(regmap, reg, (u32)(rxdesc_ring->sdma & EDMA_RXDESC_PREHEADER_BA_MASK));

	data = rxdesc_ring->count & EDMA_RXDESC_RING_SIZE_MASK;
	data |= (EDMA_RXDESC_PL_DEFAULT_VALUE & EDMA_RXDESC_PL_OFFSET_MASK)
		 << EDMA_RXDESC_PL_OFFSET_SHIFT;
	reg = EDMA_BASE_OFFSET + EDMA_REG_RXDESC_RING_SIZE(rxdesc_ring->ring_id);
	regmap_write(regmap, reg, data);

	/* Validate mitigation timer value */
	if (edma_rx_mitigation_timer < EDMA_RX_MITIGATION_TIMER_MIN ||
	    edma_rx_mitigation_timer > EDMA_RX_MITIGATION_TIMER_MAX) {
		pr_err("Invalid Rx mitigation timer configured:%d for ring:%d. Using the default timer value:%d\n",
		       edma_rx_mitigation_timer, rxdesc_ring->ring_id,
			EDMA_RX_MITIGATION_TIMER_DEF);
		edma_rx_mitigation_timer = EDMA_RX_MITIGATION_TIMER_DEF;
	}

	/* Validate mitigation packet count value */
	if (edma_rx_mitigation_pkt_cnt < EDMA_RX_MITIGATION_PKT_CNT_MIN ||
	    edma_rx_mitigation_pkt_cnt > EDMA_RX_MITIGATION_PKT_CNT_MAX) {
		pr_err("Invalid Rx mitigation packet count configured:%d for ring:%d. Using the default packet counter value:%d\n",
		       edma_rx_mitigation_timer, rxdesc_ring->ring_id,
			EDMA_RX_MITIGATION_PKT_CNT_DEF);
		edma_rx_mitigation_pkt_cnt = EDMA_RX_MITIGATION_PKT_CNT_DEF;
	}

	/* Configure the Mitigation timer */
	data = EDMA_MICROSEC_TO_TIMER_UNIT(EDMA_RX_MITIGATION_TIMER_DEF,
					   ppe_dev->clk_rate / MHZ);
	data = ((data & EDMA_RX_MOD_TIMER_INIT_MASK)
			<< EDMA_RX_MOD_TIMER_INIT_SHIFT);
	pr_debug("EDMA Rx mitigation timer value: %d\n", data);
	reg = EDMA_BASE_OFFSET + EDMA_REG_RX_MOD_TIMER(rxdesc_ring->ring_id);
	regmap_write(regmap, reg, data);

	/* Configure the Mitigation packet count */
	data = (edma_rx_mitigation_pkt_cnt & EDMA_RXDESC_LOW_THRE_MASK)
			<< EDMA_RXDESC_LOW_THRE_SHIFT;
	pr_debug("EDMA Rx mitigation packet count value: %d\n", data);
	reg = EDMA_BASE_OFFSET + EDMA_REG_RXDESC_UGT_THRE(rxdesc_ring->ring_id);
	regmap_write(regmap, reg, data);

	/* Enable ring. Set ret mode to 'opaque'. */
	reg = EDMA_BASE_OFFSET + EDMA_REG_RX_INT_CTRL(rxdesc_ring->ring_id);
	regmap_write(regmap, reg, EDMA_RX_NE_INT_EN);
}

static void edma_cfg_rx_qid_to_rx_desc_ring_mapping(void)
{
	u32 desc_index, ring_index, reg_index, data, q_id;
	struct edma_hw_info *hw_info = edma_ctx->hw_info;
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct regmap *regmap = ppe_dev->regmap;
	struct edma_ring_info *rx = hw_info->rx;
	u32 mcast_start, mcast_end, reg;
	int ret;

	desc_index = (rx->ring_start & EDMA_RX_RING_ID_MASK);

	/* Here map all the queues to ring. */
	for (q_id = EDMA_RX_QUEUE_START;
		q_id <= EDMA_CPU_PORT_QUEUE_MAX(EDMA_RX_QUEUE_START);
			q_id += EDMA_QID2RID_NUM_PER_REG) {
		reg_index = q_id / EDMA_QID2RID_NUM_PER_REG;
		ring_index = desc_index + EDMA_QUEUE_OFFSET(q_id);

		data = FIELD_PREP(EDMA_RX_RING_ID_QUEUE0_MASK, ring_index);
		data |= FIELD_PREP(EDMA_RX_RING_ID_QUEUE1_MASK, ring_index);
		data |=	FIELD_PREP(EDMA_RX_RING_ID_QUEUE2_MASK, ring_index);
		data |=	FIELD_PREP(EDMA_RX_RING_ID_QUEUE3_MASK, ring_index);

		reg = EDMA_BASE_OFFSET + EDMA_QID2RID_TABLE_MEM(reg_index);
		regmap_write(regmap, reg, data);
		pr_debug("Configure QID2RID: %d reg:0x%x to 0x%x, desc_index: %d, reg_index: %d\n",
			 q_id, EDMA_QID2RID_TABLE_MEM(reg_index), data, desc_index, reg_index);
	}

	ret = ppe_edma_queue_resource_get(edma_ctx->ppe_dev, PPE_RES_MCAST,
					  &mcast_start, &mcast_end);
	if (ret < 0) {
		pr_err("Error in extracting multicast queue values\n");
		return;
	}

	/* Map multicast queues to the first Rx ring. */
	desc_index = (rx->ring_start & EDMA_RX_RING_ID_MASK);
	for (q_id = mcast_start; q_id <= mcast_end;
			q_id += EDMA_QID2RID_NUM_PER_REG) {
		reg_index = q_id / EDMA_QID2RID_NUM_PER_REG;

		data = FIELD_PREP(EDMA_RX_RING_ID_QUEUE0_MASK, desc_index);
		data |=	FIELD_PREP(EDMA_RX_RING_ID_QUEUE1_MASK, desc_index);
		data |=	FIELD_PREP(EDMA_RX_RING_ID_QUEUE2_MASK, desc_index);
		data |=	FIELD_PREP(EDMA_RX_RING_ID_QUEUE3_MASK, desc_index);

		reg = EDMA_BASE_OFFSET + EDMA_QID2RID_TABLE_MEM(reg_index);
		regmap_write(regmap, reg, data);

		pr_debug("Configure QID2RID: %d reg:0x%x to 0x%x\n",
			 q_id, EDMA_QID2RID_TABLE_MEM(reg_index), data);
	}
}

static void edma_cfg_rx_rings_to_rx_fill_mapping(void)
{
	struct edma_hw_info *hw_info = edma_ctx->hw_info;
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct regmap *regmap = ppe_dev->regmap;
	struct edma_ring_info *rx = hw_info->rx;
	u32 i, data, reg;

	regmap_write(regmap, EDMA_BASE_OFFSET + EDMA_REG_RXDESC2FILL_MAP_0_ADDR, 0);
	regmap_write(regmap, EDMA_BASE_OFFSET + EDMA_REG_RXDESC2FILL_MAP_1_ADDR, 0);
	regmap_write(regmap, EDMA_BASE_OFFSET + EDMA_REG_RXDESC2FILL_MAP_2_ADDR, 0);

	for (i = 0; i < rx->num_rings; i++) {
		struct edma_rxdesc_ring *rxdesc_ring = &edma_ctx->rx_rings[i];
		u32 data, reg, ring_id;

		ring_id = rxdesc_ring->ring_id;
		if (ring_id >= 0 && ring_id <= 9)
			reg = EDMA_BASE_OFFSET + EDMA_REG_RXDESC2FILL_MAP_0_ADDR;
		else if (ring_id >= 10 && ring_id <= 19)
			reg = EDMA_BASE_OFFSET + EDMA_REG_RXDESC2FILL_MAP_1_ADDR;
		else
			reg = EDMA_BASE_OFFSET + EDMA_REG_RXDESC2FILL_MAP_2_ADDR;

		pr_debug("Configure RXDESC:%u to use RXFILL:%u\n",
			 ring_id,
			 rxdesc_ring->rxfill->ring_id);

		 /* Set the Rx fill ring number in the mapping register. */
		regmap_read(regmap, reg, &data);
		data |= (rxdesc_ring->rxfill->ring_id &
			 EDMA_RXDESC2FILL_MAP_RXDESC_MASK) <<
			 ((ring_id % 10) * 3);
		regmap_write(regmap, reg, data);
	}

	reg = EDMA_BASE_OFFSET + EDMA_REG_RXDESC2FILL_MAP_0_ADDR;
	regmap_read(regmap, reg, &data);
	pr_debug("EDMA_REG_RXDESC2FILL_MAP_0_ADDR: 0x%x\n", data);

	reg = EDMA_BASE_OFFSET + EDMA_REG_RXDESC2FILL_MAP_1_ADDR;
	regmap_read(regmap, reg, &data);
	pr_debug("EDMA_REG_RXDESC2FILL_MAP_1_ADDR: 0x%x\n", data);

	reg = EDMA_BASE_OFFSET + EDMA_REG_RXDESC2FILL_MAP_2_ADDR;
	regmap_read(regmap, reg, &data);
	pr_debug("EDMA_REG_RXDESC2FILL_MAP_2_ADDR: 0x%x\n", data);
}

/**
 * edma_cfg_rx_rings_enable - Enable Rx and Rxfill rings
 *
 * Enable Rx and Rxfill rings.
 */
void edma_cfg_rx_rings_enable(void)
{
	struct edma_hw_info *hw_info = edma_ctx->hw_info;
	struct edma_ring_info *rxfill = hw_info->rxfill;
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct regmap *regmap = ppe_dev->regmap;
	struct edma_ring_info *rx = hw_info->rx;
	u32 i, reg;

	/* Enable Rx rings */
	for (i = rx->ring_start; i < rx->ring_start + rx->num_rings; i++) {
		u32 data;

		reg = EDMA_BASE_OFFSET + EDMA_REG_RXDESC_CTRL(i);
		regmap_read(regmap, reg, &data);
		data |= EDMA_RXDESC_RX_EN;
		regmap_write(regmap, reg, data);
	}

	for (i = rxfill->ring_start; i < rxfill->ring_start + rxfill->num_rings; i++) {
		u32 data;

		reg = EDMA_BASE_OFFSET + EDMA_REG_RXFILL_RING_EN(i);
		regmap_read(regmap, reg, &data);
		data |= EDMA_RXFILL_RING_EN;
		regmap_write(regmap, reg, data);
	}
}

/**
 * edma_cfg_rx_rings_disable - Disable Rx and Rxfill rings
 *
 * Disable Rx and Rxfill rings.
 */
void edma_cfg_rx_rings_disable(void)
{
	struct edma_hw_info *hw_info = edma_ctx->hw_info;
	struct edma_ring_info *rxfill = hw_info->rxfill;
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct regmap *regmap = ppe_dev->regmap;
	struct edma_ring_info *rx = hw_info->rx;
	u32 i, reg;

	/* Disable Rx rings */
	for (i = 0; i < rx->num_rings; i++) {
		struct edma_rxdesc_ring *rxdesc_ring = NULL;
		u32 data;

		rxdesc_ring = &edma_ctx->rx_rings[i];
		reg = EDMA_BASE_OFFSET + EDMA_REG_RXDESC_CTRL(rxdesc_ring->ring_id);
		regmap_read(regmap, reg, &data);
		data &= ~EDMA_RXDESC_RX_EN;
		regmap_write(regmap, reg, data);
	}

	/* Disable RxFill Rings */
	for (i = 0; i < rxfill->num_rings; i++) {
		struct edma_rxfill_ring *rxfill_ring = NULL;
		u32 data;

		rxfill_ring = &edma_ctx->rxfill_rings[i];
		reg = EDMA_BASE_OFFSET + EDMA_REG_RXFILL_RING_EN(rxfill_ring->ring_id);
		regmap_read(regmap, reg, &data);
		data &= ~EDMA_RXFILL_RING_EN;
		regmap_write(regmap, reg, data);
	}
}

/**
 * edma_cfg_rx_mappings - Setup RX ring mapping
 *
 * Setup queue ID to Rx desc ring mapping.
 */
void edma_cfg_rx_ring_mappings(void)
{
	edma_cfg_rx_qid_to_rx_desc_ring_mapping();
	edma_cfg_rx_rings_to_rx_fill_mapping();
}

static void edma_cfg_rx_fill_ring_cleanup(struct edma_rxfill_ring *rxfill_ring)
{
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct regmap *regmap = ppe_dev->regmap;
	struct device *dev = ppe_dev->dev;
	u16 cons_idx, curr_idx;
	u32 data, reg;

	/* Get RxFill ring producer index */
	curr_idx = rxfill_ring->prod_idx & EDMA_RXFILL_PROD_IDX_MASK;

	/* Get RxFill ring consumer index */
	reg = EDMA_BASE_OFFSET + EDMA_REG_RXFILL_CONS_IDX(rxfill_ring->ring_id);
	regmap_read(regmap, reg, &data);
	cons_idx = data & EDMA_RXFILL_CONS_IDX_MASK;

	while (curr_idx != cons_idx) {
		struct edma_rxfill_desc *rxfill_desc;
		struct sk_buff *skb;

		/* Get RxFill descriptor */
		rxfill_desc = EDMA_RXFILL_DESC(rxfill_ring, cons_idx);

		cons_idx = (cons_idx + 1) & EDMA_RX_RING_SIZE_MASK;

		/* Get skb from opaque */
		skb = (struct sk_buff *)EDMA_RXFILL_OPAQUE_GET(rxfill_desc);
		if (unlikely(!skb)) {
			pr_err("Empty skb reference at index:%d\n",
			       cons_idx);
			continue;
		}

		dev_kfree_skb_any(skb);
	}

	/* Free RxFill ring descriptors */
	dma_free_coherent(dev, (sizeof(struct edma_rxfill_desc)
			   * rxfill_ring->count),
			   rxfill_ring->desc, rxfill_ring->dma);
	rxfill_ring->desc = NULL;
	rxfill_ring->dma = (dma_addr_t)0;
}

static int edma_cfg_rx_fill_ring_dma_alloc(struct edma_rxfill_ring *rxfill_ring)
{
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct device *dev = ppe_dev->dev;

	/* Allocate RxFill ring descriptors */
	rxfill_ring->desc = dma_alloc_coherent(dev, (sizeof(struct edma_rxfill_desc)
					       * rxfill_ring->count),
					       &rxfill_ring->dma,
					       GFP_KERNEL | __GFP_ZERO);
	if (unlikely(!rxfill_ring->desc))
		return -ENOMEM;

	return 0;
}

static int edma_cfg_rx_desc_ring_dma_alloc(struct edma_rxdesc_ring *rxdesc_ring)
{
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct device *dev = ppe_dev->dev;

	rxdesc_ring->pdesc = dma_alloc_coherent(dev, (sizeof(struct edma_rxdesc_pri)
						* rxdesc_ring->count),
				&rxdesc_ring->pdma, GFP_KERNEL | __GFP_ZERO);
	if (unlikely(!rxdesc_ring->pdesc))
		return -ENOMEM;

	rxdesc_ring->sdesc = dma_alloc_coherent(dev, (sizeof(struct edma_rxdesc_sec)
				* rxdesc_ring->count),
				&rxdesc_ring->sdma, GFP_KERNEL | __GFP_ZERO);
	if (unlikely(!rxdesc_ring->sdesc)) {
		dma_free_coherent(dev, (sizeof(struct edma_rxdesc_pri)
				  * rxdesc_ring->count),
				  rxdesc_ring->pdesc,
				  rxdesc_ring->pdma);
		rxdesc_ring->pdesc = NULL;
		rxdesc_ring->pdma = (dma_addr_t)0;
		return -ENOMEM;
	}

	return 0;
}

static void edma_cfg_rx_desc_ring_cleanup(struct edma_rxdesc_ring *rxdesc_ring)
{
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct regmap *regmap = ppe_dev->regmap;
	struct device *dev = ppe_dev->dev;
	u32 prod_idx, cons_idx, reg;

	/* Get Rxdesc consumer & producer indices */
	cons_idx = rxdesc_ring->cons_idx & EDMA_RXDESC_CONS_IDX_MASK;

	reg = EDMA_BASE_OFFSET + EDMA_REG_RXDESC_PROD_IDX(rxdesc_ring->ring_id);
	regmap_read(regmap, reg, &prod_idx);
	prod_idx = prod_idx & EDMA_RXDESC_PROD_IDX_MASK;

	/* Free any buffers assigned to any descriptors */
	while (cons_idx != prod_idx) {
		struct edma_rxdesc_pri *rxdesc_pri =
			EDMA_RXDESC_PRI_DESC(rxdesc_ring, cons_idx);
		struct sk_buff *skb;

		/* Update consumer index */
		cons_idx = (cons_idx + 1) & EDMA_RX_RING_SIZE_MASK;

		/* Get opaque from Rxdesc */
		skb = (struct sk_buff *)EDMA_RXDESC_OPAQUE_GET(rxdesc_pri);
		if (unlikely(!skb)) {
			pr_warn("Empty skb reference at index:%d\n",
				cons_idx);
			continue;
		}

		dev_kfree_skb_any(skb);
	}

	/* Update the consumer index */
	reg = EDMA_BASE_OFFSET + EDMA_REG_RXDESC_CONS_IDX(rxdesc_ring->ring_id);
	regmap_write(regmap, reg, cons_idx);

	/* Free Rxdesc ring descriptor */
	dma_free_coherent(dev, (sizeof(struct edma_rxdesc_pri)
			  * rxdesc_ring->count), rxdesc_ring->pdesc,
			  rxdesc_ring->pdma);
	rxdesc_ring->pdesc = NULL;
	rxdesc_ring->pdma = (dma_addr_t)0;

	/* Free any buffers assigned to any secondary ring descriptors */
	dma_free_coherent(dev, (sizeof(struct edma_rxdesc_sec)
			  * rxdesc_ring->count), rxdesc_ring->sdesc,
			  rxdesc_ring->sdma);
	rxdesc_ring->sdesc = NULL;
	rxdesc_ring->sdma = (dma_addr_t)0;
}

static int edma_cfg_rx_rings_setup(void)
{
	struct edma_hw_info *hw_info = edma_ctx->hw_info;
	struct edma_ring_info *rxfill = hw_info->rxfill;
	struct edma_ring_info *rx = hw_info->rx;
	u32 ring_idx, alloc_size, buf_len;

	/* Set buffer allocation size */
	if (edma_ctx->rx_buf_size) {
		alloc_size = edma_ctx->rx_buf_size +
				EDMA_RX_SKB_HEADROOM + NET_IP_ALIGN;
		buf_len = alloc_size - EDMA_RX_SKB_HEADROOM - NET_IP_ALIGN;
	} else if (edma_ctx->rx_page_mode) {
		alloc_size = EDMA_RX_PAGE_MODE_SKB_SIZE +
				EDMA_RX_SKB_HEADROOM + NET_IP_ALIGN;
		buf_len = PAGE_SIZE;
	} else {
		alloc_size = EDMA_RX_BUFFER_SIZE;
		buf_len = alloc_size - EDMA_RX_SKB_HEADROOM - NET_IP_ALIGN;
	}

	pr_debug("EDMA ctx:%p rx_ring alloc_size=%d, buf_len=%d\n",
		 edma_ctx, alloc_size, buf_len);

	/* Allocate Rx fill ring descriptors */
	for (ring_idx = 0; ring_idx < rxfill->num_rings; ring_idx++) {
		u32 ret;
		struct edma_rxfill_ring *rxfill_ring = NULL;

		rxfill_ring = &edma_ctx->rxfill_rings[ring_idx];
		rxfill_ring->count = EDMA_RX_RING_SIZE;
		rxfill_ring->ring_id = rxfill->ring_start + ring_idx;
		rxfill_ring->alloc_size = alloc_size;
		rxfill_ring->buf_len = buf_len;
		rxfill_ring->page_mode = edma_ctx->rx_page_mode;

		ret = edma_cfg_rx_fill_ring_dma_alloc(rxfill_ring);
		if (ret) {
			pr_err("Error in setting up %d rxfill ring. ret: %d",
			       rxfill_ring->ring_id, ret);
			while (--ring_idx >= 0)
				edma_cfg_rx_fill_ring_cleanup(&edma_ctx->rxfill_rings[ring_idx]);

			return -ENOMEM;
		}
	}

	/* Allocate RxDesc ring descriptors */
	for (ring_idx = 0; ring_idx < rx->num_rings; ring_idx++) {
		u32 index, queue_id = EDMA_RX_QUEUE_START;
		struct edma_rxdesc_ring *rxdesc_ring = NULL;
		u32 ret;

		rxdesc_ring = &edma_ctx->rx_rings[ring_idx];
		rxdesc_ring->count = EDMA_RX_RING_SIZE;
		rxdesc_ring->ring_id = rx->ring_start + ring_idx;

		if (queue_id > EDMA_CPU_PORT_QUEUE_MAX(EDMA_RX_QUEUE_START)) {
			pr_err("Invalid queue_id: %d\n", queue_id);
			while (--ring_idx >= 0)
				edma_cfg_rx_desc_ring_cleanup(&edma_ctx->rx_rings[ring_idx]);

			goto rxdesc_mem_alloc_fail;
		}

		/* Create a mapping between RX Desc ring and Rx fill ring.
		 * Number of fill rings are lesser than the descriptor rings
		 * Share the fill rings across descriptor rings.
		 */
		index = rxfill->ring_start +
				(ring_idx % rxfill->num_rings);
		rxdesc_ring->rxfill = &edma_ctx->rxfill_rings[index
						- rxfill->ring_start];

		ret = edma_cfg_rx_desc_ring_dma_alloc(rxdesc_ring);
		if (ret) {
			pr_err("Error in setting up %d rxdesc ring. ret: %d",
			       rxdesc_ring->ring_id, ret);
			while (--ring_idx >= 0)
				edma_cfg_rx_desc_ring_cleanup(&edma_ctx->rx_rings[ring_idx]);

			goto rxdesc_mem_alloc_fail;
		}
	}

	pr_debug("Rx descriptor count for Rx desc and Rx fill rings : %d\n",
		 EDMA_RX_RING_SIZE);

	return 0;

rxdesc_mem_alloc_fail:
	for (ring_idx = 0; ring_idx < rxfill->num_rings; ring_idx++)
		edma_cfg_rx_fill_ring_cleanup(&edma_ctx->rxfill_rings[ring_idx]);

	return -ENOMEM;
}

/**
 * edma_cfg_rx_buff_size_setup - Configure EDMA Rx jumbo buffer
 *
 * Configure EDMA Rx jumbo buffer
 */
void edma_cfg_rx_buff_size_setup(void)
{
	if (edma_ctx->rx_buf_size) {
		edma_ctx->rx_page_mode = false;
		pr_debug("Rx Jumbo mru is enabled: %d\n", edma_ctx->rx_buf_size);
	}
}

/**
 * edma_cfg_rx_rings_alloc - Allocate EDMA Rx rings
 *
 * Allocate EDMA Rx rings.
 *
 * Return 0 on success, negative error code on failure.
 */
int edma_cfg_rx_rings_alloc(void)
{
	struct edma_hw_info *hw_info = edma_ctx->hw_info;
	struct edma_ring_info *rxfill = hw_info->rxfill;
	struct edma_ring_info *rx = hw_info->rx;
	int ret;

	edma_ctx->rxfill_rings = kzalloc((sizeof(*edma_ctx->rxfill_rings) *
					 rxfill->num_rings),
					 GFP_KERNEL);
	if (!edma_ctx->rxfill_rings)
		return -ENOMEM;

	edma_ctx->rx_rings = kzalloc((sizeof(*edma_ctx->rx_rings) *
					 rx->num_rings),
					 GFP_KERNEL);
	if (!edma_ctx->rx_rings)
		goto rxdesc_ring_alloc_fail;

	pr_debug("RxDesc:%u rx (%u-%u) RxFill:%u (%u-%u)\n",
		 rx->num_rings, rx->ring_start,
		 (rx->ring_start + rx->num_rings - 1),
		 rxfill->num_rings, rxfill->ring_start,
		 (rxfill->ring_start + rxfill->num_rings - 1));

	if (edma_cfg_rx_rings_setup()) {
		pr_err("Error in setting up Rx rings\n");
		goto rx_rings_setup_fail;
	}

	/* Reset Rx descriptor ring mapped queue's configurations */
	ret = edma_cfg_rx_desc_ring_reset_queue_config();
	if (ret) {
		pr_err("Error in resetting the Rx descriptor rings configurations\n");
		edma_cfg_rx_rings_cleanup();
		return ret;
	}

	return 0;

rx_rings_setup_fail:
	kfree(edma_ctx->rx_rings);
	edma_ctx->rx_rings = NULL;
rxdesc_ring_alloc_fail:
	kfree(edma_ctx->rxfill_rings);
	edma_ctx->rxfill_rings = NULL;

	return -ENOMEM;
}

/**
 * edma_cfg_rx_rings_cleanup - Cleanup EDMA Rx rings
 *
 * Cleanup EDMA Rx rings
 */
void edma_cfg_rx_rings_cleanup(void)
{
	struct edma_hw_info *hw_info = edma_ctx->hw_info;
	struct edma_ring_info *rxfill = hw_info->rxfill;
	struct edma_ring_info *rx = hw_info->rx;
	u32 i;

	/* Free RxFill ring descriptors */
	for (i = 0; i < rxfill->num_rings; i++)
		edma_cfg_rx_fill_ring_cleanup(&edma_ctx->rxfill_rings[i]);

	/* Free Rx completion ring descriptors */
	for (i = 0; i < rx->num_rings; i++)
		edma_cfg_rx_desc_ring_cleanup(&edma_ctx->rx_rings[i]);

	kfree(edma_ctx->rxfill_rings);
	kfree(edma_ctx->rx_rings);
	edma_ctx->rxfill_rings = NULL;
	edma_ctx->rx_rings = NULL;
}

static void edma_cfg_rx_fill_ring_configure(struct edma_rxfill_ring *rxfill_ring)
{
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct regmap *regmap = ppe_dev->regmap;
	u32 ring_sz, reg;

	reg = EDMA_BASE_OFFSET + EDMA_REG_RXFILL_BA(rxfill_ring->ring_id);
	regmap_write(regmap, reg, (u32)(rxfill_ring->dma & EDMA_RING_DMA_MASK));

	ring_sz = rxfill_ring->count & EDMA_RXFILL_RING_SIZE_MASK;
	reg = EDMA_BASE_OFFSET + EDMA_REG_RXFILL_RING_SIZE(rxfill_ring->ring_id);
	regmap_write(regmap, reg, ring_sz);

	edma_rx_alloc_buffer(rxfill_ring, rxfill_ring->count - 1);
}

static void edma_cfg_rx_desc_ring_flow_control(u32 threshold_xoff, u32 threshold_xon)
{
	struct edma_hw_info *hw_info = edma_ctx->hw_info;
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct regmap *regmap = ppe_dev->regmap;
	struct edma_ring_info *rx = hw_info->rx;
	u32 data, i, reg;

	data = (threshold_xoff & EDMA_RXDESC_FC_XOFF_THRE_MASK) << EDMA_RXDESC_FC_XOFF_THRE_SHIFT;
	data |= ((threshold_xon & EDMA_RXDESC_FC_XON_THRE_MASK) << EDMA_RXDESC_FC_XON_THRE_SHIFT);

	for (i = 0; i < rx->num_rings; i++) {
		struct edma_rxdesc_ring *rxdesc_ring;

		rxdesc_ring = &edma_ctx->rx_rings[i];
		reg = EDMA_BASE_OFFSET + EDMA_REG_RXDESC_FC_THRE(rxdesc_ring->ring_id);
		regmap_write(regmap, reg, data);
	}
}

static void edma_cfg_rx_fill_ring_flow_control(int threshold_xoff, int threshold_xon)
{
	struct edma_hw_info *hw_info = edma_ctx->hw_info;
	struct edma_ring_info *rxfill = hw_info->rxfill;
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct regmap *regmap = ppe_dev->regmap;
	u32 data, i, reg;

	data = (threshold_xoff & EDMA_RXFILL_FC_XOFF_THRE_MASK) << EDMA_RXFILL_FC_XOFF_THRE_SHIFT;
	data |= ((threshold_xon & EDMA_RXFILL_FC_XON_THRE_MASK) << EDMA_RXFILL_FC_XON_THRE_SHIFT);

	for (i = 0; i < rxfill->num_rings; i++) {
		struct edma_rxfill_ring *rxfill_ring;

		rxfill_ring = &edma_ctx->rxfill_rings[i];
		reg = EDMA_BASE_OFFSET + EDMA_REG_RXFILL_FC_THRE(rxfill_ring->ring_id);
		regmap_write(regmap, reg, data);
	}
}

/**
 * edma_cfg_rx_rings - Configure EDMA Rx rings.
 *
 * Configure EDMA Rx rings.
 */
int edma_cfg_rx_rings(void)
{
	struct edma_hw_info *hw_info = edma_ctx->hw_info;
	struct edma_ring_info *rxfill = hw_info->rxfill;
	struct edma_ring_info *rx = hw_info->rx;
	u32 i;

	for (i = 0; i < rxfill->num_rings; i++)
		edma_cfg_rx_fill_ring_configure(&edma_ctx->rxfill_rings[i]);

	for (i = 0; i < rx->num_rings; i++)
		edma_cfg_rx_desc_ring_configure(&edma_ctx->rx_rings[i]);

	/* Configure Rx flow control configurations */
	edma_cfg_rx_desc_ring_flow_control(EDMA_RX_FC_XOFF_DEF, EDMA_RX_FC_XON_DEF);
	edma_cfg_rx_fill_ring_flow_control(EDMA_RX_FC_XOFF_DEF, EDMA_RX_FC_XON_DEF);

	return edma_cfg_rx_desc_ring_to_queue_mapping();
}

/**
 * edma_cfg_rx_disable_interrupts - EDMA disable RX interrupts
 *
 * Disable RX interrupt masks
 */
void edma_cfg_rx_disable_interrupts(void)
{
	struct edma_hw_info *hw_info = edma_ctx->hw_info;
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct regmap *regmap = ppe_dev->regmap;
	struct edma_ring_info *rx = hw_info->rx;
	u32 i, reg;

	for (i = 0; i < rx->num_rings; i++) {
		struct edma_rxdesc_ring *rxdesc_ring =
				&edma_ctx->rx_rings[i];
		reg = EDMA_BASE_OFFSET + EDMA_REG_RXDESC_INT_MASK(rxdesc_ring->ring_id);
		regmap_write(regmap, reg, EDMA_MASK_INT_CLEAR);
	}
}

/**
 * edma_cfg_rx_enable_interrupts - EDMA enable RX interrupts
 *
 * Enable RX interrupt masks
 */
void edma_cfg_rx_enable_interrupts(void)
{
	struct edma_hw_info *hw_info = edma_ctx->hw_info;
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct regmap *regmap = ppe_dev->regmap;
	struct edma_ring_info *rx = hw_info->rx;
	u32 i, reg;

	for (i = 0; i < rx->num_rings; i++) {
		struct edma_rxdesc_ring *rxdesc_ring =
				&edma_ctx->rx_rings[i];
		reg = EDMA_BASE_OFFSET + EDMA_REG_RXDESC_INT_MASK(rxdesc_ring->ring_id);
		regmap_write(regmap, reg, edma_ctx->intr_info.intr_mask_rx);
	}
}

/**
 * edma_cfg_rx_napi_disable - Disable NAPI for Rx
 *
 * Disable NAPI for Rx
 */
void edma_cfg_rx_napi_disable(void)
{
	struct edma_hw_info *hw_info = edma_ctx->hw_info;
	struct edma_ring_info *rx = hw_info->rx;
	u32 i;

	for (i = 0; i < rx->num_rings; i++) {
		struct edma_rxdesc_ring *rxdesc_ring;

		rxdesc_ring = &edma_ctx->rx_rings[i];

		if (!rxdesc_ring->napi_added)
			continue;

		napi_disable(&rxdesc_ring->napi);
	}
}

/**
 * edma_cfg_rx_napi_enable - Enable NAPI for Rx
 *
 * Enable NAPI for Rx
 */
void edma_cfg_rx_napi_enable(void)
{
	struct edma_hw_info *hw_info = edma_ctx->hw_info;
	struct edma_ring_info *rx = hw_info->rx;
	u32 i;

	for (i = 0; i < rx->num_rings; i++) {
		struct edma_rxdesc_ring *rxdesc_ring;

		rxdesc_ring = &edma_ctx->rx_rings[i];

		if (!rxdesc_ring->napi_added)
			continue;

		napi_enable(&rxdesc_ring->napi);
	}
}

/**
 * edma_cfg_rx_napi_delete - Delete Rx NAPI
 *
 * Delete RX NAPI
 */
void edma_cfg_rx_napi_delete(void)
{
	struct edma_hw_info *hw_info = edma_ctx->hw_info;
	struct edma_ring_info *rx = hw_info->rx;
	u32 i;

	for (i = 0; i < rx->num_rings; i++) {
		struct edma_rxdesc_ring *rxdesc_ring;

		rxdesc_ring = &edma_ctx->rx_rings[i];

		if (!rxdesc_ring->napi_added)
			continue;

		netif_napi_del(&rxdesc_ring->napi);
		rxdesc_ring->napi_added = false;
	}
}

/* Add Rx NAPI */
/**
 * edma_cfg_rx_napi_add - Add Rx NAPI
 * @netdev: Netdevice
 *
 * Add RX NAPI
 */
void edma_cfg_rx_napi_add(void)
{
	struct edma_hw_info *hw_info = edma_ctx->hw_info;
	struct edma_ring_info *rx = hw_info->rx;
	u32 i;

	if (edma_rx_napi_budget < EDMA_RX_NAPI_WORK_MIN ||
	    edma_rx_napi_budget > EDMA_RX_NAPI_WORK_MAX) {
		pr_err("Incorrect Rx NAPI budget: %d, setting to default: %d",
		       edma_rx_napi_budget, hw_info->napi_budget_rx);
		edma_rx_napi_budget = hw_info->napi_budget_rx;
	}

	for (i = 0; i < rx->num_rings; i++) {
		struct edma_rxdesc_ring *rxdesc_ring = &edma_ctx->rx_rings[i];

		netif_napi_add_weight(edma_ctx->dummy_dev, &rxdesc_ring->napi,
				      edma_rx_napi_poll, hw_info->napi_budget_rx);
		rxdesc_ring->napi_added = true;
	}

	netdev_dbg(edma_ctx->dummy_dev, "Rx NAPI budget: %d\n", edma_rx_napi_budget);
}

/**
 * edma_cfg_rx_rps_hash_map - Configure rx rps hash map.
 *
 * Initialize and configure RPS hash map for queues
 */
int edma_cfg_rx_rps_hash_map(void)
{
	cpumask_t edma_rps_cpumask = {{EDMA_RX_DEFAULT_BITMAP}};
	int map_len = 0, idx = 0, ret = 0;
	u32 q_off = EDMA_RX_QUEUE_START;
	u32 q_map[EDMA_MAX_CORE] = {0};
	u32 hash, cpu;

	/* Map all possible hash values to queues used by the EDMA Rx
	 * rings based on a bitmask, which represents the cores to be mapped.
	 * These queues are expected to be mapped to different Rx rings
	 * which are assigned to different cores using IRQ affinity configuration.
	 */
	for_each_cpu(cpu, &edma_rps_cpumask) {
		q_map[map_len] = q_off + (cpu * EDMA_MAX_PRI_PER_CORE);
		map_len++;
	}

	for (hash = 0; hash < PPE_QUEUE_HASH_NUM; hash++) {
		ret = ppe_edma_queue_offset_config(edma_ctx->ppe_dev,
						   PPE_QUEUE_CLASS_HASH, hash, q_map[idx]);
		if (ret)
			return ret;

		pr_debug("profile_id: %u, hash: %u, q_off: %u\n",
			 EDMA_CPU_PORT_PROFILE_ID, hash, q_map[idx]);
		idx = (idx + 1) % map_len;
	}

	return 0;
}

/* Configure RPS hash mapping based on bitmap */
int edma_cfg_rx_rps_bitmap(const struct ctl_table *table, int write,
			   void *buffer, size_t *lenp, loff_t *ppos)
{
	int ret;

	ret = proc_dointvec(table, write, buffer, lenp, ppos);

	if (!write)
		return ret;

	if (!edma_cfg_rx_rps_bitmap_cores ||
	    edma_cfg_rx_rps_bitmap_cores > EDMA_RX_DEFAULT_BITMAP) {
		pr_warn("Incorrect CPU bitmap: %x. Setting it to default value: %d",
			edma_cfg_rx_rps_bitmap_cores, EDMA_RX_DEFAULT_BITMAP);
		edma_cfg_rx_rps_bitmap_cores = EDMA_RX_DEFAULT_BITMAP;
	}

	ret = edma_cfg_rx_rps_hash_map();

	pr_info("EDMA RPS bitmap value: %d\n", edma_cfg_rx_rps_bitmap_cores);

	return ret;
}
