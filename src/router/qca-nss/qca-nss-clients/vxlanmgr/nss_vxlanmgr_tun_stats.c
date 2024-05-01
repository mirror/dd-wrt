/*
 **************************************************************************
 * Copyright (c) 2019-2020, The Linux Foundation. All rights reserved.
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
 **************************************************************************
 */

#include <linux/debugfs.h>
#include <linux/etherdevice.h>
#include <linux/netdevice.h>
#include <nss_api_if.h>
#include "nss_vxlanmgr.h"
#include "nss_vxlanmgr_tun_stats.h"

/*
 * VxLAN context
 */
extern struct nss_vxlanmgr_ctx vxlan_ctx;

/*
 * nss_vxlanmgr_tun_stats_str
 *	Vxlan statistics strings for nss tunnel stats
 */
static int8_t *nss_vxlanmgr_tun_stats_str[NSS_VXLANMGR_TUN_STATS_TYPE_MAX] = {
	"rx_pkts",
	"rx_bytes",
	"tx_pkts",
	"tx_bytes",
	"rx_queue_0_dropped",
	"rx_queue_1_dropped",
	"rx_queue_2_dropped",
	"rx_queue_3_dropped",
	"Except MAC DB look up failed",
	"Except Insufficient Headroom",
	"Except MAC moved",
	"Except No Policy ID",
	"Except Extra flags",
	"Except VNI Look-up failed",
	"Dropped packet malformed",
	"Dropped next node queue is full",
};

/*
 * nss_vxlanmgr_tun_stats_show()
 *	Read Vxlan Tunnel statistics
 */
static int nss_vxlanmgr_tun_stats_show(struct seq_file *m, void __attribute__((unused))*p)
{
	int i;
	struct nss_vxlanmgr_tun_ctx *tun_ctx;
	struct nss_vxlanmgr_tun_stats *vxlan_tunnel_stats;

	tun_ctx = kzalloc(sizeof(struct nss_vxlanmgr_tun_ctx), GFP_KERNEL);
	if (!tun_ctx) {
		nss_vxlanmgr_warn("Failed to allocate memory for tun_ctx\n");
		return -ENOMEM;
	}

	vxlan_tunnel_stats = kzalloc(sizeof(struct nss_vxlanmgr_tun_stats), GFP_KERNEL);
	if (!vxlan_tunnel_stats) {
		nss_vxlanmgr_warn("Failed to allocate memory for vxlan_tunnel_stats\n");
		kfree(tun_ctx);
		return -ENOMEM;
	}

	spin_lock_bh(&vxlan_ctx.tun_lock);
	memcpy(tun_ctx, m->private, sizeof(struct nss_vxlanmgr_tun_ctx));
	memcpy(vxlan_tunnel_stats, tun_ctx->stats, sizeof(struct nss_vxlanmgr_tun_stats));
	spin_unlock_bh(&vxlan_ctx.tun_lock);

	/*
	 * Tunnel stats
	 */
	seq_printf(m, "\n%s tunnel stats start:\n", tun_ctx->dev->name);

	seq_printf(m, "\t%s configuration:\n", tun_ctx->dev->name);
	seq_printf(m, "\t\tvni = %u\n", tun_ctx->vni);
	seq_printf(m, "\t\ttunnel_flags = %x\n", tun_ctx->tunnel_flags);
	seq_printf(m, "\t\tflow_label = %u\n", tun_ctx->flow_label);
	seq_printf(m, "\t\tsrc_port_min = %u\n", tun_ctx->src_port_min);
	seq_printf(m, "\t\tsrc_port_max = %u\n", tun_ctx->src_port_max);
	seq_printf(m, "\t\tdest_port = %u\n", ntohs(tun_ctx->dest_port));
	seq_printf(m, "\t\ttos = %u\n", tun_ctx->tos);
	seq_printf(m, "\t\tttl = %u\n", tun_ctx->ttl);

	seq_printf(m, "\n\tInner ifnum %u stats:\n", tun_ctx->inner_ifnum);
	for (i = 0; i < NSS_VXLANMGR_TUN_STATS_TYPE_MAX; i++) {
		seq_printf(m, "\t\t%s = %llu\n",
				nss_vxlanmgr_tun_stats_str[i],
				vxlan_tunnel_stats->inner_stats[i]);
	}

	seq_printf(m, "\n\tOuter ifnum %u stats:\n", tun_ctx->outer_ifnum);
	for (i = 0; i < NSS_VXLANMGR_TUN_STATS_TYPE_MAX; i++) {
		seq_printf(m, "\t\t%s = %llu\n",
				nss_vxlanmgr_tun_stats_str[i],
				vxlan_tunnel_stats->outer_stats[i]);
	}

	seq_printf(m, "\n\tMAC DB stats:\n");
	for (i = 0; i < NSS_VXLAN_MACDB_ENTRIES_MAX; i++) {
		if (!vxlan_tunnel_stats->mac_stats[i][0]) {
			continue;
		}
		seq_printf(m, "\t\t%pM = %llu\n",
				&vxlan_tunnel_stats->mac_stats[i][0],
				vxlan_tunnel_stats->mac_stats[i][1]);
	}
	seq_printf(m, "\n\tPackets dropped at host: %llu\n",
				vxlan_tunnel_stats->host_packet_drop);

	seq_printf(m, "\n%s tunnel stats end\n\n", tun_ctx->dev->name);
	kfree(tun_ctx);
	kfree(vxlan_tunnel_stats);
	return 0;
}

/*
 * nss_vxlanmgr_tun_stats_open()
 */
static int nss_vxlanmgr_tun_stats_open(struct inode *inode, struct file *file)
{
	return single_open(file, nss_vxlanmgr_tun_stats_show, inode->i_private);
}

/*
 * nss_vxlanmgr_tun_stats_update()
 *	Update inner and outer node statistics
 */
void nss_vxlanmgr_tun_stats_update(uint64_t *stats, struct nss_vxlan_stats_msg *stats_msg)
{
	uint32_t i;

	stats[NSS_VXLANMGR_TUN_STATS_TYPE_RX_PKTS] += stats_msg->node_stats.rx_packets;
	stats[NSS_VXLANMGR_TUN_STATS_TYPE_RX_BYTES] += stats_msg->node_stats.rx_bytes;
	stats[NSS_VXLANMGR_TUN_STATS_TYPE_TX_PKTS] += stats_msg->node_stats.tx_packets;
	stats[NSS_VXLANMGR_TUN_STATS_TYPE_TX_BYTES] += stats_msg->node_stats.tx_bytes;

	for (i = 0; i < NSS_MAX_NUM_PRI; i++) {
		stats[NSS_VXLANMGR_TUN_STATS_TYPE_RX_QUEUE_0_DROPPED + i] += stats_msg->node_stats.rx_dropped[i];
	}

	stats[NSS_VXLANMGR_TUN_STATS_TYPE_MAC_DB_LOOKUP_FAILED] +=
		stats_msg->except_mac_db_lookup_failed;
	stats[NSS_VXLANMGR_TUN_STATS_TYPE_EXCEPT_HEADROOM_INSUFFICIENT] +=
		stats_msg->except_low_hroom;
	stats[NSS_VXLANMGR_TUN_STATS_TYPE_EXCEPT_MAC_MOVE] +=
		stats_msg->except_mac_move;
	stats[NSS_VXLANMGR_TUN_STATS_TYPE_EXCEPT_NO_POLICY_ID] +=
		stats_msg->except_no_policy_id;
	stats[NSS_VXLANMGR_TUN_STATS_TYPE_EXCEPT_EXTRA_FLAGS] +=
		stats_msg->except_extra_vxlan_hdr_flags;
	stats[NSS_VXLANMGR_TUN_STATS_TYPE_EXCEPT_VNI_LOOKUP_FAILED] +=
		stats_msg->except_vni_lookup_failed;
	stats[NSS_VXLANMGR_TUN_STATS_TYPE_DROP_MALFORMED] +=
		stats_msg->dropped_malformed;
	stats[NSS_VXLANMGR_TUN_STATS_TYPE_DROP_NEXT_NODE_QUEUE_FULL] +=
		stats_msg->dropped_next_node_queue_full;
}

/*
 * nss_vxlanmgr_tun_macdb_stats_sync()
 *	Sync function for vxlan fdb entries
 */
void nss_vxlanmgr_tun_macdb_stats_sync(struct nss_vxlanmgr_tun_ctx *tun_ctx, struct nss_vxlan_msg *nvm)
{
	struct nss_vxlan_macdb_stats_msg *db_stats;
	struct nss_vxlanmgr_tun_stats *s = tun_ctx->stats;
	uint16_t i, j, nentries;

	db_stats = &nvm->msg.db_stats;
	nentries = db_stats->cnt;

	dev_hold(tun_ctx->dev);

	if (nentries > NSS_VXLAN_MACDB_ENTRIES_PER_MSG) {
		nss_vxlanmgr_warn("%px: No more than 20 entries allowed per message.\n", tun_ctx->dev);
		dev_put(tun_ctx->dev);
		return;
	}

	for (j = 0; j < nentries; j++) {
		if (!db_stats->entry[j].hits) {
			continue;
		}
		for (i = 0; i < NSS_VXLAN_MACDB_ENTRIES_MAX; i++) {
			if (ether_addr_equal((uint8_t *)&s->mac_stats[i][0],
						(uint8_t *)db_stats->entry[j].mac)) {
				s->mac_stats[i][1] += db_stats->entry[j].hits;
				break;
			}
		}
	}
	dev_put(tun_ctx->dev);
}

/*
 * nss_vxlanmgr_tun_stats_sync()
 *	Sync function for vxlan statistics
 */
void nss_vxlanmgr_tun_stats_sync(struct nss_vxlanmgr_tun_ctx *tun_ctx, struct nss_vxlan_msg *nvm)
{
	uint32_t ifnum = nvm->cm.interface;
	struct nss_vxlan_stats_msg *stats = &nvm->msg.stats;
	struct nss_vxlanmgr_tun_stats *s = tun_ctx->stats;

	if (tun_ctx->inner_ifnum == ifnum) {
		nss_vxlanmgr_tun_stats_update(s->inner_stats, stats);
	} else if (tun_ctx->outer_ifnum == ifnum) {
		nss_vxlanmgr_tun_stats_update(s->outer_stats, stats);
	} else {
		nss_vxlanmgr_warn("Invalid interface number\n");
	}
}

/*
 * nss_vxlanmgr_tun_stats_ops
 *	File operations for VxLAN tunnel stats
 */
static const struct file_operations nss_vxlanmgr_tun_stats_ops = { \
	.open = nss_vxlanmgr_tun_stats_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release
};

/*
 * nss_vxlanmgr_tun_stats_deinit()
 *	Remove the stats entry for the given interface number.
 */
void nss_vxlanmgr_tun_stats_deinit(struct nss_vxlanmgr_tun_ctx *tun_ctx)
{
	struct nss_vxlanmgr_tun_stats *stats_stats = tun_ctx->stats;

	spin_lock_bh(&vxlan_ctx.tun_lock);
	kfree(stats_stats);
	tun_ctx->stats = NULL;
	spin_unlock_bh(&vxlan_ctx.tun_lock);
}

/*
 * nss_vxlanmgr_tun_stats_init()
 *	Alloc and initialize tunnel debug stats.
 */
bool nss_vxlanmgr_tun_stats_init(struct nss_vxlanmgr_tun_ctx *tun_ctx)
{
	struct nss_vxlanmgr_tun_stats *stats_stats;

	stats_stats = kzalloc(sizeof(struct nss_vxlanmgr_tun_stats), GFP_ATOMIC);
	if (!stats_stats) {
		nss_vxlanmgr_warn("Failed to allocate memory for stats_stats\n");
		return false;
	}

	tun_ctx->stats = stats_stats;
	return true;
}

/*
 * nss_vxlanmgr_tun_stats_dentry_remove()
 *	Remove debufs file for given tunnel context.
 */
void nss_vxlanmgr_tun_stats_dentry_remove(struct nss_vxlanmgr_tun_ctx *tun_ctx)
{
	debugfs_remove(tun_ctx->dentry);
}

/*
 * nss_vxlanmgr_tun_stats_dentry_create()
 *	Create dentry for a given tunnel.
 */
bool nss_vxlanmgr_tun_stats_dentry_create(struct nss_vxlanmgr_tun_ctx *tun_ctx)
{
	char dentry_name[IFNAMSIZ];

	scnprintf(dentry_name, sizeof(dentry_name), "%s", tun_ctx->dev->name);
	tun_ctx->dentry = debugfs_create_file(dentry_name, S_IRUGO,
			tun_ctx->vxlan_ctx->dentry, tun_ctx, &nss_vxlanmgr_tun_stats_ops);
	if (!tun_ctx->dentry) {
		nss_vxlanmgr_warn("Debugfs file creation failed for tun %s\n", tun_ctx->dev->name);
		return false;
	}
	return true;
}

/*
 * nss_vxlanmgr_tun_stats_dentry_deinit()
 *	Cleanup the debugfs tree.
 */
void nss_vxlanmgr_tun_stats_dentry_deinit()
{
	debugfs_remove_recursive(vxlan_ctx.dentry);
}

/*
 * nss_vxlanmgr_tun_stats_dentry_init()
 *	Create VxLAN tunnel statistics debugfs entry.
 */
bool nss_vxlanmgr_tun_stats_dentry_init()
{
	/*
	 * initialize debugfs.
	 */
	vxlan_ctx.dentry = debugfs_create_dir("qca-nss-vxlanmgr", NULL);
	if (!vxlan_ctx.dentry) {
		nss_vxlanmgr_warn("Creating debug directory failed\n");
		return false;
	}
	return true;
}
