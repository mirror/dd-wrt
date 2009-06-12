//==========================================================================
//
//      atlas_serial.c
//
//      Serial device driver for ATLAS on-chip serial devices
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
// Author(s):   dmoseley, based on POWERPC driver by jskov
// Contributors: gthomas, jskov, dmoseley
// Date:        2000-06-23
// Purpose:     ATLAS serial device driver
// Description: ATLAS serial device driver
//
// To Do:
//   Put in magic to effectively use the FIFOs. Transmitter FIFO fill is a
//   problem, and setting receiver FIFO interrupts to happen only after
//   n chars may conflict with hal diag.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/io_serial.h>
#include <pkgconf/io.h>

#include <cyg/io/io.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/io/devtab.h>
#include <cyg/infra/diag.h>
#include <cyg/io/serial.h>

#ifdef CYGPKG_IO_SERIAL_MIPS_ATLAS

#include "atlas_serial.h"

typedef struct atlas_serial_info {
    CYG_ADDRWORD   base;
    CYG_WORD       int_num;
    cyg_interrupt  serial_interrupt;
    cyg_handle_t   serial_interrupt_handle;
    cyg_uint8      iir;
} atlas_serial_info;

static bool atlas_serial_init(struct cyg_devtab_entry *tab);
static bool atlas_serial_putc(serial_channel *chan, unsigned char c);
static Cyg_ErrNo atlas_serial_lookup(struct cyg_devtab_entry **tab, 
                                   struct cyg_devtab_entry *sub_tab,
                                   const char *name);
static unsigned char atlas_serial_getc(serial_channel *chan);
static Cyg_ErrNo atlas_serial_set_config(serial_channel *chan, cyg_uint32 key,
                                         const void *xbuf, cyg_uint32 *len);
static void atlas_serial_start_xmit(serial_channel *chan);
static void atlas_serial_stop_xmit(serial_channel *chan);

static cyg_uint32 atlas_serial_ISR(cyg_vector_t vector, cyg_addrword_t data);
static void       atlas_serial_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data);

static SERIAL_FUNS(atlas_serial_funs, 
                   atlas_serial_putc, 
                   atlas_serial_getc,
                   atlas_serial_set_config,
                   atlas_serial_start_xmit,
                   atlas_serial_stop_xmit
    );

static atlas_serial_info atlas_serial_info0 ={ATLAS_SER_16550_BASE_A,  
                                              CYGNUM_HAL_INTERRUPT_SER};

#if CYGNUM_IO_SERIAL_MIPS_ATLAS_SERIAL_A_BUFSIZE > 0
static unsigned char atlas_serial_out_buf0[CYGNUM_IO_SERIAL_MIPS_ATLAS_SERIAL_A_BUFSIZE];
static unsigned char atlas_serial_in_buf0[CYGNUM_IO_SERIAL_MIPS_ATLAS_SERIAL_A_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(atlas_serial_channel0,
                                       atlas_serial_funs, 
                                       atlas_serial_info0,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_MIPS_ATLAS_SERIAL_A_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &atlas_serial_out_buf0[0], 
                                       sizeof(atlas_serial_out_buf0),
                                       &atlas_serial_in_buf0[0], 
                                       sizeof(atlas_serial_in_buf0)
    );
#else
static SERIAL_CHANNEL(atlas_serial_channel0,
                      atlas_serial_funs, 
                      atlas_serial_info0,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_MIPS_ATLAS_SERIAL_A_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(atlas_serial_io0, 
             CYGDAT_IO_SERIAL_MIPS_ATLAS_SERIAL_A_NAME,
             0,                 // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             atlas_serial_init, 
             atlas_serial_lookup,     // Serial driver may need initializing
             &atlas_serial_channel0
    );



// Internal function to actually configure the hardware to desired baud rate, etc.
static bool
atlas_serial_config_port(serial_channel *chan, cyg_serial_info_t *new_config, bool init)
{
    atlas_serial_info *atlas_chan = (atlas_serial_info *)chan->dev_priv;
    cyg_addrword_t port = atlas_chan->base;
    cyg_uint16 baud_divisor = select_baud[new_config->baud];
    cyg_uint8 _lcr, _ier;

    if (baud_divisor == 0)
        return false;    // Invalid baud rate selected

    //
    // We may need to increase the timeout before causing a break reset.
    // According to the Atlas Users Manual (Document MD00005) The BRKRES
    // register will need to be programmed with a value larger that 0xA (the default)
    // if we are going to use a baud rate lower than 2400.
    //
    if (new_config->baud <= CYGNUM_SERIAL_BAUD_2400)
    {
        // For now, just disable the break reset entirely.
        HAL_WRITE_UINT32(HAL_ATLAS_BRKRES, 0);
    } else {
        // Put the break reset state back to the default
        HAL_WRITE_UINT32(HAL_ATLAS_BRKRES, HAL_ATLAS_BRKRES_DEFAULT_VALUE);
    }

    // Disable port interrupts while changing hardware
    HAL_READ_UINT8(port+SER_16550_IER, _ier);
    HAL_WRITE_UINT8(port+SER_16550_IER, 0);

    // Set databits, stopbits and parity.
    _lcr = select_word_length[(new_config->word_length -
                               CYGNUM_SERIAL_WORD_LENGTH_5)] | 
        select_stop_bits[new_config->stop] |
        select_parity[new_config->parity];
    HAL_WRITE_UINT8(port+SER_16550_LCR, _lcr);

    // Set baud rate.
    _lcr |= LCR_DL;
    HAL_WRITE_UINT8(port+SER_16550_LCR, _lcr);
    HAL_WRITE_UINT8(port+SER_16550_DLM, baud_divisor >> 8);
    HAL_WRITE_UINT8(port+SER_16550_DLL, baud_divisor & 0xff);
    _lcr &= ~LCR_DL;
    HAL_WRITE_UINT8(port+SER_16550_LCR, _lcr);

    if (init) {
        // Enable and clear FIFO
        HAL_WRITE_UINT8(port+SER_16550_FCR,
                        (FCR_ENABLE | FCR_CLEAR_RCVR | FCR_CLEAR_XMIT));

        if (chan->out_cbuf.len != 0) {
            HAL_WRITE_UINT8(port+SER_16550_IER, SIO_IER_ERDAI);
        } else {
            HAL_WRITE_UINT8(port+SER_16550_IER, 0);
        }
    } else {
        HAL_WRITE_UINT8(port+SER_16550_IER, _ier);
    }
    if (new_config != &chan->config) {
        chan->config = *new_config;
    }
    return true;
}

// Function to initialize the device.  Called at bootstrap time.
static bool 
atlas_serial_init(struct cyg_devtab_entry *tab)
{
    serial_channel *chan = (serial_channel *)tab->priv;
    atlas_serial_info *atlas_chan = (atlas_serial_info *)chan->dev_priv;
#ifdef CYGDBG_IO_INIT
    diag_printf("ATLAS SERIAL init - dev: %x.%d\n", atlas_chan->base, atlas_chan->int_num);
#endif
    (chan->callbacks->serial_init)(chan);  // Really only required for interrupt driven devices
    if (chan->out_cbuf.len != 0) {
        cyg_drv_interrupt_create(atlas_chan->int_num,
                                 0,         // can change IRQ0 priority
                                 (cyg_addrword_t)chan,   //  Data item passed to interrupt handler
                                 atlas_serial_ISR,
                                 atlas_serial_DSR,
                                 &atlas_chan->serial_interrupt_handle,
                                 &atlas_chan->serial_interrupt);
        cyg_drv_interrupt_attach(atlas_chan->serial_interrupt_handle);
        cyg_drv_interrupt_unmask(atlas_chan->int_num);
    }
    atlas_serial_config_port(chan, &chan->config, true);
    return true;
}

// This routine is called when the device is "looked" up (i.e. attached)
static Cyg_ErrNo 
atlas_serial_lookup(struct cyg_devtab_entry **tab, 
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
atlas_serial_putc(serial_channel *chan, unsigned char c)
{
    atlas_serial_info *atlas_chan = (atlas_serial_info *)chan->dev_priv;
    cyg_addrword_t port = atlas_chan->base;
    cyg_uint8 _lsr;

    HAL_READ_UINT8(port+SER_16550_LSR, _lsr);
    if (_lsr & SIO_LSR_THRE) {
        // Transmit buffer is empty
        HAL_WRITE_UINT8(port+SER_16550_THR, c);
        return true;
    } else {
        // No space
        return false;
    }
}

// Fetch a character from the device input buffer, waiting if necessary
static unsigned char 
atlas_serial_getc(serial_channel *chan)
{
    unsigned char c;
    atlas_serial_info *atlas_chan = (atlas_serial_info *)chan->dev_priv;
    cyg_addrword_t port = atlas_chan->base;
    cyg_uint8 _lsr;

    do {
        HAL_READ_UINT8(port+SER_16550_LSR, _lsr);
    } while ((_lsr & SIO_LSR_DR) == 0);
    HAL_READ_UINT8(port+SER_16550_RBR, c);
    return c;
}

// Set up the device characteristics; baud rate, etc.
static Cyg_ErrNo
atlas_serial_set_config(serial_channel *chan, cyg_uint32 key,
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
        if ( true != atlas_serial_config_port(chan, config, false) )
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
atlas_serial_start_xmit(serial_channel *chan)
{
    atlas_serial_info *atlas_chan = (atlas_serial_info *)chan->dev_priv;
    cyg_addrword_t port = atlas_chan->base;
    cyg_uint8 _ier;

    HAL_READ_UINT8(port+SER_16550_IER, _ier);
    _ier |= IER_XMT;                    // Enable xmit interrupt
    HAL_WRITE_UINT8(port+SER_16550_IER, _ier);

    // We should not need to call this here.  THRE Interrupts are enabled, and the DSR
    // below calls this function.  However, sometimes we get called with Master Interrupts
    // disabled, and thus the DSR never runs.  This is unfortunate because it means we
    // will be doing multiple processing steps for the same thing.
    (chan->callbacks->xmt_char)(chan);
}

// Disable the transmitter on the device
static void 
atlas_serial_stop_xmit(serial_channel *chan)
{
    atlas_serial_info *atlas_chan = (atlas_serial_info *)chan->dev_priv;
    cyg_addrword_t port = atlas_chan->base;
    cyg_uint8 _ier;

    HAL_READ_UINT8(port+SER_16550_IER, _ier);
    _ier &= ~IER_XMT;                   // Disable xmit interrupt
    HAL_WRITE_UINT8(port+SER_16550_IER, _ier);
}

// Serial I/O - low level interrupt handler (ISR)
static cyg_uint32 
atlas_serial_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    atlas_serial_info *atlas_chan = (atlas_serial_info *)chan->dev_priv;

    cyg_drv_interrupt_mask(atlas_chan->int_num);
    cyg_drv_interrupt_acknowledge(atlas_chan->int_num);
    return CYG_ISR_CALL_DSR;  // Cause DSR to be run
}

// Serial I/O - high level interrupt handler (DSR)
static void       
atlas_serial_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    atlas_serial_info *atlas_chan = (atlas_serial_info *)chan->dev_priv;
    cyg_addrword_t port = atlas_chan->base;
    cyg_uint8 _iir;

    HAL_READ_UINT8(port+SER_16550_IIR, _iir);
    _iir &= SIO_IIR_ID_MASK;
    if ( ISR_Tx_Empty == _iir ) {
        (chan->callbacks->xmt_char)(chan);
    } else if (( ISR_Rx_Avail == _iir ) || ( ISR_Rx_Char_Timeout == _iir )) {
        cyg_uint8 _c;
        HAL_READ_UINT8(port+SER_16550_RBR, _c);
        (chan->callbacks->rcv_char)(chan, _c);
    }

    cyg_drv_interrupt_unmask(atlas_chan->int_num);
}

#endif

//-------------------------------------------------------------------------
// EOF atlas_serial.c
