//==========================================================================
//
//      pcic.c
//
//      HAL PCI controller support
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
// Date:         2001-07-10
// Purpose:      Support for SH PCIC module
//
//####DESCRIPTIONEND####
//
//===========================================================================

#include <pkgconf/hal.h>

#include <cyg/infra/diag.h>             // diag_printf
#include <cyg/hal/plf_io.h>             // PCI definitions
#include <cyg/hal/hal_arch.h>           // HAL header
#include <cyg/hal/hal_intr.h>           // HAL interrupts/exceptions
#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_if.h>
#include <cyg/io/pci_hw.h>
#include <cyg/io/pci.h>

void
cyg_hal_sh_pcic_pci_init(void)
{
    cyg_uint8  next_bus;
    cyg_uint32 tmp;

    static int initialized = 0;
    if (initialized) return;
    initialized = 1;

    // PCI bus/wait state configs must match those used in the BSC.
    HAL_READ_UINT32(CYGARC_REG_BCR1, tmp);
    tmp |= 0x40000000;
    HAL_WRITE_UINT32(CYGARC_REG_PCIC_BCR1, tmp);
    HAL_READ_UINT16(CYGARC_REG_BCR2, tmp);
    HAL_WRITE_UINT32(CYGARC_REG_PCIC_BCR2, tmp);
    HAL_READ_UINT32(CYGARC_REG_WCR1, tmp);
    HAL_WRITE_UINT32(CYGARC_REG_PCIC_WCR1, tmp);
    HAL_READ_UINT32(CYGARC_REG_WCR2, tmp);
    HAL_WRITE_UINT32(CYGARC_REG_PCIC_WCR2, tmp);
    HAL_READ_UINT32(CYGARC_REG_WCR3, tmp);
    HAL_WRITE_UINT32(CYGARC_REG_PCIC_WCR3, tmp);
    HAL_READ_UINT32(CYGARC_REG_MCR, tmp);
    tmp &= ~(CYGARC_REG_MCR_MRSET | CYGARC_REG_MCR_RFSH);
    HAL_WRITE_UINT32(CYGARC_REG_PCIC_MCR, tmp);

    // Unmask all PCI related interrupts
    HAL_WRITE_UINT32(CYGARC_REG_PCIC_INTM, CYGARC_REG_PCIC_INTM_INIT);
    HAL_WRITE_UINT32(CYGARC_REG_PCIC_AINTM, CYGARC_REG_PCIC_AINTM_INIT);

    // Set host PCI config using platform specified parameters.
    HAL_WRITE_UINT32(CYGARC_REG_PCIC_CFG+CYG_PCI_CFG_COMMAND, 0xfb9000c7);
    HAL_WRITE_UINT32(CYGARC_REG_PCIC_CFG+CYG_PCI_CFG_CLASS_REV, 0x00000000);
    HAL_WRITE_UINT32(CYGARC_REG_PCIC_CFG+CYG_PCI_CFG_CACHE_LINE_SIZE, 64<<8);
    HAL_WRITE_UINT32(CYGARC_REG_PCIC_CFG+CYG_PCI_CFG_BAR_0, CYGARC_REG_PCIC_BAR0_PLF_INIT);
    HAL_WRITE_UINT32(CYGARC_REG_PCIC_CFG+CYG_PCI_CFG_BAR_1, CYGARC_REG_PCIC_BAR1_PLF_INIT);
    HAL_WRITE_UINT32(CYGARC_REG_PCIC_CFG+CYG_PCI_CFG_BAR_2, CYGARC_REG_PCIC_BAR2_PLF_INIT);
    HAL_WRITE_UINT32(CYGARC_REG_PCIC_CFG+CYG_PCI_CFG_SUB_VENDOR, 0x35051054);
    HAL_WRITE_UINT32(CYGARC_REG_PCIC_LSR0, CYGARC_REG_PCIC_LSR0_PLF_INIT);
    HAL_WRITE_UINT32(CYGARC_REG_PCIC_LSR1, CYGARC_REG_PCIC_LSR1_PLF_INIT);
    HAL_WRITE_UINT32(CYGARC_REG_PCIC_LAR0, CYGARC_REG_PCIC_LAR0_PLF_INIT);
    HAL_WRITE_UINT32(CYGARC_REG_PCIC_LAR1, CYGARC_REG_PCIC_LAR1_PLF_INIT);
    HAL_WRITE_UINT32(CYGARC_REG_PCIC_CR, CYGARC_REG_PCIC_CR_INIT);

    // Configure PCI bus.
    next_bus = 1;
    cyg_pci_configure_bus(0, &next_bus);
}

//--------------------------------------------------------------------------
// Config space accessor functions
cyg_uint32
cyg_hal_sh_pcic_pci_cfg_read_dword (cyg_uint32 bus, cyg_uint32 devfn,
                                cyg_uint32 offset)
{
    cyg_uint32 config_data;

    HAL_WRITE_UINT32(CYGARC_REG_PCIC_PAR,
                     CYGARC_REG_PCIC_PAR_ENABLE |
                     (bus << CYGARC_REG_PCIC_PAR_BUSNO_shift) |
                     (devfn << CYGARC_REG_PCIC_PAR_FUNC_shift) |
                     (offset));
    HAL_READ_UINT32(CYGARC_REG_PCIC_PDR, config_data);

    return config_data;
}

cyg_uint16
cyg_hal_sh_pcic_pci_cfg_read_word (cyg_uint32 bus, cyg_uint32 devfn,
                               cyg_uint32 offset)
{
    cyg_uint32 config_dword;

    HAL_WRITE_UINT32(CYGARC_REG_PCIC_PAR,
                     CYGARC_REG_PCIC_PAR_ENABLE |
                     (bus << CYGARC_REG_PCIC_PAR_BUSNO_shift) |
                     (devfn << CYGARC_REG_PCIC_PAR_FUNC_shift) |
                     (offset & ~3));
    HAL_READ_UINT32(CYGARC_REG_PCIC_PDR, config_dword);

    return (cyg_uint16)((config_dword >> ((offset & 3) * 8)) & 0xffff);
}

cyg_uint8
cyg_hal_sh_pcic_pci_cfg_read_byte (cyg_uint32 bus, cyg_uint32 devfn, 
                               cyg_uint32 offset)
{
    cyg_uint32 config_dword;

    HAL_WRITE_UINT32(CYGARC_REG_PCIC_PAR,
                     CYGARC_REG_PCIC_PAR_ENABLE |
                     (bus << CYGARC_REG_PCIC_PAR_BUSNO_shift) |
                     (devfn << CYGARC_REG_PCIC_PAR_FUNC_shift) |
                     (offset & ~3));
    HAL_READ_UINT32(CYGARC_REG_PCIC_PDR, config_dword);

    return (cyg_uint8)((config_dword >> ((offset & 3) * 8)) & 0xff);
}

void
cyg_hal_sh_pcic_pci_cfg_write_dword (cyg_uint32 bus, cyg_uint32 devfn,
                                 cyg_uint32 offset, cyg_uint32 data)
{
    HAL_WRITE_UINT32(CYGARC_REG_PCIC_PAR,
                     CYGARC_REG_PCIC_PAR_ENABLE |
                     (bus << CYGARC_REG_PCIC_PAR_BUSNO_shift) |
                     (devfn << CYGARC_REG_PCIC_PAR_FUNC_shift) |
                     (offset));
    HAL_WRITE_UINT32(CYGARC_REG_PCIC_PDR, data);
}

void
cyg_hal_sh_pcic_pci_cfg_write_word (cyg_uint32 bus, cyg_uint32 devfn,
                                cyg_uint32 offset, cyg_uint16 data)
{
    cyg_uint32 config_dword, shift;

    HAL_WRITE_UINT32(CYGARC_REG_PCIC_PAR,
                     CYGARC_REG_PCIC_PAR_ENABLE |
                     (bus << CYGARC_REG_PCIC_PAR_BUSNO_shift) |
                     (devfn << CYGARC_REG_PCIC_PAR_FUNC_shift) |
                     (offset & ~3));
    HAL_READ_UINT32(CYGARC_REG_PCIC_PDR, config_dword);

    shift = (offset & 3) * 8;
    config_dword &= ~(0xffff << shift);
    config_dword |= (data << shift);

    HAL_WRITE_UINT32(CYGARC_REG_PCIC_PAR,
                     CYGARC_REG_PCIC_PAR_ENABLE |
                     (bus << CYGARC_REG_PCIC_PAR_BUSNO_shift) |
                     (devfn << CYGARC_REG_PCIC_PAR_FUNC_shift) |
                     (offset & ~3));
    HAL_WRITE_UINT32(CYGARC_REG_PCIC_PDR, config_dword);
}

void 
cyg_hal_sh_pcic_pci_cfg_write_byte (cyg_uint32 bus, cyg_uint32 devfn,
                                cyg_uint32 offset, cyg_uint8  data)
{
    cyg_uint32 config_dword, shift;

    HAL_WRITE_UINT32(CYGARC_REG_PCIC_PAR,
                     CYGARC_REG_PCIC_PAR_ENABLE |
                     (bus << CYGARC_REG_PCIC_PAR_BUSNO_shift) |
                     (devfn << CYGARC_REG_PCIC_PAR_FUNC_shift) |
                     (offset & ~3));
    HAL_READ_UINT32(CYGARC_REG_PCIC_PDR, config_dword);

    shift = (offset & 3) * 8;
    config_dword &= ~(0xff << shift);
    config_dword |= (data << shift);

    HAL_WRITE_UINT32(CYGARC_REG_PCIC_PAR,
                     CYGARC_REG_PCIC_PAR_ENABLE |
                     (bus << CYGARC_REG_PCIC_PAR_BUSNO_shift) |
                     (devfn << CYGARC_REG_PCIC_PAR_FUNC_shift) |
                     (offset & ~3));
    HAL_WRITE_UINT32(CYGARC_REG_PCIC_PDR, config_dword);
}

//--------------------------------------------------------------------------
// IO space accessor functions

void
cyg_hal_sh_pcic_pci_io_write_byte (cyg_uint32 addr, cyg_uint8 data)
{
    HAL_WRITE_UINT32(CYGARC_REG_PCIC_IOBR, addr & CYGARC_REG_PCIC_IOBR_MASK);
    HAL_WRITE_UINT8(CYGARC_REG_PCIC_IO_BASE + (addr & CYGARC_REG_PCIC_IO_BASE_MASK),
                    data);
}

void
cyg_hal_sh_pcic_pci_io_write_word (cyg_uint32 addr, cyg_uint16 data)
{
    HAL_WRITE_UINT32(CYGARC_REG_PCIC_IOBR, addr & CYGARC_REG_PCIC_IOBR_MASK);
    HAL_WRITE_UINT16(CYGARC_REG_PCIC_IO_BASE + (addr & CYGARC_REG_PCIC_IO_BASE_MASK),
                     data);
}

void
cyg_hal_sh_pcic_pci_io_write_dword (cyg_uint32 addr, cyg_uint32 data)
{
    HAL_WRITE_UINT32(CYGARC_REG_PCIC_IOBR, addr & CYGARC_REG_PCIC_IOBR_MASK);
    HAL_WRITE_UINT32(CYGARC_REG_PCIC_IO_BASE + (addr & CYGARC_REG_PCIC_IO_BASE_MASK),
                     data);
}

cyg_uint8
cyg_hal_sh_pcic_pci_io_read_byte (cyg_uint32 addr)
{
    cyg_uint8 data;

    HAL_WRITE_UINT32(CYGARC_REG_PCIC_IOBR, addr & CYGARC_REG_PCIC_IOBR_MASK);
    HAL_READ_UINT8(CYGARC_REG_PCIC_IO_BASE + (addr & CYGARC_REG_PCIC_IO_BASE_MASK),
                   data);
    return data;
}

cyg_uint16
cyg_hal_sh_pcic_pci_io_read_word (cyg_uint32 addr)
{
    cyg_uint16 data;

    HAL_WRITE_UINT32(CYGARC_REG_PCIC_IOBR, addr & CYGARC_REG_PCIC_IOBR_MASK);
    HAL_READ_UINT16(CYGARC_REG_PCIC_IO_BASE + (addr & CYGARC_REG_PCIC_IO_BASE_MASK),
                    data);
    return data;
}

cyg_uint32
cyg_hal_sh_pcic_pci_io_read_dword (cyg_uint32 addr)
{
    cyg_uint32 data;

    HAL_WRITE_UINT32(CYGARC_REG_PCIC_IOBR, addr & CYGARC_REG_PCIC_IOBR_MASK);
    HAL_READ_UINT32(CYGARC_REG_PCIC_IO_BASE + (addr & CYGARC_REG_PCIC_IO_BASE_MASK),
                    data);
    return data;
}
