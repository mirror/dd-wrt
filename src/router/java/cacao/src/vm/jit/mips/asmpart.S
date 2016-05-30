/* src/vm/jit/mips/asmpart.S - Java-C interface functions for MIPS

   Copyright (C) 1996-2005, 2006, 2007, 2008
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

#include "vm/jit/mips/md-abi.hpp"
#include "vm/jit/mips/md-asm.hpp"

#include "vm/jit/abi-asm.hpp"
#include "vm/jit/methodheader.hpp"


	.text
	.set    noat


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


/* asm_vm_call_method **********************************************************
*                                                                              *
*   This function calls a Java-method (which possibly needs compilation)       *
*   with up to 4 address parameters.                                           *
*                                                                              *
*   This functions calls the JIT-compiler which eventually translates the      *
*   method into machine code.                                                  *
*                                                                              *
*   A possibly throwed exception will be returned to the caller as function    *
*   return value, so the java method cannot return a fucntion value (this      *
*   function usually calls 'main' and '<clinit>' which do not return a         *
*   function value).                                                           *
*                                                                              *
*   C-prototype:                                                               *
*    javaobject_header *asm_calljavafunction (methodinfo *m,                   *
*         void *arg1, void *arg2, void *arg3, void *arg4);                     *
*                                                                              *
*******************************************************************************/

	.ent    asm_vm_call_method

	.align  3

#if SIZEOF_VOID_P == 8

	.word   0                           /* fltsave                            */
	.word   0                           /* intsave                            */
	.word   0                           /* isleaf                             */
	.word   0                           /* frame size                         */
	.dword  0                           /* codeinfo pointer                   */

#else /* SIZEOF_VOID_P == 8 */

	.word   0                           /* fltsave                            */
	.word   0                           /* intsave                            */
	.word   0                           /* isleaf                             */
	.word   0                           /* frame size                         */
	.word   0                           /* method pointer (pointer to name)   */

#endif /* SIZEOF_VOID_P == 8 */

asm_vm_call_method:
asm_vm_call_method_int:
asm_vm_call_method_long:
asm_vm_call_method_float:
asm_vm_call_method_double:
	.set    noreorder                 /* XXX we need to recompute pv          */

	aaddiu  sp,sp,-12*8               /* allocate stack space (only 11 needed)*/
	ast     ra,0*8(sp)                /* save return address                  */

	bal     L_asm_vm_call_method_compute_pv
	ast     pv,1*8(sp)                /* procedure vector                     */
L_asm_vm_call_method_compute_pv:
	aaddiu  pv,ra,-4*4

	ast     s0,3*8(sp)                /* save callee saved register           */
	ast     a0,4*8(sp)                /* save method PV                       */

#if SIZEOF_VOID_P == 8
	s.d     fss0,5*8(sp)              /* save non JavaABI saved flt registers */
	s.d     fss1,6*8(sp)
	s.d     fss2,7*8(sp)
	s.d     fss3,8*8(sp)
	s.d     fss4,9*8(sp)
	s.d     fss5,10*8(sp)
#endif

	move    t0,a1                     /* address of data structure            */
	move    t1,a2                     /* stack argument count                 */
	move    s0,sp                     /* save stack pointer                   */

#if SIZEOF_VOID_P == 8

	ld      a0,0*8(t0)
	ld      a1,1*8(t0)
	ld      a2,2*8(t0)
	ld      a3,3*8(t0)
	ld      a4,4*8(t0)
	ld      a5,5*8(t0)
	ld      a6,6*8(t0)
	ld      a7,7*8(t0)

	l.d     fa0,8*8(t0)
	l.d     fa1,9*8(t0)
	l.d     fa2,10*8(t0)
	l.d     fa3,11*8(t0)
	l.d     fa4,12*8(t0)
	l.d     fa5,13*8(t0)
	l.d     fa6,14*8(t0)
	l.d     fa7,15*8(t0)

#else /* SIZEOF_VOID_P == 8 */

# if WORDS_BIGENDIAN == 1
	lw      a0,0*8+4(t0)
	lw      a1,1*8+4(t0)
	lw      a2,2*8+4(t0)
	lw      a3,3*8+4(t0)
# else
	lw      a0,0*8(t0)
	lw      a1,1*8(t0)
	lw      a2,2*8(t0)
	lw      a3,3*8(t0)
# endif

# if !defined(ENABLE_SOFT_FLOAT)
	l.d     fa0,4*8(t0)
	l.d     fa1,5*8(t0)
# endif

#endif /* SIZEOF_VOID_P == 8 */

	beqz    t1,L_asm_vm_call_method_stack_copy_done
	nop

	sll     t2,t1,3                   /* calculate stackframe size (* 8)      */
	asubu   sp,sp,t2                  /* create stackframe                    */
	move    t2,sp                     /* temporary stack pointer              */

L_asm_vm_call_method_stack_copy_loop:
#if SIZEOF_VOID_P == 8
	ld      t3,16*8(t0)               /* load argument                        */
	sd      t3,0(t2)                  /* store argument on stack              */
#else
# if !defined(ENABLE_SOFT_FLOAT)
	lw      t3,6*8+0(t0)              /* load argument                        */
	lw      t4,6*8+4(t0)
	sw      t3,0(t2)                  /* store argument on stack              */
	sw      t4,4(t2)
# else
#  error implement me
# endif
#endif

	aaddi   t1,t1,-1                  /* subtract 1 argument                  */
	aaddi   t0,t0,8                   /* load address of next argument        */
	aaddi   t2,t2,8                   /* increase stack pointer               */

	bgtz    t1,L_asm_vm_call_method_stack_copy_loop
	nop

L_asm_vm_call_method_stack_copy_done:
	ala     mptr,4*8(s0)              /* get address of PV                    */
	ald     pv,0*8(mptr)              /* load PV                              */
	jalr    pv
	nop
L_asm_vm_call_method_recompute_pv:
#if SIZEOF_VOID_P == 8
	aaddiu  pv,ra,-76*4               /* recompute procedure vector           */
#else
	aaddiu  pv,ra,(asm_vm_call_method - L_asm_vm_call_method_recompute_pv)
#endif

	.set    reorder                   /* XXX we need to recompute pv          */

calljava_return2:
	move    sp,s0                     /* restore stack pointer                */

	ald     ra,0*8(sp)                /* restore return address               */
	ald     pv,1*8(sp)                /* restore procedure vector             */
	ald     s0,3*8(sp)

#if SIZEOF_VOID_P == 8
	l.d     fss0,5*8(sp)              /* restore non JavaABI saved flt regs   */
	l.d     fss1,6*8(sp)
	l.d     fss2,7*8(sp)
	l.d     fss3,8*8(sp)
	l.d     fss4,9*8(sp)
	l.d     fss5,10*8(sp)
#endif

	aaddiu  sp,sp,12*8                /* free stack space                     */
	j       ra                        /* return                               */

asm_vm_call_method_exception_handler:
	move    sp,s0                     /* restore stack pointer                */
#if SIZEOF_VOID_P == 4
	aaddiu  sp,sp,-4*4                /* reserve space for 1 argument         */
#endif

	move    a0,itmp1                  
	jal     builtin_throw_exception
#if SIZEOF_VOID_P == 4
	aaddiu  sp,sp,4*4
#endif
asm_vm_call_method_end:
	b       calljava_return2

	.end    asm_vm_call_method


/* asm_handle_exception ********************************************************

   This function handles an exception. It does not use the usual calling
   conventions. The exception pointer is passed in REG_ITMP1 and the
   pc from the exception raising position is passed in REG_ITMP2. It searches
   the local exception table for a handler. If no one is found, it unwinds
   stacks and continues searching the callers.

*******************************************************************************/

	.ent    asm_handle_nat_exception

asm_handle_nat_exception:
L_asm_handle_exception_stack_loop:
#if SIZEOF_VOID_P == 8
	aaddiu  sp,sp,-6*8                  /* keep stack 16-byte aligned         */
	ast     xptr,0*8(sp)                /* save exception pointer             */
	ast     xpc,1*8(sp)                 /* save exception pc                  */
	ast     ra,3*8(sp)                  /* save RA                            */
	ast     zero,4*8(sp)                /* save maybe-leaf flag (cleared)     */
#else
	aaddiu  sp,sp,-(4*4+6*8)            /* allocate stack                     */
	ast     xptr,4*4+0*8(sp)            /* save exception pointer             */
	ast     xpc,4*4+1*8(sp)             /* save exception pc                  */
	ast     ra,4*4+3*8(sp)              /* save return address                */
	ast     zero,4*4+4*8(sp)            /* save maybe-leaf flag (cleared)     */
#endif

	move    a0,ra                       /* pass RA                            */
	jal     md_asm_codegen_get_pv_from_pc /* get PV from RA                   */

#if SIZEOF_VOID_P == 8
	ast     v0,2*8(sp)                  /* save PV                            */

	ald     a0,0*8(sp)                  /* pass xptr                          */
	ald     a1,1*8(sp)                  /* pass xpc                           */
	move    a2,v0                       /* pass PV                            */
	aaddiu  a3,sp,6*8                   /* pass Java SP                       */
#else
	ast     v0,4*4+2*8(sp)              /* save data segment pointer          */

	ald     a0,4*4+0*8(sp)              /* pass exception pointer             */
	ald     a1,4*4+1*8(sp)              /* pass exception pc                  */
	move    a2,v0                       /* pass data segment pointer          */
	aaddiu  a3,sp,(4*4+6*8)             /* pass Java stack pointer            */
#endif

	b       L_asm_handle_exception_continue

	.aent    asm_handle_exception

asm_handle_exception:
	aaddiu  sp,sp,-(ARG_CNT+TMP_CNT)*8  /* create maybe-leaf stackframe       */

	SAVE_ARGUMENT_REGISTERS(0)          /* we save arg and temp registers in  */
	SAVE_TEMPORARY_REGISTERS(ARG_CNT)   /* case this is a leaf method         */

#if SIZEOF_VOID_P == 8
	aaddiu  sp,sp,-6*8                  /* allocate stack                     */
	ast     xptr,0*8(sp)                /* save exception pointer             */
	ast     pv,2*8(sp)                  /* save PV                            */
	ast     ra,3*8(sp)                  /* save RA                            */
	addu    t0,zero,1                   /* set maybe-leaf flag                */
	ast     t0,4*8(sp)                  /* save maybe-leaf flag               */
#else
	aaddiu  sp,sp,-(4*4+6*8)            /* allocate stack                     */
	ast     xptr,4*4+0*8(sp)            /* save exception pointer             */
	ast     xpc,4*4+1*8(sp)             /* save exception pc                  */
	ast     pv,4*4+2*8(sp)              /* save data segment pointer          */
	ast     ra,4*4+3*8(sp)              /* save return address                */
	addu    t0,zero,1                   /* set maybe-leaf flag                */
	ast     t0,4*4+4*8(sp)              /* save maybe-leaf flag               */
#endif

	move    a0,xptr                     /* pass xptr                          */
	move    a1,xpc                      /* pass xpc                           */
	move    a2,pv                       /* pass PV                            */

#if SIZEOF_VOID_P == 8
	aaddiu  a3,sp,(ARG_CNT+TMP_CNT+6)*8 /* pass Java SP                       */
#else
	aaddiu  a3,sp,4*4+(ARG_CNT+TMP_CNT+6)*8 /* pass Java stack pointer        */
#endif

L_asm_handle_exception_continue:
	jal     exceptions_handle_exception
	
	beqz    v0,L_asm_handle_exception_not_catched

	move    xpc,v0                      /* move handlerpc into xpc            */

#if SIZEOF_VOID_P == 8
	ald     xptr,0*8(sp)                /* restore exception pointer          */
	ald     pv,2*8(sp)                  /* restore PV                         */
	ald     ra,3*8(sp)                  /* restore RA                         */
	ald     t0,4*8(sp)                  /* get maybe-leaf flag                */
	aaddiu  sp,sp,6*8                   /* free stackframe                    */
#else
	ald     xptr,4*4+0*8(sp)            /* restore exception pointer          */
	ald     pv,4*4+2*8(sp)              /* restore data segment pointer       */
	ald     ra,4*4+3*8(sp)              /* restore return address             */
	ald     t0,4*4+4*8(sp)              /* get maybe-leaf flag                */
	aaddiu  sp,sp,4*4+6*8               /* free stackframe                    */
#endif
	
	beqz    t0,L_asm_handle_exception_no_leaf

	RESTORE_ARGUMENT_REGISTERS(0)       /* if this is a leaf method, we have  */
	RESTORE_TEMPORARY_REGISTERS(ARG_CNT)/* to restore arg and temp registers  */
	
	aaddiu  sp,sp,(ARG_CNT+TMP_CNT)*8   /* remove maybe-leaf stackframe       */

L_asm_handle_exception_no_leaf:
	jr      xpc                         /* jump to the handler                */

L_asm_handle_exception_not_catched:
#if SIZEOF_VOID_P == 8
	ald     xptr,0*8(sp)                /* restore xptr                       */
	ald     pv,2*8(sp)                  /* restore PV                         */
	ald     ra,3*8(sp)                  /* restore RA                         */
	ald     t0,4*8(sp)                  /* get maybe-leaf flag                */
	aaddiu  sp,sp,6*8                   /* free stackframe                    */
#else
	ald     xptr,4*4+0*8(sp)            /* restore xptr                       */
	ald     pv,4*4+2*8(sp)              /* restore PV                         */
	ald     ra,4*4+3*8(sp)              /* restore RA                         */
	ald     t0,4*4+4*8(sp)              /* get maybe-leaf flag                */
	aaddiu  sp,sp,4*4+6*8               /* free stackframe                    */
#endif
	
	beqz    t0,L_asm_handle_exception_no_leaf_stack

	aaddiu  sp,sp,(ARG_CNT+TMP_CNT)*8   /* remove maybe-leaf stackframe       */
	move    t0,zero                     /* clear the maybe-leaf flag          */

L_asm_handle_exception_no_leaf_stack:
	lw      t1,FrameSize(pv)            /* get frame size                     */
	aaddu   t1,sp,t1                    /* pointer to save area               */

	lw      t2,IsLeaf(pv)               /* is leaf procedure                  */
	bnez    t2,L_asm_handle_exception_no_ra_restore

	ald     ra,-1*8(t1)                 /* restore ra                         */
	aaddiu  t1,t1,-8                    /* t1--                               */

L_asm_handle_exception_no_ra_restore:
	move    xpc,ra                      /* the new xpc is ra                  */
	lw      t2,IntSave(pv)              /* t1 = saved int register count      */
	ala     t3,ex_int2                  /* t3 = current pc                    */
	sll     t2,t2,2                     /* t2 = register count * 4            */
	asubu   t3,t3,t2                    /* t3 = IntSave - 4 * register count  */
	jr      t3                          /* jump to save position              */

	ald     s0,-8*8(t1)
	ald     s1,-7*8(t1)
	ald     s2,-6*8(t1)
	ald     s3,-5*8(t1)
	ald     s4,-4*8(t1)
	ald     s5,-3*8(t1)
	ald     s6,-2*8(t1)
	ald     s7,-1*8(t1)

ex_int2:
	sll     t2,t2,1               /* t2 = register count * 4 * 2              */
	asubu   t1,t1,t2              /* t1 = t0 - 8 * register count             */

	lw      t2,FltSave(pv)        /* t2 = saved flt register count            */
	ala     t3,ex_flt2            /* t3 = current pc                          */
	sll     t2,t2,2               /* t2 = register count * 4                  */
	asubu   t3,t3,t2              /* t3 = ex_int_sav - 4 * register count     */
	jr      t3                          /* jump to save position              */

#if SIZEOF_VOID_P == 8
	l.d     fs0,-4*8(t1)
	l.d     fs1,-3*8(t1)
	l.d     fs2,-2*8(t1)
	l.d     fs3,-1*8(t1)
#else
# if !defined(ENABLE_SOFT_FLOAT)
	l.d     fs0,-6*8(t1)
	l.d     fs1,-5*8(t1)
	l.d     fs2,-4*8(t1)
	l.d     fs3,-3*8(t1)
	l.d     fs4,-2*8(t1)
	l.d     fs5,-1*8(t1)
# endif
#endif

ex_flt2:
	lw      t1,FrameSize(pv)            /* get frame size                     */
	aaddu   sp,sp,t1                    /* unwind stack                       */
	b       L_asm_handle_exception_stack_loop

	.end    asm_handle_nat_exception


/* asm_abstractmethoderror *****************************************************

   Creates and throws an AbstractMethodError.

*******************************************************************************/

	.ent    asm_abstractmethoderror

asm_abstractmethoderror:
	aaddiu  sp,sp,-2*8                  /* create stackframe                  */
	ast     ra,0*8(sp)                  /* save return address                */
	aaddiu  a0,sp,2*8                   /* pass java sp                       */
	move    a1,ra                       /* pass exception address             */
	jal     exceptions_asm_new_abstractmethoderror
	ald     ra,0*8(sp)                  /* restore return address             */
	aaddiu  sp,sp,2*8                   /* remove stackframe                  */

	move    xptr,v0                     /* get exception pointer              */
	aaddiu  xpc,ra,-4                   /* exception address is ra - 4        */
	b       asm_handle_nat_exception

	.end    asm_abstractmethoderror


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
