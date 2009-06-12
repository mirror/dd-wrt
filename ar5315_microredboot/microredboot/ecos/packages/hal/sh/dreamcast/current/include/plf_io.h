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
// Author(s):    t@keshi.org
// Contributors: t@keshi.org
// Date:         2001-07-30
// Purpose:      Dreamcast PCI IO support macros
// Description: 
// Usage:        #include <cyg/hal/plf_io.h>
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_intr.h>

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


#define HAL_PCI_TRANSLATE_INTERRUPT( __bus, __devfn, __vec, __valid) \
	CYG_MACRO_START \
	__valid = true; \
	__vec = CYGNUM_HAL_INTERRUPT_LVL11; \
	CYG_MACRO_END

#define HAL_PCI_ALLOC_BASE_MEMORY 0
#define HAL_PCI_ALLOC_BASE_IO     0

#define HAL_PCI_PHYSICAL_MEMORY_BASE    0x00000000
#define HAL_PCI_PHYSICAL_IO_BASE        0x00000000

#define CYGARC_PCI_DMA_ADDRESS(_x_) ((_x_) & 0x1fffffff)

//-----------------------------------------------------------------------------
#endif // ifndef CYGONCE_PLF_IO_H
// End of plf_io.h
