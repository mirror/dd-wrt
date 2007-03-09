//==========================================================================
//
//      io/iosys.c
//
//      I/O Subsystem + Device Table support
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
// Purpose:     Device I/O Support
// Description: 
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/io.h>
#include <cyg/io/io.h>
#include <cyg/io/devtab.h>
#include <cyg/infra/diag.h>

//extern void cyg_io_init(void) CYGBLD_ATTRIB_CONSTRUCTOR
//  CYG_INIT_PRIORITY(CYG_INIT_BEFORE(LIBC));

// Checks that two strings are "equivalent" device names
// 'n1' is a string from the user
// 'n2' is a name in a device table entry
// 'cyg_io_compare()' will return true IFF
//    n1 == n2, for all characters
//    n2 ends in '/' and matches n1 up to the terminating '/'
// 'ptr' will get a pointer to the residual string.
static bool
cyg_io_compare(const char *n1, const char *n2, const char **ptr)
{
    while (*n1 && *n2) {
        if (*n1++ != *n2++) {
            return false;
        }
    }
    if (*n1) {
        // See if the devtab name is is a substring
        if (*(n2-1) == '/') {
            *ptr = n1;
            return true;
        }
    }
    if (*n1 || *n2) {
        return false;
    }
    *ptr = n1;
    return true;
}

//
// This function is called during system initialization.  The purpose is
// to step through all devices linked into the system, calling their
// "init" entry points.  
//

void
cyg_io_init(void)
{
    static int _init = false;
    cyg_devtab_entry_t *t;
    if (_init) return;
    for (t = &__DEVTAB__[0]; t != &__DEVTAB_END__; t++) {
#ifdef CYGDBG_IO_INIT
        diag_printf("Init device '%s'\n", t->name);
#endif
        if (t->init(t)) {
            t->status |= CYG_DEVTAB_STATUS_AVAIL;
        } else {
            // What to do if device init fails?
            // Device not [currently] available
            t->status &= ~CYG_DEVTAB_STATUS_AVAIL;  
        }
    }
    _init = true;
}

//
// Look up the devtab entry for a named device and return its handle.
// If the device is found and it has a "lookup" function, call that
// function to allow the device/driver to perform any necessary
// initializations.
//

Cyg_ErrNo
cyg_io_lookup(const char *name, cyg_io_handle_t *handle)
{
    union devtab_entry_handle_union {
        cyg_devtab_entry_t *st;
        cyg_io_handle_t h;
    } stunion;
    cyg_devtab_entry_t *t;
    Cyg_ErrNo res;
    const char *name_ptr;
    for (t = &__DEVTAB__[0]; t != &__DEVTAB_END__; t++) {
        if (cyg_io_compare(name, t->name, &name_ptr)) {
            // FUTURE: Check 'avail'/'online' here
            if (t->dep_name) {
                res = cyg_io_lookup(t->dep_name, &stunion.h);
                if (res != ENOERR) {
                    return res;
                }
            } else {
                stunion.st = NULL;
            }
            if (t->lookup) {
                // This indirection + the name pointer allows the lookup routine
                // to return a different 'devtab' handle.  This will provide for
                // 'pluggable' devices, file names, etc.
                res = (t->lookup)(&t, stunion.st, name_ptr);
                if (res != ENOERR) {
                    return res;
                }
            }
            *handle = (cyg_io_handle_t)t;
            return ENOERR;
        }
    }
    return -ENOENT;  // Not found
}

//
// 'write' data to a device.
//

Cyg_ErrNo 
cyg_io_write(cyg_io_handle_t handle, const void *buf, cyg_uint32 *len)
{
    cyg_devtab_entry_t *t = (cyg_devtab_entry_t *)handle;
    // Validate request
    if (!t->handlers->write) {
        return -EDEVNOSUPP;
    }
    // Special check.  If length is zero, this just verifies that the 
    // 'write' method exists for the given device.
    if (NULL != len && 0 == *len) {
        return ENOERR;
    }
    return t->handlers->write(handle, buf, len);
}

//
// 'read' data from a device.
//

Cyg_ErrNo 
cyg_io_read(cyg_io_handle_t handle, void *buf, cyg_uint32 *len)
{
    cyg_devtab_entry_t *t = (cyg_devtab_entry_t *)handle;
    // Validate request
    if (!t->handlers->read) {
        return -EDEVNOSUPP;
    }
    // Special check.  If length is zero, this just verifies that the 
    // 'read' method exists for the given device.
    if (NULL != len && 0 == *len) {
        return ENOERR;
    }
    return t->handlers->read(handle, buf, len);
}

Cyg_ErrNo 
cyg_io_bwrite(cyg_io_handle_t handle, const void *buf, cyg_uint32 *len, cyg_uint32 pos)
{
    cyg_devtab_entry_t *t = (cyg_devtab_entry_t *)handle;
    // Validate request
    if (!t->handlers->bwrite) {
        return -EDEVNOSUPP;
    }
    // Special check.  If length is zero, this just verifies that the 
    // 'bwrite' method exists for the given device.
    if (NULL != len && 0 == *len) {
        return ENOERR;
    }
    return t->handlers->bwrite(handle, buf, len, pos);
}

//
// 'read' data from a device.
//

Cyg_ErrNo 
cyg_io_bread(cyg_io_handle_t handle, void *buf, cyg_uint32 *len, cyg_uint32 pos)
{
    cyg_devtab_entry_t *t = (cyg_devtab_entry_t *)handle;
    // Validate request
    if (!t->handlers->bread) {
        return -EDEVNOSUPP;
    }
    // Special check.  If length is zero, this just verifies that the 
    // 'bread' method exists for the given device.
    if (NULL != len && 0 == *len) {
        return ENOERR;
    }
    return t->handlers->bread(handle, buf, len, pos);
}

//
// Check device for available input or space for output
//

cyg_bool
cyg_io_select(cyg_io_handle_t handle, cyg_uint32 which, CYG_ADDRWORD info)
{
    cyg_devtab_entry_t *t = (cyg_devtab_entry_t *)handle;
    // Validate request
    if (!t->handlers->select) {
        return -EDEVNOSUPP;
    }

    return t->handlers->select( handle, which, info );
}

//
// Get the configuration of a device.
//

Cyg_ErrNo 
cyg_io_get_config(cyg_io_handle_t handle, cyg_uint32 key, void *buf, cyg_uint32 *len)
{
    cyg_devtab_entry_t *t = (cyg_devtab_entry_t *)handle;
    // Validate request
    if (!t->handlers->get_config) {
        return -EDEVNOSUPP;
    }
    // Special check.  If length is zero, this just verifies that the 
    // 'get_config' method exists for the given device.
    if (NULL != len && 0 == *len) {
        return ENOERR;
    }
    return t->handlers->get_config(handle, key, buf, len);
}

//
// Change the configuration of a device.
//

Cyg_ErrNo 
cyg_io_set_config(cyg_io_handle_t handle, cyg_uint32 key, const void *buf, cyg_uint32 *len)
{
    cyg_devtab_entry_t *t = (cyg_devtab_entry_t *)handle;
    // Validate request
    if (!t->handlers->set_config) {
        return -EDEVNOSUPP;
    }
    // Special check.  If length is zero, this just verifies that the 
    // 'set_config' method exists for the given device.
    if (NULL != len && 0 == *len) {
        return ENOERR;
    }
    return t->handlers->set_config(handle, key, buf, len);
}

/*---------------------------------------------------------------------------*/
// Default functions for devio tables

Cyg_ErrNo cyg_devio_cwrite(cyg_io_handle_t handle, const void *buf, cyg_uint32 *len)
{
    return -EDEVNOSUPP;
}

Cyg_ErrNo cyg_devio_cread(cyg_io_handle_t handle, void *buf, cyg_uint32 *len)
{
    return -EDEVNOSUPP;
}

Cyg_ErrNo cyg_devio_bwrite(cyg_io_handle_t handle, const void *buf,
                        cyg_uint32 *len, cyg_uint32 pos)
{
    return -EDEVNOSUPP;
}

Cyg_ErrNo cyg_devio_bread(cyg_io_handle_t handle, void *buf,
                       cyg_uint32 *len, cyg_uint32 pos)
{
    return -EDEVNOSUPP;
}

Cyg_ErrNo
cyg_devio_select(cyg_io_handle_t handle, cyg_uint32 which, CYG_ADDRWORD info)
{
    CYG_UNUSED_PARAM(cyg_io_handle_t, handle);
    CYG_UNUSED_PARAM(cyg_uint32, which);
    CYG_UNUSED_PARAM(CYG_ADDRWORD, info);
    return -EDEVNOSUPP;
}

Cyg_ErrNo
cyg_devio_get_config(cyg_io_handle_t handle, cyg_uint32 key,
                     void* buf, cyg_uint32* len)
{
    CYG_UNUSED_PARAM(cyg_io_handle_t, handle);
    CYG_UNUSED_PARAM(cyg_uint32, key);
    CYG_UNUSED_PARAM(void*, buf);
    CYG_UNUSED_PARAM(cyg_uint32*, len);
    return -EDEVNOSUPP;
}

Cyg_ErrNo
cyg_devio_set_config(cyg_io_handle_t handle, cyg_uint32 key,
                     void* buf, cyg_uint32* len)
{
    CYG_UNUSED_PARAM(cyg_io_handle_t, handle);
    CYG_UNUSED_PARAM(cyg_uint32, key);
    CYG_UNUSED_PARAM(void*, buf);
    CYG_UNUSED_PARAM(cyg_uint32*, len);
    return -EDEVNOSUPP;
}

/*---------------------------------------------------------------------------*/
/* End of io/iosys.c */
