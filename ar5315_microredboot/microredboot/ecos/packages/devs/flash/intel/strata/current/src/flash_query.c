//==========================================================================
//
//      flash_query.c
//
//      Flash programming - query device
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2004 Red Hat, Inc.
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
// Author(s):    gthomas, hmt
// Contributors: gthomas
// Date:         2001-02-14
// Purpose:      
// Description:  
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#include "strata.h"

#include <pkgconf/hal.h>
#include <cyg/hal/hal_arch.h>
#include CYGHWR_MEMORY_LAYOUT_H

// Platforms may define this for special handling when accessing the query data.
#ifndef CYGHWR_FLASH_READ_QUERY
#define CYGHWR_FLASH_READ_QUERY(a) (*(a))
#endif

#define CNT 20*1000*10  // Approx 20ms

int
flash_query(unsigned char *data)  __attribute__ ((section (".2ram.flash_query")));
int
flash_query(unsigned char *data)
{
    volatile flash_t *ROM;
    int i, cnt;

    // Get base address and map addresses to virtual addresses
    ROM = FLASH_P2V( CYGNUM_FLASH_BASE );
#ifdef CYGOPT_FLASH_IS_BOOTBLOCK
    // BootBlock flash does not support full Read_Query - we have do a
    // table oriented thing above, after getting just two bytes of results:
    ROM[0] = FLASH_Read_ID;
    i = 2;
#else
    // StrataFlash supports the full Read_Query op:
    ROM[0] = FLASH_Read_Query;
    i = sizeof(struct FLASH_query);
#endif // Not CYGOPT_FLASH_IS_BOOTBLOCK

    for (cnt = CNT;  cnt > 0;  cnt--) ;
    for ( /* i */;  i > 0;  i--, ++ROM) {
        // It is very deliberate that data is chars NOT flash_t:
        // The info comes out in bytes regardless of device.
        *data++ = (unsigned char) CYGHWR_FLASH_READ_QUERY(ROM);
#ifndef CYGOPT_FLASH_IS_BOOTBLOCK
# if  8 == CYGNUM_FLASH_WIDTH
	// strata flash with 'byte-enable' contains the configuration data
	// at even addresses
	++ROM;
# endif
#endif
    }
    ROM[0] = FLASH_Reset;

    return 0;
}
