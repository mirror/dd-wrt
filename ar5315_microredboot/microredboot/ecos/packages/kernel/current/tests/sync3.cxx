//==========================================================================
//
//        sync3.cxx
//
//        Sync test 3 -- tests priorities and priority inheritance
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
//     Creates mutexes and threads to set up starvation condition.
//     Checks simple priority inheritance cures this.
//     
//     The starvation condition is caused by the highest priority
//     thread, t0 waiting on a mutex which is never released because
//     it is held by t2.  t2 never releases it because t1 will be
//     running at a priority level higher than t2 (but lower than t0).
//     
//     With priority inheritance enabled, t2 will inherit its priority
//     from t0 when t0 tries to grab the mutex.
//     
// Options:
//     CYGIMP_THREAD_PRIORITY
//     CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_INHERIT
//     CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_SIMPLE
//####DESCRIPTIONEND####

#include <pkgconf/kernel.h>

#include <cyg/kernel/thread.hxx>
#include <cyg/kernel/thread.inl>
#include <cyg/kernel/sched.hxx>
#include <cyg/kernel/mutex.hxx>
#include <cyg/kernel/sema.hxx>

#include <cyg/infra/testcase.h>

#include <cyg/kernel/sched.inl>

#if defined(CYGIMP_THREAD_PRIORITY) && \
    !defined(CYGPKG_KERNEL_SMP_SUPPORT)

// ------------------------------------------------------------------------
// Manufacture a simpler feature test macro for priority inheritance than
// the configuration gives us. We have priority inheritance if it is configured
// as the only protocol, or if it is the default protocol for dynamic protocol
// choice.
// FIXME: If we have dynamic protocol choice, we can also set priority inheritance
// as the protocol to be used on the mutexes we are interested in. At present we
// do not do this.

#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_INHERIT
# ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_DYNAMIC
#  ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_DEFAULT_INHERIT
#   define PRIORITY_INHERITANCE
#  else
#   undef PRIORITY_INHERITANCE
#  endif
# else
#  define PRIORITY_INHERITANCE
# endif
#else
# undef PRIORITY_INHERITANCE
#endif

// ------------------------------------------------------------------------

#define NTHREADS 3

#include "testaux.hxx"

static Cyg_Mutex m0;
static Cyg_Binary_Semaphore s0, s1, s2;

static cyg_ucount8 m0d = 9;

static void check_priorities_normal()
{
    CHECK( 5 == thread[0]->get_priority());
    CHECK( 6 == thread[1]->get_priority());
    CHECK( 7 == thread[2]->get_priority());
}

static void check_priorities_inherited()
{
    CHECK( 5 == thread[0]->get_priority());
    CHECK( 6 == thread[1]->get_priority());
#ifdef PRIORITY_INHERITANCE
    CHECK( 5 == thread[2]->get_current_priority());
#endif
    CHECK( 7 == thread[2]->get_priority());

}

static void entry0( CYG_ADDRWORD data )
{
    s0.wait();                  // wait until t2 has gained m0.lock
    check_priorities_normal();
    m0.lock(); {
        check_priorities_normal();
        CHECK( 2 == m0d );
        m0d = 0;
    } m0.unlock();
    check_priorities_normal();
#ifdef PRIORITY_INHERITANCE
    CYG_TEST_PASS_FINISH("Sync 3 OK -- priority inheritance worked");
#else
    CYG_TEST_FAIL_FINISH("Sync 3: thread not starved");
#endif
    // NOT REACHED
}

static void entry1( CYG_ADDRWORD data )
{
    s1.wait();
    // The delay below will allow testing of the priority inheritance
    // mechanism when scheduler does not guarantee to schedule threads
    // in strict priority order.
    for ( volatile cyg_ucount32 i=0; i < 100000; i++ )
        ; // math is hard

#ifdef PRIORITY_INHERITANCE
    // thread0 should have stopped by this point
    CYG_TEST_FAIL_FINISH("Sync 3: priority inheritance mechanism failed");
#else
    // With strict priority scheduling and no priority inheritance
    // this is expected to happen.
    CYG_TEST_PASS_FINISH("Sync 3 OK");
#endif
    CYG_TEST_FAIL_FINISH("Not reached");
}

void entry2( CYG_ADDRWORD data )
{
    m0.lock(); {
        CHECK( 9 == m0d );
        check_priorities_normal();
        s0.post();              // Now I have lock on m0, wake t0 then t1 
        check_priorities_inherited();
        s1.post();
        check_priorities_inherited();
        m0d = 2;
    } m0.unlock();
    check_priorities_normal();
    m0.lock(); {
        check_priorities_normal();
        CHECK( 0 == m0d );
        m0d = 21;
        s2.wait();              // never posted
    } m0.unlock();
}



void sync3_main(void)
{
    CYG_TEST_INIT();

    new_thread( entry0, 0);
    new_thread( entry1, 1);
    new_thread( entry2, 2);

    thread[0]->set_priority(5);
    thread[1]->set_priority(6);
    thread[2]->set_priority(7);

    Cyg_Scheduler::start();

    CYG_TEST_FAIL_FINISH("Not reached");
}

externC void
cyg_start( void )
{
#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
    cyg_hal_invoke_constructors();
#endif
    sync3_main();
}

#else // defined(CYGIMP_THREAD_PRIORITY) etc

externC void
cyg_start( void )
{ 
    CYG_TEST_INIT();
    CYG_TEST_INFO("Sync3 test requires:\n"
                         "defined(CYGIMP_THREAD_PRIORITY) &&\n"
                         "!defined(CYGPKG_KERNEL_SMP_SUPPORT)\n");
    CYG_TEST_NA("Sync3 test requirements");

}

#endif // defined(CYGIMP_THREAD_PRIORITY) etc

// EOF sync3.cxx
