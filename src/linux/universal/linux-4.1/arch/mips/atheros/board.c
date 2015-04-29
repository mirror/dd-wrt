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
//#include "board3.h"
void (*ar231x_irq_dispatch)(void);

char *board_config, *radio_config;

struct ar231x_boarddata default_config;
static int fake_config=0;

u8 *find_board_config(char *flash_limit)
{
	char *addr;
	int found = 0;
	char Name[]="Atheros AR5001 default                                            ";
	int i;
	u32 x;
	u8 mac[6],wmac[6];
	for (addr = (char *) (flash_limit - 0x1000);
		addr >= (char *) (flash_limit - 0x30000);
		addr -= 0x1000) {
		printk("scanning for board data at %X (%X)\n",addr,*(int *)addr);
		if ( *(int *)addr == 0x35333131) {
			/* config magic found */
			found = 1;
			break;
		}
	}
	printk("found %d\n",found);
	if (!found) {
		addr = NULL;
		if (strstr((char*)(0xbfc00010),"CA804.SOB") || strstr((char*)(0xbfc00010),"CE801.SOB") || strstr((char*)(0xbfc00010),"OVISCA401") || strstr((char*)(0xbfc00010),"OVISCE401") || strstr((char*)(0xbfc00010),"RCAAO1") || strstr((char*)(0xbfc00010),"RDAT81.SOB"))
		 {
		
 		printk("WARNING: No board configuration data found, creating defaults in memory!\n");
 		
 		default_config.magic=AR531X_BD_MAGIC;
 		default_config.cksum=0x1d29;
 		default_config.rev=BD_REV;
 		default_config.major=0x1;
 		default_config.minor=0x0;
 		default_config.flags=0 | BD_ENET1 | BD_UART0 | BD_RSTFACTORY | BD_SYSLED | BD_WLAN1 | BD_ISCASPER | BD_WLAN1_2G_EN | BD_WLAN1_5G_EN;
 		default_config.resetConfigGpio=0x06;
 		default_config.sysLedGpio=0x07;
 		default_config.cpuFreq=0x0aba9500;
 		default_config.sysFreq=0x02aea540;
 		default_config.cntFreq=0x055d4a80;
 		default_config.pciId=0x13;
 		default_config.memCap=0x800f;
		if (strstr((char*)(0xbfc00010),"RCAAO1") || strstr((char*)(0xbfc00010),"RDAT81.SOB"))  // emulates board data for AR5312 dual wifi devices
		{
 		default_config.flags=0 | BD_ENET1 | BD_UART0 | BD_RSTFACTORY | BD_SYSLED | BD_WLAN1 | BD_WLAN1_2G_EN | BD_WLAN1_5G_EN;		
		}


		memcpy(wmac,(char*)0xbfc00044,6);
	x=0x100;
	for (i=0; i<6; i++) {
		if (x>0xff) x=1;
		else x=0;
		x=x+wmac[5-i];
		mac[5-i]=x & 0xff;
	}

	printk ("Create Board Data\n");
	printk ("WMAC %02X:%02X:%02X:%02X:%02X:%02X\n",wmac[0],wmac[1],wmac[2],wmac[3],wmac[4],wmac[5]);
	printk (" MAC %02X:%02X:%02X:%02X:%02X:%02X\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);																										
 	for (i=0; i < 6; i++) {
	        default_config.wlan0_mac[i]=wmac[i];
	        default_config.enet0_mac[i]=wmac[i];
	        default_config.enet1_mac[i]=mac[i];
	        default_config.wlan1_mac[i]=wmac[i];
	}
	
 		
 		for (i=0; i<64 ; i++) {
 		    default_config.boardName[i] = Name[i];
 		}
		fake_config=1;
 
 		addr = (char*) &default_config;
		}
		else
		{
		printk("WARNING: No board configuration data found!\n");
		addr=NULL;
//		addr=&bdata;
		}
		
	}
	
	return addr;
}

u8 *find_radio_config(char *flash_limit, char *bconfig)
{
	int dataFound;
	u32 rconfig;
    if (!strncmp(bconfig+8,"Atheros AR5001AP Bountiful Wifi",31))
	{
	    printk( "Found Bountiful Wifi Router\n");
	    return bconfig-0x2000;
	}
	
	if (fake_config)
	    bconfig = flash_limit-0x10000;
	/* 
	 * Now find the start of Radio Configuration data, using heuristics:
	 * Search forward from Board Configuration data by 0x1000 bytes
	 * at a time until we find non-0xffffffff.
	 */
	dataFound = 0;
	for (rconfig = (u32) bconfig + 0x1000;
	     (rconfig < (u32) flash_limit);
	     rconfig += 0x1000) {
		if (*(int *)rconfig != 0xffffffff && *(int *)rconfig != 0x52434b53) {
			dataFound = 1;
			break;
		}
	}

#ifdef CONFIG_ATHEROS_AR5315
	if (!dataFound) { /* AR2316 relocates radio config to new location */
	    for (rconfig = (u32) bconfig + 0xf8;
	     	(rconfig < (u32) flash_limit - 0x1000 + 0xf8);
			 rconfig += 0x1000) {
			if (*(int *)rconfig != 0xffffffff) {
				dataFound = 1;
				break;
			}
	    }
	}
#endif

	if (!dataFound) {
		rconfig=flash_limit-0x10000;
		printk("Could not find Radio Configuration data\n");
		rconfig = 0;
	}

	return (u8 *) rconfig;
}

int __init ar231x_find_config(u8 *flash_limit)
{
	unsigned int rcfg_size;
	char *bcfg, *rcfg;

	/* Copy the board and radio data to RAM, because with the new
	 * spiflash driver, accessing the mapped memory directly is no
	 * longer safe */

	bcfg = find_board_config(flash_limit);
	if (!bcfg)
		return -ENODEV;

	board_config = kzalloc(BOARD_CONFIG_BUFSZ, GFP_KERNEL);
	ar231x_board.config = (struct ar231x_boarddata *) board_config;
	memcpy(board_config, bcfg, 0x100);

	/* Radio config starts 0x100 bytes after board config, regardless
	 * of what the physical layout on the flash chip looks like */

	rcfg = find_radio_config(flash_limit, bcfg);
	if (!rcfg)
		return -ENODEV;

	radio_config = board_config + 0x100 + ((rcfg - bcfg) & 0xfff);
	if (fake_config)
	    radio_config = board_config + 0x100 + ((0xc000) & 0xfff);
	ar231x_board.radio = radio_config;
	printk("Found board config at 0x%x\n",bcfg);
	printk("Radio config found at offset 0x%x(0x%x) (%p)\n", rcfg - bcfg, radio_config - board_config,rcfg);
	rcfg_size = BOARD_CONFIG_BUFSZ - ((rcfg - bcfg) & (BOARD_CONFIG_BUFSZ - 1));
	if (fake_config)
	    rcfg_size = BOARD_CONFIG_BUFSZ - (0xc000 & (BOARD_CONFIG_BUFSZ - 1));
	memcpy(radio_config, rcfg, rcfg_size);
	
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
unsigned int ath_cpufreq;

unsigned int getCPUClock(void)
{
		return ath_cpufreq / 1000 / 1000;
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


