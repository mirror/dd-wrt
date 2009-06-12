//=============================================================================
//
//      pcitest.c
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
// Author(s):   Scott Coulter, Jeff Frazier, Eric Breeden
// Contributors: Mark Salter
// Date:        2001-01-25
// Purpose:     
// Description: 
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <redboot.h>
#include <cyg/io/pci.h>
#include "test_menu.h"

extern int memTest (CYG_ADDRWORD startAddr, CYG_ADDRWORD endAddr);

//
// PCI Bus Test
//
// This test assumes that an IQ80310 eval board
// is installed in the secondary PCI slot. This
// second board must be configured with 32 Meg
// of SDRAM minimum.
//
//
void
pci_test (MENU_ARG arg)
{
    cyg_pci_device dev_info;
    cyg_pci_device_id devid;
    cyg_uint32 mem_size;
    cyg_uint16 cmd;
    cyg_uint32 *start, *end;
    int bus;

    // First, look for iq80310 at private and public addresses

    bus = (*ATU_PCIXSR >> 8) & 0xff;
    if (bus == 0xff)
	bus = 0;

    devid = CYG_PCI_DEV_MAKE_ID(bus, CYG_PCI_DEV_MAKE_DEVFN(__SLOT_PUB, 1));
    cyg_pci_get_device_info(devid, &dev_info);

    if (dev_info.vendor != 0x8086 || dev_info.device != 0x530d) {
	devid = CYG_PCI_DEV_MAKE_ID(bus, CYG_PCI_DEV_MAKE_DEVFN(__SLOT_PRIV, 1));
	cyg_pci_get_device_info(devid, &dev_info);

	if (dev_info.vendor != 0x8086 || dev_info.device != 0x530d) {
	    diag_printf("No iq80310 in PCI slot.\n");
	    return;
	}
    }

    cyg_pci_set_memory_base(HAL_PCI_ALLOC_BASE_MEMORY + 0x2000000);
    cyg_pci_set_io_base(HAL_PCI_ALLOC_BASE_IO);

    cyg_pci_configure_device(&dev_info);

    diag_printf ("iq80310 DRAM starts at PCI address %p, CPU address %p\n",
		 dev_info.base_address[0] & CYG_PRI_CFG_BAR_MEM_MASK,
		 dev_info.base_map[0]);

    // enable memory space and bus master
    cyg_pci_read_config_uint16(dev_info.devid, CYG_PCI_CFG_COMMAND, &cmd);
    cmd |= (CYG_PCI_CFG_COMMAND_MEMORY | CYG_PCI_CFG_COMMAND_MASTER);
    cyg_pci_write_config_uint16(dev_info.devid, CYG_PCI_CFG_COMMAND, cmd);

    start = (cyg_uint32 *)dev_info.base_map[0];
    // skip over 1st Mbyte of target DRAM
    start += 0x100000/sizeof(*start);
    // 32MB test
    mem_size = 0x2000000 - 0x100000;
    end = start + mem_size/sizeof(*start) - 1;

    diag_printf("Testing memory from %p to %p.\n", start, end);

    memTest((CYG_ADDRWORD)start, (CYG_ADDRWORD)end);

    diag_printf ("Memory test done.\n");
}

