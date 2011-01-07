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

#ifdef CONFIG_MTD_PARTITIONS
static struct mtd_partition easy50812_partitions[] =
{
	{
		.name	= "uboot",
		.offset	= 0x0,
		.size	= 0x40000,
	},
	{
		.name	= "uboot_env",
		.offset	= 0x40000,
		.size	= 0x10000,
	},
	{
		.name	= "linux",
		.offset	= 0x50000,
		.size	= 0x3B0000,
	},
};
#endif

static struct physmap_flash_data easy50812_flash_data = {
#ifdef CONFIG_MTD_PARTITIONS
	.nr_parts	= ARRAY_SIZE(easy50812_partitions),
	.parts		= easy50812_partitions,
#endif
};

static struct lq_pci_data lq_pci_data = {
	.clock      = PCI_CLOCK_INT,
	.req_mask   = 0xf,
};

static struct lq_eth_data lq_eth_data = {
	.mii_mode = REV_MII_MODE,
};

static void __init
easy50812_init(void)
{
	lq_register_gpio();
	lq_register_asc(0);
	lq_register_asc(1);
	lq_register_nor(&easy50812_flash_data);
	lq_register_wdt();
	lq_register_pci(&lq_pci_data);
	lq_register_ethernet(&lq_eth_data);
	lq_register_crypto("lq_ar9_deu");
}

MIPS_MACHINE(LANTIQ_MACH_EASY50812,
			"EASY50812",
			"EASY50812 Eval Board",
			easy50812_init);
