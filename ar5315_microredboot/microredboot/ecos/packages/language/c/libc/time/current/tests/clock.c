//=================================================================
//
//        clock.c
//
//        Testcase for C library clock()
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
// Author(s):     ctarpy, jlarmour
// Contributors:  
// Date:          1999-03-05
// Description:   Contains testcode for C library clock() function
//
//
//####DESCRIPTIONEND####

// CONFIGURATION

#include <pkgconf/libc_time.h>   // Configuration header
#include <pkgconf/system.h>
#include <pkgconf/isoinfra.h>
#include <pkgconf/infra.h>

#include <cyg/infra/testcase.h>

// This test is bound to fail often on the synthetic target -- we
// don't have exclusive access to the CPU.

#if defined(CYGPKG_HAL_SYNTH)
# define NA_MSG "Timing accuracy not guaranteed on synthetic target"
#elif !defined(CYGINT_ISO_MAIN_STARTUP)
# define NA_MSG "Requires main() startup"
#elif defined(CYGDBG_USE_TRACING)
# define NA_MSG "Cannot give an accurate test when tracing is enabled"
#endif

#ifdef NA_MSG
void
cyg_start(void)
{
    CYG_TEST_INIT();
    CYG_TEST_NA( NA_MSG );
}

#else


// INCLUDES

#include <time.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_intr.h>


// CONSTANTS

// This defines how many loops before we decide that
// the clock doesnt work
#define MAX_TIMEOUT 100000

// Percentage error before we declare fail: range 0 - 100
#define TOLERANCE 40

// Permissible absolute deviation from mean value to take care of incorrect
// failure conclusions in case of small mean values (if absolute values of
// clock() are small, percentage variation can be large)
#define FUDGE_FACTOR 6

// Number of samples to take
#define SAMPLES 30

// We ignore ctrs[0] because it's always 0
// We ignore ctrs[1] because it will always be odd since it was
// the first measurement taken at the start of the looping, and
// the initial clock measurement (in clocks[0]) was not treated as
// part of the loop and therefore can't be considered to take the same
// time.
// We ignore ctrs[2] because it always seems to be substantially faster
// that the other samples. Probably due to cache/timing effect after the
// previous loop.
// Finally, ctrs[3] is skipped because it's also very fast on ARM targets.

#define SKIPPED_SAMPLES 6


// FUNCTIONS

static int
my_abs(int i)
{
    return (i < 0) ? -i : i;
} // my_abs()


// Clock measurement is done in a separate function so that alignment
// constraints are deterministic - some processors may perform better
// in loops that are better aligned, so by making it always the same
// function, this is prevented.
// FIXME: how do we guarantee the compiler won't inline this on -O3?
static unsigned long
clock_loop( const int timeout, clock_t prevclock, clock_t *newclock )
{
    clock_t c=0;
    long i;

    for (i=0; i<timeout; i++) {
        c = clock();
        if ( c != prevclock )
            break; // Hit the next clock pulse
    }
    
    if (i==timeout)
        CYG_TEST_FAIL_FINISH("No change in clock state!");

    // it should not overflow in the lifetime of this test
    if (c < prevclock)
        CYG_TEST_FAIL_FINISH("Clock decremented!");

    *newclock = c;

    return i;
} // clock_loop()

// both of these get zeroed out
static unsigned long ctrs[SAMPLES];
static clock_t clocks[SAMPLES];

int
main(int argc, char *argv[])
{
    unsigned long mean=0, sum=0, maxerr=0;
    int i;
    unsigned int absdev;

    CYG_TEST_INIT();

    CYG_TEST_INFO("Starting tests from testcase " __FILE__ " for C library "
                  "clock() function");

    // First disable the caches - they may affect the timing loops
    // below - causing the elapsed time during the clock() call to vary.
    {
        register CYG_INTERRUPT_STATE oldints;

        HAL_DISABLE_INTERRUPTS(oldints);
        HAL_DCACHE_SYNC();
        HAL_ICACHE_DISABLE();
        HAL_DCACHE_DISABLE();
        HAL_DCACHE_SYNC();
        HAL_ICACHE_INVALIDATE_ALL();
        HAL_DCACHE_INVALIDATE_ALL();
        HAL_RESTORE_INTERRUPTS(oldints);
    }

    // This waits for a clock tick, to ensure that we are at the
    // start of a clock period. Then sit in a tight loop to get
    // the clock period. Repeat this, and make sure that it the
    // two timed periods are acceptably close.

    clocks[0] = clock();
    
    if (clocks[0] == (clock_t)-1)  // unimplemented is potentially valid.
    {
#ifdef CYGSEM_LIBC_TIME_CLOCK_WORKING
        CYG_TEST_FAIL_FINISH( "clock() returns -1, meaning unimplemented");
#else
        CYG_TEST_PASS_FINISH( "clock() returns -1, meaning unimplemented");
#endif
    } // if

    // record clocks in a tight consistent loop to avoid random variations
    for (i=1; i<SAMPLES; i++) {
        ctrs[i] = clock_loop( MAX_TIMEOUT, clocks[i-1], &clocks[i] );
    }

    for (i=0;i<SAMPLES;i++) {
        // output what we got - useful for diagnostics of occasional
        // test failures
        diag_printf("clocks[%d] = %d, ctrs[%d] = %d\n", i, clocks[i],
                    i, ctrs[i]);

        // Now we work out the error etc.
        if (i>=SKIPPED_SAMPLES) {
            sum += ctrs[i];
        }
    }

    // deduce out the average
    mean = sum / (SAMPLES-SKIPPED_SAMPLES);

    // now go through valid results and compare against average
    for (i=SKIPPED_SAMPLES;i<SAMPLES;i++) {
        unsigned long err;

        absdev = my_abs(ctrs[i]-mean);
        // use mean+1 as divisor to avoid div-by-zero
        err = (100 * absdev) / (mean+1);
        if (err > TOLERANCE && absdev > FUDGE_FACTOR) {
            diag_printf("mean=%d, ctrs[%d]=%d, err=%d\n", mean, i, ctrs[i],
                        err);
            CYG_TEST_FAIL_FINISH("clock() within tolerance");
        }
        if (err > maxerr)
            maxerr=err;
    }

    diag_printf("mean=%d, maxerr=%d\n", mean, maxerr);
    CYG_TEST_PASS_FINISH("clock() stable");

} // main()

#endif // ifndef NA_MSG

// EOF clock.c
