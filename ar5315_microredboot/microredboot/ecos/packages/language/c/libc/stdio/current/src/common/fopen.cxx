//===========================================================================
//
//      fopen.cxx
//
//      Implementation of C library file open function as per ANSI 7.9.5.3
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
// Purpose:       Implements ISO C fopen() function
// Description: 
// Usage:       
//
//####DESCRIPTIONEND####
//
//===========================================================================

// CONFIGURATION

#include <pkgconf/libc_stdio.h>   // Configuration header


// INCLUDES

#include <cyg/infra/cyg_type.h>     // Common project-wide type definitions
#include <stddef.h>                 // NULL and size_t from compiler
#include <errno.h>                  // Error codes
#include <stdio.h>                  // header for fopen()
#include <stdlib.h>                 // malloc()
#include <string.h>                 // strncmp() and strcmp()
#include <cyg/libc/stdio/stdiofiles.hxx> // C library files
#include <cyg/libc/stdio/stream.hxx>     // C library streams

#include <cyg/libc/stdio/io.inl>     // I/O system inlines

// FUNCTIONS

// placement new
inline void *operator new(size_t size, void *ptr)
{
    CYG_CHECK_DATA_PTR( ptr, "Bad pointer" );
    return ptr;
}

// process the mode string. Return true on error
static cyg_bool
process_mode( const char *mode, Cyg_StdioStream::OpenMode *rw,
              cyg_bool *binary, cyg_bool *append )
{
    *binary = *append = false; // default

    switch (mode[0]) {
    case 'r':
        *rw = Cyg_StdioStream::CYG_STREAM_READ;
        break;

    case 'a':
        *append = true;
    case 'w':
        *rw = Cyg_StdioStream::CYG_STREAM_WRITE;
        break;
        
    default:
        return true;
    } // switch

    // ANSI says additional characters may follow the sequences, that we
    // don't necessarily recognise so we just ignore them, and pretend that
    // its the end of the string

    switch (mode[1]) {
    case 'b':
        *binary = true;
        break;
    case '+':
        if (mode[0] == 'r')
	    *rw = Cyg_StdioStream::CYG_STREAM_READWRITE_NOCREATE;
	else
	    *rw = Cyg_StdioStream::CYG_STREAM_READWRITE_CREATE;
        break;
    default:
        return false;
    } // switch

    switch (mode[2]) {
    case 'b':
        *binary = true;
        break;
    case '+':
	if (mode[0] == 'r')
            *rw = Cyg_StdioStream::CYG_STREAM_READWRITE_NOCREATE;
	else
            *rw = Cyg_StdioStream::CYG_STREAM_READWRITE_CREATE;
        break;
    default:
        return false;
    } // switch
    
    return false;
} // process_mode()


static FILE *fopen_inner( cyg_stdio_handle_t dev,
                          Cyg_StdioStream::OpenMode open_mode,
                          cyg_bool binary,
                          cyg_bool append)
{
    Cyg_StdioStream *curr_stream;
    int i;
    Cyg_ErrNo err;
    int bufmode = _IOFBF;
    cyg_ucount32 bufsize = BUFSIZ;
    
    Cyg_libc_stdio_files::lock();

    // find an empty slot
    for (i=0; i < FOPEN_MAX; i++) {
        curr_stream = Cyg_libc_stdio_files::get_file_stream(i);
        if (curr_stream == NULL)
            break;
    } // for

    if (i == FOPEN_MAX) { // didn't find an empty slot
        errno = EMFILE;
        cyg_stdio_close( dev );
        Cyg_libc_stdio_files::unlock();
        return NULL;
    } // if

    // Decide the buffering mode. The default is fully buffered, but if
    // this is an interactive stream then set it to non buffered. 
    if( (dev != CYG_STDIO_HANDLE_NULL) &&
        cyg_stdio_interactive( dev ) )
        bufmode = _IONBF, bufsize = 0;
    
    // Allocate it some memory and construct it.
    curr_stream = (Cyg_StdioStream *)malloc(sizeof(*curr_stream));
    if (curr_stream == NULL) {
        cyg_stdio_close( dev );
        Cyg_libc_stdio_files::unlock();
        errno = ENOMEM;
        return NULL;
    } // if

    curr_stream = new ((void *)curr_stream) Cyg_StdioStream( dev, open_mode,
                                                             append, binary,
                                                             bufmode, bufsize );
    // it puts any error in its own error flag
    if (( err=curr_stream->get_error() )) {

        Cyg_libc_stdio_files::unlock();
        
        free( curr_stream );

        cyg_stdio_close( dev );
        
        errno = err;

        return NULL;

    } // if

    Cyg_libc_stdio_files::set_file_stream(i, curr_stream);
        
    Cyg_libc_stdio_files::unlock();

    return (FILE *)(curr_stream);

} // fopen_inner()

externC FILE *
fopen( const char *filename, const char *mode ) __THROW
{
    cyg_stdio_handle_t dev;
    Cyg_ErrNo err;
    Cyg_StdioStream::OpenMode open_mode;
    cyg_bool binary, append;
    
    // process_mode returns true on error
    if (process_mode( mode, &open_mode, &binary, &append )) {
        errno = EINVAL;
        return NULL;
    } // if

    err = cyg_stdio_open( filename, open_mode, binary, append, &dev );

    // if not found
    if (err != ENOERR) {
        errno = ENOENT;
        return NULL;
    } // if

    return fopen_inner( dev, open_mode, binary, append );
    
} // fopen()


#ifdef CYGFUN_LIBC_STDIO_OPEN_POSIX_FDFUNCS

externC int fileno( FILE *stream ) __THROW
{
    Cyg_StdioStream *real_stream = (Cyg_StdioStream *)stream;

    return real_stream->get_dev();
}

externC FILE *fdopen( int fd, const char *mode ) __THROW
{
    Cyg_StdioStream::OpenMode open_mode;
    cyg_bool binary, append;
    FILE *f;
    
    // process_mode returns true on error
    if (process_mode( mode, &open_mode, &binary, &append )) {
        errno = EINVAL;
        return NULL;
    } // if

    f = fopen_inner( (cyg_stdio_handle_t)fd, open_mode, binary, append );

    if( f == NULL )
        return f;

    // Do a null seek to initialize the file position.
    Cyg_StdioStream *real_stream = (Cyg_StdioStream *)f;
    fpos_t pos = 0;
    real_stream->set_position( pos, SEEK_CUR );
    return f;
}

#endif // def CYGFUN_LIBC_STDIO_OPEN_POSIX_FDFUNCS


// EOF fopen.cxx
