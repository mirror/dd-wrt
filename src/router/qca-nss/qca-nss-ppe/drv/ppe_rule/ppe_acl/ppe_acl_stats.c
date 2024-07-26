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
#include "ppe_acl.h"

/*
 * ppe_acl_stats_cmn_str
 * 	PPE ACL common statistics
 */
static const char *ppe_acl_stats_cmn_str[] = {
	"acl_create_req",			/* ACL create requests. */
	"acl_destroy_req",			/* ACL destroy requests. */
	"acl_free_req",				/* ACL rule free. */
	"rule_id_invalid",			/* ACL rule ID invalid. */
	"hw_index_invalid",			/* ACL rule Hardware index invalid. */
	"rule_not_found", 			/* ACL rule not found based on rule ID. */
        "acl_flow_add_invalid_id",		/* ACL flow add rule ID invalid. */
	"acl_flow_add_invalid_sc",		/* ACL flow add rule SC invalid. */
	"acl_flow_del_invalid_id",		/* ACL flow del rule ID invalid. */
	"acl_create_fail",			/* ACL create request failure. */
	"acl_create_fail_rule_table_full",	/* ACL rule table full. */
	"acl_create_fail_oom",			/* Not able to allocate rule memory. */
	"acl_create_fail_max_slices",		/* ACL rule needing more than max slices. */
	"acl_create_fail_alloc",		/* ACL rule allocation failure. */
	"acl_create_fail_rule_config",		/* ACL rule configuration failure. */
	"acl_create_fail_invalid_src",		/* ACL rule create failure due to invalid src. */
	"acl_create_fail_invalid_cmn",		/* ACL rule create failure due to invalid common fields. */
	"acl_create_fail_fill",			/* ACL rule create failure due to invalid rule and slice map. */
	"acl_create_fail_policer_sc",		/* ACL rule create failure due to policer sc table full. */
	"acl_create_fail_rule_exist",		/* ACL rule create failure due to collision. */
	"acl_create_fail_action_config",	/* ACL rule create failure due to invalid action. */
	"acl_create_fail_invalid_id",		/* ACL rule create failure due to invalid rule-ID. */
	"acl_destroy_fail_invalid_id",		/* ACL destroy failure due to invalid rule ID. */
};

/*
 * ppe_acl_stats_cmn_show()
 *	Read ppe rfs connection statistics
 */
static int ppe_acl_stats_cmn_show(struct seq_file *m, void __attribute__((unused))*ptr)
{
	struct ppe_acl_base *acl_g = &ppe_acl_gbl;
	uint64_t *stats, *stats_shadow;
	uint32_t stats_size;
	int i;

	stats_size = sizeof(struct ppe_acl_stats_cmn);

	stats = kzalloc(stats_size, GFP_KERNEL);
	if (!stats) {
		ppe_acl_warn("Error in allocating common stats\n");
		return -ENOMEM;
	}

	spin_lock_bh(&acl_g->lock);
	memcpy(stats, &acl_g->stats.cmn, sizeof(struct ppe_acl_stats_cmn));
	spin_unlock_bh(&acl_g->lock);

	seq_puts(m, "\nACL stats:\n\n");
	stats_shadow = stats;
	for (i = 0; i < sizeof(struct ppe_acl_stats) / sizeof(uint64_t); i++) {
		seq_printf(m, "\t\t [%s]:  %llu\n", ppe_acl_stats_cmn_str[i], stats_shadow[i]);
	}

	kfree(stats);
	return 0;
}

/*
 * ppe_acl_stats_cmn_open()
 *	PPE ACL common stats callback
 */
static int ppe_acl_stats_cmn_open(struct inode *inode, struct file *file)
{
	return single_open(file, ppe_acl_stats_cmn_show, inode->i_private);
}

/*
 * ppe_acl_stats_cmn_file_ops
 *	File operations for ACL common stats
 */
const struct file_operations ppe_acl_stats_cmn_file_ops = {
	.open = ppe_acl_stats_cmn_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

/*
 * ppe_acl_stats_debugfs_exit()
 *	ACL debugfs exit api
 */
void ppe_acl_stats_debugfs_exit(void)
{
	struct ppe_acl_base *acl_g = &ppe_acl_gbl;

	if (acl_g->dentry) {
		debugfs_remove_recursive(acl_g->dentry);
		acl_g->dentry = NULL;
	}
}

/*
 * ppe_acl_stats_debugfs_init()
 *	Create ACL statistics debugfs entries.
 */
int ppe_acl_stats_debugfs_init(struct dentry *root)
{
	struct ppe_acl_base *acl_g = &ppe_acl_gbl;

	acl_g->dentry = debugfs_create_dir("ppe-acl", root);
	if (!acl_g->dentry) {
		ppe_acl_warn("%p: Unable to create ACL debugfs directory\n", acl_g);
		return -1;
	}

	if (!debugfs_create_file("cmn_stats", S_IRUGO, acl_g->dentry,
			NULL, &ppe_acl_stats_cmn_file_ops)) {
		ppe_acl_warn("%p: Unable to create cmn stats file in debugfs\n", acl_g);
		debugfs_remove_recursive(acl_g->dentry);
		acl_g->dentry = NULL;
		return -1;
	}

	return 0;
}
