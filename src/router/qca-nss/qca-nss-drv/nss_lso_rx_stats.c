/*
 **************************************************************************
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
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

#include "nss_stats.h"
#include "nss_core.h"
#include "nss_lso_rx.h"

/*
 * nss_lso_rx_stats_str
 *	LSO_RX stats strings
 */
static int8_t *nss_lso_rx_stats_str[NSS_LSO_RX_STATS_MAX] = {
	"tx_dropped",
	"dropped",
	"pbuf_alloc_fail",
	"pbuf_reference_fail"
};

uint64_t nss_lso_rx_stats[NSS_LSO_RX_STATS_MAX];	/* LSO_RX statistics */

/*
 * nss_lso_rx_stats_read()
 *	Read LSO_RX stats
 */
static ssize_t nss_lso_rx_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	int32_t i;

	/*
	 * max output lines = #stats + start tag line + end tag line + three blank lines
	 */
	uint32_t max_output_lines = (NSS_STATS_NODE_MAX + 2) + (NSS_LSO_RX_STATS_MAX + 3) + 5;
	size_t size_al = NSS_STATS_MAX_STR_LENGTH * max_output_lines;
	size_t size_wr = 0;
	ssize_t bytes_read = 0;
	uint64_t *stats_shadow;

	char *lbuf = kzalloc(size_al, GFP_KERNEL);
	if (unlikely(lbuf == NULL)) {
		nss_warning("Could not allocate memory for local statistics buffer");
		return 0;
	}

	stats_shadow = kzalloc(NSS_LSO_RX_STATS_MAX * 8, GFP_KERNEL);
	if (unlikely(stats_shadow == NULL)) {
		nss_warning("Could not allocate memory for local shadow buffer");
		kfree(lbuf);
		return 0;
	}

	size_wr = scnprintf(lbuf, size_al, "lso_rx stats start:\n\n");

	size_wr = nss_stats_fill_common_stats(NSS_LSO_RX_INTERFACE, lbuf, size_wr, size_al);

	/*
	 * lso_rx node stats
	 */
	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\nlso_rx node stats:\n\n");
	spin_lock_bh(&nss_top_main.stats_lock);
	for (i = 0; (i < NSS_LSO_RX_STATS_MAX); i++) {
		stats_shadow[i] = nss_lso_rx_stats[i];
	}

	spin_unlock_bh(&nss_top_main.stats_lock);

	for (i = 0; i < NSS_LSO_RX_STATS_MAX; i++) {
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr,
					"%s = %llu\n", nss_lso_rx_stats_str[i], stats_shadow[i]);
	}

	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\nlso_rx stats end\n\n");
	bytes_read = simple_read_from_buffer(ubuf, sz, ppos, lbuf, strlen(lbuf));
	kfree(lbuf);
	kfree(stats_shadow);

	return bytes_read;
}

/*
 * nss_lso_rx_stats_ops
 */
NSS_STATS_DECLARE_FILE_OPERATIONS(lso_rx)

/*
 * nss_lso_rx_stats_dentry_create()
 *	Create lso_rx statistics debug entry.
 */
void nss_lso_rx_stats_dentry_create(void)
{
	nss_stats_create_dentry("lso_rx", &nss_lso_rx_stats_ops);
}

/*
 * nss_lso_rx_stats_sync()
 *	Handle the syncing of lso_rx node statistics.
 */
void nss_lso_rx_stats_sync(struct nss_ctx_instance *nss_ctx, struct nss_lso_rx_stats_sync *nlrss)
{
	struct nss_top_instance *nss_top = nss_ctx->nss_top;
	int j;

	spin_lock_bh(&nss_top->stats_lock);

	/*
	 * common node stats
	 */
	nss_top->stats_node[NSS_LSO_RX_INTERFACE][NSS_STATS_NODE_RX_PKTS] += nlrss->node_stats.rx_packets;
	nss_top->stats_node[NSS_LSO_RX_INTERFACE][NSS_STATS_NODE_RX_BYTES] += nlrss->node_stats.rx_bytes;
	nss_top->stats_node[NSS_LSO_RX_INTERFACE][NSS_STATS_NODE_TX_PKTS] += nlrss->node_stats.tx_packets;
	nss_top->stats_node[NSS_LSO_RX_INTERFACE][NSS_STATS_NODE_TX_BYTES] += nlrss->node_stats.tx_bytes;

	for (j = 0; j < NSS_MAX_NUM_PRI; j++) {
		nss_top->stats_node[NSS_LSO_RX_INTERFACE][NSS_STATS_NODE_RX_QUEUE_0_DROPPED + j] += nlrss->node_stats.rx_dropped[j];
	}

	/*
	 * General LSO_RX stats
	 */
	nss_lso_rx_stats[NSS_LSO_RX_STATS_TX_DROPPED] += nlrss->tx_dropped;
	nss_lso_rx_stats[NSS_LSO_RX_STATS_DROPPED] += nlrss->dropped;

	/*
	 * pbuf
	 */
	nss_lso_rx_stats[NSS_LSO_RX_STATS_PBUF_ALLOC_FAIL] += nlrss->pbuf_alloc_fail;
	nss_lso_rx_stats[NSS_LSO_RX_STATS_PBUF_REFERENCE_FAIL] += nlrss->pbuf_reference_fail;

	spin_unlock_bh(&nss_top->stats_lock);
}
