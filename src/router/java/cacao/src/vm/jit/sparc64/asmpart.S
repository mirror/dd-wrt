/* src/vm/jit/sparc64/asmpart.S - Java-C interface functions for Sparc64

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

#include "vm/jit/sparc64/md-abi.hpp"


	.register %g2,#scratch                         /* define as scratch       */
	.register %g3,#scratch                    /* XXX  reserve for application */
	.text

/* export functions ***********************************************************/

	.global asm_vm_call_method        
	.global asm_vm_call_method_int    
	.global asm_vm_call_method_long   
	.global asm_vm_call_method_float  
	.global asm_vm_call_method_double 
	.global asm_vm_call_method_exception_handler
	.global asm_vm_call_method_end

	.global asm_call_jit_compiler

	.global asm_handle_exception
	.global asm_handle_nat_exception

	.global asm_patcher_wrapper

	.global asm_abstractmethoderror
	
	.global asm_store_fp_state_reg
	.global asm_load_fp_state_reg


/* asm_vm_call_method ******************************************************
 *                                                                         *
 * This function calls a Java-method (which possibly needs compilation)    *
 *
 * If the java method is throwing an exception, NULL will be returned.
 *
 * C-prototype:
 *  java_objectheader *asm_vm_call_method(methodinfo *m, s4 vmargscount,
 *						 vm_arg *vmargs);
 **************************************************************************/

	.align 8	/* v9: All data types are aligned to their size */

	.word  0                         /* fltsave */
	.word  1                         /* intsave */
	.word  0                         /* isleaf */
	.word  0                         /* frame size */
	.xword 0                         /* method pointer (pointer to name)*/ 

asm_vm_call_method:       
asm_vm_call_method_int:   
asm_vm_call_method_long:  
asm_vm_call_method_float: 
asm_vm_call_method_double:
	
	save %sp,-((JITSTACK_CNT+2)*8),%sp
	add %sp,JITSTACK,%l1               /* pointer to usable stack      */

	/* i0: PV               */
	/* i1: ptr to arg array */
	/* i2: num stackargs    */

	ldx [%i1+0*8],%o0
	ldx [%i1+1*8],%o1
	ldx [%i1+2*8],%o2
	ldx [%i1+3*8],%o3
	ldx [%i1+4*8],%o4

	ldd [%i1+5*8],fa0
	ldd [%i1+6*8],fa1
	ldd [%i1+7*8],fa2
	ldd [%i1+8*8],fa3
	ldd [%i1+9*8],fa4

	brlez %i2, calljava_nocopy
	nop /* delay: fill me! */
	
	sllx %i2,3,%l0                     /* remaining args * 8           */
	sub %sp,%l0,%sp                    /* allocate more stack space    */
	add %sp,JITSTACK,%l1               /* adjust stack begin pointer   */
	
asm_vm_call_copyloop:
	ldx [%i1+10*8],%l0                 /* load argument from array     */
	stx %l0,[%l1]                      /* store argument to stack      */

	inc 8,%i1                          /* src++                        */
	subcc %i2,1,%i2                    /* arg_count--                  */
	bnz %xcc, asm_vm_call_copyloop     /* use cc from previous instr   */
	inc 8,%l1                          /* dst++ (delay)                */


calljava_nocopy:
	/* set pv, like a java method does */
	setx  asm_vm_call_method,%l0,pv_callee
	
	stx   %i0,[%l1 + 1*8]              /* store PV on stack            */
	mov   %l1,mptr_itmp2               /* set address of PV (-1*8)     */
	
	ldx  [1*8 + mptr_itmp2], pv_caller /* load PV from stack           */
	jmpl pv_caller,ra_caller           /* method call as in Java       */
	nop

calljava_jit2:
	/* pretend to restore pv */
	add  ra_caller,(asm_vm_call_method - calljava_jit2 + 8),zero
	
calljava_return:
	mov %o0, %i0		/* pass on the return value	*/
	return %i7 + 8		/* implicit window restore */
	nop


asm_vm_call_method_exception_handler:
	
	/* so far this function did not call any c functions */
	/* but now we need ABI compliant argslots on the stack */
	sub   %sp,6*8,%sp
	
	mov		xptr_itmp2,%o0
	call	builtin_throw_exception
	nop
	return 	%i7 + 8         		 /* implicit window restore */
asm_vm_call_method_end:
	mov    zero,%o0                  /* delay: return NULL      */
	


/****************** function asm_call_jit_compiler *****************************
*                                                                              *
*   invokes the compiler for untranslated JavaVM methods.                      *
*                                                                              *
*   Register R0 contains a pointer to the method info structure (prepared      *
*   by createcompilerstub). Using the return address in R26 and the            *
*   offset in the LDA instruction or using the value in methodptr R28 the      *
*   patching address for storing the method address can be computed:           *
*                                                                              *
*   method address was either loaded using                                     *
*   M_LDQ (REG_PV, REG_PV, a)        ; invokestatic/special    ($27)           *
*   M_LDA (REG_PV, REG_RA, low)                                                *
*   M_LDAH(REG_PV, REG_RA, high)     ; optional                                *
*   or                                                                         *
*   M_LDQ (REG_PV, REG_METHODPTR, m) ; invokevirtual/interface ($28)           *
*   in the static case the method pointer can be computed using the            *
*   return address and the lda function following the jmp instruction          *
*                                                                              *
*******************************************************************************/

asm_call_jit_compiler:
	
	/* stacksave for regsave(16) + argslots(6) + float args  */
	/* Note: +1 to keep stack 16-byte aligned                */
	save    %sp,-((16+6+FLT_ARG_CNT+1)*8),%sp

	SAVE_FLOAT_ARGUMENT_REGISTERS(22)

	mov     itmp1,%o0             /* pass methodinfo pointer                  */
	mov     mptr_itmp2,%o1        /* pass method pointer                      */
	mov     %fp,%o2				  /* pass java sp (==fp)             		  */
	mov     ra_callee,%o3         /* pass Java ra                             */
	mov     %o3,%o4               /* xpc is equal to ra                       */
	call    jit_asm_compile       /* call jit compiler                        */
	nop
	
	RESTORE_FLOAT_ARGUMENT_REGISTERS(22)

	brz     %o0,L_asm_call_jit_compiler_exception
	nop
	
	restore %o0,%g0,pv_caller     /* restore the callers window               */
	                              /* the source o0 references the old window  */
	                              /* pv_caller references the new window      */


	/* synchronise instruction cache moved somewhere else           */

	jmpl    pv_caller,zero        /* and call method, the method returns      */
	                              /* directly to the caller (ra).             */
	nop	
	
L_asm_call_jit_compiler_exception:
	/* window still open, ra_callee valid, pv_callee undefined      */

	call    exceptions_get_and_clear_exception
	nop

	mov     %o0,xptr_itmp2        /* get exception                            */
	mov     ra_callee,xpc_itmp3  /* exception address is address of call      */

	/* restore the window of the calling function */
	restore

	b      L_asm_handle_nat_exception
	nop



/* asm_handle_exception ********************************************************

   This function handles an exception. It does not use the usual calling
   conventions. The exception pointer is passed in REG_ITMP2 and the
   pc from the exception raising position is passed in REG_ITMP3. It searches
   the local exception table for a handler. If no one is found, it unwinds
   stacks and continues searching the callers.

*******************************************************************************/


asm_handle_nat_exception:
L_asm_handle_nat_exception:       /* required for PIC code                    */
L_asm_handle_exception_stack_loop:
	/* exception handling assumes that the current java method saved       */
	/* the caller's window, and has a valid pv                             */

	/* get ra and pv before saving the window */
	mov     ra_callee,itmp1
	mov     pv_callee,%g4

	save    %sp,-176,%sp
	mov     xptr_itmp2,%l0              /* save exception pointer             */
	mov     xpc_itmp3,%l1               /* save exception pc                  */
	mov     zero,%l2                    /* save maybe-leaf flag (cleared)     */

	mov     %l0,%o0                     /* pass xptr                          */
	mov     %l1,%o1                     /* pass xpc                           */
	mov     %g4,%o2                     /* pass PV                            */
	mov     %fp,%o3                     /* pass Java SP                       */

	b       L_asm_handle_exception_continue
	nop

asm_handle_exception:
	mov     pv_callee,%g4

	/* save bigger stack frame for float args and temps */
	save    %sp,-((FLT_ARG_CNT+FLT_TMP_CNT+16+CSTACK_CNT)*8),%sp

	SAVE_FLOAT_ARGUMENT_REGISTERS(CSTACK_CNT)
	SAVE_FLOAT_TEMPORARY_REGISTERS(CSTACK_CNT+FLT_ARG_CNT)

	mov     xptr_itmp2,%l0              /* save exception pointer             */
	add     zero,1,%l2                  /* set maybe-leaf flag                */

	mov     %l0,%o0                     /* pass xptr                          */
	mov     xpc_itmp3,%o1               /* pass xpc                           */
	mov     %g4,%o2                     /* pass PV                            */
	mov     %fp,%o3                     /* pass Java SP                       */

L_asm_handle_exception_continue:
	call    exceptions_handle_exception
	nop

	brz     %o0,L_asm_handle_exception_not_caught
	nop

	mov     %o0,xpc_itmp3               /* move handlerpc into xpc            */
	mov     %l0,xptr_itmp2              /* restore exception pointer          */

	brz     %l2,L_asm_handle_exception_no_leaf
	nop

	RESTORE_FLOAT_ARGUMENT_REGISTERS(CSTACK_CNT)
	RESTORE_FLOAT_TEMPORARY_REGISTERS(CSTACK_CNT+FLT_ARG_CNT)
	
L_asm_handle_exception_no_leaf:
	/* restore java window and stackframe (ra and pv are in there) */
	restore
	jmpl    xpc_itmp3, zero             /* jump to the handler                */
	nop

L_asm_handle_exception_not_caught:
	mov     %l0,xptr_itmp2              /* restore xptr                       */
	restore                             /* free our stackframe and window     */
	/* maybe leaf flag gets cleared after branch to _loop */
	
	restore                             /* unwind stack and window            */
	ba      L_asm_handle_exception_stack_loop
	mov     ra_caller,xpc_itmp3         /* the new xpc is ra (delay)          */
	



/* asm_abstractmethoderror *****************************************************

   Creates and throws an AbstractMethodError.

*******************************************************************************/

asm_abstractmethoderror:
	/* do a window save */
	save %sp,-192,%sp

	mov     %fp,%o0                     /* pass java sp(==fp)                 */
	mov     ra_callee,%o1               /* pass exception address             */
	call    exceptions_asm_new_abstractmethoderror
	nop

	mov     %o0,xptr_itmp2              /* get exception pointer              */
	sub     ra_callee,4,xpc_itmp3       /* exception address is ra - 4        */
	ba      L_asm_handle_nat_exception
	nop

	/* XXX: leave the register window open for handle_exception ??? */

/* asm_patcher_wrapper *********************************************************

   XXX

   Stack layout, when called from patcher stub
     40   return address into JIT code (patch position)
     32   pointer to virtual java_objectheader
     24   machine code (which is patched back later)
     16   unresolved class/method/field reference
      8   data segment displacement from load instructions
      0   patcher function pointer to call
   -128   WINSAVE REGS (current SP)

*******************************************************************************/
		

asm_patcher_wrapper:
	/* get pv and ra, since the emit code is not passing it on */
	mov     ra_callee,ra_caller
	mov     pv_callee,pv_caller

	/* create window and stack frame              */
	save  %sp,-((FLT_ARG_CNT+FLT_TMP_CNT+16+CSTACK_CNT+6)*8),%sp

	SAVE_FLOAT_RETURN_REGISTER(CSTACK_CNT)
	SAVE_FLOAT_ARGUMENT_REGISTERS(CSTACK_CNT+1)
	SAVE_FLOAT_TEMPORARY_REGISTERS(CSTACK_CNT+1+FLT_ARG_CNT)

	mov     itmp1,%l0               /* save itmp1                             */
	mov     itmp2,%l1               /* save itmp2                             */

	add     %fp,JITSTACK,%o0      /* pass pseudo SP                           */
	mov     pv_callee,%o1         /* pass PV                                  */
	mov     ra_callee,%o2         /* pass RA (correct for leafs)              */
	call    patcher_wrapper
	nop

	RESTORE_FLOAT_RETURN_REGISTER(CSTACK_CNT)
	RESTORE_FLOAT_ARGUMENT_REGISTERS(CSTACK_CNT+1)
	RESTORE_FLOAT_TEMPORARY_REGISTERS(CSTACK_CNT+1+FLT_ARG_CNT)

	mov     %l0,itmp1               /* restore itmp1                          */
	mov     %l1,itmp2               /* restore itmp2                          */

	brnz    %o0,L_asm_patcher_wrapper_exception
	nop

	/* load RA (patch position from patcher data on the stack */
	ldx     [%fp+JITSTACK+5*8],itmp3

	/* remove window and stack frame (and stack space allocated in the stub code */
	restore  %fp,6*8,%sp /* (source regs refer to old window, rd to new window)  */

	jmpl     itmp3,zero              /* jump to newly patched code               */
	nop

L_asm_patcher_wrapper_exception:
	mov      %o0,xptr_itmp2        /* get exception                            */
	ldx      [%fp+JITSTACK+5*8],xpc_itmp3 /* xpc is RA                         */
	restore  %fp,6*8,%sp           /* remove stack frame                       */
	ba       asm_handle_exception
	nop


    
/* asm_store_fp_state_reg **************************************************
 *                                                                         *
 * This function stores the 64-bit floating point state register to a      *
 * memory location. (which needs to be 8-byte aligned)                     *
 *                                                                         *
 * C-prototype:                                                            *
 *  void asm_store_fp_state_reg(u8 *mem);                                  *
 *	                                                                       *
 **************************************************************************/
 
asm_store_fp_state_reg:
	stx %fsr,[%o0]
	retl  /* return from leaf */
	nop
	
/* asm_load_fp_state_reg ***************************************************
 *                                                                         *
 * This function loades the 64-bit floating point state register from a    *
 * memory location. (which needs to be 8-byte aligned)                     *
 *                                                                         *
 * C-prototype:                                                            *
 *  void asm_load_fp_state_reg(u8 *mem);                                   *
 *	                                                                       *
 **************************************************************************/
 
asm_load_fp_state_reg:
	ldx [%o0],%fsr
	retl  /* return from leaf */
	nop


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
