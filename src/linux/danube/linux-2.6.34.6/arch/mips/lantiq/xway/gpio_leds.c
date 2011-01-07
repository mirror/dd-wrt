/*
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 *  Copyright (C) 2007 John Crispin <blogic@openwrt.org>
 *
 */

#include <linux/slab.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/gpio.h>

#include <xway.h>

#define LQ_STP_BASE			0x1E100BB0
#define LQ_STP_SIZE			0x40

#define LQ_STP_CON0			0x00
#define LQ_STP_CON1			0x04
#define LQ_STP_CPU0			0x08
#define LQ_STP_CPU1			0x0C
#define LQ_STP_AR			0x10

#define STP_CON0_SWU		(1 << 31)

#define LQ_STP_2HZ			(0)
#define LQ_STP_4HZ			(1 << 23)
#define LQ_STP_8HZ			(2 << 23)
#define LQ_STP_10HZ			(3 << 23)
#define LQ_STP_MASK			(0xf << 23)

#define LQ_STP_UPD_SRC_FPI	(1 << 31)
#define LQ_STP_UPD_MASK		(3 << 30)
#define LQ_STP_ADSL_SRC		(3 << 24)

#define LQ_STP_GROUP0		(1 << 0)

#define LQ_STP_RISING		0
#define LQ_STP_FALLING		(1 << 26)
#define LQ_STP_EDGE_MASK	(1 << 26)

#define lq_stp_r32(reg)				__raw_readl(virt + reg)
#define lq_stp_w32(val, reg)		__raw_writel(val, virt + reg)
#define lq_stp_w32_mask(clear, set, reg) \
        lq_w32((lq_r32(virt + reg) & ~clear) | set, virt + reg)

static int shadow = 0xffff;
static void __iomem *virt;

static int
lq_stp_direction_output(struct gpio_chip *chip, unsigned offset, int value)
{
	return 0;
}

static void
lq_stp_set(struct gpio_chip *chip, unsigned offset, int value)
{
	if(value)
		shadow |= (1 << offset);
	else
		shadow &= ~(1 << offset);
	lq_stp_w32(shadow, LQ_STP_CPU0);
}

static struct gpio_chip lq_stp_chip =
{
	.label = "lq_stp",
	.direction_output = lq_stp_direction_output,
	.set = lq_stp_set,
	.base = 48,
	.ngpio = 24,
	.can_sleep = 1,
	.owner = THIS_MODULE,
};

static int
lq_stp_hw_init(void)
{
	/* the 3 pins used to control the external stp */
	lq_gpio_request(4, 1, 0, 1, "stp-st");
	lq_gpio_request(5, 1, 0, 1, "stp-d");
	lq_gpio_request(6, 1, 0, 1, "stp-sh");

	/* sane defaults */
	lq_stp_w32(0, LQ_STP_AR);
	lq_stp_w32(0, LQ_STP_CPU0);
	lq_stp_w32(0, LQ_STP_CPU1);
	lq_stp_w32(STP_CON0_SWU, LQ_STP_CON0);
	lq_stp_w32(0, LQ_STP_CON1);

	/* rising or falling edge */
	lq_stp_w32_mask(LQ_STP_EDGE_MASK, LQ_STP_FALLING, LQ_STP_CON0);

	/* per default stp 15-0 are set */
	lq_stp_w32_mask(0, LQ_STP_GROUP0, LQ_STP_CON1);

	/* stp are update periodically by the FPID */
	lq_stp_w32_mask(LQ_STP_UPD_MASK, LQ_STP_UPD_SRC_FPI, LQ_STP_CON1);

	/* set stp update speed */
	lq_stp_w32_mask(LQ_STP_MASK, LQ_STP_8HZ, LQ_STP_CON1);

	/* adsl 0 and 1 stp are updated by the arc */
	lq_stp_w32_mask(0, LQ_STP_ADSL_SRC, LQ_STP_CON0);

	lq_pmu_enable(PMU_LED);
	return 0;
}

static int
lq_stp_probe(struct platform_device *pdev)
{
	struct resource *res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	int ret = 0;
	if (!res)
		return -ENOENT;
	res = request_mem_region(res->start, resource_size(res),
		dev_name(&pdev->dev));
	if (!res)
		return -EBUSY;
	virt = ioremap_nocache(res->start, resource_size(res));
	if(!virt)
	{
		ret = -ENOMEM;
		goto err_release_mem_region;
	}
	ret = gpiochip_add(&lq_stp_chip);
	if(!ret)
		return lq_stp_hw_init();

	iounmap(virt);
err_release_mem_region:
	release_mem_region(res->start, resource_size(res));
	return ret;
}

static struct platform_driver lq_stp_driver = {
	.probe = lq_stp_probe,
	.driver = {
		.name = "lq_stp",
		.owner = THIS_MODULE,
	},
};

int __init
init_lq_stp(void)
{
	int ret = platform_driver_register(&lq_stp_driver);
	if (ret)
		printk(KERN_INFO
			"lq_stp: error registering platfom driver");
	return ret;
}

arch_initcall(init_lq_stp);
