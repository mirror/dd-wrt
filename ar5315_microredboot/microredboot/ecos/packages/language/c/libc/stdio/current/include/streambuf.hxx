#ifndef CYGONCE_LIBC_STDIO_STREAMBUF_HXX
#define CYGONCE_LIBC_STDIO_STREAMBUF_HXX
//===========================================================================
//
//      streambuf.hxx
//
//      Internal C library stdio stream buffer interface definitions
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
// Date:         2000-04-19
// Purpose:     
// Description:  This file implements the buffer class used by Cyg_StdioStream
//               for file buffers. It is simple, and not thread-safe - that
//               is better done at a higher level for our purposes.
// Usage:        #include <cyg/libc/stdio/streambuf.hxx>
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
#include <errno.h>                // Cyg_ErrNo


// TYPE DEFINITIONS


class Cyg_StdioStreamBuffer
{
private:
    // Buffering data

#if defined(CYGSEM_LIBC_STDIO_SETVBUF_MALLOC)
    cyg_bool call_free;  // should we free the old buffer if set_buffer is
                         // called?
#else 
    cyg_uint8 static_buffer[BUFSIZ]; // Static buffer used when we cannot
                                     // use malloc()/free().
#endif

    cyg_uint8 *buffer_bottom;
    
    cyg_ucount32 buffer_size;

    cyg_uint8 *buffer_top;

    cyg_uint8 *current_buffer_position;

public:

    // CONSTRUCTORS
    
    // You can change the size, or even supply your own buffer, although
    // this only has an effect with dynamic buffers. Otherwise it is
    // silently ignored
    Cyg_StdioStreamBuffer( cyg_ucount32 size=BUFSIZ,
                           cyg_uint8 *new_buffer=NULL );


    // DESTRUCTORS

    ~Cyg_StdioStreamBuffer();

    // METHODS

    // Set up the buffer. As with the constructor, supplying a new_buffer
    // only has an effect with dynamic buffers, although EINVAL is returned
    // in that case. ENOMEM may also be returned
    Cyg_ErrNo
    set_buffer( cyg_ucount32 size=BUFSIZ, cyg_uint8 *new_buffer=NULL );

    // Find out how much buffer space is in use
    cyg_ucount32
    get_buffer_space_used( void );


    // What total size is the current buffer set to be. Can be -1 if the
    // the buffer is invalid
    cyg_count32
    get_buffer_size( void );

    
    // get buffer address to read from, and return the number of bytes
    // available to read
    cyg_ucount32
    get_buffer_addr_to_read( cyg_uint8 **buffer );

    // when finished reading from said space, tell the buffer how much was
    // actually read
    void
    set_bytes_read( cyg_ucount32 bytes );

    // return address of buffer that can be used to write into, and its
    // available capacity
    cyg_ucount32
    get_buffer_addr_to_write( cyg_uint8 **buffer );

    // when finished writing into said space, tell the buffer how much was
    // actually written
    void
    set_bytes_written( cyg_ucount32 bytes );

    // just empty the whole buffer contents
    void
    drain_buffer( void );
}; // class Cyg_StdioStreamBuffer

// INLINE FUNCTIONS

#include <cyg/libc/stdio/streambuf.inl>


#endif // if defined(CYGSEM_LIBC_STDIO_WANT_BUFFERED_IO)

#endif // CYGONCE_LIBC_STDIO_STREAMBUF_HXX multiple inclusion protection

// EOF streambuf.hxx
