/*=================================================================
//
//        kclock1.c
//
//        Kernel C API Clock test 1 - Real Time Clock
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
// Date:          1998-03-20
// Description:   Tests the Kernel Real Time Clock
//                This test creates a thread, starts the scheduler and
//                delays for a time of about 5 seconds.  This test should
//                be expected to run for about this length of time.
// Omissions:
//     Doesn't test alarms attached to RTC.
// Assumptions:
//     CYGVAR_KERNEL_COUNTERS_CLOCK must be set.
//     Resolution of clock small compared with 5s.
//     Overhead small compared with 5s.
// Options:
//     CYGIMP_KERNEL_COUNTERS_SINGLE_LIST
//     CYGIMP_KERNEL_COUNTERS_MULTI_LIST
//     CYGVAR_KERNEL_COUNTERS_CLOCK
//     CYGNUM_KERNEL_COUNTERS_MULTI_LIST_SIZE
//####DESCRIPTIONEND####
*/

#include <cyg/hal/hal_arch.h>           // CYGNUM_HAL_STACK_SIZE_TYPICAL

#include <cyg/kernel/kapi.h>

#include <cyg/infra/testcase.h>


static cyg_uint64 TEST_DELAY;

#ifdef CYGFUN_KERNEL_THREADS_TIMER

#ifdef CYGFUN_KERNEL_API_C

#include "testaux.h"

#define NTHREADS 1
#define STACKSIZE CYGNUM_HAL_STACK_SIZE_TYPICAL

static cyg_handle_t thread[NTHREADS];

static cyg_thread thread_obj[NTHREADS];
static char stack[NTHREADS][STACKSIZE];

static void entry0( cyg_addrword_t data )
{
    cyg_resolution_t res;
    cyg_uint32 ticks;
    cyg_tick_count_t count0, count1;
    cyg_handle_t rtclock, rtcounter;

    rtclock = cyg_real_time_clock();
    cyg_clock_to_counter(rtclock, &rtcounter);

    res = cyg_clock_get_resolution (rtclock);

    /* RTC takes res.dividend/res.divisor ns/tick */
    ticks = ((cyg_uint64)TEST_DELAY * res.divisor) / res.dividend;

    count0 = cyg_counter_current_value(rtcounter);
    cyg_thread_delay(ticks);
    count1 = cyg_counter_current_value(rtcounter);

    CYG_TEST_CHECK(count0+ticks <= count1,
                   "real time clock's counter not counting");

    CYG_TEST_CHECK(count1 <= cyg_current_time(),"cyg_current_time()");

    CYG_TEST_PASS_FINISH("Kernel C API Clock 1 OK");
}

void kclock1_main( void )
{
    CYG_TEST_INIT();

    if (cyg_test_is_simulator) {
        TEST_DELAY = 100000000ll;
    } else {
        TEST_DELAY = 3000000000ll;
    }

    cyg_thread_create(4, entry0 , (cyg_addrword_t)0, "kclock1",
        (void *)stack[0], STACKSIZE, &thread[0], &thread_obj[0]);
    cyg_thread_resume(thread[0]);

    cyg_scheduler_start();
}

externC void
cyg_start( void )
{
    kclock1_main();
}

#else  // def CYGFUN_KERNEL_API_C
#define N_A_MSG "Kernel C API layer disabled"
#endif // def CYGFUN_KERNEL_API_C
#else  // def CYGFUN_KERNEL_THREADS_TIMER
#define N_A_MSG "Kernel threads timer disabled"
#endif // def CYGFUN_KERNEL_THREADS_TIMER

#ifdef N_A_MSG
externC void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_NA( N_A_MSG );
}
#endif // N_A_MSG

// EOF kclock1.c
