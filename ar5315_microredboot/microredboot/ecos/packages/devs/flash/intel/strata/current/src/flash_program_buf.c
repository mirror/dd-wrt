//==========================================================================
//
//      flash_program_buf.c
//
//      Flash programming
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

// Platforms may define this for special handling when accessing the write buffer.
#ifndef CYGHWR_FLASH_WRITE_BUF
#define CYGHWR_FLASH_WRITE_BUF(a,b) (*(a) = *(b))
#endif

int
flash_program_buf(volatile flash_t *addr, flash_t *data, int len,
                  unsigned long block_mask, int buffer_size)
 __attribute__ ((section (".2ram.flash_program_buf")));
int
flash_program_buf(volatile flash_t *addr, flash_t *data, int len,
                  unsigned long block_mask, int buffer_size)
{
    volatile flash_t *ROM;
    volatile flash_t *BA;
    flash_t stat = 0;
    int timeout = 50000;
#ifdef FLASH_Write_Buffer
    int i, wc;
#endif

    // Get base address and map addresses to virtual addresses
    ROM = FLASH_P2V( CYGNUM_FLASH_BASE_MASK & (unsigned int)addr );
    BA = addr = FLASH_P2V(addr);

    // Clear any error conditions
    ROM[0] = FLASH_Clear_Status;

#ifdef FLASH_Write_Buffer
    // Write any big chunks first
    while (len >= buffer_size) {
        wc = buffer_size;
        if (wc > len) wc = len;
        len -= wc;
	// convert 'wc' in bytes to 'wc' in 'flash_t' 
        wc = wc / sizeof(flash_t);  // Word count
        *BA = FLASH_Write_Buffer;
        timeout = 5000000;
        while(((stat = ROM[0]) & FLASH_Status_Ready) != FLASH_Status_Ready) {
            if (--timeout == 0) {
                goto bad;
            }
            *BA = FLASH_Write_Buffer;
        }
        *BA = FLASHWORD(wc-1);  // Count is 0..N-1
        for (i = 0;  i < wc;  i++) {
#ifdef CYGHWR_FLASH_WRITE_ELEM
            CYGHWR_FLASH_WRITE_ELEM(addr+i, data+i);
#else
            CYGHWR_FLASH_WRITE_BUF(addr+i, data+i);
#endif
        }
        *BA = FLASH_Confirm;
    
        ROM[0] = FLASH_Read_Status;
        timeout = 5000000;
        while(((stat = ROM[0]) & FLASH_Status_Ready) != FLASH_Status_Ready) {
            if (--timeout == 0) {
                goto bad;
            }
        }
        // Jump out if there was an error
        if (stat & FLASH_ErrorMask) {
            goto bad;
        }
        // And verify the data - also increments the pointers.
        *BA = FLASH_Reset;            
        for (i = 0;  i < wc;  i++) {
            if ( *addr++ != *data++ ) {
                stat = FLASH_ErrorNotVerified;
                goto bad;
            }
        }
    }
#endif

    while (len > 0) {
        BA[0] = FLASH_Program;
#ifdef CYGHWR_FLASH_WRITE_ELEM
        CYGHWR_FLASH_WRITE_ELEM(addr, data);
#else
        *addr = *data;
#endif
        timeout = 5000000;
        while(((stat = BA[0]) & FLASH_Status_Ready) != FLASH_Status_Ready) {
            if (--timeout == 0) {
                goto bad;
            }
        }
        if (stat & FLASH_ErrorMask) {
            break;
        }
        BA[0] = FLASH_Reset;            
        if (*addr++ != *data++) {
            stat = FLASH_ErrorNotVerified;
            break;
        }
        len -= sizeof( flash_t );
    }

    // Restore ROM to "normal" mode
 bad:
    BA[0] = FLASH_Reset;            

    return stat;
}
