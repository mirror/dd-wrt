/* src/vm/jit/i386/asmpart.S - Java-C interface functions for i386

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

#include "md-asm.hpp"

#include "vm/jit/i386/arch.hpp"
#include "vm/jit/i386/md-abi.hpp"

#include "vm/jit/abi-asm.hpp"
#include "vm/jit/methodheader.hpp"


	.text


/* export functions ***********************************************************/

	.globl asm_md_init

	.globl asm_vm_call_method
	.globl asm_vm_call_method_int
	.globl asm_vm_call_method_long
	.globl asm_vm_call_method_float
	.globl asm_vm_call_method_double
	.globl asm_vm_call_method_exception_handler
	.globl asm_vm_call_method_end

	.globl asm_handle_nat_exception
	.globl asm_handle_exception

	.globl asm_abstractmethoderror

	.globl asm_builtin_f2i
	.globl asm_builtin_f2l
	.globl asm_builtin_d2i
	.globl asm_builtin_d2l

	.globl asm_compare_and_swap
	.globl asm_memory_barrier

#if defined(ENABLE_ESCAPE_CHECK)
	.globl asm_escape_check
#endif


/* asm_md_init *****************************************************************

   Initialize machine dependent stuff.

   See: http://www.srware.com/linux_numerics.txt

   This puts the X86 FPU in 64-bit precision mode.  The default under
   Linux is to use 80-bit mode, which produces subtle differences from
   FreeBSD and other systems, eg, (int)(1000*atof("0.3")) is 300 in
   64-bit mode, 299 in 80-bit mode.

   Fixes: http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=350729

*******************************************************************************/

asm_md_init:
	sub     $4,sp                       /* allocate space for the FPU state   */
	fnstcw  (sp)                        /* get the FPU state                  */
	mov     (sp),%eax
	and     $0xfcff,%ax                 /* remove the extended mode flag      */
	or      $0x0200,%ax                 /* put the double mode flag           */
	mov     %eax,(sp)                   /* store new FPU state                */
	fldcw   (sp)                        /* setup new FPU state                */
	add     $4,sp
	ret


/********************* function asm_calljavafunction ***************************
*                                                                              *
*   This function calls a Java-method (which possibly needs compilation)       *
*   with up to 4 address parameters.                                           *
*                                                                              *
*   This functions calls the JIT-compiler which eventually translates the      *
*   method into machine code.                                                  *
*                                                                              *
*   C-prototype:                                                               *
*    javaobject_header *asm_vm_call_method(methodinfo *m,                      *
*         u4 count, u4 size, void *callblock);                                 *
*                                                                              *
*******************************************************************************/

	.align	8

	.long   0                           /* fltsave                            */
	.long   0                           /* intsave                            */
	.long   0                           /* isleaf                             */
	.long   0                           /* frame size                         */
	.long   0                           /* codeinfo pointer                   */

asm_vm_call_method:
asm_vm_call_method_int:
asm_vm_call_method_long:
asm_vm_call_method_float:
asm_vm_call_method_double:
	push    bp
	mov     sp,bp                       /* save stack pointer                 */
	sub     $(4*4),sp                   /* create stackframe                  */
	and     $0xfffffff0,sp              /* align stack to 16-byte             */

	mov     t0,0*4(sp)                  /* save registers                     */
	mov     s1,1*4(sp)
	mov     s2,2*4(sp)

	mov     sp,s1                       /* save stack pointer                 */

	mov     3*4(bp),t0                  /* address of data structure          */
	mov     4*4(bp),itmp1               /* number of stack arguments          */

	cmp     $0,itmp1
	je      L_asm_vm_call_method_stack_copy_done

	mov     itmp1,itmp2
	add     $1,itmp2                    /* keep stack 16-byte aligned         */
	and     $0xfffffffe,itmp2
	shl     $3,itmp2                    /* calculate stack size               */
	sub     itmp2,sp                    /* create stack frame                 */
	mov     sp,itmp2                    /* temporary stack pointer            */

L_asm_vm_call_method_stack_copy_loop:
	mov     0(t0),itmp3                 /* load argument                      */
	mov     itmp3,0(itmp2)              /* store argument on stack            */
	mov     4(t0),itmp3
	mov     itmp3,4(itmp2)

	sub     $1,itmp1                    /* subtract 1 argument                */
	add     $8,t0                       /* set address of next argument       */
	add     $8,itmp2                    /* increase SP                        */

	cmp     $0,itmp1
	jg      L_asm_vm_call_method_stack_copy_loop

L_asm_vm_call_method_stack_copy_done:
	lea     (2*4-256)(bp),mptr          /* We subtract 256 to force the next  */
	                                    /* move instruction to have a 32-bit  */
	                                    /* offset.                            */

	mov     (0*4+256)(mptr),itmp3       /* method call as in Java             */
	call    *itmp3                      /* call JIT compiler                  */

L_asm_vm_call_method_return:
	mov     s1,sp                       /* restore stackpointer               */

	mov     0*4(sp),t0                  /* restore registers                  */
	mov     1*4(sp),s1
	mov     2*4(sp),s2

	leave
	ret

asm_vm_call_method_exception_handler:
#if defined(ENABLE_PIC_ASM)
	sub     $12, %esp
	push    %ebx
#endif
	push    xptr                        /* pass exception pointer             */
#if defined(ENABLE_PIC_ASM)
	call    .GETPC
	add     $_GLOBAL_OFFSET_TABLE_, %ebx
	call    builtin_throw_exception@PLT
	pop     %ebx
	add     $12, %esp
#else
	call    builtin_throw_exception
#endif
	add     $4,sp
asm_vm_call_method_end:
	jmp     L_asm_vm_call_method_return


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
	add     $4,sp                       /* clear return address of native stub*/
		
asm_handle_exception:
L_asm_handle_exception:                 /* required for PIC code              */
	sub     $((ARG_CNT+TMP_CNT+3)*4),sp /* keep stack 16-byte aligned         */

	SAVE_ARGUMENT_REGISTERS(0)          /* we save arg and temp registers in  */
	SAVE_TEMPORARY_REGISTERS(ARG_CNT)   /* case this is a leaf method         */

	mov     $((ARG_CNT+TMP_CNT+3)*4),itmp3 /* prepare a3 for handle_exception */
	mov     $1,t0                       /* set maybe-leaf flag                */

L_asm_handle_exception_stack_loop:
	sub     $(12*4),sp                  /* keep stack 16-byte aligned         */
	mov     xptr,4*4(sp)                /* save exception pointer             */
	mov     xpc,5*4(sp)                 /* save exception pc                  */
	add     sp,itmp3                    /* calculate Java sp into a3...       */
	add     $(12*4),itmp3
	mov     itmp3,7*4(sp)               /* ...and save it                     */
	mov     t0,8*4(sp)                  /* save maybe-leaf flag               */

	mov     xpc,0*4(sp)                 /* pass exception pc                  */
#if defined(ENABLE_PIC_ASM)
	call    .GETPC
	add     $_GLOBAL_OFFSET_TABLE_, %ebx
	call    methodtree_find@PLT
#else
	call    methodtree_find
#endif
	mov     v0,6*4(sp)                  /* save data segment pointer          */

	mov     4*4(sp),itmp3               /* pass exception pointer             */
	mov     itmp3,0*4(sp)
	mov     5*4(sp),itmp3               /* pass exception pc                  */
	mov     itmp3,1*4(sp)
	mov     v0,2*4(sp)                  /* pass data segment pointer          */
	mov     7*4(sp),itmp3               /* pass Java stack pointer            */
	mov     itmp3,3*4(sp)
#if defined(ENABLE_PIC_ASM)
	/* GOT still in %ebx */
	call    exceptions_handle_exception@PLT
#else
	call    exceptions_handle_exception
#endif

	test    v0,v0
	jz      L_asm_handle_exception_not_catched

	mov     v0,xpc                      /* move handlerpc into xpc            */
	mov     4*4(sp),xptr                /* restore exception pointer          */
	mov     8*4(sp),t0                  /* get maybe-leaf flag                */
	add     $(12*4),sp                  /* free stackframe                    */

	test    t0,t0                       /* test for maybe-leaf flag           */
	jz      L_asm_handle_exception_no_leaf

	RESTORE_ARGUMENT_REGISTERS(0)       /* if this is a leaf method, we have  */
	RESTORE_TEMPORARY_REGISTERS(ARG_CNT)/* to restore arg and temp registers  */

	add     $((ARG_CNT+TMP_CNT+3)*4),sp /* remove maybe-leaf stackframe       */

L_asm_handle_exception_no_leaf:
	jmp     *xpc                        /* jump to exception handler          */

L_asm_handle_exception_not_catched:
	mov     4*4(sp),xptr                /* restore exception pointer          */
	mov     6*4(sp),itmp3               /* restore data segment pointer       */
	mov     8*4(sp),t0                  /* get maybe-leaf flag                */
	add     $(12*4),sp                  /* free stackframe                    */

	test    t0,t0
	jz      L_asm_handle_exception_no_leaf_stack

	add     $((ARG_CNT+TMP_CNT+3)*4),sp /* remove maybe-leaf stackframe       */
	xor     t0,t0                       /* clear the maybe-leaf flag          */

L_asm_handle_exception_no_leaf_stack:
	mov     FrameSize(itmp3),itmp2      /* get frame size                     */
	add     sp,itmp2                    /* pointer to save area               */

	push    xptr                        /* we are out of registers            */

	mov     IntSave(itmp3),itmp1        /* itmp1 = saved int register count   */
	test    itmp1,itmp1
	je      noint

	cmp     $1,itmp1
	je      int1
	cmp     $2,itmp1
	je      int2

	mov     -4-3*8(itmp2),s0
int2:	
	mov     -4-2*8(itmp2),s1
int1:	
	mov     -4-1*8(itmp2),s2

	shl     $2,itmp1                    /* multiply by 4 bytes                */
	sub     itmp1,itmp2
		
noint:
#if 0
	mov     FltSave(itmp3),itmp1        /* itmp1 = saved flt register count   */
	test    itmp1,itmp1
	je      noflt

	cmp     $1,itmp1
	je      flt1
	cmp     $2,itmp1
	je      flt2
	cmp     $3,itmp1
	je      flt3
		
	fldl    -4*8(itmp2)
	fstp    %st(1)
flt3:
	fldl    -3*8(itmp2)
	fstp    %st(2)
flt2:
	fldl    -2*8(itmp2)
	fstp    %st(3)
flt1:
	fldl    -1*8(itmp2)
	fstp    %st(4)
		
noflt:
#endif
	pop     xptr                        /* restore exception pointer          */
	mov     FrameSize(itmp3),itmp2      /* get frame size                     */
	add     itmp2,sp                    /* unwind stack                       */

	pop     xpc                         /* the new xpc is return address      */
	sub     $2,xpc                      /* subtract 2-bytes for call          */

	xor     itmp3,itmp3                 /* prepare a3 for handle_exception    */

	jmp     L_asm_handle_exception_stack_loop
		

/* asm_abstractmethoderror *****************************************************

   Creates and throws an AbstractMethodError.

*******************************************************************************/

asm_abstractmethoderror:
	sub     $(3*4),sp                   /* keep stack 16-byte aligned         */
	mov     sp,itmp1                    /* pass java sp                       */
	add     $((1+3)*4),itmp1
	mov     itmp1,0*4(sp)
	mov     3*4(sp),itmp2               /* pass exception address             */
	sub     $2,itmp2
	mov     itmp2,1*4(sp)
#if defined(ENABLE_PIC_ASM)
	call    .GETPC
	add     $_GLOBAL_OFFSET_TABLE_, %ebx
	call	exceptions_asm_new_abstractmethoderror@PLT
#else
	call	exceptions_asm_new_abstractmethoderror
#endif
	                                    /* exception pointer is return value  */
	add     $(3*4),sp                   /* remove stack frame                 */

	pop     xpc                         /* get exception address              */
	sub     $2,xpc                      /* exception address is ra - 2        */
	jmp     L_asm_handle_exception


/************************ function asm_builtin_x2x *****************************
*                                                                              *
*   Wrapper functions for corner cases                                         *
*                                                                              *
*******************************************************************************/

#if defined(ENABLE_PIC_ASM)
.GETPC:
	mov     (%esp), %ebx
	ret
#endif

asm_builtin_f2i:
	sub     $(3*4),%esp
#if defined(ENABLE_PIC_ASM)
	sub     $12, %esp
	push    %ebx
	fsts    (%esp)
	call    .GETPC
	add     $_GLOBAL_OFFSET_TABLE_, %ebx
	call    builtin_f2i@PLT
	pop     %ebx
	add     $12, %esp
#else
	fsts    (%esp)
	call    builtin_f2i
#endif
	add     $(3*4),%esp
	ret

asm_builtin_d2i:
	sub     $(3*4),%esp
#if defined(ENABLE_PIC_ASM)
	sub     $12, %esp
	push    %ebx
	fstl    (%esp)
	call    .GETPC
	add     $_GLOBAL_OFFSET_TABLE_, %ebx
	call    builtin_d2i@PLT
	pop     %ebx
	add     $12, %esp
#else
	fstl    (%esp)
	call    builtin_d2i
#endif
	add     $(3*4),%esp
	ret

asm_builtin_f2l:
	sub     $(3*4),%esp
#if defined(ENABLE_PIC_ASM)
	sub     $12, %esp
	push    %ebx
	fsts    (%esp)
	call    .GETPC
	add     $_GLOBAL_OFFSET_TABLE_, %ebx
	call    builtin_f2l@PLT
	pop     %ebx
	add     $12, %esp
#else
	fsts    (%esp)
	call    builtin_f2l
#endif
	add     $(3*4),%esp
	ret

asm_builtin_d2l:
	sub     $(3*4),%esp
#if defined(ENABLE_PIC_ASM)
	sub     $12, %esp
	push    %ebx
	fstl    (%esp)
	call    .GETPC
	add     $_GLOBAL_OFFSET_TABLE_, %ebx
	call    builtin_d2l@PLT
	pop     %ebx
	add     $12, %esp
#else
	fstl    (%esp)
	call    builtin_d2l
#endif
	add     $(3*4),%esp
	ret


/* asm_compare_and_swap ********************************************************

   Does an atomic compare and swap.  Required for the lock
   implementation.

   Atomically do the following: Check if the location still contains
   `oldval`. If so, replace it by `newval` and return `oldval`.

   RETURN VALUE:
       the old value at *p

   long compare_and_swap(volatile long *p, long oldval, long newval);

*******************************************************************************/

asm_compare_and_swap:
	mov     1*4(sp),%ecx            /* load p into a register                 */
	mov     2*4(sp),%eax            /* load oldval into return register       */
	mov     3*4(sp),%edx            /* load newval into a register            */
	lock; cmpxchgl %edx,0(%ecx)
	ret


/* asm_memory_barrier **********************************************************

   A memory barrier for the Java Memory Model.

*******************************************************************************/

asm_memory_barrier:
	lock; addl $0,0(sp)
	ret


#if defined(ENABLE_ESCAPE_CHECK)
asm_escape_check:
	sub     $24,%esp

	mov     t0, 4(%esp)
	mov     itmp1, 8(%esp)
	mov     itmp2, 12(%esp)
	mov     itmp3, 16(%esp)

	mov     28(%esp), itmp1
	mov     itmp1, (%esp)

	call    escape_analysis_escape_check

	mov     4(%esp), t0
	mov     8(%esp), itmp1
	mov     12(%esp), itmp2
	mov     16(%esp), itmp3

	add     $24,sp
	ret
#endif


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
