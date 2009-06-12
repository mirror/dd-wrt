//========================================================================
//
//      getenv.cxx
//
//      Implementation of the ISO C standard getenv() function
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

#include <pkgconf/libc_stdlib.h>           // Configuration header

// INCLUDES

#include <cyg/infra/cyg_type.h>     // Common type definitions and support
#include <cyg/infra/cyg_ass.h>      // Tracing support
#include <cyg/infra/cyg_trac.h>     // Tracing support
#include <stdlib.h>                 // Main header for stdlib functions
#include <string.h>                 // strchr(), strlen() and strncmp()


// VARIABLES

externC char **environ;   // standard definition of environ

// FUNCTIONS

extern char *
getenv( const char *name )
{
    // This function assumes the POSIX 1003.1 layout of the environment
    // in the environ variable, i.e. extern char **environ is a pointer
    // to an array of character pointers to the environment strings 
    // (POSIX 3.1.2.2 and 2.6).
    //
    // getenv() searches the environment list for a string of the form
    // "name=value" (POSIX 4.6.1). The strings must have this form
    // (POSIX 2.6)

    CYG_REPORT_FUNCNAMETYPE( "getenv", "returning %08x" );

    CYG_PRECONDITION( name != NULL, "getenv() called with NULL string!" );

    CYG_REPORT_FUNCARG1( "name=%s", name );

    CYG_PRECONDITION( environ != NULL,
                      "environ not set i.e. environ == NULL" );

    CYG_CHECK_DATA_PTR( environ, "environ isn't a valid pointer!" );

    CYG_CHECK_DATA_PTR( name, "name isn't a valid pointer!" );

    // check name doesn't contain '='
    CYG_PRECONDITION( strchr(name, '=') == NULL,
                      "name contains '='!" );

    char **env_str_p;
    cyg_ucount32 len = strlen( name );

    for (env_str_p = environ; *env_str_p != NULL; ++env_str_p ) {

        CYG_CHECK_DATA_PTR( env_str_p,
                            "current environment string isn't valid!" );
        
        CYG_CHECK_DATA_PTR( *env_str_p,
                            "current environment string isn't valid!" );
        
        // do we have a match?
        if ( !strncmp(*env_str_p, name, len) ) {
            // but it could be just a prefix i.e. we could have
            // matched MYNAMEFRED when we're looking for just MYNAME, so:

            if ( (*env_str_p)[len] == '=' ) {
                // we got a match
                CYG_REPORT_RETVAL( *env_str_p + len + 1 );

                return (*env_str_p + len + 1);
            } // if
        } // if

        // check the next pointer isn't NULL, as we're about to dereference
        // it
        CYG_ASSERT( env_str_p+1 != NULL,
                    "environment contains a NULL pointer!" );
    } // for

    CYG_REPORT_RETVAL( NULL );
    return NULL;
} // getenv()

// EOF getenv.cxx
