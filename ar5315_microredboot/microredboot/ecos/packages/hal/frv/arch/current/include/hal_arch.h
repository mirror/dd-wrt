#ifndef CYGONCE_HAL_ARCH_H
#define CYGONCE_HAL_ARCH_H

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
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2004 Red Hat, Inc.
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
// Author(s):    nickg, gthomas
// Contributors: nickg, gthomas
// Date:         2001-09-07
// Purpose:      Define architecture abstractions
// Usage:        #include <cyg/hal/hal_arch.h>

//              
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>         // To decide on stack usage
#include <cyg/infra/cyg_type.h>

#if CYGINT_HAL_FRV_ARCH_FR500 != 0
#define _NGPR 64
#define _NFPR 64
#elif CYGINT_HAL_FRV_ARCH_FR400 != 0
#define _NGPR 32
#define _NFPR 32
#else
#error No architecture defined?
#endif

//--------------------------------------------------------------------------
// Common "special" register definitions

// Processor status register
#define _PSR_PIVL_SHIFT 3
#define _PSR_PIVL_MASK  (0xF<<(_PSR_PIVL_SHIFT))  // Interrupt mask level
#define _PSR_S          (1<<2)                    // Supervisor state
#define _PSR_PS         (1<<1)                    // Previous supervisor state
#define _PSR_ET         (1<<0)                    // Enable interrupts
#define _PSR_CM		(1<<13)			  // Enable conditionals

#define _PSR_INITIAL (_PSR_S|_PSR_PS|_PSR_ET|_PSR_CM)     // Supervisor mode, exceptions

// Hardware status register
#define _HSR0_FRN	(1<<11)
#define _HSR0_GRN	(1<<10)
#define _HSR0_ICE       (1<<31)                   // Instruction cache enable
#define _HSR0_DCE       (1<<30)                   // Data cache enable
#define _HSR0_IMMU      (1<<26)                   // Instruction MMU enable
#define _HSR0_DMMU      (1<<25)                   // Data MMU enable

// Debug Control Register
#define _DCR_EBE        (1 << 30)
#define _DCR_SE         (1 << 29)
#define _DCR_DRBE0      (1 << 19)
#define _DCR_DWBE0      (1 << 18)
#define _DCR_DDBE0      (1 << 17)
#define _DCR_DRBE1      (1 << 16)
#define _DCR_DWBE1      (1 << 15)
#define _DCR_DDBE1      (1 << 14)
#define _DCR_DRBE2      (1 << 13)
#define _DCR_DWBE2      (1 << 12)
#define _DCR_DDBE2      (1 << 11)
#define _DCR_DRBE3      (1 << 10)
#define _DCR_DWBE3      (1 << 9)
#define _DCR_DDBE3      (1 << 8)
#define _DCR_IBE0       (1 << 7)
#define _DCR_IBCE0      (1 << 6)
#define _DCR_IBE1       (1 << 5)
#define _DCR_IBCE1      (1 << 4)
#define _DCR_IBE2       (1 << 3)
#define _DCR_IBCE2      (1 << 2)
#define _DCR_IBE3       (1 << 1)
#define _DCR_IBCE3      (1 << 0)

// Initial contents for special registers

#define _CCR_INITIAL    0
#define _LCR_INITIAL    0
#define _CCCR_INITIAL   0

//--------------------------------------------------------------------------
//
// Saved thread state
//
typedef struct 
{
    cyg_uint32 gpr[_NGPR];  // Saved general purpose registers
    cyg_uint32 pc;          // Current [next] instruction location
    cyg_uint32 psr;         // Processor status register
    cyg_uint32 lr;          // Link register
    cyg_uint32 ccr;         // Condition codes
    cyg_uint32 cccr;
    cyg_uint32 lcr;
    cyg_int32  vector;      // Reason for last exception
} HAL_SavedRegisters;

//-------------------------------------------------------------------------
// Exception handling function.
// This function is defined by the kernel according to this prototype. It is
// invoked from the HAL to deal with any CPU exceptions that the HAL does
// not want to deal with itself. It usually invokes the kernel's exception
// delivery mechanism.

externC void cyg_hal_deliver_exception( CYG_WORD code, CYG_ADDRWORD data );

//-------------------------------------------------------------------------
// Bit manipulation macros

externC int hal_lsbindex(int);
externC int hal_msbindex(int);

#define HAL_LSBIT_INDEX(index, mask) index = hal_lsbindex(mask)
#define HAL_MSBIT_INDEX(index, mask) index = hal_msbindex(mask)

//-------------------------------------------------------------------------
// Context Initialization
// Initialize the context of a thread.
// Arguments:
// _sparg_ name of variable containing current sp, will be changed to new sp
// _thread_ thread object address, passed as argument to entry point
// _entry_ entry point address.
// _id_ bit pattern used in initializing registers, for debugging.

#define HAL_THREAD_INIT_CONTEXT( _sparg_, _thread_, _entry_, _id_ )             \
    CYG_MACRO_START                                                             \
    register CYG_WORD _sp_ = ((CYG_WORD)_sparg_) &~15;                          \
    register HAL_SavedRegisters *_regs_;                                        \
    int _i_;                                                                    \
    _regs_ = (HAL_SavedRegisters *)((_sp_) - sizeof(HAL_SavedRegisters));       \
    for( _i_ = 1;  _i_ < _NGPR;  _i_++)                                         \
        (_regs_)->gpr[_i_] = (_id_)|_i_;                                        \
    (_regs_)->gpr[8] = (CYG_WORD)(_thread_); /* R8 = arg1 = thread ptr */       \
    (_regs_)->gpr[1] = (CYG_WORD)(_sp_);     /* SP = top of stack      */       \
    (_regs_)->lr = (CYG_WORD)(_entry_);      /* LR = entry point       */       \
    (_regs_)->pc = (CYG_WORD)(_entry_);      /* PC = [initial] entry point */   \
    (_regs_)->psr = _PSR_INITIAL;            /* PSR = Interrupt enabled */      \
    (_regs_)->ccr = _CCR_INITIAL;                                               \
    (_regs_)->lcr = _LCR_INITIAL;                                               \
    (_regs_)->ccr = _CCCR_INITIAL;                                              \
    _sparg_ = (CYG_ADDRESS)_regs_;                                              \
    CYG_MACRO_END

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
// HAL_BREAKPOINT() is a code sequence that will cause a breakpoint to happen
// if executed.
// HAL_BREAKINST is the value of the breakpoint instruction and 
// HAL_BREAKINST_SIZE is its size in bytes.

// The choices for breakpoints seem to be:
//    break           0x801000C0
//    tira  gr0,#1    0xC0700001
#ifdef CYGSEM_HAL_FRV_USE_BREAK_INSTRUCTION 
#define HAL_BREAKPOINT(_label_)                   \
asm volatile (" .globl  " #_label_ "\n"            \
              #_label_":\tbreak\n"             \
    );

#define HAL_BREAKINST            0x801000C0
#else
#define HAL_BREAKPOINT(_label_)                   \
asm volatile (" .globl  " #_label_ "\n"            \
              #_label_":\ttira\tgr0,#1\n"             \
    );

#define HAL_BREAKINST            0xC0700001
#endif
#define HAL_BREAKINST_SIZE       4
#define HAL_BREAKINST_TYPE       cyg_uint32

//--------------------------------------------------------------------------
// Thread register state manipulation for GDB support.

// GDB expects the registers in this structure:
//   gr0..gr31, gr32..gr63       - 4 bytes each
//   fpr0..fpr31, fpr32..fpr63   - 4 bytes each
//   pc, psr, ccr, cccr          - 4 bytes each
//   14 <unused>                 - 4 bytes each
//   lr, lcr                     - 4 bytes each

// Translate a stack pointer as saved by the thread context macros above into
// a pointer to a HAL_SavedRegisters structure.
#define HAL_THREAD_GET_SAVED_REGISTERS( _sp_, _regs_ )  \
        (_regs_) = (HAL_SavedRegisters *)(_sp_)

// Copy a set of registers from a HAL_SavedRegisters structure into a
// GDB ordered array.    
#define HAL_GET_GDB_REGISTERS( _aregval_, _regs_ )		\
    CYG_MACRO_START                               		\
    CYG_ADDRWORD *_regval_ = (CYG_ADDRWORD *)(_aregval_);	\
    int _i_;                                 			\
                                             			\
    for( _i_ = 0; _i_ <= _NGPR; _i_++ )      			\
        _regval_[_i_] = (_regs_)->gpr[_i_];  			\
    _regval_[128] = (_regs_)->pc;             			\
    _regval_[129] = (_regs_)->psr;            			\
    _regval_[130] = (_regs_)->ccr;            			\
    _regval_[135] = (_regs_)->vector;          			\
    _regval_[145] = (_regs_)->lr;             			\
    _regval_[146] = (_regs_)->lcr;             			\
    CYG_MACRO_END

// Copy a GDB ordered array into a HAL_SavedRegisters structure.
#define HAL_SET_GDB_REGISTERS( _regs_ , _aregval_ ) 		\
    CYG_MACRO_START                                 		\
    CYG_ADDRWORD *_regval_ = (CYG_ADDRWORD *)(_aregval_);	\
    int _i_;                                  			\
                                              			\
    for( _i_ = 0; _i_ <= _NGPR; _i_++ )        			\
        (_regs_)->gpr[_i_] = _regval_[_i_];   			\
                                              			\
    (_regs_)->pc  = _regval_[128];             			\
    (_regs_)->psr = _regval_[129];             			\
    (_regs_)->ccr = _regval_[130];             			\
    (_regs_)->lr  = _regval_[145];             			\
    (_regs_)->lcr  = _regval_[146];            			\
    CYG_MACRO_END

//--------------------------------------------------------------------------
// HAL setjmp

#define CYGARC_JMP_BUF_SIZE 0x110

typedef cyg_uint32 hal_jmp_buf[CYGARC_JMP_BUF_SIZE];

externC int hal_setjmp(hal_jmp_buf env);
externC void hal_longjmp(hal_jmp_buf env, int val);

//--------------------------------------------------------------------------
// Idle thread code.
// This macro is called in the idle thread loop, and gives the HAL the
// chance to insert code. Typical idle thread behaviour might be to halt the
// processor.

externC void hal_idle_thread_action(cyg_uint32 loop_count);

#define HAL_IDLE_THREAD_ACTION(_count_) hal_idle_thread_action(_count_)

//---------------------------------------------------------------------------

// Minimal and sensible stack sizes: the intention is that applications
// will use these to provide a stack size in the first instance prior to
// proper analysis.  Idle thread stack should be this big.

//    THESE ARE NOT INTENDED TO BE MICROMETRICALLY ACCURATE FIGURES.
//           THEY ARE HOWEVER ENOUGH TO START PROGRAMMING.
// YOU MUST MAKE YOUR STACKS LARGER IF YOU HAVE LARGE "AUTO" VARIABLES!

// This is not a config option because it should not be adjusted except
// under "enough rope" sort of disclaimers.

// A minimal, optimized stack frame, rounded up - no autos
#define CYGNUM_HAL_STACK_FRAME_SIZE (4 * 150)

// Stack needed for a context switch: this is implicit in the estimate for
// interrupts so not explicitly used below:
#define CYGNUM_HAL_STACK_CONTEXT_SIZE (4 * 150)

// Interrupt + call to ISR, interrupt_end() and the DSR
#define CYGNUM_HAL_STACK_INTERRUPT_SIZE \
    ((4 * 150) + 2 * CYGNUM_HAL_STACK_FRAME_SIZE)

// Space for the maximum number of nested interrupts, plus room to call functions
#define CYGNUM_HAL_MAX_INTERRUPT_NESTING 4

#define CYGNUM_HAL_STACK_SIZE_MINIMUM \
        (CYGNUM_HAL_MAX_INTERRUPT_NESTING * CYGNUM_HAL_STACK_INTERRUPT_SIZE + \
         2 * CYGNUM_HAL_STACK_FRAME_SIZE)

#define CYGNUM_HAL_STACK_SIZE_TYPICAL \
        (CYGNUM_HAL_STACK_SIZE_MINIMUM + \
         16 * CYGNUM_HAL_STACK_FRAME_SIZE)


//--------------------------------------------------------------------------
// Macros for switching context between two eCos instances (jump from
// code in ROM to code in RAM or vice versa).
#define CYGARC_HAL_SAVE_GP()
#define CYGARC_HAL_RESTORE_GP()

#endif // CYGONCE_HAL_ARCH_H
// End of hal_arch.h
