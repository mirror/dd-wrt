// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Notifiers for addition and removal of block devices
 *
 * Copyright (c) 2024 Daniel Golle <daniel@makrotopia.org>
 */

#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/notifier.h>
#include <linux/workqueue.h>

#include "blk.h"

struct blk_device_list {
	struct device *dev;
	struct list_head list;
};

static RAW_NOTIFIER_HEAD(blk_notifier_list);
static DEFINE_MUTEX(blk_notifier_lock);
static LIST_HEAD(blk_devices);

struct blk_notify_event {
	struct delayed_work work;
	struct device *dev;
};

void blk_register_notify(struct notifier_block *nb)
{
	struct blk_device_list *existing_blkdev;

	mutex_lock(&blk_notifier_lock);
	raw_notifier_chain_register(&blk_notifier_list, nb);

	list_for_each_entry(existing_blkdev, &blk_devices, list)
		nb->notifier_call(nb, BLK_DEVICE_ADD, existing_blkdev->dev);

	mutex_unlock(&blk_notifier_lock);
}
EXPORT_SYMBOL_GPL(blk_register_notify);

void blk_unregister_notify(struct notifier_block *nb)
{
	mutex_lock(&blk_notifier_lock);
	raw_notifier_chain_unregister(&blk_notifier_list, nb);
	mutex_unlock(&blk_notifier_lock);
}
EXPORT_SYMBOL_GPL(blk_unregister_notify);

static void blk_notify_work(struct work_struct *work)
{
	struct blk_notify_event *ev =
		container_of(work, struct blk_notify_event, work.work);

	raw_notifier_call_chain(&blk_notifier_list, BLK_DEVICE_ADD, ev->dev);
	kfree(ev);
}

static int blk_call_notifier_add(struct device *dev)
{
	struct blk_device_list *new_blkdev;
	struct blk_notify_event *ev;

	new_blkdev = kmalloc(sizeof (*new_blkdev), GFP_KERNEL);
	if (!new_blkdev)
		return -ENOMEM;

	ev = kmalloc(sizeof(*ev), GFP_KERNEL);
	INIT_DEFERRABLE_WORK(&ev->work, blk_notify_work);
	ev->dev = dev;
	new_blkdev->dev = dev;
	mutex_lock(&blk_notifier_lock);
	list_add_tail(&new_blkdev->list, &blk_devices);
	schedule_delayed_work(&ev->work, msecs_to_jiffies(500));
	mutex_unlock(&blk_notifier_lock);

	return 0;
}

static void blk_call_notifier_remove(struct device *dev)
{
	struct blk_device_list *old_blkdev, *tmp;

	mutex_lock(&blk_notifier_lock);
	list_for_each_entry_safe(old_blkdev, tmp, &blk_devices, list) {
		if (old_blkdev->dev != dev)
			continue;

		list_del(&old_blkdev->list);
		kfree(old_blkdev);
	}
	raw_notifier_call_chain(&blk_notifier_list, BLK_DEVICE_REMOVE, dev);
	mutex_unlock(&blk_notifier_lock);
}

static struct class_interface blk_notifications_bus_interface __refdata = {
	.class = &block_class,
	.add_dev = &blk_call_notifier_add,
	.remove_dev = &blk_call_notifier_remove,
};

static int __init blk_notifications_init(void)
{
	return class_interface_register(&blk_notifications_bus_interface);
}
device_initcall(blk_notifications_init);
