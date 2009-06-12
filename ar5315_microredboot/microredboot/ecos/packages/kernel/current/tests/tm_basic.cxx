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
// Copyright (C) 2002, 2003 Gary Thomas
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
// Author(s):     gthomas
// Contributors:  gthomas
// Date:          1998-10-19
// Description:   Very simple kernel timing test
//####DESCRIPTIONEND####

#include <pkgconf/kernel.h>
#include <pkgconf/hal.h>

#include <cyg/kernel/sched.hxx>
#include <cyg/kernel/thread.hxx>
#include <cyg/kernel/thread.inl>
#include <cyg/kernel/mutex.hxx>
#include <cyg/kernel/sema.hxx>
#include <cyg/kernel/flag.hxx>
#include <cyg/kernel/sched.inl>
#include <cyg/kernel/clock.hxx>
#include <cyg/kernel/clock.inl>
#include <cyg/kernel/kapi.h>

#include <cyg/infra/testcase.h>
#include <cyg/infra/diag.h>

#include <cyg/kernel/test/stackmon.h>
#include CYGHWR_MEMORY_LAYOUT_H

// Define this to see the statistics with the first sample datum removed.
// This can expose the effects of caches on the speed of operations.
#undef STATS_WITHOUT_FIRST_SAMPLE

#if defined(CYGFUN_KERNEL_API_C) &&             \
    defined(CYGSEM_KERNEL_SCHED_MLQUEUE) &&     \
    defined(CYGVAR_KERNEL_COUNTERS_CLOCK) &&    \
    !defined(CYGDBG_INFRA_DIAG_USE_DEVICE) &&   \
    (CYGNUM_KERNEL_SCHED_PRIORITIES > 12)

#define NTHREADS 1
#include "testaux.hxx"

// Structure used to keep track of times
typedef struct fun_times {
    cyg_uint32 start;
    cyg_uint32 end;
} fun_times;

#define STACK_SIZE CYGNUM_HAL_STACK_SIZE_MINIMUM

#ifdef CYGMEM_REGION_ram_SIZE
#define CYG_THREAD_OVERHEAD  (STACK_SIZE+sizeof(cyg_thread)+(sizeof(fun_times)*2))
#define NTEST_THREADS        ((CYGMEM_REGION_ram_SIZE/16)/CYG_THREAD_OVERHEAD)
#define CYG_MUTEX_OVERHEAD   (sizeof(cyg_mutex_t)+sizeof(fun_times))
#define NMUTEXES             ((CYGMEM_REGION_ram_SIZE/16)/CYG_MUTEX_OVERHEAD)
#define CYG_MBOX_OVERHEAD    (sizeof(cyg_mbox)+sizeof(fun_times))
#define NMBOXES              ((CYGMEM_REGION_ram_SIZE/24)/CYG_MBOX_OVERHEAD)
#define CYG_SEMAPHORE_OVERHEAD (sizeof(cyg_sem_t)+sizeof(fun_times))
#define NSEMAPHORES          ((CYGMEM_REGION_ram_SIZE/16)/CYG_SEMAPHORE_OVERHEAD)
#define CYG_COUNTER_OVERHEAD (sizeof(cyg_counter)+sizeof(fun_times))
#define NCOUNTERS            ((CYGMEM_REGION_ram_SIZE/24)/CYG_COUNTER_OVERHEAD)
#define CYG_FLAG_OVERHEAD    (sizeof(cyg_flag_t)+sizeof(fun_times))
#define NFLAGS               ((CYGMEM_REGION_ram_SIZE/24)/CYG_FLAG_OVERHEAD)
#define CYG_ALARM_OVERHEAD   (sizeof(cyg_alarm)+sizeof(fun_times))
#define NALARMS              ((CYGMEM_REGION_ram_SIZE/16)/CYG_ALARM_OVERHEAD)
#else
// Defaults
#define NTEST_THREADS    16
#define NMUTEXES         32
#define NMBOXES          32
#define NSEMAPHORES      32
#define NFLAGS           32
#define NCOUNTERS        32
#define NALARMS          32
#endif

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
#define NFLAGS_SIM           2
#define NCOUNTERS_SIM        2
#define NALARMS_SIM          2

static int nsamples;
static int ntest_threads;
static int nthread_switches;
static int nmutexes;
static int nmboxes;
static int nsemaphores;
static int nscheds;
static int nflags;
static int ncounters;
static int nalarms;

static char stacks[NTEST_THREADS][STACK_SIZE];
static cyg_thread test_threads[NTEST_THREADS];
static cyg_handle_t threads[NTEST_THREADS];
static int overhead;
static cyg_sem_t synchro;
static fun_times thread_ft[NTEST_THREADS];

static fun_times test2_ft[NTHREAD_SWITCHES];

static cyg_mutex_t test_mutexes[NMUTEXES];
static fun_times mutex_ft[NMUTEXES];
static cyg_thread mutex_test_thread;
static cyg_handle_t mutex_test_thread_handle;

static cyg_mbox test_mboxes[NMBOXES];
static cyg_handle_t test_mbox_handles[NMBOXES];
static fun_times mbox_ft[NMBOXES];
#ifdef CYGMFN_KERNEL_SYNCH_MBOXT_PUT_CAN_WAIT
static cyg_thread mbox_test_thread;
static cyg_handle_t mbox_test_thread_handle;
#endif

static cyg_sem_t test_semaphores[NSEMAPHORES];
static fun_times semaphore_ft[NSEMAPHORES];
static cyg_thread semaphore_test_thread;
static cyg_handle_t semaphore_test_thread_handle;

static fun_times sched_ft[NSCHEDS];

static cyg_counter test_counters[NCOUNTERS];
static cyg_handle_t counters[NCOUNTERS];
static fun_times counter_ft[NCOUNTERS];

static cyg_flag_t test_flags[NFLAGS];
static fun_times flag_ft[NFLAGS];

static cyg_alarm test_alarms[NALARMS];
static cyg_handle_t alarms[NALARMS];
static fun_times alarm_ft[NALARMS];

static long rtc_resolution[] = CYGNUM_KERNEL_COUNTERS_RTC_RESOLUTION;
static long ns_per_system_clock;

#if defined(CYGVAR_KERNEL_COUNTERS_CLOCK_LATENCY)  && defined(HAL_CLOCK_LATENCY)
// Data kept by kernel real time clock measuring clock interrupt latency
extern cyg_tick_count total_clock_latency, total_clock_interrupts;
extern cyg_int32 min_clock_latency, max_clock_latency;
extern bool measure_clock_latency;
#endif

#if defined(CYGVAR_KERNEL_COUNTERS_CLOCK_DSR_LATENCY) && defined(HAL_CLOCK_LATENCY)
extern cyg_tick_count total_clock_dsr_latency, total_clock_dsr_calls;
extern cyg_int32 min_clock_dsr_latency, max_clock_dsr_latency;
extern bool measure_clock_latency;
#endif

void run_sched_tests(void);
void run_thread_tests(void);
void run_thread_switch_test(void);
void run_mutex_tests(void);
void run_mutex_circuit_test(void);
void run_mbox_tests(void);
void run_mbox_circuit_test(void);
void run_semaphore_tests(void);
void run_semaphore_circuit_test(void);
void run_counter_tests(void);
void run_flag_tests(void);
void run_alarm_tests(void);

#ifndef max
#define max(n,m) (m > n ? n : m)
#endif

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
#if defined(CYGVAR_KERNEL_COUNTERS_CLOCK_DSR_LATENCY) && defined(HAL_CLOCK_LATENCY)
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
    diag_printf("   Counters:              %5d\n", ncounters);
    diag_printf("   Flags:                 %5d\n", nflags);
    diag_printf("   Alarms:                %5d\n", nalarms);
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

// Compute a name for a thread
char *
thread_name(char *basename, int indx) {
    return "<<NULL>>";  // Not currently used
}

// test0 - null test, never executed
void
test0(cyg_uint32 indx)
{
#ifndef CYGPKG_KERNEL_SMP_SUPPORT
    // In SMP, somw of these threads will execute
    diag_printf("test0.%d executed?\n", indx);
#endif    
    cyg_thread_exit();
}

// test1 - empty test, simply exit.  Last thread signals parent.
void
test1(cyg_uint32 indx)
{
    if (indx == (cyg_uint32)(ntest_threads-1)) {
        cyg_semaphore_post(&synchro);  // Signal that last thread is dying
    }
    cyg_thread_exit();
}

// test2 - measure thread switch times
void
test2(cyg_uint32 indx)
{
    int i;
    for (i = 0;  i < nthread_switches;  i++) {
        if (indx == 0) {
            HAL_CLOCK_READ(&test2_ft[i].start);
        } else {
            HAL_CLOCK_READ(&test2_ft[i].end);
        }
        cyg_thread_yield();
    }
    if (indx == 1) {
        cyg_semaphore_post(&synchro);
    }
    cyg_thread_exit();
}

// Full-circuit mutex unlock/lock test
void
mutex_test(cyg_uint32 indx)
{
    int i;
    cyg_mutex_lock(&test_mutexes[0]);
    for (i = 0;  i < nmutexes;  i++) {
        cyg_semaphore_wait(&synchro);
        wait_for_tick(); // Wait until the next clock tick to minimize aberations
        HAL_CLOCK_READ(&mutex_ft[i].start);
        cyg_mutex_unlock(&test_mutexes[0]);
        cyg_mutex_lock(&test_mutexes[0]);
        cyg_semaphore_post(&synchro);
    }
    cyg_thread_exit();
}

// Full-circuit mbox put/get test
void
mbox_test(cyg_uint32 indx)
{
    void *item;
    do {
        item = cyg_mbox_get(test_mbox_handles[0]);
        HAL_CLOCK_READ(&mbox_ft[(int)item].end);
        cyg_semaphore_post(&synchro);
    } while ((int)item != (nmboxes-1));
    cyg_thread_exit();
}

// Full-circuit semaphore post/wait test
void
semaphore_test(cyg_uint32 indx)
{
    int i;
    for (i = 0;  i < nsemaphores;  i++) {
        cyg_semaphore_wait(&test_semaphores[0]);
        HAL_CLOCK_READ(&semaphore_ft[i].end);
        cyg_semaphore_post(&synchro);
    }
    cyg_thread_exit();
}

//
// This set of tests is used to measure kernel primitives that deal with threads
//
void
run_thread_tests(void)
{
    int i;
    cyg_priority_t prio;

    // Set my priority higher than any I plan to create
    cyg_thread_set_priority(cyg_thread_self(), 2);

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < ntest_threads;  i++) {
        HAL_CLOCK_READ(&thread_ft[i].start);
        cyg_thread_create(10,              // Priority - just a number
                          test0,           // entry
                          i,               // index
                          thread_name("thread", i),     // Name
                          &stacks[i][0],   // Stack
                          STACK_SIZE,      // Size
                          &threads[i],     // Handle
                          &test_threads[i] // Thread data structure
            );
        HAL_CLOCK_READ(&thread_ft[i].end);
    }
    show_times(thread_ft, ntest_threads, "Create thread");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < ntest_threads;  i++) {
        HAL_CLOCK_READ(&thread_ft[i].start);
        cyg_thread_yield();
        HAL_CLOCK_READ(&thread_ft[i].end);
    }
    show_times(thread_ft, ntest_threads, "Yield thread [all suspended]");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < ntest_threads;  i++) {
        HAL_CLOCK_READ(&thread_ft[i].start);
        cyg_thread_suspend(threads[i]);
        HAL_CLOCK_READ(&thread_ft[i].end);
    }
    show_times(thread_ft, ntest_threads, "Suspend [suspended] thread");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < ntest_threads;  i++) {
        HAL_CLOCK_READ(&thread_ft[i].start);
        cyg_thread_resume(threads[i]);
        HAL_CLOCK_READ(&thread_ft[i].end);
    }
    show_times(thread_ft, ntest_threads, "Resume thread");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < ntest_threads;  i++) {
        HAL_CLOCK_READ(&thread_ft[i].start);
        cyg_thread_set_priority(threads[i], 11);
        HAL_CLOCK_READ(&thread_ft[i].end);
    }
    show_times(thread_ft, ntest_threads, "Set priority");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < ntest_threads;  i++) {
        HAL_CLOCK_READ(&thread_ft[i].start);
        prio = cyg_thread_get_priority(threads[i]);
        HAL_CLOCK_READ(&thread_ft[i].end);
    }
    show_times(thread_ft, ntest_threads, "Get priority");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < ntest_threads;  i++) {
        HAL_CLOCK_READ(&thread_ft[i].start);
        cyg_thread_kill(threads[i]);
        HAL_CLOCK_READ(&thread_ft[i].end);
    }
    show_times(thread_ft, ntest_threads, "Kill [suspended] thread");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < ntest_threads;  i++) {
        HAL_CLOCK_READ(&thread_ft[i].start);
        cyg_thread_yield();
        HAL_CLOCK_READ(&thread_ft[i].end);
    }
    show_times(thread_ft, ntest_threads, "Yield [no other] thread");

    // Set my priority higher than any I plan to create
    cyg_thread_set_priority(cyg_thread_self(), 2);

    // Recreate the test set
    for (i = 0;  i < ntest_threads;  i++) {
        cyg_thread_create(10,              // Priority - just a number
                          test0,           // entry
                          i,               // index
                          thread_name("thread", i),     // Name
                          &stacks[i][0],   // Stack
                          STACK_SIZE,      // Size
                          &threads[i],     // Handle
                          &test_threads[i] // Thread data structure
            );
    }

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < ntest_threads;  i++) {
        HAL_CLOCK_READ(&thread_ft[i].start);
        cyg_thread_resume(threads[i]);
        HAL_CLOCK_READ(&thread_ft[i].end);
    }
    show_times(thread_ft, ntest_threads, "Resume [suspended low prio] thread");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < ntest_threads;  i++) {
        HAL_CLOCK_READ(&thread_ft[i].start);
        cyg_thread_resume(threads[i]);
        HAL_CLOCK_READ(&thread_ft[i].end);
    }
    show_times(thread_ft, ntest_threads, "Resume [runnable low prio] thread");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < ntest_threads;  i++) {
        HAL_CLOCK_READ(&thread_ft[i].start);
        cyg_thread_suspend(threads[i]);
        HAL_CLOCK_READ(&thread_ft[i].end);
    }
    show_times(thread_ft, ntest_threads, "Suspend [runnable] thread");


    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < ntest_threads;  i++) {
        HAL_CLOCK_READ(&thread_ft[i].start);
        cyg_thread_yield();
        HAL_CLOCK_READ(&thread_ft[i].end);
    }
    show_times(thread_ft, ntest_threads, "Yield [only low prio] thread");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < ntest_threads;  i++) {
        HAL_CLOCK_READ(&thread_ft[i].start);
        cyg_thread_suspend(threads[i]);
        HAL_CLOCK_READ(&thread_ft[i].end);
    }
    show_times(thread_ft, ntest_threads, "Suspend [runnable->not runnable]");
    for (i = 0;  i < ntest_threads;  i++) {
        cyg_thread_resume(threads[i]);
    }

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < ntest_threads;  i++) {
        HAL_CLOCK_READ(&thread_ft[i].start);
        cyg_thread_kill(threads[i]);
        HAL_CLOCK_READ(&thread_ft[i].end);
    }
    show_times(thread_ft, ntest_threads, "Kill [runnable] thread");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < ntest_threads;  i++) {
        HAL_CLOCK_READ(&thread_ft[i].start);
        cyg_thread_delete(threads[i]);
        HAL_CLOCK_READ(&thread_ft[i].end);
    }
    show_times(thread_ft, ntest_threads, "Destroy [dead] thread");

    // Recreate the test set
    for (i = 0;  i < ntest_threads;  i++) {
        cyg_thread_create(10,              // Priority - just a number
                          test0,           // entry
                          i,               // index
                          thread_name("thread", i),     // Name
                          &stacks[i][0],   // Stack
                          STACK_SIZE,      // Size
                          &threads[i],     // Handle
                          &test_threads[i] // Thread data structure
            );
        cyg_thread_resume(threads[i]);
    }

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < ntest_threads;  i++) {
        HAL_CLOCK_READ(&thread_ft[i].start);
        cyg_thread_delete(threads[i]);
        HAL_CLOCK_READ(&thread_ft[i].end);
    }
    show_times(thread_ft, ntest_threads, "Destroy [runnable] thread");

    // Set my priority lower than any I plan to create
    cyg_thread_set_priority(cyg_thread_self(), 3);

    // Set up the end-of-threads synchronizer
    cyg_semaphore_init(&synchro, 0);

    // Recreate the test set
    for (i = 0;  i < ntest_threads;  i++) {
        cyg_thread_create(2,               // Priority - just a number
                          test1,           // entry
                          i,               // index
                          thread_name("thread", i),     // Name
                          &stacks[i][0],   // Stack
                          STACK_SIZE,      // Size
                          &threads[i],     // Handle
                          &test_threads[i] // Thread data structure
            );
    }

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < ntest_threads;  i++) {
        HAL_CLOCK_READ(&thread_ft[i].start);
        cyg_thread_resume(threads[i]);
        HAL_CLOCK_READ(&thread_ft[i].end);
    }
    show_times(thread_ft, ntest_threads, "Resume [high priority] thread");
    cyg_semaphore_wait(&synchro);  // Wait for all threads to finish
    // Make sure they are all dead
    for (i = 0;  i < ntest_threads;  i++) {
        cyg_thread_delete(threads[i]);
    }

    run_thread_switch_test();
    end_of_test_group();
}

void
run_thread_switch_test(void)
{
    int i;

    // Set up for thread context switch 
    for (i = 0;  i < 2;  i++) {
        cyg_thread_create(10,              // Priority - just a number
                          test2,           // entry
                          i,               // index
                          thread_name("thread", i),     // Name
                          &stacks[i][0],   // Stack
                          STACK_SIZE,      // Size
                          &threads[i],     // Handle
                          &test_threads[i] // Thread data structure
            );
        cyg_thread_resume(threads[i]);
    }
    // Set up the end-of-threads synchronizer
    cyg_semaphore_init(&synchro, 0);
    cyg_semaphore_wait(&synchro);
    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    show_times(test2_ft, nthread_switches, "Thread switch");
    // Clean up
    for (i = 0;  i < 2;  i++) {
        cyg_thread_delete(threads[i]);
    }
}

void
run_mutex_tests(void)
{
    int i;

    // Mutex primitives
    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nmutexes;  i++) {
        HAL_CLOCK_READ(&mutex_ft[i].start);
        cyg_mutex_init(&test_mutexes[i]);
        HAL_CLOCK_READ(&mutex_ft[i].end);
    }
    show_times(mutex_ft, nmutexes, "Init mutex");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nmutexes;  i++) {
        HAL_CLOCK_READ(&mutex_ft[i].start);
        cyg_mutex_lock(&test_mutexes[i]);
        HAL_CLOCK_READ(&mutex_ft[i].end);
    }
    show_times(mutex_ft, nmutexes, "Lock [unlocked] mutex");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nmutexes;  i++) {
        HAL_CLOCK_READ(&mutex_ft[i].start);
        cyg_mutex_unlock(&test_mutexes[i]);
        HAL_CLOCK_READ(&mutex_ft[i].end);
    }
    show_times(mutex_ft, nmutexes, "Unlock [locked] mutex");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nmutexes;  i++) {
        HAL_CLOCK_READ(&mutex_ft[i].start);
        cyg_mutex_trylock(&test_mutexes[i]);
        HAL_CLOCK_READ(&mutex_ft[i].end);
    }
    show_times(mutex_ft, nmutexes, "Trylock [unlocked] mutex");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nmutexes;  i++) {
        HAL_CLOCK_READ(&mutex_ft[i].start);
        cyg_mutex_trylock(&test_mutexes[i]);
        HAL_CLOCK_READ(&mutex_ft[i].end);
    }
    show_times(mutex_ft, nmutexes, "Trylock [locked] mutex");

    // Must unlock mutices before destroying them.
    for (i = 0;  i < nmutexes;  i++) {
        cyg_mutex_unlock(&test_mutexes[i]);
    }

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nmutexes;  i++) {
        HAL_CLOCK_READ(&mutex_ft[i].start);
        cyg_mutex_destroy(&test_mutexes[i]);
        HAL_CLOCK_READ(&mutex_ft[i].end);
    }
    show_times(mutex_ft, nmutexes, "Destroy mutex");
    run_mutex_circuit_test();
    end_of_test_group();
}

void
run_mutex_circuit_test(void)
{
    int i;
    // Set my priority lower than any I plan to create
    cyg_thread_set_priority(cyg_thread_self(), 4);
    // Set up for full mutex unlock/lock test
    cyg_mutex_init(&test_mutexes[0]);
    cyg_semaphore_init(&synchro, 0);
    cyg_thread_create(3,              // Priority - just a number
                      mutex_test,           // entry
                      0,               // index
                      thread_name("thread", 0),     // Name
                      &stacks[0][0],   // Stack
                      STACK_SIZE,      // Size
                      &mutex_test_thread_handle,   // Handle
                      &mutex_test_thread    // Thread data structure
        );
    cyg_thread_resume(mutex_test_thread_handle);
    // Need to raise priority so that this thread will block on the "lock"
    cyg_thread_set_priority(cyg_thread_self(), 2);
    for (i = 0;  i < nmutexes;  i++) {
        cyg_semaphore_post(&synchro);
        cyg_mutex_lock(&test_mutexes[0]);
        HAL_CLOCK_READ(&mutex_ft[i].end);
        cyg_mutex_unlock(&test_mutexes[0]);
        cyg_semaphore_wait(&synchro);
    }
    cyg_thread_delete(mutex_test_thread_handle);
    show_times(mutex_ft, nmutexes, "Unlock/Lock mutex");
}

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

void
run_semaphore_tests(void)
{
    int i;
    cyg_count32 sem_val;
    // Semaphore primitives
    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nsemaphores;  i++) {
        HAL_CLOCK_READ(&semaphore_ft[i].start);
        cyg_semaphore_init(&test_semaphores[i], 0);
        HAL_CLOCK_READ(&semaphore_ft[i].end);
    }
    show_times(semaphore_ft, nsemaphores, "Init semaphore");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nsemaphores;  i++) {
        HAL_CLOCK_READ(&semaphore_ft[i].start);
        cyg_semaphore_post(&test_semaphores[i]);
        HAL_CLOCK_READ(&semaphore_ft[i].end);
    }
    show_times(semaphore_ft, nsemaphores, "Post [0] semaphore");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nsemaphores;  i++) {
        HAL_CLOCK_READ(&semaphore_ft[i].start);
        cyg_semaphore_wait(&test_semaphores[i]);
        HAL_CLOCK_READ(&semaphore_ft[i].end);
    }
    show_times(semaphore_ft, nsemaphores, "Wait [1] semaphore");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nsemaphores;  i++) {
        HAL_CLOCK_READ(&semaphore_ft[i].start);
        cyg_semaphore_trywait(&test_semaphores[i]);
        HAL_CLOCK_READ(&semaphore_ft[i].end);
    }
    show_times(semaphore_ft, nsemaphores, "Trywait [0] semaphore");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nsemaphores;  i++) {
        cyg_semaphore_post(&test_semaphores[i]);
        HAL_CLOCK_READ(&semaphore_ft[i].start);
        cyg_semaphore_trywait(&test_semaphores[i]);
        HAL_CLOCK_READ(&semaphore_ft[i].end);
    }
    show_times(semaphore_ft, nsemaphores, "Trywait [1] semaphore");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nsemaphores;  i++) {
        HAL_CLOCK_READ(&semaphore_ft[i].start);
        cyg_semaphore_peek(&test_semaphores[i], &sem_val);
        HAL_CLOCK_READ(&semaphore_ft[i].end);
    }
    show_times(semaphore_ft, nsemaphores, "Peek semaphore");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nsemaphores;  i++) {
        HAL_CLOCK_READ(&semaphore_ft[i].start);
        cyg_semaphore_destroy(&test_semaphores[i]);
        HAL_CLOCK_READ(&semaphore_ft[i].end);
    }
    show_times(semaphore_ft, nsemaphores, "Destroy semaphore");

    run_semaphore_circuit_test();
    end_of_test_group();
}

void
run_semaphore_circuit_test(void)
{
    int i;
    // Set my priority lower than any I plan to create
    cyg_thread_set_priority(cyg_thread_self(), 3);
    // Set up for full semaphore post/wait test
    cyg_semaphore_init(&test_semaphores[0], 0);
    cyg_semaphore_init(&synchro, 0);
    cyg_thread_create(2,              // Priority - just a number
                      semaphore_test,           // entry
                      0,               // index
                      thread_name("thread", 0),     // Name
                      &stacks[0][0],   // Stack
                      STACK_SIZE,      // Size
                      &semaphore_test_thread_handle,   // Handle
                      &semaphore_test_thread    // Thread data structure
        );
    cyg_thread_resume(semaphore_test_thread_handle);
    for (i = 0;  i < nsemaphores;  i++) {
        wait_for_tick(); // Wait until the next clock tick to minimize aberations
        HAL_CLOCK_READ(&semaphore_ft[i].start);
        cyg_semaphore_post(&test_semaphores[0]);
        cyg_semaphore_wait(&synchro);
    }
    cyg_thread_delete(semaphore_test_thread_handle);
    show_times(semaphore_ft, nsemaphores, "Post/Wait semaphore");
}

void
run_counter_tests(void)
{
    int i;
    cyg_tick_count_t val=0;

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < ncounters;  i++) {
        HAL_CLOCK_READ(&counter_ft[i].start);
        cyg_counter_create(&counters[i], &test_counters[i]);
        HAL_CLOCK_READ(&counter_ft[i].end);
    }
    show_times(counter_ft, ncounters, "Create counter");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < ncounters;  i++) {
        HAL_CLOCK_READ(&counter_ft[i].start);
        val = cyg_counter_current_value(counters[i]);
        HAL_CLOCK_READ(&counter_ft[i].end);
    }
    show_times(counter_ft, ncounters, "Get counter value");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < ncounters;  i++) {
        HAL_CLOCK_READ(&counter_ft[i].start);
        cyg_counter_set_value(counters[i], val);
        HAL_CLOCK_READ(&counter_ft[i].end);
    }
    show_times(counter_ft, ncounters, "Set counter value");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < ncounters;  i++) {
        HAL_CLOCK_READ(&counter_ft[i].start);
        cyg_counter_tick(counters[i]);
        HAL_CLOCK_READ(&counter_ft[i].end);
    }
    show_times(counter_ft, ncounters, "Tick counter");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < ncounters;  i++) {
        HAL_CLOCK_READ(&counter_ft[i].start);
        cyg_counter_delete(counters[i]);
        HAL_CLOCK_READ(&counter_ft[i].end);
    }
    show_times(counter_ft, ncounters, "Delete counter");
    end_of_test_group();
}

void
run_flag_tests(void)
{
    int i;
    cyg_flag_value_t val;

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nflags;  i++) {
        HAL_CLOCK_READ(&flag_ft[i].start);
        cyg_flag_init(&test_flags[i]);
        HAL_CLOCK_READ(&flag_ft[i].end);
    }
    show_times(flag_ft, nflags, "Init flag");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nflags;  i++) {
        HAL_CLOCK_READ(&flag_ft[i].start);
        cyg_flag_destroy(&test_flags[i]);
        HAL_CLOCK_READ(&flag_ft[i].end);
    }
    show_times(flag_ft, nflags, "Destroy flag");

    // Recreate the flags - reused in the remaining tests
    for (i = 0;  i < nflags;  i++) {
        cyg_flag_init(&test_flags[i]);
    }

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nflags;  i++) {
        HAL_CLOCK_READ(&flag_ft[i].start);
        cyg_flag_maskbits(&test_flags[i], 0);
        HAL_CLOCK_READ(&flag_ft[i].end);
    }
    show_times(flag_ft, nflags, "Mask bits in flag");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nflags;  i++) {
        HAL_CLOCK_READ(&flag_ft[i].start);
        cyg_flag_setbits(&test_flags[i], 0x11);
        HAL_CLOCK_READ(&flag_ft[i].end);
    }
    show_times(flag_ft, nflags, "Set bits in flag [no waiters]");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nflags;  i++) {
        cyg_flag_setbits(&test_flags[i], 0x11);
        HAL_CLOCK_READ(&flag_ft[i].start);
        cyg_flag_wait(&test_flags[i], 0x11, CYG_FLAG_WAITMODE_AND);
        HAL_CLOCK_READ(&flag_ft[i].end);
    }
    show_times(flag_ft, nflags, "Wait for flag [AND]");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nflags;  i++) {
        cyg_flag_setbits(&test_flags[i], 0x11);
        HAL_CLOCK_READ(&flag_ft[i].start);
        cyg_flag_wait(&test_flags[i], 0x11, CYG_FLAG_WAITMODE_OR);
        HAL_CLOCK_READ(&flag_ft[i].end);
    }
    show_times(flag_ft, nflags, "Wait for flag [OR]");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nflags;  i++) {
        cyg_flag_setbits(&test_flags[i], 0x11);
        HAL_CLOCK_READ(&flag_ft[i].start);
        cyg_flag_wait(&test_flags[i], 0x11, CYG_FLAG_WAITMODE_AND|CYG_FLAG_WAITMODE_CLR);
        HAL_CLOCK_READ(&flag_ft[i].end);
    }
    show_times(flag_ft, nflags, "Wait for flag [AND/CLR]");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nflags;  i++) {
        cyg_flag_setbits(&test_flags[i], 0x11);
        HAL_CLOCK_READ(&flag_ft[i].start);
        cyg_flag_wait(&test_flags[i], 0x11, CYG_FLAG_WAITMODE_OR|CYG_FLAG_WAITMODE_CLR);
        HAL_CLOCK_READ(&flag_ft[i].end);
    }
    show_times(flag_ft, nflags, "Wait for flag [OR/CLR]");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nflags;  i++) {
        cyg_flag_setbits(&test_flags[i], 0x11);
        HAL_CLOCK_READ(&flag_ft[i].start);
        val = cyg_flag_peek(&test_flags[i]);
        HAL_CLOCK_READ(&flag_ft[i].end);
    }
    show_times(flag_ft, nflags, "Peek on flag");

    // Destroy flags - no longer needed
    for (i = 0;  i < nflags;  i++) {
        cyg_flag_destroy(&test_flags[i]);
    }
    end_of_test_group();
}

// Alarm callback function
void
alarm_cb(cyg_handle_t alarm, cyg_addrword_t val)
{
    // empty call back
}

// Callback used to test determinancy
static volatile int alarm_cnt;
void
alarm_cb2(cyg_handle_t alarm, cyg_addrword_t indx)
{
    if (alarm_cnt == nscheds) return;
    sched_ft[alarm_cnt].start = 0;
    HAL_CLOCK_READ(&sched_ft[alarm_cnt++].end);
    if (alarm_cnt == nscheds) {
        cyg_semaphore_post(&synchro);
    }
}

static void
alarm_cb3(cyg_handle_t alarm, cyg_addrword_t indx)
{
    if (alarm_cnt == nscheds) {
        cyg_semaphore_post(&synchro);
    } else {
        sched_ft[alarm_cnt].start = 0;
        cyg_thread_resume((cyg_handle_t)indx);
    }
}

// Null thread, used to keep scheduler busy
void
alarm_test(cyg_uint32 id)
{
    while (true) {
        cyg_thread_yield();
    }
}

// Thread that suspends itself at the first opportunity
void
alarm_test2(cyg_uint32 id)
{
    cyg_handle_t me = cyg_thread_self();
    while (true) {
        HAL_CLOCK_READ(&sched_ft[alarm_cnt++].end);
        cyg_thread_suspend(me);
    }
}

void
run_alarm_tests(void)
{
    int i;
    cyg_tick_count_t init_val, step_val;
    cyg_handle_t rtc_handle;

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < ncounters;  i++) {
        cyg_counter_create(&counters[i], &test_counters[i]);
    }
    for (i = 0;  i < nalarms;  i++) {
        HAL_CLOCK_READ(&alarm_ft[i].start);
        cyg_alarm_create(counters[0], alarm_cb, 0, &alarms[i], &test_alarms[i]);
        HAL_CLOCK_READ(&alarm_ft[i].end);
    }
    show_times(alarm_ft, nalarms, "Create alarm");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    init_val = 0;  step_val = 0;
    for (i = 0;  i < nalarms;  i++) {
        HAL_CLOCK_READ(&alarm_ft[i].start);
        cyg_alarm_initialize(alarms[i], init_val, step_val);
        HAL_CLOCK_READ(&alarm_ft[i].end);
    }
    show_times(alarm_ft, nalarms, "Initialize alarm");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    init_val = 0;  step_val = 0;
    for (i = 0;  i < nalarms;  i++) {
        HAL_CLOCK_READ(&alarm_ft[i].start);
        cyg_alarm_disable(alarms[i]);
        HAL_CLOCK_READ(&alarm_ft[i].end);
    }
    show_times(alarm_ft, nalarms, "Disable alarm");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    init_val = 0;  step_val = 0;
    for (i = 0;  i < nalarms;  i++) {
        HAL_CLOCK_READ(&alarm_ft[i].start);
        cyg_alarm_enable(alarms[i]);
        HAL_CLOCK_READ(&alarm_ft[i].end);
    }
    show_times(alarm_ft, nalarms, "Enable alarm");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nalarms;  i++) {
        HAL_CLOCK_READ(&alarm_ft[i].start);
        cyg_alarm_delete(alarms[i]);
        HAL_CLOCK_READ(&alarm_ft[i].end);
    }
    show_times(alarm_ft, nalarms, "Delete alarm");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    cyg_counter_create(&counters[0], &test_counters[0]);
    cyg_alarm_create(counters[0], alarm_cb, 0, &alarms[0], &test_alarms[0]);
    init_val = 9999;  step_val = 9999;
    cyg_alarm_initialize(alarms[0], init_val, step_val);
    cyg_alarm_enable(alarms[0]);
    for (i = 0;  i < ncounters;  i++) {
        HAL_CLOCK_READ(&counter_ft[i].start);
        cyg_counter_tick(counters[0]);
        HAL_CLOCK_READ(&counter_ft[i].end);
    }
    show_times(counter_ft, ncounters, "Tick counter [1 alarm]");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    cyg_counter_create(&counters[0], &test_counters[0]);
    for (i = 0;  i < nalarms;  i++) {
        cyg_alarm_create(counters[0], alarm_cb, 0, &alarms[i], &test_alarms[i]);
        init_val = 9999;  step_val = 9999;
        cyg_alarm_initialize(alarms[i], init_val, step_val);
        cyg_alarm_enable(alarms[i]);
    }
    for (i = 0;  i < ncounters;  i++) {
        HAL_CLOCK_READ(&counter_ft[i].start);
        cyg_counter_tick(counters[0]);
        HAL_CLOCK_READ(&counter_ft[i].end);
    }
    show_times(counter_ft, ncounters, "Tick counter [many alarms]");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    cyg_counter_create(&counters[0], &test_counters[0]);
    cyg_alarm_create(counters[0], alarm_cb, 0, &alarms[0], &test_alarms[0]);
    init_val = 1;  step_val = 1;
    cyg_alarm_initialize(alarms[0], init_val, step_val);
    cyg_alarm_enable(alarms[0]);
    for (i = 0;  i < ncounters;  i++) {
        HAL_CLOCK_READ(&counter_ft[i].start);
        cyg_counter_tick(counters[0]);
        HAL_CLOCK_READ(&counter_ft[i].end);
    }
    show_times(counter_ft, ncounters, "Tick & fire counter [1 alarm]");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    cyg_counter_create(&counters[0], &test_counters[0]);
    for (i = 0;  i < nalarms;  i++) {
        cyg_alarm_create(counters[0], alarm_cb, i, &alarms[i], &test_alarms[i]);
        init_val = 1;  step_val = 1;
        cyg_alarm_initialize(alarms[i], init_val, step_val);
        cyg_alarm_enable(alarms[i]);
    }
    for (i = 0;  i < nalarms;  i++) {
        HAL_CLOCK_READ(&alarm_ft[i].start);
        cyg_counter_tick(counters[0]);
        HAL_CLOCK_READ(&alarm_ft[i].end);
    }
    for (i = 0;  i < nalarms;  i++) {
        cyg_alarm_delete(alarms[i]);
    }
    show_times(alarm_ft, nalarms, "Tick & fire counters [>1 together]");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    cyg_counter_create(&counters[0], &test_counters[0]);
    for (i = 0;  i < nalarms;  i++) {
        cyg_alarm_create(counters[0], alarm_cb, i, &alarms[i], &test_alarms[i]);
        init_val = i+1;  step_val = nalarms+1;
        cyg_alarm_initialize(alarms[i], init_val, step_val);
        cyg_alarm_enable(alarms[i]);
    }
    for (i = 0;  i < nalarms;  i++) {
        HAL_CLOCK_READ(&alarm_ft[i].start);
        cyg_counter_tick(counters[0]);
        HAL_CLOCK_READ(&alarm_ft[i].end);
    }
    for (i = 0;  i < nalarms;  i++) {
        cyg_alarm_delete(alarms[i]);
    }
    show_times(alarm_ft, nalarms, "Tick & fire counters [>1 separately]");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    cyg_clock_to_counter(cyg_real_time_clock(), &rtc_handle);
    cyg_alarm_create(rtc_handle, alarm_cb2, 0, &alarms[0], &test_alarms[0]);
    init_val = 5;  step_val = 5;  alarm_cnt = 0;
    cyg_alarm_initialize(alarms[0], init_val, step_val);
    cyg_semaphore_init(&synchro, 0);
    cyg_alarm_enable(alarms[0]);
    cyg_semaphore_wait(&synchro);
    cyg_alarm_disable(alarms[0]);
    cyg_alarm_delete(alarms[0]);
    show_times(sched_ft, nscheds, "Alarm latency [0 threads]");

    // Set my priority higher than any I plan to create
    cyg_thread_set_priority(cyg_thread_self(), 2);
    for (i = 0;  i < 2;  i++) {
        cyg_thread_create(10,              // Priority - just a number
                          alarm_test,      // entry
                          i,               // index
                          thread_name("thread", i),     // Name
                          &stacks[i][0],   // Stack
                          STACK_SIZE,      // Size
                          &threads[i],     // Handle
                          &test_threads[i] // Thread data structure
            );
        cyg_thread_resume(threads[i]);
    }
    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    cyg_clock_to_counter(cyg_real_time_clock(), &rtc_handle);
    cyg_alarm_create(rtc_handle, alarm_cb2, 0, &alarms[0], &test_alarms[0]);
    init_val = 5;  step_val = 5;  alarm_cnt = 0;
    cyg_alarm_initialize(alarms[0], init_val, step_val);
    cyg_semaphore_init(&synchro, 0);
    cyg_alarm_enable(alarms[0]);
    cyg_semaphore_wait(&synchro);
    cyg_alarm_disable(alarms[0]);
    cyg_alarm_delete(alarms[0]);
    show_times(sched_ft, nscheds, "Alarm latency [2 threads]");
    for (i = 0;  i < 2;  i++) {
        cyg_thread_suspend(threads[i]);
        cyg_thread_delete(threads[i]);
    }

    // Set my priority higher than any I plan to create
    cyg_thread_set_priority(cyg_thread_self(), 2);
    for (i = 0;  i < ntest_threads;  i++) {
        cyg_thread_create(10,              // Priority - just a number
                          alarm_test,      // entry
                          i,               // index
                          thread_name("thread", i),     // Name
                          &stacks[i][0],   // Stack
                          STACK_SIZE,      // Size
                          &threads[i],     // Handle
                          &test_threads[i] // Thread data structure
            );
        cyg_thread_resume(threads[i]);
    }
    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    cyg_clock_to_counter(cyg_real_time_clock(), &rtc_handle);
    cyg_alarm_create(rtc_handle, alarm_cb2, 0, &alarms[0], &test_alarms[0]);
    init_val = 5;  step_val = 5;  alarm_cnt = 0;
    cyg_alarm_initialize(alarms[0], init_val, step_val);
    cyg_semaphore_init(&synchro, 0);
    cyg_alarm_enable(alarms[0]);
    cyg_semaphore_wait(&synchro);
    cyg_alarm_disable(alarms[0]);
    cyg_alarm_delete(alarms[0]);
    show_times(sched_ft, nscheds, "Alarm latency [many threads]");
    for (i = 0;  i < ntest_threads;  i++) {
        cyg_thread_suspend(threads[i]);
        cyg_thread_delete(threads[i]);
    }

    // Set my priority higher than any I plan to create
    cyg_thread_set_priority(cyg_thread_self(), 2);
    cyg_thread_create(10,              // Priority - just a number
                      alarm_test2,     // entry
                      i,               // index
                      thread_name("thread", 0),     // Name
                      &stacks[0][0],   // Stack
                      STACK_SIZE,      // Size
                      &threads[0],     // Handle
                      &test_threads[0] // Thread data structure
        );
    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    cyg_clock_to_counter(cyg_real_time_clock(), &rtc_handle);
    cyg_alarm_create(rtc_handle, alarm_cb3, threads[0], &alarms[0],
                     &test_alarms[0]);
    init_val = 5;  step_val = 5;  alarm_cnt = 0;
    cyg_alarm_initialize(alarms[0], init_val, step_val);
    cyg_semaphore_init(&synchro, 0);
    cyg_alarm_enable(alarms[0]);
    cyg_semaphore_wait(&synchro);
    cyg_alarm_disable(alarms[0]);
    cyg_alarm_delete(alarms[0]);
    show_times(sched_ft, nscheds, "Alarm -> thread resume latency");
    cyg_thread_suspend(threads[0]);
    cyg_thread_delete(threads[0]);
    
    end_of_test_group();
}

void
run_sched_tests(void)
{
    int i;

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nscheds;  i++) {
        HAL_CLOCK_READ(&sched_ft[i].start);
        cyg_scheduler_lock();
        HAL_CLOCK_READ(&sched_ft[i].end);
        cyg_scheduler_unlock();
    }
    show_times(sched_ft, nscheds, "Scheduler lock");

    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nscheds;  i++) {
        cyg_scheduler_lock();
        HAL_CLOCK_READ(&sched_ft[i].start);
        cyg_scheduler_unlock();
        HAL_CLOCK_READ(&sched_ft[i].end);
    }
    show_times(sched_ft, nscheds, "Scheduler unlock [0 threads]");

    // Set my priority higher than any I plan to create
    cyg_thread_set_priority(cyg_thread_self(), 2);
    for (i = 0;  i < 1;  i++) {
        cyg_thread_create(10,              // Priority - just a number
                          test0,           // entry
                          i,               // index
                          thread_name("thread", i),     // Name
                          &stacks[i][0],   // Stack
                          STACK_SIZE,      // Size
                          &threads[i],     // Handle
                          &test_threads[i] // Thread data structure
            );
    }
    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nscheds;  i++) {
        cyg_scheduler_lock();
        HAL_CLOCK_READ(&sched_ft[i].start);
        cyg_scheduler_unlock();
        HAL_CLOCK_READ(&sched_ft[i].end);
    }
    show_times(sched_ft, nscheds, "Scheduler unlock [1 suspended]");
    for (i = 0;  i < 1;  i++) {
        cyg_thread_delete(threads[i]);
    }

    // Set my priority higher than any I plan to create
    cyg_thread_set_priority(cyg_thread_self(), 2);
    for (i = 0;  i < ntest_threads;  i++) {
        cyg_thread_create(10,              // Priority - just a number
                          test0,           // entry
                          i,               // index
                          thread_name("thread", i),     // Name
                          &stacks[i][0],   // Stack
                          STACK_SIZE,      // Size
                          &threads[i],     // Handle
                          &test_threads[i] // Thread data structure
            );
    }
    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nscheds;  i++) {
        cyg_scheduler_lock();
        HAL_CLOCK_READ(&sched_ft[i].start);
        cyg_scheduler_unlock();
        HAL_CLOCK_READ(&sched_ft[i].end);
    }
    show_times(sched_ft, nscheds, "Scheduler unlock [many suspended]");
    for (i = 0;  i < ntest_threads;  i++) {
        cyg_thread_delete(threads[i]);
    }

    // Set my priority higher than any I plan to create
    cyg_thread_set_priority(cyg_thread_self(), 2);
    for (i = 0;  i < ntest_threads;  i++) {
        cyg_thread_create(10,              // Priority - just a number
                          test0,           // entry
                          i,               // index
                          thread_name("thread", i),     // Name
                          &stacks[i][0],   // Stack
                          STACK_SIZE,      // Size
                          &threads[i],     // Handle
                          &test_threads[i] // Thread data structure
            );
        cyg_thread_resume(threads[i]);
    }
    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < nscheds;  i++) {
        cyg_scheduler_lock();
        HAL_CLOCK_READ(&sched_ft[i].start);
        cyg_scheduler_unlock();
        HAL_CLOCK_READ(&sched_ft[i].end);
    }
    show_times(sched_ft, nscheds, "Scheduler unlock [many low prio]");
    for (i = 0;  i < ntest_threads;  i++) {
        cyg_thread_delete(threads[i]);
    }
    end_of_test_group();
}

static void 
_run_all_tests(CYG_ADDRESS id)
{
    int i, j;
    cyg_uint32 tv[nsamples], tv0, tv1;
    cyg_uint32 min_stack, max_stack, total_stack, actual_stack;
    cyg_tick_count_t ticks, tick0, tick1;
#ifdef CYG_SCHEDULER_LOCK_TIMINGS
    cyg_uint32 lock_ave, lock_max;
#endif
#if defined(CYGVAR_KERNEL_COUNTERS_CLOCK_LATENCY) && defined(HAL_CLOCK_LATENCY)
    cyg_int32 clock_ave;
#endif

    disable_clock_latency_measurement();

#ifndef CYGPKG_KERNEL_SMP_SUPPORT
    cyg_test_dump_thread_stack_stats( "Startup, main stack", thread[0] );
    cyg_test_dump_interrupt_stack_stats( "Startup" );
    cyg_test_dump_idlethread_stack_stats( "Startup" );
    cyg_test_clear_interrupt_stack();
#endif
    
    diag_printf("\neCos Kernel Timings\n");
    diag_printf("Notes: all times are in microseconds (.000001) unless otherwise stated\n");
#ifdef STATS_WITHOUT_FIRST_SAMPLE
    diag_printf("       second line of results have first sample removed\n");
#endif

    cyg_thread_delay(2);  // Make sure the clock is actually running

    ns_per_system_clock = 1000000/rtc_resolution[1];

    wait_for_tick();
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
    run_sched_tests();
    run_mutex_tests();
    run_mbox_tests();
    run_semaphore_tests();
    run_counter_tests();
    run_flag_tests();
    run_alarm_tests();

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

#if defined(CYGVAR_KERNEL_COUNTERS_CLOCK_DSR_LATENCY) && defined(HAL_CLOCK_LATENCY)
    disable_clock_latency_measurement();    
    clock_ave = (total_clock_dsr_latency*1000) / total_clock_dsr_calls;
    show_ticks_in_us(clock_ave);
    show_ticks_in_us(min_clock_dsr_latency*1000);
    show_ticks_in_us(max_clock_dsr_latency*1000);
    show_ticks_in_us(0);
    diag_printf("            Clock DSR latency\n\n");
    enable_clock_latency_measurement();
#endif

#ifndef CYGPKG_KERNEL_SMP_SUPPORT    
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

    cyg_test_dump_thread_stack_stats( "All done, main stack", thread[0] );
    cyg_test_dump_interrupt_stack_stats( "All done" );
    cyg_test_dump_idlethread_stack_stats( "All done" );
#endif
    
    enable_clock_latency_measurement();

    ticks = cyg_current_time();
    diag_printf("\nTiming complete - %d ms total\n\n", (int)((ticks*ns_per_system_clock)/1000));
}

void 
run_all_tests(CYG_ADDRESS id)
{
#if CYGNUM_TESTS_RUN_COUNT < 0
    while (1)
#else       
    int i;
    for (i = 0;  i < CYGNUM_TESTS_RUN_COUNT;  i++)
#endif
        _run_all_tests(id);
    CYG_TEST_PASS_FINISH("Basic timing OK");
}

void tm_basic_main( void )
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
        nflags = NFLAGS_SIM;
        ncounters = NCOUNTERS_SIM;
        nalarms = NALARMS_SIM;  
    } else {
        nsamples = NSAMPLES;
        ntest_threads = NTEST_THREADS;
        nthread_switches = NTHREAD_SWITCHES;
        nmutexes = NMUTEXES; 
        nmboxes = NMBOXES;
        nsemaphores = NSEMAPHORES;
        nscheds = NSCHEDS;
        nflags = NFLAGS;
        ncounters = NCOUNTERS;
        nalarms = NALARMS;
    }

    // Sanity
#ifdef WORKHORSE_TEST
    ntest_threads = max(512, ntest_threads);
    nmutexes = max(1024, nmutexes);
    nsemaphores = max(1024, nsemaphores);
    nmboxes = max(1024, nmboxes);
    ncounters = max(1024, ncounters);
    nalarms = max(1024, nalarms);
#else
    ntest_threads = max(64, ntest_threads);
    nmutexes = max(32, nmutexes);
    nsemaphores = max(32, nsemaphores);
    nmboxes = max(32, nmboxes);
    ncounters = max(32, ncounters);
    nflags = max(32, nflags);
    nalarms = max(32, nalarms);
#endif

    new_thread(run_all_tests, 0);
    
    Cyg_Scheduler::scheduler.start();

}

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
    tm_basic_main();
}   

#else // CYGFUN_KERNEL_API_C

externC void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_INFO("Timing tests require:\n"
                "CYGFUN_KERNEL_API_C && \n"
                "CYGSEM_KERNEL_SCHED_MLQUEUE &&\n"
                "CYGVAR_KERNEL_COUNTERS_CLOCK &&\n"
                "!CYGDBG_INFRA_DIAG_USE_DEVICE &&\n"
                "(CYGNUM_KERNEL_SCHED_PRIORITIES > 12)\n");
    CYG_TEST_NA("Timing tests requirements");
}
#endif // CYGFUN_KERNEL_API_C, etc.

// EOF tm_basic.cxx
