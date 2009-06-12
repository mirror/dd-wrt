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
// Author(s):     nickg
// Contributors:  nickg, dmoseley
// Date:          1999-02-18
// Purpose:       Define architecture abstractions
// Usage:         #include <cyg/hal/hal_arch.h>
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>

#include <cyg/hal/var_arch.h>

//--------------------------------------------------------------------------
// Exception handling function.
// This function is defined by the kernel according to this prototype. It is
// invoked from the HAL to deal with any CPU exceptions that the HAL does
// not want to deal with itself. It usually invokes the kernel's exception
// delivery mechanism.

externC void cyg_hal_deliver_exception( CYG_WORD code, CYG_ADDRWORD data );

//--------------------------------------------------------------------------
// Bit manipulation routines

externC cyg_uint32 hal_lsbit_index(cyg_uint32 mask);
externC cyg_uint32 hal_msbit_index(cyg_uint32 mask);

#define HAL_LSBIT_INDEX(index, mask) index = hal_lsbit_index(mask);

#define HAL_MSBIT_INDEX(index, mask) index = hal_msbit_index(mask);

//--------------------------------------------------------------------------
// Context Initialization
// Initialize the context of a thread.
// Arguments:
// _sp_ name of variable containing current sp, will be written with new sp
// _thread_ thread object address, passed as argument to entry point
// _entry_ entry point address.
// _id_ bit pattern used in initializing registers, for debugging.

#ifndef HAL_THREAD_INIT_CONTEXT_EXTRA
#define HAL_THREAD_INIT_CONTEXT_EXTRA(_regs_, _id_)
#endif

#define HAL_THREAD_INIT_CONTEXT( _sp_, _thread_, _entry_, _id_ )            \
{                                                                           \
    register HAL_SavedRegisters *_regs_;                                    \
    _regs_ = (HAL_SavedRegisters *)(((CYG_ADDRWORD)(_sp_)&~15) -            \
                                    sizeof(HAL_SavedRegisters)*2);          \
    HAL_THREAD_INIT_CONTEXT_EXTRA(_regs_, _id_);                            \
    _regs_->d0    = (CYG_WORD)(_thread_);                                   \
    _regs_->d1    = (_id_)|0xddd1;                                          \
    _regs_->d2    = (_id_)|0xddd2;                                          \
    _regs_->d3    = (_id_)|0xddd3;                                          \
    _regs_->a0    = (_id_)|0xaaa0;                                          \
    _regs_->a1    = (_id_)|0xaaa1;                                          \
    _regs_->a2    = (_id_)|0xaaa2;                                          \
    _regs_->a3    = (_id_)|0xaaa3;                                          \
    _regs_->mdr   = 0;                                                      \
    _regs_->lir   = 0;                                                      \
    _regs_->lar   = 0;                                                      \
    _regs_->psw   = 0x0000F00;                                              \
    _regs_->pc    = (CYG_WORD)(_entry_);                                    \
    _sp_          = (CYG_ADDRESS)_regs_;                                    \
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
        hal_thread_switch_context((CYG_ADDRESS)_tspptr_,                \
                                  (CYG_ADDRESS)_fspptr_);

#define HAL_THREAD_LOAD_CONTEXT(_tspptr_)                               \
        hal_thread_load_context( (CYG_ADDRESS)_tspptr_ );

//--------------------------------------------------------------------------
// Execution reorder barrier.
// When optimizing the compiler can reorder code. In multithreaded systems
// where the order of actions is vital, this can sometimes cause problems.
// This macro may be inserted into places where reordering should not happen.

#define HAL_REORDER_BARRIER() asm volatile ( "" : : : "memory" )

//--------------------------------------------------------------------------
// Breakpoint support
// HAL_BREAKPOINT() is a code sequence that will cause a breakpoint to
// happen if executed.
// HAL_BREAKINST is the value of the breakpoint instruction and
// HAL_BREAKINST_SIZE is its size in bytes.
// HAL_BREAKINST_TYPE is the type.

#define HAL_BREAKPOINT(_label_)                 \
asm volatile (" .globl  _" #_label_ ";"         \
              "_"#_label_":"                    \
              ".byte 0xFF"                      \
    );

#define HAL_BREAKINST           0xFF

#define HAL_BREAKINST_SIZE      1

#define HAL_BREAKINST_TYPE      cyg_uint8

//--------------------------------------------------------------------------
// Thread register state manipulation for GDB support.

// Translate a stack pointer as saved by the thread context macros above into
// a pointer to a HAL_SavedRegisters structure.
#define HAL_THREAD_GET_SAVED_REGISTERS( _sp_, _regs_ )                     \
        (_regs_) = (HAL_SavedRegisters *)(_sp_)

#ifndef HAL_GET_GDB_EXTRA_REGISTERS
#define HAL_GET_GDB_EXTRA_REGISTERS( _regval_, _regs_ )
#endif
#ifndef HAL_SET_GDB_EXTRA_REGISTERS
#define HAL_SET_GDB_EXTRA_REGISTERS( _regs_, _regval_ )
#endif


// Copy a set of registers from a HAL_SavedRegisters structure into a
// GDB ordered array.    
//
// The CYGMON version should differ by also handling SP and PSW
// since we will be using a different stack.
#ifdef CYGPKG_CYGMON
#define HAL_GET_GDB_REGISTERS( _aregval_ , _regs_ )             \
{                                                               \
    CYG_ADDRWORD *_regval_ = (CYG_ADDRWORD *)(_aregval_);       \
                                                                \
    _regval_[0]         = (_regs_)->d0;                         \
    _regval_[1]         = (_regs_)->d1;                         \
    _regval_[2]         = (_regs_)->d2;                         \
    _regval_[3]         = (_regs_)->d3;                         \
    _regval_[4]         = (_regs_)->a0;                         \
    _regval_[5]         = (_regs_)->a1;                         \
    _regval_[6]         = (_regs_)->a2;                         \
    _regval_[7]         = (_regs_)->a3;                         \
                                                                \
    _regval_[8]         = (_regs_)->sp;                         \
    _regval_[9]         = (_regs_)->pc;                         \
    _regval_[10]        = (_regs_)->mdr;                        \
    _regval_[11]        = (_regs_)->psw;                        \
                                                                \
    _regval_[12]        = (_regs_)->lar;                        \
    _regval_[13]        = (_regs_)->lir;                        \
    HAL_GET_GDB_EXTRA_REGISTERS( _regval_, _regs_ );            \
}
#else
#define HAL_GET_GDB_REGISTERS( _aregval_ , _regs_ )             \
{                                                               \
    CYG_ADDRWORD *_regval_ = (CYG_ADDRWORD *)(_aregval_);       \
                                                                \
    _regval_[0]         = (_regs_)->d0;                         \
    _regval_[1]         = (_regs_)->d1;                         \
    _regval_[2]         = (_regs_)->d2;                         \
    _regval_[3]         = (_regs_)->d3;                         \
    _regval_[4]         = (_regs_)->a0;                         \
    _regval_[5]         = (_regs_)->a1;                         \
    _regval_[6]         = (_regs_)->a2;                         \
    _regval_[7]         = (_regs_)->a3;                         \
                                                                \
    _regval_[8] = (_regs_)->sp = (CYG_ADDRWORD)(_regs_) +       \
                                 sizeof(HAL_SavedRegisters);    \
    _regval_[9]         = (_regs_)->pc;                         \
    _regval_[10]        = (_regs_)->mdr;                        \
    _regval_[11]        = (_regs_)->psw;                        \
                                                                \
    _regval_[12]        = (_regs_)->lar;                        \
    _regval_[13]        = (_regs_)->lir;                        \
    HAL_GET_GDB_EXTRA_REGISTERS( _regval_, _regs_ );            \
}
#endif

// Copy a GDB ordered array into a HAL_SavedRegisters structure.
//
// The CYGMON version should differ by also handling SP and PSW
// since we will be using a different stack.
#ifdef CYGPKG_CYGMON
#define HAL_SET_GDB_REGISTERS( _regs_ , _aregval_ )                     \
{                                                                       \
    CYG_ADDRWORD *_regval_ = (CYG_ADDRWORD *)(_aregval_);               \
                                                                        \
    (_regs_)->d0             = _regval_[0];                             \
    (_regs_)->d1             = _regval_[1];                             \
    (_regs_)->d2             = _regval_[2];                             \
    (_regs_)->d3             = _regval_[3];                             \
    (_regs_)->a0             = _regval_[4];                             \
    (_regs_)->a1             = _regval_[5];                             \
    (_regs_)->a2             = _regval_[6];                             \
    (_regs_)->a3             = _regval_[7];                             \
                                                                        \
    (_regs_)->sp             = _regval_[8];                             \
    (_regs_)->pc             = _regval_[9];                             \
    (_regs_)->mdr            = _regval_[10];                            \
    (_regs_)->psw            = _regval_[11];                            \
                                                                        \
    (_regs_)->lar            = _regval_[12];                            \
    (_regs_)->lir            = _regval_[13];                            \
                                                                        \
    HAL_SET_GDB_EXTRA_REGISTERS( _regs_, _regval_ );                    \
}
#else
#define HAL_SET_GDB_REGISTERS( _regs_ , _aregval_ )                     \
{                                                                       \
    CYG_ADDRWORD *_regval_ = (CYG_ADDRWORD *)(_aregval_);               \
                                                                        \
    (_regs_)->d0             = _regval_[0];                             \
    (_regs_)->d1             = _regval_[1];                             \
    (_regs_)->d2             = _regval_[2];                             \
    (_regs_)->d3             = _regval_[3];                             \
    (_regs_)->a0             = _regval_[4];                             \
    (_regs_)->a1             = _regval_[5];                             \
    (_regs_)->a2             = _regval_[6];                             \
    (_regs_)->a3             = _regval_[7];                             \
                                                                        \
    (_regs_)->pc              = _regval_[9];                            \
    (_regs_)->mdr             = _regval_[10];                           \
                                                                        \
    (_regs_)->lar             = _regval_[12];                           \
    (_regs_)->lir             = _regval_[13];                           \
                                                                        \
    /* We do not allow the SP or PSW to be set. Changing the SP will    \
     * mess up the saved state. No PSW is saved on thread context       \
     * switches, so there is nowhere to save it to.                     \
     */                                                                 \
                                                                        \
     HAL_SET_GDB_EXTRA_REGISTERS( _regs_, _regval_ );                   \
}
#endif

//-------------------------------------------------------------------------
// HAL setjmp
// Note: These definitions are repeated in context.S. If changes are required
// remember to update both sets.

#define CYGARC_JMP_BUF_SP        0
#define CYGARC_JMP_BUF_D2        1
#define CYGARC_JMP_BUF_D3        2
#define CYGARC_JMP_BUF_A2        3
#define CYGARC_JMP_BUF_A3        4
#define CYGARC_JMP_BUF_LR        5

#define CYGARC_JMP_BUF_SIZE      6

typedef cyg_uint32 hal_jmp_buf[CYGARC_JMP_BUF_SIZE];

externC int hal_setjmp(hal_jmp_buf env);
externC void hal_longjmp(hal_jmp_buf env, int val);

//-------------------------------------------------------------------------
// Idle thread code.
// This macro is called in the idle thread loop, and gives the HAL the
// chance to insert code. Typical idle thread behaviour might be to halt the
// processor.

externC void hal_idle_thread_action(cyg_uint32 loop_count);

#define HAL_IDLE_THREAD_ACTION(_count_) hal_idle_thread_action(_count_)

//-----------------------------------------------------------------------------
// Minimal and sensible stack sizes: the intention is that applications
// will use these to provide a stack size in the first instance prior to
// proper analysis.  Idle thread stack should be this big.

//    THESE ARE NOT INTENDED TO BE MICROMETRICALLY ACCURATE FIGURES.
//           THEY ARE HOWEVER ENOUGH TO START PROGRAMMING.
// YOU MUST MAKE YOUR STACKS LARGER IF YOU HAVE LARGE "AUTO" VARIABLES!

// We define quite large stack needs for SPARClite, for it requires 576
// bytes (144 words) to process an interrupt and thread-switch, and
// momentarily, but needed in case of recursive interrupts, it needs 208
// words - if a sequence of saves to push out other regsets is interrupted.

// This is not a config option because it should not be adjusted except
// under "enough rope" sort of disclaimers.

// Worst case stack frame size: return link + 4 args + 4 pushed registers.
#define CYGNUM_HAL_STACK_FRAME_SIZE (40)

// Stack needed for a context switch:
#define CYGNUM_HAL_STACK_CONTEXT_SIZE (60)

// Interrupt + call to ISR, interrupt_end() and the DSR
#define CYGNUM_HAL_STACK_INTERRUPT_SIZE (128)

#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK 

// An interrupt stack which is large enough for all possible interrupt
// conditions (and only used for that purpose) exists.  "User" stacks
// can be much smaller

#define CYGNUM_HAL_STACK_SIZE_MINIMUM (CYGNUM_HAL_STACK_CONTEXT_SIZE+      \
                                       CYGNUM_HAL_STACK_INTERRUPT_SIZE*2+  \
                                       CYGNUM_HAL_STACK_FRAME_SIZE*16)
#define CYGNUM_HAL_STACK_SIZE_TYPICAL (2048)

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
// EOF hal_arch.h
