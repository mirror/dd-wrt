/**
 * @file IxOsalOsSpinLock.c (linux)
 *
 * @brief Implementation for spinlocks
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
	if(slock == NULL)
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, 
				IX_OSAL_LOG_DEV_STDOUT,
				"ixOsalSpinLockInit: Invalid SpinLock...\n", 
				0,0,0,0,0,0);
		return IX_FAIL;
	}/* SpinLock validity check. */

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
	if(slock == NULL)
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, 
				IX_OSAL_LOG_DEV_STDOUT,
				"ixOsalSpinLockLock: Invalid SpinLock Pointer ...\n", 
				0,0,0,0,0,0);
		return IX_FAIL;
	}/* SpinLock validity check. */

	spin_lock (slock); /* kernel function call */
	
	return IX_SUCCESS;
}

/**
 ***********************************************************
 * @function: ixOsalSpinLockUnlock
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
	if(slock == NULL)
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, 
				IX_OSAL_LOG_DEV_STDOUT,
				"ixOsalSpinLockUnlock: Invalid SpinLock Pointer...\n", 
				0,0,0,0,0,0);
		return IX_FAIL;
	}/* SpinLock validity check. */

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
	if(slock == NULL)
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, 
				IX_OSAL_LOG_DEV_STDOUT,
				"ixOsalSpinLockTry: Invalid SpinLock Pointer...\n", 
				0,0,0,0,0,0);
		return IX_FAIL;
	}

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
	if (slock == NULL)
	{
		ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE, 
				IX_OSAL_LOG_DEV_STDOUT,
				"ixOsalSpinLockDestroy: Invalid Spinlock Pointer.\n", 
				0,0,0,0,0,0);						
		return IX_SUCCESS;
	}

	return IX_SUCCESS;
}
