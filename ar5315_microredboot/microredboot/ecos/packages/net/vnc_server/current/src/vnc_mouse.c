//==========================================================================
//
//      vnc_mouse.c
//
//
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.
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
// Author(s):    Chris Garry <cgarry@sweeneydesign.co.uk>
// Contributors:
// Date:         2003-08-22
// Purpose:
// Description:  mouse driver for the VNC serevr
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <cyg/kernel/kapi.h>
#include <cyg/infra/diag.h>   /* diag_printf */
#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/drv_api.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/fileio/fileio.h>  /*select() functionality */
#include <cyg/io/devtab.h>
#include <pkgconf/vnc_server.h>

/* Local function prototypes */
static Cyg_ErrNo mouse_read(cyg_io_handle_t handle,
                         void *buffer,
                         cyg_uint32 *len);
static cyg_bool  mouse_select(cyg_io_handle_t handle,
                           cyg_uint32 which,
                           cyg_addrword_t info);
static Cyg_ErrNo mouse_set_config(cyg_io_handle_t handle,
                               cyg_uint32 key,
                               const void *buffer,
                               cyg_uint32 *len);
static Cyg_ErrNo mouse_get_config(cyg_io_handle_t handle,
                               cyg_uint32 key,
                               void *buffer,
                               cyg_uint32 *len);
static bool      mouse_init(struct cyg_devtab_entry *tab);
static Cyg_ErrNo mouse_lookup(struct cyg_devtab_entry **tab,
                              struct cyg_devtab_entry *st,
                              const char *name);

CHAR_DEVIO_TABLE(vnc_mouse_handlers,
                 NULL,                    /* Unsupported write() function */
                 mouse_read,
                 mouse_select,
                 mouse_get_config,
                 mouse_set_config);

CHAR_DEVTAB_ENTRY(vnc_mouse_device,
                  CYGDAT_VNC_SERVER_MOUSE_NAME,
                  NULL,                   /* Base device name */
                  &vnc_mouse_handlers,
                  mouse_init,
                  mouse_lookup,
                  NULL);                  /* Private data pointer */

#define MAX_EVENTS CYGNUM_VNC_SERVER_MOUSE_EVENTS
static cyg_uint8 data_queue[MAX_EVENTS*8];
static int queue_put_ptr = 0;
static int queue_get_ptr = 0;
static int data_in_queue = 0;

static cyg_selinfo      mouse_select_info;
static cyg_bool         mouse_select_active;
static int mouse_active = 0;


/*
 * Mouse event handler routine - called by main VNC server loop
 * when new mouse data is received from client
 */
void vnc_mouse_handler(cyg_uint8 *data)
{
    if (!mouse_active)
    {
        /* Mouse is not active yet - ignore incoming data */
        return;
    }

    /* Add the single mouse event data to the data queue */
    cyg_scheduler_lock();

    if (data_in_queue >= (MAX_EVENTS*8))
    {
        diag_printf("VNC mouse driver buffer overflow\n");
        cyg_scheduler_unlock();
        return;
    }

    data_queue[queue_put_ptr]   = 0;        /* Button info MSB */
    data_queue[queue_put_ptr+1] = data[1];  /* Button info LSB */
    data_queue[queue_put_ptr+2] = data[2];  /* x-coord MSB */
    data_queue[queue_put_ptr+3] = data[3];  /* x-coord LSB */
    data_queue[queue_put_ptr+4] = data[4];  /* y-coord MSB */
    data_queue[queue_put_ptr+5] = data[5];  /* y-coord LSB */
    data_queue[queue_put_ptr+6] = 0;
    data_queue[queue_put_ptr+7] = 0;

    /* Increment pointer */
    queue_put_ptr +=8;
    if (queue_put_ptr >= (MAX_EVENTS*8))
    {
        queue_put_ptr = 0;
    }

    /* Increment count of data in queue */
    data_in_queue += 8;

    /* Wake up select() */
    if (mouse_select_active) {
        mouse_select_active = false;
        cyg_selwakeup(&mouse_select_info);
    }

    cyg_scheduler_unlock();
}


static Cyg_ErrNo mouse_read(cyg_io_handle_t handle,
                            void *buffer,
                            cyg_uint32 *len)
{
    int total;  /* Total bytes to read */
    cyg_uint8 *bp = (unsigned char *)buffer;
    int i;

    cyg_scheduler_lock();  /* Prevent interaction with mouse_handler funcion */

    /* Each mouse event generates 8 x cyg_uint8 values in the queue
     *   0-1: Mouse button data
     *   2-3: x-coord of mouse
     *   4-5: y-coord of mouse
     *   6-7: always zero

     */
    if (*len <= data_in_queue)
    {
        /* Send all requested data */
        total = *len;
    }
    else
    {
        /* read() is requesting more data than there is in the queue */
        /* Send all data from the queue */
        total = data_in_queue;
    }

    for (i = 0; i < total; i++)
    {
        bp[i] = data_queue[queue_get_ptr];
        if (++queue_get_ptr >= (MAX_EVENTS*8))
        {
            queue_get_ptr = 0;
        }
    }

    data_in_queue -= total;

    cyg_scheduler_unlock(); // Allow DSRs again

    *len = total;
    return ENOERR;
}


static cyg_bool mouse_select(cyg_io_handle_t handle,
                             cyg_uint32 which,
                             cyg_addrword_t info)
{
    if (which == CYG_FREAD) {
        cyg_scheduler_lock();
        if (data_in_queue >= 8) {
            cyg_scheduler_unlock();
            return true;
        }

        if (!mouse_select_active) {
            mouse_select_active = true;
            cyg_selrecord(info, &mouse_select_info);
        }

        cyg_scheduler_unlock();
    }
    return false;
}


static Cyg_ErrNo mouse_set_config(cyg_io_handle_t handle,
              cyg_uint32 key,
              const void *buffer,
              cyg_uint32 *len)
{
    return EINVAL;
}


static Cyg_ErrNo mouse_get_config(cyg_io_handle_t handle,
              cyg_uint32 key,
              void *buffer,
              cyg_uint32 *len)
{
    return EINVAL;
}


static bool mouse_init(struct cyg_devtab_entry *tab)
{
    cyg_selinit(&mouse_select_info);
    return true;
}


static Cyg_ErrNo mouse_lookup(struct cyg_devtab_entry **tab,
          struct cyg_devtab_entry *st,
          const char *name)
{
    mouse_active = 1;
    return ENOERR;
}
