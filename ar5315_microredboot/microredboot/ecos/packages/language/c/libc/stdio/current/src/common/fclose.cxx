//===========================================================================
//
//      fclose.cxx
//
//      Implementation of C library file close function as per ANSI 7.9.5.1
//
//===========================================================================
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
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     jlarmour
// Contributors:  
// Date:          2000-04-20
// Purpose:       Implements ISO C fclose() function
// Description: 
// Usage:       
//
//####DESCRIPTIONEND####
//
//===========================================================================

// CONFIGURATION

#include <pkgconf/libc_stdio.h>   // Configuration header
#include <pkgconf/infra.h>

// INCLUDES

#include <cyg/infra/cyg_type.h>     // Common project-wide type definitions
#include <stddef.h>                 // NULL and size_t from compiler
#include <errno.h>                  // Error codes
#include <stdio.h>                  // header for fclose()
#include <stdlib.h>                 // free()
#include <cyg/libc/stdio/stdiofiles.hxx>  // C library files
#include <cyg/libc/stdio/stream.hxx>      // C library streams

#include <cyg/libc/stdio/io.inl>     // I/O system inlines

// FUNCTIONS


externC int
fclose( FILE *stream ) __THROW
{
    Cyg_StdioStream *real_stream = (Cyg_StdioStream *)stream;
    int i;
    Cyg_ErrNo err;

    if (NULL == real_stream)
    {
        errno = EBADF;
        return EOF;
    }

    Cyg_libc_stdio_files::lock();

    // find the stream in the table
    for (i=0; i < FOPEN_MAX; i++)
    {
        if (real_stream == Cyg_libc_stdio_files::get_file_stream(i))
            break;
    } // for

    if (i == FOPEN_MAX) // didn't find it
    {
        Cyg_libc_stdio_files::unlock();
        errno = EBADF;
        return EOF;
    } // if

    err = real_stream->close();

    if( err != ENOERR )
    {
        Cyg_libc_stdio_files::unlock();
        errno = err;
        return EOF;
    }
    
#ifdef CYGFUN_INFRA_EMPTY_DELETE_FUNCTIONS
    // Explicitly call destructor - this flushes the output too
    real_stream->~Cyg_StdioStream();

    // and free it
    free(real_stream);
#else
    delete real_stream;
#endif // CYGFUN_INFRA_EMPTY_DELETE_FUNCTIONS

    // and mark the stream available for use
    Cyg_libc_stdio_files::set_file_stream(i, NULL);
        
    Cyg_libc_stdio_files::unlock();

    return 0;

} // fclose()
        
// EOF fclose.cxx
