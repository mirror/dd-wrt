//==========================================================================
//
//      xscale_misc.c
//
//      HAL misc support code for Intel XScale cores.
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
// Author(s):    msalter
// Contributors: msalter
// Date:         2002-10-18
// Purpose:      XScale core HAL support
// Description:  Implementations of HAL interfaces
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include CYGBLD_HAL_PLATFORM_H
#include CYGHWR_MEMORY_LAYOUT_H

void
hal_xscale_core_init(void)
{
    // Initialize debug control unit to sane state
    asm volatile ("mov  r0,#0\n"
		  "mcr  p15,0,r0,c14,c8,0\n"   // ibcr0
		  "mcr  p15,0,r0,c14,c9,0\n"   // ibcr1
		  "mcr  p15,0,r0,c14,c4,0\n"   // dbcon
		  "mov  r0,#0x80000000\n"
		  "mcr  p14,0,r0,c10,c0,0\n"   // dcsr
		  : /* no outputs */
		  : /* no inputs  */
		  : "r0" );
}

/*------------------------------------------------------------------------*/
// EOF xscale_misc.c

