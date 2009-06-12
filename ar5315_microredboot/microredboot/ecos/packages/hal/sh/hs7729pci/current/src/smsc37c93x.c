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


//-----------------------------------------------------------------------------

#define SMSC_READ_UINT8(_a_, _d_)               \
    CYG_MACRO_START                             \
    cyg_uint16 t;                               \
    HAL_READ_UINT16((_a_), t);                  \
    (_d_) = (t >> 8) & 0xff;                    \
    CYG_MACRO_END

#define SMSC_WRITE_UINT8(_a_, _d_)              \
    CYG_MACRO_START                             \
    HAL_WRITE_UINT16((_a_), (_d_)<<8);          \
    CYG_MACRO_END

//-----------------------------------------------------------------------------

// From page 137+ in SMSC-37C93x.pdf
#define SMSC_CONFIG             0xa80007e0
#define SMSC_CONFIG_DATA        0xa80007e2

#define SMSC_CONFIG_DEV         0x07
#define SMSC_CONFIG_POWER       0x22
#define SMSC_CONFIG_ACTIVATE    0x30
#define SMSC_CONFIG_ENTER       0x55
#define SMSC_CONFIG_BASE_HIGH   0x60
#define SMSC_CONFIG_BASE_LOW    0x61
#define SMSC_CONFIG_IRQ         0x70
#define SMSC_CONFIG_EXIT        0xaa
#define SMSC_CONFIG_MODE        0xf0

#define SMSC_CONFIG_DEV_COM1    0x04
#define SMSC_CONFIG_DEV_COM2    0x05
#define SMSC_CONFIG_DEV_RTC     0x06

#define SMSC_CONFIG_MODE_HIGH   0x02

#define SMSC_CONFIG_ACTIVATE_ENABLE 0x01

#define SMSC_CONFIG_POWER_COM1   0x10
#define SMSC_CONFIG_POWER_COM2   0x20


void
cyg_hal_init_superIO(void)
{
    // Note: two writes unlike the 37m81x!
    SMSC_WRITE_UINT8(SMSC_CONFIG, SMSC_CONFIG_ENTER);
    SMSC_WRITE_UINT8(SMSC_CONFIG, SMSC_CONFIG_ENTER);

    // Power ON COM1 and COM2
    SMSC_WRITE_UINT8(SMSC_CONFIG_POWER, SMSC_CONFIG_POWER_COM1|SMSC_CONFIG_POWER_COM2);

    // Configure and enable COM1
    SMSC_WRITE_UINT8(SMSC_CONFIG, SMSC_CONFIG_DEV);
    SMSC_WRITE_UINT8(SMSC_CONFIG_DATA, SMSC_CONFIG_DEV_COM1);

    SMSC_WRITE_UINT8(SMSC_CONFIG, SMSC_CONFIG_BASE_HIGH);
    SMSC_WRITE_UINT8(SMSC_CONFIG_DATA, 0x03);
    SMSC_WRITE_UINT8(SMSC_CONFIG, SMSC_CONFIG_BASE_LOW);
    SMSC_WRITE_UINT8(SMSC_CONFIG_DATA, 0xf8);

    SMSC_WRITE_UINT8(SMSC_CONFIG, SMSC_CONFIG_IRQ);
    SMSC_WRITE_UINT8(SMSC_CONFIG_DATA, 3); // UIO_IRQ3

    SMSC_WRITE_UINT8(SMSC_CONFIG, SMSC_CONFIG_MODE);
    SMSC_WRITE_UINT8(SMSC_CONFIG_DATA, SMSC_CONFIG_MODE_HIGH);

    SMSC_WRITE_UINT8(SMSC_CONFIG, SMSC_CONFIG_ACTIVATE);
    SMSC_WRITE_UINT8(SMSC_CONFIG_DATA, SMSC_CONFIG_ACTIVATE_ENABLE);

    // Configure and enable COM2
    SMSC_WRITE_UINT8(SMSC_CONFIG, SMSC_CONFIG_DEV);
    SMSC_WRITE_UINT8(SMSC_CONFIG_DATA, SMSC_CONFIG_DEV_COM2);

    SMSC_WRITE_UINT8(SMSC_CONFIG, SMSC_CONFIG_BASE_HIGH);
    SMSC_WRITE_UINT8(SMSC_CONFIG_DATA, 0x02);
    SMSC_WRITE_UINT8(SMSC_CONFIG, SMSC_CONFIG_BASE_LOW);
    SMSC_WRITE_UINT8(SMSC_CONFIG_DATA, 0xf8);

    SMSC_WRITE_UINT8(SMSC_CONFIG, SMSC_CONFIG_IRQ);
    SMSC_WRITE_UINT8(SMSC_CONFIG_DATA, 4); // UIO_IRQ4

    SMSC_WRITE_UINT8(SMSC_CONFIG, SMSC_CONFIG_MODE);
    SMSC_WRITE_UINT8(SMSC_CONFIG_DATA, SMSC_CONFIG_MODE_HIGH);

    SMSC_WRITE_UINT8(SMSC_CONFIG, SMSC_CONFIG_ACTIVATE);
    SMSC_WRITE_UINT8(SMSC_CONFIG_DATA, SMSC_CONFIG_ACTIVATE_ENABLE);

    // Configure and enable RTC
    SMSC_WRITE_UINT8(SMSC_CONFIG, SMSC_CONFIG_DEV);
    SMSC_WRITE_UINT8(SMSC_CONFIG_DATA, SMSC_CONFIG_DEV_RTC);
    SMSC_WRITE_UINT8(SMSC_CONFIG, SMSC_CONFIG_ACTIVATE);
    SMSC_WRITE_UINT8(SMSC_CONFIG_DATA, SMSC_CONFIG_ACTIVATE_ENABLE);

    SMSC_WRITE_UINT8(SMSC_CONFIG, SMSC_CONFIG_EXIT);
}

//-----------------------------------------------------------------------------
// end of smsc37c93x.c
