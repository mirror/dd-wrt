//==========================================================================
//
//      mb93091_pci.c
//
//      HAL PCI board support code for Fujitsu MB93091 motherboard
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
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2001-09-07
// Purpose:      HAL board support
// Description:  Implementations of HAL board interfaces
//
//####DESCRIPTIONEND####
//
//========================================================================*/


#include <pkgconf/io_pci.h>

#include <cyg/infra/diag.h>             // diag_printf() and friends

#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_diag.h>

#include <cyg/io/pci_hw.h>
#include <cyg/io/pci.h>

externC void
_mb93091_pci_init(void)
{
    static int _init = 0;
    cyg_uint8 next_bus;
    cyg_uint32 cmd_state;

    if (_init) return;
    _init = 1;

    cyg_pci_init();

    // Enable controller - most of the basic configuration
    // was set up at boot time in "platform.inc"

    // Setup for bus mastering
    HAL_PCI_CFG_READ_UINT32(0, CYG_PCI_DEV_MAKE_DEVFN(0,0),
                            CYG_PCI_CFG_COMMAND, cmd_state);
    if ((cmd_state & CYG_PCI_CFG_COMMAND_MEMORY) == 0) {
        HAL_PCI_CFG_WRITE_UINT32(0, CYG_PCI_DEV_MAKE_DEVFN(0,0),
                                 CYG_PCI_CFG_COMMAND,
                                 CYG_PCI_CFG_COMMAND_MEMORY |
                                 CYG_PCI_CFG_COMMAND_MASTER |
                                 CYG_PCI_CFG_COMMAND_PARITY |
                                 CYG_PCI_CFG_COMMAND_SERR);
    }

    // Setup latency timer field
    HAL_PCI_CFG_WRITE_UINT8(0, CYG_PCI_DEV_MAKE_DEVFN(0,0),
                                CYG_PCI_CFG_LATENCY_TIMER, 32);

    HAL_PCI_CFG_READ_UINT32(0, CYG_PCI_DEV_MAKE_DEVFN(0x10,0),
			    CYG_PCI_CFG_COMMAND, cmd_state);
    if ((cmd_state & CYG_PCI_CFG_COMMAND_ACTIVE)) {
	    /* It was already active. Turn it off so it gets
	       configured with a base address which is actually
	       in the range Redboot can see */
	    cmd_state &= ~CYG_PCI_CFG_COMMAND_ACTIVE;
	    HAL_PCI_CFG_WRITE_UINT32(0, CYG_PCI_DEV_MAKE_DEVFN(0x10,0),
					 CYG_PCI_CFG_COMMAND, cmd_state);
    }

    // Configure PCI bus.
    next_bus = 1;
    cyg_pci_configure_bus(0, &next_bus);
}

externC void 
_mb93091_pci_translate_interrupt(int bus, int devfn, int *vec, int *valid)
{
    cyg_uint8 req;                                                            
    cyg_uint8 dev = CYG_PCI_DEV_GET_DEV(devfn);

    if (dev == CYG_PCI_MIN_DEV) {
        // On board LAN
        *vec = CYGNUM_HAL_INTERRUPT_LAN;
        *valid = true;
    } else {
        HAL_PCI_CFG_READ_UINT8(bus, devfn, CYG_PCI_CFG_INT_PIN, req);         
        if (0 != req) {                                                           
            CYG_ADDRWORD __translation[4] = {                                       
                CYGNUM_HAL_INTERRUPT_PCIINTC,   /* INTC# */                         
                CYGNUM_HAL_INTERRUPT_PCIINTB,   /* INTB# */                         
                CYGNUM_HAL_INTERRUPT_PCIINTA,   /* INTA# */                         
                CYGNUM_HAL_INTERRUPT_PCIINTD};  /* INTD# */                         
                                                                                
            /* The PCI lines from the different slots are wired like this  */       
            /* on the PCI backplane:                                       */       
            /*                pin6A     pin7B    pin7A   pin8B             */       
            /* I/O Slot 1     INTA#     INTB#    INTC#   INTD#             */       
            /* I/O Slot 2     INTD#     INTA#    INTB#   INTC#             */       
            /* I/O Slot 3     INTC#     INTD#    INTA#   INTB#             */       
            /*                                                             */       
            /* (From PCI Development Backplane, 3.2.2 Interrupts)          */       
            /*                                                             */       
            /* Devsel signals are wired to, resulting in device IDs:       */       
            /* I/O Slot 1     AD30 / dev 19      [(8+1)&3 = 1]             */       
            /* I/O Slot 2     AD29 / dev 18      [(7+1)&3 = 0]             */       
            /* I/O Slot 3     AD28 / dev 17      [(6+1)&3 = 3]             */       
                                                                                
            *vec = __translation[((req+dev)&3)];        
            *valid = true;                                                         
        } else {                                                                    
            /* Device will not generate interrupt requests. */                      
            *valid = false;                                                        
        }                                                                           
        diag_printf("Int - dev: %d, req: %d, vector: %d\n", dev, req, *vec);
    }
}

// PCI configuration space access
#define _EXT_ENABLE 0x80000000  // Could be 0x80000000

static __inline__ cyg_uint32
_cfg_addr(int bus, int devfn, int offset)
{
    return _EXT_ENABLE | (bus << 22) | (devfn << 8) | (offset << 0);
}

externC cyg_uint8 
_mb93091_pci_cfg_read_uint8(int bus, int devfn, int offset)
{
    cyg_uint32 cfg_addr, addr, status;
    cyg_uint8 cfg_val = (cyg_uint8)0xFF;

    if (!_mb93091_has_vdk)
	    return cfg_val;
#ifdef CYGPKG_IO_PCI_DEBUG
    diag_printf("%s(bus=%x, devfn=%x, offset=%x) = ", __FUNCTION__, bus, devfn, offset);
#endif // CYGPKG_IO_PCI_DEBUG
    if ((bus == 0) && (CYG_PCI_DEV_GET_DEV(devfn) == 0)) {
        // PCI bridge
        addr = _MB93091_PCI_CONFIG + ((offset << 1) ^ 0x03);
    } else {
        cfg_addr = _cfg_addr(bus, devfn, offset ^ 0x03);
        HAL_WRITE_UINT32(_MB93091_PCI_CONFIG_ADDR, cfg_addr);
        addr = _MB93091_PCI_CONFIG_DATA + ((offset & 0x03) ^ 0x03);
    }
    HAL_READ_UINT8(addr, cfg_val);
    HAL_READ_UINT16(_MB93091_PCI_STAT_CMD, status);
    if (status & _MB93091_PCI_STAT_ERROR_MASK) {
        // Cycle failed - clean up and get out
        cfg_val = (cyg_uint8)0xFF;
        HAL_WRITE_UINT16(_MB93091_PCI_STAT_CMD, status & _MB93091_PCI_STAT_ERROR_MASK);
    }
#ifdef CYGPKG_IO_PCI_DEBUG
    diag_printf("%x\n", cfg_val);
#endif // CYGPKG_IO_PCI_DEBUG
    HAL_WRITE_UINT32(_MB93091_PCI_CONFIG_ADDR, 0);
    return cfg_val;
}

externC cyg_uint16 
_mb93091_pci_cfg_read_uint16(int bus, int devfn, int offset)
{
    cyg_uint32 cfg_addr, addr, status;
    cyg_uint16 cfg_val = (cyg_uint16)0xFFFF;

    if (!_mb93091_has_vdk)
	    return cfg_val;
#ifdef CYGPKG_IO_PCI_DEBUG
    diag_printf("%s(bus=%x, devfn=%x, offset=%x) = ", __FUNCTION__, bus, devfn, offset);
#endif // CYGPKG_IO_PCI_DEBUG
    if ((bus == 0) && (CYG_PCI_DEV_GET_DEV(devfn) == 0)) {
        // PCI bridge
        addr = _MB93091_PCI_CONFIG + ((offset << 1) ^ 0x02);
    } else {
        cfg_addr = _cfg_addr(bus, devfn, offset ^ 0x02);
        HAL_WRITE_UINT32(_MB93091_PCI_CONFIG_ADDR, cfg_addr);
        addr = _MB93091_PCI_CONFIG_DATA + ((offset & 0x03) ^ 0x02);
    }
    HAL_READ_UINT16(addr, cfg_val);
    HAL_READ_UINT16(_MB93091_PCI_STAT_CMD, status);
    if (status & _MB93091_PCI_STAT_ERROR_MASK) {
        // Cycle failed - clean up and get out
        cfg_val = (cyg_uint16)0xFFFF;
        HAL_WRITE_UINT16(_MB93091_PCI_STAT_CMD, status & _MB93091_PCI_STAT_ERROR_MASK);
    }
#ifdef CYGPKG_IO_PCI_DEBUG
    diag_printf("%x\n", cfg_val);
#endif // CYGPKG_IO_PCI_DEBUG
    HAL_WRITE_UINT32(_MB93091_PCI_CONFIG_ADDR, 0);
    return cfg_val;
}

externC cyg_uint32 
_mb93091_pci_cfg_read_uint32(int bus, int devfn, int offset)
{
    cyg_uint32 cfg_addr, addr, status;
    cyg_uint32 cfg_val = (cyg_uint32)0xFFFFFFFF;

    if (!_mb93091_has_vdk)
	    return cfg_val;
#ifdef CYGPKG_IO_PCI_DEBUG
    diag_printf("%s(bus=%x, devfn=%x, offset=%x) = ", __FUNCTION__, bus, devfn, offset);
#endif // CYGPKG_IO_PCI_DEBUG
    if ((bus == 0) && (CYG_PCI_DEV_GET_DEV(devfn) == 0)) {
        // PCI bridge
        addr = _MB93091_PCI_CONFIG + (offset << 1);
    } else {
        cfg_addr = _cfg_addr(bus, devfn, offset);
        HAL_WRITE_UINT32(_MB93091_PCI_CONFIG_ADDR, cfg_addr);
        addr = _MB93091_PCI_CONFIG_DATA;
    }
    HAL_READ_UINT32(addr, cfg_val);
    HAL_READ_UINT16(_MB93091_PCI_STAT_CMD, status);
    if (status & _MB93091_PCI_STAT_ERROR_MASK) {
        // Cycle failed - clean up and get out
        cfg_val = (cyg_uint32)0xFFFFFFFF;
        HAL_WRITE_UINT16(_MB93091_PCI_STAT_CMD, status & _MB93091_PCI_STAT_ERROR_MASK);
    }
#ifdef CYGPKG_IO_PCI_DEBUG
    diag_printf("%x\n", cfg_val);
#endif // CYGPKG_IO_PCI_DEBUG
    HAL_WRITE_UINT32(_MB93091_PCI_CONFIG_ADDR, 0);
    return cfg_val;
}

externC void
_mb93091_pci_cfg_write_uint8(int bus, int devfn, int offset, cyg_uint8 cfg_val)
{
    cyg_uint32 cfg_addr, addr, status;

    if (!_mb93091_has_vdk)
	    return;
#ifdef CYGPKG_IO_PCI_DEBUG
    diag_printf("%s(bus=%x, devfn=%x, offset=%x, val=%x)\n", __FUNCTION__, bus, devfn, offset, cfg_val);
#endif // CYGPKG_IO_PCI_DEBUG
    if ((bus == 0) && (CYG_PCI_DEV_GET_DEV(devfn) == 0)) {
        // PCI bridge
        addr = _MB93091_PCI_CONFIG + ((offset << 1) ^ 0x03);
    } else {
        cfg_addr = _cfg_addr(bus, devfn, offset ^ 0x03);
        HAL_WRITE_UINT32(_MB93091_PCI_CONFIG_ADDR, cfg_addr);
        addr = _MB93091_PCI_CONFIG_DATA + ((offset & 0x03) ^ 0x03);
    }
    HAL_WRITE_UINT8(addr, cfg_val);
    HAL_READ_UINT16(_MB93091_PCI_STAT_CMD, status);
    if (status & _MB93091_PCI_STAT_ERROR_MASK) {
        // Cycle failed - clean up and get out
        HAL_WRITE_UINT16(_MB93091_PCI_STAT_CMD, status & _MB93091_PCI_STAT_ERROR_MASK);
    }
    HAL_WRITE_UINT32(_MB93091_PCI_CONFIG_ADDR, 0);
}

externC void
_mb93091_pci_cfg_write_uint16(int bus, int devfn, int offset, cyg_uint16 cfg_val)
{
    cyg_uint32 cfg_addr, addr, status;

    if (!_mb93091_has_vdk)
	    return;
#ifdef CYGPKG_IO_PCI_DEBUG
    diag_printf("%s(bus=%x, devfn=%x, offset=%x, val=%x)\n", __FUNCTION__, bus, devfn, offset, cfg_val);
#endif // CYGPKG_IO_PCI_DEBUG
    if ((bus == 0) && (CYG_PCI_DEV_GET_DEV(devfn) == 0)) {
        // PCI bridge
        addr = _MB93091_PCI_CONFIG + ((offset << 1) ^ 0x02);
    } else {
        cfg_addr = _cfg_addr(bus, devfn, offset ^ 0x02);
        HAL_WRITE_UINT32(_MB93091_PCI_CONFIG_ADDR, cfg_addr);
        addr = _MB93091_PCI_CONFIG_DATA + ((offset & 0x03) ^ 0x02);
    }
    HAL_WRITE_UINT16(addr, cfg_val);
    HAL_READ_UINT16(_MB93091_PCI_STAT_CMD, status);
    if (status & _MB93091_PCI_STAT_ERROR_MASK) {
        // Cycle failed - clean up and get out
        HAL_WRITE_UINT16(_MB93091_PCI_STAT_CMD, status & _MB93091_PCI_STAT_ERROR_MASK);
    }
    HAL_WRITE_UINT32(_MB93091_PCI_CONFIG_ADDR, 0);
}

externC void
_mb93091_pci_cfg_write_uint32(int bus, int devfn, int offset, cyg_uint32 cfg_val)
{
    cyg_uint32 cfg_addr, addr, status;

    if (!_mb93091_has_vdk)
	    return;
#ifdef CYGPKG_IO_PCI_DEBUG
    diag_printf("%s(bus=%x, devfn=%x, offset=%x, val=%x)\n", __FUNCTION__, bus, devfn, offset, cfg_val);
#endif // CYGPKG_IO_PCI_DEBUG
    if ((bus == 0) && (CYG_PCI_DEV_GET_DEV(devfn) == 0)) {
        // PCI bridge
        addr = _MB93091_PCI_CONFIG + (offset << 1);
    } else {
        cfg_addr = _cfg_addr(bus, devfn, offset);
        HAL_WRITE_UINT32(_MB93091_PCI_CONFIG_ADDR, cfg_addr);
        addr = _MB93091_PCI_CONFIG_DATA;
    }
    HAL_WRITE_UINT32(addr, cfg_val);
    HAL_READ_UINT16(_MB93091_PCI_STAT_CMD, status);
    if (status & _MB93091_PCI_STAT_ERROR_MASK) {
        // Cycle failed - clean up and get out
        HAL_WRITE_UINT16(_MB93091_PCI_STAT_CMD, status & _MB93091_PCI_STAT_ERROR_MASK);
    }
    HAL_WRITE_UINT32(_MB93091_PCI_CONFIG_ADDR, 0);
}
