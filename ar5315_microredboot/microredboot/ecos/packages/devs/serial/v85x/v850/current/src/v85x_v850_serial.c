//==========================================================================
//
//      io/serial/v85x/v85x_v850_serial.c
//
//      NEC V850 Serial I/O Interface Module (interrupt driven)
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
// Author(s):    gthomas
// Contributors: gthomas,jlarmour
// Date:         2001-03-21
// Purpose:      V850 Serial I/O module (interrupt driven version)
// Description: 
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/io_serial.h>
#include <pkgconf/io.h>
#ifdef CYGPKG_KERNEL
#include <pkgconf/kernel.h>
#endif
#include CYGBLD_HAL_TARGET_H

#include <cyg/io/io.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/io/devtab.h>
#include <cyg/io/serial.h>
#include <cyg/infra/diag.h>

#ifdef CYGVAR_KERNEL_COUNTERS_CLOCK
#include <cyg/kernel/kapi.h>
#endif

#ifdef CYGPKG_IO_SERIAL_V85X_V850

#include "v85x_v850_serial.h"

typedef struct v850_serial_info {
    CYG_ADDRWORD   base;
    CYG_WORD       int_num;
    cyg_interrupt  serial_interrupt[3];
    cyg_handle_t   serial_interrupt_handle[3];
    cyg_bool       tx_busy;
#ifdef CYGVAR_KERNEL_COUNTERS_CLOCK
    cyg_alarm      tx_timeout;
    cyg_handle_t   tx_timeout_handle;
#endif
} v850_serial_info;

static bool v850_serial_init(struct cyg_devtab_entry *tab);
static bool v850_serial_putc(serial_channel *chan, unsigned char c);
static Cyg_ErrNo v850_serial_lookup(struct cyg_devtab_entry **tab, 
                                    struct cyg_devtab_entry *sub_tab,
                                    const char *name);
static unsigned char v850_serial_getc(serial_channel *chan);
static Cyg_ErrNo v850_serial_set_config(serial_channel *chan, cyg_uint32 key,
                                        const void *xbuf, cyg_uint32 *len);
static void v850_serial_start_xmit(serial_channel *chan);
static void v850_serial_stop_xmit(serial_channel *chan);

static cyg_uint32 v850_serial_ISR(cyg_vector_t vector, cyg_addrword_t data);
static void       v850_serial_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data);

static SERIAL_FUNS(v850_serial_funs, 
                   v850_serial_putc, 
                   v850_serial_getc,
                   v850_serial_set_config,
                   v850_serial_start_xmit,
                   v850_serial_stop_xmit
    );

#ifdef CYGPKG_IO_SERIAL_V85X_V850_SERIAL0
static v850_serial_info v850_serial_info0 = {V850_REG_ASIM0,
                                             CYGNUM_HAL_VECTOR_INTSER0};
#if CYGNUM_IO_SERIAL_V85X_V850_SERIAL0_BUFSIZE > 0
static unsigned char v850_serial_out_buf0[CYGNUM_IO_SERIAL_V85X_V850_SERIAL0_BUFSIZE];
static unsigned char v850_serial_in_buf0[CYGNUM_IO_SERIAL_V85X_V850_SERIAL0_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(v850_serial_channel0,
                                       v850_serial_funs, 
                                       v850_serial_info0,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_V85X_V850_SERIAL0_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &v850_serial_out_buf0[0], sizeof(v850_serial_out_buf0),
                                       &v850_serial_in_buf0[0], sizeof(v850_serial_in_buf0)
    );
#else
static SERIAL_CHANNEL(v850_serial_channel0,
                      v850_serial_funs, 
                      v850_serial_info0,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_V85X_V850_SERIAL0_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(v850_serial_io0, 
             CYGDAT_IO_SERIAL_V85X_V850_SERIAL0_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             v850_serial_init, 
             v850_serial_lookup,     // Serial driver may need initializing
             &v850_serial_channel0
    );
#endif //  CYGPKG_IO_SERIAL_V85X_V850_SERIAL0

// Internal function to actually configure the hardware to desired baud rate, etc.
static bool
v850_serial_config_port(serial_channel *chan, cyg_serial_info_t *new_config, bool init)
{
    v850_serial_info *v850_chan = (v850_serial_info *)chan->dev_priv;
    volatile struct serial_port *port = (volatile struct serial_port *)v850_chan->base;
    unsigned char parity = select_parity[new_config->parity];
    unsigned char word_length = select_word_length[new_config->word_length-CYGNUM_SERIAL_WORD_LENGTH_5];
    unsigned char stop_bits = select_stop_bits[new_config->stop];
    int divisor, count;

    if ((select_baud[new_config->baud].count == 0) ||
        (word_length == 0xFF) ||
        (parity == 0xFF) ||
        (stop_bits == 0xFF)) {
        return false;  // Unsupported configuration
    }
    port->asim = ASIM_TxRx_Tx | ASIM_TxRx_Rx | parity | word_length | stop_bits;
    count = select_baud[new_config->baud].count;
    divisor = select_baud[new_config->baud].divisor;

    while (count > 0xFF) {
        count >>= 1;
        divisor++;
    }

    port->brgc = count;
    
    port->brgm = divisor & 0x07; 
#if CYGINT_HAL_V850_VARIANT_SB1
    port->brgm1 = divisor >> 3;
#endif

    if (new_config != &chan->config) {
        chan->config = *new_config;
    }
    return true;
}

#ifdef CYGVAR_KERNEL_COUNTERS_CLOCK
//
// The serial ports on the V850 are incredibly stupid.  There is no
// interface status register which can tell you if it is possible to
// read or write a character!  The only way to discern this is by using
// interrupts [or at least an interrupt register and polling it].  Thus
// the serial transmit code has a problem in that it will be required by
// upper layers to "send until full".  The only way to decide "not full" is
// that an interrupt has happened.  If the serial driver is being mixed
// with diagnostic I/O, then serial transmit interrupts will possibly be
// lost.  
//
// This code attempts to compenstate by using a kernel alarm to reset the
// "device is busy" flag after some timeout.  The timeout period will be
// sufficiently long so as to not interfere with normal interrupt handling.
//
static void
v850_serial_tx_timeout(cyg_handle_t alarm, cyg_addrword_t p)
{
    v850_serial_info *v850_chan = (v850_serial_info *)p;
    v850_chan->tx_busy = false;
}
#endif

// Function to initialize the device.  Called at bootstrap time.
static bool 
v850_serial_init(struct cyg_devtab_entry *tab)
{
    int i;
    serial_channel *chan = (serial_channel *)tab->priv;
    v850_serial_info *v850_chan = (v850_serial_info *)chan->dev_priv;
#ifdef CYGVAR_KERNEL_COUNTERS_CLOCK
    cyg_handle_t h;
#endif
#ifdef CYGDBG_IO_INIT
    diag_printf("V850 SERIAL init - dev: %x.%d\n", v850_chan->base, v850_chan->int_num);
#endif
    (chan->callbacks->serial_init)(chan);  // Really only required for interrupt driven devices
    if (chan->out_cbuf.len != 0) {
        for (i = 0;  i < 3;  i++) {
            cyg_drv_interrupt_create(v850_chan->int_num+i,
                                     99,                     // Priority - unused
                                     (cyg_addrword_t)chan,   // Data item passed to interrupt handler
                                     v850_serial_ISR,
                                     v850_serial_DSR,
                                     &v850_chan->serial_interrupt_handle[i],
                                     &v850_chan->serial_interrupt[i]);
            cyg_drv_interrupt_attach(v850_chan->serial_interrupt_handle[i]);
            cyg_drv_interrupt_unmask(v850_chan->int_num+i);
#ifdef CYGVAR_KERNEL_COUNTERS_CLOCK
            cyg_clock_to_counter(cyg_real_time_clock(), &h);
            cyg_alarm_create(h, v850_serial_tx_timeout, (cyg_addrword_t)v850_chan, 
                             &v850_chan->tx_timeout_handle, &v850_chan->tx_timeout);
#endif
        }
    }
    v850_chan->tx_busy = false;
    return v850_serial_config_port(chan, &chan->config, true);
}

// This routine is called when the device is "looked" up (i.e. attached)
static Cyg_ErrNo 
v850_serial_lookup(struct cyg_devtab_entry **tab, 
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
v850_serial_putc(serial_channel *chan, unsigned char c)
{
    v850_serial_info *v850_chan = (v850_serial_info *)chan->dev_priv;
    volatile struct serial_port *port = (volatile struct serial_port *)v850_chan->base;
    if (!v850_chan->tx_busy) {
        v850_chan->tx_busy = true;
        port->txs = c;
#ifdef CYGVAR_KERNEL_COUNTERS_CLOCK
        cyg_alarm_initialize(v850_chan->tx_timeout_handle, cyg_current_time()+10, 0);
#endif
        return true;
    } else {
        return false;  // Couldn't send, tx was busy
    }
}

// Fetch a character from the device input buffer, waiting if necessary
static unsigned char 
v850_serial_getc(serial_channel *chan)
{
    v850_serial_info *v850_chan = (v850_serial_info *)chan->dev_priv;
    volatile struct serial_port *port = (volatile struct serial_port *)v850_chan->base;
    return port->rxs;
}

// Set up the device characteristics; baud rate, etc.
static Cyg_ErrNo
v850_serial_set_config(serial_channel *chan, cyg_uint32 key,
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
        if ( true != v850_serial_config_port(chan, config, false) )
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
v850_serial_start_xmit(serial_channel *chan)
{
    v850_serial_info *v850_chan = (v850_serial_info *)chan->dev_priv;
    (chan->callbacks->xmt_char)(chan);  // Kick transmitter (if necessary)
    cyg_drv_interrupt_unmask(v850_chan->int_num+INT_Tx);  // Enable Tx interrupt
}

// Disable the transmitter on the device
static void 
v850_serial_stop_xmit(serial_channel *chan)
{
    v850_serial_info *v850_chan = (v850_serial_info *)chan->dev_priv;
    cyg_drv_interrupt_mask(v850_chan->int_num+INT_Tx);  // Disable Tx interrupt
}

// Serial I/O - low level interrupt handler (ISR)
static cyg_uint32 
v850_serial_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    cyg_drv_interrupt_mask(vector);
    cyg_drv_interrupt_acknowledge(vector);
    return CYG_ISR_CALL_DSR;  // Cause DSR to be run
}

// Serial I/O - high level interrupt handler (DSR)
static void       
v850_serial_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    v850_serial_info *v850_chan = (v850_serial_info *)chan->dev_priv;
    volatile struct serial_port *port = (volatile struct serial_port *)v850_chan->base;
    switch (vector-(v850_chan->int_num)) {
    case INT_ERR:
    case INT_Rx:
        (chan->callbacks->rcv_char)(chan, port->rxs);
        break;
    case INT_Tx:
        v850_chan->tx_busy = false;
#ifdef CYGVAR_KERNEL_COUNTERS_CLOCK
        cyg_alarm_initialize(v850_chan->tx_timeout_handle, 0, 0);
#endif
        (chan->callbacks->xmt_char)(chan);
        break;
    }
    cyg_drv_interrupt_unmask(vector);
}
#endif

// EOF v85x_v850_serial.c
