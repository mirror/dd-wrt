/*
 *
 * Platform specific tweaks.  This is merged into head.S by the linker.
 *
 */

#include <linux/linkage.h>
#include <asm/assembler.h>
#include <asm/cp15.h>

		.section        ".start", "ax"

/*
 * This code section is spliced into the head code by the linker
 */

__plat_uncompress_start:

	@ Preserve r8/r7 i.e. kernel entry values
	mov	r12, r8

	@ Clear MMU enable and Dcache enable bits
	mrc	p15, 0, r0, c1, c0, 0		@ Read SCTLR
	bic	r0, #CR_C|CR_M
	mcr	p15, 0, r0, c1, c0, 0		@ Write SCTLR
	nop

	@ Call the cache invalidation routine
	bl	v7_flush_dcache_all
	nop
	mov	r0,#0
	ldr	r3, =0x19022000			@ L2 cache controller, control reg
	str	r0, [r3, #0x100]		@ Disable L2 cache
	nop

	@ Restore
	mov	r8, r12
