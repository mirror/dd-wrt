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
#include "nss_eth_rx_stats.h"

/*
 * nss_eth_rx_stats_str
 *	eth_rx stats strings
 */
static int8_t *nss_eth_rx_stats_str[NSS_ETH_RX_STATS_MAX] = {
	"ticks",
	"worst_ticks",
	"iterations"
};

/*
 * nss_eth_rx_exception_stats_str
 *	Interface stats strings for unknown exceptions
 */
static int8_t *nss_eth_rx_exception_stats_str[NSS_ETH_RX_EXCEPTION_EVENT_MAX] = {
	"UNKNOWN_L3_PROTOCOL",
	"ETH_HDR_MISSING",
	"VLAN_MISSING",
	"TRUSTSEC_HDR_MISSING"
};

uint64_t nss_eth_rx_stats[NSS_ETH_RX_STATS_MAX];			/* ETH_RX statistics */
uint64_t nss_eth_rx_exception_stats[NSS_ETH_RX_EXCEPTION_EVENT_MAX];	/* Unknown protocol exception events per interface */

/*
 * nss_eth_rx_stats_read()
 *	Read ETH_RX stats
 */
static ssize_t nss_eth_rx_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	int32_t i;

	/*
	 * max output lines = #stats + start tag line + end tag line + three blank lines
	 */
	uint32_t max_output_lines = (NSS_STATS_NODE_MAX + 2) + (NSS_ETH_RX_STATS_MAX + 3) + (NSS_ETH_RX_EXCEPTION_EVENT_MAX + 3) + 5;
	size_t size_al = NSS_STATS_MAX_STR_LENGTH * max_output_lines;
	size_t size_wr = 0;
	ssize_t bytes_read = 0;
	uint64_t *stats_shadow;

	char *lbuf = kzalloc(size_al, GFP_KERNEL);
	if (unlikely(lbuf == NULL)) {
		nss_warning("Could not allocate memory for local statistics buffer");
		return 0;
	}

	/*
	 * Note: The assumption here is that we do not have more than 64 stats
	 */
	stats_shadow = kzalloc(64 * 8, GFP_KERNEL);
	if (unlikely(stats_shadow == NULL)) {
		nss_warning("Could not allocate memory for local shadow buffer");
		kfree(lbuf);
		return 0;
	}

	size_wr = scnprintf(lbuf, size_al, "eth_rx stats start:\n\n");

	size_wr = nss_stats_fill_common_stats(NSS_ETH_RX_INTERFACE, lbuf, size_wr, size_al);

	/*
	 * eth_rx node stats
	 */
	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\neth_rx node stats:\n\n");
	spin_lock_bh(&nss_top_main.stats_lock);
	for (i = 0; (i < NSS_ETH_RX_STATS_MAX); i++) {
		stats_shadow[i] = nss_eth_rx_stats[i];
	}

	spin_unlock_bh(&nss_top_main.stats_lock);

	for (i = 0; (i < NSS_ETH_RX_STATS_MAX); i++) {
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr,
					"%s = %llu\n", nss_eth_rx_stats_str[i], stats_shadow[i]);
	}

	/*
	 * Exception stats
	 */
	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\neth_rx exception stats:\n\n");

	spin_lock_bh(&nss_top_main.stats_lock);
	for (i = 0; (i < NSS_ETH_RX_EXCEPTION_EVENT_MAX); i++) {
		stats_shadow[i] = nss_eth_rx_exception_stats[i];
	}

	spin_unlock_bh(&nss_top_main.stats_lock);

	for (i = 0; (i < NSS_ETH_RX_EXCEPTION_EVENT_MAX); i++) {
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr,
					"%s = %llu\n", nss_eth_rx_exception_stats_str[i], stats_shadow[i]);
	}

	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\neth_rx stats end\n\n");
	bytes_read = simple_read_from_buffer(ubuf, sz, ppos, lbuf, strlen(lbuf));
	kfree(lbuf);
	kfree(stats_shadow);

	return bytes_read;
}

/*
 * nss_eth_rx_stats_ops
 */
NSS_STATS_DECLARE_FILE_OPERATIONS(eth_rx)

/*
 * nss_eth_rx_stats_dentry_create()
 *	Create eth_rx statistics debug entry.
 */
void nss_eth_rx_stats_dentry_create(void)
{
	nss_stats_create_dentry("eth_rx", &nss_eth_rx_stats_ops);
}

/*
 * nss_eth_rx_metadata_stats_sync()
 *	Handle the syncing of ETH_RX node statistics.
 */
void nss_eth_rx_metadata_stats_sync(struct nss_ctx_instance *nss_ctx, struct nss_eth_rx_node_sync *nens)
{
	int32_t i;
	struct nss_top_instance *nss_top = nss_ctx->nss_top;

	spin_lock_bh(&nss_top->stats_lock);

	nss_top->stats_node[NSS_ETH_RX_INTERFACE][NSS_STATS_NODE_RX_PKTS] += nens->node_stats.rx_packets;
	nss_top->stats_node[NSS_ETH_RX_INTERFACE][NSS_STATS_NODE_RX_BYTES] += nens->node_stats.rx_bytes;
	nss_top->stats_node[NSS_ETH_RX_INTERFACE][NSS_STATS_NODE_TX_PKTS] += nens->node_stats.tx_packets;
	nss_top->stats_node[NSS_ETH_RX_INTERFACE][NSS_STATS_NODE_TX_BYTES] += nens->node_stats.tx_bytes;

	for (i = 0; i < NSS_MAX_NUM_PRI; i++) {
		nss_top->stats_node[NSS_ETH_RX_INTERFACE][NSS_STATS_NODE_RX_QUEUE_0_DROPPED + i] += nens->node_stats.rx_dropped[i];
	}

	nss_eth_rx_stats[NSS_ETH_RX_STATS_TOTAL_TICKS] += nens->total_ticks;
	nss_eth_rx_stats[NSS_ETH_RX_STATS_WORST_CASE_TICKS] += nens->worst_case_ticks;
	nss_eth_rx_stats[NSS_ETH_RX_STATS_ITERATIONS] += nens->iterations;

	for (i = 0; i < NSS_ETH_RX_EXCEPTION_EVENT_MAX; i++) {
		nss_eth_rx_exception_stats[i] += nens->exception_events[i];
	}

	spin_unlock_bh(&nss_top->stats_lock);
}
