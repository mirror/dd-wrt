//=============================================================================
//
//      smsc37c93x.c
//
//      Init code for SMSC 37C93x super IO controller
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
// Date:        2001-05-30
// Description: Init code for SMSC 37C93x super IO controller
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include CYGBLD_HAL_PLATFORM_H

#include <cyg/hal/hal_arch.h>           // SAVE/RESTORE GP macros
#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_intr.h>           // interrupt vectors


// From page 137+ in SMSC-37C93x.pdf
#define SMSC_CONFIG              0xb04007e0
#define SMSC_CONFIG_DATA         0xb04007e2

#define SMSC_CONFIG_DEV          0x0700
#define SMSC_CONFIG_POWER        0x2200
#define SMSC_CONFIG_ACTIVATE     0x3000
#define SMSC_CONFIG_ENTER        0x5500
#define SMSC_CONFIG_BASE_HIGH    0x6000
#define SMSC_CONFIG_BASE_LOW     0x6100
#define SMSC_CONFIG_IRQ          0x7000
#define SMSC_CONFIG_EXIT         0xaa00
#define SMSC_CONFIG_MODE         0xf000

#define SMSC_CONFIG_DEV_COM1     0x0400
#define SMSC_CONFIG_DEV_COM2     0x0500

#define SMSC_CONFIG_MODE_HIGH    0x0200

#define SMSC_CONFIG_ACTIVATE_ENABLE 0x0100

#define SMSC_CONFIG_POWER_COM1   0x1000
#define SMSC_CONFIG_POWER_COM2   0x2000

void
cyg_hal_init_superIO(void)
{
    // Note: two writes unlike the 37m81x!
    HAL_WRITE_UINT16(SMSC_CONFIG, SMSC_CONFIG_ENTER);
    HAL_WRITE_UINT16(SMSC_CONFIG, SMSC_CONFIG_ENTER);

    // Power ON COM1
    HAL_WRITE_UINT16(SMSC_CONFIG_POWER, SMSC_CONFIG_POWER_COM1);

    // Configure and enable COM1
    HAL_WRITE_UINT16(SMSC_CONFIG, SMSC_CONFIG_DEV);
    HAL_WRITE_UINT16(SMSC_CONFIG_DATA, SMSC_CONFIG_DEV_COM1);

    HAL_WRITE_UINT16(SMSC_CONFIG, SMSC_CONFIG_BASE_HIGH);
    HAL_WRITE_UINT16(SMSC_CONFIG_DATA, 0x0300);
    HAL_WRITE_UINT16(SMSC_CONFIG, SMSC_CONFIG_BASE_LOW);
    HAL_WRITE_UINT16(SMSC_CONFIG_DATA, 0xf800);

    // Select IRQ 4 (see chap 8. Interrupt Controller)
    HAL_WRITE_UINT16(SMSC_CONFIG, SMSC_CONFIG_IRQ);
    HAL_WRITE_UINT16(SMSC_CONFIG_DATA, 4<<8);

    HAL_WRITE_UINT16(SMSC_CONFIG, SMSC_CONFIG_MODE);
    HAL_WRITE_UINT16(SMSC_CONFIG_DATA, SMSC_CONFIG_MODE_HIGH);

    HAL_WRITE_UINT16(SMSC_CONFIG, SMSC_CONFIG_ACTIVATE);
    HAL_WRITE_UINT16(SMSC_CONFIG_DATA, SMSC_CONFIG_ACTIVATE_ENABLE);

    // Exit setup mode
    HAL_WRITE_UINT16(SMSC_CONFIG, SMSC_CONFIG_EXIT);
}

//-----------------------------------------------------------------------------
// end of smsc37c93x.c
