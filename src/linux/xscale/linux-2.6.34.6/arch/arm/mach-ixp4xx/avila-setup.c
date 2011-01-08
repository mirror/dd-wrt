/*
 * arch/arm/mach-ixp4xx/avila-setup.c
 *
 * Gateworks Avila board-setup
 *
 * Author: Michael-Luke Jones <mlj28@cam.ac.uk>
 *
 * Based on ixdp-setup.c
 * Copyright (C) 2003-2005 MontaVista Software, Inc.
 *
 * Author: Deepak Saxena <dsaxena@plexity.net>
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/serial.h>
#include <linux/tty.h>
#include <linux/serial_8250.h>
#include <linux/slab.h>
#include <linux/delay.h>

#include <asm/types.h>
#include <asm/setup.h>
#include <asm/memory.h>
#include <mach/hardware.h>
#include <asm/mach-types.h>
#include <asm/irq.h>
#include <asm/mach/arch.h>
#include <asm/mach/flash.h>

static struct flash_platform_data avila_flash_data = {
	.map_name	= "cfi_probe",
	.width		= 2,
};

static struct resource avila_flash_resource = {
	.flags		= IORESOURCE_MEM,
};

static struct platform_device avila_flash = {
	.name		= "IXP4XX-Flash",
	.id		= 0,
	.dev		= {
		.platform_data = &avila_flash_data,
	},
	.num_resources	= 1,
	.resource	= &avila_flash_resource,
};

#define IXDP425_KSSPI_SELECT	4
#define IXDP425_KSSPI_TXD	3
#define IXDP425_KSSPI_CLOCK	2
#define IXDP425_KSSPI_RXD	0

#define IXDP425_GW2355_KSSPI_SELECT	3
#define IXDP425_GW2355_KSSPI_TXD	2
#define IXDP425_GW2355_KSSPI_CLOCK	1
#define IXDP425_GW2355_KSSPI_RXD	0


static struct ixp4xx_spi_pins ixdp425_spi_gpio_pins = {
	.spis_pin       = IXDP425_KSSPI_SELECT,
	.spic_pin       = IXDP425_KSSPI_CLOCK,
	.spid_pin       = IXDP425_KSSPI_TXD,
	.spiq_pin       = IXDP425_KSSPI_RXD
};

static struct platform_device ixdp425_spi_controller = {
    .name               = "IXP4XX-SPI",
	.id                 = 0,
	.dev                = {
		.platform_data  = &ixdp425_spi_gpio_pins,
	},
	.num_resources      = 0
};


static struct ixp4xx_spi_pins ixdp425_gw2355_spi_gpio_pins = {
	.spis_pin       = IXDP425_GW2355_KSSPI_SELECT,
	.spic_pin       = IXDP425_GW2355_KSSPI_CLOCK,
	.spid_pin       = IXDP425_GW2355_KSSPI_TXD,
	.spiq_pin       = IXDP425_GW2355_KSSPI_RXD
};

static struct platform_device ixdp425_gw2355_spi_controller = {
    .name               = "IXP4XX-SPI-GW2355",
	.id                 = 0,
	.dev                = {
		.platform_data  = &ixdp425_gw2355_spi_gpio_pins,
	},
	.num_resources      = 0
};


static struct ixp4xx_i2c_pins avila_i2c_gpio_pins = {
	.sda_pin	= AVILA_SDA_PIN,
	.scl_pin	= AVILA_SCL_PIN,
};

static struct platform_device avila_i2c_controller = {
	.name		= "IXP4XX-I2C",
	.id		= 0,
	.dev		= {
		.platform_data = &avila_i2c_gpio_pins,
	},
	.num_resources	= 0
};

static struct resource avila_uart_resources[] = {
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

static struct plat_serial8250_port avila_uart_data[] = {
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

static struct platform_device avila_uart = {
	.name			= "serial8250",
	.id			= PLAT8250_DEV_PLATFORM,
	.dev.platform_data	= avila_uart_data,
	.num_resources		= 2,
	.resource		= avila_uart_resources
};



static struct platform_device *avila_devices[] __initdata = {
	&avila_i2c_controller,
	&avila_flash,
	&avila_uart,
	&ixdp425_spi_controller,
	&ixdp425_gw2355_spi_controller
};





#define GPIO_EEPROM_SCL 6
#define GPIO_EEPROM_SDA 7
#define CLK_LO()      gpio_line_set(GPIO_EEPROM_SCL, IXP4XX_GPIO_LOW);
#define CLK_HI()      gpio_line_set(GPIO_EEPROM_SCL, IXP4XX_GPIO_HIGH);
#define DATA_LO()     gpio_line_set(GPIO_EEPROM_SDA, IXP4XX_GPIO_LOW);
#define DATA_HI()     gpio_line_set(GPIO_EEPROM_SDA, IXP4XX_GPIO_HIGH);
#define hal_delay_us(a) udelay(a)
#define HAL_GPIO_OUTPUT_DISABLE(gpio) gpio_line_config(gpio, IXP4XX_GPIO_IN);
#define HAL_GPIO_OUTPUT_ENABLE(gpio) gpio_line_config(gpio, IXP4XX_GPIO_OUT);
#define HAL_GPIO_OUTPUT_SET(gpio) gpio_line_set(gpio, IXP4XX_GPIO_HIGH);
typedef unsigned char cyg_uint8;

// returns non-zero if ACK bit seen
static int __init
eeprom_start(cyg_uint8 b)
{
    int i;

    CLK_HI();
    hal_delay_us(5);
    DATA_LO();
    hal_delay_us(5);
    CLK_LO();

    for (i = 7; i >= 0; i--) {
	if (b & (1 << i))
	{
	    DATA_HI();
	}
	else{
	
	    DATA_LO();
	}
	hal_delay_us(5);
	CLK_HI();
	hal_delay_us(5);
	CLK_LO();
    }
    hal_delay_us(5);
    HAL_GPIO_OUTPUT_DISABLE(GPIO_EEPROM_SDA);
    CLK_HI();
    hal_delay_us(5);
    i = (*IXP4XX_GPIO_GPINR & (1 << GPIO_EEPROM_SDA)) ? 0 : 1;
    CLK_LO();
    hal_delay_us(5);
    HAL_GPIO_OUTPUT_ENABLE(GPIO_EEPROM_SDA);

    return i;
}


static void __init
eeprom_stop(void)
{
    hal_delay_us(5);
    DATA_LO();
    hal_delay_us(5);
    CLK_HI();
    hal_delay_us(5);
    DATA_HI();
    hal_delay_us(5);
    CLK_LO();
    hal_delay_us(5);
    CLK_HI();
    hal_delay_us(5);
}


static int __init
eeprom_putb(cyg_uint8 b)
{
    int i;

    for (i = 7; i >= 0; i--) {
	if (b & (1 << i)) {
	    DATA_HI();
	}
	else{
	    DATA_LO();
	}
	CLK_HI();
	hal_delay_us(5);
	CLK_LO();
	hal_delay_us(5);
    }
    HAL_GPIO_OUTPUT_DISABLE(GPIO_EEPROM_SDA);
    CLK_HI();
    hal_delay_us(5);
    i = (*IXP4XX_GPIO_GPINR & (1 << GPIO_EEPROM_SDA)) ? 0 : 1;
    CLK_LO();
    hal_delay_us(5);

    DATA_HI();
    HAL_GPIO_OUTPUT_ENABLE(GPIO_EEPROM_SDA);

    return i;
}


static cyg_uint8 __init
eeprom_getb(int more)
{
    int i;
    cyg_uint8 b = 0;

    HAL_GPIO_OUTPUT_DISABLE(GPIO_EEPROM_SDA);
    hal_delay_us(5);

    for (i = 7; i >= 0; i--) {
	b <<= 1;
	if (*IXP4XX_GPIO_GPINR & (1 << GPIO_EEPROM_SDA))
	    b |= 1;
	CLK_HI();
	hal_delay_us(5);
	CLK_LO();
	hal_delay_us(5);
    }
    HAL_GPIO_OUTPUT_ENABLE(GPIO_EEPROM_SDA);
    if (more)
    {
	DATA_LO();
    }
    else{
	DATA_HI();
    }
    hal_delay_us(5);
    CLK_HI();
    hal_delay_us(5);
    CLK_LO();
    hal_delay_us(5);

    return b;
}


int
eeprom_read(int addr, cyg_uint8 *buf, int nbytes)
{
    cyg_uint8 start_byte;
    int i;

    start_byte = 0xA0;  // write

    if (addr & (1 << 8))
	start_byte |= 2;

    
    for (i = 0; i < 10; i++)
	if (eeprom_start(start_byte))
	    break;

    if (i == 10) {
	printk(KERN_WARNING "eeprom_read: Can't get write start ACK\n");
	return 0;
    }

    if (!eeprom_putb(addr & 0xff)) {
	printk(KERN_WARNING "eeprom_read: Can't get address ACK\n");
	return 0;
    }

    start_byte |= 1; // READ command
    if (!eeprom_start(start_byte)) {
	printk(KERN_WARNING "eeprom_read: Can't get read start ACK\n");
	return 0;
    }

    for (i = 0; i < (nbytes - 1); i++)
	*buf++ = eeprom_getb(1);

    *buf++ = eeprom_getb(0);
    hal_delay_us(5);
    eeprom_stop();

    return nbytes;
}






static void __init avila_init(void)
{
	ixp4xx_sys_init();
	char model[16];
	memset(model,0,16);

    HAL_GPIO_OUTPUT_SET(GPIO_EEPROM_SCL);
    HAL_GPIO_OUTPUT_ENABLE(GPIO_EEPROM_SCL);

    HAL_GPIO_OUTPUT_SET(GPIO_EEPROM_SDA);
    HAL_GPIO_OUTPUT_ENABLE(GPIO_EEPROM_SDA);


	eeprom_read(0x120, model, 16);
	printk(KERN_INFO "Gateworks Model %s detected!\n",model);
	avila_flash_resource.start = IXP4XX_EXP_BUS_BASE(0);
	if (!strncmp(model,"GW2369",6))
	{
	/* required for 32 mb flash access. do not enable cf driver here, this will collide with each other */
	avila_flash_resource.end = IXP4XX_EXP_BUS_BASE(0) + SZ_32M - 1;
	}else
	{
	avila_flash_resource.end = IXP4XX_EXP_BUS_BASE(0) + SZ_16M - 1;	
	}
	platform_add_devices(avila_devices, ARRAY_SIZE(avila_devices));

}

MACHINE_START(AVILA, "Gateworks Avila Network Platform")
	/* Maintainer: Deepak Saxena <dsaxena@plexity.net> */
	.phys_io	= IXP4XX_PERIPHERAL_BASE_PHYS,
	.io_pg_offst	= ((IXP4XX_PERIPHERAL_BASE_VIRT) >> 18) & 0xfffc,
	.map_io		= ixp4xx_map_io,
	.init_irq	= ixp4xx_init_irq,
	.timer		= &ixp4xx_timer,
	.boot_params	= 0x0100,
	.init_machine	= avila_init,
MACHINE_END

MACHINE_START(WAVESAT_AVILA, "Gateworks/Wavesat Avila Network Platform")
	/* Maintainer: Deepak Saxena <dsaxena@plexity.net> */
	.phys_io	= IXP4XX_PERIPHERAL_BASE_PHYS,
	.io_pg_offst	= ((IXP4XX_PERIPHERAL_BASE_VIRT) >> 18) & 0xfffc,
	.map_io		= ixp4xx_map_io,
	.init_irq	= ixp4xx_init_irq,
	.timer		= &ixp4xx_timer,
	.boot_params	= 0x0100,
	.init_machine	= avila_init,
MACHINE_END


 /*
  * Loft is functionally equivalent to Avila except that it has a
  * different number for the maximum PCI devices.  The MACHINE
  * structure below is identical to Avila except for the comment.
  */
#ifdef CONFIG_MACH_LOFT
MACHINE_START(LOFT, "Giant Shoulder Inc Loft board")
	/* Maintainer: Tom Billman <kernel@giantshoulderinc.com> */
	.phys_io	= IXP4XX_PERIPHERAL_BASE_PHYS,
	.io_pg_offst	= ((IXP4XX_PERIPHERAL_BASE_VIRT) >> 18) & 0xfffc,
	.map_io		= ixp4xx_map_io,
	.init_irq	= ixp4xx_init_irq,
	.timer		= &ixp4xx_timer,
	.boot_params	= 0x0100,
	.init_machine	= avila_init,
MACHINE_END
#endif

