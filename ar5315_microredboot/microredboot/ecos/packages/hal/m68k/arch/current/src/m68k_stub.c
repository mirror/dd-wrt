//========================================================================
//
//      m68k_stub.c
//
//      Helper functions for stub
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

#include <stddef.h>

#include <pkgconf/hal.h>

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
#include <cyg/hal/hal_stub.h>

#include <cyg/hal/hal_stub.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>

#ifdef CYGDBG_HAL_DEBUG_GDB_THREAD_SUPPORT
#include <cyg/hal/dbg-threads-api.h>    // dbg_currthread_id
#endif

/* Given a trap value TRAP, return the corresponding signal. */

int __computeSignal (unsigned int trap_number)
{
    switch (trap_number)
    {

    case CYGNUM_HAL_VECTOR_BUSERR:
    case CYGNUM_HAL_VECTOR_ADDERR:
        return SIGBUS;

    case CYGNUM_HAL_VECTOR_ILLINST:
        return SIGILL;

    case CYGNUM_HAL_VECTOR_ZERODIV:
        /* This isn't quite accurate: this is integer division only. */
        return SIGFPE;

    case CYGNUM_HAL_VECTOR_CHKINST:
    case CYGNUM_HAL_VECTOR_TRAPVINST:
        return SIGTRAP;

    case CYGNUM_HAL_VECTOR_PRIVVIOLATION:
        return SIGILL;
    case CYGNUM_HAL_VECTOR_TRACE:
        /* Instruction trace */
        return SIGTRAP;

    case CYGNUM_HAL_VECTOR_L1010:
    case CYGNUM_HAL_VECTOR_L1111:
    case CYGNUM_HAL_VECTOR_UNINITINT:
    case CYGNUM_HAL_VECTOR_SPURINT:
        return SIGTRAP;

    case CYGNUM_HAL_VECTOR_TRAPFIRST ... CYGNUM_HAL_VECTOR_TRAPLAST:
        return SIGTRAP;

    case CYGNUM_HAL_VECTOR_AUTOVEC1 ... CYGNUM_HAL_VECTOR_AUTOVEC7:
    case CYGNUM_HAL_VECTOR_INTRFIRST ... CYGNUM_HAL_VECTOR_INTRLAST:
        /* External interrupt */
        return SIGINT;

    default:
        return SIGTERM;
    }
}


/* Return the trap number corresponding to the last-taken trap. */

int __get_trap_number (void)
{
	//extern int hal_m68k_trap_number;

    // The vector is not not part of the GDB register set so get it
    // directly from the save context.
    //return hal_m68k_trap_number;

    return 1;
}

/* Set the currently-saved pc register value to PC. This also updates NPC
   as needed. */

void set_pc (target_register_t pc)
{
    put_register (PC, pc);
}


/*----------------------------------------------------------------------
 * Single-step support
 */

/* Set things up so that the next user resume will execute one instruction.
   This may be done by setting breakpoints or setting a single step flag
   in the saved user registers, for example. */

#define SR_TRACE 0x8000

void __single_step (void)
{
    target_register_t sr = get_register (PS);

    // Set trace flag in the exception context.
    sr |= SR_TRACE;

    put_register (PS, sr);
}

/* Clear the single-step state. */

void __clear_single_step (void)
{
    target_register_t sr = get_register (PS);

    // Clear single-step flag in the exception context.
    sr &= ~SR_TRACE;

    put_register (PS, sr);
}


void __install_breakpoints (void)
{
    /* NOP since single-step HW exceptions are used instead of
       breakpoints. */
}

void __clear_breakpoints (void)
{
}


/* If the breakpoint we hit is in the breakpoint() instruction, return a
   non-zero value. */

int
__is_breakpoint_function ()
{
    return get_register (PC) == (target_register_t)&CYG_LABEL_NAME(breakinst);
}


/* Skip the current instruction.  Since this is only called by the
   stub when the PC points to a breakpoint or trap instruction,
   we can safely just skip 4. */

void __skipinst (void)
{
    put_register (PC, get_register (PC) + 4);
}

#endif // CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS

