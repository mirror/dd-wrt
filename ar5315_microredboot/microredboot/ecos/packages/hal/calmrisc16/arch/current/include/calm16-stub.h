#ifndef CYGONCE_HAL_MIPS_STUB_H
#define CYGONCE_HAL_MIPS_STUB_H
//========================================================================
//
//      mips-stub.h
//
//      CalmRISC16-specific definitions for generic stub
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
// Author(s):     Red Hat, msalter
// Contributors:  Red Hat, msalter
// Date:          2001-02-12
// Purpose:       
// Description:   CalmRISC16-specific definitions for generic stub
// Usage:         
//
//####DESCRIPTIONEND####
//
//========================================================================


#include <pkgconf/system.h>

#include <cyg/hal/hal_io.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TARGET_HAS_HARVARD_MEMORY 1
typedef unsigned long target_addr_t;
#define TARGET_ADDR_IS_PROGMEM(x) (((unsigned long)(x)) >= 0x400000UL)
#define TARGET_ADDR_TO_PTR(x)     ((char *) (((unsigned long)(x)) & 0x3fffff))

#define NUMREGS    39

typedef unsigned long target_register_t;

enum regnames {
        REG_R0,   REG_R1,   REG_R2,   REG_R3,   REG_R4,   REG_R5,   REG_R6,   REG_R7,
        REG_R8,   REG_R9,   REG_R10,  REG_R11,  REG_R12,  REG_R13,  REG_R14,  REG_R15,
        REG_E8,   REG_E9,   REG_E10,  REG_E11,  REG_E12,  REG_E13,  REG_E14,  REG_E15,
        REG_A8,   REG_A9,   REG_A10,  REG_A11,  REG_A12,  REG_A13,  REG_A14,  REG_A15,
	REG_PC,   REG_SPC_FIQ, REG_SPC_IRQ,
	REG_SR,   REG_SSR_FIQ, REG_SSR_IRQ, REG_SSR_SWI
};

#define REG_LR REG_A14
#define REG_SP REG_A15

#define PC REG_PC
#define SP REG_A15

#define REGSIZE(X) 4
#define REGBYTE(X) ((X)*4)

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

extern unsigned char  __read_prog_uint8(void *addr);
extern unsigned short __read_prog_uint16(void *addr);
extern unsigned long  __read_prog_uint32(void *addr);

extern void __write_prog_uint8(void *addr, unsigned char val);
extern void __write_prog_uint16(void *addr, unsigned short val);
extern void __write_prog_uint32(void *addr, unsigned long val);

#ifdef __cplusplus
}      /* extern "C" */
#endif

#endif // ifndef CYGONCE_HAL_MIPS_STUB_H
