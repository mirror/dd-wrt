/*
 **************************************************************************
 * Copyright (c) 2018-2019, The Linux Foundation. All rights reserved.
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
#include "nss_c2c_rx.h"

/*
 * Spinlock to protect C2C_RX statistics update/read
 */
DEFINE_SPINLOCK(nss_c2c_rx_stats_lock);

/*
 * nss_c2c_rx_stats_str
 *	C2C_RX stats strings
 */
static int8_t *nss_c2c_rx_stats_str[NSS_C2C_RX_STATS_MAX] = {
	"rx_packets",
	"rx_bytes",
	"tx_packets",
	"tx_bytes",
	"rx_queue_0_dropped",
	"rx_queue_1_dropped",
	"rx_queue_2_dropped",
	"rx_queue_3_dropped",
	"pbuf_simple",
	"pbuf_sg",
	"pbuf_returning",
	"inval_dest",
};

/*
 * nss_c2c_rx_stats
 *	c2c_rx statistics
 */
uint64_t nss_c2c_rx_stats[NSS_MAX_CORES][NSS_C2C_RX_STATS_MAX];

/*
 * nss_c2c_rx_stats_read()
 *	Read C2C_RX statistics
 */
static ssize_t nss_c2c_rx_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	int32_t i, core;

	/*
	 * Max output lines = (#stats + core tag + two blank lines) * NSS_MAX_CORES  +
	 * start tag line + end tag line + three blank lines
	 */
	uint32_t max_output_lines = (NSS_C2C_RX_STATS_MAX + 3) * NSS_MAX_CORES + 5;
	size_t size_al = NSS_STATS_MAX_STR_LENGTH * max_output_lines;
	size_t size_wr = 0;
	ssize_t bytes_read = 0;
	uint64_t *stats_shadow;

	char *lbuf = kzalloc(size_al, GFP_KERNEL);
	if (unlikely(lbuf == NULL)) {
		nss_warning("Could not allocate memory for local statistics buffer");
		return -ENOMEM;
	}

	stats_shadow = kzalloc(NSS_C2C_RX_STATS_MAX * 8, GFP_KERNEL);
	if (unlikely(stats_shadow == NULL)) {
		nss_warning("Could not allocate memory for local shadow buffer");
		kfree(lbuf);
		return -ENOMEM;
	}

	size_wr = scnprintf(lbuf, size_al, "c2c_rx stats start:\n\n");

	/*
	 * C2C_RX statistics
	 */
	for (core = 0; core < NSS_MAX_CORES; core++) {
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\nc2c_rx core %d stats:\n\n", core);
		spin_lock_bh(&nss_c2c_rx_stats_lock);
		for (i = 0; i < NSS_C2C_RX_STATS_MAX; i++) {
			stats_shadow[i] = nss_c2c_rx_stats[core][i];
		}
		spin_unlock_bh(&nss_c2c_rx_stats_lock);

		for (i = 0; i < NSS_C2C_RX_STATS_MAX; i++) {
			size_wr += scnprintf(lbuf + size_wr, size_al - size_wr,
					"%s = %llu\n", nss_c2c_rx_stats_str[i], stats_shadow[i]);
		}
	}

	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\nc2c_rx stats end\n\n");
	bytes_read = simple_read_from_buffer(ubuf, sz, ppos, lbuf, strlen(lbuf));
	kfree(lbuf);
	kfree(stats_shadow);

	return bytes_read;
}

/*
 * nss_c2c_rx_stats_ops
 */
NSS_STATS_DECLARE_FILE_OPERATIONS(c2c_rx)

/*
 * nss_c2c_rx_stats_dentry_create()
 *	Create C2C_RX statistics debug entry.
 */
void nss_c2c_rx_stats_dentry_create(void)
{
	nss_stats_create_dentry("c2c_rx", &nss_c2c_rx_stats_ops);
}

/*
 * nss_c2c_rx_stats_sync()
 *	Handle the syncing of NSS C2C_RX statistics.
 */
void nss_c2c_rx_stats_sync(struct nss_ctx_instance *nss_ctx, struct nss_c2c_rx_stats *ncrs)
{
	int id = nss_ctx->id;
	int j;

	spin_lock_bh(&nss_c2c_rx_stats_lock);

	/*
	 * Common node stats
	 */
	nss_c2c_rx_stats[id][NSS_STATS_NODE_RX_PKTS] += (ncrs->pbuf_simple + ncrs->pbuf_sg + ncrs->pbuf_returning);
	nss_c2c_rx_stats[id][NSS_STATS_NODE_RX_BYTES] += ncrs->node_stats.rx_bytes;
	nss_c2c_rx_stats[id][NSS_STATS_NODE_TX_PKTS] += ncrs->node_stats.tx_packets;
	nss_c2c_rx_stats[id][NSS_STATS_NODE_TX_BYTES] += ncrs->node_stats.tx_bytes;

	for (j = 0; j < NSS_MAX_NUM_PRI; j++) {
		nss_c2c_rx_stats[id][NSS_STATS_NODE_RX_QUEUE_0_DROPPED + j] += ncrs->node_stats.rx_dropped[j];
	}

	/*
	 * C2C_RX statistics
	 */
	nss_c2c_rx_stats[id][NSS_C2C_RX_STATS_PBUF_SIMPLE] += ncrs->pbuf_simple;
	nss_c2c_rx_stats[id][NSS_C2C_RX_STATS_PBUF_SG] += ncrs->pbuf_sg;
	nss_c2c_rx_stats[id][NSS_C2C_RX_STATS_PBUF_RETURNING] += ncrs->pbuf_returning;
	nss_c2c_rx_stats[id][NSS_C2C_RX_STATS_INVAL_DEST] += ncrs->inval_dest;

	spin_unlock_bh(&nss_c2c_rx_stats_lock);
}
