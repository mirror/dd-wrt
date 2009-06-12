//==========================================================================
//
//      var_misc.c
//
//      HAL implementation miscellaneous functions
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
// Author(s):    tmichals
// Contributors: tmichals
// Date:         2002-09-20
// Purpose:      IDT 3233x variant miscellaneous functions
// Description:  This file contains miscellaneous functions provided by the
//               HAL.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>         // Base types

#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_cache.h>          // Cache handling

/*------------------------------------------------------------------------*/
// Array which stores the configured priority levels for the configured
// interrupts.

volatile CYG_BYTE hal_interrupt_level[CYGNUM_HAL_ISR_COUNT];

/*------------------------------------------------------------------------*/

void hal_variant_init(void)
{
    hal_IRQ_init();
}

void hal_c_cache_init(unsigned long config1_val)
{
    volatile unsigned val;

    HAL_DCACHE_INVALIDATE_ALL();
    HAL_ICACHE_INVALIDATE_ALL();

    // enable cached KSEG0
    asm volatile("mfc0 %0,$16;" : "=r"(val));
    val &= ~3;
    asm volatile("mtc0 %0,$16;" : : "r"(val));
}

void hal_icache_sync(void)
{
    HAL_ICACHE_INVALIDATE_ALL();
}

void hal_dcache_sync(void)
{
    HAL_DCACHE_INVALIDATE_ALL();
}

/*------------------------------------------------------------------------*/
/* End of var_misc.c                                                      */
