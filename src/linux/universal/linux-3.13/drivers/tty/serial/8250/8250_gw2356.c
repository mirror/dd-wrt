/*
 * Serial Device Initialisation for Au1x00
 *
 * (C) Copyright Embedded Alley Solutions, Inc 2005
 * Author: Pantelis Antoniou <pantelis@embeddedalley.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/errno.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/serial_core.h>
#include <linux/signal.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/irq.h>
#include <linux/platform_device.h>

#include <asm/io.h>

#include <linux/serial_8250.h>


#include "8250.h"


struct plat_serial8250_port gw2356_data[] = {
	{
		.flags = UPF_BOOT_AUTOCONF,
	},
	{
		.flags = UPF_BOOT_AUTOCONF,
	},
	{ },
};

static struct platform_device gw2356_device = {
	.name			= "serial8250",
	.id			= PLAT8250_DEV_PLATFORM1,
	.dev			= {
		.platform_data = &gw2356_data
	},
};

static int __init gw2356_init(void)
{

	*IXP4XX_EXP_CS2 = 0xBFFF3C43;
	*IXP4XX_EXP_CS3 = 0xBFFF3C43;


	set_irq_type(IRQ_IXP4XX_GPIO2, IRQT_BOTHEDGE);
	set_irq_type(IRQ_IXP4XX_GPIO12, IRQT_BOTHEDGE);

	gw2356_data[0].mapbase	= IXP4XX_EXP_BUS_BASE(2);
	gw2356_data[0].membase	= (void __iomem *)ioremap(IXP4XX_EXP_BUS_BASE(2), ixp4xx_exp_bus_size);
	gw2356_data[0].irq		= IRQ_IXP4XX_GPIO2;
	gw2356_data[0].uartclk	= 1843200;
	gw2356_data[0].regshift	= 0;
	gw2356_data[0].iotype		= UPIO_MEM;

	gw2356_data[1].mapbase	= IXP4XX_EXP_BUS_BASE(3);
	gw2356_data[1].membase	= (void __iomem *)ioremap(IXP4XX_EXP_BUS_BASE(3), ixp4xx_exp_bus_size);
	gw2356_data[1].irq		= IRQ_IXP4XX_GPIO12;
	gw2356_data[1].uartclk	= 1843200;
	gw2356_data[1].regshift	= 0;
	gw2356_data[1].iotype		= UPIO_MEM;


	return platform_device_register(&gw2356_device);
}

/* XXX: Yes, I know this doesn't yet work. */
static void __exit gw2356_exit(void)
{
	platform_device_unregister(&gw2356_device);
}

module_init(gw2356_init);
module_exit(gw2356_exit);

MODULE_AUTHOR("Chris Lang <clang@gateworks.com>");
MODULE_DESCRIPTION("8250 serial probe module for GW2356");
MODULE_LICENSE("GPL");
