#ifndef CYGONCE_IO_DEVTAB_H
#define CYGONCE_IO_DEVTAB_H
// ====================================================================
//
//      devtab.h
//
//      Device I/O Table
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

// Private include file.  This file should only be used by device 
// drivers, not application code.

#include <pkgconf/system.h>
#include <cyg/io/io.h>
#include <cyg/hal/drv_api.h>
#include <cyg/hal/hal_tables.h>

// Set of functions which handle top level I/O functions
typedef struct {
    Cyg_ErrNo (*write)(cyg_io_handle_t handle, 
                       const void *buf, 
                       cyg_uint32 *len);
    Cyg_ErrNo (*read)(cyg_io_handle_t handle, 
                      void *buf, 
                      cyg_uint32 *len);
    Cyg_ErrNo (*bwrite)(cyg_io_handle_t handle, 
                       const void *buf, 
                       cyg_uint32 *len,
                       cyg_uint32 pos);
    Cyg_ErrNo (*bread)(cyg_io_handle_t handle, 
                      void *buf, 
                      cyg_uint32 *len,
                      cyg_uint32 pos);
    cyg_bool  (*select)(cyg_io_handle_t handle,
                        cyg_uint32 which,
                        CYG_ADDRWORD info);
    Cyg_ErrNo (*get_config)(cyg_io_handle_t handle, 
                            cyg_uint32 key, 
                            void *buf, 
                            cyg_uint32 *len);
    Cyg_ErrNo (*set_config)(cyg_io_handle_t handle, 
                            cyg_uint32 key, 
                            const void *buf, 
                            cyg_uint32 *len);
} cyg_devio_table_t;


// Default functions

__externC Cyg_ErrNo cyg_devio_cwrite(cyg_io_handle_t handle, 
                                     const void *buf, cyg_uint32 *len);
__externC Cyg_ErrNo cyg_devio_cread(cyg_io_handle_t handle, 
                                    void *buf, cyg_uint32 *len);
__externC Cyg_ErrNo cyg_devio_bwrite(cyg_io_handle_t handle, 
                                     const void *buf, cyg_uint32 *len,
                                     cyg_uint32 pos);
__externC Cyg_ErrNo cyg_devio_bread(cyg_io_handle_t handle, 
                                    void *buf, cyg_uint32 *len,
                                    cyg_uint32 pos);

__externC Cyg_ErrNo cyg_devio_select(cyg_io_handle_t handle,
                                     cyg_uint32 which,
                                     CYG_ADDRWORD info);

__externC Cyg_ErrNo cyg_devio_get_config(cyg_io_handle_t handle,
                                         cyg_uint32 key,
                                         void* buf,
                                         cyg_uint32* len);

__externC Cyg_ErrNo cyg_devio_set_config(cyg_io_handle_t handle,
                                         cyg_uint32 key,
                                         void* buf,
                                         cyg_uint32* len);

// Initialization macros

#define CHAR_DEVIO_TABLE(_l,_write,_read,_select,_get_config,_set_config)    \
cyg_devio_table_t _l = {                                        \
    _write,                                                     \
    _read,                                                      \
    cyg_devio_bwrite,                                           \
    cyg_devio_bread,                                            \
    _select,                                                    \
    _get_config,                                                \
    _set_config,                                                \
};

#define BLOCK_DEVIO_TABLE(_l,_bwrite,_bread,_select,_get_config,_set_config)    \
cyg_devio_table_t _l = {                                        \
    cyg_devio_cwrite,                                           \
    cyg_devio_cread,                                            \
    _bwrite,                                                    \
    _bread,                                                     \
    _select,                                                    \
    _get_config,                                                \
    _set_config,                                                \
};

#define DEVIO_TABLE(_l,_write,_read,_select,_get_config,_set_config) \
        CHAR_DEVIO_TABLE(_l,_write,_read,_select,_get_config,_set_config)

typedef struct cyg_devtab_entry {
    const char        *name;
    const char        *dep_name;
    cyg_devio_table_t *handlers;
    bool             (*init)(struct cyg_devtab_entry *tab);
    Cyg_ErrNo        (*lookup)(struct cyg_devtab_entry **tab, 
                               struct cyg_devtab_entry *sub_tab,
                               const char *name);
    void              *priv;
    unsigned long     status;
} CYG_HAL_TABLE_TYPE cyg_devtab_entry_t;

#define CYG_DEVTAB_STATUS_AVAIL   0x0001
#define CYG_DEVTAB_STATUS_CHAR    0x1000
#define CYG_DEVTAB_STATUS_BLOCK   0x2000

extern cyg_devtab_entry_t __DEVTAB__[], __DEVTAB_END__;

#define CHAR_DEVTAB_ENTRY(_l,_name,_dep_name,_handlers,_init,_lookup,_priv)  \
cyg_devtab_entry_t _l CYG_HAL_TABLE_ENTRY(devtab) = {                   \
   _name,                                                               \
   _dep_name,                                                           \
   _handlers,                                                           \
   _init,                                                               \
   _lookup,                                                             \
   _priv,                                                               \
   CYG_DEVTAB_STATUS_CHAR                                               \
};

#define BLOCK_DEVTAB_ENTRY(_l,_name,_dep_name,_handlers,_init,_lookup,_priv)  \
cyg_devtab_entry_t _l CYG_HAL_TABLE_ENTRY(devtab) = {                   \
   _name,                                                               \
   _dep_name,                                                           \
   _handlers,                                                           \
   _init,                                                               \
   _lookup,                                                             \
   _priv,                                                               \
   CYG_DEVTAB_STATUS_BLOCK                                              \
};

#define DEVTAB_ENTRY(_l,_name,_dep_name,_handlers,_init,_lookup,_priv) \
        CHAR_DEVTAB_ENTRY(_l,_name,_dep_name,_handlers,_init,_lookup,_priv)


#define DEVTAB_ENTRY_NO_INIT(_l,_name,_dep_name,_handlers,_init,_lookup,_priv)  \
cyg_devtab_entry_t _l = {                                                       \
   _name,                                                                       \
   _dep_name,                                                                   \
   _handlers,                                                                   \
   _init,                                                                       \
   _lookup,                                                                     \
   _priv,                                                                       \
   CYG_DEVTAB_STATUS_CHAR                                                       \
};

#endif // CYGONCE_IO_DEVTAB_H
