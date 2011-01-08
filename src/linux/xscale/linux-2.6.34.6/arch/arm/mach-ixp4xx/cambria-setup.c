/*
 * arch/arm/mach-ixp4xx/cambria-setup.c
 *
 * Cambria board-setup 
 *
 * Copyright (C) 2008 Gateworks Corporation
 *
 * Author: Chris Lang <clang@gateworks.com
 *
 *
 * Based largely on ixdp425_setup.c
 * 		Author: Deepak Saxena <dsaxena@plexity.net>
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/i2c/at24.h>
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
#include <asm/io.h>
#include <linux/irq.h>


static struct flash_platform_data cambria_flash_data = {
	.map_name	= "cfi_probe",
	.width		= 2,
};

static struct resource cambria_flash_resource = {
	.flags		= IORESOURCE_MEM,
};

static struct platform_device cambria_flash = {
	.name		= "IXP4XX-Flash",
	.id		= 0,
	.dev		= {
		.platform_data = &cambria_flash_data,
	},
	.num_resources	= 1,
	.resource	= &cambria_flash_resource,
};

static struct ixp4xx_i2c_pins cambria_i2c_gpio_pins = {
	.sda_pin	= CAMBRIA_SDA_PIN,
	.scl_pin	= CAMBRIA_SCL_PIN,
};

static struct platform_device cambria_i2c_controller = {
	.name		= "IXP4XX-I2C",
	.id		= 0,
	.dev		= {
		.platform_data = &cambria_i2c_gpio_pins,
	},
	.num_resources	= 0
};

static struct resource cambria_uart_resources[] = {
	{
		.start		= IXP4XX_UART1_BASE_PHYS,
		.end		= IXP4XX_UART1_BASE_PHYS + 0x0fff,
		.flags		= IORESOURCE_MEM
	}
};

static struct plat_serial8250_port cambria_uart_data[] = {
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

static struct platform_device cambria_uart = {
	.name			= "serial8250",
	.id			= PLAT8250_DEV_PLATFORM,
	.dev.platform_data	= cambria_uart_data,
	.num_resources		= 1,
	.resource		= cambria_uart_resources
};




static struct resource cambria_pata_resources[] = {
  {
    .flags  = IORESOURCE_MEM
  },
  {
    .flags  = IORESOURCE_MEM,
  },
  {
    .name = "intrq",
    .start  = IRQ_IXP4XX_GPIO12,
    .end  = IRQ_IXP4XX_GPIO12,
    .flags  = IORESOURCE_IRQ,
  },
};

static struct ixp4xx_pata_data cambria_pata_data = {
  .cs0_bits = 0xbfff3c03,
	.cs1_bits = 0xbfff3c03,
};

static struct platform_device cambria_pata = {
  .name     = "pata_ixp4xx_cf",
  .id     = 0,
  .dev.platform_data      = &cambria_pata_data,
  .num_resources    = ARRAY_SIZE(cambria_pata_resources),
  .resource   = cambria_pata_resources,
};

static struct platform_device cambria_leds_pld = {
  .name   = "IXP4XX-PLD-LED",
  .id   = -1,
  .num_resources  = 0,
  //.resource = avila_led_resources,
};

static struct platform_device cambria_leds_mem = {
  .name   = "IXP4XX-MEM-LED",
  .id   = -1,
  .num_resources  = 0,
  //.resource = avila_led_resources,
};

static struct resource cambria_usb0_resources[] = {
	{
		.start	= 0xCD000000,
		.end	= 0xCD000300,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= 32,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct resource cambria_usb1_resources[] = {
	{
		.start	= 0xCE000000,
		.end	= 0xCE000300,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= 33,
		.flags	= IORESOURCE_IRQ,
	},
};

static u64 ehci_dma_mask = ~(u32)0;

static struct platform_device cambria_usb0_device =  {
	.name		= "ixp4xx-ehci",
	.id		= 0,
	.resource	= cambria_usb0_resources,
	.num_resources	= ARRAY_SIZE(cambria_usb0_resources),
	.dev = {
		.dma_mask		= &ehci_dma_mask,
		.coherent_dma_mask	= 0xffffffff,
	},
};

static struct platform_device cambria_usb1_device = {
	.name		= "ixp4xx-ehci",
	.id		= 1,
	.resource	= cambria_usb1_resources,
	.num_resources	= ARRAY_SIZE(cambria_usb1_resources),
	.dev = {
		.dma_mask		= &ehci_dma_mask,
		.coherent_dma_mask	= 0xffffffff,
	},
};




static struct platform_device *cambria_devices[] __initdata = {
	&cambria_i2c_controller,
	&cambria_flash,
	&cambria_uart,
	&cambria_leds_pld,
	&cambria_leds_mem
};

static struct at24_platform_data cambria_eeprom_info = {
	.byte_len	= 1024,
	.page_size	= 16,
	.flags		= AT24_FLAG_READONLY,
//	.setup		= at24_setup,
};

static struct i2c_board_info __initdata cambria_i2c_board_info[] = {
	{
		I2C_BOARD_INFO("ds1672", 0x68),
	},
	{
		I2C_BOARD_INFO("ad7418", 0x28),
	},
	{
		I2C_BOARD_INFO("24c08", 0x51),		
		.platform_data	= &cambria_eeprom_info
	},
};


static void __init cambria_init(void)
{
	ixp4xx_sys_init();

	cambria_flash_resource.start = IXP4XX_EXP_BUS_BASE(0);
	cambria_flash_resource.end =
		IXP4XX_EXP_BUS_BASE(0) + ixp4xx_exp_bus_size + ixp4xx_exp_bus_size - 1;


  platform_add_devices(cambria_devices, ARRAY_SIZE(cambria_devices));

  cambria_pata_resources[0].start = 0x53e00000;
  cambria_pata_resources[0].end = 0x53e3ffff;

  cambria_pata_resources[1].start = 0x53e40000;
  cambria_pata_resources[1].end = 0x53e7ffff;

  cambria_pata_data.cs0_cfg = IXP4XX_EXP_CS3;
  cambria_pata_data.cs1_cfg = IXP4XX_EXP_CS3;


  platform_device_register(&cambria_usb0_device);
  platform_device_register(&cambria_usb1_device);
  platform_device_register(&cambria_pata);
  i2c_register_board_info(0, cambria_i2c_board_info, ARRAY_SIZE(cambria_i2c_board_info));
}


MACHINE_START(CAMBRIA, "Gateworks Cambria Series")
  /* Maintainer: Chris Lang <clang@gateworks.com> */
  .phys_io  = IXP4XX_PERIPHERAL_BASE_PHYS,
  .io_pg_offst  = ((IXP4XX_PERIPHERAL_BASE_VIRT) >> 18) & 0xfffc,
  .map_io   = ixp4xx_map_io,
  .init_irq = ixp4xx_init_irq,
  .timer    = &ixp4xx_timer,
  .boot_params  = 0x0100,
  .init_machine = cambria_init,
MACHINE_END

