/*
 * Early initialization code for BCM94710 boards
 *
 * Copyright (C) 2004 Broadcom Corporation
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <linux/config.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <asm/bootinfo.h>
#include <asm/processor.h>
#include <typedefs.h>
#include <mips74k_core.h>

void __init prom_init(int argc, char **argv, char **envp, int *prom_vec)
{
	unsigned long mem, before, offset;

	mips_machgroup = MACH_GROUP_BRCM;
	mips_machtype  = MACH_BCM947XX;

	/* Figure out memory size by finding aliases.
	 *
	 * We assume that there will be no more than 128 MB of memory,
	 * and that the memory size will be a multiple of 1 MB.
	 *
	 * We set 'before' to be the amount of memory (in MB) before this
	 * function, i.e. one MB less than the number  of MB of memory that we
	 * *know* we have.  And we set 'offset' to be the address of 'prominit'
	 * minus 'before', so that KSEG0 or KSEG1 base + offset < 1 MB.
	 * This prevents us from overrunning 128 MB and causing a bus error.
	 */
	before = ((unsigned long) &prom_init) & (127 << 20);
	offset = ((unsigned long) &prom_init) - before;
	for (mem = before + (1 << 20); mem < (128 << 20); mem += (1 << 20))
		if (*(unsigned long *)(offset + mem) ==
		    *(unsigned long *)(prom_init)) {
			/*
			 * We may already be well past the end of memory at
			 * this point, so we'll have to compensate for it.
			 */
			mem -= before;
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

void __init prom_free_prom_memory(void)
{
}
