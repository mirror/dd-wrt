//===========================================================================
//
//      streambuf.cxx
//
//      C library stdio stream buffer functions
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

// Include buffered I/O?
#if defined(CYGSEM_LIBC_STDIO_WANT_BUFFERED_IO)


// INCLUDES

#include <cyg/infra/cyg_type.h>   // Common project-wide type definitions
#include <cyg/infra/cyg_ass.h>    // Assertion support
#include <errno.h>                // Cyg_ErrNo
#include <stdlib.h>               // malloc() and free()
#include <cyg/libc/stdio/streambuf.hxx> // header for this file, just in case

// FUNCTIONS
    
Cyg_ErrNo
Cyg_StdioStreamBuffer::set_buffer( cyg_ucount32 size,
                                   cyg_uint8 *new_buffer )
{
    
#ifdef CYGSEM_LIBC_STDIO_DYNAMIC_SETVBUF
    
    // user-supplied buffer?
    if (new_buffer != NULL) {
        CYG_CHECK_DATA_PTR(new_buffer, "new_buffer not valid");
#ifdef CYGSEM_LIBC_STDIO_SETVBUF_MALLOC
        // first check if we were responsible for the old buffer
        if (call_free) {
            free(buffer_bottom);
            call_free = false;
        }
#endif        
        buffer_bottom = new_buffer;
    }
#ifdef CYGSEM_LIBC_STDIO_SETVBUF_MALLOC    
    else if ( size != buffer_size ) { // as long as its different from
                                      // what we've got now
        cyg_uint8 *malloced_buf;

        malloced_buf = (cyg_uint8 * )malloc( size );
        if (malloced_buf == NULL)
            return ENOMEM;

        // should the old buffer be freed? This waits till after we know
        // whether the malloc succeeded
        if (call_free)
            free( buffer_bottom );
        
        buffer_bottom = malloced_buf;

        call_free=true;

    } // else if
#else
    // Here we have not been given a new buffer, but have been given
    // a new buffer size. If possible, we just shrink what we already
    // have.
    else if( size > buffer_size )
        return EINVAL;
#endif    
    
#else // ifdef CYGSEM_LIBC_STDIO_DYNAMIC_SETVBUF
    
    // In this config we can't use a different buffer, or set a
    // size greater than now. We can pretend to shrink it though
    
    // Note on the next line we compare it with the size of static_buffer
    // and not the current size, as that's what is the limiting factor
    if ( (new_buffer != NULL) || (size > sizeof(static_buffer)) )
        return EINVAL;
    
#endif // ifdef CYGSEM_LIBC_STDIO_DYNAMIC_SETVBUF
    
    buffer_top = current_buffer_position = &buffer_bottom[0];
    buffer_size = size;
    
    return ENOERR;
    
} // set_buffer()


#endif // if defined(CYGSEM_LIBC_STDIO_WANT_BUFFERED_IO)


// EOF streambuf.cxx
