//==========================================================================
//
//      flash_unlock_block.c
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
// Contributors: gthomas
// Date:         2000-09-10
// Purpose:      
// Description:  
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#include "flash.h"

#include <cyg/hal/hal_arch.h>


//
// The difficulty with this operation is that the hardware does not support
// unlocking single blocks.  However, the logical layer would like this to
// be the case, so this routine emulates it.  The hardware can clear all of
// the locks in the device at once.  This routine will use that approach and
// then reset the regions which are known to be locked.
//

#define MAX_FLASH_BLOCKS 128

int
flash_unlock_block(volatile unsigned long *block, int block_size, int blocks)
	__attribute__ ((section (".2ram.flash_unlock_block")));
int
flash_unlock_block(volatile unsigned long *block, int block_size, int blocks)
{
    volatile unsigned long *ROM, *bp;
    unsigned long stat;
    int timeout = 5000000;
    unsigned char is_locked[MAX_FLASH_BLOCKS];
    int i;

    FLASH_WRITE_ENABLE();

    block = (volatile unsigned long *)CYGARC_UNCACHED_ADDRESS((unsigned long)block);
    ROM = (volatile unsigned long *)((unsigned long)block & FLASH_BASE_MASK);

    // Clear any error conditions
    ROM[0] = FLASH_Clear_Status;

    // Get current block lock state.  This needs to access each block on
    // the device so currently locked blocks can be re-locked.
    bp = ROM;
    for (i = 0;  i < (blocks/2);  i++) {
        if (bp == block) {
            is_locked[i] = 0;
        } else {
	    *bp = FLASH_Read_Query;
            is_locked[i] = bp[2];
        }
        bp += block_size / sizeof(*bp);
    }

    // Clears all lock bits
    block[0] = FLASH_Clear_Locks;
    block[0] = FLASH_Clear_Locks_Confirm;  // Confirmation
    timeout = 5000000;
    while(((stat = ROM[0]) & FLASH_Status_Ready) != FLASH_Status_Ready) {
        if (--timeout == 0) break;
    }

    // Restore the lock state
    bp = ROM;
    for (i = 0;  i < (blocks/2);  i++) {
        if (is_locked[i]) {
            *bp = FLASH_Set_Lock;
            *bp = FLASH_Set_Lock_Confirm;  // Confirmation
            timeout = 5000000;
            while(((stat = ROM[0]) & FLASH_Status_Ready) != FLASH_Status_Ready) {
                if (--timeout == 0) break;
            }
        }
        bp += block_size / sizeof(*bp);
    }

    // Restore ROM to "normal" mode
    ROM[0] = FLASH_Reset;

    FLASH_WRITE_DISABLE();

    return stat;
}
