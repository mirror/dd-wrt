/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright © 2003 Atheros Communications, Inc.,  All Rights Reserved.
 */

/*
 * Support for the Watchdog Timer
 */

#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/slab.h>
#include <linux/ioport.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/system.h>
#include <asm/delay.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/smp_lock.h>
#include <linux/wait.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include "ar531xlnx.h"
#include "ar531x.h"
#define FACTORY_RESET 0x89ABCDEF     
/*
 * The AR5312/AR2313 hardware supports a hardware watchdog timer.
 * The timer is programmed to decrement until it hits zero.  Then
 * it generates either SYSTEM RESET or an NMI.  In order to avoid
 * one of these events, software must reset the watchdog timer
 * before it expires.
 */

/*
 * Macros to convert between "watchdog ticks" and milliseconds.
 * The watchdog counts down at the system frequency (1/4 CPU freq).
 */
#define wdticks_to_ms(wdticks) (((wdticks) * 1000) / ar531x_sys_frequency())
#define ms_to_wdticks(ms) (((ms) / 1000) * ar531x_sys_frequency())

#define AR531X_WATCHDOG_MIN_MS 1000
#define AR531X_WATCHDOG_DEFAULT_S 60
#define UDELAY_COUNT 4*1000

DECLARE_WAIT_QUEUE_HEAD(fact_reset_queue);
static struct ar531x_boarddata *ar531x_board_configuration=NULL;

static unsigned int ar531x_watchdog_timer_value = 0;
static unsigned long udelay;

static int ar531xwdt_is_open;
static int expect_close = 0;

static int margin = AR531X_WATCHDOG_DEFAULT_S; /* WDT timeout value in seconds */

module_param(margin,int,0);
MODULE_PARM_DESC(margin, "Watchdog timeout value");

#ifdef CONFIG_WATCHDOG_NOWAYOUT
static int nowayout = 1;
#else
static int nowayout = 0;
#endif

module_param(nowayout,int,0);
MODULE_PARM_DESC(nowayout, "Watchdog cannot be stopped once started (default=CONFIG_WATCHDOG_NOWAYOUT)");

void sysReset(void)
{
	 for(;;) {
#if CONFIG_AR531X_COBRA
#if CONFIG_AR5315
    /*
    ** Cold reset does not work,work around is to use the GPIO reset bit.
    */
    unsigned int reg;
    reg = sysRegRead(AR531XPLUS_GPIO_DO);
    reg &= ~(1 << AR531XPLUS_RESET_GPIO);
    sysRegWrite(AR531XPLUS_GPIO_DO, reg);
    (void)sysRegRead(AR531XPLUS_GPIO_DO); /* flush write to hardware */

#elif CONFIG_AR5317
        sysRegWrite(AR531XPLUS_COLD_RESET,AR5317_RESET_SYSTEM);
#endif
#else
        sysRegWrite(AR531X_RESET, AR531X_RESET_SYSTEM);
#endif /* CONFIG_AR531X_COBRA */
    }
} 

void factory_reset_intr(int cpl, void *dev_id)
{
        udelay = UDELAY_COUNT;
        while (udelay) {
#ifdef CONFIG_AR531X_COBRA
                if (sysRegRead(AR531XPLUS_GPIO_DI) & (1 << ar531x_board_configuration->resetConfigGpio)) {
#else
                if (sysRegRead(AR531X_GPIO_DI) & (1 << ar531x_board_configuration->resetConfigGpio)) {
#endif
                        udelay--;
		}
                else
                        break;
                udelay(1000);

        }
	if(!udelay) {
		disable_irq(AR531X_GPIO_IRQ_BASE+ar531x_board_configuration->resetConfigGpio);
		wake_up(&fact_reset_queue);	
	}
	else {
		sysReset();
	}
}

/*
 * WatchDog Interface
 */

/*
 * Notify the watchdog that we're alive.
 * Reset the watchdog timer.
 */
void
watchdog_notify_alive(void)
{
#ifdef CONFIG_AR531X_COBRA
	sysRegWrite(AR531XPLUS_WD, ar531x_watchdog_timer_value);
#else
	sysRegWrite(AR531X_WD_TIMER, ar531x_watchdog_timer_value);
#endif
	sysWbFlush();
}

/*
 * Start the watchdog timer.
 */
int
watchdog_start(unsigned int milliseconds)
{
	if (milliseconds < AR531X_WATCHDOG_MIN_MS)
		return -1;

	ar531x_watchdog_timer_value = ms_to_wdticks(milliseconds);
	watchdog_notify_alive(); /* Initialize timer */

#ifdef CONFIG_AR531X_COBRA
	/*
	 * Cause AHB error interrupt on watchdog expiration.
	 * See bug 14407 for details.
	 */
	sysRegWrite(AR531XPLUS_WDC, WDC_AHB_INTR);
#else
	/*
	 * Cause a system RESET on watchdog expiration.
	 */
	sysRegWrite(AR531X_WD_CTRL, AR531X_WD_CTRL_RESET);
#endif

    return 0;
}

/*
 * Stop the watchdog timer.
 */
int
watchdog_stop(void)
{
#ifdef CONFIG_AR531X_COBRA
	sysRegWrite(AR531XPLUS_WD, WDC_IGNORE_EXPIRATION);
#else
	sysRegWrite(AR531X_WD_CTRL, AR531X_WD_CTRL_IGNORE_EXPIRATION);
#endif
	return 0;
}

/*
 * Kernel Methods
 */
static ssize_t
ar531xwdt_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	return -EINVAL;
}

static ssize_t
ar531xwdt_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
	/* Can't seek (pwrite) on this device */
	if (ppos != &file->f_pos)
		return -ESPIPE;

	if (count) {
		if (!nowayout) {
			size_t i;

			/* In case it was set long ago */
			expect_close = 0;

			for (i = 0; i != count; i++) {
				char c;
				if (get_user(c, buf + i))
					return -EFAULT;
				if (c == 'V')
					expect_close = 1;
			}
		}
		watchdog_notify_alive();
		return 1;
	}
	return 0;
}

extern char *get_board_config(void);

static int
ar531xwdt_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
	  unsigned long arg)
{
	switch(cmd) {
		case FACTORY_RESET:
			if ((ar531x_board_configuration = get_board_config()) == NULL)
				return -EINVAL;
			if (request_irq(AR531X_GPIO_IRQ_BASE+ar531x_board_configuration->resetConfigGpio,factory_reset_intr,SA_INTERRUPT,"reset_handler",NULL)) {
				return -EINVAL;
			}
			sleep_on(&fact_reset_queue);
	}
	return 0;
			
}

static int
ar531xwdt_open(struct inode *inode, struct file *file)
{
	switch (MINOR(inode->i_rdev)) {
		case WATCHDOG_MINOR:
			
			if (ar531xwdt_is_open)
				return -EBUSY;

			/* Activate watchdog */
			if (watchdog_start(margin * 1000) < 0)
				return -EINVAL;

			ar531xwdt_is_open = 1;
			return 0;
		default:
			return -ENODEV;
	}
	return 0;
}

static int
ar531xwdt_close(struct inode *inode, struct file *file)
{
	if (MINOR(inode->i_rdev) == WATCHDOG_MINOR) {
		if (expect_close)
			watchdog_stop();
		else
			printk("WDT device closed unexpectedly. WDT will not stop!\n");

		ar531xwdt_is_open = 0;
	}
	return 0;
}

/*
 *	Notifier for system down
 */

static int
ar531xwdt_notify_sys(struct notifier_block *this, unsigned long code,
	void *unused)
{
	if (code == SYS_DOWN || code == SYS_HALT) {
		/* Turn the WDT off */
		watchdog_stop();
	}
	return NOTIFY_DONE;
}

/*
 *	Kernel Interfaces
 */

static struct file_operations ar531xwdt_fops = {
	owner:		THIS_MODULE,
	read:		ar531xwdt_read,
	write:		ar531xwdt_write,
	ioctl:		ar531xwdt_ioctl,
	open:		ar531xwdt_open,
	release:	ar531xwdt_close,
};

static struct miscdevice ar531xwdt_miscdev = {
	WATCHDOG_MINOR,
	"watchdog",
	&ar531xwdt_fops
};

/*
 *	The WDT needs to learn about soft shutdowns in order to
 *	turn the watchdog registers off.
 */

static struct notifier_block ar531xwdt_notifier = {
	ar531xwdt_notify_sys,
	NULL,
	0
};

static int __init
ar531xwdt_init(void)
{
	spin_lock_init(&ar531xwdt_lock);
	misc_register(&ar531xwdt_miscdev);
	register_reboot_notifier(&ar531xwdt_notifier);
	return 0;
}

static void __exit
ar531xwdt_exit(void)
{
	misc_deregister(&ar531xwdt_miscdev);
	unregister_reboot_notifier(&ar531xwdt_notifier);
}

module_init(ar531xwdt_init);
module_exit(ar531xwdt_exit);

MODULE_AUTHOR("Atheros Communications, Inc.");
MODULE_DESCRIPTION("Support for Atheros WiSoC Watchdog Timer");
#ifdef MODULE_LICENSE
MODULE_LICENSE("Atheros");
#endif
