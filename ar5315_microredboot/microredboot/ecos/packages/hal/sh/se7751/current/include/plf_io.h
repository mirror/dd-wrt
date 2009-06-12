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
// Author(s):    jskov 
// Contributors: jskov
// Date:         2001-07-10
// Purpose:      SE7751 IO support macros
// Description: 
// Usage:        #include <cyg/hal/plf_io.h>
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <cyg/hal/hal_intr.h>
#include <cyg/hal/mod_regs_pcic.h>

//-----------------------------------------------------------------------------
// PCI host setup details - map full 64MB in MEM1
#define CYGARC_REG_PCIC_BAR0_PLF_INIT 0xab000001
#define CYGARC_REG_PCIC_BAR1_PLF_INIT 0x0c000000
#define CYGARC_REG_PCIC_BAR2_PLF_INIT 0xd0000000
#define CYGARC_REG_PCIC_LSR0_PLF_INIT 0x03f00000
#define CYGARC_REG_PCIC_LSR1_PLF_INIT 0x00000000
#define CYGARC_REG_PCIC_LAR0_PLF_INIT 0x0c000000
#define CYGARC_REG_PCIC_LAR1_PLF_INIT 0x00000000


extern cyg_uint32 cyg_hal_sh_pcic_pci_cfg_read_dword (cyg_uint32 bus, cyg_uint32 devfn,
                                                  cyg_uint32 offset);
extern cyg_uint16 cyg_hal_sh_pcic_pci_cfg_read_word  (cyg_uint32 bus, cyg_uint32 devfn,
                                                  cyg_uint32 offset);
extern cyg_uint8 cyg_hal_sh_pcic_pci_cfg_read_byte   (cyg_uint32 bus, cyg_uint32 devfn,
                                                  cyg_uint32 offset);
extern void cyg_hal_sh_pcic_pci_cfg_write_dword (cyg_uint32 bus, cyg_uint32 devfn,
                                             cyg_uint32 offset, cyg_uint32 val);
extern void cyg_hal_sh_pcic_pci_cfg_write_word  (cyg_uint32 bus, cyg_uint32 devfn,
                                             cyg_uint32 offset, cyg_uint16 val);
extern void cyg_hal_sh_pcic_pci_cfg_write_byte   (cyg_uint32 bus, cyg_uint32 devfn,
                                              cyg_uint32 offset, cyg_uint8 val);

// Initialize the PCI bus.
externC void cyg_hal_sh_pcic_pci_init(void);
#define HAL_PCI_INIT() cyg_hal_sh_pcic_pci_init()

// Map PCI device resources starting from these addresses in PCI space.
// leave gap at start of IO and mem for super IO controller
#define HAL_PCI_ALLOC_BASE_IO     0x00002000
#define HAL_PCI_ALLOC_BASE_MEMORY 0x00000000

// This is where the PCI spaces are mapped in the CPU's address space.
#define HAL_PCI_PHYSICAL_IO_BASE     0  // no direct IO
#define HAL_PCI_PHYSICAL_MEMORY_BASE CYGARC_REG_PCIC_MEM_BASE

// Read a value from the PCI configuration space of the appropriate
// size at an address composed from the bus, devfn and offset.
#define HAL_PCI_CFG_READ_UINT8( __bus, __devfn, __offset, __val )  \
    __val = cyg_hal_sh_pcic_pci_cfg_read_byte((__bus),  (__devfn), (__offset))
    
#define HAL_PCI_CFG_READ_UINT16( __bus, __devfn, __offset, __val ) \
    __val = cyg_hal_sh_pcic_pci_cfg_read_word((__bus),  (__devfn), (__offset))

#define HAL_PCI_CFG_READ_UINT32( __bus, __devfn, __offset, __val ) \
    __val = cyg_hal_sh_pcic_pci_cfg_read_dword((__bus),  (__devfn), (__offset))

// Write a value to the PCI configuration space of the appropriate
// size at an address composed from the bus, devfn and offset.
#define HAL_PCI_CFG_WRITE_UINT8( __bus, __devfn, __offset, __val )  \
    cyg_hal_sh_pcic_pci_cfg_write_byte((__bus),  (__devfn), (__offset), (__val))

#define HAL_PCI_CFG_WRITE_UINT16( __bus, __devfn, __offset, __val ) \
    cyg_hal_sh_pcic_pci_cfg_write_word((__bus),  (__devfn), (__offset), (__val))

#define HAL_PCI_CFG_WRITE_UINT32( __bus, __devfn, __offset, __val ) \
    cyg_hal_sh_pcic_pci_cfg_write_dword((__bus),  (__devfn), (__offset), (__val))

// Read/write data to PCI IO space
extern void cyg_hal_sh_pcic_pci_io_write_byte (cyg_uint32 addr, cyg_uint8 data);
extern void cyg_hal_sh_pcic_pci_io_write_word (cyg_uint32 addr, cyg_uint16 data);
extern void cyg_hal_sh_pcic_pci_io_write_dword (cyg_uint32 addr, cyg_uint32 data);
extern cyg_uint8 cyg_hal_sh_pcic_pci_io_read_byte (cyg_uint32 addr);
extern cyg_uint16 cyg_hal_sh_pcic_pci_io_read_word (cyg_uint32 addr);
extern cyg_uint32 cyg_hal_sh_pcic_pci_io_read_dword (cyg_uint32 addr);

#define HAL_PCI_IO_READ_UINT8(addr, datum)   datum = cyg_hal_sh_pcic_pci_io_read_byte((cyg_uint32)addr)
#define HAL_PCI_IO_READ_UINT16(addr, datum)  datum = cyg_hal_sh_pcic_pci_io_read_word((cyg_uint32)addr)
#define HAL_PCI_IO_READ_UINT32(addr, datum)  datum = cyg_hal_sh_pcic_pci_io_read_dword((cyg_uint32)addr)
#define HAL_PCI_IO_WRITE_UINT8(addr, datum)  cyg_hal_sh_pcic_pci_io_write_byte((cyg_uint32)addr, datum)
#define HAL_PCI_IO_WRITE_UINT16(addr, datum) cyg_hal_sh_pcic_pci_io_write_word((cyg_uint32)addr, datum)
#define HAL_PCI_IO_WRITE_UINT32(addr, datum) cyg_hal_sh_pcic_pci_io_write_dword((cyg_uint32)addr, datum)

// Translate the PCI interrupt requested by the device (INTA#, INTB#,
// INTC# or INTD#) to the associated CPU interrupt (i.e., HAL vector).
// FIXME: These are incomplete. Not clear from the docs where the
// interrupts are routed - the below makes the onboard PCnet controller
// work.
#define HAL_PCI_TRANSLATE_INTERRUPT( __bus, __devfn, __vec, __valid)          \
    CYG_MACRO_START                                                           \
    cyg_uint8 __req;                                                          \
    HAL_PCI_CFG_READ_UINT8(__bus, __devfn, CYG_PCI_CFG_INT_PIN, __req);       \
    if (0 != __req) {                                                         \
        CYG_ADDRWORD __translation[4] = {                                     \
            CYGNUM_HAL_INTERRUPT_PCID,                                        \
            CYGNUM_HAL_INTERRUPT_PCIA,                                        \
            CYGNUM_HAL_INTERRUPT_PCIB,                                        \
            CYGNUM_HAL_INTERRUPT_PCIC};                                       \
                                                                              \
        __vec = __translation[((__req+CYG_PCI_DEV_GET_DEV(__devfn))&3)];      \
        __valid = true;                                                       \
    } else {                                                                  \
        /* Device will not generate interrupt requests. */                    \
        __valid = false;                                                      \
    }                                                                         \
    CYG_MACRO_END


#define HAL_PCI_IGNORE_DEVICE(__bus, __dev, __fn)  (0)

// Bus address translation macros
#define HAL_PCI_CPU_TO_BUS(__cpu_addr, __bus_addr)   \
    CYG_MACRO_START                                  \
    (__bus_addr) = CYGARC_BUS_ADDRESS(__cpu_addr);   \
    CYG_MACRO_END

#define HAL_PCI_BUS_TO_CPU(__bus_addr, __cpu_addr)        \
    CYG_MACRO_START                                       \
    (__cpu_addr) = CYGARC_UNCACHED_ADDRESS(__bus_addr);   \
    CYG_MACRO_END

//-----------------------------------------------------------------------------
// end of plf_io.h
#endif // CYGONCE_SH_PLF_IO_H
