/*
 **************************************************************************
 * Copyright (c) 2017-2019, The Linux Foundation. All rights reserved.
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
#include "nss_stats.h"
#include "nss_gre_redir.h"
#include "nss_gre_redir_stats.h"

/*
 * nss_gre_redir_stats_str
 *	GRE REDIR statistics string
 */
static int8_t *nss_gre_redir_stats_str[NSS_GRE_REDIR_STATS_MAX] = {
	"TX Packets",
	"TX Bytes",
	"TX Drops",
	"RX Packets",
	"RX Bytes",
	"RX Drops",
	"TX Sjack Packets",
	"RX Sjack packets",
	"TX Offload Packets",
	"RX Offload Packets",
	"US exception RX Packets",
	"US exception TX Packets",
	"DS exception RX Packets",
	"DS exception TX Packets",
	"Encap SG alloc drop",
	"Decap fail drop",
	"Decap split drop",
	"Split SG alloc fail",
	"Split linear copy fail",
	"Split not enough tailroom",
	"Exception ds invalid dst",
	"Decap eapol frames",
	"Exception ds invalid appid",
	"Headroom Unavailable",
	"Exception ds Tx completion Success",
	"Exception ds Tx completion drop"
};

/*
 * nss_gre_redir_stats()
 *	Make a row for GRE_REDIR stats.
 */
static ssize_t nss_gre_redir_stats(char *line, int len, int i, struct nss_gre_redir_tunnel_stats *s)
{
	char name[40];
	uint64_t tcnt = 0;
	int j = 0;

	switch (i) {
	case NSS_GRE_REDIR_STATS_TX_PKTS:
		tcnt = s->node_stats.tx_packets;
		return snprintf(line, len, "Common node stats start:\n\n%s = %llu\n", nss_gre_redir_stats_str[i], tcnt);
	case NSS_GRE_REDIR_STATS_TX_BYTES:
		tcnt = s->node_stats.tx_bytes;
		return snprintf(line, len, "%s = %llu\n", nss_gre_redir_stats_str[i], tcnt);
	case NSS_GRE_REDIR_STATS_TX_DROPS:
		tcnt = s->tx_dropped;
		return snprintf(line, len, "%s = %llu\n", nss_gre_redir_stats_str[i], tcnt);
	case NSS_GRE_REDIR_STATS_RX_PKTS:
		tcnt = s->node_stats.rx_packets;
		return snprintf(line, len, "%s = %llu\n", nss_gre_redir_stats_str[i], tcnt);
	case NSS_GRE_REDIR_STATS_RX_BYTES:
		tcnt = s->node_stats.rx_bytes;
		return snprintf(line, len, "%s = %llu\n", nss_gre_redir_stats_str[i], tcnt);
	case NSS_GRE_REDIR_STATS_RX_DROPS:
		tcnt = s->node_stats.rx_dropped[0];
		return snprintf(line, len, "%s = %llu\nCommon node stats end.\n", nss_gre_redir_stats_str[i], tcnt);
	case NSS_GRE_REDIR_STATS_SJACK_TX_PKTS:
		tcnt = s->sjack_tx_packets;
		return snprintf(line, len, "Offload stats start:\n\n%s = %llu\n", nss_gre_redir_stats_str[i], tcnt);
	case NSS_GRE_REDIR_STATS_OFFLOAD_TX_PKTS:
		for (j = 0; j < NSS_GRE_REDIR_MAX_RADIO; j++) {
			scnprintf(name, sizeof(name), "TX offload pkts for radio %d", j);
			tcnt += snprintf(line + tcnt, len - tcnt, "%s = %llu\n", name, s->offl_tx_pkts[j]);
		}
		return tcnt;
	case NSS_GRE_REDIR_STATS_SJACK_RX_PKTS:
		tcnt = s->sjack_rx_packets;
		return snprintf(line, len, "%s = %llu\n", nss_gre_redir_stats_str[i], tcnt);
	case NSS_GRE_REDIR_STATS_OFFLOAD_RX_PKTS:
		for (j = 0; j < NSS_GRE_REDIR_MAX_RADIO; j++) {
			scnprintf(name, sizeof(name), "RX offload pkts for radio %d", j);
			tcnt += snprintf(line + tcnt, len - tcnt, "%s = %llu\n", name, s->offl_rx_pkts[j]);
		}
		return tcnt;
	case NSS_GRE_REDIR_STATS_EXCEPTION_US_RX_PKTS:
		tcnt = s->exception_us_rx;
		return snprintf(line, len, "%s = %llu\n", nss_gre_redir_stats_str[i], tcnt);
	case NSS_GRE_REDIR_STATS_EXCEPTION_US_TX_PKTS:
		tcnt = s->exception_us_tx;
		return snprintf(line, len, "%s = %llu\n", nss_gre_redir_stats_str[i], tcnt);
	case NSS_GRE_REDIR_STATS_EXCEPTION_DS_RX_PKTS:
		tcnt = s->exception_ds_rx;
		return snprintf(line, len, "%s = %llu\n", nss_gre_redir_stats_str[i], tcnt);
	case NSS_GRE_REDIR_STATS_EXCEPTION_DS_TX_PKTS:
		tcnt = s->exception_ds_tx;
		return snprintf(line, len, "%s = %llu\n", nss_gre_redir_stats_str[i], tcnt);
	case NSS_GRE_REDIR_STATS_ENCAP_SG_ALLOC_DROP:
		tcnt = s->encap_sg_alloc_drop;
		return snprintf(line, len, "%s = %llu\n", nss_gre_redir_stats_str[i], tcnt);
	case NSS_GRE_REDIR_STATS_DECAP_FAIL_DROP:
		tcnt = s->decap_fail_drop;
		return snprintf(line, len, "%s = %llu\n", nss_gre_redir_stats_str[i], tcnt);
	case NSS_GRE_REDIR_STATS_DECAP_SPLIT_DROP:
		tcnt = s->decap_split_drop;
		return snprintf(line, len, "%s = %llu\n", nss_gre_redir_stats_str[i], tcnt);
	case NSS_GRE_REDIR_STATS_SPLIT_SG_ALLOC_FAIL:
		tcnt = s->split_sg_alloc_fail;
		return snprintf(line, len, "%s = %llu\n", nss_gre_redir_stats_str[i], tcnt);
	case NSS_GRE_REDIR_STATS_SPLIT_LINEAR_COPY_FAIL:
		tcnt = s->split_linear_copy_fail;
		return snprintf(line, len, "%s = %llu\n", nss_gre_redir_stats_str[i], tcnt);
	case NSS_GRE_REDIR_STATS_SPLIT_NOT_ENOUGH_TAILROOM:
		tcnt = s->split_not_enough_tailroom;
		return snprintf(line, len, "%s = %llu\n", nss_gre_redir_stats_str[i], tcnt);
	case NSS_GRE_REDIR_STATS_EXCEPTION_DS_INVALID_DST_DROP:
		tcnt = s->exception_ds_invalid_dst_drop;
		return snprintf(line, len, "%s = %llu\n", nss_gre_redir_stats_str[i], tcnt);
	case NSS_GRE_REDIR_STATS_DECAP_EAPOL_FRAMES:
		tcnt = s->decap_eapol_frames;
		return snprintf(line, len, "%s = %llu\n", nss_gre_redir_stats_str[i], tcnt);
	case NSS_GRE_REDIR_STATS_EXCEPTION_DS_INV_APPID:
		tcnt = s->exception_ds_inv_appid;
		return snprintf(line, len, "%s = %llu\n", nss_gre_redir_stats_str[i], tcnt);
	case NSS_GRE_REDIR_STATS_HEADROOM_UNAVAILABLE:
		tcnt = s->headroom_unavail;
		return snprintf(line, len, "%s = %llu\n", nss_gre_redir_stats_str[i], tcnt);
	case NSS_GRE_REDIR_STATS_TX_COMPLETION_SUCCESS:
		tcnt = s->tx_completion_success;
		return snprintf(line, len, "%s = %llu\n", nss_gre_redir_stats_str[i], tcnt);
	case NSS_GRE_REDIR_STATS_TX_COMPLETION_DROP:
		tcnt = s->tx_completion_drop;
		return snprintf(line, len, "%s = %llu\nOffload stats end.\n", nss_gre_redir_stats_str[i], tcnt);
	default:
		nss_warning("Unknown stats type %d.\n", i);
		return 0;
	}
}

/*
 * nss_gre_redir_stats_read()
 *	READ gre_redir tunnel stats.
 */
static ssize_t nss_gre_redir_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	struct nss_stats_data *data = fp->private_data;
	ssize_t bytes_read = 0;
	struct nss_gre_redir_tunnel_stats stats;
	size_t bytes;
	char line[80 * NSS_GRE_REDIR_MAX_RADIO];
	int start, end;
	int index = 0;

	if (data) {
		index = data->index;
	}

	/*
	 * If we are done accomodating all the GRE_REDIR tunnels.
	 */
	if (index >= NSS_GRE_REDIR_MAX_INTERFACES) {
		return 0;
	}

	for (; index < NSS_GRE_REDIR_MAX_INTERFACES; index++) {
		bool isthere;

		/*
		 * If gre_redir tunnel does not exists, then isthere will be false.
		 */
		isthere = nss_gre_redir_get_stats(index, &stats);
		if (!isthere) {
			continue;
		}

		bytes = snprintf(line, sizeof(line), "\nTunnel stats for %s\n", stats.dev->name);
		if ((bytes_read + bytes) > sz) {
			break;
		}

		if (copy_to_user(ubuf + bytes_read, line, bytes) != 0) {
			bytes_read = -EFAULT;
			goto fail;
		}
		bytes_read += bytes;
		start = NSS_GRE_REDIR_STATS_TX_PKTS;
		end = NSS_GRE_REDIR_STATS_MAX;
		while (bytes_read < sz && start < end) {
			bytes = nss_gre_redir_stats(line, sizeof(line), start, &stats);

			if ((bytes_read + bytes) > sz)
				break;

			if (copy_to_user(ubuf + bytes_read, line, bytes) != 0) {
				bytes_read = -EFAULT;
				goto fail;
			}

			bytes_read += bytes;
			start++;
		}
	}

	if (bytes_read > 0) {
		*ppos = bytes_read;
	}

	if (data) {
		data->index = index;
	}

fail:
	return bytes_read;
}

/*
 * nss_gre_redir_stats_ops
 */
NSS_STATS_DECLARE_FILE_OPERATIONS(gre_redir)

/*
 * nss_gre_redir_stats_dentry_create()
 *	Create debugfs directory entry for stats.
 */
struct dentry *nss_gre_redir_stats_dentry_create(void)
{
	struct dentry *gre_redir;
	struct dentry *tun_stats;

	gre_redir = debugfs_create_dir("gre_redir", nss_top_main.stats_dentry);
	if (unlikely(!gre_redir)) {
		nss_warning("Failed to create directory entry qca-nss-drv/stats/gre_redir/\n");
		return NULL;
	}

	tun_stats = debugfs_create_file("tun_stats", 0400, gre_redir,
			&nss_top_main, &nss_gre_redir_stats_ops);
	if (unlikely(!tun_stats)) {
		debugfs_remove_recursive(gre_redir);
		nss_warning("Failed to create file entry qca-nss-drv/stats/gre_redir/tun_stats\n");
		return NULL;
	}

	return gre_redir;
}
