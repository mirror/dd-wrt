/*
 * Watchdog driver for ARM MPcore
 *
 * Copyright (C) 2017 Felix Fietkau <nbd@nbd.name>
 */

#include <linux/export.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/watchdog.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <asm/smp_twd.h>

static void __iomem *wdt_base;
static int wdt_timeout = 60;

static int mpcore_wdt_keepalive(struct watchdog_device *wdd)
{
	static int perturb;
	u32 count;

	count = (twd_timer_get_rate() / 256) * wdt_timeout;

	/* Reload register needs a different value on each refresh */
	count += perturb;
	perturb = !perturb;

	iowrite32(count, wdt_base + TWD_WDOG_LOAD);

	return 0;
}

static int mpcore_wdt_start(struct watchdog_device *wdd)
{
	mpcore_wdt_keepalive(wdd);

	/* prescale = 256, mode = 1, enable = 1 */
	iowrite32(0x0000FF09, wdt_base + TWD_WDOG_CONTROL);

	return 0;
}

static int mpcore_wdt_stop(struct watchdog_device *wdd)
{
	iowrite32(0x12345678, wdt_base + TWD_WDOG_DISABLE);
	iowrite32(0x87654321, wdt_base + TWD_WDOG_DISABLE);
	iowrite32(0x0, wdt_base + TWD_WDOG_CONTROL);

	return 0;
}

static int mpcore_wdt_set_timeout(struct watchdog_device *wdd,
				 unsigned int timeout)
{
	mpcore_wdt_stop(wdd);
	wdt_timeout = timeout;
	mpcore_wdt_start(wdd);

	return 0;
}

static const struct watchdog_info mpcore_wdt_info = {
	.options = WDIOF_SETTIMEOUT | WDIOF_KEEPALIVEPING | WDIOF_MAGICCLOSE,
	.identity = "MPcore Watchdog",
};

static const struct watchdog_ops mpcore_wdt_ops = {
	.owner = THIS_MODULE,
	.start = mpcore_wdt_start,
	.stop  = mpcore_wdt_stop,
	.ping  = mpcore_wdt_keepalive,
	.set_timeout = mpcore_wdt_set_timeout,
};

static struct watchdog_device mpcore_wdt = {
	.info = &mpcore_wdt_info,
	.ops = &mpcore_wdt_ops,
	.min_timeout = 1,
	.max_timeout = 65535,
};

static int mpcore_wdt_probe(struct platform_device *pdev)
{
	struct resource *res;
	unsigned long rate = twd_timer_get_rate();

	pr_info("MPCore WD init. clockrate: %u prescaler: %u countrate: %u timeout: %us\n", rate, 256, rate / 256, wdt_timeout);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
		return -ENODEV;

	wdt_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(wdt_base))
		return PTR_ERR(wdt_base);

	watchdog_register_device(&mpcore_wdt);
	return 0;
}

static int mpcore_wdt_remove(struct platform_device *dev)
{
	watchdog_unregister_device(&mpcore_wdt);
	return 0;
}

static struct platform_driver mpcore_wdt_driver = {
	.probe		= mpcore_wdt_probe,
	.remove		= mpcore_wdt_remove,
	.driver		= {
		.name	= "mpcore_wdt",
	},
};

module_platform_driver(mpcore_wdt_driver);
MODULE_AUTHOR("Felix Fietkau <nbd@nbd.name>");
MODULE_LICENSE("GPL");
