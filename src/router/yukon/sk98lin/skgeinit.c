/******************************************************************************
 *
 * Name:	$Id: //Release/Yukon_1G/Shared/common/V6/skgeinit.c#18 $
 * Project:	Gigabit Ethernet Adapters, Common Modules
 * Version:	$Revision: #18 $, $Change: 9172 $
 * Date:	$DateTime: 2012/03/01 18:14:31 $
 * Purpose:	Contains functions to initialize the adapter
 *
 ******************************************************************************/

/******************************************************************************
 *
 *	LICENSE:
 *	(C)Copyright Marvell.
 *	
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *	
 *	The information in this file is provided "AS IS" without warranty.
 *	/LICENSE
 *
 ******************************************************************************/

#include "h/skdrv1st.h"
#include "h/skdrv2nd.h"

/* global variables ***********************************************************/

/* local variables ************************************************************/

struct s_QOffTab {
	int	RxQOff;				/* Receive Queue Address Offset */
	int	XsQOff;				/* Sync Tx Queue Address Offset */
	int	XaQOff;				/* Async Tx Queue Address Offset */
#ifdef SK_AVB
	int AvbQOff[AVB_LE_Q_CNT];		/* AVB Tx Queues Addresses Offsets */
	int AvbMacQOff[AVB_MAC_Q_CNT];	/* AVB MAC Tx Queues Addresses Offsets */
	int AvbClsQOff[AVB_CLS_CNT];	/* AVB Classes Offsets */
#endif /* SK_AVB */
};

static struct s_QOffTab QOffTab[] = {
	{
		Q_R1, Q_XS1, Q_XA1,
#ifdef SK_AVB
		{
			Q_AVB_TX_1, Q_AVB_TX_2, Q_AVB_TX_3, Q_AVB_TX_4,
			Q_AVB_TX_5, Q_AVB_TX_6, Q_AVB_TX_7, Q_AVB_TX_8
		},
		{
			TXMF_TCTL, TXMF_TCTL2, TXMF_TCTL3, TXMF_TCTL4, TXMF_TCTL5
		},
		{
			CLASS_A_RATE_INT, CLASS_B_RATE_INT
		}
#endif /* SK_AVB */
	},
	{
		Q_R2, Q_XS2, Q_XA2,
#ifdef SK_AVB
		{
			Q_AVB_TX_1, Q_AVB_TX_2, Q_AVB_TX_3, Q_AVB_TX_4,
			Q_AVB_TX_5, Q_AVB_TX_6, Q_AVB_TX_7, Q_AVB_TX_8
		},
		{
			TXMF_TCTL, TXMF_TCTL2, TXMF_TCTL3, TXMF_TCTL4, TXMF_TCTL5
		},
		{
			CLASS_A_RATE_INT, CLASS_B_RATE_INT
		}
#endif /* SK_AVB */
	}
};

struct s_Config {
	char	ScanString[8];
	SK_U32	Value;
};

static struct s_Config OemConfig = {
	{'O','E','M','_','C','o','n','f'},
#ifdef SK_OEM_CONFIG
	OEM_CONFIG_VALUE,
#else
	0,
#endif
};

/******************************************************************************
 *
 *	SkHwGetTimeDelta() - Get time delta from timestamp timer
 *
 * Description:
 *	Get time delta relativ to parameter 'Start'
 *
 * Returns:
 *	nothing
 */
SK_U32 SkHwGetTimeDelta(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	IoC,	/* I/O Context */
SK_U32	Start)	/* Start time */
{
	SK_U8	Byte;
	SK_U32	Delta;

	/* check timestamp timer */
	SK_IN8(IoC, GMAC_TI_ST_CTRL, &Byte);

	if (Byte == 0 || Byte == 0xff) {

		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
			("Timestamp Timer not accessible\n"));

		return(ALL_32_BITS);
	}

	if ((Byte & GMT_ST_START) == 0) {

		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
			("Timestamp Timer not running\n"));

		return(ALL_32_BITS);
	}

	/* get current value of timestamp timer */
	SK_IN32(IoC, GMAC_TI_ST_VAL, &Delta);

	if (Delta >= Start) {
		Delta -= Start;
	}
	else {
		Delta += (~Start + 1);
	}

	return(Delta);

}	/* SkHwGetTimeDelta */


#ifndef SK_SLIM
/******************************************************************************
 *
 *	SkGePortVlan() - Enable / Disable VLAN support
 *
 * Description:
 *	Enable or disable the VLAN support of the selected port.
 *	The new configuration is *not* saved over any SkGeStopPort() and
 *	SkGeInitPort() calls.
 *	Currently this function is only supported on Yukon-2/EC adapters.
 *
 * Returns:
 *	nothing
 */
void SkGePortVlan(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	IoC,	/* I/O Context */
int		Port,	/* Port number */
SK_BOOL	Enable)	/* Flag */
{
	SK_U32	RxCtrl;
	SK_U32	TxCtrl;

	if (CHIP_ID_YUKON_2(pAC)) {
		if (Enable) {
			RxCtrl = RX_VLAN_STRIP_ON;
			TxCtrl = TX_VLAN_TAG_ON;
		}
		else {
			RxCtrl = RX_VLAN_STRIP_OFF;
			TxCtrl = TX_VLAN_TAG_OFF;
		}

		SK_OUT32(IoC, MR_ADDR(Port, RX_GMF_CTRL_T), RxCtrl);

		SK_OUT32(IoC, MR_ADDR(Port, TX_GMF_CTRL_T), TxCtrl);
	}
}	/* SkGePortVlan */


/******************************************************************************
 *
 *	SkGeMacSecLE() - Enable / Disable MACsec LE (Yukon-Ext.)
 *
 * Description:
 *	Enable or disable generation of MACsec status list elements.
 *
 * Returns:
 *	nothing
 */
void SkGeMacSecLE(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	IoC,	/* I/O Context */
int		Port,	/* Port number */
SK_BOOL	Enable)	/* Flag */
{
	if (HW_SUP_MACSEC(pAC)) {
		SK_OUT32(IoC, Q_ADDR(pAC->GIni.GP[Port].PRxQOff, Q_CSR),
			Enable ? RBMU_CSR_MACSEC_LE_ENA : RBMU_CSR_MACSEC_LE_DIS);
	}
}	/* SkGeMacSecLE */


/******************************************************************************
 *
 *	SkGeRxRss() - Enable / Disable RSS Hash Calculation
 *
 * Description:
 *	Enable or disable the RSS hash calculation of the selected port.
 *	The new configuration is *not* saved over any SkGeStopPort() and
 *	SkGeInitPort() calls.
 *	Currently this function is only supported on Yukon-2/EC adapters.
 *
 * Returns:
 *	nothing
 */
void SkGeRxRss(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	IoC,	/* I/O Context */
int		Port,	/* Port number */
SK_BOOL	Enable)	/* Flag */
{
	if (CHIP_ID_YUKON_2(pAC)) {
		SK_OUT32(IoC, Q_ADDR(pAC->GIni.GP[Port].PRxQOff, Q_CSR),
			Enable ? BMU_ENA_RX_RSS_HASH : BMU_DIS_RX_RSS_HASH);
	}
}	/* SkGeRxRss */


/******************************************************************************
 *
 *	SkGeRxCsum() - Enable / Disable Receive Checksum
 *
 * Description:
 *	Enable or disable the Rx TCP/IP Checksum Check of the selected port.
 *	The new configuration is *not* saved over any SkGeStopPort() and
 *	SkGeInitPort() calls.
 *	Currently this function is only supported on Yukon-2/EC adapters.
 *
 * Returns:
 *	nothing
 */
void SkGeRxCsum(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	IoC,	/* I/O Context */
int		Port,	/* Port number */
SK_BOOL	Enable)	/* Flag */
{
	if (CHIP_ID_YUKON_2(pAC)) {
		SK_OUT32(IoC, Q_ADDR(pAC->GIni.GP[Port].PRxQOff, Q_CSR),
			Enable ? BMU_ENA_RX_CHKSUM : BMU_DIS_RX_CHKSUM);
	}
}	/* SkGeRxCsum */
#endif /* !SK_SLIM */

#ifndef DISABLE_YUKON_I
/******************************************************************************
 *
 *	SkGePollRxD() - Enable / Disable Descriptor Polling of RxD Ring
 *
 * Description:
 *	Enable or disable the descriptor polling of the receive descriptor
 *	ring (RxD) for port 'Port'.
 *	The new configuration is *not* saved over any SkGeStopPort() and
 *	SkGeInitPort() calls.
 *
 * Returns:
 *	nothing
 */
void SkGePollRxD(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port,		/* Port Index (MAC_1 + n) */
SK_BOOL PollRxD)	/* SK_TRUE (enable pol.), SK_FALSE (disable pol.) */
{
	SK_GEPORT	*pPrt;

	pPrt = &pAC->GIni.GP[Port];

	SK_OUT32(IoC, Q_ADDR(pPrt->PRxQOff, Q_CSR), (SK_U32)((PollRxD) ?
		CSR_ENA_POL : CSR_DIS_POL));
}	/* SkGePollRxD */


/******************************************************************************
 *
 *	SkGePollTxD() - Enable / Disable Descriptor Polling of TxD Rings
 *
 * Description:
 *	Enable or disable the descriptor polling of the transmit descriptor
 *	ring(s) (TxD) for port 'Port'.
 *	The new configuration is *not* saved over any SkGeStopPort() and
 *	SkGeInitPort() calls.
 *
 * Returns:
 *	nothing
 */
void SkGePollTxD(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port,		/* Port Index (MAC_1 + n) */
SK_BOOL PollTxD)	/* SK_TRUE (enable pol.), SK_FALSE (disable pol.) */
{
	SK_GEPORT	*pPrt;
	SK_U32		DWord;

	pPrt = &pAC->GIni.GP[Port];

	DWord = (SK_U32)(PollTxD ? CSR_ENA_POL : CSR_DIS_POL);

	if (pPrt->PXSQSize != 0) {
		SK_OUT32(IoC, Q_ADDR(pPrt->PXsQOff, Q_CSR), DWord);
	}

	if (pPrt->PXAQSize != 0) {
		SK_OUT32(IoC, Q_ADDR(pPrt->PXaQOff, Q_CSR), DWord);
	}
}	/* SkGePollTxD */
#endif /* !DISABLE_YUKON_I */


#ifdef SK_AVB
/******************************************************************************
 *
 *  SkYuk2AVBControl() - Control AVB feature of Yukon-Optima
 *
 * Description:
 *  This function enables or disables the AVB feature of Yukon-Optima.
 *
 * return:
 *  nothing
 */
void SkYuk2AVBControl(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port,		/* Port Index 0 or 1 */
SK_BOOL	Enable)		/* Enable / disable AVB feature */
{
	SK_GEPORT	*pPrt;
	int			q;

	if (!HW_SUP_AVB(pAC)) {
		return;
	}

	pPrt = &pAC->GIni.GP[Port];

	/* Disable all prefetch units first */
	for (q = 0; q < AVB_LE_Q_CNT; q++) {
		SK_OUT8(IoC, Y2_PREF_Q_ADDR(pPrt->PTxQOffs[q], PREF_UNIT_CTRL_REG),
			(SK_U8)PREF_UNIT_RST_SET);
	}

	for (q = 0; q < AVB_MAC_Q_CNT; q++) {
		SK_OUT8(IoC, Y2_AVB_MAC_Q_ADDR(pPrt->PAvbMacQOffs[q], TXMF_TCTL),
			(SK_U8)TXMF_TCTL_RST_SET);
	}

	SK_OUT8(IoC, GMAC_LINK_CTRL, Enable ?
		MAC_CTRL_AVB_MODE_ON : MAC_CTRL_AVB_MODE_OFF);

	if (Enable) {
		/* Start the MAC Queues */
		for (q = 0; q < AVB_MAC_Q_CNT; q++) {
			SK_OUT8(IoC, Y2_AVB_MAC_Q_ADDR(pPrt->PAvbMacQOffs[q], TXMF_TCTL),
				(SK_U8)(TXMF_TCTL_RST_CLR | TXMF_TCTL_OP_ON));

			RegisterSelectiveSetReset(pAC, IoC,
				Y2_AVB_MAC_Q_ADDR(pAC->GIni.GP[Port].PAvbMacQOffs[q], TX1_RATE_MISC),
				TX_RATE_MISC_RATE_SHAPE_DIS, 0);
		}

		SK_OUT32(IoC, Y2_AVB_CLS_ADDR(CLASS_A_RATE_INT, CLASS_A_RATE_INT),
			TX_RATE_INT_K_FACTOR_EN);
		SK_OUT32(IoC, Y2_AVB_CLS_ADDR(CLASS_B_RATE_INT, CLASS_A_RATE_INT),
			TX_RATE_INT_K_FACTOR_EN);
	}
}

/******************************************************************************
 *
 *  SkYuk2AVBModeEnabled() - Get AVB feature state of Yukon-Optima
 *
 * Description:
 *  This function returns the state of the AVB feature of Yukon-Optima (ena/dis).
 *
 * returns:
 *  SK_FALSE - AVB Mode is disabled
 *  SK_TRUE  - AVB Mode is enabled
 */
SK_BOOL SkYuk2AVBModeEnabled(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port)		/* Port Index 0 or 1 */
{
	SK_U8	Byte;

	if (!HW_SUP_AVB(pAC)) {
		return(SK_FALSE);
	}

	/* Since the bit definitions for AVB Mode and MACSEC Tx bypass
	 * are overlapping in a chip containing both of the features the
	 * AVB bits will certainly reside on a different location so this
	 * function must be updated accordingly and the driver should always
	 * use this function to get the AVB bits.
	*/
	if (pAC->GIni.GIChipId >= CHIP_ID_YUKON_OPT &&
		pAC->GIni.GIChipId <= CHIP_ID_YUKON_OP_2) {

		SK_IN8(IoC, GMAC_LINK_CTRL, &Byte);

		return((Byte & MAC_CTRL_AVB_MODE_ON) != 0);
	}

	return(SK_FALSE);
}

/******************************************************************************
 *
 *  SkYuk2SetQueueDataRate() - Set data rate of the specified MAC FIFO
 *
 * Description:
 *  This function sets the queue registers to guarantee the requested
 *  data rate.
 *
 * returns:
 *  nothing
 */
void SkYuk2SetQueueDataRate(
SK_AC	*pAC,			/* Adapter Context					*/
SK_IOC	IoC,			/* I/O Context						*/
int		Port,			/* Port Index 0 or 1				*/
int		Queue,			/* MAC FIFO queue to setup (0-3)	*/
SK_U32	DataRate,		/* Data Rate in Kbps(pow of 10 or 2)*/
SK_U32	pow)			/* power of DataRate 2 or 10		*/
{
	SK_U32	DWord;

	if (SkYuk2AVBModeEnabled(pAC, IoC, Port) &&
		Queue >= 0 && Queue < AVB_MAC_Q_CNT) {

		SK_IN32(IoC, Y2_AVB_MAC_Q_ADDR(pAC->GIni.GP[Port].PAvbMacQOffs[Queue],
			TX1_RATE_INT), &DWord);

		/* min value if datarate is too low to ensure PTP is still working */
		if (DataRate < 100) {
			DataRate = 100;
		}

		if (pow == 10) {	/* DataRate is power of 10 -> convert */
			/*
			 *	2^22 / 10^6 = 4.194304
			 ****
			 *	DataRate * 2^22 / 10^6 ==
			 *	DataRate * 2^22 / (2^6 * 5^6) ==
			 *	DataRate * 2^16 / 5^6 ==
			 *	DataRate * (62500 / 5^6 + 3036 / 5^6) ==
			 *	DataRate * (4 + 3036 / 5^6) ==
			 *	DataRate * 4 + DataRate * 3036 / 5^6 ==
			 ****
			 *	DataRate * 4 + DataRate * (3000 + 36) / 5^6 ==
			 *	DataRate * 4 + DataRate * 24 / 5^3 + DataRate * 36 / 5^6 ==
			 *	DataRate * 4 + DataRate * 24 / 5^3 + (DataRate * 24 % 5^3) * 5^3 / 5^6 + DataRate * 36 / 5^6 ==
			 *	DataRate * 4 + DataRate * 24 / 5^3 + ((DataRate * 24 % 5^3) * 5^3 + DataRate * 36) / 5^6 ==
			 ****
			 *	(every single expression is < 2^32 if DataRate < 2^20)
			 */
			if (DataRate >= 1000000) {
				DataRate = BIT_22 - 1;
			}
			else {
				DataRate = 4 * DataRate + ((3036 * DataRate + 15625 / 2) / 15625);
			}
		}
		else {	/* DataRate is power of 2 */
			DataRate *= 4;
		}

		if (DataRate > (BIT_22 - 1)) {	/* clip to max */
			DataRate = (BIT_22 - 1);
		}

		DWord = (DWord & ~(BIT_22 - 1)) | DataRate | TX_RATE_INT_K_FACTOR_EN;

		/* Yukon-Optima has 5 MAC FIFO queues but only the first 4 have a rate shaper */

		SK_OUT32(IoC, Y2_AVB_MAC_Q_ADDR(pAC->GIni.GP[Port].PAvbMacQOffs[Queue],
			TX1_RATE_INT), DWord);

		/* Enable rate shaping by clearing TX_RATE_MISC_RATE_SHAPE_DIS bit */
		RegisterSelectiveSetReset(pAC, IoC, Y2_AVB_MAC_Q_ADDR(
			pAC->GIni.GP[Port].PAvbMacQOffs[Queue], TX1_RATE_MISC),
			0, TX_RATE_MISC_RATE_SHAPE_DIS);

		SkYuk2SetClassDataRates(pAC, IoC, Port);
	}
}

/******************************************************************************
 *
 *  SkYuk2GetQueueDataRate() - Get data rate of the specified MAC FIFO
 *
 * Description:
 *  This function gets the data rate from the queue registers.
 *
 * returns:
 *  Datarate or 0 if error
 */
SK_U32 SkYuk2GetQueueDataRate(
SK_AC	*pAC,			/* Adapter Context 				*/
SK_IOC	IoC,			/* I/O Context 					*/
int		Port,			/* Port Index 0 or 1			*/
int		Queue,			/* MAC FIFO queue to setup (0-3)*/
SK_U32	pow)			/* power of DataRate 2 or 10	*/
{
	SK_U32	DataRate = 0;

	if (SkYuk2AVBModeEnabled(pAC, IoC, Port) &&
		Queue >= 0 && Queue < AVB_MAC_Q_CNT) {

		SK_IN32(IoC, Y2_AVB_MAC_Q_ADDR(pAC->GIni.GP[Port].PAvbMacQOffs[Queue],
			TX1_RATE_INT), &DataRate);

		DataRate &= (BIT_22 - 1);

		if (pow == 10) {	/* DataRate is power of 10 -> convert */
			/*
			 * 10^6 / 2^22 = 0.2384185791015625
			 ****
			 * DataRate * 10^6 / 2^22 ==
			 * DataRate * (2^6 * 5^6) / 2^22 ==
			 * DataRate * 5^6 / 2^16 ==
			 * DataRate * 15625 / 2^16 ==
			 *
			 * DataRate * (16384-512-256+8+1)/2^16
			 * DataRate * (1/4-1/128-1/256+1/8192+1/65536)
			 *
			 * (every single expression is < 2^20 if DataRate < 2^22)
			 */
			DataRate =	DataRate / 4 -
						DataRate / 128 -
						DataRate / 256 +
						DataRate / 8192 +
						DataRate / 63336;
		}
		else {	/* DataRate is power of 2 */
			DataRate /= 4;
		}
	}

	return(DataRate);
}

/****************************************************************************
 *
 * \brief Updates data rates for class A and class B
 *
 * \details Updates data rates for class A and class B
 *
 * \return nothing
 *
 *****************************************************************************/
void SkYuk2SetClassDataRates(
SK_AC	*pAC,			/*!< Adapter Context		*/
SK_IOC	IoC,			/*!< I/O Context			*/
int		Port)			/*!< Port Index 0 or 1		*/
{
	int		i;
	int		QueuesClass;
	SK_U32	DWord;
	SK_U32	QueuesRate;
	SK_U32	DataRateClassA = 0;
	SK_U32	DataRateClassB = 0;

	for (i = 0; i < AVB_MAC_Q_CNT; i++) {

		SK_IN32(IoC, Y2_AVB_MAC_Q_ADDR(pAC->GIni.GP[Port].PAvbMacQOffs[i],
			TX1_RATE_MISC),
			&DWord);

		QueuesClass = (int)(DWord & 0x3);

		SK_IN32(IoC, Y2_AVB_MAC_Q_ADDR(pAC->GIni.GP[Port].PAvbMacQOffs[i],
			TX1_RATE_INT),
			&QueuesRate);

		QueuesRate &= (BIT_22 - 1);

		switch (QueuesClass) {
		case 1:
			DataRateClassA += QueuesRate;
			break;
		case 2:
			DataRateClassB += QueuesRate;
			break;
		default:
#ifndef SK_DIAG
			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_INIT,
				("Class %d doesn't support shaping\n", QueuesClass));
#else /* !SK_DIAG */
			c_print("Class %d doesn't support shaping\n", QueuesClass);
#endif /* !SK_DIAG */
		}
	}

	/* writing Class Rates */
	SK_IN32(IoC, Y2_AVB_CLS_ADDR(CLASS_A_RATE_INT, CLASS_A_RATE_INT), &DWord);
	DWord = (DWord & ~(BIT_22 - 1)) | TX_RATE_INT_K_FACTOR_EN | DataRateClassA;
	SK_OUT32(IoC, Y2_AVB_CLS_ADDR(CLASS_A_RATE_INT, CLASS_A_RATE_INT), DWord);

	SK_IN32(IoC, Y2_AVB_CLS_ADDR(CLASS_B_RATE_INT, CLASS_A_RATE_INT), &DWord);
	DWord = (DWord & ~(BIT_22 - 1)) | TX_RATE_INT_K_FACTOR_EN | DataRateClassB;
	SK_OUT32(IoC, Y2_AVB_CLS_ADDR(CLASS_B_RATE_INT, CLASS_A_RATE_INT), DWord);
}

/******************************************************************************
 *
 *  SkYuk2AvbSetQueueClass() - Set class for the specified MAC FIFO
 *
 * Description:
 *  This function sets the class for the specified MAC FIFO queue.
 *
 * returns:
 *  nothing
 */
void SkYuk2AvbSetQueueClass(
SK_AC	*pAC,		/* Adapter Context			*/
SK_IOC	IoC,		/* I/O Context				*/
int		Port,		/* Port Index 0 or 1		*/
int		Queue,		/* MAC FIFO queue (0-3)		*/
SK_U32	Class)		/* New class for queue		*/
{
	if (SkYuk2AVBModeEnabled(pAC, IoC, Port) &&
		Queue >= 0 && Queue < AVB_MAC_Q_CNT) {

		RegisterSelectiveSetReset(pAC, IoC,
			Y2_AVB_MAC_Q_ADDR(pAC->GIni.GP[Port].PAvbMacQOffs[Queue], TX1_RATE_MISC),
				Class & TX_RATE_MISC_CLASS_MSK,
				TX_RATE_MISC_CLASS_MSK);
	}
}

/******************************************************************************
 *
 *  SkYuk2AvbSetQueuePriority() - Set class for the specified MAC FIFO
 *
 * Description:
 *  This function sets the class for the specified MAC FIFO queue.
 *
 * returns:
 *  nothing
 */
void SkYuk2AvbSetQueuePriority(
SK_AC	*pAC,			/* Adapter Context 					*/
SK_IOC	IoC,			/* I/O Context 						*/
int		Port,			/* Port Index 0 or 1				*/
int		Queue,			/* MAC FIFO queue to setup (0-3)	*/
SK_U32	Priority)		/* New priority for queue			*/
{
	if (SkYuk2AVBModeEnabled(pAC, IoC, Port) &&
		Queue >= 0 && Queue < AVB_MAC_Q_CNT) {

		RegisterSelectiveSetReset(pAC, IoC,
			Y2_AVB_MAC_Q_ADDR(pAC->GIni.GP[Port].PAvbMacQOffs[Queue], TX1_RATE_MISC),
				(Priority << TX_RATE_MISC_PRI_BASE) & TX_RATE_MISC_PRI_MSK,
				TX_RATE_MISC_PRI_MSK);
	}
}


/******************************************************************************
 *
 * \brief Maps LE queue to Tx MAC FIFO queue in AVB mode
 *
 * \details Maps LE queue to Tx MAC FIFO queue in AVB mode
 *
  * \return nothing
  *
  *****************************************************************************/
void SkYuk2AvbSetLeQueue(
SK_AC *pAC,			/*!< Adapter Context > 		*/
SK_IOC IoC,			/*!< I/ O Context > 			*/
int Port,			/*!< Port Index 0 or 1 >		*/
int Pfu,			/*!< PFU to map to queue > 	*/
int Queue)			/*!< Queue PFU should be mapped to >	*/
{
	SK_U32	Addr;
	SK_U32	Data;

	if (SkYuk2AVBModeEnabled(pAC, IoC, Port) &&
		Queue >= 1 && Queue <= AVB_MAC_Q_CNT &&
		Pfu >= 6 && Pfu <= AVB_LE_Q_CNT) {

		Addr = TPLE_ADDR(Pfu, REQ_TH);

		SK_IN32(IoC, Addr, &Data);

		Data = (Data & ~TPLE_REQ_TH_H_MAP_MSK) | ((Queue & 0x7) << 29);

		SK_OUT32(IoC, Addr, Data);
	}
}


/******************************************************************************
  *
  * \brief Maps LE queue to Tx MAC FIFO queue
  *
  * \details Maps LE queue to Tx MAC FIFO queue
  *
  * \return nothing
  *
  *****************************************************************************/
void SkYuk2AvbSetLePriority(
SK_AC	*pAC,			/*!< Adapter Context > 		*/
SK_IOC	IoC,			/*!< I/ O Context > 		*/
int		Port,			/*!< Port Index 0 or 1 >	*/
int		Pfu,			/*!< PFU to map to queue > 	*/
int		Priority)		/*!< PFU queue's priority > */
{
	SK_U32	Addr;
	SK_U32	Data;

	if (SkYuk2AVBModeEnabled(pAC, IoC, Port) &&
		Priority >= 0 && Priority <= 3 && Pfu >= 1 && Pfu <= AVB_LE_Q_CNT) {

		Addr = TPLE_ADDR(Pfu, REQ_CTRL);

		SK_IN32(IoC, Addr, &Data);

		Data = (Data & 0xfffffffc) | Priority;

		SK_OUT32(IoC, Addr, Data);
	}
}
#endif /* SK_AVB */

/******************************************************************************
 *
 *  RegisterSelectiveSetReset() - Set/reset bits in specified register
 *
 * Description:
 *  This function helps in modifying bits for registers where
 *  read-modify-write scheme is needed.
 *
 * returns:
 *  nothing
 */
void RegisterSelectiveSetReset(
SK_AC	*pAC,			/* Adapter Context */
SK_IOC	IoC,			/* I/O Context */
SK_U32	Register,		/* Offset of register to modify			*/
SK_U32	BitsToSet,		/* Bits to be set in the register		*/
SK_U32	BitsToClear)	/* Bits to be cleared in the register	*/
{
	SK_U32 Val32;

	SK_IN32(IoC, Register, &Val32);

	Val32 &= ~BitsToClear;
	Val32 |= BitsToSet;

	SK_OUT32(IoC, Register, Val32);
}

#ifndef SK_SLIM
/******************************************************************************
 *
 *	SkGeYellowLED() - Switch the yellow LED on or off.
 *
 * Description:
 *	Switch the yellow LED on or off.
 *
 * Note:
 *	This function may be called any time after SkGeInit(Level 1).
 *
 * Returns:
 *	nothing
 */
void SkGeYellowLED(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		State)		/* yellow LED state, 0 = OFF, 0 != ON */
{
	int	LedReg;

	if (CHIP_ID_YUKON_2(pAC)) {
		/* different mapping on Yukon-2 */
		LedReg = B0_CTST + 1;
	}
	else {
		LedReg = B0_LED;
	}

	if (State == 0) {
		/* Switch state LED OFF */
		SK_OUT8(IoC, LedReg, LED_STAT_OFF);
	}
	else {
		/* Switch state LED ON */
		SK_OUT8(IoC, LedReg, LED_STAT_ON);
	}
}	/* SkGeYellowLED */
#endif /* !SK_SLIM */


/******************************************************************************
 *
 *	DoCalcAddr() - Calculates the start and the end address of a queue.
 *
 * Description:
 *	This function calculates the start and the end address of a queue.
 *  Afterwards the 'StartVal' is incremented to the next start position.
 *	If the port is already initialized the calculated values
 *	will be checked against the configured values and an
 *	error will be returned, if they are not equal.
 *	If the port is not initialized the values will be written to
 *	*StartAdr and *EndAddr.
 *
 * Returns:
 *	0:	success
 *	1:	configuration error
 */
static int DoCalcAddr(
SK_AC		*pAC,				/* Adapter Context */
SK_GEPORT	SK_FAR *pPrt,		/* port index */
int			QuSize,				/* size of the queue to configure in kB */
SK_U32		SK_FAR *StartVal,	/* start value for address calculation */
SK_U32		SK_FAR *QuStartAddr,/* start address to calculate */
SK_U32		SK_FAR *QuEndAddr)	/* end address to calculate */
{
	SK_U32	EndVal;
	SK_U32	NextStart;
	int		Rtv;

	Rtv = 0;
	if (QuSize == 0) {
		EndVal = *StartVal;
		NextStart = EndVal;
	}
	else {
		EndVal = *StartVal + ((SK_U32)QuSize * 1024) - 1;
		NextStart = EndVal + 1;
	}

	if (pPrt->PState >= SK_PRT_INIT) {
		if (*StartVal != *QuStartAddr || EndVal != *QuEndAddr) {
			Rtv = 1;
		}
	}
	else {
		*QuStartAddr = *StartVal;
		*QuEndAddr = EndVal;
	}

	*StartVal = NextStart;
	return(Rtv);
}	/* DoCalcAddr */

/******************************************************************************
 *
 *	SkGeRoundQueueSize() - Round the given queue size to the adapters QZ units
 *
 * Description:
 *	This function rounds the given queue size in kBs to adapter specific
 *	queue size units (Yukon-1: 8 kB, Yukon-2/EC: 1 kB).
 *
 * Returns:
 *	the rounded queue size in kB
 */
static int SkGeRoundQueueSize(
SK_AC	*pAC,		/* Adapter Context */
int	QueueSizeKB)	/* Queue size in kB */
{
	int QueueSizeSteps;

	QueueSizeSteps = (CHIP_ID_YUKON_2(pAC)) ? QZ_STEP_Y2 : QZ_STEP;

	return((QueueSizeKB + QueueSizeSteps - 1) & ~(QueueSizeSteps - 1));
}	/* SkGeRoundQueueSize */


/******************************************************************************
 *
 *	SkGeInitAssignRamToQueues() - Allocate default queue sizes
 *
 * Description:
 *	This function assigns the memory to the different queues and ports.
 *	When DualNet is set to SK_TRUE all ports get the same amount of memory.
 *	Otherwise the first port gets most of the memory and all the
 *	other ports just the required minimum.
 *	This function can only be called when pAC->GIni.GIRamSize and
 *	pAC->GIni.GIMacsFound have been initialized, usually this happens
 *	at init level 1
 *
 * Returns:
 *	0 - ok
 *	1 - invalid input values
 *	2 - not enough memory
 */

int SkGeInitAssignRamToQueues(
SK_AC	*pAC,			/* Adapter Context */
int		ActivePort,		/* Active Port in RLMT mode */
SK_BOOL	DualNet)		/* Dual Net active */
{
	int	i;
	int	UsedKilobytes;			/* memory already assigned */
	int	ActivePortKilobytes;	/* memory available for active port */
	int	MinQueueSize;			/* min. memory for queues */
	int	TotalRamSize;			/* total memory for queues */
	SK_BOOL	DualPortYukon2;
	SK_GEPORT *pPrt;

	if (ActivePort >= pAC->GIni.GIMacsFound) {

		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_INIT,
			("SkGeInitAssignRamToQueues: ActivePort (%d) invalid\n",
			ActivePort));
		return(1);
	}

	DualPortYukon2 = (CHIP_ID_YUKON_2(pAC) && pAC->GIni.GIMacsFound == 2);

	TotalRamSize = pAC->GIni.GIRamSize;

	if (DualPortYukon2) {
		TotalRamSize *= 2;
	}

	MinQueueSize = SK_MIN_RXQ_SIZE + SK_MIN_TXQ_SIZE;

	if (MinQueueSize > pAC->GIni.GIRamSize) {
		MinQueueSize = pAC->GIni.GIRamSize;
	}

	if ((pAC->GIni.GIMacsFound * MinQueueSize +
		 RAM_QUOTA_SYNC * SK_MIN_TXQ_SIZE) > TotalRamSize) {

		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_INIT,
			("SkGeInitAssignRamToQueues: Not enough memory (%d)\n",
			TotalRamSize));
		return(2);
	}

	if (DualNet) {
		/* every port gets the same amount of memory */
		ActivePortKilobytes = TotalRamSize / pAC->GIni.GIMacsFound;

		for (i = 0; i < pAC->GIni.GIMacsFound; i++) {

			pPrt = &pAC->GIni.GP[i];

			if (DualPortYukon2) {
				ActivePortKilobytes = pAC->GIni.GIRamSize;
			}
			/* take away the minimum memory for active queues */
			ActivePortKilobytes -= MinQueueSize;

			/* receive queue gets the minimum + 80% of the rest */
			pPrt->PRxQSize = SkGeRoundQueueSize(pAC,
				(int)((long)ActivePortKilobytes * RAM_QUOTA_RX) / 100) +
				SK_MIN_RXQ_SIZE;

			ActivePortKilobytes -= (pPrt->PRxQSize - SK_MIN_RXQ_SIZE);

			/* synchronous transmit queue */
			pPrt->PXSQSize = 0;

			/* asynchronous transmit queue */
			pPrt->PXAQSize = SkGeRoundQueueSize(pAC,
				ActivePortKilobytes + SK_MIN_TXQ_SIZE);
		}
	}
	else {	/* RLMT Mode or single link adapter */

		UsedKilobytes = 0;

		/* set standby queue size defaults for all standby ports */
		for (i = 0; i < pAC->GIni.GIMacsFound; i++) {

			if (i != ActivePort) {
				pPrt = &pAC->GIni.GP[i];

				if (DualPortYukon2) {
					pPrt->PRxQSize = SkGeRoundQueueSize(pAC,
						(int)((long)(pAC->GIni.GIRamSize - MinQueueSize) *
						RAM_QUOTA_RX) / 100) + SK_MIN_RXQ_SIZE;

					pPrt->PXAQSize = pAC->GIni.GIRamSize - pPrt->PRxQSize;
				}
				else {
					pPrt->PRxQSize = SK_MIN_RXQ_SIZE;
					pPrt->PXAQSize = SK_MIN_TXQ_SIZE;
				}
				pPrt->PXSQSize = 0;

				/* Count used RAM */
				UsedKilobytes += pPrt->PRxQSize + pPrt->PXAQSize;
			}
		}
		/* what's left? */
		ActivePortKilobytes = TotalRamSize - UsedKilobytes;

		/* assign it to the active port */
		/* first take away the minimum memory */
		ActivePortKilobytes -= MinQueueSize;
		pPrt = &pAC->GIni.GP[ActivePort];

		/* receive queue gets 80% of the rest */
		pPrt->PRxQSize = SkGeRoundQueueSize(pAC,
			(int)((long)ActivePortKilobytes * RAM_QUOTA_RX) / 100);

		ActivePortKilobytes -= pPrt->PRxQSize;

		/* add the minimum memory for Rx queue */
		pPrt->PRxQSize += MinQueueSize/2;

		/* synchronous transmit queue */
		pPrt->PXSQSize = 0;

		/* asynchronous transmit queue gets 20% of the rest */
		pPrt->PXAQSize = SkGeRoundQueueSize(pAC, ActivePortKilobytes) +
			/* add the minimum memory for Tx queue */
			MinQueueSize/2;
	}

#ifdef DEBUG
	for (i = 0; i < pAC->GIni.GIMacsFound; i++) {

		pPrt = &pAC->GIni.GP[i];

		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_INIT,
			("Port %d: RxQSize=%u, TxAQSize=%u, TxSQSize=%u\n",
			i, pPrt->PRxQSize, pPrt->PXAQSize, pPrt->PXSQSize));
	}
#endif /* DEBUG */

	return(0);
}	/* SkGeInitAssignRamToQueues */


/******************************************************************************
 *
 *	SkGeCheckQSize() - Checks the Adapters Queue Size Configuration
 *
 * Description:
 *	This function verifies the Queue Size Configuration specified
 *	in the variables PRxQSize, PXSQSize, and PXAQSize of all
 *	used ports.
 *	This requirements must be fullfilled to have a valid configuration:
 *		- The size of all queues must not exceed GIRamSize.
 *		- The queue sizes must be specified in units of 8 kB.
 *		- The size of Rx queues of available ports must not be
 *		  smaller than 16 kB (Yukon-1) resp. 10 kB (Yukon-2).
 *		- The size of at least one Tx queue (synch. or asynch.)
 *		  of available ports must not be smaller than 16 kB,
 *		  resp. 10 kB (Yukon-2) when Jumbo Frames are used.
 *		- The RAM start and end addresses must not be changed
 *		  for ports which are already initialized.
 *	Furthermore SkGeCheckQSize() defines the Start and End Addresses
 *  of all ports and stores them into the HWAC port structure.
 *
 * Returns:
 *	0:	Queue Size Configuration valid
 *	1:	Queue Size Configuration invalid
 */
static int SkGeCheckQSize(
SK_AC	*pAC,		/* Adapter Context */
int		Port)		/* port index */
{
	SK_GEPORT *pPrt;
	int	i;
	int	Rtv;
	int	Rtv2;
	SK_U32	StartAddr;
#ifndef SK_SLIM
	int	UsedMem;	/* total memory used (max. found ports) */
#endif /* !SK_SLIM */

	Rtv = 0;

#ifndef SK_SLIM

	UsedMem = 0;

	for (i = 0; i < pAC->GIni.GIMacsFound; i++) {
		pPrt = &pAC->GIni.GP[i];

		if (CHIP_ID_YUKON_2(pAC)) {
			UsedMem = 0;
		}
		else if (((pPrt->PRxQSize & QZ_UNITS) != 0 ||
				  (pPrt->PXSQSize & QZ_UNITS) != 0 ||
				  (pPrt->PXAQSize & QZ_UNITS) != 0)) {

			SK_ERR_LOG(pAC, SK_ERRCL_SW, SKERR_HWI_E012, SKERR_HWI_E012MSG);
			return(1);
		}

#ifndef SK_DIAG
		if (i == Port && pAC->GIni.GIRamSize > SK_MIN_RXQ_SIZE &&
			pPrt->PRxQSize < SK_MIN_RXQ_SIZE) {
			SK_ERR_LOG(pAC, SK_ERRCL_SW, SKERR_HWI_E011, SKERR_HWI_E011MSG);
			return(1);
		}

		/*
		 * the size of at least one Tx queue (synch. or asynch.) has to be > 0.
		 * if Jumbo Frames are used, this size has to be >= 16 kB.
		 */
		if ((i == Port && pPrt->PXSQSize == 0 && pPrt->PXAQSize == 0) ||
			(pPrt->PPortUsage == SK_JUMBO_LINK &&
			((pPrt->PXSQSize > 0 && pPrt->PXSQSize < SK_MIN_TXQ_SIZE) ||
			 (pPrt->PXAQSize > 0 && pPrt->PXAQSize < SK_MIN_TXQ_SIZE)))) {
				SK_ERR_LOG(pAC, SK_ERRCL_SW, SKERR_HWI_E023, SKERR_HWI_E023MSG);
				return(1);
		}
#endif /* !SK_DIAG */

		UsedMem += pPrt->PRxQSize + pPrt->PXSQSize + pPrt->PXAQSize;

		if (UsedMem > pAC->GIni.GIRamSize) {
			SK_ERR_LOG(pAC, SK_ERRCL_SW, SKERR_HWI_E012, SKERR_HWI_E012MSG);
			return(1);
		}
	}

#endif /* !SK_SLIM */

	/* Now start address calculation */
	StartAddr = pAC->GIni.GIRamOffs;
	for (i = 0; i < pAC->GIni.GIMacsFound; i++) {

		pPrt = &pAC->GIni.GP[i];

		if (CHIP_ID_YUKON_2(pAC)) {
			StartAddr = 0;
		}

		/* Calculate/Check values for the receive queue */
		Rtv2 = DoCalcAddr(pAC, pPrt, pPrt->PRxQSize, &StartAddr,
			&pPrt->PRxQRamStart, &pPrt->PRxQRamEnd);
		Rtv |= Rtv2;

		/* Calculate/Check values for the synchronous Tx queue */
		Rtv2 = DoCalcAddr(pAC, pPrt, pPrt->PXSQSize, &StartAddr,
			&pPrt->PXsQRamStart, &pPrt->PXsQRamEnd);
		Rtv |= Rtv2;

		/* Calculate/Check values for the asynchronous Tx queue */
		Rtv2 = DoCalcAddr(pAC, pPrt, pPrt->PXAQSize, &StartAddr,
			&pPrt->PXaQRamStart, &pPrt->PXaQRamEnd);
		Rtv |= Rtv2;

		if (Rtv) {
			SK_ERR_LOG(pAC, SK_ERRCL_SW, SKERR_HWI_E013, SKERR_HWI_E013MSG);
			return(1);
		}
	}

	return(0);
}	/* SkGeCheckQSize */


/******************************************************************************
 *
 *	SkGeInitMacFifo() - Initialize the MAC FIFOs
 *
 * Description:
 *	Initialize all MAC FIFOs of the specified port
 *
 * Returns:
 *	nothing
 */
static void SkGeInitMacFifo(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port)		/* Port Index (MAC_1 + n) */
{
	SK_U16	Word;
	/*
	 * For each FIFO:
	 *	- release local reset
	 *	- use default value for MAC FIFO size
	 *	- setup defaults for the control register
	 *	- enable the FIFO
	 */

	Word = (SK_U16)GMF_RX_CTRL_DEF;

#ifndef DISABLE_YUKON_I
	/* disable Rx GMAC FIFO Flush for YUKON-Plus */
	if (pAC->GIni.GIYukonLite) {

		Word &= ~GMF_RX_F_FL_ON;
	}
#endif /* !DISABLE_YUKON_I */

	/* configure Rx GMAC FIFO */
	SK_OUT8(IoC, MR_ADDR(Port, RX_GMF_CTRL_T), (SK_U8)GMF_RST_CLR);

	SK_OUT16(IoC, MR_ADDR(Port, RX_GMF_CTRL_T), Word);

	Word = RX_FF_FL_DEF_MSK;

#ifndef SK_DIAG
	if (HW_FEATURE(pAC, HWF_WA_DEV_4115)) {
		/*
		 * Flushing must be enabled (needed for ASF see dev. #4.29),
		 * but the flushing mask should be disabled (see dev. #4.115)
		 */
		Word = 0;
	}
#endif /* !SK_DIAG */

	/* set Rx GMAC FIFO Flush Mask (after clearing reset) */
	SK_OUT16(IoC, MR_ADDR(Port, RX_GMF_FL_MSK), Word);

	/* default: 0x0a -> 56 bytes on Yukon-1 and 64 bytes on Yukon-2 */
	Word = (SK_U16)RX_GMF_FL_THR_DEF;

	if (CHIP_ID_YUKON_2(pAC) && pAC->GIni.GIAsfEnabled) {
		/* WA for dev. #4.30 & 4.89 (reduce to 0x08 -> 48 bytes) */
		Word -= 2;
	}
	else if (HW_FEATURE(pAC, HWF_WA_DEV_521)) {
		Word = 0x178;
	}
	else {
		/*
		* because Pause Packet Truncation in GMAC is not working
		* we have to increase the Flush Threshold to 64 bytes
		* in order to flush pause packets in Rx FIFO on Yukon-1
		*/
		Word++;
	}

	/* set Rx GMAC FIFO Flush Threshold (after clearing reset) */
	SK_OUT16(IoC, MR_ADDR(Port, RX_GMF_FL_THR), Word);

	/* configure Tx GMAC FIFO */
	SK_OUT8(IoC, MR_ADDR(Port, TX_GMF_CTRL_T), (SK_U8)GMF_RST_CLR);

	SK_OUT16(IoC, MR_ADDR(Port, TX_GMF_CTRL_T), (SK_U16)GMF_TX_CTRL_DEF);

	/* set Rx Pause Threshold */
	if (pAC->GIni.GIChipId == CHIP_ID_YUKON_EC_U ||
		pAC->GIni.GIChipId == CHIP_ID_YUKON_EX ||
		pAC->GIni.GIChipId >= CHIP_ID_YUKON_FE_P) {

		if (pAC->GIni.GIChipId == CHIP_ID_YUKON_EC_U ||
			pAC->GIni.GIChipId == CHIP_ID_YUKON_FE_P) {

			Word = HW_FEATURE(pAC, HWF_WA_DEV_521) ?
						SK_DEV521_ULPP : SK_ECU_ULPP;
		}
		else {
			Word = SK_EXT_ULPP;
		}

		/* set Rx Pause Upper Threshold */
		SK_OUT16(IoC, MR_ADDR(Port, RX_GMF_UP_THR), Word);

		/* set Rx Pause Lower Threshold */
		SK_OUT16(IoC, MR_ADDR(Port, RX_GMF_LP_THR), (SK_U16)SK_ECU_LLPP);

		if ((pAC->GIni.GIChipId == CHIP_ID_YUKON_EX &&
			 pAC->GIni.GIChipRev != CHIP_REV_YU_EX_A0) ||
			pAC->GIni.GIChipId >= CHIP_ID_YUKON_FE_P) {
			/* Yukon-Extreme B0 and further devices */
			/* enable Store & Forward mode for TX */
			SK_OUT32(IoC, MR_ADDR(Port, TX_GMF_CTRL_T), TX_STFW_ENA);
		}
		else {
			if (pAC->GIni.GP[Port].PPortUsage == SK_JUMBO_LINK) {
				/* set Tx GMAC FIFO Almost Empty Threshold */
				SK_OUT32(IoC, MR_ADDR(Port, TX_GMF_AE_THR),
					(SK_ECU_JUMBO_WM << 16) | (SK_U16)SK_ECU_AE_THR);
				/* disable Store & Forward mode for TX */
				SK_OUT32(IoC, MR_ADDR(Port, TX_GMF_CTRL_T), TX_STFW_DIS);
			}
			else {
				/* enable Store & Forward mode for TX */
				SK_OUT32(IoC, MR_ADDR(Port, TX_GMF_CTRL_T), TX_STFW_ENA);
			}
		}
	}

	if (HW_FEATURE(pAC, HWF_WA_DEV_520)) {
		/* disable dynamic watermark */
		SK_IN16(IoC, MR_ADDR(Port, TX_GMF_EA), &Word);

		SK_OUT16(IoC, MR_ADDR(Port, TX_GMF_EA), Word & ~TX_DYN_WM_ENA);
	}

}	/* SkGeInitMacFifo */


/******************************************************************************
 *
 *	DoInitRamQueue() - Initialize the RAM Buffer Address of a single Queue
 *
 * Description:
 *	If the queue is used, enable and initialize it.
 *	Make sure the queue is still reset, if it is not used.
 *
 * Returns:
 *	nothing
 */
void DoInitRamQueue(
SK_AC	*pAC,			/* Adapter Context */
SK_IOC	IoC,			/* I/O Context */
int		QuIoOffs,		/* Queue I/O Address Offset */
SK_U32	QuStartAddr,	/* Queue Start Address */
SK_U32	QuEndAddr,		/* Queue End Address */
int		QuType)			/* Queue Type (SK_RX_SRAM_Q|SK_RX_BRAM_Q|SK_TX_RAM_Q) */
{
	SK_U32	RxUpThresVal;
	SK_U32	RxLoThresVal;

	if (QuStartAddr != QuEndAddr) {
		/* calculate thresholds, assume we have a big Rx queue */
		RxUpThresVal = (QuEndAddr + 1 - QuStartAddr - SK_RB_ULPP) / 8;
		RxLoThresVal = (QuEndAddr + 1 - QuStartAddr - SK_RB_LLPP_B)/8;

		/* build HW address format */
		QuStartAddr = QuStartAddr / 8;
		QuEndAddr = QuEndAddr / 8;

		/* release local reset */
		SK_OUT8(IoC, RB_ADDR(QuIoOffs, RB_CTRL), RB_RST_CLR);

		/* configure addresses */
		SK_OUT32(IoC, RB_ADDR(QuIoOffs, RB_START), QuStartAddr);
		SK_OUT32(IoC, RB_ADDR(QuIoOffs, RB_END), QuEndAddr);
		SK_OUT32(IoC, RB_ADDR(QuIoOffs, RB_WP), QuStartAddr);
		SK_OUT32(IoC, RB_ADDR(QuIoOffs, RB_RP), QuStartAddr);

		switch (QuType) {
		case SK_RX_SRAM_Q:
			/* configure threshold for small Rx Queue */
			RxLoThresVal += (SK_RB_LLPP_B - SK_RB_LLPP_S) / 8;

			/* continue with SK_RX_BRAM_Q */
		case SK_RX_BRAM_Q:
			/* write threshold for Rx Queue (Pause packets) */
			SK_OUT32(IoC, RB_ADDR(QuIoOffs, RB_RX_UTPP), RxUpThresVal);
			SK_OUT32(IoC, RB_ADDR(QuIoOffs, RB_RX_LTPP), RxLoThresVal);

			/* the high priority threshold not used */
			break;
		case SK_TX_RAM_Q:
			/*
			 * GMAC Tx FIFO is only 1 kB
			 * enable Store & Forward Mode for the Tx Side
			 */
			SK_OUT8(IoC, RB_ADDR(QuIoOffs, RB_CTRL), RB_ENA_STFWD);
			break;
		}

		/* set queue operational */
		SK_OUT8(IoC, RB_ADDR(QuIoOffs, RB_CTRL), RB_ENA_OP_MD);
	}
	else {
		/* ensure the queue is still disabled */
		SK_OUT8(IoC, RB_ADDR(QuIoOffs, RB_CTRL), RB_RST_SET);
	}
}	/* DoInitRamQueue */


/******************************************************************************
 *
 *	SkGeInitRamBufs() - Initialize the RAM Buffer Queues
 *
 * Description:
 *	Initialize all RAM Buffer Queues of the specified port
 *
 * Returns:
 *	nothing
 */
static void SkGeInitRamBufs(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port)		/* Port Index (MAC_1 + n) */
{
	SK_GEPORT *pPrt;
	int RxQType;

	if (!HW_IS_RAM_IF_AVAIL(pAC)) {
		return;
	}

	pPrt = &pAC->GIni.GP[Port];

	if (pPrt->PRxQSize <= SK_MIN_RXQ_SIZE) {
		RxQType = SK_RX_SRAM_Q;		/* small Rx Queue */
	}
	else {
		RxQType = SK_RX_BRAM_Q;		/* big Rx Queue */
	}

	DoInitRamQueue(pAC, IoC, pPrt->PRxQOff, pPrt->PRxQRamStart,
		pPrt->PRxQRamEnd, RxQType);

	DoInitRamQueue(pAC, IoC, pPrt->PXsQOff, pPrt->PXsQRamStart,
		pPrt->PXsQRamEnd, SK_TX_RAM_Q);

	DoInitRamQueue(pAC, IoC, pPrt->PXaQOff, pPrt->PXaQRamStart,
		pPrt->PXaQRamEnd, SK_TX_RAM_Q);

}	/* SkGeInitRamBufs */


/******************************************************************************
 *
 *	SkGeInitRamIface() - Initialize the RAM Interface
 *
 * Description:
 *	This function initializes the Adapters RAM Interface.
 *
 * Note:
 *	This function is used in the diagnostics.
 *
 * Returns:
 *	nothing
 */
void SkGeInitRamIface(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC)		/* I/O Context */
{
	int i;
	int RamBuffers;

	if (!HW_IS_RAM_IF_AVAIL(pAC)) {
		return;
	}

	if (CHIP_ID_YUKON_2(pAC)) {
		RamBuffers = pAC->GIni.GIMacsFound;
	}
	else {
		RamBuffers = 1;
	}

	for (i = 0; i < RamBuffers; i++) {
		/* release local reset */
		SK_OUT8(IoC, SELECT_RAM_BUFFER(i, B3_RI_CTRL), (SK_U8)RI_RST_CLR);

		/* configure timeout values */
		SK_OUT8(IoC, SELECT_RAM_BUFFER(i, B3_RI_WTO_R1), SK_RI_TO_53);
		SK_OUT8(IoC, SELECT_RAM_BUFFER(i, B3_RI_WTO_XA1), SK_RI_TO_53);
		SK_OUT8(IoC, SELECT_RAM_BUFFER(i, B3_RI_WTO_XS1), SK_RI_TO_53);
		SK_OUT8(IoC, SELECT_RAM_BUFFER(i, B3_RI_RTO_R1), SK_RI_TO_53);
		SK_OUT8(IoC, SELECT_RAM_BUFFER(i, B3_RI_RTO_XA1), SK_RI_TO_53);
		SK_OUT8(IoC, SELECT_RAM_BUFFER(i, B3_RI_RTO_XS1), SK_RI_TO_53);
		SK_OUT8(IoC, SELECT_RAM_BUFFER(i, B3_RI_WTO_R2), SK_RI_TO_53);
		SK_OUT8(IoC, SELECT_RAM_BUFFER(i, B3_RI_WTO_XA2), SK_RI_TO_53);
		SK_OUT8(IoC, SELECT_RAM_BUFFER(i, B3_RI_WTO_XS2), SK_RI_TO_53);
		SK_OUT8(IoC, SELECT_RAM_BUFFER(i, B3_RI_RTO_R2), SK_RI_TO_53);
		SK_OUT8(IoC, SELECT_RAM_BUFFER(i, B3_RI_RTO_XA2), SK_RI_TO_53);
		SK_OUT8(IoC, SELECT_RAM_BUFFER(i, B3_RI_RTO_XS2), SK_RI_TO_53);
	}
}	/* SkGeInitRamIface */


/******************************************************************************
 *
 *	SkGeInitBmu() - Initialize the BMU state machines
 *
 * Description:
 *	Initialize all BMU state machines of the specified port
 *
 * Returns:
 *	nothing
 */
static void SkGeInitBmu(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port)		/* Port Index (MAC_1 + n) */
{
	SK_GEPORT	*pPrt;
	SK_U16		RxWm;
	SK_U16		TxWm;

	pPrt = &pAC->GIni.GP[Port];

	RxWm = SK_BMU_RX_WM;
	TxWm = SK_BMU_TX_WM;

	if (CHIP_ID_YUKON_2(pAC)) {

		if (pAC->GIni.GIPciBus == SK_PEX_BUS) {
			/* for better performance set it to 128 */
			RxWm = SK_BMU_RX_WM_PEX;
		}

		/* Rx Queue: Release all local resets and set the watermark */
		SK_OUT32(IoC, Q_ADDR(pPrt->PRxQOff, Q_CSR), BMU_CLR_RESET);
		SK_OUT32(IoC, Q_ADDR(pPrt->PRxQOff, Q_CSR), BMU_OPER_INIT);
		SK_OUT32(IoC, Q_ADDR(pPrt->PRxQOff, Q_CSR), BMU_FIFO_OP_ON);

		SK_OUT16(IoC, Q_ADDR(pPrt->PRxQOff, Q_WM), RxWm);

		if (pAC->GIni.GIChipId == CHIP_ID_YUKON_EC_U &&
			pAC->GIni.GIChipRev > CHIP_REV_YU_EC_U_A0) {
			/* MAC Rx RAM Read is controlled by hardware */
			SK_OUT32(IoC, Q_ADDR(pPrt->PRxQOff, Q_F), F_M_RX_RAM_DIS);
		}

		/* Tx Queues: check if the queues are used ! */
		if (pPrt->PXSQSize != 0 && HW_SYNC_TX_SUPPORTED(pAC)) {
			/* Sync Tx Queue: Release all local resets and set the watermark */
			SK_OUT32(IoC, Q_ADDR(pPrt->PXsQOff, Q_CSR), BMU_CLR_RESET);
			SK_OUT32(IoC, Q_ADDR(pPrt->PXsQOff, Q_CSR), BMU_OPER_INIT);
			SK_OUT32(IoC, Q_ADDR(pPrt->PXsQOff, Q_CSR), BMU_FIFO_OP_ON);

			SK_OUT16(IoC, Q_ADDR(pPrt->PXsQOff, Q_WM), TxWm);
		}

		if (pPrt->PXAQSize != 0) {
			/* Async Tx Queue: Release all local resets and set the watermark */
			SK_OUT32(IoC, Q_ADDR(pPrt->PXaQOff, Q_CSR), BMU_CLR_RESET);
			SK_OUT32(IoC, Q_ADDR(pPrt->PXaQOff, Q_CSR), BMU_OPER_INIT);
			SK_OUT32(IoC, Q_ADDR(pPrt->PXaQOff, Q_CSR), BMU_FIFO_OP_ON);

#if (!defined(SK_SLIM) && !defined(SK_DIAG))
			if (HW_FEATURE(pAC, HWF_TX_IP_ID_INCR_ON)) {
				/* Enable Tx IP ID Increment */
				SK_OUT32(IoC, Q_ADDR(pPrt->PXaQOff, Q_CSR), BMU_TX_IPIDINCR_ON);
			}
#endif /* !SK_SLIM && !SK_DIAG */

			SK_OUT16(IoC, Q_ADDR(pPrt->PXaQOff, Q_WM), TxWm);

			if (pAC->GIni.GIChipId == CHIP_ID_YUKON_EC_U &&
				pAC->GIni.GIChipRev == CHIP_REV_YU_EC_U_A0) {
				/* fix for Yukon-EC Ultra: set BMU FIFO level */
				SK_OUT16(IoC, Q_ADDR(pPrt->PXaQOff, Q_AL), SK_ECU_TXFF_LEV);
			}
		}
	}
#ifndef DISABLE_YUKON_I
	else {
		if (!pAC->GIni.GIPciSlot64 && !pAC->GIni.GIPciClock66) {
			/* for better performance */
			RxWm /= 2;
			TxWm /= 2;
		}

		/* Rx Queue: Release all local resets and set the watermark */
		SK_OUT32(IoC, Q_ADDR(pPrt->PRxQOff, Q_CSR), CSR_CLR_RESET);
		SK_OUT32(IoC, Q_ADDR(pPrt->PRxQOff, Q_F), RxWm);

		/*
		 * Tx Queue: Release all local resets if the queue is used !
		 *		set watermark
		 */
		if (pPrt->PXSQSize != 0) {
			SK_OUT32(IoC, Q_ADDR(pPrt->PXsQOff, Q_CSR), CSR_CLR_RESET);
			SK_OUT32(IoC, Q_ADDR(pPrt->PXsQOff, Q_F), TxWm);
		}

		if (pPrt->PXAQSize != 0) {
			SK_OUT32(IoC, Q_ADDR(pPrt->PXaQOff, Q_CSR), CSR_CLR_RESET);
			SK_OUT32(IoC, Q_ADDR(pPrt->PXaQOff, Q_F), TxWm);
		}
	}
#endif /* !DISABLE_YUKON_I */
	/*
	 * Do NOT enable the descriptor poll timers here, because
	 * the descriptor addresses are not specified yet.
	 */
}	/* SkGeInitBmu */


/******************************************************************************
 *
 *	TestStopBit() - Test the stop bit of the queue
 *
 * Description:
 *	Stopping a queue is not as simple as it seems to be.
 *	If descriptor polling is enabled, it may happen
 *	that RX/TX stop is done and SV idle is NOT set.
 *	In this case we have to issue another stop command.
 *
 * Returns:
 *	The queues control status register
 */
static SK_U32 TestStopBit(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		QuIoOffs)	/* Queue I/O Address Offset */
{
	SK_U32	QuCsr;	/* CSR contents */

	SK_IN32(IoC, Q_ADDR(QuIoOffs, Q_CSR), &QuCsr);

	if (CHIP_ID_YUKON_2(pAC)) {
		if ((QuCsr & (BMU_STOP | BMU_IDLE)) == 0) {
			/* Stop Descriptor overridden by start command */
			SK_OUT32(IoC, Q_ADDR(QuIoOffs, Q_CSR), BMU_STOP);

			SK_IN32(IoC, Q_ADDR(QuIoOffs, Q_CSR), &QuCsr);
		}
	}
#ifndef DISABLE_YUKON_I
	else {
		if ((QuCsr & (CSR_STOP | CSR_SV_IDLE)) == 0) {
			/* Stop Descriptor overridden by start command */
			SK_OUT32(IoC, Q_ADDR(QuIoOffs, Q_CSR), CSR_STOP);

			SK_IN32(IoC, Q_ADDR(QuIoOffs, Q_CSR), &QuCsr);
		}
	}
#endif /* !DISABLE_YUKON_I */

	return(QuCsr);
}	/* TestStopBit */


/******************************************************************************
 *
 *	SkGeStopPort() - Stop the Rx/Tx activity of the port 'Port'.
 *
 * Description:
 *	After calling this function the descriptor rings and Rx and Tx
 *	queues of this port may be reconfigured.
 *
 *	It is possible to stop the receive and transmit path separate or
 *	both together.
 *
 *	Dir =	SK_STOP_TX	Stops the transmit path only and resets the MAC.
 *				The receive queue is still active and
 *				the pending Rx frames may be still transferred
 *				into the RxD.
 *		SK_STOP_RX	Stop the receive path. The tansmit path
 *				has to be stopped once before.
 *		SK_STOP_ALL	SK_STOP_TX + SK_STOP_RX
 *
 *	RstMode =	SK_SOFT_RST	Resets the MAC, the PHY is still alive.
 *				SK_HARD_RST	Resets the MAC and the PHY.
 *
 * Example:
 *	1) A Link Down event was signaled for a port. Therefore the activity
 *	of this port should be stopped and a hardware reset should be issued
 *	to enable the workaround of XMAC Errata #2.
 *	But the received frames should not be discarded.
 *		...
 *		SkGeStopPort(pAC, IoC, Port, SK_STOP_TX, SK_HARD_RST);
 *		(transfer all pending Rx frames)
 *		SkGeStopPort(pAC, IoC, Port, SK_STOP_RX, SK_HARD_RST);
 *		...
 *
 *	2) An event was issued which request the driver to switch
 *	the 'virtual active' link to an other already active port
 *	as soon as possible. The frames in the receive queue of this
 *	port may be lost. But the PHY must not be reset during this
 *	event.
 *		...
 *		SkGeStopPort(pAC, IoC, Port, SK_STOP_ALL, SK_SOFT_RST);
 *		...
 *
 * Extended Description:
 *	If SK_STOP_TX is set,
 *		o disable the MAC's receive and transmitter to prevent
 *		  from sending incomplete frames
 *		o stop the port's transmit queues before terminating the
 *		  BMUs to prevent from performing incomplete PCI cycles
 *		  on the PCI bus
 *		- The network Rx and Tx activity and PCI Tx transfer is
 *		  disabled now.
 *		o reset the MAC depending on the RstMode
 *		o Stop Interval Timer and Limit Counter of Tx Arbiter,
 *		  also disable Force Sync bit and Enable Alloc bit.
 *		o perform a local reset of the port's Tx path
 *			- reset the PCI FIFO of the async Tx queue
 *			- reset the PCI FIFO of the sync Tx queue
 *			- reset the RAM Buffer async Tx queue
 *			- reset the RAM Buffer sync Tx queue
 *			- reset the MAC Tx FIFO
 *		o switch Link and Tx LED off, stop the LED counters
 *
 *	If SK_STOP_RX is set,
 *		o stop the port's receive queue
 *		- The path data transfer activity is fully stopped now.
 *		o perform a local reset of the port's Rx path
 *			- reset the PCI FIFO of the Rx queue
 *			- reset the RAM Buffer receive queue
 *			- reset the MAC Rx FIFO
 *		o switch Rx LED off, stop the LED counter
 *
 *	If all ports are stopped,
 *		o reset the RAM Interface.
 *
 * Notes:
 *	o This function may be called during the driver states RESET_PORT and
 *	  SWITCH_PORT.
 */
void SkGeStopPort(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port,		/* Port to stop (MAC_1 + n) */
int		Dir,		/* Direction to Stop (SK_STOP_RX, SK_STOP_TX, SK_STOP_ALL) */
int		RstMode)	/* Reset Mode (SK_SOFT_RST, SK_HARD_RST) */
{
	SK_GEPORT *pPrt;
#ifndef DISABLE_YUKON_I
	SK_U32	RxCsr;
#endif /* !DISABLE_YUKON_I */
	SK_U32	XsCsr;
	SK_U32	XaCsr;
	SK_U32	CsrStart;
	SK_U32	CsrStop;
	SK_U32	CsrIdle;
	SK_U32	CsrTest;
	SK_U32	StartTime;
	SK_U32	TimeOut;
	SK_U8	FifoRdShLev;	/* FIFO read shadow level */
	SK_U8	FifoRdLev;		/* FIFO read level */
	int		i;
	int		ToutCnt;

	pPrt = &pAC->GIni.GP[Port];

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("SkGeStopPort: Port %d, Dir %d, RstMode %d\n",
		 Port, Dir, RstMode));

	/* set the proper values of Q_CSR register layout depending on the chip */
	if (CHIP_ID_YUKON_2(pAC)) {
		CsrStart = BMU_START;
		CsrStop = BMU_STOP;
		CsrIdle = BMU_IDLE;
		CsrTest = BMU_IDLE;
	}
#ifndef DISABLE_YUKON_I
	else {
		CsrStart = CSR_START;
		CsrStop = CSR_STOP;
		CsrIdle = CSR_SV_IDLE;
		CsrTest = CSR_SV_IDLE | CSR_STOP;
	}
#endif /* !DISABLE_YUKON_I */

	if ((Dir & SK_STOP_TX) != 0) {

		if (!pAC->GIni.GIAsfEnabled) {
			/* disable receiver and transmitter */
			SkMacRxTxDisable(pAC, IoC, Port);
		}
		else if (pAC->GIni.GIChipId == CHIP_ID_YUKON_EX ||
				 pAC->GIni.GIChipId == CHIP_ID_YUKON_SUPR) {
			/*
			 * Enable flushing of non ASF packets;
			 * required, because MAC is not disabled
			 */
			SK_OUT32(IoC, MR_ADDR(Port, RX_GMF_CTRL_T),
				RXMF_TCTL_MACSEC_FLSH_ENA);
		}

		/* stop both transmit queues */
		if (pPrt->PXSQSize != 0) {
			SK_OUT32(IoC, Q_ADDR(pPrt->PXsQOff, Q_CSR), CsrStop);
		}

		SK_OUT32(IoC, Q_ADDR(pPrt->PXaQOff, Q_CSR), CsrStop);

		/* If the BMU is in reset, CSR_STOP will terminate immediately */

		/* get current value of timestamp timer */
		StartTime = SkHwGetTimeDelta(pAC, IoC, 0);

		ToutCnt = 0;

		/* set timeout to 10 ms */
		TimeOut = HW_MS_TO_TICKS(pAC, 10);

		do {
			XaCsr = TestStopBit(pAC, IoC, pPrt->PXaQOff);

			if (HW_SYNC_TX_SUPPORTED(pAC)) {
				XsCsr = TestStopBit(pAC, IoC, pPrt->PXsQOff);
			}
			else {
				XsCsr = XaCsr;
			}

			if (SkHwGetTimeDelta(pAC, IoC, StartTime) > TimeOut) {
				/* Timeout of 10 ms reached */
				ToutCnt++;
				if (ToutCnt > 1) {
					/*
					 * If BMU stop doesn't terminate, we assume that
					 * we have a stable state and can reset the BMU,
					 * the Prefetch Unit, and RAM buffer now.
					 */
					break;			/* ===> leave do/while loop here */
				}
				/*
				 * Cache incoherency workaround: assume a start command
				 * has been lost while sending the frame.
				 */

				/* get current value of timestamp timer */
				StartTime = SkHwGetTimeDelta(pAC, IoC, 0);

				if (pPrt->PXSQSize != 0 && (XsCsr & CsrStop) != 0) {
					SK_OUT32(IoC, Q_ADDR(pPrt->PXsQOff, Q_CSR), CsrStart);
				}

				if ((XaCsr & CsrStop) != 0) {
					SK_OUT32(IoC, Q_ADDR(pPrt->PXaQOff, Q_CSR), CsrStart);
				}

				/*
				 * After the previous operations the X(s|a)Csr does no
				 * longer contain the proper values
				 */
				XaCsr = TestStopBit(pAC, IoC, pPrt->PXaQOff);

				if (HW_SYNC_TX_SUPPORTED(pAC)) {
					XsCsr = TestStopBit(pAC, IoC, pPrt->PXsQOff);
				}
				else {
					XsCsr = XaCsr;
				}
			}
		} while (((XsCsr & CsrTest) != CsrIdle ||
				  (XaCsr & CsrTest) != CsrIdle));

		if (pAC->GIni.GIAsfEnabled) {

			pPrt->PState = (RstMode == SK_SOFT_RST) ? SK_PRT_STOP :
				SK_PRT_RESET;
		}
		else {
			/* Reset the MAC depending on the RstMode */
			if (RstMode == SK_SOFT_RST || pAC->GIni.GIDontInitPhy) {

				SkMacSoftRst(pAC, IoC, Port);
			}
			else {
#ifdef SK_DIAG
				if (HW_FEATURE(pAC, HWF_WA_DEV_472) && Port == MAC_1 &&
					pAC->GIni.GP[MAC_2].PState == SK_PRT_RUN) {

					pAC->GIni.GP[MAC_1].PState = SK_PRT_RESET;

					/* set GPHY Control reset */
					SK_OUT8(IoC, MR_ADDR(MAC_1, GPHY_CTRL), (SK_U8)GPC_RST_SET);
				}
				else {

					SkMacHardRst(pAC, IoC, Port);
				}
#else /* !SK_DIAG */
				SkMacHardRst(pAC, IoC, Port);
#endif /* !SK_DIAG */
			}
		}

		/* disable Force Sync bit and Enable Alloc bit */
		SK_OUT8(IoC, MR_ADDR(Port, TXA_CTRL),
			TXA_DIS_FSYNC | TXA_DIS_ALLOC | TXA_STOP_RC);

		/* Stop Interval Timer and Limit Counter of Tx Arbiter */
		SK_OUT32(IoC, MR_ADDR(Port, TXA_ITI_INI), 0L);
		SK_OUT32(IoC, MR_ADDR(Port, TXA_LIM_INI), 0L);

		/* Perform a local reset of the port's Tx path */
		if (CHIP_ID_YUKON_2(pAC)) {
			/* Reset the PCI FIFO of the async Tx queue */
			SK_OUT32(IoC, Q_ADDR(pPrt->PXaQOff, Q_CSR),
				BMU_FIFO_RST | BMU_RST_SET);

			/* Reset the Tx prefetch units */
			SK_OUT8(IoC, Y2_PREF_Q_ADDR(pPrt->PXaQOff, PREF_UNIT_CTRL_REG),
				PREF_UNIT_RST_SET);

			if (pPrt->PXSQSize != 0) {
				/* Reset the PCI FIFO of the sync Tx queue */
				SK_OUT32(IoC, Q_ADDR(pPrt->PXsQOff, Q_CSR),
					BMU_FIFO_RST | BMU_RST_SET);

				SK_OUT8(IoC, Y2_PREF_Q_ADDR(pPrt->PXsQOff, PREF_UNIT_CTRL_REG),
					PREF_UNIT_RST_SET);
			}

#ifdef SK_AVB
			/* Reset the AVB prefetch units */
			if (HW_SUP_AVB(pAC)) {
				/* Resets the prefetch units before it sets the AVB mode */
				SkYuk2AVBControl(pAC, IoC, Port,
					SkYuk2AVBModeEnabled(pAC, IoC, Port));
			}
#endif /* SK_AVB */
		}
#ifndef DISABLE_YUKON_I
		else {
			/* Reset the PCI FIFO of the async Tx queue */
			SK_OUT32(IoC, Q_ADDR(pPrt->PXaQOff, Q_CSR), CSR_SET_RESET);

			if (pPrt->PXSQSize != 0) {
				/* Reset the PCI FIFO of the sync Tx queue */
				SK_OUT32(IoC, Q_ADDR(pPrt->PXsQOff, Q_CSR), CSR_SET_RESET);
			}
		}
#endif /* !DISABLE_YUKON_I */

		if (HW_IS_RAM_IF_AVAIL(pAC)) {
			/* Reset the RAM Buffer async Tx queue */
			SK_OUT8(IoC, RB_ADDR(pPrt->PXaQOff, RB_CTRL), RB_RST_SET);

			if (pPrt->PXSQSize != 0) {
				/* Reset the RAM Buffer sync Tx queue */
				SK_OUT8(IoC, RB_ADDR(pPrt->PXsQOff, RB_CTRL), RB_RST_SET);
			}
		}

		/* do the reset only if ASF is not enabled */
		if (!pAC->GIni.GIAsfEnabled) {
			/* Reset Tx MAC FIFO */
			SK_OUT8(IoC, MR_ADDR(Port, TX_GMF_CTRL_T), (SK_U8)GMF_RST_SET);
		}

		/* set Pause Off */
		SK_OUT8(IoC, MR_ADDR(Port, GMAC_CTRL), (SK_U8)GMC_PAUSE_OFF);
	}

	if ((Dir & SK_STOP_RX) != 0) {

		if (CHIP_ID_YUKON_2(pAC)) {
			/*
			 * The RX Stop command will not work for Yukon-2 if the BMU does not
			 * reach the end of packet and since we can't make sure that we have
			 * incoming data, we must reset the BMU while it is not during a DMA
			 * transfer. Since it is possible that the RX path is still active,
			 * the RX RAM buffer will be stopped first, so any possible incoming
			 * data will not trigger a DMA. After the RAM buffer is stopped, the
			 * BMU is polled until any DMA in progress is ended and only then it
			 * will be reset.
			 */

			if (HW_IS_RAM_IF_AVAIL(pAC)) {
				/* disable the RAM Buffer receive queue */
				SK_OUT8(IoC, RB_ADDR(pPrt->PRxQOff, RB_CTRL), RB_DIS_OP_MD);

				i = 0xffff;
				while (--i) {
					SK_IN8(IoC, RB_ADDR(pPrt->PRxQOff, Q_RX_RSL), &FifoRdShLev);
					SK_IN8(IoC, RB_ADDR(pPrt->PRxQOff, Q_RX_RL), &FifoRdLev);

					if (FifoRdShLev == FifoRdLev) {
						break;
					}
				}
			}

			/*
			 * If the Rx side is blocked, the above loop cannot terminate.
			 * But, if there was any traffic it should be terminated, now.
			 * However, stop the Rx BMU and the Prefetch Unit !
			 */
			SK_OUT32(IoC, Q_ADDR(pPrt->PRxQOff, Q_CSR),
				BMU_FIFO_RST | BMU_RST_SET);
			/* reset the Rx prefetch unit */
			SK_OUT8(IoC, Y2_PREF_Q_ADDR(pPrt->PRxQOff, PREF_UNIT_CTRL_REG),
				PREF_UNIT_RST_SET);
		}
#ifndef DISABLE_YUKON_I
		else {
			/*
			 * The RX Stop Command will not terminate if no buffers
			 * are queued in the RxD ring. But it will always reach
			 * the Idle state. Therefore we can use this feature to
			 * stop the transfer of received packets.
			 */
			/* stop the port's receive queue */
			SK_OUT32(IoC, Q_ADDR(pPrt->PRxQOff, Q_CSR), CsrStop);

			i = 100;
			do {
				RxCsr = TestStopBit(pAC, IoC, pPrt->PRxQOff);

				/* timeout if i==0 (bug fix for #10748) */
				if (--i == 0) {
					SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_HWI_E024,
						SKERR_HWI_E024MSG);
					break;
				}
			} while ((RxCsr & CsrTest) != CsrIdle);
			/* The path data transfer activity is fully stopped now */

			/* Perform a local reset of the port's Rx path */
			/* Reset the PCI FIFO of the Rx queue */
			SK_OUT32(IoC, Q_ADDR(pPrt->PRxQOff, Q_CSR), CSR_SET_RESET);
		}
#endif /* !DISABLE_YUKON_I */

		if (HW_IS_RAM_IF_AVAIL(pAC)) {
			/* Reset the RAM Buffer receive queue */
			SK_OUT8(IoC, RB_ADDR(pPrt->PRxQOff, RB_CTRL), RB_RST_SET);
		}

		if (!pAC->GIni.GIAsfEnabled) {
			/* Reset Rx MAC FIFO */
			SK_OUT8(IoC, MR_ADDR(Port, RX_GMF_CTRL_T), (SK_U8)GMF_RST_SET);
		}

#ifndef DISABLE_YUKON_I
		/* WA for Dev. #4.169 */
		if ((pAC->GIni.GIChipId == CHIP_ID_YUKON ||
			 pAC->GIni.GIChipId == CHIP_ID_YUKON_LITE) &&
			RstMode == SK_HARD_RST) {
			/* set Link Control reset */
			SK_OUT8(IoC, MR_ADDR(Port, GMAC_LINK_CTRL), (SK_U8)GMLC_RST_SET);

			/* clear Link Control reset */
			SK_OUT8(IoC, MR_ADDR(Port, GMAC_LINK_CTRL), (SK_U8)GMLC_RST_CLR);
		}
#endif /* !DISABLE_YUKON_I */
	}
}	/* SkGeStopPort */


/******************************************************************************
 *
 *	SkGeInit0() - Level 0 Initialization
 *
 * Description:
 *	- Initialize the BMU address offsets
 *
 * Returns:
 *	nothing
 */
static void SkGeInit0(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC)		/* I/O Context */
{
	SK_GEPORT *pPrt;
	int i;
#ifdef SK_AVB
	int q;
#endif /* SK_AVB */

	for (i = 0; i < SK_MAX_MACS; i++) {
		pPrt = &pAC->GIni.GP[i];

		pPrt->PState = SK_PRT_RESET;
		pPrt->PPortUsage = SK_RED_LINK;
		pPrt->PRxQOff = QOffTab[i].RxQOff;
		pPrt->PXsQOff = QOffTab[i].XsQOff;
		pPrt->PXaQOff = QOffTab[i].XaQOff;

#ifdef SK_AVB
		for (q = 0; q < AVB_LE_Q_CNT; q++) {
			pPrt->PTxQOffs[q] = QOffTab[i].AvbQOff[q];
		}
		for (q = 0; q < AVB_MAC_Q_CNT; q++) {
			pPrt->PAvbMacQOffs[q] = QOffTab[i].AvbMacQOff[q];
		}
		for (q = 0; q <AVB_CLS_CNT; q++) {
			pPrt->PAvbClsQOffs[q] = QOffTab[i].AvbClsQOff[q];
		}
#endif /* SK_AVB */

		pPrt->PCheckPar = SK_FALSE;
		pPrt->PLinkMode = (SK_U8)SK_LMODE_AUTOFULL;
		pPrt->PLinkSpeedCap = (SK_U8)SK_LSPEED_CAP_1000MBPS;
		pPrt->PLinkSpeed = (SK_U8)SK_LSPEED_1000MBPS;
		pPrt->PLinkSpeedUsed = (SK_U8)SK_LSPEED_STAT_UNKNOWN;
		pPrt->PLinkModeConf = (SK_U8)SK_LMODE_AUTOSENSE;
		pPrt->PFlowCtrlMode = (SK_U8)SK_FLOW_MODE_SYM_OR_REM;
#ifndef SK_SLIM
		pPrt->PLinkCap = (SK_U8)(SK_LMODE_CAP_HALF | SK_LMODE_CAP_FULL |
			SK_LMODE_CAP_AUTOHALF | SK_LMODE_CAP_AUTOFULL);
		pPrt->PLinkModeStatus = (SK_U8)SK_LMODE_STAT_UNKNOWN;
		pPrt->PFlowCtrlCap = (SK_U8)SK_FLOW_MODE_SYM_OR_REM;
		pPrt->PMSCap = 0;
		pPrt->PLipaAbil = 0;
#endif /* !SK_SLIM */
		pPrt->PFlowCtrlStatus = (SK_U8)SK_FLOW_STAT_NONE;
		pPrt->PMSMode = (SK_U8)SK_MS_MODE_AUTO;
		pPrt->PMSStatus = (SK_U8)SK_MS_STAT_UNSET;
		pPrt->PLipaAutoNeg = (SK_U8)SK_LIPA_UNKNOWN;
		pPrt->PEnDetMode = SK_FALSE;
		pPrt->PAutoNegFail = SK_FALSE;
		pPrt->PHWLinkUp = SK_FALSE;
		pPrt->PPhyQLink = SK_FALSE;
		pPrt->PPhyPowerState = PHY_PM_OPERATIONAL_MODE;
#ifndef SK_SLIM
		pPrt->PMacColThres = TX_COL_DEF;
		pPrt->PMacBackOffLim = TX_BOF_LIM_DEF;
		pPrt->PMacJamIpgData = TX_IPG_JAM_DEF;
		pPrt->PMacJamIpgVal = TX_JAM_IPG_DEF;
		pPrt->PMacJamLen = TX_JAM_LEN_DEF;
		pPrt->PMacDataBlind = DATA_BLIND_DEF;
		pPrt->PMacIpgData_1G = IPG_DATA_DEF_1000;
		pPrt->PMacIpgData_FE = IPG_DATA_DEF_10_100;
		pPrt->PMacLimit4 = SK_FALSE;
#endif /* !SK_SLIM */
	}

	pAC->GIni.GILedBlinkCtrl = (SK_U16)OemConfig.Value;
	pAC->GIni.GIChipCap = 0;
	pAC->GIni.GIPexCapOffs = 0;
	pAC->GIni.GIDontInitPhy = SK_FALSE;
	pAC->GIni.GIExtLeFormat = SK_FALSE;
	pAC->GIni.GIVMainAvail = SK_TRUE;
#ifndef SK_SLIM
#ifdef SK_PHY_LP_MODE
	pAC->GIni.GIGotoD3Cold = SK_FALSE;
#endif
	pAC->GIni.GIJumTcpSegSup = SK_FALSE;
	pAC->GIni.GITxIdxRepThres = 10;
	pAC->GIni.GINumOfRssKeys = 0;
#endif /* !SK_SLIM */
	for (i = 0; i < 4; i++) {
		pAC->GIni.HwF.Features[i] = 0x00000000;
		pAC->GIni.HwF.OnMask[i]   = 0x00000000;
		pAC->GIni.HwF.OffMask[i]  = 0x00000000;
	}

}	/* SkGeInit0 */


/******************************************************************************
 *
 *	SkGeSetUpSupFeatures() - Collect Feature List for HW_FEATURE Macro
 *
 * Description:
 *	This function collects the available features and required deviation services
 *	of the adapter and provides these information in the GIHwF struct.
 *	This information is used as default value and may be overwritten by the driver
 *	using the SET_HW_FEATURE_MASK() macro in its Init0 phase.
 *
 * Notice:
 *	Using the On and Off Mask:
 *	Never switch on the same bit in both masks simultaneously.
 *	However, if doing the Off Mask will win.
 *
 * Returns:
 *	nothing
 */
static void SkGeSetUpSupFeatures(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC)		/* I/O Context */
{
	int		i;
	int		LedCfg;
	SK_U16	Word;
#ifndef SK_SLIM
	SK_APP_CONF		*pCfgVal;
	SK_APP_CONFIG	*pCfgData;
#endif

	switch (pAC->GIni.GIChipId) {
	case CHIP_ID_YUKON_EC:
#ifndef SK_SLIM
		pAC->GIni.GINumOfRssKeys = 4;
		pAC->GIni.GIJumTcpSegSup = SK_TRUE;
#endif
		if (pAC->GIni.GIChipRev == CHIP_REV_YU_EC_A1) {
			/* A0/A1 */
			pAC->GIni.HwF.Features[HW_DEV_LIST] =
				HWF_WA_DEV_42  | HWF_WA_DEV_46 | HWF_WA_DEV_43_418 |
				HWF_WA_DEV_420 | HWF_WA_DEV_423;
		}
		/* A0 - A3 */
		pAC->GIni.HwF.Features[HW_DEV_LIST] |= HWF_WA_DEV_424 |
			HWF_WA_DEV_425 | HWF_WA_DEV_427 | HWF_WA_DEV_428 |
			HWF_WA_DEV_463 | HWF_WA_DEV_483 |
			HWF_WA_DEV_4152 | HWF_WA_DEV_4167 | HWF_WA_DEV_4222;
		pAC->GIni.HwF.Features[HW_DEV_LIST_2] = HWF_WA_DEV_4216;
		break;

	case CHIP_ID_YUKON_FE:
#ifndef SK_SLIM
		pAC->GIni.GINumOfRssKeys = 4;
#endif
		pAC->GIni.HwF.Features[HW_DEV_LIST] =
			HWF_WA_DEV_427  | HWF_WA_DEV_463  |
			HWF_WA_DEV_4152 | HWF_WA_DEV_4167 | HWF_WA_DEV_4222;
		pAC->GIni.HwF.Features[HW_DEV_LIST_2] = HWF_WA_DEV_4216;
		break;

	case CHIP_ID_YUKON_XL:
#ifndef SK_SLIM
		pAC->GIni.GINumOfRssKeys = 4;
		pAC->GIni.GIJumTcpSegSup = SK_TRUE;
#endif
		pAC->GIni.HwF.Features[HW_DEV_LIST] = HWF_WA_DEV_427 | HWF_WA_DEV_483;

		switch (pAC->GIni.GIChipRev) {

		case CHIP_REV_YU_XL_A0:		/* still needed for Diag */
			pAC->GIni.HwF.Features[HW_DEV_LIST] |=
				HWF_WA_DEV_463  | HWF_WA_DEV_472  | HWF_WA_DEV_479 |
				HWF_WA_DEV_4115 | HWF_WA_DEV_4152 | HWF_WA_DEV_4167;
			break;

		case CHIP_REV_YU_XL_A1:
			pAC->GIni.HwF.Features[HW_DEV_LIST] |=
				HWF_WA_DEV_4115 | HWF_WA_DEV_4152 |
				HWF_WA_DEV_4167;
			break;

		case CHIP_REV_YU_XL_A2:
			pAC->GIni.HwF.Features[HW_DEV_LIST] |=
				HWF_WA_DEV_4115 | HWF_WA_DEV_4167;
			break;

		case CHIP_REV_YU_XL_A3:
			pAC->GIni.HwF.Features[HW_DEV_LIST] |= HWF_WA_DEV_4115;
			break;
		}
		if (pAC->GIni.GIPciBus == SK_PEX_BUS) {
			pAC->GIni.HwF.Features[HW_DEV_LIST] |= HWF_WA_DEV_4222;
		}
		pAC->GIni.HwF.Features[HW_DEV_LIST_2] = HWF_WA_DEV_4216;
		break;

	case CHIP_ID_YUKON_EC_U:
#ifndef SK_SLIM
		pAC->GIni.GINumOfRssKeys = 4;
#endif
		if (pAC->GIni.GIChipRev == CHIP_REV_YU_EC_U_A0) {
			pAC->GIni.HwF.Features[HW_DEV_LIST] =
				HWF_WA_DEV_427 | HWF_WA_DEV_483 |
				HWF_WA_DEV_4217;
		}
		else if (pAC->GIni.GIChipRev == CHIP_REV_YU_EC_U_A1) {
			pAC->GIni.HwF.Features[HW_DEV_LIST] =
				HWF_WA_DEV_427 | HWF_WA_DEV_4185 | HWF_WA_DEV_4217;

			/* check for Rev. A1 */
			SK_IN16(IoC, Q_ADDR(Q_XA1, Q_WM), &Word);

			if (Word == 0) {
				pAC->GIni.HwF.Features[HW_DEV_LIST] |=
					HWF_WA_DEV_4185CS | HWF_WA_DEV_4200;
			}
		}
		else {	/* >= CHIP_REV_YU_EC_U_B0 */
			pAC->GIni.HwF.Features[HW_DEV_LIST] =
				HWF_WA_DEV_427 | HWF_WA_DEV_4217;
			pAC->GIni.HwF.Features[HW_FEAT_LIST] = HWF_ENA_POW_SAV_W_WOL;

			if (pAC->GIni.GIChipRev == CHIP_REV_YU_EC_U_B1) {
				pAC->GIni.HwF.Features[HW_FEAT_LIST] |= HWF_NEW_FLOW_CONTROL;
			}
		}
		pAC->GIni.HwF.Features[HW_DEV_LIST] |= HWF_WA_DEV_4222;
		pAC->GIni.HwF.Features[HW_DEV_LIST_2] |= HWF_WA_DEV_4216;
		break;

	case CHIP_ID_YUKON_EX:
#ifndef SK_SLIM
		pAC->GIni.GINumOfRssKeys = 10;
#endif
		if (pAC->GIni.GIChipRev == CHIP_REV_YU_EX_A0) {
			pAC->GIni.HwF.Features[HW_DEV_LIST] =
				HWF_WA_DEV_4217 | HWF_WA_DEV_LIM_IPV6_RSS | HWF_WA_DEV_53 |
				HWF_WA_DEV_54 | HWF_WA_DEV_56 | HWF_WA_DEV_4222;
			pAC->GIni.HwF.Features[HW_DEV_LIST_2] =
				HWF_WA_DEV_51 | HWF_WA_DEV_4216 | HWF_WA_DEV_515 |
				HWF_WA_DEV_517 | HWF_WA_DEV_519;
		}
		else {
			if (pAC->GIni.GIChipRev == CHIP_REV_YU_EX_B0) {
				pAC->GIni.HwF.Features[HW_DEV_LIST_2] =
					HWF_WA_DEV_510 | HWF_WA_DEV_511 | HWF_WA_DEV_515 |
					HWF_WA_DEV_517 | HWF_WA_DEV_519;
			}
			pAC->GIni.HwF.Features[HW_FEAT_LIST] =
				HWF_ADV_CSUM_SUPPORT | HWF_PSM_SUPPORTED |
				HWF_ENA_POW_SAV_W_WOL | HWF_NEW_FLOW_CONTROL;

#ifndef SK_SLIM
			pAC->GIni.GIJumTcpSegSup = SK_TRUE;
#endif
		}
		pAC->GIni.HwF.Features[HW_DEV_LIST_2] |=
			HWF_WA_DEV_4229 | HWF_WA_DEV_548;
		break;

	case CHIP_ID_YUKON_FE_P:
#ifndef SK_SLIM
		pAC->GIni.GINumOfRssKeys = 10;
#endif
		pAC->GIni.HwF.Features[HW_FEAT_LIST] =
			HWF_ADV_CSUM_SUPPORT | HWF_PSM_SUPPORTED | HWF_ENA_POW_SAV_W_WOL;

		if (pAC->GIni.GIChipRev == CHIP_REV_YU_FE2_A0) {
			pAC->GIni.HwF.Features[HW_DEV_LIST_2] =
				HWF_WA_DEV_4229 | HWF_WA_DEV_520 | HWF_WA_DEV_521 |
				HWF_WA_DEV_547;
		}
		pAC->GIni.HwF.Features[HW_DEV_LIST_2] |= HWF_WA_DEV_548;
		break;

	case CHIP_ID_YUKON_SUPR:
#ifndef SK_SLIM
		pAC->GIni.GINumOfRssKeys = 10;
		pAC->GIni.GIJumTcpSegSup = SK_TRUE;
#endif
		pAC->GIni.HwF.Features[HW_FEAT_LIST] =
			HWF_ADV_CSUM_SUPPORT | HWF_PSM_SUPPORTED | HWF_ENA_POW_SAV_W_WOL;
		if (pAC->GIni.GIChipRev == CHIP_REV_YU_SU_A0) {
			pAC->GIni.HwF.Features[HW_DEV_LIST_2] =
				HWF_WA_DEV_4229 | HWF_WA_DEV_542 | HWF_WA_DEV_548 |
				HWF_WA_DEV_563;
		}
		break;

	case CHIP_ID_YUKON_UL_2:
#ifndef SK_SLIM
		pAC->GIni.GINumOfRssKeys = 4;
		pAC->GIni.GIJumTcpSegSup = SK_TRUE;
#endif
		pAC->GIni.HwF.Features[HW_DEV_LIST] = HWF_WA_DEV_4217 | HWF_WA_DEV_4222;
		pAC->GIni.HwF.Features[HW_FEAT_LIST] =
			HWF_PSM_SUPPORTED | HWF_ENA_POW_SAV_W_WOL;
		pAC->GIni.HwF.Features[HW_DEV_LIST_2] = HWF_WA_DEV_4216;
		break;

	case CHIP_ID_YUKON_OPT:
	case CHIP_ID_YUKON_PRM:
	case CHIP_ID_YUKON_OP_2:
#ifndef SK_SLIM
		pAC->GIni.GINumOfRssKeys = 10;
		pAC->GIni.GIJumTcpSegSup = SK_TRUE;
#endif
		pAC->GIni.HwF.Features[HW_DEV_LIST] = HWF_WA_DEV_4217 | HWF_WA_DEV_4222;
		pAC->GIni.HwF.Features[HW_FEAT_LIST] =
			HWF_ADV_CSUM_SUPPORT | HWF_PSM_SUPPORTED | HWF_ENA_POW_SAV_W_WOL;
		if (pAC->GIni.GIChipId >= CHIP_ID_YUKON_PRM) {
			pAC->GIni.HwF.Features[HW_FEAT_LIST] |= HWF_DO_EEE_ENABLE;
		}

		pAC->GIni.HwF.Features[HW_DEV_LIST_2] = HWF_WA_DEV_4216;
		if (pAC->GIni.GIChipId == CHIP_ID_YUKON_OPT ||
			pAC->GIni.GIChipId == CHIP_ID_YUKON_PRM) {
			pAC->GIni.HwF.Features[HW_DEV_LIST_2] |= HWF_WA_DEV_563;
		}
		break;
	}

	pAC->GIni.HwF.Features[HW_DEV_LIST] |= HWF_WA_DEV_4109;

	/* legacy way for LED scheme */
	LedCfg = CFG_LED_MODE(pAC->GIni.GIHwResInfo);

#ifndef SK_SLIM
	pCfgVal  = &pAC->GIni.CfgVal;
	pCfgData = &pAC->GIni.CfgData;

	/* get the Application Configuration Value */
	SK_IN32(IoC, B2_MAC_3, &pCfgVal->ApplicConfigVal);

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_INIT,
		("Application Configuration: %08lx\n",
		 pCfgVal->ApplicConfigVal));

	pCfgData->ValidConfiguration = (SK_BOOL)pCfgVal->fields.ValidConfig;

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_INIT,
		("Application Config HI: %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u\n",
		 pCfgVal->bits.D31, pCfgVal->bits.D30, pCfgVal->bits.D29, pCfgVal->bits.D28,
		 pCfgVal->bits.D27, pCfgVal->bits.D26, pCfgVal->bits.D25, pCfgVal->bits.D24,
		 pCfgVal->bits.D23, pCfgVal->bits.D22, pCfgVal->bits.D21, pCfgVal->bits.D20,
		 pCfgVal->bits.D19, pCfgVal->bits.D18, pCfgVal->bits.D17, pCfgVal->bits.D16));

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_INIT,
		("Application Config LO: %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u\n",
		 pCfgVal->bits.D15, pCfgVal->bits.D14, pCfgVal->bits.D13, pCfgVal->bits.D12,
		 pCfgVal->bits.D11, pCfgVal->bits.D10, pCfgVal->bits.D09, pCfgVal->bits.D08,
		 pCfgVal->bits.D07, pCfgVal->bits.D06, pCfgVal->bits.D05, pCfgVal->bits.D04,
		 pCfgVal->bits.D03, pCfgVal->bits.D02, pCfgVal->bits.D01, pCfgVal->bits.D00));

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_INIT,
		("Application Configuration Valid: %u (LED #%u)\n",
		 pCfgVal->fields.ValidConfig,
		 pCfgVal->fields.LED_Scheme));

	if (pCfgData->ValidConfiguration) {
		/* set the Application Configuration structure */
		pCfgData->WakeUpSpeedSel	= pCfgVal->fields.WakeUpSpeed;
		pCfgData->EnergySaveScheme	= pCfgVal->fields.EnergyScheme;
		pCfgData->LED_SchemeSel		= pCfgVal->fields.LED_Scheme;
		pCfgData->AutoNegSchemeSel	= pCfgVal->fields.AutoNegScheme;

		pCfgData->DynClockGatingEn	= (SK_BOOL)pCfgVal->fields.DynClockGating;
		pCfgData->SmartSpeedDownEn	= (SK_BOOL)pCfgVal->fields.SmartSpeed;
		pCfgData->MonitorSpeedStepEn	= (SK_BOOL)pCfgVal->fields.MonSpeedStep;
		pCfgData->HW_WakeOnLAN_En		= (SK_BOOL)pCfgVal->fields.HW_WoLAN;
		pCfgData->WakeFromShutdownEn	= (SK_BOOL)pCfgVal->fields.WakeFromS5;
		pCfgData->MemoryTypeInternal	= (SK_BOOL)pCfgVal->fields.MemoryType;

#ifndef SK_DIAG
		/* configure HW-Features accordingly */
		if (pCfgData->DynClockGatingEn &&
			pAC->GIni.GIChipId != CHIP_ID_YUKON_FE_P) {
			pAC->GIni.HwF.Features[HW_FEAT_LIST] |= HWF_D0_CLK_GAT_ENABLE;
		}

		if (pCfgData->HW_WakeOnLAN_En) {
			pAC->GIni.HwF.Features[HW_FEAT_LIST] |= HWF_HW_WOL_ENABLE;
		}

		if (pCfgData->AutoNegSchemeSel > 0) {
			pAC->GIni.HwF.Features[HW_FEAT_LIST] |= HWF_FORCE_AUTO_NEG;
		}

		/* set PHY Reverse Auto-negotiation flag for Yukon-Optima and above */
		if (pCfgData->WakeUpSpeedSel > 1 &&
			pAC->GIni.GIChipId >= CHIP_ID_YUKON_OPT) {
			pAC->GIni.GP[MAC_1].PRevAutoNeg = SK_TRUE;
		}
#endif /* !SK_DIAG */

		/* new configuration for LED scheme */
		LedCfg = pCfgData->LED_SchemeSel;
	}
#endif /* !SK_SLIM */

	if (LedCfg == CFG_LED_DUAL_ACT_LNK) {
		/* Dual LED ACT/LNK */
		pAC->GIni.GILedBlinkCtrl |= SK_DUAL_LED_ACT_LNK;
	}
	else if (LedCfg == CFG_LED_LINK_MUX_P60) {
		/* LED pin 60 used differently */
		pAC->GIni.GILedBlinkCtrl |= SK_LED_LINK_MUX_P60;
	}
	else if (LedCfg == CFG_LED_ACT_OFF_NOTR) {
		/* Activity LED off if no traffic */
		pAC->GIni.GILedBlinkCtrl |= SK_ACT_LED_NOTR_OFF;
	}
	else if (LedCfg == CFG_LED_COMB_ACT_LNK) {
		/* Combined Activity/Link LED mode */
		pAC->GIni.GILedBlinkCtrl |= SK_LED_COMB_ACT_LNK;
	}
	else if (LedCfg == CFG_LED_HP_SPC_4_LED) {
		/* HP special 4 LED mode */
		pAC->GIni.GILedBlinkCtrl |= SK_LED_COMB_ACT_LNK | SK_ACT_LED_NOTR_OFF;
	}

	/* filter the HW Features using the On and Off Mask */
	for (i = 0; i < 4; i++) {
		pAC->GIni.HwF.Features[i] =
			(pAC->GIni.HwF.Features[i] | pAC->GIni.HwF.OnMask[i]) &
				~pAC->GIni.HwF.OffMask[i];
	}
}	/* SkGeSetUpSupFeatures */


/******************************************************************************
 *
 *	SkGeInit1() - Level 1 Initialization
 *
 * Description:
 *	o Do a software reset.
 *	o Clear all reset bits.
 *	o Verify that the detected hardware is present.
 *	  Return an error if not.
 *	o Get the hardware configuration
 *		+ Read the number of MACs/Ports.
 *		+ Read the RAM size.
 *		+ Read the PCI Revision Id.
 *		+ Find out the adapters host clock speed
 *		+ Read and check the PHY type
 *
 * Returns:
 *	0:	success
 *	5:	Unexpected Chip Id detected
 *	6:	HW self test failed
 */
static int SkGeInit1(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC)		/* I/O Context */
{
	SK_U8	Byte;
	SK_U8	ChipId;
	SK_U8	ChipRev;
	SK_U8	MacCfg;
	SK_U16	Word;
	SK_U32	CtrlStat;
#ifndef SK_SLIM
	SK_U32	VauxAvail;
#endif /* !SK_SLIM */
	SK_U32	DWord;
	SK_U32	Our1;
#ifndef SK_DIAG
	SK_U32	Our5;
#endif /* !SK_DIAG */
	SK_U32	PowerDownBit;
	SK_BOOL	FiberType;
	SK_GEPORT *pPrt;
	int		RetVal;
	int		i, j;
	int		mem_mapped;

	RetVal = 0;
	ChipId = 0;

#ifndef VCPU
#if (defined(SK_SLIM) || (defined(SK_DIAG) && !defined(SK_MEM_MAPPED_IO)))
	mem_mapped = 0;
#else	/* !SK_SLIM && (!SK_DIAG || SK_MEM_MAPPED_IO) */
	mem_mapped = 1;
#endif	/* !SK_SLIM && (!SK_DIAG || SK_MEM_MAPPED_IO) */
#else	/* VCPU */
	mem_mapped = pAC->sw.mem_mapped;
#endif	/* VCPU */

	if (!mem_mapped) {
#if !defined(SK_ASF) && !defined(MV_INCLUDE_SDK_SUPPORT)
		/* for chips before Yukon-Optima 2 */
		SkPciWriteCfgDWord(pAC, PCI_OUR_REG_3 % 0x300, 0);

		if (0) {
			/* for Yukon-Optima 2 and later */
			SkPciWriteCfgDWord(pAC, PCI_OUR_REG_3, 0);
		}
#endif /* !SK_ASF && !MV_INCLUDE_SDK_SUPPORT */
	}
	else {
		/* get MAC Config. / Chip Ident. & Revision */
		SK_IN16(IoC, B2_MAC_CFG, &Word);

		ChipId = (SK_U8)(Word >> 8);

		/* save Chip Identification */
		pAC->GIni.GIChipId = ChipId;

		if (ChipId >= CHIP_ID_YUKON_EC_U) {
			/* Enable all clocks, otherwise the system will hang */
			/* Do not use PCI_C here */
			SK_OUT32(IoC, Y2_CFG_SPC + PCI_OUR_REG_3 % 0x300, 0);
		}
	}

	/* save CLK_RUN & ASF_ENABLE bits (YUKON-Lite, YUKON-EC) */
	SK_IN32(IoC, B0_CTST, &CtrlStat);

	/* release the SW-reset */
	/* Important: SW-reset has to be cleared here, to ensure
	 * the CHIP_ID can be read I/O-mapped based, too -
	 * remember the RAP register can only be written if SW-reset is cleared.
	 */
	SK_OUT8(IoC, B0_CTST, CS_RST_CLR);

	if (!mem_mapped) {
		/* get MAC Config. / Chip Ident. & Revision */
		SK_IN16(IoC, B2_MAC_CFG, &Word);

		ChipId = (SK_U8)(Word >> 8);

		/* save Chip Identification */
		pAC->GIni.GIChipId = ChipId;
	}

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("MAC_CFG: 0x%04X, B0_CTST: 0x%08lX\n", Word, CtrlStat));

	pAC->GIni.GIAsfEnabled = SK_FALSE;

	/* get MAC Configuration / Chip Revision Number */
	MacCfg = (SK_U8)Word;

	/* get Chip Revision Number */
	ChipRev = (MacCfg & CFG_CHIP_R_MSK) >> 4;

	/* save Chip Revision */
	pAC->GIni.GIChipRev = ChipRev;

	/* ASF support only for Yukon-2 */
	if (ChipId >= CHIP_ID_YUKON_XL && ChipId <= CHIP_ID_YUKON_SUPR) {

#if defined(SK_ASF) || defined(SK_SLIM) || defined(MV_INCLUDE_SDK_SUPPORT)
		if ((CtrlStat & Y2_ASF_ENABLE) != 0) {
			/* do the SW-reset only if ASF is not enabled */
			pAC->GIni.GIAsfEnabled = SK_TRUE;
		}
#else /* !SK_ASF && !SK_SLIM && !MV_INCLUDE_SDK_SUPPORT */

		/* put ASF system in reset state */
		if (ChipId == CHIP_ID_YUKON_EX || ChipId == CHIP_ID_YUKON_SUPR) {

			/* stop the watchdog */
			SK_OUT32(IoC, CPU_WDOG, 0);

			/* Do not touch bit 5..3 */
			SK_IN16(IoC, B28_Y2_ASF_STAT_CMD, &Word);

			pAC->GIni.GIAsfRunning =
				((Word & HCU_CCSR_UC_STATE_MSK) == HCU_CCSR_ASF_RUNNING);

			Word &= ~(HCU_CCSR_AHB_RST | HCU_CCSR_CPU_RST_MODE |
				HCU_CCSR_UC_STATE_MSK);

			/*
			 * CPU clock divider shouldn't be used because
			 * - ASF firmware may malfunction
			 * - Yukon-Supreme: Parallel FLASH doesn't support divided clocks
			 */
			if ((Word & HCU_CCSR_CPU_CLK_DIVIDE_MSK) != 0) {
				SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_ERR,
					("CPU Clock Divider bits are set (0x%x), cleared now\n", Word));
				Word &= ~HCU_CCSR_CPU_CLK_DIVIDE_MSK;
			}

			SK_OUT16(IoC, B28_Y2_ASF_STAT_CMD, Word);
			/* stop the watchdog */
			SK_OUT32(IoC, CPU_WDOG, 0);
		}
		else {	/* Yukon-EC / Yukon-2 */
			SK_IN8(IoC, B28_Y2_ASF_STAT_CMD, &Byte);

			pAC->GIni.GIAsfRunning = Byte & Y2_ASF_RUNNING;

			SK_OUT8(IoC, B28_Y2_ASF_STAT_CMD, (SK_U8)Y2_ASF_RESET);
		}

		/* disable ASF Unit */
		SK_OUT16(IoC, B0_CTST, Y2_ASF_DISABLE);
#endif /* !SK_ASF && !SK_SLIM && !MV_INCLUDE_SDK_SUPPORT */
	}

	if (!pAC->GIni.GIAsfEnabled) {
		/* Yukon-2: required for Diag and Power Management */
		/* set the SW-reset */
		SK_OUT8(IoC, B0_CTST, CS_RST_SET);

		/* release the SW-reset */
		SK_OUT8(IoC, B0_CTST, CS_RST_CLR);
	}

	/* enable Config Write */
	SK_TST_MODE_ON(IoC);

	if (1) {
		/* reset all error bits in the PCI STATUS register */
		/*
		 * Note: PCI Cfg cycles cannot be used, because they are not
		 *		 available on some platforms after 'boot time'.
		 */
		SK_IN16(IoC, PCI_C2(pAC, IoC, PCI_STATUS), &Word);
		SK_OUT16(IoC, PCI_C2(pAC, IoC, PCI_STATUS), Word | (SK_U16)PCI_ERRBITS);
	}
	else {
		SkPciReadCfgWord(pAC, PCI_STATUS, &Word);
		SkPciWriteCfgWord(pAC, PCI_STATUS, Word | (SK_U16)PCI_ERRBITS);
	}

	/* release Master Reset */
	SK_OUT8(IoC, B0_CTST, CS_MRST_CLR);

#ifdef CLK_RUN
	CtrlStat |= CS_CLK_RUN_ENA;

	/* restore CLK_RUN bits */
	SK_OUT16(IoC, B0_CTST, (SK_U16)(CtrlStat &
		(CS_CLK_RUN_HOT | CS_CLK_RUN_RST | CS_CLK_RUN_ENA)));
#endif /* CLK_RUN */

	if (ChipId >= CHIP_ID_YUKON_XL && ChipId <= CHIP_ID_YUKON_OP_2) {

		pAC->GIni.GIYukon2 = SK_TRUE;
		pAC->GIni.GIValIrqMask = Y2_IS_ALL_MSK;

		pAC->GIni.GIValHwIrqMask = Y2_HWE_ALL_MSK;

#ifdef SK_AVB
		if (HW_SUP_PTP(pAC)) {
			pAC->GIni.GIValIrqMask |= Y2_IS_PTP_TIST;
		}
#endif /* SK_AVB */

#ifndef SK_SLIM
		VauxAvail = Y2_VAUX_AVAIL;

		/* check if VMAIN is available */
		if ((CtrlStat & Y2_VMAIN_AVAIL) == 0) {
			pAC->GIni.GIVMainAvail = SK_FALSE;
		}
#endif /* !SK_SLIM */

		/* needed before using PEX_CAP_REGS() macro */
		if ((ChipId == CHIP_ID_YUKON_EX && ChipRev != CHIP_REV_YU_EX_A0) ||
			ChipId >= CHIP_ID_YUKON_FE_P) {

			pAC->GIni.GIPexCapOffs = PEX_CAP_REG_OFFS;
		}

		if (1) {
			SK_IN32(IoC, PCI_C2(pAC, IoC, PCI_OUR_STATUS), &DWord);
		}
		else {
			SkPciReadCfgDWord(pAC, PCI_OUR_STATUS, &DWord);
		}

		if ((DWord & PCI_OS_PCI_X) != 0) {
#ifndef SK_SLIM
			/* this is a PCI / PCI-X bus */
			if ((DWord & PCI_OS_PCIX) != 0) {
				/* this is a PCI-X bus */
				pAC->GIni.GIPciBus = SK_PCIX_BUS;

				/* PCI-X is always 64-bit wide */
				pAC->GIni.GIPciSlot64 = SK_TRUE;

				pAC->GIni.GIPciMode = (SK_U8)(PCI_OS_SPEED(DWord));
			}
			else {
				/* this is a conventional PCI bus */
				pAC->GIni.GIPciBus = SK_PCI_BUS;

				SK_IN16(IoC, PCI_C2(pAC, IoC, PCI_OUR_REG_2), &Word);

				/* check if 64-bit width is used */
				pAC->GIni.GIPciSlot64 = (SK_BOOL)
					(((DWord & PCI_OS_PCI64B) != 0) &&
					((Word & PCI_USEDATA64) != 0));

				/* check if 66 MHz PCI Clock is active */
				pAC->GIni.GIPciClock66 = (SK_BOOL)((DWord & PCI_OS_PCI66M) != 0);
			}
#endif /* !SK_SLIM */
		}
		else {
			/* this is a PEX bus */
			pAC->GIni.GIPciBus = SK_PEX_BUS;

			/* clear any PEX errors */
			if (1) {
				SK_OUT32(IoC, PCI_C2(pAC, IoC, PEX_UNC_ERR_STAT), ALL_32_BITS);
				SK_IN32(IoC, PCI_C2(pAC, IoC, PEX_UNC_ERR_STAT), &DWord);
			}
			else {
				SkPciWriteCfgDWord(pAC, PEX_UNC_ERR_STAT, ALL_32_BITS);
				SkPciReadCfgDWord(pAC, PEX_UNC_ERR_STAT, &DWord);
			}

			if ((DWord & PEX_RX_OV) != 0) {
				/* Dev. #4.205 occurred */
				pAC->GIni.GIValHwIrqMask &= ~Y2_IS_PCI_EXP;
				pAC->GIni.GIValIrqMask &= ~Y2_IS_HW_ERR;
			}

			if (1) {
				SK_IN16(IoC, PCI_C2(pAC, IoC, PEX_CAP_REGS(PEX_LNK_STAT)), &Word);
			}
			else {
				SkPciReadCfgWord(pAC, PEX_CAP_REGS(PEX_LNK_STAT), &Word);
			}

			pAC->GIni.GIPexWidth = (SK_U8)((Word & PEX_LS_LINK_WI_MSK) >> 4);

			/* read PEX Link Control */
			if (1) {
				SK_IN16(IoC, PCI_C2(pAC, IoC, PEX_CAP_REGS(PEX_LNK_CTRL)),
					&pAC->GIni.GIPexLinkCtrl);
			}
			else {
				SkPciReadCfgWord(pAC, PEX_CAP_REGS(PEX_LNK_CTRL),
					&pAC->GIni.GIPexLinkCtrl);
			}
		}
		/*
		 * Yukon-2 chips family has a different way of providing
		 * the number of MACs available
		 */
		pAC->GIni.GIMacsFound = 1;

		/* get HW Resources */
		SK_IN8(IoC, B2_Y2_HW_RES, &Byte);

		if (CHIP_ID_YUKON_2(pAC)) {
			/*
			 * OEM config value is overwritten and should not
			 * be used for Yukon-2
			 */
			pAC->GIni.GILedBlinkCtrl |= SK_ACT_LED_BLINK;

#ifndef SK_SLIM
			if (ChipId == CHIP_ID_YUKON_EC_U ||
				ChipId == CHIP_ID_YUKON_EX ||
				ChipId >= CHIP_ID_YUKON_FE_P) {
				/* LED Configuration is stored in GP_IO */
				SK_IN8(IoC, B2_GP_IO, &Byte);
			}
#endif /* !SK_SLIM */
		}

		/* save HW Resources / Application Information */
		pAC->GIni.GIHwResInfo = Byte;

		if ((Byte & CFG_DUAL_MAC_MSK) == CFG_DUAL_MAC_MSK) {

			SK_IN8(IoC, B2_Y2_CLK_GATE, &Byte);

			if (!(Byte & Y2_STATUS_LNK2_INAC)) {
				/* Link 2 activ */
				pAC->GIni.GIMacsFound++;
			}
		}

#ifdef VCPU
		if (ChipId == CHIP_ID_YUKON_XL) {
			/* temporary WA for reported number of links */
			pAC->GIni.GIMacsFound = 2;
		}
#endif /* VCPU */

		if (pAC->GIni.GIMacsFound > 1) {
			/* allow IRQ from Port 2 only on dual port adapters */
			pAC->GIni.GIValIrqMask |= Y2_IS_L2_MASK;
		}

		pAC->GIni.GIChipCap = MacCfg & 0x0f;
	}
#ifndef DISABLE_YUKON_I
	else {
		pAC->GIni.GIYukon2 = SK_FALSE;
		pAC->GIni.GIValIrqMask = IS_ALL_MSK;
		pAC->GIni.GIValHwIrqMask = 0;	/* not activated */

#ifndef SK_SLIM
		VauxAvail = CS_VAUX_AVAIL;
#endif /* !SK_SLIM */

		pAC->GIni.GIMacsFound = (MacCfg & CFG_SNG_MAC) ? 1 : 2;
	}
#endif /* !DISABLE_YUKON_I */

#ifndef SK_DIAG
	if (ChipId == CHIP_ID_YUKON_XL && ChipRev == CHIP_REV_YU_XL_A0) {
		/* Yukon-2 Chip Rev. A0 */
		return(6);
	}
#endif /* !SK_DIAG */

	/* read the adapters RAM size */
	SK_IN8(IoC, B2_E_0, &Byte);

	pAC->GIni.GIYukonLite = SK_FALSE;
	pAC->GIni.GIVauxAvail = SK_FALSE;

	if (ChipId < CHIP_ID_YUKON || ChipId > CHIP_ID_YUKON_OP_2) {
		/* unexpected Chip Id */
		return(5);
	}

	pAC->GIni.GIRamSize = (Byte == (SK_U8)0) ? 128 : (int)Byte * 4;

#ifndef SK_SLIM
	pAC->GIni.GIRamOffs = 0;

	/* WA for Yukon chip Rev. A0 */
	pAC->GIni.GIWolOffs = (ChipId == CHIP_ID_YUKON && ChipRev == 0) ?
		WOL_REG_OFFS : 0;

	/* get PM Capabilities of PCI config space */
	if (1) {
		SK_IN16(IoC, PCI_C2(pAC, IoC, PCI_PM_CAP_REG), &Word);
	}
	else {
		SkPciReadCfgWord(pAC, PCI_PM_CAP_REG, &Word);
	}

	/* check if VAUX is available */
	if (((CtrlStat & VauxAvail) != 0) &&
		/* check also if PME from D3cold is set */
		((Word & PCI_PME_D3C_SUP) != 0)) {
		/* set entry in GE init struct */
		pAC->GIni.GIVauxAvail = SK_TRUE;
	}
#endif /* !SK_SLIM */

	if (!CHIP_ID_YUKON_2(pAC)) {

		if (ChipId == CHIP_ID_YUKON_LITE) {
			/* this is Rev. A1 */
			pAC->GIni.GIYukonLite = SK_TRUE;
		}
#ifndef SK_SLIM
		else {
			/* save Flash-Address Register */
			SK_IN32(IoC, B2_FAR, &DWord);

			/* test Flash-Address Register */
			SK_OUT8(IoC, B2_FAR + 3, 0xff);
			SK_IN8(IoC, B2_FAR + 3, &Byte);

			if (Byte != 0) {
				/* this is Rev. A0 */
				pAC->GIni.GIYukonLite = SK_TRUE;

				/* restore Flash-Address Register */
				SK_OUT32(IoC, B2_FAR, DWord);
			}
		}
#endif /* !SK_SLIM */
	}
#ifndef DISABLE_YUKON_I
	else {
		/* Check for CLS = 0 (dev. #4.55) */
		if (pAC->GIni.GIPciBus != SK_PEX_BUS) {
			/* PCI and PCI-X */
			SK_IN8(IoC, PCI_C2(pAC, IoC, PCI_CACHE_LSZ), &Byte);

			if (Byte == 0) {
				/* set CLS to 2 if configured to 0 */
				SK_OUT8(IoC, PCI_C2(pAC, IoC, PCI_CACHE_LSZ), 2);
			}

			if (pAC->GIni.GIPciBus == SK_PCIX_BUS) {
				/* set Cache Line Size opt. */
				SK_IN32(IoC, PCI_C2(pAC, IoC, PCI_OUR_REG_1), &DWord);
				DWord |= PCI_CLS_OPT;
				SK_OUT32(IoC, PCI_C2(pAC, IoC, PCI_OUR_REG_1), DWord);
			}
		}
	}
#endif /* !DISABLE_YUKON_I */

	/* GeCommon also used in ASF firmware: VMain may not be available */
	if (pAC->GIni.GIVMainAvail) {
		/* switch power to VCC (WA for VAUX problem) */
		SK_OUT8(IoC, B0_POWER_CTRL, (SK_U8)(PC_VAUX_ENA | PC_VCC_ENA |
			PC_VAUX_OFF | PC_VCC_ON));
#ifdef DEBUG
		SK_IN32(IoC, B0_CTST, &DWord);

		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
			("Ctrl/Stat & Switch: 0x%08lX\n", DWord));
#endif /* DEBUG */
	}

	Byte = 0;

	if (CHIP_ID_YUKON_2(pAC)) {
		switch (ChipId) {
		/* PEX adapters work with different host clock */
		case CHIP_ID_YUKON_EC:
		case CHIP_ID_YUKON_EC_U:
		case CHIP_ID_YUKON_EX:
		case CHIP_ID_YUKON_SUPR:
		case CHIP_ID_YUKON_UL_2:
		case CHIP_ID_YUKON_OPT:
		case CHIP_ID_YUKON_PRM:
		case CHIP_ID_YUKON_OP_2:
			/* Yukon-EC and later work with 125 MHz host clock */
			pAC->GIni.GIHstClkFact = SK_FACT_125;
			break;
		case CHIP_ID_YUKON_FE:
			/* Yukon-FE works with 100 MHz host clock */
			pAC->GIni.GIHstClkFact = SK_FACT_100;
			break;
		case CHIP_ID_YUKON_FE_P:
			/* Yukon-FE+ works with 50 MHz core clock */
			pAC->GIni.GIHstClkFact = SK_FACT_50;
			break;
		case CHIP_ID_YUKON_XL:
			/* all Yukon-2 adapters work with 156 MHz host clock */
			pAC->GIni.GIHstClkFact = 2 * SK_FACT_78;

			if (ChipRev > CHIP_REV_YU_XL_A1) {
				/* enable bits are inverted */
				Byte = (SK_U8)(Y2_PCI_CLK_LNK1_DIS | Y2_COR_CLK_LNK1_DIS |
					Y2_CLK_GAT_LNK1_DIS | Y2_PCI_CLK_LNK2_DIS |
					Y2_COR_CLK_LNK2_DIS | Y2_CLK_GAT_LNK2_DIS);
			}
			break;
		default:
			SK_ERR_LOG(pAC, SK_ERRCL_SW | SK_ERRCL_INIT, SKERR_HWI_E006,
				SKERR_HWI_E006MSG);
		}

		pAC->GIni.GIPollTimerVal =
			SK_DPOLL_DEF_Y2 * (SK_U32)pAC->GIni.GIHstClkFact / 100;

		/* set power down bit */
		PowerDownBit = PCI_Y2_PHY1_POWD | PCI_Y2_PHY2_POWD;

		/* disable Core Clock Division, set Clock Select to 0 (Yukon-2) */
		SK_OUT32(IoC, B2_Y2_CLK_CTRL, Y2_CLK_DIV_DIS);

		/* enable MAC/PHY, PCI and Core Clock for both Links */
		SK_OUT8(IoC, B2_Y2_CLK_GATE, Byte);
	}
#ifndef DISABLE_YUKON_I
	else {
		/* YUKON adapters work with 78 MHz host clock */
		pAC->GIni.GIHstClkFact = SK_FACT_78;

		pAC->GIni.GIPollTimerVal = SK_DPOLL_MAX;	/* 215 ms */

		/* read the Interrupt source */
		SK_IN32(IoC, B0_ISRC, &DWord);

		if ((DWord & IS_HW_ERR) != 0) {
			/* read the HW Error Interrupt source */
			SK_IN32(IoC, B0_HWE_ISRC, &DWord);

			if ((DWord & IS_IRQ_SENSOR) != 0) {
				/* disable HW Error IRQ */
				pAC->GIni.GIValIrqMask &= ~IS_HW_ERR;
			}
		}
		/* set power down bit */
		PowerDownBit = PCI_PHY_COMA;
	}
#endif /* !DISABLE_YUKON_I */

	/* call before using first time HW_FEATURE macro */
	SkGeSetUpSupFeatures(pAC, IoC);

	if (1) {
		SK_IN32(IoC, PCI_C2(pAC, IoC, PCI_OUR_REG_1), &Our1);
	}
	else {
		SkPciReadCfgDWord(pAC, PCI_OUR_REG_1, &Our1);
	}

	Our1 &= ~PowerDownBit;

	if (ChipId == CHIP_ID_YUKON_XL && ChipRev > CHIP_REV_YU_XL_A1) {
		/* deassert Low Power for 1st PHY */
		Our1 |= PCI_Y2_PHY1_COMA;

		if (pAC->GIni.GIMacsFound > 1) {
			/* deassert Low Power for 2nd PHY */
			Our1 |= PCI_Y2_PHY2_COMA;
		}
	}
	else if (ChipId == CHIP_ID_YUKON_EC_U ||
			 ChipId == CHIP_ID_YUKON_EX ||
			 ChipId >= CHIP_ID_YUKON_FE_P) {

		/* enable all clocks */
		/* PCI_OUR_REG_3 is set at the top of the function */

#ifndef SK_DIAG
		SK_IN32(IoC, PCI_C2(pAC, IoC, PCI_OUR_REG_5), &Our5);

		if (HW_FEATURE(pAC, HWF_D0_CLK_GAT_ENABLE)) {
			/* set also bit 13 as WA for dev.#5.60 */
			Our5 |= P_REL_EN_RX_DV;
		}
		else {
			/* disable clock gating */
			SK_IN32(IoC, PCI_C2(pAC, IoC, PCI_OUR_REG_4), &DWord);

#ifdef SK_ASPM_DISABLED
			DWord &= P_PIN63_LINK_LED_ENA;
#else /* !SK_ASPM_DISABLED */
			DWord &= P_ASPM_CONTROL_MSK;
#endif /* !SK_ASPM_DISABLED */

			/* set all bits to 0 except bits 15..12 and 8 */
			SK_OUT32(IoC, PCI_C2(pAC, IoC, PCI_OUR_REG_4), DWord);

			/* set all bits to 0 except bits 28 & 27 */
			Our5 &= P_CTL_TIM_VMAIN_AV_MSK;
		}

		SK_OUT32(IoC, PCI_C2(pAC, IoC, PCI_OUR_REG_5), Our5);
#endif /* !SK_DIAG */

		/* set all bits to 0 */
		if (1) {
			SK_OUT32(IoC, PCI_C2(pAC, IoC, PCI_CFG_REG_1), 0);
		}
		else {
			SkPciWriteCfgDWord(pAC, PCI_CFG_REG_1, 0);
		}

#if !defined(SK_SLIM) && !defined(SK_DIAG)
		if (!pAC->GIni.GIAsfEnabled) {
			/* enable HW WOL */
			SK_OUT16(IoC, B0_CTST, Y2_HW_WOL_ON);
		}
#endif /* !SK_SLIM && !SK_DIAG */

		/* Enable workaround for dev. #4.107 on Yukon-Ultra & Extreme */
		SK_IN32(IoC, B2_GP_IO, &DWord);

		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
			("PCI_OUR_REG_1: 0x%08lX, GP_IO: 0x%08lX\n", Our1, DWord));

		SK_OUT32(IoC, B2_GP_IO, DWord | GLB_GPIO_STAT_RACE_DIS);

		if (ChipId == CHIP_ID_YUKON_FE_P || ChipId >= CHIP_ID_YUKON_UL_2) {
			/* read Embedded NV Memory Loader Control Register */
			SK_IN16(IoC, PCI_C2(pAC, IoC, OTP_LDR_CTRL), &Word);

			/* check EEPROM timeout flag */
			if ((Word & OTP_LDR_CTRL_EEPROM_TIMEOUT_FLAG) == 0) {
				/* set TWSI timeout value to maximum */
				Word |= OTP_LDR_CTRL_EEPROM_TIMEOUT_VAL_MSK;

				SK_OUT16(IoC, PCI_C2(pAC, IoC, OTP_LDR_CTRL), Word);
			}
			else {
				/* disable EEPROM loader to avoid TWSI timeout */
				SK_IN16(IoC, PCI_C2(pAC, IoC, VPD_CTRL_ADD), &Word);

				if ((Word & VPD_CTRL_ADD_EEPROM_EN) != 0) {

					Word &= ~VPD_CTRL_ADD_EEPROM_EN;

					SK_OUT16(IoC, PCI_C2(pAC, IoC, VPD_CTRL_ADD), Word);
				}
			}
		}
	}

#ifdef DEBUG
	/* check for link through GPHY Control */
	SK_IN32(IoC, GPHY_CTRL, &DWord);

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("GPHY_CTRL: 0x%08lX\n", DWord));

	if ((DWord & GPC_RST_CLR) != 0 && (DWord & GPC_PHY_LINK_UP) != 0) {

		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
			("PHY Link Up\n"));
	}
#endif /* DEBUG */

	/* release PHY from PowerDown/COMA Mode */
	if (1) {
		SK_OUT32(IoC, PCI_C2(pAC, IoC, PCI_OUR_REG_1), Our1);
	}
	else {
		SkPciWriteCfgDWord(pAC, PCI_OUR_REG_1, Our1);
	}

	for (i = 0; i < pAC->GIni.GIMacsFound; i++) {

		if (!pAC->GIni.GIAsfEnabled && !pAC->GIni.GIDontInitPhy) {
			/* set Link Control reset */
			SK_OUT8(IoC, MR_ADDR(i, GMAC_LINK_CTRL), (SK_U8)GMLC_RST_SET);
		}

		/* clear Link Control reset */
		SK_OUT8(IoC, MR_ADDR(i, GMAC_LINK_CTRL), (SK_U8)GMLC_RST_CLR);
	}

	/* disable Config Write */
	SK_TST_MODE_OFF(IoC);

	/* get the Connector & PMD type */
	SK_IN16(IoC, B2_CONN_TYP, &Word);

#ifndef SK_SLIM
	/* save the Connector type */
	pAC->GIni.GIConTyp = (SK_U8)Word;

	if (!CHIP_ID_YUKON_2(pAC)) {
		/* this is a conventional PCI bus */
		pAC->GIni.GIPciBus = SK_PCI_BUS;

		/* check if 64-bit PCI Slot is present */
		pAC->GIni.GIPciSlot64 = (SK_BOOL)((CtrlStat & CS_BUS_SLOT_SZ) != 0);

		/* check if 66 MHz PCI Clock is active */
		pAC->GIni.GIPciClock66 = (SK_BOOL)((CtrlStat & CS_BUS_CLOCK) != 0);
	}

	/* read PCI HW Revision Id. */
	if (1) {
		SK_IN8(IoC, PCI_C2(pAC, IoC, PCI_REV_ID), &pAC->GIni.GIPciHwRev);
	}
	else {
		SkPciReadCfgByte(pAC, PCI_REV_ID, &pAC->GIni.GIPciHwRev);
	}
#endif /* !SK_SLIM */

	Byte = (SK_U8)(Word >> 8);

	/* save the PMD type */
	pAC->GIni.GIPmdTyp = Byte;

	FiberType = (Byte == 'L' || Byte == 'S' || Byte == 'P');

	pAC->GIni.GICopperType = (SK_BOOL)(Byte == 'T' || Byte == '1' ||
		(pAC->GIni.GIYukon2 && !FiberType));

	/* read the PHY type */
	SK_IN8(IoC, B2_E_1, &Byte);

	Byte &= 0x0f;	/* the PHY type is stored in the lower nibble */

	if (((Byte < (SK_U8)SK_PHY_MARV_COPPER) || pAC->GIni.GIYukon2) &&
		!FiberType) {
		/* if this field is not initialized */
		Byte = (SK_U8)SK_PHY_MARV_COPPER;

		pAC->GIni.GICopperType = SK_TRUE;
	}

	if (!pAC->GIni.GICopperType) {
		Byte = (SK_U8)SK_PHY_MARV_FIBER;
	}

	for (i = 0; i < pAC->GIni.GIMacsFound; i++) {
#ifdef VCPU
		SK_U32	MacAddr1;
		SK_U16	MacAddr2;

		SK_IN32(IoC, B2_MAC_1 + 8 * i, &MacAddr1);
		if (0 == MacAddr1) {
			SK_IN16(IoC, B2_MAC_1 + 8 * i + 4, &MacAddr2);
			if (0 == MacAddr2) {
				/* Enable Config Write */
				SK_TST_MODE_ON(IoC);

				SK_OUT32(IoC, B2_MAC_1 + 8 * i, 0x125A0000);
				SK_OUT16(IoC, B2_MAC_1 + 8 * i + 4, 0x5634 + 0x0100 * i);

				/* Disable Config Write. */
				SK_TST_MODE_OFF(IoC);
			}
		}
#endif /* VCPU */

		pPrt = &pAC->GIni.GP[i];

		pPrt->PhyType = (int)Byte;

		pPrt->PhyAddr = PHY_ADDR_MARV;

		/* get the MAC addresses */
		for (j = 0; j < 3; j++) {
			SK_IN16(IoC, B2_MAC_1 + i * 8 + j * 2, &pPrt->PMacAddr[j]);
		}

		if (pAC->GIni.GICopperType) {

			if (ChipId == CHIP_ID_YUKON_FE ||
				ChipId == CHIP_ID_YUKON_FE_P ||
				(ChipId == CHIP_ID_YUKON_EC && pAC->GIni.GIChipCap == 2)) {

				pPrt->PLinkSpeedCap = (SK_U8)(SK_LSPEED_CAP_100MBPS |
					SK_LSPEED_CAP_10MBPS);

				pAC->GIni.GIRamSize = 4;
			}
			else {
				pPrt->PLinkSpeedCap = (SK_U8)(SK_LSPEED_CAP_1000MBPS |
					SK_LSPEED_CAP_100MBPS | SK_LSPEED_CAP_10MBPS |
					SK_LSPEED_CAP_AUTO);
			}

			pPrt->PLinkSpeed = (SK_U8)SK_LSPEED_AUTO;

#ifndef SK_SLIM
			pPrt->PMSCap = (SK_U8)(SK_MS_CAP_AUTO |
				SK_MS_CAP_MASTER | SK_MS_CAP_SLAVE);
#endif
		}

		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_INIT,
			("PHY type: %d  PHY addr: %04x\n",
			 Byte, pPrt->PhyAddr));
	}

	if (ChipId <= CHIP_ID_YUKON_XL) {
		/* clear TWSI IRQ */
		SK_OUT32(IoC, B2_I2C_IRQ, I2C_CLR_IRQ);
	}

	/* get MAC Type & set function pointers dependent on */
#ifndef SK_SLIM
	pAC->GIni.GINumOfPattern = SK_NUM_WOL_PATTERN;
#endif
	switch (ChipId) {
	case CHIP_ID_YUKON_EX:
	case CHIP_ID_YUKON_SUPR:
		pAC->GIni.GIExtLeFormat = SK_TRUE;
#ifndef SK_SLIM
		if (ChipId == CHIP_ID_YUKON_EX) {
			pAC->GIni.GINumOfPattern += 2;
		}
		else {
			pAC->GIni.GINumOfPattern += 7;	/* Yukon-Supreme got 14 patterns */
		}
#endif /* !SK_SLIM */
		/* bypass MACSec unit */
		SK_OUT16(IoC, MAC_CTRL, MAC_CTRL_BYP_MACSECRX_ON |
			MAC_CTRL_BYP_MACSECTX_ON | MAC_CTRL_BYP_RETR_ON);

		if (ChipId == CHIP_ID_YUKON_SUPR && ChipRev > CHIP_REV_YU_SU_B0) {
			/* enable MACSec clock gating */
			SK_OUT32(IoC, PCI_C2(pAC, IoC, PCI_OUR_REG_3), P_CLK_MACSEC_DIS);
		}
		break;

	case CHIP_ID_YUKON_FE_P:
		pAC->GIni.GIExtLeFormat = SK_TRUE;
	case CHIP_ID_YUKON_UL_2:
#ifndef SK_SLIM
		pAC->GIni.GINumOfPattern -= 1;
#endif
		break;

	case CHIP_ID_YUKON_OPT:
	case CHIP_ID_YUKON_PRM:
	case CHIP_ID_YUKON_OP_2:
		pAC->GIni.GIExtLeFormat = SK_TRUE;
#ifndef SK_SLIM
		pAC->GIni.GINumOfPattern += 1;
#endif

		if (ChipId == CHIP_ID_YUKON_OPT && ChipRev == 0) {
			/* disable PCI-E PHY power down (set PHY reg 0x80, bit 7 */
			SK_OUT32(IoC, Y2_PEX_PHY_DATA, (0x80UL << 16) | BIT_7);

			/* set PHY Link Detect Timer to 1.1 second (11x 100ms) */
			Word = 10;

			/* re-enable PEX PM in PEX PHY debug reg. 8 (clear bit 12) */
			SK_OUT32(IoC, Y2_PEX_PHY_DATA, PEX_DB_ACCESS | (0x08UL << 16));
		}
		else {
			/* set PHY Link Detect Timer to 0.4 second (4x 100ms) */
			Word = 3;
		}

		Word <<= PSM_CONFIG_REG4_TIMER_PHY_LINK_DETECT_BASE;

		Word |= (SK_U16)PSM_CONFIG_REG4_RST_PHY_LINK_DETECT;

		/* enable Config Write */
		SK_TST_MODE_ON(IoC);

		/* reset PHY Quick Link Detect */
		if (1) {
			SK_OUT16(IoC, PCI_C2(pAC, IoC, PSM_CONFIG_REG4), Word);
		}
		else {
			SkPciWriteCfgWord(pAC, PSM_CONFIG_REG4, Word);
		}

#ifndef SK_DIAG
		/* enable PHY Quick Link */
		pAC->GIni.GIValIrqMask |= Y2_IS_PHY_QLNK;

		/* check if PSMv2 was running before */
		SK_IN16(IoC, PCI_C2(pAC, IoC, PSM_CONFIG_REG3), &Word);

		if ((Word & PEX_LC_ASPM_LC_L1) != 0) {
			/* restore the PCIe Link Control register */
			SK_OUT16(IoC, PCI_C2(pAC, IoC, PCIE_LNKCTRL), Word);

			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
				("Restored PCIE_LNKCTRL: 0x%04X\n", Word));
		}
#endif /* !SK_DIAG */

		/* disable Config Write */
		SK_TST_MODE_OFF(IoC);

		break;
	}

#ifndef SK_SLIM
	pAC->GIni.GIMacType = SK_MAC_GMAC;

	pAC->GIni.GIFunc.pFnMacUpdateStats	= SkGmUpdateStats;
	pAC->GIni.GIFunc.pFnMacStatistic	= SkGmMacStatistic;
	pAC->GIni.GIFunc.pFnMacResetCounter	= SkGmResetCounter;
	pAC->GIni.GIFunc.pFnMacOverflow		= SkGmOverflowStatus;
#endif /* !SK_SLIM */

#ifdef SK_DIAG
	pAC->GIni.GIFunc.pFnMacPhyRead		= SkGmPhyRead;
	pAC->GIni.GIFunc.pFnMacPhyWrite		= SkGmPhyWrite;
#else /* SK_DIAG */

	if (CHIP_ID_YUKON_2(pAC)) {
		pAC->GIni.GIFunc.pSkGeSirqIsr	= SkYuk2SirqIsr;
	}
#ifndef DISABLE_YUKON_I
	else {
		pAC->GIni.GIFunc.pSkGeSirqIsr	= SkGeYuSirqIsr;
	}
#endif /* !DISABLE_YUKON_I */
#endif /* !SK_DIAG */

#ifdef SPECIAL_HANDLING
	if (ChipId == CHIP_ID_YUKON) {
		/* check HW self test result */
		SK_IN8(IoC, B2_E_3, &Byte);
		if (Byte & B2_E3_RES_MASK) {
			RetVal = 6;
		}
	}
#endif

	return(RetVal);
}	/* SkGeInit1 */


/******************************************************************************
 *
 *	SkGeInit2() - Level 2 Initialization
 *
 * Description:
 *	- start the Blink Source Counter
 *	- start the Descriptor Poll Timer
 *	- configure the MAC-Arbiter
 *	- configure the Packet-Arbiter
 *	- enable the Tx Arbiters
 *	- enable the RAM Interface Arbiter
 *
 * Returns:
 *	nothing
 */
static void SkGeInit2(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC)		/* I/O Context */
{
	SK_U16	Word;
#if !defined(SK_SLIM) && !defined(SK_DIAG)
	SK_EVPARA	Para;
#endif /* !SK_SLIM && !SK_DIAG */
	int		i;

	/* start the Descriptor Poll Timer */
	if (pAC->GIni.GIPollTimerVal != 0) {
		if (pAC->GIni.GIPollTimerVal > SK_DPOLL_MAX) {
			pAC->GIni.GIPollTimerVal = SK_DPOLL_MAX;

			SK_ERR_LOG(pAC, SK_ERRCL_SW, SKERR_HWI_E017, SKERR_HWI_E017MSG);
		}

		SK_OUT32(IoC, B28_DPT_INI, pAC->GIni.GIPollTimerVal);

		SK_OUT8(IoC, B28_DPT_CTRL, DPT_START);
	}

	/* start Time Stamp Timer with LE generation disabled */
	SK_OUT8(IoC, GMAC_TI_ST_CTRL, (SK_U8)(GMT_ST_LE_DIS | GMT_ST_START));

	/* enable the Tx Arbiters */
	for (i = 0; i < pAC->GIni.GIMacsFound; i++) {
		SK_OUT8(IoC, MR_ADDR(i, TXA_CTRL), TXA_ENA_ARB);
	}

	/* enable the RAM Interface Arbiter */
	SkGeInitRamIface(pAC, IoC);

	if (CHIP_ID_YUKON_2(pAC)) {

		if (pAC->GIni.GIPciBus == SK_PEX_BUS) {

			/* enable Config Write */
			SK_TST_MODE_ON(IoC);

			/* get Max. Read Request Size */
			if (1) {
				SK_IN16(IoC, PCI_C2(pAC, IoC, PEX_CAP_REGS(PEX_DEV_CTRL)), &Word);
			}
			else {
				SkPciReadCfgWord(pAC, PEX_CAP_REGS(PEX_DEV_CTRL), &Word);
			}

			/* check for HW-default value of 512 bytes */
			if ((Word & PEX_DC_MAX_RRS_MSK) == PEX_DC_MAX_RD_RQ_SIZE(2)) {
				/* change Max. Read Request Size to 2048 bytes */
				Word &= ~PEX_DC_MAX_RRS_MSK;
				Word |= PEX_DC_MAX_RD_RQ_SIZE(4);

				if (1) {
					SK_OUT16(IoC, PCI_C2(pAC, IoC, PEX_CAP_REGS(PEX_DEV_CTRL)), Word);
				}
				else {
					SkPciWriteCfgWord(pAC, PEX_CAP_REGS(PEX_DEV_CTRL), Word);
				}
			}

#ifdef REPLAY_TIMER
			else if (pAC->GIni.GIChipId == CHIP_ID_YUKON_EC) {
				/* PEX Ack Reply Timeout to 40 us */
				SK_OUT16(IoC, PCI_C2(pAC, IoC, PEX_ACK_RPLY_TOX1), 0x2710);
			}
#endif
			/* disable Config Write */
			SK_TST_MODE_OFF(IoC);

#if !defined(SK_SLIM) && !defined(SK_DIAG)
			SK_IN16(IoC, PCI_C2(pAC, IoC, PEX_CAP_REGS(PEX_LNK_CAP)), &Word);

			Word = (Word & PEX_CAP_MAX_WI_MSK) >> 4;

			/* compare PEX Negotiated Link Width against max. capabil */
			if (pAC->GIni.GIPexWidth != (SK_U8)Word) {

				SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
					("PEX negotiated Link width is: %d, exp.: %d\n",
					 pAC->GIni.GIPexWidth, Word));

#ifndef NDIS_MINIPORT_DRIVER
				SK_ERR_LOG(pAC, SK_ERRCL_INFO, SKERR_HWI_E026,
					SKERR_HWI_E026MSG);
#endif
				Para.Para64 = 0;
				SkEventQueue(pAC, SKGE_DRV, SK_DRV_PEX_LINK_WIDTH, Para);
			}
#endif /* !SK_SLIM && !SK_DIAG */
		}

		/*
		 * Writing the HW Error Mask Reg. will not generate an IRQ
		 * as long as the B0_IMSK is not set by the driver.
		 */
		SK_OUT32(IoC, B0_HWE_IMSK, pAC->GIni.GIValHwIrqMask);
	}
}	/* SkGeInit2 */


/******************************************************************************
 *
 *	SkGeInit() - Initialize the GE Adapter with the specified level.
 *
 * Description:
 *	Level	0:	Initialize the Module structures.
 *	Level	1:	Generic Hardware Initialization. The IOP/MemBase pointer has
 *				to be set before calling this level.
 *
 *			o Do a software reset.
 *			o Clear all reset bits.
 *			o Verify that the detected hardware is present.
 *			  Return an error if not.
 *			o Get the hardware configuration
 *				+ Set GIMacsFound with the number of MACs.
 *				+ Store the RAM size in GIRamSize.
 *				+ Save the PCI Revision ID in GIPciHwRev.
 *			o return an error
 *				if Number of MACs > SK_MAX_MACS
 *
 *			After returning from Level 0 the adapter
 *			may be accessed with I/O operations.
 *
 *	Level	2:	start the Blink Source Counter
 *
 * Returns:
 *	0:	success
 *	1:	Number of MACs exceeds SK_MAX_MACS (after level 1)
 *	2:	Adapter not present or not accessible
 *	3:	Illegal initialization level
 *	4:	Initialization level 1 call missing
 *	5:	Unexpected PHY type detected
 *	6:	HW self test failed
 */
int	SkGeInit(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Level)		/* Initialization Level */
{
	int		RetVal;		/* return value */
	SK_U32	DWord;

	RetVal = 0;
	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_INIT,
		("SkGeInit(Level %d)\n", Level));

	switch (Level) {
	case SK_INIT_DATA:
		/* Initialization Level 0 */
		SkGeInit0(pAC, IoC);
		pAC->GIni.GILevel = SK_INIT_DATA;
		break;

	case SK_INIT_IO:
		/* Initialization Level 1 */
		RetVal = SkGeInit1(pAC, IoC);
		if (RetVal != 0) {
			break;
		}

		/* check if the adapter seems to be accessible */
		SK_OUT32(IoC, B2_IRQM_INI, SK_TEST_VAL);
		SK_IN32(IoC, B2_IRQM_INI, &DWord);
		SK_OUT32(IoC, B2_IRQM_INI, 0L);

		if (DWord != SK_TEST_VAL) {
			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
				("Error accessing IRQM register, 0x%08lX\n", DWord));
			RetVal = 2;
			break;
		}

#ifdef DEBUG
		/* check if the number of GIMacsFound matches SK_MAX_MACS */
		if (pAC->GIni.GIMacsFound > SK_MAX_MACS) {
			RetVal = 1;
			break;
		}
#endif /* DEBUG */

		/* Level 1 successfully passed */
		pAC->GIni.GILevel = SK_INIT_IO;
		break;

	case SK_INIT_RUN:
		/* Initialization Level 2 */
		if (pAC->GIni.GILevel != SK_INIT_IO) {
#ifndef SK_DIAG
			SK_ERR_LOG(pAC, SK_ERRCL_SW, SKERR_HWI_E002, SKERR_HWI_E002MSG);
#endif /* !SK_DIAG */
			RetVal = 4;
			break;
		}

		SkGeInit2(pAC, IoC);

		/* Level 2 successfully passed */
		pAC->GIni.GILevel = SK_INIT_RUN;
		break;

	default:
		SK_ERR_LOG(pAC, SK_ERRCL_SW, SKERR_HWI_E003, SKERR_HWI_E003MSG);
		RetVal = 3;
		break;
	}

	return(RetVal);
}	/* SkGeInit */

/******************************************************************************
 *
 *	SkGeEnaDynClkGating() - Enable Dynamic Clock Gating
 *
 * Description:
 *	Check whether the device supports dynamic clock gating.
 *	If yes, enable it.
 *
 *	Returns:
 *		nothing
 */
void SkGeEnaDynClkGating(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC)		/* I/O Context */
{
	SK_U32	DWord;

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL, ("SkGeEnaDynClkGating\n"));

	if ((pAC->GIni.GIChipId == CHIP_ID_YUKON_EX &&
		 pAC->GIni.GIChipRev != CHIP_REV_YU_EX_A0) ||
		 pAC->GIni.GIChipId >= CHIP_ID_YUKON_FE_P) {

		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
			("Enable dynamic clock gating, Chip Rev.=%d\n",
			 pAC->GIni.GIChipRev));

		/* set events for gate / release core clock in D3 */
		SK_OUT32(IoC, PCI_C2(pAC, IoC, PCI_OUR_REG_5), PCIE_OUR5_EVENT_CLK_D3_SET);

		SK_OUT32(IoC, PCI_C2(pAC, IoC, PCI_CFG_REG_1), PCIE_CFG1_EVENT_CLK_D3_SET);

		SK_IN32(IoC, PCI_C2(pAC, IoC, PCI_OUR_REG_4), &DWord);

		/* save bit 8 for LED scheme #3 */
		DWord &= P_PIN63_LINK_LED_ENA;
		DWord |= PCIE_OUR4_DYN_CLK_GATE_SET;

		if ((pAC->GIni.GIPexLinkCtrl & PEX_LC_CLK_PM_ENA) == 0) {
			/* Force CLKREQ Enable and CLKREQn pin low */
			DWord |= P_ASPM_FORCE_CLKREQ_PIN | P_ASPM_FORCE_CLKREQ_ENA;
		}

		/* set dynamic core clock gating */
		SK_OUT32(IoC, PCI_C2(pAC, IoC, PCI_OUR_REG_4), DWord);
	}
}

/******************************************************************************
 *
 *	SkGeDeInit() - Deinitialize the adapter
 *
 * Description:
 *	All ports of the adapter will be stopped if not already done.
 *	Do a software reset and switch off all LEDs.
 *
 * Returns:
 *	nothing
 */
void SkGeDeInit(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC)		/* I/O Context */
{
	int		i;
#ifdef SK_PHY_LP_MODE_DEEP_SLEEP
	int		ChipId;
	SK_U32	DWord;
#else
	SK_U16	Word;
#endif /* SK_PHY_LP_MODE_DEEP_SLEEP */

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("SkGeDeInit:  GILevel = %d, PState = %d\n",
		 pAC->GIni.GILevel, pAC->GIni.GP[MAC_1].PState));

	/* check for static clock gating */
	if (pAC->GIni.GILevel == SK_INIT_DATA) {
		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_ERR,
			("Adapter is already de-initialized\n"));

		return;
	}

#if (!defined(SK_SLIM) && !defined(VCPU))
	/* ensure I2C is ready */
	SkI2cWaitIrq(pAC, IoC);
#endif

#ifdef SK_PHY_LP_MODE_DEEP_SLEEP
	/*
	 * for power saving purposes within mobile environments
	 * we set the PHY to coma mode.
	 */

	if (CHIP_ID_YUKON_2(pAC) && !pAC->GIni.GIAsfEnabled
#ifdef XXX
		|| (pAC->GIni.GIYukonLite && pAC->GIni.GIChipRev >= CHIP_REV_YU_LITE_A3)
#endif /* XXX */
		) {

		/* flag for SkGmEnterLowPowerMode() that the call was from here */
		pAC->GIni.GILevel = SK_INIT_IO;

		/* for all ports switch PHY to coma mode */
		for (i = 0; i < pAC->GIni.GIMacsFound; i++) {

			/* Reset Rx MAC FIFO */
			SK_OUT8(IoC, MR_ADDR(i, RX_GMF_CTRL_T), (SK_U8)GMF_RST_SET);

			(void)SkGmEnterLowPowerMode(pAC, IoC, i, PHY_PM_DEEP_SLEEP);
		}

#ifndef SK_SLIM
		if (pAC->GIni.GIVauxAvail) {
			/* switch power to VAUX */
			SK_OUT8(IoC, B0_POWER_CTRL, (SK_U8)(PC_VAUX_ENA | PC_VCC_ENA |
				PC_VAUX_ON | PC_VCC_OFF));
#ifdef DEBUG
			SK_IN32(IoC, B0_CTST, &DWord);

			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
				("Ctrl/Stat & Switch: 0x%08lX\n", DWord));
#endif /* DEBUG */
		}
#endif /* !SK_SLIM */

		ChipId = pAC->GIni.GIChipId;

		if (ChipId == CHIP_ID_YUKON_EC_U ||
			ChipId == CHIP_ID_YUKON_EX ||
			ChipId >= CHIP_ID_YUKON_FE_P) {

			if (!HW_FEATURE(pAC, HWF_HW_WOL_ENABLE)) {
				/* disable HW WOL */
				SK_OUT16(IoC, B0_CTST, Y2_HW_WOL_OFF);
			}

			if (HW_FEATURE(pAC, HWF_ENA_POW_SAV_W_WOL)) {

				SK_IN32(IoC, GPHY_CTRL, &DWord);

				/* Enable Shutdown of 2.5V regulator when Vmain not present */
				SK_OUT32(IoC, GPHY_CTRL, DWord | PHY_CFG_INT_REG_PD_ENA);

				/* stop the watchdog */
				SK_OUT32(IoC, CPU_WDOG, 0);

				/* put CPU into reset state */
				SK_OUT8(IoC, B28_Y2_ASF_STAT_CMD, (SK_U8)HCU_CCSR_ASF_RESET);

				if (!HW_FEATURE(pAC, HWF_WA_DEV_542)) {
					/* put CPU into halt state */
					SK_OUT8(IoC, B28_Y2_ASF_STAT_CMD, (SK_U8)HCU_CCSR_ASF_HALTED);
				}

				/* stop the watchdog */
				SK_OUT32(IoC, CPU_WDOG, 0);

				/* check for Yukon-Ultra A3,B0 / Yukon-Extreme A0 */
				if ((ChipId == CHIP_ID_YUKON_EC_U &&
					 pAC->GIni.GIChipRev > CHIP_REV_YU_EC_U_A0 &&
					 /* check for HW-Rev 2.0 or higher */
					 (pAC->GIni.GIPciHwRev >> 4) > 1) ||
					(ChipId == CHIP_ID_YUKON_EX &&
					 pAC->GIni.GIChipRev == CHIP_REV_YU_EX_A0)) {

					SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
						("Enable static clock gating, Chip Rev.=%d\n",
						pAC->GIni.GIChipRev));

					/* Enable static clock gating */
					SK_OUT32(IoC, PCI_C2(pAC, IoC, PCI_OUR_REG_3),
						PCIE_OUR3_WOL_D3_COLD_SET);
				}
				else {
					SkGeEnaDynClkGating(pAC, IoC);
				}
			}
		}
	}
#ifndef DISABLE_YUKON_I
	else if (pAC->GIni.GIYukonLite) {
		/* take the PHY out of reset (GP_IO) */
		SkMacClearRst(pAC, IoC, MAC_1);

		/* switch PHY to IEEE Power Down mode */
		(void)SkGmEnterLowPowerMode(pAC, IoC, 0, PHY_PM_IEEE_POWER_DOWN);

#ifndef SK_SLIM
		if (pAC->GIni.GIVauxAvail) {
			/* switch power to VAUX */
			SK_OUT8(IoC, B0_POWER_CTRL, (SK_U8)(PC_VAUX_ENA | PC_VCC_ENA |
				PC_VAUX_ON | PC_VCC_OFF));
		}
#endif /* !SK_SLIM */
	}
#endif /* !DISABLE_YUKON_I */
#else /* !SK_PHY_LP_MODE_DEEP_SLEEP */

	if (!pAC->GIni.GIAsfEnabled) {
		/* stop all current transfer activity */
		for (i = 0; i < pAC->GIni.GIMacsFound; i++) {
			if (pAC->GIni.GP[i].PState != SK_PRT_STOP &&
				pAC->GIni.GP[i].PState != SK_PRT_RESET) {

				SkGeStopPort(pAC, IoC, i, SK_STOP_ALL, SK_HARD_RST);
			}
		}
	}

	/* reset all bits in the PCI STATUS register */
	/*
	 * Note: PCI Cfg cycles cannot be used, because they are not
	 *	 available on some platforms after 'boot time'.
	 */
	if (1) {
		SK_IN16(IoC, PCI_C2(pAC, IoC, PCI_STATUS), &Word);
	}
	else {
		SkPciReadCfgWord(pAC, PCI_STATUS, &Word);
	}

	SK_TST_MODE_ON(IoC);

	if (1) {
		SK_OUT16(IoC, PCI_C2(pAC, IoC, PCI_STATUS), Word | (SK_U16)PCI_ERRBITS);
	}
	else {
		SkPciWriteCfgWord(pAC, PCI_STATUS, Word | (SK_U16)PCI_ERRBITS);
	}

	SK_TST_MODE_OFF(IoC);

	if (!pAC->GIni.GIAsfEnabled) {
		/* enable dynamic clock gating if supported */
		SkGeEnaDynClkGating(pAC, IoC);

		/* set the SW-reset */
		SK_OUT8(IoC, B0_CTST, CS_RST_SET);
	}
#endif /* !SK_PHY_LP_MODE_DEEP_SLEEP */

	pAC->GIni.GILevel = SK_INIT_DATA;

}	/* SkGeDeInit */


#ifdef XXX
/******************************************************************************
 *
 *	SkGeByPassMacSec() - Startup initialization for MAC Security unit
 *
 * Description:
 *	Set the MAC security unit into a state to bypass all data.
 *
 * Returns:
 *	nothing
 */
static void SkGeByPassMacSec(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port)		/* Port to configure */
{
	if (HW_IS_EXT_LE_FORMAT(pAC)) {
		/* Bypass all MACSec frames */
		SK_OUT16(IoC, MR_ADDR(Port, MAC_CTRL), MAC_CTRL_BYP_MACSECRX_ON |
			MAC_CTRL_BYP_MACSECTX_ON | MAC_CTRL_BYP_RETR_ON);
	}
}
#endif /* XXX */

/******************************************************************************
 *
 *	SkGeInitPort() - Initialize the specified port
 *
 * Description:
 *	PRxQSize, PXSQSize and PXAQSize has to be configured for the specified port
 *	before calling this function.
 *	The descriptor rings has to be initialized too.
 *
 *	o (Re)configure queues of the specified port.
 *	o configure the MAC of the specified port.
 *	o put ASIC and MAC(s) in operational mode.
 *	o initialize Rx/Tx and Sync LED
 *	o initialize RAM Buffers and MAC FIFOs
 *
 *	The port is ready to connect when returning.
 *
 * Note:
 *	The MAC's Rx and Tx state machine is still disabled when returning.
 *
 * Returns:
 *	0:	success
 *	1:	Queue size initialization error. The configured values
 *		for PRxQSize, PXSQSize, or PXAQSize are invalid for one
 *		or more queues. The specified port was NOT initialized.
 *		An error log entry was generated.
 *	2:	The port has to be stopped before it can be initialized again.
 */
int SkGeInitPort(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port)		/* Port to configure */
{
	SK_GEPORT *pPrt;

	pPrt = &pAC->GIni.GP[Port];

	if (SkGeCheckQSize(pAC, Port) != 0) {
		SK_ERR_LOG(pAC, SK_ERRCL_SW, SKERR_HWI_E004, SKERR_HWI_E004MSG);
		return(1);
	}

	if (pPrt->PState >= SK_PRT_INIT) {
		SK_ERR_LOG(pAC, SK_ERRCL_SW, SKERR_HWI_E005, SKERR_HWI_E005MSG);
		return(2);
	}

	/* configuration ok, initialize the Port now */

	if (pAC->GIni.GIDontInitPhy) {
		/* change PState to avoid PHY reset */
		pPrt->PState = SK_PRT_STOP;
	}

	SkGmInitMac(pAC, IoC, Port);

	SkGeInitMacFifo(pAC, IoC, Port);

	SkGeInitRamBufs(pAC, IoC, Port);

	if (pPrt->PXSQSize != 0) {
		/* enable Force Sync bit if synchronous queue available */
		SK_OUT8(IoC, MR_ADDR(Port, TXA_CTRL), TXA_ENA_FSYNC);
	}

	SkGeInitBmu(pAC, IoC, Port);

	if (pAC->GIni.GIChipId == CHIP_ID_YUKON_EX ||
		pAC->GIni.GIChipId == CHIP_ID_YUKON_SUPR) {
		/*
		 * Disable flushing of non ASF packets;
		 * must be done after initializing the BMUs;
		 * drivers without ASF support should do this too, otherwise
		 * it may happen that they cannot run on ASF devices;
		 * remember that the MAC FIFO isn't reset during initialization.
		 */
		SK_OUT32(IoC, MR_ADDR(Port, RX_GMF_CTRL_T), RXMF_TCTL_MACSEC_FLSH_DIS);
	}

	if (pAC->GIni.GIChipId >= CHIP_ID_YUKON_SUPR) {
		/* Enable RX Home Address & Routing Header checksum fix */
		SK_OUT16(IoC, RX_GMF_FL_CTRL, RX_IPV6_SA_MOB_ENA | RX_IPV6_DA_MOB_ENA);

		/* Routing Header checksum fix */
		SK_OUT32(IoC, TBMU_TEST, TBMU_TEST_ROUTING_ADD_FIX_EN);

		if (!HW_FEATURE(pAC, HWF_WA_DEV_563)) {
			/* Enable TX Home Address & Routing Header checksum fix */
			SK_OUT32(IoC, TBMU_TEST, TBMU_TEST_HOME_ADD_FIX_EN);
		}
	}

	/* mark port as initialized */
	pPrt->PState = SK_PRT_INIT;

	return(0);
}	/* SkGeInitPort */

#if defined(SK_POWER_MGMT) || defined(SK_DIAG)

/******************************************************************************
 *
 *	ComputeEvenParityForDword() - Compute even parity for a dword
 *
 * Description:
 *	Generate even parity per byte of a dword
 *	(number of 1s in a byte plus parity bit must be even.
 *
 * Returns:
 *	4-bit parity (one bit per byte)
 */
static SK_U8 ComputeEvenParityForDword(
SK_U32 Dword)
{
	/* Each byte contains b7 b6 b5 b4 b3 b2 b1 b0. */
	Dword ^= Dword >> 4;
	/* Each byte contains * * * * b7^b3 b6^b2 b5^b1 b4^b0. */
	Dword ^= Dword >> 2;
	/* Each byte contains * * * * * * b7^b5^b3^b1 b6^b4^b2^b0. */
	Dword ^= Dword >> 1;
	/* Each byte contains * * * * * * * b7^b6^b5^b4^b3^b2^b1^b0. */
	Dword &= BIT_24 | BIT_16 | BIT_8 | BIT_0;
	/* Each byte contains 0 0 0 0 0 0 0 b7^b6^b5^b4^b3^b2^b1^b0. */
	Dword |= (Dword >> (24 - 3)) | (Dword >> (16 - 2)) | (Dword >> (8 - 1));

	/* Return the lower nibble, which contains the parity bits. */
	return((SK_U8)Dword & 0x0f);
}

/******************************************************************************
 *
 *	SkGeRamRead() - Reads One quadword to RAM
 *
 * Returns:
 *	0
 */
void SkGeRamRead(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
SK_U32	Addr,		/* Address to be read at (in quadwords) */
SK_U32	*pLoDword,	/* Lower Dword to be read */
SK_U32	*pHiDword,	/* Upper Dword to be read */
int		RamSelect)	/* Select RAM buffer (for Yukon-2: Port) */
{
#ifdef SK_DIAG
	int		fl;
	SK_U8	Parity;
	SK_U8	ParityExpected;
#endif /* SK_DIAG */

	if (pAC->GIni.GIChipId == CHIP_ID_YUKON_EC_U ||	/* Rev. A1 or later */
		pAC->GIni.GIChipId == CHIP_ID_YUKON_EX ||
		pAC->GIni.GIChipId >= CHIP_ID_YUKON_FE_P) {

		RamSelect <<= RAM_ADDR_SEL_BASE;

		/* if Addr == BIT_29 use current position of the HW pointer */
		if (Addr == RAM_ADDR_ADDR_MUX) {
			Addr = RamSelect;
		}
		else {
			if (pAC->GIni.GIChipId != CHIP_ID_YUKON_SUPR) {
				/* Use given address (11 bits) */
				Addr &= 0x7ffL;
			}
			else {
				/* Yukon-Supreme has 256K SRAM so use different address mask */
				Addr &= RAM_ADDR_MSK;
			}

			Addr |= RAM_ADDR_ADDR_MUX | RamSelect;
		}

		RamSelect = 0;	/* For using SELECT_RAM_BUFFER() */
	}
	else {	/* not Yukon-Ultra and not Yukon-Extreme and not Yukon-FE+ */
		Addr += pAC->GIni.GIRamOffs / 8;
	}

#ifdef SK_DIAG
	fl = spl5();	/* disable interrupt to avoid changing RAM address */
#endif /* SK_DIAG */

	SK_OUT32(IoC, SELECT_RAM_BUFFER(RamSelect, B3_RAM_ADDR), Addr);

	/* Read Access is initiated by reading the lower dword */
	SK_IN32(IoC, SELECT_RAM_BUFFER(RamSelect, B3_RAM_DATA_LO), pLoDword);
	SK_IN32(IoC, SELECT_RAM_BUFFER(RamSelect, B3_RAM_DATA_HI), pHiDword);

#ifdef SK_DIAG
	if (pAC->GIni.GIChipId == CHIP_ID_YUKON_EC_U) {
		/* Yukon-Ultra, Rev. A1 or later */

		/* Read the Parity value */
		SK_IN8(IoC, SELECT_RAM_BUFFER(RamSelect, B3_RAM_PARITY), &Parity);

		/* Check the Parity value */
		ParityExpected = ComputeEvenParityForDword(*pLoDword) |
			(ComputeEvenParityForDword(*pHiDword) << 4);

		if (Parity != ParityExpected) {
			w_print("Wrong parity value %02x, should be %02x\n",
				Parity, ParityExpected);
		}
	}

	splx(fl);
#endif /* SK_DIAG */

	return;
}

#endif /* SK_POWER_MGMT || SK_DIAG */

#if defined(SK_POWER_MGMT) || defined(SK_DIAG) || (defined(YUK2) && !defined(SK_SLIM))

/******************************************************************************
 *
 *	SkGeRamWrite() - Writes One quadword to RAM
 *
 * Returns:
 *	0
 */
void SkGeRamWrite(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
SK_U32	Addr,		/* Address to be written to (in quadwords) */
SK_U32	LoDword,	/* Lower Dword to be written */
SK_U32	HiDword,	/* Upper Dword to be written */
int		RamSelect)	/* Select RAM buffer (for Yukon-2: Port) */
{
#ifdef SK_DIAG
	int		fl;
	SK_U8	Parity;
#endif /* SK_DIAG */

	if (pAC->GIni.GIChipId == CHIP_ID_YUKON_EC_U ||	/* Rev. A1 or later */
		pAC->GIni.GIChipId == CHIP_ID_YUKON_EX ||
		pAC->GIni.GIChipId >= CHIP_ID_YUKON_FE_P) {

		RamSelect <<= RAM_ADDR_SEL_BASE;

		/* if Addr == BIT_29 use current position of the HW pointer */
		if (Addr == RAM_ADDR_ADDR_MUX) {
			Addr = RamSelect;
		}
		else {
			if (pAC->GIni.GIChipId != CHIP_ID_YUKON_SUPR) {
				/* Use given address (11 bits) */
				Addr &= 0x7ffL;
			}
			else {
				/* Yukon-Supreme has 256K SRAM so use different address mask */
				Addr &= RAM_ADDR_MSK;
			}

			Addr |= RAM_ADDR_ADDR_MUX | RamSelect;
		}

		RamSelect = 0;	/* For using SELECT_RAM_BUFFER() on Yukon-Ultra */
	}
	else {	/* not Yukon-Ultra and not Yukon-Extreme and not Yukon-FE+ */
		Addr += pAC->GIni.GIRamOffs / 8;
	}

#ifdef SK_DIAG
	fl = spl5();	/* disable interrupt to avoid changing RAM address */
#endif /* SK_DIAG */

	SK_OUT32(IoC, SELECT_RAM_BUFFER(RamSelect, B3_RAM_ADDR), Addr);

	/* For all adapters but the Yukon-Ultra,
	 * Write Access is initiated by writing the upper dword.
	 */
	SK_OUT32(IoC, SELECT_RAM_BUFFER(RamSelect, B3_RAM_DATA_LO), LoDword);
	SK_OUT32(IoC, SELECT_RAM_BUFFER(RamSelect, B3_RAM_DATA_HI), HiDword);

#ifdef SK_DIAG
	if (pAC->GIni.GIChipId == CHIP_ID_YUKON_EC_U) {
		/* Yukon-Ultra, Rev. A1 or later */

		/* Compute even parity for two dwords */
		Parity = ComputeEvenParityForDword(LoDword) |
			(ComputeEvenParityForDword(HiDword) << 4);

		/* Check if an inverted parity value should be written */
		if (para_get_val("WriteInvParity") == 1) {
			Parity ^= 0xff;
		}

		/* Write the parity value. This initiates
		 * the write access for the Yukon-Ultra.
		 */
		SK_OUT8(IoC, SELECT_RAM_BUFFER(RamSelect, B3_RAM_PARITY), Parity);
	}

	splx(fl);
#endif /* SK_DIAG */

	return;
}

#endif /* SK_POWER_MGMT || SK_DIAG || (YUK2 && !SK_SLIM) */

#if defined(YUK2) && !defined(SK_SLIM)

/******************************************************************************
 *
 * SkYuk2RestartRxBmu() - Restart Receive BMU on Yukon-2
 *
 * return:
 *	0	o.k.
 *	1	timeout
 */
int SkYuk2RestartRxBmu(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port)		/* Port Index (MAC_1 + n) */
{
	SK_U8		Byte;
	SK_U16		Word;
	SK_U16		MacCtrl;
	SK_U16		MacCtrlSave;
	SK_U16		RxCtrl;
	SK_U16		FlushMask;
	SK_U16		FlushTrsh;
	SK_U32		RamAdr;
	SK_U32		StartTime;
	SK_U32		TimeOut;
	SK_BOOL		TimeStampDisabled;
	SK_GEPORT	*pPrt;			/* GIni Port struct pointer */
	SK_U16		WordBuffer[4];	/* Buffer to handle MAC address */
	int			i;
	int			Rtv;

	Rtv = 0;

	/* read the MAC Interrupt source register */
	SK_IN8(IoC, MR_ADDR(Port, GMAC_IRQ_SRC), &Byte);

	/* check if a packet was received by the MAC */
	if ((Byte & GM_IS_RX_COMPL) == 0) {
		/* workaround not necessary */
		return(0);
	}

	pPrt = &pAC->GIni.GP[Port];

	/*
	 1. save Rx MAC FIFO Flush Mask and Rx MAC FIFO Flush Threshold
	 2. save MAC Rx Control Register
	 3. re-initialize MAC Rx FIFO, Rx RAM Buffer Queue, PCI Rx FIFO,
		Rx BMU and Rx Prefetch Unit of the link.
	 4. set Rx MAC FIFO Flush Mask to 0xffff
		set Rx MAC FIFO Flush Threshold to a high value, e.g. 0x20
	 5. set MAC to loopback mode and switch MAC back to Rx/Tx enable
	 6. clear Rx/Tx Frame Complete IRQ in Rx/T MAC FIFO Control Register
	 7. send one packet with a size of 64bytes (size below flush threshold)
		from TXA RAM Buffer Queue to set the rx_sop flop:
		- set TxAQ Write Pointer to (packet size in qwords + 2)
		- set TxAQ Level to (packet size in qwords + 2)
		- write Internal Status Word 1 and 2 to TxAQ RAM Buffer Queue QWord 0,1
		  according to figure 61 on page 330 of Yukon-2 Spec.
		- write MAC header with Destination Address = own MAC address to
		  TxAQ RAM Buffer Queue QWords 2 and 3
		- set TxAQ Packet Counter to 1 -> packet is transmitted immediately
	 8. poll MAC IRQ Source Register for IRQ Rx/Tx Frame Complete
	 9. restore MAC Rx Control Register
	10. restore Rx MAC FIFO Flush Mask and Rx MAC FIFO Flush Threshold
	11. set MAC back to GMII mode
	*/

	/* save MAC General Purpose Control */
	GM_IN16(IoC, Port, GM_GP_CTRL, &MacCtrlSave);

	/* save Rx MAC FIFO Flush Mask */
	SK_IN16(IoC, MR_ADDR(Port, RX_GMF_FL_MSK), &FlushMask);

	/* save Rx MAC FIFO Flush Threshold */
	SK_IN16(IoC, MR_ADDR(Port, RX_GMF_FL_THR), &FlushTrsh);

	/* save MAC Rx Control Register */
	GM_IN16(IoC, Port, GM_RX_CTRL, &RxCtrl);

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("SkYuk2RestartRxBmu: Port %d, RxCtrl=0x%04X\n",
		 Port, RxCtrl));

	/* configure the MAC FIFOs */
	SkGeInitMacFifo(pAC, IoC, Port);

	SkGeInitRamBufs(pAC, IoC, Port);

	SkGeInitBmu(pAC, IoC, Port);

	/* check timestamp timer */
	SK_IN8(IoC, GMAC_TI_ST_CTRL, &Byte);

	TimeStampDisabled = (Byte & GMT_ST_START) == 0;

	if (TimeStampDisabled) {
		/* start Time Stamp Timer */
		SkMacTimeStamp(pAC, IoC, Port, SK_TRUE);
	}

	/* configure Rx MAC FIFO */
	SK_OUT16(IoC, MR_ADDR(Port, RX_GMF_CTRL_T), GMF_RX_CTRL_DEF);

	/* set Rx MAC FIFO Flush Mask */
	SK_OUT16(IoC, MR_ADDR(Port, RX_GMF_FL_MSK), 0xffff);

	/* set Rx MAC FIFO Flush Threshold */
	SK_OUT16(IoC, MR_ADDR(Port, RX_GMF_FL_THR), 32);

	/* set to promiscuous mode */
	Word = RxCtrl & ~(GM_RXCR_UCF_ENA | GM_RXCR_MCF_ENA);

	/* set MAC Rx Control Register */
	GM_OUT16(IoC, Port, GM_RX_CTRL, Word);

	MacCtrl = MacCtrlSave | GM_GPCR_LOOP_ENA | GM_GPCR_FL_PASS |
		GM_GPCR_DUP_FULL | GM_GPCR_AU_ALL_DIS;

	/* enable MAC Loopback Mode */
	GM_OUT16(IoC, Port, GM_GP_CTRL, MacCtrl);

	/* enable MAC Rx/Tx */
	GM_OUT16(IoC, Port, GM_GP_CTRL, MacCtrl | GM_GPCR_RX_ENA | GM_GPCR_TX_ENA);

	/* clear MAC IRQ Rx Frame Complete */
	SK_OUT8(IoC, MR_ADDR(Port, RX_GMF_CTRL_T), (SK_U8)GMF_CLI_RX_FC);

	/* clear MAC IRQ Tx Frame Complete */
	SK_OUT8(IoC, MR_ADDR(Port, TX_GMF_CTRL_T), (SK_U8)GMF_CLI_TX_FC);

	/* check for half-duplex */
	if ((MacCtrlSave & GM_GPCR_DUP_FULL) == 0) {
		/* get current value of timestamp timer */
		StartTime = SkHwGetTimeDelta(pAC, IoC, 0);

		/* set delay to 20 ms */
		TimeOut = HW_MS_TO_TICKS(pAC, 20);

		do {
			/* dummy read */
			GM_IN16(IoC, Port, GM_GP_STAT, &Word);

		} while (SkHwGetTimeDelta(pAC, IoC, StartTime) < TimeOut);
	}

	/* send one packet with a size of 64bytes from RAM buffer*/

	RamAdr = pPrt->PXaQRamStart / 8;

	SK_OUT32(IoC, RB_ADDR(pPrt->PXaQOff, RB_WP), RamAdr + 10);

	SK_OUT32(IoC, RB_ADDR(pPrt->PXaQOff, RB_LEV), 10);

	/* write 1st status quad word (packet end address in RAM, packet length */
	SkGeRamWrite(pAC, IoC, RamAdr, (RamAdr + 9) << 16, 64, Port);

	/* write 2nd status quad word */
	SkGeRamWrite(pAC, IoC, RamAdr + 1, 0, 0, Port);

	for (i = 0; i < 3; i++) {
		/* set MAC Destination Address */
		WordBuffer[i] = pPrt->PMacAddr[i];
	}

	WordBuffer[3] = WordBuffer[0];

	/* write DA to MAC header */
	SkGeRamWrite(pAC, IoC, RamAdr + 2, (SK_U32)(*(SK_U32 *)&WordBuffer[0]),
		(SK_U32)(*(SK_U32 *)&WordBuffer[2]), Port);

	WordBuffer[0] = WordBuffer[1];
	WordBuffer[1] = WordBuffer[2];
	WordBuffer[2] = 0x3200;		/* len / type field (big endian) */
	WordBuffer[3] = 0;

	/* write SA and type field to MAC header */
	SkGeRamWrite(pAC, IoC, RamAdr + 3, (SK_U32)(*(SK_U32 *)&WordBuffer[0]),
		(SK_U32)(*(SK_U32 *)&WordBuffer[2]), Port);

	SkGeRamWrite(pAC, IoC, RamAdr + 4, 0x4c56524d,	/* "MRVL" */
		0x00464d2d, Port);							/* "-MF"  */

	for (i = 0; i < 5; i++) {
		/* fill packet with zeroes */
		SkGeRamWrite(pAC, IoC, RamAdr + 5 + i, 0, 0, Port);
	}

	SK_OUT32(IoC, RB_ADDR(pPrt->PXaQOff, RB_PC), 1);

	/* get current value of timestamp timer */
	StartTime = SkHwGetTimeDelta(pAC, IoC, 0);

	/* set timeout to 10 ms */
	TimeOut = HW_MS_TO_TICKS(pAC, 10);

	do {
		if (SkHwGetTimeDelta(pAC, IoC, StartTime) > TimeOut) {
			Rtv = 1;
			break;
		}

		/* read the MAC Interrupt source register */
		SK_IN8(IoC, MR_ADDR(Port, GMAC_IRQ_SRC), &Byte);

	} while ((Byte & (GM_IS_TX_COMPL | GM_IS_RX_COMPL)) !=
			 (GM_IS_TX_COMPL | GM_IS_RX_COMPL));

	/* clear MAC IRQ Rx Frame Complete */
	SK_OUT8(IoC, MR_ADDR(Port, RX_GMF_CTRL_T), (SK_U8)GMF_CLI_RX_FC);

	if (TimeStampDisabled) {
		/* stop Time Stamp Timer */
		SkMacTimeStamp(pAC, IoC, Port, SK_FALSE);
	}

	/* restore MAC General Purpose Control */
	GM_OUT16(IoC, Port, GM_GP_CTRL, MacCtrlSave);

	/* restore MAC Rx Control Register */
	GM_OUT16(IoC, Port, GM_RX_CTRL, RxCtrl);

	/* restore Rx MAC FIFO Flush Mask */
	SK_OUT16(IoC, MR_ADDR(Port, RX_GMF_FL_MSK), FlushMask);

	/* restore Rx MAC FIFO Flush Threshold */
	SK_OUT16(IoC, MR_ADDR(Port, RX_GMF_FL_THR), FlushTrsh);

	return(Rtv);
}	/* SkYuk2RestartRxBmu */

/******************************************************************************
 *
 *	SkYuk2StopTx() - Stop Tx Path on Yukon-2 device family
 *
 * Description:
 *	This function stops the transmit path of Yukon-2 devices.
 *	Currently this function is only tested for Yukon-Extreme.
 *	Please use SkYuk2StartTx() to restart a stopped port.
 *
 * return:
 *	nothing
 */
void SkYuk2StopTx(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port)		/* Port Index 0 or 1 */
{
	SK_GEPORT	*pPrt;
	SK_U16		Word;
	SK_U32		XaCsr;
	SK_U32		StartTime;
	SK_U32		TimeOut;
	int			ToutCnt;

	pPrt = &pAC->GIni.GP[Port];

	/* Disable MAC Tx */
	GM_IN16(IoC, Port, GM_GP_CTRL, &Word);
	Word &= ~GM_GPCR_TX_ENA;
	GM_OUT16(IoC, Port, GM_GP_CTRL, Word);

	/* stop transmit queue */
	SK_OUT32(IoC, Q_ADDR(pPrt->PXaQOff, Q_CSR), BMU_STOP);

	/* If the BMU is in the reset state CSR_STOP will terminate immediately */

	/* get current value of timestamp timer */
	StartTime = SkHwGetTimeDelta(pAC, IoC, 0);

	ToutCnt = 0;

	/* set timeout to 10 ms */
	TimeOut = HW_MS_TO_TICKS(pAC, 10);

	do {
		XaCsr = TestStopBit(pAC, IoC, pPrt->PXaQOff);

		if (SkHwGetTimeDelta(pAC, IoC, StartTime) > TimeOut) {
			/* Timeout of 10 ms reached */
			ToutCnt++;

			if (ToutCnt > 1) {
				/*
				 * If BMU stop doesn't terminate, we assume that
				 * we have a stable state and can reset the BMU,
				 * the Prefetch Unit, and RAM buffer now.
				 */
				break;			/* ===> leave do/while loop here */
			}
			/*
			 * Cache incoherency workaround: assume a start command
			 * has been lost while sending the frame.
			 */

			/* get current value of timestamp timer */
			StartTime = SkHwGetTimeDelta(pAC, IoC, 0);

			if ((XaCsr & BMU_STOP) != 0) {
				SK_OUT32(IoC, Q_ADDR(pPrt->PXaQOff, Q_CSR), BMU_START);
			}

			/*
			 * After the previous operations the X(s|a)Csr does no
			 * longer contain the proper values
			 */
			XaCsr = TestStopBit(pAC, IoC, pPrt->PXaQOff);
		}
	} while ((XaCsr & BMU_IDLE) != BMU_IDLE);

	/* Perform a local reset of the port's Tx path */
	/* Reset the PCI FIFO of the async Tx queue */
	SK_OUT32(IoC, Q_ADDR(pPrt->PXaQOff, Q_CSR), BMU_FIFO_RST | BMU_RST_SET);

	/* Reset the Tx prefetch units */
	SK_OUT8(IoC, Y2_PREF_Q_ADDR(pPrt->PXaQOff, PREF_UNIT_CTRL_REG),
		PREF_UNIT_RST_SET);

#ifdef SK_AVB
	/* Reset the AVB prefetch units */
	if (HW_SUP_AVB(pAC)) {
		/* Resets the prefetch units before it sets the AVB mode */
		SkYuk2AVBControl(pAC, IoC, Port,
			SkYuk2AVBModeEnabled(pAC, IoC, Port));
	}
#endif /* SK_AVB */

	if (HW_IS_RAM_IF_AVAIL(pAC)) {
		/* Reset the RAM Buffer async Tx queue */
		SK_OUT8(IoC, RB_ADDR(pPrt->PXaQOff, RB_CTRL), RB_RST_SET);
	}

	if (pAC->GIni.GIAsfEnabled) {
		/*
		 * ASF FW checks start and end pointer of its ASF queue periodically.
		 * The idea is to set the transmit queue in non operational mode when
		 * stopping the port and to reset the ASF transmit queue when
		 * re-starting the transmit path.
		 * ASF will reinitialize the transmit queue by its own when trying
		 * to send the next packet.
		 */
		SK_OUT8(IoC, RB_ADDR((Port ? Q_ASF_T2 : Q_ASF_T1), RB_CTRL), RB_DIS_OP_MD);
	}

	SK_OUT8(IoC, MR_ADDR(Port, TX_GMF_CTRL_T), (SK_U8)GMF_RST_SET);
}

/******************************************************************************
 *
 *	SkYuk2StartTx() - Start Tx Path on Yukon-2 device family
 *
 * Description:
 *	This function re-starts the transmit path of a Yukon-2 device which was
 *	previously stopped with SkYuk2StopTx().
 *	Currently this function was only tested for Yukon-Extreme.
 *
 * return:
 *	nothing
 */
void SkYuk2StartTx(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port)		/* Port Index 0 or 1 */
{
	SK_GEPORT	*pPrt;
	SK_U16		Word;

	pPrt = &pAC->GIni.GP[Port];

	if (pAC->GIni.GIAsfEnabled) {
		/*
		 * ASF TX queue is in non-operational state. Reset it now.
		 * ASF firmware is checking start and end pointer its ASF queue
		 * periodically.
		 * The idea is to set the transmit queue in non operational mode when
		 * stopping the port and to reset the ASF transmit queue when
		 * re-starting the transmit path. ASF will reinitialize the
		 * transmit queue by its own when trying to send the next packet.
		 */
		SK_OUT8(IoC, RB_ADDR((Port ? Q_ASF_T2 : Q_ASF_T1), RB_CTRL), RB_RST_SET);
	}

	/* configure Tx GMAC FIFO */
	SK_OUT8(IoC, MR_ADDR(Port, TX_GMF_CTRL_T), (SK_U8)GMF_RST_CLR);

	SK_OUT16(IoC, MR_ADDR(Port, TX_GMF_CTRL_T), (SK_U16)GMF_TX_CTRL_DEF);

	if (pAC->GIni.GIChipId == CHIP_ID_YUKON_EC_U ||
		pAC->GIni.GIChipId == CHIP_ID_YUKON_EX ||
		pAC->GIni.GIChipId >= CHIP_ID_YUKON_SUPR) {
		/* set Rx Pause Threshold */
		SK_OUT16(IoC, MR_ADDR(Port, RX_GMF_LP_THR), (SK_U16)SK_ECU_LLPP);
		SK_OUT16(IoC, MR_ADDR(Port, RX_GMF_UP_THR), (SK_U16)SK_ECU_ULPP);

		if ((pAC->GIni.GIChipId == CHIP_ID_YUKON_EX &&
			 pAC->GIni.GIChipRev != CHIP_REV_YU_EX_A0) ||
			pAC->GIni.GIChipId >= CHIP_ID_YUKON_SUPR) {
			/* Yukon-Extreme B0 and further devices */
			/* enable Store & Forward mode for TX */
			SK_OUT32(IoC, MR_ADDR(Port, TX_GMF_CTRL_T), TX_STFW_ENA);
		}
		else {
			if (pAC->GIni.GP[Port].PPortUsage == SK_JUMBO_LINK) {
				/* set Tx GMAC FIFO Almost Empty Threshold */
				SK_OUT32(IoC, MR_ADDR(Port, TX_GMF_AE_THR),
					(SK_ECU_JUMBO_WM << 16) | (SK_U16)SK_ECU_AE_THR);
				/* disable Store & Forward mode for TX */
				SK_OUT32(IoC, MR_ADDR(Port, TX_GMF_CTRL_T), TX_STFW_DIS);
			}
		}
	}

	if (HW_IS_RAM_IF_AVAIL(pAC)) {
		DoInitRamQueue(pAC, IoC, pPrt->PXaQOff, pPrt->PXaQRamStart,
			pPrt->PXaQRamEnd, SK_TX_RAM_Q);
	}

	/*
	 * Tx Queue: Release all local resets if the queue is used !
	 *		set watermark
	 */

	if (pPrt->PXAQSize != 0) {

		SK_OUT32(IoC, Q_ADDR(pPrt->PXaQOff, Q_CSR), BMU_CLR_RESET);
		SK_OUT32(IoC, Q_ADDR(pPrt->PXaQOff, Q_CSR), BMU_OPER_INIT);
		SK_OUT32(IoC, Q_ADDR(pPrt->PXaQOff, Q_CSR), BMU_FIFO_OP_ON);

		if (HW_FEATURE(pAC, HWF_TX_IP_ID_INCR_ON)) {
			/* Enable Tx IP ID Increment */
			SK_OUT32(IoC, Q_ADDR(pPrt->PXaQOff, Q_CSR), BMU_TX_IPIDINCR_ON);
		}

		SK_OUT16(IoC, Q_ADDR(pPrt->PXaQOff, Q_WM), SK_BMU_TX_WM);
	}

	/* WA for dev. #4.209 */
	if (pAC->GIni.GIChipId == CHIP_ID_YUKON_EC_U &&
		pAC->GIni.GIChipRev == CHIP_REV_YU_EC_U_A1) {
		/* enable/disable Store & Forward mode for TX */
		SK_OUT32(IoC, MR_ADDR(Port, TX_GMF_CTRL_T),
			pPrt->PLinkSpeedUsed != (SK_U8)SK_LSPEED_STAT_1000MBPS ?
			TX_STFW_ENA : TX_STFW_DIS);
	}

	/* Enable MAC Tx */
	GM_IN16(IoC, Port, GM_GP_CTRL, &Word);
	Word |= GM_GPCR_TX_ENA;
	GM_OUT16(IoC, Port, GM_GP_CTRL, Word);
}

#endif /* YUK2 && !SK_SLIM */

/* End of File */

