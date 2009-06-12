//==========================================================================
//
//      uE250_pci.c
//
//      HAL support code for NMI uEngine uE250 PCI
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
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
// Author(s):    msalter
// Contributors: msalter, gthomas, David Mazur <david@mind.be>
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
#include <cyg/hal/plf_io.h>
#include <cyg/io/pci_hw.h>
#include <cyg/io/pci.h>

#ifdef CYGPKG_IO_PCI

#ifdef CYGSEM_HAL_LOAD_PCI_FPGA
static unsigned char uE250_pci_bitstream[] = {
#include "uE250_pci_bitstream.h"   
};
externC void load_fpga(void *bitstream, int len);
#endif

void
cyg_hal_plf_pci_init(void)
{
    cyg_uint8 next_bus;
    static bool _done = false;

    if (_done) return;
    _done = true;

//    diag_printf("Initializing PCI.\n");

#ifdef CYGSEM_HAL_LOAD_PCI_FPGA
    // Set MSC0 for FPGA configuration
    *PXA2X0_MSC1 = (*PXA2X0_MSC1 & 0x0000FFFF);

    // Program FPGA
    load_fpga(uE250_pci_bitstream, sizeof(uE250_pci_bitstream));
#endif

    *PXA2X0_MSC0 = (*PXA2X0_MSC0 & 0x0000FFFF) | 0x7ff10000;
    *PXA2X0_MSC1 = (*PXA2X0_MSC1 & 0x0000FFFF) | 0x70e40000;
    *PXA2X0_MSC2 = 0x70e470e4;

    *PXA2X0_MDREFR = 0x0009c018;
    *PXA2X0_MDCNFG = 0x03001bc9;

    // FIXME: Change MSC values ??
    // Set FPGA to 110.6 MHz
    *PXA2X0_GPDR0 |= (0x01 << 7);
    *PXA2X0_GPSR0 |= (0x01 << 7);
    *PXA2X0_GPDR1 |= (0x01 << (45-32));
    *PXA2X0_GPCR1 |= (0x01 << (45-32));
  
    // Set busmastering
    _pxa2x0_set_GPIO_mode(13, PXA2X0_GPIO_AF2, PXA2X0_GPIO_OUT);
    _pxa2x0_set_GPIO_mode(14, PXA2X0_GPIO_AF1, PXA2X0_GPIO_IN);

//    diag_printf("Activating PCI bridge.\n");
    PCICTL_MISC |= (1 | PCI_SDRAM_256) | PCI_TIMER;

    // Set command master
    cyg_hal_plf_pci_cfg_write_word(0, 0, CYG_PCI_CFG_COMMAND, 
                                   CYG_PCI_CFG_COMMAND_MEMORY|CYG_PCI_CFG_COMMAND_MASTER);

//    diag_printf("Scanning PCI bridge...\n");

    // Initialize PCI support
    cyg_pci_init();

    // Configure PCI bus.
    next_bus = 1;
    cyg_pci_configure_bus(0, &next_bus);

    // Set up to handle PCI interrupts
    HAL_INTERRUPT_CONFIGURE(CYGNUM_HAL_INTERRUPT_GPIO1, 0, 0);  // Falling edge
    HAL_INTERRUPT_UNMASK(CYGNUM_HAL_INTERRUPT_GPIO1);
    PCICTL_INT_RESET = 0xFF;  // Clear all pending interrupts
    PCICTL_INT_EDGE = 0xFF;   // Generate interrupts
    PCICTL_IRQ_MASK = 0x00;   // All masked

    if (0){
        cyg_uint8 devfn;
        cyg_pci_device_id devid;
        cyg_pci_device dev_info;
        int i;

        devid = CYG_PCI_DEV_MAKE_ID(next_bus-1, 0) | CYG_PCI_NULL_DEVFN;
        while (cyg_pci_find_next(devid, &devid)) {
            devfn = CYG_PCI_DEV_GET_DEVFN(devid);
            cyg_pci_get_device_info(devid, &dev_info);

            diag_printf("\n");
            diag_printf("Bus:        %d\n", CYG_PCI_DEV_GET_BUS(devid));
            diag_printf("PCI Device: %d\n", CYG_PCI_DEV_GET_DEV(devfn));
            diag_printf("PCI Func  : %d\n", CYG_PCI_DEV_GET_FN(devfn));
            diag_printf("Vendor Id : 0x%08X\n", dev_info.vendor);
            diag_printf("Device Id : 0x%08X\n", dev_info.device);
            diag_printf("Command:    0x%04X\n", dev_info.command);
            for (i = 0; i < dev_info.num_bars; i++) {
                diag_printf("  BAR[%d]    0x%08x /", i, dev_info.base_address[i]);
                diag_printf(" probed size 0x%08x / CPU addr 0x%08x\n",
                            dev_info.base_size[i], dev_info.base_map[i]);
            }
        }
    }
}

void 
_uE250_pci_translate_interrupt(int bus, int devfn, int *vector, int *valid)
{
    int dev = CYG_PCI_DEV_GET_DEV(devfn);

    if (dev <= 5) {
        *vector = _uPCI_BASE_INTERRUPT+(dev-1);
        valid = true;;
    } else {
        valid = false;
    }
}

static void
cyg_hal_plf_pci_clear_idsel(void)
{
    // Clear any active idsels
    PCICTL_MISC &= ~PCI_IDSEL_OFF;
    PCICTL_STATUS_REG = 0;
}

void 
cyg_hal_plf_pci_select_idsel(cyg_uint32 dev)
{
    cyg_hal_plf_pci_clear_idsel();
    // PCICTL_STATUS_REG = 0x0;
    PCICTL_MISC |= 1 + (((dev + 1) << PCI_IDSEL_SHIFT));
}

#define _PCI_ADDR(bus, devfn) (PCI_CONFIG_BASE |                        \
                               (bus << 16) |                            \
                               (CYG_PCI_DEV_GET_DEV(devfn) << 11) |     \
                               (CYG_PCI_DEV_GET_FN(devfn) << 8))

cyg_uint32 
cyg_hal_plf_pci_cfg_read_dword(cyg_uint32 bus, cyg_uint32 devfn, 
                               cyg_uint32 offset)
{
    cyg_uint32 config_data;
    volatile cyg_uint32 *address = (cyg_uint32 *)_PCI_ADDR(bus, devfn);

    cyg_hal_plf_pci_select_idsel(CYG_PCI_DEV_GET_DEV(devfn));
    config_data = address[offset >> 2];
    if ((PCICTL_STATUS_REG & 0x0020) == 0x0020) {
        // Configuration cycle failed
        config_data = 0xFFFFFFFF;
    }
    cyg_hal_plf_pci_clear_idsel();

    return config_data;
}

void 
cyg_hal_plf_pci_cfg_write_dword(cyg_uint32 bus, cyg_uint32 devfn, 
                                cyg_uint32 offset, cyg_uint32 data)
{
    volatile cyg_uint32 *address = (cyg_uint32 *)_PCI_ADDR(bus, devfn);

    cyg_hal_plf_pci_select_idsel(CYG_PCI_DEV_GET_DEV(devfn));
    address[offset >> 2] = data;
    cyg_hal_plf_pci_clear_idsel();
}



cyg_uint16 
cyg_hal_plf_pci_cfg_read_word(cyg_uint32 bus, cyg_uint32 devfn, 
                              cyg_uint32 offset)
{
    cyg_uint16 config_data;
    volatile cyg_uint16 *address = (cyg_uint16 *)_PCI_ADDR(bus, devfn); 

    cyg_hal_plf_pci_select_idsel(CYG_PCI_DEV_GET_DEV(devfn));
    config_data = address[offset >> 1];
    if ((PCICTL_STATUS_REG & 0x0020) == 0x0020) {
        // Configuration cycle failed
        config_data = 0xFFFF;
    }
    cyg_hal_plf_pci_clear_idsel();

    return config_data;
}

void 
cyg_hal_plf_pci_cfg_write_word(cyg_uint32 bus, cyg_uint32 devfn, 
                               cyg_uint32 offset, cyg_uint16 data)
{
    volatile cyg_uint16 *address = (cyg_uint16 *)_PCI_ADDR(bus, devfn);

    cyg_hal_plf_pci_select_idsel(CYG_PCI_DEV_GET_DEV(devfn));
    address[offset >> 1] = data;
    cyg_hal_plf_pci_clear_idsel();
}

cyg_uint8 
cyg_hal_plf_pci_cfg_read_byte(cyg_uint32 bus, cyg_uint32 devfn, 
                              cyg_uint32 offset)
{
    cyg_uint8 config_data;
    volatile cyg_uint8 *address = (cyg_uint8 *)_PCI_ADDR(bus, devfn);

    cyg_hal_plf_pci_select_idsel(CYG_PCI_DEV_GET_DEV(devfn));
    config_data = address[offset];
    if ((PCICTL_STATUS_REG & 0x0020) == 0x0020) {
        // Configuration cycle failed
        config_data = 0xFF;
    }
    cyg_hal_plf_pci_clear_idsel();

    return config_data;
}


void 
cyg_hal_plf_pci_cfg_write_byte(cyg_uint32 bus, cyg_uint32 devfn, 
                               cyg_uint32 offset, cyg_uint8 data)
{
    volatile cyg_uint8 *address = (cyg_uint8 *)_PCI_ADDR(bus, devfn);

    cyg_hal_plf_pci_select_idsel(CYG_PCI_DEV_GET_DEV(devfn));
    address[offset] = data;
    cyg_hal_plf_pci_clear_idsel();
}

//
// Note: PCI I/O space reads on this platform require additional 
// addressing gymnastics

/**
 * Reads an 8 bit value (byte) from the given address of the PCI 
 * IO space.
 **/
cyg_uint8 pci_io_read_8(cyg_uint32 address)
{
    volatile cyg_uint8 *addr = (volatile cyg_uint8 *)(((address & 0x00000003) << 22) | 
                                                      address | _PCI_READ_8);
    cyg_uint8 val = *addr;
#ifdef _PCI_DEBUG
    diag_printf("READ  PCI.8 [%p/%p] => %02x\n", address, addr, val);
#endif
    return val;
}

/**
 *  * Reads a 16 bit value (word) from the given address of the PCI 
 *   * IO space.
 *    */
cyg_uint16 pci_io_read_16(cyg_uint32 address)
{
    volatile cyg_uint16 *addr = (volatile cyg_uint16 *)(((address & 0x00000002) << 21) | 
                                                        address | _PCI_READ_16);
    cyg_uint16 val = *addr;
#ifdef _PCI_DEBUG
    diag_printf("READ  PCI.16 [%p/%p] => %04x\n", address, addr, val);
#endif
    return val;
}

/**
 *  * Reads a 32 bit value (double word) from the given address of the PCI 
 *   * IO space.
 *    */
cyg_uint32 pci_io_read_32(cyg_uint32 address)
{
    volatile cyg_uint32 *addr = (volatile cyg_uint32 *)(address | _PCI_READ_32);
    cyg_uint32 val = *addr;
#ifdef _PCI_DEBUG
    diag_printf("READ PCI.32 [%p/%p] => %02x\n", address, addr, val);
#endif
    return val;
} 

/**
 *  * Writes an 8 bit value (byte) into the given address of the PCI 
 *   * IO space.
 *    */
void pci_io_write_8(cyg_uint32 address, cyg_uint8 value)
{   
    *((volatile cyg_uint8 *)(address | _PCI_WRITE_X)) = value;
#ifdef _PCI_DEBUG
    diag_printf("WRITE PCI.8[%p] <= %02x\n", address | _PCI_WRITE_X, value);
#endif
}
  
/**
 *  * Writes a 16 bit value (single word) into the given address of the PCI 
 *   * IO space.
 *    */ 
void pci_io_write_16(cyg_uint32 address, cyg_uint16 value)
{
    *((volatile cyg_uint16 *)(address | _PCI_WRITE_X)) = value;
#ifdef _PCI_DEBUG
    diag_printf("WRITE PCI.16[%p] <= %04x\n", address | _PCI_WRITE_X, value);
#endif
}

/**
 *  * Writes a 32 bit value (double word) into the given address of the PCI 
 *   * IO space.
 *    */
void pci_io_write_32(cyg_uint32 address, cyg_uint32 value)
{
    *((volatile cyg_uint32 *)(address | _PCI_WRITE_X)) = value;
#ifdef _PCI_DEBUG
    diag_printf("WRITE PCI.32[%p] <= %08x\n", address | _PCI_WRITE_X, value);
#endif
}

#endif // CYGPKG_IO_PCI
