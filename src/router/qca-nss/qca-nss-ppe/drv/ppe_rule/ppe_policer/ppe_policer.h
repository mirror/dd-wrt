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

#include "ppe_policer_stats.h"

#define PPE_ACL_POLICER_FLOW_RULE_MAX 128
#define PPE_POLICER_PORT_RULE_MAX 8

/*
 * PPE RFS macros
 */
#if (PPE_POLICER_DEBUG_LEVEL == 3)
#define ppe_policer_assert(c, s, ...)
#else
#define ppe_policer_assert(c, s, ...) if (!(c)) { printk(KERN_CRIT "%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__); BUG_ON(!(c)); }
#endif

#if defined(CONFIG_DYNAMIC_DEBUG)
/*
 * If dynamic debug is enabled, use pr_debug.
 */
#define ppe_policer_warn(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define ppe_policer_info(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define ppe_policer_trace(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else

/*
 * Statically compile messages at different levels, when dynamic debug is disabled.
 */
#if (PPE_POLICER_DEBUG_LEVEL < 2)
#define ppe_policer_warn(s, ...)
#else
#define ppe_policer_warn(s, ...) pr_warn("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if (PPE_POLICER_DEBUG_LEVEL < 3)
#define ppe_policer_info(s, ...)
#else
#define ppe_policer_info(s, ...) pr_notice("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if (PPE_POLICER_DEBUG_LEVEL < 4)
#define ppe_policer_trace(s, ...)
#else
#define ppe_policer_trace(s, ...) pr_info("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif
#endif

struct ppe_drv_policer_acl;
struct ppe_drv_policer_port;

/*
 * struct ppe_policer
 *	Policer structure
 */
struct ppe_policer {
	struct list_head list;				/* List of active Policer rules */
	struct kref kref_cnt;				/* Reference count */
	struct net_device *dev;				/* Device associated with port policer */
	uint32_t rule_id;				/* Associated Policer rule id for ACL/FLOW policer */
	uint32_t acl_rule_id;				/* ACL rule id for Policer + FLOW case */
	union {
		struct ppe_drv_policer_acl *acl_ctx;     	  	/* PPE driver context */
		struct ppe_drv_policer_port *port_ctx;     	  	/* PPE driver context */
	} drv_ctx;
};

/*
 * ppe_policer
 *	Global ppe policer global instance
 */
struct ppe_policer_base {
	spinlock_t lock;                                	/* PPE lock */
	struct ppe_policer_stats stats;                     	/* PPE RFS statistics */
	struct dentry *dentry;					/* Debugfs entry */

	struct kref ref;					/* Reference count */

	struct list_head port_active_rules;			/* List of active Policer rules */
	struct list_head acl_active_rules;			/* List of active Policer rules */
};

int ppe_policer_id_to_hwidx(uint16_t policer_id);
void ppe_policer_deinit(void);
void ppe_policer_init(struct dentry *dentry);

extern struct ppe_policer_base gbl_ppe_policer;

