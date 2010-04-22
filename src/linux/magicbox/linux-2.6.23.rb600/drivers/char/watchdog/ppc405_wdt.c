/*
 * IBM PowerPC 405 Watchdog: A Simple Hardware Watchdog Device
 * Based on PowerPC 8xx driver by Scott Anderson which was
 * based on MixCom driver by Gergely Madarasz which was
 * based on Softdog driver by Alan Cox and PC Watchdog driver by Ken Hollis
 *
 * FILE NAME ppc405_wdt.c
 * 
 *  Armin Kuster akuster@mvista.com or source@mvista.com
 *  Sept, 2001
 *
 *  Orignial driver
 *  Author: MontaVista Software, Inc.  <source@mvista.com>
 *          Debbie Chu   <debbie_chu@mvista.com>
 *          Frank Rowand <frank_rowand@mvista.com>
 *
 * Copyright 2000 MontaVista Software Inc.
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Version 0.1 (00/06/05):
 * Version 0.2 (00/07/12) by Debbie Chu
 * Version 0.4 (01/09/19) by Armin kuster
 * Version 0.5 (01/10/10) Akuster
 * 		- removed ppc4xx_restart w/ machine_restart
 */

#define VERSION "0.5" 

#define WDT_DEFAULT_PERIOD	120	/* system default 2 minutes */
#define	MAXONEHOUR		3600UL	/* Max timeout period 60 minutes */
#define	TENMSBASE		10000UL	/* 10 ms */
#define MICROSECBASE		1000000
  
#include <linux/module.h>
#include <linux/autoconf.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/reboot.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/machdep.h>

#define WDIOC_GETPERIOD         _IOR(WATCHDOG_IOCTL_BASE, 6, int)
#define WDIOC_SETPERIOD         _IOW(WATCHDOG_IOCTL_BASE, 7, int)

int wdt_enable;
unsigned long wdt_period;
unsigned long wdt_heartbeat_count;
static unsigned long wdt_count;
int wdt_default = 1;

static int ppc405wd_opened;

static inline void
ppc405wd_update_timer(void)
{
	wdt_heartbeat_count = wdt_count; 
}

void 
ppc4xx_wdt_heartbeat(void)
{
	if (wdt_default ){
		/* default used until wdt inits */
		wdt_heartbeat_count = wdt_period * HZ;
		wdt_default = 0;
	}	
	if ((wdt_heartbeat_count > 0) || ( !wdt_enable )) {
		if (wdt_heartbeat_count > 0)
			wdt_heartbeat_count--;
		mtspr(SPRN_TSR, (TSR_ENW | TSR_WIS));
        } else
		machine_restart("Watchdog Timer Timed out, system reset!");
	ppc_md.heartbeat_count = 0;
}

/*
 *	Allow only one person to hold it open
 */
static int ppc405wd_open(struct inode *inode, struct file *file)
{
	unsigned int tcr_value;

	if(test_and_set_bit(0,&ppc405wd_opened))
		return -EBUSY;

	//MOD_INC_USE_COUNT;

	/* 
	 * There are three ways to enable the watchdog timer:
	 * 1. turn on the watchdog in the bootrom.
	 * 2. turn on the watchdog using the boot command line option,
	 *    you can specifiy "wdt=<timeout>" on the boot cmdline
	 * 3. turn on the watchdog in this routine,
	 *    the default timer period is set to 2 minutes.
	 */

	tcr_value = mfspr(SPRN_TCR);

	if ((tcr_value & TCR_WRC_MASK) != TCR_WRC(WRC_SYSTEM)) {
	    /* 
	     * watchdog reset not enabled yet, enable it
	     * The default timer period is set to 2 minutes.
	     */
#ifdef  DEBUG_WDT
                mtspr(SPRN_TCR,
		   (mfspr(SPRN_TCR) & ~TCR_WP_MASK & ~TCR_WRC_MASK) |
                   TCR_WP(WP_2_29) |
                   TCR_WRC(WRC_SYSTEM));
#else
	    mtspr(SPRN_TCR,
		   (mfspr(SPRN_TCR) & ~TCR_WP_MASK & ~TCR_WRC_MASK) |
		   TCR_WP(WP_2_25) |
		   TCR_WRC(WRC_SYSTEM));
#endif
	}

	if (wdt_period == 0)
		wdt_period = WDT_DEFAULT_PERIOD;

	wdt_count = wdt_period * HZ;
	ppc405wd_update_timer();
	wdt_enable = 1;
	mtspr(SPRN_TSR, (TSR_ENW | TSR_WIS));
	return 0;
}

static int ppc405wd_release(struct inode *inode, struct file *file)
{
	//MOD_DEC_USE_COUNT;

	clear_bit(0,&ppc405wd_opened);
	return 0;
}

static ssize_t ppc405wd_write(struct file *file, const char *data, size_t len, loff_t *ppos)
{
	/*  Can't seek (pwrite) on this device  */
	if (ppos != &file->f_pos)
		return -ESPIPE;

	if (len) {
		ppc405wd_update_timer();
		return 1;
	}
	return 0;
}

static int ppc405wd_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	unsigned long period;
	int status;
	int state;
	static struct watchdog_info ident = {
		WDIOF_KEEPALIVEPING,
		0,
		"PPC 405 watchdog"
	};
                                        
	switch (cmd) {
		case WDIOC_GETSTATUS:
			status = ppc405wd_opened;

			if (copy_to_user((int *)arg, &status, sizeof(int)))
				return -EFAULT;
			break;
		case WDIOC_GETSUPPORT:
			if (copy_to_user((struct watchdog_info *)arg, &ident, 
			    sizeof(ident))) {
				return -EFAULT;
			}
			break;
		case WDIOC_KEEPALIVE:
			ppc405wd_update_timer();
			break;
		case WDIOC_SETOPTIONS:
			if(copy_from_user(&state, (int*) arg, sizeof(int)))
				return -EFAULT;
			if (state & WDIOS_DISABLECARD) {
				wdt_enable = 0;
				printk(KERN_NOTICE "Soft watchdog timer is disabled\n");
				break;
			}
                        if (state & WDIOS_ENABLECARD) {
				ppc405wd_update_timer();
				wdt_enable = 1;
				mtspr(SPRN_TSR, (TSR_ENW | TSR_WIS));
				printk(KERN_NOTICE "Soft watchdog timer is enabled\n");
				break;
			}
		case WDIOC_GETPERIOD:
			/* return watchdog period (units = microseconds) */
			period = (wdt_period / HZ) * MICROSECBASE;
			if (copy_to_user((unsigned long *)arg, &period, 
			    sizeof(period))) {
				return -EFAULT;
			}
			break;
		case WDIOC_SETPERIOD:
			/*
			** set watchdog period (units = microseconds)
			** value of zero means maximum
			**
			** Don't set a watchdog period to a value less than
			** the requested value (period will be rounded up to
			** next available interval the watchdog supports).
			**
			** The software watchdog will expire at some point in
			** the range of (rounded up period) ..
			** (rounded up period + 1 jiffie).  If interrupts are
			** disabled so that the software watchdog is unable to
			** reset the system, then the hardware watchdog will
			** eventually reset the system.
			*/
			if (copy_from_user(&period, (unsigned long *)arg,
			    sizeof(period))) {
				return -EFAULT;
			}

			/* 
			** This code assumes HZ is 100.  Need to remove that
			** assumption.
			*/

			/*
			** The minimum period of ppc405wd_timer is a jiffie,
			** which is 10 msec when HZ is 100.  The units of
			** wdt_period is jiffies.
			**
			** The new timer period will be used at the next
			** heartbeat.
			*/
			if (period == 0)
				period = MAXONEHOUR * MICROSECBASE;        

			wdt_count = (period / TENMSBASE) + (period % TENMSBASE ? 1 : 0);
			ppc405wd_update_timer();

			break;
		default:
			return -ENOIOCTLCMD;

	}
	return 0;
}

static struct file_operations ppc405wd_fops =
{
	owner: THIS_MODULE,
	write: ppc405wd_write,
	ioctl: ppc405wd_ioctl,
	open: ppc405wd_open,
	release: ppc405wd_release,
};

static struct miscdevice ppc405wd_miscdev =
{
	.minor = WATCHDOG_MINOR,
	.name = "405_watchdog",
	.fops = &ppc405wd_fops
};

static int __init ppc405wd_init(void)
{
	misc_register(&ppc405wd_miscdev);
	printk(KERN_NOTICE "PPC 405 watchdog driver v%s\n", VERSION);
	if (wdt_period == 0)
	wdt_period = WDT_DEFAULT_PERIOD;
	wdt_count = wdt_period * HZ;
	ppc405wd_update_timer();
	mtspr(SPRN_TSR, (TSR_ENW | TSR_WIS));
	mtspr(SPRN_TCR, TCR_PIE | TCR_ARE);
	return 0;
}	

void __exit ppc405wd_exit(void)
{
	misc_deregister(&ppc405wd_miscdev);
}

module_init(ppc405wd_init);
module_exit(ppc405wd_exit);
