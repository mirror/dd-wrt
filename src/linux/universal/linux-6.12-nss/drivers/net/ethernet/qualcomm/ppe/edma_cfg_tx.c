// SPDX-License-Identifier: GPL-2.0-only
/* Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 */

/* Configure rings, Buffers and NAPI for transmit path along with
 * providing APIs to enable, disable, clean and map the Tx rings.
 */

#include <linux/dma-mapping.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/printk.h>
#include <linux/regmap.h>
#include <linux/skbuff.h>

#include "edma.h"
#include "edma_cfg_tx.h"
#include "edma_port.h"
#include "ppe.h"
#include "ppe_regs.h"

static void edma_cfg_txcmpl_ring_cleanup(struct edma_txcmpl_ring *txcmpl_ring)
{
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct device *dev =  ppe_dev->dev;

	/* Free any buffers assigned to any descriptors. */
	edma_tx_complete(EDMA_TX_RING_SIZE - 1, txcmpl_ring);

	/* Free TxCmpl ring descriptors. */
	dma_free_coherent(dev, sizeof(struct edma_txcmpl_desc)
			  * txcmpl_ring->count, txcmpl_ring->desc,
			  txcmpl_ring->dma);
	txcmpl_ring->desc = NULL;
	txcmpl_ring->dma = (dma_addr_t)0;
}

static int edma_cfg_txcmpl_ring_setup(struct edma_txcmpl_ring *txcmpl_ring)
{
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct device *dev =  ppe_dev->dev;

	/* Allocate RxFill ring descriptors. */
	txcmpl_ring->desc = dma_alloc_coherent(dev, sizeof(struct edma_txcmpl_desc)
					       * txcmpl_ring->count,
					       &txcmpl_ring->dma,
					       GFP_KERNEL | __GFP_ZERO);

	if (unlikely(!txcmpl_ring->desc))
		return -ENOMEM;

	return 0;
}

static void edma_cfg_tx_desc_ring_cleanup(struct edma_txdesc_ring *txdesc_ring)
{
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct regmap *regmap = ppe_dev->regmap;
	struct edma_txdesc_pri *txdesc = NULL;
	struct device *dev =  ppe_dev->dev;
	u32 prod_idx, cons_idx, data, reg;
	struct sk_buff *skb = NULL;

	/* Free any buffers assigned to any descriptors. */
	reg = EDMA_BASE_OFFSET + EDMA_REG_TXDESC_PROD_IDX(txdesc_ring->id);
	regmap_read(regmap, reg, &data);
	prod_idx = data & EDMA_TXDESC_PROD_IDX_MASK;

	reg = EDMA_BASE_OFFSET + EDMA_REG_TXDESC_CONS_IDX(txdesc_ring->id);
	regmap_read(regmap, reg, &data);
	cons_idx = data & EDMA_TXDESC_CONS_IDX_MASK;

	/* Walk active list, obtain skb from descriptor and free it. */
	while (cons_idx != prod_idx) {
		txdesc = EDMA_TXDESC_PRI_DESC(txdesc_ring, cons_idx);
		skb = (struct sk_buff *)EDMA_TXDESC_OPAQUE_GET(txdesc);
		dev_kfree_skb_any(skb);

		cons_idx = ((cons_idx + 1) & EDMA_TX_RING_SIZE_MASK);
	}

	/* Free Tx ring descriptors. */
	dma_free_coherent(dev, (sizeof(struct edma_txdesc_pri)
			  * txdesc_ring->count),
			  txdesc_ring->pdesc,
			  txdesc_ring->pdma);
	txdesc_ring->pdesc = NULL;
	txdesc_ring->pdma = (dma_addr_t)0;

	/* Free any buffers assigned to any secondary descriptors. */
	dma_free_coherent(dev, (sizeof(struct edma_txdesc_sec)
			  * txdesc_ring->count),
			  txdesc_ring->sdesc,
			  txdesc_ring->sdma);
	txdesc_ring->sdesc = NULL;
	txdesc_ring->sdma = (dma_addr_t)0;
}

static int edma_cfg_tx_desc_ring_setup(struct edma_txdesc_ring *txdesc_ring)
{
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct device *dev = ppe_dev->dev;

	/* Allocate RxFill ring descriptors. */
	txdesc_ring->pdesc = dma_alloc_coherent(dev, sizeof(struct edma_txdesc_pri)
						* txdesc_ring->count,
						&txdesc_ring->pdma,
						GFP_KERNEL | __GFP_ZERO);

	if (unlikely(!txdesc_ring->pdesc))
		return -ENOMEM;

	txdesc_ring->sdesc = dma_alloc_coherent(dev, sizeof(struct edma_txdesc_sec)
						* txdesc_ring->count,
						&txdesc_ring->sdma,
						GFP_KERNEL | __GFP_ZERO);

	if (unlikely(!txdesc_ring->sdesc)) {
		dma_free_coherent(dev, (sizeof(struct edma_txdesc_pri)
				  * txdesc_ring->count),
				  txdesc_ring->pdesc,
				  txdesc_ring->pdma);
		txdesc_ring->pdesc = NULL;
		txdesc_ring->pdma = (dma_addr_t)0;
		return -ENOMEM;
	}

	return 0;
}

static void edma_cfg_tx_desc_ring_configure(struct edma_txdesc_ring *txdesc_ring)
{
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct regmap *regmap = ppe_dev->regmap;
	u32 data, reg;

	reg = EDMA_BASE_OFFSET + EDMA_REG_TXDESC_BA(txdesc_ring->id);
	regmap_write(regmap, reg, (u32)(txdesc_ring->pdma & EDMA_RING_DMA_MASK));

	reg = EDMA_BASE_OFFSET + EDMA_REG_TXDESC_BA2(txdesc_ring->id);
	regmap_write(regmap, reg, (u32)(txdesc_ring->sdma & EDMA_RING_DMA_MASK));

	reg = EDMA_BASE_OFFSET + EDMA_REG_TXDESC_RING_SIZE(txdesc_ring->id);
	regmap_write(regmap, reg, (u32)(txdesc_ring->count & EDMA_TXDESC_RING_SIZE_MASK));

	reg = EDMA_BASE_OFFSET + EDMA_REG_TXDESC_PROD_IDX(txdesc_ring->id);
	regmap_write(regmap, reg, (u32)EDMA_TX_INITIAL_PROD_IDX);

	data = FIELD_PREP(EDMA_TXDESC_CTRL_FC_GRP_ID_MASK, txdesc_ring->fc_grp_id);

	/* Configure group ID for flow control for this Tx ring. */
	reg = EDMA_BASE_OFFSET + EDMA_REG_TXDESC_CTRL(txdesc_ring->id);
	regmap_write(regmap, reg, data);
}

static void edma_cfg_txcmpl_ring_configure(struct edma_txcmpl_ring *txcmpl_ring)
{
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct regmap *regmap = ppe_dev->regmap;
	u32 data, reg;

	/* Configure TxCmpl ring base address. */
	reg = EDMA_BASE_OFFSET + EDMA_REG_TXCMPL_BA(txcmpl_ring->id);
	regmap_write(regmap, reg, (u32)(txcmpl_ring->dma & EDMA_RING_DMA_MASK));

	reg = EDMA_BASE_OFFSET + EDMA_REG_TXCMPL_RING_SIZE(txcmpl_ring->id);
	regmap_write(regmap, reg, (u32)(txcmpl_ring->count & EDMA_TXDESC_RING_SIZE_MASK));

	/* Set TxCmpl ret mode to opaque. */
	reg = EDMA_BASE_OFFSET + EDMA_REG_TXCMPL_CTRL(txcmpl_ring->id);
	regmap_write(regmap, reg, EDMA_TXCMPL_RETMODE_OPAQUE);

	/* Validate mitigation timer value */
	if (edma_tx_mitigation_timer < EDMA_TX_MITIGATION_TIMER_MIN ||
	    edma_tx_mitigation_timer > EDMA_TX_MITIGATION_TIMER_MAX) {
		pr_err("Invalid Tx mitigation timer configured:%d for ring:%d. Using the default timer value:%d\n",
		       edma_tx_mitigation_timer, txcmpl_ring->id,
		       EDMA_TX_MITIGATION_TIMER_DEF);
		edma_tx_mitigation_timer = EDMA_TX_MITIGATION_TIMER_DEF;
	}

	/* Validate mitigation packet count value */
	if (edma_tx_mitigation_pkt_cnt < EDMA_TX_MITIGATION_PKT_CNT_MIN ||
	    edma_tx_mitigation_pkt_cnt > EDMA_TX_MITIGATION_PKT_CNT_MAX) {
		pr_err("Invalid Tx mitigation packet count configured:%d for ring:%d. Using the default packet counter value:%d\n",
		       edma_tx_mitigation_timer, txcmpl_ring->id,
		       EDMA_TX_MITIGATION_PKT_CNT_DEF);
		edma_tx_mitigation_pkt_cnt = EDMA_TX_MITIGATION_PKT_CNT_DEF;
	}

	/* Configure the Mitigation timer. */
	data = EDMA_MICROSEC_TO_TIMER_UNIT(EDMA_TX_MITIGATION_TIMER_DEF,
					   ppe_dev->clk_rate / MHZ);
	data = ((data & EDMA_TX_MOD_TIMER_INIT_MASK)
		<< EDMA_TX_MOD_TIMER_INIT_SHIFT);
	pr_debug("EDMA Tx mitigation timer value: %d\n", data);
	reg = EDMA_BASE_OFFSET + EDMA_REG_TX_MOD_TIMER(txcmpl_ring->id);
	regmap_write(regmap, reg, data);

	/* Configure the Mitigation packet count. */
	data = (edma_tx_mitigation_pkt_cnt & EDMA_TXCMPL_LOW_THRE_MASK)
		<< EDMA_TXCMPL_LOW_THRE_SHIFT;
	pr_debug("EDMA Tx mitigation packet count value: %d\n", data);
	reg = EDMA_BASE_OFFSET + EDMA_REG_TXCMPL_UGT_THRE(txcmpl_ring->id);
	regmap_write(regmap, reg, data);

	reg = EDMA_BASE_OFFSET + EDMA_REG_TX_INT_CTRL(txcmpl_ring->id);
	regmap_write(regmap, reg, EDMA_TX_NE_INT_EN);
}

/**
 * edma_cfg_tx_fill_per_port_tx_map - Fill Tx ring mapping.
 * @netdev: Netdevice.
 * @port_id: Port ID.
 *
 * Fill per-port Tx ring mapping in net device private area.
 */
void edma_cfg_tx_fill_per_port_tx_map(struct net_device *netdev, u32 port_id)
{
	u32 i;

	/* Ring to core mapping is done in order starting from 0 for port 1. */
	for_each_possible_cpu(i) {
		struct edma_port_priv *port_dev = (struct edma_port_priv *)netdev_priv(netdev);
		struct edma_txdesc_ring *txdesc_ring;
		u32 txdesc_ring_id;

		txdesc_ring_id = ((port_id - 1) * num_possible_cpus()) + i;
		txdesc_ring = &edma_ctx->tx_rings[txdesc_ring_id];
		port_dev->txr_map[i] = txdesc_ring;
	}
}

/**
 * edma_cfg_tx_rings_enable - Enable Tx rings.
 *
 * Enable Tx rings.
 */
void edma_cfg_tx_rings_enable(u32 port_id)
{
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct regmap *regmap = ppe_dev->regmap;
	struct edma_txdesc_ring *txdesc_ring;
	u32 i, ring_idx, reg;

	for_each_possible_cpu(i) {
		ring_idx = ((port_id - 1) * num_possible_cpus()) + i;
		txdesc_ring = &edma_ctx->tx_rings[ring_idx];
		u32 data;

		reg = EDMA_BASE_OFFSET + EDMA_REG_TXDESC_CTRL(txdesc_ring->id);
		regmap_read(regmap, reg, &data);
		data |= FIELD_PREP(EDMA_TXDESC_CTRL_TXEN_MASK, EDMA_TXDESC_TX_ENABLE);

		regmap_write(regmap, reg, data);
	}
}

/**
 * edma_cfg_tx_rings_disable - Disable Tx rings.
 *
 * Disable Tx rings.
 */
void edma_cfg_tx_rings_disable(u32 port_id)
{
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct regmap *regmap = ppe_dev->regmap;
	struct edma_txdesc_ring *txdesc_ring;
	u32 i, ring_idx, reg;

	for_each_possible_cpu(i) {
		ring_idx = ((port_id - 1) * num_possible_cpus()) + i;
		txdesc_ring = &edma_ctx->tx_rings[ring_idx];
		u32 data;

		txdesc_ring = &edma_ctx->tx_rings[i];
		reg = EDMA_BASE_OFFSET + EDMA_REG_TXDESC_CTRL(txdesc_ring->id);
		regmap_read(regmap, reg, &data);
		data &= ~EDMA_TXDESC_TX_ENABLE;
		regmap_write(regmap, reg, data);
	}
}

/**
 * edma_cfg_tx_ring_mappings - Map Tx to Tx complete rings.
 *
 * Map Tx to Tx complete rings.
 */
void edma_cfg_tx_ring_mappings(void)
{
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct edma_hw_info *hw_info = edma_ctx->hw_info;
	struct edma_ring_info *txcmpl = hw_info->txcmpl;
	struct regmap *regmap = ppe_dev->regmap;
	struct edma_ring_info *tx = hw_info->tx;
	u32 desc_index, i, data, reg;

	/* Clear the TXDESC2CMPL_MAP_xx reg before setting up
	 * the mapping. This register holds TXDESC to TXFILL ring
	 * mapping.
	 */
	regmap_write(regmap, EDMA_BASE_OFFSET + EDMA_REG_TXDESC2CMPL_MAP_0_ADDR, 0);
	regmap_write(regmap, EDMA_BASE_OFFSET + EDMA_REG_TXDESC2CMPL_MAP_1_ADDR, 0);
	regmap_write(regmap, EDMA_BASE_OFFSET + EDMA_REG_TXDESC2CMPL_MAP_2_ADDR, 0);
	regmap_write(regmap, EDMA_BASE_OFFSET + EDMA_REG_TXDESC2CMPL_MAP_3_ADDR, 0);
	regmap_write(regmap, EDMA_BASE_OFFSET + EDMA_REG_TXDESC2CMPL_MAP_4_ADDR, 0);
	regmap_write(regmap, EDMA_BASE_OFFSET + EDMA_REG_TXDESC2CMPL_MAP_5_ADDR, 0);
	desc_index = txcmpl->ring_start;

	/* 6 registers to hold the completion mapping for total 32
	 * TX desc rings (0-5, 6-11, 12-17, 18-23, 24-29 and rest).
	 * In each entry 5 bits hold the mapping for a particular TX desc ring.
	 */
	for (i = tx->ring_start; i < tx->ring_start + tx->num_rings; i++) {
		u32 reg, data;

		if (i >= 0 && i <= 5)
			reg = EDMA_BASE_OFFSET + EDMA_REG_TXDESC2CMPL_MAP_0_ADDR;
		else if (i >= 6 && i <= 11)
			reg = EDMA_BASE_OFFSET + EDMA_REG_TXDESC2CMPL_MAP_1_ADDR;
		else if (i >= 12 && i <= 17)
			reg = EDMA_BASE_OFFSET + EDMA_REG_TXDESC2CMPL_MAP_2_ADDR;
		else if (i >= 18 && i <= 23)
			reg = EDMA_BASE_OFFSET + EDMA_REG_TXDESC2CMPL_MAP_3_ADDR;
		else if (i >= 24 && i <= 29)
			reg = EDMA_BASE_OFFSET + EDMA_REG_TXDESC2CMPL_MAP_4_ADDR;
		else
			reg = EDMA_BASE_OFFSET + EDMA_REG_TXDESC2CMPL_MAP_5_ADDR;

		pr_debug("Configure Tx desc:%u to use TxCmpl:%u\n", i, desc_index);

		/* Set the Tx complete descriptor ring number in the mapping register.
		 * E.g. If (txcmpl ring)desc_index = 31, (txdesc ring)i = 28.
		 *	reg = EDMA_REG_TXDESC2CMPL_MAP_4_ADDR
		 *	data |= (desc_index & 0x1F) << ((i % 6) * 5);
		 *	data |= (0x1F << 20); -
		 *	This sets 11111 at 20th bit of register EDMA_REG_TXDESC2CMPL_MAP_4_ADDR.
		 */
		regmap_read(regmap, reg, &data);
		data |= (desc_index & EDMA_TXDESC2CMPL_MAP_TXDESC_MASK) << ((i % 6) * 5);
		regmap_write(regmap, reg, data);

		desc_index++;
		if (desc_index == txcmpl->ring_start + txcmpl->num_rings)
			desc_index = txcmpl->ring_start;
	}

	reg = EDMA_BASE_OFFSET + EDMA_REG_TXDESC2CMPL_MAP_0_ADDR;
	regmap_read(regmap, reg, &data);
	pr_debug("EDMA_REG_TXDESC2CMPL_MAP_0_ADDR: 0x%x\n", data);

	reg = EDMA_BASE_OFFSET + EDMA_REG_TXDESC2CMPL_MAP_1_ADDR;
	regmap_read(regmap, reg, &data);
	pr_debug("EDMA_REG_TXDESC2CMPL_MAP_1_ADDR: 0x%x\n", data);

	reg = EDMA_BASE_OFFSET + EDMA_REG_TXDESC2CMPL_MAP_2_ADDR;
	regmap_read(regmap, reg, &data);
	pr_debug("EDMA_REG_TXDESC2CMPL_MAP_2_ADDR: 0x%x\n", data);

	reg = EDMA_BASE_OFFSET + EDMA_REG_TXDESC2CMPL_MAP_3_ADDR;
	regmap_read(regmap, reg, &data);
	pr_debug("EDMA_REG_TXDESC2CMPL_MAP_3_ADDR: 0x%x\n", data);

	reg = EDMA_BASE_OFFSET + EDMA_REG_TXDESC2CMPL_MAP_4_ADDR;
	regmap_read(regmap, reg, &data);
	pr_debug("EDMA_REG_TXDESC2CMPL_MAP_4_ADDR: 0x%x\n", data);

	reg = EDMA_BASE_OFFSET + EDMA_REG_TXDESC2CMPL_MAP_5_ADDR;
	regmap_read(regmap, reg, &data);
	pr_debug("EDMA_REG_TXDESC2CMPL_MAP_5_ADDR: 0x%x\n", data);
}

static int edma_cfg_tx_rings_setup(void)
{
	struct edma_hw_info *hw_info = edma_ctx->hw_info;
	struct edma_ring_info *txcmpl = hw_info->txcmpl;
	struct edma_ring_info *tx = hw_info->tx;
	u32 i, j = 0;

	/* Set Txdesc flow control group id, same as port number. */
	for (i = 0; i < hw_info->max_ports; i++) {
		for_each_possible_cpu(j) {
			struct edma_txdesc_ring *txdesc_ring = NULL;
			u32 txdesc_idx = (i * num_possible_cpus()) + j;

			txdesc_ring = &edma_ctx->tx_rings[txdesc_idx];
			txdesc_ring->fc_grp_id = i + 1;
		}
	}

	/* Allocate TxDesc ring descriptors. */
	for (i = 0; i < tx->num_rings; i++) {
		struct edma_txdesc_ring *txdesc_ring = NULL;
		int ret;

		txdesc_ring = &edma_ctx->tx_rings[i];
		txdesc_ring->count = EDMA_TX_RING_SIZE;
		txdesc_ring->id = tx->ring_start + i;

		ret = edma_cfg_tx_desc_ring_setup(txdesc_ring);
		if (ret) {
			pr_err("Error in setting up %d txdesc ring. ret: %d",
			       txdesc_ring->id, ret);
			while (i-- >= 0)
				edma_cfg_tx_desc_ring_cleanup(&edma_ctx->tx_rings[i]);

			return -ENOMEM;
		}
	}

	/* Allocate TxCmpl ring descriptors. */
	for (i = 0; i < txcmpl->num_rings; i++) {
		struct edma_txcmpl_ring *txcmpl_ring = NULL;
		int ret;

		txcmpl_ring = &edma_ctx->txcmpl_rings[i];
		txcmpl_ring->count = EDMA_TX_RING_SIZE;
		txcmpl_ring->id = txcmpl->ring_start + i;

		ret = edma_cfg_txcmpl_ring_setup(txcmpl_ring);
		if (ret != 0) {
			pr_err("Error in setting up %d TxCmpl ring. ret: %d",
			       txcmpl_ring->id, ret);
			while (i-- >= 0)
				edma_cfg_txcmpl_ring_cleanup(&edma_ctx->txcmpl_rings[i]);

			goto txcmpl_mem_alloc_fail;
		}
	}

	pr_debug("Tx descriptor count for Tx desc and Tx complete rings: %d\n",
		 EDMA_TX_RING_SIZE);

	return 0;

txcmpl_mem_alloc_fail:
	for (i = 0; i < tx->num_rings; i++)
		edma_cfg_tx_desc_ring_cleanup(&edma_ctx->tx_rings[i]);

	return -ENOMEM;
}

/**
 * edma_cfg_tx_rings_alloc - Allocate EDMA Tx rings.
 *
 * Allocate EDMA Tx rings.
 */
int edma_cfg_tx_rings_alloc(void)
{
	struct edma_hw_info *hw_info = edma_ctx->hw_info;
	struct edma_ring_info *txcmpl = hw_info->txcmpl;
	struct edma_ring_info *tx = hw_info->tx;

	edma_ctx->tx_rings = kzalloc((sizeof(*edma_ctx->tx_rings) * tx->num_rings),
				     GFP_KERNEL);
	if (!edma_ctx->tx_rings)
		return -ENOMEM;

	edma_ctx->txcmpl_rings = kzalloc((sizeof(*edma_ctx->txcmpl_rings) * txcmpl->num_rings),
					 GFP_KERNEL);
	if (!edma_ctx->txcmpl_rings)
		goto txcmpl_ring_alloc_fail;

	pr_debug("Num rings - TxDesc:%u (%u-%u) TxCmpl:%u (%u-%u)\n",
		 tx->num_rings, tx->ring_start,
		(tx->ring_start + tx->num_rings - 1),
		txcmpl->num_rings, txcmpl->ring_start,
		(txcmpl->ring_start + txcmpl->num_rings - 1));

	if (edma_cfg_tx_rings_setup()) {
		pr_err("Error in setting up tx rings\n");
		goto tx_rings_setup_fail;
	}

	return 0;

tx_rings_setup_fail:
	kfree(edma_ctx->txcmpl_rings);
	edma_ctx->txcmpl_rings = NULL;

txcmpl_ring_alloc_fail:
	kfree(edma_ctx->tx_rings);
	edma_ctx->tx_rings = NULL;

	return -ENOMEM;
}

/**
 * edma_cfg_tx_rings_cleanup - Cleanup EDMA Tx rings.
 *
 * Cleanup EDMA Tx rings.
 */
void edma_cfg_tx_rings_cleanup(void)
{
	struct edma_hw_info *hw_info = edma_ctx->hw_info;
	struct edma_ring_info *txcmpl = hw_info->txcmpl;
	struct edma_ring_info *tx = hw_info->tx;
	u32 i;

	/* Free any buffers assigned to any descriptors. */
	for (i = 0; i < tx->num_rings; i++)
		edma_cfg_tx_desc_ring_cleanup(&edma_ctx->tx_rings[i]);

	/* Free Tx completion descriptors. */
	for (i = 0; i < txcmpl->num_rings; i++)
		edma_cfg_txcmpl_ring_cleanup(&edma_ctx->txcmpl_rings[i]);

	kfree(edma_ctx->tx_rings);
	kfree(edma_ctx->txcmpl_rings);
	edma_ctx->tx_rings = NULL;
	edma_ctx->txcmpl_rings = NULL;
}

/**
 * edma_cfg_tx_rings - Configure EDMA Tx rings.
 *
 * Configure EDMA Tx rings.
 */
void edma_cfg_tx_rings(void)
{
	struct edma_hw_info *hw_info = edma_ctx->hw_info;
	struct edma_ring_info *txcmpl = hw_info->txcmpl;
	struct edma_ring_info *tx = hw_info->tx;
	u32 i;

	/* Configure Tx desc ring. */
	for (i = 0; i < tx->num_rings; i++)
		edma_cfg_tx_desc_ring_configure(&edma_ctx->tx_rings[i]);

	/* Configure TxCmpl ring. */
	for (i = 0; i < txcmpl->num_rings; i++)
		edma_cfg_txcmpl_ring_configure(&edma_ctx->txcmpl_rings[i]);
}

/**
 * edma_cfg_tx_disable_interrupts - EDMA disable TX interrupts.
 *
 * Disable TX interrupt masks.
 */
void edma_cfg_tx_disable_interrupts(u32 port_id)
{
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct regmap *regmap = ppe_dev->regmap;
	struct edma_txcmpl_ring *txcmpl_ring;
	u32 i, ring_idx, reg;

	for_each_possible_cpu(i) {
		ring_idx = ((port_id - 1) * num_possible_cpus()) + i;
		txcmpl_ring = &edma_ctx->txcmpl_rings[ring_idx];
		reg = EDMA_BASE_OFFSET + EDMA_REG_TX_INT_MASK(txcmpl_ring->id);
		regmap_write(regmap, reg, EDMA_MASK_INT_CLEAR);
	}
}

/**
 * edma_cfg_tx_enable_interrupts - EDMA enable TX interrupts.
 *
 * Enable TX interrupt masks.
 */
void edma_cfg_tx_enable_interrupts(u32 port_id)
{
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct regmap *regmap = ppe_dev->regmap;
	struct edma_txcmpl_ring *txcmpl_ring;
	u32 i, ring_idx, reg;

	for_each_possible_cpu(i) {
		ring_idx = ((port_id - 1) * num_possible_cpus()) + i;
		txcmpl_ring = &edma_ctx->txcmpl_rings[ring_idx];
		reg = EDMA_BASE_OFFSET + EDMA_REG_TX_INT_MASK(txcmpl_ring->id);
		regmap_write(regmap, reg, edma_ctx->intr_info.intr_mask_txcmpl);
	}
}

/**
 * edma_cfg_tx_napi_enable - EDMA Tx NAPI.
 * @port_id: Port ID.
 *
 * Enable Tx NAPI.
 */
void edma_cfg_tx_napi_enable(u32 port_id)
{
	struct edma_txcmpl_ring *txcmpl_ring;
	u32 i, ring_idx;

	/* Enabling Tx napi for a interface with each queue. */
	for_each_possible_cpu(i) {
		ring_idx = ((port_id - 1) * num_possible_cpus()) + i;
		txcmpl_ring = &edma_ctx->txcmpl_rings[ring_idx];
		if (!txcmpl_ring->napi_added)
			continue;

		napi_enable(&txcmpl_ring->napi);
	}
}

/**
 * edma_cfg_tx_napi_disable - Disable Tx NAPI.
 * @port_id: Port ID.
 *
 * Disable Tx NAPI.
 */
void edma_cfg_tx_napi_disable(u32 port_id)
{
	struct edma_txcmpl_ring *txcmpl_ring;
	u32 i, ring_idx;

	/* Disabling Tx napi for a interface with each queue. */
	for_each_possible_cpu(i) {
		ring_idx = ((port_id - 1) * num_possible_cpus()) + i;
		txcmpl_ring = &edma_ctx->txcmpl_rings[ring_idx];
		if (!txcmpl_ring->napi_added)
			continue;

		napi_disable(&txcmpl_ring->napi);
	}
}

/**
 * edma_cfg_tx_napi_delete - Delete Tx NAPI.
 * @port_id: Port ID.
 *
 * Delete Tx NAPI.
 */
void edma_cfg_tx_napi_delete(u32 port_id)
{
	struct edma_txcmpl_ring *txcmpl_ring;
	u32 i, ring_idx;

	/* Disabling Tx napi for a interface with each queue. */
	for_each_possible_cpu(i) {
		ring_idx = ((port_id - 1) * num_possible_cpus()) + i;
		txcmpl_ring = &edma_ctx->txcmpl_rings[ring_idx];
		if (!txcmpl_ring->napi_added)
			continue;

		netif_napi_del(&txcmpl_ring->napi);
		txcmpl_ring->napi_added = false;
	}
}

/**
 * edma_cfg_tx_napi_add - TX NAPI add.
 * @netdev: Netdevice.
 * @port_id: Port ID.
 *
 * TX NAPI add.
 */
void edma_cfg_tx_napi_add(struct net_device *netdev, u32 port_id)
{
	struct edma_hw_info *hw_info = edma_ctx->hw_info;
	struct edma_txcmpl_ring *txcmpl_ring;
	u32 i, ring_idx;

	if (edma_tx_napi_budget < EDMA_TX_NAPI_WORK_MIN ||
	    edma_tx_napi_budget > EDMA_TX_NAPI_WORK_MAX) {
		pr_err("Incorrect Tx NAPI budget: %d, setting to default: %d",
		       edma_tx_napi_budget, hw_info->napi_budget_tx);
		edma_tx_napi_budget = hw_info->napi_budget_tx;
	}

	/* Adding tx napi for a interface with each queue. */
	for_each_possible_cpu(i) {
		ring_idx = ((port_id - 1) * num_possible_cpus()) + i;
		txcmpl_ring = &edma_ctx->txcmpl_rings[ring_idx];
		netif_napi_add_weight(netdev, &txcmpl_ring->napi,
				      edma_tx_napi_poll, hw_info->napi_budget_tx);
		txcmpl_ring->napi_added = true;
		netdev_dbg(netdev, "Napi added for txcmpl ring: %u\n", txcmpl_ring->id);
	}

	netdev_dbg(netdev, "Tx NAPI budget: %d\n", edma_tx_napi_budget);
}
