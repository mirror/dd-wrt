//==========================================================================
//
//        sched1.cxx
//
//        Sched test 1
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
// Date:          1998-02-26
// Description:   Tests some basic sched functions.
// Omissions:
//     Doesn't test Cyg_Scheduler::get_thread_switches() very well
//     Cyg_SchedThread
//         inherit_priority
//         disinherit_priority
// Options:       
//####DESCRIPTIONEND####

#include <pkgconf/kernel.h>

#include <cyg/kernel/sched.hxx>        // Cyg_Scheduler::start()
#include <cyg/kernel/thread.hxx>       // Cyg_Thread

#include <cyg/infra/testcase.h>

#include <cyg/kernel/sched.inl>
#include <cyg/kernel/thread.inl>

#define NTHREADS 2

#include "testaux.hxx"

static void entry0( CYG_ADDRWORD data )
{
    CHECK( 0 == Cyg_Scheduler::get_sched_lock() );
    Cyg_Scheduler::lock(); {
        CHECK( 1 == Cyg_Scheduler::get_sched_lock() );
        Cyg_Scheduler::lock(); {
            CHECK( 2 == Cyg_Scheduler::get_sched_lock() );
        } Cyg_Scheduler::unlock();
    } Cyg_Scheduler::unlock();
    cyg_ucount32 t0=Cyg_Scheduler::get_thread_switches();
    cyg_ucount32 t1=Cyg_Scheduler::get_thread_switches();
    CHECK( t1 >= t0 );
    CHECK( Cyg_Scheduler::get_current_thread() == 
                Cyg_Thread::self() );
    CYG_TEST_PASS_FINISH( "Sched 1 OK");
}

static void entry1( CYG_ADDRWORD data )
{
    Cyg_Thread::self()->sleep();    
}

void sched1_main(void)
{
    CYG_TEST_INIT();

    new_thread(entry0, 222);
    new_thread(entry1, 333);

    Cyg_Scheduler::start();
    
    CYG_TEST_FAIL_FINISH("Not reached");
}

externC void
cyg_start( void )
{
#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
    cyg_hal_invoke_constructors();
#endif
    sched1_main();
}
// EOF sched1.cxx
