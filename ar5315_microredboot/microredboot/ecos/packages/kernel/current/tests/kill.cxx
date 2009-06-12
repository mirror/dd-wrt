//==========================================================================
//
//        kill.cxx
//
//        Thread kill test
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
// Date:          1998-04-24
// Description:   Tests the functionality of thread kill() and
//                reinitalize().
//####DESCRIPTIONEND####

#include <pkgconf/kernel.h>

#include <cyg/kernel/thread.hxx>
#include <cyg/kernel/thread.inl>
#include <cyg/kernel/sched.hxx>
#include <cyg/kernel/mutex.hxx>
#include <cyg/kernel/sema.hxx>

#include <cyg/infra/testcase.h>

#ifdef CYGFUN_KERNEL_THREADS_TIMER

#include <cyg/kernel/sched.inl>

#define NTHREADS 3

#include "testaux.hxx"

// In general, this delay has to be long enough to account for slow targets
// and potential problems on e.g. the linux synthetic target to avoid
// potential problems due to timing inaccuracy and scheduling of Linux
// tasks. It is decreased further below for simulators.
int delay_ticks = 5;


static Cyg_Binary_Semaphore s0, s1;

volatile cyg_atomic thread0_state;
volatile cyg_atomic thread1_state;
volatile cyg_atomic thread2_state;

static void entry0( CYG_ADDRWORD data )
{
    Cyg_Thread *self = Cyg_Thread::self();

    thread0_state = 1;
    
    s0.wait();

    thread0_state = 2;
    
    CYG_TEST_FAIL_FINISH("Thread not killed");        

    self->exit();
}


static void entry1( CYG_ADDRWORD data )
{
    Cyg_Thread *self = Cyg_Thread::self();

    thread1_state = 1;
    
    self->delay(delay_ticks);

    if( thread2_state != 1 )
        CYG_TEST_FAIL_FINISH("Thread2 in wrong state");        
    
    thread1_state = 2;
    
    thread[0]->kill();

    thread1_state = 3;    
    
    thread[2]->kill();

    thread1_state = 4;
    
    self->delay(delay_ticks);

    thread1_state = 5;
    thread2_state = 0;
    
    thread[2]->reinitialize();
    thread[2]->resume();

    self->delay(delay_ticks);

    if( thread2_state != 1 )
        CYG_TEST_FAIL_FINISH("Thread2 in wrong state");        
    
    thread1_state = 6;

    self->delay(delay_ticks);

    if( thread2_state != 2 )
        CYG_TEST_FAIL_FINISH("Thread2 in wrong state");        
    
    thread[2]->kill();

    thread1_state = 7;
    
    CYG_TEST_PASS_FINISH("Kill OK");
    
    Cyg_Thread::self()->exit();
}

static void entry2( CYG_ADDRWORD data )
{
    thread2_state = 1;

    while( thread1_state != 6 ) continue;

    thread2_state = 2;
    
    for(;;) continue;
    
}

void release_main(void)
{
    CYG_TEST_INIT();

    if (cyg_test_is_simulator)
        delay_ticks = 2;

    new_thread( entry0, 0);
    new_thread( entry1, 1);
    new_thread( entry2, 2);

    thread[0]->set_priority(5);
    thread[1]->set_priority(6);
    thread[2]->set_priority(7);

    Cyg_Scheduler::start();

    CYG_TEST_FAIL_FINISH("Not reached");
}

externC void
cyg_start( void )
{
#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
    cyg_hal_invoke_constructors();
#endif
    release_main();
}

#else // ifdef CYGFUN_KERNEL_THREADS_TIMER

externC void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_NA("Kernel threads timer disabled");
}

#endif // ifdef CYGFUN_KERNEL_THREADS_TIMER
   
// EOF kill.cxx
