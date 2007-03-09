/**
 * @file
 *
 * @brief The file contains implementation for the OS service layer.
 *
 * 
 * @par
 * IXP400 SW Release version  2.0
 * 
 * -- Intel Copyright Notice --
 * 
 * @par
 * Copyright 2002-2005 Intel Corporation All Rights Reserved.
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

 
#include "IxOsalBackward.h"

PUBLIC IX_STATUS
ixOsServIntBind (int level, void (*routine) (void *), void *parameter)
{

    return ixOsalIrqBind ((UINT32) level,
        (IxOsalVoidFnVoidPtr) routine, parameter);
}

PUBLIC IX_STATUS
ixOsServIntUnbind (int level)
{
    return ixOsalIrqUnbind ((UINT32) level);
}

PUBLIC int
ixOsServIntLock (void)
{
    return ((int) ixOsalIrqLock ());
}

PUBLIC void
ixOsServIntUnlock (int lockKey)
{
    ixOsalIrqUnlock ((UINT32) lockKey);
}

PUBLIC int
ixOsServIntLevelSet (int level)
{
    return ((int) ixOsalIrqLevelSet ((UINT32) level));
}

PUBLIC IX_STATUS
ixOsServMutexInit (IxMutex * mutex)
{
    return ixOsalMutexInit ((IxOsalMutex *) mutex);
}

PUBLIC IX_STATUS
ixOsServMutexLock (IxMutex * mutex)
{
    return ixOsalMutexLock ((IxOsalMutex *) mutex, IX_OSAL_WAIT_NONE);
}

PUBLIC IX_STATUS
ixOsServMutexUnlock (IxMutex * mutex)
{
    return ixOsalMutexUnlock ((IxOsalMutex *) mutex);
}

PUBLIC IX_STATUS
ixOsServMutexDestroy (IxMutex * mutex)
{
    return ixOsalMutexDestroy ((IxOsalMutex *) mutex);
}

PUBLIC IX_STATUS
ixOsServFastMutexInit (IxFastMutex * mutex)
{
    return ixOsalFastMutexInit ((IxOsalFastMutex *) mutex);
}

PUBLIC IX_STATUS
ixOsServFastMutexTryLock (IxFastMutex * mutex)
{
    return ixOsalFastMutexTryLock ((IxOsalFastMutex *) mutex);
}

PUBLIC IX_STATUS
ixOsServFastMutexUnlock (IxFastMutex * mutex)
{
    return ixOsalFastMutexUnlock ((IxOsalFastMutex *) mutex);
}

PUBLIC int
ixOsServLog (int level, char *format, int arg1, int arg2, int arg3, int arg4,
    int arg5, int arg6)
{

    return ((int) ixOsalLog ((IxOsalLogLevel) level,
            IX_OSAL_LOG_DEV_STDOUT,
            format, arg1, arg2, arg3, arg4, arg5, arg6));
}

PUBLIC int
ixOsServLogLevelSet (int level)
{
    return ((int) ixOsalLogLevelSet ((UINT32) level));
}

PUBLIC void
ixOsServSleep (int microseconds)
{
    ixOsalBusySleep ((UINT32) microseconds);
}

PUBLIC void
ixOsServTaskSleep (int milliseconds)
{
    ixOsalSleep ((UINT32) milliseconds);
}

PUBLIC unsigned int
ixOsServTimestampGet (void)
{
    return ((unsigned int) ixOsalTimestampGet ());
}

/* Not implemented in OSAL */
PUBLIC void
ixOsServUnload (void)
{
    ixOsalLog (IX_OSAL_LOG_LVL_WARNING,
        IX_OSAL_LOG_DEV_STDOUT,
        "ixOsServUnload():  No longer supported in OSAL  \n",
        0, 0, 0, 0, 0, 0);
}

PUBLIC void
ixOsServYield (void)
{
    ixOsalYield ();
}
 
