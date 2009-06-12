#ifndef CYGONCE_HAL_ARM_IXP425_VAR_IO_H
#define CYGONCE_HAL_ARM_IXP425_VAR_IO_H

/*=============================================================================
//
//      var_io.h
//
//      Platform specific support.
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004 Red Hat, Inc.
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
// Contributors: msalter
// Date:         2002-12-12
// Purpose:      IXP425 specific support routines
// Description: 
// Usage:        #include <cyg/hal/hal_io.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <cyg/hal/plf_io.h>

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

extern void cyg_hal_plf_pci_io_outb(cyg_uint32 offset, cyg_uint8 value);
extern void cyg_hal_plf_pci_io_outw(cyg_uint32 offset, cyg_uint16 value);
extern void cyg_hal_plf_pci_io_outl(cyg_uint32 offset, cyg_uint32 value);
extern cyg_uint8  cyg_hal_plf_pci_io_inb(cyg_uint32 offset);
extern cyg_uint16 cyg_hal_plf_pci_io_inw(cyg_uint32 offset);
extern cyg_uint32 cyg_hal_plf_pci_io_inl(cyg_uint32 offset);

extern void cyg_hal_plf_pci_init(void);
#define HAL_PCI_INIT() cyg_hal_plf_pci_init()


// Translate the PCI interrupt requested by the device (INTA#, INTB#,
// INTC# or INTD#) to the associated CPU interrupt (i.e., HAL vector).
extern void cyg_hal_plf_pci_translate_interrupt(cyg_uint32 bus, cyg_uint32 devfn,
						CYG_ADDRWORD *vec, cyg_bool *valid);
#define HAL_PCI_TRANSLATE_INTERRUPT( __bus, __devfn, __vec, __valid)\
    cyg_hal_plf_pci_translate_interrupt(__bus, __devfn, &(__vec), &(__valid))


#ifdef CYGPKG_IO_PCI

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
#else

#define HAL_PCI_CFG_READ_UINT8( __bus, __devfn, __offset, __val )    
#define HAL_PCI_CFG_READ_UINT16( __bus, __devfn, __offset, __val )
#define HAL_PCI_CFG_READ_UINT32( __bus, __devfn, __offset, __val )
#define HAL_PCI_CFG_WRITE_UINT8( __bus, __devfn, __offset, __val )
#define HAL_PCI_CFG_WRITE_UINT16( __bus, __devfn, __offset, __val )
#define HAL_PCI_CFG_WRITE_UINT32( __bus, __devfn, __offset, __val )
#endif


// Default memory mapping details
#ifndef CYGARC_PHYSICAL_ADDRESS
#define CYGARC_PHYSICAL_ADDRESS(x) (x)
#endif

#ifndef CYGARC_UNCACHED_ADDRESS
#define CYGARC_UNCACHED_ADDRESS(x) \
  ((((cyg_uint32)(x)) < 0x10000000) ? (((cyg_uint32)(x))+0x20000000) : (x))
#endif

#ifndef CYGARC_VIRT_TO_BUS
// direct mapped at uncached address
#define CYGARC_VIRT_TO_BUS(x) (((cyg_uint32)(x) & 0xfffffff))
#endif

#ifndef CYGARC_BUS_TO_VIRT
// direct mapped
#define CYGARC_BUS_TO_VIRT(x) (x)
#endif

#define HAL_PCI_ALLOC_BASE_MEMORY    0x48000000
#define HAL_PCI_ALLOC_BASE_IO        0

#define HAL_PCI_PHYSICAL_MEMORY_BASE 0
#define HAL_PCI_PHYSICAL_IO_BASE     0

//-----------------------------------------------------------------------------
// HAL IO macros.

// Macro used to determine if given address corresponds to PCI IO address
// space. Evaluates to non-zero if address is for PCI IO.
#ifdef CYGPKG_IO_PCI
#define IXDP425_IS_PCI_IO(x) (((cyg_uint32)(x))<0x40000000)
#else
#define IXDP425_IS_PCI_IO(x) 0
#endif

//-----------------------------------------------------------------------------
// BYTE Register access.

#define HAL_READ_UINT8( _reg_, _value_ )                           \
    CYG_MACRO_START                                                \
    if (IXDP425_IS_PCI_IO(_reg_))                                  \
        (_value_) = cyg_hal_plf_pci_io_inb((cyg_uint32)(_reg_));   \
    else                                                           \
        ((_value_) = *((volatile CYG_BYTE *)(_reg_)));             \
    CYG_MACRO_END

#define HAL_WRITE_UINT8( _reg_, _value_ )                          \
    CYG_MACRO_START                                                \
    if (IXDP425_IS_PCI_IO(_reg_))                                  \
        cyg_hal_plf_pci_io_outb((cyg_uint32)(_reg_), _value_);     \
    else                                                           \
        (*((volatile CYG_BYTE *)(_reg_)) = (_value_));             \
    CYG_MACRO_END

//-----------------------------------------------------------------------------
// 16 bit access.
    
#define HAL_READ_UINT16( _reg_, _value_ )                          \
    CYG_MACRO_START                                                \
    if (IXDP425_IS_PCI_IO(_reg_))                                  \
        (_value_) = cyg_hal_plf_pci_io_inw((cyg_uint32)(_reg_));   \
    else                                                           \
        ((_value_) = *((volatile CYG_WORD16 *)(_reg_)));           \
    CYG_MACRO_END

#define HAL_WRITE_UINT16( _reg_, _value_ )                         \
    CYG_MACRO_START                                                \
    if (IXDP425_IS_PCI_IO(_reg_))                                  \
        cyg_hal_plf_pci_io_outw((cyg_uint32)(_reg_), _value_);     \
    else                                                           \
        (*((volatile CYG_WORD16 *)(_reg_)) = (_value_));           \
    CYG_MACRO_END

//-----------------------------------------------------------------------------
// 32 bit access.

#define HAL_READ_UINT32( _reg_, _value_ )                          \
    CYG_MACRO_START                                                \
    if (IXDP425_IS_PCI_IO(_reg_))                                  \
        (_value_) = cyg_hal_plf_pci_io_inl((cyg_uint32)(_reg_));   \
    else                                                           \
        ((_value_) = *((volatile CYG_WORD32 *)(_reg_)));           \
    CYG_MACRO_END

#define HAL_WRITE_UINT32( _reg_, _value_ )                         \
    CYG_MACRO_START                                                \
    if (IXDP425_IS_PCI_IO(_reg_))                                  \
        cyg_hal_plf_pci_io_outl((cyg_uint32)(_reg_), _value_);     \
    else                                                           \
        (*((volatile CYG_WORD32 *)(_reg_)) = (_value_));           \
    CYG_MACRO_END

#define HAL_IO_MACROS_DEFINED

//-----------------------------------------------------------------------------
// HAL flash functions.
// These perform special operations between RedBoot and the flash driver.

externC int hal_flash_read(void *addr, void *data, int len, void **err);
externC int hal_flash_program(void *addr, void *data, int len, void **err);

#endif // CYGONCE_HAL_ARM_IXP425_VAR_IO_H
// EOF var_io.h
