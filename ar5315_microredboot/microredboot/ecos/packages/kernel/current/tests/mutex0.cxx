//==========================================================================
//
//        mutex0.cxx
//
//        Mutex and condition variable test 0
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     dsm
// Contributors:    dsm
// Date:          1998-02-24
// Description:   Limited to checking constructors/destructors
//####DESCRIPTIONEND####

#include <pkgconf/kernel.h>

#include <cyg/kernel/mutex.hxx>

#include <cyg/infra/testcase.h>

#include "testaux.hxx"

static Cyg_Mutex mutex0;

static Cyg_Condition_Variable cvar0( mutex0 );

static bool flash( void )
{
    Cyg_Mutex m0;

    CYG_ASSERTCLASSO(m0, "error");

    Cyg_Condition_Variable cv0( m0 );

    CYG_ASSERTCLASSO(cv0, "error");

    return true;
}

void mutex0_main( void )
{
    CYG_TEST_INIT();

    CHECK(flash());
    CHECK(flash());
    
    CYG_ASSERTCLASSO(mutex0, "error");
    CYG_ASSERTCLASSO(cvar0, "error");
    
    CYG_TEST_PASS_FINISH("Mutex 0 OK");

}

externC void
cyg_start( void )
{
#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
    cyg_hal_invoke_constructors();
#endif
    mutex0_main();
}   
// EOF mutex0.cxx
