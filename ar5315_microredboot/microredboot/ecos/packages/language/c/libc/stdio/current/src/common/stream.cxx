//========================================================================
//
//      stream.cxx
//
//      Implementations of internal C library stdio stream functions
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
//========================================================================

// CONFIGURATION

#include <pkgconf/libc_stdio.h>   // Configuration header

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common project-wide type definitions
#include <cyg/infra/cyg_ass.h>     // Assertion infrastructure
#include <stddef.h>                // NULL and size_t from compiler
#include <errno.h>                 // Error codes
#include <string.h>                // memcpy() and memset()
#include <cyg/libc/stdio/stream.hxx>     // Header for this file
#include <cyg/libc/stdio/stdiosupp.hxx>  // Stdio support functions


#include <cyg/libc/stdio/io.inl>     // I/O system inlines


// FUNCTIONS

Cyg_StdioStream::Cyg_StdioStream(cyg_stdio_handle_t dev,
                                 OpenMode open_mode,
                                 cyg_bool append, cyg_bool binary,
                                 int buffer_mode, cyg_ucount32 buffer_size,
                                 cyg_uint8 *buffer_addr )
#ifdef CYGSEM_LIBC_STDIO_WANT_BUFFERED_IO
    : io_buf( buffer_size, buffer_addr )
#endif
{
    initialize( dev, open_mode, append, binary, buffer_mode,
                buffer_size, buffer_addr);
}

void Cyg_StdioStream::initialize(cyg_stdio_handle_t dev,
                                 OpenMode open_mode,
                                 cyg_bool append, cyg_bool binary,
                                 int buffer_mode, cyg_ucount32 buffer_size,
                                 cyg_uint8 *buffer_addr )
{

#ifdef CYGDBG_USE_ASSERTS
    magic_validity_word = 0xbadbad;
#endif

    my_device = dev;

    // Clear all flags
    memset( &flags, 0, sizeof(flags) );

    switch (open_mode) {
    case CYG_STREAM_READ:
        flags.opened_for_read = true;
        break;
    case CYG_STREAM_WRITE:
        flags.opened_for_write = true;
        break;
    case CYG_STREAM_READWRITE_NOCREATE:
    case CYG_STREAM_READWRITE_CREATE:
        flags.opened_for_read = true;
        flags.opened_for_write = true;
        break;
    default:
        error=EINVAL;
        return;
    } // switch
        
    
    if (flags.opened_for_write) {
#if 0
        // FIXME: need some replacement for this
        if (!my_device->write_blocking) {
            error = EDEVNOSUPP;
            return;
        } // if
#endif
#ifdef CYGSEM_LIBC_STDIO_WANT_BUFFERED_IO
        flags.last_buffer_op_was_read = false;
#endif
    } // if


    if (flags.opened_for_read) {
#if 0
        // FIXME: need some replacement for this
        if (!my_device->read_blocking) {
            error = EDEVNOSUPP;
            return;
        } // if
#endif

        // NB also if opened for read AND write, then say last op was read
#ifdef CYGSEM_LIBC_STDIO_WANT_BUFFERED_IO
        flags.last_buffer_op_was_read = true;
#endif
    } // if

    flags.binary = binary ? 1 : 0;

    error = ENOERR;
    
    // in due course we would do an equivalent to fseek(...,0, SEEK_END);
    // when appending. for now, there's nothing, except set eof
    
    flags.at_eof = append ? 1 : 0;

    position = 0;

#ifdef CYGSEM_LIBC_STDIO_WANT_BUFFERED_IO

    switch (buffer_mode) {
    case _IONBF:
        CYG_ASSERT( (buffer_size == 0) && (buffer_addr == NULL),
                    "No buffering wanted but size/address specified!" );
        flags.buffering = flags.line_buffering = false;
        break;
    case _IOLBF:
        flags.buffering = true;
        flags.line_buffering = true;
        break;
    case _IOFBF:
        flags.buffering = true;
        flags.line_buffering = false;
        break;
    default:
        error = EINVAL;
        return;
    } // switch

    // one way of checking the buffer was set up correctly
    if (flags.buffering && io_buf.get_buffer_size()==-1) {
        error = ENOMEM;
        return;
    }

#endif

#if 0 // FIXME - Need to set binary mode.
    if (my_device->open) {
        error = (*my_device->open)( my_device->cookie, 
                                    binary ? CYG_DEVICE_OPEN_MODE_RAW
                                           : CYG_DEVICE_OPEN_MODE_TEXT );
        if (error != ENOERR)
            return; // keep error code the same
    } // if
    
#endif

#ifdef CYGDBG_USE_ASSERTS
    magic_validity_word = 0x7b4321ce;
#endif
    
} // Cyg_StdioStream constructor


Cyg_StdioStream::Cyg_StdioStream( OpenMode open_mode,
                                 cyg_ucount32 buffer_size,
                                  cyg_uint8 *buffer_addr )
#ifdef CYGSEM_LIBC_STDIO_WANT_BUFFERED_IO
    : io_buf( buffer_size, buffer_addr )
#endif
{
    initialize( CYG_STDIO_HANDLE_NULL, open_mode, false, false, _IOFBF,
                buffer_size, buffer_addr );

    if( error != ENOERR )
        return;
    
    switch( open_mode )
    {
    case CYG_STREAM_READ:
        // Fix up the stream so it looks like the buffer contents has just
        // been read in.
#ifdef CYGSEM_LIBC_STDIO_WANT_BUFFERED_IO
        io_buf.set_bytes_written( buffer_size );
#endif
        break;
        
    case CYG_STREAM_WRITE:
        // Fix up the stream so it looks like the buffer is ready to accept
        // new data.
        break;

    default:
        error = EINVAL;
        return;
    }
}


Cyg_ErrNo
Cyg_StdioStream::refill_read_buffer( void )
{
    Cyg_ErrNo read_err;
    cyg_uint8 *buffer;
    cyg_uint32 len;

    CYG_ASSERTCLASS( this, "Stream object is not a valid stream!" );
    
    if (!lock_me())
        return EBADF;  // assume file is now invalid

    // first just check that we _can_ read this device!
    if (!flags.opened_for_read) {
        unlock_me();
        return EINVAL;
    }
    
#ifdef CYGSEM_LIBC_STDIO_WANT_BUFFERED_IO
    // If there is pending output to write, then this will check and
    // write it
    if (flags.buffering) {
        read_err = flush_output_unlocked();

        // we're now reading
        flags.last_buffer_op_was_read = true;

        // flush ALL streams
        if (read_err == ENOERR)
            read_err = cyg_libc_stdio_flush_all_but(this);

        if (read_err != ENOERR) {
            unlock_me();
            return read_err;
        } // if

        len = io_buf.get_buffer_addr_to_write( (cyg_uint8**)&buffer );
        if (!len) { // no buffer space available
            unlock_me();
            return ENOERR;  // isn't an error, just needs user to read out data
        } // if
    }
    else
#endif

    if (!flags.readbuf_char_in_use) {
        len = 1;
        buffer = &readbuf_char;
    }
    else {
        // no buffer space available
        unlock_me();
        return ENOERR;  // isn't an error, just needs user to read out data
    } // else

    read_err = cyg_stdio_read(my_device, buffer, &len);


#ifdef CYGSEM_LIBC_STDIO_WANT_BUFFERED_IO
    if (flags.buffering)
        io_buf.set_bytes_written( len );
    else
#endif
        flags.readbuf_char_in_use = len ? 1 : 0;

    unlock_me();

    if (read_err == ENOERR) {
        if (len == 0) {
            read_err = EAGAIN;
            flags.at_eof = true;
        }
        else
            flags.at_eof = false;
    } // if
    
    return read_err;
} // refill_read_buffer()


Cyg_ErrNo
Cyg_StdioStream::read( cyg_uint8 *user_buffer, cyg_ucount32 buffer_length,
                       cyg_ucount32 *bytes_read )
{
    Cyg_ErrNo read_err=ENOERR;
    CYG_ASSERTCLASS( this, "Stream object is not a valid stream!" );
    
    *bytes_read = 0;

    if (!lock_me())
        return EBADF;  // assume file is now invalid

    if (!flags.opened_for_read) {
        unlock_me();
        return EINVAL;
    }

#ifdef CYGFUN_LIBC_STDIO_ungetc
    if (flags.unread_char_buf_in_use && buffer_length) {
        *user_buffer++ = unread_char_buf;
        ++*bytes_read;
        flags.unread_char_buf_in_use = false;
        --buffer_length;
    } // if

#endif // ifdef CYGFUN_LIBC_STDIO_ungetc

#ifdef CYGSEM_LIBC_STDIO_WANT_BUFFERED_IO
    if (flags.buffering) {

        // need to flush output if we were writing before
        if (!flags.last_buffer_op_was_read) {
            Cyg_ErrNo err = flush_output_unlocked();

            if (ENOERR != err) {
                unlock_me();
                return err;
            }            
        }
            
        cyg_uint8 *buff_to_read_from;
        cyg_ucount32 bytes_available;
    
        bytes_available = io_buf.get_buffer_addr_to_read(
              (cyg_uint8 **)&buff_to_read_from );
        
        cyg_ucount32 count =
            (bytes_available < buffer_length) ? bytes_available : buffer_length;

        if (count) {
            memcpy( user_buffer, buff_to_read_from, count );
            io_buf.set_bytes_read( count );
            *bytes_read += count;
        } // if

    } // if
    else
        
#endif

    if (flags.readbuf_char_in_use && buffer_length) {
        *user_buffer = readbuf_char;
        *bytes_read = 1;
        flags.readbuf_char_in_use = false;
    }

    position += *bytes_read;
    

    // if we are unbuffered, we read as much as we can directly from the 
    // file system at this point.
    //
    // unless we do this, we could end up reading byte-by-byte from the filing system
    // due to the readbuf_char scheme.
    if (!flags.buffering && (*bytes_read<buffer_length)) {
        cyg_uint32 len;
        len=buffer_length-*bytes_read;
        read_err = cyg_stdio_read(my_device, user_buffer + *bytes_read, &len);      
        *bytes_read+=len;
    }
    
    unlock_me();

    return read_err;
} // read()


Cyg_ErrNo
Cyg_StdioStream::read_byte( cyg_uint8 *c )
{
    Cyg_ErrNo err=ENOERR;

    CYG_ASSERTCLASS( this, "Stream object is not a valid stream!" );
    
    if (!lock_me())
        return EBADF;  // assume file is now invalid

    if (!flags.opened_for_read) {
        unlock_me();
        return EINVAL;
    }

# ifdef CYGFUN_LIBC_STDIO_ungetc
    if (flags.unread_char_buf_in_use) {
        *c = unread_char_buf;
        flags.unread_char_buf_in_use = false;
        position++;
        unlock_me();
        return ENOERR;
    } // if
# endif // ifdef CYGFUN_LIBC_STDIO_ungetc

#ifdef CYGSEM_LIBC_STDIO_WANT_BUFFERED_IO
    if (flags.buffering) {
        // need to flush output if we were writing before
        if (!flags.last_buffer_op_was_read)
            err = flush_output_unlocked();

        if (ENOERR != err) {
            unlock_me();
            return err;
        }            
            
        cyg_uint8 *buff_to_read_from;
        cyg_ucount32 bytes_available;
    
        bytes_available=io_buf.get_buffer_addr_to_read(&buff_to_read_from);

        if (bytes_available) {
            *c = *buff_to_read_from;
            io_buf.set_bytes_read(1);
            position++;
        }
        else
            err = EAGAIN;
    } // if
    else
    
#endif


    if (flags.readbuf_char_in_use) {
        *c = readbuf_char;
        flags.readbuf_char_in_use = false;
        position++;
    }
    else
        err = EAGAIN;

    unlock_me();

    return err;
} // read_byte()


Cyg_ErrNo
Cyg_StdioStream::peek_byte( cyg_uint8 *c )
{
    Cyg_ErrNo err=ENOERR;

    CYG_ASSERTCLASS( this, "Stream object is not a valid stream!" );
    
    if (!lock_me())
        return EBADF;  // assume file is now invalid

    if (!flags.opened_for_read) {
        unlock_me();
        return EINVAL;
    }

    // this should really only be called after refill_read_buffer, but just
    // in case
#ifdef CYGSEM_LIBC_STDIO_WANT_BUFFERED_IO
    if (flags.buffering)
        err = flush_output_unlocked();

    if (err != ENOERR)
        return err;

    // we're now reading
    flags.last_buffer_op_was_read = true;
#endif

# ifdef CYGFUN_LIBC_STDIO_ungetc
    if (flags.unread_char_buf_in_use) {
        *c = unread_char_buf;
        unlock_me();
        return ENOERR;
    } // if
# endif // ifdef CYGFUN_LIBC_STDIO_ungetc

#ifdef CYGSEM_LIBC_STDIO_WANT_BUFFERED_IO
    if (flags.buffering) {
        cyg_uint8 *buff_to_read_from;
        cyg_ucount32 bytes_available;
    
        bytes_available=io_buf.get_buffer_addr_to_read(&buff_to_read_from);

        if (bytes_available) {
            *c = *buff_to_read_from;
        }
        else
            err = EAGAIN;
    } // if
    else
    
#endif


    if (flags.readbuf_char_in_use) {
        *c = readbuf_char;
    }
    else
        err = EAGAIN;

    unlock_me();

    return err;
} // peek_byte()


Cyg_ErrNo
Cyg_StdioStream::flush_output_unlocked( void )
{
#ifdef CYGSEM_LIBC_STDIO_WANT_BUFFERED_IO
    Cyg_ErrNo write_err=ENOERR;
    cyg_uint8 *buffer;
    cyg_uint32 len;

    CYG_ASSERTCLASS( this, "Stream object is not a valid stream!" );
    
    if ( flags.last_buffer_op_was_read )
        return ENOERR;

    // first just check that we _can_ write to the device!
    if ( !flags.opened_for_write )
        return EINVAL;

    // shortcut if nothing to do
    if (io_buf.get_buffer_space_used() == 0)
        return ENOERR;
        
    len = io_buf.get_buffer_addr_to_read( (cyg_uint8 **)&buffer );
    
    CYG_ASSERT( len > 0, 
                "There should be data to read but there isn't!");

    write_err = cyg_stdio_write(my_device, buffer, &len);

    // since we're doing a concerted flush, we tell the I/O layer to
    // flush too, otherwise output may just sit there forever
    if (!write_err)
        cyg_stdio_flush( my_device );
    
        // we've just read it all, so just wipe it out
    io_buf.drain_buffer();

    return write_err;

#else // ifdef CYGSEM_LIBC_STDIO_WANT_BUFFERED_IO

    CYG_ASSERTCLASS( this, "Stream object is not a valid stream!" );
    
    return ENOERR;

#endif // ifdef CYGSEM_LIBC_STDIO_WANT_BUFFERED_IO
} // flush_output_unlocked()



Cyg_ErrNo
Cyg_StdioStream::write( const cyg_uint8 *buffer,
                        cyg_ucount32 buffer_length,
                        cyg_ucount32 *bytes_written )
{
    Cyg_ErrNo write_err = ENOERR;

    CYG_ASSERTCLASS( this, "Stream object is not a valid stream!" );
    
    *bytes_written = 0;

    if (!lock_me())
        return EBADF;  // assume file is now invalid

    // first just check that we _can_ write to the device!
    if ( !flags.opened_for_write ) {
        unlock_me();
        return EINVAL;
    }

#ifdef CYGSEM_LIBC_STDIO_WANT_BUFFERED_IO
    if (flags.last_buffer_op_was_read == true)
        io_buf.drain_buffer();  // nuke input bytes to prevent confusion

    flags.last_buffer_op_was_read = false;

    if (!flags.buffering) {
#endif
        cyg_uint32 len = buffer_length;

        write_err = cyg_stdio_write(my_device, buffer, &len);

        *bytes_written = len;

#ifdef CYGSEM_LIBC_STDIO_WANT_BUFFERED_IO
    } // if
    else {
        cyg_ucount32 bytes_available;
        cyg_ucount32 bytes_to_write;
        cyg_ucount32 newline_pos;
        cyg_uint8 *write_addr;
        cyg_bool must_flush = false;
        
        while ( buffer_length > 0 ) {
            bytes_available =
                io_buf.get_buffer_addr_to_write( &write_addr );
            
            // we need to flush if we've no room or the buffer has an up
            // and coming newline
            if ( !bytes_available || must_flush ) {
                write_err = flush_output_unlocked();
                
                // harmless even if there was an error
                bytes_available =
                    io_buf.get_buffer_addr_to_write( &write_addr );

                CYG_ASSERT( bytes_available > 0,
                            "Help! still no bytes available in "
                            "write buffer" );
            } // if
            
            if (write_err) {
                unlock_me();
                return write_err;
            } // if
            
            // choose the lower of the buffer available and the length
            // to write
            bytes_to_write=(bytes_available < buffer_length) 
                ? bytes_available
                : buffer_length;
        
            // if we're line buffered, we may want want to flush if there's
            // a newline character, so lets find out
        
            if (flags.line_buffering) {
                for (newline_pos=0;
                     newline_pos<bytes_to_write;
                     newline_pos++) {
                    if (buffer[newline_pos] == '\n') {
                        break;
                    } // if
                } // for
                // if we didn't reach the end
                if (newline_pos != bytes_to_write) {
                    // shrink bytes_to_write down to the bit we need to
                    // flush including the newline itself
                    bytes_to_write = newline_pos + 1;
                    must_flush = true;
                } // if
            } // if
            
            memcpy( write_addr, buffer, bytes_to_write );
            
            *bytes_written += bytes_to_write;
            buffer += bytes_to_write;
            buffer_length -= bytes_to_write;
            io_buf.set_bytes_written( bytes_to_write );

            position += bytes_to_write;
            
        } // while
        
        if ( must_flush ) {
            write_err = flush_output_unlocked();
        } // if
    } // else
#endif // ifdef CYGSEM_LIBC_STDIO_WANT_BUFFERED_IO

    unlock_me();

    return write_err;
} // write()

// EOF stream.cxx
