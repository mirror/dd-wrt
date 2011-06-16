/*
 * @file        IxOsalOsSymbols.c
 * @author 	Intel Corporation
 * @date        25-08-2004
 *
 * @brief       description goes here
 *
 * @par
 * This file is provided under a dual BSD/GPLv2 license.  When using or 
 *   redistributing this file, you may do so under either license.
 * 
 *   GPL LICENSE SUMMARY
 * 
 *   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
 * 
 *   This program is free software; you can redistribute it and/or modify 
 *   it under the terms of version 2 of the GNU General Public License as
 *   published by the Free Software Foundation.
 * 
 *   This program is distributed in the hope that it will be useful, but 
 *   WITHOUT ANY WARRANTY; without even the implied warranty of 
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 *   General Public License for more details.
 * 
 *   You should have received a copy of the GNU General Public License 
 *   along with this program; if not, write to the Free Software 
 *   Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *   The full GNU General Public License is included in this distribution 
 *   in the file called LICENSE.GPL.
 * 
 *   Contact Information:
 *   Intel Corporation
 * 
 *   BSD LICENSE 
 * 
 *   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
 *   All rights reserved.
 * 
 *   Redistribution and use in source and binary forms, with or without 
 *   modification, are permitted provided that the following conditions 
 *   are met:
 * 
 *     * Redistributions of source code must retain the above copyright 
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright 
 *       notice, this list of conditions and the following disclaimer in 
 *       the documentation and/or other materials provided with the 
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its 
 *       contributors may be used to endorse or promote products derived 
 *       from this software without specific prior written permission.
 * 
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * 
 *  version: Security.L.1.0.3-98
 */
#include "IxOsal.h"

#ifdef IX_OSAL_MODULE
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

MODULE_LICENSE("Dual BSD/GPL");
#endif

#ifdef OSAL_EXPORT_SYMBOLS
EXPORT_SYMBOL (ixOsalMemAlloc);
EXPORT_SYMBOL (ixOsalMemFree);
EXPORT_SYMBOL (ixOsalMemCopy);
EXPORT_SYMBOL (ixOsalMemSet);
EXPORT_SYMBOL (ixOsalMemCmp);


EXPORT_SYMBOL (ixOsalThreadCreate);
EXPORT_SYMBOL (ixOsalThreadStart);
EXPORT_SYMBOL (ixOsalThreadKill);
EXPORT_SYMBOL (ixOsalThreadExit);
EXPORT_SYMBOL (ixOsalThreadPrioritySet);
EXPORT_SYMBOL (ixOsalThreadSuspend);
EXPORT_SYMBOL (ixOsalThreadResume);
EXPORT_SYMBOL (ixOsalThreadGetPolicyAndPriority);

#ifdef IX_OSAL_THREAD_EXIT_GRACEFULLY
EXPORT_SYMBOL (ixOsalThreadStopCheck);
#endif /* IX_OSAL_THREAD_EXIT_GRACEFULLY */

EXPORT_SYMBOL (ixOsalMessageQueueCreate);
EXPORT_SYMBOL (ixOsalMessageQueueDelete);
EXPORT_SYMBOL (ixOsalMessageQueueSend);
EXPORT_SYMBOL (ixOsalMessageQueueReceive);
EXPORT_SYMBOL (ixOsalSyncMessageQueueReceive);
EXPORT_SYMBOL (ixOsalSyncMessageQueueCreate);

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
EXPORT_SYMBOL (ixOsalSemaphoreWaitInterruptible);
EXPORT_SYMBOL (ixOsalSemaphorePostWakeup);
EXPORT_SYMBOL (ixOsalSemaphoreFlush);
EXPORT_SYMBOL (ixOsalSleepTick);
EXPORT_SYMBOL (ixOsalSleepUninterruptible);
EXPORT_SYMBOL (ixOsalSemaphoreDownTimeout);

EXPORT_SYMBOL (ixOsalYield);
EXPORT_SYMBOL (ixOsalSleep);
EXPORT_SYMBOL (ixOsalBusySleep);
EXPORT_SYMBOL (ixOsalTimeGet);
EXPORT_SYMBOL (ixOsalTimevalToTicks);
EXPORT_SYMBOL (ixOsalTicksToTimeval);

EXPORT_SYMBOL (ixOsalLog);
EXPORT_SYMBOL (ixOsalStdLog);
EXPORT_SYMBOL (ixOsalLogLevelSet);
EXPORT_SYMBOL (ixOsalLogSetPrefix);
EXPORT_SYMBOL (ixOsalRepeatingTimerSchedule);
EXPORT_SYMBOL (ixOsalSingleShotTimerSchedule);
EXPORT_SYMBOL (ixOsalTimerCancel);
EXPORT_SYMBOL (ixOsalTimersShow);

EXPORT_SYMBOL (ixOsalOsNameGet);
EXPORT_SYMBOL (ixOsalOsVersionGet);

/* New Functions */
EXPORT_SYMBOL (ixOsalThreadGetId);


#ifdef ENABLE_SPINLOCK

EXPORT_SYMBOL (ixOsalSpinLockInit);
EXPORT_SYMBOL (ixOsalSpinLockLock);
EXPORT_SYMBOL (ixOsalSpinLockUnlock);
EXPORT_SYMBOL (ixOsalSpinLockTry);
EXPORT_SYMBOL (ixOsalSpinLockDestroy);
EXPORT_SYMBOL (ixOsalSpinLockIsLocked);
EXPORT_SYMBOL (ixOsalSpinLockLockBh);
EXPORT_SYMBOL (ixOsalSpinLockUnlockBh);
EXPORT_SYMBOL (ixOsalSpinLockTryBh);
EXPORT_SYMBOL (ixOsalSpinLockLockIrq);
EXPORT_SYMBOL (ixOsalSpinLockUnlockIrq);
EXPORT_SYMBOL (ixOsalSpinLockTryIrq);
EXPORT_SYMBOL (ixOsalSpinLockLockIrqSave);
EXPORT_SYMBOL (ixOsalSpinLockUnlockIrqRestore);
EXPORT_SYMBOL (ixOsalSpinLockTryIrqSave);

#endif /* ENABLE_SPINLOCK */

EXPORT_SYMBOL (ixOsalAtomicGet);
EXPORT_SYMBOL (ixOsalAtomicSet);
EXPORT_SYMBOL (ixOsalAtomicAdd);
EXPORT_SYMBOL (ixOsalAtomicSub);
EXPORT_SYMBOL (ixOsalAtomicSubAndTest);
EXPORT_SYMBOL (ixOsalAtomicInc);
EXPORT_SYMBOL (ixOsalAtomicDec);
EXPORT_SYMBOL (ixOsalAtomicDecAndTest);
EXPORT_SYMBOL (ixOsalAtomicIncAndTest);

EXPORT_SYMBOL (ixOsalMemBarrier);
EXPORT_SYMBOL (ixOsalReadMemBarrier);
EXPORT_SYMBOL (ixOsalWriteMemBarrier);

EXPORT_SYMBOL (ixOsalGetRandomNum32);
EXPORT_SYMBOL (ixOsalMemAllocAtomic);
EXPORT_SYMBOL (ixOsalMemAllocAligned);
EXPORT_SYMBOL (ixOsalMemAlignedFree);

#endif /* OSAL_EXPORT_SYMBOLS */

#ifdef IX_OSAL_MODULE

static  int  __init osal_init( void )
{
        printk( "Loading OSAL Module ...\n" ) ;
            return 0;
}
 
 
static void __exit osal_exit( void )
{
        printk("Unloading OSAL Module ...\n" ) ;
}
 
module_init(osal_init);
module_exit(osal_exit);

#endif

