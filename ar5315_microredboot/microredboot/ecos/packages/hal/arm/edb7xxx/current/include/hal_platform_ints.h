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
// Author(s):    gthomas
// Contributors: gthomas
// Date:         1999-04-21
// Purpose:      Define Interrupt support
// Description:  The interrupt details for the CL7xxx are defined here.
// Usage:
//               #include <cyg/hal/hal_platform_ints.h>
//               ...
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#define CYGNUM_HAL_INTERRUPT_unused     0
#define CYGNUM_HAL_INTERRUPT_EXTFIQ     1
#define CYGNUM_HAL_INTERRUPT_BLINT      2
#define CYGNUM_HAL_INTERRUPT_WEINT      3
#define CYGNUM_HAL_INTERRUPT_MCINT      4
#define CYGNUM_HAL_INTERRUPT_CSINT      5
#define CYGNUM_HAL_INTERRUPT_EINT1      6    
#define CYGNUM_HAL_INTERRUPT_EINT2      7
#define CYGNUM_HAL_INTERRUPT_EINT3      8
#define CYGNUM_HAL_INTERRUPT_TC1OI      9
#define CYGNUM_HAL_INTERRUPT_TC2OI      10
#define CYGNUM_HAL_INTERRUPT_RTCMI      11
#define CYGNUM_HAL_INTERRUPT_TINT       12
#define CYGNUM_HAL_INTERRUPT_UTXINT1    13
#define CYGNUM_HAL_INTERRUPT_URXINT1    14
#define CYGNUM_HAL_INTERRUPT_UMSINT     15
#define CYGNUM_HAL_INTERRUPT_SSEOTI     16
#define CYGNUM_HAL_INTERRUPT_KBDINT     17
#define CYGNUM_HAL_INTERRUPT_SS2RX      18
#define CYGNUM_HAL_INTERRUPT_SS2TX      19
#define CYGNUM_HAL_INTERRUPT_UTXINT2    20
#define CYGNUM_HAL_INTERRUPT_URXINT2    21
#if defined(__EDB7211)
#define CYGNUM_HAL_INTERRUPT_MCPINT     22
#endif
#if defined(__EDB7209)
#define CYGNUM_HAL_INTERRUPT_I2SINT     22
#endif
#if defined(__EDB7312)
#define CYGNUM_HAL_INTERRUPT_DAIINT     22
#endif

#define CYGNUM_HAL_ISR_MIN              0
#if defined(__CL7111)
#define CYGNUM_HAL_ISR_MAX              21
#else
#define CYGNUM_HAL_ISR_MAX              22
#endif
#define CYGNUM_HAL_ISR_COUNT            (CYGNUM_HAL_ISR_MAX+1)

// The vector used by the Real time clock
#define CYGNUM_HAL_INTERRUPT_RTC        CYGNUM_HAL_INTERRUPT_TC2OI



//----------------------------------------------------------------------------
// Reset.

// Try and force the board into a reset state.  Since this hardware requires
// a "wakeup" signal, we can't just use a watchdog/reset approach.
externC void reset_platform(void);
#define HAL_PLATFORM_RESET() reset_platform()

#define HAL_PLATFORM_RESET_ENTRY 0xe0000000

#endif // CYGONCE_HAL_PLATFORM_INTS_H
