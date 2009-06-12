//==========================================================================
//
//        watchdog_reset.cxx
//
//        Watchdog reset test
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
// Author(s):     jskov (based on watchdog.cxx)
// Contributors:  jskov, nickg
// Date:          1999-08-27
// Description:   Tests that the watchdog timer resets the board.
//                This test needs to be run by an operator - automatic
//                testing not possible.
//####DESCRIPTIONEND####
// -------------------------------------------------------------------------

#include <pkgconf/system.h>

#include <cyg/infra/testcase.h>
#include <cyg/infra/diag.h>

// Package requirements
#if defined(CYGPKG_KERNEL)

#include <pkgconf/kernel.h>

// Package option requirements
#if defined(CYGFUN_KERNEL_THREADS_TIMER) && \
    defined(CYGVAR_KERNEL_COUNTERS_CLOCK)


#include <cyg/kernel/thread.inl>

#include <cyg/hal/hal_cache.h>

#include <cyg/io/watchdog.hxx>          // watchdog API


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

// -------------------------------------------------------------------------
// Thread body

volatile int watchdog_accuracy = 50;

void watchdog_thread( CYG_ADDRWORD id )
{
    diag_printf("Test of watchdog timer accuracy. Expect the test to run\n"
                "for at least 10 times the watchdog timeout time. After\n"
                "that time you may have to reset the board manually and/or\n"
                "restart GDB which tends to get a little confused.\n");
    diag_printf("When you get contact with the board again, read the value\n"
                "in watchdog_accuracy - it should be close to 100 if the\n"
                "watchdog timer is accurate.\n");
                
    // Disable data cache so the variable in memory gets updated.
    HAL_DCACHE_SYNC();
    HAL_DCACHE_DISABLE();

    Cyg_Watchdog::watchdog.start();
    Cyg_Watchdog::watchdog.reset();

    while (watchdog_accuracy < 400) {
        Cyg_Watchdog::watchdog.reset();
        th->delay( watchdog_delay*watchdog_accuracy/100 );
        watchdog_accuracy += 5;
    }

    CYG_TEST_FAIL_FINISH("Watchdog failed to reset board. "
                         "Timer value is off by at least a factor of 4!");    
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

#else // CYGFUN_KERNEL_THREADS_TIMER etc...
#define N_A_MSG "Needs kernel RTC/threads timer"
#endif

#else // CYGPKG_KERNEL
#define N_A_MSG "Needs Kernel"
#endif

#ifdef N_A_MSG
void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_NA( N_A_MSG);
}
#endif // N_A_MSG

// -------------------------------------------------------------------------
// EOF watchdog_reset.cxx
