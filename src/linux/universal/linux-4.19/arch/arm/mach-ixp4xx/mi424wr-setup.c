/*
 * arch/arm/mach-ixp4xx/mi424wr-setup.c
 *
 * Actiontec MI424-WR board setup
 * Copyright (c) 2008 Jose Vasconcellos
 *
 * Based on Gemtek GTWX5715 by
 * Copyright (C) 2004 George T. Joseph
 * Derived from Coyote
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include <linux/init.h>
#include <linux/device.h>
#include <linux/serial.h>
#include <linux/serial_8250.h>
#include <linux/types.h>
#include <linux/memory.h>
#include <linux/leds.h>
#include <linux/spi/spi_gpio_old.h>

#include <asm/setup.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/flash.h>

/*
 * GPIO 2,3,4 and 9 are hard wired to the Micrel/Kendin KS8995M Switch
 * and operate as an SPI type interface.  The details of the interface
 * are available on Kendin/Micrel's web site.
 */

#define MI424WR_KSSPI_SELECT		9
#define MI424WR_KSSPI_TXD		4
#define MI424WR_KSSPI_CLOCK		2
#define MI424WR_KSSPI_RXD		3


static int mi424wr_spi_boardinfo_setup(struct spi_board_info *bi,
		struct spi_master *master, void *data)
{

	strlcpy(bi->modalias, "spi-ks8995", sizeof(bi->modalias));

	bi->max_speed_hz = 5000000 /* Hz */;
	bi->bus_num = master->bus_num;
	bi->mode = SPI_MODE_0;

	return 0;
}

static struct spi_gpio_platform_data mi424wr_spi_bus_data = {
	.pin_cs			= MI424WR_KSSPI_SELECT,
	.pin_clk		= MI424WR_KSSPI_CLOCK,
	.pin_miso		= MI424WR_KSSPI_RXD,
	.pin_mosi		= MI424WR_KSSPI_TXD,
	.cs_activelow		= 1,
	.no_spi_delay		= 1,
	.boardinfo_setup	= mi424wr_spi_boardinfo_setup,
};

static struct platform_device mi424wr_spi_bus = {
	.name		= "spi-ixp4xx",
	.id		= 0,
	.dev.platform_data = &mi424wr_spi_bus_data,
};

/*
 * The "reset" button is wired to GPIO 10.
 * The GPIO is brought "low" when the button is pushed.
 */

#define MI424WR_BUTTON_GPIO	10
#define MI424WR_BUTTON_IRQ	IRQ_IXP4XX_GPIO10

#define MI424WR_MOCA_WAN_LED	11

/* Latch on CS1 - taken from Actiontec's 2.4 source code
 * 
 * default latch value
 * 0  - power alarm led (red)           0 (off)
 * 1  - power led (green)               0 (off)
 * 2  - wireless led    (green)         1 (off)
 * 3  - no internet led (red)           0 (off)
 * 4  - internet ok led (green)         0 (off)
 * 5  - moca LAN                        0 (off)
 * 6  - WAN alarm led (red)		0 (off)
 * 7  - PCI reset                       1 (not reset)
 * 8  - IP phone 1 led (green)          1 (off)
 * 9  - IP phone 2 led (green)          1 (off)
 * 10 - VOIP ready led (green)          1 (off)
 * 11 - PSTN relay 1 control            0 (PSTN)
 * 12 - PSTN relay 1 control            0 (PSTN)
 * 13 - N/A
 * 14 - N/A
 * 15 - N/A
 */

#define MI424WR_LATCH_MASK              0x04
#define MI424WR_LATCH_DEFAULT           0x1f86

#define MI424WR_LATCH_ALARM_LED         0x00
#define MI424WR_LATCH_POWER_LED         0x01
#define MI424WR_LATCH_WIRELESS_LED      0x02
#define MI424WR_LATCH_INET_DOWN_LED     0x03
#define MI424WR_LATCH_INET_OK_LED       0x04
#define MI424WR_LATCH_MOCA_LAN_LED      0x05
#define MI424WR_LATCH_WAN_ALARM_LED     0x06
#define MI424WR_LATCH_PCI_RESET         0x07
#define MI424WR_LATCH_PHONE1_LED        0x08
#define MI424WR_LATCH_PHONE2_LED        0x09
#define MI424WR_LATCH_VOIP_LED          0x10
#define MI424WR_LATCH_PSTN_RELAY1       0x11
#define MI424WR_LATCH_PSTN_RELAY2       0x12

/* initialize CS1 to default timings, Intel style, 16-bit bus */
#define MI424WR_CS1_CONFIG	0x80000002

/* Define both UARTs but they are not easily accessible.
 */

static struct resource mi424wr_uart_resources[] = {
	{
		.start	= IXP4XX_UART1_BASE_PHYS,
		.end	= IXP4XX_UART1_BASE_PHYS + 0x0fff,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= IXP4XX_UART2_BASE_PHYS,
		.end	= IXP4XX_UART2_BASE_PHYS + 0x0fff,
		.flags	= IORESOURCE_MEM,
	}
};


static struct plat_serial8250_port mi424wr_uart_platform_data[] = {
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

static struct platform_device mi424wr_uart_device = {
	.name		= "serial8250",
	.id		= PLAT8250_DEV_PLATFORM,
	.dev.platform_data	= mi424wr_uart_platform_data,
	.num_resources	= ARRAY_SIZE(mi424wr_uart_resources),
	.resource	= mi424wr_uart_resources,
};

static struct flash_platform_data mi424wr_flash_data = {
	.map_name	= "cfi_probe",
	.width		= 2,
};

static struct resource mi424wr_flash_resource = {
	.flags		= IORESOURCE_MEM,
};

static struct platform_device mi424wr_flash = {
	.name		= "IXP4XX-Flash",
	.id		= 0,
	.dev.platform_data = &mi424wr_flash_data,
	.num_resources	= 1,
	.resource	= &mi424wr_flash_resource,
};




static uint16_t latch_value = MI424WR_LATCH_DEFAULT;
static uint16_t __iomem *iobase;

static void mi424wr_latch_set_led(u8 bit, enum led_brightness value)
{

	if (((MI424WR_LATCH_MASK >> bit) & 1) ^ (value == LED_OFF))
		latch_value &= ~(0x1 << bit);
	else
		latch_value |= (0x1 << bit);

	__raw_writew(latch_value, iobase);

}



static struct platform_device *mi424wr_devices[] __initdata = {
	&mi424wr_uart_device,
	&mi424wr_flash,
	&mi424wr_spi_bus,
};

static void __init mi424wr_init(void)
{
	ixp4xx_sys_init();

	mi424wr_flash_resource.start = IXP4XX_EXP_BUS_BASE(0);
	mi424wr_flash_resource.end = IXP4XX_EXP_BUS_BASE(0) + SZ_8M - 1;

	*IXP4XX_EXP_CS0 |= IXP4XX_FLASH_WRITABLE;
	*IXP4XX_EXP_CS1 = MI424WR_CS1_CONFIG;

	/* configure button as input
	 */
	gpio_line_config(MI424WR_BUTTON_GPIO, IXP4XX_GPIO_IN);

	/* Initialize LEDs and enables PCI bus.
	 */
	iobase = ioremap_nocache(IXP4XX_EXP_BUS_BASE(1), 0x1000);
	__raw_writew(latch_value, iobase);

	platform_add_devices(mi424wr_devices, ARRAY_SIZE(mi424wr_devices));
}


MACHINE_START(MI424WR, "Actiontec MI424WR")
	/* Maintainer: Jose Vasconcellos */
	.map_io		= ixp4xx_map_io,
	.init_early	= ixp4xx_init_early,
	.init_irq	= ixp4xx_init_irq,
	.init_time	= ixp4xx_timer_init,
	.atag_offset	= 0x0100,
	.init_machine	= mi424wr_init,
#if defined(CONFIG_PCI)
	.dma_zone_size	= SZ_64M,
#endif
	.restart	= ixp4xx_restart,
MACHINE_END

