/**
 * @file IxEthAccSymbols.c
 *
 * @author Intel Corporation
 * @date 04-Oct-2002
 *
 * @brief This file declares exported symbols for linux kernel module builds.
 *
 * 
 * @par
 * IXP400 SW Release version  2.0
 * 
 * -- Intel Copyright Notice --
 * 
 * @par
 * Copyright 2002-2005 Intel Corporation All Rights Reserved.
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

#ifdef __linux

#include <linux/module.h>
#include "IxEthAcc.h"
#include "IxEthAccDataPlane_p.h"
#include "IxEthAccMac_p.h"

EXPORT_SYMBOL(ixEthAccInit);
EXPORT_SYMBOL(ixEthAccPortInit);
EXPORT_SYMBOL(ixEthAccPortTxFrameSubmit);
EXPORT_SYMBOL(ixEthAccPortTxDoneCallbackRegister);
EXPORT_SYMBOL(ixEthAccPortRxCallbackRegister);
EXPORT_SYMBOL(ixEthAccPortMultiBufferRxCallbackRegister);
EXPORT_SYMBOL(ixEthAccPortRxFreeReplenish);
EXPORT_SYMBOL(ixEthAccPortEnable);
EXPORT_SYMBOL(ixEthAccPortDisable);
EXPORT_SYMBOL(ixEthAccPortEnabledQuery);
EXPORT_SYMBOL(ixEthAccPortPromiscuousModeClear);
EXPORT_SYMBOL(ixEthAccPortPromiscuousModeSet);
EXPORT_SYMBOL(ixEthAccPortUnicastMacAddressSet);
EXPORT_SYMBOL(ixEthAccPortUnicastMacAddressGet);
EXPORT_SYMBOL(ixEthAccPortMulticastAddressJoin);
EXPORT_SYMBOL(ixEthAccPortMulticastAddressJoinAll);
EXPORT_SYMBOL(ixEthAccPortMulticastAddressLeave);
EXPORT_SYMBOL(ixEthAccPortMulticastAddressLeaveAll);
EXPORT_SYMBOL(ixEthAccPortUnicastAddressShow);
EXPORT_SYMBOL(ixEthAccPortMulticastAddressShow);
EXPORT_SYMBOL(ixEthAccPortDuplexModeSet);
EXPORT_SYMBOL(ixEthAccPortDuplexModeGet);
EXPORT_SYMBOL(ixEthAccPortTxFrameAppendFCSEnable);
EXPORT_SYMBOL(ixEthAccPortTxFrameAppendFCSDisable);
EXPORT_SYMBOL(ixEthAccPortRxFrameAppendFCSEnable);
EXPORT_SYMBOL(ixEthAccPortRxFrameAppendFCSDisable);
EXPORT_SYMBOL(ixEthAccPortTxFrameAppendPaddingEnable);
EXPORT_SYMBOL(ixEthAccPortTxFrameAppendPaddingDisable);
EXPORT_SYMBOL(ixEthAccPortRxEnable);
EXPORT_SYMBOL(ixEthAccPortRxDisable);
EXPORT_SYMBOL(ixEthAccPortTxEnable);
EXPORT_SYMBOL(ixEthAccPortTxDisable);
EXPORT_SYMBOL(ixEthAccPortLoopbackEnable);
EXPORT_SYMBOL(ixEthAccPortLoopbackDisable);
EXPORT_SYMBOL(ixEthAccTxSchedulingDisciplineSet);
EXPORT_SYMBOL(ixEthAccRxSchedulingDisciplineSet);
EXPORT_SYMBOL(ixEthAccMibIIStatsGet);
EXPORT_SYMBOL(ixEthAccMibIIStatsGetClear);
EXPORT_SYMBOL(ixEthAccMibIIStatsClear);
EXPORT_SYMBOL(ixEthAccDataPlaneShow);
EXPORT_SYMBOL(ixEthAccMiiStatsShow);
EXPORT_SYMBOL(ixEthAccMiiReadRtn);
EXPORT_SYMBOL(ixEthAccMiiWriteRtn);
EXPORT_SYMBOL(ixEthAccStatsShow);
EXPORT_SYMBOL(ixEthAccUnload);
EXPORT_SYMBOL(ixEthAccPortNpeLoopbackEnable);
EXPORT_SYMBOL(ixEthAccPortNpeLoopbackDisable);
EXPORT_SYMBOL(ixEthAccPortMacReset);

extern UINT32 ixEthDBAddressCompare(UINT8 *mac1, UINT8 *mac2);
EXPORT_SYMBOL(ixEthDBAddressCompare);              

EXPORT_SYMBOL(ixEthRxMultiBufferQMCallback);
EXPORT_SYMBOL(ixEthRxFrameQMCallback);
EXPORT_SYMBOL(ixEthTxFrameDoneQMCallback);

#endif /* __linux */
