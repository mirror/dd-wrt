//==========================================================================
//
//      wallclock_mpc5xx.cxx
//
//      mpc5xx RTC module driver.
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
// Author(s):     Bob Koninckx
// Contributors:  Bob Koninckx
// Date:          2002-01-18
// Purpose:       Wallclock driver for mpc5xx
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/wallclock.h>            // Wallclock device config

#include <cyg/hal/hal_io.h>               // IO macros
#include <cyg/infra/cyg_type.h>           // Common type definitions and support

#include <cyg/io/wallclock.hxx>           // The WallClock API
#include <cyg/io/wallclock/wallclock.inl> // Helpers

#include <cyg/hal/hal_arch.h>             // RTC register definitions

#include <cyg/infra/diag.h>               // For debugging

//-----------------------------------------------------------------------------
// Functions required for the hardware-driver API.

// Returns the number of seconds elapsed since 1970-01-01 00:00:00.
cyg_uint32 
Cyg_WallClock::get_hw_seconds(void)
{
  cyg_uint32 now;
  HAL_READ_UINT32(CYGARC_REG_IMM_RTC, now);

  return now;
}

#ifndef CYGSEM_WALLCLOCK_SET_GET_MODE

void
Cyg_WallClock::init_hw_seconds(void)
{
  cyg_uint32 key = 0x55ccaa33;
  cyg_uint16 rtcsc;

  // Write zero to the time register
  // and start up the RTC
  HAL_WRITE_UINT32(CYGARC_REG_IMM_RTCK, key);
  HAL_WRITE_UINT32(CYGARC_REG_IMM_RTCSCK, key);
  
  key = 0;
  HAL_WRITE_UINT32(CYGARC_REG_IMM_RTC, key);
  HAL_READ_UINT16(CYGARC_REG_IMM_RTCSC, rtcsc);
  rtcsc |= 0x0001;
  HAL_WRITE_UINT16(CYGARC_REG_IMM_RTCSC, rtcsc);
}

#endif // CYGSEM_WALLCLOCK_SET_GET_MODE

//-----------------------------------------------------------------------------
// End of wallclock_mpc5xx.cxx
