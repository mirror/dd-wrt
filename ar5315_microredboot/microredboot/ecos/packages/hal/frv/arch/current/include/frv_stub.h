#ifndef CYGONCE_HAL_FRV_STUB_H
#define CYGONCE_HAL_FRV_STUB_H
//========================================================================
//
//      frv_stub.h
//
//      FUJITSU-specific definitions for generic stub
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
// Date:          2001-09-14
// Purpose:       
// Description:   FUJITSU-specific definitions for generic stub
// Usage:         
//
//####DESCRIPTIONEND####
//
//========================================================================

#ifdef __cplusplus
extern "C" {
#endif

#define NUMREGS    (147)

#define REGSIZE( _x_ ) (4)

#define NUMREGBYTES (NUMREGS*4)

#ifndef TARGET_REGISTER_T_DEFINED
#define TARGET_REGISTER_T_DEFINED
typedef cyg_uint32 target_register_t;
#endif

// This information needs to match what GDB wants
enum regnames {
    R0, R1, R2, R3, R4, R5, R6, R7, 
    R8, R9, R10, R11, R12, R13, R14, R15,
    R16, R17, R18, R19, R20, R21, R22, R23,
    R24, R25, R26, R27, R28, R29, R30, R31,
    R32, R33, R34, R35, R36, R37, R38, R39,
    R40, R41, R42, R43, R44, R45, R46, R47,
    R48, R49, R50, R51, R52, R53, R54, R55,
    R56, R57, R58, R59, R60, R61, R62, R63,
    FP0, FP1, FP2, FP3, FP4, FP5, FP6, FP7, 
    FP8, FP9, FP10, FP11, FP12, FP13, FP14, FP15,
    FP16, FP17, FP18, FP19, FP20, FP21, FP22, FP23,
    FP24, FP25, FP26, FP27, FP28, FP29, FP30, FP31,
    FP32, FP33, FP34, FP35, FP36, FP37, FP38, FP39,
    FP40, FP41, FP42, FP43, FP44, FP45, FP46, FP47,
    FP48, FP49, FP50, FP51, FP52, FP53, FP54, FP55,
    FP56, FP57, FP58, FP59, FP60, FP61, FP62, FP63,
    PC, PSR, CCR, CCCR,
    _X132, _X133, _X134, _X135, _X136, _X137, _X138,
    _X139, _X140, _X141, _X142, _X143, _X144,
    LR, LCR
};
#define SP R1

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

#define CYG_HAL_GDB_ENTER_CRITICAL_IO_REGION( _old_ )                       \
do {                                                                        \
    HAL_DISABLE_INTERRUPTS(_old_);                                          \
    cyg_hal_gdb_place_break((target_register_t)&&cyg_hal_gdb_break_place ); \
} while ( 0 )

#endif // CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // ifndef CYGONCE_HAL_FRV_STUB_H
