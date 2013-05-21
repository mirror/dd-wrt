/*
 * (c) Copyright Meraki Networks 2006
 *
 */
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/fs.h>
#include <linux/ioport.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <linux/init.h>
#include <linux/platform_device.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/addrspace.h>

#define AR5315_DSLBASE          0xB1000000      /* RESET CONTROL MMR */

#define AR5315_TIMER            (AR5315_DSLBASE + 0x0030)
#define AR5315_RELOAD           (AR5315_DSLBASE + 0x0034)
#define AR5315_WD               (AR5315_DSLBASE + 0x0038)
#define AR5315_WDC              (AR5315_DSLBASE + 0x003c)
#define AR5315_COLD_RESET       (AR5315_DSLBASE + 0x0000)
#define AR5315_ISR              (AR5315_DSLBASE + 0x0020)
#define AR5315_IMR              (AR5315_DSLBASE + 0x0024)

#define AR531X_APBBASE		0x1c000000
#define AR531X_RESETTMR		(AR531X_APBBASE  + 0x3000)
#define AR531X_WD         	(AR531X_RESETTMR + 0x000c) /* watchdog timer */
#define AR531X_WDC          	(AR531X_RESETTMR + 0x0008) /* watchdog cntrl */
#define AR531X_ISR		(AR531X_RESETTMR + 0x0010) /* Intr Status Reg */
#define AR531X_IMR		(AR531X_RESETTMR + 0x0014) /* Intr Mask Reg */
#define AR531X_RESET		(AR531X_RESETTMR + 0x0020)

#define AR531X_RESET_SYSTEM     0x00000001  /* cold reset full system */

/* Cold Reset */
#define RESET_COLD_AHB              0x00000001
#define RESET_COLD_APB              0x00000002
#define RESET_COLD_CPU              0x00000004
#define RESET_COLD_CPUWARM          0x00000008
#define RESET_SYSTEM                (RESET_COLD_CPU | RESET_COLD_APB | RESET_COLD_AHB | RESET_COLD_CPUWARM)

#define AR531X_MISC_IRQ_BASE		0x20
#define AR531X_MISC_IRQ_WATCHDOG	AR531X_MISC_IRQ_BASE+7

/* AR531X_WD_CTRL register bit field definitions */
/* the watchdog will always signal an interrupt when it expires */
#define AR531X_WD_CTRL_DEFAULT           0
#define AR531X_WD_CTRL_NMI               0x0001 /* make the interrupt an NMI */
#define AR531X_WD_CTRL_RESET             0x0002 /* also reset the board (broken?) */
#define AR531X_WD_CTRL_AHB               0x0004


#define CLOCK_RATE 40000000 /* XXX WD_TIMER cycles/sec (40MHz) */

typedef u_int32_t AR531X_REG;

#define PHYS_TO_K1(physaddr) KSEG1ADDR(physaddr)

#define sysRegRead(phys)	\
	(*(volatile AR531X_REG *)PHYS_TO_K1(phys))

#define sysRegWrite(phys, val)	\
	((*(volatile AR531X_REG *)PHYS_TO_K1(phys)) = (val))

#define S_TO_CYCLES(x) ((x) * CLOCK_RATE)
#define CYCLES_TO_S(x) ((x) / CLOCK_RATE)

static unsigned long wdt_is_open;
static int heartbeat = 60;
static int started = 0;
extern const char *get_arch_type (void);

static void ar2315_wdt_print_info(void)
{
	printk("watchdog hb: %d", heartbeat);
  if (!strcmp (get_arch_type (), "Atheros AR5315"))
  {
	printk("  ISR: 0x%x", sysRegRead(AR5315_ISR));
	printk("  IMR: 0x%x", sysRegRead(AR5315_IMR));
	printk("  WD : 0x%x", sysRegRead(AR5315_WD));
	printk("  WDC: 0x%x\n", sysRegRead(AR5315_WDC));
  }else{
  	printk("  ISR: 0x%x", sysRegRead(AR531X_ISR));
	printk("  IMR: 0x%x", sysRegRead(AR531X_IMR));
	printk("  WD : 0x%x", sysRegRead(AR531X_WD));
	printk("  WDC: 0x%x\n", sysRegRead(AR531X_WDC));
  }
}

static int
ar2315_wdt_set_heartbeat(int hb)
{
	if (hb < 1 || hb > 90) {
		return -EINVAL;
	}
	heartbeat = hb;
	return 0;
}

static void 
ar2315_wdt_ping(void)
{
  if (!strcmp (get_arch_type (), "Atheros AR5315"))
    {
	sysRegWrite(AR5315_WD, S_TO_CYCLES(heartbeat));
	sysRegWrite(AR5315_ISR, 0x80);
    }else
    {
    	sysRegWrite(AR531X_WD, S_TO_CYCLES(heartbeat));
	sysRegWrite(AR531X_ISR, 0x40);
    }
}

static int 
ar2315_wdt_open(struct inode *inode, struct file *file)
{
	if (test_and_set_bit(0, &wdt_is_open))
		return -EBUSY;

	printk("ar2315_wdt: starting watchdog w/timeout %d seconds\n", heartbeat);
	ar2315_wdt_ping();
	// cliff - either one of these seems to just hang the board when the watchdog expires
	//sysRegWrite(AR5315_WDC, AR531X_WD_CTRL_NMI);
	//sysRegWrite(AR5315_WDC, AR531X_WD_CTRL_RESET);
	ar2315_wdt_print_info();

	started = 1;
	return nonseekable_open(inode, file);
}

static ssize_t 
ar2315_wdt_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	if (count) {
		ar2315_wdt_ping();
	}
	return count;
}

static int 
ar2315_wdt_release(struct inode *inode, struct file *file)
{
  if (!strcmp (get_arch_type (), "Atheros AR5315"))
  {
	printk("%s: release with %d left\n", __FUNCTION__, CYCLES_TO_S(sysRegRead(AR5315_WD)));
  }else
  {
	printk("%s: release with %d left\n", __FUNCTION__, CYCLES_TO_S(sysRegRead(AR531X_WD)));  
  }
	// uncomment the below if closing this device should prevent the
	// watchdog from rebooting the system
	//started = 0;
	//sysRegWrite(AR5315_WDC, AR531X_WD_CTRL_DEFAULT);
	//sysRegWrite(AR5315_WD, 0);

	clear_bit(0, &wdt_is_open);
	return 0;
}

static irqreturn_t 
ar2315_wdt_interrupt(int irq, void *dev_id)
{
	printk(KERN_CRIT "watchdog expired!\n");
	ar2315_wdt_print_info();

  if (!strcmp (get_arch_type (), "Atheros AR5315"))
  {
	if (started) {
		printk(KERN_CRIT "Watchdog rebooting...\n");
//		sysRegWrite(AR5315_COLD_RESET, RESET_SYSTEM);
		emergency_restart(); //2315 needs gpio based restart unlike 2317
		
	} else {
		sysRegWrite(AR5315_WDC, AR531X_WD_CTRL_DEFAULT);
		sysRegWrite(AR5315_WD, 0);
		// clear the interrupt
		sysRegWrite(AR5315_ISR, 0x80);
	}
  }else{
	if (started) {
		printk(KERN_CRIT "Watchdog rebooting...\n");
		sysRegWrite(AR531X_RESET, AR531X_RESET_SYSTEM);	
	} else {
		sysRegWrite(AR531X_WDC, AR531X_WD_CTRL_DEFAULT);
		sysRegWrite(AR531X_WD, 0);
		// clear the interrupt
		sysRegWrite(AR531X_ISR, 0x40);
	}
  
  
  }
	return IRQ_HANDLED;
}

static long
ar2315_wdt_ioctl(struct file *file, unsigned int cmd,
		 unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	int __user *p = argp;
	int new_heartbeat;
	int status = 0;

	static struct watchdog_info ident = {
		.options =		WDIOF_SETTIMEOUT |
					WDIOF_MAGICCLOSE |
					WDIOF_KEEPALIVEPING,
		.firmware_version =	1,
		.identity =		"ar2315",
	};

	switch(cmd)
	{
		default:
			return -ENOIOCTLCMD;
		case WDIOC_GETSUPPORT:
			return copy_to_user(argp, &ident, sizeof(ident)) ? -EFAULT : 0;
		case WDIOC_GETSTATUS:
			//wdt_get_status(&status);
			return put_user(status, p);
		case WDIOC_GETBOOTSTATUS:
			return put_user(0, p);
		case WDIOC_KEEPALIVE:
			ar2315_wdt_ping();
			return 0;
		case WDIOC_SETTIMEOUT:
			if (get_user(new_heartbeat, p))
				return -EFAULT;

			if (ar2315_wdt_set_heartbeat(new_heartbeat))
				return -EINVAL;

			ar2315_wdt_ping();
			/* fallthrough */
		case WDIOC_GETTIMEOUT:
			return put_user(heartbeat, p);
	}
}

static struct file_operations ar2315_wdt_fops = {
	.owner		= THIS_MODULE,
	.llseek		= no_llseek,
	.write		= ar2315_wdt_write,
	.unlocked_ioctl		= ar2315_wdt_ioctl,
	.open		= ar2315_wdt_open,
	.release	= ar2315_wdt_release,
};

static struct miscdevice ar2315_wdt_miscdev = {
	.minor	= WATCHDOG_MINOR,
	.name	= "watchdog",
	.fops	= &ar2315_wdt_fops,
};

static int
ar2315_wdt_probe(struct platform_device *dev)
{
	int ret = 0;
	ret = request_irq(AR531X_MISC_IRQ_WATCHDOG, ar2315_wdt_interrupt, IRQF_DISABLED, "ar2315_wdt", NULL);
	if (ret) {
		printk(KERN_ERR "wdt: IRQ %d is not free.\n", AR531X_MISC_IRQ_WATCHDOG);
		goto out;
	}

	if (ar2315_wdt_set_heartbeat(heartbeat)) {
		ar2315_wdt_set_heartbeat(60);
		printk(KERN_INFO "%s: heartbeat value must be 0<heartbeat<90, using %d\n",
		       __func__, 5);
	}
	ar2315_wdt_print_info();
	ar2315_wdt_ping();
	printk("%s using heartbeat %d s cycles %u\n", __func__, heartbeat, S_TO_CYCLES(heartbeat));
	ar2315_wdt_print_info();
	ret = misc_register(&ar2315_wdt_miscdev);
	if (ret) {
		printk(KERN_ERR "%s: cannot register miscdev on minor=%d (err=%d)\n",
		       __func__, WATCHDOG_MINOR, ret);
		goto out1;
	}
 out:
	return ret;
 out1:
	misc_deregister(&ar2315_wdt_miscdev);
	return ret;
}

static int
ar2315_wdt_remove(struct platform_device *dev)
{
	misc_deregister(&ar2315_wdt_miscdev);
	free_irq(AR531X_MISC_IRQ_WATCHDOG, NULL);
	return 0;
}

static struct platform_driver ar2315_wdt_driver = {
	.probe = ar2315_wdt_probe,
	.remove = ar2315_wdt_remove,
	.driver = {
		.name = "ar2315_wdt",
		.owner = THIS_MODULE,
	},
};



static int __init
init_ar2315_wdt(void)
{
	int ret = platform_driver_register(&ar2315_wdt_driver);
	if(ret)
		printk(KERN_INFO "ar2315_wdt: error registering platfom driver!");
	return ret;
}

static void __exit
exit_ar2315_wdt(void)
{
	platform_driver_unregister(&ar2315_wdt_driver);
}

module_init(init_ar2315_wdt);
module_exit(exit_ar2315_wdt);
