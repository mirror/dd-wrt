/*
 *	SoftDog	0.07:	A Software Watchdog Device
 *
 *	(c) Copyright 1996 Alan Cox <alan@redhat.com>, All Rights Reserved.
 *				http://www.redhat.com
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 *
 *	Neither Alan Cox nor CymruNet Ltd. admit liability nor provide
 *	warranty for any of this software. This material is provided
 *	"AS-IS" and at no charge.
 *
 *	(c) Copyright 1995    Alan Cox <alan@lxorguk.ukuu.org.uk>
 *
 *	Software only watchdog driver. Unlike its big brother the WDT501P
 *	driver this won't always recover a failed machine.
 *
 *  03/96: Angelo Haritsis <ah@doc.ic.ac.uk> :
 *	Modularised.
 *	Added soft_margin; use upon insmod to change the timer delay.
 *	NB: uses same minor as wdt (WATCHDOG_MINOR); we could use separate
 *	    minors.
 *
 *  19980911 Alan Cox
 *	Made SMP safe for 2.3.x
 *
 *  20011127 Joel Becker (jlbec@evilplan.org>
 *	Added soft_noboot; Allows testing the softdog trigger without
 *	requiring a recompile.
 *	Added WDIOC_GETTIMEOUT and WDIOC_SETTIMOUT.
 *
 *  20020530 Joel Becker <joel.becker@oracle.com>
 *  	Added Matt Domsch's nowayout module option.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/fs.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <linux/init.h>
#include <linux/jiffies.h>

#include <asm/uaccess.h>

#define PFX "SoftDog: "

#define TIMER_MARGIN	60		/* Default is 60 seconds */
static int soft_margin = TIMER_MARGIN;	/* in seconds */
module_param(soft_margin, int, 0);
MODULE_PARM_DESC(soft_margin, "Watchdog soft_margin in seconds. (0<soft_margin<65536, default=" __MODULE_STRING(TIMER_MARGIN) ")");

static int nowayout = WATCHDOG_NOWAYOUT;
module_param(nowayout, int, 0);
MODULE_PARM_DESC(nowayout, "Watchdog cannot be stopped once started (default=" __MODULE_STRING(WATCHDOG_NOWAYOUT) ")");

#ifdef ONLY_TESTING
static int soft_noboot = 1;
#else
static int soft_noboot = 0;
#endif  /* ONLY_TESTING */


/*
 *	Our timer
 */

static char expect_close;

extern int watchdog_timer;


/*
 *	Softdog operations
 */


static unsigned long driver_open;

/*
 *	/dev/watchdog handling
 */

static int softdog_open(struct inode *inode, struct file *file)
{
	if (test_and_set_bit(0, &driver_open))
		return -EBUSY;
	return 0;
}

static int softdog_release(struct inode *inode, struct file *file)
{
	/*
	 *	Shut off the timer.
	 * 	Lock it in if it's a module and we set nowayout
	 */
	printk(KERN_CRIT "BCMDOG: WDT device closed unexpectedly.  WDT will not stop!\n");
	return 0;
}

static ssize_t softdog_write(struct file *file, const char __user *data, size_t len, loff_t *ppos)
{
	/*
	 *	Refresh the timer.
	 */
	if(len) {
		watchdog_timer = 30 * HZ;
	}
	return len;
}

static long softdog_ioctl(struct file *file,
	unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	int __user *p = argp;
	int new_margin;
	static struct watchdog_info ident = {
		.options =		WDIOF_SETTIMEOUT |
					WDIOF_KEEPALIVEPING |
					WDIOF_MAGICCLOSE,
		.firmware_version =	0,
		.identity =		"Software Watchdog",
	};
	switch (cmd) {
		default:
			return -ENOTTY;
		case WDIOC_GETSUPPORT:
			return copy_to_user(argp, &ident,
				sizeof(ident)) ? -EFAULT : 0;
		case WDIOC_GETSTATUS:
		case WDIOC_GETBOOTSTATUS:
			return put_user(0, p);
		case WDIOC_KEEPALIVE:
			watchdog_timer = 30000 * HZ;
			return 0;
		case WDIOC_SETTIMEOUT:
			if (get_user(new_margin, p))
				return -EFAULT;
			/* Fall */
		case WDIOC_GETTIMEOUT:
			return put_user(soft_margin, p);
	}
}

/*
 *	Kernel Interfaces
 */

static const struct file_operations softdog_fops = {
	.owner		= THIS_MODULE,
	.llseek		= no_llseek,
	.write		= softdog_write,
	.unlocked_ioctl	= softdog_ioctl,
	.open		= softdog_open,
	.release	= softdog_release,
};

static struct miscdevice softdog_miscdev = {
	.minor		= WATCHDOG_MINOR,
	.name		= "watchdog",
	.fops		= &softdog_fops,
};


static char banner[] __initdata = KERN_INFO "Broadcom Watchdog Timer: 0.07 initialized.\n";

static int __init watchdog_init(void)
{
	int ret;


	ret = misc_register(&softdog_miscdev);

	printk(banner);

	return 0;
}

static void __exit watchdog_exit(void)
{
	misc_deregister(&softdog_miscdev);
}

module_init(watchdog_init);
module_exit(watchdog_exit);

MODULE_AUTHOR("Alan Cox");
MODULE_DESCRIPTION("Software Watchdog Device Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS_MISCDEV(WATCHDOG_MINOR);
