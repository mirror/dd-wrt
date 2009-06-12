//==========================================================================
//
//      loop_serial.c
//
//      Loopback serial device driver
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
// Purpose:     Loopback serial device driver
// Description: This device driver implements a pair of serial lines that are
//              connected back-to-back. Data output to one will appear as
//              input on the other. This process in in part driven by an alarm
//              object which provides a degree of separation between the two
//              channels.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/io_serial.h>
#include <pkgconf/io_serial_loop.h>
#include <cyg/hal/hal_io.h>

#include <cyg/io/io.h>
#include <cyg/io/devtab.h>
#include <cyg/io/serial.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/kernel/kapi.h>

#ifdef CYGPKG_IO_SERIAL_LOOP

//-------------------------------------------------------------------------

extern void diag_printf(const char *fmt, ...);

//-------------------------------------------------------------------------
// Forward definitions

static bool loop_serial_init(struct cyg_devtab_entry *tab);
static bool loop_serial_putc(serial_channel *chan, unsigned char c);
static Cyg_ErrNo loop_serial_lookup(struct cyg_devtab_entry **tab, 
                                   struct cyg_devtab_entry *sub_tab,
                                   const char *name);
static unsigned char loop_serial_getc(serial_channel *chan);
static Cyg_ErrNo loop_serial_set_config(serial_channel *chan, cyg_uint32 key,
                                        const void *xbuf, cyg_uint32 *len);
static void loop_serial_start_xmit(serial_channel *chan);
static void loop_serial_stop_xmit(serial_channel *chan);

#ifndef CYGPKG_IO_SERIAL_LOOP_POLLED_MODE
static void alarm_handler(cyg_handle_t alarm, cyg_addrword_t data);
#endif

//-------------------------------------------------------------------------
// Alarm object for feeding data back into serial channels

static cyg_alarm alarm_obj;

static cyg_handle_t alarm_handle;

//-------------------------------------------------------------------------
// Transfer FIFOs

#define FIFO_SIZE 16

struct fifo
{
    cyg_bool            tx_enable;
    volatile int        head;
    volatile int        tail;
    volatile int        num;
    volatile char       buf[FIFO_SIZE+1];
};

static struct fifo fifo0 = { false, 0, 0, 0 };   // from serial0 to serial1
static struct fifo fifo1 = { false, 0, 0, 0 };   // from serial1 to serial0

//-------------------------------------------------------------------------

#define BUFSIZE 128

//-------------------------------------------------------------------------
// Info for each serial device controlled

typedef struct loop_serial_info {
    struct fifo         *write_fifo;
    struct fifo         *read_fifo;
} loop_serial_info;

//-------------------------------------------------------------------------
// Callback functions exported by this driver

static SERIAL_FUNS(loop_serial_funs, 
                   loop_serial_putc, 
                   loop_serial_getc,
                   loop_serial_set_config,
                   loop_serial_start_xmit,
                   loop_serial_stop_xmit
    );

//-------------------------------------------------------------------------
// Hardware info for each serial line

#ifdef CYGPKG_IO_SERIAL_LOOP_SERIAL0
static loop_serial_info loop_serial_info0 = {
    &fifo0,
    &fifo1
};
#if CYGNUM_IO_SERIAL_LOOP_SERIAL0_BUFSIZE > 0
static unsigned char loop_serial_out_buf0[CYGNUM_IO_SERIAL_LOOP_SERIAL0_BUFSIZE];
static unsigned char loop_serial_in_buf0[CYGNUM_IO_SERIAL_LOOP_SERIAL0_BUFSIZE];
#endif
#endif // CYGPKG_IO_SERIAL_LOOP_SERIAL0

#ifdef CYGPKG_IO_SERIAL_LOOP_SERIAL1
static loop_serial_info loop_serial_info1 = {
    &fifo1,
    &fifo0
};
#if CYGNUM_IO_SERIAL_LOOP_SERIAL1_BUFSIZE > 0
static unsigned char loop_serial_out_buf1[CYGNUM_IO_SERIAL_LOOP_SERIAL1_BUFSIZE];
static unsigned char loop_serial_in_buf1[CYGNUM_IO_SERIAL_LOOP_SERIAL1_BUFSIZE];
#endif
#endif // CYGPKG_IO_SERIAL_LOOP_SERIAL1

//-------------------------------------------------------------------------
// Channel descriptions:

#ifdef CYGPKG_IO_SERIAL_LOOP_POLLED_MODE
#define SIZEOF_BUF(_x_) 0
#else
#define SIZEOF_BUF(_x_) sizeof(_x_)
#endif

#ifdef CYGPKG_IO_SERIAL_LOOP_SERIAL0
#if CYGNUM_IO_SERIAL_LOOP_SERIAL0_BUFSIZE > 0
static SERIAL_CHANNEL_USING_INTERRUPTS(loop_serial_channel0,
                                       loop_serial_funs, 
                                       loop_serial_info0,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_LOOP_SERIAL0_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &loop_serial_out_buf0[0],
                                       SIZEOF_BUF(loop_serial_out_buf0),
                                       &loop_serial_in_buf0[0],
                                       SIZEOF_BUF(loop_serial_in_buf0)
    );
#else
static SERIAL_CHANNEL(loop_serial_channel0,
                      loop_serial_funs, 
                      loop_serial_info0,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_LOOP_SERIAL0_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif
#endif // CYGPKG_IO_SERIAL_LOOP_SERIAL0
    
#ifdef CYGPKG_IO_SERIAL_LOOP_SERIAL1
#if CYGNUM_IO_SERIAL_LOOP_SERIAL1_BUFSIZE > 0
static SERIAL_CHANNEL_USING_INTERRUPTS(loop_serial_channel1,
                                       loop_serial_funs, 
                                       loop_serial_info1,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_LOOP_SERIAL1_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &loop_serial_out_buf1[0],
                                       SIZEOF_BUF(loop_serial_out_buf1),
                                       &loop_serial_in_buf1[0],
                                       SIZEOF_BUF(loop_serial_in_buf1)
    );
#else
static SERIAL_CHANNEL(loop_serial_channel1,
                      loop_serial_funs, 
                      loop_serial_info1,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_LOOP_SERIAL1_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif
#endif // CYGPKG_IO_SERIAL_LOOP_SERIAL1
    
//-------------------------------------------------------------------------
// And finally, the device table entries:

#ifdef CYGPKG_IO_SERIAL_LOOP_SERIAL0
DEVTAB_ENTRY(loop_serial_io0, 
             CYGDAT_IO_SERIAL_LOOP_SERIAL0_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             loop_serial_init, 
             loop_serial_lookup,     // Serial driver may need initializing
             &loop_serial_channel0
    );
#endif // CYGPKG_IO_SERIAL_LOOP_SERIAL0

#ifdef CYGPKG_IO_SERIAL_LOOP_SERIAL1
DEVTAB_ENTRY(loop_serial_io1, 
             CYGDAT_IO_SERIAL_LOOP_SERIAL1_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             loop_serial_init, 
             loop_serial_lookup,     // Serial driver may need initializing
             &loop_serial_channel1
    );
#endif // CYGPKG_IO_SERIAL_LOOP_SERIAL1

//-------------------------------------------------------------------------

static bool
loop_serial_config_port(serial_channel *chan, cyg_serial_info_t *new_config, bool init)
{
//    loop_serial_info *loop_chan = (loop_serial_info *)chan->dev_priv;
 
    if (new_config != &chan->config) {
        chan->config = *new_config;
    }
    return true;
}

//-------------------------------------------------------------------------
// Function to initialize the device.  Called at bootstrap time.

bool loop_serial_init(struct cyg_devtab_entry *tab)
{
    serial_channel *chan = (serial_channel *)tab->priv;
//    loop_serial_info *loop_chan = (loop_serial_info *)chan->dev_priv;

    (chan->callbacks->serial_init)(chan);  // Really only required for interrupt driven devices

#ifndef CYGPKG_IO_SERIAL_LOOP_POLLED_MODE    

    // Set up alarm for feeding data back into channels

    cyg_alarm_create( cyg_real_time_clock(),
                      alarm_handler,
                      0,
                      &alarm_handle,
                      &alarm_obj);

    cyg_alarm_initialize( alarm_handle, 1, 1 );
    
#endif
    
    loop_serial_config_port(chan, &chan->config, true);
    
    return true;
}

//-------------------------------------------------------------------------
// This routine is called when the device is "looked" up (i.e. attached)

static Cyg_ErrNo 
loop_serial_lookup(struct cyg_devtab_entry **tab, 
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
loop_serial_putc(serial_channel *chan, unsigned char c)
{
    loop_serial_info *loop_chan = (loop_serial_info *)chan->dev_priv;

    struct fifo *f = loop_chan->write_fifo;

    if( f->num == FIFO_SIZE )
        return false;

    f->buf[f->tail] = c;
    f->num++;
    f->tail++;
    if( f->tail == sizeof(f->buf) )
        f->tail = 0;
    
    return true;
}

//-------------------------------------------------------------------------

unsigned char 
loop_serial_getc(serial_channel *chan)
{
    unsigned char c;
    loop_serial_info *loop_chan = (loop_serial_info *)chan->dev_priv;

    struct fifo *f = loop_chan->read_fifo;
    
    while( f->num == 0 )
        continue;

    c = f->buf[f->head];
    f->num--;
    f->head++;
    if( f->head == sizeof(f->buf) )
        f->head = 0;
    
    return c;
}

//-------------------------------------------------------------------------

static Cyg_ErrNo
loop_serial_set_config(serial_channel *chan, cyg_uint32 key,
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
        if ( true != loop_serial_config_port(chan, config, false) )
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
loop_serial_start_xmit(serial_channel *chan)
{
#ifndef CYGPKG_IO_SERIAL_LOOP_POLLED_MODE    
    loop_serial_info *loop_chan = (loop_serial_info *)chan->dev_priv;

    loop_chan->write_fifo->tx_enable = true;
    
    (chan->callbacks->xmt_char)(chan);
#endif    
}

//-------------------------------------------------------------------------
// Disable the transmitter on the device

static void 
loop_serial_stop_xmit(serial_channel *chan)
{
#ifndef CYGPKG_IO_SERIAL_LOOP_POLLED_MODE    
    loop_serial_info *loop_chan = (loop_serial_info *)chan->dev_priv;

    loop_chan->write_fifo->tx_enable = false;
    
#endif    
}

//-------------------------------------------------------------------------

static void alarm_handler(cyg_handle_t alarm, cyg_addrword_t data)
{
    serial_channel *chan0 = &loop_serial_channel0;
    serial_channel *chan1 = &loop_serial_channel1;

    while( fifo0.num )
    {
        // Data ready for delivery to serial1

        struct fifo *f = &fifo0;
        char c;

        c = f->buf[f->head];
        f->num--;
        f->head++;
        if( f->head == sizeof(f->buf) )
            f->head = 0;

        (chan1->callbacks->rcv_char)(chan1, c);
        if( f->tx_enable )
            (chan0->callbacks->xmt_char)(chan0);        
    }

    while( fifo1.num )
    {
        // Data ready for delivery to serial0

        struct fifo *f = &fifo1;
        char c;

        c = f->buf[f->head];
        f->num--;
        f->head++;
        if( f->head == sizeof(f->buf) )
            f->head = 0;

        (chan0->callbacks->rcv_char)(chan0, c);
        if( f->tx_enable )
            (chan1->callbacks->xmt_char)(chan1);        
    }



}
    

#endif // CYGPKG_IO_SERIAL_LOOP

//-------------------------------------------------------------------------
// EOF loop.c
