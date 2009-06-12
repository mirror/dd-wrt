#ifndef CYGONCE_HAL_SH_STUB_H
#define CYGONCE_HAL_SH_STUB_H

//==========================================================================
//
//      sh_stub.h
//
//      SH stub defines
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    jskov
// Contributors: jskov, nickg
// Date:         1999-05-18
// Purpose:      Define SH stub stuff
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/hal/hal_intr.h>           // CYGNUM_HAL_EXCEPTION_TRAP

#ifdef __cplusplus
extern "C" {
#endif

#define NUMREGS    59

#define REGSIZE( _x_ ) (4)

typedef unsigned long target_register_t;

enum regnames {
    R0, R1, R2, R3, R4, R5, R6, R7, 
    R8, R9, R10, R11, R12, R13, R14, R15,
    PC, PR, GBR, VBR, MACH, MACL, SR,
    FPUL, FPSCR,
    FP0, FP1, FP2, FP3, FP4, FP5, FP6, FP7,
    FP8, FP9, FP10, FP11, FP12, FP13, FP14, FP15,
    SSR, SPC,
    ROB0, R1B0, R2B0, R3B0, R4B0, R5B0, R6B0, R7B0, 
    ROB1, R1B1, R2B1, R3B1, R4B1, R5B1, R6B1, R7B1
};

// For convenience
#define SP              R15
#define FP              R14

typedef enum regnames regnames_t;

/* Given a trap value TRAP, return the corresponding signal. */
extern int __computeSignal (unsigned int trap_number);

/* Return the SPARC trap number corresponding to the last-taken trap. */
extern int __get_trap_number (void);

/* Return the currently-saved value corresponding to register REG. */
extern target_register_t get_register (regnames_t reg);

/* Store VALUE in the register corresponding to WHICH. */
extern void put_register (regnames_t which, target_register_t value);

/* Set the currently-saved pc register value to PC. This also updates NPC
   as needed. */
extern void set_pc (target_register_t pc);

/* Set things up so that the next user resume will execute one instruction.
   This may be done by setting breakpoints or setting a single step flag
   in the saved user registers, for example. */
void __single_step (void);

/* Clear the single-step state. */
void __clear_single_step (void);

/* If the breakpoint we hit is in the breakpoint() instruction, return a
   non-zero value. */
extern int __is_breakpoint_function (void);

/* Skip the current instruction. */
extern void __skipinst (void);

extern void __install_breakpoints (void);

extern void __clear_breakpoints (void);

// We have to rewind the PC in case of a breakpoint.
#define HAL_STUB_PLATFORM_STUBS_FIXUP()                         \
    CYG_MACRO_START                                             \
    if (CYGNUM_HAL_EXCEPTION_TRAP == __get_trap_number())       \
    {                                                           \
        CYG_ADDRESS pc = get_register(PC) - 2;                  \
        if( *(cyg_uint16 *)pc == HAL_BREAKINST )                \
            put_register(PC, pc);                               \
    }                                                           \
    CYG_MACRO_END

#ifdef __cplusplus
}   /* extern "C" */
#endif

#endif // ifndef CYGONCE_HAL_PPC_STUB_H
