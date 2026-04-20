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

#include <linux/atomic.h>
#include <linux/debugfs.h>
#include <linux/slab.h>
#include "ppe_priority.h"

/*
 * ppe_priority_stats_gen_str
 * 	PPE priority connection statistics
 */
static const char *ppe_priority_stats_str[] = {
	"v4_create_ppe_rule_priority",			/* v4 create priority rule request */
	"v4_create_ppe_rule_fail",			/* v4 create priority ppe rule addition failed */
	"v4_destroy_ppe_rule_priority",			/* v4 destroy priority rule request */
	"v4_destroy_ppe_rule_fail",			/* v4 destroy priority ppe rule addition failed */

	"v6_create_ppe_rule_priority",			/* v6 create priority rule request */
	"v6_create_ppe_rule_fail",			/* v6 create priority ppe rule addition failed */
	"v6_destroy_ppe_rule_priority",			/* v6 destroy priority rule request */
	"v6_destroy_ppe_rule_fail",			/* v6 destroy priority ppe rule addition failed */
};

/*
 * ppe_priority_stats_show()
 *	Read ppe priority connection statistics
 */
static int ppe_priority_stats_show(struct seq_file *m, void __attribute__((unused))*ptr)
{
	struct ppe_priority *g_priority = &gbl_ppe_priority;
	uint64_t *stats, *stats_shadow;
	uint32_t stats_size;
	int i;

	stats_size = sizeof(g_priority->stats);

	stats = kzalloc(stats_size, GFP_KERNEL);
	if (!stats) {
		ppe_priority_warn("Error in allocating gen stats\n");
		return -ENOMEM;
	}

	spin_lock_bh(&g_priority->lock);
	memcpy(stats, &g_priority->stats, sizeof(struct ppe_priority_stats));
	spin_unlock_bh(&g_priority->lock);

	seq_puts(m, "\nPPE stats:\n\n");
	stats_shadow = stats;
	for (i = 0; i < sizeof(struct ppe_priority_stats) / sizeof(uint64_t); i++) {
		seq_printf(m, "\t\t [%s]:  %llu\n", ppe_priority_stats_str[i], stats_shadow[i]);
	}

	spin_lock_bh(&g_priority->lock);
	memcpy(stats, &g_priority->stats, sizeof(g_priority->stats));
	spin_unlock_bh(&g_priority->lock);

	kfree(stats);
	return 0;
}

/*
 * ppe_priority_stats_general_open()
 *	PPE priority gen open callback API
 */
static int ppe_priority_stats_open(struct inode *inode, struct file *file)
{
	return single_open(file, ppe_priority_stats_show, inode->i_private);
}

/*
 * ppe_priority_stats_general_file_ops
 *	File operations for EDMA common stats
 */
const struct file_operations ppe_priority_stats_file_ops = {
	.open = ppe_priority_stats_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

/*
 * ppe_priority_stats_debugfs_init()
 *	Create PPE priority statistics debug entry.
 */
int ppe_priority_stats_debugfs_init(struct dentry *root)
{
	struct ppe_priority *g_prio = &gbl_ppe_priority;
	g_prio->dentry = debugfs_create_dir("ppe-priority", root);
	if (!g_prio->dentry) {
		ppe_priority_warn("%p: Unable to create debugfs stats directory in debugfs\n", g_prio);
		return -1;
	}

	if (!debugfs_create_file("stats", S_IRUGO, g_prio->dentry,
			NULL, &ppe_priority_stats_file_ops)) {
		ppe_priority_warn("%p: Unable to create common statistics file entry in debugfs\n", g_prio);
		goto debugfs_dir_failed;
	}

	return 0;

debugfs_dir_failed:
	debugfs_remove_recursive(g_prio->dentry);
	g_prio->dentry = NULL;
	return -1;
}

/*
 * ppe_priority_stats_debugfs_exit()
 *	PPE debugfs exit api
 */
void ppe_priority_stats_debugfs_exit(void)
{
	struct ppe_priority *g_prio = &gbl_ppe_priority;
	if (g_prio->dentry) {
		debugfs_remove_recursive(g_prio->dentry);
		g_prio->dentry = NULL;
	}
}
