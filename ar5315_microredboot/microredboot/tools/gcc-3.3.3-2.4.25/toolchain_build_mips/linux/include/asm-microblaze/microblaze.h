/*
 * include/asm-microblaze/microblaze.h -- Xilinx Microblaze softcore CPU
 *
 *  Copyright (C) 2003       John Williams <jwilliams@itee.uq.edu.au>
 *  Copyright (C) 2001,2002  NEC Corporation
 *  Copyright (C) 2001,2002  Miles Bader <miles@gnu.org>
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License.  See the file COPYING in the main directory of this
 * archive for more details.
 *
 * Written by Miles Bader <miles@gnu.org>
 * Microblaze port by John Williams
 */

#ifndef __MICROBLAZE_H__
#define __MICROBLAZE_H__

#include <asm/clinkage.h>
#define CPU_MODEL	"Microblaze"
#define CPU_MODEL_LONG	"Xilinx Microblaze"
#define CPU_ARCH 	"microblaze"

/* For <asm/irq.h> */
#define NUM_CPU_IRQS	32

#ifdef __ASSEMBLY__
#define R0_RAM_ADDR (C_SYMBOL_NAME(r0_ram))
#else
extern char C_SYMBOL_NAME(r0_ram);
#define R0_RAM_ADDR ((unsigned int)&C_SYMBOL_NAME(r0_ram))
#endif
 
/* #include <asm/microblaze_cache.h>  */

#endif /* __MICROBLAZE_H__ */
