//========================================================================
//
//      startup.cxx
//
//      General startup code for the target machine
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
// Contributors:  jlarmour
// Date:          1999-01-21
// Purpose:       This provides generic startup code for the eCos system.
// Description:   We start the system with the entry point cyg_start()
//                which is called from the eCos HALs. This in turn invokes
//                cyg_prestart(), cyg_package_start() and cyg_user_start().
//                All these can be overriden by the user.
// Usage:         Override the defaults to use your own startup code,
//                which will allow you to take complete control after the
//                HAL initialization
//
//####DESCRIPTIONEND####
//
//========================================================================

// CONFIGURATION

#include <pkgconf/infra.h>   // Configuration header

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common type definitions and support
#include <cyg/infra/cyg_trac.h>    // Default tracing support

#ifdef CYGPKG_KERNEL
# include <pkgconf/kernel.h>
# include <cyg/kernel/sched.hxx>
# include <cyg/kernel/sched.inl>
#endif

// FUNCTION PROTOTYPES

externC void
cyg_start( void ) CYGBLD_ATTRIB_WEAK;

externC void
cyg_prestart( void );

externC void
cyg_package_start( void );

externC void
cyg_user_start( void );


// FUNCTIONS

void
cyg_start( void )
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARGVOID();

    cyg_prestart();

    cyg_package_start();

    cyg_user_start();

#ifdef CYGPKG_KERNEL
    Cyg_Scheduler::start();
#endif

    CYG_REPORT_RETURN();
} // cyg_start()

// EOF startup.cxx
