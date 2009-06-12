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
// Copyright (C) 2003 Nick Garnett 
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
// Author(s):   jskov
// Contributors:jskov, nickg
// Date:        1999-04-24
// Purpose:     Define architecture abstractions
// Usage:       #include <cyg/hal/hal_arch.h>

//              
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>
#include CYGBLD_HAL_VAR_EXCEPTION_MODEL_H

//-----------------------------------------------------------------------------
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

// Enable interrupts before doing the trap, or the CPU will reset.
#define HAL_BREAKPOINT(_label_)                         \
    CYG_MACRO_START                                     \
    HAL_ENABLE_INTERRUPTS();                            \
    asm volatile (" .globl " #_label_ ";" #_label_ ": " \
                  " trapa #0x20;nop;nop;nop");          \
    CYG_MACRO_END

#define HAL_BREAKINST           0xc320

#define HAL_BREAKINST_SIZE      2

//-----------------------------------------------------------------------------
// HAL setjmp

typedef struct {
    cyg_uint32 sp;
    cyg_uint32 pr;
    cyg_uint32 r8;
    cyg_uint32 r9;
    cyg_uint32 r10;
    cyg_uint32 r11;
    cyg_uint32 r12;
    cyg_uint32 r13;
    cyg_uint32 r14;
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
 
// Stack frame overhead per call. Space to save FP, PR and 8 registers.
#define CYGNUM_HAL_STACK_FRAME_SIZE (4 * 10)

// Stack needed for a context switch (shreg_context_size from sh.inc)
#ifndef CYGHWR_HAL_SH_FPU
#define CYGNUM_HAL_STACK_CONTEXT_SIZE (4 * 24)
#else
#define CYGNUM_HAL_STACK_CONTEXT_SIZE (4 * (26 + CYGHWR_HAL_SH_FPU_REGS))
#endif

// Interrupt + call to ISR, interrupt_end() and the DSR
#define CYGNUM_HAL_STACK_INTERRUPT_SIZE \
    (CYGNUM_HAL_STACK_CONTEXT_SIZE + 2 * CYGNUM_HAL_STACK_FRAME_SIZE)

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
#define CYGARC_HAL_SAVE_GP()
#define CYGARC_HAL_RESTORE_GP()

//-----------------------------------------------------------------------------
#endif // CYGONCE_HAL_ARCH_H
// End of hal_arch.h
