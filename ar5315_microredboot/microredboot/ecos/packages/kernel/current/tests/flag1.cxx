//==========================================================================
//
//        flag1.cxx
//
//        Flag test 1
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
// Author:        dsm
// Contributors:    dsm
// Date:          1998-05-11
// Description:   Tests basic flag functionality.
//####DESCRIPTIONEND####

#include <pkgconf/kernel.h>

#include <cyg/kernel/thread.hxx>        // Cyg_Thread
#include <cyg/kernel/thread.inl>
#include <cyg/kernel/sched.hxx>         // Cyg_Scheduler::start()

#include <cyg/kernel/flag.hxx>

#include <cyg/infra/testcase.h>

#include <cyg/kernel/sched.inl>


#define NTHREADS 3
#include "testaux.hxx"

static Cyg_Flag f0, f1;
#ifdef CYGFUN_KERNEL_THREADS_TIMER
static Cyg_Flag f2;
#endif

static volatile cyg_atomic q = 0;
#define FIRST_THREAD_WAIT_TIME   5
#define SECOND_THREAD_WAIT_TIME 10
#define THIRD_THREAD_WAIT_TIME  20

static void entry0( CYG_ADDRWORD data )
{
    CYG_TEST_INFO("Testing setbits() and maskbits()");
    CYG_TEST_CHECK(0==f0.peek(), "flag not initialized properly");
    f0.setbits(0x1);
    CYG_TEST_CHECK(1==f0.peek(), "setbits");
    f0.setbits(0x3);
    CYG_TEST_CHECK(3==f0.peek(), "setbits");
    f0.maskbits(~0x5);
    CYG_TEST_CHECK(2==f0.peek(), "maskbits");
    f0.setbits();
    CYG_TEST_CHECK(~0u==f0.peek(), "setbits no arg");
    f0.maskbits();
    CYG_TEST_CHECK(0==f0.peek(), "maskbits no arg");
    CYG_TEST_CHECK(0==q++, "bad synchronization");

    CYG_TEST_INFO("Testing wait()");
    f1.setbits(0x4);
    CYG_TEST_CHECK(0x4==f1.peek(), "maskbits no arg");
    CYG_TEST_CHECK(1==q++, "bad synchronization");
    f1.setbits(0x18);                   // wake t1
    f1.wait(0x11, Cyg_Flag::AND | Cyg_Flag::CLR);
    CYG_TEST_CHECK(0==f1.peek(), "flag value wrong");
    CYG_TEST_CHECK(3==q++, "bad synchronization");
    f0.setbits(0x2);                    // wake t1
    f1.wait(0x10, Cyg_Flag::AND );
    f0.setbits(0x1);                    // wake t1

    f1.wait(0x11, Cyg_Flag::AND | Cyg_Flag::CLR);

#ifdef CYGFUN_KERNEL_THREADS_TIMER
    f2.wait(0x2, Cyg_Flag::OR);    
    CYG_TEST_CHECK(20==q,"bad synchronization");
    f2.wait(0x10, Cyg_Flag::AND, 
            Cyg_Clock::real_time_clock->current_value()+THIRD_THREAD_WAIT_TIME);
    CYG_TEST_CHECK(21==q++,"bad synchronization");
#endif
    f0.wait(1, Cyg_Flag::OR);

    CYG_TEST_FAIL_FINISH("Not reached");
}

static void entry1( CYG_ADDRWORD data )
{
    f1.wait(0xc, Cyg_Flag::AND);
    CYG_TEST_CHECK(2==q++, "bad synchronization");
    CYG_TEST_CHECK(0x1c==f1.peek(), "flag value wrong");
    f1.setbits(0x1);                    // wake t0
    f0.wait(0x3, Cyg_Flag::OR);
    CYG_TEST_CHECK(4==q++, "bad synchronization");
    CYG_TEST_CHECK(2==f0.peek(), "flag value wrong");
    
    f1.setbits(0xf0);                   // wake t0,t2
    f0.wait(0x5, Cyg_Flag::AND | Cyg_Flag::CLR); // wait for t0 & t2
    CYG_TEST_CHECK(0==f0.peek(), "flag value wrong");
    CYG_TEST_CHECK(0xf0==f1.peek(), "flag value wrong");
    CYG_TEST_CHECK(5==q++, "bad synchronization");
    f1.maskbits();
    CYG_TEST_CHECK(0==f1.peek(), "flag value wrong");
    
    CYG_TEST_INFO("Testing poll()");
    f0.setbits(0x55);
    CYG_TEST_CHECK(0x55==f0.peek(), "flag value wrong");
    CYG_TEST_CHECK(0x55==f0.poll(0x3, Cyg_Flag::OR),"bad poll() return");
    CYG_TEST_CHECK(0==f0.poll(0xf, Cyg_Flag::AND),"poll()");
    CYG_TEST_CHECK(0==f0.poll(0xa, Cyg_Flag::OR),"poll()");
    CYG_TEST_CHECK(0x55==f0.peek(), "flag value wrong");
    CYG_TEST_CHECK(0x55==f0.poll(0xf, Cyg_Flag::OR | Cyg_Flag::CLR),"poll");
    CYG_TEST_CHECK(0x0==f0.peek(), "flag value wrong");
    f0.setbits(0x50);
    CYG_TEST_CHECK(0x50==f0.poll(0x10, Cyg_Flag::AND | Cyg_Flag::CLR),"poll");
    CYG_TEST_CHECK(0x0==f0.peek(), "flag value wrong");

    CYG_TEST_INFO("Testing waiting()");
    f0.maskbits();
    CYG_TEST_CHECK(!f0.waiting(), "waiting()");

#ifdef CYGFUN_KERNEL_THREADS_TIMER
    thread[1]->delay( 10 ); // allow other threads to reach wait on f1
    CYG_TEST_CHECK(f1.waiting(), "waiting() not true");
    f1.setbits();                       // wake one of t0,t2
    CYG_TEST_CHECK(f1.waiting(), "waiting() not true");
#else
    f1.setbits(0x11);                   // wake one of t0,t2
#endif
    f1.setbits(0x11);                   // wake other of t0,t2    
    CYG_TEST_CHECK(!f1.waiting(), "waiting not false");

#ifdef CYGFUN_KERNEL_THREADS_TIMER
    CYG_TEST_INFO("Testing wait() with timeout");
    q=20;
    f2.setbits(0x2);                    // synchronize with t0,t2
    CYG_TEST_CHECK(20==q,"bad synchronization");
    f2.wait(0x20, Cyg_Flag::AND,
            Cyg_Clock::real_time_clock->current_value()+SECOND_THREAD_WAIT_TIME);
    CYG_TEST_CHECK(22==q++,"bad synchronization");
#endif

    CYG_TEST_PASS_FINISH("Flag 1 OK");
}

static void entry2( CYG_ADDRWORD data )
{
    f1.wait(0x60, Cyg_Flag::OR);
    f0.setbits(0x4);

    f1.wait(0x11, Cyg_Flag::AND | Cyg_Flag::CLR);
#ifdef CYGFUN_KERNEL_THREADS_TIMER
    f2.wait(0x2, Cyg_Flag::OR);
    CYG_TEST_CHECK(20==q,"bad synchronization");
    CYG_TEST_CHECK(0==f2.wait(0x40, Cyg_Flag::AND,
                              Cyg_Clock::real_time_clock->current_value()+FIRST_THREAD_WAIT_TIME),
                   "timed wait() wrong");
    CYG_TEST_CHECK(20==q++,"bad synchronization");
    // Now wake t0 before it times out
    f2.setbits(0x10);
#endif
    f0.wait(1, Cyg_Flag::OR);    
    
    CYG_TEST_FAIL_FINISH("Not reached");
}

void flag1_main( void )
{
    CYG_TEST_INIT();

    new_thread(entry0, 0);
    new_thread(entry1, 1);
    new_thread(entry2, 2);

    Cyg_Scheduler::start();

    CYG_TEST_FAIL_FINISH("Not reached");
}

externC void
cyg_start( void )
{ 
#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
    cyg_hal_invoke_constructors();
#endif
    flag1_main();
}

// EOF flag1.cxx
