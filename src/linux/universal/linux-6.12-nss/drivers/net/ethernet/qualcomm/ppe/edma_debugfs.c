// SPDX-License-Identifier: GPL-2.0-only
/* Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 */

/* EDMA debugfs routines for display of Tx/Rx counters. */

#include <linux/cpumask.h>
#include <linux/debugfs.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/printk.h>

#include "edma.h"

#define EDMA_STATS_BANNER_MAX_LEN       80
#define EDMA_RX_RING_STATS_NODE_NAME    "EDMA_RX"
#define EDMA_TX_RING_STATS_NODE_NAME    "EDMA_TX"
#define EDMA_ERR_STATS_NODE_NAME        "EDMA_ERR"

static struct dentry *edma_dentry;
static struct dentry *stats_dentry;

static void edma_debugfs_print_banner(struct seq_file *m, char *node)
{
	u32 banner_char_len, i;

	for (i = 0; i < EDMA_STATS_BANNER_MAX_LEN; i++)
		seq_puts(m, "_");
	banner_char_len = (EDMA_STATS_BANNER_MAX_LEN - (strlen(node) + 2)) / 2;
	seq_puts(m, "\n\n");

	for (i = 0; i < banner_char_len; i++)
		seq_puts(m, "<");
	seq_printf(m, " %s ", node);

	for (i = 0; i < banner_char_len; i++)
		seq_puts(m, ">");
	seq_puts(m, "\n");

	for (i = 0; i < EDMA_STATS_BANNER_MAX_LEN; i++)
		seq_puts(m, "_");
	seq_puts(m, "\n\n");
}

static int edma_debugfs_rx_rings_stats_show(struct seq_file *m,
					    void __maybe_unused *p)
{
	struct edma_hw_info *hw_info = edma_ctx->hw_info;
	struct edma_ring_info *rxfill = hw_info->rxfill;
	struct edma_rxfill_stats *rxfill_stats;
	struct edma_rxdesc_stats *rxdesc_stats;
	struct edma_ring_info *rx = hw_info->rx;
	unsigned int start;
	u32 i;

	rxfill_stats = kcalloc(rxfill->num_rings, sizeof(*rxfill_stats), GFP_KERNEL);
	if (!rxfill_stats)
		return -ENOMEM;

	rxdesc_stats = kcalloc(rx->num_rings, sizeof(*rxdesc_stats), GFP_KERNEL);
	if (!rxdesc_stats) {
		kfree(rxfill_stats);
		return -ENOMEM;
	}

	/* Get stats for Rx fill rings. */
	for (i = 0; i < rxfill->num_rings; i++) {
		struct edma_rxfill_ring *rxfill_ring;
		struct edma_rxfill_stats *stats;

		rxfill_ring = &edma_ctx->rxfill_rings[i];
		stats = &rxfill_ring->rxfill_stats;
		do {
			start = u64_stats_fetch_begin(&stats->syncp);
			rxfill_stats[i].alloc_failed = stats->alloc_failed;
			rxfill_stats[i].page_alloc_failed = stats->page_alloc_failed;
		} while (u64_stats_fetch_retry(&stats->syncp, start));
	}

	/* Get stats for Rx Desc rings. */
	for (i = 0; i < rx->num_rings; i++) {
		struct edma_rxdesc_ring *rxdesc_ring;
		struct edma_rxdesc_stats *stats;

		rxdesc_ring = &edma_ctx->rx_rings[i];
		stats = &rxdesc_ring->rxdesc_stats;
		do {
			start = u64_stats_fetch_begin(&stats->syncp);
			rxdesc_stats[i].src_port_inval = stats->src_port_inval;
			rxdesc_stats[i].src_port_inval_type = stats->src_port_inval_type;
			rxdesc_stats[i].src_port_inval_netdev = stats->src_port_inval_netdev;
		} while (u64_stats_fetch_retry(&stats->syncp, start));
	}

	edma_debugfs_print_banner(m, EDMA_RX_RING_STATS_NODE_NAME);

	seq_puts(m, "\n#EDMA RX descriptor rings stats:\n\n");
	for (i = 0; i < rx->num_rings; i++) {
		seq_printf(m, "\t\tEDMA RX descriptor %d ring stats:\n", i + rx->ring_start);
		seq_printf(m, "\t\t rxdesc[%d]:src_port_inval = %llu\n",
			   i + rx->ring_start, rxdesc_stats[i].src_port_inval);
		seq_printf(m, "\t\t rxdesc[%d]:src_port_inval_type = %llu\n",
			   i + rx->ring_start, rxdesc_stats[i].src_port_inval_type);
		seq_printf(m, "\t\t rxdesc[%d]:src_port_inval_netdev = %llu\n",
			   i + rx->ring_start,
			   rxdesc_stats[i].src_port_inval_netdev);
		seq_puts(m, "\n");
	}

	seq_puts(m, "\n#EDMA RX fill rings stats:\n\n");
	for (i = 0; i < rxfill->num_rings; i++) {
		seq_printf(m, "\t\tEDMA RX fill %d ring stats:\n", i + rxfill->ring_start);
		seq_printf(m, "\t\t rxfill[%d]:alloc_failed = %llu\n",
			   i + rxfill->ring_start, rxfill_stats[i].alloc_failed);
		seq_printf(m, "\t\t rxfill[%d]:page_alloc_failed = %llu\n",
			   i + rxfill->ring_start, rxfill_stats[i].page_alloc_failed);
		seq_puts(m, "\n");
	}

	kfree(rxfill_stats);
	kfree(rxdesc_stats);
	return 0;
}

static int edma_debugfs_tx_rings_stats_show(struct seq_file *m,
					    void __maybe_unused *p)
{
	struct edma_hw_info *hw_info = edma_ctx->hw_info;
	struct edma_ring_info *txcmpl = hw_info->txcmpl;
	struct edma_ring_info *tx = hw_info->tx;
	struct edma_txcmpl_stats *txcmpl_stats;
	struct edma_txdesc_stats *txdesc_stats;
	unsigned int start;
	u32 i;

	txcmpl_stats = kcalloc(txcmpl->num_rings, sizeof(*txcmpl_stats), GFP_KERNEL);
	if (!txcmpl_stats)
		return -ENOMEM;

	txdesc_stats = kcalloc(tx->num_rings, sizeof(*txdesc_stats), GFP_KERNEL);
	if (!txdesc_stats) {
		kfree(txcmpl_stats);
		return -ENOMEM;
	}

	/* Get stats for Tx desc rings. */
	for (i = 0; i < tx->num_rings; i++) {
		struct edma_txdesc_ring *txdesc_ring;
		struct edma_txdesc_stats *stats;

		txdesc_ring = &edma_ctx->tx_rings[i];
		stats = &txdesc_ring->txdesc_stats;
		do {
			start = u64_stats_fetch_begin(&stats->syncp);
			txdesc_stats[i].no_desc_avail = stats->no_desc_avail;
			txdesc_stats[i].tso_max_seg_exceed = stats->tso_max_seg_exceed;
		} while (u64_stats_fetch_retry(&stats->syncp, start));
	}

	/* Get stats for Tx Complete rings. */
	for (i = 0; i < txcmpl->num_rings; i++) {
		struct edma_txcmpl_ring *txcmpl_ring;
		struct edma_txcmpl_stats *stats;

		txcmpl_ring = &edma_ctx->txcmpl_rings[i];
		stats = &txcmpl_ring->txcmpl_stats;
		do {
			start = u64_stats_fetch_begin(&stats->syncp);
			txcmpl_stats[i].invalid_buffer = stats->invalid_buffer;
			txcmpl_stats[i].errors = stats->errors;
			txcmpl_stats[i].desc_with_more_bit = stats->desc_with_more_bit;
			txcmpl_stats[i].no_pending_desc = stats->no_pending_desc;
		} while (u64_stats_fetch_retry(&stats->syncp, start));
	}

	edma_debugfs_print_banner(m, EDMA_TX_RING_STATS_NODE_NAME);

	seq_puts(m, "\n#EDMA TX complete rings stats:\n\n");
	for (i = 0; i < txcmpl->num_rings; i++) {
		seq_printf(m, "\t\tEDMA TX complete %d ring stats:\n", i + txcmpl->ring_start);
		seq_printf(m, "\t\t txcmpl[%d]:invalid_buffer = %llu\n",
			   i + txcmpl->ring_start, txcmpl_stats[i].invalid_buffer);
		seq_printf(m, "\t\t txcmpl[%d]:errors = %llu\n",
			   i + txcmpl->ring_start, txcmpl_stats[i].errors);
		seq_printf(m, "\t\t txcmpl[%d]:desc_with_more_bit = %llu\n",
			   i + txcmpl->ring_start, txcmpl_stats[i].desc_with_more_bit);
		seq_printf(m, "\t\t txcmpl[%d]:no_pending_desc = %llu\n",
			   i + txcmpl->ring_start, txcmpl_stats[i].no_pending_desc);
		seq_puts(m, "\n");
	}

	seq_puts(m, "\n#EDMA TX descriptor rings stats:\n\n");
	for (i = 0; i < tx->num_rings; i++) {
		seq_printf(m, "\t\tEDMA TX descriptor %d ring stats:\n", i + tx->ring_start);
		seq_printf(m, "\t\t txdesc[%d]:no_desc_avail = %llu\n",
			   i + tx->ring_start, txdesc_stats[i].no_desc_avail);
		seq_printf(m, "\t\t txdesc[%d]:tso_max_seg_exceed = %llu\n",
			   i + tx->ring_start, txdesc_stats[i].tso_max_seg_exceed);
		seq_puts(m, "\n");
	}

	kfree(txcmpl_stats);
	kfree(txdesc_stats);
	return 0;
}

static int edma_debugfs_err_stats_show(struct seq_file *m,
				       void __maybe_unused *p)
{
	struct edma_err_stats *err_stats, *pcpu_err_stats;
	unsigned int start;
	u32 cpu;

	err_stats = kzalloc(sizeof(*err_stats), GFP_KERNEL);
	if (!err_stats)
		return -ENOMEM;

	/* Get percpu EDMA miscellaneous stats. */
	for_each_possible_cpu(cpu) {
		pcpu_err_stats = per_cpu_ptr(edma_ctx->err_stats, cpu);
		do {
			start = u64_stats_fetch_begin(&pcpu_err_stats->syncp);
			err_stats->edma_axi_read_err +=
				pcpu_err_stats->edma_axi_read_err;
			err_stats->edma_axi_write_err +=
				pcpu_err_stats->edma_axi_write_err;
			err_stats->edma_rxdesc_fifo_full +=
				pcpu_err_stats->edma_rxdesc_fifo_full;
			err_stats->edma_rx_buf_size_err +=
				pcpu_err_stats->edma_rx_buf_size_err;
			err_stats->edma_tx_sram_full +=
				pcpu_err_stats->edma_tx_sram_full;
			err_stats->edma_tx_data_len_err +=
				pcpu_err_stats->edma_tx_data_len_err;
			err_stats->edma_tx_timeout +=
				pcpu_err_stats->edma_tx_timeout;
			err_stats->edma_txcmpl_buf_full +=
				pcpu_err_stats->edma_txcmpl_buf_full;
		} while (u64_stats_fetch_retry(&pcpu_err_stats->syncp, start));
	}

	edma_debugfs_print_banner(m, EDMA_ERR_STATS_NODE_NAME);

	seq_puts(m, "\n#EDMA error stats:\n\n");
	seq_printf(m, "\t\t axi read error = %llu\n",
		   err_stats->edma_axi_read_err);
	seq_printf(m, "\t\t axi write error = %llu\n",
		   err_stats->edma_axi_write_err);
	seq_printf(m, "\t\t Rx descriptor fifo full = %llu\n",
		   err_stats->edma_rxdesc_fifo_full);
	seq_printf(m, "\t\t Rx buffer size error = %llu\n",
		   err_stats->edma_rx_buf_size_err);
	seq_printf(m, "\t\t Tx SRAM full = %llu\n",
		   err_stats->edma_tx_sram_full);
	seq_printf(m, "\t\t Tx data length error = %llu\n",
		   err_stats->edma_tx_data_len_err);
	seq_printf(m, "\t\t Tx timeout = %llu\n",
		   err_stats->edma_tx_timeout);
	seq_printf(m, "\t\t Tx completion buffer full = %llu\n",
		   err_stats->edma_txcmpl_buf_full);

	kfree(err_stats);
	return 0;
}

static int edma_debugs_rx_rings_stats_open(struct inode *inode,
					   struct file *file)
{
	return single_open(file, edma_debugfs_rx_rings_stats_show,
			   inode->i_private);
}

static const struct file_operations edma_debugfs_rx_rings_file_ops = {
	.open = edma_debugs_rx_rings_stats_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release
};

static int edma_debugs_tx_rings_stats_open(struct inode *inode, struct file *file)
{
	return single_open(file, edma_debugfs_tx_rings_stats_show, inode->i_private);
}

static const struct file_operations edma_debugfs_tx_rings_file_ops = {
	.open = edma_debugs_tx_rings_stats_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release
};

static int edma_debugs_err_stats_open(struct inode *inode, struct file *file)
{
	return single_open(file, edma_debugfs_err_stats_show, inode->i_private);
}

static const struct file_operations edma_debugfs_misc_file_ops = {
	.open = edma_debugs_err_stats_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release
};

/**
 * edma_debugfs_teardown - EDMA debugfs teardown.
 *
 * EDMA debugfs teardown and free stats memory.
 */
void edma_debugfs_teardown(void)
{
	/* Free EDMA miscellaneous stats memory */
	edma_err_stats_free();

	debugfs_remove_recursive(edma_dentry);
	edma_dentry = NULL;
	stats_dentry = NULL;
}

/**
 * edma_debugfs_setup - EDMA debugfs setup.
 * @ppe_dev: PPE Device
 *
 * EDMA debugfs setup.
 */
int edma_debugfs_setup(struct ppe_device *ppe_dev)
{
	edma_dentry = debugfs_create_dir("edma", ppe_dev->debugfs_root);
	if (!edma_dentry) {
		pr_err("Unable to create debugfs edma directory in debugfs\n");
		goto debugfs_dir_failed;
	}

	stats_dentry = debugfs_create_dir("stats", edma_dentry);
	if (!stats_dentry) {
		pr_err("Unable to create debugfs stats directory in debugfs\n");
		goto debugfs_dir_failed;
	}

	if (!debugfs_create_file("rx_ring_stats", 0444, stats_dentry,
				 NULL, &edma_debugfs_rx_rings_file_ops)) {
		pr_err("Unable to create Rx rings statistics file entry in debugfs\n");
		goto debugfs_dir_failed;
	}

	if (!debugfs_create_file("tx_ring_stats", 0444, stats_dentry,
				 NULL, &edma_debugfs_tx_rings_file_ops)) {
		pr_err("Unable to create Tx rings statistics file entry in debugfs\n");
		goto debugfs_dir_failed;
	}

	/* Allocate memory for EDMA miscellaneous stats */
	if (edma_err_stats_alloc() < 0) {
		pr_err("Unable to allocate miscellaneous percpu stats\n");
		goto debugfs_dir_failed;
	}

	if (!debugfs_create_file("err_stats", 0444, stats_dentry,
				 NULL, &edma_debugfs_misc_file_ops)) {
		pr_err("Unable to create EDMA miscellaneous statistics file entry in debugfs\n");
		goto debugfs_dir_failed;
	}

	return 0;

debugfs_dir_failed:
	debugfs_remove_recursive(edma_dentry);
	edma_dentry = NULL;
	stats_dentry = NULL;
	return -ENOMEM;
}
