//=======================================================================
//
//      invokemain.cxx
//
//      Support for startup of ISO C environment
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
// Date:          2000-04-30
// Purpose:       Provide entry point for thread which then calls main()
// Description:   cyg_libc_invoke_main() is used as the entry point for
//                the thread object that is created to call the
//                user-supplied main() function. It sets up the arguments
//                (if any) and invokes exit() if main() returns
// Usage:       
//
//####DESCRIPTIONEND####
//
//========================================================================

// CONFIGURATION

#include <pkgconf/libc_startup.h>  // Configuration header
#include <pkgconf/isoinfra.h>

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common type definitions and support
#include <cyg/infra/cyg_trac.h>    // Common tracing support
#include <cyg/infra/cyg_ass.h>     // Common assertion support
#include <stdlib.h>                // exit()

// EXTERNS

externC cyg_bool cyg_hal_stop_constructors;

// FUNCTION PROTOTYPES

externC int
main( int argc, char *argv[] );

externC void
cyg_hal_invoke_constructors(void);

externC void
pthread_exit( void *value_ptr );

// STATICS

#ifdef CYGSEM_LIBC_INVOKE_DEFAULT_STATIC_CONSTRUCTORS
class cyg_libc_dummy_constructor_class {
public:
    cyg_libc_dummy_constructor_class(void) { ++cyg_hal_stop_constructors; }
};

static cyg_libc_dummy_constructor_class cyg_libc_dummy_constructor_obj
                                  CYGBLD_ATTRIB_INIT_PRI(CYG_INIT_PREDEFAULT);
#endif

// FUNCTIONS

externC void
cyg_libc_invoke_main( CYG_ADDRWORD )
{
    CYG_REPORT_FUNCNAME( "cyg_libc_invoke_main" );
    CYG_REPORT_FUNCARG1( "argument is %s", "ignored" );

#ifdef CYGSEM_LIBC_INVOKE_DEFAULT_STATIC_CONSTRUCTORS
    // finish invoking constructors that weren't called by default
    cyg_hal_invoke_constructors();
#endif

    // argv[argc] must be NULL according to the ISO C standard 5.1.2.2.1
    char *temp_argv[] = CYGDAT_LIBC_ARGUMENTS ; 
    int rc;
    
    rc = main( (sizeof(temp_argv)/sizeof(char *)) - 1, &temp_argv[0] );

    CYG_TRACE1( true, "main() has returned with code %d. Calling exit()",
                rc );

#ifdef CYGINT_ISO_PTHREAD_IMPL
    // It is up to pthread_exit() to call exit() if needed
    pthread_exit( (void *)rc );
    CYG_FAIL( "pthread_exit() returned!!!" );
#else
    exit(rc);
    CYG_FAIL( "exit() returned!!!" );
#endif

    CYG_REPORT_RETURN();
    
} // cyg_libc_invoke_main()

// EOF invokemain.cxx
