//==========================================================================
//
//      var_misc.c
//
//      HAL CPU variant miscellaneous functions
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
// Author(s):    nickg
// Contributors: nickg, jlarmour
// Date:         1999-01-21
// Purpose:      HAL miscellaneous functions
// Description:  This file contains miscellaneous functions provided by the
//               HAL.
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>         // Base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_cache.h>

/*------------------------------------------------------------------------*/
/* Variant specific initialization routine.                               */

void hal_variant_init(void)
{
}

/*------------------------------------------------------------------------*/
/* Cache functions.                                                       */

#if !defined(CYG_HAL_MN10300_AM31_SIM)
void cyg_hal_dcache_store(CYG_ADDRWORD base, int size)
{
#if 0    
    volatile register CYG_BYTE *way0 = HAL_DCACHE_PURGE_WAY0;
    volatile register CYG_BYTE *way1 = HAL_DCACHE_PURGE_WAY1;
    register int i;
    register CYG_ADDRWORD state;

    HAL_DCACHE_IS_ENABLED(state);
    if (state)
        HAL_DCACHE_DISABLE();

    way0 += base & 0x000007f0;
    way1 += base & 0x000007f0;
    for( i = 0; i < size; i += HAL_DCACHE_LINE_SIZE )
    {
        *(CYG_ADDRWORD *)way0 = 0;
        *(CYG_ADDRWORD *)way1 = 0;
        way0 += HAL_DCACHE_LINE_SIZE;
        way1 += HAL_DCACHE_LINE_SIZE;
    }
    if (state)
        HAL_DCACHE_ENABLE();
#else

    // Despite several people trying to understand it, the above code
    // still does not work correctly. Whether this is a result of the
    // AM31 hardware being faulty, or simply a problem with register
    // pressure causing the values of some variables to be lost as a
    // result of the purge we are not sure. Similar code on the AM33
    // works perfectly, but that processor has more registers to play
    // with.  The following code, taken from CygMon, works correctly,
    // and does not appear to exhibit the problems of the code above.
    // It's only drawback is that it purges the entire cache. However,
    // this is permitted under the definition of the cache macros, so
    // this is not a major concern at present.
    
    __asm__ (
              ".equ CHCTR,        0x20000070;"
              ".equ CHCTR_ICEN,   0x0001;"
              ".equ CHCTR_DCEN,   0x0002;"
              ".equ CHCTR_ICBUSY, 0x0004;"
              ".equ CHCTR_DCBUSY, 0x0008;"
              ".equ CHCTR_ICINV,  0x0010;"
              ".equ CHCTR_DCINV,  0x0020;"
              ".equ DCACHE_PURGE_WAY0_START, 0x28400000;"
              ".equ DCACHE_PURGE_WAY0_END,   0x28400800;"
              ".equ DCACHE_PURGE_WAY1_START, 0x28401000;"
              ".equ DCACHE_PURGE_WAY1_END,   0x28401800;"

              "mov CHCTR,a0;"

              /* DCACHE */

              /* first check if dcache enabled */
              "mov (a0),d0;"
              "btst CHCTR_DCEN,d0;"
              /* if not, jump to end */
              "beq 2f;"

              /* if so, we have to disable the cache */
              "and 0xfffffffd,d0;"
              "mov d0,(a0);"

              /* wait for it to stop being busy */
              "setlb;"
              "nop;"
              "mov (a0),d0;"
              "btst CHCTR_DCBUSY,d0;"
              "lne;"

              /* now purge the dcache */
              "mov DCACHE_PURGE_WAY0_START,a1;"
              "1:"
              "mov (0,a1),d0;"
              "add 0x10,a1;"
              "cmp DCACHE_PURGE_WAY0_END,a1;"
              "bne 1b;"
              "mov DCACHE_PURGE_WAY1_START,a1;"
              "1:"
              "mov (0,a1),d0;"
              "add 0x10,a1;"
              "cmp DCACHE_PURGE_WAY1_END,a1;"
              "bne 1b;"

              /* wait for it to stop being busy */
              "setlb;"
              "nop;"
              "mov (a0),d0;"
              "btst CHCTR_DCBUSY,d0;"
              "lne;"

              /* and re-enable */
              "or CHCTR_DCEN,d0;"
              "mov d0,(a0);"

              "2:"

              :
              :
              : "a0", "a1", "d0"
        );
    
#endif    
}
#endif


/*------------------------------------------------------------------------*/
/* End of var_misc.c                                                      */
