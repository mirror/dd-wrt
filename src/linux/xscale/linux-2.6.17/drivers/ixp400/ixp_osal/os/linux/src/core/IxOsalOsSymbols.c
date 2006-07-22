/*
 * @file        IxOsalOsSymbols.c 
 * @author 	Intel Corporation
 * @date        25-08-2004
 *
 * @brief       description goes here
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

#include <linux/module.h>
#include "IxOsal.h"

EXPORT_SYMBOL (ixOsalIrqBind);
EXPORT_SYMBOL (ixOsalIrqUnbind);
EXPORT_SYMBOL (ixOsalIrqLock);
EXPORT_SYMBOL (ixOsalIrqUnlock);
EXPORT_SYMBOL (ixOsalIrqLevelSet);
EXPORT_SYMBOL (ixOsalIrqEnable);
EXPORT_SYMBOL (ixOsalIrqDisable);

EXPORT_SYMBOL (ixOsalMemAlloc);
EXPORT_SYMBOL (ixOsalMemFree);
EXPORT_SYMBOL (ixOsalMemCopy);
EXPORT_SYMBOL (ixOsalMemSet);

EXPORT_SYMBOL (ixOsalCacheDmaMalloc);
EXPORT_SYMBOL (ixOsalCacheDmaFree);

EXPORT_SYMBOL (ixOsalThreadCreate);
EXPORT_SYMBOL (ixOsalThreadStart);
EXPORT_SYMBOL (ixOsalThreadKill);
EXPORT_SYMBOL (ixOsalThreadExit);
EXPORT_SYMBOL (ixOsalThreadPrioritySet);
EXPORT_SYMBOL (ixOsalThreadSuspend);
EXPORT_SYMBOL (ixOsalThreadResume);

EXPORT_SYMBOL (ixOsalMessageQueueCreate);
EXPORT_SYMBOL (ixOsalMessageQueueDelete);
EXPORT_SYMBOL (ixOsalMessageQueueSend);
EXPORT_SYMBOL (ixOsalMessageQueueReceive);

EXPORT_SYMBOL (ixOsalMutexInit);
EXPORT_SYMBOL (ixOsalMutexLock);
EXPORT_SYMBOL (ixOsalMutexUnlock);
EXPORT_SYMBOL (ixOsalMutexTryLock);
EXPORT_SYMBOL (ixOsalMutexDestroy);
EXPORT_SYMBOL (ixOsalFastMutexInit);
EXPORT_SYMBOL (ixOsalFastMutexTryLock);
EXPORT_SYMBOL (ixOsalFastMutexUnlock);
EXPORT_SYMBOL (ixOsalFastMutexDestroy);

EXPORT_SYMBOL (ixOsalSemaphoreInit);
EXPORT_SYMBOL (ixOsalSemaphorePost);
EXPORT_SYMBOL (ixOsalSemaphoreWait);
EXPORT_SYMBOL (ixOsalSemaphoreTryWait);
EXPORT_SYMBOL (ixOsalSemaphoreGetValue);
EXPORT_SYMBOL (ixOsalSemaphoreDestroy);

EXPORT_SYMBOL (ixOsalYield);
EXPORT_SYMBOL (ixOsalSleep);
EXPORT_SYMBOL (ixOsalBusySleep);
EXPORT_SYMBOL (ixOsalTimestampGet);
EXPORT_SYMBOL (ixOsalTimestampResolutionGet);
EXPORT_SYMBOL (ixOsalSysClockRateGet);
EXPORT_SYMBOL (ixOsalTimeGet);
EXPORT_SYMBOL (ixOsalTimevalToTicks);
EXPORT_SYMBOL (ixOsalTicksToTimeval);

EXPORT_SYMBOL (ixOsalLog);
EXPORT_SYMBOL (ixOsalLogLevelSet);
EXPORT_SYMBOL (ixOsalRepeatingTimerSchedule);
EXPORT_SYMBOL (ixOsalSingleShotTimerSchedule);
EXPORT_SYMBOL (ixOsalTimerCancel);
EXPORT_SYMBOL (ixOsalTimersShow);

EXPORT_SYMBOL (ixOsalOsNameGet);
EXPORT_SYMBOL (ixOsalOsVersionGet);
