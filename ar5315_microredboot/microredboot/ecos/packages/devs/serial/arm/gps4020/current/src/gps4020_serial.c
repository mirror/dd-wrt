//==========================================================================
//
//      io/serial/arm/gps4020_serial.c
//
//      GPS4020 Serial I/O Interface Module
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Gary Thomas
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
// Purpose:     GPS4020 Serial I/O module
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
#include <cyg/hal/hal_if.h>

#include <cyg/hal/gps4020.h>  // Hardware definitions

static short select_word_length[] = {
    -1,            // 5 bits / word (char)
    -1,            // 6 bits / word
    SMR_LENGTH_7,  // 7 bits/word
    SMR_LENGTH_8   // 8 bits/word
};

static short select_stop_bits[] = {
    -1,
    SMR_STOP_1,     // 1 stop bit
    -1,             // 1.5 stop bit
    SMR_STOP_2      // 2 stop bits
};

static short select_parity[] = {
    SMR_PARITY_OFF,                // No parity
    SMR_PARITY_ON|SMR_PARITY_EVEN, // Even parity
    SMR_PARITY_ON|SMR_PARITY_ODD,  // Odd parity
    -1,                            // Mark parity
    -1,                            // Space parity
};

// Baud rate values, based on internal system (20MHz) clock
// Note: the extra *10 stuff is for rounding.  Since these values
// are so small, a little error here can make/break the calculation
#define BAUD_DIVISOR(baud) (((((20000000*10)/(16*baud))+5)/10)-1)
static cyg_int32 select_baud[] = {
    0,                    // Unused
    BAUD_DIVISOR(50),     // 50
    BAUD_DIVISOR(75),     // 75
    BAUD_DIVISOR(110),    // 110
    0,                    // 134.5
    BAUD_DIVISOR(150),    // 150
    BAUD_DIVISOR(200),    // 200
    BAUD_DIVISOR(300),    // 300
    BAUD_DIVISOR(600),    // 600
    BAUD_DIVISOR(1200),   // 1200
    BAUD_DIVISOR(1800),   // 1800
    BAUD_DIVISOR(2400),   // 2400
    BAUD_DIVISOR(3600),   // 3600
    BAUD_DIVISOR(4800),   // 4800
    BAUD_DIVISOR(7200),   // 7200
    BAUD_DIVISOR(9600),   // 9600
    BAUD_DIVISOR(14400),  // 14400
    BAUD_DIVISOR(19200),  // 19200
    BAUD_DIVISOR(38400),  // 38400
    BAUD_DIVISOR(57600),  // 57600
    BAUD_DIVISOR(115200), // 115200
    BAUD_DIVISOR(230400), // 230400
};

typedef struct gps4020_serial_info {
    CYG_ADDRWORD   regs;                      // Pointer to UART registers
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
} gps4020_serial_info;

static bool gps4020_serial_init(struct cyg_devtab_entry *tab);
static bool gps4020_serial_putc(serial_channel *chan, unsigned char c);
static Cyg_ErrNo gps4020_serial_lookup(struct cyg_devtab_entry **tab, 
                                   struct cyg_devtab_entry *sub_tab,
                                   const char *name);
static unsigned char gps4020_serial_getc(serial_channel *chan);
static Cyg_ErrNo gps4020_serial_set_config(serial_channel *chan, cyg_uint32 key,
                                           const void *xbuf, cyg_uint32 *len);
static void gps4020_serial_start_xmit(serial_channel *chan);
static void gps4020_serial_stop_xmit(serial_channel *chan);

static cyg_uint32 gps4020_serial_tx_ISR(cyg_vector_t vector, cyg_addrword_t data);
static void       gps4020_serial_tx_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data);
static cyg_uint32 gps4020_serial_rx_ISR(cyg_vector_t vector, cyg_addrword_t data);
static void       gps4020_serial_rx_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data);
#if 0 // FIXME - handle modem & errors
static cyg_uint32 gps4020_serial_ms_ISR(cyg_vector_t vector, cyg_addrword_t data);
static void       gps4020_serial_ms_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data);
#endif

static SERIAL_FUNS(gps4020_serial_funs, 
                   gps4020_serial_putc, 
                   gps4020_serial_getc,
                   gps4020_serial_set_config,
                   gps4020_serial_start_xmit,
                   gps4020_serial_stop_xmit
    );

#ifdef CYGPKG_IO_SERIAL_ARM_GPS4020_SERIAL1
static gps4020_serial_info gps4020_serial_info1 = {GPS4020_UART1, // Data register
                                                   CYGNUM_HAL_INTERRUPT_UART1_TX,  // Tx interrupt
                                                   CYGNUM_HAL_INTERRUPT_UART1_RX,  // Rx interrupt
                                                   CYGNUM_HAL_INTERRUPT_UART1_ERR, // Modem & Error interrupt
                                                  };
#if CYGNUM_IO_SERIAL_ARM_GPS4020_SERIAL1_BUFSIZE > 0
static unsigned char gps4020_serial_out_buf1[CYGNUM_IO_SERIAL_ARM_GPS4020_SERIAL1_BUFSIZE];
static unsigned char gps4020_serial_in_buf1[CYGNUM_IO_SERIAL_ARM_GPS4020_SERIAL1_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(gps4020_serial_channel1,
                                       gps4020_serial_funs, 
                                       gps4020_serial_info1,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_GPS4020_SERIAL1_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &gps4020_serial_out_buf1[0], sizeof(gps4020_serial_out_buf1),
                                       &gps4020_serial_in_buf1[0], sizeof(gps4020_serial_in_buf1)
    );
#else
static SERIAL_CHANNEL(gps4020_serial_channel1,
                      gps4020_serial_funs, 
                      gps4020_serial_info1,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_GPS4020_SERIAL1_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(gps4020_serial_io1, 
             CYGDAT_IO_SERIAL_ARM_GPS4020_SERIAL1_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             gps4020_serial_init, 
             gps4020_serial_lookup,     // Serial driver may need initializing
             &gps4020_serial_channel1
    );
#endif //  CYGPKG_IO_SERIAL_ARM_GPS4020_SERIAL2

#ifdef CYGPKG_IO_SERIAL_ARM_GPS4020_SERIAL2
static gps4020_serial_info gps4020_serial_info2 = {GPS4020_UART2, // Data register
                                                   CYGNUM_HAL_INTERRUPT_UART2_TX,  // Tx interrupt
                                                   CYGNUM_HAL_INTERRUPT_UART2_RX,  // Rx interrupt
                                                   CYGNUM_HAL_INTERRUPT_UART2_ERR, // Modem & Error interrupt
                                                  };
#if CYGNUM_IO_SERIAL_ARM_GPS4020_SERIAL2_BUFSIZE > 0
static unsigned char gps4020_serial_out_buf2[CYGNUM_IO_SERIAL_ARM_GPS4020_SERIAL2_BUFSIZE];
static unsigned char gps4020_serial_in_buf2[CYGNUM_IO_SERIAL_ARM_GPS4020_SERIAL2_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(gps4020_serial_channel2,
                                       gps4020_serial_funs, 
                                       gps4020_serial_info2,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_GPS4020_SERIAL2_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &gps4020_serial_out_buf2[0], sizeof(gps4020_serial_out_buf2),
                                       &gps4020_serial_in_buf2[0], sizeof(gps4020_serial_in_buf2)
    );
#else
static SERIAL_CHANNEL(gps4020_serial_channel2,
                      gps4020_serial_funs, 
                      gps4020_serial_info2,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_GPS4020_SERIAL2_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(gps4020_serial_io2, 
             CYGDAT_IO_SERIAL_ARM_GPS4020_SERIAL2_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             gps4020_serial_init, 
             gps4020_serial_lookup,     // Serial driver may need initializing
             &gps4020_serial_channel2
    );
#endif //  CYGPKG_IO_SERIAL_ARM_GPS4020_SERIAL2

// Internal function to actually configure the hardware to desired baud rate, etc.
static bool
gps4020_serial_config_port(serial_channel *chan, cyg_serial_info_t *new_config, bool init)
{
    gps4020_serial_info *gps4020_chan = (gps4020_serial_info *)chan->dev_priv;
    volatile struct _gps4020_uart *regs = (volatile struct _gps4020_uart *)gps4020_chan->regs;
    unsigned int baud_divisor = select_baud[new_config->baud];
    short word_len = select_word_length[new_config->word_length - CYGNUM_SERIAL_WORD_LENGTH_5];
    short stop_bits = select_stop_bits[new_config->stop];
    short parity =   select_parity[new_config->parity];
    short mode = word_len | stop_bits | parity;
    int prescale = 0;

    if (mode >= 0) {
        while (baud_divisor > 0xFF) {
            prescale++;
            baud_divisor >>= 1;
        }
#ifdef CYGDBG_IO_INIT
        diag_printf("I/O MODE: %x, BAUD: %x\n", mode, baud_divisor);
        CYGACC_CALL_IF_DELAY_US((cyg_int32)2*100000);
#endif
        regs->mode = mode | SMR_DIV(prescale);
        regs->baud = baud_divisor;
        regs->modem_control = SMR_DTR | SMR_RTS;
        regs->control = SCR_TEN | SCR_REN | SCR_TIE | SCR_RIE;
        if (new_config != &chan->config) {
            chan->config = *new_config;
        }
        return true;
    } else {
        return false;
    }
}

// Function to initialize the device.  Called at bootstrap time.
static bool 
gps4020_serial_init(struct cyg_devtab_entry *tab)
{
    serial_channel *chan = (serial_channel *)tab->priv;
    gps4020_serial_info *gps4020_chan = (gps4020_serial_info *)chan->dev_priv;
#ifdef CYGDBG_IO_INIT
    diag_printf("GPS4020 SERIAL init - dev: %x.%d\n", gps4020_chan->regs, gps4020_chan->tx_int_num);
#endif
    (chan->callbacks->serial_init)(chan);  // Really only required for interrupt driven devices
    if (chan->out_cbuf.len != 0) {
        cyg_drv_interrupt_create(gps4020_chan->tx_int_num,
                                 99,                     // Priority - unused
                                 (cyg_addrword_t)chan,   //  Data item passed to interrupt handler
                                 gps4020_serial_tx_ISR,
                                 gps4020_serial_tx_DSR,
                                 &gps4020_chan->serial_tx_interrupt_handle,
                                 &gps4020_chan->serial_tx_interrupt);
        cyg_drv_interrupt_attach(gps4020_chan->serial_tx_interrupt_handle);
        cyg_drv_interrupt_mask(gps4020_chan->tx_int_num);
        gps4020_chan->tx_enabled = false;
    }
    if (chan->in_cbuf.len != 0) {
        cyg_drv_interrupt_create(gps4020_chan->rx_int_num,
                                 99,                     // Priority - unused
                                 (cyg_addrword_t)chan,   //  Data item passed to interrupt handler
                                 gps4020_serial_rx_ISR,
                                 gps4020_serial_rx_DSR,
                                 &gps4020_chan->serial_rx_interrupt_handle,
                                 &gps4020_chan->serial_rx_interrupt);
        cyg_drv_interrupt_attach(gps4020_chan->serial_rx_interrupt_handle);
        cyg_drv_interrupt_unmask(gps4020_chan->rx_int_num);
    }
    gps4020_serial_config_port(chan, &chan->config, true);
    return true;
}

// This routine is called when the device is "looked" up (i.e. attached)
static Cyg_ErrNo 
gps4020_serial_lookup(struct cyg_devtab_entry **tab, 
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
gps4020_serial_putc(serial_channel *chan, unsigned char c)
{
    gps4020_serial_info *gps4020_chan = (gps4020_serial_info *)chan->dev_priv;
    volatile struct _gps4020_uart *regs = (volatile struct _gps4020_uart *)gps4020_chan->regs;

    if ((regs->status & SSR_TxEmpty) != 0) {
        // Transmit buffer/FIFO is not full
        regs->TxRx = c;
        return true;
    } else {
        // No space
        return false;
    }
}

// Fetch a character from the device input buffer, waiting if necessary
static unsigned char 
gps4020_serial_getc(serial_channel *chan)
{
    unsigned char c;
    gps4020_serial_info *gps4020_chan = (gps4020_serial_info *)chan->dev_priv;
    volatile struct _gps4020_uart *regs = (volatile struct _gps4020_uart *)gps4020_chan->regs;

    while ((regs->status & SSR_RxFull) == 0) ; // Wait for character
    c = regs->TxRx;
    return c;
}

// Set up the device characteristics; baud rate, etc.
static Cyg_ErrNo
gps4020_serial_set_config(serial_channel *chan, cyg_uint32 key,
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
        if ( true != gps4020_serial_config_port(chan, config, false) )
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
gps4020_serial_start_xmit(serial_channel *chan)
{
    gps4020_serial_info *gps4020_chan = (gps4020_serial_info *)chan->dev_priv;
    gps4020_chan->tx_enabled = true;
    cyg_drv_interrupt_unmask(gps4020_chan->tx_int_num);
}

// Disable the transmitter on the device
static void 
gps4020_serial_stop_xmit(serial_channel *chan)
{
    gps4020_serial_info *gps4020_chan = (gps4020_serial_info *)chan->dev_priv;
    cyg_drv_interrupt_mask(gps4020_chan->tx_int_num);
    gps4020_chan->tx_enabled = false;
}

// Serial I/O - low level Tx interrupt handler (ISR)
static cyg_uint32 
gps4020_serial_tx_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    gps4020_serial_info *gps4020_chan = (gps4020_serial_info *)chan->dev_priv;
    cyg_drv_interrupt_mask(gps4020_chan->tx_int_num);
    cyg_drv_interrupt_acknowledge(gps4020_chan->tx_int_num);
    return CYG_ISR_CALL_DSR;  // Cause DSR to be run
}

// Serial I/O - high level Tx interrupt handler (DSR)
static void       
gps4020_serial_tx_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    gps4020_serial_info *gps4020_chan = (gps4020_serial_info *)chan->dev_priv;
    (chan->callbacks->xmt_char)(chan);
    if (gps4020_chan->tx_enabled) {
        cyg_drv_interrupt_unmask(gps4020_chan->tx_int_num);
    }
}

// Serial I/O - low level Rx interrupt handler (ISR)
static cyg_uint32 
gps4020_serial_rx_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    gps4020_serial_info *gps4020_chan = (gps4020_serial_info *)chan->dev_priv;
    cyg_drv_interrupt_mask(gps4020_chan->rx_int_num);
    cyg_drv_interrupt_acknowledge(gps4020_chan->rx_int_num);
    return CYG_ISR_CALL_DSR;  // Cause DSR to be run
}

// Serial I/O - high level Rx interrupt handler (DSR)
static void       
gps4020_serial_rx_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    gps4020_serial_info *gps4020_chan = (gps4020_serial_info *)chan->dev_priv;
    volatile struct _gps4020_uart *regs = (volatile struct _gps4020_uart *)gps4020_chan->regs;

    while ((regs->status & SSR_RxFull) != 0) 
        (chan->callbacks->rcv_char)(chan, regs->TxRx);
    cyg_drv_interrupt_unmask(gps4020_chan->rx_int_num);
}

#if 0 // FIXME - handle modem & errors
// Serial I/O - low level Ms interrupt handler (ISR)
static cyg_uint32 
gps4020_serial_ms_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    gps4020_serial_info *gps4020_chan = (gps4020_serial_info *)chan->dev_priv;
    cyg_drv_interrupt_mask(gps4020_chan->ms_int_num);
    cyg_drv_interrupt_acknowledge(gps4020_chan->ms_int_num);
    return CYG_ISR_CALL_DSR;  // Cause DSR to be run
}

// Serial I/O - high level Ms interrupt handler (DSR)
static void       
gps4020_serial_ms_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
}
#endif

