//==========================================================================
//
//      io/serial/arm/cma230_serial.c
//
//      Cogent CMA230 Serial I/O Interface Module (interrupt driven)
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
// Author(s):   gthomas
// Contributors:  gthomas
// Date:        1999-02-04
// Purpose:     CMA230 Serial I/O module (interrupt driven version)
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
#include <cyg/hal/hal_cma230.h>
#include <cyg/infra/diag.h>

#ifdef CYGPKG_IO_SERIAL_ARM_CMA230

#include "cma230_serial.h"

typedef struct cma230_serial_info {
    CYG_ADDRWORD   base;
    CYG_WORD       int_num;
    cyg_interrupt  serial_interrupt;
    cyg_handle_t   serial_interrupt_handle;
} cma230_serial_info;

static bool cma230_serial_init(struct cyg_devtab_entry *tab);
static bool cma230_serial_putc(serial_channel *chan, unsigned char c);
static Cyg_ErrNo cma230_serial_lookup(struct cyg_devtab_entry **tab, 
                                   struct cyg_devtab_entry *sub_tab,
                                   const char *name);
static unsigned char cma230_serial_getc(serial_channel *chan);
static Cyg_ErrNo cma230_serial_set_config(serial_channel *chan, cyg_uint32 key,
                                          const void *xbuf, cyg_uint32 *len);
static void cma230_serial_start_xmit(serial_channel *chan);
static void cma230_serial_stop_xmit(serial_channel *chan);

static cyg_uint32 cma230_serial_ISR(cyg_vector_t vector, cyg_addrword_t data);
static void       cma230_serial_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data);

static SERIAL_FUNS(cma230_serial_funs, 
                   cma230_serial_putc, 
                   cma230_serial_getc,
                   cma230_serial_set_config,
                   cma230_serial_start_xmit,
                   cma230_serial_stop_xmit
    );

#ifdef CYGPKG_IO_SERIAL_ARM_CMA230_SERIAL_A
static cma230_serial_info cma230_serial_info0 = {CMA101_DUARTA, 
                                                 CYGNUM_HAL_INTERRUPT_SERIAL_A};
#if CYGNUM_IO_SERIAL_ARM_CMA230_SERIAL_A_BUFSIZE > 0
static unsigned char cma230_serial_out_buf0[CYGNUM_IO_SERIAL_ARM_CMA230_SERIAL_A_BUFSIZE];
static unsigned char cma230_serial_in_buf0[CYGNUM_IO_SERIAL_ARM_CMA230_SERIAL_A_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(cma230_serial_channel0,
                                       cma230_serial_funs, 
                                       cma230_serial_info0,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_CMA230_SERIAL_A_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &cma230_serial_out_buf0[0], sizeof(cma230_serial_out_buf0),
                                       &cma230_serial_in_buf0[0], sizeof(cma230_serial_in_buf0)
    );
#else
static SERIAL_CHANNEL(cma230_serial_channel0,
                      cma230_serial_funs, 
                      cma230_serial_info0,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_CMA230_SERIAL_A_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(cma230_serial_io0, 
             CYGDAT_IO_SERIAL_ARM_CMA230_SERIAL_A_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             cma230_serial_init, 
             cma230_serial_lookup,     // Serial driver may need initializing
             &cma230_serial_channel0
    );
#endif //  CYGPKG_IO_SERIAL_ARM_CMA230_SERIAL_A

#ifdef CYGPKG_IO_SERIAL_ARM_CMA230_SERIAL_B
static cma230_serial_info cma230_serial_info1 = {CMA101_DUARTB,
                                                 CYGNUM_HAL_INTERRUPT_SERIAL_B};
#if CYGNUM_IO_SERIAL_ARM_CMA230_SERIAL_B_BUFSIZE > 0
static unsigned char cma230_serial_out_buf1[CYGNUM_IO_SERIAL_ARM_CMA230_SERIAL_B_BUFSIZE];
static unsigned char cma230_serial_in_buf1[CYGNUM_IO_SERIAL_ARM_CMA230_SERIAL_B_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(cma230_serial_channel1,
                                       cma230_serial_funs, 
                                       cma230_serial_info1,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_CMA230_SERIAL_B_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &cma230_serial_out_buf1[0], sizeof(cma230_serial_out_buf1),
                                       &cma230_serial_in_buf1[0], sizeof(cma230_serial_in_buf1)
    );
#else
static SERIAL_CHANNEL(cma230_serial_channel1,
                      cma230_serial_funs, 
                      cma230_serial_info1,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_CMA230_SERIAL_B_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(cma230_serial_io1, 
             CYGDAT_IO_SERIAL_ARM_CMA230_SERIAL_B_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             cma230_serial_init, 
             cma230_serial_lookup,     // Serial driver may need initializing
             &cma230_serial_channel1
    );
#endif //  CYGPKG_IO_SERIAL_ARM_CMA230_SERIAL_B

// Internal function to actually configure the hardware to desired baud rate, etc.
static bool
cma230_serial_config_port(serial_channel *chan, cyg_serial_info_t *new_config, bool init)
{
    cma230_serial_info *cma230_chan = (cma230_serial_info *)chan->dev_priv;
    volatile struct serial_port *port = (volatile struct serial_port *)cma230_chan->base;
    unsigned short baud_divisor = select_baud[new_config->baud];
    unsigned char _lcr, _ier;
    if (baud_divisor == 0) return false;  // Invalid configuration
    _ier = port->ier;
    port->ier = 0;  // Disable port interrupts while changing hardware
    _lcr = select_word_length[new_config->word_length - CYGNUM_SERIAL_WORD_LENGTH_5] | 
        select_stop_bits[new_config->stop] |
        select_parity[new_config->parity];
    port->lcr = _lcr;
    port->lcr |= LCR_DL;
    port->mdl = baud_divisor >> 8;
    port->ldl = baud_divisor & 0xFF;
    port->lcr &= ~LCR_DL;
    if (init) {
        port->fcr = 0x07;  // Enable and clear FIFO
        if (chan->out_cbuf.len != 0) {
            port->ier = IER_RCV;
        } else {
            port->ier = 0;
        }
        port->mcr = MCR_INT|MCR_DTR|MCR_RTS;  // Master interrupt enable
    } else {
        port->ier = _ier;
    }
    if (new_config != &chan->config) {
        chan->config = *new_config;
    }
    return true;
}

// Function to initialize the device.  Called at bootstrap time.
static bool 
cma230_serial_init(struct cyg_devtab_entry *tab)
{
    serial_channel *chan = (serial_channel *)tab->priv;
    cma230_serial_info *cma230_chan = (cma230_serial_info *)chan->dev_priv;
#ifdef CYGDBG_IO_INIT
    diag_printf("CMA230 SERIAL init - dev: %x.%d\n", cma230_chan->base, cma230_chan->int_num);
#endif
    (chan->callbacks->serial_init)(chan);  // Really only required for interrupt driven devices
    if (chan->out_cbuf.len != 0) {
        cyg_drv_interrupt_create(cma230_chan->int_num,
                                 99,                     // Priority - what goes here?
                                 (cyg_addrword_t)chan,   //  Data item passed to interrupt handler
                                 cma230_serial_ISR,
                                 cma230_serial_DSR,
                                 &cma230_chan->serial_interrupt_handle,
                                 &cma230_chan->serial_interrupt);
        cyg_drv_interrupt_attach(cma230_chan->serial_interrupt_handle);
        cyg_drv_interrupt_unmask(cma230_chan->int_num);
    }
    cma230_serial_config_port(chan, &chan->config, true);
    return true;
}

// This routine is called when the device is "looked" up (i.e. attached)
static Cyg_ErrNo 
cma230_serial_lookup(struct cyg_devtab_entry **tab, 
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
cma230_serial_putc(serial_channel *chan, unsigned char c)
{
    cma230_serial_info *cma230_chan = (cma230_serial_info *)chan->dev_priv;
    volatile struct serial_port *port = (volatile struct serial_port *)cma230_chan->base;
    if (port->lsr & LSR_THE) {
// Transmit buffer is empty
        port->thr = c;
        return true;
    } else {
// No space
        return false;
    }
}

// Fetch a character from the device input buffer, waiting if necessary
static unsigned char 
cma230_serial_getc(serial_channel *chan)
{
    unsigned char c;
    cma230_serial_info *cma230_chan = (cma230_serial_info *)chan->dev_priv;
    volatile struct serial_port *port = (volatile struct serial_port *)cma230_chan->base;
    while ((port->lsr & LSR_RSR) == 0) ;   // Wait for char
    c = port->rhr;
    return c;
}

// Set up the device characteristics; baud rate, etc.
static Cyg_ErrNo
cma230_serial_set_config(serial_channel *chan, cyg_uint32 key,
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
        if ( true != cma230_serial_config_port(chan, config, false) )
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
cma230_serial_start_xmit(serial_channel *chan)
{
    cma230_serial_info *cma230_chan = (cma230_serial_info *)chan->dev_priv;
    volatile struct serial_port *port = (volatile struct serial_port *)cma230_chan->base;
    port->ier |= IER_XMT;  // Enable xmit interrupt
}

// Disable the transmitter on the device
static void 
cma230_serial_stop_xmit(serial_channel *chan)
{
    cma230_serial_info *cma230_chan = (cma230_serial_info *)chan->dev_priv;
    volatile struct serial_port *port = (volatile struct serial_port *)cma230_chan->base;
    port->ier &= ~IER_XMT;  // Disable xmit interrupt
}

// Serial I/O - low level interrupt handler (ISR)
static cyg_uint32 
cma230_serial_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    cma230_serial_info *cma230_chan = (cma230_serial_info *)chan->dev_priv;
    cyg_drv_interrupt_mask(cma230_chan->int_num);
    cyg_drv_interrupt_acknowledge(cma230_chan->int_num);
    return CYG_ISR_CALL_DSR;  // Cause DSR to be run
}

// Serial I/O - high level interrupt handler (DSR)
static void       
cma230_serial_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    cma230_serial_info *cma230_chan = (cma230_serial_info *)chan->dev_priv;
    volatile struct serial_port *port = (volatile struct serial_port *)cma230_chan->base;
    unsigned char isr;
    isr = port->isr & 0x0E;
    if (isr == ISR_Tx) {
        (chan->callbacks->xmt_char)(chan);
    } else if (isr == ISR_Rx) {
        (chan->callbacks->rcv_char)(chan, port->rhr);
    }
    cyg_drv_interrupt_unmask(cma230_chan->int_num);
}
#endif
