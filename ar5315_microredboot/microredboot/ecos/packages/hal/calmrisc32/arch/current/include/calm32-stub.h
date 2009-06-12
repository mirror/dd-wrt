#ifndef CYGONCE_HAL_MIPS_STUB_H
#define CYGONCE_HAL_MIPS_STUB_H
//========================================================================
//
//      mips-stub.h
//
//      CalmRISC32-specific definitions for generic stub
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
// Description:   CalmRISC32-specific definitions for generic stub
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
typedef unsigned long long target_addr_t;
#define TARGET_ADDR_IS_PROGMEM(x) (((unsigned long long)(x)) >= 0x100000000ULL)
#define TARGET_ADDR_TO_PTR(x)     ((char *) (((unsigned)(x)) & 0xffffffff))

#define NUMREGS    43

#define REGSIZE(X) 4
typedef unsigned long target_register_t;

enum regnames {
    REG_B0R0,   REG_B0R1,  REG_B0R2,   REG_B0R3,   REG_B0R4,   REG_B0R5,   REG_B0R6,   REG_B0R7,
    REG_B0R8,   REG_B0R9,  REG_B0R10,  REG_B0R11,  REG_B0R12,  REG_B0R13,  REG_B0R14,  REG_B0R15,
    REG_B1R0,   REG_B1R1,  REG_B1R2,   REG_B1R3,   REG_B1R4,   REG_B1R5,   REG_B1R6,   REG_B1R7,
    REG_B1R8,   REG_B1R9,  REG_B1R10,  REG_B1R11,  REG_B1R12,  REG_B1R13,  REG_B1R14,  REG_B1R15,
    REG_PC, REG_VBR, REG_SR,
    REG_SSR_FIQ, REG_SPC_FIQ, REG_SSR_IRQ, REG_SPC_IRQ,
    REG_SSR_SWI, REG_SPC_SWI, REG_SSR_EXPT, REG_SPC_EXPT
};

typedef enum regnames regnames_t;

#define PC REG_PC
#define SP __sp_regnum()
extern int __sp_regnum(void);

#define HAL_STUB_ADD_T_REG(__ptr__,__reg__,__val__)                      \
    *__ptr__++ = __tohex (__reg__ >> 4);                                 \
    *__ptr__++ = __tohex (__reg__);                                      \
    *__ptr__++ = ':';                                                    \
    __val__ = get_register (__reg__);                                    \
    __ptr__ = __mem2hex((char *)&__val__, __ptr__, sizeof(__val__), 0);  \
    *ptr++ = ';'                                                        

#define HAL_STUB_ARCH_T_PACKET_EXTRAS(__ptr__)                           \
{                                                                        \
    target_register_t __val__;                                           \
                                                                         \
    HAL_STUB_ADD_T_REG(__ptr__,REG_SR,__val__);                          \
    if ((__val__ & CYGARC_SR_PM) == 0 || (__val__ & CYGARC_SR_BS) == 0){ \
        HAL_STUB_ADD_T_REG(__ptr__,REG_B0R13,__val__);                   \
    } else {                                                             \
        HAL_STUB_ADD_T_REG(__ptr__,REG_B1R13,__val__);                   \
    }                                                                    \
}

/* Given a trap value TRAP, return the corresponding signal. */
extern int __computeSignal (unsigned int trap_number);

/* Return the SPARC trap number corresponding to the last-taken trap. */
extern int __get_trap_number (void);

/* Return the currently-saved value corresponding to register REG. */
extern target_register_t get_register (regnames_t reg);

/* Store VALUE in the register corresponding to WHICH. */
extern void put_register (regnames_t which, target_register_t value);

/* Set the currently-saved pc register value to PC. */
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
extern unsigned int   __read_prog_uint32(void *addr);

extern void __write_prog_uint8(void *addr, unsigned char val);
extern void __write_prog_uint16(void *addr, unsigned short val);
extern void __write_prog_uint32(void *addr, unsigned int val);

#ifdef __cplusplus
}      /* extern "C" */
#endif

#endif // ifndef CYGONCE_HAL_MIPS_STUB_H
