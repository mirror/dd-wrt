/*
 *  linux/arch/arm/mach-pxa/sl2312.c
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
#include "asm/arch/sl2312.h"
#include "asm/arch/irqs.h"
#include <asm/hardware.h>
#include <asm/irq.h>
#include <linux/platform_device.h>

/*
 * device registration specific to sl2312.
 */

static u64 sl2312_dmamask = 0xffffffffUL;

static struct resource sl2312_otg_resources_1[] = {
	[0] = {
		.start  = 0x68000000,
		.end    = 0x68000fff,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = IRQ_USB0,
		.end    = IRQ_USB0,
		.flags  = IORESOURCE_IRQ,
	},
};
static struct resource sl2312_otg_resources_2[] = {
	[2] = {
		.start  = 0x69000000,
		.end    = 0x69000fff,
		.flags  = IORESOURCE_MEM,
	},
	[3] = {
		.start  = IRQ_USB1,
		.end    = IRQ_USB1,
		.flags  = IORESOURCE_IRQ,
	},
};

static struct platform_device ehci_device_1 = { 
	.name		= "ehci-hcd-FOTG2XX",
	.id		= 1,
	.dev		= {
		.dma_mask = &sl2312_dmamask,
		.coherent_dma_mask = 0xffffffff,
	},
	.num_resources  = ARRAY_SIZE(sl2312_otg_resources_1),
	.resource       = sl2312_otg_resources_1,
};

static struct platform_device ehci_device_2 = {
	.name		= "ehci-hcd-FOTG2XX",
	.id		= 2,
	.dev		= {
		.dma_mask = &sl2312_dmamask,
		.coherent_dma_mask = 0xffffffff,
	},
	.num_resources  = ARRAY_SIZE(sl2312_otg_resources_2),
	.resource       = sl2312_otg_resources_2,
};

static struct platform_device *devices[] __initdata = {
	&ehci_device_1,
	//&ehci_device_2,
};

static int __init sl2312_init(void)
{
	return platform_add_devices(devices, ARRAY_SIZE(devices));
}

subsys_initcall(sl2312_init);
