/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright © 2003 Atheros Communications, Inc.,  All Rights Reserved.
 */

/*
 * rtPhy.h - definitions for the ethernet PHY.
 * This code supports a simple 1-port ethernet phy, Realtek RTL8201BL,
 * and compatible PHYs, such as the Kendin KS8721B.
 * All definitions in this file are operating system independent!
 */

#ifndef RTPHY_H
#define RTPHY_H
#include <linux/delay.h>

/* MII Registers */

#define	GEN_ctl		00
#define	GEN_sts		01
#define	GEN_id_hi	02
#define	GEN_id_lo	03
#define	AN_adv		04
#define	AN_lpa		05
#define	AN_exp		06

/* GEN_ctl */ 
#define	PHY_SW_RST	0x8000
#define	LOOPBACK	0x4000
#define	SPEED		0x2000	/* 100 Mbit/s */
#define	AUTONEGENA	0x1000
#define	DUPLEX		0x0100	/* Duplex mode */

		
/* GEN_sts */
#define	AUTOCMPLT	0x0020	/* Autonegotiation completed */
#define	LINK		0x0004	/* Link status */

/* GEN_ids */
#define RT_PHY_ID1_EXPECTATION  0x22

/* AN_lpa */
#define	LPA_TXFD	0x0100	/* Link partner supports 100 TX Full Duplex */
#define	LPA_TX		0x0080	/* Link partner supports 100 TX Half Duplex */
#define	LPA_10FD	0x0040	/* Link partner supports 10 BT Full Duplex */
#define	LPA_10		0x0020	/* Link partner supports 10 BT Half Duplex */

#define UINT8  u8
#define UINT16  u16
#define UINT32  u32
#define TRUE    1
#define FALSE   0
#ifndef BOOL
#define BOOL    int
#endif

#define phyRegRead  ag7100_mii_read
#define phyRegWrite  ag7100_mii_write

#define sysMsDelay  mdelay

BOOL rt_phySetup(int ethUnit, UINT32 phyBase);
int rt_phyIsFullDuplex(int ethUnit);
BOOL rt_phySpeed(int phyUnit);
int rt_phyIsUp(int ethUnit);

#endif /* RTPHY_H */
