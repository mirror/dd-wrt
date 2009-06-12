//==========================================================================
//
//      uE250_plx.c
//
//      HAL support code for NMI uEngine uE250 PCI Local Bus (PLX)
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2003 Gary Thomas <gary@mind.be>
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
// Author(s):    David Mazur <david@mind.be>
// Contributors: gthomas
// Date:         2003-02-21
// Purpose:      uPCI local bus (PLX/9080) support
// Description:  
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
#include <cyg/infra/diag.h>

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_if.h>             // calling interface API
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_intr.h>           // Interrupt names
#include <cyg/hal/hal_cache.h>
#include <cyg/io/pci_hw.h>
#include <cyg/io/pci.h>

#ifdef CYGPKG_IO_PCI

externC void initialize_vga(void);

#define _DEFINE_VARS
#include <cyg/hal/plx.h>

/**
 * Initial values for the configuration registers of the PLX 9080
 */
static struct plx_init {
    cyg_uint32 address, value;
} plx_init_values[] = {
    { 0x0, 0xffe00000 },
    { 0x4, 0x80000001 },
    { 0x8, 0x0 },
    { 0xc, 0x0 },
    { 0x10,0xffe00000 },
    { 0x14,0x80000000 },
    { 0x18,0x40030141 },
    { 0x1c,0x0 },
    { 0x20,0x0 },
    { 0x24,0x0 },
    { 0x28,0x0 },
    { 0x2c,0x0 },
    { 0x78,0x0 },
    { 0x7c,0x0 },
    { 0xf0,0xffff8000 },
    { 0xf4,0x40008001 },
    { 0xf8,0x00000243 },
    { 0x900, 0x68 },
    { 0xFFFFFFFF,0},  // End marker
};

#ifdef CYGSEM_HAL_LOAD_VGA_FPGA
static unsigned char uE250_plx_bitstream[] = {
#include "uE250_plx_bitstream.h"   
};
externC void load_vga(cyg_uint8 *base, int len);
#endif

#if 0
static void
_show_pci_device(cyg_pci_device_id devid)
{
    int i, bus, devfn;
    cyg_pci_device dev_info;

    bus = CYG_PCI_DEV_GET_BUS(devid);
    devfn = CYG_PCI_DEV_GET_DEVFN(devid);
    cyg_pci_get_device_info(devid, &dev_info);
    diag_printf("\n");
    diag_printf("Bus:        %d\n", CYG_PCI_DEV_GET_BUS(devid));
    diag_printf("PCI Device: %d\n", CYG_PCI_DEV_GET_DEV(devid));
    diag_printf("PCI Func  : %d\n", CYG_PCI_DEV_GET_FN(devfn));
    diag_printf("Vendor Id : 0x%08X\n", dev_info.vendor);
    diag_printf("Device Id : 0x%08X\n", dev_info.device);
    diag_printf("Command:    %02x\n", dev_info.command);
    for (i = 0; i < CYG_PCI_MAX_BAR; i++) {
        diag_printf("  BAR[%d]    0x%08x /", i, dev_info.base_address[i]);
        diag_printf(" probed size 0x%08x / CPU addr 0x%08x\n",
                    dev_info.base_size[i], dev_info.base_map[i]);
    }
}
#endif

static /*__inline__*/ void 
write_fdc37c672_configreg(cyg_uint8 regno, cyg_uint8 data)
{
    localbus_writeb(regno, FDC37C672_INDEX);
    localbus_writeb(data, FDC37C672_DATA);
}

static /*__inline__*/ cyg_uint8 
read_fdc37c672_configreg(cyg_uint8 regno)
{
    localbus_writeb(regno, FDC37C672_INDEX);
    return localbus_readb(FDC37C672_DATA) & 0xff;
}

/**
 *  Sets up the Base Address Registers of the PLX 9080
 */
void 
initialize_plx_bridge(void)
{
    cyg_pci_device_id plx_dev = CYG_PCI_NULL_DEVID;
    struct plx_init *init = plx_init_values;
    int bus, devfn;
    cyg_pci_device dev_info;

//    diag_printf("Initializing PLX-9080 localbus controller\n");

    if (cyg_pci_find_device((cyg_uint16)0x10B5, (cyg_uint16)0x9080, &plx_dev)) {
        cyg_pci_get_device_info(plx_dev, &dev_info);
        bus = CYG_PCI_DEV_GET_BUS(plx_dev);
        devfn = CYG_PCI_DEV_GET_DEVFN(plx_dev);

        // Run PLX/9080 initialization sequence
        _plx_config_addr = dev_info.base_map[0] + LOCALBUS_CONFIG_OFFSET;
        while (init->address != (cyg_uint32)0xFFFFFFFF) {
            plx_config_writel(init->value, init->address);
            init++;
        }

        // Turn device off so it can be re-configured
        cyg_hal_plf_pci_cfg_write_byte(bus, devfn, CYG_PCI_CFG_COMMAND, 0);
        // This lets the library probe the device so it can be setup.
        cyg_pci_get_device_info(plx_dev, &dev_info);
        // Now, assign new resources to the device
        cyg_pci_configure_device(&dev_info);

        // Re-enable device now that is's been configured
        cyg_hal_plf_pci_cfg_write_byte(bus, devfn, CYG_PCI_CFG_COMMAND, 
                                       CYG_PCI_CFG_COMMAND_IO | CYG_PCI_CFG_COMMAND_MEMORY);

        // See what was assigned - for later use
        cyg_pci_get_device_info(plx_dev, &dev_info);
        _plx_config_addr = dev_info.base_map[0] + LOCALBUS_CONFIG_OFFSET;
        _plx_localbus_addr = dev_info.base_map[3];

        // A little commercial plug :-)
        localbus_writeb('M', ASCII_DISPLAY_BASE+8);
        localbus_writeb('I', ASCII_DISPLAY_BASE+12);
        localbus_writeb('N', ASCII_DISPLAY_BASE+16);
        localbus_writeb('D', ASCII_DISPLAY_BASE+20);

        // Configure SMSC FDC36c672 Super I/O controller

        localbus_writeb(0x55,FDC37C672_CONFIG); // Enter FDC37C672 configuration mode

        write_fdc37c672_configreg(7, 3);        // parallel port to IRQ 7, IO 0x378
        write_fdc37c672_configreg(0x30, 1);
        write_fdc37c672_configreg(0x70, 7);
        write_fdc37c672_configreg(0x60, 0x3);
        write_fdc37c672_configreg(0x61, 0x78);

        write_fdc37c672_configreg(7, 4);        // first serial port to IRQ4, IO 0x3f8 
        write_fdc37c672_configreg(0x30, 1);
        write_fdc37c672_configreg(0x70, 4);
        write_fdc37c672_configreg(0x60, 0x3);
        write_fdc37c672_configreg(0x61, 0xf8);

        write_fdc37c672_configreg(7, 5);        // second serial port to IRQ5, IO 0x2f8
        write_fdc37c672_configreg(0x30, 1);
        write_fdc37c672_configreg(0x70, 3);
        write_fdc37c672_configreg(0x60, 0x2);
        write_fdc37c672_configreg(0x61, 0xf8);

        write_fdc37c672_configreg(7, 7);        // PS/2 Keyboard/mouse to IRQ 1/5, IO 0x60 
        write_fdc37c672_configreg(0x30, 1);
        write_fdc37c672_configreg(0x70, 1);
        write_fdc37c672_configreg(0x72, 5);

        localbus_writeb(0xAA,FDC37C672_CONFIG); // Leave FDC37C672 configuration mode 

#ifdef CYGSEM_HAL_LOAD_VGA_FPGA
        load_vga(uE250_plx_bitstream, sizeof(uE250_plx_bitstream));
#endif
#ifdef CYGSEM_UE250_VGA_COMM
        vga_comm_init(dev_info.base_map[2]);
#endif
    } else {
        diag_printf("Can't find PLX controller!\n");
    }
}

#endif // CYGPKG_IO_PCI
