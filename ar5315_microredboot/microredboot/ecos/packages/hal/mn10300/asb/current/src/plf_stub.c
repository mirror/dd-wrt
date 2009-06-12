//=============================================================================
//
//      plf_stub.c
//
//      Platform specific code for GDB stub support.
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   dmoseley (based on the old mn10300 hal_stub.c)
// Contributors:dmoseley
// Date:        2000-08-11
// Purpose:     Platform specific code for GDB stub support.
//              
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>

#include <cyg/hal/hal_io.h>             // HAL IO macros

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS

#include <cyg/hal/hal_stub.h>

#include <cyg/hal/hal_intr.h>           // HAL interrupt macros

//---------------------------------------------------------------------------

#ifdef CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT

#define ASB2303_SER0_BASE      0xD4002000
#define _SERIAL_RXR      0x09
#define SERIAL0_RXR      ((volatile cyg_uint8 *) (ASB2303_SER0_BASE + _SERIAL_RXR))

// This ISR is called from the interrupt handler. This should only
// happen when there is no serial driver, so the code shouldn't mess
// anything up.
int cyg_hal_gdb_isr(cyg_uint32 vector, target_register_t pc)
{
    if ( CYGNUM_HAL_INTERRUPT_SERIAL_0_RX == vector ) {
        cyg_uint8 c;

        HAL_READ_UINT8 (SERIAL0_RXR, c);
        HAL_INTERRUPT_ACKNOWLEDGE (CYGNUM_HAL_INTERRUPT_SERIAL_0_RX);

        if( 3 == c )
        {
            // Ctrl-C: set a breakpoint at PC so GDB will display the
            // correct program context when stopping rather than the
            // interrupt handler.
            cyg_hal_gdb_interrupt (pc);

            // Interrupt handled. Don't call ISR proper. At return
            // from the VSR, execution will stop at the breakpoint
            // just set.
            return 0;
        }
    }

    // Not caused by GDB. Call ISR proper.
    return 1;
}

#ifndef CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT
int hal_asb_interruptible(int state)
{
    if (state) {
        HAL_WRITE_UINT8 (SERIAL0_ICR, 0);
        HAL_INTERRUPT_ACKNOWLEDGE (CYGNUM_HAL_INTERRUPT_SERIAL_0_RX)
        HAL_INTERRUPT_UNMASK (CYGNUM_HAL_INTERRUPT_SERIAL_0_RX)
    } else {
        HAL_INTERRUPT_MASK (CYGNUM_HAL_INTERRUPT_SERIAL_0_RX)
    }
    return 0;
}

void hal_asb_init_break_irq( void )
{
    // Enable serial receive interrupts.
    HAL_WRITE_UINT8 (SERIAL0_ICR, 0);
    HAL_INTERRUPT_ACKNOWLEDGE (CYGNUM_HAL_INTERRUPT_SERIAL_0_RX)
    HAL_INTERRUPT_UNMASK (CYGNUM_HAL_INTERRUPT_SERIAL_0_RX)
    HAL_ENABLE_INTERRUPTS();
}
#endif
#endif

//-----------------------------------------------------------------------------

void hal_asb_platform_init(void)
{
    extern CYG_ADDRESS hal_virtual_vector_table[64];
    extern void init_thread_syscall( void *);
    extern void install_async_breakpoint(void *epc);
//    void (*oldvsr)(void);
    extern void _default_trap_vsr(void);

    // Ensure that the breakpoint VSR points to the default VSR. This will pass
    // it on to the stubs.
//    HAL_VSR_SET( CYGNUM_HAL_VECTOR_BREAKPOINT, _default_trap_vsr, &oldvsr );

    // Install async breakpoint handler into vector table.
    hal_virtual_vector_table[35] = (CYG_ADDRESS)install_async_breakpoint;

#if !defined(CYGPKG_KERNEL) && defined(CYGDBG_HAL_DEBUG_GDB_THREAD_SUPPORT)
    // Only include this code if we do not have a kernel. Otherwise
    // the kernel supplies the functionality for the app we are linked
    // with.

    // Prepare for application installation of thread info function in
    // vector table.
    hal_virtual_vector_table[15] = 0;
    init_thread_syscall( (void *)&hal_virtual_vector_table[15] );

#endif
}
#endif // ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS

/*------------------------------------------------------------------------*/
/* Reset support                                                          */

#define RSTCTR        0xc0001004
#define CHIPRST       0x01
void hal_asb_reset(void)
{
    // Unfortunately this only resets the MN103E010
    // A full board reset is not done.  ie If the boot block select switched,
    // and a Cygmon reset called the switch change will not occur.  AFAICT
    // the only way to notice that change is to use the Reset switch on the
    // board.
    HAL_WRITE_UINT8(RSTCTR, 0x00);
    HAL_WRITE_UINT8(RSTCTR, CHIPRST);

    // Just in case.
    while (1) ;
}


//-----------------------------------------------------------------------------
// End of plf_stub.c
