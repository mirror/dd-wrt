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
// Author(s):    michael anburaj <michaelanburaj@hotmail.com>
// Contributors: michael anburaj <michaelanburaj@hotmail.com>
// Date:         2003-08-01
// Purpose:      Define Interrupt support
// Description:  The interrupt details for the Samsung SMDK2410 are defined here.
// Usage:
//               #include <cyg/hal/hal_platform_ints.h>
//               ...
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/hal/s3c2410x.h>

// These are interrupts on the SMDK2410

#define CYGNUM_HAL_INTERRUPT_EINT0      0
#define CYGNUM_HAL_INTERRUPT_EINT1      1
#define CYGNUM_HAL_INTERRUPT_EINT2      2
#define CYGNUM_HAL_INTERRUPT_EINT3      3
#define CYGNUM_HAL_INTERRUPT_EINT4_7    4
#define CYGNUM_HAL_INTERRUPT_EINT8_23   5    
#define CYGNUM_HAL_INTERRUPT_NOTUSED6   6
#define CYGNUM_HAL_INTERRUPT_BAT_FLT    7
#define CYGNUM_HAL_INTERRUPT_TICK       8
#define CYGNUM_HAL_INTERRUPT_WDT        9
#define CYGNUM_HAL_INTERRUPT_TIMER0     10
#define CYGNUM_HAL_INTERRUPT_TIMER1     11
#define CYGNUM_HAL_INTERRUPT_TIMER2     12
#define CYGNUM_HAL_INTERRUPT_TIMER3     13
#define CYGNUM_HAL_INTERRUPT_TIMER4     14
#define CYGNUM_HAL_INTERRUPT_UART2      15
#define CYGNUM_HAL_INTERRUPT_LCD        16
#define CYGNUM_HAL_INTERRUPT_DMA0       17
#define CYGNUM_HAL_INTERRUPT_DMA1       18
#define CYGNUM_HAL_INTERRUPT_DMA2       19
#define CYGNUM_HAL_INTERRUPT_DMA3       20
#define CYGNUM_HAL_INTERRUPT_SDI        21
#define CYGNUM_HAL_INTERRUPT_SPI0       22
#define CYGNUM_HAL_INTERRUPT_UART1      23
#define CYGNUM_HAL_INTERRUPT_NOTUSED24  24
#define CYGNUM_HAL_INTERRUPT_USBD       25
#define CYGNUM_HAL_INTERRUPT_USBH       26
#define CYGNUM_HAL_INTERRUPT_IIC        27
#define CYGNUM_HAL_INTERRUPT_UART0      28
#define CYGNUM_HAL_INTERRUPT_SPI1       29
#define CYGNUM_HAL_INTERRUPT_RTCC       30
#define CYGNUM_HAL_INTERRUPT_ADC        31

#define CYGNUM_HAL_INTERRUPT_NONE    -1

#define CYGNUM_HAL_ISR_MIN            0
#define CYGNUM_HAL_ISR_MAX            CYGNUM_HAL_INTERRUPT_ADC
#define CYGNUM_HAL_ISR_COUNT          (CYGNUM_HAL_ISR_MAX-CYGNUM_HAL_ISR_MIN+1)

// The vector used by the Real time clock
#define CYGNUM_HAL_INTERRUPT_RTC      CYGNUM_HAL_INTERRUPT_TIMER4



//----------------------------------------------------------------------------
// Reset.

// Watchdog is started with a very small delay to Reset immediatly.
#define HAL_PLATFORM_RESET()                                     \
do {                                                             \
    HAL_WRITE_UINT32(WTCON, 0);                                  \
    HAL_WRITE_UINT32(WTDAT, 1);                                  \
    HAL_WRITE_UINT32(WTCON, (1<<0)|(0<<2)|(0<<3)|(1<<5)|(0<<8)); \
} while(0)

// Base of flash
#define HAL_PLATFORM_RESET_ENTRY 0x00000000

#endif // CYGONCE_HAL_PLATFORM_INTS_H
