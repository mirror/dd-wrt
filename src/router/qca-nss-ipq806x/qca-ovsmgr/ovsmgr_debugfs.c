/*
 **************************************************************************
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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
 * ovsmgr_debugfs.c
 *	Debugfs implementation for OVS datapath.
 */
#include <linux/debugfs.h>
#include <linux/list.h>
#include <linux/proc_fs.h>
#include <asm/atomic.h>
#include <datapath.h>

#include "ovsmgr.h"
#include "ovsmgr_priv.h"

static char *port_type[__OVS_VPORT_TYPE_MAX] = {
	"Unspecified Vport type",
	"Network device",
	"Internal device",
	"GRE device",
	"VxLAN device",
	"Geneve device",
};

/*
 * ovsmgr_debugfs_dp_info_show()
 */
static void ovsmgr_debugfs_dp_info_show(struct ovsmgr_dp *nod, struct seq_file *m)
{
	struct ovsmgr_dp_port *nodp;
	uint8_t i = 0;

	seq_printf(m, "Datapath Dev : %s \n", nod->dev->name);
	seq_printf(m, "datapath instance : %px\n", nod->dp);
	seq_printf(m, "Statistics:\n");

	list_for_each_entry(nodp, &nod->port_list, node) {
		seq_printf(m, "Port :\n");
		seq_printf(m, "\tdevice: %s\n", nodp->dev->name);
		seq_printf(m, "\tMaster device: %s\n", nodp->master_dev->name);
		seq_printf(m, "\tinstance : %px\n", nodp->vport);
		seq_printf(m, "\tport number : %d\n", nodp->vport_num);
		seq_printf(m, "\tport type : %s\n", port_type[nodp->vport_type]);
		for (i = 0; i < OVSMGR_PORT_VLAN_MAX_CNT; i++) {
			if (nodp->vlan_info[i].vlan.h_vlan_TCI) {
				seq_printf(m, "\tPort VLAN TCI : %d\n", nodp->vlan_info[i].vlan.h_vlan_TCI);
				seq_printf(m, "\tPort VLAN ref count : %d\n", nodp->vlan_info[i].ref_cnt);
			}
		}
	}
}

/*
 * ovsmgr_debugfs_dp_show()
 */
static int ovsmgr_debugfs_dp_show(struct seq_file *m, void *p)
{
	struct ovsmgr_dp *nod;

	read_lock_bh(&ovsmgr_ctx.lock);

	seq_printf(m, "\tPackets received from DP: %llu\n", (u64)atomic64_read(&ovsmgr_ctx.stats.pkts_from_ovs_dp));
	seq_printf(m, "\tPacket forwarded to pre flow hooks : %llu\n", (u64)atomic64_read(&ovsmgr_ctx.stats.pkts_fwd_pre_flow));
	seq_printf(m, "\tPacket forwarded to post flow hooks : %llu\n", (u64)atomic64_read(&ovsmgr_ctx.stats.pkts_fwd_post_flow));

	list_for_each_entry(nod, &ovsmgr_ctx.dp_list, node) {
		ovsmgr_debugfs_dp_info_show(nod, m);
	}

	read_unlock_bh(&ovsmgr_ctx.lock);
	return 0;
}

/*
 * ovsmgr_debugfs_dp_open()
 */
static int ovsmgr_debugfs_dp_open(struct inode *inode, struct file *file)
{
	return single_open(file, ovsmgr_debugfs_dp_show, inode->i_private);
}

/*
 * Datapath file operation structure instance
 */
static const struct file_operations dp_op = {
	.open = ovsmgr_debugfs_dp_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release
};

/*
 * ovsmgr_debugfs_cleanup()
 *	Cleanup the debugfs tree.
 */
void ovsmgr_debugfs_cleanup(void)
{
	debugfs_remove_recursive(ovsmgr_ctx.dentry);
}

/*
 * ovsmgr_debugfs_init()
 *	Initialize the debugfs tree.
 */
int ovsmgr_debugfs_init(void)
{
	/*
	 * initialize debugfs.
	 */
	ovsmgr_ctx.dentry = debugfs_create_dir("qca-ovsmgr", NULL);
	if (!ovsmgr_ctx.dentry) {
		ovsmgr_warn("Creating debug directory failed\n");
		return -1;
	}

	/*
	 * Create debugfs entries for datapath
	 */
	ovsmgr_ctx.dentry_file = debugfs_create_file("ovsdp", S_IRUGO, ovsmgr_ctx.dentry, NULL, &dp_op);
	if (!ovsmgr_ctx.dentry_file) {
		ovsmgr_warn("Debugfs file creation failed\n");
		debugfs_remove_recursive(ovsmgr_ctx.dentry);
		return -1;
	}

	return 0;
}
