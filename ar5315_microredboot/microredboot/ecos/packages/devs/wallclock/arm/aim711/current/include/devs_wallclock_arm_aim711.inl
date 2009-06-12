//==========================================================================
//
//      devs/wallclock/arm/aim711/include/devs_wallclock_arm_aim711.inl
//
//      ARM Industrial Module AIM 711 RTC IO definitions
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Gary Thomas
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
// Author(s):   rcassebohm
// Contributors:rcassebohm
// Date:        2003-10-05
// Purpose:     AIM 711 RTC definitions for using DS1339
//####DESCRIPTIONEND####
//==========================================================================

#include <cyg/hal/hal_io.h>
#include <string.h>

#include CYGDAT_DEVS_WALLCLOCK_ARM_AIM711_CFG

static __inline__ void
DS_GET(cyg_uint8 *regs)
{
    hal_ks32c_i2c_msg_t msgs[2];
    int status;
    cyg_uint8 offset = 0x00;

    // write message to set the address
    msgs[0].devaddr = AIM711_RTC_ADDR | KS32C_I2C_WR;
    msgs[0].pbuf = &offset;
    msgs[0].bufsize = sizeof(offset);

    // read message
    msgs[1].devaddr = AIM711_RTC_ADDR | KS32C_I2C_RD;
    msgs[1].pbuf = regs;
    msgs[1].bufsize = DS_REGS_SIZE;

    // transfer the messages
    status = hal_ks32c_i2c_transfer(2, msgs);

    if (status < 0)
        diag_printf("%s - Can't read RTC\n", __FUNCTION__);

    return;
}

static __inline__ void
DS_PUT(cyg_uint8 *regs)
{
    hal_ks32c_i2c_msg_t msg;
    int status;
    cyg_uint8 offset = 0x00;
    cyg_uint8 buffer[1 + DS_REGS_SIZE];

    buffer[0]=offset;
    memcpy(&buffer[1], regs, DS_REGS_SIZE);

    // write message 
    msg.devaddr = AIM711_RTC_ADDR | KS32C_I2C_WR;
    msg.pbuf = buffer;
    msg.bufsize = sizeof(buffer);

    // transfer the messages
    status = hal_ks32c_i2c_transfer(1, &msg);

    if (status < 0)
        diag_printf("%s - Can't write RTC\n", __FUNCTION__);

    return;
}

// EOF devs_wallclock_arm_aim711.inl
