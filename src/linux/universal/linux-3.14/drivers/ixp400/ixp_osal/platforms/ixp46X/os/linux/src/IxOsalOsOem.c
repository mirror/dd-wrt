/**
 * @file IxOsalOsOem.c  (linux)
 *
 * @brief this file contains implementation for platform-specific
 *        functionalities.
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

#include <linux/utsname.h>
#include "IxOsal.h"

#include "IxOsalOsOem.h"

static volatile UINT32 *ixOsalOstsRegAddr = NULL;

PRIVATE BOOL IxOsalOemInitialized = FALSE;

PUBLIC UINT32 ixOsalLinuxInterruptedPc = 0;

extern struct new_utsname system_utsname;

PUBLIC UINT32
ixOsalOsIxp400TimestampGet (void)
{
    IX_STATUS ixStatus;
    /*
     * ensure the register is I/O mapped 
     */
    if (IxOsalOemInitialized == FALSE)
    {
        /*
         * Assert if not success 
         */
        ixStatus = ixOsalOemInit ();
        IX_OSAL_ASSERT (ixStatus == IX_SUCCESS);
    }

    return IX_OSAL_READ_LONG (ixOsalOstsRegAddr);
}

PUBLIC UINT32
ixOsalOsIxp400TimestampResolutionGet (void)
{
   return (UINT32)IX_OSAL_IXP400_TIME_STAMP_RESOLUTION ;
}

UINT32
ixOsalOsIxp400SysClockRateGet (void)
{
    return HZ;
}

#ifdef ENABLE_IOMEM

PUBLIC IX_STATUS
ixOsalMemMapInit (IxOsalMemoryMap *map, UINT32 numElement)
{
    UINT32 i;
    static BOOL initialized = FALSE;

    if (initialized)
    {
	ixOsalLog (IX_OSAL_LOG_LVL_DEBUG3,
	    IX_OSAL_LOG_DEV_STDOUT, "ixOsalMemMapInit: Already initialized", 
	    0, 0, 0, 0, 0, 0);

    	return IX_SUCCESS;
    }

    for (i = 0; i < numElement; i++)
    {
    	IxOsalMemoryMap *localmap = &map[i];

	/* Add runtime determined dynamic expansion bus mapping info */
    	if (!strcmp(localmap->name, IX_OSAL_MEMMAP_RESERVED))
	{
	    localmap->type = IX_OSAL_DYNAMIC_MAP;
	    localmap->physicalAddress = IX_OSAL_IXP400_EXP_BUS_PHYS_BASE;
	    localmap->size = IX_OSAL_IXP400_EXP_BUS_MAP_SIZE;
	    localmap->virtualAddress = 0;
	    localmap->mapFunction = ixOsalLinuxMemMap;
	    localmap->unmapFunction = ixOsalLinuxMemUnmap;
	    localmap->refCount = 0;
	    localmap->mapEndianType = IX_OSAL_BE | IX_OSAL_LE_AC;
	    strcpy(localmap->name, "exp bus");

	    break;
	}
    }

    if (i == numElement)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalMemMapInit(): Fail to initialize memory map.\n", 
	    0, 0, 0, 0, 0, 0);

    	return IX_FAIL;
    }

    initialized = TRUE;

    return IX_SUCCESS;
}


#endif  /* ENABLE_IOMEM */

PUBLIC IX_STATUS
ixOsalOemInit (void)
{
    /*
     * Check flag 
     */
    if (IxOsalOemInitialized == TRUE)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_DEBUG3,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalOemInit():  already initialized. \n", 0, 0, 0, 0, 0, 0);
        return IX_SUCCESS;
    }

    ixOsalOstsRegAddr =
        (volatile UINT32 *)
        IX_OSAL_MEM_MAP (IX_OSAL_IXP400_OSTS_PHYS_BASE, 4);

    if (ixOsalOstsRegAddr == NULL)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalOemInit():  Fail to map time stamp register. \n", 0, 0, 0,
            0, 0, 0);
        return IX_FAIL;
    }

    IxOsalOemInitialized = TRUE;

    return IX_SUCCESS;
}

PUBLIC IX_STATUS
ixOsalOsIxp400NameGet(INT8* osName, INT32 maxSize)
{
    strncpy(osName, system_utsname.sysname, maxSize);
    return IX_SUCCESS;
}

PUBLIC IX_STATUS
ixOsalOsIxp400VersionGet(INT8* osVersion, INT32 maxSize)
{
    strncpy(osVersion, system_utsname.release, maxSize);
    return IX_SUCCESS;
}

PUBLIC void
ixOsalOemUnload (void)
{
    /*
     * Unmap the time stamp register 
     */
    IX_OSAL_MEM_UNMAP ((UINT32) ixOsalOstsRegAddr);
    IxOsalOemInitialized = FALSE;
}


/* Platform specific fast mutex tryLock */

/* ixOsalFastMutexTryLock */
asm (".align	5;"
    ".globl	ixOsalOemFastMutexTryLock;"
    "ixOsalOemFastMutexTryLock:;"
    "mov		r1, #1;"
    "swp		r2, r1, [r0];" "mov		r0, r2;" "mov		pc, lr;");

/*
 ===========================================================
                 Thread related functions 
 ===========================================================
 */ 


/* ==================== End - Thread ==================== */


/*
 ===========================================================
                 Semaphore related functions 
 ===========================================================
*/ 

IX_STATUS 
ixOsalixp4XXFastMutexInit (IxOsalFastMutex *mutex)
{        
    return mutex ? *mutex = 0, IX_SUCCESS : IX_FAIL;
}

IX_STATUS
ixOsalixp4XXFastMutexTryLock(IxOsalFastMutex *mutex)
{
    return ixOsalOemFastMutexTryLock(mutex);
}

IX_STATUS
ixOsalixp4XXFastMutexUnlock (IxOsalFastMutex * mutex)
{
     return mutex ? *mutex = 0, IX_SUCCESS : IX_FAIL;
}

IX_STATUS
ixOsalixp4XXFastMutexDestroy (IxOsalFastMutex * mutex)
{
    *mutex = 0;
	 
	     return IX_SUCCESS;
}

/* ================== End - Semaphore =================== */

/*
 ===========================================================
                      Irq related functions  
 ===========================================================
 */
					    
void
ixOsalixp4XXSetInterruptedPc(struct pt_regs *regs)
{
    ixOsalLinuxInterruptedPc = regs->ARM_pc;
}
							 
/* ===================== End - Irq ====================== */
