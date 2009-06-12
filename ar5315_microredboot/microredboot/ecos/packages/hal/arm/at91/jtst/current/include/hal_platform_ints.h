#ifndef CYGONCE_HAL_PLATFORM_INTS_H
#define CYGONCE_HAL_PLATFORM_INTS_H
//==========================================================================
//
//      hal_platform_ints.h
//
//      HAL Interrupt and clock assignments for JTST
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
// Author(s):    amichelotti
// Contributors: gthomas
// Date:         2001-07-12
// Purpose:      Define Interrupt support
// Description:  The interrupt specifics for the JTST board/platform are
//               defined here.
//
// Usage:        #include <cyg/hal/hal_platform_ints.h>
//               ...
//
//
//####DESCRIPTIONEND####
//
//==========================================================================

// JTST MAGICHLT is the AIC FIQ that's connected to the halt signal of
// the magic DSP
#define CYGNUM_HAL_INTERRUPT_MAGICHLT 0

#define CYGNUM_HAL_INTERRUPT_TIMER0   1
#define CYGNUM_HAL_INTERRUPT_TIMER1   2
#define CYGNUM_HAL_INTERRUPT_TIMER2   3
#define CYGNUM_HAL_INTERRUPT_USART0   4
#define CYGNUM_HAL_INTERRUPT_USART1   5
#define CYGNUM_HAL_INTERRUPT_SPI0     6
#define CYGNUM_HAL_INTERRUPT_SPI1     7
#define CYGNUM_HAL_INTERRUPT_SIRQ0    8
#define CYGNUM_HAL_INTERRUPT_SIRQ1    9
#define CYGNUM_HAL_INTERRUPT_SIRQ2    10
#define CYGNUM_HAL_INTERRUPT_SIRQ3    11
#define CYGNUM_HAL_INTERRUPT_SIRQ4    12
#define CYGNUM_HAL_INTERRUPT_SIRQ5    13
//exception signal from DSP
#define CYGNUM_HAL_INTERRUPT_MAGICEX  15
#define CYGNUM_HAL_INTERRUPT_WATCHDOG 16
#define CYGNUM_HAL_INTERRUPT_PIO      17
#define CYGNUM_HAL_INTERRUPT_EXT4     24
//change mode signal system/run and run/system from DSP
//I/O operation are allowed only in system mode 
#define CYGNUM_HAL_INTERRUPT_MAGICMOD 25
#define CYGNUM_HAL_INTERRUPT_EXT5     26
// endTX from DSP dma
#define CYGNUM_HAL_INTERRUPT_ENDTXRX  27
#define CYGNUM_HAL_INTERRUPT_EXT0     28
#define CYGNUM_HAL_INTERRUPT_EXT1     29
#define CYGNUM_HAL_INTERRUPT_EXT2     30
#define CYGNUM_HAL_INTERRUPT_EXT3     31

#define CYGNUM_HAL_ISR_MIN 0
#define CYGNUM_HAL_ISR_MAX 31
#define CYGNUM_HAL_ISR_COUNT                  (CYGNUM_HAL_ISR_MAX + 1)

// The vector used by the Real time clock
#define CYGNUM_HAL_INTERRUPT_RTC               CYGNUM_HAL_INTERRUPT_TIMER0


//----------------------------------------------------------------------------
// Reset.
__externC void hal_at91_reset_cpu(void);
#define HAL_PLATFORM_RESET() hal_at91_reset_cpu()

#define HAL_PLATFORM_RESET_ENTRY 0x01010000

#endif // CYGONCE_HAL_PLATFORM_INTS_H
