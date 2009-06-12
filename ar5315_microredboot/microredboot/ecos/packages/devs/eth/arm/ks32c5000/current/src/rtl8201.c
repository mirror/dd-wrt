//==========================================================================
//
//      rtl8201.c
//
//      Driver for RTL8201 PHY
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributoris: rcassebohm, gthomas, jskov
//               Grant Edwards <grante@visi.com>
// Date:         2001-07-31
// Purpose:
// Description:
//
//####DESCRIPTIONEND####
//

#include <pkgconf/system.h>
#include <pkgconf/hal.h>
#include <cyg/hal/hal_if.h>

#include <pkgconf/devs_eth_arm_ks32c5000.h>

#include <cyg/infra/diag.h>

#include "std.h"
#include "phy.h"

// Set up the level of debug output
#if CYGPKG_DEVS_ETH_ARM_KS32C5000_DEBUG_LEVEL > 0
#define debug1_printf(args...) diag_printf(args)
#else
#define debug1_printf(args...) /* noop */
#endif
#if CYGPKG_DEVS_ETH_ARM_KS32C5000_DEBUG_LEVEL > 1
#define debug2_printf(args...) diag_printf(args)
#else
#define debug2_printf(args...) /* noop */
#endif

// address of the RTL8201 phy
#ifdef CYGPKG_DEVS_ETH_ARM_KS32C5000_PHYADDR
#define RTL8201_ADDR  CYGPKG_DEVS_ETH_ARM_KS32C5000_PHYADDR
#else
#error no Phy addr set
#endif

// RTL8201 register offsets
#define	RTL8201_CNTL_REG 	0x00
#define	RTL8201_STATUS_REG 	0x01
#define	RTL8201_ID_REG1    	0x02
#define	RTL8201_ID_REG2    	0x03
#define	RTL8201_ANA_REG    	0x04
#define	RTL8201_ANLPA_REG 	0x05
#define	RTL8201_ANE_REG    	0x06
#define	RTL8201_NSR_REG    	0x10
#define	RTL8201_LBREMR_REG    	0x11
#define	RTL8201_RXER_REG    	0x12
#define	RTL8201_10M_NIC_REG    	0x13
#define	RTL8201_PHY1_1_REG    	0x14
#define	RTL8201_PHY1_2_REG    	0x15
#define	RTL8201_PHY2_REG    	0x16
#define	RTL8201_TWISTER1_REG   	0x17
#define	RTL8201_TWISTER2_REG   	0x18
#define	RTL8201_TEST_REG   	0x19

// RTL8201 Control register bit defines
#define RTL8201_CNTL_RESET        0x8000
#define RTL8201_CNTL_LOOPBACK     0x4000
#define RTL8201_CNTL_SPEED        0x2000  // 1=100Meg, 0=10Meg
#define RTL8201_CNTL_AN           0x1000  // 1=Enable auto negotiation, 0=disable it
#define RTL8201_CNTL_PWRDN        0x0800  // 1=Enable power down
#define RTL8201_CNTL_RSTRT_AN     0x0200  // 1=Restart Auto Negotioation process
#define RTL8201_CNTL_FULL_DUP     0x0100  // 1=Enable full duplex mode, 0=half dup

#define Bit(n) (1<<(n))

#define RTL8201_STATUS_100T4        Bit(15)
#define RTL8201_STATUS_100TX_FULL   Bit(14)
#define RTL8201_STATUS_100TX        Bit(13)
#define RTL8201_STATUS_10T_FULL     Bit(12)
#define RTL8201_STATUS_10T          Bit(11)
#define RTL8201_STATUS_AN_COMLETE   Bit(5)
#define RTL8201_STATUS_LINKUP       Bit(2)

#define RTL8201_ANA_PAUSE_ENA    Bit(10)
#define RTL8201_ANA_100T4        Bit(9)
#define RTL8201_ANA_100TX_FULL   Bit(8)
#define RTL8201_ANA_100TX        Bit(7)
#define RTL8201_ANA_10T_FULL     Bit(6)
#define RTL8201_ANA_10T          Bit(5)
#define RTL8201_ANA_SEL_802_3    Bit(0)

#define RTL8201_TEST_LINK_10   Bit(1)
#define RTL8201_TEST_LINK_100  Bit(0)
#define RTL8201_TEST_PHY_ADR   (0x1f<<8)

void PhyReset(void)
{
    static int init_done=0;
    unsigned Status;

    if (init_done)
	return;

    init_done=1;

    debug2_printf("Phy addr %d\n",RTL8201_ADDR);
    // first software reset the RTL8201      
    MiiStationWrite(RTL8201_CNTL_REG, RTL8201_ADDR, RTL8201_CNTL_RESET);
    MiiStationWrite(RTL8201_CNTL_REG, RTL8201_ADDR, 0);

    // initialize auto-negotiation capabilities
    MiiStationWrite(RTL8201_ANA_REG,RTL8201_ADDR, 
                    RTL8201_ANA_100TX_FULL+
                    RTL8201_ANA_100TX+
                    RTL8201_ANA_10T_FULL+
                    RTL8201_ANA_10T+
                    RTL8201_ANA_SEL_802_3);
    // Now start an auto negotiation
    debug1_printf("Start auto negotiation\n");
    MiiStationWrite(RTL8201_CNTL_REG, RTL8201_ADDR, 
                    RTL8201_CNTL_AN+
                    RTL8201_CNTL_RSTRT_AN);
}

unsigned PhyStatus(void)
{
    unsigned Status;
    unsigned r = 0, count=0;

    debug2_printf("Wait\n");

    // Wait for auto negotiation to get completed
    do
    {
        Status = MiiStationRead(RTL8201_STATUS_REG,RTL8201_ADDR);
        CYGACC_CALL_IF_DELAY_US(10000);
        count++;
    }
    while (!(Status&RTL8201_STATUS_AN_COMLETE) && count<500);
    //If it takes longer then 5 sec stop waiting
   
    debug2_printf("Wait finished\n");

    debug1_printf("PhyStatus is ");
    Status = MiiStationRead(RTL8201_STATUS_REG,RTL8201_ADDR);
    if (Status & RTL8201_STATUS_LINKUP)
    {
        r |= PhyStatus_LinkUp;
        debug1_printf("LINK ");
    }
    Status = MiiStationRead(RTL8201_CNTL_REG,RTL8201_ADDR);
    if (Status & RTL8201_CNTL_FULL_DUP)
    {
        r |= PhyStatus_FullDuplex;
        debug1_printf("FDX ");
    }
    Status = MiiStationRead(RTL8201_TEST_REG,RTL8201_ADDR);
    if (Status & RTL8201_TEST_LINK_100)
    {
        r |=  PhyStatus_100Mb;
        debug1_printf("100MBit ");
    }
    else if (r & PhyStatus_LinkUp)
        debug2_printf("10MBit ");
    debug1_printf("(0x%x)\n",r);

    return r;
}

void PhyInterruptAck(void)
{
}
