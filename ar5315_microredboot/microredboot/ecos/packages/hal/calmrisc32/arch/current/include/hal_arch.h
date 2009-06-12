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
// Author(s):    nickg
// Contributors: nickg, dmoseley
// Date:         1999-02-17
// Purpose:      Define architecture abstractions
// Usage:        #include <cyg/hal/hal_arch.h>
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#ifndef __ASSEMBLER__
#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>

#include <cyg/hal/var_arch.h>

//--------------------------------------------------------------------------
// Processor saved states:
// The layout of this structure is also defined in "arch.inc", for assembly
// code. Do not change this without changing that (or vice versa).
// Notes: This structure is carefully laid out. It is a multiple of 8
// bytes and the pc and badvr fields are positioned to ensure that
// they are on 8 byte boundaries. 


typedef struct 
{
    CYG_WORD32          vector;
    CYG_WORD32          vbr;
    CYG_WORD32          spc_irq;
    CYG_WORD32          spc_fiq;
    CYG_WORD32          spc_swi;
    CYG_WORD32          spc_expt;
    CYG_WORD32          ssr_irq;
    CYG_WORD32          ssr_fiq;
    CYG_WORD32          ssr_swi;
    CYG_WORD32          ssr_expt;
    CYG_WORD32          bank0[16];
    CYG_WORD32          bank1[16];
} HAL_SavedRegisters;

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

// Optional FPU context initialization
#define HAL_THREAD_INIT_FPU_CONTEXT( _regs_, _id_ )

// Initialize the context of a thread.
// Arguments:
// _sparg_ name of variable containing current sp, will be written with new sp
// _thread_ thread object address, passed as argument to entry point
// _entry_ entry point address.
// _id_ bit pattern used in initializing registers, for debugging.
#define HAL_THREAD_INIT_CONTEXT( _sparg_, _thread_, _entry_, _id_ )                     \
{                                                                                       \
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
// HAL_BREAKINST_TYPE is the type.

#define HAL_BREAKPOINT(_label_)                 \
asm volatile (" .globl  " #_label_ "\n"         \
              #_label_":"                       \
              " .short 0x80e0 \n"               \
    );

#define HAL_BREAKINST           0x80e0

#define HAL_BREAKINST_SIZE      2

#define HAL_BREAKINST_TYPE      cyg_uint16

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

// Copy a set of registers from a HAL_SavedRegisters structure into a
// GDB ordered array.    
#define HAL_GET_GDB_REGISTERS( _aregval_ , _regs_ )             \
{                                                               \
    CYG_HAL_GDB_REG *_regval_ = (CYG_HAL_GDB_REG *)(_aregval_); \
    int _i_;                                                    \
                                                                \
    for( _i_ = 0; _i_ < 16; _i_++ )                             \
        _regval_[_i_] = (_regs_)->bank0[_i_];                   \
    for( _i_ = 0; _i_ < 16; _i_++ )                             \
        _regval_[16+_i_] = (_regs_)->bank1[_i_];                \
    _regval_[REG_VBR] = (_regs_)->vbr;                          \
    _regval_[REG_SSR_FIQ] = (_regs_)->ssr_fiq;                  \
    _regval_[REG_SSR_IRQ] = (_regs_)->ssr_irq;                  \
    _regval_[REG_SSR_SWI] = (_regs_)->ssr_swi;                  \
    _regval_[REG_SSR_EXPT] = (_regs_)->ssr_expt;                \
    _regval_[REG_SPC_FIQ] = (_regs_)->spc_fiq;                  \
    _regval_[REG_SPC_IRQ] = (_regs_)->spc_irq;                  \
    _regval_[REG_SPC_SWI] = (_regs_)->spc_swi;                  \
    _regval_[REG_SPC_EXPT] = (_regs_)->spc_expt;                \
    switch ((_regs_)->vector) {                                 \
      case CYGNUM_HAL_VECTOR_SWI:                               \
        _regval_[REG_SR] = (_regs_)->ssr_swi;                   \
        _regval_[REG_PC] = (_regs_)->spc_swi; break;            \
      case CYGNUM_HAL_VECTOR_IRQ:                               \
        _regval_[REG_SR] = (_regs_)->ssr_irq;                   \
        _regval_[REG_PC] = (_regs_)->spc_irq; break;            \
      case CYGNUM_HAL_VECTOR_FIQ:                               \
        _regval_[REG_SR] = (_regs_)->ssr_fiq;                   \
        _regval_[REG_PC] = (_regs_)->spc_fiq; break;            \
      default:                                                  \
        _regval_[REG_SR] = (_regs_)->ssr_expt;                  \
        _regval_[REG_PC] = (_regs_)->spc_expt; break;           \
    }                                                           \
                                                                \
}

// Copy a GDB ordered array into a HAL_SavedRegisters structure.
#define HAL_SET_GDB_REGISTERS( _regs_ , _aregval_ )             \
{                                                               \
    CYG_HAL_GDB_REG *_regval_ = (CYG_HAL_GDB_REG *)(_aregval_); \
    int _i_;                                                    \
                                                                \
    for( _i_ = 0; _i_ < 16; _i_++ )                             \
        (_regs_)->bank0[_i_] = _regval_[_i_];                   \
    for( _i_ = 0; _i_ < 16; _i_++ )                             \
        (_regs_)->bank1[_i_] = _regval_[16+_i_];                \
    (_regs_)->vbr = _regval_[REG_VBR];                          \
    (_regs_)->ssr_fiq = _regval_[REG_SSR_FIQ];                  \
    (_regs_)->ssr_irq = _regval_[REG_SSR_IRQ];                  \
    (_regs_)->ssr_swi = _regval_[REG_SSR_SWI];                  \
    (_regs_)->ssr_expt = _regval_[REG_SSR_EXPT];                \
    (_regs_)->spc_fiq = _regval_[REG_SPC_FIQ];                  \
    (_regs_)->spc_irq = _regval_[REG_SPC_IRQ];                  \
    (_regs_)->spc_swi = _regval_[REG_SPC_SWI];                  \
    (_regs_)->spc_expt = _regval_[REG_SPC_EXPT];                \
    switch (__get_trap_number()) {                              \
      case CYGNUM_HAL_VECTOR_SWI:                               \
        (_regs_)->ssr_swi = _regval_[REG_SR];                   \
        (_regs_)->spc_swi = _regval_[REG_PC]; break;            \
      case CYGNUM_HAL_VECTOR_IRQ:                               \
        (_regs_)->ssr_irq = _regval_[REG_SR];                   \
        (_regs_)->spc_irq = _regval_[REG_PC]; break;            \
      case CYGNUM_HAL_VECTOR_FIQ:                               \
        (_regs_)->ssr_fiq = _regval_[REG_SR];                   \
        (_regs_)->spc_fiq = _regval_[REG_PC]; break;            \
      default:                                                  \
        (_regs_)->ssr_expt = _regval_[REG_SR];                  \
        (_regs_)->spc_expt = _regval_[REG_PC]; break;           \
    }                                                           \
                                                                \
}

#define CYGARC_HAL_GET_PC_REG(_regs_, _val_)                    \
{                                                               \
    switch ((_regs_)->vector) {                                 \
      case CYGNUM_HAL_VECTOR_SWI:                               \
        (_val_) = (_regs_)->spc_swi; break;                     \
      case CYGNUM_HAL_VECTOR_IRQ:                               \
        (_val_) = (_regs_)->spc_irq; break;                     \
      case CYGNUM_HAL_VECTOR_FIQ:                               \
        (_val_) = (_regs_)->spc_fiq; break;                     \
      default:                                                  \
        (_val_) = (_regs_)->spc_expt; break;                    \
    }                                                           \
}

//--------------------------------------------------------------------------
// HAL setjmp
// Note: These definitions are repeated in context.S. If changes are
// required remember to update both sets.

#define CYGARC_JMP_BUF_R4        0
#define CYGARC_JMP_BUF_R5        2
#define CYGARC_JMP_BUF_R12       4
#define CYGARC_JMP_BUF_R13       8
#define CYGARC_JMP_BUF_R14      12
#define CYGARC_JMP_BUF_R15      16

#define CYGARC_JMP_BUF_SIZE     20

typedef cyg_uint16 hal_jmp_buf[CYGARC_JMP_BUF_SIZE/sizeof(cyg_uint16)];

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
#define CYGNUM_HAL_STACK_CONTEXT_SIZE ((32+10)*CYG_HAL_MIPS_REG_SIZE)

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

#endif /* __ASSEMBLER__ */


//--------------------------------------------------------------------------
// Macros for switching context between two eCos instances (jump from
// code in ROM to code in RAM or vice versa).
#define CYGARC_HAL_SAVE_GP()
#define CYGARC_HAL_RESTORE_GP()

//--------------------------------------------------------------------------
// Defines for status register bit access

#define CYGARC_SR_PM   (1<<31)
#define CYGARC_SR_RS1  (1<<30)
#define CYGARC_SR_RS0  (1<<29)
#define CYGARC_SR_BS   (1<<28)
#define CYGARC_SR_TE   (1<<26)
#define CYGARC_SR_FE   (1<<25)
#define CYGARC_SR_IE   (1<<24)



//--------------------------------------------------------------------------
#endif // CYGONCE_HAL_HAL_ARCH_H
// End of hal_arch.h
