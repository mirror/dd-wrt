/*
 * include/asm-v850/sim.h -- Machine-dependent defs for GDB v850e simulator
 *
 *  Copyright (C) 2001,02,03  NEC Electronics Corporation
 *  Copyright (C) 2001,02,03  Miles Bader <miles@gnu.org>
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License.  See the file COPYING in the main directory of this
 * archive for more details.
 *
 * Written by Miles Bader <miles@gnu.org>
 */

#ifndef __V850_SIM_H__
#define __V850_SIM_H__


#define CPU_ARCH		"v850e"
#define CPU_MODEL		"v850e"
#define CPU_MODEL_LONG		"NEC V850E"
#define PLATFORM		"gdb/v850e"
#define PLATFORM_LONG		"GDB V850E simulator"


/* Though it varies based on the speed of the host processor, the bogomips
   value on the simulator is basically always completely wrong anyway, and
   fixing it allows faster startup.  */
#define FIXED_BOGOMIPS		13


/* We use a weird value for RAM, not just 0, for testing purposes.
   These must match the values used in the linker script.  */
#define RAM_ADDR		0x8F000000
#define RAM_SIZE		0x03000000


/* For <asm/page.h> */
#define PAGE_OFFSET 		RAM_ADDR


/* For <asm/entry.h> */
/* `R0 RAM', used for a few miscellaneous variables that must be
   accessible using a load instruction relative to R0.  On real
   processors, this usually is on-chip RAM, but here we just
   choose an arbitrary address that meets the above constraint.  */
#define R0_RAM_ADDR		0xFFFFF000


/* For <asm/param.h> */
#ifndef HZ
#define HZ			24	/* Minimum supported frequency.  */
#endif

/* For <asm/irq.h> */
#define NUM_CPU_IRQS		6


#ifndef __ASSEMBLY__

/* For <asm/blkmem.h> */
/* This gives us a single arena.  */
#define CAT_ROMARRAY
/* These should be defined by sim startup.  */
extern unsigned long root_fs_image_addr, root_fs_image_len;
extern int root_fs_image_rw;
/* Make the single arena point use them.  */
#define FIXUP_ARENAS							      \
        arena[0].rw = root_fs_image_rw;					      \
	arena[0].address = root_fs_image_addr;				      \
	arena[0].length = root_fs_image_len;

#endif /* !__ASSEMBLY__ */


#endif /* __V850_SIM_H__ */
