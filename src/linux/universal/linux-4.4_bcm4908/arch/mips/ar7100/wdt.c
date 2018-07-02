#include <linux/autoconf.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/resource.h>

#include <linux/console.h>
#include <asm/serial.h>

#include <linux/tty.h>
#include <linux/time.h>
#include <linux/serial_core.h>
#include <linux/serial.h>
#include <linux/serial_8250.h>
#include <linux/miscdevice.h>

#include <asm/mach-ar7100/ar7100.h>
#include <asm/delay.h>

#define AR7100_DEFAULT_WD_TMO	(20ul * USEC_PER_SEC)

#define FACTORY_RESET		0x89ABCDEF

#define AR7100_GPIO_RESET	21

#ifdef AR7100_WDT_TEST_CODE
#	define wddbg printk
#else
#	define wddbg(junk, ...)
#endif				/* AR7100_WDT_TEST_CODE 8 */

extern uint32_t ar71xx_ahb_freq;

typedef struct {
	int open:1, can_close:1, tmo, action;
	wait_queue_head_t wq;
} ar7100_wdt_t;

static ar7100_wdt_t wdt_softc_array;

static ar7100_wdt_t *wdt = &wdt_softc_array;

irqreturn_t ar7100_wdt_isr(int, void *);

static struct irqaction ar7100_wdt_irq = {
	.handler = ar7100_wdt_isr,
	.flags = IRQF_DISABLED,
	.name = "ar7100_wdt_isr",
};

#ifdef AR7100_WDT_TEST_CODE
/* Return the value present in the watchdog register */
static inline uint32_t ar7100_get_wd_timer(void)
{
	uint32_t val;

	val = (uint32_t) ar7100_reg_rd(AR7100_WATCHDOG_TMR);
	val = (val * USEC_PER_SEC) / ar71xx_ahb_freq;

	return val;
}
#endif				/* AR7100_WDT_TEST_CODE */

/* Set the timeout value in the watchdog register */
static inline void ar7100_set_wd_timer(uint32_t usec /* micro seconds */ )
{
	usec = usec * (ar71xx_ahb_freq / USEC_PER_SEC);

	wddbg("%s: 0x%08x\n", __func__, usec);

	ar7100_reg_wr(AR7100_WATCHDOG_TMR, usec);
}

static inline int ar7100_set_wd_timer_action(uint32_t val)
{
	if (val & ~AR7100_WD_ACT_MASK) {
		return EINVAL;
	}

	wdt->action = val;

	/*
	 * bits  : 31 30 - 2 0-1
	 * access: RO  rsvd  Action
	 *
	 * Since bit 31 is read only and rest of the bits
	 * are zero, don't have to do a read-modify-write
	 */
	ar7100_reg_wr(AR7100_WATCHDOG_TMR_CONTROL, val);
	return 0;
}

#ifdef AR7100_WDT_TEST_CODE
static inline uint32_t ar7100_get_wd_timer_action(void)
{
	return (uint32_t) (ar7100_reg_rd(AR7100_WATCHDOG_TMR_CONTROL) & AR7100_WD_ACT_MASK);
}

static inline uint32_t ar7100_get_wd_timer_last(void)
{
	return ((uint32_t) (ar7100_reg_rd(AR7100_WATCHDOG_TMR_CONTROL) & AR7100_WD_LAST_MASK) >> AR7100_WD_LAST_SHIFT);
}
#endif				/* AR7100_WDT_TEST_CODE */

irqreturn_t __init ar7100_wdt_isr(int cpl, void *dev_id)
{
	unsigned delay;
	extern int ar7100_gpio_in_val(int);

#define UDELAY_COUNT 4000

	wddbg("%s: invoked\n", __func__);

	for (delay = UDELAY_COUNT; delay; delay--) {
		if (ar7100_gpio_in_val(AR7100_GPIO_RESET)) {
			break;
		}
		udelay(1000);
	}

	wddbg("%s: %d", __func__, delay);

	if (!delay) {
		wake_up(&wdt->wq);
	} else {
		extern void ar7100_restart(char *);
		ar7100_restart(NULL);
	}
	return IRQ_HANDLED;
}

static __init int ar7100wdt_open(struct inode *inode, struct file *file)
{
	wddbg("%s: called\n", __func__);

	if (MINOR(inode->i_rdev) != WATCHDOG_MINOR) {
		return -ENODEV;
	}

	if (wdt->open) {
		return -EBUSY;
	}

	wdt->open = 1;
	wdt->tmo = AR7100_DEFAULT_WD_TMO;
	wdt->action = AR7100_WD_ACT_NONE;
	wdt->can_close = 0;
	init_waitqueue_head(&wdt->wq);

	ar7100_set_wd_timer(wdt->tmo);
	ar7100_set_wd_timer_action(AR7100_WD_ACT_NONE);

	return nonseekable_open(inode, file);
}

static __init int ar7100wdt_close(struct inode *inode, struct file *file)
{
	wddbg("%s: called\n", __func__);

	if (MINOR(inode->i_rdev) != WATCHDOG_MINOR) {
		return -ENODEV;
	}

	if (!wdt->can_close) {
		wddbg("%s: clearing action\n", __func__);
		ar7100_set_wd_timer_action(AR7100_WD_ACT_NONE);
	} else {
		wddbg("%s: not clearing action\n", __func__);
	}
	wdt->open = 0;
	return 0;
}

static __init ssize_t ar7100wdt_read(struct file *file, char *buf, size_t count, loff_t * ppos)
{
	wddbg("%s: called\n", __func__);

	return -ENOTSUPP;
}

static __init int ar7100wdt_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;

	wddbg("%s: called\n", __func__);

	switch (cmd) {
	case FACTORY_RESET:
		wddbg("%s: intr action\n", __func__);

		if ((ret = request_irq(AR7100_MISC_IRQ_WATCHDOG, ar7100_wdt_isr, IRQF_DISABLED, "Watchdog Timer", wdt))) {
			wddbg("%s: request_irq %d\n", __func__, ret);
			return ret;
		}

		ar7100_set_wd_timer_action(AR7100_WD_ACT_GP_INTR);
		sleep_on(&wdt->wq);
		free_irq(AR7100_MISC_IRQ_WATCHDOG, wdt);
		break;

	default:
		ret = -EINVAL;
	}

	return ret;
}

static __init ssize_t ar7100wdt_write(struct file *file, const char *buf, size_t count, loff_t * ppos)
{
	int i;
	char c;

	wddbg("%s: called\n", __func__);

	for (i = 0; i != count; i++) {
		if (get_user(c, buf + i)) {
			return -EFAULT;
		}

		if (c == 'V') {
			wdt->can_close = 1;
			break;
		}
	}

	if (i) {
		ar7100_set_wd_timer(wdt->tmo);
		return 1;
	}

	return 0;
}

int __init ar7100wdt_init(void)
{
	struct file_operations ar7100wdt_fops = {
	      read:ar7100wdt_read,
	      write:ar7100wdt_write,
	      ioctl:ar7100wdt_ioctl,
	      open:ar7100wdt_open,
	      release:ar7100wdt_close
	};
	struct miscdevice ar7100wdt_miscdev = {
		WATCHDOG_MINOR,
		"watchdog",
		&ar7100wdt_fops
	};
	int ret;
	extern void ar7100_gpio_config_input(int);

	printk("%s: Registering WDT ", __func__);
	if ((ret = misc_register(&ar7100wdt_miscdev))) {
		printk("failed %d\n", ret);
		return -1;
	} else {
		printk("success\n");
	}

	ar7100_gpio_config_input(AR7100_GPIO_RESET);
	return 0;
}

late_initcall(ar7100wdt_init);
