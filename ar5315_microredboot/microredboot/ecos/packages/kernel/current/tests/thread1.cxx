//==========================================================================
//
//        thread1.cxx
//
//        Thread test 1
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
// Date:          1998-02-11
// Description:   Tests some basic thread functions.
// Omissions:     Cyg_ThreadTimer
//                Cyg_Thread
//                  exit -- not necessarily called
//                  yield
//                  set_priority
//                  get_priority
//                  get/set_sleep_reason
//                  get/set_wake_reason
//                  set/clear_timer
//                Cyg_ThreadQueue
//               
//####DESCRIPTIONEND####

#include <pkgconf/kernel.h>

#include <cyg/kernel/sched.hxx>
#include <cyg/kernel/thread.hxx>

#include <cyg/infra/testcase.h>

#ifdef CYGFUN_KERNEL_THREADS_TIMER

#include <cyg/kernel/sched.inl>
#include <cyg/kernel/thread.inl>

#include "testaux.hxx"

#ifdef CYGNUM_HAL_STACK_SIZE_TYPICAL
#define STACKSIZE CYGNUM_HAL_STACK_SIZE_TYPICAL
#else
#define STACKSIZE 2000
#endif

static char stack[2][STACKSIZE];

static char thread[2][sizeof(Cyg_Thread)];

static Cyg_Thread *pt0,*pt1;
static cyg_uint16 uid0,uid1;


static void entry0( CYG_ADDRWORD data )
{
    CHECK( 222 == data );

    uid0 = pt0->get_unique_id();

    pt1->suspend();       
    pt1->resume();

    do {
        pt0->delay(1);
    } while( Cyg_Thread::RUNNING == pt1->get_state() );
    
    CHECK( Cyg_Thread::SLEEPING == pt1->get_state() );

    pt1->wake();

    CHECK( uid0 != uid1 );

    CYG_TEST_PASS_FINISH("Thread 1 OK");
}

static void entry1( CYG_ADDRWORD data )
{
    CHECK( 333 == data );

    uid1 = pt1->get_unique_id();

    Cyg_Thread *self = Cyg_Thread::self();
   
    CHECK( self == pt1 );

    pt1->sleep();
    pt1->suspend();

    Cyg_Thread::exit();         // no guarantee this will be called
}

void thread1_main( void )
{
    CYG_TEST_INIT();

    pt0 = new((void *)&thread[0])
            Cyg_Thread(CYG_SCHED_DEFAULT_INFO,
                       entry0, 222, 
                       "thread 0",
                       (CYG_ADDRESS)stack[0], STACKSIZE );
    pt1 = new((void *)&thread[1])
            Cyg_Thread(CYG_SCHED_DEFAULT_INFO,
                       entry1, 333, 
                       "thread 1",
                       (CYG_ADDRESS)stack[1], STACKSIZE );

    CYG_ASSERTCLASS( pt0, "error" );
    CYG_ASSERTCLASS( pt1, "error" );

    pt0->resume();
    pt1->resume();

    Cyg_Scheduler::start();

    CYG_TEST_FAIL_FINISH("Not reached");
}

externC void
cyg_start( void )
{
#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
    cyg_hal_invoke_constructors();
#endif
    thread1_main();
}

#else // ifdef CYGFUN_KERNEL_THREADS_TIMER

externC void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_NA("Kernel threads timer disabled");
}

#endif // ifdef CYGFUN_KERNEL_THREADS_TIMER

// EOF thread1.cxx
