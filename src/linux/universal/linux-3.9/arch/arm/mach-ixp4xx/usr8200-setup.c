/*
 * arch/arm/mach-ixp4xx/usr8200-setup.c
 *
 * Board setup for the USRobotics USR8200
 *
 * Copyright (C) 2008 Peter Denison <openwrt@marshadder.org>
 *
 * based on pronghorn-setup.c:
 * 	Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 * based on coyote-setup.c:
 *      Copyright (C) 2003-2005 MontaVista Software, Inc.
 *
 * Author: Peter Denison <openwrt@marshadder.org>
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/serial.h>
#include <linux/tty.h>
#include <linux/serial_8250.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/memory.h>
#include <linux/i2c-gpio.h>
#include <linux/leds.h>

#include <asm/setup.h>
#include <asm/irq.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/flash.h>

static struct flash_platform_data usr8200_flash_data = {
	.map_name	= "cfi_probe",
	.width		= 2,
};

static struct resource usr8200_flash_resource = {
	.flags		= IORESOURCE_MEM,
};

static struct platform_device usr8200_flash = {
	.name		= "IXP4XX-Flash",
	.id		= 0,
	.dev		= {
		.platform_data	= &usr8200_flash_data,
	},
	.num_resources	= 1,
	.resource	= &usr8200_flash_resource,
};

static struct resource usr8200_uart_resources [] = {
	{
		.start		= IXP4XX_UART2_BASE_PHYS,
		.end		= IXP4XX_UART2_BASE_PHYS + 0x0fff,
		.flags		= IORESOURCE_MEM
	},
	{
		.start		= IXP4XX_UART1_BASE_PHYS,
		.end		= IXP4XX_UART1_BASE_PHYS + 0x0fff,
		.flags		= IORESOURCE_MEM
	}
};

static struct plat_serial8250_port usr8200_uart_data[] = {
	{
		.mapbase	= IXP4XX_UART2_BASE_PHYS,
		.membase	= (char *)IXP4XX_UART2_BASE_VIRT + REG_OFFSET,
		.irq		= IRQ_IXP4XX_UART2,
		.flags		= UPF_BOOT_AUTOCONF | UPF_SKIP_TEST,
		.iotype		= UPIO_MEM,
		.regshift	= 2,
		.uartclk	= IXP4XX_UART_XTAL,
	},
	{
		.mapbase	= IXP4XX_UART1_BASE_PHYS,
		.membase	= (char *)IXP4XX_UART1_BASE_VIRT + REG_OFFSET,
		.irq		= IRQ_IXP4XX_UART1,
		.flags		= UPF_BOOT_AUTOCONF | UPF_SKIP_TEST,
		.iotype		= UPIO_MEM,
		.regshift	= 2,
		.uartclk	= IXP4XX_UART_XTAL,
	},
	{ },
};

static struct platform_device usr8200_uart = {
	.name		= "serial8250",
	.id		= PLAT8250_DEV_PLATFORM,
	.dev		= {
		.platform_data	= usr8200_uart_data,
	},
	.num_resources	= 2,
	.resource	= usr8200_uart_resources,
};

static struct gpio_led usr8200_led_pin[] = {
	{
		.name		= "usr8200:usb1",
		.gpio		= 0,
		.active_low	= 1,
	},
	{
		.name		= "usr8200:usb2",
		.gpio		= 1,
		.active_low	= 1,
	},
	{
		.name		= "usr8200:ieee1394",
		.gpio		= 2,
		.active_low	= 1,
	},
	{
		.name		= "usr8200:internal",
		.gpio		= 3,
		.active_low	= 1,
	},
	{
		.name		= "usr8200:power",
		.gpio		= 14,
	}
};

static struct gpio_led_platform_data usr8200_led_data = {
	.num_leds		= ARRAY_SIZE(usr8200_led_pin),
	.leds			= usr8200_led_pin,
};

static struct platform_device usr8200_led = {
	.name			= "leds-gpio",
	.id			= -1,
	.dev.platform_data	= &usr8200_led_data,
};

#if 0
static struct eth_plat_info usr8200_plat_eth[] = {
	{ /* NPEC - LAN with Marvell 88E6060 switch */
		.phy		= IXP4XX_ETH_PHY_MAX_ADDR,
		.phy_mask	= 0x0F0000,
		.rxq		= 4,
		.txreadyq	= 21,
	}, { /* NPEB - WAN */
		.phy		= 9,
		.rxq		= 3,
		.txreadyq	= 20,
	}
};
#endif

static struct resource usr8200_rtc_resources = {
	.flags		= IORESOURCE_MEM
};

static struct platform_device usr8200_rtc = {
	.name		= "rtc7301",
	.id		= 0,
	.num_resources	= 1,
	.resource	= &usr8200_rtc_resources,
};

static struct platform_device *usr8200_devices[] __initdata = {
	&usr8200_flash,
	&usr8200_uart,
	&usr8200_led,
	&usr8200_rtc,
};

#define IXP4XX_ETH_NPEA		0x00
#define IXP4XX_ETH_NPEB		0x10
#define IXP4XX_ETH_NPEC		0x20

static void __init usr8200_init(void)
{
	ixp4xx_sys_init();

	usr8200_flash_resource.start = IXP4XX_EXP_BUS_BASE(0);
	usr8200_flash_resource.end = IXP4XX_EXP_BUS_BASE(0) + SZ_16M - 1;

	usr8200_rtc_resources.start = IXP4XX_EXP_BUS_BASE(2);
	usr8200_rtc_resources.end = IXP4XX_EXP_BUS_BASE(2) + 0x01ff;

	*IXP4XX_EXP_CS0 |= IXP4XX_FLASH_WRITABLE;
	*IXP4XX_EXP_CS2 = 0x3fff000 | IXP4XX_EXP_BUS_SIZE(0) | IXP4XX_EXP_BUS_WR_EN |
	                  IXP4XX_EXP_BUS_CS_EN | IXP4XX_EXP_BUS_BYTE_EN;
	*IXP4XX_GPIO_GPCLKR = 0x01100000;


	/* configure button as input */
	gpio_line_config(12, IXP4XX_GPIO_IN);

	platform_add_devices(usr8200_devices, ARRAY_SIZE(usr8200_devices));
}

MACHINE_START(USR8200, "USRobotics USR8200")
	/* Maintainer: Peter Denison <openwrt@marshadder.org> */
	.map_io		= ixp4xx_map_io,
	.init_early	= ixp4xx_init_early,
	.init_irq	= ixp4xx_init_irq,
	.init_time	= ixp4xx_timer_init,
	.atag_offset	= 0x0100,
	.init_machine	= usr8200_init,
#if defined(CONFIG_PCI)
	.dma_zone_size	= SZ_64M,
#endif
	.restart	= ixp4xx_restart,
MACHINE_END
