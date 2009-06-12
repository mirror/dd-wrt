//==========================================================================
//
//      mn10300_serial.c
//
//      Serial device driver for mn10300 on-chip serial devices
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
// Date:        1999-02-25
// Purpose:     MN10300 serial device driver
// Description: MN10300 serial device driver
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/io_serial.h>
#include <cyg/hal/hal_io.h>

#include <cyg/io/io.h>
#include <cyg/io/devtab.h>
#include <cyg/io/serial.h>
#include <cyg/hal/hal_intr.h>

#ifdef CYGPKG_IO_SERIAL_MN10300

#define CYG_HAL_MN10300_SERIAL_RX_FIFO

//-------------------------------------------------------------------------

extern void diag_printf(const char *fmt, ...);

//-------------------------------------------------------------------------
// Forward definitions

static bool mn10300_serial_init(struct cyg_devtab_entry *tab);
static bool mn10300_serial_putc(serial_channel *chan, unsigned char c);
static Cyg_ErrNo mn10300_serial_lookup(struct cyg_devtab_entry **tab, 
                                   struct cyg_devtab_entry *sub_tab,
                                   const char *name);
static unsigned char mn10300_serial_getc(serial_channel *chan);
static Cyg_ErrNo mn10300_serial_set_config(serial_channel *chan, cyg_uint32 key,
                                           const void *xbuf, cyg_uint32 *len);
static void mn10300_serial_start_xmit(serial_channel *chan);
static void mn10300_serial_stop_xmit(serial_channel *chan);

#ifndef CYGPKG_IO_SERIAL_MN10300_POLLED_MODE
static cyg_uint32 mn10300_serial_rx_ISR(cyg_vector_t vector, cyg_addrword_t data);
static cyg_uint32 mn10300_serial_tx_ISR(cyg_vector_t vector, cyg_addrword_t data);
static void       mn10300_serial_rx_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data);
static void       mn10300_serial_tx_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data);
#endif

//-------------------------------------------------------------------------

#define BUFSIZE 128

//-------------------------------------------------------------------------
// MN10300 serial line control register values:

// Offsets to serial control registers from base
#define SERIAL_CTR      0x0
#define SERIAL_ICR      0x4
#define SERIAL_TXB      0x8
#define SERIAL_RXB      0x9
#define SERIAL_STR      0xc
#define SERIAL_TIM      0xd

// Status register bits
#define SR_RBF          0x10
#define SR_TBF          0x20
#define SR_RXF          0x40
#define SR_TXF          0x80

// Control register bits
#define LCR_SB1         0x00
#define LCR_SB1_5       0x00
#define LCR_SB2         0x04
#define LCR_PN          0x00    // Parity mode - none
#define LCR_PS          0x40    // Forced "space" parity
#define LCR_PM          0x50    // Forced "mark" parity
#define LCR_PE          0x60    // Parity mode - even
#define LCR_PO          0x70    // Parity mode - odd
#define LCR_WL5         0x00    // not supported - use 7bit
#define LCR_WL6         0x00    // not supported - use 7bit
#define LCR_WL7         0x00    // 7 bit chars
#define LCR_WL8         0x80    // 8 bit chars
#define LCR_RXE         0x4000  // receive enable
#define LCR_TXE         0x8000  // transmit enable

#if defined(CYGPKG_HAL_MN10300_AM31)
#define LCR_TWE         0x0100  // interrupt enable (only on serial2/AM31)
#else
#define LCR_TWE         0x0000  // Bit does not exist in other variants
#endif

//-------------------------------------------------------------------------
// MN10300 timer registers:

#undef TIMER_BR
#undef TIMER_MD
#define TIMER_MD        0x00
#define TIMER_BR        0x10

//-------------------------------------------------------------------------
// Serial and timer base registers:

#if defined(CYGPKG_HAL_MN10300_AM31)

#define SERIAL0_BASE            0x34000800
#define SERIAL1_BASE            0x34000810
#define SERIAL2_BASE            0x34000820

#define TIMER0_BASE             0x34001000
#define TIMER1_BASE             0x34001001
#define TIMER2_BASE             0x34001002

#define SERIAL0_TIMER_SELECT    0x0004          // timer 0
#define SERIAL1_TIMER_SELECT    0x0004          // timer 1
#define SERIAL2_TIMER_SELECT    0x0001          // timer 2

#ifdef CYGPKG_HAL_MN10300_AM31_STDEVAL1
// The use of PORT3 to provide CTS/CTR is specific to
// the STDEVAL1 board only.
#define PORT3_MD                0x36008025
#endif

#define ENABLE_TRANSMIT_INTERRUPT(mn10300_chan) \
CYG_MACRO_START                                 \
    if( mn10300_chan->is_serial2 )              \
        cr |= LCR_TWE;                          \
    else                                        \
        cr |= LCR_TXE;                          \
CYG_MACRO_END

#define DISABLE_TRANSMIT_INTERRUPT(mn10300_chan)        \
CYG_MACRO_START                                         \
    if( mn10300_chan->is_serial2 )                      \
        cr &= ~LCR_TWE;                                 \
    else                                                \
        cr &= ~LCR_TXE;                                 \
CYG_MACRO_END

#elif defined(CYGPKG_HAL_MN10300_AM33)

#define SERIAL0_BASE            0xd4002000
#define SERIAL1_BASE            0xd4002010
#define SERIAL2_BASE            0xd4002020

#define TIMER0_BASE             0xd4003002
#define TIMER1_BASE             0xd4003001
#define TIMER2_BASE             0xd4003003

#define SERIAL0_TIMER_SELECT    0x0005          // timer 2
#define SERIAL1_TIMER_SELECT    0x0004          // timer 1
#define SERIAL2_TIMER_SELECT    0x0003          // timer 3

#define HW_TIMER0               0xd4003000

#define ENABLE_TRANSMIT_INTERRUPT(mn10300_chan)

#define DISABLE_TRANSMIT_INTERRUPT(mn10300_chan)

#else

#error Unsupported MN10300 variant

#endif

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

#if defined(CYGPKG_HAL_MN10300_AM31)

static unsigned short select_baud_01[] = {
    0,          // Unused
    0,          // 50
    0,          // 75
    0,          // 110
    0,          // 134.5
    0,          // 150
    0,          // 200
    0,          // 300
    0,          // 600
    0,          // 1200
    0,          // 1800
    0,          // 2400
    0,          // 3600
    0,          // 4800
    0,          // 7200
    195,        // 9600
    130,        // 14400
    98,         // 19200
    48,         // 38400
    32,         // 57600
    16,         // 115200
    8,          // 230400
};

// Serial 2 has its own timer register in addition to using timer 2 to
// supply the baud rate generator. Both of these must be proframmed to
// get the right baud rate. The following values come from Matsushita
// with some modifications from Cygmon.
static struct
{
    cyg_uint8   serial2_val;
    cyg_uint8   timer2_val;
} select_baud_2[] = {
    {   0,   0 },       // Unused
    {   0,   0 },       // 50
    {   0,   0 },       // 75
    {   0,   0 },       // 110
    {   0,   0 },       // 134.5
    {   0,   0 },       // 150
    {   0,   0 },       // 200
    {   0,   0 },       // 300
    { 126, 196 },       // 600
    { 125,  98 },       // 1200
    {   0,   0 },       // 1800
    { 124,  49 },       // 2400
    { 0,     0 },       // 3600
    { 124,  24 },       // 4800
    {   0,   0 },       // 7200
    {  70,  21 },       // 9600
    {   0,   0 },       // 14400
    {  70,  10 },       // 19200
    {  22,  16 },       // 38400
    {  88,   2 },       // 57600
    {  64,   1 },       // 115200
    {  62,   0 },       // 230400
};

#elif defined(CYGPKG_HAL_MN10300_AM33)

// The AM33 runs at a different clock rate and therefore has a
// different set of dividers for the baud rate.

static unsigned short select_baud_01[] = {
    0,          // Unused
    0,          // 50
    0,          // 75
    0,          // 110
    0,          // 134.5
    0,          // 150
    0,          // 200
    0,          // 300
    0,          // 600
    3168,       // 1200
    0,          // 1800
    1584,       // 2400
    0,          // 3600
    792,        // 4800
    0,          // 7200
    396,        // 9600
    0,          // 14400
    198,        // 19200
    99,         // 38400
     0,         // 57600
    33,         // 115200
    16,         // 230400
};

// Serial 2 has its own timer register in addition to using timer 2 to
// supply the baud rate generator. Both of these must be proframmed to
// get the right baud rate. The following values come from Matsushita
// with some modifications from Cygmon.

// The values in the following table differ significantly from those
// given in the Matsushita documentation. These have been determined
// by (somewhat exhaustive) experiment, the values in the documentation
// do not appear to work at all.

static struct
{
    cyg_uint8   serial2_val;
    cyg_uint8   timer2_val;
} select_baud_2[] = {
    {   0,   0 },       // Unused
    {   0,   0 },       // 50
    {   0,   0 },       // 75
    {   0,   0 },       // 110
    {   0,   0 },       // 134.5
    {   0,   0 },       // 150
    {   0,   0 },       // 200
    {   0,   0 },       // 300
    {   0,   0 },       // 600
    {   0,   0 },       // 1200
    {   0,   0 },       // 1800
    {   0,   0 },       // 2400
    {   0,   0 },       // 3600
    { 110,  56 },       // 4800
    {   0,   0 },       // 7200
    { 110,  28 },       // 9600
    {   0,   0 },       // 14400
    {  71,  21 },       // 19200
    { 102,   7 },       // 38400
    {   0,   0 },       // 57600
    {   9,  26 },       // 115200
    {   0,   0 },       // 230400
};

#else

#error Unsupported MN10300 variant

#endif

//-------------------------------------------------------------------------
// Info for each serial device controlled

typedef struct mn10300_serial_info {
    CYG_ADDRWORD        base;
    CYG_ADDRWORD        timer_base;
    CYG_WORD            timer_select;
    CYG_WORD            rx_int;
    CYG_WORD            tx_int;
    cyg_bool            is_serial2;
    cyg_interrupt       rx_interrupt;
    cyg_interrupt       tx_interrupt;
    cyg_handle_t        rx_interrupt_handle;
    cyg_handle_t        tx_interrupt_handle;
#ifdef CYG_HAL_MN10300_SERIAL_RX_FIFO
    volatile cyg_int32  fifo_head;
    volatile cyg_int32  fifo_tail;
    volatile cyg_uint8  fifo[16];
#endif    
} mn10300_serial_info;

//-------------------------------------------------------------------------
// Callback functions exported by this driver

static SERIAL_FUNS(mn10300_serial_funs, 
                   mn10300_serial_putc, 
                   mn10300_serial_getc,
                   mn10300_serial_set_config,
                   mn10300_serial_start_xmit,
                   mn10300_serial_stop_xmit
    );

//-------------------------------------------------------------------------
// Hardware info for each serial line

#ifndef CYGPKG_HAL_MN10300_AM31_STDEVAL1
#ifdef CYGPKG_IO_SERIAL_MN10300_SERIAL0
static mn10300_serial_info mn10300_serial_info0 = {
    SERIAL0_BASE,
    TIMER0_BASE,
    SERIAL0_TIMER_SELECT,
    CYGNUM_HAL_INTERRUPT_SERIAL_0_RX,
    CYGNUM_HAL_INTERRUPT_SERIAL_0_TX,
    false
};
#if CYGNUM_IO_SERIAL_MN10300_SERIAL0_BUFSIZE > 0
static unsigned char mn10300_serial_out_buf0[CYGNUM_IO_SERIAL_MN10300_SERIAL0_BUFSIZE];
static unsigned char mn10300_serial_in_buf0[CYGNUM_IO_SERIAL_MN10300_SERIAL0_BUFSIZE];
#endif
#endif // CYGPKG_IO_SERIAL_MN10300_SERIAL0
#endif

#ifdef CYGPKG_IO_SERIAL_MN10300_SERIAL1
static mn10300_serial_info mn10300_serial_info1 = {
    SERIAL1_BASE,
    TIMER1_BASE,    
    SERIAL1_TIMER_SELECT,
    CYGNUM_HAL_INTERRUPT_SERIAL_1_RX,
    CYGNUM_HAL_INTERRUPT_SERIAL_1_TX,
    false
};
#if CYGNUM_IO_SERIAL_MN10300_SERIAL1_BUFSIZE > 0
static unsigned char mn10300_serial_out_buf1[CYGNUM_IO_SERIAL_MN10300_SERIAL1_BUFSIZE];
static unsigned char mn10300_serial_in_buf1[CYGNUM_IO_SERIAL_MN10300_SERIAL1_BUFSIZE];
#endif
#endif // CYGPKG_IO_SERIAL_MN10300_SERIAL1

#ifdef CYGPKG_IO_SERIAL_MN10300_SERIAL2
static mn10300_serial_info mn10300_serial_info2 = {
    SERIAL2_BASE,
    TIMER2_BASE,    
    SERIAL2_TIMER_SELECT,
    CYGNUM_HAL_INTERRUPT_SERIAL_2_RX,
    CYGNUM_HAL_INTERRUPT_SERIAL_2_TX,
    true
};
#if CYGNUM_IO_SERIAL_MN10300_SERIAL2_BUFSIZE > 0
static unsigned char mn10300_serial_out_buf2[CYGNUM_IO_SERIAL_MN10300_SERIAL2_BUFSIZE];
static unsigned char mn10300_serial_in_buf2[CYGNUM_IO_SERIAL_MN10300_SERIAL2_BUFSIZE];
#endif
#endif // CYGPKG_IO_SERIAL_MN10300_SERIAL2


//-------------------------------------------------------------------------
// Channel descriptions:

#ifdef CYGPKG_IO_SERIAL_MN10300_POLLED_MODE
#define SIZEOF_BUF(_x_) 0
#else
#define SIZEOF_BUF(_x_) sizeof(_x_)
#endif

#ifndef CYGPKG_HAL_MN10300_AM31_STDEVAL1
#ifdef CYGPKG_IO_SERIAL_MN10300_SERIAL0
#if CYGNUM_IO_SERIAL_MN10300_SERIAL0_BUFSIZE > 0
static SERIAL_CHANNEL_USING_INTERRUPTS(mn10300_serial_channel0,
                                       mn10300_serial_funs, 
                                       mn10300_serial_info0,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_MN10300_SERIAL0_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &mn10300_serial_out_buf0[0],
                                       SIZEOF_BUF(mn10300_serial_out_buf0),
                                       &mn10300_serial_in_buf0[0],
                                       SIZEOF_BUF(mn10300_serial_in_buf0)
    );
#else
static SERIAL_CHANNEL(mn10300_serial_channel0,
                      mn10300_serial_funs, 
                      mn10300_serial_info0,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_MN10300_SERIAL0_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif
#endif // CYGPKG_IO_SERIAL_MN10300_SERIAL0
#endif
    
#ifdef CYGPKG_IO_SERIAL_MN10300_SERIAL1
#if CYGNUM_IO_SERIAL_MN10300_SERIAL1_BUFSIZE > 0
static SERIAL_CHANNEL_USING_INTERRUPTS(mn10300_serial_channel1,
                                       mn10300_serial_funs, 
                                       mn10300_serial_info1,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_MN10300_SERIAL1_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &mn10300_serial_out_buf1[0],
                                       SIZEOF_BUF(mn10300_serial_out_buf1),
                                       &mn10300_serial_in_buf1[0],
                                       SIZEOF_BUF(mn10300_serial_in_buf1)
    );
#else
static SERIAL_CHANNEL(mn10300_serial_channel1,
                      mn10300_serial_funs, 
                      mn10300_serial_info1,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_MN10300_SERIAL1_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif
#endif // CYGPKG_IO_SERIAL_MN10300_SERIAL1

#ifdef CYGPKG_IO_SERIAL_MN10300_SERIAL2
#if CYGNUM_IO_SERIAL_MN10300_SERIAL2_BUFSIZE > 0
static SERIAL_CHANNEL_USING_INTERRUPTS(mn10300_serial_channel2,
                                       mn10300_serial_funs, 
                                       mn10300_serial_info2,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_MN10300_SERIAL2_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &mn10300_serial_out_buf2[0],
                                       SIZEOF_BUF(mn10300_serial_out_buf2),
                                       &mn10300_serial_in_buf2[0],
                                       SIZEOF_BUF(mn10300_serial_in_buf2)
    );
#else
static SERIAL_CHANNEL(mn10300_serial_channel2,
                      mn10300_serial_funs, 
                      mn10300_serial_info2,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_MN10300_SERIAL2_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif
#endif // CYGPKG_IO_SERIAL_MN10300_SERIAL2
    
//-------------------------------------------------------------------------
// And finally, the device table entries:

#ifndef CYGPKG_HAL_MN10300_AM31_STDEVAL1
#ifdef CYGPKG_IO_SERIAL_MN10300_SERIAL0
// On the standard eval board serial0 is not connected. If enabled, it
// generates continuous frame error and overrun interrupts. Hence we do
// not touch it.
DEVTAB_ENTRY(mn10300_serial_io0, 
             CYGDAT_IO_SERIAL_MN10300_SERIAL0_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             mn10300_serial_init, 
             mn10300_serial_lookup,     // Serial driver may need initializing
             &mn10300_serial_channel0
    );
#endif // CYGPKG_IO_SERIAL_MN10300_SERIAL0
#endif

#ifdef CYGPKG_IO_SERIAL_MN10300_SERIAL1
DEVTAB_ENTRY(mn10300_serial_io1, 
             CYGDAT_IO_SERIAL_MN10300_SERIAL1_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             mn10300_serial_init, 
             mn10300_serial_lookup,     // Serial driver may need initializing
             &mn10300_serial_channel1
    );
#endif // CYGPKG_IO_SERIAL_MN10300_SERIAL1

#ifdef CYGPKG_IO_SERIAL_MN10300_SERIAL2
DEVTAB_ENTRY(mn10300_serial_io2, 
             CYGDAT_IO_SERIAL_MN10300_SERIAL2_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             mn10300_serial_init, 
             mn10300_serial_lookup,     // Serial driver may need initializing
             &mn10300_serial_channel2
    );
#endif // CYGPKG_IO_SERIAL_MN10300_SERIAL2

//-------------------------------------------------------------------------
// Read the serial line's status register. Serial 2 has an 8 bit status
// register while serials 0 and 1 have 16 bit registers. This function
// uses the correct size access, but passes back a 16 bit quantity for
// both.

static cyg_uint16 mn10300_read_sr( mn10300_serial_info *mn10300_chan )
{
    cyg_uint16 sr = 0;
    if( mn10300_chan->is_serial2 )
    {
        cyg_uint8 sr8;
        HAL_READ_UINT8(mn10300_chan->base+SERIAL_STR, sr8);
        sr = sr8;
    }
    else
    {
        HAL_READ_UINT16(mn10300_chan->base+SERIAL_STR, sr);        
    }

    return sr;
}

//-------------------------------------------------------------------------

static bool
mn10300_serial_config_port(serial_channel *chan, cyg_serial_info_t *new_config, bool init)
{
    mn10300_serial_info *mn10300_chan = (mn10300_serial_info *)chan->dev_priv;
    cyg_uint16 cr = 0;
    cyg_uint16 sr;
 
    // wait for the device to become quiescent. This could take some time
    // if the device had been transmitting at a low baud rate.
    do {
        sr = mn10300_read_sr(mn10300_chan);
    } while (sr & (SR_RXF|SR_TXF));

    // Disable device entirely.
    HAL_WRITE_UINT16(mn10300_chan->base+SERIAL_CTR, 0);

    // Set up the Interrupt Mode Register
    HAL_WRITE_UINT8(mn10300_chan->base+SERIAL_ICR, 0);

    // Set up baud rate
    if( mn10300_chan->is_serial2 )
    {
        // Serial 2 is a bit different from 0 and 1 in the way that the
        // baud rate is controlled.

        cyg_uint8 baud_divisor = select_baud_2[new_config->baud].timer2_val;

        if (baud_divisor == 0)
            return false; // Invalid baud rate selected
        
        HAL_WRITE_UINT8(mn10300_chan->timer_base+TIMER_BR, baud_divisor);

        HAL_WRITE_UINT8(mn10300_chan->timer_base+TIMER_MD, 0x80 );

        baud_divisor = select_baud_2[new_config->baud].serial2_val;

        HAL_WRITE_UINT8(mn10300_chan->base+SERIAL_TIM, baud_divisor);
        
        cr |= mn10300_chan->timer_select;
    }
    else
    {
        cyg_uint16 baud_divisor = select_baud_01[new_config->baud];
        cyg_uint8 timer_mode = 0x80;

        if (baud_divisor == 0)
            return false; // Invalid baud rate selected

#if defined(CYGPKG_HAL_MN10300_AM33)        
        if( baud_divisor > 255 )
        {
            // The AM33 runs at a higher clock rate than the AM31 and
            // needs a bigger divisor for low baud rates. We do this by
            // using timer 0 as a prescaler. We set it to 198 so we can then
            // use it to prescale for both serial0 and serial1 if they need
            // it.
            static int timer0_initialized = 0;
            baud_divisor /= 198;
            baud_divisor--;
            timer_mode = 0x84;
            if( !timer0_initialized )
            {
                timer0_initialized = 1;
                HAL_WRITE_UINT8(HW_TIMER0+TIMER_BR, 198 );
                HAL_WRITE_UINT8(HW_TIMER0+TIMER_MD, 0x80 );
            }
        }
#endif

        HAL_WRITE_UINT8(mn10300_chan->timer_base+TIMER_BR, baud_divisor);

        HAL_WRITE_UINT8(mn10300_chan->timer_base+TIMER_MD, timer_mode );

        cr |= mn10300_chan->timer_select;
    }

#ifdef PORT3_MD    
    HAL_WRITE_UINT8( PORT3_MD, 0x01 );
#endif
    
    // set up other config values:

    cr |= select_word_length[new_config->word_length - CYGNUM_SERIAL_WORD_LENGTH_5];
    cr |= select_stop_bits[new_config->stop];
    cr |= select_parity[new_config->parity];

    cr |= LCR_RXE | LCR_TXE;        // enable Rx and Tx
    
#ifdef CYGPKG_HAL_MN10300_AM31
    if( mn10300_chan->is_serial2 )
    {
        // AM31 has an extra TX interrupt enable bit for serial 2.
        DISABLE_TRANSMIT_INTERRUPT(mn10300_chan);
    }
#endif        

    // Write CR into hardware
    HAL_WRITE_UINT16(mn10300_chan->base+SERIAL_CTR, cr);    

    sr = mn10300_read_sr(mn10300_chan);

    if (new_config != &chan->config) {
        chan->config = *new_config;
    }
    return true;
}

//-------------------------------------------------------------------------
// Function to initialize the device.  Called at bootstrap time.

bool mn10300_serial_init(struct cyg_devtab_entry *tab)
{
    serial_channel *chan = (serial_channel *)tab->priv;
    mn10300_serial_info *mn10300_chan = (mn10300_serial_info *)chan->dev_priv;

    (chan->callbacks->serial_init)(chan);  // Really only required for interrupt driven devices

#ifndef CYGPKG_IO_SERIAL_MN10300_POLLED_MODE    
    if (chan->out_cbuf.len != 0) {
        // Install and enable the receive interrupt
        cyg_drv_interrupt_create(mn10300_chan->rx_int,
                                 4,                      // Priority - what goes here?
                                 (cyg_addrword_t)chan,   //  Data item passed to interrupt handler
                                 mn10300_serial_rx_ISR,
                                 mn10300_serial_rx_DSR,
                                 &mn10300_chan->rx_interrupt_handle,
                                 &mn10300_chan->rx_interrupt);
        cyg_drv_interrupt_attach(mn10300_chan->rx_interrupt_handle);
        cyg_drv_interrupt_unmask(mn10300_chan->rx_int);

        // Install and enable the transmit interrupt
        cyg_drv_interrupt_create(mn10300_chan->tx_int,
                                 4,                      // Priority - what goes here?
                                 (cyg_addrword_t)chan,   //  Data item passed to interrupt handler
                                 mn10300_serial_tx_ISR,
                                 mn10300_serial_tx_DSR,
                                 &mn10300_chan->tx_interrupt_handle,
                                 &mn10300_chan->tx_interrupt);
        cyg_drv_interrupt_attach(mn10300_chan->tx_interrupt_handle);
        cyg_drv_interrupt_mask(mn10300_chan->tx_int);
    }
#endif
    
    mn10300_serial_config_port(chan, &chan->config, true);
    
    return true;
}

//-------------------------------------------------------------------------
// This routine is called when the device is "looked" up (i.e. attached)

static Cyg_ErrNo 
mn10300_serial_lookup(struct cyg_devtab_entry **tab, 
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
mn10300_serial_putc(serial_channel *chan, unsigned char c)
{
    mn10300_serial_info *mn10300_chan = (mn10300_serial_info *)chan->dev_priv;
    cyg_uint8 sr = mn10300_read_sr( mn10300_chan);

    if( (sr & SR_TBF) == 0 )
    {
        HAL_WRITE_UINT8( mn10300_chan->base+SERIAL_TXB, c );

        return true;
    }
    else return false;
}

//-------------------------------------------------------------------------

unsigned char 
mn10300_serial_getc(serial_channel *chan)
{
    unsigned char c;
    mn10300_serial_info *mn10300_chan = (mn10300_serial_info *)chan->dev_priv;
    do
    {
        cyg_uint8 sr = mn10300_read_sr( mn10300_chan );

        if( (sr & SR_RBF) != 0 )
        {
            HAL_READ_UINT8( mn10300_chan->base+SERIAL_RXB, c );

            break;
        }
        
    } while(1);
    
    return c;
}

//-------------------------------------------------------------------------

static Cyg_ErrNo
mn10300_serial_set_config(serial_channel *chan, cyg_uint32 key,
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
        if ( true != mn10300_serial_config_port(chan, config, false) )
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
mn10300_serial_start_xmit(serial_channel *chan)
{
#ifndef CYGPKG_IO_SERIAL_MN10300_POLLED_MODE    
    mn10300_serial_info *mn10300_chan = (mn10300_serial_info *)chan->dev_priv;
    cyg_uint16 cr;

    HAL_READ_UINT16( mn10300_chan->base+SERIAL_CTR, cr );

    ENABLE_TRANSMIT_INTERRUPT(mn10300_chan);    

    HAL_WRITE_UINT16( mn10300_chan->base+SERIAL_CTR, cr );

    cyg_drv_interrupt_unmask(mn10300_chan->tx_int);
    
    (chan->callbacks->xmt_char)(chan);
#endif    
}

//-------------------------------------------------------------------------
// Disable the transmitter on the device

static void 
mn10300_serial_stop_xmit(serial_channel *chan)
{
#ifndef CYGPKG_IO_SERIAL_MN10300_POLLED_MODE    
    mn10300_serial_info *mn10300_chan = (mn10300_serial_info *)chan->dev_priv;
    cyg_uint16 cr;
    cyg_uint16 sr;

    // Wait until the transmitter has actually stopped before turning it off.
    
    do
    {
        sr = mn10300_read_sr( mn10300_chan );
        
    } while( sr & SR_TXF );
    
    HAL_READ_UINT16( mn10300_chan->base+SERIAL_CTR, cr );

    DISABLE_TRANSMIT_INTERRUPT(mn10300_chan);
    
    HAL_WRITE_UINT16( mn10300_chan->base+SERIAL_CTR, cr );

    cyg_drv_interrupt_mask(mn10300_chan->tx_int);
    
#endif    
}

//-------------------------------------------------------------------------
// Serial I/O - low level interrupt handlers (ISR)

#ifndef CYGPKG_IO_SERIAL_MN10300_POLLED_MODE

#ifdef CYG_HAL_MN10300_SERIAL_RX_FIFO

// This version of the RX ISR implements a simple receive FIFO. The
// MN10300 serial devices do not have hardware FIFOs (as found in
// 16550s for example), and it can be difficult at times to keep up
// with higher baud rates without overrunning. This ISR implements a
// software equivalent of the hardware FIFO, placing recieved
// characters into the FIFO as soon as they arrive. Whenever the DSR
// is run, it collects all the pending characters from the FIFO for
// delivery to the application. Neither the ISR or DSR disable
// interrupts, instead we rely on being able to write the head and
// tail pointers atomically, to implement lock-free synchronization.

static cyg_uint32 
mn10300_serial_rx_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    mn10300_serial_info *mn10300_chan = (mn10300_serial_info *)chan->dev_priv;
    cyg_uint8 sr = mn10300_read_sr( mn10300_chan);

    while( (sr & SR_RBF) != 0 )
    {
        register cyg_int32 head = mn10300_chan->fifo_head;
        cyg_uint8 c;
        int i;
        HAL_READ_UINT8( mn10300_chan->base+SERIAL_RXB, c );

        mn10300_chan->fifo[head++] = c;

        if( head >= sizeof(mn10300_chan->fifo) )
            head = 0;

        mn10300_chan->fifo_head = head;

        sr = mn10300_read_sr( mn10300_chan);

    }

    cyg_drv_interrupt_acknowledge(mn10300_chan->rx_int);
    
    return CYG_ISR_CALL_DSR;  // Cause DSR to be run
}

#else

static cyg_uint32 
mn10300_serial_rx_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    mn10300_serial_info *mn10300_chan = (mn10300_serial_info *)chan->dev_priv;
 
    cyg_drv_interrupt_mask(mn10300_chan->rx_int);
    cyg_drv_interrupt_acknowledge(mn10300_chan->rx_int);

    return CYG_ISR_CALL_DSR;  // Cause DSR to be run
}

#endif

static cyg_uint32 
mn10300_serial_tx_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    mn10300_serial_info *mn10300_chan = (mn10300_serial_info *)chan->dev_priv;

    cyg_drv_interrupt_mask(mn10300_chan->tx_int);
    cyg_drv_interrupt_acknowledge(mn10300_chan->tx_int);

    return CYG_ISR_CALL_DSR;  // Cause DSR to be run
}

#endif

//-------------------------------------------------------------------------
// Serial I/O - high level interrupt handler (DSR)

#ifndef CYGPKG_IO_SERIAL_MN10300_POLLED_MODE

#ifdef CYG_HAL_MN10300_SERIAL_RX_FIFO

static void       
mn10300_serial_rx_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    mn10300_serial_info *mn10300_chan = (mn10300_serial_info *)chan->dev_priv;
    register cyg_int32 head = mn10300_chan->fifo_head;
    register cyg_int32 tail = mn10300_chan->fifo_tail;
    
    while( head != tail )
    {
        cyg_uint8 c = mn10300_chan->fifo[tail++];

        if( tail >= sizeof(mn10300_chan->fifo) ) tail = 0;

        (chan->callbacks->rcv_char)(chan, c);
    }

    mn10300_chan->fifo_tail = tail;
}

#else

static void       
mn10300_serial_rx_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    mn10300_serial_info *mn10300_chan = (mn10300_serial_info *)chan->dev_priv;
    cyg_uint8 sr = mn10300_read_sr( mn10300_chan);

    if( (sr & SR_RBF) != 0 )
    {
        cyg_uint8 rxb;
        HAL_READ_UINT8( mn10300_chan->base+SERIAL_RXB, rxb );

        (chan->callbacks->rcv_char)(chan, rxb);
    }

    cyg_drv_interrupt_unmask(mn10300_chan->rx_int);
}

#endif

static void       
mn10300_serial_tx_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    mn10300_serial_info *mn10300_chan = (mn10300_serial_info *)chan->dev_priv;
    cyg_uint8 sr = mn10300_read_sr( mn10300_chan);

    if( (sr & SR_TBF) == 0 )
    {
        (chan->callbacks->xmt_char)(chan);
    }
    
    cyg_drv_interrupt_unmask(mn10300_chan->tx_int);
}

#endif

#endif // CYGPKG_IO_SERIAL_MN10300

//-------------------------------------------------------------------------
// EOF mn10300.c
