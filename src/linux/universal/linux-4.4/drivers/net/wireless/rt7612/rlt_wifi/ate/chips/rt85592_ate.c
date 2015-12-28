/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2011, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	rt85592_ate.c

	Abstract:
	Specific ATE funcitons and variables for RT85592

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifdef RT8592

#include "rt_config.h"

#ifndef RTMP_RF_RW_SUPPORT
#error "You Should Enable compile flag RTMP_RF_RW_SUPPORT for this chip"
#endif /* RTMP_RF_RW_SUPPORT */


struct _ATE_CHIP_STRUCT RALINK85592 =
{
	/* functions */
	.ChannelSwitch = NULL,
	.TxPwrHandler = NULL,
	.TssiCalibration = NULL,
	.ExtendedTssiCalibration = NULL /* RT5592_ATETssiCalibrationExtend */,
	.RxVGAInit = NULL,
	.AsicSetTxRxPath = NULL,
	.AdjustTxPower = NULL,
	.AsicExtraPowerOverMAC = NULL,
	.TemperCompensation = NULL,
	
	/* command handlers */
	.Set_BW_Proc = NULL,
	.Set_FREQ_OFFSET_Proc = NULL,

	/* variables */
	.maxTxPwrCnt = 5,
	.bBBPStoreTXCARR = FALSE,
	.bBBPStoreTXCARRSUPP = FALSE,	
	.bBBPStoreTXCONT = FALSE,
	.bBBPLoadATESTOP = FALSE,
};

#endif /* RT8592 */

