#ifndef CYGONCE_SOCKIO_H
#define CYGONCE_SOCKIO_H
//=============================================================================
//
//      sockio.h
//
//      Socket IO header
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     nickg
// Contributors:  nickg
// Date:          2000-05-25
// Purpose:       Socket IO header
// Description:   This header contains the external definitions of the general
//                socket IO subsystem for POSIX and EL/IX compatability.
//              
// Usage:
//              #include <sockio.h>
//              ...
//              
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <pkgconf/io_fileio.h>

#include <cyg/infra/cyg_type.h>

#include <stddef.h>             // NULL, size_t
#include <limits.h>
#include <sys/types.h>

#include <cyg/fileio/fileio.h>

//=============================================================================
// Forward definitions

struct cyg_nstab_entry;
typedef struct cyg_nstab_entry cyg_nstab_entry;

struct cyg_sock_ops;
typedef struct cyg_sock_ops cyg_sock_ops;

struct sockaddr;
typedef struct sockaddr sockaddr;

struct msghdr;
typedef struct msghdr msghdr;

#ifndef CYGPKG_NET

typedef cyg_uint32      socklen_t;      /* length type for network syscalls */

#endif

//=============================================================================
// network stack entry

struct cyg_nstab_entry
{
    cyg_bool            valid;          // true if stack initialized
    cyg_uint32          syncmode;       // synchronization protocol
    char                *name;          // stack name
    char                *devname;       // hardware device name
    CYG_ADDRWORD        data;           // private data value

    int     (*init)( cyg_nstab_entry *nste );
    int     (*socket)( cyg_nstab_entry *nste, int domain, int type,
		       int protocol, cyg_file *file );
} CYG_HAL_TABLE_TYPE;

#define NSTAB_ENTRY( _l, _syncmode, _name, _devname, _data, _init, _socket )    \
struct cyg_nstab_entry _l CYG_HAL_TABLE_ENTRY(nstab) =                          \
{                                                                               \
    false,                                                                      \
    _syncmode,                                                                  \
    _name,                                                                      \
    _devname,                                                                   \
    _data,                                                                      \
    _init,                                                                      \
    _socket                                                                     \
};

//=============================================================================

struct cyg_sock_ops
{
    int (*bind)      ( cyg_file *fp, const sockaddr *sa, socklen_t len );
    int (*connect)   ( cyg_file *fp, const sockaddr *sa, socklen_t len );
    int (*accept)    ( cyg_file *fp, cyg_file *new_fp,
                       struct sockaddr *name, socklen_t *anamelen );
    int (*listen)    ( cyg_file *fp, int len );
    int (*getname)   ( cyg_file *fp, sockaddr *sa, socklen_t *len, int peer );
    int (*shutdown)  ( cyg_file *fp, int flags );
    int (*getsockopt)( cyg_file *fp, int level, int optname,
                       void *optval, socklen_t *optlen);
    int (*setsockopt)( cyg_file *fp, int level, int optname,
                       const void *optval, socklen_t optlen);
    int (*sendmsg)   ( cyg_file *fp, const struct msghdr *m,
                       int flags, ssize_t *retsize );
    int (*recvmsg)   ( cyg_file *fp, struct msghdr *m,
                       socklen_t *namelen, ssize_t *retsize );
};

//-----------------------------------------------------------------------------
#endif // ifndef CYGONCE_SOCKIO_H
// End of sockio.h
