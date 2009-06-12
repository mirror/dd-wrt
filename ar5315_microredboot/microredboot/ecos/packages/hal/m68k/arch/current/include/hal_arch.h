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

#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>

// Include some variant specific architectural defines.
#include <cyg/hal/var_arch.h>

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
#include <cyg/hal/m68k_stub.h>
#endif // CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS

#ifndef HAL_NORMAL_SAVED_CONTEXT
/*****************************************************************************
HAL_NORMAL_SAVED_CONTEXT -- Saved by a normal context switch

     Define  a  generic   structure  to  save   a  thread  context.    Some
architecture variants will want to redefine this.

*****************************************************************************/
typedef struct
{

    //   Data regs D0-D7

    #define HAL_NORMAL_SAVED_NUM_D_REGS 8
    CYG_WORD32 d[HAL_NORMAL_SAVED_NUM_D_REGS];

    //   Address regs A0-A6

    #define HAL_NORMAL_SAVED_NUM_A_REGS 7
    CYG_ADDRESS a[HAL_NORMAL_SAVED_NUM_A_REGS];

    //   Program Counter

    CYG_ADDRESS pc;

} __attribute__ ((aligned, packed)) HAL_SavedRegisters_normal;
#endif // HAL_NORMAL_SAVED_CONTEXT

#ifndef HAL_GENERIC_SAVED_CONTEXT
/*****************************************************************************
HAL_GENERIC_SAVED_CONTEXT -- Generic saved context structure

     This is  a  generic  structure  that  should  describe  various  saved
processor contexts on this platform.

     If the variant HAL does not define this, just define a saved  register
structure with a normal context.

*****************************************************************************/
#define HAL_GENERIC_SAVED_CONTEXT \
typedef union \
{ \
    HAL_SavedRegisters_normal normal; \
} __attribute__ ((aligned, packed)) HAL_SavedRegisters;
#endif // HAL_GENERIC_SAVED_CONTEXT
HAL_GENERIC_SAVED_CONTEXT;

#ifndef HAL_THREAD_SWITCH_CONTEXT
/*****************************************************************************
HAL_THREAD_SWITCH_CONTEXT

     This macro saves the state of the currently running thread and  writes
its stack pointer to *(_fspptr_).

     It then switches to the thread context that *(_tspptr_) points to.

INPUT:

     _fspptr_: A pointer to the location to save the current thread's stack
pointer to.

     _tspptr_: A pointer to  the location containing  the stack pointer  of
the thread context to switch to.

OUTPUT:

     *(_fspptr_): Contains the  stack  pointer  of  the  previous  thread's
context.

*****************************************************************************/
#define HAL_THREAD_SWITCH_CONTEXT(_fspptr_,_tspptr_) \
CYG_MACRO_START \
    asm volatile (" pea     1f(%%pc)\n" \
                  " lea     -(8+7)*4(%%sp),%%sp\n" \
                  " movem.l %%d0-%%d7/%%a0-%%a6,(%%sp)\n" \
                  " move.l  %%sp,%0\n" \
                  " move.l  %1,%%sp\n" \
                  " movem.l (%%sp),%%d0-%%d7/%%a0-%%a6\n" \
                  " lea     (8+7)*4(%%sp),%%sp\n" \
                  " rts\n" \
                  "1:\n" \
                  : "=g" (*(_fspptr_)) \
                  : "g" (*(_tspptr_)) \
                  : "memory"); \
CYG_MACRO_END
#if HAL_NORMAL_SAVED_NUM_D_REGS != 8
    #error
#endif
#if HAL_NORMAL_SAVED_NUM_A_REGS != 7
    #error
#endif
#endif // HAL_THREAD_SWITCH_CONTEXT

#ifndef HAL_THREAD_LOAD_CONTEXT
/*****************************************************************************
HAL_THREAD_LOAD_CONTEXT

     This macro loads the thread context that *(_tspptr_) points to.

     This macro does not return.

INPUT:

     _tspptr_: A pointer to  the location containing  the stack pointer  of
the thread context to switch to.

*****************************************************************************/
#define HAL_THREAD_LOAD_CONTEXT(_tspptr_) \
CYG_MACRO_START \
    asm volatile (" move.l  %0,%%sp\n" \
                  " movem.l (%%sp),%%d0-%%d7/%%a0-%%a6\n" \
                  " lea     (8+7)*4(%%sp),%%sp\n" \
                  " rts\n" \
                  : \
                  : "g" (*(_tspptr_)) \
                  : "memory"); \
CYG_MACRO_END
#if HAL_NORMAL_SAVED_NUM_D_REGS != 8
    #error
#endif
#if HAL_NORMAL_SAVED_NUM_A_REGS != 7
    #error
#endif
#endif // HAL_THREAD_LOAD_CONTEXT

#ifndef HAL_THREAD_INIT_CONTEXT
/*****************************************************************************
HAL_THREAD_INIT_CONTEXT -- Context Initialization

     Initialize the context of a thread.

INPUT:

     _sparg_: The name of  the variable  containing the  current sp.   This
will be written with the new sp.

     _thread_: The thread object  address,  passed  as  argument  to  entry
point.

     _entry_: The thread's entry point address.

     _id_: A bit pattern used in initializing registers, for debugging.

OUTPUT:

     _sparg_: Updated with the value of the new sp.

*****************************************************************************/
#define HAL_THREAD_INIT_CONTEXT(_sparg_, _thread_, _entry_, _id_) \
    CYG_MACRO_START \
    CYG_WORD32 * _sp_ = ((CYG_WORD32*)((CYG_WORD32)(_sparg_) & ~15)); \
    HAL_SavedRegisters_normal * _regs_; \
    int _i_; \
 \
    *(--_sp_) = (CYG_WORD32)(_thread_); /* Thread's parameter. */ \
    *(--_sp_) = (CYG_WORD32)0xDEADC0DE; /* Thread's return addr. */ \
 \
    _regs_ = (HAL_SavedRegisters_normal*) \
              ((CYG_WORD32)_sp_ - sizeof(HAL_SavedRegisters_normal)); \
 \
    for (_i_=0; _i_ < HAL_NORMAL_SAVED_NUM_A_REGS; _i_++) \
        _regs_->a[_i_] = _regs_->d[_i_] = (_id_); \
    _regs_->d[_i_] = (_id_); /* D7 */ \
    /* A6, initial frame pointer should be null */ \
    _regs_->a[HAL_NORMAL_SAVED_NUM_A_REGS-1] = (CYG_ADDRESS)0; \
    /* Thread's starting PC */ \
    _regs_->pc = (CYG_ADDRESS)(_entry_); \
 \
    (_sparg_) = (CYG_ADDRESS)_regs_; \
    CYG_MACRO_END
#endif // HAL_THREAD_INIT_CONTEXT

//-----------------------------------------------------------------------------
// Bit manipulation routines

externC cyg_uint32 hal_lsbit_index(cyg_uint32 mask);
externC cyg_uint32 hal_msbit_index(cyg_uint32 mask);

#define HAL_LSBIT_INDEX(index, mask) (index) = hal_lsbit_index(mask);

#define HAL_MSBIT_INDEX(index, mask) (index) = hal_msbit_index(mask);

//-----------------------------------------------------------------------------
// Idle thread code.
// This macro is called in the idle thread loop, and gives the HAL the
// chance to insert code. Typical idle thread behaviour might be to halt the
// processor.

externC void hal_idle_thread_action(cyg_uint32 loop_count);

#define HAL_IDLE_THREAD_ACTION(_count_) hal_idle_thread_action(_count_)

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
              " trap #1"                        \
    );

#define HAL_BREAKINST           0x4E41

#define HAL_BREAKINST_SIZE      2

#if defined(CYGFUN_HAL_COMMON_KERNEL_SUPPORT) && \
      defined(CYGPKG_HAL_EXCEPTIONS)

//-----------------------------------------------------------------------------
// Exception handling function.
// This function is defined by the kernel according to this prototype. It is
// invoked from the HAL to deal with any CPU exceptions that the HAL does
// not want to deal with itself. It usually invokes the kernel's exception
// delivery mechanism.

externC void cyg_hal_deliver_exception( CYG_WORD code, CYG_ADDRWORD data );

#endif /* defined(CYGFUN_HAL_COMMON_KERNEL_SUPPORT) */

//-----------------------------------------------------------------------------
// Minimal and sensible stack sizes: the intention is that applications
// will use these to provide a stack size in the first instance prior to
// proper analysis.  Idle thread stack should be this big.

//    THESE ARE NOT INTENDED TO BE MICROMETRICALLY ACCURATE FIGURES.
//           THEY ARE HOWEVER ENOUGH TO START PROGRAMMING.
// YOU MUST MAKE YOUR STACKS LARGER IF YOU HAVE LARGE "AUTO" VARIABLES!

// This is not a config option because it should not be adjusted except
// under "enough rope" sort of disclaimers.

//      Stack frame overhead per call.  6 data registers, 5 address registers,
// frame pointer, and return address.  We  can't guess the local variables  so
// just assume that using all of the registers averages out.

#define CYGNUM_HAL_STACK_FRAME_SIZE ((6 + 5 + 1 + 1) * 4)

// Stack needed for a context switch.
//      All registers + pc + sr + vector

#ifndef CYGNUM_HAL_STACK_CONTEXT_SIZE
#define CYGNUM_HAL_STACK_CONTEXT_SIZE ((8+8+1+1+1)*4)
#endif // CYGNUM_HAL_STACK_CONTEXT_SIZE

// Interrupt + call to ISR, interrupt_end() and the DSR

#define CYGNUM_HAL_STACK_INTERRUPT_SIZE \
    ((CYGNUM_HAL_STACK_CONTEXT_SIZE) + (8*CYGNUM_HAL_STACK_FRAME_SIZE))

// We define a minimum stack size as the minimum any thread could ever
// legitimately get away with. We can throw asserts if users ask for less
// than this. Allow enough for four interrupt sources - clock, serial,
// nic, and one other

// No separate interrupt stack exists.  Make sure all threads contain
// a stack sufficiently large

#define CYGNUM_HAL_STACK_SIZE_MINIMUM                   \
        ((4*CYGNUM_HAL_STACK_INTERRUPT_SIZE)            \
         + (16*CYGNUM_HAL_STACK_FRAME_SIZE))

// Now make a reasonable choice for a typical thread size. Pluck figures
// from thin air and say 30 call frames with an average of 16 words of
// automatic variables per call frame

#define CYGNUM_HAL_STACK_SIZE_TYPICAL                   \
        (CYGNUM_HAL_STACK_SIZE_MINIMUM +                \
         (30 * (CYGNUM_HAL_STACK_FRAME_SIZE+(16*4))))

//--------------------------------------------------------------------------
// Macros for switching context between two eCos instances (jump from
// code in ROM to code in RAM or vice versa).
#define CYGARC_HAL_SAVE_GP()
#define CYGARC_HAL_RESTORE_GP()

#ifndef HAL_SETJMP
#define HAL_SETJMP
/*****************************************************************************
hal_setjmp/hal_longjmp

     We do the best  we can to define  generic setjmp and longjmp  routines
for the m68k architecture.  Some architectures will need to override this.

*****************************************************************************/

//      We must save all of  the registers that  are preserved across  routine
// calls.  The assembly code assumes  that this  structure is  defined in  the
// following format.  Any changes to this structure will result in changes  to
// the assembly code!!

typedef struct {
    cyg_uint32 d2;
    cyg_uint32 d3;
    cyg_uint32 d4;
    cyg_uint32 d5;
    cyg_uint32 d6;
    cyg_uint32 d7;
    cyg_uint32 a2;
    cyg_uint32 a3;
    cyg_uint32 a4;
    cyg_uint32 a5;
    cyg_uint32 a6;
    cyg_uint32 sp;
    cyg_uint32 pc;
} hal_jmp_buf_t;

//      This type is used by normal  routines  to  pass  the  address  of  the
// structure into our routines without  having to explicitly take the  address
// of the structure.

typedef cyg_uint32 hal_jmp_buf[sizeof(hal_jmp_buf_t) / sizeof(cyg_uint32)];

//      Define the generic setjmp and longjmp routines.

externC int hal_m68k_setjmp(hal_jmp_buf env);
externC void hal_m68k_longjmp(hal_jmp_buf env, int val);
#define hal_setjmp(_env) hal_m68k_setjmp(_env)
#define hal_longjmp(_env, _val) hal_m68k_longjmp(_env, _val)
#endif // HAL_SETJMP

//-----------------------------------------------------------------------------
#endif // CYGONCE_HAL_ARCH_H
// End of hal_arch.h

