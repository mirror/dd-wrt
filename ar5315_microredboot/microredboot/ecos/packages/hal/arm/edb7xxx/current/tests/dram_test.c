//==========================================================================
//
//        dram_test.c
//
//        eCos generic DRAM test code
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
// Author(s):     gthomas
// Contributors:  gthomas
// Date:          1999-10-22
// Description:   Tool used to test DRAM on eval boards
//####DESCRIPTIONEND####

#include <pkgconf/system.h>
#include <cyg/infra/testcase.h>

#ifdef CYGPKG_KERNEL

#include <pkgconf/kernel.h>   // Configuration header

#ifdef CYGFUN_KERNEL_API_C

#include <cyg/kernel/kapi.h>
#include <cyg/infra/diag.h>

#include <cyg/hal/hal_arch.h>
#include CYGHWR_MEMORY_LAYOUT_H  // Memory layout

#ifndef FALSE
#define FALSE 0
#define TRUE  1
#endif

#define STACK_SIZE CYGNUM_HAL_STACK_SIZE_MINIMUM
static char stack[STACK_SIZE];
static cyg_thread thread_data;
static cyg_handle_t thread_handle;

extern char __bss_end;
int total_errors;
int run_errors;
int error_count;

#define MAX_ERRORS   5
#define NUM_RUNS     4
#define REFRESH_TIME 5

int decay_time[] = { 50, 100, 200, 500, 1000 };
#define NUM_DECAY sizeof(decay_time)/sizeof(decay_time[0])

// FUNCTIONS

static void
new_test(void)
{
    error_count = 0;
}

static void
report_error(void *addr, cyg_uint32 actual, cyg_uint32 expected)
{
    total_errors++;
    run_errors++;
    if (++error_count > MAX_ERRORS) return;
    diag_printf("   0x%08x: expected - 0x%08x, actual - 0x%08x\n", 
                addr, expected, actual);
}

// Fill longwords with their own address and verify

static void
addr_test(cyg_uint32 start, cyg_uint32 end)
{
    cyg_uint32 *mp;

    new_test();
    diag_printf("-- Address test\n");
    for (mp = (cyg_uint32 *)start;  mp < (cyg_uint32 *)end;  mp++) {
        *mp = (cyg_uint32)mp;
    }
    for (mp = (cyg_uint32 *)start;  mp < (cyg_uint32 *)end;  mp++) {
        if (*mp != (cyg_uint32)mp) {
            report_error(mp, *mp, (cyg_uint32)mp);
        }
    }
}

// Fill longwords with zeros

static void
zeros_test(cyg_uint32 start, cyg_uint32 end)
{
    cyg_uint32 *mp;

    new_test();
    diag_printf("-- Zeros test\n");
    for (mp = (cyg_uint32 *)start;  mp < (cyg_uint32 *)end;  mp++) {
        *mp = (cyg_uint32)0;
    }
    for (mp = (cyg_uint32 *)start;  mp < (cyg_uint32 *)end;  mp++) {
        if (*mp != (cyg_uint32)0) {
            report_error(mp, *mp, (cyg_uint32)0);
        }
    }
}

// Fill longwords with all ones

static void
ones_test(cyg_uint32 start, cyg_uint32 end)
{
    cyg_uint32 *mp;

    new_test();
    diag_printf("-- Ones test\n");
    for (mp = (cyg_uint32 *)start;  mp < (cyg_uint32 *)end;  mp++) {
        *mp = (cyg_uint32)0xFFFFFFFF;
    }
    for (mp = (cyg_uint32 *)start;  mp < (cyg_uint32 *)end;  mp++) {
        if (*mp != (cyg_uint32)0xFFFFFFFF) {
            report_error(mp, *mp, (cyg_uint32)0xFFFFFFFF);
        }
    }
}

// Fill longwords with a "walking" bit

static void
walking_bit_test(cyg_uint32 start, cyg_uint32 end)
{
    cyg_uint32 *mp;
    cyg_uint32 bit;

    new_test();
    diag_printf("-- Walking test\n");
    bit = 1;
    for (mp = (cyg_uint32 *)start;  mp < (cyg_uint32 *)end;  mp++) {
        *mp = (cyg_uint32)bit;
        bit <<= 1;
        if (bit == 0) bit = 1;
    }
    bit = 1;
    for (mp = (cyg_uint32 *)start;  mp < (cyg_uint32 *)end;  mp++) {
        if (*mp != (cyg_uint32)bit) {
            report_error(mp, *mp, (cyg_uint32)bit);
        }
        bit <<= 1;
        if (bit == 0) bit = 1;
    }
}

// Fill longwords with an alternating pattern

static void
pattern_test(cyg_uint32 start, cyg_uint32 end)
{
    cyg_uint32 *mp;
    cyg_uint32 pat;

    new_test();
    diag_printf("-- Pattern test\n");
    pat = 0x55555555;
    for (mp = (cyg_uint32 *)start;  mp < (cyg_uint32 *)end;  mp++) {
        *mp = (cyg_uint32)pat;
    }
    for (mp = (cyg_uint32 *)start;  mp < (cyg_uint32 *)end;  mp++) {
        if (*mp != (cyg_uint32)pat) {
            report_error(mp, *mp, (cyg_uint32)pat);
        }
    }
    pat = 0xAAAAAAAA;
    for (mp = (cyg_uint32 *)start;  mp < (cyg_uint32 *)end;  mp++) {
        *mp = (cyg_uint32)pat;
    }
    for (mp = (cyg_uint32 *)start;  mp < (cyg_uint32 *)end;  mp++) {
        if (*mp != (cyg_uint32)pat) {
            report_error(mp, *mp, (cyg_uint32)pat);
        }
    }
}

// Verify if refresh works

static void
refresh_test(cyg_uint32 start, cyg_uint32 end)
{
    cyg_uint32 *mp;
    cyg_uint32 pat;

    new_test();
    diag_printf("-- Refresh test\n");
    pat = 0x55555555;
    for (mp = (cyg_uint32 *)start;  mp < (cyg_uint32 *)end;  mp++) {
        *mp = (cyg_uint32)pat;
    }
    cyg_thread_delay(REFRESH_TIME*100);
    for (mp = (cyg_uint32 *)start;  mp < (cyg_uint32 *)end;  mp++) {
        if (*mp != (cyg_uint32)pat) {
            report_error(mp, *mp, (cyg_uint32)pat);
        }
    }
    pat = 0xAAAAAAAA;
    for (mp = (cyg_uint32 *)start;  mp < (cyg_uint32 *)end;  mp++) {
        *mp = (cyg_uint32)pat;
    }
    cyg_thread_delay(REFRESH_TIME*100);
    for (mp = (cyg_uint32 *)start;  mp < (cyg_uint32 *)end;  mp++) {
        if (*mp != (cyg_uint32)pat) {
            report_error(mp, *mp, (cyg_uint32)pat);
        }
    }
}

// See how long we can "sleep" before refresh fails

static void
decay_test(cyg_uint32 start, cyg_uint32 end)
{
    cyg_uint32 *mp;
    cyg_uint32 pat;
    int i;

    diag_printf("-- Decay test\n");
    for (i = 0;  i < NUM_DECAY;  i++) {
        diag_printf("    ... %d.%02d sec delay\n", decay_time[i]/100, decay_time[i]%100);
        new_test();
        pat = 0x55555555;
        for (mp = (cyg_uint32 *)start;  mp < (cyg_uint32 *)end;  mp++) {
            *mp = (cyg_uint32)pat;
        }
        cyg_thread_delay(decay_time[i]);
        for (mp = (cyg_uint32 *)start;  mp < (cyg_uint32 *)end;  mp++) {
            if (*mp != (cyg_uint32)pat) {
                report_error(mp, *mp, (cyg_uint32)pat);
            }
        }
        pat = 0xAAAAAAAA;
        for (mp = (cyg_uint32 *)start;  mp < (cyg_uint32 *)end;  mp++) {
            *mp = (cyg_uint32)pat;
        }
        cyg_thread_delay(decay_time[i]);
        for (mp = (cyg_uint32 *)start;  mp < (cyg_uint32 *)end;  mp++) {
            if (*mp != (cyg_uint32)pat) {
                report_error(mp, *mp, (cyg_uint32)pat);
            }
        }
        diag_printf("    ... %d errors\n", error_count);
    }
}


static void
dram_exercise(cyg_addrword_t p)
{
    cyg_uint32 start_DRAM, end_DRAM;
    int i;

    CYG_TEST_INIT();

    end_DRAM = CYGMEM_REGION_ram+CYGMEM_REGION_ram_SIZE;
    start_DRAM = ((cyg_uint32)&__bss_end + 0x00000FFF) & 0xFFFFF000;
    diag_printf("DRAM test - start: 0x%08x, end: 0x%08x\n", start_DRAM, end_DRAM);

    for (i = 0;  i < NUM_RUNS;  i++) {
        diag_printf("\n*** Start run #%d of %d\n", i+1, NUM_RUNS);
        run_errors = 0;
        addr_test(start_DRAM, end_DRAM);
        ones_test(start_DRAM, end_DRAM);
        zeros_test(start_DRAM, end_DRAM);
        walking_bit_test(start_DRAM, end_DRAM);
        pattern_test(start_DRAM, end_DRAM);
        refresh_test(start_DRAM, end_DRAM);
        decay_test(start_DRAM, end_DRAM);
        diag_printf("\n*** End run, %d errors, %d total errors\n", run_errors, total_errors);
    }

    CYG_TEST_PASS_FINISH("DRAM testing complete");
}

externC void
cyg_start( void )
{
    // Create a main thread, so we can run the scheduler and have time 'pass'
    cyg_thread_create(10,                // Priority - just a number
                      dram_exercise,     // entry
                      0,                 // entry parameter
                      "DRAM test",       // Name
                      &stack[0],         // Stack
                      STACK_SIZE,        // Size
                      &thread_handle,    // Handle
                      &thread_data       // Thread data structure
            );
    cyg_thread_resume(thread_handle);  // Start it
    cyg_scheduler_start();
} // cyg_package_start()

#else /* def CYGFUN_KERNEL_API_C */
# define NA_MSG "Kernel C API layer disabled"
#endif

#else /* def CYGPKG_KERNEL */
# define NA_MSG "No kernel"
#endif


#ifdef NA_MSG
externC void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_NA(NA_MSG);
}
#endif
