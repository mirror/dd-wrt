//=========================================================================
//
//      exit.cxx
//
//      Implementation of the exit() function
//
//=========================================================================
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
//=========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    jlarmour
// Contributors: 
// Date:         2000-04-30
// Purpose:     
// Description: 
// Usage:       
//
//####DESCRIPTIONEND####
//
//=========================================================================

// CONFIGURATION

#include <pkgconf/libc_startup.h>   // Configuration header

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common type definitions and support
#include <cyg/infra/cyg_trac.h>    // Common tracing support
#include <cyg/infra/cyg_ass.h>     // Common assertion support
#include <stdio.h>                 // fflush()
#include <errno.h>                 // errno
#include <stdlib.h>                // Header for all stdlib functions
                                   // (like this one)
// EXTERNAL PROTOTYPES

externC void
cyg_libc_invoke_atexit_handlers( void );

// FUNCTIONS

externC void
exit( int status )
{
    CYG_REPORT_FUNCTION(); // shouldn't return, but CYG_FAIL will catch it
    CYG_REPORT_FUNCARG1DV( status );
    
    // Strictly the only thing exit() does is invoke the atexit handlers
    // and flush stdio buffers. Anything else is for _exit() 
    // within the implementation)

#ifdef CYGFUN_LIBC_ATEXIT
    // we start with the atexit handlers
    cyg_libc_invoke_atexit_handlers();
#endif

#ifdef CYGSEM_LIBC_EXIT_CALLS_FFLUSH

    int rc;

    CYG_TRACE0( true, "Calling fflush( NULL )" );
    rc = fflush( NULL );

    CYG_TRACE2( rc != 0, "fflush() returned non-zero. It returned %d and "
                "errno indicates the error: %s", rc, strerror(errno) );
#endif

    _exit( status );

    CYG_FAIL( "__libc_exit() returning!!!" );

    CYG_REPORT_RETURN();
} // __libc_exit()

// EOF exit.cxx
