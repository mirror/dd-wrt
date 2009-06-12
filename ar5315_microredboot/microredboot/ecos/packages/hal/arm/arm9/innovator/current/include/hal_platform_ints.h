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
// Author(s):    Patrick Doyle <wpd@delcomsys.com>
// Contributors: Patrick Doyle <wpd@delcomsys.com>
// Date:         2002-12-01
// Purpose:      Define Interrupt support
// Description:  The interrupt details for the Innovator will someday
//               be defined here.
// Usage:
//               #include <cyg/hal/hal_platform_ints.h>
//               ...
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/hal/innovator.h>

// These are interrupts on the Innovator
// TBD
#define CYGNUM_HAL_ISR_COUNT 1 // No interrupt support defined yet.

// Placeholders
#define CYGNUM_HAL_ISR_MIN            0
#define CYGNUM_HAL_ISR_MAX            0
// The vector used by the Real time clock -- fake, for now
#define CYGNUM_HAL_INTERRUPT_RTC      0


//----------------------------------------------------------------------------
// Reset.

// Enable the watchdog timer to force a reset
#define HAL_PLATFORM_RESET() CYG_MACRO_START            \
  cyg_uint16 tmp;                                       \
  HAL_READ_UINT16(CLKM_ARM_IDLECT2, tmp);               \
  HAL_WRITE_UINT16(CLKM_ARM_IDLECT2, tmp | 1);          \
  HAL_DELAY_US(4);                                      \
  HAL_WRITE_UINT16(WATCHDOG_TIMER_MODE, 0x80F5);        \
  HAL_DELAY_US(4);                                      \
  HAL_WRITE_UINT16(WATCHDOG_TIMER_MODE, 0x80F5);        \
CYG_MACRO_END

#define HAL_PLATFORM_RESET_ENTRY 0x10000000

#endif // CYGONCE_HAL_PLATFORM_INTS_H
