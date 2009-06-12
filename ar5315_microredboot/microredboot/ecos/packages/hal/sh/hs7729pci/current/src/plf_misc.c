//==========================================================================
//
//      plf_misc.c
//
//      HAL platform miscellaneous functions
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
// Author(s):    jskov
// Contributors: jskov
// Date:         2001-05-25
// Purpose:      HAL miscellaneous functions
// Description:  This file contains miscellaneous functions provided by the
//               HAL.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>

#include <cyg/hal/hal_if.h>             // interfacing API
#include <cyg/hal/plf_io.h>
#include <cyg/hal/drv_api.h>            // interrupt handling

//--------------------------------------------------------------------------
externC void cyg_hal_init_superIO(void);
static cyg_uint32 cyg_hal_plf_pci_arbiter(CYG_ADDRWORD vector, CYG_ADDRWORD data);

static cyg_interrupt intr;
static cyg_handle_t  intr_handle;

void
hal_platform_init(void)
{
    // Init superIO before calling if_init (which will use UARTs)
    cyg_hal_init_superIO();

    hal_if_init();

#if defined(CYGPKG_REDBOOT) && defined(CYGPKG_IO_PCI)
    cyg_hal_plf_pci_init();
#endif

    // Set up interrupt arbiter
    cyg_drv_interrupt_create(CYGNUM_HAL_INTERRUPT_PCI, 1,
                             0, cyg_hal_plf_pci_arbiter, NULL,
                             &intr_handle, &intr);
    cyg_drv_interrupt_attach(intr_handle);
    cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_PCI);
}

#if defined(CYGPKG_IO_PCI)
//--------------------------------------------------------------------------
// PCI stuff

// For some reason the PCI config cycles only succeed with some
// delays at suitable places.
#define _DELAY() do { int i; for (i = 0; i < 100; i++) ; } while(0)

#include <cyg/io/pci_hw.h>
#include <cyg/io/pci.h>
#include <cyg/hal/hal_if.h>
#include <cyg/hal/hal_arbiter.h>        // hal_call_isr

void
cyg_hal_plf_pci_init(void)
{
    cyg_uint8  next_bus;

    static int initialized = 0;
    if (initialized) return;
    initialized = 1;

    // Set PCI bases
    HAL_WRITE_UINT32(CYGARC_REG_PCI_IO_MEMOFFSET, CYGARC_BUS_ADDRESS(HAL_PCI_ALLOC_BASE_IO));
    HAL_WRITE_UINT32(CYGARC_REG_PCI_MEM_MEMOFFSET, CYGARC_BUS_ADDRESS(HAL_PCI_ALLOC_BASE_MEMORY));

    // Reset PCI - this does not have the desired effect; devices remain enabled.
    HAL_WRITE_UINT32(CYGARC_REG_SD0001_RESET, CYGARC_REG_SD0001_RESET_PCIRST);
    CYGACC_CALL_IF_DELAY_US(100);

    // Bring PCI out of reset
    HAL_WRITE_UINT32(CYGARC_REG_SD0001_RESET, 0);
    CYGACC_CALL_IF_DELAY_US(10000);

    // Set PCI access timeouts/retries to max
    HAL_WRITE_UINT32(CYGARC_REG_SD0001_PCI_CTL, (CYGARC_REG_SD0001_PCI_CTL_MAX_DEADLOCK_CNT
                                                 |CYGARC_REG_SD0001_PCI_CTL_MAX_RETRY_CNT));
    CYGACC_CALL_IF_DELAY_US(10000);

    // Enable controller
    // Setup for bus mastering
    cyg_hal_plf_pci_cfg_write_dword(0, CYG_PCI_DEV_MAKE_DEVFN(0,0),
				    CYG_PCI_CFG_COMMAND,
				    CYG_PCI_CFG_COMMAND_MEMORY |
				    CYG_PCI_CFG_COMMAND_MASTER |
				    CYG_PCI_CFG_COMMAND_PARITY |
				    CYG_PCI_CFG_COMMAND_SERR);

    // Setup latency timer field
    cyg_hal_plf_pci_cfg_write_byte(0, CYG_PCI_DEV_MAKE_DEVFN(0,0),
				   CYG_PCI_CFG_LATENCY_TIMER, 32);

    // Set memory base
    cyg_hal_plf_pci_cfg_write_dword(0, CYG_PCI_DEV_MAKE_DEVFN(0,0),
				    CYG_PCI_CFG_BAR_1, 0x0c000008);

    // Configure PCI bus.
    next_bus = 1;
    cyg_pci_configure_bus(0, &next_bus);
}

//--------------------------------------------------------------------------
// Config space accessor functions
cyg_uint32
cyg_hal_plf_pci_cfg_read_dword (cyg_uint32 bus, cyg_uint32 devfn,
                                cyg_uint32 offset)
{
    cyg_uint32 config_data;

    HAL_WRITE_UINT32(CYGARC_REG_PCI_CFG_ADDR,
                     CYGARC_REG_PCI_CFG_ADDR_ENABLE |
                     (bus << CYGARC_REG_PCI_CFG_ADDR_BUSNO_shift) |
                     (devfn << CYGARC_REG_PCI_CFG_ADDR_FUNC_shift) |
                     (offset));
    HAL_WRITE_UINT32(CYGARC_REG_PCI_CFG_CMD, CYGARC_REG_PCI_CFG_CMD_RCFG);
    _DELAY();
    HAL_READ_UINT32(CYGARC_REG_PCI_CFG_DATA, config_data);

    return config_data;
}

cyg_uint16
cyg_hal_plf_pci_cfg_read_word (cyg_uint32 bus, cyg_uint32 devfn,
                               cyg_uint32 offset)
{
    cyg_uint32 config_dword;

    HAL_WRITE_UINT32(CYGARC_REG_PCI_CFG_ADDR,
                     CYGARC_REG_PCI_CFG_ADDR_ENABLE |
                     (bus << CYGARC_REG_PCI_CFG_ADDR_BUSNO_shift) |
                     (devfn << CYGARC_REG_PCI_CFG_ADDR_FUNC_shift) |
                     (offset & ~3));
    HAL_WRITE_UINT32(CYGARC_REG_PCI_CFG_CMD, CYGARC_REG_PCI_CFG_CMD_RCFG);
    _DELAY();
    HAL_READ_UINT32(CYGARC_REG_PCI_CFG_DATA, config_dword);

    return (cyg_uint16)((config_dword >> ((offset & 3) * 8)) & 0xffff);
}

cyg_uint8
cyg_hal_plf_pci_cfg_read_byte (cyg_uint32 bus, cyg_uint32 devfn, 
                               cyg_uint32 offset)
{
    cyg_uint32 config_dword;

    HAL_WRITE_UINT32(CYGARC_REG_PCI_CFG_ADDR,
                     CYGARC_REG_PCI_CFG_ADDR_ENABLE |
                     (bus << CYGARC_REG_PCI_CFG_ADDR_BUSNO_shift) |
                     (devfn << CYGARC_REG_PCI_CFG_ADDR_FUNC_shift) |
                     (offset & ~3));
    HAL_WRITE_UINT32(CYGARC_REG_PCI_CFG_CMD, CYGARC_REG_PCI_CFG_CMD_RCFG);
    _DELAY();
    HAL_READ_UINT32(CYGARC_REG_PCI_CFG_DATA, config_dword);

    return (cyg_uint8)((config_dword >> ((offset & 3) * 8)) & 0xff);
}

void
cyg_hal_plf_pci_cfg_write_dword (cyg_uint32 bus, cyg_uint32 devfn,
                                 cyg_uint32 offset, cyg_uint32 data)
{
    HAL_WRITE_UINT32(CYGARC_REG_PCI_CFG_ADDR,
                     CYGARC_REG_PCI_CFG_ADDR_ENABLE |
                     (bus << CYGARC_REG_PCI_CFG_ADDR_BUSNO_shift) |
                     (devfn << CYGARC_REG_PCI_CFG_ADDR_FUNC_shift) |
                     (offset));
    HAL_WRITE_UINT32(CYGARC_REG_PCI_CFG_DATA, data);
    _DELAY();
    HAL_WRITE_UINT32(CYGARC_REG_PCI_CFG_CMD, CYGARC_REG_PCI_CFG_CMD_WCFG);
    _DELAY();
}

void
cyg_hal_plf_pci_cfg_write_word (cyg_uint32 bus, cyg_uint32 devfn,
                                cyg_uint32 offset, cyg_uint16 data)
{
    cyg_uint32 config_dword, shift;

    HAL_WRITE_UINT32(CYGARC_REG_PCI_CFG_ADDR,
                     CYGARC_REG_PCI_CFG_ADDR_ENABLE |
                     (bus << CYGARC_REG_PCI_CFG_ADDR_BUSNO_shift) |
                     (devfn << CYGARC_REG_PCI_CFG_ADDR_FUNC_shift) |
                     (offset & ~3));
    HAL_WRITE_UINT32(CYGARC_REG_PCI_CFG_CMD, CYGARC_REG_PCI_CFG_CMD_RCFG);
    _DELAY();
    HAL_READ_UINT32(CYGARC_REG_PCI_CFG_DATA, config_dword);

    shift = (offset & 3) * 8;
    config_dword &= ~(0xffff << shift);
    config_dword |= (data << shift);

    HAL_WRITE_UINT32(CYGARC_REG_PCI_CFG_ADDR,
                     CYGARC_REG_PCI_CFG_ADDR_ENABLE |
                     (bus << CYGARC_REG_PCI_CFG_ADDR_BUSNO_shift) |
                     (devfn << CYGARC_REG_PCI_CFG_ADDR_FUNC_shift) |
                     (offset & ~3));
    HAL_WRITE_UINT32(CYGARC_REG_PCI_CFG_DATA, config_dword);
    _DELAY();
    HAL_WRITE_UINT32(CYGARC_REG_PCI_CFG_CMD, CYGARC_REG_PCI_CFG_CMD_WCFG);
    _DELAY();
}

void 
cyg_hal_plf_pci_cfg_write_byte (cyg_uint32 bus, cyg_uint32 devfn,
                                cyg_uint32 offset, cyg_uint8  data)
{
    cyg_uint32 config_dword, shift;

    HAL_WRITE_UINT32(CYGARC_REG_PCI_CFG_ADDR,
                     CYGARC_REG_PCI_CFG_ADDR_ENABLE |
                     (bus << CYGARC_REG_PCI_CFG_ADDR_BUSNO_shift) |
                     (devfn << CYGARC_REG_PCI_CFG_ADDR_FUNC_shift) |
                     (offset & ~3));
    HAL_WRITE_UINT32(CYGARC_REG_PCI_CFG_CMD, CYGARC_REG_PCI_CFG_CMD_RCFG);
    _DELAY();
    HAL_READ_UINT32(CYGARC_REG_PCI_CFG_DATA, config_dword);

    shift = (offset & 3) * 8;
    config_dword &= ~(0xff << shift);
    config_dword |= (data << shift);

    HAL_WRITE_UINT32(CYGARC_REG_PCI_CFG_ADDR,
                     CYGARC_REG_PCI_CFG_ADDR_ENABLE |
                     (bus << CYGARC_REG_PCI_CFG_ADDR_BUSNO_shift) |
                     (devfn << CYGARC_REG_PCI_CFG_ADDR_FUNC_shift) |
                     (offset & ~3));
    HAL_WRITE_UINT32(CYGARC_REG_PCI_CFG_DATA, config_dword);
    _DELAY();
    HAL_WRITE_UINT32(CYGARC_REG_PCI_CFG_CMD, CYGARC_REG_PCI_CFG_CMD_WCFG);
    _DELAY();
}

//--------------------------------------------------------------------------
// IO space accessor functions

#if 0 // Don't need these after all. But keep them around just in case...

static void
pci_io_delay(void)
{
    int i = 100;
    cyg_uint32 flg;
    do {
        HAL_READ_UINT32(CYGARC_REG_PCI_CFG_FLG, flg);
    } while (i-- && (flg & CYGARC_REG_PCI_CFG_FLG_ACTIVE));

    // FIXME: what happens on timeout? Do we need to fill in 0xfffffff
    // in read data, by any chance?
}
        
static void
pci_io_status(void)
{
    // FIXME: check status...
}


void
cyg_hal_plf_pci_io_write_byte (cyg_uint32 addr, cyg_uint8 data)
{
    cyg_uint32 io_addr = addr - HAL_PCI_PHYSICAL_IO_BASE;
    int shift = io_addr & 3;

    HAL_WRITE_UINT32(CYGARC_REG_PCI_CFG_DATA, ((cyg_uint32)data << (8*shift)));
    HAL_WRITE_UINT32(CYGARC_REG_PCI_CFG_ADDR, io_addr & ~3);
    HAL_WRITE_UINT32(CYGARC_REG_PCI_CFG_CMD, (CYGARC_REG_PCI_CFG_CMD_BE0 << shift)
                     | CYGARC_REG_PCI_CFG_CMD_CMDEN
                     | CYGARC_REG_PCI_CFG_CMD_IO_WRITE
                     | CYGARC_REG_PCI_CFG_CMD_WT);
    pci_io_delay();
}

void
cyg_hal_plf_pci_io_write_word (cyg_uint32 addr, cyg_uint16 data)
{
    cyg_uint32 io_addr = addr - HAL_PCI_PHYSICAL_IO_BASE;
    int shift = io_addr & 2;

    HAL_WRITE_UINT32(CYGARC_REG_PCI_CFG_DATA, ((cyg_uint32)data << (8*shift)));
    HAL_WRITE_UINT32(CYGARC_REG_PCI_CFG_ADDR, io_addr & ~3);
    HAL_WRITE_UINT32(CYGARC_REG_PCI_CFG_CMD, ((CYGARC_REG_PCI_CFG_CMD_BE1|CYGARC_REG_PCI_CFG_CMD_BE0) << shift)
                     | CYGARC_REG_PCI_CFG_CMD_CMDEN
                     | CYGARC_REG_PCI_CFG_CMD_IO_WRITE
                     | CYGARC_REG_PCI_CFG_CMD_WT);
    pci_io_delay();
}

void
cyg_hal_plf_pci_io_write_dword (cyg_uint32 addr, cyg_uint32 data)
{
    cyg_uint32 io_addr = addr - HAL_PCI_PHYSICAL_IO_BASE;

    HAL_WRITE_UINT32(CYGARC_REG_PCI_CFG_DATA, data);
    HAL_WRITE_UINT32(CYGARC_REG_PCI_CFG_ADDR, io_addr & ~3);
    HAL_WRITE_UINT32(CYGARC_REG_PCI_CFG_CMD, 
                     (CYGARC_REG_PCI_CFG_CMD_BE3|CYGARC_REG_PCI_CFG_CMD_BE2
                      | CYGARC_REG_PCI_CFG_CMD_BE1|CYGARC_REG_PCI_CFG_CMD_BE0)
                     | CYGARC_REG_PCI_CFG_CMD_CMDEN
                     | CYGARC_REG_PCI_CFG_CMD_IO_WRITE
                     | CYGARC_REG_PCI_CFG_CMD_WT);
    pci_io_delay();
}

cyg_uint8
cyg_hal_plf_pci_io_read_byte (cyg_uint32 addr)
{
    cyg_uint32 io_addr = addr - HAL_PCI_PHYSICAL_IO_BASE;
    cyg_uint32 data;
    int shift = io_addr & 3;

    HAL_WRITE_UINT32(CYGARC_REG_PCI_CFG_ADDR, io_addr & ~3);
    HAL_WRITE_UINT32(CYGARC_REG_PCI_CFG_CMD, (CYGARC_REG_PCI_CFG_CMD_BE0 << shift)
                     | CYGARC_REG_PCI_CFG_CMD_CMDEN
                     | CYGARC_REG_PCI_CFG_CMD_IO_READ
                     | CYGARC_REG_PCI_CFG_CMD_RD);
    pci_io_delay();
    HAL_READ_UINT32(CYGARC_REG_PCI_CFG_DATA, data);
    return (cyg_uint8)(0xff & (data >> (8*shift)));
}

cyg_uint16
cyg_hal_plf_pci_io_read_word (cyg_uint32 addr)
{
    cyg_uint32 io_addr = addr - HAL_PCI_PHYSICAL_IO_BASE;
    cyg_uint32 data;
    int shift = io_addr & 2;

    HAL_WRITE_UINT32(CYGARC_REG_PCI_CFG_ADDR, io_addr & ~3);
    HAL_WRITE_UINT32(CYGARC_REG_PCI_CFG_CMD, ((CYGARC_REG_PCI_CFG_CMD_BE1|CYGARC_REG_PCI_CFG_CMD_BE0) << shift)
                     | CYGARC_REG_PCI_CFG_CMD_CMDEN
                     | CYGARC_REG_PCI_CFG_CMD_IO_READ
                     | CYGARC_REG_PCI_CFG_CMD_RD);
    pci_io_delay();
    HAL_READ_UINT32(CYGARC_REG_PCI_CFG_DATA, data);
    return (cyg_uint16)(0xffff & (data >> (shift*8)));
}

cyg_uint32
cyg_hal_plf_pci_io_read_dword (cyg_uint32 addr)
{
    cyg_uint32 io_addr = addr - HAL_PCI_PHYSICAL_IO_BASE;
    cyg_uint32 data;

    HAL_WRITE_UINT32(CYGARC_REG_PCI_CFG_ADDR, io_addr & ~3);
    HAL_WRITE_UINT32(CYGARC_REG_PCI_CFG_CMD, 
                     (CYGARC_REG_PCI_CFG_CMD_BE3|CYGARC_REG_PCI_CFG_CMD_BE2
                      | CYGARC_REG_PCI_CFG_CMD_BE1|CYGARC_REG_PCI_CFG_CMD_BE0)
                     | CYGARC_REG_PCI_CFG_CMD_CMDEN
                     | CYGARC_REG_PCI_CFG_CMD_IO_READ
                     | CYGARC_REG_PCI_CFG_CMD_RD);
    pci_io_delay();
    HAL_READ_UINT32(CYGARC_REG_PCI_CFG_DATA, data);
    return data;
}
#endif

//--------------------------------------------------------------------------
// PCI interrupt decoding
static cyg_uint32
cyg_hal_plf_pci_arbiter(CYG_ADDRWORD vector, CYG_ADDRWORD data)
{
    cyg_uint32 isr_ret, int_sts;

    HAL_READ_UINT32(CYGARC_REG_SD0001_INT_STS1, int_sts);
    if (int_sts & CYGARC_REG_SD0001_INT_INTA) {
        isr_ret = hal_call_isr (CYGNUM_HAL_INTERRUPT_PCIA);
#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN
        if (isr_ret & CYG_ISR_HANDLED)
#endif
            return isr_ret;
    }
    if (int_sts & CYGARC_REG_SD0001_INT_INTB) {
        isr_ret = hal_call_isr (CYGNUM_HAL_INTERRUPT_PCIB);
#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN
        if (isr_ret & CYG_ISR_HANDLED)
#endif
            return isr_ret;
    }
    if (int_sts & CYGARC_REG_SD0001_INT_INTC) {
        isr_ret = hal_call_isr (CYGNUM_HAL_INTERRUPT_PCIC);
#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN
        if (isr_ret & CYG_ISR_HANDLED)
#endif
            return isr_ret;
    }
    if (int_sts & CYGARC_REG_SD0001_INT_INTD) {
        isr_ret = hal_call_isr (CYGNUM_HAL_INTERRUPT_PCID);
#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN
        if (isr_ret & CYG_ISR_HANDLED)
#endif
            return isr_ret;
    }

    return 0;
}


#endif // CYGPKG_IO_PCI
//--------------------------------------------------------------------------
// eof plf_misc.c
