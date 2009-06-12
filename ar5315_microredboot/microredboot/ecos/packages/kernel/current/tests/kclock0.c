/*=================================================================
//
//        kclock0.c
//
//        Kernel C API Clock test 0
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
// Description:   Tests some basic clock functions.
//####DESCRIPTIONEND####
*/

#include <cyg/kernel/kapi.h>

#include <cyg/infra/testcase.h>

#ifdef CYGVAR_KERNEL_COUNTERS_CLOCK

#ifdef CYGFUN_KERNEL_API_C

#include "testaux.h"

cyg_alarm_t call_me;

cyg_counter counter0o, counter1o;
cyg_handle_t counter0, counter1;

cyg_alarm alarmo[3];
cyg_handle_t alarm0, alarm1, alarm2;

cyg_resolution_t res, res0, res1;

cyg_clock clock0o;
cyg_handle_t clock0;

const cyg_uint32 big_number = 3333222111u;

cyg_bool_t flash( void )
{
    cyg_counter_create( &counter0, &counter0o );
    cyg_counter_create( &counter1, &counter1o );
    
    cyg_alarm_create( counter0,
                      call_me,
                      (cyg_addrword_t)12,
                      &alarm0,
                      &alarmo[0]);

    res.dividend = 1;
    res.divisor = 2;

    cyg_clock_create( res, &clock0, &clock0o );
    cyg_clock_delete( clock0 );

    cyg_alarm_delete( alarm0 );

    cyg_counter_delete( counter0 );
    cyg_counter_delete( counter1 );

    return true;
}

/* Testing alarms
//
// call_me is a function that will be called when an alarm is
// triggered.  It updates a global variable called which is CHECKed
// explicitly to see if the approriate alarms have been called.
*/

cyg_uint16 called = 0x0;

void call_me(cyg_handle_t alarm, cyg_addrword_t data)
{
    called ^= (int)data;
}

void call_me2(cyg_handle_t alarm, cyg_addrword_t data)
{
    call_me(alarm, (cyg_addrword_t)((int)data^0x10));
}


void kclock0_main(void)
{
    CYG_TEST_INIT();

    CHECK(flash());
    CHECK(flash());

    cyg_counter_create( &counter0, &counter0o);

    CHECK( 0 == cyg_counter_current_value( counter0 ) );

    cyg_counter_tick(counter0);

    CHECK( 1 == cyg_counter_current_value(counter0) );

    cyg_counter_tick(counter0);

    CHECK( 2 == cyg_counter_current_value(counter0) );

    cyg_counter_set_value( counter0, 0xffffffff );

    CHECK( 0xffffffff == cyg_counter_current_value(counter0) );

    cyg_counter_tick(counter0); // Overflows 32 bits
    
    CHECK( 0x100000000ULL == cyg_counter_current_value(counter0) );

    cyg_counter_set_value(counter0, 11);
    CHECK( 11 == cyg_counter_current_value(counter0) );
    
    /* the call_me functions cause the "called" bits to toggle
    // checking the value of called checks the parity of # of calls
    // made by each alarm.
    */

    cyg_alarm_create(counter0,
                     call_me,  (cyg_addrword_t)0x1, &alarm0, &alarmo[0]);
    cyg_alarm_create(counter0,
                     call_me,  (cyg_addrword_t)0x2, &alarm1, &alarmo[1]);
    cyg_alarm_create(counter0,
                     call_me2, (cyg_addrword_t)0x4, &alarm2, &alarmo[2]);
    
    CHECK( 0x00 == called );
    cyg_alarm_initialize(alarm0, 12,3);
    cyg_alarm_initialize(alarm2, 21,2);
    CHECK( 0x00 == called );

    cyg_counter_tick(counter0);         /* 12 a0 */
    CHECK( 0x01 == called );

    cyg_alarm_initialize(alarm1, 13,0);
    cyg_counter_tick(counter0);         /* 13 a1 */
    CHECK( 0x03 == called );

    cyg_alarm_initialize(alarm1, 17,0);
    cyg_counter_tick(counter0);         /* 14 */
    CHECK( 0x03 == called );

    cyg_counter_tick(counter0);         /* 15 a0 */
    CHECK( 0x02 == called );

    cyg_counter_tick(counter0);         /* 16 */
    cyg_counter_tick(counter0);         /* 17 a1 */
    CHECK( 0x00 == called );

    cyg_counter_tick(counter0);         /* 18 a0 */
    CHECK( 0x01 == called );

    cyg_counter_tick(counter0);         /* 19 */
    cyg_counter_tick(counter0);         /* 20 */
    cyg_counter_tick(counter0);         /* 21 a0 a2 */
    CHECK( 0x14 == called );

    cyg_counter_tick(counter0);         /* 22 */
    cyg_counter_tick(counter0);         /* 23 a2 */
    CHECK( 0x00 == called );
    
    cyg_alarm_disable(alarm2);  

    cyg_counter_tick(counter0);         /* 24 a0 */
    cyg_counter_tick(counter0);         /* 25 */
    CHECK( 0x01 == called );
    
    cyg_alarm_enable(alarm2);           /* a2 (enabled at 25) */
    CHECK( 0x15 == called );

    cyg_counter_tick(counter0);         /* 26 */
    CHECK( 0x15 == called );
    
    cyg_counter_tick(counter0);         /* 27 a0 a2 */
    cyg_counter_tick(counter0);         /* 28 */
    CHECK( 0x00 == called );
    
    cyg_counter_tick(counter0);         /* 29 a2 */
    cyg_counter_tick(counter0);         /* 30 a0 */
    cyg_counter_tick(counter0);         /* 31 a2 */
    CHECK( 0x01 == called );

    res0.dividend = 100;
    res0.divisor   = 3;

    cyg_clock_create( res0, &clock0, &clock0o );

    res1 = cyg_clock_get_resolution(clock0);
    CHECK( res0.dividend == res1.dividend );
    CHECK( res0.divisor == res1.divisor );

    res1.dividend = 12;
    res1.divisor = 25;

    cyg_clock_set_resolution(clock0, res1);
    res0 = cyg_clock_get_resolution(clock0);
    CHECK( res0.dividend == res1.dividend );
    CHECK( res0.divisor == res1.divisor );

    cyg_clock_to_counter(clock0, &counter1);

    CHECK( 0 == cyg_counter_current_value( counter1 ) );
    CHECK( 0 == cyg_current_time() );

    cyg_counter_tick(counter1);

    CHECK( 1 == cyg_counter_current_value(counter1) );

    res0 = cyg_clock_get_resolution(cyg_real_time_clock());

    /* Current time should be 0 as interrupts will still be disabled */
    CHECK( 0 == cyg_current_time() );

    CYG_TEST_PASS_FINISH("Kernel C API Clock 0 OK");
}

externC void
cyg_start( void )
{
    kclock0_main();
}

#else // def CYGFUN_KERNEL_API_C
#define N_A_MSG "Kernel C API layer disabled"
#endif // def CYGFUN_KERNEL_API_C
#else // def CYGVAR_KERNEL_COUNTERS_CLOCK
#define N_A_MSG "Kernel real-time clock disabled"
#endif // def CYGVAR_KERNEL_COUNTERS_CLOCK

#ifdef N_A_MSG
externC void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_NA( N_A_MSG );
}
#endif // N_A_MSG

// EOF kclock0.c
