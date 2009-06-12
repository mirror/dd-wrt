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
// Author(s):    jskov
// Contributors: jskov
// Date:         2001-11-02
// Purpose:      Define Interrupt support
// Description:  The interrupt details for the Agilent AAED2000 are defined here.
// Usage:
//               #include <cyg/hal/hal_platform_ints.h>
//               ...
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

// These are interrupts on the AAEC-2000 core

#define CYGNUM_HAL_INTERRUPT_GPIO0FIQ      0
#define CYGNUM_HAL_INTERRUPT_TS            CYGNUM_HAL_INTERRUPT_GPIO0FIQ
#define CYGNUM_HAL_INTERRUPT_BLINT         1
#define CYGNUM_HAL_INTERRUPT_WEINT         2
#define CYGNUM_HAL_INTERRUPT_MCINT         3
#define CYGNUM_HAL_INTERRUPT_CSINT         4
#define CYGNUM_HAL_INTERRUPT_GPIO1INTR     5
#define CYGNUM_HAL_INTERRUPT_ETH           CYGNUM_HAL_INTERRUPT_GPIO1INTR
#define CYGNUM_HAL_INTERRUPT_GPIO2INTR     6
#define CYGNUM_HAL_INTERRUPT_PCMCIA_CD2    CYGNUM_HAL_INTERRUPT_GPIO2INTR
#define CYGNUM_HAL_INTERRUPT_GPIO3INTR     7
#define CYGNUM_HAL_INTERRUPT_PCMCIA_CD1    CYGNUM_HAL_INTERRUPT_GPIO3INTR
#define CYGNUM_HAL_INTERRUPT_TC1OI         8
#define CYGNUM_HAL_INTERRUPT_TC2OI         9
#define CYGNUM_HAL_INTERRUPT_RTCMI        10
#define CYGNUM_HAL_INTERRUPT_TINTR        11
#define CYGNUM_HAL_INTERRUPT_UART1INTR    12
#define CYGNUM_HAL_INTERRUPT_UART2INTR    13
#define CYGNUM_HAL_INTERRUPT_LCDINTR      14
#define CYGNUM_HAL_INTERRUPT_SSEOTI       15
#define CYGNUM_HAL_INTERRUPT_UART3INTR    16
#define CYGNUM_HAL_INTERRUPT_SCIINTR      17
#define CYGNUM_HAL_INTERRUPT_AACINTR      18
#define CYGNUM_HAL_INTERRUPT_MMCINTR      19
#define CYGNUM_HAL_INTERRUPT_USBINTR      20
#define CYGNUM_HAL_INTERRUPT_DMAINTR      21
#define CYGNUM_HAL_INTERRUPT_TC3OI        22
#define CYGNUM_HAL_INTERRUPT_GPIO4INTR    23
#define CYGNUM_HAL_INTERRUPT_SCI_VCCEN    CYGNUM_HAL_INTERRUPT_GPIO4INTR
#define CYGNUM_HAL_INTERRUPT_GPIO5INTR    24
#define CYGNUM_HAL_INTERRUPT_SCI_DETECT   CYGNUM_HAL_INTERRUPT_GPIO5INTR
#define CYGNUM_HAL_INTERRUPT_GPIO6INTR    25
#define CYGNUM_HAL_INTERRUPT_PCMCIA_RDY1  CYGNUM_HAL_INTERRUPT_GPIO6INTR
#define CYGNUM_HAL_INTERRUPT_GPIO7INTR    26
#define CYGNUM_HAL_INTERRUPT_PCMCIA_RDY2  CYGNUM_HAL_INTERRUPT_GPIO7INTR
#define CYGNUM_HAL_INTERRUPT_BMIINTR      27

#define CYGNUM_HAL_INTERRUPT_NONE    -1

#define CYGNUM_HAL_ISR_MIN            0
#define CYGNUM_HAL_ISR_MAX            (CYGNUM_HAL_INTERRUPT_BMIINTR)

#define CYGNUM_HAL_ISR_COUNT          (CYGNUM_HAL_ISR_MAX-CYGNUM_HAL_ISR_MIN+1)

// The vector used by the Real time clock
#define CYGNUM_HAL_INTERRUPT_RTC      CYGNUM_HAL_INTERRUPT_TC1OI

//----------------------------------------------------------------------------
// Reset.

externC void cyg_hal_arm9_soft_reset(CYG_ADDRESS);
#define HAL_PLATFORM_RESET() cyg_hal_arm9_soft_reset(0)

#define HAL_PLATFORM_RESET_ENTRY 0x00000000

#endif // CYGONCE_HAL_PLATFORM_INTS_H
