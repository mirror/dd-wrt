/*
 *  Copyright (C) 2003 Metrowerks, All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/miscdevice.h>
#include <linux/errno.h>
#include <linux/poll.h>
#include <asm/au1000.h>
#include <asm/uaccess.h>
#include <asm/au1xxx_gpio.h>


#if defined(CONFIG_MIPS_FICMMP)
	#define DOCK_GPIO	215
#else
	#error Unsupported Au1xxx Platform
#endif

#define MAKE_FLAG 0x20

#undef DEBUG

#define DEBUG 0
//#define DEBUG 1

#if DEBUG
#define DPRINTK(format, args...) printk(__FUNCTION__ ": "  format, ## args)
#else
#define DPRINTK(format, args...) do { } while (0)
#endif

/* Please note that this driver is based on a timer and is not interrupt
 * driven.  If you are going to make use of this driver, you will need to have
 * your application open the dock listing from the /dev directory first.
 */

struct au1xxx_dock {
	struct fasync_struct *fasync;
	wait_queue_head_t     read_wait;
	int open_count;
	unsigned int debounce;
	unsigned int current;
	unsigned int last;
};

static struct au1xxx_dock dock_info;


static void dock_timer_periodic(void *data);

static struct tq_struct dock_task = {
	routine:	dock_timer_periodic,
	data:		NULL
};

static int cleanup_flag = 0;
static DECLARE_WAIT_QUEUE_HEAD(cleanup_wait_queue);


static unsigned int read_dock_state(void)
{
	u32 state;

	state = au1xxx_gpio_read(DOCK_GPIO);
	
	/* printk( "Current Dock State: %d\n", state ); */

	return state;
}


static void dock_timer_periodic(void *data)
{
	struct au1xxx_dock *dock = (struct au1xxx_dock *)data;
	unsigned long dock_state;
	
	/* If cleanup wants us to die */
	if (cleanup_flag) {
		/* now cleanup_module can return */
		wake_up(&cleanup_wait_queue); 
	} else {
		/* put ourselves back in the task queue */
		queue_task(&dock_task, &tq_timer);	
	}

	/* read current dock */
	dock_state = read_dock_state();

	/* if dock states hasn't changed */
	/* save time and be done. */
	if (dock_state == dock->current) {
		return;
	}
	
	if (dock_state == dock->debounce) {
		dock->current = dock_state;
	} else {
		dock->debounce = dock_state;
	}
	if (dock->current != dock->last) {
		if (waitqueue_active(&dock->read_wait)) {
		    wake_up_interruptible(&dock->read_wait);
		}
	}
}


static ssize_t au1xxx_dock_read(struct file *filp, char *buffer, size_t count, loff_t *ppos)
{
	struct au1xxx_dock *dock = filp->private_data;
	char event[3];
	int last;
	int cur;
	int err;
	
try_again:

	while (dock->current == dock->last) {
		if (filp->f_flags & O_NONBLOCK) {
			return -EAGAIN;
		}
		interruptible_sleep_on(&dock->read_wait);
		if (signal_pending(current)) {
			return -ERESTARTSYS;
		}
	}
	
	cur  = dock->current;
	last = dock->last;

	if(cur != last)
	{
		event[0] = cur ? 'D' : 'U';
		event[1] = '\r';
		event[2] = '\n';
	}
	else
		goto try_again;
	
	dock->last = cur;
	err = copy_to_user(buffer, &event, 3);
	if (err) {
		return err;
	}
	
	return 3;
}


static int au1xxx_dock_open(struct inode *inode, struct file *filp)
{
	struct au1xxx_dock *dock = &dock_info;

	MOD_INC_USE_COUNT;

	filp->private_data = dock;

	if (dock->open_count++ == 0) {
		dock_task.data = dock;
		cleanup_flag = 0;
		queue_task(&dock_task, &tq_timer);
	}

	return 0;
}


static unsigned int au1xxx_dock_poll(struct file *filp, poll_table *wait)
{
	struct au1xxx_dock *dock = filp->private_data;
	int ret = 0;

	DPRINTK("start\n");
	poll_wait(filp, &dock->read_wait, wait);
	if (dock->current != dock->last) {
		ret = POLLIN | POLLRDNORM;
	}
	return ret;
}


static int au1xxx_dock_release(struct inode *inode, struct file *filp)
{
	struct au1xxx_dock *dock = filp->private_data;

	DPRINTK("start\n");

	if (--dock->open_count == 0) {
		cleanup_flag = 1;
		sleep_on(&cleanup_wait_queue);
	}
	MOD_DEC_USE_COUNT;
	
	return 0;
}



static struct file_operations au1xxx_dock_fops = {
	owner:		THIS_MODULE,
	read:		au1xxx_dock_read,
	poll:		au1xxx_dock_poll,
	open:		au1xxx_dock_open,
	release:	au1xxx_dock_release,
};

/*
 * The au1xxx dock is a misc device:
 * Major 10 char
 * Minor 22        /dev/dock
 * 
 * This is /dev/misc/dock if devfs is used.
 */

static struct miscdevice au1xxx_dock_dev = {
	minor:	23,
	name:	"dock",
	fops:	&au1xxx_dock_fops,
};

static int __init au1xxx_dock_init(void)
{
	struct au1xxx_dock *dock = &dock_info;
	int ret;

	DPRINTK("Initializing dock driver\n");
	dock->open_count = 0;
	cleanup_flag        = 0;
	init_waitqueue_head(&dock->read_wait);


	/* yamon configures GPIO pins for the dock
	 * no initialization needed
	 */

	ret = misc_register(&au1xxx_dock_dev);

	DPRINTK("dock driver fully initialized.\n");

	return ret;
}


static void __exit au1xxx_dock_exit(void)
{
	DPRINTK("unloading dock driver\n");
	misc_deregister(&au1xxx_dock_dev);
}


module_init(au1xxx_dock_init);
module_exit(au1xxx_dock_exit);
