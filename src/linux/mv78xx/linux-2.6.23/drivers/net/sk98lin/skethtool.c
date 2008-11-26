/******************************************************************************
 *
 * Name:        skethtool.c
 * Project:     GEnesis, PCI Gigabit Ethernet Adapter
 * Version:     $Revision: 1.9.2.8 $
 * Date:        $Date: 2007/06/25 14:40:17 $
 * Purpose:     All functions regarding ethtool handling
 *
 ******************************************************************************/

/******************************************************************************
 *
 *	(C)Copyright 1998-2002 SysKonnect GmbH.
 *	(C)Copyright 2002-2005 Marvell.
 *
 *	Driver for Marvell Yukon/2 chipset and SysKonnect Gigabit Ethernet 
 *      Server Adapters.
 *
 *	Author: Ralph Roesler (rroesler@syskonnect.de)
 *	        Mirko Lindner (mlindner@syskonnect.de)
 *
 *	Address all question to: linux@syskonnect.de
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	The information in this file is provided "AS IS" without warranty.
 *
 *****************************************************************************/

#include "h/skdrv1st.h"
#include "h/skdrv2nd.h"
#include "h/skversion.h"
#include <linux/ethtool.h>
#include <linux/module.h>
#include <linux/timer.h>

/******************************************************************************
 *
 * Local Functions
 *
 *****************************************************************************/
static void toggleLeds(unsigned long ptr);

/******************************************************************************
 *
 * External Functions and Data
 *
 *****************************************************************************/

extern void SkDimDisableModeration(SK_AC *pAC, int CurrentModeration);
extern void SkDimEnableModerationIfNeeded(SK_AC *pAC);

/******************************************************************************
 *
 * Defines
 *
 *****************************************************************************/

#ifndef ETHT_STATSTRING_LEN
#define ETHT_STATSTRING_LEN 32
#endif

#define ENABLE_FUTURE_ETH

#define SK98LIN_STAT(m)	sizeof(((SK_AC *)0)->m),offsetof(SK_AC, m)

#define SUPP_COPPER_ALL (SUPPORTED_10baseT_Half  | SUPPORTED_10baseT_Full  | \
                         SUPPORTED_100baseT_Half | SUPPORTED_100baseT_Full | \
                         SUPPORTED_1000baseT_Half| SUPPORTED_1000baseT_Full| \
                         SUPPORTED_TP)

#define ADV_COPPER_ALL  (ADVERTISED_10baseT_Half  | ADVERTISED_10baseT_Full  | \
                         ADVERTISED_100baseT_Half | ADVERTISED_100baseT_Full | \
                         ADVERTISED_1000baseT_Half| ADVERTISED_1000baseT_Full| \
                         ADVERTISED_TP)

#define SUPP_FIBRE_ALL  (SUPPORTED_1000baseT_Full | \
                         SUPPORTED_FIBRE          | \
                         SUPPORTED_Autoneg)

#define ADV_FIBRE_ALL   (ADVERTISED_1000baseT_Full | \
                         ADVERTISED_FIBRE          | \
                         ADVERTISED_Autoneg)

struct sk98lin_stats {
	char stat_string[ETHT_STATSTRING_LEN];
	int  sizeof_stat;
	int  stat_offset;
};

static struct sk98lin_stats sk98lin_etht_stats_port0[] = {
	{ "rx_packets" , SK98LIN_STAT(PnmiStruct.Stat[0].StatRxOkCts) },
	{ "tx_packets" , SK98LIN_STAT(PnmiStruct.Stat[0].StatTxOkCts) },
	{ "rx_bytes" , SK98LIN_STAT(PnmiStruct.Stat[0].StatRxOctetsOkCts) },
	{ "tx_bytes" , SK98LIN_STAT(PnmiStruct.Stat[0].StatTxOctetsOkCts) },
	{ "rx_errors" , SK98LIN_STAT(PnmiStruct.InErrorsCts) },
	{ "tx_errors" , SK98LIN_STAT(PnmiStruct.Stat[0].StatTxSingleCollisionCts) },
	{ "rx_dropped" , SK98LIN_STAT(PnmiStruct.RxNoBufCts) },
	{ "tx_dropped" , SK98LIN_STAT(PnmiStruct.TxNoBufCts) },
	{ "multicasts" , SK98LIN_STAT(PnmiStruct.Stat[0].StatRxMulticastOkCts) },
	{ "collisions" , SK98LIN_STAT(PnmiStruct.Stat[0].StatTxSingleCollisionCts) },
	{ "rx_length_errors" , SK98LIN_STAT(PnmiStruct.Stat[0].StatRxRuntCts) },
	{ "rx_buffer_overflow_errors" , SK98LIN_STAT(PnmiStruct.Stat[0].StatRxFifoOverflowCts) },
	{ "rx_crc_errors" , SK98LIN_STAT(PnmiStruct.Stat[0].StatRxFcsCts) },
	{ "rx_frame_errors" , SK98LIN_STAT(PnmiStruct.Stat[0].StatRxFramingCts) },
	{ "rx_too_short_errors" , SK98LIN_STAT(PnmiStruct.Stat[0].StatRxShortsCts) },
	{ "rx_too_long_errors" , SK98LIN_STAT(PnmiStruct.Stat[0].StatRxTooLongCts) },
	{ "rx_carrier_extension_errors", SK98LIN_STAT(PnmiStruct.Stat[0].StatRxCextCts) },
	{ "rx_symbol_errors" , SK98LIN_STAT(PnmiStruct.Stat[0].StatRxSymbolCts) },
	{ "rx_llc_mac_size_errors" , SK98LIN_STAT(PnmiStruct.Stat[0].StatRxIRLengthCts) },
	{ "rx_carrier_errors" , SK98LIN_STAT(PnmiStruct.Stat[0].StatRxCarrierCts) },
	{ "rx_jabber_errors" , SK98LIN_STAT(PnmiStruct.Stat[0].StatRxJabberCts) },
	{ "rx_missed_errors" , SK98LIN_STAT(PnmiStruct.Stat[0].StatRxMissedCts) },
	{ "tx_abort_collision_errors" , SK98LIN_STAT(stats.tx_aborted_errors) },
	{ "tx_carrier_errors" , SK98LIN_STAT(PnmiStruct.Stat[0].StatTxCarrierCts) },
	{ "tx_buffer_underrun_errors" , SK98LIN_STAT(PnmiStruct.Stat[0].StatTxFifoUnderrunCts) },
	{ "tx_heartbeat_errors" , SK98LIN_STAT(PnmiStruct.Stat[0].StatTxCarrierCts) } ,
	{ "tx_window_errors" , SK98LIN_STAT(stats.tx_window_errors) }
};

static struct sk98lin_stats sk98lin_etht_stats_port1[] = {
	{ "rx_packets" , SK98LIN_STAT(PnmiStruct.Stat[1].StatRxOkCts) },
	{ "tx_packets" , SK98LIN_STAT(PnmiStruct.Stat[1].StatTxOkCts) },
	{ "rx_bytes" , SK98LIN_STAT(PnmiStruct.Stat[1].StatRxOctetsOkCts) },
	{ "tx_bytes" , SK98LIN_STAT(PnmiStruct.Stat[1].StatTxOctetsOkCts) },
	{ "rx_errors" , SK98LIN_STAT(PnmiStruct.InErrorsCts) },
	{ "tx_errors" , SK98LIN_STAT(PnmiStruct.Stat[1].StatTxSingleCollisionCts) },
	{ "rx_dropped" , SK98LIN_STAT(PnmiStruct.RxNoBufCts) },
	{ "tx_dropped" , SK98LIN_STAT(PnmiStruct.TxNoBufCts) },
	{ "multicasts" , SK98LIN_STAT(PnmiStruct.Stat[1].StatRxMulticastOkCts) },
	{ "collisions" , SK98LIN_STAT(PnmiStruct.Stat[1].StatTxSingleCollisionCts) },
	{ "rx_length_errors" , SK98LIN_STAT(PnmiStruct.Stat[1].StatRxRuntCts) },
	{ "rx_buffer_overflow_errors" , SK98LIN_STAT(PnmiStruct.Stat[1].StatRxFifoOverflowCts) },
	{ "rx_crc_errors" , SK98LIN_STAT(PnmiStruct.Stat[1].StatRxFcsCts) },
	{ "rx_frame_errors" , SK98LIN_STAT(PnmiStruct.Stat[1].StatRxFramingCts) },
	{ "rx_too_short_errors" , SK98LIN_STAT(PnmiStruct.Stat[1].StatRxShortsCts) },
	{ "rx_too_long_errors" , SK98LIN_STAT(PnmiStruct.Stat[1].StatRxTooLongCts) },
	{ "rx_carrier_extension_errors", SK98LIN_STAT(PnmiStruct.Stat[1].StatRxCextCts) },
	{ "rx_symbol_errors" , SK98LIN_STAT(PnmiStruct.Stat[1].StatRxSymbolCts) },
	{ "rx_llc_mac_size_errors" , SK98LIN_STAT(PnmiStruct.Stat[1].StatRxIRLengthCts) },
	{ "rx_carrier_errors" , SK98LIN_STAT(PnmiStruct.Stat[1].StatRxCarrierCts) },
	{ "rx_jabber_errors" , SK98LIN_STAT(PnmiStruct.Stat[1].StatRxJabberCts) },
	{ "rx_missed_errors" , SK98LIN_STAT(PnmiStruct.Stat[1].StatRxMissedCts) },
	{ "tx_abort_collision_errors" , SK98LIN_STAT(stats.tx_aborted_errors) },
	{ "tx_carrier_errors" , SK98LIN_STAT(PnmiStruct.Stat[1].StatTxCarrierCts) },
	{ "tx_buffer_underrun_errors" , SK98LIN_STAT(PnmiStruct.Stat[1].StatTxFifoUnderrunCts) },
	{ "tx_heartbeat_errors" , SK98LIN_STAT(PnmiStruct.Stat[1].StatTxCarrierCts) } ,
	{ "tx_window_errors" , SK98LIN_STAT(stats.tx_window_errors) }
};

static int DuplexAutoNegConfMap[9][3]= {
	{ -1                     , -1         , -1              },
	{ 0                      , -1         , -1              },
	{ SK_LMODE_HALF          , DUPLEX_HALF, AUTONEG_DISABLE },
	{ SK_LMODE_FULL          , DUPLEX_FULL, AUTONEG_DISABLE },
	{ SK_LMODE_AUTOHALF      , DUPLEX_HALF, AUTONEG_ENABLE  },
	{ SK_LMODE_AUTOFULL      , DUPLEX_FULL, AUTONEG_ENABLE  },
	{ SK_LMODE_AUTOBOTH      , DUPLEX_FULL, AUTONEG_ENABLE  },
	{ SK_LMODE_AUTOSENSE     , -1         , -1              },
	{ SK_LMODE_INDETERMINATED, -1         , -1              }
};

static int SpeedConfMap[6][2] = {
	{ 0                       , -1         },
	{ SK_LSPEED_AUTO          , -1         },
	{ SK_LSPEED_10MBPS        , SPEED_10   },
	{ SK_LSPEED_100MBPS       , SPEED_100  },
	{ SK_LSPEED_1000MBPS      , SPEED_1000 },
	{ SK_LSPEED_INDETERMINATED, -1         }
};

static int AdvSpeedMap[6][2] = {
	{ 0                       , -1         },
	{ SK_LSPEED_AUTO          , -1         },
	{ SK_LSPEED_10MBPS        , ADVERTISED_10baseT_Half   | ADVERTISED_10baseT_Full },
	{ SK_LSPEED_100MBPS       , ADVERTISED_100baseT_Half  | ADVERTISED_100baseT_Full },
	{ SK_LSPEED_1000MBPS      , ADVERTISED_1000baseT_Half | ADVERTISED_1000baseT_Full},
	{ SK_LSPEED_INDETERMINATED, -1         }
};

#define SK98LIN_STATS_LEN sizeof(sk98lin_etht_stats_port0) / sizeof(struct sk98lin_stats)

static int nbrBlinkQuarterSeconds;
static int currentPortIndex;
static SK_BOOL isLocateNICrunning   = SK_FALSE;
static SK_BOOL isDualNetCard        = SK_FALSE;
static SK_BOOL doSwitchLEDsOn       = SK_FALSE;
static SK_BOOL boardWasDown[2]      = { SK_FALSE, SK_FALSE };
static struct timer_list locateNICtimer;

/******************************************************************************
 *
 * Ethtool Functions
 *
 *****************************************************************************/

/*****************************************************************************
 *
 * 	SkGeGetSettings - retrieves the current settings of the selected adapter
 *
 * Description:
 *	The current configuration of the selected adapter is returned.
 *	This configuration involves a)speed, b)duplex and c)autoneg plus
 *	a number of other variables.
 *
 * Returns:	N/A
 *
 */
int SkGeGetSettings(struct net_device *dev,
			struct ethtool_cmd *ecmd)
{
	DEV_NET			*pNet        = (DEV_NET*) dev->priv;
	SK_AC			*pAC         = pNet->pAC;
	int			port         = pNet->PortNr;
	SK_GEPORT		*pPort       = &pAC->GIni.GP[port];

	ecmd->phy_address = port;
	ecmd->speed       = SpeedConfMap[pPort->PLinkSpeedUsed][1];
	ecmd->duplex      = DuplexAutoNegConfMap[pPort->PLinkModeStatus][1];
	ecmd->autoneg     = DuplexAutoNegConfMap[pPort->PLinkModeStatus][2];
	ecmd->transceiver = XCVR_INTERNAL;

	if (pAC->GIni.GICopperType) {
		ecmd->port        = PORT_TP;
		ecmd->supported   = (SUPP_COPPER_ALL|SUPPORTED_Autoneg);
		if (pAC->GIni.GIGenesis) {
			ecmd->supported &= ~(SUPPORTED_10baseT_Half);
			ecmd->supported &= ~(SUPPORTED_10baseT_Full);
			ecmd->supported &= ~(SUPPORTED_100baseT_Half);
			ecmd->supported &= ~(SUPPORTED_100baseT_Full);
		} else {
			if (pAC->GIni.GIChipId == CHIP_ID_YUKON) {
				ecmd->supported &= ~(SUPPORTED_1000baseT_Half);
			} 
			if ((pAC->GIni.GIChipId == CHIP_ID_YUKON_FE) ||
				(pAC->GIni.GIChipId == CHIP_ID_YUKON_FE_P)) {
				ecmd->supported &= ~(SUPPORTED_1000baseT_Half);
				ecmd->supported &= ~(SUPPORTED_1000baseT_Full);
			}
		}
		if (pAC->GIni.GP[0].PLinkSpeed != SK_LSPEED_AUTO) {
			ecmd->advertising = AdvSpeedMap[pPort->PLinkSpeed][1];
			if (pAC->GIni.GIChipId == CHIP_ID_YUKON) {
				ecmd->advertising &= ~(SUPPORTED_1000baseT_Half);
			} 
		} else {
			ecmd->advertising = ecmd->supported;
		}
		if (ecmd->autoneg == AUTONEG_ENABLE) {
			ecmd->advertising |= ADVERTISED_Autoneg;
		} else {
			ecmd->advertising = ADVERTISED_TP;
		}
	} else {
		ecmd->port        = PORT_FIBRE;
		ecmd->supported   = (SUPP_FIBRE_ALL);
		ecmd->advertising = (ADV_FIBRE_ALL);
	}
	return(0);
}



/*****************************************************************************
 *
 * 	SkGeGetDrvInfo - returns generic driver and adapter information
 *
 * Description:
 *	Generic driver information is returned via this function, such as
 *	the name of the driver, its version and and firmware version.
 *	In addition to this, the location of the selected adapter is 
 *	returned as a bus info string (e.g. '01:05.0').
 *	
 * Returns:	N/A
 *
 */
void SkGeGetDrvInfo(struct net_device *dev,
			struct ethtool_drvinfo *ecmd)
{
	DEV_NET			*pNet        = (DEV_NET*) dev->priv;
	SK_AC			*pAC         = pNet->pAC;
	char			versionString[32];

	snprintf(versionString, 32, "%s (%s)", VER_STRING, PATCHLEVEL);
	strncpy(ecmd->driver, DRIVER_FILE_NAME , 32);
	strncpy(ecmd->version, versionString , 32);
	strncpy(ecmd->fw_version, "N/A", 32);
	strncpy(ecmd->bus_info, pci_name(pAC->PciDev), 32);

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,4,19)
	ecmd->n_stats = SK98LIN_STATS_LEN;
#endif
}

/*****************************************************************************
 *
 * 	SkGeGetWolSettings - retrieves the WOL settings of the 
 *	selected adapter
 *
 * Description:
 *	All current WOL settings of a selected adapter are placed in the 
 *	passed ethtool_wolinfo structure and are returned to the caller.
 *
 * Returns:	N/A
 *
 */
void SkGeGetWolSettings(struct net_device *dev,
			struct ethtool_wolinfo *ecmd)
{
	DEV_NET			*pNet        = (DEV_NET*) dev->priv;
	SK_AC			*pAC         = pNet->pAC;

	ecmd->supported = pAC->WolInfo.SupportedWolOptions;
	ecmd->wolopts   = pAC->WolInfo.ConfiguredWolOptions;
}

/*****************************************************************************
 *
 * 	SkGeGetPauseParam - retrieves the pause parameters
 *
 * Description:
 *	All current pause parameters of a selected adapter are placed 
 *	in the passed ethtool_pauseparam structure and are returned.
 *
 * Returns:	N/A
 *
 */
void SkGeGetPauseParam(struct net_device *dev,
			struct ethtool_pauseparam *ecmd)
{
	DEV_NET			*pNet        = (DEV_NET*) dev->priv;
	SK_AC			*pAC         = pNet->pAC;
	int			port         = pNet->PortNr;
	SK_GEPORT		*pPort       = &pAC->GIni.GP[port];

	/* Get the pause parameters */
	ecmd->rx_pause = 0;
	ecmd->tx_pause = 0;

	if (pPort->PFlowCtrlMode == SK_FLOW_MODE_LOC_SEND) {
		ecmd->tx_pause = 1;
	} 
	if ((pPort->PFlowCtrlMode == SK_FLOW_MODE_SYMMETRIC) ||
	    (pPort->PFlowCtrlMode == SK_FLOW_MODE_SYM_OR_REM)) {
		ecmd->tx_pause = 1;
		ecmd->rx_pause = 1;
	}

	if ((ecmd->rx_pause == 0) && (ecmd->tx_pause == 0)) {
		ecmd->autoneg = SK_FALSE;
	} else {
		ecmd->autoneg = SK_TRUE;
	}
}


/*****************************************************************************
 *
 * 	SkGeGetCoalesce - retrieves the IRQ moderation settings 
 *
 * Description:
 *	All current IRQ moderation settings of a selected adapter are placed 
 *	in the passed ethtool_coalesce structure and are returned.
 *
 * Returns:	N/A
 *
 */
int SkGeGetCoalesce(struct net_device *dev,
			struct ethtool_coalesce *ecmd)
{
	DEV_NET			*pNet        = (DEV_NET*) dev->priv;
	SK_AC			*pAC         = pNet->pAC;

	DIM_INFO *Info = &pAC->DynIrqModInfo;
	SK_BOOL UseTxIrqModeration = SK_FALSE;
	SK_BOOL UseRxIrqModeration = SK_FALSE;

	if (Info->IntModTypeSelect != C_INT_MOD_NONE) {
		if (CHIP_ID_YUKON_2(pAC)) {
			UseRxIrqModeration = SK_TRUE;
			UseTxIrqModeration = SK_TRUE;
		} else {
			if ((Info->MaskIrqModeration == IRQ_MASK_RX_ONLY) ||
			    (Info->MaskIrqModeration == IRQ_MASK_SP_RX)   ||
			    (Info->MaskIrqModeration == IRQ_MASK_RX_TX_SP)) {
				UseRxIrqModeration = SK_TRUE;
			}
			if ((Info->MaskIrqModeration == IRQ_MASK_TX_ONLY) ||
			    (Info->MaskIrqModeration == IRQ_MASK_SP_TX)   ||
			    (Info->MaskIrqModeration == IRQ_MASK_RX_TX_SP)) {
				UseTxIrqModeration = SK_TRUE;
			}
		}

		if (UseRxIrqModeration) {
			ecmd->rx_coalesce_usecs = 1000000 / Info->MaxModIntsPerSec;
		}
		if (UseTxIrqModeration) {
			ecmd->tx_coalesce_usecs = 1000000 / Info->MaxModIntsPerSec;
		}
		if (Info->IntModTypeSelect == C_INT_MOD_DYNAMIC) {
			ecmd->rate_sample_interval = Info->DynIrqModSampleInterval; 
			if (UseRxIrqModeration) {
				ecmd->use_adaptive_rx_coalesce = 1;
				ecmd->rx_coalesce_usecs_low = 
					1000000 / Info->MaxModIntsPerSecLowerLimit;
				ecmd->rx_coalesce_usecs_high = 
					1000000 / Info->MaxModIntsPerSecUpperLimit;
			}
			if (UseTxIrqModeration) {
				ecmd->use_adaptive_tx_coalesce = 1;
				ecmd->tx_coalesce_usecs_low = 
					1000000 / Info->MaxModIntsPerSecLowerLimit;
				ecmd->tx_coalesce_usecs_high = 
					1000000 / Info->MaxModIntsPerSecUpperLimit;
			}
		}
	}
	return(0);
}

/*****************************************************************************
 *
 * 	SkGeGetRxCsum - retrieves the RxCsum parameters
 *
 * Description:
 *	All current RxCsum parameters of a selected adapter are placed 
 *	in the passed net_device structure and are returned.
 *
 * Returns:	N/A
 *
 */
SK_U32 SkGeGetRxCsum(struct net_device *dev)
{

	DEV_NET			*pNet        = (DEV_NET*) dev->priv;
	SK_AC			*pAC         = pNet->pAC;
	int			port         = pNet->PortNr;

	return pAC->RxPort[port].UseRxCsum;
}


/*****************************************************************************
 *
 * 	SkGeGetStrings - retrieves the statistic strings
 *
 * Description:
 *	N/A
 *
 * Returns:	N/A
 *
 */
void SkGeGetStrings(struct net_device *dev, 
			u32 stringset,
			u8 *strings)
{
	DEV_NET			*pNet        = (DEV_NET*) dev->priv;
	int			port         = pNet->PortNr;
	int			i;

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,4,19)
	struct sk98lin_stats *sk98lin_etht_stats = 
		(port == 0) ? sk98lin_etht_stats_port0 : sk98lin_etht_stats_port1;

	switch(stringset) {
		case ETH_SS_STATS: {
			for(i=0; i < SK98LIN_STATS_LEN; i++) {
				memcpy(&strings[i * ETHT_STATSTRING_LEN],
					&(sk98lin_etht_stats[i].stat_string),
					ETHT_STATSTRING_LEN);
			}
			break;
		}
	}
#endif
}


/*****************************************************************************
 *
 * 	SkGeGetStatsLen - retrieves the statistic count
 *
 * Description:
 *	N/A
 *
 * Returns:	N/A
 *
 */
int SkGeGetStatsLen(struct net_device *dev)
{
	return SK98LIN_STATS_LEN;
}




/*****************************************************************************
 *
 * 	SkGeGetEthStats - retrieves the card statistics
 *
 * Description:
 *	All current statistics of a selected adapter are placed 
 *	in the passed ethtool_stats structure and are returned.
 *
 * Returns:	N/A
 *
 */
void SkGeGetEthStats(struct net_device *dev,
			struct ethtool_stats *stats,
			u64 *data)
{
	DEV_NET			*pNet        = (DEV_NET*) dev->priv;
	SK_AC			*pAC         = pNet->pAC;
	SK_U32			Size         = sizeof(SK_PNMI_STRUCT_DATA);
	SK_PNMI_STRUCT_DATA     *pPnmiStruct = &pAC->PnmiStruct;
	int			port         = pNet->PortNr;
	int			i;

	struct sk98lin_stats *sk98lin_etht_stats = 
		(port == 0) ? sk98lin_etht_stats_port0 : sk98lin_etht_stats_port1;

	if (netif_running(pAC->dev[port])) {
		SkPnmiGetStruct(pAC, pAC->IoBase, pPnmiStruct, &Size, port);
	}

	for(i = 0; i < SK98LIN_STATS_LEN; i++) {
		if (netif_running(pAC->dev[port])) {
			data[i] = (sk98lin_etht_stats[i].sizeof_stat ==
				sizeof(uint64_t)) ?
				*(uint64_t *)((char *)pAC +
				sk98lin_etht_stats[i].stat_offset) :
				*(uint32_t *)((char *)pAC +
				sk98lin_etht_stats[i].stat_offset);
		} else {
			data[i] = (sk98lin_etht_stats[i].sizeof_stat ==
				sizeof(uint64_t)) ? (uint64_t) 0 : (uint32_t) 0;
		}
	}
}


/*****************************************************************************
 *
 *	SkGeSetSettings - configures the settings of a selected adapter
 *
 * Description:
 *	Possible settings that may be altered are a)speed, b)duplex or 
 *	c)autonegotiation.
 *
 * Returns:
 *	==0:	everything fine, no error
 *	!=0:	the return value is the error code of the failure 
 */
int SkGeSetSettings(struct net_device *dev,
			struct ethtool_cmd *ecmd)
{
	DEV_NET			*pNet        = (DEV_NET*) dev->priv;
	SK_AC			*pAC         = pNet->pAC;
	int			port         = pNet->PortNr;

	SK_U32       Instance;
	char         Buf[4];
	unsigned int Len = 1;
	int Ret;

	if (port == 0) {
		Instance = (pAC->RlmtNets == 2) ? 1 : 2;
	} else {
		Instance = (pAC->RlmtNets == 2) ? 2 : 3;
	}

	if (((ecmd->autoneg == AUTONEG_DISABLE) || (ecmd->autoneg == AUTONEG_ENABLE)) &&
	    ((ecmd->duplex == DUPLEX_FULL) || (ecmd->duplex == DUPLEX_HALF))) {
		if (ecmd->autoneg == AUTONEG_DISABLE) {
			if (ecmd->duplex == DUPLEX_FULL) { 
				*Buf = (char) SK_LMODE_FULL;
			} else {
				*Buf = (char) SK_LMODE_HALF;
			}
		} else {
		/* Autoneg on. Enable autoparam */
			*Buf = (char) SK_LMODE_AUTOBOTH;
		}

		Ret = SkPnmiSetVar(pAC, pAC->IoBase, OID_SKGE_LINK_MODE, 
					&Buf, &Len, Instance, pNet->NetNr);
	
		if (Ret != SK_PNMI_ERR_OK) {
			return -EINVAL;
		}
	} else if (ecmd->autoneg == AUTONEG_ENABLE) {
	/* Set default values */
		*Buf = (char) SK_LMODE_AUTOBOTH;
		Ret = SkPnmiSetVar(pAC, pAC->IoBase, OID_SKGE_LINK_MODE, 
					&Buf, &Len, Instance, pNet->NetNr);
	}

	if ((ecmd->speed == SPEED_1000) ||
	    (ecmd->speed == SPEED_100)  || 
	    (ecmd->speed == SPEED_10)) {

		if (ecmd->autoneg == AUTONEG_ENABLE) {
			*Buf = (char) SK_LSPEED_AUTO;
		} else if (ecmd->speed == SPEED_1000) {
			*Buf = (char) SK_LSPEED_1000MBPS;
		} else if (ecmd->speed == SPEED_100) {
			*Buf = (char) SK_LSPEED_100MBPS;
		} else {
			*Buf = (char) SK_LSPEED_10MBPS;
		}

		Ret = SkPnmiSetVar(pAC, pAC->IoBase, OID_SKGE_SPEED_MODE, 
					&Buf, &Len, Instance, pNet->NetNr);
	
		if (Ret != SK_PNMI_ERR_OK) {
			return -EINVAL;
		}
	} else if (ecmd->autoneg == AUTONEG_ENABLE) {
		*Buf = (char) SK_LSPEED_AUTO;
		Ret = SkPnmiSetVar(pAC, pAC->IoBase, OID_SKGE_SPEED_MODE, 
					&Buf, &Len, Instance, pNet->NetNr);
	} else {
		return -EINVAL;
	}

	return(0);
}

/*****************************************************************************
 *
 *	SkGeSetWolSettings - configures the WOL settings of a selected adapter
 *
 * Description:
 *	The WOL settings of a selected adapter are configured regarding
 *	the parameters in the passed ethtool_wolinfo structure.
 *	Note that currently only wake on magic packet is supported!
 *
 * Returns:
 *	==0:	everything fine, no error
 *	!=0:	the return value is the error code of the failure 
 */
int SkGeSetWolSettings(struct net_device *dev,
			struct ethtool_wolinfo *ecmd)
{
	DEV_NET			*pNet        = (DEV_NET*) dev->priv;
	SK_AC			*pAC         = pNet->pAC;

	if (ecmd->wolopts != WAKE_MAGIC && ecmd->wolopts != 0)
		return -EOPNOTSUPP;

	if (((ecmd->wolopts & WAKE_MAGIC) == WAKE_MAGIC) || (ecmd->wolopts == 0)) {
		pAC->WolInfo.ConfiguredWolOptions = ecmd->wolopts;
		return 0;
	}
	return -EFAULT;
}


/*****************************************************************************
 *
 *	SkGeSetPauseParam - configures the pause parameters of an adapter
 *
 * Description:
 *	This function sets the Rx or Tx pause parameters 
 *
 * Returns:
 *	==0:	everything fine, no error
 *	!=0:	the return value is the error code of the failure 
 */
int SkGeSetPauseParam(struct net_device *dev,
			struct ethtool_pauseparam *ecmd)
{
	DEV_NET			*pNet        = (DEV_NET*) dev->priv;
	SK_AC			*pAC         = pNet->pAC;
	int			port         = pNet->PortNr;
	SK_GEPORT		*pPort = &pAC->GIni.GP[port];
	int			PrevSpeedVal = pPort->PLinkSpeedUsed;
	SK_U32			Instance;
	char			Buf[4];
	int			Ret;
	SK_BOOL			prevAutonegValue = SK_TRUE;
	int			prevTxPause      = 0;
	int			prevRxPause      = 0;
	unsigned int		Len              = 1;


        if (port == 0) {
                Instance = (pAC->RlmtNets == 2) ? 1 : 2;
        } else {
                Instance = (pAC->RlmtNets == 2) ? 2 : 3;
        }

	/*
	** we have to determine the current settings to see if 
	** the operator requested any modification of the flow 
	** control parameters...
	*/
	if (pPort->PFlowCtrlMode == SK_FLOW_MODE_LOC_SEND) {
		prevTxPause = 1;
	} 
	if ((pPort->PFlowCtrlMode == SK_FLOW_MODE_SYMMETRIC) ||
	    (pPort->PFlowCtrlMode == SK_FLOW_MODE_SYM_OR_REM)) {
		prevTxPause = 1;
		prevRxPause = 1;
	}

	if ((prevRxPause == 0) && (prevTxPause == 0)) {
		prevAutonegValue = SK_FALSE;
	}


	/*
	** perform modifications regarding the changes 
	** requested by the operator
	*/
	if (ecmd->autoneg != prevAutonegValue) {
		if (ecmd->autoneg == AUTONEG_DISABLE) {
			*Buf = (char) SK_FLOW_MODE_NONE;
		} else {
			*Buf = (char) SK_FLOW_MODE_SYMMETRIC;
		}
	} else {
		if(ecmd->rx_pause && ecmd->tx_pause) {
			*Buf = (char) SK_FLOW_MODE_SYMMETRIC;
		} else if (ecmd->rx_pause && !ecmd->tx_pause) {
			*Buf = (char) SK_FLOW_MODE_SYM_OR_REM;
		} else if(!ecmd->rx_pause && ecmd->tx_pause) {
			*Buf = (char) SK_FLOW_MODE_LOC_SEND;
		} else {
			*Buf = (char) SK_FLOW_MODE_NONE;
		}
	}

	Ret = SkPnmiSetVar(pAC, pAC->IoBase, OID_SKGE_FLOWCTRL_MODE,
			&Buf, &Len, Instance, pNet->NetNr);

	if (Ret != SK_PNMI_ERR_OK) {
		SK_DBG_MSG(NULL, SK_DBGMOD_DRV, SK_DBGCAT_CTRL,
		("ethtool (sk98lin): error changing rx/tx pause (%i)\n", Ret));
	}  else {
		Len = 1; /* set buffer length to correct value */
	}

	/*
	** It may be that autoneg has been disabled! Therefore
	** set the speed to the previously used value...
	*/
	*Buf = (char) PrevSpeedVal;

	Ret = SkPnmiSetVar(pAC, pAC->IoBase, OID_SKGE_SPEED_MODE, 
			&Buf, &Len, Instance, pNet->NetNr);

	if (Ret != SK_PNMI_ERR_OK) {
		SK_DBG_MSG(NULL, SK_DBGMOD_DRV, SK_DBGCAT_CTRL,
		("ethtool (sk98lin): error setting speed (%i)\n", Ret));
	}
        return 0;
}


/*****************************************************************************
 *
 *	SkGeSetCoalesce - configures the IRQ moderation of an adapter
 *
 * Description:
 *	Depending on the desired IRQ moderation parameters, either a) static,
 *	b) dynamic or c) no moderation is configured. 
 *
 * Returns:
 *	==0:	everything fine, no error
 *	!=0:	the return value is the error code of the failure 
 *
 * Notes:
 *	The supported timeframe for the coalesced interrupts ranges from
 *	33.333us (30 IntsPerSec) down to 25us (40.000 IntsPerSec).
 *	Any requested value that is not in this range will abort the request!
 */
int SkGeSetCoalesce(struct net_device *dev,
			struct ethtool_coalesce *ecmd)
{
	DEV_NET			*pNet        = (DEV_NET*) dev->priv;
	SK_AC			*pAC         = pNet->pAC;
	DIM_INFO		*Info        = &pAC->DynIrqModInfo;
	int			PrevModeration = Info->IntModTypeSelect;

	Info->IntModTypeSelect = C_INT_MOD_NONE; /* initial default */

	if ((ecmd->rx_coalesce_usecs) || (ecmd->tx_coalesce_usecs)) {
		if (ecmd->rx_coalesce_usecs) {
			if ((ecmd->rx_coalesce_usecs < 25) ||
			    (ecmd->rx_coalesce_usecs > 33333)) {
				return -EINVAL; 
			}
		}
		if (ecmd->tx_coalesce_usecs) {
			if ((ecmd->tx_coalesce_usecs < 25) ||
			    (ecmd->tx_coalesce_usecs > 33333)) {
				return -EINVAL; 
			}
		}
		if (!CHIP_ID_YUKON_2(pAC)) {
			if ((Info->MaskIrqModeration == IRQ_MASK_SP_RX) ||
			    (Info->MaskIrqModeration == IRQ_MASK_SP_TX) ||
			    (Info->MaskIrqModeration == IRQ_MASK_RX_TX_SP)) {
				Info->MaskIrqModeration = IRQ_MASK_SP_ONLY;
			} 
		}
		Info->IntModTypeSelect = C_INT_MOD_STATIC;
		if (ecmd->rx_coalesce_usecs) {
			Info->MaxModIntsPerSec = 
				1000000 / ecmd->rx_coalesce_usecs;
			if (!CHIP_ID_YUKON_2(pAC)) {
				if (Info->MaskIrqModeration == IRQ_MASK_TX_ONLY) {
					Info->MaskIrqModeration = IRQ_MASK_TX_RX;
				} 
				if (Info->MaskIrqModeration == IRQ_MASK_SP_ONLY) {
					Info->MaskIrqModeration = IRQ_MASK_SP_RX;
				} 
				if (Info->MaskIrqModeration == IRQ_MASK_SP_TX) {
					Info->MaskIrqModeration = IRQ_MASK_RX_TX_SP;
				}
			} else {
				Info->MaskIrqModeration = Y2_IRQ_MASK;
			}
		}
		if (ecmd->tx_coalesce_usecs) {
			Info->MaxModIntsPerSec = 
				1000000 / ecmd->tx_coalesce_usecs;
			if (!CHIP_ID_YUKON_2(pAC)) {
				if (Info->MaskIrqModeration == IRQ_MASK_RX_ONLY) {
					Info->MaskIrqModeration = IRQ_MASK_TX_RX;
				} 
				if (Info->MaskIrqModeration == IRQ_MASK_SP_ONLY) {
					Info->MaskIrqModeration = IRQ_MASK_SP_TX;
				} 
				if (Info->MaskIrqModeration == IRQ_MASK_SP_RX) {
					Info->MaskIrqModeration = IRQ_MASK_RX_TX_SP;
				}
			} else {
				Info->MaskIrqModeration = Y2_IRQ_MASK;
			}
		}
	}
	if ((ecmd->rate_sample_interval)  ||
	    (ecmd->rx_coalesce_usecs_low) ||
	    (ecmd->tx_coalesce_usecs_low) ||
	    (ecmd->rx_coalesce_usecs_high)||
	    (ecmd->tx_coalesce_usecs_high)) {
		if (ecmd->rate_sample_interval) {
			if ((ecmd->rate_sample_interval < 1) ||
			    (ecmd->rate_sample_interval > 10)) {
				return -EINVAL; 
			}
		}
		if (ecmd->rx_coalesce_usecs_low) {
			if ((ecmd->rx_coalesce_usecs_low < 25) ||
			    (ecmd->rx_coalesce_usecs_low > 33333)) {
				return -EINVAL; 
			}
		}
		if (ecmd->rx_coalesce_usecs_high) {
			if ((ecmd->rx_coalesce_usecs_high < 25) ||
			    (ecmd->rx_coalesce_usecs_high > 33333)) {
				return -EINVAL; 
			}
		}
		if (ecmd->tx_coalesce_usecs_low) {
			if ((ecmd->tx_coalesce_usecs_low < 25) ||
			    (ecmd->tx_coalesce_usecs_low > 33333)) {
				return -EINVAL; 
			}
		}
		if (ecmd->tx_coalesce_usecs_high) {
			if ((ecmd->tx_coalesce_usecs_high < 25) ||
			    (ecmd->tx_coalesce_usecs_high > 33333)) {
				return -EINVAL; 
			}
		}

		Info->IntModTypeSelect = C_INT_MOD_DYNAMIC;
		if (ecmd->rate_sample_interval) {
			Info->DynIrqModSampleInterval = 
				ecmd->rate_sample_interval; 
		}
		if (ecmd->rx_coalesce_usecs_low) {
			Info->MaxModIntsPerSecLowerLimit = 
				1000000 / ecmd->rx_coalesce_usecs_low;
		}
		if (ecmd->tx_coalesce_usecs_low) {
			Info->MaxModIntsPerSecLowerLimit = 
				1000000 / ecmd->tx_coalesce_usecs_low;
		}
		if (ecmd->rx_coalesce_usecs_high) {
			Info->MaxModIntsPerSecUpperLimit = 
				1000000 / ecmd->rx_coalesce_usecs_high;
		}
		if (ecmd->tx_coalesce_usecs_high) {
			Info->MaxModIntsPerSecUpperLimit = 
				1000000 / ecmd->tx_coalesce_usecs_high;
		}
	}

	if ((PrevModeration         == C_INT_MOD_NONE) &&
	    (Info->IntModTypeSelect != C_INT_MOD_NONE)) {
		SkDimEnableModerationIfNeeded(pAC);
	}
	if (PrevModeration != C_INT_MOD_NONE) {
		SkDimDisableModeration(pAC, PrevModeration);
		if (Info->IntModTypeSelect != C_INT_MOD_NONE) {
			SkDimEnableModerationIfNeeded(pAC);
		}
	}

        return 0;
}

#ifdef ENABLE_FUTURE_ETH
/*****************************************************************************
 *
 *	SkGeSetSG - set the SG parameters
 *
 * Description:
 *	This function sets the SG parameters 
 *
 * Returns:
 *	==0:	everything fine, no error
 *	!=0:	the return value is the error code of the failure 
 */
int SkGeSetSG(struct net_device *dev,
			u32 data)
{
	DEV_NET			*pNet        = (DEV_NET*) dev->priv;
	SK_AC			*pAC         = pNet->pAC;

	if (pAC->GIni.GIGenesis)
		return -EOPNOTSUPP;

	return ethtool_op_set_sg(dev, data);
}

#endif

#ifdef ENABLE_FUTURE_ETH

/*****************************************************************************
 *
 *	SkGeSetTxCsum - set the TxCsum parameters
 *
 * Description:
 *	This function sets the TxCsum parameters 
 *
 * Returns:
 *	==0:	everything fine, no error
 *	!=0:	the return value is the error code of the failure 
 */
int SkGeSetTxCsum(struct net_device *dev,
			u32 data)
{
	DEV_NET			*pNet        = (DEV_NET*) dev->priv;
	SK_AC			*pAC         = pNet->pAC;

	if (pAC->GIni.GIGenesis)
		return -EOPNOTSUPP;

	return ethtool_op_set_tx_csum(dev, data);
}

#endif

/*****************************************************************************
 *
 *	SkGeSetRxCsum - set the SkGeSetRxCsum parameters
 *
 * Description:
 *	This function sets the RxCsum parameters 
 *
 * Returns:
 *	==0:	everything fine, no error
 *	!=0:	the return value is the error code of the failure 
 */
int SkGeSetRxCsum(struct net_device *dev,
			u32 data)
{
	DEV_NET			*pNet        = (DEV_NET*) dev->priv;
	SK_AC			*pAC         = pNet->pAC;
	int			port         = pNet->PortNr;

	if (pAC->GIni.GIGenesis && data)
		return -EOPNOTSUPP;

	pAC->RxPort[port].UseRxCsum = data;
	return 0;
}

/*****************************************************************************
 *
 * 	SkGePhysId - start the locate NIC feature of the elected adapter 
 *
 * Description:
 *	This function is used if the user want to locate a particular NIC.
 *	All LEDs are regularly switched on and off, so the NIC can easily
 *	be identified.
 *
 * Returns:	
 *	==0:	everything fine, no error, locateNIC test was started
 *	!=0:	one locateNIC test runs already
 *
 */
int SkGePhysId(struct net_device *dev,
			u32 data)
{
	DEV_NET			*pNet        = (DEV_NET*) dev->priv;
	SK_AC			*pAC         = pNet->pAC;
	SK_IOC                   IoC       = pAC->IoBase;
	int			port         = pNet->PortNr;
	struct SK_NET_DEVICE	*pDev        = pAC->dev[port];
	int			OtherPort    = (port) ? 0 : 1;
	struct SK_NET_DEVICE	*pOtherDev   = pAC->dev[OtherPort];

	if (isLocateNICrunning) {
		return -EFAULT;
	}
	isLocateNICrunning = SK_TRUE;
	currentPortIndex = port;
	isDualNetCard = (pDev != pOtherDev) ? SK_TRUE : SK_FALSE;
	doSwitchLEDsOn = SK_FALSE;

	if (netif_running(pAC->dev[port])) {
		boardWasDown[0] = SK_FALSE;
	} else {
		(*pDev->open)(pDev);
		boardWasDown[0] = SK_TRUE;
	}

	if (isDualNetCard) {
		if (netif_running(pAC->dev[OtherPort])) {
			boardWasDown[1] = SK_FALSE;
		} else {
			(*pOtherDev->open)(pOtherDev);
			boardWasDown[1] = SK_TRUE;
		}
	}

	if ( (pAC->GIni.GIChipId == CHIP_ID_YUKON_XL) ||
	     (pAC->GIni.GIChipId == CHIP_ID_YUKON_EC_U) ) {
	  SkMacClearRst(pAC, IoC, port);
	}


	if ((data < 1) || (data > 30)) {
		data = 3; /* three seconds default */
	}
	nbrBlinkQuarterSeconds = 4*data;

	init_timer(&locateNICtimer);
	locateNICtimer.function = toggleLeds;
	locateNICtimer.data     = (unsigned long) pNet;
	locateNICtimer.expires  = jiffies + HZ; /* initially 1sec */
	add_timer(&locateNICtimer);

	return 0;
}

/*****************************************************************************
 *
 * 	toggleLeds - Changes the LED state of an adapter
 *
 * Description:
 *	This function changes the current state of all LEDs of an adapter so
 *	that it can be located by a user. If the requested time interval for
 *	this test has elapsed, this function cleans up everything that was 
 *	temporarily setup during the locate NIC test. This involves of course
 *	also closing or opening any adapter so that the initial board state 
 *	is recovered.
 *
 * Returns:	N/A
 *
 */
static void toggleLeds(
unsigned long ptr)  /* holds the pointer to adapter control context */
{
	DEV_NET              *pNet      = (DEV_NET*) ptr;
	SK_AC                *pAC       = pNet->pAC;
	int                   port      = pNet->PortNr;
	SK_IOC                IoC       = pAC->IoBase;
	struct SK_NET_DEVICE *pDev      = pAC->dev[port];
	int                   OtherPort = (port) ? 0 : 1;
	struct SK_NET_DEVICE *pOtherDev = pAC->dev[OtherPort];
	SK_U16                PageSelect;
	SK_BOOL               YukLedState;

	SK_U16  YukLedOn = (PHY_M_LED_MO_DUP(MO_LED_ON)  |
			    PHY_M_LED_MO_10(MO_LED_ON)   |
			    PHY_M_LED_MO_100(MO_LED_ON)  |
			    PHY_M_LED_MO_1000(MO_LED_ON) | 
			    PHY_M_LED_MO_RX(MO_LED_ON));
	SK_U16  YukLedOff = (PHY_M_LED_MO_DUP(MO_LED_OFF)  |
			     PHY_M_LED_MO_10(MO_LED_OFF)   |
			     PHY_M_LED_MO_100(MO_LED_OFF)  |
			     PHY_M_LED_MO_1000(MO_LED_OFF) | 
			     PHY_M_LED_MO_RX(MO_LED_OFF) | 
			     PHY_M_LED_MO_TX(MO_LED_OFF));

	nbrBlinkQuarterSeconds--;
	if (nbrBlinkQuarterSeconds <= 0) {
	  /*
	   * We have to stop the device again in case the device has no
	   * been up.
	   */

	  if (!boardWasDown[0]) {
	    /*
	     * The board is already up as we bring it up in case it is not.
	     */
	  } else {
	    (*pDev->stop)(pDev);
	  }
	  if (isDualNetCard) {
	    if (!boardWasDown[1]) {
	      /*
	       * The board is already up as we bring it up in case it is not.
	       */
	    } else {
	      (*pOtherDev->stop)(pOtherDev);
	    }
	    
	  }

	  isDualNetCard      = SK_FALSE;
	  isLocateNICrunning = SK_FALSE;
	  return;
	}
	doSwitchLEDsOn = (doSwitchLEDsOn) ? SK_FALSE : SK_TRUE;

	if ( (doSwitchLEDsOn) && (nbrBlinkQuarterSeconds > 2) ){
		if (pAC->GIni.GIGenesis) {
			SK_OUT8(IoC,MR_ADDR(port,LNK_LED_REG),(SK_U8)SK_LNK_ON);
			SkGeYellowLED(pAC,IoC,LED_ON >> 1);
			SkGeXmitLED(pAC,IoC,MR_ADDR(port,RX_LED_INI),SK_LED_TST);
			if (pAC->GIni.GP[port].PhyType == SK_PHY_BCOM) {
				SkXmPhyWrite(pAC,IoC,port,PHY_BCOM_P_EXT_CTRL,PHY_B_PEC_LED_ON);
			} else if (pAC->GIni.GP[port].PhyType == SK_PHY_LONE) {
				SkXmPhyWrite(pAC,IoC,port,PHY_LONE_LED_CFG,0x0800);
			} else {
				SkGeXmitLED(pAC,IoC,MR_ADDR(port,TX_LED_INI),SK_LED_TST);
			}
		} else {
		  if ( (pAC->GIni.GIChipId == CHIP_ID_YUKON_XL) ||
 		       (pAC->GIni.GIChipId == CHIP_ID_YUKON_2(pAC)) ||
 		       (pAC->GIni.GIChipId == CHIP_ID_YUKON_EC_U) ) {

		    YukLedOn = 0;
		    YukLedState = 1;
 		    YukLedOn |= PHY_M_LEDC_INIT_CTRL(YukLedState ? 9 : 8);
		    YukLedState = 1;
		    YukLedOn |= PHY_M_LEDC_STA1_CTRL(YukLedState ? 9 : 8);
		    YukLedState = 1;
		    YukLedOn |= PHY_M_LEDC_STA0_CTRL(YukLedState ? 9 : 8);
		    YukLedState = 1;
		    YukLedOn |= PHY_M_LEDC_LOS_CTRL(YukLedState ? 9 : 8);
		       
		    /* save page register */
		    SkGmPhyRead(pAC, IoC, port, PHY_MARV_EXT_ADR, &PageSelect);
		    
		    /* select page 3 for LED control */
		    SkGmPhyWrite(pAC, IoC, port, PHY_MARV_EXT_ADR, 3);
		    
		    SkGmPhyWrite(pAC, IoC, port, PHY_MARV_PHY_CTRL, YukLedOn);
		    
		    /* restore page register */
		    SkGmPhyWrite(pAC, IoC, port, PHY_MARV_EXT_ADR, PageSelect);
		  }
		  else {
		    SkGmPhyWrite(pAC,IoC,port,PHY_MARV_LED_OVER,YukLedOn);	
		  }
		}
	} else {
		if (pAC->GIni.GIGenesis) {
		        
			SK_OUT8(IoC,MR_ADDR(port,LNK_LED_REG),(SK_U8)SK_LNK_OFF);
			SkGeYellowLED(pAC,IoC,LED_OFF >> 1);
			SkGeXmitLED(pAC,IoC,MR_ADDR(port,RX_LED_INI),SK_LED_DIS);
			if (pAC->GIni.GP[port].PhyType == SK_PHY_BCOM) {
				SkXmPhyWrite(pAC,IoC,port,PHY_BCOM_P_EXT_CTRL,PHY_B_PEC_LED_OFF);
			} else if (pAC->GIni.GP[port].PhyType == SK_PHY_LONE) {
				SkXmPhyWrite(pAC,IoC,port,PHY_LONE_LED_CFG,PHY_L_LC_LEDT);
			} else {
				SkGeXmitLED(pAC,IoC,MR_ADDR(port,TX_LED_INI),SK_LED_DIS);
			}
		} else {
		  if ( (pAC->GIni.GIChipId == CHIP_ID_YUKON_XL) ||
 		       (pAC->GIni.GIChipId == CHIP_ID_YUKON_2(pAC)) ||
 		       (pAC->GIni.GIChipId == CHIP_ID_YUKON_EC_U) ) {

		    YukLedOn = 0;
		    YukLedState = 1;
 		    YukLedOn |= PHY_M_LEDC_INIT_CTRL(YukLedState ? 9 : 8);
		    YukLedState = 1;
		    YukLedOn |= PHY_M_LEDC_STA1_CTRL(YukLedState ? 9 : 8);
		    YukLedState = 1;
		    YukLedOn |= PHY_M_LEDC_STA0_CTRL(YukLedState ? 9 : 8);
		    YukLedState = 1;
		    YukLedOn |= PHY_M_LEDC_LOS_CTRL(YukLedState ? 9 : 8);

		    /* save page register */
		    SkGmPhyRead(pAC, IoC, port, PHY_MARV_EXT_ADR, &PageSelect);
		    
		    /* select page 3 for LED control */
		    SkGmPhyWrite(pAC, IoC, port, PHY_MARV_EXT_ADR, 3);
		    
		    SkGmPhyWrite(pAC, IoC, port, PHY_MARV_PHY_CTRL, YukLedOff);
		    
		    /* restore page register */
		    SkGmPhyWrite(pAC, IoC, port, PHY_MARV_EXT_ADR, PageSelect);
		  }
		  else {
		    SkGmPhyWrite(pAC,IoC,port,PHY_MARV_LED_OVER,YukLedOff);	
		  }
		}
	}

	locateNICtimer.function = toggleLeds;
	locateNICtimer.data     = (unsigned long) pNet;
	locateNICtimer.expires  = jiffies + (HZ/4);
	add_timer(&locateNICtimer);
} 

#ifdef NETIF_F_TSO
/*****************************************************************************
 *
 *	SkGeSetTSO - set the TSO parameters
 *
 * Description:
 *	This function sets the TSO parameters 
 *
 * Returns:
 *	==0:	everything fine, no error
 *	!=0:	the return value is the error code of the failure 
 */
int SkGeSetTSO(struct net_device *dev,
			u32 data)
{
	DEV_NET			*pNet        = (DEV_NET*) dev->priv;
	SK_AC			*pAC         = pNet->pAC;

	if (CHIP_ID_YUKON_2(pAC)) {
		if (data) {
			if ((pAC->GIni.GIChipId == CHIP_ID_YUKON_EC_U) && (dev->mtu > ETH_MAX_MTU)) {
				dev->features &= ~NETIF_F_TSO;
			} else {
				dev->features |= NETIF_F_TSO;
			}
		} else {
			dev->features &= ~NETIF_F_TSO;
		}
		return 0;
	}
	return -EOPNOTSUPP;

}
#endif




/*******************************************************************************
 *
 * End of file
 *
 ******************************************************************************/
