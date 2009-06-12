//=============================================================================
//
//      irq.S - Cyclone Diagnostics
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   Scott Coulter, Jeff Frazier, Eric Breeden
// Contributors:
// Date:        2001-01-25
// Purpose:     
// Description: 
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

/*
 * Low-lebvel interrupt support for IQ80310 diags
 */
	.extern iq80310_irq_handler
	.extern iq80310_fiq_handler

	.text
	.globl __diag_IRQ
  __diag_IRQ:
	ldr	sp, =__irq_stack    /* point stack pointer at IRQ stack */
	sub	lr, lr, #4 	   /* adjust lr (return to last address) */
	stmfd	sp!, {r12, lr}     /* push r12 and link reg onto stack */
	mrs	r12, spsr	   /* store spsr in r12 */
	stmfd	sp!, {r12}	   /* push spsr onto stack */
	stmfd	sp!, {r0-r11}  	   /* push all registers onto stack */
	bl	iq80310_irq_handler
	ldmfd	sp!, {r0-r12} 	   /* restore r0 thru r12 */
	msr	spsr, r12	   /* restore SPSR */
	ldmfd	sp!, {r12,pc}^	   /* restore r12 and PC, return */

	.globl __diag_FIQ
  __diag_FIQ:
	/* Cyclone FIQ handler */
	/* save registers onto stack */
	ldr     sp, =__fiq_stack	/* point stack pointer at FIQ stack */
	sub	lr, lr, #4	/* adjust link register (return to last address) */
	stmfd	sp!, {r12, lr}	/* push r12 and link reg onto stack */
	mrs	r12, spsr	/* store spsr in r12 */
	stmfd	sp!, {r12}	/* push spsr onto stack */
	stmfd	sp!, {r0-r7}	/* push r0 thru r7 (r8 - r14 are banked) */
	bl      iq80310_fiq_handler
	/* restore registers and return */
	ldmfd	sp!, {r0-r7}	/* restore r0 thru r7 (r8 - r14 are banked) */
	ldmfd	sp!, {r12}
	msr	spsr, r12	/* restore SPSR */
	ldmfd	sp!, {r12,pc}^	/* restore r12 and PC, return to inst before exception occurred */

	.globl __ignore_abort
__ignore_abort:
	subs pc,lr,#4

	.globl _cspr_enable_fiq_int
_cspr_enable_fiq_int:
	mrs	r0, cpsr
	bic	r0, r0, #0x40
	msr	cpsr, r0
	mov     pc, lr

	.globl _cspr_enable_irq_int
_cspr_enable_irq_int:
	mrs	r0, cpsr
	bic	r0, r0, #0x80
	msr	cpsr, r0
	mov     pc, lr

	.bss
	.rept 1024
	.word 0
	.endr
  __irq_stack:
	.rept 1024
	.word 0
	.endr
  __fiq_stack:


