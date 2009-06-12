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
// Contributors: gthomas
// Date:         2000-10-10
// Purpose:      Define Interrupt support
// Description:  The interrupt details for the NPWR Linux Engine are defined here.
// Usage:
//               included from <cyg/hal/hal_var_ints.h>
//               ...
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#define CYGNUM_HAL_INTERRUPT_TIMER        CYGNUM_HAL_INTERRUPT_XINT3_BIT0 // external timer
#define CYGNUM_HAL_INTERRUPT_ETH0         CYGNUM_HAL_INTERRUPT_XINT3_BIT1 // primary enet
#define CYGNUM_HAL_INTERRUPT_SCSI         CYGNUM_HAL_INTERRUPT_XINT3_BIT2 // 53c1000 SCSI
#define CYGNUM_HAL_INTERRUPT_SERIAL_B     CYGNUM_HAL_INTERRUPT_XINT3_BIT3 // 16x50 uart B
#define CYGNUM_HAL_INTERRUPT_ETH1         CYGNUM_HAL_INTERRUPT_XINT3_BIT4 // secondary enet

// The vector used by the Real time clock
#define CYGNUM_HAL_INTERRUPT_RTC        CYGNUM_HAL_INTERRUPT_TIMER
//#define CYGNUM_HAL_INTERRUPT_RTC        CYGNUM_HAL_INTERRUPT_PMU_CCNT_OVFL

#endif // CYGONCE_HAL_PLATFORM_INTS_H

// EOF hal_platform_ints.h
