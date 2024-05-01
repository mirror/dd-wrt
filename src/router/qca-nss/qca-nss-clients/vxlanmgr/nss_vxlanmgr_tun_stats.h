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

#ifndef __NSS_VXLANMGR_TUN_STATS_H
#define __NSS_VXLANMGR_TUN_STATS_H

/*
 * VxLAN statistic counters
 */
enum nss_vxlanmgr_tun_stats_type {
	NSS_VXLANMGR_TUN_STATS_TYPE_RX_PKTS,
	NSS_VXLANMGR_TUN_STATS_TYPE_RX_BYTES,
	NSS_VXLANMGR_TUN_STATS_TYPE_TX_PKTS,
	NSS_VXLANMGR_TUN_STATS_TYPE_TX_BYTES,
	NSS_VXLANMGR_TUN_STATS_TYPE_RX_QUEUE_0_DROPPED,
	NSS_VXLANMGR_TUN_STATS_TYPE_RX_QUEUE_1_DROPPED,
	NSS_VXLANMGR_TUN_STATS_TYPE_RX_QUEUE_2_DROPPED,
	NSS_VXLANMGR_TUN_STATS_TYPE_RX_QUEUE_3_DROPPED,
	NSS_VXLANMGR_TUN_STATS_TYPE_MAC_DB_LOOKUP_FAILED,
	NSS_VXLANMGR_TUN_STATS_TYPE_EXCEPT_HEADROOM_INSUFFICIENT,
	NSS_VXLANMGR_TUN_STATS_TYPE_EXCEPT_MAC_MOVE,
	NSS_VXLANMGR_TUN_STATS_TYPE_EXCEPT_NO_POLICY_ID,
	NSS_VXLANMGR_TUN_STATS_TYPE_EXCEPT_EXTRA_FLAGS,
	NSS_VXLANMGR_TUN_STATS_TYPE_EXCEPT_VNI_LOOKUP_FAILED,
	NSS_VXLANMGR_TUN_STATS_TYPE_DROP_MALFORMED,
	NSS_VXLANMGR_TUN_STATS_TYPE_DROP_NEXT_NODE_QUEUE_FULL,
	NSS_VXLANMGR_TUN_STATS_TYPE_MAX,
};

/*
 * VxLAN tunnel statistics
 */
struct nss_vxlanmgr_tun_stats {
	uint64_t inner_stats[NSS_VXLANMGR_TUN_STATS_TYPE_MAX];
	uint64_t outer_stats[NSS_VXLANMGR_TUN_STATS_TYPE_MAX];
	uint64_t host_packet_drop;
	uint64_t mac_stats[NSS_VXLAN_MACDB_ENTRIES_MAX][2];
};

/*
 * VxLAN statistics APIs
 */
extern void nss_vxlanmgr_tun_macdb_stats_sync(struct nss_vxlanmgr_tun_ctx *tun_ctx, struct nss_vxlan_msg *nvm);
extern void nss_vxlanmgr_tun_stats_sync(struct nss_vxlanmgr_tun_ctx *tun_ctx, struct nss_vxlan_msg *nvm);
extern void nss_vxlanmgr_tun_stats_deinit(struct nss_vxlanmgr_tun_ctx *tun_ctx);
extern bool nss_vxlanmgr_tun_stats_init(struct nss_vxlanmgr_tun_ctx *tun_ctx);
extern void nss_vxlanmgr_tun_stats_dentry_deinit(void);
extern bool nss_vxlanmgr_tun_stats_dentry_init(void);
extern void nss_vxlanmgr_tun_stats_dentry_remove(struct nss_vxlanmgr_tun_ctx *tun_ctx);
extern bool nss_vxlanmgr_tun_stats_dentry_create(struct nss_vxlanmgr_tun_ctx *tun_ctx);

#endif /* __NSS_VXLANMGR_TUN_STATS_H */
