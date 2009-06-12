//==========================================================================
//
//        kalarm0
//
//        Alarm functionality test
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2003 Nick Garnett <nickg@calivar.com>
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
// Alternative licenses for eCos may be arranged by contacting the
// copyright holders.
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     nickg
// Contributors:  nickg
// Date:          2003-06-25
// Description:   Tests the ability of alarms to be added and removed by the
//                alarm functions of other alarms.
//
//####DESCRIPTIONEND####
//==========================================================================

#include <cyg/kernel/kapi.h>

#include <cyg/infra/testcase.h>

//==========================================================================

#if defined(CYGVAR_KERNEL_COUNTERS_CLOCK) && \
    defined(CYGFUN_KERNEL_API_C)

#include "testaux.h"

//==========================================================================

//#define db_printf diag_printf
#define db_printf( fmt, ... )

//==========================================================================

static cyg_counter counter_obj;
static cyg_handle_t counter;
static cyg_alarm alarm_obj[3];
static cyg_handle_t alarm[3];

static cyg_uint32 alarmfn_called[3];

//==========================================================================

void alarmfn0(cyg_handle_t alarmh, cyg_addrword_t data)
{
    db_printf("%s: %d\n",__PRETTY_FUNCTION__,cyg_counter_current_value( counter ));

    // alarmfn0 just counts how many times it has been called
    
    alarmfn_called[0]++;
}

//==========================================================================

void alarmfn1(cyg_handle_t alarmh, cyg_addrword_t data)
{
    db_printf("%s: %d\n",__PRETTY_FUNCTION__,cyg_counter_current_value( counter ));

    alarmfn_called[1]++;

    // Reschedule alarm[0] to run every tick until alarm[2] next runs.
    
    cyg_alarm_initialize( alarm[0], cyg_counter_current_value( counter )+1, 1 );

}

//==========================================================================

void alarmfn2(cyg_handle_t alarmh, cyg_addrword_t data)
{
    db_printf("%s: %d\n",__PRETTY_FUNCTION__,cyg_counter_current_value( counter ));
    
    alarmfn_called[2]++;

    // Reschedule alarm[0] to run every 2 ticks until alarm[1] next runs.

    cyg_alarm_initialize( alarm[0], cyg_counter_current_value( counter )+1, 2 );

    // Reschedule alarm[1] to run every 3 ticks starting in 6 ticks time.
    
    cyg_alarm_initialize( alarm[1], cyg_counter_current_value( counter )+6, 3 );
}

//==========================================================================

void alarm0_main(void)
{
    int i;
    
    CYG_TEST_INIT();

    // Create the counter
    cyg_counter_create( &counter, &counter_obj );

    // Create the alarms
    cyg_alarm_create( counter,
                      alarmfn0,
                      0,
                      &alarm[0],
                      &alarm_obj[0]);


    cyg_alarm_create( counter,
                      alarmfn1,
                      1,
                      &alarm[1],
                      &alarm_obj[1]);

    cyg_alarm_create( counter,
                      alarmfn2,
                      2,
                      &alarm[2],
                      &alarm_obj[2]);


    // Kick it all off by starting alarm[2]
    cyg_alarm_initialize( alarm[2], 0, 10 );

    // Run the whole thing for 10000 ticks
    for( i = 0; i < 10000; i++ )
        cyg_counter_tick( counter );

    db_printf("alarmfn_called: %d %d %d\n",
                alarmfn_called[0],alarmfn_called[1],alarmfn_called[2]);

    CYG_TEST_CHECK( alarmfn_called[0]==5000, "alarmfn0 not called 5000 times\n");
    CYG_TEST_CHECK( alarmfn_called[1]==2000, "alarmfn1 not called 2000 times\n");
    CYG_TEST_CHECK( alarmfn_called[2]==1001, "alarmfn2 not called 1001 times\n");
    
    CYG_TEST_PASS_FINISH("KAlarm0");
}

//==========================================================================

externC void
cyg_start( void )
{
    alarm0_main();
}

//==========================================================================

#else

externC void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_NA( "This test needs CYGVAR_KERNEL_COUNTERS_CLOCK "
                 "and CYGFUN_KERNEL_API_C" );
}

#endif

//==========================================================================
// End of kalarm0.c
