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
#include <linux/delay.h>
#include "ag7240_phy.h"
/*#include "ag7240.h"
#include "eth_diag.h"*/
#include "athrs16_phy.h" 

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


/*depend on connection between cpu mac and s16 mac*/
#if defined (CONFIG_PORT0_AS_SWITCH)
#define ENET_UNIT_LAN 0  
#define ENET_UNIT_WAN 1
#define CFG_BOARD_AP96 1

#elif defined (CONFIG_AG7240_GE0_IS_CONNECTED)
#define ENET_UNIT_LAN 0  
#define CFG_BOARD_PB45 0
#define CFG_BOARD_AP96 1

#else
#define ENET_UNIT_LAN 1  
#define ENET_UNIT_WAN 0
#define CFG_BOARD_PB45 1
#endif


#define TRUE    1
#define FALSE   0

#define ATHR_PHY0_ADDR   0x0
#define ATHR_PHY1_ADDR   0x1
#define ATHR_PHY2_ADDR   0x2
#define ATHR_PHY3_ADDR   0x3
#define ATHR_PHY4_ADDR   0x4
#define ATHR_IND_PHY 4

#define MODULE_NAME "ATHRS16"
extern int xmii_val;
extern int ag7240_open(struct net_device *dev);
extern int ag7240_stop(struct net_device *dev);
extern ag7240_mac_t *ag7240_macs[2];

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
     ENET_UNIT_LAN,//ENET_UNIT_WAN,
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

static uint8_t athr16_init_flag = 0;

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
BOOL athrs16_phy_is_link_alive(int phyUnit);
static void phy_mode_setup(void);

static void phy_mode_setup(void) 
{
#ifdef ATHRS16_VER_1_0
    printk("phy_mode_setup\n");

    /*work around for phy4 rgmii mode*/
    phy_reg_write(ATHR_PHYBASE(ATHR_IND_PHY), ATHR_PHYADDR(ATHR_IND_PHY), 29, 18);     
    phy_reg_write(ATHR_PHYBASE(ATHR_IND_PHY), ATHR_PHYADDR(ATHR_IND_PHY), 30, 0x480c);    

    /*rx delay*/ 
    phy_reg_write(ATHR_PHYBASE(ATHR_IND_PHY), ATHR_PHYADDR(ATHR_IND_PHY), 29, 0);     
    phy_reg_write(ATHR_PHYBASE(ATHR_IND_PHY), ATHR_PHYADDR(ATHR_IND_PHY), 30, 0x824e);  

    /*tx delay*/ 
    phy_reg_write(ATHR_PHYBASE(ATHR_IND_PHY), ATHR_PHYADDR(ATHR_IND_PHY), 29, 5);     
    phy_reg_write(ATHR_PHYBASE(ATHR_IND_PHY), ATHR_PHYADDR(ATHR_IND_PHY), 30, 0x3d47);    
#endif
}

void athrs16_reg_init(int ethUinit)
{
    /* if using header for register configuration, we have to     */
    /* configure s16 register after frame transmission is enabled */
    if (athr16_init_flag)
        return;

    /*Power on strip mode setup*/
#if CFG_BOARD_PB45
    athrs16_reg_write(0x208, 0x2fd0001);  /*tx delay*/   
    athrs16_reg_write(0x108, 0x2be0001);  /*mac0 rgmii mode*/ 
#elif CFG_BOARD_AP96
    //athrs16_reg_write(0x8, 0x012e1bea);
    athrs16_reg_write(0x8, 0x01261be2);
#endif
#if 0
    athrs16_reg_write(S16_PORT_STATUS_REGISTER0, /* 0x7e */
                                    S16_PORT_STATUS_SPEED_1000MBPS 
                                    | S16_PORT_STATUS_TXMAC_EN 
                                    | S16_PORT_STATUS_RXMAC_EN
                                    | S16_PORT_STATUS_RXFLOW_EN
                                    | S16_PORT_STATUS_TXFLOW_EN
                                    | S16_PORT_STATUS_DUPLEX_FULL);

    athrs16_reg_write(S16_PORT_STATUS_REGISTER1, /* 0x230 */
                             S16_PORT_STATUS_LINK_EN
                           | S16_PORT_STATUS_RXFLOW_EN
                           | S16_PORT_STATUS_TXFLOW_EN);

    athrs16_reg_write(S16_PORT_STATUS_REGISTER2, /*0x230*/
                             S16_PORT_STATUS_LINK_EN
                           | S16_PORT_STATUS_RXFLOW_EN
                           | S16_PORT_STATUS_TXFLOW_EN);

    athrs16_reg_write(S16_PORT_STATUS_REGISTER3, /*0x230*/
                             S16_PORT_STATUS_LINK_EN
                           | S16_PORT_STATUS_RXFLOW_EN
                           | S16_PORT_STATUS_TXFLOW_EN);
    athrs16_reg_write(S16_PORT_STATUS_REGISTER4, /*0x230*/
                             S16_PORT_STATUS_LINK_EN
                           | S16_PORT_STATUS_RXFLOW_EN
                           | S16_PORT_STATUS_TXFLOW_EN);
#endif    
    athrs16_reg_write(S16_PORT_STATUS_REGISTER0, 0x7e);
    athrs16_reg_write(S16_PORT_STATUS_REGISTER1, 0x200);
    athrs16_reg_write(S16_PORT_STATUS_REGISTER2, 0x200);
    athrs16_reg_write(S16_PORT_STATUS_REGISTER3, 0x200);
    athrs16_reg_write(S16_PORT_STATUS_REGISTER4, 0x200);
#if CFG_BOARD_PB45
    athrs16_reg_write(0x600, 0x200);
    printk("CFG Board PB45 \n");
#elif CFG_BOARD_AP96
    //athrs16_reg_write(0x600, 0x0);
    printk("CFG Board AP96 \n");
//    athrs16_reg_write(0x600, 0x200);
    athrs16_reg_write(S16_PORT_STATUS_REGISTER5, /*0x230*/
                             S16_PORT_STATUS_LINK_EN
                           | S16_PORT_STATUS_RXFLOW_EN
                           | S16_PORT_STATUS_TXFLOW_EN);
#endif

    athrs16_reg_write(S16_FLD_MASK_REG, 0x003f003f);

#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)        
#ifdef HEADER_EN        
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER0, 0x6804);
#else
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER0, 0x6004);
#endif

    athrs16_reg_write(S16_PORT_CONTROL_REGISTER1, 0x6004);
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER2, 0x6004);
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER3, 0x6004);
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER4, 0x6004);    
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER5, 0x6004);    
#else
#ifdef HEADER_EN        
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER0, 0x4804);
#else
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER0, 0x4004);
#endif
#endif

#ifdef FULL_FEATURE
	hsl_dev_init(0, 2);
#endif

#if 1
   /* Enable ARP packets to CPU port */
    athrs16_reg_write(S16_ARL_TBL_CTRL_REG,(athrs16_reg_read(S16_ARL_TBL_CTRL_REG) | 0x100000));
   /* Enable Broadcast packets to CPU port */
//    athrs16_reg_write(S16_FLD_MASK_REG,(athrs16_reg_read(S16_FLD_MASK_REG) | S16_ENABLE_CPU_BROADCAST ));
    athrs16_reg_write(S16_FLD_MASK_REG,0x003f003f | S16_ENABLE_CPU_BROADCAST); // enable multicast and unicast on all ports, in case that bootloader did not initialize it in correct way
    //jumbo, 8316 only
    athrs16_reg_write(0x30,(athrs16_reg_read(0x30)&AR8316_GCTRL_MTU)|(9018 + 8 + 2));

#endif
#ifndef CONFIG_S26_SWITCH_ONLY_MODE
    athrs16_reg_write(S16_CPU_PORT_REGISTER,0x000001f0);


#ifndef CONFIG_WZRG450
// config wan = port 4, lan = 0 - 3
    /* Recognize tag packet from CPU */

#ifdef CONFIG_WZRG300NH2
//totally screwed up vlan mapping. wan port is 1, lan is 2 3 4 0
    athrs16_reg_write(S16_PORT_BASE_VLAN_REGISTER0,0xc03e0001); // 1111100000000000000001
    /* Insert PVID 1 to LAN ports */
    athrs16_reg_write(S16_PORT_BASE_VLAN_REGISTER1,0x00390001); // 1110010000000000000001
    athrs16_reg_write(S16_PORT_BASE_VLAN_REGISTER3,0x00330001); // 1100110000000000000001
    athrs16_reg_write(S16_PORT_BASE_VLAN_REGISTER4,0x002b0001); // 1010110000000000000001
    athrs16_reg_write(S16_PORT_BASE_VLAN_REGISTER5,0x001b0001); // 0110110000000000000001
   /* Insert PVID 2 to WAN port */
    athrs16_reg_write(S16_PORT_BASE_VLAN_REGISTER2,0x00010002); // 0000010000000000000010

   /* Egress tag packet to CPU and untagged packet to LAN port */

    athrs16_reg_write(S16_PORT_CONTROL_REGISTER0,0x00006204);

    athrs16_reg_write(S16_PORT_CONTROL_REGISTER1,0x00006104);
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER2,0x00006104);
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER3,0x00006104);
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER4,0x00006104);
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER5,0x00006104);

  /* Group port - 0,1,2,3 to VID 1 */
    athrs16_reg_write(S16_VLAN_FUNC_REGISTER1,0x0000083b); // 100000111011
    athrs16_reg_write(S16_VLAN_FUNC_REGISTER0,0x0001000a);

  /*  port - 4  to VID 2 */
    athrs16_reg_write(S16_VLAN_FUNC_REGISTER1,0x00000805); // 100000000101
    athrs16_reg_write(S16_VLAN_FUNC_REGISTER0,0x0002000a); 
#else
    athrs16_reg_write(S16_PORT_BASE_VLAN_REGISTER0,0xc03e0001); // 1111100000000000000001
    /* Insert PVID 1 to LAN ports */
    athrs16_reg_write(S16_PORT_BASE_VLAN_REGISTER1,0x001d0001); //  111010000000000000001
    athrs16_reg_write(S16_PORT_BASE_VLAN_REGISTER2,0x001b0001); //  110110000000000000001
    athrs16_reg_write(S16_PORT_BASE_VLAN_REGISTER3,0x00170001); //  101110000000000000001
    athrs16_reg_write(S16_PORT_BASE_VLAN_REGISTER4,0x000f0001); //  011110000000000000001
   /* Insert PVID 2 to WAN port */
    athrs16_reg_write(S16_PORT_BASE_VLAN_REGISTER5,0x00010002); // 0000010000000000000010

   /* Egress tag packet to CPU and untagged packet to LAN port */

    athrs16_reg_write(S16_PORT_CONTROL_REGISTER0,0x00006204);

    athrs16_reg_write(S16_PORT_CONTROL_REGISTER1,0x00006104);
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER2,0x00006104);
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER3,0x00006104);
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER4,0x00006104);
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER5,0x00006104);

  /* Group port - 0,1,2,3 to VID 1 */
    athrs16_reg_write(S16_VLAN_FUNC_REGISTER1,0x0000081f); // 100000011111
    athrs16_reg_write(S16_VLAN_FUNC_REGISTER0,0x0001000a);

  /*  port - 4  to VID 2 */
    athrs16_reg_write(S16_VLAN_FUNC_REGISTER1,0x00000821); // 100000100001
    athrs16_reg_write(S16_VLAN_FUNC_REGISTER0,0x0002000a); 
#endif





#else
#if 0 // double vlan tag
    athrs16_reg_write(S16_PORT_BASE_VLAN_REGISTER0,0x403e0001); // 1111100000000000000001

    /* Insert PVID 1 to LAN ports */
    athrs16_reg_write(S16_PORT_BASE_VLAN_REGISTER1,0x40010002); // 0000010000000000000001
    athrs16_reg_write(S16_PORT_BASE_VLAN_REGISTER2,0x40390001); // 1110010000000000000001
    athrs16_reg_write(S16_PORT_BASE_VLAN_REGISTER3,0x40350001); // 1101010000000000000001
    athrs16_reg_write(S16_PORT_BASE_VLAN_REGISTER4,0x402d0001); // 1011010000000000000001

   /* Insert PVID 2 to WAN port */
    athrs16_reg_write(S16_PORT_BASE_VLAN_REGISTER5,0x401d0001); // 0111010000000000000010
#else
    athrs16_reg_write(S16_PORT_BASE_VLAN_REGISTER0,0xc03e0001); // 1111100000000000000001

    /* Insert PVID 1 to LAN ports */
    athrs16_reg_write(S16_PORT_BASE_VLAN_REGISTER1,0x00010002); // 0000010000000000000001
    athrs16_reg_write(S16_PORT_BASE_VLAN_REGISTER2,0x00390001); // 1110010000000000000001
    athrs16_reg_write(S16_PORT_BASE_VLAN_REGISTER3,0x00350001); // 1101010000000000000001
    athrs16_reg_write(S16_PORT_BASE_VLAN_REGISTER4,0x002d0001); // 1011010000000000000001

   /* Insert PVID 2 to WAN port */
    athrs16_reg_write(S16_PORT_BASE_VLAN_REGISTER5,0x001d0001); // 0111010000000000000010


#endif

   /* Egress tag packet to CPU and untagged packet to LAN port */

#if 0 // double vlan tag

    athrs16_reg_write(S16_PORT_CONTROL_REGISTER0,0x00004204);
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER1,0x00004104);
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER2,0x00004104);
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER3,0x00004104);
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER4,0x00004104);
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER5,0x00004104);
#else
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER0,0x00006204);
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER1,0x00006104);
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER2,0x00006104);
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER3,0x00006104);
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER4,0x00006104);
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER5,0x00006104);


#endif
  /* Group port - 0,1,2,3 to VID 1 */
    athrs16_reg_write(S16_VLAN_FUNC_REGISTER1,0x0000083d); // 100000111101
    athrs16_reg_write(S16_VLAN_FUNC_REGISTER0,0x0001000a);

  /*  port - 4  to VID 2 */
    athrs16_reg_write(S16_VLAN_FUNC_REGISTER1,0x00000803); // 100000000011
    athrs16_reg_write(S16_VLAN_FUNC_REGISTER0,0x0002000a); 

#endif

#if 0
    printk("Disable switch learning function.\n");
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER0, athrs16_reg_read(S16_PORT_CONTROL_REGISTER0)&(~(0x1<<14)));
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER1, athrs16_reg_read(S16_PORT_CONTROL_REGISTER1)&(~(0x1<<14)));
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER2, athrs16_reg_read(S16_PORT_CONTROL_REGISTER2)&(~(0x1<<14)));
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER3, athrs16_reg_read(S16_PORT_CONTROL_REGISTER3)&(~(0x1<<14)));
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER4, athrs16_reg_read(S16_PORT_CONTROL_REGISTER4)&(~(0x1<<14)));
    athrs16_reg_write(S16_PORT_CONTROL_REGISTER5, athrs16_reg_read(S16_PORT_CONTROL_REGISTER5)&(~(0x1<<14)));
#endif
    printk("athrs16_reg_init complete.\n");

#endif
    athr16_init_flag = 1;
}

/******************************************************************************
*
* athrs16_phy_is_link_alive - test to see if the specified link is alive
*
* RETURNS:
*    TRUE  --> link is alive
*    FALSE --> link is down
*/
BOOL
athrs16_phy_is_link_alive(int phyUnit)
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
* athrs16_phy_setup - reset and setup the PHY associated with
* the specified MAC unit number.
*
* Resets the associated PHY port.
*
* RETURNS:
*    TRUE  --> associated PHY is alive
*    FALSE --> no LINKs on this ethernet unit
*/

BOOL
athrs16_phy_setup(int ethUnit)
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

        phy_reg_write(phyBase, phyAddr, ATHR_1000BASET_CONTROL,
                      ATHR_ADVERTISE_1000FULL);

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
#if 0
	/* Enable RGMII */
	phy_reg_write(0,phyUnit,0x1d,0x12);
	phy_reg_write(0,phyUnit,0x1e,0x8);
	/* Tx delay on PHY */
	phy_reg_write(0,phyUnit,0x1d,0x5);
	phy_reg_write(0,phyUnit,0x1e,0x100);
        
	/* Rx delay on PHY */
	phy_reg_write(0,phyUnit,0x1d,0x0);
	phy_reg_write(0,phyUnit,0x1e,0x8000);
#endif
        if (athrs16_phy_is_link_alive(phyUnit)) {
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
    phy_mode_setup();    
    return (liveLinks > 0);
}

/******************************************************************************
*
* athrs16_phy_is_fdx - Determines whether the phy ports associated with the
* specified device are FULL or HALF duplex.
*
* RETURNS:
*    1 --> FULL
*    0 --> HALF
*/
int
athrs16_phy_is_fdx(int ethUnit)
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

        if (athrs16_phy_is_link_alive(phyUnit)) {

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
* athrs16_phy_speed - Determines the speed of phy ports associated with the
* specified device.
*
* RETURNS:
*               AG7240_PHY_SPEED_10T, AG7240_PHY_SPEED_100TX;
*               AG7240_PHY_SPEED_1000T;
*/

int
athrs16_phy_speed(int ethUnit)
{
    int       phyUnit;
    uint16_t  phyHwStatus;
    uint32_t  phyBase;
    uint32_t  phyAddr;
    int       ii = 200;
    ag7240_phy_speed_t phySpeed = 0;

    for (phyUnit=0; phyUnit < ATHR_PHY_MAX; phyUnit++) {
        if (!ATHR_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }

        phyBase = ATHR_PHYBASE(phyUnit);
        phyAddr = ATHR_PHYADDR(phyUnit);
        phySpeed = AG7240_PHY_SPEED_10T;

        if (athrs16_phy_is_link_alive(phyUnit)) {

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
                phySpeed = AG7240_PHY_SPEED_10T;
                break;
            case 1:
                phySpeed = AG7240_PHY_SPEED_100TX;
                break;
            case 2:
                phySpeed = AG7240_PHY_SPEED_1000T;
                break;
            default:
                printk("Unkown speed read!\n");
            }
        } 

        phy_reg_write(phyBase, phyAddr, ATHR_DEBUG_PORT_ADDRESS, 0x18);
        
        if(phySpeed == AG7240_PHY_SPEED_100TX) {
            phy_reg_write(phyBase, phyAddr, ATHR_DEBUG_PORT_DATA, 0xba8);
        } else {            
            phy_reg_write(phyBase, phyAddr, ATHR_DEBUG_PORT_DATA, 0x2ea);
        }
    }

    if (ethUnit == ENET_UNIT_LAN)
         phySpeed = AG7240_PHY_SPEED_1000T;

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
athrs16_phy_is_up(int ethUnit)
{
    int           phyUnit;
    uint16_t      phyHwStatus, phyHwControl;
    athrPhyInfo_t *lastStatus;
    int           linkCount   = 0;
    int           lostLinks   = 0;
    int           gainedLinks = 0;
    uint32_t      phyBase;
    uint32_t      phyAddr;

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
                printk("\nenet%d port%d up\n",ethUnit, phyUnit);
                lastStatus->isPhyAlive = TRUE;
                }
            }
        }
    }

    return (linkCount);

}

uint32_t
athrs16_reg_read(uint32_t reg_addr)
{
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
}

void
athrs16_reg_write(uint32_t reg_addr, uint32_t reg_val)
{
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
}

void athrs16_reg_dev(ag7240_mac_t **mac)
{
    if( mac[0]) {
    ag7240_macs[0] = mac[0];
    ag7240_macs[0]->mac_speed = 0xff;
    }
    else
      printk("MAC [0] not registered \n");

    if( mac[1]) {
    ag7240_macs[1] = mac[1];
    ag7240_macs[1]->mac_speed = 0xff;
    }
    else
      printk("MAC [1] not registered \n");
    return;

}


int athrs16_ioctl(struct eth_cfg_params *etd, int cmd)
{
//    uint32_t ar7240_revid;
    if(cmd  == S26_RD_PHY) {
        if(etd->portnum != 0xf)
            etd->val = phy_reg_read(0,etd->portnum,etd->phy_reg);
        else
            etd->val = athrs16_reg_read(etd->phy_reg);
    }
    else if(cmd  == S26_WR_PHY) {
        if(etd->portnum != 0xf)
            phy_reg_write(0,etd->portnum,etd->phy_reg,etd->val);
        else
            athrs16_reg_write(etd->phy_reg,etd->val);
    }
    else
        return -EINVAL;

    return 0;
}


