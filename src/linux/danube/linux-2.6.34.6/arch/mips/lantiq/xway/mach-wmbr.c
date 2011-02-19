/*
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 *  Copyright (C) 2010 John Crispin <blogic@openwrt.org>
 */

#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/leds.h>
#include <linux/gpio.h>
#include <linux/gpio_buttons.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/physmap.h>
#include <linux/input.h>

#include <machine.h>

#include <xway.h>
#include <lantiq_platform.h>

#include "devices.h"
#include "dev-dwc_otg.h"

#ifdef CONFIG_MTD_PARTITIONS
static struct mtd_partition wmbr_partitions[] =
{
	{
		.name	= "uboot",
		.offset	= 0x0,
		.size	= 0x40000,
	},
	{
		.name	= "uboot_env",
		.offset	= 0x40000,
		.size	= 0x20000,
	},
	{
		.name	= "linux",
		.offset	= 0x60000,
		.size	= 0x1f20000,
	},
	{
		.name	= "calibration",
		.offset	= 0x1fe0000,
		.size	= 0x20000,
	},
};
#endif

static struct physmap_flash_data wmbr_flash_data = {
#ifdef CONFIG_MTD_PARTITIONS
	.nr_parts	= ARRAY_SIZE(wmbr_partitions),
	.parts		= wmbr_partitions,
#endif
};

static struct gpio_led
wmbr_leds_gpio[] __initdata = {
	{ .name = "soc:blue:movie", .gpio = 20, .active_low = 1, },
	{ .name = "soc:red:internet", .gpio = 18, .active_low = 1, },
	{ .name = "soc:green:internet", .gpio = 17, .active_low = 1, },
	{ .name = "soc:green:adsl", .gpio = 16, .active_low = 1, },
	{ .name = "soc:green:wlan", .gpio = 15, .active_low = 1, },
	{ .name = "soc:red:security", .gpio = 14, .active_low = 1, },
	{ .name = "soc:green:power", .gpio = 1, .active_low = 1, },
	{ .name = "soc:red:power", .gpio = 5, .active_low = 1, },
	{ .name = "soc:green:usb", .gpio = 28, .active_low = 1, },
};

static struct gpio_button
wmbr_gpio_buttons[] __initdata = {
	{ .desc = "aoss", .type = EV_KEY, .code = BTN_0, .threshold = 3, .gpio = 0, .active_low = 1, },
	{ .desc = "reset", .type = EV_KEY, .code = BTN_1, .threshold = 3, .gpio = 37, .active_low = 1, },
};


static struct lq_pci_data lq_pci_data = {
	.clock      = PCI_CLOCK_INT,
	.gpio   = PCI_GNT1 | PCI_REQ1,
	.irq    = {
		[14] = INT_NUM_IM0_IRL0 + 22,
	},
};

static struct lq_eth_data lq_eth_data = {
	.mii_mode = REV_MII_MODE,
	.mac		= "\xff\xff\xff\xff\xff\xff",
};

static void __init
wmbr_init(void)
{
	lq_register_gpio();
	lq_register_asc(0);
	lq_register_asc(1);
	lq_register_gpio_leds(wmbr_leds_gpio, ARRAY_SIZE(wmbr_leds_gpio));
//	lq_register_gpio_buttons(arv752dpw22_gpio_buttons, ARRAY_SIZE(arv752dpw22_gpio_buttons));
	lq_register_nor(&wmbr_flash_data);
	lq_register_wdt();
	lq_register_pci(&lq_pci_data);
#define WMBR_BRN_MAC			0x1fd0024
	memcpy_fromio(lq_eth_data.mac,(void *)KSEG1ADDR(LQ_FLASH_START + WMBR_BRN_MAC), 6);
	lq_register_ethernet(&lq_eth_data);
	xway_register_dwc(36);
	lq_register_crypto("lq_ar9_deu");
}

MIPS_MACHINE(LANTIQ_MACH_WMBR,
			"WMBR",
			"WMBR",
			wmbr_init);
