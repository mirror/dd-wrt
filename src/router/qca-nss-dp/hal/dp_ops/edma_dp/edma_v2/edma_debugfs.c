/*
 * Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <linux/debugfs.h>
#include "edma.h"
#include "edma_debug.h"
#include "edma_debugfs.h"

/*
 * edma_debugfs_print_banner()
 *	API to print the banner for a node
 */
static void edma_debugfs_print_banner(struct seq_file *m, char *node)
{
	uint32_t banner_char_len, i;

	for (i = 0; i < EDMA_STATS_BANNER_MAX_LEN; i++) {
		seq_printf(m, "_");
	}

	banner_char_len = (EDMA_STATS_BANNER_MAX_LEN - (strlen(node) + 2)) / 2;

	seq_printf(m, "\n\n");

	for (i = 0; i < banner_char_len; i++) {
		seq_printf(m, "<");
	}

	seq_printf(m, " %s ", node);

	for (i = 0; i < banner_char_len; i++) {
		seq_printf(m, ">");
	}
	seq_printf(m, "\n");

	for (i = 0; i < EDMA_STATS_BANNER_MAX_LEN; i++) {
		seq_printf(m, "_");
	}

	seq_printf(m, "\n\n");
}

/*
 * edma_debugfs_rx_rings_stats_show()
 *	EDMA debugfs rx rings stats show API
 */
static int edma_debugfs_rx_rings_stats_show(struct seq_file *m, void __attribute__((unused))*p)
{
	struct edma_rx_fill_stats *rx_fill_stats;
	struct edma_rx_desc_stats *rx_desc_stats;
	struct edma_gbl_ctx *egc = &edma_gbl_ctx;
	uint32_t rx_fill_start_id = egc->rxfill_ring_start;
	uint32_t rx_desc_start_id = egc->rxdesc_ring_start;
	uint32_t i;
	unsigned int start;

	rx_fill_stats = kzalloc(egc->num_rxfill_rings * sizeof(struct edma_rx_fill_stats),
				 GFP_KERNEL);
	if (!rx_fill_stats) {
		edma_err("Error in allocating the Rx fill stats buffer\n");
		return -ENOMEM;
	}

	rx_desc_stats = kzalloc(egc->num_rxdesc_rings * sizeof(struct edma_rx_desc_stats),
				 GFP_KERNEL);
	if (!rx_desc_stats) {
		edma_err("Error in allocating the Rx descriptor stats buffer\n");
		kfree(rx_fill_stats);
		return -ENOMEM;
	}

	/*
	 * Get stats for Rx fill rings
	 */
	for (i = 0; i < egc->num_rxfill_rings; i++) {
		struct edma_rxfill_ring *rxfill_ring;
		struct edma_rx_fill_stats *stats;

		rxfill_ring = &egc->rxfill_rings[i];
		stats = &rxfill_ring->rx_fill_stats;
		do {
			start = u64_stats_fetch_begin_irq(&stats->syncp);
			rx_fill_stats[i].alloc_failed = stats->alloc_failed;
			rx_fill_stats[i].page_alloc_failed = stats->page_alloc_failed;
		} while (u64_stats_fetch_retry_irq(&stats->syncp, start));
	}

	/*
	 * Get stats for Rx Desc rings
	 */
	for (i = 0; i < edma_gbl_ctx.num_rxdesc_rings; i++) {
		struct edma_rxdesc_ring *rxdesc_ring;
		struct edma_rx_desc_stats *stats;

		rxdesc_ring = &egc->rxdesc_rings[i];
		stats = &rxdesc_ring->rx_desc_stats;
		do {
			start = u64_stats_fetch_begin_irq(&stats->syncp);
			rx_desc_stats[i].src_port_inval = stats->src_port_inval;
			rx_desc_stats[i].src_port_inval_type = stats->src_port_inval_type;
			rx_desc_stats[i].src_port_inval_netdev = stats->src_port_inval_netdev;
		} while (u64_stats_fetch_retry_irq(&stats->syncp, start));
	}

	edma_debugfs_print_banner(m, EDMA_RX_RING_STATS_NODE_NAME);

	seq_printf(m, "\n#EDMA RX descriptor rings stats:\n\n");
	for (i = 0; i < edma_gbl_ctx.num_rxdesc_rings; i++) {
		seq_printf(m, "\t\tEDMA RX descriptor %d ring stats:\n", i + rx_desc_start_id);
		seq_printf(m, "\t\t rxdesc[%d]:src_port_inval = %llu\n",
				i + rx_desc_start_id, rx_desc_stats[i].src_port_inval);
		seq_printf(m, "\t\t rxdesc[%d]:src_port_inval_type = %llu\n",
				i + rx_desc_start_id, rx_desc_stats[i].src_port_inval_type);
		seq_printf(m, "\t\t rxdesc[%d]:src_port_inval_netdev = %llu\n",
				i + rx_desc_start_id,
				rx_desc_stats[i].src_port_inval_netdev);
		seq_printf(m, "\n");
	}

	seq_printf(m, "\n#EDMA RX fill rings stats:\n\n");
	for (i = 0; i < edma_gbl_ctx.num_rxfill_rings; i++) {
		seq_printf(m, "\t\tEDMA RX fill %d ring stats:\n", i + rx_fill_start_id);
		seq_printf(m, "\t\t rxfill[%d]:alloc_failed = %llu\n",
				i + rx_fill_start_id, rx_fill_stats[i].alloc_failed);
		seq_printf(m, "\t\t rxfill[%d]:page_alloc_failed = %llu\n",
				i + rx_fill_start_id, rx_fill_stats[i].page_alloc_failed);
		seq_printf(m, "\n");
	}

	kfree(rx_fill_stats);
	kfree(rx_desc_stats);
	return 0;
}

/*
 * edma_debugfs_tx_rings_stats_show()
 *	EDMA debugfs Tx rings stats show API
 */
static int edma_debugfs_tx_rings_stats_show(struct seq_file *m, void __attribute__((unused))*p)
{
	struct edma_tx_cmpl_stats *tx_cmpl_stats;
	struct edma_tx_desc_stats *tx_desc_stats;
	struct edma_gbl_ctx *egc = &edma_gbl_ctx;
	uint32_t tx_cmpl_start_id = egc->txcmpl_ring_start;
	uint32_t tx_desc_start_id = egc->txdesc_ring_start;
	uint32_t i;
	unsigned int start;

	tx_cmpl_stats = kzalloc(egc->num_txcmpl_rings * sizeof(struct edma_tx_cmpl_stats), GFP_KERNEL);
	if (!tx_cmpl_stats) {
		edma_err("Error in allocating the Tx complete stats buffer\n");
		return -ENOMEM;
	}

	tx_desc_stats = kzalloc(egc->num_txdesc_rings * sizeof(struct edma_tx_desc_stats), GFP_KERNEL);
	if (!tx_desc_stats) {
		edma_err("Error in allocating the Tx descriptor stats buffer\n");
		kfree(tx_cmpl_stats);
		return -ENOMEM;
	}

	/*
	 * Get stats for Tx desc rings
	 */
	for (i = 0; i < egc->num_txdesc_rings; i++) {
		struct edma_txdesc_ring *txdesc_ring;
		struct edma_tx_desc_stats *stats;

		txdesc_ring = &egc->txdesc_rings[i];
		stats = &txdesc_ring->tx_desc_stats;
		do {
			start = u64_stats_fetch_begin_irq(&stats->syncp);
			tx_desc_stats[i].no_desc_avail = stats->no_desc_avail;
			tx_desc_stats[i].tso_max_seg_exceed = stats->tso_max_seg_exceed;
		} while (u64_stats_fetch_retry_irq(&stats->syncp, start));
	}

	/*
	 * Get stats for Tx Complete rings
	 */
	for (i = 0; i < egc->num_txcmpl_rings; i++) {
		struct edma_txcmpl_ring *txcmpl_ring;
		struct edma_tx_cmpl_stats *stats;

		txcmpl_ring = &egc->txcmpl_rings[i];
		stats = &txcmpl_ring->tx_cmpl_stats;
		do {
			start = u64_stats_fetch_begin_irq(&stats->syncp);
			tx_cmpl_stats[i].invalid_buffer = stats->invalid_buffer;
			tx_cmpl_stats[i].errors = stats->errors;
			tx_cmpl_stats[i].desc_with_more_bit = stats->desc_with_more_bit;
			tx_cmpl_stats[i].no_pending_desc = stats->no_pending_desc;
		} while (u64_stats_fetch_retry_irq(&stats->syncp, start));
	}

	edma_debugfs_print_banner(m, EDMA_TX_RING_STATS_NODE_NAME);

	seq_printf(m, "\n#EDMA TX complete rings stats:\n\n");
	for (i = 0; i < edma_gbl_ctx.num_txcmpl_rings; i++) {
		seq_printf(m, "\t\tEDMA TX complete %d ring stats:\n", i + tx_cmpl_start_id);
		seq_printf(m, "\t\t txcmpl[%d]:invalid_buffer = %llu\n",
				i + tx_cmpl_start_id, tx_cmpl_stats[i].invalid_buffer);
		seq_printf(m, "\t\t txcmpl[%d]:errors = %llu\n",
				i + tx_cmpl_start_id, tx_cmpl_stats[i].errors);
		seq_printf(m, "\t\t txcmpl[%d]:desc_with_more_bit = %llu\n",
				i + tx_cmpl_start_id, tx_cmpl_stats[i].desc_with_more_bit);
		seq_printf(m, "\t\t txcmpl[%d]:no_pending_desc = %llu\n",
				i + tx_cmpl_start_id, tx_cmpl_stats[i].no_pending_desc);
		seq_printf(m, "\n");
	}

	seq_printf(m, "\n#EDMA TX descriptor rings stats:\n\n");
	for (i = 0; i < edma_gbl_ctx.num_txdesc_rings; i++) {
		seq_printf(m, "\t\tEDMA TX descriptor %d ring stats:\n", i + tx_desc_start_id);
		seq_printf(m, "\t\t txdesc[%d]:no_desc_avail = %llu\n",
				i + tx_desc_start_id, tx_desc_stats[i].no_desc_avail);
		seq_printf(m, "\t\t txdesc[%d]:tso_max_seg_exceed = %llu\n",
				i + tx_desc_start_id, tx_desc_stats[i].tso_max_seg_exceed);
		seq_printf(m, "\n");
	}

	kfree(tx_cmpl_stats);
	kfree(tx_desc_stats);
	return 0;
}

/*
 * edma_debugfs_misc_stats_show()
 *	EDMA debugfs miscellaneous stats show API
 */
static int edma_debugfs_misc_stats_show(struct seq_file *m, void __attribute__((unused))*p)
{
	struct edma_misc_stats *misc_stats, *pcpu_misc_stats;
	uint32_t cpu;
	unsigned int start;

	misc_stats = kzalloc(sizeof(struct edma_misc_stats), GFP_KERNEL);
	if (!misc_stats) {
		edma_err("Error in allocating the miscellaneous stats buffer\n");
		return -ENOMEM;
	}

	/*
	 * Get percpu EDMA miscellaneous stats
	 */
	for_each_possible_cpu(cpu) {
		pcpu_misc_stats = per_cpu_ptr(edma_gbl_ctx.misc_stats, cpu);
		do {
			start = u64_stats_fetch_begin_irq(&pcpu_misc_stats->syncp);
			misc_stats->edma_misc_axi_read_err +=
				pcpu_misc_stats->edma_misc_axi_read_err;
			misc_stats->edma_misc_axi_write_err +=
				pcpu_misc_stats->edma_misc_axi_write_err;
			misc_stats->edma_misc_rx_desc_fifo_full +=
				pcpu_misc_stats->edma_misc_rx_desc_fifo_full;
			misc_stats->edma_misc_rx_buf_size_err +=
				pcpu_misc_stats->edma_misc_rx_buf_size_err;
			misc_stats->edma_misc_tx_sram_full +=
				pcpu_misc_stats->edma_misc_tx_sram_full;
			misc_stats->edma_misc_tx_data_len_err +=
				pcpu_misc_stats->edma_misc_tx_data_len_err;
			misc_stats->edma_misc_tx_timeout +=
				pcpu_misc_stats->edma_misc_tx_timeout;
			misc_stats->edma_misc_tx_cmpl_buf_full +=
				pcpu_misc_stats->edma_misc_tx_cmpl_buf_full;
		} while (u64_stats_fetch_retry_irq(&pcpu_misc_stats->syncp, start));
	}

	edma_debugfs_print_banner(m, EDMA_MISC_STATS_NODE_NAME);

	seq_printf(m, "\n#EDMA miscellaneous stats:\n\n");
	seq_printf(m, "\t\t miscellaneous axi read error = %llu\n",
			misc_stats->edma_misc_axi_read_err);
	seq_printf(m, "\t\t miscellaneous axi write error = %llu\n",
			misc_stats->edma_misc_axi_write_err);
	seq_printf(m, "\t\t miscellaneous Rx descriptor fifo full = %llu\n",
			misc_stats->edma_misc_rx_desc_fifo_full);
	seq_printf(m, "\t\t miscellaneous Rx buffer size error = %llu\n",
			misc_stats->edma_misc_rx_buf_size_err);
	seq_printf(m, "\t\t miscellaneous Tx SRAM full = %llu\n",
			misc_stats->edma_misc_tx_sram_full);
	seq_printf(m, "\t\t miscellaneous Tx data length error = %llu\n",
			misc_stats->edma_misc_tx_data_len_err);
	seq_printf(m, "\t\t miscellaneous Tx timeout = %llu\n",
			misc_stats->edma_misc_tx_timeout);
	seq_printf(m, "\t\t miscellaneous Tx completion buffer full = %llu\n",
			misc_stats->edma_misc_tx_cmpl_buf_full);

	kfree(misc_stats);
	return 0;
}

/*
 * edma_debugs_rx_rings_stats_open()
 *	EDMA debugfs Rx rings open callback API
 */
static int edma_debugs_rx_rings_stats_open(struct inode *inode, struct file *file)
{
	return single_open(file, edma_debugfs_rx_rings_stats_show, inode->i_private);
}

/*
 * edma_debugfs_rx_rings_file_ops
 *	File operations for EDMA Rx rings stats
 */
const struct file_operations edma_debugfs_rx_rings_file_ops = {
	.open = edma_debugs_rx_rings_stats_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release
};

/*
 * edma_debugs_tx_rings_stats_open()
 *	EDMA debugfs Tx rings open callback API
 */
static int edma_debugs_tx_rings_stats_open(struct inode *inode, struct file *file)
{
	return single_open(file, edma_debugfs_tx_rings_stats_show, inode->i_private);
}

/*
 * edma_debugfs_tx_rings_file_ops
 *	File operations for EDMA Tx rings stats
 */
const struct file_operations edma_debugfs_tx_rings_file_ops = {
	.open = edma_debugs_tx_rings_stats_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release
};

/*
 * edma_debugs_misc_stats_open()
 *	EDMA debugfs miscellaneous stats open callback API
 */
static int edma_debugs_misc_stats_open(struct inode *inode, struct file *file)
{
	return single_open(file, edma_debugfs_misc_stats_show, inode->i_private);
}

/*
 * edma_debugfs_misc_file_ops
 *	File operations for EDMA miscellaneous stats
 */
const struct file_operations edma_debugfs_misc_file_ops = {
	.open = edma_debugs_misc_stats_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release
};

/*
 * edma_debugfs_init()
 *	EDMA debugfs init API
 */
int edma_debugfs_init(void)
{
	edma_gbl_ctx.root_dentry = debugfs_create_dir("qca-nss-dp", NULL);
	if (!edma_gbl_ctx.root_dentry) {
		edma_err("Unable to create debugfs qca-nss-dp directory in debugfs\n");
		return -1;
	}

	edma_gbl_ctx.stats_dentry = debugfs_create_dir("stats", edma_gbl_ctx.root_dentry);
	if (!edma_gbl_ctx.stats_dentry) {
		edma_err("Unable to create debugfs stats directory in debugfs\n");
		goto debugfs_dir_failed;
	}

	if (!debugfs_create_file("rx_ring_stats", S_IRUGO, edma_gbl_ctx.stats_dentry,
			NULL, &edma_debugfs_rx_rings_file_ops)) {
		edma_err("Unable to create Rx rings statistics file entry in debugfs\n");
		goto debugfs_dir_failed;
	}

	if (!debugfs_create_file("tx_ring_stats", S_IRUGO, edma_gbl_ctx.stats_dentry,
			NULL, &edma_debugfs_tx_rings_file_ops)) {
		edma_err("Unable to create Tx rings statistics file entry in debugfs\n");
		goto debugfs_dir_failed;
	}

	/*
	 * Allocate memory for EDMA miscellaneous stats
	 */
	if (edma_misc_stats_alloc() < 0) {
		edma_err("Unable to allocate miscellaneous percpu stats\n");
		goto debugfs_dir_failed;
	}

	if (!debugfs_create_file("misc_stats", S_IRUGO, edma_gbl_ctx.stats_dentry,
			NULL, &edma_debugfs_misc_file_ops)) {
		edma_err("Unable to create EDMA miscellaneous statistics file entry in debugfs\n");
		goto debugfs_dir_failed;
	}

	return 0;

debugfs_dir_failed:
	debugfs_remove_recursive(edma_gbl_ctx.root_dentry);
	edma_gbl_ctx.root_dentry = NULL;
	edma_gbl_ctx.stats_dentry = NULL;
	return -1;
}

/*
 * edma_debugfs_exit()
 *	EDMA debugfs exit API
 */
void edma_debugfs_exit(void)
{
	/*
	 * Free EDMA miscellaneous stats memory
	 */
	edma_misc_stats_free();

	if (edma_gbl_ctx.root_dentry) {
		debugfs_remove_recursive(edma_gbl_ctx.root_dentry);
		edma_gbl_ctx.root_dentry = NULL;
		edma_gbl_ctx.stats_dentry = NULL;
	}
}
