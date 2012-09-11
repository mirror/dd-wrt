/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright © 2003 Atheros Communications, Inc.,  All Rights Reserved.
 */

/*
 * Manage the atheros ethernet PHY.
 *
 * All definitions in this file are operating system independent!
 */

#ifdef __BDI
#include "bdi.h"
#else
#ifdef __ECOS
#if defined(CYGNUM_USE_ENET_VERBOSE)
#   undef  VERBOSE
#   define VERBOSE CYGNUM_USE_ENET_VERBOSE
#else
#   define VERBOSE 0
#endif 
#define printk             DEBUG_PRINTF
#define udelay             A_UDELAY
#define mdelay(_x)         udelay((_x)*1000)
#else
#include <linux/kernel.h>
#include <asm/delay.h>
//#ifdef CONFIG_CUS109_F1E_PHY
#define mdelay(_x)         udelay((_x)*1000)
//#endif
#endif
#endif

#ifndef VERBOSE
#define  VERBOSE           0
#endif

#if 1
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/delay.h>
#endif

#include "ag7100.h"
#include "ag7100_phy.h"
#include "ar7100.h"

#define MODULE_NAME "ATHRF1"

typedef struct {
  int              is_enet_port;
  int              mac_unit;
  unsigned int     phy_addr;
}athr_phy_t;

athr_phy_t phy_info[] = {
    {is_enet_port: 1,
     mac_unit    : 0,
//#ifdef CONFIG_CUS109_F1E_PHY
     phy_addr    : 0x00},

    {is_enet_port: 1,
     mac_unit    : 1,
     phy_addr    : 0x01}
//#else		   
//     phy_addr    : 0x00}
//#endif
};

static athr_phy_t *
athr_phy_find(int unit)
{
    int i;
    athr_phy_t *phy;

    for(i = 0; i < sizeof(phy_info)/sizeof(athr_phy_t); i++) {
        phy = &phy_info[i];
        
        if (phy->is_enet_port && (phy->mac_unit == unit)) 
            return phy;
    }
    
    return NULL;
}

int
athr_phy_setup(int unit)
{
    athr_phy_t *phy = athr_phy_find(unit);
    uint16_t  phyHwStatus;
    uint16_t  timeout;

    if (!phy) {
        printk(MODULE_NAME": \nNo phy found for unit %d\n", unit);
        return;
    }


    /*
     * After the phy is reset, it takes a little while before
     * it can respond properly.
     */

    phy_reg_write(unit, phy->phy_addr, ATHR_AUTONEG_ADVERT,
                  ATHR_ADVERTISE_ALL);

    phy_reg_write(unit, phy->phy_addr, ATHR_1000BASET_CONTROL,
                  ATHR_ADVERTISE_1000FULL);

//#ifdef CONFIG_CUS109_F1E_PHY
    /* delay rx_clk */
    phy_reg_write(unit, phy->phy_addr, 0x1D, 0x0);
    phy_reg_write(unit, phy->phy_addr, 0x1E, 0x34E);
//#endif

    /* delay tx_clk */
    phy_reg_write(unit, phy->phy_addr, 0x1D, 0x5);
//#ifdef CONFIG_CUS109_F1E_PHY
    phy_reg_write(unit, phy->phy_addr, 0x1E, 0x3C47);
//#else
//    phy_reg_write(unit, phy->phy_addr, 0x1E, 0x100);
//#endif

    /* Reset PHYs*/
    phy_reg_write(unit, phy->phy_addr, ATHR_PHY_CONTROL,
                  ATHR_CTRL_AUTONEGOTIATION_ENABLE 
                  | ATHR_CTRL_SOFTWARE_RESET);

    mdelay(500);

    /*
     * Wait up to 3 seconds for ALL associated PHYs to finish
     * autonegotiation.  The only way we get out of here sooner is
     * if ALL PHYs are connected AND finish autonegotiation.
     */
    timeout=20;
    for (;;) {
        phyHwStatus = phy_reg_read(unit, phy->phy_addr, ATHR_PHY_CONTROL);

        if (ATHR_RESET_DONE(phyHwStatus)) {
	    printk(MODULE_NAME": Port %d, Neg Success\n", unit);
            break;
        }
        if (timeout == 0) {
	    printk(MODULE_NAME": Port %d, Negogiation timeout\n", unit);
            break;
        }
        if (--timeout == 0) {
	    printk(MODULE_NAME": Port %d, Negogiation timeout\n", unit);
            break;
        }

        mdelay(150);
    }

    printk(MODULE_NAME": unit %d phy addr %x ", unit, phy->phy_addr);
    printk(MODULE_NAME": reg0 %x\n", ag7100_mii_read(0, phy->phy_addr, 0));
}

int
athr_phy_is_up(int unit)
{
    int status;
    athr_phy_t *phy = athr_phy_find(unit);

    if (!phy) 
        return 0;

    status = ag7100_mii_read(0, phy->phy_addr, ATHR_PHY_SPEC_STATUS);

    if (status & ATHR_STATUS_LINK_PASS)
        return 1;

    return 0;
}

int
athr_phy_is_fdx(int unit)
{
    int status;
    athr_phy_t *phy = athr_phy_find(unit);
    int ii = 200;

    if (!phy) 
        return 0;

    do {
    status = ag7100_mii_read(0, phy->phy_addr, ATHR_PHY_SPEC_STATUS);
	    mdelay(10);
    } while((!(status & ATHR_STATUS_RESOVLED)) && --ii);
    status = !(!(status & ATHER_STATUS_FULL_DEPLEX));

    return (status);
}

int
athr_phy_speed(int unit)
{
    int status;
    athr_phy_t *phy = athr_phy_find(unit);
    int ii = 200;

    if (!phy) 
        return 0;

    do {
        status = ag7100_mii_read(0, phy->phy_addr, ATHR_PHY_SPEC_STATUS);
	    mdelay(10);
    }while((!(status & ATHR_STATUS_RESOVLED)) && --ii);

    status = ((status & ATHER_STATUS_LINK_MASK) >> ATHER_STATUS_LINK_SHIFT);

    switch(status) {
    case 0:
        return AG7100_PHY_SPEED_10T;
    case 1:
        return AG7100_PHY_SPEED_100TX;
    case 2:
        return AG7100_PHY_SPEED_1000T;
    default:
        printk(MODULE_NAME": Unkown speed read!\n");
    }
    return -1;
}

