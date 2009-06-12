//==========================================================================
//
//      io/serial/mips/upd985xx/upd985xx_serial.c
//
//      NEC MIPS uPD985xx Serial I/O Interface Module (interrupt driven)
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
// Author(s):     hmt
// Contributors:  gthomas
// Date:          2001-07-17
// Purpose:       NEC MIPS uPD985xx Serial I/O module (interrupt driven version)
// Description: 
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/io_serial.h>
#include <pkgconf/io.h>

#include <cyg/io/io.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/io/devtab.h>
#include <cyg/io/serial.h>
#include <cyg/infra/diag.h>

#include "upd985xx_serial.h"

typedef struct upd985xx_serial_info {
//    CYG_ADDRWORD   base;
    CYG_WORD       int_num;
    cyg_interrupt  serial_interrupt;
    cyg_handle_t   serial_interrupt_handle;
} upd985xx_serial_info;

static bool upd985xx_serial_init(struct cyg_devtab_entry *tab);
static bool upd985xx_serial_putc(serial_channel *chan, unsigned char c);
static Cyg_ErrNo upd985xx_serial_lookup(struct cyg_devtab_entry **tab, 
                                   struct cyg_devtab_entry *sub_tab,
                                   const char *name);
static unsigned char upd985xx_serial_getc(serial_channel *chan);
static Cyg_ErrNo upd985xx_serial_set_config(serial_channel *chan, cyg_uint32 key,
                                          const void *xbuf, cyg_uint32 *len);
static void upd985xx_serial_start_xmit(serial_channel *chan);
static void upd985xx_serial_stop_xmit(serial_channel *chan);

static cyg_uint32 upd985xx_serial_ISR(cyg_vector_t vector, cyg_addrword_t data);
static void       upd985xx_serial_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data);

static SERIAL_FUNS(upd985xx_serial_funs, 
                   upd985xx_serial_putc, 
                   upd985xx_serial_getc,
                   upd985xx_serial_set_config,
                   upd985xx_serial_start_xmit,
                   upd985xx_serial_stop_xmit
    );

#ifdef CYGPKG_IO_SERIAL_MIPS_UPD985XX_SERIAL0
static upd985xx_serial_info upd985xx_serial_info0 = {
    // base:    (CYG_ADDRWORD)UPD985XX_UART3_CONTROL0,
    int_num: CYGNUM_HAL_INTERRUPT_UART,
};
#if CYGNUM_IO_SERIAL_MIPS_UPD985XX_SERIAL0_BUFSIZE > 0
static unsigned char upd985xx_serial_out_buf0[CYGNUM_IO_SERIAL_MIPS_UPD985XX_SERIAL0_BUFSIZE];
static unsigned char upd985xx_serial_in_buf0[CYGNUM_IO_SERIAL_MIPS_UPD985XX_SERIAL0_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(
    upd985xx_serial_channel0,
    upd985xx_serial_funs, 
    upd985xx_serial_info0,
    CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_MIPS_UPD985XX_SERIAL0_BAUD),
    CYG_SERIAL_STOP_DEFAULT,
    CYG_SERIAL_PARITY_DEFAULT,
    CYG_SERIAL_WORD_LENGTH_DEFAULT,
    CYG_SERIAL_FLAGS_DEFAULT,
    &upd985xx_serial_out_buf0[0], sizeof(upd985xx_serial_out_buf0),
    &upd985xx_serial_in_buf0[0], sizeof(upd985xx_serial_in_buf0)
    );
#else
static SERIAL_CHANNEL(
    upd985xx_serial_channel0,
    upd985xx_serial_funs, 
    upd985xx_serial_info0,
    CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_MIPS_UPD985XX_SERIAL0_BAUD),
    CYG_SERIAL_STOP_DEFAULT,
    CYG_SERIAL_PARITY_DEFAULT,
    CYG_SERIAL_WORD_LENGTH_DEFAULT,
    CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(upd985xx_serial_io0, 
             CYGDAT_IO_SERIAL_MIPS_UPD985XX_SERIAL0_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             upd985xx_serial_init, 
             upd985xx_serial_lookup,     // Serial driver may need initializing
             &upd985xx_serial_channel0
    );
#endif //  CYGPKG_IO_SERIAL_MIPS_UPD985XX_SERIAL1

// Internal function to actually configure the hardware to desired baud rate, etc.
static bool
upd985xx_serial_config_port(serial_channel *chan, cyg_serial_info_t *new_config, bool init)
{
    unsigned char parity;
    unsigned char word_length;
    unsigned char stop_bits;
    int baud;

    parity = select_parity[new_config->parity];
    word_length = select_word_length[new_config->word_length-CYGNUM_SERIAL_WORD_LENGTH_5];
    stop_bits = select_stop_bits[new_config->stop];
    baud = select_baud[new_config->baud];

    if ((word_length == 0xFF) ||
        (parity == 0xFF) ||
        (stop_bits == 0xFF)) {
        return false;  // Unsupported configuration
    }

    // First ensure we are accessing the right registers.
    // Clear the divisor latch access bit
    *UARTLCR &=~UARTLCR_DLAB;

    // Disable Receiver and Transmitter
    // No such thing in uPD985xx - but we can mask interrupts:
    *UARTIER = 0;

    // Clear sticky (writable) status bits.
    // Ensure it's in 16550 mode at least:
    *UARTFCR = UARTFCR_16550_MODE;

    // Set parity, word length, stop bits (keep DLAB clear)
    *UARTLCR = word_length | parity | stop_bits;

    // Set the desired baud rate.
    *UARTLCR |= UARTLCR_DLAB;
    *UARTDLM = (baud >> 8) & 0xff;
    *UARTDLL = baud & 0xff;
    *UARTLCR &=~UARTLCR_DLAB;

    // Enable the receiver (with interrupts) and the transmitter.
    *UARTIER = UARTIER_ERBFI;

    return true;
}

// Function to initialize the device.  Called at bootstrap time.
static bool 
upd985xx_serial_init(struct cyg_devtab_entry *tab)
{
    serial_channel *chan = (serial_channel *)tab->priv;
    upd985xx_serial_info *upd985xx_chan = (upd985xx_serial_info *)chan->dev_priv;
    int res;
#ifdef CYGDBG_IO_INIT
    diag_printf("UPD985XX SERIAL init - dev: %x.%d\n", 0 /* upd985xx_chan->base */, 
                upd985xx_chan->int_num);
#endif
    (chan->callbacks->serial_init)(chan);  // Really only required for interrupt driven devices
    if (chan->out_cbuf.len != 0) {
        cyg_drv_interrupt_create(upd985xx_chan->int_num,
                                 99,                     // Priority - unused
                                 (cyg_addrword_t)chan,   // Data item passed to interrupt handler
                                 upd985xx_serial_ISR,
                                 upd985xx_serial_DSR,
                                 &upd985xx_chan->serial_interrupt_handle,
                                 &upd985xx_chan->serial_interrupt);
        cyg_drv_interrupt_attach(upd985xx_chan->serial_interrupt_handle);
        cyg_drv_interrupt_unmask(upd985xx_chan->int_num);
    }
    res = upd985xx_serial_config_port(chan, &chan->config, true);
    return res;
}

// This routine is called when the device is "looked" up (i.e. attached)
static Cyg_ErrNo 
upd985xx_serial_lookup(struct cyg_devtab_entry **tab, 
                  struct cyg_devtab_entry *sub_tab,
                  const char *name)
{
    serial_channel *chan = (serial_channel *)(*tab)->priv;
    (chan->callbacks->serial_init)(chan);  // Really only required for interrupt driven devices
    return ENOERR;
}

// Send a character to the device output buffer.
// Return 'true' if character is sent to device
static bool
upd985xx_serial_putc(serial_channel *chan, unsigned char c)
{
    if ( 0 == (UARTLSR_THRE & *UARTLSR) )
        return false;
    *UARTTHR = (unsigned int)c;
    return true;
}

// Fetch a character from the device input buffer, waiting if necessary
static unsigned char 
upd985xx_serial_getc(serial_channel *chan)
{
    while ( 0 == (UARTLSR_DR & *UARTLSR) )
        /* do nothing */ ;

    return (char)*UARTRBR;
}

// Set up the device characteristics; baud rate, etc.
static Cyg_ErrNo
upd985xx_serial_set_config(serial_channel *chan, cyg_uint32 key,
                         const void *xbuf, cyg_uint32 *len)
{
    switch (key) {
    case CYG_IO_SET_CONFIG_SERIAL_INFO:
      {
        cyg_serial_info_t *config = (cyg_serial_info_t *)xbuf;
        if ( *len < sizeof(cyg_serial_info_t) ) {
            return -EINVAL;
        }
        *len = sizeof(cyg_serial_info_t);
        if ( true != upd985xx_serial_config_port(chan, config, false) )
            return -EINVAL;
      }
      break;
    default:
        return -EINVAL;
    }
    return ENOERR;
}

// Enable the transmitter on the device
static void
upd985xx_serial_start_xmit(serial_channel *chan)
{
    (chan->callbacks->xmt_char)(chan);  // Kick transmitter (if necessary)
    *UARTIER |= UARTIER_ERBEI;          // enable interrupts
}

// Disable the transmitter on the device
static void 
upd985xx_serial_stop_xmit(serial_channel *chan)
{
    *UARTIER &=~UARTIER_ERBEI;          // disable interrupts
}

// Serial I/O - low level interrupt handler (ISR)
static cyg_uint32 
upd985xx_serial_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    cyg_drv_interrupt_mask(vector);
    return CYG_ISR_CALL_DSR;  // Cause DSR to be run
}

// Serial I/O - high level interrupt handler (DSR)
static void       
upd985xx_serial_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;

    int stat = *UARTIIR;

    stat &= UARTIIR_UIID_MASK;

    if (stat == UARTIIR_TX_EMPTY) {
        (chan->callbacks->xmt_char)(chan);
    }
    if (stat == UARTIIR_RXD_AVAIL) {
        while (0 != (UARTLSR_DR & *UARTLSR)) {
            (chan->callbacks->rcv_char)(chan, (char)*UARTRBR);
        }
    }
    cyg_drv_interrupt_acknowledge(vector);
    cyg_drv_interrupt_unmask(vector);
}

// ------------------------------------------------------------------------
// EOF upd985xx_serial.c
