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
#include <asm/io.h>
#include <linux/platform_device.h>

/*
 * device registration specific to sl2312.
 */

static u64 sl2312_usb0_dmamask = 0xffffffffUL;

static u64 sl2312_usb1_dmamask = 0xffffffffUL;

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
		.dma_mask = &sl2312_usb0_dmamask,
		.coherent_dma_mask = 0xffffffff,
	},
	.num_resources  = ARRAY_SIZE(sl2312_otg_resources_1),
	.resource       = sl2312_otg_resources_1,
};

static struct platform_device ehci_device_2 = {
	.name		= "ehci-hcd-FOTG2XX",
	.id		= 2,
	.dev		= {
		.dma_mask = &sl2312_usb1_dmamask,
		.coherent_dma_mask = 0xffffffff,
	},
	.num_resources  = ARRAY_SIZE(sl2312_otg_resources_2),
	.resource       = sl2312_otg_resources_2,
};


static struct resource gemini_rtc_resources[] = {
	[0] = {
		.start  = SL2312_RTC_BASE,
		.end    = SL2312_RTC_BASE + 0x24,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = IRQ_RTC,
		.end    = IRQ_RTC,
		.flags  = IORESOURCE_IRQ,
	},
};

static struct platform_device gemini_rtc_device = {
	.name		= "rtc-gemini",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(gemini_rtc_resources),
	.resource	= gemini_rtc_resources,
};

/*
* PATA
*/

static u64 gemini_pata_dmamask0 = 0xffffffffUL;
static u64 gemini_pata_dmamask1 = 0xffffffffUL;

static struct resource gemini_pata_resources0[] = {
	[0] = {
		.start  = SL2312_IDE0_BASE,
		.end    = SL2312_IDE0_BASE + 0x40,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = IRQ_IDE0,
		.end    = IRQ_IDE0,
		.flags  = IORESOURCE_IRQ,
	},
};

static struct resource gemini_pata_resources1[] = {
	[0] = {
		.start  = SL2312_IDE1_BASE,
		.end    = SL2312_IDE1_BASE + 0x40,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = IRQ_IDE1,
		.end    = IRQ_IDE1,
		.flags  = IORESOURCE_IRQ,
	},
};

static struct platform_device gemini_pata_devices[] = {
	{
		.name		= "pata_gemini",
		.id		= 0,
		.dev		= {
			.dma_mask = &gemini_pata_dmamask0,
			.coherent_dma_mask = 0xffffffff,
		},
		.num_resources  = ARRAY_SIZE(gemini_pata_resources0),
		.resource       = gemini_pata_resources0,
	},
	{
		.name		= "pata_gemini",
		.id		= 1,
		.dev		= {
			.dma_mask = &gemini_pata_dmamask1,
			.coherent_dma_mask = 0xffffffff,
		},
		.num_resources  = ARRAY_SIZE(gemini_pata_resources1),
		.resource       = gemini_pata_resources1,
	},
};

int __init platform_register_pata(unsigned int i)
{
	switch (i) {
	case 0:
		return platform_device_register(&gemini_pata_devices[0]);
	case 1:
		return platform_device_register(&gemini_pata_devices[1]);
	default:
		return -EINVAL;
	}
}


static struct platform_device *devices[] __initdata = {
	&gemini_rtc_device,
	&ehci_device_1,
#ifdef CONFIG_MACH_WBD222
	&ehci_device_2,
#endif
};

static int __init sl2312_init(void)
{
//	__raw_writel( (int) RESET_USB0|RESET_USB1, IO_ADDRESS(SL2312_GLOBAL_BASE) + GLOBAL_RESET_REG);

#ifdef CONFIG_MACH_WBD222
	platform_register_pata(1);
#endif
	return platform_add_devices(devices, ARRAY_SIZE(devices));
}

subsys_initcall(sl2312_init);
