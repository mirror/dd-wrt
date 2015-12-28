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
#include "rtl8309g_phy.h"

static int rtl8309g_init_flag = 0;

#define RTK_8309G_PHY0_ADDR   0x0
#define RTK_8309G_PHY1_ADDR   0x1
#define RTK_8309G_PHY2_ADDR   0x2
#define RTK_8309G_PHY3_ADDR   0x3
#define RTK_8309G_PHY4_ADDR   0x4
#define RTK_8309G_PHY5_ADDR   0x5
#define RTK_8309G_PHY6_ADDR   0x6
#define RTK_8309G_PHY7_ADDR   0x7
#define RTK_8309G_UNIT_LAN	    1
#define RTK_LAN_PORT_VLAN       1
#define RTK_8309G_PHY_MAX		8

#define TRUE    1
#define FALSE   0

/* Convenience macros to access myPhyInfo */
#define RTK_8309G_IS_ENET_PORT(phyUnit) (RTKPhyInfo[phyUnit].isEnetPort)
#define RTK_8309G_IS_PHY_ALIVE(phyUnit) (RTKPhyInfo[phyUnit].isPhyAlive)
#define RTK_8309G_ETHUNIT(phyUnit) (RTKPhyInfo[phyUnit].ethUnit)
#define RTK_8309G_PHYBASE(phyUnit) (RTKPhyInfo[phyUnit].phyBase)
#define RTK_8309G_PHYADDR(phyUnit) (RTKPhyInfo[phyUnit].phyAddr)
#define RTK_8309G_VLAN_TABLE_SETTING(phyUnit) (RTKPhyInfo[phyUnit].VLANTableSetting)

/*
 * Track per-PHY port information.
 */
typedef struct {
	BOOL isEnetPort;	/* normal enet port */
	BOOL isPhyAlive;	/* last known state of link */
	int ethUnit;		/* MAC associated with this phy port */
	uint32_t phyBase;
	uint32_t phyAddr;	/* PHY registers associated with this phy port */
	uint32_t VLANTableSetting;	/* Value to be written to VLAN table */
} RTK8309PhyInfo_t;

/*
 * Per-PHY information, indexed by PHY unit number.
 */
static RTK8309PhyInfo_t RTKPhyInfo[] = {
	{TRUE,			/* phy port 0 -- LAN port 0 */
	 FALSE,
	 RTK_8309G_UNIT_LAN,
	 0,
	 RTK_8309G_PHY0_ADDR,
	 RTK_LAN_PORT_VLAN},
	{TRUE,			/* phy port 0 -- LAN port 1 */
	 FALSE,
	 RTK_8309G_UNIT_LAN,
	 0,
	 RTK_8309G_PHY1_ADDR,
	 RTK_LAN_PORT_VLAN},
	{TRUE,			/* phy port 0 -- LAN port 2 */
	 FALSE,
	 RTK_8309G_UNIT_LAN,
	 0,
	 RTK_8309G_PHY2_ADDR,
	 RTK_LAN_PORT_VLAN},
	{TRUE,			/* phy port 0 -- LAN port 3 */
	 FALSE,
	 RTK_8309G_UNIT_LAN,
	 0,
	 RTK_8309G_PHY3_ADDR,
	 RTK_LAN_PORT_VLAN},
	{TRUE,			/* phy port 0 -- LAN port 4 */
	 FALSE,
	 RTK_8309G_UNIT_LAN,
	 0,
	 RTK_8309G_PHY4_ADDR,
	 RTK_LAN_PORT_VLAN},
	{TRUE,			/* phy port 0 -- LAN port 5 */
	 FALSE,
	 RTK_8309G_UNIT_LAN,
	 0,
	 RTK_8309G_PHY5_ADDR,
	 RTK_LAN_PORT_VLAN},
	{TRUE,			/* phy port 0 -- LAN port 6 */
	 FALSE,
	 RTK_8309G_UNIT_LAN,
	 0,
	 RTK_8309G_PHY6_ADDR,
	 RTK_LAN_PORT_VLAN},
	{TRUE,			/* phy port 0 -- LAN port 7 */
	 FALSE,
	 RTK_8309G_UNIT_LAN,
	 0,
	 RTK_8309G_PHY7_ADDR,
	 RTK_LAN_PORT_VLAN},
};

int rtl8309g_phy_is_fdx(int ethUnit)
{
	return 1;		//Full Duplex.
}

int rtl8309g_phy_speed(int ethUnit)
{
	return AG7240_PHY_SPEED_100TX;
}

int rtl8309g_phy_is_up(int ethUnit)
{
	int phyUnit;
	uint16_t phyHwStatus;
	RTK8309PhyInfo_t *lastStatus;
	int linkCount = 0;
	uint32_t phyBase;
	uint32_t phyAddr;

	if (!rtl8309g_init_flag) {
		return 0;
	}
	if (ethUnit) {
		return 0;
	}
	for (phyUnit = 0; phyUnit < RTK_8309G_PHY_MAX; phyUnit++) {
		phyBase = RTK_8309G_PHYBASE(phyUnit);
		phyAddr = RTK_8309G_PHYADDR(phyUnit);
		lastStatus = &RTKPhyInfo[phyUnit];
		if (lastStatus->isPhyAlive) {	/* last known link status was ALIVE */
			phyHwStatus =
			    phy_reg_read(phyBase, phyAddr,
					 RTL_8309G_PHY_SPEC_STATUS);
			/* See if we've lost link */
			if ((phyHwStatus & RTL_AUTO_NEG_CHECK)
			    && (phyHwStatus & RTL_8309G_STATUS_LINK_PASS)) {
				linkCount++;
			} else {
				lastStatus->isPhyAlive = FALSE;
			}
		} else {	/* last known link status was DEAD */
			phyHwStatus =
			    phy_reg_read(phyBase, phyAddr,
					 RTL_8309G_PHY_SPEC_STATUS);
			if ((phyHwStatus & RTL_AUTO_NEG_CHECK)
			    && (phyHwStatus & RTL_8309G_STATUS_LINK_PASS)) {
				linkCount++;
				lastStatus->isPhyAlive = TRUE;
			}
		}
	}
	return (linkCount);
}

BOOL rtl8309g_phy_is_link_alive(int phyUnit)
{
	return FALSE;
}

BOOL rtl8309g_phy_setup(int ethUnit)
{
	int phyUnit;
	uint16_t phyHwStatus;
	uint16_t timeout;
	uint32_t phyBase = 0;
	BOOL foundPhy = FALSE;
	uint32_t phyAddr = 0;
	uint16_t id1,id2;

	if (!rtl8309g_init_flag) {
		return FALSE;
	}
	if (ethUnit) {
		return 0;
	}
	/* See if there's any configuration data for this enet */
	/* start auto negogiation on each phy */
	for (phyUnit = 0; phyUnit < RTK_8309G_PHY_MAX; phyUnit++) {
		foundPhy = TRUE;
		phyBase = RTK_8309G_PHYBASE(phyUnit);
		phyAddr = RTK_8309G_PHYADDR(phyUnit);
		/* Reset PHYs */
		phy_reg_write(phyBase, phyAddr, RTL_8309G_PHY_CTRL_REG,
			      1 << 15);
	}
	/*
	 * After the phy is reset, it takes a little while before
	 * it can respond properly.
	 */
	mdelay(2000);
	for (phyUnit = 0; phyUnit < RTK_8309G_PHY_MAX; phyUnit++) {
		timeout = 20;
		for (;;) {
			phyHwStatus =
			    phy_reg_read(phyBase, phyAddr,
					 RTL_8309G_PHY_SPEC_STATUS);
			if (RTL_RESET_DONE(phyHwStatus)) {
				break;
			}
			if (timeout == 0) {
				break;
			}
			if (--timeout == 0) {
				break;
			}
			mdelay(150);
		}
	}

	for (phyUnit = 0; phyUnit < RTK_8309G_PHY_MAX; phyUnit++) {
		foundPhy = TRUE;
		phyBase = RTK_8309G_PHYBASE(phyUnit);
		phyAddr = RTK_8309G_PHYADDR(phyUnit);
		id1 = 	phy_reg_read(phyBase, phyAddr ,RTL_8309G_PHY_ID1);
		id2 = 	phy_reg_read(phyBase, phyAddr ,RTL_8309G_PHY_ID2);
		printk(KERN_INFO "rtl phy%d id %X:%X\n",phyUnit,id1,id2);
	}


	return TRUE;
}

void rtl8309g_reg_init(int ethUnit)
{
	if (ethUnit) {
		return;
	}
	if (rtl8309g_init_flag) {
		return;
	}
	rtl8309g_init_flag = 1;
	return;
}
