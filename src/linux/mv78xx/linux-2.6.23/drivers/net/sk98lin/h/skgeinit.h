/******************************************************************************
 *
 * Name:	skgeinit.h
 * Project:	Gigabit Ethernet Adapters, Common Modules
 * Version:	$Revision: 2.65 $
 * Date:	$Date: 2007/07/17 08:17:49 $
 * Purpose:	Structures and prototypes for the GE Init Module
 *
 ******************************************************************************/

/******************************************************************************
 *
 *	LICENSE:
 *	(C)Copyright 1998-2002 SysKonnect.
 *	(C)Copyright 2002-2007 Marvell.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *	The information in this file is provided "AS IS" without warranty.
 *	/LICENSE
 *
 ******************************************************************************/

#ifndef __INC_SKGEINIT_H_
#define __INC_SKGEINIT_H_

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

/* defines ********************************************************************/

#define SK_TEST_VAL		0x11335577UL

/* modifying Link LED behaviour (used with SkGeLinkLED()) */
#define SK_LNK_OFF		LED_OFF
#define SK_LNK_ON		(LED_ON | LED_BLK_OFF | LED_SYNC_OFF)
#define SK_LNK_BLINK	(LED_ON | LED_BLK_ON  | LED_SYNC_ON)
#define SK_LNK_PERM		(LED_ON | LED_BLK_OFF | LED_SYNC_ON)
#define SK_LNK_TST		(LED_ON | LED_BLK_ON  | LED_SYNC_OFF)

/* parameter 'Mode' when calling SK_HWAC_LINK_LED() */
#define SK_LED_OFF		LED_OFF
#define SK_LED_ACTIVE	(LED_ON | LED_BLK_OFF | LED_SYNC_OFF)
#define SK_LED_STANDBY	(LED_ON | LED_BLK_ON  | LED_SYNC_OFF)

/* addressing LED Registers in SkGeXmitLED() */
#define XMIT_LED_INI	0
#define XMIT_LED_CNT	(RX_LED_VAL - RX_LED_INI)
#define XMIT_LED_CTRL	(RX_LED_CTRL- RX_LED_INI)
#define XMIT_LED_TST	(RX_LED_TST - RX_LED_INI)

/* parameter 'Mode' when calling SkGeXmitLED() */
#define SK_LED_DIS	0
#define SK_LED_ENA	1
#define SK_LED_TST	2

/* Counter and Timer constants, for a host clock of 62.5 MHz */
#define SK_XMIT_DUR		0x002faf08UL	/*  50 ms */
#define SK_BLK_DUR		0x01dcd650UL	/* 500 ms */

#define SK_DPOLL_DEF	0x00ee6b28UL	/* 250 ms at 62.5 MHz (Genesis) */
#define SK_DPOLL_DEF_Y2	0x0000124fUL	/*  75 us (Yukon-2) */

#define SK_DPOLL_MAX	0x00ffffffUL	/* 268 ms at 62.5 MHz */
										/* 215 ms at 78.12 MHz (Yukon) */

#define SK_FACT_62		100			/* is given in percent */
#define SK_FACT_50		 81			/* on YUKON-FE+ 50 MHz */
#define SK_FACT_53		 85			/* on GENESIS:	53.12 MHz */
#define SK_FACT_78		125			/* on YUKON:	78.12 MHz */
#define SK_FACT_100		161			/* on YUKON-FE:	100 MHz */
#define SK_FACT_125		202			/* on YUKON-EC:	125 MHz */

/* Timeout values */
#define SK_MAC_TO_53	72			/* MAC arbiter timeout */
#define SK_PKT_TO_53	0x2000		/* Packet arbiter timeout */
#define SK_PKT_TO_MAX	0xffff		/* Maximum value */
#define SK_RI_TO_53		36			/* RAM interface timeout */

#define SK_PHY_ACC_TO	600000		/* PHY access timeout */

/* RAM Buffer High Pause Threshold values */
#define SK_RB_ULPP		( 8 * 1024)	/* Upper Level in kB/8 */
#define SK_RB_LLPP_S	(10 * 1024)	/* Lower Level for small Queues */
#define SK_RB_LLPP_B	(16 * 1024)	/* Lower Level for big Queues */

/* Threshold values for Yukon-EC Ultra and Extreme*/
#define SK_ECU_ULPP		0x0080	/* Upper Pause Threshold (multiples of 8) */
#define SK_ECU_LLPP		0x0060	/* Lower Pause Threshold (multiples of 8) */
#define SK_ECU_AE_THR	0x0070	/* Almost Empty Threshold */
#define SK_ECU_TXFF_LEV	0x01a0	/* Tx BMU FIFO Level */
#define SK_ECU_JUMBO_WM	0x0400	/* Jumbo Mode Watermark */

#ifndef SK_BMU_RX_WM
#define SK_BMU_RX_WM		0x600	/* BMU Rx Watermark */
#endif

#ifndef SK_BMU_TX_WM
#define SK_BMU_TX_WM		0x600	/* BMU Tx Watermark */
#endif

/* performance sensitive drivers should set this define to 0x80 */
#ifndef SK_BMU_RX_WM_PEX
#define SK_BMU_RX_WM_PEX	0x80	/* BMU Rx Watermark for PEX */
#endif

/* XMAC II Rx High Watermark */
#define SK_XM_RX_HI_WM	0x05aa		/* 1450 */

/* XMAC II Tx Threshold */
#define SK_XM_THR_REDL	0x01fb		/* .. for redundant link usage */
#define SK_XM_THR_SL	0x01fb		/* .. for single link adapters */
#define SK_XM_THR_MULL	0x01fb		/* .. for multiple link usage */
#define SK_XM_THR_JUMBO	0x03fc		/* .. for jumbo frame usage */

/* values for PortUsage */
#define SK_RED_LINK		1		/* redundant link usage */
#define SK_MUL_LINK		2		/* multiple link usage */
#define SK_JUMBO_LINK	3		/* driver uses jumbo frames */

/* Minimum RAM Buffer Rx Queue Size */
#define SK_MIN_RXQ_SIZE	(((pAC)->GIni.GIYukon2) ? 10 : 16)		/* 10/16 kB */

/* Minimum RAM Buffer Tx Queue Size */
#define SK_MIN_TXQ_SIZE	(((pAC)->GIni.GIYukon2) ? 10 : 16)		/* 10/16 kB */

/* Queue Size units (Genesis/Yukon) */
#define QZ_UNITS		7
#define QZ_STEP			8

/* Queue Size units (Yukon-2) */
#define QZ_STEP_Y2		1

/* Percentage of queue size from whole memory */
/* 80 % for receive */
#define RAM_QUOTA_RX	80
/*  0 % for sync transfer */
#define RAM_QUOTA_SYNC	0
/* the rest (20%) is taken for async transfer */

/* Types of RAM Buffer Queues */
#define SK_RX_SRAM_Q	1	/* small receive queue */
#define SK_RX_BRAM_Q	2	/* big receive queue */
#define SK_TX_RAM_Q		3	/* small or big transmit queue */

/* parameter 'Dir' when calling SkGeStopPort() */
#define SK_STOP_TX	1	/* Stops the transmit path, resets the XMAC */
#define SK_STOP_RX	2	/* Stops the receive path */
#define SK_STOP_ALL	3	/* Stops Rx and Tx path, resets the XMAC */

/* parameter 'RstMode' when calling SkGeStopPort() */
#define SK_SOFT_RST	1	/* perform a software reset */
#define SK_HARD_RST	2	/* perform a hardware reset */

/* Init Levels */
#define SK_INIT_DATA	0	/* Init level 0: init data structures */
#define SK_INIT_IO		1	/* Init level 1: init with IOs */
#define SK_INIT_RUN		2	/* Init level 2: init for run time */

/* Link Mode Parameter */
#define SK_LMODE_HALF		1	/* Half Duplex Mode */
#define SK_LMODE_FULL		2	/* Full Duplex Mode */
#define SK_LMODE_AUTOHALF	3	/* AutoHalf Duplex Mode */
#define SK_LMODE_AUTOFULL	4	/* AutoFull Duplex Mode */
#define SK_LMODE_AUTOBOTH	5	/* AutoBoth Duplex Mode */
#define SK_LMODE_AUTOSENSE	6	/* configured mode auto sensing */
#define SK_LMODE_INDETERMINATED	7	/* indeterminated */

/* Auto-negotiation timeout in 100ms granularity */
#define SK_AND_MAX_TO		6	/* Wait 600 msec before link comes up */

/* Auto-negotiation error codes */
#define SK_AND_OK			0	/* no error */
#define SK_AND_OTHER		1	/* other error than below */
#define SK_AND_DUP_CAP		2	/* Duplex capabilities error */

/* Link Speed Capabilities */
#define SK_LSPEED_CAP_AUTO			BIT_0S	/* Automatic resolution */
#define SK_LSPEED_CAP_10MBPS		BIT_1S	/* 10 Mbps */
#define SK_LSPEED_CAP_100MBPS		BIT_2S	/* 100 Mbps */
#define SK_LSPEED_CAP_1000MBPS		BIT_3S	/* 1000 Mbps */
#define SK_LSPEED_CAP_INDETERMINATED BIT_4S /* indeterminated */

/* Link Speed Parameter */
#define SK_LSPEED_AUTO				1	/* Automatic resolution */
#define SK_LSPEED_10MBPS			2	/* 10 Mbps */
#define SK_LSPEED_100MBPS			3	/* 100 Mbps */
#define SK_LSPEED_1000MBPS			4	/* 1000 Mbps */
#define SK_LSPEED_INDETERMINATED	5	/* indeterminated */

/* Link Speed Current State */
#define SK_LSPEED_STAT_UNKNOWN		1
#define SK_LSPEED_STAT_10MBPS		2
#define SK_LSPEED_STAT_100MBPS 		3
#define SK_LSPEED_STAT_1000MBPS		4
#define SK_LSPEED_STAT_INDETERMINATED 5

/* Link Capability Parameter */
#define SK_LMODE_CAP_HALF		BIT_0S	/* Half Duplex Mode */
#define SK_LMODE_CAP_FULL		BIT_1S	/* Full Duplex Mode */
#define SK_LMODE_CAP_AUTOHALF	BIT_2S	/* AutoHalf Duplex Mode */
#define SK_LMODE_CAP_AUTOFULL	BIT_3S	/* AutoFull Duplex Mode */
#define SK_LMODE_CAP_INDETERMINATED BIT_4S /* indeterminated */

/* Link Mode Current State */
#define SK_LMODE_STAT_UNKNOWN	1	/* Unknown Duplex Mode */
#define SK_LMODE_STAT_HALF		2	/* Half Duplex Mode */
#define SK_LMODE_STAT_FULL		3	/* Full Duplex Mode */
#define SK_LMODE_STAT_AUTOHALF	4	/* Half Duplex Mode obtained by Auto-Neg */
#define SK_LMODE_STAT_AUTOFULL	5	/* Full Duplex Mode obtained by Auto-Neg */
#define SK_LMODE_STAT_INDETERMINATED 6	/* indeterminated */

/* Flow Control Mode Parameter (and capabilities) */
#define SK_FLOW_MODE_NONE		1	/* No Flow Control */
#define SK_FLOW_MODE_LOC_SEND	2	/* Local station sends PAUSE */
#define SK_FLOW_MODE_SYMMETRIC	3	/* Both stations may send PAUSE */
#define SK_FLOW_MODE_SYM_OR_REM	4	/* Both stations may send PAUSE or
					 * just the remote station may send PAUSE
					 */
#define SK_FLOW_MODE_INDETERMINATED 5	/* indeterminated */

/* Flow Control Status Parameter */
#define SK_FLOW_STAT_NONE		1	/* No Flow Control */
#define SK_FLOW_STAT_REM_SEND	2	/* Remote Station sends PAUSE */
#define SK_FLOW_STAT_LOC_SEND	3	/* Local station sends PAUSE */
#define SK_FLOW_STAT_SYMMETRIC	4	/* Both station may send PAUSE */
#define SK_FLOW_STAT_INDETERMINATED 5	/* indeterminated */

/* Master/Slave Mode Capabilities */
#define SK_MS_CAP_AUTO		BIT_0S	/* Automatic resolution */
#define SK_MS_CAP_MASTER	BIT_1S	/* This station is master */
#define SK_MS_CAP_SLAVE		BIT_2S	/* This station is slave */
#define SK_MS_CAP_INDETERMINATED BIT_3S	/* indeterminated */

/* Set Master/Slave Mode Parameter (and capabilities) */
#define SK_MS_MODE_AUTO		1	/* Automatic resolution */
#define SK_MS_MODE_MASTER	2	/* This station is master */
#define SK_MS_MODE_SLAVE	3	/* This station is slave */
#define SK_MS_MODE_INDETERMINATED 4	/* indeterminated */

/* Master/Slave Status Parameter */
#define SK_MS_STAT_UNSET	1	/* The M/S status is not set */
#define SK_MS_STAT_MASTER	2	/* This station is master */
#define SK_MS_STAT_SLAVE	3	/* This station is slave */
#define SK_MS_STAT_FAULT	4	/* M/S resolution failed */
#define SK_MS_STAT_INDETERMINATED 5	/* indeterminated */

/* parameter 'Mode' when calling SkMacSetRxCmd() */
#define SK_STRIP_FCS_ON		BIT_0S	/* Enable  FCS stripping of Rx frames */
#define SK_STRIP_FCS_OFF	BIT_1S	/* Disable FCS stripping of Rx frames */
#define SK_STRIP_PAD_ON		BIT_2S	/* Enable  pad byte stripping of Rx fr */
#define SK_STRIP_PAD_OFF	BIT_3S	/* Disable pad byte stripping of Rx fr */
#define SK_LENERR_OK_ON		BIT_4S	/* Don't chk fr for in range len error */
#define SK_LENERR_OK_OFF	BIT_5S	/* Check frames for in range len error */
#define SK_BIG_PK_OK_ON		BIT_6S	/* Don't set Rx Error bit for big frames */
#define SK_BIG_PK_OK_OFF	BIT_7S	/* Set Rx Error bit for big frames */
#define SK_SELF_RX_ON		BIT_8S	/* Enable  Rx of own packets */
#define SK_SELF_RX_OFF		BIT_9S	/* Disable Rx of own packets */

/* parameter 'Para' when calling SkMacSetRxTxEn() */
#define SK_MAC_LOOPB_ON		BIT_0S	/* Enable  MAC Loopback Mode */
#define SK_MAC_LOOPB_OFF	BIT_1S	/* Disable MAC Loopback Mode */
#define SK_PHY_LOOPB_ON		BIT_2S	/* Enable  PHY Loopback Mode */
#define SK_PHY_LOOPB_OFF	BIT_3S	/* Disable PHY Loopback Mode */
#define SK_PHY_FULLD_ON		BIT_4S	/* Enable  GMII Full Duplex */
#define SK_PHY_FULLD_OFF	BIT_5S	/* Disable GMII Full Duplex */

/* States of PState */
#define SK_PRT_RESET	0	/* the port is reset */
#define SK_PRT_STOP		1	/* the port is stopped (similar to SW reset) */
#define SK_PRT_INIT		2	/* the port is initialized */
#define SK_PRT_RUN		3	/* the port has an active link */

/* PHY power down modes */
#define PHY_PM_OPERATIONAL_MODE		0	/* PHY operational mode */
#define PHY_PM_DEEP_SLEEP			1	/* Coma mode --> minimal power */
#define PHY_PM_IEEE_POWER_DOWN		2	/* IEEE 22.2.4.1.5 compl. power down */
#define PHY_PM_ENERGY_DETECT		3	/* Energy detect */
#define PHY_PM_ENERGY_DETECT_PLUS	4	/* Energy detect plus */

/* PCI Bus Types */
#define SK_PCI_BUS		BIT_0S		/* normal PCI bus */
#define SK_PCIX_BUS		BIT_1S		/* PCI-X bus */
#define SK_PEX_BUS		BIT_2S		/* PCI-Express bus */

/* Default receive frame limit for Workaround of XMAC Errata */
#define SK_DEF_RX_WA_LIM	SK_CONSTU64(100)

/* values for GILedBlinkCtrl (LED Blink Control) */
#define SK_ACT_LED_BLINK	BIT_0S	/* Activity LED blinking */
#define SK_DUP_LED_NORMAL	BIT_1S	/* Duplex LED normal */
#define SK_LED_LINK100_ON	BIT_2S	/* Link 100M LED on */
#define SK_DUAL_LED_ACT_LNK	BIT_3S	/* Dual LED ACT/LNK configuration */
#define SK_LED_LINK_MUX_P60	BIT_4S	/* Link LED muxed to pin 60 */
#define SK_LED_COMB_ACT_LNK	BIT_5S	/* Combined ACT/LNK LED mode */
#define SK_ACT_LED_NOTR_OFF	BIT_6S	/* Activity LED off (no traffic) */

/* Link Partner Status */
#define SK_LIPA_UNKNOWN	0	/* Link partner is in unknown state */
#define SK_LIPA_MANUAL	1	/* Link partner is in detected manual state */
#define SK_LIPA_AUTO	2	/* Link partner is in auto-negotiation state */

/* Maximum Restarts before restart is ignored (3Com WA) */
#define SK_MAX_LRESTART	3	/* Max. 3 times the link is restarted */

/* Max. Auto-neg. timeouts before link detection in sense mode is reset */
#define SK_MAX_ANEG_TO	10	/* Max. 10 times the sense mode is reset */


/******************************************************************************
 *
 * HW_FEATURE() macro
 */

/* DWORD 0: Features */
#define HWF_ENA_POW_SAV_W_WOL	0x08000000UL	/* Power saving with WOL ena. */
#define HWF_FORCE_AUTO_NEG		0x04000000UL	/* Force Auto-Negotiation */
#define HWF_CLK_GATING_ENABLE	0x02000000UL	/* Enable Clock Gating */
#define HWF_RED_CORE_CLK_SUP	0x01000000UL	/* Reduced Core Clock supp. */
#define HWF_RESTORE_LOST_BARS	0x00800000UL	/* Save and restore PCI BARs */
#define HWF_ASPM_SWITCHING		0x00400000UL	/* Activate ASPM feature */
#define HWF_TX_IP_ID_INCR_ON	0x00200000UL	/* Enable Tx IP ID Increment */
#define HWF_ADV_CSUM_SUPPORT	0x00100000UL	/* Sel Csum of IP and TCP/UDP */
#define HWF_PSM_SUPPORTED		0x00080000UL	/* Power State Manager support*/


/*-RMV- DWORD 1: Deviations (all in use) */
#define HWF_WA_DEV_4222			0x18000000UL	/*-RMV- 4.222 (Done Idx rep.) */
#define HWF_WA_DEV_56			0x14000000UL	/*-RMV- 5.6 (Rx Chksum 0xffff)*/
#define HWF_WA_DEV_54			0x12000000UL	/*-RMV- 5.4 (Missing Status LE)*/
#define HWF_WA_DEV_53			0x11000000UL	/*-RMV- 5.3 (Tx Done LSOv2 rep)*/
#define HWF_WA_DEV_LIM_IPV6_RSS	0x10800000UL	/*-RMV- IPV6 RSS limitted */
#define HWF_WA_DEV_4217			0x10400000UL	/*-RMV- 4.217 (PCI-E blockage) */
#define HWF_WA_DEV_4200			0x10200000UL	/*-RMV- 4.200 (D3 Blue Screen)*/
#define HWF_WA_DEV_4185CS		0x10100000UL	/*-RMV- 4.185 (ECU 100 CS cal)*/
#define HWF_WA_DEV_4185			0x10080000UL	/*-RMV- 4.185 (ECU Tx h check)*/
#define HWF_WA_DEV_4167			0x10040000UL	/*-RMV- 4.167 (Rx OvSize Hang)*/
#define HWF_WA_DEV_4152			0x10020000UL	/*-RMV- 4.152 (RSS issue) */
#define HWF_WA_DEV_4115			0x10010000UL	/*-RMV- 4.115 (Rx MAC FIFO) */
#define HWF_WA_DEV_4109			0x10008000UL	/*-RMV- 4.109 (BIU hang) */
#define HWF_WA_DEV_483			0x10004000UL	/*-RMV- 4.83 (Rx TCP wrong) */
#define HWF_WA_DEV_479			0x10002000UL	/*-RMV- 4.79 (Rx BMU hang II) */
#define HWF_WA_DEV_472			0x10001000UL	/*-RMV- 4.72 (GPHY2 MDC clk) */
#define HWF_WA_DEV_463			0x10000800UL	/*-RMV- 4.63 (Rx BMU hang I) */
#define HWF_WA_DEV_427			0x10000400UL	/*-RMV- 4.27 (Tx Done Rep) */
#define HWF_WA_DEV_42			0x10000200UL	/*-RMV- 4.2 (pref unit burst) */
#define HWF_WA_DEV_46			0x10000100UL	/*-RMV- 4.6 (CPU crash II) */
#define HWF_WA_DEV_43_418		0x10000080UL	/*-RMV- 4.3 & 4.18 (PCI unexp */
												/*-RMV- compl&Stat BMU deadl) */
#define HWF_WA_DEV_420			0x10000040UL	/*-RMV- 4.20 (Status BMU ov) */
#define HWF_WA_DEV_423			0x10000020UL	/*-RMV- 4.23 (TCP Segm Hang) */
#define HWF_WA_DEV_424			0x10000010UL	/*-RMV- 4.24 (MAC reg overwr) */
#define HWF_WA_DEV_425			0x10000008UL	/*-RMV- 4.25 (Magic packet */
												/*-RMV- with odd offset) */
#define HWF_WA_DEV_428			0x10000004UL	/*-RMV- 4.28 (Poll-U &BigEndi)*/
#define HWF_WA_FIFO_FLUSH_YLA0	0x10000002UL	/*-RMV- dis Rx GMAC FIFO Flush*/
												/*-RMV- for Yu-L Rev. A0 only */
#define HWF_WA_COMA_MODE		0x10000001UL	/*-RMV- Coma Mode WA req */

/*-RMV- DWORD 1: Deviations */
#define HWF_WA_DEV_521			0x20000010UL	/*-RMV- 5.21 (wrong RFSW) */
#define HWF_WA_DEV_520			0x20000008UL	/*-RMV- 5.20 (Tx lost of data)*/
#define HWF_WA_DEV_511			0x20000004UL	/*-RMV- 5.11 (Tx Underrun) */
#define HWF_WA_DEV_510			0x20000002UL	/*-RMV- 5.10 (Tx Checksum) */
#define HWF_WA_DEV_51			0x20000001UL	/*-RMV- 5.1 (MACSec sync) */

#if 0
#define HWF_SYNC_TX_SUP			0x20800000UL	/* Synch. Tx Queue available */
#define HWF_SINGLE_PORT_DEVICE	0x20400000UL	/* Device has only one LAN IF */
#define HWF_JUMBO_FRAMES_SUP	0x20200000UL	/* Jumbo Frames supported */
#define HWF_TX_TCP_CSUM_SUP		0x20100000UL	/* TCP Tx checksum supported */
#define HWF_TX_UDP_CSUM_SUP		0x20080000UL	/* UDP Tx checksum supported */
#define HWF_RX_CSUM_SUP			0x20040000UL	/* RX checksum supported */
#define HWF_TCP_SEGM_SUP		0x20020000UL	/* TCP segmentation supported */
#define HWF_RSS_HASH_SUP		0x20010000UL	/* RSS Hash supported */
#define HWF_PORT_VLAN_SUP		0x20008000UL	/* VLAN can be config per port*/
#define HWF_ROLE_PARAM_SUP		0x20004000UL	/* Role parameter supported */
#define HWF_LOW_PMODE_SUP		0x20002000UL	/* Low Power Mode supported */
#define HWF_ENERGIE_DEMO_SUP	0x20001000UL	/* Energy Detect mode supp. */
#define HWF_SPEED1000_SUP		0x20000800UL	/* Line Speed 1000 supported */
#define HWF_SPEED100_SUP		0x20000400UL	/* Line Speed 100 supported */
#define HWF_SPEED10_SUP			0x20000200UL	/* Line Speed 10 supported */
#define HWF_AUTONEGSENSE_SUP	0x20000100UL	/* Autoneg Sense supported */
#define HWF_PHY_LOOPB_MD_SUP	0x20000080UL	/* PHY loopback mode supp. */
#define HWF_ASF_SUP				0x20000040UL	/* ASF support possible */
#define HWF_QS_STEPS_1KB		0x20000020UL	/* The Rx/Tx queues can be */
												/* configured with 1 kB res. */
#define HWF_OWN_RAM_PER_PORT	0x20000010UL	/* Each port has a separate */
												/* RAM buffer */
#define HWF_MIN_LED_IF			0x20000008UL	/* Minimal LED interface */
												/* (e.g. for Yukon-EC) */
#define HWF_LIST_ELEMENTS_USED	0x20000004UL	/* HW uses list elements */
												/* (otherwise desc. are used) */
#define HWF_GMAC_INSIDE			0x20000002UL	/* Device contains GMAC */
#define HWF_TWSI_PRESENT		0x20000001UL	/* TWSI sensor bus present */
#endif

/* DWORD 3: still unused */


/*
 * HW_FEATURE()	-	returns whether the feature is serviced or not
 */
#define HW_FEATURE(pAC, ReqFeature) \
	(((pAC)->GIni.HwF.Features[((ReqFeature) & 0x30000000UL) >> 28] &\
	 ((ReqFeature) & 0x0fffffffUL)) != 0)

#define HW_FEAT_LIST	0
#define HW_DEV_LIST		1
#define HW_DEV_LIST_2	2

#define SET_HW_FEATURE_MASK(pAC, List, OffMaskValue, OnMaskValue) {	\
	if ((List) == HW_FEAT_LIST || (List) == HW_DEV_LIST) {			\
		(pAC)->GIni.HwF.OffMask[List] = (OffMaskValue);				\
		(pAC)->GIni.HwF.OnMask[List] = (OnMaskValue);				\
	}																\
}

/* driver access macros for GIni structure ***********************************/

#define CHIP_ID_YUKON_2(pAC)		((pAC)->GIni.GIYukon2)

#define HW_SYNC_TX_SUPPORTED(pAC)						\
		((pAC)->GIni.GIChipId != CHIP_ID_YUKON_EC	&&	\
		 (pAC)->GIni.GIChipId != CHIP_ID_YUKON_FE	&&	\
		 (pAC)->GIni.GIChipId != CHIP_ID_YUKON_FE_P	&&	\
		 (pAC)->GIni.GIChipId != CHIP_ID_YUKON_EC_U	&&	\
		 (pAC)->GIni.GIChipId != CHIP_ID_YUKON_EX)

#define HW_HAS_NEWER_PHY(pAC)							\
		((pAC)->GIni.GIChipId == CHIP_ID_YUKON_XL	||	\
		 (pAC)->GIni.GIChipId == CHIP_ID_YUKON_EC_U	||	\
		 (pAC)->GIni.GIChipId == CHIP_ID_YUKON_EX)

#define HW_MS_TO_TICKS(pAC, MsTime) \
	((MsTime) * (62500L/100) * (pAC)->GIni.GIHstClkFact)

#define HW_IS_EXT_LE_FORMAT(pAC)	((pAC)->GIni.GIExtLeFormat)
#define HW_IS_NEW_ASF_FORMAT(pAC)	((pAC)->GIni.GIExtLeFormat)

#define HW_IS_RAM_IF_AVAIL(pAC)		(					\
	!((pAC)->GIni.GIChipId == CHIP_ID_YUKON_EC_U ||		\
	  (pAC)->GIni.GIChipId == CHIP_ID_YUKON_EX	 ||		\
	  (pAC)->GIni.GIChipId == CHIP_ID_YUKON_FE_P))

#define HW_NUM_OF_PATTERN(pAC)		((pAC)->GIni.GINumOfPattern)
#define HW_OTP_SUPPORTED(pAC)		((pAC)->GIni.GIChipId >= CHIP_ID_YUKON_FE_P)

/* general CSUM */
/* flag to distinguish between old style and new style csum offload */
#define HW_SUP_FULL_OFFLOAD(pAC)		((pAC)->GIni.GIExtLeFormat)
#define HW_SUP_NEW_LSOV1(pAC)			((pAC)->GIni.GIExtLeFormat)

/* Rx CSUM IPv4 */
#define HW_SUP_RX_CSUM_IPV4(pAC)		SK_TRUE
#define HW_SUP_RX_CSUM_OPT_IPV4(pAC)	((pAC)->GIni.GIExtLeFormat)
#define HW_SUP_RX_CSUM_TCPIPV4(pAC)		SK_TRUE
#define HW_SUP_RX_CSUM_OPT_TCPIPV4(pAC)	((pAC)->GIni.GIExtLeFormat)
#define HW_SUP_RX_CSUM_UDPIPV4(pAC)		SK_TRUE

/* Rx CSUM IPv6 */
#define HW_SUP_RX_CSUM_IPV6_EXT_HDR(pAC)((pAC)->GIni.GIExtLeFormat)
#define HW_SUP_RX_CSUM_TCPIPV6(pAC)		((pAC)->GIni.GIExtLeFormat)
#define HW_SUP_RX_CSUM_OPT_TCPIPV6(pAC)	((pAC)->GIni.GIExtLeFormat)
#define HW_SUP_RX_CSUM_UDPIPV6(pAC)		((pAC)->GIni.GIExtLeFormat)

/* Tx CSUM IPv4 */
#define HW_SUP_TX_CSUM_IPV4(pAC)		((pAC)->GIni.GIExtLeFormat)
#define HW_SUP_TX_CSUM_OPT_IPV4(pAC)	((pAC)->GIni.GIExtLeFormat)
#define HW_SUP_TX_CSUM_TCPIPV4(pAC)		(((pAC)->GIni.GIChipId != CHIP_ID_GENESIS) &&	\
											!HW_FEATURE(pAC, HWF_WA_DEV_4185CS))
#define HW_SUP_TX_CSUM_OPT_TCPIPV4(pAC)	((pAC)->GIni.GIExtLeFormat)
#define HW_SUP_TX_CSUM_UDPIPV4(pAC)		(((pAC)->GIni.GIChipId != CHIP_ID_GENESIS) &&	\
											!HW_FEATURE(pAC, HWF_WA_DEV_4185CS))
/* Tx CSUM IPv6 */
#define HW_SUP_TX_CSUM_IPV6_EXT_HDR(pAC)((pAC)->GIni.GIExtLeFormat)
#define HW_SUP_TX_CSUM_TCPIPV6(pAC)		((pAC)->GIni.GIExtLeFormat)
#define HW_SUP_TX_CSUM_OPT_TCPIPV6(pAC)	((pAC)->GIni.GIExtLeFormat)
#define HW_SUP_TX_CSUM_UDPIPV6(pAC)		((pAC)->GIni.GIExtLeFormat)

/* Tx CSUM special flag for FiFo size */
#define HW_SUP_TX_CSUM_FOR_JUMBO(pAC)	(		\
	(pAC)->GIni.GIJumTcpSegSup || 				\
	(pAC)->GIni.GIChipId == CHIP_ID_GENESIS ||	\
	(pAC)->GIni.GIChipId == CHIP_ID_YUKON ||	\
	(pAC)->GIni.GIChipId == CHIP_ID_YUKON_LITE)

/* LSO */
#define HW_SUP_LSOV1(pAC)				((pAC)->GIni.GIYukon2)
#define HW_SUP_OPT_IPV4_LSOV1(pAC)		((pAC)->GIni.GIYukon2)
#define HW_SUP_OPT_TCPIPV4_LSOV1(pAC)	((pAC)->GIni.GIYukon2)
#define HW_SUP_LSOV2(pAC)				((pAC)->GIni.GIExtLeFormat)
#define HW_SUP_IPV6_EXT_HDR_LSOV2(pAC)	((pAC)->GIni.GIExtLeFormat)
#define HW_SUP_OPT_TCPIPV6_LSOV2(pAC)	((pAC)->GIni.GIExtLeFormat)
#define HW_SUP_LSO_FOR_JUMBO(pAC)		((pAC)->GIni.GIJumTcpSegSup)

/* RSS */
#define HW_SUP_IPV4_RSS(pAC)			((pAC)->GIni.GIYukon2 && 				\
										  !HW_FEATURE(pAC, HWF_WA_DEV_4152))
#define HW_SUP_IPV6_RSS(pAC)			((pAC)->GIni.GIExtLeFormat)

/* MacSec */
#define HW_SUP_MACSEC(pAC)				SK_FALSE

/* VPD Support */
#define HW_SUP_VPD(pAC)			((pAC)->GIni.GIChipId != CHIP_ID_YUKON_FE_P)

/* SPI PROM Support */
#define HW_SUP_SPI_PROM(pAC)	((pAC)->GIni.GIChipId != CHIP_ID_YUKON_FE_P &&	\
								 (pAC)->GIni.GIChipId != CHIP_ID_YUKON_FE)

/* Battery Power Management */
#define HW_NO_BAT_POW_MANAGEMENT(pAC)					\
	((pAC)->GIni.GIChipId == CHIP_ID_YUKON_EC_U &&		\
	 (pAC)->GIni.GIChipRev >= CHIP_REV_YU_EC_U_A1 &&	\
	 (pAC)->GIni.GIChipCap == 2 &&						\
	 !HW_FEATURE(pAC, HWF_WA_DEV_4200))

/* maximal supported link speed; returns 100 or 1000 */
#define HW_MAX_LINE_SPEED(pAC)							\
	((pAC)->GIni.GP[MAC_1].PLinkSpeedCap & SK_LSPEED_CAP_1000MBPS ? 1000 : 100)

/* structures *****************************************************************/

/*
 * HW Feature structure
 */
typedef struct s_HwFeatures {
	SK_U32	Features[4];	/* Feature list */
	SK_U32	OffMask[4];		/* Off Mask */
	SK_U32	OnMask[4];		/* On Mask */
} SK_HW_FEATURES;

/*
 * MAC specific functions
 */
typedef struct s_GeMacFunc {
	int	(*pFnMacUpdateStats)(SK_AC *, SK_IOC, unsigned int);
	int	(*pFnMacStatistic)(SK_AC *, SK_IOC, unsigned int, SK_U16, SK_U32 SK_FAR *);
	int	(*pFnMacResetCounter)(SK_AC *, SK_IOC, unsigned int);
	int	(*pFnMacOverflow)(SK_AC *, SK_IOC, unsigned int, SK_U16, SK_U64 SK_FAR *);
	void (*pSkGeSirqIsr)(SK_AC *, SK_IOC, SK_U32);
#ifdef SK_DIAG
	int	(*pFnMacPhyRead)(SK_AC *, SK_IOC, int, int, SK_U16 SK_FAR *);
	int	(*pFnMacPhyWrite)(SK_AC *, SK_IOC, int, int, SK_U16);
#endif /* SK_DIAG */
} SK_GEMACFUNC;

/*
 * Port Structure
 */
typedef	struct s_GePort {
#ifndef SK_DIAG
	SK_TIMER	PWaTimer;	/* Workaround Timer */
	SK_TIMER	HalfDupChkTimer;
#endif /* !SK_DIAG */
	SK_U32	PPrevShorts;	/* Previous Short Counter checking */
	SK_U32	PPrevFcs;		/* Previous FCS Error Counter checking */
	SK_U64	PPrevRx;		/* Previous RxOk Counter checking */
	SK_U64	PRxLim;			/* Previous RxOk Counter checking */
	SK_U64	LastOctets;		/* For half duplex hang check */
	int		PLinkResCt;		/* Link Restart Counter */
	int		PAutoNegTimeOut;/* Auto-negotiation timeout current value */
	int		PAutoNegTOCt;	/* Auto-negotiation Timeout Counter */
	int		PRxQSize;		/* Port Rx Queue Size in kB */
	int		PXSQSize;		/* Port Synchronous  Transmit Queue Size in kB */
	int		PXAQSize;		/* Port Asynchronous Transmit Queue Size in kB */
	SK_U32	PRxQRamStart;	/* Receive Queue RAM Buffer Start Address */
	SK_U32	PRxQRamEnd;		/* Receive Queue RAM Buffer End Address */
	SK_U32	PXsQRamStart;	/* Sync Tx Queue RAM Buffer Start Address */
	SK_U32	PXsQRamEnd;		/* Sync Tx Queue RAM Buffer End Address */
	SK_U32	PXaQRamStart;	/* Async Tx Queue RAM Buffer Start Address */
	SK_U32	PXaQRamEnd;		/* Async Tx Queue RAM Buffer End Address */
	SK_U32	PRxOverCnt;		/* Receive Overflow Counter */
	int		PRxQOff;		/* Rx Queue Address Offset */
	int		PXsQOff;		/* Synchronous Tx Queue Address Offset */
	int		PXaQOff;		/* Asynchronous Tx Queue Address Offset */
	int		PhyType;		/* PHY used on this port */
	int		PState;			/* Port status (reset, stop, init, run) */
	int		PPortUsage;		/* Driver Port Usage */
	SK_U16	PhyId1;			/* PHY Id1 on this port */
	SK_U16	PhyAddr;		/* MDIO/MDC PHY address */
	SK_U16	PIsave;			/* Saved Interrupt status word */
	SK_U16	PSsave;			/* Saved PHY status word */
	SK_U16	PGmANegAdv;		/* Saved GPhy AutoNegAdvertisment register */
	SK_BOOL	PHWLinkUp;		/* The hardware Link is up (wiring) */
	SK_BOOL	PLinkBroken;	/* Is Link broken ? */
	SK_BOOL	PCheckPar;		/* Do we check for parity errors ? */
	SK_BOOL	HalfDupTimerActive;
	SK_U8	PLinkCap;		/* Link Capabilities */
	SK_U8	PLinkModeConf;	/* Link Mode configured */
	SK_U8	PLinkMode;		/* Link Mode currently used */
	SK_U8	PLinkModeStatus;/* Link Mode Status */
	SK_U8	PLinkSpeedCap;	/* Link Speed Capabilities (10/100/1000 Mbps) */
	SK_U8	PLinkSpeed;		/* configured Link Speed (10/100/1000 Mbps) */
	SK_U8	PLinkSpeedUsed;	/* current Link Speed (10/100/1000 Mbps) */
	SK_U8	PFlowCtrlCap;	/* Flow Control Capabilities */
	SK_U8	PFlowCtrlMode;	/* Flow Control Mode */
	SK_U8	PFlowCtrlStatus;/* Flow Control Status */
	SK_U8	PMSCap;			/* Master/Slave Capabilities */
	SK_U8	PMSMode;		/* Master/Slave Mode */
	SK_U8	PMSStatus;		/* Master/Slave Status */
	SK_BOOL	PEnDetMode;		/* Energy Detect Mode */
	SK_BOOL	PAutoNegFail;	/* Auto-negotiation fail flag */
	SK_U8	PLipaAutoNeg;	/* Auto-negotiation possible with Link Partner */
	SK_U8	PCableLen;		/* Cable Length */
	SK_U8	PMdiPairLen[4];	/* MDI[0..3] Pair Length */
	SK_U8	PMdiPairSts[4];	/* MDI[0..3] Pair Diagnostic Status */
	SK_U8	PPhyPowerState;	/* PHY current power state */
	int		PMacColThres;	/* MAC Collision Threshold */
	int		PMacJamLen;		/* MAC Jam length */
	int		PMacJamIpgVal;	/* MAC Jam IPG */
	int		PMacJamIpgData;	/* MAC IPG Jam to Data */
	int		PMacBackOffLim;	/* MAC Back-off Limit */
	int		PMacDataBlind;	/* MAC Data Blinder */
	int		PMacIpgData;	/* MAC Data IPG */
	SK_U16	PMacAddr[3];	/* MAC address */
	SK_BOOL PMacLimit4;		/* reset collision counter and backoff algorithm */
} SK_GEPORT;

/*
 * Gigabit Ethernet Initialization Struct
 * (has to be included in the adapter context)
 */
typedef	struct s_GeInit {
	int			GIChipId;		/* Chip Identification Number */
	int			GIChipRev;		/* Chip Revision Number */
	SK_U8		GIPciHwRev;		/* PCI HW Revision Number */
	SK_U8		GIPciBus;		/* PCI Bus Type (PCI / PCI-X / PCI-Express) */
	SK_U8		GIPciMode;		/* PCI / PCI-X Mode @ Clock */
	SK_U8		GIPexWidth;		/* PCI-Express Negotiated Link Width */
	SK_BOOL		GIGenesis;		/* Genesis adapter ? */
	SK_BOOL		GIYukon;		/* YUKON family (1 and 2) */
	SK_BOOL		GIYukonLite;	/* YUKON-Lite chip */
	SK_BOOL		GIYukon2;		/* YUKON-2 chip (-XL, -EC or -FE) */
	SK_U8		GIConTyp;		/* Connector Type */
	SK_U8		GIPmdTyp;		/* PMD Type */
	SK_BOOL		GICopperType;	/* Copper Type adapter ? */
	SK_BOOL		GIPciSlot64;	/* 64-bit PCI Slot */
	SK_BOOL		GIPciClock66;	/* 66 MHz PCI Clock */
	SK_BOOL		GIVauxAvail;	/* VAUX available (YUKON) */
	SK_BOOL		GIVMainAvail;	/* VMAIN available */
	SK_BOOL		GIYukon32Bit;	/* 32-Bit YUKON adapter */
	SK_BOOL		GIAsfEnabled;	/* ASF subsystem enabled */
	SK_BOOL		GIAsfRunning;	/* ASF subsystem running */
	SK_BOOL		GIExtLeFormat;	/* Extended list element format (Yukon-Ext) */
	SK_BOOL		GIJumTcpSegSup;	/* TCP Segmentation of Jumbo frames supported ?*/
	SK_BOOL		GIGotoD3Cold;	/* System set to D3cold */
	SK_U16		GILedBlinkCtrl;	/* LED Blink Control */
	int			GIMacsFound;	/* Number of MACs found on this adapter */
	int			GIMacType;		/* MAC Type used on this adapter */
	int			GIChipCap;		/* Adapter's Capabilities */
	int			GIHwResInfo;	/* HW Resources / Application Information */
	int			GIHstClkFact;	/* Host Clock Factor (HstClk / 62.5 * 100) */
	int			GILevel;		/* Initialization Level completed */
	int			GIRamSize;		/* The RAM size of the adapter in kB */
	int			GIWolOffs;		/* WOL Register Offset (HW-Bug in Rev. A) */
	int			GIPexCapOffs;	/* PCI-E Capability Reg Off (moved in Y-Ex B0)*/
	int			GINumOfPattern;	/* Number of Pattern supported by HW */
	int			GITxIdxRepThres;/* Tx Index reporting threshold */
	SK_U32		GIRamOffs;		/* RAM Address Offset for addr calculation */
	SK_U32		GIPollTimerVal;	/* Descr. Poll Timer Init Val (HstClk ticks) */
	SK_U32		GIValIrqMask;	/* Value for Interrupt Mask */
	SK_U32		GIValHwIrqMask;	/* Value for HWE Interrupt Mask */
	SK_U32		GITimeStampCnt;	/* Time Stamp High Counter (YUKON only) */
	SK_GEPORT	GP[SK_MAX_MACS];/* Port Dependent Information */
	SK_HW_FEATURES HwF;			/* HW Features struct */
	SK_GEMACFUNC GIFunc;		/* MAC depedent functions */
} SK_GEINIT;

/*
 * Error numbers and messages for skxmac2.c and skgeinit.c
 */
#define SKERR_HWI_E001		(SK_ERRBASE_HWINIT+1)
#define SKERR_HWI_E001MSG	"SkXmClrExactAddr() has got illegal parameters"
#define SKERR_HWI_E002		(SKERR_HWI_E001+1)
#define SKERR_HWI_E002MSG	"SkGeInit(): Level 1 call missing"
#define SKERR_HWI_E003		(SKERR_HWI_E002+1)
#define SKERR_HWI_E003MSG	"SkGeInit() called with illegal init Level"
#define SKERR_HWI_E004		(SKERR_HWI_E003+1)
#define SKERR_HWI_E004MSG	"SkGeInitPort(): Queue Size illegal configured"
#define SKERR_HWI_E005		(SKERR_HWI_E004+1)
#define SKERR_HWI_E005MSG	"SkGeInitPort(): cannot init running ports"
#define SKERR_HWI_E006		(SKERR_HWI_E005+1)
#define SKERR_HWI_E006MSG	"SkGeInit() called with illegal Chip Id"
#define SKERR_HWI_E007		(SKERR_HWI_E006+1)
#define SKERR_HWI_E007MSG	"SkXmInitDupMd() called with invalid Dup Mode"
#define SKERR_HWI_E008		(SKERR_HWI_E007+1)
#define SKERR_HWI_E008MSG	"SkXmSetRxCmd() called with invalid Mode"
#define SKERR_HWI_E009		(SKERR_HWI_E008+1)
#define SKERR_HWI_E009MSG	"SkGeCfgSync() called although PXSQSize zero"
#define SKERR_HWI_E010		(SKERR_HWI_E009+1)
#define SKERR_HWI_E010MSG	"SkGeCfgSync() called with invalid parameters"
#define SKERR_HWI_E011		(SKERR_HWI_E010+1)
#define SKERR_HWI_E011MSG	"SkGeInitPort(): Receive Queue Size too small"
#define SKERR_HWI_E012		(SKERR_HWI_E011+1)
#define SKERR_HWI_E012MSG	"SkGeInitPort(): invalid Queue Size specified"
#define SKERR_HWI_E013		(SKERR_HWI_E012+1)
#define SKERR_HWI_E013MSG	"SkGeInitPort(): cfg changed for running queue"
#define SKERR_HWI_E014		(SKERR_HWI_E013+1)
#define SKERR_HWI_E014MSG	"SkGeInitPort(): unknown PortUsage specified"
#define SKERR_HWI_E015		(SKERR_HWI_E014+1)
#define SKERR_HWI_E015MSG	"Illegal Link Mode parameter"
#define SKERR_HWI_E016		(SKERR_HWI_E015+1)
#define SKERR_HWI_E016MSG	"Illegal Flow Control Mode parameter"
#define SKERR_HWI_E017		(SKERR_HWI_E016+1)
#define SKERR_HWI_E017MSG	"Illegal value specified for GIPollTimerVal"
#define SKERR_HWI_E018		(SKERR_HWI_E017+1)
#define SKERR_HWI_E018MSG	"FATAL: SkGeStopPort() does not terminate (Tx)"
#define SKERR_HWI_E019		(SKERR_HWI_E018+1)
#define SKERR_HWI_E019MSG	"Illegal Speed parameter"
#define SKERR_HWI_E020		(SKERR_HWI_E019+1)
#define SKERR_HWI_E020MSG	"Illegal Master/Slave parameter"
#define SKERR_HWI_E021		(SKERR_HWI_E020+1)
#define SKERR_HWI_E021MSG	"MacUpdateStats(): cannot update statistic counter"
#define SKERR_HWI_E022		(SKERR_HWI_E021+1)
#define SKERR_HWI_E022MSG	"MacStatistic(): illegal statistic base address"
#define SKERR_HWI_E023		(SKERR_HWI_E022+1)
#define SKERR_HWI_E023MSG	"SkGeInitPort(): Transmit Queue Size too small"
#define SKERR_HWI_E024		(SKERR_HWI_E023+1)
#define SKERR_HWI_E024MSG	"FATAL: SkGeStopPort() does not terminate (Rx)"
#define SKERR_HWI_E025		(SKERR_HWI_E024+1)
#define SKERR_HWI_E025MSG	"Link Partner not Auto-Neg. able"
#define SKERR_HWI_E026		(SKERR_HWI_E025+1)
#define SKERR_HWI_E026MSG	"PEX negotiated Link width not max."
#define SKERR_HWI_E027		(SKERR_HWI_E026+1)
#define SKERR_HWI_E027MSG	""

/* function prototypes ********************************************************/

#ifndef	SK_KR_PROTO

/*
 * public functions in skgeinit.c
 */
extern void SkGePortVlan(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	SK_BOOL Enable);

extern void SkGeRxRss(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	SK_BOOL	Enable);

extern void SkGeRxCsum(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	SK_BOOL Enable);

extern void	SkGePollRxD(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	SK_BOOL	PollRxD);

extern void	SkGePollTxD(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	SK_BOOL PollTxD);

extern void	SkGeYellowLED(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		State);

extern int	SkGeCfgSync(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	SK_U32	IntTime,
	SK_U32	LimCount,
	int		SyncMode);

extern void	SkGeLoadLnkSyncCnt(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	SK_U32	CntVal);

extern void	SkGeStopPort(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	int		Dir,
	int		RstMode);

extern int	SkGeInit(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Level);

extern void	SkGeDeInit(
	SK_AC	*pAC,
	SK_IOC	IoC);

extern int	SkGeInitPort(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port);

extern void	SkGeXmitLED(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Led,
	int		Mode);

extern void	SkGeInitRamIface(
	SK_AC	*pAC,
	SK_IOC	IoC);

extern int	SkGeInitAssignRamToQueues(
	SK_AC	*pAC,
	int		Port,
	SK_BOOL	DualNet);

extern void	DoInitRamQueue(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		QuIoOffs,
	SK_U32	QuStartAddr,
	SK_U32	QuEndAddr,
	int		QuType);

extern int	SkYuk2RestartRxBmu(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port);

extern void SkYuk2StopTx(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port);
extern void SkYuk2StartTx(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port);

/*
 * public functions in skxmac2.c
 */
extern void SkMacRxTxDisable(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port);

extern void	SkMacSoftRst(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port);

extern void	SkMacHardRst(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port);

extern void	SkMacClearRst(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port);

extern void	SkXmInitMac(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port);

extern void	SkGmInitMac(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port);

extern void SkMacInitPhy(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	SK_BOOL	DoLoop);

extern void SkMacIrqDisable(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port);

extern void	SkMacFlushTxFifo(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port);

extern void	SkMacFlushRxFifo(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port);

extern void	SkMacIrq(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port);

extern int	SkMacAutoNegDone(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port);

extern void	SkMacAutoNegLipaPhy(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	SK_U16	IStatus);

extern void	SkMacSetRxTxEn(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	int		Para);

extern int	SkMacRxTxEnable(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port);

extern void	SkMacPromiscMode(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	SK_BOOL	Enable);

extern void	SkMacHashing(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	SK_BOOL	Enable);

extern void	SkMacTimeStamp(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	SK_BOOL	Enable);

extern int	SkXmPhyRead(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	int		Addr,
	SK_U16	SK_FAR *pVal);

extern int	SkXmPhyWrite(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	int		Addr,
	SK_U16	Val);

extern int	SkGmPhyRead(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	int		Addr,
	SK_U16	SK_FAR *pVal);

extern int	SkGmPhyWrite(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	int		Addr,
	SK_U16	Val);

extern void	SkXmClrExactAddr(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	int		StartNum,
	int		StopNum);

extern void	SkXmInitDupMd(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port);

extern void	SkXmInitPauseMd(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port);

extern void	SkXmAutoNegLipaXmac(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	SK_U16	IStatus);

extern int SkXmUpdateStats(
	SK_AC	*pAC,
	SK_IOC	IoC,
	unsigned int Port);

extern int SkGmUpdateStats(
	SK_AC	*pAC,
	SK_IOC	IoC,
	unsigned int Port);

extern int SkXmMacStatistic(
	SK_AC	*pAC,
	SK_IOC	IoC,
	unsigned int Port,
	SK_U16	StatAddr,
	SK_U32	SK_FAR *pVal);

extern int SkGmMacStatistic(
	SK_AC	*pAC,
	SK_IOC	IoC,
	unsigned int Port,
	SK_U16	StatAddr,
	SK_U32	SK_FAR *pVal);

extern int SkXmResetCounter(
	SK_AC	*pAC,
	SK_IOC	IoC,
	unsigned int Port);

extern int SkGmResetCounter(
	SK_AC	*pAC,
	SK_IOC	IoC,
	unsigned int Port);

extern int SkXmOverflowStatus(
	SK_AC	*pAC,
	SK_IOC	IoC,
	unsigned int Port,
	SK_U16	IStatus,
	SK_U64	SK_FAR *pStatus);

extern int SkGmOverflowStatus(
	SK_AC	*pAC,
	SK_IOC	IoC,
	unsigned int Port,
	SK_U16	MacStatus,
	SK_U64	SK_FAR *pStatus);

extern int SkGmCableDiagStatus(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	SK_BOOL	StartTest);

#ifdef SK_PHY_LP_MODE
extern int SkGmEnterLowPowerMode(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	SK_U8	Mode);

extern int SkGmLeaveLowPowerMode(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port);
#endif /* SK_PHY_LP_MODE */

#ifdef SK_DIAG
extern void	SkGePhyRead(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	int		Addr,
	SK_U16	*pVal);

extern void	SkGePhyWrite(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	int		Addr,
	SK_U16	Val);

extern void	SkMacSetRxCmd(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	int		Mode);
extern void	SkMacCrcGener(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	SK_BOOL	Enable);
extern void	SkXmSendCont(
	SK_AC	*pAC,
	SK_IOC	IoC,
	int		Port,
	SK_BOOL	Enable);
#endif /* SK_DIAG */

#else	/* SK_KR_PROTO */

/*
 * public functions in skgeinit.c
 */
extern void	SkGePollRxD();
extern void	SkGePollTxD();
extern void	SkGeYellowLED();
extern int	SkGeCfgSync();
extern void	SkGeLoadLnkSyncCnt();
extern void	SkGeStopPort();
extern int	SkGeInit();
extern void	SkGeDeInit();
extern int	SkGeInitPort();
extern void	SkGeXmitLED();
extern void	SkGeInitRamIface();
extern int	SkGeInitAssignRamToQueues();
extern void	SkGePortVlan();
extern void	SkGeRxCsum();
extern void	SkGeRxRss();
extern void	DoInitRamQueue();
extern int	SkYuk2RestartRxBmu();
extern void SkYuk2StopTx();
extern void SkYuk2StartTx();

/*
 * public functions in skxmac2.c
 */
extern void	SkMacRxTxDisable();
extern void	SkMacSoftRst();
extern void	SkMacHardRst();
extern void	SkMacClearRst();
extern void	SkMacInitPhy();
extern int	SkMacRxTxEnable();
extern void	SkMacPromiscMode();
extern void	SkMacHashing();
extern void	SkMacIrqDisable();
extern void	SkMacFlushTxFifo();
extern void	SkMacFlushRxFifo();
extern void	SkMacIrq();
extern int	SkMacAutoNegDone();
extern void	SkMacAutoNegLipaPhy();
extern void	SkMacSetRxTxEn();
extern void	SkMacTimeStamp();
extern void	SkXmInitMac();
extern int	SkXmPhyRead();
extern int	SkXmPhyWrite();
extern void	SkGmInitMac();
extern int	SkGmPhyRead();
extern int	SkGmPhyWrite();
extern void	SkXmClrExactAddr();
extern void	SkXmInitDupMd();
extern void	SkXmInitPauseMd();
extern void	SkXmAutoNegLipaXmac();
extern int	SkXmUpdateStats();
extern int	SkGmUpdateStats();
extern int	SkXmMacStatistic();
extern int	SkGmMacStatistic();
extern int	SkXmResetCounter();
extern int	SkGmResetCounter();
extern int	SkXmOverflowStatus();
extern int	SkGmOverflowStatus();
extern int	SkGmCableDiagStatus();
#ifdef SK_PHY_LP_MODE
extern int	SkGmEnterLowPowerMode();
extern int	SkGmLeaveLowPowerMode();
#endif /* SK_PHY_LP_MODE */

#ifdef SK_DIAG
extern void	SkGePhyRead();
extern void	SkGePhyWrite();
extern void	SkMacSetRxCmd();
extern void	SkMacCrcGener();
extern void	SkXmSendCont();
#endif /* SK_DIAG */

#endif /* SK_KR_PROTO */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INC_SKGEINIT_H_ */

