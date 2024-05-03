/*
 * Copyright (c) 2014 - 2017, The Linux Foundation. All rights reserved.
 *
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
 */

/*
 * rfs_main.c
 *	Receiving Flow Steering
 */

#include <linux/module.h>
#include <linux/sysfs.h>
#include <linux/skbuff.h>
#include <net/route.h>
#include <linux/inetdevice.h>
#include <linux/netfilter_bridge.h>
#include <linux/proc_fs.h>
#include <net/netfilter/nf_conntrack_acct.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_zones.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <linux/if_bridge.h>

#include "rfs.h"
#include "rfs_cm.h"
#include "rfs_nbr.h"
#include "rfs_wxt.h"
#include "rfs_rule.h"
#include "rfs_ess.h"
#include "rfs_fdb.h"


static int rfs_start (void);
static int rfs_stop (void);

/*
 * debug level
 */
int rfs_dbg_level = DBG_LVL_DEFAULT;


/*
 * feature enable
 */
static int rfs_enable = 0;

/*
 * rfs_proc_entry, root proc entry of RFS module
 */
struct proc_dir_entry *rfs_proc_entry;

/*
 * rfs_debug_entry, debug entry in proc file system
 */
static struct proc_dir_entry *rfs_debug_entry;

/*
 * rfs_enable_entry, enable entry in proc file system
 */
static struct proc_dir_entry *rfs_enable_entry;

/*
 * rfs_debug_proc_show
 *	Show debug level in proc file system
 */
static int rfs_debug_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%d\n", rfs_dbg_level);
	return 0;
}


/*
 * rfs_debug_proc_write
 *	Change debug level through proc file system
 */
static ssize_t rfs_debug_proc_write(struct file *file, const char __user *buffer,
			size_t count, loff_t *ppos)
{
	unsigned long val;
	int err = kstrtoul_from_user(buffer, count, 0, &val);
	if (err)
		return err;

	rfs_dbg_level = val;
	return count;

}


/*
 * rfs_debug_proc_open
 */
static int rfs_debug_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, rfs_debug_proc_show, NULL);
}


/*
 * struct file_operations debug_proc_fops
 */
static const struct file_operations debug_proc_fops = {
	.owner = THIS_MODULE,
	.open  = rfs_debug_proc_open,
	.read  = seq_read,
	.llseek = seq_lseek,
	.write  = rfs_debug_proc_write,
	.release = single_release,
};


/*
 * rfs_enable_proc_show
 */
static int rfs_enable_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%d\n", rfs_enable);
	return 0;
}


/*
 * rfs_enalbe_proc_write
 */
static ssize_t rfs_enable_proc_write(struct file *file, const char __user *buffer,
			size_t count, loff_t *ppos)
{
	unsigned long val;
	struct seq_file *m = file->private_data;
	int err = kstrtoul_from_user(buffer, count, 0, &val);
	if (err)
		return err;

	mutex_lock(&m->lock);
	if (rfs_enable == val) {
		mutex_unlock(&m->lock);
		return count;
	}

	if (val) {
		rfs_start();
	} else {
		rfs_stop();
	}

	rfs_enable = !!val;
	mutex_unlock(&m->lock);
	return count;

}


/*
 * rfs_enalbe_proc_open
 */
static int rfs_enable_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, rfs_enable_proc_show, NULL);
}


/*
 * struct file_operations enable_proc_fops
 */
static const struct file_operations enable_proc_fops = {
	.owner = THIS_MODULE,
	.open  = rfs_enable_proc_open,
	.read  = seq_read,
	.llseek = seq_lseek,
	.write  = rfs_enable_proc_write,
	.release = single_release,
};


/*
 * rfs_proc_init
 */
static int rfs_proc_init(void)
{
	/*
	 * Create /proc/qrfs
	 */
	rfs_proc_entry = proc_mkdir("qrfs", NULL);
	if (!rfs_proc_entry) {
		RFS_ERROR("failed to register qrfs proc entry\n");
		return -1;
	}

	rfs_debug_entry = proc_create("debug",
				S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH,
				rfs_proc_entry, &debug_proc_fops);

	rfs_enable_entry = proc_create("enable",
				S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH,
				rfs_proc_entry, &enable_proc_fops);
	return 0;
}


/*
 * rfs_proc_exit
 */
static void rfs_proc_exit(void)
{
	if (rfs_enable_entry)
		remove_proc_entry("enable", rfs_proc_entry);

	if (rfs_debug_entry)
		remove_proc_entry("debug", rfs_proc_entry);

	if (rfs_proc_entry) {
		remove_proc_entry("qrfs", NULL);
		rfs_proc_entry = NULL;
	}
}


/*
 * rfs_start
 */
static int rfs_start (void)
{
	rfs_ess_start();
	rfs_cm_start();
	rfs_nbr_start();
	rfs_wxt_start();
	rfs_fdb_start();
	return 0;
}


/*
 * rfs_stop
 */
static int rfs_stop(void)
{
	rfs_fdb_stop();
	rfs_wxt_stop();
	rfs_nbr_stop();
	rfs_cm_stop();
	rfs_ess_stop();
	rfs_rule_destroy_all();
	return 0;
}


/*
 * rfs_init()
 */
static int __init rfs_init(void)
{
	RFS_DEBUG("RFS init\n");

	/*
	 * proc file system
	 */
	if (rfs_proc_init() < 0)
		return -1;

	/*
	 * Ethernet sub-system
	 */
	if (rfs_ess_init() < 0) {
		goto exit1;
	}

	/*
	 * IPv4 connection management
	 */
	(void)rfs_cm_init();

	/*
	 * IP neighbor management
	 */
	(void)rfs_nbr_init();
	/*
	 * RFS rules
	 */
	(void)rfs_rule_init();

	/*
	 * wireless extension
	 */
	(void)rfs_wxt_init();

	/*
	 * FDB management
	 */
	(void)rfs_fdb_init();

	return 0;

exit1:
	rfs_proc_exit();

	return 0;
}


/*
 * rfs_exit()
 */
static void __exit rfs_exit(void)
{
	RFS_DEBUG("RFS exit\n");

	rfs_fdb_exit();

	rfs_wxt_exit();

	rfs_rule_exit();

	rfs_nbr_exit();

	rfs_cm_exit();

	rfs_ess_exit();

	rfs_proc_exit();

	rcu_barrier();
}


module_init(rfs_init)
module_exit(rfs_exit)

MODULE_DESCRIPTION("Receiving Flow Steering");
MODULE_LICENSE("Dual BSD/GPL");

