/* src/vm/jit/x86_64/asmpart.S - Java-C interface functions for x86_64

   Copyright (C) 1996-2013
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

#include "vm/jit/x86_64/arch.hpp"
#include "vm/jit/x86_64/md-abi.hpp"
#include "md-asm.hpp"

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

	.align	8

	.long   0                           /* fltsave                            */
	.long   0                           /* intsave                            */
	.long   0                           /* isleaf                             */
	.long   0                           /* frame size                         */
	.quad   0                           /* codeinfo pointer                   */

asm_vm_call_method:
asm_vm_call_method_int:
asm_vm_call_method_long:
asm_vm_call_method_float:
asm_vm_call_method_double:
	sub     $(7*8),sp                   /* keep stack 16-byte aligned         */
	mov     %rbx,0*8(sp)                /* %rbx is not a callee saved in cacao*/
	mov     s0,1*8(sp)
	mov     s1,2*8(sp)
	mov     s2,3*8(sp)
	mov     s3,4*8(sp)
	mov     s4,5*8(sp)

	mov     a0,6*8(sp)                  /* store method PV                    */

	mov     sp,s0                       /* save stack pointer                 */

	mov     a1,t0                       /* address of data structure          */
	mov     a2,itmp1                    /* number of stack arguments          */

	mov     0*8(t0),a0
	mov     1*8(t0),a1
	mov     2*8(t0),a2
	mov     3*8(t0),a3
	mov     4*8(t0),a4
	mov     5*8(t0),a5

	movq    6*8(t0),fa0
	movq    7*8(t0),fa1
	movq    8*8(t0),fa2
	movq    9*8(t0),fa3
	movq    10*8(t0),fa4
	movq    11*8(t0),fa5
	movq    12*8(t0),fa6
	movq    13*8(t0),fa7

	cmp     $0,itmp1l
	je      L_asm_vm_call_method_stack_copy_done

	mov     itmp1,itmp2
	add     $1,itmp2                    /* keep stack 16-byte aligned         */
	and     $0xfffffffffffffffe,itmp2
	shl     $3,itmp2                    /* calculate stack size               */
	sub     itmp2,sp                    /* create stack frame                 */
	mov     sp,itmp2                    /* temporary stack pointer            */

L_asm_vm_call_method_stack_copy_loop:
	mov     14*8(t0),itmp3              /* load argument                      */
	mov     itmp3,0(itmp2)              /* store argument on stack            */

	sub     $1,itmp1l                   /* subtract 1 argument                */
	add     $8,t0                       /* set address of next argument       */
	add     $8,itmp2                    /* increase SP                        */

	cmp     $0,itmp1l
	jg      L_asm_vm_call_method_stack_copy_loop

L_asm_vm_call_method_stack_copy_done:
	lea     (6*8-256)(s0),mptr          /* We subtract 256 to force the next  */
	                                    /* move instruction to have a 32-bit  */
	                                    /* offset.                            */

	mov     (0*8+256)(mptr),itmp3       /* load PV                            */
	call    *itmp3

L_asm_vm_call_method_return:
	mov     s0,sp                       /* restore SP                         */

	mov     0*8(sp),%rbx                /* restore callee saved registers     */
	mov     1*8(sp),s0
	mov     2*8(sp),s1
	mov     3*8(sp),s2
	mov     4*8(sp),s3
	mov     5*8(sp),s4
	add     $(7*8),sp                   /* free stack space                   */
	ret
		
asm_vm_call_method_exception_handler:
	mov     xptr,a0                     /* pass exception pointer             */
	call    PIC_SYMBOL(builtin_throw_exception)
	jmp     L_asm_vm_call_method_return

asm_vm_call_method_end:
	nop


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
	add     $8,sp                       /* clear return address of native stub*/
		
asm_handle_exception:
L_asm_handle_exception:                 /* required for PIC code              */
	sub     $((ARG_CNT+TMP_CNT)*8),sp   /* create maybe-leaf stackframe       */

	SAVE_ARGUMENT_REGISTERS(0)          /* we save arg and temp registers in  */
	SAVE_TEMPORARY_REGISTERS(ARG_CNT)   /* case this is a leaf method         */

	mov     $((ARG_CNT+TMP_CNT)*8),a3   /* prepare a3 for handle_exception    */
	mov     $1,t0                       /* set maybe-leaf flag                */

L_asm_handle_exception_stack_loop:
	sub     $(6*8),sp
	mov     xptr,0*8(sp)                /* save exception pointer             */
	mov     xpc,1*8(sp)                 /* save exception pc                  */
	add     sp,a3                       /* calculate Java sp into a3...       */
	add     $(6*8),a3
	mov     a3,3*8(sp)                  /* ...and save it                     */
	mov     t0,4*8(sp)                  /* save maybe-leaf flag               */

	mov     xpc,a0                      /* exception pc                       */
	call    PIC_SYMBOL(methodtree_find)
	mov     v0,2*8(sp)                  /* save data segment pointer          */
        
	mov     0*8(sp),a0                  /* pass exception pointer             */
	mov     1*8(sp),a1                  /* pass exception pc                  */
	mov     v0,a2                       /* pass data segment pointer          */
	mov     3*8(sp),a3                  /* pass Java stack pointer            */
	call    PIC_SYMBOL(exceptions_handle_exception)

	test    v0,v0
	jz      L_asm_handle_exception_not_catched

	mov     v0,xpc                      /* move handlerpc into xpc            */
	mov     0*8(sp),xptr                /* restore exception pointer          */
	mov     4*8(sp),t0                  /* get maybe-leaf flag                */
	add     $(6*8),sp                   /* free stack frame                   */

	test    t0,t0                       /* test for maybe-leaf flag           */
	jz      L_asm_handle_exception_no_leaf

	RESTORE_ARGUMENT_REGISTERS(0)       /* if this is a leaf method, we have  */
	RESTORE_TEMPORARY_REGISTERS(ARG_CNT)/* to restore arg and temp registers  */

	add     $((ARG_CNT+TMP_CNT)*8),sp   /* remove maybe-leaf stackframe       */

L_asm_handle_exception_no_leaf:
	jmp     *xpc                        /* jump to the handler                */

L_asm_handle_exception_not_catched:
	mov     0*8(sp),xptr                /* restore exception pointer          */
	mov     2*8(sp),itmp3               /* restore data segment pointer       */
	mov     4*8(sp),t0                  /* get maybe-leaf flag                */
	add     $(6*8),sp

	test    t0,t0
	jz      L_asm_handle_exception_no_leaf_stack

	add     $((ARG_CNT+TMP_CNT)*8),sp   /* remove maybe-leaf stackframe       */
	xor     t0,t0                       /* clear the isleaf flags             */

L_asm_handle_exception_no_leaf_stack:
	mov     FrameSize(itmp3),itmp2l     /* get frame size                     */
	add     sp,itmp2                    /* pointer to save area               */
	
	mov     IntSave(itmp3),a0l          /* a0l = saved int register count     */
	test    a0l,a0l
	je      noint
	
	cmp     $1,a0l
	je      int1
	cmp     $2,a0l
	je      int2
	cmp     $3,a0l
	je      int3
	cmp     $4,a0l
	je      int4
	
	mov     -5*8(itmp2),s0
int4:   
	mov     -4*8(itmp2),s1
int3:   
	mov     -3*8(itmp2),s2
int2:   
	mov     -2*8(itmp2),s3
int1:   
	mov     -1*8(itmp2),s4

	shl     $3,a0l                      /* multiply by 8 bytes                */
	sub     a0,itmp2
		
noint:
#if 0
	mov     FltSave(itmp3),a0l          /* a0l = saved flt register count     */
	test	a0l,a0l
	je      noflt
	
	cmpl    $1,a0l
	je      flt1
	cmpl    $2,a0l
	je      flt2
	cmpl    $3,a0l
	je      flt3
	cmpl    $4,a0l
	je      flt4

	movq    -5*8(itmp2),%xmm11
flt4:   
	movq    -4*8(itmp2),%xmm12
flt3:   
	movq    -3*8(itmp2),%xmm13
flt2:   
	movq    -2*8(itmp2),%xmm14
flt1:   
	movq    -1*8(itmp2),%xmm15
                
noflt:
#endif
	mov     FrameSize(itmp3),itmp2l     /* get frame size                     */
	add     itmp2,sp                    /* unwind stack                       */

	                                    /* exception pointer is still set     */
	pop     xpc                         /* the new xpc is return address      */
	sub     $3,xpc                      /* subtract 3 bytes for call          */

	xor     a3,a3                       /* prepare a3 for handle_exception    */
	
	jmp		L_asm_handle_exception_stack_loop


/* asm_abstractmethoderror *****************************************************

   Creates and throws an AbstractMethodError.

*******************************************************************************/

asm_abstractmethoderror:
	sub     $8,sp                       /* keep stack aligned                 */
	mov     sp,a0                       /* pass java sp                       */
	add     $2*8,a0
	mov     1*8(sp),a1                  /* pass exception address             */
	sub     $3,a1
	call	PIC_SYMBOL(exceptions_asm_new_abstractmethoderror)
	                                    /* exception pointer is return value  */
	pop     xpc                         /* dummy pop                          */
	pop     xpc                         /* get exception address              */
	sub     $3,xpc                      /* exception address is ra - 3        */
	jmp     L_asm_handle_exception


/* asm_builtin_x2x *************************************************************
*                                                                              *
*   Wrapper functions for float to int corner cases                            *
*                                                                              *
*******************************************************************************/

asm_builtin_f2i:
	sub     $(ARG_CNT*8),sp
	
	SAVE_ARGUMENT_REGISTERS(0)
	
	movq    ftmp1,fa0
	call    PIC_SYMBOL(builtin_f2i)
	
	RESTORE_ARGUMENT_REGISTERS(0)
	
	add     $(ARG_CNT*8),sp
	ret


asm_builtin_f2l:
	sub     $(ARG_CNT*8),sp
	
	SAVE_ARGUMENT_REGISTERS(0)
	
	movq    ftmp1,fa0
	call    PIC_SYMBOL(builtin_f2l)
	
	RESTORE_ARGUMENT_REGISTERS(0)
	
	add     $(ARG_CNT*8),sp
	ret


asm_builtin_d2i:
	sub     $(ARG_CNT*8),sp
	
	SAVE_ARGUMENT_REGISTERS(0)
	
	movq    ftmp1,fa0
	call    PIC_SYMBOL(builtin_d2i)
	
	RESTORE_ARGUMENT_REGISTERS(0)
	
	add     $(ARG_CNT*8),sp
	ret


asm_builtin_d2l:
	sub     $(ARG_CNT*8),sp
	
	SAVE_ARGUMENT_REGISTERS(0)
	
	movq    ftmp1,fa0
	call    PIC_SYMBOL(builtin_d2l)
	
	RESTORE_ARGUMENT_REGISTERS(0)
	
	add     $(ARG_CNT*8),sp
	ret


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
