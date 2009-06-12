/*========================================================================
//
//      mqueue1.cxx
//
//      Message queues tests
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
// Date:          2000-05-12
// Purpose:       This file provides tests for eCos mqueues
// Description:   
// Usage:         
//
//####DESCRIPTIONEND####
//
//======================================================================
*/

/* CONFIGURATION */

#include <pkgconf/kernel.h>

/* INCLUDES */

#include <cyg/infra/cyg_type.h>      // common types and externC
#include <cyg/kernel/thread.hxx>     // Cyg_Thread
#include <cyg/kernel/thread.inl>
// Specially avoid inlining here due to the way we abuse the mqueue
// implementation by making lots and lots of calls.
#define CYGPRI_KERNEL_SYNCH_MQUEUE_INLINE
#include <cyg/kernel/mqueue.hxx>     // Mqueue Header
#include <cyg/kernel/sema.hxx>       // semaphores
#include <cyg/infra/testcase.h>      // test API

// use the common kernel test magic to define 2 threads
#define NTHREADS 2
#include "testaux.hxx"

/* GLOBALS */

static char mempool[500];
static size_t storedmempoollen;
Cyg_Mqueue *mq;
static Cyg_Binary_Semaphore t0sem, t1sem;
static int calledback;


/* FUNCTIONS */

static int
my_memcmp(const void *m1, const void *m2, size_t n)
{
    char *s1 = (char *)m1;
    char *s2 = (char *)m2;

    while (n--) {
        if (*s1 != *s2)
            return *s1 - *s2;
        s1++;
        s2++;
    }
    return 0;
} // my_memcmp()

static void *
my_alloc( size_t len )
{
    if ( len > sizeof(mempool) )
        return NULL;

    storedmempoollen = len;
    return &mempool[0];
}

static void
my_free( void *ptr, size_t len )
{
    CYG_TEST_PASS_FAIL( (ptr == &mempool[0]) && (len == storedmempoollen), 
                        "Freed pool correctly");
    mq = NULL; // invalidate
}

static void
callback(Cyg_Mqueue &mq, CYG_ADDRWORD data)
{
    calledback += (int)data;
}

//************************************************************************
//************************************************************************

static void
t0( CYG_ADDRWORD data )
{
    Cyg_Mqueue::qerr_t err;
    char buf[35];
    size_t len;
    unsigned int prio;
    bool b;

    Cyg_Mqueue the_mq(4, 32, &my_alloc, &my_free, &err );
    CYG_TEST_PASS_FAIL( Cyg_Mqueue::OK == err,
                        "Create queue" );
    mq = &the_mq;

//------------------------------------------------------------------------

    err = mq->put( "Peter piper picked", sizeof("Peter piper picked"),
                   5, true );
    CYG_TEST_PASS_FAIL( Cyg_Mqueue::OK == err, "Simple (put)");

    t1sem.post();

//------------------------------------------------------------------------

    t0sem.wait();

    err = mq->get( buf, &len, &prio, true );
    b = (err == Cyg_Mqueue::OK);
    if (b)
        b = (len == sizeof("a peck of"));
    if (b)
        b = (prio == 100);
    if (b)
        b = (0 == 
             my_memcmp(buf, "a peck of", sizeof("a peck of"))
            );
    CYG_TEST_PASS_FAIL( b, "Blocking get");

//------------------------------------------------------------------------

    t0sem.wait();
    
    CYG_TEST_PASS_FAIL( 4 == mq->count(), "mq count" );
    err = mq->get( buf, &len, &prio, false );
    b = (err == Cyg_Mqueue::OK);
    if (b)
        b = (len == sizeof("pickled peppers"));
    if (b)
        b = (prio == 300);
    if (b)
        b = (0 == 
             my_memcmp(buf, "pickled peppers", sizeof("pickled peppers"))
            );
    if (b)
        b = (3 == mq->count());
    CYG_TEST_PASS_FAIL( b, "Prioritized (get 1)");

    err = mq->get( buf, &len, &prio, false );
    b = (err == Cyg_Mqueue::OK);
    if (b)
        b = (len == sizeof("."));
    if (b)
        b = (prio == 250);
    if (b)
        b = (0 == 
             my_memcmp(buf, ".", sizeof("."))
            );
    if (b)
        b = (2 == mq->count());
    CYG_TEST_PASS_FAIL( b, "Prioritized (get 2)");

    err = mq->get( buf, &len, &prio, false );
    b = (err == Cyg_Mqueue::OK);
    if (b)
        b = (len == 1);
    if (b)
        b = (prio == 225);
    if (b)
        b = (0 == 
             my_memcmp(buf, "", 1)
            );
    if (b)
        b = (1 == mq->count());
    CYG_TEST_PASS_FAIL( b, "Prioritized (get 3)");

    err = mq->get( buf, &len, &prio, false );
    b = (err == Cyg_Mqueue::OK);
    if (b)
        b = (len == sizeof("If Peter"));
    if (b)
        b = (prio == 200);
    if (b)
        b = (0 == 
             my_memcmp(buf, "If Peter", sizeof("If Peter"))
            );
    if (b)
        b = (0 == mq->count());
    CYG_TEST_PASS_FAIL( b, "Prioritized (get 4)");

//------------------------------------------------------------------------

    err = mq->get( buf, &len, &prio, false );

    CYG_TEST_PASS_FAIL( Cyg_Mqueue::WOULDBLOCK == err,
                        "Non-blocking get of empty queue" );

//------------------------------------------------------------------------
    
    Cyg_Mqueue::callback_fn_t oldcallback;
    
    oldcallback = mq->setnotify( &callback, (CYG_ADDRWORD) 42 );
    
    err = mq->put( "If Peter", sizeof("If Peter"),
                   200, false );
    CYG_TEST_PASS_FAIL( Cyg_Mqueue::OK == err,
                        "Prioritized (put in empty queue)");
    CYG_TEST_PASS_FAIL( 42 == calledback, "callback" );
    CYG_TEST_PASS_FAIL( NULL == oldcallback, "oldcallback" );

    err = mq->get( buf, &len, &prio, false );
    b = (err == Cyg_Mqueue::OK);
    if (b)
        b = (len == sizeof("If Peter"));
    if (b)
        b = (prio == 200);
    if (b)
        b = (0 == 
             my_memcmp(buf, "If Peter", sizeof("If Peter"))
            );
    CYG_TEST_PASS_FAIL( b, "Prioritized (get 2)");

    t1sem.post();

    err = mq->get( buf, &len, &prio, true );
    b = (err == Cyg_Mqueue::OK);
    if (b)
        b = (len == 32);
    if (b)
        b = (42 == calledback);
    if (b)
        b = (prio == 250);
    if (b)
        b = (0 == 
             my_memcmp(buf, "12345678901234567890123456789012", 32)
            );
    CYG_TEST_PASS_FAIL( b, "callback (blocked wait)");

//------------------------------------------------------------------------
    
    t1sem.post();
    t0sem.wait();

} // t0()


//************************************************************************
//************************************************************************


static void
t1( CYG_ADDRWORD data )
{
    Cyg_Mqueue::qerr_t err;
    char buf[35];
    size_t len;
    unsigned int prio;
    bool b;

//------------------------------------------------------------------------

    // wait till t0 says we can go
    t1sem.wait();

    err = mq->get( buf, &len, &prio, true );
    b = (err == Cyg_Mqueue::OK);
    if (b)
        b = (len == sizeof("Peter piper picked"));
    if (b)
        b = (prio == 5);
    if (b)
        b = (0 == 
             my_memcmp(buf, "Peter piper picked", sizeof("Peter piper picked"))
            );

    CYG_TEST_PASS_FAIL( b, "Simple");

//------------------------------------------------------------------------
    
    t0sem.post();         // t0 should run straight away
    Cyg_Thread::yield();  // but just in case we have a funny sched

    // by now t0 is blocked in mq->get

    err = mq->put( "a peck of", sizeof("a peck of"),
                   100, false );
    CYG_TEST_PASS_FAIL( Cyg_Mqueue::OK == err, "Block (put in empty queue)");

//------------------------------------------------------------------------
    
    err = mq->put( "If Peter", sizeof("If Peter"),
                   200, false );
    CYG_TEST_PASS_FAIL( Cyg_Mqueue::OK == err,
                        "Prioritized (put in empty queue)");

    err = mq->put( "pickled peppers", sizeof("pickled peppers"),
                   300, false );
    CYG_TEST_PASS_FAIL( Cyg_Mqueue::OK == err,
                        "Prioritized (put in queue w/1)");

    err = mq->put( ".", sizeof("."), 250, false );
    CYG_TEST_PASS_FAIL( Cyg_Mqueue::OK == err,
                        "Prioritized (put in queue w/2)");

    err = mq->put( "", 1, 225, false );
    CYG_TEST_PASS_FAIL( Cyg_Mqueue::OK == err,
                        "Prioritized (put in queue w/3)");

    err = mq->put( "foobar", 6, 1, false );
    CYG_TEST_PASS_FAIL( Cyg_Mqueue::WOULDBLOCK == err,
                        "Prioritized (full queue)");

    t0sem.post();

//------------------------------------------------------------------------
    
    t1sem.wait();
    Cyg_Thread::yield();  // but just in case we have a funny sched
    
    err = mq->put( "12345678901234567890123456789012xxxx", 32,
                   250, false );
    CYG_TEST_PASS_FAIL( Cyg_Mqueue::OK == err,
                        "callback (put in queue)");

//------------------------------------------------------------------------
    
    t1sem.wait();

    // Create an oversized queue
    {
        Cyg_Mqueue huge_mq(99999, 99999, &my_alloc, &my_free, &err );
        CYG_TEST_PASS_FAIL( Cyg_Mqueue::NOMEM == err,
                            "Oversized queue rejected" );
        // and it now gets destructed - but that shouldn't call free
        // to be called
    }

//------------------------------------------------------------------------
    
    t0sem.post();         // t0 should run straight away
    Cyg_Thread::yield();  // but just in case we have a funny sched

    // check that mq was destroyed when t0 dropped off the end
    CYG_TEST_PASS_FAIL( NULL == mq, "queue destroyed correctly" );
    
    CYG_TEST_EXIT("kernel mqueue test 1");

} // t1()


//************************************************************************
//************************************************************************

externC void
cyg_user_start(void)
{
#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
    cyg_hal_invoke_constructors();
#endif
    CYG_TEST_INIT();

    CYG_TEST_INFO( "Starting kernel mqueue test 1" );
    new_thread( t0, 0);
    new_thread( t1, 1);

#ifdef CYGIMP_THREAD_PRIORITY
    thread[0]->set_priority( 4 );
    thread[1]->set_priority( 5 ); // make sure the threads execute as intended
#endif
} // cyg_user_start()

//------------------------------------------------------------------------


/* EOF mqueue1.cxx */
