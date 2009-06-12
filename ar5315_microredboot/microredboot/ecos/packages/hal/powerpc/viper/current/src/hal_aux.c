//=============================================================================
//
//      hal_aux.c
//
//      HAL auxiliary objects and code; per platform
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   hmt
// Contributors:hmt
// Date:        1999-06-08
// Purpose:     HAL aux objects: startup tables.
// Description: Tables for per-platform initialization
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <pkgconf/hal_powerpc_quicc.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_mem.h>            // HAL memory definitions
#include <cyg/hal/quicc/ppc8xx.h>
#include <cyg/hal/hal_if.h>             // hal_if_init
#include CYGHWR_MEMORY_LAYOUT_H

// The memory map is weakly defined, allowing the application to redefine
// it if necessary. The regions defined below are the minimum requirements.
CYGARC_MEMDESC_TABLE CYGBLD_ATTRIB_WEAK = {
    // Mapping for the Viper 855/860 development boards
    CYGARC_MEMDESC_CACHE(   0xfe000000, 0x00800000 ), // ROM region
    CYGARC_MEMDESC_NOCACHE( 0xff000000, 0x00100000 ), // MCP registers
    CYGARC_MEMDESC_NOCACHE( 0xfa000000, 0x00400000 ), // Control/Status+LEDs
    CYGARC_MEMDESC_CACHE(   CYGMEM_REGION_ram, CYGMEM_REGION_ram_SIZE ), // Main memory

    CYGARC_MEMDESC_TABLE_END
};

//--------------------------------------------------------------------------
// Platform init code.
void
hal_platform_init(void)
{
    volatile EPPC *eppc = (volatile EPPC *)eppc_base();

    eppc->pip_pbpar &= ~0x00004000;   // PB17 AS GPIO
    eppc->pip_pbdir |=  0x00004000;

    hal_if_init();
}

// EOF hal_aux.c
