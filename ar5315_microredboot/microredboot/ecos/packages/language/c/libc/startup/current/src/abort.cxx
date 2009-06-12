//===========================================================================
//
//      abort.cxx
//
//      Implementation of the abort() function
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
// Author(s):     jlarmour
// Contributors:  
// Date:          2000-04-28
// Purpose:       Implement the ISO C abort() function from 7.10.4.1
// Description:   This implements abort() simply by raising SIGABRT as
//                required by ISO C
// Usage:       
//
//####DESCRIPTIONEND####
//
//===========================================================================

// CONFIGURATION

#include <pkgconf/libc_startup.h>   // Configuration header
#include <pkgconf/isoinfra.h>       // CYGINT_ISO_SIGNAL_IMPL

// INCLUDES

#include <cyg/infra/cyg_type.h> // Common type definitions and support
#include <cyg/infra/cyg_trac.h> // Tracing support

#include <stdlib.h>             // Header for all stdlib functions
                                // (like this one)
#if CYGINT_ISO_SIGNAL_IMPL > 0
# include <signal.h>
#endif

// FUNCTIONS

externC void
abort( void )
{
    CYG_REPORT_FUNCNAME( "abort" );
    
#if CYGINT_ISO_SIGNAL_IMPL > 0
    int rc;
    
    rc = raise(SIGABRT);
    
    CYG_TRACE1(1, "raise(SIGABRT) returned!!! rc=%d", rc);

    CYG_FAIL("raise(SIGABRT) returned");

#endif

    // ISO C clearly says that abort() cannot return to its caller

    // loop forever
    for (;;)
        CYG_EMPTY_STATEMENT;

    CYG_REPORT_RETURN();
} // abort()

// EOF abort.cxx
