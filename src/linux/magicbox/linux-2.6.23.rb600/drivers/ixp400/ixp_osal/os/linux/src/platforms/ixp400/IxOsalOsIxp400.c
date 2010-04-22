/**
 * @file IxOsalOsIxp400.c  (linux)
 *
 * @brief this file contains implementation for platform-specific
 *        functionalities.
 * 
 * 
 * @par
 * IXP400 SW Release Crypto version 2.3
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2005, Intel Corporation.
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

static volatile UINT32 *ixOsalOstsRegAddr = NULL;

PRIVATE BOOL IxOsalOemInitialized = FALSE;

extern struct uts_namespace init_uts_ns;

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

PUBLIC IX_STATUS
ixOsalOemInit (void)
{
    /*
     * Check flag 
     */
    if (IxOsalOemInitialized == TRUE)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalOemInit():  already initialized. \n", 0, 0, 0, 0, 0, 0);
        return IX_FAIL;
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
    strncpy(osName, init_uts_ns.name.sysname, maxSize);
    return IX_SUCCESS;
}

PUBLIC IX_STATUS
ixOsalOsIxp400VersionGet(INT8* osVersion, INT32 maxSize)
{
    strncpy(osVersion, init_uts_ns.name.release, maxSize);
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

