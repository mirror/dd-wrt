/*
 * Copyright (c) 2023, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <linux/proc_fs.h>
#include <linux/version.h>
#include "edma_procfs.h"
#include "edma.h"

static struct proc_dir_entry *edma_procfs;

/*
 * edma_procfs_ring_stats_show()
 *	EDMA procfs debug ring stats show
 */
static int edma_procfs_ring_stats_show(struct seq_file *seq, void *v)
{
	struct edma_gbl_ctx *egc = &edma_gbl_ctx;

	seq_printf(seq, "EDMA ring stats debug is %s\n", egc->enable_ring_util_stats? "enabled" : "disabled");
	return 0;
}

/*
 * edma_procfs_ring_stats_open()
 *	EDMA procfs ring stats open callback
 */
static int edma_procfs_ring_stats_open(struct inode *inode, struct file *file)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
	return single_open(file, edma_procfs_ring_stats_show, PDE_DATA(inode));
#else
	return single_open(file, edma_procfs_ring_stats_show, pde_data(inode));
#endif
}

/*
 * edma_procfs_ring_stats_write()
 *	EDMA procfs ring stats write callback
 */
static ssize_t edma_procfs_ring_stats_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	int ret;
	int enable_stats;
	char buffer[13];
	struct edma_gbl_ctx *egc = &edma_gbl_ctx;

	memset(buffer, 0, sizeof(buffer));
	if (count > sizeof(buffer) - 1)
		count = sizeof(buffer) - 1;

	if (copy_from_user(buffer, buf, count) != 0)
		return -EFAULT;

	ret = kstrtoint(strstrip(buffer), 10, &enable_stats);

	if (ret == 0 && enable_stats >= 0) {
		egc->enable_ring_util_stats = enable_stats;
	}

	return count;
}

/*
 * edma_procfs_ring_stats_fops
 *	File operations for EDMA ring stats
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
static const struct file_operations edma_procfs_ring_stats_fops = {
	.owner   = THIS_MODULE,
	.open    = edma_procfs_ring_stats_open,
	.read    = seq_read,
	.write   = edma_procfs_ring_stats_write,
	.release = single_release,
};
#else
static const struct proc_ops edma_procfs_ring_stats_fops = {
	.proc_open    = edma_procfs_ring_stats_open,
	.proc_read    = seq_read,
	.proc_write   = edma_procfs_ring_stats_write,
	.proc_release = single_release,
};
#endif

/*
 * edma_procfs_exit()
 *	EDMA procfs exit
 */
void edma_procfs_exit(void)
{
	remove_proc_entry("enable_ring_stats_utilization", edma_procfs);
}

/*
 * edma_procfs_init
 * 	EDMA procfs init
 */
void edma_procfs_init(void)
{
	edma_procfs = proc_mkdir("edma_ring_util_stats", NULL);

	if (!edma_procfs) {
		pr_err("cannot create EDMA ring util proc dir");
		return;
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
	if (!proc_create("enable_ring_stats_utilization",
				S_IRUGO | S_IWUGO,
				edma_procfs,
				&edma_procfs_ring_stats_fops))
		pr_err("cannot create enable stats\n");
#else
	if (!proc_create("enable_ring_stats_utilization",
				S_IRUGO | S_IWUGO,
				edma_procfs,
				&edma_procfs_ring_stats_fops))
		pr_err("cannot create enable stats\n");
#endif
}
