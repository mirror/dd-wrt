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
// Author(s):   dhowells
// Contributors:dmoseley
// Date:        2001-05-17
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

/*---------------------------------------------------------------------------*/
// Define the serial registers.
#define CYG_DEV_RBR 0x00   // receiver buffer register, read, dlab = 0
#define CYG_DEV_THR 0x00   // transmitter holding register, write, dlab = 0
#define CYG_DEV_DLL 0x00   // divisor latch (LS), read/write, dlab = 1
#define CYG_DEV_IER 0x04   // interrupt enable register, read/write, dlab = 0
#define CYG_DEV_DLM 0x04   // divisor latch (MS), read/write, dlab = 1
#define CYG_DEV_IIR 0x08   // interrupt identification register, read, dlab = 0
#define CYG_DEV_FCR 0x08   // fifo control register, write, dlab = 0
#define CYG_DEV_LCR 0x0C   // line control register, read/write
#define CYG_DEV_MCR 0x10   // modem control register, read/write
#define CYG_DEV_LSR 0x14   // line status register, read
#define CYG_DEV_MSR 0x18   // modem status register, read

// Interrupt Enable Register
#define SIO_IER_RCV 0x01
#define SIO_IER_XMT 0x02
#define SIO_IER_LS  0x04
#define SIO_IER_MS  0x08

// The line status register bits.
#define SIO_LSR_DR      0x01            // data ready
#define SIO_LSR_OE      0x02            // overrun error
#define SIO_LSR_PE      0x04            // parity error
#define SIO_LSR_FE      0x08            // framing error
#define SIO_LSR_BI      0x10            // break interrupt
#define SIO_LSR_THRE    0x20            // transmitter holding register empty
#define SIO_LSR_TEMT    0x40            // transmitter register empty
#define SIO_LSR_ERR     0x80            // any error condition

// The modem status register bits.
#define SIO_MSR_DCTS  0x01              // delta clear to send
#define SIO_MSR_DDSR  0x02              // delta data set ready
#define SIO_MSR_TERI  0x04              // trailing edge ring indicator
#define SIO_MSR_DDCD  0x08              // delta data carrier detect
#define SIO_MSR_CTS   0x10              // clear to send
#define SIO_MSR_DSR   0x20              // data set ready
#define SIO_MSR_RI    0x40              // ring indicator
#define SIO_MSR_DCD   0x80              // data carrier detect

// The line control register bits.
#define SIO_LCR_WLS0   0x01             // word length select bit 0
#define SIO_LCR_WLS1   0x02             // word length select bit 1
#define SIO_LCR_STB    0x04             // number of stop bits
#define SIO_LCR_PEN    0x08             // parity enable
#define SIO_LCR_EPS    0x10             // even parity select
#define SIO_LCR_SP     0x20             // stick parity
#define SIO_LCR_SB     0x40             // set break
#define SIO_LCR_DLAB   0x80             // divisor latch access bit

// Modem Control Register
#define SIO_MCR_DTR	0x01
#define SIO_MCR_RTS	0x02
#define SIO_MCR_INT	0x08   // Enable interrupts

#define SERIAL0BASE	0x86FB0000

//---------------------------------------------------------------------------

#ifdef CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT
// This ISR is called from the interrupt handler. This should only
// happen when there is no serial driver, so the code shouldn't mess
// anything up.
int cyg_hal_gdb_isr(cyg_uint32 vector, target_register_t pc)
{
    if ( CYGNUM_HAL_INTERRUPT_SERIAL_0_RX == vector ) {
        cyg_uint8 c;

	HAL_READ_UINT8(SERIAL0BASE+CYG_DEV_RBR,c);
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
void cyg_hal_plf_reset(void)
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
