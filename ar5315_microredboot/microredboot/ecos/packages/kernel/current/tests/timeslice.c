//==========================================================================
//
//        timeslice.c
//
//        Timeslice test
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
// Contributors:  nickg
// Date:          2001-06-18
// Description:   A basic timeslicing test.
//
//####DESCRIPTIONEND####
//==========================================================================

#include <pkgconf/kernel.h>
#include <pkgconf/hal.h>

#include <cyg/hal/hal_arch.h>

#include <cyg/kernel/smp.hxx>

#include <cyg/kernel/kapi.h>

#include <cyg/infra/testcase.h>
#include <cyg/infra/diag.h>

//==========================================================================

#if defined(CYGSEM_KERNEL_SCHED_TIMESLICE) &&   \
    defined(CYGFUN_KERNEL_API_C) &&             \
    defined(CYGSEM_KERNEL_SCHED_MLQUEUE) &&     \
    defined(CYGVAR_KERNEL_COUNTERS_CLOCK) &&    \
    !defined(CYGDBG_INFRA_DIAG_USE_DEVICE) &&   \
    (CYGNUM_KERNEL_SCHED_PRIORITIES > 12)

//==========================================================================

#define STACK_SIZE CYGNUM_HAL_STACK_SIZE_TYPICAL*5

#define NTHREADS_MAX (CYGNUM_KERNEL_CPU_MAX*6)

static int ncpus = CYGNUM_KERNEL_CPU_MAX;

static char test_stack[STACK_SIZE];
static cyg_thread test_thread;
static cyg_handle_t main_thread;

static char stacks[NTHREADS_MAX][STACK_SIZE];
static cyg_thread test_threads[NTHREADS_MAX];
static cyg_handle_t threads[NTHREADS_MAX];

static volatile int failed = false;

static volatile cyg_uint32 slicerun[NTHREADS_MAX][CYGNUM_KERNEL_CPU_MAX];

//==========================================================================

void 
test_thread_timeslice(CYG_ADDRESS id)
{
    for(;;)
        slicerun[id][CYG_KERNEL_CPU_THIS()]++;
}

//==========================================================================

void run_test_timeslice(int nthread)
{
    int i,j;
    cyg_uint32 cpu_total[CYGNUM_KERNEL_CPU_MAX];
    cyg_uint32 cpu_threads[CYGNUM_KERNEL_CPU_MAX];
    cyg_uint32 thread_total[NTHREADS_MAX];

    CYG_TEST_INFO( "Timeslice Test: Check timeslicing works");
    
    // Init flags.
    for (i = 0;  i < nthread;  i++)
        for( j = 0; j < ncpus; j++ )
            slicerun[i][j] = 0;
    
    // Set my priority higher than any I plan to create
    cyg_thread_set_priority(cyg_thread_self(), 2);

    for (i = 0;  i < nthread;  i++) {
        cyg_thread_create(10,              // Priority - just a number
                          test_thread_timeslice, // entry
                          i,               // index
                          "test_thread",     // Name
                          &stacks[i][0],   // Stack
                          STACK_SIZE,      // Size
                          &threads[i],     // Handle
                          &test_threads[i] // Thread data structure
            );
        cyg_thread_resume( threads[i]);
    }

    // Just wait a while, until the threads have all run for a bit.
    cyg_thread_delay( CYGNUM_KERNEL_SCHED_TIMESLICE_TICKS*100 );

    // Suspend all the threads
    for (i = 0;  i < nthread;  i++) {
        cyg_thread_suspend(threads[i]);
    }

    
    // And check that a thread ran on each CPU, and that each thread
    // ran.
    
    
    diag_printf(" Thread ");
    for( j = 0; j < ncpus; j++ )
    {
        cpu_total[j] = 0;
        cpu_threads[j] = 0;
        // "  %11d"  __123456789ab"
        diag_printf("       CPU %2d",j);
    }
    // "  %11d"  __123456789ab"
    diag_printf("        Total\n");
    for (i = 0;  i < nthread;  i++)
    {
        thread_total[i] = 0;
        diag_printf("     %2d ",i);
        for( j = 0; j < ncpus; j++ )
        {
            thread_total[i] += slicerun[i][j];
            cpu_total[j] += slicerun[i][j];
            if( slicerun[i][j] > 0 )
                cpu_threads[j]++;
            diag_printf("  %11d",slicerun[i][j]);
        }
        diag_printf("  %11d\n",thread_total[i]);
        if( thread_total[i] == 0 )
            failed++;
    }
    
    diag_printf(" Total  ");
    for( j = 0; j < ncpus; j++ )
        diag_printf("  %11d",cpu_total[j]);
    diag_printf("\n");
    diag_printf("Threads ");
    for( j = 0; j < ncpus; j++ )
    {
        diag_printf("  %11d",cpu_threads[j]);
        if( cpu_threads[j] < 2 )
            failed++;
    }
    diag_printf("\n");

    // Delete all the threads
    for (i = 0;  i < nthread;  i++) {
        cyg_thread_delete(threads[i]);
    }

    CYG_TEST_INFO( "Timeslice Test: done");
}


//==========================================================================

void 
run_tests(CYG_ADDRESS id)
{
    int step;
    int nthread;
    
    // Try to run about 10 times in total, with varying numbers of threads
    // from only one extra up to the full set:

    step = (NTHREADS_MAX - (1 + CYG_KERNEL_CPU_COUNT()))/10;
    if( step == 0 ) step = 1;
    
    for( nthread = 1 + CYG_KERNEL_CPU_COUNT() ;
         nthread <= NTHREADS_MAX ;
         nthread += step )
            run_test_timeslice(nthread);

    if( failed )
        CYG_TEST_FAIL_FINISH("Timeslice test failed\n");
    
    CYG_TEST_PASS_FINISH("Timeslice test OK");    
}

//==========================================================================

void timeslice_main( void )
{
    CYG_TEST_INIT();

    // Work out how many CPUs we actually have.
    ncpus = CYG_KERNEL_CPU_COUNT();

    cyg_thread_create(0,              // Priority - just a number
                      run_tests, // entry
                      0,               // index
                      "run_tests",     // Name
                      test_stack,   // Stack
                      STACK_SIZE,      // Size
                      &main_thread,     // Handle
                      &test_thread // Thread data structure
        );
    cyg_thread_resume( main_thread);
    
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
    timeslice_main();
}   

//==========================================================================

#else // CYGSEM_KERNEL_SCHED_TIMESLICE etc

externC void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_INFO("SMP test requires:\n"
                "CYGSEM_KERNEL_SCHED_TIMESLICE &&\n"
                "CYGPKG_KERNEL_SMP_SUPPORT &&\n"
                "CYGFUN_KERNEL_API_C && \n"
                "CYGSEM_KERNEL_SCHED_MLQUEUE &&\n"
                "CYGVAR_KERNEL_COUNTERS_CLOCK &&\n"
                "!CYGPKG_HAL_I386_LINUX &&\n"
                "!CYGDBG_INFRA_DIAG_USE_DEVICE &&\n"
                "(CYGNUM_KERNEL_SCHED_PRIORITIES > 12)\n");
    CYG_TEST_NA("SMP test requirements");
}

#endif // CYGSEM_KERNEL_SCHED_TIMESLICE etc.

//==========================================================================
// EOF timeslice.c
