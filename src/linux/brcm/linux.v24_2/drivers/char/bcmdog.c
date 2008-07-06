/*
 *	SoftDog	0.05:	A Software Watchdog Device
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
#include <linux/config.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/reboot.h>
#include <linux/smp_lock.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <typedefs.h>
#include <osl.h>
#include <bcmnvram.h>
#include <sbconfig.h>
#include <sbutils.h>
#include <sbchipc.h>
#include <hndmips.h>
#include <mipsinc.h>
#include <hndcpu.h>
#include <bcmdevs.h>

#define TIMER_MARGIN	60		/* (secs) Default is 1 minute */

static int expect_close = 0;

MODULE_LICENSE("GPL");

extern int watchdog_timer;

#ifdef CONFIG_WATCHDOG_NOWAYOUT
static int nowayout = 1;
#else
static int nowayout = 0;
#endif

extern void *bcm947xx_sbh;
/* Convenience */
#define sbh bcm947xx_sbh


/*
 *	Our timer
 */
 

static unsigned long timer_alive;


/*
 *	If the timer expires..
 */

/*
 *	Allow only one person to hold it open
 */
 
static int softdog_open(struct inode *inode, struct file *file)
{
	if(test_and_set_bit(0, &timer_alive))
		return -EBUSY;
		MOD_INC_USE_COUNT;
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

static ssize_t softdog_write(struct file *file, const char *data, size_t len, loff_t *ppos)
{
	/*  Can't seek (pwrite) on this device  */
	if (ppos != &file->f_pos)
		return -ESPIPE;
//printk(KERN_EMERG "try to trigger watchdog");
	/*
	 *	Refresh the timer.
	 */
	if(len) {
	/* Set the watchdog timer to reset after the specified number of ms */
//printk(KERN_EMERG "trigger watchdog at %d\n",watchdog);
		watchdog_timer = 30 * HZ;
		return 1;
	}
	return 0;
}

static int softdog_ioctl(struct inode *inode, struct file *file,
	unsigned int cmd, unsigned long arg)
{
	int new_margin;
	static struct watchdog_info ident = {
		WDIOF_SETTIMEOUT | WDIOF_MAGICCLOSE,
		0,
		"Broadcom Watchdog"
	};
	switch (cmd) {
		default:
			return -ENOTTY;
		case WDIOC_GETSUPPORT:
			if(copy_to_user((struct watchdog_info *)arg, &ident, sizeof(ident)))
				return -EFAULT;
			return 0;
		case WDIOC_GETSTATUS:
		case WDIOC_GETBOOTSTATUS:
			return put_user(0,(int *)arg);
		case WDIOC_KEEPALIVE:
			    watchdog_timer = 30000 * HZ;
			return 0;
		case WDIOC_SETTIMEOUT:
			if (get_user(new_margin, (int *)arg))
				return -EFAULT;
			if (new_margin < 1)
				return -EINVAL;
			//watchdog = new_margin;
			/* Fall */
		case WDIOC_GETTIMEOUT:
			return put_user(watchdog_timer, (int *)arg);
	}
}

static struct file_operations softdog_fops = {
	owner:		THIS_MODULE,
	write:		softdog_write,
	ioctl:		softdog_ioctl,
	open:		softdog_open,
	release:	softdog_release,
};

static struct miscdevice softdog_miscdev = {
	minor:		WATCHDOG_MINOR,
	name:		"watchdog",
	fops:		&softdog_fops,
};

static char banner[] __initdata = KERN_EMERG "Broadcom Watchdog Timer: 0.05, timer margin: %d sec\n";
static int __init watchdog_init(void)
{
	int ret;
	/* Set watchdog interval in ms */

	/* Please set the watchdog to 3 sec if it is less than 3 but not equal to 0 */
//	watchdog_timer = 30 * HZ;

	ret = misc_register(&softdog_miscdev);

	if (ret)
		return ret;

//	printk(banner, watchdog_timer/HZ);

	return 0;
}

static void __exit watchdog_exit(void)
{
	misc_deregister(&softdog_miscdev);
}

module_init(watchdog_init);
module_exit(watchdog_exit);
