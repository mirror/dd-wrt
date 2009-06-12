#ifndef CYGONCE_HAL_ARCH_H
#define CYGONCE_HAL_ARCH_H

//=============================================================================
//
//      hal_arch.h
//
//      Architecture specific abstractions
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2004 Gary Thomas
// Copyright (C) 2004 Jonathan Larmour <jifl@eCosCentric.com>
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
// Author(s):   nickg
// Contributors:  nickg
// Date:        1997-09-08
// Purpose:     Define architecture abstractions
// Usage:       #include <cyg/hal/hal_arch.h>

//              
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>

#include <cyg/hal/ppc_regs.h>           // CYGARC_REG_MSR_EE

//-----------------------------------------------------------------------------
// Processor saved states:

typedef struct 
{
#ifdef CYGDBG_HAL_POWERPC_FRAME_WALLS
    cyg_uint32   wall_head;
#endif

    // These are common to all saved states
    cyg_uint32   d[32];                 // Data regs
#ifdef CYGHWR_HAL_POWERPC_FPU
    double       f[32];                 // Floating point registers
#endif   
    cyg_uint32   cr;                    // Condition Reg
    cyg_uint32   xer;                   // XER
    cyg_uint32   lr;                    // Link Reg
    cyg_uint32   ctr;                   // Count Reg

    // These are saved for exceptions and interrupts, but may also
    // be saved in a context switch if thread-aware debugging is enabled.
    cyg_uint32   msr;                   // Machine State Reg
    cyg_uint32   pc;                    // Program Counter

    // This marks the limit of state saved during a context switch and
    // is used to calculate necessary stack allocation for context switches.
    // It would probably be better to have a union instead...
    cyg_uint32   context_size[0];

    // These are only saved for exceptions and interrupts
    cyg_uint32   vector;                // Vector number

#ifdef CYGDBG_HAL_POWERPC_FRAME_WALLS
    cyg_uint32   wall_tail;
#endif
} HAL_SavedRegisters;

//-----------------------------------------------------------------------------
// Exception handling function.
// This function is defined by the kernel according to this prototype. It is
// invoked from the HAL to deal with any CPU exceptions that the HAL does
// not want to deal with itself. It usually invokes the kernel's exception
// delivery mechanism.

externC void cyg_hal_deliver_exception( CYG_WORD code, CYG_ADDRWORD data );

//-----------------------------------------------------------------------------
// Bit manipulation macros

#define HAL_LSBIT_INDEX(index, mask)    \
    asm ( "neg    11,%1;"               \
          "and    11,11,%1;"            \
          "cntlzw %0,11;"               \
          "subfic %0,%0,31;"            \
          : "=r" (index)                \
          : "r" (mask)                  \
          : "r11"                       \
        );

#define HAL_MSBIT_INDEX(index, mask)            \
    asm ( "cntlzw %0,%1\n"                      \
          "subfic %0,%0,31;"                    \
          : "=r" (index)                        \
          : "r" (mask)                          \
        );

//-----------------------------------------------------------------------------
// eABI
#define CYGARC_PPC_STACK_FRAME_SIZE     56      // size of a stack frame

//-----------------------------------------------------------------------------
// Context Initialization
// Initialize the context of a thread.
// Arguments:
// _sparg_ name of variable containing current sp, will be written with new sp
// _thread_ thread object address, passed as argument to entry point
// _entry_ entry point address.
// _id_ bit pattern used in initializing registers, for debugging.

#define HAL_THREAD_INIT_CONTEXT( _sparg_, _thread_, _entry_, _id_ )           \
    CYG_MACRO_START                                                           \
    register CYG_WORD _sp_ = (((CYG_WORD)_sparg_) &~15)                       \
                                 - CYGARC_PPC_STACK_FRAME_SIZE;               \
    register HAL_SavedRegisters *_regs_;                                      \
    int _i_;                                                                  \
    ((CYG_WORD *)_sp_)[0] = 0;            /* Zero old FP and LR for EABI */   \
    ((CYG_WORD *)_sp_)[1] = 0;            /* to make GDB backtraces sane */   \
    _regs_ = (HAL_SavedRegisters *)((_sp_) - sizeof(HAL_SavedRegisters));     \
    for( _i_ = 0; _i_ < 32; _i_++ ) (_regs_)->d[_i_] = (_id_)|_i_;            \
    (_regs_)->d[01] = (CYG_WORD)(_sp_);        /* SP = top of stack      */   \
    (_regs_)->d[03] = (CYG_WORD)(_thread_);    /* R3 = arg1 = thread ptr */   \
    (_regs_)->cr = 0;                          /* CR = 0                 */   \
    (_regs_)->xer = 0;                         /* XER = 0                */   \
    (_regs_)->lr = (CYG_WORD)(_entry_);        /* LR = entry point       */   \
    (_regs_)->pc = (CYG_WORD)(_entry_);        /* set PC for thread dbg  */   \
    (_regs_)->ctr = 0;                         /* CTR = 0                */   \
    (_regs_)->msr = CYGARC_REG_MSR_EE;         /* MSR = enable irqs      */   \
    _sparg_ = (CYG_ADDRESS)_regs_;                                            \
    CYG_MACRO_END

//-----------------------------------------------------------------------------
// Context switch macros.
// The arguments are pointers to locations where the stack pointer
// of the current thread is to be stored, and from where the sp of the
// next thread is to be fetched.

externC void hal_thread_switch_context( CYG_ADDRESS to, CYG_ADDRESS from );
externC void hal_thread_load_context( CYG_ADDRESS to )
    __attribute__ ((noreturn));

#define HAL_THREAD_SWITCH_CONTEXT(_fspptr_,_tspptr_)                    \
        hal_thread_switch_context((CYG_ADDRESS)_tspptr_,(CYG_ADDRESS)_fspptr_);

#define HAL_THREAD_LOAD_CONTEXT(_tspptr_)                               \
        hal_thread_load_context( (CYG_ADDRESS)_tspptr_ );

//-----------------------------------------------------------------------------
// Execution reorder barrier.
// When optimizing the compiler can reorder code. In multithreaded systems
// where the order of actions is vital, this can sometimes cause problems.
// This macro may be inserted into places where reordering should not happen.

#define HAL_REORDER_BARRIER() asm volatile ( "" : : : "memory" )

//-----------------------------------------------------------------------------
// Breakpoint support
// HAL_BREAKPOINT() is a code sequence that will cause a breakpoint to happen
// if executed.
// HAL_BREAKINST is the value of the breakpoint instruction and 
// HAL_BREAKINST_SIZE is its size in bytes.

#define HAL_BREAKPOINT(_label_)                 \
asm volatile (" .globl  " #_label_ ";"          \
              #_label_":"                       \
              " trap"                           \
    );

#define HAL_BREAKINST           0x7d821008

#define HAL_BREAKINST_SIZE      4

//-----------------------------------------------------------------------------
// Thread register state manipulation for GDB support.
typedef struct {
    cyg_uint32  gpr[32];     // General purpose registers
	double      f0[16];      // First sixteen floating point regs
	cyg_uint32  pc;
	cyg_uint32  msr;
	cyg_uint32  cr;
	cyg_uint32  lr;
	cyg_uint32  ctr;
	cyg_uint32  xer;
	cyg_uint32  mq;
#ifdef CYGHWR_HAL_POWERPC_FPU
	double     f16[16];      // Last sixteen floating point regs
	                         // Could probably also be inserted in the middle
	                         // Adding them at the end minimises the risk of
	                         // breaking existing implementations that do not
	                         // have floating point registers.
#endif
} GDB_Registers;

// Translate a stack pointer as saved by the thread context macros above into
// a pointer to a HAL_SavedRegisters structure.
#define HAL_THREAD_GET_SAVED_REGISTERS( _sp_, _regs_ )  \
        (_regs_) = (HAL_SavedRegisters *)(_sp_)

// Copy floating point registers from a HAL_SavedRegisters structure into a
// GDB_Registers structure
#ifdef CYGHWR_HAL_POWERPC_FPU
#define HAL_GET_GDB_FLOATING_POINT_REGISTERS( _gdb_, _regs_ ) \
	CYG_MACRO_START                                           \
	double * _p_ = _gdb_->f0;                                 \
    double * _q_ = _regs_->f;                                 \
    for( _i_ = 0; _i_ < 16; _i_++)                            \
	  *_p_++ = *_q_++;                                        \
	                                                          \
    _p_ = _gdb_->f16;                                         \
    for( _i_ = 0; _i_ < 16; _i_++)                            \
	  *_p_++ = *_q_++;                                        \
	CYG_MACRO_END
#else
#define HAL_GET_GDB_FLOATING_POINT_REGISTERS( _gdb_, _regs_ ) \
	CYG_MACRO_START                                           \
	CYG_MACRO_END
#endif

// Copy a GDB_Registers structure into a HAL_SavedRegisters structure
#ifdef CYGHWR_HAL_POWERPC_FPU
#define HAL_SET_GDB_FLOATING_POINT_REGISTERS( _regs_, _gdb_) \
	CYG_MACRO_START                                          \
	double * _p_ = _regs_->f;                                \
	double * _q_ = _gdb_->f0;                                \
	for( _i_ = 0; _i_ < 16; _i_++)                           \
	  *_p_++ = *_q_++;                                       \
                                                             \
	_q_ = _gdb_->f16;                                        \
	for( _i_ = 0; _i_ < 16; _i_++)                           \
	  *_p_++ = *_q_++;                                       \
	CYG_MACRO_END
#else
#define HAL_SET_GDB_FLOATING_POINT_REGISTERS( _regs_, _gdb_)  \
	CYG_MACRO_START                                           \
	CYG_MACRO_END
#endif
	
// Copy a set of registers from a HAL_SavedRegisters structure into a
// GDB ordered array.    
#define HAL_GET_GDB_REGISTERS( _aregval_, _regs_ )              \
    CYG_MACRO_START                                             \
    union __gdbreguniontype {                                   \
      __typeof__(_aregval_) _aregval2_;                         \
      GDB_Registers *_gdbr;                                     \
    } __gdbregunion;                                            \
    __gdbregunion._aregval2_ = (_aregval_);                     \
    GDB_Registers *_gdb_ = __gdbregunion._gdbr;                 \
    int _i_;                                                    \
                                                                \
    for( _i_ = 0; _i_ < 32; _i_++ )                             \
        _gdb_->gpr[_i_] = (_regs_)->d[_i_];                     \
                                                                \
    _gdb_->pc    = (_regs_)->pc;                                \
    _gdb_->msr   = (_regs_)->msr;                               \
    _gdb_->cr    = (_regs_)->cr;                                \
    _gdb_->lr    = (_regs_)->lr;                                \
    _gdb_->ctr   = (_regs_)->ctr;                               \
    _gdb_->xer   = (_regs_)->xer;                               \
    HAL_GET_GDB_FLOATING_POINT_REGISTERS(_gdb_, _regs_);        \
    CYG_MACRO_END

// Copy a GDB ordered array into a HAL_SavedRegisters structure.
#define HAL_SET_GDB_REGISTERS( _regs_ , _aregval_ )             \
    CYG_MACRO_START                                             \
    union __gdbreguniontype {                                   \
      __typeof__(_aregval_) _aregval2_;                         \
      GDB_Registers *_gdbr;                                     \
    } __gdbregunion;                                            \
    __gdbregunion._aregval2_ = (_aregval_);                     \
    GDB_Registers *_gdb_ = __gdbregunion._gdbr;                 \
    int _i_;                                                    \
                                                                \
    for( _i_ = 0; _i_ < 32; _i_++ )                             \
        (_regs_)->d[_i_] = _gdb_->gpr[_i_];                     \
                                                                \
    (_regs_)->pc  = _gdb_->pc;                                  \
    (_regs_)->msr = _gdb_->msr;                                 \
    (_regs_)->cr  = _gdb_->cr;                                  \
    (_regs_)->lr  = _gdb_->lr;                                  \
    (_regs_)->ctr = _gdb_->ctr;                                 \
    (_regs_)->xer = _gdb_->xer;                                 \
    HAL_SET_GDB_FLOATING_POINT_REGISTERS(_regs_, _gdb_);        \
    CYG_MACRO_END

//-----------------------------------------------------------------------------
// HAL setjmp

typedef struct {
    cyg_uint32 sp;
    cyg_uint32 r2;
    cyg_uint32 r13;
    cyg_uint32 r14;
    cyg_uint32 r15;
    cyg_uint32 r16;
    cyg_uint32 r17;
    cyg_uint32 r18;
    cyg_uint32 r19;
    cyg_uint32 r20;
    cyg_uint32 r21;
    cyg_uint32 r22;
    cyg_uint32 r23;
    cyg_uint32 r24;
    cyg_uint32 r25;
    cyg_uint32 r26;
    cyg_uint32 r27;
    cyg_uint32 r28;
    cyg_uint32 r29;
    cyg_uint32 r30;
    cyg_uint32 r31;
#ifdef CYGHWR_HAL_POWERPC_FPU
    double     f14;
    double     f15;
    double     f16;
    double     f17;
    double     f18;
    double     f19;
    double     f20;
    double     f21;
    double     f22;
    double     f23;
    double     f24;
    double     f25;
    double     f26;
    double     f27;
    double     f28;
    double     f29;
    double     f30;
    double     f31;
#endif
    cyg_uint32 lr;
    cyg_uint32 cr;
} hal_jmp_buf_t;

#define CYGARC_JMP_BUF_SIZE      (sizeof(hal_jmp_buf_t) / sizeof(cyg_uint32))

typedef cyg_uint32 hal_jmp_buf[ CYGARC_JMP_BUF_SIZE ];

externC int hal_setjmp(hal_jmp_buf env);
externC void hal_longjmp(hal_jmp_buf env, int val);

//-----------------------------------------------------------------------------
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
 
// This is not a config option because it should not be adjusted except
// under "enough rope" sort of disclaimers.
 
// Stack frame overhead per call. The PPC ABI defines regs 13..31 as callee
// saved. callee saved variables are irrelevant for us as they would contain
// automatic variables, so we only count the caller-saved regs here
// So that makes r0..r12 + cr, xer, lr, ctr:
#define CYGNUM_HAL_STACK_FRAME_SIZE (4 * 17)

// Stack needed for a context switch
#define CYGNUM_HAL_STACK_CONTEXT_SIZE \
    (38*4 /* offsetof(HAL_SavedRegisters, context_size) */)

// Interrupt + call to ISR, interrupt_end() and the DSR
#define CYGNUM_HAL_STACK_INTERRUPT_SIZE \
    ((43*4 /* sizeof(HAL_SavedRegisters) */) + 2 * CYGNUM_HAL_STACK_FRAME_SIZE)

// We have lots of registers so no particular amount is added in for
// typical local variable usage.

// We define a minimum stack size as the minimum any thread could ever
// legitimately get away with. We can throw asserts if users ask for less
// than this. Allow enough for three interrupt sources - clock, serial and
// one other

#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK 

// An interrupt stack which is large enough for all possible interrupt
// conditions (and only used for that purpose) exists.  "User" stacks
// can therefore be much smaller

# define CYGNUM_HAL_STACK_SIZE_MINIMUM \
         (16*CYGNUM_HAL_STACK_FRAME_SIZE + 2*CYGNUM_HAL_STACK_INTERRUPT_SIZE)

#else

// No separate interrupt stack exists.  Make sure all threads contain
// a stack sufficiently large
# define CYGNUM_HAL_STACK_SIZE_MINIMUM                  \
        (((2+3)*CYGNUM_HAL_STACK_INTERRUPT_SIZE) +      \
         (16*CYGNUM_HAL_STACK_FRAME_SIZE))
#endif

// Now make a reasonable choice for a typical thread size. Pluck figures
// from thin air and say 30 call frames with an average of 16 words of
// automatic variables per call frame
#define CYGNUM_HAL_STACK_SIZE_TYPICAL                \
        (CYGNUM_HAL_STACK_SIZE_MINIMUM +             \
         30 * (CYGNUM_HAL_STACK_FRAME_SIZE+(16*4)))

//--------------------------------------------------------------------------
// Macros for switching context between two eCos instances (jump from
// code in ROM to code in RAM or vice versa).

// Should be defined like for MIPS, saving/restoring R2 - but is it
// actually used? I've never seen app code use R2. Something to investigate.
#define CYGARC_HAL_SAVE_GP()
#define CYGARC_HAL_RESTORE_GP()

//-----------------------------------------------------------------------------
#endif // CYGONCE_HAL_ARCH_H
// End of hal_arch.h
