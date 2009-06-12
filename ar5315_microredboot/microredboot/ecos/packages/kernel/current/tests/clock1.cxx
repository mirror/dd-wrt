//==========================================================================
//
//        clock1.cxx
//
//        Clock test 1 - Real Time Clock
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
// Date:          1998-02-16
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

#include <pkgconf/kernel.h>

#include <cyg/kernel/clock.hxx>
#include <cyg/kernel/thread.hxx>

#include <cyg/infra/testcase.h>

#include <cyg/kernel/clock.inl>
#include <cyg/kernel/thread.inl>

#ifdef CYGVAR_KERNEL_COUNTERS_CLOCK


#define NTHREADS 1
#include "testaux.hxx"

static cyg_uint32 ticks;     // Number of ticks thread[0] will delay for

static cyg_uint64 TEST_DELAY;

static void entry0( CYG_ADDRWORD data )
{
    ((Cyg_Thread *)data)->delay(ticks);

    CYG_TEST_PASS_FINISH("Clock 1 OK");
}

void clock1_main( void )
{
    CYG_TEST_INIT();

    if (cyg_test_is_simulator) {
        TEST_DELAY = 100000000ll;
    } else {
        TEST_DELAY = 3000000000ll;
    }

    new_thread(entry0, (CYG_ADDRWORD)&thread_obj[0]);

    Cyg_Clock::cyg_resolution res;

    res = Cyg_Clock::real_time_clock->get_resolution ();

    // RTC takes res.dividend/res.divisor ns/tick
    ticks = ((cyg_uint64)TEST_DELAY * res.divisor) / res.dividend;

    Cyg_Scheduler::start();
}

externC void
cyg_start( void )
{
#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
    cyg_hal_invoke_constructors();
#endif
    clock1_main();
}

#else // def CYGVAR_KERNEL_COUNTERS_CLOCK

externC void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_NA( "Kernel real-time clock disabled");
}

#endif // def CYGVAR_KERNEL_COUNTERS_CLOCK

// EOF clock1.cxx
