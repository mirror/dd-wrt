/**
 * @file IxFeatureCtrlSymbols.c
 *
 * @author Intel Corporation
 * @date 29-Jan-2003
 *
 * @brief This file declares exported symbols for linux kernel module builds.
 *
 * @version $Revision: 1.6 $
 * 
 * @par
 * IXP400 SW Release version  2.1
 * 
 * -- Intel Copyright Notice --
 * 
 * @par
 * Copyright (c) 2002-2005 Intel Corporation All Rights Reserved.
 * 
 * @par
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by Intel Corporation
 * or its suppliers or licensors.  Title to the Material remains with
 * Intel Corporation or its suppliers and licensors.
 * 
 * @par
 * The Material is protected by worldwide copyright and trade secret laws
 * and treaty provisions. No part of the Material may be used, copied,
 * reproduced, modified, published, uploaded, posted, transmitted,
 * distributed, or disclosed in any way except in accordance with the
 * applicable license agreement .
 * 
 * @par
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you by
 * disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel, except in accordance with the
 * applicable license agreement.
 * 
 * @par
 * Unless otherwise agreed by Intel in writing, you may not remove or
 * alter this notice or any other notice embedded in Materials by Intel
 * or Intel's suppliers or licensors in any way.
 * 
 * @par
 * For further details, please see the file README.TXT distributed with
 * this software.
 * 
 * @par
 * -- End Intel Copyright Notice --
 */

#if __linux
#ifdef __ixp46X
#include <linux/module.h>
#include "IxSspAcc.h"

EXPORT_SYMBOL(ixSspAccInit);
EXPORT_SYMBOL(ixSspAccUninit);
EXPORT_SYMBOL(ixSspAccFIFODataSubmit);
EXPORT_SYMBOL(ixSspAccFIFODataReceive);
EXPORT_SYMBOL(ixSspAccTxFIFOHitOrBelowThresholdCheck);
EXPORT_SYMBOL(ixSspAccRxFIFOHitOrAboveThresholdCheck);
EXPORT_SYMBOL(ixSspAccSSPPortStatusSet);
EXPORT_SYMBOL(ixSspAccFrameFormatSelect);
EXPORT_SYMBOL(ixSspAccDataSizeSelect);
EXPORT_SYMBOL(ixSspAccClockSourceSelect);
EXPORT_SYMBOL(ixSspAccSerialClockRateConfigure);
EXPORT_SYMBOL(ixSspAccRxFIFOIntEnable);
EXPORT_SYMBOL(ixSspAccRxFIFOIntDisable);
EXPORT_SYMBOL(ixSspAccTxFIFOIntEnable);
EXPORT_SYMBOL(ixSspAccTxFIFOIntDisable);
EXPORT_SYMBOL(ixSspAccLoopbackEnable);
EXPORT_SYMBOL(ixSspAccSpiSclkPolaritySet);
EXPORT_SYMBOL(ixSspAccSpiSclkPhaseSet);
EXPORT_SYMBOL(ixSspAccMicrowireControlWordSet);
EXPORT_SYMBOL(ixSspAccTxFIFOThresholdSet);
EXPORT_SYMBOL(ixSspAccRxFIFOThresholdSet);
EXPORT_SYMBOL(ixSspAccStatsGet);
EXPORT_SYMBOL(ixSspAccStatsReset);
EXPORT_SYMBOL(ixSspAccShow);
EXPORT_SYMBOL(ixSspAccSSPBusyCheck);
EXPORT_SYMBOL(ixSspAccTxFIFOLevelGet);
EXPORT_SYMBOL(ixSspAccRxFIFOLevelGet);
EXPORT_SYMBOL(ixSspAccRxFIFOOverrunCheck);
#endif /* __ixp46X */
#endif /* __linux */
