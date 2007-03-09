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
#include <IxQMgr.h>
#include <IxQMgrAqmIf_p.h>
#include <IxQMgrQAccess_p.h>

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


#endif /* __linux */
