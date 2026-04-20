/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/debugfs.h>
#include <ppe_drv_public.h>
#include <ppe_vp_public.h>
#include <nss_dp_vp.h>

#include "ppe_vp_stats.h"
#include "ppe_vp.h"

#define PPE_VP_BASE_FLAG_ENABLE_FEATURE		0x00000001
#define PPE_VP_BASE_PORT_TO_IDX(port_num)	((port_num) - PPE_DRV_VIRTUAL_START)

/*
 * PPE VP debug macros
 */
#if (PPE_VP_DEBUG_LEVEL == 3)
#define ppe_vp_assert(c, s, ...)
#else
#define ppe_vp_assert(c, s, ...) if (!(c)) { printk(KERN_CRIT "%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__); BUG_ON(!(c)); }
#endif

#if defined(CONFIG_DYNAMIC_DEBUG)
/*
 * If dynamic debug is enabled, use pr_debug.
 */
#define ppe_vp_warn(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define ppe_vp_info(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define ppe_vp_trace(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else

/*
 * Statically compile messages at different levels, when dynamic debug is disabled.
 */
#if (PPE_VP_DEBUG_LEVEL < 2)
#define ppe_vp_warn(s, ...)
#else
#define ppe_vp_warn(s, ...) pr_warn("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if (PPE_VP_DEBUG_LEVEL < 3)
#define ppe_vp_info(s, ...)
#else
#define ppe_vp_info(s, ...) pr_notice("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if (PPE_VP_DEBUG_LEVEL < 4)
#define ppe_vp_trace(s, ...)
#else
#define ppe_vp_trace(s, ...) pr_info("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif
#endif

/*
 * ppe_vp_table
 *	PPE VP allocation table.
 */
struct ppe_vp_table {
	struct ppe_vp vp_pool[PPE_DRV_VIRTUAL_MAX];	/* PPE VP table */
	struct ppe_vp __rcu *vp_allocator[PPE_DRV_VIRTUAL_MAX];
							/* VP object allocator */
	uint8_t active_vp;				/* Total active VPs in the system */
};

/*
 * ppe_vp_base
 *      Singleton structure object for PPE VP Base.
 */
struct ppe_vp_base {
	struct ppe_vp_table vp_table;			/* VP allocation Table object */

	unsigned long hw_port_stats_ticks;		/* Ticks to re-arm the hardware stats timer */
	struct timer_list hw_port_stats_timer;		/* Timer used to poll for HW stats from PPE_HW */
	struct ppe_vp_base_stats base_stats;		/* VP Stats */

	struct net_device *edma_vp_dev;			/* EDMA Device to queue VP packets */
	uint32_t flags;					/* Base VP flags */
	struct dentry *dentry;				/* Debugfs entry */
	spinlock_t lock;				/* Lock for Base infra */
	struct ctl_table_header *vp_hdr;		/* VP sysctl header, dir: /proc/sys/ppe/ppe_vp */
};

/*
 * ppe_vp_base_get_active_vp_count()
 *	Return the active VP count.
 */
static inline uint8_t ppe_vp_base_get_active_vp_count(struct ppe_vp_base *pvb)
{
	uint8_t active_vp;

	spin_lock_bh(&pvb->lock);
	active_vp = pvb->vp_table.active_vp;
	spin_unlock_bh(&pvb->lock);

	return active_vp;
}

extern struct ppe_vp *ppe_vp_base_get_vp_by_port_num(uint8_t port_num);
extern struct ppe_vp *ppe_vp_base_get_vp_by_idx(int16_t vp_idx);
extern int ppe_vp_base_deref(struct ppe_vp_base *pvb);
extern struct ppe_vp_base *ppe_vp_base_ref(struct ppe_vp_base *pvb);
extern bool ppe_vp_base_free_vp(uint8_t port_num);
extern struct ppe_vp *ppe_vp_base_alloc_vp(uint8_t port_num);
