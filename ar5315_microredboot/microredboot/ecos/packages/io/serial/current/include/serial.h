#ifndef CYGONCE_SERIAL_H
#define CYGONCE_SERIAL_H
// ====================================================================
//
//      serial.h
//
//      Device I/O 
//
// ====================================================================
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
// ====================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   gthomas
// Contributors:        gthomas
// Date:        1999-02-04
// Purpose:     Internal interfaces for serial I/O drivers
// Description:
//
//####DESCRIPTIONEND####
//
// ====================================================================

// Serial I/O interfaces

#include <pkgconf/system.h>
#include <pkgconf/io_serial.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/io/io.h>
#include <cyg/io/serialio.h>
#include <cyg/io/devtab.h>
#include <cyg/hal/drv_api.h>

#ifdef CYGPKG_IO_SERIAL_SELECT_SUPPORT
#include <cyg/fileio/fileio.h>
#endif

typedef struct serial_channel serial_channel;
typedef struct serial_funs serial_funs;

// The block transfer request functions may fail for one of two
// reasons. It's important for the caller to know which.
typedef enum {
    CYG_RCV_OK,
    CYG_RCV_FULL,
    CYG_RCV_DISABLED
} rcv_req_reply_t;

typedef enum {
    CYG_XMT_OK,
    CYG_XMT_EMPTY,
    CYG_XMT_DISABLED
} xmt_req_reply_t;

// Pointers into upper-level driver which interrupt handlers need
typedef struct {
    // Initialize the channel
    void (*serial_init)(serial_channel *chan);
    // Cause an additional character to be output if one is available
    void (*xmt_char)(serial_channel *chan);
    // Consume an input character
    void (*rcv_char)(serial_channel *chan, unsigned char c);
#if CYGINT_IO_SERIAL_BLOCK_TRANSFER
    // Request space for input characters
    rcv_req_reply_t (*data_rcv_req)(serial_channel *chan, int avail, 
                                    int* space_avail, unsigned char** space);
    // Receive operation completed
    void (*data_rcv_done)(serial_channel *chan, int chars_rcvd);
    // Request characters for transmission
    xmt_req_reply_t (*data_xmt_req)(serial_channel *chan, int space,
                                    int* chars_avail, unsigned char** chars);
    // Transmit operation completed
    void (*data_xmt_done)(serial_channel *chan, int chars_sent);
#endif
#if defined(CYGOPT_IO_SERIAL_SUPPORT_LINE_STATUS)
    void (*indicate_status)(serial_channel *chan, cyg_serial_line_status_t *s );
#endif
} serial_callbacks_t;

#if CYGINT_IO_SERIAL_BLOCK_TRANSFER
# ifdef CYGOPT_IO_SERIAL_SUPPORT_LINE_STATUS
#  define SERIAL_CALLBACKS(_l,_init,_xmt_char,_rcv_char, _data_rcv_req, _data_rcv_done, _data_xmt_req, _data_xmt_done, _indicate_status)  \
serial_callbacks_t _l = {                               \
    _init,                                              \
    _xmt_char,                                          \
    _rcv_char,                                          \
    _data_rcv_req,                                      \
    _data_rcv_done,                                     \
    _data_xmt_req,                                      \
    _data_xmt_done,                                     \
    _indicate_status                                    \
};
# else
#  define SERIAL_CALLBACKS(_l,_init,_xmt_char,_rcv_char, _data_rcv_req, _data_rcv_done, _data_xmt_req, _data_xmt_done)  \
serial_callbacks_t _l = {                               \
    _init,                                              \
    _xmt_char,                                          \
    _rcv_char,                                          \
    _data_rcv_req,                                      \
    _data_rcv_done,                                     \
    _data_xmt_req,                                      \
    _data_xmt_done                                      \
};
# endif
#else
# ifdef CYGOPT_IO_SERIAL_SUPPORT_LINE_STATUS
#  define SERIAL_CALLBACKS(_l,_init,_xmt_char,_rcv_char,_indicate_status)  \
serial_callbacks_t _l = {                               \
    _init,                                              \
    _xmt_char,                                          \
    _rcv_char,                                          \
    _indicate_status                                    \
};
# else
#  define SERIAL_CALLBACKS(_l,_init,_xmt_char,_rcv_char)  \
serial_callbacks_t _l = {                               \
    _init,                                              \
    _xmt_char,                                          \
    _rcv_char                                           \
};
# endif
#endif

extern serial_callbacks_t cyg_io_serial_callbacks;

typedef struct {
    unsigned char           *data;
    volatile int             put;
    volatile int             get;
    int                      len;
    volatile int             nb;          // count of bytes currently in buffer
    int                      low_water;   // For tx: min space in buffer before restart
                                          // For rx: max buffer used before flow unthrottled
#ifdef CYGPKG_IO_SERIAL_FLOW_CONTROL
    int                      high_water;  // For tx: unused
                                          // For rx: min buffer used before throttle
#endif
    cyg_drv_cond_t           wait;
    cyg_drv_mutex_t          lock;
    bool                     waiting;
#ifdef CYGOPT_IO_SERIAL_SUPPORT_NONBLOCKING
    bool                     blocking;
#endif
    volatile bool            abort;       // Set by an outsider to kill processing
    volatile cyg_int32       pending;     // This many bytes waiting to be sent
#ifdef CYGPKG_IO_SERIAL_SELECT_SUPPORT    
    struct CYG_SELINFO_TAG   selinfo;     // select info
#endif

#ifdef CYGDBG_USE_ASSERTS
#ifdef CYGINT_IO_SERIAL_BLOCK_TRANSFER
    bool                     block_mode_xfer_running;
#endif // CYGINT_IO_SERIAL_BLOCK_TRANSFER
#endif // CYGDBG_USE_ASSERTS
} cbuf_t;

#define CBUF_INIT(_data, _len) \
   {_data, 0, 0, _len}

#ifdef CYGPKG_IO_SERIAL_FLOW_CONTROL
typedef struct {
    cyg_uint32 flags;
#ifdef CYGOPT_IO_SERIAL_FLOW_CONTROL_SOFTWARE
    cyg_uint8  xchar;
#endif
} flow_desc_t;
#endif

// Private data which describes this channel
struct serial_channel {
    serial_funs        *funs;
    serial_callbacks_t *callbacks;
    void               *dev_priv;  // Whatever is needed by actual device routines
    cyg_serial_info_t   config;    // Current configuration
    bool                init;
    cbuf_t              out_cbuf;
    cbuf_t              in_cbuf;
#ifdef CYGPKG_IO_SERIAL_FLOW_CONTROL
    flow_desc_t         flow_desc;
#endif
#ifdef CYGOPT_IO_SERIAL_SUPPORT_LINE_STATUS
    cyg_serial_line_status_callback_fn_t status_callback;
    CYG_ADDRWORD             status_callback_priv;
#endif
};

// Flow descriptor flag values
#define CYG_SERIAL_FLOW_OUT_THROTTLED     (1<<0)
#define CYG_SERIAL_FLOW_IN_THROTTLED      (1<<1)

// Initialization macro for serial channel
#define SERIAL_CHANNEL(_l,                                              \
                       _funs,                                           \
                       _dev_priv,                                       \
                       _baud, _stop, _parity, _word_length, _flags)     \
serial_channel _l = {                                                   \
    &_funs,                                                             \
    &cyg_io_serial_callbacks,                                           \
    &(_dev_priv),                                                       \
    CYG_SERIAL_INFO_INIT(_baud, _stop, _parity, _word_length, _flags),  \
};

#define SERIAL_CHANNEL_USING_INTERRUPTS(_l,                             \
                       _funs,                                           \
                       _dev_priv,                                       \
                       _baud, _stop, _parity, _word_length, _flags,     \
                       _out_buf, _out_buflen,                           \
                       _in_buf, _in_buflen)                             \
serial_channel _l = {                                                   \
    &_funs,                                                             \
    &cyg_io_serial_callbacks,                                           \
    &(_dev_priv),                                                       \
    CYG_SERIAL_INFO_INIT(_baud, _stop, _parity, _word_length, _flags),  \
    false,                                                              \
    CBUF_INIT(_out_buf, _out_buflen),                                   \
    CBUF_INIT(_in_buf, _in_buflen)                                      \
};

// Low level interface functions
struct serial_funs {
    // Send one character to the output device, return true if consumed
    bool (*putc)(serial_channel *priv, unsigned char c);
    // Fetch one character from the device
    unsigned char (*getc)(serial_channel *priv);    
    // Change hardware configuration (baud rate, etc)
    Cyg_ErrNo (*set_config)(serial_channel *priv, cyg_uint32 key, const void *xbuf,
                            cyg_uint32 *len);
    // Enable the transmit channel and turn on transmit interrupts
    void (*start_xmit)(serial_channel *priv);
    // Disable the transmit channel and turn transmit interrupts off
    void (*stop_xmit)(serial_channel *priv);
};

#define SERIAL_FUNS(_l,_putc,_getc,_set_config,_start_xmit,_stop_xmit)  \
serial_funs _l = {                                                      \
  _putc,                                                                \
  _getc,                                                                \
  _set_config,                                                          \
  _start_xmit,                                                          \
  _stop_xmit                                                            \
};

extern cyg_devio_table_t cyg_io_serial_devio;

#endif // CYGONCE_SERIAL_H
