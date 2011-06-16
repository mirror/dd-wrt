/**
 * @file IxOsalOsSpinLock.c (linux)
 *
 * @brief Implementation for spinlocks
 *
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

/**
 ***********************************************************
 * @function: ixOsalSpinLockInit
 * 
 * @param: 	slock - IN - pointer to a spinlock_t type
 * @param: 	slockType - IN - not used
 *
 * @return: IX_STATUS - IX_SUCCESS or IX_FAIL
 * 
 * @brief: 	Initialize Spin Lock. 
 ***********************************************************
 */
PUBLIC IX_STATUS 
ixOsalSpinLockInit(IxOsalSpinLock *slock, IxOsalSpinLockType slockType)
{
    IX_OSAL_LOCAL_ENSURE(slock, 
                "ixOsalSpinLockInit():   Null spinlock pointer", 
                IX_FAIL);

    /* Spinlock type is ignored in case of Linux */
    spin_lock_init (slock); /* Kernel function call */

    return IX_SUCCESS;
}

/**
 ***********************************************************
 * @function: ixOsalSpinLockLock
 * 
 * @param: 	slock - IN - pointer to a spinlock_t type
 *
 * @return: IX_STATUS - IX_SUCCESS or IX_FAIL
 * 
 * @brief:  Acquire the basic SpinLock
 ***********************************************************
 */
PUBLIC IX_STATUS 
ixOsalSpinLockLock(IxOsalSpinLock *slock)
{
    IX_OSAL_LOCAL_ENSURE(slock, 
            "ixOsalSpinLockLock():   Null spinlock pointer", 
            IX_FAIL);

    spin_lock (slock); /* kernel function call */

    return IX_SUCCESS;
}

/**
 ***********************************************************
 * 
 * @param: 	slock - IN - pointer to a spinlock_t type
 *
 * @return: IX_STATUS - IX_SUCCESS or IX_FAIL
 * 
 * @brief:  Release the SpinLock.
 ***********************************************************
 */
PUBLIC IX_STATUS 
ixOsalSpinLockUnlock(IxOsalSpinLock *slock)
{
    IX_OSAL_LOCAL_ENSURE(slock, 
            "ixOsalSpinLockUnlock():   Null spinlock pointer", 
            IX_FAIL);

    spin_unlock (slock); /* kernel function call */

    return IX_SUCCESS;
}

/**
 ***********************************************************
 * @function: ixOsalSpinLockTry
 * 
 * @param: 	slock - IN - pointer to a spinlock_t type
 *
 * @return: IX_STATUS - IX_SUCCESS or IX_FAIL
 * 
 * @brief:  Try to acquire the SpinLock.
 ***********************************************************
 */
PUBLIC IX_STATUS 
ixOsalSpinLockTry(IxOsalSpinLock *slock)
{
    IX_OSAL_LOCAL_ENSURE(slock, 
            "ixOsalSpinLockTry(): Null spinlock pointer", 
            IX_FAIL);

    /* kernel function call */
    return spin_trylock(slock) ? IX_SUCCESS : IX_FAIL;
}

/**
 ***********************************************************
 * @function: ixOsalSpinLockDestroy
 * 
 * @param: 	slock - IN - pointer to a spinlock_t type
 *
 * @return: IX_STATUS - IX_SUCCESS or IX_FAIL
 * 
 * @brief:  Destroy the SpinLock. This is done by freeing
 * 			the memory allocated to Spinlock.
 ***********************************************************
 */
PUBLIC IX_STATUS 
ixOsalSpinLockDestroy(IxOsalSpinLock *slock)
{
    IX_OSAL_LOCAL_ENSURE(slock, 
            "ixOsalSpinLockDestroy():   Null spinlock pointer", 
            IX_FAIL);
    return IX_SUCCESS;
}

/**
 * @ingroup IxOsal
 *
 * @brief checks whether spinlock can be acquired
 *
 * @param slock - Spinlock handle
 *
 * This routine checks whether spinlock available for lock
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - IX_SUCCESS if spinlock is locked.  Returns IX_FAIL if spinlock
 * is not locked.
 */

PUBLIC IX_STATUS ixOsalSpinLockIsLocked(IxOsalSpinLock *slock)
{
    /* SpinLock  NULL pointer check. */
    IX_OSAL_LOCAL_ENSURE(slock, 
            "ixOsalSpinLockIsLocked():   NULL spinlock pointer", 
            IX_FAIL);
    
    return spin_is_locked(slock) ? IX_SUCCESS : IX_FAIL;
}


/**
 * @ingroup IxOsal
 *
 * @brief Acquires a spinlock
 *
 * @param slock - Spinlock handle
 *
 * This routine disables local irqs & then acquires a slock
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @usage  This API can be used in user context or bottom half when critical
 *         section is shared between user context or  bottom half and the
 *         irq handler
 *
 * @return - returns IX_SUCCESS if spinlock is acquired. If the spinlock is not
 *           available then it busy loops/spins till slock available.  If the
 *           spinlock handle passed is NULL then returns IX_FAIL.
 */
 
PUBLIC IX_STATUS ixOsalSpinLockLockIrq(IxOsalSpinLock *slock)
{
    /* SpinLock  NULL pointer check. */
    IX_OSAL_LOCAL_ENSURE(slock, 
            "ixOsalSpinLockLockIrq():   NULL spinlock pointer", 
            IX_FAIL);
    
    spin_lock_irq(slock);
    return IX_SUCCESS;
}

/**
 * @ingroup IxOsal
 *
 * @brief Releases the spinlock
 *
 * @param slock - Spinlock handle
 *
 * This routine releases the acquired spinlock & enables the local irqs
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @usage   This API can be used in user context or bottom half when critical
 *          section is shared between user context or  bottom half and
 *          irq handler
 *
 * @return - returns IX_SUCCESS if slock is unlocked. Returns IX_FAIL if the
 *           slock is NULL.
 */
PUBLIC IX_STATUS ixOsalSpinLockUnlockIrq(IxOsalSpinLock *slock)
{
    /* SpinLock  NULL pointer check. */
    IX_OSAL_LOCAL_ENSURE(slock, 
            "ixOsalSpinLockUnlockIrq():   Null spinlock pointer", 
            IX_FAIL);
    
    spin_unlock_irq(slock);
    return IX_SUCCESS;
}

/**
 * @ingroup IxOsal
 *
 * @brief Tries to acquire the spinlock
 *
 * @param slock - Spinlock handle
 *
 * This routine disables local irq & attempts to acquire a spinlock but
 * doesn't block the thread if spinlock not available.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @usage   This API can be used in user context or bottom half when critical
 *          section is shared between user context or  bottom half and
 *          irq handler
 *
 * @return -If spinlock is available then returns the IX_SUCCESS with
 *          spinlock locked. If spinlock not available then enables the
 *          local irqs & returns IX_FAIL
 *
 */
 
PUBLIC IX_STATUS ixOsalSpinLockTryIrq(IxOsalSpinLock *slock)
{
    /* SpinLock  NULL pointer check. */
    IX_OSAL_LOCAL_ENSURE(slock, 
            "ixOsalSpinLockTryIrq():   Null spinlock pointer", 
            IX_FAIL);

    return spin_trylock_irq(slock) ? IX_SUCCESS : IX_FAIL;

}

/**
 * @ingroup IxOsal
 *  
 * @brief Acquires a spinlock
 *  
 * @param slock - Spinlock handle
 *  
 * This routine disables bottom half & then acquires a slock
 *  
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @usage   This API can be used in user context when critical section is
 *           shared between user context & bottom half handler
 *
 * @return - returns IX_SUCCESS if spinlock is acquired. If the spinlock is not
 *           available then it busy loops/spins till slock available.  If the
 *           spinlock handle passed is NULL then returns IX_FAIL.
 */
 
PUBLIC IX_STATUS ixOsalSpinLockLockBh(IxOsalSpinLock *slock)
{
    /* SpinLock  NULL pointer check. */
    IX_OSAL_LOCAL_ENSURE(slock, 
            "ixOsalSpinLockLockBh():   Null spinlock pointer", 
            IX_FAIL);

    spin_lock_bh(slock);
    return IX_SUCCESS;

}

/**
 * @ingroup IxOsal
 *
 * @brief Releases the spin lock
 *
 * @param slock - Spinlock handle
 *
 * This routine releases the acquired spinlock & enables the 
 * bottom half handler
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @usage   This API can be used in user context when critical section is
 *           shared between user context & bottom half handler
 *
 * @return - returns IX_SUCCESS if slock is released or unlocked.
 *           Returns IX_FAIL if the slock is NULL.
 */
PUBLIC IX_STATUS ixOsalSpinLockUnlockBh(IxOsalSpinLock *slock)
{
    /* SpinLock  NULL pointer check. */
    IX_OSAL_LOCAL_ENSURE(slock, 
            "ixOsalSpinLockUnlockBh():   Null spinlock pointer", 
            IX_FAIL);
    
    spin_unlock_bh(slock);
    return IX_SUCCESS;
}

/**
 * @ingroup IxOsal
 *
 * @brief Tries to acquire the spin lock
 *
 * @param slock - Spinlock handle
 *
 * This routine disables bottom half handler & attempts to acquire a spinlock
 * but doesn't block the thread if spinlock not available. It enables the bh &
 * returns IX_FAIL if spinlock not available.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @usage   This API can be used in user context when critical section is
 *           shared between user context & bottom half handler
 *
 * @return -Returns the IX_SUCCESS with spinlock locked if the spinlock is
 *          available. Enables the local irqs  & return IX_FAIL
 *           if spinlock is not available.
 */
PUBLIC IX_STATUS ixOsalSpinLockTryBh(IxOsalSpinLock *slock)
{
    /* SpinLock  NULL pointer check. */
    IX_OSAL_LOCAL_ENSURE(slock, 
            "ixOsalSpinLockTryBh():   Null spinlock pointer",
            IX_FAIL);

    return spin_trylock_bh(slock) ? IX_SUCCESS : IX_FAIL;
}

/**
 * @ingroup IxOsal
 *
 * @brief Acquires a spinlock
 *
 * @param slock - Spinlock handle
 * @param flags - local irqs saved in flags
 *
 * @usage   This API can be used when critical section is shared between 
 *          irq routines
 *
 * This routine saves local irqs in flags & then acquires a spinlock
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - returns IX_SUCCESS if spinlock acquired. If the spinlock is not
 *           available then it busy loops/spins till slock available.
 *           If the spinlock handle passed is NULL then returns IX_FAIL.
 */
 
PUBLIC IX_STATUS ixOsalSpinLockLockIrqSave(IxOsalSpinLock *slock, 
                                           UINT32 *flags)
{
	unsigned long* _flags = (unsigned long*) flags;

    /* SpinLock  NULL pointer check. */
    IX_OSAL_LOCAL_ENSURE(slock, 
            "ixOsalSpinLockLockIrqSave():   Null spinlock pointer", 
            IX_FAIL);
    
    IX_OSAL_LOCAL_ENSURE(flags, 
            "ixOsalSpinLockLockIrqSave():   Null flags  pointer", 
            IX_FAIL);
#ifdef IX_OSAL_OS_LINUX_VERSION_2_6_18
    spin_lock_irqsave(slock, *flags);
#else
    spin_lock_irqsave(slock, *_flags);
#endif

    return IX_SUCCESS;
}

/**
 * @ingroup IxOsal
 *
 * @brief Releases the spin lock
 *
 * @param slock - Spinlock handle
 * @param flags - local irqs saved in flags
 *
 * @usage   This API can be used when critical section is shared between 
 *          irq routines
 *
 * This routine releases the acquired spin lock & restores irqs in flags
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - returns IX_SUCCESS if slock is unlocked. Returns IX_FAIL if the
 *           slock is NULL.
 */
PUBLIC IX_STATUS ixOsalSpinLockUnlockIrqRestore(IxOsalSpinLock *slock, 
                                                UINT32 *flags)
{
    /* SpinLock  NULL pointer check. */
    IX_OSAL_LOCAL_ENSURE(slock, 
            "ixOsalSpinLockUnlockIrqRestore():   Null spinlock pointer", 
            IX_FAIL);

    IX_OSAL_LOCAL_ENSURE(flags,
            "ixOsalSpinLockUnlockIrqRestore():   Null flags  pointer", 
            IX_FAIL);

#ifdef IX_OSAL_OS_LINUX_VERSION_2_6_18
    spin_unlock_irqrestore(slock, *flags);
#else
    spin_unlock_irqrestore(slock, (unsigned long)*flags);    
#endif

    return IX_SUCCESS;
}

/**
 * @ingroup IxOsal
 *
 * @brief Tries to acquire the spinlock
 *
 * @param slock - Spinlock handle
 * @param flags - local irqs saved in flags
 *
 * This routine saves irq in flags & attempts to acquire a spinlock but
 * doesn't block the thread if the spin lock not avialble. If the
 * spinlock not available then it restore the irqs & return IX_FAIL
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @usage   This API can be used when critical section is shared between 
 *          irq routines
 * 
 * @return -Returns the IX_SUCCESS with spinlock locked if the spinlock is
 *          available. Enables the local irqs & returns IX_FAIL 
 *           if spinlock not available.
 */
PUBLIC IX_STATUS ixOsalSpinLockTryIrqSave(IxOsalSpinLock *slock, UINT32 *flags)
{
	unsigned long *_flags = (unsigned long*) flags;

    /* SpinLock  NULL pointer check. */
    IX_OSAL_LOCAL_ENSURE(slock, 
            "ixOsalSpinLockTryIrqSave():   Null spinlock pointer", 
            IX_FAIL);

    IX_OSAL_LOCAL_ENSURE(flags, 
            "ixOsalSpinLockTryIrqSave():   Null flags  pointer", 
            IX_FAIL);
#ifdef IX_OSAL_OS_LINUX_VERSION_2_6_18
    return spin_trylock_irqsave(slock, *flags) ? IX_SUCCESS : IX_FAIL;
#else 
    return spin_trylock_irqsave(slock, *_flags) 
                                ? IX_SUCCESS : IX_FAIL;
#endif   
    
}
