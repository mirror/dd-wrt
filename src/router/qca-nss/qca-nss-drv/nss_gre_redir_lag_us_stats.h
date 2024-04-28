/*
 ******************************************************************************
 * Copyright (c) 2018, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * ****************************************************************************
 */

#ifndef __NSS_GRE_REDIR_LAG_US_STATS_H__
#define __NSS_GRE_REDIR_LAG_US_STATS_H__

/*
 * GRE redirect LAG upstream statistics
 */
enum nss_gre_redir_lag_us_stats_types {
	NSS_GRE_REDIR_LAG_US_STATS_AMSDU_PKTS = NSS_STATS_NODE_MAX,
	NSS_GRE_REDIR_LAG_US_STATS_AMSDU_PKTS_ENQUEUED,
	NSS_GRE_REDIR_LAG_US_STATS_AMSDU_PKTS_EXCEPTIONED,
	NSS_GRE_REDIR_LAG_US_STATS_EXCEPTIONED,
	NSS_GRE_REDIR_LAG_US_STATS_FREED,
	NSS_GRE_REDIR_LAG_US_STATS_ADD_ATTEMPT,
	NSS_GRE_REDIR_LAG_US_STATS_ADD_SUCCESS,
	NSS_GRE_REDIR_LAG_US_STATS_ADD_FAIL_TABLE_FULL,
	NSS_GRE_REDIR_LAG_US_STATS_ADD_FAIL_EXISTS,
	NSS_GRE_REDIR_LAG_US_STATS_DEL_ATTEMPT,
	NSS_GRE_REDIR_LAG_US_STATS_DEL_SUCCESS,
	NSS_GRE_REDIR_LAG_US_STATS_DEL_FAIL_NOT_FOUND,
	NSS_GRE_REDIR_LAG_US_STATS_MAX,
};

extern struct dentry *nss_gre_redir_lag_us_stats_dentry_create(void);
#endif
