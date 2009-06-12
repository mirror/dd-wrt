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
// Author(s):    hmt, jskov 
// Contributors: hmt, jskov, gthomas
// Date:         1999-08-09
// Purpose:      Fujitsu FR-V400 PCI IO support macros
// Description: 
// Usage:        #include <cyg/hal/plf_io.h>
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include CYGBLD_HAL_PLATFORM_H
#include CYGBLD_HAL_PLF_DEFS_H

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/plf_ints.h>           // Interrupt vectors

// Restrict device [slot] space
#define CYG_PCI_MAX_BUS                       1  // Only one BUS
#define CYG_PCI_MIN_DEV                      16  // Slots start at 16
#define CYG_PCI_MAX_DEV                      21  // ... and end at 20

//-----------------------------------------------------------------------------
// Resources

// Map PCI device resources starting from these addresses in PCI space.
#define HAL_PCI_ALLOC_BASE_MEMORY                 0x28000000
#define HAL_PCI_ALLOC_BASE_IO                     0x24000000

// This is where the PCI spaces are mapped in the CPU's address space.
#define HAL_PCI_PHYSICAL_MEMORY_BASE              0 // 0x28000000
#define HAL_PCI_PHYSICAL_IO_BASE                  0 // 0x24000000

// These seem to be defined multiple ways?
#define CYGMEM_SECTION_pci_window                 0x03F00000
#define CYGMEM_SECTION_pci_window_SIZE            0x00100000
#define CYGHWR_HAL_FRV_FRV400_PCI_MEM_MAP_BASE    0x03F00000
#define CYGHWR_HAL_FRV_FRV400_PCI_MEM_MAP_SIZE    0x00100000

// Initialize the PCI environment
externC void _frv400_pci_init(void);
#define HAL_PCI_INIT() \
  _frv400_pci_init()

// Translate the PCI interrupt requested by the device (INTA#, INTB#,
// INTC# or INTD#) to the associated CPU interrupt (i.e., HAL vector).
externC void _frv400_pci_translate_interrupt(int bus, int devfn, int *vec, int *valid);
#define HAL_PCI_TRANSLATE_INTERRUPT( __bus, __devfn, __vec, __valid) \
  _frv400_pci_translate_interrupt(__bus, __devfn, &__vec, &__valid)

// Read a value from the PCI configuration space of the appropriate
// size at an address composed from the bus, devfn and offset.
externC cyg_uint8 _frv400_pci_cfg_read_uint8(int bus, int dev, int offset);
#define HAL_PCI_CFG_READ_UINT8( __bus, __devfn, __offset, __val )  \
  __val = _frv400_pci_cfg_read_uint8(__bus, __devfn, __offset)
    
externC cyg_uint16 _frv400_pci_cfg_read_uint16(int bus, int dev, int offset);
#define HAL_PCI_CFG_READ_UINT16( __bus, __devfn, __offset, __val )  \
  __val = _frv400_pci_cfg_read_uint16(__bus, __devfn, __offset)
    
externC cyg_uint32 _frv400_pci_cfg_read_uint32(int bus, int dev, int offset);
#define HAL_PCI_CFG_READ_UINT32( __bus, __devfn, __offset, __val )  \
  __val = _frv400_pci_cfg_read_uint32(__bus, __devfn, __offset)

// Write a value to the PCI configuration space of the appropriate
// size at an address composed from the bus, devfn and offset.
externC void _frv400_pci_cfg_write_uint8(int bus, int dev, int offset, cyg_uint8 val);
#define HAL_PCI_CFG_WRITE_UINT8( __bus, __devfn, __offset, __val ) \
  _frv400_pci_cfg_write_uint8(__bus, __devfn, __offset, __val)

externC void _frv400_pci_cfg_write_uint16(int bus, int dev, int offset, cyg_uint16 val);
#define HAL_PCI_CFG_WRITE_UINT16( __bus, __devfn, __offset, __val ) \
  _frv400_pci_cfg_write_uint16(__bus, __devfn, __offset, __val)

externC void _frv400_pci_cfg_write_uint32(int bus, int dev, int offset, cyg_uint32 val);
#define HAL_PCI_CFG_WRITE_UINT32( __bus, __devfn, __offset, __val ) \
  _frv400_pci_cfg_write_uint32(__bus, __devfn, __offset, __val)

//-----------------------------------------------------------------------------
// end of plf_io.h
#endif // CYGONCE_PLF_IO_H
