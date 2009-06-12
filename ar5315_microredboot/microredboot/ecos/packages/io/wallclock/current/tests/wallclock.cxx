//==========================================================================
//
//        wallclock.cxx
//
//        WallClock test
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Gary Thomas
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
// Contributors:    nickg
// Date:          1998-07-20
// Description:   Tests the Kernel Wall Clock
//                This test exercises the WallClock class and checks that it
//                keeps time correctly.
//####DESCRIPTIONEND####
// -------------------------------------------------------------------------

#include <pkgconf/hal.h>
#include <pkgconf/kernel.h>

#include <cyg/kernel/thread.hxx>
#include <cyg/kernel/thread.inl>
#include <cyg/kernel/sched.hxx>
#include <cyg/kernel/mutex.hxx>
#include <cyg/kernel/sema.hxx>
#include <cyg/kernel/clock.hxx>

#include <cyg/kernel/diag.h>

#include <cyg/io/wallclock.hxx>         // The WallClock API

#include <cyg/infra/testcase.h>

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

// MN10300 sim takes 1min15secs to do one loop on 300MHz PII.
#define LOOPS_SIM       2
#define LOOPS_HW        10

cyg_int32 loops = LOOPS_HW;
cyg_tick_count one_sec;

// -------------------------------------------------------------------------
// Thread body


void wallclock_thread( CYG_ADDRWORD id )
{
    cyg_uint32 base;
    cyg_uint32 wtime;
    cyg_tick_count ticks;
    cyg_bool ok = true;

    CYG_TEST_INFO("Testing accuracy of wallclock (10 seconds silence)");

    // Check clock only every other second since the call itself may
    // take about a second.

    // Start by synchronizing to next wallclock increment, then wait for
    // a bit to get away from the exact time of the increment (or minor
    // inaccuracies in the HW clock will cause failures).
    wtime = Cyg_WallClock::wallclock->get_current_time();
    do {
        base = Cyg_WallClock::wallclock->get_current_time();
    } while (base == wtime);
    th->delay( one_sec/10 );

    for( int i = 0; i < loops; i += 2 )
    {
        // Make a note of the time
        ticks = Cyg_Clock::real_time_clock->current_value();

        wtime = Cyg_WallClock::wallclock->get_current_time();
        if(wtime != base+i)
        {
            diag_printf("offset %d, read %d, expected %d\n", i, wtime, base+i);
            ok = false;
        }

        // then calculate how much the above took so the delay
        // below can be made accurate.
        ticks = Cyg_Clock::real_time_clock->current_value() - ticks;

        th->delay( 2*one_sec - ticks );
    }

    if ( ! cyg_test_is_simulator ) {
        CYG_TEST_INFO("Tick output. Two seconds between each output.");

        // Start by synchronizing to next wallclock increment, then
        // wait for a bit to get away from the exact time of the
        // increment (or minor inaccuracies in the HW clock will cause
        // failures).
        wtime = Cyg_WallClock::wallclock->get_current_time();
        do {
            base = Cyg_WallClock::wallclock->get_current_time();
        } while (base == wtime);
        th->delay( one_sec/10 );

        for( int i = 0; i < loops; i += 2 )
        {
            // Make a note of the time
            ticks = Cyg_Clock::real_time_clock->current_value();
            
            wtime = Cyg_WallClock::wallclock->get_current_time();
            if(wtime != base+i)
            {
                diag_printf("wallclock drift: saw %d expected %d\n", 
                            wtime, base+i);
            }
            CYG_TEST_STILL_ALIVE(i, "2xtick...");
            
            // then calculate how much the above took so the delay
            // below can be made accurate.
            ticks = Cyg_Clock::real_time_clock->current_value() - ticks;
            
            th->delay( 2*one_sec - ticks );
        }
    }

    if (ok)
        CYG_TEST_PASS_FINISH("Wallclock OK");
    else
        CYG_TEST_FAIL_FINISH( "Clock out of sync" );
    
}

// -------------------------------------------------------------------------


#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
externC void
cyg_hal_invoke_constructors();
#endif

externC void
cyg_start( void )
{
#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
    cyg_hal_invoke_constructors();
#endif
    CYG_TEST_INIT();

    // Dont run this test if we are in a simulator, it just takes far too long.
    if( cyg_test_is_simulator )
        CYG_TEST_NA("Wallclock test takes too long in simulators");
    
    CYG_TEST_INFO("Starting wallclock test");

    Cyg_Clock::cyg_resolution res = Cyg_Clock::real_time_clock->get_resolution();
    
    one_sec = ( res.divisor * 1000000000LL ) / res.dividend ;
    
    th = new((void *)&thread) Cyg_Thread(CYG_SCHED_DEFAULT_INFO,
                                         wallclock_thread,
                                         0,
                                         "wallclock_thread",
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
// EOF wallclock.cxx
