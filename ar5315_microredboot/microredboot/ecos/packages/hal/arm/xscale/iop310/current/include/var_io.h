#ifndef CYGONCE_HAL_ARM_VAR_IO_H
#define CYGONCE_HAL_ARM_VAR_IO_H

/*=============================================================================
//
//      var_io.h
//
//      Platform specific support (register layout, etc)
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002 Gary Thomas
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
// Author(s):    jskov
// Contributors: jskov, gthomas
// Date:         2002-01-28
// Purpose:      Platform specific support routines
// Description: 
// Usage:        #include <cyg/hal/hal_io.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/hal_arm_xscale_iop310.h>

#include <cyg/hal/hal_iop310.h>

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_intr.h>           // Interrupt vectors

#include <cyg/hal/plf_io.h>

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

/* primary PCI bus definitions */ 
#ifndef PRIMARY_MEM_BASE
#define PRIMARY_BUS_NUM		0
#define PRIMARY_MEM_BASE	0x80000000
#define PRIMARY_DAC_BASE	0x84000000
#define PRIMARY_IO_BASE		0x90000000
#define PRIMARY_MEM_LIMIT	0x83ffffff
#define PRIMARY_DAC_LIMIT	0x87ffffff
#define PRIMARY_IO_LIMIT	0x9000ffff
#endif

/* secondary PCI bus definitions */
#ifndef SECONDARY_MEM_BASE
#define	SECONDARY_BUS_NUM	1
#define SECONDARY_MEM_BASE	0x88000000
#define SECONDARY_DAC_BASE	0x8c000000
#define SECONDARY_IO_BASE	0x90010000
#define SECONDARY_MEM_LIMIT	0x8bffffff
#define SECONDARY_DAC_LIMIT	0x8fffffff
#define SECONDARY_IO_LIMIT	0x9001ffff
#endif

// Initialize the PCI bus.
externC void cyg_hal_plf_pci_init(void);
#define HAL_PCI_INIT() cyg_hal_plf_pci_init()

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

//-----------------------------------------------------------------------------
// Resources

// Map PCI device resources starting from these addresses in PCI space.
#ifndef HAL_PCI_ALLOC_BASE_MEMORY
#define HAL_PCI_ALLOC_BASE_MEMORY (SECONDARY_MEM_BASE)
#define HAL_PCI_ALLOC_BASE_IO     (SECONDARY_IO_BASE)
#endif

// This is where the PCI spaces are mapped in the CPU's address space.
#ifndef HAL_PCI_PHYSICAL_MEMORY_BASE
#define HAL_PCI_PHYSICAL_MEMORY_BASE    0x00000000
#define HAL_PCI_PHYSICAL_IO_BASE        0x00000000
#endif


// Some of SDRAM is aliased as uncached memory for drivers.
#ifndef CYGARC_UNCACHED_ADDRESS
#define CYGARC_UNCACHED_ADDRESS(_x_) \
  (((((unsigned long)(_x_)) >> 28)==0xA) ? (((unsigned long)(_x_))|0x40000000) : (unsigned long)(_x_))
#endif
#ifndef CYGARC_VIRT_TO_BUS
#define CYGARC_VIRT_TO_BUS(_x_) \
  (((((unsigned long)(_x_)) >> 28)==0xA) ? (unsigned long)(_x_) : (((unsigned long)(_x_))&~0x40000000))
#endif
#ifndef CYGARC_BUS_TO_VIRT
#define CYGARC_BUS_TO_VIRT(_x_) \
  (((((unsigned long)(_x_)) >> 28)==0xA) ? (((unsigned long)(_x_))|0x40000000) : (_x_))
#endif

#ifndef CYGARC_PHYSICAL_ADDRESS
#define CYGARC_PHYSICAL_ADDRESS(x) (x)
#endif

#endif // CYGONCE_HAL_ARM_VAR_IO_H
// EOF var_io.h
