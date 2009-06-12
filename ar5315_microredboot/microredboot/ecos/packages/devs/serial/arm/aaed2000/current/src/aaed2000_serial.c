//==========================================================================
//
//      io/serial/arm/aaed2000_serial.c
//
//      Agilent AAED2000 Serial I/O Interface Module (interrupt driven)
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
// Author(s):   gthomas, jskov
// Contributors:gthomas, jskov
// Date:        2001-11-12
// Purpose:     AAED2000 Serial I/O module (interrupt driven version)
// Description: 
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/io_serial.h>
#include <pkgconf/io.h>
#include <cyg/io/io.h>
#include <cyg/hal/hal_intr.h>           // interrupt
#include <cyg/hal/hal_io.h>             // register base
#include <cyg/io/devtab.h>
#include <cyg/io/serial.h>
#include <cyg/infra/diag.h>

#include "aaed2000_serial.h"

typedef struct aaed2000_serial_info {
    CYG_ADDRWORD   base;
    CYG_WORD       int_num;
    cyg_interrupt  serial_interrupt;
    cyg_handle_t   serial_interrupt_handle;
} aaed2000_serial_info;

static bool aaed2000_serial_init(struct cyg_devtab_entry *tab);
static bool aaed2000_serial_putc(serial_channel *chan, unsigned char c);
static Cyg_ErrNo aaed2000_serial_lookup(struct cyg_devtab_entry **tab, 
                                   struct cyg_devtab_entry *sub_tab,
                                   const char *name);
static unsigned char aaed2000_serial_getc(serial_channel *chan);
static Cyg_ErrNo aaed2000_serial_set_config(serial_channel *chan, 
                                       cyg_uint32 key,
                                       const void *xbuf, cyg_uint32 *len);
static void aaed2000_serial_start_xmit(serial_channel *chan);
static void aaed2000_serial_stop_xmit(serial_channel *chan);

static cyg_uint32 aaed2000_serial_ISR(cyg_vector_t vector, cyg_addrword_t data);
static void       aaed2000_serial_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data);

static SERIAL_FUNS(aaed2000_serial_funs, 
                   aaed2000_serial_putc, 
                   aaed2000_serial_getc,
                   aaed2000_serial_set_config,
                   aaed2000_serial_start_xmit,
                   aaed2000_serial_stop_xmit
    );

#ifdef CYGPKG_IO_SERIAL_ARM_AAED2000_SERIAL0
static aaed2000_serial_info aaed2000_serial_info0 = {0x80000800,
                                                     CYGNUM_HAL_INTERRUPT_UART3INTR};
#if CYGNUM_IO_SERIAL_ARM_AAED2000_SERIAL0_BUFSIZE > 0
static unsigned char aaed2000_serial_out_buf0[CYGNUM_IO_SERIAL_ARM_AAED2000_SERIAL0_BUFSIZE];
static unsigned char aaed2000_serial_in_buf0[CYGNUM_IO_SERIAL_ARM_AAED2000_SERIAL0_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(aaed2000_serial_channel0,
                                       aaed2000_serial_funs, 
                                       aaed2000_serial_info0,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_AAED2000_SERIAL0_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &aaed2000_serial_out_buf0[0], sizeof(aaed2000_serial_out_buf0),
                                       &aaed2000_serial_in_buf0[0], sizeof(aaed2000_serial_in_buf0)
    );
#else
static SERIAL_CHANNEL(aaed2000_serial_channel0,
                      aaed2000_serial_funs, 
                      aaed2000_serial_info0,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_AAED2000_SERIAL0_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(aaed2000_serial_io0, 
             CYGDAT_IO_SERIAL_ARM_AAED2000_SERIAL0_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             aaed2000_serial_init, 
             aaed2000_serial_lookup,     // Serial driver may need initializing
             &aaed2000_serial_channel0
    );
#endif //  CYGPKG_IO_SERIAL_ARM_AAED2000_SERIAL0

// Internal function to actually configure the hardware to desired baud rate, etc.
static bool
aaed2000_serial_config_port(serial_channel *chan, cyg_serial_info_t *new_config, bool init)
{
    aaed2000_serial_info *aaed2000_chan = (aaed2000_serial_info *)chan->dev_priv;
    CYG_ADDRWORD base = aaed2000_chan->base;
    unsigned short baud_divisor = select_baud[new_config->baud];
    cyg_uint32 _lcr, _intm;
    if (baud_divisor == 0) return false;

    // Register writes don't take effect till the UART is enabled.
    HAL_WRITE_UINT32(base+AAEC_UART_CTRL, AAEC_UART_CTRL_ENAB);

    // Disable port interrupts while changing hardware
    HAL_READ_UINT32(base+AAEC_UART_INTM, _intm);
    HAL_WRITE_UINT32(base+AAEC_UART_INTM, 0);

    _lcr = select_word_length[new_config->word_length - CYGNUM_SERIAL_WORD_LENGTH_5] | 
        select_stop_bits[new_config->stop] |
        select_parity[new_config->parity];
    HAL_WRITE_UINT32(base+AAEC_UART_BAUD, baud_divisor);
    if (init) {
        _lcr |= AAEC_UART_LCR_FIFO;  // Enable FIFO
        if (chan->out_cbuf.len != 0) {
            _intm = AAEC_UART_INT_RIS|AAEC_UART_INT_RTIS;
        } else {
            _intm = 0;
        }
    }
    if (new_config != &chan->config) {
        chan->config = *new_config;
    }

    // Set line control and (re)enable interrupts
    HAL_WRITE_UINT32(base+AAEC_UART_LCR, _lcr);
    HAL_WRITE_UINT32(base+AAEC_UART_INTM, _intm);

    return true;
}

// Function to initialize the device.  Called at bootstrap time.
static bool 
aaed2000_serial_init(struct cyg_devtab_entry *tab)
{
    serial_channel *chan = (serial_channel *)tab->priv;
    aaed2000_serial_info *aaed2000_chan = (aaed2000_serial_info *)chan->dev_priv;
#ifdef CYGDBG_IO_INIT
    diag_printf("AAED2000 SERIAL init - dev: 0x%08x.%d\n", 
                aaed2000_chan->base, aaed2000_chan->int_num);
#endif
    (chan->callbacks->serial_init)(chan);  // Really only required for interrupt driven devices
    if (chan->out_cbuf.len != 0) {
        cyg_drv_interrupt_create(aaed2000_chan->int_num,
                                 1,                    // Priority - unused
                                 (cyg_addrword_t)chan, //  Data item passed to interrupt handler
                                 aaed2000_serial_ISR,
                                 aaed2000_serial_DSR,
                                 &aaed2000_chan->serial_interrupt_handle,
                                 &aaed2000_chan->serial_interrupt);
        cyg_drv_interrupt_attach(aaed2000_chan->serial_interrupt_handle);
        cyg_drv_interrupt_unmask(aaed2000_chan->int_num);
    }
    aaed2000_serial_config_port(chan, &chan->config, true);
    return true;
}

// This routine is called when the device is "looked" up (i.e. attached)
static Cyg_ErrNo 
aaed2000_serial_lookup(struct cyg_devtab_entry **tab, 
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
aaed2000_serial_putc(serial_channel *chan, unsigned char c)
{
    aaed2000_serial_info *aaed2000_chan = (aaed2000_serial_info *)chan->dev_priv;
    CYG_ADDRWORD base = aaed2000_chan->base;
    cyg_uint32 _status;

    HAL_READ_UINT32(base+AAEC_UART_STATUS, _status);
    if (_status & AAEC_UART_STATUS_TxFF) {
        // No space
        return false;
    } else {
        // Transmit buffer is empty
        HAL_WRITE_UINT32(base+AAEC_UART_DATA, (cyg_uint32)c);
        return true;
    }
}

// Fetch a character from the device input buffer, waiting if necessary
static unsigned char 
aaed2000_serial_getc(serial_channel *chan)
{
    aaed2000_serial_info *aaed2000_chan = (aaed2000_serial_info *)chan->dev_priv;
    CYG_ADDRWORD base = aaed2000_chan->base;
    cyg_uint32 _status;
    cyg_uint32 _c;

    do {
        HAL_READ_UINT32(base+AAEC_UART_STATUS, _status);
    } while (_status & AAEC_UART_STATUS_RxFE);

    HAL_READ_UINT32(base+AAEC_UART_DATA, _c);
    return (unsigned char)_c;
}

// Set up the device characteristics; baud rate, etc.
static Cyg_ErrNo
aaed2000_serial_set_config(serial_channel *chan, cyg_uint32 key,
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
        if ( true != aaed2000_serial_config_port(chan, config, false) )
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
aaed2000_serial_start_xmit(serial_channel *chan)
{
    aaed2000_serial_info *aaed2000_chan = (aaed2000_serial_info *)chan->dev_priv;
    CYG_ADDRWORD base = aaed2000_chan->base;
    cyg_uint32 _intm;

    HAL_READ_UINT32(base+AAEC_UART_INTM, _intm);
    _intm |= AAEC_UART_INT_TIS;
    HAL_WRITE_UINT32(base+AAEC_UART_INTM, _intm);
}

// Disable the transmitter on the device
static void 
aaed2000_serial_stop_xmit(serial_channel *chan)
{
    aaed2000_serial_info *aaed2000_chan = (aaed2000_serial_info *)chan->dev_priv;
    CYG_ADDRWORD base = aaed2000_chan->base;
    cyg_uint32 _intm;

    HAL_READ_UINT32(base+AAEC_UART_INTM, _intm);
    _intm &= ~AAEC_UART_INT_TIS;
    HAL_WRITE_UINT32(base+AAEC_UART_INTM, _intm);
}

// Serial I/O - low level interrupt handler (ISR)
static cyg_uint32 
aaed2000_serial_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    aaed2000_serial_info *aaed2000_chan = (aaed2000_serial_info *)chan->dev_priv;
    cyg_drv_interrupt_mask(aaed2000_chan->int_num);
    cyg_drv_interrupt_acknowledge(aaed2000_chan->int_num);
    return CYG_ISR_CALL_DSR;  // Cause DSR to be run
}

// Serial I/O - high level interrupt handler (DSR)
static void       
aaed2000_serial_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    aaed2000_serial_info *aaed2000_chan = (aaed2000_serial_info *)chan->dev_priv;
    CYG_ADDRWORD base = aaed2000_chan->base;
    cyg_uint32 _intres, _c;

    HAL_READ_UINT32(base+AAEC_UART_INTRES, _intres);
    _intres &= (AAEC_UART_INT_TIS | AAEC_UART_INT_RIS | AAEC_UART_INT_RTIS);
    while (_intres) {

        // Empty Rx FIFO
        if (_intres & (AAEC_UART_INT_RIS|AAEC_UART_INT_RTIS)) {
            HAL_READ_UINT32(base+AAEC_UART_DATA, _c);
            (chan->callbacks->rcv_char)(chan, (unsigned char)_c);
        }

        // Fill into Tx FIFO. xmt_char will mask the interrupt when it
        // runs out of chars, so doing this in a loop is OK.
        if (_intres & AAEC_UART_INT_TIS) {
            (chan->callbacks->xmt_char)(chan);
        }

        HAL_READ_UINT32(base+AAEC_UART_INTRES, _intres);
        _intres &= (AAEC_UART_INT_TIS | AAEC_UART_INT_RIS | AAEC_UART_INT_RTIS);
    }

    cyg_drv_interrupt_unmask(aaed2000_chan->int_num);
}
