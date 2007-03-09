//==========================================================================
//
//        memfix2.cxx
//
//        Fixed memory pool test 2
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
// Description:   test allocation and freeing in fixed memory pools
//####DESCRIPTIONEND####

#include <pkgconf/memalloc.h>
#include <pkgconf/system.h>

#ifdef CYGPKG_KERNEL
#include <pkgconf/kernel.h>

#include <cyg/kernel/sched.hxx>         // Cyg_Scheduler::start()
#include <cyg/kernel/thread.hxx>        // Cyg_Thread
#include <cyg/kernel/thread.inl>
#include <cyg/kernel/sema.hxx>

#include <cyg/kernel/sched.inl>


#define NTHREADS 1
#include "testaux.hxx"

#endif

#include <cyg/memalloc/memfixed.hxx>

#include <cyg/infra/testcase.h>

static const cyg_int32 memsize = 1024;

static cyg_uint8 mem[memsize];

#define NUM_PTRS 16                     // Should be even
#define BLOCKSIZE 12

static Cyg_Mempool_Fixed mempool(mem, memsize, BLOCKSIZE);

static cyg_uint8 *ptr[NUM_PTRS];

// We make a number of passes over a table of pointers which point to
// blocks of allocated memory.  The block is freed and a new block
// allocated.  The order of the processing of blocks is varied.
static void entry( CYG_ADDRWORD data )
{
    for(cyg_ucount32 passes = 0; passes < 10; passes++) {


        // The order which the table is processed varies according to
        // stepsize.
        cyg_ucount8 stepsize = (passes*2 + 1) % NUM_PTRS; // odd
        

        for(cyg_ucount8 c=0, i=0; c < NUM_PTRS; c++) {
            i = (i+stepsize) % NUM_PTRS;
            if(ptr[i]) {
                for(cyg_ucount32 j=BLOCKSIZE;j--;) {
                    CYG_TEST_CHECK(ptr[i][j]==i, "Memory corrupted");
                }
                CYG_TEST_CHECK(mempool.free(ptr[i]), "bad free");
            }
            ptr[i] = mempool.try_alloc();

            CYG_TEST_CHECK(NULL != ptr[i], "Memory pool not big enough");
            CYG_TEST_CHECK(mem<=ptr[i] && ptr[i]+BLOCKSIZE < mem+memsize,
                           "Allocated region not within pool");
            
            // Scribble over memory to check whether region overlaps
            // with other regions.  The contents of the memory are
            // checked on freeing.  This also tests that the memory
            // does not overlap with allocator memory structures.
            for(cyg_ucount32 j=BLOCKSIZE;j--;) {
                ptr[i][j]=i;
            }
        }
    }
    
    CYG_TEST_PASS_FINISH("Fixed memory pool 2 OK");
}


void memfix2_main( void )
{
    CYG_TEST_INIT();
    CYG_TEST_INFO("Starting Fixed memory pool 2 test");

    for(cyg_ucount32 i = 0; i<NUM_PTRS; i++) {
        ptr[i]      = NULL;
    }

#ifdef CYGPKG_KERNEL
    new_thread(entry, 0);
    Cyg_Scheduler::start();
#else
    entry(0);
#endif

    CYG_TEST_FAIL_FINISH("Not reached");
}

externC void
cyg_start( void )
{ 
#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
    cyg_hal_invoke_constructors();
#endif
    memfix2_main();
}
// EOF memfix2.cxx
