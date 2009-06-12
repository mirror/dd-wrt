#ifndef CYGONCE_LIBC_STDIO_STDIOSUPP_HXX
#define CYGONCE_LIBC_STDIO_STDIOSUPP_HXX
//========================================================================
//
//      stdiosupp.hxx
//
//      Support for C library standard I/O routines
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
// Usage:         #include <cyg/libc/stdio/stdiosupp.hxx>
//
//####DESCRIPTIONEND####
//
//========================================================================

// CONFIGURATION

#include <pkgconf/libc_stdio.h>   // Configuration header

// INCLUDES

#include <cyg/infra/cyg_type.h>     // common type definitions and support
#include <stdio.h>                  // Main standard I/O header
#include <cyg/libc/stdio/stream.hxx>// Cyg_StdioStream

// FUNCTION PROTOTYPES

//========================================================================

// Miscellaneous support functions

externC cyg_stdio_handle_t
Cyg_libc_stdio_find_filename( const char *filename,
                              const Cyg_StdioStream::OpenMode rw,
                              const cyg_bool binary,
                              const cyg_bool append );

externC Cyg_ErrNo
cyg_libc_stdio_flush_all_but( Cyg_StdioStream *not_this_stream );

#endif // CYGONCE_LIBC_STDIOL_STDIOSUPP_HXX multiple inclusion protection

// EOF stdiosupp.hxx
