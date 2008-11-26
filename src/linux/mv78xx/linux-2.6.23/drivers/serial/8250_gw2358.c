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


struct plat_serial8250_port gw2358_data[] = {
	{
		.flags = UPF_BOOT_AUTOCONF,
	},
	{
		.flags = UPF_BOOT_AUTOCONF,
	},
	{ },
};

static struct platform_device gw2358_device = {
	.name			= "serial8250",
	.id			= PLAT8250_DEV_PLATFORM1,
	.dev			= {
		.platform_data = &gw2358_data
	},
};

static int __init gw2358_init(void)
{

	*IXP4XX_EXP_CS3 |= 0xbfff3c03;
	*IXP4XX_EXP_CS3 |= 0xbfff3c03;

	printk("gw2358_serial\n");
	set_irq_type(IRQ_IXP4XX_GPIO3, IRQT_BOTHEDGE);
	set_irq_type(IRQ_IXP4XX_GPIO4, IRQT_BOTHEDGE);

//	gw2358_data[0].mapbase	= 0x53F80000;
//	gw2358_data[0].membase	= (void __iomem *)ioremap(0x53f80000, 0x40000);
	gw2358_data[0].mapbase	= 0x53FC0000;
	gw2358_data[0].membase	= (void __iomem *)ioremap(0x53FC0000, 0x40000);
	gw2358_data[0].irq		= IRQ_IXP4XX_GPIO3;
	gw2358_data[0].uartclk	= 1843200;
	gw2358_data[0].regshift	= 0;
	gw2358_data[0].iotype		= UPIO_MEM;

//	gw2358_data[1].mapbase	= 0x53FC0000;
//	gw2358_data[1].membase	= (void __iomem *)ioremap(0x53FC0000, 0x40000);
	gw2358_data[1].mapbase	= 0x53F80000;
	gw2358_data[1].membase	= (void __iomem *)ioremap(0x53f80000, 0x40000);
	gw2358_data[1].irq		= IRQ_IXP4XX_GPIO4;
	gw2358_data[1].uartclk	= 1843200;
	gw2358_data[1].regshift	= 0;
	gw2358_data[1].iotype		= UPIO_MEM;


	return platform_device_register(&gw2358_device);
}

/* XXX: Yes, I know this doesn't yet work. */
static void __exit gw2358_exit(void)
{
	platform_device_unregister(&gw2358_device);
}

module_init(gw2358_init);
module_exit(gw2358_exit);

MODULE_AUTHOR("Chris Lang <clang@gateworks.com>");
MODULE_DESCRIPTION("8250 serial probe module for GW2358");
MODULE_LICENSE("GPL");
