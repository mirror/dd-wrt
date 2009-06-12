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
// Copyright (C) 2002 Gary Thomas
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
// Contributors: msalter, gthomas
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

#include <cyg/hal/hal_iop310.h>         // registers
#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_platform_ints.h>  // Platform overrides, setups, etc

// *** 80200 CPU ***
#define CYGNUM_HAL_INTERRUPT_reserved0     0
#define CYGNUM_HAL_INTERRUPT_PMU_PMN0_OVFL 1 // See Ch.12 - Performance Mon.
#define CYGNUM_HAL_INTERRUPT_PMU_PMN1_OVFL 2 // PMU counter 0/1 overflow
#define CYGNUM_HAL_INTERRUPT_PMU_CCNT_OVFL 3 // PMU clock overflow
#define CYGNUM_HAL_INTERRUPT_BCU_INTERRUPT 4 // See Ch.11 - Bus Control Unit
#define CYGNUM_HAL_INTERRUPT_NIRQ          5 // external IRQ
#define CYGNUM_HAL_INTERRUPT_NFIQ          6 // external FIQ

// *** XINT6 interrupts ***
#define CYGNUM_HAL_INTERRUPT_DMA_0         7
#define CYGNUM_HAL_INTERRUPT_DMA_1         8
#define CYGNUM_HAL_INTERRUPT_DMA_2         9
#define CYGNUM_HAL_INTERRUPT_GTSC         10 // Global Time Stamp Counter
#define CYGNUM_HAL_INTERRUPT_PEC          11 // Performance Event Counter
#define CYGNUM_HAL_INTERRUPT_AAIP         12 // application accelerator unit

// *** XINT7 interrupts ***
// I2C interrupts
#define CYGNUM_HAL_INTERRUPT_I2C_TX_EMPTY 13
#define CYGNUM_HAL_INTERRUPT_I2C_RX_FULL  14
#define CYGNUM_HAL_INTERRUPT_I2C_BUS_ERR  15
#define CYGNUM_HAL_INTERRUPT_I2C_STOP     16
#define CYGNUM_HAL_INTERRUPT_I2C_LOSS     17
#define CYGNUM_HAL_INTERRUPT_I2C_ADDRESS  18
// Messaging Unit interrupts
#define CYGNUM_HAL_INTERRUPT_MESSAGE_0           19
#define CYGNUM_HAL_INTERRUPT_MESSAGE_1           20
#define CYGNUM_HAL_INTERRUPT_DOORBELL            21
#define CYGNUM_HAL_INTERRUPT_NMI_DOORBELL        22 // FIQ
#define CYGNUM_HAL_INTERRUPT_QUEUE_POST          23
#define CYGNUM_HAL_INTERRUPT_OUTBOUND_QUEUE_FULL 24 // FIQ
#define CYGNUM_HAL_INTERRUPT_INDEX_REGISTER      25
// PCI Address Translation Unit
#define CYGNUM_HAL_INTERRUPT_BIST                26

// *** External board interrupts (XINT3) ***
#define CYGNUM_HAL_INTERRUPT_XINT3_BIT0   27
#define CYGNUM_HAL_INTERRUPT_XINT3_BIT1   28
#define CYGNUM_HAL_INTERRUPT_XINT3_BIT2   29
#define CYGNUM_HAL_INTERRUPT_XINT3_BIT3   30
#define CYGNUM_HAL_INTERRUPT_XINT3_BIT4   31
#define CYGNUM_HAL_INTERRUPT_XINT3_BITS    5 // Number of significant bits

// *** NMI Interrupts go to FIQ ***
#define CYGNUM_HAL_INTERRUPT_MCU_ERR       35
#define CYGNUM_HAL_INTERRUPT_PATU_ERR      36
#define CYGNUM_HAL_INTERRUPT_SATU_ERR      37
#define CYGNUM_HAL_INTERRUPT_PBDG_ERR      38
#define CYGNUM_HAL_INTERRUPT_SBDG_ERR      39
#define CYGNUM_HAL_INTERRUPT_DMA0_ERR      40
#define CYGNUM_HAL_INTERRUPT_DMA1_ERR      41
#define CYGNUM_HAL_INTERRUPT_DMA2_ERR      42
#define CYGNUM_HAL_INTERRUPT_MU_ERR        43
#define CYGNUM_HAL_INTERRUPT_reserved52    44
#define CYGNUM_HAL_INTERRUPT_AAU_ERR       45
#define CYGNUM_HAL_INTERRUPT_BIU_ERR       46

// *** ATU FIQ sources ***
#define CYGNUM_HAL_INTERRUPT_P_SERR        47
#define CYGNUM_HAL_INTERRUPT_S_SERR        48

#define CYGNUM_HAL_ISR_MIN               0
#define CYGNUM_HAL_ISR_MAX               48

#define CYGNUM_HAL_ISR_COUNT            (CYGNUM_HAL_ISR_MAX+1)

externC void hal_delay_us(cyg_uint32 usecs);
#define HAL_DELAY_US(n)          hal_delay_us(n);

//----------------------------------------------------------------------------
// Reset.

// FIXME - Can we reset the board?
#define HAL_PLATFORM_RESET()   CYG_EMPTY_STATEMENT

#define HAL_PLATFORM_RESET_ENTRY 0x00000000

// ------------------------------------------------------------------------
// Dynamically set the timer interrupt rate.
// Not for application use at all.

externC void
hal_clock_reinitialize(          int *pfreq,    /* inout */
                        unsigned int *pperiod,  /* inout */
                        unsigned int old_hz );  /* in */

#define HAL_CLOCK_REINITIALIZE( _freq, _period, _old_hz ) \
        hal_clock_reinitialize( &_freq, &_period, _old_hz )

#endif // CYGONCE_HAL_VAR_INTS_H
// EOF hal_var_ints.h
