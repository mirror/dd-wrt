/*
 *  Atheros AR7XXX/AR9XXX SoC GPIO API support
 *
 *  Copyright (C) 2008-2011 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/spinlock.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/gpio.h>

#include <asm/mach-ar71xx/ar71xx.h>
#include "dev-leds-gpio.h"

static DEFINE_SPINLOCK(ar71xx_gpio_lock);

void set_wl0_gpio(int gpio, int val);
void set_wl1_gpio(int gpio, int val);

unsigned long ar71xx_gpio_count;
EXPORT_SYMBOL(ar71xx_gpio_count);

void __ar71xx_gpio_set_value(unsigned gpio, int value)
{
	void __iomem *base = ar71xx_gpio_base;
	if (gpio >= 48) {
		set_wl1_gpio(gpio - 48, value);
		return;
	}
	if (gpio >= 32) {
		set_wl0_gpio(gpio - 32, value);
		return;
	}
	if (value)
		__raw_writel(1 << gpio, base + AR71XX_GPIO_REG_SET);
	else
		__raw_writel(1 << gpio, base + AR71XX_GPIO_REG_CLEAR);
}

EXPORT_SYMBOL(__ar71xx_gpio_set_value);

int __ar71xx_gpio_get_value(unsigned gpio)
{
	if (gpio >= 32)
		return 0;
	return (__raw_readl(ar71xx_gpio_base + AR71XX_GPIO_REG_IN) >> gpio) & 1;
}

EXPORT_SYMBOL(__ar71xx_gpio_get_value);

static int ar71xx_gpio_get_value(struct gpio_chip *chip, unsigned offset)
{
	return __ar71xx_gpio_get_value(offset);
}

static void ar71xx_gpio_set_value(struct gpio_chip *chip, unsigned offset, int value)
{
	__ar71xx_gpio_set_value(offset, value);
}

static int ar71xx_gpio_direction_input(struct gpio_chip *chip, unsigned offset)
{
	void __iomem *base = ar71xx_gpio_base;
	unsigned long flags;
	if (offset >= 32)
		return 0;

	spin_lock_irqsave(&ar71xx_gpio_lock, flags);

	__raw_writel(__raw_readl(base + AR71XX_GPIO_REG_OE) & ~(1 << offset), base + AR71XX_GPIO_REG_OE);

	spin_unlock_irqrestore(&ar71xx_gpio_lock, flags);

	return 0;
}

static int ar71xx_gpio_direction_output(struct gpio_chip *chip, unsigned offset, int value)
{
	void __iomem *base = ar71xx_gpio_base;
	unsigned long flags;
	if (offset >= 48) {
		set_wl1_gpio(offset - 48, value);
		return 0;
	}
	if (offset >= 32) {
		set_wl0_gpio(offset - 32, value);
		return 0;
	}

	spin_lock_irqsave(&ar71xx_gpio_lock, flags);

	if (value)
		__raw_writel(1 << offset, base + AR71XX_GPIO_REG_SET);
	else
		__raw_writel(1 << offset, base + AR71XX_GPIO_REG_CLEAR);

	__raw_writel(__raw_readl(base + AR71XX_GPIO_REG_OE) | (1 << offset), base + AR71XX_GPIO_REG_OE);

	spin_unlock_irqrestore(&ar71xx_gpio_lock, flags);

	return 0;
}

static struct gpio_chip ar71xx_gpio_chip = {
	.label = "ar71xx",
	.get = ar71xx_gpio_get_value,
	.set = ar71xx_gpio_set_value,
	.direction_input = ar71xx_gpio_direction_input,
	.direction_output = ar71xx_gpio_direction_output,
	.base = 0,
	.ngpio = AR71XX_GPIO_COUNT,
};

void ar71xx_gpio_function_enable(u32 mask)
{
	void __iomem *base = ar71xx_gpio_base;
	unsigned long flags;

	spin_lock_irqsave(&ar71xx_gpio_lock, flags);

	__raw_writel(__raw_readl(base + AR71XX_GPIO_REG_FUNC) | mask, base + AR71XX_GPIO_REG_FUNC);
	/* flush write */
	(void)__raw_readl(base + AR71XX_GPIO_REG_FUNC);

	spin_unlock_irqrestore(&ar71xx_gpio_lock, flags);
}

void ar71xx_gpio_function_disable(u32 mask)
{
	void __iomem *base = ar71xx_gpio_base;
	unsigned long flags;

	spin_lock_irqsave(&ar71xx_gpio_lock, flags);

	__raw_writel(__raw_readl(base + AR71XX_GPIO_REG_FUNC) & ~mask, base + AR71XX_GPIO_REG_FUNC);
	/* flush write */
	(void)__raw_readl(base + AR71XX_GPIO_REG_FUNC);

	spin_unlock_irqrestore(&ar71xx_gpio_lock, flags);
}

void ar71xx_gpio_function_setup(u32 set, u32 clear)
{
	void __iomem *base = ar71xx_gpio_base;
	unsigned long flags;

	spin_lock_irqsave(&ar71xx_gpio_lock, flags);

	__raw_writel((__raw_readl(base + AR71XX_GPIO_REG_FUNC) & ~clear) | set, base + AR71XX_GPIO_REG_FUNC);
	/* flush write */
	(void)__raw_readl(base + AR71XX_GPIO_REG_FUNC);

	spin_unlock_irqrestore(&ar71xx_gpio_lock, flags);
}

EXPORT_SYMBOL(ar71xx_gpio_function_setup);

static struct gpio_led generic_leds_gpio[] __initdata = {
	{
	 .name = "generic_0",
	 .gpio = 0,
	 .active_low = 0,
	 },
	{
	 .name = "generic_1",
	 .gpio = 1,
	 .active_low = 0,

	 },
	{
	 .name = "generic_2",
	 .gpio = 2,
	 .active_low = 0,
	 },
	{
	 .name = "generic_3",
	 .gpio = 3,
	 .active_low = 0,
	 },
	{
	 .name = "generic_4",
	 .gpio = 4,
	 .active_low = 0,
	 },
#if !defined(CONFIG_DIR825) && !defined(CONFIG_WNDR3700)
	{
	 .name = "generic_5",
	 .gpio = 5,
	 .active_low = 0,
	 },
#endif
	{
	 .name = "generic_6",
	 .gpio = 6,
	 .active_low = 0,
	 },
#if !defined(CONFIG_DIR825) && !defined(CONFIG_WNDR3700)
	{
	 .name = "generic_7",
	 .gpio = 7,
	 .active_low = 0,
	 },
#endif
	{
	 .name = "generic_8",
	 .gpio = 8,
	 .active_low = 0,
	 },
	{
	 .name = "generic_9",
	 .gpio = 9,
	 .active_low = 0,
	 },
	{
	 .name = "generic_10",
	 .gpio = 10,
	 .active_low = 0,
	 },
	{
	 .name = "generic_11",
	 .gpio = 11,
	 .active_low = 0,
	 },
	{
	 .name = "generic_12",
	 .gpio = 12,
	 .active_low = 0,
	 },
	{
	 .name = "generic_13",
	 .gpio = 13,
	 .active_low = 0,
	 },
	{
	 .name = "generic_14",
	 .gpio = 14,
	 .active_low = 0,
	 },
	{
	 .name = "generic_15",
	 .gpio = 15,
	 .active_low = 0,
	 },
	{
	 .name = "generic_16",
	 .gpio = 16,
	 .active_low = 0,
	 },
	{
	 .name = "generic_17",
	 .gpio = 17,
	 .active_low = 0,
	 },
#ifndef CONFIG_TPLINK
	{
	 .name = "generic_18",
	 .gpio = 18,
	 .active_low = 0,
	 },
#ifndef CONFIG_BUFFALO
	{
	 .name = "generic_19",
	 .gpio = 19,
	 .active_low = 0,
	 },
#endif
#endif
#ifndef CONFIG_BUFFALO
	{
	 .name = "generic_20",
	 .gpio = 20,
	 .active_low = 0,
	 },
#endif
	{
	 .name = "generic_21",
	 .gpio = 21,
	 .active_low = 0,
	 },
	{
	 .name = "generic_22",
	 .gpio = 22,
	 .active_low = 0,
	 },
	{
	 .name = "generic_23",
	 .gpio = 23,
	 .active_low = 0,
	 },
	{
	 .name = "generic_24",
	 .gpio = 24,
	 .active_low = 0,
	 },
	{
	 .name = "generic_25",
	 .gpio = 25,
	 .active_low = 0,
	 },
	{
	 .name = "generic_26",
	 .gpio = 26,
	 .active_low = 0,
	 },
	{
	 .name = "generic_27",
	 .gpio = 27,
	 .active_low = 0,
	 },
	{
	 .name = "generic_28",
	 .gpio = 28,
	 .active_low = 0,
	 },
	{
	 .name = "generic_29",
	 .gpio = 29,
	 .active_low = 0,
	 },
	{
	 .name = "generic_30",
	 .gpio = 30,
	 .active_low = 0,
	 },
	{
	 .name = "generic_31",
	 .gpio = 31,
	 .active_low = 0,
	 },

//wl gpios
#ifndef CONFIG_AR9100
	{
	 .name = "wireless_generic_0",
	 .gpio = 32,
	 .active_low = 1,
	 },
	{
	 .name = "wireless_generic_1",
	 .gpio = 33,
	 .active_low = 1,
	 },
	{
	 .name = "wireless_generic_2",
	 .gpio = 34,
	 .active_low = 1,
	 },
	{
	 .name = "wireless_generic_3",
	 .gpio = 35,
	 .active_low = 1,
	 },
	{
	 .name = "wireless_generic_4",
	 .gpio = 36,
	 .active_low = 1,
	 },
	{
	 .name = "wireless_generic_5",
	 .gpio = 37,
	 .active_low = 1,
	 },
	{
	 .name = "wireless_generic_6",
	 .gpio = 38,
	 .active_low = 1,
	 },
	{
	 .name = "wireless_generic_7",
	 .gpio = 39,
	 .active_low = 1,
	 },
	{
	 .name = "wireless_generic_8",
	 .gpio = 40,
	 .active_low = 1,
	 },
	{
	 .name = "wireless_generic_9",
	 .gpio = 41,
	 .active_low = 1,
	 },
	{
	 .name = "wireless_generic_10",
	 .gpio = 42,
	 .active_low = 1,
	 },
	{
	 .name = "wireless_generic_11",
	 .gpio = 43,
	 .active_low = 1,
	 },
	{
	 .name = "wireless_generic_12",
	 .gpio = 44,
	 .active_low = 1,
	 },
	{
	 .name = "wireless_generic_13",
	 .gpio = 45,
	 .active_low = 1,
	 },
	{
	 .name = "wireless_generic_14",
	 .gpio = 46,
	 .active_low = 1,
	 },
	{
	 .name = "wireless_generic_15",
	 .gpio = 47,
	 .active_low = 1,
	 },
	{
	 .name = "wireless_generic_16",
	 .gpio = 48,
	 .active_low = 1,
	 },
	{
	 .name = "wireless_generic_17",
	 .gpio = 49,
	 .active_low = 1,
	 },
	{
	 .name = "wireless_generic_18",
	 .gpio = 50,
	 .active_low = 1,
	 },
	{
	 .name = "wireless_generic_19",
	 .gpio = 51,
	 .active_low = 1,
	 },
	{
	 .name = "wireless_generic_20",
	 .gpio = 52,
	 .active_low = 1,
	 },
	{
	 .name = "wireless_generic_21",
	 .gpio = 53,
	 .active_low = 1,
	 },
	{
	 .name = "wireless_generic_22",
	 .gpio = 54,
	 .active_low = 1,
	 },
	{
	 .name = "wireless_generic_23",
	 .gpio = 55,
	 .active_low = 1,
	 },
	{
	 .name = "wireless_generic_24",
	 .gpio = 56,
	 .active_low = 1,
	 },
	{
	 .name = "wireless_generic_25",
	 .gpio = 57,
	 .active_low = 1,
	 },
	{
	 .name = "wireless_generic_26",
	 .gpio = 58,
	 .active_low = 1,
	 },
	{
	 .name = "wireless_generic_27",
	 .gpio = 59,
	 .active_low = 1,
	 },
	{
	 .name = "wireless_generic_28",
	 .gpio = 60,
	 .active_low = 1,
	 },
	{
	 .name = "wireless_generic_29",
	 .gpio = 61,
	 .active_low = 1,
	 },
	{
	 .name = "wireless_generic_30",
	 .gpio = 62,
	 .active_low = 1,
	 },
	{
	 .name = "wireless_generic_31",
	 .gpio = 63,
	 .active_low = 1,
	 },
#endif
};

void __init ar71xx_gpio_init(void)
{
	int err;
	printk(KERN_INFO "Register LED Device\n");
	if (!request_mem_region(AR71XX_GPIO_BASE, AR71XX_GPIO_SIZE, "AR71xx GPIO controller"))
		panic("cannot allocate AR71xx GPIO registers page");

	ar71xx_gpio_chip.ngpio = 64;

	err = gpiochip_add(&ar71xx_gpio_chip);
	if (err)
		panic("cannot add AR71xx GPIO chip, error=%d", err);
	int i;
	for (i = 0; i < sizeof(generic_leds_gpio) / sizeof(struct gpio_led); i++) {
		generic_leds_gpio[i].default_state = LEDS_GPIO_DEFSTATE_KEEP;
	}

	ar71xx_add_device_leds_gpio(-1, sizeof(generic_leds_gpio) / sizeof(struct gpio_led), generic_leds_gpio);
}
