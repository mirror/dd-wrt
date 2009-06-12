//==========================================================================
//
//      lxt970.c
//
//      Driver for LXT970 PHY
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
// Contributors: gthomas, jskov
//               Grant Edwards <grante@visi.com>
// Date:         2001-07-31
// Purpose:      
// Description:  
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include "std.h"
#include "phy.h"

// address of the LX970 phy
#ifdef CYGPKG_DEVS_ETH_ARM_KS32C5000_PHYADDR
#define LX970_ADDR  CYGPKG_DEVS_ETH_ARM_KS32C5000_PHYADDR
#else
#define LX970_ADDR  1
#endif

// LX970 register offsets
#define	LX970_CNTL_REG 		0x00
#define	LX970_STATUS_REG 	0x01
#define	LX970_ID_REG1    	0x02
#define	LX970_ID_REG2    	0x03
#define	LX970_ANA_REG    	0x04
#define	LX970_ANLPAR_REG 	0x05
#define	LX970_ANE_REG    	0x06
#define	LX970_MIRROR_REG    	0x10
#define	LX970_INTEN_REG    	0x11
#define	LX970_INTSTAT_REG    	0x12
#define	LX970_CONFIG_REG    	0x13
#define	LX970_CHIPSTAT_REG    	0x14

// LX970 Control register bit defines
#define LX970_CNTL_RESET        0x8000
#define LX970_CNTL_LOOPBACK     0x4000
#define LX970_CNTL_SPEED        0x2000  // 1=100Meg, 0=10Meg
#define LX970_CNTL_AN           0x1000  // 1=Enable auto negotiation, 0=disable it
#define LX970_CNTL_PWRDN        0x0800  // 1=Enable power down
#define LX970_CNTL_ISOLATE      0x0400  // 1=Isolate from MII
#define LX970_CNTL_RSTRT_AN     0x0200  // 1=Restart Auto Negotioation process
#define LX970_CNTL_FULL_DUP     0x0100  // 1=Enable full duplex mode, 0=half dup
#define LX970_CNTL_TST_COLL     0x0080  // 1=Enable collision test

#define Bit(n) (1<<(n))

#define LX970_ANA_PAUSE_ENA    Bit(10)
#define LX970_ANA_100T4        Bit(9)
#define LX970_ANA_100TX_FULL   Bit(8)
#define LX970_ANA_100TX        Bit(7)
#define LX970_ANA_10T_FULL     Bit(6)
#define LX970_ANA_10T          Bit(5)
#define LX970_ANA_SEL_802_3    Bit(0)

#define LX970_CHIPSTAT_LINKUP    Bit(13)
#define LX970_CHIPSTAT_FULLDUP   Bit(12)
#define LX970_CHIPSTAT_100M      Bit(11)
#define LX970_CHIPSTAT_ANEG_DONE Bit(9)
#define LX970_CHIPSTAT_PAGE_RX   Bit(8)
#define LX970_CHIPSTAT_LOWVOLT   Bit(2)

// phy functions for Level1 PHY LXT970

void PhyReset(void)
{
    // first software reset the LX970      
    MiiStationWrite(LX970_CNTL_REG, LX970_ADDR, LX970_CNTL_RESET);
    MiiStationWrite(LX970_CNTL_REG, LX970_ADDR, 0);

    // set low level drive for MII lines, enable interrupt output
    MiiStationWrite(17, LX970_ADDR, BIT3+BIT1);
   
    // default values for 100M encryption are wrong, so fix them
    // and configure LEDC to be activity indicator
     MiiStationWrite(19, LX970_ADDR, BIT7);
  
    // initialize auto-negotiation capabilities
    MiiStationWrite(LX970_ANA_REG,LX970_ADDR, 
                    LX970_ANA_PAUSE_ENA+
                    LX970_ANA_100T4+
                    LX970_ANA_100TX_FULL+
                    LX970_ANA_100TX+
                    LX970_ANA_10T_FULL+
                    LX970_ANA_10T+
                    LX970_ANA_SEL_802_3);
#if 1
    // Now start an auto negotiation
    MiiStationWrite(LX970_CNTL_REG, LX970_ADDR, 
                    LX970_CNTL_AN+
                    LX970_CNTL_RSTRT_AN);
#else
    // force to 10M full duplex
    MiiStationWrite(LX970_CNTL_REG, LX970_ADDR,
                    LX970_CNTL_FULL_DUP);
#endif
}

unsigned PhyStatus(void)
{
  unsigned lxt970Status = MiiStationRead(LX970_CHIPSTAT_REG,LX970_ADDR);
  unsigned r = 0;
  if (lxt970Status & LX970_CHIPSTAT_LINKUP)
    r |= PhyStatus_LinkUp;
  if (lxt970Status & LX970_CHIPSTAT_FULLDUP)
    r |= PhyStatus_FullDuplex;
  if (lxt970Status & LX970_CHIPSTAT_100M)
    r |=  PhyStatus_100Mb;
  return r;
}

void PhyInterruptAck(void)
{
  MiiStationRead(1,LX970_ADDR);
  MiiStationRead(18,LX970_ADDR);
}

// EOF lxt970.c
