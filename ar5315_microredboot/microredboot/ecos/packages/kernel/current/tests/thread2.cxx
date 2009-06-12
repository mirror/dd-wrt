//==========================================================================
//
//        thread2.cxx
//
//        Thread test 2
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
// Date:          1998-02-19
// Description:
//    tests scheduler & threads & priorities
//    + create multiple threads with various priorities
//    + check highest priority running thread is always running
//    + check next highest runs when highest suspended
//    + check several threads of equal priority share time
//      (only !CYGINT_KERNEL_SCHEDULER_UNIQUE_PRIORITIES)
//    + check other threads are starved
//    + check setting priority dynamically causes a thread to
//      become/stay current/non-current
// Omissions:
//     check yield
//     check can set threads with min and max priority
// Options:
//    CYGINT_KERNEL_SCHEDULER_UNIQUE_PRIORITIES
//    CYGIMP_THREAD_PRIORITY
//    CYGNUM_KERNEL_SCHED_PRIORITIES
//    CYGSEM_KERNEL_SCHED_BITMAP
//    CYGSEM_KERNEL_SCHED_MLQUEUE
//####DESCRIPTIONEND####
//==========================================================================

#include <pkgconf/kernel.h>

#include <cyg/kernel/thread.hxx>
#include <cyg/kernel/thread.inl>
#include <cyg/kernel/sched.hxx>
#include <cyg/kernel/mutex.hxx>
#include <cyg/kernel/sema.hxx>

#include <cyg/infra/testcase.h>

#include <cyg/kernel/sched.inl>

// ------------------------------------------------------------------------

#if defined(CYGIMP_THREAD_PRIORITY) && \
    !defined(CYGPKG_KERNEL_SMP_SUPPORT)

// ------------------------------------------------------------------------

static Cyg_Counting_Semaphore s0, s1, s2;

static volatile cyg_ucount8 q = 0;

static Cyg_Thread *thread0, *thread1, *thread2;

#define NTHREADS 3
#include "testaux.hxx"

// ------------------------------------------------------------------------

static void entry0( CYG_ADDRWORD data )
{
    CHECK(  0 == q++ );
    s0.wait();
    CHECK(  3 == q++ );
    s1.post();
    CHECK(  4 == q++ );
    s0.wait();
    s0.wait();
    CYG_TEST_PASS_FINISH("Thread 2 OK");
}

// ------------------------------------------------------------------------

static void entry1( CYG_ADDRWORD data )
{
    CHECK(  1 == q++ );
    s1.wait();
    CHECK(  5 == q++ );
    thread0->set_priority(9);
    s0.post();
    CHECK(  6 == q++ );
    thread2->set_priority(3);
    CHECK(  8 == q++ );
    s2.post();
    CHECK( 12 == q++ );
    CHECK( 9 == thread0->get_priority() );
    CHECK( 6 == thread1->get_priority() );
    CHECK( 7 == thread2->get_priority() );
    q = 100;
#if !(CYGINT_KERNEL_SCHEDULER_UNIQUE_PRIORITIES) \
    && defined(CYGSEM_KERNEL_SCHED_TIMESLICE)
    thread2->set_priority(6);
    CHECK( 6 == thread1->get_priority() );
    CHECK( 6 == thread2->get_priority() );

    while ( 100 == q )
        ;
    CHECK( 101 == q++ );
    s1.wait();
    CHECK( 103 == q++ );
#endif
    s0.post();
    s1.wait();
}

// ------------------------------------------------------------------------

static void entry2( CYG_ADDRWORD data )
{
    CHECK(  2 == q++ );
    s0.post();
    CHECK(  7 == q++ );
    s2.wait();
    CHECK(  9 == q++ );
    thread1->set_priority(6);
    CHECK( 10 == q++ );
    thread2->set_priority(2);
    CHECK( 11 == q++ );
    thread2->set_priority(7);

#if !(CYGINT_KERNEL_SCHEDULER_UNIQUE_PRIORITIES) \
    && defined(CYGSEM_KERNEL_SCHED_TIMESLICE)
    CHECK( 6 == thread1->get_priority() );
    CHECK( 6 == thread2->get_priority() );

    CHECK( 100 == q++ );
    while ( 101 == q )
        ;
    CHECK( 102 == q++ );
    s1.post();
#endif
    s0.post();
    s2.wait();
}


// ------------------------------------------------------------------------

void thread2_main( void )
{
    CYG_TEST_INIT();
    
    thread0 = new_thread( entry0, 0 );
    thread1 = new_thread( entry1, 1 );
    thread2 = new_thread( entry2, 2 );

    thread0->resume();
    thread1->resume();
    thread2->resume();

    thread0->set_priority(5);
    thread1->set_priority(6);
    thread2->set_priority(7);

    if( 9 >= CYG_THREAD_MIN_PRIORITY )
        CYG_TEST_FAIL_FINISH("Test requires priorities up to 9");

    if( 2 <= CYG_THREAD_MAX_PRIORITY )
        CYG_TEST_FAIL_FINISH("Test requires priorities as low as 2");

    Cyg_Scheduler::start();

    CYG_TEST_FAIL_FINISH("Unresolved");
}

// ------------------------------------------------------------------------

externC void
cyg_start( void )
{
#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
    cyg_hal_invoke_constructors();
#endif
    thread2_main();
}
// ------------------------------------------------------------------------


#else // CYGPKG_KERNEL_SMP_SUPPORT etc

// ------------------------------------------------------------------------

externC void
cyg_start( void )
{ 
    CYG_TEST_INIT();
    CYG_TEST_INFO("Thread2 test requires:\n"
                         "defined(CYGIMP_THREAD_PRIORITY) &&\n"
                         "!defined(CYGPKG_KERNEL_SMP_SUPPORT)\n");
    CYG_TEST_NA("Thread2 test requirements");
}

// ------------------------------------------------------------------------

#endif // CYGPKG_KERNEL_SMP_SUPPORT etc

// ------------------------------------------------------------------------
// EOF thread2.cxx
