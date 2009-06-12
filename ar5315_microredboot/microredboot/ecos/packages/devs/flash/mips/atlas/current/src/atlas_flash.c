//==========================================================================
//
//      atlas_flash.c
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
// Date:         2000-12-06
// Purpose:      
// Description:  
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/infra/diag.h>

#define  _FLASH_PRIVATE_
#include <cyg/io/flash.h>

#include "flash.h"

#include <string.h>

#define _si(p) ((p[1]<<8)|p[0])

int
flash_hwr_init(void)
{
    struct FLASH_query data, *qp;
    int num_regions, region_size;

    flash_dev_query(&data);

    qp = &data;
    if (/*(qp->manuf_code == FLASH_Intel_code) && */
        (strncmp(qp->id, "QRY", 3) == 0)) {
        num_regions = _si(qp->num_regions)+1;
        region_size = _si(qp->region_size)*256;

        flash_info.block_size = region_size*2;   // Pairs of chips in parallel
        flash_info.blocks = num_regions*2;	 // and pairs of chips in serial
        flash_info.start = (void *)0x9c000000;
        flash_info.end = (void *)0x9e000000;
        return FLASH_ERR_OK;
    } else {
        (*flash_info.pf)("Can't identify FLASH, sorry\n");
        diag_dump_buf((void*)&data, sizeof(data));
        return FLASH_ERR_HWR;
    }
}

// Map a hardware status to a package error
int
flash_hwr_map_error(int err)
{
    if (err & 0x007E007E) {
        (*flash_info.pf)("Err = %x\n", err);
        if (err & 0x00100010) {
            return FLASH_ERR_PROGRAM;
        } else 
        if (err & 0x00200020) {
            return FLASH_ERR_ERASE;
        } else 
        return FLASH_ERR_HWR;  // FIXME
    } else {
        return FLASH_ERR_OK;
    }
}

// See if a range of FLASH addresses overlaps currently running code
bool
flash_code_overlaps(void *start, void *end)
{
    extern char _stext[], _etext[];
    unsigned long p_stext, pstart, p_etext, pend;

    p_stext = CYGARC_PHYSICAL_ADDRESS((unsigned long)&_stext);
    p_etext = CYGARC_PHYSICAL_ADDRESS((unsigned long)&_etext);

    // if _stext/_etext in boot shadow region, convert to
    // system flash address
    if ((p_stext >= 0x1fc00000) && (p_etext <= 0x20000000)) {
	p_stext -= 0x02000000;
	p_etext -= 0x02000000;
    }

    pstart = CYGARC_PHYSICAL_ADDRESS((unsigned long)start);
    pend = CYGARC_PHYSICAL_ADDRESS((unsigned long)end);

    return (((p_stext >= pstart) && (p_stext < pend)) ||
	    ((p_etext >= pstart) && (p_etext < pend)));
}
