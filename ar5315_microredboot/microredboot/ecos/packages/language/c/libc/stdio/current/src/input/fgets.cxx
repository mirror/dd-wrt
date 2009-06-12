//========================================================================
//
//      fgets.cxx
//
//      ISO C standard I/O fgets() function
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
// Date:          2001-03-14
// Purpose:       Implementation of ISO C standard I/O fgets() function
// Description: 
// Usage:       
//
//####DESCRIPTIONEND####
//
//=======================================================================

// CONFIGURATION

#include <pkgconf/libc_stdio.h>   // Configuration header

// INCLUDES

#include <cyg/infra/cyg_type.h>     // Common type definitions
#include <cyg/infra/cyg_ass.h>      // Assertion support
#include <cyg/infra/cyg_trac.h>     // Tracing support
#include <stddef.h>                 // NULL and size_t from compiler
#include <stdio.h>                  // header for this file
#include <errno.h>                  // error codes
#include <cyg/libc/stdio/stream.hxx>// Cyg_StdioStream

// FUNCTIONS

// FIXME: should be reworked to read buffer at a time, and scan that
// for newlines, rather than reading byte at a time.

externC char *
fgets( char *s, int n, FILE *stream ) __THROW
{
    Cyg_StdioStream *real_stream = (Cyg_StdioStream *)stream;
    Cyg_ErrNo err=ENOERR;
    cyg_uint8 c;
    cyg_uint8 *str=(cyg_uint8 *)s;
    int nch;

    CYG_CHECK_DATA_PTRC( s );
    CYG_CHECK_DATA_PTRC( stream );
    CYG_PRECONDITION( n > 0, "requested 0 or negative chars");

    CYG_REPORT_FUNCTYPE( "returning string %08x");
    CYG_REPORT_FUNCARG3( "s=%08x, n=%d, stream=%08x", s, n, stream );
    
    for (nch=1; nch < n; nch++) {
        err = real_stream->read_byte( &c );
        
        // if nothing to read, try again ONCE after refilling buffer
        if (EAGAIN == err) {
            err = real_stream->refill_read_buffer();
            if ( !err )
                err = real_stream->read_byte( &c );

            if (EAGAIN == err) {
                if (1 == nch) {   // indicates EOF at start
                    CYG_REPORT_RETVAL( NULL );
                    return NULL;
                } else
                    break; // EOF
            } // if
        } // if
        
        *str++ = c;
        if ('\n' == c)
            break;
    } // while
    
    *str = '\0'; // NULL terminate it
    
    if (err && EAGAIN != err) {
        real_stream->set_error( err );
        errno = err;
        CYG_REPORT_RETVAL( NULL );
        return NULL;
    } // if

    CYG_REPORT_RETVAL( s );
    return s;

} // fgets()

// EOF fgets.cxx
