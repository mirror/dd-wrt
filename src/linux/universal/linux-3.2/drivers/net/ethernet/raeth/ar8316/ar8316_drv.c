#include <linux/init.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

//#include <linux/delay.h>

#include "ralink_gpio.h"
#include "ra_ioctl.h"

#ifdef  CONFIG_DEVFS_FS
#include <linux/devfs_fs_kernel.h>
static  devfs_handle_t devfs_handle;
#endif

#define NAME		"ar8316"
#define AR8316_DEVNAME	"ar8316"
int ar8316_major = 207;

MODULE_DESCRIPTION("Realtek ar8316 support");
MODULE_AUTHOR("Jiahao");
MODULE_LICENSE("GPL");

#include <linux/types.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/delay.h>

//#include "ag7100_phy.h"
//#include "ag7100.h"
#include "athrs16_phy.h"

//#ifdef FULL_FEATURE
#if 0
extern void* ssdk_mdio_set;
extern void* ssdk_mdio_get;
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

u32 mii_mgr_read(u32 phy_addr, u32 phy_register, u32 *read_data);
u32 mii_mgr_write(u32 phy_addr, u32 phy_register, u32 write_data);
u32 ar_mii_mgr_read(u32 phy_addr, u32 phy_register);

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
     ENET_UNIT_WAN,
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
static uint32_t athrs16_reg_read(uint32_t reg_addr);
static void athrs16_reg_write(uint32_t reg_addr, uint32_t reg_val);

uint32_t phy_reg_read(uint32_t phy_base, uint32_t phy_addr, uint32_t phy_reg)
{
	return ar_mii_mgr_read(phy_base + phy_addr, phy_reg);
}

void phy_reg_write(uint32_t phy_base, uint32_t phy_addr, uint32_t phy_reg, uint32_t reg_val)
{
	mii_mgr_write(phy_base + phy_addr, phy_reg, reg_val);
}

void phy_mode_setup(void) 
{
    printk("phy_mode_setup\n");

    /*work around for phy4 rgmii mode*/
    phy_reg_write(ATHR_PHYBASE(ATHR_IND_PHY), ATHR_PHYADDR(ATHR_IND_PHY), 29, 18);     
    phy_reg_write(ATHR_PHYBASE(ATHR_IND_PHY), ATHR_PHYADDR(ATHR_IND_PHY), 30, 0x480c);    

    /*rx delay*/ 
    phy_reg_write(ATHR_PHYBASE(ATHR_IND_PHY), ATHR_PHYADDR(ATHR_IND_PHY), 29, 0);     
    phy_reg_write(ATHR_PHYBASE(ATHR_IND_PHY), ATHR_PHYADDR(ATHR_IND_PHY), 30, 0x824e);  

    /*tx delay*/ 
    phy_reg_write(ATHR_PHYBASE(ATHR_IND_PHY), ATHR_PHYADDR(ATHR_IND_PHY), 29, 5);     
    phy_reg_write(ATHR_PHYBASE(ATHR_IND_PHY), ATHR_PHYADDR(ATHR_IND_PHY), 30, 0x3c47);    

}

static BOOL
athrs16_bist_test(void)
{
    uint32_t times = 2, regc = 0xa0, regr = 0xa4, data = 0, i;

    while (--times) {
        athrs16_reg_write(regc, 0x80700000);

        i = 0x1000;
        while(i--) {
            if(!(athrs16_reg_read(regc)&0x80000000))
                break;
            udelay(5);
        }

        if (0 == i)
            return FALSE;

        data = athrs16_reg_read(regc);

        if(!(data & 0xe000))
            break;
        
        if (!times || !(data & 0x40000000))
            return FALSE;
        
        athrs16_reg_write(regr, (data & 0x1fff) | 0x80000000);

    }

    /* test pass then reset switch */
    printk(MODULE_NAME ": resetting...\n");
    athrs16_reg_write(0x0, athrs16_reg_read(0x0)|0x80000000);

    i = 20;
    while(i--) {
        mdelay(100);
        if(!(athrs16_reg_read(0x0)&0x80000000))
            break;
    }

    if (i)
        printk(MODULE_NAME ": reset done.\n");
    else
        printk(MODULE_NAME ": reset fail.\n"); 

    return TRUE;
}

void athrs16_reg_init()
{
    /* if using header for register configuration, we have to     */
    /* configure s16 register after frame transmission is enabled */
    if (athr16_init_flag)
        return;

//#ifdef FULL_FEATURE
#if 0
    ssdk_mdio_set = phy_reg_write;
    ssdk_mdio_get = phy_reg_read;
    ssdk_reduced_init(0, 2, 1);
#else
    if(FALSE == athrs16_bist_test()) {
        printk(MODULE_NAME ": bist test fail.\n");
    }
#endif

    /*Power on strip mode setup*/

    
    athrs16_reg_write(0x100, 0x7e);
    athrs16_reg_write(0x600, 0x0);
    athrs16_reg_write(0x8, 0x010e1bea);


    athrs16_reg_write(0x2c, 0x043f003f);

#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)        
#ifdef HEADER_EN        
    athrs16_reg_write(0x104, 0x6804);
#else
    athrs16_reg_write(0x104, 0x6004);
#endif

    athrs16_reg_write(0x204, 0x6004);
    athrs16_reg_write(0x304, 0x6004);
    athrs16_reg_write(0x404, 0x6004);
    athrs16_reg_write(0x504, 0x6004);    
    
#else
#ifdef HEADER_EN        
    athrs16_reg_write(0x104, 0x4804);
#else
    athrs16_reg_write(0x104, 0x4004);
#endif
#endif

    printk(MODULE_NAME ": athrs16_reg_init complete.\n");

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
*               AG7100_PHY_SPEED_10T, AG7100_PHY_SPEED_100TX;
*               AG7100_PHY_SPEED_1000T;
*/

/*
int
athrs16_phy_speed(int ethUnit)
{
    int       phyUnit;
    uint16_t  phyHwStatus;
    uint32_t  phyBase;
    uint32_t  phyAddr;
    int       ii = 200;
    ag7100_phy_speed_t phySpeed = AG7100_PHY_SPEED_10T;

    for (phyUnit=0; phyUnit < ATHR_PHY_MAX; phyUnit++) {
        if (!ATHR_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }

        phyBase = ATHR_PHYBASE(phyUnit);
        phyAddr = ATHR_PHYADDR(phyUnit);
        phySpeed = AG7100_PHY_SPEED_10T;

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
         phySpeed = AG7100_PHY_SPEED_1000T;

    return phySpeed;
}
*/

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
                lastStatus->isPhyAlive = TRUE;
                }
            }
        }
    }

    return (linkCount);

}

static uint32_t
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

static void
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

int
athr_ioctl(uint32_t *args, int cmd)
{
//#ifdef FULL_FEATURE
#if 0
    if (sw_ioctl(args, cmd))
        return -EOPNOTSUPP;

    return 0;
#else
    printk("EOPNOTSUPP\n");
    return -EOPNOTSUPP;
#endif
}

#define GPIO_DIR_OUT	1
#define GPIO_DIR_IN	0

int rt_gpio_ioctl(unsigned int req, int idx, unsigned long arg)
{
	unsigned long tmp;

	req &= RALINK_GPIO_DATA_MASK;

	switch(req) {
	case RALINK_GPIO_READ_BIT:
//		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODATA));
		if (idx <= 23)
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODATA));
		else if (idx <= 39)
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924DATA));
		else
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO5140DATA));

//		if (0L <= idx && idx < RALINK_GPIO_DATA_LEN) {
//			tmp = (tmp >> idx) & 1L;
//			return tmp;
//		}
		if (idx <= 23)
			tmp = (tmp >> idx) & 1L;
		else if (idx <= 39)
			tmp = (tmp >> (idx-24)) & 1L;
		else
			tmp = (tmp >> (idx-40)) & 1L;
		return tmp;

		break;
	case RALINK_GPIO_WRITE_BIT:
//		if (0L <= idx && idx < RALINK_GPIO_DATA_LEN) {
//			tmp =le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODATA));
//			if (arg & 1L)
//				tmp |= (1L << idx);
//			else
//				tmp &= ~(1L << idx);
//			*(volatile u32 *)(RALINK_REG_PIODATA)= cpu_to_le32(tmp);
//		}

		if (idx <= 23) {
			tmp =le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODATA));
			if (arg & 1L)
				tmp |= (1L << idx);
			else
				tmp &= ~(1L << idx);
			*(volatile u32 *)(RALINK_REG_PIODATA)= cpu_to_le32(tmp);
		}
		else if (idx <= 39) {
			tmp =le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924DATA));
			if (arg & 1L)
				tmp |= (1L << (idx-24));
			else
				tmp &= ~(1L << (idx-24));
			*(volatile u32 *)(RALINK_REG_PIO3924DATA)= cpu_to_le32(tmp);
		}
		else {
			tmp =le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO5140DATA));
			if (arg & 1L)
				tmp |= (1L << (idx-40));
			else
				tmp &= ~(1L << (idx-40));
			*(volatile u32 *)(RALINK_REG_PIO5140DATA)= cpu_to_le32(tmp);
		}

		break;
	default:
		return -1;
	}
	return 0;
}

int
ralink_gpio_write_bit(int idx, int value)
{
	unsigned int req;
	value &= 1;
       
	if (0L <= idx && idx < RALINK_GPIO_NUMBER)
		req = RALINK_GPIO_WRITE_BIT;
	else
		return -1;

	return rt_gpio_ioctl(req, idx, value);
}

int
ralink_gpio_read_bit(int idx, int *value)
{
	unsigned int req;
	*value = 0;

	if (0L <= idx && idx < RALINK_GPIO_NUMBER)
		req = RALINK_GPIO_READ_BIT;
	else
		return -1;

	if ((*value = rt_gpio_ioctl(req, idx, value)) < 0)
		return -1;

	return *value;
}

int
ralink_initGpioPin(unsigned int idx, int dir)
{
	unsigned long tmp;

	if (idx < 0 || RALINK_GPIO_NUMBER <= idx)
		return -1;

//	tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODIR));
	if (dir == GPIO_DIR_OUT)
	{
//		tmp |= (1L << idx);
		if (idx <= 23) {
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODIR));
			tmp |= (1L << idx);
		}
		else if (idx <= 39) {
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924DIR));
			tmp |= (1L << (idx-24));
		}
		else {
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO5140DIR));
			tmp |= (1L << (idx-40));
		}
	}
	else if (dir == GPIO_DIR_IN)
	{		
//		tmp &= ~(1L << idx);
		if (idx <= 23) {
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODIR));
			tmp &= ~(1L << idx);
		}
		else if (idx <= 39) {
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924DIR));
			tmp &= ~(1L << (idx-24));
		}
		else {
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO5140DIR));
			tmp &= ~(1L << (idx-40));
		}
	}
	else
		return -1;

	if (idx <= 23) {
		*(volatile u32 *)(RALINK_REG_PIODIR) = cpu_to_le32(tmp);
	}
	else if (idx <= 39) {
		*(volatile u32 *)(RALINK_REG_PIO3924DIR) = cpu_to_le32(tmp);
	}
	else {
		*(volatile u32 *)(RALINK_REG_PIO5140DIR) = cpu_to_le32(tmp);
	}

	return 0;
}

typedef struct {
	unsigned int idx;
	unsigned int value;
} asus_gpio_info;

int ar8316_ioctl(struct inode *inode, struct file *file, unsigned int req,
		unsigned long arg)
{
	unsigned long tmp;
	asus_gpio_info info;
	ra_mii_ioctl_data mii;

	switch(req) {
	case RAETH_MII_READ:
		copy_from_user(&mii, (ra_mii_ioctl_data *)arg, sizeof(mii));
		mii_mgr_read(mii.phy_id, mii.reg_num, &mii.val_out);
		printk("phy %d, reg %d, val 0x%x\n", mii.phy_id, mii.reg_num, mii.val_out);
		copy_to_user((ra_mii_ioctl_data *)arg, &mii, sizeof(mii));
		break;

	case RAETH_MII_WRITE:
		copy_from_user(&mii, (ra_mii_ioctl_data *)arg, sizeof(mii));
		printk("phy %d, reg %d, val 0x%x\n", mii.phy_id, mii.reg_num, mii.val_in);
		mii_mgr_write(mii.phy_id, mii.reg_num, mii.val_in);
		break;

	case RALINK_GPIO_SET_DIR:	// 0x01
		copy_from_user(&info, (asus_gpio_info *)arg, sizeof(info));
		if (info.idx < 0 || RALINK_GPIO_NUMBER <= info.idx)
			return -1;

		if (info.value == GPIO_DIR_OUT)
		{
			if (info.idx <= 23) {
				tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODIR));
				tmp |= (1L << info.idx);
			}
			else if (info.idx <= 39) {
				tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924DIR));
				tmp |= (1L << (info.idx-24));
			}
			else {
				tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO5140DIR));
				tmp |= (1L << (info.idx-40));
			}
		}
		else if (info.value == GPIO_DIR_IN)
		{		
			if (info.idx <= 23) {
				tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODIR));
				tmp &= ~(1L << info.idx);
			}
			else if (info.idx <= 39) {
				tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924DIR));
				tmp &= ~(1L << (info.idx-24));
			}
			else {
				tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO5140DIR));
				tmp &= ~(1L << (info.idx-40));
			}
		}
		else
			return -1;
	
		if (info.idx <= 23) {
			*(volatile u32 *)(RALINK_REG_PIODIR) = cpu_to_le32(tmp);
		}
		else if (info.idx <= 39) {
			*(volatile u32 *)(RALINK_REG_PIO3924DIR) = cpu_to_le32(tmp);
		}
		else {
			*(volatile u32 *)(RALINK_REG_PIO5140DIR) = cpu_to_le32(tmp);
		}
	
		return 0;

		break;
	case RALINK_GPIO_READ_BIT:	// 0x04
//		copy_from_user(&info, (asus_gpio_info *)arg, sizeof(info));
		copy_from_user(&info.idx, (int __user *)arg, sizeof(int));

		if (info.idx <= 23)
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODATA));
		else if (info.idx <= 39)
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924DATA));
		else
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO5140DATA));

		if (info.idx <= 23)
			tmp = (tmp >> info.idx) & 1L;
		else if (info.idx <= 39)
			tmp = (tmp >> (info.idx-24)) & 1L;
		else
			tmp = (tmp >> (info.idx-40)) & 1L;
//		return tmp;
//		put_user(tmp, (int __user *)&((*(asus_gpio_info *)arg).value));
		put_user(tmp, (int __user *)arg);

		break;
	case RALINK_GPIO_WRITE_BIT:	// 0x05
		copy_from_user(&info, (asus_gpio_info *)arg, sizeof(info));
		if (info.idx <= 23) {
			tmp =le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODATA));
			if (info.value & 1L)
				tmp |= (1L << info.idx);
			else
				tmp &= ~(1L << info.idx);
			*(volatile u32 *)(RALINK_REG_PIODATA)= cpu_to_le32(tmp);
		}
		else if (info.idx <= 39) {
			tmp =le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924DATA));
			if (info.value & 1L)
				tmp |= (1L << (info.idx-24));
			else
				tmp &= ~(1L << (info.idx-24));
			*(volatile u32 *)(RALINK_REG_PIO3924DATA)= cpu_to_le32(tmp);
		}
		else {
			tmp =le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO5140DATA));
			if (info.value & 1L)
				tmp |= (1L << (info.idx-40));
			else
				tmp &= ~(1L << (info.idx-40));
			*(volatile u32 *)(RALINK_REG_PIO5140DATA)= cpu_to_le32(tmp);
		}

		break;
	default:
		printk("[ar8316_ioctl]: unsupported req num\n");
		return -ENOIOCTLCMD;
	}

	return 0;
}

int ar8316_open(struct inode *inode, struct file *file)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
	MOD_INC_USE_COUNT;
#else
	try_module_get(THIS_MODULE);
#endif
	return 0;
}

int ar8316_release(struct inode *inode, struct file *file)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
	MOD_DEC_USE_COUNT;
#else
	module_put(THIS_MODULE);
#endif
	return 0;
}

struct file_operations ar8316_fops =
{
	owner:		THIS_MODULE,
	ioctl:		ar8316_ioctl,
	open:		ar8316_open,
	release:	ar8316_release,
};

int __init ar8316_init(void)
{
#ifdef  CONFIG_DEVFS_FS
	if (devfs_register_chrdev(ar8316_major, AR8316_DEVNAME,
				&ar8316_fops)) {
		printk(KERN_ERR NAME ": unable to register character device\n");
		return -EIO;
	}
	devfs_handle = devfs_register(NULL, AR8316_DEVNAME,
			DEVFS_FL_DEFAULT, ar8316_major, 0,
			S_IFCHR | S_IRUGO | S_IWUGO, &ar8316_fops, NULL);
#else
	int r = 0;
	r = register_chrdev(ar8316_major, AR8316_DEVNAME,
			&ar8316_fops);
	if (r < 0) {
		printk(KERN_ERR NAME ": unable to register character device\n");
		return r;
	}
	if (ar8316_major == 0) {
		ar8316_major = r;
		printk(KERN_DEBUG NAME ": got dynamic major %d\n", r);
	}
#endif

	athrs16_reg_init();
	phy_mode_setup();

	printk("Atheros AR8316 driver initialized\n");
	return 0;
}

void __exit ar8316_exit(void)
{
#ifdef  CONFIG_DEVFS_FS
	devfs_unregister_chrdev(ar8316_major, AR8316_DEVNAME);
	devfs_unregister(devfs_handle);
#else
	unregister_chrdev(ar8316_major, AR8316_DEVNAME);
#endif

	printk("Atheros AR8316 driver exited\n");
}

module_init(ar8316_init);
module_exit(ar8316_exit);

