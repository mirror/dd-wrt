#ifndef CYGONCE_HAL_VAR_INTS_H
#define CYGONCE_HAL_VAR_INTS_H
//==========================================================================
//
//      hal_var_ints.h
//
//      HAL Interrupt and clock support
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
// Author(s):    msalter
// Contributors: msalter
// Date:         2001-12-03
// Purpose:      Define Interrupt support
// Description:  The interrupt details for XScale CPUs are defined here.
// Usage:
//		 #include <pkgconf/system.h>
//		 #include CYGBLD_HAL_VARIANT_H
//               #include CYGBLD_HAL_VAR_INTS_H
//
//               ...
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/hal/hal_verde.h>         // registers

//
// Values for the vector argument of cyg_drv_interrupt_create() and
// other interrupt related interfaces.
//
#define CYGNUM_HAL_INTERRUPT_NONE         -1
#define CYGNUM_HAL_INTERRUPT_DMA0_EOT      0
#define CYGNUM_HAL_INTERRUPT_DMA0_EOC      1
#define CYGNUM_HAL_INTERRUPT_DMA1_EOT      2
#define CYGNUM_HAL_INTERRUPT_DMA1_EOC      3
#define CYGNUM_HAL_INTERRUPT_RSVD_4        4
#define CYGNUM_HAL_INTERRUPT_RSVD_5        5
#define CYGNUM_HAL_INTERRUPT_AA_EOT        6
#define CYGNUM_HAL_INTERRUPT_AA_EOC        7
#define CYGNUM_HAL_INTERRUPT_CORE_PMON     8
#define CYGNUM_HAL_INTERRUPT_TIMER0        9
#define CYGNUM_HAL_INTERRUPT_TIMER1        10
#define CYGNUM_HAL_INTERRUPT_I2C_0         11
#define CYGNUM_HAL_INTERRUPT_I2C_1         12
#define CYGNUM_HAL_INTERRUPT_MESSAGING     13
#define CYGNUM_HAL_INTERRUPT_ATU_BIST      14
#define CYGNUM_HAL_INTERRUPT_PERFMON       15
#define CYGNUM_HAL_INTERRUPT_CORE_PMU      16
#define CYGNUM_HAL_INTERRUPT_BIU_ERR       17
#define CYGNUM_HAL_INTERRUPT_ATU_ERR       18
#define CYGNUM_HAL_INTERRUPT_MCU_ERR       19
#define CYGNUM_HAL_INTERRUPT_DMA0_ERR      20
#define CYGNUM_HAL_INTERRUPT_DMA1_ERR      21
#define CYGNUM_HAL_INTERRUPT_RSVD_22       22
#define CYGNUM_HAL_INTERRUPT_AA_ERR        23
#define CYGNUM_HAL_INTERRUPT_MSG_ERR       24
#define CYGNUM_HAL_INTERRUPT_SSP           25
#define CYGNUM_HAL_INTERRUPT_MSG_IBPQ      26
#define CYGNUM_HAL_INTERRUPT_XINT0         27
#define CYGNUM_HAL_INTERRUPT_XINT1         28
#define CYGNUM_HAL_INTERRUPT_XINT2         29
#define CYGNUM_HAL_INTERRUPT_XINT3         30
#define CYGNUM_HAL_INTERRUPT_HPI           31

#define CYGNUM_HAL_VAR_ISR_MAX  31

#define CYGNUM_HAL_ISR_MIN      0
#define CYGNUM_HAL_ISR_MAX      31

#define CYGNUM_HAL_ISR_COUNT    (CYGNUM_HAL_ISR_MAX+1)

#define CYGNUM_HAL_INTERRUPT_RTC  CYGNUM_HAL_INTERRUPT_TIMER0

#endif // CYGONCE_HAL_VAR_INTS_H

// ------------------------------------------------------------------------
// Dynamically set the timer interrupt rate.
// Not for application use at all.

externC void
hal_clock_reinitialize(          int *pfreq,    /* inout */
                        unsigned int *pperiod,  /* inout */
                        unsigned int old_hz );  /* in */

#define HAL_CLOCK_REINITIALIZE( _freq, _period, _old_hz ) \
        hal_clock_reinitialize( &_freq, &_period, _old_hz )

//----------------------------------------------------------------------------
// Reset.
#define HAL_PLATFORM_RESET()                                               \
    CYG_MACRO_START                                                        \
    cyg_uint32 ctrl;                                                       \
                                                                           \
    /* By disabling interupts we will just hang in the loop below      */  \
    /* if for some reason the software reset fails.                    */  \
    HAL_DISABLE_INTERRUPTS(ctrl);                                          \
                                                                           \
    *ATU_PCSR = PCSR_RESET_I_BUS | PCSR_RESET_P_BUS;                       \
                                                                           \
    for(;;); /* hang here forever if reset fails */                        \
    CYG_MACRO_END

// Fallback (never really used)
#define HAL_PLATFORM_RESET_ENTRY 0x00000000

// EOF hal_var_ints.h
