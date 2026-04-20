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


#include <linux/debugfs.h>
#include <linux/debugfs.h>
#include <linux/module.h>
#include <ppe_drv.h>
#include "ppe_rfs/ppe_rfs.h"
#if !defined(NSS_PPE_LOWMEM_PROFILE_16M)
#include "ppe_acl/ppe_acl.h"
#include "ppe_policer/ppe_policer.h"
#endif
#if !defined(NSS_PPE_LOWMEM_PROFILE_16M) && !defined(NSS_PPE_LOWMEM_PROFILE_256M)
#include "ppe_mirror/ppe_mirror.h"
#include "ppe_priority/ppe_priority.h"
#endif

struct dentry *d_rule;

/*
 * ppe_rule_module_init()
 *	module init for ppe rule driver.
 */
static int __init ppe_rule_module_init(void)
{
	struct dentry *root = ppe_drv_get_dentry();
	if (!root) {
		printk("Unable to get root stats directory\n");
		return -1;
	}

	d_rule = debugfs_create_dir("ppe-rule", root);
	if (!d_rule) {
		printk("Unable to create debugfs stats directory in debugfs\n");
		return -1;
	}
	ppe_rfs_init(d_rule);
#if !defined(NSS_PPE_LOWMEM_PROFILE_16M)
	ppe_acl_init(d_rule);
	ppe_policer_init(d_rule);
#endif
#if !defined(NSS_PPE_LOWMEM_PROFILE_16M) && !defined(NSS_PPE_LOWMEM_PROFILE_256M)
	ppe_mirror_init(d_rule);
	ppe_priority_init(d_rule);
#endif

	printk("PPE-RULE module loaded successfully\n");
	return 0;
}
module_init(ppe_rule_module_init);

/*
 * ppe_rule_module_exit()
 *	module exit for ppe rule driver.
 */
static void __exit ppe_rule_module_exit(void)
{
	ppe_rfs_deinit();

#if !defined(NSS_PPE_LOWMEM_PROFILE_16M) && !defined(NSS_PPE_LOWMEM_PROFILE_256M)
	ppe_mirror_deinit();
	ppe_priority_deinit();
#endif
#if !defined(NSS_PPE_LOWMEM_PROFILE_16M)
	ppe_policer_deinit();
	ppe_acl_deinit();
#endif
	debugfs_remove_recursive(d_rule);
	printk("PPE-RULE module unloaded");
}
module_exit(ppe_rule_module_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("PPE RULE driver");
