//==========================================================================
//
//      tx3904_serial.c
//
//      Serial device driver for TX3904 on-chip serial devices
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
// Author(s):   nickg
// Contributors: nickg
// Date:        1999-03-3
// Purpose:     TX3904 serial device driver
// Description: TX3904 serial device driver
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/io_serial.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_intr.h>

#include <cyg/io/io.h>
#include <cyg/io/devtab.h>
#include <cyg/io/serial.h>

#ifdef CYGPKG_IO_SERIAL_TX39_JMR3904

cyg_bool cyg_hal_is_break(char *buf, int size);
void cyg_hal_user_break( CYG_ADDRWORD *regs );

//-------------------------------------------------------------------------

extern void diag_printf(const char *fmt, ...);

//-------------------------------------------------------------------------
// Forward definitions

static bool tx3904_serial_init(struct cyg_devtab_entry *tab);
static bool tx3904_serial_putc(serial_channel *chan, unsigned char c);
static Cyg_ErrNo tx3904_serial_lookup(struct cyg_devtab_entry **tab, 
                                   struct cyg_devtab_entry *sub_tab,
                                   const char *name);
static unsigned char tx3904_serial_getc(serial_channel *chan);
static Cyg_ErrNo tx3904_serial_set_config(serial_channel *chan, cyg_uint32 key,
                                          const void *xbuf, cyg_uint32 *len);
static void tx3904_serial_start_xmit(serial_channel *chan);
static void tx3904_serial_stop_xmit(serial_channel *chan);

#ifndef CYGPKG_IO_SERIAL_TX39_JMR3904_POLLED_MODE
static cyg_uint32 tx3904_serial_ISR(cyg_vector_t vector, cyg_addrword_t data, cyg_addrword_t *regs);
static void       tx3904_serial_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data);
#endif


//-------------------------------------------------------------------------
// TX3904 serial line control register values:

// Offsets to serial control registers from base
#define SERIAL_CR       0x00
#define SERIAL_SR       0x04
#define SERIAL_ICR      0x08
#define SERIAL_ISR      0x0C
#define SERIAL_FCR      0x10
#define SERIAL_BRG      0x14
#define SERIAL_TXB      0x20
#define SERIAL_RXB      0x30

// Status register bits
#define ISR_RXRDY       0x01
#define ISR_TXRDY       0x02
#define ISR_ERROR       0x04

// Control register bits
#define LCR_SB1         0x0000
#define LCR_SB1_5       0x0000
#define LCR_SB2         0x0004
#define LCR_PN          0x0000    // Parity mode - none
#define LCR_PS          0x0000    // Forced "space" parity
#define LCR_PM          0x0000    // Forced "mark" parity
#define LCR_PE          0x0018    // Parity mode - even
#define LCR_PO          0x0010    // Parity mode - odd
#define LCR_WL5         0x0001    // not supported - use 7bit
#define LCR_WL6         0x0001    // not supported - use 7bit
#define LCR_WL7         0x0001    // 7 bit chars
#define LCR_WL8         0x0000    // 8 bit chars

#define LCR_BRG         0x0020    // Select baud rate generator

#define ICR_RXE         0x0001  // receive enable
#define ICR_TXE         0x0002  // transmit enable

//-------------------------------------------------------------------------
// Tables to map input values to hardware settings

static unsigned char select_word_length[] = {
    LCR_WL5,    // 5 bits / word (char)
    LCR_WL6,
    LCR_WL7,
    LCR_WL8
};

static unsigned char select_stop_bits[] = {
    0,
    LCR_SB1,    // 1 stop bit
    LCR_SB1_5,  // 1.5 stop bit
    LCR_SB2     // 2 stop bits
};

static unsigned char select_parity[] = {
    LCR_PN,     // No parity
    LCR_PE,     // Even parity
    LCR_PO,     // Odd parity
    LCR_PM,     // Mark parity
    LCR_PS,     // Space parity
};

// The values in this table plug straight into the BRG register
// in the serial driver hardware. They comprise a baud rate divisor
// in the bottom 8 bits and a clock selector in the top 8 bits.
// These figures all come from Toshiba.

#if (CYGHWR_HAL_MIPS_CPU_FREQ == 50)

static unsigned short select_baud[] = {
    0,          // Unused
    0,          // 50
    0,          // 75
    0,          // 110
    0,          // 134.5
    0,          // 150
    0,          // 200
    0,          // 300
    0x0300|20,  // 600
    0x0300|10,  // 1200
    0,          // 1800
    0x0300|05,  // 2400
    0,          // 3600
    0x0300|10,  // 4800
    0,          // 7200
    0x0200|05,  // 9600
    0,          // 14400
    0x0100|10,  // 19200
    0x0100|05,  // 38400
    0,          // 57600
    0,          // 115200
    0,          // 230400
};

#elif (CYGHWR_HAL_MIPS_CPU_FREQ == 66)

static unsigned short select_baud[] = {
    0,          // Unused
    0,          // 50
    0,          // 75
    0,          // 110
    0,          // 134.5
    0,          // 150
    0,          // 200
    0,          // 300
    0x0300|27,  // 600
    0x0200|54,  // 1200
    0,          // 1800
    0x0200|27,  // 2400
    0,          // 3600
    0x0100|54,  // 4800
    0,          // 7200
    0x0100|27,  // 9600
    0,          // 14400
    0x0000|54,  // 19200
    0x0000|27,  // 38400
    0,          // 57600
    0,          // 115200
    0,          // 230400
};

#else

#error Unsupported CPU frequency

#endif

//-------------------------------------------------------------------------
// Info for each serial device controlled

typedef struct tx3904_serial_info {
    CYG_ADDRWORD   base;
    CYG_WORD       int_num;
    cyg_interrupt  interrupt;
    cyg_handle_t   interrupt_handle;
    cyg_uint8      input_char;
    cyg_bool       input_char_valid;
    cyg_bool       output_ready;
    cyg_uint16     cur_baud;
} tx3904_serial_info;

//-------------------------------------------------------------------------
// Callback functions exported by this driver

static SERIAL_FUNS(tx3904_serial_funs, 
                   tx3904_serial_putc, 
                   tx3904_serial_getc,
                   tx3904_serial_set_config,
                   tx3904_serial_start_xmit,
                   tx3904_serial_stop_xmit
    );

//-------------------------------------------------------------------------
// Hardware info for each serial line

#ifdef CYGPKG_IO_SERIAL_TX39_JMR3904_SERIAL0
static tx3904_serial_info tx3904_serial_info0 = {
    0xFFFFF300,
    CYGNUM_HAL_INTERRUPT_SIO_0
};
#if CYGNUM_IO_SERIAL_TX39_JMR3904_SERIAL0_BUFSIZE > 0
static unsigned char tx3904_serial_out_buf0[CYGNUM_IO_SERIAL_TX39_JMR3904_SERIAL0_BUFSIZE];
static unsigned char tx3904_serial_in_buf0[CYGNUM_IO_SERIAL_TX39_JMR3904_SERIAL0_BUFSIZE];
#endif
#endif // CYGPKG_IO_SERIAL_TX39_JMR3904_SERIAL0

#ifdef CYGPKG_IO_SERIAL_TX39_JMR3904_SERIAL1
static tx3904_serial_info tx3904_serial_info1 = {
    0xFFFFF400,
    CYGNUM_HAL_INTERRUPT_SIO_1
};
#if CYGNUM_IO_SERIAL_TX39_JMR3904_SERIAL1_BUFSIZE > 0
static unsigned char tx3904_serial_out_buf1[CYGNUM_IO_SERIAL_TX39_JMR3904_SERIAL1_BUFSIZE];
static unsigned char tx3904_serial_in_buf1[CYGNUM_IO_SERIAL_TX39_JMR3904_SERIAL1_BUFSIZE];
#endif
#endif // CYGPKG_IO_SERIAL_TX39_JMR3904_SERIAL1

//-------------------------------------------------------------------------
// Channel descriptions:

#ifdef CYGPKG_IO_SERIAL_TX39_JMR3904_POLLED_MODE
#define SIZEOF_BUF(_x_) 0
#else
#define SIZEOF_BUF(_x_) sizeof(_x_)
#endif

#ifdef CYGPKG_IO_SERIAL_TX39_JMR3904_SERIAL0
#if CYGNUM_IO_SERIAL_TX39_JMR3904_SERIAL0_BUFSIZE > 0
static SERIAL_CHANNEL_USING_INTERRUPTS(tx3904_serial_channel0,
                                       tx3904_serial_funs, 
                                       tx3904_serial_info0,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_TX39_JMR3904_SERIAL0_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &tx3904_serial_out_buf0[0],
                                       SIZEOF_BUF(tx3904_serial_out_buf0),
                                       &tx3904_serial_in_buf0[0],
                                       SIZEOF_BUF(tx3904_serial_in_buf0)
    );
#else
static SERIAL_CHANNEL(tx3904_serial_channel0,
                      tx3904_serial_funs, 
                      tx3904_serial_info0,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_TX39_JMR3904_SERIAL0_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif
#endif // CYGPKG_IO_SERIAL_TX39_JMR3904_SERIAL0

#ifdef CYGPKG_IO_SERIAL_TX39_JMR3904_SERIAL1
#if CYGNUM_IO_SERIAL_TX39_JMR3904_SERIAL1_BUFSIZE > 0
static SERIAL_CHANNEL_USING_INTERRUPTS(tx3904_serial_channel1,
                                       tx3904_serial_funs, 
                                       tx3904_serial_info1,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_TX39_JMR3904_SERIAL1_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &tx3904_serial_out_buf1[0],
                                       SIZEOF_BUF(tx3904_serial_out_buf1),
                                       &tx3904_serial_in_buf1[0],
                                       SIZEOF_BUF(tx3904_serial_in_buf1)
    );
#else
static SERIAL_CHANNEL(tx3904_serial_channel1,
                      tx3904_serial_funs, 
                      tx3904_serial_info1,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_TX39_JMR3904_SERIAL1_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif
#endif // CYGPKG_IO_SERIAL_TX39_JMR3904_SERIAL1

//-------------------------------------------------------------------------
// And finally, the device table entries:

#ifdef CYGPKG_IO_SERIAL_TX39_JMR3904_SERIAL0
DEVTAB_ENTRY(tx3904_serial_io0, 
             CYGDAT_IO_SERIAL_TX39_JMR3904_SERIAL0_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             tx3904_serial_init, 
             tx3904_serial_lookup,     // Serial driver may need initializing
             &tx3904_serial_channel0
    );
#endif // CYGPKG_IO_SERIAL_TX39_JMR3904_SERIAL0

#ifdef CYGPKG_IO_SERIAL_TX39_JMR3904_SERIAL1
DEVTAB_ENTRY(tx3904_serial_io1, 
             CYGDAT_IO_SERIAL_TX39_JMR3904_SERIAL1_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             tx3904_serial_init, 
             tx3904_serial_lookup,     // Serial driver may need initializing
             &tx3904_serial_channel1
    );
#endif // CYGPKG_IO_SERIAL_TX39_JMR3904_SERIAL1

// ------------------------------------------------------------------------
// Delay for some number of character times. This is based on the baud
// rate currently set. We use the numbers that plug in to the BRG
// clock select and divider to control two loops. The innermost delay
// loop uses a count that is derived from dividing the CPU frequency
// by the BRG granularity (and we then add 1 to compensate for any
// rounding).  This gives the number of cycles that the innermost loop
// must consume.  For the sake of simplicity we assume that this loop
// will take 1 cycle per loop, which is roughly true in optimized
// code.

void delay_char_time(tx3904_serial_info *tx3904_chan, int n)
{
    static cyg_uint16 clock_val[4] = { 4, 16, 64, 256 };
    cyg_uint16 baud_val = select_baud[tx3904_chan->cur_baud];
    cyg_count32 clock_loop = clock_val[baud_val>>8];
    cyg_count32 div_loop = baud_val & 0xFF;
    cyg_count32 bit_time = ((CYGHWR_HAL_MIPS_CPU_FREQ_ACTUAL)/(2457600)) + 1;
    
    n *= 11;    // allow for start and stop bits and 8 data bits
    
    while( n-- )
    {
        cyg_count32 i,j,k;

        for( i = 0; i < clock_loop; i++ )
            for( j = 0; j < div_loop; j++ )
                for( k = 0; k < bit_time; k++ )
                    continue;
    }
}

//-------------------------------------------------------------------------

static bool
tx3904_serial_config_port(serial_channel *chan, cyg_serial_info_t *new_config, bool init)
{
    tx3904_serial_info *tx3904_chan = (tx3904_serial_info *)chan->dev_priv;
    cyg_uint16 cr = 0;
    cyg_uint16 icr = 0;
    cyg_uint16 baud_divisor = select_baud[new_config->baud];

    if (baud_divisor == 0)
        return false;  // Invalid baud rate selected

    // set up other config values:

    cr |= select_word_length[new_config->word_length - CYGNUM_SERIAL_WORD_LENGTH_5];
    cr |= select_stop_bits[new_config->stop];
    cr |= select_parity[new_config->parity];

    // Source transfer clock from BRG
    cr |= LCR_BRG;

#ifndef CYGPKG_IO_SERIAL_TX39_JMR3904_POLLED_MODE    
    // Enable RX interrupts only at present
#ifdef CYGPKG_IO_SERIAL_TX39_JMR3904_SERIAL0
    if ((chan->out_cbuf.len != 0) || (chan == &tx3904_serial_channel0)) {
#else
    if (chan->out_cbuf.len != 0) {
#endif
        icr |= ICR_RXE;
    }
#endif

    // Avoid any interrupts while we are fiddling with the line parameters.
    cyg_drv_interrupt_mask(tx3904_chan->int_num);

    
    // In theory we should wait here for the transmitter to drain the
    // FIFO so we dont change the line parameters with characters
    // unsent. Unfortunately the TX39 serial devices do not allow us
    // to discover when the FIFO is empty.

    delay_char_time(tx3904_chan, 8);
    
    // Disable device entirely.
//    HAL_WRITE_UINT16(tx3904_chan->base+SERIAL_CR, 0);
//    HAL_WRITE_UINT8(tx3904_chan->base+SERIAL_ICR, 0);

    // Reset the FIFOs

    HAL_WRITE_UINT16(tx3904_chan->base+SERIAL_FCR, 7);
    HAL_WRITE_UINT16(tx3904_chan->base+SERIAL_FCR, 0);
    
    // Set up baud rate

    HAL_WRITE_UINT16( tx3904_chan->base+SERIAL_BRG, baud_divisor );

    // Write CR into hardware
    HAL_WRITE_UINT16(tx3904_chan->base+SERIAL_CR, cr);    

    // Write ICR into hardware
    HAL_WRITE_UINT16(tx3904_chan->base+SERIAL_ICR, icr);    

    // Re-enable interrupts.
    cyg_drv_interrupt_unmask(tx3904_chan->int_num);

    // Save current baud rate
    tx3904_chan->cur_baud = new_config->baud;

    if (new_config != &chan->config) {
        chan->config = *new_config;
    }
    return true;
}

//-------------------------------------------------------------------------
// Function to initialize the device.  Called at bootstrap time.

bool tx3904_serial_init(struct cyg_devtab_entry *tab)
{
    serial_channel *chan = (serial_channel *)tab->priv;
    tx3904_serial_info *tx3904_chan = (tx3904_serial_info *)chan->dev_priv;

    (chan->callbacks->serial_init)(chan);  // Really only required for interrupt driven devices

    tx3904_chan->cur_baud = CYGNUM_SERIAL_BAUD_38400;
        
#ifndef CYGPKG_IO_SERIAL_TX39_JMR3904_POLLED_MODE    
    if (chan->out_cbuf.len != 0) {
        // Install and enable the interrupt
        cyg_drv_interrupt_create(tx3904_chan->int_num,
                                 4,                      // Priority
                                 (cyg_addrword_t)chan,   //  Data item passed to interrupt handler
                                 (cyg_ISR_t *)tx3904_serial_ISR,
                                 tx3904_serial_DSR,
                                 &tx3904_chan->interrupt_handle,
                                 &tx3904_chan->interrupt);
        cyg_drv_interrupt_attach(tx3904_chan->interrupt_handle);
        cyg_drv_interrupt_unmask(tx3904_chan->int_num);
    }
#endif
    
    tx3904_serial_config_port(chan, &chan->config, true);
    
    return true;
}

//-------------------------------------------------------------------------
// This routine is called when the device is "looked" up (i.e. attached)

static Cyg_ErrNo 
tx3904_serial_lookup(struct cyg_devtab_entry **tab, 
                  struct cyg_devtab_entry *sub_tab,
                  const char *name)
{
    serial_channel *chan = (serial_channel *)(*tab)->priv;
    (chan->callbacks->serial_init)(chan);  // Really only required for interrupt driven devices
    return ENOERR;
}

//-------------------------------------------------------------------------
// Return 'true' if character is sent to device

bool
tx3904_serial_putc(serial_channel *chan, unsigned char c)
{
    tx3904_serial_info *tx3904_chan = (tx3904_serial_info *)chan->dev_priv;
    cyg_uint16 isr;

    HAL_READ_UINT16( tx3904_chan->base+SERIAL_ISR, isr );

    if( isr & ISR_TXRDY )
    {
        HAL_WRITE_UINT8( tx3904_chan->base+SERIAL_TXB, c );

        isr &= ~ISR_TXRDY;
        
        HAL_WRITE_UINT16( tx3904_chan->base+SERIAL_ISR, isr );

        return true;
    }
    else return false;
}

//-------------------------------------------------------------------------

unsigned char 
tx3904_serial_getc(serial_channel *chan)
{
    unsigned char c;
    tx3904_serial_info *tx3904_chan = (tx3904_serial_info *)chan->dev_priv;
    cyg_uint16 isr;

    do
    {
        HAL_READ_UINT16( tx3904_chan->base+SERIAL_ISR, isr );

        // Eliminate any RX errors
        if( isr & ISR_ERROR )
        {
            cyg_uint16 sr = 0;
            
            isr &= ISR_ERROR;

//            HAL_READ_UINT16( tx3904_chan->base+SERIAL_SR, sr );

            HAL_WRITE_UINT16( tx3904_chan->base+SERIAL_SR, sr );            
            HAL_WRITE_UINT16( tx3904_chan->base+SERIAL_ISR, isr );
        }
        
    } while( (isr & ISR_RXRDY) != ISR_RXRDY );
    
    HAL_READ_UINT8( tx3904_chan->base+SERIAL_RXB, c );

    isr &= ~ISR_RXRDY;
        
    HAL_WRITE_UINT16( tx3904_chan->base+SERIAL_ISR, isr );

    return c;
}

//-------------------------------------------------------------------------

static Cyg_ErrNo
tx3904_serial_set_config(serial_channel *chan, cyg_uint32 key,
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
        if ( true != tx3904_serial_config_port(chan, config, false) )
            return -EINVAL;
      }
      break;
    default:
        return -EINVAL;
    }
    return ENOERR;
}

//-------------------------------------------------------------------------
// Enable the transmitter on the device

static void
tx3904_serial_start_xmit(serial_channel *chan)
{
#ifndef CYGPKG_IO_SERIAL_TX39_JMR3904_POLLED_MODE    
    tx3904_serial_info *tx3904_chan = (tx3904_serial_info *)chan->dev_priv;
    cyg_uint16 icr;

    HAL_READ_UINT16( tx3904_chan->base+SERIAL_ICR, icr );

    icr |= ICR_TXE;

    HAL_WRITE_UINT16( tx3904_chan->base+SERIAL_ICR, icr );
#endif    
}

//-------------------------------------------------------------------------
// Disable the transmitter on the device

static void 
tx3904_serial_stop_xmit(serial_channel *chan)
{
#ifndef CYGPKG_IO_SERIAL_TX39_JMR3904_POLLED_MODE    
    tx3904_serial_info *tx3904_chan = (tx3904_serial_info *)chan->dev_priv;
    cyg_uint16 icr;

    HAL_READ_UINT16( tx3904_chan->base+SERIAL_ICR, icr );

    icr &= ~ICR_TXE;

    HAL_WRITE_UINT16( tx3904_chan->base+SERIAL_ICR, icr );
#endif    
}

//-------------------------------------------------------------------------
// Serial I/O - low level interrupt handlers (ISR)

#ifndef CYGPKG_IO_SERIAL_TX39_JMR3904_POLLED_MODE

static cyg_uint32 
tx3904_serial_ISR(cyg_vector_t vector, cyg_addrword_t data, cyg_addrword_t *regs)
{
    serial_channel *chan = (serial_channel *)data;
    tx3904_serial_info *tx3904_chan = (tx3904_serial_info *)chan->dev_priv;
    cyg_uint8 isr;
    cyg_uint32 result = 0;
    
    cyg_drv_interrupt_mask(tx3904_chan->int_num);
    cyg_drv_interrupt_acknowledge(tx3904_chan->int_num);

    HAL_READ_UINT16( tx3904_chan->base+SERIAL_ISR, isr );

    // Eliminate any RX errors
    if( isr & ISR_ERROR )
    {
        cyg_uint16 sr = 0;
            
        isr &= ~ISR_ERROR;

        HAL_READ_UINT16( tx3904_chan->base+SERIAL_SR, sr );
        
        HAL_WRITE_UINT16( tx3904_chan->base+SERIAL_SR, 0 );            
    }

    // Check for a TX interrupt and set the flag if so.
    if( isr & ISR_TXRDY )
    {
        isr &= ~ISR_TXRDY;

        tx3904_chan->output_ready = true;

        result |= CYG_ISR_CALL_DSR;  // Cause DSR to be run
    }
    
    
    // Check here for an RX interrupt and fetch the character. If it
    // is a ^C then call into GDB stub to handle it.
    
    if( isr & ISR_RXRDY )
    {
        cyg_uint8 rxb;
        HAL_READ_UINT8( tx3904_chan->base+SERIAL_RXB, rxb );

        isr &= ~ISR_RXRDY;

        if( cyg_hal_is_break( &rxb , 1 ) )
            cyg_hal_user_break( regs );
        else
        {
            tx3904_chan->input_char = rxb;
            tx3904_chan->input_char_valid = true;
            result |= CYG_ISR_CALL_DSR;  // Cause DSR to be run
        }

    }

    HAL_WRITE_UINT16( tx3904_chan->base+SERIAL_ISR, isr );
    
    return result;
}


#endif

//-------------------------------------------------------------------------
// Serial I/O - high level interrupt handler (DSR)

#ifndef CYGPKG_IO_SERIAL_TX39_JMR3904_POLLED_MODE

static void       
tx3904_serial_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    tx3904_serial_info *tx3904_chan = (tx3904_serial_info *)chan->dev_priv;
    cyg_uint8 isr;

    HAL_READ_UINT16( tx3904_chan->base+SERIAL_ISR, isr );

    if( tx3904_chan->input_char_valid )
    {
        (chan->callbacks->rcv_char)(chan, tx3904_chan->input_char);

        tx3904_chan->input_char_valid = false;

#if 0        
        // And while we are here, pull any further characters out of the
        // FIFO. This should help to reduce the interrupt rate.

        HAL_READ_UINT16( tx3904_chan->base+SERIAL_ISR, isr );
    
        while( isr & ISR_RXRDY )
        {
            cyg_uint8 rxb;
            HAL_READ_UINT8( tx3904_chan->base+SERIAL_RXB, rxb );

            (chan->callbacks->rcv_char)(chan, rxb);
            
            isr &= ~ISR_RXRDY;
        
            HAL_WRITE_UINT16( tx3904_chan->base+SERIAL_ISR, isr );
            HAL_READ_UINT16( tx3904_chan->base+SERIAL_ISR, isr );        
        }
#endif
        
    }

    if( tx3904_chan->output_ready )
    {
        (chan->callbacks->xmt_char)(chan);

        tx3904_chan->output_ready = false;
    }
    
    cyg_drv_interrupt_unmask(tx3904_chan->int_num);
}

#endif
#endif // CYGPKG_IO_SERIAL_TX39_JMR3904

//-------------------------------------------------------------------------
// EOF tx3904_serial.c
