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
// Author(s):    msalter
// Contributors: msalter
// Date:         2002-01-10
// Purpose:      Intel IQ80321 PCI IO support macros
// Description: 
// Usage:        #include <cyg/hal/plf_io.h>
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/iq80321.h>
#include CYGBLD_HAL_PLF_INTS_H

// Initialize the PCI bus.
externC void cyg_hal_plf_pci_init(void);
#define HAL_PCI_INIT() cyg_hal_plf_pci_init()

//-----------------------------------------------------------------------------
// Resources

#define _PCI_MEM_BASE       0x80000000
#define _PCI_MEM_DAC_BASE   0x00000000
#define _PCI_IO_BASE	    0x90000000
#define _PCI_MEM_LIMIT      0x83ffffff
#define _PCI_IO_LIMIT       0x9000ffff

extern cyg_uint32 hal_pci_alloc_base_memory;
extern cyg_uint32 hal_pci_alloc_base_io;

extern cyg_uint32 hal_pci_physical_memory_base;
extern cyg_uint32 hal_pci_physical_io_base;

extern cyg_uint32 hal_pci_inbound_window_base;
extern cyg_uint32 hal_pci_inbound_window_mask;

#define HAL_PCI_PHYSICAL_MEMORY_BASE hal_pci_physical_memory_base
#define HAL_PCI_PHYSICAL_IO_BASE     hal_pci_physical_io_base

// Map PCI device resources starting from these addresses in PCI space.
#define HAL_PCI_ALLOC_BASE_MEMORY hal_pci_alloc_base_memory
#define HAL_PCI_ALLOC_BASE_IO     hal_pci_alloc_base_io

#ifdef CYGSEM_HAL_ARM_IQ80321_FAB_C
#define __INTA_ROUTING CYGNUM_HAL_INTERRUPT_XINT0
#define __INTB_ROUTING CYGNUM_HAL_INTERRUPT_XINT1

#define __NIC_PUB  3
#define __NIC_PRIV 5

#define __SLOT_PUB  4
#define __SLOT_PRIV 6
#else
#define __INTA_ROUTING CYGNUM_HAL_INTERRUPT_XINT2
#define __INTB_ROUTING CYGNUM_HAL_INTERRUPT_XINT3

#define __NIC_PUB  8
#define __NIC_PRIV 4

#define __SLOT_PUB  2
#define __SLOT_PRIV 6
#endif

// Translate the PCI interrupt requested by the device (INTA#, INTB#,
// INTC# or INTD#) to the associated CPU interrupt (i.e., HAL vector).
#define HAL_PCI_TRANSLATE_INTERRUPT( __bus, __devfn, __vec, __valid)           \
    CYG_MACRO_START                                                            \
    cyg_uint32 __dev = CYG_PCI_DEV_GET_DEV(__devfn);                           \
    cyg_uint32 __fn  = CYG_PCI_DEV_GET_FN(__devfn);                            \
    cyg_uint32 __xbus = ((*ATU_PCIXSR >> 8) & 0xff);                           \
    if (__xbus == 0xff) __xbus = 0;                                            \
    __valid = false;                                                           \
    if (__fn==0 && (__dev==__NIC_PUB || __dev==__NIC_PRIV) && __bus==__xbus) { \
        __vec = CYGNUM_HAL_INTERRUPT_ETHERNET;                                 \
        __valid = true;                                                        \
    } else {                                                                   \
        cyg_uint8 __req;                                                       \
        HAL_PCI_CFG_READ_UINT8(__bus, __devfn, CYG_PCI_CFG_INT_PIN, __req);    \
        switch(__req) {                                                        \
          case 1: /* INTA */                                                   \
            __vec=CYGNUM_HAL_INTERRUPT_XINT0; __valid=true; break;             \
          case 2: /* INTB */                                                   \
            __vec=CYGNUM_HAL_INTERRUPT_XINT1; __valid=true; break;             \
        }                                                                      \
    }                                                                          \
    CYG_MACRO_END

// SDRAM is aliased as uncached memory for drivers.
#ifdef CYG_HAL_MEMORY_MAP_NORMAL
#define CYGARC_UNCACHED_ADDRESS(_x_) \
  (((((unsigned long)(_x_)) >> 29)==0x0) ? (((unsigned long)(_x_))|0xC0000000) : (_x_))
#else
#define CYGARC_UNCACHED_ADDRESS(_x_) \
  (((((unsigned long)(_x_)) >> 29)==0x5) ? (((unsigned long)(_x_))+0x20000000) : (_x_))
#endif

#define CYGARC_VIRT_TO_BUS(_x_) \
  (((unsigned long)(_x_) & 0x1fffffff) | hal_pci_inbound_window_base)
#define CYGARC_BUS_TO_VIRT(_x_) \
  (((unsigned long)(_x_) & hal_pci_inbound_window_mask) | 0xC0000000)


static inline unsigned cygarc_physical_address(unsigned va)
{
    unsigned *ram_mmutab = (unsigned *)(SDRAM_BASE | 0x4000);
    unsigned pte;

    pte = ram_mmutab[va >> 20];

    return (pte & 0xfff00000) | (va & 0xfffff);
}

#define CYGARC_PHYSICAL_ADDRESS(_x_) cygarc_physical_address(_x_)

#ifdef CYG_HAL_MEMORY_MAP_NORMAL
static inline unsigned cygarc_virtual_address(unsigned pa)
{
    if (0xa0000000 <= pa && pa < 0xc0000000)
	return pa - 0xa0000000;
    if (pa < 0x90100000)
	return pa + 0x20000000;
    return pa;
}

#define CYGARC_VIRTUAL_ADDRESS(_x_) cygarc_virtual_address(_x_)
#else
#define CYGARC_VIRTUAL_ADDRESS(_x_) (_x_)
#endif    



//-----------------------------------------------------------------------------
// end of plf_io.h
#endif // CYGONCE_PLF_IO_H
