/*
 *  linux/arch/arm/lib/copypage-fa.S
 *
 *  Copyright (C) 2005 Faraday Corp.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  ASM optimised string functions
 * 05/18/2005 :	Luke Lee created, modified from copypage-v4wb.S
 */
#include <linux/linkage.h>
#include <linux/init.h>
#include <asm/asm-offsets.h>

	.text
/*
 * ARMv4 optimised copy_user_page for Faraday processors
 *
 * We flush the destination cache lines just before we write the data into the
 * corresponding address.  Since the Dcache is read-allocate, this removes the
 * Dcache aliasing issue.  The writes will be forwarded to the write buffer,
 * and merged as appropriate.
 *
 * Note: We rely on all ARMv4 processors implementing the "invalidate D line"
 * instruction.  If your processor does not supply this, you have to write your
 * own copy_user_page that does the right thing.
 *
 * copy_user_page(to,from,vaddr)
 */
	.align	4
ENTRY(fa_copy_user_page)
#ifdef CONFIG_CPU_DCACHE_WRITETHROUGH
	/* Write through */
	stmfd	sp!, {r4, lr}			@ 2
	mov	r2, #PAGE_SZ/32			@ 1

	ldmia	r1!, {r3, r4, ip, lr}		@ 4
1:	stmia	r0!, {r3, r4, ip, lr}		@ 4
	ldmia	r1!, {r3, r4, ip, lr}		@ 4+1
	subs	r2, r2, #1			@ 1
	stmia	r0!, {r3, r4, ip, lr}		@ 4
	ldmneia	r1!, {r3, r4, ip, lr}		@ 4
	bne	1b				@ 1

	mcr	p15, 0, r2, c7, c7, 0		@ flush ID cache
	ldmfd	sp!, {r4, pc}			@ 3
#else
	/* Write back */
	stmfd	sp!, {r4, lr}			@ 2
	mov	r2, #PAGE_SZ/32			@ 1

1:	ldmia	r1!, {r3, r4, ip, lr}		@ 4
	mcr	p15, 0, r0, c7, c6, 1		@ 1   invalidate D line
	stmia	r0!, {r3, r4, ip, lr}		@ 4
	ldmia	r1!, {r3, r4, ip, lr}		@ 4
	mcr	p15, 0, r0, c7, c6, 1		@ 1   invalidate D line
	stmia	r0!, {r3, r4, ip, lr}		@ 4
	subs	r2, r2, #1			@ 1
	bne	1b
	mcr	p15, 0, r2, c7, c10, 4		@ 1   drain WB
	ldmfd	sp!, {r4, pc}			@ 3
#endif

/*
 * ARMv4 optimised clear_user_page
 *
 * Same story as above.
 */
	.align	4
ENTRY(fa_clear_user_page)
	str	lr, [sp, #-4]!
	mov	r1, #PAGE_SZ/32			@ 1
	mov	r2, #0				@ 1
	mov	r3, #0				@ 1
	mov	ip, #0				@ 1
	mov	lr, #0				@ 1
#ifdef CONFIG_CPU_DCACHE_WRITETHROUGH
	/* Write through */
1:	stmia	r0!, {r2, r3, ip, lr}		@ 4
	stmia	r0!, {r2, r3, ip, lr}		@ 4
	subs	r1, r1, #1			@ 1
	bne	1b				@ 1

	mcr	p15, 0, r1, c7, c7, 0		@ flush ID cache
	ldr	pc, [sp], #4
#else
	/* Write back */
1:	mcr	p15, 0, r0, c7, c6, 1		@ 1   invalidate D line
	stmia	r0!, {r2, r3, ip, lr}		@ 4
	mcr	p15, 0, r0, c7, c6, 1		@ 1   invalidate D line
	stmia	r0!, {r2, r3, ip, lr}		@ 4
	subs	r1, r1, #1			@ 1
	bne	1b				@ 1
	mcr	p15, 0, r1, c7, c10, 4		@ 1   drain WB
	ldr	pc, [sp], #4
#endif

	__INITDATA

	.type	fa_user_fns, #object
ENTRY(fa_user_fns)
	.long	fa_clear_user_page
	.long	fa_copy_user_page
	.size	fa_user_fns, . - fa_user_fns
