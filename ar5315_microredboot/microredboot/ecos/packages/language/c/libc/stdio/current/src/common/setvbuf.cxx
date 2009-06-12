//===========================================================================
//
//      setvbuf.cxx
//
//      Implementation of C library buffering setup as per ANSI 7.9.5.6
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
// Author(s):    jlarmour
// Contributors: 
// Date:         2000-04-20
// Purpose:     
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
#include <stdio.h>                  // header for setvbuf()
#include <cyg/libc/stdio/stream.hxx>// C libray streams

// FUNCTIONS


externC int
setvbuf( FILE *stream, char *buf, int mode, size_t size ) __THROW
{
#ifdef CYGSEM_LIBC_STDIO_WANT_BUFFERED_IO

    Cyg_StdioStream *real_stream = (Cyg_StdioStream *)stream;
    Cyg_ErrNo err;
    
#ifdef CYGSEM_LIBC_STDIO_THREAD_SAFE_STREAMS
    if ( !real_stream->lock_me() ) {
        errno = EBADF;
        return EBADF;
    } // if
#endif

    err = real_stream->io_buf.set_buffer( (cyg_ucount32) size, 
                                          (cyg_uint8 *) buf );
    if (!err) {
        switch (mode) {
        case _IONBF:
            CYG_ASSERT( (size == 0) && (buf == NULL),
                        "No buffering wanted but size/address specified!" );
            real_stream->flags.buffering = false;
            real_stream->flags.line_buffering = false;
            break;
        case _IOLBF:
            real_stream->flags.buffering = true;
            real_stream->flags.line_buffering = true;
            break;
        case _IOFBF:
            real_stream->flags.buffering = true;
            real_stream->flags.line_buffering = false;
            break;
        default:
            err = EINVAL;
            break;
        } // switch
    } // if

#ifdef CYGSEM_LIBC_STDIO_THREAD_SAFE_STREAMS
    real_stream->unlock_me();
#endif
    
    if (err) {
        errno = err;
        return err;
    } // if
        
    return 0;

#else // ifdef CYGSEM_LIBC_STDIO_WANT_BUFFERED_IO

    errno = ENOSUPP;
    return ENOSUPP;

#endif // ifdef CYGSEM_LIBC_STDIO_WANT_BUFFERED_IO

} // setvbuf()

// EOF setvbuf.cxx
