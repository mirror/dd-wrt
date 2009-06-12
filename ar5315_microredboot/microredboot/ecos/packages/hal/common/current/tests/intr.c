/*=================================================================
//
//        intr.c
//
//        HAL Interrupt API test
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
// Author(s):     nickg
// Contributors:  nickg
// Date:          1998-10-08
//####DESCRIPTIONEND####
*/

#include <pkgconf/hal.h>

#include <cyg/infra/testcase.h>

#include <cyg/hal/hal_intr.h>
#include <cyg/hal/drv_api.h>

// Include HAL/Platform specifics
#include CYGBLD_HAL_PLATFORM_H

#ifdef CYGPKG_KERNEL
#include <pkgconf/kernel.h>             // Need to look for the RTC config.
#endif

// Fallback defaults (in case HAL didn't define these)
#ifndef CYGNUM_HAL_RTC_NUMERATOR       
#define CYGNUM_HAL_RTC_NUMERATOR     1000000000
#define CYGNUM_HAL_RTC_DENOMINATOR   100
#define CYGNUM_HAL_RTC_PERIOD        9999
#endif

// -------------------------------------------------------------------------

#define ISR_DATA        0xAAAA1234

// -------------------------------------------------------------------------

volatile cyg_count32 ticks = 0;
static cyg_interrupt intr;
static cyg_handle_t  intr_handle;

// -------------------------------------------------------------------------

cyg_uint32 isr( cyg_uint32 vector, CYG_ADDRWORD data )
{
    CYG_TEST_CHECK( ISR_DATA == data , "Bad data passed to ISR");
    CYG_TEST_CHECK( CYGNUM_HAL_INTERRUPT_RTC == vector ,
                    "Bad vector passed to ISR");

    HAL_CLOCK_RESET( vector, CYGNUM_HAL_RTC_PERIOD );

    HAL_INTERRUPT_ACKNOWLEDGE( vector );
    
    ticks++;

    return CYG_ISR_HANDLED;
}

// -------------------------------------------------------------------------


void intr_main( void )
{
    CYG_INTERRUPT_STATE oldints;

    cyg_drv_interrupt_create(CYGNUM_HAL_INTERRUPT_RTC, 1,
                             ISR_DATA, isr, NULL, &intr_handle, &intr);
    cyg_drv_interrupt_attach(intr_handle);
    HAL_CLOCK_INITIALIZE( CYGNUM_HAL_RTC_PERIOD );
    cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_RTC);

    HAL_ENABLE_INTERRUPTS();

    while( ticks < 10 )
    {
        
    }

    HAL_DISABLE_INTERRUPTS(oldints);

    CYG_TEST_PASS_FINISH("HAL interrupt test");
}


// -------------------------------------------------------------------------

externC void
cyg_start( void )
{
    CYG_TEST_INIT();

    // Attaching the ISR will not succeed if the kernel real-time
    // clock has been configured in.
#ifndef CYGVAR_KERNEL_COUNTERS_CLOCK

    intr_main();

#else

    CYG_TEST_NA("Cannot override kernel real-time clock.");

#endif
}


// -------------------------------------------------------------------------
// EOF intr.c
