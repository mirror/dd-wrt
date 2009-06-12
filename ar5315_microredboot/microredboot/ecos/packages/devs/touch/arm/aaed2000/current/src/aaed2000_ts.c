//==========================================================================
//
//      aaed2000_ts.c
//
//      Touchscreen driver for the Agilent aaed2000
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
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2002-03-05
// Purpose:      
// Description:  Touchscreen driver for Agilent AAED2000
//
//####DESCRIPTIONEND####
//
//==========================================================================


#include <pkgconf/devs_touch_aaed2000.h>

#include <cyg/kernel/kapi.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/drv_api.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/aaed2000.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/diag.h>

#include <cyg/fileio/fileio.h>  // For select() functionality
static cyg_selinfo      ts_select_info; 
static cyg_bool         ts_select_active;

#include <cyg/io/devtab.h>

/* ADS7846 flags */
#define ADS_START               (1 << 7)
#define ADS_MEASURE_Y           (0x01 << 4)
#define ADS_MEASURE_X           (0x05 << 4)
#define ADS_MODE_12_BIT         0
#define ADS_PD0                 0

// Misc constants
#define TS_INT (1<<0)
#define X_THRESHOLD 0x80
#define Y_THRESHOLD 0x80

// Functions in this module

static Cyg_ErrNo ts_read(cyg_io_handle_t handle, 
                         void *buffer, 
                         cyg_uint32 *len);
static cyg_bool  ts_select(cyg_io_handle_t handle, 
                           cyg_uint32 which, 
                           cyg_addrword_t info);
static Cyg_ErrNo ts_set_config(cyg_io_handle_t handle, 
                               cyg_uint32 key, 
                               const void *buffer, 
                               cyg_uint32 *len);
static Cyg_ErrNo ts_get_config(cyg_io_handle_t handle, 
                               cyg_uint32 key, 
                               void *buffer, 
                               cyg_uint32 *len);
static bool      ts_init(struct cyg_devtab_entry *tab);
static Cyg_ErrNo ts_lookup(struct cyg_devtab_entry **tab, 
                           struct cyg_devtab_entry *st, 
                           const char *name);

CHAR_DEVIO_TABLE(aaed2000_ts_handlers,
                 NULL,                                   // Unsupported write() function
                 ts_read,
                 ts_select,
                 ts_get_config,
                 ts_set_config);

CHAR_DEVTAB_ENTRY(aaed2000_ts_device,
                  CYGDAT_DEVS_TOUCH_AAED2000_NAME,
                  NULL,                                   // Base device name
                  &aaed2000_ts_handlers,
                  ts_init,
                  ts_lookup,
                  NULL);                                  // Private data pointer

struct _event {
    short button_state;
    short xPos, yPos;
    short _unused;
};
#define MAX_EVENTS CYGNUM_DEVS_TOUCH_AAED2000_EVENT_BUFFER_SIZE
static int   num_events;
static int   _event_put, _event_get;
static struct _event _events[MAX_EVENTS];

static bool _is_open = false;
#ifdef DEBUG_RAW_EVENTS
static unsigned char _ts_buf[512];
static int _ts_buf_ptr = 0;
#endif

#define STACK_SIZE CYGNUM_HAL_STACK_SIZE_TYPICAL
static char ts_scan_stack[STACK_SIZE];
static cyg_thread ts_scan_thread_data;
static cyg_handle_t ts_scan_thread_handle;
#define SCAN_FREQ 20 // Hz
//#define SCAN_FREQ 5 // Hz
#define SCAN_DELAY ((1000/SCAN_FREQ)/10)


typedef struct {
    short min;
    short max;
    short span;
} bounds;

static bounds xBounds = {1024, 0, 1024};
static bounds yBounds = {1024, 0, 1024};

static Cyg_ErrNo 
ts_read(cyg_io_handle_t handle, 
        void *buffer, 
        cyg_uint32 *len)
{
    struct _event *ev;
    int tot = *len;
    unsigned char *bp = (unsigned char *)buffer;

    cyg_scheduler_lock();  // Prevent interaction with DSR code
    while (tot >= sizeof(struct _event)) {
        if (num_events > 0) {
            ev = &_events[_event_get++];
            if (_event_get == MAX_EVENTS) {
                _event_get = 0;
            }
            // Self calibrate
            if (ev->xPos > xBounds.max) xBounds.max = ev->xPos;
            if (ev->xPos < xBounds.min) xBounds.min = ev->xPos;
            if (ev->yPos > yBounds.max) yBounds.max = ev->yPos;
            if (ev->yPos < yBounds.min) yBounds.min = ev->yPos;
            if ((xBounds.span = xBounds.max - xBounds.min) <= 1) {
                xBounds.span = 1;
            }
            if ((yBounds.span = yBounds.max - yBounds.min) <= 1) {
                yBounds.span = 1;
            }
            // Scale values - done here so these potentially lengthy
            // operations take place outside of interrupt processing
#ifdef DEBUG
            diag_printf("Raw[%d,%d], X[%d,%d,%d], Y[%d,%d,%d]",
                        ev->xPos, ev->yPos,
                        xBounds.max, xBounds.min, xBounds.span,
                        yBounds.max, yBounds.min, yBounds.span);
#endif
            ev->xPos = 640 - (((xBounds.max - ev->xPos) * 640) / xBounds.span);
            ev->yPos = 480 - (((yBounds.max - ev->yPos) * 480) / yBounds.span);
#ifdef DEBUG
            diag_printf(", Cooked[%d,%d]\n",
                        ev->xPos, ev->yPos);
#endif
            memcpy(bp, ev, sizeof(*ev));
            bp += sizeof(*ev);
            tot -= sizeof(*ev);
            num_events--;
        } else {
            break;  // No more events
        }
    }
    cyg_scheduler_unlock(); // Allow DSRs again
    *len -= tot;
    return ENOERR;
}

static cyg_bool  
ts_select(cyg_io_handle_t handle, 
          cyg_uint32 which, 
          cyg_addrword_t info)
{
    if (which == CYG_FREAD) {
        cyg_scheduler_lock();  // Prevent interaction with DSR code
        if (num_events > 0) {
            cyg_scheduler_unlock();  // Reallow interaction with DSR code
            return true;
        }        
        if (!ts_select_active) {
            ts_select_active = true;
            cyg_selrecord(info, &ts_select_info);
        }
        cyg_scheduler_unlock();  // Reallow interaction with DSR code
    }
    return false;
}

static Cyg_ErrNo 
ts_set_config(cyg_io_handle_t handle, 
              cyg_uint32 key, 
              const void *buffer, 
              cyg_uint32 *len)
{
    return EINVAL;
}

static Cyg_ErrNo 
ts_get_config(cyg_io_handle_t handle, 
              cyg_uint32 key, 
              void *buffer, 
              cyg_uint32 *len)
{
    return EINVAL;
}

static bool      
ts_init(struct cyg_devtab_entry *tab)
{
    cyg_uint32 _dummy;

    // Initialize SSP interface
#if 0
    while (*(volatile cyg_uint32 *)AAEC_SSP_SR & AAEC_SSP_SR_RNE) {
        _dummy = *(volatile cyg_uint32 *)AAEC_SSP_DR;  // Drain FIFO
    }
#endif
    *(volatile cyg_uint32 *)AAEC_SSP_CR0 = 
        (1 << AAEC_SSP_CR0_SSE) |                    // SSP enable
        (37 << AAEC_SSP_CR0_SCR) |                   // Serial clock rate
        (AAEC_SSP_CR0_FRF_NAT << AAEC_SSP_CR0_FRF) | // MicroWire
        ((12-1) << AAEC_SSP_CR0_SIZE);               // 12 bit words
    *(volatile cyg_uint32 *)AAEC_SSP_CR1 =
        (1 << AAEC_SSP_CR1_FEN);                     // Enable FIFO
    *(volatile cyg_uint32 *)AAEC_SSP_CPSR = 2;       // Clock prescale
    *(volatile cyg_uint32 *)AAEC_PFDDR &= ~(1<<0);  // TS uses port F bit 0
    cyg_drv_interrupt_acknowledge(CYGNUM_HAL_INTERRUPT_TS);
    cyg_selinit(&ts_select_info);
    return true;
}

static cyg_uint32
read_ts(int axis)
{
    cyg_uint32 res;

    *(volatile cyg_uint32 *)AAEC_SSP_DR = (axis | ADS_START | ADS_MODE_12_BIT | ADS_PD0);
    *(volatile cyg_uint32 *)AAEC_SSP_DR = (axis | ADS_START | ADS_MODE_12_BIT);
    // Wait for data
    while ((*(volatile cyg_uint32 *)AAEC_SSP_SR & AAEC_SSP_SR_RNE) == 0); 
    res = *(volatile cyg_uint32 *)AAEC_SSP_DR;  // ignore first datum
    // Wait for data
    while ((*(volatile cyg_uint32 *)AAEC_SSP_SR & AAEC_SSP_SR_RNE) == 0); 
    res = *(volatile cyg_uint32 *)AAEC_SSP_DR;
    return res;
}

static void
ts_scan(cyg_addrword_t param)
{
    short lastX, lastY;
    short x, y;
    struct _event *ev;
    bool pen_down;

    diag_printf("Touch Screen thread started\n");
    // Discard the first sample - it's always 0
    x = read_ts(ADS_MEASURE_X);
    y = read_ts(ADS_MEASURE_Y);
    lastX = lastY = -1;
    pen_down = false;
    while (true) {
        cyg_thread_delay(SCAN_DELAY);
        if ((*(volatile cyg_uint32 *)AAEC_PFDR & TS_INT) == 0) {
            // Pen is down
            x = read_ts(ADS_MEASURE_X);
            y = read_ts(ADS_MEASURE_Y);
//            diag_printf("X = %x, Y = %x\n", x, y);
            if ((x < X_THRESHOLD) || (y < Y_THRESHOLD)) {
                // Ignore 'bad' samples
                continue;
            }
            lastX = x;  lastY = y;
            pen_down = true;
        } else {
            if (pen_down) {
                // Capture first 'up' event
                pen_down = false;
                x = lastX;
                y = lastY;
            } else {
                continue;  // Nothing new to report
            }
        }
        if (num_events < MAX_EVENTS) {
            num_events++;
            ev = &_events[_event_put++];
            if (_event_put == MAX_EVENTS) {
                _event_put = 0;
            }
            ev->button_state = pen_down ? 0x04 : 0x00;
            ev->xPos = x;
            ev->yPos = y;
            if (ts_select_active) {
                ts_select_active = false;
                cyg_selwakeup(&ts_select_info);
            }
        }
#ifdef DEBUG_RAW_EVENTS
        memcpy(&_ts_buf[_ts_buf_ptr], pkt->data, 8);
        _ts_buf_ptr += 8;
        if (_ts_buf_ptr == 512) {
            diag_printf("TS handler\n");
            diag_dump_buf(_ts_buf, 512);
            _ts_buf_ptr = 0;
        }
#endif
    }
}

static Cyg_ErrNo 
ts_lookup(struct cyg_devtab_entry **tab, 
          struct cyg_devtab_entry *st, 
          const char *name)
{
    if (!_is_open) {
        _is_open = true;
        cyg_thread_create(1,                      // Priority
                          ts_scan,                // entry
                          0,                      // entry parameter
                          "Touch Screen scan",    // Name
                          &ts_scan_stack[0],      // Stack
                          STACK_SIZE,             // Size
                          &ts_scan_thread_handle, // Handle
                          &ts_scan_thread_data    // Thread data structure
        );
        cyg_thread_resume(ts_scan_thread_handle);    // Start it
    }
    return ENOERR;
}
