/*
 *  linux/arch/arm/mach-cns3xxx/laguna.c
 *
 *  Copyright (c) 2008 Cavium Networks 
 *  Copyright (C) 2008 ARM Limited
 *  Copyright (C) 2000 Deep Blue Solutions Ltd
 * 
 *  This file is free software; you can redistribute it and/or modify 
 *  it under the terms of the GNU General Public License, Version 2, as 
 *  published by the Free Software Foundation. 
 *
 *  This file is distributed in the hope that it will be useful, 
 *  but AS-IS and WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, TITLE, or 
 *  NONINFRINGEMENT.  See the GNU General Public License for more details. 
 *
 *  You should have received a copy of the GNU General Public License 
 *  along with this file; if not, write to the Free Software 
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA or 
 *  visit http://www.gnu.org/licenses/. 
 *
 *  This file may also be available under a different license from Cavium. 
 *  Contact Cavium Networks for more information
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/if_ether.h>
#include <linux/socket.h>
#include <linux/netdevice.h>
#include <linux/vmalloc.h>

#include <linux/serial.h>
#include <linux/tty.h>
#include <linux/serial_8250.h>
#include <linux/slab.h>
#include <linux/spi/spi.h>
#include <linux/spi/flash.h>
#include <linux/i2c.h>
#include <linux/i2c/at24.h>
#include <linux/leds.h>
#include <linux/i2c/pca953x.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/physmap.h>
#include <linux/mmc/host.h>
#include <mach/board.h>
#include <mach/dmac.h>
#include <mach/lm.h>
#include <mach/sdhci.h>
#include <mach/pm.h>
#include <mach/misc.h>

#include <asm/types.h>
#include <asm/setup.h>
#include <asm/memory.h>
#include <mach/hardware.h>
#include <asm/mach-types.h>
#include <asm/irq.h>
#include <asm/mach/arch.h>
#include <linux/irq.h>
#include <linux/squashfs_fs.h>

#include "core.h"

struct laguna_board_info {
	char model[6];
	u32 config_bitmap;
	u32 config2_bitmap;
	u8 nor_flash_size;
	u8 spi_flash_size;
};

static struct laguna_board_info laguna_info __initdata;

/*
 * Cavium Networks ARM11 MPCore platform devices
 */

static struct mtd_partition laguna_norflash_partitions[] = {
	/* Bootloader */
	{
		.name = "bootloader",
		.offset = 0,
		.size = SZ_256K,
		.mask_flags = 0, /* force read-only */
	},
	/* Bootloader params */
	{
		.name = "params",
		.offset = SZ_256K,
		.size = SZ_128K,
		.mask_flags = 0,
	},
	/* linux */
	{
		.name = "linux",
		.offset = SZ_256K + SZ_128K,
		.size = SZ_2M,
		.mask_flags = 0,
	},
	/* Root FS */
	{
		.name = "rootfs",
		.offset = SZ_256K + SZ_128K + SZ_2M,
		.size = SZ_16M - SZ_256K - SZ_128K - SZ_2M,
		.mask_flags = 0,
	},
	/* ddwrt */
	{
		.name = "ddwrt",
		.offset = SZ_256K + SZ_128K + SZ_2M,
		.size = SZ_128K,
		.mask_flags = 0,
	},
	/* NVRAM */
	{
		.name = "nvram",
		.offset = SZ_256K + SZ_128K + SZ_2M,
		.size = SZ_128K,
		.mask_flags = 0,
	}
};

static struct physmap_flash_data laguna_norflash_data = {
	.width = 2,
	.parts = laguna_norflash_partitions,
	.nr_parts = ARRAY_SIZE(laguna_norflash_partitions),
};

static struct resource laguna_norflash_resource = {
	.start		= CNS3XXX_FLASH0_BASE,
	.end		= CNS3XXX_FLASH0_BASE + SZ_16M - 1,
	.flags		= IORESOURCE_MEM,
};

static struct platform_device laguna_norflash_device = {
	.name = "physmap-flash",
	.id = 0,
	.dev = {
		.platform_data = &laguna_norflash_data,
	},
	.num_resources = 1,
	.resource = &laguna_norflash_resource,
};

/* UART0 */
static struct resource laguna_uart_resources[] = {
  {
		.start = CNS3XXX_UART0_BASE,
		.end   = CNS3XXX_UART0_BASE + SZ_4K - 1,
    .flags    = IORESOURCE_MEM
  },{
		.start = CNS3XXX_UART1_BASE,
		.end   = CNS3XXX_UART1_BASE + SZ_4K - 1,
    .flags    = IORESOURCE_MEM
  },{
		.start = CNS3XXX_UART2_BASE,
		.end   = CNS3XXX_UART2_BASE + SZ_4K - 1,
    .flags    = IORESOURCE_MEM
  },
};

static struct plat_serial8250_port laguna_uart_data[] = {
	{
		.membase        = (char*) (CNS3XXX_UART0_BASE_VIRT),
		.mapbase        = (CNS3XXX_UART0_BASE),
		.irq            = IRQ_CNS3XXX_UART0,
		.iotype         = UPIO_MEM,
		.flags          = UPF_BOOT_AUTOCONF | UPF_FIXED_TYPE | UPF_NO_TXEN_TEST,
		.regshift       = 2,
		.uartclk        = 24000000,
		.type						= PORT_16550A,
	},{
		.membase        = (char*) (CNS3XXX_UART1_BASE_VIRT),
		.mapbase        = (CNS3XXX_UART1_BASE),
		.irq            = IRQ_CNS3XXX_UART1,
		.iotype         = UPIO_MEM,
		.flags          = UPF_BOOT_AUTOCONF | UPF_FIXED_TYPE | UPF_NO_TXEN_TEST,
		.regshift       = 2,
		.uartclk        = 24000000,
		.type						= PORT_16550A,
	},{
		.membase        = (char*) (CNS3XXX_UART2_BASE_VIRT),
		.mapbase        = (CNS3XXX_UART2_BASE),
		.irq            = IRQ_CNS3XXX_UART2,
		.iotype         = UPIO_MEM,
		.flags          = UPF_BOOT_AUTOCONF | UPF_FIXED_TYPE | UPF_NO_TXEN_TEST,
		.regshift       = 2,
		.uartclk        = 24000000,
		.type						= PORT_16550A,
	},
	{ },
};

static struct platform_device laguna_uart = {
  .name     = "serial8250",
  .id     = PLAT8250_DEV_PLATFORM,
  .dev.platform_data  = laguna_uart_data,
  .num_resources    = 3,
  .resource   = laguna_uart_resources
};

/* SDIO, MMC/SD */
static struct resource laguna_sdio_resource[] = {
	{
		.start = CNS3XXX_SDIO_BASE,
		.end   = CNS3XXX_SDIO_BASE + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},{
		.start = IRQ_CNS3XXX_SDIO,
		.end   = IRQ_CNS3XXX_SDIO,
		.flags = IORESOURCE_IRQ,
	},
};

struct cns3xxx_sdhci_platdata laguna_sdio_platform_data = {
	.max_width	= 4,
	.host_caps	= (MMC_CAP_4_BIT_DATA | MMC_CAP_MMC_HIGHSPEED | MMC_CAP_SD_HIGHSPEED),
};

static u64 laguna_device_sdhci_dmamask = 0xffffffffUL;

static struct platform_device laguna_sdio_device = {
	.name		= "cns3xxx-sdhci",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(laguna_sdio_resource),
	.resource	= laguna_sdio_resource,
	.dev		= {
		.dma_mask		= &laguna_device_sdhci_dmamask,
		.coherent_dma_mask	= 0xffffffffUL,
		.platform_data		= &laguna_sdio_platform_data,
	}
};

static struct pca953x_platform_data laguna_pca_data = {
	.gpio_base = 100,
};

static struct resource laguna_i2c_resource[] = {
	{
		.start		= CNS3XXX_SSP_BASE + 0x20,
		.end			= 0x7100003f,
		.flags		= IORESOURCE_MEM,
	},{
		.start		= IRQ_CNS3XXX_I2C,
		.flags		= IORESOURCE_IRQ,
	},
};

static struct platform_device laguna_i2c_controller_device = {
	.name		= "cns3xxx-i2c",
	.num_resources	= 2,
	.resource	= laguna_i2c_resource,
};

static struct resource laguna_usb_ehci_resource[] = {
	{
		.start = CNS3XXX_USB_BASE,
		.end   = CNS3XXX_USB_BASE + SZ_16M - 1,
		.flags = IORESOURCE_MEM,
	},{
		.start = IRQ_CNS3XXX_USB_EHCI,
		.flags = IORESOURCE_IRQ,
	},
};

static u64 laguna_usb_dma_mask = 0xffffffffULL;

static struct platform_device laguna_usb_ehci_device = {
	.name		= "cns3xxx-ehci",
	.num_resources	= ARRAY_SIZE(laguna_usb_ehci_resource),
	.resource	= laguna_usb_ehci_resource,
	.dev		= {
		.dma_mask		= &laguna_usb_dma_mask,
		.coherent_dma_mask	= 0xffffffffULL,
	},
};

static struct resource laguna_usb_ohci_resource[] = {
	{
		.start          = CNS3XXX_USB_OHCI_BASE,
		.end            = CNS3XXX_USB_OHCI_BASE + SZ_16M - 1,
		.flags          = IORESOURCE_MEM,
	},{
		.start          = IRQ_CNS3XXX_USB_OHCI,
		.flags          = IORESOURCE_IRQ,
	},
};

static u64 laguna_usb_ohci_dma_mask = 0xffffffffULL;
static struct platform_device laguna_usb_ohci_device = {
    .name = "cns3xxx-ohci",
    .dev                = {
        .dma_mask       = &laguna_usb_ohci_dma_mask,
        .coherent_dma_mask = 0xffffffffULL,
     },
    .num_resources = 2,
    .resource = laguna_usb_ohci_resource,
};

static struct platform_device laguna_usb_otg_device = {
	.name = "dwc_otg_platform_driver",
};

static struct resource laguna_ahci_resource[] = {
	{
		.start		= CNS3XXX_SATA2_BASE,
		.end		= CNS3XXX_SATA2_BASE + CNS3XXX_SATA2_SIZE - 1,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= IRQ_CNS3XXX_SATA,
		.end		= IRQ_CNS3XXX_SATA,
		.flags		= IORESOURCE_IRQ,
	},
};

static u64 laguna_device_ahci_dmamask = 0xffffffffUL;

static struct platform_device laguna_ahci = {
	.name		= "cns3xxx_ahci",
	.id		= -1,
	.dev		= {
		.dma_mask		= &laguna_device_ahci_dmamask,
		.coherent_dma_mask	= 0xffffffffUL,
	},
	.resource	= laguna_ahci_resource,
	.num_resources	= ARRAY_SIZE(laguna_ahci_resource),
};

/* SPI Flash */
static struct mtd_partition laguna_spiflash_partitions[] = {
	/* Bootloader */
	{
		.name		= "bootloader",
		.offset		= 0,
		.size		= SZ_256K,
	},
	/* Bootloader params */
	{
		.name		= "params",
		.offset		= SZ_256K,
		.size		= SZ_256K,
	},
	/* linux */
	{
		.name = "linux",
		.offset = SZ_512K,
		.size = SZ_2M,
		.mask_flags = 0,
	},
	/* FileSystem */
	{
		.name		= "rootfs",
		.offset		= SZ_512K + SZ_2M ,
		.size		= SZ_16M - SZ_512K - SZ_2M,
	},
	/* ddwrt */
	{
		.name = "ddwrt",
		.offset = SZ_512K + SZ_2M,
		.size = SZ_128K,
		.mask_flags = 0,
	},
	/* NVRAM */
	{
		.name = "nvram",
		.offset = SZ_512K + SZ_2M,
		.size = SZ_128K,
		.mask_flags = 0,
	}
};

static struct flash_platform_data laguna_spiflash_data = {
	.parts		= laguna_spiflash_partitions,
	.nr_parts	= ARRAY_SIZE(laguna_spiflash_partitions),
};

static struct spi_board_info __initdata laguna_spi_devices[] = {
	{
		.modalias		= "m25p80",
		.platform_data		=  &laguna_spiflash_data,
		.max_speed_hz		= 50000000,
		.bus_num		= 1,
		.chip_select		= 0,
	},
};

static struct platform_device laguna_spi_controller_device = {
	.name		= "cns3xxx_spi",
};

static struct gpio_led laguna_gpio_leds[] = {
	{
		.name = "user1", /* Green Led */
		.gpio = 115,
		.active_low = 1,
	},
	{
		.name = "user2", /* Red Led */
		.gpio = 114,
		.active_low = 1,
	},
};

static struct gpio_led_platform_data laguna_gpio_leds_data = {
	.num_leds = 2,
	.leds = laguna_gpio_leds,
};

static struct platform_device laguna_gpio_leds_device = {
	.name = "leds-gpio",
	.id = -1,
	.dev.platform_data = &laguna_gpio_leds_data,
};

static struct eth_plat_info laguna_net_data = {
	.ports = 3,	// Bring Up both Eth port by Default 
};

static struct platform_device laguna_net_device = {
	.name = "cns3xxx-net",
	.id = -1,
	.dev.platform_data = &laguna_net_data,
};

static struct memory_accessor *at24_mem_acc;

static void at24_setup(struct memory_accessor *mem_acc, void *context)
{
	char buf[8];

	at24_mem_acc = mem_acc;

  /* Read MAC addresses */
	if (at24_mem_acc->read(at24_mem_acc, buf, 0x100, 6) == 6)
		memcpy(&laguna_net_data.eth0_hwaddr, buf, ETH_ALEN);
	if (at24_mem_acc->read(at24_mem_acc, buf, 0x106, 6) == 6)
		memcpy(&laguna_net_data.eth1_hwaddr, buf, ETH_ALEN);
	if (at24_mem_acc->read(at24_mem_acc, buf, 0x10C, 6) == 6)
		memcpy(&laguna_net_data.eth2_hwaddr, buf, ETH_ALEN);
	if (at24_mem_acc->read(at24_mem_acc, buf, 0x112, 6) == 6)
		memcpy(&laguna_net_data.cpu_hwaddr, buf, ETH_ALEN);

	/* Read out Model Information */
	if (at24_mem_acc->read(at24_mem_acc, buf, 0x130, 16) == 16)
		memcpy(&laguna_info.model, buf, 16);
	if (at24_mem_acc->read(at24_mem_acc, buf, 0x140, 1) == 1)
		memcpy(&laguna_info.nor_flash_size, buf, 1);
	if (at24_mem_acc->read(at24_mem_acc, buf, 0x141, 1) == 1)
		memcpy(&laguna_info.spi_flash_size, buf, 1);
	if (at24_mem_acc->read(at24_mem_acc, buf, 0x142, 4) == 4)
		memcpy(&laguna_info.config_bitmap, buf, 8);
	if (at24_mem_acc->read(at24_mem_acc, buf, 0x146, 4) == 4)
		memcpy(&laguna_info.config2_bitmap, buf, 8);
};

static struct at24_platform_data laguna_eeprom_info = {
	.byte_len = 1024,
	.page_size = 16,
	.flags = AT24_FLAG_READONLY,
	.setup = at24_setup,
};

static struct i2c_board_info __initdata laguna_i2c_devices[] = {
	{
		I2C_BOARD_INFO("pca9555", 0x23),
		.platform_data = &laguna_pca_data,
	},
	{
		I2C_BOARD_INFO("gsp", 0x29),
	},
	{
		I2C_BOARD_INFO ("24c08",0x50),
		.platform_data = &laguna_eeprom_info,
	},
	{
		I2C_BOARD_INFO("ds1672", 0x68),
	},
};

static void __init laguna_init(void)
{
	cns3xxx_sys_init();

	platform_device_register(&laguna_i2c_controller_device);

	i2c_register_board_info(0, laguna_i2c_devices, ARRAY_SIZE(laguna_i2c_devices));

	pm_power_off = cns3xxx_power_off;
}
#define LE8221_SPI_CS		1
#define SI3226_SPI_CS		1

#define CNS3XXX_SPI_INTERRUPT
#undef CNS3XXX_SPI_INTERRUPT	/* Interrupt is not supported for D2 and SEN */

/*
 * define access macros
 */
#define SPI_MEM_MAP_VALUE(reg_offset)		(*((u32 volatile *)(CNS3XXX_SSP_BASE_VIRT + reg_offset)))

#define SPI_CONFIGURATION_REG			SPI_MEM_MAP_VALUE(0x40)
#define SPI_SERVICE_STATUS_REG			SPI_MEM_MAP_VALUE(0x44)
#define SPI_BIT_RATE_CONTROL_REG		SPI_MEM_MAP_VALUE(0x48)
#define SPI_TRANSMIT_CONTROL_REG		SPI_MEM_MAP_VALUE(0x4C)
#define SPI_TRANSMIT_BUFFER_REG			SPI_MEM_MAP_VALUE(0x50)
#define SPI_RECEIVE_CONTROL_REG			SPI_MEM_MAP_VALUE(0x54)
#define SPI_RECEIVE_BUFFER_REG			SPI_MEM_MAP_VALUE(0x58)
#define SPI_FIFO_TRANSMIT_CONFIG_REG		SPI_MEM_MAP_VALUE(0x5C)
#define SPI_FIFO_TRANSMIT_CONTROL_REG		SPI_MEM_MAP_VALUE(0x60)
#define SPI_FIFO_RECEIVE_CONFIG_REG		SPI_MEM_MAP_VALUE(0x64)
#define SPI_INTERRUPT_STATUS_REG		SPI_MEM_MAP_VALUE(0x68)
#define SPI_INTERRUPT_ENABLE_REG		SPI_MEM_MAP_VALUE(0x6C)

#define SPI_TRANSMIT_BUFFER_REG_ADDR		(CNS3XXX_SSP_BASE +0x50)
#define SPI_RECEIVE_BUFFER_REG_ADDR		(CNS3XXX_SSP_BASE +0x58)


static int __init laguna_model_setup(void)
{
	if (!machine_is_gw2388())
		return 0;

	printk("Running on Gateworks Laguna %s\n", laguna_info.model);

	if (strncmp(laguna_info.model, "GW", 2) == 0) {
		if (laguna_info.config_bitmap & ETH0_LOAD)
			laguna_net_data.ports |= BIT(0);
		if (strncmp(laguna_info.model, "GW2388", 6) == 0)
		{
		if (laguna_info.config_bitmap & ETH1_LOAD)
			laguna_net_data.ports |= BIT(1);
		if (laguna_info.config_bitmap & ETH2_LOAD)
			laguna_net_data.ports |= BIT(2);
		}
		if (laguna_net_data.ports)
			platform_device_register(&laguna_net_device);
		
		if (laguna_info.config_bitmap & (SATA0_LOAD | SATA1_LOAD))
			platform_device_register(&laguna_ahci);

		if (laguna_info.config_bitmap & (PCIe0_LOAD))
			cns3xxx_pcie_init(1);

		if (laguna_info.config_bitmap & (PCIe1_LOAD))
			cns3xxx_pcie_init(2);

		if (laguna_info.config_bitmap & (USB0_LOAD))
			platform_device_register(&laguna_usb_otg_device);

		if (laguna_info.config_bitmap & (USB1_LOAD)) {
			platform_device_register(&laguna_usb_ehci_device);
			platform_device_register(&laguna_usb_ohci_device);
		}

		if (laguna_info.config_bitmap & (SD_LOAD))
			platform_device_register(&laguna_sdio_device);

		if (laguna_info.config_bitmap & (UART0_LOAD))
			laguna_uart.num_resources = 1;
		if (laguna_info.config_bitmap & (UART1_LOAD))
			laguna_uart.num_resources = 2;
		if (laguna_info.config_bitmap & (UART2_LOAD))
			laguna_uart.num_resources = 3;
		platform_device_register(&laguna_uart);
		printk(KERN_EMERG "notflash size %d\n",laguna_info.nor_flash_size);
		if ((laguna_info.config2_bitmap & (NOR_FLASH_LOAD)) && strncmp(laguna_info.model, "GW2388", 6) == 0) {
			printk(KERN_EMERG "detecting NOR FLASH\n");
//			if (laguna_info.nor_flash_size < 1 || laguna_info.nor_flash_size > 5)
//			    laguna_info.nor_flash_size = 2; //guess default for wrong config 
			laguna_norflash_partitions[2].size = SZ_16M - SZ_256K - SZ_128K  - SZ_128K;
			laguna_norflash_resource.end = CNS3XXX_FLASH0_BASE + SZ_16M - 1;
			switch (laguna_info.nor_flash_size) {
				case 1:
					laguna_norflash_partitions[2].size = SZ_8M - SZ_256K - SZ_128K  - SZ_128K;
					laguna_norflash_resource.end = CNS3XXX_FLASH0_BASE + SZ_8M - 1;
				break;
				case 2:
					laguna_norflash_partitions[2].size = SZ_16M - SZ_256K - SZ_128K  - SZ_128K;
					laguna_norflash_resource.end = CNS3XXX_FLASH0_BASE + SZ_16M - 1;
				break;
				case 3:
					laguna_norflash_partitions[2].size = SZ_32M - SZ_256K - SZ_128K  - SZ_128K;
					laguna_norflash_resource.end = CNS3XXX_FLASH0_BASE + SZ_32M - 1;
				break;
				case 4:
					laguna_norflash_partitions[2].size = SZ_64M - SZ_256K - SZ_128K  - SZ_128K;
					laguna_norflash_resource.end = CNS3XXX_FLASH0_BASE + SZ_64M - 1;
				break;
				case 5:
					laguna_norflash_partitions[2].size = SZ_128M - SZ_256K - SZ_128K  - SZ_128K;
					laguna_norflash_resource.end = CNS3XXX_FLASH0_BASE + SZ_128M - 1;
				break;
			}
			unsigned int flashsize = laguna_norflash_partitions[2].size + SZ_256K + SZ_128K + SZ_128K;
			unsigned char *buf = (unsigned char *)ioremap(laguna_norflash_resource.start,laguna_norflash_resource.end - laguna_norflash_resource.start + 1);
			unsigned int offset=0;
			int tmplen;
			int filesyssize=0;
			int erasesize=0x20000;
			while(offset<laguna_norflash_resource.end)
			{
			if (*((__u32 *) buf) == SQUASHFS_MAGIC || *((__u16 *) buf) == 0x1985) 
			{
			printk(KERN_EMERG "found squashfs\n");
	    		struct squashfs_super_block *sb = (struct squashfs_super_block *) buf;
			filesyssize = sb->bytes_used;
			tmplen = offset + filesyssize;
			tmplen +=  (erasesize - 1);
			tmplen &= ~(erasesize - 1);
			filesyssize = tmplen - offset;
			laguna_norflash_partitions[3].offset = offset;
			laguna_norflash_partitions[3].size = filesyssize;
			laguna_norflash_partitions[4].offset = offset + filesyssize;
			laguna_norflash_partitions[4].size = (flashsize- SZ_128K) - laguna_norflash_partitions[4].offset;
			laguna_norflash_partitions[5].offset = (flashsize - SZ_128K);
			laguna_norflash_partitions[5].size = SZ_128K;
			break;
			}
			buf+=0x1000;
			offset+=0x1000;
			}

			platform_device_register(&laguna_norflash_device);
		}

		if ((laguna_info.config2_bitmap & (SPI_FLASH_LOAD)) && strncmp(laguna_info.model, "GW2380", 6) == 0) {
			printk(KERN_EMERG "detecting SPI FLASH\n");
			SPI_CONFIGURATION_REG = 0x40000000;
			HAL_MISC_ENABLE_SPI_SERIAL_FLASH_BANK_ACCESS();
			laguna_spiflash_partitions[2].size		= SZ_16M - SZ_512K;
#if 0
			switch (laguna_info.spi_flash_size) {
				case 1:
					laguna_spiflash_partitions[2].size		= SZ_4M - SZ_512K;
				break;
				case 2:
					laguna_spiflash_partitions[2].size		= SZ_8M - SZ_512K;
				break;
				case 3:
					laguna_spiflash_partitions[2].size		= SZ_16M - SZ_512K;
				break;
				case 4:
					laguna_spiflash_partitions[2].size		= SZ_32M - SZ_512K;
				break;
				case 5:
					laguna_spiflash_partitions[2].size		= SZ_64M - SZ_512K;
				break;
			}
#endif
			unsigned int flashsize = laguna_spiflash_partitions[2].size + SZ_512K;
			unsigned char *buf = (unsigned char *)ioremap(CNS3XXX_SPI_FLASH_BASE,SZ_16M-1);
		//	unsigned char *buf = (unsigned char *)CNS3XXX_SPI_FLASH_BASE;
			unsigned int offset=0;
			int tmplen;
			int filesyssize=0;
			int erasesize=0x40000;
			while(offset<SZ_8M)
			{
			if (*((__u32 *) buf) == SQUASHFS_MAGIC || *((__u16 *) buf) == 0x1985) 
			{
			struct squashfs_super_block *sb;
			char *block = vmalloc(0x40000);
			memcpy(block,buf,0x40000);
			sb = (struct squashfs_super_block*)block;
			printk(KERN_EMERG "found squashfs @0x%08X magic=0x%08X\n",offset,*((__u32 *) buf));
			filesyssize = sb->bytes_used;
			vfree(block);
			printk(KERN_EMERG "filesystem size 0x%08X\n",filesyssize);
			tmplen = offset + filesyssize;
			tmplen +=  (erasesize - 1);
			tmplen &= ~(erasesize - 1);
			filesyssize = tmplen - offset;
			printk(KERN_EMERG "filesystem size 0x%08X\n",filesyssize);
			laguna_spiflash_partitions[3].offset = offset;
			laguna_spiflash_partitions[3].size = filesyssize;
			laguna_spiflash_partitions[4].offset = offset + filesyssize;
			laguna_spiflash_partitions[4].size = (flashsize- SZ_256K) - laguna_spiflash_partitions[4].offset;
			laguna_spiflash_partitions[5].offset = (flashsize - SZ_256K);
			laguna_spiflash_partitions[5].size = SZ_256K;
			break;
			}
			buf+=0x1000;
			offset+=0x1000;
			}
			HAL_MISC_DISABLE_SPI_SERIAL_FLASH_BANK_ACCESS();

		if (strncmp(laguna_info.model, "GW2380", 6) == 0)
			spi_register_board_info(laguna_spi_devices, ARRAY_SIZE(laguna_spi_devices));

		}

		if (laguna_info.config_bitmap & (SPI0_LOAD | SPI1_LOAD))
		{
			platform_device_register(&laguna_spi_controller_device);
		}

		/*
		 *	Do any model specific setup not known by the bitmap by matching
		 *  the first 6 characters of the model name
		 */

		if (strncmp(laguna_info.model, "GW2388", 6) == 0)
		{
			platform_device_register(&laguna_gpio_leds_device);
		}
	} else {
		// Do some defaults here, not sure what yet
	}

	return 0;
}
late_initcall(laguna_model_setup);

MACHINE_START(GW2388, "Gateworks Laguna Platform")
	.phys_io	= CNS3XXX_UART0_BASE,
	.io_pg_offst	= (CNS3XXX_UART0_BASE_VIRT >> 18) & 0xfffc,
	.boot_params	= 0x00000100,
	.map_io		= cns3xxx_map_io,
	.init_irq	= cns3xxx_init_irq,
	.timer		= &cns3xxx_timer,
	.init_machine	= laguna_init,
MACHINE_END
