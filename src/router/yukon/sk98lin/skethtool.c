/******************************************************************************
 *
 * Name:        skethtool.c
 * Project:     Yukon/Yukon2, PCI Gigabit Ethernet Adapter
 * Purpose:     All functions regarding ethtool handling
 *
 ******************************************************************************/

/******************************************************************************
 *
 *	    (C)Copyright 1998-2002 SysKonnect GmbH.
 *	    (C)Copyright 2002-2012 Marvell.
 *
 *	    Driver for Marvell Yukon/2 chipset and SysKonnect Gigabit Ethernet
 *      Server Adapters.
 *
 *	    Address all question to: support@marvell.com
 *
 *      LICENSE:
 *      (C)Copyright Marvell.
 *      
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *      
 *      The information in this file is provided "AS IS" without warranty.
 *      /LICENSE
 *
 *****************************************************************************/

#include "h/skdrv1st.h"
#include "h/skdrv2nd.h"
#include "h/skversion.h"
#include <linux/ethtool.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/random.h>


/******************************************************************************
 *
 * Local Functions
 *
 *****************************************************************************/
static void toggleLeds(unsigned long ptr);
#ifdef SK98LIN_DIAG
static void SkGeGetDiagStrings(u8 *data);
#endif
/******************************************************************************
 *
 * External Functions and Data
 *
 *****************************************************************************/

extern void SkDimDisableModeration(SK_AC *pAC, int CurrentModeration);
extern void SkDimEnableModerationIfNeeded(SK_AC *pAC);
extern SK_BOOL SkY2AllocateResources(SK_AC *pAC);
extern void SkY2FreeResources(SK_AC *pAC);
extern SK_BOOL	BoardAllocMem(SK_AC *pAC);
extern void	BoardFreeMem(SK_AC *pAC);
extern int SK_DEVINIT SkGeTestInt(struct SK_NET_DEVICE *dev, SK_AC *pAC, SK_U32 Int);

#ifdef SK98LIN_DIAG_LOOPBACK
extern int SkGeOpen(struct SK_NET_DEVICE *dev);
extern int SkGeClose(struct SK_NET_DEVICE *dev);
extern int SkY2Xmit(struct sk_buff *skb,struct SK_NET_DEVICE *dev);
#endif

#ifdef DEBUG
extern void DumpMsg(struct sk_buff*, char*);
#endif


/******************************************************************************
 *
 * Defines
 *
 *****************************************************************************/

#ifndef ETHT_STATSTRING_LEN
#define ETHT_STATSTRING_LEN 32
#endif


#ifdef SK98LIN_DIAG
const char SkGeGstringsTest[][ETH_GSTRING_LEN] = {
	"Register test    (offline)", "Eeprom test      (offline)",
	"Interrupt test   (offline)", "Loopback test    (offline)"
};
#define SK98LIN_TEST_LEN sizeof(SkGeGstringsTest) / ETH_GSTRING_LEN
#endif


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
	DEV_NET			*pNet        = PPRIV;
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
		ecmd->supported   &= ~(SUPPORTED_1000baseT_Half);


		if (pAC->GIni.GIChipId == CHIP_ID_YUKON) {
			ecmd->supported &= ~(SUPPORTED_1000baseT_Half);
		}
		if ((pAC->GIni.GIChipId == CHIP_ID_YUKON_FE) ||
			(pAC->GIni.GIChipId == CHIP_ID_YUKON_FE_P)) {
			ecmd->supported &= ~(SUPPORTED_1000baseT_Half);
			ecmd->supported &= ~(SUPPORTED_1000baseT_Full);
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
	DEV_NET			*pNet        = PPRIV;
	SK_AC			*pAC         = pNet->pAC;
	char			versionString[32];

	snprintf(versionString, 32, "%s (%s)", VER_STRING, PATCHLEVEL);
	strncpy(ecmd->driver, DRIVER_FILE_NAME , 32);
	strncpy(ecmd->version, versionString , 32);
	strncpy(ecmd->fw_version, FW_VERSION, 32);
	strncpy(ecmd->bus_info, pci_name(pAC->PciDev), 32);
	ecmd->n_stats = SK98LIN_STATS_LEN;
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
	DEV_NET			*pNet        = PPRIV;
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
	DEV_NET			*pNet        = PPRIV;
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
		ecmd->rx_pause = 1;
		ecmd->tx_pause = 1;
	}

	/* Check the autoneg value */
	ecmd->autoneg = 0;
	ecmd->autoneg = pPort->PLinkMode != SK_LMODE_HALF &&
		pPort->PLinkMode != SK_LMODE_FULL;
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
	DEV_NET			*pNet        = PPRIV;
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

	DEV_NET			*pNet        = PPRIV;
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
	DEV_NET	*pNet     = PPRIV;
	int	port      = pNet->PortNr;
	int	i;

	struct sk98lin_stats *sk98lin_etht_stats =
		(port == 0) ? sk98lin_etht_stats_port0 : sk98lin_etht_stats_port1;

#ifdef SK98LIN_DIAG
	/* Diag/test strings */
	if (stringset == ETH_SS_TEST) {
		SkGeGetDiagStrings(strings);
	}
#endif

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
 * 	 SkGeGetSsetCount - retrieves the new statistic count
 *
 * Description:
 *	N/A
 *
 * Returns:	N/A
 *
 */
int SkGeGetSsetCount(struct net_device *dev, int sset)
{
	switch (sset) {
#ifdef SK98LIN_DIAG
	case ETH_SS_TEST:
		return SK98LIN_TEST_LEN;
#endif
	case ETH_SS_STATS:
		return SK98LIN_STATS_LEN;
	default:
		return -EOPNOTSUPP;
	}
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
	DEV_NET			*pNet        = PPRIV;
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
	DEV_NET			*pNet        = PPRIV;
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

	/* Not supported configuration */
	if ((ecmd->autoneg == AUTONEG_DISABLE) && (ecmd->speed == SPEED_1000) &&
		(ecmd->duplex == DUPLEX_HALF)) {
		printk("Unsupported speed/duplex configuration. 1000HD not available.\n");
		return -EINVAL;
	} else if ((ecmd->autoneg == AUTONEG_DISABLE) && (ecmd->speed == SPEED_1000)) {
		printk("Autonegotiation mandatory for Gigabit speed.\n");
		return -EINVAL;
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
		} else {
		/* Set the parameter */
			pAC->LinkInfo[port].PLinkModeConf = (int) *Buf;
		}
	} else if (ecmd->autoneg == AUTONEG_ENABLE) {
	/* Set default values */
		*Buf = (char) SK_LMODE_AUTOBOTH;
		Ret = SkPnmiSetVar(pAC, pAC->IoBase, OID_SKGE_LINK_MODE,
					&Buf, &Len, Instance, pNet->NetNr);
		pAC->LinkInfo[port].PLinkModeConf = SK_LMODE_AUTOBOTH;
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
		} else {
			pAC->LinkInfo[port].PLinkSpeed = (int) *Buf;
		}
	} else if (ecmd->autoneg == AUTONEG_ENABLE) {
		*Buf = (char) SK_LSPEED_AUTO;
		Ret = SkPnmiSetVar(pAC, pAC->IoBase, OID_SKGE_SPEED_MODE,
					&Buf, &Len, Instance, pNet->NetNr);
		pAC->LinkInfo[port].PLinkSpeed = SK_LSPEED_AUTO;
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
	DEV_NET			*pNet        = PPRIV;
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
	DEV_NET			*pNet        = PPRIV;
	SK_AC			*pAC         = pNet->pAC;
	int			port         = pNet->PortNr;
	SK_GEPORT		*pPort = &pAC->GIni.GP[port];
	int			PrevSpeedVal = pPort->PLinkSpeedUsed;
	SK_U32			Instance;
	char			Buf[4];
	int			Ret = SK_PNMI_ERR_OK;
	SK_BOOL			prevAutonegValue = SK_TRUE;
	int			prevTxPause      = 0;
	int			prevRxPause      = 0;
	unsigned int		Len              = 1;


        if (port == 0) {
                Instance = (pAC->RlmtNets == 2) ? 1 : 2;
        } else {
                Instance = (pAC->RlmtNets == 2) ? 2 : 3;
        }

	if ((pPort->PFlowCtrlMode == SK_FLOW_MODE_SYMMETRIC) ||
	    (pPort->PFlowCtrlMode == SK_FLOW_MODE_SYM_OR_REM)) {
		prevTxPause = 1;
		prevRxPause = 1;
	}

	/* Bugfix for not supported TX only mode */
	if (ecmd->rx_pause != ecmd->tx_pause) {
		printk("Unsupported flow control configuration. Single tx or rx configuration not supported.\n");
		return 1;
	}

	if ((ecmd->rx_pause != prevRxPause) ||
	(ecmd->tx_pause != prevTxPause)) {
	/* Pause frames changed - Set the new value*/
		if(ecmd->rx_pause && ecmd->tx_pause) {
			*Buf = (char) SK_FLOW_MODE_SYMMETRIC;
		} else if (ecmd->rx_pause && !ecmd->tx_pause) {
			printk("Unsupported flow control configuration. Single tx not available.\n");
			*Buf = (char) SK_FLOW_MODE_SYM_OR_REM;
		} else if(!ecmd->rx_pause && ecmd->tx_pause) {
			*Buf = (char) SK_FLOW_MODE_LOC_SEND;
		} else {
			*Buf = (char) SK_FLOW_MODE_NONE;
		}

		/* Set the data */
		Ret = SkPnmiSetVar(pAC, pAC->IoBase, OID_SKGE_FLOWCTRL_MODE,
			&Buf, &Len, Instance, pNet->NetNr);

		if (Ret != SK_PNMI_ERR_OK) {
			SK_DBG_MSG(NULL, SK_DBGMOD_DRV, SK_DBGCAT_CTRL,
			("ethtool (sk98lin): error changing rx/tx pause (%i)\n", Ret));
		}
	}

	/* Auto-negotiation */
	prevAutonegValue = 0;
	prevAutonegValue = pPort->PLinkMode != SK_LMODE_HALF &&
		pPort->PLinkMode != SK_LMODE_FULL;

	if (ecmd->autoneg != prevAutonegValue) {
	/* Autoneg changed */
		if (ecmd->autoneg == AUTONEG_DISABLE) {
			*Buf = (char) SK_LMODE_FULL;
		} else {
		/* Autoneg on. Enable autoparam */
			*Buf = (char) SK_LMODE_AUTOBOTH;
		}

		Ret = SkPnmiSetVar(pAC, pAC->IoBase, OID_SKGE_LINK_MODE,
			&Buf, &Len, Instance, pNet->NetNr);

		if (Ret != SK_PNMI_ERR_OK) {
			SK_DBG_MSG(NULL, SK_DBGMOD_DRV, SK_DBGCAT_CTRL,
			("ethtool (sk98lin): error changing link mode (%i)\n", Ret));
		}
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
	DEV_NET			*pNet        = PPRIV;
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

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
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
	return ethtool_op_set_sg(dev, data);
}

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
	return ethtool_op_set_tx_csum(dev, data);
}

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
	DEV_NET			*pNet        = PPRIV;
	SK_AC			*pAC         = pNet->pAC;
	int			port         = pNet->PortNr;

	pAC->RxPort[port].UseRxCsum = data;
	return 0;
}
#endif

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
	DEV_NET			*pNet        = PPRIV;
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
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,29)
		(*pDev->netdev_ops->ndo_open)(pDev);
#else
		(*pDev->open)(pDev);
#endif
		boardWasDown[0] = SK_TRUE;
	}

	if (isDualNetCard) {
		if (netif_running(pAC->dev[OtherPort])) {
			boardWasDown[1] = SK_FALSE;
		} else {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,29)
			(*pOtherDev->netdev_ops->ndo_open)(pOtherDev);
#else
			(*pOtherDev->open)(pOtherDev);
#endif
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
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,29)
	    (*pDev->netdev_ops->ndo_stop)(pDev);
#else
	    (*pDev->stop)(pDev);
#endif
	  }
	  if (isDualNetCard) {
	    if (!boardWasDown[1]) {
	      /*
	       * The board is already up as we bring it up in case it is not.
	       */
	    } else {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,29)
	      (*pOtherDev->netdev_ops->ndo_stop)(pOtherDev);
#else
	      (*pOtherDev->stop)(pOtherDev);
#endif
	    }

	  }

	  isDualNetCard      = SK_FALSE;
	  isLocateNICrunning = SK_FALSE;
	  return;
	}
	doSwitchLEDsOn = (doSwitchLEDsOn) ? SK_FALSE : SK_TRUE;

	if ( (doSwitchLEDsOn) && (nbrBlinkQuarterSeconds > 2) ){
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
		} else {
			SkGmPhyWrite(pAC,IoC,port,PHY_MARV_LED_OVER,YukLedOn);
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
		} else {
			SkGmPhyWrite(pAC,IoC,port,PHY_MARV_LED_OVER,YukLedOff);
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
	DEV_NET			*pNet        = PPRIV;
	SK_AC			*pAC         = pNet->pAC;

	if (CHIP_ID_YUKON_2(pAC)) {
		if (data) {
			if (((pAC->GIni.GIChipId == CHIP_ID_YUKON_EC_U) && (dev->mtu > ETH_MAX_MTU))
#ifdef	SK_AVB
			|| (pAC->GIni.GIChipId == CHIP_ID_YUKON_OPT)
			|| (pAC->GIni.GIChipId == CHIP_ID_YUKON_PRM)
#endif
			) {
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


/*
 * Ring parameter
 */


/*****************************************************************************
 *
 *	SkGeGetRingParam - Show ring parameter
 *
 * Description:
 *	Show the rx/tx ring parameter information for the specified
 *	ethernet device for
 *
 * Returns:
 *	N/A
 */

void SkGeGetRingParam(struct net_device *dev,
		struct ethtool_ringparam *eth_ring)
{
	DEV_NET		*pNet        = PPRIV;
	SK_AC		*pAC         = pNet->pAC;
	int		Port         = pNet->PortNr;

	if (CHIP_ID_YUKON_2(pAC)) {
		eth_ring->rx_max_pending = NUMBER_OF_RX_LE;
		eth_ring->rx_mini_max_pending = 0;
		eth_ring->rx_jumbo_max_pending = 0;
		eth_ring->tx_max_pending = NUMBER_OF_TX_LE;

		eth_ring->rx_pending = pAC->NrOfRxLe[Port];
		eth_ring->rx_mini_pending = 0;
		eth_ring->rx_jumbo_pending = 0;
		eth_ring->tx_pending = pAC->NrOfTxLe[Port];
	} else {
		eth_ring->rx_max_pending = pAC->RxDescrPerRing;
		eth_ring->rx_mini_max_pending = 0;
		eth_ring->rx_jumbo_max_pending = 0;
		eth_ring->tx_max_pending = pAC->TxDescrPerRing;

		eth_ring->rx_pending = pAC->RxPort[Port].RxdRingFree;
		eth_ring->rx_mini_pending = 0;
		eth_ring->rx_jumbo_pending = 0;
		eth_ring->tx_pending = pAC->TxPort[Port].TxdRingFree;
	}

}

/*****************************************************************************
 *
 *	SkGeSetRingParam - Set ring parameter
 *
 * Description:
 *	Changes the rx/tx ring  parameters  of  the  specified  ethernet
 *	device.
 *
 * Returns:
 *	STATUS
 */

int SkGeSetRingParam(struct net_device *dev,
		struct ethtool_ringparam *eth_ring)
{
	DEV_NET		*pNet        = PPRIV;
	SK_AC		*pAC         = pNet->pAC;
	int		Port         = pNet->PortNr;

	if (CHIP_ID_YUKON_2(pAC)) {
		/* Values check */
		if (eth_ring->rx_pending > NUMBER_OF_RX_LE ||
			eth_ring->rx_pending < MIN_NUMBER_OF_RX_LE ||
			eth_ring->tx_pending > NUMBER_OF_TX_LE ||
			eth_ring->tx_pending < MIN_NUMBER_OF_TX_LE)
			return -EINVAL;

		/* Disable device */
		SkDrvEnterDiagMode(pAC);

		/* Free resources */
		SkY2FreeResources(pAC);
	} else {
	/* Yukon not supported */
		return -EOPNOTSUPP;
	}

	/* Set the new value */
	pAC->NrOfRxLe[Port] = eth_ring->rx_pending;
	pAC->NrOfTxLe[Port] = eth_ring->tx_pending;

	/* Allocate resources */
	if (!SkY2AllocateResources(pAC)) {
		printk("No memory for Yukon2 settings\n");
		return -ENOMEM;
	}

	/* Enable device */
	SkDrvLeaveDiagMode(pAC);

	return 0;
}

/*
 * Eeprom functions
 */

/*****************************************************************************
 *
 *	SkGeGetEepromLen - Check for exceeding total eeprom len
 *
 * Description:
 *	This function checks the total eeprom length
 *
 * Returns:
 *	eeprom size
 */

int SkGeGetEepromLen(struct net_device *dev)
{
	DEV_NET		*pNet        = PPRIV;
	SK_AC		*pAC         = pNet->pAC;
	SK_U16		Word;

	/* get the size */
	SK_IN16(pAC->IoBase, PCI_C2(pAC, pAC->IoBase, PCI_OUR_REG_2), &Word);
	return 1 << ( ((Word & PCI_VPD_ROM_SZ) >> 14) + 8);
}

/*****************************************************************************
 *
 *	SkGeGetEeprom - Dump the EEPROM data
 *
 * Description:
 *	Retrieves  and  prints an EEPROM dump for the specified ethernet
 *	device.  When raw is enabled, then it dumps the raw EEPROM  data
 * 	to  stdout.  The length and offset parameters allow dumping
 *	certain portions of the EEPROM.  Default is to dump the entire
 *	EEPROM.
 *
 * Returns:
 *	status
 */
int SkGeGetEeprom(struct net_device *dev,
		struct ethtool_eeprom *eeprom,
		u8 *data)
{
	DEV_NET		*pNet        = PPRIV;
	SK_AC		*pAC         = pNet->pAC;
	int		Rtv = 0;

	if (!pci_find_capability(pAC->PciDev, PCI_CAP_ID_VPD)) {
	/* Not available */
		return -EAGAIN;
	}

	eeprom->magic = SK98LIN_EEPROM_MAGIC;
	Rtv = VpdReadBlock(pAC, pAC->IoBase, data, eeprom->offset, eeprom->len);
	return 0;
}

/*****************************************************************************
 *
 *	SkGeSetEeprom - Write new EEPROM data
 *
 * Description:
 *	If  value  is  specified,  changes EEPROM byte for the specified
 *	ethernet device.  offset and value specify which byte  and  it's
 *	new  value. If value is not specified, stdin is read and written
 *	to the EEPROM. The length and offset parameters allow writing to
 * 	certain  portions  of  the  EEPROM.   Because  of the persistent
 *	nature of writing to the EEPROM,  a  device-specific  magic  key
 *	must  be specified to prevent the accidental writing to the
 *	EEPROM.
 *
 * Returns:
 *	status
 */
int SkGeSetEeprom(struct net_device *dev,
		struct ethtool_eeprom *eeprom,
		u8 *data)
{
	DEV_NET		*pNet        = PPRIV;
	SK_AC		*pAC         = pNet->pAC;
	int		Rtv = 0;

	if (!pci_find_capability(pAC->PciDev, PCI_CAP_ID_VPD)) {
	/* Not available */
		return -EAGAIN;
	}

	/* Check the device magic */
	if (eeprom->magic != SK98LIN_EEPROM_MAGIC) {
		return -EINVAL;
	}

	/* Partial writes not supported */
	if ((eeprom->offset & 3) || (eeprom->len & 3)) {
		return -EINVAL;
	}

	Rtv = VpdWriteBlock(pAC, pAC->IoBase, data, eeprom->offset, eeprom->len);
	return 0;
}

/*
 * Register functions
 */
static int SkGeRegAccessOk(SK_AC *pAC, unsigned int b)
{
	/* This complicated switch statement is to make sure and
	 * only access regions that are unreserved.
	 * Some blocks are only valid on dual port cards.
	 *
	 * Based on an idea from Stephen Hemminger from the sky2
	 * driver
	 */
	switch (b) {
	/* second port */
	case 5:		/* Tx Arbiter 2 */
	case 9:		/* RX2 */
	case 14 ... 15:	/* TX2 */
	case 17: case 19: /* Ram Buffer 2 */
	case 22 ... 23: /* Tx Ram Buffer 2 */
	case 25:	/* Rx MAC Fifo 1 */
	case 27:	/* Tx MAC Fifo 2 */
	case 31:	/* GPHY 2 */
	case 40 ... 47: /* Pattern Ram 2 */
	case 52: case 54: /* TCP Segmentation 2 */
	case 112 ... 116: /* GMAC 2 */
		return pAC->MaxPorts > 1;

	case 0:		/* Control */
	case 2:		/* Mac address */
	case 4:		/* Tx Arbiter 1 */
	case 7:		/* PCI express reg */
	case 8:		/* RX1 */
	case 12 ... 13: /* TX1 */
	case 16: case 18:/* Rx Ram Buffer 1 */
	case 20 ... 21: /* Tx Ram Buffer 1 */
	case 24:	/* Rx MAC Fifo 1 */
	case 26:	/* Tx MAC Fifo 1 */
	case 28 ... 29: /* Descriptor and status unit */
	case 30:	/* GPHY 1*/
	case 32 ... 39: /* Pattern Ram 1 */
	case 48: case 50: /* TCP Segmentation 1 */
	case 56 ... 60:	/* PCI space */
	case 80 ... 84:	/* GMAC 1 */
		return 1;

	default:
		return 0;
	}
}


/*****************************************************************************
 *
 *	SkGeGetRegsLen - Get the register size
 *
 * Description:
 *	This function checks the total register length
 *
 * Returns:
 *	register size
 */

int SkGeGetRegsLen(struct net_device *dev)
{

	/* ethtool_get_regs always provides full size (16k) buffer */
	return 0x4000;
}

/*****************************************************************************
 *
 *	SkGeGetRegs - Returns copy of control register region
 *
 * Description:
 *	Retrieves  and prints a register dump for the specified ethernet device.
 *	The register format for some devices is known and decoded others are
 *	printed in hex.  When raw is  enabled,  then ethtool  dumps  the  raw
 *	register data to stdout.  If file is specified, then use contents of
 *	previous raw register dump, rather than reading from the device.
 *
 * Returns:
 *	status
 */
void SkGeGetRegs(struct net_device *dev,
		struct ethtool_regs *regs,
		void *data)
{
	DEV_NET		*pNet        = PPRIV;
	SK_AC		*pAC         = pNet->pAC;
	SK_U32		key;

	const void __iomem *pIoMem = pAC->IoBase;
	regs->version = 1;

	for (key = 0; key < 128; key++) {
		if (key == 3) {
		/* skip diagnostic ram region in block 3 */
			memcpy_fromio(data + 0x10, pIoMem + 0x10, 128 - 0x10);
		} else if (SkGeRegAccessOk(pAC, key)) {
			memcpy_fromio(data, pIoMem, 128);
		} else {
			memset(data, 0, 128);
		}

		data += 128;
		pIoMem += 128;
	}
}


/*
 * Simple diag tests
 */

#ifdef SK98LIN_DIAG
/*****************************************************************************
 *
 * 	SkGeDiagTestCnt - initialize the test counter
 *
 * Description:
 *	This function initializes the adapter self test counter. Tests for
 *	the adapter are stored in the structure SkGeGstringsTest
 *
 * Returns:
 *	TEST_LENGHT
 */
int SkGeDiagTestCnt(struct net_device *netdev)
{
	return SK98LIN_TEST_LEN;
}

/*****************************************************************************
 *
 * 	SkGeGetDiagStrings - Generate the strings
 *
 * Description:
 *	This function initializes the adapter self test strings structure. Names for
 *	the adapter tests are stored in the structure SkGeGstringsTest
 *
 * Returns:	N/A
 *
 */
static void SkGeGetDiagStrings(u8 *data)
{
	memcpy(data, *SkGeGstringsTest,
		sizeof(SkGeGstringsTest));

	return;
}

#ifdef SK98LIN_DIAG_LOOPBACK
/*****************************************************************************
 *
 * 	SkGeVerifyLoopbackData - Verify the loopback data
 *
 * Description:
 *	Verify the loopback diag test data
 * Returns:
 *	return value
 */
void SkGeVerifyLoopbackData(
SK_AC		*pAC,
struct sk_buff	*pMsg)
{
	SK_U8	*sendData = NULL;
	SK_U8	*recvData = NULL;
	int	sendLength  = 0;
	int	recvLength = 0;
	int	index;

	pAC->YukonLoopbackStatus = GBE_LOOPBACK_VERIFYING;	/* verifying... */

	/* Data buffer received */
	recvData = (SK_U8*)pMsg->data;
	recvLength = pMsg->len;

	/* Data buffer sent */
	sendData = (SK_U8*)pAC->pYukonLoopbackSendMsg->data;
	sendLength = pAC->pYukonLoopbackSendMsg->len;

	if (recvLength != sendLength) {
		printk("%s: verify error: length received:%d expected:%d\n",
			DRV_NAME, recvLength, sendLength);
		pAC->YukonLoopbackStatus = GBE_LOOPBACK_ERROR;	/* error */
		return;
	}

	for (index = 0; index < recvLength; index++) {
		if (recvData[index] != sendData[index]) {
			printk("%s: verify error at offset: %d received: 0x%02x expected: 0x%02x\n",
				DRV_NAME, index, recvData[index], sendData[index]);
			pAC->YukonLoopbackStatus = GBE_LOOPBACK_ERROR;	/* error */
			return;
		}
	}

	pAC->YukonLoopbackStatus = GBE_LOOPBACK_OK;	/* success */

	return;
}


/*****************************************************************************
 *
 * 	SkGeCreateLoopbackData - Create the loopback data
 *
 * Description:
 *	Create the loopback diag test data
 * Returns:
 *	return value
 */
static int SkGeCreateLoopbackData(
SK_AC		*pAC,
SK_U8		*data,
int		length)
{
	SK_U32 i;
	SK_U32 headerLen = (GBE_LOOPBACK_ADDRESS_SIZE + GBE_LOOPBACK_ADDRESS_SIZE + 2);

	/* Destination address field and source address field */
	memcpy((SK_I8*)&data[0], (SK_I8*)&pAC->Addr.Net[0].CurrentMacAddress.a[0], GBE_LOOPBACK_ADDRESS_SIZE);
	memcpy((SK_I8*)&data[6], (SK_I8*)&pAC->Addr.Net[0].CurrentMacAddress.a[0], GBE_LOOPBACK_ADDRESS_SIZE);

	/* Security check */
	if (length < GBE_LOOPBACK_MIN_EP_SIZE)
		length = GBE_LOOPBACK_MIN_EP_SIZE;
	if (length > GBE_LOOPBACK_MAX_EP_SIZE)
		length = GBE_LOOPBACK_MAX_EP_SIZE;

	*(SK_U16*)(&data[12]) = length - headerLen;


	for(i = 0; (headerLen+i) < length; i++) {
		data[headerLen+i] = (u8) (i & 0xff);
	}

	return ( length );
}

/*****************************************************************************
 *
 * 	SkGeExecLoopbackTest - Execute the loopback diag test
 *
 * Description:
 *	Execute the loopback test
 * Returns:
 *	return value
 */
static int SkGeExecLoopbackTest(SK_AC *pAC, struct net_device *dev)
{
	SK_U32			rc           = 1; /* Error */
	SK_U8			*data        = NULL;
	SK_I32			length       = 0;
	SK_U32			pattern      = 0;
	struct sk_buff		*pMsg        = NULL;
	SK_U32			StartTime;
	SK_U32			Delta;
	SK_U32			TimeOut;


	data = (SK_U8*)kmalloc(GBE_LOOPBACK_MAX_EP_SIZE - GBE_LOOPBACK_ADDRESS_SIZE, GFP_KERNEL); /* 1514 */
	if (data == NULL) {
		printk("%s: error allocating memory\n", DRV_NAME);
		return rc;
	}

	/* Send loopback packets to the card */
	for (pattern = GBE_LOOPBACK_MIN_EP_SIZE;
		pattern <= GBE_LOOPBACK_MAX_EP_SIZE; pattern += 2) {

		/* Alloc the packet memory */
		pMsg = alloc_skb(GBE_LOOPBACK_MAX_EP_SIZE, GFP_ATOMIC);
		if (pMsg == NULL) {
			printk("%s: not enough memory for pMsg\n", DRV_NAME);
			break;
		}

		memset(data, 0, GBE_LOOPBACK_MAX_EP_SIZE);
		length = SkGeCreateLoopbackData(pAC, data, pattern);

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,22)
		skb_copy_to_linear_data(pMsg, data,
			length);
#else
		eth_copy_and_sum(pMsg, (char *) data,
			length, 0);
#endif
		skb_put(pMsg, length);

		/* Send test data */
		pAC->YukonLoopbackStatus = GBE_LOOPBACK_START;
		pAC->pYukonLoopbackSendMsg = pMsg;

#ifdef DEBUG
		DumpMsg(pMsg, "SkGeExecLoopbackTest");
#endif

		/* Send the test data */
		SkY2Xmit(pMsg, dev);

		/* get current value of timestamp timer */
		StartTime = SkHwGetTimeDelta(pAC, pAC->IoBase, 0);

		/* set timeout */
		TimeOut = HW_MS_TO_TICKS(pAC, GBE_LOOPBACK_TIMEOUT * 1000);

		while ((pAC->YukonLoopbackStatus == GBE_LOOPBACK_START) ||
			(pAC->YukonLoopbackStatus == GBE_LOOPBACK_VERIFYING)) {
			msleep(10);

			/* get delta value of timestamp timer */
			Delta = SkHwGetTimeDelta(pAC, pAC->IoBase, StartTime);

			/* timeout */
			if (Delta > TimeOut) {
				pAC->YukonLoopbackStatus = GBE_LOOPBACK_ERROR_TIMEOUT;
				break;
			}
		}
	}

	/* Free the data */
	kfree(data);

	/* Check the result */
	if (pAC->YukonLoopbackStatus == GBE_LOOPBACK_ERROR) {
		printk("%s: loopback test failed\n", DRV_NAME);
	} else if (pAC->YukonLoopbackStatus == GBE_LOOPBACK_ERROR_TIMEOUT) {
		printk("%s: loopback test timeout\n", DRV_NAME);
	} else {
		/* OK */
		rc = 0;
	}

	return rc;
}

/*****************************************************************************
 *
 * 	SkGeLoopbackTest - Loopback diag test
 *
 * Description:
 *	Start loopback test
 * Returns:
 *	IRQ_HANDLED if everything is ok
 *      IRQ_NONE on error
 */
static int SkGeLoopbackTest(SK_AC *pAC, struct net_device *dev, u64 *data)
{
	DEV_NET			*pNet        = PPRIV;
	int			Port         = pNet->PortNr;
	SK_U32			rc           = 0;
	SK_U16			SWord;
	SK_EVPARA		Para;
	SK_U16			*OutAddr;

	*data = 0;

	/* Open and initialize the hardware */
	SkGeOpen(dev);
	SkDrvEvent(pAC, pAC->IoBase, SK_DRV_LINK_DOWN, Para);


	SkMacRxTxDisable(pAC, pAC->IoBase, Port);

	/* put mac into loopback mode */
	SkMacHardRst(pAC, pAC->IoBase, Port);
	SkMacClearRst(pAC, pAC->IoBase, Port);

	/* setup General Purpose Control Register */
	GM_OUT16(pAC->IoBase, Port, GM_GP_CTRL,
		GM_GPCR_SPEED_1000 | GM_GPCR_DUP_FULL |
		GM_GPCR_AU_FCT_DIS | GM_GPCR_AU_ALL_DIS);

	/* dummy read the Interrupt Source Register */
	SK_IN16(pAC->IoBase, MR_ADDR(Port, GMAC_IRQ_SRC), &SWord);

	pAC->GIni.GP[Port].PLinkSpeedCap =
				(SK_U8)(SK_LSPEED_CAP_1000MBPS |
				SK_LSPEED_CAP_100MBPS | SK_LSPEED_CAP_10MBPS |
				SK_LSPEED_CAP_AUTO);

	SkMacInitPhy(pAC, pAC->IoBase, Port, SK_FALSE);

	SkGmResetCounter(pAC, pAC->IoBase, Port);

	/* setup Transmit Control Register */
	GM_OUT16(pAC->IoBase, Port, GM_TX_CTRL, (SK_U16)TX_COL_THR(pAC->GIni.GP[Port].PMacColThres));

	/* setup Receive Control Register */
	GM_OUT16(pAC->IoBase, Port, GM_RX_CTRL,
		GM_RXCR_UCF_ENA | GM_RXCR_MCF_ENA | GM_RXCR_CRC_DIS);

	/* setup Transmit Flow Control Register */
	GM_OUT16(pAC->IoBase, Port, GM_TX_FLOW_CTRL, 0xffff);

	/* setup Transmit Parameter Register */
	SWord = (SK_U16)(TX_JAM_LEN_VAL(pAC->GIni.GP[Port].PMacJamLen) |
		TX_JAM_IPG_VAL(pAC->GIni.GP[Port].PMacJamIpgVal) |
		TX_IPG_JAM_DATA(pAC->GIni.GP[Port].PMacJamIpgData) |
		TX_BACK_OFF_LIM(pAC->GIni.GP[Port].PMacBackOffLim));
	GM_OUT16(pAC->IoBase, Port, GM_TX_PARAM, SWord);

	/* configure the Serial Mode Register */
	SWord = (SK_U16)(DATA_BLIND_VAL(pAC->GIni.GP[Port].PMacDataBlind) |
		GM_SMOD_VLAN_ENA |
		IPG_DATA_VAL((pAC->GIni.GP[Port].PLinkSpeedUsed == SK_LSPEED_STAT_1000MBPS) ?
		pAC->GIni.GP[Port].PMacIpgData_1G : pAC->GIni.GP[Port].PMacIpgData_FE));

	if (pAC->GIni.GP[Port].PMacLimit4) {
		/* reset of collision counter after 4 consecutive collisions */
		SWord |= GM_SMOD_LIMIT_4;
	}

	if (pAC->GIni.GP[Port].PPortUsage == SK_JUMBO_LINK) {
		/* enable jumbo mode (Max. Frame Length = 9018) */
		SWord |= GM_SMOD_JUMBO_ENA;
	}

	if (HW_FEATURE(pAC, HWF_NEW_FLOW_CONTROL)) {
		/* enable new Flow-Control */
		SWord |= GM_NEW_FLOW_CTRL;
	}

	GM_OUT16(pAC->IoBase, Port, GM_SERIAL_MODE, SWord);

	GM_OUT16(pAC->IoBase, Port, GM_TX_IRQ_MSK, 0);
	GM_OUT16(pAC->IoBase, Port, GM_RX_IRQ_MSK, 0);
	GM_OUT16(pAC->IoBase, Port, GM_TR_IRQ_MSK, 0);

	/* put phy into loopback mode */

	/* disable auto-negotiation */
	pAC->GIni.GP[Port].PLinkMode = SK_LMODE_FULL;

	/* set Force Link Pass for loopback */
	GM_IN16(pAC->IoBase, Port, GM_GP_CTRL, &SWord);
	SWord |= GM_GPCR_FL_PASS | GM_GPCR_AU_ALL_DIS;
	GM_OUT16(pAC->IoBase, Port, GM_GP_CTRL, SWord);

	/* Set port's current physical MAC address. */
	OutAddr = (SK_U16 *) &pAC->Addr.Net[0].CurrentMacAddress.a[0];
	GM_OUTADDR(pAC->IoBase, Port, GM_SRC_ADDR_1L, OutAddr);


	/* GMAC Rx/Tx enable */
	SkMacSetRxTxEn(pAC, pAC->IoBase, Port,
		SK_MAC_LOOPB_OFF | SK_PHY_FULLD_ON);
	SkMacInitPhy(pAC, pAC->IoBase, Port, SK_TRUE);

	if ((pAC->GIni.GIChipId == CHIP_ID_YUKON_FE) &&
		(pAC->GIni.GP[Port].PLinkSpeed == SK_LSPEED_STAT_100MBPS)) {
		/* Disable link in PHY loopback mode using special register */
		SkGmPhyWrite(pAC, pAC->IoBase, Port, PHY_MARV_PAGE_ADDR, 2);
		SkGmPhyWrite(pAC, pAC->IoBase, Port, PHY_MARV_PAGE_DATA, 0x876F);
	}

	/* Transmit test data, return result of loopback test */
	rc = SkGeExecLoopbackTest(pAC, dev);

#ifdef DEBUG
	if (rc) {
	/* Test failed */
		printk("%s:     test failed\n", SK_DRV_NAME);
	} else {
		printk("%s:     test ok\n", SK_DRV_NAME);
	}
#endif

	/* Close the device and return */
	SkGeClose(dev);

	pAC->LoopbackRunning = SK_FALSE;

	return rc;
}
#endif

/*****************************************************************************
 *
 * 	SkGeIntrTest - SW interrupt test
 *
 * Description:
 *	Handle software interrupt used during MSI test
 * Returns:
 *	IRQ_HANDLED if everything is ok
 *      IRQ_NONE on error
 */
static int SkGeIntrTest(SK_AC *pAC, struct net_device *dev, u64 *data)
{
	SK_U32		AllocFlagSave;
	AllocFlagSave = pAC->AllocFlag;
	pAC->AllocFlag = 0;
	*data = 0;

	if ((pci_enable_msi(pAC->PciDev) == 0) && (CHIP_ID_YUKON_2(pAC))) {
	/* MSI test for Yukon2 */
		*data = SkGeTestInt(dev, pAC, 0);
		pci_disable_msi(pAC->PciDev);
		pAC->AllocFlag = 0;
		if (!*data) {
			pAC->AllocFlag = AllocFlagSave;
			return *data;
		}
	}

	/* INTx */
	*data = SkGeTestInt(dev, pAC, SK_IRQ_SHARED);

	pAC->AllocFlag = AllocFlagSave;
	return *data;
}


/*****************************************************************************
 *
 * 	SkGeEEPromTest - EEPROM read test
 *
 * Description:
 *  Brief Read RAW data from EEPROM.
 *  This function is used for the eeprom test.
 * Returns:
 *	0 == if everything is ok
 *     0 != on error
 */
static int SkGeEEPromTest(SK_AC *pAC, struct net_device *dev, u64 *data)
{
	char 		*pData = NULL;
	SK_U32		eeprom_len, read_len;
#if 0
	char		Key[] = VPD_NAME;
	int		StrLen = 80;
	char		Str[80];
#endif
	*data = 0;	// Passed

	eeprom_len = SkGeGetEepromLen(dev);
	if (!eeprom_len) {
		*data = 1;
		return *data;
	}

	pData = kmalloc(eeprom_len, GFP_KERNEL);

	/* Read the block */
	read_len = VpdReadBlock(pAC, pAC->IoBase, pData, 0, eeprom_len);
	if (!read_len) {
		printk("%s: EEprom read block failed!\n", SK_DRV_NAME);
		*data = 1;
		goto out;
	}

#if 0
	/* VPD Value check */
	Ret = VpdRead(pAC, pAC->IoBase, Key, Str, &StrLen);
	if (Ret) {
		printk("%s: EEprom value read failed! Ret: 0x%x\n", SK_DRV_NAME, Ret);
		goto out;
	}
#endif

out:
	kfree(pData);
	return *data;
}

/*****************************************************************************
 *
 * 	SkGeRegTest - Register read test
 *
 * Returns:
 *	0 == if everything is ok
 *     0 != on error
 */
static int SkGeRegTest(SK_AC *pAC, struct net_device *dev, u64 *data)
{
	char 		*pData = NULL;
	SK_U32		key;
	const void 	__iomem *pIoMem = pAC->IoBase;

	/* Set tdefault */
	*data = 0;
	return *data;

	/* Read */
	pData = kmalloc(256, GFP_KERNEL);
	for (key = 0; key < 128; key++) {
		if (key == 3) {
			// Don't read
			// memcpy_fromio(pData, pIoMem + 0x10, 128 - 0x10);
		} else if (SkGeRegAccessOk(pAC, key)) {
			memcpy_fromio(pData, pIoMem, 128);
		} else {
			memset(pData, 0, 128);
		}

		pIoMem += 128;
	}

	kfree(pData);
	return *data;
}

/*****************************************************************************
 *
 * 	SkGeDiagTest - self test
 *
 * Description:
 *	This function is used if the user want to start a self hardware test.
 *
 * Returns:	N/A
 *
 */

void SkGeDiagTest(struct net_device *dev,
                            struct ethtool_test *ethtest, u64 *data)
{
	DEV_NET		*pNet        = PPRIV;
	SK_AC		*pAC         = pNet->pAC;

	/* error by default */
	data[0] = -1;		// Register test
	data[1] = -1;		// Eeprom test
	data[2] = -1;		// Interrupt test
	data[3] = -1;		// Loopback test

	/* Check the state */
	if ((netif_running(dev)) || (pAC->MaxPorts > 0)) {
	/* Link state up */
		/* Online tests */
		printk("Note: ONLINE test is NOT supported\n");
		ethtest->flags |= ETH_TEST_FL_FAILED;
		return;
	}

	if (ethtest->flags == ETH_TEST_FL_OFFLINE) {
		/* Offline tests */
		if (SkGeRegTest(pAC, dev, &data[0]))
			ethtest->flags |= ETH_TEST_FL_FAILED;

		if (SkGeEEPromTest(pAC, dev, &data[1]))
			ethtest->flags |= ETH_TEST_FL_FAILED;

		if (SkGeIntrTest(pAC, dev, &data[2]))
			ethtest->flags |= ETH_TEST_FL_FAILED;
#ifdef SK98LIN_DIAG_LOOPBACK
		if (SkGeLoopbackTest(pAC, dev, &data[3]))
			ethtest->flags |= ETH_TEST_FL_FAILED;
#endif
	}

}

#endif

/*******************************************************************************
 *
 * End of file
 *
 ******************************************************************************/
