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
#include "nss_stats.h"
#include "nss_gre_redir_mark.h"
#include "nss_gre_redir_mark_stats.h"

#define NSS_GRE_REDIR_MARK_STATS_STR_LEN 50
#define NSS_GRE_REDIR_MARK_STATS_LEN ((NSS_GRE_REDIR_MARK_STATS_MAX + 7 ) * NSS_GRE_REDIR_MARK_STATS_STR_LEN)
/*
 * nss_gre_redir_mark_stats_str
 *	GRE redir mark statistics string
 */
static int8_t *nss_gre_redir_mark_stats_str[NSS_GRE_REDIR_MARK_STATS_MAX] = {
	"TX Packets",
	"TX Bytes",
	"RX Packets",
	"RX Bytes",
	"RX Drops",
	"HLOS Magic Failed",
	"Tx Inv_dst_if Drops",
	"Tx Dst_if Enqueue",
	"Tx Dst_if Enqueue Drops",
	"Invalid Appid",
	"Headroom Unavailable",
	"Tx Completion Host Enqueue Success",
	"Tx Completion Host Enqueue Drops",
};

/*
 * nss_gre_redir_mark_stats_cpy()
 *	Fill the stats.
 */
static ssize_t nss_gre_redir_mark_stats_cpy(char *lbuf, int len, int i, struct nss_gre_redir_mark_stats *s)
{
	uint64_t tcnt = 0;

	switch (i) {
	case NSS_GRE_REDIR_MARK_STATS_TX_PKTS:
		tcnt = s->stats[NSS_GRE_REDIR_MARK_STATS_TX_PKTS];
		return scnprintf(lbuf, len, "Common node stats start:\n\n%s = %llu\n", nss_gre_redir_mark_stats_str[i], tcnt);
	case NSS_GRE_REDIR_MARK_STATS_TX_BYTES:
		tcnt = s->stats[NSS_GRE_REDIR_MARK_STATS_TX_BYTES];
		return scnprintf(lbuf, len, "%s = %llu\n", nss_gre_redir_mark_stats_str[i], tcnt);
	case NSS_GRE_REDIR_MARK_STATS_RX_PKTS:
		tcnt = s->stats[NSS_GRE_REDIR_MARK_STATS_RX_PKTS];
		return scnprintf(lbuf, len, "%s = %llu\n", nss_gre_redir_mark_stats_str[i], tcnt);
	case NSS_GRE_REDIR_MARK_STATS_RX_BYTES:
		tcnt = s->stats[NSS_GRE_REDIR_MARK_STATS_RX_BYTES];
		return scnprintf(lbuf, len, "%s = %llu\n", nss_gre_redir_mark_stats_str[i], tcnt);
	case NSS_GRE_REDIR_MARK_STATS_RX_DROPS:
		tcnt = s->stats[NSS_GRE_REDIR_MARK_STATS_RX_DROPS];
		return scnprintf(lbuf, len, "%s = %llu\nCommon node stats end.\n", nss_gre_redir_mark_stats_str[i], tcnt);
	case NSS_GRE_REDIR_MARK_STATS_HLOS_MAGIC_FAILED:
		tcnt = s->stats[NSS_GRE_REDIR_MARK_STATS_HLOS_MAGIC_FAILED];
		return scnprintf(lbuf, len, "Offload stats start:\n\n%s = %llu\n", nss_gre_redir_mark_stats_str[i], tcnt);
	case NSS_GRE_REDIR_MARK_STATS_INV_DST_IF_DROPS:
		tcnt = s->stats[NSS_GRE_REDIR_MARK_STATS_INV_DST_IF_DROPS];
		return scnprintf(lbuf, len, "%s = %llu\n", nss_gre_redir_mark_stats_str[i], tcnt);
	case NSS_GRE_REDIR_MARK_STATS_DST_IF_ENQUEUE:
		tcnt = s->stats[NSS_GRE_REDIR_MARK_STATS_DST_IF_ENQUEUE];
		return scnprintf(lbuf, len, "%s = %llu\n", nss_gre_redir_mark_stats_str[i], tcnt);
	case NSS_GRE_REDIR_MARK_STATS_DST_IF_ENQUEUE_DROPS:
		tcnt = s->stats[NSS_GRE_REDIR_MARK_STATS_DST_IF_ENQUEUE_DROPS];
		return scnprintf(lbuf, len, "%s = %llu\n", nss_gre_redir_mark_stats_str[i], tcnt);
	case NSS_GRE_REDIR_MARK_STATS_INV_APPID:
		tcnt = s->stats[NSS_GRE_REDIR_MARK_STATS_INV_APPID];
		return scnprintf(lbuf, len, "%s = %llu\n", nss_gre_redir_mark_stats_str[i], tcnt);
	case NSS_GRE_REDIR_MARK_STATS_HEADROOM_UNAVAILABLE:
		tcnt = s->stats[NSS_GRE_REDIR_MARK_STATS_HEADROOM_UNAVAILABLE];
		return scnprintf(lbuf, len, "%s = %llu\n", nss_gre_redir_mark_stats_str[i], tcnt);
	case NSS_GRE_REDIR_MARK_STATS_TX_COMPLETION_SUCCESS:
		tcnt = s->stats[NSS_GRE_REDIR_MARK_STATS_TX_COMPLETION_SUCCESS];
		return scnprintf(lbuf, len, "%s = %llu\n", nss_gre_redir_mark_stats_str[i], tcnt);
	case NSS_GRE_REDIR_MARK_STATS_TX_COMPLETION_DROPS:
		tcnt = s->stats[NSS_GRE_REDIR_MARK_STATS_TX_COMPLETION_DROPS];
		return scnprintf(lbuf, len, "%s = %llu\nOffload stats end.\n", nss_gre_redir_mark_stats_str[i], tcnt);
	default:
		nss_warning("Unknown stats type %d.\n", i);
		return 0;
	}
}

/*
 * nss_gre_redir_mark_stats_read()
 *	READ GRE redir mark stats.
 */
static ssize_t nss_gre_redir_mark_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	struct nss_gre_redir_mark_stats stats;
	size_t size_wr = 0;
	int start, end;
	ssize_t bytes_read = 0;
	bool isthere;
	size_t size_al = ((NSS_GRE_REDIR_MARK_STATS_MAX + 7 ) * NSS_GRE_REDIR_MARK_STATS_STR_LEN);

	char *lbuf = kzalloc(size_al, GFP_KERNEL);
	if (unlikely(!lbuf)) {
		nss_warning("Could not allocate memory for local statistics buffer");
		return 0;
	}

	/*
	 * If GRE redir mark does not exists, then (isthere) will be false.
	 */
	isthere = nss_gre_redir_mark_get_stats((void*)&stats);
	if (!isthere) {
		nss_warning("Could not get GRE redirect stats");
		kfree(lbuf);
		return 0;
	}

	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\nGRE redir mark stats\n");

	start = NSS_GRE_REDIR_MARK_STATS_TX_PKTS;
	end = NSS_GRE_REDIR_MARK_STATS_MAX;
	while (start < end) {
		size_wr += nss_gre_redir_mark_stats_cpy(lbuf + size_wr, size_al - size_wr, start, &stats);
		start++;
	}

	bytes_read = simple_read_from_buffer(ubuf, sz, ppos, lbuf, size_wr);

	kfree(lbuf);
	return bytes_read;
}

/*
 * nss_gre_redir_mark_stats_ops
 */
NSS_STATS_DECLARE_FILE_OPERATIONS(gre_redir_mark)

/*
 * nss_gre_redir_mark_stats_dentry_create()
 *	Create debugfs directory entry for stats.
 */
struct dentry *nss_gre_redir_mark_stats_dentry_create(void)
{
	struct dentry *gre_redir_mark;

	gre_redir_mark = debugfs_create_file("gre_redir_mark", 0400, nss_top_main.stats_dentry,
			&nss_top_main, &nss_gre_redir_mark_stats_ops);
	if (unlikely(!gre_redir_mark)) {
		nss_warning("Failed to create file entry qca-nss-drv/stats/gre_redir_mark/\n");
		return NULL;
	}

	return gre_redir_mark;
}
