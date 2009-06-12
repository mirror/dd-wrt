//==========================================================================
//
//      flash_erase_block.c
//
//      Flash programming
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
// Contributors: gthomas, msalter
// Date:         2000-07-14
// Purpose:      
// Description:  
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#include "flash.h"

#include <pkgconf/hal.h>
#include <cyg/hal/hal_arch.h>


int flash_erase_block(volatile unsigned char *block)
	__attribute__ ((section (".2ram.flash_erase_block")));
int flash_erase_block(volatile unsigned char *block)
{
    volatile unsigned char *ROM;
    unsigned short stat;
    int timeout = 50000;
    int len;

    // First 4K page of flash at physcial address zero is
    // virtually mapped to address 0xa0000000.
    ROM = FLASH_P2V((unsigned)block & 0xFF800000);

    // Clear any error conditions
    ROM[0] = FLASH_Clear_Status;

    // Erase block
    ROM[0] = FLASH_Block_Erase;
    *FLASH_P2V(block) = FLASH_Confirm;
    timeout = 5000000;
    while(((stat = ROM[0]) & FLASH_Status_Ready) != FLASH_Status_Ready) {
        if (--timeout == 0) break;
    }

    // Restore ROM to "normal" mode
    ROM[0] = FLASH_Reset;

    // If an error was reported, see if the block erased anyway
    if (stat & 0x7E) {
        len = FLASH_BLOCK_SIZE;
        while (len > 0) {
	    if (*FLASH_P2V(block) != 0xFF)
		break;
	    block++;
            len -= sizeof(*block);
        }
        if (len == 0) stat = 0;
    }

    return stat;
}
