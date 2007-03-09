/*=================================================================
//
//        cache.c
//
//        HAL Cache timing test
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
// Contributors:    dsm, nickg
// Date:          1998-06-18
//####DESCRIPTIONEND####
*/

#include <pkgconf/hal.h>

#include <cyg/infra/testcase.h>

#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/infra/diag.h>

#if defined(HAL_DCACHE_SIZE) && HAL_DCACHE_SIZE != 0

// -------------------------------------------------------------------------
// If the HAL does not supply this, we supply our own version

#ifndef HAL_DCACHE_PURGE_ALL

#ifdef HAL_DCACHE_SYNC

# define HAL_DCACHE_PURGE_ALL() HAL_DCACHE_SYNC()

#else

static cyg_uint8 dca[HAL_DCACHE_SIZE + HAL_DCACHE_LINE_SIZE*2];

#define HAL_DCACHE_PURGE_ALL()                                          \
CYG_MACRO_START                                                         \
    volatile cyg_uint8 *addr = &dca[HAL_DCACHE_LINE_SIZE];              \
    volatile cyg_uint8 tmp = 0;                                         \
    int i;                                                              \
    for( i = 0; i < HAL_DCACHE_SIZE; i += HAL_DCACHE_LINE_SIZE )        \
    {                                                                   \
        tmp = addr[i];                                                  \
    }                                                                   \
CYG_MACRO_END

#endif // HAL_DCACHE_SYNC

#endif // HAL_DCACHE_PURGE_ALL

// -------------------------------------------------------------------------

#ifndef MAX_STRIDE
#define MAX_STRIDE 64
#endif

volatile char m[(HAL_DCACHE_SIZE/HAL_DCACHE_LINE_SIZE)*MAX_STRIDE];

// -------------------------------------------------------------------------

static void time0(register cyg_uint32 stride)
{
    register cyg_uint32 j,k;
    register char c;

    diag_printf("stride=%d\n", stride);

    k = 0;
    if ( cyg_test_is_simulator )
        k = 3960;

    for(; k<4000;k++) {
        for(j=0; j<(HAL_DCACHE_SIZE/HAL_DCACHE_LINE_SIZE); j++) {
            c=m[stride*j];
        }
    }
}

// -------------------------------------------------------------------------

void time1(void)
{
    cyg_uint32 i;

    for(i=1; i<=MAX_STRIDE; i+=i) {
        time0(i);
    }
}

// -------------------------------------------------------------------------
// This test could be improved by counting number of passes possible 
// in a given number of ticks.

static void entry0( void )
{
    register CYG_INTERRUPT_STATE oldints;

#ifdef HAL_CACHE_UNIFIED

    HAL_DISABLE_INTERRUPTS(oldints);
    HAL_DCACHE_PURGE_ALL();             // rely on above definition
    HAL_UCACHE_INVALIDATE_ALL();
    HAL_UCACHE_DISABLE();
    HAL_RESTORE_INTERRUPTS(oldints);
    CYG_TEST_INFO("Cache off");
    time1();

    HAL_DISABLE_INTERRUPTS(oldints);
    HAL_DCACHE_PURGE_ALL();             // rely on above definition
    HAL_UCACHE_INVALIDATE_ALL();
    HAL_UCACHE_ENABLE();
    HAL_RESTORE_INTERRUPTS(oldints);
    CYG_TEST_INFO("Cache on");
    time1();

#else // HAL_CACHE_UNIFIED

    HAL_DISABLE_INTERRUPTS(oldints);
    HAL_DCACHE_PURGE_ALL();
    HAL_ICACHE_INVALIDATE_ALL();
    HAL_DCACHE_INVALIDATE_ALL();
    HAL_ICACHE_DISABLE();
    HAL_DCACHE_DISABLE();
    HAL_RESTORE_INTERRUPTS(oldints);
    CYG_TEST_INFO("Dcache off Icache off");
    time1();

    HAL_DISABLE_INTERRUPTS(oldints);
    HAL_DCACHE_PURGE_ALL();
    HAL_ICACHE_INVALIDATE_ALL();
    HAL_DCACHE_INVALIDATE_ALL();
    HAL_ICACHE_DISABLE();
    HAL_DCACHE_ENABLE();
    HAL_RESTORE_INTERRUPTS(oldints);
    CYG_TEST_INFO("Dcache on  Icache off");
    time1();

    HAL_DISABLE_INTERRUPTS(oldints);
    HAL_DCACHE_PURGE_ALL();
    HAL_ICACHE_INVALIDATE_ALL();
    HAL_DCACHE_INVALIDATE_ALL();
    HAL_ICACHE_ENABLE();
    HAL_DCACHE_DISABLE();
    HAL_RESTORE_INTERRUPTS(oldints);
    CYG_TEST_INFO("Dcache off Icache on");
    time1();

    HAL_DISABLE_INTERRUPTS(oldints);
    HAL_DCACHE_PURGE_ALL();
    HAL_ICACHE_INVALIDATE_ALL();
    HAL_DCACHE_INVALIDATE_ALL();
    HAL_ICACHE_ENABLE();
    HAL_DCACHE_ENABLE();
    HAL_RESTORE_INTERRUPTS(oldints);
    CYG_TEST_INFO("Dcache on  Icache on");
    time1();

    HAL_DISABLE_INTERRUPTS(oldints);
    HAL_DCACHE_PURGE_ALL();
    HAL_ICACHE_INVALIDATE_ALL();
    HAL_DCACHE_INVALIDATE_ALL();
    HAL_ICACHE_DISABLE();
    HAL_DCACHE_DISABLE();
    HAL_RESTORE_INTERRUPTS(oldints);
    CYG_TEST_INFO("Dcache off Icache off");
    time1();

#endif // HAL_CACHE_UNIFIED

    CYG_TEST_PASS_FINISH("End of test");
}

// -------------------------------------------------------------------------

void cache_main( void )
{
    CYG_TEST_INIT();

    entry0();

}

// -------------------------------------------------------------------------

externC void
cyg_start( void )
{
    cache_main();
}

#else // HAL_DCACHE_SIZE
#define N_A_MSG "No cache"
#endif // HAL_DCACHE_SIZE

#ifdef N_A_MSG
externC void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_NA( N_A_MSG );
}
#endif // N_A_MSG

// -------------------------------------------------------------------------
/* EOF cache.c */
