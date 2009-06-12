#ifndef CYGONCE_PLF_IO_H
#define CYGONCE_PLF_IO_H

//=============================================================================
//
//      plf_io.h
//
//      Platform specific IO support
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    msalter
// Contributors: msalter, gthomas
// Date:         2002-01-10
// Purpose:      Intel Xscale (PXA250) PCI IO support macros
// Description: 
// Usage:        #include <cyg/hal/plf_io.h>
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/uE250.h>

#define PCI_CONTROL_BASE 0x37040000

#define PCI_CTL_IO(x)      (*((volatile cyg_uint32*)(PCI_CONTROL_BASE+(x))))
#define PCI_CTL_IO_BYTE(x) (*((volatile cyg_uint8*)(PCI_CONTROL_BASE+(x))))

#define PCICTL_MEM_REMAP           PCI_CTL_IO(0x0000)
#define PCICTL_IO_REMAP            PCI_CTL_IO(0x0004)
#define PCICTL_CONFIG_REMAP        PCI_CTL_IO(0x0008)
#define PCICTL_INT_RESET           PCI_CTL_IO(0x0010)
#define PCICTL_STATUS_REG          PCI_CTL_IO_BYTE(0x0010)
#define PCICTL_INT_EDGE            PCI_CTL_IO_BYTE(0x0011)
#define PCICTL_IRQ_MASK            PCI_CTL_IO(0x0014)
#define PCICTL_MISC                PCI_CTL_IO(0x001C)

#define PCI_RESET          (1 << 0)
#define PCI_WRITEBUF       (1 << 1)
#define PCI_TIMER          (1 << 2)
#define PCI_IDSEL_OFF      (15 << 4)
#define PCI_IDSEL_0        (1 << 4)
#define PCI_IDSEL_1        (2 << 4)
#define PCI_IDSEL_2        (3 << 4)
#define PCI_IDSEL_3        (4 << 4)
#define PCI_IDSEL_4        (5 << 4)
#define PCI_IDSEL_5        (6 << 4)
#define PCI_SDRAM_64       (0 << 16)
#define PCI_SDRAM_128      (1 << 16)
#define PCI_SDRAM_256      (2 << 16)
#define PCI_SYSTEM_RESET   (1 << 31)

#define PCI_INT_A          (1 << 0)
#define PCI_INT_B          (1 << 1)
#define PCI_INT_C          (1 << 2)
#define PCI_INT_D          (1 << 3)
#define PCI_TARGET_ABORT   (1 << 4)
#define PCI_MASTER_ABORT   (1 << 5)
#define PCI_PARITY_ERROR   (1 << 6)
#define PCI_SYS_ERROR      (1 << 7)
#define PCI_BUS_TIMEOUT    (1 << 8)

#define PCI_INT_A_ENABLE   (1 << 0)
#define PCI_INT_B_ENABLE   (1 << 1)
#define PCI_INT_C_ENABLE   (1 << 2)
#define PCI_INT_D_ENABLE   (1 << 3)

#define PCI_IDSEL_SHIFT  4

// FIXME: Use virtual address
#define PCI_CONFIG_BASE  0x15000000

#define HAL_PCI_INIT() cyg_hal_plf_pci_init()


// Map PCI device resources starting from these addresses in PCI space.
#define HAL_PCI_PHYSICAL_MEMORY_BASE 0x0C000000     // CPU address
#define HAL_PCI_ALLOC_BASE_MEMORY    0x00000000     // PCI address
#define HAL_PCI_PHYSICAL_IO_BASE     0x16000000     // CPU address
#define HAL_PCI_ALLOC_BASE_IO        0x00000000     // PCI address
#define _PCI_READ_8   0x01000000                    // 
#define _PCI_READ_16  0x00000000                    // Byte address munging
#define _PCI_READ_32  0x00800000                    //
#define _PCI_WRITE_X  0x00000000

//-----------------------------------------------------------------------------

extern cyg_uint32 cyg_hal_plf_pci_cfg_read_dword (cyg_uint32 bus,
						  cyg_uint32 devfn,
						  cyg_uint32 offset);
extern cyg_uint16 cyg_hal_plf_pci_cfg_read_word  (cyg_uint32 bus,
						  cyg_uint32 devfn,
						  cyg_uint32 offset);
extern cyg_uint8 cyg_hal_plf_pci_cfg_read_byte   (cyg_uint32 bus,
						  cyg_uint32 devfn,
						  cyg_uint32 offset);
extern void cyg_hal_plf_pci_cfg_write_dword (cyg_uint32 bus,
					     cyg_uint32 devfn,
					     cyg_uint32 offset,
					     cyg_uint32 val);
extern void cyg_hal_plf_pci_cfg_write_word  (cyg_uint32 bus,
					     cyg_uint32 devfn,
					     cyg_uint32 offset,
					     cyg_uint16 val);
extern void cyg_hal_plf_pci_cfg_write_byte   (cyg_uint32 bus,
					      cyg_uint32 devfn,
					      cyg_uint32 offset,
					      cyg_uint8 val);

// Read a value from the PCI configuration space of the appropriate
// size at an address composed from the bus, devfn and offset.
#define HAL_PCI_CFG_READ_UINT8( __bus, __devfn, __offset, __val )  \
    __val = cyg_hal_plf_pci_cfg_read_byte((__bus),  (__devfn), (__offset))
    
#define HAL_PCI_CFG_READ_UINT16( __bus, __devfn, __offset, __val ) \
    __val = cyg_hal_plf_pci_cfg_read_word((__bus),  (__devfn), (__offset))

#define HAL_PCI_CFG_READ_UINT32( __bus, __devfn, __offset, __val ) \
    __val = cyg_hal_plf_pci_cfg_read_dword((__bus),  (__devfn), (__offset))

// Write a value to the PCI configuration space of the appropriate
// size at an address composed from the bus, devfn and offset.
#define HAL_PCI_CFG_WRITE_UINT8( __bus, __devfn, __offset, __val )  \
    cyg_hal_plf_pci_cfg_write_byte((__bus),  (__devfn), (__offset), (__val))

#define HAL_PCI_CFG_WRITE_UINT16( __bus, __devfn, __offset, __val ) \
    cyg_hal_plf_pci_cfg_write_word((__bus),  (__devfn), (__offset), (__val))

#define HAL_PCI_CFG_WRITE_UINT32( __bus, __devfn, __offset, __val ) \
    cyg_hal_plf_pci_cfg_write_dword((__bus),  (__devfn), (__offset), (__val))

  // Initialize the PCI bus.
externC void cyg_hal_plf_pci_init(void);
#define HAL_PCI_INIT() cyg_hal_plf_pci_init()

externC void _uE250_pci_translate_interrupt(int bus, int devfn, int *vector, int *valid);
#define HAL_PCI_TRANSLATE_INTERRUPT(__bus, __devfn, __vector, __valid) \
  _uE250_pci_translate_interrupt(__bus, __devfn, &__vector, &__valid)

// Special I/O access functions
externC cyg_uint8 pci_io_read_8(cyg_uint32 address);
externC cyg_uint16 pci_io_read_16(cyg_uint32 address);
externC cyg_uint32 pci_io_read_32(cyg_uint32 address);
externC void pci_io_write_8(cyg_uint32 address, cyg_uint8 value);
externC void pci_io_write_16(cyg_uint32 address, cyg_uint16 value);
externC void pci_io_write_32(cyg_uint32 address, cyg_uint32 value);

#define CYG_PCI_MAX_DEV 6
#define CYG_PCI_MAX_BUS 1
#define CYG_PCI_MAX_FN 1


#define HAL_IDE_NUM_CONTROLLERS 1  // Default card has two controllers - maybe should be 2?

#define HAL_IDE_READ_UINT8( __ctlr, __reg, __val) \
    __val = cyg_hal_plf_ide_read_uint8((__ctlr),  (__reg))

#define HAL_IDE_READ_UINT16( __ctlr, __reg, __val) \
    __val = cyg_hal_plf_ide_read_uint16((__ctlr),  (__reg))

#define HAL_IDE_WRITE_UINT8( __ctlr, __reg, __val) \
    cyg_hal_plf_ide_write_uint8((__ctlr),  (__reg), (__val))

#define HAL_IDE_WRITE_UINT16( __ctlr, __reg, __val) \
    cyg_hal_plf_ide_write_uint16((__ctlr),  (__reg), (__val))


#define HAL_IDE_WRITE_CONTROL( __ctlr, __val) \
    cyg_hal_plf_ide_write_control((__ctlr),  (__val))

#define HAL_IDE_INIT() cyg_hal_plf_ide_init()


// SDRAM is aliased as uncached memory for drivers.
#define CYGARC_UNCACHED_ADDRESS(_x_) \
  (((((unsigned long)(_x_)) >> 29)==0x0) ? (((unsigned long)(_x_))|0xc0000000) : (_x_))

static inline unsigned cygarc_physical_address(unsigned va)
{
    unsigned *ram_mmutab = (unsigned *)(SDRAM_BASE | 0x4000);
    unsigned pte;

    pte = ram_mmutab[va >> 20];

    return (pte & 0xfff00000) | (va & 0xfffff);
}

// FIXME
#undef CYGARC_PHYSICAL_ADDRESS
#define CYGARC_PHYSICAL_ADDRESS(_x_) cygarc_physical_address(_x_)

static inline unsigned cygarc_virtual_address(unsigned pa)
{
    if (0xa0000000 <= pa && pa < 0xc0000000)
	return pa - 0xa0000000;
    return pa;
}

#define CYGARC_VIRTUAL_ADDRESS(_x_) cygarc_virtual_address(_x_)
#define CYGARC_PCI_DMA_ADDRESS(_x_) (_x_)

//-----------------------------------------------------------------------------
// end of plf_io.h
#endif // CYGONCE_PLF_IO_H
