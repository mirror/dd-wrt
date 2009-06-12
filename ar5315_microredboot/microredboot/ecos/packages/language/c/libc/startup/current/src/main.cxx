//=======================================================================
//
//      main.cxx
//
//      Default main() function
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
// Purpose:       Provide a default empty main() function
// Description:   This file provides a default empty main() function so
//                that things don't fall apart if the user starts the
//                ISO C environment (the default setting) but doesn't
//                provide their own main(). This is taken advantage of
//                in some tests (for example)
// Usage:         Obviously simply override main() in your own program to
//                prevent the use of the one below
//
//####DESCRIPTIONEND####
//
//========================================================================

// CONFIGURATION

#include <pkgconf/system.h>        // for CYGPKG_KERNEL

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common type definitions and support
#include <cyg/infra/cyg_trac.h>    // Common tracing support
#include <cyg/infra/cyg_ass.h>     // Common assertion support

// FUNCTION PROTOTYPES

// We provide a weakly named main to allow this to link if the user
// doesn't provide their own main. This isn't strictly good behaviour,
// but if the user wants good performance then of _course_ they should
// play with the config options and this won't be called. Or it might
// be "giving them enough rope" etc. :-)

externC int
main( int argc, char *argv[] ) __attribute__((weak));

externC void
cyg_user_start(void);

// FUNCTIONS

externC int
main( int argc, char *argv[] )
{
    CYG_REPORT_FUNCNAMETYPE("main", "returning %d" );

    // try to be helpful by diagnosing malformed arguments
    CYG_PRECONDITION( argv != NULL, "argv is NULL!" );
    CYG_PRECONDITION( argv[argc] == NULL, "argv[argc] isn't NULL!" );

    CYG_REPORT_FUNCARG2("argc=%d, argv[0]=%s", argc,
                        (CYG_ADDRWORD)((argv[0]==NULL) ? "NULL" : argv[0]) );

    CYG_TRACE0( true, "This is the system-supplied default main()" );

    // If the kernel isn't present then cyg_libc_invoke_main() will have assumed
    // main() is present and will call it. But if we're here, then
    // evidently the user didn't supply one - which can't be right. So we
    // assume that they have useful code to run in cyg_user_start() instead.
    // Its better than just exiting
#ifndef CYGPKG_KERNEL
    cyg_user_start();
#endif

    CYG_REPORT_RETVAL(0);
    return 0; // some CPUs have 0 hard-wired - faster than a reg
} // main()

// EOF main.cxx
