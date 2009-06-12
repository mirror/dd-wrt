//=============================================================================
//
//      hal_diag.c
//
//      HAL diagnostic output code
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

#include <pkgconf/hal.h>

#include <cyg/hal/hal_diag.h>           // our header.

#if defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS)
#include <cyg/hal/hal_stub.h>           // hal_output_gdb_string
#endif

#include <cyg/infra/cyg_type.h>         // base types, externC
#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_intr.h>           // Interrupt macros

#include <cyg/hal/hal_arch.h>

#define CYG_KERNEL_DIAG_SERIAL

//-----------------------------------------------------------------------------
// Serial diag functions.
#ifdef CYG_KERNEL_DIAG_SERIAL

// Include the serial driver.

void hal_diag_init(void)
{
    CYG_WORD16 clk_div;

    /*   We must first enable the UART output pins from the general-purpose */
    /* I/O module.                                                          */

    /*   Enable the UART0 pins in the port B control register.              */

    MCF5272_SIM->gpio.pbcnt = ((MCF5272_SIM->gpio.pbcnt &
                                ~(MCF5272_GPIO_PBCNT_URT0_MSK)) |
                               (MCF5272_GPIO_PBCNT_URT0_EN));

    /*   Before we do anything else,  make sure  we have  enabled CTS  (our */
    /* RTS) in case the  device  we  are  using  relies  on  hardware  flow */
    /* control.  Note that this step is  our only attempt at hardware  flow */
    /* control.                                                             */

    MCF5272_SIM->uart[0].uop1 = 1;

	/* Initialize UART0 */
	
	/* Reset Transmitter */
    MCF5272_SIM->uart[0].ucr = MCF5272_UART_UCR_RTX;

	/* Reset Receiver */
    MCF5272_SIM->uart[0].ucr = MCF5272_UART_UCR_RRX;

	/* Reset Mode Register */
    MCF5272_SIM->uart[0].ucr = MCF5272_UART_UCR_RMR;

    MCF5272_SIM->uart[0].ucr = MCF5272_UART_UCR_RES;
    MCF5272_SIM->uart[0].ucr = MCF5272_UART_UCR_RBC;

    /*   Mode register 1 sets the UART to  8 data bits with no parity,  and */
    /* mode register 2  forces 1 stop  bit.  Reading or  write to the  mode */
    /* register switches it from umr1 to umr2.  To set it to umr1, we  must */
    /* write a reset mode register command to the command register.         */

    MCF5272_SIM->uart[0].umr = MCF5272_UART_UMR_8BNP;
    MCF5272_SIM->uart[0].umr = MCF5272_UART_UMR_1S;

    /*   Select a prescaled (by 1/32) CLKIN for the clock source.           */

    MCF5272_SIM->uart[0].usr_ucsr = MCF5272_UART_UCSR_CLKIN;

    /*   Calculate baud settings                                            */
    clk_div = (CYG_WORD16)
              ((CYGHWR_HAL_SYSTEM_CLOCK_MHZ*1000000)/
               (CYGHWR_HAL_M68K_MCF52xx_MCF5272_MCF5272C3_DIAG_BAUD * 32));
    MCF5272_SIM->uart[0].udu = clk_div >> 8;
    MCF5272_SIM->uart[0].udl = clk_div & 0x00ff;

    /*   Enable the transmitter and receiver.                               */
    MCF5272_SIM->uart[0].ucr = MCF5272_UART_UCR_TXRXEN;

}

void hal_diag_write_char(cyg_int8 ch)
{

    /* Loop until the transmit data holding register is empty. */
    while (!(MCF5272_SIM->uart[0].usr_ucsr & MCF5272_UART_USR_TXRDY));

    /* Write the character to the transmit status register. */
    MCF5272_SIM->uart[0].urb_utb = ch;

    /*   Loop until the transmit data  FIFO  and  the  shift  register  are */
    /* empty.                                                               */

    while ((MCF5272_SIM->uart[0].utf & MCF5272_UART_UTF_TXB) ||
           (!(MCF5272_SIM->uart[0].usr_ucsr & MCF5272_UART_USR_TXEMP)));

}

cyg_int8 hal_diag_read_char(void)
{

    return 0;
}

#endif // ifdef CYG_KERNEL_DIAG_SERIAL

//-----------------------------------------------------------------------------
// End of hal_diag.c

