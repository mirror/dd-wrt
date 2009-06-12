//==========================================================================
//
//        watchdog2.cxx
//
//        Watchdog test
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
// Author(s):     nickg
// Contributors:  nickg, jskov
// Date:          1998-07-20
// Description:   Tests the watchdog driver.
//                This test tries to make sure the watchdog does not trigger
//                early. On boards where the watchdog performs a reset on
//                timeout, there will be no FAIL message - but it should
//                PASS even on those boards when the driver implementation
//                is correct.
//
//####DESCRIPTIONEND####
// -------------------------------------------------------------------------

#include <pkgconf/kernel.h>

#include <cyg/kernel/thread.hxx>
#include <cyg/kernel/thread.inl>
#include <cyg/kernel/sched.hxx>
#include <cyg/kernel/mutex.hxx>
#include <cyg/kernel/sema.hxx>
#include <cyg/kernel/clock.hxx>

#include <cyg/kernel/diag.h>

#include <cyg/io/watchdog.hxx>

#include <cyg/infra/testcase.h>
#include <cyg/infra/diag.h>

#if defined(CYGFUN_KERNEL_THREADS_TIMER) && \
    defined(CYGVAR_KERNEL_COUNTERS_CLOCK)

#include <cyg/kernel/sched.inl>

// -------------------------------------------------------------------------
// Data for the test

#ifdef CYGNUM_HAL_STACK_SIZE_TYPICAL
#define STACKSIZE CYGNUM_HAL_STACK_SIZE_TYPICAL
#else
#define STACKSIZE       (2*1024)        // size of thread stack
#endif

char thread_stack[STACKSIZE];

inline void *operator new(size_t size, void *ptr) { return ptr; };

// array of threads.
char thread[sizeof(Cyg_Thread)];

Cyg_Thread *th;

//cyg_tick_count one_sec;
cyg_tick_count watchdog_delay;
volatile bool watchdog_fired;
volatile cyg_tick_count watchdog_time;

#define WATCHDOG_CYCLES_PER_LOOP_HW     7
#define WATCHDOG_CYCLES_PER_LOOP_SIM    2
#define WATCHDOG_TICKS_TILL_TIMEOUT     10

// -------------------------------------------------------------------------
// Action functions:

#ifdef CYGINT_WATCHDOG_RESETS_ON_TIMEOUT
void watchdog_action1( CYG_ADDRWORD data )
{
    watchdog_fired = true;
    watchdog_time = Cyg_Clock::real_time_clock->current_value();
}
#endif

// -------------------------------------------------------------------------
// Thread body

void watchdog_thread( CYG_ADDRWORD id )
{
    cyg_tick_count watchdog_start_time;
    cyg_tick_count thread_start_time, thread_end_time;
    cyg_ucount8 watchdog_cycles_per_loop;

    if (cyg_test_is_simulator) {
        watchdog_cycles_per_loop = WATCHDOG_CYCLES_PER_LOOP_SIM;
    } else {
        watchdog_cycles_per_loop = WATCHDOG_CYCLES_PER_LOOP_HW;
    }

    CYG_TEST_INFO("Testing that watchdog does not fire early");

    watchdog_fired = false;
    Cyg_Watchdog::watchdog.start();

#ifdef CYGINT_WATCHDOG_RESETS_ON_TIMEOUT
    // First test that the watchdog does not trigger when being reset.
    Cyg_Watchdog_Action wdaction( watchdog_action1, 0 );
    wdaction.install();
#endif
    Cyg_Watchdog::watchdog.reset();

    watchdog_start_time = Cyg_Clock::real_time_clock->current_value();
    watchdog_fired = false;
        
    for( cyg_ucount8 i = 0; i < watchdog_cycles_per_loop; i++ ) {
        thread_start_time = Cyg_Clock::real_time_clock->current_value();
        th->delay( watchdog_delay-2 );
        Cyg_Watchdog::watchdog.reset();
        if (watchdog_fired) {
            thread_end_time = Cyg_Clock::real_time_clock->current_value();
            diag_printf("Watchdog fired prematurely after %d ticks\n", 
                        (int)(watchdog_time - watchdog_start_time));
            diag_printf("  Thread slept for %d ticks, loop #%d\n", 
                        (int)(thread_end_time - thread_start_time), i);
            CYG_TEST_FAIL_FINISH("Watchdog triggered unexpectedly");
            break;
        } else {
            // No printing - delays are fatal!
            // CYG_TEST_STILL_ALIVE(i, "Resetting watchdog...");
            Cyg_Watchdog::watchdog.reset();
            watchdog_fired = false;
            watchdog_start_time = Cyg_Clock::real_time_clock->current_value();
        }
    }

    CYG_TEST_PASS_FINISH("Watchdog OK");    
}

// -------------------------------------------------------------------------

    
externC void
cyg_start( void )
{
    CYG_TEST_INIT();

#if !defined(CYGIMP_WATCHDOG_EMULATE) && defined(CYGPKG_HAL_MN10300_STDEVAL1)
    // Workaround for PR 17974
    if( cyg_test_is_simulator )
        CYG_TEST_NA("Watchdog device not implemented in MN10300 simulator.");
#endif


    Cyg_Clock::cyg_resolution res = Cyg_Clock::real_time_clock->get_resolution();
    
    cyg_uint64 wres = Cyg_Watchdog::watchdog.get_resolution();

    // Calculate how many clock ticks there are in a watchdog cycle.
    
    watchdog_delay = ((cyg_tick_count)wres * (cyg_tick_count)res.divisor );
    watchdog_delay /= res.dividend;
    
    th = new((void *)&thread) Cyg_Thread(CYG_SCHED_DEFAULT_INFO,
                                         watchdog_thread,
                                         0,
                                         "watchdog_thread",
                                         (CYG_ADDRESS)thread_stack,
                                         STACKSIZE
        );

    th->resume();

    // Get the world going
    Cyg_Scheduler::scheduler.start();

}

#else // if defined(CYGFUN_KERNEL_THREADS_TIMER) etc...

externC void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_NA("Kernel real-time clock/threads timer disabled");
}

#endif // if defined(CYGFUN_KERNEL_THREADS_TIMER) etc...

// -------------------------------------------------------------------------
// EOF watchdog.cxx
