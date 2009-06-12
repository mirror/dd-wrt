//==========================================================================
//
//      sh_hs7729pci_flash.c
//
//      Flash programming for Fujitsu/AMD device on SH HS7729PCI board
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
// Author(s):    jskov
// Contributors: jskov
// Date:         2001-05-28
// Purpose:      
// Description:  
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/infra/cyg_type.h>

//--------------------------------------------------------------------------
// Device properties

#define CYGNUM_FLASH_INTERLEAVE	(2)
#define CYGNUM_FLASH_SERIES	(1)
#define CYGNUM_FLASH_WIDTH      (16)
#define CYGNUM_FLASH_BASE 	(plf_flash_base)

static cyg_uint32 plf_flash_base;

//--------------------------------------------------------------------------
// Platform specific extras

static void plf_flash_init(void);
#define CYGHWR_FLASH_AM29XXXXX_PLF_INIT() plf_flash_init()

//--------------------------------------------------------------------------
// Now include the driver code.
#include "cyg/io/flash_am29xxxxx.inl"

//--------------------------------------------------------------------------
// Magic to determine location of flash
// This works by querying at the location where we expect the EPROM to
// be. Then the returned data is compared with the data at that location.
// If there's a match, assume that to be the location of the EPROM.
// Otherwise it's at the other location.
//
// This is done to avoid having separate configurations for the two board
// configurations. This is simple, has negligible overhead, and Just Works.
void
plf_flash_init(void)
{
    flash_data_t id[2];
    flash_data_t* p;

    plf_flash_base = 0xa0400000;
    flash_dev_query(id);

    p = (flash_data_t*)0xa0400000;
    if (id[0] == *p++ && id[1] == *p)
        plf_flash_base = 0xa0000000;
        
    // Make that the cached region.
    plf_flash_base &= ~0x20000000;
}

// ------------------------------------------------------------------------
// EOF sh_hs7729pci_flash.c
