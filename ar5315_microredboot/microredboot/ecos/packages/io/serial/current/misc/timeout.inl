#ifndef CYGONCE_IO_SERIAL_MISC_TIMEOUT_INL
#define CYGONCE_IO_SERIAL_MISC_TIMEOUT_INL
//==========================================================================
//
//        timeout.inl
//
//        Simple timeout support for serial I/O testing.
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
// Author(s):     gthomas
// Contributors:  gthomas
// Date:          1999-02-05
// Description:   Simple timeout functions
//####DESCRIPTIONEND####

// Timeout support

typedef void (timeout_fun)(void *);
#ifndef NTIMEOUTS
#define NTIMEOUTS 8
#endif
typedef struct {
    cyg_int32     delta;  // Number of "ticks" in the future for this timeout
    timeout_fun  *fun;    // Function to execute when it expires
    void         *arg;    // Argument to pass when it does
} timeout_entry;
static timeout_entry timeouts[NTIMEOUTS];
static cyg_handle_t timeout_alarm_handle;
static cyg_alarm timeout_alarm;
static cyg_int32 last_delta;

static void
do_timeout(cyg_handle_t alarm, cyg_addrword_t data)
{
    int i;
    cyg_int32 min_delta;
    timeout_entry *e = timeouts;
    min_delta = 0x7FFFFFFF;  // Maxint
    for (i = 0;  i < NTIMEOUTS;  i++, e++) {
        if (e->delta) {
            e->delta -= last_delta;
            if (e->delta == 0) {
                // Time for this item to 'fire'
                (e->fun)(e->arg);
            } else {
                if (e->delta < min_delta) min_delta = e->delta;
            }
        }
    }
    if (min_delta != 0x7FFFFFFF) {
        // Still something to do, schedule it
        cyg_alarm_initialize(timeout_alarm_handle, cyg_current_time()+min_delta, 0);
        last_delta = min_delta;
    }
}

static cyg_uint32
timeout(cyg_int32 delta, timeout_fun *fun, void *arg)
{
    int i;
    cyg_int32 min_delta;
    static bool init = false;
    timeout_entry *e = timeouts;
    cyg_uint32 stamp;
    if (!init) {
        cyg_handle_t h;
        cyg_clock_to_counter(cyg_real_time_clock(), &h);
        cyg_alarm_create(h, do_timeout, 0, &timeout_alarm_handle, &timeout_alarm);
        init = true;
    }
    stamp = 0;  // Assume no slots available
    for (i = 0;  i < NTIMEOUTS;  i++, e++) {
        if ((e->delta == 0) && (e->fun == 0)) {
            // Free entry
            e->delta = delta;
            e->fun = fun;
            e->arg = arg;
            stamp = (cyg_uint32)e;
            break;
        }
    }
    e = timeouts;
    min_delta = 0x7FFFFFFF;
    for (i = 0;  i < NTIMEOUTS;  i++, e++) {
        if (e->delta && (e->delta < min_delta)) min_delta = e->delta;
    }
    if (min_delta != 0x7FFFFFFF) {
        // Still something to do, schedule it
        cyg_alarm_initialize(timeout_alarm_handle, cyg_current_time()+min_delta, 0);
        last_delta = min_delta;
    }
    return stamp;
}

static void
untimeout(cyg_uint32 stamp)
{
    if (stamp != 0) {
        timeout_entry *e = (timeout_entry *)stamp;
        if (e->fun != 0) {
            e->delta = 0;
            e->fun = 0;
            e->arg = 0;
        }
    }
}

#endif // CYGONCE_IO_SERIAL_MISC_TIMEOUT_INL
