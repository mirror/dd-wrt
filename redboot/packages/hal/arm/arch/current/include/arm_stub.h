#ifndef CYGONCE_HAL_ARM_STUB_H
#define CYGONCE_HAL_ARM_STUB_H
//========================================================================
//
//      arm_stub.h
//
//      ARM-specific definitions for generic stub
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
// Author(s):     Red Hat, gthomas
// Contributors:  Red Hat, gthomas
// Date:          1998-11-26
// Purpose:       
// Description:   ARM-specific definitions for generic stub
// Usage:         
//
//####DESCRIPTIONEND####
//
//========================================================================

#ifdef __cplusplus
extern "C" {
#endif

// The ARM has float (and possibly other coprocessor) registers that are
// larger than it can hold in a target_register_t.
#define TARGET_HAS_LARGE_REGISTERS

// ARM stub has special needs for register handling (not all regs are the
// the same size), so special put_register and get_register are provided.
#define CYGARC_STUB_REGISTER_ACCESS_DEFINED 1

#define NUMREGS    (16+8+2)  // 16 GPR, 8 FPR (unused), 2 PS

#define REGSIZE( _x_ ) (((_x_) < F0 || (_x_) >= FPS) ? 4 : 12)

// Comment out to allow for default gdb stub packet buffer which is larger.
// #define NUMREGBYTES ((16*4)+(8*12)+(2*4))

#ifndef TARGET_REGISTER_T_DEFINED
#define TARGET_REGISTER_T_DEFINED
typedef unsigned long target_register_t;
#endif

enum regnames {
    R0, R1, R2, R3, R4, R5, R6, R7, 
    R8, R9, R10, FP, IP, SP, LR, PC,
    F0, F1, F2, F3, F4, F5, F6, F7, 
    FPS, PS
};

#define HAL_STUB_REGISTERS_SIZE \
 ((sizeof(GDB_Registers) + sizeof(target_register_t) - 1) / sizeof(target_register_t))

#define PS_N 0x80000000
#define PS_Z 0x40000000
#define PS_C 0x20000000
#define PS_V 0x10000000

#define PS_I CPSR_IRQ_DISABLE
#define PS_F CPSR_FIQ_DISABLE

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

extern int __is_bsp_syscall(void);

//------------------------------------------------------------------------
// Special definition of CYG_HAL_GDB_ENTER_CRITICAL_IO_REGION

#ifdef CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT
// we can only do this at all if break support is enabled:

#ifdef __thumb__
// If this macro is used from Thumb code, we need to pass this information
// along to the place_break function so it can do the right thing.
#define CYG_HAL_GDB_ENTER_CRITICAL_IO_REGION( _old_ )                         \
do {                                                                          \
    HAL_DISABLE_INTERRUPTS(_old_);                                            \
    cyg_hal_gdb_place_break((target_register_t)((unsigned long)&&cyg_hal_gdb_break_place + 1));\
} while ( 0 )

#else

#define CYG_HAL_GDB_ENTER_CRITICAL_IO_REGION( _old_ )                       \
do {                                                                        \
    HAL_DISABLE_INTERRUPTS(_old_);                                          \
    cyg_hal_gdb_place_break((target_register_t)&&cyg_hal_gdb_break_place ); \
} while ( 0 )

#endif // __thumb_

// Also define ..LEAVE.. with a trick to *use* the label - sometimes the
// tools want to move the label, which is bad.
#define CYG_HAL_GDB_LEAVE_CRITICAL_IO_REGION( _old_ )                         \
do {                                                                          \
    cyg_hal_gdb_remove_break( (target_register_t)&&cyg_hal_gdb_break_place ); \
    HAL_RESTORE_INTERRUPTS(_old_);                                            \
    _old_ = 1; /* actually use the label as a label... */                     \
cyg_hal_gdb_break_place:                                                      \
    if ( (_old_)-- > 0 ) /* ...or the compiler might move it! */              \
        goto cyg_hal_gdb_break_place;                                         \
} while ( 0 )

#endif // CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // ifndef CYGONCE_HAL_ARM_STUB_H
