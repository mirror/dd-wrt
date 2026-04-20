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

#ifndef __PPE_POLICER_STATS_H
#define __PPE_POLICER_STATS_H

#define PPE_POLICER_STATS_NODE_NAME		"PPE_POLICER"

/*
 * ppe_policer_stats
 *	Message structure for ppe general stats
 */
struct ppe_policer_stats {
	atomic64_t port_rule_id_invalid;		/* Port Policer create rule success */
	atomic64_t port_rule_not_found;			/* Port Policer create rule success */
	atomic64_t acl_rule_id_invalid;			/* Port Policer create rule success */
	atomic64_t acl_rule_not_found;			/* Port Policer create rule success */
	atomic64_t policer_destroy_fail_invalid_id;
	atomic64_t acl_create_fail_oom;			/* Port Policer destroy rule success */
	atomic64_t acl_create_fail_rule_table_full;		/* Port Policer destroy rule fail */
	atomic64_t policer_acl_create_req;			/* acl Policer destroy rule success */
	atomic64_t policer_acl_already_exists;			/* acl policer already exists */
	atomic64_t port_create_fail_oom;			/* Port Policer destroy rule success */
	atomic64_t port_create_fail_rule_table_full;		/* Port Policer destroy rule fail */
	atomic64_t policer_port_create_req;			/* acl Policer destroy rule success */
	atomic64_t create_port_policer_success;		/* Port Policer create rule success */
	atomic64_t create_port_policer_failed;		/* Port Policer create rule fail */
	atomic64_t create_acl_policer_success;		/* acl Policer create rule success */
	atomic64_t create_acl_policer_failed;		/* acl Policer create rule fail */
	atomic64_t destroy_port_policer_success;	/* Port Policer destroy rule success */
	atomic64_t destroy_port_policer_failed;		/* Port Policer destroy rule fail */
	atomic64_t destroy_acl_policer_success;		/* acl Policer destroy rule success */
	atomic64_t destroy_acl_policer_failed;		/* acl Policer destroy rule fail */
	atomic64_t v4_create_ppe_rule_flow_policer;		/* Policer create rule request */
	atomic64_t v4_create_ppe_rule_fail;		/* Policer create rule fail */
	atomic64_t v4_destroy_ppe_rule_fail;		/* Policer create rule fail */
	atomic64_t v6_create_ppe_rule_fail;		/* Policer destroy rule request */
	atomic64_t v6_create_ppe_rule_flow_policer;		/* Policer create rule request */
	atomic64_t v6_destroy_ppe_rule_fail;		/* Policer destroy rule fail */
};

/*
 * ppe_policer_stats_dec()
 *	Decrement stats counter.
 */
static inline void ppe_policer_stats_dec(atomic64_t *stat)
{
	atomic64_dec(stat);
}

/*
 * ppe_policer_stats_inc()
 *	Increment stats counter.
 */
static inline void ppe_policer_stats_inc(atomic64_t *stat)
{
	atomic64_inc(stat);
}

int ppe_policer_stats_debugfs_init(struct dentry *dentry);
void ppe_policer_stats_debugfs_exit(void);

#endif
