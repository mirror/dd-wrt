/*******************************************************************************
 * 
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
 *             
 *****************************************************************************/

#if !defined(__IX_LOCKS_H__)
#define __IX_LOCKS_H__


#if defined(__cplusplus)
extern "C"
{
#endif /* end defined(__cplusplus) */

#if (_IX_OS_TYPE_ == _IX_OS_LINUX_KERNEL_)

    /* under LINUX KERNEL mode make all locks as spin locks */
#if defined (IX_RM_USE_NATIVE_BH_SPINLOCKS) 
    #define IX_LOCK_TYPE            IxOsalSpinLock
    #define IX_LOCK_T(lock)         IxOsalSpinLock lock
    #define IX_LOCK_INIT(lock)      ixOsalSpinLockInit(&lock, 1)
    #define IX_LOCK_FINI(lock)      ixOsalSpinLockDestroy(&lock)
    #define IX_LOCK(lock)           spin_lock_bh(&lock)
    #define IX_UNLOCK(lock)         spin_unlock_bh(&lock)
#else
    #define IX_LOCK_TYPE            IxOsalSpinLock
    #define IX_LOCK_T(lock)         IxOsalSpinLock lock
    #define IX_LOCK_INIT(lock)      ixOsalSpinLockInit(&lock, 1)
    #define IX_LOCK_FINI(lock)      ixOsalSpinLockDestroy(&lock)
    #define IX_LOCK(lock)           ixOsalSpinLockLockBh(&lock)
    #define IX_UNLOCK(lock)         ixOsalSpinLockUnlockBh(&lock)
#endif /* defined (IX_RM_USE_NATIVE_BH_SPINLOCKS) */   

#else /* (_IX_OS_TYPE_ == _IX_OS_WIN32_) || ((_IX_OS_TYPE_ == _IX_OS_LINUX_USER_) */
    
    /* under LINUX USER & WIN32 mode make all locks as semaphores */
   
    #define IX_LOCK_TYPE            IxOsalSemaphore
    #define IX_LOCK_T(lock)         IxOsalSemaphore lock = 0
    #define IX_LOCK_INIT(lock)      ixOsalSemaphoreInit(&lock, IX_SEM_AVAILABLE)
    #define IX_LOCK_FINI(lock)      (lock!=0)? (ixOsalSemaphoreDestroy(&lock)||(lock=0)) : IX_FAIL
    #define IX_LOCK(lock)           ixOsalSemaphoreWaitInterruptible(&lock, IX_OSAL_WAIT_FOREVER)
    #define IX_UNLOCK(lock)         ixOsalSemaphorePostWakeup(&lock) 

#endif /* (_IX_OS_TYPE_ == _IX_OS_DIAGNOSTIC_) */





#if defined(__cplusplus)
}
#endif /* end defined(__cplusplus) */

#endif /* end !defined(__IX_LOCKS_H__) */
