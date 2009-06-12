//==========================================================================
//
//      ppc405_pci.c
//
//      HAL variant support code for PCI on PowerPC 405GP
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
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
// Purpose:      HAL PCI support
// Description:  Implementations of HAL PCI interfaces
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#define CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include CYGBLD_HAL_PLATFORM_H

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros
#include <cyg/infra/diag.h>             // diag_printf() and friends

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>           // Interrupt names
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/ppc_regs.h>           // Hardware definitions
#include <cyg/hal/hal_if.h>             // calling interface API

#include <pkgconf/io_pci.h>
#include <cyg/io/pci_hw.h>
#include <cyg/io/pci.h>

// PCI support

externC void
hal_ppc405_pci_init(void)
{
    static int _init = 0;
    cyg_uint8 next_bus;
    cyg_uint32 cmd_state, bridge_state;

    if (_init) return;
    _init = 1;

    // Configure PCI bridge
    HAL_WRITE_UINT32LE(PCIL0_PMM0PCILA, 0);
    HAL_WRITE_UINT32LE(PCIL0_PMM0PCIHA, 0);
    HAL_WRITE_UINT32LE(PCIL0_PMM0LA, HAL_PCI_PHYSICAL_MEMORY_BASE);
    HAL_WRITE_UINT32LE(PCIL0_PMM0MA, ~(0x10000000-1) | 0x00000001);
    HAL_PCI_CFG_WRITE_UINT32(0, CYG_PCI_DEV_MAKE_DEVFN(0,0), CYG_PCI_CFG_BAR_1, 0);    
    HAL_WRITE_UINT32LE(PCIL0_PTM1LA, 0);
    HAL_WRITE_UINT32LE(PCIL0_PTM1MS, ~(0x10000000-1) | 0x00000001);
    // Indicate that the bridge has been configured
    HAL_PCI_CFG_READ_UINT32(0, CYG_PCI_DEV_MAKE_DEVFN(0,0), 0x60, bridge_state);
    bridge_state |= 0x0001;
    HAL_PCI_CFG_WRITE_UINT32(0, CYG_PCI_DEV_MAKE_DEVFN(0,0), 0x60, bridge_state);
    // Setup for bus mastering
    HAL_PCI_CFG_READ_UINT32(0, CYG_PCI_DEV_MAKE_DEVFN(0,0),
                            CYG_PCI_CFG_COMMAND, cmd_state);
    cyg_pci_init();
    if ((cmd_state & CYG_PCI_CFG_COMMAND_MEMORY) == 0) {
#if defined(CYGPKG_IO_PCI_DEBUG)
        diag_printf("Configure PCI bus\n");
#endif
        HAL_PCI_CFG_WRITE_UINT32(0, CYG_PCI_DEV_MAKE_DEVFN(0,0),
                                 CYG_PCI_CFG_COMMAND,
                                 CYG_PCI_CFG_COMMAND_MEMORY |
                                 CYG_PCI_CFG_COMMAND_MASTER |
                                 CYG_PCI_CFG_COMMAND_PARITY |
                                 CYG_PCI_CFG_COMMAND_SERR);

        // Setup latency timer field
        HAL_PCI_CFG_WRITE_UINT8(0, CYG_PCI_DEV_MAKE_DEVFN(0,0),
                                CYG_PCI_CFG_LATENCY_TIMER, 32);

        // Configure PCI bus.
        next_bus = 1;
        cyg_pci_configure_bus(0, &next_bus);
    }
#if defined(CYGSEM_HAL_POWERPC_PPC405_PCI_SHOW_BUS)
    if (1) {
        cyg_uint8 req;                                                            
        cyg_uint8 devfn;
        cyg_pci_device_id devid;
        cyg_pci_device dev_info;
        int i;

        devid = CYG_PCI_DEV_MAKE_ID(next_bus-1, 0) | CYG_PCI_NULL_DEVFN;
        while (cyg_pci_find_next(devid, &devid)) {
            devfn = CYG_PCI_DEV_GET_DEVFN(devid);
            cyg_pci_get_device_info(devid, &dev_info);
            HAL_PCI_CFG_READ_UINT8(0, devfn, CYG_PCI_CFG_INT_PIN, req);         

            diag_printf("\n");
            diag_printf("Bus: %d", CYG_PCI_DEV_GET_BUS(devid));
            diag_printf(", PCI Device: %d", CYG_PCI_DEV_GET_DEV(devfn));
            diag_printf(", PCI Func: %d\n", CYG_PCI_DEV_GET_FN(devfn));
            diag_printf("  Vendor Id: 0x%04X", dev_info.vendor);
            diag_printf(", Device Id: 0x%04X", dev_info.device);
            diag_printf(", Command: 0x%04X", dev_info.command);
            diag_printf(", IRQ: %d\n", req);
            for (i = 0; i < dev_info.num_bars; i++) {
                diag_printf("  BAR[%d]    0x%08x /", i, dev_info.base_address[i]);
                diag_printf(" probed size 0x%08x / CPU addr 0x%08x\n",
                            dev_info.base_size[i], dev_info.base_map[i]);
            }
        }
    }
#endif
}

externC void 
hal_ppc405_pci_translate_interrupt(int bus, int devfn, int *vec, int *valid)
{
    cyg_uint8 req;                                                            
    cyg_uint8 dev = CYG_PCI_DEV_GET_DEV(devfn);

    if ((dev >= CYG_PCI_MIN_DEV) && (dev < CYG_PCI_MAX_DEV)) {
        HAL_PCI_CFG_READ_UINT8(bus, devfn, CYG_PCI_CFG_INT_PIN, req);         
        if (0 != req) {                                                           
#ifdef CYG_PCI_IRQ_MAP
            char pci_irq_table[][4] = CYG_PCI_IRQ_MAP;
#else
#error "Need platform defined IRQ map"
#endif
            *vec = pci_irq_table[dev-CYG_PCI_MIN_DEV][req-1];
            *valid = (*vec != -1);
        } else {                                                                    
            /* Device will not generate interrupt requests. */                      
            *valid = false;                                                        
        }                                                                           
#if defined(CYGPKG_IO_PCI_DEBUG)
        diag_printf("Int - dev: %d, req: %d, vector: %d\n", dev, req, *vec);
#endif
    } else {
        *valid = false;  // Invalid device
    }
}

// PCI configuration space access
#define _EXT_ENABLE 0x80000000  // Could be 0x80000000

static __inline__ cyg_uint32
_cfg_addr(int bus, int devfn, int offset)
{
    return _EXT_ENABLE | (bus << 16) | (devfn << 8) | (offset << 0);
}

externC cyg_uint8 
hal_ppc405_pci_cfg_read_uint8(int bus, int devfn, int offset)
{
    cyg_uint32 cfg_addr;
    cyg_uint8 cfg_val = (cyg_uint8) 0xFF;

#ifdef CYGPKG_IO_PCI_DEBUG
    diag_printf("%s(bus=%x, devfn=%x, offset=%x) = ", __FUNCTION__, bus, devfn, offset);
#endif // CYGPKG_IO_PCI_DEBUG
    cfg_addr = _cfg_addr(bus, devfn, offset);
    HAL_WRITE_UINT32LE(PCIC0_CFGADDR, cfg_addr);
    HAL_READ_UINT8LE(PCIC0_CFGDATA|(offset & 0x03), cfg_val);
#ifdef CYGPKG_IO_PCI_DEBUG
    diag_printf("%x\n", cfg_val);
#endif // CYGPKG_IO_PCI_DEBUG
    return cfg_val;
}

externC cyg_uint16 
hal_ppc405_pci_cfg_read_uint16(int bus, int devfn, int offset)
{
    cyg_uint32 cfg_addr;
    cyg_uint16 cfg_val = (cyg_uint16) 0xFFFF;

#ifdef CYGPKG_IO_PCI_DEBUG
    diag_printf("%s(bus=%x, devfn=%x, offset=%x) = ", __FUNCTION__, bus, devfn, offset);
#endif // CYGPKG_IO_PCI_DEBUG
    cfg_addr = _cfg_addr(bus, devfn, offset);
    HAL_WRITE_UINT32LE(PCIC0_CFGADDR, cfg_addr);
    HAL_READ_UINT16LE(PCIC0_CFGDATA|(offset & 0x03), cfg_val);
#ifdef CYGPKG_IO_PCI_DEBUG
    diag_printf("%x\n", cfg_val);
#endif // CYGPKG_IO_PCI_DEBUG
    return cfg_val;
}

externC cyg_uint32 
hal_ppc405_pci_cfg_read_uint32(int bus, int devfn, int offset)
{
    cyg_uint32 cfg_addr;
    cyg_uint32 cfg_val = (cyg_uint32) 0xFFFFFFFF;

#ifdef CYGPKG_IO_PCI_DEBUG
    diag_printf("%s(bus=%x, devfn=%x, offset=%x) = ", __FUNCTION__, bus, devfn, offset);
#endif // CYGPKG_IO_PCI_DEBUG
    cfg_addr = _cfg_addr(bus, devfn, offset);
    HAL_WRITE_UINT32LE(PCIC0_CFGADDR, cfg_addr);
    HAL_READ_UINT32LE(PCIC0_CFGDATA, cfg_val);
#ifdef CYGPKG_IO_PCI_DEBUG
    diag_printf("%x\n", cfg_val);
#endif // CYGPKG_IO_PCI_DEBUG
    return cfg_val;
}

externC void
hal_ppc405_pci_cfg_write_uint8(int bus, int devfn, int offset, cyg_uint8 cfg_val)
{
    cyg_uint32 cfg_addr;

#ifdef CYGPKG_IO_PCI_DEBUG
    diag_printf("%s(bus=%x, devfn=%x, offset=%x, val=%x)\n", __FUNCTION__, bus, devfn, offset, cfg_val);
#endif // CYGPKG_IO_PCI_DEBUG
    cfg_addr = _cfg_addr(bus, devfn, offset);
    HAL_WRITE_UINT32LE(PCIC0_CFGADDR, cfg_addr);
    HAL_WRITE_UINT8LE(PCIC0_CFGDATA|(offset & 0x03), cfg_val);
}

externC void
hal_ppc405_pci_cfg_write_uint16(int bus, int devfn, int offset, cyg_uint16 cfg_val)
{
    cyg_uint32 cfg_addr;

#ifdef CYGPKG_IO_PCI_DEBUG
    diag_printf("%s(bus=%x, devfn=%x, offset=%x, val=%x)\n", __FUNCTION__, bus, devfn, offset, cfg_val);
#endif // CYGPKG_IO_PCI_DEBUG
    cfg_addr = _cfg_addr(bus, devfn, offset);
    HAL_WRITE_UINT32LE(PCIC0_CFGADDR, cfg_addr);
    HAL_WRITE_UINT16LE(PCIC0_CFGDATA|(offset & 0x03), cfg_val);
}

externC void
hal_ppc405_pci_cfg_write_uint32(int bus, int devfn, int offset, cyg_uint32 cfg_val)
{
    cyg_uint32 cfg_addr;

#ifdef CYGPKG_IO_PCI_DEBUG
    diag_printf("%s(bus=%x, devfn=%x, offset=%x, val=%x)\n", __FUNCTION__, bus, devfn, offset, cfg_val);
#endif // CYGPKG_IO_PCI_DEBUG
    cfg_addr = _cfg_addr(bus, devfn, offset);
    HAL_WRITE_UINT32LE(PCIC0_CFGADDR, cfg_addr);
    HAL_WRITE_UINT32LE(PCIC0_CFGDATA, cfg_val);
}

/*------------------------------------------------------------------------*/
// EOF ppc405_pci.c
