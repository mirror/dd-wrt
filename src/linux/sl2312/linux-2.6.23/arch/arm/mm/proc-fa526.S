/*
 *  linux/arch/arm/mm/proc-fa526.S: MMU functions for FA526
 *
 *  Copyright (C) 2005 Faraday Corp.
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
 *
 *
 * These are the low level assembler for performing cache and TLB
 * functions on the fa526.
 *
 *  Written by : Luke Lee
 */
#include <linux/linkage.h>
#include <linux/init.h>
#include <asm/assembler.h>
#include <asm/pgtable.h>
#include <asm/pgtable-hwdef.h>
#include <asm/elf.h>
#include <asm/hardware.h>
#include <asm/page.h>
#include <asm/ptrace.h>
#include <asm/system.h>
#include "proc-macros.S"

#define CACHE_DLINESIZE	   16

	.text
/*
 * cpu_fa526_proc_init()
 */
ENTRY(cpu_fa526_proc_init)
	/* MMU is already ON here, ICACHE, DCACHE conditionally disabled */

        mov     r0, #1
	nop
	nop
        mcr     p15, 0, r0, c1, c1, 0		@ turn-on ECR
	nop
	nop

	mrc	p15, 0, r0, c1, c0, 0		@ read ctrl register

#ifdef CONFIG_CPU_FA_BTB
	orr	r0, r0, #CR_Z
#else
	bic	r0, r0, #CR_Z
#endif
#ifdef CONFIG_CPU_FA_WB_DISABLE
	mov	r1, #0
	mcr	p15, 0, r1, c7, c10, 4		@ drain write buffer
	nop
	nop
	bic	r0, r0, #CR_W
#else
	orr	r0, r0, #CR_W
#endif
#ifdef CONFIG_CPU_DCACHE_DISABLE
	bic	r0, r0, #CR_C
#else
	orr	r0, r0, #CR_C
#endif
#ifdef CONFIG_CPU_ICACHE_DISABLE
	bic	r0, r0, #CR_I
#else
	orr	r0, r0, #CR_I
#endif

	nop
	nop
    	mcr	p15, 0, r0, c1, c0, 0
	nop
	nop

	mov	r5, lr
	bl	fa_initialize_cache_info	@ destroy r0~r4
	mov	pc, r5				@ return


/*
 * cpu_fa526_proc_fin()
 */
ENTRY(cpu_fa526_proc_fin)
	stmfd	sp!, {lr}
	mov	ip, #PSR_F_BIT | PSR_I_BIT | SVC_MODE
	msr	cpsr_c, ip

	bl	fa_flush_kern_cache_all
	mrc	p15, 0, r0, c1, c0, 0		@ ctrl register
	bic	r0, r0, #0x1000			@ ...i............
	bic	r0, r0, #0x000e			@ ............wca.
	mcr	p15, 0, r0, c1, c0, 0		@ disable caches

	nop
	nop
	ldmfd	sp!, {pc}

/*
 * cpu_fa526_reset(loc)
 *
 * Perform a soft reset of the system.  Put the CPU into the
 * same state as it would be if it had been reset, and branch
 * to what would be the reset vector.
 *
 * loc: location to jump to for soft reset
 */
	.align	4
ENTRY(cpu_fa526_reset)
	mov	ip, #0
	mcr	p15, 0, ip, c7, c7, 0		@ invalidate I,D caches
#ifndef CONFIG_CPU_FA_WB_DISABLE
	mcr	p15, 0, ip, c7, c10, 4		@ drain WB
#endif
	mcr	p15, 0, ip, c8, c7, 0		@ invalidate I & D TLBs
	mrc	p15, 0, ip, c1, c0, 0		@ ctrl register
	bic	ip, ip, #0x000f			@ ............wcam
	bic	ip, ip, #0x1100			@ ...i...s........

	bic	ip, ip, #0x0800			@ BTB off
	mcr	p15, 0, ip, c1, c0, 0		@ ctrl register
	nop
	nop
	mov	pc, r0

/*
 * cpu_fa526_do_idle()
 */
	.align	4
ENTRY(cpu_fa526_do_idle)

#ifdef CONFIG_CPU_FA_IDLE
	nop
	nop
	mcr	p15, 0, r0, c7, c0, 4		@ Wait for interrupt (IDLE mode)
#endif
	mov	pc, lr


ENTRY(cpu_fa526_dcache_clean_area)

#ifndef CONFIG_CPU_DCACHE_DISABLE
#ifndef CONFIG_CPU_DCACHE_WRITETHROUGH
1:	mcr	p15, 0, r0, c7, c10, 1		@ clean D entry
	add	r0, r0, #CACHE_DLINESIZE
	subs	r1, r1, #CACHE_DLINESIZE
	bhi	1b
#endif
#endif
	mov	pc, lr


/* =============================== PageTable ============================== */

/*
 * cpu_fa526_switch_mm(pgd)
 *
 * Set the translation base pointer to be as described by pgd.
 *
 * pgd: new page tables
 */
	.align	4

	.globl	fault_address
fault_address:
	.long	0

ENTRY(cpu_fa526_switch_mm)

	mov	ip, #0
#ifndef CONFIG_CPU_DCACHE_DISABLE
#ifdef CONFIG_CPU_DCACHE_WRITETHROUGH
	mcr	p15, 0, ip, c7, c6, 0		@ invalidate D cache
#else
	mcr	p15, 0, ip, c7, c14, 0		@ Clean and invalidate whole DCache
#endif
#endif /*CONFIG_CPU_DCACHE_DISABLE*/

#ifndef CONFIG_CPU_ICACHE_DISABLE
	mcr	p15, 0, ip, c7, c5, 0		@ invalidate I cache
#endif

#ifndef CONFIG_CPU_FA_WB_DISABLE
	mcr	p15, 0, ip, c7, c10, 4		@ drain WB
#endif

#ifdef CONFIG_CPU_FA_BTB
	mcr	p15, 0, ip, c7, c5, 6		@ invalidate BTB since mm changed
	nop
	nop
#endif
	bic	r0, r0, #0xff			@ clear bits [7:0]
	bic	r0, r0, #0x3f00			@ clear bits [13:8]
	mcr	p15, 0, r0, c2, c0, 0		@ load page table pointer
	mcr	p15, 0, ip, c8, c7, 0		@ invalidate UTLB
	nop
	nop
	mov	pc, lr

/*
 * cpu_fa526_set_pte_ext(ptep, pte, ext)
 *
 * Set a PTE and flush it out
 */
	.align	4
ENTRY(cpu_fa526_set_pte_ext)
	str	r1, [r0], #-2048		@ linux version

	eor	r1, r1, #L_PTE_PRESENT | L_PTE_YOUNG | L_PTE_WRITE | L_PTE_DIRTY

	bic	r2, r1, #PTE_SMALL_AP_MASK
	bic	r2, r2, #PTE_TYPE_MASK
	orr	r2, r2, #PTE_TYPE_SMALL

	tst	r1, #L_PTE_USER			@ User?
	orrne	r2, r2, #PTE_SMALL_AP_URO_SRW

	tst	r1, #L_PTE_WRITE | L_PTE_DIRTY	@ Write and Dirty?
	orreq	r2, r2, #PTE_SMALL_AP_UNO_SRW

	tst	r1, #L_PTE_PRESENT | L_PTE_YOUNG	@ Present and Young?
	movne	r2, #0

#ifdef CONFIG_CPU_DCACHE_WRITETHROUGH
	eor	r3, r2, #0x0a			@ C & small page?  1010
	tst	r3, #0x0b			@		   1011
	biceq	r2, r2, #4
#endif
	str	r2, [r0]			@ hardware version

	mov	r2, #0
	mcr	p15, 0, r2, c7, c10, 0		@ clean D cache	all

#ifndef CONFIG_CPU_FA_WB_DISABLE
	mcr	p15, 0, r2, c7, c10, 4		@ drain WB
#endif
#ifdef CONFIG_CPU_FA_BTB
	mcr	p15, 0, r2, c7, c5, 6		@ invalidate BTB
	nop
	nop
#endif
	mov	pc, lr

	__INIT

	.type	__fa526_setup, #function
__fa526_setup:
	/* On return of this routine, r0 must carry correct flags for CFG register */
	mov	r0, #0
	mcr	p15, 0, r0, c7, c7		@ invalidate I,D caches on v4
	mcr	p15, 0, r0, c7, c10, 4		@ drain write buffer on v4
	mcr	p15, 0, r0, c8, c7		@ invalidate I,D TLBs on v4

	mcr	p15, 0, r0, c7, c5, 5		@ invalidate IScratchpad RAM

        mov     r0, #1
        mcr     p15, 0, r0, c1, c1, 0		@ turn-on ECR

	mrc	p15, 0, r0, c9, c1, 0		@ DScratchpad
	bic	r0, r0, #1
	mcr	p15, 0, r0, c9, c1, 0
	mrc	p15, 0, r0, c9, c1, 1		@ IScratchpad
	bic	r0, r0, #1
	mcr	p15, 0, r0, c9, c1, 1

	mov	r0, #0
	mcr	p15, 0, r0, c1, c1, 0		@ turn-off ECR

#ifdef CONFIG_CPU_FA_BTB
	mcr	p15, 0, r0, c7, c5, 6		@ invalidate BTB All
	nop
	nop
#endif

	mov	r0, #0x1f			@ Domains 0, 1 = manager, 2 = client
	mcr	p15, 0, r0, c3, c0		@ load domain access register

	mrc	p15, 0, r0, c1, c0		@ get control register v4
	ldr	r5, fa526_cr1_clear
	bic	r0, r0, r5
	ldr	r5, fa526_cr1_set
	orr	r0, r0, r5

#ifdef CONFIG_CPU_FA_BTB
	orr	r0, r0, #CR_Z
#else
	bic	r0, r0, #CR_Z
#endif
#ifdef CONFIG_CPU_FA_WB_DISABLE
	mov	r12, #0
	mcr	p15, 0, r12, c7, c10, 4		@ drain write buffer
	nop
	nop
	bic	r0, r0, #CR_W			@ .... .... .... 1...
#else
	orr	r0, r0, #CR_W
#endif

	mov	pc, lr
	.size	__fa526_setup, . - __fa526_setup

	/*
	 * .RVI ZFRS BLDP WCAM
	 * ..11 0001 .111 1101
	 *
	 */
	.type	fa526_cr1_clear, #object
	.type	fa526_cr1_set, #object
fa526_cr1_clear:
	.word	0x3f3f
fa526_cr1_set:
	.word	0x317D

	__INITDATA

/*
 * Purpose : Function pointers used to access above functions - all calls
 *	     come through these
 */
	.type	fa526_processor_functions, #object
fa526_processor_functions:
	.word	v4_early_abort
	.word	cpu_fa526_proc_init
	.word	cpu_fa526_proc_fin
	.word	cpu_fa526_reset
	.word   cpu_fa526_do_idle
	.word	cpu_fa526_dcache_clean_area
	.word	cpu_fa526_switch_mm
	.word	cpu_fa526_set_pte_ext
	.size	fa526_processor_functions, . - fa526_processor_functions

	.section ".rodata"

	.type	cpu_arch_name, #object
cpu_arch_name:
	.asciz	"armv4"
	.size	cpu_arch_name, . - cpu_arch_name

	.type	cpu_elf_name, #object
cpu_elf_name:
	.asciz	"v4"
	.size	cpu_elf_name, . - cpu_elf_name

	.type	cpu_fa526_name, #object
cpu_fa526_name:
	.ascii	"FA526"
#ifndef CONFIG_CPU_ICACHE_DISABLE
	.ascii	"i"
#endif
#ifndef CONFIG_CPU_DCACHE_DISABLE
	.ascii	"d"
#ifdef CONFIG_CPU_DCACHE_WRITETHROUGH
	.ascii	"(wt)"
#else
	.ascii	"(wb)"
#endif
#endif
	.ascii	"\0"
	.size	cpu_fa526_name, . - cpu_fa526_name

	.align

	.section ".proc.info.init", #alloc, #execinstr

#ifdef CONFIG_CPU_DCACHE_WRITETHROUGH
#define __PMD_SECT_BUFFERABLE	0
#else
#define __PMD_SECT_BUFFERABLE	PMD_SECT_BUFFERABLE
#endif

	.type	__fa526_proc_info,#object
__fa526_proc_info:
	.long	0x66015261
	.long	0xff01fff1
	.long   PMD_TYPE_SECT | \
		__PMD_SECT_BUFFERABLE | \
		PMD_SECT_CACHEABLE | \
		PMD_BIT4 | \
		PMD_SECT_AP_WRITE | \
		PMD_SECT_AP_READ
	.long   PMD_TYPE_SECT | \
		PMD_BIT4 | \
		PMD_SECT_AP_WRITE | \
		PMD_SECT_AP_READ
	b	__fa526_setup
	.long	cpu_arch_name
	.long	cpu_elf_name
	.long	HWCAP_SWP | HWCAP_HALF
	.long	cpu_fa526_name
	.long	fa526_processor_functions
	.long	fa_tlb_fns
	.long	fa_user_fns
	.long	fa_cache_fns
	.size	__fa526_proc_info, . - __fa526_proc_info


