//==========================================================================
//
//      devs/wallclock/powerpc/moab/include/devs_wallclock_moab.inl
//
//      TAMS MOAB RTC IO definitions
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
// Author(s):   rajt
// Contributors:rajt
// Date:        2001-07-19
// Purpose:     MOAB RTC definitions for using DS1307
//####DESCRIPTIONEND####
//==========================================================================

#include CYGDAT_DEVS_WALLCLOCK_MOAB_CFG

static __inline__ void
DS_GET(cyg_uint8 *regs)
{
    cyg_uint8 addr, page;

    // Read RTC 
    addr = 0x00;
    page = 0xD1;
    if (!hal_ppc405_i2c_put_bytes(page, &addr, 1)) {
        diag_printf("%s - Can't select page %x\n", __FUNCTION__, page);
        return;
    }
    if (!hal_ppc405_i2c_get_bytes(page, regs, DS_REGS_SIZE)) {
        diag_printf("%s - Can't read RTC\n", __FUNCTION__);
        return;
    }
#if 0
    diag_printf("RTC data - read\n");
    diag_dump_buf(regs, DS_REGS_SIZE);
#endif

#ifdef RTC_TEST
    addr = 0x10;
    page = 0xD1;
    if (!hal_ppc405_i2c_put_bytes(page, &addr, 1)) {
        diag_printf("%s - Can't select page %x\n", __FUNCTION__, page);
        return;
    }
    if (!hal_ppc405_i2c_get_bytes(page, regs, DS_REGS_SIZE)) {
        diag_printf("%s - Can't read RTC\n", __FUNCTION__);
        return;
    }
    diag_printf("RTC data - test\n");
    diag_dump_buf(regs, DS_REGS_SIZE);
#endif
}

static __inline__ void
DS_PUT(cyg_uint8 *regs)
{
    cyg_uint8 addr_data[DS_REGS_SIZE+1], page;
    int i;

#if 0
    diag_printf("RTC data - write\n");
    diag_dump_buf(regs, DS_REGS_SIZE);
#endif
    // Update RTC in one swoop
    addr_data[0] = 0x00;  // Starting register address
    for (i = 0;  i < DS_REGS_SIZE;  i++) {
        addr_data[i+1] = regs[i];
    }
    page = 0xD0;
    if (!hal_ppc405_i2c_put_bytes(page, addr_data, DS_REGS_SIZE+1)) {
        diag_printf("%s - Can't write registers\n", __FUNCTION__);
        return;
    }
#ifdef RTC_TEST
    // Test RTC by copying the registers to some of the RAM
    addr_data[0] = 0x10;  // Starting register address
    for (i = 0;  i < DS_REGS_SIZE;  i++) {
        addr_data[i+1] = regs[i];
    }
    page = 0xD0;
    if (!hal_ppc405_i2c_put_bytes(page, addr_data, DS_REGS_SIZE+1)) {
        diag_printf("%s - Can't write registers\n", __FUNCTION__);
        return;
    }
#endif
}

// EOF devs_wallclock_moab.inl
