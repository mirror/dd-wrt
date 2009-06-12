/*=================================================================
//
//        kcache2.c
//
//        Cache feature/timing tests
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Gary Thomas
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
// Author(s):     jskov, based on kcache1.c by dsm
// Contributors:  jskov, gthomas
// Date:          1998-12-10
// Description:   Tests some of the more exotic cache macros.
//####DESCRIPTIONEND####
*/

#include <cyg/hal/hal_arch.h>           // CYGNUM_HAL_STACK_SIZE_TYPICAL

#include <cyg/kernel/kapi.h>

#include <cyg/infra/testcase.h>

#ifdef CYGVAR_KERNEL_COUNTERS_CLOCK
#ifdef CYGFUN_KERNEL_API_C

#include <cyg/infra/diag.h>
#include <cyg/hal/hal_cache.h>

// -------------------------------------------------------------------------

#define NTHREADS 1
#define STACKSIZE CYGNUM_HAL_STACK_SIZE_TYPICAL

// The following are defaults for loop variables. Note they will be overriden
// on simulator targets, where detected - there is no point testing a cache
// which doesn't exist :-).

#define TEST_DZERO_LOOPS 5000  // default number of loops for test_dzero()
#define TIME_ILOCK_LOOPS 10000 // default number of loops for time_ilock()
#define TIME_DLOCK_LOOPS 10000 // default number of loops for time_dlock()

// Define this to enable a simple, but hopefully useful, data cache
// test.  It may help discover if the cache support has been defined
// properly (in terms of size and shape)
#ifdef HAL_DCACHE_LINE_SIZE
#define _TEST_DCACHE_OPERATION
#endif

static cyg_handle_t thread[NTHREADS];

static cyg_thread thread_obj[NTHREADS];
static char stack[NTHREADS][STACKSIZE];

#define MAXSIZE 1<<18

volatile char m[MAXSIZE];

// -------------------------------------------------------------------------
// Test of data cache zero.
//  o Timing comparison with instructions doing the same amount of work.
//  o Check that area cleared with the DCACHE_ZERO macro contains zeros.
#ifdef HAL_DCACHE_ZERO
static void test_dzero(void)
{
    register cyg_uint32 k, i;
    cyg_tick_count_t count0, count1;
    cyg_ucount32 t;
    volatile cyg_uint32* aligned_p;
    volatile cyg_uint32* p;
    register CYG_INTERRUPT_STATE oldints;
    cyg_ucount32 test_dzero_loops = TEST_DZERO_LOOPS;

    CYG_TEST_INFO("Data cache zero");

    if (cyg_test_is_simulator)
        test_dzero_loops=10;

    aligned_p =  (volatile cyg_uint32*) 
        (((unsigned long) &m[HAL_DCACHE_LINE_SIZE*2]) 
         & ~(HAL_DCACHE_LINE_SIZE-1));

    // Time with conventional instructions.
    HAL_DISABLE_INTERRUPTS(oldints);
    HAL_DCACHE_SYNC();
    HAL_DCACHE_DISABLE();
    HAL_DCACHE_SYNC();
    HAL_DCACHE_INVALIDATE_ALL();
    HAL_DCACHE_ENABLE();
    HAL_RESTORE_INTERRUPTS(oldints);
    count0 = cyg_current_time();
    for (k = 0; k < test_dzero_loops; k++) {
        p = aligned_p;
        for (i = 0; i < HAL_DCACHE_SETS; i++) {
#if (16 == HAL_DCACHE_LINE_SIZE)
            *p++ = 0;
            *p++ = 0;
            *p++ = 0;
            *p++ = 0;
#elif (32 == HAL_DCACHE_LINE_SIZE)
            *p++ = 0;
            *p++ = 0;
            *p++ = 0;
            *p++ = 0;
            *p++ = 0;
            *p++ = 0;
            *p++ = 0;
            *p++ = 0;
#else
#error "Not defined for this cache line size."
#endif
        }

        HAL_DISABLE_INTERRUPTS(oldints);
        HAL_DCACHE_SYNC();
        HAL_DCACHE_DISABLE();
        HAL_DCACHE_SYNC();
        HAL_DCACHE_INVALIDATE_ALL();
        HAL_DCACHE_ENABLE();
        HAL_RESTORE_INTERRUPTS(oldints);
    }
    count1 = cyg_current_time();
    t = count1 - count0;
    diag_printf("time with instructions:    %d\n", t);

    // Initialize the area with non-zero so we can check whether
    // the macro cleared the area properly.
    p = aligned_p;
    for (i = 0; 
         i < HAL_DCACHE_SETS*HAL_DCACHE_LINE_SIZE/sizeof(cyg_uint32); 
         i++) {
        *p++ = 0xdeadbeef;
    }

    // Time with DCACHE_ZERO.
    HAL_DISABLE_INTERRUPTS(oldints);
    HAL_DCACHE_SYNC();
    HAL_DCACHE_DISABLE();
    HAL_DCACHE_SYNC();
    HAL_DCACHE_INVALIDATE_ALL();
    HAL_DCACHE_ENABLE();
    HAL_RESTORE_INTERRUPTS(oldints);
    count0 = cyg_current_time();
    for (k = 0; k < test_dzero_loops; k++) {
        HAL_DCACHE_ZERO(aligned_p, HAL_DCACHE_SETS*HAL_DCACHE_LINE_SIZE);

        HAL_DISABLE_INTERRUPTS(oldints);
        HAL_DCACHE_SYNC();
        HAL_DCACHE_DISABLE();
        HAL_DCACHE_SYNC();
        HAL_DCACHE_INVALIDATE_ALL();
        HAL_DCACHE_ENABLE();
        HAL_RESTORE_INTERRUPTS(oldints);
    }
    count1 = cyg_current_time();
    t = count1 - count0;
    diag_printf("time with HAL_DCACHE_ZERO: %d\n", t);

    // Verify that the area was actually cleared.
    {
        cyg_uint32 d;

        d = 0;
        p = aligned_p;
        for (i = 0; 
             i < HAL_DCACHE_SETS*HAL_DCACHE_LINE_SIZE/sizeof(cyg_uint32); 
             i++) {
            d |= *p++;
        }

        CYG_TEST_CHECK(0 == d, "region not properly cleared");
    }

}
#endif

// -------------------------------------------------------------------------
// Test of data cache write hint.
// Just check that the macro compiles.
#ifdef HAL_DCACHE_WRITE_HINT
static void test_dwrite_hint(void)
{
    register cyg_uint32 k;
    register CYG_INTERRUPT_STATE oldints;

    CYG_TEST_INFO("Data cache write hint");

    HAL_DISABLE_INTERRUPTS(oldints);
    HAL_DCACHE_SYNC();
    HAL_DCACHE_DISABLE();
    HAL_DCACHE_SYNC();
    HAL_DCACHE_INVALIDATE_ALL();
    HAL_DCACHE_ENABLE();
    HAL_RESTORE_INTERRUPTS(oldints);

    HAL_DCACHE_WRITE_HINT(&m[HAL_DCACHE_LINE_SIZE*2], 2*HAL_DCACHE_LINE_SIZE);
    for (k = 0; k < 20; k++);
    m[HAL_DCACHE_LINE_SIZE*2] = 42;
}
#endif

// -------------------------------------------------------------------------
// Test of data cache read hint.
// Just check that the macro compiles.
#ifdef HAL_DCACHE_READ_HINT
static void test_dread_hint(void)
{
    register char c;
    register cyg_uint32 k;
    register CYG_INTERRUPT_STATE oldints;

    CYG_TEST_INFO("Data cache read hint");

    HAL_DISABLE_INTERRUPTS(oldints);
    HAL_DCACHE_SYNC();
    HAL_DCACHE_DISABLE();
    HAL_DCACHE_SYNC();
    HAL_DCACHE_INVALIDATE_ALL();
    HAL_DCACHE_ENABLE();
    HAL_RESTORE_INTERRUPTS(oldints);

    HAL_DCACHE_READ_HINT(&m[HAL_DCACHE_LINE_SIZE*2], 2*HAL_DCACHE_LINE_SIZE);
    for (k = 0; k < 20; k++);
    c = m[HAL_DCACHE_LINE_SIZE*2];
}
#endif

// -------------------------------------------------------------------------
// Test of data cache line store
//  o No semantic requirement.
//  o Check that flushed data is written to memory.
//  o Simple invocation check of macro.
#ifdef HAL_DCACHE_STORE
static void test_dstore(void)
{
    volatile cyg_uint8* aligned_p;
    cyg_int32 i;
    register CYG_INTERRUPT_STATE oldints;

    CYG_TEST_INFO("Data cache store region");

    for (i = 0; i < HAL_DCACHE_LINE_SIZE*16; i++)
        m[i] = 0;
    
    HAL_DISABLE_INTERRUPTS(oldints);
    HAL_DCACHE_SYNC();
    HAL_DCACHE_DISABLE();
    HAL_DCACHE_SYNC();
    HAL_DCACHE_INVALIDATE_ALL();
    HAL_DCACHE_ENABLE();

    aligned_p =  (volatile cyg_uint8*) 
        (((unsigned long) &m[HAL_DCACHE_LINE_SIZE*2]) 
         & ~(HAL_DCACHE_LINE_SIZE-1));

    aligned_p[0] = 42 + aligned_p[1]; // Load causes cache to be used!

    HAL_DCACHE_STORE(aligned_p, HAL_DCACHE_LINE_SIZE);

    CYG_TEST_CHECK(42 == aligned_p[0],
                   "memory didn't contain flushed data");

    HAL_DCACHE_INVALIDATE_ALL(); // Discard...

    CYG_TEST_CHECK(42 == aligned_p[0],
                   "memory didn't contain flushed data after invalidate all");

    HAL_RESTORE_INTERRUPTS(oldints);
}
#endif

// -------------------------------------------------------------------------
// Test of data cache total flush (sync).
//  o No semantic requirement.
//  o Check that flushed data is written to memory.
//  o Simple invocation check of macro.
#ifdef HAL_DCACHE_LINE_SIZE // So we can find our way around memory

#ifdef _TEST_DCACHE_OPERATION
static void
test_dcache_operation(void)
{
    long *lp = (long *)m;
    int i, errs;
    cyg_uint32 oldints;

    CYG_TEST_INFO("Data cache basic");

    HAL_DISABLE_INTERRUPTS(oldints);
    HAL_DCACHE_SYNC();
    HAL_DCACHE_DISABLE();
    HAL_DCACHE_SYNC();
    // Fill test buffer
    for (i = 0;  i < sizeof(m)/sizeof(*lp);  i++) {
        lp[i] = i;
    }
    HAL_DCACHE_INVALIDATE_ALL();
    HAL_DCACHE_ENABLE();
    // Now push data through the cache
    // Note: 256 seems like a reasonable offset.  It may be useful to actually
    // compute this (and the size of the test region) based on cache geometry
    for (i = 256;  i < 256+HAL_DCACHE_SIZE/sizeof(*lp);  i++) {
        lp[i] = 0xFF000000 + i;
    }
    // Now force cache clean and off
    HAL_DCACHE_SYNC();
    HAL_DCACHE_DISABLE();
    // Verify the data
    diag_printf("Verify data with cache off\n");
    errs = 0;
    for (i = 0;  i < sizeof(m)/sizeof(*lp);  i++) {
        if ((i >= 256) && (i < 256+HAL_DCACHE_SIZE/sizeof(*lp))) {
            if (lp[i] != (0xFF000000 + i)) {
                if (++errs < 16) {
                    diag_printf("Data inside test range changed - was: %x, is %x, index: %x\n",
                                0xFF000000+i, lp[i], i);
                }
            }
        } else {
            if (lp[i] != i) {
                if (++errs < 16) {
                    diag_printf("Data outside test range changed - was: %x, is %x, index: %x\n",
                                i, lp[i], i);
                }
            }
        }
    }
    CYG_TEST_CHECK(0 == errs, "dcache basic failed");
#if 0 // Additional information
    diag_printf("%d total errors during compare\n", errs);
    diag_dump_buf(&lp[240], 128);
#endif
    HAL_RESTORE_INTERRUPTS(oldints);
}
#endif

static void test_dsync(void)
{
    volatile cyg_uint8* aligned_p;
    cyg_int32 i;
    register CYG_INTERRUPT_STATE oldints;

    CYG_TEST_INFO("Data cache sync all");

    for (i = 0; i < HAL_DCACHE_LINE_SIZE*16; i++)
        m[i] = 0;
    
    HAL_DISABLE_INTERRUPTS(oldints);
    HAL_DCACHE_SYNC();
    HAL_DCACHE_DISABLE();
    HAL_DCACHE_SYNC();
    HAL_DCACHE_INVALIDATE_ALL();
    HAL_DCACHE_ENABLE();

    aligned_p =  (volatile cyg_uint8*) 
        (((unsigned long) &m[HAL_DCACHE_LINE_SIZE*2]) 
         & ~(HAL_DCACHE_LINE_SIZE-1));

    aligned_p[0] = 42 + aligned_p[1]; // Load causes cache to be used!
    aligned_p[HAL_DCACHE_LINE_SIZE] = 43 + aligned_p[HAL_DCACHE_LINE_SIZE + 1];

    HAL_DCACHE_SYNC();

    CYG_TEST_CHECK(42 == aligned_p[0],
                   "memory didn't contain flushed data");
    CYG_TEST_CHECK(43 == aligned_p[HAL_DCACHE_LINE_SIZE], 
                   "memory didn't contain flushed data next block");

    HAL_DCACHE_INVALIDATE_ALL();

    CYG_TEST_CHECK(42 == aligned_p[0],
                   "memory didn't contain flushed data after invalidate");
    CYG_TEST_CHECK(43 == aligned_p[HAL_DCACHE_LINE_SIZE], 
                   "memory didn't contain flushed data next block after invalidate");

    HAL_RESTORE_INTERRUPTS(oldints);

    HAL_ICACHE_INVALIDATE_ALL();    
    HAL_DCACHE_DISABLE();

    CYG_TEST_CHECK(42 == aligned_p[0],
                   "memory didn't contain flushed data after disable");
    CYG_TEST_CHECK(43 == aligned_p[HAL_DCACHE_LINE_SIZE], 
                   "memory didn't contain flushed data next block after disable");

    HAL_DCACHE_ENABLE();
}
#endif // HAL_DCACHE_LINE_SIZE

// -------------------------------------------------------------------------
// Test of data cache line flush.
//  o Requires write-back cache.
//  o Check that flushed data is written to memory.
//  o Simple range check of macro.
#ifdef HAL_DCACHE_QUERY_WRITE_MODE // only if we know this, can we test:
#ifdef HAL_DCACHE_FLUSH
static void test_dflush(void)
{
    volatile cyg_uint8* aligned_p;
    cyg_int32 i;
    register CYG_INTERRUPT_STATE oldints;

    CYG_TEST_INFO("Data cache flush region");

    for (i = 0; i < HAL_DCACHE_LINE_SIZE*16; i++)
        m[i] = 0;
    
    HAL_DISABLE_INTERRUPTS(oldints);
    HAL_DCACHE_SYNC();
    HAL_DCACHE_DISABLE();
    HAL_DCACHE_SYNC();
    HAL_DCACHE_INVALIDATE_ALL();
    HAL_DCACHE_ENABLE();
    HAL_RESTORE_INTERRUPTS(oldints);

    aligned_p =  (volatile cyg_uint8*) 
        (((unsigned long) &m[HAL_DCACHE_LINE_SIZE*2]) 
         & ~(HAL_DCACHE_LINE_SIZE-1));

    HAL_DISABLE_INTERRUPTS(oldints);

    aligned_p[0] = 42 + aligned_p[1]; // Load causes cache to be used!
    aligned_p[HAL_DCACHE_LINE_SIZE] = 43 + aligned_p[HAL_DCACHE_LINE_SIZE + 1];

    HAL_DCACHE_FLUSH(aligned_p, HAL_DCACHE_LINE_SIZE);

    HAL_DCACHE_DISABLE();

    HAL_RESTORE_INTERRUPTS(oldints);

    CYG_TEST_CHECK(42 == aligned_p[0],
                   "memory didn't contain flushed data");
    CYG_TEST_CHECK(0 == aligned_p[HAL_DCACHE_LINE_SIZE], 
                   "flushed beyond region");

    HAL_DCACHE_ENABLE();
}
#endif
#endif

// -------------------------------------------------------------------------
// Test of data cache disable (which does NOT force contents out to RAM)
//  o Requires write-back cache [so NOT invoked unconditionally]
//  o Check that dirty data is not written to memory and is invalidated
//    in the cache.
//  o Simple invocation check of macro.
#ifdef HAL_DCACHE_QUERY_WRITE_MODE // only if we know this, can we test:
static void test_ddisable(void)
{
    volatile cyg_uint8* aligned_p;
    cyg_int32 i;
    register CYG_INTERRUPT_STATE oldints;

    CYG_TEST_INFO("Data cache gross disable");

    for (i = 0; i < HAL_DCACHE_LINE_SIZE*16; i++)
        m[i] = 0;
    
    HAL_DISABLE_INTERRUPTS(oldints);
    HAL_DCACHE_SYNC();
    HAL_DCACHE_DISABLE();
    HAL_DCACHE_SYNC();
    HAL_DCACHE_INVALIDATE_ALL();
    HAL_DCACHE_ENABLE();
    HAL_RESTORE_INTERRUPTS(oldints);

    aligned_p =  (volatile cyg_uint8*) 
        (((unsigned long) &m[HAL_DCACHE_LINE_SIZE*2]) 
         & ~(HAL_DCACHE_LINE_SIZE-1));

    HAL_DISABLE_INTERRUPTS(oldints);

    aligned_p[0] = 43 + aligned_p[1]; // Load causes cache to be used!
    aligned_p[HAL_DCACHE_LINE_SIZE-1] = 43;

    aligned_p[HAL_DCACHE_LINE_SIZE] = 42 + aligned_p[HAL_DCACHE_LINE_SIZE + 1];

    HAL_DCACHE_DISABLE();

    HAL_RESTORE_INTERRUPTS(oldints);

    CYG_TEST_CHECK(0 == aligned_p[0] &&
                   0 == aligned_p[HAL_DCACHE_LINE_SIZE-1],
                   "cache/memory contained invalidated data");
    CYG_TEST_CHECK(0 == aligned_p[HAL_DCACHE_LINE_SIZE],
                   "next block contained invalidated data");

    HAL_DCACHE_ENABLE();
}
#endif // def HAL_DCACHE_QUERY_WRITE_MODE

// -------------------------------------------------------------------------
// Test of data cache total invalidate.
//  o Requires write-back cache.
//  o Check that invalidated data is not written to memory and is invalidated
//    in the cache.
//  o Simple invocation check of macro.
#ifdef HAL_DCACHE_QUERY_WRITE_MODE // only if we know this, can we test:
#ifdef HAL_DCACHE_INVALIDATE_ALL
static void test_dinvalidate_all(void)
{
    volatile cyg_uint8* aligned_p;
    cyg_int32 i;
    register CYG_INTERRUPT_STATE oldints;

    CYG_TEST_INFO("Data cache invalidate all");

    for (i = 0; i < HAL_DCACHE_LINE_SIZE*16; i++)
        m[i] = 0;
    
    HAL_DISABLE_INTERRUPTS(oldints);
    HAL_DCACHE_SYNC();
    HAL_DCACHE_DISABLE();
    HAL_DCACHE_SYNC();
    HAL_DCACHE_INVALIDATE_ALL();
    HAL_DCACHE_ENABLE();

    aligned_p =  (volatile cyg_uint8*) 
        (((unsigned long) &m[HAL_DCACHE_LINE_SIZE*2]) 
         & ~(HAL_DCACHE_LINE_SIZE-1));

    aligned_p[0] = 43 + aligned_p[1]; // Load causes cache to be used!
    aligned_p[HAL_DCACHE_LINE_SIZE-1] = 43;

    aligned_p[HAL_DCACHE_LINE_SIZE] = 42 + aligned_p[HAL_DCACHE_LINE_SIZE + 1];

    HAL_DCACHE_INVALIDATE_ALL();

    HAL_RESTORE_INTERRUPTS(oldints);

    CYG_TEST_CHECK(0 == aligned_p[0] &&
                   0 == aligned_p[HAL_DCACHE_LINE_SIZE-1],
                   "cache/memory contained invalidated data");
    CYG_TEST_CHECK(0 == aligned_p[HAL_DCACHE_LINE_SIZE],
                   "next block contained invalidated data");
}
#endif
#endif // def HAL_DCACHE_QUERY_WRITE_MODE

// -------------------------------------------------------------------------
// Test of data cache line invalidate.
//  o Requires write-back cache.
//  o Check that invalidated data is not written to memory and is invalidated
//    in the cache.
//  o Simple range check of macro.
#ifdef HAL_DCACHE_QUERY_WRITE_MODE // only if we know this, can we test:
#ifdef HAL_DCACHE_INVALIDATE
static void test_dinvalidate(void)
{
    volatile cyg_uint8* aligned_p;
    cyg_int32 i;
    register CYG_INTERRUPT_STATE oldints;

    CYG_TEST_INFO("Data cache invalidate region");

    for (i = 0; i < HAL_DCACHE_LINE_SIZE*16; i++)
        m[i] = 0;
    
    HAL_DISABLE_INTERRUPTS(oldints);
    HAL_DCACHE_SYNC();
    HAL_DCACHE_DISABLE();
    HAL_DCACHE_SYNC();
    HAL_DCACHE_INVALIDATE_ALL();
    HAL_DCACHE_ENABLE();
    HAL_RESTORE_INTERRUPTS(oldints);

    aligned_p =  (volatile cyg_uint8*) 
        (((unsigned long) &m[HAL_DCACHE_LINE_SIZE*2]) 
         & ~(HAL_DCACHE_LINE_SIZE-1));

    HAL_DISABLE_INTERRUPTS(oldints);

    aligned_p[0] = 43 + aligned_p[1]; // Load causes cache to be used!
    aligned_p[HAL_DCACHE_LINE_SIZE-1] = 43;

    aligned_p[HAL_DCACHE_LINE_SIZE] = 42 + aligned_p[HAL_DCACHE_LINE_SIZE + 1];

    HAL_DCACHE_INVALIDATE(aligned_p, HAL_DCACHE_LINE_SIZE);

    HAL_RESTORE_INTERRUPTS(oldints);

    CYG_TEST_CHECK(0 == aligned_p[0] &&
                   0 == aligned_p[HAL_DCACHE_LINE_SIZE-1],
                   "cache/memory contained invalidated data");
    CYG_TEST_CHECK(42 == aligned_p[HAL_DCACHE_LINE_SIZE],
                   "invalidated beyond range");

    HAL_DCACHE_SYNC();
    HAL_DCACHE_DISABLE();

    CYG_TEST_CHECK(0 == aligned_p[0] &&
                   0 == aligned_p[HAL_DCACHE_LINE_SIZE-1],
                   "cache/memory contained invalidated data after SYNC/DIS");
    CYG_TEST_CHECK(42 == aligned_p[HAL_DCACHE_LINE_SIZE],
                   "invalidated beyond range after SYNC/DIS");

    HAL_DCACHE_ENABLE();
}
#endif
#endif // def HAL_DCACHE_QUERY_WRITE_MODE

// -------------------------------------------------------------------------
// Test of instruction cache locking.
//  o Time difference between repeatedly executing a bunch of instructions
//    with and without locking.
#ifdef HAL_ICACHE_LOCK
static void iloop(unsigned long* start, unsigned long* end, int dummy)
{
    // dummy is just used to fool the compiler to not move the labels
    // around. All callers should call with dummy=0;

    register char c;
    register CYG_INTERRUPT_STATE oldints;

    if (1 == dummy) goto label_end;

 label_start:
    // Invalidating shouldn't affect locked lines.
    HAL_DISABLE_INTERRUPTS(oldints);
    HAL_ICACHE_DISABLE();
    HAL_ICACHE_INVALIDATE_ALL();
    HAL_ICACHE_ENABLE();
    HAL_RESTORE_INTERRUPTS(oldints);

    c = m[HAL_DCACHE_LINE_SIZE*0];
    c = m[HAL_DCACHE_LINE_SIZE*1];
    c = m[HAL_DCACHE_LINE_SIZE*2];
    c = m[HAL_DCACHE_LINE_SIZE*3];
    c = m[HAL_DCACHE_LINE_SIZE*4];
    c = m[HAL_DCACHE_LINE_SIZE*5];
    c = m[HAL_DCACHE_LINE_SIZE*6];
    c = m[HAL_DCACHE_LINE_SIZE*7];
    c = m[HAL_DCACHE_LINE_SIZE*8];
    c = m[HAL_DCACHE_LINE_SIZE*9];
    c = m[HAL_DCACHE_LINE_SIZE*10];
    c = m[HAL_DCACHE_LINE_SIZE*11];
    c = m[HAL_DCACHE_LINE_SIZE*12];
    c = m[HAL_DCACHE_LINE_SIZE*13];
    c = m[HAL_DCACHE_LINE_SIZE*14];
    c = m[HAL_DCACHE_LINE_SIZE*15];
    c = m[HAL_DCACHE_LINE_SIZE*16];
    c = m[HAL_DCACHE_LINE_SIZE*17];
    c = m[HAL_DCACHE_LINE_SIZE*18];
    c = m[HAL_DCACHE_LINE_SIZE*19];
    c = m[HAL_DCACHE_LINE_SIZE*20];
    c = m[HAL_DCACHE_LINE_SIZE*21];
    c = m[HAL_DCACHE_LINE_SIZE*22];
    c = m[HAL_DCACHE_LINE_SIZE*23];
    c = m[HAL_DCACHE_LINE_SIZE*24];
    c = m[HAL_DCACHE_LINE_SIZE*25];
    c = m[HAL_DCACHE_LINE_SIZE*26];
    c = m[HAL_DCACHE_LINE_SIZE*27];
    c = m[HAL_DCACHE_LINE_SIZE*28];
    c = m[HAL_DCACHE_LINE_SIZE*29];
    c = m[HAL_DCACHE_LINE_SIZE*30];
    c = m[HAL_DCACHE_LINE_SIZE*31];
    c = m[HAL_DCACHE_LINE_SIZE*32];
    c = m[HAL_DCACHE_LINE_SIZE*33];
    c = m[HAL_DCACHE_LINE_SIZE*34];
    c = m[HAL_DCACHE_LINE_SIZE*35];
    c = m[HAL_DCACHE_LINE_SIZE*36];
    c = m[HAL_DCACHE_LINE_SIZE*37];
    c = m[HAL_DCACHE_LINE_SIZE*38];
    c = m[HAL_DCACHE_LINE_SIZE*39];
    c = m[HAL_DCACHE_LINE_SIZE*40];
    c = m[HAL_DCACHE_LINE_SIZE*41];
    c = m[HAL_DCACHE_LINE_SIZE*42];
    c = m[HAL_DCACHE_LINE_SIZE*43];
    c = m[HAL_DCACHE_LINE_SIZE*44];
    c = m[HAL_DCACHE_LINE_SIZE*45];
    c = m[HAL_DCACHE_LINE_SIZE*46];
    c = m[HAL_DCACHE_LINE_SIZE*47];
    c = m[HAL_DCACHE_LINE_SIZE*48];
    c = m[HAL_DCACHE_LINE_SIZE*49];
    c = m[HAL_DCACHE_LINE_SIZE*50];
    c = m[HAL_DCACHE_LINE_SIZE*51];
    c = m[HAL_DCACHE_LINE_SIZE*52];
    c = m[HAL_DCACHE_LINE_SIZE*53];
    c = m[HAL_DCACHE_LINE_SIZE*54];
    c = m[HAL_DCACHE_LINE_SIZE*55];
    c = m[HAL_DCACHE_LINE_SIZE*56];
    c = m[HAL_DCACHE_LINE_SIZE*57];
    c = m[HAL_DCACHE_LINE_SIZE*58];
    c = m[HAL_DCACHE_LINE_SIZE*59];
    c = m[HAL_DCACHE_LINE_SIZE*60];
    c = m[HAL_DCACHE_LINE_SIZE*61];
    c = m[HAL_DCACHE_LINE_SIZE*62];
    c = m[HAL_DCACHE_LINE_SIZE*63];

 label_end:

    *start = (unsigned long) &&label_start;
    *end = (unsigned long) &&label_end;

    if (1 == dummy) goto label_start;
}

static void time_ilock(void)
{
    register cyg_ucount32 k;
    cyg_tick_count_t count0, count1;
    cyg_ucount32 t;
    unsigned long start, end;
    register cyg_ucount32 time_ilock_loops = TIME_ILOCK_LOOPS;

    CYG_TEST_INFO("Instruction cache lock");

    if (cyg_test_is_simulator)
        time_ilock_loops = 10;

    count0 = cyg_current_time();
    for (k = 0; k < time_ilock_loops; k++) {
        iloop(&start, &end, 0);
    }
    count1 = cyg_current_time();
    t = count1 - count0;
    diag_printf("time without lock: %d\n", t);

    HAL_ICACHE_LOCK(start, end-start);

    count0 = cyg_current_time();
    for (k = 0; k < time_ilock_loops; k++) {
        iloop(&start, &end, 0);
    }
    count1 = cyg_current_time();
    t = count1 - count0;
    diag_printf("time with lock:    %d\n", t);

    HAL_ICACHE_UNLOCK(start, end-start);
}
#endif // ifdef HAL_ICACHE_LOCK

// -------------------------------------------------------------------------
// Test of data cache locking.
//  o Time difference between repeatedly accessing a memory region
//    with and without locking.
#ifdef HAL_DCACHE_LOCK
static void dloop(void)
{
    register cyg_uint32 j;
    register char c;
    register CYG_INTERRUPT_STATE oldints;

    HAL_DISABLE_INTERRUPTS(oldints);
    HAL_DCACHE_SYNC();
    HAL_DCACHE_DISABLE();
    HAL_DCACHE_SYNC();
    HAL_DCACHE_INVALIDATE_ALL();
    HAL_DCACHE_ENABLE();
    HAL_RESTORE_INTERRUPTS(oldints);
    for (j = 0; j < HAL_DCACHE_SETS; j++) {
        c = m[HAL_DCACHE_LINE_SIZE*j];
    }
}

static void time_dlock(void)
{
    register cyg_ucount32 k;
    cyg_tick_count_t count0, count1;
    cyg_ucount32 t;
    register cyg_ucount32 time_dlock_loops = TIME_DLOCK_LOOPS;

    CYG_TEST_INFO("Data cache lock");

    if (cyg_test_is_simulator)
        time_dlock_loops = 10;

    count0 = cyg_current_time();
    for (k = 0; k < time_dlock_loops; k++) {
        dloop();
    }
    count1 = cyg_current_time();
    t = count1 - count0;
    diag_printf("time without lock: %d\n", t);

    HAL_DCACHE_LOCK(&m[0], HAL_DCACHE_SETS*HAL_DCACHE_LINE_SIZE);

    count0 = cyg_current_time();
    for (k = 0; k < time_dlock_loops; k++) {
        dloop();
    }
    count1 = cyg_current_time();
    t = count1 - count0;
    diag_printf("time with lock:    %d\n", t);

    HAL_DCACHE_UNLOCK(&m[0], HAL_DCACHE_SETS*HAL_DCACHE_LINE_SIZE);
}
#endif // ifdef HAL_DCACHE_LOCK

// -------------------------------------------------------------------------
static void entry0( cyg_addrword_t data )
{
    int numtests = 0;
#ifdef HAL_DCACHE_QUERY_WRITE_MODE
    int wmode;
#endif
#ifdef HAL_DCACHE_LOCK
    time_dlock(); numtests++;
#endif
#ifdef HAL_ICACHE_LOCK
    time_ilock(); numtests++;
#endif
#ifdef HAL_DCACHE_LINE_SIZE // So we can find our way around memory
    test_dsync(); numtests++;
#endif
#ifdef HAL_DCACHE_STORE
    test_dstore(); numtests++;
#endif
#ifdef _TEST_DCACHE_OPERATION
    test_dcache_operation(); numtests++;
#endif
#ifdef HAL_DCACHE_READ_HINT
    test_dread_hint(); numtests++;
#endif
#ifdef HAL_DCACHE_WRITE_HINT
    test_dwrite_hint(); numtests++;
#endif
#ifdef HAL_DCACHE_ZERO
    test_dzero(); numtests++;
#endif

    // The below tests only work on a copy-back cache.
#ifdef HAL_DCACHE_QUERY_WRITE_MODE
    HAL_DCACHE_QUERY_WRITE_MODE( wmode );
 
    if ( HAL_DCACHE_WRITEBACK_MODE == wmode ) {
        test_ddisable(); numtests++;
#ifdef HAL_DCACHE_INVALIDATE
        test_dinvalidate_all(); numtests++;
#endif
#ifdef HAL_DCACHE_FLUSH
        test_dflush(); numtests++;
#endif
#ifdef HAL_DCACHE_INVALIDATE
        test_dinvalidate(); numtests++;
#endif
    }
#endif // def HAL_DCACHE_QUERY_WRITE_MODE
    if ( numtests ) {
        CYG_TEST_PASS_FINISH("End of test");
    }
    else {
        CYG_TEST_NA( "No applicable cache tests" );
    }
}

// -------------------------------------------------------------------------

void kcache2_main( void )
{
    CYG_TEST_INIT();

    cyg_thread_create(4, entry0 , (cyg_addrword_t)0, "kcache2",
        (void *)stack[0], STACKSIZE, &thread[0], &thread_obj[0]);
    cyg_thread_resume(thread[0]);

    cyg_scheduler_start();
}

// -------------------------------------------------------------------------

externC void
cyg_start( void )
{
    kcache2_main();
}

// -------------------------------------------------------------------------

#else // def CYGFUN_KERNEL_API_C
#define N_A_MSG "Kernel C API layer disabled"
#endif // def CYGFUN_KERNEL_API_C
#else // def CYGVAR_KERNEL_COUNTERS_CLOCK
#define N_A_MSG "Kernel real-time clock disabled"
#endif // def CYGVAR_KERNEL_COUNTERS_CLOCK

#ifdef N_A_MSG
externC void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_NA( N_A_MSG );
}
#endif // N_A_MSG

// -------------------------------------------------------------------------
/* EOF kcache2.c */
