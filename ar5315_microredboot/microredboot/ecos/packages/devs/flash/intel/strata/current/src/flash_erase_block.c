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

int flash_erase_block(volatile flash_t *block, unsigned int block_size)
	__attribute__ ((section (".2ram.flash_erase_block")));
int flash_erase_block(volatile flash_t *block, unsigned int block_size)
{
    volatile flash_t *ROM;
    flash_t stat = 0;
    int timeout = 50000;
    int len, block_len, erase_block_size;
    volatile flash_t *eb;

    // Get base address and map addresses to virtual addresses
    ROM = FLASH_P2V(CYGNUM_FLASH_BASE_MASK & (unsigned int)block);
    eb = block = FLASH_P2V(block);
    block_len = block_size;

#ifdef CYGOPT_FLASH_IS_BOOTBLOCK
#define BLOCKSIZE (0x10000*CYGNUM_FLASH_DEVICES)
#define ERASE_BLOCKSIZE (0x2000*CYGNUM_FLASH_DEVICES)
    if ((eb - ROM) < BLOCKSIZE/(sizeof eb[0])) {
        erase_block_size = ERASE_BLOCKSIZE;
    } else {
        erase_block_size = block_size;
    }
#else
    erase_block_size = block_size;
#endif

    // Clear any error conditions
    ROM[0] = FLASH_Clear_Status;

    // Erase block
    while (block_len > 0) {
        eb[0] = FLASH_Block_Erase;
        eb[0] = FLASH_Confirm;
	
        timeout = 5000000;
        while(((stat = eb[0]) & FLASH_Status_Ready) != FLASH_Status_Ready) {
            if (--timeout == 0) break;
        }

        block_len -= erase_block_size;
        eb = FLASH_P2V((unsigned int)eb + erase_block_size);
    }

    // Restore ROM to "normal" mode
    ROM[0] = FLASH_Reset;

    // If an error was reported, see if the block erased anyway
    if (stat & FLASH_ErrorMask ) {
        len = block_size;
        while (len > 0) {
            if (*block++ != FLASH_BlankValue ) break;
            len -= sizeof(*block);
        }
        if (len == 0) stat = 0;
    }

    return stat;
}
