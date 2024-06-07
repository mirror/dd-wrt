/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 *
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE
 * USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <linux/debugfs.h>
#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/reset.h>
#include <nss_dp_dev.h>
#include "edma.h"
#include "edma_cfg_tx.h"
#include "edma_regs.h"
#include "edma_debug.h"

#ifdef NSS_DP_MHT_SW_PORT_MAP
/*
 * edma_cfg_tx_set_mht_mdio_slv_pause()
 *	Set the MDIO_SLV pause mapping to VP_PORTS
 */
void edma_cfg_tx_set_mht_mdio_slv_pause(struct edma_gbl_ctx *egc)
{
	uint32_t data_reg_0 = 0;
	uint32_t data_reg_1 = 0;
	uint32_t id1;
	uint32_t id2;

	for (id1 = 0; id1 < EDMA_GLOBAL_MDIO_SLV_PAUSE_ID_REG0_MAX; id1++)
		data_reg_0 |= EDMA_GLOBAL_MDIO_SLV_PAUSE_ID_MAP_VPPORT(
				(id1 & EDMA_GLOBAL_MDIO_SLV_VPPORTS_MAX), id1);

	for (id2 = 0; id2 < EDMA_GLOBAL_MDIO_SLV_PAUSE_ID_REG1_MAX; id2++) {
		data_reg_1 |= EDMA_GLOBAL_MDIO_SLV_PAUSE_ID_MAP_VPPORT(
				(id1 & EDMA_GLOBAL_MDIO_SLV_VPPORTS_MAX), id2);
		id1++;
	}
	edma_reg_write(EDMA_MDIO_SLV_PASUE_MAP_0, data_reg_0);
	edma_reg_write(EDMA_MDIO_SLV_PASUE_MAP_1, data_reg_1);
}

/*
 * edma_cfg_tx_set_max_ports()
 *	Configure EDMA Tx max ports
 */
void edma_cfg_tx_set_max_ports(struct edma_gbl_ctx *egc)
{
	/*
	 * Max ports include all MHT switch ports when module param
	 * nss_dp_mht_multi_txring is enabled.
	 * Max ports is Individual ports + MHT SWT ports
	 *
	 */
	if (dp_global_ctx.is_mht_dev) {
		egc->max_tx_ports = NSS_DP_HAL_MAX_TX_PORTS;
		return;
	}

	/*
	 * Max ports equals Individual ports +
	 *		SWT port(single port represents switch) +
	 * 		VP ports
	 */
	egc->max_tx_ports = NSS_DP_MAX_PORTS;
}
#endif

/*
 * edma_cfg_tx_cmpl_ring_cleanup()
 *	Cleanup resources for one TxCmpl ring
 */
static void edma_cfg_tx_cmpl_ring_cleanup(struct edma_gbl_ctx *egc,
				struct edma_txcmpl_ring *txcmpl_ring)
{
	/*
	 * Free any buffers assigned to any descriptors
	 */
	edma_tx_complete(EDMA_TX_RING_SIZE - 1, txcmpl_ring);

	/*
	 * Free TxCmpl ring descriptors
	 */
	kfree(txcmpl_ring->desc);
	txcmpl_ring->desc = NULL;
	txcmpl_ring->dma = (dma_addr_t)0;
}

/*
 * edma_cfg_tx_cmpl_ring_setup()
 *	Setup resources for one TxCmpl ring
 */
static int edma_cfg_tx_cmpl_ring_setup(struct edma_txcmpl_ring *txcmpl_ring)
{
	txcmpl_ring->desc = kmalloc(roundup((sizeof(struct edma_txcmpl_desc) * txcmpl_ring->count),
						SMP_CACHE_BYTES), GFP_KERNEL | __GFP_ZERO);
	if (!txcmpl_ring->desc) {
		edma_err("Descriptor alloc for TXCMPL ring %u failed\n",
				txcmpl_ring->id);
		return -ENOMEM;
	}

	txcmpl_ring->dma = (dma_addr_t)virt_to_phys(txcmpl_ring->desc);

	return 0;
}

/*
 * edma_cfg_tx_desc_ring_cleanup()
 *	Cleanup resources for one TxDesc ring
 *
 * This API expects ring to be disabled by caller
 */
static void edma_cfg_tx_desc_ring_cleanup(struct edma_gbl_ctx *egc,
				struct edma_txdesc_ring *txdesc_ring)
{
	struct sk_buff *skb = NULL;
	struct edma_pri_txdesc *txdesc = NULL;
	uint32_t prod_idx, cons_idx, data, cons_idx_prev;;

	/*
	 * Free any buffers assigned to any descriptors
	 */
	data = edma_reg_read(EDMA_REG_TXDESC_PROD_IDX(txdesc_ring->id));
	prod_idx = data & EDMA_TXDESC_PROD_IDX_MASK;

	data = edma_reg_read(EDMA_REG_TXDESC_CONS_IDX(txdesc_ring->id));
	cons_idx_prev = cons_idx = data & EDMA_TXDESC_CONS_IDX_MASK;

	/*
	 * Walk active list, obtain skb from descriptor and free it
	 */
	while (cons_idx != prod_idx) {
		txdesc = EDMA_TXDESC_PRI_DESC(txdesc_ring, cons_idx);
		skb = (struct sk_buff *)EDMA_TXDESC_OPAQUE_GET(txdesc);
		dev_kfree_skb_any(skb);

		cons_idx = ((cons_idx + 1) & EDMA_TX_RING_SIZE_MASK);
	}

	edma_reg_write(EDMA_REG_TXDESC_PROD_IDX(txdesc_ring->id), cons_idx_prev);

	/*
	 * Free Tx ring descriptors
	 */
	kfree(txdesc_ring->pdesc);
	txdesc_ring->pdesc = NULL;
	txdesc_ring->pdma = (dma_addr_t)0;

	/*
	 * TODO:
	 * Free any buffers assigned to any secondary descriptors
	 */
	kfree(txdesc_ring->sdesc);
	txdesc_ring->sdesc = NULL;
	txdesc_ring->sdma = (dma_addr_t)0;
}

/*
 * edma_cfg_tx_desc_ring_setup()
 *	Setup resources for one TxDesc ring
 */
static int edma_cfg_tx_desc_ring_setup(struct edma_txdesc_ring *txdesc_ring)
{
	/*
	 * Allocate Tx ring descriptors
	 */
	txdesc_ring->pdesc = kmalloc(roundup((sizeof(struct edma_pri_txdesc) * txdesc_ring->count),
						SMP_CACHE_BYTES), GFP_KERNEL | __GFP_ZERO);
	if (!txdesc_ring->pdesc) {
		edma_err("Descriptor alloc for TXDESC ring %u failed\n",
				txdesc_ring->id);
		return -ENOMEM;
	}

	txdesc_ring->pdma = (dma_addr_t)virt_to_phys(txdesc_ring->pdesc);

	/*
	 * Allocate sencondary Tx ring descriptors
	 */
	txdesc_ring->sdesc = kmalloc(roundup((sizeof(struct edma_sec_txdesc) * txdesc_ring->count),
						SMP_CACHE_BYTES), GFP_KERNEL | __GFP_ZERO);
	if (!txdesc_ring->sdesc) {
		edma_err("Descriptor alloc for secondary TXDESC ring %u failed\n",
				txdesc_ring->id);
		kfree(txdesc_ring->pdesc);
		txdesc_ring->pdesc = NULL;
		txdesc_ring->pdma = (dma_addr_t)0;
		return -ENOMEM;
	}

	txdesc_ring->sdma = (dma_addr_t)virt_to_phys(txdesc_ring->sdesc);

	return 0;
}

/*
 * edma_cfg_tx_desc_ring_configure()
 *	Configure one TxDesc ring in EDMA HW
 */
static void edma_cfg_tx_desc_ring_configure(struct edma_txdesc_ring *txdesc_ring)
{
	/*
	 * Configure TXDESC ring
	 */
	edma_reg_write(EDMA_REG_TXDESC_BA(txdesc_ring->id),
			(uint32_t)(txdesc_ring->pdma &
			EDMA_RING_DMA_MASK));

	edma_reg_write(EDMA_REG_TXDESC_BA2(txdesc_ring->id),
			(uint32_t)(txdesc_ring->sdma &
			EDMA_RING_DMA_MASK));

	edma_reg_write(EDMA_REG_TXDESC_RING_SIZE(txdesc_ring->id),
			(uint32_t)(txdesc_ring->count &
			EDMA_TXDESC_RING_SIZE_MASK));

	edma_reg_write(EDMA_REG_TXDESC_PROD_IDX(txdesc_ring->id),
			(uint32_t)EDMA_TX_INITIAL_PROD_IDX);

	/*
	 * Configure group ID for flow control for this Tx ring
	 */
	edma_reg_write(EDMA_REG_TXDESC_CTRL(txdesc_ring->id),
			EDMA_TXDESC_CTRL_FC_GRP_ID_SET(txdesc_ring->fc_grp_id));
}

/*
 * edma_cfg_tx_cmpl_ring_configure()
 *	Configure one TxCmpl ring in EDMA HW
 */
static void edma_cfg_tx_cmpl_ring_configure(struct edma_txcmpl_ring *txcmpl_ring)
{
	struct edma_gbl_ctx *egc = &edma_gbl_ctx;
	uint32_t data;

	/*
	 * Configure TxCmpl ring base address
	 */
	edma_reg_write(EDMA_REG_TXCMPL_BA(txcmpl_ring->id),
			(uint32_t)(txcmpl_ring->dma & EDMA_RING_DMA_MASK));
	edma_reg_write(EDMA_REG_TXCMPL_RING_SIZE(txcmpl_ring->id),
			(uint32_t)(txcmpl_ring->count
			& EDMA_TXDESC_RING_SIZE_MASK));

	/*
	 * Set TxCmpl ret mode to opaque
	 */
	edma_reg_write(EDMA_REG_TXCMPL_CTRL(txcmpl_ring->id),
			EDMA_TXCMPL_RETMODE_OPAQUE);

	/*
	 * Validate mitigation timer value
	 */
	if ((nss_dp_tx_mitigation_timer < EDMA_TX_MITIGATION_TIMER_MIN) ||
			(nss_dp_tx_mitigation_timer > EDMA_TX_MITIGATION_TIMER_MAX)) {
		edma_err("Invalid Tx mitigation timer configured:%d for ring:%d."
				" Using the default timer value:%d\n",
				nss_dp_tx_mitigation_timer, txcmpl_ring->id,
				NSS_DP_TX_MITIGATION_TIMER_DEF);
		nss_dp_tx_mitigation_timer = NSS_DP_TX_MITIGATION_TIMER_DEF;
	}

	/*
	 * Validate mitigation packet count value
	 */
	if ((nss_dp_tx_mitigation_pkt_cnt < EDMA_TX_MITIGATION_PKT_CNT_MIN) ||
			(nss_dp_tx_mitigation_pkt_cnt > EDMA_TX_MITIGATION_PKT_CNT_MAX)) {
		edma_err("Invalid Tx mitigation packet count configured:%d for ring:%d."
				" Using the default packet counter value:%d\n",
				nss_dp_tx_mitigation_timer, txcmpl_ring->id,
				NSS_DP_TX_MITIGATION_PKT_CNT_DEF);
		nss_dp_tx_mitigation_pkt_cnt = NSS_DP_TX_MITIGATION_PKT_CNT_DEF;
	}

	/*
	 * Configure the Mitigation timer
	 */
	data = MICROSEC_TO_TIMER_UNIT(nss_dp_tx_mitigation_timer, egc->edma_timer_rate);
	data = ((data & EDMA_TX_MOD_TIMER_INIT_MASK)
			<< EDMA_TX_MOD_TIMER_INIT_SHIFT);
	edma_info("EDMA Tx mitigation timer value: %d\n", data);
	edma_reg_write(EDMA_REG_TX_MOD_TIMER(txcmpl_ring->id), data);

	/*
	 * Configure the Mitigation packet count
	 */
	data = (nss_dp_tx_mitigation_pkt_cnt & EDMA_TXCMPL_LOW_THRE_MASK)
			<< EDMA_TXCMPL_LOW_THRE_SHIFT;
	edma_info("EDMA Tx mitigation packet count value: %d\n", data);
	edma_reg_write(EDMA_REG_TXCMPL_UGT_THRE(txcmpl_ring->id), data);

	edma_reg_write(EDMA_REG_TX_INT_CTRL(txcmpl_ring->id), EDMA_TX_NE_INT_EN);
}

/*
 * edma_cfg_tx_cmpl_mapping_fill()
 *	API to fill tx complete ring mapping per core
 */
void edma_cfg_tx_cmpl_mapping_fill(struct edma_gbl_ctx *egc)
{
	uint32_t i, j;

	for (i = 0; i < EDMA_TX_RING_PER_CORE_MAX; i++) {
		for_each_possible_cpu(j) {
			int32_t txdesc_id = egc->tx_map[i][j];
			if (txdesc_id < 0) {
				continue;
			}

			egc->txcmpl_map[i][j] =
					egc->tx_to_txcmpl_map[txdesc_id];
			edma_debug("txcmpl_map[i][j]: %d\n",
					egc->txcmpl_map[i][j]);
		}
	}
}

/*
 * edma_cfg_tx_fill_per_port_tx_map()
 *	API to fill per-port Tx ring mapping in net device private area.
 */
void edma_cfg_tx_fill_per_port_tx_map(struct net_device *netdev, uint32_t macid)
{
	uint32_t i;
#ifdef NSS_DP_MHT_SW_PORT_MAP
	uint32_t sw_port, j;
#endif
	struct nss_dp_dev *dp_dev = (struct nss_dp_dev *)netdev_priv(netdev);
	uint32_t txdesc_start = edma_gbl_ctx.txdesc_ring_start;
	struct edma_txdesc_ring *txdesc_ring;
	uint32_t txdesc_ring_id;

	for_each_possible_cpu(i) {

		txdesc_ring_id = edma_gbl_ctx.tx_map[nss_dp_get_idx_from_macid(macid)][i];
		txdesc_ring = &edma_gbl_ctx.txdesc_rings[txdesc_ring_id - txdesc_start];
		dp_dev->dp_info.txr_map[0][i] = txdesc_ring;
#ifdef NSS_DP_MHT_SW_PORT_MAP
		if (dp_dev->nss_dp_mht_dev)
			dp_dev->dp_info.txr_sw_port_map[0][i] = txdesc_ring;
#endif
	}

#ifdef NSS_DP_MHT_SW_PORT_MAP
	if (!dp_dev->nss_dp_mht_dev)
		return;

	sw_port = 1;
	for (i = NSS_DP_HAL_MAX_PORTS; i < edma_gbl_ctx.max_tx_ports; i++) {
		for_each_possible_cpu(j) {
			txdesc_ring_id = edma_gbl_ctx.tx_map[i][j];
			txdesc_ring = &edma_gbl_ctx.txdesc_rings[txdesc_ring_id - txdesc_start];
			dp_dev->dp_info.txr_sw_port_map[sw_port][j] = txdesc_ring;
		}
		sw_port++;
	}
#endif
}

/*
 * edma_cfg_tx_rings_enable()
 *	API to enable TX rings
 */
void edma_cfg_tx_rings_enable(struct edma_gbl_ctx *egc)
{
	uint32_t i;

	/*
	 * Enable Tx rings
	 */
	for (i = 0; i < egc->num_txdesc_rings; i++) {
		uint32_t data;
		struct edma_txdesc_ring *txdesc_ring = &egc->txdesc_rings[i];

		data = edma_reg_read(EDMA_REG_TXDESC_CTRL(txdesc_ring->id));
		data |= EDMA_TXDESC_CTRL_TXEN_SET(EDMA_TXDESC_TX_ENABLE);
		edma_reg_write(EDMA_REG_TXDESC_CTRL(txdesc_ring->id), data);
	}
}

/*
 * edma_cfg_tx_ring_reset()
 *	API to reset the individual TX ring
 *	NOTE: Caller is expected to ensure that no packets
 *	will be coming on the ring and the producer and consumer
 *	indexes are the same. (Currently used by PPE-DS only)
 */
void edma_cfg_tx_ring_reset(struct edma_txdesc_ring *ring)
{
	uint32_t data = 0;

	/*
	 * Reset the ring - wait untill the reset operation is done.
	 */
	data = edma_reg_read(EDMA_REG_TXDESC_CTRL(ring->id));
	data |= EDMA_TXDESC_TX_RESET;
	edma_reg_write(EDMA_REG_TXDESC_CTRL(ring->id), data);
	do {
		data = edma_reg_read(EDMA_REG_TXDESC_CTRL(ring->id));
		data &= EDMA_TXDESC_TX_RESET;
	} while (data);

	/*
	 * Reset the software producer index.
	 */
	ring->prod_idx = 0;
}

/*
 * edma_cfg_tx_rings_disable()
 *	API to disable TX rings
 */
void edma_cfg_tx_rings_disable(struct edma_gbl_ctx *egc)
{
	uint32_t i;

	/*
	 * Disable Tx rings
	 */
	for (i = 0; i < egc->num_txdesc_rings; i++) {
		struct edma_txdesc_ring *txdesc_ring = NULL;
		uint32_t data;

		txdesc_ring = &egc->txdesc_rings[i];
		data = edma_reg_read(EDMA_REG_TXDESC_CTRL(txdesc_ring->id));
		data &= ~EDMA_TXDESC_TX_ENABLE;
		edma_reg_write(EDMA_REG_TXDESC_CTRL(txdesc_ring->id), data);
	}
}

/*
 * edma_cfg_tx_mapping()
 *	API to map TX to TX complete rings
 */
void edma_cfg_tx_mapping(struct edma_gbl_ctx *egc)
{
	uint32_t desc_index, i;

	/*
	 * Clear the TXDESC2CMPL_MAP_xx reg before setting up
	 * the mapping. This register holds TXDESC to TXFILL ring
	 * mapping.
	 */
	edma_reg_write(EDMA_REG_TXDESC2CMPL_MAP_0, 0);
	edma_reg_write(EDMA_REG_TXDESC2CMPL_MAP_1, 0);
	edma_reg_write(EDMA_REG_TXDESC2CMPL_MAP_2, 0);
	edma_reg_write(EDMA_REG_TXDESC2CMPL_MAP_3, 0);
	edma_reg_write(EDMA_REG_TXDESC2CMPL_MAP_4, 0);
	edma_reg_write(EDMA_REG_TXDESC2CMPL_MAP_5, 0);
	desc_index = egc->txcmpl_ring_start;

	/*
	 * 6 registers to hold the completion mapping for total 32
	 * TX desc rings (0-5, 6-11, 12-17, 18-23, 24-29 and rest).
	 * In each entry 5 bits hold the mapping for a particular TX desc ring.
	 */
	for (i = egc->txdesc_ring_start; i < egc->txdesc_ring_end; i++) {
		uint32_t reg, data;

		if ((i >= 0) && (i <= 5)) {
			reg = EDMA_REG_TXDESC2CMPL_MAP_0;
		} else if ((i >= 6) && (i <= 11)) {
			reg = EDMA_REG_TXDESC2CMPL_MAP_1;
		} else if ((i >= 12) && (i <= 17)) {
			reg = EDMA_REG_TXDESC2CMPL_MAP_2;
		} else if ((i >= 18) && (i <= 23)) {
			reg = EDMA_REG_TXDESC2CMPL_MAP_3;
		} else if ((i >= 24) && (i <= 29)) {
			reg = EDMA_REG_TXDESC2CMPL_MAP_4;
		} else {
			reg = EDMA_REG_TXDESC2CMPL_MAP_5;
		}

		edma_debug("Configure TXDESC:%u to use TXCMPL:%u\n", i, desc_index);

		/*
		 * Set the Tx complete descriptor ring number in the mapping register.
		 * E.g. If (txcmpl ring)desc_index = 31, (txdesc ring)i = 28.
		 * 	reg = EDMA_REG_TXDESC2CMPL_MAP_4
		 * 	data |= (desc_index & 0x1F) << ((i % 6) * 5);
		 * 	data |= (0x1F << 20); -
		 * 	This sets 11111 at 20th bit of register EDMA_REG_TXDESC2CMPL_MAP_4
		 */
		data = edma_reg_read(reg);
		data |= (desc_index & EDMA_TXDESC2CMPL_MAP_TXDESC_MASK) << ((i % 6) * 5);
		edma_reg_write(reg, data);

		egc->tx_to_txcmpl_map[i] = desc_index;

		desc_index++;
		if (desc_index == egc->txcmpl_ring_end) {
			desc_index = egc->txcmpl_ring_start;
		}
	}

	edma_debug("EDMA_REG_TXDESC2CMPL_MAP_0: 0x%x\n", edma_reg_read(EDMA_REG_TXDESC2CMPL_MAP_0));
	edma_debug("EDMA_REG_TXDESC2CMPL_MAP_1: 0x%x\n", edma_reg_read(EDMA_REG_TXDESC2CMPL_MAP_1));
	edma_debug("EDMA_REG_TXDESC2CMPL_MAP_2: 0x%x\n", edma_reg_read(EDMA_REG_TXDESC2CMPL_MAP_2));
	edma_debug("EDMA_REG_TXDESC2CMPL_MAP_3: 0x%x\n", edma_reg_read(EDMA_REG_TXDESC2CMPL_MAP_3));
	edma_debug("EDMA_REG_TXDESC2CMPL_MAP_4: 0x%x\n", edma_reg_read(EDMA_REG_TXDESC2CMPL_MAP_4));
	edma_debug("EDMA_REG_TXDESC2CMPL_MAP_5: 0x%x\n", edma_reg_read(EDMA_REG_TXDESC2CMPL_MAP_5));
}

#if defined(NSS_DP_POINT_OFFLOAD)
/*
 * edma_cfg_tx_point_offload_mapping()
 *	API to setup TX ring mapping
 */
void edma_cfg_tx_point_offload_mapping(struct edma_gbl_ctx *egc)
{
	uint32_t data, reg, ring_id = egc->txdesc_point_offload_ring;

	if ((ring_id >= 0) && (ring_id <= 5)) {
		reg = EDMA_REG_TXDESC2CMPL_MAP_0;
	} else if ((ring_id >= 6) && (ring_id <= 11)) {
		reg = EDMA_REG_TXDESC2CMPL_MAP_1;
	} else if ((ring_id >= 12) && (ring_id <= 17)) {
		reg = EDMA_REG_TXDESC2CMPL_MAP_2;
	} else if ((ring_id >= 18) && (ring_id <= 23)) {
		reg = EDMA_REG_TXDESC2CMPL_MAP_3;
	} else if ((ring_id >= 24) && (ring_id <= 29)) {
		reg = EDMA_REG_TXDESC2CMPL_MAP_4;
	} else {
		reg = EDMA_REG_TXDESC2CMPL_MAP_5;
	}

	edma_debug("Configure point offload TXDESC:%u to use TXCMPL:%u\n", ring_id, egc->txcmpl_point_offload_ring);

	/*
	 * Set the Tx complete descriptor ring number in the mapping register.
	 * E.g. If (txcmpl ring)desc_index = 31, (txdesc ring)i = 28.
	 * 	reg = EDMA_REG_TXDESC2CMPL_MAP_4
	 * 	data |= (desc_index & 0x1F) << ((i % 6) * 5);
	 * 	data |= (0x1F << 20); -
	 * 	This sets 11111 at 20th bit of register EDMA_REG_TXDESC2CMPL_MAP_4
	 */
	data = edma_reg_read(reg);
	data |= (egc->txcmpl_point_offload_ring & EDMA_TXDESC2CMPL_MAP_TXDESC_MASK) << ((ring_id % 6) * 5);
	edma_reg_write(reg, data);

	egc->tx_to_txcmpl_map[ring_id] = egc->txcmpl_point_offload_ring;
}
#endif

/*
 * edma_cfg_tx_rings_setup()
 *	Allocate/setup resources for EDMA rings
 */
static int edma_cfg_tx_rings_setup(struct edma_gbl_ctx *egc)
{
	uint32_t i, j = 0;
#ifdef NSS_DP_MHT_SW_PORT_MAP
	uint32_t edma_max_gmac = egc->max_tx_ports;

	/*
	 * Set Txdesc flow control group id
	 * Note: Only valid for HAL Ports. Not valid for dummy ports
	 * Mapping is done for MAX GMAC ports, VP ports needs to be excluded.
	 */
	if (!dp_global_ctx.is_mht_dev)
		edma_max_gmac = egc->max_tx_ports - NSS_DP_VP_HAL_MAX_PORTS;
#else
	uint32_t edma_max_gmac = EDMA_MAX_GMACS;
#endif
	for (i = 0; i < edma_max_gmac; i++) {
		for_each_possible_cpu(j) {
			struct edma_txdesc_ring *txdesc_ring = NULL;
			uint32_t txdesc_idx = egc->tx_map[i][j]
						- egc->txdesc_ring_start;

			txdesc_ring = &egc->txdesc_rings[txdesc_idx];
			txdesc_ring->fc_grp_id = egc->tx_fc_grp_map[i];
		}
	}

	/*
	 * Allocate TxDesc ring descriptors
	 */
	for (i = 0; i < egc->num_txdesc_rings; i++) {
		struct edma_txdesc_ring *txdesc_ring = NULL;
		int32_t ret;

		txdesc_ring = &egc->txdesc_rings[i];
		txdesc_ring->count = EDMA_TX_RING_SIZE;
		txdesc_ring->id = egc->txdesc_ring_start + i;

		ret = edma_cfg_tx_desc_ring_setup(txdesc_ring);
		if (ret != 0) {
			edma_err("Error in setting up %d txdesc ring. ret: %d",
					 txdesc_ring->id, ret);
			while (i-- >= 0) {
				edma_cfg_tx_desc_ring_cleanup(egc,
					&egc->txdesc_rings[i]);
			}

			return -ENOMEM;
		}
	}

	/*
	 * Allocate TxCmpl ring descriptors
	 */
	for (i = 0; i < egc->num_txcmpl_rings; i++) {
		struct edma_txcmpl_ring *txcmpl_ring = NULL;
		int32_t ret;

		txcmpl_ring = &egc->txcmpl_rings[i];
		txcmpl_ring->count = EDMA_TX_RING_SIZE;
		txcmpl_ring->id = egc->txcmpl_ring_start + i;

		ret = edma_cfg_tx_cmpl_ring_setup(txcmpl_ring);
		if (ret != 0) {
			edma_err("Error in setting up %d txcmpl ring. ret: %d",
					 txcmpl_ring->id, ret);
			while (i-- >= 0) {
				edma_cfg_tx_cmpl_ring_cleanup(egc,
						&egc->txcmpl_rings[i]);
			}

			goto txcmpl_mem_alloc_fail;
		}
	}

	edma_info("Tx descriptor count for Tx desc and Tx complete rings: %d\n", EDMA_TX_RING_SIZE);
	edma_cfg_tx_cmpl_mapping_fill(egc);

	return 0;

txcmpl_mem_alloc_fail:
	for (i = 0; i < egc->num_txdesc_rings; i++)
		edma_cfg_tx_desc_ring_cleanup(egc, &egc->txdesc_rings[i]);

	return -ENOMEM;
}

/*
 * edma_cfg_tx_rings_alloc()
 *	Allocate EDMA Tx rings
 */
int32_t edma_cfg_tx_rings_alloc(struct edma_gbl_ctx *egc)
{
	egc->txdesc_rings = kzalloc((sizeof(struct edma_txdesc_ring) *
				egc->num_txdesc_rings), GFP_KERNEL);
	if (!egc->txdesc_rings) {
		edma_err("Error in allocating txdesc ring\n");
		return -ENOMEM;
	}

	egc->txcmpl_rings = kzalloc((sizeof(struct edma_txcmpl_ring) *
				egc->num_txcmpl_rings), GFP_KERNEL);
	if (!egc->txcmpl_rings) {
		edma_err("Error in allocating txcmpl ring\n");
		goto txcmpl_ring_alloc_fail;
	}

	edma_info("Num rings - TxDesc:%u (%u-%u) TxCmpl:%u (%u-%u)\n",
			egc->num_txdesc_rings, egc->txdesc_ring_start,
			(egc->txdesc_ring_start + egc->num_txdesc_rings - 1),
			egc->num_txcmpl_rings, egc->txcmpl_ring_start,
			(egc->txcmpl_ring_start + egc->num_txcmpl_rings - 1));

	if (edma_cfg_tx_rings_setup(egc)) {
		edma_err("Error in setting up tx rings\n");
		goto tx_rings_setup_fail;
	}

	return 0;

tx_rings_setup_fail:
	kfree(egc->txcmpl_rings);
	egc->txcmpl_rings = NULL;
txcmpl_ring_alloc_fail:
	kfree(egc->txdesc_rings);
	egc->txdesc_rings = NULL;
	return -ENOMEM;
}

/*
 * edma_cfg_tx_rings_cleanup()
 *	Cleanup EDMA rings
 */
void edma_cfg_tx_rings_cleanup(struct edma_gbl_ctx *egc)
{
	uint32_t i;

	/*
	 * Free any buffers assigned to any descriptors
	 */
	for (i = 0; i < egc->num_txdesc_rings; i++) {
		edma_cfg_tx_desc_ring_cleanup(egc, &egc->txdesc_rings[i]);
	}

	/*
	 * Free Tx completion descriptors
	 */
	for (i = 0; i < egc->num_txcmpl_rings; i++) {
		edma_cfg_tx_cmpl_ring_cleanup(egc, &egc->txcmpl_rings[i]);
	}

	kfree(egc->txdesc_rings);
	kfree(egc->txcmpl_rings);
	egc->txdesc_rings = NULL;
	egc->txcmpl_rings = NULL;
}

/*
 * edma_cfg_tx_rings()
 *	Configure EDMA rings
 */
void edma_cfg_tx_rings(struct edma_gbl_ctx *egc)
{
	uint32_t i = 0;

	/*
	 * Configure TXDESC ring
	 */
	for (i = 0; i < egc->num_txdesc_rings; i++) {
		edma_cfg_tx_desc_ring_configure(&egc->txdesc_rings[i]);
	}

	/*
	 * Configure TXCMPL ring
	 */
	for (i = 0; i < egc->num_txcmpl_rings; i++) {
		edma_cfg_tx_cmpl_ring_configure(&egc->txcmpl_rings[i]);
	}
}

/*
 * edma_cfg_tx_napi_enable()
 *	Enable Tx NAPI
 */
void edma_cfg_tx_napi_enable(struct edma_gbl_ctx *egc)
{
	uint32_t i;

	for (i = 0; i < egc->num_txcmpl_rings; i++) {
		struct edma_txcmpl_ring *txcmpl_ring;

		txcmpl_ring = &egc->txcmpl_rings[i];

		if (!txcmpl_ring->napi_added) {
			continue;
		}

		/*
		 * TODO:
		 * Enable NAPI separately for each port at the time of DP open
		 */
		napi_enable(&txcmpl_ring->napi);
	}
}

/*
 * edma_cfg_tx_napi_disable()
 *	Disable Tx NAPI
 */
void edma_cfg_tx_napi_disable(struct edma_gbl_ctx *egc)
{
	uint32_t i;

	for (i = 0; i < egc->num_txcmpl_rings; i++) {
		struct edma_txcmpl_ring *txcmpl_ring;

		txcmpl_ring = &egc->txcmpl_rings[i];

		if (!txcmpl_ring->napi_added) {
			continue;
		}

		napi_disable(&txcmpl_ring->napi);
	}
}

/*
 * edma_cfg_tx_napi_delete()
 *	Delete Tx NAPI
 */
void edma_cfg_tx_napi_delete(struct edma_gbl_ctx *egc)
{
	uint32_t i;

	for (i = 0; i < egc->num_txcmpl_rings; i++) {
		struct edma_txcmpl_ring *txcmpl_ring;

		txcmpl_ring = &egc->txcmpl_rings[i];

		if (!txcmpl_ring->napi_added) {
			continue;
		}

		netif_napi_del(&txcmpl_ring->napi);
		txcmpl_ring->napi_added = false;
	}
}

/*
 * edma_cfg_tx_napi_add()
 *	TX NAPI add API
 */
void edma_cfg_tx_napi_add(struct edma_gbl_ctx *egc, struct net_device *netdev, uint32_t macid)
{
	uint32_t i;
	uint32_t index, ring_idx;
	struct edma_txcmpl_ring *txcmpl_ring;
#ifdef NSS_DP_MHT_SW_PORT_MAP
	struct nss_dp_dev *dp_dev = (struct nss_dp_dev *)netdev_priv(netdev);
#endif

	if ((nss_dp_tx_napi_budget < EDMA_TX_NAPI_WORK_MIN) ||
		(nss_dp_tx_napi_budget > EDMA_TX_NAPI_WORK_MAX)) {
		edma_err("Incorrect Tx NAPI budget: %d, setting to default: %d",
				nss_dp_tx_napi_budget, NSS_DP_HAL_TX_NAPI_BUDGET);
		nss_dp_tx_napi_budget = NSS_DP_HAL_TX_NAPI_BUDGET;
	}

	/*
	 * Adding tx napi for a interface with each queue.
	 */
	index = nss_dp_get_idx_from_macid(macid);
	for_each_possible_cpu(i) {
		ring_idx = egc->txcmpl_map[index][i] - egc->txcmpl_ring_start;
		txcmpl_ring = &egc->txcmpl_rings[ring_idx];
#ifdef NSS_DP_MHT_SW_PORT_MAP
		if (txcmpl_ring->napi_added) {
			continue;
		}
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
		netif_napi_add(netdev, &txcmpl_ring->napi,
				edma_tx_napi_poll, nss_dp_tx_napi_budget);
#else
		netif_threaded_napi_add_weight(netdev, &txcmpl_ring->napi,
				edma_tx_napi_poll, nss_dp_tx_napi_budget);
#endif
		txcmpl_ring->napi_added = true;
		edma_debug("Napi added for txcmpl ring: %u\n", txcmpl_ring->id);
	}

#ifdef NSS_DP_MHT_SW_PORT_MAP
	if (!dp_dev->nss_dp_mht_dev)
		goto done;

	for (index = NSS_DP_MHT_MAP_IDX; index < egc->max_tx_ports; index++) {
		for_each_possible_cpu(i) {
			ring_idx = egc->txcmpl_map[index][i] -
					egc->txcmpl_ring_start;
			txcmpl_ring = &egc->txcmpl_rings[ring_idx];

#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
			netif_napi_add(netdev, &txcmpl_ring->napi,
					edma_tx_napi_poll,
					nss_dp_tx_napi_budget);
#else
			netif_threaded_napi_add_weight(netdev, &txcmpl_ring->napi,
					edma_tx_napi_poll,
					nss_dp_tx_napi_budget);
#endif
			txcmpl_ring->napi_added = true;
			edma_debug("Napi added for txcmpl ring: %u\n",
					txcmpl_ring->id);
		}
	}
done:
#endif
	edma_info("Tx NAPI budget: %d\n", nss_dp_tx_napi_budget);
}
