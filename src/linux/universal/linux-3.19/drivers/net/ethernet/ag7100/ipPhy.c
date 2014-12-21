/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright © 2003 Atheros Communications, Inc.,  All Rights Reserved.
 */

/*
 * Manage the ICPLUS ethernet PHY.
 *
 * All definitions in this file are operating system independent!
 */

#include <linux/types.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/delay.h>

#include "ag7100_phy.h"

#include "ipPhy.h"

/* PHY selections and access functions */

typedef enum {
    PHY_SRCPORT_INFO, 
    PHY_PORTINFO_SIZE,
} PHY_CAP_TYPE;

typedef enum {
    PHY_SRCPORT_NONE,
    PHY_SRCPORT_VLANTAG, 
    PHY_SRCPORT_TRAILER,
} PHY_SRCPORT_TYPE;

#ifdef DEBUG
#define DRV_DEBUG 1
#endif
#define DRV_DEBUG 1

#if DRV_DEBUG
#define DRV_DEBUG_PHYERROR  0x00000001
#define DRV_DEBUG_PHYCHANGE 0x00000002
#define DRV_DEBUG_PHYSETUP  0x00000004

int ipPhyDebug = DRV_DEBUG_PHYERROR;

#define DRV_LOG(FLG, X0, X1, X2, X3, X4, X5, X6)    \
{                                                   \
    if (ipPhyDebug & (FLG)) {                       \
        logMsg(X0, X1, X2, X3, X4, X5, X6);         \
    }                                               \
}

#define DRV_MSG(x,a,b,c,d,e,f)                      \
    logMsg(x,a,b,c,d,e,f)

#define DRV_PRINT(FLG, X)                           \
{                                                   \
    if (ipPhyDebug & (FLG)) {                       \
        printk X;                                   \
    }                                               \
}

#else /* !DRV_DEBUG */
#define DRV_LOG(DBG_SW, X0, X1, X2, X3, X4, X5, X6)
#define DRV_MSG(x,a,b,c,d,e,f)
#define DRV_PRINT(DBG_SW,X)
#endif

#define IP_LAN_PORT_VLAN          1
#define IP_WAN_PORT_VLAN          2

#define ENET_UNIT_DEFAULT 0

#define BOOL    uint32_t
#define TRUE    1
#define FALSE   0

/*
 * Track per-PHY port information.
 */
typedef struct {
    BOOL   isEnetPort;       /* normal enet port */
    BOOL   isPhyAlive;       /* last known state of link */
    int    ethUnit;          /* MAC associated with this phy port */
    uint32_t phyBase;
    uint32_t phyAddr;          /* PHY registers associated with this phy port */
    uint32_t VLANTableSetting; /* Value to be written to VLAN table */
} ipPhyInfo_t;

/*
 * Per-PHY information, indexed by PHY unit number.
 */
ipPhyInfo_t ipPhyInfo[] = {
    /*
     * On AP30/AR5312, all PHYs are associated with MAC0.
     * AP30/AR5312's MAC1 isn't used for anything.
     * CONFIG_VENETDEV==1 (router) configuration:
     *    Ports 0,1,2, and 3 are "LAN ports"
     *    Port 4 is a WAN port
     *    Port 5 connects to MAC0 in the AR5312
     * CONFIG_VENETDEV==0 (bridge) configuration:
     *    Ports 0,1,2,3,4 are "LAN ports"
     *    Port 5 connects to the MAC0 in the AR5312
     */
    {TRUE,   /* phy port 0 -- LAN port 0 */
     FALSE,
     ENET_UNIT_DEFAULT,
     0,
     IP_PHY0_ADDR,
     IP_LAN_PORT_VLAN
    },

    {TRUE,   /* phy port 1 -- LAN port 1 */
     FALSE,
     ENET_UNIT_DEFAULT,
     0,
     IP_PHY1_ADDR,
     IP_LAN_PORT_VLAN
    },

    {TRUE,   /* phy port 2 -- LAN port 2 */
     FALSE,
     ENET_UNIT_DEFAULT,
     0,
     IP_PHY2_ADDR, 
     IP_LAN_PORT_VLAN
    },

    {TRUE,   /* phy port 3 -- LAN port 3 */
     FALSE,
     ENET_UNIT_DEFAULT,
     0,
     IP_PHY3_ADDR, 
     IP_LAN_PORT_VLAN
    },

    {TRUE,   /* phy port 4 -- WAN port or LAN port 4 */
     FALSE,
     ENET_UNIT_DEFAULT,
     0,
     IP_PHY4_ADDR, 
     IP_LAN_PORT_VLAN   /* Send to all ports */
    },

    {FALSE,  /* phy port 5 -- CPU port (no RJ45 connector) */
     TRUE,
     ENET_UNIT_DEFAULT,
     0,
     0x00, 
     IP_LAN_PORT_VLAN    /* Send to all ports */
    },
};

#define IP_GLOBALREGBASE    0

#define IP_PHY_MAX (sizeof(ipPhyInfo) / sizeof(ipPhyInfo[0]))

/* Range of valid PHY IDs is [MIN..MAX] */
#define IP_ID_MIN 0
#define IP_ID_MAX (IP_PHY_MAX-1)

/* Convenience macros to access myPhyInfo */
#define IP_IS_ENET_PORT(phyUnit) (ipPhyInfo[phyUnit].isEnetPort)
#define IP_IS_PHY_ALIVE(phyUnit) (ipPhyInfo[phyUnit].isPhyAlive)
#define IP_ETHUNIT(phyUnit) (ipPhyInfo[phyUnit].ethUnit)
#define IP_PHYBASE(phyUnit) (ipPhyInfo[phyUnit].phyBase)
#define IP_PHYADDR(phyUnit) (ipPhyInfo[phyUnit].phyAddr)
#define IP_VLAN_TABLE_SETTING(phyUnit) (ipPhyInfo[phyUnit].VLANTableSetting)


#define IP_IS_ETHUNIT(phyUnit, ethUnit) \
            (IP_IS_ENET_PORT(phyUnit) &&        \
            IP_ETHUNIT(phyUnit) == (ethUnit))

/* Forward references */
BOOL       ip_phyIsLinkAlive(int phyUnit);
static void ip_VLANInit(int ethUnit);
static void ip_verifyReady(int ethUnit);
#if DEBUG
void       ip_phyShow(int phyUnit);
void       ip_phySet(int phyUnit, uint32_t regnum, uint32_t value);
void       ip_globalSet(uint32_t phyAddr, uint32_t regnum, uint32_t value);
#endif

/******************************************************************************
*
* ip_phyIsLinkAlive - test to see if the specified link is alive
*
* RETURNS:
*    TRUE  --> link is alive
*    FALSE --> link is down
*/
BOOL
ip_phyIsLinkAlive(int phyUnit)
{
    uint16_t phyHwStatus;
    uint32_t phyBase;
    uint32_t phyAddr;

    phyBase = IP_PHYBASE(phyUnit);
    phyAddr = IP_PHYADDR(phyUnit);

    phyHwStatus = phy_reg_read(phyBase, phyAddr, IP_PHY_STATUS);

    if (phyHwStatus & IP_STATUS_LINK_PASS) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/******************************************************************************
*
* ip_VLANInit - initialize "port-based VLANs" for the specified enet unit.
*/
static void
ip_VLANInit(int ethUnit)
{
    int     phyUnit;
    uint32_t  phyBase;
    uint32_t  phyReg;

    phyBase = IP_GLOBALREGBASE;
    
    for (phyUnit=0; phyUnit < IP_PHY_MAX; phyUnit++) {
        if (IP_ETHUNIT(phyUnit) != ethUnit) {
            continue;
        }
        phy_reg_write(phyBase, IP_GLOBAL_PHY29_ADDR, 
                    IP_GLOBAL_PHY29_24_REG + ((phyUnit == 5) ? (phyUnit + 1) : phyUnit),
                                    IP_VLAN_TABLE_SETTING(phyUnit));
        
        /* Send all packets to all ports */
        phyReg = phy_reg_read(phyBase, IP_GLOBAL_PHY30_ADDR, IP_GLOBAL_PHY30_1_REG);
        phyReg = phyReg | ((1 << phyUnit) << IP_VLAN1_OUTPUT_PORT_MASK_S);
        phy_reg_write(phyBase, IP_GLOBAL_PHY30_ADDR, IP_GLOBAL_PHY30_1_REG, phyReg);
    }
    phyReg = phy_reg_read(phyBase, IP_GLOBAL_PHY30_ADDR, IP_GLOBAL_PHY30_9_REG);
    phyReg = phyReg | TAG_VLAN_ENABLE;
    phyReg = phyReg & ~VID_INDX_SEL_M;
    phy_reg_write(phyBase, IP_GLOBAL_PHY30_ADDR, IP_GLOBAL_PHY30_9_REG, phyReg);

}


static void
ip_verifyReady(int ethUnit)
{
    int     phyUnit;
    uint32_t  phyBase = 0;
    uint32_t  phyAddr;
    uint16_t  phyID1;
    uint16_t  phyID2;

    /*
     * The first read to the Phy port registers always fails and
     * returns 0.   So get things started with a bogus read.
     */
    for (phyUnit=0; phyUnit < IP_PHY_MAX; phyUnit++) {
        if (!IP_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }

        phyBase = IP_PHYBASE(phyUnit);
        phyAddr = IP_PHYADDR(phyUnit);
    
        phyID1 = phy_reg_read(phyBase, phyAddr, IP_PHY_ID1); /* returns 0 */
        break;
    }

    for (phyUnit=0; phyUnit < IP_PHY_MAX; phyUnit++) {
        if (!IP_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }

        /*******************/
        /* Verify phy port */
        /*******************/
        phyBase = IP_PHYBASE(phyUnit);
        phyAddr = IP_PHYADDR(phyUnit);
    
        phyID1 = phy_reg_read(phyBase, phyAddr, IP_PHY_ID1);
        if (phyID1 != IP_PHY_ID1_EXPECTATION) {
            DRV_PRINT(DRV_DEBUG_PHYERROR,
                      ("Invalid PHY ID1 for enet%d port%d.  Expected 0x%04x, read 0x%04x\n",
                       ethUnit,
                       phyUnit,
                       IP_PHY_ID1_EXPECTATION,
                       phyID1));
            return;
        }
    
        phyID2 = phy_reg_read(phyBase, phyAddr, IP_PHY_ID2);
        if ((phyID2 & IP_OUI_LSB_MASK) != IP_OUI_LSB_EXPECTATION) {
            DRV_PRINT(DRV_DEBUG_PHYERROR,
                      ("Invalid PHY ID2 for enet%d port %d.  Expected 0x%04x, read 0x%04x\n",
                       ethUnit,
                       phyUnit,
                       IP_OUI_LSB_EXPECTATION,
                       phyID2));
            return;
        }
    
        DRV_PRINT(DRV_DEBUG_PHYSETUP,
                  ("Found PHY enet%d port%d: model 0x%x revision 0x%x\n",
                   ethUnit,
                   phyUnit,
                   (phyID2 & IP_MODEL_NUM_MASK) >> IP_MODEL_NUM_SHIFT,
                   (phyID2 & IP_REV_NUM_MASK) >> IP_REV_NUM_SHIFT));
    
    }
}


/******************************************************************************
*
* ip_phySetup - reset and setup the PHY associated with
* the specified MAC unit number.
*
* Resets the associated PHY port.
*
* RETURNS:
*    TRUE  --> associated PHY is alive
*    FALSE --> no LINKs on this ethernet unit
*/

BOOL
ip_phySetup(int ethUnit)
{
    int     phyUnit;
    uint16_t  phyHwStatus;
    uint16_t  timeout;
    int     liveLinks = 0;
    uint32_t  phyBase = 0;
    BOOL    foundPhy = FALSE;
    uint32_t  phyAddr;
    
    /* Reset PHYs*/
    for (phyUnit=0; phyUnit < IP_PHY_MAX; phyUnit++) {
        if (!IP_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }

        phyBase = IP_PHYBASE(phyUnit);
        phyAddr = IP_PHYADDR(phyUnit);

        phy_reg_write(phyBase, phyAddr, IP_PHY_CONTROL,
                    IP_CTRL_SOFTWARE_RESET);
    }
    /*
     * After the phy is reset, it takes a little while before
     * it can respond properly.
     */
    mdelay(300);
    /* Verify that the switch is what we think it is, and that it's ready */
    ip_verifyReady(ethUnit);

    /* See if there's any configuration data for this enet */
    for (phyUnit=0; phyUnit < IP_PHY_MAX; phyUnit++) {
        if (IP_ETHUNIT(phyUnit) != ethUnit) {
            continue;
        }

        phyBase = IP_PHYBASE(phyUnit);
        foundPhy = TRUE;
        break;
    }

    if (!foundPhy) {
        return FALSE; /* No PHY's configured for this ethUnit */
    }

#ifdef COBRA_TODO
    /* Initialize global switch settings */

    /* Initialize the aging time */

    /* Set the learning properties */
#endif

    /* start auto negogiation on each phy */
    for (phyUnit=0; phyUnit < IP_PHY_MAX; phyUnit++) {
        if (!IP_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }

        phyBase = IP_PHYBASE(phyUnit);
        phyAddr = IP_PHYADDR(phyUnit);
        
        phy_reg_write(phyBase, phyAddr, IP_AUTONEG_ADVERT,
                                        IP_ADVERTISE_ALL);
        phy_reg_write(phyBase, phyAddr, IP_PHY_CONTROL,
                    IP_CTRL_AUTONEGOTIATION_ENABLE | IP_CTRL_START_AUTONEGOTIATION);
    }

    /*
     * Wait up to .75 seconds for ALL associated PHYs to finish
     * autonegotiation.  The only way we get out of here sooner is
     * if ALL PHYs are connected AND finish autonegotiation.
     */
    timeout=5;
    for (phyUnit=0; (phyUnit < IP_PHY_MAX) /*&& (timeout > 0) */; phyUnit++) {
        if (!IP_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }
        for (;;) {
            phyBase = IP_PHYBASE(phyUnit);
            phyAddr = IP_PHYADDR(phyUnit);

            phyHwStatus = phy_reg_read(phyBase, phyAddr, IP_PHY_STATUS);

            if (IP_AUTONEG_DONE(phyHwStatus)) {
                DRV_PRINT(DRV_DEBUG_PHYSETUP,
                          ("Port %d, Neg Success\n", phyUnit));
                break;
            }
            if (timeout == 0) {
                DRV_PRINT(DRV_DEBUG_PHYSETUP,
                          ("Port %d, Negogiation timeout\n", phyUnit));
                break;
            }
            if (--timeout == 0) {
                DRV_PRINT(DRV_DEBUG_PHYSETUP,
                          ("Port %d, Negogiation timeout\n", phyUnit));
                break;
            }

            mdelay(150);
        }
    }

    /*
     * All PHYs have had adequate time to autonegotiate.
     * Now initialize software status.
     *
     * It's possible that some ports may take a bit longer
     * to autonegotiate; but we can't wait forever.  They'll
     * get noticed by mv_phyCheckStatusChange during regular
     * polling activities.
     */
    for (phyUnit=0; phyUnit < IP_PHY_MAX; phyUnit++) {
        if (!IP_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }

        if (ip_phyIsLinkAlive(phyUnit)) {
            liveLinks++;
            IP_IS_PHY_ALIVE(phyUnit) = TRUE;
        } else {
            IP_IS_PHY_ALIVE(phyUnit) = FALSE;
        }

        DRV_PRINT(DRV_DEBUG_PHYSETUP,
            ("eth%d: Phy Status=%4.4x\n",
            ethUnit, 
            phy_reg_read(IP_PHYBASE(phyUnit),
                       IP_PHYADDR(phyUnit),
                       IP_PHY_STATUS)));
    }

    ip_VLANInit(ethUnit);

    return (liveLinks > 0);
}

/******************************************************************************
*
* ip_phyIsDuplexFull - Determines whether the phy ports associated with the
* specified device are FULL or HALF duplex.
*
* RETURNS:
*    1  --> FULL
*    0 --> HALF
*/
int
ip_phyIsFullDuplex(int ethUnit)
{
    int     phyUnit;
    uint32_t  phyBase;
    uint32_t  phyAddr;
    uint16_t  phyHwStatus;

    for (phyUnit=0; phyUnit < IP_PHY_MAX; phyUnit++) {
        if (!IP_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }

        if (ip_phyIsLinkAlive(phyUnit)) {

            phyBase = IP_PHYBASE(phyUnit);
            phyAddr = IP_PHYADDR(phyUnit);

            phyHwStatus = phy_reg_read(phyBase, phyAddr, IP_LINK_PARTNER_ABILITY);
            printk("ipPhy.c: phyHwStatus 0x%x\n",phyHwStatus);
            if ((phyHwStatus & IP_LINK_100BASETX_FULL_DUPLEX) || 
                (phyHwStatus & IP_LINK_10BASETX_FULL_DUPLEX)) {
                return TRUE;
            }
        }
        return -1;
    }

    return FALSE;

}


/******************************************************************************
*
* ip_phyIsSpeed100 - Determines the speed of phy ports associated with the
* specified device.
*
* RETURNS:
*    TRUE --> 100Mbit
*    FALSE --> 10Mbit
*/

BOOL
ip_phySpeed(int ethUnit)
{
    int     phyUnit;
    uint16_t  phyHwStatus;
    uint32_t  phyBase;
    uint32_t  phyAddr;

    for (phyUnit=0; phyUnit < IP_PHY_MAX; phyUnit++) {
        if (!IP_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }

        if (ip_phyIsLinkAlive(phyUnit)) {

            phyBase = IP_PHYBASE(phyUnit);
            phyAddr = IP_PHYADDR(phyUnit);

            phyHwStatus = phy_reg_read(phyBase, phyAddr, IP_LINK_PARTNER_ABILITY);

            if (phyHwStatus & IP_LINK_100BASETX) {
                return AG7100_PHY_SPEED_100TX;
            }
        }
    }

    return AG7100_PHY_SPEED_10T;
}

/*****************************************************************************
*
* ip_phyCheckStatusChange -- checks for significant changes in PHY state.
*
* A "significant change" is:
*     dropped link (e.g. ethernet cable unplugged) OR
*     autonegotiation completed + link (e.g. ethernet cable plugged in)
*
* When a PHY is plugged in, phyLinkGained is called.
* When a PHY is unplugged, phyLinkLost is called.
*/

void
ip_phyIsUp(int ethUnit)
{

    int           phyUnit;
    uint16_t        phyHwStatus;
    ipPhyInfo_t   *lastStatus;
    int           linkCount   = 0;
    int           lostLinks   = 0;
    int           gainedLinks = 0;
    uint32_t        phyBase;
    uint32_t        phyAddr;

    for (phyUnit=0; phyUnit < IP_PHY_MAX; phyUnit++) {
        if (!IP_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }

        phyBase = IP_PHYBASE(phyUnit);
        phyAddr = IP_PHYADDR(phyUnit);

        lastStatus = &ipPhyInfo[phyUnit];
        phyHwStatus = phy_reg_read(phyBase, phyAddr, IP_PHY_STATUS);

        if (lastStatus->isPhyAlive) { /* last known link status was ALIVE */
            /* See if we've lost link */
            if (phyHwStatus & IP_STATUS_LINK_PASS) {
                linkCount++;
            } else {
                lostLinks++;
#ifdef COBRA_TODO
                mv_flushATUDB(phyUnit);
#endif
                DRV_PRINT(DRV_DEBUG_PHYCHANGE,("\nenet%d port%d down\n",
                                               ethUnit, phyUnit));
                lastStatus->isPhyAlive = FALSE;
            }
        } else { /* last known link status was DEAD */
            /* Check for AutoNegotiation complete */
            if (IP_AUTONEG_DONE(phyHwStatus)) {
                gainedLinks++;
                linkCount++;
                DRV_PRINT(DRV_DEBUG_PHYCHANGE,("\nenet%d port%d up\n",
                                               ethUnit, phyUnit));
                lastStatus->isPhyAlive = TRUE;
            }
        }
    }

    return (linkCount);

#if 0
    if (linkCount == 0) {
        if (lostLinks) {
            /* We just lost the last link for this MAC */
            phyLinkLost(ethUnit);
        }
    } else {
        if (gainedLinks == linkCount) {
            /* We just gained our first link(s) for this MAC */
            phyLinkGained(ethUnit);
        }
    }
#endif
}

#if DEBUG

/* Define the registers of interest for a phyShow command */
typedef struct ipRegisterTableEntry_s {
    uint32_t regNum;
    char  *regIdString;
} ipRegisterTableEntry_t;

ipRegisterTableEntry_t ipPhyRegisterTable[] = {
    {IP_PHY_CONTROL,                 "PHY Control                     "},
    {IP_PHY_STATUS,                  "PHY Status                      "},
    {IP_PHY_ID1,                     "PHY Identifier 1                "},
    {IP_PHY_ID2,                     "PHY Identifier 2                "},
    {IP_AUTONEG_ADVERT,              "Auto-Negotiation Advertisement  "},
    {IP_LINK_PARTNER_ABILITY,        "Link Partner Ability            "},
    {IP_AUTONEG_EXPANSION,           "Auto-Negotiation Expansion      "},
};
int ipPhyNumRegs = sizeof(ipPhyRegisterTable) / sizeof(ipPhyRegisterTable[0]);


ipRegisterTableEntry_t ipPhy29GlobalRegisterTable[] = {
    {IP_GLOBAL_PHY29_18_REG,        "29_18_REG   "},
    {IP_GLOBAL_PHY29_19_REG,        "29_19_REG   "},
    {IP_GLOBAL_PHY29_20_REG,        "29_20_REG   "},
    {IP_GLOBAL_PHY29_21_REG,        "29_21_REG   "},
    {IP_GLOBAL_PHY29_22_REG,        "29_22_REG   "},
    {IP_GLOBAL_PHY29_23_REG,        "29_23_REG   "},
    {IP_GLOBAL_PHY29_24_REG,        "29_24_REG   "},
    {IP_GLOBAL_PHY29_25_REG,        "29_25_REG   "},
    {IP_GLOBAL_PHY29_26_REG,        "29_26_REG   "},
    {IP_GLOBAL_PHY29_27_REG,        "29_27_REG   "},
    {IP_GLOBAL_PHY29_28_REG,        "29_28_REG   "},
    {IP_GLOBAL_PHY29_29_REG,        "29_29_REG   "},
    {IP_GLOBAL_PHY29_30_REG,        "29_30_REG   "},
    {IP_GLOBAL_PHY29_31_REG,        "29_31_REG   "},
};
int ipPhy29GlobalNumRegs =
    sizeof(ipPhy29GlobalRegisterTable) / sizeof(ipPhy29GlobalRegisterTable[0]);


ipRegisterTableEntry_t ipPhy30GlobalRegisterTable[] = {
    {IP_GLOBAL_PHY30_0_REG,   "30_0_REG    "},
    {IP_GLOBAL_PHY30_1_REG,   "30_1_REG    "},
    {IP_GLOBAL_PHY30_2_REG,   "30_2_REG    "},
    {IP_GLOBAL_PHY30_3_REG,   "30_3_REG    "},
    {IP_GLOBAL_PHY30_4_REG,   "30_4_REG    "},
    {IP_GLOBAL_PHY30_5_REG,   "30_5_REG    "},
    {IP_GLOBAL_PHY30_6_REG,   "30_6_REG    "},
    {IP_GLOBAL_PHY30_7_REG,   "30_7_REG    "},
    {IP_GLOBAL_PHY30_8_REG,   "30_8_REG    "},
    {IP_GLOBAL_PHY30_9_REG,   "30_9_REG    "},
    {IP_GLOBAL_PHY30_10_REG,  "30_10_REG   "},
    {IP_GLOBAL_PHY30_11_REG,  "30_11_REG   "},
    {IP_GLOBAL_PHY30_12_REG,  "30_12_REG   "},
    {IP_GLOBAL_PHY30_13_REG,  "30_13_REG   "},
    {IP_GLOBAL_PHY30_16_REG,  "30_16_REG   "},
    {IP_GLOBAL_PHY30_17_REG,  "30_17_REG   "},
    {IP_GLOBAL_PHY30_18_REG,  "30_18_REG   "},
    {IP_GLOBAL_PHY30_20_REG,  "30_20_REG   "},
    {IP_GLOBAL_PHY30_21_REG,  "30_21_REG   "},
    {IP_GLOBAL_PHY30_22_REG,  "30_22_REG   "},
    {IP_GLOBAL_PHY30_23_REG,  "30_23_REG   "},
    {IP_GLOBAL_PHY30_24_REG,  "30_24_REG   "},
    {IP_GLOBAL_PHY30_25_REG,  "30_25_REG   "},
    {IP_GLOBAL_PHY30_26_REG,  "30_26_REG   "},
    {IP_GLOBAL_PHY30_27_REG,  "30_27_REG   "},
    {IP_GLOBAL_PHY30_28_REG,  "30_28_REG   "},
    {IP_GLOBAL_PHY30_29_REG,  "30_29_REG   "},
    {IP_GLOBAL_PHY30_30_REG,  "30_30_REG   "},
    {IP_GLOBAL_PHY30_31_REG,  "30_31_REG   "},
};
int ipPhy30GlobalNumRegs =
    sizeof(ipPhy30GlobalRegisterTable) / sizeof(ipPhy30GlobalRegisterTable[0]);

ipRegisterTableEntry_t ipPhy31GlobalRegisterTable[] = {
    {IP_GLOBAL_PHY31_0_REG,   "31_0_REG    "},
    {IP_GLOBAL_PHY31_1_REG,   "31_1_REG    "},
    {IP_GLOBAL_PHY31_2_REG,   "31_2_REG    "},
    {IP_GLOBAL_PHY31_3_REG,   "31_3_REG    "},
    {IP_GLOBAL_PHY31_4_REG,   "31_4_REG    "},
    {IP_GLOBAL_PHY31_5_REG,   "31_5_REG    "},
    {IP_GLOBAL_PHY31_6_REG,   "31_6_REG    "},
};

int ipPhy31GlobalNumRegs =
    sizeof(ipPhy31GlobalRegisterTable) / sizeof(ipPhy31GlobalRegisterTable[0]);


/*****************************************************************************
*
* ip_phyShow - Dump the state of a PHY.
* There are two sets of registers for each phy port:
*  "phy registers" and
*  "switch port registers"
* We dump 'em all, plus the switch global registers.
*/
void
ip_phyShow(int phyUnit)
{
    int     i;
    uint16_t  value;
    uint32_t  phyBase;
    uint32_t  phyAddr;

    if (!ip_validPhyId(phyUnit)) {
        return;
    }

    phyBase        = IP_PHYBASE(phyUnit);
    phyAddr        = IP_PHYADDR(phyUnit);

    printk("PHY state for PHY%d (enet%d, phyBase 0x%8x, phyAddr 0x%x)\n",
           phyUnit,
           IP_ETHUNIT(phyUnit),
           IP_PHYBASE(phyUnit),
           IP_PHYADDR(phyUnit));

    printk("PHY Registers:\n");
    for (i=0; i < ipPhyNumRegs; i++) {

        value = phy_reg_read(phyBase, phyAddr, ipPhyRegisterTable[i].regNum);

        printk("Reg %02d (0x%02x) %s = 0x%08x\n",
               ipPhyRegisterTable[i].regNum,
               ipPhyRegisterTable[i].regNum,
               ipPhyRegisterTable[i].regIdString,
               value);
    }

    phyBase = IP_GLOBALREGBASE;

    printk("Switch Global Registers:\n");
    printk("Phy29 Registers:\n");
    for (i=0; i < ipPhy29GlobalNumRegs; i++) {

        value = phy_reg_read(phyBase, IP_GLOBAL_PHY29_ADDR,
                           ipPhy29GlobalRegisterTable[i].regNum);

        printk("Reg %02d (0x%02x) %s = 0x%08x\n",
               ipPhy29GlobalRegisterTable[i].regNum,
               ipPhy29GlobalRegisterTable[i].regNum,
               ipPhy29GlobalRegisterTable[i].regIdString,
               value);
    }

    printk("Phy30 Registers:\n");
    for (i=0; i < ipPhy30GlobalNumRegs; i++) {

        value = phy_reg_read(phyBase, IP_GLOBAL_PHY30_ADDR,
                           ipPhy30GlobalRegisterTable[i].regNum);

        printk("Reg %02d (0x%02x) %s = 0x%08x\n",
               ipPhy30GlobalRegisterTable[i].regNum,
               ipPhy30GlobalRegisterTable[i].regNum,
               ipPhy30GlobalRegisterTable[i].regIdString,
               value);
    }
    printk("Phy31 Registers:\n");
    for (i=0; i < ipPhy31GlobalNumRegs; i++) {

        value = phy_reg_read(phyBase, IP_GLOBAL_PHY31_ADDR,
                           ipPhy31GlobalRegisterTable[i].regNum);

        printk("Reg %02d (0x%02x) %s = 0x%08x\n",
               ipPhy31GlobalRegisterTable[i].regNum,
               ipPhy31GlobalRegisterTable[i].regNum,
               ipPhy31GlobalRegisterTable[i].regIdString,
               value);
    }
}

/*****************************************************************************
*
* ip_phySet - Modify the value of a PHY register (debug only).
*/
void
ip_phySet(int phyUnit, uint32_t regnum, uint32_t value)
{
    uint32_t  phyBase;
    uint32_t  phyAddr;

    if (ip_validPhyId(phyUnit)) {

        phyBase = IP_PHYBASE(phyUnit);
        phyAddr = IP_PHYADDR(phyUnit);

        phy_reg_write(phyBase, phyAddr, regnum, value);
    }
}

/*****************************************************************************
*
* ip_globalSet - Modify the value of a global register
* (debug only).
*/
void
ip_globalSet(uint32_t phyAddr, uint32_t regnum, uint32_t value)
{
    uint32_t  phyBase;

    phyBase = IP_GLOBALREGBASE;

    phy_reg_write(phyBase, phyAddr, regnum, value);
}


#endif
