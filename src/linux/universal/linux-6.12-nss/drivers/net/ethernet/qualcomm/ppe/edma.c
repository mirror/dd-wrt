// SPDX-License-Identifier: GPL-2.0-only
 /* Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
  */

 /* Qualcomm Ethernet DMA driver setup, HW configuration, clocks and
  * interrupt initializations.
  */

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/printk.h>
#include <linux/regmap.h>
#include <linux/reset.h>

#include "edma.h"
#include "edma_cfg_tx.h"
#include "edma_cfg_rx.h"
#include "ppe_regs.h"

#define EDMA_IRQ_NAME_SIZE		32

/* Global EDMA context. */
struct edma_context *edma_ctx;
static char **edma_txcmpl_irq_name;
static char **edma_rxdesc_irq_name;

/* Module params. */
static int page_mode;
module_param(page_mode, int, 0);
MODULE_PARM_DESC(page_mode, "Enable page mode (default:0)");

static int rx_buff_size;
module_param(rx_buff_size, int, 0640);
MODULE_PARM_DESC(rx_buff_size, "Rx Buffer size for Jumbo MRU value (default:0)");

int edma_rx_napi_budget = EDMA_RX_NAPI_WORK_DEF;
module_param(edma_rx_napi_budget, int, 0444);
MODULE_PARM_DESC(edma_rx_napi_budget, "Rx NAPI budget (default:128, min:16, max:512)");

int edma_tx_napi_budget = EDMA_TX_NAPI_WORK_DEF;
module_param(edma_tx_napi_budget, int, 0444);
MODULE_PARM_DESC(edma_tx_napi_budget, "Tx NAPI budget (default:512 for ipq95xx, min:16, max:512)");

int edma_rx_mitigation_pkt_cnt = EDMA_RX_MITIGATION_PKT_CNT_DEF;
module_param(edma_rx_mitigation_pkt_cnt, int, 0444);
MODULE_PARM_DESC(edma_rx_mitigation_pkt_cnt,
		 "Rx mitigation packet count value (default:16, min:0, max: 256)");

s32 edma_rx_mitigation_timer = EDMA_RX_MITIGATION_TIMER_DEF;
module_param(edma_rx_mitigation_timer, int, 0444);
MODULE_PARM_DESC(edma_dp_rx_mitigation_timer,
		 "Rx mitigation timer value in microseconds (default:25, min:0, max: 1000)");

int edma_tx_mitigation_timer = EDMA_TX_MITIGATION_TIMER_DEF;
module_param(edma_tx_mitigation_timer, int, 0444);
MODULE_PARM_DESC(edma_tx_mitigation_timer,
		 "Tx mitigation timer value in microseconds (default:250, min:0, max: 1000)");

int edma_tx_mitigation_pkt_cnt = EDMA_TX_MITIGATION_PKT_CNT_DEF;
module_param(edma_tx_mitigation_pkt_cnt, int, 0444);
MODULE_PARM_DESC(edma_tx_mitigation_pkt_cnt,
		 "Tx mitigation packet count value (default:16, min:0, max: 256)");

static int tx_requeue_stop;
module_param(tx_requeue_stop, int, 0640);
MODULE_PARM_DESC(tx_requeue_stop, "Disable Tx requeue function (default:0)");

/* Priority to multi-queue mapping. */
static u8 edma_pri_map[PPE_QUEUE_INTER_PRI_NUM] = {
	0, 1, 2, 3, 4, 5, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7};

enum edma_clk_id {
	EDMA_CLK,
	EDMA_CFG_CLK,
	EDMA_CLK_MAX
};

static const char * const clock_name[EDMA_CLK_MAX] = {
	[EDMA_CLK] = "edma",
	[EDMA_CFG_CLK] = "edma-cfg",
};

/* Rx Fill ring info for IPQ9574. */
static struct edma_ring_info ipq9574_rxfill_ring_info = {
	.max_rings = 8,
	.ring_start = 4,
	.num_rings = 4,
};

/* Rx ring info for IPQ9574. */
static struct edma_ring_info ipq9574_rx_ring_info = {
	.max_rings = 24,
	.ring_start = 20,
	.num_rings = 4,
};

/* Tx ring info for IPQ9574. */
static struct edma_ring_info ipq9574_tx_ring_info = {
	.max_rings = 32,
	.ring_start = 8,
	.num_rings = 24,
};

/* Tx complete ring info for IPQ9574. */
static struct edma_ring_info ipq9574_txcmpl_ring_info = {
	.max_rings = 32,
	.ring_start = 8,
	.num_rings = 24,
};

/* HW info for IPQ9574. */
static struct edma_hw_info ipq9574_hw_info = {
	.rxfill = &ipq9574_rxfill_ring_info,
	.rx = &ipq9574_rx_ring_info,
	.tx = &ipq9574_tx_ring_info,
	.txcmpl = &ipq9574_txcmpl_ring_info,
	.max_ports = 6,
	.napi_budget_rx = 128,
	.napi_budget_tx = 512,
};

static int edma_clock_set_and_enable(struct device *dev,
				     const char *id, unsigned long rate)
{
	struct device_node *edma_np;
	struct clk *clk = NULL;
	int ret;

	edma_np = of_get_child_by_name(dev->of_node, "edma");

	clk = devm_get_clk_from_child(dev, edma_np, id);
	if (IS_ERR(clk)) {
		dev_err(dev, "clk %s get failed\n", id);
		of_node_put(edma_np);
		return PTR_ERR(clk);
	}

	ret = clk_set_rate(clk, rate);
	if (ret) {
		dev_err(dev, "set %lu rate for %s failed\n", rate, id);
		of_node_put(edma_np);
		return ret;
	}

	ret = clk_prepare_enable(clk);
	if (ret) {
		dev_err(dev, "clk %s enable failed\n", id);
		of_node_put(edma_np);
		return ret;
	}

	of_node_put(edma_np);

	dev_dbg(dev, "set %lu rate for %s\n", rate, id);

	return 0;
}

static int edma_clock_init(void)
{
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct device *dev = ppe_dev->dev;
	unsigned long ppe_rate;
	int ret;

	ppe_rate = ppe_dev->clk_rate;

	ret = edma_clock_set_and_enable(dev, clock_name[EDMA_CLK],
					ppe_rate);
	if (ret)
		return ret;

	ret = edma_clock_set_and_enable(dev, clock_name[EDMA_CFG_CLK],
					ppe_rate);
	if (ret)
		return ret;

	return 0;
}

/**
 * edma_err_stats_alloc - Allocate stats memory
 *
 * Allocate memory for per-CPU error stats.
 */
int edma_err_stats_alloc(void)
{
	u32 i;

	edma_ctx->err_stats = alloc_percpu(*edma_ctx->err_stats);
	if (!edma_ctx->err_stats)
		return -ENOMEM;

	for_each_possible_cpu(i) {
		struct edma_err_stats *stats;

		stats = per_cpu_ptr(edma_ctx->err_stats, i);
		u64_stats_init(&stats->syncp);
	}

	return 0;
}

/**
 * edma_err_stats_free - Free stats memory
 *
 * Free memory of per-CPU error stats.
 */
void edma_err_stats_free(void)
{
	if (edma_ctx->err_stats) {
		free_percpu(edma_ctx->err_stats);
		edma_ctx->err_stats = NULL;
	}
}

/**
 * edma_configure_ucast_prio_map_tbl - Configure unicast priority map table.
 *
 * Map int_priority values to priority class and initialize
 * unicast priority map table for default profile_id.
 */
static int edma_configure_ucast_prio_map_tbl(void)
{
	u8 pri_class, int_pri;
	int ret = 0;

	/* Set the priority class value for every possible priority. */
	for (int_pri = 0; int_pri < PPE_QUEUE_INTER_PRI_NUM; int_pri++) {
		pri_class = edma_pri_map[int_pri];

		/* Priority offset should be less than maximum supported
		 * queue priority.
		 */
		if (pri_class > EDMA_PRI_MAX_PER_CORE - 1) {
			pr_err("Configured incorrect priority offset: %d\n",
			       pri_class);
			return -EINVAL;
		}

		ret = ppe_edma_queue_offset_config(edma_ctx->ppe_dev,
						   PPE_QUEUE_CLASS_PRIORITY, int_pri, pri_class);

		if (ret) {
			pr_err("Failed with error: %d to set queue priority class for int_pri: %d for profile_id: %d\n",
			       ret, int_pri, 0);
			return ret;
		}

		pr_debug("profile_id: %d, int_priority: %d, pri_class: %d\n",
			 0, int_pri, pri_class);
	}

	return ret;
}

static void edma_disable_misc_interrupt(void)
{
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct regmap *regmap = ppe_dev->regmap;
	u32 reg;

	reg = EDMA_BASE_OFFSET + EDMA_REG_MISC_INT_MASK_ADDR;
	regmap_write(regmap, reg, EDMA_MASK_INT_CLEAR);
}

static void edma_enable_misc_interrupt(void)
{
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct regmap *regmap = ppe_dev->regmap;
	u32 reg;

	reg = EDMA_BASE_OFFSET + EDMA_REG_MISC_INT_MASK_ADDR;
	regmap_write(regmap, reg, edma_ctx->intr_info.intr_mask_misc);
}

static irqreturn_t edma_misc_handle_irq(int irq,
					__maybe_unused void *ctx)
{
	struct edma_err_stats *stats = this_cpu_ptr(edma_ctx->err_stats);
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct regmap *regmap = ppe_dev->regmap;
	u32 misc_intr_status, data, reg;

	/* Read Misc intr status */
	reg = EDMA_BASE_OFFSET + EDMA_REG_MISC_INT_STAT_ADDR;
	regmap_read(regmap, reg, &data);
	misc_intr_status = data & edma_ctx->intr_info.intr_mask_misc;

	pr_debug("Received misc irq %d, status: %d\n", irq, misc_intr_status);

	if (FIELD_GET(EDMA_MISC_AXI_RD_ERR_MASK, misc_intr_status)) {
		pr_err("MISC AXI read error received\n");
		u64_stats_update_begin(&stats->syncp);
		++stats->edma_axi_read_err;
		u64_stats_update_end(&stats->syncp);
	}

	if (FIELD_GET(EDMA_MISC_AXI_WR_ERR_MASK, misc_intr_status)) {
		pr_err("MISC AXI write error received\n");
		u64_stats_update_begin(&stats->syncp);
		++stats->edma_axi_write_err;
		u64_stats_update_end(&stats->syncp);
	}

	if (FIELD_GET(EDMA_MISC_RX_DESC_FIFO_FULL_MASK, misc_intr_status)) {
		if (net_ratelimit())
			pr_err("MISC Rx descriptor fifo full error received\n");
		u64_stats_update_begin(&stats->syncp);
		++stats->edma_rxdesc_fifo_full;
		u64_stats_update_end(&stats->syncp);
	}

	if (FIELD_GET(EDMA_MISC_RX_ERR_BUF_SIZE_MASK, misc_intr_status)) {
		if (net_ratelimit())
			pr_err("MISC Rx buffer size error received\n");
		u64_stats_update_begin(&stats->syncp);
		++stats->edma_rx_buf_size_err;
		u64_stats_update_end(&stats->syncp);
	}

	if (FIELD_GET(EDMA_MISC_TX_SRAM_FULL_MASK, misc_intr_status)) {
		if (net_ratelimit())
			pr_err("MISC Tx SRAM full error received\n");
		u64_stats_update_begin(&stats->syncp);
		++stats->edma_tx_sram_full;
		u64_stats_update_end(&stats->syncp);
	}

	if (FIELD_GET(EDMA_MISC_TX_CMPL_BUF_FULL_MASK, misc_intr_status)) {
		if (net_ratelimit())
			pr_err("MISC Tx complete buffer full error received\n");
		u64_stats_update_begin(&stats->syncp);
		++stats->edma_txcmpl_buf_full;
		u64_stats_update_end(&stats->syncp);
	}

	if (FIELD_GET(EDMA_MISC_DATA_LEN_ERR_MASK, misc_intr_status)) {
		if (net_ratelimit())
			pr_err("MISC data length error received\n");
		u64_stats_update_begin(&stats->syncp);
		++stats->edma_tx_data_len_err;
		u64_stats_update_end(&stats->syncp);
	}

	if (FIELD_GET(EDMA_MISC_TX_TIMEOUT_MASK, misc_intr_status)) {
		if (net_ratelimit())
			pr_err("MISC Tx timeout error received\n");
		u64_stats_update_begin(&stats->syncp);
		++stats->edma_tx_timeout;
		u64_stats_update_end(&stats->syncp);
	}

	return IRQ_HANDLED;
}

static int edma_irq_register(void)
{
	struct edma_hw_info *hw_info = edma_ctx->hw_info;
	struct edma_ring_info *txcmpl = hw_info->txcmpl;
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct edma_ring_info *rx = hw_info->rx;
	struct device *dev = ppe_dev->dev;
	int ret;
	u32 i;

	/* Request IRQ for TXCMPL rings. */
	edma_txcmpl_irq_name = kzalloc((sizeof(char *) * txcmpl->num_rings), GFP_KERNEL);
	if (!edma_txcmpl_irq_name)
		return -ENOMEM;

	for (i = 0; i < txcmpl->num_rings; i++) {
		edma_txcmpl_irq_name[i] = kzalloc((sizeof(char *) * EDMA_IRQ_NAME_SIZE),
						  GFP_KERNEL);
		if (!edma_txcmpl_irq_name[i]) {
			ret = -ENOMEM;
			goto txcmpl_ring_irq_name_alloc_fail;
		}

		snprintf(edma_txcmpl_irq_name[i], EDMA_IRQ_NAME_SIZE, "edma_txcmpl_%d",
			 txcmpl->ring_start + i);

		irq_set_status_flags(edma_ctx->intr_info.intr_txcmpl[i], IRQ_DISABLE_UNLAZY);

		ret = request_irq(edma_ctx->intr_info.intr_txcmpl[i],
				  edma_tx_handle_irq, IRQF_SHARED,
				  edma_txcmpl_irq_name[i],
				  (void *)&edma_ctx->txcmpl_rings[i]);
		if (ret) {
			pr_err("TXCMPL ring IRQ:%d request %d failed\n",
			       edma_ctx->intr_info.intr_txcmpl[i], i);
			goto txcmpl_ring_intr_req_fail;
		}

		pr_debug("TXCMPL ring: %d IRQ:%d request success: %s\n",
			 txcmpl->ring_start + i,
			 edma_ctx->intr_info.intr_txcmpl[i],
			 edma_txcmpl_irq_name[i]);
	}

	/* Request IRQ for RXDESC rings. */
	edma_rxdesc_irq_name = kzalloc((sizeof(char *) * rx->num_rings),
				       GFP_KERNEL);
	if (!edma_rxdesc_irq_name) {
		ret = -ENOMEM;
		goto rxdesc_irq_name_alloc_fail;
	}

	for (i = 0; i < rx->num_rings; i++) {
		edma_rxdesc_irq_name[i] = kzalloc((sizeof(char *) * EDMA_IRQ_NAME_SIZE),
						  GFP_KERNEL);
		if (!edma_rxdesc_irq_name[i]) {
			ret = -ENOMEM;
			goto rxdesc_ring_irq_name_alloc_fail;
		}

		snprintf(edma_rxdesc_irq_name[i], 20, "edma_rxdesc_%d",
			 rx->ring_start + i);

		irq_set_status_flags(edma_ctx->intr_info.intr_rx[i], IRQ_DISABLE_UNLAZY);

		ret = request_irq(edma_ctx->intr_info.intr_rx[i],
				  edma_rx_handle_irq, IRQF_SHARED,
				  edma_rxdesc_irq_name[i],
				  (void *)&edma_ctx->rx_rings[i]);
		if (ret) {
			pr_err("RXDESC ring IRQ:%d request failed\n",
			       edma_ctx->intr_info.intr_rx[i]);
			goto rx_desc_ring_intr_req_fail;
		}

		pr_debug("RXDESC ring: %d IRQ:%d request success: %s\n",
			 rx->ring_start + i,
			 edma_ctx->intr_info.intr_rx[i],
			 edma_rxdesc_irq_name[i]);
	}

	/* Request Misc IRQ */
	ret = request_irq(edma_ctx->intr_info.intr_misc, edma_misc_handle_irq,
			  IRQF_SHARED, "edma_misc",
			  (void *)dev);
	if (ret) {
		pr_err("MISC IRQ:%d request failed\n",
		       edma_ctx->intr_info.intr_misc);
		goto misc_intr_req_fail;
	}

	return 0;

misc_intr_req_fail:
	/* Free IRQ for RXDESC rings */
	for (i = 0; i < rx->num_rings; i++) {
		synchronize_irq(edma_ctx->intr_info.intr_rx[i]);
		free_irq(edma_ctx->intr_info.intr_rx[i],
			 (void *)&edma_ctx->rx_rings[i]);
	}
rx_desc_ring_intr_req_fail:
	for (i = 0; i < rx->num_rings; i++)
		kfree(edma_rxdesc_irq_name[i]);
rxdesc_ring_irq_name_alloc_fail:
	kfree(edma_rxdesc_irq_name);
rxdesc_irq_name_alloc_fail:
	for (i = 0; i < txcmpl->num_rings; i++) {
		synchronize_irq(edma_ctx->intr_info.intr_txcmpl[i]);
		free_irq(edma_ctx->intr_info.intr_txcmpl[i],
			 (void *)&edma_ctx->txcmpl_rings[i]);
	}
txcmpl_ring_intr_req_fail:
	for (i = 0; i < txcmpl->num_rings; i++)
		kfree(edma_txcmpl_irq_name[i]);
txcmpl_ring_irq_name_alloc_fail:
	kfree(edma_txcmpl_irq_name);

	return ret;
}

static int edma_irq_init(void)
{
	struct edma_hw_info *hw_info = edma_ctx->hw_info;
	struct edma_ring_info *txcmpl = hw_info->txcmpl;
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct edma_ring_info *rx = hw_info->rx;
	char edma_irq_name[EDMA_IRQ_NAME_SIZE];
	struct device *dev = ppe_dev->dev;
	struct platform_device *pdev;
	struct device_node *edma_np;
	u32 i;

	pdev = to_platform_device(dev);
	edma_np = of_get_child_by_name(dev->of_node, "edma");
	edma_ctx->intr_info.intr_txcmpl = kzalloc((sizeof(*edma_ctx->intr_info.intr_txcmpl) *
						  txcmpl->num_rings), GFP_KERNEL);
	if (!edma_ctx->intr_info.intr_txcmpl) {
		of_node_put(edma_np);
		return -ENOMEM;
	}

	/* Get TXCMPL rings IRQ numbers. */
	for (i = 0; i < txcmpl->num_rings; i++) {
		snprintf(edma_irq_name, sizeof(edma_irq_name), "edma_txcmpl_%d",
			 txcmpl->ring_start + i);
		edma_ctx->intr_info.intr_txcmpl[i] = of_irq_get_byname(edma_np, edma_irq_name);
		if (edma_ctx->intr_info.intr_txcmpl[i] < 0) {
			dev_err(dev, "%s: txcmpl_info.intr[%u] irq get failed\n",
				edma_np->name, i);
			of_node_put(edma_np);
			kfree(edma_ctx->intr_info.intr_txcmpl);
			return edma_ctx->intr_info.intr_txcmpl[i];
		}

		dev_dbg(dev, "%s: intr_info.intr_txcmpl[%u] = %u\n",
			edma_np->name, i, edma_ctx->intr_info.intr_txcmpl[i]);
	}

	edma_ctx->intr_info.intr_rx = kzalloc((sizeof(*edma_ctx->intr_info.intr_rx) *
					      rx->num_rings), GFP_KERNEL);
	if (!edma_ctx->intr_info.intr_rx) {
		of_node_put(edma_np);
		kfree(edma_ctx->intr_info.intr_txcmpl);
		return -ENOMEM;
	}

	/* Get RXDESC rings IRQ numbers. */
	for (i = 0; i < rx->num_rings; i++) {
		snprintf(edma_irq_name, sizeof(edma_irq_name), "edma_rxdesc_%d",
			 rx->ring_start + i);
		edma_ctx->intr_info.intr_rx[i] = of_irq_get_byname(edma_np, edma_irq_name);
		if (edma_ctx->intr_info.intr_rx[i] < 0) {
			dev_err(dev, "%s: rx_queue_map_info.intr[%u] irq get failed\n",
				edma_np->name, i);
			of_node_put(edma_np);
			kfree(edma_ctx->intr_info.intr_rx);
			kfree(edma_ctx->intr_info.intr_txcmpl);
			return edma_ctx->intr_info.intr_rx[i];
		}

		dev_dbg(dev, "%s: intr_info.intr_rx[%u] = %u\n",
			edma_np->name, i, edma_ctx->intr_info.intr_rx[i]);
	}

	/* Get misc IRQ number. */
	edma_ctx->intr_info.intr_misc = of_irq_get_byname(edma_np, "edma_misc");
	if (edma_ctx->intr_info.intr_misc < 0) {
		dev_err(dev, "%s: misc_intr irq get failed\n", edma_np->name);
		of_node_put(edma_np);
		kfree(edma_ctx->intr_info.intr_rx);
		kfree(edma_ctx->intr_info.intr_txcmpl);
		return edma_ctx->intr_info.intr_misc;
	}

	of_node_put(edma_np);

	dev_dbg(dev, "%s: misc IRQ:%u\n", edma_np->name,
		edma_ctx->intr_info.intr_misc);

	return 0;
}

static int edma_alloc_rings(void)
{
	if (edma_cfg_tx_rings_alloc()) {
		pr_err("Error in allocating Tx rings\n");
		return -ENOMEM;
	}

	if (edma_cfg_rx_rings_alloc()) {
		pr_err("Error in allocating Rx rings\n");
		goto rx_rings_alloc_fail;
	}

	return 0;

rx_rings_alloc_fail:
	edma_cfg_tx_rings_cleanup();

	return -ENOMEM;
}

static int edma_hw_reset(void)
{
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct device *dev = ppe_dev->dev;
	struct reset_control *edma_hw_rst;
	struct device_node *edma_np;
	const char *reset_string;
	u32 count, i;
	int ret;

	/* Count and parse reset names from DTSI. */
	edma_np = of_get_child_by_name(dev->of_node, "edma");
	count = of_property_count_strings(edma_np, "reset-names");
	if (count < 0) {
		dev_err(dev, "EDMA reset entry not found\n");
		of_node_put(edma_np);
		return -EINVAL;
	}

	for (i = 0; i < count; i++) {
		ret = of_property_read_string_index(edma_np, "reset-names",
						    i, &reset_string);
		if (ret) {
			dev_err(dev, "Error reading reset-names");
			of_node_put(edma_np);
			return -EINVAL;
		}

		edma_hw_rst = of_reset_control_get_exclusive(edma_np, reset_string);
		if (IS_ERR(edma_hw_rst)) {
			of_node_put(edma_np);
			return PTR_ERR(edma_hw_rst);
		}

		/* 100ms delay is required by hardware to reset EDMA. */
		reset_control_assert(edma_hw_rst);
		fsleep(100);

		reset_control_deassert(edma_hw_rst);
		fsleep(100);

		reset_control_put(edma_hw_rst);
		dev_dbg(dev, "EDMA HW reset, i:%d reset_string:%s\n", i, reset_string);
	}

	of_node_put(edma_np);

	return 0;
}

static int edma_hw_configure(void)
{
	struct edma_hw_info *hw_info = edma_ctx->hw_info;
	struct ppe_device *ppe_dev = edma_ctx->ppe_dev;
	struct regmap *regmap = ppe_dev->regmap;
	u32 data, reg, i;
	int ret;

	reg = EDMA_BASE_OFFSET + EDMA_REG_MAS_CTRL_ADDR;
	ret = regmap_read(regmap, reg, &data);
	if (ret)
		return ret;

	pr_debug("EDMA ver %d hw init\n", data);

	/* Setup private data structure. */
	edma_ctx->intr_info.intr_mask_rx = EDMA_RXDESC_INT_MASK_PKT_INT;
	edma_ctx->intr_info.intr_mask_txcmpl = EDMA_TX_INT_MASK_PKT_INT;

	/* Reset EDMA. */
	ret = edma_hw_reset();
	if (ret) {
		pr_err("Error in resetting the hardware. ret: %d\n", ret);
		return ret;
	}

	/* Allocate memory for netdevices. */
	edma_ctx->netdev_arr = kzalloc((sizeof(**edma_ctx->netdev_arr) *
					 hw_info->max_ports),
					 GFP_KERNEL);
	if (!edma_ctx->netdev_arr)
		return -ENOMEM;

	edma_ctx->dummy_dev = alloc_netdev_dummy(0);
	if (!edma_ctx->dummy_dev) {
		ret = -ENOMEM;
		pr_err("Failed to allocate dummy device. ret: %d\n", ret);
		goto dummy_dev_alloc_failed;
	}

	/* Set EDMA jumbo MRU if enabled or set page mode. */
	if (edma_ctx->rx_buf_size) {
		edma_ctx->rx_page_mode = false;
		pr_debug("Rx Jumbo mru is enabled: %d\n", edma_ctx->rx_buf_size);
	} else {
		edma_ctx->rx_page_mode = page_mode;
	}

	ret = edma_alloc_rings();
	if (ret) {
		pr_err("Error in initializaing the rings. ret: %d\n", ret);
		goto edma_alloc_rings_failed;
	}

	/* Disable interrupts. */
	for (i = 1; i <= hw_info->max_ports; i++)
		edma_cfg_tx_disable_interrupts(i);

	edma_cfg_rx_disable_interrupts();
	edma_disable_misc_interrupt();

	edma_cfg_rx_rings_disable();

	edma_cfg_rx_ring_mappings();
	edma_cfg_tx_ring_mappings();

	edma_cfg_tx_rings();

	ret = edma_cfg_rx_rings();
	if (ret) {
		pr_err("Error in configuring Rx rings. ret: %d\n", ret);
		goto edma_cfg_rx_rings_failed;
	}

	/* Configure DMA request priority, DMA read burst length,
	 * and AXI write size.
	 */
	data = FIELD_PREP(EDMA_DMAR_BURST_LEN_MASK, EDMA_BURST_LEN_ENABLE);
	data |= FIELD_PREP(EDMA_DMAR_REQ_PRI_MASK, 0);
	data |= FIELD_PREP(EDMA_DMAR_TXDATA_OUTSTANDING_NUM_MASK, 31);
	data |= FIELD_PREP(EDMA_DMAR_TXDESC_OUTSTANDING_NUM_MASK, 7);
	data |= FIELD_PREP(EDMA_DMAR_RXFILL_OUTSTANDING_NUM_MASK, 7);

	reg = EDMA_BASE_OFFSET + EDMA_REG_DMAR_CTRL_ADDR;
	ret = regmap_write(regmap, reg, data);
	if (ret)
		return ret;

	/* Configure Tx Timeout Threshold. */
	data = EDMA_TX_TIMEOUT_THRESH_VAL;

	reg = EDMA_BASE_OFFSET + EDMA_REG_TX_TIMEOUT_THRESH_ADDR;
	ret = regmap_write(regmap, reg, data);
	if (ret)
		return ret;

	/* Set Miscellaneous error mask. */
	data = EDMA_MISC_AXI_RD_ERR_MASK |
		EDMA_MISC_AXI_WR_ERR_MASK |
		EDMA_MISC_RX_DESC_FIFO_FULL_MASK |
		EDMA_MISC_RX_ERR_BUF_SIZE_MASK |
		EDMA_MISC_TX_SRAM_FULL_MASK |
		EDMA_MISC_TX_CMPL_BUF_FULL_MASK |
		EDMA_MISC_DATA_LEN_ERR_MASK;
	data |= EDMA_MISC_TX_TIMEOUT_MASK;
	edma_ctx->intr_info.intr_mask_misc = data;

	edma_cfg_rx_rings_enable();
	edma_cfg_rx_napi_add();
	edma_cfg_rx_napi_enable();

	/* Global EDMA enable and padding enable. */
	data = EDMA_PORT_PAD_EN | EDMA_PORT_EDMA_EN;

	reg = EDMA_BASE_OFFSET + EDMA_REG_PORT_CTRL_ADDR;
	ret = regmap_write(regmap, reg, data);
	if (ret)
		return ret;

	/* Initialize unicast priority map table. */
	ret = (int)edma_configure_ucast_prio_map_tbl();
	if (ret) {
		pr_err("Failed to initialize unicast priority map table: %d\n",
		       ret);
		goto configure_ucast_prio_map_tbl_failed;
	}

	/* Initialize RPS hash map table. */
	ret = edma_cfg_rx_rps_hash_map();
	if (ret) {
		pr_err("Failed to configure rps hash table: %d\n",
		       ret);
		goto edma_cfg_rx_rps_hash_map_failed;
	}

	return 0;

edma_cfg_rx_rps_hash_map_failed:
configure_ucast_prio_map_tbl_failed:
	edma_cfg_rx_napi_disable();
	edma_cfg_rx_napi_delete();
	edma_cfg_rx_rings_disable();
edma_cfg_rx_rings_failed:
	edma_cfg_tx_rings_cleanup();
	edma_cfg_rx_rings_cleanup();
edma_alloc_rings_failed:
	free_netdev(edma_ctx->dummy_dev);
dummy_dev_alloc_failed:
	kfree(edma_ctx->netdev_arr);

	return ret;
}

/**
 * edma_destroy - EDMA Destroy.
 * @ppe_dev: PPE device
 *
 * Free the memory allocated during setup.
 */
void edma_destroy(struct ppe_device *ppe_dev)
{
	struct edma_hw_info *hw_info = edma_ctx->hw_info;
	struct edma_ring_info *txcmpl = hw_info->txcmpl;
	struct edma_ring_info *rx = hw_info->rx;
	u32 i;

	if (edma_ctx->rx_rps_ctl_table_hdr) {
		unregister_sysctl_table(edma_ctx->rx_rps_ctl_table_hdr);
		edma_ctx->rx_rps_ctl_table_hdr = NULL;
	}

	/* Disable interrupts. */
	for (i = 1; i <= hw_info->max_ports; i++)
		edma_cfg_tx_disable_interrupts(i);

	edma_cfg_rx_disable_interrupts();
	edma_disable_misc_interrupt();

	/* Free IRQ for TXCMPL rings. */
	for (i = 0; i < txcmpl->num_rings; i++) {
		synchronize_irq(edma_ctx->intr_info.intr_txcmpl[i]);

		free_irq(edma_ctx->intr_info.intr_txcmpl[i],
			 (void *)&edma_ctx->txcmpl_rings[i]);
		kfree(edma_txcmpl_irq_name[i]);
	}
	kfree(edma_txcmpl_irq_name);

	/* Free IRQ for RXDESC rings */
	for (i = 0; i < rx->num_rings; i++) {
		synchronize_irq(edma_ctx->intr_info.intr_rx[i]);
		free_irq(edma_ctx->intr_info.intr_rx[i],
			 (void *)&edma_ctx->rx_rings[i]);
		kfree(edma_rxdesc_irq_name[i]);
	}
	kfree(edma_rxdesc_irq_name);

	/* Free Misc IRQ */
	synchronize_irq(edma_ctx->intr_info.intr_misc);
	free_irq(edma_ctx->intr_info.intr_misc, (void *)(ppe_dev->dev));

	kfree(edma_ctx->intr_info.intr_rx);
	kfree(edma_ctx->intr_info.intr_txcmpl);

	edma_cfg_rx_napi_disable();
	edma_cfg_rx_napi_delete();
	edma_cfg_rx_rings_disable();
	edma_cfg_rx_rings_cleanup();
	edma_cfg_tx_rings_cleanup();

	free_netdev(edma_ctx->dummy_dev);
	kfree(edma_ctx->netdev_arr);
}

/* EDMA Rx RPS core sysctl table */
static struct ctl_table edma_rx_rps_core_table[] = {
	{
		.procname	=	"rps_bitmap_cores",
		.data		=	&edma_cfg_rx_rps_bitmap_cores,
		.maxlen		=	sizeof(int),
		.mode		=	0644,
		.proc_handler	=	edma_cfg_rx_rps_bitmap
	},
};

/**
 * edma_setup - EDMA Setup.
 * @ppe_dev: PPE device
 *
 * Configure Ethernet global ctx, clocks, hardware and interrupts.
 *
 * Return 0 on success, negative error code on failure.
 */
int edma_setup(struct ppe_device *ppe_dev)
{
	struct device *dev = ppe_dev->dev;
	int ret;

	edma_ctx = devm_kzalloc(dev, sizeof(*edma_ctx), GFP_KERNEL);
	if (!edma_ctx)
		return -ENOMEM;

	edma_ctx->hw_info = &ipq9574_hw_info;
	edma_ctx->ppe_dev = ppe_dev;
	edma_ctx->rx_buf_size = rx_buff_size;

	edma_ctx->tx_requeue_stop = false;
	if (tx_requeue_stop != 0)
		edma_ctx->tx_requeue_stop = true;

	edma_ctx->rx_rps_ctl_table_hdr = register_sysctl("net/edma",
							 edma_rx_rps_core_table);
	if (!edma_ctx->rx_rps_ctl_table_hdr) {
		pr_err("Rx rps sysctl table configuration failed\n");
		return -EINVAL;
	}

	/* Configure the EDMA common clocks. */
	ret = edma_clock_init();
	if (ret) {
		dev_err(dev, "Error in configuring the EDMA clocks\n");
		return ret;
	}

	dev_dbg(dev, "QCOM EDMA common clocks are configured\n");

	ret = edma_hw_configure();
	if (ret) {
		dev_err(dev, "Error in edma configuration\n");
		return ret;
	}

	ret = edma_irq_init();
	if (ret) {
		dev_err(dev, "Error in irq initialization\n");
		return ret;
	}

	ret = edma_irq_register();
	if (ret) {
		dev_err(dev, "Error in irq registration\n");
		kfree(edma_ctx->intr_info.intr_rx);
		kfree(edma_ctx->intr_info.intr_txcmpl);
		return ret;
	}

	edma_cfg_rx_enable_interrupts();
	edma_enable_misc_interrupt();

	dev_info(dev, "EDMA configuration successful\n");

	return 0;
}

/**
 * ppe_edma_queue_offset_config - Configure queue offset for EDMA interface
 * @ppe_dev: PPE device
 * @class: The class to configure queue offset
 * @index: Class index, internal priority or hash value
 * @queue_offset: Queue offset value
 *
 * PPE EDMA queue offset is configured based on the PPE internal priority or
 * RSS hash value, the profile ID is fixed to 0 for EDMA interface.
 *
 * Return 0 on success, negative error code on failure.
 */
int ppe_edma_queue_offset_config(struct ppe_device *ppe_dev,
				 enum ppe_queue_class_type class,
				 int index, int queue_offset)
{
	if (class == PPE_QUEUE_CLASS_PRIORITY)
		return ppe_queue_ucast_offset_pri_set(ppe_dev, 0,
						      index, queue_offset);

	return ppe_queue_ucast_offset_hash_set(ppe_dev, 0,
					       index, queue_offset);
}

/**
 * ppe_edma_queue_resource_get - Get EDMA queue resource
 * @ppe_dev: PPE device
 * @type: Resource type
 * @res_start: Resource start ID returned
 * @res_end: Resource end ID returned
 *
 * PPE EDMA queue resource includes unicast queue and multicast queue.
 *
 * Return 0 on success, negative error code on failure.
 */
int ppe_edma_queue_resource_get(struct ppe_device *ppe_dev, int type,
				int *res_start, int *res_end)
{
	if (type != PPE_RES_UCAST && type != PPE_RES_MCAST)
		return -EINVAL;

	return ppe_port_resource_get(ppe_dev, 0, type, res_start, res_end);
};

/**
 * ppe_edma_ring_to_queues_config - Map EDMA ring to PPE queues
 * @ppe_dev: PPE device
 * @ring_id: EDMA ring ID
 * @num: Number of queues mapped to EDMA ring
 * @queues: PPE queue IDs
 *
 * PPE queues are configured to map with the special EDMA ring ID.
 *
 * Return 0 on success, negative error code on failure.
 */
int ppe_edma_ring_to_queues_config(struct ppe_device *ppe_dev, int ring_id,
				   int num, int queues[])
{
	u32 queue_bmap[PPE_RING_TO_QUEUE_BITMAP_WORD_CNT] = {};
	int index;

	for (index = 0; index < num; index++)
		queue_bmap[queues[index] / 32] |= BIT_MASK(queues[index] % 32);

	return ppe_ring_queue_map_set(ppe_dev, ring_id, queue_bmap);
}
