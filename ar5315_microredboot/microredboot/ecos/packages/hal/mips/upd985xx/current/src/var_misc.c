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
// Author(s):    hmt, nickg
// Contributors: nickg, jlarmour
// Date:         2001-05-24
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

#include <cyg/hal/hal_arch.h>           // everything...
#include <cyg/hal/hal_cache.h>          // HAL_ICACHE_ENABLE();
#include <cyg/hal/hal_if.h>             // hal_if_init();
#include <cyg/hal/hal_intr.h>           // HAL_INTERRUPT_UNMASK()
#include <cyg/hal/hal_arbiter.h>        // hal_call_isr()

/*------------------------------------------------------------------------*/
/* Variant specific initialization routine.                               */

volatile CYG_ADDRESS    hal_interrupt_handlers[CYGNUM_HAL_ISR_COUNT];
volatile CYG_ADDRWORD   hal_interrupt_data[CYGNUM_HAL_ISR_COUNT];
volatile CYG_ADDRESS    hal_interrupt_objects[CYGNUM_HAL_ISR_COUNT];

// ------------------------------------------------------------------------
// An ISR for decoding and calling the additional S_ISR external interrupt
// sources; this has to be an external routine because the S_ISR register
// is read-clear, and the interrupt sources are edge-triggered so they do
// not re-assert themselves - so we must address multiple sources per
// actual interrupt.

static cyg_uint32 _arbitration_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data);

#ifndef CYGOPT_HAL_MIPS_UPD985XX_HARDWARE_BUGS_S2
static cyg_uint32 _arbitration_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data)
{
    cyg_uint32 s_isr;
    int bit, vecnum;
    cyg_uint32 isr_ret = 0;
    // decode interrupt source and for each active source call the ISR
   
    s_isr = *S_ISR;                     // Read once only
    s_isr &= *S_IMR;                    // Keep unmasked bits

    for ( bit = 1, vecnum = CYGNUM_HAL_INTERRUPT_SYSCTL_LOW;
          vecnum <=  CYGNUM_HAL_INTERRUPT_SYSCTL_HI;
          bit <<= 1, vecnum++ )
        if ( bit & s_isr )
            isr_ret |= hal_call_isr( vecnum );
    
    return isr_ret & ~CYG_ISR_CALL_DSR; // Since we have no DSR.
}
#endif // NOT defined CYGOPT_HAL_MIPS_UPD985XX_HARDWARE_BUGS_S2

// ------------------------------------------------------------------------

void hal_variant_init(void)
{
    int i;
    for ( i = 0; i < CYGNUM_HAL_ISR_COUNT; i++ ) {
        hal_interrupt_handlers [i] = (CYG_ADDRESS)&hal_default_isr;
        hal_interrupt_data     [i] = 0;
        hal_interrupt_objects  [i] = 0;
    }

    // Interrupt mask shadow variable initialization
    hal_interrupt_sr_mask_shadow = 0;

    // Enable the IBUS arbiter so that internal devices can work
    // (Ie. USB and ether)
    *S_GMR |= S_GMR_IAEN;

    // Enable writing to the flash device per se (ie. no SEGV)
    *RMMDR |= RMMDR_FLASH_WRITE_ENABLE;

    // Mask off external interrupt sources and clear any pending.
    *S_IMR = 0;
    i = *S_ISR;
    // Enable sysctl interrupt (in status reg) for those external sources.
    // "True" enable is in the external control reg, handled generically
    // by HAL_INTERRUPT_UNMASK for those vector numbers.
    HAL_INTERRUPT_ATTACH( CYGNUM_HAL_INTERRUPT_SYSCTL, &_arbitration_isr, 0, 0);
    HAL_INTERRUPT_UNMASK( CYGNUM_HAL_INTERRUPT_SYSCTL );

#ifdef CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT
    hal_if_init();
#endif

    // The uPD985xx only has an enable that works on both caches. So we
    // only need to enable one of them for both to work.
    HAL_DCACHE_ENABLE();
    // HAL_ICACHE_ENABLE();
}

// ------------------------------------------------------------------------
//
// Routines to handle soft-masking and calling pent-up interrupts.
//

#ifdef CYGOPT_HAL_MIPS_UPD985XX_HARDWARE_BUGS_S2

// Assume all these are called with interrupts disabled globally

// We deal in vector numbers throughout: these contain "true" vector
// number shifts, whereas we have to shift hardware regs before use.
static int once_mask = 0; // Those unmasked ever
static int soft_mask = 0; // Those unmasked right now
static int pent_mask = 0; // Those pending right now

void cyg_hal_interrupt_mask( int vec )
{
    CYG_ASSERT( CYGNUM_HAL_INTERRUPT_SYSCTL_LOW <= vec, "vec underflow" );
    CYG_ASSERT( CYGNUM_HAL_INTERRUPT_SYSCTL_HI  >= vec, "vec overflow" );
    // We only manipulate the soft mask - NEVER the S_IMR.
    soft_mask &=~ (1<<vec);
}

void cyg_hal_interrupt_unmask( int vec )
{
    CYG_ASSERT( CYGNUM_HAL_INTERRUPT_SYSCTL_LOW <= vec, "vec underflow" );
    CYG_ASSERT( CYGNUM_HAL_INTERRUPT_SYSCTL_HI  >= vec, "vec overflow" );
    // If this is the very first time of unmasking, unmask in the S_IMR &c
    // also.
    if ( 0 == ( (1<<vec) & once_mask ) ) {
        once_mask |= (1<<vec);
        *S_IMR |= (1 << ((vec - CYGNUM_HAL_INTERRUPT_SYSCTL_LOW)));
    }
    // We manipulate the soft mask and call any pent-up interrupt
    soft_mask |= (1<<vec);
    if ( (1<<vec) & pent_mask ) {
        pent_mask &=~ (1<<vec); // "Acknowledge" the pent-up interrupt
        hal_call_isr( vec ); // this does it all!
    }
}

void cyg_hal_interrupt_acknowledge( int vec )
{
    CYG_ASSERT( CYGNUM_HAL_INTERRUPT_SYSCTL_LOW <= vec ||
                CYGNUM_HAL_INTERRUPT_SYSCTL     == vec, "vec underflow" );
    CYG_ASSERT( CYGNUM_HAL_INTERRUPT_SYSCTL_HI  >= vec, "vec overflow" );
    // (no harm done if this is CYGNUM_HAL_INTERRUPT_SYSCTL)
    pent_mask &=~ (1<<vec); // "Acknowledge" the pent-up interrupt
}


static cyg_uint32 _arbitration_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data)
{
    cyg_uint32 s_isr;
    int vecnum;
    cyg_uint32 isr_ret = 0;
    // decode interrupt source and for each active source call the ISR
   
    s_isr = *S_ISR;                     // Read once only

    s_isr <<= CYGNUM_HAL_INTERRUPT_SYSCTL_LOW;
    // Ignore the hardware mask; use the soft mask

    // Any that are set in S_ISR and masked out in soft_mask become
    // pending:
    pent_mask |= (s_isr & ~soft_mask);

    s_isr &= soft_mask;                    // Keep unmasked bits

    for ( vecnum = CYGNUM_HAL_INTERRUPT_SYSCTL_LOW;
          vecnum <=  CYGNUM_HAL_INTERRUPT_SYSCTL_HI;
          vecnum++ )
        if ( (1<<vecnum) & s_isr )
            isr_ret |= hal_call_isr( vecnum );
    
    return isr_ret & ~CYG_ISR_CALL_DSR; // Since we have no DSR.
}

#endif // CYGOPT_HAL_MIPS_UPD985XX_HARDWARE_BUGS_S2

/*------------------------------------------------------------------------*/
/* End of var_misc.c                                                      */
