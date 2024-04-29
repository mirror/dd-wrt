/*
 ******************************************************************************
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
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

#ifndef __NSS_GRE_REDIR_MARK_STATS_H__
#define __NSS_GRE_REDIR_MARK_STATS_H__

/*
 * GRE REDIR statistics types
 */
enum nss_gre_redir_mark_stats_types {
	NSS_GRE_REDIR_MARK_STATS_TX_PKTS,
	NSS_GRE_REDIR_MARK_STATS_TX_BYTES,
	NSS_GRE_REDIR_MARK_STATS_RX_PKTS,
	NSS_GRE_REDIR_MARK_STATS_RX_BYTES,
	NSS_GRE_REDIR_MARK_STATS_RX_DROPS,
	NSS_GRE_REDIR_MARK_STATS_HLOS_MAGIC_FAILED,
	NSS_GRE_REDIR_MARK_STATS_INV_DST_IF_DROPS,
	NSS_GRE_REDIR_MARK_STATS_DST_IF_ENQUEUE,
	NSS_GRE_REDIR_MARK_STATS_DST_IF_ENQUEUE_DROPS,
	NSS_GRE_REDIR_MARK_STATS_INV_APPID,
	NSS_GRE_REDIR_MARK_STATS_HEADROOM_UNAVAILABLE,
	NSS_GRE_REDIR_MARK_STATS_TX_COMPLETION_SUCCESS,
	NSS_GRE_REDIR_MARK_STATS_TX_COMPLETION_DROPS,
	NSS_GRE_REDIR_MARK_STATS_MAX
};

/*
 * NSS core stats -- for H2N/N2H gre_redir_mark debug stats
 */
struct nss_gre_redir_mark_stats {
	uint64_t stats[NSS_GRE_REDIR_MARK_STATS_MAX];
};

/*
 * NSS GRE REDIR Mark statistics APIs
 */
extern struct dentry *nss_gre_redir_mark_stats_dentry_create(void);

#endif /* __NSS_GRE_REDIR_MARK_STATS_H__ */
