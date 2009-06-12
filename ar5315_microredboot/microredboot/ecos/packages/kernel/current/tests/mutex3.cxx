//==========================================================================
//
//        mutex3.cxx
//
//        Mutex test 3 - priority inheritance
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
// Author(s):     hmt
// Contributors:  hmt
// Date:          2000-01-06
// Description:   Tests mutex priority inheritance
//####DESCRIPTIONEND####

#include <pkgconf/hal.h>
#include <pkgconf/kernel.h>

#include <cyg/kernel/sched.hxx>        // Cyg_Scheduler::start()
#include <cyg/kernel/thread.hxx>       // Cyg_Thread

#include <cyg/kernel/mutex.hxx>

#include <cyg/infra/testcase.h>

#include <cyg/kernel/sched.inl>
#include <cyg/kernel/thread.inl>

#include <cyg/infra/diag.h>             // diag_printf

#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
externC void
cyg_hal_invoke_constructors();
#endif

// ------------------------------------------------------------------------
//
// These checks should be enough; any other scheduler which has priorities
// should manifest as having no priority inheritance, but otherwise fine,
// so the test should work correctly.

#if defined(CYGVAR_KERNEL_COUNTERS_CLOCK) &&    \
    (CYGNUM_KERNEL_SCHED_PRIORITIES > 20) &&    \
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
#   define PRIORITY_INHERITANCE "dynamic-default-inherit"
#  endif
# else
#  define PRIORITY_INHERITANCE "static-inherit"
# endif
#endif

#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_CEILING
# ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_DYNAMIC
#  ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_DEFAULT_CEILING
#   if CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_DEFAULT_PRIORITY <= 5
#    define PRIORITY_INHERITANCE "dynamic-default-ceiling-high"
#   elif CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_DEFAULT_PRIORITY >= 15
#    define NO_PRIORITY_INHERITANCE "dynamic-default-ceiling-low"
#   else
#    define PRIORITY_UNKNOWN "dynamic-default-ceiling-mid"
#   endif
#  endif
# else
#  if CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_DEFAULT_PRIORITY <= 5
#   define PRIORITY_INHERITANCE "static-ceiling-high"
#  elif CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_DEFAULT_PRIORITY >= 15
#   define NO_PRIORITY_INHERITANCE "static-ceiling-low"
#  else
#   define PRIORITY_UNKNOWN "static-ceiling-mid"
#  endif
# endif
#endif

#ifndef PRIORITY_INHERITANCE
# ifndef NO_PRIORITY_INHERITANCE
#  define NO_PRIORITY_INHERITANCE "no scheme selected"
# endif
#endif

// ------------------------------------------------------------------------
// Management functions
//
// Stolen from testaux.hxx and copied in here because I want to be able to
// reset the world also.

#define NTHREADS 7

static inline void *operator new(size_t size, void *ptr) { return ptr; };

#define STACKSIZE CYGNUM_HAL_STACK_SIZE_TYPICAL

static Cyg_Thread *thread[NTHREADS] = { 0 };

typedef CYG_WORD64 CYG_ALIGNMENT_TYPE;

static CYG_ALIGNMENT_TYPE thread_obj[NTHREADS] [
   (sizeof(Cyg_Thread)+sizeof(CYG_ALIGNMENT_TYPE)-1)
     / sizeof(CYG_ALIGNMENT_TYPE)                     ];

static CYG_ALIGNMENT_TYPE stack[NTHREADS] [
   (STACKSIZE+sizeof(CYG_ALIGNMENT_TYPE)-1)
     / sizeof(CYG_ALIGNMENT_TYPE)                     ];

static volatile int nthreads = 0;

static Cyg_Thread *new_thread( cyg_thread_entry *entry,
                               CYG_ADDRWORD data,
                               CYG_ADDRWORD priority,
                               int do_resume )
{
    int _nthreads = nthreads++;

    CYG_ASSERT(_nthreads < NTHREADS, 
               "Attempt to create more than NTHREADS threads");

    thread[_nthreads] = new( (void *)&thread_obj[_nthreads] )
        Cyg_Thread(priority,
                   entry, data, 
                   NULL,                // no name
                   (CYG_ADDRESS)stack[_nthreads], STACKSIZE );

    if ( do_resume )
        thread[_nthreads]->resume();

    return thread[_nthreads];
}


static void kill_threads( void )
{
    CYG_ASSERT(nthreads <= NTHREADS, 
               "More than NTHREADS threads");
    CYG_ASSERT( Cyg_Thread::self() == thread[0],
                "kill_threads() not called from thread 0");
    while ( nthreads > 1 ) {
        nthreads--;
        if ( NULL != thread[nthreads] ) {
            thread[nthreads]->kill();
            thread[nthreads]->~Cyg_Thread();
            thread[nthreads] = NULL;
        }
    }
    CYG_ASSERT(nthreads == 1,
               "No threads left");
}

// ------------------------------------------------------------------------

#define DELAYFACTOR 1 // for debugging

// ------------------------------------------------------------------------

static Cyg_Mutex mutex;

// These are for reporting back to the master thread
volatile int got_it  = 0;
volatile int t3ran   = 0;
volatile int t3ended = 0;
volatile int extras[4] = {0,0,0,0};
    
volatile int go_flag = 0; // but this one controls thread 3 from thread 2

// ------------------------------------------------------------------------
// 0 to 3 of these run generally to interfere with the other processing,
// to cause multiple prio inheritances, and clashes in any orders.

static void extra_thread( CYG_ADDRWORD data )
{
#define XINFO( z ) \
    do { z[13] = '0' + data; CYG_TEST_INFO( z ); } while ( 0 )

    static char running[]  = "Extra thread Xa running";
    static char exiting[]  = "Extra thread Xa exiting";
    static char resumed[]  = "Extra thread Xa resumed";
    static char locked[]   = "Extra thread Xa locked";
    static char unlocked[] = "Extra thread Xa unlocked";

    XINFO( running );

    Cyg_Thread *self = Cyg_Thread::self();

    self->suspend();

    XINFO( resumed );

    mutex.lock();

    XINFO( locked );

    mutex.unlock();

    XINFO( unlocked );

    extras[ data ] ++;

    XINFO( exiting );

}

// ------------------------------------------------------------------------

static void t1( CYG_ADDRWORD data )
{
    Cyg_Thread *self = Cyg_Thread::self();

    CYG_TEST_INFO( "Thread 1 running" );

    self->suspend();

    mutex.lock();

    got_it++;

    CYG_TEST_CHECK( 0 == t3ended, "T3 ended prematurely [T1,1]" );

    mutex.unlock();

    CYG_TEST_CHECK( 0 == t3ended, "T3 ended prematurely [T1,2]" );

    // That's all.

    CYG_TEST_INFO( "Thread 1 exit" );
}

// ------------------------------------------------------------------------

static void t2( CYG_ADDRWORD data )
{
    Cyg_Thread *self = Cyg_Thread::self();
    int i;
    cyg_tick_count then, now;


    CYG_TEST_INFO( "Thread 2 running" );

    CYG_TEST_CHECK( 0 == (data & ~0x77), "Bad T2 arg: extra bits" );
    CYG_TEST_CHECK( 0 == (data & (data >> 4)), "Bad T2 arg: overlap" );

    self->suspend();

    // depending on our config argument, optionally restart some of the
    // extra threads to throw noise into the scheduler:
    for ( i = 0; i < 3; i++ )
        if ( (1 << i) & data )          // bits 0-2 control
            thread[i+4]->resume();      // made sure extras are thread[4-6]

    self->delay( DELAYFACTOR * 10 );    // let those threads run

    Cyg_Scheduler::lock();              // do this next lot atomically

    go_flag = 1;                        // unleash thread 3
    thread[1]->resume();                // resume thread 1

    // depending on our config argument, optionally restart some of the
    // extra threads to throw noise into the scheduler at this later point:
    for ( i = 4; i < 7; i++ )
        if ( (1 << i) & data )          // bits 4-6 control
            thread[i]->resume();        // made sure extras are thread[4-6]

    Cyg_Scheduler::unlock();           // let scheduling proceed

    // Need a delay (but not a CPU yield) to allow t3 to awaken and act on
    // the go_flag, otherwise we check these details below too soon.
    // Actually, waiting for the clock to tick a couple of times would be
    // better, so that is what we will do.  Must be a busy-wait.
    then = Cyg_Clock::real_time_clock->current_value();
    do {
        now = Cyg_Clock::real_time_clock->current_value();
        // Wait longer than the delay in t3 waiting on go_flag
    } while ( now < (then + 3) );

#ifdef PRIORITY_UNKNOWN
    CYG_TEST_INFO( "Not checking: " PRIORITY_UNKNOWN );
#else
#ifdef PRIORITY_INHERITANCE
    CYG_TEST_INFO( "Checking priority scheme: " PRIORITY_INHERITANCE );
    CYG_TEST_CHECK( 1 == t3ran, "Thread 3 did not run" );
    CYG_TEST_CHECK( 1 == got_it, "Thread 1 did not get the mutex" );
#else
    CYG_TEST_INFO( "Checking NO priority scheme: " NO_PRIORITY_INHERITANCE );
    CYG_TEST_CHECK( 0 == t3ran, "Thread 3 DID run" );
    CYG_TEST_CHECK( 0 == got_it, "Thread 1 DID get the mutex" );
#endif
#endif

    CYG_TEST_CHECK( 0 == t3ended, "Thread 3 ended prematurely [T2,1]" );

    self->delay( DELAYFACTOR * 20 );    // let those threads run

    CYG_TEST_CHECK( 1 == t3ran, "Thread 3 did not run" );
    CYG_TEST_CHECK( 1 == got_it, "Thread 1 did not get the mutex" );
    CYG_TEST_CHECK( 1 == t3ended, "Thread 3 has not ended" );

    for ( i = 0; i < 3; i++ )
        if ( (1 << i) & (data | data >> 4) ) // bits 0-2 and 4-6 control
            CYG_TEST_CHECK( 1 == extras[i+1], "Extra thread did not run" );
        else
            CYG_TEST_CHECK( 0 == extras[i+1], "Extra thread ran" );

    CYG_TEST_PASS( "Thread 2 exiting, AOK" );
    // That's all: restart the control thread.
    thread[0]->resume();
}

// ------------------------------------------------------------------------

static void t3( CYG_ADDRWORD data )
{
    Cyg_Thread *self = Cyg_Thread::self();

    CYG_TEST_INFO( "Thread 3 running" );

    mutex.lock();

    self->delay( DELAYFACTOR * 5 );    // let thread 3a run

    thread[2]->resume();                // resume thread 2

    while ( 0 == go_flag )
        self->delay(1);                 // wait until we are told to go

    t3ran ++;                           // record the fact

    CYG_TEST_CHECK( 0 == got_it, "Thread 1 claims to have got my mutex" );
    
    mutex.unlock();
    
    t3ended ++;                         // record that we came back

    CYG_TEST_CHECK( 1 == got_it, "Thread 1 did not get the mutex" );

    CYG_TEST_INFO( "Thread 3 exit" );
}

// ------------------------------------------------------------------------

static void control_thread( CYG_ADDRWORD data )
{
    Cyg_Thread *self = Cyg_Thread::self();
    int i;

    CYG_TEST_INIT();
    CYG_TEST_INFO( "Control Thread running" );

    // Go through the 27 possibilitied of resuming the extra threads
    //     0: not at all
    //     1: early in the process
    //     2: later on
    // which are represented by bits 0-3 and 4-6 resp in the argument to
    // thread 2 (none set means no resume at all).
    for ( i = 0; i < 27; i++ ) {
        static int xx[] = { 0, 1, 16 };
        int j = i % 3;
        int k = (i / 3) % 3;
        int l = (i / 9) % 3;

        int d = xx[j] | (xx[k]<<1) | (xx[l]<<2) ;

        if ( cyg_test_is_simulator && (0 != i && 13 != i && 26 != i) )
            continue;    // 13 is 111 base 3, 26 is 222 base 3

#ifdef PRIORITY_INHERITANCE
        // If the simple scheme plus relay enhancement, or any other
        // *complete* scheme, we can run all three ancillary threads no
        // problem, so no special action here.

#else
        // If no priority inheritance at all, running threads 1a and 2a is
        // OK, but not thread 3a; it blocks the world.
        if ( l )                        // Cannot run thread 3a if no
            break;                      //     priority inheritance at all.
#endif

        mutex = Cyg_Mutex();            // Reinitialize this

        got_it  = 0;
        t3ran   = 0;
        t3ended = 0;
        for ( int z = 0; z < 4; z++ ) extras[z] = 0;
        go_flag = 0;
        
        new_thread( t1, 0,  5, 1 );            // Slot 1
        new_thread( t2, d, 10, 1 );            // Slot 2
        new_thread( t3, 0, 15, 1 );            // Slot 3
        
        new_thread( extra_thread, 1,  8, j );  // Slot 4
        new_thread( extra_thread, 2, 12, k );  // Slot 5
        new_thread( extra_thread, 3, 17, l );  // Slot 6
        
        {
            static char *a[] = { "inactive", "run early", "run late" };
            diag_printf( "\n----- [%2d] New Cycle: 0x%02x, Threads 1a %s, 2a %s, 3a %s -----\n",
                         i, d,  a[j], a[k], a[l] );
        }

        self->suspend();
        
        kill_threads();
        mutex.~Cyg_Mutex();
    }
    CYG_TEST_EXIT( "Control Thread exit" );
}

// ------------------------------------------------------------------------

externC void
cyg_user_start( void )
{ 
#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
    cyg_hal_invoke_constructors();
#endif
    new_thread( control_thread, 0, 2, 1 );
}

#else // CYGVAR_KERNEL_COUNTERS_CLOCK &c

externC void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_INFO("Mutex3 test requires:\n"
                         "CYGVAR_KERNEL_COUNTERS_CLOCK &&\n"
                         "(CYGNUM_KERNEL_SCHED_PRIORITIES > 20) &&\n"
                         "!defined(CYGPKG_KERNEL_SMP_SUPPORT)\n");
    CYG_TEST_NA("Mutex3 test requirements");
}
#endif // CYGVAR_KERNEL_COUNTERS_CLOCK &c


// ------------------------------------------------------------------------
// Documentation: enclosed is the design of this test.
//
// It has been carefully constructed so that it does NOT use other kernel
// facilities (aside from delay-task) to test that priority inheritance is
// working, or not, as intended by the configuration.
//
// These notes describe the flow of control in one run of the test with the
// ancillary tasks optionally interspersed.  The details of how those extra
// tasks are or are not allowed to run are not described.
// 
// 
// 
// The only change in the test that depends on whether there is inheritance or
// not is the check in thread 2 on "3-ran" and "got it" flags marked ****
// 
// 
// volatile &c booleans:
//         "got it"     = FALSE
//         "3-ran"      = FALSE
//         "3-ended"    = FALSE
//         "extras"[3]  = FALSE
// 
// thread 1.  prio 5, self-suspend.
// 
// thread 1a, prio 8, self-suspend.
// 
// thread 2.  prio 10, self-suspend.
// 
// thread 2a, prio 12, self-suspend.
// 
// thread 3.  prio 15, runs, lock mutex, resume(2)
// 
// thread 3a, prio 17, self-suspend.
// 
//        2.  runs,
//        2.  resume(3a) +++OPTIONAL
//        2.  resume(2a) +++OPTIONAL
//        2.  resume(1a) +++OPTIONAL
//        [1a lock-fail]	thread 3->prio := 8
// 
//        [3. runs maybe, does the looping thing]
// 
//        2.  sleep a while...
// 
//        [2a lock-fail]	thread 3->prio := 12
// 
//        [3. runs maybe, does the looping thing]
// 
//        [3a lock-fail]   thread 3->prio unchanged
// 
//        [3. runs maybe, does the looping thing]
// 
//        2.  lock scheduler
//        2.  set "go-flag"
//        2.  resume(1)
//        2.  resume(1a) +++OPTIONAL
//        2.  resume(2a) +++OPTIONAL
//        2.  resume(3a) +++OPTIONAL
//        2.  unlock scheduler
// 
//        1.  runs, lock mutex - thread 3 has it locked
//
//        2.  busy-waits a bit for thread 3 to come out of its delay() loop.
//            This must be a *busy*wait so that 3 can only run via the
//            inherited raised priority.
// 
//        [xa. all do the same: lock mutex,                ]
//        [xa. unlock mutex                                ]
//        [xa. set a flag "extras"[x] to say we are done.  ]
//        [xa. exit                                        ]
// 
// 
// 
// INHERIT
// -------
// 
//                 thread 3->prio := 5
// 
//        3.  runs,
//        3.  set a flag to say "3-ran",
//        3.  loop with a sleep(1) until "go-flag" is set.
//        3.  check "got it" is false,
//        3.  then unlock mutex,
// 
//                 thread 3->prio := 15
// 
//        1.  runs, set a flag to say "got it",
//        1.  check "3-ended" flag is false
//        1.  unlock mutex,
//        1.  check "3-ended" flag is still false
//        1.  exit.
// 
//        [1a locks, unlocks, exits]
// 
//        2.  runs, check "3-ran" and "got it" flags are TRUE ****
//        2.  check "3-ended" flag is false
//        2.  sleeps for a while so that...
// 
//        [2a locks, unlocks, exits]
//            
//        3.  runs, set "3-ended" flag,
//        3.  check "3-ran" and "got it" flags
//        3.  exit
// 
//        [3a locks, unlocks, exits]
// 
//        2.  awakens, checks all flags true,
//        2.  check that all "extra" threads that we started have indeed run
//        2.  end of test.
// 
// 
// 
// 
// NO-INHERIT
// ----------
//                 thread 1 is waiting on the mutex
// 
//        [1a lock-fail]
// 
//        2.  runs, checks that "3-ran" and "got it" flags are FALSE ****
//        2.  check "3-ended" flag is false
//        2.  sleeps for a while so that...
// 
//        [2a. lock-fail]
//            
//        3.  runs, set a flag to say "3-ran",
//        3.  check "got it" is false,
//        3.  then unlock mutex,
// 
//        1.  runs, set a flag to say "got it",
//        1.  check "3-ended" flag is false
//        1.  unlock mutex,
//        1.  check "3-ended" flag is still false
//        1.  exit.
// 
//        [1a locks, unlocks, exits]
//        [2a locks, unlocks, exits]
// 
//        3.  runs, set "3-ended" flag,
//        3.  check "3-ran" and "got it" flags
//        3.  exit
// 
//        [3a locks, unlocks, exits]
//                
//        2.  awakens, checks all flags true, 
//        2.  check that all "extra" threads that we started have indeed run
//        2.  end of test.
// 
// 
// (the end)
// 
// 
// ------------------------------------------------------------------------

// EOF mutex3.cxx
