#ifndef CYGONCE_HAL_PLATFORM_INTS_H
#define CYGONCE_HAL_PLATFORM_INTS_H
//==========================================================================
//
//      hal_platform_ints.h
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
// Author(s):    hmt
// Contributors: hmt
// Date:         1999-04-21
// Purpose:      Define Interrupt support
// Description:  The interrupt details for the EBSA285 are defined here.
// Usage:
//               #include <cyg/hal/hal_platform_ints.h>
//               ...
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#define CYGNUM_HAL_INTERRUPT_reserved0                   0
#define CYGNUM_HAL_INTERRUPT_SOFT_IRQ                    1
#define CYGNUM_HAL_INTERRUPT_SERIAL_RX                   2
#define CYGNUM_HAL_INTERRUPT_SERIAL_TX                   3
#define CYGNUM_HAL_INTERRUPT_TIMER_1                     4
#define CYGNUM_HAL_INTERRUPT_TIMER_2                     5
#define CYGNUM_HAL_INTERRUPT_TIMER_3                     6    
#define CYGNUM_HAL_INTERRUPT_TIMER_4                     7
#define CYGNUM_HAL_INTERRUPT_IRQ_IN_0                    8
#define CYGNUM_HAL_INTERRUPT_IRQ_IN_1                    9
#define CYGNUM_HAL_INTERRUPT_IRQ_IN_2                    10
#define CYGNUM_HAL_INTERRUPT_IRQ_IN_3                    11
#define CYGNUM_HAL_INTERRUPT_XBUS_CS_0                   12
#define CYGNUM_HAL_INTERRUPT_XBUS_CS_1                   13
#define CYGNUM_HAL_INTERRUPT_XBUS_CS_2                   14
#define CYGNUM_HAL_INTERRUPT_DOORBELL                    15
#define CYGNUM_HAL_INTERRUPT_DMA_1                       16
#define CYGNUM_HAL_INTERRUPT_DMA_2                       17
#define CYGNUM_HAL_INTERRUPT_PCI_IRQ                     18
#define CYGNUM_HAL_INTERRUPT_PMCSR                       19
#define CYGNUM_HAL_INTERRUPT_reserved20                  20
#define CYGNUM_HAL_INTERRUPT_reserved21                  21
#define CYGNUM_HAL_INTERRUPT_BIST                        22
#define CYGNUM_HAL_INTERRUPT_SERR                        23
#define CYGNUM_HAL_INTERRUPT_SDRAM_PARITY                24
#define CYGNUM_HAL_INTERRUPT_I2O_POST                    25
#define CYGNUM_HAL_INTERRUPT_reserved26                  26
#define CYGNUM_HAL_INTERRUPT_DISCARD_TIMER               27
#define CYGNUM_HAL_INTERRUPT_PCI_DATA_PARITY             28
#define CYGNUM_HAL_INTERRUPT_PCI_MASTER_ABORT            29
#define CYGNUM_HAL_INTERRUPT_PCI_TARGET_ABORT            30
#define CYGNUM_HAL_INTERRUPT_PCI_PARITY_ERROR            31

#define CYGNUM_HAL_ISR_MIN               0
#define CYGNUM_HAL_ISR_MAX              31

#define CYGNUM_HAL_ISR_COUNT            (CYGNUM_HAL_ISR_MAX+1)

// The vector used by the Real time clock
#define CYGNUM_HAL_INTERRUPT_RTC        CYGNUM_HAL_INTERRUPT_TIMER_3


//----------------------------------------------------------------------------
// Reset.
#include <cyg/hal/hal_ebsa285.h>        // registers
#include <cyg/hal/hal_io.h>             // IO macros

#define HAL_PLATFORM_RESET()                                               \
    CYG_MACRO_START                                                        \
    cyg_uint32 ctrl;                                                       \
                                                                           \
    /* If watchdog is already enabled, writing to timer4 has no effect. */ \
    /* But by disabling interupts and just hanging in the loop below    */ \
    /* the timer might run out eventually (not guaranteed).             */ \
    HAL_DISABLE_INTERRUPTS(ctrl);                                          \
                                                                           \
    /* Set timer4 (must be done before enabling watchdog) */               \
    HAL_WRITE_UINT32(SA110_TIMER4_LOAD, 2);                                \
    HAL_WRITE_UINT32(SA110_TIMER4_CONTROL, SA110_TIMER_CONTROL_ENABLE);    \
                                                                           \
    /* Enable watchdog */                                                  \
    HAL_READ_UINT32(SA110_CONTROL, ctrl);                                  \
    ctrl |= SA110_CONTROL_WATCHDOG;                                        \
    HAL_WRITE_UINT32(SA110_CONTROL, ctrl);                                 \
                                                                           \
    for(;;); /* wait for it */                                             \
    CYG_MACRO_END

#define HAL_PLATFORM_RESET_ENTRY 0x41000000

#endif // CYGONCE_HAL_PLATFORM_INTS_H
