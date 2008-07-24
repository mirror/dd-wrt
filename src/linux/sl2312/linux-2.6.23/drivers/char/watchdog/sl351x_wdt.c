#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/platform_device.h>
#include <asm/uaccess.h>
#include <asm/arch/sl2312.h>
#include <asm/arch/hardware.h>
#include <asm/arch/irqs.h>
#include <asm/arch/watchdog.h>
#include <asm/io.h>
#include <linux/interrupt.h>

#define WATCHDOG_TEST 1
#define PFX "sl351x-wdt: "

#define _WATCHDOG_COUNTER  0x00
#define _WATCHDOG_LOAD     0x04
#define _WATCHDOG_RESTART  0x08
#define _WATCHDOG_CR       0x0C
#define _WATCHDOG_STATUS   0x10
#define _WATCHDOG_CLEAR    0x14
#define _WATCHDOG_INTRLEN  0x18

static struct resource  *wdt_mem;
static struct resource  *wdt_irq;
static void __iomem     *wdt_base;
static int 		wdt_margin = WATCHDOG_TIMEOUT_MARGIN;	/* in range of 0 .. 60s */

static int open_state = WATCHDOG_DRIVER_CLOSE;
static int wd_expire = 0;

static void watchdog_enable(void)
{
	unsigned long wdcr;

	wdcr = readl(wdt_base + _WATCHDOG_CR);
	wdcr |= (WATCHDOG_WDENABLE_MSK|WATCHDOG_WDRST_MSK);
#ifdef WATCHDOG_TEST
	wdcr |= WATCHDOG_WDINTR_MSK;
//	wdcr &= ~WATCHDOG_WDRST_MSK;
#endif
	wdcr &= ~WATCHDOG_WDCLOCK_MSK;
	writel(wdcr, wdt_base + _WATCHDOG_CR);
}

static void watchdog_set_timeout(unsigned long timeout)
{
	timeout = WATCHDOG_TIMEOUT_SCALE * timeout;
	writel(timeout, wdt_base + _WATCHDOG_LOAD);
	writel(WATCHDOG_RESTART_VALUE, wdt_base + _WATCHDOG_RESTART);
}

static void watchdog_keepalive(void)
{
	writel(WATCHDOG_RESTART_VALUE, wdt_base + _WATCHDOG_RESTART);
}

static void watchdog_disable(void)
{
	unsigned long wdcr;

	wdcr = readl(wdt_base + _WATCHDOG_CR);
	wdcr &= ~WATCHDOG_WDENABLE_MSK;
	writel(wdcr, wdt_base + _WATCHDOG_CR);
}


#ifdef WATCHDOG_TEST
static irqreturn_t watchdog_irq(int irq, void *dev_id, struct pt_regs *regs)
{
	unsigned int clear;

	writel(WATCHDOG_CLEAR_STATUS, wdt_base + _WATCHDOG_CLEAR);
	printk(KERN_INFO PFX "Watchdog timeout, resetting system...\n");

	clear = __raw_readl(IO_ADDRESS(SL2312_INTERRUPT_BASE)+0x0C);
	clear &= 0x01;
	__raw_writel(clear,IO_ADDRESS(SL2312_INTERRUPT_BASE)+0x08);
	wd_expire = 1;
	return IRQ_HANDLED;
}

#endif

#define OPTIONS WDIOF_SETTIMEOUT | WDIOF_KEEPALIVEPING | WDIOF_MAGICCLOSE
static struct watchdog_info sl351x_wdt_ident = {
	.options          =     OPTIONS,
	.firmware_version =     0,
	.identity         =     "sl351x Watchdog",
};

struct file_operations watchdog_fops = {
	.write		= watchdog_write,
	.read		= watchdog_read,
	.open		= watchdog_open,
	.release	= watchdog_release,
	.ioctl		= watchdog_ioctl,
};

static int watchdog_open(struct inode *inode, struct file *filp)
{
	if (open_state == WATCHDOG_DRIVER_OPEN)
		return -EBUSY;

	wd_expire = 0;

	watchdog_disable();
	watchdog_set_timeout(wdt_margin);
	watchdog_enable();

	printk(KERN_INFO PFX "watchog timer enabled, margin: %ds.\n", wdt_margin);
	open_state = WATCHDOG_DRIVER_OPEN;

	return nonseekable_open(inode, filp);
}

static int watchdog_release(struct inode *inode, struct file *filp)
{
	watchdog_disable();

	open_state = WATCHDOG_DRIVER_CLOSE;
	wd_expire = 0;
	printk(KERN_INFO PFX "watchog timer disabled, margin: %ds.\n", wdt_margin);

	return 0;
}

static ssize_t watchdog_read(struct file *filp, char *buf, size_t count, loff_t *off)
{
	int i;
	unsigned long val;


	for(i=0;i< count;i++)
	{
		if ((i%4)==0)
			val = *((unsigned long *)WATCHDOG_COUNTER);
		buf[i] = (val & 0xFF);
		val >>= 8;
	}
	return count;
}

static ssize_t watchdog_write(struct file *filp, const char *buf, size_t len, loff_t *off)
{
	/*  Refresh the timer. */
	if (len) {
		watchdog_keepalive();
	}
	return len;

}

static int watchdog_ioctl(struct inode *inode, struct file *filp,
			  unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	int margin;

	switch(cmd)
	{
	case WDIOC_GETSUPPORT:
		return copy_to_user(argp, &sl351x_wdt_ident,
				    sizeof(sl351x_wdt_ident)) ? -EFAULT : 0;

	case WDIOC_GETSTATUS:
	case WDIOC_GETBOOTSTATUS:
		return put_user(0, (int __user*)argp);

	case WDIOC_KEEPALIVE:
		watchdog_keepalive();
		return 0;

	case WDIOC_SETTIMEOUT:
		if (get_user(margin, (int __user*)argp))
			return -EFAULT;

		/* Arbitrary, can't find the card's limits */
		if ((margin < 0) || (margin > 60))
			return -EINVAL;

		// watchdog_disable();
		wdt_margin = margin;
		watchdog_set_timeout(margin);
		watchdog_keepalive();
		// watchdog_enable();

		/* Fall through */

	case WDIOC_GETTIMEOUT:
		return put_user(wdt_margin, (int *)arg);

	default:
		return -ENOIOCTLCMD;
	}
}

static struct miscdevice wd_dev= {
	WATCHDOG_MINOR,
	"watchdog",
	&watchdog_fops
};

static char banner[] __initdata = KERN_INFO "SL351x Watchdog Timer, (c) 2007 WILIBOX\n";

static int sl351x_wdt_probe(struct platform_device *pdev)
{
	struct resource *res;
	int ret, size;
	unsigned long wdcr;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		printk(KERN_INFO PFX "failed to get memory region resouce\n");
		return -ENOMEM;
	}

	size = (res->end-res->start)+1;

	wdt_mem = request_mem_region(res->start, size, pdev->name);
	if (wdt_mem == NULL) {
		printk(KERN_INFO PFX "failed to get memory region\n");
		return -ENOENT;
	}

	wdt_base = ioremap(res->start, size);
	if (wdt_base == NULL) {
		printk(KERN_INFO PFX "failed to ioremap() region\n");
		return -EINVAL;
	}

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (res == NULL) {
		 printk(KERN_INFO PFX "failed to get irq resource\n");
		 return -ENOENT;
	}

	wdt_irq = res;

	ret = request_irq(res->start, watchdog_irq, 0, pdev->name, pdev);
	if (ret != 0) {
		printk(KERN_INFO PFX "failed to install irq (%d)\n", ret);
		return ret;
	}

	wdcr = readl(wdt_base + _WATCHDOG_CR);
	if (wdcr & WATCHDOG_WDENABLE_MSK) {
		printk(KERN_INFO PFX "Found watchdog in enabled state, reseting ...\n");
		wdcr &= ~WATCHDOG_WDENABLE_MSK;
		writel(wdcr, wdt_base + _WATCHDOG_CR);
	}

	ret = misc_register(&wd_dev);

	return ret;
}

static int sl351x_wdt_remove(struct platform_device *pdev)
{
	if (wdt_base != NULL) {
		iounmap(wdt_base);
		wdt_base = NULL;
	}

	if (wdt_irq != NULL) {
		free_irq(wdt_irq->start, pdev);
		release_resource(wdt_irq);
		wdt_irq = NULL;
	}

	if (wdt_mem != NULL) {
		release_resource(wdt_mem);
		wdt_mem = NULL;
	}

	misc_deregister(&wd_dev);

	return 0;
}

static void sl351x_wdt_shutdown(struct platform_device *dev)
{
	watchdog_disable();
}

#ifdef CONFIG_PM
static int sl351x_wdt_suspend(struct platform_device *dev, pm_message_t state)
{
	watchdog_disable();
}

static int sl351x_wdt_resume(struct platform_device *dev)
{
	watchdog_set_timeout(wdt_margin);
	watchdog_enable();
}

#else
#define sl351x_wdt_suspend	NULL
#define sl351x_wdt_resume	NULL
#endif

static struct platform_driver sl351x_wdt_driver = {
	.probe          = sl351x_wdt_probe,
	.remove         = sl351x_wdt_remove,
	.shutdown       = sl351x_wdt_shutdown,
	.suspend        = sl351x_wdt_suspend,
	.resume         = sl351x_wdt_resume,
	.driver         = {
		.owner  = THIS_MODULE,
		.name   = "sl351x-wdt",
	},
};

static int __init watchdog_init(void)
{
	printk(banner);
	return platform_driver_register(&sl351x_wdt_driver);
}

static void __exit watchdog_exit(void)
{
	platform_driver_unregister(&sl351x_wdt_driver);
}

module_init(watchdog_init);
module_exit(watchdog_exit);
