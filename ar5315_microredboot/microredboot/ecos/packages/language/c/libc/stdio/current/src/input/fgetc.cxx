//===========================================================================
//
//      fgetc.cxx
//
//      ISO C standard I/O get character functions
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
// Purpose:      Provide the fgetc() function. Also provides the function
//               version of getc() and getchar()
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
#include <cyg/infra/cyg_ass.h>      // Standard eCos assertion support
#include <cyg/infra/cyg_trac.h>     // Standard eCos tracing support
#include <stddef.h>                 // NULL and size_t from compiler
#include <stdio.h>                  // header for this file
#include <errno.h>                  // error codes
#include <cyg/libc/stdio/stream.hxx>// Cyg_StdioStream

// FUNCTIONS

externC int
fgetc( FILE *stream ) __THROW
{
    Cyg_StdioStream *real_stream = (Cyg_StdioStream *)stream;
    cyg_ucount32 bytes_read;
    Cyg_ErrNo err;
    cyg_uint8 c;

    CYG_REPORT_FUNCNAMETYPE("fgetc", "returning char %d");
    CYG_REPORT_FUNCARG1XV( stream );
    
    CYG_CHECK_DATA_PTR( stream, "stream is not a valid pointer" );

    err = real_stream->read( &c, 1, &bytes_read );

    // Why do we need this?  Because the buffer might be empty.
    if (!err && !bytes_read) { // if no err, but nothing to read, try again
        err = real_stream->refill_read_buffer();
        if ( !err )
            err = real_stream->read( &c, 1, &bytes_read );
    } // if

    CYG_ASSERT( (ENOERR != err) || (1 == bytes_read), "Didn't read 1 byte!" );

    if (err)
    {
        if ( EAGAIN != err ) {
            real_stream->set_error( err );
            errno = err;
        }
        CYG_REPORT_RETVAL(EOF);
        return EOF;
    } // if
    
    CYG_REPORT_RETVAL((int)c);
    return (int)c;

} // fgetc()


// Also define getc() even though it can be a macro.
// Undefine it first though
#undef getchar

externC int
getchar( void ) __THROW
{
    return fgetc( stdin );
} // getchar()


// EXPORTED SYMBOLS

// Also define getc() even though it can be a macro.
// Undefine it first though
#undef getc

externC int
getc( FILE * ) __THROW CYGBLD_ATTRIB_WEAK_ALIAS(fgetc);

// EOF fgetc.cxx
