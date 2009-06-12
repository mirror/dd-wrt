//=============================================================================
//
//      m1543c.c
//
//      Init code for M1543C super IO controller
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
// Author(s):   jskov
// Contributors:jskov
// Date:        2001-07-10
// Description: Init code for M1543C super IO controller
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include CYGBLD_HAL_PLATFORM_H

#include <cyg/hal/hal_arch.h>           // SAVE/RESTORE GP macros
#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_intr.h>           // interrupt vectors
#include <cyg/io/pci.h>                 // PCI macro


//-----------------------------------------------------------------------------

#define SIO_READ_UINT8(_a_, _d_)               \
    CYG_MACRO_START                            \
    HAL_PCI_IO_READ_UINT8((_a_), (_d_));       \
    CYG_MACRO_END

#define SIO_WRITE_UINT8(_a_, _d_)              \
    CYG_MACRO_START                            \
    HAL_PCI_IO_WRITE_UINT8((_a_), (_d_));      \
    CYG_MACRO_END

//-----------------------------------------------------------------------------

#define SIO_CONFIG             0x000003f0
#define SIO_CONFIG_DATA        0x000003f1

#define SIO_CONFIG_DEV         0x07
#define SIO_CONFIG_POWER       0x22
#define SIO_CONFIG_ACTIVATE    0x30
#define SIO_CONFIG_ENTER1      0x51
#define SIO_CONFIG_ENTER2      0x23
#define SIO_CONFIG_BASE_HIGH   0x60
#define SIO_CONFIG_BASE_LOW    0x61
#define SIO_CONFIG_IRQ         0x70
#define SIO_CONFIG_EXIT        0xbb
#define SIO_CONFIG_MODE        0xf0

#define SIO_CONFIG_DEV_COM1    0x04
#define SIO_CONFIG_DEV_COM2    0x05
#define SIO_CONFIG_DEV_RTC     0x06

#define SIO_CONFIG_MODE_HIGH   0x02

#define SIO_CONFIG_ACTIVATE_ENABLE 0x01

#define SIO_CONFIG_POWER_COM1   0x10
#define SIO_CONFIG_POWER_COM2   0x20


void
cyg_hal_init_superIO(void)
{
#if 0
    // Enable PCI to ISA bridge (magic from Hitachi monitor).
    cyg_pci_write_config_uint32(CYG_PCI_DEV_MAKE_ID(0, 2<<3), 0x40, 0x8020c77f);
    cyg_pci_write_config_uint32(CYG_PCI_DEV_MAKE_ID(0, 2<<3), 0x44, 0x00001b9d);
    cyg_pci_write_config_uint32(CYG_PCI_DEV_MAKE_ID(0, 2<<3), 0x48, 0x00009315);
    cyg_pci_write_config_uint32(CYG_PCI_DEV_MAKE_ID(0, 2<<3), 0x4c, 0x0000000f);
#endif

    // Enter SuperIO config mode
    SIO_WRITE_UINT8(SIO_CONFIG, SIO_CONFIG_ENTER1);
    SIO_WRITE_UINT8(SIO_CONFIG, SIO_CONFIG_ENTER2);

    // Configure and enable COM1
    SIO_WRITE_UINT8(SIO_CONFIG, SIO_CONFIG_DEV);
    SIO_WRITE_UINT8(SIO_CONFIG_DATA, SIO_CONFIG_DEV_COM1);

    SIO_WRITE_UINT8(SIO_CONFIG, SIO_CONFIG_BASE_HIGH);
    SIO_WRITE_UINT8(SIO_CONFIG_DATA, 0x03);
    SIO_WRITE_UINT8(SIO_CONFIG, SIO_CONFIG_BASE_LOW);
    SIO_WRITE_UINT8(SIO_CONFIG_DATA, 0xf8);

    // Select IRQ 4 (see chap 8. Interrupt Controller)
    SIO_WRITE_UINT8(SIO_CONFIG, SIO_CONFIG_IRQ);
    SIO_WRITE_UINT8(SIO_CONFIG_DATA, 4);

    SIO_WRITE_UINT8(SIO_CONFIG, SIO_CONFIG_ACTIVATE);
    SIO_WRITE_UINT8(SIO_CONFIG_DATA, SIO_CONFIG_ACTIVATE_ENABLE);

    SIO_WRITE_UINT8(SIO_CONFIG, SIO_CONFIG_EXIT);
}

//-----------------------------------------------------------------------------
// end of smsc37c93x.c
