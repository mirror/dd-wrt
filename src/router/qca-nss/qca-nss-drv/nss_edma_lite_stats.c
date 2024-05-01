/*
 * Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
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

/*
 * nss_edma_lite_stats.c
 *	NSS EDMA statistics APIs
 */

#include "nss_edma_lite_stats.h"
#include "nss_edma_lite_strings.h"

/*
 * Declare atomic notifier data structure for statistics.
 */
ATOMIC_NOTIFIER_HEAD(nss_edma_lite_stats_notifier);

struct nss_edma_lite_stats edma_stats;

/*
 **********************************
 EDMA statistics APIs
 **********************************
 */

/*
 * nss_edma_lite_node_stats_read()
 *      Read EDMA node stats
 */
static ssize_t nss_edma_lite_node_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	int32_t i;

	/*
	 * max output lines = #stats + start tag line + end tag line + three blank lines
	 */
	uint32_t max_output_lines = (NSS_STATS_NODE_MAX + 2) + 3;
	size_t size_al = NSS_STATS_MAX_STR_LENGTH * max_output_lines;
	size_t size_wr = 0;
	ssize_t bytes_read = 0;
	uint64_t *stats_shadow;

	char *lbuf = kzalloc(size_al, GFP_KERNEL);
	if (unlikely(lbuf == NULL)) {
		nss_warning("Could not allocate memory for local statistics buffer");
		return 0;
	}

	stats_shadow = kzalloc(NSS_STATS_NODE_MAX * sizeof(uint64_t), GFP_KERNEL);
	if (unlikely(stats_shadow == NULL)) {
		nss_warning("Could not allocate memory for local shadow buffer");
		kfree(lbuf);
		return 0;
	}

	size_wr += nss_stats_banner(lbuf, size_wr, size_al, "edma", NSS_STATS_SINGLE_CORE);

	/*
	 * Common node stats
	 */
	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "Node stats:\n\n");
	spin_lock_bh(&nss_top_main.stats_lock);

	for (i = 0; (i < NSS_STATS_NODE_MAX); i++)
		stats_shadow[i] = edma_stats.node_stats[i];

	spin_unlock_bh(&nss_top_main.stats_lock);
	size_wr += nss_stats_print("edma_node", NULL, NSS_STATS_SINGLE_INSTANCE
					, nss_edma_lite_strings_stats_node
					, stats_shadow
					, NSS_STATS_NODE_MAX
					, lbuf, size_wr, size_al);
	bytes_read = simple_read_from_buffer(ubuf, sz, ppos, lbuf, strlen(lbuf));
	kfree(lbuf);
	kfree(stats_shadow);

	return bytes_read;
}

/*
 * nss_edma_lite_txring_stats_read()
 *	Read EDMA Tx ring stats
 */
static ssize_t nss_edma_lite_txring_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	int32_t i = 0;

	/*
	 * max output lines = #stats + start tag line + end tag line + three blank lines
	 */
	uint32_t max_output_lines = (NSS_EDMA_LITE_STATS_TX_MAX + 2) + 3;
	size_t size_al = NSS_STATS_MAX_STR_LENGTH * max_output_lines;
	size_t size_wr = 0;
	ssize_t bytes_read = 0;
	uint64_t *stats_shadow;

	char *lbuf = kzalloc(size_al, GFP_KERNEL);
	if (unlikely(lbuf == NULL)) {
		nss_warning("Could not allocate memory for local statistics buffer");
		return 0;
	}

	stats_shadow = kzalloc(NSS_EDMA_LITE_STATS_TX_MAX * sizeof(uint64_t), GFP_KERNEL);
	if (unlikely(stats_shadow == NULL)) {
		nss_warning("Could not allocate memory for local shadow buffer");
		kfree(lbuf);
		return 0;
	}

	size_wr = scnprintf(lbuf, size_al, "edma Tx ring stats start:\n\n");

	/*
	 * Tx ring stats
	 */
	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "Tx ring stats:\n\n");
	spin_lock_bh(&nss_top_main.stats_lock);
	for (i = 0; (i < NSS_EDMA_LITE_STATS_TX_MAX); i++)
		stats_shadow[i] = edma_stats.tx_stats[i];

	spin_unlock_bh(&nss_top_main.stats_lock);

	size_wr += nss_stats_print("edma_tx_ring", NULL, NSS_STATS_SINGLE_INSTANCE
					, nss_edma_lite_strings_stats_tx
					, stats_shadow
					, NSS_EDMA_LITE_STATS_TX_MAX
					, lbuf, size_wr, size_al);
	bytes_read = simple_read_from_buffer(ubuf, sz, ppos, lbuf, strlen(lbuf));
	kfree(lbuf);
	kfree(stats_shadow);

	return bytes_read;
}

/*
 * nss_edma_lite_rxring_stats_read()
 *	Read EDMA rxring stats
 */
static ssize_t nss_edma_lite_rxring_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	int32_t i= 0;

	/*
	 * max output lines = #stats + start tag line + end tag line + three blank lines
	 */
	uint32_t max_output_lines = (NSS_EDMA_LITE_STATS_RX_MAX + 2) + 3;
	size_t size_al = NSS_STATS_MAX_STR_LENGTH * max_output_lines;
	size_t size_wr = 0;
	ssize_t bytes_read = 0;
	uint64_t *stats_shadow;

	char *lbuf = kzalloc(size_al, GFP_KERNEL);
	if (unlikely(lbuf == NULL)) {
		nss_warning("Could not allocate memory for local statistics buffer");
		return 0;
	}

	stats_shadow = kzalloc(NSS_EDMA_LITE_STATS_RX_MAX * sizeof(uint64_t), GFP_KERNEL);
	if (unlikely(stats_shadow == NULL)) {
		nss_warning("Could not allocate memory for local shadow buffer");
		kfree(lbuf);
		return 0;
	}

	/*
	 * RX ring stats
	 */
	spin_lock_bh(&nss_top_main.stats_lock);
	for (i = 0; (i < NSS_EDMA_LITE_STATS_RX_MAX); i++)
		stats_shadow[i] = edma_stats.rx_stats[i];

	spin_unlock_bh(&nss_top_main.stats_lock);
	size_wr += nss_stats_print("edma_rx_ring", NULL, NSS_STATS_SINGLE_INSTANCE
					, nss_edma_lite_strings_stats_rx
					, stats_shadow
					, NSS_EDMA_LITE_STATS_RX_MAX
					, lbuf, size_wr, size_al);
	bytes_read = simple_read_from_buffer(ubuf, sz, ppos, lbuf, strlen(lbuf));
	kfree(lbuf);
	kfree(stats_shadow);

	return bytes_read;
}

/*
 * nss_edma_lite_txcmplring_stats_read()
 *	Read EDMA txcmplring stats
 */
static ssize_t nss_edma_lite_txcmplring_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	int32_t i = 0;

	/*
	 * max output lines = #stats + start tag line + end tag line + three blank lines
	 */
	uint32_t max_output_lines = (NSS_EDMA_LITE_STATS_TXCMPL_MAX + 2) + 3;
	size_t size_al = NSS_STATS_MAX_STR_LENGTH * max_output_lines;
	size_t size_wr = 0;
	ssize_t bytes_read = 0;
	uint64_t *stats_shadow;

	char *lbuf = kzalloc(size_al, GFP_KERNEL);
	if (unlikely(lbuf == NULL)) {
		nss_warning("Could not allocate memory for local statistics buffer");
		return 0;
	}

	stats_shadow = kzalloc(NSS_EDMA_LITE_STATS_TXCMPL_MAX * sizeof(uint64_t), GFP_KERNEL);
	if (unlikely(stats_shadow == NULL)) {
		nss_warning("Could not allocate memory for local shadow buffer");
		kfree(lbuf);
		return 0;
	}

	size_wr = scnprintf(lbuf, size_al, "edma Tx cmpl ring stats start:\n\n");

	/*
	 * Tx cmpl ring stats
	 */
	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "Tx cmpl ring stats:\n\n");
	spin_lock_bh(&nss_top_main.stats_lock);
	for (i = 0; (i < NSS_EDMA_LITE_STATS_TXCMPL_MAX); i++)
		stats_shadow[i] = edma_stats.txcmpl_stats[i];

	spin_unlock_bh(&nss_top_main.stats_lock);
	size_wr += nss_stats_print("edma_tx_cmpl_ring", NULL, NSS_STATS_SINGLE_INSTANCE
					, nss_edma_lite_strings_stats_txcmpl
					, stats_shadow
					, NSS_EDMA_LITE_STATS_TXCMPL_MAX
					, lbuf, size_wr, size_al);
	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\nedma Tx cmpl ring stats end\n\n");
	bytes_read = simple_read_from_buffer(ubuf, sz, ppos, lbuf, strlen(lbuf));
	kfree(lbuf);
	kfree(stats_shadow);

	return bytes_read;
}

/*
 * nss_edma_lite_rxfillring_stats_read()
 *	Read EDMA rxfillring stats
 */
static ssize_t nss_edma_lite_rxfillring_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	int32_t i = 0;

	/*
	 * max output lines = #stats + start tag line + end tag line + three blank lines
	 */
	uint32_t max_output_lines = (NSS_EDMA_LITE_STATS_RXFILL_MAX + 2) + 3;
	size_t size_al = NSS_STATS_MAX_STR_LENGTH * max_output_lines;
	size_t size_wr = 0;
	ssize_t bytes_read = 0;
	uint64_t *stats_shadow;

	char *lbuf = kzalloc(size_al, GFP_KERNEL);
	if (unlikely(lbuf == NULL)) {
		nss_warning("Could not allocate memory for local statistics buffer");
		return 0;
	}

	stats_shadow = kzalloc(NSS_EDMA_LITE_STATS_RXFILL_MAX * sizeof(uint64_t), GFP_KERNEL);
	if (unlikely(stats_shadow == NULL)) {
		nss_warning("Could not allocate memory for local shadow buffer");
		kfree(lbuf);
		return 0;
	}

	size_wr = scnprintf(lbuf, size_al, "edma Rx fill ring stats start:\n\n");

	/*
	 * Rx fill ring stats
	 */
	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "Rx fill ring stats:\n\n");
	spin_lock_bh(&nss_top_main.stats_lock);
	for (i = 0; i < NSS_EDMA_LITE_STATS_RXFILL_MAX; i++) {
		stats_shadow[i] = edma_stats.rxfill_stats[i];
	}

	spin_unlock_bh(&nss_top_main.stats_lock);
	size_wr += nss_stats_print("edma_rx_fill_ring", NULL
					, NSS_STATS_SINGLE_INSTANCE
					, nss_edma_lite_strings_stats_rxfill
					, stats_shadow
					, NSS_EDMA_LITE_STATS_RXFILL_MAX
					, lbuf, size_wr, size_al);
	bytes_read = simple_read_from_buffer(ubuf, sz, ppos, lbuf, strlen(lbuf));
	kfree(lbuf);
	kfree(stats_shadow);

	return bytes_read;
}

/*
 * nss_edma_lite_err_stats_read()
 *      Read EDMA err stats
 */
static ssize_t nss_edma_lite_err_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	int32_t i;

	/*
	 * max output lines = #stats + start tag line + end tag line + three blank lines
	 */
	uint32_t max_output_lines = (NSS_EDMA_LITE_ERR_STATS_MAX + 2) + 3;
	size_t size_al = NSS_STATS_MAX_STR_LENGTH * max_output_lines;
	size_t size_wr = 0;
	ssize_t bytes_read = 0;
	uint64_t *stats_shadow;

	char *lbuf = kzalloc(size_al, GFP_KERNEL);
	if (unlikely(lbuf == NULL)) {
		nss_warning("Could not allocate memory for local statistics buffer");
		return 0;
	}

	stats_shadow = kzalloc(NSS_EDMA_LITE_ERR_STATS_MAX * sizeof(uint64_t), GFP_KERNEL);
	if (unlikely(stats_shadow == NULL)) {
		nss_warning("Could not allocate memory for local shadow buffer");
		kfree(lbuf);
		return 0;
	}

	size_wr = scnprintf(lbuf, size_al, "edma error stats start:\n\n");

	/*
	 * Common node stats
	 */
	spin_lock_bh(&nss_top_main.stats_lock);

	for (i = 0; (i < NSS_EDMA_LITE_ERR_STATS_MAX); i++)
		stats_shadow[i] = edma_stats.err[i];

	spin_unlock_bh(&nss_top_main.stats_lock);
	size_wr += nss_stats_print("edma_err", NULL, NSS_STATS_SINGLE_INSTANCE
					, nss_edma_lite_strings_stats_err_map
					, stats_shadow
					, NSS_EDMA_LITE_ERR_STATS_MAX
					, lbuf, size_wr, size_al);
	bytes_read = simple_read_from_buffer(ubuf, sz, ppos, lbuf, strlen(lbuf));
	kfree(lbuf);
	kfree(stats_shadow);

	return bytes_read;
}

/*
 * edma_node_stats_ops
 */
NSS_STATS_DECLARE_FILE_OPERATIONS(edma_lite_node);

/*
 * edma_txring_stats_ops
 */
NSS_STATS_DECLARE_FILE_OPERATIONS(edma_lite_txring);

/*
 * edma_rxring_stats_ops
 */
NSS_STATS_DECLARE_FILE_OPERATIONS(edma_lite_rxring);

/*
 * edma_txcmplring_stats_ops
 */
NSS_STATS_DECLARE_FILE_OPERATIONS(edma_lite_txcmplring);

/*
 * edma_rxfillring_stats_ops
 */
NSS_STATS_DECLARE_FILE_OPERATIONS(edma_lite_rxfillring);

/*
 * edma_err_stats_ops
 */
NSS_STATS_DECLARE_FILE_OPERATIONS(edma_lite_err);

/*
 * nss_edma_lite_stats_dentry_create()
 *	Create edma statistics debug entry.
 */
void nss_edma_lite_stats_dentry_create(void)
{
	struct dentry *edma_d = NULL;
	struct dentry *edma_rings_dir_d = NULL;
	struct dentry *edma_tx_d = NULL;
	struct dentry *edma_rx_d = NULL;
	struct dentry *edma_txcmpl_d = NULL;
	struct dentry *edma_rxfill_d = NULL;
	struct dentry *edma_err_stats_d = NULL;
	struct dentry *edma_node_stats_d = NULL;

	edma_d = debugfs_create_dir("edma_lite", nss_top_main.stats_dentry);
	if (unlikely(edma_d == NULL)) {
		nss_warning("Failed to create qca-nss-drv/stats/edma directory");
		return;
	}

	/*
	 *  edma node stats
	 */
	edma_node_stats_d = debugfs_create_file("node_stats", 0400, edma_d, &nss_top_main, &nss_edma_lite_node_stats_ops);
	if (unlikely(edma_node_stats_d == NULL)) {
		nss_warning("Failed to create qca-nss-drv/stats/edma/node_stats file");
		return;
	}

	/*
	 *  edma error stats
	 */
	edma_err_stats_d = debugfs_create_file("err_stats", 0400, edma_d, &nss_top_main, &nss_edma_lite_err_stats_ops);
	if (unlikely(edma_err_stats_d == NULL)) {
		nss_warning("Failed to create qca-nss-drv/stats/edma/err_stats file");
		return;
	}

	/*
	 * edma ring stats
	 */
	edma_rings_dir_d = debugfs_create_dir("rings", edma_d);
	if (unlikely(edma_rings_dir_d == NULL)) {
		nss_warning("Failed to create qca-nss-drv/stats/edma/rings directory");
		return;
	}

	/*
	 * edma tx ring stats
	 */
	edma_tx_d = debugfs_create_file("tx", 0400, edma_rings_dir_d, &nss_top_main, &nss_edma_lite_txring_stats_ops);
	if (unlikely(edma_tx_d == NULL)) {
		nss_warning("Failed to create qca-nss-drv/stats/edma/rings/tx file");
		return;
	}

	/*
	 * edma rx ring stats
	 */
	edma_rx_d = debugfs_create_file("rx", 0400, edma_rings_dir_d, &nss_top_main, &nss_edma_lite_rxring_stats_ops);
	if (unlikely(edma_rx_d == NULL)) {
		nss_warning("Failed to create qca-nss-drv/stats/edma/rings/rx file");
		return;
	}

	/*
	 * edma tx cmpl ring stats
	 */
	edma_txcmpl_d = debugfs_create_file("txcmpl", 0400, edma_rings_dir_d, &nss_top_main, &nss_edma_lite_txcmplring_stats_ops);
	if (unlikely(edma_txcmpl_d == NULL)) {
		nss_warning("Failed to create qca-nss-drv/stats/edma/rings/txcmpl file");
		return;
	}

	/*
	 * edma rx fill ring stats
	 */
	edma_rxfill_d = debugfs_create_file("rxfill", 0400, edma_rings_dir_d, &nss_top_main, &nss_edma_lite_rxfillring_stats_ops);
	if (unlikely(edma_rxfill_d == NULL)) {
		nss_warning("Failed to create qca-nss-drv/stats/edma/rings/rxfill file");
		return;
	}
}

/*
 * nss_edma_lite_node_stats_sync()
 *	Handle the syncing of EDMA node statistics.
 */
void nss_edma_lite_node_stats_sync(struct nss_ctx_instance *nss_ctx, struct nss_edma_lite_node_stats_sync *nerss)
{
	int32_t i;
	struct nss_top_instance *nss_top = nss_ctx->nss_top;

	spin_lock_bh(&nss_top->stats_lock);

	/*
	 * edma node stats
	 */
	edma_stats.node_stats[NSS_STATS_NODE_RX_PKTS] += nerss->node_stats.rx_packets;
	edma_stats.node_stats[NSS_STATS_NODE_RX_BYTES] += nerss->node_stats.rx_bytes;
	edma_stats.node_stats[NSS_STATS_NODE_TX_PKTS] += nerss->node_stats.tx_packets;
	edma_stats.node_stats[NSS_STATS_NODE_TX_BYTES] += nerss->node_stats.tx_bytes;

	for (i = 0; i < NSS_MAX_NUM_PRI; i++) {
		edma_stats.node_stats[NSS_STATS_NODE_RX_QUEUE_0_DROPPED + i] += nerss->node_stats.rx_dropped[i];
	}

	spin_unlock_bh(&nss_top->stats_lock);
}

/*
 * nss_edma_lite_ring_stats_sync()
 *	Handle the syncing of EDMA ring statistics.
 */
void nss_edma_lite_ring_stats_sync(struct nss_ctx_instance *nss_ctx, struct nss_edma_lite_ring_stats_sync *nerss)
{
	struct nss_top_instance *nss_top = nss_ctx->nss_top;

	spin_lock_bh(&nss_top->stats_lock);

	/*
	 * edma tx ring stats
	 */
	edma_stats.tx_stats[NSS_EDMA_LITE_STATS_TX_ERR] += nerss->tx_ring.tx_err;
	edma_stats.tx_stats[NSS_EDMA_LITE_STATS_TX_DROPPED] += nerss->tx_ring.tx_dropped;
	edma_stats.tx_stats[NSS_EDMA_LITE_STATS_TX_DESC] += nerss->tx_ring.desc_cnt;

	/*
	 * edma rx ring stats
	 */
	edma_stats.rx_stats[NSS_EDMA_LITE_STATS_RX_DESC] += nerss->rx_ring.desc_cnt;

	/*
	 * edma tx cmpl ring stats
	 */
	edma_stats.txcmpl_stats[NSS_EDMA_LITE_STATS_TXCMPL_DESC] += nerss->txcmpl_ring.desc_cnt;

	/*
	 * edma rx fill ring stats
	 */
	edma_stats.rxfill_stats[NSS_EDMA_LITE_STATS_RXFILL_DESC] += nerss->rxfill_ring.desc_cnt;

	spin_unlock_bh(&nss_top->stats_lock);
}

/*
 * nss_edma_lite_err_stats_sync()
 *	Handle the syncing of EDMA error statistics.
 */
void nss_edma_lite_err_stats_sync(struct nss_ctx_instance *nss_ctx, struct nss_edma_lite_err_stats_sync *nerss)
{

	struct nss_top_instance *nss_top = nss_ctx->nss_top;

	spin_lock_bh(&nss_top->stats_lock);
	edma_stats.err[NSS_EDMA_LITE_ALLOC_FAIL_CNT] += nerss->alloc_fail_cnt;
	spin_unlock_bh(&nss_top->stats_lock);
}

/*
 * nss_edma_lite_stats_notify()
 *	Calls statistics notifier.
 *
 * Leverage NSS-FW statistics timing to update Netlink.
 */
void nss_edma_lite_stats_notify(struct nss_ctx_instance *nss_ctx)
{
	uint32_t core_id = nss_ctx->id;

	atomic_notifier_call_chain(&nss_edma_lite_stats_notifier, NSS_STATS_EVENT_NOTIFY, (void *)&core_id);
}

/*
 * nss_edma_lite_stats_register_notifier()
 *	Registers statistics notifier.
 */
int nss_edma_lite_stats_register_notifier(struct notifier_block *nb)
{
	return atomic_notifier_chain_register(&nss_edma_lite_stats_notifier, nb);
}
EXPORT_SYMBOL(nss_edma_lite_stats_register_notifier);

/*
 * nss_edma_lite_stats_unregister_notifier()
 *	Deregisters stats notifier.
 */
int nss_edma_lite_stats_unregister_notifier(struct notifier_block *nb)
{
	return atomic_notifier_chain_unregister(&nss_edma_lite_stats_notifier, nb);
}
EXPORT_SYMBOL(nss_edma_lite_stats_unregister_notifier);
