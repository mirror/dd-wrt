/*
 *  linux/arch/arm/mach-2312/sl3516_device.c
 *
 *  Author:	Nicolas Pitre
 *  Created:	Nov 05, 2002
 *  Copyright:	MontaVista Software Inc.
 *
 * Code specific to sl2312 aka Bulverde.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/pm.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include "asm/arch/sl2312.h"
#include "asm/arch/irqs.h"
#include <asm/hardware.h>
#include <asm/irq.h>

/*
 * device registration specific to sl2312.
 */

static u64 sl3516_dmamask = 0xffffffffUL;

static struct resource sl3516_sata_resources[] = {
	[0] = {
		.start  = 0x63400000,
		.end    = 0x63400040,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = IRQ_IDE1,
		.end    = IRQ_IDE1,
		.flags  = IORESOURCE_IRQ,
	},
};

static struct platform_device sata_device = {
	.name		= "lepus-sata",
	.id		= -1,
	.dev		= {
		.dma_mask = &sl3516_dmamask,
		.coherent_dma_mask = 0xffffffff,
	},
	.num_resources  = ARRAY_SIZE(sl3516_sata_resources),
	.resource       = sl3516_sata_resources,
};

static struct resource sl3516_sata0_resources[] = {
	[0] = {
		.start  = 0x63000000,
		.end    = 0x63000040,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = IRQ_IDE0,
		.end    = IRQ_IDE0,
		.flags  = IORESOURCE_IRQ,
	},
};

static struct platform_device sata0_device = {
	.name		= "lepus-sata0",
	.id		= -1,
	.dev		= {
		.dma_mask = &sl3516_dmamask,
		.coherent_dma_mask = 0xffffffff,
	},
	.num_resources  = ARRAY_SIZE(sl3516_sata0_resources),
	.resource       = sl3516_sata0_resources,
};

static struct resource sl351x_wdt_resources[] = {
	[0] = {
		.start  = SL2312_WAQTCHDOG_BASE + 0x00,
		.end    = SL2312_WAQTCHDOG_BASE + 0x1C,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = IRQ_WATCHDOG,
		.end    = IRQ_WATCHDOG,
		.flags  = IORESOURCE_IRQ,
	},
};

static struct platform_device sl351x_wdt = {
	.name		= "sl351x-wdt",
	.id		= -1,
	.resource	= sl351x_wdt_resources,
	.num_resources	= ARRAY_SIZE(sl351x_wdt_resources),
};

static struct platform_device *sata_devices[] __initdata = {
	&sata_device,
	&sata0_device,
	&sl351x_wdt,
};

static int __init sl3516_init(void)
{
	return platform_add_devices(sata_devices, ARRAY_SIZE(sata_devices));
}

subsys_initcall(sl3516_init);
