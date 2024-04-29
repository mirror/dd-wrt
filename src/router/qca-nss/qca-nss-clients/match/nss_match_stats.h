/*
 *******************************************************************************
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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
 ********************************************************************************
 **/

#ifndef __NSS_MATCH_STATS_H
#define __NSS_MATCH_STATS_H

#include <nss_api_if.h>
#include <linux/module.h>
#include <linux/ctype.h>
#include <linux/debugfs.h>

#define NSS_STATS_MAX_STR_LENGTH 96
#define NSS_MATCH_STATS_INSTANCE_MAX NSS_MATCH_STATS_MAX + NSS_MATCH_INSTANCE_RULE_MAX

/*
 * nss_match_stats_type
 * 	Match stats types.
 */
enum nss_match_stats_type {
	NSS_MATCH_STATS_RX_PACKETS,
	NSS_MATCH_STATS_RX_BYTES,
	NSS_MATCH_STATS_TX_PACKETS,
	NSS_MATCH_STATS_TX_BYTES,
	NSS_MATCH_STATS_RX_QUEUE_0_DROP,
	NSS_MATCH_STATS_RX_QUEUE_1_DROP,
	NSS_MATCH_STATS_RX_QUEUE_2_DROP,
	NSS_MATCH_STATS_RX_QUEUE_3_DROP,
	NSS_MATCH_STATS_MAX
};

/*
 * nss_match_db_node_stats
 *	Common node stats for match.
 */
struct nss_match_db_node_stats {
	uint64_t rx_packets;			/* Number of packets received. */
	uint64_t rx_bytes;			/* Number of bytes received. */
	uint64_t tx_packets;			/* Number of packets transmitted. */
	uint64_t tx_bytes;			/* Number of bytes transmitted. */
	uint64_t rx_dropped[NSS_MAX_NUM_PRI];	/* Packets dropped on receive due to queue full. */
};

/*
 * nss_match_stats
 *	Match rule stats.
 */
struct nss_match_stats {
	struct nss_match_db_node_stats pstats;
	uint64_t hit_count[NSS_MATCH_INSTANCE_RULE_MAX];
};

/*
 * NSS match statistics APIs
 */
extern bool nss_match_stats_debugfs_create(struct dentry *match_config);

#endif /* __NSS_MATCH_STATS_H */
