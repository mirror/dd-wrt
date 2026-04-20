/*
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <linux/debugfs.h>
#include "ppe_ds_stats.h"
#include <ppe_drv.h>

static struct dentry *dbgfs;
struct ppe_ds_stats ppe_ds_node_stats[PPE_DS_MAX_NODE];

/*
 * ppe_ds_node_stats_show()
 *	Show the stats of per node
 */
static int ppe_ds_node_stats_show(struct seq_file *m,
		void __attribute__((unused))*ptr)
{
	int i = 0;

	for (i = 0; i < PPE_DS_MAX_NODE; i++) {
		seq_printf(m, "Node %d\n", i);
		seq_printf(m, "Tx pkts %llu\n", atomic64_read(&ppe_ds_node_stats[i].tx_pkts));
		seq_printf(m, "Rx pkts %llu\n\n", atomic64_read(&ppe_ds_node_stats[i].rx_pkts));
	}
	return 0;
}

/*
 * ppe_ds_node_stats_open()
 *	PPE DS STATS open callback API
 */
static int ppe_ds_node_stats_open(struct inode *inode, struct file *file)
{
	return single_open(file, ppe_ds_node_stats_show, inode->i_private);
}

/*
 * ppe_ds_node_stats_general_file_ops
 *	File operations for DS Node stats
 */
const struct file_operations ppe_ds_node_stats_file_ops = {
	.open = ppe_ds_node_stats_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

/*
 * ppe_ds_node_stats_debugfs_exit()
 *	PPE debugfs exit api
 */
void ppe_ds_node_stats_debugfs_exit(void)
{
	debugfs_remove_recursive(dbgfs);
	dbgfs = NULL;
}

/*
 * ppe_ds_node_stats_debugfs_init
 *	Create PPE statistics debug entry.
 */
int ppe_ds_node_stats_debugfs_init(void)
{
	struct dentry *root = ppe_drv_get_dentry();

	if (!root) {
		ppe_ds_warn("Unable to get root stats directory\n");
		return -1;
	}

	dbgfs = debugfs_create_dir("ppe_ds", root);
	if (!dbgfs) {
		ppe_ds_warn("%p: Unable to create debugfs stats directory in debugfs\n", dbgfs);
		return -1;
	}

	if (!debugfs_create_file("ppe_ds_node_stats", S_IRUGO, dbgfs, NULL, &ppe_ds_node_stats_file_ops)) {
		ppe_ds_warn("%p: Unable to create common statistics file entry in debugfs\n", dbgfs);
		goto debugfs_dir_failed;
	}

	return 0;

debugfs_dir_failed:
	debugfs_remove_recursive(dbgfs);
	dbgfs = NULL;
	return -1;
}
