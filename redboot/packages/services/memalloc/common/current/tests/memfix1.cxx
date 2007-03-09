//==========================================================================
//
//        memfix1.cxx
//
//        Fixed memory pool test 1
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
// Author(s):     dsm, jlarmour
// Contributors:   
// Date:          2000-06-18
// Description:   Tests basic fixed memory pool functionality
//####DESCRIPTIONEND####

#include <pkgconf/memalloc.h>
#include <pkgconf/system.h>

#ifdef CYGPKG_KERNEL
#include <pkgconf/kernel.h>

#include <cyg/kernel/sched.hxx>        // Cyg_Scheduler::start()
#include <cyg/kernel/thread.hxx>       // Cyg_Thread

#include <cyg/kernel/sched.inl>
#include <cyg/kernel/thread.inl>

#include <cyg/kernel/timer.hxx>         // Cyg_Timer
#include <cyg/kernel/clock.inl>         // Cyg_Clock

#define NTHREADS 2
#include "testaux.hxx"

#endif

#include <cyg/memalloc/memfixed.hxx>

#include <cyg/infra/testcase.h>

static const cyg_int32 memsize = 10240;

static cyg_uint8 mem[2][memsize];

static Cyg_Mempool_Fixed mempool0(mem[0], memsize, 100);

static Cyg_Mempool_Fixed mempool1(mem[1], memsize, 316);


static void check_in_mp0(cyg_uint8 *p, cyg_int32 size)
{
    CYG_TEST_CHECK(NULL != p,
                   "Allocation failed");
    CYG_TEST_CHECK(mem[0] <= p && p+size < mem[1],
                   "Block outside memory pool");
}


static void entry0( CYG_ADDRWORD data )
{
    cyg_int32 f0,f1,f2,t0;
    cyg_uint8 *p0, *p1, *p2;
    Cyg_Mempool_Status stat;
    
    mempool0.get_status( CYG_MEMPOOL_STAT_ORIGBASE|
                         CYG_MEMPOOL_STAT_BLOCKSIZE|
                         CYG_MEMPOOL_STAT_ORIGSIZE, stat );
    CYG_TEST_CHECK(mem[0] == stat.origbase, "get_status: base wrong");
    CYG_TEST_CHECK(memsize == stat.origsize, "get_status: size wrong");
    CYG_TEST_CHECK(100 == stat.blocksize, "get_status: blocksize wrong");
    
    mempool1.get_status( CYG_MEMPOOL_STAT_BLOCKSIZE, stat );
    CYG_TEST_CHECK(316 == stat.blocksize, "get_status: pool1 blocksize wrong" );

    mempool0.get_status( CYG_MEMPOOL_STAT_TOTALFREE|
                         CYG_MEMPOOL_STAT_ARENASIZE, stat );
    t0 = stat.arenasize;
    CYG_TEST_CHECK(t0 > 0, "Negative total memory" );
    f0 = stat.totalfree;
    CYG_TEST_CHECK(f0 > 0, "Negative free memory" );
    CYG_TEST_CHECK(t0 <= memsize, "get_totalsize: Too much memory");
    CYG_TEST_CHECK(f0 <= t0 , "More memory free than possible" );

    mempool0.get_status( CYG_MEMPOOL_STAT_WAITING, stat );
    CYG_TEST_CHECK( !stat.waiting,
                    "Thread waiting for memory; there shouldn't be");
    
#ifdef CYGSEM_MEMALLOC_ALLOCATOR_FIXED_THREADAWARE
    p0 = mempool0.alloc();
#else
    p0 = mempool0.try_alloc();
#endif
    check_in_mp0(p0, 100);

    mempool0.get_status( CYG_MEMPOOL_STAT_TOTALFREE, stat );
    f1 = stat.totalfree;
    CYG_TEST_CHECK(f1 > 0, "Negative free memory" );
    CYG_TEST_CHECK(f1 < f0, "Free memory didn't decrease after allocation" );

    p1 = NULL;
    while((p2 = mempool0.try_alloc()))
        p1 = p2;
    
    mempool0.get_status( CYG_MEMPOOL_STAT_TOTALFREE, stat );
    f1 = stat.totalfree;
    CYG_TEST_CHECK(mempool0.free(p0), "Couldn't free");

    mempool0.get_status( CYG_MEMPOOL_STAT_TOTALFREE, stat );
    f2 = stat.totalfree;
    CYG_TEST_CHECK(f2 > f1, "Free memory didn't increase after free" );
    
    // should be able to reallocate now a block is free
    p0 = mempool0.try_alloc();
    check_in_mp0(p0, 100);

    CYG_TEST_CHECK(p1+100 <= p0 || p1 >= p0+100,
                   "Ranges of allocated memory overlap");

    CYG_TEST_CHECK(mempool0.free(p0), "Couldn't free");
    CYG_TEST_CHECK(mempool0.free(p1), "Couldn't free");
    
#ifdef CYGSEM_MEMALLOC_ALLOCATOR_FIXED_THREADAWARE
# ifdef CYGFUN_KERNEL_THREADS_TIMER
    // This shouldn't have to wait
    p0 = mempool0.alloc(Cyg_Clock::real_time_clock->current_value()+100000);
    check_in_mp0(p0, 100);
    p1 = mempool0.alloc(Cyg_Clock::real_time_clock->current_value()+20);
    check_in_mp0(p1, 10);
    p1 = mempool0.alloc(Cyg_Clock::real_time_clock->current_value()+20);
    CYG_TEST_CHECK(NULL == p1, "Timed alloc unexpectedly worked");
    
    // Expect thread 1 to have run while processing previous timed
    // allocation.  It should therefore tbe waiting.
    mempool1.get_status( CYG_MEMPOOL_STAT_WAITING, stat );
    CYG_TEST_CHECK(stat.waiting, "There should be a thread waiting");
# endif
#endif
    
    CYG_TEST_PASS_FINISH("Fixed memory pool 1 OK");
}

#ifdef CYGSEM_MEMALLOC_ALLOCATOR_FIXED_THREADAWARE
static void entry1( CYG_ADDRWORD data )
{
    while(NULL != mempool1.alloc())
        ;
    CYG_TEST_FAIL("alloc returned NULL");
}
#endif


void memfix1_main( void )
{
    CYG_TEST_INIT();
    CYG_TEST_INFO("Starting Fixed memory pool 1 test");

#ifdef CYGSEM_MEMALLOC_ALLOCATOR_FIXED_THREADAWARE
    new_thread(entry0, 0);
    new_thread(entry1, 1);

    Cyg_Scheduler::start();
#elif defined(CYGPKG_KERNEL)
    new_thread(entry0, 0);

    Cyg_Scheduler::start();
#else
    entry0(0);
#endif

    CYG_TEST_FAIL_FINISH("Not reached");
}

externC void
cyg_start( void )
{ 
#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
    cyg_hal_invoke_constructors();
#endif
    memfix1_main();
}
// EOF memfix1.cxx
