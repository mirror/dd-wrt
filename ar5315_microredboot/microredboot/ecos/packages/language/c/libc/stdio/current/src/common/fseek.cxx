//===========================================================================
//
//      fseek.cxx
//
//      Implementation of C library file positioning functions as per ANSI 7.9.9
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
// Author(s):     nickg
// Contributors:  
// Date:          2000-07-12
// Purpose:       Implements ISO C file positioning functions
// Description: 
// Usage:       
//
//####DESCRIPTIONEND####
//
//===========================================================================

// CONFIGURATION

#include <pkgconf/libc_stdio.h>   // Configuration header

#include <cyg/infra/cyg_type.h>     // Common project-wide type definitions
#include <cyg/infra/cyg_ass.h>      // Assertion support
#include <cyg/infra/cyg_trac.h>     // Tracing support
#include <stddef.h>                 // NULL and size_t from compiler
#include <errno.h>                  // Error codes
#include <stdio.h>                  // header for fseek() etc.
#include <cyg/libc/stdio/stdiofiles.hxx> // C library files
#include <cyg/libc/stdio/stream.hxx>     // C library streams

//========================================================================

// ISO C 7.9.9 File positioning functions

externC int
fgetpos( FILE * stream , fpos_t *pos  ) __THROW
{
    Cyg_StdioStream *real_stream = (Cyg_StdioStream *)stream;
    Cyg_ErrNo err;
    int ret = 0;
    
    CYG_REPORT_FUNCNAME( "fgetpos" );

    err = real_stream->get_position( pos );
    
    if( err != ENOERR )
    {
        errno = err;
        ret = -1;
    }
    
    CYG_REPORT_RETVAL( ret );
    return ret;
}

externC int
fseek( FILE * stream , long int offset , int whence  ) __THROW
{
    Cyg_StdioStream *real_stream = (Cyg_StdioStream *)stream;
    Cyg_ErrNo err;
    int ret = 0;
    
    CYG_REPORT_FUNCNAME( "fgetpos" );

    err = real_stream->set_position( (fpos_t)offset, whence );
    
    if( err != ENOERR )
    {
        errno = err;
        ret = -1;
    }
    
    CYG_REPORT_RETVAL( ret );
    return ret;
}

externC int
fsetpos( FILE * stream , const fpos_t * pos ) __THROW
{
    Cyg_StdioStream *real_stream = (Cyg_StdioStream *)stream;
    Cyg_ErrNo err;
    int ret = 0;
    
    CYG_REPORT_FUNCNAME( "fgetpos" );

    err = real_stream->set_position( *pos, SEEK_SET );
    
    if( err != ENOERR )
    {
        errno = err;
        ret = -1;
    }
    
    CYG_REPORT_RETVAL( ret );
    return ret;
}

externC long int
ftell( FILE * stream  ) __THROW
{
    Cyg_StdioStream *real_stream = (Cyg_StdioStream *)stream;
    Cyg_ErrNo err;
    long int ret = 0;
    fpos_t pos;
    
    CYG_REPORT_FUNCNAME( "ftell" );

    err = real_stream->get_position( &pos );
    
    if( err != ENOERR )
    {
        errno = err;
        ret = -1;
    }
    else ret = pos;
    
    CYG_REPORT_RETVAL( ret );
    return ret;
}

externC void
rewind( FILE * stream  ) __THROW
{
    Cyg_StdioStream *real_stream = (Cyg_StdioStream *)stream;    
    (void)fseek( stream, 0L, SEEK_SET );

    real_stream->set_error( ENOERR );
}


// EOF fseek.cxx
