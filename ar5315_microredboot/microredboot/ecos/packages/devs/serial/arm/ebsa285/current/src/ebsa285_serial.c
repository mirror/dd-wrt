//==========================================================================
//
//      devs/serial/arm/ebsa285/current/src/ebsa285_serial.c
//
//      ARM EBSA285 Serial I/O Interface Module (interrupt driven)
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
// Author(s):   hmt
// Contributors:  hmt
// Date:        1999-07-26
// Purpose:     EBSA285 Serial I/O module (interrupt driven version)
// Description: 
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/io_serial.h>
#include <pkgconf/io.h>

#ifdef CYGPKG_IO_SERIAL_ARM_EBSA285

#include <cyg/io/io.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/io/devtab.h>
#include <cyg/io/serial.h>
#include <cyg/infra/diag.h>

#include <cyg/hal/hal_ebsa285.h>         // Hardware definitions

// ------------------------------------------------------------------------
// Baud rates and the like, table-driven setup
#define FCLK_MHZ 50

struct _baud {
    unsigned char divisor_high, divisor_low;
};

// The indexing of this table must match the enum in serialio.h
// The arithmetic is (clock/4)/(baud * 16) - 1

#define NONE {0,0}
const static struct _baud bauds[] = {
#if (FCLK_MHZ == 50)
    NONE,                  // unused
    NONE,                  // 50
    NONE,                  // 75
    NONE,                  // 110
    NONE,                  // 134.5
    NONE,                  // 150
    NONE,                  // 200
    { 0xA, 0x2B },         // 300   2603  = 0x0A2B
    { 0x5, 0x15 },         // 600   1301  = 0x0515
    { 0x2, 0x8A },         // 1200  650   = 0x028A
    { 0x1, 0xB1 },         // 1800  433   = 0x01B1
    { 0x1, 0x45 },         // 2400  325   = 0x0145
    { 0x0, 0xD8 },         // 3600  216   = 0x00D8
    { 0x0, 0xA2 },         // 4800  162   = 0x00A2
    { 0x0, 0x6B },         // 7200  107   = 0x006B
    { 0x0, 0x50 },         // 9600  80    = 0x0050
    { 0x0, 0x35 },         // 14400 53    = 0x0035
    { 0x0, 0x28 },         // 19200 40    = 0x0028
    { 0x0, 0x13 },         // 38400 19    = 0x0013
    NONE,                  // 57600 
    NONE,                  // 115200
    NONE                   // 230400
#elif (FCLK_MHZ == 60)
#error NOT SUPPORTED - these figures are more for documentation
    { /*   300, */ 0xC, 0x34},                  /* 2603  = 0x0A2B */
    { /*   600, */ 0x6, 0x19},                  /* 1301  = 0x0515 */
    { /*  1200, */ 0x3, 0x0C},                  /* 650   = 0x028A */
    { /*  2400, */ 0x1, 0x86},                  /* 325   = 0x0145 */
    { /*  4800, */ 0x0, 0xC2},                  /* 162   = 0x00A2 */
    { /*  9600, */ 0x0, 0x61},                  /* 80    = 0x0050 */
    { /* 19200, */ 0x0, 0x30},                  /* 40    = 0x0028 */
    { /* 38400, */ 0x0, 0x17},                  /* 19    = 0x0013 */
#endif
};

static int select_word_length[] = {
    SA110_UART_DATA_LENGTH_5_BITS,      // 5 bits
    SA110_UART_DATA_LENGTH_6_BITS,      // 6 bits
    SA110_UART_DATA_LENGTH_7_BITS,      // 7 bits
    SA110_UART_DATA_LENGTH_8_BITS       // 8 bits
};

static int select_stop_bits[] = {
    -1,                          // unused
    SA110_UART_STOP_BITS_ONE,    // 1 stop bit
    -1,                          // 1.5 stop bit
    SA110_UART_STOP_BITS_TWO     // 2 stop bits
};

static int select_parity[] = {
    SA110_UART_PARITY_DISABLED,                           // No parity
    SA110_UART_PARITY_ENABLED | SA110_UART_PARITY_EVEN,   // Even parity
    SA110_UART_PARITY_ENABLED | SA110_UART_PARITY_ODD,    // Odd parity
    -1,                                                   // Mark parity
    -1                                                    // Space parity
};

// ------------------------------------------------------------------------
// some forward references

struct ebsa285_serial_interrupt {
    CYG_WORD       int_num;
    cyg_interrupt  serial_interrupt;
    cyg_handle_t   serial_interrupt_handle;
};

typedef struct ebsa285_serial_info {
    struct ebsa285_serial_interrupt rx;
    struct ebsa285_serial_interrupt tx;
    int tx_active;
} ebsa285_serial_info;

static bool ebsa285_serial_init(struct cyg_devtab_entry *tab);
static bool ebsa285_serial_putc(serial_channel *chan, unsigned char c);
static Cyg_ErrNo ebsa285_serial_lookup(struct cyg_devtab_entry **tab, 
                                   struct cyg_devtab_entry *sub_tab,
                                   const char *name);
static unsigned char ebsa285_serial_getc(serial_channel *chan);
static Cyg_ErrNo ebsa285_serial_set_config(serial_channel *chan, cyg_uint32 key,
                                           const void *xbuf, cyg_uint32 *len);
                                           
static void ebsa285_serial_start_xmit(serial_channel *chan);
static void ebsa285_serial_stop_xmit(serial_channel *chan);

static cyg_uint32 ebsa285_serial_rx_ISR(cyg_vector_t vector, cyg_addrword_t data);
static void       ebsa285_serial_rx_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data);
static cyg_uint32 ebsa285_serial_tx_ISR(cyg_vector_t vector, cyg_addrword_t data);
static void       ebsa285_serial_tx_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data);

static SERIAL_FUNS(ebsa285_serial_funs, 
                   ebsa285_serial_putc, 
                   ebsa285_serial_getc,
                   ebsa285_serial_set_config,
                   ebsa285_serial_start_xmit,
                   ebsa285_serial_stop_xmit
    );


// ------------------------------------------------------------------------
// this is dummy in config: there is only one device on the EBSA285
#ifdef CYGPKG_IO_SERIAL_ARM_EBSA285_SERIAL

static ebsa285_serial_info ebsa285_serial_info1 = {
    { CYGNUM_HAL_INTERRUPT_SERIAL_RX },
    { CYGNUM_HAL_INTERRUPT_SERIAL_TX },
    0
};

#if CYGNUM_IO_SERIAL_ARM_EBSA285_SERIAL_BUFSIZE > 0
static unsigned char ebsa285_serial_out_buf[CYGNUM_IO_SERIAL_ARM_EBSA285_SERIAL_BUFSIZE];
static unsigned char ebsa285_serial_in_buf[CYGNUM_IO_SERIAL_ARM_EBSA285_SERIAL_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(ebsa285_serial_channel,
                                       ebsa285_serial_funs, 
                                       ebsa285_serial_info1,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_EBSA285_SERIAL_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &ebsa285_serial_out_buf[0], sizeof(ebsa285_serial_out_buf),
                                       &ebsa285_serial_in_buf[0], sizeof(ebsa285_serial_in_buf)
    );
#else
static SERIAL_CHANNEL(ebsa285_serial_channel,
                      ebsa285_serial_funs, 
                      ebsa285_serial_info1,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_EBSA285_SERIAL_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(ebsa285_serial_io, 
             CYGDAT_IO_SERIAL_ARM_EBSA285_SERIAL_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             ebsa285_serial_init, 
             ebsa285_serial_lookup,     // Serial driver may need initializing
             &ebsa285_serial_channel
    );
#endif //  CYGPKG_IO_SERIAL_ARM_EBSA285_SERIAL

// ------------------------------------------------------------------------


// ------------------------------------------------------------------------
// Internal function to actually configure the hardware to desired baud rate, etc.
static bool
ebsa285_serial_config_port(serial_channel *chan, cyg_serial_info_t *new_config, bool init)
{
    int dummy, h, m, l;
    
    // Make sure everything is off
    *SA110_UART_CONTROL_REGISTER = SA110_UART_DISABLED | SA110_SIR_DISABLED;
    
    // Read the RXStat to drain the fifo
    dummy = *SA110_UART_RXSTAT;

    // Set the baud rate - this also turns the uart on.
    // 
    // Note that the ordering of these writes is critical,
    // and the writes to the H_BAUD_CONTROL and CONTROL_REGISTER
    // are necessary to force the UART to update its register
    // contents.

    l = bauds[new_config->baud].divisor_low;   // zeros in unused slots here
    m = bauds[new_config->baud].divisor_high;  // and here
    h = SA110_UART_BREAK_DISABLED    |      
        select_stop_bits[new_config->stop] |   // -1s in unused slots for these
        select_parity[new_config->parity]  |   // and these
        SA110_UART_FIFO_ENABLED |              // and these below
        select_word_length[new_config->word_length - CYGNUM_SERIAL_WORD_LENGTH_5];

    if ( 0 != (l + m) && h >= 0 && h < 256 ) {
        *SA110_UART_L_BAUD_CONTROL = l;
        *SA110_UART_M_BAUD_CONTROL = m;
        *SA110_UART_H_BAUD_CONTROL = h;
        init = true; // AOK
    }
    else if ( init ) {
        // put in some sensible defaults
        *SA110_UART_L_BAUD_CONTROL   = 0x13; // bp->divisor_low;
        *SA110_UART_M_BAUD_CONTROL   = 0x00; // bp->divisor_high;
        *SA110_UART_H_BAUD_CONTROL = SA110_UART_BREAK_DISABLED    |
            SA110_UART_PARITY_DISABLED   |
            SA110_UART_STOP_BITS_ONE     |
            SA110_UART_FIFO_ENABLED      |
            SA110_UART_DATA_LENGTH_8_BITS;
    }

    // All set, re-enable the device:
    *SA110_UART_CONTROL_REGISTER = SA110_UART_ENABLED | SA110_SIR_DISABLED;

    if (init && new_config != &chan->config) {
        // record the new setup
        chan->config = *new_config;
    }
    // All done
    return init;
}

// Function to initialize the device.  Called at bootstrap time.
static bool 
ebsa285_serial_init(struct cyg_devtab_entry *tab)
{
    serial_channel *chan = (serial_channel *)tab->priv;
    ebsa285_serial_info *ebsa285_chan = (ebsa285_serial_info *)chan->dev_priv;
#ifdef CYGDBG_IO_INIT
    diag_printf("EBSA285 SERIAL init - dev: %x\n", ebsa285_chan);
#endif
    (chan->callbacks->serial_init)(chan);  // Really only required for interrupt driven devices
    if (chan->out_cbuf.len != 0) {

        // first for rx
        cyg_drv_interrupt_create(ebsa285_chan->rx.int_num,
                                 99,                   // Priority - unused
                                 (cyg_addrword_t)chan, //  Data item passed to interrupt handler
                                 ebsa285_serial_rx_ISR,
                                 ebsa285_serial_rx_DSR,
                                 &ebsa285_chan->rx.serial_interrupt_handle,
                                 &ebsa285_chan->rx.serial_interrupt);
        cyg_drv_interrupt_attach(ebsa285_chan->rx.serial_interrupt_handle);
        cyg_drv_interrupt_unmask(ebsa285_chan->rx.int_num);

        // then for tx
        cyg_drv_interrupt_create(ebsa285_chan->tx.int_num,
                                 99,                   // Priority - unused
                                 (cyg_addrword_t)chan, //  Data item passed to interrupt handler
                                 ebsa285_serial_tx_ISR,
                                 ebsa285_serial_tx_DSR,
                                 &ebsa285_chan->tx.serial_interrupt_handle,
                                 &ebsa285_chan->tx.serial_interrupt);
        cyg_drv_interrupt_attach(ebsa285_chan->tx.serial_interrupt_handle);
        // DO NOT cyg_drv_interrupt_unmask(ebsa285_chan->tx.int_num);
        ebsa285_chan->tx_active = 0;
    }
    (void)ebsa285_serial_config_port(chan, &chan->config, true);
    return true;
}

// This routine is called when the device is "looked" up (i.e. attached)
static Cyg_ErrNo 
ebsa285_serial_lookup(struct cyg_devtab_entry **tab, 
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
ebsa285_serial_putc(serial_channel *chan, unsigned char c)
{
    if ((*SA110_UART_FLAG_REGISTER & SA110_TX_FIFO_STATUS_MASK) == SA110_TX_FIFO_BUSY)
        return false; // No space
    
    *SA110_UART_DATA_REGISTER = c; // Transmit buffer is empty
    return true;
}

// Fetch a character from the device input buffer, waiting if necessary
static unsigned char 
ebsa285_serial_getc(serial_channel *chan)
{
    unsigned char c;
    while ((*SA110_UART_FLAG_REGISTER & SA110_RX_FIFO_STATUS_MASK) == SA110_RX_FIFO_EMPTY)
        ; // wait for char
    c = (char)(*SA110_UART_DATA_REGISTER & 0xFF);
    // no error checking... no way to return the info
    return c;
}

// Set up the device characteristics; baud rate, etc.
static Cyg_ErrNo
ebsa285_serial_set_config(serial_channel *chan, cyg_uint32 key,
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
        if ( true != ebsa285_serial_config_port(chan, config, false) )
            return -EINVAL;
      }
      break;
    default:
        return -EINVAL;
    }
    return ENOERR;
}

// Enable the transmitter on the device (nope, already in use by hal_diag)
static void
ebsa285_serial_start_xmit(serial_channel *chan)
{
    ebsa285_serial_info *ebsa285_chan = (ebsa285_serial_info *)chan->dev_priv;
    ebsa285_chan->tx_active = 1;
    cyg_drv_interrupt_unmask(ebsa285_chan->tx.int_num);
}

// Disable the transmitter on the device (nope, remains in use by hal_diag)
static void 
ebsa285_serial_stop_xmit(serial_channel *chan)
{
    ebsa285_serial_info *ebsa285_chan = (ebsa285_serial_info *)chan->dev_priv;
    cyg_drv_interrupt_mask(ebsa285_chan->tx.int_num);
    ebsa285_chan->tx_active = 0;
}

// Serial I/O - low level interrupt handlers (ISR)
static cyg_uint32 
ebsa285_serial_rx_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    ebsa285_serial_info *ebsa285_chan = (ebsa285_serial_info *)chan->dev_priv;
    cyg_drv_interrupt_mask(ebsa285_chan->rx.int_num);
    cyg_drv_interrupt_acknowledge(ebsa285_chan->rx.int_num);
    return CYG_ISR_CALL_DSR;  // Cause DSR to be run
}

static cyg_uint32 
ebsa285_serial_tx_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    ebsa285_serial_info *ebsa285_chan = (ebsa285_serial_info *)chan->dev_priv;
    cyg_drv_interrupt_mask(ebsa285_chan->tx.int_num);
    cyg_drv_interrupt_acknowledge(ebsa285_chan->tx.int_num);
    return CYG_ISR_CALL_DSR;  // Cause DSR to be run
}

// Serial I/O - high level interrupt handlers (DSR)
static void       
ebsa285_serial_rx_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    ebsa285_serial_info *ebsa285_chan = (ebsa285_serial_info *)chan->dev_priv;
    if ((*SA110_UART_FLAG_REGISTER & SA110_RX_FIFO_STATUS_MASK) != SA110_RX_FIFO_EMPTY) {
        char c;
        int status;
        c = (char)(*SA110_UART_DATA_REGISTER & 0xFF);
        status = *SA110_UART_RXSTAT;
        if ( 0 == (status & (SA110_UART_FRAMING_ERROR_MASK |
                             SA110_UART_PARITY_ERROR_MASK  |
                             SA110_UART_OVERRUN_ERROR_MASK)) )
            (chan->callbacks->rcv_char)(chan, c);
    }
    cyg_drv_interrupt_unmask(ebsa285_chan->rx.int_num);
}

static void       
ebsa285_serial_tx_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    ebsa285_serial_info *ebsa285_chan = (ebsa285_serial_info *)chan->dev_priv;
    if ((*SA110_UART_FLAG_REGISTER & SA110_TX_FIFO_STATUS_MASK) != SA110_TX_FIFO_BUSY) {
        (chan->callbacks->xmt_char)(chan);
    }
    if ( ebsa285_chan->tx_active ) // it might be halted in callback above
        cyg_drv_interrupt_unmask(ebsa285_chan->tx.int_num);
}
#endif // CYGPKG_IO_SERIAL_ARM_EBSA285

// ------------------------------------------------------------------------
// EOF ebsa285_serial.c
