/*
 * Early initialization code for BCM94710 boards
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: prom.c,v 1.2 2008/07/04 01:05:29 Exp $
 */

#include <linux/config.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <asm/bootinfo.h>
#include <asm/cpu.h>
#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <hndsoc.h>
#include <siutils.h>
#include <hndcpu.h>
#include <mipsinc.h>
#include <mips74k_core.h>

void __init
prom_init(void)
{
	unsigned long mem;

	mips_machgroup = MACH_GROUP_BRCM;
	mips_machtype = MACH_BCM947XX;

	/* Figure out memory size by finding aliases */
	for (mem = (1 << 20); mem < (128 << 20); mem <<= 1) {
		if (*(unsigned long *)((unsigned long)(prom_init) + mem) == 
		    *(unsigned long *)(prom_init))
			break;
	} 

	/* Ignoring the last page when ddr size is 128M. Cached
	 * accesses to last page is causing the processor to prefetch
	 * using address above 128M stepping out of the ddr address
	 * space.
	 */
	if (MIPS74K(current_cpu_data.processor_id) && (mem == 0x8000000))
		mem -= 0x1000;

	add_memory_region(0, mem, BOOT_MEM_RAM);
}

void __init
prom_free_prom_memory(void)
{
}
