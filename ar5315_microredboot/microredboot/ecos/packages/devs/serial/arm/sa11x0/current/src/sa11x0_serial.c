//==========================================================================
//
//      io/serial/arm/sa11x0/sa11x0_serial.c
//
//      StrongARM SA11x0 Serial I/O Interface Module (interrupt driven)
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
// Author(s):     gthomas
// Contributors:  gthomas
// Date:          2000-05-08
// Purpose:       StrongARM SA11x0 Serial I/O module (interrupt driven version)
// Description: 
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/io_serial.h>
#include <pkgconf/io.h>
#include <pkgconf/kernel.h>

#include <cyg/io/io.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/io/devtab.h>
#include <cyg/io/serial.h>
#include <cyg/infra/diag.h>

#ifdef CYGPKG_IO_SERIAL_ARM_SA11X0

#include "sa11x0_serial.h"

typedef struct sa11x0_serial_info {
    CYG_ADDRWORD   base;
    CYG_WORD       int_num;
    cyg_interrupt  serial_interrupt;
    cyg_handle_t   serial_interrupt_handle;
} sa11x0_serial_info;

static bool sa11x0_serial_init(struct cyg_devtab_entry *tab);
static bool sa11x0_serial_putc(serial_channel *chan, unsigned char c);
static Cyg_ErrNo sa11x0_serial_lookup(struct cyg_devtab_entry **tab, 
                                   struct cyg_devtab_entry *sub_tab,
                                   const char *name);
static unsigned char sa11x0_serial_getc(serial_channel *chan);
static Cyg_ErrNo sa11x0_serial_set_config(serial_channel *chan, cyg_uint32 key,
                                          const void *xbuf, cyg_uint32 *len);
static void sa11x0_serial_start_xmit(serial_channel *chan);
static void sa11x0_serial_stop_xmit(serial_channel *chan);

static cyg_uint32 sa11x0_serial_ISR(cyg_vector_t vector, cyg_addrword_t data);
static void       sa11x0_serial_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data);

static SERIAL_FUNS(sa11x0_serial_funs, 
                   sa11x0_serial_putc, 
                   sa11x0_serial_getc,
                   sa11x0_serial_set_config,
                   sa11x0_serial_start_xmit,
                   sa11x0_serial_stop_xmit
    );

#ifdef CYGPKG_IO_SERIAL_ARM_SA11X0_SERIAL0
static sa11x0_serial_info sa11x0_serial_info0 = {(CYG_ADDRWORD)SA11X0_UART3_CONTROL0,
                                                 CYGNUM_HAL_INTERRUPT_UART3};
#if CYGNUM_IO_SERIAL_ARM_SA11X0_SERIAL0_BUFSIZE > 0
static unsigned char sa11x0_serial_out_buf0[CYGNUM_IO_SERIAL_ARM_SA11X0_SERIAL0_BUFSIZE];
static unsigned char sa11x0_serial_in_buf0[CYGNUM_IO_SERIAL_ARM_SA11X0_SERIAL0_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(sa11x0_serial_channel0,
                                       sa11x0_serial_funs, 
                                       sa11x0_serial_info0,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_SA11X0_SERIAL0_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &sa11x0_serial_out_buf0[0], sizeof(sa11x0_serial_out_buf0),
                                       &sa11x0_serial_in_buf0[0], sizeof(sa11x0_serial_in_buf0)
    );
#else
static SERIAL_CHANNEL(sa11x0_serial_channel0,
                      sa11x0_serial_funs, 
                      sa11x0_serial_info0,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_SA11X0_SERIAL0_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(sa11x0_serial_io0, 
             CYGDAT_IO_SERIAL_ARM_SA11X0_SERIAL0_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             sa11x0_serial_init, 
             sa11x0_serial_lookup,     // Serial driver may need initializing
             &sa11x0_serial_channel0
    );
#endif //  CYGPKG_IO_SERIAL_ARM_SA11X0_SERIAL1

#ifdef CYGPKG_IO_SERIAL_ARM_SA11X0_SERIAL1
static sa11x0_serial_info sa11x0_serial_info1 = {(CYG_ADDRWORD)SA11X0_UART1_CONTROL0,
                                                 CYGNUM_HAL_INTERRUPT_UART1};
#if CYGNUM_IO_SERIAL_ARM_SA11X0_SERIAL1_BUFSIZE > 0
static unsigned char sa11x0_serial_out_buf1[CYGNUM_IO_SERIAL_ARM_SA11X0_SERIAL1_BUFSIZE];
static unsigned char sa11x0_serial_in_buf1[CYGNUM_IO_SERIAL_ARM_SA11X0_SERIAL1_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(sa11x0_serial_channel1,
                                       sa11x0_serial_funs, 
                                       sa11x0_serial_info1,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_SA11X0_SERIAL1_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &sa11x0_serial_out_buf1[0], sizeof(sa11x0_serial_out_buf1),
                                       &sa11x0_serial_in_buf1[0], sizeof(sa11x0_serial_in_buf1)
    );
#else
static SERIAL_CHANNEL(sa11x0_serial_channel1,
                      sa11x0_serial_funs, 
                      sa11x0_serial_info1,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_SA11X0_SERIAL1_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(sa11x0_serial_io1, 
             CYGDAT_IO_SERIAL_ARM_SA11X0_SERIAL1_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             sa11x0_serial_init, 
             sa11x0_serial_lookup,     // Serial driver may need initializing
             &sa11x0_serial_channel1
    );
#endif //  CYGPKG_IO_SERIAL_ARM_SA11X0_SERIAL1

// Internal function to actually configure the hardware to desired baud rate, etc.
static bool
sa11x0_serial_config_port(serial_channel *chan, cyg_serial_info_t *new_config, bool init)
{
    sa11x0_serial_info *sa11x0_chan = (sa11x0_serial_info *)chan->dev_priv;
    volatile struct serial_port *port = (volatile struct serial_port *)sa11x0_chan->base;
    unsigned char parity = select_parity[new_config->parity];
    unsigned char word_length = select_word_length[new_config->word_length-CYGNUM_SERIAL_WORD_LENGTH_5];
    unsigned char stop_bits = select_stop_bits[new_config->stop];
    int baud = SA11X0_UART_BAUD_RATE_DIVISOR(select_baud[new_config->baud]);
    if ((word_length == 0xFF) ||
        (parity == 0xFF) ||
        (stop_bits == 0xFF)) {
        return false;  // Unsupported configuration
    }
    // Disable Receiver and Transmitter (clears FIFOs)
    port->ctl3 = SA11X0_UART_RX_DISABLED |
                 SA11X0_UART_TX_DISABLED;

    // Clear sticky (writable) status bits.
    port->stat0 = SA11X0_UART_RX_IDLE |
                  SA11X0_UART_RX_BEGIN_OF_BREAK |
                  SA11X0_UART_RX_END_OF_BREAK;

    // Set parity, word length, stop bits
    port->ctl0 = parity |
                 word_length |
                 stop_bits;

    // Set the desired baud rate.
    port->ctl1 = (baud >> 8) & SA11X0_UART_H_BAUD_RATE_DIVISOR_MASK;
    port->ctl2 = baud & SA11X0_UART_L_BAUD_RATE_DIVISOR_MASK;

    // Enable the receiver (with interrupts) and the transmitter.
    port->ctl3 = SA11X0_UART_RX_ENABLED |
                 SA11X0_UART_TX_ENABLED |
                 SA11X0_UART_RX_FIFO_INT_ENABLED;
    return true;
}

// Function to initialize the device.  Called at bootstrap time.
static bool 
sa11x0_serial_init(struct cyg_devtab_entry *tab)
{
    serial_channel *chan = (serial_channel *)tab->priv;
    sa11x0_serial_info *sa11x0_chan = (sa11x0_serial_info *)chan->dev_priv;
    int res;
#ifdef CYGDBG_IO_INIT
    diag_printf("SA11X0 SERIAL init - dev: %x.%d\n", sa11x0_chan->base, sa11x0_chan->int_num);
#endif
    (chan->callbacks->serial_init)(chan);  // Really only required for interrupt driven devices
    if (chan->out_cbuf.len != 0) {
        cyg_drv_interrupt_create(sa11x0_chan->int_num,
                                 99,                     // Priority - unused
                                 (cyg_addrword_t)chan,   // Data item passed to interrupt handler
                                 sa11x0_serial_ISR,
                                 sa11x0_serial_DSR,
                                 &sa11x0_chan->serial_interrupt_handle,
                                 &sa11x0_chan->serial_interrupt);
        cyg_drv_interrupt_attach(sa11x0_chan->serial_interrupt_handle);
        cyg_drv_interrupt_unmask(sa11x0_chan->int_num);
    }
    res = sa11x0_serial_config_port(chan, &chan->config, true);
    return res;
}

// This routine is called when the device is "looked" up (i.e. attached)
static Cyg_ErrNo 
sa11x0_serial_lookup(struct cyg_devtab_entry **tab, 
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
sa11x0_serial_putc(serial_channel *chan, unsigned char c)
{
    sa11x0_serial_info *sa11x0_chan = (sa11x0_serial_info *)chan->dev_priv;
    volatile struct serial_port *port = (volatile struct serial_port *)sa11x0_chan->base;
    if (port->stat1 & SA11X0_UART_TX_FIFO_NOT_FULL) {
        port->data = c;
        return true;
    } else {
        return false;  // Couldn't send, tx was busy
    }
}

// Fetch a character from the device input buffer, waiting if necessary
static unsigned char 
sa11x0_serial_getc(serial_channel *chan)
{
    sa11x0_serial_info *sa11x0_chan = (sa11x0_serial_info *)chan->dev_priv;
    volatile struct serial_port *port = (volatile struct serial_port *)sa11x0_chan->base;
    return port->data;
}

// Set up the device characteristics; baud rate, etc.
static Cyg_ErrNo
sa11x0_serial_set_config(serial_channel *chan, cyg_uint32 key,
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
        if ( true != sa11x0_serial_config_port(chan, config, false) )
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
sa11x0_serial_start_xmit(serial_channel *chan)
{
    sa11x0_serial_info *sa11x0_chan = (sa11x0_serial_info *)chan->dev_priv;
    volatile struct serial_port *port = (volatile struct serial_port *)sa11x0_chan->base;
    (chan->callbacks->xmt_char)(chan);  // Kick transmitter (if necessary)
    port->ctl3 |= SA11X0_UART_TX_FIFO_INT_ENABLED;
}

// Disable the transmitter on the device
static void 
sa11x0_serial_stop_xmit(serial_channel *chan)
{
    sa11x0_serial_info *sa11x0_chan = (sa11x0_serial_info *)chan->dev_priv;
    volatile struct serial_port *port = (volatile struct serial_port *)sa11x0_chan->base;
    port->ctl3 &= ~SA11X0_UART_TX_FIFO_INT_ENABLED;
}

// Serial I/O - low level interrupt handler (ISR)
static cyg_uint32 
sa11x0_serial_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    cyg_drv_interrupt_mask(vector);
    cyg_drv_interrupt_acknowledge(vector);
    return CYG_ISR_CALL_DSR;  // Cause DSR to be run
}

// Serial I/O - high level interrupt handler (DSR)
static void       
sa11x0_serial_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    sa11x0_serial_info *sa11x0_chan = (sa11x0_serial_info *)chan->dev_priv;
    volatile struct serial_port *port = (volatile struct serial_port *)sa11x0_chan->base;
    unsigned int stat0 = port->stat0;
    if (stat0 & SA11X0_UART_TX_SERVICE_REQUEST) {
        (chan->callbacks->xmt_char)(chan);
    }
    if (stat0 & SA11X0_UART_RX_INTS) {
        while (port->stat1 & SA11X0_UART_RX_FIFO_NOT_EMPTY) {
            (chan->callbacks->rcv_char)(chan, port->data);
        }
        port->stat0 = SA11X0_UART_RX_IDLE;  // Need to clear this manually
    }
    if (stat0 & (SA11X0_UART_RX_BEGIN_OF_BREAK |
                 SA11X0_UART_RX_END_OF_BREAK ) ) {
        // Need to clear any of these manually also or noise
        // from plugging in can cause an interrupt loop!
        port->stat0 = stat0 & (SA11X0_UART_RX_BEGIN_OF_BREAK |
                               SA11X0_UART_RX_END_OF_BREAK );
    }
    cyg_drv_interrupt_unmask(vector);
}
#endif
