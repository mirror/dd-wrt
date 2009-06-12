/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright ? 2007 Atheros Communications, Inc.,  All Rights Reserved.
 */

/*
 * Manage the atheros ethernet PHY.
 *
 * All definitions in this file are operating system independent!
 */

/*#if defined(linux)
#include <linux/config.h>
#include <linux/types.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/delay.h>

#include "ar531xlnx.h"
#endif
*/
#if defined(__ECOS)
#include "ae531xecos.h"
#endif

#include "ae531xmac.h"
#include "ae531xreg.h"
#include "athrs26_phy.h"

//KW
#ifdef HEADER_EN
#undef HEADER_EN
#endif

#ifdef HEADER_REG_CONF
#undef HEADER_REG_CONF
#endif

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

/*****************************Device ID***************************/
#define DEVICE_ID_OFFSET 0x0
#define LOAD_EEPROM_E_BOFFSET 16

/*
 * Track per-PHY port information.
 */
typedef struct {
    BOOL   isEnetPort;       /* normal enet port */
    BOOL   isPhyAlive;       /* last known state of link */
    int    ethUnit;          /* MAC associated with this phy port */
    UINT32 phyBase;
    UINT32 phyAddr;          /* PHY registers associated with this phy port */
    UINT32 VLANTableSetting; /* Value to be written to VLAN table */
} athrPhyInfo_t;

/*
 * Per-PHY information, indexed by PHY unit number.
 */
static athrPhyInfo_t athrPhyInfo[] = {
    {TRUE,   /* phy port 0 -- LAN port 0 */
     FALSE,
     ENET_UNIT_LAN,
     (UINT32) (PHYS_TO_K1(AR2316_ENET0)+AE531X_PHY_OFFSET),
     ATHR_PHY0_ADDR,
     ATHR_LAN_PORT_VLAN
    },

    {TRUE,   /* phy port 1 -- LAN port 1 */
     FALSE,
     ENET_UNIT_LAN,
     (UINT32) (PHYS_TO_K1(AR2316_ENET0)+AE531X_PHY_OFFSET),
     ATHR_PHY1_ADDR,
     ATHR_LAN_PORT_VLAN
    },

    {TRUE,   /* phy port 2 -- LAN port 2 */
     FALSE,
     ENET_UNIT_LAN,
     (UINT32) (PHYS_TO_K1(AR2316_ENET0)+AE531X_PHY_OFFSET),
     ATHR_PHY2_ADDR, 
     ATHR_LAN_PORT_VLAN
    },

    {TRUE,   /* phy port 3 -- LAN port 3 */
     FALSE,
     ENET_UNIT_LAN,
     (UINT32) (PHYS_TO_K1(AR2316_ENET0)+AE531X_PHY_OFFSET),
     ATHR_PHY3_ADDR, 
     ATHR_LAN_PORT_VLAN
    },

    {TRUE,   /* phy port 4 -- WAN port or LAN port 4 */
     FALSE,
     1,
     (UINT32) (PHYS_TO_K1(AR2316_ENET0)+AE531X_PHY_OFFSET),
     ATHR_PHY4_ADDR, 
     ATHR_LAN_PORT_VLAN   /* Send to all ports */
    },
    
    {FALSE,  /* phy port 5 -- CPU port (no RJ45 connector) */
     TRUE,
     ENET_UNIT_LAN,
     (UINT32) (PHYS_TO_K1(AR2316_ENET0)+AE531X_PHY_OFFSET),
     0x00, 
     ATHR_LAN_PORT_VLAN    /* Send to all ports */
    },
};

static UINT8 athr26_init = 0;

#ifdef HEADER_EN
typedef struct {
    UINT8 data[HEADER_MAX_DATA];
    UINT8 len;
    UINT32 seq;
} cmd_resp_t;

static cmd_resp_t cmd_resp;
static DECLARE_WAIT_QUEUE_HEAD (hd_conf_wait);
static int wait_flag = 0;
static ag7100_mac_t *lan_mac;
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
BOOL       athrs26_phy_is_link_alive(int phyUnit);
static UINT32 athrs26_reg_read(UINT16 reg_addr);
static void athrs26_reg_write(UINT16 reg_addr, UINT32 reg_val);

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
    UINT16 phyHwStatus;
    UINT32 phyBase;
    UINT32 phyAddr;

    phyBase = ATHR_PHYBASE(phyUnit);
    phyAddr = ATHR_PHYADDR(phyUnit);

    phyHwStatus = phyRegRead(phyBase, phyAddr, ATHR_PHY_SPEC_STATUS);

    if (phyHwStatus & ATHR_STATUS_LINK_PASS)
        return TRUE;

    return FALSE;
}

void athrs26_reg_init()
{
    UINT32 phyval;
    if (athr26_init)
        return;
    DEBUGOUT("athrs26_reg_init");
    athrs26_reg_write(0x200, 0x200);
    athrs26_reg_write(0x300, 0x200);
    athrs26_reg_write(0x400, 0x200);
    athrs26_reg_write(0x500, 0x200);
    athrs26_reg_write(0x600, 0x200);

#ifdef S26_VER_1_0
    phyRegWrite(ATHR_PHYBASE(0), 0, 29, 41);
    phyRegWrite(ATHR_PHYBASE(0), 0, 30, 0);
    phyRegWrite(ATHR_PHYBASE(0), 1, 29, 41);
    phyRegWrite(ATHR_PHYBASE(0), 1, 30, 0);
    phyRegWrite(ATHR_PHYBASE(0), 2, 29, 41);
    phyRegWrite(ATHR_PHYBASE(0), 2, 30, 0);
    phyRegWrite(ATHR_PHYBASE(0), 3, 29, 41);
    phyRegWrite(ATHR_PHYBASE(0), 3, 30, 0);
    phyRegWrite(ATHR_PHYBASE(0), 4, 29, 41);
    phyRegWrite(ATHR_PHYBASE(0), 4, 30, 0);
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
phySetup(int ethUnit, UINT32 _phyBase)
{
    int       phyUnit;
    UINT16  phyHwStatus;
    UINT16  timeout;
    int       liveLinks = 0;
    UINT32  phyBase = 0;
    BOOL      foundPhy = FALSE;
    UINT32  phyAddr = 0;
    

    /* See if there's any configuration data for this enet */
    /* start auto negogiation on each phy */
    DEBUGOUT("start auto negogiation on each phy");
    for (phyUnit=0; phyUnit < ATHR_PHY_MAX; phyUnit++) {
        if (!ATHR_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }
        
        foundPhy = TRUE;
        phyBase = ATHR_PHYBASE(phyUnit);
        phyAddr = ATHR_PHYADDR(phyUnit);
        
        phyRegWrite(phyBase, phyAddr, ATHR_AUTONEG_ADVERT,
                      ATHR_ADVERTISE_ALL);

        /* Reset PHYs*/
        phyRegWrite(phyBase, phyAddr, ATHR_PHY_CONTROL,
                      ATHR_CTRL_AUTONEGOTIATION_ENABLE 
                      | ATHR_CTRL_SOFTWARE_RESET);
    }

    if (!foundPhy) {
        DEBUGOUT("No PHY's configured for this ethUnit");
        return FALSE; /* No PHY's configured for this ethUnit */
    }
    
    /*
     * After the phy is reset, it takes a little while before
     * it can respond properly.
     */
//    sysMsDelay(1000);

#ifndef HEADER_REG_CONF 
    /* if using header for register configuration, we have to     */
    /* configure s26 register after frame transmission is enabled */
    if (ethUnit == ENET_UNIT_LAN) {
        athrs26_reg_init();
    }
#endif
    
#ifdef DEBUG_CMD  
    athena_reg_access_init();
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
phyIsFullDuplex(int ethUnit)
{
    int       phyUnit;
    UINT32  phyBase;
    UINT32  phyAddr;
    UINT16  phyHwStatus;
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
                phyHwStatus = phyRegRead(phyBase, phyAddr, 
                                               ATHR_PHY_SPEC_STATUS);
                if (!(phyHwStatus & ATHR_STATUS_RESOVLED))
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

BOOL 
phyIsSpeed100(int ethUnit)
{
    int       phyUnit;
    UINT16  phyHwStatus;
    UINT32  phyBase;
    UINT32  phyAddr;
    int       ii = 200;

    //KW
    //if (ethUnit == ENET_UNIT_LAN)
    //    return AG7100_PHY_SPEED_100TX;

    for (phyUnit=0; phyUnit < ATHR_PHY_MAX; phyUnit++) {
        if (!ATHR_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }

        if (athrs26_phy_is_link_alive(phyUnit)) {

            phyBase = ATHR_PHYBASE(phyUnit);
            phyAddr = ATHR_PHYADDR(phyUnit);
            do {
                phyHwStatus = phyRegRead(phyBase, phyAddr, 
                                              ATHR_PHY_SPEC_STATUS);
                if (!(phyHwStatus & ATHR_STATUS_RESOVLED))
            	    sysMsDelay(10);
            }while((!(phyHwStatus & ATHR_STATUS_RESOVLED)) && --ii);
            
            phyHwStatus = ((phyHwStatus & ATHER_STATUS_LINK_MASK) >>
                           ATHER_STATUS_LINK_SHIFT);

            switch(phyHwStatus) {
            case 0:
                return FALSE;
            case 1:
                return TRUE;
            default:
                printk("Unkown speed read!\n");
            }
        }
    }
    //KW
    //return AG7100_PHY_SPEED_10T;
    return FALSE;
}

/*****************************************************************************
*
* athrs26_phyCheckStatusChange -- checks for significant changes in PHY state.
*
* A "significant change" is:
*     dropped link (e.g. ethernet cable unplugged) OR
*     autonegotiation completed + link (e.g. ethernet cable plugged in)
*
* When a PHY is plugged in, phyLinkGained is called.
* When a PHY is unplugged, phyLinkLost is called.
*/

void
athrs26_phyCheckStatusChange(int ethUnit)
{
    int           phyUnit;
    UINT16      phyHwStatus;
    athrPhyInfo_t *lastStatus;
    int           linkCount   = 0;
    int           lostLinks   = 0;
    int           gainedLinks = 0;
    UINT32      phyBase;
    UINT32      phyAddr;

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
        phyHwStatus = phyRegRead(phyBase, phyAddr, ATHR_PHY_SPEC_STATUS);

        if (lastStatus->isPhyAlive) { /* last known link status was ALIVE */
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
            phyHwStatus = phyRegRead(phyBase, phyAddr, ATHR_PHY_STATUS);
            if (!ATHR_RESET_DONE(phyHwStatus))
                continue;

            /* Check for AutoNegotiation complete */            
            if (ATHR_AUTONEG_DONE(phyHwStatus)) {
                //printk("autoneg done\n");
                gainedLinks++;
                linkCount++;
                DRV_PRINT(DRV_DEBUG_PHYCHANGE,("\nenet%d port%d up\n",
                                               ethUnit, phyUnit));
                lastStatus->isPhyAlive = TRUE;
            }
        }
    }

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
}

#ifdef HEADER_EN
#ifdef HEADER_REG_CONF
static int
athrs26_header_config_reg (struct net_device *dev, UINT8 wr_flag, 
                           UINT16 reg_addr, UINT16 cmd_len,
                           UINT8 *val) 
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
athrs26_header_write_reg(UINT16 reg_addr, UINT16 cmd_len, UINT8 *reg_data)
{
    long timeout;
    int i = 2;

    atomic_inc(&seqcnt);
    do {
        athrs26_header_config_reg(lan_mac->mac_dev, 0, reg_addr, cmd_len, reg_data);
        timeout = HZ;     
        timeout = wait_event_interruptible_timeout (hd_conf_wait, 
                                                    wait_flag != 0, timeout);
        wait_flag = 0;
        if (timeout) 
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
athrs26_header_read_reg(UINT16 reg_addr, UINT16 cmd_len, UINT8 *reg_data)
{
    long timeout;
    int i = 2;

    atomic_inc(&seqcnt);
    do {
        athrs26_header_config_reg(lan_mac->mac_dev, 1, reg_addr, cmd_len, reg_data);
        timeout = HZ;  
        timeout = wait_event_interruptible_timeout (hd_conf_wait, 
                                                    wait_flag != 0, timeout);
           
        wait_flag = 0;    
        if (timeout) 
            break;
        else 
            printk("write time out\n");
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

void athrs26_reg_dev(ag7100_mac_t *mac)
{
    lan_mac = mac;
}
#endif

static UINT32
athrs26_reg_read(UINT16 reg_addr)
{
#ifndef HEADER_REG_CONF	
    UINT16 reg_word_addr, phy_val, phy_val1;
    UINT32 phy_addr;
    UINT8  phy_reg; 
    
    /* read the first 16 bits*/
    reg_word_addr = (reg_addr / 4) * 2;
    /* configure register high address */
    phy_addr = 0x18;
    phy_reg = 0x0;
    phy_val = (reg_word_addr >> 8) & 0x1ff;         /* bit16-8 of reg address*/
    phyRegWrite (ATHR_PHYBASE(0), phy_addr, phy_reg, phy_val);

    /* read register with low address */
    phy_addr = 0x10 | ((reg_word_addr >> 5) & 0x7); /* bit7-5 of reg address */
    phy_reg = reg_word_addr & 0x1f;                 /* bit4-0 of reg address */
    phy_val = phyRegRead(ATHR_PHYBASE(0), phy_addr, phy_reg);

    /* read the second 16 bits*/
    reg_word_addr++;
    /* configure register high address */
    phy_addr = 0x18;
    phy_reg = 0x0;
    phy_val1 = (reg_word_addr >> 8) & 0x1ff;         /* bit16-8 of reg address*/
    phyRegWrite (ATHR_PHYBASE(0), phy_addr, phy_reg, phy_val);

    /* read register with low address */
    phy_addr = 0x10 | ((reg_word_addr >> 5) & 0x7); /* bit7-5 of reg address */
    phy_reg = reg_word_addr & 0x1f;                 /* bit4-0 of reg address */
    phy_val1 = phyRegRead(ATHR_PHYBASE(0), phy_addr, phy_reg);
        
    return (phy_val1 << 16) | phy_val;
#else
    UINT8 reg_data[4];
    
    memset (reg_data, 0, 4);
    athrs26_header_read_reg(reg_addr, 4, reg_data);
    return (reg_data[0] | (reg_data[1] << 8) | (reg_data[2] << 16) | (reg_data[3] << 24));
#endif    
}

static void
athrs26_reg_write(UINT16 reg_addr, UINT32 reg_val)
{
#ifndef HEADER_REG_CONF
    
    UINT16 reg_word_addr, phy_val;
    UINT32 phy_addr;
    UINT8  phy_reg; 
    DEBUGOUT("athrs26_reg_write, not define HEADER_REG_CONF");
    /* write the first 16 bits*/
    reg_word_addr = (reg_addr / 4) * 2;    
    /* configure register high address */
    phy_addr = 0x18;
    phy_reg = 0x0;
    phy_val = (reg_word_addr >> 8) & 0x1ff;         /* bit16-8 of reg address*/
    DEBUGOUT("athrs26_reg_write ->phyRegWrite");
    phyRegWrite (ATHR_PHYBASE(0), phy_addr, phy_reg, phy_val);

    /* read register with low address */
    phy_addr = 0x10 | ((reg_word_addr >> 5) & 0x7); /* bit7-5 of reg address */
    phy_reg = reg_word_addr & 0x1f;                 /* bit4-0 of reg address */
    phy_val = reg_val & 0xffff;
    phyRegWrite (ATHR_PHYBASE(0), phy_addr, phy_reg, phy_val);
    
    /* write the second 16 bits*/
    reg_word_addr++;    
    /* configure register high address */
    phy_addr = 0x18;
    phy_reg = 0x0;
    phy_val = (reg_word_addr >> 8) & 0x1ff;         /* bit16-8 of reg address*/
    phyRegWrite (ATHR_PHYBASE(0), phy_addr, phy_reg, phy_val);

    /* read register with low address */
    phy_addr = 0x10 | ((reg_word_addr >> 5) & 0x7); /* bit7-5 of reg address */
    phy_reg = reg_word_addr & 0x1f;                 /* bit4-0 of reg address */
    phy_val = (reg_val >> 16) & 0xffff;
    phyRegWrite (ATHR_PHYBASE(0), phy_addr, phy_reg, phy_val);   
#else
    UINT8 reg_data[4];
    
    memset (reg_data, 0, 4);    
    DEBUGOUT("athrs26_reg_write, define HEADER_REG_CONF");
    reg_data[0] = (UINT8)(0x00ff & reg_val); 
    reg_data[1] = (UINT8)((0xff00 & reg_val) >> 8);      
    reg_data[2] = (UINT8)((0xff0000 & reg_val) >> 16);
    reg_data[3] = (UINT8)((0xff000000 & reg_val) >> 24);
    athrs26_header_write_reg (reg_addr, 4, reg_data);
#endif
}


int
athr_ioctl(UINT32 unit, UINT32 *args)
{
#ifdef DEBUG_CMD
    switch (args[0]) {
    case ATHRCGREG: {
        UINT32 reg_val;
        
        reg_val = (UINT32)athrs26_reg_read(args[1]);
        if (copy_to_user((void __user *)args[2], &reg_val, 
            sizeof(UINT32)))
          return EFAULT;

        return 0;
    }

    case ATHRCSREG: {
        athrs26_reg_write ((UINT16)args[1], (UINT32)args[2]);
        return 0;
    }

    case ATHRCGETY: {
        UINT32 reg_addr, cmd_num, index;
        UINT16 entry_len;
        UINT8 reg_tmp[4];
        UINT32 reg_val;
        ebu_hsl_reg_t *p_reg = (ebu_hsl_reg_t *)athena_reg_ptr_get();
        EBU_STATUS rtn;

        if (!p_reg)
            return ENXIO;
        
        cmd_num = args[1];
        index = args[2];  

        reg_addr = reg_desc[cmd_num].reg_addr;
        entry_len = 4;
        
        rtn = p_reg->reg_entry_get (0, reg_addr, 
                                    index * reg_desc[cmd_num].entry_offset,
                                    entry_len, reg_tmp); 
        
        if (rtn == EBU_ERROR)
            return EINVAL;

        reg_val = ((UINT32)((reg_tmp[0] << 24) & 0xff000000)) | 
                  ((UINT32)((reg_tmp[1] << 16) & 0x00ff0000)) |
                  ((UINT32)((reg_tmp[2] << 8) & 0x0000ff00)) | 
                  ((UINT32)reg_tmp[3]) ;

      if (copy_to_user((void __user *)args[3], &reg_val,
            sizeof(UINT32)))
        return EFAULT;

        return 0;
    }
  
    case ATHRCSETY: {
        UINT32 reg_addr, cmd_num, index;
        UINT16 entry_len;
        UINT8 reg_tmp[4];
        ebu_hsl_reg_t *p_reg = (ebu_hsl_reg_t *)athena_reg_ptr_get(); 
        EBU_STATUS rtn;
       
        if (!p_reg)
            return ENXIO;
        
        cmd_num = args[1];
        index = args[2];
    
        reg_addr = reg_desc[cmd_num].reg_addr;
        entry_len = 4;
        reg_tmp[0] = (UINT8)((args[3] & 0xff000000) >> 24);//reg address high
        reg_tmp[1] = (UINT8)((args[3] & 0x00ff0000) >> 16);
        reg_tmp[2] = (UINT8)((args[3] & 0x0000ff00) >> 8);//reg address low
        reg_tmp[3] = (UINT8)(args[3] & 0x000000ff);
    
        rtn = p_reg->reg_entry_set (0, reg_addr, (UINT16)
                                    (index * reg_desc[cmd_num].entry_offset),
                                    entry_len, reg_tmp);
        if (rtn == EBU_ERROR)
            return EINVAL;
        
        return 0;
    }
        
    case ATHRCGFLD: {
        UINT32 reg_addr, cmd_num, index;
        UINT16 entry_offset, field_offset, field_len;
        UINT32 field_val;
        UINT8 field_tmp[4];
        ebu_hsl_reg_t *p_reg = (ebu_hsl_reg_t *)athena_reg_ptr_get(); 
        EBU_STATUS rtn;

        if (!p_reg)
            return ENXIO;
        
        cmd_num = args[1];
        index = args[2];  

        memset(field_tmp, 0, sizeof(field_tmp));
        reg_addr = reg_desc[field_desc[cmd_num].reg_id].reg_addr;
        field_len = field_desc[cmd_num].bit_len;
        field_offset = field_desc[cmd_num].bit_offset;
        entry_offset = index * 
                       reg_desc[field_desc[cmd_num].reg_id].entry_offset;        
        rtn = p_reg->reg_field_get (0, reg_addr, entry_offset,
                                    field_offset, field_len, field_tmp);
        
        if (rtn == EBU_ERROR)
            return EINVAL;

        switch ((field_len-1) / 8) {
        case 0:
            field_val = field_tmp[0];
            break;
            
        case 1:
            field_val = ((UINT32)((field_tmp[0] << 8) & 0xff00)) | 
                        ((UINT32)field_tmp[1]);
            break;
            
        case 2:
            field_val = ((UINT32)((field_tmp[0] << 16) & 0xff0000)) | 
                        ((UINT32)((field_tmp[1] << 8) & 0xff00)) |                 
                        ((UINT32)field_tmp[2]);
            break;
            
        case 3:
            field_val = ((UINT32)((field_tmp[0] << 24) & 0xff000000)) | 
                        ((UINT32)((field_tmp[1] << 16) & 0xff0000)) |                 
                        ((UINT32)((field_tmp[2] << 8) & 0xff00)) |                 
                        ((UINT32)field_tmp[3]);
            break;
            
        default:
            return EINVAL;
        }
        
      if (copy_to_user((void __user *)args[3], &field_val,
                       sizeof(UINT32)))
        return EFAULT;

        return 0;          
    }
  
    case ATHRCSFLD:
    {
        UINT32 reg_addr, cmd_num, index;
        UINT16 entry_offset, field_offset, field_len;
        UINT8 field_tmp[4];
        ebu_hsl_reg_t *p_reg = (ebu_hsl_reg_t *)athena_reg_ptr_get(); 
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
        memset(field_tmp, 0, sizeof(field_tmp));
        
        switch ((field_len-1) / 8) {
        case 0:
            field_tmp[0] = (UINT8)args[3];
            break;
            
        case 1:
            field_tmp[0] = (UINT8)((0xff00 & args[3]) >> 8);
            field_tmp[1] = (UINT8)(0x00ff & args[3]);     
            break;
            
        case 2:
            field_tmp[0] = (UINT8)((0xff0000 & args[3]) >> 16);
            field_tmp[1] = (UINT8)((0xff00 & args[3]) >> 8);      
            field_tmp[2] = (UINT8)(0x00ff & args[3]);   
            break;
            
        case 3:
            field_tmp[0] = (UINT8)((0xff0000 & args[3]) >> 24);
            field_tmp[1] = (UINT8)((0xff0000 & args[3]) >> 16);
            field_tmp[2] = (UINT8)((0xff00 & args[3]) >> 8);      
            field_tmp[3] = (UINT8)(0x00ff & args[3]); 

            break;
            
        default:
            return EBU_ERROR;
        }

        rtn = p_reg->reg_field_set (0, reg_addr, entry_offset,
                                    field_offset, field_len, field_tmp);
        
        if (rtn == EBU_ERROR)
            return EINVAL;
    
        return 0;
    }

    default:
        return -EOPNOTSUPP;
    }
#endif /* DEBUG_CMD*/
    //KW
    //return -EOPNOTSUPP;
    return TRUE;
}


