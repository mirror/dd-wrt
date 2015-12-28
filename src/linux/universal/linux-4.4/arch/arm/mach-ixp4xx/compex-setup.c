/*
 * arch/arm/mach-ixp4xx/compex-setup.c
 *
 * Ccompex WP18 / NP18A board-setup
 *
 * Copyright (C) 2007 Imre Kaloz <Kaloz@openwrt.org>
 *
 * based on ixdp425-setup.c:
 *	Copyright (C) 2003-2005 MontaVista Software, Inc.
 *
 * Author: Imre Kaloz <Kaloz@openwrt.org>
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
#include <mach/hardware.h>
#include <asm/mach-types.h>
#include <asm/irq.h>
#include <asm/mach/arch.h>
#include <asm/mach/flash.h>

static struct flash_platform_data compex_flash_data = {
	.map_name	= "cfi_probe",
	.width		= 2,
};

static struct resource compex_flash_resource = {
	.flags		= IORESOURCE_MEM,
};

static struct platform_device compex_flash = {
	.name		= "IXP4XX-Flash",
	.id		= 0,
	.dev		= {
		.platform_data = &compex_flash_data,
	},
	.num_resources	= 1,
	.resource	= &compex_flash_resource,
};

static struct resource compex_uart_resources[] = {
	{
		.start		= IXP4XX_UART1_BASE_PHYS,
		.end		= IXP4XX_UART1_BASE_PHYS + 0x0fff,
		.flags		= IORESOURCE_MEM
	},
	{
		.start		= IXP4XX_UART2_BASE_PHYS,
		.end		= IXP4XX_UART2_BASE_PHYS + 0x0fff,
		.flags		= IORESOURCE_MEM
	}
};

static struct plat_serial8250_port compex_uart_data[] = {
	{
		.mapbase	= IXP4XX_UART1_BASE_PHYS,
		.membase	= (char *)IXP4XX_UART1_BASE_VIRT + REG_OFFSET,
		.irq		= IRQ_IXP4XX_UART1,
		.flags		= UPF_BOOT_AUTOCONF | UPF_SKIP_TEST,
		.iotype		= UPIO_MEM,
		.regshift	= 2,
		.uartclk	= IXP4XX_UART_XTAL,
	},
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

static struct platform_device compex_uart = {
	.name			= "serial8250",
	.id			= PLAT8250_DEV_PLATFORM,
	.dev.platform_data	= compex_uart_data,
	.num_resources		= 2,
	.resource		= compex_uart_resources
};

static struct platform_device *compex_devices[] __initdata = {
	&compex_flash,
	&compex_uart
};

/*#define PCI_CSR_HOST               (1 << 0)
#define PCI_CSR_ARBEN              (1 << 1)
#define PCI_CSR_ADS                (1 << 2)
#define PCI_CSR_PDS                (1 << 3)
#define PCI_CSR_ABE                (1 << 4)
#define PCI_CSR_DBT                (1 << 5)
#define PCI_CSR_ASE                (1 << 8)
#define PCI_CSR_IC                 (1 << 15)
#define PCI_CSR_PRST               (1 << 16)
#define REG32(a,b) ((volatile unsigned int *)((a)+(b)))
#define IXP425_PCI_CFG_BASE        0xC0000000
#define IXP425_PCI_CSR             REG32(IXP425_PCI_CFG_BASE,0x1C)
*/
static void __init compex_init(void)
{
 	u32 data;
 	int i;
 
 	/* WP188 support,
 	   WP188 has a RTL8201 (phy addr = 1) in place of the Marvell switch */
/* 	data = (1<<31) | (1<<21) | (3<<16);
 	for(i=0; i<4; i++) {
 		*(u32 *) (IXP4XX_EthB_BASE_VIRT + 0x80 + i*4) = data & 0xff;
 		data >>=8;
 	}
 	for ( i=0 ; i<5000 ; i++ ) {
 		data = *(u32 *) (IXP4XX_EthB_BASE_VIRT + 0x80 + 3*4);
 		if ( (data & 0x80) == 0 )
 			break;
 	}
 	for(i=0; i<4; i++) {
 		data |= (*(u32 *) (IXP4XX_EthB_BASE_VIRT + 0x90 + i*4) & 0xff) << (i*8);
 	}
 	data &= 0xffff;
*/     

	ixp4xx_sys_init();

	compex_flash_resource.start = IXP4XX_EXP_BUS_BASE(0);
	compex_flash_resource.end =
		IXP4XX_EXP_BUS_BASE(0) + SZ_32M - 1;

// 	if ( data == 0x8201 ) {
 //		compex_flash_resource.end =
 //			IXP4XX_EXP_BUS_BASE(0) + 2*ixp4xx_exp_bus_size - 1;
 //	}
 		

	platform_add_devices(compex_devices, ARRAY_SIZE(compex_devices));
}

#ifdef CONFIG_MACH_COMPEX
MACHINE_START(COMPEX, "Compex WP18 / NP18A / WP188")
	/* Maintainer: Imre Kaloz <Kaloz@openwrt.org> */
	.map_io		= ixp4xx_map_io,
	.init_early	= ixp4xx_init_early,
	.init_irq	= ixp4xx_init_irq,
	.init_time	= ixp4xx_timer_init,
	.atag_offset	= 0x0100,
	.init_machine	= compex_init,
#if defined(CONFIG_PCI)
	.dma_zone_size	= SZ_64M,
#endif
	.restart	= ixp4xx_restart,
MACHINE_END
#endif
