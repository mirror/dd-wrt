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
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2000-05-08
// Purpose:      Define Interrupt support
// Description:  The interrupt details for the SA1110/Assabet are defined here.
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

#include <cyg/hal/hal_sa11x0.h>         // registers

#define CYGNUM_HAL_INTERRUPT_GPIO0   0
#define CYGNUM_HAL_INTERRUPT_GPIO1   1
#define CYGNUM_HAL_INTERRUPT_GPIO2   2
#define CYGNUM_HAL_INTERRUPT_GPIO3   3
#define CYGNUM_HAL_INTERRUPT_GPIO4   4
#define CYGNUM_HAL_INTERRUPT_GPIO5   5
#define CYGNUM_HAL_INTERRUPT_GPIO6   6
#define CYGNUM_HAL_INTERRUPT_GPIO7   7
#define CYGNUM_HAL_INTERRUPT_GPIO8   8
#define CYGNUM_HAL_INTERRUPT_GPIO9   9
#define CYGNUM_HAL_INTERRUPT_GPIO10  10
#define CYGNUM_HAL_INTERRUPT_GPIO    11  // Don't use directly!
#define CYGNUM_HAL_INTERRUPT_LCD     12
#define CYGNUM_HAL_INTERRUPT_UDC     13
#define CYGNUM_HAL_INTERRUPT_UART1   15
#define CYGNUM_HAL_INTERRUPT_UART2   16
#define CYGNUM_HAL_INTERRUPT_UART3   17
#define CYGNUM_HAL_INTERRUPT_MCP     18
#define CYGNUM_HAL_INTERRUPT_SSP     19
#define CYGNUM_HAL_INTERRUPT_TIMER0  26
#define CYGNUM_HAL_INTERRUPT_TIMER1  27
#define CYGNUM_HAL_INTERRUPT_TIMER2  28
#define CYGNUM_HAL_INTERRUPT_TIMER3  29
#define CYGNUM_HAL_INTERRUPT_HZ      30
#define CYGNUM_HAL_INTERRUPT_ALARM   31

// GPIO bits 31..11 can generate interrupts as well, but they all
// end up clumped into interrupt signal #11.  Using the symbols
// below allow for detection of these separately.

#define CYGNUM_HAL_INTERRUPT_GPIO11  (32+11)
#define CYGNUM_HAL_INTERRUPT_GPIO12  (32+12)
#define CYGNUM_HAL_INTERRUPT_GPIO13  (32+13)
#define CYGNUM_HAL_INTERRUPT_GPIO14  (32+14)
#define CYGNUM_HAL_INTERRUPT_GPIO15  (32+15)
#define CYGNUM_HAL_INTERRUPT_GPIO16  (32+16)
#define CYGNUM_HAL_INTERRUPT_GPIO17  (32+17)
#define CYGNUM_HAL_INTERRUPT_GPIO18  (32+18)
#define CYGNUM_HAL_INTERRUPT_GPIO19  (32+19)
#define CYGNUM_HAL_INTERRUPT_GPIO20  (32+20)
#define CYGNUM_HAL_INTERRUPT_GPIO21  (32+21)
#define CYGNUM_HAL_INTERRUPT_GPIO22  (32+22)
#define CYGNUM_HAL_INTERRUPT_GPIO23  (32+23)
#define CYGNUM_HAL_INTERRUPT_GPIO24  (32+24)
#define CYGNUM_HAL_INTERRUPT_GPIO25  (32+25)
#define CYGNUM_HAL_INTERRUPT_GPIO26  (32+26)
#define CYGNUM_HAL_INTERRUPT_GPIO27  (32+27)

#define CYGNUM_HAL_INTERRUPT_NONE    -1

#define CYGNUM_HAL_ISR_MIN            0
#define CYGNUM_HAL_ISR_MAX           (27+32)

#define CYGNUM_HAL_ISR_COUNT            (CYGNUM_HAL_ISR_MAX+1)

// The vector used by the Real time clock
#define CYGNUM_HAL_INTERRUPT_RTC        CYGNUM_HAL_INTERRUPT_TIMER0

// SA11x0 method for reading clock interrupt latency
#ifdef CYGVAR_KERNEL_COUNTERS_CLOCK_LATENCY
externC void hal_clock_latency(cyg_uint32 *);
# define HAL_CLOCK_LATENCY( _pvalue_ ) \
         hal_clock_latency( (cyg_uint32 *)(_pvalue_) )
#endif


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
    /* Software reset. */                                                  \
    *SA11X0_RESET_SOFTWARE_RESET = SA11X0_INVOKE_SOFTWARE_RESET;           \
                                                                           \
    for(;;); /* hang here forever if reset fails */                        \
    CYG_MACRO_END

// Fallback (never really used)
#define HAL_PLATFORM_RESET_ENTRY 0x00000000

#endif // CYGONCE_HAL_VAR_INTS_H
