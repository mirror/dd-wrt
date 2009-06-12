//==========================================================================
//
//        tm_basic.cxx
//
//        Basic timing test / scaffolding
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002 Jonathan Larmour
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
// Author(s):     gthomas,nickg
// Contributors:  jlarmour
// Date:          1998-10-19
// Description:   Very simple kernel timing test
//####DESCRIPTIONEND####
//==========================================================================


#include <cyg/infra/testcase.h>
#include <cyg/infra/diag.h>
#include <pkgconf/posix.h>
#include <pkgconf/system.h>
#ifdef CYGPKG_KERNEL
#include <pkgconf/kernel.h>
#endif

#ifndef CYGPKG_POSIX_SIGNALS
#define NA_MSG "No POSIX signals"
#elif !defined(CYGPKG_POSIX_TIMERS)
#define NA_MSG "No POSIX timers"
#elif !defined(CYGPKG_POSIX_PTHREAD)
#define NA_MSG "POSIX threads not enabled"
#elif !defined(CYGFUN_KERNEL_API_C)
#define NA_MSG "Kernel C API not enabled"
#elif !defined(CYGSEM_KERNEL_SCHED_MLQUEUE)
#define NA_MSG "Kernel mlqueue scheduler not enabled"
#elif !defined(CYGVAR_KERNEL_COUNTERS_CLOCK)
#define NA_MSG "Kernel clock not enabled"
#elif CYGNUM_KERNEL_SCHED_PRIORITIES <= 12
#define NA_MSG "Kernel scheduler properties <= 12"
#endif

//==========================================================================

#ifdef NA_MSG
extern "C" void
cyg_start(void)
{
    CYG_TEST_INIT();
    CYG_TEST_NA(NA_MSG);
}
#else

#include <pkgconf/kernel.h>
#include <pkgconf/hal.h>

#include <cyg/kernel/sched.hxx>
#include <cyg/kernel/thread.hxx>
#include <cyg/kernel/thread.inl>
#include <cyg/kernel/mutex.hxx>
#include <cyg/kernel/sema.hxx>
#include <cyg/kernel/sched.inl>
#include <cyg/kernel/clock.hxx>
#include <cyg/kernel/clock.inl>
#include <cyg/kernel/kapi.h>

#include <cyg/infra/testcase.h>

#include <cyg/kernel/test/stackmon.h>
#include CYGHWR_MEMORY_LAYOUT_H


// POSIX headers

#include <sys/types.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <signal.h>
#include <errno.h>

//==========================================================================
// Define this to see the statistics with the first sample datum removed.
// This can expose the effects of caches on the speed of operations.

#undef STATS_WITHOUT_FIRST_SAMPLE

//==========================================================================

// Structure used to keep track of times
typedef struct fun_times {
    cyg_uint32 start;
    cyg_uint32 end;
} fun_times;

//==========================================================================

#define STACK_SIZE (PTHREAD_STACK_MIN*2)

// Defaults
#define NTEST_THREADS    16
#define NMUTEXES         32
#define NMBOXES          32
#define NSEMAPHORES      32
#define NTIMERS          32


#define NSAMPLES         32
#define NTHREAD_SWITCHES 128
#define NSCHEDS          128

#define NSAMPLES_SIM         2
#define NTEST_THREADS_SIM    2
#define NTHREAD_SWITCHES_SIM 4
#define NMUTEXES_SIM         2
#define NMBOXES_SIM          2
#define NSEMAPHORES_SIM      2
#define NSCHEDS_SIM          4
#define NTIMERS_SIM          2

//==========================================================================

static int nsamples;
static int ntest_threads;
static int nthread_switches;
static int nmutexes;
static int nmboxes;
static int nsemaphores;
static int nscheds;
static int ntimers;

static char stacks[NTEST_THREADS][STACK_SIZE];
static pthread_t threads[NTEST_THREADS];
static int overhead;
static sem_t synchro;
static fun_times thread_ft[NTEST_THREADS];

static fun_times test2_ft[NTHREAD_SWITCHES];

static pthread_mutex_t test_mutexes[NMUTEXES];
static fun_times mutex_ft[NMUTEXES];
static pthread_t mutex_test_thread_handle;

#if 0
static cyg_mbox test_mboxes[NMBOXES];
static cyg_handle_t test_mbox_handles[NMBOXES];
static fun_times mbox_ft[NMBOXES];
static cyg_thread mbox_test_thread;
static cyg_handle_t mbox_test_thread_handle;
#endif

static sem_t test_semaphores[NSEMAPHORES];
static fun_times semaphore_ft[NSEMAPHORES];
static pthread_t semaphore_test_thread_handle;

static fun_times sched_ft[NSCHEDS];

static timer_t timers[NTIMERS];
static fun_times timer_ft[NTIMERS];

static long rtc_resolution[] = CYGNUM_KERNEL_COUNTERS_RTC_RESOLUTION;
static long ns_per_system_clock;

#if defined(CYGVAR_KERNEL_COUNTERS_CLOCK_LATENCY)
// Data kept by kernel real time clock measuring clock interrupt latency
extern cyg_tick_count total_clock_latency, total_clock_interrupts;
extern cyg_int32 min_clock_latency, max_clock_latency;
extern bool measure_clock_latency;
#endif

#if defined(CYGVAR_KERNEL_COUNTERS_CLOCK_DSR_LATENCY)
extern cyg_tick_count total_clock_dsr_latency, total_clock_dsr_calls;
extern cyg_int32 min_clock_dsr_latency, max_clock_dsr_latency;
extern bool measure_clock_latency;
#endif

//==========================================================================

void run_sched_tests(void);
void run_thread_tests(void);
void run_thread_switch_test(void);
void run_mutex_tests(void);
void run_mutex_circuit_test(void);
void run_mbox_tests(void);
void run_mbox_circuit_test(void);
void run_semaphore_tests(void);
void run_semaphore_circuit_test(void);
void run_timer_tests(void);

//==========================================================================

#ifndef max
#define max(n,m) (m > n ? n : m)
#endif

//==========================================================================
// Wait until a clock tick [real time clock] has passed.  This should keep it
// from happening again during a measurement, thus minimizing any fluctuations
void
wait_for_tick(void)
{
    cyg_tick_count_t tv0, tv1;
    tv0 = cyg_current_time();
    while (true) {
        tv1 = cyg_current_time();
        if (tv1 != tv0) break;
    }
}

//--------------------------------------------------------------------------
// Display a number of ticks as microseconds
// Note: for improved calculation significance, values are kept in ticks*1000
void
show_ticks_in_us(cyg_uint32 ticks)
{
    long long ns;
    ns = (ns_per_system_clock * (long long)ticks) / CYGNUM_KERNEL_COUNTERS_RTC_PERIOD;
    ns += 5;  // for rounding to .01us
    diag_printf("%5d.%02d", (int)(ns/1000), (int)((ns%1000)/10));
}

//--------------------------------------------------------------------------
//
// If the kernel is instrumented to measure clock interrupt latency, these
// measurements can be drastically perturbed by printing via "diag_printf()"
// since that code may run with interrupts disabled for long periods.
//
// In order to get accurate/reasonable latency figures _for the kernel 
// primitive functions beint tested_, the kernel's latency measurements
// are suspended while the printing actually takes place.
//
// The measurements are reenabled after the printing, thus allowing for
// fair measurements of the kernel primitives, which are not distorted
// by the printing mechanisms.

#if defined(CYGVAR_KERNEL_COUNTERS_CLOCK_LATENCY) && defined(HAL_CLOCK_LATENCY)
void
disable_clock_latency_measurement(void)
{
    wait_for_tick();
    measure_clock_latency = false;
}

void
enable_clock_latency_measurement(void)
{
    wait_for_tick();
    measure_clock_latency = true;
}

// Ensure that the measurements are reasonable (no startup anomalies)
void
reset_clock_latency_measurement(void)
{
  disable_clock_latency_measurement();
  total_clock_latency = 0;
  total_clock_interrupts = 0;
  min_clock_latency = 0x7FFFFFFF;
  max_clock_latency = 0;
#if defined(CYGVAR_KERNEL_COUNTERS_CLOCK_DSR_LATENCY)  
  total_clock_dsr_latency = 0;
  total_clock_dsr_calls = 0;
  min_clock_dsr_latency = 0x7FFFFFFF;
  max_clock_dsr_latency = 0;
#endif  
  enable_clock_latency_measurement();
  
}
#else
#define disable_clock_latency_measurement()
#define enable_clock_latency_measurement()
#define reset_clock_latency_measurement()
#endif

//--------------------------------------------------------------------------

void
show_times_hdr(void)
{
    disable_clock_latency_measurement();
    diag_printf("\n");
    diag_printf("                                 Confidence\n");
    diag_printf("     Ave     Min     Max     Var  Ave  Min  Function\n");
    diag_printf("  ======  ======  ======  ====== ========== ========\n");
    enable_clock_latency_measurement();
}

void
show_times_detail(fun_times ft[], int nsamples, char *title, bool ignore_first)
{
    int i, delta, min, max, con_ave, con_min, ave_dev;
    int start_sample, total_samples;   
    cyg_int32 total, ave;

    if (ignore_first) {
        start_sample = 1;
        total_samples = nsamples-1;
    } else {
        start_sample = 0;
        total_samples = nsamples;
    }
    total = 0;
    min = 0x7FFFFFFF;
    max = 0;
    for (i = start_sample;  i < nsamples;  i++) {
        if (ft[i].end < ft[i].start) {
            // Clock wrapped around (timer tick)
            delta = (ft[i].end+CYGNUM_KERNEL_COUNTERS_RTC_PERIOD) - ft[i].start;
        } else {
            delta = ft[i].end - ft[i].start;
        }
        delta -= overhead;
        if (delta < 0) delta = 0;
        delta *= 1000;
        total += delta;
        if (delta < min) min = delta;
        if (delta > max) max = delta;
    }
    ave = total / total_samples;
    total = 0;
    ave_dev = 0;
    for (i = start_sample;  i < nsamples;  i++) {
        if (ft[i].end < ft[i].start) {
            // Clock wrapped around (timer tick)
            delta = (ft[i].end+CYGNUM_KERNEL_COUNTERS_RTC_PERIOD) - ft[i].start;
        } else {
            delta = ft[i].end - ft[i].start;
        }
        delta -= overhead;
        if (delta < 0) delta = 0;
        delta *= 1000;
        delta = delta - ave;
        if (delta < 0) delta = -delta;
        ave_dev += delta;
    }
    ave_dev /= total_samples;
    con_ave = 0;
    con_min = 0;
    for (i = start_sample;  i < nsamples;  i++) {
        if (ft[i].end < ft[i].start) {
            // Clock wrapped around (timer tick)
            delta = (ft[i].end+CYGNUM_KERNEL_COUNTERS_RTC_PERIOD) - ft[i].start;
        } else {
            delta = ft[i].end - ft[i].start;
        }
        delta -= overhead;
        if (delta < 0) delta = 0;
        delta *= 1000;
        if ((delta <= (ave+ave_dev)) && (delta >= (ave-ave_dev))) con_ave++;
        if ((delta <= (min+ave_dev)) && (delta >= (min-ave_dev))) con_min++;
    }
    con_ave = (con_ave * 100) / total_samples;
    con_min = (con_min * 100) / total_samples;
    show_ticks_in_us(ave);
    show_ticks_in_us(min);
    show_ticks_in_us(max);
    show_ticks_in_us(ave_dev);
    disable_clock_latency_measurement();
    diag_printf("  %3d%% %3d%%", con_ave, con_min);
    diag_printf(" %s\n", title);
    enable_clock_latency_measurement();
}

void
show_times(fun_times ft[], int nsamples, char *title)
{
    show_times_detail(ft, nsamples, title, false);
#ifdef STATS_WITHOUT_FIRST_SAMPLE
    show_times_detail(ft, nsamples, "", true);
#endif
}

//--------------------------------------------------------------------------

void
show_test_parameters(void)
{
    disable_clock_latency_measurement();
    diag_printf("\nTesting parameters:\n");
    diag_printf("   Clock samples:         %5d\n", nsamples);
    diag_printf("   Threads:               %5d\n", ntest_threads);
    diag_printf("   Thread switches:       %5d\n", nthread_switches);
    diag_printf("   Mutexes:               %5d\n", nmutexes);
    diag_printf("   Mailboxes:             %5d\n", nmboxes);
    diag_printf("   Semaphores:            %5d\n", nsemaphores);
    diag_printf("   Scheduler operations:  %5d\n", nscheds);
    diag_printf("   Timers:                %5d\n", ntimers);
    diag_printf("\n"); 
    enable_clock_latency_measurement();
}

void
end_of_test_group(void)
{
    disable_clock_latency_measurement();
    diag_printf("\n"); 
    enable_clock_latency_measurement();
}

//--------------------------------------------------------------------------
// Compute a name for a thread

char *
thread_name(char *basename, int indx) {
    return "<<NULL>>";  // Not currently used
}

//--------------------------------------------------------------------------
// test0 - null test, just return

void *
test0(void *indx)
{
    return indx;
}

//--------------------------------------------------------------------------
// test3 - loop, yeilding repeatedly and checking for cancellation

void *
test3(void *indx)
{
    for(;;)
    {
        sched_yield();
        pthread_testcancel();
    }
    
    return indx;
}

//--------------------------------------------------------------------------
// test1 - empty test, simply exit.  Last thread signals parent.

void *
test1( void *indx)
{
    if ((cyg_uint32)indx == (cyg_uint32)(ntest_threads-1)) {
        sem_post(&synchro);  // Signal that last thread is dying
    }
    return indx;
}

//--------------------------------------------------------------------------
// test2 - measure thread switch times

void *
test2(void *indx)
{
    int i;
    for (i = 0;  i < nthread_switches;  i++) {
        if ((int)indx == 0) {
            HAL_CLOCK_READ(&test2_ft[i].start);
        } else {
            HAL_CLOCK_READ(&test2_ft[i].end);
        }
        sched_yield();
    }
    if ((int)indx == 1) {
        sem_post(&synchro);
    }

    return indx;
}

//--------------------------------------------------------------------------
// Full-circuit mutex unlock/lock test

void *
mutex_test(void * indx)
{
    int i;
    pthread_mutex_lock(&test_mutexes[0]);
    for (i = 0;  i < nmutexes;  i++) {
        sem_wait(&synchro);
        wait_for_tick(); // Wait until the next clock tick to minimize aberations
        HAL_CLOCK_READ(&mutex_ft[i].start);
        pthread_mutex_unlock(&test_mutexes[0]);
        pthread_mutex_lock(&test_mutexes[0]);
        sem_post(&synchro);
    }
    return indx;
}

//--------------------------------------------------------------------------
// Full-circuit mbox put/get test

#if 0
void
mbox_test(cyg_uint32 indx)
{
    void *item;
    do {
        item = cyg_mbox_get(test_mbox_handles[0]);
        HAL_CLOCK_READ(&mbox_ft[(int)item].end);
        cyg_semaphore_post(&synchro);
    } while ((int)item != (nmboxes-1));
    cyg_thread_exit(0);
}
#endif

//--------------------------------------------------------------------------
// Full-circuit semaphore post/wait test

void *
semaphore_test(void * indx)
{
    int i;
    for (i = 0;  i < nsemaphores;  i++) {
        sem_wait(&test_semaphores[0]);
        HAL_CLOCK_READ(&semaphore_ft[i].end);
        sem_post(&synchro);
    }
    return indx;
}

//--------------------------------------------------------------------------
//
// This set of tests is used to measure kernel primitives that deal with threads
//

void
run_thread_tests(void)
{

    
    int i;
    struct sched_param schedparam;
    pthread_attr_t attr;
    int policy;
    void *retval;
    
    // Set my priority higher than any I plan to create
    schedparam.sched_priority = 30;
    pthread_setschedparam( pthread_self(), SCHED_RR, &schedparam );

    // Initiaize thread creation attributes

    pthread_attr_init( &attr );
    pthread_attr_setinheritsched( &attr, PTHREAD_EXPLICIT_SCHED );
    pthread_attr_setschedpolicy( &attr, SCHED_RR );
    schedparam.sched_priority = 10;
    pthread_attr_setschedparam( &attr, &schedparam );
    
    
    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < ntest_threads;  i++) {
        HAL_CLOCK_READ(&thread_ft[i].start);

        pthread_attr_setstackaddr( &attr, &stacks[i][STACK_SIZE] );
        pthread_attr_setstacksize( &attr, STACK_SIZE );
        pthread_create( &threads[i],
                        &attr,
                        test0,
                        (void *)i
                        );
        
        HAL_CLOCK_READ(&thread_ft[i].end);
    }
    show_times(thread_ft, ntest_threads, "Create thread");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < ntest_threads;  i++) {
        HAL_CLOCK_READ(&thread_ft[i].start);
        sched_yield();
        HAL_CLOCK_READ(&thread_ft[i].end);
    }
    show_times(thread_ft, ntest_threads, "Yield thread [all lower priority]");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < ntest_threads;  i++) {
        HAL_CLOCK_READ(&thread_ft[i].start);

        schedparam.sched_priority = 11;
        pthread_attr_setschedparam( &attr, &schedparam );
        pthread_setschedparam(threads[i], SCHED_RR, &schedparam);

        HAL_CLOCK_READ(&thread_ft[i].end);
    }
    show_times(thread_ft, ntest_threads, "Set priority");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < ntest_threads;  i++) {
        HAL_CLOCK_READ(&thread_ft[i].start);
        pthread_getschedparam( threads[i], &policy, &schedparam );
        HAL_CLOCK_READ(&thread_ft[i].end);
    }
    show_times(thread_ft, ntest_threads, "Get priority");

    cyg_thread_delay(1);        // Let the test threads run

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < ntest_threads;  i++) {
        HAL_CLOCK_READ(&thread_ft[i].start);
        pthread_join(threads[i], &retval);
        HAL_CLOCK_READ(&thread_ft[i].end);
    }
    show_times(thread_ft, ntest_threads, "Join exited thread");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < ntest_threads;  i++) {
        HAL_CLOCK_READ(&thread_ft[i].start);
        sched_yield();
        HAL_CLOCK_READ(&thread_ft[i].end);
    }
    show_times(thread_ft, ntest_threads, "Yield [no other] thread");

    
    // Recreate the test set

    schedparam.sched_priority = 10;
    pthread_attr_setschedparam( &attr, &schedparam );
    
    for (i = 0;  i < ntest_threads;  i++) {
        pthread_attr_setstackaddr( &attr, &stacks[i][STACK_SIZE] );
        pthread_attr_setstacksize( &attr, STACK_SIZE );
        pthread_create( &threads[i],
                        &attr,
                        test3,
                        (void *)i
                        );
    }

    cyg_thread_delay(1);        // Let the test threads run    
    
    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < ntest_threads;  i++) {
        HAL_CLOCK_READ(&thread_ft[i].start);
        pthread_cancel(threads[i]);
        HAL_CLOCK_READ(&thread_ft[i].end);
    }
    show_times(thread_ft, ntest_threads, "Cancel [running] thread");

    cyg_thread_delay(1);        // Let the test threads do their cancellations
    
    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < ntest_threads;  i++) {
        HAL_CLOCK_READ(&thread_ft[i].start);
        pthread_join(threads[i], &retval);
        HAL_CLOCK_READ(&thread_ft[i].end);
    }
    show_times(thread_ft, ntest_threads, "Join [cancelled] thread");

    
    // Set my priority lower than any I plan to create
    schedparam.sched_priority = 5;
    pthread_setschedparam( pthread_self(), SCHED_RR, &schedparam );
    
    // Set up the end-of-threads synchronizer
    sem_init(&synchro, 0, 0);

    schedparam.sched_priority = 10;
    pthread_attr_setschedparam( &attr, &schedparam );
    
    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < ntest_threads;  i++) {
        HAL_CLOCK_READ(&thread_ft[i].start);

        pthread_attr_setstackaddr( &attr, &stacks[i][STACK_SIZE] );
        pthread_attr_setstacksize( &attr, STACK_SIZE );
        pthread_create( &threads[i],
                        &attr,
                        test2,
                        (void *)i
                        );
        
        HAL_CLOCK_READ(&thread_ft[i].end);
    }
    show_times(thread_ft, ntest_threads, "Create [high priority] thread");

    sem_wait(&synchro);  // Wait for all threads to finish

    // Make sure they are all dead
    for (i = 0;  i < ntest_threads;  i++) {
        pthread_join(threads[i], &retval);
    }

    run_thread_switch_test();
    end_of_test_group();

}

//--------------------------------------------------------------------------

void
run_thread_switch_test(void)
{

    int i;
    struct sched_param schedparam;
    pthread_attr_t attr;
    void *retval;

    // Set my priority higher than any I plan to create
    schedparam.sched_priority = 30;
    pthread_setschedparam( pthread_self(), SCHED_RR, &schedparam );

    // Initiaize thread creation attributes

    pthread_attr_init( &attr );
    pthread_attr_setinheritsched( &attr, PTHREAD_EXPLICIT_SCHED );
    pthread_attr_setschedpolicy( &attr, SCHED_RR );
    schedparam.sched_priority = 10;
    pthread_attr_setschedparam( &attr, &schedparam );
    
    // Set up the end-of-threads synchronizer

    sem_init(&synchro, 0, 0);
        
    // Set up for thread context switch 

    for (i = 0;  i < 2;  i++) {
        pthread_attr_setstackaddr( &attr, &stacks[i][STACK_SIZE] );
        pthread_attr_setstacksize( &attr, STACK_SIZE );
        pthread_create( &threads[i],
                        &attr,
                        test2,
                        (void *)i
                        );
    }

    wait_for_tick(); // Wait until the next clock tick to minimize aberations    
    
    sem_wait(&synchro);

    show_times(test2_ft, nthread_switches, "Thread switch");

    // Clean up
    for (i = 0;  i < 2;  i++) {
        pthread_join(threads[i], &retval);
    }

}


//--------------------------------------------------------------------------

void
run_mutex_tests(void)
{

    int i;
    pthread_mutexattr_t attr;

    pthread_mutexattr_init( &attr );
    
    // Mutex primitives
    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nmutexes;  i++) {
        HAL_CLOCK_READ(&mutex_ft[i].start);
        pthread_mutex_init(&test_mutexes[i], &attr);
        HAL_CLOCK_READ(&mutex_ft[i].end);
    }
    show_times(mutex_ft, nmutexes, "Init mutex");


    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nmutexes;  i++) {
        HAL_CLOCK_READ(&mutex_ft[i].start);
        pthread_mutex_lock(&test_mutexes[i]);
        HAL_CLOCK_READ(&mutex_ft[i].end);
    }
    show_times(mutex_ft, nmutexes, "Lock [unlocked] mutex");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nmutexes;  i++) {
        HAL_CLOCK_READ(&mutex_ft[i].start);
        pthread_mutex_unlock(&test_mutexes[i]);
        HAL_CLOCK_READ(&mutex_ft[i].end);
    }
    show_times(mutex_ft, nmutexes, "Unlock [locked] mutex");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nmutexes;  i++) {
        HAL_CLOCK_READ(&mutex_ft[i].start);
        pthread_mutex_trylock(&test_mutexes[i]);
        HAL_CLOCK_READ(&mutex_ft[i].end);
    }
    show_times(mutex_ft, nmutexes, "Trylock [unlocked] mutex");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nmutexes;  i++) {
        HAL_CLOCK_READ(&mutex_ft[i].start);
        pthread_mutex_trylock(&test_mutexes[i]);
        HAL_CLOCK_READ(&mutex_ft[i].end);
    }
    show_times(mutex_ft, nmutexes, "Trylock [locked] mutex");

    // Must unlock mutices before destroying them.
    for (i = 0;  i < nmutexes;  i++) {
        pthread_mutex_unlock(&test_mutexes[i]);
    }

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nmutexes;  i++) {
        HAL_CLOCK_READ(&mutex_ft[i].start);
        pthread_mutex_destroy(&test_mutexes[i]);
        HAL_CLOCK_READ(&mutex_ft[i].end);
    }
    show_times(mutex_ft, nmutexes, "Destroy mutex");


    run_mutex_circuit_test();
    end_of_test_group();
}

//--------------------------------------------------------------------------

void
run_mutex_circuit_test(void)
{
    int i;
    pthread_mutexattr_t mattr;
    struct sched_param schedparam;
    pthread_attr_t attr;
    void *retval;

    // Set my priority lower than any I plan to create
    schedparam.sched_priority = 5;
    pthread_setschedparam( pthread_self(), SCHED_RR, &schedparam );

    // Initiaize thread creation attributes

    pthread_attr_init( &attr );
    pthread_attr_setinheritsched( &attr, PTHREAD_EXPLICIT_SCHED );
    pthread_attr_setschedpolicy( &attr, SCHED_RR );
    schedparam.sched_priority = 10;
    pthread_attr_setschedparam( &attr, &schedparam );
    
    // Set up for full mutex unlock/lock test
    pthread_mutexattr_init( &mattr );    
    pthread_mutex_init(&test_mutexes[0], &mattr);
    sem_init(&synchro, 0, 0);

    pthread_attr_setstackaddr( &attr, &stacks[0][STACK_SIZE] );
    pthread_attr_setstacksize( &attr, STACK_SIZE );
    pthread_create( &mutex_test_thread_handle,
                    &attr,
                    mutex_test,
                    (void *)0
        );
    
    // Need to raise priority so that this thread will block on the "lock"
    schedparam.sched_priority = 20;
    pthread_setschedparam( pthread_self(), SCHED_RR, &schedparam );
    
    for (i = 0;  i < nmutexes;  i++) {
        sem_post(&synchro);
        pthread_mutex_lock(&test_mutexes[0]);
        HAL_CLOCK_READ(&mutex_ft[i].end);
        pthread_mutex_unlock(&test_mutexes[0]);
        sem_wait(&synchro);
    }
    pthread_join(mutex_test_thread_handle, &retval);
    show_times(mutex_ft, nmutexes, "Unlock/Lock mutex");

}


//--------------------------------------------------------------------------
// Message queue tests

// Currently disabled, pending implementation of POSIX message queues

#if 0
void
run_mbox_tests(void)
{
    int i, cnt;
    void *item;
    // Mailbox primitives
    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nmboxes;  i++) {
        HAL_CLOCK_READ(&mbox_ft[i].start);
        cyg_mbox_create(&test_mbox_handles[i], &test_mboxes[i]);
        HAL_CLOCK_READ(&mbox_ft[i].end);
    }
    show_times(mbox_ft, nmboxes, "Create mbox");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nmboxes;  i++) {
        HAL_CLOCK_READ(&mbox_ft[i].start);
        cnt = cyg_mbox_peek(test_mbox_handles[i]);
        HAL_CLOCK_READ(&mbox_ft[i].end);
    }
    show_times(mbox_ft, nmboxes, "Peek [empty] mbox");

#ifdef CYGMFN_KERNEL_SYNCH_MBOXT_PUT_CAN_WAIT
    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nmboxes;  i++) {
        HAL_CLOCK_READ(&mbox_ft[i].start);
        cyg_mbox_put(test_mbox_handles[i], (void *)i);
        HAL_CLOCK_READ(&mbox_ft[i].end);
    }
    show_times(mbox_ft, nmboxes, "Put [first] mbox");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nmboxes;  i++) {
        HAL_CLOCK_READ(&mbox_ft[i].start);
        cnt = cyg_mbox_peek(test_mbox_handles[i]);
        HAL_CLOCK_READ(&mbox_ft[i].end);
    }
    show_times(mbox_ft, nmboxes, "Peek [1 msg] mbox");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nmboxes;  i++) {
        HAL_CLOCK_READ(&mbox_ft[i].start);
        cyg_mbox_put(test_mbox_handles[i], (void *)i);
        HAL_CLOCK_READ(&mbox_ft[i].end);
    }
    show_times(mbox_ft, nmboxes, "Put [second] mbox");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nmboxes;  i++) {
        HAL_CLOCK_READ(&mbox_ft[i].start);
        cnt = cyg_mbox_peek(test_mbox_handles[i]);
        HAL_CLOCK_READ(&mbox_ft[i].end);
    }
    show_times(mbox_ft, nmboxes, "Peek [2 msgs] mbox");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nmboxes;  i++) {
        HAL_CLOCK_READ(&mbox_ft[i].start);
        item = cyg_mbox_get(test_mbox_handles[i]);
        HAL_CLOCK_READ(&mbox_ft[i].end);
    }
    show_times(mbox_ft, nmboxes, "Get [first] mbox");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nmboxes;  i++) {
        HAL_CLOCK_READ(&mbox_ft[i].start);
        item = cyg_mbox_get(test_mbox_handles[i]);
        HAL_CLOCK_READ(&mbox_ft[i].end);
    }
    show_times(mbox_ft, nmboxes, "Get [second] mbox");
#endif // ifdef CYGMFN_KERNEL_SYNCH_MBOXT_PUT_CAN_WAIT

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nmboxes;  i++) {
        HAL_CLOCK_READ(&mbox_ft[i].start);
        cyg_mbox_tryput(test_mbox_handles[i], (void *)i);
        HAL_CLOCK_READ(&mbox_ft[i].end);
    }
    show_times(mbox_ft, nmboxes, "Tryput [first] mbox");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nmboxes;  i++) {
        HAL_CLOCK_READ(&mbox_ft[i].start);
        item = cyg_mbox_peek_item(test_mbox_handles[i]);
        HAL_CLOCK_READ(&mbox_ft[i].end);
    }
    show_times(mbox_ft, nmboxes, "Peek item [non-empty] mbox");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nmboxes;  i++) {
        HAL_CLOCK_READ(&mbox_ft[i].start);
        item = cyg_mbox_tryget(test_mbox_handles[i]);
        HAL_CLOCK_READ(&mbox_ft[i].end);
    }
    show_times(mbox_ft, nmboxes, "Tryget [non-empty] mbox");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nmboxes;  i++) {
        HAL_CLOCK_READ(&mbox_ft[i].start);
        item = cyg_mbox_peek_item(test_mbox_handles[i]);
        HAL_CLOCK_READ(&mbox_ft[i].end);
    }
    show_times(mbox_ft, nmboxes, "Peek item [empty] mbox");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nmboxes;  i++) {
        HAL_CLOCK_READ(&mbox_ft[i].start);
        item = cyg_mbox_tryget(test_mbox_handles[i]);
        HAL_CLOCK_READ(&mbox_ft[i].end);
    }
    show_times(mbox_ft, nmboxes, "Tryget [empty] mbox");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nmboxes;  i++) {
        HAL_CLOCK_READ(&mbox_ft[i].start);
        cyg_mbox_waiting_to_get(test_mbox_handles[i]);
        HAL_CLOCK_READ(&mbox_ft[i].end);
    }
    show_times(mbox_ft, nmboxes, "Waiting to get mbox");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nmboxes;  i++) {
        HAL_CLOCK_READ(&mbox_ft[i].start);
        cyg_mbox_waiting_to_put(test_mbox_handles[i]);
        HAL_CLOCK_READ(&mbox_ft[i].end);
    }
    show_times(mbox_ft, nmboxes, "Waiting to put mbox");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nmboxes;  i++) {
        HAL_CLOCK_READ(&mbox_ft[i].start);
        cyg_mbox_delete(test_mbox_handles[i]);
        HAL_CLOCK_READ(&mbox_ft[i].end);
    }
    show_times(mbox_ft, nmboxes, "Delete mbox");

    run_mbox_circuit_test();
    end_of_test_group();
}

//--------------------------------------------------------------------------

void
run_mbox_circuit_test(void)
{
#ifdef CYGMFN_KERNEL_SYNCH_MBOXT_PUT_CAN_WAIT
    int i;
    // Set my priority lower than any I plan to create
    cyg_thread_set_priority(cyg_thread_self(), 3);
    // Set up for full mbox put/get test
    cyg_mbox_create(&test_mbox_handles[0], &test_mboxes[0]);
    cyg_semaphore_init(&synchro, 0);
    cyg_thread_create(2,              // Priority - just a number
                      mbox_test,           // entry
                      0,               // index
                      thread_name("thread", 0),     // Name
                      &stacks[0][0],   // Stack
                      STACK_SIZE,      // Size
                      &mbox_test_thread_handle,   // Handle
                      &mbox_test_thread    // Thread data structure
        );
    cyg_thread_resume(mbox_test_thread_handle);
    for (i = 0;  i < nmboxes;  i++) {
        wait_for_tick(); // Wait until the next clock tick to minimize aberations
        HAL_CLOCK_READ(&mbox_ft[i].start);
        cyg_mbox_put(test_mbox_handles[0], (void *)i);
        cyg_semaphore_wait(&synchro);
    }
    cyg_thread_delete(mbox_test_thread_handle);
    show_times(mbox_ft, nmboxes, "Put/Get mbox");
#endif
}

#endif

//--------------------------------------------------------------------------

void
run_semaphore_tests(void)
{

    int i;
    int sem_val;

    // Semaphore primitives
    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nsemaphores;  i++) {
        HAL_CLOCK_READ(&semaphore_ft[i].start);
        sem_init(&test_semaphores[i], 0, 0);
        HAL_CLOCK_READ(&semaphore_ft[i].end);
    }
    show_times(semaphore_ft, nsemaphores, "Init semaphore");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nsemaphores;  i++) {
        HAL_CLOCK_READ(&semaphore_ft[i].start);
        sem_post(&test_semaphores[i]);
        HAL_CLOCK_READ(&semaphore_ft[i].end);
    }
    show_times(semaphore_ft, nsemaphores, "Post [0] semaphore");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nsemaphores;  i++) {
        HAL_CLOCK_READ(&semaphore_ft[i].start);
        sem_wait(&test_semaphores[i]);
        HAL_CLOCK_READ(&semaphore_ft[i].end);
    }
    show_times(semaphore_ft, nsemaphores, "Wait [1] semaphore");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nsemaphores;  i++) {
        HAL_CLOCK_READ(&semaphore_ft[i].start);
        sem_trywait(&test_semaphores[i]);
        HAL_CLOCK_READ(&semaphore_ft[i].end);
    }
    show_times(semaphore_ft, nsemaphores, "Trywait [0] semaphore");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nsemaphores;  i++) {
        sem_post(&test_semaphores[i]);
        HAL_CLOCK_READ(&semaphore_ft[i].start);
        sem_trywait(&test_semaphores[i]);
        HAL_CLOCK_READ(&semaphore_ft[i].end);
    }
    show_times(semaphore_ft, nsemaphores, "Trywait [1] semaphore");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nsemaphores;  i++) {
        HAL_CLOCK_READ(&semaphore_ft[i].start);
        sem_getvalue(&test_semaphores[i], &sem_val);
        HAL_CLOCK_READ(&semaphore_ft[i].end);
    }
    show_times(semaphore_ft, nsemaphores, "Get value of semaphore");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nsemaphores;  i++) {
        HAL_CLOCK_READ(&semaphore_ft[i].start);
        sem_destroy(&test_semaphores[i]);
        HAL_CLOCK_READ(&semaphore_ft[i].end);
    }
    show_times(semaphore_ft, nsemaphores, "Destroy semaphore");

    run_semaphore_circuit_test();
    end_of_test_group();
}

//--------------------------------------------------------------------------

void
run_semaphore_circuit_test(void)
{

    int i;
    struct sched_param schedparam;
    pthread_attr_t attr;
    void *retval;

    // Set my priority lower than any I plan to create
    schedparam.sched_priority = 5;
    pthread_setschedparam( pthread_self(), SCHED_RR, &schedparam );

    // Initiaize thread creation attributes

    pthread_attr_init( &attr );
    pthread_attr_setinheritsched( &attr, PTHREAD_EXPLICIT_SCHED );
    pthread_attr_setschedpolicy( &attr, SCHED_RR );
    schedparam.sched_priority = 10;
    pthread_attr_setschedparam( &attr, &schedparam );
    
    // Set up for full semaphore post/wait test
    sem_init(&test_semaphores[0], 0, 0);
    sem_init(&synchro, 0, 0);

    pthread_attr_setstackaddr( &attr, &stacks[0][STACK_SIZE] );
    pthread_attr_setstacksize( &attr, STACK_SIZE );
    pthread_create( &semaphore_test_thread_handle,
                    &attr,
                    semaphore_test,
                    (void *)0
        );
    
    
    for (i = 0;  i < nsemaphores;  i++) {
        wait_for_tick(); // Wait until the next clock tick to minimize aberations
        HAL_CLOCK_READ(&semaphore_ft[i].start);
        sem_post(&test_semaphores[0]);
        sem_wait(&synchro);
    }
    pthread_join(semaphore_test_thread_handle, &retval);
    
    show_times(semaphore_ft, nsemaphores, "Post/Wait semaphore");


}

//--------------------------------------------------------------------------

// Timer callback function
void
sigrt0(int signo, siginfo_t *info, void *context)
{
    diag_printf("sigrt0 called\n");
    // empty call back
}

// Callback used to test determinancy
static volatile int timer_cnt;
void
sigrt1(int signo, siginfo_t *info, void *context)
{
    if (timer_cnt == nscheds) return;
    sched_ft[timer_cnt].start = 0;
    HAL_CLOCK_READ(&sched_ft[timer_cnt++].end);
    if (timer_cnt == nscheds) {
        sem_post(&synchro);
    }
}

static sem_t timer_sem;

static void
sigrt2(int signo, siginfo_t *info, void *context)
{
    if (timer_cnt == nscheds) {
        sem_post(&synchro);
        sem_post(&timer_sem);        
    } else {
        sched_ft[timer_cnt].start = 0;
        sem_post(&timer_sem);
    }
}

// Null thread, used to keep scheduler busy
void *
timer_test(void * id)
{
    while (true) {
        cyg_thread_yield();
        pthread_testcancel();
    }

    return id;
}

// Thread that suspends itself at the first opportunity
void *
timer_test2(void *id)
{
    while (timer_cnt != nscheds) {
        HAL_CLOCK_READ(&sched_ft[timer_cnt++].end);
        sem_wait(&timer_sem);
    }
    return id;
}

void
run_timer_tests(void)
{
    int res;
    int i;
    struct sigaction sa;
    struct sigevent sigev;
    struct itimerspec tp;
    
    // Install signal handlers
    sigemptyset( &sa.sa_mask );
    sa.sa_flags = SA_SIGINFO;

    sa.sa_sigaction = sigrt0;
    sigaction( SIGRTMIN, &sa, NULL );

    sa.sa_sigaction = sigrt1;
    sigaction( SIGRTMIN+1, &sa, NULL );

    sa.sa_sigaction = sigrt2;
    sigaction( SIGRTMIN+2, &sa, NULL );

    // Set up common bits of sigevent

    sigev.sigev_notify = SIGEV_SIGNAL;

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < ntimers;  i++) {
        HAL_CLOCK_READ(&timer_ft[i].start);
        sigev.sigev_signo = SIGRTMIN;
        sigev.sigev_value.sival_ptr = (void*)(&timers[i]);
        res = timer_create( CLOCK_REALTIME, &sigev, &timers[i]);
        HAL_CLOCK_READ(&timer_ft[i].end);
        CYG_ASSERT( res == 0 , "timer_create() returned error");
    }
    show_times(timer_ft, ntimers, "Create timer");


    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    tp.it_value.tv_sec = 0;
    tp.it_value.tv_nsec = 0;
    tp.it_interval.tv_sec = 0;
    tp.it_interval.tv_nsec = 0;
    for (i = 0;  i < ntimers;  i++) {
        HAL_CLOCK_READ(&timer_ft[i].start);
        res = timer_settime( timers[i], 0, &tp, NULL );
        HAL_CLOCK_READ(&timer_ft[i].end);
        CYG_ASSERT( res == 0 , "timer_settime() returned error");
    }
    show_times(timer_ft, ntimers, "Initialize timer to zero");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    tp.it_value.tv_sec = 1;
    tp.it_value.tv_nsec = 250000000;
    tp.it_interval.tv_sec = 0;
    tp.it_interval.tv_nsec = 0;
    for (i = 0;  i < ntimers;  i++) {
        HAL_CLOCK_READ(&timer_ft[i].start);
        res = timer_settime( timers[i], 0, &tp, NULL );
        HAL_CLOCK_READ(&timer_ft[i].end);
        CYG_ASSERT( res == 0 , "timer_settime() returned error");
    }
    show_times(timer_ft, ntimers, "Initialize timer to 1.25 sec");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    tp.it_value.tv_sec = 0;
    tp.it_value.tv_nsec = 0;
    tp.it_interval.tv_sec = 0;
    tp.it_interval.tv_nsec = 0;
    for (i = 0;  i < ntimers;  i++) {
        HAL_CLOCK_READ(&timer_ft[i].start);
        res = timer_settime( timers[i], 0, &tp, NULL );
        HAL_CLOCK_READ(&timer_ft[i].end);
        CYG_ASSERT( res == 0 , "timer_settime() returned error");
    }
    show_times(timer_ft, ntimers, "Disable timer");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < ntimers;  i++) {
        HAL_CLOCK_READ(&timer_ft[i].start);
        res = timer_delete( timers[i] );
        HAL_CLOCK_READ(&timer_ft[i].end);
        CYG_ASSERT( res == 0 , "timer_settime() returned error");
    }
    show_times(timer_ft, ntimers, "Delete timer");
    
    

    sigev.sigev_signo = SIGRTMIN+1;
    sigev.sigev_value.sival_ptr = (void*)(&timers[i]);
    res = timer_create( CLOCK_REALTIME, &sigev, &timers[0]);
    CYG_ASSERT( res == 0 , "timer_create() returned error");
    tp.it_value.tv_sec = 0;
    tp.it_value.tv_nsec = 50000000;
    tp.it_interval.tv_sec = 0;
    tp.it_interval.tv_nsec = 50000000;;
    timer_cnt = 0;
    res = timer_settime( timers[0], 0, &tp, NULL );
    CYG_ASSERT( res == 0 , "timer_settime() returned error");
    sem_init(&synchro, 0, 0);
    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    do
    { res = sem_wait(&synchro);
    } while( res == -1 && errno == EINTR );
    CYG_ASSERT( res == 0 , "sem_wait() returned error");        
    tp.it_value.tv_sec = 0;
    tp.it_value.tv_nsec = 0;
    tp.it_interval.tv_sec = 0;
    tp.it_interval.tv_nsec = 0;
    res = timer_settime( timers[0], 0, &tp, NULL );
    CYG_ASSERT( res == 0 , "timer_settime() returned error");
    res = timer_delete( timers[0] );
    CYG_ASSERT( res == 0 , "timer_delete() returned error");
    show_times(sched_ft, nscheds, "Timer latency [0 threads]");


    

    struct sched_param schedparam;
    pthread_attr_t attr;
    void *retval;
    
    // Set my priority higher than any I plan to create
    schedparam.sched_priority = 20;
    pthread_setschedparam( pthread_self(), SCHED_RR, &schedparam );

    
    // Initiaize thread creation attributes

    pthread_attr_init( &attr );
    pthread_attr_setinheritsched( &attr, PTHREAD_EXPLICIT_SCHED );
    pthread_attr_setschedpolicy( &attr, SCHED_RR );
    schedparam.sched_priority = 10;
    pthread_attr_setschedparam( &attr, &schedparam );
    
    for (i = 0;  i < 2;  i++) {
        pthread_attr_setstackaddr( &attr, &stacks[i][STACK_SIZE] );
        pthread_attr_setstacksize( &attr, STACK_SIZE );
        res = pthread_create( &threads[i],
                        &attr,
                        timer_test,
                        (void *)i
                        );
        CYG_ASSERT( res == 0 , "pthread_create() returned error");
    }

    wait_for_tick(); // Wait until the next clock tick to minimize aberations

    sigev.sigev_signo = SIGRTMIN+1;
    sigev.sigev_value.sival_ptr = (void*)(&timers[i]);
    res = timer_create( CLOCK_REALTIME, &sigev, &timers[0]);
    CYG_ASSERT( res == 0 , "timer_create() returned error");
    tp.it_value.tv_sec = 0;
    tp.it_value.tv_nsec = 50000000;
    tp.it_interval.tv_sec = 0;
    tp.it_interval.tv_nsec = 50000000;;
    timer_cnt = 0;
    res = timer_settime( timers[0], 0, &tp, NULL );
    CYG_ASSERT( res == 0 , "timer_settime() returned error");
    
    sem_init(&synchro, 0, 0);
    do
    { res = sem_wait(&synchro);
    } while( res == -1 && errno == EINTR );
    CYG_ASSERT( res == 0 , "sem_wait() returned error");        
    res = timer_delete(timers[0]);
    CYG_ASSERT( res == 0 , "timerdelete() returned error");    
    show_times(sched_ft, nscheds, "Timer latency [2 threads]");
    for (i = 0;  i < 2;  i++) {
        pthread_cancel(threads[i]);
        pthread_join(threads[i], &retval);
    }


    
    for (i = 0;  i < ntest_threads;  i++) {
        pthread_attr_setstackaddr( &attr, &stacks[i][STACK_SIZE] );
        pthread_attr_setstacksize( &attr, STACK_SIZE );
        res = pthread_create( &threads[i],
                        &attr,
                        timer_test,
                        (void *)i
                        );
        CYG_ASSERT( res == 0 , "pthread_create() returned error");
    }
    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    sigev.sigev_signo = SIGRTMIN+1;
    sigev.sigev_value.sival_ptr = (void*)(&timers[i]);
    res = timer_create( CLOCK_REALTIME, &sigev, &timers[0]);
    CYG_ASSERT( res == 0 , "timer_create() returned error");
    tp.it_value.tv_sec = 0;
    tp.it_value.tv_nsec = 50000000;
    tp.it_interval.tv_sec = 0;
    tp.it_interval.tv_nsec = 50000000;;
    timer_cnt = 0;
    res = timer_settime( timers[0], 0, &tp, NULL );
    CYG_ASSERT( res == 0 , "timer_settime() returned error");
    
    sem_init(&synchro, 0, 0);
    do
    { res = sem_wait(&synchro);
    } while( res == -1 && errno == EINTR );
    CYG_ASSERT( res == 0 , "sem_wait() returned error");        
    res = timer_delete(timers[0]);
    CYG_ASSERT( res == 0 , "timerdelete() returned error");        
    show_times(sched_ft, nscheds, "Timer latency [many threads]");
    for (i = 0;  i < ntest_threads;  i++) {
        pthread_cancel(threads[i]);
        pthread_join(threads[i], &retval);
    }

    sem_init(&synchro, 0, 0);
    sem_init(&timer_sem, 0, 0);    
    pthread_attr_setstackaddr( &attr, &stacks[0][STACK_SIZE] );
    pthread_attr_setstacksize( &attr, STACK_SIZE );
    res = pthread_create( &threads[0],
                          &attr,
                          timer_test2,
                          (void *)0
        );
    CYG_ASSERT( res == 0 , "pthread_create() returned error");
    
    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    sigev.sigev_signo = SIGRTMIN+2;
    sigev.sigev_value.sival_ptr = (void*)(threads[0]);
    res = timer_create( CLOCK_REALTIME, &sigev, &timers[0]);
    CYG_ASSERT( res == 0 , "timer_create() returned error");
    tp.it_value.tv_sec = 0;
    tp.it_value.tv_nsec = 50000000;
    tp.it_interval.tv_sec = 0;
    tp.it_interval.tv_nsec = 50000000;;
    timer_cnt = 0;
    res = timer_settime( timers[0], 0, &tp, NULL );
    CYG_ASSERT( res == 0 , "timer_settime() returned error");

    do
    { res = sem_wait(&synchro);
    } while( res == -1 && errno == EINTR );
    CYG_ASSERT( res == 0 , "sem_wait() returned error");        
    res = timer_delete(timers[0]);
    CYG_ASSERT( res == 0 , "timerdelete() returned error");        
    show_times(sched_ft, nscheds, "Timer -> thread post latency");
    sem_post(&timer_sem);
//    pthread_cancel(threads[0]);
    pthread_join(threads[0], &retval);


    end_of_test_group();
}


//--------------------------------------------------------------------------

void 
run_all_tests()
{
    int i;
    cyg_uint32 tv[nsamples], tv0, tv1;
//    cyg_uint32 min_stack, max_stack, total_stack, actual_stack, j;
    cyg_tick_count_t ticks, tick0, tick1;
#ifdef CYG_SCHEDULER_LOCK_TIMINGS
    cyg_uint32 lock_ave, lock_max;
#endif
#if defined(CYGVAR_KERNEL_COUNTERS_CLOCK_LATENCY) && defined(HAL_CLOCK_LATENCY)
    cyg_int32 clock_ave;
#endif

    disable_clock_latency_measurement();

//    cyg_test_dump_thread_stack_stats( "Startup, main stack", thread[0] );
    cyg_test_dump_interrupt_stack_stats( "Startup" );
    cyg_test_dump_idlethread_stack_stats( "Startup" );
    cyg_test_clear_interrupt_stack();

    diag_printf("\neCos Kernel Timings\n");
    diag_printf("Notes: all times are in microseconds (.000001) unless otherwise stated\n");
#ifdef STATS_WITHOUT_FIRST_SAMPLE
    diag_printf("       second line of results have first sample removed\n");
#endif

    cyg_thread_delay(2);  // Make sure the clock is actually running

    ns_per_system_clock = 1000000/rtc_resolution[1];

    for (i = 0;  i < nsamples;  i++) {
        HAL_CLOCK_READ(&tv[i]);
    }
    tv0 = 0;
    for (i = 1;  i < nsamples;  i++) {
        tv0 += tv[i] - tv[i-1];
    }
    end_of_test_group();
    
    overhead = tv0 / (nsamples-1);
    diag_printf("Reading the hardware clock takes %d 'ticks' overhead\n", overhead);
    diag_printf("... this value will be factored out of all other measurements\n");

    // Try and measure how long the clock interrupt handling takes
    for (i = 0;  i < nsamples;  i++) {
        tick0 = cyg_current_time();
        while (true) {
            tick1 = cyg_current_time();
            if (tick0 != tick1) break;
        }
        HAL_CLOCK_READ(&tv[i]);
    }
    tv1 = 0;
    for (i = 0;  i < nsamples;  i++) {
        tv1 += tv[i] * 1000;
    }
    tv1 = tv1 / nsamples;
    tv1 -= overhead;  // Adjust out the cost of getting the timer value
    diag_printf("Clock interrupt took");
    show_ticks_in_us(tv1);
    diag_printf(" microseconds (%d raw clock ticks)\n", tv1/1000);
    enable_clock_latency_measurement();

    ticks = cyg_current_time();

    show_test_parameters();
    show_times_hdr();

    reset_clock_latency_measurement();

    run_thread_tests();
    run_mutex_tests();
//    run_mbox_tests();
    run_semaphore_tests();
    run_timer_tests();

#ifdef CYG_SCHEDULER_LOCK_TIMINGS
    Cyg_Scheduler::get_lock_times(&lock_ave, &lock_max);
    diag_printf("\nMax lock:");
    show_ticks_in_us(lock_max);
    diag_printf(", Ave lock:");
    show_ticks_in_us(lock_ave);
    diag_printf("\n");
#endif

#if defined(CYGVAR_KERNEL_COUNTERS_CLOCK_LATENCY) && defined(HAL_CLOCK_LATENCY)
    // Display latency figures in same format as all other numbers
    disable_clock_latency_measurement();
    clock_ave = (total_clock_latency*1000) / total_clock_interrupts;
    show_ticks_in_us(clock_ave);
    show_ticks_in_us(min_clock_latency*1000);
    show_ticks_in_us(max_clock_latency*1000);
    show_ticks_in_us(0);
    diag_printf("            Clock/interrupt latency\n\n");
    enable_clock_latency_measurement();    
#endif

#if defined(CYGVAR_KERNEL_COUNTERS_CLOCK_DSR_LATENCY)
    disable_clock_latency_measurement();    
    clock_ave = (total_clock_dsr_latency*1000) / total_clock_dsr_calls;
    show_ticks_in_us(clock_ave);
    show_ticks_in_us(min_clock_dsr_latency*1000);
    show_ticks_in_us(max_clock_dsr_latency*1000);
    show_ticks_in_us(0);
    diag_printf("            Clock DSR latency\n\n");
    enable_clock_latency_measurement();
#endif

#if 0    
    disable_clock_latency_measurement();
    min_stack = STACK_SIZE;
    max_stack = 0;
    total_stack = 0;
    for (i = 0;  i < (int)NTEST_THREADS;  i++) {
        for (j = 0;  j < STACK_SIZE;  j++) {
            if (stacks[i][j]) break;
        }
        actual_stack = STACK_SIZE-j;
        if (actual_stack < min_stack) min_stack = actual_stack;
        if (actual_stack > max_stack) max_stack = actual_stack;
        total_stack += actual_stack;
    }
    for (j = 0;  j < STACKSIZE;  j++) {
        if (((char *)stack[0])[j]) break;
    }
    diag_printf("%5d   %5d   %5d  (main stack: %5d)  Thread stack used (%d total)\n", 
                total_stack/NTEST_THREADS, min_stack, max_stack, 
                STACKSIZE - j, STACK_SIZE);
#endif
    
//    cyg_test_dump_thread_stack_stats( "All done, main stack", thread[0] );
    cyg_test_dump_interrupt_stack_stats( "All done" );
    cyg_test_dump_idlethread_stack_stats( "All done" );

    enable_clock_latency_measurement();

    ticks = cyg_current_time();
    diag_printf("\nTiming complete - %d ms total\n\n", (int)((ticks*ns_per_system_clock)/1000));

    CYG_TEST_PASS_FINISH("Basic timing OK");
}

int main( int argc, char **argv )
{
    CYG_TEST_INIT();

    if (cyg_test_is_simulator) {
        nsamples = NSAMPLES_SIM;
        ntest_threads = NTEST_THREADS_SIM;
        nthread_switches = NTHREAD_SWITCHES_SIM;
        nmutexes = NMUTEXES_SIM;
        nmboxes = NMBOXES_SIM;
        nsemaphores = NSEMAPHORES_SIM;
        nscheds = NSCHEDS_SIM;
        ntimers = NTIMERS_SIM;  
    } else {
        nsamples = NSAMPLES;
        ntest_threads = NTEST_THREADS;
        nthread_switches = NTHREAD_SWITCHES;
        nmutexes = NMUTEXES; 
        nmboxes = NMBOXES;
        nsemaphores = NSEMAPHORES;
        nscheds = NSCHEDS;
        ntimers = NTIMERS;
    }

    // Sanity
#ifdef WORKHORSE_TEST
    ntest_threads = max(512, ntest_threads);
    nmutexes = max(1024, nmutexes);
    nsemaphores = max(1024, nsemaphores);
    nmboxes = max(1024, nmboxes);
    ncounters = max(1024, ncounters);
    ntimers = max(1024, ntimers);
#else
    ntest_threads = max(64, ntest_threads);
    nmutexes = max(32, nmutexes);
    nsemaphores = max(32, nsemaphores);
    nmboxes = max(32, nmboxes);
    ntimers = max(32, ntimers);
#endif

    run_all_tests();
   
}

#endif // CYGFUN_KERNEL_API_C, etc.

// EOF tm_basic.cxx
