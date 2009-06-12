//==========================================================================
//
//      jtst_misc.c
//
//      HAL misc board support code for Atmel JTST eval board
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Nick Garnett <nickg@calivar.com>
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
// Contributors: gthomas, jskov, tkoeller, nickg,amichelotti
// Date:         2004-06-6
// Purpose:      HAL board support
// Description:  Implementations of HAL board interfaces
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/hal.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_misc.h>
#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/hal/var_io.h>             // platform registers

// in JTST only has a 7 segments display available
void 
hal_at91_led(int val){
  unsigned led=BIT27|BIT26|BIT25|BIT24|BIT23|BIT22|BIT21;
  // turn all leds off
  HAL_WRITE_UINT32(AT91_PIO+AT91_PIO_SODR,led);

  switch(val&0xf){
  case 0:
    led = BIT27|BIT26|BIT25|BIT24|BIT23|BIT21;
    break;
  case 1:
    led = BIT26|BIT25;
    break;
  case 2:
    led = BIT27|BIT26|BIT24|BIT23|BIT22;
    break;
  case 3:
    led = BIT27|BIT26|BIT25|BIT24|BIT22;
    break;
  case 4:
    led = BIT26|BIT21|BIT22|BIT25;
    break;
  case 5:
    led = BIT27|BIT25|BIT24|BIT21|BIT22;
    break;
  case 6:
    led = BIT27|BIT25|BIT24|BIT21|BIT22|BIT23;
    break;
  case 7:
    led = BIT27|BIT26|BIT25;
    break;
  case 8:
    led = BIT27|BIT26|BIT25|BIT24|BIT23|BIT22|BIT21;
    break;
  case 9:
    led = BIT27|BIT26|BIT25|BIT22|BIT21;
    break;
  default: // Show an E for out of range values
    led = BIT27|BIT24|BIT23|BIT22|BIT21;
  }
  // leds on
  HAL_WRITE_UINT32(AT91_PIO+AT91_PIO_CODR,led);
}

void
hal_at91_set_leds(int val){
  hal_at91_led(val);
}

//--------------------------------------------------------------------------
// EOF jtst_misc.c
