/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright © 2007 Atheros Communications, Inc.,  All Rights Reserved.
 */

/*
 * Manage the atheros Remote PHY.
 *
 * All definitions in this file are operating system independent!
 */

#include <linux/config.h>
#include <linux/types.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/delay.h>
#include "ag7240_phy.h"
#include "ag7240.h"




#define MODULE_NAME "ATHRRPHY"

/*
 * Track per-PHY port information.
 */
typedef struct {
  int              is_enet_port;
  int              mac_unit;
  unsigned int     phy_addr;
}athr_phy_t;

athr_phy_t phy_info[] = {
    {is_enet_port: 1,
     mac_unit    : 0,
     phy_addr    : 0x00}
};

static athr_phy_t *
athr_vir_phy_find(int unit)
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
athr_vir_phy_setup(int unit)
{
    athr_phy_t *phy = athr_vir_phy_find(unit);
    uint16_t  phyHwStatus;
    uint16_t  timeout;

    if (!phy) {
        printk(MODULE_NAME": \nNo phy found for unit %d\n", unit);
        return;
    }
    printk(MODULE_NAME": unit %d phy addr %x ", unit, phy->phy_addr);
}

int
athr_vir_phy_is_up(int unit)
{
    int status;
    athr_phy_t *phy = athr_vir_phy_find(unit);

    if (!phy) 
        return 0;

    status = ATHR_STATUS_LINK_PASS;
    if (status & ATHR_STATUS_LINK_PASS)
        return 1;

    return 0;
}

int
athr_vir_phy_is_fdx(int unit)
{
    int status;
    athr_phy_t *phy = athr_vir_phy_find(unit);
    int ii = 200;

    if (!phy) 
        return 0;
    status = ATHER_STATUS_FULL_DUPLEX;
    status = !(!(status & ATHER_STATUS_FULL_DUPLEX));
    return (status);
}

int
athr_vir_phy_speed(int unit)
{
    int status;
    athr_phy_t *phy = athr_vir_phy_find(unit);
    int ii = 200;

    if (!phy) 
        return 0;
    status = AG7240_PHY_SPEED_1000T;
    switch(status) {
    case 0:
        return AG7240_PHY_SPEED_10T;
    case 1:
        return AG7240_PHY_SPEED_100TX;
    case 2:
        return AG7240_PHY_SPEED_1000T;
    default:
        printk(MODULE_NAME": Unkown speed read!\n");
    }
    return -1;
}

