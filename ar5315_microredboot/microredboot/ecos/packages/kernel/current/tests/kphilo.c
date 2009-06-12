//==========================================================================
//
//        kphilo.c
//
//        A test of the dining philosophers problem
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
// Date:          1998-02-24
// Description:   A test of the dining philosophers problem
//####DESCRIPTIONEND####
// 

#include <cyg/kernel/kapi.h>

#include <cyg/infra/cyg_ass.h>
#include <cyg/kernel/diag.h>

// -------------------------------------------------------------------------
// Data for the philosophers problem

#define PHILOSOPHERS    15               // number of philosophers
#define STACKSIZE       (2*1024)        // size of thread stack

// array of stacks for philosopher threads
char thread_stack[PHILOSOPHERS][STACKSIZE];

// array of threads.
cyg_thread thread[PHILOSOPHERS];

cyg_handle_t thread_handle[PHILOSOPHERS];

// array of chopsticks
cyg_sem_t chopstick[PHILOSOPHERS];

cyg_ucount32 data_index;

// -------------------------------------------------------------------------
// State recording and display

static char pstate[PHILOSOPHERS+1];     // state vector showing what each
                                        // philosopher is doing

cyg_mutex_t state_mutex;

#ifdef CYG_HAL_MN10300_MN103002
static cyg_count8 eaters = 0;
#endif

void change_state(int id, char newstate)
{
    cyg_mutex_lock(&state_mutex);

#ifdef CYG_HAL_MN10300_MN103002
    if( pstate[id] == 'E' ) eaters--;
    if( newstate == 'E' ) eaters++;
//    led(eaters);
#endif
    
    pstate[id] = newstate;

    diag_write_string(pstate);
#if 0
    diag_write_char(' ');
    diag_write_dec(Cyg_Scheduler::get_thread_switches());
#endif    
    diag_write_char('\n');

    cyg_mutex_unlock(&state_mutex);
    
}

char get_state( int id)
{
    char s;
    cyg_mutex_lock(&state_mutex);
    
    s = pstate[id];

    cyg_mutex_unlock(&state_mutex);

    return s;
}

// -------------------------------------------------------------------------
// Thread to behave like a philosopher

void Philosopher( cyg_addrword_t vid )
{
    cyg_uint32 id = (cyg_uint32)vid;
    cyg_sem_t *first_stick = &chopstick[id];
    cyg_sem_t *second_stick = &chopstick[(id+1)%PHILOSOPHERS];
#ifdef CYGPKG_INFRA_DEBUG
    int left_philo = ((id==0)?PHILOSOPHERS:id)-1;
    int right_philo = (id==PHILOSOPHERS-1)?0:(id+1);
#endif
    
    CYG_ASSERT( id >= 0 && id < PHILOSOPHERS, "Bad id");

    // Deadlock avoidance. The easiest way to make the philosophers
    // behave is to make each pick up the lowest numbered stick
    // first. This is how it works out anyway for all the philosophers
    // except the last, who must have his sticks swapped.
    
    if( id == PHILOSOPHERS-1 )
    {
        cyg_sem_t *t = first_stick;
        first_stick = second_stick;
        second_stick = t;
    }
    
    for(;;)
    {
        cyg_ucount32 val;
        
        // The following variable is shared by all philosophers.
        // It is incremented unprotected, but this does not matter
        // since it is only present to introduce a little variability
        // into the think and eat times.
    
        static volatile int cycle = 0;
    
        // Think for a bit

        cyg_thread_delay((id+cycle++)%12);    // Cogito ergo sum...

        // I am now hungry, try to get the chopsticks

        change_state(id,'H');
        
        // Get the first stick
        cyg_semaphore_wait(first_stick);
    
        // Get the second stick
        cyg_semaphore_wait(second_stick);
    
        // Got them, now eat

        change_state(id,'E');
                
        // Check that the world is as I think it is...
        cyg_semaphore_peek( first_stick, &val);
        CYG_ASSERT( val == 0, "Not got first stick");
        cyg_semaphore_peek( second_stick, &val);
        CYG_ASSERT( val == 0, "Not got second stick");
        CYG_ASSERT( get_state(left_philo) != 'E', "Left neighbour also eating!!");
        CYG_ASSERT( get_state(right_philo) != 'E', "Right neighbour also eating!!");
        
        cyg_thread_delay((id+cycle++)%6);    // munch munch

        // Finished eating, put down sticks.

        change_state(id,'T');   

        cyg_semaphore_post( first_stick );
        cyg_semaphore_post( second_stick );

    }
}

// -------------------------------------------------------------------------

externC void
cyg_start( void )
{
    int i;
    
    diag_init();

    diag_write_string("Philosophers\n");
    diag_write_string("Started\n");

    // Zero last element in state so it acts like
    // a string.
    pstate[PHILOSOPHERS] = 0;

#if 1
    for( i = 0; i < PHILOSOPHERS; i++ )
    {
        change_state(i,'T');            // starting state

        cyg_thread_create(4, Philosopher, (cyg_addrword_t)i, "philosopher",
            (void *)(&thread_stack[i]), STACKSIZE,
            &thread_handle[i], &thread[i]);

        // resume it
        cyg_thread_resume(thread_handle[i]);

        // and make the matching chopstick present
        cyg_semaphore_init( &chopstick[i], 1);
    }
#endif
    
    // Get the world going
    cyg_scheduler_start();

}

// -------------------------------------------------------------------------
// EOF kphilo.c
