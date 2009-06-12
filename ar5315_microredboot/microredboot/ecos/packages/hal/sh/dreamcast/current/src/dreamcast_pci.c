//=============================================================================
//
//      dreamcast_pci.c
//
//      Dreamcast PCI code 
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
// Author(s):   t@keshi.org
// Contributors:t@keshi.org
// Date:        2001-07-30
// Purpose:     Dreamcast PCI code
//              
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include CYGBLD_HAL_PLATFORM_H
#include CYGHWR_MEMORY_LAYOUT_H

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_if.h>             // calling interface API
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>           // Interrupt names
#include <cyg/hal/hal_cache.h>
#include <cyg/io/pci_hw.h>
#include <cyg/io/pci.h>

#define GAPSPCI_REGS            0xa1001400
#define GAPSPCI_DMA_BASE        0x01840000
#define GAPSPCI_DMA_SIZE        32768
#define GAPSPCI_BBA_CONFIG      0xa1001600
#define GAPSPCI_INTC            0xa05f6924

static cyg_bool gapspci_initialized;

void cyg_hal_plf_pci_init(void)
{
        int i;
        cyg_uint32 val;
#if 0
        char idbuf[16];

        for(i=0; i<16; i++)
                HAL_READ_UINT8(GAPSPCI_REGS+i, idbuf[i]);

        if(strncmp(idbuf, "GAPSPCI_BRIDGE_2", 16))
                return;
#endif

        gapspci_initialized = false;

        HAL_WRITE_UINT32(GAPSPCI_REGS+0x18, 0x5a14a501);

        for(i=0; i<1000000; i++);

        HAL_READ_UINT32(GAPSPCI_REGS+0x18, val);
        if (val != 1) return;

        HAL_WRITE_UINT32(GAPSPCI_REGS+0x20, 0x01000000);
        HAL_WRITE_UINT32(GAPSPCI_REGS+0x24, 0x01000000);

        HAL_WRITE_UINT32(GAPSPCI_REGS+0x28, GAPSPCI_DMA_BASE);
        HAL_WRITE_UINT32(GAPSPCI_REGS+0x2c, GAPSPCI_DMA_BASE+GAPSPCI_DMA_SIZE);

        HAL_WRITE_UINT32(GAPSPCI_REGS+0x14, 1);
        HAL_WRITE_UINT32(GAPSPCI_REGS+0x34, 1);

#if 1
        /* Setting up Broadband Adapter */
        HAL_WRITE_UINT16(GAPSPCI_BBA_CONFIG+0x06, 0xf900);
        HAL_WRITE_UINT32(GAPSPCI_BBA_CONFIG+0x30, 0x00000000);
        HAL_WRITE_UINT8 (GAPSPCI_BBA_CONFIG+0x3c, 0x00);
        HAL_WRITE_UINT8 (GAPSPCI_BBA_CONFIG+0x0d, 0xf0);
        HAL_WRITE_UINT16(GAPSPCI_BBA_CONFIG+0x04, 0x0006);
        HAL_WRITE_UINT32(GAPSPCI_BBA_CONFIG+0x10, 0x00002001);
        HAL_WRITE_UINT32(GAPSPCI_BBA_CONFIG+0x14, 0x01000000);
#endif

        /* Enable interrupt */
        HAL_READ_UINT32(GAPSPCI_INTC, val);
        val |= (1<<3);
        HAL_WRITE_UINT32(GAPSPCI_INTC, val);

        gapspci_initialized = true;
}


#define BBA_SELECTED(bus, devfn) (bus==0 && devfn==0)

cyg_uint32 cyg_hal_plf_pci_cfg_read_dword (cyg_uint32 bus,
                                           cyg_uint32 devfn,
                                           cyg_uint32 offset)
{
        cyg_uint32 val;
        if (!gapspci_initialized || !BBA_SELECTED(bus, devfn))
                return 0xffffffff;
        HAL_READ_UINT32(GAPSPCI_BBA_CONFIG+offset, val);
        return val;
}


cyg_uint16 cyg_hal_plf_pci_cfg_read_word  (cyg_uint32 bus,
                                           cyg_uint32 devfn,
                                           cyg_uint32 offset)
{
        cyg_uint16 val;
        if (!gapspci_initialized || !BBA_SELECTED(bus, devfn))
                return 0xffff;
        HAL_READ_UINT16(GAPSPCI_BBA_CONFIG+offset, val);
        return val;
}


cyg_uint8 cyg_hal_plf_pci_cfg_read_byte   (cyg_uint32 bus,
                                           cyg_uint32 devfn,
                                           cyg_uint32 offset)
{
        cyg_uint8 val;
        if (!gapspci_initialized || !BBA_SELECTED(bus, devfn))
                return 0xff;
        HAL_READ_UINT8(GAPSPCI_BBA_CONFIG+offset, val);
        return val;
}


void cyg_hal_plf_pci_cfg_write_dword (cyg_uint32 bus,
                                      cyg_uint32 devfn,
                                      cyg_uint32 offset,
                                      cyg_uint32 val)
{
        if (gapspci_initialized && BBA_SELECTED(bus, devfn))
                HAL_WRITE_UINT32(GAPSPCI_BBA_CONFIG+offset, val);
}


void cyg_hal_plf_pci_cfg_write_word  (cyg_uint32 bus,
                                      cyg_uint32 devfn,
                                      cyg_uint32 offset,
                                      cyg_uint16 val)
{
        if (gapspci_initialized && BBA_SELECTED(bus, devfn))
                HAL_WRITE_UINT16(GAPSPCI_BBA_CONFIG+offset, val);
}


void cyg_hal_plf_pci_cfg_write_byte   (cyg_uint32 bus,
                                       cyg_uint32 devfn,
                                       cyg_uint32 offset,
                                       cyg_uint8 val)
{
        if (gapspci_initialized && BBA_SELECTED(bus, devfn))
                HAL_WRITE_UINT8(GAPSPCI_BBA_CONFIG+offset, val);
}

//-----------------------------------------------------------------------------
// End of dreamcast_pci.c
