#ifndef CYGONCE_LIBC_STDIO_STREAM_HXX
#define CYGONCE_LIBC_STDIO_STREAM_HXX
//========================================================================
//
//      stream.hxx
//
//      Internal C library stdio stream interface definitions
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
// Date:          2001-03-16
// Purpose:     
// Description: 
// Usage:         #include <cyg/libc/stdio/stream.hxx>
//
//####DESCRIPTIONEND####
//
//========================================================================

// CONFIGURATION

#include <pkgconf/libc_stdio.h>   // Configuration header

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common project-wide type definitions
#include <cyg/infra/cyg_ass.h>     // Get assertion macros, as appropriate
#include <errno.h>                 // Cyg_ErrNo
#include <stdio.h>                 // fpos_t and IOBUF defines
#include <cyg/libc/stdio/io.hxx>     // Physical IO support
#include <cyg/libc/stdio/streambuf.hxx>  // Stdio stream file buffers

#ifdef CYGSEM_LIBC_STDIO_THREAD_SAFE_STREAMS
#include <pkgconf/kernel.h>
#include <cyg/kernel/mutex.hxx>    // Cyg_Mutex
#endif

// TYPE DEFINITIONS

class Cyg_StdioStream;
__externC Cyg_ErrNo
cyg_libc_stdio_flush_all_but( Cyg_StdioStream * );

class Cyg_StdioStream
{
    friend int setvbuf( FILE *, char *, int, size_t ) __THROW;
    friend Cyg_ErrNo
    cyg_libc_stdio_flush_all_but( Cyg_StdioStream * );

private:

    // error status for this file
    Cyg_ErrNo error;


    cyg_stdio_handle_t my_device;

#ifdef CYGFUN_LIBC_STDIO_ungetc
    cyg_uint8 unread_char_buf;
#endif

#ifdef CYGSEM_LIBC_STDIO_WANT_BUFFERED_IO
    Cyg_StdioStreamBuffer io_buf; // read/write buffer
#endif
    cyg_uint8 readbuf_char; // a one character emergency "buffer"
                            // only used when configured to not buffer
                            // (i.e. !CYGSEM_LIBC_STDIO_WANT_BUFFERED_IO)
                            // or set at runtime to not buffer (i.e.
                            // buffering mode is _IONBF)

    // some flags indicating the state of the file. Some of it is internal
    // state, which should not be public. Use bitfields to save
    // space, which means using "unsigned int" rather than cyg_uintX
    struct {
        unsigned int at_eof                  : 1; // Have we reached eof?

        unsigned int opened_for_read         : 1; // opened_for_read and

        unsigned int opened_for_write        : 1; // opened_for_write can
                                                  // be set simultaneously

        unsigned int binary                  : 1; // opened for binary or
                                                  // text mode?
        
#ifdef CYGFUN_LIBC_STDIO_ungetc
        unsigned int unread_char_buf_in_use  : 1; // unget buf in use?
#endif

#ifdef CYGSEM_LIBC_STDIO_WANT_BUFFERED_IO
        unsigned int buffering               : 1; // Is this file buffered?

        unsigned int line_buffering          : 1; // If so, is it line
                                                  // buffered? If it is
                                                  // buffered, but NOT line
                                                  // buffered, it must be
                                                  // fully buffered

        unsigned int last_buffer_op_was_read : 1; // did we last read from
                                                  // the buffer. If not we
                                                  // must have written
#endif
        unsigned int readbuf_char_in_use     : 1; // is the above
                                                  // readbuf_char in use?
        
    } flags;

    // current position for reading/writing
    fpos_t position;

#ifdef CYGSEM_LIBC_STDIO_THREAD_SAFE_STREAMS
    Cyg_Mutex stream_lock;  // used for locking this stream
#endif // ifdef CYGSEM_LIBC_STDIO_THREAD_SAFE_STREAMS

#ifdef CYGDBG_USE_ASSERTS
    // a value to check that this class is hopefully valid
    cyg_ucount32 magic_validity_word;
#endif

public:
    // different modes when constructing (i.e. opening).
    typedef enum {
        CYG_STREAM_READ,
        CYG_STREAM_WRITE,
        CYG_STREAM_READWRITE_NOCREATE,
        CYG_STREAM_READWRITE_CREATE
    } OpenMode;

    // CONSTRUCTORS

    // This constructs the stream - effectively opens the file. This is
    // used for static initialisation, and actually calls construct below
    //
    // dev is a valid Cyg_Device_Table_t, although it does not have to
    // be a member of the device table object itself
    //
    // open_mode is one of CYG_STREAM_READ, CYG_STREAM_WRITE,
    // CYG_STREAM_READWRITE_NOCREATE or CYG_STREAM_READWRITE_CREATE
    //
    // append is true if the file position should be set at EOF on opening
    //
    // binary is true if this is a binary stream. Otherwise it is a text
    // stream and character conversions (especially newline) may occur
    //
    // buffer_mode is one of _IONBF, _IOLBF, _IOFBF (from <stdio.h>)
    // If buffer_mode is _IONBF, buffer_size should still be set to 0
    // and buffer_addr to NULL. If buffering is not configured, none
    // of buffer_mode, buffer_size and buffer_addr have any effect
    //
    // buffer_size is the size of buffer to use
    //
    // buffer_addr is the address of a user supplied buffer. By default
    // (when NULL) a system one is provided.
    //
    // The "return code" is set by assignment to the error member of this
    // stream - use the get_error() method to check

    Cyg_StdioStream( cyg_stdio_handle_t dev, OpenMode open_mode,
                     cyg_bool append, cyg_bool binary, int buffer_mode,
                     cyg_ucount32 buffer_size=BUFSIZ,
                     cyg_uint8 *buffer_addr=NULL );

    Cyg_StdioStream( OpenMode open_mode, 
                     cyg_ucount32 buffer_size=BUFSIZ,
                     cyg_uint8 *buffer_addr=NULL );

private:    
    void initialize( cyg_stdio_handle_t dev, OpenMode open_mode,
                     cyg_bool append, cyg_bool binary, int buffer_mode,
                     cyg_ucount32 buffer_size=BUFSIZ,
                     cyg_uint8 *buffer_addr=NULL );
public:
    
    // DESTRUCTOR

    ~Cyg_StdioStream();


    // MEMBER FUNCTIONS

    // Close the stream. This should be called before the destructor,
    // so we can see and report any errors produced.
    Cyg_ErrNo close();

    // Refill read buffer from the stream - note this blocks until
    // something arrives on the stream
    Cyg_ErrNo
    refill_read_buffer( void );


    // Read not more than buffer_length bytes from the read buffer into the
    // user buffer.
    // The number of bytes put into the user buffer is written
    // into *bytes_read
    Cyg_ErrNo
    read( cyg_uint8 *user_buffer, cyg_ucount32 buffer_length,
          cyg_ucount32 *bytes_read );


    // Read a single byte from the stream. Returns EAGAIN if no character
    // available or EEOF if end of file (as well as setting the EOF state)
    Cyg_ErrNo
    read_byte( cyg_uint8 *c );

    // Read a single byte from the stream, but don't remove it. Returns
    // EAGAIN if no character available or EEOF if end of file (as well
    // as setting the EOF state)
    Cyg_ErrNo
    peek_byte( cyg_uint8 *c );


    // Return a byte into the stream - basically the same as ungetc()
    Cyg_ErrNo
    unread_byte( cyg_uint8 c );

    // the number of bytes available to read without needing to refill the
    // buffer
    cyg_ucount32
    bytes_available_to_read( void );

    Cyg_ErrNo
    write( const cyg_uint8 *buffer, cyg_ucount32 buffer_length,
           cyg_ucount32 *bytes_written );

    Cyg_ErrNo
    write_byte( cyg_uint8 c );


    Cyg_ErrNo
    flush_output( void );

    Cyg_ErrNo
    flush_output_unlocked( void );

    // prevent multiple access in thread safe mode

    // lock_me() returns false if it couldn't be locked, which could
    // happen if the file descriptor is bad

    cyg_bool
    lock_me( void );

    // trylock_me() returns false if it couldn't be locked, probably
    // because it is already locked
    cyg_bool
    trylock_me( void );

    void
    unlock_me( void );

    // get error status for this file 
    Cyg_ErrNo
    get_error( void );

    // set error status for this file.
    void
    set_error( Cyg_ErrNo errno_to_set );

    // are we at EOF? true means we are, false means no
    cyg_bool
    get_eof_state( void );

    // Set whether we are at EOF.
    void
    set_eof_state( cyg_bool eof_to_set );

    // retrieve position
    Cyg_ErrNo
    get_position( fpos_t *pos );

    // set absolute position. whence is SEEK_SET, SEEK_CUR, or SEEK_END
    Cyg_ErrNo
    set_position( fpos_t pos, int whence );

    // Return my_device
    cyg_stdio_handle_t get_dev() { return my_device; };
    
    CYGDBG_DEFINE_CHECK_THIS
};

// INLINE FUNCTIONS

#include <cyg/libc/stdio/stream.inl>

#endif // CYGONCE_LIBC_STDIO_STREAM_HXX multiple inclusion protection

// EOF stream.hxx
