//========================================================================
//
//      fflush.cxx
//
//      Implementation of C library file flush function as per ANSI 7.9.5.2
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
// Purpose:       Provides ISO C fflush() function
// Description: 
// Usage:       
//
//####DESCRIPTIONEND####
//
//========================================================================

// CONFIGURATION

#include <pkgconf/libc_stdio.h>   // Configuration header

// INCLUDES

#include <cyg/infra/cyg_type.h>     // Common project-wide type definitions
#include <stddef.h>                 // NULL and size_t from compiler
#include <errno.h>                  // Error codes
#include <stdio.h>                  // header for fflush()
#include <cyg/libc/stdio/stream.hxx>// C library streams
#include <cyg/libc/stdio/stdiofiles.hxx>  // C library files
#include <cyg/libc/stdio/stdiosupp.hxx>   // support for stdio

// FUNCTIONS

// flush all but one stream
externC Cyg_ErrNo
cyg_libc_stdio_flush_all_but( Cyg_StdioStream *not_this_stream )
{
    cyg_bool files_flushed[FOPEN_MAX] = { false }; // sets all to 0
    cyg_bool loop_again, looped = false;
    cyg_ucount32 i;
    Cyg_ErrNo err=ENOERR;
    Cyg_StdioStream *stream;

    do {
        loop_again = false;

        for (i=0; (i<FOPEN_MAX) && !err; i++) {
            if (files_flushed[i] == false) {
                
                stream = Cyg_libc_stdio_files::get_file_stream(i);
                
                if ((stream == NULL) || (stream == not_this_stream)) {
                    // if it isn't a valid stream, set its entry in the
                    // list of files flushed since we don't want to
                    // flush it
                    // Ditto if its the one we're meant to miss
                    
                    files_flushed[i] = true;
                } // if
                else {
                    // valid stream
#ifdef CYGSEM_LIBC_STDIO_WANT_BUFFERED_IO
                    // only buffers which we've written to need flushing
                    if ( !stream->flags.last_buffer_op_was_read)
#endif
                    {
                        // we try to flush the first time through so that
                        // everything is flushed that can be flushed.
                        // The second time through we should just wait
                        // in case some other lowerpri thread has locked the
                        // stream, otherwise we will spin needlessly and
                        // never let the lower pri thread run!
                        if ( (looped && stream->lock_me()) || 
                             stream->trylock_me() ) {
                            err = stream->flush_output_unlocked();
                            stream->unlock_me();
                            files_flushed[i] = true;
                        } // if
                        else {
                            loop_again = true;
                            looped = true;
                        }
                    }
                } // else
            } // if
        } // for
    } // do
    while(loop_again && !err);
    
    return err;
} // cyg_libc_stdio_flush_all_but()

externC int
fflush( FILE *stream ) __THROW
{
    Cyg_StdioStream *real_stream = (Cyg_StdioStream *)stream;
    Cyg_ErrNo err;

    if (stream == NULL) {  // tells us to flush ALL streams
        err = cyg_libc_stdio_flush_all_but(NULL);
    } // if
    else {
        err = real_stream->flush_output();
    } // else

    if (err) {
        errno = err;
        return EOF;
    } // if

    return 0;

} // fflush()
        
// EOF fflush.cxx
