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

#include <linux/autoconf.h>
#include <linux/types.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/delay.h>
#include "ag7100_phy.h"
#include "ag7100.h"

#ifdef DEBUG_CMD
#include "error.h"
#include "config.h"
#include "hsl.h"
#include "reg_access.h"
#include "athena_reg.h"
#include "reg_list.h"
#endif

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

#ifdef HEADER_EN
static cmd_resp_t cmd_resp;
static DECLARE_WAIT_QUEUE_HEAD (hd_conf_wait);
static int wait_flag = 0;
static ag7100_mac_t *ag7100_macs[2];
static atomic_t seqcnt = ATOMIC_INIT(0);
#endif

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
static uint32_t athrs26_reg_read(uint16_t reg_addr);
static void athrs26_reg_write(uint16_t reg_addr, uint32_t reg_val);

#if !defined(HEADER_REG_CONF) && !defined(CONFIG_AR9100) 
#define get_field_val(_reg, _mask, _shift, _res_reg)     \
    do { \
        unsigned int temp;	\ 
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

    mask = PLL_DIV_MASK << PLL_DIV_SHIFT;
    get_field_val(AR7100_PLL_CONFIG, mask, PLL_DIV_SHIFT, pll_fb);

    mask = AHB_DIV_MASK << AHB_DIV_SHIFT;
    get_field_val(AR7100_PLL_CONFIG, mask, AHB_DIV_SHIFT, old_ahb_div);

    mask = CPU_DIV_MASK << CPU_DIV_SHIFT;
    get_field_val(AR7100_PLL_CONFIG, mask, CPU_DIV_SHIFT, cpu_div);
    
    //ahb_div = ((((pll_fb + 1) * 40)*2/(200*(cpu_div + 1))) + 1)/2 - 1;
    ahb_div = ( (2*pll_fb + 2)/(5*cpu_div + 5) + 1)/2 - 1;
    mask = AHB_DIV_MASK << AHB_DIV_SHIFT;
    set_field_val(AR7100_PLL_CONFIG, mask, AHB_DIV_SHIFT, ahb_div);
}

void ag7100_ahb_feq_restore(void)
{
    unsigned int mask = 0;
    mask = AHB_DIV_MASK << AHB_DIV_SHIFT;
    set_field_val(AR7100_PLL_CONFIG, mask, AHB_DIV_SHIFT, old_ahb_div); 
}
#endif

void athrs26_reg_init()
{
    int i = 20;
    
    /* if using header for register configuration, we have to     */
    /* configure s26 register after frame transmission is enabled */
    if (athr26_init_flag)
        return;
    
#if (!defined(CONFIG_AR9100)) && (!defined(HEADER_REG_CONF))
    ag7100_ahb_feq_adjust();  
#endif  
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

    athrs26_reg_write(0x200, 0x200);
    athrs26_reg_write(0x300, 0x200);
    athrs26_reg_write(0x400, 0x200);
    athrs26_reg_write(0x500, 0x200);
    athrs26_reg_write(0x600, 0x7d);

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

#if (!defined(CONFIG_AR9100)) && (!defined(HEADER_REG_CONF))
        ag7100_ahb_feq_restore(); 
#endif

#ifdef DEBUG_CMD  
    athena_reg_access_init();
#endif

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
    netif_stop_queue(dev);
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
    struct net_device  *dev = mac->mac_dev;
    
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
    struct net_device  *dev = mac->mac_dev;
    uint32_t reg_val;
    
    if ((mac->mac_unit == ENET_UNIT_LAN))
        return;
    
    switch (speed) {
        case AG7100_PHY_SPEED_100TX:
            athrs26_reg_write (0x600, 0x7d);
            break;
           
        case AG7100_PHY_SPEED_10T:
            athrs26_reg_write (0x600, 0x7c);
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
     if (ethUnit == ENET_UNIT_LAN)
        mdelay(1000);
     else
        mdelay(3000);
    
    /*
     * Wait up to 3 seconds for ALL associated PHYs to finish
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
#ifdef S26_VER_1_0
        //turn off power saving
        phy_reg_write(0, phyUnit, 29, 41);
        phy_reg_write(0, phyUnit, 30, 0);
       	printk("def_ S26_VER_1_0\n");
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
                phyHwStatus = ag7100_mii_read (phyBase, phyAddr, 
                                               ATHR_PHY_SPEC_STATUS);
                mdelay(10);
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
    int       phyUnit;
    uint16_t  phyHwStatus;
    uint32_t  phyBase;
    uint32_t  phyAddr;
    int       ii = 200;

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
                mdelay(10);
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
                printk("Unkown speed read!\n");
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

#ifdef HEADER_EN
#ifdef HEADER_REG_CONF
static int
athrs26_header_config_reg (struct net_device *dev, uint8_t wr_flag, 
                           uint16_t reg_addr, uint16_t cmd_len,
                           uint8_t *val) 
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
    reg_cmd.seq_num = atomic_read(&seqcnt);   

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

    /*fill reg data*/
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
athrs26_header_write_reg(uint16_t reg_addr, uint16_t cmd_len, uint8_t *reg_data)
{
    long timeout;
    int i = 2;

    atomic_inc(&seqcnt);

    do {
        wait_flag = 0;
        athrs26_header_config_reg(ag7100_macs[0]->mac_dev, 0, reg_addr, cmd_len, reg_data);
        timeout = HZ;     
        if (!in_interrupt()) {
            timeout = wait_event_interruptible_timeout (hd_conf_wait, 
                                                        wait_flag != 0, timeout);
        }
        if (timeout || ((reg_addr==0)&&(reg_data[3]&0x80)))  //ignore reset write echo 
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
athrs26_header_read_reg(uint16_t reg_addr, uint16_t cmd_len, uint8_t *reg_data)
{
    long timeout;
    int i = 2;

    if (in_interrupt())
        return -1;
        
    atomic_inc(&seqcnt);
    do {
        wait_flag = 0;          
        athrs26_header_config_reg(ag7100_macs[0]->mac_dev, 1, reg_addr, cmd_len, reg_data);
        timeout = HZ;  
        timeout = wait_event_interruptible_timeout (hd_conf_wait, 
                                                    wait_flag != 0, timeout);
 
        if (timeout) 
            break;
        else 
            printk("read time out\n");
    } while(i--);   
    
    if ((i==0) || (atomic_read(&seqcnt) != cmd_resp.seq) || (cmd_len != cmd_resp.len)) {
        return -1;
    }

    memcpy (reg_data, cmd_resp.data, cmd_len);
    return 0;   
}
#endif

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
#endif

static uint32_t
athrs26_reg_read(uint16_t reg_addr)
{
#ifndef HEADER_REG_CONF	
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
#else
    uint8_t reg_data[4];
    
    memset (reg_data, 0, 4);
    athrs26_header_read_reg(reg_addr, 4, reg_data);
    return (reg_data[0] | (reg_data[1] << 8) | (reg_data[2] << 16) | (reg_data[3] << 24));
#endif    
}

static void
athrs26_reg_write(uint16_t reg_addr, uint32_t reg_val)
{
#ifndef HEADER_REG_CONF
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
#else
    uint8_t reg_data[4];
    
    memset (reg_data, 0, 4);    
    reg_data[0] = (uint8_t)(0x00ff & reg_val); 
    reg_data[1] = (uint8_t)((0xff00 & reg_val) >> 8);      
    reg_data[2] = (uint8_t)((0xff0000 & reg_val) >> 16);
    reg_data[3] = (uint8_t)((0xff000000 & reg_val) >> 24);

    athrs26_header_write_reg (reg_addr, 4, reg_data);
#endif
}

int
athr_ioctl(uint32_t unit, uint32_t *args)
{
#ifdef DEBUG_CMD
    switch (args[0]) {
    case ATHRCGREG: {
        uint32_t reg_val=0;
        
        reg_val = (uint32_t)athrs26_reg_read(args[1]);
        if (copy_to_user((void __user *)args[2], &reg_val, 
            sizeof(uint32_t)))
          return EFAULT;
        return 0;
    }

    case ATHRCSREG: {
        athrs26_reg_write ((uint16_t)args[1], (uint32_t)args[2]);
        return 0;
    }

    case ATHRCGETY: {
        uint32_t reg_addr, cmd_num, index;
        uint16_t entry_len;
        uint32_t reg_val;
        sw_hsl_reg_func_t *p_reg = (sw_hsl_reg_func_t *)athena_reg_ptr_get();
        EBU_STATUS rtn;

        if (!p_reg)
            return ENXIO;
        
        cmd_num = args[1];
        index = args[2];  

        reg_addr = reg_desc[cmd_num].reg_addr;
        entry_len = 4;
        
        rtn = p_reg->reg_entry_get (0, reg_addr, 
                                    index * reg_desc[cmd_num].entry_offset,
                                    entry_len, (uint8_t *)&reg_val, 4); 

        if (rtn == EBU_ERROR)
            return EINVAL;

        if (copy_to_user((void __user *)args[3], &reg_val, sizeof(uint32_t)))
            return EFAULT;

        return 0;
    }
  
    case ATHRCSETY: {
        uint32_t reg_addr, cmd_num, index;
        uint16_t entry_len;
        sw_hsl_reg_func_t *p_reg = (sw_hsl_reg_func_t *)athena_reg_ptr_get(); 
        EBU_STATUS rtn;
       
        if (!p_reg)
            return ENXIO;
        
        cmd_num = args[1];
        index = args[2];
    
        reg_addr = reg_desc[cmd_num].reg_addr;
        entry_len = 4;
    
        rtn = p_reg->reg_entry_set (0, reg_addr, (uint16_t)
                                    (index * reg_desc[cmd_num].entry_offset),
                                    entry_len, (uint8_t *)&args[3], 4);
        if (rtn == EBU_ERROR)
            return EINVAL;
        
        return 0;
    }
        
    case ATHRCGFLD: {
        uint32_t reg_addr, cmd_num, index;
        uint16_t entry_offset, field_offset, field_len;
        uint32_t field_val;
        sw_hsl_reg_func_t *p_reg = (sw_hsl_reg_func_t *)athena_reg_ptr_get(); 
        EBU_STATUS rtn;

        if (!p_reg)
            return ENXIO;
        
        cmd_num = args[1];
        index = args[2];  

        reg_addr = reg_desc[field_desc[cmd_num].reg_id].reg_addr;
        field_len = field_desc[cmd_num].bit_len;
        field_offset = field_desc[cmd_num].bit_offset;
        entry_offset = index * 
                       reg_desc[field_desc[cmd_num].reg_id].entry_offset; 

        rtn = p_reg->reg_field_get (0, reg_addr, entry_offset,
                                    field_offset, field_len, (uint8_t *)&field_val, 4);
        if (rtn == EBU_ERROR)
            return EINVAL;

      if (copy_to_user((void __user *)args[3], &field_val,
                       sizeof(uint32_t)))
        return EFAULT;

        return 0;          
    }
  
    case ATHRCSFLD:
    {
        uint32_t reg_addr, cmd_num, index;
        uint16_t entry_offset, field_offset, field_len;
        sw_hsl_reg_func_t *p_reg = (sw_hsl_reg_func_t *)athena_reg_ptr_get(); 
        EBU_STATUS rtn;

        if (!p_reg)
            return ENXIO;
        
        cmd_num = args[1];
        index = args[2];  

        reg_addr = reg_desc[field_desc[cmd_num].reg_id].reg_addr;
        field_len = field_desc[cmd_num].bit_len;
        field_offset = field_desc[cmd_num].bit_offset;

        entry_offset = index * 
                       reg_desc[field_desc[cmd_num].reg_id].entry_offset;

        rtn = p_reg->reg_field_set (0, reg_addr, entry_offset,
                                    field_offset, field_len, (uint8_t *)&args[3], 4);
        
        if (rtn == EBU_ERROR)
            return EINVAL;
    
        return 0;
    }

    default:
        return -EOPNOTSUPP;
    }
#endif /* DEBUG_CMD*/

    return -EOPNOTSUPP;
}


