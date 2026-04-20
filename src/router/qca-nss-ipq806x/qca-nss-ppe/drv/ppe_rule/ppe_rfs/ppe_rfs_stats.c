/*
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <linux/atomic.h>
#include <linux/debugfs.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_net.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>
#include <linux/vmalloc.h>
#include <linux/debugfs.h>
#include "ppe_rfs.h"

/*
 * ppe_rfs_stats_gen_str
 * 	PPE RFS connection statistics
 */
static const char *ppe_rfs_stats_str[] = {
	"v4_create_ppe_rule_rfs",			/* v4 create rfs rule request */
	"v4_create_mem_fail",				/* v4 create rfs memory allocation failed */
	"v4_create_flow_interface_fail",		/* v4 create rfs flow interface invalid */
	"v4_create_return_interface_fail",		/* v4 create rfs return interface invalid */
	"v4_create_flow_top_interface_fail",		/* v4 create rfs flow top interface invalid */
	"v4_create_return_top_interface_fail",		/* v4 create rfs return top interface invalid */
	"v4_create_rfs_not_enabled",			/* v4 create rfs not enabled on both tx and rx interface */
	"v4_create_ppe_rule_fail",			/* v4 create rfs ppe rule addition failed */
	"v4_create_rfs_direction_check_fail",		/* v4 create rfs enabled on both tx and rx interface */
	"v4_destroy_ppe_rule_rfs",			/* v4 destroy rfs rule request */
	"v4_destroy_mem_fail",				/* v4 destroy rfs memory allocation failed */
	"v4_destroy_rfs_not_enabled",			/* v4 destroy rfs not enabled */
	"v4_destroy_ppe_rule_fail",			/* v4 destroy rfs ppe rule addition failed */
	"v4_destroy_rfs_direction_check_fail",			/* v4 destroy rfs enabled on both tx and rx interface */

	"v6_create_ppe_rule_rfs",			/* v6 create rfs rule request */
	"v6_create_mem_fail",				/* v6 create rfs memory allocation failed */
	"v6_create_flow_interface_fail",		/* v6 create rfs flow interface invalid */
	"v6_create_return_interface_fail",		/* v6 create rfs return interface invalid */
	"v6_create_flow_top_interface_fail",		/* v6 create rfs flow top interface invalid */
	"v6_create_return_top_interface_fail",		/* v6 create rfs return top interface invalid */
	"v6_create_rfs_not_enabled",			/* v6 create rfs not enabled on both tx and rx interface */
	"v6_create_ppe_rule_fail",			/* v6 create rfs ppe rule addition failed */
	"v6_create_rfs_direction_check_fail",			/* v6 destroy rfs enabled on both tx and rx interface */
	"v6_destroy_ppe_rule_rfs",			/* v6 destroy rfs rule request */
	"v6_destroy_mem_fail",				/* v6 destroy rfs memory allocation failed */
	"v6_destroy_rfs_not_enabled",			/* v6 destroy rfs not enabled */
	"v6_destroy_ppe_rule_fail",			/* v6 destroy rfs ppe rule addition failed */
	"v6_destroy_rfs_direction_check_fail",			/* v6 destroy rfs enabled on both tx and rx interface */
};

/*
 * ppe_rfs_stats_show()
 *	Read ppe rfs connection statistics
 */
static int ppe_rfs_stats_show(struct seq_file *m, void __attribute__((unused))*ptr)
{
	struct ppe_rfs *g_rfs = &gbl_ppe_rfs;
	uint64_t *stats, *stats_shadow;
	uint32_t stats_size;
	int i;

	stats_size = sizeof(g_rfs->stats);

	stats = kzalloc(stats_size, GFP_KERNEL);
	if (!stats) {
		ppe_rfs_warn("Error in allocating gen stats\n");
		return -ENOMEM;
	}

	spin_lock_bh(&g_rfs->lock);
	memcpy(stats, &g_rfs->stats, sizeof(struct ppe_rfs_stats));
	spin_unlock_bh(&g_rfs->lock);

	seq_puts(m, "\nPPE stats:\n\n");
	stats_shadow = stats;
	for (i = 0; i < sizeof(struct ppe_rfs_stats) / sizeof(uint64_t); i++) {
		seq_printf(m, "\t\t [%s]:  %llu\n", ppe_rfs_stats_str[i], stats_shadow[i]);
	}

	spin_lock_bh(&g_rfs->lock);
	memcpy(stats, &g_rfs->stats, sizeof(g_rfs->stats));
	spin_unlock_bh(&g_rfs->lock);

	kfree(stats);
	return 0;
}

/*
 * ppe_rfs_stats_general_open()
 *	PPE rfs gen open callback API
 */
static int ppe_rfs_stats_open(struct inode *inode, struct file *file)
{
	return single_open(file, ppe_rfs_stats_show, inode->i_private);
}

/*
 * ppe_rfs_stats_general_file_ops
 *	File operations for EDMA common stats
 */
const struct file_operations ppe_rfs_stats_file_ops = {
	.open = ppe_rfs_stats_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

/*
 * ppe_rfs_stats_debugfs_init()
 *	Create PPE statistics debug entry.
 */
int ppe_rfs_stats_debugfs_init(struct dentry *root)
{
	struct ppe_rfs *g_rfs = &gbl_ppe_rfs;
	g_rfs->dentry = debugfs_create_dir("ppe-rfs", root);
	if (!g_rfs->dentry) {
		ppe_rfs_warn("%p: Unable to create debugfs stats directory in debugfs\n", g_rfs);
		return -1;
	}

	if (!debugfs_create_file("stats", S_IRUGO, g_rfs->dentry,
			NULL, &ppe_rfs_stats_file_ops)) {
		ppe_rfs_warn("%p: Unable to create common statistics file entry in debugfs\n", g_rfs);
		goto debugfs_dir_failed;
	}

	return 0;

debugfs_dir_failed:
	debugfs_remove_recursive(g_rfs->dentry);
	g_rfs->dentry = NULL;
	return -1;
}

/*
 * ppe_rfs_stats_debugfs_exit()
 *	PPE debugfs exit api
 */
void ppe_rfs_stats_debugfs_exit(void)
{
	struct ppe_rfs *g_rfs = &gbl_ppe_rfs;
	if (g_rfs->dentry) {
		debugfs_remove_recursive(g_rfs->dentry);
		g_rfs->dentry = NULL;
	}
}
