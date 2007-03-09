#ifndef CYGONCE_HAL_HAL_ARBITER_H
#define CYGONCE_HAL_HAL_ARBITER_H

//=============================================================================
//
//      hal_arbiter.h
//
//      Functionality used by ISR arbiters
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   jskov
// Contributors:jskov
// Date:        2001-06-29
// Purpose:     Functionality used by ISR arbiters
// Usage:       #include <cyg/hal/hal_arbiter.h>
//                           
//####DESCRIPTIONEND####
//
//=============================================================================

#include <cyg/hal/hal_intr.h>           // hal_interrupt_x tables
#include <cyg/hal/drv_api.h>            // CYG_ISR_HANDLED

//=============================================================================
// Function used to call ISRs from ISR arbiters
// An arbiter is hooked on the shared interrupt vector and looks like this:
//
//  cyg_uint32 _arbitration_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data)
//  {
//     cyg_uint32 isr_ret;
//     // decode interrupt source and for each active source call the ISR
//     if (source_A_active) {
//         isr_ret = hal_call_isr (CYGNUM_HAL_INTERRUPT_SOURCE_A);
//  #ifdef CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN
//         if (isr_ret & CYG_ISR_HANDLED)
//  #endif
//             return isr_ret;
//     }
//     if (source_B_active) {
//         isr_ret = hal_call_isr (CYGNUM_HAL_INTERRUPT_SOURCE_B);
//  #ifdef CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN
//         if (isr_ret & CYG_ISR_HANDLED)
//  #endif
//             return isr_ret;
//     }
//  ...
//     return 0;
//  }
//
// Remember to attach and enable the arbiter source:
//    HAL_INTERRUPT_ATTACH(CYGNUM_HAL_INTERRUPT_ARBITER, &_arbitration_isr, 0, 0);
//    HAL_INTERRUPT_SET_LEVEL(CYGNUM_HAL_INTERRUPT_ARBITER, 1);
//    HAL_INTERRUPT_UNMASK(CYGNUM_HAL_INTERRUPT_ARBITER);
//

typedef cyg_uint32 cyg_ISR(cyg_uint32 vector, CYG_ADDRWORD data);

extern void cyg_interrupt_post_dsr( CYG_ADDRWORD intr_obj );

#ifndef CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN

static inline cyg_uint32
hal_call_isr (cyg_uint32 vector)
{
    cyg_ISR *isr;
    CYG_ADDRWORD data;
    cyg_uint32 isr_ret;

    isr = (cyg_ISR*) hal_interrupt_handlers[vector];
    data = hal_interrupt_data[vector];

    isr_ret = (*isr) (vector, data);

    if (isr_ret & CYG_ISR_CALL_DSR) {
        cyg_interrupt_post_dsr (hal_interrupt_objects[vector]);
    }

    return isr_ret & ~CYG_ISR_CALL_DSR;
}

#else

// In chained mode, assume vector 0 points to the chain
// handler. Simply call it with the vector number and let it find the
// ISR to call - it will also post DSRs as required.
static inline cyg_uint32
hal_call_isr (cyg_uint32 vector)
{
    cyg_ISR *isr;
    CYG_ADDRWORD data;
    cyg_uint32 isr_ret;

    isr = (cyg_ISR*) hal_interrupt_handlers[0];
    data = hal_interrupt_data[0];

    isr_ret = (*isr) (vector, data);

    return isr_ret;
}

#endif

//-----------------------------------------------------------------------------
#endif // CYGONCE_HAL_HAL_ARBITER_H
// End of hal_arbiter.h
