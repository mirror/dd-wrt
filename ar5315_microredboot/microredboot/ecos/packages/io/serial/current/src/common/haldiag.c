//==========================================================================
//
//      io/serial/common/haldiag.c
//
//      Serial I/O interface module using HAL I/O routines
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
// Author(s):   gthomas
// Contributors:  gthomas
// Date:        1999-02-04
// Purpose:     HAL/diag serial driver
// Description: 
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/io.h>
#include <pkgconf/io_serial.h>
#ifdef CYGPKG_IO_SERIAL_HALDIAG
#include <cyg/io/io.h>
#include <cyg/io/devtab.h>
#include <cyg/io/serial.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/hal_diag.h>

static bool haldiag_init(struct cyg_devtab_entry *tab);
static bool haldiag_putc(serial_channel *chan, unsigned char c);
static unsigned char haldiag_getc(serial_channel *chan);
static Cyg_ErrNo haldiag_set_config(serial_channel *chan, cyg_uint32 key,
                                    const void *xbuf, cyg_uint32 *len);

static SERIAL_FUNS(haldiag_funs, 
                   haldiag_putc, 
                   haldiag_getc,
                   haldiag_set_config,
                   0,                      // start xmit - not used
                   0                       // stop xmit - not used
    );

static int _no_data;
static SERIAL_CHANNEL(haldiag_channel0,
               haldiag_funs, 
               _no_data,
               CYG_SERIAL_BAUD_DEFAULT,
               CYG_SERIAL_STOP_DEFAULT,
               CYG_SERIAL_PARITY_DEFAULT,
               CYG_SERIAL_WORD_LENGTH_DEFAULT,
               CYG_SERIAL_FLAGS_DEFAULT
    );
DEVTAB_ENTRY(haldiag_io0, 
             "/dev/haldiag",
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             haldiag_init, 
             0,                     // No initialization/lookup needed
             &haldiag_channel0);

static void
haldiag_config_port(serial_channel *chan)
{
}

static bool 
haldiag_init(struct cyg_devtab_entry *tab)
{
    serial_channel *chan = (serial_channel *)tab->priv;
#ifdef CYGDBG_IO_INIT
    diag_printf("HAL/diag SERIAL init\n");
#endif
    haldiag_config_port(chan);
    return true;
}

// Return 'true' if character is sent to device
static bool
haldiag_putc(serial_channel *chan, unsigned char c)
{
    HAL_DIAG_WRITE_CHAR(c);
    return true;
}

static unsigned char 
haldiag_getc(serial_channel *chan)
{
    unsigned char c;
    HAL_DIAG_READ_CHAR(c);
    return c;
}

static Cyg_ErrNo
haldiag_set_config(serial_channel *chan, cyg_uint32 key, const void *xbuf,
                   cyg_uint32 *len)
{
    switch (key) {
    case CYG_IO_SET_CONFIG_SERIAL_INFO:
        diag_printf("%s\n", __FUNCTION__);
        return ENOERR;
    default:
        return -EINVAL;
    }
}
#endif // CYGPKG_IO_SERIAL_HALDIAG
