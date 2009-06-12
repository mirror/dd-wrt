//==========================================================================
//
//      moab_ide.c
//
//      HAL support code for IDE devices on TAMS MOAB
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Gary Thomas
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
// Author(s):    msalter
// Contributors: msalter, gthomas
// Date:         2002-01-04
// Purpose:      PCI support
// Description:  Implementations of HAL PCI interfaces
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include CYGBLD_HAL_PLATFORM_H
#include CYGHWR_MEMORY_LAYOUT_H

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros
#include <cyg/infra/diag.h>             // diag_printf()

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_if.h>             // calling interface API
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>           // Interrupt names
#include <cyg/hal/hal_cache.h>
#include <cyg/io/pci_hw.h>
#include <cyg/io/pci.h>

// Debug control
//   0 = no messages
//   1 = identification/discovery
//   2 = detailed I/O
#define IDE_DEBUG 0

int _plf_ide_num_controllers = 0;

static struct {
    cyg_uint32 cmd_bar;
    cyg_uint32 ctl_bar;
} ide_ctrl[HAL_IDE_NUM_CONTROLLERS];

cyg_uint8 
cyg_hal_plf_ide_read_uint8(int ctlr, cyg_uint32 reg)
{
    cyg_uint8 val;
    HAL_READ_UINT8(ide_ctrl[ctlr].cmd_bar + reg, val);
#if (IDE_DEBUG & 0x02)
    diag_printf("IDE8[%d.%d] => %x\n", ctlr, reg, val);
#endif
    return val;
}

void 
cyg_hal_plf_ide_write_uint8(int ctlr, cyg_uint32 reg, cyg_uint8 val)
{
    HAL_WRITE_UINT8(ide_ctrl[ctlr].cmd_bar + reg, val);
#if (IDE_DEBUG & 0x02)
    diag_printf("IDE8[%d.%d] <= %x\n", ctlr, reg, val);
#endif
}

cyg_uint16 
cyg_hal_plf_ide_read_uint16(int ctlr, cyg_uint32 reg)
{
    cyg_uint16 val;
    HAL_READ_UINT16(ide_ctrl[ctlr].cmd_bar + reg, val);
#if (IDE_DEBUG & 0x02)
    diag_printf("IDE16[%d.%d] => %x\n", ctlr, reg, val);
#endif
    return val;
}

void 
cyg_hal_plf_ide_write_uint16(int ctlr, cyg_uint32 reg, cyg_uint16 val)
{
    HAL_WRITE_UINT16(ide_ctrl[ctlr].cmd_bar + reg, val);
#if (IDE_DEBUG & 0x02)
    diag_printf("IDE16[%d.%d] <= %x\n", ctlr, reg, val);
#endif
}

void 
cyg_hal_plf_ide_write_control(int ctlr, cyg_uint8 val)
{
    HAL_WRITE_UINT8(ide_ctrl[ctlr].ctl_bar, val);
#if (IDE_DEBUG & 0x02)
    diag_printf("IDECTL[%d] <= %x\n", ctlr, val);
#endif
}

static cyg_bool
find_ide_match_func( cyg_uint16 v, cyg_uint16 d, cyg_uint32 c, void *p )
{
    return ((v == 0x105A) && (d == 0x4D68));
}

int
cyg_hal_plf_ide_init(void)
{
    int i;
    cyg_pci_device_id ide_dev = CYG_PCI_NULL_DEVID;
    cyg_pci_device ide_info;

#if (IDE_DEBUG & 0x01)
    diag_printf("Initializing IDE controller\n");
#endif
    if (cyg_pci_find_matching(&find_ide_match_func, NULL, &ide_dev)) {
        cyg_pci_get_device_info(ide_dev, &ide_info);
#if (IDE_DEBUG & 0x01)
        for (i = 0;  i < 6;  i++) {
            diag_printf("IDE - base[%d]: %08p, size: %08p, map: %08p\n",
                        i, ide_info.base_address[i], ide_info.base_size[i], ide_info.base_map[i]);
        }
#endif
        for (i = 0;  i < HAL_IDE_NUM_CONTROLLERS;  i++) {
            ide_ctrl[i].cmd_bar = ide_info.base_map[(2*i)+0] & 0xFFFFFFFE;
            ide_ctrl[i].ctl_bar = ide_info.base_map[(2*i)+1] & 0xFFFFFFFE;
        }
        return HAL_IDE_NUM_CONTROLLERS;
    } else {
#if (IDE_DEBUG & 0x01)
        diag_printf("Can't find IDE controller!\n");
#endif
        return 0;
    }
}
