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

#ifndef __PPE_PRIORITY_STATS_H
#define __PPE_PRIORITY_STATS_H

#define PPE_PRIORITY_STATS_NODE_NAME		"PPE_PRIORITY"

/*
 * ppe_priority_stats
 *	Message structure for ppe general stats
 */
struct ppe_priority_stats {
	atomic64_t v4_create_ppe_rule_priority;		/* v4 create priority rule request */
	atomic64_t v4_create_ppe_rule_priority_fail;	/* v4 create priority ppe rule addition failed */
	atomic64_t v4_destroy_ppe_rule_priority;	/* v4 destroy priority rule request */
	atomic64_t v4_destroy_ppe_rule_priority_fail;		/* v4 destroy priority ppe rule addition failed */

	atomic64_t v6_create_ppe_rule_priority;		/* v6 create priority rule request */
	atomic64_t v6_create_ppe_rule_priority_fail;	/* v6 create priority ppe rule addition failed */
	atomic64_t v6_destroy_ppe_rule_priority;	/* v6 destroy priority rule request */
	atomic64_t v6_destroy_ppe_rule_priority_fail;	/* v6 destroy priority ppe rule addition failed */
};

/*
 * ppe_priority_stats_dec()
 *	Decrement stats counter.
 */
static inline void ppe_priority_stats_dec(atomic64_t *stat)
{
	atomic64_dec(stat);
}

/*
 * ppe_priority_stats_inc()
 *	Increment stats counter.
 */
static inline void ppe_priority_stats_inc(atomic64_t *stat)
{
	atomic64_inc(stat);
}

int ppe_priority_stats_debugfs_init(struct dentry *dentry);
void ppe_priority_stats_debugfs_exit(void);
#endif
