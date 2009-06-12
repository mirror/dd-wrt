#ifndef CYGONCE_HAL_PPC_STUB_H
#define CYGONCE_HAL_PPC_STUB_H
//========================================================================
//
//      ppc_stub.h
//
//      PowerPC-specific definitions for generic stub
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
// Contributors:  Red Hat, jskov
// Date:          1998-08-20
// Purpose:       
// Description:   PowerPC-specific definitions for generic stub
// Usage:         
//
//####DESCRIPTIONEND####
//
//========================================================================

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long target_register_t;

#define NUMREGS    71
	
#ifdef CYGHWR_HAL_POWERPC_FPU
// The PowerPC has floating point registers that are larger than what it
// can hold in a target_register_t
#define TARGET_HAS_LARGE_REGISTERS

// PowerPC stub has special needs for register handling because flating point
// registers are bigger than the rest. Special put_register and get_register
// are provided
#define CYGARC_STUB_REGISTER_ACCESS_DEFINED 1
	
// extra space needed for floating point registers
#define HAL_STUB_REGISTERS_SIZE ((sizeof(GDB_Registers) + sizeof(target_register_t) - 1)/sizeof(target_register_t))
#endif
	
#define REGSIZE( _x_ ) (((_x_) >= F0 && (_x_) <= F31) ? 8 : 4)

enum regnames {
    R0, R1, R2, R3, R4, R5, R6, R7, 
    R8, R9, R10, R11, R12, R13, R14, R15,
    R16, R17, R18, R19, R20, R21, R22, R23, 
    R24, R25, R26, R27, R28, R29, R30, R31, 
    F0, F1, F2, F3, F4, F5, F6, F7, 
    F8, F9, F10, F11, F12, F13, F14, F15, 
    F16, F17, F18, F19, F20, F21, F22, F23, 
    F24, F25, F26, F27, F28, F29, F30, F31, 
    PC, PS, CND, LR, CNT, XER, MQ
};

// For convenience
#define SP              R1

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

#ifdef __cplusplus
}   /* extern "C" */
#endif

#endif // ifndef CYGONCE_HAL_PPC_STUB_H
