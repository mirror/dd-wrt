//==========================================================================
//
//        bin_sem2.cxx
//
//        Binary semaphore test 2
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
// Author(s):     nickg,dsm
// Contributors:    dsm
// Date:          1998-03-10
// Description:
//     Dining philosophers test.  Based on philo.cxx
//####DESCRIPTIONEND####

#include <pkgconf/kernel.h>

#include <cyg/kernel/sched.hxx>
#include <cyg/kernel/thread.hxx>
#include <cyg/kernel/thread.inl>
#include <cyg/kernel/mutex.hxx>

#include <cyg/kernel/sema.hxx>

#include <cyg/infra/testcase.h>

#include <cyg/kernel/sched.inl>

static cyg_ucount16 PHILO_LOOPS = 1000;

#define PHILOSOPHERS 15
#define NTHREADS PHILOSOPHERS
#include "testaux.hxx"

static Cyg_Binary_Semaphore chopstick[PHILOSOPHERS];

static char pstate[PHILOSOPHERS+1];       // state of each philosopher

static cyg_ucount16 state_changes = 0;
// state_changes keep track of number of changes to pstate so
// we can exit after we've seen enough.


static Cyg_Mutex pstate_mutex;
static Cyg_Mutex cycle_mutex;

static inline int left(cyg_count8 i)
{
    return (0 == i) ? PHILOSOPHERS-1 : i-1 ;
}
static inline int right(cyg_count8 i)
{
    return (PHILOSOPHERS == i+1) ? 0 : i+1 ;
}

void change_state(int id, char newstate)
{
    if (PHILO_LOOPS == state_changes++) 
        CYG_TEST_PASS_FINISH("Binary Semaphore 2 OK");
    

    pstate_mutex.lock(); {
        pstate[id] = newstate;
        bool all_hungry = true;         // until proved otherwise
        for(cyg_ucount8 i=0; i < PHILOSOPHERS; i++) {
            if('E' == pstate[i]) {
                CHECK('E' != pstate[left(i)]);
                CHECK('E' != pstate[right(i)]);
            }
            if('H' != pstate[i]) {
                all_hungry = false;
            }
        }
        // Theoretically it is possible for all the philosophers to be
        // hungry but not waiting on semaphores.  But in practice this
        // means something is wrong.
        CHECK(false == all_hungry);        
    } pstate_mutex.unlock();
}

char get_state(int id)
{
    pstate_mutex.lock();
    
    char s = pstate[id];

    pstate_mutex.unlock();

    return s;
}

// -------------------------------------------------------------------------
// Thread to behave like a philosopher

void Philosopher( CYG_ADDRESS id )
{
    Cyg_Thread *self = Cyg_Thread::self();
    Cyg_Binary_Semaphore *first_stick = &chopstick[id];
    Cyg_Binary_Semaphore *second_stick = &chopstick[(id+1)%PHILOSOPHERS];
    
    CHECK( id >= 0 && id < PHILOSOPHERS);

    // Deadlock avoidance. The easiest way to make the philosophers
    // behave is to make each pick up the lowest numbered stick
    // first. This is how it works out anyway for all the philosophers
    // except the last, who must have his sticks swapped.
    
    if( id == PHILOSOPHERS-1 )
    {
        Cyg_Binary_Semaphore *t = first_stick;
        first_stick = second_stick;
        second_stick = t;
    }
    
    
    // The following variable is shared by all philosophers.
    // It is incremented unprotected, but this does not matter
    // since it is only present to introduce a little variability
    // into the think and eat times.
    
    static int cycle = 0;
    
    for(;;)
    {
        // Think for a bit

        self->delay((id+cycle++)%12);    // Cogito ergo sum...

        // I am now hungry, try to get the chopsticks
        change_state(id,'H');

        // Get the sticks
        first_stick->wait();
        second_stick->wait();

        // Got them, now eat
        change_state(id,'E');
                
        // Check that the world is as I think it is...
        CYG_TEST_CHECK( !first_stick->posted(),
                        "Not got first stick");
        CYG_TEST_CHECK( !second_stick->posted(),
                        "Not got second stick");
        CYG_TEST_CHECK( get_state(left(id)) != 'E',
                        "Left neighbour also eating!!");
        CYG_TEST_CHECK( get_state(right(id)) != 'E',
                        "Right neighbour also eating!!");
        
        self->delay((id+cycle++)%6);    // munch munch

        // Finished eating, put down sticks.

        change_state(id,'T');   

        // put sticks back on table
        first_stick->post();
        second_stick->post();
    }
}

// -------------------------------------------------------------------------

void bin_sem2_main( void )
{
    CYG_TEST_INIT();

    if (cyg_test_is_simulator)
        PHILO_LOOPS = 100;

    for( int i = 0; i < PHILOSOPHERS; i++ )
    {
        pstate[i] = 'T';            // starting state
        new_thread(Philosopher, i);

        // make the matching chopstick present
        chopstick[i].post();
    }
    
    Cyg_Scheduler::scheduler.start();
}

externC void
cyg_start( void )
{ 
#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
    cyg_hal_invoke_constructors();
#endif
    bin_sem2_main();
}
// EOF bin_sem2.cxx
