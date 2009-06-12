//==========================================================================
//
//      arm9_misc.c
//
//      HAL misc board support code for ARM9
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
// Author(s):    gthomas
// Contributors: jskov
// Date:         2000-05-08
// Purpose:      HAL board support
// Description:  Implementations of HAL board interfaces
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include CYGBLD_HAL_PLATFORM_H

#include <cyg/infra/cyg_type.h>         // base types

#include <cyg/hal/hal_if.h>             // HAL ROM/if
#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_cache.h>

// Most initialization has already been done before we get here.
// All we do here is enable the caches.

externC void plf_hardware_init(void);

void hal_hardware_init(void)
{
    // Perform any platform specific initializations
    plf_hardware_init();

    // Set up eCos/ROM interfaces
    hal_if_init();

#ifndef CYG_HAL_STARTUP_RAM
    // Invalidate caches
    HAL_DCACHE_INVALIDATE_ALL();
    HAL_ICACHE_INVALIDATE_ALL();
#endif
    // Enable caches
#ifdef CYGSEM_HAL_ENABLE_DCACHE_ON_STARTUP
    HAL_DCACHE_ENABLE();
#endif
#ifdef CYGSEM_HAL_ENABLE_ICACHE_ON_STARTUP
    HAL_ICACHE_ENABLE();
#endif
}

void
cyg_hal_arm9_soft_reset(CYG_ADDRESS entry)
{

    /* It would probably make more sense to have the
       clear/drain/invalidate after disabling the cache and MMU, but
       then we'd have to know the (unmapped) address of this code. */
    asm volatile ("mrs r1,cpsr;"
                  "bic r1,r1,#0x1F;"  /* Put processor in SVC mode */
                  "orr r1,r1,#0x13;"
                  "msr cpsr,r1;"

                  "mov r1, #0;"
                  "mcr p15,0,r1,c7,c7,0;"  /* clear I+DCache */
                  "mcr p15,0,r1,c7,c10,4;" /* Drain Write Buffer */
                  "mcr p15,0,r1,c8,c7,0;"  /* Invalidate TLBs */
                  "mrc p15,0,r1,c1,c0,0;"
                  "bic r1,r1,#0x1000;"     /* disable ICache */
                  "bic r1,r1,#0x0007;"     /* disable DCache, MMU and alignment faults */
                  "mcr p15,0,r1,c1,c0,0;"
                  "nop;"                   /* delay 1 */
                  "mov pc, %0;"            /* delay 2  - next instruction should be fetched flat */
                  : : "r" (entry) : "r1");
    for(;;);
}

/*------------------------------------------------------------------------*/
// EOF arm9_misc.c
