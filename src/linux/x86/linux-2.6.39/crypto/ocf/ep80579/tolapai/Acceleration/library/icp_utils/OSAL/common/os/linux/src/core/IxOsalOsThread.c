/**
 * @file IxOsalOsThread.c (linux)
 *
 * @brief OS-specific thread implementation.
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

#include <linux/sched.h>

#ifdef IX_OSAL_OS_LINUX_VERSION_2_6

#include <linux/list.h>
#include <linux/kthread.h>
#include <linux/hardirq.h>

#endif /* IX_OSAL_OS_LINUX_VERSION_2_6 */

/*  Thread Data structure */
struct IxOsalOsThreadData
{
   IxOsalVoidFnVoidPtr entryPoint;
   void                *arg;
};

/* declaring mutexes */
DECLARE_MUTEX (IxOsalThreadMutex);
DECLARE_MUTEX (IxOsalThreadStopMutex);

#ifndef IX_OSAL_OS_LINUX_VERSION_2_6   /* ! Linux-Kernel Version 2.6 */

struct IxOsalOsThreadData thread_data;
struct task_struct        *kill_task = NULL;

#endif


#ifdef IX_OSAL_OS_LINUX_VERSION_2_6   /* Linux Kernel Version 2.6 */

/*  Thread info structure */
struct IxOsalOsThreadInfo
{
    struct IxOsalOsThreadData data;
    IxOsalThread              ptid;
    struct list_head          list;
};


/* Thread attribute is ignored in Create */

PUBLIC IX_STATUS
ixOsalThreadCreate (IxOsalThread * ptrTid,
    IxOsalThreadAttr * threadAttr, IxOsalVoidFnVoidPtr entryPoint, void *arg)
{
    *ptrTid = kthread_create((void *)entryPoint, arg, "%s",
                (NULL != threadAttr && NULL != threadAttr->name)
                ? threadAttr->name: "OSAL");

    return IX_SUCCESS;
}


PUBLIC IX_OSAL_INLINE BOOL
ixOsalThreadStopCheck(void)
{
    INT32 err = kthread_should_stop();
    return err ? TRUE : FALSE;
}

/**
 * Start the thread
 */
PUBLIC IX_STATUS
ixOsalThreadStart (IxOsalThread *tId)
{
    if (NULL == *tId)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalThreadStart(): Invalid Thread ID!\n", 0, 0, 0, 0, 0, 0);
             return IX_FAIL;
    }
 
    wake_up_process(*tId);
    return IX_SUCCESS;
}

#ifdef IX_OSAL_THREAD_EXIT_GRACEFULLY
/* Exit status check is possible only in kernel 2.6.10 and greater */

/*
 * Kill the kernel thread. This shall not be used if the thread function
 * implements do_exit()
 */
PUBLIC IX_STATUS
ixOsalThreadKill (IxOsalThread * tid)
{
    struct task_struct *task = (struct task_struct*)*tid;

    /* Can't kill already defunc thread */
    if (EXIT_DEAD == task->exit_state || EXIT_ZOMBIE == task->exit_state)
    {
         return IX_FAIL;
    }

    if (-EINTR == kthread_stop(task))
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalThreadKill(): Failed to kill thread\n", 0, 0, 0, 0, 0, 0);
       
        return IX_FAIL;
    }

    return IX_SUCCESS;
}

#else /* __! EXIT_GRACEFULLY */

PUBLIC IX_STATUS
ixOsalThreadKill (IxOsalThread * tid)
{
  kill_proc ((pid_t)*tid, SIGKILL, 1);
  return IX_SUCCESS;
}

#endif /* IX_OSAL_THREAD_EXIT_GRACEFULLY */



/********************************************************************
 * UINT32 priority - the value of priority can range from 0 to 39   *
 *                   with 0 being the highest priority.				*
 * 																	*
 * Any value for priority more than 39 will be silently rounded off *
 * to 39 in this implementation. Internally, the range is converted *
 * to the corresponding nice value that can range from -20 to 19.	*
 ********************************************************************/
 
PUBLIC IX_STATUS
ixOsalThreadPrioritySet (IxOsalThread * tid, UINT32 priority)
{
    struct task_struct *pTask = (struct task_struct*)*tid;

    IX_OSAL_LOCAL_ENSURE(tid,
            "ixOsalThreadPrioritySet():  Null pointer", 
            IX_FAIL);

    if (priority > IX_OSAL_PRIO_SET_MAX_VALID_VAL)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                   "ixOsalThreadPrioritySet(): FAIL \n", 0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

    if (priority > IX_OSAL_PRIO_SET_MAX_VAL) 
    {
         priority = IX_OSAL_PRIO_SET_MAX_VAL;
    }
    if (priority < 0) 
    {
         priority = 0;
    }

    /* sending the nice equivalent of priority as the parameter */
    set_user_nice ( pTask, priority - IX_OSAL_NICE_VAL_DIFFERENCE ); 

    ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE, IX_OSAL_LOG_DEV_STDOUT,
        "ixOsalThreadPrioritySet(): Priority changed successfully \n",
        0, 0, 0, 0, 0, 0);

    return IX_SUCCESS;
}

/**
 ***********************************************************
 * End of code cleanup section 
 ***********************************************************
 */

#else  /* ! LINUX_VERSION_2_6 */

PUBLIC IX_OSAL_INLINE BOOL
ixOsalThreadStopCheck()
{
    if (current == kill_task)
    {
        kill_task = NULL;
        up(&IxOsalThreadStopMutex);
        return TRUE;
    }
    return FALSE;
}

static int
thread_internal (void *unused)
{
    IxOsalVoidFnVoidPtr entryPoint = thread_data.entryPoint;
    void *arg = thread_data.arg;
    static int seq = 0;

    daemonize();
    reparent_to_init ();
    exit_files (current);

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
    thread_data.entryPoint = entryPoint;
    thread_data.arg = arg;

    /*
     * kernel_thread takes: int (*fn)(void *)  as the first input.
     */
    *ptrTid = kernel_thread (thread_internal, NULL, CLONE_SIGHAND);

    if (*ptrTid < 0)
    {
        up (&IxOsalThreadMutex);
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
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
    ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE, IX_OSAL_LOG_DEV_STDOUT,
        "ixOsalThreadStart(): not implemented in linux\n",
        0, 0, 0, 0, 0, 0);
    return IX_SUCCESS;
}


PUBLIC IX_STATUS
ixOsalThreadKill (IxOsalThread * tid)
{
    down(&IxOsalThreadStopMutex);
    kill_task = find_task_by_pid(*tid);

    if (kill_task)
    {
            wake_up_process(kill_task);

            return IX_SUCCESS;
    }

    ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
       "ixOsalThreadKill: Task %d was dead\n", *tid, 0, 0, 0, 0, 0);

    /* Kill failed, remove the mutex */
    up(&IxOsalThreadStopMutex);
    return IX_FAIL;
}

#endif /* IX_OSAL_OS_LINUX_VERSION_2_6 */

PUBLIC void
ixOsalThreadExit (void)
{
    ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE, IX_OSAL_LOG_DEV_STDOUT,
        "ixOsalThreadExit(): not implemented in linux\n", 0, 0, 0, 0, 0, 0);
}

PUBLIC IX_STATUS
ixOsalThreadSuspend (IxOsalThread * tId)
{
    ixOsalLog (IX_OSAL_LOG_LVL_WARNING, IX_OSAL_LOG_DEV_STDOUT,
       "ixOsalThreadSuspend(): not implemented in linux\n", 0, 0, 0, 0, 0, 0);
    return IX_SUCCESS;
}

PUBLIC IX_STATUS
ixOsalThreadResume (IxOsalThread * tId)
{
    ixOsalLog (IX_OSAL_LOG_LVL_WARNING, IX_OSAL_LOG_DEV_STDOUT,
        "ixOsalThreadResume(): not implemented in linux \n", 0, 0, 0, 0, 0, 0);
    return IX_SUCCESS;

}

IX_STATUS 
ixOsalThreadGetId(IxOsalThread *ptrTid)
{
    *ptrTid = (IxOsalThread)current->pid;
 
    return IX_SUCCESS;
 
} /* ixOsalThreadGetId */

PUBLIC IX_STATUS 
ixOsalThreadGetPolicyAndPriority(
            IxOsalThread        *tid, 
            UINT32              *policy, 
            UINT32              *priority)
{
#ifdef IX_OSAL_OS_LINUX_VER_GT_2_6_18
    struct task_struct *pTask = (struct task_struct*)*tid;
 
    IX_OSAL_ENSURE_RETURN(tid, "Null pointer");
    /* set the policy of existing */
    *policy = pTask->policy;
    *priority = pTask->rt_priority;
    return IX_SUCCESS;

#else
    IX_STATUS err = IX_SUCCESS;
    struct task_struct *pTask;
 
    IX_OSAL_ENSURE_RETURN(tid, "Null pointer");
    lock_kernel();
 
    /* In case of Linux Kernel 2.4:
     * pTask = find_task_by_pid(*tid); */
 
    pTask = *tid;
 
    if(pTask == 0)
    {
        err = IX_FAIL;
    }else{
        *policy = pTask->policy; 
        *priority = pTask->rt_priority;
    }
 
    unlock_kernel();
    return err;
#endif /* IX_OSAL_OS_LINUX_VER_BT_2_6_18 */

}

