//==========================================================================
//
//        thread0.cxx
//
//        Thread test 0
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
// Date:          1998-02-11
// Description:   Limited to checking constructors/destructors
// Omissions:
//     Thread constructors with 2 or 3 args are not supported at time
// of writing test.
//####DESCRIPTIONEND####

#include <pkgconf/kernel.h>

#include <cyg/kernel/thread.hxx>

#include <cyg/infra/testcase.h>

#include <cyg/kernel/thread.inl>

#include "testaux.hxx"

#define STACKSIZE CYGNUM_HAL_STACK_SIZE_TYPICAL

static char stack[STACKSIZE];

static cyg_thread_entry entry;

static void entry( CYG_ADDRWORD data )
{
}

static int *p;

static bool flash( void )
{
#if 0 // no facility to allocate stack exists yet.
    Cyg_Thread t0( entry, 0x111 );

    CYG_ASSERTCLASS(&t0, "error");

    Cyg_Thread t1( entry, (CYG_ADDRWORD)&t0, STACKSIZE );

    CYG_ASSERTCLASS(&t1, "error");
#endif
    Cyg_Thread t2( CYG_SCHED_DEFAULT_INFO,
                   entry, (CYG_ADDRWORD)p, 
                   "thread t2",
                   (CYG_ADDRESS)stack, STACKSIZE );

    CYG_ASSERTCLASS(&t2, "error");

    return true;
}

void thread0_main( void )
{
    CYG_TEST_INIT();

    CHECK(flash());
    CHECK(flash());
    
    CYG_TEST_PASS_FINISH("Thread 0 OK");
    
}

externC void
cyg_start( void )
{ 
#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
    cyg_hal_invoke_constructors();
#endif
    thread0_main();
}
// EOF thread0.cxx
