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

#ifndef __PPE_ACL_STATS_H
#define __PPE_ACL_STATS_H

#define PPE_ACL_STATS_NODE_NAME		"PPE_ACL"

/*
 * ppe_acl_stats_cmn
 *	Structure for ACL common stats
 */
struct ppe_acl_stats_cmn {
	atomic64_t acl_create_req;			/* ACL create requests. */
	atomic64_t acl_destroy_req;			/* ACL destroy requests. */
	atomic64_t acl_free_req;			/* ACL rule free. */
	atomic64_t rule_id_invalid;			/* ACL rule ID invalid. */
	atomic64_t acl_hw_index_invalid;		/* ACL hardware index invalid. */
	atomic64_t rule_not_found;			/* ACL rule not found based on rule ID. */
	atomic64_t acl_flow_add_invalid_id;		/* ACL flow add rule ID invalid. */
	atomic64_t acl_flow_add_invalid_sc;		/* ACL flow add rule SC invalid. */
	atomic64_t acl_flow_del_invalid_id;		/* ACL flow del rule ID invalid. */

	/*
	 * Create failures.
	 */
	atomic64_t acl_create_fail;			/* ACL create request failure. */
	atomic64_t acl_create_fail_rule_table_full;	/* ACL rule table full. */
	atomic64_t acl_create_fail_oom;			/* Not able to allocate rule memory. */
	atomic64_t acl_create_fail_max_slices;		/* ACL rule needing more than max slices. */
	atomic64_t acl_create_fail_alloc;		/* ACL rule allocation failure. */
	atomic64_t acl_create_fail_rule_config;		/* ACL rule configuration failure. */
	atomic64_t acl_create_fail_invalid_src;		/* ACL rule create failure due to invalid src. */
	atomic64_t acl_create_fail_invalid_cmn;		/* ACL rule create failure due to invalid common fields. */
	atomic64_t acl_create_fail_fill;		/* ACL rule create failure due to rule to slice mapping. */
	atomic64_t acl_create_fail_policer_sc;		/* ACL rule create failure due to sc table full. */
	atomic64_t acl_create_fail_rule_exist;		/* ACL rule create failure due to collision. */
	atomic64_t acl_create_fail_action_config;	/* ACL rule create failure due to invalid action. */
	atomic64_t acl_create_fail_invalid_id;		/* ACL rule create failure due to invalid rule-ID. */

	/*
	 * Destroy failures.
	 */
	atomic64_t acl_destroy_fail_invalid_id;		/* ACL destroy failure due to invalid rule ID. */
};

/*
 * ppe_acl_stats
 *	Structure for ACL statistics
 */
struct ppe_acl_stats {
	struct ppe_acl_stats_cmn cmn;		/* Common ACL stats */
};

/*
 * ppe_acl_stats_dec()
 *	Decrement stats counter.
 */
static inline void ppe_acl_stats_dec(atomic64_t *stat)
{
	atomic64_dec(stat);
}

/*
 * ppe_acl_stats_inc()
 *	Increment stats counter.
 */
static inline void ppe_acl_stats_inc(atomic64_t *stat)
{
	atomic64_inc(stat);
}

int ppe_acl_stats_debugfs_init(struct dentry *dentry);
void ppe_acl_stats_debugfs_exit(void);

#endif
