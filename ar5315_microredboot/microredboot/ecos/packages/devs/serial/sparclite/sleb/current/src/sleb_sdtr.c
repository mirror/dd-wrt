//==========================================================================
//
//      io/serial/sparclite/sleb_sdtr.c
//
//      Serial I/O interface module for SPARClite Eval Board (SLEB)
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
// Purpose:     SLEB serial I/O module
// Description: 
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/io.h>
#include <pkgconf/io_serial.h>
#include <cyg/io/io.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/io/devtab.h>
#include <cyg/io/serial.h>

#ifdef  CYGPKG_IO_SERIAL_SPARCLITE_SLEB

#include "sleb_sdtr.h"

extern void diag_printf(const char *fmt, ...);

#define BUFSIZE 128

typedef struct sleb_sdtr_info {
    CYG_ADDRWORD   base;
    CYG_WORD       tx_int_num;
    CYG_WORD       rx_int_num;
    cyg_interrupt  tx_serial_interrupt;
    cyg_handle_t   tx_serial_interrupt_handle;
    cyg_interrupt  rx_serial_interrupt;
    cyg_handle_t   rx_serial_interrupt_handle;
    cyg_uint8      cmd_reg;
    bool           xmit_enabled;
} sleb_sdtr_info;

static bool sleb_sdtr_init(struct cyg_devtab_entry *tab);
static bool sleb_sdtr_putc(serial_channel *chan, unsigned char c);
static Cyg_ErrNo sleb_sdtr_lookup(struct cyg_devtab_entry **tab, 
                                   struct cyg_devtab_entry *sub_tab,
                                   const char *name);
static unsigned char sleb_sdtr_getc(serial_channel *chan);
static Cyg_ErrNo sleb_sdtr_set_config(serial_channel *chan, cyg_uint32 key,
                                      const void *xbuf, cyg_uint32 *len);
static void sleb_sdtr_start_xmit(serial_channel *chan);
static void sleb_sdtr_stop_xmit(serial_channel *chan);

static cyg_uint32 sleb_sdtr_tx_ISR(cyg_vector_t vector, cyg_addrword_t data);
static void       sleb_sdtr_tx_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data);
static cyg_uint32 sleb_sdtr_rx_ISR(cyg_vector_t vector, cyg_addrword_t data);
static void       sleb_sdtr_rx_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data);

static SERIAL_FUNS(sleb_sdtr_funs, 
                   sleb_sdtr_putc, 
                   sleb_sdtr_getc,
                   sleb_sdtr_set_config,
                   sleb_sdtr_start_xmit,
                   sleb_sdtr_stop_xmit
    );

#ifdef CYGPKG_IO_SERIAL_SPARCLITE_SLEB_CON1
static sleb_sdtr_info sleb_sdtr_info0 = {SLEB_SDTR0_BASE, SLEB_SDTR0_TX_INT, SLEB_SDTR0_RX_INT};
#if CYGNUM_IO_SERIAL_SPARCLITE_SLEB_CON1_BUFSIZE > 0
static unsigned char sleb_sdtr_out_buf0[CYGNUM_IO_SERIAL_SPARCLITE_SLEB_CON1_BUFSIZE];
static unsigned char sleb_sdtr_in_buf0[CYGNUM_IO_SERIAL_SPARCLITE_SLEB_CON1_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(sleb_sdtr_channel0,
                                       sleb_sdtr_funs, 
                                       sleb_sdtr_info0,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_SPARCLITE_SLEB_CON1_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &sleb_sdtr_out_buf0[0], sizeof(sleb_sdtr_out_buf0),
                                       &sleb_sdtr_in_buf0[0], sizeof(sleb_sdtr_in_buf0)
    );
#else
static SERIAL_CHANNEL(sleb_sdtr_channel0,
                      sleb_sdtr_funs, 
                      sleb_sdtr_info0,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_SPARCLITE_SLEB_CON1_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(sleb_sdtr_io0, 
             CYGDAT_IO_SERIAL_SPARCLITE_SLEB_CON1_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             sleb_sdtr_init, 
             sleb_sdtr_lookup,     // Serial driver may need initializing
             &sleb_sdtr_channel0
    );
#endif // CYGPKG_IO_SERIAL_SPARCLITE_SLEB_CON1

#ifdef  CYGPKG_IO_SERIAL_SPARCLITE_SLEB_CON2
static sleb_sdtr_info sleb_sdtr_info1 = {SLEB_SDTR1_BASE, SLEB_SDTR1_TX_INT, SLEB_SDTR1_RX_INT};
#if CYGNUM_IO_SERIAL_SPARCLITE_SLEB_CON2_BUFSIZE > 0
static unsigned char sleb_sdtr_out_buf1[CYGNUM_IO_SERIAL_SPARCLITE_SLEB_CON2_BUFSIZE];
static unsigned char sleb_sdtr_in_buf1[CYGNUM_IO_SERIAL_SPARCLITE_SLEB_CON2_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(sleb_sdtr_channel1,
                                       sleb_sdtr_funs, 
                                       sleb_sdtr_info1,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_SPARCLITE_SLEB_CON2_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &sleb_sdtr_out_buf1[0], sizeof(sleb_sdtr_out_buf1),
                                       &sleb_sdtr_in_buf1[0], sizeof(sleb_sdtr_in_buf1)
    );
#else
static SERIAL_CHANNEL(sleb_sdtr_channel1,
                      sleb_sdtr_funs, 
                      sleb_sdtr_info1,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_SPARCLITE_SLEB_CON2_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(sleb_sdtr_io1, 
             CYGDAT_IO_SERIAL_SPARCLITE_SLEB_CON2_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             sleb_sdtr_init, 
             sleb_sdtr_lookup,     // Serial driver may need initializing
             &sleb_sdtr_channel1
    );
#endif // CYGPKG_IO_SERIAL_SPARCLITE_SLEB_CON2

// Internal function to actually configure the hardware to desired baud rate, etc.
static bool
sleb_sdtr_config_port(serial_channel *chan, cyg_serial_info_t *new_config, bool init)
{
    sleb_sdtr_info *sdtr_chan = (sleb_sdtr_info *)chan->dev_priv;
    CYG_ADDRWORD port = sdtr_chan->base;
    cyg_int32 baud_divisor;
    cyg_int32 clk, tval;
    unsigned char mode;
#if 0
    if ((new_config->baud < CYGNUM_SERIAL_BAUD_MIN) || (new_config->baud > CYGNUM_SERIAL_BAUD_MAX))
        return false;  // Invalid baud rate
#endif
    baud_divisor = select_baud[new_config->baud];
    if (baud_divisor == 0)
        return false; // Unsupported baud rate
    // Reset the port
    HAL_SPARC_86940_WRITE(SDTR_CONTROL(port), SDTR_CMD_RST);
    // Write the mode
    mode = SDTR_MODE_MODE_ASYNC16 | 
        select_word_length[new_config->word_length - CYGNUM_SERIAL_WORD_LENGTH_5] | 
        select_stop_bits[new_config->stop] |
        select_parity[new_config->parity];
    HAL_SPARC_86940_WRITE(SDTR_CONTROL(port), mode);
    // Set baud rate clock.
    // ***** CAUTION! Both ports use the same time, thus they must both run at the same baud rate!
    clk = *SLEB_CLOCK_SWITCH;  // Compute board speed
    if (clk & 0x80)  clk = 10;
    clk = (clk & 0x3F) * 1000000;  // in MHz
    tval = (clk / (baud_divisor * 32)) - 1;
    HAL_SPARC_86940_WRITE(SLEB_TIMER3_RELOAD, tval);
    // Set up control register
    sdtr_chan->cmd_reg = SDTR_CMD_RTS | SDTR_CMD_DTR | SDTR_CMD_TxEN;
#ifdef CYGPKG_IO_SERIAL_SPARCLITE_SLEB_CON1
    // Cygmon needs the receiver
    if ((chan->out_cbuf.len != 0) || (chan == &sleb_sdtr_channel0)) {
#else
    if (chan->out_cbuf.len != 0) {
#endif
        sdtr_chan->cmd_reg |= SDTR_CMD_RxEN;
    }
    if (init) {
        sdtr_chan->xmit_enabled = false;
    }
    HAL_SPARC_86940_WRITE(SDTR_CONTROL(port), sdtr_chan->cmd_reg);
    if (new_config != &chan->config)
        chan->config = *new_config;
    return true;
}

// Function to initialize the device.  Called at bootstrap time.
static bool 
sleb_sdtr_init(struct cyg_devtab_entry *tab)
{
    serial_channel *chan = (serial_channel *)tab->priv;
    sleb_sdtr_info *sdtr_chan = (sleb_sdtr_info *)chan->dev_priv;
#ifdef CYGDBG_IO_INIT
    diag_printf("SLEB SERIAL init - dev: %x.%d.%d\n", sdtr_chan->base, sdtr_chan->tx_int_num, sdtr_chan->rx_int_num);
#endif
    (chan->callbacks->serial_init)(chan);  // Really only required for interrupt driven devices
    if (chan->out_cbuf.len != 0) {
        cyg_drv_interrupt_create(sdtr_chan->tx_int_num,
                                 99,                     // Priority - unused
                                 (cyg_addrword_t)chan,   //  Data item passed to interrupt handler
                                 sleb_sdtr_tx_ISR,
                                 sleb_sdtr_tx_DSR,
                                 &sdtr_chan->tx_serial_interrupt_handle,
                                 &sdtr_chan->tx_serial_interrupt);
        cyg_drv_interrupt_attach(sdtr_chan->tx_serial_interrupt_handle);
        cyg_drv_interrupt_mask(sdtr_chan->tx_int_num);
        cyg_drv_interrupt_create(sdtr_chan->rx_int_num,
                                 99,                     // Priority - unused
                                 (cyg_addrword_t)chan,   //  Data item passed to interrupt handler
                                 sleb_sdtr_rx_ISR,
                                 sleb_sdtr_rx_DSR,
                                 &sdtr_chan->rx_serial_interrupt_handle,
                                 &sdtr_chan->rx_serial_interrupt);
        cyg_drv_interrupt_attach(sdtr_chan->rx_serial_interrupt_handle);
        cyg_drv_interrupt_unmask(sdtr_chan->rx_int_num);
    }
    sleb_sdtr_config_port(chan, &chan->config, true);
    return true;
}

// This routine is called when the device is "looked" up (i.e. attached)
static Cyg_ErrNo 
sleb_sdtr_lookup(struct cyg_devtab_entry **tab, 
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
sleb_sdtr_putc(serial_channel *chan, unsigned char c)
{
    sleb_sdtr_info *sdtr_chan = (sleb_sdtr_info *)chan->dev_priv;
    CYG_ADDRWORD port = sdtr_chan->base;
    cyg_uint8 status;
    HAL_SPARC_86940_READ(SDTR_STATUS(port), status);
    if (status & SDTR_STAT_TxRDY) {
// Transmit buffer is empty
        HAL_SPARC_86940_WRITE(SDTR_TXDATA(port), c);
        return true;
    } else {
// No space
        return false;
    }
}

// Fetch a character from the device input buffer, waiting if necessary
static unsigned char 
sleb_sdtr_getc(serial_channel *chan)
{
    unsigned char c;
    sleb_sdtr_info *sdtr_chan = (sleb_sdtr_info *)chan->dev_priv;
    CYG_ADDRWORD port = sdtr_chan->base;
    cyg_uint8 status;
    HAL_SPARC_86940_READ(SDTR_STATUS(port), status);
    while ((status & SDTR_STAT_RxRDY) == 0) 
        HAL_SPARC_86940_READ(SDTR_STATUS(port), status);  // Wait for char
    HAL_SPARC_86940_READ(SDTR_RXDATA(port), c);
    return c;
}

// Set up the device characteristics; baud rate, etc.
static Cyg_ErrNo
sleb_sdtr_set_config(serial_channel *chan, cyg_uint32 key,
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
        if ( true != sleb_sdtr_config_port(chan, config, false) )
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
sleb_sdtr_start_xmit(serial_channel *chan)
{
    sleb_sdtr_info *sdtr_chan = (sleb_sdtr_info *)chan->dev_priv;
    sdtr_chan->xmit_enabled = true;
    cyg_drv_interrupt_unmask(sdtr_chan->tx_int_num);
}

// Disable the transmitter on the device
static void 
sleb_sdtr_stop_xmit(serial_channel *chan)
{
    sleb_sdtr_info *sdtr_chan = (sleb_sdtr_info *)chan->dev_priv;
    cyg_drv_interrupt_mask(sdtr_chan->tx_int_num);
    sdtr_chan->xmit_enabled = false;
}

// Serial I/O - low level interrupt handler (ISR)
static cyg_uint32 
sleb_sdtr_tx_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    sleb_sdtr_info *sdtr_chan = (sleb_sdtr_info *)chan->dev_priv;
    cyg_drv_interrupt_mask(sdtr_chan->tx_int_num);
    cyg_drv_interrupt_acknowledge(sdtr_chan->tx_int_num);
    return CYG_ISR_CALL_DSR;  // Cause DSR to be run
}

// Serial I/O - high level interrupt handler (DSR)
static void       
sleb_sdtr_tx_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    sleb_sdtr_info *sdtr_chan = (sleb_sdtr_info *)chan->dev_priv;
    (chan->callbacks->xmt_char)(chan);
    if (sdtr_chan->xmit_enabled)
        cyg_drv_interrupt_unmask(sdtr_chan->tx_int_num);
}

// Serial I/O - low level interrupt handler (ISR)
static cyg_uint32 
sleb_sdtr_rx_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    sleb_sdtr_info *sdtr_chan = (sleb_sdtr_info *)chan->dev_priv;
    cyg_drv_interrupt_mask(sdtr_chan->rx_int_num);
    return CYG_ISR_CALL_DSR;  // Cause DSR to be run
}

// Serial I/O - high level interrupt handler (DSR)
static void       
sleb_sdtr_rx_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    sleb_sdtr_info *sdtr_chan = (sleb_sdtr_info *)chan->dev_priv;
    CYG_ADDRWORD port = sdtr_chan->base;
    cyg_uint8 status, c;
    HAL_SPARC_86940_READ(SDTR_STATUS(port), status);
    if ((status & SDTR_STAT_RxRDY) != 0) {
        HAL_SPARC_86940_READ(SDTR_RXDATA(port), c);
        (chan->callbacks->rcv_char)(chan, c);
    }
    cyg_drv_interrupt_acknowledge(sdtr_chan->rx_int_num);
    cyg_drv_interrupt_unmask(sdtr_chan->rx_int_num);
}

#endif //  CYGPKG_IO_SERIAL_SPARCLITE_SLEB
