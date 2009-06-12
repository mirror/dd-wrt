//==========================================================================
//
//        bin_sem3.cxx
//
//        Binary semaphore test 3
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
// Author(s):     David Brennan
// Contributors:  David Brennan
// Date:          2003-06-06
// Description:   Tests basic binary semaphore timeout functionality.
//####DESCRIPTIONEND####

#include <pkgconf/kernel.h>

#include <cyg/kernel/sched.hxx>        // Cyg_Scheduler::start()
#include <cyg/kernel/thread.hxx>       // Cyg_Thread
#include <cyg/kernel/thread.inl>

#include <cyg/kernel/sema.hxx>

#include <cyg/infra/testcase.h>

#include <cyg/kernel/sched.inl>

#define NTHREADS 2

#include "testaux.hxx"

static Cyg_Binary_Semaphore s0(true), s1(false), s2;

static volatile cyg_ucount8 q = 0;

static void entry0( CYG_ADDRWORD data )
{
    s0.wait();
    CHECK( 0 == q++ );
    s1.post();
    s0.wait();
    CHECK( 2 == q++ );
    CHECK( ! s0.posted() );
#ifndef CYGFUN_KERNEL_THREADS_TIMER
    CHECK( ! s0.trywait() );
#else // !CYGFUN_KERNEL_THREADS_TIMER
    CHECK( ! s0.wait(10) );
#endif // !CYGFUN_KERNEL_THREADS_TIMER
    s0.post();
    CHECK( 3 == q++ );
    CHECK( s0.posted() );
    s1.post();
    CHECK( ! s2.posted() );
    s2.wait();
    CHECK( 5 == q++ );
    CYG_TEST_PASS_FINISH("Binary Semaphore 3 OK");
}

static void entry1( CYG_ADDRWORD data )
{
    CHECK( s1.posted() );
    s1.wait();
    CHECK( 1 == q++ );
    CHECK( ! s0.posted() );
    s0.post();
    s1.wait();
    CHECK( 4 == q++ );
    CHECK( s0.posted() );
#ifndef CYGFUN_KERNEL_THREADS_TIMER
    CHECK( s0.trywait() );
#else // !CYGFUN_KERNEL_THREADS_TIMER
    CHECK( s0.wait(10) );
#endif // !CYGFUN_KERNEL_THREADS_TIMER
    CHECK( ! s0.posted() );
    s2.post();
    s0.wait();
    CYG_TEST_FAIL_FINISH("Not reached");
}

void bin_sem3_main( void )
{
    CYG_TEST_INIT();

    new_thread( entry0, 0);
    new_thread( entry1, 1);
    
#ifdef CYGIMP_THREAD_PRIORITY
    thread[0]->set_priority( 4 );
    thread[1]->set_priority( 5 ); // make sure the threads execute as intended
#endif

    Cyg_Scheduler::start();
    
    CYG_TEST_FAIL_FINISH("Not reached");
}

externC void
cyg_start( void )
{ 
#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
    cyg_hal_invoke_constructors();
#endif
    bin_sem3_main();
}
// EOF bin_sem1.cxx
