/*
 **************************************************************************
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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
 **************************************************************************
 */

#include <linux/types.h>
#include <nss_api_if.h>
#include <linux/debugfs.h>
#include "nss_match_db.h"
#include "nss_match_priv.h"

/*
 * nss_match_stats_str
 *	MATCH statistics strings for nss MATCH stats
 */
static int8_t *nss_match_stats_str[NSS_MATCH_STATS_INSTANCE_MAX] = {
	"rx_packets",
	"rx_bytes",
	"tx_packets",
	"tx_bytes",
	"rx_queue_0_drop",
	"rx_queue_1_drop",
	"rx_queue_2_drop",
	"rx_queue_3_drop",
};

/*
 * nss_match_stats_read()
 *	Read MATCH statistics
 */
static ssize_t nss_match_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	int instance_count = nss_match_table_count_get();
	uint32_t max_output_lines = 2 /* header & footer for instance stats */
					+ instance_count * (NSS_MATCH_STATS_INSTANCE_MAX + 2) /*instance stats */
					+ 2;
	size_t size_al = NSS_STATS_MAX_STR_LENGTH * max_output_lines;
	size_t size_wr = 0;
	int id, i;
	ssize_t bytes_read = 0;
	struct nss_match_stats match_stats = {0};
	char *lbuf = kzalloc(size_al, GFP_KERNEL);
	if (unlikely(lbuf == NULL)) {
		nss_match_warn("Could not allocate memory for local statistics buffer\n");
		return 0;
	}

	/*
	 * Session stats
	 */
	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\nmatch instance stats start:\n\n");
	for (id = 1; id <= NSS_MATCH_INSTANCE_MAX; id++) {
		if (!nss_match_db_table_validate(id)) {
			continue;
		}

		if (!nss_match_db_stats_get(id, &match_stats)) {
			nss_match_warn("Could not read stats for table_id = %d\n", id);
			continue;
		}

		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "Table_id= %d\n\n", id);

		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "if_num of match instance = %d\n\n", nss_match_get_ifnum_by_table_id(id));

		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "%s = %llu\n", nss_match_stats_str[NSS_MATCH_STATS_RX_PACKETS],
					match_stats.pstats.rx_packets);
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "%s = %llu\n", nss_match_stats_str[NSS_MATCH_STATS_RX_BYTES],
					match_stats.pstats.rx_bytes);
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "%s = %llu\n", nss_match_stats_str[NSS_MATCH_STATS_TX_PACKETS],
					match_stats.pstats.tx_packets);
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "%s = %llu\n", nss_match_stats_str[NSS_MATCH_STATS_TX_BYTES],
					match_stats.pstats.tx_bytes);
		for (i = 0; i < NSS_MAX_NUM_PRI ; ++i) {
			size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "%s = %llu\n",
					nss_match_stats_str[i + NSS_MATCH_STATS_RX_QUEUE_0_DROP],
					match_stats.pstats.rx_dropped[i]);
		}

		for (i = 0; i < NSS_MATCH_INSTANCE_RULE_MAX; ++i) {
			if (match_stats.hit_count[i] <= 0) {
				continue;
			}
			size_wr += scnprintf(lbuf + size_wr, size_al - size_wr,
					"Hit count of rule ID %d = %llu\n", i+1,
					match_stats.hit_count[i]);
		}

		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n");
	}

	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\nMatch instance stats end\n");
	bytes_read = simple_read_from_buffer(ubuf, sz, ppos, lbuf, size_wr);

	kfree(lbuf);
	return bytes_read;
}

/*
 * nss_match_stats_table_read()
 *	Read match table entry
 */
static ssize_t nss_match_stats_table_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	/*
	 * TODO: Optimize buffer size needed for table information.
	 */
	uint32_t max_output_lines = 2 + NSS_MATCH_INSTANCE_MAX * 2		/* for 2 maskset */
		+ NSS_MATCH_INSTANCE_MAX * NSS_MATCH_INSTANCE_RULE_MAX *(9)	/* for:  4 rule fields + 1 rule_id + 3 action fields + 1 hit count*/
		+ 2;
	int i;
	int if_num;
	size_t size_wr = 0, buflen = 0;
	size_t size_al = NSS_STATS_MAX_STR_LENGTH * max_output_lines;
	ssize_t bytes_read = 0;
	char *lbuf = kzalloc(size_al, GFP_KERNEL);
	char *bufp;
	if (unlikely(lbuf == NULL)) {
		nss_match_warn("%px: Could not allocate memory for local statistics buffer", fp);
		return 0;
	}

	for (i = 1; i <= NSS_MATCH_INSTANCE_MAX; ++i) {
		if_num = nss_match_get_ifnum_by_table_id(i);
		if (if_num < 0) {
			continue;
		}
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\nTable_id =%d\n", i);
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "Match if_num = %d\n\n", if_num);
		bufp = lbuf + size_wr;
		buflen = size_al - size_wr;
		size_wr += nss_match_db_table_read(i, buflen, bufp);

	}

	bytes_read = simple_read_from_buffer(ubuf, sz, ppos, lbuf, size_wr);
	kfree(lbuf);
	return bytes_read;
}

/*
 * nss_match_stats_show_table_ops
 */
static const struct file_operations nss_match_stats_show_table_ops = {
	.read = nss_match_stats_table_read,
};

/*
 * nss_match_stats_ops
 */
static const struct file_operations nss_match_stats_ops = {
	.read = nss_match_stats_read,
};

/*
 * nss_match_stats_debugfs_create()
 *	Create MATCH node statistics debug entry.
 */
bool nss_match_stats_debugfs_create(struct dentry *match_config)
{
	if (!debugfs_create_file("showtable", 0400, match_config, NULL, &nss_match_stats_show_table_ops)) {
		nss_match_warn("Cannot create match display dentry file");
		debugfs_remove_recursive(match_config);
		return false;
	}

	if (!debugfs_create_file("stats", 0400, match_config, NULL, &nss_match_stats_ops)) {
		nss_match_warn("Cannot create MATCH dentry file");
		return false;
	}

	return true;
}
