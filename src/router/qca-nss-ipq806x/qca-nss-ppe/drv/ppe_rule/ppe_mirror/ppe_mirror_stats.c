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
#include <linux/vmalloc.h>
#include <linux/debugfs.h>
#include <linux/netdevice.h>
#include <ppe_drv_port.h>
#include "ppe_mirror.h"

/*
 * ppe_mirror_stats_str
 *	PPE mirror stats
 */
static const char *ppe_mirror_stats_str[] = {
	"Mirrored_packets",			/* Mirrored packets. */
	"Mirrored_bytes",			/* Mirrored bytes. */
};

/*
 * ppe_mirror_stats_cmn_str
 *	PPE mirror common statistics
 */
static const char *ppe_mirror_stats_cmn_str[] = {
	"mirror_acl_mapping_add_req",			/* ACL mapping add requests. */
	"mirror_acl_mapping_delete_req",		/* ACL mapping delete requests. */
	"mirror_acl_mapping_add_success",		/* ACL mapping add success. */
	"mirror_acl_mapping_delete_success",		/* ACL mapping delete success. */
	"mirror_acl_active_mapping_count",		/* ACL active mapping cound. */
	"mirror_acl_mapping_add_fail_invalid_rule_id",	/* ACL mapping add fail invalid rule ID. */
	"mirror_acl_mapping_add_fail_rule_not_found",	/* ACL mapping add fail rule not found. */
	"mirror_acl_mapping_add_map_exist",		/* ACL mapping add fail mapping exists. */
	"mirror_acl_mapping_add_map_nomem",		/* ACL mapping add fail no memory. */
	"mirror_acl_mapping_add_invalid_group_info",	/* ACL mapping add fail group invalid. */
	"mirror_acl_mapping_del_fail_invalid_rule_id",	/* ACL mapping delete fail invalid rule ID. */
	"mirror_acl_mapping_del_fail_rule_not_found",	/* ACL mapping delete fail rule not found. */
	"mirror_acl_mapping_del_fail_map_not_found",	/* ACL mapping delete fail mapping not found. */
	"mirror_acl_mapping_del_fail_group_not_found",	/* ACL mapping delete fail group not found. */
	"mirror_acl_process_mapping_invalid",		/* ACL mapping not found for mirrored packets. */
	"mirror_acl_process_group_invalid",		/* ACL group info not found for mirrored packets. */
	"mirror_acl_mapping_invalid_capture_core",	/* ACL mapppng invalid capture core selection. */
	"mirror_acl_mapping_fail_en_capture_core",	/* ACL mapping failed to enable capture core. */
};

/*
 * ppe_mirror_group_stats_show()
 *	Read ppe mirror group statistics
 */
static int ppe_mirror_group_stats_show(struct seq_file *m, void __attribute__((unused))*ptr)
{
	struct ppe_mirror *mirror_g = &gbl_ppe_mirror;
	struct ppe_mirror_group_info *group_info = NULL;
	struct ppe_mirror_group_info *cur_group;
	uint64_t *mirror_stats_shadow;
	uint16_t active_groups = 0;
	int16_t i = 0;
	int16_t idx;

	/*
	 * Start displaying the group information.
	 */
	seq_printf(m, "\n################ Mirror Group Statistics ################\n");

	spin_lock_bh(&mirror_g->lock);
	active_groups = mirror_g->no_of_active_mirror_groups;
	group_info = kmalloc(sizeof(struct ppe_mirror_group_info)*active_groups, GFP_ATOMIC);
	if (!group_info) {
		ppe_mirror_warn("Failed to allocate memory for group stats\n");
		spin_unlock_bh(&mirror_g->lock);
		return -ENOMEM;
	}

	/*
	 * Copy all the group information for display while taking a lock.
	 */
	list_for_each_entry(cur_group, &mirror_g->active_mirror_groups, list) {
		memcpy(group_info + i, cur_group, sizeof(struct ppe_mirror_group_info));
		i++;
	}

	spin_unlock_bh(&mirror_g->lock);

	seq_printf(m, "\nNumber of active Mirror groups (Active Count = %u)\n", active_groups);

	for (idx = 0; idx < active_groups; idx++) {
		struct ppe_mirror_group_info *shadow_group = (struct ppe_mirror_group_info *)(group_info + idx);
		seq_printf(m, "\t\tGroup Number: %u\n", idx);
		seq_printf(m, "\t\t\tNetdev if num: %u\n", shadow_group->group_dev->ifindex);
		seq_printf(m, "\t\t\tNetdev name: %s\n", shadow_group->group_dev->name);

		mirror_stats_shadow = (uint64_t *)(&shadow_group->acl_stats);
		seq_printf(m, "\n\t\t\t Mirrored packets group stats:\n");
		for (i = 0; i < (sizeof(struct ppe_mirror_stats) / sizeof(uint64_t)); i++)
			seq_printf(m, "\t\t\t[%s]:  %llu\n", ppe_mirror_stats_str[i], mirror_stats_shadow[i]);
	}

	kfree(group_info);
	return 0;
}

/*
 * ppe_mirror_group_stats_open()
 *	PPE mirror group stats callback API
 */
static int ppe_mirror_group_stats_open(struct inode *inode, struct file *file)
{
	return single_open(file, ppe_mirror_group_stats_show, inode->i_private);
}

/*
 * ppe_mirror_acl_stats_show()
 *	Read ppe ACL mirror statistics
 */
static int ppe_mirror_acl_stats_show(struct seq_file *m, void __attribute__((unused))*ptr)
{
	struct ppe_mirror *mirror_g = &gbl_ppe_mirror;
	struct ppe_mirror_acl_map *mirror_mapping = NULL;
	uint64_t *mirror_stats_shadow;
	int idx, i;

	mirror_mapping = (struct ppe_mirror_acl_map *)kmalloc(sizeof(struct ppe_mirror_acl_map) * PPE_MIRROR_ACL_HW_INDEX_MAX, GFP_KERNEL);
	if (!mirror_mapping) {
		ppe_mirror_warn("Error in allocating ACL stats\n");
		return -ENOMEM;
	}

	spin_lock_bh(&mirror_g->lock);
	memcpy(mirror_mapping, &mirror_g->mirror_mapping, sizeof(struct ppe_mirror_acl_map) * PPE_MIRROR_ACL_HW_INDEX_MAX);
	spin_unlock_bh(&mirror_g->lock);

	seq_puts(m, "\nPPE MIRROR ACL stats:\n\n");
	for (idx = 0; idx < PPE_MIRROR_ACL_HW_INDEX_MAX; idx++) {
		struct ppe_mirror_acl_map *shadow_mapping = (struct ppe_mirror_acl_map *)(mirror_mapping + idx);
		if (shadow_mapping->is_valid) {
			seq_printf(m, "\t\tACL id : %u\n", shadow_mapping->acl_rule_id);
			seq_printf(m, "\t\t\tIs mapping valid : %d\n", shadow_mapping->is_valid);
			seq_printf(m, "\t\t\tGroup Dev : %s\n", shadow_mapping->group_info->group_dev->name);
			seq_printf(m, "\t\t\tGroup Dev ifindex : %d\n", shadow_mapping->group_info->group_dev->ifindex);

			mirror_stats_shadow = (uint64_t *)(&shadow_mapping->acl_stats);
			seq_printf(m, "\n\t\t\t Mirrored packets ACL stats:\n");
			for (i = 0; i < (sizeof(struct ppe_mirror_stats) / sizeof(uint64_t)); i++) {
				seq_printf(m, "\t\t\t[%s]:  %llu\n", ppe_mirror_stats_str[i], mirror_stats_shadow[i]);
			}
		}
	}

	kfree(mirror_mapping);
	return 0;
}

/*
 * ppe_mirror_pdev_stats_show()
 *	Read ppe PDEV mirror statistics
 */
static int ppe_mirror_pdev_stats_show(struct seq_file *m, void __attribute__((unused))*ptr)
{
	struct ppe_mirror *mirror_g = &gbl_ppe_mirror;
	struct ppe_mirror_port_group_info *group_info = NULL;
	uint64_t *mirror_stats_shadow;
	int i;

	group_info = kmalloc(sizeof(struct ppe_mirror_port_group_info), GFP_ATOMIC);
	if (!group_info) {
		ppe_mirror_warn("Error in allocating ACL stats\n");
		return -ENOMEM;
	}

	spin_lock_bh(&mirror_g->lock);
	memcpy(group_info, &mirror_g->port_group_info, sizeof(struct ppe_mirror_port_group_info));
	spin_unlock_bh(&mirror_g->lock);

	if (!group_info->group_dev) {
		seq_printf(m, "\t\tNo group netdevice for physical ports !\n");
		kfree(group_info);
		return 0;
	}

	seq_printf(m, "\t\tPdev Group Details\n");
	seq_printf(m, "\t\t\tNetdev if num: %d\n", group_info->group_dev->ifindex);
	seq_printf(m, "\t\t\tNetdev name: %s\n", group_info->group_dev->name);

	mirror_stats_shadow = (uint64_t *)(&group_info->pdev_stats);
	seq_printf(m, "\n\t\t\t Mirrored packets group PDEV stats:\n");
	for (i = 0; i < (sizeof(struct ppe_mirror_stats) / sizeof(uint64_t)); i++)
		seq_printf(m, "\t\t\t[%s]:  %llu\n", ppe_mirror_stats_str[i], mirror_stats_shadow[i]);

	kfree(group_info);

	return 0;
}

/*
 * ppe_mirror_acl_stats_open()
 *	PPE mirror acl stats callback API
 */
static int ppe_mirror_acl_stats_open(struct inode *inode, struct file *file)
{
	return single_open(file, ppe_mirror_acl_stats_show, inode->i_private);
}

/*
 * ppe_mirror_pdev_stats_open()
 *	PPE mirror cmn stats callback API
 */
static int ppe_mirror_pdev_stats_open(struct inode *inode, struct file *file)
{
	return single_open(file, ppe_mirror_pdev_stats_show, inode->i_private);
}

/*
 * ppe_mirror_cmn_stats_show()
 *	Read ppe mirror statistics
 */
static int ppe_mirror_cmn_stats_show(struct seq_file *m, void __attribute__((unused))*ptr)
{
	struct ppe_mirror *mirror_g = &gbl_ppe_mirror;
	uint64_t *stats, *stats_shadow;
	uint32_t stats_size;
	int i;

	stats_size = sizeof(struct ppe_mirror_cmn_stats);

	stats = kzalloc(stats_size, GFP_KERNEL);
	if (!stats) {
		ppe_mirror_warn("Error in allocating common stats\n");
		return -ENOMEM;
	}

	spin_lock_bh(&mirror_g->lock);
	memcpy(stats, &mirror_g->stats, sizeof(struct ppe_mirror_cmn_stats));
	spin_unlock_bh(&mirror_g->lock);

	seq_puts(m, "\nPPE MIRROR common stats:\n\n");
	stats_shadow = stats;
	for (i = 0; i < sizeof(struct ppe_mirror_cmn_stats) / sizeof(uint64_t); i++) {
		seq_printf(m, "\t\t [%s]:  %llu\n", ppe_mirror_stats_cmn_str[i], stats_shadow[i]);
	}

	kfree(stats);
	return 0;
}

/*
 * ppe_mirror_cmn_stats_open()
 *	PPE mirror cmn stats callback API
 */
static int ppe_mirror_cmn_stats_open(struct inode *inode, struct file *file)
{
	return single_open(file, ppe_mirror_cmn_stats_show, inode->i_private);
}

/*
 * ppe_mirror_group_stats_file_ops
 *	File operations for MIRROR group stats
 */
const struct file_operations ppe_mirror_group_stats_file_ops = {
	.open = ppe_mirror_group_stats_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

/*
 * ppe_mirror_acl_stats_file_ops
 *	File operations for MIRROR ACL stats
 */
const struct file_operations ppe_mirror_acl_stats_file_ops = {
	.open = ppe_mirror_acl_stats_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

/*
 * ppe_mirror_pdev_stats_file_ops
 *	File operations for MIRROR Pdev stats
 */
const struct file_operations ppe_mirror_pdev_stats_file_ops = {
	.open = ppe_mirror_pdev_stats_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

/*
 * ppe_mirror_cmn_stats_file_ops
 *	File operations for MIRROR common stats
 */
const struct file_operations ppe_mirror_cmn_stats_file_ops = {
	.open = ppe_mirror_cmn_stats_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

/*
 * ppe_mirror_stats_debugfs_init()
 *	Create PPE statistics debug entry.
 */
int ppe_mirror_stats_debugfs_init(struct dentry *root)
{
	struct ppe_mirror *mirror_g = &gbl_ppe_mirror;
	mirror_g->dentry = debugfs_create_dir("ppe-mirror", root);
	if (!mirror_g->dentry) {
		ppe_mirror_warn("%p: Unable to create debugfs stats directory in debugfs\n", mirror_g);
		return -1;
	}

	if (!debugfs_create_file("common_stats", S_IRUGO, mirror_g->dentry,
				NULL, &ppe_mirror_cmn_stats_file_ops)) {
		ppe_mirror_warn("%p: Unable to create common statistics file entry in debugfs\n", mirror_g);
		goto debugfs_dir_failed;
	}

	if (!debugfs_create_file("acl_stats", S_IRUGO, mirror_g->dentry,
				NULL, &ppe_mirror_acl_stats_file_ops)) {
		ppe_mirror_warn("%p: Unable to create ACL statistics file entry in debugfs\n", mirror_g);
		goto debugfs_dir_failed;
	}

	if (!debugfs_create_file("pdev_stats", S_IRUGO, mirror_g->dentry,
				NULL, &ppe_mirror_pdev_stats_file_ops)) {
		ppe_mirror_warn("%p: Unable to create PDEV statistics file entry in debugfs\n", mirror_g);
		goto debugfs_dir_failed;
	}

	if (!debugfs_create_file("group_stats", S_IRUGO, mirror_g->dentry,
				NULL, &ppe_mirror_group_stats_file_ops)) {
		ppe_mirror_warn("%p: Unable to create group statistics file entry in debugfs\n", mirror_g);
		goto debugfs_dir_failed;
	}

	return 0;

debugfs_dir_failed:
	debugfs_remove_recursive(mirror_g->dentry);
	mirror_g->dentry = NULL;
	return -1;
}

/*
 * ppe_mirror_stats_debugfs_exit()
 *	PPE debugfs exit api
 */
void ppe_mirror_stats_debugfs_exit(void)
{
	struct ppe_mirror *mirror_g = &gbl_ppe_mirror;
	if (mirror_g->dentry) {
		debugfs_remove_recursive(mirror_g->dentry);
		mirror_g->dentry = NULL;
	}
}
