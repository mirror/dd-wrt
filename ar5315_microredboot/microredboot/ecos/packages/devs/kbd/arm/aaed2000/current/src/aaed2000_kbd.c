//==========================================================================
//
//      aaed2000_kbd.c
//
//      Keyboard driver for the Agilent AAED2000
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
// Date:         2002-03-11
// Purpose:      
// Description:  Keyboardd driver for Agilent AAED2000
//
//####DESCRIPTIONEND####
//
//==========================================================================


#include <pkgconf/devs_kbd_aaed2000.h>

#include <cyg/kernel/kapi.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/drv_api.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/aaed2000.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>

#include <cyg/fileio/fileio.h>  // For select() functionality
static cyg_selinfo      kbd_select_info; 
static cyg_bool         kbd_select_active;

#include <cyg/io/devtab.h>

// Functions in this module

static Cyg_ErrNo kbd_read(cyg_io_handle_t handle, 
                          void *buffer, 
                          cyg_uint32 *len);
static cyg_bool  kbd_select(cyg_io_handle_t handle, 
                            cyg_uint32 which, 
                            cyg_addrword_t info);
static Cyg_ErrNo kbd_set_config(cyg_io_handle_t handle, 
                                cyg_uint32 key, 
                                const void *buffer, 
                                cyg_uint32 *len);
static Cyg_ErrNo kbd_get_config(cyg_io_handle_t handle, 
                                cyg_uint32 key, 
                                void *buffer, 
                                cyg_uint32 *len);
static bool      kbd_init(struct cyg_devtab_entry *tab);
static Cyg_ErrNo kbd_lookup(struct cyg_devtab_entry **tab, 
                            struct cyg_devtab_entry *st, 
                            const char *name);

CHAR_DEVIO_TABLE(aaed2000_kbd_handlers,
                 NULL,                                   // Unsupported write() function
                 kbd_read,
                 kbd_select,
                 kbd_get_config,
                 kbd_set_config);

CHAR_DEVTAB_ENTRY(aaed2000_kbd_device,
                  CYGDAT_DEVS_KBD_AAED2000_NAME,
                  NULL,                                   // Base device name
                  &aaed2000_kbd_handlers,
                  kbd_init,
                  kbd_lookup,
                  NULL);                                  // Private data pointer

#define MAX_EVENTS CYGNUM_DEVS_KBD_AAED2000_EVENT_BUFFER_SIZE
static int   num_events;
static int   _event_put, _event_get;
static unsigned char _events[MAX_EVENTS];

static bool _is_open = false;

#define STACK_SIZE CYGNUM_HAL_STACK_SIZE_TYPICAL
static char kbd_scan_stack[STACK_SIZE];
static cyg_thread kbd_scan_thread_data;
static cyg_handle_t kbd_scan_thread_handle;
#define SCAN_FREQ 20 // Hz
//#define SCAN_FREQ 5 // Hz
#define SCAN_DELAY ((1000/SCAN_FREQ)/10)

static void
kbd_scan(cyg_addrword_t param)
{
    unsigned char ch;
    unsigned char *ev;

    diag_printf("Keyboard scan\n");
    while (true) {
        cyg_thread_delay(SCAN_DELAY);
        if (aaed2000_KeyboardTest()) {
            ch = aaed2000_KeyboardGetc();
            if (num_events < MAX_EVENTS) {
                num_events++;
                ev = &_events[_event_put++];
                if (_event_put == MAX_EVENTS) {
                    _event_put = 0;
                }
                *ev = ch;
                if (kbd_select_active) {
                    kbd_select_active = false;
                    cyg_selwakeup(&kbd_select_info);
                }
            }
        }
    }
}

static Cyg_ErrNo 
kbd_read(cyg_io_handle_t handle, 
         void *buffer, 
         cyg_uint32 *len)
{
    unsigned char *ev;
    int tot = *len;
    unsigned char *bp = (unsigned char *)buffer;

    cyg_scheduler_lock();  // Prevent interaction with DSR code
    while (tot >= sizeof(*ev)) {
        if (num_events > 0) {
            ev = &_events[_event_get++];
            if (_event_get == MAX_EVENTS) {
                _event_get = 0;
            }
            memcpy(bp, ev, sizeof(*ev));
            bp += sizeof(*ev);
            tot -= sizeof(*ev);
            num_events--;
        } else {
            break;  // No more events
        }
    }
    cyg_scheduler_unlock(); // Allow DSRs again
    diag_dump_buf(buffer, tot);
    *len -= tot;
    return ENOERR;
}

static cyg_bool  
kbd_select(cyg_io_handle_t handle, 
           cyg_uint32 which, 
           cyg_addrword_t info)
{
    if (which == CYG_FREAD) {
        cyg_scheduler_lock();  // Prevent interaction with DSR code
        if (num_events > 0) {
            cyg_scheduler_unlock();  // Reallow interaction with DSR code
            return true;
        }        
        if (!kbd_select_active) {
            kbd_select_active = true;
            cyg_selrecord(info, &kbd_select_info);
        }
        cyg_scheduler_unlock();  // Reallow interaction with DSR code
    }
    return false;
}

static Cyg_ErrNo 
kbd_set_config(cyg_io_handle_t handle, 
               cyg_uint32 key, 
               const void *buffer, 
               cyg_uint32 *len)
{
    return EINVAL;
}

static Cyg_ErrNo 
kbd_get_config(cyg_io_handle_t handle, 
               cyg_uint32 key, 
               void *buffer, 
               cyg_uint32 *len)
{
    return EINVAL;
}

static bool      
kbd_init(struct cyg_devtab_entry *tab)
{
    cyg_selinit(&kbd_select_info);
    return true;
}

static Cyg_ErrNo 
kbd_lookup(struct cyg_devtab_entry **tab, 
           struct cyg_devtab_entry *st, 
           const char *name)
{
    if (!_is_open) {
        _is_open = true;
        cyg_thread_create(1,                       // Priority
                          kbd_scan,                // entry
                          0,                       // entry parameter
                          "Keyboard scan",         // Name
                          &kbd_scan_stack[0],      // Stack
                          STACK_SIZE,              // Size
                          &kbd_scan_thread_handle, // Handle
                          &kbd_scan_thread_data    // Thread data structure
        );
        cyg_thread_resume(kbd_scan_thread_handle);    // Start it
    }
    return ENOERR;
}



