//==========================================================================
//
//      io/serial/arm/e7t_serial.c
//
//      ARM AEB-2 Serial I/O Interface Module (interrupt driven)
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
// ====================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    Lars.Lindqvist@combitechsystems.com
// Contributors: jlarmour
// Date:         2001-10-19
// Purpose:      ARM AEB-2 Serial I/O Interface Module (interrupt driven)
// Description:
//
//####DESCRIPTIONEND####
//
// ====================================================================

#include <pkgconf/system.h>
#include <pkgconf/io_serial.h>
#include <pkgconf/io.h>
#include <cyg/io/io.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/io/devtab.h>
#include <cyg/io/serial.h>
#include <cyg/io/serialio.h>
#include <cyg/infra/diag.h>

#ifdef CYGPKG_IO_SERIAL_ARM_E7T

#include "e7t_serial.h"

typedef struct e7t_serial_info {
    CYG_ADDRWORD   base;
    CYG_WORD       tx_int_num;
    CYG_WORD       rx_int_num;
    cyg_interrupt  serial_tx_interrupt;
    cyg_interrupt  serial_rx_interrupt;
    cyg_handle_t   serial_tx_interrupt_handle;
    cyg_handle_t   serial_rx_interrupt_handle;
    bool           tx_enabled;
} e7t_serial_info;

static bool e7t_serial_init(struct cyg_devtab_entry *tab);
static bool e7t_serial_putc(serial_channel *chan, unsigned char c);
static Cyg_ErrNo e7t_serial_lookup(struct cyg_devtab_entry **tab, 
                                   struct cyg_devtab_entry *sub_tab,
                                   const char *name);
static unsigned char e7t_serial_getc(serial_channel *chan);
static Cyg_ErrNo e7t_serial_set_config(serial_channel *chan, 
                                       cyg_uint32 key,
                                       const void *xbuf, cyg_uint32 *len);
static void e7t_serial_start_xmit(serial_channel *chan);
static void e7t_serial_stop_xmit(serial_channel *chan);

static cyg_uint32 e7t_serial_tx_ISR(cyg_vector_t vector, cyg_addrword_t data);
static void       e7t_serial_tx_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data);
static cyg_uint32 e7t_serial_rx_ISR(cyg_vector_t vector, cyg_addrword_t data);
static void       e7t_serial_rx_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data);

static SERIAL_FUNS(e7t_serial_funs, 
                   e7t_serial_putc, 
                   e7t_serial_getc,
                   e7t_serial_set_config,
                   e7t_serial_start_xmit,
                   e7t_serial_stop_xmit
    );

#ifdef CYGPKG_IO_SERIAL_ARM_E7T_SERIAL0
static e7t_serial_info e7t_serial_info0 = {0x07FFd000, 
                                           CYGNUM_HAL_INTERRUPT_UART0_TX,
                                           CYGNUM_HAL_INTERRUPT_UART0_RX};
#if CYGNUM_IO_SERIAL_ARM_E7T_SERIAL0_BUFSIZE > 0
static unsigned char e7t_serial_out_buf0[CYGNUM_IO_SERIAL_ARM_E7T_SERIAL0_BUFSIZE];
static unsigned char e7t_serial_in_buf0[CYGNUM_IO_SERIAL_ARM_E7T_SERIAL0_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(e7t_serial_channel0,
                                       e7t_serial_funs, 
                                       e7t_serial_info0,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_E7T_SERIAL0_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &e7t_serial_out_buf0[0], sizeof(e7t_serial_out_buf0),
                                       &e7t_serial_in_buf0[0], sizeof(e7t_serial_in_buf0)
    );
#else 
static SERIAL_CHANNEL(e7t_serial_channel0,
                      e7t_serial_funs, 
                      e7t_serial_info0,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_E7T_SERIAL0_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(e7t_serial_io0, 
             CYGDAT_IO_SERIAL_ARM_E7T_SERIAL0_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             e7t_serial_init, 
             e7t_serial_lookup,     // Serial driver may need initializing
             &e7t_serial_channel0
    );
#endif //  CYGPKG_IO_SERIAL_ARM_E7T_SERIAL0

#ifdef CYGPKG_IO_SERIAL_ARM_E7T_SERIAL1
static e7t_serial_info e7t_serial_info1 = {0x07FFe000,
                                           CYGNUM_HAL_INTERRUPT_UART1_TX,
                                           CYGNUM_HAL_INTERRUPT_UART1_RX};
#if CYGNUM_IO_SERIAL_ARM_E7T_SERIAL1_BUFSIZE > 0
static unsigned char e7t_serial_out_buf1[CYGNUM_IO_SERIAL_ARM_E7T_SERIAL1_BUFSIZE];
static unsigned char e7t_serial_in_buf1[CYGNUM_IO_SERIAL_ARM_E7T_SERIAL1_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(e7t_serial_channel1,
                                       e7t_serial_funs, 
                                       e7t_serial_info1,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_E7T_SERIAL1_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &e7t_serial_out_buf1[0], sizeof(e7t_serial_out_buf1),
                                       &e7t_serial_in_buf1[0], sizeof(e7t_serial_in_buf1)
    );
#else
static SERIAL_CHANNEL(e7t_serial_channel1,
                      e7t_serial_funs, 
                      e7t_serial_info1,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_E7T_SERIAL1_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(e7t_serial_io1, 
             CYGDAT_IO_SERIAL_ARM_E7T_SERIAL1_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             e7t_serial_init, 
             e7t_serial_lookup,     // Serial driver may need initializing
             &e7t_serial_channel1
    );
#endif //  CYGPKG_IO_SERIAL_ARM_E7T_SERIAL1

// Internal function to actually configure the hardware to desired baud rate, etc.
static bool
e7t_serial_config_port(serial_channel *chan, cyg_serial_info_t *new_config, bool init)
{
  e7t_serial_info *e7t_chan = (e7t_serial_info *)chan->dev_priv;
  volatile struct serial_port *port = (volatile struct serial_port *)e7t_chan->base;
  cyg_uint32 word_length = select_word_length[(new_config->word_length)-CYGNUM_SERIAL_WORD_LENGTH_5];
  cyg_uint32 stop_bits = select_stop_bits[(new_config->stop)-CYGNUM_SERIAL_STOP_1];
  cyg_uint32 parity_mode = select_parity[(new_config->parity)-CYGNUM_SERIAL_PARITY_NONE];
  cyg_uint32 baud_divisor = select_baud[(new_config->baud)-CYGNUM_SERIAL_BAUD_50];
  cyg_uint32 res = word_length | stop_bits | parity_mode | ULCON_SCI | ULCON_IROFF;
  if ((word_length|stop_bits|parity_mode|baud_divisor) == U_NOT_SUPP) {
    return false;
  };
  port->REG_ULCON = res;
  port->REG_UCON = UCON_RXMINT | UCON_RXSIOFF | UCON_TXMINT | UCON_DSROFF | UCON_SBKOFF | UCON_LPBOFF;
  port->REG_UBRDIV = baud_divisor;
  if (new_config != &chan->config) {
    chan->config = *new_config;
  };
  return true;
}

// Function to initialize the device.  Called at bootstrap time.
static bool 
e7t_serial_init(struct cyg_devtab_entry *tab)
{
    serial_channel *chan = (serial_channel *)tab->priv;
    e7t_serial_info *e7t_chan = (e7t_serial_info *)chan->dev_priv;
#ifdef CYGDBG_IO_INIT
    diag_printf("E7T SERIAL init - dev: %x.%d.%d\n", e7t_chan->base, e7t_chan->tx_int_num, e7t_chan->rx_int_num);
#endif
    (chan->callbacks->serial_init)(chan);  // Really only required for interrupt driven devices
    if (chan->out_cbuf.len != 0) {         // If bufferlength > 0 then interrupts are used for tx
        cyg_drv_interrupt_create(e7t_chan->tx_int_num,
                                 99,                     // Priority - unused
                                 (cyg_addrword_t)chan,   //  Data item passed to interrupt handler
                                 e7t_serial_tx_ISR,
                                 e7t_serial_tx_DSR,
                                 &e7t_chan->serial_tx_interrupt_handle,
                                 &e7t_chan->serial_tx_interrupt);
        cyg_drv_interrupt_attach(e7t_chan->serial_tx_interrupt_handle);
        cyg_drv_interrupt_mask(e7t_chan->tx_int_num);
        e7t_chan->tx_enabled = false;
    }
    if (chan->in_cbuf.len != 0) {         // If bufferlength > 0 then interrupts are used for rx
        cyg_drv_interrupt_create(e7t_chan->rx_int_num,
                                 99,                     // Priority - unused
                                 (cyg_addrword_t)chan,   //  Data item passed to interrupt handler
                                 e7t_serial_rx_ISR,
                                 e7t_serial_rx_DSR,
                                 &e7t_chan->serial_rx_interrupt_handle,
                                 &e7t_chan->serial_rx_interrupt);
        cyg_drv_interrupt_attach(e7t_chan->serial_rx_interrupt_handle);
        cyg_drv_interrupt_unmask(e7t_chan->rx_int_num);
    }
    e7t_serial_config_port(chan, &chan->config, true);
    return true;
}

// This routine is called when the device is "looked" up (i.e. attached)
static Cyg_ErrNo 
e7t_serial_lookup(struct cyg_devtab_entry **tab, 
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
e7t_serial_putc(serial_channel *chan, unsigned char c)
{
    e7t_serial_info *e7t_chan = (e7t_serial_info *)chan->dev_priv;
    volatile struct serial_port *port = (volatile struct serial_port *)e7t_chan->base;

    if (port->REG_USTAT & USTAT_TBE) {
      // Transmit buffer is empty
      port->REG_UTXBUF = c;
      return true;
    } else {
      // No space
      return false;
    }
}

// Fetch a character from the device input buffer, waiting if necessary
static unsigned char 
e7t_serial_getc(serial_channel *chan)
{
    unsigned char c;
    e7t_serial_info *e7t_chan = (e7t_serial_info *)chan->dev_priv;
    volatile struct serial_port *port = (volatile struct serial_port *)e7t_chan->base;
    while ((port->REG_USTAT & USTAT_RDR) == 0) ;   // Wait for char
    c = port->REG_URXBUF;
    return c;
}

// Set up the device characteristics; baud rate, etc.
static Cyg_ErrNo
e7t_serial_set_config(serial_channel *chan, cyg_uint32 key,
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
        if ( true != e7t_serial_config_port(chan, config, false) )
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
e7t_serial_start_xmit(serial_channel *chan)
{
    e7t_serial_info *e7t_chan = (e7t_serial_info *)chan->dev_priv;
    e7t_chan->tx_enabled = true;
    (chan->callbacks->xmt_char)(chan);
    if (e7t_chan->tx_enabled) {
        cyg_drv_interrupt_unmask(e7t_chan->tx_int_num);
    }
}

// Disable the transmitter on the device
static void 
e7t_serial_stop_xmit(serial_channel *chan)
{
    e7t_serial_info *e7t_chan = (e7t_serial_info *)chan->dev_priv;
    cyg_drv_interrupt_mask(e7t_chan->tx_int_num);
    e7t_chan->tx_enabled = false;
}

// Serial I/O - low level tx interrupt handler (ISR)
static cyg_uint32 
e7t_serial_tx_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    e7t_serial_info *e7t_chan = (e7t_serial_info *)chan->dev_priv;
    cyg_drv_interrupt_mask(e7t_chan->tx_int_num);
    cyg_drv_interrupt_acknowledge(e7t_chan->tx_int_num);
    return CYG_ISR_CALL_DSR;  // Cause DSR to be run
}

// Serial I/O - high level tx interrupt handler (DSR)
static void       
e7t_serial_tx_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    e7t_serial_info *e7t_chan = (e7t_serial_info *)chan->dev_priv;
    (chan->callbacks->xmt_char)(chan);
    if (e7t_chan->tx_enabled) {
        cyg_drv_interrupt_unmask(e7t_chan->tx_int_num);
    }
}

// Serial I/O - low level rx interrupt handler (ISR)
static cyg_uint32 
e7t_serial_rx_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    e7t_serial_info *e7t_chan = (e7t_serial_info *)chan->dev_priv;
    cyg_drv_interrupt_mask(e7t_chan->rx_int_num);
    cyg_drv_interrupt_acknowledge(e7t_chan->rx_int_num);
    return CYG_ISR_CALL_DSR;  // Cause DSR to be run
}

// Serial I/O - high level rx interrupt handler (DSR)
static void       
e7t_serial_rx_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    e7t_serial_info *e7t_chan = (e7t_serial_info *)chan->dev_priv;
    volatile struct serial_port *port = (volatile struct serial_port *)e7t_chan->base;
    (chan->callbacks->rcv_char)(chan, port->REG_URXBUF);
    cyg_drv_interrupt_unmask(e7t_chan->rx_int_num);
}

#endif // CYGPKG_IO_SERIAL_ARM_E7T

// EOF e7t_serial.c
