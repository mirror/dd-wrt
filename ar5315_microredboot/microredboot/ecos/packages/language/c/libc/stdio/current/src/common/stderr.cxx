//========================================================================
//
//      stderr.cxx
//
//      Initialization of stderr stream for ISO C library
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
// Purpose:       Initialization of stderr stream for ISO C library
// Description:   We put all this in a separate file in the hope that if
//                no-one uses stderr, then it is not included in the image
// Usage:         
//
//####DESCRIPTIONEND####
//
//========================================================================

// CONFIGURATION

#include <pkgconf/libc_stdio.h>          // Configuration header

// Don't let us get the stream definitions of stdin/out/err from stdio.h
// since we are going to break the type safety :-(
#define CYGPRI_LIBC_STDIO_NO_DEFAULT_STREAMS

// And we don't need the stdio inlines, which will otherwise complain if
// stdin/out/err are not available
#ifdef CYGIMP_LIBC_STDIO_INLINES
# undef CYGIMP_LIBC_STDIO_INLINES
#endif

// INCLUDES

#include <cyg/infra/cyg_type.h>           // Common type definitions and support
#include <cyg/libc/stdio/stream.hxx>      // Cyg_StdioStream class
#include <cyg/libc/stdio/stdiofiles.hxx>  // C library files
#include <cyg/libc/stdio/stdiosupp.hxx>   // Cyg_libc_stdio_find_filename()

// CONSTANTS

// add 2 to the priority to allow it to be the fd 2 if necessary
#define PRIO (CYG_INIT_LIBC+2)

// STATICS

Cyg_StdioStream
cyg_libc_stdio_stderr CYGBLD_ATTRIB_INIT_PRI(PRIO) = Cyg_StdioStream(
    Cyg_libc_stdio_find_filename(CYGDAT_LIBC_STDIO_DEFAULT_CONSOLE,
                                 Cyg_StdioStream::CYG_STREAM_WRITE,
                                 false, false),
    Cyg_StdioStream::CYG_STREAM_WRITE, false, false, _IONBF, 0, NULL );

// CLASSES

// This is a dummy class just so we can execute arbitrary code when
// stderr is requested

class cyg_libc_dummy_stderr_init_class {
public:
    cyg_libc_dummy_stderr_init_class(void) { 
        Cyg_libc_stdio_files::set_file_stream(2, &cyg_libc_stdio_stderr);
    }
};

// And here's an instance of the class just to make the code run
static cyg_libc_dummy_stderr_init_class cyg_libc_dummy_stderr_init
                                             CYGBLD_ATTRIB_INIT_PRI(PRIO);

// and finally stderr itself
__externC Cyg_StdioStream * const stderr;
Cyg_StdioStream * const stderr=&cyg_libc_stdio_stderr;

// EOF stderr.cxx
