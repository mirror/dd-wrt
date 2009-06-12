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

#ifdef CYGHWR_HAL_MIPS_64BIT
# define CYG_HAL_MIPS_REG CYG_WORD64
# define CYG_HAL_MIPS_REG_SIZE 8
#else
# define CYG_HAL_MIPS_REG CYG_WORD32
# define CYG_HAL_MIPS_REG_SIZE 4
#endif

#if defined(CYGHWR_HAL_MIPS_FPU)
# if defined(CYGHWR_HAL_MIPS_FPU_64BIT)
#  define CYG_HAL_FPU_REG CYG_WORD64
# elif defined(CYGHWR_HAL_MIPS_FPU_32BIT)
#  define CYG_HAL_FPU_REG CYG_WORD32
# else
# error MIPS FPU register size not defined
# endif
#endif

typedef struct 
{
    // These are common to all saved states
    CYG_HAL_MIPS_REG    d[32];          /* Data regs                    */
    CYG_HAL_MIPS_REG    hi;             /* hi word of mpy/div reg       */
    CYG_HAL_MIPS_REG    lo;             /* lo word of mpy/div reg       */
#ifdef CYGHWR_HAL_MIPS_FPU
    CYG_HAL_FPU_REG     f[32];          /* FPU registers                */
    CYG_WORD32          fcr31;          /* FPU control/status register  */
    CYG_WORD32          fppad;          /* Dummy location to make this  */
                                        /* structure a multiple of 8    */
                                        /* bytes long.                  */
#endif
    
    // These are only saved for exceptions and interrupts
    CYG_WORD32          vector;         /* Vector number                */
    CYG_WORD32          sr;             /* Status Reg                   */
    CYG_HAL_MIPS_REG    pc;             /* Program Counter              */
    CYG_WORD32          cache;          /* Cache control register       */


    // These are only saved for exceptions, and are not restored
    // when continued.
    CYG_WORD32          cause;          /* Exception cause register     */
    CYG_HAL_MIPS_REG    badvr;          /* Bad virtual address reg      */
    CYG_WORD32          prid;           /* Processor Version            */
    CYG_WORD32          config;         /* Config register              */
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
#ifdef CYGHWR_HAL_MIPS_FPU
#define HAL_THREAD_INIT_FPU_CONTEXT( _regs_, _id_ )                             \
{                                                                               \
    for( _i_ = 0; _i_ < 32; _i_++ ) (_regs_)->f[_i_] = (_id_)|0xFF00|_i_;       \
    (_regs_)->fcr31 = 0x01000000;                                               \
}
#else
#define HAL_THREAD_INIT_FPU_CONTEXT( _regs_, _id_ )
#endif


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
    HAL_THREAD_INIT_FPU_CONTEXT( _regs_, _id_ );                                        \
    (_regs_)->d[29] = (CYG_HAL_MIPS_REG)(_sp_);       /* SP = top of stack      */      \
    (_regs_)->d[04] = (CYG_HAL_MIPS_REG)(_thread_);   /* R4 = arg1 = thread ptr */      \
    (_regs_)->lo = 0;                                 /* LO = 0                 */      \
    (_regs_)->hi = 0;                                 /* HI = 0                 */      \
    (_regs_)->d[30] = (CYG_HAL_MIPS_REG)(_sp_);       /* FP = top of stack      */      \
    (_regs_)->d[31] = (CYG_HAL_MIPS_REG)(_entry_);    /* RA(d[31]) = entry point*/      \
    (_regs_)->pc = (CYG_WORD)(_entry_);               /* PC = entry point       */      \
    (_regs_)->sr  = 0x00000001;                       /* SR = ls 3 bits only    */      \
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
// HAL_BREAKINST_TYPE is the type.

#define HAL_BREAKPOINT(_label_)                 \
asm volatile (" .globl  " #_label_ ";"          \
              #_label_":"                       \
              " break   5"                      \
    );

#define HAL_BREAKINST           0x0005000d

#define HAL_BREAKINST_SIZE      4

#define HAL_BREAKINST_TYPE      cyg_uint32

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

// If the CPU has an FPU, we also need to move the FPU registers.
#ifdef CYGHWR_HAL_MIPS_FPU
#define HAL_GET_GDB_FPU_REGISTERS( _regval_ , _regs_ )  \
CYG_MACRO_START                                         \
    int _i_;                                            \
    for( _i_ = 0; _i_ < 32; _i_++ )                     \
        _regval_[38+_i_] = (_regs_)->f[_i_];            \
    _regval_[70] = (_regs_)->fcr31;                     \
CYG_MACRO_END
#define HAL_SET_GDB_FPU_REGISTERS( _regs_ , _regval_ )  \
CYG_MACRO_START                                         \
    int _i_;                                            \
    for( _i_ = 0; _i_ < 32; _i_++ )                     \
        (_regs_)->f[_i_] = _regval_[38+_i_];            \
    (_regs_)->fcr31 = _regval_[70];                     \
CYG_MACRO_END
#else
#define HAL_GET_GDB_FPU_REGISTERS( _regval_ , _regs_ )
#define HAL_SET_GDB_FPU_REGISTERS( _regs_ , _regval_ )
#endif

// Some targets also report the state of all the coprocessor 0
// registers to GDB. If that is the case then
// CYGPKG_HAL_MIPS_GDB_REPORT_CP0 will be defined and the
// HAL_[G|S]ET_CP0_REGISTER_*() macros will be defined.

#ifdef CYGPKG_HAL_MIPS_GDB_REPORT_CP0

#define HAL_GET_GDB_CP0_REGISTERS( _regval_, _regs_ )                   \
    HAL_GET_CP0_REGISTER_32( _regval_[74],   0, 0 ); /* index    */     \
    HAL_GET_CP0_REGISTER_32( _regval_[75],   1, 0 ); /* random   */     \
    HAL_GET_CP0_REGISTER_32( _regval_[76],   2, 0 ); /* EntryLo0 */     \
    HAL_GET_CP0_REGISTER_32( _regval_[77],   3, 0 ); /* EntryLo1 */     \
    HAL_GET_CP0_REGISTER_64( _regval_[78],   4, 0 ); /* context  */     \
    HAL_GET_CP0_REGISTER_32( _regval_[79],   5, 0 ); /* PageMask */     \
    HAL_GET_CP0_REGISTER_32( _regval_[80],   6, 0 ); /* Wired    */     \
    (_regval_)[81] = 0xC0C0C006;                                        \
    (_regval_)[82] = (_regs_)->badvr;                /* BadVr    */     \
    HAL_GET_CP0_REGISTER_32( _regval_[83],   9, 0 ); /* Count    */     \
    HAL_GET_CP0_REGISTER_64( _regval_[84],  10, 0 ); /* EntryHi  */     \
    HAL_GET_CP0_REGISTER_32( _regval_[85],  11, 0 ); /* Compare  */     \
    (_regval_)[86] = (_regs_)->sr;                   /* Status   */     \
    (_regval_)[87] = (_regs_)->cause;                /* Cause    */     \
    HAL_GET_CP0_REGISTER_64( _regval_[88],  14, 0 ); /* EPC      */     \
    HAL_GET_CP0_REGISTER_32( _regval_[89],  15, 0 ); /* PRId     */     \
    HAL_GET_CP0_REGISTER_32( _regval_[90],  16, 0 ); /* Config   */     \
    HAL_GET_CP0_REGISTER_32( _regval_[91],  17, 0 ); /* LLAddr   */     \
    HAL_GET_CP0_REGISTER_64( _regval_[92],  18, 0 ); /* WatchLo  */     \
    HAL_GET_CP0_REGISTER_32( _regval_[93],  19, 0 ); /* WatchHi  */     \
    HAL_GET_CP0_REGISTER_64( _regval_[94],  20, 0 ); /* XContext */     \
    (_regval_)[95] = 0xC0C0C021;                                        \
    (_regval_)[96] = 0xC0C0C022;                                        \
    HAL_GET_CP0_REGISTER_32( _regval_[97],  23, 0 ); /* Debug    */     \
    HAL_GET_CP0_REGISTER_64( _regval_[98],  24, 0 ); /* DEPC     */     \
    (_regval_)[99] = 0xC0C0C025;                                        \
    HAL_GET_CP0_REGISTER_32( _regval_[100], 26, 0 ); /* ErrCtl   */     \
    HAL_GET_CP0_REGISTER_32( _regval_[101], 27, 0 ); /* CacheErr */     \
    HAL_GET_CP0_REGISTER_32( _regval_[102], 28, 0 ); /* TagLo    */     \
    HAL_GET_CP0_REGISTER_32( _regval_[103], 29, 0 ); /* TagHi    */     \
    HAL_GET_CP0_REGISTER_64( _regval_[104], 30, 0 ); /* ErrorEPC */     \
    HAL_GET_CP0_REGISTER_64( _regval_[105], 31, 0 ); /* DESAVE   */     \
    HAL_GET_CP0_REGISTER_32( _regval_[106], 16, 1 ); /* Config1  */

#define HAL_SET_GDB_CP0_REGISTERS( _regval_, _regs_ )                   \
    HAL_SET_CP0_REGISTER_32( _regval_[74],   0, 0 ); /* index    */     \
    HAL_SET_CP0_REGISTER_32( _regval_[76],   2, 0 ); /* EntryLo0 */     \
    HAL_SET_CP0_REGISTER_32( _regval_[77],   3, 0 ); /* EntryLo1 */     \
    HAL_SET_CP0_REGISTER_64( _regval_[78],   4, 0 ); /* context  */     \
    HAL_SET_CP0_REGISTER_32( _regval_[79],   5, 0 ); /* PageMask */     \
    HAL_SET_CP0_REGISTER_32( _regval_[80],   6, 0 ); /* Wired    */     \
    HAL_SET_CP0_REGISTER_32( _regval_[83],   9, 0 ); /* Count    */     \
    HAL_SET_CP0_REGISTER_64( _regval_[84],  10, 0 ); /* EntryHi  */     \
    HAL_SET_CP0_REGISTER_32( _regval_[85],  11, 0 ); /* Compare  */     \
    HAL_SET_CP0_REGISTER_32( _regval_[90],  16, 0 ); /* Config   */     \
    HAL_SET_CP0_REGISTER_64( _regval_[92],  18, 0 ); /* WatchLo  */     \
    HAL_SET_CP0_REGISTER_32( _regval_[93],  19, 0 ); /* WatchHi  */     \
    HAL_SET_CP0_REGISTER_64( _regval_[94],  20, 0 ); /* XContext */     \
    HAL_SET_CP0_REGISTER_32( _regval_[97],  23, 0 ); /* Debug    */     \
    HAL_SET_CP0_REGISTER_64( _regval_[98],  24, 0 ); /* DEPC     */     \
    HAL_SET_CP0_REGISTER_32( _regval_[100], 26, 0 ); /* ErrCtl   */     \
    HAL_SET_CP0_REGISTER_32( _regval_[101], 27, 0 ); /* CacheErr */     \
    HAL_SET_CP0_REGISTER_32( _regval_[102], 28, 0 ); /* TagLo    */     \
    HAL_SET_CP0_REGISTER_32( _regval_[103], 29, 0 ); /* TagHi    */     \
    HAL_SET_CP0_REGISTER_64( _regval_[105], 31, 0 ); /* DESAVE   */

#else
#define HAL_GET_GDB_CP0_REGISTERS( _regval_, _regs_ )
#define HAL_SET_GDB_CP0_REGISTERS( _regval_, _regs_ )
#endif

// Copy a set of registers from a HAL_SavedRegisters structure into a
// GDB ordered array.    
#define HAL_GET_GDB_REGISTERS( _aregval_ , _regs_ )             \
{                                                               \
    CYG_HAL_GDB_REG *_regval_ = (CYG_HAL_GDB_REG *)(_aregval_); \
    int _i_;                                                    \
                                                                \
    for( _i_ = 0; _i_ < 32; _i_++ )                             \
        _regval_[_i_] = (_regs_)->d[_i_];                       \
                                                                \
    HAL_GET_GDB_FPU_REGISTERS( _regval_, _regs_ );              \
                                                                \
    _regval_[32] = (_regs_)->sr;                                \
    _regval_[33] = (_regs_)->lo;                                \
    _regval_[34] = (_regs_)->hi;                                \
    _regval_[35] = (_regs_)->badvr;                             \
    _regval_[36] = (_regs_)->cause;                             \
    _regval_[37] = (_regs_)->pc;                                \
                                                                \
    HAL_GET_GDB_CP0_REGISTERS( _regval_, _regs_ );              \
}

// Copy a GDB ordered array into a HAL_SavedRegisters structure.
#define HAL_SET_GDB_REGISTERS( _regs_ , _aregval_ )             \
{                                                               \
    CYG_HAL_GDB_REG *_regval_ = (CYG_HAL_GDB_REG *)(_aregval_); \
    int _i_;                                                    \
                                                                \
    for( _i_ = 0; _i_ < 32; _i_++ )                             \
        (_regs_)->d[_i_] = _regval_[_i_];                       \
                                                                \
    HAL_SET_GDB_FPU_REGISTERS( _regs_, _regval_ );              \
                                                                \
    (_regs_)->sr = _regval_[32];                                \
    (_regs_)->lo = _regval_[33];                                \
    (_regs_)->hi = _regval_[34];                                \
    (_regs_)->badvr = _regval_[35];                             \
    (_regs_)->cause = _regval_[36];                             \
    (_regs_)->pc = _regval_[37];                                \
                                                                \
    HAL_SET_GDB_CP0_REGISTERS( _regval_, _regs_ );              \
}

//--------------------------------------------------------------------------
// HAL setjmp
// Note: These definitions are repeated in hal_arch.h. If changes are
// required remember to update both sets.

#define CYGARC_JMP_BUF_SP        0
#define CYGARC_JMP_BUF_R16       1
#define CYGARC_JMP_BUF_R17       2
#define CYGARC_JMP_BUF_R18       3
#define CYGARC_JMP_BUF_R19       4
#define CYGARC_JMP_BUF_R20       5
#define CYGARC_JMP_BUF_R21       6
#define CYGARC_JMP_BUF_R22       7
#define CYGARC_JMP_BUF_R23       8
#define CYGARC_JMP_BUF_R28       9
#define CYGARC_JMP_BUF_R30      10
#define CYGARC_JMP_BUF_R31      11

#define CYGARC_JMP_BUF_SIZE     12

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
#if defined(CYGHWR_HAL_MIPS_FPU)
# if defined(CYGHWR_HAL_MIPS_FPU_64BIT)
#define CYGNUM_HAL_STACK_CONTEXT_SIZE (((32+12)*CYG_HAL_MIPS_REG_SIZE)+(32*8))
# elif defined(CYGHWR_HAL_MIPS_FPU_32BIT)
#define CYGNUM_HAL_STACK_CONTEXT_SIZE (((32+12)*CYG_HAL_MIPS_REG_SIZE)+(32*4))
# else
# error MIPS FPU register size not defined
# endif
#else
#define CYGNUM_HAL_STACK_CONTEXT_SIZE ((32+10)*CYG_HAL_MIPS_REG_SIZE)
#endif



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

// Convenience macros for accessing memory cached or uncached
#define CYGARC_KSEG_MASK                               (0xE0000000)
#define CYGARC_KSEG_CACHED                             (0x80000000)
#define CYGARC_KSEG_UNCACHED                           (0xA0000000)
#define CYGARC_KSEG_CACHED_BASE                        (0x80000000)
#define CYGARC_KSEG_UNCACHED_BASE                      (0xA0000000)
#ifndef __ASSEMBLER__
#define CYGARC_CACHED_ADDRESS(x)                       ((((CYG_ADDRESS)(x)) & ~CYGARC_KSEG_MASK) | CYGARC_KSEG_CACHED)
#define CYGARC_UNCACHED_ADDRESS(x)                     ((((CYG_ADDRESS)(x)) & ~CYGARC_KSEG_MASK) | CYGARC_KSEG_UNCACHED)
#define CYGARC_PHYSICAL_ADDRESS(x)                     (((CYG_ADDRESS)(x)) & ~CYGARC_KSEG_MASK)
#else // __ASSEMBLER__
#define CYGARC_CACHED_ADDRESS(x)                       ((((x)) & ~CYGARC_KSEG_MASK) | CYGARC_KSEG_CACHED)
#define CYGARC_UNCACHED_ADDRESS(x)                     ((((x)) & ~CYGARC_KSEG_MASK) | CYGARC_KSEG_UNCACHED)
#define CYGARC_PHYSICAL_ADDRESS(x)                     (((x)) & ~CYGARC_KSEG_MASK)
#define CYGARC_ADDRESS_REG_CACHED(reg)                 \
        and     reg, reg, ~CYGARC_KSEG_MASK;           \
        or      reg, reg, CYGARC_KSEG_CACHED
#define CYGARC_ADDRESS_REG_UNCACHED(reg)               \
        and     reg, reg, ~CYGARC_KSEG_MASK;           \
        or      reg, reg, CYGARC_KSEG_UNCACHED
#endif /* __ASSEMBLER__ */

//--------------------------------------------------------------------------
// Macros for switching context between two eCos instances (jump from
// code in ROM to code in RAM or vice versa).
#define CYGARC_HAL_SAVE_GP()                    \
    CYG_MACRO_START                             \
    register CYG_ADDRWORD __gp_save;            \
    asm volatile ( "move   %0,$28;"             \
                   ".extern _gp;"               \
                   "la     $gp,_gp;"            \
                     : "=r"(__gp_save))

#define CYGARC_HAL_RESTORE_GP()                                 \
    asm volatile ( "move   $gp,%0;" :: "r"(__gp_save) );        \
    CYG_MACRO_END


//--------------------------------------------------------------------------
// Macro for finding return address. 
#define CYGARC_HAL_GET_RETURN_ADDRESS(_x_, _dummy_) \
  asm volatile ( "move %0,$31;" : "=r" (_x_) )

#define CYGARC_HAL_GET_RETURN_ADDRESS_BACKUP(_dummy_)

//--------------------------------------------------------------------------
#endif // CYGONCE_HAL_HAL_ARCH_H
// End of hal_arch.h
