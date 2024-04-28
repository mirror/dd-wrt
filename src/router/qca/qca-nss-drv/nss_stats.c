/*
 **************************************************************************
 * Copyright (c) 2013-2018, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

/*
 * nss_stats.c
 *	NSS stats APIs
 *
 */

#include "nss_tx_rx_common.h"
#include "nss_core.h"
#include "nss_stats.h"

/*
 * nss_stats_str_drv
 *	Host driver stats strings
 */
static int8_t *nss_stats_str_drv[NSS_STATS_DRV_MAX] = {
	"nbuf_alloc_errors",
	"paged_buf_alloc_errors",
	"tx_queue_full[0]",
	"tx_queue_full[1]",
	"tx_buffers_empty",
	"tx_paged_buffers_empty",
	"tx_buffers_pkt",
	"tx_buffers_cmd",
	"tx_buffers_crypto",
	"tx_buffers_reuse",
	"rx_buffers_empty",
	"rx_buffers_pkt",
	"rx_buffers_cmd_resp",
	"rx_buffers_status_sync",
	"rx_buffers_crypto",
	"rx_buffers_virtual",
	"tx_skb_simple",
	"tx_skb_nr_frags",
	"tx_skb_fraglist",
	"rx_skb_simple",
	"rx_skb_nr_frags",
	"rx_skb_fraglist",
	"rx_bad_desciptor",
	"nss_skb_count",
	"rx_chain_seg_processed",
	"rx_frag_seg_processed",
	"tx_buffers_cmd_queue_full",
#ifdef NSS_MULTI_H2N_DATA_RING_SUPPORT
	"tx_buffers_data_queue_0",
	"tx_buffers_data_queue_1",
	"tx_buffers_data_queue_2",
	"tx_buffers_data_queue_3",
	"tx_buffers_data_queue_4",
	"tx_buffers_data_queue_5",
	"tx_buffers_data_queue_6",
	"tx_buffers_data_queue_7",
#endif
};

/*
 * nss_stats_str_gmac
 *	GMAC stats strings
 */
static int8_t *nss_stats_str_gmac[NSS_STATS_GMAC_MAX] = {
	"ticks",
	"worst_ticks",
	"iterations"
};

/*
 * nss_stats_str_node
 *	Interface stats strings per node
 */
static int8_t *nss_stats_str_node[NSS_STATS_NODE_MAX] = {
	"rx_packets",
	"rx_bytes",
	"tx_packets",
	"tx_bytes",
	"rx_queue_0_dropped",
	"rx_queue_1_dropped",
	"rx_queue_2_dropped",
	"rx_queue_3_dropped",
};

/*
 * nss_stats_create_dentry()
 *	Create statistics debug entry for subsystem.
 */
void nss_stats_create_dentry(char *name, const struct file_operations *ops)
{
	if (!debugfs_create_file(name, 0400, nss_top_main.stats_dentry, &nss_top_main, ops)) {
		nss_warning("Faied to create debug entry for subsystem %s\n", name);
	}
}

/*
 * nss_stats_fill_common_stats()
 *	Fill common node statistics.
 */
size_t nss_stats_fill_common_stats(uint32_t if_num, char *lbuf, size_t size_wr, size_t size_al)
{
	uint64_t stats_shadow[NSS_STATS_NODE_MAX];
	int i;

	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "common node stats:\n\n");
	spin_lock_bh(&nss_top_main.stats_lock);
	for (i = 0; (i < NSS_STATS_NODE_MAX); i++) {
		stats_shadow[i] = nss_top_main.stats_node[if_num][i];
	}
	spin_unlock_bh(&nss_top_main.stats_lock);

	for (i = 0; (i < NSS_STATS_NODE_MAX); i++) {
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr,
					"%s = %llu\n", nss_stats_str_node[i], stats_shadow[i]);
	}

	return size_wr;
}

/*
 * nss_drv_stats_read()
 *	Read HLOS driver stats
 */
static ssize_t nss_drv_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	int32_t i;

	/*
	 * max output lines = #stats + start tag line + end tag line + three blank lines
	 */
	uint32_t max_output_lines = NSS_STATS_DRV_MAX + 5;
	size_t size_al = NSS_STATS_MAX_STR_LENGTH * max_output_lines;
	size_t size_wr = 0;
	ssize_t bytes_read = 0;
	uint64_t *stats_shadow;

	char *lbuf = kzalloc(size_al, GFP_KERNEL);
	if (unlikely(lbuf == NULL)) {
		nss_warning("Could not allocate memory for local statistics buffer");
		return 0;
	}

	stats_shadow = kzalloc(NSS_STATS_DRV_MAX * 8, GFP_KERNEL);
	if (unlikely(stats_shadow == NULL)) {
		nss_warning("Could not allocate memory for local shadow buffer");
		kfree(lbuf);
		return 0;
	}

	size_wr = scnprintf(lbuf, size_al, "drv stats start:\n\n");
	for (i = 0; (i < NSS_STATS_DRV_MAX); i++) {
		stats_shadow[i] = NSS_PKT_STATS_READ(&nss_top_main.stats_drv[i]);
	}

	for (i = 0; (i < NSS_STATS_DRV_MAX); i++) {
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr,
					"%s = %llu\n", nss_stats_str_drv[i], stats_shadow[i]);
	}

	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\ndrv stats end\n\n");
	bytes_read = simple_read_from_buffer(ubuf, sz, ppos, lbuf, strlen(lbuf));
	kfree(lbuf);
	kfree(stats_shadow);

	return bytes_read;
}

/*
 * nss_gmac_stats_read()
 *	Read GMAC stats
 */
static ssize_t nss_gmac_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	uint32_t i, id;

	/*
	 * max output lines = ((#stats + start tag + one blank) * #GMACs) + start/end tag + 3 blank
	 */
	uint32_t max_output_lines = ((NSS_STATS_GMAC_MAX + 2) * NSS_MAX_PHYSICAL_INTERFACES) + 5;
	size_t size_al = NSS_STATS_MAX_STR_LENGTH * max_output_lines;
	size_t size_wr = 0;
	ssize_t bytes_read = 0;
	uint64_t *stats_shadow;

	char *lbuf = kzalloc(size_al, GFP_KERNEL);
	if (unlikely(lbuf == NULL)) {
		nss_warning("Could not allocate memory for local statistics buffer");
		return 0;
	}

	stats_shadow = kzalloc(NSS_STATS_GMAC_MAX * 8, GFP_KERNEL);
	if (unlikely(stats_shadow == NULL)) {
		nss_warning("Could not allocate memory for local shadow buffer");
		kfree(lbuf);
		return 0;
	}

	size_wr = scnprintf(lbuf, size_al, "gmac stats start:\n\n");

	for (id = 0; id < NSS_MAX_PHYSICAL_INTERFACES; id++) {
		spin_lock_bh(&nss_top_main.stats_lock);
		for (i = 0; (i < NSS_STATS_GMAC_MAX); i++) {
			stats_shadow[i] = nss_top_main.stats_gmac[id][i];
		}

		spin_unlock_bh(&nss_top_main.stats_lock);

		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "GMAC ID: %d\n", id);
		for (i = 0; (i < NSS_STATS_GMAC_MAX); i++) {
			size_wr += scnprintf(lbuf + size_wr, size_al - size_wr,
					"%s = %llu\n", nss_stats_str_gmac[i], stats_shadow[i]);
		}
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n");
	}

	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\ngmac stats end\n\n");
	bytes_read = simple_read_from_buffer(ubuf, sz, ppos, lbuf, strlen(lbuf));
	kfree(lbuf);
	kfree(stats_shadow);

	return bytes_read;
}

/*
 * nss_wt_stats_read()
 *	Reads and formats worker thread statistics and outputs them to ubuf
 */
static ssize_t nss_wt_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	struct nss_stats_data *data = fp->private_data;
	struct nss_ctx_instance *nss_ctx = data->nss_ctx;
	struct nss_project_irq_stats *shadow;
	uint32_t thread_count = nss_ctx->worker_thread_count;
	uint32_t irq_count = nss_ctx->irq_count;

	/*
	 * Three lines for each IRQ
	 */
	uint32_t max_output_lines = thread_count * 3 * irq_count;
	size_t size_al = max_output_lines * NSS_STATS_MAX_STR_LENGTH;
	size_t size_wr = 0;
	ssize_t bytes_read = 0;
	char *lbuf;
	int i;
	int j;

	lbuf = kzalloc(size_al, GFP_KERNEL);
	if (unlikely(!lbuf)) {
		nss_warning("Could not allocate memory for local statistics buffer\n");
		return 0;
	}

	shadow = kzalloc(thread_count * irq_count * sizeof(struct nss_project_irq_stats), GFP_KERNEL);
	if (unlikely(!shadow)) {
		nss_warning("Could not allocate memory for stats shadow\n");
		kfree(lbuf);
		return 0;
	}

	spin_lock_bh(&nss_top_main.stats_lock);
	if (unlikely(!nss_ctx->wt_stats)) {
		spin_unlock_bh(&nss_top_main.stats_lock);
		nss_warning("Worker thread statistics not allocated\n");
		kfree(lbuf);
		kfree(shadow);
		return 0;
	}
	for (i = 0; i < thread_count; ++i) {

		/*
		 * The statistics shadow is an array with thread_count * irq_count
		 * items in it. Each item is located at the index:
		 *      (thread number) * (irq_count) + (irq number)
		 * thus simulating a two-dimensional array.
		 */
		for (j = 0; j < irq_count; ++j) {
			shadow[i * irq_count + j] = nss_ctx->wt_stats[i].irq_stats[j];
		}
	}
	spin_unlock_bh(&nss_top_main.stats_lock);

	for (i = 0; i < thread_count; ++i) {
		for (j = 0; j < irq_count; ++j) {
			struct nss_project_irq_stats *is = &(shadow[i * irq_count + j]);
			if (!(is->count)) {
				continue;
			}

			size_wr += scnprintf(lbuf + size_wr, size_al - size_wr,
				"t-%d:irq-%d callback: 0x%x, count: %llu\n",
				i, j, is->callback, is->count);
			size_wr += scnprintf(lbuf + size_wr, size_al - size_wr,
				"t-%d:irq-%d tick min: %10u  avg: %10u  max:%10u\n",
				i, j, is->ticks_min, is->ticks_avg, is->ticks_max);
			size_wr += scnprintf(lbuf + size_wr, size_al - size_wr,
				"t-%d:irq-%d insn min: %10u  avg: %10u  max:%10u\n\n",
				i, j, is->insn_min, is->insn_avg, is->insn_max);
		}
	}
	bytes_read = simple_read_from_buffer(ubuf, sz, ppos, lbuf, strlen(lbuf));
	kfree(lbuf);
	kfree(shadow);

	return bytes_read;
}

/*
 * nss_stats_open()
 */
int nss_stats_open(struct inode *inode, struct file *filp)
{
	struct nss_stats_data *data = NULL;

	data = kzalloc(sizeof(struct nss_stats_data), GFP_KERNEL);
	if (!data) {
		return -ENOMEM;
	}
	memset(data, 0, sizeof (struct nss_stats_data));
	data->if_num = NSS_DYNAMIC_IF_START;
	data->index = 0;
	data->edma_id = (nss_ptr_t)inode->i_private;
	data->nss_ctx = (struct nss_ctx_instance *)(inode->i_private);
	filp->private_data = data;

	return 0;
}

/*
 * nss_stats_release()
 */
int nss_stats_release(struct inode *inode, struct file *filp)
{
	struct nss_stats_data *data = filp->private_data;

	if (data) {
		kfree(data);
	}

	return 0;
}

/*
 * drv_stats_ops
 */
NSS_STATS_DECLARE_FILE_OPERATIONS(drv)

/*
 * gmac_stats_ops
 */
NSS_STATS_DECLARE_FILE_OPERATIONS(gmac)

/*
 * wt_stats_ops
 */
NSS_STATS_DECLARE_FILE_OPERATIONS(wt)

/*
 * nss_stats_clean()
 * 	Cleanup NSS statistics files
 */
void nss_stats_clean(void)
{
	/*
	 * Remove debugfs tree
	 */
	if (likely(nss_top_main.top_dentry != NULL)) {
		debugfs_remove_recursive(nss_top_main.top_dentry);
		nss_top_main.top_dentry = NULL;
	}
}

/*
 * nss_stats_init()
 * 	Enable NSS statistics
 */
void nss_stats_init(void)
{
	struct dentry *core_dentry = NULL;
	struct dentry *wt_dentry = NULL;
	char file_name[10];
	int i;

	/*
	 * NSS driver entry
	 */
	nss_top_main.top_dentry = debugfs_create_dir("qca-nss-drv", NULL);
	if (unlikely(nss_top_main.top_dentry == NULL)) {
		nss_warning("Failed to create qca-nss-drv directory in debugfs");

		/*
		 * Non availability of debugfs directory is not a catastrophy
		 * We can still go ahead with other initialization
		 */
		return;
	}

	nss_top_main.stats_dentry = debugfs_create_dir("stats", nss_top_main.top_dentry);
	if (unlikely(nss_top_main.stats_dentry == NULL)) {
		nss_warning("Failed to create qca-nss-drv directory in debugfs");

		/*
		 * Non availability of debugfs directory is not a catastrophy
		 * We can still go ahead with rest of initialization
		 */
		return;
	}

	/*
	 * Create files to obtain statistics
	 */

	/*
	 * drv_stats
	 */
	nss_stats_create_dentry("drv", &nss_drv_stats_ops);

	/*
	 * gmac_stats
	 */
	nss_stats_create_dentry("gmac", &nss_gmac_stats_ops);

	/*
	 * Per-project stats
	 */
	nss_top_main.project_dentry = debugfs_create_dir("project",
						nss_top_main.stats_dentry);
	if (unlikely(nss_top_main.project_dentry == NULL)) {
		nss_warning("Failed to create qca-nss-drv/stats/project directory in debugfs");
		return;
	}

	for (i = 0; i < nss_top_main.num_nss; ++i) {
		memset(file_name, 0, sizeof(file_name));
		scnprintf(file_name, sizeof(file_name), "core%d", i);
		core_dentry = debugfs_create_dir(file_name,
						nss_top_main.project_dentry);
		if (unlikely(core_dentry == NULL)) {
			nss_warning("Failed to create qca-nss-drv/stats/project/core%d directory in debugfs", i);
			return;
		}

		wt_dentry = debugfs_create_file("worker_threads",
						0400,
						core_dentry,
						&(nss_top_main.nss[i]),
						&nss_wt_stats_ops);
		if (unlikely(wt_dentry == NULL)) {
			nss_warning("Failed to create qca-nss-drv/stats/project/core%d/worker_threads file in debugfs", i);
			return;
		}
	}

	nss_log_init();
}
