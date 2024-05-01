/*
 **************************************************************************
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
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

#include "nss_core.h"
#include "nss_gre_redir_lag.h"
#include "nss_gre_redir_lag_ds_stats.h"

/*
 * nss_gre_redir_lag_ds_stats_str
 *	GRE REDIR LAG DS common statistics strings.
 */
static uint8_t *nss_gre_redir_lag_ds_stats_str[NSS_GRE_REDIR_LAG_DS_STATS_MAX] = {
	"rx_packets",
	"rx_bytes",
	"tx_packets",
	"tx_bytes",
	"rx_queue_0_dropped",
	"rx_queue_1_dropped",
	"rx_queue_2_dropped",
	"rx_queue_3_dropped",
	"dst_invalid",
	"exception_packets"
};

/*
 * nss_gre_redir_lag_ds_tunnel_stats()
 *	Make a row for GRE_REDIR LAG DS stats.
 */
static ssize_t nss_gre_redir_lag_ds_cmn_stats_read_entry(char *line, int len, int type, struct nss_gre_redir_lag_ds_tun_stats *s)
{
	uint64_t tcnt = 0;

	switch (type) {
	case NSS_STATS_NODE_RX_PKTS:
		tcnt = s->rx_packets;
		return snprintf(line, len, "Common node stats start:\n\n%s = %llu\n", nss_gre_redir_lag_ds_stats_str[type], tcnt);
	case NSS_STATS_NODE_RX_BYTES:
		tcnt = s->rx_bytes;
		return snprintf(line, len, "%s = %llu\n", nss_gre_redir_lag_ds_stats_str[type], tcnt);
	case NSS_STATS_NODE_TX_PKTS:
		tcnt = s->tx_packets;
		return snprintf(line, len, "%s = %llu\n", nss_gre_redir_lag_ds_stats_str[type], tcnt);
	case NSS_STATS_NODE_TX_BYTES:
		tcnt = s->tx_bytes;
		return snprintf(line, len, "%s = %llu\n", nss_gre_redir_lag_ds_stats_str[type], tcnt);
	case NSS_STATS_NODE_RX_QUEUE_0_DROPPED:
		tcnt = s->rx_dropped[0];
		return snprintf(line, len, "%s = %llu\n", nss_gre_redir_lag_ds_stats_str[type], tcnt);
#if (NSS_MAX_NUM_PRI > 1)
	case NSS_STATS_NODE_RX_QUEUE_1_DROPPED:
		tcnt = s->rx_dropped[1];
		return snprintf(line, len, "%s = %llu\n", nss_gre_redir_lag_ds_stats_str[type], tcnt);
	case NSS_STATS_NODE_RX_QUEUE_2_DROPPED:
		tcnt = s->rx_dropped[2];
		return snprintf(line, len, "%s = %llu\n", nss_gre_redir_lag_ds_stats_str[type], tcnt);
	case NSS_STATS_NODE_RX_QUEUE_3_DROPPED:
		tcnt = s->rx_dropped[3];
		return snprintf(line, len, "%s = %llu\n", nss_gre_redir_lag_ds_stats_str[type], tcnt);
#endif
	case NSS_GRE_REDIR_LAG_DS_STATS_DST_INVALID:
		tcnt = s->dst_invalid;
		return snprintf(line, len, "%s = %llu\n", nss_gre_redir_lag_ds_stats_str[type], tcnt);
	case NSS_GRE_REDIR_LAG_DS_STATS_EXCEPTION_PKT:
		tcnt = s->exception_cnt;
		return snprintf(line, len, "%s = %llu\n", nss_gre_redir_lag_ds_stats_str[type], tcnt);

	default:
		return 0;
	}
}

/*
 * nss_gre_redir_lag_ds_stats_read()
 *	Read gre_redir_lag_ds tunnel stats.
 */
static ssize_t nss_gre_redir_lag_ds_cmn_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	ssize_t bytes_read = 0;
	struct nss_stats_data *data = fp->private_data;
	struct nss_gre_redir_lag_ds_tun_stats stats;
	size_t bytes;
	char line[80];
	int start;

	while (data->index < NSS_GRE_REDIR_LAG_MAX_NODE) {
		if (nss_gre_redir_lag_ds_get_cmn_stats(&stats, data->index)) {
			break;
		}

		data->index++;
	}

	if (data->index == NSS_GRE_REDIR_LAG_MAX_NODE) {
		return 0;
	}

	bytes = snprintf(line, sizeof(line), "\nTunnel stats for \n");
	if (copy_to_user(ubuf, line, bytes) != 0) {
		return -EFAULT;
	}

	bytes_read += bytes;
	start = NSS_STATS_NODE_RX_PKTS;
	while (bytes_read < sz && start < NSS_GRE_REDIR_LAG_DS_STATS_MAX) {
		bytes = nss_gre_redir_lag_ds_cmn_stats_read_entry(line, sizeof(line), start, &stats);

		if ((bytes_read + bytes) > sz)
			break;

		if (copy_to_user(ubuf + bytes_read, line, bytes) != 0) {
			return -EFAULT;
		}

		bytes_read += bytes;
		start++;
	}

	data->index++;
	return bytes_read;
}

/*
 * nss_gre_redir_lag_ds_stats_ops
 */
NSS_STATS_DECLARE_FILE_OPERATIONS(gre_redir_lag_ds_cmn)

/*
 * nss_gre_redir_lag_ds_stats_dentry_create()
 *	Create debugfs directory entry.
 */
struct dentry *nss_gre_redir_lag_ds_stats_dentry_create(void)
{
	struct dentry *gre_redir;
	struct dentry *cmn_stats;

	gre_redir = nss_gre_redir_get_dentry();
	if (unlikely(!gre_redir)) {
		nss_warning("Failed to retrieve directory entry qca-nss-drv/stats/gre_redir/\n");
		return NULL;
	}

	cmn_stats = debugfs_create_file("lag_ds_cmn_stats", 0400, gre_redir,
			&nss_top_main, &nss_gre_redir_lag_ds_cmn_stats_ops);
	if (unlikely(!cmn_stats)) {
		nss_warning("Failed to create qca-nss-drv/stats/gre_redir/lag_ds_cmn_stats file\n");
		return NULL;
	}

	return cmn_stats;
}
