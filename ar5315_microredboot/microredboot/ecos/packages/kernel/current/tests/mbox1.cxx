//==========================================================================
//
//        mbox1.cxx
//
//        Mbox test 1
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
// Author:        dsm
// Contributors:    dsm
// Date:          1998-05-19
// Description:   Tests basic mbox functionality.
//####DESCRIPTIONEND####

#include <pkgconf/kernel.h>

#include <cyg/kernel/thread.hxx>        // Cyg_Thread
#include <cyg/kernel/thread.inl>
#include <cyg/kernel/sched.hxx>         // Cyg_Scheduler::start()

#include <cyg/kernel/mbox.hxx>

#include <cyg/infra/testcase.h>

#include <cyg/kernel/sched.inl>

#include <cyg/kernel/timer.hxx>         // Cyg_Timer
#include <cyg/kernel/clock.inl>         // Cyg_Clock

#define NTHREADS 2
#include "testaux.hxx"

static Cyg_Mbox m0, m1, m2;

static volatile cyg_atomic q = 0;

#ifndef CYGMTH_MBOX_PUT_CAN_WAIT
#define PUT tryput
#endif

static void entry0( CYG_ADDRWORD data )
{
    cyg_count8 u,i;

    CYG_TEST_INFO("Testing put() and tryput() without wakeup");
    CYG_TEST_CHECK(!m0.waiting_to_get(), "mbox not initialized properly");
    CYG_TEST_CHECK(0==m0.peek(), "mbox not initialized properly");
    CYG_TEST_CHECK(NULL==m0.peek_item(), "mbox not initialized properly");
    m0.PUT((void *)55);
    CYG_TEST_CHECK(1==m0.peek(), "peek() wrong");
    CYG_TEST_CHECK(55==(cyg_count8)m0.peek_item(), "peek_item() wrong");
    for(u=1; m0.tryput((void*)u); u++) {
        CYG_TEST_CHECK(55==(cyg_count8)m0.peek_item(), "peek_item() wrong");
        CYG_TEST_CHECK(u+1==m0.peek(), "peek() wrong");
    }
    CYG_TEST_CHECK(u == CYGNUM_KERNEL_SYNCH_MBOX_QUEUE_SIZE, "mbox not configured size");

    // m0 now contains ( 55 1 2 .. u-1 )
    CYG_TEST_CHECK(u==m0.peek(), "peek() wrong");
    CYG_TEST_CHECK(55==(cyg_count8)m0.peek_item(), "peek_item() wrong");

    CYG_TEST_INFO("Testing get(), tryget()");
    
    i = (cyg_count8)m0.tryget();
    CYG_TEST_CHECK( 55 == i, "Got wrong message" );
    for(cyg_count8 j=1; j<u;j++) {
        CYG_TEST_CHECK( j == (cyg_count8)m0.peek_item(), "peek_item()" );
        CYG_TEST_CHECK( m0.peek() == u - j, "peek() wrong" );
        i = (cyg_count8)m0.get();
        CYG_TEST_CHECK( j == i, "Got wrong message" );
    }
    
    CYG_TEST_CHECK( NULL == m0.peek_item(), "peek_item()" );
    CYG_TEST_CHECK( 0 == m0.peek(), "peek()");
    
    // m0 now empty

    CYG_TEST_CHECK(!m0.waiting_to_put(), "waiting_to_put()");
    CYG_TEST_CHECK(!m0.waiting_to_get(), "waiting_to_get()");

    CYG_TEST_INFO("Testing get(), blocking");
    
    CYG_TEST_CHECK(0==q++, "bad synchronization");
    m1.PUT((void*)99);                  // wakes t1
    i = (cyg_count8)m0.get();          // sent by t1
    CYG_TEST_CHECK(3==i, "Recieved wrong message");
    CYG_TEST_CHECK(2==q++, "bad synchronization");

#ifdef CYGFUN_KERNEL_THREADS_TIMER
    CYG_TEST_CHECK(NULL==m0.get(
        Cyg_Clock::real_time_clock->current_value() + 10),
                   "unexpectedly found message");
    CYG_TEST_CHECK(3==q++, "bad synchronization");
    // Allow t1 to run as this get times out
    // t1 must not be waiting...
    CYG_TEST_CHECK(m0.waiting_to_get(), "waiting_to_get()");

    m0.PUT((void*)7);                   // wake t1 from timed get
#ifdef CYGMTH_MBOX_PUT_CAN_WAIT
    q=10;
    while(m0.tryput((void*)6))          // fill m0's queue
        ;
    // m0 now contains ( 6 ... 6 )
    CYG_TEST_CHECK(10==q++, "bad synchronization");
    m1.put((void*)4);                   // wake t1
    CYG_TEST_CHECK(!m0.put((void*)8, 2), "timed put() unexpectedly worked");
    CYG_TEST_CHECK(12==q++, "bad synchronization");
    // m0 still contains ( 6 ... 6 )
    m0.put((void*)9);
    CYG_TEST_CHECK(13==q++, "bad synchronization");
#endif
#endif
    i=(cyg_count8)m2.get();
    CYG_TEST_FAIL_FINISH("Not reached");
}

static void entry1( CYG_ADDRWORD data )
{
    cyg_count8 i;
    i = (cyg_count8)m1.get();
    CYG_TEST_CHECK(1==q++, "bad synchronization");
    m0.PUT((void *)3);                  // wake t0

#ifdef CYGFUN_KERNEL_THREADS_TIMER
    CYG_TEST_INFO("Testing timed functions");
    CYG_TEST_CHECK(7==(cyg_count8)m0.get(
        Cyg_Clock::real_time_clock->current_value() + 20), "timed get()");
    CYG_TEST_CHECK(4==q++, "bad synchronization");
#ifdef CYGMTH_MBOX_PUT_CAN_WAIT
    CYG_TEST_CHECK(4==(cyg_count8)m1.get());

    CYG_TEST_CHECK(11==q++, "bad synchronization");
    thread[0]->delay(20);    // allow t0 to reach put on m1
    CYG_TEST_CHECK(14==q++, "bad synchronization");
    CYG_TEST_CHECK(m0.waiting_to_put(), "waiting_to_put()");
    do {
        // after first get m0 contains ( 6 .. 6 9 )
        i=(cyg_count8)m0.tryget();
    } while(6==i);
    CYG_TEST_CHECK(9==i,"put gone awry");
#endif
#endif
    CYG_TEST_PASS_FINISH("Mbox 1 OK");
}

void mbox1_main( void )
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
    mbox1_main();
}

// EOF mbox1.cxx
