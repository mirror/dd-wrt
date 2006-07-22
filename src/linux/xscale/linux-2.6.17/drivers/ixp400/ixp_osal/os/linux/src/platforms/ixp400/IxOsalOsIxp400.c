/**
 * @file IxOsalOsIxp400.c  (linux)
 *
 * @brief this file contains implementation for platform-specific
 *        functionalities.
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

#include <linux/utsname.h>
#include "IxOsal.h"

static volatile UINT32 *ixOsalOstsRegAddr = NULL;

PRIVATE BOOL IxOsalOemInitialized = FALSE;

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

