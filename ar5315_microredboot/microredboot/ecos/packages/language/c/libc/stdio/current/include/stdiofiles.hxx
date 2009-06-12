#ifndef CYGONCE_LIBC_STDIO_STDIOFILES_HXX
#define CYGONCE_LIBC_STDIO_STDIOFILES_HXX
//========================================================================
//
//      stdiofiles.hxx
//
//      ISO C library stdio central file access
//
//========================================================================
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
//========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     jlarmour
// Contributors:  
// Date:          2000-04-19
// Purpose:     
// Description: 
// Usage:         #include <cyg/libc/stdio/stdiosupp.hxx>
//
//####DESCRIPTIONEND####
//
//========================================================================

// CONFIGURATION

#include <pkgconf/libc_stdio.h>    // libc stdio configuration

// INCLUDES

#include <cyg/infra/cyg_type.h>          // cyg_bool, cyg_ucount8, cyg_ucount16
#include <cyg/libc/stdio/stream.hxx>     // Cyg_StdioStream

#ifdef CYGSEM_LIBC_STDIO_THREAD_SAFE_STREAMS
# include <cyg/kernel/mutex.hxx>   // mutexes
#endif

// CLASSES

class Cyg_libc_stdio_files
{
    // List of open files - global for now
    static
    Cyg_StdioStream *files[FOPEN_MAX];

# ifdef CYGSEM_LIBC_STDIO_THREAD_SAFE_STREAMS
    // lock for the above
    static
    Cyg_Mutex files_lock;
# endif

public:

# if FOPEN_MAX < 256
    typedef cyg_ucount8 fd_t;
# else
    typedef cyg_ucount16 fd_t;
# endif

    static Cyg_StdioStream *
    get_file_stream( fd_t fd );

    static void
    set_file_stream( fd_t fd, Cyg_StdioStream *stream );


    // the following functions lock(), trylock() and unlock() do nothing
    // if we haven't got thread-safe streams
    static cyg_bool
    lock(void);

    static cyg_bool
    trylock(void);

    static void
    unlock(void);
    
}; // class Cyg_libc_stdio_files


// Inline functions for this class
#include <cyg/libc/stdio/stdiofiles.inl>

#endif // CYGONCE_LIBC_STDIO_STDIOFILES_HXX multiple inclusion protection

// EOF stdiofiles.hxx
