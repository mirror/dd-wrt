//==========================================================================
//
//        cnt_sem1.cxx
//
//        Counting semaphore test 1
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
// Description:   Tests basic counting semaphore functionality.
//####DESCRIPTIONEND####

#include <pkgconf/kernel.h>

#include <cyg/kernel/thread.hxx>       // Cyg_Thread
#include <cyg/kernel/thread.inl>
#include <cyg/kernel/sched.hxx>        // Cyg_Scheduler::start()

#include <cyg/kernel/sema.hxx>

#include <cyg/infra/testcase.h>

#include <cyg/kernel/sched.inl>


#define NTHREADS 2
#include "testaux.hxx"

static Cyg_Counting_Semaphore s0(0), s1(2), s2;

static volatile cyg_ucount8 q = 0;

static void entry0( CYG_ADDRWORD data )
{
    s0.wait();
    CHECK( 1 == q++ );
    s1.post();
    s0.wait();
    CHECK( 3 == q++ );
    CHECK( 0 == s0.peek() );
    CHECK( ! s0.trywait() );
    s0.post();
    CHECK( 4 == q++ );
    CHECK( 1 == s0.peek() );
    s0.post();
    CHECK( 2 == s0.peek() );
    s1.post();
    CHECK( 0 == s2.peek() );
    s2.wait();
    CHECK( 6 == q++ );
    CYG_TEST_PASS_FINISH("Counting Semaphore 1 OK");
}

static void entry1( CYG_ADDRWORD data )
{
    CHECK( 2 == s1.peek() );
    s1.wait();
    CHECK( 1 == s1.peek() );
    s1.wait();
    CHECK( 0 == q++ );
    CHECK( 0 == s0.peek() );
    s0.post();
    s1.wait();
    CHECK( 2 == q++ );
    s0.post();
    s1.wait();
    CHECK( 5 == q++ );
    CHECK( 2 == s0.peek() );
    CHECK( s0.trywait() );
    CHECK( 1 == s0.peek() );
    CHECK( s0.trywait() );
    CHECK( 0 == s0.peek() );
    s2.post();
    s0.wait();
    CYG_TEST_FAIL_FINISH("Not reached");
}

void cnt_sem1_main( void )
{
    CYG_TEST_INIT();

    new_thread(entry0, 0);
    new_thread(entry1, 1);

    Cyg_Scheduler::start();

    CYG_TEST_FAIL_FINISH("Not reached");
}

externC void
cyg_start( void )
{ 
#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
    cyg_hal_invoke_constructors();
#endif
    cnt_sem1_main();
}

// EOF cnt_sem1.cxx
