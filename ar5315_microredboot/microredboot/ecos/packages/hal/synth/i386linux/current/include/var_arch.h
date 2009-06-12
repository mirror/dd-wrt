#ifndef CYGONCE_HAL_VAR_ARCH_H
#define CYGONCE_HAL_VAR_ARCH_H

//=============================================================================
//
//      var_arch.h
//
//      Per-processor information such as processor save states.
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
// Author(s):   proven
// Contributors:proven, pjo, nickg,bartv
// Date:        1998-10-05
// Purpose:     Define architecture abstractions
// Usage:       #include <cyg/hal/hal_arch.h>
//
//####DESCRIPTIONEND####
//
//=============================================================================


#include <cyg/infra/cyg_type.h>

//-----------------------------------------------------------------------------
// Processor saved states. This structure is also defined in arch.inc for
// assembly code. Do not change this without changing that (or vice versa).
// Note: there is no need to worry about floating point contexts, see context.S

typedef struct 
{
    cyg_uint32  esp;
    cyg_uint32  next_context;           // only used when dropping through...
    cyg_uint32  ebp;                    // ...from switch_ to load_context
    cyg_uint32  ebx;
    cyg_uint32  esi;
    cyg_uint32  edi;
    cyg_bool    interrupts;             // Are interrupts enabled for this thread?
} HAL_SavedRegisters;


//-----------------------------------------------------------------------------
// Bit manipulation routines. These are provided by the processor variant
// HAL to allow for processor-specific implementations.

#define HAL_LSBIT_INDEX(index, mask)            \
CYG_MACRO_START                                 \
    asm volatile( "bsfl %1,%0\n"                \
                  : "=r" (index)                \
                  : "r" (mask)                  \
                );                              \
CYG_MACRO_END

#define HAL_MSBIT_INDEX(index, mask)            \
CYG_MACRO_START                                 \
    asm volatile( "bsrl %1,%0\n"                \
                  : "=r" (index)                \
                  : "r" (mask)                  \
                );                              \
CYG_MACRO_END

//-----------------------------------------------------------------------------
// Context Initialization
// Initialize the context of a thread.
// Arguments:
// _sp_ name of variable containing current sp, will be written with new sp
// _thread_ thread object address, passed as argument to entry point
// _entry_ entry point address.
// _id_ bit pattern used in initializing registers, for debugging.

#define HAL_THREAD_INIT_CONTEXT( _sparg_, _thread_, _entry_, _id_ )       \
    CYG_MACRO_START                                                       \
    register CYG_WORD* _sp_ = ((CYG_WORD*)((_sparg_) &~15));              \
    register HAL_SavedRegisters *_regs_;                                  \
                                                                          \
    /* The 'ret' executed at the end of hal_thread_load_context will  */  \
    /* use the last entry on the stack as a return pointer (_entry_). */  \
    /* Cyg_HardwareThread::thread_entry expects one argument at stack */  \
    /* offset 4 (_thread_). The (0xDEADBEEF) entry is the return addr */  \
    /* for thread_entry (which is never used).                        */  \
    *(--_sp_) = (CYG_WORD)(0);                                            \
    *(--_sp_) = (CYG_WORD)(0);                                            \
    *(--_sp_) = (CYG_WORD)(0);                                            \
    *(--_sp_) = (CYG_WORD)(0);                                            \
    *(--_sp_) = (CYG_WORD)(_thread_);                                     \
    *(--_sp_) = (CYG_WORD)(0);                                            \
    *(--_sp_) = (CYG_WORD)(_entry_);                                      \
                                                                          \
    _regs_ = (HAL_SavedRegisters *)                                       \
               ((unsigned long)_sp_ - sizeof(HAL_SavedRegisters));        \
    _regs_->esp    = (CYG_WORD) _sp_;                                     \
    _regs_->ebx    = (CYG_WORD)(_id_);                                    \
    _regs_->ebp    = (CYG_WORD)(_id_);                                    \
    _regs_->esi    = (CYG_WORD)(_id_);                                    \
    _regs_->edi    = (CYG_WORD)(_id_);                                    \
    _regs_->interrupts = true;                                            \
    (_sparg_)      = (CYG_ADDRESS) _regs_;                                \
    CYG_MACRO_END

//-----------------------------------------------------------------------------
// Context switch macros.
// The arguments are pointers to locations where the stack pointer
// of the current thread is to be stored, and from where the sp of the
// next thread is to be fetched.

externC void hal_thread_switch_context( CYG_ADDRESS _to_, CYG_ADDRESS _from_ );
externC void hal_thread_load_context( CYG_ADDRESS _to_ )
    __attribute__ ((noreturn));

#define HAL_THREAD_SWITCH_CONTEXT(_fspptr_,_tspptr_)                    \
        hal_thread_switch_context((CYG_ADDRESS)_tspptr_,(CYG_ADDRESS)_fspptr_);

#define HAL_THREAD_LOAD_CONTEXT(_tspptr_)                               \
        hal_thread_load_context( (CYG_ADDRESS)_tspptr_ );

//-----------------------------------------------------------------------------
// HAL setjmp

#define CYGARC_JMP_BUF_SP        0
#define CYGARC_JMP_BUF_EBP       1
#define CYGARC_JMP_BUF_EBX       2
#define CYGARC_JMP_BUF_ESI       3
#define CYGARC_JMP_BUF_EDI       4
#define CYGARC_JMP_BUF_PC        5

#define CYGARC_JMP_BUF_SIZE      6

typedef cyg_uint32 hal_jmp_buf[CYGARC_JMP_BUF_SIZE];

externC int hal_setjmp(hal_jmp_buf env);
externC void hal_longjmp(hal_jmp_buf env, int val);

//-----------------------------------------------------------------------------
// Minimal and sensible stack sizes: the intention is that applications
// will use these to provide a stack size in the first instance prior to
// proper analysis.  Idle thread stack should be this big.

//    THESE ARE NOT INTENDED TO BE MICROMETRICALLY ACCURATE FIGURES.
//           THEY ARE HOWEVER ENOUGH TO START PROGRAMMING.
// YOU MUST MAKE YOUR STACKS LARGER IF YOU HAVE LARGE "AUTO" VARIABLES!
 
// This is not a config option because it should not be adjusted except
// under "enough rope" sort of disclaimers.
 
// Stack frame overhead per call. 3 local registers (edi, esi, ebx) and
// return address.
#define CYGNUM_HAL_STACK_FRAME_SIZE (4 * 4)

// Stack needed for a context switch (i386reg_context_size from i386.inc)
#define CYGNUM_HAL_STACK_CONTEXT_SIZE (4 * 24)

// Interrupt stack size. Interrupts are handled by signals so the relevant
// data is MINSIGSTKSIZE (see man sigaltstack) or 2048. Given the
// multiplier *15 for STACK_SIZE_MINIMUM, this should be adequate.
#define CYGNUM_HAL_STACK_INTERRUPT_SIZE 2048

// We define a minimum stack size as the minimum any thread could ever
// legitimately get away with. We can throw asserts if users ask for less
// than this. Allow enough for three interrupt sources - clock, serial and
// one other
//
// On the synthetic target memory is cheap so comparatively large stacks
// are possible. This avoids stack overflow problems when working with
// the synthetic target, although arguably the problem is now deferred to
// when the application is moved to real hardware where it will be more
// difficult to track down.
#define CYGNUM_HAL_STACK_SIZE_MINIMUM (16 * 1024)
#define CYGNUM_HAL_STACK_SIZE_TYPICAL (32 * 1024)

//--------------------------------------------------------------------------
// Macros for switching context between two eCos instances (jump from
// code in ROM to code in RAM or vice versa).
#define CYGARC_HAL_SAVE_GP()
#define CYGARC_HAL_RESTORE_GP()

//--------------------------------------------------------------------------
#endif // CYGONCE_HAL_VAR_ARCH_H
// End of var_arch.h
