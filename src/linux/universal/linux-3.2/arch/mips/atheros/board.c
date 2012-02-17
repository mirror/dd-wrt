/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2003 Atheros Communications, Inc.,  All Rights Reserved.
 * Copyright (C) 2006 FON Technology, SL.
 * Copyright (C) 2006 Imre Kaloz <kaloz@openwrt.org>
 * Copyright (C) 2006-2009 Felix Fietkau <nbd@openwrt.org>
 */

#include <generated/autoconf.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/random.h>
#include <linux/etherdevice.h>
#include <asm/irq_cpu.h>
#include <asm/reboot.h>
#include <asm/io.h>
#include <asm/irq.h>

#include <ar231x_platform.h>
#include "devices.h"
#include "ar5312.h"
#include "ar2315.h"

void (*ar231x_irq_dispatch)(void);

static inline bool
check_radio_magic(u8 *addr)
{
	addr += 0x7a; /* offset for flash magic */
	if ((addr[0] == 0x5a) && (addr[1] == 0xa5)) {
		return 1;
	}
	return 0;
}

static inline bool
check_board_data(u8 *flash_limit, u8 *addr, bool broken)
{
	/* config magic found */
	if (*((u32 *)addr) == AR531X_BD_MAGIC)
		return 1;

	if (!broken)
		return 0;

	if (check_radio_magic(addr + 0xf8))
		ar231x_board.radio = addr + 0xf8;
	if ((addr < flash_limit + 0x10000) &&
	     check_radio_magic(addr + 0x10000))
		ar231x_board.radio = addr + 0x10000;

	if (ar231x_board.radio) {
		/* broken board data detected, use radio data to find the offset,
		 * user will fix this */
		return 1;
	}
	return 0;
}

static u8 *
find_board_config(u8 *flash_limit, bool broken)
{
	u8 *addr;
	int found = 0;

	for (addr = flash_limit - 0x1000;
		addr >= flash_limit - 0x30000;
		addr -= 0x1000) {

		if (check_board_data(flash_limit, addr, broken)) {
			found = 1;
			break;
		}
	}

	if (!found)
		addr = NULL;

	return addr;
}

static u8 *
find_radio_config(u8 *flash_limit, u8 *board_config)
{
	int found;
	u8 *radio_config;

	/*
	 * Now find the start of Radio Configuration data, using heuristics:
	 * Search forward from Board Configuration data by 0x1000 bytes
	 * at a time until we find non-0xffffffff.
	 */
	found = 0;
	for (radio_config = board_config + 0x1000;
	     (radio_config < flash_limit);
	     radio_config += 0x1000) {
		if ((*(u32 *)radio_config != 0xffffffff) &&
		    check_radio_magic(radio_config)) {
			found = 1;
			break;
		}
	}

	/* AR2316 relocates radio config to new location */
	if (!found) {
	    for (radio_config = board_config + 0xf8;
			(radio_config < flash_limit - 0x1000 + 0xf8);
			 radio_config += 0x1000) {
			if ((*(u32 *)radio_config != 0xffffffff) &&
				check_radio_magic(radio_config)) {
				found = 1;
				break;
			}
	    }
	}

	if (!found) {
		printk("Could not find Radio Configuration data\n");
		radio_config = 0;
	}

	return (u8 *) radio_config;
}

int __init
ar231x_find_config(u8 *flash_limit)
{
	struct ar231x_boarddata *config;
	unsigned int rcfg_size;
	int broken_boarddata = 0;
	u8 *bcfg, *rcfg;
	u8 *board_data;
	u8 *radio_data;
	u8 *mac_addr;
	u32 offset;

	ar231x_board.config = NULL;
	ar231x_board.radio = NULL;
	/* Copy the board and radio data to RAM, because accessing the mapped
	 * memory of the flash directly after booting is not safe */

	/* Try to find valid board and radio data */
	bcfg = find_board_config(flash_limit, false);

	/* If that fails, try to at least find valid radio data */
	if (!bcfg) {
		bcfg = find_board_config(flash_limit, true);
		broken_boarddata = 1;
	}

	if (!bcfg) {
		printk(KERN_WARNING "WARNING: No board configuration data found!\n");
		return -ENODEV;
	}

	board_data = kzalloc(BOARD_CONFIG_BUFSZ, GFP_KERNEL);
	ar231x_board.config = (struct ar231x_boarddata *) board_data;
	memcpy(board_data, bcfg, 0x100);
	if (broken_boarddata) {
		printk(KERN_WARNING "WARNING: broken board data detected\n");
		config = ar231x_board.config;
		if (!memcmp(config->enet0_mac, "\x00\x00\x00\x00\x00\x00", 6)) {
			printk(KERN_INFO "Fixing up empty mac addresses\n");
			config->resetConfigGpio = 0xffff;
			config->sysLedGpio = 0xffff;
			random_ether_addr(config->wlan0_mac);
			config->wlan0_mac[0] &= ~0x06;
			random_ether_addr(config->enet0_mac);
			random_ether_addr(config->enet1_mac);
		}
	}


	/* Radio config starts 0x100 bytes after board config, regardless
	 * of what the physical layout on the flash chip looks like */

	if (ar231x_board.radio)
		rcfg = (u8 *) ar231x_board.radio;
	else
		rcfg = find_radio_config(flash_limit, bcfg);

	if (!rcfg)
		return -ENODEV;

	radio_data = board_data + 0x100 + ((rcfg - bcfg) & 0xfff);
	ar231x_board.radio = radio_data;
	offset = radio_data - board_data;
	printk(KERN_INFO "Radio config found at offset 0x%x(0x%x)\n", rcfg - bcfg, offset);
	rcfg_size = BOARD_CONFIG_BUFSZ - offset;
	memcpy(radio_data, rcfg, rcfg_size);

	mac_addr = &radio_data[0x1d * 2];
	if (is_broadcast_ether_addr(mac_addr)) {
		printk(KERN_INFO "Radio MAC is blank; using board-data\n");
		memcpy(mac_addr, ar231x_board.config->wlan0_mac, ETH_ALEN);
	}

	return 0;
}

static void
ar231x_halt(void)
{
	local_irq_disable();
	while (1);
}

void __init
plat_mem_setup(void)
{
	_machine_halt = ar231x_halt;
	pm_power_off = ar231x_halt;

	ar5312_plat_setup();
	ar2315_plat_setup();

	/* Disable data watchpoints */
	write_c0_watchlo0(0);
}


asmlinkage void
plat_irq_dispatch(void)
{
	ar231x_irq_dispatch();
}

void __init
plat_time_init(void)
{
	ar5312_time_init();
	ar2315_time_init();
}

unsigned int __cpuinit
get_c0_compare_int(void)
{
	return CP0_LEGACY_COMPARE_IRQ;
}

int getCPUClock(void)
{

		if (is_2315())
		    return ar2315_cpu_frequency();
		else
		    return ar5312_cpu_frequency();
		

}
void __init
arch_init_irq(void)
{
	clear_c0_status(ST0_IM);
	mips_cpu_irq_init();

	/* Initialize interrupt controllers */
	ar5312_irq_init();
	ar2315_irq_init();
}


