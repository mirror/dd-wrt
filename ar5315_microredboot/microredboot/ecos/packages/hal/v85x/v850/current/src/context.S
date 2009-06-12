// #=============================================================================
// #
// #	context.S
// #
// #	NEC/V850 context switch code
// #
// #=============================================================================
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
// #=============================================================================
// ######DESCRIPTIONBEGIN####
// #
// # Author(s): 	nickg, gthomas
// # Contributors:	nickg, gthomas
// # Date:	1998-09-15
// # Purpose:	NEC/V850 context switch code
// # Description:	This file contains implementations of the thread context 
// #	switch routines. It also contains the longjmp() and setjmp() routines.
// #
// #####DESCRIPTIONEND####
// #
// #=============================================================================

#include <pkgconf/hal.h>

#include <cyg/hal/nec_offsets.inc>

	.text
		
// ------------------------------------------------------------------------------
//  hal_thread_switch_context
//  Switch thread contexts
//  R6 = address of sp of next thread to execute
//  R7 = address of sp save location of current thread
        .globl  __hal_thread_switch_context
__hal_thread_switch_context:             
        addi    -CYGARC_EXCEPTION_FRAME_SIZE,sp,sp        
        st.w    ep,CYGARC_REG_EP[sp]
        mov     sp,ep
        sst.w   r1,CYGARC_REG_R1[ep]        
        sst.w   r2,CYGARC_REG_R2[ep]        
        addi    CYGARC_EXCEPTION_FRAME_SIZE,ep,r1
        sst.w   r1,CYGARC_REG_SP[ep]    // sp
        sst.w   r4,CYGARC_REG_R4[ep]        
        sst.w   r5,CYGARC_REG_R5[ep]        
        sst.w   r6,CYGARC_REG_R6[ep]        
        sst.w   r7,CYGARC_REG_R7[ep]        
        sst.w   r8,CYGARC_REG_R8[ep]        
        sst.w   r9,CYGARC_REG_R9[ep]        
        sst.w   r10,CYGARC_REG_R10[ep]        
        sst.w   r11,CYGARC_REG_R11[ep]        
        sst.w   r12,CYGARC_REG_R12[ep]        
        sst.w   r13,CYGARC_REG_R13[ep]        
        sst.w   r14,CYGARC_REG_R14[ep]        
        sst.w   r15,CYGARC_REG_R15[ep]        
        sst.w   r16,CYGARC_REG_R16[ep]        
        sst.w   r17,CYGARC_REG_R17[ep]        
        sst.w   r18,CYGARC_REG_R18[ep]        
        sst.w   r19,CYGARC_REG_R19[ep]        
        sst.w   r20,CYGARC_REG_R20[ep]        
        sst.w   r21,CYGARC_REG_R21[ep]        
        sst.w   r22,CYGARC_REG_R22[ep]        
        sst.w   r23,CYGARC_REG_R23[ep]        
        sst.w   r24,CYGARC_REG_R24[ep]        
        sst.w   r25,CYGARC_REG_R25[ep]        
        sst.w   r26,CYGARC_REG_R26[ep]        
        sst.w   r27,CYGARC_REG_R27[ep]        
        sst.w   r28,CYGARC_REG_R28[ep]        
        sst.w   r29,CYGARC_REG_R29[ep]        
        sst.w   lp,CYGARC_REG_LP[ep]
        sst.w   lp,CYGARC_REG_PC[ep]
        stsr    PSW,r1
        st.w    r1,CYGARC_REG_PSW[ep]
        st.w    sp,0[r7]        // return new stack pointer

	# Now load the destination thread by dropping through
	# to hal_thread_load_context
// ------------------------------------------------------------------------------
//  hal_thread_load_context
//  Load thread context
//  R6 = address of sp of next thread to execute
//  Note that this function is also the second half of hal_thread_switch_context
//  and is simply dropped into from it.

        .globl  __hal_thread_load_context
__hal_thread_load_context:       
        di                              // disable interrupts while updating state
        ld.w    0[r6],ep                // get pointer to saved state
        ld.w    CYGARC_REG_PSW[ep],r6
        sld.w   CYGARC_REG_LP[ep],r7  
        sld.w   CYGARC_REG_R1[ep],r1    // restore volatile registers
        sld.w   CYGARC_REG_R2[ep],r2
        ldsr    r6,EIPSW                // avoids pipline bubble
        ldsr    r7,EIPC
        sld.w   CYGARC_REG_R4[ep],r4
        sld.w   CYGARC_REG_R5[ep],r5
        sld.w   CYGARC_REG_R6[ep],r6
        sld.w   CYGARC_REG_R7[ep],r7
        sld.w   CYGARC_REG_R8[ep],r8
        sld.w   CYGARC_REG_R9[ep],r9
        sld.w   CYGARC_REG_R10[ep],r10
        sld.w   CYGARC_REG_R11[ep],r11
        sld.w   CYGARC_REG_R12[ep],r12
        sld.w   CYGARC_REG_R13[ep],r13
        sld.w   CYGARC_REG_R14[ep],r14
        sld.w   CYGARC_REG_R15[ep],r15
        sld.w   CYGARC_REG_R16[ep],r16
        sld.w   CYGARC_REG_R17[ep],r17
        sld.w   CYGARC_REG_R18[ep],r18
        sld.w   CYGARC_REG_R19[ep],r19
        sld.w   CYGARC_REG_R20[ep],r20
        sld.w   CYGARC_REG_R21[ep],r21
        sld.w   CYGARC_REG_R22[ep],r22
        sld.w   CYGARC_REG_R23[ep],r23
        sld.w   CYGARC_REG_R24[ep],r24
        sld.w   CYGARC_REG_R25[ep],r25
        sld.w   CYGARC_REG_R26[ep],r26
        sld.w   CYGARC_REG_R27[ep],r27
        sld.w   CYGARC_REG_R28[ep],r28
        sld.w   CYGARC_REG_R29[ep],r29
        sld.w   CYGARC_REG_LP[ep],lp
        sld.w   CYGARC_REG_SP[ep],sp
        sld.w   CYGARC_REG_EP[ep],ep
        reti
        
// ------------------------------------------------------------------------------
//  HAL longjmp, setjmp implementations

#define fp r29        

        .globl  _hal_setjmp
_hal_setjmp:    
        st.w    r1,CYGARC_JMPBUF_R1[r6]        
        st.w    r2,CYGARC_JMPBUF_R2[r6]        
        st.w    r4,CYGARC_JMPBUF_R4[r6]        
        st.w    r5,CYGARC_JMPBUF_R5[r6]        
        st.w    sp,CYGARC_JMPBUF_SP[r6]        
        st.w    gp,CYGARC_JMPBUF_GP[r6]        
        st.w    tp,CYGARC_JMPBUF_TP[r6]        
        st.w    r20,CYGARC_JMPBUF_R20[r6]        
        st.w    r21,CYGARC_JMPBUF_R21[r6]        
        st.w    r22,CYGARC_JMPBUF_R22[r6]        
        st.w    r23,CYGARC_JMPBUF_R23[r6]        
        st.w    r24,CYGARC_JMPBUF_R24[r6]        
        st.w    r25,CYGARC_JMPBUF_R25[r6]        
        st.w    r26,CYGARC_JMPBUF_R26[r6]        
        st.w    r27,CYGARC_JMPBUF_R27[r6]        
        st.w    r28,CYGARC_JMPBUF_R28[r6]        
        st.w    fp,CYGARC_JMPBUF_FP[r6]        
        st.w    ep,CYGARC_JMPBUF_EP[r6]        
        st.w    lp,CYGARC_JMPBUF_LP[r6]
        mov     r0,r10
        jmp     [lp]        

//  hal_longjmp loads state from [arg0] and returns arg1

        .globl  _hal_longjmp
_hal_longjmp:           	
        ld.w    CYGARC_JMPBUF_SP[r6],sp
        ld.w    CYGARC_JMPBUF_GP[r6],gp               
        ld.w    CYGARC_JMPBUF_TP[r6],tp                
        ld.w    CYGARC_JMPBUF_R1[r6],r1
        ld.w    CYGARC_JMPBUF_R2[r6],r2        
        ld.w    CYGARC_JMPBUF_R4[r6],r4        
        ld.w    CYGARC_JMPBUF_R5[r6],r5        
        ld.w    CYGARC_JMPBUF_R20[r6],r20        
        ld.w    CYGARC_JMPBUF_R21[r6],r21        
        ld.w    CYGARC_JMPBUF_R22[r6],r22        
        ld.w    CYGARC_JMPBUF_R23[r6],r23        
        ld.w    CYGARC_JMPBUF_R24[r6],r24        
        ld.w    CYGARC_JMPBUF_R25[r6],r25                
        ld.w    CYGARC_JMPBUF_R26[r6],r26                
        ld.w    CYGARC_JMPBUF_R27[r6],r27                
        ld.w    CYGARC_JMPBUF_R28[r6],r28                
        ld.w    CYGARC_JMPBUF_FP[r6],fp
        ld.w    CYGARC_JMPBUF_EP[r6],ep
        ld.w    CYGARC_JMPBUF_LP[r6],lp
        mov     r7,r10
        jmp     [lp]        

// ------------------------------------------------------------------------------
//  end of context.S

	
