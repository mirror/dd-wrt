//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2003 Atheros Communications, Inc.
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
// Alternative licenses for eCos may be arranged by contacting the copyright
// holders.
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    Atheros Communications, Inc.
// Contributors: Atheros Engineering
// Date:         2003-10-22
// Purpose:      
// Description:  AR531X ethernet hardware driver
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================
/*
 * Manage the ethernet PHY.
 * This code supports a simple 1-port ethernet phy, Realtek RTL8201BL,
 * and compatible PHYs, such as the Kendin KS8721B.
 * All definitions in this file are operating system independent!
 */

#if defined(linux)
#include <linux/config.h>
#include <linux/types.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/delay.h>

#include "ar531xlnx.h"
#endif

#if defined(__ECOS)
#include "ae531xecos.h"
#endif


#include "ae531xmac.h"
#include "ae531xreg.h"
#include "rtPhy.h"

#if /* DEBUG */ 1
#define RT_DEBUG_ERROR     0x00000001
#define RT_DEBUG_PHYSETUP  0x00000002
#define RT_DEBUG_PHYCHANGE 0x00000004

int rtPhyDebug = RT_DEBUG_ERROR;

#define RT_PRINT(FLG, X)                            \
{                                                   \
    if (rtPhyDebug & (FLG)) {                       \
        DEBUG_PRINTF X;                             \
    }                                               \
}
#else
#define RT_PRINT(FLG, X)
#endif

/*
 * Track per-PHY port information.
 */
typedef struct {
    BOOL   phyAlive;    /* last known state of link */
    UINT32 phyBase;
    UINT32 phyAddr;
} rtPhyInfo_t;

#define ETH_PHY_ADDR		1

/*
 * This table defines the mapping from phy units to
 * per-PHY information.
 *
 * This table is somewhat board-dependent.
 */
rtPhyInfo_t rtPhyInfo[] = {
    {phyAlive: FALSE,  /* PHY 0 */
     phyBase: 0,       /* filled in by rt_phySetup */
     phyAddr: ETH_PHY_ADDR},                             

    {phyAlive: FALSE,  /* PHY 1 */
     phyBase: 0,       /* filled in by rt_phySetup */
     phyAddr: ETH_PHY_ADDR}
};

/* Convert from phy unit# to (phyBase, phyAddr) pair */
#define RT_PHYBASE(phyUnit) (rtPhyInfo[phyUnit].phyBase)
#define RT_PHYADDR(phyUnit) (rtPhyInfo[phyUnit].phyAddr)


/******************************************************************************
*
* rt_phySetup - reset and setup the PHY associated with
* the specified MAC unit number.
*
* Resets the associated PHY port.
*
* RETURNS:
*    TRUE  --> associated PHY is alive
*    FALSE --> no LINKs on this ethernet unit
*/

BOOL
rt_phySetup(int ethUnit, UINT32 phyBase)
{
    BOOL    linkAlive = FALSE;
    UINT32  phyAddr;

    RT_PHYBASE(ethUnit) = phyBase;

    phyAddr = RT_PHYADDR(ethUnit);

    /* Reset phy */
    phyRegWrite(phyBase, phyAddr, GEN_ctl, PHY_SW_RST | AUTONEGENA);

    sysMsDelay(1500);

    return linkAlive;
}

/******************************************************************************
*
* rt_phyIsDuplexFull - Determines whether the phy ports associated with the
* specified device are FULL or HALF duplex.
*
* RETURNS:
*    TRUE  --> FULL
*    FALSE --> HALF
*/
BOOL
rt_phyIsFullDuplex(int ethUnit)
{
    UINT16  phyCtl;
    UINT32  phyBase;
    UINT32  phyAddr;

    phyBase = RT_PHYBASE(ethUnit);
    phyAddr = RT_PHYADDR(ethUnit);

    phyCtl = phyRegRead(phyBase, phyAddr, GEN_ctl);

    if (phyCtl & DUPLEX) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/******************************************************************************
*
* rt_phyIsSpeed100 - Determines the speed of phy ports associated with the
* specified device.
*
* RETURNS:
*    TRUE --> 100Mbit
*    FALSE --> 10Mbit
*/
BOOL
rt_phyIsSpeed100(int phyUnit)
{
    UINT16  phyLpa;
    UINT32  phyBase;
    UINT32  phyAddr;

    phyBase = RT_PHYBASE(phyUnit);
    phyAddr = RT_PHYADDR(phyUnit);

    phyLpa = phyRegRead(phyBase, phyAddr, AN_lpa);

    if (phyLpa & (LPA_TXFD | LPA_TX)) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/*****************************************************************************
*
* rt_phyCheckStatusChange -- checks for significant changes in PHY state.
*
* A "significant change" is:
*     dropped link (e.g. ethernet cable unplugged) OR
*     autonegotiation completed + link (e.g. ethernet cable plugged in)
*
* On AR5311, there is a 1-to-1 mapping of ethernet units to PHYs.
* When a PHY is plugged in, phyLinkGained is called.
* When a PHY is unplugged, phyLinkLost is called.
*/
void
rt_phyCheckStatusChange(int ethUnit)
{
    UINT16          phyHwStatus;
    rtPhyInfo_t     *lastStatus = &rtPhyInfo[ethUnit];
    UINT32          phyBase;
    UINT32          phyAddr;

    phyBase = RT_PHYBASE(ethUnit);
    phyAddr = RT_PHYADDR(ethUnit);

    phyHwStatus = phyRegRead(phyBase, phyAddr, GEN_sts);

    if (lastStatus->phyAlive) { /* last known status was ALIVE */
        /* See if we've lost link */
        if (!(phyHwStatus & LINK)) {
            RT_PRINT(RT_DEBUG_PHYCHANGE,("\nethmac%d link down\n", ethUnit));
            lastStatus->phyAlive = FALSE;
            phyLinkLost(ethUnit);
        }
    } else { /* last known status was DEAD */
        /* Check for AN complete */
        if ((phyHwStatus & (AUTOCMPLT | LINK)) == (AUTOCMPLT | LINK)) {
            RT_PRINT(RT_DEBUG_PHYCHANGE,("\nethmac%d link up\n", ethUnit));
            lastStatus->phyAlive = TRUE;
            phyLinkGained(ethUnit);
        }
    }
}

#if DEBUG

/* Define the PHY registers of interest for a phyShow command */
struct rtRegisterTable_s {
    UINT32 regNum;
    char  *regIdString;
} rtRegisterTable[] =
{
    {GEN_ctl,    "Basic Mode Control (GEN_ctl)    "},
    {GEN_sts,    "Basic Mode Status (GEN_sts)     "},
    {GEN_id_hi,  "PHY Identifier 1 (GET_id_hi)    "},
    {GEN_id_lo,  "PHY Identifier 2 (GET_id_lo)    "},
    {AN_adv,     "Auto-Neg Advertisement (AN_adv) "},
    {AN_lpa,     "Auto-Neg Link Partner Ability   "},
    {AN_exp,     "Auto-Neg Expansion              "},
};

int rtNumRegs = sizeof(rtRegisterTable) / sizeof(rtRegisterTable[0]);

/*
 * Dump the state of a PHY.
 */
void
rt_phyShow(int phyUnit)
{
    int i;
    UINT16  value;
    UINT32  phyBase;
    UINT32  phyAddr;

    phyBase = RT_PHYBASE(phyUnit);
    phyAddr = RT_PHYADDR(phyUnit);

    printf("PHY state for ethphy%d\n", phyUnit);

    for (i=0; i<rtNumRegs; i++) {

        value = phyRegRead(phyBase, phyAddr, rtRegisterTable[i].regNum);

        printf("Reg %02d (0x%02x) %s = 0x%08x\n",
            rtRegisterTable[i].regNum, rtRegisterTable[i].regNum,
            rtRegisterTable[i].regIdString, value);
    }
}

/*
 * Modify the value of a PHY register.
 * This makes it a bit easier to modify PHY values during debug.
 */
void
rt_phySet(int phyUnit, UINT32 regnum, UINT32 value)
{
    UINT32  phyBase;
    UINT32  phyAddr;

    phyBase = RT_PHYBASE(phyUnit);
    phyAddr = RT_PHYADDR(phyUnit);

    phyRegWrite(phyBase, phyAddr, regnum, value);
}
#endif
