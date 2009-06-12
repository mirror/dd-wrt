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
// Contributors: gthomas, dmoseley
// Date:         2000-07-26
// Purpose:      
// Description:  
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#include "flash.h"

#include <pkgconf/hal.h>
#include <cyg/hal/hal_arch.h>


#define CNT 20*1000*10  // Approx 200ms

int
flash_query(unsigned char *data) 
	__attribute__ ((section (".2ram.flash_query")));
int
flash_query(unsigned char *data)
{
    volatile unsigned long *lROM;
    volatile unsigned char *cROM;
    int i, cnt;

    lROM = (unsigned long *)0x08000000;
    cROM = (unsigned char *)0x08000000;

    lROM[0] = FLASH_Read_ID;
    for (cnt = CNT;  cnt > 0;  cnt--) ;
    for (i = 0;  i < 8;  i++) {
        *data++ = cROM[i];
    }
    lROM[0] = FLASH_Reset;

    return 0;
}
