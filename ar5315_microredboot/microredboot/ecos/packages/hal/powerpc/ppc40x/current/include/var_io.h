#ifndef CYGONCE_HAL_VAR_IO_H
#define CYGONCE_HAL_VAR_IO_H

//==========================================================================
//
//      var_io.h
//
//      PowerPC 40x variant I/O support
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002, 2003 Gary Thomas
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
// Contributors: jskov,gthomas
// Date:         2000-08-27
// Purpose:      Provide PPC40x I/O functions
// Description:  
// Usage:        Included via the architecture register header:
//               #include <cyg/hal/hal_io.h>
//               ...
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include CYGBLD_HAL_PLF_IO_H

#ifdef CYGPKG_IO_PCI
// These must be defined by the platform
#if !defined(CYG_PCI_MAX_BUS)
#error "Missing CYG_PCI_MAX_BUS platform definition"
#endif
#if !defined(CYG_PCI_MIN_DEV)
#error "Missing CYG_PCI_MIN_DEV platform definition"
#endif
#if !defined(CYG_PCI_MAX_DEV)
#error "Missing CYG_PCI_MAX_DEV platform definition"
#endif
#if !defined(HAL_PCI_ALLOC_BASE_MEMORY)
#error "Missing HAL_PCI_ALLOC_BASE_MEMORY platform definition"
#endif
#if !defined(HAL_PCI_ALLOC_BASE_IO)
#error "Missing HAL_PCI_ALLOC_BASE_IO platform definition"
#endif
#if !defined(HAL_PCI_PHYSICAL_MEMORY_BASE)
#error "Missing HAL_PCI_PHYSICAL_MEMORY_BASE platform definition"
#endif
#if !defined(HAL_PCI_PHYSICAL_IO_BASE)
#error "Missing HAL_PCI_PHYSICAL_IO_BASE platform definition"
#endif
//#if !defined(CYGMEM_SECTION_pci_window)
//#error "Missing CYGMEM_SECTION_pci_window platform definition"
//#endif
//#if !defined(CYGMEM_SECTION_pci_window_SIZE)
//#error "Missing CYGMEM_SECTION_pci_window_SIZE platform definition"
//#endif

// Initialize the PCI environment
externC void hal_ppc405_pci_init(void);
#define HAL_PCI_INIT() \
  hal_ppc405_pci_init()

// Translate the PCI interrupt requested by the device (INTA#, INTB#,
// INTC# or INTD#) to the associated CPU interrupt (i.e., HAL vector).
externC void hal_ppc405_pci_translate_interrupt(int bus, int devfn, int *vec, int *valid);
#define HAL_PCI_TRANSLATE_INTERRUPT( __bus, __devfn, __vec, __valid) \
  hal_ppc405_pci_translate_interrupt(__bus, __devfn, &__vec, &__valid)

// Read a value from the PCI configuration space of the appropriate
// size at an address composed from the bus, devfn and offset.
externC cyg_uint8 hal_ppc405_pci_cfg_read_uint8(int bus, int dev, int offset);
#define HAL_PCI_CFG_READ_UINT8( __bus, __devfn, __offset, __val )  \
  __val = hal_ppc405_pci_cfg_read_uint8(__bus, __devfn, __offset)
    
externC cyg_uint16 hal_ppc405_pci_cfg_read_uint16(int bus, int dev, int offset);
#define HAL_PCI_CFG_READ_UINT16( __bus, __devfn, __offset, __val )  \
  __val = hal_ppc405_pci_cfg_read_uint16(__bus, __devfn, __offset)
    
externC cyg_uint32 hal_ppc405_pci_cfg_read_uint32(int bus, int dev, int offset);
#define HAL_PCI_CFG_READ_UINT32( __bus, __devfn, __offset, __val )  \
  __val = hal_ppc405_pci_cfg_read_uint32(__bus, __devfn, __offset)

// Write a value to the PCI configuration space of the appropriate
// size at an address composed from the bus, devfn and offset.
externC void hal_ppc405_pci_cfg_write_uint8(int bus, int dev, int offset, cyg_uint8 val);
#define HAL_PCI_CFG_WRITE_UINT8( __bus, __devfn, __offset, __val ) \
  hal_ppc405_pci_cfg_write_uint8(__bus, __devfn, __offset, __val)

externC void hal_ppc405_pci_cfg_write_uint16(int bus, int dev, int offset, cyg_uint16 val);
#define HAL_PCI_CFG_WRITE_UINT16( __bus, __devfn, __offset, __val ) \
  hal_ppc405_pci_cfg_write_uint16(__bus, __devfn, __offset, __val)

externC void hal_ppc405_pci_cfg_write_uint32(int bus, int dev, int offset, cyg_uint32 val);
#define HAL_PCI_CFG_WRITE_UINT32( __bus, __devfn, __offset, __val ) \
  hal_ppc405_pci_cfg_write_uint32(__bus, __devfn, __offset, __val)
#endif // CYGPKG_IO_PCI

static __inline__ unsigned long
_le32(unsigned long val)
{
    return (((val & 0x000000FF) << 24) |
            ((val & 0x0000FF00) <<  8) |
            ((val & 0x00FF0000) >>  8) |
            ((val & 0xFF000000) >> 24));
}

static __inline__ unsigned short
_le16(unsigned short val)
{
    return (((val & 0x000000FF) << 8) |
            ((val & 0x0000FF00) >> 8));
}

#define HAL_WRITE_UINT32LE(_addr_, _val_) \
  HAL_WRITE_UINT32(_addr_, _le32(_val_))
#define HAL_WRITE_UINT16LE(_addr_, _val_) \
  HAL_WRITE_UINT16(_addr_, _le16(_val_))
#define HAL_WRITE_UINT8LE(_addr_, _val_) \
  HAL_WRITE_UINT8(_addr_, _val_)
#define HAL_READ_UINT32LE(_addr_, _val_)        \
  {                                             \
      HAL_READ_UINT32(_addr_, _val_);           \
      _val_ = _le32(_val_);                     \
  }
#define HAL_READ_UINT16LE(_addr_, _val_)        \
  {                                             \
      HAL_READ_UINT16(_addr_, _val_);           \
      _val_ = _le16(_val_);                     \
  }
#define HAL_READ_UINT8LE(_addr_, _val_)        \
  HAL_READ_UINT8(_addr_, _val_)

//-----------------------------------------------------------------------------
// Additional functions exported by HAL (no good place to define them!)
//
#if !defined(__ASSEMBLER__)
#if defined(CYGHWR_HAL_POWERPC_PPC4XX_405) || defined(CYGHWR_HAL_POWERPC_PPC4XX_405GP)
externC bool hal_ppc405_i2c_put_bytes(int addr, cyg_uint8 *val, int len);
externC bool hal_ppc405_i2c_get_bytes(int addr, cyg_uint8 *val, int len);
#endif
#endif

//-----------------------------------------------------------------------------
#endif // ifdef CYGONCE_HAL_VAR_IO_H
// End of var_io.h
