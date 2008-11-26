#include <Copyright.h>

/********************************************************************************
 * * gtDrvSwRegs.h
 * *
 * * DESCRIPTION:
 * *       definitions of the register map of QuarterDeck Device
 * *
 * * DEPENDENCIES:
 * *
 * * FILE REVISION NUMBER:
 * *
 * *******************************************************************************/
#ifndef __gtDrvSwRegsh
#define __gtDrvSwRegsh

/* QuarterDeck Per Port Registers */
#define QD_REG_PORT_STATUS		0x0
#define QD_REG_PCS_CONTROL		0x1		/* for Sapphire family */
#define QD_REG_LIMIT_PAUSE_CONTROL		0x2		/* Jamming control register */
#define QD_REG_SWITCH_ID		0x3
#define QD_REG_PORT_CONTROL		0x4
#define QD_REG_PORT_CONTROL1		0x5
#define QD_REG_PORT_VLAN_MAP		0x6
#define QD_REG_PVID			0x7
#define QD_REG_PORT_CONTROL2		0x8	/* for Sapphire family */
#define QD_REG_INGRESS_RATE_CTRL	0x9	/* for Sapphire family */
#define QD_REG_EGRESS_RATE_CTRL		0xA	/* for Sapphire family */
#define QD_REG_RATE_CTRL0		0x9
#define QD_REG_RATE_CTRL		0xA
#define QD_REG_PAV			0xB
#define QD_REG_PORT_ATU_CONTROL		0xC
#define QD_REG_PRI_OVERRIDE		0xD
#define QD_REG_POLICY_CONTROL	0xE
#define QD_REG_PORT_ETH_TYPE	0xF
#define QD_REG_RX_COUNTER		0x10
#define QD_REG_TX_COUNTER		0x11
#define QD_REG_DROPPED_COUNTER	0x12

#define QD_REG_INDISCARD_LO_COUNTER		0x10
#define QD_REG_INDISCARD_HI_COUNTER		0x11
#define QD_REG_INFILTERED_COUNTER		0x12
#define QD_REG_OUTFILTERED_COUNTER		0x13

#define QD_REG_Q_COUNTER		0x1B
#define QD_REG_RATE_CONTROL		0x0A
#define QD_REG_PORT_ASSOCIATION		0x0B
#define QD_REG_IEEE_PRI_REMAP_3_0	0x18	/* for Sapphire family */
#define QD_REG_IEEE_PRI_REMAP_7_4	0x19	/* for Sapphire family */

#define QD_REG_PROVIDER_TAG		0x1A		/* for Schooner family */

/* QuarterDeck Global Registers */
#define QD_REG_GLOBAL_STATUS		0x0
#define QD_REG_MACADDR_01		0x1
#define QD_REG_MACADDR_23		0x2
#define QD_REG_MACADDR_45		0x3
#define QD_REG_GLOBAL_CONTROL		0x4
#define QD_REG_GLOBAL_CONTROL2		0x1C	/* for Sapphire, Schooner family */
#define QD_REG_CORETAG_TYPE		0x19		/* for Ruby family */
#define QD_REG_MONITOR_CONTROL	0x1A		/* for Ruby family */
#define QD_REG_MANGEMENT_CONTROL	0x1A	/* for Schooner family */
#define QD_REG_TOTAL_FREE_COUNTER	0x1B	/* for Schooner family */

/* QuarterDeck Global 2 Registers */
#define QD_REG_PHYINT_SOURCE	0x0
#define QD_REG_DEVINT_SOURCE	0x0
#define QD_REG_DEVINT_MASK		0x1
#define QD_REG_MGMT_ENABLE_2X	0x2
#define QD_REG_MGMT_ENABLE		0x3
#define QD_REG_FLOWCTRL_DELAY	0x4
#define QD_REG_MANAGEMENT		0x5
#define QD_REG_ROUTING_TBL		0x6
#define QD_REG_TRUNK_MASK_TBL	0x7
#define QD_REG_TRUNK_ROUTING	0x8
#define QD_REG_INGRESS_RATE_COMMAND	0x9
#define QD_REG_INGRESS_RATE_DATA	0xA
#define QD_REG_PVT_ADDR			0xB
#define QD_REG_PVT_DATA			0xC
#define QD_REG_SWITCH_MAC		0xD
#define QD_REG_ATU_STATS		0xE
#define QD_REG_PRIORITY_OVERRIDE	0xF
#define QD_REG_PTP_COMMAND		0x16
#define QD_REG_PTP_DATA			0x17
#define QD_REG_SMI_PHY_CMD		0x18
#define QD_REG_SMI_PHY_DATA		0x19
#define QD_REG_WD_CONTROL		0x1B
#define QD_REG_QOS_WEIGHT		0x1C
#define QD_REG_SDET_POLARITY	0x1D


/* Definition for STU,VTU,RMON,and ATU Registers */
#define QD_REG_ATU_FID_REG		0x1
#define QD_REG_VTU_FID_REG		0x2
#define QD_REG_STU_SID_REG		0x3
#define QD_REG_VTU_OPERATION		0x5
#define QD_REG_VTU_VID_REG		0x6
#define QD_REG_VTU_DATA1_REG		0x7
#define QD_REG_VTU_DATA2_REG		0x8
#define QD_REG_VTU_DATA3_REG		0x9
#define QD_REG_STATS_OPERATION		0x1D
#define QD_REG_STATS_COUNTER3_2		0x1E
#define QD_REG_STATS_COUNTER1_0		0x1F

#define QD_REG_ATU_CONTROL		0xA
#define QD_REG_ATU_OPERATION		0xB
#define QD_REG_ATU_DATA_REG		0xC
#define QD_REG_ATU_MAC_BASE		0xD
#define QD_REG_IP_PRI_BASE		0x10
#define QD_REG_IEEE_PRI			0x18


/* Definitions for MIB Counter */
#define GT_STATS_NO_OP			0x0
#define GT_STATS_FLUSH_ALL		0x1
#define GT_STATS_FLUSH_PORT		0x2
#define GT_STATS_READ_COUNTER		0x4
#define GT_STATS_CAPTURE_PORT		0x5

#define QD_PHY_CONTROL_REG			0
#define QD_PHY_AUTONEGO_AD_REG			4
#define QD_PHY_NEXTPAGE_TX_REG			7
#define QD_PHY_AUTONEGO_1000AD_REG		9
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
#define QD_PHY_SPEED_MSB		0x40

#define QD_PHY_POWER_BIT			11
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

/* Gigabit Phy related definition */
#define QD_GIGPHY_1000X_FULL_CAP	0x8
#define QD_GIGPHY_1000X_HALF_CAP	0x4
#define QD_GIGPHY_1000T_FULL_CAP	0x2
#define QD_GIGPHY_1000T_HALF_CAP	0x1

#define QD_GIGPHY_1000X_CAP		(QD_GIGPHY_1000X_FULL_CAP|QD_GIGPHY_1000X_HALF_CAP)
#define QD_GIGPHY_1000T_CAP		(QD_GIGPHY_1000T_FULL_CAP|QD_GIGPHY_1000T_HALF_CAP)

#define QD_GIGPHY_1000X_FULL		0x20
#define QD_GIGPHY_1000X_HALF		0x40

#define QD_GIGPHY_1000T_FULL		0x200
#define QD_GIGPHY_1000T_HALF		0x100

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


/* Bit definition for DEVICE Interrupt */
#define QD_DEV_INT_WATCHDOG			0x8000
#define QD_DEV_INT_JAMLIMIT			0x4000
#define QD_DEV_INT_DUPLEX_MISMATCH	0x2000

/* Definition for Multi Address Mode */
#define QD_REG_SMI_COMMAND		0x0
#define QD_REG_SMI_DATA			0x1

/* Bit definition for QD_REG_SMI_COMMAND */
#define QD_SMI_BUSY				0x8000
#define QD_SMI_MODE				0x1000
#define QD_SMI_MODE_BIT			12
#define QD_SMI_OP_BIT			10
#define QD_SMI_OP_SIZE			2
#define QD_SMI_DEV_ADDR_BIT		5
#define QD_SMI_DEV_ADDR_SIZE	5
#define QD_SMI_REG_ADDR_BIT		0
#define QD_SMI_REG_ADDR_SIZE	5

#define QD_SMI_CLAUSE45			0
#define QD_SMI_CLAUSE22			1

#define QD_SMI_WRITE			0x01
#define QD_SMI_READ				0x02

#endif /* __gtDrvSwRegsh */
