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

#ifndef __PPE_MIRROR_STATS_H
#define __PPE_MIRROR_STATS_H

/*
 * ppe_mirror_stats
 *	PPE mirror match counters
 */
struct ppe_mirror_stats {
	atomic64_t packets;
	atomic64_t bytes;
};

/*
 * ppe_mirror_cmn_stats
 *	Message structure for ppe mirror common stats
 */
struct ppe_mirror_cmn_stats {
	/*
	 * Common success counts.
	 */
	atomic64_t acl_mapping_add_req;			/* ACL mapping add request */
	atomic64_t acl_mapping_del_req;			/* ACL mapping delete request */
	atomic64_t acl_mapping_add_success;		/* ACL mapping add success count */
	atomic64_t acl_mapping_del_success;		/* ACL mapping delete success count */
	atomic64_t acl_mapping_count;			/* Total ACL to netdevice mapping count */

	/*
	 * Common failure counts.
	 */
	atomic64_t acl_mapping_add_fail_invalid_rule_id;	/* Invalid rule ID for add */
	atomic64_t acl_mapping_add_fail_rule_not_found;		/* No ACL rule found for mapping request */
	atomic64_t acl_mapping_add_map_exist;			/* ACL mapping addition failure due to collision */
	atomic64_t acl_mapping_add_map_nomem;			/* ACL mapping addition failure due to Memory issue */
	atomic64_t acl_mapping_add_invalid_group_info;		/* ACL mapping addition failure due to Memory issue */
	atomic64_t acl_mapping_del_fail_invalid_rule_id;	/* Invalid rule ID for delete */
	atomic64_t acl_mapping_del_fail_rule_not_found;		/* No ACL rule found for delete mapping request */
	atomic64_t acl_mapping_del_fail_map_not_found;		/* ACL mapping deletion failure as mapping not found */
	atomic64_t acl_mapping_del_fail_group_not_found;	/* ACL mapping deletion failure as group not found */
	atomic64_t acl_mirror_process_mapping_invalid;		/* ACL mapping not found for mirrored packets */
	atomic64_t acl_mirror_process_group_invalid;		/* ACL group is not found for mirrored packets */

	/*
	 * Capture core related counts.
	 */
	atomic64_t acl_mapping_invalid_capture_core;	/* ACL mapping invalid capture core received. */
	atomic64_t acl_mapping_fail_en_capture_core;	/* ACL capture core en failed. */
};

/*
 * ppe_mirror_stats_dec()
 *	Decrement stats counter.
 */
static inline void ppe_mirror_stats_dec(atomic64_t *stat)
{
	atomic64_dec(stat);
}

/*
 * ppe_mirror_stats_inc()
 *	Increment stats counter.
 */
static inline void ppe_mirror_stats_inc(atomic64_t *stat)
{
	atomic64_inc(stat);
}

/*
 * ppe_mirror_update_stats()
 *	Update packets and bytes stats.
 */
static inline void ppe_mirror_update_stats(struct ppe_mirror_stats *stats, uint32_t bytes) {
	atomic64_inc(&stats->packets);
	atomic64_add(bytes, &stats->bytes);
}

int ppe_mirror_stats_debugfs_init(struct dentry *dentry);
void ppe_mirror_stats_debugfs_exit(void);

#endif
