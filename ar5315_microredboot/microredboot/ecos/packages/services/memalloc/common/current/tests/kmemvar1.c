/*==========================================================================
//
//        kmemvar1.cxx
//
//        Kernel C API Variable memory pool test 1
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
// Contributors:  dsm
// Date:          1998-06-08
// Description:   Tests basic variable memory pool functionality
//####DESCRIPTIONEND####
*/

#include <pkgconf/memalloc.h>

#include <cyg/infra/testcase.h>

#ifdef CYGFUN_MEMALLOC_KAPI

#include <cyg/hal/hal_arch.h>           // CYGNUM_HAL_STACK_SIZE_TYPICAL

#include <pkgconf/kernel.h>

#include <cyg/kernel/kapi.h>

#define NTHREADS 2
#define STACKSIZE CYGNUM_HAL_STACK_SIZE_TYPICAL

static cyg_handle_t thread[NTHREADS];

static cyg_thread thread_obj[NTHREADS];
static char stack[NTHREADS][STACKSIZE];


#define MEMSIZE 10240

static cyg_uint8 mem[2][MEMSIZE];

static cyg_mempool_var mempool_obj[2];
static cyg_handle_t mempool0, mempool1;

static void check_in_mp0(cyg_uint8 *p, cyg_int32 size)
{
    CYG_TEST_CHECK(NULL != p,
                   "Allocation failed");
    CYG_TEST_CHECK(mem[0] <= p && p+size < mem[1],
                   "Block outside memory pool");
}

static void entry0( cyg_addrword_t data )
{
    cyg_uint8 *p0, *p1;
    cyg_int32 most_of_mem=MEMSIZE/4*3;
    cyg_mempool_info info0, info1, info2;
    
    cyg_mempool_var_get_info(mempool0, &info0);
    
    CYG_TEST_CHECK(mem[0] == info0.base, "get_arena: base wrong");
    CYG_TEST_CHECK(MEMSIZE == info0.size, "get_arena: size wrong");

    CYG_TEST_CHECK(0 < info0.maxfree && info0.maxfree <= info0.size,
                   "get_arena: maxfree wildly wrong");
    
    CYG_TEST_CHECK(-1 == info0.blocksize, "get_blocksize wrong" );

    CYG_TEST_CHECK(info0.totalmem > 0, "Negative total memory" );
    CYG_TEST_CHECK(info0.freemem > 0, "Negative free memory" );
    CYG_TEST_CHECK(info0.totalmem <= MEMSIZE, 
                   "info.totalsize: Too much memory");
    CYG_TEST_CHECK(info0.freemem <= info0.totalmem ,
                   "More memory free than possible" );

    CYG_TEST_CHECK( !cyg_mempool_var_waiting(mempool0),
                    "Thread waiting for memory; there shouldn't be");
    
    CYG_TEST_CHECK( NULL == cyg_mempool_var_try_alloc(mempool0, MEMSIZE+1),
                    "Managed to allocate too much memory");
    
#ifdef CYGSEM_MEMALLOC_ALLOCATOR_VARIABLE_THREADAWARE
    p0 = cyg_mempool_var_alloc(mempool0, most_of_mem);
    check_in_mp0(p0, most_of_mem);

    cyg_mempool_var_get_info(mempool0, &info1);

    CYG_TEST_CHECK(info1.freemem > 0, "Negative free memory" );
    CYG_TEST_CHECK(info1.freemem < info0.freemem,
                   "Free memory didn't decrease after allocation" );

    CYG_TEST_CHECK( NULL == cyg_mempool_var_try_alloc(mempool0, most_of_mem),
                    "Managed to allocate too much memory");
    
    cyg_mempool_var_free(mempool0, p0);

    cyg_mempool_var_get_info(mempool0, &info2);
    CYG_TEST_CHECK(info2.freemem > info1.freemem,
                   "Free memory didn't increase after free" );
#endif
    
    // should be able to reallocate now memory is free
    p0 = cyg_mempool_var_try_alloc(mempool0, most_of_mem);
    check_in_mp0(p0, most_of_mem);

    p1 = cyg_mempool_var_try_alloc(mempool0, 10);
    check_in_mp0(p1, 10);
    
    CYG_TEST_CHECK(p1+10 <= p0 || p1 >= p0+MEMSIZE,
                   "Ranges of allocated memory overlap");

    cyg_mempool_var_free(mempool0, p0);
    cyg_mempool_var_free(mempool0, p1);
    
#ifdef CYGSEM_MEMALLOC_ALLOCATOR_VARIABLE_THREADAWARE
# ifdef CYGFUN_KERNEL_THREADS_TIMER
    // This shouldn't have to wait
    p0 = cyg_mempool_var_timed_alloc(mempool0, most_of_mem,
                                     cyg_current_time()+100000);
    check_in_mp0(p0, most_of_mem);
    p1 = cyg_mempool_var_timed_alloc(mempool0, most_of_mem,
                                     cyg_current_time()+2);
    CYG_TEST_CHECK(NULL == p1, "Timed alloc unexpectedly worked");
    p1 = cyg_mempool_var_timed_alloc(mempool0, 10,
                                     cyg_current_time()+2);
    check_in_mp0(p1, 10);
    
    // Expect thread 1 to have run while processing previous timed
    // allocation.  It should therefore tbe waiting.
    CYG_TEST_CHECK(cyg_mempool_var_waiting(mempool1), "There should be a thread waiting");
# endif
#endif
    
    CYG_TEST_PASS_FINISH("Kernel C API Variable memory pool 1 OK");
}

#ifdef CYGSEM_MEMALLOC_ALLOCATOR_VARIABLE_THREADAWARE
static void entry1( cyg_addrword_t data )
{
    cyg_mempool_var_alloc(mempool1, MEMSIZE+1);
    CYG_TEST_FAIL("Oversized alloc returned");
}
#endif


void kmemvar1_main( void )
{
    CYG_TEST_INIT();
    CYG_TEST_INFO("Starting Kernel C API Variable memory pool 1 test");

    cyg_thread_create(4, entry0 , (cyg_addrword_t)0, "kmemvar1-0",
        (void *)stack[0], STACKSIZE, &thread[0], &thread_obj[0]);
    cyg_thread_resume(thread[0]);

#ifdef CYGSEM_MEMALLOC_ALLOCATOR_VARIABLE_THREADAWARE
    cyg_thread_create(4, entry1 , (cyg_addrword_t)1, "kmemvar1-1",
        (void *)stack[1], STACKSIZE, &thread[1], &thread_obj[1]);
    cyg_thread_resume(thread[1]);
#endif

    cyg_mempool_var_create(mem[0], MEMSIZE, &mempool0, &mempool_obj[0]);
    cyg_mempool_var_create(mem[1], MEMSIZE, &mempool1, &mempool_obj[1]);

    cyg_scheduler_start();

    CYG_TEST_FAIL_FINISH("Not reached");
}

externC void
cyg_start( void )
{ 
    kmemvar1_main();
}

#else /* ifdef CYGFUN_MEMALLOC_KAPI */
externC void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_NA("Kernel C API layer disabled");
}
#endif /* ifdef CYGFUN_MEMALLOC_KAPI */

/* EOF kmemvar1.c */
