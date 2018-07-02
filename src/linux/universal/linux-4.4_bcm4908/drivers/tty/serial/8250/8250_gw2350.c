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


static struct resource cambria_optional_uart_resources[] = {
	{
		.start	= 0x52000000,
		.end	= 0x52000fff,
		.flags	= IORESOURCE_MEM
	},
	{
		.start	= 0x53000000,
		.end	= 0x53000fff,
		.flags	= IORESOURCE_MEM
	}
};

static struct plat_serial8250_port cambria_optional_uart_data[] = {
	{
		.flags		= UPF_BOOT_AUTOCONF,
		.iotype		= UPIO_MEM_DELAY,
		.regshift	= 0,
		.uartclk	= 1843200,
		.rw_delay	= 2,
	},
	{
		.flags		= UPF_BOOT_AUTOCONF,
		.iotype		= UPIO_MEM_DELAY,
		.regshift	= 0,
		.uartclk	= 1843200,
		.rw_delay	= 2,
	},
  { },
};

static struct platform_device cambria_optional_uart = {
	.name		= "serial8250",
	.id		= PLAT8250_DEV_PLATFORM1,
	.dev.platform_data	= cambria_optional_uart_data,
	.num_resources	= 2,
	.resource	= cambria_optional_uart_resources,
};

static int __init gw2350_init(void)
{

	*IXP4XX_EXP_CS2 = 0xBFFF3C43;
	irq_set_irq_type(IRQ_IXP4XX_GPIO3, IRQ_TYPE_EDGE_RISING);
	cambria_optional_uart_data[0].mapbase	= 0x52FF0000;
	cambria_optional_uart_data[0].membase	= (void __iomem *)ioremap(0x52FF0000, 0x0fff);
	cambria_optional_uart_data[0].irq		= IRQ_IXP4XX_GPIO3;

	*IXP4XX_EXP_CS3 = 0xBFFF3C43;
	irq_set_irq_type(IRQ_IXP4XX_GPIO4, IRQ_TYPE_EDGE_RISING);
	cambria_optional_uart_data[1].mapbase	= 0x53FF0000;
	cambria_optional_uart_data[1].membase	= (void __iomem *)ioremap(0x53FF0000, 0x0fff);
	cambria_optional_uart_data[1].irq		= IRQ_IXP4XX_GPIO4;

	platform_device_register(&cambria_optional_uart);

return 0;
}

/* XXX: Yes, I know this doesn't yet work. */
static void __exit gw2350_exit(void)
{
	platform_device_unregister(&cambria_optional_uart);
}

module_init(gw2350_init);
module_exit(gw2350_exit);

MODULE_AUTHOR("Chris Lang <clang@gateworks.com>");
MODULE_DESCRIPTION("8250 serial probe module for GW2350");
MODULE_LICENSE("GPL");
