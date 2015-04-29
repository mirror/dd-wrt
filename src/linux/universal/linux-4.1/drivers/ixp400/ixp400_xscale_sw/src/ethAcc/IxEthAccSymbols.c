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
 * IXP400 SW Release version 2.4
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2007, Intel Corporation.
 * All rights reserved.
 * 
 * @par
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * 
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * 
 * @par
 * -- End of Copyright Notice --
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
EXPORT_SYMBOL(ixEthAccPortDisable);/*
EXPORT_SYMBOL(ixEthAccPortEnabledQuery);*/
EXPORT_SYMBOL(ixEthAccPortPromiscuousModeClear);
EXPORT_SYMBOL(ixEthAccPortPromiscuousModeSet);
EXPORT_SYMBOL(ixEthAccPortUnicastMacAddressSet);
EXPORT_SYMBOL(ixEthAccPortUnicastMacAddressGet);
EXPORT_SYMBOL(ixEthAccPortMulticastAddressJoin);
EXPORT_SYMBOL(ixEthAccPortMulticastAddressJoinAll);
EXPORT_SYMBOL(ixEthAccPortMulticastAddressLeave);
EXPORT_SYMBOL(ixEthAccPortMulticastAddressLeaveAll);/*
EXPORT_SYMBOL(ixEthAccPortUnicastAddressShow);
EXPORT_SYMBOL(ixEthAccPortMulticastAddressShow);*/
EXPORT_SYMBOL(ixEthAccPortDuplexModeSet);
EXPORT_SYMBOL(ixEthAccPortDuplexModeGet);
EXPORT_SYMBOL(ixEthAccPortTxFrameAppendFCSEnable);
EXPORT_SYMBOL(ixEthAccPortTxFrameAppendFCSDisable);
EXPORT_SYMBOL(ixEthAccPortRxFrameAppendFCSEnable);
EXPORT_SYMBOL(ixEthAccPortRxFrameAppendFCSDisable);/*
EXPORT_SYMBOL(ixEthAccPortTxFrameAppendPaddingEnable);
EXPORT_SYMBOL(ixEthAccPortTxFrameAppendPaddingDisable);
EXPORT_SYMBOL(ixEthAccPortRxEnable);
EXPORT_SYMBOL(ixEthAccPortRxDisable);
EXPORT_SYMBOL(ixEthAccPortTxEnable);
EXPORT_SYMBOL(ixEthAccPortTxDisable);
EXPORT_SYMBOL(ixEthAccPortLoopbackEnable);
EXPORT_SYMBOL(ixEthAccPortLoopbackDisable);
EXPORT_SYMBOL(ixEthAccStopRequest);
EXPORT_SYMBOL(ixEthAccStartRequest);
EXPORT_SYMBOL(ixEthAccStopDoneCheck);*/
EXPORT_SYMBOL(ixEthAccTxSchedulingDisciplineSet);
EXPORT_SYMBOL(ixEthAccRxSchedulingDisciplineSet);
EXPORT_SYMBOL(ixEthAccMibIIStatsGet);
EXPORT_SYMBOL(ixEthAccMibIIStatsGetClear);/*
EXPORT_SYMBOL(ixEthAccMibIIStatsClear);
EXPORT_SYMBOL(ixEthAccDataPlaneShow);
EXPORT_SYMBOL(ixEthAccMiiStatsShow);*/
EXPORT_SYMBOL(ixEthAccMiiReadRtn);
EXPORT_SYMBOL(ixEthAccMiiWriteRtn);/*
EXPORT_SYMBOL(ixEthAccStatsShow);
EXPORT_SYMBOL(ixEthAccMacInit);
EXPORT_SYMBOL(ixEthAccMacUninit); */
EXPORT_SYMBOL(ixEthAccUninit);
/*
EXPORT_SYMBOL(ixEthAccPortNpeLoopbackEnable);
EXPORT_SYMBOL(ixEthAccPortNpeLoopbackDisable);
EXPORT_SYMBOL(ixEthAccPortMacReset);
EXPORT_SYMBOL(ixEthAccMacStateRestore);
EXPORT_SYMBOL(ixEthAccQMStatusUpdate);*/
EXPORT_SYMBOL(ixEthAccQMgrRxNotificationEnable);
EXPORT_SYMBOL(ixEthAccQMgrRxNotificationDisable);/*
EXPORT_SYMBOL(ixEthAccQMgrRxQEntryGet);

EXPORT_SYMBOL(ixEthAccMiiPortIdPhyAddrSet);
extern UINT32 ixEthDBAddressCompare(UINT8 *mac1, UINT8 *mac2);
EXPORT_SYMBOL(ixEthDBAddressCompare);              
EXPORT_SYMBOL(ixEthRxMultiBufferQMCallback);
EXPORT_SYMBOL(ixEthRxFrameQMCallback);
EXPORT_SYMBOL(ixEthAccRxMultiBufferPriorityPoll);*/
EXPORT_SYMBOL(ixEthAccRxPriorityPoll);
EXPORT_SYMBOL(ixEthTxFrameDoneQMCallback);/*
EXPORT_SYMBOL(IxEthNpePortIdTable);
EXPORT_SYMBOL(IxEthNpePortIndexTable);
EXPORT_SYMBOL(IxEthEthPortIdToLogicalIdTable);
EXPORT_SYMBOL(IxEthEthPortIdToPhyAddressTable);
EXPORT_SYMBOL(IxEthAccPortInfo);
*/
#endif /* __linux */
