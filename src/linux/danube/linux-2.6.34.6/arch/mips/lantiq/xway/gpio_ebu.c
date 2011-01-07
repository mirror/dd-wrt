/*
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 *  Copyright (C) 2010 John Crispin <blogic@openwrt.org>
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/gpio.h>

#include <xway.h>

#define LQ_EBU_BUSCON	0x1e7ff
#define LQ_EBU_WP		0x80000000

static int shadow = 0x0000;
static void __iomem *virt;

static int
lq_ebu_direction_output(struct gpio_chip *chip, unsigned offset, int value)
{
	return 0;
}

static void
lq_ebu_set(struct gpio_chip *chip, unsigned offset, int value)
{
	unsigned long flags;
	if(value)
		shadow |= (1 << offset);
	else
		shadow &= ~(1 << offset);
	spin_lock_irqsave(&ebu_lock, flags);
	lq_w32(LQ_EBU_BUSCON, LQ_EBU_BUSCON1);
	*((__u16*)virt) = shadow;
	lq_w32(LQ_EBU_BUSCON | LQ_EBU_WP, LQ_EBU_BUSCON1);
	spin_unlock_irqrestore(&ebu_lock, flags);
}

static struct gpio_chip
lq_ebu_chip =
{
	.label = "lq_ebu",
	.direction_output = lq_ebu_direction_output,
	.set = lq_ebu_set,
	.base = 32,
	.ngpio = 16,
	.can_sleep = 1,
	.owner = THIS_MODULE,
};

static int __devinit
lq_ebu_probe(struct platform_device *pdev)
{
	struct resource *res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	int ret = 0;
	if (!res)
		return -ENOENT;
	res = request_mem_region(res->start, resource_size(res),
		dev_name(&pdev->dev));
	if (!res)
		return -EBUSY;

	/* tell the ebu controller which mem addr we will be using */
	lq_w32(pdev->resource->start | 0x1, LQ_EBU_ADDRSEL1);
	lq_w32(LQ_EBU_BUSCON | LQ_EBU_WP, LQ_EBU_BUSCON1);

	virt = ioremap_nocache(res->start, resource_size(res));
	if (!virt)
	{
		dev_err(&pdev->dev, "Failed to ioremap mem region\n");
		ret = -ENOMEM;
		goto err_release_mem_region;
	}
	/* grab the default settings passed form the platform code */
	shadow = (unsigned int) pdev->dev.platform_data;

	ret = gpiochip_add(&lq_ebu_chip);
	if (!ret)
		return 0;

err_release_mem_region:
	release_mem_region(res->start, resource_size(res));
	return ret;
}

static struct platform_driver
lq_ebu_driver = {
	.probe = lq_ebu_probe,
	.driver = {
		.name = "lq_ebu",
		.owner = THIS_MODULE,
	},
};

static int __init
init_lq_ebu(void)
{
	return platform_driver_register(&lq_ebu_driver);
}

arch_initcall(init_lq_ebu);
