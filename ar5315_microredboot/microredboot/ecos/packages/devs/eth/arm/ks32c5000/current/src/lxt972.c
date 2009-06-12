//==========================================================================
//
//      lxt972.c
//
//      Driver for LXT972 PHY
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
// Author(s):    cgarry
// Contributors: Based on code from: gthomas, jskov
//               Grant Edwards <grante@visi.com>
// Date:         2002-10-18
// Purpose:      
// Description:  
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/devs_eth_arm_ks32c5000.h>

#include "std.h"
#include "phy.h"

#define Bit(n) (1<<(n))

// address of the LX972 phy
#ifdef CYGPKG_DEVS_ETH_ARM_KS32C5000_PHYADDR
#define LX972_ADDR  CYGPKG_DEVS_ETH_ARM_KS32C5000_PHYADDR
#else
#define LX972_ADDR  0
#endif

// LX972 register offsets
#define LX972_CTRL_REG          0x00
#define LX972_STATUS1_REG       0x01
#define LX972_PHY_ID1_REG       0x02
#define LX972_PHY_ID2_REG       0x03
#define LX972_AN_ADVRT_REG      0x04
#define LX972_AN_LPAR_REG       0x05
#define LX972_AN_EXP_REG        0x06
#define LX972_AN_NEXTPAGE_REG   0x07
#define LX972_AN_LPAR_NP_REG    0x08
#define LX972_PORT_CONFIG_REG   0x10
#define LX972_STATUS2_REG       0x11
#define LX972_INT_ENAB_REG      0x12
#define LX972_INT_STAT_REG      0x13
#define LX972_LED_CONFIG_REG    0x14
#define LX972_DIG_CONFIG_REG    0x1A
#define LX972_TX_CTRL2_REG      0x1E

// LX972 Control register bit defines
#define LX972_CTRL_RESET        0x8000
#define LX972_CTRL_LOOPBACK     0x4000
#define LX972_CTRL_SPEED        0x2000  // 1=100Meg, 0=10Meg
#define LX972_CTRL_AN           0x1000  // 1=Enable auto negotiation, 0=disable it
#define LX972_CTRL_PWRDN        0x0800  // 1=Enable power down
#define LX972_CTRL_ISOLATE      0x0400  // 1=Isolate from MII
#define LX972_CTRL_RSTRT_AN     0x0200  // 1=Restart Auto Negotioation process
#define LX972_CTRL_FULL_DUP     0x0100  // 1=Enable full duplex mode, 0=half dup
#define LX972_CTRL_TST_COLL     0x0080  // 1=Enable collision test

// LX972 Interrupt enable register bit defines
#define LX972_INT_ENAB_ANMSK     Bit(7)
#define LX972_INT_ENAB_SPEEDMSK  Bit(6)
#define LX972_INT_ENAB_DUPLEXMSK Bit(5)
#define LX972_INT_ENAB_LINKMSK   Bit(4)
#define LX972_INT_ENAB_INTEN     Bit(1)
#define LX972_INT_ENAB_TINT      Bit(0)

// Map LED values from CDL file to reg defines
#define LINK_SPEED                                LX972_LED_CONFIG_SPEED_STATUS
#define TX_ACTIVITY                               LX972_LED_CONFIG_TX_STATUS
#define RX_ACTIVITY                               LX972_LED_CONFIG_RX_STATUS
#define COLLISION_STATUS                          LX972_LED_CONFIG_COLLISION_STATUS
#define DUPLEX_STATUS                             LX972_LED_CONFIG_DUPLEX_STATUS
#define LINK_ACTIVITY                             LX972_LED_CONFIG_RXTX_ACTIVITY
#define LINK_STATUS_RX_STATUS_COMBINED            LX972_LED_CONFIG_LINK_RX_COMB
#define LINK_STATUS_LINK_ACTIVITY_COMBINED        LX972_LED_CONFIG_LINK_ACT_COMB
#define DUPLEX_STATUS_COLLISION_STATUS_COMBINED   LX972_LED_CONFIG_DUPLEX_COLL_COMB
#define LINK_STATUS                               LX972_LED_CONFIG_LINK_STATUS
#define TEST_ON                                   LX972_LED_CONFIG_TEST_ON
#define TEST_OFF                                  LX972_LED_CONFIG_TEST_OFF
#define TEST_BLINK_FAST                           LX972_LED_CONFIG_BLINK_FAST
#define TEST_BLINK_SLOW                           LX972_LED_CONFIG_BLINK_SLOW

// LX972 LED Config register bit defines
#define LX972_LED_CONFIG_LED1SHIFT 12
#define LX972_LED_CONFIG_LED2SHIFT 8
#define LX972_LED_CONFIG_LED3SHIFT 4
#define LX972_LED_CONFIG_SPEED_STATUS     0x0
#define LX972_LED_CONFIG_TX_STATUS        0x1
#define LX972_LED_CONFIG_RX_STATUS        0x2
#define LX972_LED_CONFIG_COLLISION_STATUS 0x3
#define LX972_LED_CONFIG_LINK_STATUS      0x4
#define LX972_LED_CONFIG_DUPLEX_STATUS    0x5
#define LX972_LED_CONFIG_RXTX_ACTIVITY    0x7
#define LX972_LED_CONFIG_TEST_ON          0x8
#define LX972_LED_CONFIG_TEST_OFF         0x9
#define LX972_LED_CONFIG_BLINK_FAST       0xA
#define LX972_LED_CONFIG_BLINK_SLOW       0xB
#define LX972_LED_CONFIG_LINK_RX_COMB     0xC
#define LX972_LED_CONFIG_LINK_ACT_COMB    0xD
#define LX972_LED_CONFIG_DUPLEX_COLL_COMB 0xE
#define LX972_LED_CONFIG_STRETCH_100MS    0x8
#define LX972_LED_CONFIG_ENAB_LED_STRETCH Bit(1)

// LX972 Auto Negotiation Advertisement register bit defines
#define LX972_AN_ADVRT_NEXT_PAGE    Bit(15)
#define LX972_AN_ADVRT_PAUSE_ENA    Bit(10)
#define LX972_AN_ADVRT_100T4        Bit(9)
#define LX972_AN_ADVRT_100TX_FULL   Bit(8)
#define LX972_AN_ADVRT_100TX        Bit(7)
#define LX972_AN_ADVRT_10T_FULL     Bit(6)
#define LX972_AN_ADVRT_10T          Bit(5)
#define LX972_AN_ADVRT_SEL_802_3    Bit(0)

// LX972 Status register #2 bit defines
#define LX972_STATUS2_100M          Bit(14)
#define LX972_STATUS2_LINKUP        Bit(10)
#define LX972_STATUS2_FULLDUP       Bit(9)
#define LX972_STATUS2_ANEG_DONE     Bit(7)

// phy functions for Level1 PHY LXT972

void PhyReset(void)
{
    unsigned CtrlRegData;
    // First software reset the LX972
    MiiStationWrite(LX972_CTRL_REG, LX972_ADDR, LX972_CTRL_RESET);
    MiiStationWrite(LX972_CTRL_REG, LX972_ADDR, 0);

    // Wait until the LX972 reset cycle has completed
    // The Control register will read 0x7FFF until the reset cycle has completed
    CtrlRegData = 0x7FFF;
    while (CtrlRegData == 0x7FFF)
    {
        CtrlRegData = MiiStationRead(LX972_CTRL_REG, LX972_ADDR);
    }

    // Set up the LEDs' modes
    MiiStationWrite(LX972_LED_CONFIG_REG, LX972_ADDR,
                    (CYGPKG_DEVS_ETH_ARM_KS32C5000_PHY_LXT972_LED1 << LX972_LED_CONFIG_LED1SHIFT)
                  | (CYGPKG_DEVS_ETH_ARM_KS32C5000_PHY_LXT972_LED2 << LX972_LED_CONFIG_LED2SHIFT)
                  | (CYGPKG_DEVS_ETH_ARM_KS32C5000_PHY_LXT972_LED3 << LX972_LED_CONFIG_LED3SHIFT)
                  | LX972_LED_CONFIG_STRETCH_100MS
                  | LX972_LED_CONFIG_ENAB_LED_STRETCH);

    // Set MII drive strength
    MiiStationWrite(LX972_DIG_CONFIG_REG, LX972_ADDR, 0);

    // Enable interrupts
    MiiStationWrite(LX972_INT_ENAB_REG, LX972_ADDR,
                    LX972_INT_ENAB_ANMSK
                  | LX972_INT_ENAB_SPEEDMSK
                  | LX972_INT_ENAB_DUPLEXMSK
                  | LX972_INT_ENAB_LINKMSK
                  | LX972_INT_ENAB_INTEN);

    // Initialize auto-negotiation capabilities
    // Next page not enabled
    MiiStationWrite(LX972_AN_ADVRT_REG, LX972_ADDR,
                    LX972_AN_ADVRT_PAUSE_ENA
                  | LX972_AN_ADVRT_100T4
                  | LX972_AN_ADVRT_100TX_FULL
                  | LX972_AN_ADVRT_100TX
                  | LX972_AN_ADVRT_10T_FULL
                  | LX972_AN_ADVRT_10T
                  | LX972_AN_ADVRT_SEL_802_3);
#if 1
    // Now start an auto negotiation
    MiiStationWrite(LX972_CTRL_REG, LX972_ADDR,
                    LX972_CTRL_AN
                  | LX972_CTRL_RSTRT_AN);
#else
    // force to 10M full duplex
    MiiStationWrite(LX972_CTRL_REG, LX972_ADDR,
                    LX972_CTRL_FULL_DUP);
#endif
}

unsigned PhyStatus(void)
{
  unsigned lxt972Status = MiiStationRead(LX972_STATUS2_REG,LX972_ADDR);
  unsigned r = 0;
  if (lxt972Status & LX972_STATUS2_LINKUP)
    r |= PhyStatus_LinkUp;
  if (lxt972Status & LX972_STATUS2_FULLDUP)
    r |= PhyStatus_FullDuplex;
  if (lxt972Status & LX972_STATUS2_100M)
    r |=  PhyStatus_100Mb;
  return r;
}

void PhyInterruptAck(void)
{
  MiiStationRead(LX972_STATUS1_REG, LX972_ADDR);
  MiiStationRead(LX972_INT_STAT_REG, LX972_ADDR);
}

// EOF lxt972.c
