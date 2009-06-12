//==========================================================================
//
//      moab_nand_flash.c
//
//      Flash programming for Toshiba NAND FLASH device on MOAB board
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Nick Garnett <nickg@calivar.com>
// Copyright (C) 2003, 2004 Gary Thomas
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
// Author(s):    Gary Thomas <gary@mlbassoc.com>
// Contributors: 
// Date:         2003-09-02
// Purpose:      
// Description:  
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/infra/cyg_type.h>
#define CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
#include <cyg/hal/ppc_regs.h>           // Platform registers
#include <cyg/hal/hal_io.h>             // I/O macros
#include <cyg/hal/hal_if.h>             // Virtual vector interfaces

// There's a single TC58256 or TC58DVG02 on the MOAB board. 

#define CYGHWR_DEVS_FLASH_TOSHIBA_TC58256
#define CYGHWR_DEVS_FLASH_TOSHIBA_TC58DVG02
#define CYGNUM_FLASH_BLANK	(1)
#define CYGNUM_FLASH_INTERLEAVE	(1)
#define CYGNUM_FLASH_WIDTH      (8)
#define CYGNUM_FLASH_SERIES	(1)
#define CYGNUM_FLASH_BASE 	(_MOAB_NAND)

#define CYGHWR_FLASH_TC58XXX_CE   moab_CE 
#define CYGHWR_FLASH_TC58XXX_CLE  moab_CLE 
#define CYGHWR_FLASH_TC58XXX_ALE  moab_ALE
#define CYGHWR_FLASH_TC58XXX_RDY  moab_RDY

//
// Select the NAND device - this will be held until the
// entire command/data sequence has completed (which is
// necessary for read sequential)
//
static void __inline__
moab_CE(int state)
{
    cyg_uint32 gpio_or;
    HAL_READ_UINT32(GPIO_OR, gpio_or);
    if (state) {
        // Assert CE
        gpio_or &= ~_MOAB_CE;
    } else {
        // De-assert CE
        gpio_or |= _MOAB_CE;
    }
    HAL_WRITE_UINT32(GPIO_OR, gpio_or);
}

//
// Prepare to send a command byte to the NAND interface
//
static void __inline__
moab_CLE(int state)
{
    cyg_uint32 gpio_or;
    HAL_READ_UINT32(GPIO_OR, gpio_or);
    if (state) {
        // Assert CLE
        gpio_or |= _MOAB_CLE;
    } else {
        // De-assert CLE
        gpio_or &= ~_MOAB_CLE;
    }
    HAL_WRITE_UINT32(GPIO_OR, gpio_or);
}

//
// Prepare to send address byte(s) to the NAND interface
//
static void __inline__
moab_ALE(int state)
{
    cyg_uint32 gpio_or;

    HAL_READ_UINT32(GPIO_OR, gpio_or);
    if (state) {
        // Assert ALE
        gpio_or |= _MOAB_ALE;
    } else {
        // De-assert ALE
        gpio_or &= ~_MOAB_ALE;
    }
    HAL_WRITE_UINT32(GPIO_OR, gpio_or);
}

//
// Wait until the NAND device is ready (after a long operation)
//
static bool __inline__
moab_RDY(void)
{    
    cyg_uint32 gpio_ir;

    HAL_READ_UINT32(GPIO_IR, gpio_ir);
    return ((gpio_ir & _MOAB_RDY) != 0);
}

#include <cyg/io/flash_tc58xxx.inl>


// ------------------------------------------------------------------------
// EOF moab_nand_flash.c
