//==========================================================================
//
//      dev/DP83847.c
//
//      Ethernet transciever (PHY) support 
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
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
// Author(s):    gthomas
// Contributors: 
// Date:         2003-08-01
// Purpose:      
// Description:  Support for ethernet NS DP83847 PHY
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/devs_eth_phy.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/diag.h>

#include <cyg/hal/hal_arch.h>
#include <cyg/hal/drv_api.h>
#include <cyg/hal/hal_if.h>
#include <cyg/hal/hal_tables.h>

#include <cyg/io/eth_phy.h>
#include <cyg/io/eth_phy_dev.h>

static bool dp83847_stat(eth_phy_access_t *f, int *state)
{
    unsigned short phy_state;
    int tries;

    // Read negotiated state
    if (_eth_phy_read(f, 0x10, f->phy_addr, &phy_state)) {
        if ((phy_state & 0x10) == 0) {
            diag_printf("... waiting for auto-negotiation");
            for (tries = 0;  tries < CYGINT_DEVS_ETH_PHY_AUTO_NEGOTIATION_TIME;  tries++) {
                if (_eth_phy_read(f, 0x10, f->phy_addr, &phy_state)) {
                    if ((phy_state & 0x10) != 0) {
                        break;
                    }
                }
                CYGACC_CALL_IF_DELAY_US(1000000);   // 1 second
                diag_printf(".");
            }
            diag_printf("\n");
        }
        if ((phy_state & 0x10) != 0) {
            *state = 0;
            if ((phy_state & 0x0001) != 0) *state |= ETH_PHY_STAT_LINK;
            if ((phy_state & 0x0002) == 0) *state |= ETH_PHY_STAT_100MB;
            if ((phy_state & 0x0004) != 0) *state |= ETH_PHY_STAT_FDX;
            return true;
        }
    }
    return false;
}

_eth_phy_dev("National Semiconductor DP83847", 0x20005c30, dp83847_stat)
