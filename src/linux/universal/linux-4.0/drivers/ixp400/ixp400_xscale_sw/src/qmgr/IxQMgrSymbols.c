/**
 * @file IxQMgrSymbols.c
 *
 * @author Intel Corporation
 * @date 26-Jan-2006
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
#include "IxQMgr_sp.h"
#include "IxQMgrQAccess_p.h"
#include "IxQMgrQCfg_p.h"
#include "IxQMgrHwQIfIxp400_p.h"
			    
EXPORT_SYMBOL(ixQMgrInit);/*
EXPORT_SYMBOL(ixQMgrUnload);
EXPORT_SYMBOL(ixQMgrShow);
EXPORT_SYMBOL(ixQMgrQShow);
EXPORT_SYMBOL(ixQMgrQConfig);
EXPORT_SYMBOL(ixQMgrQUnconfig);
EXPORT_SYMBOL(ixQMgrQSizeInEntriesGet);
EXPORT_SYMBOL(ixQMgrWatermarkSet);
EXPORT_SYMBOL(ixQMgrAvailableSramAddressGet);
EXPORT_SYMBOL(ixQMgrAvailableSramAddressInBlockGet);
EXPORT_SYMBOL(ixQMgrQRead);
EXPORT_SYMBOL(ixQMgrQPeek);
EXPORT_SYMBOL(ixQMgrQWrite);
EXPORT_SYMBOL(ixQMgrQReadWithChecks);
EXPORT_SYMBOL(ixQMgrQWriteWithChecks);
EXPORT_SYMBOL(ixQMgrQPoke);
EXPORT_SYMBOL(ixQMgrQNumEntriesGet);
EXPORT_SYMBOL(ixQMgrQStatusGet);
EXPORT_SYMBOL(ixQMgrQStatusGetWithChecks);*/
EXPORT_SYMBOL(ixQMgrDispatcherPrioritySet);
EXPORT_SYMBOL(ixQMgrNotificationEnable);
EXPORT_SYMBOL(ixQMgrNotificationDisable);
EXPORT_SYMBOL(ixQMgrDispatcherLoopGet);/*
EXPORT_SYMBOL(ixQMgrNotificationCallbackSet);
EXPORT_SYMBOL(ixQMgrHwQIfBaseAddressSet);
EXPORT_SYMBOL(ixQMgrQReadMWordsMinus1);
EXPORT_SYMBOL(ixQMgrHwQIfQInterruptRegRead);
EXPORT_SYMBOL(ixQMgrHwQIfQInterruptRegWrite);
EXPORT_SYMBOL(ixQMgrDispatcherLoopRunB0);
EXPORT_SYMBOL(ixQMgrDispatcherLoopRunB0LLP);
EXPORT_SYMBOL(ixQMgrStickyInterruptRegEnable);*/
EXPORT_SYMBOL(ixQMgrCallbackTypeSet);/*
EXPORT_SYMBOL(ixQMgrCallbackTypeGet);
EXPORT_SYMBOL(ixQMgrPeriodicDone);
EXPORT_SYMBOL(ixQMgrLLPShow);
*/
extern volatile UINT32 * ixQMgrHwQIfQueAccRegAddr[];
//EXPORT_SYMBOL(ixQMgrHwQIfQueAccRegAddr);

extern UINT32 ixQMgrHwQIfQueLowStatRegAddr[];
extern UINT32 ixQMgrHwQIfQueLowStatBitsOffset[];
extern UINT32 ixQMgrHwQIfQueLowStatBitsMask;
extern UINT32 ixQMgrHwQIfQueUppStat0RegAddr;
extern UINT32 ixQMgrHwQIfQueUppStat1RegAddr;
extern UINT32 ixQMgrHwQIfQueUppStat0BitMask[];
extern UINT32 ixQMgrHwQIfQueUppStat1BitMask[];
/*
EXPORT_SYMBOL(ixQMgrHwQIfQueLowStatRegAddr);
EXPORT_SYMBOL(ixQMgrHwQIfQueLowStatBitsOffset);
EXPORT_SYMBOL(ixQMgrHwQIfQueLowStatBitsMask);
EXPORT_SYMBOL(ixQMgrHwQIfQueUppStat0RegAddr);
EXPORT_SYMBOL(ixQMgrHwQIfQueUppStat1RegAddr);
EXPORT_SYMBOL(ixQMgrHwQIfQueUppStat0BitMask);
EXPORT_SYMBOL(ixQMgrHwQIfQueUppStat1BitMask);
*/
extern UINT32 hwQBaseAddress;
//EXPORT_SYMBOL(hwQBaseAddress);

extern BOOL qMgrIsInitialized;
//EXPORT_SYMBOL(qMgrIsInitialized);

extern IxQMgrQInlinedReadWriteInfo ixQMgrQInlinedReadWriteInfo[];
/*
EXPORT_SYMBOL(ixQMgrQInlinedReadWriteInfo);
EXPORT_SYMBOL(ixQMgrDispatcherLoopEnable);
EXPORT_SYMBOL(ixQMgrDispatcherLoopDisable);
EXPORT_SYMBOL(ixQMgrDispatcherLoopStatusGet);*/
EXPORT_SYMBOL(ixQMgrDispatcherInterruptModeSet);

#endif /* __linux */
