//========================================================================
//
//      fread.cxx
//
//      ANSI Stdio fread() function
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
// Date:          2000-04-20
// Purpose:     
// Description: 
// Usage:       
//
//####DESCRIPTIONEND####
//
//=======================================================================

// CONFIGURATION

#include <pkgconf/libc_stdio.h>   // Configuration header

// INCLUDES

#include <cyg/infra/cyg_type.h>     // Common project-wide type definitions
#include <cyg/infra/cyg_ass.h>      // Assertion support
#include <cyg/infra/cyg_trac.h>     // Tracing support
#include <stddef.h>                 // NULL and size_t from compiler
#include <stdio.h>                  // header for this file
#include <errno.h>                  // error codes
#include <cyg/libc/stdio/stream.hxx>// Cyg_StdioStream

// FUNCTIONS

externC size_t
fread( void *ptr, size_t object_size, size_t num_objects, FILE *stream ) __THROW
{
    Cyg_StdioStream *real_stream = (Cyg_StdioStream *)stream;
    cyg_ucount32 bytes_read;
    cyg_ucount32 bytes_to_read;
    cyg_ucount32 total_read;
    Cyg_ErrNo err;
    cyg_uint8 *ptrc = (cyg_uint8 *)ptr;
    

    CYG_REPORT_FUNCNAMETYPE( "fread", "read %d objects" );
    CYG_REPORT_FUNCARG4( "ptr=%08x, object_size=%d, num_objects=%d, "
                         "stream=%08x", ptr, object_size, num_objects,
                         stream );

    bytes_to_read = object_size*num_objects;
    total_read = 0;

    if ( !bytes_to_read ) {
        CYG_REPORT_RETVAL(0);
        return 0;
    } // if

    err = real_stream->read( (cyg_uint8 *)ptr, bytes_to_read,
                             &bytes_read );
    bytes_to_read -= bytes_read;
    total_read += bytes_read;
    ptrc += bytes_read;

    while (!err && bytes_to_read) { // if no err, but not finished - get next
        err = real_stream->refill_read_buffer();
        if ( !err ) {
            err = real_stream->read( (cyg_uint8 *)ptrc, bytes_to_read,
                                     &bytes_read );
            bytes_to_read -= bytes_read;
            total_read += bytes_read;
            ptrc += bytes_read;
        } // if
    } // while

    if (err) {
        if (err != EAGAIN) {
            real_stream->set_error( err );
            errno = err;
        }
    } // if
    
    // we return the number of _objects_ read. Simple division is
    // sufficient as this returns the quotient rather than rounding
    CYG_REPORT_RETVAL( bytes_read/object_size );
    return total_read/object_size;

} // fread()

// EOF fread.cxx
