//==========================================================================
//
//      io/serial/common/serial.c
//
//      High level serial driver
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
// Author(s):    gthomas
// Contributors: gthomas, grante, jlarmour, jskov
// Date:         1999-02-04
// Purpose:      Top level serial driver
// Description: 
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/io.h>
#include <pkgconf/io_serial.h>

#include <cyg/io/io.h>
#include <cyg/io/devtab.h>
#include <cyg/io/serial.h>
#include <cyg/infra/cyg_ass.h>      // assertion support
#include <cyg/infra/diag.h>         // diagnostic output

static Cyg_ErrNo serial_write(cyg_io_handle_t handle, const void *buf, cyg_uint32 *len);
static Cyg_ErrNo serial_read(cyg_io_handle_t handle, void *buf, cyg_uint32 *len);
static cyg_bool serial_select(cyg_io_handle_t handle, cyg_uint32 which, CYG_ADDRWORD info);
static Cyg_ErrNo serial_get_config(cyg_io_handle_t handle, cyg_uint32 key, void *buf, cyg_uint32 *len);
static Cyg_ErrNo serial_set_config(cyg_io_handle_t handle, cyg_uint32 key, const void *buf, cyg_uint32 *len);

DEVIO_TABLE(cyg_io_serial_devio,
            serial_write,
            serial_read,
            serial_select,
            serial_get_config,
            serial_set_config
    );

static void serial_init(serial_channel *chan);
static void serial_xmt_char(serial_channel *chan);
static void serial_rcv_char(serial_channel *chan, unsigned char c);
#ifdef CYGOPT_IO_SERIAL_SUPPORT_LINE_STATUS
static void serial_indicate_status(serial_channel *chan,
                                   cyg_serial_line_status_t *s);
#endif
#if CYGINT_IO_SERIAL_BLOCK_TRANSFER
static rcv_req_reply_t serial_data_rcv_req(serial_channel *chan, int avail, 
                                           int* space_avail, 
                                           unsigned char** space);
static void serial_data_rcv_done(serial_channel *chan, int chars_rcvd);
static xmt_req_reply_t serial_data_xmt_req(serial_channel *chan, int space,
                                           int* chars_avail, 
                                           unsigned char** chars);
static void serial_data_xmt_done(serial_channel *chan, int chars_sent);
# ifdef CYGOPT_IO_SERIAL_SUPPORT_LINE_STATUS
SERIAL_CALLBACKS(cyg_io_serial_callbacks, 
                 serial_init, 
                 serial_xmt_char, 
                 serial_rcv_char,
                 serial_data_rcv_req,
                 serial_data_rcv_done,
                 serial_data_xmt_req,
                 serial_data_xmt_done,
                 serial_indicate_status);

# else
SERIAL_CALLBACKS(cyg_io_serial_callbacks, 
                 serial_init, 
                 serial_xmt_char, 
                 serial_rcv_char,
                 serial_data_rcv_req,
                 serial_data_rcv_done,
                 serial_data_xmt_req,
                 serial_data_xmt_done);
# endif
#else
# ifdef CYGOPT_IO_SERIAL_SUPPORT_LINE_STATUS
SERIAL_CALLBACKS(cyg_io_serial_callbacks, 
                 serial_init, 
                 serial_xmt_char, 
                 serial_rcv_char,
                 serial_indicate_status);
# else
SERIAL_CALLBACKS(cyg_io_serial_callbacks, 
                 serial_init, 
                 serial_xmt_char, 
                 serial_rcv_char);
# endif
#endif

// ---------------------------------------------------------------------------

#ifdef CYGPKG_IO_SERIAL_FLOW_CONTROL

static __inline__ void
throttle_tx( serial_channel *chan )
{
    chan->flow_desc.flags |= CYG_SERIAL_FLOW_OUT_THROTTLED;
    // the throttling itself occurs in the serial_xmt_char() callback
}

static __inline__ void
restart_tx( serial_channel *chan )
{
    serial_funs *funs = chan->funs;

    chan->flow_desc.flags &= ~CYG_SERIAL_FLOW_OUT_THROTTLED;

#ifdef CYGPKG_IO_SERIAL_SELECT_SUPPORT
    // See if there is now enough room to say it is available
    // for writing
    {
        cbuf_t *cbuf = &chan->out_cbuf;
        int space;
        
        space = cbuf->len - cbuf->nb;
        if (space >= cbuf->low_water)
            cyg_selwakeup( &cbuf->selinfo );
    }
#endif
    if ( chan->out_cbuf.nb > 0 )
        (funs->start_xmit)(chan);
}

static __inline__ void
throttle_rx( serial_channel *chan, cyg_bool force )
{
    serial_funs *funs = chan->funs;
#ifdef CYGOPT_IO_SERIAL_FLOW_CONTROL_SOFTWARE
    cyg_uint32 prev_flags = chan->flow_desc.flags;
#endif

    chan->flow_desc.flags |= CYG_SERIAL_FLOW_IN_THROTTLED;
#ifdef CYGOPT_IO_SERIAL_FLOW_CONTROL_SOFTWARE
    // send an xoff if not already done (throttled)
    if ( force ||
        ((chan->config.flags & CYGNUM_SERIAL_FLOW_XONXOFF_RX) &&
         (0==(prev_flags & CYG_SERIAL_FLOW_IN_THROTTLED))) ) {
        CYG_ASSERT(force||(chan->flow_desc.xchar=='\0')||(chan->flow_desc.xchar==CYGDAT_IO_SERIAL_FLOW_CONTROL_XOFF_CHAR), "xchar already set (XOFF)");
        chan->flow_desc.xchar = CYGDAT_IO_SERIAL_FLOW_CONTROL_XOFF_CHAR;
        // Make sure xmit is running so we can send it
        (funs->start_xmit)(chan); 
    }
#endif
#ifdef CYGOPT_IO_SERIAL_FLOW_CONTROL_HW
    {
        cyg_uint32 i=1;
        cyg_uint32 len = sizeof(i);
        
        // set hardware flow control - don't care if it fails
        if ( force || (chan->config.flags & CYGNUM_SERIAL_FLOW_RTSCTS_RX) ||
             (chan->config.flags & CYGNUM_SERIAL_FLOW_DSRDTR_RX) )
            (funs->set_config)(chan,
                               CYG_IO_SET_CONFIG_SERIAL_HW_RX_FLOW_THROTTLE,
                               &i, &len);
    }
#endif
}

static __inline__ void
restart_rx( serial_channel *chan, cyg_bool force )
{
    serial_funs *funs = chan->funs;
#ifdef CYGOPT_IO_SERIAL_FLOW_CONTROL_SOFTWARE
    cyg_uint32 prev_flags = chan->flow_desc.flags;
#endif

    chan->flow_desc.flags &= ~CYG_SERIAL_FLOW_IN_THROTTLED;
#ifdef CYGOPT_IO_SERIAL_FLOW_CONTROL_SOFTWARE
    // send an xon, if we haven't already
    if ( force ||
        ((chan->config.flags & CYGNUM_SERIAL_FLOW_XONXOFF_RX) &&
         (prev_flags & CYG_SERIAL_FLOW_IN_THROTTLED)) ) {
        CYG_ASSERT(force||(chan->flow_desc.xchar=='\0')||(chan->flow_desc.xchar==CYGDAT_IO_SERIAL_FLOW_CONTROL_XON_CHAR), "xchar already set (XON)");
        chan->flow_desc.xchar = CYGDAT_IO_SERIAL_FLOW_CONTROL_XON_CHAR;
        (funs->start_xmit)(chan);  // Make sure xmit is running so we can send it
    }
#endif
#ifdef CYGOPT_IO_SERIAL_FLOW_CONTROL_HW
    {
        cyg_uint32 i=0;
        cyg_uint32 len = sizeof(i);
        
        // set hardware flow control - don't care if it fails
        if ( force || (chan->config.flags & CYGNUM_SERIAL_FLOW_RTSCTS_RX) ||
             (chan->config.flags & CYGNUM_SERIAL_FLOW_DSRDTR_RX) )
            (funs->set_config)(chan,
                               CYG_IO_SET_CONFIG_SERIAL_HW_RX_FLOW_THROTTLE,
                               &i, &len);
    }
#endif
}

#endif

// ---------------------------------------------------------------------------

static void
serial_init(serial_channel *chan)
{
    if (chan->init) return;
    if (chan->out_cbuf.len != 0) {
#ifdef CYGDBG_IO_INIT
        diag_printf("Set output buffer - buf: %x len: %d\n", chan->out_cbuf.data, chan->out_cbuf.len);
#endif
        chan->out_cbuf.waiting = false;
        chan->out_cbuf.abort = false;
#ifdef CYGOPT_IO_SERIAL_SUPPORT_NONBLOCKING
        chan->out_cbuf.blocking = true;
#endif
        chan->out_cbuf.pending = 0;
        cyg_drv_mutex_init(&chan->out_cbuf.lock);
        cyg_drv_cond_init(&chan->out_cbuf.wait, &chan->out_cbuf.lock);
        chan->out_cbuf.low_water = chan->out_cbuf.len / 4;
#ifdef CYGPKG_IO_SERIAL_SELECT_SUPPORT
        cyg_selinit( &chan->out_cbuf.selinfo );
#endif        
    }
    if (chan->in_cbuf.len != 0) {
        cbuf_t *cbuf = &chan->in_cbuf;

#ifdef CYGDBG_IO_INIT
        diag_printf("Set input buffer - buf: %x len: %d\n", cbuf->data, cbuf->len);
#endif
        cbuf->waiting = false;
        cbuf->abort = false;
#ifdef CYGOPT_IO_SERIAL_SUPPORT_NONBLOCKING
        cbuf->blocking = true;
#endif
#ifdef CYGPKG_IO_SERIAL_SELECT_SUPPORT
        cyg_selinit( &cbuf->selinfo );
#endif
#ifdef CYGPKG_IO_SERIAL_FLOW_CONTROL
        cbuf->low_water =
            (CYGNUM_IO_SERIAL_FLOW_CONTROL_LOW_WATER_PERCENT * cbuf->len) / 100;
        cbuf->high_water =
            (CYGNUM_IO_SERIAL_FLOW_CONTROL_HIGH_WATER_PERCENT * cbuf->len) / 100;
# ifdef CYGOPT_IO_SERIAL_FLOW_CONTROL_SOFTWARE
        // But make sure it is at least 35 below buffer size, to allow
        // for 16 byte fifos, twice, plus some latency before s/w flow
        // control can kick in. This doesn't apply to h/w flow control
        // as it is near-instantaneous
        if ( (cbuf->len - cbuf->high_water) < 35 )
            cbuf->high_water = cbuf->len - 35;
        // and just in case...
        if ( cbuf->high_water <= 0 )
            cbuf->high_water = 1;
        if ( cbuf->low_water > cbuf->high_water )
            cbuf->low_water = cbuf->high_water;
# endif
#endif
        cyg_drv_mutex_init(&cbuf->lock);
        cyg_drv_cond_init(&cbuf->wait, &cbuf->lock);
    }
#ifdef CYGOPT_IO_SERIAL_SUPPORT_LINE_STATUS
    chan->status_callback = NULL;
#endif

#ifdef CYGDBG_USE_ASSERTS
#if CYGINT_IO_SERIAL_BLOCK_TRANSFER
    chan->in_cbuf.block_mode_xfer_running = false;
    chan->out_cbuf.block_mode_xfer_running = false;
#endif // CYGINT_IO_SERIAL_BLOCK_TRANSFER
#endif // CYGDBG_USE_ASSERTS
    chan->init = true;
}

// ---------------------------------------------------------------------------

static Cyg_ErrNo 
serial_write(cyg_io_handle_t handle, const void *_buf, cyg_uint32 *len)
{
    cyg_devtab_entry_t *t = (cyg_devtab_entry_t *)handle;
    serial_channel *chan = (serial_channel *)t->priv;
    serial_funs *funs = chan->funs;
    cyg_int32 size = *len;
    cyg_uint8 *buf = (cyg_uint8 *)_buf;
    int next;
    cbuf_t *cbuf = &chan->out_cbuf;
    Cyg_ErrNo res = ENOERR;

    cyg_drv_mutex_lock(&cbuf->lock);
    cbuf->abort = false;

    if (cbuf->len == 0) {
        // Non interrupt driven (i.e. polled) operation
        while (size-- > 0) {
#ifdef CYGPKG_IO_SERIAL_FLOW_CONTROL
            while ( ( 0 != (chan->flow_desc.flags & CYG_SERIAL_FLOW_OUT_THROTTLED) ) ||
                    ((funs->putc)(chan, *buf) == false) ) 
                ;  // Ignore full, keep trying
#else
            while ((funs->putc)(chan, *buf) == false) 
                ;  // Ignore full, keep trying
#endif
            buf++;
        }
    } else {
        cyg_drv_dsr_lock();  // Avoid race condition testing pointers
        while (size > 0) {       
            next = cbuf->put + 1;
            if (next == cbuf->len) next = 0;
            if (cbuf->nb == cbuf->len) {
                cbuf->waiting = true;
                // Buffer full - wait for space
#ifdef CYGPKG_IO_SERIAL_FLOW_CONTROL
                if ( 0 == (chan->flow_desc.flags & CYG_SERIAL_FLOW_OUT_THROTTLED) )
#endif
                    (funs->start_xmit)(chan);  // Make sure xmit is running

                // Check flag: 'start_xmit' may have obviated the need
                // to wait :-)
                if (cbuf->waiting) {
#ifdef CYGOPT_IO_SERIAL_SUPPORT_NONBLOCKING
                    // Optionally return if configured for non-blocking mode.
                    if (!cbuf->blocking) {
                        *len -= size;   // number of characters actually sent
                        cbuf->waiting = false;
                        res = (*len == 0) ? -EAGAIN : ENOERR;
                        break;
                    }
#endif // CYGOPT_IO_SERIAL_SUPPORT_NONBLOCKING
                    cbuf->pending += size;  // Have this much more to send [eventually]
                    if( !cyg_drv_cond_wait(&cbuf->wait) )
                        cbuf->abort = true;
                    cbuf->pending -= size;
                }
                if (cbuf->abort) {
                    // Give up!
                    *len -= size;   // number of characters actually sent
                    cbuf->abort = false;
                    cbuf->waiting = false;
                    res = -EINTR;
                    break;
                }
            } else {
                cbuf->data[cbuf->put++] = *buf++;
                cbuf->put = next;
                cbuf->nb++;
                size--;  // Only count if actually sent!
            }
        }
#ifdef CYGPKG_IO_SERIAL_FLOW_CONTROL
        if ( 0 == (chan->flow_desc.flags & CYG_SERIAL_FLOW_OUT_THROTTLED) )
#endif
            (funs->start_xmit)(chan);  // Start output as necessary
        cyg_drv_dsr_unlock();
    }
    cyg_drv_mutex_unlock(&cbuf->lock);
    return res;
}


// ---------------------------------------------------------------------------

static Cyg_ErrNo 
serial_read(cyg_io_handle_t handle, void *_buf, cyg_uint32 *len)
{
    cyg_devtab_entry_t *t = (cyg_devtab_entry_t *)handle;
    serial_channel *chan = (serial_channel *)t->priv;
    serial_funs *funs = chan->funs;
    cyg_uint8 *buf = (cyg_uint8 *)_buf;
    cyg_int32 size = 0;
    cbuf_t *cbuf = &chan->in_cbuf;
    Cyg_ErrNo res = ENOERR;
#ifdef XX_CYGDBG_DIAG_BUF
            extern int enable_diag_uart;
            int _enable = enable_diag_uart;
            int _time, _stime;
            externC cyg_tick_count_t cyg_current_time(void);
#endif // CYGDBG_DIAG_BUF

    cyg_drv_mutex_lock(&cbuf->lock);
    cbuf->abort = false;

    if (cbuf->len == 0) {
        // Non interrupt driven (i.e. polled) operation
        while (size++ < *len) {
            cyg_uint8 c = (funs->getc)(chan);
#ifdef CYGOPT_IO_SERIAL_FLOW_CONTROL_SOFTWARE
            // for software flow control, if the driver returns one of the
            // characters we act on it and then drop it (the app must not
            // see it)
            if ( chan->config.flags & CYGNUM_SERIAL_FLOW_XONXOFF_TX ) {
                if ( c == CYGDAT_IO_SERIAL_FLOW_CONTROL_XOFF_CHAR ) {
                    throttle_tx( chan );
                } else if ( c == CYGDAT_IO_SERIAL_FLOW_CONTROL_XON_CHAR ) {
                    restart_tx( chan );
                }
                else
                    *buf++ = c;
            }
            else
                *buf++ = c;
#else
            *buf++ = c;
#endif    
        }
    } else {
        cyg_drv_dsr_lock();  // Avoid races
        while (size < *len) {
            if (cbuf->nb > 0) {
#ifdef CYGPKG_IO_SERIAL_FLOW_CONTROL
                if ( (cbuf->nb <= cbuf->low_water) && 
                     (chan->flow_desc.flags & CYG_SERIAL_FLOW_IN_THROTTLED) )
                    restart_rx( chan, false );
#endif
                *buf++ = cbuf->data[cbuf->get];
                if (++cbuf->get == cbuf->len) cbuf->get = 0;
                cbuf->nb--;
                size++;
            } else {
#ifdef CYGOPT_IO_SERIAL_SUPPORT_NONBLOCKING
                if (!cbuf->blocking) {
                    *len = size;        // characters actually read
                    res = size == 0 ? -EAGAIN : ENOERR;
                    break;
                }
#endif // CYGOPT_IO_SERIAL_SUPPORT_NONBLOCKING
                cbuf->waiting = true;
#ifdef XX_CYGDBG_DIAG_BUF
                enable_diag_uart = 0;
                HAL_CLOCK_READ(&_time);
                _stime = (int)cyg_current_time();
                diag_printf("READ wait - get: %d, put: %d, time: %x.%x\n", cbuf->get, cbuf->put, _stime, _time);
                enable_diag_uart = _enable;
#endif // CYGDBG_DIAG_BUF
                if( !cyg_drv_cond_wait(&cbuf->wait) )
                    cbuf->abort = true;
#ifdef XX_CYGDBG_DIAG_BUF
                enable_diag_uart = 0;
                HAL_CLOCK_READ(&_time);
                _stime = (int)cyg_current_time();
                diag_printf("READ continue - get: %d, put: %d, time: %x.%x\n", cbuf->get, cbuf->put, _stime, _time);
                enable_diag_uart = _enable;
#endif // CYGDBG_DIAG_BUF
                if (cbuf->abort) {
                    // Give up!
                    *len = size;        // characters actually read
                    cbuf->abort = false;
                    cbuf->waiting = false;
                    res = -EINTR;
                    break;
                }
            }
        }
        cyg_drv_dsr_unlock();
    }
#ifdef XX_CYGDBG_DIAG_BUF
    cyg_drv_isr_lock();
    enable_diag_uart = 0;
    HAL_CLOCK_READ(&_time);
    _stime = (int)cyg_current_time();
    diag_printf("READ done - size: %d, len: %d, time: %x.%x\n", size, *len, _stime, _time);
    enable_diag_uart = _enable;
    cyg_drv_isr_unlock();
#endif // CYGDBG_DIAG_BUF
    cyg_drv_mutex_unlock(&cbuf->lock);
    return res;
}


// ---------------------------------------------------------------------------

static cyg_bool
serial_select(cyg_io_handle_t handle, cyg_uint32 which, CYG_ADDRWORD info)
{
#ifdef CYGPKG_IO_SERIAL_SELECT_SUPPORT

    cyg_devtab_entry_t *t = (cyg_devtab_entry_t *)handle;
    serial_channel *chan = (serial_channel *)t->priv;

    
    switch( which )
    {
    case CYG_FREAD:
        {
            cbuf_t *cbuf = &chan->in_cbuf;

            // Check for data in the input buffer. If there is none,
            // register the select operation, otherwise return true.

            if( cbuf->nb == 0 )
                cyg_selrecord( info, &cbuf->selinfo );
            else return true;
        }
        break;
        
    case CYG_FWRITE:
        {
            // Check for space in the output buffer. If there is none,
            // register the select operation, otherwise return true.

            cbuf_t *cbuf = &chan->out_cbuf;
            int space = cbuf->len - cbuf->nb;
#ifdef CYGPKG_IO_SERIAL_FLOW_CONTROL
            if ( (space < cbuf->low_water) ||
                 (chan->flow_desc.flags & CYG_SERIAL_FLOW_OUT_THROTTLED) )
                cyg_selrecord( info, &cbuf->selinfo );
#else
            if (space < cbuf->low_water)
                cyg_selrecord( info, &cbuf->selinfo );
#endif
            else return true;
        }
        break;

    case 0: // exceptions - none supported
        break;
    }
    return false;
#else

    // With no select support, we simply return true.
    return true;
#endif    
}


// ---------------------------------------------------------------------------

static Cyg_ErrNo 
serial_get_config(cyg_io_handle_t handle, cyg_uint32 key, void *xbuf,
                  cyg_uint32 *len)
{
    cyg_devtab_entry_t *t = (cyg_devtab_entry_t *)handle;
    serial_channel *chan = (serial_channel *)t->priv;
    cyg_serial_info_t *buf = (cyg_serial_info_t *)xbuf;
    Cyg_ErrNo res = ENOERR;
    cbuf_t *out_cbuf = &chan->out_cbuf;
    cbuf_t *in_cbuf = &chan->in_cbuf;
    serial_funs *funs = chan->funs;

    switch (key) {
    case CYG_IO_GET_CONFIG_SERIAL_INFO:
        if (*len < sizeof(cyg_serial_info_t)) {
            return -EINVAL;
        }
        *buf = chan->config;
        *len = sizeof(chan->config);
        break;       

    case CYG_IO_GET_CONFIG_SERIAL_BUFFER_INFO:
        // return rx/tx buffer sizes and counts
        {
            cyg_serial_buf_info_t *p;
            if (*len < sizeof(cyg_serial_buf_info_t))
                return -EINVAL;
          
            *len = sizeof(cyg_serial_buf_info_t);
            p = (cyg_serial_buf_info_t *)xbuf;
            
            p->rx_bufsize = in_cbuf->len;
            if (p->rx_bufsize)
                p->rx_count = in_cbuf->nb;
            else
                p->rx_count = 0;
            
            p->tx_bufsize = out_cbuf->len;
            if (p->tx_bufsize)
                p->tx_count = out_cbuf->nb;
            else
                p->tx_count = 0;
        }
      break;
      
    case CYG_IO_GET_CONFIG_SERIAL_OUTPUT_DRAIN:
// Wait for any pending output to complete
        if (out_cbuf->len == 0) break;  // Nothing to do if not buffered
        cyg_drv_mutex_lock(&out_cbuf->lock);  // Stop any further output processing
        cyg_drv_dsr_lock();
        while (out_cbuf->pending || (out_cbuf->nb > 0)) {
            out_cbuf->waiting = true;
            if(!cyg_drv_cond_wait(&out_cbuf->wait) )
                res = -EINTR;
        }
        cyg_drv_dsr_unlock();
        cyg_drv_mutex_unlock(&out_cbuf->lock);
        break;

    case CYG_IO_GET_CONFIG_SERIAL_INPUT_FLUSH:
        // Flush any buffered input
        if (in_cbuf->len == 0) break;  // Nothing to do if not buffered
        cyg_drv_mutex_lock(&in_cbuf->lock);  // Stop any further input processing
        cyg_drv_dsr_lock();
        if (in_cbuf->waiting) {
            in_cbuf->abort = true;
            cyg_drv_cond_broadcast(&in_cbuf->wait);
            in_cbuf->waiting = false;
        }
        in_cbuf->get = in_cbuf->put = in_cbuf->nb = 0;  // Flush buffered input

        // Pass to the hardware driver in case it wants to flush FIFOs etc.
        (funs->set_config)(chan,
                           CYG_IO_SET_CONFIG_SERIAL_INPUT_FLUSH,
                           NULL, NULL);
        cyg_drv_dsr_unlock();
        cyg_drv_mutex_unlock(&in_cbuf->lock);
#ifdef CYGPKG_IO_SERIAL_FLOW_CONTROL
        // Restart receiver if it was shutdown
        if ((chan->flow_desc.flags & CYG_SERIAL_FLOW_IN_THROTTLED) != 0) {
            restart_rx( chan, false );
        }
#endif
        break;

    case CYG_IO_GET_CONFIG_SERIAL_ABORT:
        // Abort any outstanding I/O, including blocked reads
        // Caution - assumed to be called from 'timeout' (i.e. DSR) code
        if (in_cbuf->len != 0) {
            in_cbuf->abort = true;
            cyg_drv_cond_broadcast(&in_cbuf->wait);
        }
        if (out_cbuf->len != 0) {
            out_cbuf->abort = true;
            cyg_drv_cond_broadcast(&out_cbuf->wait);
        }
        break;

    case CYG_IO_GET_CONFIG_SERIAL_OUTPUT_FLUSH:
// Throw away any pending output
        if (out_cbuf->len == 0) break;  // Nothing to do if not buffered
        cyg_drv_mutex_lock(&out_cbuf->lock);  // Stop any further output processing
        cyg_drv_dsr_lock();
        if (out_cbuf->nb > 0) {
            out_cbuf->get = out_cbuf->put = out_cbuf->nb = 0;  // Empties queue!
            (funs->stop_xmit)(chan);  // Done with transmit
        }
        // Pass to the hardware driver in case it wants to flush FIFOs etc.
        (funs->set_config)(chan,
                           CYG_IO_SET_CONFIG_SERIAL_OUTPUT_FLUSH,
                           NULL, NULL);
        if (out_cbuf->waiting) {
            out_cbuf->abort = true;
            cyg_drv_cond_broadcast(&out_cbuf->wait);
            out_cbuf->waiting = false;
        }
        cyg_drv_dsr_unlock();
        cyg_drv_mutex_unlock(&out_cbuf->lock);
        break;

#ifdef CYGOPT_IO_SERIAL_SUPPORT_NONBLOCKING
    case CYG_IO_GET_CONFIG_READ_BLOCKING:
        if (*len < sizeof(cyg_uint32)) {
            return -EINVAL;
        }
        *(cyg_uint32*)xbuf = (in_cbuf->blocking) ? 1 : 0;
        break;

    case CYG_IO_GET_CONFIG_WRITE_BLOCKING:
        if (*len < sizeof(cyg_uint32)) {
            return -EINVAL;
        }
        *(cyg_uint32*)xbuf = (out_cbuf->blocking) ? 1 : 0;
        break;
#endif // CYGOPT_IO_SERIAL_SUPPORT_NONBLOCKING

    default:
        res = -EINVAL;
    }
    return res;
}


// ---------------------------------------------------------------------------

static Cyg_ErrNo 
serial_set_config(cyg_io_handle_t handle, cyg_uint32 key, const void *xbuf, 
                  cyg_uint32 *len)
{
    Cyg_ErrNo res = ENOERR;
    cyg_devtab_entry_t *t = (cyg_devtab_entry_t *)handle;
    serial_channel *chan = (serial_channel *)t->priv;
#ifdef CYGOPT_IO_SERIAL_SUPPORT_NONBLOCKING
    cbuf_t *out_cbuf = &chan->out_cbuf;
    cbuf_t *in_cbuf = &chan->in_cbuf;
#endif
    serial_funs *funs = chan->funs;

    switch (key) {
#ifdef CYGOPT_IO_SERIAL_SUPPORT_NONBLOCKING
    case CYG_IO_SET_CONFIG_READ_BLOCKING:
        if (*len < sizeof(cyg_uint32) || 0 == in_cbuf->len) {
            return -EINVAL;
        }
        in_cbuf->blocking = (1 == *(cyg_uint32*)xbuf) ? true : false;
        break;
    case CYG_IO_SET_CONFIG_WRITE_BLOCKING:
        if (*len < sizeof(cyg_uint32) || 0 == out_cbuf->len) {
            return -EINVAL;
        }
        out_cbuf->blocking = (1 == *(cyg_uint32*)xbuf) ? true : false;
        break;
#endif // CYGOPT_IO_SERIAL_SUPPORT_NONBLOCKING

#ifdef CYGPKG_IO_SERIAL_FLOW_CONTROL
    case CYG_IO_SET_CONFIG_SERIAL_FLOW_CONTROL_METHOD:
        {
            cyg_uint32 *f = (cyg_uint32 *)xbuf;

            if (*len < sizeof(*f))
                return -EINVAL;

            cyg_drv_dsr_lock();

            chan->config.flags &= ~(CYGNUM_SERIAL_FLOW_XONXOFF_RX|
                                    CYGNUM_SERIAL_FLOW_XONXOFF_TX|
                                    CYGNUM_SERIAL_FLOW_RTSCTS_RX|
                                    CYGNUM_SERIAL_FLOW_RTSCTS_TX|
                                    CYGNUM_SERIAL_FLOW_DSRDTR_RX|
                                    CYGNUM_SERIAL_FLOW_DSRDTR_TX);
            chan->config.flags |= (*f & (
#ifdef CYGOPT_IO_SERIAL_FLOW_CONTROL_SOFTWARE
                CYGNUM_SERIAL_FLOW_XONXOFF_RX|
                CYGNUM_SERIAL_FLOW_XONXOFF_TX|
#endif
#ifdef CYGOPT_IO_SERIAL_FLOW_CONTROL_HW
                CYGNUM_SERIAL_FLOW_RTSCTS_RX|
                CYGNUM_SERIAL_FLOW_RTSCTS_TX|
                CYGNUM_SERIAL_FLOW_DSRDTR_RX|
                CYGNUM_SERIAL_FLOW_DSRDTR_TX|
#endif
                0));
#ifdef CYGOPT_IO_SERIAL_FLOW_CONTROL_HW
            // up to hardware driver to clear flags if rejected
            res = (funs->set_config)(chan,
                                     CYG_IO_SET_CONFIG_SERIAL_HW_FLOW_CONFIG,
                                     NULL, NULL);
#endif
            cyg_drv_dsr_unlock();
        }
        break;

    case CYG_IO_SET_CONFIG_SERIAL_FLOW_CONTROL_FORCE:
        {
            cyg_uint32 *f = (cyg_uint32 *)xbuf;

            if (*len < sizeof(*f))
                return -EINVAL;
            
            cyg_drv_dsr_lock();
            switch (*f) {
            case CYGNUM_SERIAL_FLOW_THROTTLE_RX:
                throttle_rx( chan, true );
                break;
            case CYGNUM_SERIAL_FLOW_RESTART_RX:
                restart_rx( chan, true );
                break;
            case CYGNUM_SERIAL_FLOW_THROTTLE_TX:
                throttle_tx( chan );
                break;
            case CYGNUM_SERIAL_FLOW_RESTART_TX:
                restart_tx( chan );
                break;
            default:
                res = -EINVAL;
                break;
            }
            cyg_drv_dsr_unlock();
        }
        break;
#endif // CYGPKG_IO_SERIAL_FLOW_CONTROL

#ifdef CYGOPT_IO_SERIAL_SUPPORT_LINE_STATUS
    case CYG_IO_SET_CONFIG_SERIAL_STATUS_CALLBACK:
        {
            cyg_serial_line_status_callback_fn_t newfn;
            CYG_ADDRWORD newpriv;
            cyg_serial_line_status_callback_t *tmp = 
                (cyg_serial_line_status_callback_t *)xbuf;
            
            if ( *len < sizeof(*tmp) )
                return -EINVAL;

            newfn = tmp->fn;
            newpriv = tmp->priv;

            // prevent callbacks while we do this
            cyg_drv_dsr_lock();
            // store old callbacks in same structure
            tmp->fn = chan->status_callback;
            tmp->priv = chan->status_callback_priv;
            chan->status_callback = newfn;
            chan->status_callback_priv = newpriv;
            cyg_drv_dsr_unlock();
            *len = sizeof(*tmp);
        }  
        break;
#endif

    default:
        // pass down to lower layers
        return (funs->set_config)(chan, key, xbuf, len);
    }
    return res;
}

// ---------------------------------------------------------------------------

static void
serial_xmt_char(serial_channel *chan)
{
    cbuf_t *cbuf = &chan->out_cbuf;
    serial_funs *funs = chan->funs;
    unsigned char c;
    int space;

#if CYGINT_IO_SERIAL_BLOCK_TRANSFER
    CYG_ASSERT(false == cbuf->block_mode_xfer_running,
               "Attempting char xmt while block transfer is running");
#endif
#ifdef CYGOPT_IO_SERIAL_FLOW_CONTROL_SOFTWARE
    // if we are required to send an XON/XOFF char, send it before
    // anything else
    // FIXME: what if XON gets corrupted in transit to the other end?
    // Should we resend XON even though the other end may not be wanting
    // to send us stuff at this point?
    if ( chan->config.flags & CYGNUM_SERIAL_FLOW_XONXOFF_RX ) {
        if ( chan->flow_desc.xchar ) {
            if ( (funs->putc)(chan, chan->flow_desc.xchar) ) {
                chan->flow_desc.xchar = '\0';
            } else {  // otherwise there's no space and we have to wait
                return;
            }
        }
    }
#endif
#ifdef CYGPKG_IO_SERIAL_FLOW_CONTROL
    // if we're meant to be throttled, just stop and leave
    if ( chan->flow_desc.flags & CYG_SERIAL_FLOW_OUT_THROTTLED ) {
        (funs->stop_xmit)(chan);  // Stop transmitting for now
        return;
    }
#endif
    while (cbuf->nb > 0) {
        c = cbuf->data[cbuf->get];
        if ((funs->putc)(chan, c)) {
            cbuf->get++;
            if (cbuf->get == cbuf->len) cbuf->get = 0;
            cbuf->nb--;
        } else {
            // See if there is now enough room to restart writer
            space = cbuf->len - cbuf->nb;
            if (space >= cbuf->low_water) {
                if (cbuf->waiting) {
                    cbuf->waiting = false;
                    cyg_drv_cond_broadcast(&cbuf->wait);
                }
#ifdef CYGPKG_IO_SERIAL_SELECT_SUPPORT
                cyg_selwakeup( &cbuf->selinfo );
#endif                    
            }
            return;  // Need to wait for more space
        }
    }
    (funs->stop_xmit)(chan);  // Done with transmit

    // must signal waiters, and wake up selecters for the case when
    // this was the last char to be sent and they hadn't been signalled
    // before (e.g. because of flow control)
    if (cbuf->waiting) {
        cbuf->waiting = false;
        cyg_drv_cond_broadcast(&cbuf->wait);
    }
#ifdef CYGPKG_IO_SERIAL_SELECT_SUPPORT
    cyg_selwakeup( &cbuf->selinfo );
#endif                    
}

// ---------------------------------------------------------------------------

static void
serial_rcv_char(serial_channel *chan, unsigned char c)
{
    cbuf_t *cbuf = &chan->in_cbuf;

#if CYGINT_IO_SERIAL_BLOCK_TRANSFER
    CYG_ASSERT(false == cbuf->block_mode_xfer_running,
               "Attempting char rcv while block transfer is running");
#endif
#ifdef CYGOPT_IO_SERIAL_FLOW_CONTROL_SOFTWARE
    // for software flow control, if the driver returns one of the characters
    // we act on it and then drop it (the app must not see it)
    if ( chan->config.flags & CYGNUM_SERIAL_FLOW_XONXOFF_TX ) {
        if ( c == CYGDAT_IO_SERIAL_FLOW_CONTROL_XOFF_CHAR ) {
            throttle_tx( chan );
            return; // it wasn't a "real" character
        } else if ( c == CYGDAT_IO_SERIAL_FLOW_CONTROL_XON_CHAR ) {
            restart_tx( chan );
            return; // it wasn't a "real" character
        }
    }
#endif    
#ifdef CYGPKG_IO_SERIAL_FLOW_CONTROL
    // If we've hit the high water mark, tell the other side to stop
    if ( cbuf->nb >= cbuf->high_water ) {
        throttle_rx( chan, false );
    }
#endif
#ifdef CYGPKG_IO_SERIAL_SELECT_SUPPORT
    // Wake up any pending selectors if we are about to
    // put some data into a previously empty buffer.
    if( cbuf->nb == 0 )
        cyg_selwakeup( &cbuf->selinfo );
#endif

    // If the flow control is not enabled/sufficient and the buffer is
    // already full, just throw new characters away.

    if ( cbuf->nb < cbuf->len ) {
        cbuf->data[cbuf->put++] = c;
        if (cbuf->put == cbuf->len) cbuf->put = 0;
        cbuf->nb++;
    } // note trailing else
#ifdef CYGOPT_IO_SERIAL_SUPPORT_LINE_STATUS
    else {
        // Overrun. Report the error.
        cyg_serial_line_status_t stat;
        stat.which = CYGNUM_SERIAL_STATUS_OVERRUNERR;
        serial_indicate_status(chan, &stat);
    }
#endif

    if (cbuf->waiting) {
#ifdef XX_CYGDBG_DIAG_BUF
            extern int enable_diag_uart;
            int _enable = enable_diag_uart;
            int _time, _stime;
            externC cyg_tick_count_t cyg_current_time(void);
            enable_diag_uart = 0;
            HAL_CLOCK_READ(&_time);
            _stime = (int)cyg_current_time();
            diag_printf("Signal reader - time: %x.%x\n", _stime, _time);
            enable_diag_uart = _enable;
#endif // CYGDBG_DIAG_BUF
        cbuf->waiting = false;
        cyg_drv_cond_broadcast(&cbuf->wait);
    }
}

//----------------------------------------------------------------------------
// Flow control indication callback

#ifdef CYGOPT_IO_SERIAL_SUPPORT_LINE_STATUS
static void
serial_indicate_status(serial_channel *chan, cyg_serial_line_status_t *s )
{
#ifdef CYGPKG_IO_SERIAL_FLOW_CONTROL
    if ( CYGNUM_SERIAL_STATUS_FLOW == s->which ) {
        if ( s->value )
            restart_tx( chan );
        else
            throttle_tx( chan );
    }
#endif
    if ( chan->status_callback )
        (*chan->status_callback)(s, chan->status_callback_priv);
}
#endif // ifdef CYGOPT_IO_SERIAL_SUPPORT_LINE_STATUS

//----------------------------------------------------------------------------
// Block transfer functions. Not all drivers require these. Those that
// do must follow the required semantics:
//
// Attempt to transfer as much via the block transfer function as
// possible, _but_ if that fails, do the remaining bytes via the
// single-char function. That ensures that all policy decisions can be
// made in this driver, and not in the device driver.
//
// Note: if the driver uses DMA for transmission, an initial failing
// call to the xmt_req function must cause the start_xmit function to
// fall-back to regular CPU-interrupt based single-character
// transmission.

#if CYGINT_IO_SERIAL_BLOCK_TRANSFER

static rcv_req_reply_t
serial_data_rcv_req(serial_channel *chan, int avail, 
                    int* space_avail, unsigned char** space)
{
    cbuf_t *cbuf = &chan->in_cbuf;
    int gap;

#ifdef CYGOPT_IO_SERIAL_FLOW_CONTROL_SOFTWARE
    // When there is software flow-control, force the serial device
    // driver to use the single-char xmt/rcv functions, since these
    // have to make policy decision based on the data. Rcv function
    // may also have to transmit data to throttle the xmitter.
    if (chan->config.flags & (CYGNUM_SERIAL_FLOW_XONXOFF_TX|CYGNUM_SERIAL_FLOW_XONXOFF_RX))
        return CYG_RCV_DISABLED;
#endif

    CYG_ASSERT(false == cbuf->block_mode_xfer_running,
               "Attempting new block transfer while another is running");
    // Check for space
    gap = cbuf->nb;
    if (gap == cbuf->len)
        return CYG_RCV_FULL;

#ifdef CYGDBG_USE_ASSERTS
    cbuf->block_mode_xfer_running = true;
#endif

    if (0 == gap) {
        // Buffer is empty. Reset put/get indexes to get max transfer in
        // one chunk.
        cbuf->get = 0;
        cbuf->put = 0;
        gap = cbuf->len;
    } else {
        // Free space (G = get, P = put, x = data, . = empty)
        //  positive: xxxxP.....Gxxx
        //  negative: ..GxxxxxP.....        [offer last chunk only]

        // First try for a gap between put and get locations
        gap = cbuf->get - cbuf->put;
        if (gap < 0) {
            // If failed, the gap is between put and the end of buffer
            gap = cbuf->len - cbuf->put;
        }
    }

    if (avail < gap) gap = avail;   // bound by what's available from hw
    
    *space_avail = gap;
    *space = &cbuf->data[cbuf->put];

    CYG_ASSERT((gap+cbuf->nb) <= cbuf->len, "Buffer will overflow");
    CYG_ASSERT(cbuf->put < cbuf->len, "Invalid put ptr");
    CYG_ASSERT(cbuf->get < cbuf->len, "Invalid get ptr");

    return CYG_RCV_OK;
}

static void
serial_data_rcv_done(serial_channel *chan, int chars_rcvd)
{
    cbuf_t *cbuf = &chan->in_cbuf;

    cbuf->put += chars_rcvd;
    cbuf->nb += chars_rcvd;

    if (cbuf->put == cbuf->len) cbuf->put = 0;

    CYG_ASSERT(cbuf->nb <= cbuf->len, "Buffer overflow");
    CYG_ASSERT(cbuf->put < cbuf->len, "Invalid put ptr");
    CYG_ASSERT(cbuf->get < cbuf->len, "Invalid get ptr");

    if (cbuf->waiting) {
        cbuf->waiting = false;
        cyg_drv_cond_broadcast(&cbuf->wait);
    }
#ifdef CYGPKG_IO_SERIAL_FLOW_CONTROL
    // If we've hit the high water mark, tell the other side to stop
    if ( cbuf->nb >= cbuf->high_water ) {
        throttle_rx( chan, false );
    }
#endif
#ifdef CYGPKG_IO_SERIAL_SELECT_SUPPORT
    // Wake up any pending selectors if we have
    // put some data into a previously empty buffer.
    if (chars_rcvd == cbuf->nb)
        cyg_selwakeup( &cbuf->selinfo );
#endif

#ifdef CYGDBG_USE_ASSERTS
    cbuf->block_mode_xfer_running = false;
#endif
}

static xmt_req_reply_t
serial_data_xmt_req(serial_channel *chan, int space,
                    int* chars_avail, unsigned char** chars)
{
    cbuf_t *cbuf = &chan->out_cbuf;
    int avail;

#ifdef CYGOPT_IO_SERIAL_FLOW_CONTROL_SOFTWARE
    // When there is software flow-control, force the serial device
    // driver to use the single-char xmt/rcv functions, since these
    // have to make policy decision based on the data. Rcv function
    // may also have to transmit data to throttle the xmitter.
    if (chan->config.flags & (CYGNUM_SERIAL_FLOW_XONXOFF_TX|CYGNUM_SERIAL_FLOW_XONXOFF_RX))
        return CYG_XMT_DISABLED;
#endif

    CYG_ASSERT(false == cbuf->block_mode_xfer_running,
               "Attempting new block transfer while another is running");

#ifdef CYGPKG_IO_SERIAL_FLOW_CONTROL
    // if we're meant to be throttled, just stop and leave
    if ( chan->flow_desc.flags & CYG_SERIAL_FLOW_OUT_THROTTLED ) {
        (chan->funs->stop_xmit)(chan);  // Stop transmitting for now
        return CYG_XMT_EMPTY;
    }
#endif

    // Available data (G = get, P = put, x = data, . = empty)
    //  0:        no data
    //  negative: xxxxP.....Gxxx        [offer last chunk only]
    //  positive: ..GxxxxxP.....
    if (0 == cbuf->nb)
        return CYG_XMT_EMPTY;

#ifdef CYGDBG_USE_ASSERTS
    cbuf->block_mode_xfer_running = true;
#endif

    if (cbuf->get >= cbuf->put) {
        avail = cbuf->len - cbuf->get;
    } else {
        avail = cbuf->put - cbuf->get;
    }

    if (avail > space) avail = space;   // bound by space in hardware
    
    *chars_avail = avail;
    *chars = &cbuf->data[cbuf->get];

    CYG_ASSERT(avail <= cbuf->len, "Avail overflow");
    CYG_ASSERT(cbuf->nb <= cbuf->len, "Buffer overflow");
    CYG_ASSERT(cbuf->put < cbuf->len, "Invalid put ptr");
    CYG_ASSERT(cbuf->get < cbuf->len, "Invalid get ptr");

    return CYG_XMT_OK;
}

static void
serial_data_xmt_done(serial_channel *chan, int chars_sent)
{
    cbuf_t *cbuf = &chan->out_cbuf;
    serial_funs *funs = chan->funs;
    int space;

    cbuf->get += chars_sent;
    cbuf->nb -= chars_sent;

    if (cbuf->get == cbuf->len) cbuf->get = 0;

    CYG_ASSERT(cbuf->nb <= cbuf->len, "Buffer overflow");
    CYG_ASSERT(cbuf->nb >= 0, "Buffer underflow");
    CYG_ASSERT(cbuf->put < cbuf->len, "Invalid put ptr");
    CYG_ASSERT(cbuf->get < cbuf->len, "Invalid get ptr");

    if (0 == cbuf->nb) {
        (funs->stop_xmit)(chan);  // Done with transmit
        cbuf->get = cbuf->put = 0; // reset ptrs if empty
    }

    // See if there is now enough room to restart writer
    space = cbuf->len - cbuf->nb;
    if (space >= cbuf->low_water) {
        if (cbuf->waiting) {
            cbuf->waiting = false;
            cyg_drv_cond_broadcast(&cbuf->wait);
        }
#ifdef CYGPKG_IO_SERIAL_SELECT_SUPPORT
        cyg_selwakeup( &cbuf->selinfo );
#endif                    
    }

#ifdef CYGDBG_USE_ASSERTS
    cbuf->block_mode_xfer_running = false;
#endif
}

#endif // CYGINT_IO_SERIAL_BLOCK_TRANSFER

// ---------------------------------------------------------------------------

// EOF serial.c
