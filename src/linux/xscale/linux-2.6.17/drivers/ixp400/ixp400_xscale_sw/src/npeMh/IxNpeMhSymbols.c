/**
 * @file IxNpeMhSymbols.c
 *
 * @author Intel Corporation
 * @date 04-Oct-2002
 *
 * @brief This file declares exported symbols for linux kernel module builds.
 *
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

#ifdef __linux

#include <linux/module.h>
#include <IxNpeMh.h>

#include "IxNpeMhConfig_p.h"

EXPORT_SYMBOL(ixNpeMhInitialize);
EXPORT_SYMBOL(ixNpeMhUnload);
EXPORT_SYMBOL(ixNpeMhUnsolicitedCallbackRegister);
EXPORT_SYMBOL(ixNpeMhUnsolicitedCallbackForRangeRegister);
EXPORT_SYMBOL(ixNpeMhMessageSend);
EXPORT_SYMBOL(ixNpeMhMessageWithResponseSend);
EXPORT_SYMBOL(ixNpeMhMessagesReceive);
EXPORT_SYMBOL(ixNpeMhShow);
EXPORT_SYMBOL(ixNpeMhShowReset);
/*
extern void ixNpeMhConfigIsr (void *parameter);
extern BOOL ixNpeMhConfigInFifoIsFull(IxNpeMhNpeId npeId);
extern BOOL ixNpeMhConfigOutFifoIsEmpty (IxNpeMhNpeId npeId);
extern void ixNpeMhConfigLockRelease (IxNpeMhNpeId npeId);
extern void ixNpeMhConfigLockGet (IxNpeMhNpeId npeId);
extern void ixNpeMhConfigOutFifoRead (IxNpeMhNpeId npeId,IxNpeMhMessage *message);
extern void ixNpeMhConfigInFifoWrite (IxNpeMhNpeId npeId,IxNpeMhMessage message);
extern struct IxNpeMhConfigNpeInfo ixNpeMhConfigNpeInfo[IX_NPEMH_NUM_NPES];

EXPORT_SYMBOL(ixNpeMhConfigIsr);
*/
EXPORT_SYMBOL(ixNpeMhConfigInFifoIsFull);
EXPORT_SYMBOL(ixNpeMhConfigOutFifoIsEmpty);
EXPORT_SYMBOL(ixNpeMhConfigLockRelease);
EXPORT_SYMBOL(ixNpeMhConfigLockGet);
EXPORT_SYMBOL(ixNpeMhConfigOutFifoRead);
EXPORT_SYMBOL(ixNpeMhConfigInFifoWrite);
EXPORT_SYMBOL(ixNpeMhConfigNpeInfo);

#endif /* __linux */
