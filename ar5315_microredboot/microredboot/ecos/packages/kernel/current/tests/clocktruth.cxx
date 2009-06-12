//==========================================================================
//
//        clocktruth.cxx
//
//        Clock Converter test
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
// Author(s):     hmt
// Contributors:  hmt
// Date:          2001-06-05
// Description:   Tests the Kernel Real Time Clock for accuracy using a human
// 
//####DESCRIPTIONEND####


// This is for a human to watch to sanity check the clock rate.
// It's easier to see what's happening if you enable this:
#define nRUNFOREVER


#include <pkgconf/kernel.h>

#include <cyg/kernel/clock.hxx>
#include <cyg/kernel/sema.hxx>
#include <cyg/kernel/thread.hxx>

#include <cyg/infra/testcase.h>

#include <cyg/kernel/clock.inl>
#include <cyg/kernel/thread.inl>

#ifdef CYGVAR_KERNEL_COUNTERS_CLOCK

#include <cyg/infra/diag.h>

#define NTHREADS 1
#include "testaux.hxx"

#ifdef RUNFOREVER
#define ENDPOINT 8192
#else
#define ENDPOINT 20
#endif

static cyg_alarm_fn alarmfunc;
static void alarmfunc( Cyg_Alarm *alarm, CYG_ADDRWORD data )
{
    Cyg_Binary_Semaphore *sp = (Cyg_Binary_Semaphore *)data;
    sp->post();
}


static void entry0( CYG_ADDRWORD data )
{
    cyg_uint32 now, then;
    int i;

    Cyg_Clock *rtc = Cyg_Clock::real_time_clock;

    Cyg_Binary_Semaphore sema;

    Cyg_Alarm alarm( rtc, &alarmfunc, (CYG_ADDRWORD)&sema );

    // First, print 100 lines as fast as you can, of distinct ticks.
    for ( i = 0; i < 100; i++ ) {
        now = rtc->current_value_lo();
        then = now;
        while ( then == now )
            now = rtc->current_value_lo();

        diag_printf( "INFO<time now %8d>\n", now );
    }

    diag_printf( "INFO<per-second times are: %8d>\n", rtc->current_value_lo() );
    for ( i = 0; i < 20; i++ ) {
        Cyg_Thread::counted_sleep( 100 );
        diag_printf( "INFO<per-second time %2d is %8d>\n",
                     i, rtc->current_value_lo() );
    }

    alarm.initialize( rtc->current_value() + 100, 100 );
    alarm.enable();
    for ( i = 0; i < ENDPOINT; i++ ) {
        sema.wait();
        diag_printf( "INFO<alarm time %2d is %8d>\n",
                     i, rtc->current_value_lo() );
    }

    CYG_TEST_PASS_FINISH("Clock truth OK");
}

void clocktruth_main( void )
{
    CYG_TEST_INIT();
    new_thread(entry0, (CYG_ADDRWORD)&thread_obj[0]);
    Cyg_Scheduler::start();
}

externC void
cyg_start( void )
{
#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
    cyg_hal_invoke_constructors();
#endif
    clocktruth_main();
}

#else // def CYGVAR_KERNEL_COUNTERS_CLOCK

externC void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_NA( "Kernel real-time clock disabled");
}

#endif // def CYGVAR_KERNEL_COUNTERS_CLOCK

// EOF clocktruth.cxx
