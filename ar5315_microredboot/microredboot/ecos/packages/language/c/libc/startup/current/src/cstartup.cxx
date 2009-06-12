//=======================================================================
//
//      cstartup.cxx
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
// Purpose:     
// Description: 
// Usage:       
//
//####DESCRIPTIONEND####
//
//========================================================================

// CONFIGURATION

#include <pkgconf/system.h>         // CYGPKG_KERNEL
#include <pkgconf/libc_startup.h>   // Configuration header

// INCLUDES

#include <cyg/infra/cyg_type.h>     // Common type definitions and support
#include <cyg/infra/cyg_trac.h>     // Common tracing support

#ifdef CYGPKG_KERNEL
# include <pkgconf/kernel.h>        // eCos kernel configuration
# include <cyg/kernel/thread.hxx>   // eCos thread support
# include <cyg/kernel/thread.inl>
#endif


// FUNCTION PROTOTYPES

externC void
cyg_libc_invoke_main( CYG_ADDRWORD );

// EXTERNS

#ifdef CYGSEM_LIBC_STARTUP_MAIN_THREAD
extern Cyg_Thread cyg_libc_main_thread;

// FUNCTIONS

externC void
cyg_iso_c_start( void )
{
    static int initialized=0;

    CYG_REPORT_FUNCNAME( "cyg_iso_c_start" );
    CYG_REPORT_FUNCARGVOID();

    if (initialized++ == 0) {
        CYG_TRACE0( true, "Resuming cyg_libc_main_thread" );
        cyg_libc_main_thread.resume();
    }
    CYG_REPORT_RETURN();

} // cyg_iso_c_start()

// define an object that will automatically call cyg_iso_c_start()

class cyg_libc_startup_dummy_constructor_class {
public:
    cyg_libc_startup_dummy_constructor_class() { cyg_iso_c_start(); }
};

static cyg_libc_startup_dummy_constructor_class cyg_libc_startup_obj
                                  CYGBLD_ATTRIB_INIT_PRI(CYG_INIT_LIBC);

#elif defined( CYGSEM_LIBC_STARTUP_MAIN_INITCONTEXT )

// otherwise replace the default cyg_user_start(), but of course keep
// it a weak symbol in case the user wants to override

externC void
cyg_user_start(void) CYGBLD_ATTRIB_WEAK;

externC void
cyg_user_start(void)
{
    cyg_libc_invoke_main(0);
}

externC void
cyg_iso_c_start( void )
{
    static int initialized=0;

    CYG_REPORT_FUNCNAME( "cyg_iso_c_start" );
    CYG_REPORT_FUNCARGVOID();

    // In case they want to explicitly invoke the C library from
    // cyg_user_start() themselves
    if (initialized++ == 0) {
        cyg_libc_invoke_main(0);
    }
    CYG_REPORT_RETURN();
}
#else

externC void
cyg_iso_c_start( void ) {}

#endif

// EOF cstartup.cxx
