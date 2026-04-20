/*
 * Copyright (c) 2023-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include "ppe_mirror_stats.h"
#include <ppe_drv_iface.h>
#include <ppe_mirror.h>
#include <ppe_acl.h>
#include <ppe_drv_port.h>
#include <ppe_drv_dp.h>

/*
 * PPE MIRROR macros
 */
#if (PPE_MIRROR_DEBUG_LEVEL == 3)
#define ppe_mirror_assert(c, s, ...)
#else
#define ppe_mirror_assert(c, s, ...) if (!(c)) { printk(KERN_CRIT "%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__); BUG_ON(!(c)); }
#endif

#if defined(CONFIG_DYNAMIC_DEBUG)
/*
 * If dynamic debug is enabled, use pr_debug.
 */
#define ppe_mirror_warn(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define ppe_mirror_info(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define ppe_mirror_trace(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else

/*
 * Statically compile messages at different levels, when dynamic debug is disabled.
 */
#if (PPE_MIRROR_DEBUG_LEVEL < 2)
#define ppe_mirror_warn(s, ...)
#else
#define ppe_mirror_warn(s, ...) pr_warn("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if (PPE_MIRROR_DEBUG_LEVEL < 3)
#define ppe_mirror_info(s, ...)
#else
#define ppe_mirror_info(s, ...) pr_notice("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if (PPE_MIRROR_DEBUG_LEVEL < 4)
#define ppe_mirror_trace(s, ...)
#else
#define ppe_mirror_trace(s, ...) pr_info("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif
#endif

#define PPE_MIRROR_ACL_RULE_MAX		2048
#define PPE_MIRROR_ACL_HW_INDEX_MAX	1024
#define PPE_MIRROR_CAPTURE_CORE_MAX	NR_CPUS

/*
 * ppe_mirror_port_group_info
 *	Port mirror group information.
 */
struct ppe_mirror_port_group_info {
	struct net_device *group_dev;		/* Group net device */
	ppe_mirror_capture_callback_t cb;	/* Capture callback */
	void *app_data;				/* App data */
	struct ppe_mirror_stats pdev_stats;	/* PDEV Group level stats info */
};

/*
 * ppe_mirror_group_info
 *	Mirror group related information.
 */
struct ppe_mirror_group_info {
	struct list_head list;			/* List head for active list */
	struct net_device *group_dev;		/* Group net device */
	ppe_mirror_capture_callback_t cb;	/* Capture callback */
	void *app_data;				/* App data */
	struct ppe_mirror_stats acl_stats;	/* ACL Group level stats info */
	uint16_t number_of_mappings;		/* Number of mappings on this group */
};

/*
 * ppe_mirror_acl_map
 *	PPE ACL rule mapping DB
 */
struct ppe_mirror_acl_map {
	uint16_t acl_rule_id;				/* PPE software rule index */
	struct ppe_mirror_group_info *group_info;	/* Pointer to the node for the group in the group list */
	struct ppe_mirror_stats acl_stats;		/* Mirror ACL stats */
	bool is_valid;					/* Valid flag for this ACL index */
};

/*
 * ppe_mirror
 *	Global ppe mirror instance.
 */
struct ppe_mirror {
	spinlock_t lock;							/* PPE Mirror lock */
	struct ppe_mirror_acl_map mirror_mapping[PPE_MIRROR_ACL_HW_INDEX_MAX];	/* PPE Mirror map to ACL hardware index */
	struct ppe_mirror_port_group_info port_group_info;			/* Physical port group info */
	struct list_head active_mirror_groups;					/* List for active mirror groups */
	struct ppe_mirror_cmn_stats stats;					/* PPE Mirror Common statistics */
	struct dentry *dentry;							/* Debugfs root entry */
	uint8_t no_of_active_mirror_groups;					/* Number of active mirror groups */
};

/*
 * PPE Mirror manager APIs.
 */
void ppe_mirror_deinit(void);
void ppe_mirror_init(struct dentry *dentry);

extern struct ppe_mirror gbl_ppe_mirror;
