//========================================================================
//
//      _exit.cxx
//
//      _exit() as from POSIX 1003.1 section 3.2.2 "Terminate a process"
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
// Date:          2000-04-28
// Purpose:       Provides POSIX 1003.1 _exit() function
// Description: 
// Usage:       
//
//####DESCRIPTIONEND####
//
//========================================================================

// CONFIGURATION

#include <pkgconf/libc_startup.h>          // Configuration header
#include <pkgconf/system.h>

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common type definitions and support
#include <cyg/infra/cyg_trac.h>    // Common tracing support
#include <cyg/infra/cyg_ass.h>     // Common assertion support
#include <stdlib.h>                // Header for all stdlib functions
                                   // (like this one)
#ifdef CYGPKG_KERNEL
# include <pkgconf/kernel.h>       // kernel configuration
# include <cyg/kernel/thread.hxx>  // eCos kernel for thread termination
# include <cyg/kernel/thread.inl>  // and
# include <cyg/kernel/sched.hxx>   // for stopping the scheduler
# include <cyg/kernel/sched.inl>
#endif


// FUNCTIONS

externC void
_exit( int status )
{
    CYG_REPORT_FUNCTION(); // shouldn't return, but CYG_FAIL will catch it
    CYG_REPORT_FUNCARG1DV( status );

    CYG_ASSERT( status == 0, "Program _exiting with non-zero error status");
    
#ifdef CYGPKG_KERNEL

# ifdef CYGSEM_LIBC_EXIT_STOPS_SYSTEM

    Cyg_Scheduler::lock(); // prevent rescheduling

    for (;;)
        CYG_EMPTY_STATEMENT;

# else

    Cyg_Thread::exit();

# endif
#endif

    // loop forever
    for (;;)
        CYG_EMPTY_STATEMENT;

    CYG_FAIL( "_exit() returning!!!" );

    CYG_REPORT_RETURN();
} // _exit()


// EOF _exit.cxx
