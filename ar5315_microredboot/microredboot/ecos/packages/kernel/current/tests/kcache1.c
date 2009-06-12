/*=================================================================
//
//        kcache1.c
//
//        Cache timing test
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
// Contributors:  dsm, nickg
// Date:          1998-06-18
//####DESCRIPTIONEND####
*/

#include <cyg/hal/hal_arch.h>           // CYGNUM_HAL_STACK_SIZE_TYPICAL

#include <cyg/kernel/kapi.h>

#include <cyg/infra/testcase.h>

#include <cyg/hal/hal_cache.h>

#if defined(HAL_DCACHE_SIZE) || defined(HAL_UCACHE_SIZE)
#ifdef CYGVAR_KERNEL_COUNTERS_CLOCK
#ifdef CYGFUN_KERNEL_API_C

#include <cyg/infra/diag.h>
#include <cyg/hal/hal_intr.h>

// -------------------------------------------------------------------------
// If the HAL does not supply this, we supply our own version

#ifndef HAL_DCACHE_PURGE_ALL
# ifdef HAL_DCACHE_SYNC

#define HAL_DCACHE_PURGE_ALL() HAL_DCACHE_SYNC()

# else

static cyg_uint8 dca[HAL_DCACHE_SIZE + HAL_DCACHE_LINE_SIZE*2];

#define HAL_DCACHE_PURGE_ALL()                                          \
CYG_MACRO_START                                                         \
    volatile cyg_uint8 *addr = &dca[HAL_DCACHE_LINE_SIZE];              \
    volatile cyg_uint8 tmp = 0;                                         \
    int i;                                                              \
    for( i = 0; i < HAL_DCACHE_SIZE; i += HAL_DCACHE_LINE_SIZE )        \
    {                                                                   \
        tmp = addr[i];                                                  \
    }                                                                   \
CYG_MACRO_END

# endif
#endif

// -------------------------------------------------------------------------

#define NTHREADS 1
#define STACKSIZE CYGNUM_HAL_STACK_SIZE_TYPICAL

static cyg_handle_t thread[NTHREADS];

static cyg_thread thread_obj[NTHREADS];
static char stack[NTHREADS][STACKSIZE];

#ifndef MAX_STRIDE
#define MAX_STRIDE 64
#endif

volatile char m[(HAL_DCACHE_SIZE/HAL_DCACHE_LINE_SIZE)*MAX_STRIDE];

// -------------------------------------------------------------------------

static void time0(register cyg_uint32 stride)
{
    register cyg_uint32 j,k;
    cyg_tick_count_t count0, count1;
    cyg_ucount32 t;
    register char c;

    count0 = cyg_current_time();

    k = 0;
    if ( cyg_test_is_simulator )
        k = 3960;

    for(; k<4000;k++) {
        for(j=0; j<(HAL_DCACHE_SIZE/HAL_DCACHE_LINE_SIZE); j++) {
            c=m[stride*j];
        }
    }

    count1 = cyg_current_time();
    t = count1 - count0;
    diag_printf("stride=%d, time=%d\n", stride, t);
}

// -------------------------------------------------------------------------

void time1(void)
{
    cyg_uint32 i;

    for(i=1; i<=MAX_STRIDE; i+=i) {
        time0(i);
    }
}

// -------------------------------------------------------------------------
// With an ICache invalidate in the middle:
#ifdef HAL_ICACHE_INVALIDATE_ALL
static void time0II(register cyg_uint32 stride)
{
    register cyg_uint32 j,k;
    cyg_tick_count_t count0, count1;
    cyg_ucount32 t;
    register char c;

    count0 = cyg_current_time();

    k = 0;
    if ( cyg_test_is_simulator )
        k = 3960;

    for(; k<4000;k++) {
        for(j=0; j<(HAL_DCACHE_SIZE/HAL_DCACHE_LINE_SIZE); j++) {
            HAL_ICACHE_INVALIDATE_ALL();
            c=m[stride*j];
        }
    }

    count1 = cyg_current_time();
    t = count1 - count0;
    diag_printf("stride=%d, time=%d\n", stride, t);
}

// -------------------------------------------------------------------------

void time1II(void)
{
    cyg_uint32 i;

    for(i=1; i<=MAX_STRIDE; i+=i) {
        time0II(i);
    }
}
#endif
// -------------------------------------------------------------------------
// With a DCache invalidate in the middle:
// This is guaranteed to produce bogus timing results since interrupts
// have to be disabled to prevent accidental loss of state.
#ifdef HAL_DCACHE_INVALIDATE_ALL
static void time0DI(register cyg_uint32 stride)
{
    register cyg_uint32 j,k;
    volatile cyg_tick_count_t count0;
    cyg_tick_count_t count1;
    cyg_ucount32 t;
    register char c;
    register CYG_INTERRUPT_STATE oldints;

    count0 = cyg_current_time();

    HAL_DISABLE_INTERRUPTS(oldints);
    HAL_DCACHE_SYNC();

    k = 0;
    if ( cyg_test_is_simulator )
        k = 3960;

    for(; k<4000;k++) {
        for(j=0; j<(HAL_DCACHE_SIZE/HAL_DCACHE_LINE_SIZE); j++) {
            HAL_DCACHE_INVALIDATE_ALL();
            c=m[stride*j];
        }
    }
    HAL_RESTORE_INTERRUPTS(oldints);    

    count1 = cyg_current_time();
    t = count1 - count0;
    diag_printf("stride=%d, time=%d\n", stride, t);
}

// -------------------------------------------------------------------------

void time1DI(void)
{
    cyg_uint32 i;

    for(i=1; i<=MAX_STRIDE; i+=i) {
        time0DI(i);
    }
}
#endif
// -------------------------------------------------------------------------
// This test could be improved by counting number of passes possible 
// in a given number of ticks.

static void entry0( cyg_addrword_t data )
{
    register CYG_INTERRUPT_STATE oldints;

#ifdef HAL_CACHE_UNIFIED

    HAL_DISABLE_INTERRUPTS(oldints);
    HAL_DCACHE_PURGE_ALL();             // rely on above definition
    HAL_UCACHE_INVALIDATE_ALL();
    HAL_UCACHE_DISABLE();
    HAL_RESTORE_INTERRUPTS(oldints);
    CYG_TEST_INFO("Cache off");
    time1();

    HAL_DISABLE_INTERRUPTS(oldints);
    HAL_DCACHE_PURGE_ALL();             // rely on above definition
    HAL_UCACHE_INVALIDATE_ALL();
    HAL_UCACHE_ENABLE();
    HAL_RESTORE_INTERRUPTS(oldints);
    CYG_TEST_INFO("Cache on");
    time1();

#ifdef HAL_DCACHE_INVALIDATE_ALL
    HAL_DISABLE_INTERRUPTS(oldints);
    HAL_DCACHE_PURGE_ALL();
    HAL_UCACHE_INVALIDATE_ALL();
    HAL_UCACHE_ENABLE();
    HAL_RESTORE_INTERRUPTS(oldints);    
    CYG_TEST_INFO("Cache on: invalidate Cache (expect bogus timing)");
    time1DI();
#endif

#else // HAL_CACHE_UNIFIED

    HAL_DISABLE_INTERRUPTS(oldints);
    HAL_DCACHE_PURGE_ALL();
    HAL_ICACHE_INVALIDATE_ALL();
    HAL_DCACHE_INVALIDATE_ALL();
    HAL_ICACHE_DISABLE();
    HAL_DCACHE_DISABLE();
    HAL_RESTORE_INTERRUPTS(oldints);
    CYG_TEST_INFO("Dcache off Icache off");
    time1();

    HAL_DISABLE_INTERRUPTS(oldints);
    HAL_DCACHE_PURGE_ALL();
    HAL_ICACHE_INVALIDATE_ALL();
    HAL_DCACHE_INVALIDATE_ALL();
    HAL_ICACHE_DISABLE();
    HAL_DCACHE_ENABLE();
    HAL_RESTORE_INTERRUPTS(oldints);    
    CYG_TEST_INFO("Dcache on  Icache off");
    time1();

    HAL_DISABLE_INTERRUPTS(oldints);
    HAL_DCACHE_PURGE_ALL();
    HAL_ICACHE_INVALIDATE_ALL();
    HAL_DCACHE_INVALIDATE_ALL();
    HAL_ICACHE_ENABLE();
    HAL_DCACHE_DISABLE();
    HAL_RESTORE_INTERRUPTS(oldints);    
    CYG_TEST_INFO("Dcache off Icache on");
    time1();

    HAL_DISABLE_INTERRUPTS(oldints);
    HAL_DCACHE_PURGE_ALL();
    HAL_ICACHE_INVALIDATE_ALL();
    HAL_DCACHE_INVALIDATE_ALL();
    HAL_ICACHE_ENABLE();
    HAL_DCACHE_ENABLE();
    HAL_RESTORE_INTERRUPTS(oldints);    
    CYG_TEST_INFO("Dcache on Icache on");
    time1();


    HAL_DISABLE_INTERRUPTS(oldints);
    HAL_DCACHE_PURGE_ALL();
    HAL_ICACHE_INVALIDATE_ALL();
    HAL_DCACHE_INVALIDATE_ALL();
    HAL_ICACHE_DISABLE();
    HAL_DCACHE_DISABLE();
    HAL_RESTORE_INTERRUPTS(oldints);    
    CYG_TEST_INFO("Dcache off Icache off (again)");
    time1();

#if defined(HAL_DCACHE_INVALIDATE_ALL) || defined(HAL_ICACHE_INVALIDATE_ALL)
    HAL_DISABLE_INTERRUPTS(oldints);
    HAL_DCACHE_PURGE_ALL();
    HAL_ICACHE_INVALIDATE_ALL();
    HAL_DCACHE_INVALIDATE_ALL();
    HAL_ICACHE_ENABLE();
    HAL_DCACHE_ENABLE();
    HAL_RESTORE_INTERRUPTS(oldints);    
    CYG_TEST_INFO("Dcache on Icache on (again)");
    time1();

#if defined(CYGPKG_HAL_MIPS)
    // In some architectures, the time taken for the next two tests is
    // very long, partly because HAL_XCACHE_INVALIDATE_ALL() is implemented
    // with a loop over the cache. Hence these tests take longer than the
    // testing infrastructure is prepared to wait. The simplest way to get
    // these tests to run quickly is to make them think they are running
    // under a simulator.
    // If the target actually is a simulator, skip the below - it's very
    // slow on the simulator, even with reduced loop counts.
    if (cyg_test_is_simulator)
        CYG_TEST_PASS_FINISH("End of test");

#if defined(CYGPKG_HAL_MIPS_TX49)
    // The TX49 has a large cache, and even with reduced loop count,
    // 90+ seconds elapses between each INFO output.
    CYG_TEST_PASS_FINISH("End of test");
#endif

    cyg_test_is_simulator = 1;
#endif    

#ifdef HAL_ICACHE_INVALIDATE_ALL
    HAL_DISABLE_INTERRUPTS(oldints);
    HAL_DCACHE_PURGE_ALL();
    HAL_ICACHE_INVALIDATE_ALL();
    HAL_DCACHE_INVALIDATE_ALL();
    HAL_ICACHE_ENABLE();
    HAL_DCACHE_ENABLE();
    HAL_RESTORE_INTERRUPTS(oldints);    
    CYG_TEST_INFO("Dcache on Icache on: invalidate ICache each time");
    time1II();
#endif
#ifdef HAL_DCACHE_INVALIDATE_ALL
    HAL_DISABLE_INTERRUPTS(oldints);
    HAL_DCACHE_PURGE_ALL();
    HAL_ICACHE_INVALIDATE_ALL();
    HAL_DCACHE_INVALIDATE_ALL();
    HAL_ICACHE_ENABLE();
    HAL_DCACHE_ENABLE();
    HAL_RESTORE_INTERRUPTS(oldints);    
    CYG_TEST_INFO("Dcache on Icache on: invalidate DCache (expect bogus times)");
    time1DI();
#endif
#endif // either INVALIDATE_ALL macro

#endif // HAL_CACHE_UNIFIED

    CYG_TEST_PASS_FINISH("End of test");
}

// -------------------------------------------------------------------------

void kcache2_main( void )
{
    CYG_TEST_INIT();

    cyg_thread_create(4, entry0 , (cyg_addrword_t)0, "kcache1",
        (void *)stack[0], STACKSIZE, &thread[0], &thread_obj[0]);
    cyg_thread_resume(thread[0]);

    cyg_scheduler_start();
}

// -------------------------------------------------------------------------

externC void
cyg_start( void )
{
    kcache2_main();
}

// -------------------------------------------------------------------------

#else // def CYGFUN_KERNEL_API_C
#define N_A_MSG "Kernel C API layer disabled"
#endif // def CYGFUN_KERNEL_API_C
#else // def CYGVAR_KERNEL_COUNTERS_CLOCK
#define N_A_MSG "Kernel real-time clock disabled"
#endif // def CYGVAR_KERNEL_COUNTERS_CLOCK
#else // def HAL_DCACHE_SIZE
#define N_A_MSG "No caches defined"
#endif // def HAL_DCACHE_SIZE

#ifdef N_A_MSG
externC void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_NA( N_A_MSG );
}
#endif // N_A_MSG

// -------------------------------------------------------------------------
/* EOF kcache1.c */
