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

#define BUTTON_SELECT	(1<<1)
#define BUTTON_1	(1<<2)
#define BUTTON_2	(1<<3)
#define BUTTON_ONOFF	(1<<6)
#define BUTTON_3	(1<<7)
#define BUTTON_4	(1<<8)
#define BUTTON_LEFT	(1<<9)
#define BUTTON_DOWN	(1<<10)
#define BUTTON_RIGHT	(1<<11)
#define BUTTON_UP	(1<<12)

#define BUTTON_MASK (\
    BUTTON_SELECT	\
    | BUTTON_1   	\
    | BUTTON_2		\
    | BUTTON_ONOFF	\
    | BUTTON_3		\
    | BUTTON_4		\
    | BUTTON_LEFT	\
    | BUTTON_DOWN	\
    | BUTTON_RIGHT	\
    | BUTTON_UP		\
    )

#define BUTTON_INVERT (\
    BUTTON_SELECT	\
    | BUTTON_1   	\
    | BUTTON_2		\
    | BUTTON_3		\
    | BUTTON_4		\
    | BUTTON_LEFT	\
    | BUTTON_DOWN	\
    | BUTTON_RIGHT	\
    | BUTTON_UP		\
    )



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
 * your application open the buttons listing from the /dev directory first.
 */

struct hydrogen3_buttons {
	struct fasync_struct *fasync;
	wait_queue_head_t     read_wait;
	int open_count;
	unsigned int debounce;
	unsigned int current;
	unsigned int last;
};

static struct hydrogen3_buttons buttons_info;


static void button_timer_periodic(void *data);

static struct tq_struct button_task = {
	routine:	button_timer_periodic,
	data:		NULL
};

static int cleanup_flag = 0;
static DECLARE_WAIT_QUEUE_HEAD(cleanup_wait_queue);


static unsigned int read_button_state(void)
{
	unsigned long state;

	state = inl(SYS_PINSTATERD) & BUTTON_MASK;
	state ^= BUTTON_INVERT;
	
	DPRINTK( "Current Button State: %d\n", state );

	return state;
}


static void button_timer_periodic(void *data)
{
	struct hydrogen3_buttons *buttons = (struct hydrogen3_buttons *)data;
	unsigned long button_state;
	
	// If cleanup wants us to die
	if (cleanup_flag) {
		wake_up(&cleanup_wait_queue);		// now cleanup_module can return
	} else {
		queue_task(&button_task, &tq_timer);	// put ourselves back in the task queue
	}

	// read current buttons
	button_state = read_button_state();

	// if no buttons are down and nothing to do then
	// save time and be done.
	if ((button_state == 0) && (buttons->current == 0)) {
		return;
	}
	
	if (button_state == buttons->debounce) {
		buttons->current = button_state;
	} else {
		buttons->debounce = button_state;
	}
//	printk("0x%04x\n", button_state);
	if (buttons->current != buttons->last) {
		if (waitqueue_active(&buttons->read_wait)) {
		    wake_up_interruptible(&buttons->read_wait);
		}
	}
}


static ssize_t hydrogen3_buttons_read(struct file *filp, char *buffer, size_t count, loff_t *ppos)
{
	struct hydrogen3_buttons *buttons = filp->private_data;
	char events[16];
	int index;
	int last;
	int cur;
	int bit;
	int bit_mask;
	int err;
	
	DPRINTK("start\n");

try_again:

	while (buttons->current == buttons->last) {
		if (filp->f_flags & O_NONBLOCK) {
			return -EAGAIN;
		}
		interruptible_sleep_on(&buttons->read_wait);
		if (signal_pending(current)) {
			return -ERESTARTSYS;
		}
	}
	
	cur  = buttons->current;
	last = buttons->last;
	
	index    = 0;
	bit_mask = 1;
	for (bit = 0; (bit < 16) && count; bit++) {
		if ((cur ^ last) & bit_mask) {
			if (cur & bit_mask) {
				events[index] = (bit | MAKE_FLAG) + 'A';
				last |= bit_mask;
			} else {
				events[index] = bit + 'A';
				last &= ~bit_mask;
			}
			index++;
			count--;
		}
		bit_mask <<= 1;
	}
	buttons->last = last;
	
	if (index == 0) {
		goto try_again;
	}
	
	err = copy_to_user(buffer, events, index);
	if (err) {
		return err;
	}
	
	return index;
}


static int hydrogen3_buttons_open(struct inode *inode, struct file *filp)
{
	struct hydrogen3_buttons *buttons = &buttons_info;

	DPRINTK("start\n");
	MOD_INC_USE_COUNT;

	filp->private_data = buttons;

	if (buttons->open_count++ == 0) {
		button_task.data = buttons;
		cleanup_flag = 0;
		queue_task(&button_task, &tq_timer);
	}

	return 0;
}


static unsigned int hydrogen3_buttons_poll(struct file *filp, poll_table *wait)
{
	struct hydrogen3_buttons *buttons = filp->private_data;
	int ret = 0;

	DPRINTK("start\n");
	poll_wait(filp, &buttons->read_wait, wait);
	if (buttons->current != buttons->last) {
		ret = POLLIN | POLLRDNORM;
	}
	return ret;
}


static int hydrogen3_buttons_release(struct inode *inode, struct file *filp)
{
	struct hydrogen3_buttons *buttons = filp->private_data;

	DPRINTK("start\n");

	if (--buttons->open_count == 0) {
		cleanup_flag = 1;
		sleep_on(&cleanup_wait_queue);
	}
	MOD_DEC_USE_COUNT;
	
	return 0;
}



static struct file_operations hydrogen3_buttons_fops = {
	owner:		THIS_MODULE,
	read:		hydrogen3_buttons_read,
	poll:		hydrogen3_buttons_poll,
	open:		hydrogen3_buttons_open,
	release:	hydrogen3_buttons_release,
};

/*
 * The hydrogen3 buttons is a misc device:
 * Major 10 char
 * Minor 22        /dev/buttons
 * 
 * This is /dev/misc/buttons if devfs is used.
 */

static struct miscdevice hydrogen3_buttons_dev = {
	minor:	22,
	name:	"buttons",
	fops:	&hydrogen3_buttons_fops,
};

static int __init hydrogen3_buttons_init(void)
{
	struct hydrogen3_buttons *buttons = &buttons_info;
	int ret;

	DPRINTK("Initializing buttons driver\n");
	buttons->open_count = 0;
	cleanup_flag        = 0;
	init_waitqueue_head(&buttons->read_wait);


	// yamon configures GPIO pins for the buttons
	// no initialization needed

	ret = misc_register(&hydrogen3_buttons_dev);

	DPRINTK("Buttons driver fully initialized.\n");

	return ret;
}


static void __exit hydrogen3_buttons_exit(void)
{
	DPRINTK("unloading buttons driver\n");
	misc_deregister(&hydrogen3_buttons_dev);
}


module_init(hydrogen3_buttons_init);
module_exit(hydrogen3_buttons_exit);
