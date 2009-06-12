#ifndef CYGONCE_HAL_PLATFORM_INTS_H
#define CYGONCE_HAL_PLATFORM_INTS_H
//==========================================================================
//
//      hal_platform_ints.h
//
//      HAL Interrupt and clock assignments for AEB-1
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
// Date:         1999-02-20
// Purpose:      Define Interrupt support
// Description:  The interrupt specifics for the AEB-1 board/platform are
//               defined here.
//              
// Usage:        #include <cyg/hal/hal_platform_ints.h>
//               ...
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#define CYGNUM_HAL_INTERRUPT_EXT0              0
#define CYGNUM_HAL_INTERRUPT_EXT1              1
#define CYGNUM_HAL_INTERRUPT_EXT2              2
#define CYGNUM_HAL_INTERRUPT_EXT3              3
#define CYGNUM_HAL_INTERRUPT_TIMER0            6
#define CYGNUM_HAL_INTERRUPT_TIMER1            7
#define CYGNUM_HAL_INTERRUPT_TIMER2            8
#define CYGNUM_HAL_INTERRUPT_UART0             9
#define CYGNUM_HAL_INTERRUPT_UART1            10
#define CYGNUM_HAL_INTERRUPT_UART2            11
#define CYGNUM_HAL_INTERRUPT_WATCHDOG         12
#define CYGNUM_HAL_INTERRUPT_unused           13

#define CYGNUM_HAL_ISR_MIN                     0
#define CYGNUM_HAL_ISR_MAX                     13
#define CYGNUM_HAL_ISR_COUNT                   14

// The vector used by the Real time clock
#define CYGNUM_HAL_INTERRUPT_RTC               CYGNUM_HAL_INTERRUPT_TIMER0


//----------------------------------------------------------------------------
// Reset.
extern void hal_aeb_reset(void);
#define HAL_PLATFORM_RESET()             hal_aeb_reset()

#define HAL_PLATFORM_RESET_ENTRY 0x4018000

#endif // CYGONCE_HAL_PLATFORM_INTS_H
