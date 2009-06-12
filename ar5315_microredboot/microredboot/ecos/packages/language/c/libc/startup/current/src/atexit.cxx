//========================================================================
//
//      atexit.cxx
//
//      Implementation of the atexit() function
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
// Author(s):    jlarmour
// Contributors: 
// Date:         2000-04-30
// Purpose:     
// Description: 
// Usage:       
//
//####DESCRIPTIONEND####
//
//========================================================================

// CONFIGURATION

#include <pkgconf/libc_startup.h>   // Configuration header

// Include atexit() ?
#ifdef CYGFUN_LIBC_ATEXIT

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common type definitions and support
#include <cyg/infra/cyg_trac.h>    // Common tracing support
#include <cyg/infra/cyg_ass.h>     // Common assertion support
#include <stdlib.h>                // Header for all stdlib functions
                                   // (like this one)

// STATICS

// cyg_libc_atexit_handlers contains the functions to call.
// cyg_libc_atexit_handlers_count contains the number of valid handlers
// or you can consider the next free handler slot in
// cyg_libc_atexit_handlers.

static __atexit_fn_t
cyg_libc_atexit_handlers[ CYGNUM_LIBC_ATEXIT_HANDLERS ];

static cyg_ucount32 cyg_libc_atexit_handlers_count;


// FUNCTIONS

externC void
cyg_libc_invoke_atexit_handlers( void )
{
    CYG_REPORT_FUNCNAME( "cyg_libc_invoke_atexit_handlers");
    CYG_REPORT_FUNCARGVOID();
    
    cyg_ucount32 i;

    for (i=cyg_libc_atexit_handlers_count; i>0; --i) {

        CYG_TRACE1( true,
                    "Calling function registered with atexit at addr %08x",
                    cyg_libc_atexit_handlers[i-1] );
        CYG_CHECK_FUNC_PTR( cyg_libc_atexit_handlers[i-1],
                            "Function to call in atexit handler list "
                            "isn't valid! Even though it was when "
                            "entered!" );

        (*cyg_libc_atexit_handlers[i-1])();

    } // for
        
    CYG_REPORT_RETURN();
} // cyg_libc_invoke_atexit_handlers()


externC int
atexit( __atexit_fn_t func_to_register )
{
    CYG_REPORT_FUNCNAMETYPE( "atexit", "returning %d" );
    CYG_REPORT_FUNCARG1XV( func_to_register );

    CYG_CHECK_FUNC_PTR( func_to_register,
                       "atexit() not passed a valid function argument!" );

    // have we any slots left?
    if (cyg_libc_atexit_handlers_count >=
        sizeof(cyg_libc_atexit_handlers)/sizeof(__atexit_fn_t) ) {

        CYG_REPORT_RETVAL( 1 ); 
        return 1; // failure
    } // if

    cyg_libc_atexit_handlers[cyg_libc_atexit_handlers_count++] =
        func_to_register;

    CYG_REPORT_RETVAL(0);
    return 0;
} // atexit()


#endif // ifdef CYGFUN_LIBC_ATEXIT

// EOF atexit.cxx
