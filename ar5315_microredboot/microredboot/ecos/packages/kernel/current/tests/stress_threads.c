//==========================================================================
//
//        stress_threads.cxx
//
//        Basic thread stress test
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
// Author(s):     rosalia
// Contributors:  rosalia, jskov
// Date:          1999-04-13
// Description:   Very simple thread stress test, with some memory
//                allocation and alarm handling.
//
// Notes:
//  If client_makes_request is big, it means that there are made many more
//  client requests than can be serviced. Consequently, clients are wasting
//  CPU time and should be sleeping more.
//
//  The list of handler invocations show how many threads are running
//  at the same time. The more powerful the CPU, the more the numbers
//  should spread out.
//####DESCRIPTIONEND####

#include <pkgconf/system.h>
#include <cyg/infra/testcase.h>

#include <cyg/hal/hal_arch.h>

#if defined(CYGPKG_KERNEL) && defined(CYGPKG_IO) && defined(CYGPKG_ISOINFRA)

#include <pkgconf/kernel.h>
#include <pkgconf/isoinfra.h>
#include CYGHWR_MEMORY_LAYOUT_H

#if defined(CYGFUN_KERNEL_API_C)

#include <cyg/kernel/kapi.h>

#ifdef CYGINT_ISO_STDIO_FORMATTED_IO

#include <stdio.h>
#include <stdlib.h>

#if defined(CYGPKG_LIBM)

#include <math.h>
#include <assert.h>

#include <cyg/kernel/test/stackmon.h>

#if defined(CYGFUN_KERNEL_THREADS_TIMER)
#if CYGINT_ISO_MALLOC

/* if TIME_LIMIT is defined, it represents the number of seconds this
   test should last; if it is undefined the test will go forever */
#define DEATH_TIME_LIMIT 20
/* #undef DEATH_TIME_LIMIT */

// STACK_SIZE is typical +2kB for printf family calls which use big
// auto variables. Add more for handler which calls perform_stressful_tasks()
#define STACK_SIZE (2*1024 + CYGNUM_HAL_STACK_SIZE_TYPICAL)
#define STACK_SIZE_HANDLER (STACK_SIZE + 30*CYGNUM_HAL_STACK_FRAME_SIZE)

#define N_MAIN 1

// If we have instrumentation enabled, make the execution time in the
// simulator even shorter that we were going to anyway.
#ifdef CYGPKG_KERNEL_INSTRUMENT
#define SIM_DELAY_DIVISOR 100
#else
#define SIM_DELAY_DIVISOR 10
#endif

//-----------------------------------------------------------------------
// Some targets need to define a smaller number of handlers due to
// memory restrictions.
#if defined(CYGMEM_REGION_ram_SIZE) && (CYGMEM_REGION_ram_SIZE < 0x80000)
#define MAX_HANDLERS 4
#define N_LISTENERS 1
#define N_CLIENTS 1

#undef STACK_SIZE
#undef STACK_SIZE_HANDLER
#define STACK_SIZE (1024 + CYGNUM_HAL_STACK_SIZE_TYPICAL)
#define STACK_SIZE_HANDLER (STACK_SIZE + 10*CYGNUM_HAL_STACK_FRAME_SIZE)
#endif

//-----------------------------------------------------------------------
// If no target specific definitions, use defaults
#ifndef MAX_HANDLERS
#define MAX_HANDLERS 19
#define N_LISTENERS 4
#define N_CLIENTS 4
#endif

/* Allocate priorities in this order. This ensures that handlers
   (which are the ones using the CPU) get enough CPU time to actually
   complete their tasks. 

   The empty space ensures that if libc main() thread should happen to
   be in the priority range of the handlers, no handlers are
   accidently reduced so much in priority to get below
   listeners/clients. */

#define P_MAIN_PROGRAM    1
#define P_MAIN_PROGRAM_E  (P_MAIN_PROGRAM+N_MAIN)

#define P_BASE_HANDLER    (P_MAIN_PROGRAM_E)
#define P_BASE_HANDLER_E  (P_BASE_HANDLER+MAX_HANDLERS)

#define P_BASE_EMPTY      (P_BASE_HANDLER_E)
#define P_BASE_EMPTY_E    (P_BASE_EMPTY+2)

#define P_BASE_LISTENER   (P_BASE_EMPTY_E)
#define P_BASE_LISTENER_E (P_BASE_LISTENER+N_LISTENERS)

#define P_BASE_CLIENT     (P_BASE_LISTENER_E)
#define P_BASE_CLIENT_E   (P_BASE_CLIENT+N_CLIENTS)

#define P_MAX             (P_BASE_CLIENT_E)

/* Ensure there's room for what we request */
#if (CYGNUM_KERNEL_SCHED_PRIORITIES >= P_MAX)

/* if we use the bitmap scheduler we must make sure we don't use the
   same priority more than once, so we must store those already in use */
static volatile char priority_in_use[P_MAX];

/* We may not get the priority we ask for (scheduler may decide to ignore
   schedule hint). So keep a table of priorities actually assigned to
   the threads. This information may come in handy for debugging - it's
   not actively used by the code. */
static volatile int  priority_translation[P_MAX];

/* now declare (and allocate space for) some kernel objects, like the
   threads we will use */
cyg_thread main_thread_s;
cyg_thread handler_thread_s[MAX_HANDLERS];
cyg_thread listener_thread_s[N_LISTENERS];
cyg_thread client_thread_s[N_CLIENTS];

/* space for stacks for all threads */
char main_stack[STACK_SIZE];
char handler_stack[MAX_HANDLERS][STACK_SIZE_HANDLER];
char listener_stack[N_LISTENERS][STACK_SIZE];
char client_stack[N_CLIENTS][STACK_SIZE];

/* now the handles for the threads */
cyg_handle_t mainH;
cyg_handle_t handlerH[MAX_HANDLERS];
cyg_handle_t listenerH[N_LISTENERS];
cyg_handle_t clientH[N_CLIENTS];

/* space for thread names */
char thread_name[P_MAX][20];

/* and now variables for the procedure which is the thread */
cyg_thread_entry_t main_program, client_program, listener_program, 
    handler_program;

/* a few mutexes used in the code */
cyg_mutex_t client_request_lock, handler_slot_lock, statistics_print_lock, 
    free_handler_lock;

/* global variables with which the handler IDs and thread priorities
   to free are communicated from handlers to main_program. Access to
   these are protected by free_handler_lock. An id of -1 means the
   that the variables are empty. */
volatile int free_handler_pri = 0;
volatile int free_handler_id = -1;

/* a global variable with which the client and server coordinate */
volatile int client_makes_request = 0;

/* if this is true, clients will not make requests */
volatile int clients_paused = 0;


/* indicates that it's time to print out a report */
volatile int time_to_report = 0;
/* print status after a delay of this many secs. */
int time_report_delay;

/*** now application-specific variables ***/
/* an array that stores whether the handler threads are in use */
volatile int handler_thread_in_use[MAX_HANDLERS];
/* total count of active handlers */
volatile int handler_thread_in_use_count;


/***** statistics-gathering variables *****/
struct s_statistics {
    /* store the number of times each handler has been invoked */
    unsigned long handler_invocation_histogram[MAX_HANDLERS];

    /* store how many times malloc has been attempted and how many times
       it has failed */
    unsigned long malloc_tries, malloc_failures;

    /* how many threads have been created */
    unsigned long thread_creations, thread_exits;
};

struct s_statistics statistics;

/* some function prototypes; those with the sc_ prefix are
   "statistics-collecting" versions of the cyg_ primitives */
cyg_addrword_t sc_thread_create(
    cyg_addrword_t      sched_info,             /* scheduling info (eg pri)  */
    cyg_thread_entry_t  *entry,                 /* entry point function      */
    cyg_addrword_t      entry_data,             /* entry data                */
    char                *name,                  /* optional thread name      */
    void                *stack_base,            /* stack base, NULL = alloc  */
    cyg_ucount32        stack_size,             /* stack size, 0 = default   */
    cyg_handle_t        *handle,                /* returned thread handle    */
    cyg_thread          *thread                 /* put thread here           */
    );

void start_handler(void);
void stop_handler(int handler_id, int handler_pri);
void perform_stressful_tasks(void);
void permute_array(char a[], int size, int seed);
void setup_death_alarm(cyg_addrword_t data, cyg_handle_t *deathHp,
                       cyg_alarm *death_alarm_p, int *killed_p);
void print_statistics(int print_full);

/* we need to declare the alarm handling function (which is defined
   below), so that we can pass it to cyg_alarm_initialize() */
cyg_alarm_t report_alarm_func, death_alarm_func;

/* handle and alarm for the report alarm */
cyg_handle_t report_alarmH, counterH, system_clockH;
cyg_alarm report_alarm;

/* main launches all the threads of the test */
int
main(void)
{
    int i;

    CYG_TEST_INIT();
    CYG_TEST_INFO("Stress threads test compiled on " __DATE__);

    cyg_mutex_init(&client_request_lock);
    cyg_mutex_init(&statistics_print_lock);
    cyg_mutex_init(&free_handler_lock);

    /* initialize statistics */
    memset(&statistics, 0, sizeof(statistics));

    /* clear priority table */
    for (i = 0; i < sizeof(priority_in_use); i++)
        priority_in_use[i] = 0;

    /* initialize main thread */
    {
        priority_translation[P_MAIN_PROGRAM] =
            sc_thread_create(P_MAIN_PROGRAM, main_program, (cyg_addrword_t) 0,
                             "main_program", (void *) main_stack, STACK_SIZE,
                             &mainH, &main_thread_s);
        priority_in_use[P_MAIN_PROGRAM]++;
    }

    /* initialize all handler threads to not be in use */
    for (i = 0; i < MAX_HANDLERS; ++i) {
        handler_thread_in_use[i] = 0;
    }
    handler_thread_in_use_count = 0;
    for (i = 0; i < N_LISTENERS; ++i) {
        int prio = P_BASE_LISTENER + i;
        char* name = &thread_name[prio][0];
        sprintf(name, "listener-%02d", i);
        priority_translation[prio] =
            sc_thread_create(prio, listener_program, (cyg_addrword_t) i,
                             name, (void *) listener_stack[i], STACK_SIZE,
                             &listenerH[i], &listener_thread_s[i]);
        CYG_ASSERT(0 == priority_in_use[prio], "Priority already in use!");
        priority_in_use[prio]++;
    }
    for (i = 0; i < N_CLIENTS; ++i) {
        int prio = P_BASE_CLIENT + i;
        char* name = &thread_name[prio][0];
        sprintf(name, "client-%02d", i);
        priority_translation[prio] =
            sc_thread_create(prio, client_program, (cyg_addrword_t) i,
                             name, (void *) client_stack[i], STACK_SIZE,
                             &(clientH[i]), &client_thread_s[i]);
        CYG_ASSERT(0 == priority_in_use[prio], "Priority already in use!");
        priority_in_use[prio]++;
    }

    cyg_thread_resume(mainH);
    for (i = 0; i < N_CLIENTS; ++i) {
        cyg_thread_resume(clientH[i]);
    }
    for (i = 0; i < N_LISTENERS; ++i) {
        cyg_thread_resume(listenerH[i]);
    }

    /* set up the alarm which gives periodic wakeups to say "time to
       print a report */
    system_clockH = cyg_real_time_clock();
    cyg_clock_to_counter(system_clockH, &counterH);

    cyg_alarm_create(counterH, report_alarm_func,
                     (cyg_addrword_t) 4000,
                     &report_alarmH, &report_alarm);
    if (cyg_test_is_simulator) {
        time_report_delay = 2;
    } else {
        time_report_delay = 60;
    }

    cyg_alarm_initialize(report_alarmH, cyg_current_time()+200, 
                         time_report_delay*100);

    return 0;
}

/* main_program() -- frees resources and prints status. */
void main_program(cyg_addrword_t data)
{
#ifdef DEATH_TIME_LIMIT
    cyg_handle_t deathH;
    cyg_alarm death_alarm;
    int is_dead = 0;

    setup_death_alarm(0, &deathH, &death_alarm, &is_dead);
#endif /* DEATH_TIME_LIMIT */

    for (;;) {
        int handler_id = -1;
        int handler_pri = 0;

        cyg_mutex_lock(&free_handler_lock); {
            // If any handler has left its ID, copy the ID and
            // priority values to local variables, and free up the
            // global communication variables again.
            if (-1 != free_handler_id) {
                handler_id = free_handler_id;
                handler_pri = free_handler_pri;
                free_handler_id = -1;
            }
        } cyg_mutex_unlock(&free_handler_lock);

        if (-1 != handler_id) {
            stop_handler(handler_id, handler_pri);
        }

        // If it's time to report status or quit, set pause flag and
        // keep looping until all handlers have stopped.
        if (time_to_report) {
            // Pause clients
            cyg_mutex_lock(&client_request_lock); {
                clients_paused = 1;
            } cyg_mutex_unlock(&client_request_lock);

            // When all handlers have stopped, we can print statistics
            // knowing that all (handler allocated) resources should have
            // been freed. That is, we should be able to determine leaks.
            if (0 == handler_thread_in_use_count) {
                print_statistics(0);

                // We've done the printing now. Resume the system.
                time_to_report = 0;
                cyg_mutex_lock(&client_request_lock); {
                    clients_paused = 0;
                } cyg_mutex_unlock(&client_request_lock);
            } 
        }

#ifdef DEATH_TIME_LIMIT
        // Stop test if time.
        if (is_dead) {
            // Pause clients
            cyg_mutex_lock(&client_request_lock); {
                clients_paused = 1;
            } cyg_mutex_unlock(&client_request_lock);

            // When all handlers have stopped, we can print statistics
            // knowing that all (handler allocated) resources should have
            // been freed. That is, we should be able to determine leaks.
            if (0 == handler_thread_in_use_count) {
                print_statistics(1);
                CYG_TEST_PASS_FINISH("Kernel thread stress test OK");
            }
        }
#endif /* DEATH_TIME_LIMIT */

        cyg_thread_delay(3);
    }
}

/* client_program() -- an obnoxious client which makes a lot of requests */
void client_program(cyg_addrword_t data)
{
    int delay;

    system_clockH = cyg_real_time_clock();
    cyg_clock_to_counter(system_clockH, &counterH);

    for (;;) {
        delay = (rand() % 20);

        /* now send a request to the server */
        cyg_mutex_lock(&client_request_lock); {
            if (0 == clients_paused)
                client_makes_request++;
        } cyg_mutex_unlock(&client_request_lock);

        cyg_thread_delay(10+delay);
    }
}

/* listener_program() -- listens for a request and spawns a handler to
   take care of the request */
void listener_program(cyg_addrword_t data)
{
    for (;;) {
        int make_request = 0;
        cyg_mutex_lock(&client_request_lock); {
            if (client_makes_request > 0) {
                --client_makes_request;
                make_request = 1;
            }
        } cyg_mutex_unlock(&client_request_lock);
        
        if (make_request)
            start_handler();
        
        cyg_thread_delay(2 + (rand() % 10));
    }
}

/* handler_program() -- is spawned to handle each incoming request */
void handler_program(cyg_addrword_t data)
{
    /* here is where we perform specific stressful tasks */
    perform_stressful_tasks();

    cyg_thread_delay(4 + (int) (0.5*log(1.0 + fabs((rand() % 1000000)))));

    {
        // Loop until the handler id and priority can be communicated to
        // the main_program.
        int freed = 0;
        do {
            cyg_mutex_lock(&free_handler_lock); {
                if (-1 == free_handler_id) {
                    free_handler_id = data;
                    free_handler_pri = P_BASE_HANDLER+(int) data;
                    freed = 1;
                }
            } cyg_mutex_unlock(&free_handler_lock);
            if (!freed)
                cyg_thread_delay(2);
        } while (!freed);
    }

    // Then exit.
    cyg_thread_exit();
}

/* start a new handler */
void start_handler(void)
{
    int prio;
    char* name;
    int handler_slot = 0;
    int found = 0;

    while (!found) {
        cyg_mutex_lock(&handler_slot_lock); {
            for (handler_slot = 0; handler_slot < MAX_HANDLERS;++handler_slot){
                if (!handler_thread_in_use[handler_slot]) {
                    found = 1;
                    handler_thread_in_use[handler_slot]++;
                    handler_thread_in_use_count++;
                    break;
                }
            }
        } cyg_mutex_unlock(&handler_slot_lock);
        if (!found)
            cyg_thread_delay(1);
    }

    CYG_ASSERT(1 == handler_thread_in_use[handler_slot], 
               "Handler usage count wrong!");

    prio = P_BASE_HANDLER+handler_slot;
    CYG_ASSERT(0 == priority_in_use[prio], "Priority already in use!");
    priority_in_use[prio]++;

    name = &thread_name[prio][0];
    sprintf(name, "handler-%02d/%02d", handler_slot, prio);

    priority_translation[prio] =
        sc_thread_create(prio, handler_program,
                         (cyg_addrword_t) handler_slot,
                         name, (void *) handler_stack[handler_slot],
                         STACK_SIZE_HANDLER, &handlerH[handler_slot],
                         &handler_thread_s[handler_slot]);
    cyg_thread_resume(handlerH[handler_slot]);
    ++statistics.handler_invocation_histogram[handler_slot];
}

/* free a locked handler thread */
void stop_handler(int handler_id, int handler_pri)
{
    // Finally delete the handler thread. This must be done in a
    // loop, waiting for the call to return true. If it returns
    // false, go to sleep for a bit, so the killed thread gets a
    // chance to run and complete its business.
    while (!cyg_thread_delete(handlerH[handler_id])) {
        cyg_thread_delay(1);
    }
    ++statistics.thread_exits;
    
    // Free the handler resources.
    cyg_mutex_lock(&handler_slot_lock); {
        handler_thread_in_use[handler_id]--;
        handler_thread_in_use_count--;
        priority_in_use[handler_pri]--;
        CYG_ASSERT(0 == priority_in_use[handler_pri], 
                   "Priority not in use!");
        CYG_ASSERT(0 == handler_thread_in_use[handler_id], 
                   "Handler not in use!");
        CYG_ASSERT(0 <= handler_thread_in_use_count, 
                   "Stopped more handlers than was started!");
    } cyg_mutex_unlock(&handler_slot_lock);
        
}


/* do things which will stress the system */
void perform_stressful_tasks()
{
#define MAX_MALLOCED_SPACES 100  /* do this many mallocs at most */
#define MALLOCED_BASE_SIZE 1    /* basic size in bytes */
    char *spaces[MAX_MALLOCED_SPACES];
    int  sizes[MAX_MALLOCED_SPACES];
    unsigned int i, j, size;

    cyg_uint8 pool_space[10][100];
    cyg_handle_t mempool_handles[10];
    cyg_mempool_fix mempool_objects[10];

    /* here I use malloc, which uses the kernel's variable memory pools.
       note that malloc/free is a bit simple-minded here: it does not
       try to really fragment things, and it does not try to make the
       allocation/deallocation concurrent with other thread execution
       (although I'm about to throw in a yield()) */
    for (i = 0; i < MAX_MALLOCED_SPACES; ++i) {
        ++statistics.malloc_tries;
        size = (i*2+1)*MALLOCED_BASE_SIZE;
        spaces[i] = (char *) malloc(size);
        sizes[i] = size;

        if (spaces[i] != NULL) {
            // Fill with a known value (differs between chunk).
            for (j = 0; j < size; ++j) {
                spaces[i][j] = 0x50 | ((j+i) & 0x0f);
            }
        }

        if (i % (MAX_MALLOCED_SPACES/10) == 0) {
            cyg_thread_yield();
        }
        if (i % (MAX_MALLOCED_SPACES/15) == 0) {
            cyg_thread_delay(i % 5);
        }
    }

    cyg_thread_delay(5);

    /* now free it all up */
    for (i = 0; i < MAX_MALLOCED_SPACES; ++i) {
        if (spaces[i] != NULL) {
            size = sizes[i];
            for (j = 0; j < size; ++j) {
                // Validate chunk data.
                if ((0x50 | ((j+i) & 0x0f)) != spaces[i][j]) {
                    printf("Bad byte in chunk\n");
                }
                spaces[i][j] = 0xAA;    /* write a bit pattern */
            }
            free(spaces[i]);
        } else {
            ++statistics.malloc_failures;
        }
    }

    /* now allocate and then free some fixed-size memory pools; for
       now this is simple-minded because it does not have many threads
       sharing the memory pools and racing for memory. */
    for (i = 0; i < 10; ++i) {
        cyg_mempool_fix_create(pool_space[i], 100, (i+1)*3,
                               &mempool_handles[i], &mempool_objects[i]);
    }

    for (i = 0; i < 10; ++i) {
        spaces[i] = cyg_mempool_fix_try_alloc(mempool_handles[i]);
    }

    for (i = 0; i < 10; ++i) {
        if (spaces[i]) {
            cyg_mempool_fix_delete(mempool_handles[i]);
        }
    }
}

/* report_alarm_func() is invoked as an alarm handler, so it should be
   quick and simple.  in this case it sets a global flag which is
   checked by main_program. */
void report_alarm_func(cyg_handle_t alarmH, cyg_addrword_t data)
{
    time_to_report = 1;
}

#ifdef DEATH_TIME_LIMIT
/* this sets up death alarms. it gets the handle and alarm from the
   caller, since they must persist for the life of the alarm */
void setup_death_alarm(cyg_addrword_t data, cyg_handle_t *deathHp,
                       cyg_alarm *death_alarm_p, int *killed_p)
{
    cyg_handle_t system_clockH, counterH;
    cyg_resolution_t rtc_res;

    system_clockH = cyg_real_time_clock();
    cyg_clock_to_counter(system_clockH, &counterH);

    cyg_alarm_create(counterH, death_alarm_func,
                     (cyg_addrword_t) killed_p,
                     deathHp, death_alarm_p);
    rtc_res = cyg_clock_get_resolution(system_clockH);
    {
        cyg_tick_count_t tick_delay;
        tick_delay = (long long)
            ((1000000000.0*rtc_res.divisor)
             *((double)DEATH_TIME_LIMIT)/((double)rtc_res.dividend));
        if ( cyg_test_is_simulator )
            tick_delay /= SIM_DELAY_DIVISOR;
#ifdef CYGPKG_HAL_SYNTH
        // 20 seconds is a long time compared to the run time of other tests.
        // Reduce to 10 seconds, allowing more tests to get run.
        tick_delay /= 2;
#endif

        cyg_alarm_initialize(*deathHp, cyg_current_time() + tick_delay, 0);
    }
}
#endif

/* death_alarm_func() is the alarm handler that kills the current
   thread after a specified timeout. It does so by setting a flag the
   thread is constantly checking. */
void death_alarm_func(cyg_handle_t alarmH, cyg_addrword_t data)
{
    int *killed_p;
    killed_p = (int *) data;
    *killed_p = 1;
}

/* now I write the sc_ versions of the cyg_functions */
cyg_addrword_t sc_thread_create(
    cyg_addrword_t      sched_info,            /* scheduling info (eg pri)  */
    cyg_thread_entry_t  *entry,                /* entry point function      */
    cyg_addrword_t      entry_data,            /* entry data                */
    char                *name,                 /* optional thread name      */
    void                *stack_base,           /* stack base, NULL = alloc  */
    cyg_ucount32        stack_size,            /* stack size, 0 = default   */
    cyg_handle_t        *handle,               /* returned thread handle    */
    cyg_thread          *thread                /* put thread here           */
    )
{
    ++statistics.thread_creations;

    cyg_thread_create(sched_info, entry, entry_data, name,
                      stack_base, stack_size, handle, thread);

    return cyg_thread_get_priority(*handle);
}


#define MINS_HOUR (60)
#define MINS_DAY  (60*24)

void print_statistics(int print_full)
{
    int i;
    static int stat_dumps = 0;
    static int print_count = 0;
    static int shift_count = 0;
    int minutes;

    stat_dumps++;

    // Find number of minutes.
    minutes = time_report_delay*stat_dumps / 60;

    if (!print_full) {
        // Return if time/minutes not integer.
        if ((time_report_delay*stat_dumps % 60) != 0)
            return;

        // After the first day, only dump stat once per day. Do print
        // a . on the hour though.
        if ((minutes > MINS_DAY) && ((minutes % MINS_DAY) != 0)) {
            if ((minutes % MINS_HOUR) == 0) {
                printf(".");
                fflush(stdout);
            }
            return;
        }

        // After the first hour of the first day, only dump stat once
        // per hour. Do print . each minute though.
        if ((minutes < MINS_DAY) && (minutes > MINS_HOUR)
            && ((minutes % MINS_HOUR) != 0)) {
            printf(".");
            fflush(stdout);
            return;
        }
    }

    printf("\nState dump %d (%d hours, %d minutes) [numbers >>%d]\n",
           ++print_count, minutes / MINS_HOUR, minutes % MINS_HOUR, 
           shift_count);

    cyg_mutex_lock(&statistics_print_lock); {
        //--------------------------------
        // Information private to this test:
        printf(" Handler-invocations: ");
        for (i = 0; i < MAX_HANDLERS; ++i) {
            printf("%4lu ", statistics.handler_invocation_histogram[i]);
        }
        printf("\n");
        printf(" malloc()-tries/failures: -- %7lu %7lu\n",
               statistics.malloc_tries, statistics.malloc_failures);
        printf(" client_makes_request:       %d\n", client_makes_request);

        // Check for big numbers and reduce if getting close to overflow
        if (statistics.malloc_tries > 0x40000000) {
            shift_count++;
            for (i = 0; i < MAX_HANDLERS; ++i) {
                statistics.handler_invocation_histogram[i] >>= 1;
            }
            statistics.malloc_tries >>= 1;
            statistics.malloc_failures >>= 1;
        }
    } cyg_mutex_unlock(&statistics_print_lock);

#if CYGINT_ISO_MALLINFO
    //--------------------------------
    // System information
    {
        struct mallinfo mem_info;
       
        mem_info = mallinfo();
        
        printf(" Memory system: Total=0x%08x Free=0x%08x Max=0x%08x\n", 
               mem_info.arena, mem_info.fordblks, mem_info.maxfree);
    }
#endif

    // Dump stack status
    printf(" Stack usage:\n");
    cyg_test_dump_interrupt_stack_stats( "  Interrupt" );
    cyg_test_dump_idlethread_stack_stats( "  Idle" );

    cyg_test_dump_stack_stats("  Main", main_stack, 
                              main_stack + sizeof(main_stack));
    for (i = 0; i < MAX_HANDLERS; i++) {
        cyg_test_dump_stack_stats("  Handler", handler_stack[i], 
                                  handler_stack[i] + sizeof(handler_stack[i]));
    }
    for (i = 0; i < N_LISTENERS; i++) {
        cyg_test_dump_stack_stats("  Listener", listener_stack[i], 
                                  listener_stack[i] + sizeof(listener_stack[i]));
    }
    for (i = 0; i < N_CLIENTS; i++) {
        cyg_test_dump_stack_stats("  Client", client_stack[i], 
                                  client_stack[i] + sizeof(client_stack[i]));
    }
}

#else /* (CYGNUM_KERNEL_SCHED_PRIORITIES >=    */
/* (N_MAIN+N_CLIENTS+N_LISTENERS+MAX_HANDLERS)) */
#define N_A_MSG "not enough priorities available"
#endif /* (CYGNUM_KERNEL_SCHED_PRIORITIES >=    */
/* (N_MAIN+N_CLIENTS+N_LISTENERS+MAX_HANDLERS)) */

#else /* CYGINT_ISO_MALLOC */
# define N_A_MSG "this test needs malloc"
#endif /* CYGINT_ISO_MALLOC */

#else /* CYGFUN_KERNEL_THREADS_TIMER */
# define N_A_MSG "this test needs kernel threads timer"
#endif /* CYGFUN_KERNEL_THREADS_TIMER */

#else /* CYGPKG_LIBM */
# define N_A_MSG "this test needs libm"
#endif /* CYGPKG_LIBM */

#else /* CYGINT_ISO_STDIO_FORMATTED_IO */
# define N_A_MSG "this test needs stdio formatted I/O"
#endif /* CYGINT_ISO_STDIO_FORMATTED_IO */

#else // def CYGFUN_KERNEL_API_C
# define N_A_MSG "this test needs Kernel C API"
#endif

#else // def CYGPKG_KERNEL && CYGPKG_IO && CYGPKG_ISOINFRA
# define N_A_MSG "this tests needs Kernel, isoinfra and IO"
#endif

#ifdef N_A_MSG
externC void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_NA( N_A_MSG);
}
#endif // N_A_MSG
