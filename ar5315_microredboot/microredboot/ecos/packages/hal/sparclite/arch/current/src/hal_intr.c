//==========================================================================
//
//      hal_intr.c
//
//      SPARClite Architecture specific interrupt dispatch tables
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    hmt
// Contributors: hmt
// Date:         1999-02-20
// Purpose:      Interrupt handler tables for SPARClite.
//              
//####DESCRIPTIONEND####
//
//==========================================================================


#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_arch.h>

#include <cyg/infra/cyg_ass.h> // for CYG_FAIL() below

// ------------------------------------------------------------------------
// First level C default interrupt handler.

//static int count = 0;

cyg_uint32 hal_default_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data)
{
    return 0; // 0x1def0000 + vector + (count += 0x0100);
}

// ------------------------------------------------------------------------
// First level C exception handler.

externC void __handle_exception (void);

externC HAL_SavedRegisters *_hal_registers;

void cyg_hal_exception_handler(CYG_ADDRWORD vector, CYG_ADDRWORD data,
                               CYG_ADDRWORD stackpointer )
{
#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
    // Set the pointer to the registers of the current exception
    // context. At entry the GDB stub will expand the
    // HAL_SavedRegisters structure into a (bigger) register array.
    _hal_registers = (HAL_SavedRegisters *)stackpointer;

    __handle_exception();

#elif defined(CYGFUN_HAL_COMMON_KERNEL_SUPPORT) && \
      defined(CYGPKG_HAL_EXCEPTIONS)
    // We should decode the vector and pass a more appropriate
    // value as the second argument. For now we simply pass a
    // pointer to the saved registers. We should also divert
    // breakpoint and other debug vectors into the debug stubs.

    cyg_hal_deliver_exception( vector, stackpointer );

#else
    CYG_FAIL("Exception!!!");
#endif    
    return;
}

// ISR tables
volatile
CYG_ADDRESS    hal_interrupt_handlers[CYGNUM_HAL_VSR_COUNT] = {
    (CYG_ADDRESS)hal_default_isr,
    (CYG_ADDRESS)hal_default_isr,
    (CYG_ADDRESS)hal_default_isr,
    (CYG_ADDRESS)hal_default_isr,

    (CYG_ADDRESS)hal_default_isr,
    (CYG_ADDRESS)hal_default_isr,
    (CYG_ADDRESS)hal_default_isr,
    (CYG_ADDRESS)hal_default_isr,

    (CYG_ADDRESS)hal_default_isr,
    (CYG_ADDRESS)hal_default_isr,
    (CYG_ADDRESS)hal_default_isr,
    (CYG_ADDRESS)hal_default_isr,

    (CYG_ADDRESS)hal_default_isr,
    (CYG_ADDRESS)hal_default_isr,
    (CYG_ADDRESS)hal_default_isr,
    (CYG_ADDRESS)hal_default_isr,  /* 16 of these */

    (CYG_ADDRESS)cyg_hal_exception_handler,
    (CYG_ADDRESS)cyg_hal_exception_handler,
    (CYG_ADDRESS)cyg_hal_exception_handler,
    (CYG_ADDRESS)cyg_hal_exception_handler,
    (CYG_ADDRESS)cyg_hal_exception_handler,

    (CYG_ADDRESS)cyg_hal_exception_handler,
    (CYG_ADDRESS)cyg_hal_exception_handler,
    (CYG_ADDRESS)cyg_hal_exception_handler,
    (CYG_ADDRESS)cyg_hal_exception_handler,
    (CYG_ADDRESS)cyg_hal_exception_handler,

    (CYG_ADDRESS)cyg_hal_exception_handler, /* 11 of these */
};

volatile
CYG_ADDRWORD   hal_interrupt_data[CYGNUM_HAL_VSR_COUNT] = {
 0x11da1a00, 0x11da1a01, 0x11da1a02, 0x11da1a03,
 0x11da1a04, 0x11da1a05, 0x11da1a06, 0x11da1a07,
 0x11da1a08, 0x11da1a09, 0x11da1a0a, 0x11da1a0b,
 0x11da1a0c, 0x11da1a0d, 0x11da1a0e, 0x11da1a0f,
 0xeeda1a00, 0xeeda1a01, 0xeeda1a02, 0xeeda1a03, 0xeeda1a04,
 0xeeda1a05, 0xeeda1a06, 0xeeda1a07, 0xeeda1a08, 0xeeda1a09,
 0xeeda1a0A
};

volatile
CYG_ADDRESS    hal_interrupt_objects[CYGNUM_HAL_VSR_COUNT] = {
    0,    0,    0,    0,
    0,    0,    0,    0,
    0,    0,    0,    0,
    0,    0,    0,    0,

    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,
    0,
};

// EOF hal_intr.c
