/*
 *  linux/arch/arm/mm/tlb-fa.S
 *
 *  Copyright (C) 2005 Faraday Corp.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  ARM architecture version 4, Faraday variation.
 *  This assume an unified TLBs, with a write buffer, and branch target buffer (BTB)
 *
 *  Processors: FA520 FA526 FA626
 *  03/31/2005 : Created by Luke Lee, modified from tlb-v4wbi.S
 *  05/06/2005 : Fixed buggy CPU versions that did not invalidate the associated
 *               data cache entries when invalidating TLB entries.
 */
#include <linux/linkage.h>
#include <linux/init.h>
#include <asm/asm-offsets.h>
#include <asm/tlbflush.h>
#include "proc-macros.S"


/*
 *	flush_user_tlb_range(start, end, mm)
 *
 *	Invalidate a range of TLB entries in the specified address space.
 *
 *	- start - range start address
 *	- end   - range end address
 *	- mm    - mm_struct describing address space
 */
	.align	4
ENTRY(fa_flush_user_tlb_range)

	vma_vm_mm ip, r2
	act_mm	r3				@ get current->active_mm
	eors	r3, ip, r3			@ == mm ?
	movne	pc, lr				@ no, we dont do anything
	mov	r3, #0

#ifndef CONFIG_CPU_FA_WB_DISABLE
	mcr	p15, 0, r3, c7, c10, 4		@ drain WB
#endif

	vma_vm_flags r2, r2
	bic	r0, r0, #0x0ff
	bic	r0, r0, #0xf00

1:	mcr	p15, 0, r0, c8, c7, 1		@ invalidate UTLB entry
	add	r0, r0, #PAGE_SZ
	cmp	r0, r1
	bls	1b				@ Luke Lee 05/19/2005 blo -> bls

#ifdef CONFIG_CPU_FA_BTB
	mcr	p15, 0, r3, c7, c5, 6		@ invalidate BTB
	nop
	nop
#endif
	mov	pc, lr


ENTRY(fa_flush_kern_tlb_range)
	mov	r3, #0

	mcr	p15, 0, r3, c7, c10, 0		@ clean Dcache all 06/03/2005

#ifndef CONFIG_CPU_FA_WB_DISABLE
	mcr	p15, 0, r3, c7, c10, 4		@ drain WB
#endif

	bic	r0, r0, #0x0ff
	bic	r0, r0, #0xf00
1:
	mcr	p15, 0, r0, c8, c7, 1		@ invalidate UTLB entry
	add	r0, r0, #PAGE_SZ
	cmp	r0, r1
	bls	1b				@ Luke Lee 05/19/2005 blo -> bls

#ifdef CONFIG_CPU_FA_BTB
	mcr	p15, 0, r3, c7, c5, 6		@ invalidate BTB
	nop
	nop
#endif
	mov	pc, lr


	__INITDATA

	.type	fa_tlb_fns, #object
ENTRY(fa_tlb_fns)
	.long	fa_flush_user_tlb_range
	.long	fa_flush_kern_tlb_range
	.long	fa_tlb_flags
	.size	fa_tlb_fns, . - fa_tlb_fns
