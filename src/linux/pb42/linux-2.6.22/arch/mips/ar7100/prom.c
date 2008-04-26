
/*
 * Prom setup file for ar7100
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/bootmem.h>

#include <asm/bootinfo.h>
#include <asm/addrspace.h>

#include "ar7100.h"

#define sysRegRead(phys)	\
	(*(volatile u32 *)KSEG1ADDR(phys))

void __init prom_init(void)
{
	int memsz = 0x1000000, argc = fw_arg0, i;
	char **arg = (char**) fw_arg1;

	/* 
	 * if user passes kernel args, ignore the default one 
	 */
/*	if (argc > 1) {
		arcs_cmdline[0] = '\0';

		for (i = 1; i < argc; i++) 
			printk("arg %d: %s\n", i, arg[i]);

		for (i = 1; i < argc; i++) {
			if (strlen(arcs_cmdline) + strlen(arg[i] + 1)
			    >= sizeof(arcs_cmdline))
				break;
			strcat(arcs_cmdline, arg[i]);
			strcat(arcs_cmdline, " ");
		}
	}*/

	mips_machgroup = MACH_GROUP_AR7100;
	mips_machtype  = MACH_ATHEROS_AP81;

//	int memcfg = sysRegRead(AR7100_DDR_CONFIG);

//	memsize   = 1 + ((memcfg & SDRAM_DATA_WIDTH_M) >> SDRAM_DATA_WIDTH_S);
//	memsize <<= 1 + ((memcfg & SDRAM_COL_WIDTH_M) >> SDRAM_COL_WIDTH_S);
//	memsize <<= 1 + ((memcfg & SDRAM_ROW_WIDTH_M) >> SDRAM_ROW_WIDTH_S);
//	memsize <<= 3;

	/*
	 * By default, use all available memory.  You can override this
	 * to use, say, 8MB by specifying "mem=8M" as an argument on the
	 * linux bootup command line.
	 */
	add_memory_region(0, memsz, BOOT_MEM_RAM);
}

void __init prom_free_prom_memory(void)
{
}
