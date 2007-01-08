/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2003 Atheros Communications, Inc.,  All Rights Reserved.
 * Copyright (C) 2006 FON Technology, SL.
 * Copyright (C) 2006 Imre Kaloz <kaloz@openwrt.org>
 * Copyright (C) 2006 Felix Fietkau <nbd@openwrt.org>
 */

/*
 * Platform devices for AR531x SoC.
 */

#include <linux/autoconf.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <asm/io.h>
#include <ar531x_platform.h>

#include "ar531xlnx.h"

static struct resource ar531x_eth_res[] = {
	{
		.name = "eth_membase",
		.flags = IORESOURCE_MEM,
		.start = 0xb0500000,
		.end = 0xb0502000,
	},
	{
		.name = "eth_irq",
		.flags = IORESOURCE_IRQ,
		.start = 4,
		.end = 4,
	},
};

static struct ar531x_eth ar531x_eth_data = {
	.phy = 1,
	.mac = 0,
	.reset_base = 0x11000004,
	.reset_mac = 0x800,
	.reset_phy = 0x400,
};

static struct platform_device ar531x_eth = {
	.id = 0,
	.name = "ar531x-eth",
	.dev.platform_data = &ar531x_eth_data,
	.resource = ar531x_eth_res,
	.num_resources = ARRAY_SIZE(ar531x_eth_res)
};

static struct platform_device ar531x_wmac = {
	.id = 0,
	.name = "ar531x-wmac",
	/* FIXME: add resources */
};

static struct resource ar531x_spiflash_res[] = {
	{
		.name = "flash_base",
		.flags = IORESOURCE_MEM,
		.start = 0xa8000000,
		.end = 0xa8400000,
	},
	{
		.name = "flash_regs",
		.flags = IORESOURCE_MEM,
		.start = 0x11300000,
		.end = 0x11300012,
	},
};

static struct platform_device ar531x_spiflash = {
	.id = 0,
	.name = "spiflash",
	.resource = ar531x_spiflash_res,
	.num_resources = ARRAY_SIZE(ar531x_spiflash_res)
};

static __initdata struct platform_device *ar531x_devs[] = {
	&ar531x_eth,
	&ar531x_wmac,
	&ar531x_spiflash
};



static void *flash_regs;

static inline __u32 spiflash_regread32(int reg)
{
	volatile __u32 *data = (__u32 *)(flash_regs + reg);

	return (*data);
}

static inline void spiflash_regwrite32(int reg, __u32 data)
{
	volatile __u32 *addr = (__u32 *)(flash_regs + reg);

	*addr = data;
}

#define SPI_FLASH_CTL      0x00
#define SPI_FLASH_OPCODE   0x04
#define SPI_FLASH_DATA     0x08

static __u8 spiflash_probe(void)
{
	 __u32 reg;

	do {
		reg = spiflash_regread32(SPI_FLASH_CTL);
	} while (reg & SPI_CTL_BUSY);

	spiflash_regwrite32(SPI_FLASH_OPCODE, 0xab);

	reg = (reg & ~SPI_CTL_TX_RX_CNT_MASK) | 4 |
        	(1 << 4) | SPI_CTL_START;

	spiflash_regwrite32(SPI_FLASH_CTL, reg);
 
	do {
  		reg = spiflash_regread32(SPI_FLASH_CTL);
	} while (reg & SPI_CTL_BUSY);

	reg = (__u32) spiflash_regread32(SPI_FLASH_DATA);
	reg &= 0xff;

	return (u8) reg;
}

static u8 *find_board_config(void)
{
	char *addr;
	int found = 0;

	for (addr = (char *) (ar531x_spiflash_res[0].end - 0x1000);
		addr >= (char *) (ar531x_spiflash_res[0].end - 0x30000);
		addr -= 0x1000) {

		if ( *(int *)addr == 0x35333131) {
			/* config magic found */
			found = 1;
			break;
		}
	}

	if (!found) {
		printk("WARNING: No board configuration data found!\n");
		addr = NULL;
	}
	
	return addr;
}

static void *find_radio_config(char *board_config)
{
	int dataFound;
	u32 radio_config;
	
	/* 
	 * Now find the start of Radio Configuration data, using heuristics:
	 * Search forward from Board Configuration data by 0x1000 bytes
	 * at a time until we find non-0xffffffff.
	 */
	dataFound = 0;
	for (radio_config = (u32) board_config + 0x1000;
	     (radio_config < (u32) ar531x_spiflash_res[0].end);
	     radio_config += 0x1000) {
		if (*(int *)radio_config != 0xffffffff) {
			dataFound = 1;
			break;
		}
	}

	if (!dataFound) { /* AR2316 relocates radio config to new location */
	    for (radio_config = (u32) board_config + 0xf8;
	     	(radio_config < (u32) ar531x_spiflash_res[0].end - 0x1000 + 0xf8);
			 radio_config += 0x1000) {
			if (*(int *)radio_config != 0xffffffff) {
				dataFound = 1;
				break;
			}
	    }
	}

	if (!dataFound) {
		printk("Could not find Radio Configuration data\n");
		radio_config = 0;
	}

	return (u8 *) radio_config;
}


#define STM_8MBIT_SIGNATURE     0x13
#define STM_16MBIT_SIGNATURE    0x14
#define STM_32MBIT_SIGNATURE    0x15
#define STM_64MBIT_SIGNATURE    0x16


static void __init ar531x_init_flash(void)
{
	u8 sig;
	u32 flash_size = 0;
	unsigned int rcfg_size;
	char *bcfg, *rcfg, *board_config, *radio_config;
	struct ar531x_config *config;
	
	/* probe the flash chip size */
	flash_regs = ioremap_nocache(ar531x_spiflash_res[1].start, ar531x_spiflash_res[1].end - ar531x_spiflash_res[1].start);
	sig = spiflash_probe();
	iounmap(flash_regs);

	switch(sig) {
		case STM_8MBIT_SIGNATURE:
			flash_size = 0x00100000;
			break;
		case STM_16MBIT_SIGNATURE:
			flash_size = 0x00200000;
			break;
		case STM_32MBIT_SIGNATURE:
			flash_size = 0x00400000;
			break;
		case STM_64MBIT_SIGNATURE:
			flash_size = 0x00800000;
			break;
	}

	if (!flash_size)
		return;
	
	ar531x_spiflash_res[0].end = ar531x_spiflash_res[0].start + flash_size;
	
	/* Copy the board and radio data to RAM, because with the new
	 * spiflash driver, accessing the mapped memory directly is no
	 * longer safe */

	bcfg = find_board_config();
	if (!bcfg)
		return;

	board_config = kmalloc(0x1000, GFP_KERNEL);
	memcpy(board_config, bcfg, 0x100);
	ar531x_eth_data.board_config = board_config;

	/* Radio config starts 0x100 bytes after board config, regardless
	 * of what the physical layout on the flash chip looks like */

	rcfg = find_radio_config(bcfg);
	if (!rcfg)
		return;
	printk("Radio config found at offset 0x%x\n", rcfg - bcfg);
	radio_config = board_config + 0x100 + ((rcfg - bcfg) & 0xfff);
	rcfg_size = 0x1000 - ((rcfg - bcfg) & 0xfff);
	memcpy(radio_config, rcfg, rcfg_size);

	config = (struct ar531x_config *) kzalloc(sizeof(struct ar531x_config), GFP_KERNEL);
	config->board = board_config;
	config->radio = radio_config;
	config->unit = 0;
	config->tag = (u_int16_t) (sysRegRead(AR5315_SREV) & REV_CHIP);
	ar531x_wmac.dev.platform_data = config;
}

static int __init ar531x_register_devices(void)
{
	ar531x_init_flash();
	return platform_add_devices(ar531x_devs, ARRAY_SIZE(ar531x_devs));
}


arch_initcall(ar531x_register_devices);
