#ifndef CYGONCE_HAL_MIPS_STUB_H
#define CYGONCE_HAL_MIPS_STUB_H
//========================================================================
//
//      mips-stub.h
//
//      MIPS-specific definitions for generic stub
//
//========================================================================
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
//========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     Red Hat, nickg
// Contributors:  Red Hat, nickg, dmoseley
// Date:          1998-06-08
// Purpose:       
// Description:   MIPS-specific definitions for generic stub
// Usage:         
//
//####DESCRIPTIONEND####
//
//========================================================================


#include <pkgconf/system.h>
#include <pkgconf/hal_mips.h>

#include <cyg/hal/hal_io.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(CYGPKG_HAL_MIPS_GDB_REPORT_CP0)
#define NUMREGS   107
#else
#define NUMREGS    90
#endif

#if defined(__mips64)
  // The simple case of 64-bit regs represented to GDB as 64-bit regs.

  #define REGSIZE(X) 8
  typedef unsigned long long target_register_t;

# ifdef CYGINT_HAL_MIPS_STUB_REPRESENT_32BIT_AS_64BIT
#  error __mips64 & CYGINT_HAL_MIPS_STUB_REPRESENT_32BIT_AS_64BIT
# endif

#elif defined(CYGINT_HAL_MIPS_STUB_REPRESENT_32BIT_AS_64BIT)

    // This is a catch-all for the common case:
    // Even though we are only working with 32 bit registers, GDB expects 64 bits
#define REGSIZE(X) 8
typedef unsigned long target_register_t;
    // We need to sign-extend the registers so GDB doesn't get confused.
#define CYGARC_SIGN_EXTEND_REGISTERS

    // The following platform-specific cases can be removed as/when/if they
    // get modified to implement CYGINT_HAL_MIPS_STUB_REPRESENT_32BIT_AS_64BIT

#elif defined(CYGPKG_HAL_MIPS_VR4300)
  // Even though we are only working with 32 bit registers, GDB expects 64 bits
  #define REGSIZE(X) 8
  typedef unsigned long long target_register_t;
  // We need to sign-extend the registers so GDB doesn't get confused.
  #define CYGARC_SIGN_EXTEND_REGISTERS
#elif 0 // defined(CYGPKG_HAL_MIPS_TX49)
  // Even though we are only working with 32 bit registers, GDB expects 64 bits
  #define REGSIZE(X) 8
  typedef unsigned long target_register_t;

  // We need to sign-extend the registers so GDB doesn't get confused.
  #define CYGARC_SIGN_EXTEND_REGISTERS
#elif defined(CYGPKG_HAL_MIPS_RM7000)
  // Even though we are only working with 32 bit registers, GDB expects 64 bits
  #define REGSIZE(X) 8
  typedef unsigned long target_register_t;

  // We need to sign-extend the registers so GDB doesn't get confused.
  #define CYGARC_SIGN_EXTEND_REGISTERS
#elif 0 //defined(CYGPKG_HAL_MIPS_MIPS32)
  // Even though we are only working with 32 bit registers, GDB expects 64 bits
  #define REGSIZE(X) 8
  typedef unsigned long target_register_t;

  // We need to sign-extend the registers so GDB doesn't get confused.
  #define CYGARC_SIGN_EXTEND_REGISTERS
#else

  // The simplest case of 32-bit regs represented to GDB as 32-bit regs.
  #define REGSIZE(X) 4
  typedef unsigned long target_register_t;
#endif

enum regnames {
        REG_ZERO,   REG_AT,     REG_V0,     REG_V1,     REG_A0,     REG_A1,     REG_A2,     REG_A3,
        REG_T0,     REG_T1,     REG_T2,     REG_T3,     REG_T4,     REG_T5,     REG_T6,     REG_T7,
        REG_S0,     REG_S1,     REG_S2,     REG_S3,     REG_S4,     REG_S5,     REG_S6,     REG_S7,
        REG_T8,     REG_T9,     REG_K0,     REG_K1,     REG_GP,     REG_SP,     REG_S8,     REG_RA,
        REG_SR,     REG_LO,     REG_HI,     REG_BAD,    REG_CAUSE,  REG_PC,
        REG_CONFIG = 84,    REG_CACHE,  REG_DEBUG,  REG_DEPC,   REG_EPC
};
#define USE_LONG_NAMES_FOR_ENUM_REGNAMES

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
#if !defined(SET_PC_PROTOTYPE_EXISTS) && !defined(set_pc)
#define SET_PC_PROTOTYPE_EXISTS
extern void set_pc (target_register_t pc);
#endif

/* Set things up so that the next user resume will execute one instruction.
   This may be done by setting breakpoints or setting a single step flag
   in the saved user registers, for example. */
#ifndef __single_step
void __single_step (void);
#endif

/* Clear the single-step state. */
void __clear_single_step (void);

extern int __is_bsp_syscall(void);

extern int hal_syscall_handler(void);
    
/* If the breakpoint we hit is in the breakpoint() instruction, return a
   non-zero value. */
#ifndef __is_breakpoint_function
extern int __is_breakpoint_function (void);
#endif

/* Skip the current instruction. */
extern void __skipinst (void);

extern void __install_breakpoints (void);

extern void __clear_breakpoints (void);

extern void __install_breakpoint_list (void);

extern void __clear_breakpoint_list (void);

#ifdef __cplusplus
}      /* extern "C" */
#endif

#endif // ifndef CYGONCE_HAL_MIPS_STUB_H
