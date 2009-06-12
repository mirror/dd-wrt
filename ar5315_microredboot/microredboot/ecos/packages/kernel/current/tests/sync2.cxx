//==========================================================================
//
//        sync2.cxx
//
//        Sync test 2 -- test of different locking mechanisms
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
// Date:          1998-02-18
// Description: 
//     Creates some threads and tests the various synchronization
//     mechanisms.  Four threads are created t0..t3.  t0 and t3 grab a
//     mutex and check they have exclusive access to shared variable.
//     t0,t1,t2 post each other in a loop with a semaphore so that
//     only one is running at any time.  t1,t2,t3 do a similar thing
//     with counting semaphores, except that there are two active
//     threads.
// Omissions:
//     Doesn't test condition variables
//                
//####DESCRIPTIONEND####

#include <pkgconf/kernel.h>

#include <cyg/kernel/thread.hxx>
#include <cyg/kernel/thread.inl>
#include <cyg/kernel/sched.hxx>
#include <cyg/kernel/mutex.hxx>
#include <cyg/kernel/sema.hxx>

#include <cyg/infra/testcase.h>

#include <cyg/kernel/sched.inl>

#define NTHREADS 4

#include "testaux.hxx"

static Cyg_Mutex m0;
static Cyg_Binary_Semaphore s0, s1, s2(1);
static Cyg_Counting_Semaphore cs0, cs1, cs2, cs3;

static const cyg_ucount16 n = 1000; 
static cyg_ucount8 m0d=99, sd=2, cd0=99, cd1=99;

static void entry0( CYG_ADDRWORD data )
{
    for(cyg_ucount16 i=0; i<n; i++) {
        s2.wait();
        CHECK( 2 == sd );
        sd = 0;
        m0.lock(); {
            m0d = 0;
            s0.post();
            CHECK( 0 == m0d );
        } m0.unlock();
    }
    // wait for 3 explicit posts to indicate threads have stopped.
    for(cyg_ucount8 i=0; i<3; i++)
        cs3.wait();

    CHECK( ! s0.posted() );
    CHECK( ! s1.posted() );
    CHECK(   s2.posted() );

    CHECK( 0 == cs0.peek() );
    CHECK( 0 == cs1.peek() );
    CHECK( 0 == cs2.peek() );
    CHECK( 0 == cs3.peek() );

    CHECK( 0 == cd0 );
    CHECK( 0 == cd1 );
    CYG_TEST_PASS_FINISH("Sync 2 OK");
    CYG_TEST_FAIL_FINISH("Not reached");
}

static void entry1( CYG_ADDRWORD data )
{
    for(cyg_ucount16 i=0; i<n; i++) {
        s0.wait();
        CHECK( 0 == sd );
        sd = 1;
        cd0 = 1;
        cs1.post();
        cd1 = 1;
        cs1.post();
        s1.post();
        cs0.wait();
        CHECK( 0 == cd0 );
        cs0.wait();
        CHECK( 0 == cd1 );
    }
    cs3.post();
    s0.wait();
    CYG_TEST_FAIL_FINISH("Not reached");
}

static void entry2( CYG_ADDRWORD data )
{
    for(cyg_ucount16 i=0; i<n; i++) {
        s1.wait();
        CHECK( 1 == sd );
        sd = 2;
        cs1.wait();
        CHECK( 1 == cd0 );
        cd0 = 2;
        cs2.post();
        s2.post();
        cs1.wait();
        CHECK( 1 == cd1 );
        cd1 = 2;
        cs2.post();
    }
    cs3.post();
    s1.wait();
    CYG_TEST_FAIL_FINISH("Not reached");
}

static void entry3( CYG_ADDRWORD data )
{
    for(cyg_ucount16 i=0; i < n*2; i++)  {
        cs2.wait();
        CHECK( 2 == cd0 || 2 == cd1 );
        m0.lock(); {
            m0d = 3;
            if( 2 == cd0 ) 
                cd0 = 0;
            else {
                CHECK( 2 == cd1 );
                cd1 = 0;
            }
            cs0.post();
            CHECK( 3 == m0d );
        } m0.unlock();
    }
    cs3.post();
    cs1.wait();
    CYG_TEST_FAIL_FINISH("Not reached");
}


void sync2_main(void)
{
    CYG_TEST_INIT();

    new_thread(entry0, 0);
    new_thread(entry1, 1);
    new_thread(entry2, 2);
    new_thread(entry3, 3);

    Cyg_Scheduler::start();

    CYG_TEST_PASS_FINISH("Not reached");
}

externC void
cyg_start( void )
{
#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
    cyg_hal_invoke_constructors();
#endif
    sync2_main();
}

// EOF sync2.cxx
