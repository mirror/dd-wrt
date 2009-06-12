#ifndef CYGONCE_HAL_HAL_ARCH_H
#define CYGONCE_HAL_HAL_ARCH_H

//==========================================================================
//
//      hal_arch.h
//
//      Architecture specific abstractions
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    nickg,gthomas
// Contributors: nickg,jlarmour
// Date:         2001-03-21
// Purpose:      Define architecture abstractions
// Usage:        #include <cyg/hal/hal_arch.h>
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>

#include <cyg/hal/var_arch.h>

//--------------------------------------------------------------------------
// Processor saved states:

#define CYG_HAL_NEC_REG CYG_WORD32
#define CYG_HAL_NEC_REG_SIZE 4

typedef struct 
{
    // These are common to all saved states
    CYG_HAL_NEC_REG     d[32];          /* Data regs                    */
    CYG_ADDRWORD        pc;             /* Program Counter              */
    CYG_ADDRWORD        psw;            /* Status Reg                   */
    
    // These are only saved for exceptions and interrupts
    CYG_ADDRWORD        cause;          /* Exception cause register     */   
    CYG_ADDRWORD        vector;         /* Exception/interrupt number   */   
} HAL_SavedRegisters;

//
// Processor state register
//
#define CYGARC_PSW_ID   0x20     // Interrupt disable
#define CYGARC_PSW_EP   0x40     // Exception in progress
#define CYGARC_PSW_NP   0x80     // NMI in progress

//--------------------------------------------------------------------------
// Exception handling function.
// This function is defined by the kernel according to this prototype. It is
// invoked from the HAL to deal with any CPU exceptions that the HAL does
// not want to deal with itself. It usually invokes the kernel's exception
// delivery mechanism.

externC void cyg_hal_deliver_exception( CYG_WORD code, CYG_ADDRWORD data );

//--------------------------------------------------------------------------
// Bit manipulation macros

externC cyg_uint32 hal_lsbit_index(cyg_uint32 mask);
externC cyg_uint32 hal_msbit_index(cyg_uint32 mask);

#define HAL_LSBIT_INDEX(index, mask) index = hal_lsbit_index(mask);

#define HAL_MSBIT_INDEX(index, mask) index = hal_msbit_index(mask);

//--------------------------------------------------------------------------
// Context Initialization

#define CYG_HAL_NEC_INIT_PSW  0x00000000

// Initialize the context of a thread.
// Arguments:
// _sparg_ name of variable containing current sp, will be written with new sp
// _thread_ thread object address, passed as argument to entry point
// _entry_ entry point address.
// _id_ bit pattern used in initializing registers, for debugging.
#define HAL_THREAD_INIT_CONTEXT( _sparg_, _thread_, _entry_, _id_ )                     \
{                                                                                       \
    register CYG_WORD _sp_ = ((CYG_WORD)_sparg_)-56;                                    \
    register HAL_SavedRegisters *_regs_;                                                \
    int _i_;                                                                            \
    _sp_ = _sp_ & 0xFFFFFFF0;                                                           \
    _regs_ = (HAL_SavedRegisters *)(((_sp_) - sizeof(HAL_SavedRegisters))&0xFFFFFFF0);  \
    for( _i_ = 0; _i_ < 32; _i_++ ) (_regs_)->d[_i_] = (_id_)|_i_;                      \
    (_regs_)->d[03] = (CYG_HAL_NEC_REG)(_sp_);       /* SP = top of stack      */      \
    (_regs_)->d[04] = (CYG_HAL_NEC_REG)(_sp_);       /* GP = top of stack      */      \
    (_regs_)->d[29] = (CYG_HAL_NEC_REG)(_sp_);       /* FP = top of stack      */      \
    (_regs_)->d[06] = (CYG_HAL_NEC_REG)(_thread_);   /* R6 = arg1 = thread ptr */      \
    (_regs_)->d[31] = (CYG_HAL_NEC_REG)(_entry_);    /* RA(d[31]) = entry point*/      \
    (_regs_)->pc = (CYG_WORD)(_entry_);              /* PC = entry point       */      \
    (_regs_)->psw = CYG_HAL_NEC_INIT_PSW;                                              \
    _sparg_ = (CYG_ADDRESS)_regs_;                                                      \
}

//--------------------------------------------------------------------------
// Context switch macros.
// The arguments are pointers to locations where the stack pointer
// of the current thread is to be stored, and from where the sp of the
// next thread is to be fetched.

externC void hal_thread_switch_context( CYG_ADDRESS to, CYG_ADDRESS from );
externC void hal_thread_load_context( CYG_ADDRESS to )
    __attribute__ ((noreturn));

#define HAL_THREAD_SWITCH_CONTEXT(_fspptr_,_tspptr_)                    \
        hal_thread_switch_context( (CYG_ADDRESS)_tspptr_,               \
                                   (CYG_ADDRESS)_fspptr_);

#define HAL_THREAD_LOAD_CONTEXT(_tspptr_)                               \
        hal_thread_load_context( (CYG_ADDRESS)_tspptr_ );

//--------------------------------------------------------------------------
// Execution reorder barrier.
// When optimizing the compiler can reorder code. In multithreaded systems
// where the order of actions is vital, this can sometimes cause problems.
// This macro may be inserted into places where reordering should not happen.
// The "memory" keyword is potentially unnecessary, but it is harmless to
// keep it.

#define HAL_REORDER_BARRIER() asm volatile ( "" : : : "memory" )

//--------------------------------------------------------------------------
// Breakpoint support
// HAL_BREAKPOINT() is a code sequence that will cause a breakpoint to
// happen if executed.
// HAL_BREAKINST is the value of the breakpoint instruction and
// HAL_BREAKINST_SIZE is its size in bytes.

#define HAL_BREAKPOINT(_label_)                 \
asm volatile (" .globl  _" #_label_ ";"         \
              "_"#_label_":"                    \
              " br _"#_label_                   \
    );

#define HAL_BREAKINST           0x0585
#define HAL_BREAKINST_SIZE      2

//--------------------------------------------------------------------------
// Thread register state manipulation for GDB support.

// Default to a 32 bit register size for GDB register dumps.
#ifndef CYG_HAL_GDB_REG
#define CYG_HAL_GDB_REG CYG_WORD32
#endif

// Translate a stack pointer as saved by the thread context macros above into
// a pointer to a HAL_SavedRegisters structure.
#define HAL_THREAD_GET_SAVED_REGISTERS( _sp_, _regs_ )          \
        (_regs_) = (HAL_SavedRegisters *)(_sp_)


//--------------------------------------------------------------------------
// HAL setjmp

typedef struct {
    cyg_uint32 sp;
    cyg_uint32 gp;
    cyg_uint32 tp;
    cyg_uint32 r1,r2,r4,r5;
    cyg_uint32 r20, r21, r22, r23;
    cyg_uint32 r24, r25, r26, r27, r28;
    cyg_uint32 fp;
    cyg_uint32 ep;
    cyg_uint32 lp;
} hal_jmp_buf_t;

#define CYGARC_JMP_BUF_SIZE      (sizeof(hal_jmp_buf_t) / sizeof(cyg_uint32))

typedef cyg_uint32 hal_jmp_buf[ CYGARC_JMP_BUF_SIZE ];

externC int hal_setjmp(hal_jmp_buf env);
externC void hal_longjmp(hal_jmp_buf env, int val);

//-------------------------------------------------------------------------
// Idle thread code.
// This macro is called in the idle thread loop, and gives the HAL the
// chance to insert code. Typical idle thread behaviour might be to halt the
// processor.

externC void hal_idle_thread_action(cyg_uint32 loop_count);

#define HAL_IDLE_THREAD_ACTION(_count_) hal_idle_thread_action(_count_)

//--------------------------------------------------------------------------
// Hardware specific test exit code.  This is defined here simply to make
// setting a breakpoint on this function viable.
//
#define CYGHWR_TEST_PROGRAM_EXIT()              \
{                                               \
    static volatile int ctr;                    \
    while (1) ctr++;                            \
}

//--------------------------------------------------------------------------
// Minimal and sensible stack sizes: the intention is that applications
// will use these to provide a stack size in the first instance prior to
// proper analysis.  Idle thread stack should be this big.

//    THESE ARE NOT INTENDED TO BE MICROMETRICALLY ACCURATE FIGURES.
//           THEY ARE HOWEVER ENOUGH TO START PROGRAMMING.
// YOU MUST MAKE YOUR STACKS LARGER IF YOU HAVE LARGE "AUTO" VARIABLES!

// This is not a config option because it should not be adjusted except
// under "enough rope" sort of disclaimers.

// Typical case stack frame size: return link + 4 pushed registers + some locals.
#define CYGNUM_HAL_STACK_FRAME_SIZE (48)

// Stack needed for a context switch:
#define CYGNUM_HAL_STACK_CONTEXT_SIZE (((32+12)*CYG_HAL_NEC_REG_SIZE)+(32*4))


// Interrupt + call to ISR, interrupt_end() and the DSR
#define CYGNUM_HAL_STACK_INTERRUPT_SIZE (4+2*CYGNUM_HAL_STACK_CONTEXT_SIZE) 

#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK

// An interrupt stack which is large enough for all possible interrupt
// conditions (and only used for that purpose) exists.  "User" stacks
// can be much smaller

#define CYGNUM_HAL_STACK_SIZE_MINIMUM (CYGNUM_HAL_STACK_CONTEXT_SIZE+      \
                                       CYGNUM_HAL_STACK_INTERRUPT_SIZE*2+  \
                                       CYGNUM_HAL_STACK_FRAME_SIZE*8)
#define CYGNUM_HAL_STACK_SIZE_TYPICAL (CYGNUM_HAL_STACK_SIZE_MINIMUM+1024)

#else // CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK 

// No separate interrupt stack exists.  Make sure all threads contain
// a stack sufficiently large.

#define CYGNUM_HAL_STACK_SIZE_MINIMUM (4096)
#define CYGNUM_HAL_STACK_SIZE_TYPICAL (4096)

#endif

//--------------------------------------------------------------------------
// Macros for switching context between two eCos instances (jump from
// code in ROM to code in RAM or vice versa).
#define CYGARC_HAL_SAVE_GP()
#define CYGARC_HAL_RESTORE_GP()

//--------------------------------------------------------------------------
#endif // CYGONCE_HAL_HAL_ARCH_H
// End of hal_arch.h
