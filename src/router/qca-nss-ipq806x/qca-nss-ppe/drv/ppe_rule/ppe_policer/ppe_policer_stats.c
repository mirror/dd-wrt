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
#include <linux/of_irq.h>
#include "ppe_policer.h"

/*
 * ppe_policer_stats_str
 *	PPE policer statistics
 */
static const char *ppe_policer_stats_str[] = {
	"policer_port_rule_id_invalid",				/* Port Policer create rule success */
	"policer_port_rule_not_found",				/* Port Policer create rule success */
	"policer_acl_rule_id_invalid",				/* Port Policer create rule success */
	"policer_acl_rule_not_found",				/* Port Policer create rule success */
	"policer_destroy_fail_invalid_id",
	"policer_acl_already_exists",				/* ACL Policer already exits */
	"policer_acl_create_fail_oom",				/* Port Policer destroy rule success */
	"policer_acl_create_fail_rule_table_full",		/* Port Policer destroy rule fail */
	"policer_policer_acl_create_req",			/* acl Policer destroy rule success */
	"policer_port_create_fail_oom",				/* Port Policer destroy rule success */
	"policer_port_create_fail_rule_table_full",		/* Port Policer destroy rule fail */
	"policer_port_create_req",			/* acl Policer destroy rule success */
	"policer_create_port_policer_success",			/* Port policer creation successful */
	"policer_create_port_policer_failed",			/* Port policer creation failure */
	"policer_destroy_port_policer_success",			/* Port policer destroy successful */
	"policer_destroy_port_policer_failed",			/* Port policer destroy failure */
	"policer_create_acl_policer_success",			/* Acl policer creation successful */
	"policer_create_acl_policer_failed",			/* Acl policer creation failure */
	"policer_destroy_acl_policer_success",			/* Acl policer destroy successful */
	"policer_destroy_acl_policer_failed",			/* Acl policer destroy failure */
	"policer_v4_create_ppe_rule_flow_policer",		/* Policer create rule request */
	"policer_v4_create_ppe_rule_fail",			/* Policer create rule fail */
	"policer_v4_destroy_ppe_rule_fail",			/* Policer create rule fail */
	"policer_v6_create_ppe_rule_flow_policer",		/* Policer create rule request */
	"policer_v6_create_ppe_rule_fail",			/* Policer create rule fail */
	"policer_v6_destroy_ppe_rule_fail",			/* Policer create rule fail */
};

/*
 * ppe_policer_stats_show()
 *	Read ppe policer connection statistics
 */
static int ppe_policer_stats_show(struct seq_file *m, void __attribute__((unused))*ptr)
{
	struct ppe_policer_base *g_pol = &gbl_ppe_policer;
	uint64_t *stats, *stats_shadow;
	uint32_t stats_size;
	int i;

	stats_size = sizeof(g_pol->stats);

	stats = kzalloc(stats_size, GFP_KERNEL);
	if (!stats) {
		ppe_policer_warn("Error in allocating gen stats\n");
		return -ENOMEM;
	}

	spin_lock_bh(&g_pol->lock);
	memcpy(stats, &g_pol->stats, sizeof(struct ppe_policer_stats));
	spin_unlock_bh(&g_pol->lock);

	seq_puts(m, "\nPPE stats:\n\n");
	stats_shadow = stats;
	for (i = 0; i < sizeof(struct ppe_policer_stats) / sizeof(uint64_t); i++) {
		seq_printf(m, "\t\t [%s]:  %llu\n", ppe_policer_stats_str[i], stats_shadow[i]);
	}

	kfree(stats);
	return 0;
}

/*
 * ppe_policer_stats_general_open()
 *	PPE policer gen open callback API
 */
static int ppe_policer_stats_open(struct inode *inode, struct file *file)
{
	return single_open(file, ppe_policer_stats_show, inode->i_private);
}

/*
 * ppe_policer_stats_general_file_ops
 *	File operations for EDMA common stats
 */
const struct file_operations ppe_policer_stats_file_ops = {
	.open = ppe_policer_stats_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

/*
 * ppe_policer_stats_debugfs_init()
 *	Create PPE statistics debug entry.
 */
int ppe_policer_stats_debugfs_init(struct dentry *root)
{
	struct ppe_policer_base *g_pol = &gbl_ppe_policer;
	g_pol->dentry = debugfs_create_dir("ppe-policer", root);
	if (!g_pol->dentry) {
		ppe_policer_warn("%p: Unable to create debugfs stats directory in debugfs\n", g_pol);
		return -1;
	}

	if (!debugfs_create_file("stats", S_IRUGO, g_pol->dentry,
			NULL, &ppe_policer_stats_file_ops)) {
		ppe_policer_warn("%p: Unable to create common statistics file entry in debugfs\n", g_pol);
		goto debugfs_dir_failed;
	}

	return 0;

debugfs_dir_failed:
	debugfs_remove_recursive(g_pol->dentry);
	g_pol->dentry = NULL;
	return -1;
}

/*
 * ppe_policer_stats_debugfs_exit()
 *	PPE debugfs exit api
 */
void ppe_policer_stats_debugfs_exit(void)
{
	struct ppe_policer_base *g_pol = &gbl_ppe_policer;
	if (g_pol->dentry) {
		debugfs_remove_recursive(g_pol->dentry);
		g_pol->dentry = NULL;
	}
}
