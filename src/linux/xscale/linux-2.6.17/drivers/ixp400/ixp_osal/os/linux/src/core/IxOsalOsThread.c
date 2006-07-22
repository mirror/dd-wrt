/**
 * @file IxOsalOsThread.c (linux)
 *
 * @brief OS-specific thread implementation.
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

#include <linux/sched.h>

#include "IxOsal.h"

PRIVATE struct
{
    IxOsalVoidFnVoidPtr entryPoint;
    void *arg;
} IxOsalOsThreadData;

/* declaring mutex */
DECLARE_MUTEX (IxOsalThreadMutex);

static int
thread_internal (void *unused)
{
    IxOsalVoidFnVoidPtr entryPoint = IxOsalOsThreadData.entryPoint;
    void *arg = IxOsalOsThreadData.arg;
    static int seq = 0;

    lock_kernel();
    daemonize ("IxOsal %d", seq+1);
    unlock_kernel();

    snprintf(current->comm, sizeof(current->comm), "IxOsal %d", ++seq);

    up (&IxOsalThreadMutex);

    entryPoint (arg);
    return 0;
}

/* Thread attribute is ignored */
PUBLIC IX_STATUS
ixOsalThreadCreate (IxOsalThread * ptrTid,
    IxOsalThreadAttr * threadAttr, IxOsalVoidFnVoidPtr entryPoint, void *arg)
{
    down (&IxOsalThreadMutex);
    IxOsalOsThreadData.entryPoint = entryPoint;
    IxOsalOsThreadData.arg = arg;

    /*
     * kernel_thread takes: int (*fn)(void *)  as the first input.
     */
    *ptrTid = kernel_thread (thread_internal, NULL, CLONE_SIGHAND);

    if (*ptrTid < 0)
    {
        up (&IxOsalThreadMutex);
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalThreadCreate(): fail to generate thread \n",
            0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

    return IX_SUCCESS;

}

/* 
 * Start thread after given its thread handle
 */
PUBLIC IX_STATUS
ixOsalThreadStart (IxOsalThread * tId)
{
    /* Thread already started upon creation */
    return IX_SUCCESS;
}

/*
 * In Linux threadKill does not actually destroy the thread,
 * it will stop the signal handling.
 */
PUBLIC IX_STATUS
ixOsalThreadKill (IxOsalThread * tid)
{
    kill_proc (*tid, SIGKILL, 1);
    return IX_SUCCESS;
}

PUBLIC void
ixOsalThreadExit (void)
{
    ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,
        IX_OSAL_LOG_DEV_STDOUT,
        "ixOsalThreadExit(): not implemented \n", 0, 0, 0, 0, 0, 0);
}

PUBLIC IX_STATUS
ixOsalThreadPrioritySet (IxOsalOsThread * tid, UINT32 priority)
{
    return IX_SUCCESS;
}

PUBLIC IX_STATUS
ixOsalThreadSuspend (IxOsalThread * tId)
{
    ixOsalLog (IX_OSAL_LOG_LVL_WARNING,
        IX_OSAL_LOG_DEV_STDOUT,
        "ixOsalThreadSuspend(): not implemented in linux \n",
        0, 0, 0, 0, 0, 0);
    return IX_SUCCESS;

}

PUBLIC IX_STATUS
ixOsalThreadResume (IxOsalThread * tId)
{
    ixOsalLog (IX_OSAL_LOG_LVL_WARNING,
        IX_OSAL_LOG_DEV_STDOUT,
        "ixOsalThreadResume(): not implemented in linux \n",
        0, 0, 0, 0, 0, 0);
    return IX_SUCCESS;

}
