//==========================================================================
//
//      io/serial/common/tty.c
//
//      TTY (terminal-like interface) I/O driver
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
// Purpose:     Device driver for tty I/O, layered on top of serial I/O
// Description: 
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/io.h>
#include <pkgconf/io_serial.h>
#ifdef CYGPKG_IO_SERIAL_TTY
#include <cyg/io/io.h>
#include <cyg/io/devtab.h>
#include <cyg/io/ttyio.h>
#include <cyg/infra/diag.h>

static bool tty_init(struct cyg_devtab_entry *tab);
static Cyg_ErrNo tty_lookup(struct cyg_devtab_entry **tab, 
                               struct cyg_devtab_entry *sub_tab,
                               const char *name);
static Cyg_ErrNo tty_write(cyg_io_handle_t handle, const void *buf, cyg_uint32 *len);
static Cyg_ErrNo tty_read(cyg_io_handle_t handle, void *buf, cyg_uint32 *len);
static cyg_bool  tty_select(cyg_io_handle_t handle, cyg_uint32 which, CYG_ADDRWORD info);
static Cyg_ErrNo tty_get_config(cyg_io_handle_t handle, cyg_uint32 key, void *buf, cyg_uint32 *len);
static Cyg_ErrNo tty_set_config(cyg_io_handle_t handle, cyg_uint32 key, const void *buf, cyg_uint32 *len);

struct tty_private_info {
    cyg_tty_info_t     dev_info;
    cyg_io_handle_t dev_handle;
};

static DEVIO_TABLE(tty_devio,
                   tty_write,
                   tty_read,
                   tty_select,
                   tty_get_config,
                   tty_set_config
    );

#ifdef CYGPKG_IO_SERIAL_TTY_TTYDIAG
static struct tty_private_info tty_private_info_diag;
DEVTAB_ENTRY(tty_io_diag, 
//             "/dev/console",       
//             CYGDAT_IO_SERIAL_TTY_CONSOLE,   // Based on driver for this device
             "/dev/ttydiag",
             "/dev/haldiag",
             &tty_devio, 
             tty_init, 
             tty_lookup,      // Execute this when device is being looked up
             &tty_private_info_diag);
#endif

#ifdef CYGPKG_IO_SERIAL_TTY_TTY0
static struct tty_private_info tty_private_info0;
DEVTAB_ENTRY(tty_io0, 
             "/dev/tty0",
             CYGDAT_IO_SERIAL_TTY_TTY0_DEV,
             &tty_devio, 
             tty_init, 
             tty_lookup,      // Execute this when device is being looked up
             &tty_private_info0);
#endif

#ifdef CYGPKG_IO_SERIAL_TTY_TTY1
static struct tty_private_info tty_private_info1;
DEVTAB_ENTRY(tty_io1, 
             "/dev/tty1", 
             CYGDAT_IO_SERIAL_TTY_TTY1_DEV,
             &tty_devio, 
             tty_init, 
             tty_lookup,      // Execute this when device is being looked up
             &tty_private_info1);
#endif

#ifdef CYGPKG_IO_SERIAL_TTY_TTY2
static struct tty_private_info tty_private_info2;
DEVTAB_ENTRY(tty_io2, 
             "/dev/tty2", 
             CYGDAT_IO_SERIAL_TTY_TTY2_DEV,
             &tty_devio, 
             tty_init, 
             tty_lookup,      // Execute this when device is being looked up
             &tty_private_info2);
#endif

static bool 
tty_init(struct cyg_devtab_entry *tab)
{
    struct tty_private_info *priv = (struct tty_private_info *)tab->priv;
#ifdef CYGDBG_IO_INIT
    diag_printf("Init tty channel: %x\n", tab);
#endif
    priv->dev_info.tty_out_flags = CYG_TTY_OUT_FLAGS_DEFAULT;
    priv->dev_info.tty_in_flags = CYG_TTY_IN_FLAGS_DEFAULT;
    return true;
}

static Cyg_ErrNo 
tty_lookup(struct cyg_devtab_entry **tab, 
           struct cyg_devtab_entry *sub_tab,
           const char *name)
{
    cyg_io_handle_t chan = (cyg_io_handle_t)sub_tab;
    struct tty_private_info *priv = (struct tty_private_info *)(*tab)->priv;
#if 0
    cyg_int32 len;
#endif
    priv->dev_handle = chan;
#if 0
    len = sizeof(cyg_serial_info_t);
    // Initialize configuration
    cyg_io_get_config(chan, CYG_SERIAL_GET_CONFIG, 
                      (void *)&priv->dev_info.serial_config, &len);
#endif
    return ENOERR;
}

#define BUFSIZE 64

static Cyg_ErrNo 
tty_write(cyg_io_handle_t handle, const void *_buf, cyg_uint32 *len)
{
    cyg_devtab_entry_t *t = (cyg_devtab_entry_t *)handle;
    struct tty_private_info *priv = (struct tty_private_info *)t->priv;
    cyg_io_handle_t chan = (cyg_io_handle_t)priv->dev_handle;
    cyg_int32 size, bytes_successful, actually_written;
    cyg_uint8 xbuf[BUFSIZE];
    cyg_uint8 c;
    cyg_uint8 *buf = (cyg_uint8 *)_buf;
    Cyg_ErrNo res = -EBADF;
    // assert(chan)
    size = 0;
    bytes_successful = 0;
    actually_written = 0;
    while (bytes_successful++ < *len) {
        c = *buf++;
        if ((c == '\n') &&
            (priv->dev_info.tty_out_flags & CYG_TTY_OUT_FLAGS_CRLF)) {
            xbuf[size++] = '\r';
        }
        xbuf[size++] = c;
        // Always leave room for possible CR/LF expansion
        if ((size >= (BUFSIZE-1)) ||
            (bytes_successful == *len)) {
            res = cyg_io_write(chan, xbuf, &size);
            if (res != ENOERR) {
                *len = actually_written;
                return res;
            }
            actually_written += size;
            size = 0;
        }
    }
    return res;
}

static Cyg_ErrNo 
tty_read(cyg_io_handle_t handle, void *_buf, cyg_uint32 *len)
{
    cyg_devtab_entry_t *t = (cyg_devtab_entry_t *)handle;
    struct tty_private_info *priv = (struct tty_private_info *)t->priv;
    cyg_io_handle_t chan = (cyg_io_handle_t)priv->dev_handle;
    cyg_uint32 clen;
    cyg_int32 size;
    Cyg_ErrNo res;
    cyg_uint8 c;
    cyg_uint8 *buf = (cyg_uint8 *)_buf;
    // assert(chan)
    size = 0;
    while (size < *len) {
        clen = 1;
        res = cyg_io_read(chan, &c, &clen);
        if (res != ENOERR) {
            *len = size;
            return res;
        }
        buf[size++] = c;
        if ((priv->dev_info.tty_in_flags & CYG_TTY_IN_FLAGS_BINARY) == 0) {
            switch (c) {
            case '\b':    /* drop through */
            case 0x7f:
                size -= 2;  // erase one character + 'backspace' char
                if (size < 0) {
                    size = 0;
                } else if (priv->dev_info.tty_in_flags & CYG_TTY_IN_FLAGS_ECHO) {
                    clen = 3;
                    cyg_io_write(chan, "\b \b", &clen);
                }
                break;
            case '\r':
                if (priv->dev_info.tty_in_flags & CYG_TTY_IN_FLAGS_CRLF) {
                    /* Don't do anything because a '\n' will come next */
                    break;
                }
                if (priv->dev_info.tty_in_flags & CYG_TTY_IN_FLAGS_CR) {
                    c = '\n';  // Map CR -> LF
                }
                /* drop through */
            case '\n':
                if (priv->dev_info.tty_in_flags & CYG_TTY_IN_FLAGS_ECHO) {
                    if (priv->dev_info.tty_out_flags & CYG_TTY_OUT_FLAGS_CRLF) {
                        clen = 2;
                        cyg_io_write(chan, "\r\n", &clen);
                    } else {
                        clen = 1;
                        cyg_io_write(chan, &c, &clen);
                    }
                }
                buf[size-1] = c;
                *len = size;
                return ENOERR;
            default:
                if (priv->dev_info.tty_in_flags & CYG_TTY_IN_FLAGS_ECHO) {
                    clen = 1;
                    cyg_io_write(chan, &c, &clen);
                }
                break;
            }
        }
    }
    *len = size;
    return ENOERR;
}

static cyg_bool
tty_select(cyg_io_handle_t handle, cyg_uint32 which, CYG_ADDRWORD info)
{
    cyg_devtab_entry_t *t = (cyg_devtab_entry_t *)handle;
    struct tty_private_info *priv = (struct tty_private_info *)t->priv;
    cyg_io_handle_t chan = (cyg_io_handle_t)priv->dev_handle;    

    // Just pass it on to next driver level
    return cyg_io_select( chan, which, info );
}

static Cyg_ErrNo 
tty_get_config(cyg_io_handle_t handle, cyg_uint32 key, void *buf, cyg_uint32 *len)
{
    cyg_devtab_entry_t *t = (cyg_devtab_entry_t *)handle;
    struct tty_private_info *priv = (struct tty_private_info *)t->priv;
    cyg_io_handle_t chan = (cyg_io_handle_t)priv->dev_handle;
    Cyg_ErrNo res = ENOERR;
#if 0
    cyg_int32 current_len;
#endif
    // assert(chan)
    switch (key) {
    case CYG_IO_GET_CONFIG_TTY_INFO:
        if (*len < sizeof(cyg_tty_info_t)) {
            return -EINVAL;
        }
#if 0
        current_len = sizeof(cyg_serial_info_t);
        res = cyg_io_get_config(chan, CYG_SERIAL_GET_CONFIG, 
                                (void *)&priv->dev_info.serial_config, &current_len);
        if (res != ENOERR) {
            return res;
        }
#endif
        *(cyg_tty_info_t *)buf = priv->dev_info;
        *len = sizeof(cyg_tty_info_t);
        break;
    default:  // Assume this is a 'serial' driver control
        res = cyg_io_get_config(chan, key, buf, len);
    }
    return res;
}

static Cyg_ErrNo 
tty_set_config(cyg_io_handle_t handle, cyg_uint32 key, const void *buf, cyg_uint32 *len)
{
    cyg_devtab_entry_t *t = (cyg_devtab_entry_t *)handle;
    struct tty_private_info *priv = (struct tty_private_info *)t->priv;
    cyg_io_handle_t chan = (cyg_io_handle_t)priv->dev_handle;
#if 0
    cyg_int32 current_len;
#endif
    Cyg_ErrNo res = ENOERR;
    // assert(chan)
    switch (key) {
    case CYG_IO_SET_CONFIG_TTY_INFO:
        if (*len != sizeof(cyg_tty_info_t)) {
            return -EINVAL;
        }
        priv->dev_info = *(cyg_tty_info_t *)buf;
        break;
    default: // Pass on to serial driver
        res = cyg_io_set_config(chan, key, buf, len);
    }
    return res;
}
#endif // CYGPKG_IO_SERIAL_TTY
