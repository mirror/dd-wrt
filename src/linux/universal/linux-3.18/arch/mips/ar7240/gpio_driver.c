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
#include <linux/delay.h>

#include <asm/mach-ar71xx/ar71xx.h>
#include "dev-leds-gpio.h"
#include "ar7240.h"

static DEFINE_SPINLOCK(ar71xx_gpio_lock);

void set_wl0_gpio(int gpio, int val);
void set_wmac_gpio(int gpio, int val);
void set_wl1_gpio(int gpio, int val);

unsigned long ar71xx_gpio_count;
EXPORT_SYMBOL(ar71xx_gpio_count);

void shift_register_set(u_int32_t index, u_int32_t val);


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
#ifdef CONFIG_ARCHERC7V4
	if (gpio >= 24) {
		shift_register_set(gpio-24, value);
		return;
	}
#endif
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

#ifdef CONFIG_ARCHERC7V4
	if (offset >= 24) {
		shift_register_set(offset-24, value);
		return;
	}
#endif

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

#ifdef CONFIG_ARCHERC7V4

enum SHIFT_REGISTER_OUTPUT {
    Q7_2G_WIFI_LED = 0,
    Q6_WAN_OLED,
    Q5_WAN_GLED,
    Q4_LAN1_LED,
    Q3_LAN2_LED,
    Q2_LAN3_LED,
    Q1_LAN4_LED,
    Q0_WPS_LED,
    SHIFT_REGISTER_OUTPUT_NUM
};


#define AP152_GPIO_SHIFT_OE     1
#define AP152_GPIO_SHIFT_SER    14
#define AP152_GPIO_SHIFT_SRCLK  15
#define AP152_GPIO_SHIFT_RCLK   16
#define AP152_GPIO_SHIFT_SRCLR  21

#define SHIFT_REG_TW_FACTOR	                120
#define SHIFT_REG_SER_SRCLK_DELAY_FACTOR	150
#define SHIFT_REG_SRCLR_RCLK_DELAY_FACTOR	80



void shift_register_set(u_int32_t index, u_int32_t val2) 
{
	static u_int32_t val = 255;
	u_int32_t value;

	__ar71xx_gpio_set_value(AP152_GPIO_SHIFT_RCLK, 0);

	if (val2)
	    val|=(1<<index);
	else
	    val&=~(1<<index);
//	printk(KERN_NOTICE "set %d=%d == %X\n",index,val2,val);
	for(index = 0; index < SHIFT_REGISTER_OUTPUT_NUM; index++)
	{
		value = (val >> index) & 0x1;
        __ar71xx_gpio_set_value(AP152_GPIO_SHIFT_SER, value);
        ndelay(SHIFT_REG_SER_SRCLK_DELAY_FACTOR);
		__ar71xx_gpio_set_value(AP152_GPIO_SHIFT_SRCLK, 1);
		ndelay(SHIFT_REG_TW_FACTOR);
		__ar71xx_gpio_set_value(AP152_GPIO_SHIFT_SRCLK, 0);
	}
	__ar71xx_gpio_set_value(AP152_GPIO_SHIFT_RCLK, 1);
    ndelay(SHIFT_REG_TW_FACTOR);
	__ar71xx_gpio_set_value(AP152_GPIO_SHIFT_RCLK, 0);
	ndelay(SHIFT_REG_TW_FACTOR);
}
#endif
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
	if (soc_is_ar934x() ||
		 soc_is_qca953x() ||
		 soc_is_qca955x() ||
		 soc_is_qca956x() ||
		 soc_is_qcn550x() ||
		 soc_is_tp9343()) {

	__raw_writel(__raw_readl(base + AR934X_GPIO_REG_FUNC) | mask, base + AR934X_GPIO_REG_FUNC);
	/* flush write */
	(void)__raw_readl(base + AR934X_GPIO_REG_FUNC);
	}else{
	__raw_writel(__raw_readl(base + AR71XX_GPIO_REG_FUNC) | mask, base + AR71XX_GPIO_REG_FUNC);	
	/* flush write */
	(void)__raw_readl(base + AR71XX_GPIO_REG_FUNC);
	}
	/* flush write */
	(void)__raw_readl(base + AR71XX_GPIO_REG_FUNC);

	spin_unlock_irqrestore(&ar71xx_gpio_lock, flags);
}

void ar71xx_gpio_function_disable(u32 mask)
{
	void __iomem *base = ar71xx_gpio_base;
	unsigned long flags;

	spin_lock_irqsave(&ar71xx_gpio_lock, flags);

	if (soc_is_ar934x() ||
		 soc_is_qca953x() ||
		 soc_is_qca955x() ||
		 soc_is_qca956x() ||
		 soc_is_qcn550x() ||
		 soc_is_tp9343()) {

	__raw_writel(__raw_readl(base + AR934X_GPIO_REG_FUNC) & ~mask, base + AR934X_GPIO_REG_FUNC);
	/* flush write */
	(void)__raw_readl(base + AR934X_GPIO_REG_FUNC);
	}else{
	__raw_writel(__raw_readl(base + AR71XX_GPIO_REG_FUNC) & ~mask, base + AR71XX_GPIO_REG_FUNC);	
	/* flush write */
	(void)__raw_readl(base + AR71XX_GPIO_REG_FUNC);
	}

	spin_unlock_irqrestore(&ar71xx_gpio_lock, flags);
}

void ar71xx_gpio_function_setup(u32 set, u32 clear)
{
	void __iomem *base = ar71xx_gpio_base;
	unsigned long flags;

	spin_lock_irqsave(&ar71xx_gpio_lock, flags);

	if (soc_is_ar934x() ||
		 soc_is_qca953x() ||
		 soc_is_qca955x() ||
		 soc_is_qca956x() ||
		 soc_is_qcn550x() ||
		 soc_is_tp9343()) {
	__raw_writel((__raw_readl(base + AR934X_GPIO_REG_FUNC) & ~clear) | set, base + AR934X_GPIO_REG_FUNC);
	/* flush write */
	(void)__raw_readl(base + AR934X_GPIO_REG_FUNC);
	}else{
	__raw_writel((__raw_readl(base + AR71XX_GPIO_REG_FUNC) & ~clear) | set, base + AR71XX_GPIO_REG_FUNC);
	
	/* flush write */
	(void)__raw_readl(base + AR71XX_GPIO_REG_FUNC);
	}

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
#elif CONFIG_XD3200
	 .active_low = 1,
#elif CONFIG_ERC
	 .active_low = 1,
#else
	 .active_low = 0,
#endif
	 },
#endif
#ifndef CONFIG_PERU
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
#endif
	{
	 .name = "generic_4",
	 .gpio = 4,
#if defined(CONFIG_WR841V9)
	 .active_low = 1,
#else
	 .active_low = 0,
#endif
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
#ifdef CONFIG_WR941V6
	 .active_low = 1,
#else
	 .active_low = 0,
#endif
	 },
#endif
	{
	 .name = "generic_7",
	 .gpio = 7,
#if defined(CONFIG_UAPAC) || defined(CONFIG_WR941V6)
	 .active_low = 1,
#else
	 .active_low = 0,
#endif
	 },
	{
	 .name = "generic_8",
	 .gpio = 8,
#if defined(CONFIG_UAPAC) || defined(CONFIG_WR941V6) || defined(CONFIG_ARCHERC7V5)
	 .active_low = 1,
#else
	 .active_low = 0,
#endif
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
#if defined(CONFIG_WR841V9)
	 .active_low = 1,
#else
	 .active_low = 0,
#endif
	 },
#if !defined(CONFIG_UAPAC) && !defined(CONFIG_UAPACPRO) && !defined(CONFIG_FMS2111)
	{
	 .name = "generic_12",
	 .gpio = 12,
#if defined(CONFIG_PERU)
	 .active_low = 0,
#elif defined(CONFIG_WR841V8)
	 .active_low = 1,
#elif defined(CONFIG_CPE880) || defined(CONFIG_XD9531)
	 .active_low = 1,
#else
	 .active_low = 0,
#endif
	 },
#endif
#if defined(CONFIG_DIR825C1) || defined(CONFIG_DIR615E) || defined(CONFIG_ERC)
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
#ifndef CONFIG_ARCHERC25
	{
	 .name = "generic_14",
	 .gpio = 14,
#if defined(CONFIG_DIR615E) || defined(CONFIG_WR841V9) || defined(CONFIG_ERC) || defined(CONFIG_WR1043V4)
	 .active_low = 1,
#else
	 .active_low = 0,
#endif
	 },
	{
	 .name = "generic_15",
	 .gpio = 15,
#if defined(CONFIG_DIR615E) || defined(CONFIG_WR841V9)  || defined(CONFIG_ERC)  || defined(CONFIG_ARCHERC7V5)
	 .active_low = 1,
#else
	 .active_low = 0,
#endif
	 },
	{
	 .name = "generic_16",
	 .gpio = 16,
#if defined(CONFIG_DIR615E) || defined(CONFIG_WR841V9)  || defined(CONFIG_ERC) || defined(CONFIG_ARCHERC7V5) || defined(CONFIG_XD9531)
	 .active_low = 1,
#else
	 .active_low = 0,
#endif
	 },
#endif
#if !defined(CONFIG_RAMBUTAN) || defined(CONFIG_DW02_412H)
#if defined(CONFIG_MACH_HORNET) && !defined(CONFIG_ERC)
	{
	 .name = "generic_17",
	 .gpio = 17,
	 .active_low = 1,
	 },
#else
	{
	 .name = "generic_17",
	 .gpio = 17,
#if defined(CONFIG_DIR615E) || defined(CONFIG_ERC) || defined(CONFIG_ARCHERC7V5) || defined(CONFIG_XD9531) || defined(CONFIG_DW02_412H)
	 .active_low = 1,
#elif defined(CONFIG_CPE880)
	 .active_low = 1,
#else
	 .active_low = 0,
#endif
	 },
#endif
#endif
	{
	 .name = "generic_18",
	 .gpio = 18,
#if defined(CONFIG_WR841V8) || defined(CONFIG_DAP3310)
	 .active_low = 1,
#elif defined(CONFIG_CPE880)
	 .active_low = 1,
#else
	 .active_low = 0,
#endif
	 },
#ifndef CONFIG_ARCHERC25
	{
	 .name = "generic_19",
	 .gpio = 19,
#if defined(CONFIG_WR841V8) || defined(CONFIG_DAP3310)
	 .active_low = 1,
#elif defined(CONFIG_XD3200)
	 .active_low = 1,
#elif defined(CONFIG_CPE880)
	 .active_low = 1,
#else
	 .active_low = 0,
#endif
	 },
#endif
	{
	 .name = "generic_20",
	 .gpio = 20,
#if defined(CONFIG_WR841V8) || defined(CONFIG_WR1043V4)
	 .active_low = 1,
#elif defined(CONFIG_CPE880)
	 .active_low = 1,
#else
	 .active_low = 0,
#endif
	 },
#ifndef CONFIG_ARCHERC25
	{
	 .name = "generic_21",
	 .gpio = 21,
#if defined(CONFIG_WR841V8) || defined(CONFIG_WR1043V4) || defined(CONFIG_ARCHERC7V5)
	 .active_low = 1,
#elif defined(CONFIG_CPE880)
	 .active_low = 1,
#else
	 .active_low = 0,
#endif
	 },
#endif
#ifndef CONFIG_FMS2111
	{
	 .name = "generic_22",
	 .gpio = 22,
#if defined(CONFIG_CPE880)
	 .active_low = 1,
#else
	 .active_low = 0,
#endif
 	 },
#ifndef CONFIG_RAMBUTAN
	{
	 .name = "generic_23",
	 .gpio = 23,
	 .active_low = 0,
	 },
#endif
#endif //CONFIG_FMS2111
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

#define AR934X_GPIO_REG_OUT_FUNC0	0x2c
#define AR934X_GPIO_OUT_GPIO		0
#define AR934X_GPIO_OUT_LED_LINK0	41
#define AR934X_GPIO_OUT_LED_LINK1	42
#define AR934X_GPIO_OUT_LED_LINK2	43
#define AR934X_GPIO_OUT_LED_LINK3	44
#define AR934X_GPIO_OUT_LED_LINK4	45
#define AR934X_GPIO_OUT_EXT_LNA0	46
#define AR934X_GPIO_OUT_EXT_LNA1	47

static void __init ath79_gpio_output_select(unsigned gpio, u8 val)
{
	void __iomem *base = ar71xx_gpio_base;
	unsigned long flags;
	unsigned int reg;
	u32 t, s;


	if (gpio >= AR934X_GPIO_COUNT)
		return;

	reg = AR934X_GPIO_REG_OUT_FUNC0 + 4 * (gpio / 4);
	s = 8 * (gpio % 4);

	spin_lock_irqsave(&ar71xx_gpio_lock, flags);

	t = __raw_readl(base + reg);
	t &= ~(0xff << s);
	t |= val << s;
	__raw_writel(t, base + reg);

	/* flush write */
	(void) __raw_readl(base + reg);

	spin_unlock_irqrestore(&ar71xx_gpio_lock, flags);
}


static void ar934x_set_ext_lna_gpio(unsigned chain, int gpio)
{
	unsigned int sel;

	if (WARN_ON(chain > 1))
		return;

	if (chain == 0)
		sel = AR934X_GPIO_OUT_EXT_LNA0;
	else
		sel = AR934X_GPIO_OUT_EXT_LNA1;

	ath79_gpio_output_select(gpio, sel);
}


#ifdef CONFIG_COMFAST_WATCHDOG
#define	XWDT_AUTOFEED_DURATION		(HZ / 3)
static int gpio_external_wdt = -1;
static int wdt_timeout = -1, wdt_autofeed_count = 0;

static void watchdog_fire(unsigned long);
static struct timer_list watchdog_ticktock = TIMER_INITIALIZER(watchdog_fire, 0, 0);
static void external_wdt_toggle(void);

static void enable_external_wdt(int gpio)
{
	gpio_external_wdt = gpio;
	
	external_wdt_toggle();
	
	wdt_timeout = -1;
	mod_timer(&watchdog_ticktock, jiffies + XWDT_AUTOFEED_DURATION);
}

static void external_wdt_toggle(void)
{
	static u32 data;
	data ++;
	gpio_set_value(gpio_external_wdt, data & 0x01);
}

void ath79_external_wdt_disable(void)
{
	if(gpio_external_wdt >= 0) {
		wdt_timeout = -1;
		mod_timer(&watchdog_ticktock, jiffies + XWDT_AUTOFEED_DURATION);
	}
}
EXPORT_SYMBOL(ath79_external_wdt_disable);

void ath79_external_wdt_trigger(void)
{
	if(gpio_external_wdt >= 0) {
		//printk(KERN_ERR "XWDT TRIGGER\n");
		wdt_autofeed_count = 0;
		mod_timer(&watchdog_ticktock, jiffies + XWDT_AUTOFEED_DURATION);
	}
}
EXPORT_SYMBOL(ath79_external_wdt_trigger);

void ath79_external_wdt_set_timeout(int timeout)
{
	if(gpio_external_wdt >= 0) {
		wdt_timeout = timeout;
		external_wdt_toggle();
		//printk(KERN_ERR "XWDT SET TIMEOUT: %d\n", timeout);
	}
}
EXPORT_SYMBOL(ath79_external_wdt_set_timeout);

static void watchdog_fire(unsigned long data)
{
	if(wdt_timeout > 0) 
		wdt_autofeed_count++;
	
	if((wdt_timeout < 0) || (wdt_autofeed_count < wdt_timeout)) {
		//printk(KERN_ERR "XWDT AUTOFEED: %d\n", wdt_autofeed_count);
		external_wdt_toggle();
		mod_timer(&watchdog_ticktock, jiffies + XWDT_AUTOFEED_DURATION);
	}
}

static void ext_lna_control_gpio_setup(int gpio_rx0, int gpio_rx1)
{
	ath79_gpio_output_select(gpio_rx0, AR934X_GPIO_OUT_EXT_LNA0);
	ath79_gpio_output_select(gpio_rx1, AR934X_GPIO_OUT_EXT_LNA1);
}

#endif
void serial_print(char *fmt, ...);

void __init ar71xx_gpio_init(void)
{
	int err;
	u32 t, rddata;
	if (!request_mem_region(AR71XX_GPIO_BASE, AR71XX_GPIO_SIZE, "AR71xx GPIO controller"))
		panic("cannot allocate AR71xx GPIO registers page");

#ifdef CONFIG_ARCHERC7V4
	__ar71xx_gpio_set_value(AP152_GPIO_SHIFT_OE, 0);
#endif
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
#ifndef CONFIG_ARCHERC25
	ar71xx_add_device_leds_gpio(-1, sizeof(generic_leds_gpio) / sizeof(struct gpio_led), generic_leds_gpio);
#endif
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
// test
//	ar71xx_gpio_function_disable(AR724X_GPIO_FUNC_ETH_SWITCH_LED0_EN |
//				     AR724X_GPIO_FUNC_ETH_SWITCH_LED1_EN | AR724X_GPIO_FUNC_ETH_SWITCH_LED2_EN | AR724X_GPIO_FUNC_ETH_SWITCH_LED3_EN | AR724X_GPIO_FUNC_ETH_SWITCH_LED4_EN);

#ifdef CONFIG_ALFANX
	ar71xx_gpio_function_setup(AR724X_GPIO_FUNC_JTAG_DISABLE,
				  AR724X_GPIO_FUNC_ETH_SWITCH_LED0_EN |
				  AR724X_GPIO_FUNC_ETH_SWITCH_LED1_EN |
				  AR724X_GPIO_FUNC_ETH_SWITCH_LED2_EN |
				  AR724X_GPIO_FUNC_ETH_SWITCH_LED3_EN |
				  AR724X_GPIO_FUNC_ETH_SWITCH_LED4_EN);
#endif
#ifdef CONFIG_XD3200
	printk(KERN_INFO "Yuncore Fix\n");
	ar71xx_gpio_function_enable(AR934X_GPIO_FUNC_JTAG_DISABLE);

	ar71xx_gpio_direction_output(NULL, 14, 1);
	ar71xx_gpio_direction_output(NULL, 15, 1);
	ar71xx_gpio_direction_output(NULL, 16, 1);
	ar71xx_gpio_direction_output(NULL, 17, 1);

#endif
#ifdef CONFIG_PERU
	ar71xx_gpio_function_enable(AR934X_GPIO_FUNC_JTAG_DISABLE);
#endif
#ifdef CONFIG_MACH_HORNET
	t = ar71xx_reset_rr(AR933X_RESET_REG_BOOTSTRAP);
	t |= AR933X_BOOTSTRAP_MDIO_GPIO_EN;
	ar71xx_reset_wr(AR933X_RESET_REG_BOOTSTRAP, t);
//      gpio_request(26, "USB power");
//      gpio_direction_output(26, 1);
#endif
#if !defined(CONFIG_RAMBUTAN) && defined(CONFIG_MTD_NAND_ATH)
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
#ifdef CONFIG_WDR3500

#define WDR3500_GPIO_LED_WAN		18
#define WDR3500_GPIO_LED_LAN1		19
#define WDR3500_GPIO_LED_LAN2		20
#define WDR3500_GPIO_LED_LAN3		21
#define WDR3500_GPIO_LED_LAN4		22

	ath79_gpio_output_select(WDR3500_GPIO_LED_LAN1,
				 AR934X_GPIO_OUT_LED_LINK3);
	ath79_gpio_output_select(WDR3500_GPIO_LED_LAN2,
				 AR934X_GPIO_OUT_LED_LINK2);
	ath79_gpio_output_select(WDR3500_GPIO_LED_LAN3,
				 AR934X_GPIO_OUT_LED_LINK1);
	ath79_gpio_output_select(WDR3500_GPIO_LED_LAN4,
				 AR934X_GPIO_OUT_LED_LINK0);
	ath79_gpio_output_select(WDR3500_GPIO_LED_WAN,
				 AR934X_GPIO_OUT_LED_LINK4);
#elif CONFIG_WDR4300
	if (is_ar934x()) {
	    ar934x_set_ext_lna_gpio(0,18);
	    ar934x_set_ext_lna_gpio(1,19);
	}
#endif
//WR650AC
#ifdef CONFIG_COMFAST_WATCHDOG

#ifdef CONFIG_XD9531
#define	XWDT_TRIGGER	17
	printk(KERN_INFO "setup watchdog XD9531\n");
#endif
#ifdef CONFIG_WR615N
#define	XWDT_TRIGGER	13
	printk(KERN_INFO "setup watchdog WR615N\n");
#endif
#ifdef CONFIG_E380AC
#define	XWDT_TRIGGER	17
	printk(KERN_INFO "setup watchdog E380AC\n");
#endif
#ifdef CONFIG_E325N
#define	XWDT_TRIGGER	16
	printk(KERN_INFO "setup watchdog E325N and LNA\n");
	ext_lna_control_gpio_setup(13, 14);
#endif
#ifdef CONFIG_WR650AC
#define	XWDT_TRIGGER	17
	printk(KERN_INFO "setup watchdog WR650AC\n");
#endif
#ifdef CONFIG_E355AC
#define	XWDT_TRIGGER	13
	printk(KERN_INFO "setup watchdog E355AC\n");
#endif
	ath79_gpio_output_select(XWDT_TRIGGER, 0);	
	enable_external_wdt(XWDT_TRIGGER);
#endif



}
