//==========================================================================
//
//      verde_pci.c
//
//      HAL support code for Verde PCI
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.
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
// Author(s):    msalter
// Contributors: msalter
// Date:         2002-01-30
// Purpose:      PCI support
// Description:  Implementations of HAL PCI interfaces
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include CYGBLD_HAL_PLATFORM_H

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_if.h>             // calling interface API
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>           // Interrupt names
#include <cyg/hal/hal_cache.h>
#include <cyg/io/pci_hw.h>
#include <cyg/io/pci.h>

#ifdef CYGPKG_IO_PCI

// Use "naked" attribute to suppress C prologue/epilogue
// This is a data abort handler which simply returns. Data aborts
// occur during configuration cycles if no device is present.
static void __attribute__ ((naked))
__pci_abort_handler(void) 
{
    asm ( "subs	pc, lr, #4\n" );
}

static cyg_uint32 orig_abort_vec;

#define DEBUG_CONFIG_VERBOSE 0

static inline void
pci_config_setup(cyg_uint32 bus, cyg_uint32 devfn, cyg_uint32 offset)
{
    cyg_uint32 dev = CYG_PCI_DEV_GET_DEV(devfn);
    cyg_uint32 fn  = CYG_PCI_DEV_GET_FN(devfn);
    cyg_uint8 localbus;

    localbus = (*ATU_PCIXSR >> 8) & 0xff;
    if (localbus == 0xff)
	localbus = 0;

    /* Offsets must be dword-aligned */
    offset &= ~3;
	
    /* Immediate bus use type 0 config, all others use type 1 config */
#if DEBUG_CONFIG_VERBOSE
    diag_printf("config: localbus[%d] bus[%d] dev[%d] fn[%d] offset[0x%x]\n",
		localbus, bus, dev, fn, offset);
#endif

    if (bus == localbus)
        *ATU_OCCAR = ( (1 << (dev + 16)) | (dev << 11) | (fn << 8) | offset | 0 );
    else
        *ATU_OCCAR = ( (bus << 16) | (dev << 11) | (fn << 8) | offset | 1 );

    orig_abort_vec = ((volatile cyg_uint32 *)0x20)[4];
    ((volatile unsigned *)0x20)[4] = (unsigned)__pci_abort_handler;
}

static inline int
pci_config_cleanup(cyg_uint32 bus)
{
    cyg_uint32 status = 0, err = 0;

    status = *ATU_ATUSR;
    if ((status & 0xF900) != 0) {
	err = 1;
	*ATU_ATUSR = status & 0xF900;
    }

    ((volatile unsigned *)0x20)[4] = orig_abort_vec;

    return err;
}


cyg_uint32
cyg_hal_plf_pci_cfg_read_dword (cyg_uint32 bus, cyg_uint32 devfn, cyg_uint32 offset)
{
    cyg_uint32 config_data;
    int err;

    pci_config_setup(bus, devfn, offset);

    config_data = *ATU_OCCDR;

    err = pci_config_cleanup(bus);

#if DEBUG_CONFIG_VERBOSE
    diag_printf("config read dword: data[0x%x] err[%d]\n",
		config_data, err);
#endif
    if (err)
      return 0xffffffff;
    else
      return config_data;
}


void
cyg_hal_plf_pci_cfg_write_dword (cyg_uint32 bus,
				 cyg_uint32 devfn,
				 cyg_uint32 offset,
				 cyg_uint32 data)
{
    int err;

    pci_config_setup(bus, devfn, offset);

    *ATU_OCCDR = data;

    err = pci_config_cleanup(bus);

#if DEBUG_CONFIG_VERBOSE
    diag_printf("config write dword: data[0x%x] err[%d]\n",
		data, err);
#endif
}


cyg_uint16
cyg_hal_plf_pci_cfg_read_word (cyg_uint32 bus,
			       cyg_uint32 devfn,
			       cyg_uint32 offset)
{
    cyg_uint16 config_data;
    int err;

    pci_config_setup(bus, devfn, offset & ~3);

    config_data = (cyg_uint16)(((*ATU_OCCDR) >> ((offset % 0x4) * 8)) & 0xffff);

    err = pci_config_cleanup(bus);

#if DEBUG_CONFIG_VERBOSE
    diag_printf("config read word: data[0x%x] err[%d]\n",
		config_data, err);
#endif
    if (err)
      return 0xffff;
    else
      return config_data;
}

void
cyg_hal_plf_pci_cfg_write_word (cyg_uint32 bus,
				cyg_uint32 devfn,
				cyg_uint32 offset,
				cyg_uint16 data)
{
    int err;
    cyg_uint32 mask, temp;

    pci_config_setup(bus, devfn, offset & ~3);

    mask = ~(0x0000ffff << ((offset % 0x4) * 8));

    temp = (cyg_uint32)(((cyg_uint32)data) << ((offset % 0x4) * 8));
    *ATU_OCCDR = (*ATU_OCCDR & mask) | temp; 

    err = pci_config_cleanup(bus);

#if DEBUG_CONFIG_VERBOSE
    diag_printf("config write word: data[0x%x] err[%d]\n",
		data, err);
#endif
}

cyg_uint8
cyg_hal_plf_pci_cfg_read_byte (cyg_uint32 bus,
			       cyg_uint32 devfn,
			       cyg_uint32 offset)
{
    int err;
    cyg_uint8 config_data;

    pci_config_setup(bus, devfn, offset & ~3);

    config_data = (cyg_uint8)(((*ATU_OCCDR) >> ((offset % 0x4) * 8)) & 0xff);

    err = pci_config_cleanup(bus);

#if DEBUG_CONFIG_VERBOSE
    diag_printf("config read byte: data[0x%x] err[%d]\n",
		config_data, err);
#endif
    if (err)
	return 0xff;
    else
	return config_data;
}


void
cyg_hal_plf_pci_cfg_write_byte (cyg_uint32 bus,
				cyg_uint32 devfn,
				cyg_uint32 offset,
				cyg_uint8 data)
{
    int err;
    cyg_uint32 mask, temp;

    pci_config_setup(bus, devfn, offset & ~3);

    mask = ~(0x000000ff << ((offset % 0x4) * 8));
    temp = (cyg_uint32)(((cyg_uint32)data) << ((offset % 0x4) * 8));
    *ATU_OCCDR = (*ATU_OCCDR & mask) | temp; 

    err = pci_config_cleanup(bus);

#if DEBUG_CONFIG_VERBOSE
    diag_printf("config write byte: data[0x%x] err[%d]\n",
		data, err);
#endif
}

#endif // CYGPKG_IO_PCI


