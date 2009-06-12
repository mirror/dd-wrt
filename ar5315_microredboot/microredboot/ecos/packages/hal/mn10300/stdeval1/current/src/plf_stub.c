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
// Contributors:nickg, jskov
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
// We use serial1 on MN103002
#define SERIAL1_CR       ((volatile cyg_uint16 *)0x34000810)
#define SERIAL1_ICR      ((volatile cyg_uint8 *) 0x34000814)
#define SERIAL1_TXR      ((volatile cyg_uint8 *) 0x34000818)
#define SERIAL1_RXR      ((volatile cyg_uint8 *) 0x34000819)
#define SERIAL1_SR       ((volatile cyg_uint16 *)0x3400081c)

// Timer 1 provided baud rate divisor
#define TIMER1_MD       ((volatile cyg_uint8 *)0x34001001)
#define TIMER1_BR       ((volatile cyg_uint8 *)0x34001011)
#define TIMER1_CR       ((volatile cyg_uint8 *)0x34001021)

#define PORT3_MD        ((volatile cyg_uint8 *)0x36008025)

// Mystery register
#define TMPSCNT         ((volatile cyg_uint8 *)0x34001071)

#define SIO1_LSTAT_TRDY  0x20
#define SIO1_LSTAT_RRDY  0x10


//---------------------------------------------------------------------------

#ifdef CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT
// This ISR is called from the interrupt handler. This should only
// happen when there is no serial driver, so the code shouldn't mess
// anything up.
int cyg_hal_gdb_isr(cyg_uint32 vector, target_register_t pc)
{
    if ( CYGNUM_HAL_INTERRUPT_SERIAL_1_RX == vector ) {
        cyg_uint8 c;

        HAL_READ_UINT8 (SERIAL1_RXR, c);
        HAL_INTERRUPT_ACKNOWLEDGE (CYGNUM_HAL_INTERRUPT_SERIAL_1_RX);
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

int hal_stdeval1_interruptible(int state)
{
    if (state) {
        HAL_WRITE_UINT8 (SERIAL1_ICR, 0);
        HAL_INTERRUPT_ACKNOWLEDGE (CYGNUM_HAL_INTERRUPT_SERIAL_1_RX)
        HAL_INTERRUPT_UNMASK (CYGNUM_HAL_INTERRUPT_SERIAL_1_RX)
    } else {
        HAL_INTERRUPT_MASK (CYGNUM_HAL_INTERRUPT_SERIAL_1_RX)
    }
    return 0;
}

void hal_stdeval1_init_break_irq( void )
{
    // Enable serial receive interrupts.
    HAL_WRITE_UINT8 (SERIAL1_ICR, 0);
    HAL_INTERRUPT_ACKNOWLEDGE (CYGNUM_HAL_INTERRUPT_SERIAL_1_RX)
    HAL_INTERRUPT_UNMASK (CYGNUM_HAL_INTERRUPT_SERIAL_1_RX)
    HAL_ENABLE_INTERRUPTS();
}
#endif

// Initialize the current serial port.
void hal_stdeval1_init_serial( void )
{
    // 48 translates to 38400 baud.
    HAL_WRITE_UINT8 (TIMER1_BR, 48);

    // Timer1 sourced from IOCLK
    HAL_WRITE_UINT8 (TIMER1_MD, 0x80);

    // Mode on PORT3, used for serial line controls.
    HAL_WRITE_UINT8 (PORT3_MD, 0x01);

    // No interrupts for now.
    HAL_WRITE_UINT8 (SERIAL1_ICR, 0x00);

    // Source from timer 1, 8bit chars, enable tx and rx
    HAL_WRITE_UINT16 (SERIAL1_CR, 0xc084);
}

// Write C to the current serial port.
void hal_stdeval1_put_char( int c )
{
    cyg_uint16 sr;

    do {
        HAL_READ_UINT16 (SERIAL1_SR, sr);
    } while ((sr & SIO1_LSTAT_TRDY) != 0);

    HAL_WRITE_UINT8 (SERIAL1_TXR, c);
}

// Read one character from the current serial port.
int hal_stdeval1_get_char( void )
{
    char c;
    cyg_uint16 sr;

    do {
        HAL_READ_UINT16 (SERIAL1_SR, sr);
    } while ((sr & SIO1_LSTAT_RRDY) == 0);

    HAL_READ_UINT8 (SERIAL1_RXR, c);

    return c;
}

#endif // ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
//-----------------------------------------------------------------------------
// End of plf_stub.c
