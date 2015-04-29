/**
 * @file
 *
 * @brief The file contains implementation for the OS service layer.
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
 
