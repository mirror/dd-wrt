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
// Author(s):    sfurman
// Contributors: 
// Date:         2003-01-17
// Purpose:      Define architecture abstractions
// Usage:        #include <cyg/hal/hal_arch.h>
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#ifndef CYGONCE_HAL_HAL_ARCH_H
#define CYGONCE_HAL_HAL_ARCH_H

// Include macros to access special-purpose registers (SPRs)
#include <cyg/hal/spr_defs.h>

#define CYG_HAL_OPENRISC_REG_SIZE 4

#ifndef __ASSEMBLER__
#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>

//--------------------------------------------------------------------------
// Processor saved states:
// The layout of this structure is also defined in "arch.inc", for assembly
// code. Do not change this without changing that (or vice versa).

#define CYG_HAL_OPENRISC_REG CYG_WORD32

typedef struct 
{
    // These are common to all saved states
    CYG_HAL_OPENRISC_REG    r[32];          // GPR regs
    CYG_HAL_OPENRISC_REG    machi;          // High and low words of
    CYG_HAL_OPENRISC_REG    maclo;          //   multiply/accumulate reg
    
    // These are only saved for exceptions and interrupts
    CYG_WORD32              vector;         /* Vector number            */
    CYG_WORD32              sr;             /* Status Reg               */
    CYG_HAL_OPENRISC_REG    pc;             /* Program Counter          */

    // Saved only for exceptions, and not restored when continued:
    // Effective address of instruction/data access that caused exception
    CYG_HAL_OPENRISC_REG    eear;           /* Exception effective address reg */
} HAL_SavedRegisters;

//--------------------------------------------------------------------------
//  Utilities

// Move from architecture special register (SPR)
#define MFSPR(_spr_)                                      \
({  CYG_HAL_OPENRISC_REG _result_;                        \
    asm volatile ("l.mfspr %0, r0, %1;"                   \
        : "=r"(_result_)                                  \
        : "K"(_spr_)                                      \
    );                                                    \
    _result_;})

// Move data to architecture special registers (SPR)
#define MTSPR(_spr_, _val_)                               \
CYG_MACRO_START                                           \
    CYG_HAL_OPENRISC_REG val = _val_;                     \
    asm volatile ("l.mtspr r0, %0, %1;"                   \
        :                                                 \
        : "r"(val), "K"(_spr_)                            \
    );                                                    \
CYG_MACRO_END

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

// NOTE - Below can be optimized with l.ff1 instruction if that optional
//        instruction is implemented in HW.  OR12k does not implement
//        it at this time, however.
#define HAL_MSBIT_INDEX(index, mask) index = hal_msbit_index(mask);

//--------------------------------------------------------------------------
// Context Initialization


// Initialize the context of a thread.
// Arguments:
// _sparg_ name of variable containing current sp, will be written with new sp
// _thread_ thread object address, passed as argument to entry point
// _entry_ entry point address.
// _id_ bit pattern used in initializing registers, for debugging.
#define HAL_THREAD_INIT_CONTEXT( _sparg_, _thread_, _entry_, _id_ )                     \
{                                                                                       \
    int _i_;                                                                            \
    register CYG_WORD _sp_ = ((CYG_WORD)_sparg_);                                       \
    register HAL_SavedRegisters *_regs_;                                                \
    _regs_ = (HAL_SavedRegisters *)(((_sp_) - sizeof(HAL_SavedRegisters)) & ~(CYGARC_ALIGNMENT));\
    _sp_ &= ~(CYGARC_ALIGNMENT);                                                        \
    for( _i_ = 1; _i_ < 32; _i_++ ) (_regs_)->r[_i_] = (_id_)|_i_;                      \
    (_regs_)->r[1] = (CYG_HAL_OPENRISC_REG)(_sp_);       /* SP = top of stack      */   \
    (_regs_)->r[2] = (CYG_HAL_OPENRISC_REG)(_sp_);       /* FP = top of stack      */   \
    (_regs_)->r[3] = (CYG_HAL_OPENRISC_REG)(_thread_);   /* R3 = arg1 = thread ptr */   \
    (_regs_)->maclo = 0;                                 /* MACLO = 0              */   \
    (_regs_)->machi = 0;                                 /* MACHI = 0              */   \
    (_regs_)->sr = (SPR_SR_TEE|SPR_SR_IEE);              /* Interrupts enabled     */   \
    (_regs_)->pc = (CYG_HAL_OPENRISC_REG)(_entry_);      /* PC = entry point       */   \
    (_regs_)->r[9] = (CYG_HAL_OPENRISC_REG)(_entry_);    /* PC = entry point       */   \
    _sparg_ = (CYG_ADDRESS)_regs_;                                                      \
}

//--------------------------------------------------------------------------
// Context switch macros.

// The arguments to these macros are *pointers* to locations where the
// stack pointer of the thread is to be stored/retrieved, i.e. *not*
// the value of the stack pointer itself.

externC void hal_thread_switch_context( CYG_ADDRESS to, CYG_ADDRESS from );
externC void hal_thread_load_context( CYG_ADDRESS to )
    __attribute__ ((noreturn));

#define HAL_THREAD_SWITCH_CONTEXT(_fspptr_,_tspptr_)                    \
        hal_thread_switch_context( (CYG_ADDRESS)_tspptr_,               \
                                   (CYG_ADDRESS)_fspptr_);

#define HAL_THREAD_LOAD_CONTEXT(_tspptr_)                               \
        hal_thread_load_context( (CYG_ADDRESS)_tspptr_ );

// Translate a stack pointer as saved by the thread context macros above into
// a pointer to a HAL_SavedRegisters structure.
#define HAL_THREAD_GET_SAVED_REGISTERS( _sp_, _regs_ )  \
        (_regs_) = (HAL_SavedRegisters *)(_sp_)

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
//    occur if executed.
// HAL_BREAKINST is the value of the breakpoint instruction and...
// HAL_BREAKINST_SIZE is its size in bytes and...
// HAL_BREAKINST_TYPE is its type.

#define HAL_BREAKPOINT(_label_)                 \
asm volatile (" .globl  _" #_label_ ";"         \
              "_" #_label_ ":"                  \
              " l.trap 1;"                      \
    );

#define HAL_BREAKINST           (0x21000001)    // l.trap 1 instruction

#define HAL_BREAKINST_SIZE      4

#define HAL_BREAKINST_TYPE      cyg_uint32

//--------------------------------------------------------------------------
// Thread register state manipulation for GDB support.

// Default to a 32 bit register size for GDB register dumps.
#ifndef CYG_HAL_GDB_REG
#define CYG_HAL_GDB_REG CYG_WORD32
#endif

// Register layout expected by GDB
typedef struct 
{
    CYG_HAL_OPENRISC_REG    r[32];          // GPR regs
    CYG_HAL_OPENRISC_REG    pc;             // Program Counter
    CYG_HAL_OPENRISC_REG    sr;             // Supervisor/Status Reg
} GDB_Registers;

// Copy a set of registers from a HAL_SavedRegisters structure into a
// GDB_Registers structure.
#define HAL_GET_GDB_REGISTERS( _aregval_, _regs_ )              \
    CYG_MACRO_START                                             \
    GDB_Registers *_gdb_ = (GDB_Registers *)(_aregval_);        \
    int _i_;                                                    \
                                                                \
    for( _i_ = 0; _i_ <  32; _i_++ ) {                          \
        _gdb_->r[_i_] = (_regs_)->r[_i_];                       \
    }                                                           \
                                                                \
    _gdb_->pc = (_regs_)->pc;                                   \
    _gdb_->sr = (_regs_)->sr;                                   \
    CYG_MACRO_END

// Copy a set of registers from a GDB_Registers structure into a
// HAL_SavedRegisters structure.
#define HAL_SET_GDB_REGISTERS( _regs_ , _aregval_ )             \
    CYG_MACRO_START                                             \
    GDB_Registers *_gdb_ = (GDB_Registers *)(_aregval_);        \
    int _i_;                                                    \
                                                                \
    for( _i_ = 0; _i_ <  32; _i_++ )                            \
        (_regs_)->r[_i_] = _gdb_->r[_i_];                       \
                                                                \
    (_regs_)->pc = _gdb_->pc;                                   \
    (_regs_)->sr = _gdb_->sr;                                   \
    CYG_MACRO_END

//--------------------------------------------------------------------------
// HAL setjmp
// Note: These definitions are repeated in context.S. If changes are
// required remember to update both sets.

#define CYGARC_JMP_BUF_R1        0
#define CYGARC_JMP_BUF_R2        1
#define CYGARC_JMP_BUF_R9        2
#define CYGARC_JMP_BUF_R10       3
#define CYGARC_JMP_BUF_R12       4
#define CYGARC_JMP_BUF_R14       5
#define CYGARC_JMP_BUF_R16       6
#define CYGARC_JMP_BUF_R18       7
#define CYGARC_JMP_BUF_R20       8
#define CYGARC_JMP_BUF_R22       9
#define CYGARC_JMP_BUF_R24      10
#define CYGARC_JMP_BUF_R26      11
#define CYGARC_JMP_BUF_R28      12
#define CYGARC_JMP_BUF_R30      13

#define CYGARC_JMP_BUF_SIZE     14

typedef CYG_HAL_OPENRISC_REG hal_jmp_buf[CYGARC_JMP_BUF_SIZE];

externC int hal_setjmp(hal_jmp_buf env);
externC void hal_longjmp(hal_jmp_buf env, int val);

//-------------------------------------------------------------------------
// Idle thread code.
// This macro is called in the idle thread loop, and gives the HAL the
// chance to run code when no threads are runnable. Typical idle
// thread behaviour might be to halt the processor.

externC void hal_idle_thread_action(cyg_uint32 loop_count);

#define HAL_IDLE_THREAD_ACTION(_count_) hal_idle_thread_action(_count_)

//--------------------------------------------------------------------------
// Minimal and sensible stack sizes: the intention is that applications
// will use these to provide a stack size in the first instance prior to
// proper analysis.  Idle thread stack should be this big.

// *** THESE ARE NOT INTENDED TO BE GUARANTEED SUFFICIENT STACK SIZES ***
// They are, however, enough to start programming.
// You might, for example, need to make your stacks larger if you have
// large "auto" variables.

// This is not a config option because it should not be adjusted except
// under "enough rope to hang yourself" sort of disclaimers.

// Typical case stack frame size: return link + 10 caller-saved temporaries + 4 locals.
#define CYGNUM_HAL_STACK_FRAME_SIZE (15 * CYG_HAL_OPENRISC_REG_SIZE)

// Stack needed for a context switch:
#define CYGNUM_HAL_STACK_CONTEXT_SIZE (38 * 4)  // sizeof(HAL_SavedRegisters)

// Interrupt + call to ISR, interrupt_end() and the DSR
#define CYGNUM_HAL_STACK_INTERRUPT_SIZE (CYGNUM_HAL_STACK_CONTEXT_SIZE + 2*CYGNUM_HAL_STACK_FRAME_SIZE) 

// We define a minimum stack size as the minimum any thread could ever
// legitimately get away with. We can throw asserts if users ask for less
// than this. Allow enough for three interrupt sources - clock, serial and
// one other

// If interrupts are segregated onto their own stack...
#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK 

// An interrupt stack which is large enough for all possible interrupt
// conditions (and only used for that purpose) exists.  "User" stacks
// can therefore be much smaller
// NOTE - interrupt stack sizes can be smaller if we don't allow interrupts
//         to nest.

# define CYGNUM_HAL_STACK_SIZE_MINIMUM \
         ((3 * 5)*CYGNUM_HAL_STACK_FRAME_SIZE + 2*CYGNUM_HAL_STACK_INTERRUPT_SIZE)

#else

// No separate interrupt stack exists.  Make sure all threads contain
// a stack sufficiently large
# define CYGNUM_HAL_STACK_SIZE_MINIMUM                  \
        (( 3*CYGNUM_HAL_STACK_INTERRUPT_SIZE) +         \
         (25*CYGNUM_HAL_STACK_FRAME_SIZE))
#endif

// Now make a reasonable choice for a typical thread size. Pluck figures
// from thin air and say 40 call frames
#define CYGNUM_HAL_STACK_SIZE_TYPICAL                \
        (CYGNUM_HAL_STACK_SIZE_MINIMUM +             \
         40 * (CYGNUM_HAL_STACK_FRAME_SIZE))

#endif /* __ASSEMBLER__ */

//--------------------------------------------------------------------------
// Macros for switching context between two eCos instances (jump from
// code in ROM to code in RAM or vice versa).
// These are NOP's in the case of OpenRISC.
#define CYGARC_HAL_SAVE_GP()
#define CYGARC_HAL_RESTORE_GP()

//--------------------------------------------------------------------------
// Macro for finding return address of current function
#define CYGARC_HAL_GET_RETURN_ADDRESS(_x_, _dummy_) \
  asm volatile ( "l.ori %0,r9,0;" : "=r" (_x_) )

#define CYGARC_HAL_GET_RETURN_ADDRESS_BACKUP(_dummy_)

//--------------------------------------------------------------------------
#endif // CYGONCE_HAL_HAL_ARCH_H
// End of hal_arch.h
