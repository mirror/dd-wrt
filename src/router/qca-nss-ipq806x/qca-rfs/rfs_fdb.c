/*
 * Copyright (c) 2015-2017 The Linux Foundation. All rights reserved.
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
 * rfs_fdb.c
 *	Receiving Flow Streering - FDB Manager
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/sysfs.h>
#include <linux/skbuff.h>
#include <linux/inetdevice.h>
#include <linux/netfilter_bridge.h>
#include <linux/if_bridge.h>
#include <linux/jhash.h>
#include <linux/proc_fs.h>
#include <net/netevent.h>
#include <net/route.h>

#include "rfs.h"
#include "rfs_wxt.h"
#include "rfs_rule.h"

#define MAX_BRIDGE_NUMBER 6
/*
 * Per-module structure.
 */
struct rfs_fdb {
	int is_running;
};


static struct rfs_fdb __fdb;


/*
 * fdb_event_callback
 */
static int fdb_event_callback(struct notifier_block *notifier, unsigned long event, void *ctx)
{
	struct br_fdb_event *fe;
        struct net_device *dev, *vdev=NULL;
	int cpu;

	fe = (struct br_fdb_event*)ctx;
	dev = fe->dev;

	if (!dev)
		return NOTIFY_DONE;

	if (is_vlan_dev(dev)) {
		RFS_DEBUG("Virtual device[%s] will be replaced", dev->name);
		vdev = dev;
		dev = vlan_dev_real_dev(dev);
		RFS_DEBUG("real dev[%s] instead of \n",	dev->name);
	}

	if (!dev->wireless_handlers)
		return NOTIFY_DONE;

	if (fe->is_local)
		return NOTIFY_DONE;

	switch (event) {
	case BR_FDB_EVENT_ADD:
		cpu = rfs_wxt_get_cpu(dev->ifindex);
		if (cpu == RPS_NO_CPU)
			break;
		RFS_DEBUG("STA %pM joining\n", (unsigned char*) fe->addr);
		rfs_rule_create_mac_rule((unsigned char*) fe->addr, (uint16_t)cpu, vdev?vdev:dev, 0);
		break;
	case BR_FDB_EVENT_DEL:
		RFS_DEBUG("STA %pM leaving\n", (unsigned char*) fe->addr);
		rfs_rule_destroy_mac_rule((unsigned char*) fe->addr, vdev?vdev:dev, 0);
		break;
	}

	return NOTIFY_DONE;
}

static struct notifier_block fdb_notifier = {
	.notifier_call = fdb_event_callback,
};


/*
 * fdb_get_brport_by_mac
 */
static int fdb_get_brport_by_mac(unsigned char *mac, struct net_device **ndev, int num_max)
{
	struct net_device *dev;
	struct net_device *pdev = NULL;
	int num = 0;

	rcu_read_lock();
	for_each_netdev_rcu(&init_net, dev){
		if (!(dev->priv_flags & IFF_EBRIDGE))
			continue;
		/* This condition is for date branch, we should remove it
		 * after new NSS kernel patch is merged to date branch.
		 */
#if (LINUX_VERSION_CODE == KERNEL_VERSION(3, 18, 21))
		pdev = br_port_dev_get(dev, mac);
#else
		/*
		 * Pass in NULL (for skb) and 0 for cookie since doing FDB lookup only
		 */
		pdev = br_port_dev_get(dev, mac, NULL, 0);
#endif
		if (pdev) {
			*ndev++ = pdev;
			num ++;
		}
		if (num >= num_max)
			break;
	}
	rcu_read_unlock();
	return num;
}


/*
 * fdb_update_callback
 */
static int fdb_update_callback(struct notifier_block *notifier,
			       unsigned long event, void *ctx)
{
	unsigned char *mac = (unsigned char*)ctx;
	struct net_device *ndev[MAX_BRIDGE_NUMBER];
	int cpu;
	int i=0, devnum = MAX_BRIDGE_NUMBER;

	devnum = fdb_get_brport_by_mac(mac,ndev,MAX_BRIDGE_NUMBER);
	for (; i < devnum; i++) {
		RFS_DEBUG("STA %pM being cleared\n", (unsigned char*)mac);
		if (rfs_rule_find_mac_rule(mac, ndev[i], 0)){
			rfs_rule_destroy_mac_rule(mac, ndev[i], 0);
		}

		cpu = rfs_wxt_get_cpu(ndev[i]->ifindex);
		if (cpu != RPS_NO_CPU){
			RFS_DEBUG("STA %pM rejoining\n", (unsigned char*)mac);
			rfs_rule_create_mac_rule((unsigned char*)mac, (uint16_t)cpu, ndev[i], 0);
		}

		dev_put(ndev[i]);
	}
	return NOTIFY_DONE;
}

static struct notifier_block fdb_update_notifier = {
	.notifier_call = fdb_update_callback,
};

/*
 * rfs_fdb_start
 */
int rfs_fdb_start(void)
{
	struct rfs_fdb *fdb = &__fdb;

	if (fdb->is_running)
		return 0;

	RFS_DEBUG("RFS fdb start\n");
	br_fdb_register_notify(&fdb_notifier);
	br_fdb_update_register_notify(&fdb_update_notifier);
	fdb->is_running = 1;
	return 0;
}


/*
 * rfs_fdb_stop
 */
int rfs_fdb_stop(void)
{
	struct rfs_fdb *fdb = &__fdb;

	if (!fdb->is_running)
		return 0;

	RFS_DEBUG("RFS fdb stop\n");
	br_fdb_update_unregister_notify(&fdb_update_notifier);
	br_fdb_unregister_notify(&fdb_notifier);
	fdb->is_running = 0;
	return 0;
}


/*
 * rfs_fdb_init
 */
int rfs_fdb_init(void)
{
	struct rfs_fdb *fdb = &__fdb;

	RFS_DEBUG("RFS fdb init\n");
	fdb->is_running = 0;
	return 0;
}


/*
 * rfs_fdb_exit
 */
void rfs_fdb_exit(void)
{
	RFS_DEBUG("RFS fdb exit\n");
	rfs_fdb_stop();
}

