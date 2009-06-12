//==========================================================================
//
//      dev/eth_phy.c
//
//      Ethernet transciever (PHY) support 
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2003, 2004 Gary Thomas
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
// Author(s):    gthomas
// Contributors: 
// Date:         2003-08-01
// Purpose:      
// Description:  API support for ethernet PHY
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/diag.h>

#include <cyg/hal/hal_arch.h>
#include <cyg/hal/drv_api.h>
#include <cyg/hal/hal_if.h>
#include <cyg/hal/hal_tables.h>

#include <cyg/io/eth_phy.h>
#include <cyg/io/eth_phy_dev.h>

// Define table boundaries
CYG_HAL_TABLE_BEGIN( __ETH_PHY_TAB__, _eth_phy_devs );
CYG_HAL_TABLE_END( __ETH_PHY_TAB_END__, _eth_phy_devs );
extern struct _eth_phy_dev_entry __ETH_PHY_TAB__[], __ETH_PHY_TAB_END__;

// MII interface
#define MII_Start            0x40000000
#define MII_Read             0x20000000
#define MII_Write            0x10000000
#define MII_Cmd              0x30000000
#define MII_Phy(phy)         (phy << 23)
#define MII_Reg(reg)         (reg << 18)
#define MII_TA               0x00020000

//
// PHY unit access (via MII channel, using bit-level operations)
//

static cyg_uint32
phy_cmd(eth_phy_access_t *f, cyg_uint32 cmd)
{
    cyg_uint32  retval;
    int         i, off;
    bool        is_read = ((cmd & MII_Cmd) == MII_Read);

    // Set both bits as output
    (f->ops.bit_level_ops.set_dir)(1);

    // Preamble
    for (i = 0; i < 32; i++) {
        (f->ops.bit_level_ops.set_clock)(0);
        (f->ops.bit_level_ops.set_data)(1);
        CYGACC_CALL_IF_DELAY_US(1);
        (f->ops.bit_level_ops.set_clock)(1);
        CYGACC_CALL_IF_DELAY_US(1);
    }

    // Command/data
    for (i = 0, off = 31; i < (is_read ? 14 : 32); i++, --off) {
        (f->ops.bit_level_ops.set_clock)(0);
        (f->ops.bit_level_ops.set_data)((cmd >> off) & 0x00000001);
        CYGACC_CALL_IF_DELAY_US(1);
        (f->ops.bit_level_ops.set_clock)(1);
        CYGACC_CALL_IF_DELAY_US(1);
    }

    retval = cmd;

    // If read, fetch data register
    if (is_read) {
        retval >>= 16;

        (f->ops.bit_level_ops.set_clock)(0);
        (f->ops.bit_level_ops.set_dir)(0);
        CYGACC_CALL_IF_DELAY_US(1);
        (f->ops.bit_level_ops.set_clock)(1);
        CYGACC_CALL_IF_DELAY_US(1);
        (f->ops.bit_level_ops.set_clock)(0);
        CYGACC_CALL_IF_DELAY_US(1);

        for (i = 0, off = 15; i < 16; i++, off--) {
            (f->ops.bit_level_ops.set_clock)(1);
            retval <<= 1;
            retval |= (f->ops.bit_level_ops.get_data)();
            CYGACC_CALL_IF_DELAY_US(1);
            (f->ops.bit_level_ops.set_clock)(0);
            CYGACC_CALL_IF_DELAY_US(1);
        }
    }

    // Set both bits as output
    (f->ops.bit_level_ops.set_dir)(1);

    // Postamble
    for (i = 0; i < 32; i++) {
        (f->ops.bit_level_ops.set_clock)(0);
        (f->ops.bit_level_ops.set_data)(1);
        CYGACC_CALL_IF_DELAY_US(1);
        (f->ops.bit_level_ops.set_clock)(1);
        CYGACC_CALL_IF_DELAY_US(1);
    }

    return retval;
}

externC bool
_eth_phy_init(eth_phy_access_t *f)
{
    int addr;
    unsigned short state;
    unsigned long id;
    struct _eth_phy_dev_entry *dev;

    if (f->init_done) return true;
    (f->init)();
    // Scan to determine PHY address
    f->init_done = true;
    for (addr = 0;  addr < 0x20;  addr++) {
        if (_eth_phy_read(f, PHY_ID1, addr, &state)) {
            id = state << 16;
            if (_eth_phy_read(f, PHY_ID2, addr, &state)) {
                id |= state;
                f->phy_addr = addr;
                for (dev = __ETH_PHY_TAB__; dev != &__ETH_PHY_TAB_END__;  dev++) {
                    if (dev->id == id) {
                        diag_printf("PHY: %s\n", dev->name);
                        f->dev = dev;
                        return true;
                    }
                }
                diag_printf("Unsupported PHY device - id: %x\n", id);
                break;  // Can't handle this PHY
            }
        }
    }
    f->init_done = false;
    return false;
}

externC void
_eth_phy_reset(eth_phy_access_t *f)
{
    if (!f->init_done) {
        diag_printf("PHY reset without init on PHY: %x\n", f);
        return;
    }
    (f->init)();
}

externC void
_eth_phy_write(eth_phy_access_t *f, int reg, int addr, unsigned short data)
{
    if (!f->init_done) {
        diag_printf("PHY write without init on PHY: %x\n", f);
        return;
    }
    if (f->ops_type == PHY_BIT_LEVEL_ACCESS_TYPE) {
        phy_cmd(f, MII_Start | MII_Write | MII_Phy(addr) | MII_Reg(reg) | MII_TA | data);
    } else {
        (f->ops.reg_level_ops.put_reg)(reg, addr, data);
    }
}

externC bool
_eth_phy_read(eth_phy_access_t *f, int reg, int addr, unsigned short *val)
{
    cyg_uint32 ret;

    if (!f->init_done) {
        diag_printf("PHY read without init on PHY: %x\n", f);
        return false;
    }
    if (f->ops_type == PHY_BIT_LEVEL_ACCESS_TYPE) {
        ret = phy_cmd(f, MII_Start | MII_Read | MII_Phy(addr) | MII_Reg(reg) | MII_TA);
        *val = ret;
        return true;
    } else {
        return (f->ops.reg_level_ops.get_reg)(reg, addr, val);
    }
}

externC int
_eth_phy_cfg(eth_phy_access_t *f, int mode)
{
    int phy_timeout = 5*1000;  // Wait 5 seconds max for link to clear
    bool phy_ok;
    unsigned short reset_mode, phy_state;
    int i;

    if (!f->init_done) {
        diag_printf("PHY config without init on PHY: %x\n", f);
        return 0;
    }

    // Reset PHY (transceiver)
    phy_ok = false;
    _eth_phy_reset(f);

    _eth_phy_write(f, PHY_BMCR, f->phy_addr, PHY_BMCR_RESET);
    for (i = 0;  i < 5*100;  i++) {
        phy_ok = _eth_phy_read(f, PHY_BMCR, f->phy_addr, &phy_state);            
        diag_printf("PHY: %x\n", phy_state);
        if (phy_ok && !(phy_state & PHY_BMCR_RESET)) break;
        CYGACC_CALL_IF_DELAY_US(10000);   // 10ms
    }
    if (!phy_ok || (phy_state & PHY_BMCR_RESET)) {
        diag_printf("PPC405: Can't get PHY unit to soft reset: %x\n", phy_state);
        return 0;
    }

    reset_mode = PHY_BMCR_RESTART | PHY_BMCR_AUTO_NEG;
    _eth_phy_write(f, PHY_BMCR, f->phy_addr, reset_mode);
    while (phy_timeout-- >= 0) {
        phy_ok = _eth_phy_read(f, PHY_BMSR, f->phy_addr, &phy_state);
        if (phy_ok && (phy_state & PHY_BMSR_AUTO_NEG)) {
            break;
        } else {
            CYGACC_CALL_IF_DELAY_US(10000);   // 10ms
        }
    }
    if (phy_timeout <= 0) {
        diag_printf("** PPC405 Warning: PHY LINK UP failed: %04x\n", phy_state);
        return 0;
    }

    return _eth_phy_state(f);
}

externC int
_eth_phy_state(eth_phy_access_t *f)
{
    int state = 0;

    if (!f->init_done) {
        diag_printf("PHY state without init on PHY: %x\n", f);
        return 0;
    }
    if ((f->dev->stat)(f, &state)) {
        return state;
    } else {
        return 0;
    }
    return state;
}
