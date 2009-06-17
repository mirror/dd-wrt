/*
 * head.S
 * originally written for xscale based linux kernel decompression
 * now adapted for AR531X based redboot stub and kernel loader
 * copyright 2009 Sebastian Gottschall / NewMedia-NET GmbH / DD-WRT.COM
 * licensed under GPL conditions
 */

#include <linux/autoconf.h>
#include <linux/init.h>
#include <linux/threads.h>

#include <asm/addrspace.h>
#include <asm/asm.h>
#include <asm/asmmacro.h>
#include <asm/irqflags.h>
#include <asm/regdef.h>
#include <asm/page.h>
#include <asm/mipsregs.h>
#include <asm/stackframe.h>


#define DECOMPRESS_STACK_SIZE	0x2000

/*
 * Use a magic section here to allow each board to insert its own code into
 * the start sequence...
 */

	.section	".start"
	.align

start:

	/* Board specific start code will be inserted here by linker */

/*
 * Switch to the standard text section now for the decompression stuff
 */
	.text
	.set	noreorder

LEAF(main)
	/* Clear BSS */

	la	t0, __bss_start
	sw	zero, (t0)
	la	t1, (_end - 4)
1:
	addiu	t0, 4
	bne	t0, t1, 1b
	sw	zero, (t0)

	/* Setup a stack pointer */

	la	t0, decompress_stack
	addiu	t0, t0, DECOMPRESS_STACK_SIZE - 32
	subu	sp, t0, 4*SZREG

	/* Call the decompression code */
	
	la	a0, _load_addr
	la	a1, _end
	la	a2, (_end + 0x100000)
	lui	t0, 0xfff0
	jal	decompress_kernel
	and	a2,a2,t0			/* DELAY SLOT */

	/* Jump to the kernel's entry point */
	la	t1, bootoffset  
	nop
	lw	t0, 0(t1)
	nop
	j	t0 
	nop
	END(main)

	/* Stack space in BSS */
	
	.comm	decompress_stack, DECOMPRESS_STACK_SIZE, 32
