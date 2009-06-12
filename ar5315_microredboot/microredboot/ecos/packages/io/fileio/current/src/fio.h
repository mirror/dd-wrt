#ifndef CYGONCE_FIO_H
#define CYGONCE_FIO_H
//=============================================================================
//
//      fio.h
//
//      Fileio private header
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002 Nick Garnett
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
// Purpose:       Fileio private header
// Description:   This file contains private definitions for communication
//                between the parts of the fileio package.
//              
// Usage:
//              #include "fio.h"
//              ...
//              
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <pkgconf/io_fileio.h>
#include <pkgconf/isoinfra.h>

#include <cyg/infra/cyg_type.h>

#include <stddef.h>             // NULL, size_t
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>

#include <cyg/fileio/fileio.h>
#include <cyg/fileio/sockio.h>

#include <errno.h>

#ifdef CYGPKG_KERNEL
#include <pkgconf/kernel.h>
#include <cyg/kernel/mutex.hxx>        // mutex definitions

#define FILEIO_MUTEX_LOCK(_m_)   ((_m_).lock())
#define FILEIO_MUTEX_UNLOCK(_m_) ((_m_).unlock())

#else
#define FILEIO_MUTEX_LOCK(_m_) 
#define FILEIO_MUTEX_UNLOCK(_m_)
#endif


//=============================================================================
// POSIX API support

#ifdef CYGPKG_POSIX
#include <pkgconf/posix.h>
#include <cyg/posix/export.h>

#define CYG_FILEIO_FUNCTION_START() CYG_POSIX_FUNCTION_START()

#define CYG_FILEIO_FUNCTION_FINISH() CYG_POSIX_FUNCTION_FINISH()

#else

#define CYG_FILEIO_FUNCTION_START() CYG_EMPTY_STATEMENT

#define CYG_FILEIO_FUNCTION_FINISH() CYG_EMPTY_STATEMENT

#endif

#ifdef CYGPKG_POSIX_SIGNALS

#define CYG_FILEIO_SIGMASK_SET( __set, __oset ) \
        CYG_PTHREAD_SIGMASK_SET( __set, __oset )

#define CYG_FILEIO_SIGPENDING() CYG_POSIX_SIGPENDING()

#define CYG_FILEIO_DELIVER_SIGNALS( __mask ) \
        CYG_POSIX_DELIVER_SIGNALS( __mask )

#else

#define CYG_FILEIO_SIGMASK_SET( __set, __oset ) \
CYG_MACRO_START \
CYG_UNUSED_PARAM( const sigset_t*, __set ); \
CYG_UNUSED_PARAM( const sigset_t*, __oset ); \
CYG_MACRO_END

#define CYG_FILEIO_SIGPENDING() (0)

#define CYG_FILEIO_DELIVER_SIGNALS( __mask ) CYG_UNUSED_PARAM( const sigset_t*, __mask )

typedef int sigset_t;

#endif

//=============================================================================
// Fileio function entry and return macros.

// Handle entry to a fileio package function. 
#define FILEIO_ENTRY()                          \
    CYG_REPORT_FUNCTYPE( "returning %d" );      \
    CYG_FILEIO_FUNCTION_START();                \

// Do a fileio package defined return. This requires the error code
// to be placed in errno, and if it is non-zero, -1 returned as the
// result of the function. This also gives us a place to put any
// generic tidyup handling needed for things like signal delivery and
// cancellation.
#define FILEIO_RETURN(err)                      \
CYG_MACRO_START                                 \
    int __retval = 0;                           \
    CYG_FILEIO_FUNCTION_FINISH();               \
    if( err != 0 ) __retval = -1, errno = err;  \
    CYG_REPORT_RETVAL( __retval );              \
    return __retval;                            \
CYG_MACRO_END

#define FILEIO_RETURN_VALUE(val)                \
CYG_MACRO_START                                 \
    CYG_FILEIO_FUNCTION_FINISH();               \
    CYG_REPORT_RETVAL( val );                   \
    return val;                                 \
CYG_MACRO_END

#define FILEIO_RETURN_VOID()                    \
CYG_MACRO_START                                 \
    CYG_FILEIO_FUNCTION_FINISH();               \
    CYG_REPORT_RETURN();                        \
    return;                                     \
CYG_MACRO_END

//=============================================================================
// Cancellation support
// If the POSIX package is present we want to include cancellation points
// in the routines that are defined to contain them.
// The macro CYG_CANCELLATION_POINT does this.

#ifdef CYGINT_ISO_PTHREAD_IMPL

# include <pthread.h>

# define CYG_CANCELLATION_POINT pthread_testcancel()

#else

# define CYG_CANCELLATION_POINT CYG_EMPTY_STATEMENT

#endif

//=============================================================================
// Internal exports

//-----------------------------------------------------------------------------
// Exports from misc.cxx

// Current directory info
__externC cyg_mtab_entry *cyg_cdir_mtab_entry;
__externC cyg_dir cyg_cdir_dir;

__externC int cyg_mtab_lookup( cyg_dir *dir, const char **name, cyg_mtab_entry **mte);

__externC void cyg_fs_lock( cyg_mtab_entry *mte, cyg_uint32 syncmode );

__externC void cyg_fs_unlock( cyg_mtab_entry *mte, cyg_uint32 syncmode );

//-----------------------------------------------------------------------------
// Exports from fd.cxx

__externC void cyg_fd_init();

__externC cyg_file *cyg_file_alloc();

__externC void cyg_file_free(cyg_file * fp);

__externC int cyg_fd_alloc(int low);

__externC void cyg_fd_assign(int fd, cyg_file *fp);

__externC int cyg_fd_free(int fd);

__externC cyg_file *cyg_fp_get( int fd );

__externC void cyg_fp_free( cyg_file *fp );

__externC void cyg_file_lock( cyg_file *fp, cyg_uint32 syncmode );

__externC void cyg_file_unlock( cyg_file *fp, cyg_uint32 syncmode );

//-----------------------------------------------------------------------------
// Exports from socket.cxx

__externC void cyg_nstab_init();

//-----------------------------------------------------------------------------
#endif // ifndef CYGONCE_FIO_H
// End of fio.h
