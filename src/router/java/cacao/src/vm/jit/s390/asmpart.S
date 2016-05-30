/* src/vm/jit/s390/asmpart.S - Java-C interface functions for s390

   Copyright (C) 2006, 2007 R. Grafl, A. Krall, C. Kruegel,
   C. Oates, R. Obermaisser, M. Platter, M. Probst, S. Ring,
   E. Steiner, C. Thalinger, D. Thuernbeck, P. Tomsich, C. Ullrich,
   J. Wenninger, Institut f. Computersprachen - TU Wien

   This file is part of CACAO.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

*/


#include "config.h"

#include "vm/jit/s390/arch.h"
#include "vm/jit/s390/md-abi.h"
#include "vm/jit/s390/md-asm.h"

#include "vm/jit/abi-asm.hpp"
#include "vm/jit/methodheader.hpp"

	.text


/* export functions ***********************************************************/

	.globl asm_vm_call_method
	.globl asm_vm_call_method_int
	.globl asm_vm_call_method_long
	.globl asm_vm_call_method_float
	.globl asm_vm_call_method_double
	.globl asm_vm_call_method_exception_handler
	.globl asm_vm_call_method_end

	.globl asm_handle_exception
	.globl asm_handle_nat_exception

	.globl asm_abstractmethoderror

	.globl asm_builtin_f2i
	.globl asm_builtin_f2l
	.globl asm_builtin_d2i
	.globl asm_builtin_d2l


asm_abstractmethoderror:
	.long 0
asm_builtin_f2i:
	.long 0
asm_builtin_f2l:
	.long 0
asm_builtin_d2i:
	.long 0
asm_builtin_d2l:
	.long 0

/* Generates a PIC call.
 *
 * func: function to call
 * tag: tag unique for this call to generate a label
 * tmp: one temporary register needed for calculation
 *
 * The offset table used is located at the bottom of this file.
 *
 * Note: destroys r12. r12 MUST contain GOT for PIC calls!
 */
#define CALL_PIC(func, tag)                                                         \
	bras   %r14, L_bras_##tag;                           /* get PC */               \
L_bras_##tag:                                                                       \
	l      %r12, L_offsets - L_bras_##tag (%r14);        /* load offset to PLT */   \
	la     %r12, L_offsets - L_bras_##tag (%r12, %r14);  /* load PLT address */     \
	l      %r14, L_offset_##func - L_bras_##tag (%r14);  /* load offset to func */  \
	bas    %r14, 0(%r14, %r12);

/********************* function asm_calljavafunction ***************************
*                                                                              *
*   This function calls a Java-method (which possibly needs compilation)       *
*   with up to 4 address parameters.                                           *
*                                                                              *
*   This functions calls the JIT-compiler which eventually translates the      *
*   method into machine code.                                                  *
*                                                                              *
*   C-prototype:                                                               *
*    javaobject_header *asm_calljavamethod (methodinfo *m,                     *
*         void *arg1, void *arg2, void *arg3, void *arg4);                     *
*                                                                              *
*******************************************************************************/

	.long   0                         /* fltsave                              */
	.long   0                         /* intsave                              */
	.long   0                         /* IsLeaf                               */
	.long   0                         /* frame size                           */
	.long   0                         /* codeinfo pointer                     */

asm_vm_call_method:
asm_vm_call_method_int:
asm_vm_call_method_long:
asm_vm_call_method_float:
asm_vm_call_method_double:

	ahi sp, -8*4                /* allocate stack frame */

	/* a0: PV */
	/* a1: data structure */
	/* a2: number of stack arguments */

	st    s0, 0*4(sp)           /* store used calle saved registers */
	st    s1, 1*4(sp)
	st    a0, 2*4(sp)
	st    %r12, 3*4(sp)         /* %r12 is callee saved, we'll need it as GOT */
	st    pv, 4*4(sp)
	st    a4, 5*4(sp)           /* a4 is callee saved in terms of C abi */
	std   ftmp1, 6*4(sp)        /* ftmp1 and ftmp2 are callees saved in terms of C abi */
	std   ftmp2, 8*4(sp)
	st    ra, 10*4(sp)

	lr    s0, a1                /* data structure */
	lr    %r0, a2               /* number of stack arguments */

	l     a0, 0*8+4(s0)         /* big endian */
	l     a1, 1*8+4(s0)
	l     a2, 2*8+4(s0)
	l     a3, 3*8+4(s0)
	l     a4, 4*8+4(s0)         

	ld    fa0, 5*8(s0)
	ld    fa1, 6*8(s0)

	lr    s1, sp                /* backup stack pointer */

	ltr   %r0, %r0              /* are there any stack arguments ? */
	je    L_asm_vm_call_method_stack_copy_done
	lr    %r1, %r0              /* copy number of stack arguments */
	sll   %r1, 3                /* calculate stackframe size */ 
	sr    sp, %r1               /* allocate stack frame */
	lr    %r1, sp               /* temporary stack pointer */

L_asm_vm_call_method_stack_copy_loop:

	mvc   0(8, %r1), 7*8(s0)    /* copy argument */
	ahi   %r1, 8                /* increase sp */
	ahi   s0, 8                 /* set address of next argument */
	ahi   %r0, -1               /* substract 1 argument */
	jh    L_asm_vm_call_method_stack_copy_loop

L_asm_vm_call_method_stack_copy_done:

	la    mptr, 2*4(s1)         /* load method pointer */
	l     pv, 0(mptr)           /* load procedure vector from method pointer */
	basr  ra, pv                /* call method */

L_asm_vm_call_method_return:

	lr    sp, s1                /* restore stack pointer */
	l     s0, 0*4(sp)           /* restore used callee saved registers */
	l     s1, 1*4(sp)
	l     %r12, 3*4(sp)
	l     pv, 4*4(sp)
	l     a4, 5*4(sp)
	ld    ftmp1, 6*4(sp)
	ld    ftmp2, 8*4(sp)
	l     ra, 10*4(sp)

	ahi   sp, 8*4               /* remove stackframe */
	br    ra                    /* return */


asm_vm_call_method_exception_handler:
	lr    a0, xptr

	ahi   sp, -96
	CALL_PIC(builtin_throw_exception, avcmeh)
	ahi   sp, 96

	j     L_asm_vm_call_method_return

asm_vm_call_method_end:
	brc   0,0	

/* asm_handle_exception ********************************************************
*                                                                              *
*   This function handles an exception. It does not use the usual calling      *
*   conventions. The exception pointer is passed in REG_ITMP1 and the          *
*   pc from the exception raising position is passed in REG_ITMP2. It searches *
*   the local exception table for a handler. If no one is found, it unwinds    *
*   stacks and continues searching the callers.                                *
*                                                                              *
*******************************************************************************/

asm_handle_nat_exception:
L_asm_handle_nat_exception:
	/* TODO really nothing here ? */
asm_handle_exception:
L_asm_handle_exception:

	/* a wrapper for md_handle_exception */

#	define STACKFRAMESIZE (96 + (16 * 4) + (16 * 8) + (4 * 4))
#	define REGS 96
#	define FREGS (96 + (16 * 4))
#	define OUT (96 + (16 * 4) + (16 * 8))

	ahi   sp, -STACKFRAMESIZE   /* allocate stack frame containing the arrays */

	/* store special registers to array */

	st    xptr, REGS+(0*4)(sp)
	st    xpc, REGS+(1*4)(sp)
	st    pv, REGS+(13*4)(sp)
	la    itmp3, STACKFRAMESIZE(sp)
	st    itmp3, REGS+(15*4)(sp)

	/* store temporary and argument registers */

	stm   a0, a4, REGS+(2*4)(sp)
	std   %f0, FREGS+(0*8)(sp)
	std   %f1, FREGS+(1*8)(sp)
	std   %f2, FREGS+(2*8)(sp)
	std   %f3, FREGS+(3*8)(sp)
	std   %f5, FREGS+(5*8)(sp)
	std   %f7, FREGS+(7*8)(sp)
	std   %f8, FREGS+(8*8)(sp)
	std   %f9, FREGS+(9*8)(sp)
	std   %f10, FREGS+(10*8)(sp)
	std   %f11, FREGS+(11*8)(sp)
	std   %f12, FREGS+(12*8)(sp)
	std   %f13, FREGS+(13*8)(sp)
	std   %f14, FREGS+(14*8)(sp)
	std   %f15, FREGS+(15*8)(sp)

	/* store %r12 used as GOT */
	
	st    %r12, REGS+(12*4)(sp)

	/* call md_handle_exception */

	la    a0, REGS(sp)
	la    a1, FREGS(sp)
	la    a2, OUT(sp)

	CALL_PIC(md_handle_exception, ahe)

	/* restore %r12 */

	l     %r12, REGS+(12*4)(sp)

	l     itmp3, OUT+(2*4)(sp)  /* out[2] contains maybe leaf flag */
	ltr   itmp3, itmp3           
	je    L_restore_saved

L_restore_temporary_and_argument:

	/* if we are maybe leaf,
	 * we have to restore argument and temporary registers
	 */

	lm    a0, a4, REGS+(2*4)(sp)
	ld    %f0, FREGS+(0*8)(sp)
	ld    %f1, FREGS+(1*8)(sp)
	ld    %f2, FREGS+(2*8)(sp)
	ld    %f3, FREGS+(3*8)(sp)
	ld    %f5, FREGS+(5*8)(sp)
	ld    %f7, FREGS+(7*8)(sp)
	ld    %f8, FREGS+(8*8)(sp)
	ld    %f9, FREGS+(9*8)(sp)
	ld    %f10, FREGS+(10*8)(sp)
	ld    %f11, FREGS+(11*8)(sp)
	ld    %f12, FREGS+(12*8)(sp)
	ld    %f13, FREGS+(13*8)(sp)
	ld    %f14, FREGS+(14*8)(sp)
	ld    %f15, FREGS+(15*8)(sp)

	j     L_restore_done

L_restore_saved:

	/* if we are not a maybe leaf,
	 * we have to restore callee saved registers of the callee
	 */

	l     itmp3, OUT+(0*4)(sp)  /* out[0] contains IntSav */

	ahi   itmp3, -1
	jl    L_int_done
	l     s5, REGS+(12*4)(sp)

	ahi   itmp3, -1
	jl    L_int_done
	l     s4, REGS+(11*4)(sp)

	ahi   itmp3, -1
	jl    L_int_done
	l     s3, REGS+(10*4)(sp)

	ahi   itmp3, -1
	jl    L_int_done
	l     s2, REGS+(9*4)(sp)

	ahi   itmp3, -1
	jl    L_int_done
	l     s1, REGS+(8*4)(sp)

	ahi   itmp3, -1
	jl    L_int_done
	l     s0, REGS+(7*4)(sp)

L_int_done:

	/* restore callee saved float registers */

	/* there are currently none */

L_flt_done:

L_restore_done:

	/* write new values for special registers */

	l     xptr, REGS+(0*4)(sp)
	l     xpc, REGS+(1*4)(sp)
	l     pv, REGS+(13*4)(sp)  
	l     sp, REGS+(15*4)(sp)   

	br    xpc                   /* jump to handler */

#	undef STACKFRAMESIZE
#	undef REGS
#	undef FREGS
#	undef OUT

/* Offset table for PIC calls, see CALL_PIC */

L_offsets:
	.long  _GLOBAL_OFFSET_TABLE_ - L_offsets
L_offset_builtin_throw_exception:
	.long  builtin_throw_exception@PLTOFF
L_offset_md_handle_exception:
	.long  md_handle_exception@PLTOFF


/*
 * These are local overrides for various environment variables in Emacs.
 * Please do not remove this and leave it at the end of the file, where
 * Emacs will automagically detect them.
 * ---------------------------------------------------------------------
 * Local variables:
 * mode: asm
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:noexpandtab:sw=4:ts=4:
 */
