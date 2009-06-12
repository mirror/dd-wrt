//=============================================================================
//
//      pci_hw.c
//
//      PCI hardware library
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
// Author(s):    jskov, from design by nickg 
// Contributors: jskov
// Date:         1999-08-09
// Purpose:      PCI hardware configuration access
// Description: 
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <cyg/io/pci_hw.h>

// CYG_PCI_PRESENT only gets defined for targets that provide PCI HAL support.
// See pci_hw.h for details.
#ifdef CYG_PCI_PRESENT

// Init
void 
cyg_pcihw_init(void)
{
    HAL_PCI_INIT();
}

// Read functions
void 
cyg_pcihw_read_config_uint8( cyg_uint8 bus, cyg_uint8 devfn, 
                             cyg_uint8 offset, cyg_uint8 *val)
{
    HAL_PCI_CFG_READ_UINT8(bus, devfn, offset, *val);
}

void 
cyg_pcihw_read_config_uint16( cyg_uint8 bus, cyg_uint8 devfn, 
                              cyg_uint8 offset, cyg_uint16 *val)
{
    HAL_PCI_CFG_READ_UINT16(bus, devfn, offset, *val);
}

void
cyg_pcihw_read_config_uint32( cyg_uint8 bus, cyg_uint8 devfn, 
                              cyg_uint8 offset, cyg_uint32 *val)
{
    HAL_PCI_CFG_READ_UINT32(bus, devfn, offset, *val);
}

// Write functions
void 
cyg_pcihw_write_config_uint8( cyg_uint8 bus, cyg_uint8 devfn, 
                             cyg_uint8 offset, cyg_uint8 val)
{
    HAL_PCI_CFG_WRITE_UINT8(bus, devfn, offset, val);
}

void 
cyg_pcihw_write_config_uint16( cyg_uint8 bus, cyg_uint8 devfn, 
                              cyg_uint8 offset, cyg_uint16 val)
{
    HAL_PCI_CFG_WRITE_UINT16(bus, devfn, offset, val);
}

void
cyg_pcihw_write_config_uint32( cyg_uint8 bus, cyg_uint8 devfn, 
                              cyg_uint8 offset, cyg_uint32 val)
{
    HAL_PCI_CFG_WRITE_UINT32(bus, devfn, offset, val);
}

// Interrupt translation
cyg_bool
cyg_pcihw_translate_interrupt( cyg_uint8 bus, cyg_uint8 devfn,
                               CYG_ADDRWORD *vec)
{
    cyg_bool valid;

    HAL_PCI_TRANSLATE_INTERRUPT(bus, devfn, *vec, valid);

    return valid;
}

#endif // ifdef CYG_PCI_PRESENT

//-----------------------------------------------------------------------------
// end of pci_hw.c
