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

#ifndef __PPE_RFS_STATS_H
#define __PPE_RFS_STATS_H

#define PPE_RFS_STATS_NODE_NAME		"PPE_RFS"

/*
 * ppe_rfs_stats
 *	Message structure for ppe general stats
 */
struct ppe_rfs_stats {
	atomic64_t v4_create_ppe_rule_rfs;		/* v4 create rfs rule request */
	atomic64_t v4_create_mem_fail;			/* v4 create rfs memory allocation failed */
	atomic64_t v4_create_flow_interface_fail;	/* v4 create rfs flow interface invalid */
	atomic64_t v4_create_return_interface_fail;	/* v4 create rfs return interface invalid */
	atomic64_t v4_create_flow_top_interface_fail;	/* v4 create rfs flow top interface invalid */
	atomic64_t v4_create_return_top_interface_fail; /* v4 create rfs return top interface invalid */
	atomic64_t v4_create_rfs_not_enabled;		/* v4 create rfs not enabled on both tx and rx interface */
	atomic64_t v4_create_ppe_rule_fail;		/* v4 create rfs ppe rule addition failed */
	atomic64_t v4_create_rfs_direction_check_fail;	/* v4 create rfs enabled on both tx and rx interface */
	atomic64_t v4_destroy_ppe_rule_rfs;		/* v4 destroy rfs rule request */
	atomic64_t v4_destroy_mem_fail;			/* v4 destroy rfs memory allocation failed */
	atomic64_t v4_destroy_rfs_not_enabled;		/* v4 destroy rfs not enabled */
	atomic64_t v4_destroy_ppe_rule_fail;		/* v4 destroy rfs ppe rule addition failed */
	atomic64_t v4_destroy_rfs_direction_check_fail;	/* v4 destroy rfs enabled on both tx and rx interface */

	atomic64_t v6_create_ppe_rule_rfs;		/* v6 create rfs rule request */
	atomic64_t v6_create_mem_fail;			/* v6 create rfs memory allocation failed */
	atomic64_t v6_create_flow_interface_fail;	/* v6 create rfs flow interface invalid */
	atomic64_t v6_create_return_interface_fail;	/* v6 create rfs return interface invalid */
	atomic64_t v6_create_flow_top_interface_fail;	/* v6 create rfs flow top interface invalid */
	atomic64_t v6_create_return_top_interface_fail;	/* v6 create rfs return top interface invalid */
	atomic64_t v6_create_rfs_not_enabled;		/* v6 create rfs not enabled on both tx and rx interface */
	atomic64_t v6_create_ppe_rule_fail;		/* v6 create rfs ppe rule addition failed */
	atomic64_t v6_create_rfs_direction_check_fail;		/* v6 destroy rfs enabled on both tx and rx interface */
	atomic64_t v6_destroy_ppe_rule_rfs;		/* v6 destroy rfs rule request */
	atomic64_t v6_destroy_mem_fail;			/* v6 destroy rfs memory allocation failed */
	atomic64_t v6_destroy_rfs_not_enabled;		/* v6 destroy rfs not enabled */
	atomic64_t v6_destroy_ppe_rule_fail;		/* v6 destroy rfs ppe rule addition failed */
	atomic64_t v6_destroy_rfs_direction_check_fail;	/* v6 destroy rfs enabled on both tx and rx interface */
};

/*
 * ppe_rfs_stats_dec()
 *	Decrement stats counter.
 */
static inline void ppe_rfs_stats_dec(atomic64_t *stat)
{
	atomic64_dec(stat);
}

/*
 * ppe_rfs_stats_inc()
 *	Increment stats counter.
 */
static inline void ppe_rfs_stats_inc(atomic64_t *stat)
{
	atomic64_inc(stat);
}

int ppe_rfs_stats_debugfs_init(struct dentry *dentry);
void ppe_rfs_stats_debugfs_exit(void);

#endif
