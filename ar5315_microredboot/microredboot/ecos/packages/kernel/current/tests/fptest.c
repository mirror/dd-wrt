//==========================================================================
//
//        fptest.cxx
//
//        Basic FPU test
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2003 Nick Garnett
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
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     nickg@calivar.com
// Contributors:  nickg@calivar.com
// Date:          2003-01-27
// Description:   Simple FPU test. This is not very sophisticated as far
//                as checking FPU performance or accuracy. It is more
//                concerned with checking that several threads doing FP
//                operations do not interfere with eachother's use of the
//                FPU.
//
//####DESCRIPTIONEND####
//==========================================================================

#include <pkgconf/kernel.h>
#include <pkgconf/hal.h>

#include <cyg/hal/hal_arch.h>

#include <cyg/kernel/kapi.h>

#include <cyg/infra/testcase.h>
#include <cyg/infra/diag.h>

//#include <cyg/kernel/test/stackmon.h>
//#include CYGHWR_MEMORY_LAYOUT_H

//==========================================================================

#if defined(CYGFUN_KERNEL_API_C) &&             \
    defined(CYGSEM_KERNEL_SCHED_MLQUEUE) &&     \
    (CYGNUM_KERNEL_SCHED_PRIORITIES > 12)

//==========================================================================
// Base priority for all threads.

#define BASE_PRI        5

//==========================================================================
// Runtime
//
// This is the number of ticks that the program will run for. 3000
// ticks is equal to 30 seconds in the default configuration. For
// simulators we reduce the run time to 3 simulated seconds.

#define RUN_TICKS       3000
#define RUN_TICKS_SIM   300

//==========================================================================
// Thread parameters

#define STACK_SIZE (CYGNUM_HAL_STACK_SIZE_MINIMUM)

static cyg_uint8 stacks[3][STACK_SIZE];
static cyg_handle_t thread[3];
static cyg_thread thread_struct[3];

//==========================================================================
// Alarm parameters.

static cyg_alarm alarm_struct;
static cyg_handle_t alarm;

static cyg_count8 cur_thread = 0;
static cyg_count32 alarm_ticks = 0;
static cyg_count32 run_ticks = RUN_TICKS;

//==========================================================================

static int errors = 0;

//==========================================================================
// Random number generator. Ripped out of the C library.

static int rand( unsigned int *seed )
{
// This is the code supplied in Knuth Vol 2 section 3.6 p.185 bottom

#define RAND_MAX 0x7fffffff
#define MM 2147483647    // a Mersenne prime
#define AA 48271         // this does well in the spectral test
#define QQ 44488         // (long)(MM/AA)
#define RR 3399          // MM % AA; it is important that RR<QQ

    *seed = AA*(*seed % QQ) - RR*(unsigned int)(*seed/QQ);
    if (*seed < 0)
        *seed += MM;

    return (int)( *seed & RAND_MAX );
}

//==========================================================================
// Test calculation.
//
// Generates an array of random FP values and then repeatedly applies
// a calculation to them and checks that the same result is reached
// each time. The calculation, in the macro CALC, is intended to make
// maximum use of the FPU registers. However, the i386 compiler
// doesn't let this expression get very complex before it starts
// spilling values out to memory.

static void do_test( double *values,
                     int count,
                     int loops,
                     int test,
                     const char *name)
{
    unsigned int i, j;
    // volatiles necessary to force
    // values to 64 bits for comparison
    volatile double sum = 1.0;
    volatile double last_sum;
    unsigned int seed;
    
#define V(__i) (values[(__i)%count])
#define CALC ((V(i-1)*V(i+1))*(V(i-2)*V(i+2))*(V(i-3)*sum))

    seed = ((unsigned int)&i)*count;

    // Set up an array of values...
    for( i = 0; i < count; i++ )
        values[i] = (double)rand( &seed )/(double)0x7fffffff;

    // Now calculate something from them...
    for( i = 0; i < count; i++ )
        sum += CALC;
    last_sum = sum;
    
    // Now recalculate the sum in a loop and look for errors
    for( j = 0; j < loops ; j++ )
    {
        sum = 1.0;
        for( i = 0; i < count; i++ )
            sum += CALC;

        if( sum != last_sum )
        {
            union double_int_union {
                double d;
                cyg_uint32 i[2];
            } diu_sum, diu_lastsum;

            diu_sum.d = sum;
            diu_lastsum.d = last_sum;
            
            errors++;
            if (sizeof(double) != 2*sizeof(cyg_uint32)) {
                diag_printf("Warning: sizeof(double) != 2*sizeof(cyg_uint32), therefore next line may\n"
                            "have invalid sum/last_sum values\n");
            }
            diag_printf("%s: Sum mismatch! %d sum=[%08x:%08x] last_sum=[%08x:%08x]\n",
                        name,j, diu_sum.i[0], diu_sum.i[1], diu_lastsum.i[0], diu_lastsum.i[1] );
        }
        
#if 0
        if( ((j*count)%1000000) == 0 )
            diag_printf("INFO:<%s: %2d calculations done>\n",name,j*count);
#endif
    }

}

//==========================================================================
// Alarm handler
//
// This is called every tick. It lowers the priority of the currently
// running thread and raises the priority of the next. Thus we
// implement a form of timelslicing between the threads at one tick
// granularity.

static void alarm_fn(cyg_handle_t alarm, cyg_addrword_t data)
{
    alarm_ticks++;

    if( alarm_ticks >= run_ticks )
    {
        if( errors )
            CYG_TEST_FAIL("Errors detected");
        else
            CYG_TEST_PASS("OK");            
        
        CYG_TEST_FINISH("FP Test done");
    }
    else
    {
        cyg_thread_set_priority( thread[cur_thread], BASE_PRI );

        cur_thread = (cur_thread+1)%3;

        cyg_thread_set_priority( thread[cur_thread], BASE_PRI-1 );
    }
}


//==========================================================================

#define FP1_COUNT 1000

static double fpt1_values[FP1_COUNT];

void fptest1( CYG_ADDRWORD id )
{
    while(1)
        do_test( fpt1_values, FP1_COUNT, 2000000000, id, "fptest1" );
}

//==========================================================================

#define FP2_COUNT 10000

static double fpt2_values[FP2_COUNT];

void fptest2( CYG_ADDRWORD id )
{
    while(1)
        do_test( fpt2_values, FP2_COUNT, 2000000000, id, "fptest2" );
}

//==========================================================================

#define FP3_COUNT 100

static double fpt3_values[FP3_COUNT];

void fptest3( CYG_ADDRWORD id )
{
    while(1)
        do_test( fpt3_values, FP3_COUNT, 2000000000, id, "fptest3" );
}

//==========================================================================

void fptest_main( void )
{
    
    CYG_TEST_INIT();

    if( cyg_test_is_simulator )
    {
        run_ticks = RUN_TICKS_SIM;
    }

    CYG_TEST_INFO("Run fptest in cyg_start");
    do_test( fpt3_values, FP3_COUNT, 1000, 0, "start" );
    CYG_TEST_INFO( "cyg_start run done");
    
    cyg_thread_create( BASE_PRI-1,
                       fptest1,
                       0,
                       "fptest1",
                       &stacks[0][0],
                       STACK_SIZE,
                       &thread[0],
                       &thread_struct[0]);

    cyg_thread_resume( thread[0] );

    cyg_thread_create( BASE_PRI,
                       fptest2,
                       1,
                       "fptest2",
                       &stacks[1][0],
                       STACK_SIZE,
                       &thread[1],
                       &thread_struct[1]);

    cyg_thread_resume( thread[1] );

    cyg_thread_create( BASE_PRI,
                       fptest3,
                       2,
                       "fptest3",
                       &stacks[2][0],
                       STACK_SIZE,
                       &thread[2],
                       &thread_struct[2]);

    cyg_thread_resume( thread[2] );

    cyg_alarm_create( cyg_real_time_clock(),
                      alarm_fn,
                      0,
                      &alarm,
                      &alarm_struct );

    cyg_alarm_initialize( alarm, cyg_current_time()+1, 1 );
    
    cyg_scheduler_start();

}

//==========================================================================

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
    fptest_main();
}   

//==========================================================================

#else // CYGFUN_KERNEL_API_C...

externC void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_INFO("FP test requires:\n"
                "CYGFUN_KERNEL_API_C && \n"
                "CYGSEM_KERNEL_SCHED_MLQUEUE && \n"
                "(CYGNUM_KERNEL_SCHED_PRIORITIES > 12)\n");
    CYG_TEST_NA("FP test requirements");
}

#endif // CYGFUN_KERNEL_API_C, etc.

//==========================================================================
// EOF fptest.cxx
