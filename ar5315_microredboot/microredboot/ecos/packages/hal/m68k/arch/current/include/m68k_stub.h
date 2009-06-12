#ifndef CYGONCE_HAL_M68K_STUB_H
#define CYGONCE_HAL_M68K_STUB_H
//========================================================================
//
//      m68k_stub.h
//
//      M68K-specific definitions for generic stub
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

#include <pkgconf/system.h>
#include <pkgconf/hal.h>



#ifdef CYGPKG_IO_SERIAL
#include <pkgconf/io_serial.h>
#endif

#include <cyg/hal/hal_diag.h>

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS

#include <cyg/infra/cyg_type.h>         // CYG_UNUSED_PARAM, externC

#define HAL_STUB_PLATFORM_INIT_SERIAL()       HAL_DIAG_INIT()

#define HAL_STUB_PLATFORM_GET_CHAR()                                        \
((cyg_int8)({                                                               \
    cyg_int8 _ch_;                                                          \
    HAL_DIAG_READ_CHAR(_ch_);                                               \
    _ch_;                                                                   \
}))

#define HAL_STUB_PLATFORM_PUT_CHAR(c)         HAL_DIAG_WRITE_CHAR((c))

#define HAL_STUB_PLATFORM_SET_BAUD_RATE(baud) CYG_UNUSED_PARAM(int,(baud))

#define HAL_STUB_PLATFORM_RESET()             HAL_DIAG_INIT()

#define HAL_STUB_PLATFORM_INIT()              HAL_DIAG_INIT()

#endif // ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS



#define NUMREGS 18

#define REGSIZE( _x_ ) (4)

typedef unsigned long target_register_t;

enum regnames {
    D0, D1, D2, D3, D4, D5, D6, D7,
    A0, A1, A2, A3, A4, A5, FP, SP,
    PS, PC,
};

typedef enum regnames regnames_t;

/* Given a trap value TRAP, return the corresponding signal. */
externC int __computeSignal (unsigned int trap_number);

/* Return the SPARC trap number corresponding to the last-taken trap. */
externC int __get_trap_number (void);

/* Return the currently-saved value corresponding to register REG. */
externC target_register_t get_register (regnames_t reg);

/* Store VALUE in the register corresponding to WHICH. */
externC void put_register (regnames_t which, target_register_t value);

/* Set the currently-saved pc register value to PC. This also updates NPC
   as needed. */
externC void set_pc (target_register_t pc);

/* Set things up so that the next user resume will execute one instruction.
   This may be done by setting breakpoints or setting a single step flag
   in the saved user registers, for example. */
externC void __single_step (void);

/* Clear the single-step state. */
externC void __clear_single_step (void);

/* If the breakpoint we hit is in the breakpoint() instruction, return a
   non-zero value. */
externC int __is_breakpoint_function (void);

/* Skip the current instruction. */
externC void __skipinst (void);

externC void __install_breakpoints (void);

externC void __clear_breakpoints (void);

#endif // ifndef CYGONCE_HAL_M68K_STUB_H
