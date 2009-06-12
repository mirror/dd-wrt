/*
 * uclinux/include/asm-armnommu/arch-S3C44B0X/hardware.h
 *
 * Copyright (C) 2003 Thomas Eschenbacher <eschenbacher@sympat.de>
 *
 * Hardware definitions of MBA44 board, currently only
 * the CPUs registers and definitions are included from
 * s3c44b0x.h.
 */

#ifndef __ASM_ARCH_HARDWARE_H
#define __ASM_ARCH_HARDWARE_H

#include <linux/config.h>
#include <asm/arch/s3c44b0x.h>

#ifndef __ASSEMBLER__

#define HARD_RESET_NOW()  { arch_hard_reset(); }

#endif /* ! __ASSEMBLER__ */

#endif /* __ASM_ARCH_HARDWARE_H */
