#ifndef CYGONCE_PCI_HW_H
#define CYGONCE_PCI_HW_H
//=============================================================================
//
//      pci_hw.h
//
//      PCI hardware library
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2004 Red Hat, Inc.
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
// Author(s):    jskov, from design by nickg 
// Contributors: jskov
// Date:         1999-08-09
// Purpose:      PCI hardware configuration access
// Usage:
//              #include <cyg/io/pci_hw.h>
// Description: 
//          This API is used by the PCI library to access the PCI bus
//          configuration space. Although this should not normally be
//          necessary, this API may also be used by device driver or 
//          application code to perform PCI bus operations not supported
//          by the PCI library.
//
//####DESCRIPTIONEND####
//
//=============================================================================

// Rely on hal_io.h to include plf_io.h if it exists for the selected target.
#include <cyg/hal/hal_io.h>             // HAL_PCI_ macros

#ifdef HAL_PCI_INIT

// This varible selects whether the PCI library gets built (requires
// HAL_PCI_INIT to be defined by the platform io header file)
#define CYG_PCI_PRESENT

#include <cyg/infra/cyg_type.h>

#include <cyg/io/pci_cfg.h>


// This is the lowest level where devfns are used.
#define CYG_PCI_DEV_MAKE_DEVFN(__dev, __fn) (((__dev)<<3)|(__fn))
#define CYG_PCI_DEV_GET_DEV(__devfn) ((__devfn>>3)&0x1f)
#define CYG_PCI_DEV_GET_FN(__devfn) (__devfn&0x7)

// Some buggy PCI chips force us to ignore certain devices so that
// they may be handled specially.
#ifdef HAL_PCI_IGNORE_DEVICE
#define CYG_PCI_IGNORE_DEVICE(__bus, __dev, __fn) \
            HAL_PCI_IGNORE_DEVICE((__bus), (__dev), (__fn))
#else
#define CYG_PCI_IGNORE_DEVICE(__bus, __dev, __fn) 0
#endif

// Ignore certain BARs at discretion of HAL
#ifdef HAL_PCI_IGNORE_BAR
#define CYG_PCI_IGNORE_BAR(__dinfo, __bar) \
            HAL_PCI_IGNORE_BAR((__dinfo), (__bar))
#else
#define CYG_PCI_IGNORE_BAR(__dinfo, __bar) 0
#endif


// Init
externC void cyg_pcihw_init(void);

// Read functions
externC void cyg_pcihw_read_config_uint8( cyg_uint8 bus, cyg_uint8 devfn,
                                          cyg_uint8 offset, cyg_uint8 *val);
externC void cyg_pcihw_read_config_uint16( cyg_uint8 bus, cyg_uint8 devfn,
                                           cyg_uint8 offset, cyg_uint16 *val);
externC void cyg_pcihw_read_config_uint32( cyg_uint8 bus, cyg_uint8 devfn,
                                           cyg_uint8 offset, cyg_uint32 *val);

// Write functions
externC void cyg_pcihw_write_config_uint8( cyg_uint8 bus, cyg_uint8 devfn,
                                           cyg_uint8 offset, cyg_uint8 val);
externC void cyg_pcihw_write_config_uint16( cyg_uint8 bus, cyg_uint8 devfn,
                                            cyg_uint8 offset, cyg_uint16 val);
externC void cyg_pcihw_write_config_uint32( cyg_uint8 bus, cyg_uint8 devfn,
                                            cyg_uint8 offset, cyg_uint32 val);

// Interrupt translation
externC cyg_bool cyg_pcihw_translate_interrupt( cyg_uint8 bus, cyg_uint8 devfn,
                                                CYG_ADDRWORD *vec);

#endif // ifdef HAL_PCI_INIT

//-----------------------------------------------------------------------------
#endif // ifndef CYGONCE_PCI_HW_H
// End of pci_hw.h

