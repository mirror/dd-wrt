//==========================================================================
//
//        intr0.c
//
//        Interrupt controls test
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
// Author(s):     jskov
// Contributors:  jskov
// Date:          2001-01-18
//####DESCRIPTIONEND####
//==========================================================================

#include <pkgconf/system.h>

#include <cyg/infra/testcase.h>

#if defined(CYGPKG_KERNEL)

#include <pkgconf/kernel.h>

#if defined(CYGFUN_KERNEL_API_C)

#include <cyg/hal/hal_arch.h>           // CYGNUM_HAL_STACK_SIZE_TYPICAL

#include <cyg/kernel/kapi.h>

#include <cyg/hal/hal_intr.h>

// -------------------------------------------------------------------------

#define NTHREADS 1
#define STACKSIZE CYGNUM_HAL_STACK_SIZE_TYPICAL

static cyg_handle_t thread[NTHREADS];

static cyg_thread thread_obj[NTHREADS];
static char stack[NTHREADS][STACKSIZE];

// -------------------------------------------------------------------------

// This is (tick time * 1.5) 
#define TICK_DELAY (1500000 / CYGNUM_HAL_RTC_DENOMINATOR)

static void
entry0( cyg_addrword_t data )
{
    int tick;

    // Scheduler and thus timer interrupts are running by the
    // time we get here.

    // Wait for next tick
    tick = cyg_current_time();
    do {} while (cyg_current_time() == tick);
    tick = cyg_current_time();

    // Then mask timer interrupts
    HAL_INTERRUPT_MASK(CYGNUM_HAL_INTERRUPT_RTC);

    // and wait for the time when the next tick should have come
    // and check it didn't trigger an interrupt
    hal_delay_us(TICK_DELAY);
    CYG_TEST_CHECK(cyg_current_time() == tick, "Timer interrupt while masked");

    // Now change interrupt level, and make the check again. Changing
    // level should not affect interrupt mask state.
    HAL_INTERRUPT_SET_LEVEL(CYGNUM_HAL_INTERRUPT_RTC, 8);
    hal_delay_us(TICK_DELAY);
    CYG_TEST_CHECK(cyg_current_time() == tick, 
                   "Timer interrupt after changing level");

    // Finally unmask the interrupt and make sure it results in ticks.
    HAL_INTERRUPT_UNMASK(CYGNUM_HAL_INTERRUPT_RTC);
    hal_delay_us(TICK_DELAY);
    CYG_TEST_CHECK(cyg_current_time() != tick, 
                   "No timer interrupt after unmask");

    CYG_TEST_PASS_FINISH("SH intr0 test end");
}

// -------------------------------------------------------------------------

externC void
cyg_start( void )
{
    CYG_TEST_INIT();

    cyg_thread_create(4, entry0 , (cyg_addrword_t)0, "intr",
        (void *)stack[0], STACKSIZE, &thread[0], &thread_obj[0]);
    cyg_thread_resume(thread[0]);

    cyg_scheduler_start();
}

// -------------------------------------------------------------------------

#else // def CYGFUN_KERNEL_API_C
#define N_A_MSG "Kernel C API layer disabled"
#endif // def CYGFUN_KERNEL_API_C
#else // def CYGPKG_KERNEL
#define N_A_MSG "Needs kernel"
#endif // def CYGPKG_KERNEL

#ifdef N_A_MSG
externC void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_NA( N_A_MSG );
}
#endif // N_A_MSG

// -------------------------------------------------------------------------
// EOF intr0.c
