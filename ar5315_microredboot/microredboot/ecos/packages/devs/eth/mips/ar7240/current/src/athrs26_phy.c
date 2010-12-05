/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright © 2007 Atheros Communications, Inc.,  All Rights Reserved.
 */

/*
 * Manage the atheros ethernet PHY.
 *
 * All definitions in this file are operating system independent!
 */

#include "ag7240.h"
#include "ag7240_phy.h"

#ifdef CONFIG_ATHRS26_PHY
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

#define DRV_LOG(DBG_SW, X0, X1, X2, X3, X4, X5, X6)
#define DRV_MSG(x,a,b,c,d,e,f)
#define DRV_PRINT(DBG_SW,X)

#define DRV_DEBUG_PHYERROR  0x00000001
#define DRV_DEBUG_PHYCHANGE 0x00000002
#define DRV_DEBUG_PHYSETUP  0x00000004

#define ATHR_LAN_PORT_VLAN          1
#define ATHR_WAN_PORT_VLAN          2

#define ENET_UNIT_LAN 0

#define TRUE    1
#define FALSE   0

#define ATHR_PHY0_ADDR   0x0
#define ATHR_PHY1_ADDR   0x1
#define ATHR_PHY2_ADDR   0x2
#define ATHR_PHY3_ADDR   0x3
#define ATHR_PHY4_ADDR   0x4

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
} athrPhyInfo_t;

/*
 * Per-PHY information, indexed by PHY unit number.
 */
static athrPhyInfo_t athrPhyInfo[] = {
    {TRUE,   /* phy port 0 -- LAN port 0 */
     FALSE,
     ENET_UNIT_LAN,
     0,
     ATHR_PHY0_ADDR,
     ATHR_LAN_PORT_VLAN
    },

    {TRUE,   /* phy port 1 -- LAN port 1 */
     FALSE,
     ENET_UNIT_LAN,
     0,
     ATHR_PHY1_ADDR,
     ATHR_LAN_PORT_VLAN
    },

    {TRUE,   /* phy port 2 -- LAN port 2 */
     FALSE,
     ENET_UNIT_LAN,
     0,
     ATHR_PHY2_ADDR, 
     ATHR_LAN_PORT_VLAN
    },

    {TRUE,   /* phy port 3 -- LAN port 3 */
     FALSE,
     ENET_UNIT_LAN,
     0,
     ATHR_PHY3_ADDR, 
     ATHR_LAN_PORT_VLAN
    },

    {TRUE,   /* phy port 4 -- WAN port or LAN port 4 */
     FALSE,
     1,
     0,
     ATHR_PHY4_ADDR, 
     ATHR_LAN_PORT_VLAN   /* Send to all ports */
    },

    {FALSE,  /* phy port 5 -- CPU port (no RJ45 connector) */
     TRUE,
     ENET_UNIT_LAN,
     0,
     0x00, 
     ATHR_LAN_PORT_VLAN    /* Send to all ports */
    },
};

static uint8_t athr26_init_flag = 0;
#define ATHR_GLOBALREGBASE    0

//#define ATHR_PHY_MAX (sizeof(athrPhyInfo) / sizeof(athrPhyInfo[0]))
#define ATHR_PHY_MAX 5

/* Range of valid PHY IDs is [MIN..MAX] */
#define ATHR_ID_MIN 0
#define ATHR_ID_MAX (ATHR_PHY_MAX-1)

/* Convenience macros to access myPhyInfo */
#define ATHR_IS_ENET_PORT(phyUnit) (athrPhyInfo[phyUnit].isEnetPort)
#define ATHR_IS_PHY_ALIVE(phyUnit) (athrPhyInfo[phyUnit].isPhyAlive)
#define ATHR_ETHUNIT(phyUnit) (athrPhyInfo[phyUnit].ethUnit)
#define ATHR_PHYBASE(phyUnit) (athrPhyInfo[phyUnit].phyBase)
#define ATHR_PHYADDR(phyUnit) (athrPhyInfo[phyUnit].phyAddr)
#define ATHR_VLAN_TABLE_SETTING(phyUnit) (athrPhyInfo[phyUnit].VLANTableSetting)


#define ATHR_IS_ETHUNIT(phyUnit, ethUnit) \
            (ATHR_IS_ENET_PORT(phyUnit) &&        \
            ATHR_ETHUNIT(phyUnit) == (ethUnit))

#define ATHR_IS_WAN_PORT(phyUnit) (!(ATHR_ETHUNIT(phyUnit)==ENET_UNIT_LAN))
            
/* Forward references */
BOOL athrs26_phy_is_link_alive(int phyUnit);
static uint32_t athrs26_reg_read(uint16_t reg_addr);
static void athrs26_reg_write(uint16_t reg_addr, uint32_t reg_val);

#define get_field_val(_reg, _mask, _shift, _res_reg)     \
    do { \
        unsigned int temp;	\ 
        temp = ar7240_reg_rd(_reg); \
        temp &= (unsigned int)_mask;\
        _res_reg  = temp >> _shift; \
    } while (0)

#define set_field_val(_reg, _mask, _shift, _val)                \
    do { \
        unsigned int temp; \
        temp = ar7240_reg_rd(_reg); \
        temp &= ~_mask;  \
        temp |= _val << _shift;  \
        ar7240_reg_wr(_reg, temp);\
    } while (0)


void athrs26_reg_init()
{
    if (athr26_init_flag)
        return;

    athrs26_reg_write(0x200, 0x200);
    athrs26_reg_write(0x300, 0x200);
    athrs26_reg_write(0x400, 0x200);
    athrs26_reg_write(0x500, 0x200);
    athrs26_reg_write(0x600, 0x7d);

#ifdef S26_VER_1_0
    phy_reg_write(0, 0, 29, 41);
    phy_reg_write(0, 0, 30, 0);
    phy_reg_write(0, 1, 29, 41);
    phy_reg_write(0, 1, 30, 0);
    phy_reg_write(0, 2, 29, 41);
    phy_reg_write(0, 2, 30, 0);
    phy_reg_write(0, 3, 29, 41);
    phy_reg_write(0, 3, 30, 0);
    phy_reg_write(0, 4, 29, 41);
    phy_reg_write(0, 4, 30, 0);
#endif
        
    athrs26_reg_write(0x38, 0xc000050e);
        
#ifdef HEADER_EN        
    athrs26_reg_write(0x104, 0x4804);
#else
    athrs26_reg_write(0x104, 0x4004);
#endif
       
    athrs26_reg_write(0x60, 0xffffffff);
    athrs26_reg_write(0x64, 0xaaaaaaaa);
    athrs26_reg_write(0x68, 0x55555555);    
    athrs26_reg_write(0x6c, 0x0);    

    athrs26_reg_write(0x70, 0x41af);

    athr26_init_flag = 1;
}

static unsigned int phy_val_saved = 0;
/******************************************************************************
*
* athrs26_phy_off - power off the phy to change its speed
*
* Power off the phy
*/
void athrs26_phy_off(ag7100_mac_t *mac)
{
    if (mac->mac_unit == ENET_UNIT_LAN)
        return;

    phy_val_saved = phy_reg_read(0, ATHR_PHY4_ADDR, ATHR_PHY_CONTROL);
    phy_reg_write(0, ATHR_PHY4_ADDR, ATHR_PHY_CONTROL, phy_val_saved | 0x800);
}

/******************************************************************************
*
* athrs26_phy_on - power on the phy after speed changed
*
* Power on the phy
*/
void athrs26_phy_on(ag7100_mac_t *mac)
{
    if ((mac->mac_unit == ENET_UNIT_LAN) || (phy_val_saved == 0))
        return;
        
    phy_reg_write(0, ATHR_PHY4_ADDR, ATHR_PHY_CONTROL, phy_val_saved & 0xf7ff);
}

/******************************************************************************
*
* athrs26_mac_speed_set - set mac in s26 speed mode (actually RMII mode)
*
* Set mac speed mode
*/
void athrs26_mac_speed_set(ag7100_mac_t *mac, ag7100_phy_speed_t speed)
{
    uint32_t reg_val;
    
    if ((mac->mac_unit == ENET_UNIT_LAN))
        return;
    
    reg_val = athrs26_reg_read(0x600);
    
    switch (speed) {
        case AG7100_PHY_SPEED_100TX:
            athrs26_reg_write (0x600, (reg_val & 0xfffffffc) | 0x1);
            break;
           
        case AG7100_PHY_SPEED_10T:
            athrs26_reg_write (0x600, (reg_val & 0xfffffffc));
            break; 
        
        default:
            break;  
    }
}

/******************************************************************************
*
* athrs26_phy_is_link_alive - test to see if the specified link is alive
*
* RETURNS:
*    TRUE  --> link is alive
*    FALSE --> link is down
*/
BOOL
athrs26_phy_is_link_alive(int phyUnit)
{
    uint16_t phyHwStatus;
    uint32_t phyBase;
    uint32_t phyAddr;

    phyBase = ATHR_PHYBASE(phyUnit);
    phyAddr = ATHR_PHYADDR(phyUnit);

    phyHwStatus = phy_reg_read(phyBase, phyAddr, ATHR_PHY_SPEC_STATUS);

    if (phyHwStatus & ATHR_STATUS_LINK_PASS)
        return TRUE;

    return FALSE;
}


/******************************************************************************
*
* athrs26_phy_setup - reset and setup the PHY associated with
* the specified MAC unit number.
*
* Resets the associated PHY port.
*
* RETURNS:
*    TRUE  --> associated PHY is alive
*    FALSE --> no LINKs on this ethernet unit
*/

BOOL
athrs26_phy_setup(int ethUnit)
{
    int         phyUnit;
    uint16_t    phyHwStatus;
    uint16_t    timeout;
    int         liveLinks = 0;
    uint32_t    phyBase = 0;
    BOOL        foundPhy = FALSE;
    uint32_t    phyAddr = 0;
    

    /* See if there's any configuration data for this enet */
    /* start auto negogiation on each phy */
    for (phyUnit=0; phyUnit < ATHR_PHY_MAX; phyUnit++) {
        if (!ATHR_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }

        foundPhy = TRUE;
        phyBase = ATHR_PHYBASE(phyUnit);
        phyAddr = ATHR_PHYADDR(phyUnit);
        
        phy_reg_write(phyBase, phyAddr, ATHR_AUTONEG_ADVERT,
                      ATHR_ADVERTISE_ALL);

        /* Reset PHYs*/
        phy_reg_write(phyBase, phyAddr, ATHR_PHY_CONTROL,
                      ATHR_CTRL_AUTONEGOTIATION_ENABLE 
                      | ATHR_CTRL_SOFTWARE_RESET);
    }

    if (!foundPhy) {
        return FALSE; /* No PHY's configured for this ethUnit */
    }
    
    /*
     * After the phy is reset, it takes a little while before
     * it can respond properly.
     */
    sysMsDelay(1000);
    
    /*
     * Wait up to .75 seconds for ALL associated PHYs to finish
     * autonegotiation.  The only way we get out of here sooner is
     * if ALL PHYs are connected AND finish autonegotiation.
     */
    for (phyUnit=0; (phyUnit < ATHR_PHY_MAX) /*&& (timeout > 0) */; phyUnit++) {
        if (!ATHR_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }

        timeout=20;
        for (;;) {
            phyHwStatus = phy_reg_read(phyBase, phyAddr, ATHR_PHY_CONTROL);

            if (ATHR_RESET_DONE(phyHwStatus)) {
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
    for (phyUnit=0; phyUnit < ATHR_PHY_MAX; phyUnit++) {
        if (!ATHR_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }

        if (athrs26_phy_is_link_alive(phyUnit)) {
            liveLinks++;
            ATHR_IS_PHY_ALIVE(phyUnit) = TRUE;
        } else {
            ATHR_IS_PHY_ALIVE(phyUnit) = FALSE;
        }

        DRV_PRINT(DRV_DEBUG_PHYSETUP,
            ("eth%d: Phy Specific Status=%4.4x\n",
            ethUnit, 
            phy_reg_read(ATHR_PHYBASE(phyUnit),
                       ATHR_PHYADDR(phyUnit),
                       ATHR_PHY_SPEC_STATUS)));
    }
    
    return (liveLinks > 0);
}

/******************************************************************************
*
* athrs26_phy_is_fdx - Determines whether the phy ports associated with the
* specified device are FULL or HALF duplex.
*
* RETURNS:
*    1 --> FULL
*    0 --> HALF
*/
int
athrs26_phy_is_fdx(int ethUnit)
{
    int         phyUnit;
    uint32_t    phyBase;
    uint32_t    phyAddr;
    uint16_t    phyHwStatus;
    int         ii = 200;
    
    if (ethUnit == ENET_UNIT_LAN)
        return TRUE;
    
    for (phyUnit=0; phyUnit < ATHR_PHY_MAX; phyUnit++) {
        if (!ATHR_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }

        if (athrs26_phy_is_link_alive(phyUnit)) {

            phyBase = ATHR_PHYBASE(phyUnit);
            phyAddr = ATHR_PHYADDR(phyUnit);

            do {
                phyHwStatus = ag7100_mii_read(phyBase, phyAddr, 
                                              ATHR_PHY_SPEC_STATUS);
        	sysMsDelay(10);
            } while((!(phyHwStatus & ATHR_STATUS_RESOVLED)) && --ii);

            if (phyHwStatus & ATHER_STATUS_FULL_DEPLEX)
                return TRUE;
        }
    }

    return FALSE;
}


/******************************************************************************
*
* athrs26_phy_speed - Determines the speed of phy ports associated with the
* specified device.
*
* RETURNS:
*               AG7100_PHY_SPEED_10T, AG7100_PHY_SPEED_100TX;
*               AG7100_PHY_SPEED_1000T;
*/

int
athrs26_phy_speed(int ethUnit)
{
    int         phyUnit;
    uint16_t    phyHwStatus;
    uint32_t    phyBase;
    uint32_t    phyAddr;
    int         ii = 200;
    
    if (ethUnit == ENET_UNIT_LAN)
        return AG7100_PHY_SPEED_100TX;

    for (phyUnit=0; phyUnit < ATHR_PHY_MAX; phyUnit++) {
        if (!ATHR_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }

        if (athrs26_phy_is_link_alive(phyUnit)) {

            phyBase = ATHR_PHYBASE(phyUnit);
            phyAddr = ATHR_PHYADDR(phyUnit);
            
            do {
                phyHwStatus = ag7100_mii_read(phyBase, phyAddr, 
                                              ATHR_PHY_SPEC_STATUS);
                sysMsDelay(10);
            }while((!(phyHwStatus & ATHR_STATUS_RESOVLED)) && --ii);

            phyHwStatus = ((phyHwStatus & ATHER_STATUS_LINK_MASK) >>
                           ATHER_STATUS_LINK_SHIFT);

            switch(phyHwStatus) {
            case 0:
                return AG7100_PHY_SPEED_10T;
            case 1:
                return AG7100_PHY_SPEED_100TX;
            case 2:
                return AG7100_PHY_SPEED_1000T;
            default:
                AG7100_PRINT(AG7100_DEBUG_ERROR, ("Unkown speed read!\n"));
            }
        }
    }

    return AG7100_PHY_SPEED_10T;
}

/*****************************************************************************
*
* athr_phy_is_up -- checks for significant changes in PHY state.
*
* A "significant change" is:
*     dropped link (e.g. ethernet cable unplugged) OR
*     autonegotiation completed + link (e.g. ethernet cable plugged in)
*
* When a PHY is plugged in, phyLinkGained is called.
* When a PHY is unplugged, phyLinkLost is called.
*/

int
athrs26_phy_is_up(int ethUnit)
{
    int             phyUnit;
    uint16_t      phyHwStatus, phyHwControl;
    athrPhyInfo_t  *lastStatus;
    int             linkCount   = 0;
    int             lostLinks   = 0;
    int             gainedLinks = 0;
    uint32_t        phyBase;
    uint32_t        phyAddr;

    for (phyUnit=0; phyUnit < ATHR_PHY_MAX; phyUnit++) {
        if (!ATHR_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }

        phyBase = ATHR_PHYBASE(phyUnit);
        phyAddr = ATHR_PHYADDR(phyUnit);

        lastStatus = &athrPhyInfo[phyUnit];

        if (lastStatus->isPhyAlive) { /* last known link status was ALIVE */
            phyHwStatus = phy_reg_read(phyBase, phyAddr, ATHR_PHY_SPEC_STATUS);

            /* See if we've lost link */
            if (phyHwStatus & ATHR_STATUS_LINK_PASS) {
                linkCount++;
            } else {
                lostLinks++;
                DRV_PRINT(DRV_DEBUG_PHYCHANGE,("\nenet%d port%d down\n",
                                               ethUnit, phyUnit));
                lastStatus->isPhyAlive = FALSE;
            }
        } else { /* last known link status was DEAD */
            /* Check for reset complete */
            phyHwStatus = phy_reg_read(phyBase, phyAddr, ATHR_PHY_STATUS);
            if (!ATHR_RESET_DONE(phyHwStatus))
                continue;

            phyHwControl = phy_reg_read(phyBase, phyAddr, ATHR_PHY_CONTROL);
            /* Check for AutoNegotiation complete */            
            if ((!(phyHwControl & ATHR_CTRL_AUTONEGOTIATION_ENABLE)) 
                 || ATHR_AUTONEG_DONE(phyHwStatus)) {
                phyHwStatus = phy_reg_read(phyBase, phyAddr, 
                                           ATHR_PHY_SPEC_STATUS);

                if (phyHwStatus & ATHR_STATUS_LINK_PASS) {
                gainedLinks++;
                linkCount++;
                DRV_PRINT(DRV_DEBUG_PHYCHANGE,("\nenet%d port%d up\n",
                                               ethUnit, phyUnit));
                lastStatus->isPhyAlive = TRUE;
                }
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

static uint32_t
athrs26_reg_read(uint16_t reg_addr)
{
    uint16_t reg_word_addr;
    uint32_t phy_addr, phy_val0, phy_val1, phy_val;
    uint8_t  phy_reg; 
    
    /* read the first 16 bits*/
    reg_word_addr = (reg_addr / 4) * 2;
    /* configure register high address */
    phy_addr = 0x18;
    phy_reg = 0x0;
    phy_val0 = (reg_word_addr >> 8) & 0x1ff;         /* bit16-8 of reg address*/
    phy_reg_write (0, phy_addr, phy_reg, phy_val0);

    /* read register with low address */
    phy_addr = 0x10 | ((reg_word_addr >> 5) & 0x7); /* bit7-5 of reg address */
    phy_reg = reg_word_addr & 0x1f;                 /* bit4-0 of reg address */
    phy_val0 = phy_reg_read(0, phy_addr, phy_reg);

    /* read the second 16 bits*/
    reg_word_addr++;
    /* configure register high address */
    phy_addr = 0x18;
    phy_reg = 0x0;
    phy_val1 = (reg_word_addr >> 8) & 0x1ff;         /* bit16-8 of reg address*/
    phy_reg_write (0, phy_addr, phy_reg, phy_val1);

    /* read register with low address */
    phy_addr = 0x10 | ((reg_word_addr >> 5) & 0x7); /* bit7-5 of reg address */
    phy_reg = reg_word_addr & 0x1f;                 /* bit4-0 of reg address */
    phy_val1 = phy_reg_read(0, phy_addr, phy_reg);
    phy_val = ((phy_val1 << 16) | phy_val0);
    return phy_val;
}

static void
athrs26_reg_write(uint16_t reg_addr, uint32_t reg_val)
{
    uint16_t reg_word_addr, phy_val;
    uint32_t phy_addr;
    uint8_t  phy_reg; 

    /* write the first 16 bits*/
    reg_word_addr = (reg_addr / 4) * 2;    
    /* configure register high address */
    phy_addr = 0x18;
    phy_reg = 0x0;
    phy_val = (reg_word_addr >> 8) & 0x1ff;         /* bit16-8 of reg address*/
    phy_reg_write (0, phy_addr, phy_reg, phy_val);

    /* read register with low address */
    phy_addr = 0x10 | ((reg_word_addr >> 5) & 0x7); /* bit7-5 of reg address */
    phy_reg = reg_word_addr & 0x1f;                 /* bit4-0 of reg address */
    phy_val = reg_val & 0xffff;
    phy_reg_write (0, phy_addr, phy_reg, phy_val);
    
    /* write the second 16 bits*/
    reg_word_addr++;    
    /* configure register high address */
    phy_addr = 0x18;
    phy_reg = 0x0;
    phy_val = (reg_word_addr >> 8) & 0x1ff;         /* bit16-8 of reg address*/
    phy_reg_write (0, phy_addr, phy_reg, phy_val);

    /* read register with low address */
    phy_addr = 0x10 | ((reg_word_addr >> 5) & 0x7); /* bit7-5 of reg address */
    phy_reg = reg_word_addr & 0x1f;                 /* bit4-0 of reg address */
    phy_val = (reg_val >> 16) & 0xffff;
    phy_reg_write (0, phy_addr, phy_reg, phy_val);   
}

#endif