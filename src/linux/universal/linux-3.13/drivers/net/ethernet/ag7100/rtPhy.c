/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright © 2003 Atheros Communications, Inc.,  All Rights Reserved.
 */

/*
 * Manage the ethernet PHY.
 * This code supports a simple 1-port ethernet phy, Realtek RTL8201BL,
 * and compatible PHYs, such as the Kendin KS8721B.
 * All definitions in this file are operating system independent!
 */

#include <linux/types.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/delay.h>

#include "ag7100_phy.h"

#if /* DEBUG */ 1
#define RT_DEBUG_ERROR     0x00000001
#define RT_DEBUG_PHYSETUP  0x00000002
#define RT_DEBUG_PHYCHANGE 0x00000004

int rtPhyDebug = RT_DEBUG_ERROR;

#define RT_PRINT(FLG, X)                            \
{                                                   \
    if (rtPhyDebug & (FLG)) {                       \
        printk X;                             \
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

#define ETH_PHY_ADDR		2

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


#if 1
    /* Reset phy */
    phyRegWrite(phyBase, phyAddr, GEN_ctl, PHY_SW_RST | AUTONEGENA);

    //mdelay(700);

    //printk("Rtphy id %#x\n", phyRegRead(0, phyAddr, 3));
    //phyRegWrite(0, phyAddr, GEN_ctl, 0x8000);
    //mdelay(1500);
#endif
    /*ag7100_mii_write(0, 2, 0, 0x8000);
    ag7100_mii_write(0, 2, 0, 0x6100);
    ag7100_mii_write(0, 2, 0x11, 0x100);*/
    /*phyRegWrite(0, 2, 0, 0x6100);
    phyRegWrite(0, 2, 0x11, 0x100);*/
    printk("r0 %#x id %#x r17 %#x\n", 
            phyRegRead(0, phyAddr, 0),
            phyRegRead(0, phyAddr, 3),
            phyRegRead(0, phyAddr, 0x11));

    return linkAlive;
}

/******************************************************************************
*
* rt_phyIsDuplexFull - Determines whether the phy ports associated with the
* specified device are FULL or HALF duplex.
*
* RETURNS:
*    1  --> FULL
*    0 --> HALF
*/
int
rt_phyIsFullDuplex(int ethUnit)
{
    UINT16  phyCtl;
    UINT32  phyBase;
    UINT32  phyAddr;

    phyBase = RT_PHYBASE(ethUnit);
    phyAddr = RT_PHYADDR(ethUnit);

    phyCtl = phyRegRead(phyBase, phyAddr, GEN_ctl);

    if (phyCtl & DUPLEX) {
        return 1;
    } else {
        return 0;
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
rt_phySpeed(int phyUnit)
{
    UINT16  phyLpa;
    UINT32  phyBase;
    UINT32  phyAddr;

    phyBase = RT_PHYBASE(phyUnit);
    phyAddr = RT_PHYADDR(phyUnit);

    phyLpa = phyRegRead(phyBase, phyAddr, AN_lpa);

    if (phyLpa & (LPA_TXFD | LPA_TX)) {
        return AG7100_PHY_SPEED_100TX;
    } else {
        return AG7100_PHY_SPEED_10T;
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
int
rt_phyIsUp(int ethUnit)
{
    UINT16          phyHwStatus;
    rtPhyInfo_t     *lastStatus = &rtPhyInfo[ethUnit];
    UINT32          phyBase;
    UINT32          phyAddr;

    phyBase = RT_PHYBASE(ethUnit);
    phyAddr = RT_PHYADDR(ethUnit);

    //mdelay(1000);
    phyHwStatus = phyRegRead(phyBase, phyAddr, GEN_sts);

    if (lastStatus->phyAlive) { /* last known status was ALIVE */
        /* See if we've lost link */
        if (!(phyHwStatus & LINK)) {
            RT_PRINT(RT_DEBUG_PHYCHANGE,("\nethmac%d link down\n", ethUnit));
            lastStatus->phyAlive = FALSE;
        }
    } else { /* last known status was DEAD */
        /* Check for AN complete */
        if ((phyHwStatus & (AUTOCMPLT | LINK)) == (AUTOCMPLT | LINK)) {
            RT_PRINT(RT_DEBUG_PHYCHANGE,("\nethmac%d link up\n", ethUnit));
            lastStatus->phyAlive = TRUE;
        }
    }
    return (lastStatus->phyAlive);
}

#ifdef DEBUG

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
