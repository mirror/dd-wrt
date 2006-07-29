/**
 * @file IxQMgrSymbols.c
 *
 * @author Intel Corporation
 * @date 04-Oct-2002
 *
 * @brief This file declares exported symbols for linux kernel module builds.
 *
 * 
 * @par
 * IXP400 SW Release Crypto version 2.1
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2005, Intel Corporation.
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
#include <IxQMgr.h>
#include <IxQMgrAqmIf_p.h>
#include <IxQMgrQAccess_p.h>

#include "IxQMgrQCfg_p.h"
#include "IxQMgrAqmIf_p.h"


extern IxQMgrQInlinedReadWriteInfo ixQMgrQInlinedReadWriteInfo[];

EXPORT_SYMBOL(ixQMgrInit);
EXPORT_SYMBOL(ixQMgrUnload);
EXPORT_SYMBOL(ixQMgrShow);
EXPORT_SYMBOL(ixQMgrQShow);
EXPORT_SYMBOL(ixQMgrQConfig);
EXPORT_SYMBOL(ixQMgrQSizeInEntriesGet);
EXPORT_SYMBOL(ixQMgrWatermarkSet);
EXPORT_SYMBOL(ixQMgrAvailableSramAddressGet);
EXPORT_SYMBOL(ixQMgrQRead);
EXPORT_SYMBOL(ixQMgrQPeek);
EXPORT_SYMBOL(ixQMgrQWrite);
EXPORT_SYMBOL(ixQMgrQReadWithChecks);
EXPORT_SYMBOL(ixQMgrQWriteWithChecks);
EXPORT_SYMBOL(ixQMgrQPoke);
EXPORT_SYMBOL(ixQMgrQNumEntriesGet);
EXPORT_SYMBOL(ixQMgrQStatusGet);
EXPORT_SYMBOL(ixQMgrDispatcherPrioritySet);
EXPORT_SYMBOL(ixQMgrNotificationEnable);
EXPORT_SYMBOL(ixQMgrNotificationDisable);
EXPORT_SYMBOL(ixQMgrDispatcherLoopRunA0);
EXPORT_SYMBOL(ixQMgrDispatcherLoopRunB0);
EXPORT_SYMBOL(ixQMgrDispatcherLoopRunB0LLP);
EXPORT_SYMBOL(ixQMgrDispatcherLoopGet);
EXPORT_SYMBOL(ixQMgrStickyInterruptRegEnable);
EXPORT_SYMBOL(ixQMgrNotificationCallbackSet);
EXPORT_SYMBOL(ixQMgrCallbackTypeSet);
EXPORT_SYMBOL(ixQMgrCallbackTypeGet);
EXPORT_SYMBOL(ixQMgrPeriodicDone);
EXPORT_SYMBOL(ixQMgrLLPShow);
EXPORT_SYMBOL(ixQMgrAqmIfBaseAddressSet);
EXPORT_SYMBOL(ixQMgrQReadMWordsMinus1);
EXPORT_SYMBOL(ixQMgrAqmIfQInterruptRegRead);
EXPORT_SYMBOL(ixQMgrAqmIfQInterruptRegWrite);

extern volatile UINT32 * ixQMgrAqmIfQueAccRegAddr[];
extern UINT32 ixQMgrAqmIfQueLowStatRegAddr[];
extern UINT32 ixQMgrAqmIfQueLowStatBitsOffset[];
extern UINT32 ixQMgrAqmIfQueLowStatBitsMask;
extern UINT32 ixQMgrAqmIfQueUppStat0RegAddr;
extern UINT32 ixQMgrAqmIfQueUppStat1RegAddr;
extern UINT32 ixQMgrAqmIfQueUppStat0BitMask[];
extern UINT32 ixQMgrAqmIfQueUppStat1BitMask[];
extern UINT32 aqmBaseAddress;
extern BOOL qMgrIsInitialized;

EXPORT_SYMBOL(ixQMgrAqmIfQueAccRegAddr);
EXPORT_SYMBOL(ixQMgrAqmIfQueLowStatRegAddr);
EXPORT_SYMBOL(ixQMgrAqmIfQueLowStatBitsOffset);
EXPORT_SYMBOL(ixQMgrAqmIfQueLowStatBitsMask);
EXPORT_SYMBOL(ixQMgrAqmIfQueUppStat0RegAddr);
EXPORT_SYMBOL(ixQMgrAqmIfQueUppStat1RegAddr);
EXPORT_SYMBOL(ixQMgrAqmIfQueUppStat0BitMask);
EXPORT_SYMBOL(ixQMgrAqmIfQueUppStat1BitMask);
EXPORT_SYMBOL(ixQMgrQInlinedReadWriteInfo);
EXPORT_SYMBOL(aqmBaseAddress);
EXPORT_SYMBOL(qMgrIsInitialized);
EXPORT_SYMBOL(ixQMgrQEntrySizeInWordsGet);
EXPORT_SYMBOL(ixQMgrAqmIfBaseAddressGet);
EXPORT_SYMBOL(ixQMgrQSizeInWordsGet);
extern IX_STATUS
ixQMgrQStatusGetWithChecks (IxQMgrQId qId,
                            IxQMgrQStatus *qStatus);

EXPORT_SYMBOL(ixQMgrQStatusGetWithChecks);


#endif /* __linux */
