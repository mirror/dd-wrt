#ifndef CYGONCE_IO_H
#define CYGONCE_IO_H
// ====================================================================
//
//      io.h
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
// Purpose:     Describe low level I/O interfaces.
// Description:
//
//####DESCRIPTIONEND####
//
// ====================================================================

// This file contains the user-level visible I/O interfaces

#include <pkgconf/system.h>
#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>

#ifdef CYGPKG_ERROR
#include <cyg/error/codes.h>
#else
#error I/O subsystem requires 'error' package
#endif

// typedef int Cyg_ErrNo;

#ifdef __cplusplus
extern "C" {
#endif

typedef void *cyg_io_handle_t;

// Lookup a device and return it's handle
Cyg_ErrNo cyg_io_lookup(const char *name, 
                        cyg_io_handle_t *handle);
// Write data to a device
Cyg_ErrNo cyg_io_write(cyg_io_handle_t handle, 
                       const void *buf, 
                       cyg_uint32 *len);
// Read data from a device
Cyg_ErrNo cyg_io_read(cyg_io_handle_t handle, 
                      void *buf, 
                      cyg_uint32 *len);
// Write data to a block device
Cyg_ErrNo cyg_io_bwrite(cyg_io_handle_t handle, 
                       const void *buf, 
                       cyg_uint32 *len,
                       cyg_uint32 pos);
// Read data from a block device
Cyg_ErrNo cyg_io_bread(cyg_io_handle_t handle, 
                      void *buf, 
                      cyg_uint32 *len,
                      cyg_uint32 pos);
// Get the configuration of a device
Cyg_ErrNo cyg_io_get_config(cyg_io_handle_t handle, 
                            cyg_uint32 key,
                            void *buf, 
                            cyg_uint32 *len);
// Change the configuration of a device
Cyg_ErrNo cyg_io_set_config(cyg_io_handle_t handle, 
                            cyg_uint32 key,
                            const void *buf, 
                            cyg_uint32 *len);
// Test a device for readiness    
cyg_bool cyg_io_select(cyg_io_handle_t handle,
                       cyg_uint32 which,
                       CYG_ADDRWORD info);

#ifdef __cplusplus
}
#endif

#endif  /* CYGONCE_IO_H */
/* EOF io.h */
