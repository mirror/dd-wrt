/*
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 *  Copyright (C) 2010 John Crispin <blogic@openwrt.org>
 */

#include <linux/slab.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>

#include <lantiq.h>

#define LQ_GPIO0_BASE_ADDR	0x1E100B10
#define LQ_GPIO1_BASE_ADDR	0x1E100B40
#define LQ_GPIO_SIZE		0x30

#define LQ_GPIO_OUT			0x00
#define LQ_GPIO_IN			0x04
#define LQ_GPIO_DIR			0x08
#define LQ_GPIO_ALTSEL0		0x0C
#define LQ_GPIO_ALTSEL1		0x10
#define LQ_GPIO_OD			0x14

#define PINS_PER_PORT		16

#define lq_gpio_getbit(m, r, p)		!!(lq_r32(m + r) & (1 << p))
#define lq_gpio_setbit(m, r, p)		lq_w32_mask(0, (1 << p), m + r)
#define lq_gpio_clearbit(m, r, p)	lq_w32_mask((1 << p), 0, m + r)

struct lq_gpio
{
	void __iomem *membase;
	struct gpio_chip chip;
};

int
gpio_to_irq(unsigned int gpio)
{
	return -EINVAL;
}
EXPORT_SYMBOL(gpio_to_irq);

int
lq_gpio_setconfig(unsigned int pin, unsigned int reg, unsigned int val)
{
	void __iomem *membase = (void*)KSEG1ADDR(LQ_GPIO0_BASE_ADDR);
	if(pin >= (2 * PINS_PER_PORT))
		return -EINVAL;
	if(pin >= PINS_PER_PORT)
	{
		pin -= PINS_PER_PORT;
		membase += LQ_GPIO_SIZE;
	}
	if(val)
		lq_w32_mask(0, (1 << pin), membase + reg);
	else
		lq_w32_mask((1 << pin), 0, membase + reg);
	return 0;
}
EXPORT_SYMBOL(lq_gpio_setconfig);

int
lq_gpio_request(unsigned int pin, unsigned int alt0,
	unsigned int alt1, unsigned int dir, const char *name)
{
	void __iomem *membase = (void*)KSEG1ADDR(LQ_GPIO0_BASE_ADDR);
	if(pin >= (2 * PINS_PER_PORT))
		return -EINVAL;
	if(gpio_request(pin, name))
	{
		printk("failed to register %s gpio\n", name);
		return -EBUSY;
	}
	if(dir)
		gpio_direction_output(pin, 1);
	else
		gpio_direction_input(pin);
	if(pin >= PINS_PER_PORT)
	{
		pin -= PINS_PER_PORT;
		membase += LQ_GPIO_SIZE;
	}
	if(alt0)
		lq_gpio_setbit(membase, LQ_GPIO_ALTSEL0, pin);
	else
		lq_gpio_clearbit(membase, LQ_GPIO_ALTSEL0, pin);
	if(alt1)
		lq_gpio_setbit(membase, LQ_GPIO_ALTSEL1, pin);
	else
		lq_gpio_clearbit(membase, LQ_GPIO_ALTSEL1, pin);
	return 0;
}
EXPORT_SYMBOL(lq_gpio_request);

static void
lq_gpio_set(struct gpio_chip *chip, unsigned int offset, int value)
{
	struct lq_gpio *lq_gpio = container_of(chip, struct lq_gpio, chip);
	if(value)
		lq_gpio_setbit(lq_gpio->membase, LQ_GPIO_OUT, offset);
	else
		lq_gpio_clearbit(lq_gpio->membase, LQ_GPIO_OUT, offset);
}

static int
lq_gpio_get(struct gpio_chip *chip, unsigned int offset)
{
	struct lq_gpio *lq_gpio = container_of(chip, struct lq_gpio, chip);
	return lq_gpio_getbit(lq_gpio->membase, LQ_GPIO_IN, offset);
}

static int
lq_gpio_direction_input(struct gpio_chip *chip, unsigned int offset)
{
	struct lq_gpio *lq_gpio = container_of(chip, struct lq_gpio, chip);
	lq_gpio_clearbit(lq_gpio->membase, LQ_GPIO_OD, offset);
	lq_gpio_clearbit(lq_gpio->membase, LQ_GPIO_DIR, offset);
	return 0;
}

static int
lq_gpio_direction_output(struct gpio_chip *chip, unsigned int offset, int value)
{
	struct lq_gpio *lq_gpio = container_of(chip, struct lq_gpio, chip);
	lq_gpio_setbit(lq_gpio->membase, LQ_GPIO_OD, offset);
	lq_gpio_setbit(lq_gpio->membase, LQ_GPIO_DIR, offset);
	lq_gpio_set(chip, offset, value);
	return 0;
}

static int
lq_gpio_req(struct gpio_chip *chip, unsigned offset)
{
	struct lq_gpio *lq_gpio = container_of(chip, struct lq_gpio, chip);
	lq_gpio_clearbit(lq_gpio->membase, LQ_GPIO_ALTSEL0, offset);
	lq_gpio_clearbit(lq_gpio->membase, LQ_GPIO_ALTSEL1, offset);
	return 0;
}

static int
lq_gpio_probe(struct platform_device *pdev)
{
	struct lq_gpio *lq_gpio = kzalloc(sizeof(struct lq_gpio), GFP_KERNEL);
	struct resource *res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	int ret = 0;
	if(!res)
	{
		ret = -ENOENT;
		goto err_free;
	}
	res = request_mem_region(res->start, resource_size(res),
		dev_name(&pdev->dev));
	if(!res)
	{
		ret = -EBUSY;
		goto err_free;
	}
	lq_gpio->membase = ioremap_nocache(res->start, resource_size(res));
	if(!lq_gpio->membase)
	{
		ret = -ENOMEM;
		goto err_release_mem_region;
	}
	lq_gpio->chip.label = "lq_gpio";
	lq_gpio->chip.direction_input = lq_gpio_direction_input;
	lq_gpio->chip.direction_output = lq_gpio_direction_output;
	lq_gpio->chip.get = lq_gpio_get;
	lq_gpio->chip.set = lq_gpio_set;
	lq_gpio->chip.request = lq_gpio_req;
	lq_gpio->chip.base = PINS_PER_PORT * pdev->id;
	lq_gpio->chip.ngpio = PINS_PER_PORT;
	platform_set_drvdata(pdev, lq_gpio);
	ret = gpiochip_add(&lq_gpio->chip);
	if(!ret)
		return 0;

	iounmap(lq_gpio->membase);
err_release_mem_region:
	release_mem_region(res->start, resource_size(res));
err_free:
    kfree(lq_gpio);
	return ret;
}

static struct platform_driver
lq_gpio_driver = {
	.probe = lq_gpio_probe,
	.driver = {
		.name = "lq_gpio",
		.owner = THIS_MODULE,
	},
};

int __init
lq_gpio_init(void)
{
	int ret = platform_driver_register(&lq_gpio_driver);
	if(ret)
		printk(KERN_INFO "lq_gpio : Error registering platfom driver!");
	return ret;
}

postcore_initcall(lq_gpio_init);
