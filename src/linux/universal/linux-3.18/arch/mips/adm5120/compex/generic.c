/*
 *  Compex WP54 board support
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include "compex.h"

#define WP54_KEYS_POLL_INTERVAL		20
#define WP54_KEYS_DEBOUNCE_INTERVAL	(3 * WP54_KEYS_POLL_INTERVAL)

static struct mtd_partition wp54g_wrt_partitions[] = {
	{
		.name	= "cfe",
		.offset	= 0,
		.size	= 0x050000,
		.mask_flags = MTD_WRITEABLE,
	} , {
		.name	= "trx",
		.offset	= MTDPART_OFS_APPEND,
		.size	= 0x3A0000,
	} , {
		.name	= "nvram",
		.offset	= MTDPART_OFS_APPEND,
		.size	= 0x010000,
	}
};

static struct adm5120_pci_irq wp54_pci_irqs[] __initdata = {
	PCIIRQ(2, 0, 1, ADM5120_IRQ_PCI0),
};


static void __init wp54_setup(void)
{
	compex_generic_setup();

	adm5120_pci_set_irq_map(ARRAY_SIZE(wp54_pci_irqs), wp54_pci_irqs);
}

MIPS_MACHINE(MACH_ADM5120_WP54, "WP54", "Compex WP54 family", wp54_setup);

static void __init wp54_wrt_setup(void)
{
	adm5120_flash0_data.nr_parts = ARRAY_SIZE(wp54g_wrt_partitions);
	adm5120_flash0_data.parts = wp54g_wrt_partitions;

	wp54_setup();
}

MIPS_MACHINE(MACH_ADM5120_WP54G_WRT, "WP54G-WRT", "Compex WP54G-WRT",
	     wp54_wrt_setup);
