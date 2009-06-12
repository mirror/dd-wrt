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
// Author(s):    hmt, jskov, nickg
// Contributors: hmt, jskov, nickg
// Date:         1999-08-09
// Purpose:      VRC4373 PCI IO support macros
// Description: 
// Usage:        #include <cyg/hal/plf_io.h>
//
// Note:         Based on information in 
//               "VRC4373 System Controller Data Sheet"
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>

#include <cyg/hal/hal_io.h>     // IO macros
#include <cyg/hal/hal_intr.h>   // Interrupt vectors

//-----------------------------------------------------------------------------
// PCI access registers

#define HAL_PCI_ADDRESS_WINDOW_1        0xAF000014
#define HAL_PCI_ADDRESS_WINDOW_2        0xAF000018
#define HAL_PCI_IO_WINDOW               0xAF000024
#define HAL_PCI_CONFIG_SPACE_DATA       0xAF000028
#define HAL_PCI_CONFIG_SPACE_ADDR       0xAF00002C
#define HAL_PCI_ENABLE_REG              0xAF000074

//-----------------------------------------------------------------------------
// Mappings for PCI memory and IO spaces

// This is where the PCI spaces are mapped in the CPU's (virtual)
// address space.
#define HAL_PCI_PHYSICAL_IO_BASE        0xA0000000
#define HAL_PCI_PHYSICAL_MEMORY_BASE    0xBC100000

//-----------------------------------------------------------------------------

// Initialize the PCI bus.
externC void cyg_hal_plf_pci_init(void);
#define HAL_PCI_INIT() cyg_hal_plf_pci_init()

// Compute address necessary to access PCI config space for the given
// bus and device.
#define HAL_PCI_CONFIG_ADDRESS( __bus, __devfn, __offset )               \
    ({                                                                   \
    cyg_uint32 __addr;                                                   \
    cyg_uint32 __dev = CYG_PCI_DEV_GET_DEV(__devfn);                     \
    if (0 == __bus) {                                                    \
            __addr = (1 << (__dev+16));                                  \
    } else {                                                             \
        /* We need better info about how to form type 1 addresses */     \
        __addr = 0x800000000 | (__bus << 16) | (__dev << 11);            \
    }                                                                    \
    __addr |= CYG_PCI_DEV_GET_FN(__devfn) << 8;                          \
    __addr |= (__offset)&~3;                                             \
    __addr;                                                              \
    })

// The following function allows us to read locations in the PCI config
// space that we are not sure contains a valid device. Support in platform.S
// catches and fixes any bus errors caused.
externC CYG_WORD32 hal_pci_config_read(CYG_ADDRESS addr);
#define HAL_PCI_CONFIG_READ( __addr, __val ) __val = hal_pci_config_read(__addr)

// Read a value from the PCI configuration space of the appropriate
// size at an address composed from the bus, devfn and offset.
// We can only make 32 bit accesses to the PCI config data register, hence
// all the shifing and masking in these macros.
// Note that we may only use HAL_PCI_CONFIG_READ() here.
#define HAL_PCI_CFG_READ_UINT8( __bus, __devfn, __offset, __val )       \
CYG_MACRO_START                                                         \
    CYG_WORD32 _val_;                                                   \
    CYG_WORD32 _offset_ = __offset & 3;                                 \
    HAL_WRITE_UINT32(HAL_PCI_CONFIG_SPACE_ADDR,                         \
                     HAL_PCI_CONFIG_ADDRESS(__bus, __devfn, __offset)); \
    HAL_PCI_CONFIG_READ(HAL_PCI_CONFIG_SPACE_DATA, _val_);              \
    __val = (CYG_BYTE)((_val_>>(_offset_*8))&0xFF);                     \
CYG_MACRO_END

#define HAL_PCI_CFG_READ_UINT16( __bus, __devfn, __offset, __val )      \
CYG_MACRO_START                                                         \
    CYG_WORD32 _val_;                                                   \
    CYG_WORD32 _offset_ = __offset & 2;                                 \
    HAL_WRITE_UINT32(HAL_PCI_CONFIG_SPACE_ADDR,                         \
                     HAL_PCI_CONFIG_ADDRESS(__bus, __devfn, __offset)); \
    HAL_PCI_CONFIG_READ(HAL_PCI_CONFIG_SPACE_DATA, _val_);              \
    __val = (CYG_WORD16)((_val_>>(_offset_*8))&0xFFFF);                 \
CYG_MACRO_END

#define HAL_PCI_CFG_READ_UINT32( __bus, __devfn, __offset, __val )      \
CYG_MACRO_START                                                         \
    CYG_WORD32 _val_;                                                   \
    HAL_WRITE_UINT32(HAL_PCI_CONFIG_SPACE_ADDR,                         \
                     HAL_PCI_CONFIG_ADDRESS(__bus, __devfn, __offset)); \
    HAL_PCI_CONFIG_READ(HAL_PCI_CONFIG_SPACE_DATA, _val_);              \
    __val = (CYG_WORD32)_val_;                                          \
CYG_MACRO_END

// Write a value to the PCI configuration space of the appropriate
// size at an address composed from the bus, devfn and offset.
// We can only make 32 bit accesses to the PCI config data register, hence
// all the shifing and masking in these macros.
// Note that we do not need to use HAL_PCI_CONFIG_READ() here since we
// should not be writing to config space locations that we do not already
// know are valid.
#define HAL_PCI_CFG_WRITE_UINT8( __bus, __devfn, __offset, __val )      \
CYG_MACRO_START                                                         \
    CYG_WORD32 _val_;                                                   \
    CYG_WORD32 _offset_ = __offset & 3;                                 \
    HAL_WRITE_UINT32(HAL_PCI_CONFIG_SPACE_ADDR,                         \
                     HAL_PCI_CONFIG_ADDRESS(__bus, __devfn, __offset)); \
    HAL_READ_UINT32(HAL_PCI_CONFIG_SPACE_DATA, _val_);                  \
    _val_ &= ~(0xFF<<(_offset_*8));                                     \
    _val_ |= (__val)<<(_offset_*8);                                     \
    HAL_WRITE_UINT32(HAL_PCI_CONFIG_SPACE_DATA, _val_);                 \
CYG_MACRO_END

#define HAL_PCI_CFG_WRITE_UINT16( __bus, __devfn, __offset, __val )     \
CYG_MACRO_START                                                         \
    CYG_WORD32 _val_;                                                   \
    CYG_WORD32 _offset_ = __offset & 2;                                 \
    HAL_WRITE_UINT32(HAL_PCI_CONFIG_SPACE_ADDR,                         \
                     HAL_PCI_CONFIG_ADDRESS(__bus, __devfn, __offset)); \
    HAL_READ_UINT32(HAL_PCI_CONFIG_SPACE_DATA, _val_);                  \
    _val_ &= ~(0xFFFF<<(_offset_*8));                                   \
    _val_ |= (__val)<<(_offset_*8);                                     \
    HAL_WRITE_UINT32(HAL_PCI_CONFIG_SPACE_DATA, _val_);                  \
CYG_MACRO_END

#define HAL_PCI_CFG_WRITE_UINT32( __bus, __devfn, __offset, __val )     \
CYG_MACRO_START                                                         \
    HAL_WRITE_UINT32(HAL_PCI_CONFIG_SPACE_ADDR,                         \
                     HAL_PCI_CONFIG_ADDRESS(__bus, __devfn, __offset)); \
    HAL_WRITE_UINT32(HAL_PCI_CONFIG_SPACE_DATA, __val);                 \
CYG_MACRO_END


//-----------------------------------------------------------------------------
// Resources

// Map PCI device resources starting from these addresses in PCI space.
#define HAL_PCI_ALLOC_BASE_MEMORY       0
#define HAL_PCI_ALLOC_BASE_IO           0x0c000000

// Translate the PCI interrupt requested by the device (INTA#, INTB#,
// INTC# or INTD#) to the associated CPU interrupt (i.e., HAL vector).
// We don't actually know what the mappings are at present for this
// board. The following is therefore just a temporary guess until
// we can find out.

#define HAL_PCI_TRANSLATE_INTERRUPT( __bus, __devfn, __vec, __valid)          \
    CYG_MACRO_START                                                           \
    cyg_uint8 __req;                                                          \
    HAL_PCI_CFG_READ_UINT8(__bus, __devfn, CYG_PCI_CFG_INT_PIN, __req);       \
    if (0 != __req) {                                                         \
        CYG_ADDRWORD __translation[4] = {                                     \
            CYGNUM_HAL_INTERRUPT_PCI_INTA,  /* INTA# */                       \
            CYGNUM_HAL_INTERRUPT_PCI_INTB,  /* INTB# */                       \
            CYGNUM_HAL_INTERRUPT_PCI_INTC,  /* INTC# */                       \
            CYGNUM_HAL_INTERRUPT_PCI_INTD };/* INTD# */                       \
                                                                              \
        __vec = __translation[(((__req-1)+CYG_PCI_DEV_GET_DEV(__devfn))&3)];  \
                                                                              \
        __valid = true;                                                       \
    } else {                                                                  \
        /* Device will not generate interrupt requests. */                    \
        __valid = false;                                                      \
    }                                                                         \
    CYG_MACRO_END

// Ignore all devices on not on bus 0 since those all seem to map to the
// Nile II controller itself.
#define HAL_PCI_IGNORE_DEVICE(__bus, __dev, __fn) \
    ((__bus) != 0)

//-----------------------------------------------------------------------------
// end of plf_io.h
#endif // CYGONCE_PLF_IO_H
