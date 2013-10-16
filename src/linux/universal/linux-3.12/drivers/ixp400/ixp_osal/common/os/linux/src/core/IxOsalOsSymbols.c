/*
 * @file        IxOsalOsSymbols.c
 * @author 	Intel Corporation
 * @date        25-08-2004
 *
 * @brief       description goes here
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

#ifdef IX_OSAL_MODULE
#include <linux/init.h>
#endif
#include <linux/module.h>
#include <linux/moduleparam.h>

#include "IxOsal.h"

/*
EXPORT_SYMBOL (ixOsalMemAlloc);
EXPORT_SYMBOL (ixOsalMemFree);
EXPORT_SYMBOL (ixOsalMemCopy);
EXPORT_SYMBOL (ixOsalMemSet);


EXPORT_SYMBOL (ixOsalThreadCreate);
EXPORT_SYMBOL (ixOsalThreadStart);
EXPORT_SYMBOL (ixOsalThreadKill);
EXPORT_SYMBOL (ixOsalThreadExit);
EXPORT_SYMBOL (ixOsalThreadPrioritySet);
EXPORT_SYMBOL (ixOsalThreadSuspend);
EXPORT_SYMBOL (ixOsalThreadResume);

#ifdef IX_OSAL_THREAD_EXIT_GRACEFULLY
EXPORT_SYMBOL (ixOsalThreadStopCheck);
#endif 

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

EXPORT_SYMBOL (ixOsalYield);*/
EXPORT_SYMBOL (ixOsalSleep);/*
EXPORT_SYMBOL (ixOsalBusySleep);
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

#ifdef IX_OSAL_OSSL_SHIMLAYER_SUPPORT

EXPORT_SYMBOL (ixOsalThreadGetId);
EXPORT_SYMBOL (ixOsalThreadSetPolicyAndPriority);

EXPORT_SYMBOL (ixOsalSemaphoreWaitInterruptible);
EXPORT_SYMBOL (ixOsalSemaphorePostWakeup);
EXPORT_SYMBOL (ixOsalSemaphoreFlush);
EXPORT_SYMBOL (ixOsalSleepTick);
EXPORT_SYMBOL (ixOsalSleepUninterruptible);
#endif 

#ifdef ENABLE_SPINLOCK

EXPORT_SYMBOL (ixOsalSpinLockInit);
EXPORT_SYMBOL (ixOsalSpinLockLock);
EXPORT_SYMBOL (ixOsalSpinLockUnlock);
EXPORT_SYMBOL (ixOsalSpinLockTry);
EXPORT_SYMBOL (ixOsalSpinLockDestroy);

#endif

*/

#ifdef IX_OSAL_MODULE

static  int osal_init( void )
{
        printk( "Loading OSAL Module ...\n" ) ;
            return 0;
}
 
 
static void osal_exit( void )
{
        printk("Unloading OSAL Module ...\n" ) ;
}
 
module_init(osal_init);
module_exit(osal_exit);

#endif

