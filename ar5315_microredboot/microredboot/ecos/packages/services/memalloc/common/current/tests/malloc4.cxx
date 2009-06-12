//=================================================================
//
//        malloc4.cxx
//
//        Stress test malloc(), calloc(), realloc() and free()
//
//=================================================================
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
//=================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     jlarmour
// Contributors:  
// Date:          2000-05-30
// Description:   Contains a rigorous multithreaded test for malloc(),
//                calloc(), realloc() and free() functions
//
//
//####DESCRIPTIONEND####

// #define DEBUGTEST

// INCLUDES

#include <pkgconf/system.h>
#include <pkgconf/memalloc.h> // config header
#ifdef CYGPKG_ISOINFRA
# include <pkgconf/isoinfra.h>
# include <stdlib.h>
#endif
#ifdef CYGPKG_KERNEL
# include <pkgconf/kernel.h>
# include <cyg/kernel/thread.hxx>
# include <cyg/kernel/thread.inl>
# include <cyg/kernel/sched.hxx>
# include <cyg/kernel/sched.inl>
# include <cyg/kernel/sema.hxx>
#endif
#include <cyg/infra/testcase.h>

#if !defined(CYGPKG_KERNEL)
# define NA_MSG "Requires kernel"
#elif !defined(CYGFUN_KERNEL_THREADS_TIMER)
# define NA_MSG "Requires thread timers"
#elif !defined(CYGPKG_ISOINFRA)
# define NA_MSG "Requires isoinfra package"
#elif !CYGINT_ISO_MALLOC
# define NA_MSG "Requires malloc"
#elif !CYGINT_ISO_MALLINFO
# define NA_MSG "Requires mallinfo"
#elif !CYGINT_ISO_RAND
# define NA_MSG "Requires rand"
#elif defined(CYGIMP_MEMALLOC_MALLOC_DLMALLOC) && \
      !defined(CYGIMP_MEMALLOC_ALLOCATOR_DLMALLOC_THREADAWARE)
# define NA_MSG "Requires thread-safe dlmalloc"
#elif defined(CYGIMP_MEMALLOC_MALLOC_VARIABLE_SIMPLE) && \
      !defined(CYGSEM_MEMALLOC_ALLOCATOR_VARIABLE_THREADAWARE)
# define NA_MSG "Requires thread-safe variable block allocator"
#endif

#ifdef NA_MSG

externC void
cyg_start(void)
{
    CYG_TEST_INIT();
    CYG_TEST_NA( NA_MSG );
    CYG_TEST_FINISH("Done");
}
#else
//#define DEBUGTEST 1
#define NTHREADS 4
#include "testaux.hxx"

#include <cyg/infra/diag.h>

Cyg_Counting_Semaphore startsema;

volatile int stopnow = 0;

struct ptr {
    char* volatile p;
    volatile size_t size;
    volatile unsigned char busy;
};

#define STRINGIFY1( _x_ ) #_x_
#define STRINGIFY( _x_ ) STRINGIFY1( _x_ )

#define NUM_PTRS 100
#define WAITFORMEMDELAYMAX (cyg_test_is_simulator ? 1 : 3)
#define LOOPDELAYMAX       (cyg_test_is_simulator ? 1 : 3)
#define ITERATIONS         (cyg_test_is_simulator ? 10 : 200)
#define OUTPUTINTERVAL     (cyg_test_is_simulator ? 1 : 10)

int iterations = ITERATIONS;

static struct ptr ptrs[ NUM_PTRS ];

static __inline__ int
myrand(int limit, unsigned int *seed)
{
    int r;
    double l=(double)(limit+1);
    r=(int)( l*rand_r(seed) / (RAND_MAX+1.0) );
    return r;
}

size_t memsize;

static void
fill_with_data( struct ptr *p )
{
    unsigned int i, j;
    for (i=0; i < (p->size/4); i++)
        ((unsigned int *)p->p)[i] = (unsigned int)p;
    for ( j=i*4; j < p->size ; j++ )
        p->p[j] = ((char *)p)[j-i*4];
}

static void
check_data( struct ptr *p )
{
    unsigned int i, j;
    for (i=0; i < (p->size/4); i++)
        CYG_TEST_CHECK( ((unsigned int *)p->p)[i] == (unsigned int)p,
                        "Data didn't compare correctly");
    for ( j=i*4; j < p->size ; j++ )
        CYG_TEST_CHECK( p->p[j] == ((char *)p)[j-i*4],
                        "Data didn't compare correctly");
}

static void
check_zeroes( struct ptr *p )
{
    unsigned int i, j;
    for (i=0; i < (p->size/4); i++)
        CYG_TEST_CHECK( ((int *)p->p)[i] == 0,
                        "Zeroed data didn't compare correctly");
    for ( j=i*4; j < p->size ; j++ )
        CYG_TEST_CHECK( p->p[j] == 0,
                        "Zeroed data didn't compare correctly");
}


static void
thrmalloc( CYG_ADDRWORD data )
{
    int r, i;
    void *mem;
    unsigned int seed;

    startsema.wait();
        
    while (!stopnow) {
        r = myrand( NUM_PTRS-1, &seed );
        
        for (i=r+1; ; i++) {
            Cyg_Scheduler::lock();
            if (i == NUM_PTRS)
                i=0;
            if (!ptrs[i].busy && (ptrs[i].p == NULL) )
                break;
            Cyg_Scheduler::unlock();
            if ( i==r ) {
                Cyg_Thread::self()->delay( myrand(WAITFORMEMDELAYMAX, &seed) );
            }
        }
        ptrs[i].busy = 1;
        Cyg_Scheduler::unlock();
        r = myrand(memsize, &seed);
        mem = malloc(r);
        ptrs[i].p = (char *)mem;
        ptrs[i].size = r;
        if ( NULL != mem ) {
#ifdef DEBUGTEST
            diag_printf("malloc=%08x size=%d\n", mem, r);
#endif
            fill_with_data( &ptrs[i] );
        }
        ptrs[i].busy = 0;        
        Cyg_Thread::self()->delay( myrand(LOOPDELAYMAX, &seed) );
    }
}

static void
thrcalloc( CYG_ADDRWORD data )
{
    int r, i;
    void *mem;
    unsigned int seed;

    startsema.wait();
        
    while (!stopnow) {
        r = myrand( NUM_PTRS-1, &seed );
        
        for (i=r+1; ; i++) {
            Cyg_Scheduler::lock();
            if (i == NUM_PTRS)
                i=0;
            if (!ptrs[i].busy && (ptrs[i].p == NULL) )
                break;
            Cyg_Scheduler::unlock();
            if ( i==r ) {
                Cyg_Thread::self()->delay( myrand(WAITFORMEMDELAYMAX, &seed) );
            }
        }
        ptrs[i].busy = 1;
        Cyg_Scheduler::unlock();
        r = myrand(memsize, &seed);
        mem = calloc( 1, r );
        ptrs[i].p = (char *)mem;
        ptrs[i].size = r;
        if ( NULL != mem ) {
#ifdef DEBUGTEST
            diag_printf("calloc=%08x size=%d\n", mem, r);
#endif
            check_zeroes( &ptrs[i] );
            fill_with_data( &ptrs[i] );
        }
        ptrs[i].busy = 0;        
        Cyg_Thread::self()->delay( myrand(LOOPDELAYMAX, &seed) );
    }
}

static void
thrrealloc( CYG_ADDRWORD data )
{
    int r, i;
    void *mem;
    unsigned int seed;

    startsema.wait();
        
    while (!stopnow) {
        r = myrand( NUM_PTRS-1, &seed );
        
        for (i=r+1; ; i++) {
            Cyg_Scheduler::lock();
            if (i == NUM_PTRS)
                i=0;
            if (!ptrs[i].busy && (ptrs[i].p != NULL) )
                break;
            Cyg_Scheduler::unlock();
            if ( i==r ) {
                Cyg_Thread::self()->delay( myrand(WAITFORMEMDELAYMAX, &seed) );
            }
        }
        ptrs[i].busy = 1;
        Cyg_Scheduler::unlock();
        check_data( &ptrs[i] );
        r = myrand(memsize - 1, &seed) + 1;
        mem = realloc( (void *)ptrs[i].p, r );
        if ( NULL != mem ) {
#ifdef DEBUGTEST
            diag_printf("realloc=%08x oldsize=%d newsize=%d\n", mem, ptrs[i].size, r);
#endif
            ptrs[i].size = r;
            ptrs[i].p = (char *)mem;
            fill_with_data( &ptrs[i] );
        }
        ptrs[i].busy = 0;        
        Cyg_Thread::self()->delay( myrand(LOOPDELAYMAX, &seed) );
    }
}

static void
thrfree( CYG_ADDRWORD data )
{
    int r, i;
    int iter = 0;
    struct mallinfo minfo;
    unsigned int seed;

    minfo = mallinfo();
    memsize = (unsigned long) minfo.maxfree;
    diag_printf("INFO:<Iteration 0, arenasize=%d, space free=%d, maxfree=%d>\n",
                minfo.arena, minfo.fordblks, minfo.maxfree );

    // wake the three threads above.
    startsema.post(); startsema.post(); startsema.post();
        
    Cyg_Thread::self()->delay(1);

    while (1) {
        if ( (iter > 0) && (0 == (iter % OUTPUTINTERVAL)) ) {
            minfo = mallinfo();
            diag_printf("INFO:<Iteration %d, arenasize=%d, "
                        "space free=%d, maxfree=%d>\n",
                        iter, minfo.arena, minfo.fordblks, minfo.maxfree );
        }

        if ( iterations == iter++ )
            stopnow++;

        r = myrand( NUM_PTRS-1, &seed );
        
        for (i=r+1; ; i++) {
            Cyg_Scheduler::lock();
            if (i >= NUM_PTRS)
                i=0;
            if (!ptrs[i].busy && (ptrs[i].p != NULL) )
                break;
            Cyg_Scheduler::unlock();
            if ( i==r ) {
                if ( stopnow ) {
                    // we may have gone round all the ptrs even though one
                    // or more of them was busy, so check again just for that
                    int j;
                    for (j=0; j<NUM_PTRS; j++)
                        if (ptrs[j].busy)
                            break;
                    if ( j<NUM_PTRS )
                        continue;
                    struct mallinfo minfo;

                    minfo = mallinfo();
                    diag_printf("INFO:<Iteration %d, arenasize=%d, "
                                "space free=%d, maxfree=%d>\n",
                                iter, minfo.arena, minfo.fordblks,
                                minfo.maxfree );
                    CYG_TEST_PASS_FINISH("malloc4 test completed successfully");
                } else {
                    Cyg_Thread::self()->delay( 
                        myrand(WAITFORMEMDELAYMAX, &seed) );
                }
            }
        }
        ptrs[i].busy = 1;
        Cyg_Scheduler::unlock();
        check_data( &ptrs[i] );
#ifdef DEBUGTEST
        diag_printf("about to free %08x\n", ptrs[i].p);
#endif
        free( (void *)ptrs[i].p );
        ptrs[i].p = NULL;
        ptrs[i].busy = 0;
        Cyg_Thread::self()->delay( myrand(LOOPDELAYMAX, &seed) );
    }
}


externC void
cyg_start(void)
{
#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
    cyg_hal_invoke_constructors();
#endif
    CYG_TEST_INIT();
    CYG_TEST_INFO("Starting malloc4 test");

    new_thread(thrmalloc, 0);
    new_thread(thrcalloc, 1);
    new_thread(thrrealloc, 2);
    new_thread(thrfree, 3);

    Cyg_Scheduler::start();

    CYG_TEST_FAIL_FINISH("Not reached");
} // cyg_start()

#endif // !NA_MSG

// EOF malloc4.cxx
