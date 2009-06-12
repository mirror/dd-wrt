//==========================================================================
//
//      h8300_sci_serial.c
//
//      H8/300 Serial SCI I/O Interface Module (interrupt driven)
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
// Author(s):   jskov
// Contributors:gthomas, jskov
// Date:        1999-05-24
// Purpose:     H8/300 Serial I/O module (interrupt driven version)
// Description: 
//
// Note: Since interrupt sources from the same SCI channel share the same
//       interrupt level, there is no risk of races when altering the
//       channel's control register from ISRs and DSRs. However, when 
//       altering the control register from user-level code, interrupts
//       must be disabled while the register is being accessed.
//
// FIXME: Receiving in polled mode prevents duplex transfers from working for
//        some reason.
//####DESCRIPTIONEND####
//==========================================================================

#include <pkgconf/io_serial.h>
#include <pkgconf/io.h>

// FIXME: This is necessary since the SCIF driver may be overriding
// CYGDAT_IO_SERIAL_DEVICE_HEADER. Need a better way to include two
// different drivers.
#include <pkgconf/io_serial_h8300_sci.h>

#include <cyg/io/io.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/io/devtab.h>
#include <cyg/infra/diag.h>
#include <cyg/io/serial.h>

#ifdef CYGDAT_IO_SERIAL_H8300_SCI
// The SCI controller register layout on the SH3/7708.
#define SCI_SCSMR                0      // serial mode register
#define SCI_SCBRR                1      // bit rate register
#define SCI_SCSCR                2      // serial control register
#define SCI_SCTDR                3      // transmit data register
#define SCI_SCSSR                4      // serial status register
#define SCI_SCRDR                5      // receive data register
#define SCI_SCSPTR               -4     // serial port register

static short select_word_length[] = {
    -1,
    -1,
    CYGARC_REG_SCSMR_CHR,               // 7 bits
    0                                   // 8 bits
};

static short select_stop_bits[] = {
    -1,
    0,                                  // 1 stop bit
    -1,
    CYGARC_REG_SCSMR_STOP               // 2 stop bits
};

static short select_parity[] = {
    0,                                  // No parity
    CYGARC_REG_SCSMR_PE,                // Even parity
    CYGARC_REG_SCSMR_PE|CYGARC_REG_SCSMR_OE, // Odd parity
    -1,
    -1
};

static unsigned short select_baud[] = {
    0,    // Unused
    CYGARC_SCBRR_CKSx(50)<<8 | CYGARC_SCBRR_N(50),
    CYGARC_SCBRR_CKSx(75)<<8 | CYGARC_SCBRR_N(75),
    CYGARC_SCBRR_CKSx(110)<<8 | CYGARC_SCBRR_N(110),
    CYGARC_SCBRR_CKSx(134)<<8 | CYGARC_SCBRR_N(134),
    CYGARC_SCBRR_CKSx(150)<<8 | CYGARC_SCBRR_N(150),
    CYGARC_SCBRR_CKSx(200)<<8 | CYGARC_SCBRR_N(200),
    CYGARC_SCBRR_CKSx(300)<<8 | CYGARC_SCBRR_N(300),
    CYGARC_SCBRR_CKSx(600)<<8 | CYGARC_SCBRR_N(600),
    CYGARC_SCBRR_CKSx(1200)<<8 | CYGARC_SCBRR_N(1200),
    CYGARC_SCBRR_CKSx(1800)<<8 | CYGARC_SCBRR_N(1800),
    CYGARC_SCBRR_CKSx(2400)<<8 | CYGARC_SCBRR_N(2400),
    CYGARC_SCBRR_CKSx(3600)<<8 | CYGARC_SCBRR_N(3600),
    CYGARC_SCBRR_CKSx(4800)<<8 | CYGARC_SCBRR_N(4800),
    CYGARC_SCBRR_CKSx(7200)<<8 | CYGARC_SCBRR_N(7200),
    CYGARC_SCBRR_CKSx(9600)<<8 | CYGARC_SCBRR_N(9600),
    CYGARC_SCBRR_CKSx(14400)<<8 | CYGARC_SCBRR_N(14400),
    CYGARC_SCBRR_CKSx(19200)<<8 | CYGARC_SCBRR_N(19200),
    CYGARC_SCBRR_CKSx(38400)<<8 | CYGARC_SCBRR_N(38400),
    CYGARC_SCBRR_CKSx(57600)<<8 | CYGARC_SCBRR_N(57600),
    CYGARC_SCBRR_CKSx(115200)<<8 | CYGARC_SCBRR_N(115200),
    CYGARC_SCBRR_CKSx(230400)<<8 | CYGARC_SCBRR_N(230400)
};


typedef struct h8300_sci_info {
    CYG_ADDRWORD   data;                // Pointer to data register
    
    CYG_WORD       er_int_num,          // Error interrupt number
                   rx_int_num,          // Receive interrupt number
                   tx_int_num;          // Transmit interrupt number

    CYG_ADDRWORD   ctrl_base;           // Base address of SCI controller

    cyg_interrupt  serial_er_interrupt, 
                   serial_rx_interrupt, 
                   serial_tx_interrupt;
    cyg_handle_t   serial_er_interrupt_handle, 
                   serial_rx_interrupt_handle, 
                   serial_tx_interrupt_handle;

    bool           tx_enabled;
} sh_sci_info;

static bool h8300_serial_init(struct cyg_devtab_entry *tab);
static bool h8300_serial_putc(serial_channel *chan, unsigned char c);
static Cyg_ErrNo h8300_serial_lookup(struct cyg_devtab_entry **tab, 
                                   struct cyg_devtab_entry *sub_tab,
                                   const char *name);
static unsigned char h8300_serial_getc(serial_channel *chan);
static Cyg_ErrNo h8300_serial_set_config(serial_channel *chan, cyg_uint32 key,
                                      const void *xbuf, cyg_uint32 *len);
static void h8300_serial_start_xmit(serial_channel *chan);
static void h8300_serial_stop_xmit(serial_channel *chan);

static cyg_uint32 h8300_serial_tx_ISR(cyg_vector_t vector, cyg_addrword_t data);
static void       h8300_serial_tx_DSR(cyg_vector_t vector, cyg_ucount32 count, 
                                   cyg_addrword_t data);
static cyg_uint32 h8300_serial_rx_ISR(cyg_vector_t vector, cyg_addrword_t data);
static void       h8300_serial_rx_DSR(cyg_vector_t vector, cyg_ucount32 count, 
                                   cyg_addrword_t data);
static cyg_uint32 h8300_serial_er_ISR(cyg_vector_t vector, cyg_addrword_t data);
static void       h8300_serial_er_DSR(cyg_vector_t vector, cyg_ucount32 count, 
                                   cyg_addrword_t data);

static SERIAL_FUNS(h8300_serial_funs, 
                   h8300_serial_putc, 
                   h8300_serial_getc,
                   h8300_serial_set_config,
                   h8300_serial_start_xmit,
                   h8300_serial_stop_xmit
    );


static h8300_sci_info h8300_serial_info =
{
    NULL,
    CYGNUM_HAL_INTERRUPT_ERI0,
    CYGNUM_HAL_INTERRUPT_RXI0,
    CYGNUM_HAL_INTERRUPT_TXI0,
    CYGARC_REG_SMR0
};

#if CYGNUM_IO_SERIAL_H8300_H8300H_SERIAL1_BUFSIZE > 0
static unsigned char h8300_serial_out_buf[CYGNUM_IO_SERIAL_H8300_CQ7708_SERIAL1_BUFSIZE];
static unsigned char h8300_serial_in_buf[CYGNUM_IO_SERIAL_H8300_CQ7708_SERIAL1_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(h8300_serial_channel,
                                       h8300_serial_funs, 
                                       h8300_serial_info,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_H8300_H8300H_SERIAL1_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &h8300_serial_out_buf[0], 
                                       sizeof(h8300_serial_out_buf),
                                       &h8300_serial_in_buf[0],  
                                       sizeof(h8300_serial_in_buf)
    );
#else
static SERIAL_CHANNEL(h8300_serial_channel,
                      h8300_serial_funs, 
                      h8300_serial_info,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_H8300_H8300H_SERIAL1_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(h8300_serial_io,
             CYGDAT_IO_SERIAL_H8300_H8300H_SERIAL1_NAME,
             0,                 // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             h8300_serial_init, 
             h8300_serial_lookup,          // Serial driver may need initializing
             &h8300_serial_channel
    );

// Internal function to actually configure the hardware to desired baud rate,
// etc.
static bool
h8300_serial_config_port(serial_channel *chan, cyg_serial_info_t *new_config, 
                      bool init)
{
    cyg_uint16 baud_divisor = select_baud[new_config->baud];
    h8300_sci_info *h8300_chan = (h8300_sci_info *)chan->dev_priv;
    cyg_uint8 _scr, _smr;

    // Check configuration request
    if ((-1 == select_word_length[(new_config->word_length -
                                  CYGNUM_SERIAL_WORD_LENGTH_5)])
        || -1 == select_stop_bits[new_config->stop]
        || -1 == select_parity[new_config->parity]
        || baud_divisor == 0)
        return false;

    // Disable SCI interrupts while changing hardware
    HAL_READ_UINT8(h8300_chan->ctrl_base+SCI_SCSCR, _scr);
    HAL_WRITE_UINT8(h8300_chan->ctrl_base+SCI_SCSCR, 0);

    // Set databits, stopbits and parity.
    _smr = select_word_length[(new_config->word_length -
                               CYGNUM_SERIAL_WORD_LENGTH_5)] | 
        select_stop_bits[new_config->stop] |
        select_parity[new_config->parity];
    HAL_WRITE_UINT8(h8300_chan->ctrl_base+SCI_SCSMR, _smr);

    // Set baud rate.
    _smr &= ~CYGARC_REG_SCSMR_CKSx_MASK;
    _smr |= baud_divisor >> 8;
    HAL_WRITE_UINT8(h8300_chan->ctrl_base+SCI_SCSMR, _smr);
    HAL_WRITE_UINT8(h8300_chan->ctrl_base+SCI_SCBRR, baud_divisor & 0xff);

    // Clear the status register.
    HAL_WRITE_UINT8(h8300_chan->ctrl_base+SCI_SCSSR, 0);

    if (init) {
        // Always enable transmitter and receiver.
        _scr = CYGARC_REG_SCSCR_TE | CYGARC_REG_SCSCR_RE;

        if (chan->out_cbuf.len != 0)
            _scr |= CYGARC_REG_SCSCR_TIE; // enable tx interrupts

        if (chan->in_cbuf.len != 0)
            _scr |= CYGARC_REG_SCSCR_RIE; // enable rx interrupts
    }
     
    HAL_WRITE_UINT8(h8300_chan->ctrl_base+SCI_SCSCR, _scr);

    if (new_config != &chan->config) {
        chan->config = *new_config;
    }
    return true;
}

// Function to initialize the device.  Called at bootstrap time.
static bool 
h8300_serial_init(struct cyg_devtab_entry *tab)
{
    serial_channel *chan = (serial_channel *)tab->priv;
    h8300_sci_info *h8300_chan = (h8300_sci_info *)chan->dev_priv;
#ifdef CYGDBG_IO_INIT
    diag_printf("SH SERIAL init - dev: %x.%d\n", 
                h8300_chan->data, h8300_chan->rx_int_num);
#endif
    // Really only required for interrupt driven devices
    (chan->callbacks->serial_init)(chan);

    if (chan->out_cbuf.len != 0) {
        cyg_drv_interrupt_create(h8300_chan->tx_int_num,
                                 3,
                                 (cyg_addrword_t)chan, // Data item passed to interrupt handler
                                 h8300_serial_tx_ISR,
                                 h8300_serial_tx_DSR,
                                 &h8300_chan->serial_tx_interrupt_handle,
                                 &h8300_chan->serial_tx_interrupt);
        cyg_drv_interrupt_attach(h8300_chan->serial_tx_interrupt_handle);
        cyg_drv_interrupt_unmask(h8300_chan->tx_int_num);
        h8300_chan->tx_enabled = false;
    }
    if (chan->in_cbuf.len != 0) {
        // Receive interrupt
        cyg_drv_interrupt_create(h8300_chan->rx_int_num,
                                 3,
                                 (cyg_addrword_t)chan, // Data item passed to interrupt handler
                                 h8300_serial_rx_ISR,
                                 h8300_serial_rx_DSR,
                                 &h8300_chan->serial_rx_interrupt_handle,
                                 &h8300_chan->serial_rx_interrupt);
        cyg_drv_interrupt_attach(h8300_chan->serial_rx_interrupt_handle);
        // Receive error interrupt
        cyg_drv_interrupt_create(h8300_chan->er_int_num,
                                 3,
                                 (cyg_addrword_t)chan, // Data item passed to interrupt handler
                                 h8300_serial_er_ISR,
                                 h8300_serial_er_DSR,
                                 &h8300_chan->serial_er_interrupt_handle,
                                 &h8300_chan->serial_er_interrupt);
        cyg_drv_interrupt_attach(h8300_chan->serial_er_interrupt_handle);
        // This unmasks both interrupt sources.
        cyg_drv_interrupt_unmask(h8300_chan->rx_int_num);
    }
    h8300_serial_config_port(chan, &chan->config, true);
    return true;
}

// This routine is called when the device is "looked" up (i.e. attached)
static Cyg_ErrNo 
h8300_serial_lookup(struct cyg_devtab_entry **tab, 
                  struct cyg_devtab_entry *sub_tab,
                  const char *name)
{
    serial_channel *chan = (serial_channel *)(*tab)->priv;

    // Really only required for interrupt driven devices
    (chan->callbacks->serial_init)(chan);
    return ENOERR;
}

// Send a character to the device output buffer.
// Return 'true' if character is sent to device
static bool
h8300_serial_putc(serial_channel *chan, unsigned char c)
{
    cyg_uint8 _ssr;
    h8300_sci_info *h8300_chan = (h8300_sci_info *)chan->dev_priv;

    HAL_READ_UINT8(h8300_chan->ctrl_base+SCI_SCSSR, _ssr);
    if (_ssr & CYGARC_REG_SCSSR_TDRE) {
// Transmit buffer is empty
        HAL_WRITE_UINT8(h8300_chan->ctrl_base+SCI_SCTDR, c);
        // Clear empty flag.
        HAL_WRITE_UINT8(h8300_chan->ctrl_base+SCI_SCSSR, 
                        CYGARC_REG_SCSSR_CLEARMASK & ~CYGARC_REG_SCSSR_TDRE);
        return true;
    } else {
// No space
        return false;
    }
}

// Fetch a character from the device input buffer, waiting if necessary
static unsigned char 
h8300_serial_getc(serial_channel *chan)
{
    h8300_sci_info *h8300_chan = (h8300_sci_info *)chan->dev_priv;
    unsigned char c;
    cyg_uint8 _ssr;

    do {
        HAL_READ_UINT8(h8300_chan->ctrl_base+SCI_SCSSR, _ssr);
    } while ((_ssr & CYGARC_REG_SCSSR_RDRF) == 0);

    HAL_READ_UINT8(h8300_chan->ctrl_base+SCI_SCRDR, c);

    // Clear buffer full flag.
    HAL_WRITE_UINT8(h8300_chan->ctrl_base+SCI_SCSSR, 
                    CYGARC_REG_SCSSR_CLEARMASK & ~CYGARC_REG_SCSSR_RDRF);

    return c;
}

// Set up the device characteristics; baud rate, etc.
static Cyg_ErrNo
h8300_serial_set_config(serial_channel *chan, cyg_uint32 key,
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
        if ( true != h8300_serial_config_port(chan, config, false) )
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
h8300_serial_start_xmit(serial_channel *chan)
{
    cyg_uint8 _scr;
    h8300_sci_info *h8300_chan = (h8300_sci_info *)chan->dev_priv;

    h8300_chan->tx_enabled = true;

    // Mask the interrupts (all sources of the unit) while changing
    // the CR since a rx interrupt in the middle of this would result
    // in a bad CR state.
    cyg_drv_interrupt_mask(h8300_chan->rx_int_num);

    HAL_READ_UINT8(h8300_chan->ctrl_base+SCI_SCSCR, _scr);
    _scr |= CYGARC_REG_SCSCR_TIE;       // Enable xmit interrupt
    HAL_WRITE_UINT8(h8300_chan->ctrl_base+SCI_SCSCR, _scr);

    cyg_drv_interrupt_unmask(h8300_chan->rx_int_num);
}

// Disable the transmitter on the device
static void 
h8300_serial_stop_xmit(serial_channel *chan)
{
    cyg_uint8 _scr;
    h8300_sci_info *h8300_chan = (h8300_sci_info *)chan->dev_priv;

    h8300_chan->tx_enabled = false;

    // Mask the interrupts (all sources of the unit) while changing
    // the CR since a rx interrupt in the middle of this would result
    // in a bad CR state.
    cyg_drv_interrupt_mask(h8300_chan->rx_int_num);

    HAL_READ_UINT8(h8300_chan->ctrl_base+SCI_SCSCR, _scr);
    _scr &= ~CYGARC_REG_SCSCR_TIE;      // Disable xmit interrupt
    HAL_WRITE_UINT8(h8300_chan->ctrl_base+SCI_SCSCR, _scr);

    cyg_drv_interrupt_unmask(h8300_chan->rx_int_num);
}

// Serial I/O - low level tx interrupt handler (ISR)
static cyg_uint32 
h8300_serial_tx_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    h8300_sci_info *h8300_chan = (h8300_sci_info *)chan->dev_priv;
    cyg_uint8 _scr;

    HAL_READ_UINT8(h8300_chan->ctrl_base+SCI_SCSCR, _scr);
    _scr &= ~CYGARC_REG_SCSCR_TIE;      // mask out tx interrupts
    HAL_WRITE_UINT8(h8300_chan->ctrl_base+SCI_SCSCR, _scr);

    return CYG_ISR_CALL_DSR;  // Cause DSR to be run
}

// Serial I/O - high level tx interrupt handler (DSR)
static void       
h8300_serial_tx_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    h8300_sci_info *h8300_chan = (h8300_sci_info *)chan->dev_priv;

    (chan->callbacks->xmt_char)(chan);

    if (h8300_chan->tx_enabled) {
        cyg_uint8 _scr;

        HAL_READ_UINT8(h8300_chan->ctrl_base+SCI_SCSCR, _scr);
        _scr |= CYGARC_REG_SCSCR_TIE;       // unmask tx interrupts
        HAL_WRITE_UINT8(h8300_chan->ctrl_base+SCI_SCSCR, _scr);
    }
}

// Serial I/O - low level RX interrupt handler (ISR)
static cyg_uint32 
h8300_serial_rx_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    h8300_sci_info *h8300_chan = (h8300_sci_info *)chan->dev_priv;
    cyg_uint8 _scr;

    HAL_READ_UINT8(h8300_chan->ctrl_base+SCI_SCSCR, _scr);
    _scr &= ~CYGARC_REG_SCSCR_RIE;      // mask rx interrupts
    HAL_WRITE_UINT8(h8300_chan->ctrl_base+SCI_SCSCR, _scr);
    return CYG_ISR_CALL_DSR;  // Cause DSR to be run
}

// Serial I/O - high level rx interrupt handler (DSR)
static void       
h8300_serial_rx_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    h8300_sci_info *h8300_chan = (h8300_sci_info *)chan->dev_priv;
    cyg_uint8 _ssr, _scr;

    HAL_READ_UINT8(h8300_chan->ctrl_base+SCI_SCSSR, _ssr);
    if (_ssr & CYGARC_REG_SCSSR_RDRF) {
        cyg_uint8 _c;
        HAL_READ_UINT8(h8300_chan->ctrl_base+SCI_SCRDR, _c);
        // Clear buffer full flag.
        HAL_WRITE_UINT8(h8300_chan->ctrl_base+SCI_SCSSR, 
                        CYGARC_REG_SCSSR_CLEARMASK & ~CYGARC_REG_SCSSR_RDRF);

        (chan->callbacks->rcv_char)(chan, _c);
    }

    HAL_READ_UINT8(h8300_chan->ctrl_base+SCI_SCSCR, _scr);
    _scr |= CYGARC_REG_SCSCR_RIE;       // unmask rx interrupts
    HAL_WRITE_UINT8(h8300_chan->ctrl_base+SCI_SCSCR, _scr);
}

static volatile int h8300_serial_error_orer = 0;
static volatile int h8300_serial_error_fer = 0;
static volatile int h8300_serial_error_per = 0;

// Serial I/O - low level error interrupt handler (ISR)
static cyg_uint32 
h8300_serial_er_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    h8300_sci_info *h8300_chan = (h8300_sci_info *)chan->dev_priv;
    cyg_uint8 _scr;

    HAL_READ_UINT8(h8300_chan->ctrl_base+SCI_SCSCR, _scr);
    _scr &= ~CYGARC_REG_SCSCR_RIE;      // mask rx interrupts
    HAL_WRITE_UINT8(h8300_chan->ctrl_base+SCI_SCSCR, _scr);
    return CYG_ISR_CALL_DSR;            // Cause DSR to be run
}

// Serial I/O - high level error interrupt handler (DSR)
static void       
h8300_serial_er_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    h8300_sci_info *h8300_chan = (h8300_sci_info *)chan->dev_priv;
    cyg_uint8 _ssr, _ssr2, _scr;

    HAL_READ_UINT8(h8300_chan->ctrl_base+SCI_SCSSR, _ssr);
    _ssr2 = CYGARC_REG_SCSSR_CLEARMASK;

    if (_ssr & CYGARC_REG_SCSSR_ORER) {
        _ssr2 &= ~CYGARC_REG_SCSSR_ORER;
        h8300_serial_error_orer++;
    }
    if (_ssr & CYGARC_REG_SCSSR_FER) {
        _ssr2 &= ~CYGARC_REG_SCSSR_FER;
        h8300_serial_error_fer++;
    }
    if (_ssr & CYGARC_REG_SCSSR_PER) {
        _ssr2 &= ~CYGARC_REG_SCSSR_PER;
        h8300_serial_error_per++;
    }
    HAL_WRITE_UINT8(h8300_chan->ctrl_base+SCI_SCSSR, _ssr2);

    HAL_READ_UINT8(h8300_chan->ctrl_base+SCI_SCSCR, _scr);
    _scr |= CYGARC_REG_SCSCR_RIE;       // unmask rx interrupts
    HAL_WRITE_UINT8(h8300_chan->ctrl_base+SCI_SCSCR, _scr);
}

#endif // ifdef CYGDAT_IO_SERIAL_H8300_SCI
