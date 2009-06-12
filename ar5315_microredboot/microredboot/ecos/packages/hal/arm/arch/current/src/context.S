// #===========================================================================
// #
// #    context.S
// #
// #    ARM context switch code
// #
// #===========================================================================
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
// #===========================================================================
// ######DESCRIPTIONBEGIN####
// #
// # Author(s):    nickg, gthomas
// # Contributors: nickg, gthomas
// # Date:         1998-09-15
// # Purpose:      ARM context switch code
// # Description:  This file contains implementations of the thread context 
// #               switch routines. It also contains the longjmp() and setjmp()
// #               routines.
// #
// #####DESCRIPTIONEND####
// #
// #===========================================================================

#include <pkgconf/hal.h>

#include "arm.inc"

        .text

// ----------------------------------------------------------------------------
//  function declaration macro (start body in ARM mode)
                
#ifdef __thumb__
#define FUNC_START_ARM(_name_, _r_)              \
        .code   16                              ;\
        .thumb_func                             ;\
        .globl _name_                           ;\
_name_:                                         ;\
        ldr     _r_,=_name_ ## _ARM             ;\
        bx      _r_                             ;\
        .code   32                              ;\
_name_ ## _ARM:

#else

#define FUNC_START_ARM(_name_, _r_) \
        .globl _name_; \
_name_: 
        
#endif
        
// ----------------------------------------------------------------------------
//  hal_thread_switch_context
//  Switch thread contexts
//  R0 = address of sp of next thread to execute
//  R1 = address of sp save location of current thread

// Need to save/restore R4..R12, R13 (sp), R14 (lr)

// Note: this is a little wasteful since r0..r3 don't need to be saved.
// They are saved here though so that the information can match the HAL_SavedRegisters
        
FUNC_START_ARM(hal_thread_switch_context, r2)
        sub     ip,sp,#20               // skip svc_sp, svc_lr, vector, cpsr, and pc
        stmfd   ip!,{sp,lr}
        mov     sp,ip
        stmfd   sp!,{r0-r10,fp,ip}
        mrs     r2,cpsr
        str     r2,[sp,#armreg_cpsr]
        str     sp,[r1]                 // return new stack pointer
#ifdef __thumb__
        b       hal_thread_load_context_ARM // skip mode switch stuff
#endif

        # Now load the destination thread by dropping through
        # to hal_thread_load_context
        
// ----------------------------------------------------------------------------
//  hal_thread_load_context
//  Load thread context
//  R0 = address of sp of next thread to execute
//  Note that this function is also the second half of
//  hal_thread_switch_context and is simply dropped into from it.
        
FUNC_START_ARM(hal_thread_load_context, r2)
        ldr     fp,[r0]                 // get context to restore
        mrs     r0,cpsr                 // disable IRQ's
        orr     r0,r0,#CPSR_IRQ_DISABLE|CPSR_FIQ_DISABLE
        msr     cpsr,r0
        ldr     r0,[fp,#armreg_cpsr]
        msr     spsr,r0
        ldmfd   fp,{r0-r10,fp,ip,sp,lr}
#ifdef __thumb__
        mrs     r1,spsr                 // r1 is scratch 
                                        // [r0 holds initial thread arg]
        msr     cpsr,r1                 // hopefully no mode switch here!
        bx      lr
#else
        movs    pc,lr                   // also restores saved PSR
#endif

// ----------------------------------------------------------------------------
//  HAL longjmp, setjmp implementations
//  hal_setjmp saves only to callee save registers 4-14
//  and lr into buffer supplied in r0[arg0]

FUNC_START_ARM(hal_setjmp, r2)
        stmea   r0,{r4-r14}
        mov     r0,#0
#ifdef __thumb__
        bx      lr
#else
        mov     pc,lr;          # return
#endif

//  hal_longjmp loads state from r0[arg0] and returns
        
FUNC_START_ARM(hal_longjmp, r2)
        ldmfd   r0,{r4-r14}
        mov     r0,r1;          # return [arg1]
#ifdef __thumb__
        bx      lr
#else
        mov     pc,lr
#endif

// ----------------------------------------------------------------------------
//  end of context.S
