/*
 **************************************************************************
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
 **************************************************************************
 */

#include <nss_api_if.h>
#include "nss_connmgr_tunipip6_priv.h"

/*
 * nss_tunipip6_stats_str
 *	tunipip6 statistics strings for NSS tunnel stats
 */
static int8_t *nss_tunipip6_stats_str[NSS_TUNIPIP6_STATS_MAX] = {
	"rx pkts",
	"rx bytes",
	"tx pkts",
	"tx bytes",
	"rx queue 0 dropped",
	"rx queue 1 dropped",
	"rx queue 2 dropped",
	"rx queue 3 dropped",
	"encap low headroom",
	"encap unhandled_protocol",
	"encap enqueue fail",
	"encap tunnel exist",
	"encap total fmr_count",
	"encap fmr add count",
	"encap fmr del count",
	"encap fmr flush count",
	"encap fmr update_count",
	"encap fmr add_fail count",
	"encap fmr del_fail count",
	"encap error no fmr",
	"encap bmr add count",
	"encap bmr del count",
	"encap error bmr exist",
	"encap error no bmr",
	"decap enqueue fail",
};

/*
 * nss_tunipip6_stats_show()
 *	Read tunipip6 tunnel statistics
 */
static int nss_tunipip6_stats_show(struct seq_file *m, void __attribute__((unused))*p)
{
	int i;
	struct nss_tunipip6_instance *tun_inst;
	struct nss_tunipip6_stats *tunipip6_tunnel_stats;

	tun_inst = vzalloc(sizeof(struct nss_tunipip6_instance));
	if (!tun_inst) {
		nss_tunipip6_warning("Failed to allocate memory for tun_inst\n");
		return -ENOMEM;
	}

	tunipip6_tunnel_stats = vzalloc(sizeof(struct nss_tunipip6_stats));
	if (!tunipip6_tunnel_stats) {
		nss_tunipip6_warning("Failed to allocate memory for tunipip6_tunnel_stats\n");
		vfree(tun_inst);
		return -ENOMEM;
	}

	/*
	 * Copy the tunnel and stats information from the tunnel instance.
	 */
	spin_lock_bh(&tunipip6_ctx.lock);
	memcpy(tun_inst, m->private, sizeof(struct nss_tunipip6_instance));
	memcpy(tunipip6_tunnel_stats, &tun_inst->stats, sizeof(struct nss_tunipip6_stats));
	spin_unlock_bh(&tunipip6_ctx.lock);

	seq_printf(m, "\n\tInner ifnum %u stats:\n", tun_inst->inner_ifnum);
	for (i = 0; i < NSS_TUNIPIP6_STATS_MAX; i++) {
		seq_printf(m, "\t\t%s = %llu\n",
				nss_tunipip6_stats_str[i],
				tunipip6_tunnel_stats->inner_stats[i]);
	}

	seq_printf(m, "\n\tOuter ifnum %u stats:\n", tun_inst->outer_ifnum);
	for (i = 0; i < NSS_TUNIPIP6_STATS_MAX; i++) {
		seq_printf(m, "\t\t%s = %llu\n",
				nss_tunipip6_stats_str[i],
				tunipip6_tunnel_stats->outer_stats[i]);
	}

	seq_printf(m, "\n%s tunnel stats end\n\n", tun_inst->dev->name);
	vfree(tun_inst);
	vfree(tunipip6_tunnel_stats);
	return 0;
}

/*
 * nss_tunipip6_stats_open()
 */
static int nss_tunipip6_stats_open(struct inode *inode, struct file *file)
{
	return single_open(file, nss_tunipip6_stats_show, inode->i_private);
}

/*
 * nss_tunipip6_stats_update()
 *	Update inner or outer node statistics
 */
static void nss_tunipip6_stats_update(uint64_t *stats, struct nss_tunipip6_stats_sync_msg *stats_msg)
{
	uint32_t i, *src;
	uint64_t *dest = stats;

	src = &stats_msg->node_stats.rx_packets;
	for (i = NSS_TUNIPIP6_STATS_RX_PKTS; i < NSS_TUNIPIP6_STATS_MAX; i++, src++, dest++) {
		*dest += *src;
	}
}

/*
 * nss_tunipip6_stats_sync()
 *	Sync function for tunipip6 statistics
 */
void nss_tunipip6_stats_sync(struct net_device *dev, struct nss_tunipip6_msg *ntm)
{
	uint32_t ifnum = ntm->cm.interface;
	struct nss_tunipip6_stats_sync_msg *stats = &ntm->msg.stats_sync;
	struct nss_tunipip6_instance *ntii;
	struct nss_tunipip6_stats *s;

	spin_lock_bh(&tunipip6_ctx.lock);
	ntii = nss_tunipip6_find_instance(dev);
	if (!ntii) {
		spin_unlock_bh(&tunipip6_ctx.lock);
		nss_tunipip6_warning("%px: Not able to find context for device: %s\n", dev, dev->name);
		return;
	}

	s = &ntii->stats;
	if (ntii->inner_ifnum == ifnum) {
		nss_tunipip6_stats_update(s->inner_stats, stats);
	} else if (ntii->outer_ifnum == ifnum) {
		nss_tunipip6_stats_update(s->outer_stats, stats);
	} else {
		nss_tunipip6_warning("%px: Netdev=%s invalid interface number. Interface No: %u\n", dev, dev->name, ifnum);
	}

	spin_unlock_bh(&tunipip6_ctx.lock);
}

/*
 * nss_tunipip6_stats_ops
 *	File operations for tunipip6 tunnel stats
 */
static const struct file_operations nss_tunipip6_stats_ops = { \
	.open = nss_tunipip6_stats_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release
};

/*
 * nss_tunipip6_stats_dentry_destroy()
 *	Remove debufs file for given tunnel.
 */
void nss_tunipip6_stats_dentry_destroy(struct nss_tunipip6_instance *tun_inst)
{
	debugfs_remove(tun_inst->dentry);
}

/*
 * nss_tunipip6_stats_dentry_create()
 *	Create dentry for a given tunnel.
 */
bool nss_tunipip6_stats_dentry_create(struct nss_tunipip6_instance *tun_inst)
{
	char dentry_name[IFNAMSIZ];

	scnprintf(dentry_name, sizeof(dentry_name), "%s", tun_inst->dev->name);
	tun_inst->dentry = debugfs_create_file(dentry_name, S_IRUGO,
			tunipip6_ctx.tunipip6_dentry_dir, tun_inst, &nss_tunipip6_stats_ops);
	if (!tun_inst->dentry) {
		nss_tunipip6_warning("Debugfs file creation failed for tun %s\n", tun_inst->dev->name);
		return false;
	}

	return true;
}

/*
 * nss_tunipip6_stats_dentry_deinit()
 *	Cleanup the debugfs tree.
 */
void nss_tunipip6_stats_dentry_deinit(void)
{
	if (tunipip6_ctx.tunipip6_dentry_dir) {
		debugfs_remove_recursive(tunipip6_ctx.tunipip6_dentry_dir);
	}
}

/*
 * nss_tunipip6_stats_dentry_init()
 *	Create tunipip6 tunnel statistics debugfs entry.
 */
bool nss_tunipip6_stats_dentry_init(void)
{
	/*
	 * Initialize debugfs directory.
	 */
	tunipip6_ctx.tunipip6_dentry_dir = debugfs_create_dir("qca-nss-tunipip6", NULL);
	if (!tunipip6_ctx.tunipip6_dentry_dir) {
		nss_tunipip6_warning("Failed to create debug entry for subsystem: qca-nss-tunipip6\n");
		return false;
	}

	return true;
}
