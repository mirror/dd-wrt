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

#include <linux/types.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include "ag7100_phy.h"
#include "ag7100.h"

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

#define MODULE_NAME "ATHRS26"

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

static cmd_resp_t cmd_resp;
static DECLARE_WAIT_QUEUE_HEAD (hd_conf_wait);
static int wait_flag = 0;
static ag7100_mac_t *ag7100_macs[2];
static atomic_t seqcnt = ATOMIC_INIT(0);

#define ATHR_GLOBALREGBASE    0

//#define ATHR_PHY_MAX (sizeof(ipPhyInfo) / sizeof(ipPhyInfo[0]))
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
static uint32_t athrs26_reg_read(uint32_t reg_addr);
static void athrs26_reg_write(uint32_t reg_addr, uint32_t reg_val);

#if !defined(HEADER_REG_CONF) && !defined(CONFIG_AR9100) 
#define get_field_val(_reg, _mask, _shift, _res_reg)     \
    do { \
        unsigned int temp;  \
        temp = ar7100_reg_rd(_reg); \
        temp &= (unsigned int)_mask;\
        _res_reg  = temp >> _shift; \
    } while (0)

#define set_field_val(_reg, _mask, _shift, _val)                \
    do { \
        unsigned int temp; \
        temp = ar7100_reg_rd(_reg); \
        temp &= ~_mask;  \
        temp |= _val << _shift;  \
        ar7100_reg_wr(_reg, temp);\
    } while (0)

static unsigned int old_ahb_div = 0;

void ag7100_ahb_feq_adjust(void)
{
    unsigned int pll_fb = 0, ahb_div = 0, cpu_div = 0, mask = 0; 

    mask = AR71XX_PLL_DIV_MASK << AR71XX_PLL_DIV_SHIFT;
    get_field_val(AR7100_PLL_CONFIG, mask, AR71XX_PLL_DIV_SHIFT, pll_fb);

    mask = AR71XX_AHB_DIV_MASK << AR71XX_AHB_DIV_SHIFT;
    get_field_val(AR7100_PLL_CONFIG, mask, AR71XX_AHB_DIV_SHIFT, old_ahb_div);

    mask = AR71XX_CPU_DIV_MASK << AR71XX_CPU_DIV_SHIFT;
    get_field_val(AR7100_PLL_CONFIG, mask, AR71XX_CPU_DIV_SHIFT, cpu_div);
    
    //ahb_div = ((((pll_fb + 1) * 40)*2/(200*(cpu_div + 1))) + 1)/2 - 1;
    ahb_div = ( (2*pll_fb + 2)/(5*cpu_div + 5) + 1)/2 - 1;
    mask = AR71XX_AHB_DIV_MASK << AR71XX_AHB_DIV_SHIFT;
    set_field_val(AR7100_PLL_CONFIG, mask, AR71XX_AHB_DIV_SHIFT, ahb_div);
}

void ag7100_ahb_feq_restore(void)
{
    unsigned int mask = 0;
    mask = AR71XX_AHB_DIV_MASK << AR71XX_AHB_DIV_SHIFT;
    set_field_val(AR7100_PLL_CONFIG, mask, AR71XX_AHB_DIV_SHIFT, old_ahb_div); 
}
#endif

#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
static uint16_t port_def_vid[5] = {1, 1, 1, 1, 1};
static uint8_t cpu_egress_tagged_flag = 0;

inline uint8_t is_cpu_egress_tagged(void)
{
    return cpu_egress_tagged_flag;
}

void set_cpu_egress_tagged(uint8_t is_tagged)
{
    cpu_egress_tagged_flag = is_tagged;   
}

inline uint16_t athrs26_defvid_get(uint32_t port_id)
{
    if ((port_id == 0) || (port_id > 5))
        return 0;

    return port_def_vid[port_id - 1];
}

BOOL athrs26_defvid_set(uint32_t port_id, uint16_t def_vid)
{
    if ((def_vid == 0) || (def_vid > 4094))
        return FALSE;

    if ((port_id == 0) || (port_id > 5))
        return FALSE;
      
    port_def_vid[port_id - 1] = def_vid;
    return TRUE;
}
#endif

void athrs26_reg_init()
{
    int i = 20;
    
    /* if using header for register configuration, we have to     */
    /* configure s26 register after frame transmission is enabled */
    if (athr26_init_flag)
        return;
    
#if 0
	______________________________________________________
	u-boot has deasserted the RMII's TX_CTL, the following
	additional reset in athrs26_reg_init() is not needed.
	______________________________________________________

    /* reset switch */
    printk(MODULE_NAME ": resetting s26\n");
    athrs26_reg_write(0x0, athrs26_reg_read(0x0)|0x80000000);

    while(i--) {
        mdelay(100);
        if(!(athrs26_reg_read(0x0)&0x80000000))
            break;
    }

    mdelay(3000);
    printk(MODULE_NAME ": s26 reset done\n");

    phy_reg_write(0, ATHR_PHY4_ADDR, 0, 0x0800);
#endif

    athrs26_reg_write(0x200, 0x200);
    athrs26_reg_write(0x300, 0x200);
    athrs26_reg_write(0x400, 0x200);
    athrs26_reg_write(0x500, 0x200);
    athrs26_reg_write(0x600, 0x7d);

    athrs26_reg_write(0x38, 0xc000050e);

#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)        
#ifdef HEADER_EN        
    athrs26_reg_write(0x104, 0x6804);
#else
    athrs26_reg_write(0x104, 0x6004);
#endif

    athrs26_reg_write(0x204, 0x6004);
    athrs26_reg_write(0x304, 0x6004);
    athrs26_reg_write(0x404, 0x6004);
    athrs26_reg_write(0x504, 0x6004);    
    athrs26_reg_write(0x604, 0x6004);    
#else

    athrs26_reg_write(0x38, 0xc000050e);      
#ifdef HEADER_EN        
    athrs26_reg_write(0x104, 0x4804);
#else
    athrs26_reg_write(0x104, 0x4004);
#endif
#endif
       
    athrs26_reg_write(0x60, 0xffffffff);
    athrs26_reg_write(0x64, 0xaaaaaaaa);
    athrs26_reg_write(0x68, 0x55555555);    
    athrs26_reg_write(0x6c, 0x0);    

//    athrs26_reg_write(0x70, 0x41af);


#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
    set_cpu_egress_tagged(0); /* use set_cpu_egress_tagged(1) to let s26 forward frame to cpu always tagged */
                              /* some applications need to know the vlan information, but please make sure  */
                              /* define HEADER_EN in this case */ 
#endif

#ifdef FULL_FEATURE
    athena_init(0, 2);
#endif
    athrs26_reg_write(0x30,(athrs26_reg_read(0x30)&AR8216_GCTRL_MTU)|1528);


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
    struct net_device  *dev = mac->mac_dev;
    
    if (mac->mac_unit == ENET_UNIT_LAN)
        return;
        
    netif_carrier_off(dev);
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
    mdelay(2000);
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
    
#if (!defined(CONFIG_AR9100)) && (!defined(HEADER_REG_CONF))
    ag7100_ahb_feq_adjust();
#endif

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
#if (!defined(CONFIG_AR9100)) && (!defined(HEADER_REG_CONF))
    ag7100_ahb_feq_restore();
#endif
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
    int       phyUnit;
    uint16_t  phyHwStatus;
    uint16_t  timeout;
    int       liveLinks = 0;
    uint32_t  phyBase = 0;
    BOOL      foundPhy = FALSE;
    uint32_t  phyAddr = 0;
    

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

        mdelay(1000);
    
    /*
     * Wait up to 1 seconds for ALL associated PHYs to finish
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

            mdelay(150);
        }
#ifdef CONFIG_AR9100

        /* fix IOT */
        phy_reg_write(0, phyUnit, 29, 0x14);
        phy_reg_write(0, phyUnit, 30, 0x1352); 
        
#ifdef S26_VER_1_0
        //turn off power saving
        phy_reg_write(0, phyUnit, 29, 41);
        phy_reg_write(0, phyUnit, 30, 0);
        printk("def_ S26_VER_1_0\n");
#endif
#endif
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
#ifndef CONFIG_AR9100
    if (ethUnit == ENET_UNIT_LAN) {
        ag7100_ahb_feq_adjust();  
        athrs26_reg_init();
        ag7100_ahb_feq_restore();       
    }    
#endif
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
    int       phyUnit;
    uint32_t  phyBase;
    uint32_t  phyAddr;
    uint16_t  phyHwStatus;
    int       ii = 200;

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
                phyHwStatus = phy_reg_read (phyBase, phyAddr, 
                                               ATHR_PHY_SPEC_STATUS);
		        if(phyHwStatus & ATHR_STATUS_RESOVLED)
			        break;
                mdelay(10);
            } while(--ii);
            
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
    int       phyUnit;
    uint16_t  phyHwStatus;
    uint32_t  phyBase;
    uint32_t  phyAddr;
    int       ii = 200;
    ag7100_phy_speed_t phySpeed;

    for (phyUnit=0; phyUnit < ATHR_PHY_MAX; phyUnit++) {
        if (!ATHR_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }

        phyBase = ATHR_PHYBASE(phyUnit);
        phyAddr = ATHR_PHYADDR(phyUnit);
        phySpeed = AG7100_PHY_SPEED_10T;

        if (athrs26_phy_is_link_alive(phyUnit)) {

            do {
                phyHwStatus = phy_reg_read(phyBase, phyAddr, 
                                              ATHR_PHY_SPEC_STATUS);
		        if(phyHwStatus & ATHR_STATUS_RESOVLED)
			        break;
                mdelay(10);
            }while(--ii);
            
            phyHwStatus = ((phyHwStatus & ATHER_STATUS_LINK_MASK) >>
                           ATHER_STATUS_LINK_SHIFT);

            switch(phyHwStatus) {
            case 0:
                phySpeed = AG7100_PHY_SPEED_10T;
                break;
            case 1:
                phySpeed = AG7100_PHY_SPEED_100TX;
                break;
            case 2:
                phySpeed = AG7100_PHY_SPEED_1000T;
                break;
            default:
                printk("Unkown speed read!\n");
            }
        } 

        phy_reg_write(phyBase, phyAddr, ATHR_DEBUG_PORT_ADDRESS, 0x18);
        
        if(phySpeed == AG7100_PHY_SPEED_100TX) {
            phy_reg_write(phyBase, phyAddr, ATHR_DEBUG_PORT_DATA, 0xba8);
        } else {            
            phy_reg_write(phyBase, phyAddr, ATHR_DEBUG_PORT_DATA, 0x2ea);
        }
    }

    if (ethUnit == ENET_UNIT_LAN)
         phySpeed = AG7100_PHY_SPEED_100TX;

    return phySpeed;
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
    int           phyUnit;
    uint16_t      phyHwStatus, phyHwControl;
    athrPhyInfo_t *lastStatus;
    int           linkCount   = 0;
    int           lostLinks   = 0;
    int           gainedLinks = 0;
    uint32_t      phyBase;
    uint32_t      phyAddr;

#ifdef HEADER_REG_CONF
    /* if using header to config s26, the link of MAC0 should always be up */
    if (ethUnit == ENET_UNIT_LAN)
        return 1;
#endif

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

static int
athrs26_header_config_reg (struct net_device *dev, uint8_t wr_flag, 
                           uint32_t reg_addr, uint16_t cmd_len,
                           uint8_t *val, uint32_t seq_num) 
{
    struct sk_buff *skb;
    at_header_t at_header;
    reg_cmd_t reg_cmd;
  
    /*allocate skb*/        
    skb = dev_alloc_skb(64);
    if (!skb) {
        printk("allocate skb fail\n");
        return -1;
    }
    skb_put(skb, 60);     
    
    /*fill at_header*/
    at_header.reserved0 = 0x10;  //default 
    at_header.priority = 0;
    at_header.type = 0x5;
    at_header.broadcast = 0;  
    at_header.from_cpu = 1;
    at_header.reserved1 = 0x01; //default   
    at_header.port_num = 0;

    skb->data[0] = at_header.port_num;
    skb->data[0] |= at_header.reserved1 << 4;
    skb->data[0] |= at_header.from_cpu << 6;
    skb->data[0] |= at_header.broadcast << 7;
    
    skb->data[1] = at_header.type;  
    skb->data[1] |= at_header.priority << 4;    
    skb->data[1] |= at_header.reserved0 << 6;     
     
    /*fill reg cmd*/
    if(cmd_len > 4) 
        cmd_len = 4;//only support 32bits register r/w
      
    reg_cmd.reg_addr = reg_addr&0x3FFFC;
    reg_cmd.cmd_len = cmd_len;
    reg_cmd.cmd = wr_flag;
    reg_cmd.reserved2 = 0x5; //default
    reg_cmd.seq_num = seq_num;   

    skb->data[2] = reg_cmd.reg_addr & 0xff;
    skb->data[3] = (reg_cmd.reg_addr & 0xff00) >> 8;
    skb->data[4] = (reg_cmd.reg_addr & 0x30000) >> 16;
    skb->data[4] |= reg_cmd.cmd_len << 4;
    skb->data[5] = reg_cmd.cmd << 4;
    skb->data[5] |= reg_cmd.reserved2 << 5;
    skb->data[6] = (reg_cmd.seq_num & 0x7f) << 1;
    skb->data[7] = (reg_cmd.seq_num & 0x7f80) >> 7;
    skb->data[8] = (reg_cmd.seq_num & 0x7f8000) >> 15;
    skb->data[9] = (reg_cmd.seq_num & 0x7f800000) >> 23;
  
    if(!wr_flag)//write
        memcpy(skb->data + 10, val, cmd_len);
    
    skb->dev = dev;

    /* add identify for header */
    skb->cb[0] = 0x7f;
    skb->cb[1] = 0x5d;
    
    /*start xmit*/
    header_xmit(skb, dev);

    return 0;
}


static int
__athrs26_header_write_reg(uint32_t reg_addr, uint16_t cmd_len, uint8_t *reg_data,
                           uint32_t seq_num)
{
    long timeout;
    int i = 2;
    uint8_t reg_tmp[4];
  
    /*fill reg data*/
    reg_tmp[0] = (uint8_t)(0x00ff & (*((uint32_t *)reg_data))); 
    reg_tmp[1] = (uint8_t)((0xff00 & (*((uint32_t *)reg_data))) >> 8);      
    reg_tmp[2] = (uint8_t)((0xff0000 & (*((uint32_t *)reg_data))) >> 16);
    reg_tmp[3] = (uint8_t)((0xff000000 & (*((uint32_t *)reg_data))) >> 24);
  
    do {
        wait_flag = 0;
        athrs26_header_config_reg(ag7100_macs[0]->mac_dev, 0, reg_addr, cmd_len, 
                                  reg_tmp, seq_num);
        timeout = HZ;     
        if (!in_interrupt()) {
            timeout = wait_event_interruptible_timeout (hd_conf_wait, 
                                                        wait_flag != 0, timeout);
        }
        if (timeout || ((reg_addr == 0)&&(reg_tmp[3]&0x80)))  //ignore reset write echo 
            break;
        else
            printk("write time out\n");
    } while (i--);

    if(i==0) {
        return -1;
    }
    
    return 0;   
}

static int
__athrs26_header_read_reg(uint32_t reg_addr, uint16_t cmd_len, uint8_t *reg_data, 
                          uint32_t seq_num)
{
    long timeout;
    int i = 2;

    if (in_interrupt())
        return -1;
        
    do {
        wait_flag = 0;          
        athrs26_header_config_reg(ag7100_macs[0]->mac_dev, 1, reg_addr,
                                  cmd_len, reg_data, seq_num);
        timeout = HZ;  
        timeout = wait_event_interruptible_timeout (hd_conf_wait, 
                                                    wait_flag != 0, timeout);
 
        if (timeout) 
            break;
        else 
            printk("read time out\n");
    } while(i--);   
    
    if ((i==0) || (seq_num != cmd_resp.seq) || (cmd_len != cmd_resp.len)) {
        return -1;
    }
  
    (*((uint32_t *)reg_data)) = cmd_resp.data[0] | (cmd_resp.data[1] << 8)
                             | (cmd_resp.data[2] << 16)| (cmd_resp.data[3] << 24);

    return 0;   
}

static int
set_cpu_port_learn(int flags)
{
    uint32_t reg_data = 0;
    uint32_t reg_addr = 0x104;
    uint32_t seq_num = 0x7fffffff;
    
      __athrs26_header_read_reg(reg_addr, 4, (uint8_t *)&reg_data, seq_num);
    
    if(flags)
      reg_data |= 0x1<<14;  //LEARN_EN enable
    else
      reg_data &= ~(0x1<<14); //LEARN_EN disable
    
    __athrs26_header_write_reg (reg_addr, 4, (uint8_t *)&reg_data, seq_num);
}

static int
get_cpu_port_learn(void)
{
    uint32_t reg_data = 0;
    uint32_t reg_addr = 0x104;
    uint32_t seq_num = 0x7fffffff;
      
    __athrs26_header_read_reg(reg_addr, 4, (uint8_t *)&reg_data, seq_num);
    
    return  (reg_data >> 14) & 0x1; 
}

int
athrs26_header_write_reg(uint32_t reg_addr, uint16_t cmd_len, uint8_t *reg_data)
{
    int flagsave = 0;
  
    if(flagsave = get_cpu_port_learn())
        set_cpu_port_learn(0);//LEARN_EN disable

    /*reserve seqcnt 0x7fffffff for set_cpu_port_learn*/ 
    if(atomic_read(&seqcnt) == 0x7ffffffe){
        atomic_set(&seqcnt, 0);
    } else {
        atomic_inc(&seqcnt);
    }

    __athrs26_header_write_reg(reg_addr, 4, reg_data, atomic_read(&seqcnt));

    if(flagsave && 
        !((reg_addr == 0x104) && ((*(uint32_t *)reg_data>>14 & 0x1) == 0)))
        set_cpu_port_learn(1);//LEARN_EN enable

    return 0;   
}

int
athrs26_header_read_reg(uint32_t reg_addr, uint16_t cmd_len, uint8_t *reg_data)
{
    int flagsave = 0;
    
    if(flagsave = get_cpu_port_learn())
      set_cpu_port_learn(0);//LEARN_EN disable
    
    /*reserve seqcnt 0x7fffffff for set_cpu_port_learn*/ 
      if(atomic_read(&seqcnt) == 0x7ffffffe){
      atomic_set(&seqcnt, 0);
    } else {
      atomic_inc(&seqcnt);
    }
    
      __athrs26_header_read_reg(reg_addr, 4, reg_data, atomic_read(&seqcnt));
    
    /*for r/w register 0x104*/
    if((reg_addr == 0x104) && (flagsave == 1))
      *(uint32_t *)reg_data |= 0x1<<14;  //LEARN_EN enable
    else if ((reg_addr == 0x104) && (flagsave == 0))
      *(uint32_t *)reg_data &= ~(0x1<<14); //LEARN_EN disable
    
    if(flagsave)
      set_cpu_port_learn(1);//LEARN_EN enable
    
    return 0;
}

int header_receive_skb(struct sk_buff *skb)
{
    wait_flag = 1;
    cmd_resp.len = skb->data[4] >> 4;
    if (cmd_resp.len > 10)
        goto out;
    
    cmd_resp.seq = skb->data[6] >> 1;
    cmd_resp.seq |= skb->data[7] << 7;
    cmd_resp.seq |= skb->data[8] << 15;
    cmd_resp.seq |= skb->data[9] << 23;

    if (cmd_resp.seq < atomic_read(&seqcnt))
        goto out;
          
    memcpy (cmd_resp.data, (skb->data + 10), cmd_resp.len);
    wake_up_interruptible(&hd_conf_wait);
    
out:    
    kfree_skb(skb); 
}

void athrs26_reg_dev(ag7100_mac_t **mac)
{
    ag7100_macs[0] = mac[0];
    ag7100_macs[0]->mac_speed = 0xff;
    ag7100_macs[1] = mac[1];
    ag7100_macs[1]->mac_speed = 0xff;

}

static uint32_t
athrs26_reg_read(uint32_t reg_addr)
{
#ifndef HEADER_REG_CONF 
    uint32_t reg_word_addr;
    uint32_t phy_addr, tmp_val, reg_val;
    uint16_t phy_val;
    uint8_t phy_reg;

    /* change reg_addr to 16-bit word address, 32-bit aligned */
    reg_word_addr = (reg_addr & 0xfffffffc) >> 1;

    /* configure register high address */
    phy_addr = 0x18;
    phy_reg = 0x0;
    phy_val = (uint16_t) ((reg_word_addr >> 8) & 0x1ff);  /* bit16-8 of reg address */
    phy_reg_write(0, phy_addr, phy_reg, phy_val);

    /* For some registers such as MIBs, since it is read/clear, we should */
    /* read the lower 16-bit register then the higher one */

    /* read register in lower address */
    phy_addr = 0x10 | ((reg_word_addr >> 5) & 0x7); /* bit7-5 of reg address */
    phy_reg = (uint8_t) (reg_word_addr & 0x1f);   /* bit4-0 of reg address */
    reg_val = (uint32_t) phy_reg_read(0, phy_addr, phy_reg);

    /* read register in higher address */
    reg_word_addr++;
    phy_addr = 0x10 | ((reg_word_addr >> 5) & 0x7); /* bit7-5 of reg address */
    phy_reg = (uint8_t) (reg_word_addr & 0x1f);   /* bit4-0 of reg address */
    tmp_val = (uint32_t) phy_reg_read(0, phy_addr, phy_reg);
    reg_val |= (tmp_val << 16);

    return reg_val;
#else
    uint8_t reg_data[4];
    
    memset (reg_data, 0, 4);
    athrs26_header_read_reg(reg_addr, 4, reg_data);
    return *((uint32_t *)reg_data);
#endif    
}

static void
athrs26_reg_write(uint32_t reg_addr, uint32_t reg_val)
{
#ifndef HEADER_REG_CONF
    uint32_t reg_word_addr;
    uint32_t phy_addr;
    uint16_t phy_val;
    uint8_t phy_reg;

    /* change reg_addr to 16-bit word address, 32-bit aligned */
    reg_word_addr = (reg_addr & 0xfffffffc) >> 1;

    /* configure register high address */
    phy_addr = 0x18;
    phy_reg = 0x0;
    phy_val = (uint16_t) ((reg_word_addr >> 8) & 0x1ff);  /* bit16-8 of reg address */
    phy_reg_write(0, phy_addr, phy_reg, phy_val);

    /* For some registers such as ARL and VLAN, since they include BUSY bit */
    /* in lower address, we should write the higher 16-bit register then the */
    /* lower one */

    /* read register in higher address */
    reg_word_addr++;
    phy_addr = 0x10 | ((reg_word_addr >> 5) & 0x7); /* bit7-5 of reg address */
    phy_reg = (uint8_t) (reg_word_addr & 0x1f);   /* bit4-0 of reg address */
    phy_val = (uint16_t) ((reg_val >> 16) & 0xffff);
    phy_reg_write(0, phy_addr, phy_reg, phy_val);

    /* write register in lower address */
    reg_word_addr--;
    phy_addr = 0x10 | ((reg_word_addr >> 5) & 0x7); /* bit7-5 of reg address */
    phy_reg = (uint8_t) (reg_word_addr & 0x1f);   /* bit4-0 of reg address */
    phy_val = (uint16_t) (reg_val & 0xffff);
    phy_reg_write(0, phy_addr, phy_reg, phy_val); 
#else
    athrs26_header_write_reg (reg_addr, 4, (uint8_t *)&reg_val);
#endif
}

int
athr_ioctl(uint32_t *args, int cmd)
{
#ifdef FULL_FEATURE
    if (sw_ioctl(args, cmd))
        return -EOPNOTSUPP;

    return 0;
#else
    return -EOPNOTSUPP;
#endif
}


