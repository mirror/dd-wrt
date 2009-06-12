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
// Author(s):   nickg, jskov (based on the old mn10300 hal_stub.c)
// Contributors:nickg, jskov, dhowells
// Date:        1999-02-12
// Purpose:     Platform specific code for GDB stub support.
//              
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS

#include <cyg/hal/hal_stub.h>

#include <cyg/hal/hal_io.h>             // HAL IO macros
#include <cyg/hal/hal_intr.h>           // HAL interrupt macros

//---------------------------------------------------------------------------
// MN10300 Serial line

// We use serial0 on AM33
#define SERIAL0_CR       ((volatile cyg_uint16 *)0xd4002000)
#define SERIAL0_ICR      ((volatile cyg_uint8 *) 0xd4002004)
#define SERIAL0_TXR      ((volatile cyg_uint8 *) 0xd4002008)
#define SERIAL0_RXR      ((volatile cyg_uint8 *) 0xd4002009)
#define SERIAL0_SR       ((volatile cyg_uint16 *)0xd400200c)

// Timer 2 provides baud rate divisor
#define TIMER0_MD       ((volatile cyg_uint8 *)0xd4003002)
#define TIMER0_BR       ((volatile cyg_uint8 *)0xd4003012)
#define TIMER0_CR       ((volatile cyg_uint8 *)0xd4003022)

#define SIO1_LSTAT_TRDY  0x20
#define SIO1_LSTAT_RRDY  0x10


//---------------------------------------------------------------------------

#ifdef CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT
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

int hal_stb_interruptible(int state)
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

void hal_stb_init_break_irq( void )
{
    // Enable serial receive interrupts.
    HAL_WRITE_UINT8 (SERIAL0_ICR, 0);
    HAL_INTERRUPT_ACKNOWLEDGE (CYGNUM_HAL_INTERRUPT_SERIAL_0_RX)
    HAL_INTERRUPT_UNMASK (CYGNUM_HAL_INTERRUPT_SERIAL_0_RX)
    HAL_ENABLE_INTERRUPTS();
}
#endif

//-----------------------------------------------------------------------------

void hal_stb_platform_init(void)
{
    extern CYG_ADDRESS hal_virtual_vector_table[64];
    extern void init_thread_syscall( void *);
    extern void install_async_breakpoint(void *epc);
//    void (*oldvsr)(void);
    extern void __default_exception_vsr(void);

    // Ensure that the breakpoint VSR points to the default VSR. This will pass
    // it on to the stubs.
//    HAL_VSR_SET( CYGNUM_HAL_VECTOR_BREAKPOINT, __default_exception_vsr, &oldvsr );

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
//-----------------------------------------------------------------------------
// End of plf_stub.c
