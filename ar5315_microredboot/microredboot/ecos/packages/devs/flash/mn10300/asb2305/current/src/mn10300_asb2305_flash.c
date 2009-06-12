//==========================================================================
//
//      mn10300_asb2305_flash.c
//
//      Flash programming for Matsushita ASB2305
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
// Author(s):    dhowells
// Contributors: dhowells
// Date:         2001-06-01
// Purpose:      
// Description:  
//              
//####DESCRIPTIONEND####
//
//==========================================================================
#include <pkgconf/devs_flash_mn10300_asb2305.h>

//--------------------------------------------------------------------------
// Device properties

#if defined(CYGHWR_DEVS_FLASH_MN10300_ASB2305_BANK_SysFlash)
// We use a four chip parallel AM29DL324BE module plugged into the System Flash socket
// on the ASB2305 board.
#define CYGNUM_FLASH_INTERLEAVE	(4)
#define CYGNUM_FLASH_SERIES	(1)
#define CYGNUM_FLASH_WIDTH	(8)
#define CYGNUM_FLASH_16AS8      (1)
#define CYGNUM_FLASH_BASE 	(0x84000000u)	/* cached flash chip address */

#elif defined(CYGHWR_DEVS_FLASH_MN10300_ASB2305_BANK_BootPROM)
// We have a single AM29LV800TA soldered onto the board as the boot PROM
#define CYGNUM_FLASH_INTERLEAVE	(1)
#define CYGNUM_FLASH_SERIES	(1)
#define CYGNUM_FLASH_WIDTH	(16)
#define CYGNUM_FLASH_BASE 	(0x80000000u)	/* cached flash chip address */

#else
#error unknown flash bank selected
#endif

//--------------------------------------------------------------------------
// Platform specific extras

// Offset of uncached shadow region
#define FLASH_P2V( _a_ ) ((volatile flash_data_t *)((CYG_ADDRWORD)(_a_)+0x20000000))

//--------------------------------------------------------------------------
// Now include the driver code.
#include "cyg/io/flash_am29xxxxx.inl"

// ------------------------------------------------------------------------
// EOF mn10300_asb2305_flash.c
