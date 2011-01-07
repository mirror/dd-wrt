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
#include <linux/etherdevice.h>

#include <machine.h>

#include <xway.h>
#include <lantiq_platform.h>

#include "devices.h"
#include "dev-dwc_otg.h"

#define ARV452_LATCH_SWITCH		(1 << 10)

#ifdef CONFIG_MTD_PARTITIONS
static struct mtd_partition arv45xx_partitions[] =
{
	{
		.name	= "uboot",
		.offset	= 0x0,
		.size	= 0x20000,
	},
	{
		.name	= "uboot_env",
		.offset	= 0x20000,
		.size	= 0x10000,
	},
	{
		.name	= "linux",
		.offset	= 0x30000,
		.size	= 0x3c0000,
	},
	{
		.name	= "board_config",
		.offset	= 0x3f0000,
		.size	= 0x10000,
	},
};
#endif

static struct physmap_flash_data arv45xx_flash_data = {
#ifdef CONFIG_MTD_PARTITIONS
	.nr_parts	= ARRAY_SIZE(arv45xx_partitions),
	.parts		= arv45xx_partitions,
#endif
};

static struct lq_pci_data lq_pci_data = {
	.clock      = PCI_CLOCK_EXT,
	.req_mask   = 0xf,
};

static struct lq_eth_data lq_eth_data = {
	.mii_mode	= REV_MII_MODE,
	.mac		= "\xff\xff\xff\xff\xff\xff",
};

static struct gpio_led
arv4518_leds_gpio[] __initdata = {
	{ .name = "soc:blue:power", .gpio = 3, .active_low = 1, },
	{ .name = "soc:blue:adsl", .gpio = 4, .active_low = 1, },
	{ .name = "soc:blue:internet", .gpio = 5, .active_low = 1, },
	{ .name = "soc:red:power", .gpio = 6, .active_low = 1, },
	{ .name = "soc:yello:wps", .gpio = 7, .active_low = 1, },
	{ .name = "soc:red:wps", .gpio = 9, .active_low = 1, },
	{ .name = "soc:blue:voip", .gpio = 32, .active_low = 1, },
	{ .name = "soc:blue:fxs1", .gpio = 33, .active_low = 1, },
	{ .name = "soc:blue:fxs2", .gpio = 34, .active_low = 1, },
	{ .name = "soc:blue:fxo", .gpio = 35, .active_low = 1, },
	{ .name = "soc:blue:voice", .gpio = 36, .active_low = 1, },
	{ .name = "soc:blue:usb", .gpio = 37, .active_low = 1, },
	{ .name = "soc:blue:wlan", .gpio = 38, .active_low = 1, },
};

static struct gpio_led
arv452_leds_gpio[] __initdata = {
	{ .name = "soc:blue:power", .gpio = 3, .active_low = 1, },
	{ .name = "soc:blue:adsl", .gpio = 4, .active_low = 1, },
	{ .name = "soc:blue:internet", .gpio = 5, .active_low = 1, },
	{ .name = "soc:red:power", .gpio = 6, .active_low = 1, },
	{ .name = "soc:yello:wps", .gpio = 7, .active_low = 1, },
	{ .name = "soc:red:wps", .gpio = 9, .active_low = 1, },
	{ .name = "soc:blue:voip", .gpio = 32, .active_low = 1, },
	{ .name = "soc:blue:fxs1", .gpio = 33, .active_low = 1, },
	{ .name = "soc:blue:fxs2", .gpio = 34, .active_low = 1, },
	{ .name = "soc:blue:fxo", .gpio = 35, .active_low = 1, },
	{ .name = "soc:blue:voice", .gpio = 36, .active_low = 1, },
	{ .name = "soc:blue:usb", .gpio = 37, .active_low = 1, },
	{ .name = "soc:blue:wlan", .gpio = 38, .active_low = 1, },
};

static struct gpio_led arv4525_leds_gpio[] __initdata = {
	{ .name = "soc:green:festnetz", .gpio = 4, .active_low = 1, },
	{ .name = "soc:green:internet", .gpio = 5, .active_low = 1, },
	{ .name = "soc:green:dsl", .gpio = 6, .active_low = 1, },
	{ .name = "soc:green:wlan", .gpio = 8, .active_low = 1, },
	{ .name = "soc:green:online", .gpio = 9, .active_low = 1, },
};

static void
arv45xx_register_ethernet(void)
{
#define ARV45XX_BRN_MAC			0x3f0016
	memcpy_fromio(lq_eth_data.mac,
		(void *)KSEG1ADDR(LQ_FLASH_START + ARV45XX_BRN_MAC), 6);
	lq_register_ethernet(&lq_eth_data);
}

static void __init
arv4518_init(void)
{
	lq_register_gpio();
	lq_register_gpio_ebu(0);
	lq_register_gpio_leds(arv4518_leds_gpio, ARRAY_SIZE(arv4518_leds_gpio));
	lq_register_asc(0);
	lq_register_asc(1);
	lq_register_nor(&arv45xx_flash_data);
	lq_register_pci(&lq_pci_data);
	lq_register_wdt();
	arv45xx_register_ethernet();
	xway_register_dwc(14);
}

MIPS_MACHINE(LANTIQ_MACH_ARV4518,
			"ARV4518",
			"ARV4518 - SMC7908A-ISP",
			arv4518_init);

static void __init
arv452_init(void)
{
	lq_register_gpio();
	lq_register_gpio_ebu(ARV452_LATCH_SWITCH);
	lq_register_gpio_leds(arv452_leds_gpio, ARRAY_SIZE(arv452_leds_gpio));
	lq_register_asc(0);
	lq_register_asc(1);
	lq_register_nor(&arv45xx_flash_data);
	lq_register_pci(&lq_pci_data);
	lq_register_wdt();
	arv45xx_register_ethernet();
	xway_register_dwc(28);
}

MIPS_MACHINE(LANTIQ_MACH_ARV452,
			"ARV452",
			"ARV452 - Airties WAV-281, Arcor A800",
			arv452_init);

static void __init
arv4525_init(void)
{
	lq_register_gpio();
	lq_register_gpio_leds(arv4525_leds_gpio, ARRAY_SIZE(arv4525_leds_gpio));
	lq_register_asc(0);
	lq_register_asc(1);
	lq_register_nor(&arv45xx_flash_data);
	lq_register_pci(&lq_pci_data);
	lq_register_wdt();
	lq_eth_data.mii_mode = MII_MODE;
	arv45xx_register_ethernet();
}

MIPS_MACHINE(LANTIQ_MACH_ARV4525,
			"ARV4525",
			"ARV4525 - Speedport W502V",
			arv4525_init);
