/*
 **************************************************************************
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
 **************************************************************************
 */

/*
 * nss_ovpnmgr_debugfs.c
 *	Debugfs implementation for OVPN.
 */
#include <linux/debugfs.h>
#include <linux/crypto.h>
#include <linux/list.h>
#include <linux/proc_fs.h>

#include <nss_api_if.h>
#include <nss_qvpn.h>
#include "nss_ovpnmgr.h"
#include "nss_ovpnmgr_crypto.h"
#include "nss_ovpnmgr_tun.h"
#include "nss_ovpnmgr_app.h"
#include "nss_ovpnmgr_debugfs.h"
#include "nss_ovpnmgr_priv.h"
#include "nss_ovpnmgr_route.h"

/*
 * nss_ovpnmgr_debugfs_tun_info_show()
 */
static void nss_ovpnmgr_debugfs_tun_info_show(struct nss_ovpnmgr_tun *tun, struct seq_file *m)
{
	int i;

	seq_printf(m, "\tTunnel ID: %u\n", tun->tunnel_id);
	seq_printf(m, "\t\tNetdev: %s\n", tun->dev->name);
	seq_printf(m, "\t\tEncap ifnum: %u\n", tun->inner.ifnum);
	seq_printf(m, "\t\tDecap ifnum: %u\n", tun->outer.ifnum);
	seq_printf(m, "\t\tOVPN Flags: %x\n", tun->tun_cfg.flags);

	if (tun->tun_cfg.flags & NSS_OVPNMGR_HDR_FLAG_IPV6) {
		seq_printf(m, "\t\tSrc IP: %pI6c\n", &tun->tun_hdr.src_ip[0]);
		seq_printf(m, "\t\tDst IP: %pI6c\n", &tun->tun_hdr.dst_ip[0]);
	} else {
		seq_printf(m, "\t\tSrc IP: %pI4n\n", &tun->tun_hdr.src_ip[0]);
		seq_printf(m, "\t\tDst IP: %pI4n\n", &tun->tun_hdr.dst_ip[0]);
	}

	if (!(tun->tun_cfg.flags & NSS_OVPNMGR_HDR_FLAG_L4_PROTO_TCP)) {
		seq_printf(m, "\t\tL4 Protocol: UDP\n");
	}

	seq_printf(m, "\t\tSrc Port: %u\n", ntohs(tun->tun_hdr.src_port));
	seq_printf(m, "\t\tDst Port: %u\n", ntohs(tun->tun_hdr.dst_port));
	seq_printf(m, "\t\tCrypto Session: (Inner)\n");
	seq_printf(m, "\t\t\tActive: \n");
	seq_printf(m, "\t\t\t\tIndex: %u\n", tun->inner.active.crypto_idx);
	seq_printf(m, "\t\t\t\tKeyId: %u\n", tun->inner.active.key_id);
	seq_printf(m, "\t\t\tExpiring: \n");
	seq_printf(m, "\t\t\t\tIndex: %u\n", tun->inner.expiring.crypto_idx);
	seq_printf(m, "\t\t\t\tKeyId: %u\n", tun->inner.expiring.key_id);
	seq_printf(m, "\t\tCrypto Session: (Outer)\n");
	seq_printf(m, "\t\t\tActive: \n");
	seq_printf(m, "\t\t\t\tIndex: %u\n", tun->outer.active.crypto_idx);
	seq_printf(m, "\t\t\t\tKeyId: %u\n", tun->outer.active.key_id);
	seq_printf(m, "\t\t\tExpiring: \n");
	seq_printf(m, "\t\t\t\tIndex: %u\n", tun->outer.expiring.crypto_idx);
	seq_printf(m, "\t\t\t\tKeyId: %u\n", tun->outer.expiring.key_id);

	seq_puts(m, "\tStatistics:\n\t\tEncap:\n");

	seq_printf(m, "\t\t\trx_packets: %llu\n", tun->inner.stats.rx_packets);
	seq_printf(m, "\t\t\trx_bytes: %llu\n", tun->inner.stats.rx_bytes);
	seq_printf(m, "\t\t\ttx_packets: %llu\n", tun->inner.stats.tx_packets);
	seq_printf(m, "\t\t\ttx_bytes: %llu\n", tun->inner.stats.tx_bytes);

	for (i = 0 ; i < NSS_MAX_NUM_PRI; i++) {
		seq_printf(m, "\t\t\trx_queue_%d_dropped: %llu\n", i, tun->inner.stats.rx_dropped[i]);
	}

	for (i = 0; i < NSS_CRYPTO_CMN_RESP_ERROR_MAX; i++) {
		seq_printf(m, "\t\t\tfail_crypto[%d]: %llu\n", i, tun->inner.stats.fail_crypto[i]);
	}

	for (i = 0; i < NSS_QVPN_PKT_DROP_EVENT_MAX; i++) {
		seq_printf(m, "\t\t\tfail_offload[%d]: %llu\n", i, tun->inner.stats.fail_offload[i]);
	}

	seq_printf(m, "\t\t\thost_pkt_drops: %llu\n", tun->inner.stats.host_pkt_drop);

	for (i = 0; i < NSS_QVPN_EXCEPTION_EVENT_MAX; i++) {
		seq_printf(m, "\t\t\texception[%d]: %llu\n", i, tun->inner.stats.exception[i]);
	}

	seq_puts(m, "\t\tDecap:\n");

	seq_printf(m, "\t\t\trx_packets: %llu\n", tun->outer.stats.rx_packets);
	seq_printf(m, "\t\t\trx_bytes: %llu\n", tun->outer.stats.rx_bytes);
	seq_printf(m, "\t\t\ttx_packets: %llu\n", tun->outer.stats.tx_packets);
	seq_printf(m, "\t\t\ttx_bytes: %llu\n", tun->outer.stats.tx_bytes);

	for (i = 0 ; i < NSS_MAX_NUM_PRI; i++) {
		seq_printf(m, "\t\t\trx_queue_%d_dropped: %llu\n", i, tun->outer.stats.rx_dropped[i]);
	}

	for (i = 0; i < NSS_CRYPTO_CMN_RESP_ERROR_MAX; i++) {
		seq_printf(m, "\t\t\tfail_crypto[%d]: %llu\n", i, tun->outer.stats.fail_crypto[i]);
	}

	for (i = 0; i < NSS_QVPN_PKT_DROP_EVENT_MAX; i++) {
		seq_printf(m, "\t\t\tfail_offload[%d]: %llu\n", i, tun->outer.stats.fail_offload[i]);
	}

	seq_printf(m, "\t\t\thost_pkt_drops: %llu\n", tun->outer.stats.host_pkt_drop);

	for (i = 0; i < NSS_QVPN_EXCEPTION_EVENT_MAX; i++) {
		seq_printf(m, "\t\t\texception[%d]: %llu\n", i, tun->outer.stats.exception[i]);
	}

	seq_putc(m, '\n');
}

/*
 * nss_ovpnmgr_debugfs_tun_list_show()
 */
static int nss_ovpnmgr_debugfs_tun_list_show(struct seq_file *m, void *p)
{
	struct nss_ovpnmgr_app *app = m->private;
	struct nss_ovpnmgr_tun *tun;

	read_lock_bh(&ovpnmgr_ctx.lock);

	list_for_each_entry(tun, &app->tun_list, list) {
		nss_ovpnmgr_debugfs_tun_info_show(tun, m);
	}

	read_unlock_bh(&ovpnmgr_ctx.lock);
	return 0;
}

/*
 * nss_ovpnmgr_debugfs_tun_list_open()
 */
static int nss_ovpnmgr_debugfs_tun_list_open(struct inode *inode, struct file *file)
{
	return single_open(file, nss_ovpnmgr_debugfs_tun_list_show, inode->i_private);
}

/*
 * tun file operation structure instance
 */
static const struct file_operations tun_list_op = {
	.open = nss_ovpnmgr_debugfs_tun_list_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release
};

/*
 * nss_ovpnmgr_debugfs_init()
 *	Initialize the debugfs tree.
 */
int nss_ovpnmgr_debugfs_init(void)
{
	/*
	 * initialize debugfs.
	 */
	ovpnmgr_ctx.dentry = debugfs_create_dir("qca-nss-ovpnmgr", NULL);
	if (!ovpnmgr_ctx.dentry) {
		nss_ovpnmgr_warn("Creating debug directory failed\n");
		return -1;
	}

	return 0;
}

/*
 * nss_ovpnmgr_debugfs_create()
 *	Create debufs file for registered OVPN application.
 */
void nss_ovpnmgr_debugfs_create(struct nss_ovpnmgr_app *app)
{
	char dentry_name[NSS_OVPNMGR_DEBUGFS_MAX_NAME_SIZE];

	scnprintf(dentry_name, sizeof(dentry_name), "ovpn_app_%s", app->dev->name);

	/*
	 * Create debugfs entries for SA, flow and subnet
	 */
	app->dentry = debugfs_create_file(dentry_name, S_IRUGO, ovpnmgr_ctx.dentry, app, &tun_list_op);
	if (!app->dentry) {
		nss_ovpnmgr_warn("Debugfs file creation failed for tun\n");
	}
}

/*
 * nss_ovpnmgr_debugfs_remove()
 *	Remove debufs file for registered OVPN application.
 */
void nss_ovpnmgr_debugfs_remove(struct dentry *dentry)
{
	debugfs_remove(dentry);
}

/*
 * nss_ovpnmgr_debugfs_cleanup()
 *	Cleanup the debugfs tree.
 */
void nss_ovpnmgr_debugfs_cleanup(void)
{
	debugfs_remove_recursive(ovpnmgr_ctx.dentry);
}
