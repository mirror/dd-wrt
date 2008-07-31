/*
 *  linux/arch/arm/mach-epxa10db/arch.c
 *
 *  Copyright (C) 2000 Deep Blue Solutions Ltd
 *  Copyright (C) 2001 Altera Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <linux/types.h>
#include <linux/init.h>

#include <asm/hardware.h>
#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/mach/time.h>
#include <asm/mach/arch.h>

extern void sl2312_map_io(void);
extern void sl2312_init_irq(void);
extern unsigned long sl2312_gettimeoffset (void);
extern void __init sl2312_time_init(void);

static struct sys_timer sl2312_timer = {
	.init		= sl2312_time_init,
	.offset		= sl2312_gettimeoffset,
};

static void __init
sl2312_fixup(struct machine_desc *desc, struct tag *tags,
                 char **cmdline, struct meminfo *mi)
{
        mi->nr_banks      = 1;
        mi->bank[0].start = 0;
#ifdef CONFIG_GEMINI_IPI
        mi->bank[0].size  = (64*1024*1024);  // 128M
#else
        mi->bank[0].size  = (32*1024*1024);  // 128M
#endif
        mi->bank[0].node  = 0;
}

/* MACHINE_START(SL2312, "GeminiA")
	MAINTAINER("Storlink Semi")
	BOOT_MEM(0x00000000, 0x90000000, 0xf0000000)
        FIXUP(sl2312_fixup)
	MAPIO(sl2312_map_io)
	INITIRQ(sl2312_init_irq)
	.timer = &sl2312_timer,
MACHINE_END */

MACHINE_START(SL2312, "GeminiA")
	/* .phys_ram	= 0x00000000, */
	.phys_io	= 0x7fffc000,
	.io_pg_offst	= ((0xffffc000) >> 18) & 0xfffc,
	.boot_params	= 0x100,
	.fixup      = sl2312_fixup,
	.map_io		= sl2312_map_io,
	.init_irq	= sl2312_init_irq,
	.timer		= &sl2312_timer,
MACHINE_END
