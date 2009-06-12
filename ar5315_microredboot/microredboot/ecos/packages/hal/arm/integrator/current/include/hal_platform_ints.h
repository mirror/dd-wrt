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
// Author(s):    David A Rusling
// Contributors: Philippe Robin
// Date:         November 7, 2000
// Purpose:      Define Interrupt support
// Description:  The interrupt details for the INTEGRATOR are defined here.
// Usage:
//               #include <cyg/hal/hal_platform_ints.h>
//               ...
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#define CYGNUM_HAL_INTERRUPT_SOFTINT                     0
#define CYGNUM_HAL_INTERRUPT_UARTINT0                    1
#define CYGNUM_HAL_INTERRUPT_UARTINT1                    2
#define CYGNUM_HAL_INTERRUPT_KMIINT0                     3
#define CYGNUM_HAL_INTERRUPT_KMIINT1                     4
#define CYGNUM_HAL_INTERRUPT_TIMERINT0                   5
#define CYGNUM_HAL_INTERRUPT_TIMERINT1                   6
#define CYGNUM_HAL_INTERRUPT_TIMERINT2                   7
#define CYGNUM_HAL_INTERRUPT_RTCINT                      8
#define CYGNUM_HAL_INTERRUPT_EXPINT0                     9
#define CYGNUM_HAL_INTERRUPT_EXPINT1                     10
#define CYGNUM_HAL_INTERRUPT_EXPINT2                     11
#define CYGNUM_HAL_INTERRUPT_EXPINT3                     12
#define CYGNUM_HAL_INTERRUPT_PCIINT0                     13
#define CYGNUM_HAL_INTERRUPT_PCIINT1                     14
#define CYGNUM_HAL_INTERRUPT_PCIINT2                     15
#define CYGNUM_HAL_INTERRUPT_PCIINT3                     16
#define CYGNUM_HAL_INTERRUPT_V3INT                       17
#define CYGNUM_HAL_INTERRUPT_CPINT0                      18
#define CYGNUM_HAL_INTERRUPT_CPINT1                      19
#define CYGNUM_HAL_INTERRUPT_LBUSTIMEOUT                 20
#define CYGNUM_HAL_INTERRUPT_APCINT                      21
#define CYGNUM_HAL_INTERRUPT_CM_SOFTINT                  24
#define CYGNUM_HAL_INTERRUPT_CM_COMMRX                   25
#define CYGNUM_HAL_INTERRUPT_CM_COMMTX                   26

#define CYGNUM_HAL_ISR_MIN                        0
#define CYGNUM_HAL_ISR_MAX                        26
#define CYGNUM_HAL_ISR_COUNT                      27

// The vector used by the Real time clock
#define CYGNUM_HAL_INTERRUPT_RTC                  CYGNUM_HAL_INTERRUPT_TIMERINT2

//----------------------------------------------------------------------------
// Microsecond delay support.

externC void hal_delay_us(cyg_uint32 usecs);
#define HAL_DELAY_US(n)          hal_delay_us(n);

//----------------------------------------------------------------------------
// Reset.

#define HAL_PLATFORM_RESET()			\
{						\
    HAL_WRITE_UINT8( INTEGRATOR_SC_CTRLS , 1 );	\
}

#define HAL_PLATFORM_RESET_ENTRY 0x24000000

//----------------------------------------------------------------------------

#endif // CYGONCE_HAL_PLATFORM_INTS_H
