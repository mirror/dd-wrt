/* src/vm/jit/alpha/asmpart.S - Java-C interface functions for alpha

   Copyright (C) 1996-2012
   CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO

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

#include "vm/jit/alpha/md-abi.hpp"
#include "vm/jit/alpha/md-asm.hpp"

#include "vm/jit/abi-asm.hpp"
#include "vm/jit/methodheader.hpp"


	.text
	.set    noat
	.set    noreorder


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

	.globl asm_md_init
	.globl asm_cacheflush


/* asm_vm_call_method **********************************************************
*                                                                              *
*   This function calls a Java-method (which possibly needs compilation)       *
*   with up to 4 address parameters.                                           *
*                                                                              *
*   This functions calls the JIT-compiler which eventually translates the      *
*   method into machine code.                                                  *
*                                                                              *
*   C-prototype:                                                               *
*    javaobject_header *asm_calljavafunction (methodinfo *m,                   *
*         void *arg1, void *arg2, void *arg3, void *arg4);                     *
*                                                                              *
*******************************************************************************/

	.ent    asm_vm_call_method

	.align  3

	.long   0                           /* fltsave                            */
	.long   1                           /* intsave                            */
	.long   0                           /* isleaf                             */
	.long   0                           /* frame size                         */
	.quad   0                           /* codeinfo pointer                   */

asm_vm_call_method:
asm_vm_call_method_int:
asm_vm_call_method_long:
asm_vm_call_method_float:
asm_vm_call_method_double:
	ldgp    gp,0(pv)
	lda     sp,-5*8(sp)                 /* allocate stack space               */
	stq     ra,0*8(sp)                  /* save return address                */
	stq     gp,1*8(sp)                  /* save global pointer                */

	stq     s0,3*8(sp)
	stq     a0,4*8(sp)                  /* save method PV                     */

	mov     a1,t0                       /* address of argument array          */
	mov     a2,t1                       /* stack argument count               */
	mov     sp,s0                       /* save stack pointer                 */

	ldq     a0,0*8(t0)
	ldq     a1,1*8(t0)
	ldq     a2,2*8(t0)
	ldq     a3,3*8(t0)
	ldq     a4,4*8(t0)
	ldq     a5,5*8(t0)

	ldt     fa0,6*8(t0)
	ldt     fa1,7*8(t0)
	ldt     fa2,8*8(t0)
	ldt     fa3,9*8(t0)
	ldt     fa4,10*8(t0)
	ldt     fa5,11*8(t0)

	beq     t1,L_asm_vm_call_method_stack_copy_done

	negq    t1,t2                       /* calculate stackframe size (* 8)    */
	s8addq  t2,sp,sp                    /* create stackframe                  */
	mov     sp,t2                       /* temporary stack pointer            */

L_asm_vm_call_method_stack_copy_loop:
	ldq     t3,12*8(t0)                 /* load argument                      */
	stq     t3,0(t2)                    /* store argument on stack            */

	subq    t1,1,t1                     /* subtract 1 argument                */
	addq    t0,8,t0                     /* load address of next argument      */
	addq    t2,8,t2                     /* increase stack pointer             */

	bgt     t1,L_asm_vm_call_method_stack_copy_loop

L_asm_vm_call_method_stack_copy_done:
	lda     mptr,4*8(s0)                /* get address of PV                  */
	ldq     pv,0*8(mptr)                /* load PV                            */
	jmp     ra,(pv)
L_asm_vm_call_method_recompute_pv:
	lda     pv,(asm_vm_call_method - L_asm_vm_call_method_recompute_pv)(ra)

L_asm_vm_call_method_recompute_return:
	mov     s0,sp                       /* restore stack pointer              */

	ldq     ra,0*8(sp)                  /* restore RA                         */
	ldq     gp,1*8(sp)                  /* restore global pointer             */
	ldq     s0,3*8(sp)

	lda     sp,5*8(sp)                  /* free stack space                   */
	jmp     zero,(ra)

asm_vm_call_method_exception_handler:
	mov     s0,sp                       /* restore stack pointer              */
	ldq     gp,1*8(sp)                  /* restore global pointer             */
	mov     itmp1,a0
	jsr     ra,builtin_throw_exception

asm_vm_call_method_end:					
	br      L_asm_vm_call_method_recompute_return

	.end    asm_vm_call_method


/* asm_handle_exception ********************************************************

   This function handles an exception. It does not use the usual calling
   conventions. The exception pointer is passed in REG_ITMP1 and the
   pc from the exception raising position is passed in REG_ITMP2. It searches
   the local exception table for a handler. If no one is found, it unwinds
   stacks and continues searching the callers.

   ATTENTION: itmp3 == gp!

*******************************************************************************/

	.ent    asm_handle_nat_exception

asm_handle_nat_exception:
L_asm_handle_nat_exception:       /* required for PIC code                    */
L_asm_handle_exception_stack_loop:
	lda     sp,-6*8(sp)                 /* keep stack 16-byte aligned         */
	stq     xptr,0*8(sp)                /* save xptr                          */
	stq     xpc,1*8(sp)                 /* save xpc                           */
	stq     ra,3*8(sp)                  /* save RA                            */
	stq     zero,4*8(sp)                /* save maybe-leaf flag (cleared)     */

	mov     ra,a0                       /* pass RA                            */

	br      ra,L_asm_handle_exception_load_gp
L_asm_handle_exception_load_gp:
	ldgp    gp,0(ra)                    /* load gp                            */

	jsr     ra,md_asm_codegen_get_pv_from_pc /* get PV from RA                */
	stq     v0,2*8(sp)                  /* save PV                            */

	ldq     a0,0*8(sp)                  /* pass xptr                          */
	ldq     a1,1*8(sp)                  /* pass xpc                           */
	mov     v0,a2                       /* pass PV                            */
	addq    sp,6*8,a3                   /* pass Java SP                       */

	br      L_asm_handle_exception_continue

	.aent    asm_handle_exception

asm_handle_exception:
L_asm_handle_exception:                 /* required for PIC code              */
	lda     sp,-(ARG_CNT+TMP_CNT)*8(sp) /* create maybe-leaf stackframe       */

	SAVE_ARGUMENT_REGISTERS(0)          /* we save arg and temp registers in  */
	SAVE_TEMPORARY_REGISTERS(ARG_CNT)   /* case this is a leaf method         */

	lda     sp,-6*8(sp)                 /* keep stack 16-byte aligned         */
	stq     xptr,0*8(sp)                /* save xptr                          */
	stq     pv,2*8(sp)                  /* save PV                            */
	stq     ra,3*8(sp)                  /* save RA                            */
	lda     t0,1(zero)                  /* set maybe-leaf flag                */
	stq     t0,4*8(sp)                  /* save maybe-leaf flag               */

	br      ra,L_asm_handle_exception_load_gp_2
L_asm_handle_exception_load_gp_2:
	ldgp    gp,0(ra)                    /* load gp                            */

	mov     xptr,a0                     /* pass xptr                          */
	mov     xpc,a1                      /* pass xpc                           */
	mov     pv,a2                       /* pass PV                            */
	lda     a3,(ARG_CNT+TMP_CNT+6)*8(sp)/* pass Java SP                       */

L_asm_handle_exception_continue:
	jsr     ra,exceptions_handle_exception

	beq     v0,L_asm_handle_exception_not_catched

	mov     v0,xpc                      /* move handlerpc into xpc            */
	ldq     xptr,0*8(sp)                /* restore xptr                       */
	ldq     pv,2*8(sp)                  /* restore PV                         */
	ldq     ra,3*8(sp)                  /* restore RA                         */
	ldq     t0,4*8(sp)                  /* get maybe-leaf flag                */
	lda     sp,6*8(sp)                  /* free stack frame                   */

	beq     t0,L_asm_handle_exception_no_leaf

	RESTORE_ARGUMENT_REGISTERS(0)       /* if this is a leaf method, we have  */
	RESTORE_TEMPORARY_REGISTERS(ARG_CNT)/* to restore arg and temp registers  */
	
	lda     sp,(ARG_CNT+TMP_CNT)*8(sp)  /* remove maybe-leaf stackframe       */

L_asm_handle_exception_no_leaf:
	jmp     zero,(xpc)                  /* jump to the handler                */

L_asm_handle_exception_not_catched:
	ldq     xptr,0*8(sp)                /* restore xptr                       */
	ldq     pv,2*8(sp)                  /* restore PV                         */
	ldq     ra,3*8(sp)                  /* restore RA                         */
	ldq     t0,4*8(sp)                  /* get maybe-leaf flag                */
	lda     sp,6*8(sp)

	beq     t0,L_asm_handle_exception_no_leaf_stack

	lda     sp,(ARG_CNT+TMP_CNT)*8(sp)  /* remove maybe-leaf stackframe       */
	mov     zero,t0                     /* clear the maybe-leaf flag          */

L_asm_handle_exception_no_leaf_stack:
	ldl     t1,FrameSize(pv)            /* get frame size                     */
	addq    t1,sp,t1                    /* pointer to save area               */

	ldl     t2,IsLeaf(pv)               /* is leaf procedure                  */
	bne     t2,L_asm_handle_exception_no_ra_restore

	ldq     ra,-1*8(t1)                 /* restore ra                         */
	subq    t1,8,t1                     /* t1--                               */

L_asm_handle_exception_no_ra_restore:
	mov     ra,xpc                      /* the new xpc is ra                  */
	ldl     t2,IntSave(pv)              /* t2 = saved int register count      */
	br      t3,ex_int1                  /* t3 = current pc                    */
ex_int1:
	lda     t3,(ex_int2 - ex_int1)(t3)
	negl    t2,t2                       /* negate register count              */
	s4addq  t2,t3,t3                    /* t2 = IntSave - register count * 4  */
	jmp     zero,(t3)                   /* jump to save position              */

	ldq     s0,-7*8(t1)
	ldq     s1,-6*8(t1)
	ldq     s2,-5*8(t1)
	ldq     s3,-4*8(t1)
	ldq     s4,-3*8(t1)
	ldq     s5,-2*8(t1)
	ldq     s6,-1*8(t1)

ex_int2:
	s8addq  t2,t1,t1                    /* t1 = t1 - 8 * register count       */

	ldl     t2,FltSave(pv)              /* t2 = saved flt register count      */
	br      t3,ex_flt1                  /* t3 = current pc                    */
ex_flt1:
	lda     t3,(ex_flt2 - ex_flt1)(t3)
	negl    t2,t2                       /* negate register count              */
	s4addq  t2,t3,t3                    /* t2 = FltSave - 4 * register count  */
	jmp     zero,(t3)                   /* jump to save position              */

	ldt     fs0,-8*8(t1)
	ldt     fs1,-7*8(t1)
	ldt     fs2,-6*8(t1)
	ldt     fs3,-5*8(t1)
	ldt     fs4,-4*8(t1)
	ldt     fs5,-3*8(t1)
	ldt     fs6,-2*8(t1)
	ldt     fs7,-1*8(t1)

ex_flt2:
	ldl     t1,FrameSize(pv)            /* get frame size                     */
	addq    sp,t1,sp                    /* unwind stack                       */
	br      L_asm_handle_exception_stack_loop

	.end    asm_handle_nat_exception


/* asm_abstractmethoderror *****************************************************

   Creates and throws an AbstractMethodError.

*******************************************************************************/

	.ent    asm_abstractmethoderror

asm_abstractmethoderror:
	ldgp    gp,0(t12)                   /* load gp                            */
	subq    sp,2*8,sp                   /* create stackframe                  */
	stq     ra,0*8(sp)                  /* save return address                */
	addq    sp,2*8,a0                   /* pass java sp                       */
	mov     ra,a1                       /* pass exception address             */
	jsr     ra,exceptions_asm_new_abstractmethoderror
	ldq     ra,0*8(sp)                  /* restore return address             */
	addq    sp,2*8,sp                   /* remove stackframe                  */

	mov     v0,xptr                     /* get exception pointer              */
	subq    ra,4,xpc                    /* exception address is ra - 4        */
	br      L_asm_handle_nat_exception

	.end    asm_abstractmethoderror


/* asm_md_init *****************************************************************

   Initialize machine dependent stuff.

   Determines if the byte support instruction set (21164a and higher)
   is available.

*******************************************************************************/

	.ent    asm_md_init

asm_md_init:
	.long   0x47e03c20                  /* amask   1,v0                       */
	jmp     zero,(ra)                   /* return                             */

	.end    asm_md_init


/* asm_cacheflush **************************************************************

   XXX

*******************************************************************************/

	.ent    asm_cacheflush

asm_cacheflush:
	call_pal PAL_imb              /* synchronize instruction cache            */
	jmp     zero,(ra)

	.end    asm_cacheflush


/* disable exec-stacks ********************************************************/

#if defined(__linux__) && defined(__ELF__)
	.section .note.GNU-stack,"",%progbits
#endif


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
