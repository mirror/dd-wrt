//==========================================================================
//
//      io/serial/arm/edb7xxx_serial.c
//
//      Cirrus Logic EDB7XXX Serial I/O Interface Module (interrupt driven)
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
// Purpose:     EDB7XXX Serial I/O module (interrupt driven version)
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

#ifdef CYGPKG_IO_SERIAL_ARM_EDB7XXX

#include "edb7xxx_serial.h"

typedef struct edb7xxx_serial_info {
    CYG_ADDRWORD   data,                      // Pointer to data register
                   control,                   // Pointer to baud rate/line control register
                   stat,                      // Pointer to system flags for this port
                   syscon;                    // Pointer to system control for this port
    CYG_WORD       tx_int_num,                // Transmit interrupt number
                   rx_int_num,                // Receive interrupt number
                   ms_int_num;                // Modem Status Change interrupt number
    cyg_interrupt  serial_tx_interrupt, 
                   serial_rx_interrupt, 
                   serial_ms_interrupt;
    cyg_handle_t   serial_tx_interrupt_handle, 
                   serial_rx_interrupt_handle, 
                   serial_ms_interrupt_handle;
    bool           tx_enabled;
} edb7xxx_serial_info;

static bool edb7xxx_serial_init(struct cyg_devtab_entry *tab);
static bool edb7xxx_serial_putc(serial_channel *chan, unsigned char c);
static Cyg_ErrNo edb7xxx_serial_lookup(struct cyg_devtab_entry **tab, 
                                   struct cyg_devtab_entry *sub_tab,
                                   const char *name);
static unsigned char edb7xxx_serial_getc(serial_channel *chan);
static Cyg_ErrNo edb7xxx_serial_set_config(serial_channel *chan, cyg_uint32 key,
                                           const void *xbuf, cyg_uint32 *len);
static void edb7xxx_serial_start_xmit(serial_channel *chan);
static void edb7xxx_serial_stop_xmit(serial_channel *chan);

static cyg_uint32 edb7xxx_serial_tx_ISR(cyg_vector_t vector, cyg_addrword_t data);
static void       edb7xxx_serial_tx_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data);
static cyg_uint32 edb7xxx_serial_rx_ISR(cyg_vector_t vector, cyg_addrword_t data);
static void       edb7xxx_serial_rx_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data);
static cyg_uint32 edb7xxx_serial_ms_ISR(cyg_vector_t vector, cyg_addrword_t data);
static void       edb7xxx_serial_ms_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data);

static SERIAL_FUNS(edb7xxx_serial_funs, 
                   edb7xxx_serial_putc, 
                   edb7xxx_serial_getc,
                   edb7xxx_serial_set_config,
                   edb7xxx_serial_start_xmit,
                   edb7xxx_serial_stop_xmit
    );

#ifdef CYGPKG_IO_SERIAL_ARM_EDB7XXX_SERIAL1
static edb7xxx_serial_info edb7xxx_serial_info1 = {UARTDR1, // Data register
                                                 UBLCR1,  // Port control
                                                 SYSFLG1, // Status
                                                 SYSCON1, // System config
                                                 CYGNUM_HAL_INTERRUPT_UTXINT1, // Tx interrupt
                                                 CYGNUM_HAL_INTERRUPT_URXINT1, // Rx interrupt
                                                 0 /*CYGNUM_HAL_INTERRUPT_UMSINT*/}; // Modem control
#if CYGNUM_IO_SERIAL_ARM_EDB7XXX_SERIAL1_BUFSIZE > 0
static unsigned char edb7xxx_serial_out_buf1[CYGNUM_IO_SERIAL_ARM_EDB7XXX_SERIAL1_BUFSIZE];
static unsigned char edb7xxx_serial_in_buf1[CYGNUM_IO_SERIAL_ARM_EDB7XXX_SERIAL1_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(edb7xxx_serial_channel1,
                                       edb7xxx_serial_funs, 
                                       edb7xxx_serial_info1,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_EDB7XXX_SERIAL1_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &edb7xxx_serial_out_buf1[0], sizeof(edb7xxx_serial_out_buf1),
                                       &edb7xxx_serial_in_buf1[0], sizeof(edb7xxx_serial_in_buf1)
    );
#else
static SERIAL_CHANNEL(edb7xxx_serial_channel1,
                      edb7xxx_serial_funs, 
                      edb7xxx_serial_info1,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_EDB7XXX_SERIAL1_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(edb7xxx_serial_io1, 
             CYGDAT_IO_SERIAL_ARM_EDB7XXX_SERIAL1_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             edb7xxx_serial_init, 
             edb7xxx_serial_lookup,     // Serial driver may need initializing
             &edb7xxx_serial_channel1
    );
#endif //  CYGPKG_IO_SERIAL_ARM_EDB7XXX_SERIAL2

#ifdef CYGPKG_IO_SERIAL_ARM_EDB7XXX_SERIAL2
static edb7xxx_serial_info edb7xxx_serial_info2 = {UARTDR2, // Data register
                                                 UBLCR2,  // Port control
                                                 SYSFLG2, // Status
                                                 SYSCON2, // System config
                                                 CYGNUM_HAL_INTERRUPT_UTXINT2, // Tx interrupt
                                                 CYGNUM_HAL_INTERRUPT_URXINT2, // Rx interrupt
                                                 0};      // No modem control
#if CYGNUM_IO_SERIAL_ARM_EDB7XXX_SERIAL2_BUFSIZE > 0
static unsigned char edb7xxx_serial_out_buf2[CYGNUM_IO_SERIAL_ARM_EDB7XXX_SERIAL2_BUFSIZE];
static unsigned char edb7xxx_serial_in_buf2[CYGNUM_IO_SERIAL_ARM_EDB7XXX_SERIAL2_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(edb7xxx_serial_channel2,
                                       edb7xxx_serial_funs, 
                                       edb7xxx_serial_info2,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_EDB7XXX_SERIAL2_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &edb7xxx_serial_out_buf2[0], sizeof(edb7xxx_serial_out_buf2),
                                       &edb7xxx_serial_in_buf2[0], sizeof(edb7xxx_serial_in_buf2)
    );
#else
static SERIAL_CHANNEL(edb7xxx_serial_channel2,
                      edb7xxx_serial_funs, 
                      edb7xxx_serial_info2,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_EDB7XXX_SERIAL2_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(edb7xxx_serial_io2, 
             CYGDAT_IO_SERIAL_ARM_EDB7XXX_SERIAL2_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             edb7xxx_serial_init, 
             edb7xxx_serial_lookup,     // Serial driver may need initializing
             &edb7xxx_serial_channel2
    );
#endif //  CYGPKG_IO_SERIAL_ARM_EDB7XXX_SERIAL2

// Internal function to actually configure the hardware to desired baud rate, etc.
static bool
edb7xxx_serial_config_port(serial_channel *chan, cyg_serial_info_t *new_config, bool init)
{
    edb7xxx_serial_info *edb7xxx_chan = (edb7xxx_serial_info *)chan->dev_priv;
    volatile cyg_uint32 *syscon = (volatile cyg_uint32 *)edb7xxx_chan->syscon;
    volatile cyg_uint32 *blcfg = (volatile cyg_uint32 *)edb7xxx_chan->control;
    unsigned int baud_divisor = select_baud[new_config->baud];
    cyg_uint32 _lcr;
    if (baud_divisor == 0) return false;
    // Disable port interrupts while changing hardware
    _lcr = select_word_length[new_config->word_length - CYGNUM_SERIAL_WORD_LENGTH_5] | 
        select_stop_bits[new_config->stop] |
        select_parity[new_config->parity] |
        UBLCR_FIFOEN | UART_BITRATE(baud_divisor);
#ifdef CYGDBG_IO_INIT
    diag_printf("Set CTL: %x = %x\n", blcfg, _lcr);
#endif
    *blcfg = _lcr;
    *syscon |= SYSCON1_UART1EN;
    if (new_config != &chan->config) {
        chan->config = *new_config;
    }
    return true;
}

// Function to initialize the device.  Called at bootstrap time.
static bool 
edb7xxx_serial_init(struct cyg_devtab_entry *tab)
{
    serial_channel *chan = (serial_channel *)tab->priv;
    edb7xxx_serial_info *edb7xxx_chan = (edb7xxx_serial_info *)chan->dev_priv;
#ifdef CYGDBG_IO_INIT
    diag_printf("EDB7XXX SERIAL init - dev: %x.%d\n", edb7xxx_chan->control, edb7xxx_chan->tx_int_num);
#endif
    (chan->callbacks->serial_init)(chan);  // Really only required for interrupt driven devices
    if (chan->out_cbuf.len != 0) {
        cyg_drv_interrupt_create(edb7xxx_chan->tx_int_num,
                                 99,                     // Priority - unused
                                 (cyg_addrword_t)chan,   //  Data item passed to interrupt handler
                                 edb7xxx_serial_tx_ISR,
                                 edb7xxx_serial_tx_DSR,
                                 &edb7xxx_chan->serial_tx_interrupt_handle,
                                 &edb7xxx_chan->serial_tx_interrupt);
        cyg_drv_interrupt_attach(edb7xxx_chan->serial_tx_interrupt_handle);
        cyg_drv_interrupt_mask(edb7xxx_chan->tx_int_num);
        edb7xxx_chan->tx_enabled = false;
    }
    if (chan->in_cbuf.len != 0) {
        cyg_drv_interrupt_create(edb7xxx_chan->rx_int_num,
                                 99,                     // Priority - unused
                                 (cyg_addrword_t)chan,   //  Data item passed to interrupt handler
                                 edb7xxx_serial_rx_ISR,
                                 edb7xxx_serial_rx_DSR,
                                 &edb7xxx_chan->serial_rx_interrupt_handle,
                                 &edb7xxx_chan->serial_rx_interrupt);
        cyg_drv_interrupt_attach(edb7xxx_chan->serial_rx_interrupt_handle);
        cyg_drv_interrupt_unmask(edb7xxx_chan->rx_int_num);
    }
    edb7xxx_serial_config_port(chan, &chan->config, true);
    return true;
}

// This routine is called when the device is "looked" up (i.e. attached)
static Cyg_ErrNo 
edb7xxx_serial_lookup(struct cyg_devtab_entry **tab, 
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
edb7xxx_serial_putc(serial_channel *chan, unsigned char c)
{
    edb7xxx_serial_info *edb7xxx_chan = (edb7xxx_serial_info *)chan->dev_priv;
    volatile cyg_uint8  *data = (volatile cyg_uint8  *)edb7xxx_chan->data;
    volatile cyg_uint32 *stat = (volatile cyg_uint32 *)edb7xxx_chan->stat;
    if ((*stat & SYSFLG1_UTXFF1) == 0) {
// Transmit buffer/FIFO is not full
        *data = c;
        return true;
    } else {
// No space
        return false;
    }
}

// Fetch a character from the device input buffer, waiting if necessary
static unsigned char 
edb7xxx_serial_getc(serial_channel *chan)
{
    unsigned char c;
    edb7xxx_serial_info *edb7xxx_chan = (edb7xxx_serial_info *)chan->dev_priv;
    volatile cyg_uint32 *data = (volatile cyg_uint32 *)edb7xxx_chan->data;
    volatile cyg_uint32 *stat = (volatile cyg_uint32 *)edb7xxx_chan->stat;
    while (*stat & SYSFLG1_URXFE1) ; // Wait for char
    c = *data;
    return c;
}

// Set up the device characteristics; baud rate, etc.
static Cyg_ErrNo
edb7xxx_serial_set_config(serial_channel *chan, cyg_uint32 key,
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
        if ( true != edb7xxx_serial_config_port(chan, config, false) )
            return -EINVAL;
      }
      break;
    default:
        return -EINVAL;
    }
    return ENOERR;
}

// Enable the transmitter (interrupt) on the device
static void
edb7xxx_serial_start_xmit(serial_channel *chan)
{
    edb7xxx_serial_info *edb7xxx_chan = (edb7xxx_serial_info *)chan->dev_priv;
    edb7xxx_chan->tx_enabled = true;
    cyg_drv_interrupt_unmask(edb7xxx_chan->tx_int_num);
}

// Disable the transmitter on the device
static void 
edb7xxx_serial_stop_xmit(serial_channel *chan)
{
    edb7xxx_serial_info *edb7xxx_chan = (edb7xxx_serial_info *)chan->dev_priv;
    cyg_drv_interrupt_mask(edb7xxx_chan->tx_int_num);
    edb7xxx_chan->tx_enabled = false;
}

// Serial I/O - low level Tx interrupt handler (ISR)
static cyg_uint32 
edb7xxx_serial_tx_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    edb7xxx_serial_info *edb7xxx_chan = (edb7xxx_serial_info *)chan->dev_priv;
    cyg_drv_interrupt_mask(edb7xxx_chan->tx_int_num);
    cyg_drv_interrupt_acknowledge(edb7xxx_chan->tx_int_num);
    return CYG_ISR_CALL_DSR;  // Cause DSR to be run
}

// Serial I/O - high level Tx interrupt handler (DSR)
static void       
edb7xxx_serial_tx_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    edb7xxx_serial_info *edb7xxx_chan = (edb7xxx_serial_info *)chan->dev_priv;
    (chan->callbacks->xmt_char)(chan);
    if (edb7xxx_chan->tx_enabled) {
        cyg_drv_interrupt_unmask(edb7xxx_chan->tx_int_num);
    }
}

// Serial I/O - low level Rx interrupt handler (ISR)
static cyg_uint32 
edb7xxx_serial_rx_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    edb7xxx_serial_info *edb7xxx_chan = (edb7xxx_serial_info *)chan->dev_priv;
    cyg_drv_interrupt_mask(edb7xxx_chan->rx_int_num);
    cyg_drv_interrupt_acknowledge(edb7xxx_chan->rx_int_num);
    return CYG_ISR_CALL_DSR;  // Cause DSR to be run
}

// Serial I/O - high level Rx interrupt handler (DSR)
static void       
edb7xxx_serial_rx_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    edb7xxx_serial_info *edb7xxx_chan = (edb7xxx_serial_info *)chan->dev_priv;
    volatile cyg_uint32 *datreg = (volatile cyg_uint32 *)edb7xxx_chan->data;
    volatile cyg_uint32 *stat = (volatile cyg_uint32 *)edb7xxx_chan->stat;
    while (!(*stat & SYSFLG1_URXFE1))
        (chan->callbacks->rcv_char)(chan, *datreg);
    cyg_drv_interrupt_unmask(edb7xxx_chan->rx_int_num);
}

// Serial I/O - low level Ms interrupt handler (ISR)
static cyg_uint32 
edb7xxx_serial_ms_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    edb7xxx_serial_info *edb7xxx_chan = (edb7xxx_serial_info *)chan->dev_priv;
    cyg_drv_interrupt_mask(edb7xxx_chan->ms_int_num);
    cyg_drv_interrupt_acknowledge(edb7xxx_chan->ms_int_num);
    return CYG_ISR_CALL_DSR;  // Cause DSR to be run
}

// Serial I/O - high level Ms interrupt handler (DSR)
static void       
edb7xxx_serial_ms_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
}
#endif // CYGPKG_IO_SERIAL_ARM_EDB7XXX

