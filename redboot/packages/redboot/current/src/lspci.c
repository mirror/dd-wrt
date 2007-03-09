//==========================================================================
//
//      lspci.c
//
//      RedBoot PCI bus info dump
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2004 Red Hat, Inc.
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
// Contributors: msalter
// Date:         2004-09-15
// Purpose:      
// Description:  
//              
// This code is part of RedBoot (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <redboot.h>

#ifdef CYGPKG_IO_PCI

#include <cyg/io/pci.h>

static void
do_lspci(int argc, char *argv[])
{
    cyg_uint8 devfn;
    cyg_pci_device_id devid = CYG_PCI_NULL_DEVID;
    cyg_pci_device dev_info;
    int i;

    while (cyg_pci_find_next(devid, &devid)) {
        devfn = CYG_PCI_DEV_GET_DEVFN(devid);
	
	// Get the device info
	cyg_pci_get_device_info(devid, &dev_info);

	diag_printf("%d:%d:%d   ",
		    CYG_PCI_DEV_GET_BUS(devid),
		    CYG_PCI_DEV_GET_DEV(devfn),
		    CYG_PCI_DEV_GET_FN(devfn));

	diag_printf("Vendor[%04x] Device[%04x] Type[%02x] Class[%06x]\n",
		    dev_info.vendor,
		    dev_info.device,
		    dev_info.header_type,
		    (dev_info.class_rev >> 8) & 0xffffff);

	if (dev_info.command & CYG_PCI_CFG_COMMAND_ACTIVE) {
	    // dump bars
	    for (i = 0; i < CYG_PCI_MAX_BAR; i++)
		if (dev_info.base_address[i])
		    diag_printf("        BAR%d: %08x\n", i, dev_info.base_address[i]);
	}
    }
}

RedBoot_cmd("lspci", 
            "Dump information on PCI devices",
            "",
            do_lspci
    );

#endif  // CYGPKG_IO_PCI
