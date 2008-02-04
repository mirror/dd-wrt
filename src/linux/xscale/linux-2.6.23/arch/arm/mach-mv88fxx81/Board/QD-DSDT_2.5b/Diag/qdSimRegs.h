#include <Copyright.h>

/********************************************************************************
* gtSimRegs.h
*
* DESCRIPTION:
*       This file includes the declaration of the struct to hold the addresses
*       of switch (global & per-port).
*
* DEPENDENCIES:
*       QuarterDeck register MAP.
*
* FILE REVISION NUMBER:
*
*******************************************************************************/

#ifndef __qdSimRegsh
#define __qdSimRegsh

/* QuarterDeck Per Port Registers */
#define QD_REG_PORT_STATUS		0x0
#define QD_REG_SWITCH_ID		0x3
#define QD_REG_PORT_CONTROL		0x4
#define QD_REG_PORT_VLAN_MAP	0x6
#define QD_REG_PVID				0x7
#define QD_REG_RATE_CTRL		0xA
#define QD_REG_PAV				0xB
#define QD_REG_RXCOUNTER		0x10
#define QD_REG_TXCOUNTER		0x11
#define QD_REG_Q_COUNTER		0x1B

/* QuarterDeck Global Registers */
#define QD_REG_GLOBAL_STATUS	0x0
#define QD_REG_MACADDR_01		0x1
#define QD_REG_MACADDR_23		0x2
#define QD_REG_MACADDR_45		0x3
#define QD_REG_GLOBAL_CONTROL	0x4

/* the following VTU entries are added for Fullsail and Clippership */
#define QD_REG_VTU_OPERATION		0x5
#define QD_REG_VTU_VID_REG		0x6
#define QD_REG_VTU_DATA1_REG		0x7
#define QD_REG_VTU_DATA2_REG		0x8
#define QD_REG_VTU_DATA3_REG		0x9
#define QD_REG_STATS_OPERATION		0x1D
#define QD_REG_STATS_COUNTER3_2		0x1E
#define QD_REG_STATS_COUNTER1_0		0x1F
 
#define QD_REG_ATU_CONTROL		0xA
#define QD_REG_ATU_OPERATION	0xB
#define QD_REG_ATU_DATA_REG		0xC
#define QD_REG_ATU_MAC_BASE		0xD
#define QD_REG_ATU_MAC_01		0xD
#define QD_REG_ATU_MAC_23		0xE
#define QD_REG_ATU_MAC_45		0xF
#define QD_REG_IP_PRI_BASE		0x10
#define QD_REG_IP_PRI_REG0		0x10
#define QD_REG_IP_PRI_REG1		0x11
#define QD_REG_IP_PRI_REG2		0x12
#define QD_REG_IP_PRI_REG3		0x13
#define QD_REG_IP_PRI_REG4		0x14
#define QD_REG_IP_PRI_REG5		0x15
#define QD_REG_IP_PRI_REG6		0x16
#define QD_REG_IP_PRI_REG7		0x17
#define QD_REG_IEEE_PRI			0x18

/* Definition for QD_REG_PORT_STATUS */
#define QD_PORT_STATUS_DUPLEX	0x200

/* Definitions for MIB Counter */
#define GT_STATS_NO_OP			0x0
#define GT_STATS_FLUSH_ALL		0x1
#define GT_STATS_FLUSH_PORT		0x2
#define GT_STATS_READ_COUNTER		0x4
#define GT_STATS_CAPTURE_PORT		0x5

#define QD_PHY_CONTROL_REG				0
#define QD_PHY_AUTONEGO_AD_REG			4
#define QD_PHY_NEXTPAGE_TX_REG			7
#define QD_PHY_SPEC_CONTROL_REG			16
#define QD_PHY_INT_ENABLE_REG			18
#define QD_PHY_INT_STATUS_REG			19
#define QD_PHY_INT_PORT_SUMMARY_REG		20

/* Bit Definition for QD_PHY_CONTROL_REG */
#define QD_PHY_RESET			0x8000
#define QD_PHY_LOOPBACK			0x4000
#define QD_PHY_SPEED			0x2000
#define QD_PHY_AUTONEGO			0x1000
#define QD_PHY_POWER			0x800
#define QD_PHY_ISOLATE			0x400
#define QD_PHY_RESTART_AUTONEGO		0x200
#define QD_PHY_DUPLEX			0x100

#define QD_PHY_POWER_BIT				11
#define QD_PHY_RESTART_AUTONEGO_BIT		9

/* Bit Definition for QD_PHY_AUTONEGO_AD_REG */
#define QD_PHY_NEXTPAGE			0x8000
#define QD_PHY_REMOTEFAULT		0x4000
#define QD_PHY_PAUSE			0x400
#define QD_PHY_100_FULL			0x100
#define QD_PHY_100_HALF			0x80
#define QD_PHY_10_FULL			0x40
#define QD_PHY_10_HALF			0x20

#define QD_PHY_MODE_AUTO_AUTO	(QD_PHY_100_FULL | QD_PHY_100_HALF | QD_PHY_10_FULL | QD_PHY_10_HALF)
#define QD_PHY_MODE_100_AUTO	(QD_PHY_100_FULL | QD_PHY_100_HALF)
#define QD_PHY_MODE_10_AUTO		(QD_PHY_10_FULL | QD_PHY_10_HALF)
#define QD_PHY_MODE_AUTO_FULL	(QD_PHY_100_FULL | QD_PHY_10_FULL)
#define QD_PHY_MODE_AUTO_HALF	(QD_PHY_100_HALF | QD_PHY_10_HALF)

#define QD_PHY_MODE_100_FULL	QD_PHY_100_FULL
#define QD_PHY_MODE_100_HALF	QD_PHY_100_HALF
#define QD_PHY_MODE_10_FULL		QD_PHY_10_FULL	
#define QD_PHY_MODE_10_HALF		QD_PHY_10_HALF	

/* Bit definition for QD_PHY_INT_ENABLE_REG */
#define QD_PHY_INT_SPEED_CHANGED		0x4000
#define QD_PHY_INT_DUPLEX_CHANGED		0x2000
#define QD_PHY_INT_PAGE_RECEIVED		0x1000
#define QD_PHY_INT_AUTO_NEG_COMPLETED		0x800
#define QD_PHY_INT_LINK_STATUS_CHANGED		0x400
#define QD_PHY_INT_SYMBOL_ERROR			0x200
#define QD_PHY_INT_FALSE_CARRIER		0x100
#define QD_PHY_INT_FIFO_FLOW			0x80
#define QD_PHY_INT_CROSSOVER_CHANGED		0x40
#define QD_PHY_INT_POLARITY_CHANGED		0x2
#define QD_PHY_INT_JABBER			0x1

#endif /* __qdSimRegsh */
