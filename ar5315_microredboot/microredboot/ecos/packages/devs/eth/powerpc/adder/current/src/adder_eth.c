//==========================================================================
//
//      adder_eth.c
//
//      Ethernet device driver specifics for Analogue & Micro Adder (PPC850)
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002 Gary Thomas
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
//####BSDCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from OpenBSD or other sources,
// and are covered by the appropriate copyright disclaimers included herein.
//
// -------------------------------------------
//
//####BSDCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2002-11-25
// Purpose:      
// Description:  platform driver specifics for A&M Adder
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

// Ethernet device driver support for PHY on Adder/MPC850

#include <pkgconf/system.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/diag.h>

#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_if.h>
#include <cyg/hal/drv_api.h>

#include CYGDAT_DEVS_QUICC_ETH_INL  // Platform specifics
#include <cyg/hal/quicc/ppc8xx.h>                  // QUICC structure definitions

// MII interface
#define MII_Start            0x40000000
#define MII_Read             0x20000000
#define MII_Write            0x10000000
#define MII_Cmd              0x30000000
#define MII_Phy(phy)         (phy << 23)
#define MII_Reg(reg)         (reg << 18)
#define MII_TA               0x00020000

// Transceiver mode
#define PHY_BMCR             0x00    // Register number
#define PHY_BMCR_RESET       0x8000
#define PHY_BMCR_LOOPBACK    0x4000
#define PHY_BMCR_100MB       0x2000
#define PHY_BMCR_AUTO_NEG    0x1000
#define PHY_BMCR_POWER_DOWN  0x0800
#define PHY_BMCR_ISOLATE     0x0400
#define PHY_BMCR_RESTART     0x0200
#define PHY_BMCR_FULL_DUPLEX 0x0100
#define PHY_BMCR_COLL_TEST   0x0080

#define PHY_BMSR             0x01    // Status register
#define PHY_BMSR_AUTO_NEG    0x0020  
#define PHY_BMSR_LINK        0x0004

// Bits in port D - used for 2 wire MII interface
#define MII_DATA             0x1000
#define MII_CLOCK            0x0800

#define MII_SET_DATA(val)                      \
    if (val) {                                 \
        eppc->pio_pddat |= MII_DATA;           \
    } else {                                   \
        eppc->pio_pddat &= ~MII_DATA;          \
    }

#define MII_GET_DATA()                         \
    ((eppc->pio_pddat & MII_DATA) != 0)

#define MII_SET_CLOCK(val)                     \
    if (val) {                                 \
        eppc->pio_pddat |= MII_CLOCK;          \
    } else {                                   \
        eppc->pio_pddat &= ~MII_CLOCK;         \
    }

static cyg_uint32
phy_cmd(cyg_uint32 cmd)
{
    volatile EPPC *eppc = (volatile EPPC *)eppc_base();
    cyg_uint32  retval;
    int         i, off;
    bool        is_read = ((cmd & MII_Cmd) == MII_Read);

    // Set both bits as output
    eppc->pio_pddir |= MII_DATA | MII_CLOCK;

    // Preamble
    for (i = 0; i < 32; i++) {
        MII_SET_CLOCK(0);
        MII_SET_DATA(1);
        CYGACC_CALL_IF_DELAY_US(1);
        MII_SET_CLOCK(1);
        CYGACC_CALL_IF_DELAY_US(1);
    }

    // Command/data
    for (i = 0, off = 31; i < (is_read ? 14 : 32); i++, --off) {
        MII_SET_CLOCK(0);
        MII_SET_DATA((cmd >> off) & 0x00000001);
        CYGACC_CALL_IF_DELAY_US(1);
        MII_SET_CLOCK(1);
        CYGACC_CALL_IF_DELAY_US(1);
    }

    retval = cmd;

    // If read, fetch data register
    if (is_read) {
        retval >>= 16;

        MII_SET_CLOCK(0);
        eppc->pio_pddir &= ~MII_DATA;  // Data bit is now input
        CYGACC_CALL_IF_DELAY_US(1);
        MII_SET_CLOCK(1);
        CYGACC_CALL_IF_DELAY_US(1);
        MII_SET_CLOCK(0);
        CYGACC_CALL_IF_DELAY_US(1);

        for (i = 0, off = 15; i < 16; i++, off--) {
            MII_SET_CLOCK(1);
            retval <<= 1;
            retval |= MII_GET_DATA();
            CYGACC_CALL_IF_DELAY_US(1);
            MII_SET_CLOCK(0);
            CYGACC_CALL_IF_DELAY_US(1);
        }
    }

    // Set both bits as output
    eppc->pio_pddir |= MII_DATA | MII_CLOCK;

    // Postamble
    for (i = 0; i < 32; i++) {
        MII_SET_CLOCK(0);
        MII_SET_DATA(1);
        CYGACC_CALL_IF_DELAY_US(1);
        MII_SET_CLOCK(1);
        CYGACC_CALL_IF_DELAY_US(1);
    }

    return retval;
}

//
// PHY unit access (via MII channel)
//
static void
phy_write(int reg, int addr, unsigned short data)
{
    phy_cmd(MII_Start | MII_Write | MII_Phy(addr) | MII_Reg(reg) | MII_TA | data);
}

static bool
phy_read(int reg, int addr, unsigned short *val)
{
    cyg_uint32 ret;

    ret = phy_cmd(MII_Start | MII_Read | MII_Phy(addr) | MII_Reg(reg) | MII_TA);
    *val = ret;
    return true;
}

bool
_adder_reset_phy(void)
{
    volatile EPPC *eppc = (volatile EPPC *)eppc_base();
    int phy_timeout = 5*1000;  // Wait 5 seconds max for link to clear
    bool phy_ok;
    unsigned short phy_state = 0;
    int phy_unit = -1;
    int i;

    // Reset PHY (transceiver)
    eppc->pip_pbdat &= ~0x00004000;  // Reset PHY chip
    CYGACC_CALL_IF_DELAY_US(15000);  // > 10ms
    eppc->pip_pbdat |= 0x00004000;   // Enable PHY chip

    phy_ok = false;
    
    // Try and discover how this PHY is wired
    for (i = 0; i < 0x20; i++) {
        phy_read(PHY_BMCR, i, &phy_state);
        if ((phy_state & PHY_BMCR_RESET) == 0) {
            phy_unit = i;
            break;
        }
    }
    if (phy_unit < 0) {
        diag_printf("QUICC ETH - Can't locate PHY\n");
        return false;
    } else {
#if 0
        diag_printf("QUICC ETH - using PHY %d\n", phy_unit);
#endif
    }
    if (phy_read(PHY_BMSR, phy_unit, &phy_state)) {
        if ((phy_state & PHY_BMSR_LINK) !=  PHY_BMSR_LINK) {
            unsigned short reset_mode;
            phy_write(PHY_BMCR, phy_unit, PHY_BMCR_RESET);
            for (i = 0;  i < 10;  i++) {
                phy_ok = phy_read(PHY_BMCR, phy_unit, &phy_state);
                if (!phy_ok) break;
                if (!(phy_state & PHY_BMCR_RESET)) break;
            }
            if (!phy_ok || (phy_state & PHY_BMCR_RESET)) {
                diag_printf("QUICC/ETH: Can't get PHY unit to soft reset: %x\n", phy_state);
                return false;
            }
            reset_mode = PHY_BMCR_RESTART | PHY_BMCR_AUTO_NEG | PHY_BMCR_FULL_DUPLEX;
            phy_write(PHY_BMCR, phy_unit, reset_mode);
            while (phy_timeout-- >= 0) {
                phy_ok = phy_read(PHY_BMSR, phy_unit, &phy_state);
                if (phy_ok && (phy_state & PHY_BMSR_LINK)) {
                    break;
                } else {
                    CYGACC_CALL_IF_DELAY_US(10000);   // 10ms
                }
            }
            if (phy_timeout <= 0) {
                diag_printf("** QUICC/ETH Warning: PHY LINK UP failed\n");
            }
        }
        else {
            diag_printf("** QUICC/ETH Info: PHY LINK already UP \n");
        }
    }

    return phy_ok;
}

