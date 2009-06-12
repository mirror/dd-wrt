#ifndef CYGONCE_HAL_MN10300_STUB_H
#define CYGONCE_HAL_MN10300_STUB_H
//========================================================================
//
//      mn10300_stub.h
//
//      MN10300-specific definitions for generic stub
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
// Author(s):     Red Hat, jskov
// Contributors:  Red Hat, jskov, dmoseley
// Date:          1998-11-06
// Purpose:       
// Description:   MN10300-specific definitions for generic stub
// Usage:         
//
//####DESCRIPTIONEND####
//
//========================================================================

#ifdef __cplusplus
extern "C" {
#endif

#if defined(CYGPKG_HAL_MN10300_AM33) && (CYGHWR_HAL_MN10300_AM33_REVISION == 2)
#define NUMREGS    64
#else
#define NUMREGS    32
#endif

#define REGSIZE( _x_ ) (4)

typedef unsigned long target_register_t;

enum regnames {
  D0, D1, D2, D3, A0, A1, A2, A3,
  SP, PC, MDR, PSW, LIR, LAR
#ifdef CYGPKG_HAL_MN10300_AM33
  , MDRQ,
  R0, R1, R2, R3, R4, R5, R6, R7,
  SSP, MSP, USP, MCRH, MCRL, MCVF
#if CYGHWR_HAL_MN10300_AM33_REVISION == 2
  // FPU registers for AM33/2.00
  , FP_START, FPCR=FP_START,
  XXXX1, XXXX2, // unused
  FS0,  FS1,  FS2,  FS3,  FS4,  FS5,  FS6,  FS7,
  FS8,  FS9,  FS10, FS11, FS12, FS13, FS14, FS15,
  FS16, FS17, FS18, FS19, FS20, FS21, FS22, FS23,
  FS24, FS25, FS26, FS27, FS28, FS29, FS30, FS31, FP_END=FS31,
#endif
#endif  
};

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

extern int __is_bsp_syscall(void);

extern int hal_syscall_handler(void);

#ifdef __cplusplus
}   /* extern "C" */
#endif

#endif // ifndef CYGONCE_HAL_MN10300_STUB_H
