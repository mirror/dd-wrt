//==========================================================================
//
//        release.cxx
//
//        Thread release test
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
// Contributors:    nickg
// Date:          1998-04-24
// Description:   Tests the functionality of thread release().
//####DESCRIPTIONEND####

#include <pkgconf/kernel.h>

#include <cyg/kernel/thread.hxx>
#include <cyg/kernel/thread.inl>
#include <cyg/kernel/sched.hxx>
#include <cyg/kernel/mutex.hxx>
#include <cyg/kernel/sema.hxx>

#include <cyg/infra/testcase.h>

#include <cyg/kernel/sched.inl>

#define NTHREADS 2

#include "testaux.hxx"

static Cyg_Binary_Semaphore s0, s1;

static void entry0( CYG_ADDRWORD data )
{
    Cyg_Thread *self = Cyg_Thread::self();
    
    if (!s0.wait() )
    {
        if( self->get_wake_reason() != Cyg_Thread::BREAK )
            CYG_TEST_FAIL_FINISH("Wake reason not BREAK");
    }
    else
    {
            CYG_TEST_FAIL_FINISH("Thread not released");        
    }

    s1.post();
    
    self->exit();
}


static void entry1( CYG_ADDRWORD data )
{
    Cyg_Thread *self = Cyg_Thread::self();    

    // Give the other thread a chance to wait in SMP systems.
    for( int i = 0; i < 100; i++ )
        self->yield();
    
    thread[0]->release();

    s1.wait();
    
    CYG_TEST_PASS_FINISH("Release OK");
    
    Cyg_Thread::self()->exit();
}

void release_main(void)
{
    CYG_TEST_INIT();

    new_thread( entry0, 0);
    new_thread( entry1, 1);

    thread[0]->set_priority(5);
    thread[1]->set_priority(6);

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
   
// EOF release.cxx
