//==========================================================================
//
//      hal_misc.c
//
//      HAL miscellaneous functions
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

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros
#include <cyg/infra/diag.h>             // diag_printf

#include <cyg/hal/hal_arch.h>           // HAL header

#include <cyg/hal/hal_intr.h>           // VSR/ISR defines

//--------------------------------------------------------------------------
// ISR tables
volatile CYG_ADDRESS  cyg_hal_interrupt_handlers[CYGNUM_HAL_ISR_COUNT];
volatile CYG_ADDRWORD cyg_hal_interrupt_data[CYGNUM_HAL_ISR_COUNT];
volatile CYG_ADDRESS  cyg_hal_interrupt_objects[CYGNUM_HAL_ISR_COUNT];

//--------------------------------------------------------------------------
// VSR table

//      The cyg_hal_vsr_table table is variant-specific.  Some processors must
// have the VSR table at specific locations.

/*****************************************************************************
hal_default_exception_handler -- First level C exception handler

     The assembly default VSR  handler calls  this routine  to handler  the
exception.  When this routine returns, the  state is restored to the  state
pointed to by regs.

     We declare this  routine as  weak so  that other  handlers can  easily
become the default exception handler.

INPUT:

     vector: The exception vector number.

     regs: A pointer to the saved state.

OUTPUT:

RETURN VALUE:

     None

*****************************************************************************/

externC void
hal_default_exception_handler(CYG_WORD vector, HAL_SavedRegisters *regs)
                                                    __attribute__ ((weak));

void hal_default_exception_handler(CYG_WORD vector, HAL_SavedRegisters *regs)
{

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
    externC void __handle_exception(void);
    externC HAL_SavedRegisters * _hal_registers;

    // Set the pointer to the registers of the current exception
    // context. At entry the GDB stub will expand the
    // HAL_SavedRegisters structure into a (bigger) register array.
    _hal_registers = regs;

    __handle_exception();

#elif defined(CYGFUN_HAL_COMMON_KERNEL_SUPPORT) && \
      defined(CYGPKG_HAL_EXCEPTIONS)

    // We should decode the vector and pass a more appropriate
    // value as the second argument. For now we simply pass a
    // pointer to the saved registers. We should also divert
    // breakpoint and other debug vectors into the debug stubs.

    cyg_hal_deliver_exception(vector, (CYG_ADDRWORD)regs);

#else

    CYG_FAIL("Exception!!!");

#endif

    return;
}

//---------------------------------------------------------------------------
// Default ISRs

externC cyg_uint32
hal_default_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data)
{
    diag_printf("Spurious Interrupt: %d\n", vector);

//    CYG_FAIL("Spurious Interrupt!!!");
    return 0;
}

//---------------------------------------------------------------------------
// Idle thread action

void
hal_idle_thread_action( cyg_uint32 count )
{
}

//---------------------------------------------------------------------------
// Determine the index of the ls bit of the supplied mask.

cyg_uint32
hal_lsbit_index(cyg_uint32 mask)
{
    cyg_uint32 n = mask;

    static const signed char tab[64] =
    { -1, 0, 1, 12, 2, 6, 0, 13, 3, 0, 7, 0, 0, 0, 0, 14, 10,
      4, 0, 0, 8, 0, 0, 25, 0, 0, 0, 0, 0, 21, 27 , 15, 31, 11,
      5, 0, 0, 0, 0, 0, 9, 0, 0, 24, 0, 0 , 20, 26, 30, 0, 0, 0,
      0, 23, 0, 19, 29, 0, 22, 18, 28, 17, 16, 0
    };

    n &= ~(n-1UL);
    n = (n<<16)-n;
    n = (n<<6)+n;
    n = (n<<4)+n;

    return tab[n>>26];
}

//---------------------------------------------------------------------------
// Determine the index of the ms bit of the supplied mask.

cyg_uint32
hal_msbit_index(cyg_uint32 mask)
{
    cyg_uint32 x = mask;
    cyg_uint32 w;

    // Phase 1: make word with all ones from that one to the right.
    x |= x >> 16;
    x |= x >> 8;
    x |= x >> 4;
    x |= x >> 2;
    x |= x >> 1;

    // Phase 2: calculate number of "1" bits in the word.
    w = (x & 0x55555555) + ((x >> 1) & 0x55555555);
    w = (w & 0x33333333) + ((w >> 2) & 0x33333333);
    w = w + (w >> 4);
    w = (w & 0x000F000F) + ((w >> 8) & 0x000F000F);
    return (cyg_uint32)((w + (w >> 16)) & 0xFF);

}

//---------------------------------------------------------------------------

