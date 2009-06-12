/*
 * uclinux/include/asm-armnommu/arch-S3C3410/hardware.h
 *
 * 2003 Thomas Eschenbacher <thomas.eschenbacher@gmx.de>
 *
 * Hardware definitions of SMDK40100 board, currently only
 * the CPUs registers and definitions are included from
 * s3c3410.h.
 */

#ifndef __ASM_ARCH_HARDWARE_H
#define __ASM_ARCH_HARDWARE_H

#include <linux/config.h>
#include <asm/arch/s3c3410.h>

#ifndef __ASSEMBLER__

/**************************************** RAM definitions */
// #define MAPTOPHYS(a)      ((unsigned long)a)
// #define KERNTOPHYS(a)     ((unsigned long)(&a))
// #define GET_MEMORY_END(p) ((p->u1.s.page_size) * (p->u1.s.nr_pages))
// #define PARAMS_BASE       0x7000
#define HARD_RESET_NOW()  { arch_hard_reset(); }

#endif /* ! __ASSEMBLER__ */

#endif /* __ASM_ARCH_HARDWARE_H */
