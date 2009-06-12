#ifndef CYGONCE_LIBC_STDIO_STDIOFILES_INL
#define CYGONCE_LIBC_STDIO_STDIOFILES_INL
//========================================================================
//
//      stdiofiles.inl
//
//      ISO C library stdio central file inlines
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
// Usage:         Do not include this file directly. Instead use:
//                #include <cyg/libc/stdio/stdiofiles.hxx>
//
//####DESCRIPTIONEND####
//
//========================================================================

// CONFIGURATION

#include <pkgconf/libc_stdio.h>          // libc stdio configuration

// INCLUDES

#include <cyg/infra/cyg_type.h>          // cyg_bool
#include <cyg/infra/cyg_ass.h>           // Assert interface
#include <cyg/libc/stdio/stdiofiles.hxx> // header for this file
#include <cyg/libc/stdio/stream.hxx>     // Cyg_StdioStream

#ifdef CYGSEM_LIBC_STDIO_THREAD_SAFE_STREAMS
# include <cyg/kernel/mutex.hxx>         // mutexes
#endif

// INLINE METHODS

inline Cyg_StdioStream *
Cyg_libc_stdio_files::get_file_stream( fd_t fd )
{
    CYG_PRECONDITION( (fd < FOPEN_MAX),
                  "Attempt to open larger file descriptor than FOPEN_MAX!" );

    return files[fd];

} // Cyg_libc_stdio_files::get_file_stream()
            
inline void
Cyg_libc_stdio_files::set_file_stream( fd_t fd, Cyg_StdioStream *stream )
{
    CYG_PRECONDITION( (fd < FOPEN_MAX),
                  "Attempt to set larger file descriptor than FOPEN_MAX!" );

    files[fd] = stream;

} // Cyg_libc_stdio_files::set_file_stream()


inline cyg_bool
Cyg_libc_stdio_files::lock(void)
{
# ifdef CYGSEM_LIBC_STDIO_THREAD_SAFE_STREAMS
    return files_lock.lock();
# else
    return true;
# endif
} // Cyg_libc_stdio_files::lock()

inline cyg_bool
Cyg_libc_stdio_files::trylock(void)
{
# ifdef CYGSEM_LIBC_STDIO_THREAD_SAFE_STREAMS
    return files_lock.trylock();
# else
    return true;
# endif
} // Cyg_libc_stdio_files::trylock()

inline void
Cyg_libc_stdio_files::unlock(void)
{
# ifdef CYGSEM_LIBC_STDIO_THREAD_SAFE_STREAMS
    files_lock.unlock();
# endif
} // Cyg_libc_stdio_files::unlock()


#endif // CYGONCE_LIBC_STDIO_STDIOFILES_INL multiple inclusion protection

// EOF stdiofiles.inl
