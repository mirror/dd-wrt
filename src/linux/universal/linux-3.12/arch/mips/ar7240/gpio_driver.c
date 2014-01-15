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
#include "ar7240.h"

static DEFINE_SPINLOCK(ar71xx_gpio_lock);

void set_wl0_gpio(int gpio, int val);
void set_wmac_gpio(int gpio, int val);
void set_wl1_gpio(int gpio, int val);

unsigned long ar71xx_gpio_count;
EXPORT_SYMBOL(ar71xx_gpio_count);

void __ar71xx_gpio_set_value(unsigned gpio, int value)
{
	void __iomem *base = ar71xx_gpio_base;
#ifdef CONFIG_WASP_SUPPORT
	if (gpio >= 48) {
		set_wmac_gpio(gpio - 48, value);
		return;
	}
#endif
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

#ifdef CONFIG_WASP_SUPPORT
	__raw_writel(__raw_readl(base + AR71XX_GPIO_REG_OE) | (1 << offset), base + AR71XX_GPIO_REG_OE);

#else
	__raw_writel(__raw_readl(base + AR71XX_GPIO_REG_OE) & ~(1 << offset), base + AR71XX_GPIO_REG_OE);
#endif
	spin_unlock_irqrestore(&ar71xx_gpio_lock, flags);

	return 0;
}

static int ar71xx_gpio_direction_output(struct gpio_chip *chip, unsigned offset, int value)
{
	void __iomem *base = ar71xx_gpio_base;
	unsigned long flags;
	if (offset >= 32) {
		set_wl0_gpio(offset - 32, value);
		return 0;
	}

	spin_lock_irqsave(&ar71xx_gpio_lock, flags);

	if (value)
		__raw_writel(1 << offset, base + AR71XX_GPIO_REG_SET);
	else
		__raw_writel(1 << offset, base + AR71XX_GPIO_REG_CLEAR);
#ifdef CONFIG_WASP_SUPPORT
	__raw_writel(__raw_readl(base + AR71XX_GPIO_REG_OE) & ~(1 << offset), base + AR71XX_GPIO_REG_OE);

#else
	__raw_writel(__raw_readl(base + AR71XX_GPIO_REG_OE) | (1 << offset), base + AR71XX_GPIO_REG_OE);

#endif
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
	.ngpio = 48,
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
#ifdef CONFIG_WPE72
	 .active_low = 1,
#else
	 .active_low = 0,
#endif
	 },
#ifndef CONFIG_WDR2543
	{
	 .name = "generic_1",
	 .gpio = 1,
#ifdef CONFIG_WPE72
	 .active_low = 1,
#else
	 .active_low = 0,
#endif
	 },
#endif
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
	{
	 .name = "generic_5",
	 .gpio = 5,
	 .active_low = 0,
	 },
#ifndef CONFIG_WDR2543
	{
	 .name = "generic_6",
	 .gpio = 6,
	 .active_low = 0,
	 },
#endif
	{
	 .name = "generic_7",
	 .gpio = 7,
	 .active_low = 0,
	 },
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
#if defined(CONFIG_WR841V8)
	 .active_low = 1,
#else
	 .active_low = 0,
#endif
	 },
#if defined(CONFIG_DIR825C1) || defined(CONFIG_DIR615E)
	{
	 .name = "generic_13",
	 .gpio = 13,
	 .active_low = 1,
	 },
#else
	{
	 .name = "generic_13",
	 .gpio = 13,
	 .active_low = 0,
	 },
#endif
	{
	 .name = "generic_14",
	 .gpio = 14,
#if defined(CONFIG_DIR615E)
	 .active_low = 1,
#else
	 .active_low = 0,
#endif
	 },
	{
	 .name = "generic_15",
	 .gpio = 15,
#if defined(CONFIG_DIR615E)
	 .active_low = 1,
#else
	 .active_low = 0,
#endif
	 },
	{
	 .name = "generic_16",
	 .gpio = 16,
#if defined(CONFIG_DIR615E)
	 .active_low = 1,
#else
	 .active_low = 0,
#endif
	 },
#if defined(CONFIG_MACH_HORNET)
	{
	 .name = "generic_17",
	 .gpio = 17,
	 .active_low = 1,
	 },
#else
	{
	 .name = "generic_17",
	 .gpio = 17,
#if defined(CONFIG_DIR615E)
	 .active_low = 1,
#else
	 .active_low = 0,
#endif
	 },
#endif
	{
	 .name = "generic_18",
	 .gpio = 18,
#if defined(CONFIG_WR841V8)
	 .active_low = 1,
#else
	 .active_low = 0,
#endif
	 },
	{
	 .name = "generic_19",
	 .gpio = 19,
#if defined(CONFIG_WR841V8)
	 .active_low = 1,
#else
	 .active_low = 0,
#endif
	 },
	{
	 .name = "generic_20",
	 .gpio = 20,
#if defined(CONFIG_WR841V8)
	 .active_low = 1,
#else
	 .active_low = 0,
#endif
	 },
	{
	 .name = "generic_21",
	 .gpio = 21,
#if defined(CONFIG_WR841V8)
	 .active_low = 1,
#else
	 .active_low = 0,
#endif
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
#ifndef CONFIG_MACH_HORNET

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
	 }
	,
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
#ifdef CONFIG_WASP_SUPPORT
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
	 }
	,
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
#endif
};

void serial_print(char *fmt, ...);

void __init ar71xx_gpio_init(void)
{
	int err;
	u32 t, rddata;

	if (!request_mem_region(AR71XX_GPIO_BASE, AR71XX_GPIO_SIZE, "AR71xx GPIO controller"))
		panic("cannot allocate AR71xx GPIO registers page");

#ifdef CONFIG_MACH_HORNET
	ar71xx_gpio_chip.ngpio = 32;
#else
#ifdef CONFIG_WASP_SUPPORT
	ar71xx_gpio_chip.ngpio = 64;
#else
	ar71xx_gpio_chip.ngpio = 48;
#endif
#endif

	err = gpiochip_add(&ar71xx_gpio_chip);
	if (err)
		panic("cannot add AR71xx GPIO chip, error=%d", err);
	int i;
	for (i = 0; i < sizeof(generic_leds_gpio) / sizeof(struct gpio_led); i++) {
		generic_leds_gpio[i].default_state = LEDS_GPIO_DEFSTATE_KEEP;
	}
	ar71xx_add_device_leds_gpio(-1, sizeof(generic_leds_gpio) / sizeof(struct gpio_led), generic_leds_gpio);

#ifdef CONFIG_MACH_HORNET
	printk(KERN_INFO "disable Hornet LED\n");
	ar71xx_gpio_function_disable(AR933X_GPIO_FUNC_ETH_SWITCH_LED0_EN |
				     AR933X_GPIO_FUNC_ETH_SWITCH_LED1_EN | AR933X_GPIO_FUNC_ETH_SWITCH_LED2_EN | AR933X_GPIO_FUNC_ETH_SWITCH_LED3_EN | AR933X_GPIO_FUNC_ETH_SWITCH_LED4_EN);
#else
#ifdef CONFIG_WR741
	printk(KERN_INFO "fixup WR741 Switch LED's\n");
	ar71xx_gpio_function_disable(AR724X_GPIO_FUNC_ETH_SWITCH_LED0_EN |
				     AR724X_GPIO_FUNC_ETH_SWITCH_LED1_EN | AR724X_GPIO_FUNC_ETH_SWITCH_LED2_EN | AR724X_GPIO_FUNC_ETH_SWITCH_LED3_EN | AR724X_GPIO_FUNC_ETH_SWITCH_LED4_EN);

#endif
#endif


#ifdef CONFIG_MACH_HORNET
	t = ar71xx_reset_rr(AR933X_RESET_REG_BOOTSTRAP);
	t |= AR933X_BOOTSTRAP_MDIO_GPIO_EN;
	ar71xx_reset_wr(AR933X_RESET_REG_BOOTSTRAP, t);
//      gpio_request(26, "USB power");
//      gpio_direction_output(26, 1);
#endif
#ifdef CONFIG_MTD_NAND_ATH
#define ATH_APB_BASE			0x18000000	/* 384M */
#define ATH_GPIO_BASE			ATH_APB_BASE+0x00040000
#define ATH_GPIO_OUT_FUNCTION2		ATH_GPIO_BASE+0x34
#define ATH_GPIO_OUT_FUNCTION2_ENABLE_GPIO_11(x)	((0xff&x)<<24)
#define ATH_GPIO_FUNCTIONS		ATH_GPIO_BASE+0x6c

	printk(KERN_INFO "fixup WNDR3700v4 wifi led\n");
	// enable gpio 11
	rddata = ar7240_reg_rd(ATH_GPIO_OUT_FUNCTION2);
	rddata = rddata & 0x00ffffff;
	rddata = rddata | ATH_GPIO_OUT_FUNCTION2_ENABLE_GPIO_11(0x0);
	ar7240_reg_wr(ATH_GPIO_OUT_FUNCTION2, rddata);
	//disable jtag
	ar7240_reg_rmw_set(ATH_GPIO_FUNCTIONS, (1 << 1));

	// finally disable 2.4 ghz led and let the driver handle it
	__ar71xx_gpio_set_value(11, 1);
#endif
}
