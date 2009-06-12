//==========================================================================
//
//        thread_gdb.c
//
//        A test for thread support in GDB
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
// Date:          1998-09-21
// Description:   GDB thread support test.
//####DESCRIPTIONEND####
// 

#include <cyg/kernel/kapi.h>

#include <cyg/infra/cyg_ass.h>

#include <cyg/infra/testcase.h>

#if defined(CYGFUN_KERNEL_API_C) && defined(CYGSEM_KERNEL_SCHED_MLQUEUE) &&\
    (CYGNUM_KERNEL_SCHED_PRIORITIES > 26)

#include <cyg/hal/hal_arch.h>           // for CYGNUM_HAL_STACK_SIZE_TYPICAL

// -------------------------------------------------------------------------

#define THREADS                 10

#ifdef CYGNUM_HAL_STACK_SIZE_TYPICAL
#define STACKSIZE CYGNUM_HAL_STACK_SIZE_TYPICAL
#else
#define STACKSIZE               (2*1024)
#endif

#define CONTROLLER_PRI_HI       0
#define CONTROLLER_PRI_LO       25

#define WORKER_PRI              3
#define WORKER_PRI_RANGE        20


// -------------------------------------------------------------------------

// array of stacks for threads
char thread_stack[THREADS][STACKSIZE];

// array of threads.
cyg_thread thread[THREADS];

cyg_handle_t thread_handle[THREADS];

volatile cyg_uint8 worker_state;

#define WORKER_STATE_WAIT       1
#define WORKER_STATE_BREAK      2
#define WORKER_STATE_EXIT       9

cyg_mutex_t worker_mutex;
cyg_cond_t worker_cv;

cyg_count32 workers_asleep = 0;

cyg_count32 thread_count[THREADS];

cyg_priority_t thread_pri[THREADS];



// -------------------------------------------------------------------------

extern void breakme(void)
{
}

// -------------------------------------------------------------------------

void worker( cyg_addrword_t id )
{
    for(;;)
    {
        thread_count[id]++;
        thread_pri[id] = cyg_thread_get_priority( cyg_thread_self() );
        
        switch( worker_state )
        {
        case WORKER_STATE_BREAK:
            if( 0 == (id % 4) )
                breakme();
            
        case WORKER_STATE_WAIT:
            cyg_mutex_lock( &worker_mutex );
            workers_asleep++;
            cyg_cond_wait( &worker_cv );
            workers_asleep--;
            cyg_mutex_unlock( &worker_mutex );
            break;

        case WORKER_STATE_EXIT:
            cyg_thread_exit();
            
        }
    }
}

// -------------------------------------------------------------------------

void controller( cyg_addrword_t id )
{
    cyg_priority_t pri;
    int i;

    cyg_mutex_init( &worker_mutex );
    cyg_cond_init( &worker_cv, &worker_mutex );
    
// 1 thread, it is running, it calls BREAKME();
//  +++ Thread status returned:
//  +++ 1 thread, running, is the current one
    
    breakme();

// Create N more threads; they are all suspended after creation.  Adjust
// the priorities of all the threads to lower than the controlling thread.
// Make them all be distinct priorities as far as possible.  BREAKME();
//  +++ 1 thread, running, + N suspended ones of different prios.

    for( i = 1; i < THREADS; i++ )
    {
        pri = CONTROLLER_PRI_HI + 1 + i % WORKER_PRI_RANGE;

        cyg_thread_create(pri, worker, (cyg_addrword_t)i, "worker",
                          (void *)(&thread_stack[i]), STACKSIZE,
                          &thread_handle[i], &thread[i]);

    }

    breakme();

// Adjust the priorities of all the threads to lower than the controlling
// thread.  Make them all be THE SAME priority.  BREAKME();
//  +++ 1 thread, running, + N suspended ones of same prio.

    for( i = 1; i < THREADS; i++ )
    {
        cyg_thread_set_priority( thread_handle[i], WORKER_PRI );
    }

    breakme();
    
// Release all the N threads, BREAKME();
//  +++ 1 thread, running, + N ready ones of same prio.

    for( i = 1; i < THREADS; i++ )
    {
        cyg_thread_resume( thread_handle[i] );
    }

    breakme();

// Adjust the priorities of all the threads, lower than the controlling
// thread.  Make them all be distinct priorities as far as possible.
// BREAKME();
//  +++ 1 thread, running, + N ready ones of different prios.

    for( i = 1; i < THREADS; i++ )
    {
        pri = CONTROLLER_PRI_HI + 1 + i % WORKER_PRI_RANGE;
        
        cyg_thread_set_priority( thread_handle[i], pri );
    }

    breakme();
    
// Command all the N threads to sleep; switch my own priority to lower
// than theirs so that they all run and sleep, then I get back in and
// BREAKME();
// +++ 1 thread, running, + N sleeping ones of different prios.

    worker_state = WORKER_STATE_WAIT;

    cyg_thread_set_priority( thread_handle[0], CONTROLLER_PRI_LO );

    breakme();
    
// Make them all be THE SAME priority; BREAKME();
//  +++ 1 thread, running, + N sleeping ones of same prio.

    for( i = 1; i < THREADS; i++ )
    {
        cyg_thread_set_priority( thread_handle[i], WORKER_PRI );
    }

    breakme();

// Wake them all up, they'll loop once and sleep again; I get in and
// BREAKME();
//  +++ 1 thread, running, + N sleeping ones of same prio.

    cyg_cond_broadcast( &worker_cv );

    breakme();

// Adjust the priorities of all the threads, higher than the controlling
// thread.  Make them all be distinct priorities as far as possible.
// BREAKME();
//  +++ 1 thread, running, + N sleeping ones of different prios.

    for( i = 1; i < THREADS; i++ )
    {
        pri = CONTROLLER_PRI_HI + 1 + i % WORKER_PRI_RANGE;
        
        cyg_thread_set_priority( thread_handle[i], pri );
    }

    breakme();

// Wake them all up, they'll loop once and sleep again; I get in and
// BREAKME();
//  +++ 1 thread, running, + N sleeping ones of different prios.

    cyg_cond_broadcast( &worker_cv );

    breakme();

// Set them all the same prio, set me to the same prio, BREAKME();
//  +++ 1 running, + N sleeping, *all* the same prio.

    for( i = 0; i < THREADS; i++ )
    {
        cyg_thread_set_priority( thread_handle[i], WORKER_PRI );
    }

    breakme();

// Wake them all up, they'll loop once and sleep again; I get in and
// BREAKME(); repeatedly until they have all slept again.
//  +++ 1 running, + some sleeping, some ready, *all* the same prio.

    cyg_cond_broadcast( &worker_cv );

//    cyg_thread_yield();
    
    do
    {
        breakme();
        
    } while( workers_asleep != THREADS-1 ); 

    breakme();
    
// Suspend some of the threads, BREAKME();
//  +++ 1 running, + some sleeping, some suspended, *all* the same prio.

    for( i = 1; i < THREADS; i++ )
    {
        // suspend every 3rd thread
        if( 0 == (i % 3) ) cyg_thread_suspend( thread_handle[i] );
    }

    breakme();


// Change the prios all different, change my prio to highest , BREAKME();
//  +++ 1 running, + some sleeping, some suspended, different prios.

    cyg_thread_set_priority( thread_handle[0], CONTROLLER_PRI_HI );
        
    for( i = 1; i < THREADS; i++ )
    {
        pri = CONTROLLER_PRI_HI + 1 + i % WORKER_PRI_RANGE;
        
        cyg_thread_set_priority( thread_handle[i], pri );
    }

    breakme();
    
// Wake up all the threads, BREAKME();
//  +++ 1 running, + some ready, some suspended/ready, different prios.

    cyg_cond_broadcast( &worker_cv );

    breakme();


// Change my prio to lowest, let all the threads run, BREAKME().
//  +++ 1 running + some sleeping, some suspended/ready, different prios.

    cyg_thread_set_priority( thread_handle[0], CONTROLLER_PRI_LO );
    
    breakme();
    
// Resume all the threads, BREAKME();
//  +++ 1 running, + N ready, different prios.

    for( i = 1; i < THREADS; i++ )
    {
        cyg_thread_resume( thread_handle[i] );
    }
    
    breakme();
    
// Command some of the N threads to call BREAKME(); themselves (then sleep
// again).  Change my prio to low, so that they all get to run and hit the
// breakpoint.
//  +++ A different one running every time, others in a mixture of
//      ready and sleeping states.
    
    worker_state = WORKER_STATE_BREAK;

    cyg_cond_broadcast( &worker_cv );
    
    cyg_thread_set_priority( thread_handle[0], CONTROLLER_PRI_LO );
    
    breakme();
    
// Command all the threads to exit; switch my own priority to lower
// than theirs so that they all run and exit, then I get back in and
// BREAKME();
//  +++ 1 thread, running, + N dormant ones.

    worker_state = WORKER_STATE_EXIT;

    cyg_cond_broadcast( &worker_cv );
    
    breakme();

#if 0

// Cannot do this yet...
    
// Destroy some of the N threads to invalidate their IDs.  Re-create them
// with new IDs, so that we get IDs 1,2,4,6,8,11,12,13,14,15 in use instead
// of 1,2,3,4,5,6,7,8,9, if you see what I mean.  Do all the above again.
// Loop forever, or whatever...

#endif

    breakme();
    
    CYG_TEST_PASS_FINISH("GDB Thread test OK");
    
}

// -------------------------------------------------------------------------

externC void
cyg_start( void )
{

    CYG_TEST_INIT();
    
    cyg_thread_create(CONTROLLER_PRI_HI, controller, (cyg_addrword_t)0, "controller",
                      (void *)(&thread_stack[0]), STACKSIZE,
                      &thread_handle[0], &thread[0]);
    
    // resume it
    cyg_thread_resume(thread_handle[0]);

    
    // Get the world going
    cyg_scheduler_start();

    CYG_TEST_FAIL_FINISH("Not reached");
    
}

#else /* def CYGFUN_KERNEL_API_C */

externC void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_PASS_FINISH("Incorrect configuration for this test");
}
#endif /* def CYGFUN_KERNEL_API_C && ... */

// -------------------------------------------------------------------------
// EOF thread_gdb.c
