/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright © 2003 Atheros Communications, Inc.,  All Rights Reserved.
 */

/*
 * Manage the ADMTEK ethernet PHY.
 *
 * All definitions in this file are operating system independent!
 */


#include "ag7240_phy.h"
#include  "adm_phy.h"

#ifdef CONFIG_ADM6996FC_PHY
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

#if DRV_DEBUG
#define DRV_DEBUG_PHYERROR  0x00000001
#define DRV_DEBUG_PHYCHANGE 0x00000002
#define DRV_DEBUG_PHYSETUP  0x00000004

int admPhyDebug = DRV_DEBUG_PHYERROR;

#define DRV_LOG(FLG, X0, X1, X2, X3, X4, X5, X6)    \
{                                                   \
    if (admPhyDebug & (FLG)) {                      \
        logMsg(X0, X1, X2, X3, X4, X5, X6);         \
    }                                               \
}

#define DRV_MSG(x,a,b,c,d,e,f)                      \
    logMsg(x,a,b,c,d,e,f)

#define DRV_PRINT(FLG, X)                           \
{                                                   \
    if (admPhyDebug & (FLG)) {                      \
        diag_printf X;                              \
    }                                               \
}

#else /* !DRV_DEBUG */
#define DRV_LOG(DBG_SW, X0, X1, X2, X3, X4, X5, X6)
#define DRV_MSG(x,a,b,c,d,e,f)
#define DRV_PRINT(DBG_SW,X)
#endif


#define ADM_LAN_PORT_VLAN          1
#define ADM_WAN_PORT_VLAN          2

#define ENET_UNIT_DEFAULT 1

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
} admPhyInfo_t;

#define ADM_PHY0_ADDR   0x10
#define ADM_PHY1_ADDR   0x11
#define ADM_PHY2_ADDR   0x12
#define ADM_PHY3_ADDR   0x13
#define ADM_PHY4_ADDR   0x14

#define P0_TXL  0xcc
#define P5_TXL  0xdc

#define P0_TXH  0xcd
#define P5_TXH  0xdd

#define P0_TXBL 0xde
#define P5_TXBL 0xee

#define P0_TXBH 0xdf
#define P5_TXBH 0xef

#define P0_RXL  0xac
#define P5_RXL  0xb8

#define P0_RXH  0xa9
#define P5_RXH  0xb9

#define P0_ERRL 0x102
#define P5_ERRL 0x112

#define P0_ERRH 0x103
#define P5_ERRH 0x113

/*
 * Per-PHY information, indexed by PHY unit number.
 */
admPhyInfo_t admPhyInfo[] = {
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
     ADM_PHY0_ADDR,
     ADM_LAN_PORT_VLAN
    },

    {TRUE,   /* phy port 1 -- LAN port 1 */
     FALSE,
     ENET_UNIT_DEFAULT,
     0,
     ADM_PHY1_ADDR,
     ADM_LAN_PORT_VLAN
    },

    {TRUE,   /* phy port 2 -- LAN port 2 */
     FALSE,
     ENET_UNIT_DEFAULT,
     0,
     ADM_PHY2_ADDR, 
     ADM_LAN_PORT_VLAN
    },

    {TRUE,   /* phy port 3 -- LAN port 3 */
     FALSE,
     ENET_UNIT_DEFAULT,
     0,
     ADM_PHY3_ADDR, 
     ADM_LAN_PORT_VLAN
    },

    {TRUE,   /* phy port 4 -- WAN port or LAN port 4 */
     FALSE,
     0,
     0,
     ADM_PHY4_ADDR, 
     ADM_LAN_PORT_VLAN   /* Send to all ports */
    },

    {FALSE,  /* phy port 5 -- CPU port (no RJ45 connector) */
     TRUE,
     ENET_UNIT_DEFAULT,
     0,
     0x00, 
     ADM_LAN_PORT_VLAN    /* Send to all ports */
    },
};

#define ADM_GLOBALREGBASE    0

//#define ADM_PHY_MAX (sizeof(admPhyInfo) / sizeof(admPhyInfo[0]))
#define ADM_PHY_MAX 5

/* Range of valid PHY IDs is [MIN..MAX] */
#define ADM_ID_MIN 0
#define ADM_ID_MAX (ADM_PHY_MAX-1)

/* Convenience macros to access myPhyInfo */
#define ADM_IS_ENET_PORT(phyUnit) (admPhyInfo[phyUnit].isEnetPort)
#define ADM_IS_PHY_ALIVE(phyUnit) (admPhyInfo[phyUnit].isPhyAlive)
#define ADM_ETHUNIT(phyUnit) (admPhyInfo[phyUnit].ethUnit)
#define ADM_PHYBASE(phyUnit) (admPhyInfo[phyUnit].phyBase)
#define ADM_PHYADDR(phyUnit) (admPhyInfo[phyUnit].phyAddr)
#define ADM_VLAN_TABLE_SETTING(phyUnit) (admPhyInfo[phyUnit].VLANTableSetting)


/*#define ADM_IS_ETHUNIT(phyUnit, ethUnit) \
            (ADM_IS_ENET_PORT(phyUnit) &&        \
            ADM_ETHUNIT(phyUnit) == (ethUnit))*/
#define ADM_IS_ETHUNIT(_x, _y)   1

/* Forward references */
BOOL       adm_phyIsLinkAlive(int phyUnit);
static void adm_VLANInit(int ethUnit);
static void adm_verifyReady(int ethUnit);

/******************************************************************************
*
* adm_phyIsLinkAlive - test to see if the specified link is alive
*
* RETURNS:
*    TRUE  --> link is alive
*    FALSE --> link is down
*/
BOOL
adm_phyIsLinkAlive(int phyUnit)
{
    uint16_t phyHwStatus;
    uint32_t phyBase;
    uint32_t phyAddr;

    phyBase = ADM_PHYBASE(phyUnit);
    phyAddr = ADM_PHYADDR(phyUnit);

    phyHwStatus = phy_reg_read(phyBase, phyAddr, ADM_PHY_STATUS);

    if (phyHwStatus & ADM_STATUS_LINK_PASS) {
        return TRUE;
    } else {
        return FALSE;
    }
}


/******************************************************************************
*
* adm_phySetup - reset and setup the PHY associated with
* the specified MAC unit number.
*
* Resets the associated PHY port.
*
* RETURNS:
*    TRUE  --> associated PHY is alive
*    FALSE --> no LINKs on this ethernet unit
*/

BOOL
adm_phySetup(int ethUnit)
{
    int     phyUnit, global;
    uint16_t  phyHwStatus;
    uint16_t  timeout;
    int     liveLinks = 0;
    uint32_t  phyBase = 0;
    BOOL    foundPhy = FALSE;
    uint32_t  phyAddr;
    static int inited = 0;
    
    /* Reset PHYs*/
    for (phyUnit=0; phyUnit < ADM_PHY_MAX; phyUnit++) {
        if (!ADM_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }

        phyBase = ADM_PHYBASE(phyUnit);
        phyAddr = ADM_PHYADDR(phyUnit);

        phy_reg_write(phyBase, phyAddr, ADM_PHY_CONTROL,
                    ADM_CTRL_SOFTWARE_RESET);
    }
    /*
     * After the phy is reset, it takes a little while before
     * it can respond properly.
     */
    sysMsDelay(300);

    /* See if there's any configuration data for this enet */
    for (phyUnit=0; phyUnit < ADM_PHY_MAX; phyUnit++) {
        if (!ADM_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }

        phyBase = ADM_PHYBASE(phyUnit);
        foundPhy = TRUE;
        break;
    }

    if (!foundPhy) {
        return FALSE; /* No PHY's configured for this ethUnit */
    }

    /* start auto negogiation on each phy */
    for (phyUnit=0; phyUnit < ADM_PHY_MAX; phyUnit++) {
        if (!ADM_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }

        phyBase = ADM_PHYBASE(phyUnit);
        phyAddr = ADM_PHYADDR(phyUnit);
        
        phy_reg_write(phyBase, phyAddr, ADM_AUTONEG_ADVERT,
                                        ADM_ADVERTISE_ALL);

        phy_reg_write(phyBase, phyAddr, ADM_PHY_CONTROL,
                    ADM_CTRL_AUTONEGOTIATION_ENABLE | ADM_CTRL_START_AUTONEGOTIATION);
    }

    /*
     * Wait up to .75 seconds for ALL associated PHYs to finish
     * autonegotiation.  The only way we get out of here sooner is
     * if ALL PHYs are connected AND finish autonegotiation.
     */
    timeout=5;
    for (phyUnit=0; (phyUnit < ADM_PHY_MAX) /*&& (timeout > 0) */; phyUnit++) {
        if (!ADM_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }
        for (;;) {
            phyBase = ADM_PHYBASE(phyUnit);
            phyAddr = ADM_PHYADDR(phyUnit);

            phyHwStatus = phy_reg_read(phyBase, phyAddr, ADM_PHY_STATUS);

            if (ADM_AUTONEG_DONE(phyHwStatus)) {
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

            sysMsDelay(150);
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
    for (phyUnit=0; phyUnit < ADM_PHY_MAX; phyUnit++) {
        if (!ADM_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }

        if (adm_phyIsLinkAlive(phyUnit)) {
            liveLinks++;
            ADM_IS_PHY_ALIVE(phyUnit) = TRUE;
        } else {
            ADM_IS_PHY_ALIVE(phyUnit) = FALSE;
        }

        DRV_PRINT(DRV_DEBUG_PHYSETUP,
            ("eth%d: Phy Status=%4.4x\n",
            ethUnit, 
            phy_reg_read(ADM_PHYBASE(phyUnit),
                       ADM_PHYADDR(phyUnit),
                       ADM_PHY_STATUS)));
    }

    /*
     * XXX
     */
    phy_reg_write(0, 0, 0x10, 0x50);
    return (liveLinks > 0);
}

/******************************************************************************
*
* adm_phyIsDuplexFull - Determines whether the phy ports associated with the
* specified device are FULL or HALF duplex.
*
* RETURNS:
*    1  --> FULL
*    0 --> HALF
*/
int
adm_phyIsFullDuplex(int ethUnit)
{
    int     phyUnit;
    uint32_t  phyBase;
    uint32_t  phyAddr;
    uint16_t  phyHwStatus;

    for (phyUnit=0; phyUnit < ADM_PHY_MAX; phyUnit++) {
        if (!ADM_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }

        if (adm_phyIsLinkAlive(phyUnit)) {

            phyBase = ADM_PHYBASE(phyUnit);
            phyAddr = ADM_PHYADDR(phyUnit);

            phyHwStatus = phy_reg_read(phyBase, phyAddr, ADM_LINK_PARTNER_ABILITY);
            if ((phyHwStatus & ADM_LINK_100BASETX_FULL_DUPLEX) || 
                (phyHwStatus & ADM_LINK_10BASETX_FULL_DUPLEX)) {
                return TRUE;
            }
        }
        return -1;
    }

    return FALSE;

}


/******************************************************************************
*
* adm_phyIsSpeed100 - Determines the speed of phy ports associated with the
* specified device.
*
* RETURNS:
*    TRUE --> 100Mbit
*    FALSE --> 10Mbit
*/

BOOL
adm_phySpeed(int ethUnit)
{
    int     phyUnit;
    uint16_t  phyHwStatus;
    uint32_t  phyBase;
    uint32_t  phyAddr;

    for (phyUnit=0; phyUnit < ADM_PHY_MAX; phyUnit++) {
        if (!ADM_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }

        if (adm_phyIsLinkAlive(phyUnit)) {

            phyBase = ADM_PHYBASE(phyUnit);
            phyAddr = ADM_PHYADDR(phyUnit);

            phyHwStatus = phy_reg_read(phyBase, phyAddr, ADM_LINK_PARTNER_ABILITY);

            if (phyHwStatus & ADM_LINK_100BASETX) {
                return AG7100_PHY_SPEED_100TX;
            }
        }
    }

    return AG7100_PHY_SPEED_10T;
}

/*****************************************************************************
*
* adm_phyCheckStatusChange -- checks for significant changes in PHY state.
*
* A "significant change" is:
*     dropped link (e.g. ethernet cable unplugged) OR
*     autonegotiation completed + link (e.g. ethernet cable plugged in)
*
* When a PHY is plugged in, phyLinkGained is called.
* When a PHY is unplugged, phyLinkLost is called.
*/

int
adm_phyIsUp(int ethUnit)
{

    int           phyUnit;
    uint16_t        phyHwStatus;
    admPhyInfo_t   *lastStatus;
    int           linkCount   = 0;
    int           lostLinks   = 0;
    int           gainedLinks = 0;
    uint32_t        phyBase;
    uint32_t        phyAddr;

    for (phyUnit=0; phyUnit < ADM_PHY_MAX; phyUnit++) {
        if (!ADM_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }

        phyBase = ADM_PHYBASE(phyUnit);
        phyAddr = ADM_PHYADDR(phyUnit);


        lastStatus = &admPhyInfo[phyUnit];
        phyHwStatus = phy_reg_read(phyBase, phyAddr, ADM_PHY_STATUS);

        if (lastStatus->isPhyAlive) { /* last known link status was ALIVE */
            /* See if we've lost link */
            if (phyHwStatus & ADM_STATUS_LINK_PASS) {
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
            if (ADM_AUTONEG_DONE(phyHwStatus)) {
                //printk("autoneg done\n");
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

#define adm_counter_parse(_reg, _cnt, _cnthi) do {   \
  _cnt   = phy_reg_read(0, ((_reg##L & (0x1f << 5)) >> 5), (_reg##L & 0x1f)); \
  _cnthi = phy_reg_read(0, ((_reg##H & (0x1f << 5)) >> 5), (_reg##H & 0x1f)); \
}while(0);

void
adm_get_counters()
{
    int count, counthi;

    adm_counter_parse(P0_TX, count, counthi);
    //printk("P0 Tx: %10d ", (counthi << 16)|count);

    adm_counter_parse(P0_TXB, count, counthi);
    //printk("P0 TxB: %#x ", (counthi << 16)|count);

    adm_counter_parse(P0_RX, count, counthi);
    //printk("P0 Rx: %10d ", (counthi << 16)|count);

    adm_counter_parse(P0_ERR, count, counthi);
    //printk("P0 ERR: %10d\n", (counthi << 16)|count);

    adm_counter_parse(P5_TX, count, counthi);
    //printk("P5 Tx: %10d ", (counthi << 16)|count);

    adm_counter_parse(P5_TXB, count, counthi);
    //printk("P5 TxB: %#x ", (counthi << 16)|count);

    adm_counter_parse(P5_RX, count, counthi);
    //printk("P5 Rx: %10d ", (counthi << 16)|count);

    adm_counter_parse(P5_ERR, count, counthi);
    //printk("P5 ERR: %10d\n", (counthi << 16)|count);

}


#endif