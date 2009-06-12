//===========================================================================
//
//      strlen.cxx
//
//      ANSI standard strlen() routine
//
//===========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Nick Garnett <nickg@calivar.com>
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
// Contributors: nickg
// Date:         2000-04-14
// Purpose:      
// Description:  strlen() function, to satisfy C++ runtime needs. This
//               function functions correctly, but in the interests of
//               keeping code size down, it uses the smallest implementation
//               possible, and is consequently not very fast.
// Usage:       
//
//####DESCRIPTIONEND####
//
//===========================================================================

// CONFIGURATION

#include <pkgconf/system.h>   // Configuration header
#include <pkgconf/infra.h>   // Configuration header

//==========================================================================
// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common type definitions
#include <cyg/infra/cyg_trac.h>    // Tracing support
#include <cyg/infra/cyg_ass.h>     // Assertion support
#include <stddef.h>                // Compiler definitions such as size_t, NULL etc.

//==========================================================================
// EXPORTED SYMBOLS

extern "C" size_t
strlen( const char *s ) CYGBLD_ATTRIB_WEAK_ALIAS(__strlen);

//==========================================================================
// FUNCTIONS

extern "C" size_t
__strlen( const char *s )
{
    int retval;
    
    CYG_REPORT_FUNCNAMETYPE( "__strlen", "returning length %d" );
    CYG_REPORT_FUNCARG1( "s=%08x", s );

    CYG_CHECK_DATA_PTR( s, "s is not a valid pointer!" );

    const char *start = s;
    
    while (*s)
        s++;
    
    retval = s - start;

    CYG_REPORT_RETVAL( retval );

    return retval;

} // __strlen()

//==========================================================================
// EOF strlen.cxx
