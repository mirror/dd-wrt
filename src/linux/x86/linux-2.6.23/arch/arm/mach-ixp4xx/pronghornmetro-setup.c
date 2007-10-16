/*
 * arch/arm/mach-ixp4xx/pronghornmetro-setup.c
 *
 * Board setup for ADI Engineering Pronghorn Metro
 *
 * Copyright (C) 2003-2005 MontaVista Software, Inc.
 *
 * Author: Copied from coyote-setup.c
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/serial.h>
#include <linux/tty.h>
#include <linux/serial_8250.h>
#include <linux/slab.h>

#include <asm/types.h>
#include <asm/setup.h>
#include <asm/memory.h>
#include <asm/hardware.h>
#include <asm/irq.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/flash.h>

static struct flash_platform_data pronghornmetro_flash_data = {
	.map_name	= "cfi_probe",
	.width		= 2,
};

static struct resource pronghornmetro_flash_resource = {
	.flags		= IORESOURCE_MEM,
};

static struct platform_device pronghornmetro_flash = {
	.name		= "IXP4XX-Flash",
	.id		= 0,
	.dev		= {
		.platform_data = &pronghornmetro_flash_data,
	},
	.num_resources	= 1,
	.resource	= &pronghornmetro_flash_resource,
};

static struct resource pronghornmetro_uart_resource = {
	.start	= IXP4XX_UART2_BASE_PHYS,
	.end	= IXP4XX_UART2_BASE_PHYS + 0x0fff,
	.flags	= IORESOURCE_MEM,
};

static struct ixp4xx_i2c_pins pronghornmetro_i2c_gpio_pins = {
	.sda_pin	= PRONGHORNMETRO_SDA_PIN,
	.scl_pin	= PRONGHORNMETRO_SCL_PIN,
};

static struct platform_device pronghornmetro_i2c_controller = {
	.name		= "IXP4XX-I2C",
	.id		= 0,
	.dev		= {
		.platform_data = &pronghornmetro_i2c_gpio_pins,
	},
	.num_resources	= 0
};

static struct plat_serial8250_port pronghornmetro_uart_data[] = {
	{
		.mapbase	= IXP4XX_UART2_BASE_PHYS,
		.membase	= (char *)IXP4XX_UART2_BASE_VIRT + REG_OFFSET,
		.irq		= IRQ_IXP4XX_UART2,
		.flags		= UPF_BOOT_AUTOCONF | UPF_SKIP_TEST,
		.iotype		= UPIO_MEM,
		.regshift	= 2,
		.uartclk	= IXP4XX_UART_XTAL,
	},
	{ },
};

static struct platform_device pronghornmetro_uart = {
	.name		= "serial8250",
	.id		= PLAT8250_DEV_PLATFORM,
	.dev			= {
		.platform_data	= pronghornmetro_uart_data,
	},
	.num_resources	= 1,
	.resource	= &pronghornmetro_uart_resource,
};

static struct platform_device *pronghornmetro_devices[] __initdata = {
	&pronghornmetro_i2c_controller,
	&pronghornmetro_flash,
	&pronghornmetro_uart
};

static void __init pronghornmetro_init(void)
{
	ixp4xx_sys_init();

	pronghornmetro_flash_resource.start = IXP4XX_EXP_BUS_BASE(0);
	pronghornmetro_flash_resource.end = IXP4XX_EXP_BUS_BASE(0) + SZ_32M - 1;

	*IXP4XX_EXP_CS0 |= IXP4XX_FLASH_WRITABLE;
	*IXP4XX_EXP_CS1 = *IXP4XX_EXP_CS0;

	platform_add_devices(pronghornmetro_devices, ARRAY_SIZE(pronghornmetro_devices));
}

#ifdef CONFIG_MACH_PRONGHORNMETRO
MACHINE_START(PRONGHORNMETRO, "ADI Engineering Pronghorn Metro")
	.phys_io	= IXP4XX_PERIPHERAL_BASE_PHYS,
	.io_pg_offst	= ((IXP4XX_PERIPHERAL_BASE_VIRT) >> 18) & 0xfffc,
	.map_io		= ixp4xx_map_io,
	.init_irq	= ixp4xx_init_irq,
	.timer		= &ixp4xx_timer,
	.boot_params	= 0x0100,
	.init_machine	= pronghornmetro_init,
MACHINE_END
#endif
