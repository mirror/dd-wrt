#ifndef CYGONCE_LIBC_STDIO_STREAM_INL
#define CYGONCE_LIBC_STDIO_STREAM_INL
//========================================================================
//
//      stream.inl
//
//      Inline functions for internal C library stdio stream interface
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
// Date:          2000-04-19
// Purpose:     
// Description: 
// Usage:         Do not include this file -
//                #include <cyg/libc/stdio/stream.hxx> instead.
//
//####DESCRIPTIONEND####
//
//========================================================================

// CONFIGURATION

#include <pkgconf/libc_stdio.h>    // Configuration header

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common project-wide type definitions
#include <stddef.h>                // NULL and size_t from compiler
#include <errno.h>                 // Error codes
#include <cyg/libc/stdio/stream.hxx> // Just be sure that this really is
                                   // included

#include <cyg/libc/stdio/io.inl>     // I/O system inlines

// FUNCTIONS

#ifdef CYGDBG_USE_ASSERTS
inline cyg_bool
Cyg_StdioStream::check_this( cyg_assert_class_zeal zeal ) const
{
    // check that it has the magic word set meaning it is valid.
    if ( magic_validity_word != 0x7b4321ce )
        return false;
    return true;
} // check_this()

#endif // ifdef CYGDBG_USE_ASSERTS



// LOCKING FUNCTIONS

// returns true on success
inline cyg_bool
Cyg_StdioStream::lock_me( void )
{
    CYG_ASSERTCLASS( this, "Stream object is not a valid stream!" );
    
#ifdef CYGSEM_LIBC_STDIO_THREAD_SAFE_STREAMS
    return stream_lock.lock();
#else
    // otherwise it "worked"
    return true;
#endif
    
} // lock_me()


// returns true on success
inline cyg_bool
Cyg_StdioStream::trylock_me( void )
{
    CYG_ASSERTCLASS( this, "Stream object is not a valid stream!" );
    
#ifdef CYGSEM_LIBC_STDIO_THREAD_SAFE_STREAMS
    return stream_lock.trylock();
#else
    // otherwise it "worked"
    return true;
#endif
    
} // lock_me()


inline void
Cyg_StdioStream::unlock_me( void )
{
    CYG_ASSERTCLASS( this, "Stream object is not a valid stream!" );
    
#ifdef CYGSEM_LIBC_STDIO_THREAD_SAFE_STREAMS
    stream_lock.unlock();
#endif
} // unlock_me()


// DESTRUCTOR

inline Cyg_ErrNo
Cyg_StdioStream::close()
{
    Cyg_ErrNo err = ENOERR;
    
    if (!lock_me())
        return EBADF;

    if( my_device != CYG_STDIO_HANDLE_NULL )
    {
        flush_output_unlocked();

        err = cyg_stdio_close( my_device );
    
        if( err == ENOERR )
            my_device = CYG_STDIO_HANDLE_NULL;
    }
    
    unlock_me();
    
    return err;
} // close()

inline
Cyg_StdioStream::~Cyg_StdioStream()
{
    CYG_ASSERTCLASS( this, "Stream object is not a valid stream!" );

    // Close the device if it has not already been closed.
    if( my_device != CYG_STDIO_HANDLE_NULL )
        close();
    
#ifdef CYGDBG_USE_ASSERTS
    magic_validity_word = 0xbadbad;
#endif
} // Cyg_StdioStream destructor


// MEMBER FUNCTIONS


// this is currently just a wrapper around write, but having this interface
// leaves scope for optimisations in future
inline Cyg_ErrNo
Cyg_StdioStream::write_byte( cyg_uint8 c )
{
    cyg_ucount32 dummy_bytes_written;
    Cyg_ErrNo err;

    CYG_ASSERTCLASS( this, "Stream object is not a valid stream!" );
    
    err = write( &c, 1, &dummy_bytes_written );

    CYG_ASSERT( (err!=ENOERR) || (dummy_bytes_written==1),
                "Single byte not written, but no error returned!" );

    return err;
} // write_byte()


inline Cyg_ErrNo
Cyg_StdioStream::unread_byte( cyg_uint8 c )
{
    CYG_ASSERTCLASS( this, "Stream object is not a valid stream!" );
    
#ifdef CYGFUN_LIBC_STDIO_ungetc
    if (!lock_me())
        return EBADF;  // assume file is now invalid

    if (flags.unread_char_buf_in_use) {
        unlock_me();
        return ENOMEM;
    } // if

    flags.unread_char_buf_in_use = true;
    unread_char_buf = c;

    // can't be at EOF any more
    flags.at_eof = false;

    if (position)    // position is always 0 for certain devices
        --position;
    
    unlock_me();

    return ENOERR;

#else // ifdef CYGFUN_LIBC_STDIO_ungetc

    return ENOSYS;
#endif // ifdef CYGFUN_LIBC_STDIO_ungetc
} // unread_byte()


inline cyg_ucount32
Cyg_StdioStream::bytes_available_to_read( void )
{
    cyg_ucount32 bytes=0;

    CYG_ASSERTCLASS( this, "Stream object is not a valid stream!" );
    
#ifdef CYGFUN_LIBC_STDIO_ungetc
    if (flags.unread_char_buf_in_use)
        ++bytes;
#endif 

#ifdef CYGSEM_LIBC_STDIO_WANT_BUFFERED_IO

    // either the last operation was a read, which attempted to read bytes
    // into the buffer, or there are no bytes in the buffer

    if (flags.buffering) {
        if (flags.last_buffer_op_was_read == true)
            bytes += io_buf.get_buffer_space_used();
    }
    else

#endif

    if (flags.readbuf_char_in_use)
        ++bytes;

    return bytes;
} // bytes_available_to_read()



inline Cyg_ErrNo
Cyg_StdioStream::flush_output( void )
{
    Cyg_ErrNo err;

    CYG_ASSERTCLASS( this, "Stream object is not a valid stream!" );
    
    if (!lock_me())
        return EBADF;  // assume file is now invalid
    
    err = flush_output_unlocked();

    unlock_me();
  
    return err;
} // flush_output()


// get error status for this file
inline Cyg_ErrNo
Cyg_StdioStream::get_error( void )
{
    Cyg_ErrNo err_temp;
    
    CYG_ASSERTCLASS( this, "Stream object is not a valid stream!" );
    
    if (!lock_me())
        return EBADF;     // well, we've certainly got an error now!
    
    err_temp = error;

    unlock_me();

    return err_temp;
} // get_error()


// set error status for this file
inline void
Cyg_StdioStream::set_error( Cyg_ErrNo errno_to_set )
{
    CYG_ASSERTCLASS( this, "Stream object is not a valid stream!" );
    
    if (!lock_me())
    {
        errno = EBADF; // best we can do - we can't trust error to be there
        return;
    } // if
    
    errno = error = errno_to_set;

    if ( EEOF == error )
        flags.at_eof = 1;

    unlock_me();
} // set_error()


// are we at EOF? true means we are, false means no
inline cyg_bool
Cyg_StdioStream::get_eof_state( void )
{
    cyg_bool eof_temp;

    CYG_ASSERTCLASS( this, "Stream object is not a valid stream!" );
    
    if (!lock_me())
        return false;     // not much we can do here
    
    eof_temp = flags.at_eof;

    unlock_me();
    
    return eof_temp;
} // get_eof_state()


// Set whether we are at EOF.
inline void
Cyg_StdioStream::set_eof_state( cyg_bool eof_to_set )
{
    CYG_ASSERTCLASS( this, "Stream object is not a valid stream!" );
    
    if (!lock_me())
        return;     // not much we can do here
    
    flags.at_eof = eof_to_set;

    unlock_me();
} // set_eof_state()


// retrieve position
inline Cyg_ErrNo
Cyg_StdioStream::get_position( fpos_t *pos )
{
    CYG_ASSERTCLASS( this, "Stream object is not a valid stream!" );
    
    if (!lock_me())
        return EBADF; // assume file is now invalid

    *pos = position;

    unlock_me();

    return ENOERR;

} // get_position()


// set absolute position
inline Cyg_ErrNo
Cyg_StdioStream::set_position( fpos_t pos, int whence )
{
    CYG_ASSERTCLASS( this, "Stream object is not a valid stream!" );
    
#ifndef CYGPKG_LIBC_STDIO_FILEIO    
    // this is currently a workaround until we have real files
    // this will be corrected when we decide the true filesystem interface

    Cyg_ErrNo err;
    cyg_uint8 c;

    if ((whence != SEEK_CUR) || pos < 0)
        return ENOSYS;

    if (!lock_me())
        return EBADF; // assume file is now invalid

    // Drain read buffer
    
    for ( ; pos > 0 ; pos-- ) {
        err = read_byte( &c );
        if (err == EAGAIN)
            err=refill_read_buffer();

        // if read_byte retured error, or refill_read_buffer returned error
        if (err) {
            unlock_me();
            return err;
        } // if
    } // for

    unlock_me();

    return ENOERR;
    
#else

    if (!lock_me())
        return EBADF; // assume file is now invalid

    if ( whence != SEEK_END ) {
        off_t bytesavail = (off_t)bytes_available_to_read();
        off_t abspos = (whence == SEEK_CUR) ? position + pos : pos;
        off_t posdiff = abspos - position;

        if ( posdiff >= 0 && bytesavail > posdiff ) {
            // can just "seek" within the existing buffer
#ifdef CYGFUN_LIBC_STDIO_ungetc
            if (posdiff>0 && flags.unread_char_buf_in_use) {
                flags.unread_char_buf_in_use = false;
                posdiff--;
            }
#endif
#ifdef CYGSEM_LIBC_STDIO_WANT_BUFFERED_IO
            if (posdiff>0 && flags.buffering) {
                io_buf.set_bytes_read(posdiff);
                posdiff=0;
            } else 
#endif
            if (posdiff>0 && flags.readbuf_char_in_use) {
                flags.readbuf_char_in_use = false;
                posdiff--;
            }
            CYG_ASSERT(posdiff==0, "Failed to seek within buffer correctly");

            position = abspos;
            unlock_me();
            return ENOERR;
        } // endif (bytesavail > posdiff)

        if (whence == SEEK_CUR) {
            position += bytesavail;
        }
    } //endif (whence != SEEK_END)

    Cyg_ErrNo err;

    // Flush output if any present.
    err = flush_output_unlocked();

    if( err == ENOERR )
    {
        off_t newpos=pos;
 
        // Clear any input out of input buffer and any ungot chars
        // from unread buffer.
        io_buf.drain_buffer();
    
#ifdef CYGFUN_LIBC_STDIO_ungetc
        flags.unread_char_buf_in_use = false;
#endif

        // Clear EOF indicator.
        flags.at_eof = false;

        // Seek the file to the correct place
        err = cyg_stdio_lseek( my_device, &newpos, whence );
       
        if ( err == ENOERR) {
          // update stream pos
          position = newpos;
        }
    }
    
    unlock_me();

    return err;
#endif    
    
} // set_position()


#endif // CYGONCE_LIBC_STDIO_STREAM_INL multiple inclusion protection

// EOF stream.inl
